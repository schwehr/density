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
/// \file 
/// \brief Create wrapper iv files for volume density files.




/***************************************************************************
 * INCLUDES
 ***************************************************************************/

#include <cassert>

#include <cstdlib>
#include <cstdio>

// C++ includes
#include <iostream>
#include <iomanip>
#include <fstream>

#include <string>	// Good STL data types.
#include <vector>

#include "vol_iv_cmd.h"

using namespace std;

/***************************************************************************
 * MACROS, DEFINES, GLOBALS
 ***************************************************************************/

#include "debug.H" // provides FAILED_HERE, UNUSED, DebugPrintf
#ifndef NDEBUG
int debug_level;
#endif

/// Let the debugger find out which version is being used.
static const UNUSED char* RCSid ="@(#) $Id$";

/***************************************************************************
 * LOCAL FUNCTIONS
 ***************************************************************************/

#include <VolumeViz/nodes/SoTransferFunction.h> // ColorMapType

/// \brief Return the PredefColorMap from user input.  Defaults to NONE
///
SoTransferFunction::PredefColorMap
getPredefCmap (const int given, const char *cstr, bool &ok) {
  ok=true;
  if (!given) return SoTransferFunction::NONE;
  assert(cstr);
  string str(cstr);
  if (0==str.compare("NONE"))        {DebugPrintf(BOMBASTIC,("NONE\n"));        return(SoTransferFunction::NONE);}
  if (0==str.compare("GREY"))        {DebugPrintf(BOMBASTIC,("GREY\n"));        return(SoTransferFunction::GREY);}
  if (0==str.compare("GRAY"))        {DebugPrintf(BOMBASTIC,("GRAY\n"));        return(SoTransferFunction::GRAY);}
  if (0==str.compare("TEMPERATURE")) {DebugPrintf(BOMBASTIC,("TEMPERATURE\n")); return(SoTransferFunction::TEMPERATURE);}
  if (0==str.compare("PHYSICS"))     {DebugPrintf(BOMBASTIC,("PHSYICS\n"));     return(SoTransferFunction::PHYSICS);}
  if (0==str.compare("STANDARD"))    {DebugPrintf(BOMBASTIC,("STANDARD\n"));    return(SoTransferFunction::STANDARD);}
  if (0==str.compare("GLOW"))        {DebugPrintf(BOMBASTIC,("GLOW\n"));        return(SoTransferFunction::GLOW);}
  if (0==str.compare("BLUE_RED"))    {DebugPrintf(BOMBASTIC,("BLUE_RED\n"));    return(SoTransferFunction::BLUE_RED);}
  if (0==str.compare("SEISMIC"))     {DebugPrintf(BOMBASTIC,("SEISMIC\n"));     return(SoTransferFunction::SEISMIC);}
  cerr << "ERROR: unknown PredefColorMap... " << str << endl;
  ok=false;
  return(SoTransferFunction::NONE);
}

/// \brief Parse a string to find out what type of colormap.  Defaults to RGBA.
/// \param cmaptype_given Bool of if the user specified anything
/// \param cmaptype_arg What the user tried to tell us
/// \param ok \a false something cookoo happened... very likely with user typos
/// \return SoTransferFunction::ColorMapType of ALPHA, LUM_ALPHA, or RGBA
SoTransferFunction::ColorMapType
getCmapType(const int cmaptype_given, const char *cmaptype_arg, bool &ok) {
  DebugPrintf(TRACE,("getCmapType: %d\n",int(cmaptype_given)));
  ok=true;
  if (!cmaptype_given) return (SoTransferFunction::RGBA); // FIX: should this be the default?
  assert(cmaptype_arg);
  string str(cmaptype_arg);
  if (0==str.compare("ALPHA")) {DebugPrintf(VERBOSE,("ALPHA\n"));return (SoTransferFunction::ALPHA);}
  if (0==str.compare("LUM_ALPHA")) return (SoTransferFunction::LUM_ALPHA);
  if (0==str.compare("RGBA")) return (SoTransferFunction::RGBA);
  cerr << "ERROR: unknown colormap type string: " << str << endl;
  ok=false;
  return (SoTransferFunction::RGBA);
}

