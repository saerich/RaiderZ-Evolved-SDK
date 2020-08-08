// Copyright 2009 Autodesk, Inc.  All rights reserved.
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law.  They may not
// be disclosed to, copied or used by any third party without the prior written
// consent of Autodesk, Inc.

#pragma once
#include "NativeInclude.h"

#pragma managed(push, on)


class SunMasterClassDesc : public ClassDesc {
public:
	int IsPublic();
	void * Create(BOOL loading = FALSE);
	// This method returns the name of the class.  This name appears 
	// in the button for the plug-in in the MAX user interface.
	const TCHAR* ClassName();
	int BeginCreate(Interface *i);
	int EndCreate(Interface *i);
	SClass_ID SuperClassID();
	Class_ID ClassID();
	const TCHAR* Category();
};

#pragma managed(pop)
