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
/// \brief Create OpenInventor models to display XYZ points.

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

// C includes
#include <cassert>

#include <cstdlib>
#include <cstdio>

// C++ includes
#include <iostream>
#include <iomanip>
#include <fstream>

#include <string>	// Good STL data types.
#include <vector>

// Local includes
#include "xyz_iv_cmd.h"

using namespace std;

/***************************************************************************
 * MACROS, DEFINES, GLOBALS
 ***************************************************************************/

#include "debug.H" // provides FAILED_HERE, UNUSED, DebugPrintf
int debug_level=0;

/// Let the debugger find out which version is being used.
static const UNUSED char* RCSid ="$Id$";

/***************************************************************************
 * Local routines
 ***************************************************************************/

/// \brief load ascii whitespace delimited text xyz into vectors.
/// \return \a true if all went well.  \a false if trouble of any kind
/// \param filename File to open and read data from
/// \param x,y,z Return the xyz coordinates of each point
///
/// Unlike Lisa's code, this one does NOT alter the sigmas on loading
/// which is what the adread subroutine did.`
/// You must call SiteSigma if doing a Site based Parametric Bootstrap
bool
LoadData(const string filename,vector<float> &x,vector<float> &y,vector<float> &z) {
  DebugPrintf(TRACE,("loading file: %s\n",filename.c_str()));
  ifstream in(filename.c_str(),ios::in);
  if (!bool(in)) {cerr << "failed to open file: " << filename << endl; return false; }

  // FIX: allow comments, blank lines, etc.
  vector<float> tmp(3,0.);
  while (in >> tmp[0] >> tmp[1] >> tmp[2]) {
    DebugPrintf(BOMBASTIC,("read: %+.7f %+.7f %+.7f\n",tmp[0],tmp[1],tmp[2]));
    x.push_back(tmp[0]);  y.push_back(tmp[1]);  z.push_back(tmp[2]);
  }
  return (true);
}


bool Color(ofstream &o, const string &color_str) {
  // FIX: validate the string
  o << "\tBaseColor {rgb [ " << color_str << " ] }" << endl;
  return (true);
}

/// \brief Write out a small box at each point
/// \param o Write the Inventor model info to this stream
/// \param x,y,z Float vectors with the point data
/// \param w Width, Height and Depth for the cube
/// \return \a false if there was trouble
bool Boxes(ofstream &o, 
	 const vector<float> &x, const vector<float> &y, const vector<float> &z,
	 const float w)
{
  DebugPrintf(TRACE,("Boxes: %d points, width = %f\n",int(x.size()),w));
  if (x.size() != y.size() || y.size()!=z.size()) return (false);
  for (size_t i=0;i<x.size();i++) {
    o << "\tSeparator{ Translation { translation " << x[i] << " " << y[i] << " " << z[i]
      << " } Cube { width "<<w<<" height "<<w<<" depth "<<w<<" } }"<<endl;
  }
  return (true);
}

/// \brief Write out a small sphere at each point
/// \param o Write the Inventor model info to this stream
/// \param x,y,z Float vectors with the point data
/// \param r Radius of the spheres
/// \return \a false if there was trouble
///
/// You may want to add an SoComplexity node in front of the spheres. If rendering is
/// too slow.
///
///  Complexity { value .10 }
bool Spheres(ofstream &o, 
	 const vector<float> &x, const vector<float> &y, const vector<float> &z,
	 const float r)
{
  DebugPrintf(TRACE,("Spheres: %d points, radius = %f\n",int(x.size()),r));
  if (x.size() != y.size() || y.size()!=z.size()) return (false);
  for (size_t i=0;i<x.size();i++) {
    o << "\tSeparator{ Translation { translation " << x[i] << " " << y[i] << " " << z[i]
      << " } Sphere { radius "<<r<<" } }"<<endl;
  }
  return (true);
}


