/**********************************************************************
 *<
	FILE: EditPoly.h

	DESCRIPTION: Edit Poly modifier classes

	CREATED BY: Steve Anderson

	HISTORY: created April 2004

 *>	Copyright (c) 2004 Discreet, All Rights Reserved.
 **********************************************************************/

#ifndef __EDITPOLYMOD__
#define __EDITPOLYMOD__

#include "iparamm2.h"
#include "sbmtlapi.h"
#include "istdplug.h"
#include "nsclip.h"
#include "iEPolyMod.h"
#include "stdmat.h"
#include "macroRec.h"
#include "../PolyPaint/PolyPaint.h"
#include "../../EditableModifierCommon/CreateFaceMouseProcTemplate.h"
#include "IActionItemOverrideManager.h"
#include <set>

//#define SS_PROTOTYPE

const Class_ID kEDIT_MOD_CLASS_ID(0x79aa6e1d, 0x71a075b7);
#define WM_UPDATE_CACHE		(WM_USER+0x28e)

// Get display flags based on selLevel.
const DWORD editPolyLevelToDispFlags[] = {0, MNDISP_VERTTICKS|MNDISP_SELVERTS,
		MNDISP_SELEDGES, MNDISP_SELEDGES, MNDISP_SELFACES, MNDISP_SELFACES, 0};


//preview selection levels.
#define PS_NONE_SEL		0
#define PS_SUBOBJ_SEL	1
#define PS_MULTI_SEL	2


// Named selection set levels:
#define NS_VERTEX 0
#define NS_EDGE 1
#define NS_FACE 2

// Conversion from selLevel to named selection level:
static int namedSetLevel[] = { NS_VERTEX, NS_VERTEX, NS_EDGE, NS_EDGE, NS_FACE, NS_FACE };
static int namedClipLevel[] = { CLIP_VERT, CLIP_VERT, CLIP_EDGE, CLIP_EDGE, CLIP_FACE, CLIP_FACE };

// For hit testing...
const int hitLevel[] = { 0, SUBHIT_MNVERTS, SUBHIT_MNEDGES,
				SUBHIT_MNEDGES|SUBHIT_OPENONLY,
				SUBHIT_MNFACES, SUBHIT_MNFACES, 0 };

// Flags for handling different sorts of selection within Edit Poly.
#define MN_EDITPOLY_OP_SELECT (MN_USER<<1)
#define MN_EDITPOLY_STACK_SELECT (MN_USER<<2)

#define MAX_MATID	0xffff

// Reference ID's for EPolyMod
enum EditPolyRefID {
	EDIT_PBLOCK_REF,
	EDIT_SLICEPLANE_REF,
	EDIT_MASTER_POINT_REF,
	EDIT_POINT_BASE_REF
};

enum RingLoopUpdateID {
	RING_UP,
	LOOP_UP,
	RING_DOWN,
	LOOP_DOWN,
	RING_UP_ADD,
	LOOP_UP_ADD,
	RING_DOWN_ADD,
	LOOP_DOWN_ADD,
	RING_UP_SUBTRACT,
	LOOP_UP_SUBTRACT,
	RING_DOWN_SUBTRACT,
	LOOP_DOWN_SUBTRACT
};

class EditPolyMod;
class EditPolyData;

class EditPolyCreateVertCMode;
class EditPolyCreateEdgeCMode;
class EditPolyCreateFaceCMode;
class EditPolyDivideEdgeCMode;
class EditPolyDivideFaceCMode;
class EditPolyBridgeBorderCMode;
class EditPolyBridgeEdgeCMode;
class EditPolyBridgePolygonCMode;
class EditPolyExtrudeCMode;
class EditPolyExtrudeVECMode;
class EditPolyChamferCMode;
class EditPolyBevelCMode;
class EditPolyInsetCMode;			
class EditPolyOutlineCMode;
class EditPolyCutCMode;				
class EditPolyQuickSliceCMode;		
class EditPolyWeldCMode;
class EditPolyHingeFromEdgeCMode;	
class EditPolyPickHingeCMode;
class EditPolyPickBridge1CMode;
class EditPolyPickBridge2CMode;
class EditPolyEditTriCMode;
class EditPolyTurnEdgeCMode;

class EditPolyAttachPickMode;
class EditPolyShapePickMode;

#ifdef SS_PROTOTYPE
class EditPolyEditSSMode;
#endif

// Operation-related data that's specific to one LocalModData.
// Includes vertex & face creation data, Edit Triangulation edits, Target Weld targets, etc.
class LocalPolyOpData
{
public:
	virtual int OpID ()=0;
	virtual void DeleteThis ()=0;
	virtual LocalPolyOpData *Clone ()=0;
	virtual Matrix3 *OSTransform () { return NULL; }	// If non-NULL, is updated to local mod context TM inverse.
	virtual void SetCommitted (bool committed) { }

	virtual void RescaleWorldUnits (float f) { }
	virtual IOResult Save(ISave *isave)=0;
	virtual IOResult Load(ILoad *iload)=0;
};

class PolyOperation
{
protected:
	BitArray mSelection;
	Tab<float> mSoftSelection;
	int mNewFaceCt, mNewVertCt, mNewEdgeCt;
	Tab<int> mNewMapVertCt;

public:
	PolyOperation () : mNewFaceCt(0), mNewVertCt(0), mNewEdgeCt(0) { }

	void RecordSelection (MNMesh & mesh, EditPolyMod *pMod, EditPolyData *pData);
	virtual void SetUserFlags (MNMesh & mesh);
	void CopyBasics (PolyOperation *pCopyTo);
	IOResult SaveBasics (ISave *isave);
	IOResult LoadBasics (ILoad *iload);
	void MacroRecordBasics ();
	bool Apply (MNMesh & mesh, LocalPolyOpData *pOpData) {
		int onumv = mesh.numv;
		int onumf = mesh.numf;
		int onume = mesh.nume;
		mNewMapVertCt.SetCount (mesh.numm + NUM_HIDDENMAPS);
		for (int mp=-NUM_HIDDENMAPS; mp<mesh.numm; mp++) {
			int nmp = mp + NUM_HIDDENMAPS;
			if (mesh.M(mp)->GetFlag (MN_DEAD)) 
				mNewMapVertCt[nmp] = 0;
			else
				mNewMapVertCt[nmp] = -mesh.M(mp)->numv;
		}
		bool ret = Do (mesh, pOpData);
		mNewFaceCt = mesh.numf - onumf;
		mNewVertCt = mesh.numv - onumv;
		mNewEdgeCt = mesh.nume - onume;
		for (int mp=-NUM_HIDDENMAPS; mp<mesh.numm; mp++) {
			if (mesh.M(mp)->GetFlag (MN_DEAD)) continue;
			int nmp = mp + NUM_HIDDENMAPS;
			mNewMapVertCt[nmp] += mesh.M(mp)->numv;
			if (mNewMapVertCt[nmp]<0) mNewMapVertCt[nmp] = 0;
		}
		return ret;
	}

	int NewFaceCount () const { return mNewFaceCt; }
	int NewEdgeCount () const { return mNewEdgeCt; }
	int NewVertexCount () const { return mNewVertCt; }
	int NewMapVertexCount (int mapChannel) const {
		int nchan = NUM_HIDDENMAPS + mapChannel;
		if (nchan >= mNewMapVertCt.Count()) return 0;
		return mNewMapVertCt[nchan];
	}

	virtual int OpID () { return ep_op_null; }
	// Which selection levels in EPoly is this operation applicable to?
	virtual bool SelLevelSupport (int selLevel) { return true; }
	// The selection level we should set MN_USER flags on to indicate which components to affect:
	// (return MNM_SL_OBJECT if you don't require a selection.)
	virtual int MeshSelLevel () { return MNM_SL_OBJECT; }

	virtual TCHAR *Name () { return GetString (IDS_EDITPOLY_CHOOSE_OPERATION); }
	virtual int DialogID () { return 0; }

	// These Parameter methods make it easy to manage the subset of parameters
	// that are significant to this operation.  Parameters supported by these methods
	// are automatically copied, saved, and loaded in CopyBasics, SaveBasics, and LoadBasics,
	// and are filled in with their correct values in the default implementation of GetValues.
	// They only support float and int parameters presently.
	// Simple PolyOperations can use these to avoid having to implement Save, Load, and GetValues.
	virtual void GetParameters (Tab<int> & paramList) { }
	virtual void *Parameter (int paramID) { return NULL; }
	virtual int ParameterSize (int paramID) { return sizeof(float); }	// note - also = sizeof(int).
	virtual int TransformReference () { return 0; }	// indicates no transform reference.
	virtual Matrix3 *Transform () { return NULL; } // pointer to the transform.
	virtual Matrix3 *ModContextTM () { return NULL; }	// If the operation depends on object space, and is shared across multiple ModContexts, it should return a pointer here to get the MC->OS transform.
	virtual bool UseSoftSelection () { return false; }
	virtual MapBitArray *PreserveMapSettings () { return NULL; }	// if the operation depends on preserve map settings, return a pointer to a MapBitArray here.
	virtual bool CanDelete () { return false; }	// return true if the operation can delete existing geometry.
	virtual bool CanAnimate () { return true; } // return false if the operation cannot be animated in any sensible way.
	virtual bool ChangesSelection () { return false; } // return true if the selection should be updated to the MN_USER flagged subobjects after performing this operation.
	virtual void RescaleWorldUnits (float f) { }
	virtual void Reset () { }	// Resets any local data in static PolyOps after they've been committed or reset.
	virtual void MacroRecord (LocalPolyOpData *pOpData);

	virtual void GetValues (EPolyMod *epoly, TimeValue t, Interval & ivalid);
	// For operations that require information from a node parameter, such as
	// extrude along spline, the following methods should be used to get node data from
	// the param block, and to clear any cached data that may no longer be valid.
	// The paramID is given in ClearNode, so you don't wind up clearing your node-based data when
	// some other node has changed.  (-1 indicates "clear all node-based data", otherwise look for
	// epm_spline_node, etc.)
	virtual void GetNode (EPolyMod *epoly, TimeValue t, Interval & ivalid) { }
	virtual void ClearNode (int paramID) { }
	virtual PolyOperation *Clone () {
		PolyOperation *ret = new PolyOperation();
		CopyBasics (ret);
		return ret;
	}
	virtual IOResult Save (ISave *isave);
	virtual IOResult Load (ILoad *iload);
	virtual void DeleteThis () { delete this; }

protected:
	virtual bool Do (MNMesh & mesh, LocalPolyOpData *pOpData) { return false; }
};

class PolyOperationRecord
{
private:
	PolyOperationRecord *mpNext;
	PolyOperation *mpOp;
	LocalPolyOpData *mpData;

public:
	PolyOperationRecord (PolyOperation *pOp) : mpOp(pOp), mpData(NULL), mpNext(NULL) { }
	PolyOperationRecord (PolyOperation *pOp, LocalPolyOpData *pData) : mpOp(pOp),
		mpData(pData), mpNext(NULL) { }

	~PolyOperationRecord () { if (mpOp) mpOp->DeleteThis(); if (mpData) mpData->DeleteThis(); }

	PolyOperationRecord *Next () const { return mpNext; }
	void SetNext (PolyOperationRecord *pNext) { mpNext = pNext; }
	LocalPolyOpData *LocalData () const { return mpData; }
	void SetLocalData (LocalPolyOpData *pData) { mpData = pData; }
	PolyOperation *Operation () const { return mpOp; }
	void SetOperation (PolyOperation *pOp) { mpOp = pOp; }

	PolyOperationRecord *Clone ();
	void DeleteThis() { delete this; }
};

const ActionTableId kEditPolyActionID = 0x4a8e46da;

class EditPolyMod;
class EditPolyActionCB : public ActionCallback {
	EditPolyMod *mpMod; //changed from EPolyMode to EditPolyMode so we can call the unexposed functions like the paint deform stuff

public:
	EditPolyActionCB (EditPolyMod *m) : mpMod(m) { }
	BOOL ExecuteAction(int id);
};

