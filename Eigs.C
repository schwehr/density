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

/// \brief 
///        

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

//#include <gsl/gsl_rng.h>
//#include <gsl/gsl_randist.h> // gaussian distributed random number generator
// C headers
#include <gsl/gsl_eigen.h> 

// C++ headers
#include <iostream>
#include <iomanip>
#include <fstream>


using namespace std;
/***************************************************************************
 * LOCAL MACROS and DEFINES
 ***************************************************************************/

#ifdef __GNUC__
#define UNUSED __attribute((__unused__))
#else
/*! \def UNUSED
  \brief GNU CC attribute to denote unused paramters in function calls.
  The attribute remove compiler warning for unused arguments and variable.  Only works
  for GNU compilers such as gcc and g++.
*/
#define UNUSED
#endif

#ifdef REGRESSION_TEST
#define FAILED_HERE cout <<  __FILE__ << ":" << __LINE__ << " test failed" << endl
#else
/*! \def FAILED_HERE
  \brief Used FAILED_HERE; to emit a string that looks like a compiler warning
  Use this to allow emacs to jump to this source line with C-x `
*/
#define FAILED_HERE // Empty
#endif

/***************************************************************************
 * GLOBALS
 ***************************************************************************/

/// Let the debugger find out which version is being used.
static const UNUSED char* RCSid ="@(#) $Id$";

/***************************************************************************
 * EIGS
 ***************************************************************************/

// inline
double rad2deg (const float &rad) {return(rad/M_PI * 180.);}
double deg2rad (const float &deg) {return(deg/180. * M_PI);}

// convert xyz to theta (t) and phi(p) in radians
// go polar
void xyz2tpr (const double x, const double y, const double z, double &theta, double &phi, double &radius) {
  const double pi=M_PI;
  radius = sqrt(x*x+y*y+z*z);
  theta = acos(z/radius);
  if (0. == x) {
    if (y<0)  phi = 3 * pi * 0.5;
    else phi = pi * 0.5;
    return;
  }
  phi = atan(y/x);
  if (x<0) phi += pi;
  if (phi<0) phi += 2*pi;
}




/// Use K when you don't know if they are talking about value or vector.
enum EigsEnum {KMIN, KINT, KMAX};

size_t GetIndex(const EigsEnum which, const gsl_vector *v) {
  const double a=gsl_vector_get(v,0),b=gsl_vector_get(v,1),c=gsl_vector_get(v,2);
#ifndef NDEBUG
// FIX: For the isotropic case, we need to return a different index for each.
  if ( (a == b || a == c) || ( b == c ) )
    assert(false && "write code to handle this case");
#endif
  switch(which) { // sandwich
  case KMIN:
    if (a<b && a<c) return (0);
    else if (b<c) return (1);
    else return(2);
  case KINT:
    if ( (a<b && a>c) || (a>b && a<c) )  return (0);
    else if ((b<a && b>c) || (b>a && b<c)) return (1);
    else return(2);
  case KMAX:
    if (a>b && a>c) return (0);
    else if (b>c) return (1);
    else return(2);
  default: assert(false && "There goes the house of cards."); return(false);
  }
  assert(false);  // never reach heres
  return (0); // FIX: replace with badValue
}

bool GetEig(const EigsEnum which, const gsl_matrix *eigenvecs, const gsl_vector *eigenvals,
	    float vec[3], float &val) {
  assert (eigenvecs);
  assert (eigenvals);
  size_t index=GetIndex(which,eigenvals);
  //vec[0] = gsl_matrix_get(eigenvecs,index,0);
  //vec[1] = gsl_matrix_get(eigenvecs,index,1);
  //  vec[2] = gsl_matrix_get(eigenvecs,index,2);
  vec[0] = - gsl_matrix_get(eigenvecs,0,index);
  vec[1] = - gsl_matrix_get(eigenvecs,1,index);
  vec[2] = - gsl_matrix_get(eigenvecs,2,index);
  val = gsl_vector_get(eigenvals,index);
  return(true);
}

// put in lower hemisphere
void flip (double &dec, double &dip) {
  if (dip<0.) {dip=-dip;dec=dec-180;}
  if (dec<0.) {dec+=360;}
}
void flip (float &dec, float &dip) {
  if (dip<0.) {dip=-dip;dec=dec-180;}
  if (dec<0.) {dec+=360;}
}

