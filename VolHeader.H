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

#ifndef _VOL_HEADER_H_
#define _VOL_HEADER_H_

#include <vector>
#include <string>

/// \brief Convert host byte order to network byte order (Big Endian)
uint32_t hton_uint32(const uint32_t value);
/// \brief Convert network byte order (Big Endian) to host byte order
uint32_t ntoh_uint32(const uint32_t value);

/// \brief Convert host byte order to network byte order (Big Endian)
float hton_float(const float value);
/// \brief Convert network byte order (Big Endian) to host byte order
float ntoh_float(const float value);


/// \brief Header for the Vol format file
///
/// Same definition as in raw2vol.C
///
/// http://doc.coin3d.org/SIMVoleon/classSoVRVolFileReader.html
///
/// This is kind of close:
///
/// http://www.cs.purdue.edu/homes/sun/Teach/530_01S/Assignments/Data/FORMAT.TXT///
/// Stores data internally in host byte order

class  VolHeader {
public:
  /// Everything accept dimensions set to defaults
  VolHeader(const size_t width, const size_t height, const size_t depth);
  VolHeader(const size_t width, const size_t height, const size_t depth,
	    const size_t bitsPerVoxel,
	    const float scaleX, const float scaleY, const float scaleZ,
	    const float rotX,const float rotY, const float rotZ);

  /// \brief Read a volume header from a file.
  /// \param filename File to read from
  /// \param ok Passes back \a true if file read ok
  VolHeader(const std::string filename, bool &ok);

  /// \brief Use fwrite to put out the header
  /// \return Returns the number of bytes written
  size_t write(FILE *outFile);

  //size_t getSize() const {return(sizeof(VolHeader));}

  uint32_t getMagicNumber() const; ///< Return value in host byte order
  uint32_t getHeaderLength() const; ///< Return value in host byte order
  uint32_t getWidth() const; ///< Return in host byte order
  uint32_t getHeight() const; ///< Return in host byte order
  uint32_t getImages() const; ///< Return in host byte order
  uint32_t getBitsPerVoxel() const; ///< Return in host byte order
  uint32_t getIndexBits() const; ///< Return in host byte order

  float getScaleX() const; ///< Return in host byte order
  float getScaleY() const; ///< Return in host byte order
  float getScaleZ() const; ///< Return in host byte order

  float getRotX() const; ///< Return in host byte order
  float getRotY() const; ///< Return in host byte order
  float getRotZ() const; ///< Return in host byte order

  /// This is the expected size of the header.
  static size_t requiredSize() {assert(52==sizeof(VolHeader));return(sizeof(VolHeader));}
  /// Host byte order magic number
  static uint32_t hMagicNum() {return(0x0b7e7759);}
  /// Network byte order magic number
  static uint32_t nMagicNum() {return(hton_uint32(hMagicNum()));}

private:
  uint32_t magic_number;  ///< hton_uint32(0x0b7e7759)
  uint32_t header_length; ///< This had better be the same as sizeof(VolHeader)
  uint32_t width;         ///< Width in cells/pixels in the x direction.  How many cells wide?
  uint32_t height;        ///< Height in cells/pixels in the y direction
  uint32_t images;        ///< Depth in cells/pixels in the z direction
  uint32_t bits_per_voxel; ///< We are only going to support 8 for now!
  /// \bug Do not know what to do with index bits
  uint32_t index_bits;    ///< 0.  What are index bits for?
  /// Scale the model in the x-axis.  Usually 1.0
  float scaleX, scaleY, scaleZ;
  /// You could rotate this model here
  float rotX, rotY, rotZ;
};

#endif // _VOL_HEADER_H_
