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

/// \brief Display XYZ points as OpenInventor models.

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
static const UNUSED char* RCSid ="@(#) $Id$";

/***************************************************************************
 * Local routines
 ***************************************************************************/

/// \brief load ascii whitespace delimited text xyz into vectors.
/// \return \a true if all went well.  \a false if trouble of any kind
/// \param filename File to open and read data from
/// \param s Return vector of data.  The \a s diagonalized matrix parameters.  See s_eigs
/// \param sigmas Return vector of simgma errors
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

  vector <float> x,y,z;
  for (size_t i=0;i<a.inputs_num;i++) {
    LoadData(string(a.inputs[i]),x,y,z);
  }


  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}
