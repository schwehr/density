// g++ -o s_bootstrap s_bootstrap.C -I/sw/include -L/sw/lib -lgsl -lgslcblas

/*

Random: double gsl_ran_gaussian (const gsl_rng * r, double sigma)

This function returns a Gaussian random variate, with mean zero and
standard deviation sigma. The probability distribution for Gaussian
random variates is,

p(x) dx = {1 \over \sqrt{2 \pi \sigma^2}} \exp (-x^2 / 2\sigma^2) dx

for x in the range -\infty to +\infty. Use the transformation z = \mu
+ x on the numbers returned by gsl_ran_gaussian to obtain a Gaussian
distribution with mean \mu. This function uses the Box-Mueller
algorithm which requires two calls to the random number generator r.

*/

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include <iostream>

using namespace std;

int main(int argc, char *argv) {
  gsl_rng * r;  /* global generator */
  const gsl_rng_type * T;

  gsl_rng_env_setup();

  T = gsl_rng_default;
  r = gsl_rng_alloc (T);
  
  //printf ("generator type: %s\n", gsl_rng_name (r));
  //printf ("seed = %lu\n", gsl_rng_default_seed);
  {
    FILE * devRandom = fopen ("/dev/random", "r");
    assert (devRandom);
    unsigned long int s;
    fread (&s,sizeof(unsigned long int),1,devRandom);
    //cout << "devRandom = "  << s << endl;
    gsl_rng_set (r, s);
  }

  //printf ("first value = %lu\n", gsl_rng_get (r));

  //cout << "gausian: " <<  gsl_ran_gaussian (r, 1.0) << endl; 
  for (int i=0; i<10000; i++) cout << gsl_ran_gaussian (r, 1.0) << endl; 

  return 0;
}
