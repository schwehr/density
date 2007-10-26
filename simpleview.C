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
/// \brief Basic viewer for OpenInventor/Coin and Voleon data.
/// Supports any file format that Coin/Voleon support.

/// This describes how to do animation:
/// file:///sw/share/Coin/html/classSoOffscreenRenderer.html

/// \bug Need to breakup simpleview info support libraries that can be
/// used to render a flight path without having SoQt/SoXt/SoWin

/// \todo FIX add an option to interpolate between ssml nodes.

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

// libxml2
//#define WITH_LIBXML
#ifdef WITH_LIBXML
#include <Inventor/SbTime.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#endif


// QT so that we can resize the display
#include <qwidget.h>

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

// Stuff for the SSML animations
#include <Inventor/nodes/SoRotationXYZ.h> // For statestamp
#include <Inventor/SoPath.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/actions/SoSearchAction.h>

// Draggers
//#include <Inventor/draggers/SoTranslate1Dragger.h>
#include <Inventor/draggers/SoSpotLightDragger.h>

// Voleon Includes
#ifndef NO_VOLEON
#include <VolumeViz/nodes/SoVolumeRendering.h>
#endif

// local includes
#include "simpleview_cmd.h"
#include "InventorUtilities.H"

using namespace std;

/***************************************************************************
 * MACROS, DEFINES, GLOBALS
 ***************************************************************************/

bool streq (const char *s1, const char *s2) {return 0==strcmp(s1,s2);}

// FIX: this needs to be a separate header
SoSeparator *MakeSoLineSet (vector<SoSpotLightDragger *> &draggerVec);


#include "debug.H" // provides FAILED_HERE, UNUSED, DebugPrintf
int debug_level=0;

/// Let the debugger find out which version is being used.
static const UNUSED char* RCSid ="$Id$";

/// \brief Only for RotationXYZ
struct StateStampJoints {
    SbTime time;
    vector<SoRotationXYZ *> rotNodes;
    vector<SoSFFloat> angles;
};

//typedef vector<StateStampJoints> StateStampJointsVec;



/***************************************************************************
 * SCENEINFO - the big global variable
 ***************************************************************************/

/// \brief State information for the callbacks.  This is as bad as global variables.

class SceneInfo {
public:
    SceneInfo();  
  SoCamera *camera; ///< Fiddle with this to look around the scene
  SoSeparator *root; ///< Top of the scene graph

  vector<SoSpotLightDragger *> draggerVec; ///< All of the draggers that have been added.
  SoSwitch *draggerSwitch; ///< On/off of the separators.  May add code to show one at a time

  gengetopt_args_info *a; ///< a for args.  These are the command line arguments

  bool animating;  ///< set to true to start animation;
  bool render_frames_to_disk;  ///< true, we write out each animated frame
  //float percent; ///< Where we are between the two current waypoints

  bool connect_the_dots; ///< true if showing last path
  SoSeparator *connect_sep;

  float  cur_percent; ///< how far between waypoints
  size_t cur_mark;   //< Which waypoint are we on

  SbColor background; ///< Background rendering color

  // Use magic numbers to make sure you actually get this class when you
  // must do a cast to/from void pointer
  static uint8_t nominalMagicNumber() {return(178);}  ///< find out the magic number
  bool magicOk() const {return(nominalMagicNumber()==magicNumber);} ///< Test the class integrity

#ifdef WITH_LIBXML
    // This code blows chunks
    xmlNodeSetPtr ssml_nodes;  ///< State Stamp Markup nodes that give joint positions. A libxml2 data structure.
    int ssml_current_index;  ///< Current location within the stamps
    SbTime lastTimeReal; ///< Previous time that was rendered in real wall clock time
    SbTime lastTimeSSML; ///< Previous time that was rendered in the frame of the SSML
    double startTimeSSML; ///< Earliest (oldest) time in the State Stamp markup language
    double endTimeSSML; ///< Latest (newest) time in the ssml
    vector <StateStampJoints> statestamps;
    // FIX: add a time scale concept... faster/slower than realtime
    float timeScale;
#endif

private:
  uint8_t magicNumber; ///< Help make sure we have a valid SceneInfo structure 
};

