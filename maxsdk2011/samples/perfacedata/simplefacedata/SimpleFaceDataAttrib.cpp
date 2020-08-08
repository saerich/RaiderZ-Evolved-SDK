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
	FILE: SimpleFaceDataAttrib.cpp

	DESCRIPTION:	Material ID Channel CA Implementation
					- class desc instance / access
					- class definition
					- Post load callback
					- Mixin FP instance

	AUTHOR: ktong - created 02.11.2006
***************************************************************************/

#include "SimpleFaceDataCommon.h"
#include "SimpleFaceDataAttrib.h"
#include "SimpleFaceDataUndo.h"
#include "SimpleFPChannel.h"
#include "PointerFPChannel.h"
#include "SimpleFaceDataValidator.h"
#include <maxscrpt\maxscrpt.h>
#include <maxscrpt\funcs.h>

//! \brief CHUNKID_SIMPLE_ATTR - ID's for saving/loading SimpleFaceDataAttrib.
//! ADD TO THE END TO STAY COMPATIBLE WITH ALREADY SAVED FILES.
enum CHUNKID_SIMPLE_ATTR {
	CHUNKID_SIMPLE_ATTR_CHANNEL,
	CHUNKID_SIMPLE_ATTR_CHANNELTYPE,
};

// ****************************************************************************
// local statics
// ****************************************************************************
static SimpleFaceDataAttribClassDesc theSimpleFaceDataAttribCD;
ClassDesc2* GetSimpleFaceDataAttribDesc() {return &theSimpleFaceDataAttribCD;}

// fp parameter validators
static ParamIndexValidator theIndexValidator;
static ParamTypeValidator theTypeValidator;
static ParamArrayValidator theArrayValidator;
static ParamSelectValidator theSelectValidator;

// ****************************************************************************
// SimpleFaceDataAttrib FP Interface
// ****************************************************************************
// Instance of the ISimpleFaceDataChannel class descriptor.
// Declares publish names/types of the functions.
static FPInterfaceDesc theSimpleFaceDataChannelFPDesc(
	SimpleFaceDataChannel_InterfaceID, _T("simpleFaceChannel"), IDS_UNDEFINED, &theSimpleFaceDataAttribCD, FP_MIXIN,

	ISimpleFaceDataChannel::eFpGetValue, _T("getValue"), IDS_UNDEFINED, TYPE_FPVALUE_BV, 0, 1,
		_T("face"), IDS_UNDEFINED, TYPE_INDEX, f_validator, &theIndexValidator,

	ISimpleFaceDataChannel::eFpSetValue, _T("setValue"), IDS_UNDEFINED, TYPE_BOOL, 0, 2,
		_T("face"), IDS_UNDEFINED, TYPE_INDEX, f_validator, &theIndexValidator,
		_T("value"), IDS_UNDEFINED, TYPE_VALUE, f_validator, &theTypeValidator,

	ISimpleFaceDataChannel::eFpGetValues, _T("getValues"), IDS_UNDEFINED, TYPE_FPVALUE_BV, 0, 0,

	ISimpleFaceDataChannel::eFpSetValues, _T("setValues"), IDS_UNDEFINED, TYPE_BOOL, 0, 1,
		_T("values"), IDS_UNDEFINED, TYPE_VALUE, f_validator, &theArrayValidator,

	ISimpleFaceDataChannel::eFpGetValueBySelection, _T("getValueBySelection"), IDS_UNDEFINED, TYPE_FPVALUE_BV, 0, 1,
		_T("faces"), IDS_UNDEFINED, TYPE_BITARRAY, f_validator, &theSelectValidator,

	ISimpleFaceDataChannel::eFpSetValueBySelection, _T("setValueBySelection"), IDS_UNDEFINED, TYPE_BOOL, 0, 2,
		_T("faces"), IDS_UNDEFINED, TYPE_BITARRAY, f_validator, &theSelectValidator,
		_T("value"), IDS_UNDEFINED, TYPE_VALUE, f_validator, &theTypeValidator,

	properties,

	ISimpleFaceDataChannel::eFpGetChannelName, ISimpleFaceDataChannel::eFpSetChannelName, _T("name"), IDS_UNDEFINED, TYPE_STRING,
	ISimpleFaceDataChannel::eFpGetChannelID, FP_NO_FUNCTION, _T("id"), IDS_UNDEFINED, TYPE_DWORD_TAB_BV,
	ISimpleFaceDataChannel::eFpGetChannelType, FP_NO_FUNCTION, _T("type"), IDS_UNDEFINED, TYPE_ENUM, ISimpleFaceDataChannel::eFpChannelTypeEnum,
	ISimpleFaceDataChannel::eFpGetNumFaces, FP_NO_FUNCTION, _T("numFaces"), IDS_UNDEFINED, TYPE_DWORD,

	enums,
	ISimpleFaceDataChannel::eFpChannelTypeEnum, 6,
				_T("integer"),		TYPE_INT,
				_T("index"),		TYPE_INDEX,
				_T("float"),		TYPE_FLOAT,
				_T("boolean"),		TYPE_BOOL,
				_T("point2"),		TYPE_POINT2_BV,
				_T("point3"),		TYPE_POINT3_BV,

	end
);

