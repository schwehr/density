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

//####################################################################
// TEST CODE
//####################################################################
#ifdef REGRESSION_TEST

bool test1() {
  cout << "      test1" << endl;

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
  gsl_matrix_set(A,1,2,s[4]); //gamma(2,3)=d(5);
  gsl_matrix_set(A,2,1,s[4]); //gamma(3,2)=d(5);
  gsl_matrix_set(A,0,2,s[5]); //gamma(1,3)=d(6);
  gsl_matrix_set(A,2,0,s[5]); //gamma(3,1)=d(6);

  gsl_vector *eigenval = gsl_vector_alloc(3); // unordered eigen val
  gsl_matrix *eigenvec = gsl_matrix_alloc(3,3); // results stored here ordered by eval
  int r = gsl_eigen_symmv (A, eigenval, eigenvec, w) ;
  if (r) {
    cout << "ERROR ("<<r<<"): " << gsl_strerror(r) << endl;
    
  }
  /*
    int gsl_eigen_symmv (gsl_matrix * A, gsl_vector * eval, gsl_matrix
    * evec, gsl_eigen_symmv_workspace * w)

    This function computes the eigenvalues and eigenvectors of the
    real symmetric matrix A. Additional workspace of the appropriate
    size must be provided in w. The diagonal and lower triangular part
    of A are destroyed during the computation, but the strict upper
    triangular part is not referenced. The eigenvalues are stored in
    the vector eval and are unordered. The corresponding eigenvectors
    are stored in the columns of the matrix evec. For example, the
    eigenvector in the first column corresponds to the first
    eigenvalue. The eigenvectors are guaranteed to be mutually
    orthogonal and normalised to unit magnitude.
  */
    
  gsl_eigen_symmv_free (w);
  gsl_matrix_free(A);
  gsl_matrix_free(eigenvec);
  gsl_vector_free(eigenval);
  return (true);
} // test1


int main (UNUSED int argc, char *argv[]) {
  // Put test code here
  bool ok=true;


  if (!test1()) {FAILED_HERE;ok=false;}

  cout << "  " << argv[0] << " test:  " << (ok?"ok":"failed")<<endl;
  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}
#endif // REGRESSION_TEST