// fill a lisa style eigen vector array.  see s_eigs
bool
GetEigs(const gsl_matrix *eigenvec, const gsl_vector *eigenval, float newEigs[9]) {
  bool ok=true;
  float vec[3];
  float val;
  double t,p,r;  // polar coords phi, theta, radius
  if (!GetEig(KMIN,eigenvec, eigenval, vec, val)) ok=false;
  xyz2tpr(vec[0],vec[1],vec[2],t,p,r);
  newEigs[0] = val; newEigs[1] = rad2deg(p); newEigs[2] = 90-rad2deg(t);
  cout << "GetEigs1: " << vec[0]<<" "<<vec[1]<<" "<<vec[2]<<" "<<t<<" "<<p<<" "<<r << " " <<val<< endl;

  flip(newEigs[1],newEigs[2]);

  if (!GetEig(KINT,eigenvec, eigenval, vec, val)) ok=false;
  xyz2tpr(vec[0],vec[1],vec[2],t,p,r);
  newEigs[3] = val; newEigs[4] = rad2deg(p); newEigs[5] = 90-rad2deg(t);
  cout << "GetEigs2: " << vec[0]<<" "<<vec[1]<<" "<<vec[2]<<" "<<t<<" "<<p<<" "<<r << " " <<val<< endl;

  flip(newEigs[4],newEigs[5]);

  if (!GetEig(KMAX,eigenvec, eigenval, vec, val)) ok=false;
  xyz2tpr(vec[0],vec[1],vec[2],t,p,r);
  newEigs[6] = val; newEigs[7] = rad2deg(p); newEigs[8] = 90-rad2deg(t);
  cout << "GetEigs3: " << vec[0]<<" "<<vec[1]<<" "<<vec[2]<<" "<<t<<" "<<p<<" "<<r << " " <<val<< endl;

  flip(newEigs[7],newEigs[8]);

  return(ok);
}


//####################################################################
// TEST CODE
//####################################################################
#ifdef REGRESSION_TEST

static bool isEqual (const float a, const float b, const float del) {
  return ( ( a<b+del && a > b-del) ? true : false );
}

bool test1() {
  //int r; // result from gsl calls.  0 is ok, !=0 is trouble
  bool ok=true;
  cout << "      test1" << endl;
  {
    double t,p,r;

    xyz2tpr(-0.640764058, -0.405717313,  0.651778281, t,p,r);
    if (!isEqual(t, 0.860869467, 0.0001)) {FAILED_HERE; ok=false;} 
    if (!isEqual(p,  3.70605087, 0.0001)) {FAILED_HERE; ok=false;} 
    if (!isEqual(r, 1., 0.0001)) {FAILED_HERE; ok=false;} 

    xyz2tpr(-0.148749143, -0.767265439, -0.623840928, t,p,r);
    if (!isEqual(t, 2.24444389 , 0.0001)) {FAILED_HERE; ok=false;} 
    if (!isEqual(p, 4.52089548, 0.0001)) {FAILED_HERE; ok=false;} 
    if (!isEqual(r, 1., 0.0001)) {FAILED_HERE; ok=false;} 

    xyz2tpr(0.753189981, -0.49668628, 0.431285977, t,p,r);
    if (!isEqual(t, 1.12487864, 0.0001)) {FAILED_HERE; ok=false;} 
    if (!isEqual(p, 5.70020008, 0.0001)) {FAILED_HERE; ok=false;} 
    if (!isEqual(r, 1., 0.0001)) {FAILED_HERE; ok=false;} 
  }

  gsl_vector *v = gsl_vector_alloc(3); // unordered eigen val
  assert(v);

  gsl_vector_set(v,0,0.1);
  gsl_vector_set(v,1,0.2);
  gsl_vector_set(v,2,0.3);
  if (0!=GetIndex(KMIN,v)) {FAILED_HERE; ok=false;}
  if (1!=GetIndex(KINT,v)) {FAILED_HERE; ok=false;}
  if (2!=GetIndex(KMAX,v)) {FAILED_HERE; ok=false;}

  gsl_vector_set(v,0,0.1);
  gsl_vector_set(v,1,0.3);
  gsl_vector_set(v,2,0.2);
  if (0!=GetIndex(KMIN,v)) {FAILED_HERE; ok=false;}
  if (2!=GetIndex(KINT,v)) {FAILED_HERE; ok=false;}
  if (1!=GetIndex(KMAX,v)) {FAILED_HERE; ok=false;}

  gsl_vector_set(v,0,0.3);
  gsl_vector_set(v,1,0.2);
  gsl_vector_set(v,2,0.1);
  if (2!=GetIndex(KMIN,v)) {FAILED_HERE; ok=false;}
  if (1!=GetIndex(KINT,v)) {FAILED_HERE; ok=false;}
  if (0!=GetIndex(KMAX,v)) {FAILED_HERE; ok=false;}

  gsl_vector_set(v,0,0.3);
  gsl_vector_set(v,1,0.1);
  gsl_vector_set(v,2,0.2);
  if (1!=GetIndex(KMIN,v)) {FAILED_HERE; ok=false;}
  if (2!=GetIndex(KINT,v)) {FAILED_HERE; ok=false;}
  if (0!=GetIndex(KMAX,v)) {FAILED_HERE; ok=false;}

  gsl_vector_set(v,0,0.2);
  gsl_vector_set(v,1,0.1);
  gsl_vector_set(v,2,0.3);
  if (1!=GetIndex(KMIN,v)) {FAILED_HERE; ok=false;}
  if (0!=GetIndex(KINT,v)) {FAILED_HERE; ok=false;}
  if (2!=GetIndex(KMAX,v)) {FAILED_HERE; ok=false;}

  gsl_vector_free(v);
  return (ok);
} // test1

