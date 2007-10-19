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

#ifndef _CDF_H_
#define _CDF_H_

#include <vector>

/// \file
/// \brief Header providing the CDF class (Cumulative Distribution Function)


/// \brief create a CDF (Cumulative Distribution Function) of a set of values
/// @param data What to make the CDF from
/// @param value Returns the data value for this entry of the cdf.  These will be sorted from smallest to largest.
/// The value table will be sparse.
/// @param percent What percent for each value entry.  This is the % of the data included using this value.
/// The first value will \a not be 0.  The final value should be ~1.
size_t cdf(const std::vector<size_t> &data, std::vector<size_t> &value, std::vector<float> percent);

/// \brief cumulative density function

class Cdf {
public:
  /// This constructor assumes that is ok to copy and sort the data... that it is not too big.
  Cdf(const std::vector<size_t> &data, bool countZeros);

  /// Get the CDF at the point value
  float getCDF(const size_t value);
  /// Get the bracketing percents for a point... there may be many with the same value
  float getCDF(const size_t value, float &bottom, float &top);

  /// Dump the cdf to stdout
  void print();

  /// \brief Write the data out to a file in a style suitable for gnuplot
  ///
  /// gnuplot
  ///
  /// plot 'filename' with lines
  void writeForGraphing(const std::string &filename);

private:
  std::vector<size_t> value;
  std::vector<float> percent;
};


#endif // _CDF_H_
