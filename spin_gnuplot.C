// $Revision$  $Author$  $Date$
/*
    Copyright (C) 2004  Kurt Schwehr

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

/// \file
/// \brief Produce frames for a gnuplot movie

#include <vector>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// local includes
#include "spin_gnuplot_cmd.h"

/***************************************************************************
 * LOCAL MACROS and DEFINES
 ***************************************************************************/

#include "debug.H" // provides FAILED_HERE, UNUSED, DebugPrintf
int debug_level;

/***************************************************************************
 * LOCAL FUNCTIONS
 ***************************************************************************/

/// \brief Read the contents of a file into a string.
/// \param filename File to read from disk
/// \param result Returns \a true if ok, \a false if something bad
/// \return The entire file turned into a string

/// Hope the file is small!  What happens with a huge beast of a file?<br>
/// FIX: return a reference?<br>
/// FIX: the params are not getting handled by doxygen.  Why?<br>
string
GetStringFromFile(const char *filename, bool &result) {
  assert(filename);
  result=true;  // Innocent until proven guilty
  ifstream in(filename, ios::in);
  if (!in.is_open()) {
    cerr << "ERROR: unable to open file "<<filename<<" in " << __FUNCTION__ << endl;
    result=false;
    return (string("ERROR: unable to read ") + string(filename));
  }

  string lines;
  const size_t bufSize=1024;
  char buf[bufSize];
  while (in.getline(buf,bufSize)) {
    lines += buf;
    lines += "\n";
  }

  return (lines);
}


bool
AddFile(ofstream &out, const char *filename) {
  assert(filename);
  ifstream in(filename, ios::in);
  if (!in.is_open()) {
    cerr << "ERROR: unable to open file "<<filename<<" in " << __FUNCTION__ << endl;
    return (false);
  }

  out << "#\n# PREAMBLE from " << filename << "\n#" << endl;

  const size_t bufSize=1024;
  char buf[bufSize];
  while (in.getline(buf,bufSize)) {
    out << buf << endl;
  }
  return (true);
}


/// \brief Animate over a box path
/// \param out Stream to write gnuplot commands to.
/// \param plotCmd string to pass to splot
/// \param stepSize How much to rotate for each step - for set view
/// \param xmin,xmax Horizontal rotation
/// \param ymin,ymax Vertical look range
/// \param basename beginning of the frame filename
/// \param ext Extension to add to the end of the frame name (please include the . if you want one)
/// \return \a false if something bad happened
///
/// This is the first example of a path traversal algorithm.  Use it
/// for an example if you would like to write some sort of new path.
/// FIX: need to have a 2D way to step through plots too.  Maybe be able to set
/// a different data file for each frame?
bool
BoxAnimation (ofstream &out, const string &plotCmd, const float stepSize,
	      const float xmin, const float xmax, const float ymin, const float ymax,
	      const string basename, const string ext)
{
  float rot_x, rot_y;
  size_t frameNum=0;
  const size_t frameNameSize=1024;
  char frameName[frameNameSize];
  rot_y=ymin;
  for (rot_x=xmin; rot_x<=xmax; rot_x+= stepSize) {
    if (0>snprintf(frameName,frameNameSize,"%s%04d.%s",basename.c_str(),int(frameNum++),ext.c_str())) {
      perror ("snprintf failed in BoxAnimation");
      FAILED_HERE;
      return false;
    }
    //out << "set view " << rot_x << "," << rot_y << "\n"
    // FIX: make sure that the xy order is right
    out << "set view " << rot_y << "," << rot_x << "\n"
	<< "set output '" << frameName << "'\n"
	<< "splot " << plotCmd.c_str() << "\n"
	<< endl;
    // FIX: error check cout
  }
  for (rot_y=ymin; rot_y<=ymax; rot_y+= stepSize) {
    if (0>snprintf(frameName,frameNameSize,"%s%04d.%s",basename.c_str(),int(frameNum++),ext.c_str())) {
      perror ("snprintf failed in BoxAnimation");
      FAILED_HERE;
      return false;
    }
    out << "set view " << rot_y << "," << rot_x << "\n"
	<< "set output '" << frameName << "'\n"
	<< "splot " << plotCmd.c_str() << "\n"
	<< endl;
 }

  return (true);
}


/***************************************************************************
 * MAIN 
 ***************************************************************************/
int main (int argc, char *argv[]) {
  gengetopt_args_info a;  // a == args
  if (0!=cmdline_parser(argc,argv,&a)){cerr<<"MELT DOWN: should never get here"<<endl;return (EXIT_FAILURE);}

  debug_level=a.verbosity_arg;

  if (a.FileInput_given && a.StringInput_given) {
    cerr << "ERROR can not specify both File and String input for the  "<< endl;
    return (EXIT_FAILURE);
  }

  ofstream out(a.out_arg, ios::out);
  if (!out.is_open()) {
    cerr << "ERROR: Failed to open " << a.out_arg << endl;
    return (EXIT_FAILURE);
  }

  //
  // Write out a bit of header info 
  //
  out << "# -*- gnuplot -*-\n" 
      << "# Autogenerated gnuplot file created with the following command line:\n\n# "
    ;

  for (int i=0;i<argc;i++) out << argv[i] << " ";
  out << "\n" << endl;

  out << "set terminal " << a.format_arg << endl;


  //
  // Do the preable after header stuff so that users can override
  //
  if (a.preamble_given) {
    if (!AddFile(out,a.preamble_arg)) {
      // bad kitty! bad!
      cerr << "ERROR: failed to add preable from " << a.preamble_arg << endl;
      return (EXIT_FAILURE);
    }
  }

  out << "set terminal " << a.format_arg << endl;

  //string plotCmd("splot ");
  string plotCmd;

  if (a.FileInput_given) {
    bool result;
    plotCmd += GetStringFromFile(a.FileInput_arg,result);
    if (!result) {
      cerr << "ERROR: unable to read plot string from " << a.FileInput_arg << endl;
      exit(EXIT_FAILURE);
    }
  } else if (a.StringInput_given) {
    plotCmd += string(a.StringInput_arg);
  } else {
    cerr << "ERROR: listen bub...  You need to specify either SpringInput or FileInput\n" 
	 << "  I have nothing to plot, so I give up." << endl;
    exit(EXIT_FAILURE);
  }

  DebugPrintf(TRACE,("Plot command: %s\n",plotCmd.c_str()));


  if (!BoxAnimation (out, plotCmd, a.StepSize_arg, a.xmin_arg, a.xmax_arg, a.zmin_arg, a.zmax_arg,
		     string(a.basename_arg), string(a.format_arg))) {
    cerr << "ERROR: Failed to animate" << endl;
    return (EXIT_FAILURE);
  }



  return(EXIT_SUCCESS);
} // main()

