
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
#include "xyzdensity_cmd.h"  // gengetopt command line interface

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


bool LoadData(const string &filename, Density &d) {

  ifstream in(filename.c_str(),ios::in);
  cout << "FIX: check that file opened ok" << endl;

  float _x,_y,_z;
  while (in >> _x >> _y >> _z) d.addPoint(_x,_y,_z);

  return(true);
}

//######################################################################
// MAIN
//######################################################################

int main (int argc, char *argv[]) {
#if 0
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
#endif


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

  Density dens(a.width_arg,a.tall_arg,a.depth_arg,
	       a.xmin_arg, a.xmax_arg,
	       a.ymin_arg, a.ymax_arg,
	       a.zmin_arg, a.zmax_arg
	       );

  if (!LoadData(infile,dens)) {
    cerr << endl
	 << "ERROR: Unable to read data from file." << endl
	 << endl
	 << "  Data must be ascii space separated positive tripples on each line like this" << endl
	 << endl
	 << "    10.2 999999.2 3200.1231235"
	 << endl;
  }

  //if (!dens.writeVol(outfile,size_t(a.bpv_arg),packing,0.f,0.f,0.f)) {
  if (!dens.writeVol(outfile,size_t(a.bpv_arg),packing)) {
    cerr << " ERROR: Unable to correctly write out vol file" << endl;
    return(EXIT_FAILURE);
  }

  return (EXIT_SUCCESS);
}
