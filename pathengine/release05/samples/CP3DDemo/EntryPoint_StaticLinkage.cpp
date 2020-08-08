//**********************************************************************
//
// Copyright (c) 2006
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#include "platform_common/Header.h"
#include "libs/Content_Processing_3D/interface/cContentProcessing3D.h"
#include "libs/TestBed_Application/interface/ClientEntryPoint.h"
#include "libs/TestBed_Interface/interface/Singletons_TestBed.h"
#include "platform_common/BasicErrorHandler.h"
#include "common/MemoryLog.h"
#include "common/MemoryTracking.h"
#include "common/MemoryOverwriteChecking.h"
#include "common/MemoryOverwriteChecking2.h"
#include "common/interface/Error.h"
#include "Demo.h"

cContentProcessing3D gContentProcessing3D;

void ClientEntryPoint(osinterface *ip, int argc, char **argv)    
{        
    SetGlobalErrorHandler(&cBasicErrorHandler::refInstance());
    nSingletons::init_TestBed(ip);
    Demo(&nSingletons::pathEngineI(), &nSingletons::testBedI(), &gContentProcessing3D);
    nSingletons::shutDown_TestBed();
}