class EditPolyMod : public Modifier, public IMeshSelect, public EPolyMod,
	public EventUser, public AttachMatDlgUser, public MeshPaintHandler {
friend class EditPolySelectRestore;
friend class EditPolyOperationProc;
friend class EditPolySlicePlaneRestore;
friend class EditPolyBackspaceUser;

	// Our references:
	// We always need a Parameter Block:
	IParamBlock2 *mpParams;
	
	// And we need a TM controller for subobject transforms or for the Slice Plane:
	Control *mpSliceControl;

	// Point animation controllers (for use with transform operation)
	MasterPointControl	*mpPointMaster;		// Master track controller - includes all nodes' points wrapped into one.
	Tab<Control*>		mPointControl;

	// These tables describe the names of the nodes associated with each block of points in the master point controller.
	// mPointControl[mPointRange[j]] through mPointControl[mPointRange[j+1]]-1 are the controllers for vertices
	// in the node named mPointNode[j].
	Tab<int>	mPointRange;
	MaxSDK::Array<TSTR>	mPointNode;

	
	DWORD		selLevel;				// Selection level - this local data is saved/loaded.
	MapBitArray mPreserveMapSettings;
	int			mLastOperation;			// Repeat-last data:

	static EditPolyActionCB		*mpEPolyActions;			// Static keyboard shortcut table:
	static Tab<PolyOperation*>	mOpList;					// Static list of available operations:
	static void					InitializeOperationList ();
	static EditPolyMod			*mpCurrentEditMod;			// Static pointer to the modifier instance that's currently 
															// being edited (if any):

	static Tab<ActionItem *> mOverrides;			//static list of our override actions...

	PolyOperation *mpCurrentOperation;						// Current operation for this instance:

	//Last constrainType set.. used for toggles and actions..
	int mLastConstrainType;

	// Standard command modes:
	static SelectModBoxCMode			*selectMode;
	static MoveModBoxCMode				*moveMode;
	static RotateModBoxCMode			*rotMode;
	static UScaleModBoxCMode			*uscaleMode;
	static NUScaleModBoxCMode			*nuscaleMode;
	static SquashModBoxCMode			*squashMode;

	// Our command modes:
	static EditPolyCreateVertCMode		*createVertMode;
	static EditPolyCreateEdgeCMode		*createEdgeMode;
	static EditPolyCreateFaceCMode		*createFaceMode;
	static EditPolyAttachPickMode		*attachPickMode;
	static EditPolyDivideEdgeCMode		*divideEdgeMode;
	static EditPolyDivideFaceCMode		*divideFaceMode;
	static EditPolyBridgeBorderCMode	*bridgeBorderMode;
	static EditPolyBridgeEdgeCMode		*bridgeEdgeMode;
	static EditPolyBridgePolygonCMode	*bridgePolyMode;
	static EditPolyExtrudeCMode			*extrudeMode;
	static EditPolyExtrudeVECMode		*extrudeVEMode;
	static EditPolyChamferCMode			*chamferMode;
	static EditPolyBevelCMode			*bevelMode;
	static EditPolyInsetCMode			*insetMode;
	static EditPolyOutlineCMode			*outlineMode;
	static EditPolyCutCMode				*cutMode;
	static EditPolyQuickSliceCMode		*quickSliceMode;
	static EditPolyWeldCMode			*weldMode;
	static EditPolyHingeFromEdgeCMode	*hingeFromEdgeMode;
	static EditPolyPickHingeCMode		*pickHingeMode;
	static EditPolyPickBridge1CMode		*pickBridgeTarget1;
	static EditPolyPickBridge2CMode		*pickBridgeTarget2;
	static EditPolyEditTriCMode			*editTriMode;
	static EditPolyTurnEdgeCMode		*turnEdgeMode;
#ifdef SS_PROTOTYPE
	static EditPolyEditSSMode			*editSSMode;
#endif

	
	static EditPolyShapePickMode		*mpShapePicker;	// Pick mode for getting spline for extrude along spline:
	static EditPolyBackspaceUser		backspacer;
	
	static bool		mUpdateCachePosted;
	static DWORD	mHitLevelOverride;
	static DWORD	mDispLevelOverride;
	static bool		mForceIgnoreBackfacing;
	static bool		mSuspendConstraints;
	static bool		mHitTestResult;
	static bool		mIgnoreNewInResult;
	static bool		mDisplayResult;
	static bool		mSliceMode;				// Pseudo-command-mode for slice plane
	static int		mLastCutEnd;			// Necessary in order to support sequential cuts:

	// Attach, Detach, Create Shape options
	static int 		mAttachType;
	static int 		mCreateShapeType;
	static int 		mCloneTo;

	static bool 	mAttachCondense;
	static bool 	mDetachToObject;
	static bool 	mDetachAsClone;
	static TSTR		mCreateShapeName ;
	static TSTR		mDetachName;

	// UI for the operation-specific dialog proc:
	static IParamMap2 	*mpDialogOperation;
	static IParamMap2 	*mpDialogAnimate;
	static IParamMap2 	*mpDialogSelect;

	static IParamMap2 	*mpDialogSoftsel;
	static IParamMap2 	*mpDialogGeom;
	static IParamMap2 	*mpDialogSubobj;
	static IParamMap2 	*mpDialogSurface;
	static IParamMap2 	*mpDialogFaceSmooth;
	static IParamMap2 	*mpDialogPaintDeform;

	static bool 		mRollupSelect;
	static bool 		mRollupSoftsel;
	static bool 		mRollupGeom;
	static bool 		mRollupSubobj ;
	static bool 		mRollupAnimate ;
	static bool 		mRollupSurface ;
	static bool 		mRollupFaceSmooth ;
	static bool 		mRollupPaintDeform;

	// Named Selection data
	Tab<TSTR*> namedSel[3];		
	Tab<DWORD> ids[3];

	DWORD mCurrentDisplayFlags;

	RefmsgKillCounter	killRefmsg;

	bool GetCreateShape ();
	bool GetDetachObject ();
	bool GetDetachClonedObject ();

	bool	m_Transforming; 

	// ctrl and alt key 
	bool	mCtrlKey;
	bool	mAltKey;

	int		mRingSpinnerValue; 
	int		mLoopSpinnerValue;

	int mSelLevelOverride;//override of the selection if not -1 then we use that as the selection intuitive sub object modeling

	public:
	// Interface pointer:
	static IObjParam *ip;

	// cage colors 
	Point3			m_CageColor;
	Point3			m_SelectedCageColor;
	IColorSwatch*	m_iColorCageSwatch;
	IColorSwatch*	m_iColorSelectedCageSwatch;



	EditPolyMod();

	// From Animatable
	void				DeleteThis() { delete this; }
	void				GetClassName(TSTR& s) {s = GetString(IDS_EDIT_POLY_MOD);}  
	virtual Class_ID	ClassID () { return kEDIT_MOD_CLASS_ID;}
	void				ResetClassParams (BOOL fileReset);
	void				SavePreferences ();
	void				LoadPreferences ();

	RefTargetHandle		Clone (RemapDir& remap = DefaultRemapDir());
	TCHAR				*GetObjectName() { return GetString(IDS_EDIT_POLY_MOD); }
	void				*GetInterface(ULONG id) { if (id == I_MESHSELECT) return (IMeshSelect *)this; else return Modifier::GetInterface(id); }

	void 				BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev);
	void 				EndEditParams (IObjParam *ip,ULONG flags,Animatable *next);		

	// From modifier. We can change or depend on pretty much anything.
	ChannelMask			ChannelsUsed()  {return PART_GEOM|PART_TOPO|PART_SELECT|PART_TEXMAP|PART_VERTCOLOR; }
	ChannelMask			ChannelsChanged() {return PART_GEOM|PART_TOPO|PART_SELECT|PART_TEXMAP|PART_SUBSEL_TYPE|PART_VERTCOLOR; }
	Class_ID			InputType() { return polyObjectClassID; }
	void				ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	float				*SoftSelection (TimeValue t, EditPolyData *pData, Interval & validity);
	Interval 			LocalValidity(TimeValue t);
	Interval 			GetValidity (TimeValue t);
	BOOL				DependOnTopology(ModContext &mc);

	// From BaseObject
	CreateMouseCallBack	*GetCreateMouseCallBack() {return NULL;}
	MNMesh				*GetMeshToHitTest (EditPolyData *pData);
	int					HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
	void				UpdateDisplayFlags ();
	MNMesh				*GetMeshToDisplay (EditPolyData *pData);
	int					Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc);
	void 				GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);
	void 				GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
	void 				GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);

	// Slice subobject display, hit-testing:
	void 	UpdateSliceUI ();
	void 	EnterSliceMode ();
	void 	ExitSliceMode ();
	void 	GetSlicePlaneBoundingBox (TimeValue t, INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);
	void 	DisplaySlicePlane (TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc);
	int		HitTestSlice (TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
	Matrix3 CompSliceMatrix(TimeValue t,INode *inode,ModContext *mc);

	bool ShowGizmoConditions ();	// Indicates if we can currently show a gizmo:
	bool ShowGizmo ();				// Indicates if we are currently showing a gizmo:


	// ParamBlock2 access:
	int				NumParamBlocks () { return 1; }
	IParamBlock2	*GetParamBlock(int i) { return mpParams; }
	IParamBlock2	*GetParamBlockByID(BlockID id) { return (mpParams->ID() == id) ? mpParams : NULL; }

	// Reference Management:
	int				NumRefs() {return EDIT_POINT_BASE_REF + mPointControl.Count();}
	RefTargetHandle GetReference(int i);
	void			SetReference(int i, RefTargetHandle rtarg);
	RefResult		NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, PartID& partID, RefMessage message);

	// Animatable management:
	int			NumSubs() { return EDIT_POINT_BASE_REF; }	// leave off the individual point controllers.
	Animatable	*SubAnim(int i) { return (Animatable *)GetReference(i); }
	TSTR		SubAnimName(int i);
	BOOL		AssignController(Animatable *control,int subAnim);
	int			SubNumToRefNum(int subNum) {return subNum;}
	BOOL		SelectSubAnim(int subNum);

	// Point controller methods (for use with transform operation):
	void CreatePointControllers ();
	void AllocPointControllers (int count);

	//void ReplacePointControllers (Tab<Control *> &nc);
	void DeletePointControllers ();	// deletes all of them - appropriate when committing/cancelling Transform.
	void PlugControllers (TimeValue t, BitArray &set, INode *pNode);
	void PlugControllers (TimeValue t, Point3 *pOffsets, int numOffsets, INode *pNode);
	BOOL PlugController (TimeValue t, int vertex, int nodeStart, INode *pNode);
	void UpdateTransformData (TimeValue t, Interval & valid, EditPolyData *pData);

	IParamBlock2 *getParamBlock () { return mpParams; }
	MapBitArray GetPreserveMapSettings () const { return mPreserveMapSettings; }
	void		SetPreserveMapSettings (const MapBitArray & mapSettings);
	
	// Scriptable versions:
	void EpModSetPreserveMap (int mapChannel, bool preserve);
	bool EpModGetPreserveMap (int mapChannel) { return mPreserveMapSettings[mapChannel]; }

	// Subobject API
	int			NumSubObjTypes();
	ISubObjType *GetSubObjType(int i);

	// Selection stuff from BaseObject:
	void ActivateSubobjSel(int level, XFormModes& modes);
	void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE);
	void ClearSelection(int selLevel);
	void SelectAll(int selLevel);
	void InvertSelection(int selLevel);

	// From IMeshSelect
	DWORD	GetSelLevel();
	void	SetSelLevel(DWORD level);
	void	LocalDataChanged();
	int	GetCalcMeshSelLevel(int); //new for mz prototype we may have verts or edges selected at the element level


	// Access to EPolyMod interface:
	BaseInterface *GetInterface (Interface_ID id);

	// Selection stuff from EPolyMod:
	int 	GetEPolySelLevel() { return selLevel; }
	int 	GetMNSelLevel() { return meshSelLevel[selLevel]; }
	void	SetEPolySelLevel (int sl) { if (ip) ip->SetSubObjectLevel (sl); else selLevel=sl; }
	int		EpModConvertSelection (int epSelLevelFrom, int epSelLevelTo, bool requireAll);
	int		EpModConvertSelectionToBorder (int epSelLevelFrom, int epSelLevelTo);

	// Selection related stuff:
	void UpdateSelLevelDisplay ();
	void SetEnableStates();
	void SetNumSelLabel();		
	void UpdateCageCheckboxEnable ();
	void UpdateCache(TimeValue t);
	bool IsVertexSelected (EditPolyData *pData, int vertexID);
	bool IsEdgeSelected (EditPolyData *pData, int edgeID);
	bool IsFaceSelected (EditPolyData *pData, int faceID);
	void GetVertexTempSelection (EditPolyData *pData, BitArray & vertexTempSel);

	void NotifyInputChanged (Interval changeInt, PartID partID, RefMessage message, ModContext *mc);

	// IO
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
	IOResult SaveLocalData(ISave *isave, LocalModData *ld);
	IOResult LoadLocalData(ILoad *iload, LocalModData **pld);

	void InvalidateDistanceCache ();
	void InvalidateSoftSelectionCache ();
	void RescaleWorldUnits (float f);

	void UpdateOperationDialogParams ();
	bool EpModShowOperationDialog ();
	void EpModCloseOperationDialog ();
	bool EpModShowingOperationDialog ();

	void UpdateOpChoice ();
	void UpdateOperationDialog ();
	void UpdateEnables (int paramPanelID);
	void SetSubobjectLevel(int level);
	void UpdateUIToSelectionLevel (int oldSelLevel);
	HWND GetDlgHandle(int paramPanelID);
	void InvalidateDialogElement (int elem);
	void InvalidateNumberSelected ();
	void InvalidateSurfaceUI();
	void InvalidateFaceSmoothUI();

	void ExitAllCommandModes (bool exSlice=true, bool exStandardModes=true);

	void UpdateAlignParameters (TimeValue t);

	void EpModCommitUnlessAnimating (TimeValue t);
	void EpModCommit (TimeValue t);
	void EpModCommit (TimeValue t, bool macroRecord, bool clearAfter=true, bool undoable=true);
	void EpModCommitAndRepeat (TimeValue t);
	void EpModCancel ();
	void ResetOperationParams ();

	int						GetNumOperations () { return mOpList.Count (); }
	int						GetPolyOperationID ();
	PolyOperation			*GetPolyOperation ();
	static PolyOperation	*GetPolyOperationByIndex (int i) { return mOpList[i]; }
	static PolyOperation	*GetPolyOperationByID (int opID);

	// EventUser methods:
	void Notify() { /*delete key was pressed*/
		switch (meshSelLevel[selLevel]) {
			case MNM_SL_VERTEX:
				EpModButtonOp (ep_op_delete_vertex);
				break;
			case MNM_SL_EDGE:
				EpModButtonOp (ep_op_delete_edge);
				break;
			case MNM_SL_FACE:
				EpModButtonOp (ep_op_delete_face);
				break;
		}
	}

	// from AttachMatDlgUser
	int		GetAttachMat() { return mAttachType; }
	void	SetAttachMat(int value) { mAttachType = value; }
	BOOL	GetCondenseMat() { return mAttachCondense; }
	void	SetCondenseMat(BOOL sw) { mAttachCondense = (sw!=0); }

	// Shift-cloning:Operations -- in polyedops.cpp
	void CloneSelSubComponents(TimeValue t);
	void AcceptCloneSelSubComponents(TimeValue t);
	
	// Transform stuff:
	void Move(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE);
	void Rotate(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin=FALSE);
	void Scale(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE);
	void Transform (TimeValue t, Matrix3& partm, Matrix3 tmAxis, BOOL localOrigin, Matrix3 xfrm, int type);
	void TransformStart (TimeValue t);
	void TransformHoldingFinish (TimeValue t);
	void TransformFinish (TimeValue t);
	void TransformCancel (TimeValue t);

	void EpModMoveSelection (Point3 &val, Matrix3 &partm, Matrix3 &tmAxis, BOOL localOrigin, TimeValue t);
	void EpModRotateSelection (Quat &val, Matrix3 &partm, Matrix3 &tmAxis, BOOL localOrigin, TimeValue t);
	void EpModScaleSelection (Point3 &val, Matrix3 &partm, Matrix3 &tmAxis, BOOL localOrigin, TimeValue t);

	void EpModMoveSlicePlane (Point3 &val, Matrix3 &partm, Matrix3 &tmAxis, TimeValue t);
	void EpModRotateSlicePlane (Quat &val, Matrix3 &partm, Matrix3 &tmAxis, BOOL localOrigin, TimeValue t);
	void EpModScaleSlicePlane (Point3 &val, Matrix3 &partm, Matrix3 &tmAxis, BOOL localOrigin, TimeValue t);

	// Slice plane accessors:
	void	EpResetSlicePlane ();
	Matrix3 EpGetSlicePlaneTM (TimeValue t);
	void 	EpGetSlicePlane (Point3 & planeNormal, Point3 & planeCenter, TimeValue t);
	void 	EpSetSlicePlane (Point3 & planeNormal, Point3 & planeCenter, TimeValue t);
	bool 	EpInSliceMode () { return mSliceMode && EpInSlice(); }
	bool 	EpInSlice ();
	bool 	InSlicePreview ();

	void 	SetOperation (int opcode, bool autoCommit, bool macroRecord=true);
	void 	EpModSetOperation (int opcode) { SetOperation (opcode, false); }
	void 	EpModButtonOp (int opcode) { SetOperation (opcode, true); }
	void 	EpModPopupDialog (int opcode);
	void 	EpModLocalDataChanged (DWORD parts);
	void 	EpModRefreshScreen ();
	void 	EpModAboutToChangeSelection ();

	int		EpGetLastOperation () { return mLastOperation; }
	void	EpSetLastOperation (int op);
	void	EpModRepeatLast ();

	void ToggleShadedFaces ();

	// EditPolyModes stuff:
	CommandMode *getCommandMode (int mode);
	void 		EpModToggleCommandMode (int mode);
	void 		EpModEnterCommandMode (int mode);
	void 		EpModEnterPickMode (int mode);
	int			EpModGetCommandMode ();
	int			EpModGetPickMode ();

	// Selection access.
	BitArray	*EpModGetSelection (int meshSelLevel, INode *pNode=NULL);
	bool		EpModSetSelection (int meshSelLevel, BitArray & selection, INode *pNode=NULL);
	bool		EpModSelect (int meshSelLevel, BitArray & selection, bool invert=false, bool select=true, INode *pNode=NULL);
	
	void		UpdateLoopEdgeSelection(const int in_spinVal );
	void		UpdateRingEdgeSelection(const int in_spinVal );

	void		UpdateCageColor();			
	void		UpdateSelectedCageColor();

	// Used in command modes to change which level we're hit testing on.
	void 		SetHitLevelOverride (DWORD hlo) { mHitLevelOverride = hlo; }
	void 		ClearHitLevelOverride () { mHitLevelOverride = 0x0; }
	DWORD		GetHitLevelOverride () { return mHitLevelOverride; }
	DWORD		CurrentHitLevel (int *selByVert=NULL);

	void SetHitTestResult (bool ignoreNew=false) {
		mHitTestResult = true;
		mIgnoreNewInResult = ignoreNew;
	}

	void ClearHitTestResult () {
		mHitTestResult = false;
	}
	void SetDisplayResult () { mDisplayResult = true; }
	void ClearDisplayResult () { mDisplayResult = false; }

	void	SetDisplayLevelOverride (DWORD dlo) { mDispLevelOverride = dlo; UpdateDisplayFlags (); }
	void	ClearDisplayLevelOverride () { mDispLevelOverride = 0x0; UpdateDisplayFlags (); }
	DWORD	GetDisplayLevelOverride () { return mDispLevelOverride; }

	// Also used by command modes where needed:
	void	ForceIgnoreBackfacing (bool force) { mForceIgnoreBackfacing = force; }
	bool	GetForceIgnoreBackfacing () { return mForceIgnoreBackfacing; }

	// In EditPolyData.cpp, related closely to EditPolyData methods:
	INode			*EpModGetPrimaryNode ();
	LocalModData	*GetPrimaryLocalModData ();
	Matrix3			EpModGetNodeTM (TimeValue t, INode *node=NULL);
	EditPolyData	*GetEditPolyDataForNode (INode *node);

	void EpModSetPrimaryNode (INode *node);

	int 			EpModCreateVertex (Point3 p, INode *pNode=NULL);
	int 			EpModCreateFace (Tab<int> *vertex, INode *pNode=NULL);
	int 			EpModCreateEdge (int v1, int v2, INode *pNode=NULL);
	void 			EpModSetDiagonal (int v1, int v2, INode *pNode=NULL);
	void 			EpModCut (int startLevel, int startIndex, Point3 startPoint, Point3 normal, INode *pNode=NULL);
	void 			EpModSetCutEnd (Point3 endPoint, INode *pNode=NULL);
	int				EpModGetLastCutEnd () { return mLastCutEnd; }
	virtual void	EpModClearLastCutEnd () { mLastCutEnd = -1; }
	void 			EpModDivideEdge (int edge, float prop, INode *pNode=NULL);
	void 			EpModDivideFace (int face, Tab<float> *bary, INode *pNode=NULL);
	void 			EpModWeldVerts (int v1, int v2, INode *pNode=NULL);
	void 			EpModWeldEdges (int e1, int e2, INode *pNode=NULL);

	void			EpModSetHingeEdge (int edge, Matrix3 modContextTM, INode *pNode=NULL);
	int				EpModGetHingeEdge (INode *pNode);

	void 			EpModBridgeBorders (int edge1, int edge2, INode *pNode=NULL);
	void 			EpModBridgePolygons (int face1, int face2, INode *pNode=NULL);
	void 			EpModSetBridgeNode (INode *pNode);
	INode			*EpModGetBridgeNode ();
	bool			EpModReadyToBridgeSelected ();
	void			EpModBridgeEdges (const int edge1, const int edge2, INode *pNode=NULL);
	void 			EpModDragInit (INode *pNode, TimeValue t);
	void 			EpModDragMove (Tab<Point3> & offset, INode *pNode, TimeValue t);
	void 			EpModDragCancel (INode *pNode);
	void 			EpModDragAccept (INode *pNode, TimeValue t);
	void			EpModTurnDiagonal (int face, int diag, INode *pNode=NULL);
	void 			EpModAttach (INode *node, INode *pNode=NULL, TimeValue t=0);
	void 			EpModMultiAttach (Tab<INode *> &nodeTab, INode *pNode=NULL, TimeValue t=0);
	void 			EpModDetachToObject (TSTR & newObjectName, TimeValue t);
	void 			EpModCreateShape (TSTR & shapeObjectName, TimeValue t);
	void			EpModListOperations (INode *pNode=NULL);
	void			EpModUpdateRingEdgeSelection ( int in_val, INode *pNode = NULL );
	void			EpModUpdateLoopEdgeSelection ( int in_val, INode *pNode = NULL );
	void			EpModSetRingShift( int in_newPos = 0 , bool in_MoveOnly = true, bool in_Add = false);
	void			EpModSetLoopShift( int in_newPos = 0 , bool in_MoveOnly = true, bool in_Add = false);


	MNMesh			*EpModGetMesh (INode *pNode=NULL);
	MNMesh			*EpModGetOutputMesh (INode *pNode=NULL);

	Modifier		*GetModifier () { return this; }
	IObjParam		*EpModGetIP() { return ip; }

	// Access to information about each mesh.
	// (This is always based on the mesh _before_ any current operation.)
	int		EpMeshGetNumVertices (INode *pNode=NULL);
	Point3	EpMeshGetVertex (int vertIndex, INode *pNode=NULL);
	int 	EpMeshGetVertexFaceCount (int vertIndex, INode *pNode=NULL);
	int 	EpMeshGetVertexFace (int vertIndex, int whichFace, INode *pNode=NULL);
	int 	EpMeshGetVertexEdgeCount (int vertIndex, INode *pNode=NULL);
	int 	EpMeshGetVertexEdge (int vertIndex, int whichEdge, INode *pNode=NULL);
	int 	EpMeshGetNumEdges (INode *pNode=NULL);
	int 	EpMeshGetEdgeVertex (int edgeIndex, int end, INode *pNode=NULL);
	int 	EpMeshGetEdgeFace (int edgeIndex, int side, INode *pNode=NULL);

	int 	EpMeshGetNumFaces(INode *pNode=NULL);
	int 	EpMeshGetFaceDegree (int faceIndex, INode *pNode=NULL);
	int 	EpMeshGetFaceVertex (int faceIndex, int corner, INode *pNode=NULL);
	int 	EpMeshGetFaceEdge (int faceIndex, int side, INode *pNode=NULL);
	int 	EpMeshGetFaceDiagonal (int faceIndex, int diagonal, int end, INode *pNode=NULL);
	int 	EpMeshGetFaceMaterial (int faceIndex, INode *pNode=NULL);
	DWORD	EpMeshGetFaceSmoothingGroup (int faceIndex, INode *pNode=NULL);

	int		EpMeshGetNumMapChannels (INode *pNode=NULL);
	bool	EpMeshGetMapChannelActive (int mapChannel, INode *pNode=NULL);
	int		EpMeshGetNumMapVertices (int mapChannel, INode *pNode=NULL);
	UVVert	EpMeshGetMapVertex (int mapChannel, int vertIndex, INode *pNode=NULL);
	int		EpMeshGetMapFaceVertex (int mapChannel, int faceIndex, int corner, INode *pNode=NULL);

	//new methods in Max 9 - adding functionality that previously only existed in editable poly
	//Most of the following get methods should be const, but it is not possible since they call non-const methods
	//Modifying the code to utilize const would end up affecting this entire file and possibly many others
	Point3		EPMeshGetFaceNormal( const int in_faceIndex, INode *in_pNode = NULL );
	Point3		EPMeshGetFaceCenter( const int in_faceIndex, INode *in_pNode = NULL );
	float		EPMeshGetFaceArea( const int in_faceIndex, INode *in_pNode = NULL );
	BitArray *	EPMeshGetOpenEdges( INode *in_pNode = NULL );
	bool		EPMeshGetVertsByFlag( BitArray & out_vset, const DWORD in_flags, const DWORD in_fmask=0x0, INode *in_pNode = NULL );
	bool		EPMeshGetEdgesByFlag( BitArray & out_eset, const DWORD in_flags, const DWORD in_fmask=0x0, INode *in_pNode = NULL );
	bool		EPMeshGetFacesByFlag( BitArray & out_fset, const DWORD in_flags, const DWORD in_fmask=0x0, INode *in_pNode = NULL );
	void		EPMeshSetVertexFlags( BitArray & in_vset, const DWORD in_flags, DWORD in_fmask=0x0, const bool in_undoable = true, INode *in_pNode = NULL );
	void		EPMeshSetEdgeFlags( BitArray & in_eset, const DWORD in_flags, DWORD in_fmask=0x0, const bool in_undoable = true, INode *in_pNode = NULL );
	void		EPMeshSetFaceFlags( BitArray & in_fset, const DWORD in_flags, DWORD in_fmask=0x0, const bool in_undoable = true, INode *in_pNode = NULL );
	int			EPMeshGetVertexFlags( const int in_vertexIndex, INode *in_pNode = NULL );
	int			EPMeshGetEdgeFlags( const int in_edgeIndex, INode *in_pNode = NULL );
	int			EPMeshGetFaceFlags( const int in_faceIndex, INode *in_pNode = NULL );
	void		EPMeshGetVertsUsingEdge( BitArray & out_vset, const BitArray & in_eset, INode *in_pNode = NULL );
	void		EPMeshGetEdgesUsingVert( BitArray & out_eset, BitArray & in_vset, INode *in_pNode = NULL );
	void		EPMeshGetFacesUsingEdge( BitArray & out_fset, BitArray & in_eset, INode *in_pNode = NULL );
	void		EPMeshGetElementsUsingFace( BitArray & out_eset, BitArray & in_fset, BitArray & in_fenceSet, INode *in_pNode = NULL );
	void		EPMeshGetFacesUsingVert( BitArray & out_fset, BitArray & in_vset, INode *in_pNode = NULL );
	void		EPMeshGetVertsUsingFace( BitArray & out_vset, BitArray & in_fset, INode *in_pNode = NULL );
	void		EPMeshSetVert( const BitArray & in_vset, const Point3 & in_point, INode *in_pNode = NULL );

	// Named subobject selections:
	BOOL 	SupportsNamedSubSels() {return TRUE;}
	void 	ActivateSubSelSet(TSTR &setName);
	void 	NewSetFromCurSel(TSTR &setName);
	void 	RemoveSubSelSet(TSTR &setName);
	void 	SetupNamedSelDropDown();
	int		NumNamedSelSets();
	TSTR 	GetNamedSelSetName(int i);
	void 	SetNamedSelSetName(int i,TSTR &newName);
	void 	NewSetByOperator(TSTR &newName,Tab<int> &sets,int op);

	// Local methods for handling named selection sets
	int		FindSet(TSTR &setName,int level);
	DWORD	AddSet(TSTR &setName,int level);
	void 	RemoveSet(TSTR &setName,int level);
	void 	UpdateSetNames ();	// Reconciles names with PolyMeshSelData.
	void 	ClearSetNames();
	void 	NSCopy();
	void 	NSPaste();
	BOOL 	GetUniqueSetName(TSTR &name);
	int		SelectNamedSet();
	void	UpdateNamedSelDropDown ();

	// Accessors for clone, detach, etc static options:
	TSTR 	&GetDetachName () { return mDetachName; }
	void 	SetDetachName (TSTR & name) { mDetachName = name; }
	TSTR 	&GetShapeName () { return mCreateShapeName; }
	void 	SetShapeName (TSTR & name) { mCreateShapeName = name; }
	bool 	GetDetachToObject () { return mDetachToObject; }
	void 	SetDetachToObject (bool val) { mDetachToObject = val; SavePreferences (); }
	bool 	GetDetachAsClone () { return mDetachAsClone; }
	void 	SetDetachAsClone (bool val) { mDetachAsClone = val; SavePreferences (); }
	int		GetCloneTo () { return mCloneTo; }
	void	SetCloneTo (int ct) { mCloneTo=ct; }
	int		GetShapeType () { return mCreateShapeType; }
	void	SetShapeType (int ct) { mCreateShapeType=ct; SavePreferences(); }

	// New stuff to support painting interface.
	void GetSoftSelData( GenSoftSelData& softSelData, TimeValue t );

	// From MeshPaintHandler
	void		PrePaintStroke (PaintModeID paintMode);

	void		GetPaintHosts( Tab<MeshPaintHost*>& hosts, Tab<INode*>& nodes );
	float		GetPaintValue( PaintModeID paintMode );
	void		SetPaintValue( PaintModeID paintMode, float value );
	PaintAxisID	GetPaintAxis( PaintModeID paintMode );
	void		SetPaintAxis( PaintModeID paintMode, PaintAxisID axis );
	
	// per-host data...
	MNMesh*		GetPaintObject( MeshPaintHost* host );
	void		GetPaintInputSel( MeshPaintHost* host, FloatTab& selValues );
	void		GetPaintInputVertPos( MeshPaintHost* host, Point3Tab& vertPos );
	void		GetPaintMask( MeshPaintHost* host, BitArray& sel, FloatTab& softSel );
	
	// paint actions
	void		ApplyPaintSel( MeshPaintHost* host );
	void		ApplyPaintDeform( MeshPaintHost* host,BitArray *invalidVerts );
	void		RevertPaintDeform( MeshPaintHost* host,BitArray *invalidVerts );
	void		EpModPaintDeformCommit();
	void		EpModPaintDeformCancel();
	
	// notifications
	void		OnPaintModeChanged( PaintModeID prevMode, PaintModeID curMode );
	void		OnPaintDataActivate( PaintModeID paintMode, BOOL onOff );
	void		OnPaintDataRedraw( PaintModeID prevMode );
	void		OnPaintBrushChanged();

	// Event handlers
	void		OnParamSet(PB2Value& v, ParamID id, int tabIndex, TimeValue t);
	void		OnParamGet(PB2Value& v, ParamID id, int tabIndex, TimeValue t, Interval &valid);
	void		OnPaintBtn( int paintMode, TimeValue t );

	TCHAR		*GetPaintUndoString (PaintModeID mode);

	int GetSelectMode(); //0 is none, normal, 1 is subobject, 2 is multi select

	// alt ctrl key handling 
	bool		getAltKey() { return mAltKey;}
	void		setAltKey( const bool in_val) { mAltKey = in_val; }
	bool		getCtrlKey() {return mCtrlKey;}
	void		setCtrlKey( const bool in_val) { mCtrlKey = in_val; }

	int			getRingValue() { return mRingSpinnerValue; }
	void		setRingValue( const int in_ringVal) { mRingSpinnerValue = in_ringVal;}
	int			getLoopValue() { return mLoopSpinnerValue; }
	void		setLoopValue( const int in_loopVal) { mLoopSpinnerValue = in_loopVal;}

	//preview/highlights
	int			HitTestHighlight(TimeValue t, INode* inode,  int flags,
          ViewExp *vpt, ModContext* mc, HitRegion &hr,GraphicsWindow *gw, EditPolyData *pData);


	void		EpModHighlightChanged (); //needed to redraw screen and update highlight text
	void		InvalidateNumberHighlighted(BOOL zeroOut = FALSE); //if zeroout is true we zero out the mHilightedItems tab on the EditPolyMods.

	void		DisplayHighlightFace (TimeValue t, GraphicsWindow *gw, MNMesh *mesh, int ff);
	void		DisplayHighlightEdge (TimeValue t, GraphicsWindow *gw, MNMesh *mesh, int ee);
	void		DisplayHighlightVert (TimeValue t, GraphicsWindow *gw,MNMesh *mesh, int vv);

	//new constrain toggling behavior
	int			GetLastConstrainType() {return mLastConstrainType;}
	void		SetLastConstrainType(int which) {mLastConstrainType = which;}
	
	//generic functions, mainly used to hit test multiple subs at once for highlighting and soft sel cmode.
	static MeshSubHitRec * GetClosestHitRec(SubObjHitList &hitList);
	static SubObjHitList * GetClosestHitList( SubObjHitList &vertHitList,SubObjHitList &edgeHitList,SubObjHitList &faceHitList,
 								   int vertRes, int edgeRes, int faceRes,
								   MeshSubHitRec *vertClosest,MeshSubHitRec *edgeClosest,MeshSubHitRec *faceClosest,
								   int &res);

