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


#ifndef __INVENTOR_UTILITIES_H__
#define __INVENTOR_UTILITIES_H__

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

using namespace std;

/***************************************************************************
 * Vector Utilities
 ***************************************************************************/

SbVec3f ToSbVec3f (const vector<float> &v);
vector<float> ToVector (const SbVec3f &SbV);
SbVec3f InterpolateVec(const SbVec3f &v1, SbVec3f &v2, const float percent);
SbRotation InterpolateRotations(const SbRotation &rot1,const SbRotation &rot2,const float percent);
vector<float>InterpolatePos(const vector<float> &v1, const vector<float> &v2,const float percent);

/***************************************************************************
 * Utilities
 ***************************************************************************/


/// \brief Given a draggers set the camera view from it
///
/// This is the simpler case where we do not have to interpolate
/// between to draggers
void SetCameraFromDragger(SoCamera *c,SoSpotLightDragger *d);

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
			SoNode *root, size_t &frame_num, const SbColor &background);



/// \brief Handle the --list command line option.  Spits out the list
/// of filetypes that work for rendering to files
void ListWriteFileTypes();

/// \brief Check with coin/simage to see if the image extension works
/// \param type String of the file extension.  For example \a rgb, \a png, \a jpg
/// \return \a false if something went really bad with the lookup.
bool CheckTypeValid(const string &type);

/// \brief Load dragger/way points from a disk file
/// \param filename Ascii file to load from
/// \param root Where in the scene graph location to add the draggers  (should be draggerSwitch
/// \param draggerVec A vector in which to keep the way points handy
/// \return \a false if trouble loading the draggers/waypoints
bool LoadSpotLightDraggers(const string filename, SoSeparator *root, vector<SoSpotLightDragger *> &draggerVec);



/// \brief Save an ascii file of all the draggers for editing and reloading
/// \param filename file to write to
/// \param draggerVec pointers to all the SoSpotLightDragger way points
/// \return \a false if had trouble writing the file
bool SaveSpotLightDraggersAscii(const string &filename, vector<SoSpotLightDragger *> &draggerVec);

/// \brief Write a scene graph to disk as ascii
/// \param filename File to write ascii IV file to
/// \param root Starting point of the scene graph to same
/// \return \a false if we were not able to open the file.
bool WriteSceneGraph (const string &filename, SoNode *root);


/// \brief Help avoid problems with library paths.  Especially on Darwin/OSX
/// \return true if have the proper path.  False if found something bad or lacking
bool CheckLibraryPath();

#endif //  __INVENTOR_UTILITIES_H__
