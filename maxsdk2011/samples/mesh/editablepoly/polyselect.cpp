//*****************************************************************************
// Copyright 2009 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise accompanies 
// this software in either electronic or hard copy form.   
//
//*****************************************************************************
//
//	FILE: PolySelect.cpp
//  DESCRIPTION: A selection modifier for PolyMeshes
//	CREATED BY:	Steve Anderson, based on Mesh Select by Rolf Berteig
//
//	HISTORY: Created October 20, 1998
//*****************************************************************************

#include "EPoly.h"

#ifndef NO_MODIFIER_POLY_SELECT // JP Morel - July 23rd 2002

#include "iparamm2.h"
#include "MeshDLib.h"
#include "namesel.h"
#include "nsclip.h"
#include "istdplug.h"
#include "iColorMan.h"
#include "MaxIcon.h"
#include "../PolyPaint/PolyPaint.h"
#include "IActionItemOverrideManager.h"
#include "EditSoftSelectionMode.h"

class INodeTabAutoDispose : public INodeTab { //helper class for INodeTab
	public:
		~INodeTabAutoDispose() {DisposeTemporary();}
};

//helper macro to iterate through the modData objects for each node
#define FOR_EACH_MODDATA												\
	ModContextList _mcList;												\
	INodeTabAutoDispose _nodes;											\
	ip->GetModContexts(_mcList,_nodes);									\
	PolyMeshSelData* modData = NULL;											\
	for( int i=0; i<_mcList.Count(); i++ )								\
		if( (modData=(PolyMeshSelData*)_mcList[i]->localData)!=NULL )

#define FOR_EACH_MODDATA2												\
	mpIP->GetModContexts(_mcList,_nodes);								\
	for( int i=0; i<_mcList.Count(); i++ )								\
		if( (modData=(ProjectionModData*)_mcList[i]->localData)!=NULL )


//=============================================================================
//
//	Class PolyMeshSelMod
//
//=============================================================================

#define POLYMESH_SEL_CLASS_ID Class_ID(0x7ce5206b, 0x31181505)

static GenSubObjType SOT_Vertex(1);
static GenSubObjType SOT_Edge(2);
static GenSubObjType SOT_Border(9);
static GenSubObjType SOT_Face(4);
static GenSubObjType SOT_Element(5);

// Named selection set levels:
#define NS_VERTEX 0
#define NS_EDGE 1
#define NS_FACE 2
static int namedSetLevel[] = { NS_VERTEX, NS_VERTEX, NS_EDGE, NS_EDGE, NS_FACE, NS_FACE };
static int namedClipLevel[] = { CLIP_VERT, CLIP_VERT, CLIP_EDGE, CLIP_EDGE, CLIP_FACE, CLIP_FACE };

#define WM_UPDATE_CACHE		(WM_USER+0x28e)

// Table converts editable poly selection levels to MNMesh ones.
static int meshSelLevel[] = { MNM_SL_OBJECT, MNM_SL_VERTEX, MNM_SL_EDGE,
								MNM_SL_EDGE, MNM_SL_FACE, MNM_SL_FACE };

enum ePolySelectOps { epsop_sel_grow, epsop_sel_shrink, epsop_select_ring, epsop_select_loop };

#define IDC_SELVERTEX 0x3260
#define IDC_SELEDGE 0x3261
#define IDC_SELBORDER 0x3262
#define IDC_SELFACE 0x3263
#define IDC_SELELEMENT 0x3264

// PolyMeshSelMod flags:
#define PS_DISP_END_RESULT 0x0001

// PolyMeshSelMod References:
#define REF_PBLOCK 0
class PolySelectActionCB;
class PolyMeshSelMod : public Modifier, public IMeshSelect, public FlagUser, public MeshPaintHandler,public EditSSCB {	
public:
	DWORD selLevel;
	Tab<TSTR*> namedSel[3];		
	Tab<DWORD> ids[3];
	IParamBlock2 *pblock;

	static IObjParam *ip;
	static PolyMeshSelMod *editMod;
	static SelectModBoxCMode *selectMode;
	static BOOL updateCachePosted;
	static EditSSMode *	editSSMode;
	static Tab<ActionItem*> mOverrides;
	static PolySelectActionCB		*mpActions;			

	int			mRingSpinner; 
	int			mLoopSpinner; 
	bool		mAltKey; 
	bool		mCtrlKey;




	PolyMeshSelMod();

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) {s = GetString(IDS_POLYMESH_SEL_MOD);}  
	virtual Class_ID ClassID() { return POLYMESH_SEL_CLASS_ID;}		
	RefTargetHandle Clone(RemapDir& remap);
	TCHAR *GetObjectName() {return GetString(IDS_POLYMESH_SEL_MOD);}
	void* GetInterface(ULONG id) { if (id == I_MESHSELECT) return (IMeshSelect *)this; else return Modifier::GetInterface(id); }

	// From modifier
	ChannelMask ChannelsUsed()  {return PART_GEOM|PART_TOPO;}
	ChannelMask ChannelsChanged() {return PART_SELECT|PART_SUBSEL_TYPE|PART_GEOM|PART_TOPO;}	// There is no real select channel for PolyObjects; changes to selection affect vertices (PART_GEOM) and faces and edges (PART_TOPO)
	Class_ID InputType() {return polyObjectClassID;}
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	Interval LocalValidity(TimeValue t);
	Interval GetValidity (TimeValue t);
	BOOL DependOnTopology(ModContext &mc) {return TRUE;}

	// From BaseObject
	CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 
	void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);		
	int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc);
	void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);
	void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
	void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);

	void ActivateSubobjSel(int level, XFormModes& modes);
	void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE);
	void ClearSelection(int selLevel);
	void SelectAll(int selLevel);
	void InvertSelection(int selLevel);
	void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);

	void ShowEndResultChanged (BOOL showEndResult) { NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE); }

	BOOL SupportsNamedSubSels() {return TRUE;}
	void ActivateSubSelSet(TSTR &setName);
	void NewSetFromCurSel(TSTR &setName);
	void RemoveSubSelSet(TSTR &setName);
	void SetupNamedSelDropDown();
	int NumNamedSelSets();
	TSTR GetNamedSelSetName(int i);
	void SetNamedSelSetName(int i,TSTR &newName);
	void NewSetByOperator(TSTR &newName,Tab<int> &sets,int op);

	// NS: New SubObjType API
	int NumSubObjTypes();
	ISubObjType *GetSubObjType(int i);

	// From IMeshSelect
	DWORD GetSelLevel();
	void SetSelLevel(DWORD level);
	void LocalDataChanged();

	// IO
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
	IOResult LoadNamedSelChunk(ILoad *iload,int level);
	IOResult SaveLocalData(ISave *isave, LocalModData *ld);
	IOResult LoadLocalData(ILoad *iload, LocalModData **pld);

	int NumParamBlocks () { return 1; }
	IParamBlock2 *GetParamBlock (int i) { return pblock; }
	IParamBlock2 *GetParamBlockByID (short id) { return (pblock->ID() == id) ? pblock : NULL; }

	int NumRefs() { return 1; }
	RefTargetHandle GetReference(int i) { return pblock; }
	void SetReference(int i, RefTargetHandle rtarg) { pblock = (IParamBlock2 *) rtarg; }

	int NumSubs() {return 1;}
	Animatable* SubAnim(int i) { return GetReference(i); }
	TSTR SubAnimName(int i) {return GetString (IDS_PARAMETERS);}

	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
	   PartID& partID, RefMessage message);

	// russom - 1/20/04 - FID 14957
	void ConvertSelection (int epSelLevelFrom, int epSelLevelTo, bool requireAll);
	void ConvertSelectionToBorder (int epSelLevelFrom, int epSelLevelTo);
	void SelectionOp(int opcode);

	void UpdateSelLevelDisplay ();
	void SelectFrom(int from);
	void SetEnableStates();
	void SelectOpenEdges(int selType=0);
	void SelectByMatID(int id);
	void SetNumSelLabel();		
	void UpdateCache(TimeValue t);
	void InvalidateVDistances ();
	void SetSoftSelEnables ();
	void InvalidateDialogElement (int elem);
	void GetSoftSelData( GenSoftSelData& softSelData, TimeValue t );

	// Local methods for handling named selection sets
	int FindSet(TSTR &setName,int level);		
	DWORD AddSet(TSTR &setName,int level);
	void RemoveSet(TSTR &setName,int level);
	void UpdateSetNames ();	// Reconciles names with PolyMeshSelData.
	void ClearSetNames();
	void NSCopy();
	void NSPaste();
	BOOL GetUniqueSetName(TSTR &name);
	int SelectNamedSet();
	void UpdateNamedSelDropDown ();

	// From MeshPaintHandler
	void		PrePaintStroke (PaintModeID paintMode) { }
	void		GetPaintHosts( Tab<MeshPaintHost*>& hosts, Tab<INode*>& nodes );
	float		GetPaintValue( PaintModeID paintMode );
	void		SetPaintValue( PaintModeID paintMode, float value );
	PaintAxisID	GetPaintAxis( PaintModeID paintMode ) {return PAINTAXIS_NONE;} //not supported
	void		SetPaintAxis( PaintModeID paintMode, PaintAxisID axis ) {} //not supported
	// per-host data...
	MNMesh*		GetPaintObject( MeshPaintHost* host );
	void		GetPaintInputSel( MeshPaintHost* host, FloatTab& selValues );
	void		GetPaintInputVertPos( MeshPaintHost* host, Point3Tab& vertPos );
	void		GetPaintMask( MeshPaintHost* host, BitArray& sel, FloatTab& softSel ) {} //not supported
	// paint actions
	void		ApplyPaintSel( MeshPaintHost* host );
	void		ApplyPaintDeform( MeshPaintHost* host,  BitArray *partialRevert ) {} //not supported
	void		RevertPaintDeform( MeshPaintHost* host,  BitArray *partialRevert ) {} //not supported
	// notifications
	void		OnPaintModeChanged( PaintModeID prevMode, PaintModeID curMode );
	void		OnPaintDataActivate( PaintModeID paintMode, BOOL onOff );
	void		OnPaintDataRedraw( PaintModeID prevMode );
	void		OnPaintBrushChanged();


	// Event handlers
	void OnParamSet(PB2Value& v, ParamID id, int tabIndex, TimeValue t);
	void OnParamGet(PB2Value& v, ParamID id, int tabIndex, TimeValue t, Interval &valid);
	void OnPaintBtn( int paintMode, TimeValue t );

	// Resource strings needed
	TCHAR *GetPaintUndoString (PaintModeID mode);

	//from EditSSCB
	void DoAccept(TimeValue t);
	void SetPinch(TimeValue t, float pinch);
	void SetBubble(TimeValue t, float bubble);
	float GetPinch(TimeValue t);
	float GetBubble(TimeValue t);
	void SetFalloff(TimeValue t,float falloff);
	float GetFalloff(TimeValue t);

	void EditSoftSelectionMode();


	void 		setAltKey( bool inVal) {mAltKey = inVal; };
	void 		setCtrlKey( bool inVal) {mCtrlKey = inVal; };
	bool 		getCtrlKey() { return mCtrlKey;};
	bool 		getAltKey() { return mAltKey;};

};
EditSSMode *	PolyMeshSelMod::editSSMode = NULL;
PolySelectActionCB *PolyMeshSelMod::mpActions = NULL;
Tab<ActionItem*> PolyMeshSelMod::mOverrides;

class PolyMeshSelData : public LocalModData, public IMeshSelectData, public MeshPaintHost {
private:
	// Temp data used only for soft selections.
	MNTempData *temp;

	Interval vdValid;	// validity interval of vertex distances from selection.
	BitArray *mpNewSelection;
	MNMeshSelectionConverter mSelConv;

public:
	// LocalModData
	void* GetInterface(ULONG id) { if (id == I_MESHSELECTDATA) return(IMeshSelectData*)this; else return LocalModData::GetInterface(id); }

	// Selection sets
	BitArray vertSel;
	BitArray faceSel;
	BitArray edgeSel;

	// Lists of named selection sets
	GenericNamedSelSetList vselSet;
	GenericNamedSelSetList fselSet;
	GenericNamedSelSetList eselSet;

	BOOL held;
	MNMesh *mesh;

	PolyMeshSelData (MNMesh &mesh);
	PolyMeshSelData () : held(false), mesh(NULL), temp(NULL), mpNewSelection(NULL) { vdValid.SetEmpty (); }
	~PolyMeshSelData() { FreeCache(); if (mpNewSelection) delete mpNewSelection; }
	LocalModData *Clone();
	MNMesh *GetMesh() {return mesh;}
	MNTempData *GetTempData() {return temp;}
	void SetCache(MNMesh &mesh);
	void FreeCache();
	void SynchBitArrays();

	void SelVertByFace();
	void SelVertByEdge();
	void SelFaceByVert();
	void SelFaceByEdge();
	void SelElementByVert();
	void SelElementByEdge();
	void SelBorderByVert();
	void SelBorderByFace();
	void SelEdgeByVert();
	void SelEdgeByFace();

	// russom - 1/20/04 - FID 1495
	void GrowSelection(IMeshSelect *imod, int level);
	void ShrinkSelection(IMeshSelect *imod, int level);
	void SelectEdgeRing(IMeshSelect *imod);
	void SelectEdgeLoop(IMeshSelect *imod);

	//laurent r - 3/05 
	void SelectEdgeLoopShift(IMeshSelect *imod, const int in_loop_shift, const bool in_remove, const bool in_add);
	void SelectEdgeRingShift(IMeshSelect *imod, const int in_ring_shift, const bool in_remove, const bool in_add);

	// From IMeshSelectData:
	BitArray GetVertSel() { return vertSel; }
	BitArray GetFaceSel() { return faceSel; }
	BitArray GetEdgeSel() { return edgeSel; }

	void SetVertSel(BitArray &set, IMeshSelect *imod, TimeValue t);
	void SetFaceSel(BitArray &set, IMeshSelect *imod, TimeValue t);
	void SetEdgeSel(BitArray &set, IMeshSelect *imod, TimeValue t);

	GenericNamedSelSetList & GetNamedVertSelList () { return vselSet; }
	GenericNamedSelSetList & GetNamedEdgeSelList () { return eselSet; }
	GenericNamedSelSetList & GetNamedFaceSelList () { return fselSet; }
	GenericNamedSelSetList & GetNamedSel (int nsl) {
		if (nsl==NS_VERTEX) return vselSet;
		if (nsl==NS_EDGE) return eselSet;
		return fselSet;
	}

	// New selection methods:
	void SetupNewSelection (int selLevel);
	BitArray *GetNewSelection () { return mpNewSelection; }
	void TranslateNewSelection (int selLevelFrom, int selLevelTo);
	void ApplyNewSelection (int selLevel, bool keepOld=false, bool invert=false, bool select=true);
	void SetConverterFlag (DWORD flag, bool value) { mSelConv.SetFlag (flag, value); }

	void InvalidateVDistances () { vdValid.SetEmpty (); }
	void GetWeightedVertSel (int nv, float *sel, TimeValue t, PolyObject *pobj,
		float falloff, float pinch, float bubble, int edgeDist, bool ignoreBackfacing,
		Interval & eDistValid, bool lockedSoftsel);
};












// russom - 1/20/04 - FID 1495
class PSComponentFlagRestore : public RestoreObj {
	PolyMeshSelMod *mod;
	PolyMeshSelData *data;
	int sl;
	Tab<DWORD> undo, redo;

public:
	PSComponentFlagRestore (PolyMeshSelMod *m, PolyMeshSelData *d, int selLevel);
	void Restore (int isUndo);
	void Redo ();
	TSTR Description () { return TSTR(_T("Component flag change restore")); }
	int Size () { return sizeof(void *) + sizeof(int) + 2*sizeof(Tab<DWORD>); }
};

class SelectRestore : public RestoreObj {
public:
	BitArray usel, rsel;
	BitArray *sel;
	PolyMeshSelMod *mod;
	PolyMeshSelData *d;
	int level;

	SelectRestore(PolyMeshSelMod *m, PolyMeshSelData *d);
	SelectRestore(PolyMeshSelMod *m, PolyMeshSelData *d, int level);
	void Restore(int isUndo);
	void Redo();
	int Size() { return 1; }
	void EndHold() {d->held=FALSE;}
	TSTR Description() { return TSTR(_T("SelectRestore")); }
};

class AddSetRestore : public RestoreObj {
public:
	BitArray set;		
	DWORD id;
	TSTR name;
	GenericNamedSelSetList *setList;

	AddSetRestore (GenericNamedSelSetList *sl, DWORD i, TSTR &n) : setList(sl), id(i), name(n) { }
	void Restore(int isUndo) {
		set  = *setList->GetSet (id);
		setList->RemoveSet(id);
	}
	void Redo() { setList->InsertSet (set, id, name); }
			
	TSTR Description() {return TSTR(_T("Add NS Set"));}
};

class AddSetNameRestore : public RestoreObj {
public:		
	TSTR name;
	DWORD id;
	int index;	// location in list.

	PolyMeshSelMod *et;
	Tab<TSTR*> *sets;
	Tab<DWORD> *ids;

	AddSetNameRestore (PolyMeshSelMod *e, DWORD idd, Tab<TSTR*> *s,Tab<DWORD> *i) : et(e), sets(s), ids(i), id(idd) { }
	void Restore(int isUndo);
	void Redo();				
	TSTR Description() {return TSTR(_T("Add Set Name"));}
};

void AddSetNameRestore::Restore(int isUndo) {
	int sct = sets->Count();
	for (index=0; index<sct; index++) if ((*ids)[index] == id) break;
	if (index >= sct) return;

	name = *(*sets)[index];
	delete (*sets)[index];
	sets->Delete (index, 1);
	ids->Delete (index, 1);
	if (et->ip) et->ip->NamedSelSetListChanged();
}

void AddSetNameRestore::Redo() {
	TSTR *nm = new TSTR(name);
	if (index < sets->Count()) {
		sets->Insert (index, 1, &nm);
		ids->Insert (index, 1, &id);
	} else {
		sets->Append (1, &nm);
		ids->Append (1, &id);
	}
	if (et->ip) et->ip->NamedSelSetListChanged();
}

class DeleteSetRestore : public RestoreObj {
public:
	BitArray set;
	DWORD id;
	TSTR name;
	GenericNamedSelSetList *setList;

	DeleteSetRestore(GenericNamedSelSetList *sl,DWORD i, TSTR & n) {
		setList = sl;
		id = i;
		BitArray *ptr = setList->GetSet(id);
		if (ptr) set = *ptr;
		name = n;
	}
	void Restore(int isUndo) { setList->InsertSet(set, id, name); }
	void Redo() { setList->RemoveSet(id); }
	TSTR Description() {return TSTR(_T("Delete Set"));}
};

class DeleteSetNameRestore : public RestoreObj {
public:		
	TSTR name;
	DWORD id;
	PolyMeshSelMod *et;
	Tab<TSTR*> *sets;
	Tab<DWORD> *ids;

	DeleteSetNameRestore(Tab<TSTR*> *s,PolyMeshSelMod *e,Tab<DWORD> *i, DWORD id) {
		sets = s; et = e;
		this->id = id;
		ids = i;
		int index = -1;
		for (int j=0; j<sets->Count(); j++) {
			if ((*ids)[j]==id) {
				index = j;
				break;
				}
			}
		if (index>=0) {
			name = *(*sets)[index];
			}
		}   		
	void Restore(int isUndo) {			
		TSTR *nm = new TSTR(name);			
		//sets->Insert(index,1,&nm);
		sets->Append(1,&nm);
		ids->Append(1,&id);
		if (et->ip) et->ip->NamedSelSetListChanged();
		}
	void Redo() {
		int index = -1;
		for (int j=0; j<sets->Count(); j++) {
			if ((*ids)[j]==id) {
				index = j;
				break;
				}
			}
		if (index>=0) {
			sets->Delete(index,1);
			ids->Delete(index,1);
			}
		//sets->Delete(index,1);
		if (et->ip) et->ip->NamedSelSetListChanged();
		}
			
	TSTR Description() {return TSTR(_T("Delete Set Name"));}
};

class SetNameRestore : public RestoreObj {
public:
	TSTR undo, redo;
	DWORD id;
	Tab<TSTR*> *sets;
	Tab<DWORD> *ids;
	PolyMeshSelMod *et;
	SetNameRestore(Tab<TSTR*> *s,PolyMeshSelMod *e,Tab<DWORD> *i,DWORD id) {
		this->id = id;
		ids = i;
		sets = s; et = e;
		int index = -1;
		for (int j=0; j<sets->Count(); j++) {
			if ((*ids)[j]==id) {
				index = j;
				break;
				}
			}
		if (index>=0) {
			undo = *(*sets)[index];
			}
		}

	void Restore(int isUndo) {
		int index = -1;
		for (int j=0; j<sets->Count(); j++) {
			if ((*ids)[j]==id) {
				index = j;
				break;
				}
			}
		if (index>=0) {
			redo = *(*sets)[index];
			*(*sets)[index] = undo;
			}			
		if (et->ip) et->ip->NamedSelSetListChanged();
		}
	void Redo() {
		int index = -1;
		for (int j=0; j<sets->Count(); j++) {
			if ((*ids)[j]==id) {
				index = j;
				break;
			}
		}
		if (index>=0) {
			*(*sets)[index] = redo;
		}
		if (et->ip) et->ip->NamedSelSetListChanged();
	}
			
	TSTR Description() {return TSTR(_T("Set Name"));}
};

class SelRingLoopRestore : public RestoreObj {
	BitArray		mSelectedEdges;
	PolyMeshSelMod	*mpEditPoly;
	PolyMeshSelData *mpEditPolyData; 
	int 			mRingValue;
	int				mLoopValue; 

public:
	SelRingLoopRestore(PolyMeshSelMod *in_selMod, PolyMeshSelData *in_seleData) : 
	mpEditPoly(in_selMod), mpEditPolyData(in_seleData) 
	{
		mSelectedEdges	= mpEditPolyData->GetEdgeSel(); 
		mRingValue		= mpEditPoly->mRingSpinner;
		mLoopValue		= mpEditPoly->mLoopSpinner;
	}