private:
	void		getTransformationMatrices( Matrix3 & refMatrix, Matrix3 & worldTransformationMatrix, INode * node );
	void		convertPointToCurrentCoordSys( Point3 &refPoint, INode* node );
	void		convertPointFromCurrentCoordSys( Point3 &refPoint, INode * node );
	INode *		getSelectedNodeFromContextList();

	void SetSelectMode(int mode); //0, none,subobj, multi
	void SetConstraintType(int what);

	void SetCommandModeFromOld(int oldCommandMode); //new for gouda when changing sub obj mode, we keep the current command mode active
};

// EditPolyData flags:
const DWORD kEPDataCommit = 0x01;
const DWORD kEPDataHeld = 0x02;
const DWORD kEPDataPrimary = 0x08;
const DWORD kEPDataLastCut = 0x2000;

// Data efficiency / caching:
// Need to make mpMesh a full copy, and use it most of the time.
// Need to note number of faces, vertices, edges created "last time" by each operation,
// so that we can do big VAlloc, FAlloc, EAlloc at the beginning.

class EditPolyData : public LocalModData, public IMeshSelectData, public MeshPaintHost {
friend class EditPolySelectRestore;
friend class EditPolyHideRestore;
friend class EditPolyDataFlagRestore;
friend class EditPolyMod;

