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

/// \file 
/// \brief Basic viewer for OpenInventor/Coin and Voleon data.
/// Supports any file format that Coin/Voleon support.

/// This describes how to do animation:
/// file:///sw/share/Coin/html/classSoOffscreenRenderer.html

/// \bug Need to breakup simpleview info support libraries that can be
/// used to render a flight path without having SoQt/SoXt/SoWin


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

// Draggers
//#include <Inventor/draggers/SoTranslate1Dragger.h>
#include <Inventor/draggers/SoSpotLightDragger.h>

// Voleon Includes
#include <VolumeViz/nodes/SoVolumeRendering.h>

// local includes
#include "simpleview_cmd.h"

using namespace std;

/***************************************************************************
 * MACROS, DEFINES, GLOBALS
 ***************************************************************************/

#include "debug.H" // provides FAILED_HERE, UNUSED, DebugPrintf
int debug_level=0;

/// Let the debugger find out which version is being used.
static const UNUSED char* RCSid ="$Id$";

/***************************************************************************
 * SCENEINFO - the big global variable
 ***************************************************************************/

class SceneInfo {
public:
  SceneInfo();
  SoCamera *camera;
  //SoNode *root;
  SoSeparator *root;

  vector<SoSpotLightDragger *> draggerVec; ///< All of the draggers that have been added.
  SoSwitch *draggerSwitch; ///< On/off of the separators.  May add code to show one at a time

  gengetopt_args_info *a; ///< a for args.  These are the command line arguments

  bool animating;  ///< set to true to start animation;
  bool render_frames_to_disk;  ///< true, we write out each animated frame
  //float percent; ///< Where we are between the two current waypoints


  /// Use magic numbers to make sure you actually get this class when you
  /// must do a cast to/from void pointer
  static uint8_t nominalMagicNumber() {return(178);}
  //uint8_t getMagicNumber() const {return(magicNumber);}
  bool magicOk() const {return(nominalMagicNumber()==magicNumber);} ///< Test the class integrity
private:
  uint8_t magicNumber; ///< Help make sure we have a valid SceneInfo structure 
};

/// Make a SceneInfo object with all pointers set to zero and not animating
SceneInfo::SceneInfo() {
  magicNumber = nominalMagicNumber();
  camera = 0; root = 0; draggerSwitch = 0; draggerSwitch = 0;
  a = 0;
  animating=render_frames_to_disk=false;
}


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
InterpolateVec(const SbVec3f &v1, SbVec3f &v2,
			     const float percent)
{
  return (v1+(v2-v1)*percent);
}


SbRotation InterpolateRotations(const SbRotation &rot1,const SbRotation &rot2,
				const float percent)
{
#if 0
  // Try 1.  camera roles
  SbVec3f axis1; float rad1;
  rot1.getValue(axis1,rad1);
  SbVec3f axis2; float rad2;
  rot2.getValue(axis2,rad2);
  SbVec3f newAxis = InterpolateVec(axis1,axis2,percent);
  SbRotation newRot(newAxis, rad1 + (rad2-rad1)*percent);
#endif

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
// FIX: could optimize by keeping one renderer in memory

/// \brief Ship off a picture to disk
/// \param basename start the file name with this
/// \param filetype the extension for the image format: jpg, png, etc...
/// \param width,height The size of the rendered frame
/// \param root Top of the scene graph to render
/// \param frame_num return the frame number that we wrote
/// \return \a false if we flailed trying to get a pretty picture to disk
bool RenderFrameToDisk (const string &basename,const string &filetype,
			const int width, const int height,
			SoNode *root, size_t &frame_num)
{
  assert (root);
  SbViewportRegion viewport(width,height);
  SoOffscreenRenderer *renderer = new SoOffscreenRenderer(viewport);

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
  ok = renderer->writeToFile(filename.c_str(), filetype_sb);
  if (!ok) {cerr << "Failed to render" << endl; }
  delete (renderer);
  return (ok);
}


/// \brief Handle the --list command line option.  Spits out the list
/// of filetypes that work for rendering to files
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
  cout << "Number of file types: " << numFileTypes << endl;
  for (int i=0;i<numFileTypes;i++) {
    SbPList extlist;
    SbString fullname, descr;
    r->getWriteFiletypeInfo(i,extlist,fullname,descr);
    cout << i<< ": " <<fullname.getString() << " - " << descr.getString() << endl;
    for (int ext=0;ext<extlist.getLength(); ext++) {
      cout << "   " << ext << ": " << (const char *)extlist[ext] << endl;
    }
  }
  delete r;
  delete viewport;

} // ListWriteFileTypes