	void Restore(int isUndo) 
	{
		BitArray	l_sel		= mpEditPolyData->GetEdgeSel();
		int			l_ringVal	= mpEditPoly->mRingSpinner;
		int			l_loopVal	= mpEditPoly->mLoopSpinner;

		mpEditPolyData->SetEdgeSel(mSelectedEdges,mpEditPoly,0);
		
		mpEditPoly->mRingSpinner	= mRingValue;
		mpEditPoly->mLoopSpinner	= mLoopValue;

		mSelectedEdges				= l_sel;
		mRingValue					= l_ringVal;
		mLoopValue					= l_loopVal;

		mpEditPoly->LocalDataChanged();
	};

	void Redo() 
	{
		BitArray	l_sel		= mpEditPolyData->GetEdgeSel();
		int			l_ringVal	= mpEditPoly->mRingSpinner;
		int			l_loopVal	= mpEditPoly->mLoopSpinner;

		mpEditPolyData->SetEdgeSel(mSelectedEdges,mpEditPoly,0);

		mpEditPoly->mRingSpinner	= mRingValue;
		mpEditPoly->mLoopSpinner	= mLoopValue;

		mSelectedEdges				= l_sel;
		mRingValue					= l_ringVal;
		mLoopValue					= l_loopVal;

		mpEditPoly->LocalDataChanged();

	};
	TSTR Description() {return TSTR(_T("RingLoopRestore"));}
};



//--- Accelerator table for Poly Select -------------------

const ActionTableId kPolySelectActionID = 0x2993ebf;

class PolySelectActionCB : public ActionCallback {
	PolyMeshSelMod *mpMod;
public:
	PolySelectActionCB (PolyMeshSelMod *m) : mpMod(m) { }
	BOOL ExecuteAction(int id);
};

BOOL PolySelectActionCB::ExecuteAction (int id) {
	switch (id) {
	case ID_PS_EDIT_SOFT_SELECTION_MODE:
		mpMod->EditSoftSelectionMode();
		return true;
	}
	return false;
}

const int kNumPolySelectActions = 1;

static ActionDescription polySelectActions[] = {
	ID_PS_EDIT_SOFT_SELECTION_MODE,
		IDS_EDIT_SOFT_SELECTION_MODE,
		IDS_EDIT_SOFT_SELECTION_MODE,
		IDS_POLYSELECT

};

ActionTable* GetPolySelectActions() {
	TSTR name = GetString (IDS_POLYSELECT);
	HACCEL hAccel = LoadAccelerators (hInstance, MAKEINTRESOURCE (IDR_ACCEL_POLYSELECT));
	ActionTable* pTab = NULL;
	pTab = new ActionTable (kPolySelectActionID, kPolySelectActionID,
		name, hAccel, kNumPolySelectActions, polySelectActions, hInstance);
    GetCOREInterface()->GetActionManager()->RegisterActionContext(kPolySelectActionID, name.data());

	IActionItemOverrideManager *actionOverrideMan = static_cast<IActionItemOverrideManager*>(GetCOREInterface(IACTIONITEMOVERRIDEMANAGER_INTERFACE ));
	if(actionOverrideMan)
	{
		//register the action items
		TSTR desc;
		ActionItem *item = pTab->GetAction(ID_PS_EDIT_SOFT_SELECTION_MODE);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}
	}

	return pTab;
}

//--- ClassDescriptor and class vars ---------------------------------

#define SEL_OBJECT	0
#define SEL_VERTEX	1
#define SEL_EDGE	2
#define SEL_BORDER	3
#define SEL_FACE	4
#define SEL_ELEMENT	5
#define SEL_CURRENT	6

IObjParam *PolyMeshSelMod::ip              = NULL;
PolyMeshSelMod *PolyMeshSelMod::editMod         = NULL;
SelectModBoxCMode *PolyMeshSelMod::selectMode      = NULL;
BOOL PolyMeshSelMod::updateCachePosted = FALSE;

class PolySelectClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new PolyMeshSelMod; }
	const TCHAR *	ClassName() { return GetString(IDS_POLYMESH_SEL_MOD); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return POLYMESH_SEL_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_MOD_CATEGORY);}
	const TCHAR*	InternalName() { return _T("PolyMesh Select"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE HInstance() { return hInstance; }
	int             NumActionTables(){return 1;}
	ActionTable *GetActionTable(int i) {return GetPolySelectActions ();}
};

static PolySelectClassDesc pSelDesc;
ClassDesc2* GetPolySelectDesc() {return &pSelDesc;}

class SelectProc : public ParamMap2UserDlgProc {
public:
	PolyMeshSelMod *mod;
	SelectProc () { mod = NULL; }
	void SetEnables (HWND hWnd);
	void UpdateSelLevelDisplay (HWND hWnd);
	INT_PTR DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
	void DeleteThis() { }
};

static SelectProc theSelectProc;

class SoftSelProc : public ParamMap2UserDlgProc {
public:
	PolyMeshSelMod *mod;
	SoftSelProc () { mod = NULL; }
	void SetEnables (HWND hWnd);
	void DrawCurve (HWND hWnd);
	INT_PTR DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
	void DeleteThis() { }
};

static SoftSelProc theSoftSelProc;

class PolyMeshSelModPBAccessor : public PBAccessor {
	public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
		{ ((PolyMeshSelMod*)owner)->OnParamSet( v, id, tabIndex, t ); }
	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid)
		{ ((PolyMeshSelMod*)owner)->OnParamGet( v, id, tabIndex, t, valid ); }
};
static PolyMeshSelModPBAccessor thePolyMeshSelModPBAccessor;

// Table to convert selLevel values to mesh selLevel flags.
const int meshLevel[] = { MNM_SL_OBJECT, MNM_SL_VERTEX, MNM_SL_EDGE, MNM_SL_EDGE, MNM_SL_FACE, MNM_SL_FACE};

// Get display flags based on selLevel.
const DWORD levelDispFlags[] = {0, MNDISP_VERTTICKS|MNDISP_SELVERTS,
		MNDISP_SELEDGES, MNDISP_SELEDGES, MNDISP_SELFACES, MNDISP_SELFACES};

// For hit testing...
const int hitLevel[] = { 0, SUBHIT_MNVERTS, SUBHIT_MNEDGES,
				SUBHIT_MNEDGES|SUBHIT_OPENONLY,
				SUBHIT_MNFACES, SUBHIT_MNFACES };

// Parameter block IDs:
// Blocks themselves:
enum { ps_pblock };
// Parameter maps:
enum { ps_map_main, ps_map_softsel };
// Parameters in first block:
enum { ps_use_softsel, ps_use_edist, ps_edist, ps_affect_backfacing,
		ps_falloff, ps_pinch, ps_bubble, ps_by_vertex, ps_ignore_backfacing,
		ps_matid, ps_lock_softsel, ps_paintsel_value, ps_paintsel_size, 
		ps_paintsel_strength,ps_loop_sel,ps_ring_sel, ps_paintsel_mode };

static ParamBlockDesc2 polysel_softsel_desc ( ps_pblock,
									_T("polySelectSoftSelection"),
									IDS_SOFTSEL, &pSelDesc,
									P_AUTO_CONSTRUCT | P_AUTO_UI | P_MULTIMAP,
									REF_PBLOCK,
	//rollout descriptions
	2,
	ps_map_main, IDD_POLYMESH_SELECT, IDS_PARAMETERS, 0, 0, NULL,
	ps_map_softsel, IDD_SOFTSEL, IDS_SOFTSEL, 0, APPENDROLL_CLOSED, NULL,

	// params
	ps_use_softsel, _T("useSoftSelection"), TYPE_BOOL, P_RESET_DEFAULT, IDS_USE_SOFTSEL,
		p_default, FALSE,
		p_ui, ps_map_softsel, TYPE_SINGLECHEKBOX, IDC_USE_SOFTSEL,
		end,

	ps_use_edist, _T("softselUseEdgeDistance"), TYPE_BOOL, P_RESET_DEFAULT, IDS_USE_EDIST,
		p_default, FALSE, // Preserve selection
		p_ui, ps_map_softsel, TYPE_SINGLECHEKBOX, IDC_USE_EDIST,
		end,

	ps_edist, _T("softselEdgeDist"), TYPE_INT, P_RESET_DEFAULT|P_ANIMATABLE, IDS_EDGE_DIST,
		p_default, 1,
		p_range, 1, 9999999,
		p_ui, ps_map_softsel, TYPE_SPINNER, EDITTYPE_POS_INT, IDC_EDIST, IDC_EDIST_SPIN, .2f,
		end,

	ps_affect_backfacing, _T("softselAffectBackfacing"), TYPE_BOOL, P_RESET_DEFAULT, IDS_AFFECT_BACKFACING,
		p_default, true,
		p_ui, ps_map_softsel, TYPE_SINGLECHEKBOX, IDC_AFFECT_BACKFACING,
		end,

	ps_falloff, _T("softselFalloff"), TYPE_WORLD, P_RESET_DEFAULT|P_ANIMATABLE, IDS_FALLOFF,
		p_default, 20.0f,
		p_range, 0.0f, 999999.f,
		p_ui, ps_map_softsel, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
			IDC_FALLOFF, IDC_FALLOFFSPIN, SPIN_AUTOSCALE,
		end,

	ps_pinch, _T("softselPinch"), TYPE_FLOAT, P_RESET_DEFAULT|P_ANIMATABLE, IDS_PINCH,
		p_default, 0.0f,
		p_range, -999999.f, 999999.f,
		p_ui, ps_map_softsel, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_PINCH, IDC_PINCHSPIN, SPIN_AUTOSCALE,
		end,

	ps_bubble, _T("softselBubble"), TYPE_FLOAT, P_RESET_DEFAULT|P_ANIMATABLE, IDS_BUBBLE,
		p_default, 0.0f,
		p_range, -999999.f, 999999.f,
		p_ui, ps_map_softsel, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_BUBBLE, IDC_BUBBLESPIN, SPIN_AUTOSCALE,
		end,

	ps_lock_softsel, _T("lockSoftSel"), TYPE_BOOL, P_TRANSIENT, IDS_LOCK_SOFTSEL,
		p_accessor,		&thePolyMeshSelModPBAccessor,
		p_ui, ps_map_softsel, TYPE_SINGLECHEKBOX, IDC_LOCK_SOFTSEL,
		end,

	ps_paintsel_mode, _T("paintSelMode"), TYPE_INT, P_TRANSIENT, IDS_PAINTSEL_MODE,
		p_accessor,		&thePolyMeshSelModPBAccessor,
		p_default, PAINTMODE_OFF,
		end,

	ps_paintsel_value, _T("paintSelValue"), TYPE_FLOAT, 0, IDS_PAINTSEL_VALUE,
		p_accessor,		&thePolyMeshSelModPBAccessor,
		p_default, 1.0f,
		p_range, 0.0f, 1.0f,
		p_ui, ps_map_softsel, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_PAINTSEL_VALUE_EDIT, IDC_PAINTSEL_VALUE_SPIN, 0.01f,
		end,

	ps_paintsel_size, _T("paintSelSize"), TYPE_FLOAT, P_TRANSIENT, IDS_PAINTSEL_SIZE,
		p_accessor,		&thePolyMeshSelModPBAccessor,
		p_range, 0.0f, 999999.f,
		p_ui, ps_map_softsel, TYPE_SPINNER, EDITTYPE_UNIVERSE,
			IDC_PAINTSEL_SIZE_EDIT, IDC_PAINTSEL_SIZE_SPIN, SPIN_AUTOSCALE,
		end,

	ps_paintsel_strength, _T("paintSelStrength"), TYPE_FLOAT, P_TRANSIENT, IDS_PAINTSEL_STRENGTH,
		p_accessor,		&thePolyMeshSelModPBAccessor,
		p_range, 0.0f, 1.0f,
		p_ui, ps_map_softsel, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_PAINTSEL_STRENGTH_EDIT, IDC_PAINTSEL_STRENGTH_SPIN, 0.01f,
		end,

	ps_by_vertex, _T("byVertex"), TYPE_BOOL, P_RESET_DEFAULT, IDS_SEL_BY_VERTEX,
		p_default, false,
		p_ui, ps_map_main, TYPE_SINGLECHEKBOX, IDC_MS_SEL_BYVERT,
		end,

	ps_ignore_backfacing, _T("ignoreBackfacing"), TYPE_BOOL, P_RESET_DEFAULT, IDS_IGNORE_BACKFACING,
		p_default, false,
		p_ui, ps_map_main, TYPE_SINGLECHEKBOX, IDC_MS_IGNORE_BACKFACES,
		end,

	ps_matid, _T("materialID"), TYPE_INT, P_RESET_DEFAULT|P_TRANSIENT, IDS_MATERIAL_ID,
		p_default, 1,
		p_range, 1, 65535,
		p_ui, ps_map_main, TYPE_SPINNER, EDITTYPE_INT,
			IDC_MS_MATID, IDC_MS_MATIDSPIN, .5f,
		end,
	end
);

//--- MeshSel mod methods -------------------------------

PolyMeshSelMod::PolyMeshSelMod() {
	selLevel			= SEL_OBJECT;
	pblock				= NULL;
	mRingSpinner		= 0; 
	mLoopSpinner		= 0; 
	mAltKey				= false; 
	mCtrlKey			= false; 
	
	pSelDesc.MakeAutoParamBlocks(this);

}

RefTargetHandle PolyMeshSelMod::Clone(RemapDir& remap) {
	PolyMeshSelMod *mod = new PolyMeshSelMod();
	mod->selLevel = selLevel;
	mod->ReplaceReference (0, remap.CloneRef(pblock));
	for (int i=0; i<3; i++)
	{
		mod->namedSel[i].SetCount (namedSel[i].Count());
		for (int j=0; j<namedSel[i].Count(); j++) mod->namedSel[i][j] = new TSTR (*namedSel[i][j]);
		mod->ids[i].SetCount (ids[i].Count());
		for (int j=0; j<ids[i].Count(); j++) mod->ids[i][j] = ids[i][j];
	}

	mod->paintSelActive = paintSelActive;

	BaseClone(this, mod, remap);
	return mod;
}

Interval PolyMeshSelMod::LocalValidity(TimeValue t)
{
  // aszabo|feb.05.02 If we are being edited, return NEVER 
	// to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;
	return GetValidity(t);
}

Interval PolyMeshSelMod::GetValidity (TimeValue t) {
	Interval ret = FOREVER;
	int useAR;
	pblock->GetValue (ps_use_softsel, t, useAR, ret);
	if (!useAR) return ret;
	int useEDist, eDist;
	pblock->GetValue (ps_use_edist, t, useEDist, ret);
	if (useEDist) pblock->GetValue (ps_edist, t, eDist, ret);
	float f, p, b;
	pblock->GetValue (ps_falloff, t, f, ret);
	pblock->GetValue (ps_pinch, t, p, ret);
	pblock->GetValue (ps_bubble, t, b, ret);
	return ret;
}

void PolyMeshSelData::GetWeightedVertSel (int nv, float *wvs, TimeValue t, PolyObject *pobj,
										  float falloff, float pinch, float bubble, int edist, bool ignoreBackfacing,
										  Interval & edistValid, bool lockedSoftsel) {
	if (!temp) temp = new MNTempData;
	temp->SetMesh (mesh);

	if( lockedSoftsel )
	{
		// There's a chance that our locked soft selection was based on a different number of vertices.
		// Deal with that here:
		int numToCopy = nv;
		//DbgAssert (nv == GetPaintSelCount());

		if (nv>GetPaintSelCount()) numToCopy = GetPaintSelCount();

		if (numToCopy>0) memcpy (wvs, GetPaintSelValues(), numToCopy*sizeof(float));
		// Zero out the rest if needed:
		for (int i=numToCopy; i<nv; i++) wvs[i] = 0.0f;
	}
	else { //soft selection is NOT locked
		temp->InvalidateSoftSelection ();	// have to do, or it might remember last time's falloff, etc.

		if (!vdValid.InInterval (t)) {
			temp->InvalidateDistances ();
			vdValid = pobj->ChannelValidity (t,GEOM_CHAN_NUM);
			vdValid &= pobj->ChannelValidity (t,TOPO_CHAN_NUM);
			if (edist) vdValid &= edistValid;
		}
		Tab<float> *vwTable = temp->VSWeight (edist, edist, ignoreBackfacing,
			falloff, pinch, bubble, MN_SEL);
		
		if (vwTable == NULL )
			return; 

		int min = (nv<vwTable->Count()) ? nv : vwTable->Count();
		if (min) memcpy (wvs, vwTable->Addr(0), min*sizeof(float));
		// zero out any accidental leftovers:
		for (int i=min; i<nv; i++) wvs[i] = 0.0f;
	}

	// Record in our cached mesh as well:
	memcpy (mesh->getVSelectionWeights(), wvs, nv*sizeof(float));
}

void PolyMeshSelMod::ModifyObject (TimeValue t, ModContext &mc, ObjectState *os, INode *node) {
	if (!os->obj->IsSubClassOf(polyObjectClassID)) return;
	PolyObject *pobj = (PolyObject*)os->obj;
	PolyMeshSelData *d  = (PolyMeshSelData*)mc.localData;
	if (!pobj->GetMesh().GetFlag (MN_MESH_FILLED_IN)) pobj->GetMesh().FillInMesh();
	if (!d) mc.localData = d = new PolyMeshSelData(pobj->GetMesh());

	BitArray vertSel = d->vertSel;
	BitArray faceSel = d->faceSel;
	BitArray edgeSel = d->edgeSel;
	vertSel.SetSize(pobj->GetMesh().VNum(),TRUE);
	faceSel.SetSize(pobj->GetMesh().FNum(),TRUE);
	edgeSel.SetSize(pobj->GetMesh().ENum(),TRUE);
	pobj->GetMesh().VertexSelect (vertSel);
	pobj->GetMesh().FaceSelect (faceSel);
	pobj->GetMesh().EdgeSelect (edgeSel);
	pobj->GetMesh().selLevel = meshLevel[selLevel];

	int useAR;
	Interval outValid;
	outValid = pobj->ChannelValidity (t,SELECT_CHAN_NUM);
	pblock->GetValue (ps_use_softsel, t, useAR, outValid);

	// Update the cache used for display, hit-testing, painting:
	if (!d->GetMesh()) {
		d->SetCache (pobj->GetMesh());
		UpdatePaintObject (d);
	} else *(d->GetMesh()) = pobj->GetMesh();

	if (useAR) {
		float bubble, pinch, falloff;
		pblock->GetValue (ps_falloff, t, falloff, outValid);
		pblock->GetValue (ps_pinch, t, pinch, outValid);
		pblock->GetValue (ps_bubble, t, bubble, outValid);
		bool lockedSoftsel = (pblock->GetInt(ps_lock_softsel)? true:false);

		int useEDist, edist=0, affectBackfacing;
		Interval edistValid=FOREVER;
		pblock->GetValue (ps_use_edist, t, useEDist, outValid);
		if (useEDist) {
			pblock->GetValue (ps_edist, t, edist, edistValid);
			outValid &= edistValid;
		}

		pblock->GetValue (ps_affect_backfacing, t, affectBackfacing, outValid);

		pobj->GetMesh().SupportVSelectionWeights ();
		float *vs = pobj->GetMesh().getVSelectionWeights ();
		d->mesh->SupportVSelectionWeights ();
		int nv = pobj->GetMesh().VNum();
		d->GetWeightedVertSel (nv, vs, t, pobj, falloff, pinch, bubble,
			edist, !affectBackfacing, edistValid, lockedSoftsel);
	} else {
		pobj->GetMesh().setVDataSupport (VDATA_SELECT, FALSE);
		d->mesh->setVDataSupport(VDATA_SELECT, FALSE);
	}

	// Set display flags - but be sure to turn off vertex display in stack result if
	// "Show End Result" is turned on - we want the user to just see the Poly Select
	// level vertices (from the Display method).
	pobj->GetMesh().dispFlags = 0;
	if ((selLevel != SEL_VERTEX) || !ip || !ip->GetShowEndResult())
		pobj->GetMesh().SetDispFlag (levelDispFlags[selLevel]);
	pobj->SetChannelValidity (SELECT_CHAN_NUM, outValid);
}

void PolyMeshSelMod::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc) {
	if (!mc->localData) return;
	if (partID == PART_SELECT) return;
	if( InPaintMode() ) EndPaintMode();
	((PolyMeshSelData*)mc->localData)->FreeCache();
	if (!ip || (editMod!=this) || updateCachePosted) return;

	if (!pSelDesc.NumParamMaps ()) return;
	IParamMap2 *pmap = pSelDesc.GetParamMap (ps_map_main);
	if (!pmap) return;
	HWND hWnd = pmap->GetHWnd();
	if (!hWnd) return;
	TimeValue t = ip->GetTime();
	PostMessage(hWnd,WM_UPDATE_CACHE,(WPARAM)t,0);
	updateCachePosted = TRUE;
}

void PolyMeshSelMod::UpdateCache(TimeValue t) {
	NotifyDependents(Interval(t,t), PART_GEOM|SELECT_CHANNEL|PART_SUBSEL_TYPE|
		PART_DISPLAY|PART_TOPO, REFMSG_MOD_EVAL);
	updateCachePosted = FALSE;
}

class PSInvalidateDistanceEnumProc : public ModContextEnumProc {
public:
	PSInvalidateDistanceEnumProc () { }
	BOOL proc (ModContext *mc);
};

BOOL PSInvalidateDistanceEnumProc::proc (ModContext *mc) {
	if (!mc->localData) return true;
	PolyMeshSelData *psd = (PolyMeshSelData *) mc->localData;
	psd->InvalidateVDistances ();
	return true;
}

static PSInvalidateDistanceEnumProc thePSInvalidateDistanceEnumProc;

void PolyMeshSelMod::InvalidateVDistances () {
	EnumModContexts (&thePSInvalidateDistanceEnumProc);
}

void PolyMeshSelMod::UpdateSelLevelDisplay () {
	if (theSelectProc.mod != this) return;
	if (!pSelDesc.NumParamMaps ()) return;
	IParamMap2 *pmap = pSelDesc.GetParamMap (ps_map_main);
	if (!pmap) return;
	HWND hWnd = pmap->GetHWnd();
	if (!hWnd) return;
	theSelectProc.UpdateSelLevelDisplay (hWnd);
}

static int butIDs[] = { 0, IDC_SELVERTEX, IDC_SELEDGE, IDC_SELBORDER, IDC_SELFACE, IDC_SELELEMENT };
void SelectProc::UpdateSelLevelDisplay (HWND hWnd) {
	if (!mod) return;
	ICustToolbar *l_iToolbar = GetICustToolbar(GetDlgItem(hWnd,IDC_MS_SELTYPE));
	ICustButton* but = NULL;
	for (int i=1; i<6; i++) {
		but = l_iToolbar->GetICustButton (butIDs[i]);
		but->SetCheck ((DWORD)i==mod->selLevel);
		ReleaseICustButton (but);
	}
	ReleaseICustToolbar (l_iToolbar);
	UpdateWindow(hWnd);
}

