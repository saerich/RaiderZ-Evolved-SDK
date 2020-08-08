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
// DESCRIPTION: Defines a class for monitoring a Node for its PART_TM messages.
// AUTHOR: Larry.Minton - created May.14.2005
//***************************************************************************/

#include "ctrl.h"
#include "NodeTransformMonitor.h"
#include "ICustAttribContainer.h"

static BYTE CurrentNodeTransformMonitorVersion = 2;

NodeTransformMonitor::NodeTransformMonitor() 
{
	theNodeWatcher = NULL; 
	suspendTMMessagePassing = suspendDelMessagePassing = suspendFlagNodesMessagePassing = suspendSetMessagePassing= deleteMe = false;
	suspendProcessEnumDependents = false;
	forwardPartTM = forwardFlagNodes = forwardEnumDependents = true;
}
NodeTransformMonitor::NodeTransformMonitor(INode *theNode, bool forwardTransformChangeMsgs, bool forwardFlagNodesMsgs, bool forwardEnumDependentsCalls)
{
	theNodeWatcher = new RefTargMonitorRefMaker(*this, theNode);
	suspendTMMessagePassing = suspendDelMessagePassing = suspendFlagNodesMessagePassing = suspendSetMessagePassing = deleteMe = false;
	suspendProcessEnumDependents = false;
	forwardPartTM = forwardTransformChangeMsgs;
	forwardFlagNodes = forwardFlagNodesMsgs;
	forwardEnumDependents = forwardEnumDependentsCalls;
}
NodeTransformMonitor::~NodeTransformMonitor() 
{ 
	theHold.Suspend();
	// if deleteMe is true, we are in delete resulting from the watched node being deleted. We are being called from ProcessRefTargMonitorMsg
	// which is handling the message REFMSG_NODETRANSFORMMONITOR_TARGET_DELETED. This message is being sent by theNodeWatcher, and we don't 
	// want to delete it here. Instead, we will return REF_AUTO_DELETE from ProcessRefTargMonitorMsg to delete theNodeWatcher 
	if (theNodeWatcher && !deleteMe)
		theNodeWatcher->DeleteThis(); 
	theNodeWatcher = NULL;
	DeleteAllRefs(); 
	theHold.Resume();
}

void NodeTransformMonitor::DeleteThis()
{
	// if suspendDelMessagePassing is true, we are in the middle of handling a REFMSG_NODETRANSFORMMONITOR_TARGET_DELETED
	// message in ProcessRefTargMonitorMsg (the watched node was deleted). Delay deletion until we are back in ProcessRefTargMonitorMsg.
	if (suspendDelMessagePassing)
	{
		deleteMe = true;
		return;
	}
	delete this;
}

void NodeTransformMonitor::SetNode(INode* theNode)
{
	if (theNodeWatcher == NULL)
		theNodeWatcher = new RefTargMonitorRefMaker(*this, theNode);
	else
		theNodeWatcher->SetRef(theNode);
}

INode* NodeTransformMonitor::GetNode()
{
	if (theNodeWatcher == NULL)
		return NULL;
	return (INode*)theNodeWatcher->GetRef();
}

void NodeTransformMonitor::SetForwardTransformChangeMsgs(bool state)
{
	forwardPartTM = state;
}

bool NodeTransformMonitor::GetForwardTransformChangeMsgs()
{
	return forwardPartTM;
}

void NodeTransformMonitor::SetForwardFlagNodesMsgs(bool state)
{
	forwardFlagNodes = state;
}

bool NodeTransformMonitor::GetForwardFlagNodesMsgs()
{
	return forwardFlagNodes;
}

void NodeTransformMonitor::SetForwardEnumDependentsCalls(bool state)
{
	forwardEnumDependents = state;
}

bool NodeTransformMonitor::GetForwardEnumDependentsCalls()
{
	return forwardEnumDependents;
}