// ****************************************************************************
// Material ID Channel Custom Attribute
// ****************************************************************************
//-----------------------------------------------------------------------------
SimpleFaceDataAttrib::SimpleFaceDataAttrib()
//-----------------------------------------------------------------------------
{
	mChannelID = SimpleFaceData_ClassID;
}

//-----------------------------------------------------------------------------
ReferenceTarget* SimpleFaceDataAttrib::Clone(RemapDir &remap)
//-----------------------------------------------------------------------------
{
	SimpleFaceDataAttrib *pnew = new SimpleFaceDataAttrib;
	pnew->SetChannelID(mChannelID);
	pnew->SetChannelType(mChannelType);
	BaseClone(this, pnew, remap);
	return pnew;
}

//-----------------------------------------------------------------------------
const TCHAR* SimpleFaceDataAttrib::GetName()
// custom attribute name
//-----------------------------------------------------------------------------
{
	return GetChannelName();
}

//-----------------------------------------------------------------------------
IOResult SimpleFaceDataAttrib::Save(ISave* isave)
//-----------------------------------------------------------------------------
{
	IOResult res = IO_OK;
	ULONG nb;

	std::auto_ptr<ISimpleFPChannel> pContainer = WrapFaceChannel(_GetFaceChannel(), mChannelType);

	// save the channel data
	if ( pContainer->GetChannel() ) {
		isave->BeginChunk(CHUNKID_SIMPLE_ATTR_CHANNELTYPE);
		res = isave->Write(&mChannelType, sizeof(mChannelType), &nb);
		isave->EndChunk();
		isave->BeginChunk(CHUNKID_SIMPLE_ATTR_CHANNEL);
		res = (pContainer)->Save(isave);
		isave->EndChunk();
	}
	return res;
}

//-----------------------------------------------------------------------------
IOResult SimpleFaceDataAttrib::Load(ILoad* iload)
// load the channel
//-----------------------------------------------------------------------------
{
	IOResult res;
	IOResult error = IO_OK;
	ULONG nb;

	// Allocate a new channel. 
	// This will be registered in a PLCB to be used or deleted.
	// We have to bind this to the object later because there is no guarantee the parent exists now.
	std::auto_ptr<ISimpleFPChannel> pWrap = WrapFaceChannel(NULL, TYPE_UNSPECIFIED);
	while (IO_OK==(res=iload->OpenChunk())) 
	{ 
		switch(iload->CurChunkID()) 
		{
		case CHUNKID_SIMPLE_ATTR_CHANNELTYPE:
			res = iload->Read(&mChannelType, sizeof(mChannelType), &nb);
		break;
		case CHUNKID_SIMPLE_ATTR_CHANNEL:
		{
			if (mChannelType != TYPE_UNSPECIFIED) {
				pWrap = WrapFaceChannel(
					CreateFaceChannel(mChannelType, 0),
				mChannelType);
				res = pWrap->Load(iload);
			} else {
				res = IO_ERROR;
			}
		}
		break;
		}
		iload->CloseChunk(); 
		if (res != IO_OK) {
			error = res;
			break;
		}
	}

	// register a post load event if the container load was successful
	if (error == IO_OK) {
		if (pWrap->GetChannel()) {
			SetChannelID(pWrap->GetChannelID());
			SimpleFaceDataAttribPostLoadCB *pLoader = new SimpleFaceDataAttribPostLoadCB(this, pWrap->GetChannel());
			iload->RegisterPostLoadCallback(pLoader);
		}
		res = IO_OK;
	} else { // clear allocated channel, was never used.
		if (pWrap->GetChannel()) {
			pWrap->GetChannel()->DeleteThis();
		}
		res = error;
	}
	return res;
}

