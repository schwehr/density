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

#include "xyzvol_cmp_cmd.h"

using namespace std;

/***************************************************************************
 * MACROS, DEFINES, GLOBALS
 ***************************************************************************/

#include "debug.H" // provides FAILED_HERE, UNUSED, DebugPrintf
#ifndef NDEBUG
int debug_level;
#endif

/// Let the debugger find out which version is being used.
static const UNUSED char* RCSid ="@(#) $Id$";

//######################################################################
// MAIN
//######################################################################

int main (int argc, char *argv[]) {
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
  DebugPrintf(TRACE,("Debug level = %d",debug_level));
#endif

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


  //ifstream in(a.inxyz_arg,ios::in);
  //if (!in.is_open()) {cerr<<"Failed to open "<<a.inxyz_arg<<endl;return(EXIT_FAILURE);}
  bool ok=true;
  for (size_t filenum=0;filenum < a.inputs_num; filenum++) {
    DebugPrintf(TRACE,("testing file: %s",a.inputs[filenum]));
    ifstream in(a.inputs[filenum],ios::in);
    string filename(a.inputs[filenum]);
    if (!in.is_open()) {cerr<<"Failed to open "<<a.inputs[filenum]<<endl;ok=false;continue;}

    char buf[1024];
    while (in.getline(buf,1024)) {
      if ('#'==buf[0]) continue; // Comment
      istringstream istr(buf);
      float x,y,z;
    
      istr >> x >> y >> z;
      const size_t cellIndex = d.getCell(x,y,z);
      const size_t count = d.getCellCount(cellIndex);
      cout << densityFileName << " " << filename << " " << x << " " << y << " " << z << "    "
	   << count << " " << float(count)/d.getCountInside() << " " << cdf[count] << endl;

    }
  } // for filenum
  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}
