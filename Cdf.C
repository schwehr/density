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
    } else {
      count++;
    }
  }
} // Cdf::Cdf


float Cdf::getCDF(const size_t val) {
  size_t offset;
  for (offset=0;offset<value.size() && val<value[offset];offset++);
  if (offset>=value.size()) return (1.f);
  if (val==value[offset]) return (percent[offset]);
  if (0==offset && val < value[offset]) return (0.f);
  if (val < value[offset]) return (value[offset-1]);
  assert(false); // Should not be able to reach here
  return(0);
}



//####################################################################
// TEST CODE
//####################################################################
#ifdef REGRESSION_TEST

bool test1() {
  cout << "      test1" << endl;

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
