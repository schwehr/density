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

/// \brief Provide help for debugging and such.  Provides macros for
/// debug levels, regression testing, and suppression of warning for
/// unused variables.

//  Thanks to Dan Christian and Dave Wettergreen getting me hooked on
// the debug level stuff.
//
// I know, I am mixing both cout and printf.  It seems to work for
// me!


#ifndef _DEBUG_H_
#define _DEBUG_H_


#ifdef __GNUC__
#define UNUSED __attribute((__unused__))
#else
/*!
  \brief GNU CC attribute to denote unused paramters in function calls.
  The attribute remove compiler warning for unused arguments and variable.  Only works
  for GNU compilers such as gcc and g++.

  http://gcc.gnu.org/onlinedocs/gcc-3.4.2/gcc/Variable-Attributes.html
*/
#define UNUSED
#endif

#ifdef REGRESSION_TEST
#define FAILED_HERE cout <<  __FILE__ << ":" << __LINE__ << " test failed" << endl
#else
/*!
  \brief Use \a 'FAILED_HERE;' to emit a string that looks like a compiler warning
  Use this to allow emacs to jump to this source line with C-x `
*/
#define FAILED_HERE // Empty
#endif


/* Debugging levels... the higher the # the more output you get. */
// FIX: will these clash when we link other files with this in?  foo.o and bar.o both use debug.h?
const int ALWAYS=0;	///< Hint: if you use always, it won't work without -g.  FIX: still true?
const int TERSE=1;	///< Just the bare minimum
const int TRACE=4;	///< Trace major function calls, constructors, and methods
const int VERBOSE=8;	///< Lots and lots of junk
const int BOMBASTIC=20;	///< Major spew... when life sucks.

#ifdef NDEBUG
#  define DebugPrintf(n,s)		/* expand to nothing */
#else
   /// The main program needs to define this as a non-extern.
   extern int debug_level;
/// \brief Debugging messages that the user can control.  Disappear when compiling for optimization.
#  define DebugPrintf(n,s)  (((n) <= debug_level) ? printf s :0)
#endif


#endif // _DEBUG_H_
