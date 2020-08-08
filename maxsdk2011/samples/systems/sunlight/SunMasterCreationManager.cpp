// Copyright 2009 Autodesk, Inc.  All rights reserved.
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law.  They may not
// be disclosed to, copied or used by any third party without the prior written
// consent of Autodesk, Inc.

#include "SunMasterCreationManager.h"
#include "sunclass.h"
#include "compass.h"
#include "natlight.h"

#pragma managed

#define SUN_RGB Point3(0.88235294f, 0.88235294f, 0.88235294f)  // 225

SunMasterCreationManager::SunMasterCreationManager() :
	node0(nullptr),
	theMaster(nullptr),
	createInterface(nullptr),
	cDesc(nullptr),
	mat(),
	pt0(),
	center(),
	attachedToNode(false),
	daylightSystem(false),
	ignoreSelectionChange(false)
{ }


// This initializes a few variables, creates a new sun master object and 
// starts the editing of the objects parameters.
void SunMasterCreationManager::Begin( IObjCreate *ioc, ClassDesc *desc, bool daylight )
{
	// This just initializes the variables.
	createInterface = ioc;
	cDesc           = desc;	 //class descriptor of the master controller
	DeleteAllRefsFromMe();
	attachedToNode = FALSE;
	daylightSystem  = daylight;
	// This creates a new sun master object and starts the editing
	// of the objects parameters.
	CreateNewMaster(NULL);
}

int SunMasterCreationManager::NumRefs()
{
	return REFERENCE_COUNT;
}

// This method sets the ith reference.  
void SunMasterCreationManager::SetReference(int i, RefTargetHandle rtarg) 
{ 
	switch(i) 
	{
	case NODE_0_REFERENCE_INDEX: 
		node0 = static_cast<INode *>(rtarg); 
		break;

	case SUN_MASTER_REFERENCE_INDEX:
		theMaster = static_cast<SunMaster *>(rtarg);
		break;

	default: 
		assert(0); 
	}
}
// This method returns the ith node.  
RefTargetHandle SunMasterCreationManager::GetReference(int i) 
{ 
	switch(i) 
	{
	case NODE_0_REFERENCE_INDEX: 
		return node0;

	case SUN_MASTER_REFERENCE_INDEX:
		return theMaster;

	default: 
		return NULL;
	}
}

//SunMasterCreationManager::~SunMasterCreationManager
void SunMasterCreationManager::End()
{
	if (NULL != theMaster) {
#ifndef NO_CREATE_TASK
		theMaster->EndEditParams( (IObjParam*)createInterface, 
			TRUE/*destroy*/, NULL );
#endif
		DeleteAllRefsFromMe();
	}
}

// This method is used to receive change notification messages from the 
// item that is referenced - the first created node in the system.
RefResult SunMasterCreationManager::NotifyRefChanged(
	Interval changeInt, 
	RefTargetHandle hTarget, 
	PartID& partID,  
	RefMessage message) 
{
	bool targetIsNode0 = (hTarget == static_cast<RefTargetHandle>(node0));
	bool observedSelectionChange = 
		(REFMSG_TARGET_SELECTIONCHANGE == message && !ignoreSelectionChange);
	bool isDeleteMessage = (REFMSG_TARGET_DELETED == message);

	if( targetIsNode0 && (observedSelectionChange || isDeleteMessage) )
	{ 
		// This method is sent if the reference target (node0) is
		// selected or deselected.
		// This is used so that if the user deselects this slave node
		// the creation process will begin with a new item (and not 
		// continue to edit the existing item if the UI controls are 
		// adjusted)

		if(NULL != node0 && !node0->Selected())
		{
			// This will set node0 to NULL
			DeleteReference(NODE_0_REFERENCE_INDEX);
		}

		if( NULL != theMaster )
		{
			// refresh the panel
			HWND temp = theMaster->hMasterParams;
#ifndef NO_CREATE_TASK
			theMaster->EndEditParams( (IObjParam*)createInterface, FALSE/*destroy*/,NULL );
#endif
			// This creates a new SunMaster object and starts
			// the parameter editing process.
			CreateNewMaster(temp);	
			// Indicate that no nodes have been attached yet - 
			// no system created...
			attachedToNode = FALSE;
		}
	}
	return REF_SUCCEED;
}

