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

/// \brief Help make color maps
///
/// Want to be able to read things similiar to GMT Color Pallet Files
/// (cpt).  For example: /sw/share/gmt/cpt/GMT_cool.cpt.
///
/// You CPT files need to go from [0..1]

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

// C includes
#include <cassert>

#include <cstdlib>
#include <cstdio>

// C++ includes
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream> // string stream

// STL Includes
#include <string>
#include <vector>

// Libraries
#include <Inventor/SbColor.h> // for HSV to RGB conversion


// Local includes
#include "volmakecmap_cmd.h"

using namespace std;

/***************************************************************************
 * MACROS, DEFINES, GLOBALS
 ***************************************************************************/

#include "debug.H" // provides FAILED_HERE, UNUSED, DebugPrintf
int debug_level=0;

/// Let the debugger find out which version is being used.
static const UNUSED char* RCSid ="$Id$";

/***************************************************************************
 * Local routines
 ***************************************************************************/

/// \brief Volume color pallet table.  Similar to GMT CPT, but has differences

class ColorPallet {
public:
  //ColorPallet();
  /// \parameter ok \a true if load went ok
  ColorPallet(const string &filename, bool &ok);
  /// offset must be between 0 and 255
  bool getRGBA(const size_t offset, float &r, float &g, float &b, float &a) const;
  bool setRGBA(const size_t offset, const float r, const float g, const float b, const float a);

  enum ColorModel { UNKNOWN_CLR_MODEL, ALPHA, LUM_ALPHA, RGBA, HSVA };
  void setColorModel (const ColorModel cm) {colorModel=cm;}
  ColorModel getColorModel () const {return (colorModel);}
private:
  /// Look for a color model = string and set the type
  bool checkColorModel(const string &str);
  bool insertCmapEntry(const float v1, const float r1, const float g1, const float b1, const float a1,
		       const float v2, const float r2, const float g2, const float b2, const float a2);
  vector<float> r,g,b,a;
  ColorModel colorModel;
};

bool ColorPallet::setRGBA(const size_t offset, const float _r, const float _g, const float _b, const float _a) {
  if (256<=offset) return(false);
  assert (HSVA==colorModel || RGBA==colorModel); // FIX: cope with other 2
  r[offset] = _r;
  g[offset] = _g;
  b[offset] = _b;
  a[offset] = _a;
  assert (0<=_r && _r<=1.0);  assert (0<=_g && _g<=1.0);  assert (0<=_b && _b<=1.0);  assert (0<=_a && _a<=1.0);
  return(true);
}

bool ColorPallet::getRGBA(const size_t offset, float &_r, float &_g, float &_b, float &_a) const {
  if (256<=offset) return(false);
  assert (HSVA==colorModel || RGBA==colorModel); // FIX: cope with other 2
  _r = r[offset];
  _g = g[offset];
  _b = b[offset];
  _a = a[offset];
  assert (0<=_r && _r<=1.0);  assert (0<=_g && _g<=1.0);  assert (0<=_b && _b<=1.0);  assert (0<=_a && _a<=1.0);
  return(true);
}

ColorPallet::ColorPallet(const string &filename, bool &ok) {
  ok=true;
  r.resize(256,0.);   g.resize(256,0.);   b.resize(256,0.);   a.resize(256,0.); 
  colorModel = RGBA; // default to RGB unless we see a COLOR_MODEL = HSV

  ifstream in(filename.c_str(),ios::in);
  if (!in.is_open()) {cerr << "file open failed: " << filename << endl; ok=false; return;}

  const size_t bufSize=256;
  char buf[bufSize];
#ifndef NDEBUG
  // FIX: Mac OSX 10.3.5 has no man for memset!
  memset(buf,0,bufSize); // Make life easier in the debugger
#endif


  while (in.getline(buf,bufSize)) {
    if ('#'==buf[0]) continue; // comment
    const string bufStr(buf);
    if (checkColorModel(bufStr)) continue; // Check for COLOR_MODEL = 
    
    float v1,r1,g1,b1,a1,  v2,r2,g2,b2,a2;
    istringstream istr(buf);
    istr >> v1>>r1>>g1>>b1 >>a1 >>v2>>r2>>g2>>b2>>a2;
    if (istr.fail()) {
      cerr << "Read failed for:" << bufStr << endl;
      continue;
    }

    if ( (HSVA!=colorModel) && !( (0.<=v1 && v1 <= 1.) 
	    && (0.<=r1 && r1 <= 1.) && (0.<=g1 && g1 <= 1.) && (0.<=b1 && b1 <= 1.) && (0.<=a1 && a1 <= 1.)
	    && (0.<=v2 && v2 <= 1.) 
	    && (0.<=r2 && r2 <= 1.) && (0.<=g2 && g2 <= 1.) && (0.<=b2 && b2 <= 1.) && (0.<=a2 && a2 <= 1.)
	    ))
      {
	cerr << "WARNING: all values must be between 0.0 and 1.0" << endl << "\t" << bufStr << endl;
	ok=false;
	continue;
      }
    // FIX: check the istr for an error
    if (!insertCmapEntry(v1,r1,g1,b1,a1, v2,r2,g2,b2,a2)) ok=false;
  }
} // ColorPallet(filename) constructor

