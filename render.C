// $Revision$  $Author$  $Date$
/*
    Unlike most of this library, this file is released only under the
    GPL since it links with Coin.

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

/// \brief Non-graphical rendering engine for waypoints, iv files, and
/// SIM Voleon models

/// This describes how to do animation:
/// file:///sw/share/Coin/html/classSoOffscreenRenderer.html

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
// Top level includes
#include <Inventor/SoInteraction.h>
#include <Inventor/SoOffscreenRenderer.h>
#include <Inventor/SoOutput.h>

// Sensors
//#include <Inventor/sensors/SoTimerSensor.h>

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
#include <VolumeViz/nodes/SoVolumeRendering.h>

// local includes
#include "render_cmd.h"

using namespace std;

/***************************************************************************
 * MACROS, DEFINES, GLOBALS
 ***************************************************************************/

// FIX: this needs to be a separate header
SoSeparator *MakeSoLineSet (vector<SoSpotLightDragger *> &draggerVec);


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
  SoSeparator *root;

  vector<SoSpotLightDragger *> draggerVec; ///< All of the draggers that have been added.
  SoSwitch *draggerSwitch; ///< On/off of the separators.  May add code to show one at a time

  gengetopt_args_info *a; ///< a for args.  These are the command line arguments

  //float  cur_percent; ///< how far between waypoints
  //size_t cur_mark;   //< Which waypoint are we on

  /// Use magic numbers to make sure you actually get this class when you
  /// must do a cast to/from void pointer
  static uint8_t nominalMagicNumber() {return(178);}
  bool magicOk() const {return(nominalMagicNumber()==magicNumber);} ///< Test the class integrity

private:
  uint8_t magicNumber; ///< Help make sure we have a valid SceneInfo structure 
};

