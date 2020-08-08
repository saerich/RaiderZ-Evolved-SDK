// Copyright 2009 Autodesk, Inc.  All rights reserved.
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law.  They may not
// be disclosed to, copied or used by any third party without the prior written
// consent of Autodesk, Inc.


#pragma once
#include "NativeInclude.h"

#pragma managed(push, on)

class SunMaster;

// This is the class used to manage the creation process of the ring array
// in the 3D viewports.
class SunMasterCreationManager : public MouseCallBack, ReferenceMaker 
{
private:
	INode *node0;
	// This holds a pointer to the SunMaster object.  This is used
	// to call methods on the sun  master such as BeginEditParams() and
	// EndEditParams() which bracket the editing of the UI parameters.
	SunMaster *theMaster;
	// This holds the interface pointer for calling methods provided
	// by MAX.
	IObjCreate *createInterface;

	ClassDesc *cDesc;
	// This holds the nodes TM relative to the CP
	Matrix3 mat;
	// This holds the initial mouse point the user clicked when
	// creating the ring array.
	IPoint2 pt0;
	// This holds the center point of the ring array
	Point3 center;
	// This flag indicates the dummy nodes have been hooked up to
	// the master node and the entire system has been created.
	BOOL attachedToNode;
	bool daylightSystem;

	// This method is called to create a new SunMaster object (theMaster)
	// and begin the editing of the systems user interface parameters.
	void CreateNewMaster(HWND rollup);

	// This flag is used to catch the reference message that is sent
	// when the system plug-in itself selects a node in the scene.
	// When the user does this, the plug-in recieves a reference message
	// that it needs to respond to.
	int ignoreSelectionChange;

	// --- Inherited virtual methods of ReferenceMaker ---
	// use named constants to keep track of references
	static const int NODE_0_REFERENCE_INDEX = 0;
	static const int SUN_MASTER_REFERENCE_INDEX = 1;
	static const int REFERENCE_COUNT = 2;

	// This returns the number of references this item has.
	virtual int NumRefs();

	// This methods retrieves the ith reference.
	virtual RefTargetHandle GetReference(int i);

	// This methods stores the ith reference.
	virtual void SetReference(int i, RefTargetHandle rtarg);

	// This method recieves the change notification messages sent
	// when the the reference target changes.
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
		PartID& partID,  RefMessage message);

public:
	// This method is called just before the creation command mode is
	// pushed on the command stack.
	void Begin( IObjCreate *ioc, ClassDesc *desc, bool daylight );
	// This method is called after the creation command mode is finished 
	// and is about to be popped from the command stack.
	void End();

	// Constructor.
	SunMasterCreationManager();
	
	// --- Inherited virtual methods from MouseCallBack
	// This is the method that handles the user / mouse interaction
	// when the system plug-in is being created in the viewports.
	int proc( HWND hwnd, int msg, int point, int flag, IPoint2 m );

#if defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
	bool IsActive() const { return theMaster != NULL; }
#endif	// defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
};

#pragma managed(pop)