static bool oldShowEnd;

//override for all of the command modes
class EditSSOverride : public IActionItemOverride 
{
protected:
	BOOL mActiveOverride;
	int mPrevPaintMode;
	PolyMeshSelMod *mpObj;
	CommandMode *mOldCommandMode;
public:
	EditSSOverride(PolyMeshSelMod *m):mPrevPaintMode(PAINTMODE_OFF),mActiveOverride(FALSE),mOldCommandMode(NULL),mpObj(m){}
	BOOL IsOverrideActive();
	BOOL StartOverride();
	BOOL EndOverride();
};

BOOL EditSSOverride::IsOverrideActive()
{
	return mActiveOverride;
}


BOOL EditSSOverride::StartOverride()
{
	if(mActiveOverride==TRUE) //already TRUE, exit
		return FALSE;
	mOldCommandMode = mpObj->ip->GetCommandMode();
	mPrevPaintMode = mpObj->GetPaintMode();
	if(mOldCommandMode!=mpObj->editSSMode)
		mpObj->ip->PushCommandMode(mpObj->editSSMode);
	else
		return FALSE;

	mActiveOverride = TRUE;

	return TRUE;
}

BOOL EditSSOverride::EndOverride()
{
	if(mActiveOverride==FALSE) //wasn't active had a problem
		return FALSE;
	mActiveOverride = FALSE;
	mpObj->ip->DeleteMode(mpObj->editSSMode);

	if(mPrevPaintMode!= PAINTMODE_OFF)
		mpObj->OnPaintBtn( mPrevPaintMode, GetCOREInterface()->GetTime());
	return TRUE;
}

void PolyMeshSelMod::BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev) {
	this->ip = ip;	
	editMod  = this;
	UpdateSetNames ();

	// Use our classdesc2 to put up our parammap2 maps:
	pSelDesc.BeginEditParams(ip, this, flags, prev);
	theSelectProc.mod = this;
	pSelDesc.SetUserDlgProc (&polysel_softsel_desc, ps_map_main, &theSelectProc);
	theSoftSelProc.mod = this;
	pSelDesc.SetUserDlgProc (&polysel_softsel_desc, ps_map_softsel, &theSoftSelProc);

	selectMode = new SelectModBoxCMode(this,ip);
	editSSMode = new EditSSMode(this,this,ip);

	// Add our accelerator table (keyboard shortcuts)
	mpActions = new PolySelectActionCB (this);
	ip->GetActionManager()->ActivateActionTable (mpActions, kPolySelectActionID);

	//register all of the overrides with this modifier, first though unregister any still around for wthatever reason,though EndEditParams should handlethis
	IActionItemOverrideManager *actionOverrideMan = static_cast<IActionItemOverrideManager*>(GetCOREInterface(IACTIONITEMOVERRIDEMANAGER_INTERFACE ));

	if(actionOverrideMan)
	{
		for(int i=0;i<mOverrides.Count();++i)
		{
			actionOverrideMan->DeactivateActionItemOverride(mOverrides[i]);		
		}
	}
	mOverrides.ZeroCount();

	ActionTable *pTable = mpActions->GetTable();
	ActionItem *item = pTable->GetAction(ID_PS_EDIT_SOFT_SELECTION_MODE);
	mOverrides.Append(1,&item);
	EditSSOverride *commandModeOverride = new EditSSOverride(this);
	actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);

	// Add our sub object type
	// TSTR type1(GetString(IDS_PM_VERTEX));
	// TSTR type2(GetString(IDS_PM_EDGE));
	// TSTR type3(GetString(IDS_PM_BORDER));
	// TSTR type4(GetString(IDS_PM_FACE));
	// TSTR type5(GetString(IDS_PM_ELEMENT));
	// const TCHAR *ptype[] = {type1, type2, type3, type4, type5};
	// This call is obsolete. Please see BaseObject::NumSubObjTypes() and BaseObject::GetSubObjType()
	// ip->RegisterSubObjectTypes(ptype, 5);

	// Restore the selection level.
	ip->SetSubObjectLevel(selLevel);

	SetEnableStates ();
	UpdateSelLevelDisplay ();
	SetNumSelLabel();

	// Set show end result.
	oldShowEnd = ip->GetShowEndResult() ? TRUE : FALSE;
	ip->SetShowEndResult (GetFlag (PS_DISP_END_RESULT));

	MeshPaintHandler::BeginEditParams();

	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);	
}

void PolyMeshSelMod::EndEditParams (IObjParam *ip,ULONG flags,Animatable *next) {

	MeshPaintHandler::EndEditParams();

	pSelDesc.EndEditParams (ip, this, flags, next);
	theSelectProc.mod = NULL;
	theSoftSelProc.mod = NULL;

	//unregister all of the overrides.  Need to do this before deactiviting the keyboard shortcuts.
	IActionItemOverrideManager *actionOverrideMan = static_cast<IActionItemOverrideManager*>(GetCOREInterface(IACTIONITEMOVERRIDEMANAGER_INTERFACE ));
	if(actionOverrideMan)
	{
		for(int i=0;i<mOverrides.Count();++i)
		{
			actionOverrideMan->DeactivateActionItemOverride(mOverrides[i]);		
		}
	}
	mOverrides.ZeroCount();

	// Deactivate our keyboard shortcuts
	if (mpActions) {
		ip->GetActionManager()->DeactivateActionTable (mpActions, kPolySelectActionID);
		delete mpActions;
		mpActions = NULL;
	}

	ip->DeleteMode(selectMode);
	if (selectMode) delete selectMode;
	selectMode = NULL;

	ip->DeleteMode(editSSMode);
	if(editSSMode)
	{
		delete editSSMode;
		editSSMode = NULL;
	}

	// Reset show end result
	SetFlag (PS_DISP_END_RESULT, ip->GetShowEndResult() ? TRUE : FALSE);
	ip->SetShowEndResult(oldShowEnd);

	this->ip = NULL;
	editMod  = NULL;

	TimeValue t = ip->GetTime();
	// aszabo|feb.05.02 This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
}

int PolyMeshSelMod::HitTest (TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc) {
	Interval valid;
	int savedLimits, res = 0;
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;
	
	int selByVert, ignoreBackfaces;
	pblock->GetValue (ps_by_vertex, t, selByVert, FOREVER);
	pblock->GetValue (ps_ignore_backfacing, t, ignoreBackfaces, FOREVER);

	// Setup GW
	MakeHitRegion(hr,type, crossing,DEF_PICKBOX_SIZE,p);
	gw->setHitRegion(&hr);
	Matrix3 mat = inode->GetObjectTM(t);
	gw->setTransform(mat);	
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	if (ignoreBackfaces) gw->setRndLimits (gw->getRndLimits() | GW_BACKCULL);
	else gw->setRndLimits (gw->getRndLimits() & ~GW_BACKCULL);
	gw->clearHitCode();

	SubObjHitList hitList;
	MeshSubHitRec* rec = NULL;	

	if (!mc->localData) return 0;
	PolyMeshSelData *pData = (PolyMeshSelData *) mc->localData;
	//pData->SetConverterFlag (MNM_SELCONV_IGNORE_BACK, ignoreBackfaces?true:false);
	pData->SetConverterFlag (MNM_SELCONV_REQUIRE_ALL, (crossing||(type==HITTYPE_POINT))?false:true);

	MNMesh *pMesh = pData->GetMesh();
	if (!pMesh) return 0;
	if ((selLevel > SEL_VERTEX) && selByVert) {
		res = pMesh->SubObjectHitTest(gw, gw->getMaterial(), &hr,
			flags|hitLevel[SEL_VERTEX]|SUBHIT_MNUSECURRENTSEL, hitList);
	} else {
		res = pMesh->SubObjectHitTest(gw, gw->getMaterial(), &hr,
			flags|hitLevel[selLevel], hitList);
	}

	rec = hitList.First();
	while (rec) {
		vpt->LogHit(inode,mc,rec->dist,rec->index,NULL);
		rec = rec->Next();
	}

	gw->setRndLimits(savedLimits);	
	return res;	
}

int PolyMeshSelMod::Display (TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc) {
	if (!ip || !ip->GetShowEndResult ()) return 0;
	if (!selLevel) return 0;
	if (!mc->localData) return 0;

	PolyMeshSelData *modData = (PolyMeshSelData *) mc->localData;
	MNMesh *mesh = modData->GetMesh();
	if (!mesh) return 0;

	// Set up GW
	GraphicsWindow *gw = vpt->getGW();
	Matrix3 tm = inode->GetObjectTM(t);
	int savedLimits;
	gw->setRndLimits((savedLimits = gw->getRndLimits()) & ~GW_ILLUM);
	gw->setTransform(tm);

	// We need to draw a "gizmo" version of the mesh:
	Point3 colSel=GetSubSelColor();
	Point3 colTicks=GetUIColor (COLOR_VERT_TICKS);
	Point3 colGiz=GetUIColor(COLOR_GIZMOS);
	Point3 colGizSel=GetUIColor(COLOR_SEL_GIZMOS);
	gw->setColor (LINE_COLOR, colGiz);

	Point3 rp[3];
	int i;

#ifdef MESH_CAGE_BACKFACE_CULLING
	if (savedLimits & GW_BACKCULL) mesh->UpdateBackfacing (gw, true);
#endif

	int es[3];
	gw->startSegments();
	for (i=0; i<mesh->nume; i++) {
		if (mesh->e[i].GetFlag (MN_DEAD|MN_HIDDEN)) continue;
		if (!mesh->e[i].GetFlag (MN_EDGE_INVIS)) {
			es[0] = GW_EDGE_VIS;
		} else {
			if (selLevel < SEL_EDGE) continue;
			if (selLevel > SEL_FACE) continue;
			es[0] = GW_EDGE_INVIS;
		}
#ifdef MESH_CAGE_BACKFACE_CULLING
		if ((savedLimits & GW_BACKCULL) && mesh->e[i].GetFlag (MN_BACKFACING)) continue;
#endif
		if ((selLevel == SEL_EDGE) || (selLevel==SEL_BORDER)) {
			if (modData->GetEdgeSel()[i]) gw->setColor (LINE_COLOR, colGizSel);
			else gw->setColor (LINE_COLOR, colGiz);
		}
		if ((selLevel == SEL_FACE) || (selLevel == SEL_ELEMENT)) {
			if (modData->GetFaceSel()[mesh->e[i].f1] || ((mesh->e[i].f2>-1) && modData->GetFaceSel()[mesh->e[i].f2]))
				gw->setColor (LINE_COLOR, colGizSel);
			else gw->setColor (LINE_COLOR, colGiz);
		}
		rp[0] = mesh->v[mesh->e[i].v1].p;
		rp[1] = mesh->v[mesh->e[i].v2].p;
//		gw->polyline (2, rp, NULL, NULL, FALSE, es);
		if (es[0] == GW_EDGE_VIS)
			gw->segment(rp,1);
		else gw->segment(rp,0);
	}
	gw->endSegments();

	if (selLevel == SEL_VERTEX) {
		float *ourvw = NULL;
		int affectRegion=FALSE;
		if (pblock) pblock->GetValue (ps_use_softsel, t, affectRegion, FOREVER);
		if (affectRegion) ourvw = mesh->getVSelectionWeights ();
		gw->startMarkers();
		for (i=0; i<mesh->numv; i++) {
			if (mesh->v[i].GetFlag (MN_DEAD|MN_HIDDEN)) continue;

#ifdef MESH_CAGE_BACKFACE_CULLING
			if ((savedLimits & GW_BACKCULL) && !getDisplayBackFaceVertices()) {
				if (mesh->v[i].GetFlag(MN_BACKFACING)) continue;
			}
#endif

			if (modData->GetVertSel()[i]) gw->setColor (LINE_COLOR, colSel);
			else {
				if (ourvw) gw->setColor (LINE_COLOR, SoftSelectionColor (ourvw[i]));
				else gw->setColor (LINE_COLOR, colTicks);
			}

			if(getUseVertexDots()) gw->marker (&(mesh->v[i].p), VERTEX_DOT_MARKER(getVertexDotType()));
			else gw->marker (&(mesh->v[i].p), PLUS_SIGN_MRKR);
		}
		gw->endMarkers();
	}
	gw->setRndLimits(savedLimits);
	return 0;	
}

void PolyMeshSelMod::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) {
	if (!ip || !ip->GetShowEndResult() || !mc->localData) return;
	if (!selLevel) return;
	PolyMeshSelData *modData = (PolyMeshSelData *) mc->localData;
	MNMesh *mesh = modData->GetMesh();
	if (!mesh) return;
	Matrix3 tm = inode->GetObjectTM(t);
	box = mesh->getBoundingBox (&tm);
}

void PolyMeshSelMod::GetSubObjectCenters (SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) {
	if (!mc->localData) return;
	if (selLevel == SEL_OBJECT) return;	// shouldn't happen.
	PolyMeshSelData *modData = (PolyMeshSelData *) mc->localData;
	MNMesh *mesh = modData->GetMesh();
	if (!mesh) return;
	Matrix3 tm = node->GetObjectTM(t);

	// For Mesh Select, we merely return the center of the bounding box of the current selection.
	BitArray sel = mesh->VertexTempSel ();
	if (!sel.NumberSet()) return;
	Box3 box;
	for (int i=0; i<mesh->numv; i++) if (sel[i]) box += mesh->v[i].p * tm;
	cb->Center (box.Center(),0);
}

void PolyMeshSelMod::GetSubObjectTMs (SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) {
	if (!mc->localData) return;
	if (selLevel == SEL_OBJECT) return;	// shouldn't happen.
	PolyMeshSelData *modData = (PolyMeshSelData *) mc->localData;
	MNMesh *mesh = modData->GetMesh();
	if (!mesh) return;
	Matrix3 tm = node->GetObjectTM(t);

	// For Mesh Select, we merely return the center of the bounding box of the current selection.
	BitArray sel = mesh->VertexTempSel ();
	if (!sel.NumberSet()) return;
	Box3 box;
	for (int i=0; i<mesh->numv; i++) if (sel[i]) box += mesh->v[i].p * tm;
	Matrix3 ctm(1);
	ctm.SetTrans (box.Center());
	cb->TM (ctm,0);
}

void PolyMeshSelMod::ActivateSubobjSel(int level, XFormModes& modes) {
	// Set the meshes level
	selLevel = level;

	// Fill in modes with our sub-object modes
	if (level!=SEL_OBJECT) {
		modes = XFormModes(NULL,NULL,NULL,NULL,NULL,selectMode);
	}

	// Update UI
	UpdateSelLevelDisplay ();
	SetEnableStates ();
	SetNumSelLabel ();
	SetSoftSelEnables ();

	// Setup named selection sets	
	SetupNamedSelDropDown();

	// Invalidate the weighted vertex caches
	InvalidateVDistances ();

	NotifyDependents(FOREVER, PART_SUBSEL_TYPE|PART_DISPLAY, REFMSG_CHANGE);
	ip->PipeSelLevelChanged();
	NotifyDependents(FOREVER, SELECT_CHANNEL|DISP_ATTRIB_CHANNEL|SUBSEL_TYPE_CHANNEL, REFMSG_CHANGE);
}

DWORD PolyMeshSelMod::GetSelLevel () {
	switch (selLevel) {
	case SEL_OBJECT: return IMESHSEL_OBJECT;
	case SEL_VERTEX: return IMESHSEL_VERTEX;
	case SEL_EDGE:
	case SEL_BORDER:
		return IMESHSEL_EDGE;
	}
	return IMESHSEL_FACE;
}

void PolyMeshSelMod::SetSelLevel(DWORD sl) {
	switch (sl) {
	case IMESHSEL_OBJECT:
		selLevel = SEL_OBJECT;
		break;
	case IMESHSEL_VERTEX:
		selLevel = SEL_VERTEX;
		break;
	case IMESHSEL_EDGE:
		// Don't change if we're already in an edge level:
		if (GetSelLevel() == IMESHSEL_EDGE) break;
		selLevel = SEL_EDGE;
		break;
	case IMESHSEL_FACE:
		// Don't change if we're already in a face level:
		if (GetSelLevel()==IMESHSEL_FACE) break;
		selLevel = SEL_FACE;
		break;
	}
	if (ip && editMod==this) ip->SetSubObjectLevel(selLevel);
}

void PolyMeshSelMod::LocalDataChanged() {
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	if (ip && editMod==this) {
		SetNumSelLabel();
		UpdateNamedSelDropDown ();
	}
	InvalidateVDistances ();
}

void PolyMeshSelMod::UpdateNamedSelDropDown () {
	if (!ip) return;
	if (selLevel == SEL_OBJECT) {
		ip->ClearCurNamedSelSet ();
		return;
	}
	// See if this selection matches a named set
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts (mcList, nodes);
	BitArray nselmatch;
	nselmatch.SetSize (namedSel[namedSetLevel[selLevel]].Count());
	nselmatch.SetAll ();
	int nd, i, foundone=FALSE;
	for (nd=0; nd<mcList.Count(); nd++) {
		PolyMeshSelData *d = (PolyMeshSelData *) mcList[nd]->localData;
		if (!d) continue;
		foundone = TRUE;
		switch (selLevel) {
		case SEL_VERTEX:
			for (i=0; i<nselmatch.GetSize(); i++) {
				if (!nselmatch[i]) continue;
				if (!(*(d->vselSet.sets[i]) == d->vertSel)) nselmatch.Clear(i);
			}
			break;
		case SEL_EDGE:
		case SEL_BORDER:
			for (i=0; i<nselmatch.GetSize(); i++) {
				if (!nselmatch[i]) continue;
				if (!(*(d->eselSet.sets[i]) == d->edgeSel)) nselmatch.Clear(i);
			}
			break;
		default:
			for (i=0; i<nselmatch.GetSize(); i++) {
				if (!nselmatch[i]) continue;
				if (!(*(d->fselSet.sets[i]) == d->faceSel)) nselmatch.Clear(i);
			}
			break;
		}
		if (nselmatch.NumberSet () == 0) break;
	}
	if (foundone && nselmatch.AnyBitSet()) {
		for (i=0; i<nselmatch.GetSize(); i++) if (nselmatch[i]) break;
		ip->SetCurNamedSelSet (*(namedSel[namedSetLevel[selLevel]][i]));
	} else ip->ClearCurNamedSelSet ();
}

void PolyMeshSelMod::SelectSubComponent (HitRecord *firstHit, BOOL selected, BOOL all, BOOL invert) {
	if (selLevel == SEL_OBJECT) return;	// shouldn't happen.
	if (!ip) return;

	PolyMeshSelData *pData = NULL;
	HitRecord* hr = NULL;

	ip->ClearCurNamedSelSet();

	int selByVert;
	pblock->GetValue (ps_by_vertex, TimeValue(0), selByVert, FOREVER);

	// Prepare a bitarray representing the particular hits we got:
	if (selByVert) {
		for (hr=firstHit; hr!=NULL; hr = hr->Next()) {
			pData = (PolyMeshSelData*)hr->modContext->localData;
			if (!pData->GetNewSelection ()) {
				pData->SetupNewSelection (SEL_VERTEX);
				pData->SynchBitArrays();
			}
			pData->GetNewSelection()->Set (hr->hitInfo, true);
			if (!all) break;
		}
	} else {
		for (hr=firstHit; hr!=NULL; hr = hr->Next()) {
			pData = (PolyMeshSelData*)hr->modContext->localData;
			if (!pData->GetNewSelection ()) {
				pData->SetupNewSelection (selLevel);
				pData->SynchBitArrays();
			}
			pData->GetNewSelection()->Set (hr->hitInfo, true);
			if (!all) break;
		}
	}

	// Now that the hit records are translated into bitarrays, apply them:
	for (hr=firstHit; hr!=NULL; hr = hr->Next()) {
		pData = (PolyMeshSelData*)hr->modContext->localData;
		if (!pData->GetNewSelection ()) {
			if (!all) break;
			continue;
		}

		// Translate the hits into the correct selection level:
		if (selByVert && (selLevel>SEL_VERTEX)) {
			pData->TranslateNewSelection (SEL_VERTEX, selLevel);
		} else {
			if (selLevel == SEL_BORDER) pData->TranslateNewSelection (SEL_EDGE, SEL_BORDER);
			if (selLevel == SEL_ELEMENT) pData->TranslateNewSelection (SEL_FACE, SEL_ELEMENT);
		}

		if (GetKeyState(VK_SHIFT) & 0x8000) //Select loop or ring if Shift key is pressed
		{
			MNMesh* mm = pData->mesh;
			BitArray* newSel = pData->GetNewSelection();
			BitArray currentSel;
			bool foundLoopRing = false;
			IMNMeshUtilities13* l_mesh13 = static_cast<IMNMeshUtilities13*>(mm->GetInterface( IMNMESHUTILITIES13_INTERFACE_ID ));
			if (l_mesh13)
			{
				if (selLevel == SEL_VERTEX)
				{
					mm->getVertexSel(currentSel);
					foundLoopRing = l_mesh13->FindLoopVertex(*newSel,currentSel);
				}
				else if (selLevel == SEL_EDGE)
				{
					mm->getEdgeSel(currentSel);
					foundLoopRing = l_mesh13->FindLoopOrRingEdge(*newSel,currentSel);
				}
				else if (selLevel == SEL_FACE)
				{
					mm->getFaceSel(currentSel);
					foundLoopRing = l_mesh13->FindLoopFace(*newSel,currentSel);
				}
				if (!foundLoopRing) (*newSel).ClearAll(); //If no loop or ring is found, do not add anything to selection
			}
		}

		// Create the undo object:
		if (theHold.Holding()) theHold.Put (new SelectRestore (this, pData));

		// Apply the new selection.
		pData->ApplyNewSelection (selLevel, true, invert?true:false, selected?true:false);
		if (!all) break;
	}

	LocalDataChanged ();
}

