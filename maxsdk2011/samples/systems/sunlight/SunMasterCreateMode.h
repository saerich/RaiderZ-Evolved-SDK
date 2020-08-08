// Copyright 2009 Autodesk, Inc.  All rights reserved.
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law.  They may not
// be disclosed to, copied or used by any third party without the prior written
// consent of Autodesk, Inc.

#pragma once
#include "NativeInclude.h"
#include "SunMasterCreationManager.h"

#pragma managed(push, on)

// This is the command mode that manages the overall process when 
// the system is created.  
// See the Advanced Topics section on Command Modes and Mouse Procs for 
// more details on these methods.
class SunMasterCreateMode : public CommandMode, public MaxSDK::Util::Noncopyable
{
public:
	static SunMasterCreateMode* GetInstance();
	static void Destroy();
	
	// These two methods just call the creation proc method of the same
	// name. 
	// This creates a new sun master object and starts the editing
	// of the objects parameters.  This is called just before the 
	// command mode is pushed on the stack to begin the creation
	// process.
	void Begin( IObjCreate *ioc, ClassDesc *desc, bool daylight );
	
	// This terminates the editing of the sun masters parameters.
	// This is called just before the command mode is removed from
	// the command stack.
	void End();
	
	// This returns the type of command mode this is.  See the online
	// help under this method for a list of the available choices.
	// In this case we are a creation command mode.
	int Class();
	
	// Returns the ID of the command mode. This value should be the 
	// constant CID_USER plus some random value chosen by the developer.
	int ID();
	
	// This method returns a pointer to the mouse proc that will
	// handle the user/mouse interaction.  It also establishes the number 
	// of points that may be accepted by the mouse proc.  In this case
	// we set the number of points to 100000.  The user process will 
	// terminate before this (!) after the mouse proc returns FALSE.
	// The mouse proc returned from this method is an instance of
	// SunMasterCreationManager.  Note that that class is derived
	// from MouseCallBack.
	MouseCallBack *MouseProc(int *numPoints);
	
	// This method is called to flag nodes in the foreground plane.
	// We just return the standard CHANGE_FG_SELECTED value to indicate
	// that selected nodes will go into the foreground.  This allows
	// the system to speed up screen redraws.  See the Advanced Topics
	// section on Foreground / Background planes for more details.
	ChangeForegroundCallback *ChangeFGProc();
	
	// This method returns TRUE if the command mode needs to change the
	// foreground proc (using ChangeFGProc()) and FALSE if it does not. 
	BOOL ChangeFG( CommandMode *oldMode );
	
	// This method is called when a command mode becomes active.  We
	// don't need to do anything at this time so our implementation is NULL
	void EnterMode();
	
	// This method is called when the command mode is replaced by 
	// another mode.  Again, we don't need to do anything.
	void ExitMode();
	
	BOOL IsSticky();
	
#if defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
	bool IsActive() const;
#endif	// defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)

private:
	SunMasterCreateMode();
	virtual ~SunMasterCreateMode();
	
	// This instance of SunMasterCreationMangaer handles the user/mouse
	// interaction as the sun array is created.
	IObjCreate *ip;
	SunMasterCreationManager proc;

	static SunMasterCreateMode* sInstance;
};


#pragma managed(pop)
