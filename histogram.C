//#include <stdlib.h>

#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

int main (int argc, char *argv[]) {
  assert(3==argc);
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
