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
#include <iomanip>
#include <fstream>
#include <string>

#include "kdsPmagL.H" // L is for local
#include "SiteSigma.H"
#include "Bootstrap.H"

using namespace std;

// return true if all went well.
// false if trouble of any kind

// Unlike Lisa's code, this one does NOT alter the sigmas on loading
// which is what her adread subroutine did.
// Have to call SiteSigma if doing a Site based Parametric Bootstrap
bool
LoadS(const string filename,vector <SVec> &s,vector<float> &sigmas) {
  ifstream in(filename.c_str(),ios::in);
  if (!bool(in)) {cerr << "failed to open file: " << filename << endl; return false; }

  // FIX: detect formats -  {s[6]}, {s[6],sigma}, {name, sigma, s[6]}
  // FIX: only do {s[6],sigma} for now
  SVec tmp(6,0.);  float tmpSigma;
  while (in >> tmp[0] >> tmp[1] >> tmp[2] >> tmp[3] >> tmp[4] >> tmp[5] >> tmpSigma) {
    s.push_back(tmp);  sigmas.push_back(tmpSigma);
  }
  // FIX: do we need to normalize so that the trace is 1?
  // Not all data will have a trace==1??
  return (true);
}

//////////////////////////////////////////////////////////////////////
// MAIN
//////////////////////////////////////////////////////////////////////
#ifndef REGRESSION_TEST  // NOT testing

enum BootTypeEnum { SITE_PARAMETRIC, SAMPLE_PARAMETRIC };

int main(int argc, char *argv[]) {
  if (4!=argc) {
    cerr << endl
	 << "Usage: " << argv[0] << " -{P,p} sFile numSamples " << endl
	 << endl
	 << "  -p Sample parametric bootstrap" << endl
	 << "  -P Site   parametric bootstrap" << endl
	 << endl
	 << "  e.g.:  " << argv[0] << " -P as1-crypt.s 500 > as1-bootPsite"<<endl
	 << endl;
      exit(EXIT_FAILURE);
  }


  gsl_rng * r;  /* global generator */
  const gsl_rng_type *T;
  gsl_rng_env_setup();
  T = gsl_rng_default;
  r = gsl_rng_alloc (T);
  { unsigned long int s;  getDevRandom(s);  gsl_rng_set (r, s); } // Set the Seed

  BootTypeEnum type;
  {
    string typestr(argv[1]);
    if      (string::npos != typestr.find(string("-P"))) type=SITE_PARAMETRIC;
    else if (string::npos != typestr.find(string("-p"))) type=SAMPLE_PARAMETRIC;
    else {
      cerr << "first argument must be either: " << endl
	   << "  -p    sample paramtric" << endl
	   << "  -P    site   paramtric" << endl;
    }
  }
  const string filename(argv[2]);
  const size_t numSamples(atoi(argv[3])); // how many samples to generate

  vector<SVec> s;
  vector<float> sigmas;
  if (!LoadS(filename,s,sigmas)) {
    cerr << "ERROR: can't load datafile.  Tough luck... goodbye" << endl;
    exit(EXIT_FAILURE);
  }

  const float siteSigma = (SITE_PARAMETRIC==type)?SiteSigma(s):-666.;
#if 0
  switch (type) {
  case SITE_PARAMETRIC:   cout << "SITE"   << endl; break;
  case SAMPLE_PARAMETRIC: cout << "SAMPLE" << endl; break;
  default:  assert(false);
  }
#endif

  SVec newSample(6,0.);
  cout << setiosflags(ios::fixed) /*<< setw(14)*/ << setprecision(10);
  for (size_t i=0;i<numSamples; i++) {
    switch (type) {
    case SITE_PARAMETRIC:   BootstrapParametricSite   (s,siteSigma,newSample, r); Print(newSample);
      cout << endl;
      //cout << " 0.00000" << endl;
      break;
    case SAMPLE_PARAMETRIC: BootstrapParametricSample (s,sigmas   ,newSample, r); Print(newSample);
      cout << endl;
      //cout << " 0.00000" << endl;
      break;
    default:
      assert(false);
    }
  }

  return (EXIT_SUCCESS);
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

  gsl_rng *r;
  const gsl_rng_type *T;
  gsl_rng_env_setup();
  T = gsl_rng_default;
  r = gsl_rng_alloc (T);
  // Let it always start with the same value
  gsl_rng_set(r,0);
  //{ unsigned long int seed; getDevRandom(seed); gsl_rng_set(r,s); }

  SVec newSample;
  const size_t   siteNum = BootstrapParametricSite  (s,siteSigma,newSample, r);
  cout << "Site   boot picked sample " << siteNum << endl;
  const size_t sampleNum = BootstrapParametricSample(s,sigmas   ,newSample, r);
  cout << "Sample boot picked sample " << sampleNum << endl;

  return (true);
}

bool Test2 () {
  { char   a;  cout << "  devRandom char  : " << short(getDevRandom (a)) << endl; }
  { short  a;  cout << "  devRandom short : " << getDevRandom (a) << endl; }
  { int    a;  cout << "  devRandom int   : " << getDevRandom (a) << endl; }
  { long   a;  cout << "  devRandom long  : " << getDevRandom (a) << endl; }
  { float  a;  cout << "  devRandom float : " << getDevRandom (a) << endl; }
  { double a;  cout << "  devRandom double: " << getDevRandom (a) << endl; }
  return (true);
}

int main(UNUSED int argc, char *argv[]) {
  bool ok=true;

  if (!Test1()) {FAILED_HERE;ok=false;};
  if (!Test2()) {FAILED_HERE;ok=false;};

  cout << argv[0] << " :" << (ok?"ok":"FAILED") << endl;
  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}
#endif // REGRESSION_TEST