RefResult NodeTransformMonitor::ProcessRefTargMonitorMsg(
	Interval changeInt, 
	RefTargetHandle hTarget, 
	PartID& partID,  
	RefMessage message,
	bool fromNode)
{
	// only pass along a message if we are not already passing the same message. This breaks
	// the message loop recursion
	if (!suspendTMMessagePassing && fromNode && message == REFMSG_CHANGE && partID==PART_TM && forwardPartTM)
	{
		suspendTMMessagePassing = true;
		NotifyDependents(changeInt,partID,message);
		suspendTMMessagePassing = false;
	}
	else if (!suspendDelMessagePassing && fromNode && message == REFMSG_TARGET_DELETED)
	{
		// We can't send the REFMSG_TARGET_DELETED message, because it will look like this NodeTransformMonitor
		// is being deleted rather than node. That does very bad things to any RestoreObjs that hold this NodeTransformMonitor
		// as a reference. Send REFMSG_NODETRANSFORMMONITOR_TARGET_DELETED instead, and pass
		// the node being deleted as hTarg.
		// We are by definition being called from theNodeWatcher. If an attempt is made to delete this NodeTransformMonitor
		// via DeleteThis while processing this message, delay the deletion until after the NotifyDependents call returns
		// (since we are accessing member variables) and return REF_AUTO_DELETE so that theNodeWatcher is deleted. 
		suspendDelMessagePassing = true;
		NotifyDependents(changeInt,PART_ALL,REFMSG_NODETRANSFORMMONITOR_TARGET_DELETED,NOTIFY_ALL,FALSE,hTarget);
		suspendDelMessagePassing = false;
		if (deleteMe)
		{
			delete this;
			return REF_AUTO_DELETE;
		}
	}
	else if (!suspendFlagNodesMessagePassing && fromNode && message == REFMSG_FLAG_NODES_WITH_SEL_DEPENDENTS && forwardFlagNodes)
	{
		// this message is used for identifying which nodes to save when doing a save selected.
		suspendFlagNodesMessagePassing = true;
		NotifyDependents(changeInt,partID,message,NOTIFY_ALL,TRUE,hTarget);
		suspendFlagNodesMessagePassing = false;
	}
	else if(!suspendSetMessagePassing && message==REFMSG_REFTARGMONITOR_TARGET_SET)
	{
		suspendSetMessagePassing = true;
		NotifyDependents(changeInt,PART_ALL,REFMSG_NODETRANSFORMMONITOR_TARGET_SET,NOTIFY_ALL,FALSE,hTarget);
		suspendSetMessagePassing = false;
	}
	return REF_SUCCEED;

}

int NodeTransformMonitor::ProcessEnumDependents(DependentEnumProc* dep)
{
	if (!suspendProcessEnumDependents && forwardEnumDependents)
	{
		suspendProcessEnumDependents = true;
		int res = DoEnumDependents(dep);
		suspendProcessEnumDependents = false;
		return res;
	}
	else
		return 0;
}

int NodeTransformMonitor::NumIndirectRefs()
{
	return 1;
}

RefTargetHandle NodeTransformMonitor::GetIndirectReference(int i)
{
	return (theNodeWatcher) ? theNodeWatcher->GetRef() : NULL;
}

void NodeTransformMonitor::SetIndirectReference(int i, RefTargetHandle rtarg)
{
	if (theNodeWatcher == NULL)
		theNodeWatcher = new RefTargMonitorRefMaker(*this, rtarg);
	else
		theNodeWatcher->SetRef(rtarg); 
}

RefResult NodeTransformMonitor::NotifyRefChanged(
	Interval changeInt, 
	RefTargetHandle hTarget, 
	PartID& partID,  
	RefMessage message)
{
	return REF_SUCCEED;
}

