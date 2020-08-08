//**************************************************************************/
// Copyright (c) 1998-2005 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Defines a class for monitoring a ReferenceTarget for deletion.
// AUTHOR: Larry.Minton - created April.20.2006
//***************************************************************************/

#include "ctrl.h"
#include "RefTargMonitorClass.h"
#include "ICustAttribContainer.h"

static BYTE CurrentRefTargMonitorVersion = 2;

RefTargMonitorClass::RefTargMonitorClass() 
{
	theRefTargWatcher = NULL; 
	suspendDelMessagePassing = suspendSetMessagePassing = deleteMe = false;
	shouldPersistIndirectRef = false;
}
RefTargMonitorClass::RefTargMonitorClass(RefTargetHandle theRefTarg)
{
	theRefTargWatcher = new RefTargMonitorRefMaker(*this, theRefTarg);
	suspendDelMessagePassing = suspendSetMessagePassing = deleteMe = false;
	shouldPersistIndirectRef = false;
}
RefTargMonitorClass::~RefTargMonitorClass() 
{ 
	theHold.Suspend();
	// if deleteMe is true, we are in delete resulting from the watched RefTarg being deleted. We are being 
	// called from ProcessRefTargMonitorMsg which is handling the message REFMSG_REFTARGMONITOR_TARGET_DELETED. 
	// This message is being sent by theRefTargWatcher, and we don't want to delete it here. Instead, we will 
	// return REF_AUTO_DELETE from ProcessRefTargMonitorMsg to delete theRefTargWatcher 
	if (theRefTargWatcher && !deleteMe)
		theRefTargWatcher->DeleteThis(); 
	theRefTargWatcher = NULL;
	DeleteAllRefs(); 
	theHold.Resume();
}

void RefTargMonitorClass::DeleteThis()
{
	// if suspendDelMessagePassing is true, we are in the middle of handling a 
	// REFMSG_REFTARGMONITOR_TARGET_DELETED message in ProcessRefTargMonitorMsg (the watched RefTarg 
	// was deleted). Delay deletion until we are back in ProcessRefTargMonitorMsg.
	if (suspendDelMessagePassing)
	{
		deleteMe = true;
		return;
	}
	delete this;
}

void RefTargMonitorClass::SetRefTarg(RefTargetHandle theRefTarg)
{
	if (theRefTargWatcher == NULL)
		theRefTargWatcher = new RefTargMonitorRefMaker(*this, theRefTarg);
	else
		theRefTargWatcher->SetRef(theRefTarg);
}

RefTargetHandle RefTargMonitorClass::GetRefTarg()
{
	if (theRefTargWatcher == NULL)
		return NULL;
	return theRefTargWatcher->GetRef();
}

bool RefTargMonitorClass::GetPersist()
{
	return shouldPersistIndirectRef;
}

void RefTargMonitorClass::SetPersist(bool persist)
{
	shouldPersistIndirectRef = persist;
}

RefResult RefTargMonitorClass::ProcessRefTargMonitorMsg(
	Interval changeInt, 
	RefTargetHandle hTarget, 
	PartID& partID,  
	RefMessage message,
	bool fromRefTarg)
{
	// only pass along a message if we are not already passing the same message. This breaks
	// the message loop recursion
	if (!suspendDelMessagePassing && fromRefTarg && message == REFMSG_TARGET_DELETED)
	{
		// We can't send the REFMSG_TARGET_DELETED message, because it will look like this RefTargMonitorClass
		// is being deleted rather than RefTarg. That does very bad things to any RestoreObjs that hold this 
		// RefTargMonitorClass as a reference. Send REFMSG_REFTARGMONITOR_TARGET_DELETED instead, and pass
		// the RefTarg being deleted as hTarg.
		// We are by definition being called from theRefTargWatcher. If an attempt is made to delete this 
		// RefTargMonitorClass via DeleteThis while processing this message, delay the deletion until after 
		// the NotifyDependents call returns (since we are accessing member variables) and return 
		// REF_AUTO_DELETE so that theRefTargWatcher is deleted. 
		suspendDelMessagePassing = true;
		NotifyDependents(changeInt,PART_ALL,REFMSG_REFTARGMONITOR_TARGET_DELETED,NOTIFY_ALL,FALSE,hTarget);
		suspendDelMessagePassing = false;
		if (deleteMe)
		{
			delete this;
			return REF_AUTO_DELETE;
		}
	}
	else if(!suspendSetMessagePassing &&message==REFMSG_REFTARGMONITOR_TARGET_SET)
	{
		suspendSetMessagePassing = true;
		NotifyDependents(changeInt,PART_ALL,REFMSG_REFTARGMONITOR_TARGET_SET,NOTIFY_ALL,FALSE,hTarget);
		suspendSetMessagePassing = false;
	}
	return REF_SUCCEED;

}

int RefTargMonitorClass::ProcessEnumDependents(DependentEnumProc* dep)
{
	return 0;
}

int RefTargMonitorClass::NumIndirectRefs()
{
	return 1;
}

