
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h> // gaussian distributed random number generator

#include <iostream>
#include <fstream>

#include "Bootstrap.H"

using namespace std;

// returns the sample index that was picked for this iteration
size_t
BootstrapParametricSample(const vector<SVec> &s, const vector<float> &sigmas,
			  SVec &newSample,   gsl_rng *r)
{
  assert (s.size()==sigmas.size());
  assert (r);
  if (6!=newSample.size()) {
#ifndef NDEBUG
    cerr << "warning... resizing new sample" << endl;
#endif
    newSample.resize(6);
  }
  const size_t sampleNum = size_t(gsl_rng_uniform(r)*s.size());
  assert (6==s[sampleNum].size());
  assert (0<=sigmas[sampleNum] && sigmas[sampleNum] <= 1.0); // FIX: Make this just a warning?

  for (size_t i=0;i<6;i++) {
    const float delta = sigmas[sampleNum] * gsl_ran_gaussian (r, 1.0);
    //cout << i << " delta = " << delta << endl;
    newSample[i] = s[sampleNum][i] + delta;
  }
#ifdef REGRESSION_TEST
  cout << sampleNum << " Sample Was: "; Print(s[sampleNum]);
  cout << sampleNum << " Sample Now: "; Print(newSample);
#endif
  return (sampleNum);
}


// returns the sample index that was picked for this iteration
size_t
BootstrapParametricSite(const vector<SVec> &s, const float sigma, //const vector<float> &sigmas,
		    SVec &newSample,   gsl_rng *r) {
  assert (s.size()>0);
  // FIX: make sigma a warning, not an assert
  assert (0<=sigma && sigma<1); // FIX: sigma should in the range of  0.000 to 0.001, right?
  assert (r);
  if (6!=newSample.size()) {
#ifndef NDEBUG
    cerr << "warning... resizing new sample" << endl;
#endif
    newSample.resize(6);
  }
  const int sampleNum = int(gsl_rng_uniform(r)*s.size());

  for (size_t i=0;i<6;i++) {
    const float delta = sigma * gsl_ran_gaussian (r, 1.0);
    //cout << i << " delta = " << delta << endl;
    newSample[i] = s[sampleNum][i] + delta;
  }
#ifdef REGRESSION_TEST
  cout << sampleNum << " Sample Was: "; Print(s[sampleNum]);
  cout << sampleNum << " Sample Now: "; Print(newSample);
#endif
  return (sampleNum);
}
