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
/// \brief Volume Header class for SimVoleon formated voxel data


/***************************************************************************
 * INCLUDES
 ***************************************************************************/

#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <term.h>
#include <sys/select.h>
#include <unistd.h>  // Select 
#include <sys/mman.h>	// mmap
#include <sys/types.h>
#include <sys/stat.h>

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
 * MACROS, DEFINES, GLOBALS
 ***************************************************************************/

#include "debug.H" // provides FAILED_HERE, UNUSED, DebugPrintf

/// Let the debugger find out which version is being used.
static const UNUSED char* RCSid ="@(#) $Id$";


//####################################################################
// VOLHEADER METHODS
//####################################################################

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
} 

float
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
#else
#  error UNKNOWN ENDIAN TYPE!
#endif
}


VolHeader::VolHeader(const size_t _width, const size_t _height, const size_t depth)
{
  magic_number=hMagicNum();
  header_length=requiredSize();
  assert(52==header_length);  // Can't handle header with extra padding yet.

  width =_width;
  height=_height;
  images=depth;

  bits_per_voxel=8;
  index_bits=0;
  scaleX=scaleY=scaleZ=1.f;
  rotX=rotY=rotZ=1.f;
}

VolHeader::VolHeader(const size_t _width, const size_t _height, const size_t depth,
	    const size_t _bitsPerVoxel,
	    const float _scaleX, const float _scaleY, const float _scaleZ,
	    const float _rotX, const float _rotY, const float _rotZ)
{
  magic_number=hMagicNum();
  header_length=requiredSize();
  assert(52==header_length);

  width =_width;
  height=_height;
  images=depth;

  bits_per_voxel=_bitsPerVoxel;
  index_bits=0; // Grrr... what is this?

  scaleX=_scaleX;  scaleY=_scaleY;  scaleZ=_scaleZ;
  rotX=_rotX;  rotY=_rotY;  rotZ=_rotZ;
}

// It is not okay to talk to a this class if it returns false
VolHeader::VolHeader(const std::string filename, bool &ok) {
  ok=true;
  struct stat sb;
  int r = stat (filename.c_str(), &sb);
  if (0 != r) {
    perror("stat to get file size FAILED");
    ok=false; return;
  }

  if (8> sb.st_size) {
    cerr << "File too small to be a valid volume/voxel file" << endl;
    ok=false;
    return;
  }

  int fd = open (filename.c_str(), O_RDONLY, 0);
  if (-1==fd) {
    perror ("open failed");
    return ;
  }
  char *file = (char *)mmap (0, sb.st_size, PROT_READ,  MAP_FILE, fd, 0);
  if ((char *)(-1)==file) {
    perror ("mmap FAILED");
    return ;
  }
  close(fd); // Close does not munmap

  char *next=file;

  uint32_t *mnPtr  = (uint32_t *)(next); next+=sizeof(uint32_t);
  uint32_t *hlPtr  = (uint32_t *)(next); next+=sizeof(uint32_t);
  uint32_t *wPtr   = (uint32_t *)(next); next+=sizeof(uint32_t);
  uint32_t *hPtr   = (uint32_t *)(next); next+=sizeof(uint32_t);
  uint32_t *iPtr   = (uint32_t *)(next); next+=sizeof(uint32_t);
  uint32_t *bpvPtr = (uint32_t *)(next); next+=sizeof(uint32_t);
  uint32_t *ibPtr  = (uint32_t *)(next); next+=sizeof(uint32_t);

  float *sxPtr = (float *)(next); next+=sizeof(float);
  float *syPtr = (float *)(next); next+=sizeof(float);
  float *szPtr = (float *)(next); next+=sizeof(float);

  float *rxPtr = (float *)(next); next+=sizeof(float);
  float *ryPtr = (float *)(next); next+=sizeof(float);
  float *rzPtr = (float *)(next); next+=sizeof(float);

  if (nMagicNum() != *mnPtr) {ok=false;cerr<<"bad magic number"<<endl;}
  if (requiredSize() != ntoh_uint32(*hlPtr)) {ok=false;cerr<<"bad header size"<<endl;}

  magic_number = ntoh_uint32(*mnPtr);
  header_length = ntoh_uint32(*hlPtr);

  width = ntoh_uint32(*wPtr);
  height = ntoh_uint32(*hPtr);
  images = ntoh_uint32(*iPtr);
  bits_per_voxel = ntoh_uint32(*bpvPtr);
  index_bits = ntoh_uint32(*ibPtr);

  scaleX = ntoh_float(*sxPtr);
  scaleY = ntoh_float(*syPtr);
  scaleZ = ntoh_float(*szPtr);

  rotX = ntoh_float(*rxPtr);
  rotY = ntoh_float(*ryPtr);
  rotZ = ntoh_float(*rzPtr);

  {
    int r = munmap(file,sb.st_size);
    if (-1==r) {perror ("munmap failed");ok=false; return;}
  }
  return;
} // VolHeader(filename)

