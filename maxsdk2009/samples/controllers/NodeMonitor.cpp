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
// DESCRIPTION: Defines a class for monitoring a Node for deletion.
// AUTHOR: Larry.Minton - created Jan.11.2006
//***************************************************************************/

#include "ctrl.h"
#include "NodeMonitor.h"
#include "ICustAttribContainer.h"

static BYTE CurrentNodeMonitorVersion = 1;

NodeMonitor::NodeMonitor() 
{
	theNodeWatcher = NULL; 
	suspendDelMessagePassing = suspendSetMessagePassing = deleteMe = false;
}
NodeMonitor::NodeMonitor(INode *theNode)
{
	theNodeWatcher = new RefTargMonitorRefMaker(*this, theNode);
	suspendDelMessagePassing = suspendSetMessagePassing = deleteMe = false;
}
NodeMonitor::~NodeMonitor() 
{ 
	theHold.Suspend();
	// if deleteMe is true, we are in delete resulting from the watched node being deleted. We are being called from ProcessRefTargMonitorMsg
	// which is handling the message REFMSG_NODEMONITOR_TARGET_DELETED. This message is being sent by theNodeWatcher, and we don't 
	// want to delete it here. Instead, we will return REF_AUTO_DELETE from ProcessRefTargMonitorMsg to delete theNodeWatcher 
	if (theNodeWatcher && !deleteMe)
		theNodeWatcher->DeleteThis(); 
	theNodeWatcher = NULL;
	DeleteAllRefs(); 
	theHold.Resume();
}

void NodeMonitor::DeleteThis()
{
	// if suspendDelMessagePassing is true, we are in the middle of handling a REFMSG_NODEMONITOR_TARGET_DELETED
	// message in ProcessRefTargMonitorMsg (the watched node was deleted). Delay deletion until we are back in ProcessRefTargMonitorMsg.
	if (suspendDelMessagePassing)
	{
		deleteMe = true;
		return;
	}
	delete this;
}

void NodeMonitor::SetNode(INode* theNode)
{
	if (theNodeWatcher == NULL)
		theNodeWatcher = new RefTargMonitorRefMaker(*this, theNode);
	else
		theNodeWatcher->SetRef(theNode);
}

INode* NodeMonitor::GetNode()
{
	if (theNodeWatcher == NULL)
		return NULL;
	return (INode*)theNodeWatcher->GetRef();
}

RefResult NodeMonitor::ProcessRefTargMonitorMsg(
	Interval changeInt, 
	RefTargetHandle hTarget, 
	PartID& partID,  
	RefMessage message,
	bool fromNode)
{
	// only pass along a message if we are not already passing the same message. This breaks
	// the message loop recursion
	if (!suspendDelMessagePassing && fromNode && message == REFMSG_TARGET_DELETED)
	{
		// We can't send the REFMSG_TARGET_DELETED message, because it will look like this NodeMonitor
		// is being deleted rather than node. That does very bad things to any RestoreObjs that hold this NodeMonitor
		// as a reference. Send REFMSG_NODEMONITOR_TARGET_DELETED instead, and pass
		// the node being deleted as hTarg.
		// We are by definition being called from theNodeWatcher. If an attempt is made to delete this NodeMonitor
		// via DeleteThis while processing this message, delay the deletion until after the NotifyDependents call returns
		// (since we are accessing member variables) and return REF_AUTO_DELETE so that theNodeWatcher is deleted. 
		suspendDelMessagePassing = true;
		NotifyDependents(changeInt,PART_ALL,REFMSG_NODEMONITOR_TARGET_DELETED,NOTIFY_ALL,FALSE,hTarget);
		suspendDelMessagePassing = false;
		if (deleteMe)
		{
			delete this;
			return REF_AUTO_DELETE;
		}
	}
	else if(!suspendSetMessagePassing && message==REFMSG_REFTARGMONITOR_TARGET_SET)
	{
		suspendSetMessagePassing = true;
		NotifyDependents(changeInt,PART_ALL,REFMSG_NODEMONITOR_TARGET_SET,NOTIFY_ALL,FALSE,hTarget);
		suspendSetMessagePassing = false;
	}

	return REF_SUCCEED;

}

