// $Revision$  $Author$  $Date$
/*
    Copyright (C) 2004  Kurt Schwehr

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

/// \file
/// \brief Command line program to do paleomag s data processing into xyz.



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
#include "VecAngle.H" // ldi2xyz() and xyz2tpr()

using namespace std;


/***************************************************************************
 * LOCAL MACROS and DEFINES
 ***************************************************************************/

#include "debug.H" // provides FAILED_HERE, UNUSED, DebugPrintf
//#ifndef NDEBUG
// Can be used even in non-debug mode for huge runs
int debug_level;
//#endif

/***************************************************************************
 * GLOBALS
 ***************************************************************************/

/// Let the debugger find out which version is being used.
static const UNUSED char* RCSid ="@(#) $Id$";

/***************************************************************************
 * LOCAL FUNCTIONS
 ***************************************************************************/



/// \brief load ascii whitespace delimited text into vectors.
/// \return \a true if all went well.  \a false if trouble of any kind
/// \param filename File to open and read data from
/// \param s Return vector of data.  The \a s diagonalized matrix parameters.  See s_eigs
/// \param sigmas Return vector of sigma errors
///
/// Unlike Lisa's code, this one does NOT alter the sigmas on loading
/// which is what the adread subroutine did.`
/// You must call SiteSigma if doing a Site based Parametric Bootstrap
bool
LoadS(const string filename,vector <SVec> &s,vector<float> &sigmas) {
  ifstream in(filename.c_str(),ios::in);
  if (!bool(in)) {cerr << "failed to open file: " << filename << endl; return false; }

  // FIX: detect formats -  {s[6]}, {s[6],sigma}, {name, sigma, s[6]}
  // FIX: only do {s[6],sigma} for now
  SVec tmp(6,0.);  float tmpSigma;
  while (in >> tmp[0] >> tmp[1] >> tmp[2] >> tmp[3] >> tmp[4] >> tmp[5] >> tmpSigma) {
    s.push_back(tmp);  
    sigmas.push_back(tmpSigma);
  }
  // FIX: do we need to normalize so that the trace is 1?
  // Not all data will have a trace==1??
  return (true);
}

//////////////////////////////////////////////////////////////////////
// MAIN
//////////////////////////////////////////////////////////////////////
#ifndef REGRESSION_TEST  // NOT testing

/// \brief What kind of boot strap to do?
///
/// \a Site parametic looks at all the data and uses the Hext method to
/// create one sigma that is applied to all samples.
///
/// \a Sample parametric uses the sigma at the end of each sample
/// (possition 7) to make a new sample that particular measurement.

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

