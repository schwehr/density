// $Revision$  $Author$  $Date$

/// \brief Load a voxel volume and tell use about it.
///        


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
#include "VolHeader.H"
#include "volinfo_cmd.h"
#include "Density.H"

using namespace std;


//#include <fcntl.h>   /* File control definitions */
//#include <errno.h>   /* Error number definitions */
//#include <termios.h> /* POSIX terminal control definitions */
//#include <term.h>
//#include <sys/select.h>
//#include <unistd.h>  // Select 
//#include <sys/mman.h>	// mmap
#include <sys/types.h>
#include <sys/stat.h>


/***************************************************************************
 * MACROS, DEFINES, GLOBALS
 ***************************************************************************/

#include "debug.H" // provides FAILED_HERE, UNUSED, DebugPrintf
#ifndef NDEBUG
int debug_level;
#endif

/// Let the debugger find out which version is being used.
static const UNUSED char* RCSid ="@(#) $Id$";

//####################################################################
// MAIN
//####################################################################

int main (const int argc, char *argv[]) {
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
  DebugPrintf(TRACE,("Debug level = %d",debug_level));
#endif

  for (int i=0; i<a.in_given;i++) {
    cout << i << " " << a.in_arg[i] << endl;
    const string filename(a.in_arg[i]);
    bool r;

    struct stat sb;
    {
      int r = stat (filename.c_str(), &sb);
      if (0 != r) {perror("stat to get file size FAILED");ok=false;continue;}
    }


    VolHeader v(filename,r);
    if (!r) {ok=false; cerr << "Failed to read file: " << filename << endl; continue;}
    cout << endl;
    cout << "FILE           = " << filename << endl
         << "File size      = " << sb.st_size << endl
	 << "magic_number   = " << v.getMagicNumber() << endl
	 << "header_length  = " << v.getHeaderLength() << endl
	 << "width          = " << v.getWidth()  << "   (x)" << endl
	 << "height         = " << v.getHeight() << "   (y)" << endl
	 << "images         = " << v.getImages()  << "   (depth/z)" << endl
      ;
    cout << "bits_per_voxel = " << v.getBitsPerVoxel() << "       bytes = "<<v.getBitsPerVoxel()/8<< endl
	 << "index_bits     = " << v.getIndexBits() << endl;

    cout << "scaleX         = " << v.getScaleX() << endl
	 << "scaleY         = " << v.getScaleY() << endl
	 << "scaleZ         = " << v.getScaleZ() << endl
	 << "rotX           = " << v.getRotX() << endl
	 << "rotY           = " << v.getRotY() << endl
	 << "rotZ           = " << v.getRotZ() << endl
      ;

    if (0!=a.range_given) {
      Density d(filename,r);
      if (!r) {ok=false; cerr << "Failed to read file: " << filename << endl; continue;}
      cout << "minVal         = " << d.getMinCount() << endl
	   << "maxVal         = " << d.getMaxCount() << endl;
    }
  } // for a.in_given


  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}
