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
#ifndef _VECANGLE_H_
#define _VECANGLE_H_

#include <math.h>

/// \file
/// \brief  Utilities for vectors and angles.

/// \brief Converter radians to degrees
/// \param rad Angle in radians.
/// \return Angle in degrees
/// 
/// Note: does \a not force the degrees to be in [0..360]
inline double rad2deg (const float &rad) {return(rad/M_PI * 180.);}
/// \brief Converter degrees to radians
/// \param deg Angle in degrees.
/// \return Angle in radians.
/// 
/// Note: does \a not force the radians to be in [0..2PI]
inline double deg2rad (const float &deg) {return(deg/180. * M_PI);}

/// \brief put a vector in the lower hemisphere
/// \param dec Declination from north clockwise.  angle in degrees
/// \param dip Dip of vector.  Returns 0 is horizontal, 90 is vertical, down
/// \bug Should this be a template?
void flip (double &dec, double &dip); 
/// \brief put a vector in the lower hemisphere
/// \param dec Declination from north clockwise.  angle in degrees
/// \param dip Dip of vector.  Returns 0 is horizontal, 90 is vertical, down
void flip (float &dec, float &dip);


/// \brief renormalize an angle to be in [0..2*M_PI]
/// \param angleRad  Angle in radians.  No limit to the value
/// \return angle in [0..2*M_PI]
/// \bug will be slow for angles that are very far from the returned range.  1000000*M_PI would take a while
float normRadAngle(float angleRad);

/// \brief rotate an XY vector about the origin
/// \param x1 Input x value
/// \param y1 Input y value
/// \param angleRad Angle in radians to rotate by.  Right handed rotation.
/// \param x2 Output x value
/// \param y2 Output y value
///
/// An example... rotateXY (1.0, 0.0, M_PI/4, x2, y2) will give:
/// (0.0, 1.0)
void rotateXY (const float x1, const float y1, const float angleRad, float &x2, float &y2);


/// \brief convert xyz to theta (t) and phi(p) in radians.  Go polar.
/// \param x Cartesian x coord.
/// \param y Cartesian y coord.
/// \param z Cartesian z coord.
/// \param theta  Angle in radians from horizontal.  0 is vertical, PI/2 is horizontal, PI is down;
/// \param phi    Angle in radians in the horizontal plane. 0 is +x, PI is -x, PI/2 is +y. 
/// \param radius Distance from the origin
void xyz2tpr (const double x, const double y, const double z, double &theta, double &phi, double &radius);

/// \brief Takes a triple that comes from the output of s_eigs and gives the cartesian coordinates
/// \param len Eigen value
/// \param dec Declination in degrees.  0 is north (+y), 90, is east (+x), 180 is south (-y)
/// \param inc Inclination in degrees.  0 is horizontal, while 90 is straight down.
/// \param xyz Return values of x (East), y (North), z (Up)
void ldi2xyz( const float len, const float dec, const float inc, std::vector<float> &xyz);

/// \brief Compare two floats to see if they are close to each other
/// \param a,b values to compare
/// \param del how much are a and b allowed to differ
/// \return \a true if the two values are within del of each other
inline bool isEqual (const float a, const float b, const float del) {
  return ( ( a<b+del && a > b-del) ? true : false );
}


#endif // _VECANGLE_H_