void PolyMeshSelMod::ClearSelection(int selLevel) {
	if (!ip) return;
	if (GetKeyState(VK_SHIFT) & 0x8000) return; //Do not clear selection when attempting to select loop or ring
	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);
	PolyMeshSelData* d = NULL;
	for (int i=0; i<list.Count(); i++) {
		d = (PolyMeshSelData*)list[i]->localData;
		if (!d) continue;

		// Check if we have anything selected first:
		switch (selLevel) {
		case SEL_VERTEX:
			if (!d->vertSel.NumberSet()) continue;
			else break;
		case SEL_FACE:
		case SEL_ELEMENT:
			if (!d->faceSel.NumberSet()) continue;
			else break;
		case SEL_EDGE:
		case SEL_BORDER:
			if (!d->edgeSel.NumberSet()) continue;
			else break;
		}

		if (theHold.Holding() && !d->held) theHold.Put(new SelectRestore(this,d));
		d->SynchBitArrays();
		switch (selLevel) {
		case SEL_VERTEX:
			d->vertSel.ClearAll();
			break;
		case SEL_FACE:
		case SEL_ELEMENT:
			d->faceSel.ClearAll();
			break;
		case SEL_BORDER:
		case SEL_EDGE:
			d->edgeSel.ClearAll();
			break;
		}
	}
	nodes.DisposeTemporary();
	LocalDataChanged ();
}

void PolyMeshSelMod::SelectAll(int selLevel) {
	if (selLevel == SEL_BORDER) { SelectOpenEdges (); return; }
	if (!ip) return;
	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);
	PolyMeshSelData* d = NULL;
	for (int i=0; i<list.Count(); i++) {
		d = (PolyMeshSelData*)list[i]->localData;
		if (!d) continue;
		if (theHold.Holding() && !d->held) theHold.Put(new SelectRestore(this,d));
		d->SynchBitArrays();
		switch (selLevel) {
		case SEL_VERTEX:
			d->vertSel.SetAll();
			break;
		case SEL_FACE:
		case SEL_ELEMENT:
			d->faceSel.SetAll();
			break;
		case SEL_EDGE:
			d->edgeSel.SetAll();
			break;
		}
	}
	nodes.DisposeTemporary();
	LocalDataChanged ();
}

void PolyMeshSelMod::InvertSelection(int selLevel) {
	ModContextList list;
	INodeTab nodes;	
	if (!ip) return;
	ip->GetModContexts(list,nodes);
	PolyMeshSelData* d = NULL;
	for (int i=0; i<list.Count(); i++) {
		d = (PolyMeshSelData*)list[i]->localData;
		if (!d) continue;
		if (theHold.Holding() && !d->held) theHold.Put(new SelectRestore(this,d));
		d->SynchBitArrays();
		switch (selLevel) {
		case SEL_VERTEX:
			d->vertSel = ~d->vertSel;
			break;
		case SEL_FACE:
		case SEL_ELEMENT:
			d->faceSel = ~d->faceSel;
			break;
		case SEL_EDGE:
		case SEL_BORDER:
			d->edgeSel = ~d->edgeSel;
			break;
		}
	}
	if (selLevel == SEL_BORDER) { SelectOpenEdges (-1); }	// indicates deselect only
	nodes.DisposeTemporary();
	LocalDataChanged ();
}

void PolyMeshSelMod::SelectByMatID(int id) {
	if (!ip) return;
	BOOL add = GetKeyState(VK_CONTROL)<0 ? TRUE : FALSE;
	BOOL sub = GetKeyState(VK_MENU)<0 ? TRUE : FALSE;
	theHold.Begin();
	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);
	PolyMeshSelData* d = NULL;	
	for (int i=0; i<list.Count(); i++) {
		d = (PolyMeshSelData*)list[i]->localData;
		if (!d) continue;
		if (!d->held) theHold.Put(new SelectRestore(this,d));
		d->SynchBitArrays();
		if (!add && !sub) d->faceSel.ClearAll();
		MNMesh *mesh = d->mesh;
		if (!mesh) continue;
		for (int i=0; i<mesh->FNum(); i++) {
			if (mesh->f[i].material == (MtlID)id) {
				if (sub) d->faceSel.Clear(i);
				else d->faceSel.Set(i);
			}
		}
	}
	nodes.DisposeTemporary();
	theHold.Accept (GetString (IDS_SELECT_BY_MATID));

	LocalDataChanged ();
	ip->RedrawViews(ip->GetTime());
}

void PolyMeshSelMod::SelectOpenEdges (int selType) {
	if (!ip) return;
	bool localHold = false;
	if (!theHold.Holding()) {
		localHold = true;
		theHold.Begin();
	}
	ModContextList list;
	INodeTab nodes;
	ip->GetModContexts(list,nodes);
	PolyMeshSelData* d = NULL;
	for (int i=0; i<list.Count(); i++) {
		d = (PolyMeshSelData*)list[i]->localData;		
		if (!d) continue;
		if (!d->mesh) continue;

		if (!d->held) theHold.Put(new SelectRestore(this,d));
		d->SynchBitArrays();

		for (int j=0; j<d->mesh->ENum(); j++) {
			MNEdge *me = d->mesh->E(j);
			if (me->GetFlag (MN_DEAD)) continue;
			if (me->f2 < 0) {
				if (selType < 0) continue;
				me->SetFlag (MN_SEL);
				d->edgeSel.Set (j);
			} else {
				if (selType > 0) continue;
				me->ClearFlag (MN_SEL);
				d->edgeSel.Clear (j);
			}
		}
	}
	nodes.DisposeTemporary();
	if (localHold) theHold.Accept(GetString (IDS_SELECT_OPEN));
	LocalDataChanged ();
	ip->RedrawViews(ip->GetTime());
}

// russom - 1/20/04 - FID 14958
void PolyMeshSelMod::ConvertSelection (int epSelLevelFrom, int epSelLevelTo, bool requireAll) {
	ModContextList list;
	INodeTab nodes;
	if (!ip) return;
	ip->GetModContexts(list,nodes);
	PolyMeshSelData* d = NULL;

	theHold.Begin();
	for (int m=0; m<list.Count(); m++) {
		d = (PolyMeshSelData*)list[m]->localData;
		if (!d) continue;

		MNMesh *mm = d->GetMesh();
		if( !mm ) continue;

		DbgAssert (mm->GetFlag (MN_MESH_FILLED_IN));
		if (!mm->GetFlag (MN_MESH_FILLED_IN)) return;

		if (epSelLevelFrom == SEL_CURRENT) epSelLevelFrom = selLevel;
		if (epSelLevelTo == SEL_CURRENT) epSelLevelTo = selLevel;
		if (epSelLevelFrom == epSelLevelTo) return;

		// We don't select the whole object here.
		if (epSelLevelTo == SEL_OBJECT) return;

		// If 'from' is the whole object, we select all.
		BitArray sel;
		int i;
		if (epSelLevelFrom == SEL_OBJECT) {
			switch (epSelLevelTo) {
			case SEL_VERTEX: 
				sel.SetSize (mm->numv);
				sel.SetAll();
				d->SetVertSel (sel, this, ip->GetTime());
				break;
			case SEL_EDGE:
				sel.SetSize (mm->nume);
				sel.SetAll();
				d->SetEdgeSel (sel, this, ip->GetTime());
				break;
			case SEL_BORDER:
				sel.SetSize (mm->nume);
				for (i=0; i<mm->nume; i++) sel.Set (i, mm->e[i].f2<0);
				d->SetEdgeSel (sel, this, ip->GetTime());
				break;
			default:
				sel.SetSize (mm->numf);
				sel.SetAll(); 
				d->SetFaceSel (sel, this, ip->GetTime());
				break;
			}
		} else {
			int mslFrom = meshSelLevel[epSelLevelFrom];
			int mslTo = meshSelLevel[epSelLevelTo];

			if (theHold.Holding() && !d->held) 
				theHold.Put (new PSComponentFlagRestore (this, d, mslTo));
			d->SynchBitArrays();

			// Clear the selection first
			switch (mslTo) {
			case MNM_SL_VERTEX:
				mm->ClearVFlags (MN_SEL);
				break;
			case MNM_SL_EDGE:
				mm->ClearEFlags (MN_SEL);
				break;
			case MNM_SL_FACE:
				mm->ClearFFlags (MN_SEL);
				break;
			}

			// Then propegate flags using the convenient MNMesh method:
			mm->PropegateComponentFlags (mslTo, MN_SEL, mslFrom, MN_SEL, requireAll);

			// Then either update the EPoly selection BitArrays, or convert to Border/Element selections:
			switch (epSelLevelTo) {
			case SEL_VERTEX:
				mm->getVertexSel (d->vertSel);
				break;
			case SEL_EDGE:
				mm->getEdgeSel (d->edgeSel);
				break;
			case SEL_FACE:
				mm->getFaceSel (d->faceSel);
				break;

			case SEL_BORDER:
				sel.SetSize(mm->nume);
				sel.ClearAll();

				for (i=0; i<sel.GetSize(); i++) {
					if (!mm->e[i].GetFlag (MN_SEL)) continue;
					if (sel[i]) continue;
					if (mm->e[i].f2 > -1) continue;
					mm->BorderFromEdge (i, sel);
				}
				d->SetEdgeSel (sel, this, TimeValue(0));
				break;

			case SEL_ELEMENT:
				sel.SetSize(mm->numf);
				sel.ClearAll();

				for (int i=0; i<mm->numf; i++) {
					if (!mm->f[i].GetFlag (MN_SEL)) continue;
					if (sel[i]) continue;
					mm->ElementFromFace (i, sel);
				}
				d->SetFaceSel (sel, this, TimeValue(0));
				break;
			}
		}
	}
	theHold.Accept(GetString(IDS_CONVERT_SELECTION));
	nodes.DisposeTemporary();
	LocalDataChanged ();
	ip->RedrawViews(ip->GetTime());
}



void PolyMeshSelMod::ConvertSelectionToBorder (int in_epSelLevelFrom, int in_epSelLevelTo) 
{

	if (!ip) 
		return ;

	if (in_epSelLevelFrom == SEL_CURRENT) 
		in_epSelLevelFrom = selLevel;

	if (in_epSelLevelTo == SEL_CURRENT) 
		in_epSelLevelTo = selLevel;

	if (in_epSelLevelFrom == in_epSelLevelTo) 
		return ;

	// We don't select the whole object here.
	if (in_epSelLevelTo == SEL_OBJECT) 
		return ;


	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);

	int l_numComponentsSelected = 0;
	DWORD MN_NOT_SELECTED	= MN_USER << 1; 

	DWORD l_flag; 
	DWORD l_BorderFlag = MN_USER | MN_NOT_SELECTED ; 

	BitArray l_newSel;

	in_epSelLevelFrom	= meshSelLevel[in_epSelLevelFrom];
	in_epSelLevelTo		= meshSelLevel[in_epSelLevelTo];


	for ( int i = 0; i < list.Count(); i++) 
	{
		PolyMeshSelData *pData = (PolyMeshSelData*)list[i]->localData;

		if (!pData) 
			continue;

		MNMesh *pMesh = pData->GetMesh ();

		if (!pMesh) 
			continue;

		DbgAssert (pMesh->GetFlag (MN_MESH_FILLED_IN));

		if (!pMesh->GetFlag (MN_MESH_FILLED_IN)) 
			continue;

		switch (in_epSelLevelTo) 
		{
			case MNM_SL_VERTEX:
				pMesh->ClearVFlags (l_BorderFlag);
				break;
			case MNM_SL_EDGE:
				pMesh->ClearEFlags (l_BorderFlag);
				break;
			case MNM_SL_FACE:
				pMesh->ClearFFlags (l_BorderFlag);
				break;
		}


		switch (in_epSelLevelFrom) {
		case MNM_SL_FACE: 
			{
				pMesh->ClearVFlags (l_BorderFlag);
				pMesh->ClearEFlags (l_BorderFlag);
				for (int i=0; i <pMesh->numf; i++)
				{
					if (pMesh->f[i].GetFlag (MN_DEAD)) 
						continue;

					if (pMesh->f[i].GetFlag (MN_SEL))
						l_flag = MN_USER;
					else 
						l_flag = MN_NOT_SELECTED;

					for (int j = 0; j < pMesh->f[i].deg; j++) 
					{
						if ( in_epSelLevelTo == MNM_SL_VERTEX )
						{
							pMesh->v[pMesh->f[i].vtx[j]].SetFlag (l_flag);
						}
						else if (in_epSelLevelTo == MNM_SL_EDGE )
						{
							// set all edges of this face to being set
							pMesh->e[pMesh->f[i].edg[j]].SetFlag (l_flag);
						}
					}

				}

				if ( in_epSelLevelTo == MNM_SL_VERTEX )
				{
					l_newSel.SetSize (pMesh->numv);
					for (int i=0; i<pMesh->numv; i++) 
						l_newSel.Set (i, pMesh->v[i].GetFlag (MN_USER) && (pMesh->v[i].GetFlag (MN_NOT_SELECTED) ||
						pMesh->vedg[i].Count() > pMesh->vfac[i].Count()));

					pData->SetVertSel(l_newSel, this, ip->GetTime());

				}
				else if ( in_epSelLevelTo == MNM_SL_EDGE )
				{
					l_newSel.SetSize (pMesh->nume);
					for (int i=0; i<pMesh->nume; i++) 
						l_newSel.Set (i, pMesh->e[i].GetFlag (MN_USER) && (pMesh->e[i].GetFlag (MN_NOT_SELECTED)||
						pMesh->e[i].f2 < 0));
					pData->SetEdgeSel(l_newSel, this, ip->GetTime());
				}

			}
			break;

		case MNM_SL_EDGE: //in_epSelLevelFrom
			{
				for (int i=0; i <pMesh->nume; i++)
				{
					if (pMesh->e[i].GetFlag (MN_DEAD)) continue;
					if (pMesh->e[i].GetFlag (MN_SEL))
						l_flag = MN_USER;
					else 
						l_flag = MN_NOT_SELECTED;

					if ( in_epSelLevelTo == MNM_SL_VERTEX )
					{
						// set all vertices of this edge to being set
						pMesh->v[pMesh->e[i].v1].SetFlag (l_flag);
						pMesh->v[pMesh->e[i].v2].SetFlag (l_flag);
					}
					else if (in_epSelLevelTo == MNM_SL_FACE )
					{
						// set all faces of this face to being set
						pMesh->f[pMesh->e[i].f1].SetFlag (l_flag);
						if (pMesh->e[i].f2 != -1 )
							pMesh->f[pMesh->e[i].f2].SetFlag (l_flag);
					}

				}

				if ( in_epSelLevelTo == MNM_SL_VERTEX )
				{
					l_newSel.SetSize (pMesh->numv);
					for (int i=0; i<pMesh->numv; i++) 
						l_newSel.Set (i, pMesh->v[i].GetFlag (MN_USER) && pMesh->v[i].GetFlag (MN_NOT_SELECTED));
					pData->SetVertSel(l_newSel, this, ip->GetTime());
				}
				else if ( in_epSelLevelTo == MNM_SL_FACE )
				{
					l_newSel.SetSize (pMesh->numf);
					for (int i=0; i<pMesh->numf; i++) 
						l_newSel.Set (i, pMesh->f[i].GetFlag (MN_USER) && pMesh->f[i].GetFlag (MN_NOT_SELECTED));
					pData->SetFaceSel(l_newSel, this, ip->GetTime());
				}
			}
			break ; 
		case MNM_SL_VERTEX: //in_epSelLevelFrom
			{
				pMesh->ClearVFlags (l_BorderFlag);
				pMesh->ClearEFlags (l_BorderFlag);

				for (int i = 0; i < pMesh->numv; i++)
				{
					if (pMesh->v[i].GetFlag (MN_DEAD)) 
						continue;


					for ( int j = 0; j < pMesh->vedg[i].Count(); j++) 
					{
						// determine which vertices are at the 'border
						// is this vertex on the 'border' ? 
						if (  (pMesh->v[i].GetFlag (MN_SEL)) &&
							(!pMesh->v[pMesh->e[pMesh->vedg[i][j]].v1].GetFlag(MN_SEL)||
							!pMesh->v[pMesh->e[pMesh->vedg[i][j]].v2].GetFlag(MN_SEL) || 
							pMesh->e[pMesh->vedg[i][j]].f2 < 0 ))
						{
							pMesh->v[i].SetFlag (l_BorderFlag);
						}
					}

				}

				// process now the 'border vertices'
				l_newSel.SetSize (pMesh->nume);
				for (int i = 0; i < pMesh->numv; i++) 
				{
					if ( pMesh->v[i].GetFlag(l_BorderFlag)) 
					{
						for ( int j = 0; j < pMesh->vedg[i].Count(); j++) 
						{
							if (( pMesh->e[pMesh->vedg[i][j]].v1 == i && 
								pMesh->v[pMesh->e[pMesh->vedg[i][j]].v2].GetFlag(l_BorderFlag)) ||	
								(pMesh->e[pMesh->vedg[i][j]].v2 == i  &&
								pMesh->v[pMesh->e[pMesh->vedg[i][j]].v1].GetFlag(l_BorderFlag) ))
								// this edge is a border edge 
								pMesh->e[pMesh->vedg[i][j]].SetFlag(l_BorderFlag);


						}

					}


					if ( in_epSelLevelTo == MNM_SL_EDGE )
					{

						// process now the border edges. 
						for (int i = 0; i < pMesh->nume; i++) 
						{
							l_newSel.Set (i, pMesh->e[i].GetFlag (l_BorderFlag));
						}

						pData->SetEdgeSel(l_newSel, this, ip->GetTime());
					}
					else if (  in_epSelLevelTo == MNM_SL_FACE )
					{
						// use the 'border' edges to figure out the 'border' faces 
						l_newSel.SetSize (pMesh->numf);
						for (int i = 0; i < pMesh->nume; i++) 
						{
							if (  pMesh->e[i].GetFlag (l_BorderFlag))
							{
								// this is a edge border 
								for ( int k = 0 ; k < 2 ; k++ )
								{
									// process both faces 
									int l_faceIndex = 0;

									if ( k == 0 )
										l_faceIndex = pMesh->e[i].f1;
									else 
										l_faceIndex = pMesh->e[i].f2;

									if ( l_faceIndex >= 0 )
									{
										int l_numOfSelectedVertices = 0 ;
										for ( int j =0; j < pMesh->f[l_faceIndex].deg ; j++)
										{
											// loop through all vertices 
											if ( pMesh->v[pMesh->f[l_faceIndex].vtx[j]].GetFlag(MN_SEL))
											{
												l_numOfSelectedVertices++;
											}
										}

										// this is an interior face , who's edge is on the border 
										if ( l_numOfSelectedVertices == pMesh->f[l_faceIndex].deg)
											l_newSel.Set (l_faceIndex, true);
									}

								}

							}

						}

						pData->SetFaceSel(l_newSel, this, ip->GetTime());


					}
				}			
			}
			break ; 

		default: 
			break;



		}
		l_numComponentsSelected += l_newSel.NumberSet();

		// cleanup temp flags 
		switch (in_epSelLevelTo) 
		{
			case MNM_SL_VERTEX:
				pMesh->ClearVFlags (l_BorderFlag);
				break;
			case MNM_SL_EDGE:
				pMesh->ClearEFlags (l_BorderFlag);
				break;
			case MNM_SL_FACE:
				pMesh->ClearFFlags (l_BorderFlag);
				break;
		}
	}


	theHold.Accept(GetString(IDS_CONVERT_SELECTION));
	nodes.DisposeTemporary();
	LocalDataChanged ();
	ip->RedrawViews(ip->GetTime());


	
}


void PolyMeshSelMod::SelectionOp(int opcode) {
	ModContextList list;
	INodeTab nodes;
	if (!ip) return;
	ip->GetModContexts(list,nodes);
	PolyMeshSelData* d = NULL;

	theHold.Begin();
	for (int m=0; m<list.Count(); m++) {
		d = (PolyMeshSelData*)list[m]->localData;
		if (!d) continue;

		if (!d->held) theHold.Put(new SelectRestore(this,d));

		switch( opcode ) {
			case epsop_sel_grow:
				d->GrowSelection(this, selLevel);
				break;
			case epsop_sel_shrink:
				d->ShrinkSelection(this, selLevel);
				break;
			case epsop_select_ring:
				d->SelectEdgeRing(this);
				break;
			case epsop_select_loop:
				d->SelectEdgeLoop(this);
				break;
		}
	}
	theHold.Accept(GetString(IDS_PM_SELECT));
	nodes.DisposeTemporary();
	LocalDataChanged ();
	ip->RedrawViews(ip->GetTime());
}

void PolyMeshSelMod::SelectFrom(int from) {
	ModContextList list;
	INodeTab nodes;
	if (!ip) return;
	ip->GetModContexts(list,nodes);
	PolyMeshSelData* d = NULL;

	theHold.Begin();
	for (int i=0; i<list.Count(); i++) {
		d = (PolyMeshSelData*)list[i]->localData;
		if (!d) continue;

		if (!d->held) theHold.Put(new SelectRestore(this,d));
		d->SynchBitArrays();

		switch (selLevel) {
		case SEL_VERTEX: 
			if (from==SEL_FACE) d->SelVertByFace();
			else d->SelVertByEdge();
			break;
		case SEL_FACE:
			if (from==SEL_VERTEX) d->SelFaceByVert();
			else d->SelFaceByEdge();
			break;
		case SEL_ELEMENT:
			if (from==SEL_VERTEX) d->SelElementByVert();
			else d->SelElementByEdge();
			break;
		case SEL_BORDER:
			if (from==SEL_VERTEX) d->SelBorderByVert();
			else d->SelBorderByFace();
			break;
		case SEL_EDGE:
			if (from==SEL_VERTEX) d->SelEdgeByVert();
			else d->SelEdgeByFace();
			break;
		}
	}
	theHold.Accept(GetString(IDS_PM_SELECT));
	nodes.DisposeTemporary();
	LocalDataChanged ();
	ip->RedrawViews(ip->GetTime());
}

#define SELLEVEL_CHUNKID		0x0100
#define FLAGS_CHUNKID			0x0120
#define VERTSEL_CHUNKID			0x0200
#define FACESEL_CHUNKID			0x0210
#define EDGESEL_CHUNKID			0x0220

#define NAMEDVSEL_NAMES_CHUNK	0x2805
#define NAMEDFSEL_NAMES_CHUNK	0x2806
#define NAMEDESEL_NAMES_CHUNK	0x2807
#define NAMEDSEL_STRING_CHUNK	0x2809
#define NAMEDSEL_ID_CHUNK		0x2810

#define VSELSET_CHUNK			0x2845
#define FSELSET_CHUNK			0x2846
#define ESELSET_CHUNK			0x2847

#define MESHPAINTHANDLER_CHUNK	0x2848
#define MESHPAINTHOST_CHUNK		0x2849

