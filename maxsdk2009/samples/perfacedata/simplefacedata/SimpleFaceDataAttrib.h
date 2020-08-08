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
	FILE: SimpleFaceDataAttrib.h

	DESCRIPTION:	Header for Material ID Channel CA
					- Class ID
					- Class Decl - inherits Mixin FP
					- Class Desc
					- Post load callback

	AUTHOR: ktong - created 02.11.2006
***************************************************************************/


#ifndef	_SimplFaceDataAttrib_H_
#define _SimplFaceDataAttrib_H_

#include "ISimpleFPChannel.h"
#include "CustAttrib.h"
#include "ISimpleFaceDataChannel.h"

#include <memory> // for auto-pointers

// ****************************************************************************
// SimpleFaceData Custom Attribute Class ID
// ****************************************************************************
#define SimpleFaceData_ClassID Class_ID(0x1eb17c86, 0x6d283115)

// ****************************************************************************
// SimpleFaceData Custom Attribute
//! \brief SimpleFaceDataAttrib - CA that manages the SimpleFaceData channel. 
/*! Class inherits from the CustAttrib class. This is an object level custom
attribute. This CA maintains a SimpleFaceChannel channel on an object.
This CA is  managed with the SimpleFaceManagerImp CORE interface.
*/
//! \see SimpleFaceManagerImp
//! \see SimpleFPChannel
// ****************************************************************************
class SimpleFaceDataAttrib : public CustAttrib, public ISimpleFaceDataChannel
{ 
	Class_ID mChannelID; // ID of the associated SimpleFaceData channel

	int mChannelType; // channel type
public:
	SimpleFaceDataAttrib();

	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
							   PartID& partID,  RefMessage message){return REF_SUCCEED;}

	int	NumParamBlocks() { return 0; }					// return number of ParamBlocks in this instance
	IParamBlock2* GetParamBlock(int i) { return NULL; } // return i'th ParamBlock
	IParamBlock2* GetParamBlockByID(BlockID id) { return NULL; } // return id'd ParamBlock

	int NumRefs() { return 1;}
	RefTargetHandle GetReference(int i) { return NULL; }
	void SetReference(int i, RefTargetHandle rtarg) {}

	int NumSubs()  { return 0; }
	Animatable* SubAnim(int i) { return NULL; }
	TSTR SubAnimName(int i){ return TSTR("");} 

	void BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next);
	SClass_ID		SuperClassID() {return CUST_ATTRIB_CLASS_ID;}
	Class_ID 		ClassID() {return SimpleFaceData_ClassID;}

	ReferenceTarget *Clone(RemapDir &remap = DefaultRemapDir());
	bool CheckCopyAttribTo(ICustAttribContainer *to) { return true; }
	
	TCHAR* GetName();
	void		DeleteThis() { delete this;}

	void		GetClassName(TSTR &s){s = GetString(IDS_SIMPLEFACEDATAATTRIB_NAME);}

	// save/load
	IOResult Save(ISave* isave);
	IOResult Load(ILoad* iload);

	// Mixin interface
	BaseInterface*		GetInterface(Interface_ID id);
	FPInterfaceDesc*	GetDesc();

protected:
	//! \brief _GetFaceChannel - retrieve the channel represented by this CA.
	/*! This function actually performs a dependents enum to find the object that owns this CA,
	acquires it's FaceDataMgr, then asks the FaceDataMgr to enum its face data channels to find
	our SimpleFaceChannel. The alternative is to house the SimpleFaceChannel in this CA,
	but we run into issues of the CA and the FaceDataMgr both trying to manage the same data
	during operations (e.g. node cloning, where the CA is cloned by its CA container, and the 
	face data channels are cloned by the FaceDataMgr).
	\return A pointer to the SimpleFaceChannel. NULL on failure.
	*/
	IFaceDataChannel* _GetFaceChannel();

