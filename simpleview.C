#include <Inventor/Qt/SoQt.h>
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/nodes/SoPickStyle.h>
#include <Inventor/nodes/SoLightModel.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoTransformSeparator.h>
#include <Inventor/nodes/SoScale.h>
#include <Inventor/nodes/SoEventCallback.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/sensors/SoTimerSensor.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/manips/SoClipPlaneManip.h>
#include <Inventor/events/SoKeyboardEvent.h>

#include <VolumeViz/nodes/SoTransferFunction.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/nodes/SoVolumeRender.h>
#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <VolumeViz/readers/SoVRVolFileReader.h>

int
main( int narg, char* argv[] )
{
    QWidget* myWindow = SoQt::init(argv[0]);
    SoVolumeRendering::init();
    if ( myWindow==NULL ) return 0;
    if ( narg != 2 )
    {
        printf("Syntax:\noifileviewer <filename>\n\n");
        return 0;
    }

    SoInput mySceneInput;
    if ( !mySceneInput.openFile( argv[1] ))
        return 0;

    SoSeparator* myGraph = SoDB::readAll(&mySceneInput);
    if ( !myGraph ) return 0;
    mySceneInput.closeFile();

    SoQtExaminerViewer* myViewer= new SoQtExaminerViewer(myWindow);
    myViewer->setSceneGraph( myGraph );
    myViewer->show();

    SoQt::show(myWindow);
    SoQt::mainLoop();

    return 0;
}

