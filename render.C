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
#ifndef NO_VOLEON
#include <VolumeViz/nodes/SoVolumeRendering.h>
#endif

// local includes
#include "render_cmd.h"
#include "InventorUtilities.H"

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

//#include <Inventor/nodes/SoDirectionalLight.h>

class SceneInfo {
public:
  SceneInfo();
  SoCamera *camera;
  SoSeparator *root;

  vector<SoSpotLightDragger *> draggerVec; ///< All of the draggers that have been added.
  SoSwitch *draggerSwitch; ///< On/off of the separators.  May add code to show one at a time
  //SoDirectionalLight *headlight;
  SoPointLight *headlight; ///< if this is not NULL, then move with camera

  gengetopt_args_info *a; ///< a for args.  These are the command line arguments

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
  headlight=0;
}

/***************************************************************************
 * UTILITUES
 ***************************************************************************/

void Print (const SoCamera *camera) {
  assert (camera);
  const SoSFVec3f *p = &camera->position;
  SbVec3f p_sb= p->getValue();
  float x,y,z;
  p_sb.getValue(x,y,z);
  const SoSFRotation *o = &camera->orientation;
  SbRotation o_sb = o->getValue();
  SbVec3f axis;
  float radians;
  o_sb.getValue(axis,radians);
  float ax,ay,az;
  axis.getValue(ax,ay,az);
  cout << "Camera ->  pos: " << x << " " << y << " " << y << "   --  rot: "
       << ax << " " << ay << " " << az << " " << radians << endl;
}

/***************************************************************************
 * TIMER FOR ANIMATION
 ***************************************************************************/

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

/// \brief interpolate between two draggers and render a frame to disk
bool WaypointRenderFrameToDisk (const SoSpotLightDragger *d1, const SoSpotLightDragger *d2,
				const float cur_percent, SoCamera *camera, 
				const string &basename, const string &typeStr,
				const int width, const int height,
				SoNode *root, size_t &frame_num, SoPointLight *headlight
				) 
{
  assert(d1);      assert(d2);
  assert(camera);  assert(root);

  SbVec3f pos1 = d1->translation.getValue();
  SbVec3f pos2 = d2->translation.getValue();

  vector<float> v1 = ToVector(pos1);
  vector<float> v2 = ToVector(pos2);
  
  vector<float> v3 = InterpolatePos (v1,v2,cur_percent);
  SbVec3f pos3 = ToSbVec3f (v3);
 
  camera->position = pos3;

  SbRotation rot1 = d1->rotation.getValue();
  SbRotation rot2 = d2->rotation.getValue();
  SbRotation newRot = SbRotation::slerp (rot1, rot2, cur_percent);
  camera->orientation = newRot;

  if (headlight) {  headlight->location = pos3;  }

#ifndef NDEBUG
  if (debug_level >= VERBOSE) Print(camera);   // Show some camera locations!
#endif

  DebugPrintf (TRACE,("ANIMATION: Rendering frame to disk file\n"));
  SbColor background(0,0,0);  // FIX: do background colors!
  if (!RenderFrameToDisk (basename,typeStr, width, height, root, frame_num, background) ) {
    cerr << "ERROR: unable to write frame" << endl;
    return (false);
  }
  DebugPrintf(VERBOSE,("ANIMATION: Finished writing frame number %04d\n",int(frame_num)));
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

  if (!CheckLibraryPath()) { cerr << "Bailing.  Library path check failed." << endl;  return(EXIT_FAILURE);}

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
#ifndef NO_VOLEON
  SoVolumeRendering::init();
#endif

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
    si->camera->nearDistance=a.near_arg;
    si->camera-> farDistance=a.far_arg;
  }

  if (a.headlight_flag) {
    //si->headlight = new SoDirectionalLight;
    si->headlight = new SoPointLight;
    si->root->addChild(si->headlight);
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

  if (2>si->draggerVec.size()) {
    cerr << "ERROR: you must have at least two waypoints in your .wpt file" << endl;
    exit (EXIT_FAILURE);
  }


  if (a.loop_flag) {
    cout << "Adding first waypoint to the end to form a loop" << endl;
    si->draggerVec.push_back(si->draggerVec[0]);
  }


  //////////////////////////////
  // ANIMATE - no timer needed!
  //////////////////////////////

  const string basenameStr(a.basename_arg);
  const string typeStr(a.type_arg);

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
      size_t frame_num;

      const bool r = WaypointRenderFrameToDisk (d1,d2, cur_percent, si->camera, 
						basenameStr, typeStr,
						a.width_arg, a.height_arg,
						si->root, frame_num,
						si->headlight
						);
      if (!r) {
	cerr << "ERROR: failed to write a frame.  I give up." << endl;
	exit(EXIT_FAILURE);
      }
    } // for cur_percent
  } // for wpt

  // Render that last missing frame.  If looping to start, we do not need this frame
  if (!a.loop_flag) {
    DebugPrintf (TRACE,("Rendering final frame.\n"));
    size_t frame_num;
    const bool r = WaypointRenderFrameToDisk (si->draggerVec[si->draggerVec.size()-1],
					      si->draggerVec[si->draggerVec.size()-1],
					      0., si->camera, 
					      basenameStr, typeStr,
					      a.width_arg, a.height_arg,
					      si->root, frame_num,
					      si->headlight
					      );
    if (!r) {
      cerr << "ERROR: failed to write a frame.  I give up." << endl;
      exit(EXIT_FAILURE);
    }

  }

  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}