/// \brief Check with coin/simage to see if the image extension works
/// \param type String of the file extension.  For example \a rgb, \a png, \a jpg
/// \return \a false if something went really bad with the lookup.
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

/// \brief Load dragger/way points from a disk file
/// \param filename Ascii file to load from
/// \param root Where in the scene graph location to add the draggers  (should be draggerSwitch
/// \param draggerVec A vector in which to keep the way points handy
/// \return \a false if trouble loading the draggers/waypoints
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



/// \brief Save an ascii file of all the draggers for editing and reloading
/// \param filename file to write to
/// \param draggerVec pointers to all the SoSpotLightDragger way points
/// \return \a false if had trouble writing the file
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

#if 0
/// \brief Save an ascii file of all the draggers for editing and reloading
/// \param filename file to write to
/// \param draggerVec pointers to all the SoSpotLightDragger way points
/// \return \a false if had trouble writing the file
///
/// \bug FIX: am not able to load these scene graphs in without crashing coin
bool
SaveSpotLightDraggersIV(const string &filename, SoSwitch *sw) {
  assert(sw);
  if (0==sw->getNumChildren()) {cerr << "WARNING: No waypoints to write"<<endl;return(false);}
  SoOutput o;
  if (!o.openFile(filename.c_str())) {
    cerr<<"ERROR: unable to open file to dump iv to: " << filename << endl;
    return(false);
  }

  SoWriteAction wa(&o);
  for (int i=0;i< sw->getNumChildren();i++) {
    DebugPrintf(BOMBASTIC,("Writing switch child # %d\n",i));
    wa.apply(sw->getChild(i));
  }
  return (true);
} // SaveSpotLightDraggersIV
#endif

/// \brief Write a scene graph to disk as ascii
/// \param filename File to write ascii IV file to
/// \param root Starting point of the scene graph to same
/// \return \a false if we were not able to open the file.
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


/***************************************************************************
 * KEYBOARD STUFF
 ***************************************************************************/

/// \brief Send help on keyboard shortcuts to stdout
void PrintKeyboardShorts() {
  cout << endl
       << "Keyboard shortcuts:" << endl << endl
       << "\t a - Toggle (A)nimation -  Start/Stop" << endl
    // FIX: implement these!
    // << "\t b - Jump to the (B)eginning of the animation sequence" << endl
    // << "\t c - (C)onnect the dots.  Put lines between the waypoints" << endl
       << "\t d - (D)ump the current view to an image" << endl
       << "\t f - Write scene graph to a (F)File" << endl
       << "\t h - Print out this (H)elp list" << endl
       << "\t i - (I)nsert a waypoint at the current camera location" << endl
       << "\t r - (R)Render frames as they animate" << endl
       << "\t s - (S)how/Hide way point markers" << endl
       << "\t w - Write the current set of (W)aypoints to a file" << endl
       << endl
       << "\t - - Decrease the debug level" << endl
       << "\t + - Incease  the debug level" << endl
       << "\t - " << endl
       << endl
       << "\t Do not forget to use the right mouse button or option-left mouse for more"<<endl
       << endl;

}

