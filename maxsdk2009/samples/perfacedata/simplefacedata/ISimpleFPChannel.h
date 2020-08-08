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
	FILE: ISimpleFPChannel.h

	DESCRIPTION: Header for a FPValue-exposed interface for a 
					SimpleFaceChannel wrapper.

	CREATED BY: Kuo-Cheng Tong

	AUTHOR: ktong - created 01.30.2006
***************************************************************************/
#ifndef _ISIMPLEFPCHANNEL_H_
#define _ISIMPLEFPCHANNEL_H_

#include "SimpleFaceData.h"
#include "IDataChannel.h"

class ISimpleFPChannel
{
public:
	//! \brief GetChannel - Get the wrapped channel
	virtual IFaceDataChannel* GetChannel()=0;

	//! \brief GetChannelName - Get the channel name.
	virtual TCHAR* GetChannelName()=0;

	//! \brief SetChannelName - Set the channel name
	virtual void SetChannelName(const TCHAR* pName)=0;

	//! \brief GetChannelID - Get the channel ID.
	virtual Class_ID GetChannelID()=0;

	//! \brief SetChannelID - Set the channel ID.
	virtual void SetChannelID(const Class_ID &classID)=0;

	//! \brief GetType - Get the data type of the channel.
	virtual int GetChannelType()=0;

	//! \brief GetNumFaces - Number of faces in this channel
	virtual ULONG GetNumFaces()=0;

	//! \brief SetNumFaces - Set the number of faces in this channel
	virtual void SetNumFaces(ULONG numFaces)=0;

	//! \brief Compare - Compare two FPValues as types of this channel.
	virtual BOOL Compare(const FPValue &val1, const FPValue &val2)=0;

	//! \brief GetValue - Get the value of the specified face from the specified channel.
	virtual BOOL GetValue(ULONG face, FPValue &val)=0;

	//! \brief SetValue - Set the value of the specified face from the specified channel.
	virtual BOOL	SetValue(ULONG face, const FPValue* pVal)=0;

	//! \brief GetValues - Returns a copy of the channel list. Empty list on failure.
	virtual BOOL	GetValues(FPValue &val)=0;

	//! \brief SetValues - Set the channel with a list of values. The new list and the channel must match in size.
	virtual BOOL SetValues(const FPValue* pVal)=0;

	virtual IOResult	Save(ISave* isave)=0;
	virtual IOResult	Load(ILoad* iload)=0;
};

#endif