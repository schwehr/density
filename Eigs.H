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
#ifndef _EIGS_H_
#define _EIGS_H_

#include <gsl/gsl_eigen.h> 

#include <vector>


/// \file
/// \brief  Handle Eigenvalue translation to-from matrix, polar, and xyz coords

/// Use K when you don't know if they are talking about value or vector.
/// Otherwise use Vmax, Vint, Vmin for eigen VECTORS or Tmax, Tint, Tmin for tau/eigen VALUES
enum EigsEnum {KMAX=1, KINT=2, KMIN=3};

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

/// \brief convert xyz to theta (t) and phi(p) in radians.  Go polar.
/// \param x Cartesian x coord.
/// \param y Cartesian y coord.
/// \param z Cartesian z coord.
/// \param theta  Angle in radians from horizontal.  0 is vertical, PI/2 is horizontal, PI is down;
/// \param phi    Angle in radians in the horizontal plane. 0 is +x, PI is -x, PI/2 is +y. 
/// \param radius Distance from the origin
void xyz2tpr (const double x, const double y, const double z, double &theta, double &phi, double &radius);

/// \brief Used to pick out the index for Vmax/Tmax, Vint/Tint, Vmin/Tmin.
/// Meant just to be a helper
size_t GetIndex(const EigsEnum which, const gsl_vector *v);

/// \brief Pick out a particular eigen vector/value
/// \param which KMIN, KINT, KMAX
/// \param eigenvecs Precomputed eigen vectors.  Use gsl_eigensymmv
/// \param eigenvals Precomputed eigen values.  Use gsl_eigensymmv
/// \param vec Return array for the eigen vector
/// \param val Return the eigen value.  Should be less than 1!!! (t1+t2+t3 == 1.0)
bool GetEig(const EigsEnum which, const gsl_matrix *eigenvecs, const gsl_vector *eigenvals,
	    float vec[3], float &val);

/// \brief Fill a lisa style eigen vector array.  see s_eigs.
/// \param eigenvec Unsorted eigen vector.  They are in the order of the eigen values 
/// \param eigenval Unsorted eigen values
/// \param newEigs Store the values here are Tmin, DECmin, INCmin,
/// Tint, DECint, INCint, Tmax, DECmax, INCmax
/// \return false if there was some sort of problem.  true on success
///
/// Has to sort through the eigenvals to figure out the order of min, int, and max
bool GetEigs(const gsl_matrix *eigenvec, const gsl_vector *eigenval, float newEigs[9]);

/// \brief Takes a triple that comes from the output of s_eigs and gives the cartesian coordinates
/// \param len Eigen value
/// \param dec Declination in degrees.  0 is north (+y), 90, is east (+x), 180 is south (-y)
/// \param inc Inclination in degrees.  0 is horizontal, while 90 is straight down.
/// \param xyz Return values of x (East), y (North), z (Up)
void ldi2xyz( const float len, const float dec, const float inc, std::vector<float> &xyz);


/// \brief Engine to pass in s values and retrieve eigen parameters in different formats
///

/// Designed to be a reusable container, so you do not have to pay for
/// the cost of allocating and freeing GNU Scientific Library working
/// variables over and over.  This class does a little bit more work
/// than necessary in that it calls GetEigs no matter what, but you
/// probably want that anyway, right?
class S_Engine {
public:

  /// \brief Construct an S_Engine object, but leave it invalid.
  ///
  /// You need to
  /// call setS() before pulling out data with getXYZ() or other get
  /// methods.  You should recycle one S_Engine rather than
  /// create/destroy a new object for each s array that you want to
  /// process.  Each object has to manage 4 gsl objects.
  S_Engine();

  ~S_Engine();  ///< Deallocate gsl matrix and workspace

  /// \brief Construct an S_Engine with an initial s value.
  /// \param s Vector of size 6 or 7.  It will ignore the 7th value which is your sigma
  ///
  /// You can re-use this object by calling setS to reset it to a new value
  S_Engine(const std::vector<float> &s);

  /// \brief Insert a set of s values and compute the eigenvalues and vectors
  /// Side Effect: Sets valid to true
  bool setS(const std::vector<float> &s);

  /// \brief Get the cartisian coordinate for a particular eigen vector
  /// \param which KMIN, KINT, or KMAX
  /// \param xyz vector of size 3 to fill with the x,y, and z values
  bool getXYZ(const EigsEnum which, std::vector<float> &xyz) const;

  /// \brief Do we have a working s value that can be queried?
  /// \return false if no s has been set or if there was trouble allocating the gsl data.
  bool isValid() const {return(valid);}

  // FIX: could implement these...
  //S_Engine(const float s[6]);
  //S_Engine(const float s[7]);
  //bool getS(std::vector<float> &s);
  // Lisa Style ???
  //bool getEigs(std::vector<float> &V, vector<float> &T); ///< min, int, max sorted
  //bool getEig(const EigsEnum which, std::vector<float> &V, float &T);

  //bool getTau(const EigsEnum which, float &T);
  //bool getXYZ(const EigsEnum which, float &x, float &y, float &z);
  //bool getPTR(std::vector<float> &ptr);
  //bool getPTR(float &phi, float &theta, float &radius);
  //bool getLDI(float &length, float &dec, float &inc);
private:
  bool init(); ///< Used by constructors to allocate up GSL variables
  bool valid; ///< True if an s value has been set
  std::vector<float> s; ///< see s_eigs.  Store 6 values.  Ignores the 7th value (sigma)

  gsl_eigen_symmv_workspace *w; ///< GNU Scientific Library workspace for decomposition
  gsl_matrix *A; ///< Matrix that we will get eigen parameters from
  gsl_vector *eigenval; ///< unordered eigen values
  gsl_matrix *eigenvec; ///< results stored here ordered by eigen val

  /// \brief Lisa Tauxe style eigen parameters.  See s_eigs program
  ///
  /// 3 tau_i, dec_i inc_i triples of eigenval and eigenvectors 
  ///
  /// In groups of three with min in [0..2], int in [3..5], and max in [6..8]
  float eigs[9];

  //vector<float> xyz; ///< size==9, min, int, max triples
  //vector<float> ptr; ///< size==9, min, int, max triples
  //vector<float> eigs; ///< same format as s_eigs
};

// Calculate eigs when an S is set
// then eigs to xyz ... see python code eigs2xyz.py
// xyztoptr


#endif // _EIGS_H_
