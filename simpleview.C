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

// Inventor/Coin
#include <Inventor/Qt/SoQt.h>
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>
#include <Inventor/nodes/SoSeparator.h>

#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/events/SoLocation2Event.h>
#include <Inventor/events/SoKeyboardEvent.h>

#include <Inventor/nodes/SoEventCallback.h>

#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoSwitch.h>

#include <Inventor/draggers/SoTranslate1Dragger.h>
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
 * KEYBOARD STUFF
 ***************************************************************************/

class SceneInfo {
public:
  SceneInfo();
  SoCamera *camera;
  SoSeparator *root;

  vector<SoSpotLightDragger *> draggerVec; // All of the draggers that have been added.
  //SoSeparator *draggerSep;
  SoSwitch *draggerSwitch;

  gengetopt_args_info *a; ///< a for args.  These are the command line arguments

  bool animating;  // set to true to start animation;
};

/// Make a SceneInfo object with all pointers set to zero and not animating
SceneInfo::SceneInfo() {
  camera = 0; root = 0; draggerSwitch = 0; draggerSwitch = 0; a = 0;
  animating=false;
}


/// \brief Gets called whenever the user presses a key in interactive mode
/// \param data This will be the SceneInfo structure.
void
keyPressCallback(void *data, SoEventCallback *cb) {
  assert(data); assert(cb);

  SceneInfo *si = (SceneInfo *)data;

  assert (si);
  assert (si->camera);
  assert (si->root);

  const SoKeyboardEvent *keyEvent = (const SoKeyboardEvent *)cb->getEvent();

  cerr << "Keypressed: " << keyEvent->getPrintableCharacter() << endl;

  if (SO_KEY_PRESS_EVENT(keyEvent, D)) {
    DebugPrintf(TRACE,("Keyhit: D"));
    cerr << "d" << endl;
    return;
  }


} // keyPressCallback


/***************************************************************************
 * MAIN
 ***************************************************************************/

int main(int argc, char *argv[])
{
  bool ok=true;

  gengetopt_args_info a;
  if (0!=cmdline_parser(argc,argv,&a)) {
    cerr << "FIX: should never get here" << endl;
    cerr << "Early exit" << endl;
    return (EXIT_FAILURE);
  }

#ifdef NDEBUG
  if (a.verbosity_given) {
    cerr << "Verbosity is totally ignored for optimized code.  Continuing in silent mode" << endl;
  }
#else // debugging
  debug_level = a.verbosity_arg;
  DebugPrintf(TRACE,("Debug level = %d\n",debug_level));
#endif


  QWidget* myWindow = SoQt::init(argv[0]);
  SoVolumeRendering::init();
  if ( myWindow==NULL ) return (EXIT_FAILURE);

  SoSeparator *root = new SoSeparator;
  root->ref();

  SoQtExaminerViewer* myViewer = new SoQtExaminerViewer(myWindow);
  for (size_t i=0;i<a.inputs_num;i++) {
    DebugPrintf (TRACE,("Adding file: %s\n",a.inputs[i]));
    SoInput mySceneInput;
    if ( !mySceneInput.openFile( a.inputs[i] ))  return (EXIT_FAILURE);

    SoSeparator* node = SoDB::readAll(&mySceneInput);
    if ( !node ) {cerr << "failed to load iv file: "<<a.inputs[i] << endl; return (EXIT_FAILURE);}
    mySceneInput.closeFile();

    root->addChild(node);
  }
  myViewer->setSceneGraph( root );
  myViewer->show();

  SceneInfo *si = new SceneInfo;
  si->root=root;
  si->a=&a;
  si->camera = myViewer->getCamera();

  {
    // Set up callback
    SoEventCallback 	*keyEventHandler = new SoEventCallback;
    assert (keyEventHandler);
    keyEventHandler->addEventCallback(SoKeyboardEvent::getClassTypeId(), keyPressCallback, si);
    root->addChild(keyEventHandler);
  }


  SoQt::show(myWindow);
  SoQt::mainLoop();

  // probably can never reach here.
  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}

