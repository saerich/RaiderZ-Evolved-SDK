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

#include "INodeMonitor.h"
#include "IIndirectRefMaker.h"

class NodeMonitor : public ReferenceTarget, public INodeMonitor, public IRefTargMonitor, public IIndirectReferenceMaker {
public:
	RefTargMonitorRefMaker *theNodeWatcher;
	BYTE loadVer;
	bool suspendDelMessagePassing;
	bool suspendSetMessagePassing;
	bool deleteMe;

	NodeMonitor();
	NodeMonitor(INode *theNode);
	~NodeMonitor();

	// INodeMonitor

	void SetNode(INode *theNode);
	INode* GetNode();

	// IRefTargMonitor

	RefResult ProcessRefTargMonitorMsg(
		Interval changeInt, 
		RefTargetHandle hTarget, 
		PartID& partID,  
		RefMessage message,
		bool fromNode);

	int ProcessEnumDependents(DependentEnumProc* dep);

	// IIndirectReferenceMaker

	int NumIndirectRefs();
	RefTargetHandle GetIndirectReference(int i);
	void SetIndirectReference(int i, RefTargetHandle rtarg);
	BOOL ShouldPersistIndirectRef(RefTargetHandle ref) { return FALSE; } // Just a watcher...

	// ReferenceMaker

	RefResult NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
		PartID& partID,  
		RefMessage message);

	RefTargetHandle Clone(RemapDir& remap);

	IOResult Load(ILoad *iload);
	IOResult Save(ISave *isave);
	int RemapRefOnLoad(int iref);

	void DeleteThis();
	void GetClassName(TSTR& s) {s=_T("NodeMonitor");} 
	Class_ID ClassID() {return NODEMONITOR_CLASS_ID;}
	SClass_ID SuperClassID() {return REF_TARGET_CLASS_ID;}

	int NumRefs();
	ReferenceTarget* GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);

	void* GetInterface(DWORD in_interfaceID)
	{
		if (IID_IINDIRECTREFERENCEMAKER == in_interfaceID) 
			return (IIndirectReferenceMaker*)this; 
		else if (IID_REFTARG_MONITOR == in_interfaceID)
			return (IRefTargMonitor*)this;
		else 
			return ReferenceTarget::GetInterface(in_interfaceID);
	}


	BaseInterface* GetInterface(Interface_ID in_interfaceID){ 
		if (in_interfaceID == IID_NODEMONITOR) 
			return (INodeMonitor*)this; 
		else 
			return ReferenceTarget::GetInterface(in_interfaceID);
	}
};