bool ColorPallet::checkColorModel(const string &str) {
  DebugPrintf(VERBOSE+1,("checkColorModel(%s)\n",str.c_str()));
  if (string::npos == str.find("COLOR_MODEL")) return (false);
  if (string::npos != str.find("ALPHA"))     {setColorModel(ALPHA); DebugPrintf(TRACE,("ALPHA\n"));return (TRUE);}
  if (string::npos != str.find("LUM_ALPHA")) {setColorModel(LUM_ALPHA); DebugPrintf(TRACE,("L_A\n")); return (TRUE);}
  if (string::npos != str.find("RGBA"))      {setColorModel(RGBA); DebugPrintf(TRACE,("RGBA\n")); return (TRUE);}
  if (string::npos != str.find("HSVA"))      {setColorModel(HSVA); DebugPrintf(TRACE,("HSVA\n")); return (TRUE);}
  cerr << "WARNING: Malformed color model.  IgnoringL\t" << str << endl;
  return (false);
}

bool ColorPallet::insertCmapEntry(const float v1, const float r1, const float g1, const float b1, const float a1,
				  const float v2, const float r2, const float g2, const float b2, const float a2)
{
  // FIX: explain better what's wrong
  if (v1>v2) {cerr <<"Bad cmap entry. 0" << endl; return(false);}
  if (v1==v2) {cerr <<"Bad cmap entry. 1" << endl; return(false);}
  if (0>v1 || 1<v1) {cerr <<"Bad cmap entry. 2" << endl; return(false);}
  if (0>v2 || 2<v1) {cerr <<"Bad cmap entry. 3" << endl; return(false);}

  // slopes
  const float mr = (r2-r1)/(v2-v1); const float br = r1 - mr * v1;
  const float mg = (g2-g1)/(v2-v1); const float bg = g1 - mg * v1;
  const float mb = (b2-b1)/(v2-v1); const float bb = b1 - mb * v1;
  const float ma = (a2-a1)/(v2-v1); const float ba = a1 - ma * v1;

  // FIX: better algorithm needed
  for (size_t i;i<256; i++) {
    const float v = i/255.;
    if (v1>v || v2<v) continue;
    r[i] = v * mr + br; if (1.0<r[i]) r[i]=1.f; // Handle floating point jitter
    g[i] = v * mg + bg; if (1.0<g[i]) g[i]=1.f; // Handle floating point jitter
    b[i] = v * mb + bb; if (1.0<b[i]) b[i]=1.f; // Handle floating point jitter
    a[i] = v * ma + ba; if (1.0<a[i]) a[i]=1.f; // Handle floating point jitter
    
  }

  return(true);
}


/***************************************************************************
 * MAIN
 ***************************************************************************/

int main (int argc, char *argv[]) {
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
  DebugPrintf(TRACE,("Debug level = %d\n",debug_level));
#endif

  //////////////////////////////////////////////////////////////////////
  // ERROR CHECK ARGS
  //////////////////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////////////////////
  // GET TO WORK YOU SLACKER
  //////////////////////////////////////////////////////////////////////

  ofstream o(a.out_arg,ios::out);
  if (!o.is_open()) {cerr << "Failed to open output file: " << a.out_arg << endl; return (EXIT_FAILURE);}

  if (a.cpt_given) {
    bool r;
    ColorPallet cpt(string(a.cpt_arg),r);
    if (!r) {ok=false; cerr << "NOT writing output file do to bad color pallet" << endl;}
    else {
      if (a.zero_given) {
	for (size_t i=0;i<size_t(a.zero_given);i++) {
	  if (!(0<=a.zero_arg[i] && a.zero_arg[i]<=255)) {
	    cerr << "WARNING: zero index must be in the range of 0..255.  You gave: " << a.zero_arg[i] << endl;
	    continue;
	  }
	  cpt.setRGBA(a.zero_arg[i],0.f,0.f,0.f,0.f);
	}
      }


      for (size_t i=0;i<256;i++) {
	float r,g,b,a;
	if (cpt.getRGBA(i,r,g,b,a)) {
	  o << r << " " << g << " " << b << " " << a << endl;
	} else {
	  cerr << "Entry ERROR for color table entry: " << i << endl;
	}
      }
    } // if r
  }

  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}

