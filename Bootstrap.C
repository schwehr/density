// $Revision$  $Author$  $Date$
/*
    Bootstrap resampling
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

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h> // gaussian distributed random number generator

#include <iostream>
#include <iomanip>
#include <fstream>

#include "Bootstrap.H"

using namespace std;

/***************************************************************************
 * MACROS, DEFINES, GLOBALS
 ***************************************************************************/

#include "debug.H" // provides FAILED_HERE, UNUSED, DebugPrintf

/// Let the debugger find out which version is being used.
static const UNUSED char* RCSid ="@(#) $Id$";

/***************************************************************************
 * FUNCTIONS
 ***************************************************************************/

void Print(const SVec &sv) {
  assert(6==sv.size());
  for(size_t i=0;i<sv.size();i++) cout << sv[i] << " ";
}

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
    newSample[i] = s[sampleNum][i] + delta;
  }

  // Must normalize
  {
    const float trace=newSample[0]+newSample[1]+newSample[2];
    for (size_t i=0;i<6;i++) newSample[i]/=trace;
  }
  return (sampleNum);
}

// FIX: should this be exposed or static?
//float Trace(const SVec &s) {
//  assert (3<s.size());
//  return (s[0]+s[1]+s[2]);
//}


// returns the sample index that was picked for this iteration
size_t
BootstrapParametricSite(const vector<SVec> &s, const float sigma, //const vector<float> &sigmas,
		    SVec &newSample,   gsl_rng *r) {
  assert (s.size()>0);
  assert (0<=sigma && sigma<1); // FIX: sigma should in the range of  0.000 to 0.001, right?
  assert (r);
  if (6!=newSample.size()) newSample.resize(6);
  const int sampleNum = int(gsl_rng_uniform(r)*s.size());

  const SVec sample=s[sampleNum];

  for (size_t i=0;i<6;i++) {
    const float delta = sigma * gsl_ran_gaussian (r, 1.0);
    newSample[i] = sample[i] + delta;
  }

  // must normalize each sample!
  {
    const float trace=newSample[0]+newSample[1]+newSample[2];
    for (size_t i=0;i<6;i++) newSample[i]/=trace;
  }

  return (sampleNum);
}
