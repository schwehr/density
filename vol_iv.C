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
/// \brief Create wrapper iv files for volume density files.




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

#include "vol_iv_cmd.h"

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

/***************************************************************************
 * LOCAL FUNCTIONS
 ***************************************************************************/


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

  // FIX: allow more than one input volume
  if (1!=a.inputs_num) {
    cerr << "ERROR: Must specify either 0 or 1 inputfile.  You gave 2 or more!" << endl;
    return (EXIT_FAILURE);
  }

  ofstream o(a.out_arg,ios::out);
  if (!o.is_open()) {
    cerr << "ERROR: failed to open output file!" << endl;
    return (EXIT_FAILURE);
  }

  o << "#Inventor V2.1 ascii" << endl << endl
    << "# Voleon IV wrapper file by: $Id$ " << endl;

  {
    o << "Separator { " << endl;

    if (a.scale_given) o << "\tScale { scaleFactor " 
			 << a.scale_arg << " " << a.scale_arg << " " << a.scale_arg
			 << "} " << endl;

    if (a.box_given)
      o << "\tSeparator {" << endl
	<< "\t\tDrawStyle { style LINES }" << endl
	<< "\t\tPickStyle { style UNPICKABLE }" << endl
	<< "\t\tCube { width "<<a.box_arg<<" height "<<a.box_arg<<" depth "<<a.box_arg<<"}" << endl
	<< "\t}" << endl;
    o << "\tSoVolumeData {" << endl
      << "\t\tfileName \"" << a.inputs[0] << "\"" << endl
      << "\t}" <<endl
      ;


    o << "\tSoTransferFunction {" << endl;
    if(a.predefcmap_given) o << "\t\tpredefColorMap " << a.predefcmap_arg << endl;
    if(a.cmaptype_given) o << "\t\tcolorMapType" << a.cmaptype_arg << endl;
    if(a.cmap_given) {
      o << "\t\tcolorMap [ ";
      cerr << "FIX: write out a color map here" << endl;
      o << "\t\t]" << endl;
    }
    o << "\t}" << endl;

    // file:///sw/share/SIMVoleon/html/classSoVolumeRender.html
    o << "\tSoVolumeRender {" << endl;
    if (a.interpolation_given)    o << "\t\tinterpolation "    << a.interpolation_arg << endl;
    if (a.composition_given)      o << "\t\tcompsition "       << a.composition_arg << endl;
    if (a.numslicescontrol_given) o << "\t\tnumSlicesControl " << a.numslicescontrol_arg << endl;
    if (a.numslices_given)        o << "\t\tnumSlices "        << a.numslices_arg << endl;


    //o << "\t\tinterpolation LINEAR" << endl;
    //o << "    #composition SUM_INTENSITY" << endl;
    //o << "    #composition MAX_INTENSITY" << endl;
    //o << "    numSlicesControl ALL" << endl;
    //o << "    numSlices 40" << endl;
    //o << "" << endl;
    o << "\t}" << endl;

    o << "} # End of safety separator" << endl;
  }
  o << endl << "# EOF ( " << a.out_arg << " )" << endl;


  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}
