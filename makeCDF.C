//#include <stdlib.h>

#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

int main (int argc, char *argv[]) {
  assert(2==argc);
  string filename(argv[1]);
  vector<float> data;

  {
    ifstream in(filename.c_str(),ios::in);
    if (!bool(in)) {cerr << "failed to open file: " << filename << endl; exit(EXIT_FAILURE); }

    float tmp;
    while(in >> tmp) {data.push_back(tmp);}
  }

  sort(data.begin(),data.end());
#if 0
  for(vector<float>::iterator i=data.begin();i!=data.end();i++) 
    cout << *i << endl;
#endif

  for (size_t i=0;i<data.size();i++) {
    cout << data[i] << " " << i/float(data.size()) << endl;
  }



  return (EXIT_SUCCESS);
}