/// \brief Read in a color map and check if it is ok
/// \param o Output stream to write color map to
/// \param filename File to read the color map in from
/// \param cmaptype What type of color map do we have?  They have different number of columns
/// \return \a false if there is trouble 
bool WriteColorMap(ofstream &o, const string &filename, SoTransferFunction::ColorMapType cmaptype)  {
  DebugPrintf(TRACE,("WriteColorMap: %s %d\n",filename.c_str(), int(cmaptype)));
  ifstream in(filename.c_str(), ios::in);
  if (!in.is_open()) {cerr<<"ERROR: unable to open color map... "<<filename<<endl;return(false); }

  o << setprecision(4);

  switch (cmaptype) {
  case SoTransferFunction::ALPHA:
    {
      float tmp;      size_t count=0;
      while(in >> tmp) {
	if (256<=count) {cerr << "WARNING!  Too many color map entries!"<<endl; break;}
	o << "\t\t\t"<<tmp<<","<<endl;
	count++;
      }
      if (count!=256) {
	cerr << "WARNING! Not exactly 256 ALPHA values."<<endl;
      } // count!=256
    }
    break;
  case SoTransferFunction::LUM_ALPHA:
    {
      float a,b;      size_t count=0;
      while(in >> a >> b) {
	if (256<=count) {cerr << "WARNING!  Too many color map entries!"<<endl; break;}
	o << "\t\t\t" << a << "," << b << "," << endl;
	count++;
      }
      if (count!=256) {cerr << "WARNING! Not exactly 256 LUM/ALPHA pairs."<<endl;}
    }
    break;
  case SoTransferFunction::RGBA:
    {
#ifndef NDEBUG
      vector<float> av,bv,cv,dv;
#endif
      float a,b,c,d;      size_t count=0;
      while(in >> a >> b >> c >> d) {
	if (256<=count) {cerr << "WARNING!  Too many color map entries!"<<endl; break;}
	o << "\t\t\t" << a << "," << b << "," << c << "," << d << "," << endl;
	count++;
#ifndef NDEBUG
	av.push_back(a); bv.push_back(b); cv.push_back(c); dv.push_back(d);
#endif
      }
      if (count!=256) {cerr << "WARNING! Not exactly 256 RGBA values - got "<<count<<endl;
#ifndef NDEBUG
      if (debug_level >= BOMBASTIC-1) {
	cerr << "Found sizes:" << av.size() << " " << dv.size()<<endl;
	for (size_t i=0;i<av.size();i++)
	  cerr << "i: " << av[i] << " " << bv[i] << " " << cv[i] << " " << dv[i] << endl;
      }
#endif
      }
    }
    break;
  default: assert(false && "Visiting Davey Jones' Locker... http://dusk.geo.orst.edu/djl/");
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
  DebugPrintf(TRACE,("%s Debug level = %d\n",argv[0],debug_level));
#endif

  //////////////////////////////////////////////////////////////////////
  // ERROR CHECK ARGS
  //////////////////////////////////////////////////////////////////////

  if (1!=a.inputs_num) {
    cerr << "ERROR: Must specify either 0 or 1 inputfile.  You gave 2 or more!" << endl;
    cerr << "  Files you specified (might be bad args)" << endl;
    for (size_t i=0; i< a.inputs_num; i++)
      cerr << i << ": " << a.inputs[i] << endl;

    return (EXIT_FAILURE);
  }

  {
    bool r;
    getPredefCmap(a.predefcmap_given, a.predefcmap_arg, r);
    if (!r) {cerr << "ERROR: bad predefined color map.  Bye." << endl; return(EXIT_FAILURE);}
  }

  {
    bool r;
    getCmapType(a.cmaptype_given, a.cmaptype_arg,r);
    if (!r) {cerr << "ERROR: bad color map type.  Bye." << endl; return(EXIT_FAILURE);}
  }


  //////////////////////////////////////////////////////////////////////
  // GO SPEED RACER
  //////////////////////////////////////////////////////////////////////

  ofstream o(a.out_arg,ios::out);
  if (!o.is_open()) {
    cerr << "ERROR: failed to open output file!" << endl;
    return (EXIT_FAILURE);
  }

  o << "#Inventor V2.1 ascii" << endl << endl
    << "# Voleon IV wrapper file by: $Id$ " << endl;

  o << "# cmdline: ";
  for (int i=0;i < argc;i++) o << argv[i] << " ";
  o << endl << endl;


  {
    o << "Separator { " << endl;

    if (a.scale_given) o << "\tScale { scaleFactor " 
			 << a.scale_arg << " " << a.scale_arg << " " << a.scale_arg
			 << "} " << endl;

    if (a.box_given) {
      DebugPrintf(TRACE,("Doing wireframe box: %f\n",a.box_arg));
      o << "\tSeparator {" << endl
	<< "\t\tDrawStyle { style LINES }" << endl
	<< "\t\tPickStyle { style UNPICKABLE }" << endl
	<< "\t\tCube { width "<<a.box_arg<<" height "<<a.box_arg<<" depth "<<a.box_arg<<"}" << endl
	<< "\t}" << endl;
    }
    o << "\tSoVolumeData {" << endl
      << "\t\tfileName \"" << a.inputs[0] << "\"" << endl
      << "\t}" <<endl
      ;


    o << "\tSoTransferFunction {" << endl;
    if(a.predefcmap_given) o << "\t\tpredefColorMap " << a.predefcmap_arg << endl;
    if(a.cmaptype_given) o << "\t\tcolorMapType " << a.cmaptype_arg << endl;
    if(a.cmap_given) {
      bool r;
      if (SoTransferFunction::NONE != getPredefCmap(a.predefcmap_given, a.predefcmap_arg, r)) 
	cerr << "WARNING: you specified a predefined color map with a color map.  Are you crazy?" << endl;
      DebugPrintf(TRACE,("cmap_given\n"));
      o << "\t\tcolorMap [ ";
      const SoTransferFunction::ColorMapType cmaptype = getCmapType(a.cmaptype_given, a.cmaptype_arg,r);
      if (r && !WriteColorMap(o,string(a.cmap_arg),cmaptype)) {
	ok=false; cerr << "ERROR: Failed to write color map" << endl;
      }
      o << "\t\t]" << endl;
    }
    o << "\t}" << endl;

    // file:///sw/share/SIMVoleon/html/classSoVolumeRender.html
    o << "\tSoVolumeRender {" << endl;
    if (a.interpolation_given)    o << "\t\tinterpolation "    << a.interpolation_arg << endl;
    if (a.composition_given)      o << "\t\tcomposition "      << a.composition_arg << endl;
    if (a.numslicescontrol_given) o << "\t\tnumSlicesControl " << a.numslicescontrol_arg << endl;
    if (a.numslices_given)        o << "\t\tnumSlices "        << a.numslices_arg << endl;
    o << "\t}" << endl;

    o << "} # End of safety separator" << endl;
  }
  o << endl << "# EOF ( " << a.out_arg << " )" << endl;


  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}