static int namedSelID[] = {NAMEDVSEL_NAMES_CHUNK,NAMEDESEL_NAMES_CHUNK,NAMEDFSEL_NAMES_CHUNK, NAMEDFSEL_NAMES_CHUNK, NAMEDFSEL_NAMES_CHUNK};

IOResult PolyMeshSelMod::Save(ISave *isave) {
	IOResult res;
	ULONG nb;
	Modifier::Save(isave);

	isave->BeginChunk(SELLEVEL_CHUNKID);
	res = isave->Write(&selLevel, sizeof(selLevel), &nb);
	isave->EndChunk();

	DWORD flags = ExportFlags ();
	isave->BeginChunk (FLAGS_CHUNKID);
	res = isave->Write (&flags, sizeof(DWORD), &nb);
	isave->EndChunk ();

	for (int j=0; j<3; j++) {
		if (namedSel[j].Count()) {
			isave->BeginChunk(namedSelID[j]);			
			for (int i=0; i<namedSel[j].Count(); i++) {
				isave->BeginChunk(NAMEDSEL_STRING_CHUNK);
				isave->WriteWString(*namedSel[j][i]);
				isave->EndChunk();

				isave->BeginChunk(NAMEDSEL_ID_CHUNK);
				isave->Write(&ids[j][i],sizeof(DWORD),&nb);
				isave->EndChunk();
			}
			isave->EndChunk();
		}
	}

	isave->BeginChunk(MESHPAINTHANDLER_CHUNK);
	res = MeshPaintHandler::Save(isave);
	isave->EndChunk();
	

	return res;
}

IOResult PolyMeshSelMod::LoadNamedSelChunk(ILoad *iload,int level) {	
	IOResult res;
	DWORD ix=0;
	ULONG nb;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
		case NAMEDSEL_STRING_CHUNK: {
			TCHAR* name = NULL;
			res = iload->ReadWStringChunk(&name);
			TSTR *newName = new TSTR(name);
			namedSel[level].Append(1,&newName);				
			ids[level].Append(1,&ix);
			ix++;
			break;
			}
		case NAMEDSEL_ID_CHUNK:
			iload->Read(&ids[level][ids[level].Count()-1],sizeof(DWORD), &nb);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

IOResult PolyMeshSelMod::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	DWORD flags;

	Modifier::Load(iload);

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
		case SELLEVEL_CHUNKID:
			iload->Read(&selLevel, sizeof(selLevel), &nb);
			break;

		case FLAGS_CHUNKID:
			iload->Read (&flags, sizeof(DWORD), &nb);
			ImportFlags (flags);
			break;

		case NAMEDVSEL_NAMES_CHUNK:
			res = LoadNamedSelChunk(iload,0);
			break;

		case NAMEDESEL_NAMES_CHUNK:
			res = LoadNamedSelChunk(iload,1);
			break;

		case NAMEDFSEL_NAMES_CHUNK:
			res = LoadNamedSelChunk(iload,2);
			break;

		case MESHPAINTHANDLER_CHUNK:
			res = MeshPaintHandler::Load(iload);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

IOResult PolyMeshSelMod::SaveLocalData(ISave *isave, LocalModData *ld) {	
	PolyMeshSelData *d = (PolyMeshSelData*)ld;

	isave->BeginChunk(VERTSEL_CHUNKID);
	d->vertSel.Save(isave);
	isave->EndChunk();

	isave->BeginChunk(FACESEL_CHUNKID);
	d->faceSel.Save(isave);
	isave->EndChunk();

	isave->BeginChunk(EDGESEL_CHUNKID);
	d->edgeSel.Save(isave);
	isave->EndChunk();
	
	if (d->vselSet.Count()) {
		isave->BeginChunk(VSELSET_CHUNK);
		d->vselSet.Save(isave);
		isave->EndChunk();
	}
	if (d->eselSet.Count()) {
		isave->BeginChunk(ESELSET_CHUNK);
		d->eselSet.Save(isave);
		isave->EndChunk();
	}
	if (d->fselSet.Count()) {
		isave->BeginChunk(FSELSET_CHUNK);
		d->fselSet.Save(isave);
		isave->EndChunk();
	}

	isave->BeginChunk(MESHPAINTHOST_CHUNK);
	((MeshPaintHost*)d)->Save(isave);
	isave->EndChunk();


	return IO_OK;
}

IOResult PolyMeshSelMod::LoadLocalData(ILoad *iload, LocalModData **pld) {
	PolyMeshSelData *d = new PolyMeshSelData;
	*pld = d;
	IOResult res;	
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
		case VERTSEL_CHUNKID:
			d->vertSel.Load(iload);
			break;
		case FACESEL_CHUNKID:
			d->faceSel.Load(iload);
			break;
		case EDGESEL_CHUNKID:
			d->edgeSel.Load(iload);
			break;

		case VSELSET_CHUNK:
			res = d->vselSet.Load(iload);
			break;
		case FSELSET_CHUNK:
			res = d->fselSet.Load(iload);
			break;
		case ESELSET_CHUNK:
			res = d->eselSet.Load(iload);
			break;

		case MESHPAINTHOST_CHUNK:
			res = ((MeshPaintHost*)d)->Load(iload);
			break;

		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}



// Window Procs ------------------------------------------------------

void PolyMeshSelMod::SetEnableStates() {
	if (!pSelDesc.NumParamMaps ()) return;
	IParamMap2 *pmap = pSelDesc.GetParamMap (ps_map_main);
	if (!pmap) return;
	HWND hWnd = pmap->GetHWnd();
	if (!hWnd) return;
	theSelectProc.SetEnables (hWnd);
}

void SelectProc::SetEnables (HWND hParams) {
	if (!mod) return;
	int selLevel = mod->selLevel;
	ICustButton* but = NULL;
	ISpinnerControl* spin = NULL;

	bool obj = (selLevel == SEL_OBJECT) ? TRUE : FALSE;
	bool vert = (selLevel == SEL_VERTEX) ? TRUE : FALSE;
	bool edge = (selLevel == SEL_EDGE)||(selLevel == SEL_BORDER) ? TRUE : FALSE;
	bool face = (selLevel == SEL_FACE)||(selLevel == SEL_ELEMENT) ? TRUE : FALSE;

	EnableWindow (GetDlgItem (hParams, IDC_MS_SEL_BYVERT), edge||face);
	EnableWindow (GetDlgItem (hParams, IDC_MS_IGNORE_BACKFACES), !obj);

	// russom - 1/20/04 - FID 1495
	but = GetICustButton (GetDlgItem (hParams, IDC_MS_SELECTION_GROW));
	if (but) {
		but->Enable (!obj);
		ReleaseICustButton (but);
	}

	but = GetICustButton (GetDlgItem (hParams, IDC_MS_SELECTION_SHRINK));
	if (but) {
		but->Enable (!obj);
		ReleaseICustButton (but);
	}

	but = GetICustButton (GetDlgItem (hParams, IDC_MS_EDGE_RING_SEL));
	if (but) {
		but->Enable (edge);
		ReleaseICustButton(but);
	}

	but = GetICustButton (GetDlgItem (hParams, IDC_MS_EDGE_LOOP_SEL));
	if (but) {
		but->Enable (edge);
		ReleaseICustButton(but);
	}


	but = GetICustButton (GetDlgItem (hParams, IDC_MS_GETVERT));
	but->Enable (face||edge);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hParams, IDC_MS_GETEDGE));
	but->Enable (vert||face);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hParams, IDC_MS_GETFACE));
	but->Enable (vert||edge);
	ReleaseICustButton (but);

	EnableWindow (GetDlgItem (hParams, IDC_MS_SELBYMAT_BOX), face);
	EnableWindow (GetDlgItem (hParams, IDC_MS_SELBYMAT_TEXT), face);
	but = GetICustButton (GetDlgItem (hParams, IDC_MS_SELBYMAT));
	but->Enable (face);
	ReleaseICustButton (but);
	spin = GetISpinner (GetDlgItem (hParams, IDC_MS_MATIDSPIN));
	spin->Enable (face);
	ReleaseISpinner (spin);

	spin = GetISpinner (GetDlgItem (hParams, IDC_MS_EDGE_RING_SEL_SPIN));
	if (spin )
	{	
		spin->Enable (edge);
		EnableWindow (GetDlgItem (hParams, IDC_MS_EDGE_RING_SEL_SPIN), edge);
		ReleaseISpinner (spin);
	}

	spin = GetISpinner (GetDlgItem (hParams, IDC_MS_EDGE_LOOP_SEL_SPIN));
	if (spin )
	{	
		spin->Enable (edge);
		EnableWindow (GetDlgItem (hParams, IDC_MS_EDGE_RING_SEL_SPIN), edge);
		ReleaseISpinner (spin);
	}


	but = GetICustButton (GetDlgItem (hParams, IDC_MS_COPYNS));
	but->Enable (!obj);
	ReleaseICustButton(but);
	but = GetICustButton (GetDlgItem (hParams,IDC_MS_PASTENS));
	but->Enable (!obj && GetMeshNamedSelClip (namedClipLevel[selLevel]));
	ReleaseICustButton(but);

	but = GetICustButton (GetDlgItem (hParams, IDC_MS_SELECT_OPEN));
	but->Enable (edge);
	ReleaseICustButton (but);
}

INT_PTR SelectProc::DlgProc (TimeValue t, IParamMap2 *map,
										HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (!mod) 
		return FALSE;

	ICustToolbar	*l_iToolbar = NULL ;
	ISpinnerControl *l_iSpin	= NULL; 
	int matid;

	switch (msg) {
	case WM_INITDIALOG:
		l_iToolbar = GetICustToolbar(GetDlgItem(hWnd,IDC_MS_SELTYPE));
		l_iToolbar->SetImage (GetPolySelImageHandler()->LoadImages());
		l_iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,0,5,0,5,24,23,24,23,IDC_SELVERTEX));
		l_iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,1,6,1,6,24,23,24,23,IDC_SELEDGE));
		l_iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,2,7,2,7,24,23,24,23,IDC_SELBORDER));
		l_iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,3,8,3,8,24,23,24,23,IDC_SELFACE));
		l_iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,4,9,4,9,24,23,24,23,IDC_SELELEMENT));
		ReleaseICustToolbar(l_iToolbar);

		l_iSpin = SetupIntSpinner (hWnd, IDC_MS_EDGE_RING_SEL_SPIN, IDC_MS_EDGE_RING_SEL_FOR_SPIN, -9999999, 9999999, 0);
		if (l_iSpin) {
			l_iSpin->SetAutoScale(FALSE);
			ReleaseISpinner (l_iSpin);
		}

		l_iSpin = SetupIntSpinner (hWnd, IDC_MS_EDGE_LOOP_SEL_SPIN, IDC_MS_EDGE_LOOP_SEL_FOR_SPIN, -9999999, 9999999, 0);
		if (l_iSpin) {
			l_iSpin->SetAutoScale(FALSE);
			ReleaseISpinner (l_iSpin);
		}

		UpdateSelLevelDisplay (hWnd);
		SetEnables (hWnd);
		break;

	case WM_UPDATE_CACHE:
		mod->UpdateCache((TimeValue)wParam);
 		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_MS_SELBYMAT:
			mod->pblock->GetValue (ps_matid, t, matid, FOREVER);
			mod->SelectByMatID(matid-1);
			break;

		case IDC_SELVERTEX:
			if (mod->selLevel == SEL_VERTEX) 
				mod->ip->SetSubObjectLevel (SEL_OBJECT);
			else {
				// russom - 1/20/04 - FID 1495
				if (GetKeyState (VK_CONTROL)<0) {
					bool requireAll = (GetKeyState (VK_SHIFT)<0) ? true : false;
					mod->ConvertSelection (SEL_CURRENT, SEL_VERTEX, requireAll);
				}
				else if (GetKeyState (VK_SHIFT)<0) 
				{
					// new with 8.0 convert it into the border vertices 
					mod->ConvertSelectionToBorder(SEL_CURRENT, SEL_VERTEX);
				}
				mod->ip->SetSubObjectLevel (SEL_VERTEX);
			}
			break;

		case IDC_SELEDGE:
			if (mod->selLevel == SEL_EDGE) 
				mod->ip->SetSubObjectLevel (SEL_OBJECT);
			else {
				// russom - 1/20/04 - FID 1495
				if (GetKeyState (VK_CONTROL)<0) {
					bool requireAll = (GetKeyState (VK_SHIFT)<0) ? true : false;
					mod->ConvertSelection (SEL_CURRENT, SEL_EDGE, requireAll);
				}
				else if (GetKeyState (VK_SHIFT)<0) 
				{
					// new with 8.0 convert it into the border vertices 
					mod->ConvertSelectionToBorder(SEL_CURRENT, SEL_EDGE);
				}

				mod->ip->SetSubObjectLevel (SEL_EDGE);
			}
			break;

		case IDC_SELBORDER:
			if (mod->selLevel == SEL_BORDER) 
				mod->ip->SetSubObjectLevel (SEL_OBJECT);
			else {
				// russom - 1/20/04 - FID 1495
				if (GetKeyState (VK_CONTROL)<0) {
					bool requireAll = (GetKeyState (VK_SHIFT)<0) ? true : false;
					mod->ConvertSelection (SEL_CURRENT, SEL_BORDER, requireAll);
				}
				mod->ip->SetSubObjectLevel (SEL_BORDER);
			}
			break;

		case IDC_SELFACE:
			if (mod->selLevel == SEL_FACE) 
				mod->ip->SetSubObjectLevel (SEL_OBJECT);
			else {
				// russom - 1/20/04 - FID 1495
				if (GetKeyState (VK_CONTROL)<0) {
					bool requireAll = (GetKeyState (VK_SHIFT)<0) ? true : false;
					mod->ConvertSelection (SEL_CURRENT, SEL_FACE, requireAll);
				}
				else if (GetKeyState (VK_SHIFT)<0) 
				{
					// new with 8.0 convert it into the border vertices 
					mod->ConvertSelectionToBorder(SEL_CURRENT, SEL_FACE);
				}

				mod->ip->SetSubObjectLevel (SEL_FACE);
			}
			break;

		case IDC_SELELEMENT:
			if (mod->selLevel == SEL_ELEMENT) 
				mod->ip->SetSubObjectLevel (SEL_OBJECT);
			else {
				// russom - 1/20/04 - FID 1495
				if (GetKeyState (VK_CONTROL)<0) {
					bool requireAll = (GetKeyState (VK_SHIFT)<0) ? true : false;
					mod->ConvertSelection (SEL_CURRENT, SEL_ELEMENT, requireAll);
				}
				mod->ip->SetSubObjectLevel (SEL_ELEMENT);
			}
			break;

		// russom - 1/20/04 - FID 1495
		case IDC_MS_SELECTION_GROW:
			mod->SelectionOp(epsop_sel_grow);
			break;
		case IDC_MS_SELECTION_SHRINK:
			mod->SelectionOp(epsop_sel_shrink);
			break;
		case IDC_MS_EDGE_RING_SEL:
			if( mod->selLevel == SEL_EDGE )
				mod->SelectionOp(epsop_select_ring);
			break;
		case IDC_MS_EDGE_LOOP_SEL:
			if( mod->selLevel == SEL_EDGE )
				mod->SelectionOp(epsop_select_loop);
			break;

		case IDC_MS_SELECT_OPEN: mod->SelectOpenEdges (); break;

		case IDC_MS_GETVERT: mod->SelectFrom(SEL_VERTEX); break;
		case IDC_MS_GETEDGE: mod->SelectFrom(SEL_EDGE); break;
		case IDC_MS_GETFACE: mod->SelectFrom(SEL_FACE); break;

		case IDC_MS_COPYNS:  mod->NSCopy();  break;
		case IDC_MS_PASTENS: mod->NSPaste(); break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code != TTN_NEEDTEXT) break;
		LPTOOLTIPTEXT lpttt;
		lpttt = (LPTOOLTIPTEXT)lParam;				
		switch (lpttt->hdr.idFrom) {
		case IDC_SELVERTEX:
			lpttt->lpszText = GetString (IDS_PM_VERTEX);
			break;
		case IDC_SELEDGE:
			lpttt->lpszText = GetString (IDS_PM_EDGE);
			break;
		case IDC_SELBORDER:
			lpttt->lpszText = GetString(IDS_PM_BORDER);
			break;
		case IDC_SELFACE:
			lpttt->lpszText = GetString(IDS_PM_FACE);
			break;
		case IDC_SELELEMENT:
			lpttt->lpszText = GetString(IDS_PM_ELEMENT);
			break;
		}
		break;

	case CC_SPINNER_BUTTONDOWN:
		{	
			switch (LOWORD(wParam)) 
			{
				case IDC_MS_EDGE_LOOP_SEL_SPIN:
				case IDC_MS_EDGE_RING_SEL_SPIN:	
				{
					

					bool			l_ctrl	= (GetKeyState (VK_CONTROL)< 0) ? true : false;
					bool			l_alt	= (GetKeyState (VK_MENU) < 0)?  true : false;
					ModContextList	list;
					INodeTab		nodes;
					PolyMeshSelData *l_polySelData = NULL ;


					mod->ip->GetModContexts(list,nodes);
					for (int m = 0; m < list.Count(); m++) 
					{
						l_polySelData = (PolyMeshSelData*)list[m]->localData;
						if (!l_polySelData) 
							continue;
						// this is needed in order to restore the selection after an undo 
						theHold.Begin();  // (uses restore object in paramblock.)

						theHold.Put (new SelRingLoopRestore(mod,l_polySelData));
						if (LOWORD(wParam) == IDC_MS_EDGE_LOOP_SEL_SPIN )
							theHold.Accept(GetString(IDS_RING_EDGE_SEL));
						else 
							theHold.Accept(GetString(IDS_LOOP_EDGE_SEL));

					}

					if ( l_alt || l_ctrl )
					{
						l_iSpin = (ISpinnerControl*)lParam;

						if ( l_iSpin )
						{
							// enable the one click behaviour : this allow to do 1 click , while using the 
							// ctl or alt key. laurent r 05/05
							l_iSpin->Execute(I_EXEC_SPINNER_ONE_CLICK_ENABLE);

							if ( l_alt )
							{
								// disable the alt slow down. 
								l_iSpin->Execute(I_EXEC_SPINNER_ALT_DISABLE);
							}
						}
					}


					mod->setAltKey(l_alt);
					mod->setCtrlKey(l_ctrl);
				}
				break;
			}
			break;
		}
		break;

	case CC_SPINNER_BUTTONUP:
		switch (LOWORD(wParam)) 
		{
		case IDC_MS_EDGE_LOOP_SEL_SPIN:
		case IDC_MS_EDGE_RING_SEL_SPIN:
			l_iSpin = (ISpinnerControl*)lParam;
			if ( l_iSpin )
			{
				// reset all 
				l_iSpin->Execute(I_EXEC_SPINNER_ALT_ENABLE);
				l_iSpin->Execute(I_EXEC_SPINNER_ONE_CLICK_DISABLE);
			}
			break;
		}
		break;

	case CC_SPINNER_CHANGE:
		switch (LOWORD(wParam)) 
		{
		
		case IDC_MS_EDGE_LOOP_SEL_SPIN:
		case IDC_MS_EDGE_RING_SEL_SPIN:
			bool			l_add			= mod->getCtrlKey();
			bool			l_remove		= mod->getAltKey() ;
			ModContextList	list;
			INodeTab		nodes;
		
			PolyMeshSelData *l_polySelData	= NULL ;

			l_iSpin = (ISpinnerControl*)lParam;

			if ( l_iSpin )
			{

				mod->ip->GetModContexts(list,nodes);
				for (int m = 0; m < list.Count(); m++) 
				{
					l_polySelData = (PolyMeshSelData*)list[m]->localData;
					if (!l_polySelData) 
						continue;

					if ( LOWORD(wParam) == IDC_MS_EDGE_LOOP_SEL_SPIN )
					{
						l_polySelData->SelectEdgeLoopShift(mod,l_iSpin->GetIVal() - mod->mLoopSpinner,l_remove,l_add );
						mod->mLoopSpinner = l_iSpin->GetIVal();
					}
					else 
					{
						l_polySelData->SelectEdgeRingShift(mod,l_iSpin->GetIVal() - mod->mRingSpinner,l_remove,l_add );
						mod->mRingSpinner = l_iSpin->GetIVal();
					}

				}
			}

			break;


		}

		break; 

	
	default: return FALSE;
	}
	return TRUE;
}

#define GRAPHSTEPS 20

void SoftSelProc::DrawCurve (HWND hWnd) {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd,&ps);

	float pinch, falloff, bubble;
	ISpinnerControl *spin = GetISpinner(GetDlgItem(hWnd,IDC_FALLOFFSPIN));
	falloff = spin->GetFVal();
	ReleaseISpinner(spin);	

	spin = GetISpinner(GetDlgItem(hWnd,IDC_PINCHSPIN));
	pinch = spin->GetFVal();
	ReleaseISpinner(spin);

	spin = GetISpinner(GetDlgItem(hWnd,IDC_BUBBLESPIN));
	bubble = spin->GetFVal();
	ReleaseISpinner(spin);	

	TSTR label = FormatUniverseValue(falloff);
	SetWindowText(GetDlgItem(hWnd,IDC_FARLEFTLABEL),label);
	SetWindowText(GetDlgItem(hWnd,IDC_FARRIGHTLABEL),label);
	label = FormatUniverseValue (0.0f);
	SetWindowText(GetDlgItem(hWnd,IDC_NEARLABEL),label);

	Rect rect, orect;
	GetClientRectP(GetDlgItem(hWnd,IDC_AR_GRAPH),&rect);
	orect = rect;

	SelectObject(hdc,GetStockObject(NULL_PEN));
	SelectObject(hdc,GetStockObject(WHITE_BRUSH));
	Rectangle(hdc,rect.left,rect.top,rect.right,rect.bottom);	
	SelectObject(hdc,GetStockObject(NULL_BRUSH));
	
	rect.left   += 3;
	rect.right  -= 3;
	rect.top    += 20;
	rect.bottom -= 20;
	
	SelectObject(hdc,CreatePen(PS_DOT,0,GetCustSysColor(COLOR_BTNFACE)));
	MoveToEx(hdc,orect.left,rect.top,NULL);
	LineTo(hdc,orect.right,rect.top);
	MoveToEx(hdc,orect.left,rect.bottom,NULL);
	LineTo(hdc,orect.right,rect.bottom);
	MoveToEx(hdc,(rect.left+rect.right)/2,orect.top,NULL);
	LineTo(hdc,(rect.left+rect.right)/2,orect.bottom);
	DeleteObject(SelectObject(hdc,GetStockObject(BLACK_PEN)));
	
	MoveToEx(hdc,rect.left,rect.bottom,NULL);
	for (int i=0; i<=GRAPHSTEPS; i++) {
		float dist = falloff * float(abs(i-GRAPHSTEPS/2))/float(GRAPHSTEPS/2);		
		float y = AffectRegionFunction (dist,falloff,pinch,bubble);
		int ix = rect.left + int(float(rect.w()-1) * float(i)/float(GRAPHSTEPS));
		int	iy = rect.bottom - int(y*float(rect.h()-2)) - 1;
		if (iy<orect.top) iy = orect.top;
		if (iy>orect.bottom-1) iy = orect.bottom-1;
		LineTo(hdc, ix, iy);
	}
	
	WhiteRect3D(hdc,orect,TRUE);
	EndPaint(hWnd,&ps);
}

