
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

#include <string>	// Good STL data types.
#include <vector>

// Local includes
#include "Density.H"

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
  cout << "Starting " << argv[0] << endl;
  assert(3==argc);
  const string filename(argv[1]);//("as2-slump.xyz");
  const int numCellsInt(atoi(argv[2]));
  assert (0<numCellsInt);
  const size_t numCells(numCellsInt);

  Density d(numCells,numCells,numCells, -0.5,0.5,  -0.5,0.5,  -0.5,0.5);

  vector<float> x,y,z;
  ifstream in(filename.c_str(),ios::in);
  {
    float _x,_y,_z;
    while (in >> _x >> _y >> _z) {
      x.push_back(_x);y.push_back(_y);z.push_back(_z);
      d.addPoint(_x,_y,_z);
    }
  }
  //d.printCellCounts();
  d.writeVolScale(string("density.vol"));
  return (EXIT_SUCCESS);
}