	BitArray 	mVertSel ;
	BitArray 	mEdgeSel;
	BitArray 	mFaceSel;
	BitArray 	mVertHide ;
	BitArray 	mFaceHide;

	MNMesh 		*mpMesh;			// Simple copies of the mesh before and after modification
	MNMesh 		*mpMeshOutput;	// Simple copies of the mesh before and after modification
	MNMesh 		*mpPaintMesh;	// shallow copies of data from mpMesh, LocalPaintDeformData.
	BitArray	*mpNewSelection;
	DWORD		mFlags;


	PolyOperationRecord		*mpOpList;
	MNTempData				*mpTemp;
	MNTempData				*mpTempOutput;
	LocalPolyOpData			*mpPolyOpData;
	MNMeshSelectionConverter mSelConv;

	float		mSlicePlaneSize;
	Interval	mVertexDistanceValidity;
	Interval	mGeomValid;
	Interval	mTopoValid;
	Interval	mMeshValid;

	// Lists of named selection sets
	GenericNamedSelSetList vselSet;
	GenericNamedSelSetList fselSet;
	GenericNamedSelSetList eselSet;

	std::set<int> mHighlightedItems; //the index of the item (v,e or f) that we have highlighted.

public:
	// LocalModData methods
	void* GetInterface(ULONG id) { if (id == I_MESHSELECTDATA) return(IMeshSelectData*)this; else return LocalModData::GetInterface(id); }

	EditPolyData (PolyObject *pObj, TimeValue t);
	EditPolyData ();
	~EditPolyData();

	void			Initialize (PolyObject *pObj, TimeValue t);
	LocalModData	*Clone();
	MNMesh			*GetMesh() {return mpMesh;}
	MNMesh			*GetMeshOutput() { return mpMeshOutput; }

	void			SetCache(PolyObject *pObj, TimeValue t);
	void			FreeCache();
	void			InvalidateCache () { mMeshValid.SetEmpty(); }
	Interval		CacheValidity () { return mMeshValid; }
	void 			SynchBitArrays();
	void 			SetOutputCache(MNMesh &mesh);
	void 			FreeOutputCache();

	MNMesh			*GetPaintMesh ();
	void			FreePaintMesh ();

	// From IMeshSelectData:
	BitArray GetVertSel() { return mVertSel; }
	BitArray GetFaceSel() { return mFaceSel; }
	BitArray GetEdgeSel() { return mEdgeSel; }

	void 	SetVertSel (BitArray &set, IMeshSelect *imod, TimeValue t);
	void 	SetFaceSel (BitArray &set, IMeshSelect *imod, TimeValue t);
	void 	SetEdgeSel (BitArray &set, IMeshSelect *imod, TimeValue t);

	void	SetVertHide (BitArray & set, EditPolyMod *pmod);
	void	SetFaceHide (BitArray & set, EditPolyMod *pmod);

	// Apparently have to do something for these methods...
	GenericNamedSelSetList & GetNamedVertSelList () { return vselSet; }
	GenericNamedSelSetList & GetNamedEdgeSelList () { return eselSet; }
	GenericNamedSelSetList & GetNamedFaceSelList () { return fselSet; }
	GenericNamedSelSetList & GetNamedSel (int nsl) {
		if (nsl==NS_VERTEX) return vselSet;
		if (nsl==NS_EDGE) return eselSet;
		return fselSet;
	}

	// Accessor:
	void SetFlag (DWORD flag) { mFlags |= flag; }
	void ClearFlag (DWORD flag) { if (mFlags & flag) mFlags -= flag; }
	bool GetFlag (DWORD flag) { return (mFlags & flag) > 0; }

	// New selection methods:
	void		SetupNewSelection (int meshSelLevel);
	BitArray 	*GetNewSelection () { return mpNewSelection; }
	BitArray 	*GetCurrentSelection (int meshSelLevel, bool useStackSelection);
	bool 		ApplyNewSelection (EditPolyMod *pMod, bool keepOld=false, bool invert=false, bool select=true);
	void 		TranslateNewSelection (int selLevelFrom, int selLevelTo);
	void 		SetConverterFlag (DWORD flag, bool value) { mSelConv.SetFlag (flag, value); }
	void 		FreeNewSelection () { if (mpNewSelection) { delete mpNewSelection; mpNewSelection=NULL; } }

	// PushOperation: Pushes an op onto the end of the stack (the last to be applied).
	void			PushOperation (PolyOperation *pOp);

	// PopOperation: Pops an op off the end of the stack (the last one to be applied).
	PolyOperation	*PopOperation ();
	void 			DeleteAllOperations ();
	void 			ApplyAllOperations (MNMesh & mesh);
	void 			ApplyHide (MNMesh & mesh);
	bool 			HasCommittedOperations () { return mpOpList != NULL; }
	void 			RescaleWorldUnits (float f);

	float		*GetSoftSelection (TimeValue t,float falloff, float pinch, float bubble, int edist, bool ignoreBackfacing,Interval & edistValid);
	void		InvalidateTempData (DWORD parts);
	void 		InvalidateDistances ();
	void 		InvalidateSoftSelection ();
	MNTempData *TempData ();
	MNTempData *TempDataOutput ();

	// This is called after setting some mesh components to be MN_DEAD,
	// and just before calling mesh.CollapseDeadStructs - giving us a chance to synchronize
	// our selection data.
	void CollapseDeadSelections (EditPolyMod *pMod, MNMesh & mesh);

	void Hide (EditPolyMod *pMod, bool isFaceLevel, bool hideSelected);
	void Unhide (EditPolyMod *pMod, bool isFaceLevel);
	bool HiddenVertex (int i) { return (i<mVertHide.GetSize()) ? (mVertHide[i]!=0) : false; }
	bool HiddenFace (int i) { return (i<mFaceHide.GetSize()) ? (mFaceHide[i]!=0) : false; }
	void GrowSelection(IMeshSelect *imod, int level);
	void ShrinkSelection(IMeshSelect *imod, int level);
	void SelectEdgeRing(IMeshSelect *imod);
	void SelectEdgeLoop(IMeshSelect *imod);
	void SelectByMaterial (EditPolyMod *pMod);
	void SelectBySmoothingGroup (EditPolyMod *pMod);
	void AcquireStackSelection (EditPolyMod *pMod, int level);

	LocalPolyOpData *GetPolyOpData () { return mpPolyOpData; }
	void 			SetPolyOpData (int opID);
	void 			SetPolyOpData (LocalPolyOpData *pOp) { mpPolyOpData = pOp; }
	LocalPolyOpData *RemovePolyOpData() { LocalPolyOpData *ret = mpPolyOpData; mpPolyOpData = NULL; return ret; }
	LocalPolyOpData *CreateLocalPolyOpData (int opID);
	void			ClearPolyOpData () {
						if (mpPolyOpData != NULL) mpPolyOpData->DeleteThis ();
						mpPolyOpData = NULL;
					}

	float GetSlicePlaneSize () { return mSlicePlaneSize; }
	void SetSlicePlaneSize (float size) { mSlicePlaneSize=size; }
};

class EditPolyDataSetFlagEnumProc : public ModContextEnumProc {
private:
	DWORD	mFlag;
	bool	mSet;

public:
	EditPolyDataSetFlagEnumProc (DWORD flag, bool set) : mFlag(flag), mSet(set) { }
	BOOL proc (ModContext *mc) {
		EditPolyData *pData = (EditPolyData *) mc->localData;
		if (pData != NULL) {
			if (mSet) pData->SetFlag (mFlag);
			else pData->ClearFlag (mFlag);
		}
		return true;
	}
};

// Class EPolyBackspaceUser: Used to process backspace key input.
class EditPolyBackspaceUser : public EventUser {
	EditPolyMod *mpEditPoly;
public:
	void Notify();
	void SetEPoly (EditPolyMod *m) { mpEditPoly=m; }
};

// --- Class Descriptor -------------------------------

class EditPolyModClassDesc : public ClassDesc2 {
public:
	int 			IsPublic() { return true; }
	void *			Create(BOOL loading = FALSE);
	const TCHAR *	ClassName() { return GetString(IDS_EDIT_POLY_MOD); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return kEDIT_MOD_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_MAX_EDIT);}

	// The following are used by MAX Script and the schematic view:
	const TCHAR		*InternalName() { return _T("EditPolyMod"); }
	HINSTANCE		HInstance() { return hInstance; }

#ifndef RENDER_VER
	int NumActionTables() { return 1; }
#else
	int NumActionTables() { return 0; }
#endif
	ActionTable*  GetActionTable (int i);
};

// --- Command Modes & Mouse Procs -------------------------------

// Virtual mouse procs: these are useful for multiple actual procs.
class EditPolyPickEdgeMouseProc : public MouseCallBack {
protected:
	EPolyMod *mpEPoly;
	IObjParam *ip;

public:
	EditPolyPickEdgeMouseProc(EPolyMod* e, IObjParam *i) : mpEPoly(e), ip(i) { }
	
	HitRecord		*HitTestEdges(IPoint2 &m, ViewExp *vpt, float *prop, Point3 *snapPoint);
	virtual int		proc (HWND hwnd, int msg, int point, int flags, IPoint2 m );
	virtual bool	EdgePick (HitRecord *hr, float prop)=0;
	virtual bool	HitTestResult () { return false; }
};

class EditPolyPickFaceMouseProc : public MouseCallBack {
protected:
	EPolyMod	*mpEPoly;
	IObjParam	*ip;
	int			face;	// Face we picked
	Tab<float>	bary;	// Barycentric coords - size of face degree, with up to 3 nonzero values summing to 1
	INode		*mpNode;	// Node we picked on

public:
	EditPolyPickFaceMouseProc(EPolyMod* e, IObjParam *i) : mpEPoly(e), ip(i), mpNode(NULL) { }
	
	HitRecord		*HitTestFaces(IPoint2 &m, ViewExp *vpt);
	void			ProjectHitToFace (IPoint2 &m, ViewExp *vpt, HitRecord *hr, Point3 *snapPoint);
	virtual int		proc (HWND hwnd, int msg, int point, int flags, IPoint2 m);
	virtual void	FacePick()=0;
	virtual bool	HitTestResult () { return false; }
};

// Used in both create edge and edit triangulation:
class EditPolyConnectVertsMouseProc : public MouseCallBack {
protected:
	EPolyMod	*mpEPoly;
	IObjParam	*ip;
	int			v1;
	int			v2;
	Tab<int>	neighbors;
	IPoint2		m1;
	IPoint2		lastm;

public:
	EditPolyConnectVertsMouseProc (EPolyMod *e, IObjParam *i) : mpEPoly(e), ip(i) { }
	
	HitRecord		*HitTestVertices (IPoint2 & m, ViewExp *vpt);
	void			DrawDiag (HWND hWnd, const IPoint2 & m);
	void			SetV1 (int vv);
	int				proc (HWND hwnd, int msg, int point, int flags, IPoint2 m);
	virtual void	VertConnect () {}
};

// Actual procs & command modes:

