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

// STL Headers
#include <vector>

#include "Eigs.H"

using namespace std;

/***************************************************************************
 * MACROS, DEFINES, GLOBALS
 ***************************************************************************/

#include "debug.H" // provides FAILED_HERE, UNUSED, DebugPrintf

/// Let the debugger find out which version is being used.
static const UNUSED char* RCSid ="@(#) $Id$";

/***************************************************************************
 * EIGS
 ***************************************************************************/


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
  //cout << "GetEigs1: " << vec[0]<<" "<<vec[1]<<" "<<vec[2]<<" "<<t<<" "<<p<<" "<<r << " " <<val<< endl;

  flip(newEigs[1],newEigs[2]);

  if (!GetEig(KINT,eigenvec, eigenval, vec, val)) ok=false;
  xyz2tpr(vec[0],vec[1],vec[2],t,p,r);
  newEigs[3] = val; newEigs[4] = rad2deg(p); newEigs[5] = 90-rad2deg(t);
  //cout << "GetEigs2: " << vec[0]<<" "<<vec[1]<<" "<<vec[2]<<" "<<t<<" "<<p<<" "<<r << " " <<val<< endl;

  flip(newEigs[4],newEigs[5]);

  if (!GetEig(KMAX,eigenvec, eigenval, vec, val)) ok=false;
  xyz2tpr(vec[0],vec[1],vec[2],t,p,r);
  newEigs[6] = val; newEigs[7] = rad2deg(p); newEigs[8] = 90-rad2deg(t);
  //cout << "GetEigs3: " << vec[0]<<" "<<vec[1]<<" "<<vec[2]<<" "<<t<<" "<<p<<" "<<r << " " <<val<< endl;

  flip(newEigs[7],newEigs[8]);

  return(ok);
}

void ldi2xyz( const float len, const float dec, const float inc, vector<float> &xyz) {
  assert(xyz.size()==3);
  const float incRad = deg2rad(inc);
  const float decRad = deg2rad(dec);
  const float x = len * cos(incRad) * sin(decRad);
  const float y = len * cos(incRad) * cos(decRad);
  const float z = -sin(incRad)*len;
  xyz[0] = x; xyz[1]=y; xyz[2]=z;
}




//####################################################################
// S_Engine
//####################################################################

S_Engine::S_Engine() {
  valid=false;
  if (!init()) {cerr<<"S_Engine ERROR: bad state.  Danger Will Robinson!"<<endl; return;}
  s.resize(6);
  valid=false;
}

S_Engine::S_Engine(const std::vector<float> &_s) {
  valid=false;
  if (_s.size()!=6 && _s.size()!=7) {
    cerr<< "S_Engine ERROR: requires s vec of size 6 or 7" << endl;
    valid=false; return;
  }
  if (!init()) {assert(!valid); return;}
  s.resize(6);
  if (!setS(_s)) {
    cerr << "S_Engine ERROR: unable to set s vector" << endl;
    valid=false; assert(!valid); return;
  }
  valid=true;
}

bool S_Engine::init() {
  // Setup GNU Scientific Library
  w = gsl_eigen_symmv_alloc (3);
  A = gsl_matrix_alloc(3,3);
  eigenval = gsl_vector_alloc(3);
  eigenvec = gsl_matrix_alloc(3,3);
  if (!w || !A || !eigenval || !eigenvec) {
    cerr << "S_Engine ERROR: Unable to allocate gsl workspace, vector, or matrix"<<endl;
    return (false); // valid is still false
  }
  return(true);
}

S_Engine::~S_Engine() {
  // Vectors clean themselves up
  if (w) gsl_eigen_symmv_free (w);
  if (A) gsl_matrix_free(A);
  if (eigenvec) gsl_matrix_free(eigenvec);
  if (eigenval) gsl_vector_free(eigenval);
}


bool S_Engine::setS(const vector<float> &_s) {
  assert(6==_s.size() || 7==_s.size());
  // FIX: is there a copier that is better/faster?
  for (size_t i=0;i<6;i++) s[i] = _s[i];

  gsl_matrix_set(A,0,0,s[0]);
  gsl_matrix_set(A,1,1,s[1]);
  gsl_matrix_set(A,2,2,s[2]);
  gsl_matrix_set(A,0,1,s[3]);
  gsl_matrix_set(A,1,0,s[3]);
  gsl_matrix_set(A,1,2,s[4]);
  gsl_matrix_set(A,2,1,s[4]);
  gsl_matrix_set(A,0,2,s[5]);
  gsl_matrix_set(A,2,0,s[5]);

  // computes the eigenvalues and eigenvectors of the real symmetric matrix A
  int r = gsl_eigen_symmv (A, eigenval, eigenvec, w) ;
  if (r) {
    valid=false;
    cout << "ERROR ("<<r<<"): " << gsl_strerror(r) << endl;FAILED_HERE;return(false);
  }
  if (!GetEigs(eigenvec, eigenval, eigs)) {valid=false;FAILED_HERE;return(false);}
  valid=true;
  return(valid);
}

