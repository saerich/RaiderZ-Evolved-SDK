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
// DESCRIPTION: Defines a class for monitoring a RefTarg for deletion.
// AUTHOR: Larry.Minton - created April.20.2006
//***************************************************************************/

#include "IRefTargMonitor.h"
#include "IRefTargMonitorClass.h"
#include "IIndirectRefMaker.h"

class RefTargMonitorClass : public ReferenceTarget, public IRefTargMonitorClass, public IRefTargMonitor, public IIndirectReferenceMaker {
public:
	RefTargMonitorRefMaker *theRefTargWatcher;
	BYTE loadVer;
	bool suspendDelMessagePassing;
	bool suspendSetMessagePassing;
	bool deleteMe;
	bool shouldPersistIndirectRef;

	RefTargMonitorClass();
	RefTargMonitorClass(RefTargetHandle theRefTarg);
	~RefTargMonitorClass();

	// IRefTargMonitorClass

	void SetRefTarg(RefTargetHandle theRefTarg);
	RefTargetHandle GetRefTarg();
	bool GetPersist();
	void SetPersist(bool persist);

	// IRefTargMonitor

	RefResult ProcessRefTargMonitorMsg(
		Interval changeInt, 
		RefTargetHandle hTarget, 
		PartID& partID,  
		RefMessage message,
		bool fromRefTarg);

	int ProcessEnumDependents(DependentEnumProc* dep);

	// IIndirectReferenceMaker

	int NumIndirectRefs();
	RefTargetHandle GetIndirectReference(int i);
	void SetIndirectReference(int i, RefTargetHandle rtarg);
	BOOL ShouldPersistIndirectRef(RefTargetHandle ref);

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
	void GetClassName(TSTR& s) {s=_T("RefTargMonitor");} 
	Class_ID ClassID() {return REFTARGMONITOR_CLASS_ID;}
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
		if (in_interfaceID == IID_REFTARGMONITOR_CLASS) 
			return (IRefTargMonitorClass*)this; 
		else 
			return ReferenceTarget::GetInterface(in_interfaceID);
	}
};
