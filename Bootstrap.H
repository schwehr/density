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
#ifndef _BOOTSTRAP_H_
#define _BOOTSTRAP_H_

/// \file
/// \brief Header for resampling data points based on their known error.
/// Can do both sample based and parametric (group) based sigmas.


#include <gsl/gsl_rng.h>
#include <cstdio>

#include <vector>

#include "kdsPmagL.H"

/// \brief Print out the values in a 6 or 7 value s diagonal matrix w/ or without sigma
/// \param sv really a vector<float>
void Print(const SVec &sv);

/// \brief Draw a random sample from the raw dataset, but perturbed by sample's sigma
/// \param s All of the s 6 value diagonal matrix.  See k15_s.
/// \param sigmas All the sigmas (7th value) for the s values.  length must be the same as for \a s
/// \param newSample returns the new 6 values
/// \param r the GSL random number generator that we are using
/// \return the index into s that was randomly chosen
///
/// This is not really a bootstrap!  But is can be used to generate
/// bootstraps.  The alorithm here is to randomly select one of the
/// samples in \a s, then the matching simga from \a sigmas.  This
/// selection is done with a gsl_ran_uniform so all samples are
/// equally likely.
///
/// Then for each value in the selected \a s, a delta is created by
/// sigmas[sampleNum] * gsl_ran_gaussian(r,1.0) then added to that s
/// value.  The 6 new s values are then renormalized to have a trace
/// of 1.
///
/// http://www.gnu.org/software/gsl/manual/gsl-ref_19.html#SEC288
///
/// \bug Is this really the right way?  Can only have up to 1 sigma error.  How to allow 2 sigma error.  Or N sigma


size_t BootstrapParametricSample(const std::vector<SVec> &s, const std::vector<float> &sigmas,
				 SVec &newSample,   gsl_rng *r);

/// \brief Draw a random sample from the raw dataset, but perturbed by a single sigma
/// \param s All of the s 6 value diagonal matrix.  See k15_s.
/// \param sigma sigma to apply across the s values in a gaussian distribution.  Usually a Hext site value.  See SiteSigma()
/// \param newSample returns the new 6 values
/// \param r the GSL random number generator that we are using
/// \return the index into s that was randomly chosen
///
/// This is not really a bootstrap!  But is can be used to generate
/// bootstraps.  The alorithm here is to randomly select one of the
/// samples in \a s.  This selection is done with a gsl_ran_uniform so
/// all samples are equally likely.
///
/// Then for each value in the selected \a s, a delta is created by
/// sigma * gsl_ran_gaussian(r,1.0) then added to that s value.  The 6 new s
/// values are then renormalized to have a trace of 1.
///
/// http://www.gnu.org/software/gsl/manual/gsl-ref_19.html#SEC288
///
/// \bug Is this really the right way?  Can only have up to 1 sigma error.  How to allow 2 sigma error.  Or N sigma


size_t BootstrapParametricSite  (const std::vector<SVec> &s, const float sigma, 
				 SVec &newSample,   gsl_rng *r);


/// \brief This is used as a better random seed to pass to the GSL random number engine.
/// \param randomSample Give is a variable to fill.  This function will fill it with who knows what.
/// \return returns the random value.  The arg determines the size.
/// Use the systems /dev/random entropy generator produce a random value for any data type.
/// /dev/random will return how ever many bytes that are requested, so
/// it just uses the size of the caller supplied data type.

template <class T>
T getDevRandom(T &randomSample) {
    FILE * devRandom = fopen ("/dev/random", "r");
    assert (devRandom);
    fread (&randomSample,sizeof(T),1,devRandom);
    fclose (devRandom);
    return (randomSample);
}

#endif // _BOOTSTRAP_H_
