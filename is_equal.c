/*#include <iostream>
using namespace std; */
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
