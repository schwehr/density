// $Revision$ $Author$ $Date$
#ifndef _DENSITY_FLAGGED_H_
#define _DENSITY_FLAGGED_H_

#include <vector>

//#include "Density.H"
class Density;

/// \brief Density function with tracking of used points for growing a volumn
///
/// Take the standard density class and allow growing of a connected
/// usage surface.  Uses largest neighbor traversal.  This should be SLOW

class DensityFlagged : public Density {
public:
  DensityFlagged(const size_t width, const size_t height, const size_t depth,
		 const float minX, const float maxX,
		 const float minY, const float maxY,
		 const float minZ, const float maxZ
	  );

  /// Return the index of the highest count cell.  First occurance of this high value
  size_t getLargest() const;
  /// Return the index of the highest count cell without flag set true.  First occurance of this high value
  size_t getLargestUnflagged() const;

  /// \brief Which cell next to this cell has the largest count?
  /// \param index Cell number to look around
  /// \return cell index of largest or \a Density::badValue if nothing possible.  Picks lowest index if there are duplicates
  size_t getLargestNeighbor(const size_t index) const;
  /// \brief Which cell next to this cell has the largest count?  Ignore those that have been flagged
  /// \param index Cell number to look around
  /// \return cell index of largest or \a Density::badValue if nothing possible.  Picks lowest index if there are duplicates
  size_t getLargestUnflaggedNeighbor(const size_t index) const;

  /// \brief flagged means a vell has been used
  /// \a false if not flagged or \a true if flagged
  bool isFlagged(size_t i) const {assert(isValidCell(i)); return(flags[i]);}
  void setFlag(size_t i, bool v=true) {assert(isValidCell(i)); flags[i]=v;}

private:
  std::vector<bool> flags;
  std::vector<size_t> used;
};


#endif // _DENSITY_FLAGGED_H_
