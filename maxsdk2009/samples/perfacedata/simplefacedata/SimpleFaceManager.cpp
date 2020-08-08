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
	FILE: SimpleFaceManager.cpp

	DESCRIPTION: Implementation of ISimpleFaceDataManager
					- class definition
					- class instance / FP constructor

	AUTHOR: ktong - created 02.15.2006
***************************************************************************/


#include "SimpleFaceManager.h"
#include "SimpleFaceDataAttrib.h"
#include "SimpleFaceDataCommon.h"
#include "ICustAttribContainer.h"
#include "SimpleFaceManagerUndo.h"
#include "SimpleFaceDataValidator.h"

static ParamClassIDValidator theIDValidator;
static ParamObjectValidator theObjectValidator;
// Instance of the ISimpleFaceDataManager implementation.
// Declares publish names/types of the functions.
static SimpleFaceManagerImp theSimpleFaceManagerImp(
	SimpleFaceDataManager_InterfaceID, _T("simpleFaceManager"), IDS_UNDEFINED, NULL, FP_CORE,
	ISimpleFaceDataManager::eFpAddChannel, _T("addChannel"), IDS_UNDEFINED, TYPE_INTERFACE, 0, 4,
		_T("object"), IDS_UNDEFINED, TYPE_OBJECT, f_validator, &theObjectValidator,
		_T("type"), IDS_UNDEFINED, TYPE_ENUM, ISimpleFaceDataManager::eFpChannelTypeEnum, f_keyArgDefault, TYPE_INT,
		_T("id"), IDS_UNDEFINED, TYPE_DWORD_TAB, f_keyArgDefault, NULL, f_validator, &theIDValidator,
		_T("name"), IDS_UNDEFINED, TYPE_STRING, f_keyArgDefault, NULL,

	ISimpleFaceDataManager::eFpRemoveChannel, _T("removeChannel"), IDS_UNDEFINED, TYPE_VOID, 0, 2,
		_T("object"), IDS_UNDEFINED, TYPE_OBJECT,
		_T("id"), IDS_UNDEFINED, TYPE_DWORD_TAB, f_validator, &theIDValidator,

	ISimpleFaceDataManager::eFpGetChannel, _T("getChannel"), IDS_UNDEFINED, TYPE_INTERFACE, 0, 2,
		_T("object"), IDS_UNDEFINED, TYPE_OBJECT, f_validator, &theObjectValidator,
		_T("id"), IDS_UNDEFINED, TYPE_DWORD_TAB,  f_validator, &theIDValidator,

	ISimpleFaceDataManager::eFpGetChannels, _T("getChannels"), IDS_UNDEFINED, TYPE_INTERFACE_TAB_BV, 0, 1,
		_T("object"), IDS_UNDEFINED, TYPE_OBJECT, f_validator, &theObjectValidator,
		
	enums,
	ISimpleFaceDataManager::eFpChannelTypeEnum, 6,
				_T("integer"),		TYPE_INT,
				_T("index"),		TYPE_INDEX,
				_T("float"),		TYPE_FLOAT,
				_T("boolean"),		TYPE_BOOL,
				_T("point2"),		TYPE_POINT2_BV,
				_T("point3"),		TYPE_POINT3_BV,

	end
);