void SoftSelProc::SetEnables (HWND hWnd) {
	bool obj = !mod || (mod->GetSelLevel() == SEL_OBJECT);
	EnableWindow (GetDlgItem (hWnd, IDC_USE_SOFTSEL), !obj);

	IParamBlock2 *pblock = mod->GetParamBlock (0);
	int softSel, useEDist;
	pblock->GetValue (ps_use_softsel, TimeValue(0), softSel, FOREVER);
	pblock->GetValue (ps_use_edist, TimeValue(0), useEDist, FOREVER);

	bool enable = (!obj && softSel) ? TRUE : FALSE;
	BOOL isPainting = (mod->GetPaintMode()==PAINTMODE_PAINTSEL? TRUE:FALSE);
	BOOL isBlurring = (mod->GetPaintMode()==PAINTMODE_BLURSEL? TRUE:FALSE);
	BOOL isErasing  = (mod->GetPaintMode()==PAINTMODE_ERASESEL? TRUE:FALSE);

	EnableWindow (GetDlgItem (hWnd, IDC_LOCK_SOFTSEL), enable);
	EnableWindow (GetDlgItem (hWnd, IDC_PAINTSEL_VALUE_LABEL), enable);
	EnableWindow (GetDlgItem (hWnd, IDC_PAINTSEL_SIZE_LABEL), enable);
	EnableWindow (GetDlgItem (hWnd, IDC_PAINTSEL_STRENGTH_LABEL), enable);
	ICustButton* but = GetICustButton(GetDlgItem(hWnd,IDC_PAINTSEL_PAINT));
	but->SetCheck( isPainting );
	but->Enable (enable);
	ReleaseICustButton(but);
	but = GetICustButton(GetDlgItem(hWnd,IDC_PAINTSEL_BLUR));
	but->SetCheck( isBlurring );
	but->Enable (enable);
	ReleaseICustButton(but);
	but = GetICustButton(GetDlgItem(hWnd,IDC_PAINTSEL_REVERT));
	but->SetCheck( isErasing );
	but->Enable (enable);
	ReleaseICustButton(but);
	ISpinnerControl* spin = NULL;
	spin = GetISpinner (GetDlgItem (hWnd, IDC_PAINTSEL_VALUE_SPIN));
	spin->Enable (enable);
	ReleaseISpinner (spin);
	spin = GetISpinner (GetDlgItem (hWnd, IDC_PAINTSEL_SIZE_SPIN));
	spin->Enable (enable);
	ReleaseISpinner (spin);
	spin = GetISpinner (GetDlgItem (hWnd, IDC_PAINTSEL_STRENGTH_SPIN));
	spin->Enable (enable);
	ReleaseISpinner (spin);

	//If the selection is locked, disable the remaining controls
	if( pblock->GetInt(ps_lock_softsel, TimeValue(0)) )
		enable = FALSE;
	EnableWindow (GetDlgItem (hWnd, IDC_USE_EDIST), enable);
	EnableWindow (GetDlgItem (hWnd, IDC_AFFECT_BACKFACING), enable);
	spin = GetISpinner (GetDlgItem (hWnd, IDC_EDIST_SPIN));
	spin->Enable (enable && useEDist);
	ReleaseISpinner (spin);
	spin = GetISpinner (GetDlgItem (hWnd, IDC_FALLOFFSPIN));
	spin->Enable (enable);
	ReleaseISpinner (spin);
	spin = GetISpinner (GetDlgItem (hWnd, IDC_PINCHSPIN));
	spin->Enable (enable);
	ReleaseISpinner (spin);
	spin = GetISpinner (GetDlgItem (hWnd, IDC_BUBBLESPIN));
	spin->Enable (enable);
	ReleaseISpinner (spin);
	EnableWindow (GetDlgItem (hWnd, IDC_FALLOFF_LABEL), enable);
	EnableWindow (GetDlgItem (hWnd, IDC_PINCH_LABEL), enable);
	EnableWindow (GetDlgItem (hWnd, IDC_BUBBLE_LABEL), enable);
}

INT_PTR SoftSelProc::DlgProc (TimeValue t, IParamMap2 *map,
										HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (!mod) return FALSE;
	Rect rect;

	switch (msg) {
	case WM_INITDIALOG: {
			ICustButton *btnPaint = GetICustButton(GetDlgItem(hWnd,IDC_PAINTSEL_PAINT));
			ICustButton *btnBlur  = GetICustButton(GetDlgItem(hWnd,IDC_PAINTSEL_BLUR));
			ICustButton *btnErase = GetICustButton(GetDlgItem(hWnd,IDC_PAINTSEL_REVERT));
			if( btnPaint ) btnPaint->SetType(CBT_CHECK), ReleaseICustButton(btnPaint);
			if( btnBlur )  btnBlur->SetType(CBT_CHECK),  ReleaseICustButton(btnBlur);
			if( btnErase ) btnErase->SetType(CBT_CHECK), ReleaseICustButton(btnErase);
			ShowWindow(GetDlgItem(hWnd,IDC_AR_GRAPH),SW_HIDE);
			SetEnables (hWnd);
		}
		break;
		
	case WM_PAINT:
		DrawCurve(hWnd);
		return FALSE;

	case CC_SPINNER_CHANGE:
		switch (LOWORD(wParam)) {
		case IDC_EDIST_SPIN:
		case IDC_FALLOFFSPIN:
		case IDC_PINCHSPIN:
		case IDC_BUBBLESPIN:
			GetClientRectP(GetDlgItem(hWnd,IDC_AR_GRAPH),&rect);
			InvalidateRect(hWnd,&rect,FALSE);
			break;
		}
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_USE_SOFTSEL:
		case IDC_USE_EDIST:
			SetEnables (hWnd);
			break;
		case IDC_PAINTSEL_PAINT:
			mod->OnPaintBtn( PAINTMODE_PAINTSEL, t );
			SetEnables (hWnd);
			break;
		case IDC_PAINTSEL_BLUR:
			mod->OnPaintBtn( PAINTMODE_BLURSEL, t );
			SetEnables (hWnd);
			break;
		case IDC_PAINTSEL_REVERT:
			mod->OnPaintBtn( PAINTMODE_ERASESEL, t );
			SetEnables (hWnd);
			break;
		case IDC_PAINTSEL_OPTIONS:
			GetMeshPaintMgr()->BringUpOptions();
			break;
		}
		return false;

	default:
		return FALSE;
	}
	return TRUE;
}

static INT_PTR CALLBACK PickSetNameDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static TSTR *name;

	switch (msg) {
		case WM_INITDIALOG: {
			name = (TSTR*)lParam;
			ICustEdit *edit =GetICustEdit(GetDlgItem(hWnd,IDC_SET_NAME));
			edit->SetText(*name);
			ReleaseICustEdit(edit);
			break;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK: {
					ICustEdit *edit =GetICustEdit(GetDlgItem(hWnd,IDC_SET_NAME));
					TCHAR buf[256];
					edit->GetText(buf,256);
					*name = TSTR(buf);
					ReleaseICustEdit(edit);
					EndDialog(hWnd,1);
					break;
					}

				case IDCANCEL:
					EndDialog(hWnd,0);
					break;
				}
			break;

		default:
			return FALSE;
		};
	return TRUE;
	}

BOOL PolyMeshSelMod::GetUniqueSetName(TSTR &name) {
	while (1) {				
		Tab<TSTR*> &setList = namedSel[namedSetLevel[selLevel]];

		BOOL unique = TRUE;
		for (int i=0; i<setList.Count(); i++) {
			if (name==*setList[i]) {
				unique = FALSE;
				break;
			}
		}
		if (unique) break;

		if (!ip) return FALSE;
		if (!DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_NAMEDSET_PASTE),
			ip->GetMAXHWnd(), PickSetNameDlgProc, (LPARAM)&name)) return FALSE;
	}
	return TRUE;
}

static INT_PTR CALLBACK PickSetDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:	{
		Tab<TSTR*> *setList = (Tab<TSTR*>*)lParam;
		for (int i=0; i<setList->Count(); i++) {
			int pos  = SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_ADDSTRING,0,
				(LPARAM)(TCHAR*)*(*setList)[i]);
			SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_SETITEMDATA,pos,i);
		}
		break;
	}

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_NS_LIST:
			if (HIWORD(wParam)!=LBN_DBLCLK) break;
			// fall through

		case IDOK:
			int sel;
			sel = SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_GETCURSEL,0,0);
			if (sel!=LB_ERR) {
				int res =SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_GETITEMDATA,sel,0);
				EndDialog(hWnd,res);
				break;
			}
			// fall through

		case IDCANCEL:
			EndDialog(hWnd,-1);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}


// Methods from MeshPaintHandler
void PolyMeshSelMod::GetPaintHosts( Tab<MeshPaintHost*>& hosts, Tab<INode*>& nodes ) {
	hosts.SetCount(0), nodes.SetCount(0);
	FOR_EACH_MODDATA {
		MeshPaintHost* host = modData;
		hosts.Append( 1, &(host) );
		nodes.Append( 1, &(_nodes[i]) );
	}
}

float PolyMeshSelMod::GetPaintValue( PaintModeID paintMode ) {
	if( (pblock==NULL) || (!IsPaintSelMode(paintMode)) ) return 0;
	return pblock->GetFloat(ps_paintsel_value);
}

void PolyMeshSelMod::SetPaintValue( PaintModeID paintMode, float value ) {
	if( (pblock==NULL) || (!IsPaintSelMode(paintMode)) ) return;
	pblock->SetValue(ps_paintsel_value,0,value);
}

MNMesh* PolyMeshSelMod::GetPaintObject( MeshPaintHost* host ) {
	PolyMeshSelData* modData = static_cast<PolyMeshSelData*>(host);
	return modData->GetMesh();
}

void PolyMeshSelMod::GetPaintInputSel( MeshPaintHost* host, FloatTab& selValues ) {
	PolyMeshSelData* modData = static_cast<PolyMeshSelData*>(host);

	GenSoftSelData softSelData;
	GetSoftSelData( softSelData, GetCOREInterface()->GetTime() );
	FloatTab* weights = modData->GetTempData()->VSWeight(
		softSelData.useEdgeDist, softSelData.edgeIts, softSelData.ignoreBack,
		softSelData.falloff, softSelData.pinch, softSelData.bubble );

	selValues = *weights;
}

void PolyMeshSelMod::GetPaintInputVertPos( MeshPaintHost* host, Point3Tab& vertPos ) {
	PolyMeshSelData* modData = static_cast<PolyMeshSelData*>(host);
	MNMesh* mesh = modData->GetMesh();
	int count = mesh->VNum();
	vertPos.SetCount( count );
	for( int i=0; i<count; i++ ) vertPos[i] = mesh->V(i)->p;
}


void PolyMeshSelMod::ApplyPaintSel( MeshPaintHost* host ) {
	PolyMeshSelData* modData = static_cast<PolyMeshSelData*>(host);
	modData->InvalidateVDistances();
}

void PolyMeshSelMod::OnPaintModeChanged( PaintModeID prevMode, PaintModeID curMode )
	{ SetSoftSelEnables(); }
void PolyMeshSelMod::OnPaintDataActivate( PaintModeID paintMode, BOOL onOff ) {
	SetSoftSelEnables();
	InvalidateDialogElement( ps_lock_softsel );
}

void PolyMeshSelMod::OnPaintDataRedraw( PaintModeID prevMode ) {
	NotifyDependents(FOREVER, PART_DISPLAY | PART_SELECT, REFMSG_CHANGE);
	if (ip) ip->RedrawViews( ip->GetTime() );
}

void PolyMeshSelMod::OnPaintBrushChanged() {
	InvalidateDialogElement( ps_paintsel_size );
	InvalidateDialogElement( ps_paintsel_strength );
}

// Helper methods
static HWND GetHWnd( IParamBlock2* pblock, DWORD mapID ) {
	if( pblock==NULL ) return NULL;
	ParamBlockDesc2* blockDesc = pblock->GetDesc();
	if( blockDesc==NULL ) return NULL;
	ClassDesc2* classDesc = blockDesc->cd;
	if( classDesc==NULL ) return NULL;
	IParamMap2 *pmap = classDesc->GetParamMap(blockDesc,mapID);
	return (pmap==NULL?  NULL : pmap->GetHWnd());
}

static BOOL IsButtonEnabled(HWND hParent, DWORD id) {
	if( hParent==NULL ) return FALSE;
	ICustButton* but = GetICustButton(GetDlgItem(hParent,id));
	BOOL retval = but->IsEnabled();
	ReleaseICustButton(but);
	return retval;
}

// Event handlers
void PolyMeshSelMod::OnParamSet(PB2Value& v, ParamID id, int tabIndex, TimeValue t) {
	switch( id ) {
	case ps_use_softsel:
		//When the soft selection is turned off, end any active paint session
		if( (!v.i) && InPaintMode() ) EndPaintMode();
		break;
	case ps_lock_softsel:	{
			BOOL selActive = IsPaintDataActive(PAINTMODE_SEL);
			if( v.i && !selActive ) //Enable the selection lock; Activate the paintmesh selection
				ActivatePaintData(PAINTMODE_SEL);
			else if( (!v.i) && selActive ) { //Disable the selection lock; De-activate the paintmesh selection
				if( GetPaintMode()!=PAINTMODE_OFF ) EndPaintMode(); //end any active paint mode
				DeactivatePaintData(PAINTMODE_SEL);
			}
			break;
		}
	case ps_paintsel_size:
		SetPaintBrushSize( v.f );
		UpdatePaintBrush();
		break;
	case ps_paintsel_strength:
		SetPaintBrushStrength( v.f );
		UpdatePaintBrush();
		break;
	case ps_paintsel_value:
		UpdatePaintBrush();
		break;
	case ps_paintsel_mode:
		if( v.i!=GetPaintMode() ) {
			BOOL valid = FALSE;
			HWND hwnd = GetHWnd( pblock, ps_map_softsel );
			if( v.i==PAINTMODE_PAINTSEL )	valid = IsButtonEnabled( hwnd, IDC_PAINTSEL_PAINT );
			if( v.i==PAINTMODE_BLURSEL )	valid = IsButtonEnabled( hwnd, IDC_PAINTSEL_BLUR );
			if( v.i==PAINTMODE_ERASESEL )	valid = IsButtonEnabled( hwnd, IDC_PAINTSEL_REVERT );
			if( v.i==PAINTMODE_OFF )		valid = TRUE;
			if( valid ) OnPaintBtn(v.i, t);
		}
		break;
	}
}

void PolyMeshSelMod::OnParamGet(PB2Value& v, ParamID id, int tabIndex, TimeValue t, Interval &valid) {
	switch( id ) {
	//The selection lock is defined as being "on" anytime the paintmesh selection is active
	case ps_lock_softsel:		v.i = IsPaintDataActive(PAINTMODE_PAINTSEL); break;
	case ps_paintsel_size:		v.f = GetPaintBrushSize(); break;
	case ps_paintsel_strength:	v.f = GetPaintBrushStrength(); break;
	case ps_paintsel_mode:
		v.i = GetPaintMode();
		if( !IsPaintSelMode(v.i) ) v.i=PAINTMODE_OFF;
		break;
	}
}

void PolyMeshSelMod::OnPaintBtn( int paintMode, TimeValue t ) {
	if( (paintMode==GetPaintMode()) || (paintMode==PAINTMODE_OFF) ) EndPaintMode();
	else {
		if( !pblock->GetInt(ps_use_softsel) )	pblock->SetValue( ps_use_softsel, t, TRUE );	//enable soft selection
		if( !pblock->GetInt(ps_lock_softsel) )	pblock->SetValue( ps_lock_softsel, t, TRUE );	//enable the soft selection lock
		BeginPaintMode( (PaintModeID)paintMode );
	}
}

TCHAR *PolyMeshSelMod::GetPaintUndoString (PaintModeID mode) {
	if (IsPaintSelMode(mode)) return GetString (IDS_RESTOREOBJ_MESHPAINTSEL);
	return GetString (IDS_RESTOREOBJ_MESHPAINTDEFORM);
}


// PolyMeshSelData -----------------------------------------------------

LocalModData *PolyMeshSelData::Clone() {
	PolyMeshSelData *d = new PolyMeshSelData;
	d->vertSel = vertSel;
	d->faceSel = faceSel;
	d->edgeSel = edgeSel;
	d->vselSet = vselSet;
	d->eselSet = eselSet;
	d->fselSet = fselSet;
	d->paintSelCount = paintSelCount;
	d->paintSelBuf = paintSelBuf;
	return d;
}

PolyMeshSelData::PolyMeshSelData(MNMesh &mesh) : mpNewSelection(NULL), held(false), mesh(NULL), temp(NULL) {
	mesh.getVertexSel (vertSel);
	mesh.getFaceSel (faceSel);
	mesh.getEdgeSel (edgeSel);
	vdValid = NEVER;
}

void PolyMeshSelData::SynchBitArrays() {
	if (!mesh) return;
	if (vertSel.GetSize() != mesh->VNum()) vertSel.SetSize(mesh->VNum(),TRUE);
	if (faceSel.GetSize() != mesh->FNum()) faceSel.SetSize(mesh->FNum(),TRUE);
	if (edgeSel.GetSize() != mesh->ENum()) edgeSel.SetSize(mesh->ENum(),TRUE);
	UpdatePaintObject(mesh);
}

void PolyMeshSelData::SetCache(MNMesh &mesh) {
	if (this->mesh) delete this->mesh;
	this->mesh = new MNMesh(mesh);
	if (temp) delete temp;
	temp = NULL;
	SynchBitArrays ();
}

void PolyMeshSelData::FreeCache() {
	if (mesh) delete mesh;
	mesh = NULL;
	if (temp) delete temp;
	temp = NULL;
}

void PolyMeshSelData::SetVertSel(BitArray &set, IMeshSelect *imod, TimeValue t) {
	PolyMeshSelMod *mod = (PolyMeshSelMod *) imod;
	if (theHold.Holding()) theHold.Put (new SelectRestore (mod, this, SEL_VERTEX));
	vertSel = set;
	if (mesh) mesh->VertexSelect (set);
	InvalidateVDistances ();
}

void PolyMeshSelData::SetFaceSel(BitArray &set, IMeshSelect *imod, TimeValue t) {
	PolyMeshSelMod *mod = (PolyMeshSelMod *) imod;
	if (theHold.Holding()) theHold.Put (new SelectRestore (mod, this, SEL_FACE));
	faceSel = set;
	if (mesh) mesh->FaceSelect (set);
	InvalidateVDistances ();
}

void PolyMeshSelData::SetEdgeSel(BitArray &set, IMeshSelect *imod, TimeValue t) {
	PolyMeshSelMod *mod = (PolyMeshSelMod *) imod;
	if (theHold.Holding()) theHold.Put (new SelectRestore (mod, this, SEL_EDGE));
	edgeSel = set;
	if (mesh) mesh->EdgeSelect (set);
	InvalidateVDistances ();
}

void PolyMeshSelData::SelVertByFace() {
	DbgAssert (mesh);
	if (!mesh) return;
	for (int i=0; i<mesh->FNum(); i++) {
		if (!faceSel[i]) continue;
		MNFace *mf = mesh->F(i);
		if (mf->GetFlag (MN_DEAD)) continue;
		for (int j=0; j<mf->deg; j++) vertSel.Set(mf->vtx[j]);
	}
}

void PolyMeshSelData::SelVertByEdge() {
	DbgAssert (mesh);
	if (!mesh) return;
	if (!mesh->GetFlag (MN_MESH_FILLED_IN)) return;
	for (int i=0; i<mesh->ENum(); i++) {
		if (!edgeSel[i]) continue;
		MNEdge *me = mesh->E(i);
		if (me->GetFlag (MN_DEAD)) continue;
		vertSel.Set(me->v1);
		vertSel.Set(me->v2);
	}
}

void PolyMeshSelData::SelFaceByVert() {
	DbgAssert (mesh);
	if (!mesh) return;
	for (int i=0; i<mesh->FNum(); i++) {
		MNFace * mf = mesh->F(i);
      int j;
		for (j=0; j<mf->deg; j++) if (vertSel[mf->vtx[j]]) break;
		if (j<mf->deg) faceSel.Set (i);
	}
}

void PolyMeshSelData::SelFaceByEdge() {
	DbgAssert (mesh);
	if (!mesh) return;
	if (!mesh->GetFlag (MN_MESH_FILLED_IN)) return;
	for (int i=0; i<mesh->FNum(); i++) {
		MNFace * mf = mesh->F(i);
      int j;
		for (j=0; j<mf->deg; j++) if (edgeSel[mf->edg[j]]) break;
		if (j<mf->deg) faceSel.Set(i);
	}
}

void PolyMeshSelData::SelElementByVert () {
	DbgAssert (mesh);
	if (!mesh) return;
	BitArray nfsel;
	nfsel.SetSize (mesh->FNum());
	for (int i=0; i<mesh->FNum(); i++) {
		MNFace * mf = mesh->F(i);
      int j;
		for (j=0; j<mf->deg; j++) if (vertSel[mf->vtx[j]]) break;
		if (j==mf->deg) continue;
		mesh->ElementFromFace (i, nfsel);
	}
	faceSel |= nfsel;
}

void PolyMeshSelData::SelElementByEdge () {
	DbgAssert (mesh);
	if (!mesh) return;
	if (!mesh->GetFlag (MN_MESH_FILLED_IN)) return;
	BitArray nfsel;
	nfsel.SetSize (mesh->FNum());
	for (int i=0; i<mesh->FNum(); i++) {
		MNFace * mf = mesh->F(i);
		for (int j=0; j<mf->deg; j++) {
			if (edgeSel[mf->edg[j]]) {
				mesh->ElementFromFace (i, nfsel);
				break;
			}
		}
	}
	faceSel |= nfsel;
}