/// \brief Gets called whenever the user presses a key in interactive mode
/// \param data This will be the SceneInfo structure.
/// \param cb SoKeyboardEvent that records the keypress or release
///
/// Make sure that this stays in sync with PrintKeyboardShorts()
void
keyPressCallback(void *data, SoEventCallback *cb) {
  assert(data); assert(cb);

  SceneInfo *si = (SceneInfo *)data;
  //assert(si->getMagicNumber()==SceneInfo::nominalMagicNumber());
  assert (si->magicOk());

  assert (si);
  assert (si->camera);
  assert (si->root);

  const SoKeyboardEvent *keyEvent = (const SoKeyboardEvent *)cb->getEvent();

  //cerr << "Keypressed: " << keyEvent->getPrintableCharacter() << endl;
  DebugPrintf(VERBOSE+4,("keyPressCallback: '%c'\n",keyEvent->getPrintableCharacter()));

  //
  // A - toggle animating
  //
  if (SO_KEY_PRESS_EVENT(keyEvent, A)) {
    si->animating = (si->animating?false:true); // flip
    cout << "toggling animation to " << (si->animating?"true":"false") << endl;
    //cout.flush();
    return;
  }

  //
  // D - Dump a rendering of the scene
  //
  if (SO_KEY_PRESS_EVENT(keyEvent, D)) {
    DebugPrintf(TRACE,("Keyhit: D - render  %d %d\n",si->a->width_arg,si->a->height_arg));
    size_t frame_num;
    if (!RenderFrameToDisk (string(si->a->basename_arg),string(si->a->type_arg),
			    si->a->width_arg, si->a->height_arg,
			    si->root, frame_num) )
      cerr << "ERROR: unable to write you artistic work.  You have been sensored.  Not my fault." << endl;

    DebugPrintf(TRACE+1,("Finished writing frame number %04d\n",int(frame_num)));
    return;
  } // KEY_PRESS D - dump screen

  //
  // H - Print Help to stdout
  //
  if (SO_KEY_PRESS_EVENT(keyEvent, H)) { PrintKeyboardShorts(); return; }

  //
  // F - Write scene graph to a file
  //
  if (SO_KEY_PRESS_EVENT(keyEvent, F)) {
    const string filename = "tmp.iv";
    DebugPrintf(TRACE,("Keyhit: F - write scenegraph to a (F)ile: %s\n",filename.c_str()));
    if (!WriteSceneGraph(filename, si->root)) {
      cerr << "ERROR: Unable to write scene graph to file: " << filename << endl;
    }
    return;
  }

  //
  // I - insert a waypoint marker
  //
  if (SO_KEY_PRESS_EVENT(keyEvent, I)) {
    DebugPrintf(TRACE,("Keyhit: I - insert waypoint\n"));
    SoCamera *c = si->camera;
    float x,y,z;
    SbVec3f pos = c->position.getValue();
    pos.getValue (x,y,z);
    DebugPrintf  (VERBOSE,("pos xyz: %f %f %f\n", x, y, z));
    SbRotation rot = si->camera->orientation.getValue();
    {
      SbVec3f axis; float radians;
      rot.getValue(axis,radians);
      axis.getValue(x,y,z);
      DebugPrintf(VERBOSE,("axis/radians: [ %f %f %f ] %f\n",x,y,z,radians));
    }

    // Add marker
    {
      SoSeparator *sep = new SoSeparator;

      SoSpotLightDragger *mark = new SoSpotLightDragger;
      {
	static int waypointNum=0;
	si->draggerVec.push_back(mark);
	
	char buf[128];
	snprintf(buf, 128,"_%d",waypointNum++); // name can't start with a number
	mark->setName (buf);
      }
      mark->translation.setValue(pos);
      mark->rotation.setValue(rot);

      sep->addChild(mark);
      si->draggerSwitch->addChild(sep);
    }
    return;
  } // KEY_PRESS I - Insert waypoint


  // + - Toggle render mode for during animation.  If true write all frames to disk
  if (SO_KEY_PRESS_EVENT(keyEvent, R)) {
    si->render_frames_to_disk = (si->render_frames_to_disk?false:true); // flip

    DebugPrintf(TRACE,("Keyhit: R - toggle render to %s\n",(si->render_frames_to_disk?"true":"false")));
    return;
  }


  //
  // S - show/hide waypoint markers
  //
  if (SO_KEY_PRESS_EVENT(keyEvent, S)) {
    if (SO_SWITCH_ALL == si->draggerSwitch->whichChild.getValue())
      si->draggerSwitch->whichChild = SO_SWITCH_NONE;
    else si->draggerSwitch->whichChild = SO_SWITCH_ALL;
    return;
  }

  //
  // W - write waypoints to a file
  //
  if (SO_KEY_PRESS_EVENT(keyEvent, W)) {
    const string filenameAscii("tmp.wpt.ascii");
    const string filenameIV("tmp.wpt.iv");

    DebugPrintf(TRACE,("Keyhit: W - write waypoints: %s %s\n",filenameAscii.c_str(),filenameIV.c_str()));
#if 0
    if (!SaveSpotLightDraggersIV(filenameIV, si->draggerSwitch))
      cerr << "WARNING: waypoint save to " << filenameIV << "failed." << endl;
#endif
    if (!SaveSpotLightDraggersAscii(filenameAscii, si->draggerVec))
      cerr << "WARNING: waypoint save to " << filenameAscii << "failed." << endl;
    return;
  } // KEY_PRESS W - write waypoints


  //
  // + - Increase the debugging level ... equal is that key with the '+' on it too.
  //
  // FIX: Describe which debug level we are at
  if (SO_KEY_PRESS_EVENT(keyEvent, EQUAL)) {
    debug_level++;
    DebugPrintf(TRACE,("Keyhit: + - Increase debug_level to %d\n",debug_level));
    return;
  }

  // - - Decrease the debugging level
  if (SO_KEY_PRESS_EVENT(keyEvent, MINUS)) {
    if (0<debug_level) debug_level--;
    DebugPrintf(TRACE,("Keyhit: + - Decrease debug_level to %d\n",debug_level));
    return;
  }

} // keyPressCallback



