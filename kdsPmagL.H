#ifndef _KDS_PMAG_L_H_
#define _KDS_PMAG_L_H_

#include <vector>
typedef std::vector<float> SVec; // should always size()==6

#ifdef __GNUC__
#define UNUSED __attribute((__unused__))
#else
#define UNUSED
#endif

// Nice way to say where the test failed... takes emacs right to the problem
#ifdef REGRESSION_TEST
#define FAILED_HERE cout <<  __FILE__ << ":" << __LINE__ << " test failed" << endl
#else
#define FAILED_HERE // Empty
#endif

#endif