int NodeMonitor::ProcessEnumDependents(DependentEnumProc* dep)
{
	return 0;
}

int NodeMonitor::NumIndirectRefs()
{
	return 1;
}

RefTargetHandle NodeMonitor::GetIndirectReference(int i)
{
	return (theNodeWatcher) ? theNodeWatcher->GetRef() : NULL;
}

void NodeMonitor::SetIndirectReference(int i, RefTargetHandle rtarg)
{
	if (theNodeWatcher == NULL)
		theNodeWatcher = new RefTargMonitorRefMaker(*this, rtarg);
	else
		theNodeWatcher->SetRef(rtarg); 
}

RefResult NodeMonitor::NotifyRefChanged(
	Interval changeInt, 
	RefTargetHandle hTarget, 
	PartID& partID,  
	RefMessage message)
{
	return REF_SUCCEED;
}

RefTargetHandle NodeMonitor::Clone(RemapDir& remap) 
{
	NodeMonitor* newobj = new NodeMonitor();

	if (theNodeWatcher)
	{
		RefTargetHandle watchedNode = theNodeWatcher->GetRef();
		RefTargetHandle targetNode = remap.FindMapping( watchedNode );

		// if targetNode is NULL, the watched node hasn't been cloned. It might be cloned
		// later, or it might not. Register a post patch proc to check after all cloning is 
		// finished. If watched node isn't cloned, newobj should watch this watched node.
		bool doBackPatch = false;
		if (targetNode == NULL)
		{
			targetNode = watchedNode;
			doBackPatch = true;
		}
		newobj->theNodeWatcher = new RefTargMonitorRefMaker(*newobj,targetNode);
		if (doBackPatch)
			remap.AddPostPatchProc(newobj->theNodeWatcher,false);
	}


	BaseClone(this, newobj, remap);
	return newobj;
}

#define NODEMONITOR_VERSION			0x5fa1

IOResult NodeMonitor::Load(ILoad *iload)
{
	ULONG 	nb;
	IOResult res;
	USHORT id;

	loadVer = 0;

	while (IO_OK==(res=iload->OpenChunk())) 
	{
		switch (id = iload->CurChunkID()) 
		{
		case NODEMONITOR_VERSION:
			iload->Read(&loadVer, sizeof(BYTE), &nb);
			break;
		}
		iload->CloseChunk();
	}
	return IO_OK;
}

IOResult NodeMonitor::Save(ISave *isave)
{
	ULONG 	nb;

	isave->BeginChunk(NODEMONITOR_VERSION);
	isave->Write(&CurrentNodeMonitorVersion, sizeof(BYTE), &nb);
	isave->EndChunk();

	return IO_OK;
}

int NodeMonitor::RemapRefOnLoad(int iref) 
{
	return iref; 
}


int NodeMonitor::NumRefs() 
{
	return 0;
}

ReferenceTarget* NodeMonitor::GetReference(int i) 
{ 
	return NULL;
}

void NodeMonitor::SetReference(int i, RefTargetHandle rtarg) 
{
}

class NodeMonitorDesc: public ClassDesc {
public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new NodeMonitor;}
	const TCHAR *	ClassName() { return _T("NodeMonitor"); }
	SClass_ID		SuperClassID() { return REF_TARGET_CLASS_ID; }
	Class_ID		ClassID() { return NODEMONITOR_CLASS_ID; }
	const TCHAR* 	Category() {return _T("");}
};

static NodeMonitorDesc nodeMonitorDesc;
ClassDesc *GetNodeMonitorDesc() {return &nodeMonitorDesc;}

//Function Publishing descriptor for Mixin interface
//*****************************************************
static FPInterfaceDesc INodeMonitor_interface(
	IID_NODEMONITOR, _T("INodeMonitor"), 0, &nodeMonitorDesc, FP_MIXIN,
	properties,
		INodeMonitor::kfpGetnode, INodeMonitor::kfpSetnode, _T("node"), 0, TYPE_INODE,
	end
	);

//  Get Descriptor method for Mixin Interface
//  *****************************************
FPInterfaceDesc* INodeMonitor::GetDesc()
{
	return &INodeMonitor_interface;
}
// End of Function Publishing Code