RefTargetHandle NodeTransformMonitor::Clone(RemapDir& remap) 
{
	NodeTransformMonitor* newobj = new NodeTransformMonitor();
	newobj->forwardPartTM = forwardPartTM;
	newobj->forwardFlagNodes = forwardFlagNodes;
	newobj->forwardEnumDependents = forwardEnumDependents;

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

#define NODETRANSFORMMONITOR_BEGIN_CHUNK		0x5fa0
#define NODETRANSFORMMONITOR_VERSION			0x5fa1
#define NODETRANSFORMMONITOR_PARTTM_CHUNK		0x5fa2
#define NODETRANSFORMMONITOR_FLAGNODES_CHUNK	0x5fa3
#define NODETRANSFORMMONITOR_ENUMDEP_CHUNK		0x5fa4

IOResult NodeTransformMonitor::Load(ILoad *iload)
{
	ULONG 	nb;
	IOResult res;
	USHORT id;
	bool isNullPtr;
	bool skipCloseChunk;

	loadVer = 0;

	while (IO_OK==(res=iload->OpenChunk())) 
	{
		skipCloseChunk = false;
		switch (id = iload->CurChunkID()) 
		{
		// Note: following chunk no longer written, but is present to handle alpha/beta r8 files
		// The node being monitored is now being set on load via the IIndirectReference interface
		case NODETRANSFORMMONITOR_BEGIN_CHUNK: 
			iload->Read(&isNullPtr, sizeof(bool), &nb);
			if (!isNullPtr)
			{
				skipCloseChunk = true;
				iload->CloseChunk();
				theNodeWatcher = new RefTargMonitorRefMaker(*this);
				theNodeWatcher->Load(iload);
			}
			break;
		case NODETRANSFORMMONITOR_VERSION:
			iload->Read(&loadVer, sizeof(BYTE), &nb);
			break;
		case NODETRANSFORMMONITOR_PARTTM_CHUNK:
			iload->Read(&forwardPartTM, sizeof(bool), &nb);
			break;
		case NODETRANSFORMMONITOR_FLAGNODES_CHUNK:
			iload->Read(&forwardFlagNodes, sizeof(bool), &nb);
			break;
		case NODETRANSFORMMONITOR_ENUMDEP_CHUNK:
			iload->Read(&forwardEnumDependents, sizeof(bool), &nb);
			break;
		}
		if (!skipCloseChunk)
			iload->CloseChunk();
	}
	return IO_OK;
}

IOResult NodeTransformMonitor::Save(ISave *isave)
{
	ULONG 	nb;

	// Note: following chunk no longer written, but is read to handle alpha/beta r8 files
	// The node being monitored is now being set on load via the IIndirectReference interface
/*
	bool isNullPtr = (theNodeWatcher == NULL);
	isave->BeginChunk(NODETRANSFORMMONITOR_BEGIN_CHUNK);
	IOResult res = isave->Write(&isNullPtr, sizeof(bool), &nb);
	if(res != IO_OK) return res;
	isave->EndChunk(); 
	if (theNodeWatcher)
		res = theNodeWatcher->Save(isave);
	if(res != IO_OK) return res;
*/
	isave->BeginChunk(NODETRANSFORMMONITOR_VERSION);
	isave->Write(&CurrentNodeTransformMonitorVersion, sizeof(BYTE), &nb);
	isave->EndChunk();

	isave->BeginChunk(NODETRANSFORMMONITOR_PARTTM_CHUNK);
	isave->Write(&forwardPartTM, sizeof(bool), &nb);
	isave->EndChunk();

	isave->BeginChunk(NODETRANSFORMMONITOR_FLAGNODES_CHUNK);
	isave->Write(&forwardFlagNodes, sizeof(bool), &nb);
	isave->EndChunk();

	isave->BeginChunk(NODETRANSFORMMONITOR_ENUMDEP_CHUNK);
	isave->Write(&forwardEnumDependents, sizeof(bool), &nb);
	isave->EndChunk();

	return IO_OK;
}

int NodeTransformMonitor::RemapRefOnLoad(int iref) 
{
	return (loadVer == 0) ? -1 : iref; 
}


int NodeTransformMonitor::NumRefs() 
{
	return 0;
}

ReferenceTarget* NodeTransformMonitor::GetReference(int i) 
{ 
	return NULL;
}

void NodeTransformMonitor::SetReference(int i, RefTargetHandle rtarg) 
{
}

class NodeTransformMonitorDesc: public ClassDesc {
public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new NodeTransformMonitor;}
	const TCHAR *	ClassName() { return _T("NodeTransformMonitor"); }
	SClass_ID		SuperClassID() { return REF_TARGET_CLASS_ID; }
	Class_ID		ClassID() { return NODETRANSFORMMONITOR_CLASS_ID; }
	const TCHAR* 	Category() {return _T("");}
};

static NodeTransformMonitorDesc nodeTransformMonitorDesc;
ClassDesc *GetNodeTransformMonitorDesc() {return &nodeTransformMonitorDesc;}

//Function Publishing descriptor for Mixin interface
//*****************************************************
static FPInterfaceDesc INodeTransformMonitor_interface(
	IID_NODETRANSFORMMONITOR, _T("INodeTransformMonitor"), 0, &nodeTransformMonitorDesc, FP_MIXIN,
	properties,
		INodeTransformMonitor::kfpGetnode, INodeTransformMonitor::kfpSetnode, _T("node"), 0, TYPE_INODE,
		INodeTransformMonitor::kfpGetforwardPartTM, INodeTransformMonitor::kfpSetforwardPartTM, _T("forwardTransformChangeMsgs"), 0, TYPE_bool,
		INodeTransformMonitor::kfpGetforwardFlagNodes, INodeTransformMonitor::kfpSetforwardFlagNodes, _T("forwardFlagNodesMsgs"), 0, TYPE_bool,
		INodeTransformMonitor::kfpGetforwardEnumDependents, INodeTransformMonitor::kfpSetforwardEnumDependents, _T("forwardEnumDependents"), 0, TYPE_bool,
	end
	);

//  Get Descriptor method for Mixin Interface
//  *****************************************
FPInterfaceDesc* INodeTransformMonitor::GetDesc()
{
	return &INodeTransformMonitor_interface;
}
// End of Function Publishing Code

