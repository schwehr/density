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

//####################################################################
// Globals
//####################################################################


/// Let the debugger find out which version is being used.
static const UNUSED char* RCSid ="@(#) $Id$";

//####################################################################
// MAIN
//####################################################################

int main (const int argc, char *argv[]) {
  if (argc<2) {
    cout << "Usage : " << argv[0] << " filenames" << endl
	 << "Tell about vol formatted voxel files." << endl
      ;
    return (EXIT_FAILURE);
  }

  bool ok=true;
  for (int i=1;i<argc;i++) {
    const string filename(argv[i]);
    bool r;
    VolHeader v(filename,r);
    if (!r) {ok=false; cerr << "Failed to read file: " << filename << endl; continue;}

    cout << "FILE:    " << filename << endl
	 << "magic_number   = " << v.getMagicNumber() << endl
	 << "header_length  = " << v.getHeaderLength() << endl
	 << "width  = " << v.getWidth()  << "   (x)" << endl
	 << "height = " << v.getHeight() << "   (y)" << endl
	 << "images = " << v.getImages()  << "   (depth/z)" << endl
      ;
    cout << "scaleX = " << v.getScaleX() << endl
	 << "scaleY = " << v.getScaleY() << endl
	 << "scaleZ = " << v.getScaleZ() << endl
	 << "rotX   = " << v.getRotX() << endl
	 << "rotY   = " << v.getRotY() << endl
	 << "rotZ   = " << v.getRotZ() << endl
      ;
  }

  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}
