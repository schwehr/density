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
/// \brief Code that is shared between simpleview and render

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

// c includes
#include <cstdio>
#include <cassert>

// C++ includes
#include <iostream>
#include <fstream>
#include <sstream> // string stream

// STL includes
#include <vector>
#include <string>

//
// Inventor/Coin
//
// SoQt
#include <Inventor/Qt/SoQt.h>
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>

// Top level includes
#include <Inventor/SoOffscreenRenderer.h>
#include <Inventor/SoOutput.h>

// Sensors
#include <Inventor/sensors/SoTimerSensor.h>

// Actions
#include <Inventor/actions/SoWriteAction.h>

// Events
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/events/SoLocation2Event.h>
#include <Inventor/events/SoKeyboardEvent.h>

// Nodes
#include <Inventor/nodes/SoEventCallback.h>

#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoPointLight.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/nodes/SoCoordinate3.h>

// Draggers
//#include <Inventor/draggers/SoTranslate1Dragger.h>
#include <Inventor/draggers/SoSpotLightDragger.h>

// Voleon Includes
#ifndef NO_VOLEON
#include <VolumeViz/nodes/SoVolumeRendering.h>
#endif

//#include "InventorUtilities.H"
#include "debug.H"

/// Let the debugger find out which version is being used.
static const UNUSED char* RCSid ="$Id$";

using namespace std;

/***************************************************************************
 * Vector Utilities
 ***************************************************************************/


SbVec3f ToSbVec3f (const vector<float> &v) {
  assert (3==v.size());
  float x,y,z;
  x=v[0]; y=v[1]; z=v[2];
  SbVec3f s;
  s.setValue(x,y,z);
  return (s);
}

vector<float> ToVector (const SbVec3f &SbV) {
  vector<float> v;
  float x,y,z;
  SbV.getValue(x,y,z);
  v.push_back(x);
  v.push_back(y);
  v.push_back(z);
  return (v);
}


SbVec3f
InterpolateVec(const SbVec3f &v1, SbVec3f &v2, const float percent)
{
  return (v1+(v2-v1)*percent);
}


SbRotation InterpolateRotations(const SbRotation &rot1,const SbRotation &rot2,
				const float percent)
{
  float q1[4], q2[4], q3[4];
  rot1.getValue(q1[0],q1[1],q1[2],q1[3]);
  rot2.getValue(q2[0],q2[1],q2[2],q2[3]);

  int i;
  for (i=0;i<4;i++)
    q3[i] = q1[i]+(q2[i]-q1[i])*percent;

  SbRotation newRot(q3);
  return (newRot);
}


vector<float>
InterpolatePos(const vector<float> &v1, const vector<float> &v2,
			     const float percent)
{
  assert (v1.size()==v2.size());
  vector<float> v3;
  size_t i;
  for (i=0;i<v1.size();i++) {
    const float a = v1[i];
    const float b = v2[i];
    const float delta = b-a;
    const float newVal= a + delta*percent;
    v3.push_back(newVal);
  }
  return (v3);
}

/***************************************************************************
 * Utilities
 ***************************************************************************/


void SetCameraFromDragger(SoCamera *c,SoSpotLightDragger *d) {
  assert (d); assert(c);
  SbVec3f pos=d->translation.getValue();
  c->position = pos;
  SbRotation rot = d->rotation.getValue();
  c->orientation = rot;
}



bool RenderFrameToDisk (const string &basename,const string &filetype,
			const int width, const int height,
			SoNode *root, size_t &frame_num, const SbColor &background)
{
  assert (root);
  DebugPrintf(VERBOSE,("w,h=%d,%d\n",width,height));
  SbViewportRegion viewport(width,height);
  SoOffscreenRenderer *renderer = new SoOffscreenRenderer(viewport);

  //renderer->setBackgroundColor (background);

#ifndef NDEBUG
  SbVec2s maxRes = renderer->getMaximumResolution();
  DebugPrintf(VERBOSE,("maxRes: %d %d\n",maxRes[0], maxRes[1]));
#endif    

  SbBool ok = renderer->render(root);
  if (!ok) {  cerr << "Unable to render!" << endl;  delete(renderer); return(false);   }

  static int count=0;
  char countBuf[12];
  frame_num = size_t(count);  snprintf(countBuf,12,"%04d",count++);
  const string filename =
    string(basename)
    + string(countBuf)
    + string(".")
    + filetype;
  SbName filetype_sb(filetype.c_str());
  DebugPrintf (VERBOSE,("rendering offscreen - writeToFile:  %s  %s\n",filename.c_str(), filetype_sb.getString()));
  ok = renderer->writeToFile(filename.c_str(), filetype_sb);
  if (!ok) {cerr << "Failed to render" << endl; }
  delete (renderer);
  return (ok);
}




void ListWriteFileTypes() {
  if (!SoDB::isInitialized()) SoDB::init();

  SbViewportRegion *viewport= new SbViewportRegion();
  SoOffscreenRenderer *r = new SoOffscreenRenderer(*viewport);
  const int numFileTypes = r->getNumWriteFiletypes();
  if (0==numFileTypes) {
    cout << "Only SGI RGB and Postscript are supported\n" << endl;
    delete r;
    delete viewport;
    return;
  }
  cout << "Number of file type drivers: " << numFileTypes << endl;
  size_t count=1;
  for (int i=0;i<numFileTypes;i++) {
    SbPList extlist;
    SbString fullname, descr;
    r->getWriteFiletypeInfo(i,extlist,fullname,descr);
    //cout << i<< ": " <<fullname.getString() << " - " << descr.getString() << endl;
    for (int ext=0;ext<extlist.getLength(); ext++) {
      cout << "   " << count++ << ": " << (const char *)extlist[ext] << endl;
    }
  }
  delete r;
  delete viewport;

} // ListWriteFileTypes


