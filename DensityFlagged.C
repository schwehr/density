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

/// \brief Implementation of flagged density/traversal by largest neighbor

/// Similiar to the paint bucket algorithm

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
#include "Density.H"
#include "DensityFlagged.H"

using namespace std;

/***************************************************************************
 * MACROS, DEFINES, GLOBALS
 ***************************************************************************/

#include "debug.H" // provides FAILED_HERE, UNUSED, DebugPrintf
#ifdef REGRESSION_TEST
int debug_level=0;
#endif

/// Let the debugger find out which version is being used.
static const UNUSED char* RCSid ="@(#) $Id$";


//####################################################################
// DENSITYFLAGGED METHODS
//####################################################################


DensityFlagged::DensityFlagged(const size_t _width, const size_t _height, const size_t _depth,
		 const float minX, const float maxX,
		 const float minY, const float maxY,
		 const float minZ, const float maxZ)
{
  //Density(_width,_height,_depth, minX,maxX, minY,maxY, minZ,maxZ);
  resize(_width,_height,_depth, minX,maxX, minY,maxY, minZ,maxZ);

  flags.resize(getSize(),false);
  //for (vector<bool>::iterator i=flags.begin();i!=flags.end();i++) *i=false;

  return;
} // DensityFlagged constructor


size_t DensityFlagged::getLargest() const {
  size_t max=0;
  size_t maxIndex=badValue();
  for (size_t i=0;i<counts.size();i++)
    if (counts[i]>max) {
      max = counts[i];
      maxIndex=i;
    }
  return maxIndex;
}

size_t DensityFlagged::getLargestUnflagged() const {
  size_t max=0;
  size_t maxIndex=badValue();
  for (size_t i=0;i<counts.size();i++)
    if (counts[i]>max && !isFlagged(i)) {
      max = counts[i];
      maxIndex=i;
    }
  return maxIndex;
}



// Won't return an empty cell
size_t DensityFlagged::getLargestNeighbor(const size_t index) const {
  assert(isValidCell(index));
  //size_t n[NUM_NEIGHBORS];
  //size_t cts[NUM_NEIGHBORS];
  size_t maxVal=0;
  size_t maxIndex=badValue(); // send out a badValue if 1x1x1 cells
  for (size_t i=0;i<NUM_NEIGHBORS;i++) {
    const size_t n=getCellCount(getCellNeighbor(index,NeighborEnum(i)));
    if (isValidCell(n)) {
      //cts[i] = getCellCount(n[i]);
      const size_t cts = getCellCount(n);
      if (cts>maxVal) {maxVal=cts; maxIndex=n;}
    } // if
  } // for
    
  return (maxIndex);
}

size_t DensityFlagged::getLargestUnflaggedNeighbor(const size_t index) const {
  assert(isValidCell(index));
  size_t maxVal=0;
  size_t maxIndex=badValue(); // send out a badValue if 1x1x1 cells
  for (size_t i=0;i<NUM_NEIGHBORS;i++) {
    const size_t neighbor=getCellNeighbor(index,NeighborEnum(i));
    if (!isValidCell(neighbor)||isFlagged(neighbor)) continue;
    const size_t cts = getCellCount(neighbor);
    if (cts>maxVal) {maxVal=cts; maxIndex=neighbor;}
  } // for
    
  return (maxIndex);

}

size_t DensityFlagged::getLargestNeighborOfFlagged() const {
  size_t max=0;
  size_t maxIndex=badValue();
  for (size_t i=0;i<used.size();i++) {
    const size_t localMax=getLargestUnflaggedNeighbor(used[i]);
    if (localMax==badValue()) continue;
    if (counts[localMax]>max) {
      max = counts[localMax];
      maxIndex=localMax;
    }
  } // for
  return(maxIndex);
}

size_t DensityFlagged::getNumFlagged() const {
  size_t cnt=0;
  // FIX: use an algorithm?
  for (size_t i=0;i<counts.size();i++) if (isFlagged(i)) cnt++;
  assert(used.size()==cnt);
  return (cnt);
}