bool test2() {
  int r; // result from gsl calls.  0 is ok, !=0 is trouble
  bool ok=true;
  cout << "      test2" << endl;

  // head -1 as3-undef.s
  double s[6] = {0.33294922,0.33564946,0.33140135,-.00533517,0.00335697,0.00760979};//,0.00080515};
  double eigs[9]={0.32183051, 212.34,  40.68,0.33734459,  79.03,  38.60,0.34082493, 326.60,  25.55};
  assert(eigs);
  //if (0!=d.getCountInside())     {FAILED_HERE;return(false);}

  // 0,0 1,0 2,0
  // 0,1 1,1 2,1
  // 0,2 1,2 2,2
  

  gsl_eigen_symmv_workspace *w = gsl_eigen_symmv_alloc (3);
  gsl_matrix *A = gsl_matrix_alloc(3,3);
  gsl_matrix_set(A,0,0,s[0]);
  gsl_matrix_set(A,1,1,s[1]);
  gsl_matrix_set(A,2,2,s[2]);
  gsl_matrix_set(A,0,1,s[3]);
  gsl_matrix_set(A,1,0,s[3]);
  gsl_matrix_set(A,1,2,s[4]);
  gsl_matrix_set(A,2,1,s[4]);
  gsl_matrix_set(A,0,2,s[5]);
  gsl_matrix_set(A,2,0,s[5]);

  gsl_vector *eigenval = gsl_vector_alloc(3); // unordered eigen val
  assert(eigenval);
  gsl_matrix *eigenvec = gsl_matrix_alloc(3,3); // results stored here ordered by eval
  assert(eigenvec);
  r = gsl_eigen_symmv (A, eigenval, eigenvec, w) ;
  if (r) {cout << "ERROR ("<<r<<"): " << gsl_strerror(r) << endl;ok=false;FAILED_HERE;}

  float newEigs[9];
  if (!GetEigs(eigenvec, eigenval, newEigs)) {FAILED_HERE;ok=false;}


  for (size_t i=0;i<9;i++) {cout << " " <<    eigs[i];} cout << endl;
  for (size_t i=0;i<9;i++) {cout << " " << newEigs[i];} cout << endl;

  for (size_t i=0;i<9;i++) {
    if (!isEqual(eigs[i],newEigs[i],0.06)) {
      FAILED_HERE; ok=false;
      cout << "      i = " << i << " "<< eigs[i] << " " << newEigs[i] << endl;
    }
  }

  
  //printf ("Eigen vectors:\n");
  //r = gsl_matrix_fprintf(stdout,eigenvec,"%f");
  //if(r){cout<<"ERROR:"<<gsl_strerror(r)<<endl;ok=false;FAILED_HERE;}

  //printf ("Eigen Values:\n");
  //r = gsl_vector_fprintf(stdout,eigenval,"%f");
  //if(r){cout<<"ERROR:"<<gsl_strerror(r)<<endl;ok=false;FAILED_HERE;}

  gsl_eigen_symmv_free (w);
  gsl_matrix_free(A);
  gsl_matrix_free(eigenvec);
  gsl_vector_free(eigenval);
  return (ok);
} // test2


int main (UNUSED int argc, char *argv[]) {
  // Put test code here
  bool ok=true;


  if (!test1()) {FAILED_HERE;ok=false;}
  if (!test2()) {FAILED_HERE;ok=false;}

  cout << "  " << argv[0] << " test:  " << (ok?"ok":"failed")<<endl;
  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}
#endif // REGRESSION_TEST