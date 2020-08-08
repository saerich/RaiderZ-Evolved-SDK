//*****************************************************************************/
// File:		springs.h
//
// Description:	header for the spring system global settings
//
// Created:		27-may-2005
//
// Copyright (c) 1998-2005 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//*****************************************************************************/

#ifndef __SPRINGS_H_
#define __SPRINGS_H_


///////////////////////////////////////////////////////////////////////////////
//
//				class SpringSysGlobalSettings
//
//	Variables and methods related to the global spring controller settings.
// These really belong to the SpringSys class, but being part of the SDK, that 
// class cannot be changed in this version (v8).
//
//	Existing behaviour of SpringSys inside spring controllers was to keep 
// results accurate at all time.  As a result, if anything was invalidating
// a spring controller, it ended up always resimulating from its initial frame.
//
//	The class holds a boolean indicating whether we are currently using the  
// accurate method (simulate from frame start) or a new, quicker method (only 
// simulate from a certain number of frames back).  Springs will only be using
// the new, quicker method if the user has turned it on by checking Springs 
// Quick Edit in the animation preferences page of the user preferences.  Even
// then it will only use the quicker method temporarily, automatically switching    
// from the usual accurate mode to the quick mode between MouseCycleStarted  
// and MouseCycleCompleted calls.
//
///////////////////////////////////////////////////////////////////////////////
class SpringSysGlobalSettings
{
public:
	SpringSysExport static bool IsAlwaysAccurate() {
		return s_alwaysAccurate; }
	SpringSysExport static void SetAlwaysAccurate(bool in_alwaysAccurate = true) {
		s_alwaysAccurate = in_alwaysAccurate; }

private:
	SpringSysExport static bool	s_alwaysAccurate;
};

#endif	//	__SPRINGS_H_