class EditPolyCreateVertMouseProc : public MouseCallBack {
private:
	EPolyMod *mpEPoly;
	IObjParam *ip;
public:
	EditPolyCreateVertMouseProc (EPolyMod* e, IObjParam *i) : mpEPoly(e), ip(i) { }
	int proc (HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class EditPolyCreateVertCMode : public CommandMode {
private:
	ChangeFGObject				fgProc;
	EditPolyCreateVertMouseProc proc;
	EditPolyMod					*mpEditPoly;

public:
	EditPolyCreateVertCMode(EditPolyMod* e, IObjParam *i) : fgProc(e), proc(e,i), mpEditPoly(e) { }
	
	int							Class() { return MODIFY_COMMAND; }
	int							ID() { return CID_CREATEVERT; }
	MouseCallBack				*MouseProc(int *numPoints) {*numPoints=1; return &proc;}
	ChangeForegroundCallback	*ChangeFGProc() {return &fgProc;}
	BOOL						ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void 						EnterMode();
	void 						ExitMode();
};

class EditPolyCreateEdgeMouseProc : public EditPolyConnectVertsMouseProc {
public:
	EditPolyCreateEdgeMouseProc (EPolyMod *e, IObjParam *i) : EditPolyConnectVertsMouseProc (e,i) {}
	void VertConnect ();
};

class EditPolyCreateEdgeCMode : public CommandMode {
private:
	ChangeFGObject				fgProc;
	EditPolyCreateEdgeMouseProc proc;
	EditPolyMod					*mpEditPoly;

public:
	EditPolyCreateEdgeCMode(EditPolyMod* e, IObjParam *i) : fgProc(e), proc(e,i), mpEditPoly(e) { }
	
	int 						Class() { return MODIFY_COMMAND; }
	int 						ID() { return CID_CREATEEDGE; }
	MouseCallBack				*MouseProc(int *numPoints) {*numPoints=2; return &proc;}
	ChangeForegroundCallback	*ChangeFGProc() {return &fgProc;}
	BOOL 						ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void 						EnterMode();
	void 						ExitMode();
};

class EditPolyCreateFaceMouseProc : public CreateFaceMouseProcTemplate {
	EPolyMod *m_EPoly;
protected:
	TSTR GetFaceCreationUndoMessage() {return GetString (IDS_CREATE_FACE);}
	TSTR GetFailedFaceCreationMessage() {return GetString(IDS_ILLEGAL_NEW_FACE);}
	TSTR GetFailedFaceCreationTitle() {return GetString(IDS_EDIT_POLY_MOD);}
	int GetMaxDegree() {return CreateFaceMouseProcTemplate::UNLIMITED_DEGREE;}

	bool CreateVertex(const Point3& in_worldLocation, int& out_vertex);
	bool CreateFace(MaxSDK::Array<int>& in_vertices);
	bool HitTestVertex(IPoint2 in_mouse, ViewExp* in_viewport, INode*& out_node, int& out_vertex);
	Point3 GetVertexWP(int in_vertex);
	void DisplayDataChanged() {m_EPoly->EpModLocalDataChanged(PART_DISPLAY);}

	void ShowExistingVertexCursor() { SetCursor(GetInterface()->GetSysCursor(SYSCUR_SELECT));}
	void ShowCreateVertexCursor();

public:
	EditPolyCreateFaceMouseProc(EPolyMod* in_e, IObjParam* in_i);
};

class EditPolyCreateFaceCMode : public CommandMode {
	ChangeFGObject				fgProc;
	EditPolyMod					*mpEditPoly;
	EditPolyCreateFaceMouseProc proc;

public:
	EditPolyCreateFaceCMode(EditPolyMod* e, IObjParam *i) : fgProc(e), proc(e,i), mpEditPoly(e) { }
	
	int 						Class() { return MODIFY_COMMAND; }
	int 						ID() { return CID_CREATEFACE; }
	MouseCallBack				*MouseProc(int *numPoints) {*numPoints=999999; return &proc;}
	ChangeForegroundCallback	*ChangeFGProc() {return &fgProc;}
	BOOL 						ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void 						EnterMode();
	void 						ExitMode();

	void 						Display (GraphicsWindow *gw);
	void 						Backspace () { proc.Backspace(); }
};

class EditPolyAttachPickMode : public PickModeCallback, public PickNodeCallback {
	EditPolyMod	*mpEditPoly;
	IObjParam	*ip;

public:
	EditPolyAttachPickMode() : mpEditPoly(NULL), ip(NULL) { }
	
	void 				SetPolyObject (EditPolyMod *e, IObjParam *i) { mpEditPoly=e; ip=i; }
	void 				ClearPolyObject () { mpEditPoly=NULL; ip=NULL; }
	BOOL 				HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
	BOOL 				Pick(IObjParam *ip,ViewExp *vpt);
	void 				EnterMode(IObjParam *ip);
	void 				ExitMode(IObjParam *ip);		

	BOOL 				Filter(INode *node);
	BOOL 				RightClick(IObjParam *ip,ViewExp *vpt) {return true;}
	PickNodeCallback	*GetFilter() {return this;}
};

class EditPolyAttachHitByName : public HitByNameDlgCallback {
	EditPolyMod *mpEditPoly;
	IObjParam	*ip;

public:
	EditPolyAttachHitByName (EditPolyMod *e, IObjParam *iop) : mpEditPoly(e), ip(iop) { }
	
	TCHAR 	*dialogTitle() { return GetString(IDS_ATTACH_LIST); }
	TCHAR 	*buttonText() { return GetString(IDS_ATTACH); }
	int		filter(INode *node);	
	void	proc(INodeTab &nodeTab);
};

class EditPolyShapePickMode : public PickModeCallback, public PickNodeCallback {
	EditPolyMod *mpMod;
	IObjParam	*ip;

public:
	EditPolyShapePickMode() : mpMod(NULL), ip(NULL) { }
	
	void 				SetMod (EditPolyMod *pMod, IObjParam *i) { mpMod = pMod; ip = i; }
	void 				ClearMod () { mpMod=NULL; ip=NULL; }
	BOOL 				HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
	BOOL 				Pick(IObjParam *ip,ViewExp *vpt);
	void 				EnterMode(IObjParam *ip);
	void 				ExitMode(IObjParam *ip);		

	BOOL 				Filter(INode *node);
	BOOL 				RightClick(IObjParam *ip,ViewExp *vpt) {return TRUE;}
	PickNodeCallback	*GetFilter() {return this;}
};

class EditPolyDivideEdgeProc : public EditPolyPickEdgeMouseProc {
public:
	EditPolyDivideEdgeProc(EPolyMod* e, IObjParam *i) : EditPolyPickEdgeMouseProc(e,i) {}
	
	bool EdgePick (HitRecord *hr, float prop);
	bool HitTestResult () { return true; }
};

class EditPolyDivideEdgeCMode : public CommandMode {
private:
	ChangeFGObject			fgProc;		
	EditPolyDivideEdgeProc	proc;
	EditPolyMod				*mpEditPoly;

public:
	EditPolyDivideEdgeCMode(EditPolyMod* e, IObjParam *i) : fgProc(e), proc(e,i), mpEditPoly(e) { }
	int 						Class() { return MODIFY_COMMAND; }
	int 						ID() { return CID_DIVIDEEDGE; }
	MouseCallBack				*MouseProc(int *numPoints) {*numPoints=1; return &proc;}
	ChangeForegroundCallback	*ChangeFGProc() {return &fgProc;}
	BOOL						ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void 						EnterMode();
	void 						ExitMode();
};

class EditPolyDivideFaceProc : public EditPolyPickFaceMouseProc {
public:
	EditPolyDivideFaceProc(EPolyMod* e, IObjParam *i) : EditPolyPickFaceMouseProc(e,i) {}
	
	void FacePick();
	bool HitTestResult () { return true; }
};

class EditPolyDivideFaceCMode : public CommandMode {
private:
	ChangeFGObject			fgProc;		
	EditPolyDivideFaceProc	proc;
	EditPolyMod*			mpEditPoly;

public:
	EditPolyDivideFaceCMode(EditPolyMod* e, IObjParam *i) : fgProc(e), proc(e,i), mpEditPoly(e) { }
	
	int 						Class() { return MODIFY_COMMAND; }
	int 						ID() { return CID_DIVIDEFACE; }
	MouseCallBack				*MouseProc(int *numPoints) {*numPoints=1; return &proc;}
	ChangeForegroundCallback	*ChangeFGProc() {return &fgProc;}
	BOOL 						ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void 						EnterMode();
	void 						ExitMode();
};

class EditPolyBridgeMouseProc : public MouseCallBack {
protected:
	EPolyMod	*mpEPoly;
	IObjParam	*ip;
	int			hit1;
	int			hit2;
	IPoint2		 m1;
	IPoint2		lastm;
	int			mHitLevel;

public:
	EditPolyBridgeMouseProc (EPolyMod *e, IObjParam *i, int hitLevel)
		: mpEPoly(e), ip(i), mHitLevel(hitLevel) { }

	HitRecord		*HitTest (IPoint2 & m, ViewExp *vpt);
	void			DrawDiag (HWND hWnd, const IPoint2 & m);
	int				proc (HWND hwnd, int msg, int point, int flags, IPoint2 m);
	virtual void	Bridge () {}
};

class EditPolyBridgeBorderProc : public EditPolyBridgeMouseProc {
public:
	EditPolyBridgeBorderProc (EPolyMod *e, IObjParam *i)
		: EditPolyBridgeMouseProc (e,i,SUBHIT_MNEDGES|SUBHIT_OPENONLY) { }
	
	void Bridge ();
};

class EditPolyBridgeBorderCMode : public CommandMode {
private:
	ChangeFGObject				fgProc;		
	EditPolyBridgeBorderProc	proc;
	EditPolyMod					*mpEditPoly;

public:
	EditPolyBridgeBorderCMode (EditPolyMod* e, IObjParam *i) : fgProc(e), proc(e,i), mpEditPoly(e) { }
	
	int 						Class() { return MODIFY_COMMAND; }
	int 						ID() { return CID_BRIDGE_BORDER; }
	MouseCallBack				*MouseProc(int *numPoints) {*numPoints=3; return &proc;}
	ChangeForegroundCallback	*ChangeFGProc() {return &fgProc;}
	BOOL 						ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void 						EnterMode();
	void 						ExitMode();
};

class EditPolyBridgeEdgeProc : public EditPolyBridgeMouseProc {
public:
	EditPolyBridgeEdgeProc (EPolyMod *e, IObjParam *i)
		: EditPolyBridgeMouseProc (e,i,SUBHIT_MNEDGES|SUBHIT_OPENONLY) { }
	void Bridge ();
};

class EditPolyBridgeEdgeCMode : public CommandMode {
private:
	ChangeFGObject			fgProc;		
	EditPolyBridgeEdgeProc	proc;
	EditPolyMod				*mpEditPoly;

public:
	EditPolyBridgeEdgeCMode (EditPolyMod* e, IObjParam *i) : fgProc(e), proc(e,i), mpEditPoly(e) { }
	
	int 						Class() { return MODIFY_COMMAND; }
	int 						ID() { return CID_BRIDGE_EDGE; }
	MouseCallBack				*MouseProc(int *numPoints) {*numPoints=3; return &proc;}
	ChangeForegroundCallback	*ChangeFGProc() {return &fgProc;}
	BOOL 						ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void 						EnterMode();
	void 						ExitMode();
};

class EditPolyBridgePolygonProc : public EditPolyBridgeMouseProc {
public:
	EditPolyBridgePolygonProc (EPolyMod *e, IObjParam *i)
		: EditPolyBridgeMouseProc (e,i,SUBHIT_MNFACES) { }
	void Bridge ();
};

class EditPolyBridgePolygonCMode : public CommandMode {
private:
	ChangeFGObject				fgProc;		
	EditPolyBridgePolygonProc	proc;
	EditPolyMod					*mpEditPoly;

public:
	EditPolyBridgePolygonCMode (EditPolyMod* e, IObjParam *i) : fgProc(e), proc(e,i), mpEditPoly(e) { }
	
	int 						Class() { return MODIFY_COMMAND; }
	int 						ID() { return CID_BRIDGE_POLYGON; }
	MouseCallBack				*MouseProc(int *numPoints) {*numPoints=2; return &proc;}
	ChangeForegroundCallback	*ChangeFGProc() {return &fgProc;}
	BOOL 						ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void 						EnterMode();
	void 						ExitMode();
};

class EditPolyExtrudeProc : public MouseCallBack {
private:
	EPolyMod	*mpEditPoly;
	Interface	*ip;
	IPoint2		om;
	bool		mInExtrude;
	float		mStartHeight;

	void BeginExtrude (TimeValue t);
	void EndExtrude (TimeValue t, bool accept);
	void DragExtrude (TimeValue t, float value);

public:
	EditPolyExtrudeProc (EPolyMod *e, IObjParam *i) : mpEditPoly(e), ip(i),
		mInExtrude(false), mStartHeight(0.0f) { }
	
		int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class EditPolyExtrudeSelectionProcessor : public GenModSelectionProcessor {
protected:
	HCURSOR GetTransformCursor();
public:
	EditPolyExtrudeSelectionProcessor(EditPolyExtrudeProc *mc, EditPolyMod *e, IObjParam *i) 
		: GenModSelectionProcessor(mc,e,i) {}
};

class EditPolyExtrudeCMode : public CommandMode {
private:
	ChangeFGObject						fgProc;
	EditPolyExtrudeSelectionProcessor	mouseProc;
	EditPolyExtrudeProc					eproc;
	EditPolyMod							*mpEditPoly;

public:
	EditPolyExtrudeCMode(EditPolyMod* e, IObjParam *i) :
	  fgProc(e), mouseProc(&eproc,e,i), eproc(e,i), mpEditPoly(e) { }
	
	int 						Class() { return MODIFY_COMMAND; }
	int 						ID() { return CID_EXTRUDE; }
	MouseCallBack				*MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
	ChangeForegroundCallback	*ChangeFGProc() { return &fgProc; }
	BOOL 						ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void 						EnterMode();
	void 						ExitMode();
};

class EditPolyChamferMouseProc : public MouseCallBack {
	friend class EditPolyChamferCMode;
private:
	EPolyMod	*mpEditPoly;
	Interface	*ip;
	IPoint2		om;
	bool		mInChamfer;
	int			mChamferOperation;
	float		mStartAmount;

	void Begin (TimeValue t, int meshSelLevel);
	void End (TimeValue t, bool accept);
	void Drag (TimeValue t, float amount);

public:
	EditPolyChamferMouseProc (EPolyMod* e, IObjParam *i) : mpEditPoly(e), ip(i), mInChamfer(false) { }
	int proc (HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class EditPolyChamferSelectionProcessor : public GenModSelectionProcessor {
	EPolyMod *mpEditPoly;
protected:
	HCURSOR GetTransformCursor();
public:
	EditPolyChamferSelectionProcessor(EditPolyChamferMouseProc *mc, EditPolyMod *e, IObjParam *i) 
		: GenModSelectionProcessor(mc,e,i), mpEditPoly(e) { }
};

class EditPolyChamferCMode : public CommandMode {
private:
	ChangeFGObject						fgProc;
	EditPolyChamferSelectionProcessor	mouseProc;
	EditPolyChamferMouseProc			eproc;
	EditPolyMod							*mpEditPoly;

public:
	EditPolyChamferCMode (EditPolyMod* e, IObjParam *i) :
	  fgProc(e), mouseProc(&eproc,e,i), eproc(e,i), mpEditPoly(e) { }
	
	int 						Class() { return MODIFY_COMMAND; }
	int 						ID() { return CID_POLYCHAMFER; }
	MouseCallBack				*MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
	ChangeForegroundCallback	*ChangeFGProc() { return &fgProc; }
	BOOL 						ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void 						EnterMode();
	void 						ExitMode();
};

class EditPolyBevelMouseProc : public MouseCallBack {
private:
	EPolyMod	*mpEditPoly;
	Interface	*ip;
	IPoint2		m0 ;
	IPoint2		m1;
	bool		m0set;
	bool		m1set;
	float		height;
	bool		mInBevel;
	float		mStartOutline;
	float		mStartHeight;

	void Begin (TimeValue t);
	void End (TimeValue t, bool accept);
	void DragHeight (TimeValue t, float height);
	void DragOutline (TimeValue t, float outline);

public:
	EditPolyBevelMouseProc(EPolyMod* e, IObjParam *i) : mpEditPoly(e), ip(i),
		m0set(false), m1set(false), mStartOutline(0.0f), mStartHeight(0.0f), mInBevel(false) { }
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class EditPolyBevelSelectionProcessor : public GenModSelectionProcessor {
protected:
	HCURSOR GetTransformCursor();
public:
	EditPolyBevelSelectionProcessor(EditPolyBevelMouseProc *mc, EditPolyMod *e, IObjParam *i) 
		: GenModSelectionProcessor(mc,e,i) {}
};


class EditPolyBevelCMode : public CommandMode {
private:
	ChangeFGObject					fgProc;
	EditPolyBevelSelectionProcessor mouseProc;
	EditPolyBevelMouseProc			eproc;
	EditPolyMod						*mpEditPoly;
public:
	EditPolyBevelCMode(EditPolyMod* e, IObjParam *i) :
	  fgProc(e), mouseProc(&eproc,e,i), eproc(e,i), mpEditPoly(e) { }
	
	int							Class() { return MODIFY_COMMAND; }
	int							ID() { return CID_BEVEL; }
	MouseCallBack				*MouseProc(int *numPoints) { *numPoints=3; return &mouseProc; }
	ChangeForegroundCallback	*ChangeFGProc() { return &fgProc; }
	BOOL						ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void						EnterMode();
	void						ExitMode();
};


#ifdef SS_PROTOTYPE
class EditPolyEditSSMouseProc : public MouseCallBack {
private:
	EPolyMod	*mpEditPoly;
	Interface	*ip;
	IPoint2		m0 ;
	IPoint2		m1;
	bool		m0set;
	bool		m0move;
	bool		m1set;
	bool		mInEditSS;
	bool		mHitMade;
	float		mStartPinch;
	float		mStartBubble;

	void SetFalloff(ViewExp *vpt,TimeValue t);
	void Begin();
	void End (TimeValue t, bool accept);
	void DragPinch (TimeValue t, float height);
	void DragBubble (TimeValue t, float outline);

public:
	EditPolyEditSSMouseProc(EPolyMod* e, IObjParam *i) : mpEditPoly(e), ip(i),
		m0set(false),m0move(false), m1set(false), mStartPinch(0.0f), mStartBubble(0.0f), mInEditSS(false) { }
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};


class EditPolyEditSSMode : public CommandMode {
private:
	ChangeFGObject					fgProc;
	EditPolyEditSSMouseProc			eproc;
	EditPolyMod						*mpEditPoly;

public:
	EditPolyEditSSMode(EditPolyMod* e, IObjParam *i) :
	  fgProc(e), eproc(e,i), mpEditPoly(e) { }
	
	int							Class() { return MODIFY_COMMAND; }
	int							ID() { return CID_BEVEL; }
	MouseCallBack				*MouseProc(int *numPoints) { *numPoints=3; return &eproc; }
//	MouseCallBack				*MouseProc(int *numPoints) { *numPoints=3; return &eproc; }
	ChangeForegroundCallback	*ChangeFGProc() { return &fgProc; }
	BOOL						ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void						EnterMode();
	void						ExitMode();
};

#endif

class EditPolyInsetMouseProc : public MouseCallBack {
private:
	EPolyMod	*mpEditPoly;
	Interface	*ip;
	IPoint2		m0;
	bool		m0set;
	bool		mInInset;
	float		mStartAmount;

	void Begin (TimeValue t);
	void End (TimeValue t, bool accept);
	void Drag (TimeValue t, float amount);

public:
	EditPolyInsetMouseProc(EPolyMod* e, IObjParam *i) : mpEditPoly(e), ip(i),
		m0set(false), mInInset(false), mStartAmount(0.0f) { }
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class EditPolyInsetSelectionProcessor : public GenModSelectionProcessor {
protected:
	HCURSOR GetTransformCursor();
public:
	EditPolyInsetSelectionProcessor(EditPolyInsetMouseProc *mc, EditPolyMod *e, IObjParam *i) 
		: GenModSelectionProcessor(mc,e,i) {}
};

class EditPolyInsetCMode : public CommandMode {
private:
	ChangeFGObject					fgProc;
	EditPolyInsetSelectionProcessor mouseProc;
	EditPolyInsetMouseProc			eproc;
	EditPolyMod						*mpEditPoly;

public:
	EditPolyInsetCMode(EditPolyMod* e, IObjParam *i) :
	  fgProc(e), mouseProc(&eproc,e,i), eproc(e,i), mpEditPoly(e) { }
	
	int							Class() { return MODIFY_COMMAND; }
	int							ID() { return CID_INSET; }
	MouseCallBack				*MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
	ChangeForegroundCallback	*ChangeFGProc() { return &fgProc; }
	BOOL 						ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void 						EnterMode();
	void 						ExitMode();
};

class EditPolyOutlineMouseProc : public MouseCallBack {
private:
	EPolyMod	*mpEditPoly;
	Interface	*ip;
	IPoint2		m0;

	bool mInOutline;
	float mStartAmount;

	void Begin (TimeValue t);
	void Accept (TimeValue t);
	void Cancel ();
	void Drag (TimeValue t, float amount);

public:
	EditPolyOutlineMouseProc(EPolyMod* e, IObjParam *i) : mpEditPoly(e), ip(i), mInOutline(false) { }
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class EditPolyOutlineSelectionProcessor : public GenModSelectionProcessor {
protected:
	HCURSOR GetTransformCursor();
public:
	EditPolyOutlineSelectionProcessor(EditPolyOutlineMouseProc *mc, EditPolyMod *e, IObjParam *i) 
		: GenModSelectionProcessor(mc,e,i) {}
};

class EditPolyOutlineCMode : public CommandMode {
private:
	ChangeFGObject						fgProc;
	EditPolyOutlineSelectionProcessor	mouseProc;
	EditPolyOutlineMouseProc			eproc;
	EditPolyMod							*mpEditPoly;

public:
	EditPolyOutlineCMode(EditPolyMod* e, IObjParam *i) :
	  fgProc(e), mouseProc(&eproc,e,i), eproc(e,i), mpEditPoly(e) { }
	
	int							Class() { return MODIFY_COMMAND; }
	int							ID() { return CID_OUTLINE_FACE; }
	MouseCallBack				*MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
	ChangeForegroundCallback	*ChangeFGProc() { return &fgProc; }
	BOOL 						ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void 						EnterMode();
	void 						ExitMode();
};

class EditPolyExtrudeVEMouseProc : public MouseCallBack {
private:
	EPolyMod	*mpEditPoly;
	Interface	*ip;
	IPoint2		m0;
	bool		mInExtrude;
	float		mStartHeight;
	float		mStartWidth;

	void BeginExtrude (TimeValue t);
	void EndExtrude (TimeValue t, bool accept);
	void DragExtrude (TimeValue t, float height, float width);

public:
	EditPolyExtrudeVEMouseProc(EPolyMod* e, IObjParam *i) : mpEditPoly(e), ip(i), mInExtrude(false) { }
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m );
};

class EditPolyExtrudeVESelectionProcessor : public GenModSelectionProcessor {
private:
	EPolyMod *mpEPoly;
protected:
	HCURSOR GetTransformCursor();
public:
	EditPolyExtrudeVESelectionProcessor(EditPolyExtrudeVEMouseProc *mc, EditPolyMod *e, IObjParam *i) 
		: GenModSelectionProcessor(mc,e,i), mpEPoly(e) { }
};

class EditPolyExtrudeVECMode : public CommandMode {
private:
	ChangeFGObject						fgProc;	
	EditPolyExtrudeVESelectionProcessor mouseProc;
	EditPolyExtrudeVEMouseProc			eproc;
	EditPolyMod							*mpEditPoly;

public:
	EditPolyExtrudeVECMode(EditPolyMod *e, IObjParam *i) :
	  fgProc(e), mouseProc(&eproc,e,i), eproc(e,i), mpEditPoly(e) { }
	
	int 						Class() { return MODIFY_COMMAND; }
	int 						ID() { return CID_EXTRUDE_VERTEX_OR_EDGE; }
	MouseCallBack				*MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
	ChangeForegroundCallback	*ChangeFGProc() {return &fgProc;}
	BOOL						ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void 						EnterMode();
	void 						ExitMode();
};

class EditPolyCutProc : public MouseCallBack {
friend class EditPolyCutCMode;
private:
	EPolyMod	*mpEPoly;
	IObjParam	*ip;
	int 		mStartLevel ;
	int 		mStartIndex;
	Point3		mStartPoint;
	Matrix3		mObj2world;
	IPoint2		mMouse1;
	IPoint2		mMouse2;
	int			mLastHitLevel;
	int			mLastHitIndex;
	Point3		mLastHitPoint ;
	Point3		mLastHitDirection;
	bool		mSuspendPreview;

	// In the following, "completeAnalysis" means that we want to fill in the information
	// about exactly which component we hit, and what point in object space we hit.
	// (So we fill in mLastHitIndex and mLastHitPoint.)
	// (We also fill in the obj2world transform of our node here.)
	// If completeAnalysis is false, all we care about is knowing what SO level
	// we're mousing over (so we (always) fill in mLastHitLevel).
	HitRecord 	*HitTestVerts(IPoint2 &m, ViewExp *vpt, bool completeAnalysis);
	HitRecord 	*HitTestEdges(IPoint2 &m, ViewExp *vpt, bool completeAnalysis);
	HitRecord 	*HitTestFaces(IPoint2 &m, ViewExp *vpt, bool completeAnalysis);
	HitRecord 	*HitTestAll (IPoint2 &m, ViewExp *vpt, int flags, bool completeAnalysis);
	void		DrawCutter (HWND hwnd);

	void Start ();
	void Update ();
	void Cancel ();
	void Accept ();

public:
	EditPolyCutProc (EPolyMod* e, IObjParam *i) : mpEPoly(e), ip(i), mStartIndex(-1) { }
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m );
};

class EditPolyCutCMode : public CommandMode {
private:
	ChangeFGObject	fgProc;	
	EditPolyCutProc proc;
	EditPolyMod		*mpEditPoly;

public:
	EditPolyCutCMode(EditPolyMod* e, IObjParam *i) : fgProc(e), proc(e,i), mpEditPoly(e) { }
	
	int 						Class() { return MODIFY_COMMAND; }
	int 						ID() { return CID_CUT; }
	MouseCallBack				*MouseProc(int *numPoints) {*numPoints=999; return &proc;}
	ChangeForegroundCallback	*ChangeFGProc() {return &fgProc;}
	BOOL						ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void 						EnterMode();
	void 						ExitMode();
};

class EditPolyQuickSliceProc : public MouseCallBack {
friend class EditPolyQuickSliceCMode;
private:
	EPolyMod	*mpEPoly;
	IObjParam	*mpInterface;
	IPoint2		mMouse1;
	IPoint2		mMouse2;
	Point3		mLocal1;
	Point3		mLocal2;
	Point3		mZDir;
	bool		mSlicing;
	bool		mSuspendPreview;
	Matrix3		mWorld2ModContext;
	Point3		mBoxCenter;

	void Start ();
	void Update ();
	void Cancel ();
	void Accept ();

public:
	EditPolyQuickSliceProc(EPolyMod *e, IObjParam *i) : mpEPoly(e), mpInterface(i), mSlicing(false) { }
	void	DrawSlicer (HWND hWnd);
	void	ComputeSliceParams ();
	int		proc(HWND hwnd, int msg, int point, int flags, IPoint2 m );
};

class EditPolyQuickSliceCMode : public CommandMode {
private:
	ChangeFGObject			fgProc;	
	EditPolyQuickSliceProc	proc;
	EditPolyMod				*mpEditPoly;

public:
	EditPolyQuickSliceCMode(EditPolyMod* e, IObjParam *i) : fgProc(e), proc(e,i), mpEditPoly(e) { }
	
	int							Class() { return MODIFY_COMMAND; }
	int							ID() { return CID_QUICKSLICE; }
	MouseCallBack				*MouseProc(int *numPoints) {*numPoints=3; return &proc;}
	ChangeForegroundCallback	*ChangeFGProc() {return &fgProc;}
	BOOL						ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void						EnterMode();
	void						ExitMode();
	bool						Slicing () { return proc.mSlicing; }
};

class EditPolyWeldMouseProc : public MouseCallBack {
	EPolyMod	*mpEditPoly;
	IObjParam	*ip;
	int			targ1;
	IPoint2		m1;
	IPoint2		oldm2;

public:
	EditPolyWeldMouseProc(EPolyMod *e, IObjParam *i) : mpEditPoly(e), ip(i), targ1(-1) { }
	
	void		DrawWeldLine (HWND hWnd, IPoint2 &m);
	bool		CanWeldEdges (int e1, int e2);
	bool		CanWeldVertices (int v1, int v2);
	HitRecord	*HitTest (IPoint2 &m, ViewExp *vpt);
	int			proc (HWND hwnd, int msg, int point, int flags, IPoint2 m);
	void		Reset() { targ1=-1; }
};

class EditPolyWeldCMode : public CommandMode {
	ChangeFGObject			fgProc;
	EditPolyWeldMouseProc	mproc;
	EditPolyMod				*mpEditPoly;

public:
	EditPolyWeldCMode(EditPolyMod* mod, IObjParam *i) :
	  fgProc(mod), mproc(mod,i), mpEditPoly(mod) { }
	
	int 						Class() { return MODIFY_COMMAND; }
	int 						ID() { return CID_WELD; }
	MouseCallBack				*MouseProc(int *numPoints) {*numPoints=2; return &mproc;}
	ChangeForegroundCallback	*ChangeFGProc() {return &fgProc; }
	BOOL 						ChangeFG( CommandMode *oldMode ) {return oldMode->ChangeFGProc() != &fgProc;}
	void 						EnterMode();
	void 						ExitMode();
};

class EditPolyHingeFromEdgeProc : public MouseCallBack {
	EPolyMod	*mpEPoly;
	IObjParam	*ip;
	IPoint2		mFirstPoint;
	IPoint2		mCurrentPoint;
	HitRecord	*mpHitRec;
	bool		mEdgeFound;
	float		mStartAngle;

	void Start ();
	void Update ();
	void Cancel ();
	void Accept ();

public:
	EditPolyHingeFromEdgeProc (EPolyMod *e, IObjParam *i) : mpEPoly(e), ip(i), mEdgeFound(false) { }
	
	HitRecord	*HitTestEdges (ViewExp *vpt);
	int			proc(HWND hwnd, int msg, int point, int flags, IPoint2 m );
	void		Reset () { Cancel (); }
};

class EditPolyHingeFromEdgeCMode : public CommandMode {
private:
	ChangeFGObject				fgProc;	
	EditPolyHingeFromEdgeProc	proc;
	EditPolyMod					*mpEditPoly;

public:
	EditPolyHingeFromEdgeCMode (EditPolyMod* e, IObjParam *i) : fgProc(e), proc(e,i), mpEditPoly(e) { }
	
	int							Class() { return MODIFY_COMMAND; }
	int							ID() { return CID_HINGE_FROM_EDGE; }
	MouseCallBack				*MouseProc(int *numPoints) {*numPoints=2; return &proc;}
	ChangeForegroundCallback	*ChangeFGProc() {return &fgProc;}
	BOOL						ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void						EnterMode();
	void						ExitMode();
};

class EditPolyPickHingeProc : public EditPolyPickEdgeMouseProc {
public:
	EditPolyPickHingeProc(EPolyMod* e, IObjParam *i) : EditPolyPickEdgeMouseProc(e,i) {}
	bool EdgePick (HitRecord *hr, float prop);
};

class EditPolyPickHingeCMode : public CommandMode {
private:
	ChangeFGObject			fgProc;
	EditPolyPickHingeProc	proc;
	EditPolyMod				*mpEditPoly;

public:
	EditPolyPickHingeCMode(EditPolyMod* e, IObjParam *i) : fgProc(e), proc(e,i), mpEditPoly(e) { }
	
	int							Class() { return MODIFY_COMMAND; }
	int							ID() { return CID_PICK_HINGE; }
	MouseCallBack				*MouseProc(int *numPoints) {*numPoints=1; return &proc;}
	ChangeForegroundCallback	*ChangeFGProc() {return &fgProc;}
	BOOL						ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void 						EnterMode();
	void 						ExitMode();
};

class EditPolyPickBridgeTargetProc : public MouseCallBack {
protected:
	EPolyMod	*mpEPoly;
	IObjParam	*ip;
	INode		*mpBridgeNode;
	int			mWhichEnd;

public:
	EditPolyPickBridgeTargetProc (EPolyMod* e, IObjParam *i, int end)
		: mpEPoly(e), ip(i), mWhichEnd(end) { }
	
	HitRecord	*HitTest(IPoint2 &m, ViewExp *vpt);
	void		FindCurrentBridgeNode ();
	int			proc (HWND hwnd, int msg, int point, int flags, IPoint2 m );
};

class EditPolyPickBridge1CMode : public CommandMode {
private:
	ChangeFGObject					fgProc;
	EditPolyPickBridgeTargetProc	proc;
	EditPolyMod						*mpEditPoly;

public:
	EditPolyPickBridge1CMode(EditPolyMod* e, IObjParam *i) : fgProc(e), proc(e,i,0), mpEditPoly(e) { }
	
	int 						Class() { return MODIFY_COMMAND; }
	int 						ID() { return CID_PICK_BRIDGE_1; }
	MouseCallBack				*MouseProc(int *numPoints) {*numPoints=1; return &proc;}
	ChangeForegroundCallback	*ChangeFGProc() {return &fgProc;}
	BOOL 						ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void 						EnterMode();
	void 						ExitMode();
};

class EditPolyPickBridge2CMode : public CommandMode {
private:
	ChangeFGObject					fgProc;
	EditPolyPickBridgeTargetProc	proc;
	EditPolyMod						*mpEditPoly;

public:
	EditPolyPickBridge2CMode(EditPolyMod* e, IObjParam *i) : fgProc(e), proc(e,i,1), mpEditPoly(e) { }
	
	int 						Class() { return MODIFY_COMMAND; }
	int 						ID() { return CID_PICK_BRIDGE_2; }
	MouseCallBack				*MouseProc(int *numPoints) {*numPoints=1; return &proc;}
	ChangeForegroundCallback	*ChangeFGProc() {return &fgProc;}
	BOOL 						ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void 						EnterMode();
	void 						ExitMode();
};

class EditPolyEditTriProc : public EditPolyConnectVertsMouseProc {
public:
	EditPolyEditTriProc(EPolyMod* e, IObjParam *i) : EditPolyConnectVertsMouseProc(e,i) { }
	void VertConnect ();
};

class EditPolyEditTriCMode : public CommandMode {
	ChangeFGObject		fgProc;
	EditPolyEditTriProc proc;
	EditPolyMod			*mpEditPoly;

public:
	EditPolyEditTriCMode(EditPolyMod* e, IObjParam *i) : fgProc(e), proc(e,i), mpEditPoly(e) { }
	
	int 						Class() { return MODIFY_COMMAND; }
	int 						ID() { return CID_EDITTRI; }
	MouseCallBack				*MouseProc(int *numPoints) {*numPoints=1; return &proc;}
	ChangeForegroundCallback	*ChangeFGProc() {return &fgProc;}
	BOOL 						ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void 						EnterMode();
	void 						ExitMode();
};

// We can't just use PickEdgeMouseProc for this, because we allow the user to pick either
// edges or diagonals.
class EditPolyTurnEdgeProc : public MouseCallBack {
private:
	EPolyMod *mpEPoly;
	IObjParam *ip;

public:
	EditPolyTurnEdgeProc(EPolyMod* e, IObjParam *i) : mpEPoly(e), ip(i) {}
	
	HitRecord	*HitTestEdges (IPoint2 &m, ViewExp *vpt);
	HitRecord	*HitTestDiagonals (IPoint2 &m, ViewExp *vpt);
	int			proc (HWND hwnd, int msg, int point, int flags, IPoint2 m );
};

class EditPolyTurnEdgeCMode : public CommandMode {
	ChangeFGObject			fgProc;
	EditPolyTurnEdgeProc	proc;
	EditPolyMod				*mpEditPoly;

public:
	EditPolyTurnEdgeCMode(EditPolyMod* e, IObjParam *i) : fgProc(e), proc(e,i), mpEditPoly(e) { }
	
	int							Class() { return MODIFY_COMMAND; }
	int							ID() { return CID_TURNEDGE; }
	MouseCallBack				*MouseProc(int *numPoints) {*numPoints=1; return &proc;}
	ChangeForegroundCallback	*ChangeFGProc() {return &fgProc;}
	BOOL						ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void 						EnterMode();
	void 						ExitMode();
};

// Restore Objects:

// Not really a restore object - just used in drag moves.
class TempMoveRestore {
friend EditPolyMod;
friend EditPolyData;

	Tab<Point3> init;
	BitArray	active;

public:
	TempMoveRestore (EditPolyMod *e, EditPolyData *d);
	void Restore (EditPolyMod *e, EditPolyData *d);
};

class EditPolySelectRestore : public RestoreObj {
	BitArray		mUndoSel;
	BitArray		mRedoSel;
	EditPolyMod		*mpMod;
	EditPolyData	*mpData;
	int				mLevel;

public:
	EditPolySelectRestore (EditPolyMod *pMod, EditPolyData *pData);
	EditPolySelectRestore (EditPolyMod *pMod, EditPolyData *pData, int level);
	
	void	After ();
	void	Restore(int isUndo);
	void	Redo();
	int		Size() { return 2*sizeof(void *) + 2*sizeof(BitArray) + sizeof(int); }
	void	EndHold() { mpData->ClearFlag (kEPDataHeld); }
	TSTR	Description() { return TSTR(_T("EditPolySelectRestore")); }
};

class EditPolyHideRestore : public RestoreObj {
	BitArray		mUndoHide;
	BitArray		mRedoHide;
	EditPolyMod		*mpMod;
	EditPolyData	*mpData;
	bool			mFaceLevel;

public:
	EditPolyHideRestore (EditPolyMod *pMod, EditPolyData *pData);
	EditPolyHideRestore (EditPolyMod *pMod, EditPolyData *pData, bool isFaceLevel);
	
	void 	After ();
	void 	Restore(int isUndo);
	void 	Redo();
	int		Size() { return 2*sizeof(void *) + 2*sizeof(BitArray) + sizeof(bool); }
	void	EndHold() { mpData->ClearFlag (kEPDataHeld); }
	TSTR	Description() { return TSTR(_T("EditPolyHideRestore")); }
};

class SelRingLoopRestore : public RestoreObj {
	BitArray		mSelectedEdges;
	EditPolyMod		*mpEditPoly;
	EditPolyData	*mpEditPolyData; 
	int				m_ringValue;
	int				m_loopValue;

public:
	SelRingLoopRestore(EditPolyMod *in_selMod, EditPolyData *in_selData) : 
		mpEditPoly(in_selMod), 
		mpEditPolyData(in_selData) 
		{

			mSelectedEdges	= mpEditPolyData->GetEdgeSel(); 
			m_ringValue		= mpEditPoly->getRingValue();
			m_loopValue		= mpEditPoly->getLoopValue();
	}

	void Restore(int isUndo) 
	{
		int			l_ringValue = mpEditPoly->getRingValue();
		int			l_loopValue = mpEditPoly->getLoopValue();
		BitArray	l_sel		= mpEditPolyData->GetEdgeSel();

		mpEditPolyData->SetEdgeSel(mSelectedEdges,mpEditPoly,0);
		mpEditPoly->setRingValue(m_ringValue);
		mpEditPoly->setLoopValue(m_loopValue);

		mSelectedEdges	= l_sel;
		m_ringValue		= l_ringValue;
		m_loopValue		= l_loopValue;

		mpEditPoly->LocalDataChanged();

	}

	void Redo() 
	{
		int			l_ringValue = mpEditPoly->getRingValue();
		int			l_loopValue = mpEditPoly->getLoopValue();
		BitArray	l_sel		= mpEditPolyData->GetEdgeSel();

		mpEditPolyData->SetEdgeSel(mSelectedEdges,mpEditPoly,0);
		mpEditPoly->setRingValue(m_ringValue);
		mpEditPoly->setLoopValue(m_loopValue);

		mSelectedEdges	= l_sel;
		m_ringValue		= l_ringValue;
		m_loopValue		= l_loopValue;

		mpEditPoly->LocalDataChanged();
	}

	TSTR Description() {return TSTR(_T("RingLoopRestore"));}
};




class AddOperationRestoreObj : public RestoreObj
{
private:
	EditPolyData	*mpData;
	EditPolyMod		*mpMod;
	PolyOperation	*mpOp;
public:
	AddOperationRestoreObj (EditPolyMod *pMod, EditPolyData *pData)
		: mpMod(pMod), mpData(pData), mpOp(NULL) { }
	~AddOperationRestoreObj () { if (mpOp) mpOp->DeleteThis(); }
	
	void Restore (int isUndo) {
		mpOp = mpData->PopOperation ();
		mpData->FreeCache();
		mpMod->NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
	}

	void Redo () {
		mpData->PushOperation (mpOp);
		mpOp = NULL;
		mpData->FreeCache ();
		mpMod->NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
	}

	int Size () { return 8; }

	TSTR Description() { return TSTR(_T("AddOperation")); }
};

class EditPolyFlagRestore : public RestoreObj
{
	EditPolyMod *	mpEPMod;
	EditPolyData *	mpEPData;
	int				mSelectionLevel;
	Tab<DWORD>		undo, redo;

public:
	EditPolyFlagRestore ( EditPolyMod *epMod, EditPolyData *epData, int selLevel);
	void Restore (int isUndo);
	void Redo ();
	TSTR Description () { return TSTR(_T("Edit Poly flag change restore")); }
	int Size () { return sizeof(void *) + sizeof(int) + 2*sizeof(Tab<DWORD>); }
};

//--- Parameter map/block descriptors -------------------------------

// ParamBlock2: Enumerate the parameter blocks:
enum { kEditPolyParams };

// Then enumerate all the rollouts we may need:
enum {	ep_animate, 
		ep_select, 
		ep_softsel, 
		ep_geom,
		ep_subobj, 
		ep_surface, 
		ep_settings, 
		ep_paintdeform,
		ep_face_smooth };

// Finally, do the operations themselves.

class LocalCreateData : public LocalPolyOpData
{
private:
	Tab<Point3> mNewVertex;
	Tab<int>	mNewFaceDegree;
	Tab<int *>	mNewFaceVertex;

public:
	LocalCreateData () { }
	LocalCreateData (LocalCreateData *from) { *this = *from; }
	~LocalCreateData ();

	void 	CreateVertex (Point3 newPoint) { mNewVertex.Append (1, &newPoint); }
	void 	RemoveLastVertex () {mNewVertex.Delete (mNewVertex.Count()-1, 1);}
	bool 	CreateFace (int *vertexArray, int degree);
	void 	RemoveLastFace ();

	int		NumVertices () { return mNewVertex.Count(); }
	int		NumFaces () { return mNewFaceDegree.Count(); }
	Point3	GetVertex (int i) { return mNewVertex[i]; }
	int		GetFaceDegree (int i) { return mNewFaceDegree[i]; }
	int		GetFaceVertex (int i, int j) { return mNewFaceVertex[i][j]; }
	int		*GetFaceVertices (int i) { return mNewFaceVertex[i]; }

	LocalCreateData & operator= (const LocalCreateData & from);

	int				OpID() { return ep_op_create; }
	void			DeleteThis () { delete this; }
	LocalPolyOpData *Clone () { return new LocalCreateData(this); }

	void RescaleWorldUnits (float f) {
		for (int i=0; i<mNewVertex.Count(); i++) mNewVertex[i] *= f;
	}

	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
};

class LocalCreateEdgeData : public LocalPolyOpData
{
private:
	Tab<int> mEdgeList;

public:
	LocalCreateEdgeData () { }
	LocalCreateEdgeData (LocalCreateEdgeData *pFrom) { mEdgeList = pFrom->mEdgeList; }

	void			AddEdge (int v1, int v2) { mEdgeList.Append (1, &v1, 1); mEdgeList.Append (1, &v2); }
	void			RemoveLastEdge () { mEdgeList.Delete (mEdgeList.Count()-2, 2); }

	int				NumEdges () { return mEdgeList.Count()/2; }
	int				GetVertex (int edgeIndex, int end) { return mEdgeList[edgeIndex*2+end]; }

	int				OpID() { return ep_op_create_edge; }
	void			DeleteThis () { delete this; }
	LocalPolyOpData *Clone () { return new LocalCreateEdgeData(this); }
	
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
};

class LocalAttachData : public LocalPolyOpData
{
private:
	Tab<MNMesh *>	 mMesh;
	Tab<int>		mMyMatLimit;

public:
	LocalAttachData () { }
	LocalAttachData (LocalAttachData & from) : mMyMatLimit(from.mMyMatLimit), mMesh(from.mMesh) { }

	void AppendNode (INode *node, EPolyMod *pMod, INode *myNode, TimeValue t, int attachMaterialType, bool condense);
	void RemoveLastNode ();

	MNMesh *PopMesh (int & matLimit);
	void	PushMesh (MNMesh *pMesh, int matLimit);

	int		NumMeshes () { return mMesh.Count(); }
	MNMesh	&GetMesh (int i) { return *(mMesh[i]); }
	int		GetMaterialLimit (int i) { return mMyMatLimit[i]; }

	int		OpID () { return ep_op_attach; }
	void	DeleteThis () { delete this; }

	LocalPolyOpData *Clone() { return new LocalAttachData (*this); }

	IOResult	Save (ISave *isave);
	IOResult	Load(ILoad *iload);
	void		RescaleWorldUnits (float f);
};

class LocalCutData : public LocalPolyOpData
{
private:
	Tab<int>	mStartLevel;
	Tab<int>	mStartIndex;
	Tab<Point3> mStart ;
	Tab<Point3> mEnd ;
	Tab<Point3> mNormal;
	int			mEndVertex;	// not saved.

public:
	LocalCutData (): mEndVertex(0) { }
	LocalCutData (LocalCutData *pFrom) : mStartLevel(pFrom->mStartLevel),
		mStartIndex(pFrom->mStartIndex), mStart(pFrom->mStart),
		mEnd(pFrom->mEnd), mNormal(pFrom->mNormal), mEndVertex(0) { }

	void 	AddCut (int startLevel, int startIndex, Point3 & start, Point3 & normal);
	void 	RemoveLast ();
	void 	SetEndPoint (Point3 &end) { mEnd[mEnd.Count()-1] = end; }

	int		NumCuts () { return mStartLevel.Count(); }
	int 	StartLevel (int i) { return mStartLevel[i]; }
	int 	StartIndex (int i) { return mStartIndex[i]; }
	Point3	Start (int i) { return mStart[i]; }
	Point3	End (int i) { return mEnd[i]; }
	Point3	Normal (int i) { return mNormal[i]; }

	// Keep track of the end of the cut, for the benefit of the cut mode.
	// (This is temporary data, not saved.  It's computed during PolyOpCut::Do.)
	void	SetEndVertex (int i) { mEndVertex = i; }
	int		GetEndVertex () { return mEndVertex; }

	int		OpID () { return ep_op_cut; }
	void	DeleteThis () { delete this; }

	LocalPolyOpData *Clone () { return new LocalCutData(this); }

	void RescaleWorldUnits (float f);

	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
};

class LocalSetDiagonalData : public LocalPolyOpData
{
private:
	Tab<int> mDiagList;

public:
	LocalSetDiagonalData () { }
	LocalSetDiagonalData (LocalSetDiagonalData *pFrom) { mDiagList = pFrom->mDiagList; }

	void	AddDiagonal (int v1, int v2) { mDiagList.Append (1, &v1, 1); mDiagList.Append (1, &v2); }
	void	RemoveLastDiagonal () { mDiagList.Delete (mDiagList.Count()-2, 2); }

	int		NumDiagonals () { return mDiagList.Count()/2; }
	int		GetVertex (int diagIndex, int end) { return mDiagList[diagIndex*2+end]; }

	int		OpID() { return ep_op_edit_triangulation; }
	void	DeleteThis () { delete this; }

	LocalPolyOpData *Clone () { return new LocalSetDiagonalData(this); }

	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
};

class LocalTurnEdgeData : public LocalPolyOpData {
private:
	// Data organization: we need to keep the edges and diagonals in the correct sequential order.
	// So we enter an edge with its index in mEdgeList, and a -1 in mFaceList,
	// and we enter a diag with its index in mEdgeList, and the face in mFaceList.
	Tab<int> mEdgeList;
	Tab<int> mFaceList;

public:
	LocalTurnEdgeData () { }
	LocalTurnEdgeData (LocalTurnEdgeData *pFrom) : mEdgeList(pFrom->mEdgeList), mFaceList(pFrom->mFaceList) { }

	void 	TurnEdge (int e) { mEdgeList.Append (1, &e); int negOne(-1); mFaceList.Append (1, &negOne); }
	void 	TurnDiagonal (int face, int diag) { mEdgeList.Append (1, &diag); mFaceList.Append (1, &face); }
	void 	RemoveLast () { int last = mEdgeList.Count()-1; mEdgeList.Delete (last,1); mFaceList.Delete(last,1); }

	int		NumTurns () { return mEdgeList.Count(); }
	bool 	IsEdge (int i) { return mFaceList[i]<0; }
	bool 	IsDiag (int i) { return mFaceList[i]>-1; }
	int 	GetEdge (int i) { return mEdgeList[i]; }
	int 	GetDiagonal (int i) { return mEdgeList[i]; }
	int 	GetFace (int i) { return mFaceList[i]; }

	int 	OpID() { return ep_op_turn_edge; }
	void	DeleteThis () { delete this; }

	LocalPolyOpData *Clone () { return new LocalTurnEdgeData(this); }

	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
};

class LocalHingeFromEdgeData : public LocalPolyOpData {
private:
	int mEdge;

public:
	LocalHingeFromEdgeData () : mEdge(-1) { }
	LocalHingeFromEdgeData (LocalHingeFromEdgeData *pFrom) : mEdge(pFrom->mEdge) { }

	void	SetEdge (int e) { mEdge = e; }
	int		GetEdge () { return mEdge; }

	int		OpID () { return ep_op_hinge_from_edge; }
	void	DeleteThis () { delete this; }

	LocalPolyOpData *Clone () { return new LocalHingeFromEdgeData (this); }

	IOResult Save (ISave *isave);
	IOResult Load (ILoad *iload);
};

class LocalBridgeBorderData : public LocalPolyOpData
{
public:
	LocalBridgeBorderData() { }
	int				OpID() { return ep_op_bridge_border; }
	LocalPolyOpData *Clone() { return new LocalBridgeBorderData(); }
	void			DeleteThis () { delete this; }
	IOResult		Save(ISave *isave) { return IO_OK; }
	IOResult		Load(ILoad *iload) { return IO_OK; }
};

class LocalBridgePolygonData : public LocalPolyOpData
{
public:
	LocalBridgePolygonData() { }
	int OpID() { return ep_op_bridge_polygon; }
	LocalPolyOpData *Clone() { return new LocalBridgePolygonData(); }
	void DeleteThis () { delete this; }
	IOResult Save(ISave *isave) { return IO_OK; }
	IOResult Load(ILoad *iload) { return IO_OK; }
};

// This class is abstract - no OpID() or Clone().
// It's used for LocalTargetWeldVertexData and LocalTargetWeldEdgeData.
class LocalTargetWeldData : public LocalPolyOpData
{
private:
	Tab<int> mStart;
	Tab<int> mTarget;
public:
	LocalTargetWeldData () { }
	LocalTargetWeldData (LocalTargetWeldData *pFrom) : mStart(pFrom->mStart), mTarget(pFrom->mTarget) { }

	void	AddWeld (int start, int target) { mStart.Append (1, &start); mTarget.Append (1, &target); }
	void	RemoveWeld () { int last = mStart.Count()-1; mStart.Delete(last,1); mTarget.Delete (last, 1); }

	int		NumWelds () { return mStart.Count(); }
	int		GetStart (int weld) { return mStart[weld]; }
	int		GetTarget (int weld) { return mTarget[weld]; }

	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
};

class LocalTargetWeldVertexData : public LocalTargetWeldData
{
public:
	LocalTargetWeldVertexData () { }
	LocalTargetWeldVertexData (LocalTargetWeldVertexData *pFrom) : LocalTargetWeldData(pFrom) { }
	
	int				OpID() { return ep_op_target_weld_vertex; }
	LocalPolyOpData *Clone () { return new LocalTargetWeldVertexData(this); }
	void			DeleteThis () { delete this; }
};

class LocalTargetWeldEdgeData : public LocalTargetWeldData
{
public:
	LocalTargetWeldEdgeData () { }
	LocalTargetWeldEdgeData (LocalTargetWeldEdgeData *pFrom) : LocalTargetWeldData(pFrom) { }
	
	int				OpID() { return ep_op_target_weld_edge; }
	LocalPolyOpData *Clone () { return new LocalTargetWeldEdgeData(this); }
	void			DeleteThis () { delete this; }
};

class LocalInsertVertexEdgeData : public LocalPolyOpData
{
private:
	Tab<float>	mProp;
	Tab<int>	mEdge;

public:
	LocalInsertVertexEdgeData () { }
	LocalInsertVertexEdgeData (LocalInsertVertexEdgeData *pFrom)
		: mEdge(pFrom->mEdge), mProp(pFrom->mProp) { }

	void	InsertVertex (int edge, float prop) { mEdge.Append (1, &edge); mProp.Append (1, &prop); }
	void	RemoveLast () { int last = mEdge.Count()-1; mEdge.Delete (last, 1); mProp.Delete (last, 1); }

	int		NumInserts () { return mEdge.Count(); }
	int		GetEdge (int which) { return mEdge[which]; }
	float	GetProp (int which) { return mProp[which]; }

	int		OpID() { return ep_op_insert_vertex_edge; }
	void	DeleteThis () { delete this; }

	LocalPolyOpData *Clone () { return new LocalInsertVertexEdgeData(this); }
	
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
};

class LocalInsertVertexFaceData : public LocalPolyOpData
{
private:
	Tab<int>		mFace;
	Tab<int>		mNumBary;
	Tab<float *>	mBary;

public:
	LocalInsertVertexFaceData () { }
	LocalInsertVertexFaceData (LocalInsertVertexFaceData *pFrom);
	~LocalInsertVertexFaceData ();

	void	InsertVertex (int face, Tab<float> & bary);
	void	RemoveLast ();

	int 	NumInserts () { return mFace.Count(); }
	int 	GetFace (int which) { return mFace[which]; }
	int 	GetNumBary (int which) { return mNumBary[which]; }
	float	*GetBary (int which) { return mBary[which]; }

	int		OpID() { return ep_op_insert_vertex_face; }
	void	DeleteThis () { delete this; }

	LocalPolyOpData *Clone () { return new LocalInsertVertexFaceData(this); }

	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
};

class LocalPaintDeformData : public LocalPolyOpData
{
private:
	MNMesh mVertexMesh;

public:
	LocalPaintDeformData () { }
	LocalPaintDeformData (const LocalPaintDeformData *pFrom) : mVertexMesh(pFrom->mVertexMesh) { }

	int		NumVertices () { return mVertexMesh.numv; }
	MNVert	*Vertices () { return mVertexMesh.v; }
	void	ClearVertices () { mVertexMesh.numv = 0; }
	void	SetOffsets (int count, Point3 *pOffsets, MNMesh *origMesh);
	MNMesh	*GetMesh() { return &mVertexMesh; }

	int		OpID() { return ep_op_paint_deform; }
	void	DeleteThis () { delete this; }

	LocalPolyOpData *Clone () { return new LocalPaintDeformData(this); }

	IOResult Save (ISave *isave);
	IOResult Load (ILoad *iload);

	void	RescaleWorldUnits (float f);
};

class LocalTransformData : public LocalPolyOpData
{
private:
	Tab<Point3> mOffset;
	Tab<Point3> mTempOffset;
	TSTR		mNodeName;

public:
	LocalTransformData () { }
	LocalTransformData (const LocalTransformData *pFrom) : mOffset(pFrom->mOffset), mNodeName(pFrom->mNodeName) { }

	int		NumOffsets () { return mOffset.Count(); }
	void	SetNumOffsets (int num) { mOffset.SetCount (num); }
	Point3	*OffsetPointer (int vertex) { return (mOffset.Count()>vertex) ? mOffset.Addr(vertex) : NULL; }
	Point3	&Offset(int vertex) { return mOffset[vertex]; }

	void 	SetTempOffset (Tab<Point3> & offset) { mTempOffset = offset; }
	void 	ClearTempOffset () { mTempOffset.SetCount (0, false); }
	bool 	HasTempOffset (int vertex) { return (mTempOffset.Count()>vertex) && (mTempOffset[vertex] != Point3(0,0,0)); }
	Point3	&TempOffset (int vertex) { return mTempOffset[vertex]; }
	int		NumTempOffsets () { return mTempOffset.Count(); }

	void	SetNodeName (TSTR & name) { mNodeName = name; }
	TSTR	&GetNodeName () { return mNodeName; }

	int NumVertices () { return (mTempOffset.Count() > mOffset.Count()) ? mTempOffset.Count() : mOffset.Count(); }
	Point3 CurrentOffset (int vertex) {
		Point3 ret((mOffset.Count()>vertex)?mOffset[vertex]:Point3(0,0,0));
		if (HasTempOffset(vertex)) ret += mTempOffset[vertex];
		return ret;
	}

	int		OpID () { return ep_op_transform; }
	void	DeleteThis () { delete this; }

	LocalPolyOpData *Clone () { return new LocalTransformData(this); }

	IOResult Save (ISave *isave);
	IOResult Load (ILoad *iload);

	void RescaleWorldUnits (float f);
};

class LocalBridgeEdgeData : public LocalPolyOpData
{
private:
	Tab<int> m_sourceEdges;
	Tab<int> m_targetEdges;

public:
	LocalBridgeEdgeData() { }
	
	int				OpID() { return ep_op_bridge_edge; }
	LocalPolyOpData *Clone() { return new LocalBridgeEdgeData(); }

	int 			NumSourceEdges () { return m_sourceEdges.Count() ;  }
	int 			NumTargetEdges () { return m_targetEdges.Count() ;  }

	void 			SetSourceEdges (Tab<int> & in_sourceEdges) { m_sourceEdges = in_sourceEdges; }
	void 			SettargetEdges (Tab<int> & in_targetEdges) { m_targetEdges = in_targetEdges; }

	void			DeleteThis () { delete this; }

	IOResult 		Save(ISave *isave) { return IO_OK; }
	IOResult 		Load(ILoad *iload) { return IO_OK; }
};

#endif
