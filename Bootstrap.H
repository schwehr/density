#ifndef _BOOTSTRAP_H_
#define _BOOTSTRAP_H_

#include <gsl/gsl_rng.h>
#include <cstdio>

#include <vector>

#include "kdsPmagL.H"

size_t BootstrapParametricSample(const std::vector<SVec> &s, const std::vector<float> &sigmas,
				 SVec &newSample,   gsl_rng *r);
size_t BootstrapParametricSite  (const std::vector<SVec> &s, const float sigma, 
				 SVec &newSample,   gsl_rng *r);

template <class T>
T getDevRandom(T &randomSample) {
    FILE * devRandom = fopen ("/dev/random", "r");
    assert (devRandom);
    fread (&randomSample,sizeof(T),1,devRandom);
    fclose (devRandom);
    return (randomSample);
}

#endif // _BOOTSTRAP_H_
