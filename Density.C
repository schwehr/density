// $Revision$  $Author$  $Date$

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
#include "VolHeader.H"
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


Density::Density() {
  cout << "FIX: make sure that later, things get setup ok.  Density()" << endl;
}

void
Density::resize(const size_t _width, const size_t _height, const size_t _depth,
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
  counts.resize(width*height*depth,0); // set all to zero
  //for (size_t i=0;i<counts.size();i++) counts[i]=0;
  totalPointsInside=0;
  return;
} // resize()

Density::Density(const size_t _width, const size_t _height, const size_t _depth,
		 const float minX, const float maxX,
		 const float minY, const float maxY,
		 const float minZ, const float maxZ)
{
  resize(_width, _height, _depth,  minX,maxX,   minY,maxY,   minZ,maxZ);
}

#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <term.h>
#include <sys/select.h>
#include <unistd.h>  // Select 

#include <sys/mman.h>	// mmap
#include <sys/types.h>
#include <sys/stat.h>

/// @param pointer to the next data item
/// @param bitsPerVoxel yada yada... NOT bytes
/// @param val Returns value read here
/// @param readOK bool set to false if trouble
/// @return Pointer to the next data item or 0 if an error
/// @bug Would be nice to handle 1, 2, and 4 bit voxels for huge grids
/// Just need to add a new arg of the bit index
char *ReadDataUnsigned(char *data, const size_t bitsPerVoxel, size_t &val, bool &ok) {
  assert(data);
#ifndef NDEBUG
  if (0==(bitsPerVoxel/8)) {ok=false;return (0);}
  if (4<(bitsPerVoxel/8)) {ok=false;return (0);}
  if (0!=(bitsPerVoxel%8)) {ok=false;return (0);}
#endif

  switch (bitsPerVoxel) {
  case  8: {uint8_t  *uPtr=(uint8_t *)data; val = *uPtr;} break;
  case 16: {uint16_t *uPtr=(uint16_t *)data; val = *uPtr;} break;
  case 32: {uint32_t *uPtr=(uint32_t *)data; val = *uPtr;} break;
  default:
    assert(false && "Bailey yells when you step on his tail");
  }

  return (data+(bitsPerVoxel/8));
}


Density::Density(const std::string &filename, bool &ok) {
  ok=true;  // Be optimistic that we will kick butt
  bool r; // temp result code
  VolHeader v(filename,r);
  if (!r) {
    ok=false;
    cerr << "Density could not load the header."<< endl
	 << "  What kind of crap are you trying to feed me?" << endl;
    return;
  }

  // Validate file size
  struct stat sb;
  {
    int r = stat (filename.c_str(), &sb);
    if (0 != r) {perror("stat to get file size FAILED");ok=false; return; }
  }
  const size_t numCells = v.getWidth()*v.getHeight()*v.getImages();
  const size_t bitsPerVoxel = v.getBitsPerVoxel();
  if (bitsPerVoxel%8 != 0) {cerr << "ERROR: Can NOT handle non-byte-aligned data!"<<endl;ok=false;return;}
  if ((v.getHeaderLength() + numCells*(bitsPerVoxel/8)) != sb.st_size)
    {FAILED_HERE;cerr << "ERROR: File size invalid!!"<<endl;ok=false;return;}


  cout << "FIX: use the scale to set the x,y, and z ranges" << endl;
  resize(v.getWidth(),v.getHeight(),v.getImages(), -0.5,0.5, -0.5,0.5, -0.5,0.5);

  int fd = open (filename.c_str(), O_RDONLY, 0);
  if (-1==fd) { perror ("open failed"); return ;  }
  char *file = (char *)mmap (0, sb.st_size, PROT_READ,  MAP_FILE, fd, 0);
  if ((char *)(-1)==file) { perror ("mmap FAILED");  return ;  }
  close(fd); // Close does not munmap

  char *next=file+v.getHeaderLength();
  for (size_t i=0;i<numCells && next ;i++) {
    bool readOK;
    size_t val;
    next = ReadDataUnsigned(next, v.getBitsPerVoxel(), val, readOK);
    if (!readOK) {cerr<<"Data read error!" << endl;ok=false; break;}
    addPoints(i,val);
  }


  {int r = munmap(file,sb.st_size); if (-1==r) {perror ("munmap failed");ok=false; return;}}
  return; // ok is the return code
} // Density - load from a file



bool
Density::addPoint(const float x, const float y, const float z) {
  const size_t cellNum=getCell(x,y,z);
  if (cellNum==badValue()) {
#ifdef REGRESSION_TEST
    cout << "         Point outside volume: " << x << " " << y << " " << z << endl;
#endif    
    return (false); // Outside!!
  }
  assert (cellNum<counts.size());
  counts[cellNum]++;
  totalPointsInside++;
  return(true);
}

