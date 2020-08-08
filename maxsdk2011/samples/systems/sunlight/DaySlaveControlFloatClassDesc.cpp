// Copyright 2009 Autodesk, Inc. All rights reserved.
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law. They may not
// be disclosed to, copied or used by any third party without the prior written
// consent of Autodesk, Inc.

#include "DaySlaveControlFloatClassDesc.h"
#include "sunclass.h"
#include "sunlight.h"

#pragma managed


int DaySlaveControlFloatClassDesc::IsPublic() 
{ 
	return 0; 
}

void * DaySlaveControlFloatClassDesc::Create(BOOL) 
{ 
	return new SlaveControl(true); 
}

const TCHAR * DaySlaveControlFloatClassDesc::ClassName() 
{
	return MaxSDK::GetResourceString(IDS_SLAVE_FLOAT_CLASS);
}

SClass_ID DaySlaveControlFloatClassDesc::SuperClassID() 
{ 
	return CTRL_FLOAT_CLASS_ID; 
}

Class_ID DaySlaveControlFloatClassDesc::ClassID() 
{ 
	return Class_ID(DAYLIGHT_SLAVE_CONTROL_CID1, DAYLIGHT_SLAVE_CONTROL_CID2); 
}

// The slave controllers don't appear in any of the drop down lists, 
// so they just return a null string.
const TCHAR* DaySlaveControlFloatClassDesc::Category()
{ 
	return _T(""); 
}