public:  // "internal" helper functions
	void SetChannelID(const Class_ID &channelID);	// \_the manager needs access to associate a channel with this CA
	void SetChannelType(int type);					// /

	/*! \brief Creates a temporary wrapper around a per-face data channel. The wrapper cannot be permanent
	because an IFaceDataChannel is transient, so it's pointer cannot be relied upon beyond the immediate
	scope in which it is obtained. Auto_ptrs transfer and free the allocated data they wrap.
	*/
	static std::auto_ptr<ISimpleFPChannel> WrapFaceChannel(IFaceDataChannel* pChan, int type);
	//! \brief Factory for allocating a new per-face data channel.
	static IFaceDataChannel* CreateFaceChannel(int type, ULONG size);

public: 	// ISimpleFaceDataChannel external interface
	//! \brief SetChannelName - Set the name of the channel.
	void SetChannelName(const TCHAR* pName);

	//! \brief GetChannelName - Get the name of the channel.
	TCHAR* GetChannelName();

	//! \brief ChannelID - Retrieve the ID of the associated channel as a 2-array of ULONG/DWORD (for maxscript)
	Class_ID	GetChannelID();
	Tab<DWORD> ChannelID();

	//! \brief ChannelType - get the data type of the channel.
	int	ChannelType();

	//! \brief NumFaces - return the number of faces in a specific channel. 0 on failure.
	ULONG	NumFaces();

	//! \brief GetValue - Get the value of the specified face from the specified channel.
	FPValue GetValue(ULONG face);

	//! \brief SetValue - Set the value of the specified face from the specified channel.
	BOOL SetValue(ULONG face, Value* pVal);

	//! \brief GetValues - Returns a copy of the channel list. Empty list on failure.
	FPValue GetValues();

	//! \brief SetValues - Set the channel with a list of values. The new list and the channel must match in size.
	BOOL SetValues(Value* pVal);

	//! \brief GetValueBySelection - Returns the value of the selected faces, or NULL if they don't share a common value.
	FPValue GetValueBySelection(const BitArray* pFaces);

	//! \brief SetValues - Set the selected faces of a channel with a value.
	BOOL SetValueBySelection(const BitArray* pFaces, Value* pVal);
};

// ****************************************************************************
// SimpleFaceDataAttrib Class Descriptor
//! \brief SimpleFaceDataAttribClassDesc - Class descriptor. Report in DLLEntry.cpp
//! \see LibNumberClasses
//! \see LibClassDesc
// ****************************************************************************
class SimpleFaceDataAttribClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new SimpleFaceDataAttrib; }
	const TCHAR *	ClassName() { return GetString(IDS_SIMPLEFACEDATAATTRIB_NAME); }
	SClass_ID		SuperClassID() { return CUST_ATTRIB_CLASS_ID; }
	Class_ID		ClassID() { return SimpleFaceData_ClassID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("SimpleFaceData"); }		// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
};

// ****************************************************************************
//! \brief SimpleFaceDataAttribPostLoadCB - Post-load callback to attach loaded 
//! channel data to parent object.
/*! This class inherits from PostLoadCallback and is used by the load function
of SimpleFaceDataAttrib to register a post-load event. This is needed because
references do not exist at load time, so the CA must wait until post-load (when
references have been created) to add the loaded channel data to the parent. An
instance of this class must be dynamically allocated in order to persist after
the registration call, and thus self-deleted when the instance is invoked.
*/
// ****************************************************************************
class SimpleFaceDataAttribPostLoadCB : public PostLoadCallback
{
	//! \mpTarget - Assign the target SimpleFaceDataAttrib CA.
	/*! Assign a pointer to the SimpleFaceDataAttrib instance that invoked this
	callback. This instance grants access to the owning mesh object for loading
	and attaching per-face data.
	*/
	SimpleFaceDataAttrib* mpTarget;
	//! \brief SetLoadSource - Assign the source SimpleFaceChannel to load.
	/*! Assign a pointer to an allocated channel so that it can persist past
	the callback registration. mpSource is deleted when this callback ends,
	regardless if it is copied by the intended destination.
	*/
	IFaceDataChannel* mpSource;
public:
	SimpleFaceDataAttribPostLoadCB(SimpleFaceDataAttrib* pCA, IFaceDataChannel* pSource);
	~SimpleFaceDataAttribPostLoadCB();

	void proc(ILoad* iload);
};

#endif
//EOF
