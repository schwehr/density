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
/// \brief Load a vol voxel file and write it out in some other format


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

// Local includes
#include "VolHeader.H"
#include "volhdr_edit_cmd.h"  // gengetopt command line interface

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

/// \brief using fread and fwrite, copy a file from in to out
///

bool FileCopy (FILE *in, FILE *out, const size_t len) {
  const size_t blockSize=1024;
  DebugPrintf(TRACE,("FileCopy: len=%d blockSize=%d\n",int(len),int(blockSize)));

  assert(in);
  assert(out);
  assert(std::numeric_limits<size_t>::max() != len); // same as -1
  assert(1==sizeof(char));

  size_t cur;
  char buf[blockSize];
  for (cur=0; (cur/blockSize) < (len/blockSize); cur+=blockSize) {
    if (blockSize != fread(buf,sizeof(char),blockSize,in)) { 
      perror ("Failed read data block\n"); return (false);
    }

    DebugPrintf(BOMBASTIC,("writing block: cur=%d\n",int(cur)));
    if (blockSize != fwrite (buf,sizeof(char),blockSize,out)) {
      perror ("Failed to copy data block\n"); return (false);
    }
  }
  const size_t remain=len-cur;
  if ( 0 < remain ) {
    if (remain != fread(buf,sizeof(char),remain,in)) { 
      perror ("Failed read data block\n"); return (false);
    }

    DebugPrintf(BOMBASTIC,("wt last block: cur=%d remain=%d\n",int(cur),int(remain)));
    if (remain!=fwrite(buf,sizeof(char),remain,out)) {
      perror ("Failed to copy tail data block"); return (false);
    }
  }
  return (true);
}

#include <sys/types.h>
#include <sys/stat.h>

/// \brief Add a file on to the end of an existing FILE that we have open
/// \param out FILE that has been opened with fopen (filename,"wb")
/// \param filename Name of the file to tack on the end of out
/// \return \a false if something bad happened
bool AppendFile(FILE *out, const string filename) {
  FILE *in = fopen(filename.c_str(),"rb");
  if (!in) {perror("failed to open file");cerr<<"   "<<filename<<endl;exit(EXIT_FAILURE);}

  struct stat sb;
  {
    int r = stat (filename.c_str(), &sb);
    if (0 != r) {perror("stat to get file size FAILED");return(false);}
  }
  const size_t len=sb.st_size;  
  const size_t blockSize=1024;
  
  size_t cur;
  char buf[blockSize];
  for (cur=0; (cur/blockSize) < (len/blockSize); cur+=blockSize) {
    if (blockSize != fread(buf,sizeof(char),blockSize,in)) { 
      perror ("Failed read data block\n"); return (false);
    }

    DebugPrintf(BOMBASTIC,("writing block: cur=%d\n",int(cur)));
    if (blockSize != fwrite (buf,sizeof(char),blockSize,out)) {
      perror ("Failed to copy data block\n"); return (false);
    }
  }
  const size_t remain=len-cur;
  if ( 0 < remain ) {
    if (remain != fread(buf,sizeof(char),remain,in)) { 
      perror ("Failed read data block\n"); return (false);
    }

    DebugPrintf(BOMBASTIC,("wt last block: cur=%d remain=%d\n",int(cur),int(remain)));
    if (remain!=fwrite(buf,sizeof(char),remain,out)) {
      perror ("Failed to copy tail data block"); return (false);
    }
  }
  return (true);
}

//######################################################################
// MAIN
//######################################################################

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

  if (1<a.inputs_num) {
    cerr << "ERROR: Must specify either 0 or 1 inputfile.  You gave 2 or more!" << endl;
  } else if (1==a.inputs_num) {
    //
    // Use the input file as a template
    //
    bool r;
    DebugPrintf(TRACE,("Loading file\n"));
    VolHeader hdr(string(a.inputs[0]),r);
    if (!r) {cerr << "Unable to open file." <<endl<<"   "<<a.inputs[0]<<endl; return (EXIT_FAILURE); }
    VolHeader hdrOrig(string(a.inputs[0]),r);
    if (!r) {cerr << "Unable to open file." <<endl<<"   "<<a.inputs[0]<<endl; return (EXIT_FAILURE); }
    FILE *orig = fopen(a.inputs[0],"rb");
    if (!orig) {perror("failed to open file");cerr<<"   "<<a.inputs[0]<<endl;exit(EXIT_FAILURE);}

    if (a.magic_given) hdr.setMagicNumber(a.magic_arg);
    if (a.header_len_given) hdr.setHeaderLength(a.header_len_arg);
    if (a.width_given) hdr.setWidth(a.width_arg);
    if (a.tall_given) hdr.setHeight(a.tall_arg);
    if (a.depth_given) hdr.setImages(a.depth_arg);
    if (a.bpv_given) hdr.setBitsPerVoxel(a.bpv_arg);
    if (a.index_bits_given) hdr.setIndexBits(a.index_bits_arg);

    if (a.xscale_given) hdr.setScaleX(a.xscale_arg);
    if (a.yscale_given) hdr.setScaleY(a.yscale_arg);
    if (a.zscale_given) hdr.setScaleZ(a.zscale_arg);

    if (a.xrot_given) hdr.setRotX(a.xrot_arg);
    if (a.yrot_given) hdr.setRotY(a.yrot_arg);
    if (a.zrot_given) hdr.setRotZ(a.zrot_arg);

    FILE *o = fopen(a.out_arg,"wb");
    if (!o) {perror("failed to open output file");cerr << "   " <<a.out_arg<< endl;exit(EXIT_FAILURE);}

    hdr.write(o);
    if (0 != fseek(orig,hdrOrig.getHeaderLength(), SEEK_SET)) {
      perror ("Unable to seek to the data"); exit(EXIT_FAILURE);
    }

    if (!a.nodata_given && !a.data_given) {
      if (!FileCopy (orig, o, hdrOrig.getDataSize())) {
	ok=false; cerr << "ERROR: Unable to copy data from src to dest" << endl;
      }
    }

    if (a.data_given) {
      if (!AppendFile(o,string(a.data_arg)))
	{cerr << "ERROR: Append of data file failed" <<endl; ok=false;}
    }

    if (0!=fclose(orig)) {perror("Failed to close src "); ok=false;}
    if (0!=fclose(o))    {perror("Failed to close dest"); ok=false;}


    
  } else {
    //
    // Start from scratch
    //
    FILE *o = fopen(a.out_arg,"wb");
    if (!o) {perror("failed to open output file");cerr << "   " <<a.out_arg<< endl;exit(EXIT_FAILURE);}

    VolHeader hdr(a.width_arg,a.tall_arg,a.depth_arg,
		  a.bpv_arg,
		  a.xscale_arg,a.yscale_arg,a.zscale_arg,
		  a.xrot_arg,a.yrot_arg,a.zrot_arg);

    if (a.magic_given) hdr.setMagicNumber(a.magic_arg);
    if (a.header_len_given) hdr.setHeaderLength(a.header_len_arg);
    if (a.index_bits_given) hdr.setIndexBits(a.index_bits_arg);

    // FIX: need better error checking
    if(0==hdr.write(o)) {cerr << "ERROR: header write failed" <<endl; ok=false;}

    if (a.data_given) {
      if (!AppendFile(o,string(a.data_arg)))
	{cerr << "ERROR: Append of data file failed" <<endl; ok=false;}
    }

  }


  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}
