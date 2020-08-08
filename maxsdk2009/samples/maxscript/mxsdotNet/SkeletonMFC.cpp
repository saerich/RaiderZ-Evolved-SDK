//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: SkeletonMFC.cpp : Defines the initialization routines for the DLL.
// AUTHOR: Stephane.Rouleau - created Jan.1.2006
//***************************************************************************/
// 

#include "stdafx.h"
#include "SkeletonMFC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//    any functions exported from this DLL which call into
//    MFC must have the AFX_MANAGE_STATE macro added at the
//    very beginning of the function.
//
//    For example:
//
//    extern "C" BOOL PASCAL EXPORT ExportedFunction()
//    {
//       AFX_MANAGE_STATE(AfxGetStaticModuleState());
//       // normal function body here
//    }
//
//    It is very important that this macro appear in each
//    function, prior to any calls into MFC.  This means that
//    it must appear as the first statement within the 
//    function, even before any object variable declarations
//    as their constructors may generate calls into the MFC
//    DLL.
//
//    Please see MFC Technical Notes 33 and 58 for additional
//    details.
//

// CSkeletonMFCApp

BEGIN_MESSAGE_MAP(CSkeletonMFCApp, CWinApp)
END_MESSAGE_MAP()

// CSkeletonMFCApp construction

CSkeletonMFCApp::CSkeletonMFCApp()
{
   // TODO: add construction code here,
   // Place all significant initialization in InitInstance

   // NEVER use AfxGetInstance() here
}

// The one and only CSkeletonMFCApp object

CSkeletonMFCApp theApp;

// CSkeletonMFCApp initialization

BOOL CSkeletonMFCApp::InitInstance()
{
	CWinApp::InitInstance();
	// You CAN use AfxGetInstance() here

	AfxEnableControlContainer();

	return TRUE;
}

int CSkeletonMFCApp::ExitInstance()
{
	return 0;
}