void PolyMeshSelData::SelEdgeByVert() {
	DbgAssert (mesh);
	if (!mesh) return;
	if (!mesh->GetFlag (MN_MESH_FILLED_IN)) return;
	for (int i=0; i<mesh->ENum(); i++) {
		MNEdge *me = mesh->E(i);
		if (me->GetFlag (MN_DEAD)) continue;
		if (vertSel[me->v1] || vertSel[me->v2]) edgeSel.Set (i);
	}
}

void PolyMeshSelData::SelEdgeByFace() {
	DbgAssert (mesh);
	if (!mesh) return;
	if (!mesh->GetFlag (MN_MESH_FILLED_IN)) return;
	for (int i=0; i<mesh->FNum(); i++) {
		if (!faceSel[i]) continue;
		MNFace *mf = mesh->F(i);
		if (mf->GetFlag (MN_DEAD)) continue;
		for (int j=0; j<mf->deg; j++) edgeSel.Set(mf->edg[j]);
	}
}

void PolyMeshSelData::SelBorderByVert() {
	DbgAssert (mesh);
	if (!mesh) return;
	if (!mesh->GetFlag (MN_MESH_FILLED_IN)) return;
	BitArray nesel;
	nesel.SetSize (mesh->ENum());
	for (int i=0; i<mesh->ENum(); i++) {
		MNEdge *me = mesh->E(i);
		if (me->GetFlag (MN_DEAD)) continue;
		if (me->f2 > -1) continue;
		if (vertSel[me->v1] || vertSel[me->v2]) mesh->BorderFromEdge (i, nesel);
	}
	edgeSel |= nesel;
}

void PolyMeshSelData::SelBorderByFace() {
	DbgAssert (mesh);
	if (!mesh) return;
	if (!mesh->GetFlag (MN_MESH_FILLED_IN)) return;
	BitArray nesel;
	nesel.SetSize (mesh->ENum());
	for (int i=0; i<mesh->FNum(); i++) {
		MNFace *mf = mesh->F(i);
		if (!faceSel[i]) continue;
		if (mf->GetFlag (MN_DEAD)) continue;
		for (int j=0; j<mf->deg; j++) mesh->BorderFromEdge (mf->edg[j], nesel);
	}
	edgeSel |= nesel;
}

// russom - 1/20/04 - FID 1495
void PolyMeshSelData::GrowSelection (IMeshSelect *imod, int level) {
	DbgAssert (mesh);
	if( !mesh ) return;

	BitArray newSel;
	int mnSelLevel = mesh->selLevel;
	if (mnSelLevel == MNM_SL_CURRENT) mnSelLevel = meshSelLevel[level];
	DbgAssert (mesh->GetFlag (MN_MESH_FILLED_IN));
	if (!mesh->GetFlag (MN_MESH_FILLED_IN)) return;

	SynchBitArrays();

	switch (level) {
	case SEL_VERTEX: 
		mesh->ClearEFlags (MN_USER);
		mesh->PropegateComponentFlags (MNM_SL_EDGE, MN_USER, mnSelLevel, MN_SEL);
		newSel.SetSize (mesh->numv);
		for (int i=0; i<mesh->nume; i++) {
			if (mesh->e[i].GetFlag (MN_USER)) {
				newSel.Set (mesh->e[i].v1);
				newSel.Set (mesh->e[i].v2);
			}
		}
		SetVertSel (newSel, imod, TimeValue(0));
		break;
	case SEL_EDGE:
		mesh->ClearVFlags (MN_USER);
		mesh->PropegateComponentFlags (MNM_SL_VERTEX, MN_USER, mnSelLevel, MN_SEL);
		newSel.SetSize (mesh->nume);
		for (int i=0; i<mesh->nume; i++) {
			if (mesh->v[mesh->e[i].v1].GetFlag (MN_USER) || mesh->v[mesh->e[i].v2].GetFlag (MN_USER))
				newSel.Set (i);
		}
		SetEdgeSel (newSel, imod, TimeValue(0));
		break;
	case SEL_FACE:
		mesh->ClearVFlags (MN_USER);
		mesh->PropegateComponentFlags (MNM_SL_VERTEX, MN_USER, mnSelLevel, MN_SEL);
		newSel.SetSize (mesh->numf);
		for (int i=0; i<mesh->numf; i++) {
         int j;
			for (j=0; j<mesh->f[i].deg; j++) {
				if (mesh->v[mesh->f[i].vtx[j]].GetFlag (MN_USER)) break;
			}
			if (j<mesh->f[i].deg) newSel.Set (i);
		}
		SetFaceSel (newSel, imod, TimeValue(0));
		break;
	}
}

void PolyMeshSelData::ShrinkSelection (IMeshSelect *imod, int level) {
	DbgAssert (mesh);
	if( !mesh ) return;

	BitArray newSel;
	int mnSelLevel = mesh->selLevel;
	if (mnSelLevel == MNM_SL_CURRENT) mnSelLevel = meshSelLevel[level];
	DbgAssert (mesh->GetFlag (MN_MESH_FILLED_IN));
	if (!mesh->GetFlag (MN_MESH_FILLED_IN)) return;

	SynchBitArrays();

	switch (level) {
	case SEL_VERTEX: 
		// Find the edges between two selected vertices.
		mesh->ClearEFlags (MN_USER);
		mesh->PropegateComponentFlags (MNM_SL_EDGE, MN_USER, mnSelLevel, MN_SEL, true);
		if (vertSel.GetSize() != mesh->numv) mesh->getVertexSel (vertSel);
		newSel = vertSel;
		// De-select all the vertices touching edges to unselected vertices:
		for (int i=0; i<mesh->nume; i++) {
			if (!mesh->e[i].GetFlag (MN_USER)) {
				newSel.Clear (mesh->e[i].v1);
				newSel.Clear (mesh->e[i].v2);
			}
		}
		SetVertSel (newSel, imod, TimeValue(0));
		break;
	case SEL_EDGE:
		// Find all vertices used by only selected edges:
		mesh->ClearVFlags (MN_USER);
		mesh->PropegateComponentFlags (MNM_SL_VERTEX, MN_USER, mnSelLevel, MN_SEL, true);
		if (edgeSel.GetSize() != mesh->nume) mesh->getEdgeSel (edgeSel);
		newSel = edgeSel;
		for (int i=0; i<mesh->nume; i++) {
			// Deselect edges with at least one vertex touching a nonselected edge:
			if (!mesh->v[mesh->e[i].v1].GetFlag (MN_USER) || !mesh->v[mesh->e[i].v2].GetFlag (MN_USER))
				newSel.Clear (i);
		}
		SetEdgeSel (newSel, imod, TimeValue(0));
		break;
	case SEL_FACE:
		// Find all vertices used by only selected faces:
		mesh->ClearVFlags (MN_USER);
		mesh->PropegateComponentFlags (MNM_SL_VERTEX, MN_USER, mnSelLevel, MN_SEL, true);
		if (faceSel.GetSize() != mesh->numf) mesh->getFaceSel (faceSel);
		newSel = faceSel;
		for (int i=0; i<mesh->numf; i++) {
         int j;
			for (j=0; j<mesh->f[i].deg; j++) {
				if (!mesh->v[mesh->f[i].vtx[j]].GetFlag (MN_USER)) break;
			}
			// Deselect faces with at least one vertex touching a nonselected face:
			if (j<mesh->f[i].deg) newSel.Clear (i);
		}
		SetFaceSel (newSel, imod, TimeValue(0));
		break;
	}
}

void PolyMeshSelData::SelectEdgeRing(IMeshSelect *imod) {
	DbgAssert (mesh);
	if( !mesh ) return;
	BitArray l_currentSel = GetEdgeSel();
	mesh->SelectEdgeRing (l_currentSel);
	SetEdgeSel (l_currentSel, imod, TimeValue(0));
}

void PolyMeshSelData::SelectEdgeLoop(IMeshSelect *imod) {
	DbgAssert (mesh);
	if( !mesh ) return;
	BitArray l_currentSel = GetEdgeSel();
	mesh->SelectEdgeLoop (l_currentSel);
	SetEdgeSel (l_currentSel, imod, TimeValue(0));
}

void PolyMeshSelData::SelectEdgeLoopShift(IMeshSelect *imod, const int in_loop_shift, const bool in_remove, const bool in_add) 
{
	DbgAssert (mesh);
	if( !mesh ) return;

	BitArray l_newSel;
	l_newSel.SetSize(mesh->nume);
	l_newSel.ClearAll();

	
	BitArray l_currentSel = GetEdgeSel();

	IMNMeshUtilities8* l_meshToBridge = static_cast<IMNMeshUtilities8*>(mesh->GetInterface( IMNMESHUTILITIES8_INTERFACE_ID ));

	if (l_meshToBridge )
	{
		if ( in_loop_shift < 0 )
			l_meshToBridge->SelectEdgeLoopShift (-1,l_newSel);
		else if ( in_loop_shift > 0 )
			l_meshToBridge->SelectEdgeLoopShift (1,l_newSel);

		// update local value 
		if ( in_add )
		{
			// add the current selection to the shifted selection
			l_newSel |= l_currentSel ;
		}
		else if ( in_remove )
		{
			l_newSel &= l_currentSel;
		}

		if ( !l_newSel.IsEmpty()  )
		{
			PolyMeshSelMod *mod = (PolyMeshSelMod *) imod;
			mod->ClearSelection(SEL_EDGE);
			// update the mesh selection with a new bitarray
			SetEdgeSel (l_newSel, imod, TimeValue(0));
		}

	}

	
}

void PolyMeshSelData::SelectEdgeRingShift(IMeshSelect *imod, const int in_ring_shift,const bool in_remove, const bool in_add) 
{
	DbgAssert (mesh);
	if( !mesh ) 
		return;

	BitArray l_currentSel = GetEdgeSel();
	BitArray l_newSel;

	l_newSel.SetSize(mesh->nume);
	l_newSel.ClearAll();

	IMNMeshUtilities8* l_meshToBridge = static_cast<IMNMeshUtilities8*>(mesh->GetInterface( IMNMESHUTILITIES8_INTERFACE_ID ));
	if (l_meshToBridge )
	{
		if ( in_ring_shift <= 0 )
			l_meshToBridge->SelectEdgeRingShift (-1,l_newSel);
		else 
			l_meshToBridge->SelectEdgeRingShift (1,l_newSel);

		if ( in_add )
		{
			// add the current selection to the shifted selection
			l_newSel |= l_currentSel ;
		}
		else if ( in_remove )
		{
			l_newSel &= l_currentSel;
		}

		if ( !l_newSel.IsEmpty()  )
		{
			PolyMeshSelMod *mod = (PolyMeshSelMod *) imod;
			mod->ClearSelection(SEL_EDGE);
			// update the mesh selection with a new bitarray
			SetEdgeSel (l_newSel, imod, TimeValue(0));
		}
	}
}

// SelectRestore --------------------------------------------------

SelectRestore::SelectRestore(PolyMeshSelMod *m, PolyMeshSelData *data) {
	mod     = m;
	level   = mod->selLevel;
	d       = data;
	d->held = TRUE;
	switch (level) {
	case SEL_OBJECT: DbgAssert(0); break;
	case SEL_VERTEX: usel = d->vertSel; break;
	case SEL_EDGE:
	case SEL_BORDER:
		usel = d->edgeSel;
		break;
	default:
		usel = d->faceSel;
		break;
	}
}

SelectRestore::SelectRestore(PolyMeshSelMod *m, PolyMeshSelData *data, int sLevel) {
	mod     = m;
	level   = sLevel;
	d       = data;
	d->held = TRUE;
	switch (level) {
	case SEL_OBJECT: DbgAssert(0); break;
	case SEL_VERTEX: usel = d->vertSel; break;
	case SEL_EDGE:
	case SEL_BORDER:
		usel = d->edgeSel; break;
	default:
		usel = d->faceSel; break;
	}
}

void SelectRestore::Restore(int isUndo) {
	if (isUndo) {
		switch (level) {			
		case SEL_VERTEX:
			rsel = d->vertSel; break;
		case SEL_FACE: 
		case SEL_ELEMENT:
			rsel = d->faceSel; break;
		case SEL_EDGE:
		case SEL_BORDER:
			rsel = d->edgeSel; break;
		}
	}
	switch (level) {		
	case SEL_VERTEX:
		d->vertSel = usel; break;
	case SEL_FACE:
	case SEL_ELEMENT:
		d->faceSel = usel; break;
	case SEL_BORDER:
	case SEL_EDGE:
		d->edgeSel = usel; break;
	}
	mod->LocalDataChanged ();
}

void SelectRestore::Redo() {
	switch (level) {		
	case SEL_VERTEX:
		d->vertSel = rsel; break;
	case SEL_FACE:
	case SEL_ELEMENT:
		d->faceSel = rsel; break;
	case SEL_EDGE:
	case SEL_BORDER:
		d->edgeSel = rsel; break;
	}
	mod->LocalDataChanged ();
}

// PSComponentFlagRestore --------------------------------------------------

PSComponentFlagRestore::PSComponentFlagRestore (PolyMeshSelMod *m, PolyMeshSelData *d, int selLevel) {
	mod = m;
	data = d;
	data->held = TRUE;
	sl = selLevel;
	int i;
	
	switch (sl) {
	case MNM_SL_OBJECT:
		return;

	case MNM_SL_VERTEX:
		undo.SetCount (data->mesh->numv);
		for (i=0; i<undo.Count(); i++) {
			undo[i] = data->mesh->v[i].ExportFlags ();
		}
		break;

	case MNM_SL_EDGE:
		undo.SetCount (data->mesh->nume);
		for (i=0; i<undo.Count(); i++) {
			undo[i] = data->mesh->e[i].ExportFlags ();
		}
		break;

	case MNM_SL_FACE:
		undo.SetCount (data->mesh->numf);
		for (i=0; i<undo.Count(); i++) {
			undo[i] = data->mesh->f[i].ExportFlags ();
		}
		break;
	}
}

void PSComponentFlagRestore::Restore (int isUndo) {
	int i, max = undo.Count ();

	switch (sl) {
	case MNM_SL_OBJECT:
		return;

	case MNM_SL_VERTEX:
		redo.SetCount (data->mesh->numv);
		for (i=0; i<redo.Count(); i++) {
			redo[i] = data->mesh->v[i].ExportFlags ();
			if (i<max) data->mesh->v[i].ImportFlags (undo[i]);
		}
		data->mesh->getVertexSel (data->vertSel);
		break;

	case MNM_SL_EDGE:
		redo.SetCount (data->mesh->nume);
		for (i=0; i<redo.Count(); i++) {
			redo[i] = data->mesh->e[i].ExportFlags ();
			if (i<max) data->mesh->e[i].ImportFlags (undo[i]);
		}
		data->mesh->getEdgeSel (data->edgeSel);
		break;

	case MNM_SL_FACE:
		redo.SetCount (data->mesh->numf);
		for (i=0; i<redo.Count(); i++) {
			redo[i] = data->mesh->f[i].ExportFlags ();
			if (i<max) data->mesh->f[i].ImportFlags (undo[i]);
		}
		data->mesh->getFaceSel (data->faceSel);
		break;
	}

	mod->LocalDataChanged();
}

void PSComponentFlagRestore::Redo () {
	int i, max = undo.Count ();

	switch (sl) {
	case MNM_SL_OBJECT:
		return;

	case MNM_SL_VERTEX:
		if (max>data->mesh->numv) max = data->mesh->numv;
		for (i=0; i<max; i++) data->mesh->v[i].ImportFlags (redo[i]);
		data->mesh->getVertexSel (data->vertSel);
		break;

	case MNM_SL_EDGE:
		if (max>data->mesh->nume) max = data->mesh->nume;
		for (i=0; i<max; i++) data->mesh->e[i].ImportFlags (redo[i]);
		data->mesh->getEdgeSel (data->edgeSel);
		break;

	case MNM_SL_FACE:
		if (max>data->mesh->numf) max = data->mesh->numf;
		for (i=0; i<max; i++) data->mesh->f[i].ImportFlags (redo[i]);
		data->mesh->getFaceSel (data->faceSel);
		break;
	}

	mod->LocalDataChanged();
}

//--- Named selection sets -----------------------------------------

int PolyMeshSelMod::FindSet(TSTR &setName, int nsl) {
	for (int i=0; i<namedSel[nsl].Count(); i++) {
		if (setName == *namedSel[nsl][i]) return i;
	}
	return -1;
}

DWORD PolyMeshSelMod::AddSet(TSTR &setName,int nsl) {
	DWORD id = 0;
	TSTR *name = new TSTR(setName);
	int nsCount = namedSel[nsl].Count();

	// Find an empty id to assign to this set.
	BOOL found = FALSE;
	while (!found) {
		found = TRUE;
		for (int i=0; i<ids[nsl].Count(); i++) {
			if (ids[nsl][i]!=id) continue;
			id++;
			found = FALSE;
			break;
		}
	}

	// Find location in alphabetized list:
   int pos;
	for (pos=0; pos<nsCount; pos++) if (setName < *(namedSel[nsl][pos])) break;
	if (pos == nsCount) {
		namedSel[nsl].Append(1,&name);
		ids[nsl].Append(1,&id);
	} else {
		namedSel[nsl].Insert (pos, 1, &name);
		ids[nsl].Insert (pos, 1, &id);
	}

	return id;
}

void PolyMeshSelMod::RemoveSet(TSTR &setName,int nsl) {
	int i = FindSet(setName,nsl);
	if (i<0) return;
	delete namedSel[nsl][i];
	namedSel[nsl].Delete(i,1);
	ids[nsl].Delete(i,1);
}

void PolyMeshSelMod::UpdateSetNames () {
	if (!ip) return;

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	for (int i=0; i<mcList.Count(); i++) {
		PolyMeshSelData *meshData = (PolyMeshSelData*)mcList[i]->localData;
		if ( !meshData ) continue;
		for (int nsl=0; nsl<3; nsl++) {
			// Make sure the namedSel array is in alpha order.
			// (Crude bubble sort since we expect that it will be.)
			int j, k, kmax = namedSel[nsl].Count();
			for (k=1; k<kmax; k++) {
				if (*(namedSel[nsl][k-1]) < *(namedSel[nsl][k])) continue;
				for (j=0; j<k-1; j++) {
					if (*(namedSel[nsl][j]) > *(namedSel[nsl][k])) break;
				}
				// j now represents the point at which k should be inserted.
				TSTR *hold = namedSel[nsl][k];
				DWORD dhold = ids[nsl][k];
				int j2;
				for (j2=k; j2>j; j2--) {
					namedSel[nsl][j2] = namedSel[nsl][j2-1];
					ids[nsl][j2] = ids[nsl][j2-1];
				}
				namedSel[nsl][j] = hold;
				ids[nsl][j] = dhold;
			}

			GenericNamedSelSetList & gnsl = meshData->GetNamedSel(nsl);
			// Check for old, unnamed or misnamed sets with ids.
			for (k=0; k<gnsl.Count(); k++) {
				for (j=0; j<ids[nsl].Count(); j++) if (ids[nsl][j] == gnsl.ids[k]) break;
				if (j == ids[nsl].Count()) continue;
				if (gnsl.names[k] && !(*(gnsl.names[k]) == *(namedSel[nsl][j]))) {
					delete gnsl.names[k];
					gnsl.names[k] = NULL;
				}
				if (gnsl.names[k]) continue;
				gnsl.names[k] = new TSTR(*(namedSel[nsl][j]));
			}
			gnsl.Alphabetize ();

			// Now check lists against each other, adding any missing elements.
			for (j=0; j<gnsl.Count(); j++) {
				if (*(gnsl.names[j]) == *(namedSel[nsl][j])) continue;
				if (j>= namedSel[nsl].Count()) {
					TSTR *nname = new TSTR(*gnsl.names[j]);
					DWORD nid = gnsl.ids[j];
					namedSel[nsl].Append (1, &nname);
					ids[nsl].Append (1, &nid);
					continue;
				}
				if (*(gnsl.names[j]) > *(namedSel[nsl][j])) {
					BitArray baTemp;
					gnsl.InsertSet (j, baTemp, ids[nsl][j], *(namedSel[nsl][j]));
					continue;
				}
				// Otherwise:
				TSTR *nname = new TSTR(*gnsl.names[j]);
				DWORD nid = gnsl.ids[j];
				namedSel[nsl].Insert (j, 1, &nname);
				ids[nsl].Insert (j, 1, &nid);
			}
			for (; j<namedSel[nsl].Count(); j++) {
				BitArray baTemp;
				gnsl.AppendSet (baTemp, ids[nsl][j], *(namedSel[nsl][j]));
			}
		}
	}

	nodes.DisposeTemporary();
}

void PolyMeshSelMod::ClearSetNames() {
	for (int i=0; i<3; i++) {
		for (int j=0; j<namedSel[i].Count(); j++) {
			delete namedSel[i][j];
			namedSel[i][j] = NULL;
		}
	}
}

void PolyMeshSelMod::ActivateSubSelSet(TSTR &setName) {
	ModContextList mcList;
	INodeTab nodes;
	int nsl = namedSetLevel[selLevel];
	int index = FindSet (setName, nsl);	
	if (index<0 || !ip) return;

	theHold.Begin ();
	ip->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		PolyMeshSelData *meshData = (PolyMeshSelData*)mcList[i]->localData;
		if (!meshData) continue;
		if (theHold.Holding() && !meshData->held) theHold.Put(new SelectRestore(this,meshData));

		BitArray *set = NULL;

		switch (nsl) {
		case NS_VERTEX:
			set = meshData->vselSet.GetSet(ids[nsl][index]);
			if (set) {
				if (set->GetSize()!=meshData->vertSel.GetSize()) {
					set->SetSize(meshData->vertSel.GetSize(),TRUE);
				}
				meshData->SetVertSel (*set, this, ip->GetTime());
			}
			break;

		case NS_FACE:
			set = meshData->fselSet.GetSet(ids[nsl][index]);
			if (set) {
				if (set->GetSize()!=meshData->faceSel.GetSize()) {
					set->SetSize(meshData->faceSel.GetSize(),TRUE);
				}
				meshData->SetFaceSel (*set, this, ip->GetTime());
			}
			break;

		case NS_EDGE:
			set = meshData->eselSet.GetSet(ids[nsl][index]);
			if (set) {
				if (set->GetSize()!=meshData->edgeSel.GetSize()) {
					set->SetSize(meshData->edgeSel.GetSize(),TRUE);
				}
				meshData->SetEdgeSel (*set, this, ip->GetTime());
			}
			break;
		}
	}
	
	nodes.DisposeTemporary();
	LocalDataChanged ();
	theHold.Accept (GetString (IDS_SELECT));
	ip->RedrawViews(ip->GetTime());
}