// FIX: count be optimized to sum everytime we add a voxel
size_t DensityFlagged::getFlaggedCount() const {
#ifndef NDEBUG
  size_t cnt1=0;
  for (size_t i=0;i<counts.size();i++) if (isFlagged(i)) cnt1+=counts[i];
#endif
  size_t cnt=0;
  for (size_t i=0;i<used.size();i++) cnt+=counts[used[i]];
  assert(cnt==cnt1);
  return (cnt);
}



void DensityFlagged::buildBlob(const float percent) {
  assert(0==getNumFlagged());

  // Set our finishing threshold
  const size_t maxCount=size_t(percent*getCountInside());

  // Get started with the highest density
  const size_t start = getLargest();
  setFlag(start);
  used.push_back(start);

  while (maxCount > getFlaggedCount()) {
    const size_t next = getLargestNeighborOfFlagged();
    if (badValue()==next) break; // no more found
    setFlag(next);
    used.push_back(next);
  }
  cout << "all done:  cells=" << getNumUsed() << "  counts=" << getFlaggedCount() << endl;
}


void DensityFlagged::printBlob() const {
  cout << "  *** BLOB OF DOOM ***" << endl <<endl;
  const float total=float(getCountInside());
  size_t cnts=0; // Running total
  for (size_t i=0;i<used.size();i++) {
    cnts += counts[used[i]];
    cout << i << ":  cell=" << used[i] << "  cnts=" << counts[used[i]] 
	 << "  percent="<< cnts/total << endl;
  }
}

//####################################################################
// TEST CODE
//####################################################################
#ifdef REGRESSION_TEST

bool test1() {
  bool ok=true;
  cout << "      test1" << endl;

  DensityFlagged df(2,2,2, 0.,2., 0.,2., 0.,2.);
  for (size_t i=0;i<df.getSize();i++)
    if (df.isFlagged(i)) {ok=false; FAILED_HERE;}

  for (size_t i=0;i<10;i++) df.addPoint(0.1,0.1,0.1); // 0
  for (size_t i=0;i< 9;i++) df.addPoint(1.1,0.1,0.1); // 1
  for (size_t i=0;i< 8;i++) df.addPoint(0.1,1.1,0.1); // 2
  for (size_t i=0;i< 7;i++) df.addPoint(1.1,1.1,0.1); // 3

  for (size_t i=0;i< 6;i++) df.addPoint(0.1,0.1,1.1); // 4
  for (size_t i=0;i< 5;i++) df.addPoint(1.1,0.1,1.1); // 5
  for (size_t i=0;i< 4;i++) df.addPoint(0.1,1.1,1.1); // 6
  for (size_t i=0;i< 1;i++) df.addPoint(1.1,1.1,1.1); // 7

  {
    if (0!=df.getCellFromWHD(0,0,0))  {ok=false; FAILED_HERE;}
    if (1!=df.getCellFromWHD(1,0,0))  {ok=false; FAILED_HERE;}
    if (5!=df.getCellFromWHD(1,0,1))  {ok=false; FAILED_HERE;}
  }

  if (0!=df.getLargest())  {ok=false; FAILED_HERE;}
  if (0!=df.getLargestUnflagged())  {ok=false; FAILED_HERE;}
  df.setFlag(0);
  if (1!=df.getLargestUnflagged())  {ok=false; FAILED_HERE;}
  df.setFlag(0,false);
  if (0!=df.getLargest())  {ok=false; FAILED_HERE;}

  df.buildBlob(0.7); // % conf
  df.printBlob();

  return(ok);
} // test1

int main (UNUSED int argc, char *argv[]) {
  // Put test code here
  bool ok=true;

  cout << "      Size of Density   (in bytes): " << sizeof(DensityFlagged) << endl;

  if (!test1()) {FAILED_HERE;ok=false;}

  cout << "  " << argv[0] << " test:  " << (ok?"ok":"failed")<<endl;
  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}
#endif // REGRESSION_TEST

