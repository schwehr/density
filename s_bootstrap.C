// $Revision$  $Author$  $Date$
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


/***************************************************************************
 * LOCAL MACROS and DEFINES
 ***************************************************************************/

#include "debug.H" // provides FAILED_HERE, UNUSED, DebugPrintf
#ifndef NDEBUG
int debug_level;
#endif

/***************************************************************************
 * GLOBALS
 ***************************************************************************/

/// Let the debugger find out which version is being used.
static const UNUSED char* RCSid ="@(#) $Id$";

/***************************************************************************
 * LOCAL FUNCTIONS
 ***************************************************************************/


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

enum BootTypeEnum { BAD_PARAMETRIC, SITE_PARAMETRIC, SAMPLE_PARAMETRIC };
enum FormatEnum {BAD_FORMAT, XYZ_FORMAT,TPR_FORMAT,S_FORMAT};
#include "s_bootstrap_cmd.h" // Command line args
#include "Eigs.H" // Let's us convert to other coords

bool GetFiles(const size_t numArg, char **in_arg,vector<string> &inFiles) {
  assert(in_arg);
  assert(*in_arg); // Must be at least 1 file in!
  for (size_t i=0; i<numArg;i++) inFiles.push_back(string(in_arg[i]));
  return (true);
}	

BootTypeEnum GetParametricType(const int site_given, const int sample_given) {
  if (site_given && sample_given) {
    cerr << "ERROR: can not specify both site and sample paramteric bootstrap" <<endl;
    exit (EXIT_FAILURE);
  } else if (site_given) return(SITE_PARAMETRIC);
  return(SAMPLE_PARAMETRIC); // default
}  


FormatEnum GetFormat(const char *format_arg) {
  assert(format_arg);
  string format(format_arg);
  if (0==string("s").compare(format)) return(S_FORMAT);
  if (0==string("tpr").compare(format)) return(TPR_FORMAT);
  if (0==string("xyz").compare(format)) return(XYZ_FORMAT);
  cerr << "ERROR: format must be one of s, tpr, or xyz" << endl;
  exit(EXIT_FAILURE);
  //return(BAD_FORMAT);
}


/// \param oneFile true if out1, out2, and out3 are all the same file
bool DoS_Bootstrap(const vector<string> &inFiles,
		   ofstream &out1, ofstream &out2, ofstream &out3,
		   const int numout_arg, const FormatEnum format, const BootTypeEnum type,
		   const int draw)
{
  bool ok=true;
  assert(1==numout_arg || 3==numout_arg);
  assert(0<draw);

  // Setup random number generator
  gsl_rng * r;  /* global generator */
  const gsl_rng_type *T;
  gsl_rng_env_setup();
  T = gsl_rng_default;
  r = gsl_rng_alloc (T);
  { unsigned long int s;  getDevRandom(s);  gsl_rng_set (r, s); } // Set the Seed

  S_Engine sengine; // For converting to xyz or ptr

  vector<SVec> s;
  vector<float> sigmas;

  vector<float> Vmin(3,0), Vint(3,0), Vmax(3,0); // for xyz or tpr

  for(size_t i=0;i<inFiles.size();i++) {
    cout << "reading file " << inFiles[i] << endl;
    s.clear(); sigmas.clear();
    if (!LoadS(inFiles[i],s,sigmas)) {
      cerr << "ERROR - can't load datafile, skipping: " << inFiles[i] << endl;
      ok=false; continue;
    }

    const float siteSigma = (SITE_PARAMETRIC==type)?SiteSigma(s):-666.;

    SVec newSample(6,0.);
    out1 << setiosflags(ios::fixed) << setprecision(10);
    out2 << setiosflags(ios::fixed) << setprecision(10);
    out3 << setiosflags(ios::fixed) << setprecision(10);

    // Draw out and bootstrap 'draw' number of samples
    for (size_t i=0;i<size_t(draw); i++) {
      switch (type) {
      case SITE_PARAMETRIC:   BootstrapParametricSite   (s,siteSigma,newSample, r); break;
      case SAMPLE_PARAMETRIC: BootstrapParametricSample (s,sigmas   ,newSample, r); break;
      default:	assert(false);
      }
      // Now what do we do with the new sample?
      switch(format) {
      case S_FORMAT:
	for (size_t i=0;i<newSample.size();i++) out1 << newSample[i] << " ";
	break;
      //case PTR_FORMAT: break;  NOT SUPPORTED YET
      case XYZ_FORMAT:
	{
	  sengine.setS(newSample);
	  sengine.getXYZ(KMIN, Vmin);
	  sengine.getXYZ(KINT, Vint);
	  sengine.getXYZ(KMAX, Vmax);
	  if (1==numout_arg) {
	    for (size_t i=0;i<3;i++) out3 << Vmin[i] << " ";  // V3
	    for (size_t i=0;i<3;i++) out2 << Vint[i] << " ";  // V2
	    for (size_t i=0;i<3;i++) out1 << Vmax[i] << " ";  // V1
	  } else {
	    for (size_t i=0;i<3;i++){out1<<Vmin[i]<<" "; out2<<Vint[i]<<" "; out3<<Vmax[i]<<" ";}
	  }
	} // case XYZ
	break;
      default: assert(false && "Hell in a hand basket");
      }

      switch (numout_arg) {
      case 1: out1 << endl; break;
      case 3: out1 << endl; out2 << endl; out3 << endl; break;
      default: assert(false && "What are we gonna do now, man?!?!");
      }
    } // for draws



  } // for inFiles


  return (ok);
}

