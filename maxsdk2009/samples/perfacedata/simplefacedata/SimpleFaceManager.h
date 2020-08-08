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
	FILE: SimpleFaceManager.h

	DESCRIPTION: Header for an implementation of ISimpleFaceDataManager
					- class declaration
					- function map

	AUTHOR: ktong - created 02.15.2006
***************************************************************************/


#include "SimpleFaceData.h"
#include "ISimpleFaceDataManager.h"

#include <randgenerator.h> // to generate new classIDs

class SimpleFaceManagerImp : public ISimpleFaceDataManager
{
	DECLARE_DESCRIPTOR(SimpleFaceManagerImp)

	BEGIN_FUNCTION_MAP
		FN_4(eFpAddChannel, TYPE_INTERFACE, AddChannel, TYPE_OBJECT, TYPE_ENUM, TYPE_DWORD_TAB, TYPE_STRING)
		VFN_2(eFpRemoveChannel, RemoveChannel, TYPE_OBJECT, TYPE_DWORD_TAB)
		FN_2(eFpGetChannel, TYPE_INTERFACE, GetChannel, TYPE_OBJECT, TYPE_DWORD_TAB)
		FN_1(eFpGetChannels, TYPE_INTERFACE_TAB_BV, GetChannels, TYPE_OBJECT)
	END_FUNCTION_MAP

	//! \brief Adds the SimpleFaceData custom attribute to an editable mesh or poly object.
    ISimpleFaceDataChannel* AddChannel(Object* pObj, int type, const Tab<DWORD>* pChannelID, const TCHAR* pChannelName) const;
	ISimpleFaceDataChannel* AddChannel(Object* pObj, int type, const Class_ID* pChannelID, const TCHAR* pChannelName) const;


	//! \brief RemoveChannel - Removes the SimpleFaceData custom attribute
	//! from an editable mesh or poly object.
    void RemoveChannel(Object* pObj, const Tab<DWORD>* pChannelID);
	void RemoveChannel(Object* pObj, const Class_ID &channelID);

	//! \brief GetChannel - Retrieve a channel with the speicifed channel ID
    ISimpleFaceDataChannel* GetChannel(Object* pObj, const Tab<DWORD>* pChannelID) const;
	ISimpleFaceDataChannel* GetChannel(Object* pObj, const Class_ID &channelID) const;

    Tab<ISimpleFaceDataChannel*> GetChannels(Object* pObj) const;
};