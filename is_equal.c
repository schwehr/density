/*#include <iostream>
using namespace std; */
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

#include <stdio.h>
#include <stdlib.h>

#define true  (1==1)
#define false (0==1)

int
isEqual (const float a, const float b, const float del) {
  return ( ( a<b+del && a > b-del) ? true : false );
}

int main (int argc, char *argv[]) {
  float v1,v2,sigma;
  int eq;

  if (4!=argc) {
    fprintf (stderr,"USAGE: %s value1 value2 sigma", argv[0]);
    return (EXIT_FAILURE);
  }

  v1    = atof(argv[1]);
  v2    = atof(argv[2]);
  sigma = atof(argv[3]);

  eq = isEqual(v1,v2,sigma);
  /*cout << (eq?"Equal":"NOT Equal") << endl; */
  return (eq?EXIT_SUCCESS:EXIT_FAILURE);
}