/***************************************************************************
 * TIMER FOR ANIMATION
 ***************************************************************************/

/// \brief Handle details of moving the camera between waypoints
/// \param data SceneInfo data structure
/// \param sensor The timer that went off
void timerSensorCallback(void *data, SoSensor *sensor) {
  assert(data);
  assert(sensor);
  SceneInfo *si = (SceneInfo *)data;
  static bool initialized=false;

  if (!si->animating) {
    initialized=false;
    return;  // not turned on right now
  }
  if (si->draggerVec.size()<2) { cout << "nothing to animate!" << endl;   return;  }

  // Now lets animate things!

  static float percent=10.;
  static size_t cur=0;

  // handle setup.
  if (!initialized) {
    initialized=true;
    cout << "Initialization for animation." << endl;
    percent = 0.;
    cur = 0;
  }

  if (percent >= 1.0) {
    percent=0.;
    cur++;
    cout << "Switching to mark #" << cur << endl;
    //if (si->draggerVec.size()-1<=cur) {
    if (si->draggerVec.size()==cur) {
      //cout << "Looping" << endl;
      cur=0;
    }
  }

  //
  // Calc the camera position and set it!
  //
  //cout << "using: " << cur << " " << cur+1 << endl;
  SoSpotLightDragger *d1 = si->draggerVec[cur];
  SoSpotLightDragger *d2;
  if (cur==si->draggerVec.size()-1) {
    d2 = si->draggerVec[0];
    //cout << "Using " << cur << "-0" << endl;
  } else d2 = si->draggerVec[cur+1];

  SbVec3f pos1 = d1->translation.getValue();
  SbVec3f pos2 = d2->translation.getValue();

  vector<float> v1 = ToVector(pos1);
  vector<float> v2 = ToVector(pos2);

  
  vector<float> v3 = InterpolatePos (v1,v2,percent);
  SbVec3f pos3 = ToSbVec3f (v3);

  si->camera->position = pos3;

  SbRotation rot1 = d1->rotation.getValue();
  SbRotation rot2 = d2->rotation.getValue();
  SbRotation newRot = SbRotation::slerp (rot1, rot2, percent);

  si->camera->orientation = newRot;

  DebugPrintf (VERBOSE,("Mark: %d      Step:  %f\n",int(cur),percent));
  percent += si->a->percent_arg; // User configurable jump

  if (si->render_frames_to_disk) {
    DebugPrintf (TRACE,("ANIMATION: Rendering frame to disk file\n"));
    size_t frame_num;
    if (!RenderFrameToDisk (string(si->a->basename_arg),string(si->a->type_arg),
			si->a->width_arg, si->a->height_arg,
			si->root, frame_num)
	) {
      cerr << "ERROR: unable to write you artistic work.  You have been sensored.  Not my fault." << endl;
    }
    DebugPrintf(TRACE+1,("ANIMATION: Finished writing frame number %04d\n",int(frame_num)));
  }

  return;
}


/// \brief Return a lineset that does through all the dragger waypoints
SoSeparator *MakeSoLineSet (vector<SoSpotLightDragger *> &draggerVec) {
  SoLineset *lines = new SoLineSet;
  if (!lines) {cerr << "ERROR: Unable to allocate lines"<<endl;return 0;}
  SoCoordinate3 *coords = new SoCoordinate3;
  if (!coords) {cerr << "ERROR: Unable to allocate lines"<<endl; delete lines; return 0;}
  SoSeparator *sep = new SoSeparator;
  // FIX error check

  sep->addChild(coords);
  sep->addChild(lines);
  for (size_t i=0;i<draggerVec.size();i++) {
    SbVec3f xyz(draggerVec[i]->translation.getValue());
    float x, y, z;
    xyz.getValue(x,y,z);
    coords->point.set1Value(i,x,y,z);
  }
  lines->numVertices.setValue(draggerVec.size());
  
  return(sep);
}


/// \brief Tell if a value is between 2 numbers, inclusive
/// \return \a true if v1<=check<=v2  or v2<=check<=v1
bool InRange (const float check, const float v1, const float v2) {
  float a,b;
  if (v1<v2) {a=v1;b=v2;} else {a=v2;b=v1;}
  if (check < a || check > b) return (false);
  return (true);
}

/***************************************************************************
 * MAIN
 ***************************************************************************/

