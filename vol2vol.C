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
/// \brief Load a vol voxel file and write it out in some other format


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

#include <string>	// Good STL data types.
#include <vector>

// Local includes
#include "Density.H"
#include "vol2vol_cmd.h"  // gengetopt command line interface

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
  bool r;

  gengetopt_args_info a;
  if (0!=cmdline_parser(argc,argv,&a)) {
    cerr << "FIX: should never get here" << endl;
    cerr << "Early exit" << endl;
    return (EXIT_FAILURE);
  }

  
  if (0!=a.pack_arg && 1!=a.pack_arg && 2!=a.pack_arg) {
    cerr << endl 
	 << "ERROR: Packing must be 0, 1, or 2!" << endl
	 << endl
	 << "  For usage info:" <<endl
	 << "    " << argv[0] << " --help" << endl;
    return (EXIT_FAILURE);
  }

  if (8!=a.bpv_arg && 16!=a.bpv_arg && 32!=a.bpv_arg) {
    cerr << endl 
	 << "ERROR: Bits per voxel must be 8, 16, or 32!" << endl
	 << endl
	 << "  For usage info:" <<endl
	 << "    " << argv[0] << " --help" << endl;
    return (EXIT_FAILURE);
  }

  const PackType packing=PackType(a.pack_arg);
  const string infile (a.in_arg);
  const string outfile(a.out_arg);

  Density dens(infile,r);
  if (!r) {cerr << " ERROR: unable to load volume file"<<endl; return(EXIT_FAILURE);}

  // FIX: Add ability to change number of cells.

  // FIX: if none of the [xyz]scale parameters are given, use the scale straight from the volheader

  r = dens.writeVol(outfile,size_t(a.bpv_arg),packing,a.xscale_arg,a.yscale_arg,a.zscale_arg);

  if (!r) cerr << " ERROR: Unable to correctly write out vol file" << endl;


  return (r?EXIT_SUCCESS:EXIT_FAILURE);
}