void Density::printCellCounts()const {
  cout << "# " << endl
<< "# i cx cy cz counts x y z " << endl
<< "# " << endl
<< "# " << endl
    ;
  for (size_t i=0; i<width*height*depth;i++) {
    float x,y,z; getCellCenter(i,x,y,z);
    size_t cx,cy,cz;
    getCellXYZ(i,cx,cy,cz);
    cout << i << "   " << counts[i] 
	 << "   " << cx << " " << cy << " " << cz 
	 << "   " << x << " " << y << " " << z 
	 << endl;
  }
}

size_t Density::getCellFromWHD(const size_t xIndex, const size_t yIndex, const size_t zIndex) const {
  const size_t zOff = zIndex * getWidth() * getHeight();
  const size_t yOff = yIndex * getWidth();
  const size_t  off = xIndex + yOff + zOff;
  if (!isValidCell(off)) {
    cerr << "WARNING: getCellFromWHD out of bounds" << endl;
    FAILED_HERE;
    return (badValue());
  }
  return (off);
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
  assert (isValidCell(index));
  assert (index<1000000);
  cz = index/(getWidth() * getHeight());
  const size_t i2=index-cz*(getWidth() * getHeight());
  assert (i2<100000);
  cy = i2/getWidth();
  cx = i2 - cy*getWidth();
  //cout << "getCellXYZ:  " << index << " " << cx << " " << cy << " " << cz << endl;
  assert (cx<100000);
  assert (cy<100000);
  assert (cz<100000);
}


size_t Density::getCellNeighbor(const size_t i, NeighborEnum which) const {
  assert (isValidCell(i));
  size_t cx,cy,cz;
  getCellXYZ (i,cx,cy,cz);

  size_t cxNew=cx,cyNew=cy,czNew=cz;
  switch (which) {
  case LEFT:  cxNew = cx - 1;  if (cxNew>cx)             return (badValue());  break;
  case RIGHT: cxNew = cx + 1;  if (getWidth() <= cxNew)  return (badValue());  break;
  case FRONT: cyNew = cy - 1;  if (cyNew>cy)             return (badValue());  break;
  case BACK:  cyNew = cy + 1;  if (getHeight() <= cyNew) return (badValue());  break;
  case BELOW: czNew = cz - 1;  if (czNew>cz)             return (badValue());  break;
  case ABOVE: czNew = cz + 1;  if (getDepth() <= czNew)  return (badValue());  break;
  default:
    assert(false);
  }
  
  return (getCellFromWHD(cxNew,cyNew,czNew));
} // getCellNeighbor


void Density::getCellCenter(const size_t cellNum, float &x, float &y, float &z) const {
  assert(isValidCell(cellNum));
  size_t cx, cy, cz;  // number of cells from the origin
  getCellXYZ(cellNum,cx,cy,cz);
  //cout << "cIndex: " << cx << " " <<cy<<" "<<cz<<endl;
  assert (cx<1000000);
  assert (cy<1000000);
  assert (cz<1000000);
  x = xR[0] + (cx+0.5) * dx;
  y = yR[0] + (cy+0.5) * dy;
  z = zR[0] + (cz+0.5) * dz;
}

bool Density::writeVol(const string &filename) {
  FILE *o=fopen(filename.c_str(),"wb");
  if (!o) {perror("failed to open output file");cerr << "   " << filename << endl;return(false);}

  {
    VolHeader hdr(getWidth(),getHeight(),getDepth());
    hdr.write(o);
  }

  const size_t min=getMinCount();
  const size_t max=getMaxCount();

  // http://doc.coin3d.org/SIMVoleon/classSoVolumeData.html#a1
  for (size_t i=0;i<counts.size();i++) {
    // FIX: how do we fit the data?
    const unsigned char data=scaleCount(i,min,max);
#ifdef REGRESSION_TEST
    cout << "         writing: " << i<<"("<<counts[i]<<")  -> " << int(data)
	 << "    (" << min <<","<<max<<")"<<endl; 
#endif

    // No endian issue with 1 byte data
    if (1!=fwrite(&data,sizeof(data),1,o)) {perror("write failed");fclose(o);return(false);}
  }

  if (0!=fclose(o)) {perror("close failed... bizarre");return(false);}
  return (true);
}

unsigned char Density::scaleCount(const size_t i, const size_t min, const size_t max) const {
  const float _0to1 = float(counts[i]-min)/(max-min);
  const unsigned char r = (unsigned char)(_0to1 * std::numeric_limits<unsigned char>::max());
  return (r);
}

