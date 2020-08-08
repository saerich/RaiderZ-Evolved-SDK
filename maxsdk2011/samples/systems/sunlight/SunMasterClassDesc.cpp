// Copyright 2009 Autodesk, Inc.  All rights reserved.
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law.  They may not
// be disclosed to, copied or used by any third party without the prior written
// consent of Autodesk, Inc.

#include "SunMasterClassDesc.h"
#include "SunMasterCreateMode.h"
#include "sunclass.h"
#include "sunlight.h"

#pragma managed

int SunMasterClassDesc::IsPublic() 
{ 
	return 1; 
}

void * SunMasterClassDesc::Create(BOOL) 
{ 
	return new SunMaster(false); 
}

// This method returns the name of the class.  This name appears 
// in the button for the plug-in in the MAX user interface.
const TCHAR* SunMasterClassDesc::ClassName() 
{
	return MaxSDK::GetResourceString(IDS_SUN_CLASS);
}

// This is the method of the class descriptor that actually begins the 
// creation process in the viewports.
int SunMasterClassDesc::BeginCreate(Interface *i)
{
	// Save the interface pointer passed in.  This is used to call 
	// methods provided by MAX itself.
	IObjCreate *iob = i->GetIObjCreate();

	SunMasterCreateMode::GetInstance()->Begin( iob, this, false );
	// Set the current command mode to the SunMasterCreateMode.
	iob->PushCommandMode( SunMasterCreateMode::GetInstance() );

	return TRUE;
}

// This is the method of the class descriptor that terminates the 
// creation process.
int SunMasterClassDesc::EndCreate(Interface *i)
{
	SunMasterCreateMode::GetInstance()->End();
	// Remove the command mode from the command stack.
	i->RemoveMode( SunMasterCreateMode::GetInstance() );
	return TRUE;
}

SClass_ID SunMasterClassDesc::SuperClassID() 
{ 
	return SYSTEM_CLASS_ID; 
} 

Class_ID SunMasterClassDesc::ClassID() 
{ 
	return SUNLIGHT_CLASS_ID; 
}

const TCHAR* SunMasterClassDesc::Category() 
{ 
	return _T("");  
}
