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

#ifndef _DENSITY_H_
#define _DENSITY_H_

#include <vector>
#include <string>

/// \file
/// \brief Convert an xyz point set into a volume density
///        Uses a voxel representation.  Provides the \a Density class



/// \brief How to squeeze data into the output sample size
/// If the counts are greater than the max value of the output
/// bitsPerPixel, this is how we should handle the values.
/// Additionally PACK_SCALE will scale no mater what.
/// You can control the color scheme of these points on load time
/// in SIM Voleon using a \a colorMap in an SoTransferFunction node

enum PackType {PACK_SCALE, PACK_CLIP, PACK_WRAP};


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

  /// \brief Load a vol file from disk
  /// \param filename File to load from
  /// \param ok Set to \a true if the file was read ok
  ///
  /// Do not use the Density structure ok was \a false.
  Density(const std::string &filename, bool &ok);

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



  /// \brief Change the volume range/scale. Does not destroy the data.  Just where we think it lives
  /// @param minX, maxX 3D space x range of voxel volume.
  /// @param minY, maxY 3D space x range of voxel volume.
  /// @param minZ, maxZ 3D space x range of voxel volume.
  void rescale(const float minX, const float maxX,
	       const float minY, const float maxY,
	       const float minZ, const float maxZ
	      );


  /// Add a point into the voxel structure.  Figures out which cell for you
  /// \return \a true if inside the bounding box, \a false if outside and unrecorded
  bool addPoint(const float x, const float y, const float z);
  /// Add points to a cell at \a index
  void addPoints(const size_t index,const size_t count) {assert(isValidCell(index)); counts[index]+=count; totalPointsInside+=count;}
  /// Print to stdout the cell number and the count
  void printCellCounts() const;
  size_t getWidth()  const {return (width);} ///< num of cells wide
  size_t getHeight() const {return (height);} ///< num of cells tall
  size_t getDepth()  const {return (depth);} ///< num of cell front to back
  size_t getSize() const {return (counts.size());} ///< How many voxels in this density space?

  /// How many points so far have been added that actually fall in the voxels' volumes
  size_t getCountInside() const {return(totalPointsInside);}
  /// \brief How many points failed to get added with addPoint().
  ///
  /// This will be zero for volumes loaded from files.
  size_t getCountOutside() const {return(totalPointsOutside);}

  /// \brief Which cell number a point in space goes to.
  /// \return Cell number or \a badValue if x,y,z is not in the volume
  size_t getCell(const float x, const float y, const float z) const;
  bool   isValidCell(const size_t i) const {return(i<counts.size()?true:false);}
  size_t getCellX(const float x) const {return(size_t((x-xR[0])/dx));}
  size_t getCellY(const float y) const {return(size_t((y-yR[0])/dy));}
  size_t getCellZ(const float z) const {return(size_t((z-zR[0])/dz));}
  void   getCellXYZ(const size_t index, size_t &cx, size_t &cy, size_t &cz) const;
  size_t getCellFromWHD(const size_t xIndex, const size_t yIndex, const size_t zIndex) const;


  /// \brief Which neightbor cell/voxel are we talking about.
  /// In order then are x-axis less, x more, y less, y more, z less, z more
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

  /// \brief Write out a vol formatted file for SIM Voleon in uchar format
  /// \return \a true on success or \a false if there was an error
  /// This write scales the data between min/max
  bool writeVolScale(const std::string &filename) const;

  /// Compress data based on PackType
  size_t scaleValue(const size_t value, const PackType p, const size_t bitsPerVoxel) const;

  /// \brief Write out a Voleon style voxel 3D cell file.
  /// \return \a true if success or \a false if there was some trouble
  /// Uses the x,y, and z ranges to set the Scale[XYZ] fields of the header
  /// \bug What does index_bits do?
  bool writeVol(const std::string &filename,const size_t bitsPerPixel,const PackType p) const;

  /// \brief Write out a Voleon style voxel 3D cell file.
  /// \return \a true if success or \a false if there was some trouble
  /// The user/caller defines the scale... since I don't understand scaling.
  /// \bug What does index_bits do?
  /// @param filename File to write out to
  /// @param bitsPerVoxel Can be 8, 16, or 32
  /// @param p PackType 0=SCALE, 1=CLIP, 2=WRAP
  /// @param scaleX Scale the x axis by using the vol header
  /// @param scaleY Scale the y axis by using the vol header
  /// @param scaleZ Scale the z axis by using the vol header
  /// @param rotX Rotate around x axis using the vol header
  /// @param rotY Rotate around y axis using the vol header
  /// @param rotZ Rotate around z axis using the vol header
  bool writeVol(const std::string &filename,
		       const size_t bitsPerVoxel,const PackType p,
		       const float scaleX, const float scaleY, const float scaleZ,
		       const float rotX=0.f, const float rotY=0.f,const float rotZ=0.f
		) const;


  /// This is the value for an unknown or bad entry.  Should be a huge number that we would never encounter.
  static size_t badValue() {return(std::numeric_limits<size_t>::max());}

  /// Return a number scaled between 0 and 255
  unsigned char scaleCount(const size_t i, const size_t min, const size_t max) const;

  /// \brief fill two vectors with the count and the percent.  Both will be of size getMaxCount()
  /// \param cdfpercent How many cells have have the value of the index or smaller?
  /// \return \a false if there was some trouble
  bool buildCDF(std::vector<float> &cdfpercent) const;

private:
  // Computing min/max doesn't change the object, so mutable
  mutable bool stale; ///< set to true when max and min have to be recomputed
  mutable size_t maxCache, minCache;
  /// Call this on anytime the counts change.
  /// This is only called when the object changes, so NOT const!
  void invalidateCache() {stale=true; maxCache=minCache=badValue();}
  void computeMinMax() const;

public:

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
  size_t totalPointsOutside; ///< How many points have failed to add since they were outside the volume
};
#endif // _DENSITY_H_
