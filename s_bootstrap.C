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
#include <fstream>

#include "kdsPmagL.H" // L is for local
#include "SiteSigma.H"

using namespace std;

// return true if all went well.
// false if trouble of any kind

// Unlike Lisa's code, this one does NOT alter the sigmas on loading
// which is what her adread subroutine did.
bool
LoadS(const string filename,vector <SVec> &s,vector<float> &sigmas) {
  ifstream in(filename.c_str(),ios::in);
  if (!bool(in)) {cerr << "failed to open file: " << filename << endl; return false; }

  // FIX: detect formats -  {s[6]}, {s[6],sigma}, {name, sigma, s[6]}
  // FIX: only do {s[6],sigma} for now
  SVec tmp(6,0.);
  float tmpSigma;
  while (in >> tmp[0] >> tmp[1] >> tmp[2] >> tmp[3] >> tmp[4] >> tmp[5] >> tmpSigma) {
    //{static int i=0; cout << i++ << ": ";}
    //for (size_t i=0;i<6;i++) cout << tmp[i] << " ";
    //cout << endl; // cout << sigma << endl;
    s.push_back(tmp);
    sigmas.push_back(tmpSigma);
  }
  return (true);
}


#ifndef REGRESSION_TEST
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
#endif // !REGRESSION_TEST

//////////////////////////////////////////////////////////////////////
// REGRESSION TESTS
//////////////////////////////////////////////////////////////////////

#ifdef REGRESSION_TEST
static bool
isEqual (const float a, const float b, const float del) {
  return ( ( a<b+del && a > b-del) ? true : false );
}

bool Test1 (void) {
  vector<SVec> s;
  vector<float> sigmas;
  if (!LoadS(string("as1-crypt.s"),s,sigmas)) {FAILED_HERE; return false;};
  const float siteSigma = SiteSigma(s);
  const float expectedSiteSigma = 0.00133387523;
  if (!isEqual(siteSigma, expectedSiteSigma, 0.000001)) {FAILED_HERE; return false;}
  
  return (true);
}



int main(UNUSED int argc, char *argv[]) {
  bool ok=true;

  if (!Test1()) {FAILED_HERE;ok=false;};

  cout << argv[0] << " :" << (ok?"ok":"FAILED") << endl;
  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}
#endif // REGRESSION_TEST
