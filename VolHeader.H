// $Revision$  $Author$  $Date$
#ifndef _VOL_HEADER_H_
#define _VOL_HEADER_H_

#include <vector>
#include <string>

/// \brief Header for the Vol format file
///
/// Same definition as in raw2vol.C
///
/// http://doc.coin3d.org/SIMVoleon/classSoVRVolFileReader.html
///
/// This is kind of close:
///
/// http://www.cs.purdue.edu/homes/sun/Teach/530_01S/Assignments/Data/FORMAT.TXT///
/// Stores data internally in network byte order

class  VolHeader {
public:
  /// Everything accept dimensions set to defaults
  VolHeader(const size_t width, const size_t height, const size_t depth);
  /// \brief Read a volume header from a file.
  /// \param filename File to read from
  /// \param ok Passes back \a true if file read ok
  VolHeader(const std::string filename, bool &ok);

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

private:
  // WARNING: Do NOT add or remove data members to this class!!!
  uint32_t magic_number;  ///< hton_uint32(0x0b7e7759)
  uint32_t header_length; ///< This had better be the same as sizeof(VolHeader)
  uint32_t width;         ///< Width in cells/pixels in the x direction.  How many cells wide?
  uint32_t height;        ///< Height in cells/pixels in the y direction
  uint32_t images;        ///< Depth in cells/pixels in the z direction
  uint32_t bits_per_voxel; ///< We are only going to support 8 for now!
  uint32_t index_bits;    ///< 0.  What are index bits for?
  /// Scale the model in the x-axis.  Usually 1.0
  float scaleX, scaleY, scaleZ;
  /// You could rotate this model here, but we won't!
  float rotX, rotY, rotZ;
};

#endif // _VOL_HEADER_H_
