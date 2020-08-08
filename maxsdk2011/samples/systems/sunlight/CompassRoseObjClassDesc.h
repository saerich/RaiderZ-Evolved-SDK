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

class CompassRoseObjClassDesc : public ClassDesc 
{
public:
	int IsPublic();
	void *Create(BOOL loading = FALSE);
	const TCHAR *ClassName();
	SClass_ID SuperClassID();
	Class_ID ClassID();
	const TCHAR* Category();
	void ResetClassParams(BOOL fileReset);
};

#pragma managed(pop)
