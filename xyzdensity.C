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
/// \brief Convert an xyz point set into a volume density
///        Handles everything known about a triangle in a mesh


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

#include <string>	// Good STL data types.
#include <vector>

// Local includes
#include "Density.H"
#include "xyzdensity_cmd.h"  // gengetopt command line interface

using namespace std;

/***************************************************************************
 * MACROS, DEFINES, GLOBALS
 ***************************************************************************/

#include "debug.H" // provides FAILED_HERE, UNUSED, DebugPrintf
int debug_level;  // Now used even in optimized mode

/// Let the debugger find out which version is being used.
static const UNUSED char* RCSid ="@(#) $Id$";

/***************************************************************************
 * LOCAL FUNCTIONS
 ***************************************************************************/
// FIX: allow comments and use istr like LoadData4
bool LoadData(const string &filename, Density &d) {

  ifstream in(filename.c_str(),ios::in);
  if (!in.is_open()) {
    cerr << "ERROR: failed to open " << filename << endl;
    return(false);
  }

  float _x,_y,_z;
  size_t count=0; // to give feed back on progress
  while (in >> _x >> _y >> _z) {
    if      (10<=debug_level) { if (0==count%100000  && 0!=count) cout << count << endl; count ++; }
    else if ( 4<=debug_level) { if (0==count%1000000 && 0!=count) cout << count << endl; count ++;}
    d.addPoint(_x,_y,_z);
  }

  return(true);
}

/// FIX: validate this function!
/// \brief Load xyzc ascii data where c is the value to put in the cell
/// \param filename File of ascii x y z c tuples
/// \param d Density structure to fill (it is not emptied first)
/// \return \a false if trouble
bool LoadData4(const string &filename, Density &d) {

  ifstream in(filename.c_str(),ios::in);
  if (!in.is_open()) {
    cerr << "ERROR: failed to open " << filename << endl;
    return(false);
  }

  float _x,_y,_z;
  size_t c;
  size_t count=0; // to give feed back on progress
  char buf[1024];
  while (in.getline(buf,1024)) {
    if ('#'==buf[0]) continue; // Comment
    istringstream istr(buf);
    istr >> _x >> _y >> _z >> c; // FIX: error checking!
    if      (10<=debug_level) { if (0==count%100000  && 0!=count) cout << count << endl; count ++; }
    else if ( 4<=debug_level) { if (0==count%1000000 && 0!=count) cout << count << endl; count ++;}
    size_t index = d.getCell(_x,_y,_z);
    d.addPoints(index,c);
  }

  return(true);
}

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

  if (a.xmin_arg>=a.xmax_arg) {cerr<<"ERROR: xmax must be greater than xmin" << endl; return(EXIT_FAILURE);}
  if (a.ymin_arg>=a.ymax_arg) {cerr<<"ERROR: ymax must be greater than ymin" << endl; return(EXIT_FAILURE);}
  if (a.zmin_arg>=a.zmax_arg) {cerr<<"ERROR: zmax must be greater than zmin" << endl; return(EXIT_FAILURE);}


  if (a.autoscale_given)
    if ( a.xscale_given || a.yscale_given || a.zscale_given) {
      cerr << "ERROR: can not specify autoscale and specify scales too" << endl;
      return (EXIT_FAILURE);
    }


  const PackType packing=PackType(a.pack_arg);
  const string outfile(a.out_arg);

  Density dens(a.width_arg,a.tall_arg,a.depth_arg,
	       a.xmin_arg, a.xmax_arg,
	       a.ymin_arg, a.ymax_arg,
	       a.zmin_arg, a.zmax_arg
	       );

  bool ok=true; // Exit status

  for (size_t i=0;i<a.inputs_num;i++) {
    DebugPrintf(TRACE,("Loading xyz file: %s\n",a.inputs[i]));
    const string infile (a.inputs[i]);

    if (a.xyzc_flag) {
      if (!LoadData4(infile,dens)) {
	cerr << endl
	     << "ERROR: Unable to read data from file." << endl << endl
	     << "  Data must be ascii space separated triples on each line.  For example:" << endl
	     << endl
	     << "    10.2 999999.2 3200.1231235" << endl;
	ok = false;
      }
    } else {
      if (!LoadData(infile,dens)) {
	cerr << endl
	     << "ERROR: Unable to read data from file." << endl << endl
	     << "  Data must be ascii space separated triples on each line.  For example:" << endl
	     << endl
	     << "    10.2 999999.2 3200.1231235" << endl;
	ok = false;
    }
    }
  }
  // FIX: add rotation handling

  DebugPrintf(TRACE,("Points added = %d    Points missed = %d\n",
		     int(dens.getCountInside()), int(dens.getCountOutside())));

  bool r;
  if (a.autoscale_given) 
    r = dens.writeVol(outfile,size_t(a.bpv_arg),packing);
  else
    r = dens.writeVol(outfile,size_t(a.bpv_arg),packing,a.xscale_arg,a.yscale_arg,a.zscale_arg);

  if (!r) {ok=false; cerr << " ERROR: Unable to correctly write out vol file" << endl;}

  DebugPrintf(VERBOSE+1,("Exit status: %s\n", (ok?"ok":"failure") ));

  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}
