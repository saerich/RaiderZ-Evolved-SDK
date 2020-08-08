// Copyright 2009 Autodesk, Inc. All rights reserved.
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law. They may not
// be disclosed to, copied or used by any third party without the prior written
// consent of Autodesk, Inc.

#include "SlaveControlPosClassDesc.h"
#include "sunclass.h"
#include "sunlight.h"

#pragma managed

int SlaveControlPosClassDesc::IsPublic() 
{ 
	return 0; 
}

void * SlaveControlPosClassDesc::Create(BOOL) 
{ 
	return new SlaveControl(false); 
}

const TCHAR * SlaveControlPosClassDesc::ClassName() 
{
 return MaxSDK::GetResourceString(IDS_SLAVE_POS_CLASS);
}

SClass_ID SlaveControlPosClassDesc::SuperClassID() 
{ 
	return CTRL_POSITION_CLASS_ID; 
}

Class_ID SlaveControlPosClassDesc::ClassID() 
{ 
	return Class_ID(SLAVE_CONTROL_CID1, SLAVE_CONTROL_CID2); 
}

// The slave controllers don't appear in any of the drop down lists, 
// so they just return a null string.
const TCHAR* SlaveControlPosClassDesc::Category() 
{ 
	return _T(""); 
}