// ****************************************************************************
// SimpleFaceDataAttribPostLoadCB post load callback
// ****************************************************************************
//-----------------------------------------------------------------------------
SimpleFaceDataAttribPostLoadCB::SimpleFaceDataAttribPostLoadCB(SimpleFaceDataAttrib* pCA, IFaceDataChannel* pSource)
//-----------------------------------------------------------------------------
{
	mpSource = pSource;
	mpTarget = pCA;
}
//-----------------------------------------------------------------------------
SimpleFaceDataAttribPostLoadCB::~SimpleFaceDataAttribPostLoadCB()
//-----------------------------------------------------------------------------
{
	if (mpSource) {
		delete mpSource; // haven't used this yet, never will, delete it
	}
}
//-----------------------------------------------------------------------------
void SimpleFaceDataAttribPostLoadCB::proc(ILoad* /*iload*/)
//-----------------------------------------------------------------------------
{
	Animatable* pObj = SimpleFaceDataCommon::FindParentObjFromAttrib(mpTarget);
	IFaceDataMgr* pFDMgr = SimpleFaceDataCommon::FindFaceDataFromObj(pObj);
	ULONG numFaces = SimpleFaceDataCommon::GetNumFacesFromObject(pObj);
	ULONG curFaces = 0;

	if (pFDMgr && mpSource) {
		// validate the data, in case it's been corrupted
		curFaces = mpSource->Count();
		if (curFaces < numFaces) { // need to size up. Add faces (fill with the default value).
			mpSource->FacesCreated(curFaces, numFaces - curFaces);
		} else if (curFaces > numFaces) { // size down. Just Truncate.
			mpSource->FacesDeleted(numFaces, curFaces - numFaces);
		}
		pFDMgr->AddFaceDataChan(mpSource); // add the channel to the manager
		mpSource = NULL; // channel has been used, transfer ownership
	}
	delete this; // this instance was allocated in order to persist past the registration call. delete self.
}

// ****************************************************************************
// From FP Interface
// ****************************************************************************

//-----------------------------------------------------------------------------
BaseInterface* SimpleFaceDataAttrib::GetInterface(Interface_ID id)
//-----------------------------------------------------------------------------
{
	if (id == SimpleFaceDataChannel_InterfaceID) {
        return static_cast<ISimpleFaceDataChannel*>(this);
	} else {
        return CustAttrib::GetInterface(id);
	}
}

//-----------------------------------------------------------------------------
FPInterfaceDesc* SimpleFaceDataAttrib::GetDesc()
//-----------------------------------------------------------------------------
{
	return &theSimpleFaceDataChannelFPDesc;
}

// ****************************************************************************
// Internal accessors
// ****************************************************************************
//-----------------------------------------------------------------------------
IFaceDataChannel* SimpleFaceDataAttrib::_GetFaceChannel()
//-----------------------------------------------------------------------------
{
	Animatable* pObj = SimpleFaceDataCommon::FindParentObjFromAttrib(this);
	IFaceDataMgr* pFDMgr = SimpleFaceDataCommon::FindFaceDataFromObj(pObj);
	if (pFDMgr) {
		return pFDMgr->GetFaceDataChan(mChannelID);
	}
	return NULL;
}

//-----------------------------------------------------------------------------
std::auto_ptr<ISimpleFPChannel> SimpleFaceDataAttrib::WrapFaceChannel(IFaceDataChannel* pChan, int type)
//-----------------------------------------------------------------------------
{
	switch (type)
	{
	case TYPE_BOOL:
		return std::auto_ptr<ISimpleFPChannel>(new SimpleFPChannel<TYPE_BOOL_TYPE>(pChan));
	break;
	case TYPE_INDEX:
	case TYPE_INT:
		return std::auto_ptr<ISimpleFPChannel>(new SimpleFPChannel<TYPE_INT_TYPE>(pChan));
	break;
	case TYPE_FLOAT:
		return std::auto_ptr<ISimpleFPChannel>(new SimpleFPChannel<TYPE_FLOAT_TYPE>(pChan));
	break;
	case TYPE_POINT2_BV:
		return std::auto_ptr<ISimpleFPChannel>(new PointerFPChannel<TYPE_POINT2_BV_TYPE>(pChan));
	break;
	case TYPE_POINT3_BV:
		return std::auto_ptr<ISimpleFPChannel>(new PointerFPChannel<TYPE_POINT3_BV_TYPE>(pChan));
	break;
	}
	return std::auto_ptr<ISimpleFPChannel>(new SimpleFPChannel<TYPE_INT_TYPE>(NULL));
}