/// Make a SceneInfo object with all pointers set to zero and not animating
SceneInfo::SceneInfo() {
  magicNumber = nominalMagicNumber();
  camera = 0; root = 0; draggerSwitch = 0; draggerSwitch = 0;
  a = 0;
  // Animation position
  //cur_percent=0; // how far between waypoints
  //cur_mark=0;   // Which waypoint are we on
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


/// \brief Given a draggers set the camera view from it
///
/// This is the simpler case where we do not have to interpolate
/// between to draggers
void SetCameraFromDragger(SoCamera *c,SoSpotLightDragger *d) {
  assert (d); assert(c);
  SbVec3f pos=d->translation.getValue();
  c->position = pos;
  SbRotation rot = d->rotation.getValue();
  c->orientation = rot;
}



/// \brief Ship off a picture to disk
/// \param basename start the file name with this
/// \param filetype the extension for the image format: jpg, png, etc...
/// \param width,height The size of the rendered frame
/// \param root Top of the scene graph to render
/// \param frame_num return the frame number that we wrote
/// \return \a false if we flailed trying to get a pretty picture to disk
///
/// \bug FIX: could optimize by keeping one renderer in memory

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





/***************************************************************************
 * KEYBOARD STUFF
 ***************************************************************************/

// gone


/***************************************************************************
 * TIMER FOR ANIMATION
 ***************************************************************************/

// timer is gone!




/// \brief Return a lineset that does through all the dragger waypoints
///
/// Would be better if I did what Alex did back in `98 and use engines with 
/// the draggers so the line gets automatically updated with drags
SoSeparator *MakeSoLineSet (vector<SoSpotLightDragger *> &draggerVec) {
  DebugPrintf(TRACE,("MakeSoLineSet with %d waypoints\n",int(draggerVec.size())));
  SoLineSet *lines = new SoLineSet;
  if (!lines) {cerr << "ERROR: Unable to allocate lines"<<endl;return 0;}
  SoCoordinate3 *coords = new SoCoordinate3;
  if (!coords) {cerr << "ERROR: Unable to allocate lines"<<endl; lines->ref(); lines->unref();return 0;}
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
  if (a.percent_given)
    if (!InRange(a.percent_arg,0.0,1.0)) {cerr<<"ERROR: Interval must be 0.0<=i<=1.0"<<endl; return (EXIT_FAILURE);}

  //
  // Init the SoDB and SimVoleon - no SoQT!
  //
  SoDB::init();
  SoInteraction::init();  // Initialize for draggers
  SoVolumeRendering::init();


  if (a.type_given)
    if (!CheckTypeValid(string(a.type_arg)))
      { cerr << "File type not valid: --type=" << a.type_arg << endl; return (EXIT_FAILURE); }

  DebugPrintf(VERBOSE,("FIX: check the range on width and height!\n"));

  SceneInfo *si = new SceneInfo;
  si->a=&a;

  si->root = new SoSeparator;
  si->root->ref();

  {
    // Need a camera in the scene for the SoOffscreenRender to work
    si->camera = new SoPerspectiveCamera;
    si->root->addChild(si->camera);
  }

  // FIX: do something better with the lights
  // TOP light
  { SoPointLight *pl = new SoPointLight; si->root->addChild(pl); pl->location = SbVec3f(0.0f,0.0f,50.0f); }
  // BOT light
  { SoPointLight *pl = new SoPointLight; si->root->addChild(pl); pl->location = SbVec3f(0.0f,0.0f,-50.0f); }

  for (size_t i=0;i<a.inputs_num;i++) {
    DebugPrintf (TRACE,("Adding file: %s\n",a.inputs[i]));
    SoInput mySceneInput;
    if ( !mySceneInput.openFile( a.inputs[i] ))  return (EXIT_FAILURE);

    SoSeparator* node = SoDB::readAll(&mySceneInput);
    if ( !node ) {cerr << "failed to load iv file: "<<a.inputs[i] << endl; return (EXIT_FAILURE);}
    mySceneInput.closeFile();

    si->root->addChild(node);
  }

  // Move this to the SceneInfo constructor
  {
    SoSwitch *s = new SoSwitch(40); // We expect a lot of children.
    assert(s);
    s->setName ("draggerSwitch");
    s->whichChild = SO_SWITCH_ALL; // SO_SWITCH_NONE
    si->root->addChild(s);
    si->draggerSwitch=s;
  }


  if (!LoadSpotLightDraggers(string(a.waypoints_arg), (SoSeparator *)si->draggerSwitch, si->draggerVec)) {
    cerr << "ERROR: failed to load waypoints.  We're toast." << endl;
    exit (EXIT_FAILURE);
  }
  si->draggerSwitch->whichChild = SO_SWITCH_NONE; // don't show our path

  // FIX: make sure we have at least 2 waypoints!


  //////////////////////////////
  // ANIMATE - no timer needed!
  //////////////////////////////

  // -1 cause we do not want to render past last waypoint
  // we are going from the current way point in wpt to the next waypoint
  // FIX: don't miss last frame
  for (size_t wpt=0;wpt<si->draggerVec.size()-1; wpt++) {
    cout << "Now at waypoint " << wpt << " going to " << wpt+1 << endl;

    SoSpotLightDragger *d1 = si->draggerVec[wpt];
    SoSpotLightDragger *d2 = si->draggerVec[wpt+1];
    assert (d1); assert(d2);

    for (float cur_percent=0.; cur_percent < 1.0; cur_percent += a.percent_arg) {
      cout << "percent: " << cur_percent << endl;
       SbVec3f pos1 = d1->translation.getValue();
       SbVec3f pos2 = d2->translation.getValue();

       vector<float> v1 = ToVector(pos1);
       vector<float> v2 = ToVector(pos2);
  
       vector<float> v3 = InterpolatePos (v1,v2,cur_percent);
       SbVec3f pos3 = ToSbVec3f (v3);
 
       si->camera->position = pos3;

       SbRotation rot1 = d1->rotation.getValue();
       SbRotation rot2 = d2->rotation.getValue();
       SbRotation newRot = SbRotation::slerp (rot1, rot2, cur_percent);
       si->camera->orientation = newRot;

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

  }


  // probably can never reach here.
  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}

