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
/// \brief Help turn 1D data stream into a stream suitable a gnuplot histogram

#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

int main (int argc, char *argv[]) {
  if (3!=argc) {
    cerr << "USAGE:" << endl << endl
	 << "\t" << argv[0] << " filename numBins" << endl;
    return (EXIT_FAILURE);
  }
  string filename(argv[1]);
  const size_t numBins(atoi(argv[2]));
  assert (100000>numBins);

  vector<float> data;
  {
    ifstream in(filename.c_str(),ios::in);
    if (!bool(in)) {cerr << "failed to open file: " << filename << endl; exit(EXIT_FAILURE); }
    float tmp;  while(in >> tmp) {data.push_back(tmp);}
  }

  cerr << "data size: " << data.size() << endl;
  assert (data.size()>0);

  sort(data.begin(),data.end());

  const float binSize = (*(data.end()-1) - *data.begin()) / numBins;

  cout << "# Kurt's histogram tool" << endl
       << "# number of bins: " << numBins << endl
       << "# bin size: " << binSize << endl
       << "# " << endl
       << "# gnuplot command:" << endl
       << "# " << endl
       << "#      plot '" << filename << "' width lines" << endl
       << "# " << endl
       << "# Note: this could be done with step or fstep too" << endl
       << endl;

  size_t bin=0;
  size_t count=0; // number of samples in the bin
  size_t binStart=0;

  cout << data[0] << " " << 0 << endl;
  for (size_t i=0;i<data.size();i++,count++) {
    const size_t curBin = int((data[i]-data[0])/binSize);
    if (curBin!=bin) {
      cout << data[binStart] << " " << count << endl;
      cout << data[i] << " " << count << endl;
      bin=curBin;
      count=0;
      binStart=i;
    }
  }
  cout << *(data.end()-1) << " " << 0 << endl;
  cout << "# Max value: " << *(data.end()-1) << endl;

  return (EXIT_SUCCESS);
}
