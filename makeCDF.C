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
