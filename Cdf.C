// $Revision$  $Author$  $Date$

/// \brief Make a CDF from a list of values


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
#include "Cdf.H"

using namespace std;

/***************************************************************************
 * LOCAL MACROS and DEFINES
 ***************************************************************************/

#ifdef __GNUC__
#define UNUSED __attribute((__unused__))
#else
/*! \def UNUSED
  \brief GNU CC attribute to denote unused paramters in function calls.
  The attribute remove compiler warning for unused arguments and variable.  Only works
  for GNU compilers such as gcc and g++.
*/
#define UNUSED
#endif

#ifdef REGRESSION_TEST
#define FAILED_HERE cout <<  __FILE__ << ":" << __LINE__ << " test failed" << endl
#else
/*! \def FAILED_HERE
  \brief Used FAILED_HERE; to emit a string that looks like a compiler warning
  Use this to allow emacs to jump to this source line with C-x `
*/
#define FAILED_HERE // Empty
#endif

/***************************************************************************
 * GLOBALS
 ***************************************************************************/

/// Let the debugger find out which version is being used.
static const UNUSED char* RCSid ="@(#) $Id$";

//####################################################################
// CDF FUNCTIONS
//####################################################################
#if 0
size_t
cdf(const std::vector<size_t> &data, std::vector<size_t> &value, std::vector<float> percent)
{
  

}
#endif

//####################################################################
// CDF Methods
//####################################################################


// Need to optimize more
// This is for smaller data sets where it is ok to copy the data.
// Won't work well for huge vectors where we run out of RAM and have to swap
Cdf::Cdf(const vector<size_t> &data, bool countZeros) {
  vector<size_t> d(data);
  sort(d.begin(),d.end());

  //float cumulativePercent=0.f;
  size_t curVal;
  size_t start;
  size_t count=1;
  if (countZeros) {
    curVal= d[0];
    start=1;
  } else {
    // Find first non-zero value
    size_t i;
    for (i=0;i<d.size() && 0==d[i];i++);
    if (i>=d.size()) return; // whoops... no non-zero data
    curVal=d[i];
    start=i;
  }
  // make sure to not loop on i==0;
  for (size_t i=start;i<d.size();i++) {
    //cout << data[i] << " " << i/float(data.size()) << endl;
    if (curVal!=d[i]) {
      value.push_back(curVal);
      percent.push_back(float(i)/d.size());
      curVal=d[i];
      count=0; // Is count actually used?
    } else {
      count++;
    }
  }
  value.push_back(d[d.size()-1]);
  percent.push_back(1.0);
} // Cdf::Cdf


float Cdf::getCDF(const size_t val) {
  size_t offset;
  for (offset=0;offset<value.size() && val>value[offset];offset++) {
  }
  if (offset>=value.size()) return (1.f);
  if (val==value[offset]) return (percent[offset]);
  if (0==offset && val < value[offset]) return (0.f);
  if (val < value[offset]) return (value[offset-1]);
  assert(false); // Should not be able to reach here
  return(0);
}

float Cdf::getCDF(const size_t val, float &bottom, float &top) {
  float result;
  size_t offset;
  for (offset=0;offset<value.size() && val>value[offset];offset++);
  if (offset>=value.size()) result=1.f;
  else if (val==value[offset]) result=percent[offset];
  else if (0==offset && val < value[offset]) result = 0.f;
  else if (val < value[offset]) result = value[offset-1];
  else {assert(false);}

  if (0==offset) {bottom=0.; top=result;}
  else {
    top=result;
    bottom=percent[offset-1];
  }

  return(result);
}


void Cdf::print() {
  for (size_t i=0;i<value.size();i++) {
    cout << value[i] << " " << percent[i] << endl;
  }
}

void Cdf::writeForGraphing(const string &filename) {
  ofstream o(filename.c_str(),ios::out);
  o << "0 0" << endl;
  for (size_t i=0;i<value.size();i++) {
    if (0!=i) o << value[i-1] << " " << percent[i] << endl;
    o << value[i] << " " << percent[i] << endl;
  }
  //o << value[value.size()-1] << " " << 1.0 <<endl;
  //o << value[value.size()-1]+1 << " " << 1.0 << endl;
}


//####################################################################
// TEST CODE
//####################################################################
#ifdef REGRESSION_TEST

static bool
isEqual (const float a, const float b, const float del) {
  return ( ( a<b+del && a > b-del) ? true : false );
}

void print(const vector<size_t> &v) {
  for (vector<size_t>::const_iterator i=v.begin();i!=v.end();i++)
    cout << *i << endl;
}

bool test1() {
  cout << "      test1" << endl;
  vector<size_t> t;
  for(size_t i=0;i<20;i++) t.push_back(i);
  for(size_t i=0;i<10;i++) t.push_back(10);
  for(size_t i=0;i<12;i++) t.push_back(11);
  for(size_t i=0;i<10;i++) t.push_back(13);
  //print(t);

  Cdf cdf(t,true);
  cdf.print();
  cdf.writeForGraphing(string("test1.cdf"));

  if (!isEqual(0.1153,cdf.getCDF(5), 0.001)) {FAILED_HERE; return(false);}
  {
    float top,bot;
    if (!isEqual(0.1153,cdf.getCDF(5,bot,top), 0.001)) {FAILED_HERE; return(false);}
    if (!isEqual(0.09615, bot, 0.001)) {FAILED_HERE; return(false);}
    if (!isEqual(0.11538, top, 0.001)) {FAILED_HERE; return(false);}

    if (!isEqual(0.01923,cdf.getCDF(0,bot,top), 0.001)) {FAILED_HERE; return(false);}
    if (!isEqual(0.00000, bot, 0.0001)) {FAILED_HERE; return(false);}
    if (!isEqual(0.01923, top, 0.0001)) {FAILED_HERE; return(false);}

    if (!isEqual(1.00000,cdf.getCDF(19,bot,top), 0.001)) {FAILED_HERE; return(false);}
    if (!isEqual(0.98077, bot, 0.0001)) {FAILED_HERE; return(false);}
    if (!isEqual(1.00000, top, 0.0001)) {FAILED_HERE; return(false);}

  }

  return (true);
}

int main (UNUSED int argc, char *argv[]) {
  // Put test code here
  bool ok=true;

  if (!test1()) {FAILED_HERE;ok=false;}

  cout << "  " << argv[0] << " test:  " << (ok?"ok":"failed")<<endl;
  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}
#endif // REGRESSION_TEST
