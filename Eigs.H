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

inline double rad2deg (const float &rad) {return(rad/M_PI * 180.);}
inline double deg2rad (const float &deg) {return(deg/180. * M_PI);}

/// convert xyz to theta (t) and phi(p) in radians.  go polar
void xyz2tpr (const double x, const double y, const double z, double &theta, double &phi, double &radius);

/// \brief Used to pick out the index for Vmax/Tmax, Vint/Tint, Vmin/Tmin.
/// Meant just to be a helper
size_t GetIndex(const EigsEnum which, const gsl_vector *v);

/// \brief Pick out a particular eigen vector/value
/// \param which KMIN, KINT, KMAX
bool GetEig(const EigsEnum which, const gsl_matrix *eigenvecs, const gsl_vector *eigenvals,
	    float vec[3], float &val);

//// fill a lisa style eigen vector array.  see s_eigs
bool GetEigs(const gsl_matrix *eigenvec, const gsl_vector *eigenval, float newEigs[9]);

// Takes a triple that comes from the output of s_eigs
/// \param len Eigen value
/// \param dec Declination in degrees
/// \param inc Inclination in degrees
/// \param xyz Return values of x (East), y (North), z (Up)
void ldi2xyz( const float len, const float dec, const float inc, std::vector<float> &xyz);



///// FIX:  Create a class that takes an s and returns eigs
///// or xyz or ptr
// Idea is to cache the workspace so we don't have to keep constructing and destroying workspaces
// Put don't have to keep one.
class S_Engine {
public:
  //S_Engine();
  ~S_Engine();  ///< Deallocate gsl matrix and workspace
  //S_Engine(const float s[6]);
  //S_Engine(const float s[7]);
  S_Engine(const std::vector<float> &s);
  bool setS(const std::vector<float> &s);
  //bool getS(std::vector<float> &s);
  // Lisa Style ???
  //bool getEigs(std::vector<float> &V, vector<float> &T); ///< min, int, max sorted
  //bool getEig(const EigsEnum which, std::vector<float> &V, float &T);

  //bool getTau(const EigsEnum which, float &T);
  bool getXYZ(const EigsEnum which, std::vector<float> &xyz);
  //bool getXYZ(const EigsEnum which, float &x, float &y, float &z);
  //bool getPTR(std::vector<float> &ptr);
  //bool getPTR(float &phi, float &theta, float &radius);
  //bool getLDI(float &length, float &dec, float &inc);
private:
  bool isValid; ///< True if an s value has been set
  std::vector<float> s; ///< see seigs.  Store 6 or 7

  gsl_eigen_symmv_workspace *w;
  gsl_matrix *A;
  gsl_vector *eigenval; // unordered eigen val
  gsl_matrix *eigenvec; // results stored here ordered by eigen val

  float eigs[9];

  //vector<float> xyz; ///< size==9, min, int, max triples
  //vector<float> ptr; ///< size==9, min, int, max triples
  //vector<float> eigs; ///< same format as s_eigs
};

// Calculate eigs when an S is set
// then eigs to xyz ... see python code eigs2xyz.py
// xyztoptr


#endif // _EIGS_H_