size_t Density::getMaxCount() const {
  size_t max = std::numeric_limits<size_t>::min();
  for (size_t i=0;i<counts.size();i++) if (counts[i]>max) max=counts[i];
  return (max);
}
size_t Density::getMinCount() const {
  size_t min = std::numeric_limits<size_t>::max();
  for (size_t i=0;i<counts.size();i++) if (counts[i]<min) min=counts[i];
  return(min);
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

#if 0
  {
    cout << "Danger: "<< endl;
    Density dx(10,1,1,  -5,5.,  0.,1.,  0.,1.);
    for (size_t i=0;i<10;i++) {
      float x,y,z;
      dx.getCellCenter(i,x,y,z);
      cout << " " << x << " " << y << " " << z << endl;
    }
  }
#endif
#if 0
  {
    cout << "Danger: Y"<< endl;
    Density dy(1,10,1,  0,1.,  -5.,5.,  0.,1.);
    for (size_t i=0;i<10;i++) {
      float x,y,z;
      dy.getCellCenter(i,x,y,z);
      cout << " Y: " << x << " " << y << " " << z << endl;
    }
  }
#endif

#if 0
  {
    cout << "Danger: Z"<< endl;
    Density dy(1,1,10,  0,1.,  0.,1.,  -5.,5.);
    for (size_t i=0;i<10;i++) {
      float x,y,z;
      dy.getCellCenter(i,x,y,z);
      cout << " Y: " << x << " " << y << " " << z << endl;
    }
  }
#endif

  return(true);
} // test2



bool test3() {
  cout << "      test3" << endl;
  const string filename("test3.vol");
  {
    // Test writing a file
    Density d(2,2,2,  0.,2.,  0.,2.,  0.,2.);
    d.addPoint(0.1,0.1,0.1);  d.addPoint(0.1,0.1,0.1);
    d.addPoint(1.5,0.1,0.1);  d.addPoint(0.1,1.5,0.1);
    for (size_t i=0;i<10;i++) d.addPoint(1.5,1.5,1.5);

    if (!d.writeVol(filename)) {FAILED_HERE;return(false);}
  }
  {
    // FIX: Would be better to read someone elses file.
    bool result;
    Density d(filename,result);
    if (!result) {FAILED_HERE;return(FALSE);}
    if (!d.writeVol(filename+string("2"))) {FAILED_HERE;return(false);}
  }


  return(true);
}


bool test4() {
  bool ok=true;
  {
    // This is a pretty small/simple test case
    Density d;
    d.resize(1,2,3, 0.,2., 0.,2., 0.,2.);

    if (1!=d.getWidth()) {ok=false;FAILED_HERE;}
    if (2!=d.getHeight()) {ok=false;FAILED_HERE;}
    if (3!=d.getDepth()) {ok=false;FAILED_HERE;}
    if (6!=d.getSize()) {ok=false;FAILED_HERE;}

    if (0!=d.getCellFromWHD(0,0,0))  {ok=false; FAILED_HERE;}
    if (1!=d.getCellFromWHD(0,1,0)) {ok=false;FAILED_HERE;}

    if (Density::badValue()!=d.getCellNeighbor(0,Density::LEFT)) {ok=false;FAILED_HERE;}
    if (Density::badValue()!=d.getCellNeighbor(0,Density::RIGHT)) {ok=false;FAILED_HERE;}

    if (Density::badValue()!=d.getCellNeighbor(0,Density::FRONT)) {ok=false;FAILED_HERE;}
    if (1!=d.getCellNeighbor(0,Density::BACK)) {ok=false;FAILED_HERE;}

    if (Density::badValue()!=d.getCellNeighbor(0,Density::BELOW)) {ok=false;FAILED_HERE;}
    if (2!=d.getCellNeighbor(0,Density::ABOVE)) {ok=false;FAILED_HERE;}
  }

  //Density(string("filename")); //FIX: Implement and test
  return(ok);
}

int main (UNUSED int argc, char *argv[]) {
  // Put test code here
  bool ok=true;

  cout << "      Size of Density   (in bytes): " << sizeof(Density) << endl;
  cout << "      Size of VolHeader (in bytes): " << sizeof(VolHeader) << endl;

  if (52 != sizeof(VolHeader)) {FAILED_HERE;ok=false;}

  cout << "      Size of float: " << sizeof(1.f) << endl;
  cout << "      Size of double: " << sizeof(1.) << endl;
  if (4 != sizeof(1.f)) {FAILED_HERE;ok=false;}
  if (8 != sizeof(1.)) {FAILED_HERE;ok=false;}

#ifdef BIGENDIAN
  if (0x00010203!=hton_uint32(0x00010203)) {FAILED_HERE;ok=false;}
#endif

  if (4!=sizeof(float)) {FAILED_HERE;ok=false;} // Must be 4 for vol_header
  if (4!=sizeof(uint32_t)) {FAILED_HERE;ok=false;} // Must be 4 for vol_header

  if (!test1()) {FAILED_HERE;ok=false;}
  if (!test2()) {FAILED_HERE;ok=false;}
  if (!test3()) {FAILED_HERE;ok=false;} // test writing
  if (!test4()) {FAILED_HERE;ok=false;}

  cout << "  " << argv[0] << " test:  " << (ok?"ok":"failed")<<endl;
  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}
#endif // REGRESSION_TEST
