
/// \brief Convert an xyz point set into a volume density
///        Uses a voxel representation


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


//####################################################################
// DENSITY METHODS
//####################################################################


Density::Density(const size_t _width, const size_t _height, const size_t _depth,
		 const float minX, const float maxX,
		 const float minY, const float maxY,
		 const float minZ, const float maxZ)
{
  assert (_width  < 10000); // Would be bad to be much larger
  assert (_height < 10000); // Would be bad to be much larger
  assert (_depth  < 10000); // Would be bad to be much larger
  assert (minX < maxX);
  assert (minY < maxY);
  assert (minZ < maxZ);
  width  = _width;
  height = _height;
  depth  = _depth;
  dx = (maxX-minX)/width;
  dy = (maxY-minY)/height;
  dz = (maxZ-minZ)/depth;
  xR[0]=minX;xR[1]=maxX;
  yR[0]=minY;yR[1]=maxY;
  zR[0]=minZ;zR[1]=maxZ;
  counts.resize(width*height*depth);
  for (size_t i=0;i<counts.size();i++) counts[i]=0;
  totalPointsInside=0;
}

bool
Density::addPoint(const float x, const float y, const float z) {
  const size_t cellNum=getCell(x,y,z);
  if (cellNum==badValue()) return (false); // Outside!!
  assert (cellNum<counts.size());
  counts[cellNum]++;
  totalPointsInside++;
  return(true);
}

void Density::printCellCounts()const {
  for (size_t i=0; i<width*height*depth;i++) {
    float x,y,z; getCellCenter(i,x,y,z);
    size_t cx,cy,cz;
    cout << i << " " << counts[i] 
	 << " " << cx << " " << cy << " " << cz 
	 << " " << x << " " << y << " " << z 
	 << endl;
  }
}

size_t
Density::getCell(const float x, const float y, const float z) const {
  if (!(xR[0] <= x && x <= xR[1])) return (badValue());  // Outside
  if (!(yR[0] <= y && y <= yR[1])) return (badValue());  // Outside
  if (!(zR[0] <= z && z <= zR[1])) return (badValue());  // Outside

  const size_t xIndex = getCellX(x);  //size_t((x-xR[0])/dx);
  const size_t yIndex = getCellY(y);  //size_t((y-yR[0])/dy);
  const size_t zIndex = getCellZ(z);  //size_t((z-zR[0])/dz);

  const size_t zOff = zIndex * getWidth() * getHeight();
  const size_t yOff = yIndex * getWidth();
  const size_t  off = xIndex + yOff + zOff;
  assert (off < 1e9);  // FIX: get rid of this contrain as someday this may be ok (64bit machines baby!)
  // wait, wasn't the dec alpha a 64bit machine in 1992?  Yet we've regressed.
  return (off);
}

void Density::getCellXYZ(const size_t index, size_t &cx, size_t &cy, size_t &cz) const {
  cz = index/(getWidth() * getHeight());
  const size_t i2=index-cz*getDepth();
  cy = i2/getWidth();
  cx = i2 - cy*getWidth();
}




void Density::getCellCenter(const size_t cellNum, float &x, float &y, float &z) const {
  size_t cx, cy, cz;  // number of cells from the origin
  getCellXYZ(cellNum,cx,cy,cz);
  x = (cx+0.5) * dx;
  y = (cy+0.5) * dy;
  z = (cz+0.5) * dz;
}

//####################################################################
// TEST CODE
//####################################################################
#ifdef REGRESSION_TEST

bool test1() {
  cout << "      test1" << endl;

  Density d(1,1,1,  0.,1.,  0.,1.,  0.,1.);
  if (0!=d.getCountInside())     {FAILED_HERE;return(false);}
  if (0!=d.getCell(0.1,0.1,0.1)) {FAILED_HERE;return(false);}
  if (0!=d.getCell(0.9,0.9,0.9)) {FAILED_HERE;return(false);}

  if (!d.addPoint(0.5,0.5,0.5)) {FAILED_HERE;return(false);}
  if (1!=d.getCountInside())    {FAILED_HERE;return(false);}

  if (d.addPoint(5,0.5,0.5)) {FAILED_HERE;return(false);}
  if (1!=d.getCountInside()) {FAILED_HERE;return(false);}
  if (d.addPoint(.5,5,0.5))  {FAILED_HERE;return(false);}
  if (1!=d.getCountInside()) {FAILED_HERE;return(false);}
  if (d.addPoint(.5,0.5,5))  {FAILED_HERE;return(false);}
  if (1!=d.getCountInside()) {FAILED_HERE;return(false);}

  if (1!=d.getWidth()) {FAILED_HERE;return(false);}
  if (1!=d.getHeight()) {FAILED_HERE;return(false);}
  if (1!=d.getDepth()) {FAILED_HERE;return(false);}

  cout << "  cell counts: " <<  endl;
  d.printCellCounts();

  return (true);
} // test1

bool test2() {
  cout << "      test2" << endl;

  Density d(2,1,1,  0.,2.,  0.,1.,  0.,1.);
  if (0!=d.getCountInside())     {FAILED_HERE;return(false);}
  d.addPoint(0.5,.1,.1);
  if (1!=d.getCellCount(0))     {FAILED_HERE;return(false);}
  if (0!=d.getCellCount(1))     {FAILED_HERE;return(false);}
  d.addPoint(1.5,.1,.1);
  if (1!=d.getCellCount(0))     {FAILED_HERE;return(false);}
  if (1!=d.getCellCount(1))     {FAILED_HERE;return(false);}

  return(true);
} // test2


int main (UNUSED int argc, char *argv[]) {
  // Put test code here
  bool ok=true;

  cout << "      Size of Density Class (in bytes): " << sizeof(Density) << endl;

  if (!test1()) {FAILED_HERE;ok=false;}
  if (!test2()) {FAILED_HERE;ok=false;}

  cout << "  " << argv[0] << " test:  " << (ok?"ok":"failed")<<endl;
  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}
#endif // REGRESSION_TEST
