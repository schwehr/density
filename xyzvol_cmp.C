// $Revision$  $Author$  $Date$
/*
    Copyright (C) 2004  Kurt Schwehr

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
/// \file 
/// \brief Compare xyz samples to a volume.  Get the cdf desity %.


/***************************************************************************
 * INCLUDES
 ***************************************************************************/

#include <cassert>

#include <cstdlib>
#include <cstdio>

// C++ includes
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream> // string stream

// STL types
//#include <string>
//#include <vector>

// Local includes
#include "Density.H"
#include "VecAngle.H"

#include "xyzvol_cmp_cmd.h"

using namespace std;

/***************************************************************************
 * MACROS, DEFINES, GLOBALS
 ***************************************************************************/

#include "debug.H" // provides FAILED_HERE, UNUSED, DebugPrintf
int debug_level;

/// Let the debugger find out which version is being used.
static const UNUSED char* RCSid ="@(#) $Id$";

/***************************************************************************
 * HELPERS
 ***************************************************************************/


// Get back the max value and the position.
float findmax(vector<size_t> &d, size_t &position) {
  //float _max=-HUGE;
  size_t _max = 0;
  position = numeric_limits<size_t>::max();
  size_t i;
  for (i=0;i<d.size();i++) {
    if (d[i]>_max) {
      _max = d[i];
      position = i;
    }
  }
  return (_max);
}

/***************************************************************************
 * MAIN
 ***************************************************************************/

int main (int argc, char *argv[]) {
  gengetopt_args_info a;
  if (0!=cmdline_parser(argc,argv,&a)) {
    cerr << "FIX: should never get here" << endl;
    cerr << "Early exit" << endl;
    return (EXIT_FAILURE);
  }

  debug_level = a.verbosity_arg;
  DebugPrintf(TERSE,("Starting %s\n",argv[0]));
  DebugPrintf(TRACE,("Debug level = %d\n",debug_level));
#ifndef NDEBUG
  if (debug_level>=VERBOSE) {
    cout << "Command line: ";
    for (int i=0;i<argc;i++) cout << argv[i] << " ";
    cout << endl;
  }
#endif  

  if (0==a.inputs_num) {
    cerr << "ERROR: must specify input xys files to compare to the volume" << endl;
    exit(EXIT_FAILURE);
  }

  bool r;
  string densityFileName(a.density_arg);
  Density d(densityFileName,r);
  if (!r) {
    cerr << "ERROR: unable to open volume file: " << densityFileName << endl;
    return (EXIT_FAILURE);
  }

  if (a.xmin_given ||a.xmax_given ||  a.ymin_given ||a.ymax_given ||  a.zmin_given ||a.zmax_given)
    d.rescale(a.xmin_arg,a.xmax_arg,a.ymin_arg,a.ymax_arg,a.zmin_arg,a.zmax_arg);

  vector<float> cdf;
  if (!d.buildCDF(cdf)) {
    cerr << "ERROR: cdf failed to build" << endl;
    return (EXIT_FAILURE);
  }

#if 0
  for (size_t i=0; i<cdf.size(); i++) {
    cout << i << " " << cdf[i] << endl;
  }
#endif

  bool ok=true;

  ofstream out;
  const bool use_cout = ('-' == a.out_arg[0]);
  if (use_cout) {
    DebugPrintf (VERBOSE,("Setting output to stdout"));
    //out = cout;
  } else {
    out.open(a.out_arg,ios::out);
    if (!out.is_open()) {
      cerr << "ERROR: Unable to open output file." << endl;
      return(EXIT_FAILURE);
    }
  }


  for (size_t filenum=0;filenum < a.inputs_num; filenum++) {
    DebugPrintf(TRACE,("testing file: %s\n",a.inputs[filenum]));
    ifstream in(a.inputs[filenum],ios::in);
    string filename(a.inputs[filenum]);
    if (!in.is_open()) {cerr<<"Failed to open "<<a.inputs[filenum]<<endl;ok=false;continue;}

    out.setf(ios::right,ios::adjustfield);
    out << setiosflags(ios::fixed) << setprecision(6) << setw(12);

    char buf[1024];
    while (in.getline(buf,1024)) {
      if ('#'==buf[0]) continue; // Comment
      istringstream istr(buf);
      float x,y,z;
    
      istr >> x >> y >> z;
      {
	const size_t cellIndex = d.getCell(x,y,z);
	const size_t count = d.getCellCount(cellIndex);
	if (use_cout)
	  cout << densityFileName << " " << filename << " " << x << "\t" << y << "\t" << z << "\t  "
	       << count << "\t" << float(count)/d.getCountInside() << "\t" << cdf[count];
	else
	  out << densityFileName << " " << filename << " " << x << "\t" << y << "\t" << z << "\t  "
	      << count << "\t" << float(count)/d.getCountInside() << "\t" << cdf[count];
      }

      if (a.rotate_fit_given) {
	const size_t steps=500;	// FIX: need a better way to figure out the best rotational step

	DebugPrintf(VERBOSE+1,("Starting rotation compare\n"));
	vector<size_t> countCircle;
	vector<float> countAngle;

	size_t lastCell=numeric_limits<size_t>::max();
	float _x,_y;
	for (size_t i=0;i<steps;i++) {
	  const float angle=i * 2*M_PI/steps;
	  rotateXY(x,y,angle,_x,_y);
	  const size_t cellIndex = d.getCell(_x,_y,z);
	  if (cellIndex != lastCell) {
	    countCircle.push_back(d.getCellCount(cellIndex));
	    countAngle.push_back(angle);
	    //countCircle[i] = d.getCellCount(cellIndex);
	    lastCell=cellIndex;
	    cout << i << ": " << angle << " " << countCircle[countCircle.size()-1] << endl;
	  }
	} // for steps

	size_t pos;
	findmax(countCircle, pos);
	cout << "MAX is at " << pos << ": " << countCircle[pos] << " " << countAngle[pos] << endl;
	out << "\trotmax:\t" << countCircle[pos] << "\t" << countAngle[pos] << endl;
      } // rotate_fit_given
      out << endl;
    } // While new samples in the input file


  } // for filenum
  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}