void PolyMeshSelMod::NewSetFromCurSel(TSTR &setName) {
	if (!ip) return;
	ModContextList mcList;
	INodeTab nodes;
	DWORD id = -1;
	int nsl = namedSetLevel[selLevel];
	int index = FindSet(setName, nsl);
	if (index<0) {
		id = AddSet(setName, nsl);
		if (theHold.Holding()) theHold.Put (new AddSetNameRestore (this, id, &namedSel[nsl], &ids[nsl]));
	} else id = ids[nsl][index];

	ip->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		PolyMeshSelData *meshData = (PolyMeshSelData*)mcList[i]->localData;
		if (!meshData) continue;
		
		BitArray *set = NULL;

		switch (nsl) {
		case NS_VERTEX:	
			if (index>=0 && (set = meshData->vselSet.GetSet(id)) != NULL) {
				*set = meshData->vertSel;
			} else {
				meshData->vselSet.InsertSet (meshData->vertSel,id, setName);
				if (theHold.Holding()) theHold.Put (new AddSetRestore (&(meshData->vselSet), id, setName));
			}
			break;

		case NS_FACE:
			if (index>=0 && (set = meshData->fselSet.GetSet(id)) != NULL) {
				*set = meshData->faceSel;
			} else {
				meshData->fselSet.InsertSet(meshData->faceSel,id, setName);
				if (theHold.Holding()) theHold.Put (new AddSetRestore (&(meshData->fselSet), id, setName));
			}
			break;

		case NS_EDGE:
			if (index>=0 && (set = meshData->eselSet.GetSet(id)) != NULL) {
				*set = meshData->edgeSel;
			} else {
				meshData->eselSet.InsertSet(meshData->edgeSel, id, setName);
				if (theHold.Holding()) theHold.Put (new AddSetRestore (&(meshData->eselSet), id, setName));
			}
			break;
		}
	}	
	nodes.DisposeTemporary();
}

void PolyMeshSelMod::RemoveSubSelSet(TSTR &setName) {
	int nsl = namedSetLevel[selLevel];
	int index = FindSet (setName, nsl);
	if (index<0 || !ip) return;		

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	DWORD id = ids[nsl][index];

	for (int i = 0; i < mcList.Count(); i++) {
		PolyMeshSelData *meshData = (PolyMeshSelData*)mcList[i]->localData;
		if (!meshData) continue;		

		switch (nsl) {
		case NS_VERTEX:	
			if (theHold.Holding()) theHold.Put(new DeleteSetRestore(&meshData->vselSet,id, setName));
			meshData->vselSet.RemoveSet(id);
			break;

		case NS_FACE:
			if (theHold.Holding()) theHold.Put(new DeleteSetRestore(&meshData->fselSet,id, setName));
			meshData->fselSet.RemoveSet(id);
			break;

		case NS_EDGE:
			if (theHold.Holding()) theHold.Put(new DeleteSetRestore(&meshData->eselSet,id, setName));
			meshData->eselSet.RemoveSet(id);
			break;
		}
	}
	
	if (theHold.Holding()) theHold.Put(new DeleteSetNameRestore(&(namedSel[nsl]),this,&(ids[nsl]),id));
	RemoveSet (setName, nsl);
	ip->ClearCurNamedSelSet();
	nodes.DisposeTemporary();
}

void PolyMeshSelMod::SetupNamedSelDropDown() {
	if (selLevel == SEL_OBJECT) return;
	if (!ip) return;
	ip->ClearSubObjectNamedSelSets();
	int nsl = namedSetLevel[selLevel];
	for (int i=0; i<namedSel[nsl].Count(); i++)
		ip->AppendSubObjectNamedSelSet(*namedSel[nsl][i]);
	UpdateNamedSelDropDown ();
}

int PolyMeshSelMod::NumNamedSelSets() {
	int nsl = namedSetLevel[selLevel];
	return namedSel[nsl].Count();
}

TSTR PolyMeshSelMod::GetNamedSelSetName(int i) {
	int nsl = namedSetLevel[selLevel];
	return *namedSel[nsl][i];
}

void PolyMeshSelMod::SetNamedSelSetName(int i,TSTR &newName) {
	int nsl = namedSetLevel[selLevel];
	if (theHold.Holding()) theHold.Put(new SetNameRestore(&namedSel[nsl],this,&ids[nsl],ids[nsl][i]));
	*namedSel[nsl][i] = newName;
}

void PolyMeshSelMod::NewSetByOperator(TSTR &newName,Tab<int> &sets,int op) {
	if (!ip) return;
	int nsl = namedSetLevel[selLevel];
	DWORD id = AddSet(newName, nsl);
	if (theHold.Holding())
		theHold.Put(new AddSetNameRestore(this, id, &namedSel[nsl], &ids[nsl]));

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	for (int i = 0; i < mcList.Count(); i++) {
		PolyMeshSelData *meshData = (PolyMeshSelData*)mcList[i]->localData;
		if (!meshData) continue;
	
		BitArray bits;
		GenericNamedSelSetList *setList = NULL;

		switch (nsl) {
		case NS_VERTEX: setList = &meshData->vselSet; break;
		case NS_FACE: setList = &meshData->fselSet; break;			
		case NS_EDGE:   setList = &meshData->eselSet; break;			
		}		

		bits = (*setList)[sets[0]];

		for (int i=1; i<sets.Count(); i++) {
			switch (op) {
			case NEWSET_MERGE:
				bits |= (*setList)[sets[i]];
				break;

			case NEWSET_INTERSECTION:
				bits &= (*setList)[sets[i]];
				break;

			case NEWSET_SUBTRACT:
				bits &= ~((*setList)[sets[i]]);
				break;
			}
		}
		setList->InsertSet (bits, id, newName);
		if (theHold.Holding()) theHold.Put(new AddSetRestore(setList, id, newName));
	}
}

void PolyMeshSelMod::NSCopy() {
	int index = SelectNamedSet();
	if (index<0) return;
	if (!ip) return;

	int nsl = namedSetLevel[selLevel];
	MeshNamedSelClip *clip = new MeshNamedSelClip(*namedSel[nsl][index]);

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		PolyMeshSelData *meshData = (PolyMeshSelData*)mcList[i]->localData;
		if (!meshData) continue;

		GenericNamedSelSetList *setList = NULL;
		switch (nsl) {
		case NS_VERTEX: setList = &meshData->vselSet; break;				
		case NS_FACE: setList = &meshData->fselSet; break;			
		case NS_EDGE: setList = &meshData->eselSet; break;			
		}		

		BitArray *bits = new BitArray(*setList->sets[index]);
		clip->sets.Append(1,&bits);
	}
	SetMeshNamedSelClip(clip, namedClipLevel[selLevel]);
	
	// Enable the paste button
	if (!pSelDesc.NumParamMaps ()) return;
	IParamMap2 *pmap = pSelDesc.GetParamMap (ps_map_main);
	if (!pmap) return;
	HWND hWnd = pmap->GetHWnd();
	if (!hWnd) return;
	ICustButton* but = NULL;
	but = GetICustButton(GetDlgItem(hWnd,IDC_MS_PASTENS));
	but->Enable();
	ReleaseICustButton(but);
}

void PolyMeshSelMod::NSPaste() {
	if (!ip) return;
	int nsl = namedSetLevel[selLevel];
	MeshNamedSelClip *clip = GetMeshNamedSelClip(namedClipLevel[selLevel]);
	if (!clip) return;	
	TSTR name = clip->name;
	if (!GetUniqueSetName(name)) return;

	ModContextList mcList;
	INodeTab nodes;
	theHold.Begin();

	DWORD id = AddSet (name, nsl);	
	if (theHold.Holding())
		theHold.Put(new AddSetNameRestore(this, id, &namedSel[nsl], &ids[nsl]));	

	ip->GetModContexts(mcList,nodes);
	for (int i = 0; i < mcList.Count(); i++) {
		PolyMeshSelData *meshData = (PolyMeshSelData*)mcList[i]->localData;
		if (!meshData) continue;

		GenericNamedSelSetList *setList = NULL;
		switch (nsl) {
		case NS_VERTEX: setList = &meshData->vselSet; break;
		case NS_EDGE: setList = &meshData->eselSet; break;
		case NS_FACE: setList = &meshData->fselSet; break;
		}
				
		if (i>=clip->sets.Count()) {
			BitArray bits;
			setList->InsertSet(bits,id,name);
		} else setList->InsertSet(*clip->sets[i],id,name);
		if (theHold.Holding()) theHold.Put(new AddSetRestore(setList, id, name));
	}	
	
	ActivateSubSelSet(name);
	ip->SetCurNamedSelSet(name);
	theHold.Accept(GetString (IDS_PASTE_NS));
	SetupNamedSelDropDown();
}

int PolyMeshSelMod::SelectNamedSet() {
	Tab<TSTR*> &setList = namedSel[namedSetLevel[selLevel]];
	if (!ip) return false;
	return DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_NAMEDSET_SEL),
		ip->GetMAXHWnd(), PickSetDlgProc, (LPARAM)&setList);
}

void PolyMeshSelMod::SetNumSelLabel() {	
	TSTR buf;
	int num = 0, which = 0;

	if (!ip) return;
	if (!pSelDesc.NumParamMaps ()) return;
	IParamMap2 *pmap = pSelDesc.GetParamMap (ps_map_main);
	if (!pmap) return;
	HWND hParams = pmap->GetHWnd();
	if (!hParams) return;

	ModContextList mcList;
	INodeTab nodes;

	ip->GetModContexts(mcList,nodes);
	for (int i = 0; i < mcList.Count(); i++) {
		PolyMeshSelData *meshData = (PolyMeshSelData*)mcList[i]->localData;
		if (!meshData) continue;

		switch (selLevel) {
		case SEL_VERTEX:
			num += meshData->vertSel.NumberSet();
			if (meshData->vertSel.NumberSet() == 1) {
				for (which=0; which<meshData->vertSel.GetSize(); which++) if (meshData->vertSel[which]) break;
			}
			break;
		case SEL_FACE:
		case SEL_ELEMENT:
			num += meshData->faceSel.NumberSet();
			if (meshData->faceSel.NumberSet() == 1) {
				for (which=0; which<meshData->faceSel.GetSize(); which++) if (meshData->faceSel[which]) break;
			}
			break;
		case SEL_EDGE:
		case SEL_BORDER:
			num += meshData->edgeSel.NumberSet();
			if (meshData->edgeSel.NumberSet() == 1) {
				for (which=0; which<meshData->edgeSel.GetSize(); which++) if (meshData->edgeSel[which]) break;
			}
			break;
		}
	}

	switch (selLevel) {
	case SEL_VERTEX:
		if (num==1) buf.printf (GetString(IDS_PM_WHICHVERTSEL), which+1);
		else buf.printf(GetString(IDS_PM_NUMVERTSEL),num);
		break;

	case SEL_FACE:
	case SEL_ELEMENT:
		if (num==1) buf.printf (GetString(IDS_PM_WHICHFACESEL), which+1);
		else buf.printf(GetString(IDS_PM_NUMFACESEL),num);
		break;

	case SEL_EDGE:
	case SEL_BORDER:
		if (num==1) buf.printf (GetString(IDS_PM_WHICHEDGESEL), which+1);
		else buf.printf(GetString(IDS_PM_NUMEDGESEL),num);
		break;

	case SEL_OBJECT:
		buf = GetString (IDS_PM_OBJECTSEL);
		break;
	}

	SetDlgItemText(hParams,IDC_MS_NUMBER_SEL,buf);
}

RefResult PolyMeshSelMod::NotifyRefChanged (Interval changeInt, RefTargetHandle hTarget, 
   		PartID& partID, RefMessage message) 
{

	if ((message == REFMSG_CHANGE) && (hTarget == pblock)) 
	{
		// if this was caused by a NotifyDependents from pblock, LastNotifyParamID()
		// will contain ID to update, else it will be -1 => inval whole rollout

		int pid = pblock->LastNotifyParamID();
		InvalidateDialogElement (pid);

		if ((pid == ps_edist) || (pid == ps_use_edist)) 
		{
			// Because the technique we use to measure vertex distance has changed, we
			// need to invalidate the currently cached distances.
			InvalidateVDistances ();
		}
 
	}
	return(REF_SUCCEED);
}

int PolyMeshSelMod::NumSubObjTypes() 
{ 
	return 5;
}

ISubObjType *PolyMeshSelMod::GetSubObjType(int i) 
{

	static bool initialized = false;
	if(!initialized)
	{
		initialized = true;
		SOT_Vertex.SetName(GetString(IDS_PM_VERTEX));
		SOT_Edge.SetName(GetString(IDS_PM_EDGE));
		SOT_Border.SetName(GetString(IDS_PM_BORDER));
		SOT_Face.SetName(GetString(IDS_PM_FACE));
		SOT_Element.SetName(GetString(IDS_PM_ELEMENT));
	}

	switch(i)
	{
	case -1:
		if(GetSubObjectLevel() > 0)
			return GetSubObjType(GetSubObjectLevel()-1);
		break;
	case 0:
		return &SOT_Vertex;
	case 1:
		return &SOT_Edge;
	case 2:
		return &SOT_Border;
	case 3:
		return &SOT_Face;
	case 4:
		return &SOT_Element;
	}
	return NULL;
}

void PolyMeshSelMod::SetSoftSelEnables () {
	if (!pblock) return;
	if (!pSelDesc.NumParamMaps ()) return;
	IParamMap2 *pmap = pSelDesc.GetParamMap (ps_map_softsel);
	if (!pmap) return;
	HWND hWnd = pmap->GetHWnd();
	if (!hWnd) return;
	theSoftSelProc.SetEnables(hWnd);
}


void PolyMeshSelMod::DoAccept(TimeValue t)
{

}
void PolyMeshSelMod::SetPinch(TimeValue t, float pinch)
{
	pblock->SetValue (ps_pinch, t, pinch);

}
void PolyMeshSelMod::SetBubble(TimeValue t, float bubble)
{
	pblock->SetValue (ps_bubble, t, bubble);
}
float PolyMeshSelMod::GetPinch(TimeValue t)
{
	return pblock->GetFloat (ps_pinch, t);
}

float PolyMeshSelMod::GetBubble(TimeValue t)
{
	return pblock->GetFloat (ps_bubble, t);
}

float PolyMeshSelMod::GetFalloff(TimeValue t)
{
	return pblock->GetFloat (ps_falloff, t);
}

void PolyMeshSelMod::SetFalloff(TimeValue t,float falloff)
{
	pblock->SetValue (ps_falloff, t, falloff);
}

void PolyMeshSelMod::EditSoftSelectionMode()
{
	if (!ip) return;
	CommandMode *currentMode = ip->GetCommandMode ();
	if(currentMode!=editSSMode)
		ip->PushCommandMode(editSSMode);
	else
		ip->DeleteMode(editSSMode);
}





void PolyMeshSelMod::InvalidateDialogElement (int elem) {
	if (!pblock) return;
	if (!pSelDesc.NumParamMaps ()) return;
	IParamMap2 *pmap = pSelDesc.GetParamMap (ps_map_main);
	if (pmap) pmap->Invalidate (elem);
	pmap = pSelDesc.GetParamMap (ps_map_softsel);
	if (pmap) pmap->Invalidate (elem);

	if (editMod != this) return;
	HWND hWnd = pmap->GetHWnd();
	if (!hWnd) return;

	switch (elem) {
	case ps_use_softsel:
	case ps_use_edist:
	case ps_lock_softsel:
		theSoftSelProc.SetEnables (hWnd);
	}
}

void PolyMeshSelMod::GetSoftSelData( GenSoftSelData& softSelData, TimeValue t ) {
	softSelData.useSoftSel =	pblock->GetInt (ps_use_softsel, t);
	softSelData.useEdgeDist =	pblock->GetInt (ps_use_edist, t);
	softSelData.edgeIts =		pblock->GetInt (ps_edist, t);
	softSelData.ignoreBack =	(pblock->GetInt (ps_affect_backfacing, t)? FALSE:TRUE);
	softSelData.falloff =		pblock->GetFloat (ps_falloff, t);
	softSelData.pinch =			pblock->GetFloat (ps_pinch, t);
	softSelData.bubble =		pblock->GetFloat (ps_bubble, t);
}

void PolyMeshSelData::SetupNewSelection (int selLevel) {
	DbgAssert (mesh);
	if (!mesh) return;
	if (!mpNewSelection) mpNewSelection = new BitArray;
	switch (selLevel) {
	case SEL_VERTEX:
		mpNewSelection->SetSize(mesh->numv);
		break;
	case SEL_EDGE:
	case SEL_BORDER:
		mpNewSelection->SetSize(mesh->nume);
		break;
	case SEL_FACE:
	case SEL_ELEMENT:
		mpNewSelection->SetSize(mesh->numf);
		break;
	}
}

void PolyMeshSelData::TranslateNewSelection (int selLevelFrom, int selLevelTo) {
	if (!mpNewSelection) return;
	if (!mesh) {
		DbgAssert (0);
		return;
	}

	BitArray intermediateSelection;
	BitArray toSelection;
	switch (selLevelFrom) {
	case SEL_VERTEX:
		switch (selLevelTo) {
		case SEL_EDGE:
			mSelConv.VertexToEdge (*mesh, *mpNewSelection, toSelection);
			break;
		case SEL_BORDER:
			mSelConv.VertexToEdge (*mesh, *mpNewSelection, intermediateSelection);
			mSelConv.EdgeToBorder (*mesh, intermediateSelection, toSelection);
			break;
		case SEL_FACE:
			mSelConv.VertexToFace (*mesh, *mpNewSelection, toSelection);
			break;
		case SEL_ELEMENT:
			mSelConv.VertexToFace (*mesh, *mpNewSelection, intermediateSelection);
			mSelConv.FaceToElement (*mesh, intermediateSelection, toSelection);
			break;
		}
		break;

	case SEL_EDGE:
		if (selLevelTo == SEL_BORDER) {
			mSelConv.EdgeToBorder (*mesh, *mpNewSelection, toSelection);
		}
		break;

	case SEL_FACE:
		if (selLevelTo == SEL_ELEMENT) {
			mSelConv.FaceToElement (*mesh, *mpNewSelection, toSelection);
		}
		break;
	}

	if (toSelection.GetSize() == 0) return;
	*mpNewSelection = toSelection;
}

void PolyMeshSelData::ApplyNewSelection (int selLevel, bool keepOld, bool invert, bool select) {
	if (!mpNewSelection) return;
	if (!mesh) {
		DbgAssert (0);
		return;
	}

	int properSize=0;
	BitArray properSelection;
	switch (selLevel) {
	case SEL_VERTEX:
		properSize = mesh->numv;
		break;
	case SEL_EDGE:
	case SEL_BORDER:
		properSize = mesh->nume;
		break;
	case SEL_FACE:
	case SEL_ELEMENT:
		properSize = mesh->numf;
		break;
	}
	if (mpNewSelection->GetSize () != properSize) {
		mpNewSelection->SetSize (properSize);
		DbgAssert (0);
	}

	if (keepOld) {
		switch (selLevel) {
		case SEL_VERTEX:
			properSelection = vertSel;
			break;
		case SEL_EDGE:
		case SEL_BORDER:
			properSelection = edgeSel;
			break;
		case SEL_FACE:
		case SEL_ELEMENT:
			properSelection = faceSel;
			break;
		}
		if (!properSize) return;

		if (properSelection.GetSize () != properSize) {
			properSelection.SetSize (properSize);
			DbgAssert (0);
		}

		if (invert) {
			// Bits in result should be set if set in exactly one of current, incoming selections
			properSelection ^= (*mpNewSelection);
		} else {
			if (select) {
				// Result set if set in either of current, incoming:
				properSelection |= (*mpNewSelection);
			} else {
				// Result set if in current, and _not_ in incoming:
				properSelection &= ~(*mpNewSelection);
			}
		}
		switch (selLevel) {
		case SEL_VERTEX:
			vertSel = properSelection;
			break;
		case SEL_EDGE:
		case SEL_BORDER:
			edgeSel = properSelection;
			break;
		case SEL_FACE:
		case SEL_ELEMENT:
			faceSel = properSelection;
			break;
		}
	} else {
		switch (selLevel) {
		case SEL_VERTEX:
			vertSel = *mpNewSelection;
			break;
		case SEL_EDGE:
		case SEL_BORDER:
			edgeSel = *mpNewSelection;
			break;
		case SEL_FACE:
		case SEL_ELEMENT:
			faceSel = *mpNewSelection;
			break;
		}
	}


	delete mpNewSelection;
	mpNewSelection = NULL;
}

#endif // NO_MODIFIER_POLY_SELECT


static PolySelImageHandler thePolySelImageHandler;
PolySelImageHandler *GetPolySelImageHandler () {
	return &thePolySelImageHandler;
}

HIMAGELIST PolySelImageHandler::LoadImages() {
	if (images ) return images;

	HBITMAP hBitmap, hMask;
	images = ImageList_Create(24, 23, ILC_COLOR|ILC_MASK, 10, 0);
	hBitmap = LoadBitmap(hInstance,MAKEINTRESOURCE (IDB_SELTYPES));
	hMask = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_SELMASK));
	ImageList_Add (images, hBitmap, hMask);
	DeleteObject (hBitmap);
	DeleteObject (hMask);
	return images;
}

HIMAGELIST PolySelImageHandler::LoadPlusMinus () {
	if (hPlusMinus) return hPlusMinus;

	HBITMAP hBitmap, hMask;
	hPlusMinus = ImageList_Create(12, 12, ILC_MASK, 6, 0);
	hBitmap     = LoadBitmap (hInstance,MAKEINTRESOURCE(IDB_PLUSMINUS));
	hMask       = LoadBitmap (hInstance,MAKEINTRESOURCE(IDB_PLUSMINUSMASK));
	ImageList_Add (hPlusMinus, hBitmap, hMask);
	DeleteObject (hBitmap);
	DeleteObject (hMask);
	return hPlusMinus;
}