bool CheckTypeValid(const string &type) {
  if (!SoDB::isInitialized()) SoDB::init();

  SbViewportRegion *viewport= new SbViewportRegion();
  SoOffscreenRenderer *r = new SoOffscreenRenderer(*viewport);
  SbName name(type.c_str());

  SbBool ok=r->isWriteSupported(name);

  delete r;
  delete viewport;
  return (ok);
}


bool
LoadSpotLightDraggers (const string filename, SoSeparator *root, vector<SoSpotLightDragger *> &draggerVec)
{
  bool ok=true;
  assert(root);
  DebugPrintf(TRACE,("LoadSpotLightDraggers %s %d\n",filename.c_str(),int(draggerVec.size())));

  ifstream in(filename.c_str(),ios::in);
  if (!in.is_open()){cerr<<"ERROR opening "<<filename<<endl; return(false);}
  const size_t bufSize=256;
  char buf[bufSize];
#ifndef NDEBUG
  memset(buf,0,bufSize);
#endif
  while (in.getline(buf,bufSize)) {
    if ('#'==buf[0]) continue;
    istringstream istr(buf);
    float a1,a2,a3,r_angle,x,y,z,angle;
    istr >> a1 >> a2 >> a3 >> r_angle >> x >> y >> z >> angle;

    SoSpotLightDragger *wpt = new SoSpotLightDragger;
    draggerVec.push_back(wpt);
    {
      SbRotation rot(SbVec3f(a1,a2,a3),r_angle);
      
      SbVec3f pos(x,y,z);
      wpt->translation.setValue(pos);
      wpt->rotation.setValue(rot);
      wpt->angle=angle;
    }
    {
      static int num=0;  char buf[128];
      snprintf(buf, 128,"_%d",num++); // name can't start with a number
      wpt->setName (buf);
    }
    SoSeparator *s = new SoSeparator;
    s->addChild(wpt);
    root->addChild(s); // draggerSwitch
  }	

  return (ok);
} // LoadSpotLightDraggers	



bool
SaveSpotLightDraggersAscii (const string &filename, vector<SoSpotLightDragger *> &draggerVec) {
  bool ok=true;
  DebugPrintf(TRACE,("SaveSpotLightDraggers %s %d\n",filename.c_str(),int(draggerVec.size())));

  if(0==draggerVec.size()){cerr<<"WARNING: skipping save.  There are 0 waypoints"<<endl;return(false);}

  ofstream o(filename.c_str(),ios::out);
  if (!o.is_open()) {cerr<<"ERROR: unable to open waypoint file: "<<filename<<endl; return (false);}

  o << "#waypoints" << endl
    << "# rotaxis1 rotaxis2 rotaxis3 rotAngle x y z cutoffangle" << endl
    << "# written by: " << RCSid << endl;

  for (size_t i=0;i<draggerVec.size();i++) {
    SoSpotLightDragger *d = draggerVec[i];
    assert (d);
    SbVec3f axis; float r_angle;
    float a1,a2,a3;
    d->rotation.getValue(axis,r_angle);
    axis.getValue(a1,a2,a3);
    SbVec3f xyz(d->translation.getValue());
    float x, y, z;
    xyz.getValue(x,y,z);
    const float angle = d->angle.getValue(); // FIX: do not currently use angle for anything
    o << a1 << "\t"<< a2 << "\t"<< a3 
      << "\t" << r_angle
      << "\t" << x << "\t" << y << "\t" << z
      << "\t" << angle << endl;
  }

  return (ok);
} // SaveSpotLightDraggersAscii

bool
WriteSceneGraph (const string &filename, SoNode *root) {
  SoOutput o;
  if (!o.openFile(filename.c_str())) {
    cerr<<"ERROR: unable to open file to dump iv to: " << filename << endl;
    return(false);
  }

  SoWriteAction wa(&o);
  wa.apply(root); // Error checking?
  return (true);
}

bool CheckLibraryPath () {
#ifdef __APPLE__ // DARWIN/MacOSX
  char *path = getenv("DYLD_LIBRARY_PATH");
  if (!path) {
    cerr  << __FILE__ << ":" << __LINE__ << " ERROR!\n"
	  << "  Unable to find DYLD_LIBRARY_PATH.  Must have it set for simage to work!\n"
	  << "  e.g. for bash\n\n"
	  << "        export DYLD_LIBRARY_PATH=/sw/lib\n"
	  << endl;
    return (false);
  }
  if (0==strstr(path,"/sw/lib")) {
    cerr << "WARNING:  did not find /sw/lib in your DYLD_LIBRARY_PATH.\n"
	 << "  This may be ok, but do not be surprised if simage will not write images to disk\n"
	 << "  Continuing on despite the danger\n"
	 << endl;
  }
#endif

  // FIX: add checks needed for linux

  return true;
}

