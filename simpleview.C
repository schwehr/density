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

#include <Inventor/Qt/SoQt.h>
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>
#include <Inventor/nodes/SoSeparator.h>

#include <VolumeViz/nodes/SoVolumeRendering.h>


#include <iostream>
using namespace std;

int
main( int narg, char* argv[] )
{
    QWidget* myWindow = SoQt::init(argv[0]);
    SoVolumeRendering::init();
    if ( myWindow==NULL ) return (EXIT_FAILURE);
    if ( narg != 2 ) {
      cerr << " USAGE: " << argv[0]<< " <filename>" << endl <<endl;
      return (EXIT_FAILURE);
    }

    SoQtExaminerViewer* myViewer = new SoQtExaminerViewer(myWindow);
    {
      SoInput mySceneInput;
      if ( !mySceneInput.openFile( argv[1] ))  return (EXIT_FAILURE);

      SoSeparator* myGraph = SoDB::readAll(&mySceneInput);
      if ( !myGraph ) return (EXIT_FAILURE);
      mySceneInput.closeFile();
      
      myViewer->setSceneGraph( myGraph );
      myViewer->show();
    }
    SoQt::show(myWindow);
    SoQt::mainLoop();

    // probably can never reach here.
    return (EXIT_FAILURE);
}

