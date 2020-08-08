// Copyright 2009 Autodesk, Inc.  All rights reserved.
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law.  They may not
// be disclosed to, copied or used by any third party without the prior written
// consent of Autodesk, Inc.

#include "SunMasterCreateMode.h"
#include "sunlight.h"

#pragma managed

#define CID_RINGCREATE	CID_USER + 0x509C2DF4

SunMasterCreateMode* SunMasterCreateMode::sInstance = nullptr;

SunMasterCreateMode* SunMasterCreateMode::GetInstance()
{
	if(nullptr == sInstance)
	{
		sInstance = new SunMasterCreateMode();
	}
	
	return sInstance;
}

void SunMasterCreateMode::Destroy()
{
	delete sInstance;
	sInstance = nullptr;
}

SunMasterCreateMode::SunMasterCreateMode() :
	ip(nullptr)
{ }

SunMasterCreateMode::~SunMasterCreateMode()
{ 
	ip = nullptr;
}

// These two methods just call the creation proc method of the same
// name. 
// This creates a new sun master object and starts the editing
// of the objects parameters.  This is called just before the 
// command mode is pushed on the stack to begin the creation
// process.
void SunMasterCreateMode::Begin( IObjCreate *ioc, ClassDesc *desc, bool daylight ) 
{ 
	ip=ioc;
	proc.Begin( ioc, desc, daylight ); 
}

// This terminates the editing of the sun masters parameters.
// This is called just before the command mode is removed from
// the command stack.
void SunMasterCreateMode::End() 
{ 
	proc.End(); 
}

// This returns the type of command mode this is.  See the online
// help under this method for a list of the available choices.
// In this case we are a creation command mode.
int SunMasterCreateMode::Class() 
{ 
	return CREATE_COMMAND; 
}

// Returns the ID of the command mode. This value should be the 
// constant CID_USER plus some random value chosen by the developer.
int SunMasterCreateMode::ID() 
{ 
	return CID_RINGCREATE; 
}

// This method returns a pointer to the mouse proc that will
// handle the user/mouse interaction.  It also establishes the number 
// of points that may be accepted by the mouse proc.  In this case
// we set the number of points to 100000.  The user process will 
// terminate before this (!) after the mouse proc returns FALSE.
// The mouse proc returned from this method is an instance of
// SunMasterCreationManager.  Note that that class is derived
// from MouseCallBack.
MouseCallBack *SunMasterCreateMode::MouseProc(int *numPoints) 
{ 
	*numPoints = 100000; return &proc; 
}

// This method is called to flag nodes in the foreground plane.
// We just return the standard CHANGE_FG_SELECTED value to indicate
// that selected nodes will go into the foreground.  This allows
// the system to speed up screen redraws.  See the Advanced Topics
// section on Foreground / Background planes for more details.
ChangeForegroundCallback *SunMasterCreateMode::ChangeFGProc() 
{ 
	return CHANGE_FG_SELECTED; 
}

// This method returns TRUE if the command mode needs to change the
// foreground proc (using ChangeFGProc()) and FALSE if it does not. 
BOOL SunMasterCreateMode::ChangeFG( CommandMode *oldMode ) 
{ 
	return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED); 
}

// This method is called when a command mode becomes active.  We
// don't need to do anything at this time so our implementation is NULL
void SunMasterCreateMode::EnterMode() 
{
	SetCursor(LoadCursor(MaxSDK::GetHInstance(), MAKEINTRESOURCE(IDC_CROSS_HAIR)));
	ip->PushPrompt( const_cast<TCHAR*>(MaxSDK::GetResourceString(IDS_SUN_CREATE_PROMPT)));
}

// This method is called when the command mode is replaced by 
// another mode.  Again, we don't need to do anything.
void SunMasterCreateMode::ExitMode() 
{
	ip->PopPrompt();
	SetCursor(LoadCursor(NULL, IDC_ARROW));
}

BOOL SunMasterCreateMode::IsSticky() 
{ 
	return FALSE; 
}

#if defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
bool SunMasterCreateMode::IsActive() const 
{
	return proc.IsActive(); 
}
#endif	// defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)

