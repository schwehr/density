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



/***************************************************************************
 * INCLUDES
 ***************************************************************************/

// c includes
#include <cstdio>
#include <cassert>

// C++ includes
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

//
// Inventor/Coin
//
#include <Inventor/Qt/SoQt.h>
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>

#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/events/SoLocation2Event.h>
#include <Inventor/events/SoKeyboardEvent.h>

#include <Inventor/nodes/SoEventCallback.h>

#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoPointLight.h>
#include <Inventor/nodes/SoSeparator.h>

#include <Inventor/draggers/SoTranslate1Dragger.h>
#include <Inventor/draggers/SoSpotLightDragger.h>

#include <Inventor/SoOffscreenRenderer.h>

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
private:
  uint8_t magicNumber; ///< Help make sure we have a valid SceneInfo structure 
public:
  SceneInfo();
  SoCamera *camera;
  //SoNode *root;
  SoSeparator *root;

  vector<SoSpotLightDragger *> draggerVec; // All of the draggers that have been added.
  //SoSeparator *draggerSep;
  SoSwitch *draggerSwitch;

  gengetopt_args_info *a; ///< a for args.  These are the command line arguments

  bool animating;  // set to true to start animation;

  static uint8_t nominalMagicNumber() {return(178);}
  uint8_t getMagicNumber() const {return(magicNumber);}
};

/// Make a SceneInfo object with all pointers set to zero and not animating
SceneInfo::SceneInfo() {
  magicNumber = nominalMagicNumber();
  camera = 0; root = 0; draggerSwitch = 0; draggerSwitch = 0; a = 0;
  animating=false;
}


/***************************************************************************
 * Utilities
 ***************************************************************************/
/// \brief Handle the --list command line option
void ListWriteFileTypes() {
  if (!SoDB::isInitialized()) SoDB::init();

  // FIX: make sure there are no leaks!
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
/// \param root Where in the scene graph location to add the draggers
/// \param draggerVec A vector in which to keep the way points handy
/// \return \a false if trouble loading the draggers/waypoints
bool
LoadSpotLightDraggers (const string filename, SoSeparator *root, vector<SoSpotLightDragger *> &draggerVec) {
  bool ok=true;
  assert(root);
  DebugPrintf(TRACE,("LoadSpotLightDraggers %s %d\n",filename.c_str(),int(draggerVec.size())));

  // FIX: nuke the old waypoints

  return (ok);
}

bool
SaveSpotLightDraggers (const string filename, vector<SoSpotLightDragger *> &draggerVec) {
  bool ok=true;
  DebugPrintf(TRACE,("SaveSpotLightDraggers %s %d\n",filename.c_str(),int(draggerVec.size())));

  if (0==draggerVec.size()) {
    cerr << "WARNING: skipping save.  There are zero waypoints." << endl;
    return(false);
  }

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
    axis.getValue(a1,a2,a3);
    d->rotation.getValue(axis,r_angle);
    SbVec3f xyz(d->translation.getValue());
    float x, y, z;
    xyz.getValue(x,y,z);
    const float angle = d->angle.getValue(); // FIX: do not currently use angle for anything
    o << a1 << " "<< a2 << " "<< a3 
      << " " << r_angle
      << " " << x << " " << y << " " << z
      << " " << angle << endl;

  }

  return (ok);
} 

/***************************************************************************
 * KEYBOARD STUFF
 ***************************************************************************/

/// \brief Send help on keyboard shortcuts to stdout
void PrintKeyboardShorts() {
  cout << endl
       << "Keyboard shortcuts:" << endl << endl
       << "\ta - Toggle animation -  Start/Stop" << endl
       << "\td - Dump the current view to an image" << endl
       << "\th - Print out this help list" << endl
       << "\ts - Show/Hide way point markers" << endl
       << "\tw - Write the current set of waypoints to a file" << endl
       << endl
       << "\tDo not forget to use the right mouse button or option-left mouse for more"<<endl
       << endl;

}

/// \brief Gets called whenever the user presses a key in interactive mode
/// \param data This will be the SceneInfo structure.
void
keyPressCallback(void *data, SoEventCallback *cb) {
  assert(data); assert(cb);

  SceneInfo *si = (SceneInfo *)data;
  assert(si->getMagicNumber()==SceneInfo::nominalMagicNumber());

  assert (si);
  assert (si->camera);
  assert (si->root);

  const SoKeyboardEvent *keyEvent = (const SoKeyboardEvent *)cb->getEvent();

  //cerr << "Keypressed: " << keyEvent->getPrintableCharacter() << endl;
  DebugPrintf(VERBOSE,("keyPressCallback: '%c'\n",keyEvent->getPrintableCharacter()));

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
    cerr << "d" << endl;

    SbViewportRegion viewport(si->a->width_arg,si->a->height_arg);
    SoOffscreenRenderer *renderer = new SoOffscreenRenderer(viewport);

    SbVec2s maxRes = renderer->getMaximumResolution();
    DebugPrintf(VERBOSE,("maxRes: %d %d\n",maxRes[0], maxRes[1]));
    
    SbBool ok = renderer->render(si->root);
    if (!ok) {  cerr << "Unable to render!" << endl;  return;   }
    {
      static int count=0;
      char countBuf[12];
      snprintf(countBuf,12,"%04d",count++);
      const string filename =
	string(si->a->basename_arg)
	+ string(countBuf)
	+ string(".")
	+ string(si->a->type_arg);
      SbName filetype(si->a->type_arg);
      ok = renderer->writeToFile(filename.c_str(), filetype);
      if (!ok) {cerr << "Failed to render" << endl; }
    }
    delete (renderer);
    return;
  } // KEY_PRESS D - dump screen

  if (SO_KEY_PRESS_EVENT(keyEvent, H)) { PrintKeyboardShorts(); return; }

  //
  // S - show/hide waypoint markers
  //
  if (SO_KEY_PRESS_EVENT(keyEvent, H)) {
    //cout << "(H)ide/show way points\n";
    if (SO_SWITCH_ALL == si->draggerSwitch->whichChild.getValue())
      si->draggerSwitch->whichChild = SO_SWITCH_NONE;
    else si->draggerSwitch->whichChild = SO_SWITCH_ALL;
    return;
  }


  //
  // W - write waypoints to a file
  //
  if (SO_KEY_PRESS_EVENT(keyEvent, W)) {
    const string filename("tmp.wpt");
    DebugPrintf(TRACE,("Keyhit: W - write waypoints: %s\n",filename.c_str()));
    if (!SaveSpotLightDraggers(filename, si->draggerVec))
      cerr << "WARNING: waypoint save to " << filename << "failed." << endl;
    return;
  } // KEY_PRESS W - write waypoints

  cout << "Add +/- for up and down on debug level" << endl;

} // keyPressCallback





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
//   si->root=root;
//   //si->root=myViewer->getSceneManager()->getSceneGraph();

//   //si->camera = myViewer->getCamera();


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

  
//   SceneInfo *si = new SceneInfo;
//   si->root=root;
//   //si->root=myViewer->getSceneManager()->getSceneGraph();
//   si->a=&a;
//   //si->camera = myViewer->getCamera();



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



  SoQt::show(myWindow);
  SoQt::mainLoop();

  // probably can never reach here.
  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}

