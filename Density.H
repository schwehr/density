/* $Revision$  $Author$  $Date$ */
#ifndef _DENSITY_H_
#define _DENSITY_H_

#include <vector>

/// \brief Voxel density handling class.  How many points per cell.

///        Points loose their actual xyz when added to the grid

class Density {
public:
  Density(const size_t width, const size_t height, const size_t depth,
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
  size_t getCountInside() const {return(totalPointsInside);}

  /// \brief Which cell number a point in space goes to.
  /// \return Cell number or \a badValue if x,y,z is not in the volume
  size_t getCell(const float x, const float y, const float z) const;
  bool isValidCell(size_t i) const {return(i<counts.size()?true:false);}
  size_t getCellX(const float x) const {return(size_t((x-xR[0]/dx)));}
  size_t getCellY(const float y) const {return(size_t((y-yR[0]/dy)));}
  size_t getCellZ(const float z) const {return(size_t((z-zR[0]/dz)));}
  void getCellXYZ(const size_t index, size_t &cx, size_t &cy, size_t &cz) const;

  void getCellCenter(const size_t cellNum, float &x, float &y, float &z) const;

  size_t getCellCount(size_t i) const{assert(isValidCell(i));return(counts[i]);}


  /// This is the value for an unknown or bad entry.  Should be a huge number that we would never encounter.
  static size_t badValue() {return(std::numeric_limits<size_t>::max());}

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
