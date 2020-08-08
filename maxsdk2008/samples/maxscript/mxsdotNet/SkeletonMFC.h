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
// DESCRIPTION: SkeletonMFC.h : main header file for SkeletonMFC.cpp
// AUTHOR: Stephane.Rouleau - created Jan.1.2006
//***************************************************************************/
// 

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

// CSkeletonMFCApp
// See SkeletonMFC.cpp for the implementation of this class
//

class CSkeletonMFCApp : public CWinApp
{
public:
	CSkeletonMFCApp();

// Overrides
public:
	virtual BOOL InitInstance();
 
	virtual int ExitInstance();
// Overrides

 // ClassWizard generated virtual function overrides

 //{{AFX_VIRTUAL(CPlugInApp)

 //}}AFX_VIRTUAL
 //{{AFX_MSG(CPlugInApp)

 // NOTE - the ClassWizard will add and remove member functions here.

 // DO NOT EDIT what you see in these blocks of generated code !

 //}}AFX_MSG


	DECLARE_MESSAGE_MAP()
};