// ****************************************************************************
// Manager implementation
// ****************************************************************************
// ----------------------------------------------------------------------------
ISimpleFaceDataChannel* SimpleFaceManagerImp::AddChannel(Object* pObj, int type, const Class_ID* pChannelID, const TCHAR* pChannelName) const
// ----------------------------------------------------------------------------
{
	pObj = SimpleFaceDataCommon::FindBaseFromObject(pObj);

	// We can work with Face Data in a couple different object types:
	if ( pObj && (pObj->IsSubClassOf(triObjectClassID) || pObj->IsSubClassOf(polyObjectClassID)) )
	{
		// add custom attribute to object
		ICustAttribContainer* cc = pObj->GetCustAttribContainer();

		if (!cc) { // add a container channel
			pObj->AllocCustAttribContainer();
			cc = pObj->GetCustAttribContainer();
		}

		IFaceDataMgr* pFDMgr = SimpleFaceDataCommon::FindFaceDataFromObj(pObj);
		if (cc && pFDMgr && 
			((pChannelID == NULL) || !pFDMgr->GetFaceDataChan(*pChannelID)) // no id specified, or the specified id does not already exist
			) {
			// create channel ID
			Class_ID newChannelID;
			if (pChannelID) { // ID specified, just assign it
				newChannelID = *pChannelID;
			} else {	// generate a non conflicting ID
				RandGenerator classIDGenerator;
				do
				{
					newChannelID.SetPartA(0x7fffffff & classIDGenerator.rand());
					newChannelID.SetPartB(0x7fffffff & classIDGenerator.rand());
				} while (pFDMgr->GetFaceDataChan(newChannelID)); // channel exists, make a new ID
			}
			// create a channel
			std::auto_ptr<ISimpleFPChannel> pNewChannel = SimpleFaceDataAttrib::WrapFaceChannel(
					SimpleFaceDataAttrib::CreateFaceChannel(type, SimpleFaceDataCommon::GetNumFacesFromObject(pObj)),
					type);
			if (pNewChannel->GetChannel()) {
				// create attrib
				SimpleFaceDataAttrib* pNewAttrib = new SimpleFaceDataAttrib();

				// associate a channel with the attribute
				pNewChannel->SetChannelID(newChannelID);
				pNewAttrib->SetChannelID(pNewChannel->GetChannelID());
				pNewAttrib->SetChannelType(type);
				if (pChannelName) {
					pNewChannel->SetChannelName(pChannelName);
				}

				// we need a restore object put for pFDMgr
				if (theHold.Holding()) {
					theHold.Put(new AddChannelUndo(pObj, pNewChannel->GetChannelID()));
				}
				pFDMgr->AddFaceDataChan( pNewChannel->GetChannel() );
				cc->AppendCustAttrib( pNewAttrib ); // does undo

				return pNewAttrib;
			}
		}
	}
	return NULL;
}
// ----------------------------------------------------------------------------
ISimpleFaceDataChannel* SimpleFaceManagerImp::AddChannel(Object* pObj, int type, const Tab<DWORD>* pChannelID, const TCHAR* pChannelName) const
// ----------------------------------------------------------------------------
{
	if (pChannelID == NULL) { // id unspecified
		return AddChannel(pObj, type, (Class_ID*)NULL, pChannelName);
	}
	if (pChannelID && (pChannelID->Count() == 2)) {// id is specified and is the right size
		Class_ID channelID = Class_ID((*pChannelID)[0], (*pChannelID)[1]);
		return AddChannel(pObj, type, &channelID, pChannelName);
	}
	return NULL;
}
// ----------------------------------------------------------------------------
void SimpleFaceManagerImp::RemoveChannel(Object* pObj, const Class_ID &channelID)
// ----------------------------------------------------------------------------
{
	pObj = SimpleFaceDataCommon::FindBaseFromObject(pObj);

	if (pObj == NULL) {
		return;
	}
	IFaceDataMgr* pFDMgr = SimpleFaceDataCommon::FindFaceDataFromObj(pObj);
	ICustAttribContainer* cc = pObj->GetCustAttribContainer();

	if (cc) {
		for (int i = 0; i < cc->GetNumCustAttribs(); i++) {
			CustAttrib* ca = cc->GetCustAttrib(i);
			if (ca->ClassID() == SimpleFaceData_ClassID) {
				SimpleFaceDataAttrib* pCA = static_cast<SimpleFaceDataAttrib*>(ca);
				if (pCA->GetChannelID() == channelID) {
					// need to put a restore object here since pFDMgr doesn't do this.
					if (pFDMgr) {
						if (theHold.Holding()) {
							theHold.Put(new RemoveChannelUndo(pObj, pCA->GetChannelID()));
						}
						pFDMgr->RemoveFaceDataChan(pCA->GetChannelID());
					}
					
					// custom attribute container does undo
					cc->RemoveCustAttrib(i);
				}
			}
		}
	}
}
// ----------------------------------------------------------------------------
void SimpleFaceManagerImp::RemoveChannel(Object* pObj, const Tab<DWORD>* pChannelID)
// ----------------------------------------------------------------------------
{
	if (pChannelID && (pChannelID->Count() == 2) ) {
		RemoveChannel(pObj, Class_ID((*pChannelID)[0], (*pChannelID)[1]));
	}
}
// ----------------------------------------------------------------------------
ISimpleFaceDataChannel* SimpleFaceManagerImp::GetChannel(Object* pObj, const Class_ID &channelID) const
// ----------------------------------------------------------------------------
{
	if (pObj == NULL) {
		return NULL;
	}
	ICustAttribContainer* cc = pObj->GetCustAttribContainer();
	if (cc) {
		for (int i = 0; i < cc->GetNumCustAttribs(); i++) {
			CustAttrib* ca = cc->GetCustAttrib(i);
			if (ca->ClassID() == SimpleFaceData_ClassID) {
				SimpleFaceDataAttrib* pCA = static_cast<SimpleFaceDataAttrib*>(ca);
				if (pCA->GetChannelID() == channelID) {
					return pCA;
				}
			}
		}
	}
	return NULL;
}
// ----------------------------------------------------------------------------
ISimpleFaceDataChannel* SimpleFaceManagerImp::GetChannel(Object* pObj, const Tab<DWORD>* pChannelID) const
// ----------------------------------------------------------------------------
{
	if (pChannelID && (pChannelID->Count() == 2) ) {
		return GetChannel(pObj, Class_ID((*pChannelID)[0], (*pChannelID)[1]));
	}
	return NULL;
}

// ----------------------------------------------------------------------------
Tab<ISimpleFaceDataChannel*> SimpleFaceManagerImp::GetChannels(Object* pObj) const
// ----------------------------------------------------------------------------
{
	Tab<ISimpleFaceDataChannel*> channels;
	if (pObj == NULL) {
		return channels;
	}
	ICustAttribContainer* cc = pObj->GetCustAttribContainer();
	if (cc) {
		for (int i = 0; i < cc->GetNumCustAttribs(); i++) {
			CustAttrib* ca = cc->GetCustAttrib(i);
			if (ca->ClassID() == SimpleFaceData_ClassID) {
				ISimpleFaceDataChannel* pCA = static_cast<SimpleFaceDataAttrib*>(ca);
				channels.Append(1, &pCA);
			}
		}
	}
	return channels;
}