/// \brief Use SoLineSet to connect the dots
bool Linked(ofstream &o, 
	 const vector<float> &x, const vector<float> &y, const vector<float> &z)
{
  DebugPrintf(TRACE,("Linked: %d points\n",int(x.size())));
  if (x.size() != y.size() || y.size()!=z.size()) return (false);
  o << "\tCoordinate3 { point ["<<endl;
  for (size_t i=0;i<x.size();i++) {
    o << "\t\t"<<x[i]<<" "<<y[i]<<" "<<z[i]<<","<<endl;
  }
  o << "\t] } # Coordinate3" << endl;
  o << "\tLineSet { numVertices [ "<<x.size()<< " ] }" << endl;
  return (true);
}

/// \brief Use SoLineSet to connect the dots
bool PolarLines(ofstream &o, 
	 const vector<float> &x, const vector<float> &y, const vector<float> &z)
{
  DebugPrintf(TRACE,("PolarLines: %d points\n",int(x.size())));
  if (x.size() != y.size() || y.size()!=z.size()) return (false);
  o << "\tCoordinate3 { point ["<<endl
    << "\t\t0. 0. 0.,"<<endl;
  for (size_t i=0;i<x.size();i++) {
    o << "\t\t"<<x[i]<<" "<<y[i]<<" "<<z[i]<<","<<endl;
  }
  o << "\t] } # Coordinate3" << endl;

  o << "\tIndexedLineSet { coordIndex [ " << endl;
  for (size_t i=0;i<x.size();i++) {
    o << "\t\t0,"<<i+1<<",-1,"<<endl;
  }
  o << "\t] } # IndexedLineSet" << endl;

  return (true);
}

/***************************************************************************
 * MAIN
 ***************************************************************************/

int main (int argc, char *argv[]) {
  bool ok=true;

  gengetopt_args_info a;
  if (0!=cmdline_parser(argc,argv,&a)) {
    cerr << "FIX: should never get here" << endl;
    cerr << "Early exit" << endl;
    return (EXIT_FAILURE);
  }

#ifdef NDEBUG
  if (a.verbosity_given) {
    cerr << "Verbosity is totally ignored for optimized code.  Continuing in silent mode" << endl;
  }
#else // debugging
  debug_level = a.verbosity_arg;
  DebugPrintf(TRACE,("Debug level = %d\n",debug_level));
#endif

  //////////////////////////////////////////////////////////////////////
  // ERROR CHECK ARGS
  //////////////////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////////////////////
  // GET TO WORK YOU SLACKER
  //////////////////////////////////////////////////////////////////////

  if (0==a.inputs_num) {
    cerr << "WARNING: No input files specified!" << endl;
    ok=false;
  }

  vector <float> x,y,z;
  for (size_t i=0;i<a.inputs_num;i++) {
    if (!LoadData(string(a.inputs[i]),x,y,z)) {
      ok=false;  cerr << "WARNING: failed to load an input file.  Continuing."<<endl;
    }
  }

  ofstream o(a.out_arg,ios::out);

  o << "#Inventor V2.1 ascii" << endl<<endl;
  o << "# Written by " << RCSid << endl << endl;

  o << "# cmdline: ";
  for (int i=0;i < argc;i++) o << argv[i] << " ";
  o << endl << endl;

  o << "Separator {"  << endl;

  if (a.color_given) {
    if (!Color(o,string(a.color_arg))) {
      ok=false; cerr << "WARNING: failed to write colorx. Continuing." << endl;
    }
  }
  if (a.box_given) {
    if (!Boxes(o,x,y,z,a.box_arg)) {
      ok=false; cerr << "WARNING: failed to write boxes. Continuing." << endl;
    }
  }

  if (a.linked_given) {
    if (!Linked(o,x,y,z)) {
      ok=false; cerr << "WARNING: failed to write linked. Continuing." << endl;
    }
  }

  if (a.polarlines_given) {
    if (!PolarLines(o,x,y,z)) {
      ok=false; cerr << "WARNING: failed to write polarlines. Continuing." << endl;
    }
  }

  if (a.sphere_given) {
    if (!Spheres(o,x,y,z,a.sphere_arg)) {
      ok=false; cerr << "WARNING: failed to write sphere. Continuing." << endl;
    }
  }

  o << "} # Separator - EOF"  << endl;
  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}
