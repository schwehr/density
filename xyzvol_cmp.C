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
 * LOCAL MACROS and DEFINES
 ***************************************************************************/

#ifdef __GNUC__
#define UNUSED __attribute((__unused__))
#else
/*! \def UNUSED
  \brief GNU CC attribute to denote unused paramters in function calls.
  The attribute remove compiler warning for unused arguments and variable.  Only works
  for GNU compilers such as gcc and g++.
*/
#define UNUSED
#endif

#ifdef REGRESSION_TEST
#define FAILED_HERE cout <<  __FILE__ << ":" << __LINE__ << " test failed" << endl
#else
/*! \def FAILED_HERE
  \brief Used FAILED_HERE; to emit a string that looks like a compiler warning
  Use this to allow emacs to jump to this source line with C-x `
*/
#define FAILED_HERE // Empty
#endif

/***************************************************************************
 * GLOBALS
 ***************************************************************************/

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


  bool r;
  string densityFileName(a.density_arg);
  Density d(densityFileName,r);
  if (!r) {
    cerr << "ERROR: unable to open volume file: " << densityFileName << endl;
    return (EXIT_FAILURE);
  }

  if (a.xmin_given ||a.xmax_given ||  a.ymin_given ||a.ymax_given ||  a.zmin_given ||a.zmax_given)
    d.rescale(a.xmin_arg,a.xmax_arg,a.ymin_arg,a.ymax_arg,a.zmin_arg,a.zmax_arg);

  ifstream in(a.inxyz_arg,ios::in);
  if (!in.is_open()) {cerr<<"Failed to open "<<a.inxyz_arg<<endl;return(EXIT_FAILURE);}

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

  char buf[1024];
  while (in.getline(buf,1024)) {
    if ('#'==buf[0]) continue; // Comment
    istringstream istr(buf);
    float x,y,z;
    
    istr >> x >> y >> z;
    const size_t cellIndex = d.getCell(x,y,z);
    const size_t count = d.getCellCount(cellIndex);
    cout << x << " " << y << " " << z << "    " << cellIndex << "     "
	 << count << " " << float(count)/d.getCountInside() << " " << cdf[count] << endl;

  }

  return (EXIT_SUCCESS);
}
