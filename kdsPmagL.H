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

#ifndef _KDS_PMAG_L_H_
#define _KDS_PMAG_L_H_

#include <vector>

/// This class is used for the 6 or 7 value s vector.  Probably a bad name.
typedef std::vector<float> SVec; // should always size()==6

/// Place this right before function or method args that may not get
/// used in certain cases and the compiler will stop complaining
#ifdef __GNUC__
#define UNUSED __attribute((__unused__))
#else
#define UNUSED
#endif

/// Nice way to say where the test failed... takes emacs right to the problem
#ifdef REGRESSION_TEST
#define FAILED_HERE cout <<  __FILE__ << ":" << __LINE__ << " test failed" << endl
#else
#define FAILED_HERE // Empty
#endif

#endif