RefTargetHandle RefTargMonitorClass::GetIndirectReference(int i)
{
	return (theRefTargWatcher) ? theRefTargWatcher->GetRef() : NULL;
}

void RefTargMonitorClass::SetIndirectReference(int i, RefTargetHandle rtarg)
{
	if (theRefTargWatcher == NULL)
		theRefTargWatcher = new RefTargMonitorRefMaker(*this, rtarg);
	else
		theRefTargWatcher->SetRef(rtarg); 
}

BOOL RefTargMonitorClass::ShouldPersistIndirectRef(RefTargetHandle ref) 
{
	return shouldPersistIndirectRef; 
}

RefResult RefTargMonitorClass::NotifyRefChanged(
	Interval changeInt, 
	RefTargetHandle hTarget, 
	PartID& partID,  
	RefMessage message)
{
	return REF_SUCCEED;
}

RefTargetHandle RefTargMonitorClass::Clone(RemapDir& remap) 
{
	HoldSuspend hs;
	RefTargMonitorClass* newobj = new RefTargMonitorClass();
	newobj->shouldPersistIndirectRef = shouldPersistIndirectRef;

	if (theRefTargWatcher)
	{
		RefTargetHandle watchedRefTarg = theRefTargWatcher->GetRef();
		RefTargetHandle targetRefTarg = remap.FindMapping( watchedRefTarg );

		// if targetRefTarg is NULL, the watched RefTarg hasn't been cloned. It might be cloned
		// later, or it might not. Register a post patch proc to check after all cloning is 
		// finished. If watched RefTarg isn't cloned, newobj should watch this watched RefTarg.
		bool doBackPatch = false;
		if (targetRefTarg == NULL)
		{
			targetRefTarg = watchedRefTarg;
			doBackPatch = true;
		}
		newobj->theRefTargWatcher = new RefTargMonitorRefMaker(*newobj,targetRefTarg);
		if (doBackPatch)
			remap.AddPostPatchProc(newobj->theRefTargWatcher,false);
	}


	BaseClone(this, newobj, remap);
	return newobj;
}

#define REFTARGMONITOR_VERSION			0x55a1
#define REFTARGMONITOR_PERSIST			0x55a2

IOResult RefTargMonitorClass::Load(ILoad *iload)
{
	ULONG 	nb;
	IOResult res;
	USHORT id;

	loadVer = 0;

	while (IO_OK==(res=iload->OpenChunk())) 
	{
		switch (id = iload->CurChunkID()) 
		{
		case REFTARGMONITOR_VERSION:
			iload->Read(&loadVer, sizeof(BYTE), &nb);
			break;
		case REFTARGMONITOR_PERSIST:
			iload->Read(&shouldPersistIndirectRef, sizeof(bool), &nb);
			break;
		}
		iload->CloseChunk();
	}
	return IO_OK;
}

IOResult RefTargMonitorClass::Save(ISave *isave)
{
	ULONG 	nb;

	isave->BeginChunk(REFTARGMONITOR_VERSION);
	isave->Write(&CurrentRefTargMonitorVersion, sizeof(BYTE), &nb);
	isave->EndChunk();

	isave->BeginChunk(REFTARGMONITOR_PERSIST);
	isave->Write(&shouldPersistIndirectRef, sizeof(bool), &nb);
	isave->EndChunk();

	return IO_OK;
}

int RefTargMonitorClass::RemapRefOnLoad(int iref) 
{
	return iref; 
}


int RefTargMonitorClass::NumRefs() 
{
	return 0;
}

ReferenceTarget* RefTargMonitorClass::GetReference(int i) 
{ 
	return NULL;
}

void RefTargMonitorClass::SetReference(int i, RefTargetHandle rtarg) 
{
}

class RefTargMonitorDesc: public ClassDesc {
public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new RefTargMonitorClass;}
	const TCHAR *	ClassName() { return _T("RefTargMonitor"); }
	SClass_ID		SuperClassID() { return REF_TARGET_CLASS_ID; }
	Class_ID		ClassID() { return REFTARGMONITOR_CLASS_ID; }
	const TCHAR* 	Category() {return _T("");}
};

static RefTargMonitorDesc refTargMonitorDesc;
ClassDesc *GetRefTargMonitorDesc() {return &refTargMonitorDesc;}

//Function Publishing descriptor for Mixin interface
//*****************************************************
static FPInterfaceDesc IRefTargMonitorClass_interface(
	IID_REFTARGMONITOR_CLASS, _T("IRefTargMonitor"), 0, &refTargMonitorDesc, FP_MIXIN,
	properties,
		IRefTargMonitorClass::kfpGetRefTarg, IRefTargMonitorClass::kfpSetRefTarg, _T("refTarg"), 0, TYPE_REFTARG,
		IRefTargMonitorClass::kfpGetPersist, IRefTargMonitorClass::kfpSetPersist, _T("persist"), 0, TYPE_bool,
	end
	);

//  Get Descriptor method for Mixin Interface
//  *****************************************
FPInterfaceDesc* IRefTargMonitorClass::GetDesc()
{
	return &IRefTargMonitorClass_interface;
}
// End of Function Publishing Code

