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


private:
  /// \brief Which cell number a point in space goes to.
  /// \return Cell number or \a badValue if x,y,z is not in the volume
  size_t getCell(const float x, const float y, const float z);

  /// This is the value for an unknown or bad entry.  Should be a huge number that we would never encounter.
  static size_t badValue() {return(std::numeric_limits<size_t>::max());}

  size_t cellSize[3]; /// width, height, depth count of cells
  float xRange[2]; /// min,max of the voxel bounding box
  float yRange[2]; /// min,max of the voxel bounding box
  float zRange[2]; /// min,max of the voxel bounding box
  vector<size_t> count; /// number of points in each voxel
};
#endif // _DENSITY_H_
