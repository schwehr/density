// $Revision$  $Author$  $Date$
#ifndef _CDF_H_
#define _CDF_H_

#include <vector>

/// \brief create a CDF (Cumulative Distribution Function) of a set of values
/// @param data What to make the CDF from
/// @param value Returns the data value for this entry of the cdf.  These will be sorted from smallest to largest.
/// The value table will be sparse.
/// @param percent What percent for each value entry.  This is the % of the data included using this value.
/// The first value will \a not be 0.  The final value should be ~1.
size_t cdf(const std::vector<size_t> &data, std::vector<size_t> &value, std::vector<float> percent);

class Cdf {
public:
  /// This constructor assumes that is ok to copy and sort the data... that it is not too big.
  Cdf(const std::vector<size_t> &data, bool countZeros);

  /// Get the CDF at the point value
  float getCDF(const size_t value);
  /// Get the bracketing percents for a point... there may be many with the same value
  float getCDF(const size_t value, float &bottom, float &top);

private:
  std::vector<size_t> value;
  std::vector<float> percent;
};


#endif // _CDF_H_
