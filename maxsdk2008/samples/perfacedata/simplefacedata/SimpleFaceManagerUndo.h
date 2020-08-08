//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
/**********************************************************************
 	FILE: SimpleFaceManagerUndo.h

	DESCRIPTION: Header for restore objects for SimpleFaceManager.
					- add channel
					- remove channel

	AUTHOR: ktong - created 02.16.2006
/***************************************************************************/

#include "SimpleFaceData.h"
#include "SimpleFaceDataCommon.h"

// ****************************************************************************
// Add channel undo object
// Can't gain access to store THE channel pointer on undo, so have to make 
// a new one during redo.
// Used by SimpleFaceDataAttrib
// ****************************************************************************
class AddChannelUndo : public RestoreObj
{
protected:
	Animatable* mpObj;			// the channel host object
	IFaceDataChannel* mpCon;	// a copy of the data channel to undo
	Class_ID mChannelID;		// the id of the channel to undo
public:
	AddChannelUndo(Animatable* pObj, const Class_ID &channelID);
	~AddChannelUndo();
	void	Restore(int isUndo);
	void	Redo();
	int		Size();
	void	EndHold();
	TSTR	Description();
	INT_PTR	Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3);
};

// ****************************************************************************
// Remove channel undo object
// Can't gain access to store THE channel pointer on creation or redo, so 
// have to make a new one on undo.
// Used by SimpleFaceDataAttrib
// ****************************************************************************
class RemoveChannelUndo : public RestoreObj
{
protected:
	Animatable* mpObj;			// the channel host object
	IFaceDataChannel* mpCon;	// a copy of the data channel to undo
	Class_ID mChannelID;		// the id of the channel to undo
public:
	RemoveChannelUndo(Animatable* pObj, const Class_ID &channelID);
	~RemoveChannelUndo();
	void	Restore(int isUndo);
	void	Redo();
	int		Size();
	void	EndHold();
	TSTR	Description();
	INT_PTR	Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3);
};