/// Make a SceneInfo object with all pointers set to zero and not animating
SceneInfo::SceneInfo():camera(0), root(0),draggerSwitch(0),a(0)
		      ,animating(false),render_frames_to_disk(false)
		      ,connect_the_dots(false),connect_sep(0)
		      ,cur_percent(0), cur_mark(0)
#ifdef WITH_LIBXML
		      ,ssml_nodes(0), ssml_current_index(0)
		      ,timeScale(1.)
		       //,lastTimeReal(0), lastTimeSSML(0)
#endif
{
  magicNumber = nominalMagicNumber();
  //camera = 0; root = 0; draggerSwitch = 0; draggerSwitch = 0;
  //a = 0;
  //animating=render_frames_to_disk=false;

  //connect_the_dots=false;
  //connect_sep=0;

  // Animation position
  //cur_percent=0; // how far between waypoints
  //cur_mark=0;   // Which waypoint are we on
}





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



/***************************************************************************
 * KEYBOARD STUFF
 ***************************************************************************/

/// \brief Send help on keyboard shortcuts to stdout
void PrintKeyboardShorts() {
  cout << endl
       << "Keyboard shortcuts:" << endl << endl
       << "\t a - Toggle (A)nimation -  Start/Stop" << endl
    // FIX: implement these!
       << "\t b - Jump to the (B)eginning of the animation sequence" << endl
       << "\t c - (C)onnect the dots.  Put lines between the waypoints" << endl
       << "\t d - (D)ump the current view to an image" << endl
       << "\t f - Write scene graph to a (F)File" << endl
       << "\t h - Print out this (H)elp list" << endl
       << "\t i - (I)nsert a waypoint at the current camera location" << endl

       << "\t n - (N)ext waypoint" << endl
       << "\t p - (P)revious waypoint" << endl
    
       << "\t r - (R)Render frames as they animate" << endl
       << "\t s - (S)how/Hide way point markers" << endl
       << "\t w - Write the current set of (W)aypoints to a file" << endl
       << endl
       << "\t - - Decrease the debug level" << endl
       << "\t + - Incease  the debug level" << endl
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
  // B - Jump to the (B)eginning of the animation sequence
  //
  if (SO_KEY_PRESS_EVENT(keyEvent, B)) {
    DebugPrintf(TRACE,("Keyhit: B - Jump to the (B)eginning of the animation sequence\n"));
    si->cur_percent=0;
    si->cur_mark=0;
    if (si->draggerVec.size()>0) {
      DebugPrintf(VERBOSE,("Setting the camera"));
      SetCameraFromDragger(si->camera, si->draggerVec[si->cur_mark]);
    } else { DebugPrintf(VERBOSE,("No draggers... not setting the camera"));}
    return;
  }

  //
  // C - (C)onnect the dots.  Put lines between the waypoints
  //
  if (SO_KEY_PRESS_EVENT(keyEvent, C)) {
    DebugPrintf(TRACE,("Keyhit: C - Toggle connect the dots. Was %s\n",(si->connect_the_dots?"TR":"FL")));
    si->connect_the_dots = (si->connect_the_dots?false:true); // flip
    cout << "toggling connect_the_dots to " << (si->connect_the_dots?"true":"false") << endl;

    // transition from false -> true
    if (si->connect_the_dots) {
      // create the chain and add it to the sep.
      assert(0==si->connect_sep->getNumChildren ());
      si->connect_sep->addChild(MakeSoLineSet(si->draggerVec));
      assert(1==si->connect_sep->getNumChildren ());
    } else {
      // transition from true -> false
      //assert(1==si->connect_sep->getNumChildren ());
      if (1==si->connect_sep->getNumChildren ()) {
	si->connect_sep->removeChild(0);
      } else { DebugPrintf (VERBOSE,("No child to delete.  Must be first time?")); }
      assert(0==si->connect_sep->getNumChildren ());
    }
    
  }  

  //
  // D - Dump a rendering of the scene
  //
  if (SO_KEY_PRESS_EVENT(keyEvent, D)) {
    DebugPrintf(TRACE,("Keyhit: D - render  %d %d\n",si->a->width_arg,si->a->height_arg));

    float r=-666.,g=-666.,b=-666.;
    if (3!=sscanf(si->a->color_arg,"%f,%f,%f",&r,&g,&b)) {
      cerr << "ERROR: XCORE_BG_COLOR should be 3 floats.  Found...\n" << si->a->color_arg << "\n";
      exit(EXIT_FAILURE);
    }
    r/=255.;  g/=255.;  b/=255.;
    if(0.>r || 1.0<r) {  cerr << "ERROR: red   must be between 0.0 and 1.0\n"; exit(EXIT_FAILURE); }
    if(0.>g || 1.0<g) {  cerr << "ERROR: green must be between 0.0 and 1.0\n"; exit(EXIT_FAILURE); }
    if(0.>b || 1.0<b) {  cerr << "ERROR: blue  must be between 0.0 and 1.0\n"; exit(EXIT_FAILURE); }
    SbColor background(r,g,b);

    size_t frame_num;
    if (!RenderFrameToDisk (string(si->a->basename_arg),string(si->a->type_arg),
			    si->a->width_arg, si->a->height_arg,
			    si->root, frame_num, background) )
      cerr << __FILE__ << " " << __LINE__ << " ERROR: Unable to write your artistic work.\n  You have been sensored." << endl;

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


  //
  // N - Jump to the next waypoint.
  //
  if (SO_KEY_PRESS_EVENT(keyEvent, N)) {
    if (0==si->draggerVec.size()) return;
    si->cur_percent=0;
    si->cur_mark++;
    if (si->cur_mark >= si->draggerVec.size()) si->cur_mark=0;
    SetCameraFromDragger(si->camera, si->draggerVec[si->cur_mark]);
    DebugPrintf(TRACE,("Keyhit: N - now at waypoint %d\n",int(si->cur_mark)));
    return;
  }

  //
  // P - Jump to the previous waypoint.
  //
  if (SO_KEY_PRESS_EVENT(keyEvent, P)) {
    if (0==si->draggerVec.size()) return;
    si->cur_percent=0;
    if (0==si->cur_mark) si->cur_mark=si->draggerVec.size()-1;
    else si->cur_mark--;

    SetCameraFromDragger(si->camera, si->draggerVec[si->cur_mark]);
    DebugPrintf(TRACE,("Keyhit: P - now at waypoint %d\n",int(si->cur_mark)));
    return;
  }



  // R - Toggle render mode for during animation.  If true write all frames to disk
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
///
/// \bug The first frame rendered to disk is not right.  So for now ignore frame 0000
void timerSensorCallback(void *data, SoSensor *sensor) {
  assert(data);
  assert(sensor);
  SceneInfo *si = (SceneInfo *)data;


#ifdef WITH_LIBXML
  // Take care of time updating and joint angles for state stamp XML control
  //cout << "hello" << endl;
  if (si->ssml_nodes) {
      SbTime now = SbTime::getTimeOfDay();
      const double deltaT = si->timeScale*(now.getValue() - si->lastTimeReal.getValue());

      si->lastTimeReal = now;

      DebugPrintf (BOMBASTIC,("deltaT %.4f\n",deltaT));
      si->lastTimeSSML.setValue(si->lastTimeSSML.getValue()+deltaT);

      bool needUpdate=false;  // Flag true if the joint angles have changed

      cout << "deltaT " << deltaT << "  " << si->lastTimeSSML.getValue() << endl;

      if (si->lastTimeSSML.getValue() >= si->endTimeSSML) {
	  DebugPrintf (TRACE,("Looping back to the beginning of the SSML\n"));
	  si->lastTimeSSML.setValue(si->startTimeSSML);
	  si->ssml_current_index=0;
	  needUpdate=true;
      } else {
	  while (si->lastTimeSSML > si->statestamps[si->ssml_current_index].time) {
	      needUpdate=true;
	      si->ssml_current_index++;
	      if (si->lastTimeSSML.getValue() >= si->endTimeSSML) {
		  si->lastTimeSSML.setValue(si->startTimeSSML);
		  si->ssml_current_index=0;
	      }
	  }
      }

      if (needUpdate) {
	  cout << si->ssml_current_index << " ";
	  for (size_t i=0;i<si->statestamps[si->ssml_current_index].rotNodes.size(); i++) {
	      //cout << "setting angle: " << si->statestamps[si->ssml_current_index].angles[i].getValue() << endl;
	      SoSFFloat angle = si->statestamps[si->ssml_current_index].angles[i];
	      si->statestamps[si->ssml_current_index].rotNodes[i]->angle 
		  = angle;
	      cout << angle.getValue() << " ";
	  }
	  cout << endl;
      }
      
      //double currentTimeSSML = si->lastTimeSSML.getValue();
      //DebugPrintf (BOMBASTIC,("new SSML time %.4f\n",currentTimeSSML));

  } else {
      DebugPrintf (BOMBASTIC,("No ssml loaded\n"));
  }
#endif


  if (!si->animating) {return;}  // not turned on right now


  if (si->draggerVec.size()<2) { si->animating=false;  cout << "nothing to animate!" << endl;   return;  }

  //
  // Check to see if we have to switch nodes or loop back to the beginning
  //
  if (si->cur_percent >= 1.0) {
    // FIX: do we include the beginning way point or not
    // -1 gives beginning  -2 give last waypoint as the end
    if (si->a->noloop_flag && (si->draggerVec.size()-1)==si->cur_mark) {
      cout << "Animation finished.  Use 'b' to go back to the beginning." << endl;
      si->animating=false;
      return;
    }
    si->cur_percent=0.;
    si->cur_mark++;
    if (si->draggerVec.size()==si->cur_mark) { si->cur_mark=0; } // back to the beginning
    cout << "Switching to mark #" << si->cur_mark << endl;
  }

  //
  // Calc the camera position and set it!
  //
  SoSpotLightDragger *d1 = si->draggerVec[si->cur_mark];
  SoSpotLightDragger *d2;
  if (si->cur_mark==si->draggerVec.size()-1) { d2 = si->draggerVec[0]; } // Loop back to the first mark
  else d2 = si->draggerVec[si->cur_mark+1];

  SbVec3f pos1 = d1->translation.getValue();
  SbVec3f pos2 = d2->translation.getValue();

  vector<float> v1 = ToVector(pos1);
  vector<float> v2 = ToVector(pos2);
  
  vector<float> v3 = InterpolatePos (v1,v2,si->cur_percent);
  SbVec3f pos3 = ToSbVec3f (v3);

  si->camera->position = pos3;

  SbRotation rot1 = d1->rotation.getValue();
  SbRotation rot2 = d2->rotation.getValue();
  SbRotation newRot = SbRotation::slerp (rot1, rot2, si->cur_percent);

  si->camera->orientation = newRot;

  DebugPrintf (VERBOSE,("Mark: %d      Step:  %f\n",int(si->cur_mark),si->cur_percent));
  si->cur_percent += si->a->percent_arg; // User configurable jump



  if (si->render_frames_to_disk) {
    printf ("rendering frames to disk currently crashes the X11 server for Apple\n");
#if 0
    DebugPrintf (TRACE,("ANIMATION: Rendering frame to disk file\n"));
    size_t frame_num;
    if (!RenderFrameToDisk (string(si->a->basename_arg),string(si->a->type_arg),
			si->a->width_arg, si->a->height_arg,
			si->root, frame_num,si->background)
	) {
      cerr << "ERROR: Unable to write your artistic work.\n  You have been sensored." << endl;
    }
    DebugPrintf(TRACE+1,("ANIMATION: Finished writing frame number %04d\n",int(frame_num)));
#endif
  }

  return;
}


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

/// \brief Convert the libxml2 tree to a C++ vector based data structure to avoid trouble

bool LoadStateStampToVectors(vector<StateStampJoints> &statestamps, xmlNodeSetPtr nodeset, SoNode *root) {
    bool success=true;
    assert (root);

    for (int i=0; i<nodeset->nodeNr; i++) {
	xmlNodePtr n= nodeset->nodeTab[i];
	const xmlChar *timeStampStr = xmlGetProp(n,(xmlChar*)"Time");
	//cout << "Loading time: " << timeStampStr << endl;
	struct StateStampJoints ssj;
	double timeStamp;
	sscanf((const char *)timeStampStr,"%lf",&timeStamp);
	ssj.time = SbTime(timeStamp);

	// Loop through all the joints
	for (xmlNodePtr joint=n->children; joint ; joint=joint->next) {
	    if (!streq("Joint",(const char *)joint->name)) {
		if (!streq("text",(const char *)joint->name)) 
		    cout << "\nIgnoring: "<< joint->name << endl;
		continue; // not the right child
	    }
	    const char *name = (const char*)(xmlGetProp(joint,(xmlChar*)"Name"));
	    const char *radStr = (const char*)(xmlGetProp(joint,(xmlChar*)"Radians"));
	    //printf ("%.4f %s\n",timeStamp,name);
	    SoSFFloat angle;
	    angle.setValue(strtod(radStr,0));
	    //cout << "\t" << name << " -> " << angle.getValue() << endl;

	    // Find the node so that we can cache the pointer and get back to it quickly
	    SoSearchAction mySearchAction;
	    mySearchAction.setType(SoRotationXYZ::getClassTypeId(), FALSE); // Only search for SoRotationsXYZ
	    mySearchAction.setName(name);
	    mySearchAction.setInterest(SoSearchAction::FIRST);
	    mySearchAction.apply(root);
	    SoPath *path = mySearchAction.getPath();
	    if (!path) {
		cerr << "WARNING can't find RotationXYZ named: " << name << endl;
		continue;  // FIX: should this just fail?
	    }
	    SoRotationXYZ *rotNode = (SoRotationXYZ *)(path->getTail());

	    ssj.rotNodes.push_back(rotNode);
	    ssj.angles.push_back(angle);
	    
	}

	statestamps.push_back(ssj);
    }
    
    //cerr << "Boom" << endl;    exit(EXIT_FAILURE);
    return success;
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


#if 1
  if (!CheckLibraryPath()) { cerr << "Bailing.  Library path check failed." << endl;  return(EXIT_FAILURE);}
#else
  cerr << "Skipping lib check!!!  DANGER\n";  // Use this for in the debugger if you must
#endif


  if (a.list_given) { ListWriteFileTypes();  return (EXIT_SUCCESS); }
  if (a.keys_given) { PrintKeyboardShorts();  return (EXIT_SUCCESS); }
  //
  // Check range of arguments
  //
  if (a.interval_given) 
    if (!InRange(a.interval_arg,0.001,10.0)) {cerr<<"ERROR: Interval must be 0.001<=i<=10.0"<<endl; return (EXIT_FAILURE);}
  if (a.percent_given)
    if (!InRange(a.percent_arg,0.0,1.0)) {cerr<<"ERROR: Interval must be 0.0<=i<=1.0"<<endl; return (EXIT_FAILURE);}


  //
  // Init the SoDB and SimVoleon - Must do the init before any use of Coin3D/IV data types/structures
  //
  QWidget* myWindow = SoQt::init(argv[0]);
  if ( myWindow==NULL ) return (EXIT_FAILURE);

  SceneInfo *si = new SceneInfo;
  si->a=&a;



  // Use QWidget to give a better window size
  myWindow->resize(a.width_arg,a.height_arg);
#ifndef NO_VOLEON
  SoVolumeRendering::init();
#endif

  if (a.type_given)
    if (!CheckTypeValid(string(a.type_arg)))
      { cerr << "File type not valid: --type=" << a.type_arg << endl; return (EXIT_FAILURE); }

  DebugPrintf(VERBOSE,("FIX: check the range on width and height!\n"));


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


#ifdef WITH_LIBXML

  if (!a.statestamp_given) {
      DebugPrintf(VERBOSE,("No state stamp xml history given\n"));
  } else {
      DebugPrintf(TERSE,("statestamp... %s\n",a.statestamp_arg));
      //DebugPrintf(VERBOSE,("statestamp_orig... %s\n",a.statestamp_orig));
  }

  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  if (a.statestamp_given) {
      si->timeScale=a.timescale_arg;
      DebugPrintf(TERSE,("TimeScale (higher is faster.  1==realtime): %f\n",a.timescale_arg));

      
      xmlDocPtr ssmldoc=xmlParseFile(a.statestamp_arg);
      if (!ssmldoc) { fprintf(stderr,"FATAL ERROR: failed to load state stamp markup file.\n"); exit(EXIT_FAILURE); }
      //xmlNodePtr cur = xmlDocGetRootElement(ssmldoc);
      xmlXPathContextPtr context = xmlXPathNewContext(ssmldoc);
      if (!context) {fprintf(stderr,"FATAL ERROR: Can not get xmlXPathContextPtr!\n"); exit(EXIT_FAILURE);}
      xmlXPathObjectPtr result = xmlXPathEvalExpression((xmlChar *)"//StateStampHistory/StateStamp", context);
      xmlXPathFreeContext(context);
      if (!result) {fprintf(stderr,"FATAL ERROR: XPath meltdown!\n  There will be no fighting in the war room.\n"); exit(EXIT_FAILURE);}
      if(xmlXPathNodeSetIsEmpty(result->nodesetval)) {xmlXPathFreeObject(result);fprintf(stderr,"FATAL ERROR: not statestamps found\n");exit(EXIT_FAILURE);}
      xmlNodeSetPtr nodeset=result->nodesetval;
      DebugPrintf(TRACE,("Found %i state stamp nodes\n",nodeset->nodeNr));
      const int numnodes=nodeset->nodeNr;

      const xmlChar *startTime=xmlGetProp(nodeset->nodeTab[0],(xmlChar*)"Time");
      const xmlChar *endTime=xmlGetProp(nodeset->nodeTab[numnodes-1],(xmlChar*)"Time");
      DebugPrintf(TRACE,("  Time range: %s -> %s \n",startTime,endTime));

      sscanf((const char *)startTime,"%lf",&(si->startTimeSSML));
      sscanf((const char *)endTime,"%lf",&(si->endTimeSSML));
      DebugPrintf(BOMBASTIC,("  Time range: %.4f -> %.4f \n",si->startTimeSSML,si->endTimeSSML));

      si->ssml_nodes=nodeset;
      si->lastTimeReal.setToTimeOfDay();
      double ssmlStart;
      sscanf((const char *)endTime,"%lf",&ssmlStart);
      si->lastTimeSSML.setValue(ssmlStart);

      DebugPrintf (BOMBASTIC,("  Initial real time: %10.5f\n",si->lastTimeReal.getValue()));
      DebugPrintf (BOMBASTIC,("  Initial SSML time: %10.5f\n",si->lastTimeSSML.getValue()));

      // Put all of the critical XML values into a vector such that the main loop is fast and simple
      // Crappy ass knockoff of viz 1.0
      if (!LoadStateStampToVectors(si->statestamps, nodeset,si->root)) {
	  fprintf (stderr,"You are hosed bub.  Could not load the ssml for some ugly reason\n");
	  exit(EXIT_FAILURE);
      }

      
      //xmlNodeSetPtr nodeset=0;
  } // statestamp_given

  //printf ("EARLY EXIT\n");
  //exit(1);
  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////

#endif // WITH_LIBXML



  // Set background color
  {
    float r=-666.,g=-666.,b=-666.;
    if (3!=sscanf(a.color_arg,"%f,%f,%f",&r,&g,&b)) {
      cerr << "ERROR: XCORE_BG_COLOR should be 3 floats.  Found...\n" << a.color_arg << "\n";
      exit(EXIT_FAILURE);
    }
    r/=255.;  g/=255.;  b/=255.;
    if(0.>r || 1.0<r) {  cerr << "ERROR: red   must be between 0.0 and 1.0\n"; exit(EXIT_FAILURE); }
    if(0.>g || 1.0<g) {  cerr << "ERROR: green must be between 0.0 and 1.0\n"; exit(EXIT_FAILURE); }
    if(0.>b || 1.0<b) {  cerr << "ERROR: blue  must be between 0.0 and 1.0\n"; exit(EXIT_FAILURE); }
    SbColor c(r,g,b);
    si->background = c;
    myViewer->setBackgroundColor(c);
  }

  // View all so we get a sane initial view
  si->camera->viewAll(si->root,myViewer->getViewportRegion(),10);
  
  myViewer->setSceneGraph( si->root );
  myViewer->show();


  {
    // Set up callback
    SoEventCallback 	*keyEventHandler = new SoEventCallback;
    assert (keyEventHandler);
    keyEventHandler->addEventCallback(SoKeyboardEvent::getClassTypeId(), keyPressCallback, si);
    si->root->addChild(keyEventHandler);
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
  si->connect_sep = new SoSeparator; // Maybe this and the switch above should be in the constructor!
  si->connect_sep->setName("connect_the_dots");
  si->root->addChild(si->connect_sep);


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

