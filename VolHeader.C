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
// VOLHEADER METHODS
//####################################################################

/// \brief Convert network byte order (Big Endian) to host byte order
uint32_t  
ntoh_uint32(const uint32_t value) {
#ifdef BIGENDIAN
  return(value);
#elif LITTLEENDIAN
  return(hton_uint32(value));
#else
#  error UNKOWN ENDIAN TYPE
#endif
}

/// \brief Convert network byte order (Big Endian) to host byte order
float
ntoh_float(const float value) {
#ifdef BIGENDIAN
  return(value);
#elif LITTLEENDIAN
  return(hton_float(value));
#else
#  error UNKOWN ENDIAN TYPE
#endif
}

/// \brief Convert host byte order to network byte order (Big Endian)
uint32_t  
hton_uint32(const uint32_t value)
{
#ifdef BIGENDIAN
  return(value);  // NOP!  Woo hoo!
#elif LITTLEENDIAN
  cout << "FIX WARNING: LITTLEENDIAN is not yet tested!" << endl;
  uint32_t tmp;
  const char *t1=(char *) &value;
  char *t2=(char *) &tmp;
  t2[0]=t1[3];
  t2[1]=t1[2];
  t2[2]=t1[1];
  t2[3]=t1[0];
  return(tmp);
#else
#  error UNKNOWN ENDIAN TYPE!
#endif
  // Cool idea:  assert(0 && "message to go with an assert");
} 

static float
hton_float(const float value)
{
#ifdef BIGENDIAN
  return(value);  // NOP!  Woo hoo!
#elif LITTLEENDIAN
  cout << "FIX WARNING: LITTLEENDIAN is not yet tested!" << endl;
  float tmp;
  const char *t1=(char *) &value;
  char *t2=(char *) &tmp;
  t2[0]=t1[3];
  t2[1]=t1[2];
  t2[2]=t1[1];
  t2[3]=t1[0];
  return(tmp);
#elsex
#  error UNKNOWN ENDIAN TYPE!
#endif
}


VolHeader::VolHeader(const size_t _width, const size_t _height, const size_t depth)
{
  magic_number=hton_uint32(0x0b7e7759);
  header_length=hton_uint32(sizeof(VolHeader));
  assert(52==header_length);

  width=hton_uint32(uint32_t(_width));
  height=hton_uint32(uint32_t(_height));
  images=hton_uint32(uint32_t(depth));

  bits_per_voxel=hton_uint32(8);
  index_bits=0;  // 0==hton_uint32(0) no matter what
  scaleX=scaleY=scaleZ=(hton_float(1.f));
  rotX=rotY=rotZ=(hton_float(1.f));
}

VolHeader::VolHeader(const std::string filename, bool &ok) {
  ok=true;
  assert (false);
}

uint32_t VolHeader::getMagicNumber() const { return (ntoh_uint32(magic_number)); }
uint32_t VolHeader::getHeaderLength() const { return (ntoh_uint32(header_length)); }
uint32_t VolHeader::getWidth() const { return (ntoh_uint32(width)); }
uint32_t VolHeader::getHeight() const { return (ntoh_uint32(height)); }
uint32_t VolHeader::getImages() const { return (ntoh_uint32(images)); }
uint32_t VolHeader::getBitsPerVoxel() const { return (ntoh_uint32(bits_per_voxel)); }
uint32_t VolHeader::getIndexBits() const { return (ntoh_uint32(index_bits)); }

float VolHeader::getScaleX() const { return (ntoh_float(scaleX)); }
float VolHeader::getScaleY() const { return (ntoh_float(scaleY)); }
float VolHeader::getScaleZ() const { return (ntoh_float(scaleZ)); }
float VolHeader::getRotX() const { return (ntoh_float(rotX)); }
float VolHeader::getRotY() const { return (ntoh_float(rotY)); }
float VolHeader::getRotZ() const { return (ntoh_float(rotZ)); }



//####################################################################
// TEST CODE
//####################################################################
#ifdef REGRESSION_TEST

bool test1() {
  cout << "      test1" << endl;

  VolHeader v(10,10,10);

  return (true);
} // test1

int main (UNUSED int argc, char *argv[]) {
  // Put test code here
  bool ok=true;

  cout << "      Size of VolHeader (in bytes): " << sizeof(VolHeader) << endl;
  if (52 != sizeof(VolHeader)) {FAILED_HERE;ok=false;}

  cout << "      Size of float: " << sizeof(1.f) << endl;
  if (4 != sizeof(1.f)) {FAILED_HERE;ok=false;}
  cout << "      Size of double: " << sizeof(1.) << endl;
  if (8 != sizeof(1.)) {FAILED_HERE;ok=false;}

#ifdef BIGENDIAN
  if (0x00010203!=hton_uint32(0x00010203)) {FAILED_HERE;ok=false;}
#endif

  if (4!=sizeof(float)) {FAILED_HERE;ok=false;} // Must be 4 for vol_header
  if (4!=sizeof(uint32_t)) {FAILED_HERE;ok=false;} // Must be 4 for vol_header

  if (!test1()) {FAILED_HERE;ok=false;}

  cout << "  " << argv[0] << " test:  " << (ok?"ok":"failed")<<endl;
  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}
#endif // REGRESSION_TEST