bool S_Engine::getXYZ(const EigsEnum which, vector<float> &xyz) const {
  if (!valid) return(false);
  assert(w);  assert(A); assert(eigenval); assert(eigenvec);
  size_t base;
  switch(which) {
  case KMIN: base=0*3; break;
  case KINT: base=1*3; break;
  case KMAX: base=2*3; break;
  default: assert(false); return(false);
  }
  ldi2xyz(eigs[base+0],eigs[base+1], eigs[base+2],xyz);

  return (true);
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

    xyz2tpr( 1.,0.,0.,t,p,r); cout << "  1  0  0 -> "<<t << " " << p << " " << r << endl;
    xyz2tpr(-1.,0.,0.,t,p,r); cout << " -1  0  0 -> "<<t << " " << p << " " << r << endl;
    xyz2tpr(0., 1.,0.,t,p,r); cout << "  0  1  0 -> "<<t << " " << p << " " << r << endl;
    xyz2tpr(0.,-1.,0.,t,p,r); cout << "  0 -1  0 -> "<<t << " " << p << " " << r << endl;
    xyz2tpr(0.,0., 1.,t,p,r); cout << "  0  0  1 -> "<<t << " " << p << " " << r << endl;
    xyz2tpr(0.,0.,-1.,t,p,r); cout << "  0  0 -1 -> "<<t << " " << p << " " << r << endl;


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


bool test3() {
  bool ok=true;
  cout << "      test3" << endl;

  // head -1 as1-crypt.s
  float s7[7] = {0.34406993,0.34145042,0.31447965,-.00168017,0.00414139,0.00152699,0.00052489};
  //float s6[6] = {0.34406993,0.34145042,0.31447965,-.00168017,0.00414139,0.00152699};
  float eigs[9] = {0.31375569, 248.97, 80.68,
		   0.34134611, 62.79, 9.26,
		   0.34489819, 152.96,   0.99};
  float xyzs[9] = {-0.0474277317059,-0.0182342984952,-0.309613878292,
		   0.299615527636,0.154047575992,-0.0549276500731,
		   0.156771598472,-0.307151292216,-0.00595911637815};

  // test length dec inc to xyz.  Same as eigs2xyz.py
  {
    vector<float> xyz(3,0); // set to length 3

    // North = + y
    ldi2xyz(1,0,0,xyz);
    //for (size_t i=0;i<3;i++) cout << i << " " << xyz[i] << endl;
    if (!isEqual(xyz[0],0,0.001)) {FAILED_HERE;ok=false;}
    if (!isEqual(xyz[1],1,0.001)) {FAILED_HERE;ok=false;}
    if (!isEqual(xyz[2],0,0.001)) {FAILED_HERE;ok=false;}

    // East == +x
    ldi2xyz(1.,90.,0.,xyz);
    if (!isEqual(xyz[0],1,0.001)) {FAILED_HERE;ok=false;}
    if (!isEqual(xyz[1],0,0.001)) {FAILED_HERE;ok=false;}
    if (!isEqual(xyz[2],0,0.001)) {FAILED_HERE;ok=false;}

    // Straight down
    ldi2xyz(1,0,90,xyz);
    if (!isEqual(xyz[0],0,0.001)) {FAILED_HERE;ok=false;}
    if (!isEqual(xyz[1],0,0.001)) {FAILED_HERE;ok=false;}
    if (!isEqual(xyz[2],-1,0.001)) {FAILED_HERE;ok=false;}

    ldi2xyz(eigs[0],eigs[1],eigs[2],xyz);
    for (size_t i=0;i<3;i++) cout << i << " " << xyz[i] << " " << xyzs[i] << endl;
    if (!isEqual(xyz[0],xyzs[0],0.001)) {FAILED_HERE;ok=false;}
    if (!isEqual(xyz[1],xyzs[1],0.001)) {FAILED_HERE;ok=false;}
    if (!isEqual(xyz[2],xyzs[2],0.001)) {FAILED_HERE;ok=false;}
  }

  {
    S_Engine se;
    if (se.isValid()) {FAILED_HERE;ok=false;}
    vector<float> sv7(&s7[0],&s7[7]); 
    se.setS(sv7);
    if (!se.isValid()) {FAILED_HERE;ok=false;}
  }

  {
    vector<float> sv7(&s7[0],&s7[7]); 
    S_Engine se(sv7);
    //vector<float> sv6(&s7[0],&s7[6]); 
    //se.setS(sv6);
    //se.setS(sv7);
    vector<float> xyz(3,0);
    se.getXYZ(KMIN,xyz);
    if (!isEqual(xyz[0],xyzs[0],0.001)) {FAILED_HERE;ok=false;}
    if (!isEqual(xyz[1],xyzs[1],0.001)) {FAILED_HERE;ok=false;}
    if (!isEqual(xyz[2],xyzs[2],0.001)) {FAILED_HERE;ok=false;}
  }

  return(ok);
}

int main (UNUSED int argc, char *argv[]) {
  // Put test code here
  bool ok=true;

  if (!test1()) {FAILED_HERE;ok=false;}
  if (!test2()) {FAILED_HERE;ok=false;}
  if (!test3()) {FAILED_HERE;ok=false;}

  cout << "  " << argv[0] << " test:  " << (ok?"ok":"failed")<<endl;
  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}
#endif // REGRESSION_TEST
