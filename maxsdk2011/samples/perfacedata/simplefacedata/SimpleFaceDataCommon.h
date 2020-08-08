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
	FILE: SimpleFaceDataCommon.h

	DESCRIPTION:	Utility functions used by SimpleFaceDataAttrib

	CREATED BY: Kuo-Cheng Tong

	AUTHOR: ktong - created 01.30.2006
***************************************************************************/

#ifndef _SIMPLEFACEDATACOMMON_H_
#define _SIMPLEFACEDATACOMMON_H_

#include "SimpleFaceData.h"
#include "CustAttrib.h"
#include "IFaceDataMgr.h"

// ****************************************************************************
// Defines
// ****************************************************************************
#define IDS_UNDEFINED -1

// ****************************************************************************
// SimpleFaceData parent object finder callback
//! \brief FindParentObjFromAttribCB - Dependendent enumerator used
//! to find the parent object of a custom attribute.
// ****************************************************************************
class FindParentObjFromAttribCB : public DependentEnumProc
{
protected:
	Animatable* mpParent;
	Animatable* mpSelf;
public:
	FindParentObjFromAttribCB(Animatable* pSelf);
	~FindParentObjFromAttribCB();
	int proc(ReferenceMaker* rmaker);
	Animatable* GetParent();
};

// ****************************************************************************
//! \brief SimpleFaceDataCommon - Contains globals and functions used by
//! SimpleFaceData custom attributes and classes.
// ****************************************************************************
class SimpleFaceDataCommon
{
public:
	//! \brief FindParentObjFromAttrib - Retrieve the object with this CA applied to it.
	static Animatable*		FindParentObjFromAttrib(CustAttrib* pCA);
	//! \brief FindFaceDataFromObj - Retrieve the face data manager from this object.
	static IFaceDataMgr*	FindFaceDataFromObj(Animatable* pObj);
	//! \brief FindAttribFromParent - Get a specific custom attribute from this anim's custattrib container.
	static CustAttrib*		FindAttribFromParent(Animatable* pAnim, const Class_ID& cid);
	//! \brief FindBaseFromObject - find the base object pointed at by this object.
	static Object*			FindBaseFromObject(Object* pObj);
	//! \brief GetNumFacesFromObject - return the number of faces the object has.
	static ULONG			GetNumFacesFromObject(Animatable* pAnim);
	//! \brief FindFaceDataChanFromObj - return a face data channel from an object.
	static IFaceDataChannel* FindFaceDataChanFromObj(Animatable* pObj, const Class_ID& cid);
};

#endif

//EOF