/// \brief Actually do the boot strap
/// \param inFiles vector of files to read in and bootstrap
/// \param out1Max, out2Int, out3Min Each of the streams to write to.  vmax,
/// vint, vmin.  Make them all the same and set oneFile true to get
/// one file will all 9 parameters
/// \param numout_arg 1 if out1, out2, and out3 are all the same file, otherwise should be 3
/// \param format How do we want the output to look.  (S, XYZ, other some other day)
/// \param type PARAMETRIC_SITE or PARAMETRIC_SAMPLE
/// \param draw How many sample to draw out of the magic hat
bool DoS_Bootstrap(const vector<string> &inFiles,
		   ofstream &out1Max, ofstream &out2Int, ofstream &out3Min,
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
    DebugPrintf (TRACE,("Reading file: %s\n",inFiles[i].c_str()));
    s.clear(); sigmas.clear();
    if (!LoadS(inFiles[i],s,sigmas)) {
      cerr << "ERROR - can't load datafile, skipping: " << inFiles[i] << endl;
      ok=false; continue;
    }

    const float siteSigma = (SITE_PARAMETRIC==type)?SiteSigma(s):-666.;

    SVec newSample(6,0.);
    out1Max << setiosflags(ios::fixed) << setprecision(10);
    out2Int << setiosflags(ios::fixed) << setprecision(10);
    out3Min << setiosflags(ios::fixed) << setprecision(10);

    // Draw out and bootstrap 'draw' number of samples
    for (size_t i=0;i<size_t(draw); i++) {
      if (10<=debug_level) { if (0==i%100000) cout << i <<" "<<i/float(draw) << endl; }
      else if (4<=debug_level) { if (0==i%1000000) cout << i <<" "<<i/float(draw) << endl; }

      switch (type) {
      case SITE_PARAMETRIC:   BootstrapParametricSite   (s,siteSigma,newSample, r); break;
      case SAMPLE_PARAMETRIC: BootstrapParametricSample (s,sigmas   ,newSample, r); break;
      default:	assert(false);
      }
      // Now what do we do with the new sample?
      switch(format) {
      case S_FORMAT:
	for (size_t i=0;i<newSample.size();i++) out1Max << newSample[i] << " ";
	break;
      //case PTR_FORMAT: break;  NOT SUPPORTED YET
      case XYZ_FORMAT:
	{
	  sengine.setS(newSample);
	  sengine.getXYZ(KMIN, Vmin);
	  sengine.getXYZ(KINT, Vint);
	  sengine.getXYZ(KMAX, Vmax);
	  if (1==numout_arg) {
	    for (size_t i=0;i<3;i++) out3Min << Vmin[i] << " ";  // V3
	    for (size_t i=0;i<3;i++) out2Int << Vint[i] << " ";  // V2
	    for (size_t i=0;i<3;i++) out1Max << Vmax[i] << " ";  // V1
	  } else {
	    for (size_t i=0;i<3;i++){out1Max<<Vmax[i]<<" "; out2Int<<Vint[i]<<" "; out3Min<<Vmin[i]<<" ";}
	  }
	} // case XYZ
	break;
      default: assert(false && "Hell in a hand basket");
      }

      switch (numout_arg) {
      case 1: out1Max << "\n"; break;
      case 3: out1Max << "\n"; out2Int << "\n"; out3Min << "\n"; break;
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

  //#ifdef NDEBUG
  //if (a.verbosity_given) {
  //  cerr << "Verbosity is totally ignored for optimized code.  Continuing in silent mode" << endl;
  //}
  //#else // debugging
  debug_level = a.verbosity_arg; // now used even in OPTIMIZE mode
  DebugPrintf(TRACE,("Debug level = %d\n",debug_level));
  //#endif

  vector<string> inFiles;
  if (0==a.inputs_num) {cerr << "ERROR: must specify at least one input file" << endl; return(EXIT_FAILURE);}
  if (!GetFiles(a.inputs_num,a.inputs,inFiles)) {cerr << "Doh!  What happened?" << endl; return(EXIT_FAILURE);}

  if (1!=a.numout_arg && 3!=a.numout_arg) {
    cerr << "ERROR: numout must be either 1 or 3" << endl
	 << "   found: " << a.numout_arg << endl;
    return(EXIT_FAILURE);
  }

  const FormatEnum format = GetFormat(a.format_arg);
  DebugPrintf(VERBOSE,("format: %s (%d)\n",a.format_arg,int(format)));

  // Can't have S_FORMAT and 3 outfiles.  Does not make sense!!!
  if (3==a.numout_arg && S_FORMAT==format) {
    cerr << "ERROR:  you can't have 3 output files with the S_FORMAT." << endl
	 << "  That would be ludicrous" << endl;
    return (EXIT_FAILURE);
  }


  const BootTypeEnum type = GetParametricType(a.site_given,a.sample_given);
#ifndef NDEBUG
  if (debug_level > TRACE)
    cerr << "Bootstrap type: " << (SITE_PARAMETRIC==type?"site":"sample") << " parametric"<<endl;
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
    const string o1NameMax(string(a.out_arg)+string("1.vmax"));
    const string o2NameInt(string(a.out_arg)+string("2.vint"));
    const string o3NameMin(string(a.out_arg)+string("3.vmin"));

    ofstream o1Max(o1NameMax.c_str(),ios::out);
    ofstream o2Int(o2NameInt.c_str(),ios::out);
    ofstream o3Min(o3NameMin.c_str(),ios::out);

    if (!o1Max.is_open() || !o2Int.is_open() || !o3Min.is_open() ) ok=false;
    if (ok && !DoS_Bootstrap(inFiles, o1Max,o2Int,o3Min, a.numout_arg, format, type, a.draw_arg)) {
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
