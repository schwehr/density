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

#include <gsl/gsl_rng.h>
#include <cstdio>

#include <vector>

#include "kdsPmagL.H"

void Print(const SVec &sv);

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
