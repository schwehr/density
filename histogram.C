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
/// \brief Help turn 1D data stream into a stream suitable a gnuplot histogram

#include <vector>
#include <iostream>
#include <fstream>

// local includes
#include "histogram_cmd.h"

using namespace std;

int main (int argc, char *argv[]) {
  gengetopt_args_info a;  // a == args
  if (0!=cmdline_parser(argc,argv,&a)){cerr<<"MELT DOWN: should never get here"<<endl;return (EXIT_FAILURE);}

  //string filename(argv[1]);
  //const size_t numBins(atoi(argv[2]));
  assert (100000>a.bins_arg);
  assert (0<a.bins_arg);

  vector<float> data;
  {
    for (size_t i=0;i<a.inputs_num;i++) {
      string filename(a.inputs[i]);
      cout << "Reading file: " << filename << endl;
      ifstream in(filename.c_str(),ios::in);
      if (!bool(in)) {cerr << "failed to open file: " << filename << endl; exit(EXIT_FAILURE); }
      float tmp;  while(in >> tmp) {data.push_back(tmp);}
    }
  }

  cerr << "data size: " << data.size() << endl;
  assert (data.size()>0);

  sort(data.begin(),data.end());

  const float min=(a.min_given?a.min_arg:*data.begin());
  const float max=(a.max_given?a.max_arg:*(data.end()-1));
  const float binSize=(max-min)/a.bins_arg;

  ofstream o(a.out_arg,ios::out);
  if (!o.is_open()) {cerr<<"ERROR: unable to open waypoint file: "<<a.out_arg<<endl; return (false);}

  o << "# Kurt's histogram tool" << endl
    << "# number of bins: " << a.bins_arg << endl
    << "# bin size: " << binSize << endl
    << "# " << endl
    << "# gnuplot command:" << endl
    << "# " << endl
    << "#      plot '" << a.out_arg << "' width lines" << endl
    << "# " << endl
    << "# Note: this could be done with step or fstep too" << endl
    << endl;

#if 0
  size_t bin=0;
  size_t count=0; // number of samples in the bin
  size_t binStart=0;
  o << min << " " << 0 << endl;
  for (size_t i=0;i<data.size();i++,count++) {
    if (data[i]<min) continue; // skip data outside the range
    const size_t curBin = int((data[i]-min)/binSize);
    if (curBin!=bin) {
      o << data[binStart] << " " << count << endl;
      o << data[i] << " " << count << endl;
      bin=curBin;
      count=0;
      binStart=i;
    }
  }
  o << max << " " << 0 << endl;
  o << "# Max value: " << *(data.end()-1) << endl;
#endif

  o << min << " " << 0 << endl;
  // FIX: use STL and make this more efficient!
  for (size_t bin=0;bin<size_t(a.bins_arg);bin++) {
    size_t count=0;
    const float binmin = min + binSize * bin;
    const float binmax = min + binSize * (bin+1);
    for (size_t i=0;i<data.size();i++) {
      if (data[i]<=binmin) continue;
      if (data[i]>binmax) break;
      count++;
    }
    o << binmin << " " << count << "\n"
      << binmax << " " << count << endl;
  }
  o << max << " " << 0 << "\n";


  return (EXIT_SUCCESS);
}
