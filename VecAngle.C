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

/// \file
/// \brief Implement vector, angle, and coordinate frame functions


/***************************************************************************
 * INCLUDES
 ***************************************************************************/

// C headers
#include <cmath>
// C++ headers
#include <iostream>
#include <iomanip>
#include <fstream>

// STL Headers
#include <vector>

#include "VecAngle.H"

using namespace std;

/***************************************************************************
 * MACROS, DEFINES, GLOBALS
 ***************************************************************************/

#include "debug.H" // provides FAILED_HERE, UNUSED, DebugPrintf

/// Let the debugger find out which version is being used.
static const UNUSED char* RCSid ="@(#) $Id$";


/***************************************************************************
 * Vector/Angle helpers
 ***************************************************************************/

float normRadAngle(float angleRad) {
  while (angleRad<0.) angleRad += 2*M_PI;
  while (angleRad>2*M_PI) angleRad -= 2*M_PI;
  assert (0<= angleRad && angleRad <= 2*M_PI);
  return (angleRad);
}


void rotateXY (const float x1, const float y1, const float angleRad, float &x2, float &y2) {
  //assert (0<= angleRad && angleRad <= 2*M_PI);
  const float radius = sqrt(x1*x1 + y1*y1);
  const float oldAngle = atan2(y1,x1);
  const float newAngle = normRadAngle(oldAngle + angleRad);
  x2 = radius * cos(newAngle);
  y2 = radius * sin(newAngle);
}

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
// TEST CODE
//####################################################################
#ifdef REGRESSION_TEST


bool test1() {
  cout << "      test1" << endl;
  bool ok=true;

  if (!isEqual(normRadAngle(-M_PI),M_PI,0.0001)) {FAILED_HERE;ok=false;}
  if (!isEqual(normRadAngle(3*M_PI),M_PI,0.0001)) {FAILED_HERE;ok=false;}

  float x1,y1,x2,y2;
  x1=1.0; y1=0.0; rotateXY(x1,y1,0.,x2,y2);
  if (!isEqual(x1,x2,0.0001)){FAILED_HERE;ok=false;}

  float angle = M_PI/2;

  rotateXY(x1,y1,angle,x2,y2);
  cout << "  " << x1 << " " << y1 << " " << angle << "    " << x2 << " " << y2 << endl;
  if (!isEqual(x2,0.f,0.0001)){FAILED_HERE;ok=false;}
  if (!isEqual(y2,1.f,0.0001)){FAILED_HERE;ok=false;}


  angle = -M_PI/2;
  rotateXY(x1,y1,angle,x2,y2);
  cout << "  " << x1 << " " << y1 << " " << angle << "    " << x2 << " " << y2 << endl;
  if (!isEqual(x2,0.f,0.0001)){FAILED_HERE;ok=false;}
  if (!isEqual(y2,-1.f,0.0001)){FAILED_HERE;ok=false;}


  return (ok);
}

int main (UNUSED int argc, char *argv[]) {
  // Put test code here
  bool ok=true;

  if (!test1()) {FAILED_HERE;ok=false;}

  cout << "  " << argv[0] << " test:  " << (ok?"ok":"failed")<<endl;
  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}
#endif // REGRESSION_TEST