int main(int argc, char *argv[])
{
  bool ok=true;

  gengetopt_args_info a;  // a == args
  if (0!=cmdline_parser(argc,argv,&a)){cerr<<"MELT DOWN: should never get here"<<endl;return (EXIT_FAILURE);}

#ifdef NDEBUG
  if (a.verbosity_given) {
    cerr << "Verbosity is totally ignored for optimized code.  Continuing in silent mode" << endl;
  }
#else // debugging
  debug_level = a.verbosity_arg;
  DebugPrintf(TRACE,("Debug level = %d\n",debug_level));
#endif


  if (a.list_given) { ListWriteFileTypes();  return (EXIT_SUCCESS); }
  //
  // Check range of arguments
  //
  if (a.interval_given) 
    if (!InRange(a.interval_arg,0.001,10.0)) {cerr<<"ERROR: Interval must be 0.001<=i<=10.0"<<endl; return (EXIT_FAILURE);}
  if (a.percent_given)
    if (!InRange(a.percent_arg,0.0,1.0)) {cerr<<"ERROR: Interval must be 0.0<=i<=1.0"<<endl; return (EXIT_FAILURE);}

  //
  // Init the SoDB and SimVoleon
  //
  QWidget* myWindow = SoQt::init(argv[0]);
  if ( myWindow==NULL ) return (EXIT_FAILURE);
  SoVolumeRendering::init();

  if (a.type_given)
    if (!CheckTypeValid(string(a.type_arg)))
      { cerr << "File type not valid: --type=" << a.type_arg << endl; return (EXIT_FAILURE); }

  DebugPrintf(VERBOSE,("FIX: check the range on width and height!\n"));
  // FIX: allow rendering without opening a window

  SceneInfo *si = new SceneInfo;
  si->a=&a;

  si->root = new SoSeparator;
  si->root->ref();

  {
    // Need a camera in the scene for the SoOffscreenRender to work
    si->camera = new SoPerspectiveCamera;
    si->root->addChild(si->camera);
  }

  // TOP light
  { SoPointLight *pl = new SoPointLight; si->root->addChild(pl); pl->location = SbVec3f(0.0f,0.0f,50.0f); }
  // BOT light
  { SoPointLight *pl = new SoPointLight; si->root->addChild(pl); pl->location = SbVec3f(0.0f,0.0f,-50.0f); }



  SoQtExaminerViewer* myViewer = new SoQtExaminerViewer(myWindow);
  for (size_t i=0;i<a.inputs_num;i++) {
    DebugPrintf (TRACE,("Adding file: %s\n",a.inputs[i]));
    SoInput mySceneInput;
    if ( !mySceneInput.openFile( a.inputs[i] ))  return (EXIT_FAILURE);

    SoSeparator* node = SoDB::readAll(&mySceneInput);
    if ( !node ) {cerr << "failed to load iv file: "<<a.inputs[i] << endl; return (EXIT_FAILURE);}
    mySceneInput.closeFile();

    si->root->addChild(node);
  }

  
  myViewer->setSceneGraph( si->root );
  myViewer->show();


  {
    // Set up callback
    SoEventCallback 	*keyEventHandler = new SoEventCallback;
    assert (keyEventHandler);
    keyEventHandler->addEventCallback(SoKeyboardEvent::getClassTypeId(), keyPressCallback, si);
    si->root->addChild(keyEventHandler);
  }

  {
    SoSwitch *s = new SoSwitch(40); // We expect a lot of children.
    assert(s);
    s->setName ("draggerSwitch");
    s->whichChild = SO_SWITCH_ALL; // SO_SWITCH_NONE
    si->root->addChild(s);
    si->draggerSwitch=s;
  }

  if (a.waypoints_given) {
    // FIX: make load go under an soswitch?
    if (!LoadSpotLightDraggers(string(a.waypoints_arg), (SoSeparator *)si->draggerSwitch, si->draggerVec)) {
      cerr << "WARNING: failed to load waypoints.  Continuing anyways." << endl;
    }
  }

  // Write out keyboard shortcuts
  if (0<debug_level) PrintKeyboardShorts();

  // Setup a timer callback for animating...
  {
    SoTimerSensor *timer = new SoTimerSensor (timerSensorCallback,si);
    assert (timer);
    //timer->setInterval(.25);
    DebugPrintf(TRACE+1,("Setting animation timer to %f seconds\n",a.interval_arg));
    timer->setInterval(a.interval_arg);
    timer->schedule();
  }


  SoQt::show(myWindow);
  SoQt::mainLoop();

  // probably can never reach here.
  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}

