// $Revision$  $Author$  $Date$
#ifndef _DENSITY_H_
#define _DENSITY_H_

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
/// http://www.cs.purdue.edu/homes/sun/Teach/530_01S/Assignments/Data/FORMAT.TXT

class  VolHeader {
public:
  /// Everything accept dimensions set to defaults
  VolHeader(const size_t width, const size_t height, const size_t depth);
  //size_t getSize() const {return(sizeof(VolHeader));}
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


/// \brief Voxel density handling class.  How many points per cell.
///
///        Points loose their actual xyz when added to the grid
///
/*! What do we can this space representation?
    - \a bvd - Blob of a Volume Distribution/Density
    - \a 3Dpdf - probability density functione
    - \a vdf - Volume Density Function
*/

class Density {
public:
  /// \brief Contructor that does almost nothing.  FIX: Can we get rid of this?
  /// \bug Do not want this constructor to exist!
  /// If just this is called, life is bad!  Does not setup anything really.
  Density();

  /// \brief Create a voxel space of widht,height, and depth number of cells.
  /// @param width Number of cells wide/x
  /// @param height Number of cells deep/y
  /// @param depth Number of cells tall/z
  /// @param minX, maxX 3D space x range of voxel volume.
  /// @param minY, maxY 3D space x range of voxel volume.
  /// @param minZ, maxZ 3D space x range of voxel volume.
  Density(const size_t width, const size_t height, const size_t depth,
	  const float minX, const float maxX,
	  const float minY, const float maxY,
	  const float minZ, const float maxZ
	  );

  /// Load a vol file from disk
  Density(const std::string &filename) {assert(false);}  // FIX: Implement

  /// \brief Change the volumes size.  Dumps all counts. Does not shrink memory footprint if smaller
  /// @param width Number of cells wide/x
  /// @param height Number of cells deep/y
  /// @param depth Number of cells tall/z
  /// @param minX, maxX 3D space x range of voxel volume.
  /// @param minY, maxY 3D space x range of voxel volume.
  /// @param minZ, maxZ 3D space x range of voxel volume.
  void resize(const size_t width, const size_t height, const size_t depth,
	      const float minX, const float maxX,
	      const float minY, const float maxY,
	      const float minZ, const float maxZ
	      );


  /// Add a point into the voxel structure.  Figures out which cell for you
  /// \return \a true if inside the bounding box, \a false if outside and unrecorded
  bool addPoint(const float x, const float y, const float z);
  /// Print to stdout the cell number and the count
  void printCellCounts() const;
  size_t getWidth()  const {return (width);} ///< num of cells wide
  size_t getHeight() const {return (height);} ///< num of cells tall
  size_t getDepth()  const {return (depth);} ///< num of cell front to back
  size_t getSize() const {return (counts.size());} ///< How many voxels in this density space?
  /// How many points so far have been added that actually fall in the voxels' volumes
  size_t getCountInside() const {return(totalPointsInside);}

  /// \brief Which cell number a point in space goes to.
  /// \return Cell number or \a badValue if x,y,z is not in the volume
  size_t getCell(const float x, const float y, const float z) const;
  bool   isValidCell(const size_t i) const {return(i<counts.size()?true:false);}
  size_t getCellX(const float x) const {return(size_t((x-xR[0])/dx));}
  size_t getCellY(const float y) const {return(size_t((y-yR[0])/dy));}
  size_t getCellZ(const float z) const {return(size_t((z-zR[0])/dz));}
  void   getCellXYZ(const size_t index, size_t &cx, size_t &cy, size_t &cz) const;
  size_t getCellFromWHD(const size_t xIndex, const size_t yIndex, const size_t zIndex) const;


  /// x less, x more, y less, y more, z less, z more
  enum NeighborEnum {LEFT,RIGHT,FRONT,BACK,BELOW,ABOVE,NUM_NEIGHBORS=6};
  /// returns \a badValue if the neighbor is outside of the volume
  size_t getCellNeighbor(const size_t i, NeighborEnum which) const; 

  /// Given a cell index number, what is the location of the center?
  void getCellCenter(const size_t cellNum, float &x, float &y, float &z) const;

  /// How many points have been added to a cell?
  size_t getCellCount(size_t i) const{assert(isValidCell(i));return(counts[i]);}


  float getDX() const {return(dx);}  ///< How wide is one cell
  float getDY() const {return(dy);}  ///< How heigh is one cell
  float getDZ() const {return(dz);}  ///< How deep is one cell

  /// Write out a vol formatted file for SIM Voleon
  /// \return \a true on success or \a false if there was an error
  bool writeVol(const std::string &filename);

  /// This is the value for an unknown or bad entry.  Should be a huge number that we would never encounter.
  static size_t badValue() {return(std::numeric_limits<size_t>::max());}

  /// Return a number scaled between 0 and 255
  unsigned char scaleCount(const size_t i, const size_t min, const size_t max) const;
  size_t getMaxCount() const; ///< What is the maximum count in one cell across the whole count vector?
  size_t getMinCount() const; ///< What is the minimum count in one cell across the whole count vector?


#ifndef REGRESSION_TEST
private:
#endif
  //size_t cellSize[3]; 
  size_t width;  ///< width in number of cells
  size_t height; ///< height in number of cells
  size_t depth;  ///< depth in number of cells

  float dx; ///< Cell size
  float dy; ///< Cell size
  float dz; ///< Cell size

  float xR[2]; ///< min,max x Range of the voxel bounding box
  float yR[2]; ///< min,max y Range of the voxel bounding box
  float zR[2]; ///< min,max z Range of the voxel bounding box

  std::vector<size_t> counts; ///< number of points in each voxel
  size_t totalPointsInside; ///< Does not include points that were outside!
  
};
#endif // _DENSITY_H_