//-----------------------------------------------------------------------------
IFaceDataChannel* SimpleFaceDataAttrib::CreateFaceChannel(int type, ULONG size)
//-----------------------------------------------------------------------------
{
	IFaceDataChannel* pChan = NULL;
  	switch (type)
	{
	case TYPE_BOOL:
		pChan = new SimpleFaceChannel<TYPE_BOOL_TYPE>(type, size, FALSE);
	break;
	case TYPE_INDEX:
	case TYPE_INT:
		pChan = new SimpleFaceChannel<TYPE_INT_TYPE>(type, size, 0);
	break;
	case TYPE_FLOAT:
		pChan = new SimpleFaceChannel<TYPE_FLOAT_TYPE>(type, size, 0.0f);
	break;
	case TYPE_POINT2_BV:
		pChan = new SimpleFaceChannel<TYPE_POINT2_BV_TYPE>(type, size, Point2(0,0));
	break;
	case TYPE_POINT3_BV:
		pChan = new SimpleFaceChannel<TYPE_POINT3_BV_TYPE>(type, size, Point3(0,0,0));
	break;
	}
	return pChan;
}

//-----------------------------------------------------------------------------
void SimpleFaceDataAttrib::SetChannelType(int type)
//-----------------------------------------------------------------------------
{
	mChannelType = type;
}

// ****************************************************************************
// External interface
// ****************************************************************************
//-----------------------------------------------------------------------------
TCHAR* SimpleFaceDataAttrib::GetChannelName()
// exposed channel name property
//-----------------------------------------------------------------------------
{
	std::auto_ptr<ISimpleFPChannel>pChan = WrapFaceChannel(_GetFaceChannel(), mChannelType);
	if (pChan->GetChannel()) {
		return pChan->GetChannelName();
	}
	return GetString(IDS_SIMPLEFACEDATAATTRIB_NAME);
}

//-----------------------------------------------------------------------------
void SimpleFaceDataAttrib::SetChannelName(const TCHAR* pName)
// exposed channel name property
//-----------------------------------------------------------------------------
{
	std::auto_ptr<ISimpleFPChannel>pChan = WrapFaceChannel(_GetFaceChannel(), mChannelType);
	if (pChan->GetChannel() && pName && (_tcslen(pName) > 0)) {
		if (theHold.Holding()) {
			theHold.Put(new NameUndo(SimpleFaceDataCommon::FindParentObjFromAttrib(this), 
				pChan->GetChannelID(), 
				mChannelType));
		}
		pChan->SetChannelName(pName);
	}
}

//-----------------------------------------------------------------------------
Class_ID SimpleFaceDataAttrib::GetChannelID()
//-----------------------------------------------------------------------------
{
	return mChannelID;
}

//-----------------------------------------------------------------------------
void SimpleFaceDataAttrib::SetChannelID(const Class_ID &channelID)
//-----------------------------------------------------------------------------
{
	mChannelID = channelID;
}

//-----------------------------------------------------------------------------
ULONG SimpleFaceDataAttrib::NumFaces()
//-----------------------------------------------------------------------------
{
	std::auto_ptr<ISimpleFPChannel>pChan = WrapFaceChannel(_GetFaceChannel(), mChannelType);
	ULONG num = 0;
	if (pChan->GetChannel()) {
		num = pChan->GetNumFaces();
	}
	return num;
}

//-----------------------------------------------------------------------------
int SimpleFaceDataAttrib::ChannelType()
//-----------------------------------------------------------------------------
{
	std::auto_ptr<ISimpleFPChannel>pChan = WrapFaceChannel(_GetFaceChannel(), mChannelType);
	if (pChan->GetChannel()) {
		return pChan->GetChannelType();
	}
	return TYPE_UNSPECIFIED;
}

//-----------------------------------------------------------------------------
FPValue SimpleFaceDataAttrib::GetValue(ULONG face)
//-----------------------------------------------------------------------------
{
	std::auto_ptr<ISimpleFPChannel>pChan = WrapFaceChannel(_GetFaceChannel(), mChannelType);
	FPValue val(TYPE_VALUE, NULL);
	if (pChan->GetChannel()) {
		pChan->GetValue(face, val);
	}
	return val;
}

//-----------------------------------------------------------------------------
BOOL SimpleFaceDataAttrib::SetValue(ULONG face, Value* pVal)
//-----------------------------------------------------------------------------
{
	std::auto_ptr<ISimpleFPChannel>pChan = WrapFaceChannel(_GetFaceChannel(), mChannelType);
	BOOL res = FALSE;
	if (pChan->GetChannel()) {
		if (theHold.Holding()) {
			theHold.Put(new ValueUndo(SimpleFaceDataCommon::FindParentObjFromAttrib(this), 
				pChan->GetChannelID(), 
				face,
				mChannelType));
		}
		try {
			// use maxscript sdk to convert from a value to an FPValue
			// val_to_FPValue will also convert from one number type (TYPE_INT, TYPE_STRING) to another (TYPE_INDEX)
			FPValue val;
			InterfaceFunction::val_to_FPValue(pVal, (ParamType2)mChannelType, val); //throws if unable to convert
			res = pChan->SetValue(face, &val);
			InterfaceFunction::release_param(val, (ParamType2)mChannelType, pVal);
		} catch (...) {
			res = FALSE;
		}
	}
	return res;
}