// This method is called to create a new sun master object and 
// begin the parameter editing process.
void SunMasterCreationManager::CreateNewMaster(HWND rollup)
{
	theHold.Suspend();
	ReplaceReference(SUN_MASTER_REFERENCE_INDEX, 
		new SunMaster(daylightSystem), 
		true);
	theHold.Resume();
#ifndef NO_CREATE_TASK
	//Possibly point to an existing dialog rollup
	theMaster->hMasterParams = rollup;
	// Start the edit parameters process
	theMaster->BeginEditParams( (IObjParam*)createInterface, BEGIN_EDIT_CREATE,NULL );
#endif
}


// This is the method of MouseCallBack that is used to handle the user/mouse
// interaction during the creation process of the system plug-in.
int SunMasterCreationManager::proc( 
	HWND hwnd,
	int msg,
	int point,
	int flag,
	IPoint2 m )
{
	// This is set to TRUE if we should keep processing and FALSE
	// if the message processing should end.
	int res = TRUE;
	// This is the helper object at the center of the system.
	static INode *dummyNode = NULL;
	// This is the radius of the sun system.
	float r;

	// The two objects we create here.
	//static HelperObject* compassObj = NULL;
	static CompassRoseObject* compassObj = NULL;
	static GenLight* lightObj = NULL;
	static NatLightAssembly* natLightObj = NULL;

	createInterface->GetMacroRecorder()->Disable();  // JBW, disable for now; systems not creatable in MAXScript

	// Attempt to get the viewport interface from the window
	// handle passed
	ViewExp *vpx = createInterface->GetViewport(hwnd); 
	assert( vpx );
	TimeValue t = createInterface->GetTime();
	// Set the cursor
	SetCursor(LoadCursor(MaxSDK::GetHInstance(), MAKEINTRESOURCE(IDC_CROSS_HAIR)));


	// Process the mouse message sent...
	switch ( msg ) {
		case MOUSE_POINT:
		case MOUSE_MOVE: 
		{
			switch (point) 
			{
				case 0: {
					// Mouse Down: This point defines the center of the sun system.
					pt0 = m;
					// Make sure the master object exists
					assert(theMaster);

					// Reset our pointers.
					compassObj = NULL;
					lightObj = NULL;
					natLightObj = NULL;
					dummyNode = NULL;

					mat.IdentityMatrix();
					if ( createInterface->SetActiveViewport(hwnd) ) {
						createInterface->GetMacroRecorder()->Enable();  
						return FALSE;
					}

					// Make sure the view in the viewport is not parallel
					// to the creation plane - if it is the mouse points
					// clicked by the user cannot be converted to 3D points.
					if (createInterface->IsCPEdgeOnInView()) {
						createInterface->GetMacroRecorder()->Enable();  
						return FALSE;
					}

					if ( attachedToNode) {
						// As an experiment we won't allow any other to be created.
						//createInterface->PushPrompt( MaxSDK::GetResourceString(IDS_SUN_COMPLETE_PROMPT));
						//return FALSE;

						// A previous sun system exists - terminate the editing
						// of it's parameters and send it on it's way...
						// Hang on to the last one's handle to the rollup
						HWND temp = theMaster->hMasterParams;

		#ifndef NO_CREATE_TASK
						theMaster->EndEditParams( (IObjParam*)createInterface,0,NULL );
		#endif
						// Get rid of the references.  This sets node0 = NULL.
						DeleteAllRefsFromMe();

						// This creates a new SunMaster object (theMaster)
						// and starts the parameter editing process.
						CreateNewMaster(temp);
					}

					// Begin hold for undo
					theHold.Begin();

					createInterface->RedrawViews(t, REDRAW_BEGIN);

					// Snap the inital mouse point
					center = vpx->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
					mat.SetTrans(center);

					// Create the compass (a helper).
					compassObj = (CompassRoseObject*)createInterface->
						CreateInstance(HELPER_CLASS_ID, COMPASS_CLASS_ID); 			
					assert(compassObj);
					dummyNode = createInterface->CreateObjectNode(compassObj);
					createInterface->SetNodeTMRelConstPlane(dummyNode, mat);
					if (theMaster->daylightSystem) {
						// Force the compass to be created in the XY Plane with
						// up in the +Z direction.
						TimeValue t = createInterface->GetTime();
						Matrix3 tm(1);
						tm.SetTrans(dummyNode->GetNodeTM(t).GetTrans());
						dummyNode->SetNodeTM(t, tm);
					}
					createInterface->RedrawViews(t, REDRAW_INTERACTIVE);

					res = TRUE;
					break;
				}

				case 1: {
					// Mouse Drag: Set on-screen size of the compass
					Point3 p1 = vpx->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
					compassObj->axisLength = Length(p1 - center);
					compassObj->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);

					if (msg == MOUSE_POINT) 
					{
						INode* newNode;
						if (theMaster->daylightSystem) 
						{
							// Create the NaturalLightAssembly
							newNode = NatLightAssembly::CreateAssembly(natLightObj, createInterface);
							assert(natLightObj);
							assert (newNode);

							natLightObj->SetMultController(new SlaveControl(theMaster,LIGHT_MULT));
							natLightObj->SetIntenseController(new SlaveControl(theMaster,LIGHT_INTENSE));
							natLightObj->SetSkyCondController(new SlaveControl(theMaster,LIGHT_SKY));
						} 
						else 
						{
							// Create the Light
							lightObj = 
								(GenLight *)createInterface->CreateInstance(LIGHT_CLASS_ID,Class_ID(DIR_LIGHT_CLASS_ID,0));
							assert(lightObj);
							SetUpSun(SYSTEM_CLASS_ID, SUNLIGHT_CLASS_ID, createInterface, lightObj, SUN_RGB);

							// Create a new node given the instance.
							newNode = createInterface->CreateObjectNode(lightObj);
							assert (newNode);
							createInterface->AddLightToScene(newNode);
							TSTR nodename = MaxSDK::GetResourceString(IDS_LIGHT_NAME);
							createInterface->MakeNameUnique(nodename);
							newNode->SetName(nodename);
						}

						// Create a new slave controller of the master control.
						SlaveControl* slave = new SlaveControl(theMaster,LIGHT_TM);
						// Set the transform controller used by the node.
						newNode->SetTMController(slave);
						// Attach the new node as a child of the central node.
						dummyNode->AttachChild(newNode);
						INodeMonitor* nodeMon = static_cast<INodeMonitor*>(theMaster->theObjectMon->GetInterface(IID_NODEMONITOR));
						nodeMon->SetNode(newNode);
						//theMaster->thePoint=dummyNode;
						//the master references the point so it can get
						//notified when the	user rotates it
						theMaster->ReplaceReference(REF_POINT, dummyNode );
						DbgAssert(theMaster->thePoint == dummyNode);
						// Indicate that the system is attached
						attachedToNode = TRUE;

						if (daylightSystem) {
							theMaster->ReplaceReference(REF_LOOKAT, static_cast<Control*>(
								CreateInstance(CTRL_MATRIX3_CLASS_ID, Class_ID(PRS_CONTROL_CLASS_ID,0))));
							Control* lookat = static_cast<Control*>(
								CreateInstance(CTRL_ROTATION_CLASS_ID, Class_ID(LOOKAT_CONSTRAINT_CLASS_ID, 0)));
							assert(lookat != NULL);
							Animatable* a = lookat;
							ILookAtConstRotation* ilookat = GetILookAtConstInterface(a);
							assert(ilookat != NULL);
							ilookat->AppendTarget(theMaster->thePoint);
							ilookat->SetTargetAxis(2);
							ilookat->SetTargetAxisFlip(true);
							ilookat->SetVLisAbs(false);
							theMaster->theLookat->SetRotationController(lookat);

							theMaster->ReplaceReference(REF_MULTIPLIER, static_cast<Control*>(
								CreateInstance(CTRL_FLOAT_CLASS_ID, Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID,0))));
						}

						// Reference the node so we'll get notifications.
						// This is done so when the node is selected or 
						// deselected the parameter editing process can 
						// be started and stopped.
						ReplaceReference( 0, newNode, FALSE );
						// Set the initial radius of the system to one
						theMaster->SetRad(TimeValue(0),1.0f);
						// construction plane.  This is used during creating
						// so you can set the position of the node in terms of 
						// the construction plane and not in world units.
						createInterface->SetNodeTMRelConstPlane(dummyNode, mat);
						if (theMaster->daylightSystem) 
						{
							// Force the compass to be created in the XY Plane with
							// up in the +Z direction.
							TimeValue t = createInterface->GetTime();
							Matrix3 tm(1);
							tm.SetTrans(dummyNode->GetNodeTM(t).GetTrans());
							dummyNode->SetNodeTM(t, tm);
						}
					}

					createInterface->RedrawViews(t, REDRAW_INTERACTIVE);
					// This indicates the mouse proc should continue to recieve messages.
					res = TRUE;
					break;
				}

				case 2: {
					// Mouse Pick and release: 
					// The user is dragging the mouse to set the radius
					// (i.e. the distance of the light from the compass center)
					if (node0) {
						// Calculate the radius as the distance from the initial
						// point to the current point
						r = (float)fabs(vpx->SnapLength(vpx->GetCPDisp(center,Point3(0,1,0),pt0,m)));
						// Set the radius at time 0.
						theMaster->SetRad(0, r);
						if (daylightSystem) {
							LightObject* l = natLightObj->GetSun();
							if (l != NULL) 
							{
								l->SetTDist(t, r);
							}
						} else {
							lightObj->SetTDist(t, r);
						}
						theMaster->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
		#ifndef NO_CREATE_TASK
						// Update the UI spinner to reflect the current value.
						theMaster->radSpin->SetValue(r, FALSE );
		#endif
					}

					res = TRUE;
					if (msg == MOUSE_POINT) {
						// The mouse has been released - finish the creation 
						// process.  Select the first node so if we go into 
						// the motion branch we'll see it's parameters.

						// We set the flag to ignore the selection change
						// because the NotifyRefChanged() method recieves
						// a message when the node is selected or deselected
						// and terminates the parameter editing process.  This
						// flag causes it to ignore the selection change.
						ignoreSelectionChange = TRUE;
						INodeMonitor* nodeMon = 
							static_cast<INodeMonitor*>(theMaster->theObjectMon->GetInterface(IID_NODEMONITOR));
						createInterface->SelectNode(nodeMon->GetNode());
						if (daylightSystem) {
							SunMaster::SetInCreate(false);
							theMaster->UpdateUI(t);
						}
						ignoreSelectionChange = FALSE;
						// This registers the undo object with the system so
						// the user may undo the creation.
						theHold.Accept(const_cast<TCHAR*>(MaxSDK::GetResourceString(IDS_UNDO)));
						// Set the return value to FALSE to indicate the 
						// creation process is finished.  This mouse proc will
						// no longer be called until the user creates another
						// system.
						res = FALSE;
					}
					createInterface->RedrawViews(t, res ? REDRAW_INTERACTIVE : REDRAW_END);
		#ifdef RENDER_VER
					if (FALSE == res) {
						createInterface->RemoveMode(NULL);
					}
		#endif
					break;
				}
			}
		}
		break;

		case MOUSE_PROPCLICK:
			// right click while between creations
			createInterface->RemoveMode(NULL);
			break;

		case MOUSE_ABORT:
			{
				// The user has right-clicked the mouse to abort the  creation process.
				assert(theMaster);
				HWND temp = theMaster->hMasterParams;
#ifndef NO_CREATE_TASK
				theMaster->EndEditParams( (IObjParam*)createInterface, 0,NULL );
#endif
				theHold.Cancel();  
				DeleteReference(NODE_0_REFERENCE_INDEX);

				// This creates a new SunMaster object and starts the parameter editing process.
				CreateNewMaster(temp);  

				// the viewports are now dirty, schedule a repaint to avoid leaving
				// compass artifacts behind.  (I've tried a few different methods
				// to get the screen to refresh correctly upon exit from this 
				// message processing.  This way seems to be the most reliable.)
				for(int i=0; i < GetCOREInterface9()->getNumViewports(); ++i)
				{
					GetCOREInterface9()->ViewportInvalidate(i);
				}
				createInterface->RedrawViews(t, REDRAW_NORMAL);

				// Indicate the nodes have not been hooked up to the system yet.
				attachedToNode = FALSE;
				res = FALSE;						
			}
			break;

		case MOUSE_FREEMOVE:
			{
				vpx->SnapPreview(m,m);
			}
			break;
	} // switch

	createInterface->ReleaseViewport(vpx); 
	// Returns TRUE if processing should continue and false if it
	// has been aborted.
	createInterface->GetMacroRecorder()->Enable();  
	return res;
}