size_t VolHeader::write(FILE *o) {
  assert (o);   if (!o) return(0);
  size_t bytes=0;
  size_t r;  // Number of bytes for each write

  {
    uint32_t uBuf;
    uBuf = hton_uint32(getMagicNumber()); bytes += (r=fwrite(&uBuf,1,sizeof(uint32_t),o));
    if (sizeof(uint32_t)!=r) {perror("header write trouble"); return(bytes);}
    uBuf = hton_uint32(getHeaderLength()); bytes += (r=fwrite(&uBuf,1,sizeof(uint32_t),o));
    if (sizeof(uint32_t)!=r) {perror("header write trouble"); return(bytes);}

    uBuf = hton_uint32(getWidth()); bytes += (r=fwrite(&uBuf,1,sizeof(uint32_t),o));
    if (sizeof(uint32_t)!=r) {perror("header write trouble"); return(bytes);}
    uBuf = hton_uint32(getHeight()); bytes += (r=fwrite(&uBuf,1,sizeof(uint32_t),o));
    if (sizeof(uint32_t)!=r) {perror("header write trouble"); return(bytes);}
    uBuf = hton_uint32(getImages()); bytes += (r=fwrite(&uBuf,1,sizeof(uint32_t),o));
    if (sizeof(uint32_t)!=r) {perror("header write trouble"); return(bytes);}
    uBuf = hton_uint32(getBitsPerVoxel()); bytes += (r=fwrite(&uBuf,1,sizeof(uint32_t),o));
    if (sizeof(uint32_t)!=r) {perror("header write trouble"); return(bytes);}
    uBuf = hton_uint32(getIndexBits()); bytes += (r=fwrite(&uBuf,1,sizeof(uint32_t),o));
    if (sizeof(uint32_t)!=r) {perror("header write trouble"); return(bytes);}
  }
  {
    float fBuf;
    fBuf = hton_float(getScaleX()); bytes += (r=fwrite(&fBuf,1,sizeof(float),o));
    if (sizeof(float)!=r) {perror("header write trouble"); return(bytes);}
    fBuf = hton_float(getScaleY()); bytes += (r=fwrite(&fBuf,1,sizeof(float),o));
    if (sizeof(float)!=r) {perror("header write trouble"); return(bytes);}
    fBuf = hton_float(getScaleZ()); bytes += (r=fwrite(&fBuf,1,sizeof(float),o));
    if (sizeof(float)!=r) {perror("header write trouble"); return(bytes);}
    fBuf = hton_float(getRotX()); bytes += (r=fwrite(&fBuf,1,sizeof(float),o));
    if (sizeof(float)!=r) {perror("header write trouble"); return(bytes);}
    fBuf = hton_float(getRotY()); bytes += (r=fwrite(&fBuf,1,sizeof(float),o));
    if (sizeof(float)!=r) {perror("header write trouble"); return(bytes);}
    fBuf = hton_float(getRotZ()); bytes += (r=fwrite(&fBuf,1,sizeof(float),o));
    if (sizeof(float)!=r) {perror("header write trouble"); return(bytes);}
  }

  assert(52==getHeaderLength());  // FIX: add padding?? if header is longer than 52?

  return(bytes);
}

uint32_t VolHeader::getMagicNumber() const { return (magic_number); }
uint32_t VolHeader::getHeaderLength() const { return (header_length); }
uint32_t VolHeader::getWidth() const { return (width); }
uint32_t VolHeader::getHeight() const { return (height); }
uint32_t VolHeader::getImages() const { return (images); }
uint32_t VolHeader::getBitsPerVoxel() const { return (bits_per_voxel); }
uint32_t VolHeader::getIndexBits() const { return (index_bits); }

float VolHeader::getScaleX() const { return (scaleX); }
float VolHeader::getScaleY() const { return (scaleY); }
float VolHeader::getScaleZ() const { return (scaleZ); }
float VolHeader::getRotX() const { return (rotX); }
float VolHeader::getRotY() const { return (rotY); }
float VolHeader::getRotZ() const { return (rotZ); }


size_t VolHeader::getDataSize() const {
  // FIX: is it bytesPerCell = (bits_per_voxel+index_bits)/8 ??
  assert(0==(bits_per_voxel%8));  // Can NOT handle non byte aligned data yet
  assert(0==index_bits); // Can not handle these yet
  return ((bits_per_voxel/8)*width*height*images);
}



//####################################################################
// TEST CODE
//####################################################################
#ifdef REGRESSION_TEST

bool test1() {
  bool ok=true;
  cout << "      test1" << endl;

  {
    VolHeader v(10,10,10);
  }

  return (ok);
} // test1


bool test2() {
  bool ok=true;
  cout << "      test2" << endl;

  bool okLoad;
  VolHeader v(string("test3.vol"),okLoad);
  if (!okLoad) {ok=false; cerr << "failed to load test3.vol header"<<endl;}

  return (ok);
} // test2

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
  if (!test2()) {FAILED_HERE;ok=false;}

  cout << "  " << argv[0] << " test:  " << (ok?"ok":"failed")<<endl;
  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}
#endif // REGRESSION_TEST