//-----------------------------------------------------------------------------
FPValue SimpleFaceDataAttrib::GetValues()
//-----------------------------------------------------------------------------
{
	std::auto_ptr<ISimpleFPChannel>pChan = WrapFaceChannel(_GetFaceChannel(), mChannelType);
	FPValue val(TYPE_VALUE, NULL);
	if (pChan->GetChannel()) {
		pChan->GetValues(val);
	}
	return val;
}
//-----------------------------------------------------------------------------
BOOL SimpleFaceDataAttrib::SetValues(Value* pVal)
//-----------------------------------------------------------------------------
{
	std::auto_ptr<ISimpleFPChannel>pChan = WrapFaceChannel(_GetFaceChannel(), mChannelType);
	BOOL res = FALSE;
	if (pChan->GetChannel()) {
		if (theHold.Holding()) {
			theHold.Put(new ValueListUndo(SimpleFaceDataCommon::FindParentObjFromAttrib(this),
				pChan->GetChannelID(),
				mChannelType));
		}
		try {
			// use maxscript sdk to convert from a value to an FPValue
			// val_to_FPValue will also convert from one number type (TYPE_INT, TYPE_STRING) to another (TYPE_INDEX)
			FPValue val;
			InterfaceFunction::val_to_FPValue(pVal, (ParamType2)(mChannelType + TYPE_TAB), val); //throws if unable to convert
			res = pChan->SetValues(&val);
			InterfaceFunction::release_param(val, (ParamType2)(mChannelType + TYPE_TAB), pVal);
		} catch (...) {
			res = FALSE;
		}
	}
	return res;
}
//-----------------------------------------------------------------------------
FPValue SimpleFaceDataAttrib::GetValueBySelection(const BitArray* pFaces)
//-----------------------------------------------------------------------------
{
	std::auto_ptr<ISimpleFPChannel>pChan = WrapFaceChannel(_GetFaceChannel(), mChannelType);
	FPValue res(TYPE_VALUE, NULL);
	if (pChan->GetChannel() && pFaces) {
		FPValue result;
		BOOL found = FALSE;
		for (ULONG i = 0; (i < pFaces->GetSize() && i < NumFaces()); i++) {
			if ((*pFaces)[i]) {
				pChan->GetValue(i, result);
				if (!found) {
					res = result;
					found = TRUE;
				}
				if (!pChan->Compare(result, res)) {
					res = FPValue(TYPE_VALUE, NULL);
					break;
				}
			}
		}
	}
	return res;
}
//-----------------------------------------------------------------------------
BOOL SimpleFaceDataAttrib::SetValueBySelection(const BitArray* pFaces, Value* pVal)
//-----------------------------------------------------------------------------
{
	std::auto_ptr<ISimpleFPChannel>pChan = WrapFaceChannel(_GetFaceChannel(), mChannelType);
	BOOL res = FALSE;
	if (pChan->GetChannel()) {
		if (theHold.Holding()) {
			theHold.Put(new ValueListUndo(SimpleFaceDataCommon::FindParentObjFromAttrib(this),
				pChan->GetChannelID(),
				mChannelType));
		}
		try {
			// use maxscript sdk to convert from a value to an FPValue
			// val_to_FPValue will also convert from one number type (TYPE_INT, TYPE_STRING) to another (TYPE_INDEX)
			FPValue val;
			InterfaceFunction::val_to_FPValue(pVal, (ParamType2)mChannelType, val);
			for (ULONG i = 0; (i < pFaces->GetSize() && i < NumFaces()); i++) {
				if ((*pFaces)[i]) {
					pChan->SetValue(i, &val);
				}
			}
			InterfaceFunction::release_param(val, (ParamType2)mChannelType, pVal);
			res = TRUE;
		} catch (...) {
			res = FALSE;
		}
	}
	return res;
}

//-----------------------------------------------------------------------------
Tab<DWORD> SimpleFaceDataAttrib::ChannelID()
//-----------------------------------------------------------------------------
{
	Tab<DWORD> classID;
	classID.SetCount(2);
	classID[0] = mChannelID.PartA();
	classID[1] = mChannelID.PartB();
	return classID;
}
// EOF
