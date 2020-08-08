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
	FILE: SimpleFaceDataUndo.h

	DESCRIPTION:	Header file for restoreObj's for SimpleFaceDataAttrib.
					Class decl's for:
					- name undo
					- single value undo
					- value list undo

	AUTHOR: ktong - created 02.11.2006
***************************************************************************/

#ifndef _SIMPLEFACEDATAUNDO_H_
#define _SIMPLEFACEDATAUNDO_H_

#include "SimpleFaceDataAttrib.h"

// ****************************************************************************
// channel name undo object
// ****************************************************************************
class NameUndo : public RestoreObj
{
protected:
	TSTR mOldName;
	TSTR mNewName;
	Animatable* mpObj;
	Class_ID mChannelID;
	int mType;
public:
	NameUndo(Animatable* pObj, const Class_ID &channelID, int type);
	~NameUndo();
	void	Restore(int isUndo);
	void	Redo();
	int		Size();
	void	EndHold();
	TSTR	Description();
	INT_PTR	Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3);
};

// ****************************************************************************
// channel values undo object
// ****************************************************************************
class ValueListUndo : public RestoreObj
{
protected:
	Class_ID mChannelID;
	FPValue mOldValue;
	FPValue mNewValue;
	Animatable* mpObj;
	int mType;
public:
	ValueListUndo(Animatable* pObj, const Class_ID &channelID, int type);
	~ValueListUndo();
	void	Restore(int isUndo);
	void	Redo();
	int		Size();
	void	EndHold();
	TSTR	Description();
	INT_PTR	Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3);
};

// ****************************************************************************
// channel value undo object
// ****************************************************************************
class ValueUndo : public RestoreObj
{
protected:
	Class_ID mChannelID;
	FPValue mOldValue;
	FPValue mNewValue;
	ULONG mAt;
	Animatable* mpObj;
	int mType;
public:
	ValueUndo(Animatable* pObj, const Class_ID &channelID, ULONG at, int type);
	~ValueUndo();
	void	Restore(int isUndo);
	void	Redo();
	int		Size();
	void	EndHold();
	TSTR	Description();
	INT_PTR	Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3);
};

#endif
//EOF