//////////////////////////////////////////////////////////////////////
// MAIN
//////////////////////////////////////////////////////////////////////

int main (const int argc, char *argv[]) {
  bool ok=true;

  gengetopt_args_info a;
  if (0!=cmdline_parser(argc,argv,&a)) {
    cerr << "FIX: should never get here" << endl;
    cerr << "Early exit" << endl;
    return (EXIT_FAILURE);
  }

#ifdef NDEBUG
  if (a.verbosity_given) {
    cerr << "Verbosity is totally ignored for optimized code.  Continuing in silent mode" << endl;
  }
#else // debugging
  debug_level = a.verbosity_arg;
  DebugPrintf(TRACE,("Debug level = %d",debug_level));
#endif

  vector<string> inFiles;
  if (0==a.inputs_num) {cerr << "ERROR: must specify at least one input file" << endl; return(EXIT_FAILURE);}
  if (!GetFiles(a.inputs_num,a.inputs,inFiles)) {cerr << "Doh!  What happened?" << endl; return(EXIT_FAILURE);}

  if (1!=a.numout_arg && 3!=a.numout_arg) {
    cerr << "ERROR: numout must be either 1 or 3" << endl
	 << "   found: " << a.numout_arg << endl;
    return(EXIT_FAILURE);
  }

  const FormatEnum format = GetFormat(a.format_arg);
  DebugPrintf(VERBOSE,("format: %s (%d)",a.format_arg,int(format)));

  // Can't have S_FORMAT and 3 outfiles.  Does not make sense!!!
  if (3==a.numout_arg && S_FORMAT==format) {
    cerr << "ERROR:  you can't have 3 output files with the S_FORMAT." << endl
	 << "  That would be ludicrous" << endl;
    return (EXIT_FAILURE);
  }


  const BootTypeEnum type = GetParametricType(a.site_given,a.sample_given);
#ifndef NDEBUG
  cerr << "Bootstrap type: " << (SITE_PARAMETRIC==type?"site":"sample") << "parametric"<<endl;
#endif  
  
  if (1==a.numout_arg) {
    // just one file
    ofstream out(a.out_arg,ios::out);
    if (out.is_open()) {
      if (!DoS_Bootstrap(inFiles, out,out,out, a.numout_arg, format, type, a.draw_arg)) {
	ok=false; cerr << "ERROR:  " << argv[0] << " failed in bootstrap routine." << endl;
      }
    } else {ok=false; cerr << "Failed to open output file" << endl;}
  } else {
    // 3 different files.
    const string o1Name(string(a.out_arg)+string("1"));
    const string o2Name(string(a.out_arg)+string("2"));
    const string o3Name(string(a.out_arg)+string("3"));

    ofstream o1(o1Name.c_str(),ios::out);
    ofstream o2(o2Name.c_str(),ios::out);
    ofstream o3(o3Name.c_str(),ios::out);

    if (!o1.is_open() || !o2.is_open() || !o3.is_open() ) ok=false;
    if (ok && !DoS_Bootstrap(inFiles, o1,o2,o3, a.numout_arg, format, type, a.draw_arg)) {
      ok=false; cerr << "ERROR:  " << argv[0] << " failed in bootstrap routine." << endl;
    }
  }

  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
} // main

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
