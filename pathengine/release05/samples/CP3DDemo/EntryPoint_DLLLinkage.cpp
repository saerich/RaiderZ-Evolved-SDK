//**********************************************************************
//
// Copyright (c) 2007
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#include "Header.h"
#include "EntryPoint.h"
#include "MessageBox.h"
#include "Demo.h"
#include "LoadContentProcessing3D.h"
#include "i_contentprocessing3d.h"
#include <fstream>
#include <sstream>

class cErrorHandler : public iErrorHandler
{
    cErrorHandler(){}
public:
    eAction handle(const char* errorType, const char* errorString, const char *const* attributes)
    {
        ShowMessageBox(errorType, errorString);
        return CONTINUE;
    }
    static cErrorHandler& refInstance()
    {
        static cErrorHandler theInstance;
        return theInstance;
    }
};

void
demo(iPathEngine* pathEngine, iTestBed* testBed)
{
  // load the 3D content processing DLL and obtain the root interface pointer

    iContentProcessing3D* cp3D = LoadContentProcessing3D("ContentProcessing3D.dll", &cErrorHandler::refInstance());

  // check if the dll loaded is compatible with the headers used for compilation

    if(cp3D->getInterfaceMajorVersion() != CONTENT_PROCESSING_3D_INTERFACE_MAJOR_VERSION
        ||
        cp3D->getInterfaceMinorVersion() < CONTENT_PROCESSING_3D_INTERFACE_MINOR_VERSION)
    {
        cErrorHandler::refInstance().handle("Fatal", "LoadPathEngine: 3D content processing dll version is incompatible with headers used for compilation.",0);
        return;
    }

    Demo(pathEngine, testBed, cp3D);
}
