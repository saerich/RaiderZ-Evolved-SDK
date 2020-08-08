//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Defines a ring array system object
// AUTHOR: Unknown - created Nov.11.1994
//***************************************************************************/

#include "prim.h"
#include <props.h>
#include <dummy.h>
#include <Simpobj.h>
#include <notify.h>
#include <IIndirectRefTargContainer.h>
#include <3dsmaxport.h>

#ifndef NO_OBJECT_RINGARRAY	// russom - 10/12/01

// External declarations
extern HINSTANCE hInstance;

// Constants
#define SLAVE_CONTROL_CLASS_ID 0x9100
#define IID_RING_ARRAY_MASTER Interface_ID(0x638c6c52, 0xc140d40)

//------------------------------------------------------------------------------
class RingMaster: public ReferenceTarget, public FPMixinInterface 
{
	public:
		RingMaster(int numNodes);
		~RingMaster();

		// Methods for retrieving\setting the i-th node of the system. 
		// i is a zero based index.
		INode* GetSlaveNode(int i);
		void SetSlaveNode(int i, INode * node);
		// Methods for accessing the ring array's parameters 
		void SetNum(TimeValue t, int n, BOOL notify=TRUE); 
		void SetRad(TimeValue t, float r); 
		void SetCyc(TimeValue t, float r); 
		void SetAmp(TimeValue t, float r); 
		void SetPhs(TimeValue t, float r); 
		int GetNum(TimeValue t, Interval& valid = Interval(0,0) ); 	
		float GetRad(TimeValue t, Interval& valid = Interval(0,0) ); 	
		float GetCyc(TimeValue t, Interval& valid = Interval(0,0) ); 	
		float GetAmp(TimeValue t, Interval& valid = Interval(0,0) ); 	
		float GetPhs(TimeValue t, Interval& valid = Interval(0,0) ); 	

		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method, int id);
		void UpdateUI(TimeValue t);
		void UpdateAnimKeyBrackets(TimeValue t, int pbIndex);

		// From Animatable
		int NumSubs()  { return 1; }
		Animatable* SubAnim(int i) { return mPBlock; }
		TSTR SubAnimName(int i) { return GetString(IDS_DS_RINGARRAYPAR);}		
		Class_ID ClassID() { return Class_ID(RINGARRAY_CLASS_ID,0); }  
		SClass_ID SuperClassID() { return SYSTEM_CLASS_ID; }  
		void GetClassName(TSTR& s) { s = GetString(IDS_DB_RING_ARRAY_CLASS); }
		void DeleteThis() { delete this; }		
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev );
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next );
		void GetSystemNodes(INodeTab &nodes, SysNodeContext ctxt);

		// From Reference Target
		RefTargetHandle Clone(RemapDir& remap);
		int NumRefs() { return kNUM_REFS;	};
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		// From ReferenceMaker
		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage);
		
		// --- BaseInterface methods
		virtual BaseInterface*	GetInterface(Interface_ID);
		virtual void						DeleteInterface() { delete this; }

		// --- From FPInterface
		virtual FPInterfaceDesc* GetDesc();
		virtual FPInterfaceDesc* GetDescByID(Interface_ID id);
		
	public:
		// --- Function publishing 
		enum FPEnums 	{ 
			kfpSystemNodeContexts,
		};

		enum FPIDs { 
		 kRM_GetNodes,
		 kRM_GetNode,
		 kRM_GetNodeNum, kRM_SetNodeNum,
		 kRM_GetCycle, kRM_SetCycle,
		 kRM_GetAmplitute, kRM_SetAmplitude,
		 kRM_GetRadius, kRM_SetRadius,
		 kRM_GetPhase, kRM_SetPhase,
		}; 
		
		BEGIN_FUNCTION_MAP
			FN_2(kRM_GetNodes, TYPE_INT, fpGetNodes, TYPE_INODE_TAB_BR, TYPE_ENUM);
			// use TYPE_INDEX because Maxscript has 1 based arrays
			FN_1(kRM_GetNode, TYPE_INODE, GetSlaveNode, TYPE_INDEX);
			PROP_FNS(kRM_GetNodeNum, fpGetNodeNum, kRM_SetNodeNum, fpSetNodeNum, TYPE_INT);
			PROP_FNS(kRM_GetCycle, fpGetCycle, kRM_SetCycle, fpSetCycle, TYPE_FLOAT);
			PROP_FNS(kRM_GetPhase, fpGetPhase, kRM_SetPhase, fpSetPhase, TYPE_FLOAT);
			PROP_FNS(kRM_GetAmplitute, fpGetAmplitude, kRM_SetAmplitude, fpSetAmplitude, TYPE_FLOAT);
			PROP_FNS(kRM_GetRadius, fpGetRadius, kRM_SetRadius, fpSetRadius, TYPE_FLOAT);
		END_FUNCTION_MAP

		int fpGetNodes(Tab<INode*>& systemNodes, unsigned int context) {
			GetSystemNodes(static_cast<INodeTab&>(systemNodes), static_cast<SysNodeContext>(context));
			return systemNodes.Count();
		}

		int fpGetNodeNum() {
			return GetNum(GetCOREInterface()->GetTime());
		}
		void fpSetNodeNum(int nodeNum) {
			SetNum(GetCOREInterface()->GetTime(), nodeNum);
		}
		float fpGetCycle() {
			return GetCyc(GetCOREInterface()->GetTime());
		}
		void fpSetCycle(float cycle) {
			SetCyc(GetCOREInterface()->GetTime(), cycle);
		}
		float fpGetPhase() {
			return GetPhs(GetCOREInterface()->GetTime());
		}
		void fpSetPhase(float phase) {
			SetPhs(GetCOREInterface()->GetTime(), phase);
		}
		float fpGetAmplitude() {
			return GetAmp(GetCOREInterface()->GetTime());
		}
		void fpSetAmplitude(float amplitude) {
			SetAmp(GetCOREInterface()->GetTime(), amplitude);
		}
		float fpGetRadius() {
			return GetRad(GetCOREInterface()->GetTime());
		}
		void fpSetRadius(float radius) {
			SetRad(GetCOREInterface()->GetTime(), radius);
		}


	public:
		// Parameter block indices
		enum EParamBlkIdx 
		{
			PB_RAD = 0,
			PB_CYC = 1,
			PB_AMP = 2,
			PB_PHS = 3,
		};
		// Class vars
		HWND hMasterParams;
		static IObjParam *iObjParams;
		static int dlgNum;
		static float dlgRadius;
		static float dlgAmplitude;
		static float dlgCycles;
		static float dlgPhase;
		ISpinnerControl *numSpin;
		ISpinnerControl *radSpin;
		ISpinnerControl *ampSpin;
		ISpinnerControl *cycSpin;
		ISpinnerControl *phsSpin;

		// Default parameter values
		static const int kDEF_NODE_NUM, kMIN_NODE_NUM, kMAX_NODE_NUM;
		static const float kDEF_RADIUS, kMIN_RADIUS, kMAX_RADIUS;
		static const float kDEF_AMPLITUDE, kMIN_AMPLITUDE, kMAX_AMPLITUDE;
		static const float kDEF_CYCLES, kMIN_CYCLES, kMAX_CYCLES;
		static const float kDEF_PHASE, kMIN_PHASE, kMAX_PHASE;

	private:
		// List of node ptr loaded from legacy files
		Tab<INode*> mLegacyNodeTab;
		// Loads files that are considered legacy
		IOResult DoLoadLegacy(ILoad *iload);
		// Loads non-legacy files
		IOResult DoLoad(ILoad *iload);

		enum eReferences 
		{
			kREF_PARAM_BLK = 0,
			kREF_NODE_CONTAINER = 1,
			kNUM_REFS,
		};

		// References
		IParamBlock* mPBlock;
		IIndirectRefTargContainer* mNodes;

		// Function publishing
		static FPInterfaceDesc mFPInterfaceDesc;

		// File IO related const
		static const USHORT kNUMNODES_CHUNK = 0x100;
		static const USHORT kNODE_ID_CHUNK = 0x110;
		static const USHORT kVERSION_CHUNK = 0x120;
		// Version number, saved into max file
		static const unsigned short kVERSION_01	= 1;
		static const unsigned short kVERSION_CURRENT = kVERSION_01;
		unsigned short mVerLoaded;

	private:
		class RingMasterPLC;
		class NodeNumRestore;

	friend RingMasterPLC;
	friend NodeNumRestore;
};

//------------------------------------------------------------------------------
// PostLoadCallback
//------------------------------------------------------------------------------
class RingMaster::RingMasterPLC : public PostLoadCallback
{
	public:
		RingMasterPLC(RingMaster& ringMaster) : mRingMaster(ringMaster) { }
		void proc(ILoad *iload) 
		{
			for (int i = 0; i<mRingMaster.mLegacyNodeTab.Count(); ++i) {
				mRingMaster.SetSlaveNode(i, mRingMaster.mLegacyNodeTab[i]);
			}
			delete this;
		}

	private:
		RingMaster& mRingMaster;
};

//------------------------------------------------------------------------------
// Undo \ redo
//------------------------------------------------------------------------------

class RingMaster::NodeNumRestore: public RestoreObj 
{
	const RingMaster& mRingMaster;
	// Number of nodes before change (undo changes current node num to this value)
	int mPrevNodeNum;
	// Number of nodes after change (redo changes current node num to this value)
	int mNewNodeNum;

	public:
		NodeNumRestore(const RingMaster& ringMaster, int newNodeNum); 
		~NodeNumRestore() {}
		void Restore(int isUndo);
		void Redo();
		TSTR Description() { return TSTR("NodeNumRestore"); }
};

RingMaster::NodeNumRestore::NodeNumRestore(
	const RingMaster& ringMaster, 
	int newNodeNum)
: mRingMaster(ringMaster)
{ 
	mPrevNodeNum = mRingMaster.mNodes->GetNumItems(); 
	mNewNodeNum = newNodeNum;
}

void RingMaster::NodeNumRestore::Restore(int isUndo) 
{
	if (mRingMaster.hMasterParams && mRingMaster.numSpin) {
		mRingMaster.numSpin->SetValue(mPrevNodeNum, FALSE );
	}
	const_cast<RingMaster&>(mRingMaster).NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

void RingMaster::NodeNumRestore::Redo() 
{	
	if (mRingMaster.hMasterParams && mRingMaster.numSpin) {
		mRingMaster.numSpin->SetValue(mNewNodeNum, FALSE );
	}
	const_cast<RingMaster&>(mRingMaster).NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

//------------------------------------------------------------------------------
// Ring Master
//-------------------------------------------------------------------------------
Control* GetNewSlaveControl(RingMaster *master, int i);

//------------------------------------------------------------------------------
class RingMasterClassDesc:public ClassDesc 
{
	public:
	int IsPublic() { return 1; }
	void*	Create(BOOL loading = FALSE) { 
		return new RingMaster((loading ? 0 : RingMaster::dlgNum)); 
	}
	const TCHAR *	ClassName() { return GetString(IDS_DB_RING_ARRAY_CLASS); }
	int BeginCreate(Interface *i);
	int EndCreate(Interface *i);
	SClass_ID SuperClassID() { return SYSTEM_CLASS_ID; }
	Class_ID ClassID() { return Class_ID(RINGARRAY_CLASS_ID,0); }
	const TCHAR* Category() { return _T("");  }
};
static RingMasterClassDesc theRingMasterClassDesc;

ClassDesc* GetRingMasterDesc() { return &theRingMasterClassDesc; }

//------------------------------------------------------------------------------
const int RingMaster::kDEF_NODE_NUM = 4;
const int RingMaster::kMIN_NODE_NUM = 1;
const int RingMaster::kMAX_NODE_NUM = 200;

const float RingMaster::kDEF_RADIUS = 100.0f;
const float RingMaster::kMIN_RADIUS = 0.0f;
const float RingMaster::kMAX_RADIUS = 500.0f;

const float RingMaster::kDEF_AMPLITUDE = 20.0f;
const float RingMaster::kMIN_AMPLITUDE = 0.0f;
const float RingMaster::kMAX_AMPLITUDE = 500.0f;

const float RingMaster::kDEF_CYCLES = 3.0f;
const float RingMaster::kMIN_CYCLES = 0.0f;
const float RingMaster::kMAX_CYCLES = 10.0f;

const float RingMaster::kDEF_PHASE = 1.0f;
const float RingMaster::kMIN_PHASE = -1000.0f;
const float RingMaster::kMAX_PHASE = 1000.0f;


IObjParam *RingMaster::iObjParams = NULL;

int RingMaster::dlgNum = 	RingMaster::kDEF_NODE_NUM;
float RingMaster::dlgRadius = RingMaster::kDEF_RADIUS;
float RingMaster::dlgAmplitude = RingMaster::kDEF_AMPLITUDE;
float RingMaster::dlgCycles = RingMaster::kDEF_CYCLES;
float RingMaster::dlgPhase = RingMaster::kDEF_PHASE;

//------------------------------------------------------------------------------
RingMaster::RingMaster(int numNodes = 0) : mNodes(NULL), mPBlock(NULL), hMasterParams(NULL),
numSpin(NULL), radSpin(NULL), ampSpin(NULL), cycSpin(NULL), phsSpin(NULL)

{
	ParamBlockDesc desc[] = {
		{ TYPE_FLOAT, NULL, TRUE },
		{ TYPE_FLOAT, NULL, TRUE },
		{ TYPE_FLOAT, NULL, TRUE },
		{ TYPE_FLOAT, NULL, TRUE }
	};

	ReplaceReference(kREF_PARAM_BLK, CreateParameterBlock( desc, 4 ));
	IIndirectRefTargContainer* nodeMonCont = 
		reinterpret_cast<IIndirectRefTargContainer*>(GetCOREInterface()->CreateInstance(
		REF_TARGET_CLASS_ID, INDIRECT_REFTARG_CONTAINER_CLASS_ID));
	ReplaceReference(kREF_NODE_CONTAINER, nodeMonCont);

	SetRad( TimeValue(0), dlgRadius );
	SetCyc( TimeValue(0), dlgCycles );
	SetAmp( TimeValue(0), dlgAmplitude );
	SetPhs( TimeValue(0), dlgPhase );

	if (numNodes > 0) 
	{
		HoldSuspend holdSuspend;
		mNodes->SetItem(numNodes-1, NULL);
	}
}

RefTargetHandle RingMaster::Clone(RemapDir& remap) 
{
  RingMaster* newm = new RingMaster(0);	
	newm->ReplaceReference(kREF_PARAM_BLK, remap.CloneRef(mPBlock));
	newm->ReplaceReference(kREF_NODE_CONTAINER, remap.CloneRef(mNodes));

	BaseClone(this, newm, remap);
	return(newm);
}

RingMaster::~RingMaster() 
{
}

void RingMaster::UpdateUI(TimeValue t)
{
	if (hMasterParams) 
	{
		radSpin->SetValue( GetRad(t), FALSE );
		ampSpin->SetValue( GetAmp(t), FALSE );
		cycSpin->SetValue( GetCyc(t), FALSE );
		phsSpin->SetValue( GetPhs(t), FALSE );
		numSpin->SetValue( GetNum(t), FALSE );
		UpdateAnimKeyBrackets(t, PB_RAD);
		UpdateAnimKeyBrackets(t, PB_AMP);
		UpdateAnimKeyBrackets(t, PB_CYC);
		UpdateAnimKeyBrackets(t, PB_PHS);
	}
}

void RingMaster::UpdateAnimKeyBrackets(TimeValue t, int i)
{
	switch (i)
	{
		case PB_RAD:
			radSpin->SetKeyBrackets(mPBlock->KeyFrameAtTime(PB_RAD,t));
		break;
		
		case PB_AMP:
			ampSpin->SetKeyBrackets(mPBlock->KeyFrameAtTime(PB_AMP,t));
		break;
		
		case PB_CYC:
			cycSpin->SetKeyBrackets(mPBlock->KeyFrameAtTime(PB_CYC,t));
		break;
		
		case PB_PHS:
			phsSpin->SetKeyBrackets(mPBlock->KeyFrameAtTime(PB_PHS,t));
		break;
		
		default:
			DbgAssert(false);
			break;
	}
}

INode* RingMaster::GetSlaveNode(int i) 
{ 
	if (mNodes != NULL) {
		ReferenceTarget* refTarg = mNodes->GetItem(i);
		if (refTarg != NULL) {
			return static_cast<INode*>(refTarg->GetInterface(INODE_INTERFACE));
		}
	}
	return NULL;
}

void RingMaster::SetSlaveNode(int i, INode * node) 
{
	if (mNodes != NULL) {
		mNodes->SetItem(i, node);
	}
}


void RingMaster::GetSystemNodes(INodeTab &nodes, SysNodeContext ctxt)
{
	switch (ctxt) 
	{
		case kSNCClone:
		case kSNCFileMerge:
		case kSNCFileSave:
		case kSNCDelete:
		{
			if (mNodes != NULL)
			{
				int nodeCount = mNodes->GetNumItems();
				for (int i = 0; i < nodeCount; i++) 
				{
					ReferenceTarget* refTarg = mNodes->GetItem(i);
					INode* node = NULL;
					if (refTarg != NULL) {
						node = static_cast<INode*>(refTarg->GetInterface(INODE_INTERFACE));
					}
					nodes.Append(1, &node);
				}
			}
		}
		break;
		
		default:
		break;
	}
}

RefTargetHandle RingMaster::GetReference(int i)  
{ 
	if (kREF_PARAM_BLK == i) {
		return mPBlock;
	}
	else if (kREF_NODE_CONTAINER == i) {
		return mNodes;
	}
	return NULL;
}


void RingMaster::SetReference(int i, RefTargetHandle rtarg) 
{
	if (kREF_PARAM_BLK == i) {
		mPBlock = static_cast<IParamBlock*>(rtarg); 
	}
	else if (kREF_NODE_CONTAINER == i) {
		mNodes = static_cast<IIndirectRefTargContainer*>(rtarg);
	}
}		

#define TWO_PI 6.283185307f

// This is the crux of the controller: it takes an input (parent) matrix, modifies
// it accordingly.  
void RingMaster::GetValue(
	TimeValue t, 
	void *val, 
	Interval &valid, 
	GetSetMethod method,
	int id) 
{
	float radius = 0.0f, amplitude = 0.0f, cycles = 0.0f, phase = 0.0f;
	radius = GetRad(t, valid);
	amplitude = GetAmp(t, valid);
	cycles = GetCyc(t, valid);
	phase = GetPhs(t, valid);
	Matrix3 tmat, *mat = (Matrix3*)val;
	tmat.IdentityMatrix();
	float ang = float(id)*TWO_PI/(float)GetNum(t, valid);
	tmat.Translate(Point3(radius, 0.0f, amplitude*(float)cos(cycles*ang + TWO_PI*phase)));
	tmat.RotateZ(ang);
	
	(*mat) = (method == CTRL_RELATIVE) ? tmat*(*mat) : tmat;

	// Make sure spinners track when animating and in Motion Panel
	UpdateUI(t);
}

RefResult RingMaster::NotifyRefChanged(
	Interval changeInt, 
	RefTargetHandle hTarget, 
	PartID& partID, 
	RefMessage message ) 
{
	switch (message) 
	{
		case REFMSG_CHANGE:
			if(iObjParams)
				UpdateUI(iObjParams->GetTime());
			break;
		case REFMSG_GET_PARAM_DIM: 
		{ 
			// the ParamBlock needs info to display in the tree view
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) 
			{
				case PB_RAD:	
						gpd->dim = stdWorldDim; 
				break;	  
				case PB_CYC:	
					gpd->dim = stdWorldDim; 
				break;	  
				case PB_AMP:	
					gpd->dim = stdWorldDim; 
				break;	  
				case PB_PHS:	
					gpd->dim = stdWorldDim; 
				break;	  
				
				default:
					DbgAssert(false);
				break;									
			}
			return REF_HALT; 
		}
		break;

		case REFMSG_GET_PARAM_NAME: 
		{
			// the ParamBlock needs info to display in the tree view
			GetParamName *gpn = (GetParamName*)partID;
			switch (gpn->index) 
			{
				case PB_RAD: 
					gpn->name = GetString(IDS_DS_RADIUS);	
				break;
				case PB_CYC: 
					gpn->name = GetString(IDS_DS_CYCLES);	
				break;
				case PB_AMP: 
					gpn->name = GetString(IDS_DS_AMPLITUDE);	
				break;
				case PB_PHS: 
					gpn->name = GetString(IDS_DS_PHASE);	
				break;
				default:
					DbgAssert(false);
				break;
			}
			return REF_HALT; 
		}
		break;

	} // end switch

	return (REF_SUCCEED);
}

void RingMaster::SetNum(TimeValue t, int n, BOOL notify) 
{ 
	if (n < 1 || n == GetNum(t)) {
		return;
	}

	if (mNodes != NULL && mNodes->GetNumItems() > 0) 
	{
		if (theHold.Holding()) {
			theHold.Put(new NodeNumRestore(*this, n));
		}

		int nodeCount = mNodes->GetNumItems();
		if (n < nodeCount) 
		{
			// remove nodes;
			for (int i = nodeCount-1; i >= n; i--) 
			{
				ReferenceTarget* refTarg = mNodes->GetItem(i);
				// If the user is in create mode, and has not created any system nodes yet
				// the reftarget will be NULL. In this case, we still need to shrink the node array
				if (refTarg != NULL) 
				{
					INode* node = static_cast<INode*>(refTarg->GetInterface(INODE_INTERFACE));
					if (node) 
					{
						if (node->Selected()) 
						{
							// Dont want to delete selected nodes, so just stop and update the
							// spinner in the UI. We won't be called recursively, since the number 
							// of nodes has been updated already
							n = i+1;
							if (hMasterParams) 
							{
								DbgAssert(numSpin != NULL);
								numSpin->SetValue(n, TRUE);
							}
							break;
						}
						node->Delete(t, TRUE);
					}
				}
				mNodes->RemoveItem(i);
			}
		}
		else 
		{
			// add (indirect refs to) nodes
			mNodes->SetItem(n-1, NULL);

			// Create the new nodes
			if (mNodes->GetItem(0) != NULL)
			{
				ReferenceTarget* refTarg = mNodes->GetItem(0);
				INode* firstNode = static_cast<INode*>(refTarg->GetInterface(INODE_INTERFACE));
				DbgAssert(firstNode != NULL);
				Object* obj = firstNode->GetObjectRef();
				DbgAssert(obj);
				INode* parentNode = firstNode->GetParentNode();
				DbgAssert(parentNode != NULL);
				Interface* ip = GetCOREInterface();
				for (int i = nodeCount; i < n; i++) 
				{
					INode* newNode = ip->CreateObjectNode(obj);
					Control* slave = GetNewSlaveControl(this, i);
					newNode->SetTMController(slave);
					newNode->FlagForeground(t, FALSE);
					parentNode->AttachChild(newNode);
					SetSlaveNode(i, newNode);
				}
			}
		}
	}
	DbgAssert(mNodes->GetNumItems() == n);

	if (notify) {
		NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}
}

int RingMaster::GetNum(TimeValue t, Interval& valid ) 
{ 	
	DbgAssert(mNodes != NULL);
	return mNodes->GetNumItems();
}


//------------------------------------------------------------------------------
void RingMaster::SetRad(TimeValue t, float r) 
{ 
	mPBlock->SetValue( PB_RAD, t, r );
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

float RingMaster::GetRad(TimeValue t, Interval& valid ) 
{ 	
	float f = 0.0f;
	mPBlock->GetValue( PB_RAD, t, f, valid );
	return f;
}

//--------------------------------------------------
void RingMaster::SetCyc(TimeValue t, float r) 
{ 
	mPBlock->SetValue( PB_CYC, t, r );
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

float RingMaster::GetCyc(TimeValue t, Interval& valid ) 
{ 	
	float f = 0.0f;
	mPBlock->GetValue( PB_CYC, t, f, valid );
	return f;
}

//--------------------------------------------------
void RingMaster::SetAmp(TimeValue t, float r) 
{ 
	mPBlock->SetValue( PB_AMP, t, r );
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

float RingMaster::GetAmp(TimeValue t, Interval& valid ) 
{ 	
	float f = 0.0f;
	mPBlock->GetValue( PB_AMP, t, f, valid );
	return f;
}

//--------------------------------------------------
void RingMaster::SetPhs(TimeValue t, float r) 
{ 
	mPBlock->SetValue( PB_PHS, t, r );
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

float RingMaster::GetPhs(TimeValue t, Interval& valid ) 
{ 	
	float f = 0.0f;
	mPBlock->GetValue( PB_PHS, t, f, valid );
	return f;
}

//------------------------------------------------------------------------------
INT_PTR CALLBACK MasterParamDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
  RingMaster *mc = DLGetWindowLongPtr<RingMaster *>( hDlg);
	if (!mc && message != WM_INITDIALOG ) {
		return FALSE;
	}
	
	DbgAssert(mc->iObjParams);
	switch ( message ) 
	{
		case WM_INITDIALOG:
			mc = (RingMaster *)lParam;
      DLSetWindowLongPtr( hDlg, mc);
			SetDlgFont( hDlg, mc->iObjParams->GetAppHFont() );
			
			mc->radSpin  = GetISpinner(GetDlgItem(hDlg, IDC_RADSPINNER));
			mc->cycSpin  = GetISpinner(GetDlgItem(hDlg, IDC_CYCSPINNER));
			mc->ampSpin  = GetISpinner(GetDlgItem(hDlg, IDC_AMPSPINNER));
			mc->phsSpin  = GetISpinner(GetDlgItem(hDlg, IDC_PHSSPINNER));
			mc->numSpin  = GetISpinner(GetDlgItem(hDlg, IDC_NUMSPINNER));

			mc->radSpin->SetLimits(RingMaster::kMIN_RADIUS, RingMaster::kMAX_RADIUS, FALSE);
			mc->cycSpin->SetLimits(RingMaster::kMIN_CYCLES, RingMaster::kMAX_CYCLES, FALSE);
			mc->ampSpin->SetLimits(RingMaster::kMIN_AMPLITUDE, RingMaster::kMAX_AMPLITUDE, FALSE);
			mc->phsSpin->SetLimits(RingMaster::kMIN_PHASE, RingMaster::kMAX_PHASE, FALSE);
			mc->numSpin->SetLimits(RingMaster::kMIN_NODE_NUM, RingMaster::kMAX_NODE_NUM, FALSE);

			mc->radSpin->SetScale(float(0.1) );
			mc->ampSpin->SetScale(float(0.1) );
			mc->phsSpin->SetScale(float(0.1) );
			mc->numSpin->SetScale(float(0.1) );

			mc->radSpin->SetValue( mc->GetRad(mc->iObjParams->GetTime()), FALSE );
			mc->cycSpin->SetValue( mc->GetCyc(mc->iObjParams->GetTime()), FALSE );
			mc->ampSpin->SetValue( mc->GetAmp(mc->iObjParams->GetTime()), FALSE );
			mc->phsSpin->SetValue( mc->GetPhs(mc->iObjParams->GetTime()), FALSE );
			mc->numSpin->SetValue( mc->GetNum(mc->iObjParams->GetTime()), FALSE );

			mc->radSpin->LinkToEdit( GetDlgItem(hDlg,IDC_RADIUS), EDITTYPE_POS_UNIVERSE );			
			mc->cycSpin->LinkToEdit( GetDlgItem(hDlg,IDC_CYCLES), EDITTYPE_FLOAT );			
			mc->ampSpin->LinkToEdit( GetDlgItem(hDlg,IDC_AMPLITUDE), EDITTYPE_FLOAT );			
			mc->phsSpin->LinkToEdit( GetDlgItem(hDlg,IDC_PHASE), EDITTYPE_FLOAT );			
			mc->numSpin->LinkToEdit( GetDlgItem(hDlg,IDC_NUMNODES), EDITTYPE_INT );			
			
			return FALSE;	// DB 2/27

		case WM_DESTROY:
			ReleaseISpinner( mc->radSpin );
			ReleaseISpinner( mc->cycSpin );
			ReleaseISpinner( mc->ampSpin );
			ReleaseISpinner( mc->phsSpin );
			ReleaseISpinner( mc->numSpin );
			mc->radSpin = NULL;
			mc->cycSpin = NULL;
			mc->ampSpin = NULL;
			mc->phsSpin = NULL;
			mc->numSpin = NULL;
			return FALSE;

		case CC_SPINNER_CHANGE:	
		{
			if (!theHold.Holding()) {
				theHold.Begin();
			}
			TimeValue t = mc->iObjParams->GetTime();
			switch ( LOWORD(wParam) ) 
			{
				case IDC_RADSPINNER: 
					mc->SetRad(t,  mc->radSpin->GetFVal() );  
					mc->UpdateAnimKeyBrackets(t, RingMaster::PB_RAD);
					break;
				case IDC_CYCSPINNER: 
					mc->SetCyc(t,  mc->cycSpin->GetFVal() );  
					mc->UpdateAnimKeyBrackets(t, RingMaster::PB_CYC);
					break;
				case IDC_AMPSPINNER: 
					mc->SetAmp(t,  mc->ampSpin->GetFVal() );  
					mc->UpdateAnimKeyBrackets(t, RingMaster::PB_AMP);
					break;
				case IDC_PHSSPINNER: 
					mc->SetPhs(t,  mc->phsSpin->GetFVal() );  
					mc->UpdateAnimKeyBrackets(t, RingMaster::PB_PHS);
					break;
				case IDC_NUMSPINNER: 
					mc->SetNum(t,  mc->numSpin->GetIVal() );  
					break;
			}
			DbgAssert(mc->iObjParams);
			mc->iObjParams->RedrawViews(t, REDRAW_INTERACTIVE, mc);
			return TRUE;
		}

		case CC_SPINNER_BUTTONDOWN:
			theHold.Begin();
			return TRUE;

		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP:
			if (HIWORD(wParam) || message == WM_CUSTEDIT_ENTER) 
				theHold.Accept(GetString(IDS_DS_PARAMCHG));
			else 
				theHold.Cancel();
			mc->iObjParams->RedrawViews(mc->iObjParams->GetTime(), REDRAW_END, mc);
			return TRUE;

		case WM_MOUSEACTIVATE:
			mc->iObjParams->RealizeParamPanel();
			return FALSE;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			mc->iObjParams->RollupMouseMessage(hDlg, message, wParam, lParam);
			return FALSE;

		case WM_COMMAND:			
			return FALSE;

		default:
			return FALSE;
		}
}

void RingMaster::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	iObjParams = ip;
	
	if (!hMasterParams) 
	{
		hMasterParams = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_SAMPLEPARAM),
				MasterParamDialogProc,
				GetString(IDS_RB_PARAMETERS), 
				(LPARAM)this );		
		ip->RegisterDlgWnd(hMasterParams);
		
	} 
}
		
void RingMaster::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
	if (hMasterParams == NULL) {
		return;
	}
	dlgRadius   = radSpin->GetFVal();
	dlgAmplitude   = ampSpin->GetFVal();
	dlgCycles   = cycSpin->GetFVal();
	dlgPhase   = phsSpin->GetFVal();
	dlgNum   = numSpin->GetIVal();
	
	DLSetWindowLongPtr(hMasterParams, 0);
	ip->UnRegisterDlgWnd(hMasterParams);
	ip->DeleteRollupPage(hMasterParams);
	hMasterParams = NULL;
	
	iObjParams = NULL;
}

// IO
IOResult RingMaster::Save(ISave *isave) 
{
	ULONG nb;
	// Version chunck must be first 
	isave->BeginChunk(kVERSION_CHUNK);	
	unsigned short ver = kVERSION_CURRENT;
	isave->Write(&ver, sizeof(unsigned short), &nb);			
	isave->EndChunk();

	return IO_OK;
}

IOResult RingMaster::Load(ILoad *iload) 
{
	// Legacy max files do not contain a RingMaster version number
	IOResult res = IO_OK;
	USHORT next = iload->PeekNextChunkID();
	if (next != kVERSION_CHUNK)
		res = DoLoadLegacy(iload); 
	else
		res = DoLoad(iload);
	return res;
}

IOResult RingMaster::DoLoad(ILoad *iload)
{
	IOResult res;

	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID()) 
		{
			case kVERSION_CHUNK:
			{
				ULONG 	nb;
				iload->Read(&mVerLoaded, sizeof(unsigned short), &nb);
				if (kVERSION_CURRENT > mVerLoaded) {
					iload->SetObsolete();
				}
			}
			break;
		}
		iload->CloseChunk();
	}
	return IO_OK;
}

IOResult RingMaster::DoLoadLegacy(ILoad *iload)
{
	ULONG nb;
	IOResult res;
	int numNodes = -1;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())  
		{
			case kNUMNODES_CHUNK: 
			{
				res = iload->Read(&numNodes, sizeof(numNodes), &nb);
				mLegacyNodeTab.SetCount(numNodes);
			}
			break;
			
			case kNODE_ID_CHUNK:
				DbgAssert(numNodes >= 0);
				ULONG id = -1;
				for (int i = 0; i<numNodes; i++) 
				{
					iload->Read(&id, sizeof(ULONG), &nb);
					if (id != 0xffffffff) {
						iload->RecordBackpatch(id, (void**)&mLegacyNodeTab[i]);
					}
				}
			break;
		}
	
		iload->CloseChunk();
		if (res != IO_OK) {
			return res;
		}
	}
	iload->SetObsolete();

	iload->RegisterPostLoadCallback(new RingMasterPLC(*this));

	return IO_OK;
}


//------------------------------------------------------------------------------
// Slave controls
//------------------------------------------------------------------------------
class SlaveControl : public Control
{
	public:		
		RingMaster *master;
		ULONG id;

		SlaveControl(BOOL loading=FALSE) { master = NULL; id = 0; }
		SlaveControl(const SlaveControl& ctrl);
		SlaveControl(const RingMaster* m, int i);
		void SetID( ULONG i) { id = i;}
		virtual ~SlaveControl() {}	
		SlaveControl& operator=(const SlaveControl& ctrl);

		// From Control
		void Copy(Control *from) {}
		void CommitValue(TimeValue t) {}
		void RestoreValue(TimeValue t) {}
		virtual BOOL IsLeaf() {return FALSE;}
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValue(TimeValue t, void *val, int commit=1, GetSetMethod method=CTRL_ABSOLUTE);
		BOOL IsReplaceable() {return FALSE;}
		BOOL CanCopyAnim() {return FALSE;}

		// From Animatable
		void* GetInterface(ULONG id);
		int NumSubs()  { return master ? master->NumSubs() : 0; }
		Animatable* SubAnim(int i) { return master->SubAnim(i); }
		TSTR SubAnimName(int i) { return master->SubAnimName(i); }
		Class_ID ClassID() { return Class_ID(SLAVE_CONTROL_CLASS_ID,0); }  
		SClass_ID SuperClassID() { return CTRL_MATRIX3_CLASS_ID; }  
		void GetClassName(TSTR& s) { s = GetString(IDS_DB_SLAVECONTROL_CLASS); }
		void DeleteThis() { delete this; }		
		int IsKeyable(){ return 0;}
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev) { DbgAssert(master); master->BeginEditParams(ip,flags,prev); } 
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next) { DbgAssert(master); master->EndEditParams(ip,flags,next); } 
		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		// From ReferenceTarget
		RefTargetHandle Clone(RemapDir& remap);
		int NumRefs() { return 1;	};	
		RefTargetHandle GetReference(int i)  { DbgAssert(i==0); return master; }
		void SetReference(int i, RefTargetHandle rtarg) { DbgAssert(i==0); master = (RingMaster *)rtarg; }		
		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage) {return REF_SUCCEED;}
	};



class SlaveControlClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE) { return new SlaveControl(); }
	const TCHAR *	ClassName() { return GetString(IDS_DB_SLAVE_CONTROL); }
	SClass_ID		SuperClassID() { return CTRL_MATRIX3_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(SLAVE_CONTROL_CLASS_ID,0); }
	const TCHAR* 	Category() { return _T("");  }
	};

static SlaveControlClassDesc slvDesc;

ClassDesc* GetSlaveControlDesc() { return &slvDesc; }

Control* GetNewSlaveControl(RingMaster *master, int i) 
{
	return new SlaveControl(master,i);
}

SlaveControl::SlaveControl(const SlaveControl& ctrl) 
{
	master = ctrl.master;
	id = ctrl.id;
}

SlaveControl::SlaveControl(const RingMaster* m, int i) 
{
	master = NULL;
	id = i;
	ReplaceReference( 0, (ReferenceTarget *)m);
}

RefTargetHandle SlaveControl::Clone(RemapDir& remap) 
{
	SlaveControl *sl = new SlaveControl;
	sl->id = id;
	sl->ReplaceReference(0, remap.CloneRef(master));
	BaseClone(this, sl, remap);
	return sl;
}

SlaveControl& SlaveControl::operator=(const SlaveControl& ctrl) 
{
	master = ctrl.master;
	id = ctrl.id;
	return (*this);
}

void SlaveControl::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method) 
{
	DbgAssert(master);
	master->GetValue(t,val,valid,method,id);	
}


void SlaveControl::SetValue(TimeValue t, void *val, int commit, GetSetMethod method) { }

void* SlaveControl::GetInterface(ULONG id) 
{
	if (id==I_MASTER) 
		return (ReferenceTarget*)master;
	else 
		return Control::GetInterface(id);
}

// IO
#define SLAVE_ID_CHUNK 0x200
IOResult SlaveControl::Save(ISave *isave) 
{
	ULONG nb;
	isave->BeginChunk(SLAVE_ID_CHUNK);
	isave->Write(&id,sizeof(id), &nb);
	isave->EndChunk();
	return IO_OK;
}

IOResult SlaveControl::Load(ILoad *iload) 
{
	ULONG nb;
	IOResult res;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch(iload->CurChunkID())  
		{
			case SLAVE_ID_CHUNK:
				res = iload->Read(&id,sizeof(id), &nb);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
	return IO_OK;
}

//------------------------------------------------------------------------------
class RingMasterCreationManager : public MouseCallBack, ReferenceMaker 
{
	public:
		CreateMouseCallBack *createCB;	
		INode *node0;
		RingMaster *theMaster;
		IObjCreate *createInterface;
		ClassDesc *cDesc;
		Matrix3 mat;  // the nodes TM relative to the CP
		IPoint2 pt0;
		Point3 center;
		BOOL attachedToNode;
		int lastPutCount;

		void CreateNewMaster();
			
		int ignoreSelectionChange;

		int NumRefs() { return 1; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		// StdNotifyRefChanged calls this, which can change the partID to new value 
		// If it doesnt depend on the particular message& partID, it should return
		// REF_DONTCARE
    RefResult NotifyRefChanged(
			Interval changeInt, 
			RefTargetHandle hTarget, 
    	PartID& partID,  
			RefMessage message);

		void Begin( IObjCreate *ioc, ClassDesc *desc );
		void End();
		
		RingMasterCreationManager()	{
			ignoreSelectionChange = FALSE;
		}
		int proc( HWND hwnd, int msg, int point, int flag, IPoint2 m );
};

#define CID_BONECREATE	CID_USER + 1

class RingMasterCreateMode : public CommandMode 
{
		RingMasterCreationManager proc;
	public:
		void Begin( IObjCreate *ioc, ClassDesc *desc ) { proc.Begin( ioc, desc ); }
		void End() { proc.End(); }
		int Class() { return CREATE_COMMAND; }
		int ID() { return CID_BONECREATE; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints = 100000; return &proc; }
		ChangeForegroundCallback *ChangeFGProc() { return CHANGE_FG_SELECTED; }
		BOOL ChangeFG( CommandMode *oldMode ) { return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED); }
		void EnterMode() { SetCursor( LoadCursor( hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR) ) ); }
		void ExitMode() { SetCursor( LoadCursor(NULL, IDC_ARROW) ); }
		BOOL IsSticky() { return FALSE; }
};

static RingMasterCreateMode theRingMasterCreateMode;

void RingMasterCreationManager::Begin( IObjCreate *ioc, ClassDesc *desc )
{
	createInterface = ioc;
	cDesc           = desc;
	createCB        = NULL;
	node0			= NULL;
	theMaster 		= NULL;
	attachedToNode = FALSE;
	CreateNewMaster();
}

void RingMasterCreationManager::SetReference(int i, RefTargetHandle rtarg) 
{ 
	switch(i) 
	{
		case 0: 
			node0 = (INode *)rtarg; 
		break;
		default: 
			DbgAssert(0); 
		break;
	}
}

RefTargetHandle RingMasterCreationManager::GetReference(int i) 
{ 
	switch(i) 
	{
		case 0: 
			return (RefTargetHandle)node0;
		default: 
			DbgAssert(0); 
		break;
	}
	return NULL;
}

void RingMasterCreationManager::End()
{
	if (theMaster) 
	{
		BOOL bDestroy = TRUE;
		theMaster->EndEditParams( (IObjParam*)createInterface, bDestroy, NULL );
		if ( !attachedToNode ) 
		{
			theHold.Suspend(); 
			//delete lgtObject;
			theMaster->DeleteAllRefsFromMe();
			theMaster->DeleteAllRefsToMe();
			theMaster->DeleteThis(); 
			theMaster = NULL;
			theHold.Resume();


			// DS 8/21/97: If something has been put on the undo stack since this 
			// object was created, we have to flush the undo stack.
			if (theHold.GetGlobalPutCount()!=lastPutCount) {
				GetSystemSetting(SYSSET_CLEAR_UNDO);
			}
		} 
		else if ( node0 ) 
		{
		 // Get rid of the references.
			DeleteAllRefsFromMe();
		}
		theMaster = NULL; //JH 9/15/97
	}	
}

RefResult RingMasterCreationManager::NotifyRefChanged(
	Interval changeInt, 
	RefTargetHandle hTarget, 
	PartID& partID,  
	RefMessage message) 
{
	switch (message) 
	{
		case REFMSG_TARGET_SELECTIONCHANGE:
		 	if ( ignoreSelectionChange ) {
				break;
			}
		 	if (theMaster) 
			{
				// this will set node0 ==NULL;
				DeleteAllRefsFromMe();
				goto endEdit;
			}
			else 
			{
				return REF_SUCCEED;  //JH 9.15.97 
			}
			// fall through

		case REFMSG_TARGET_DELETED:
			if (hTarget == node0)
			{
				node0 = NULL;
			}
			if (theMaster) 
			{
				endEdit:
				BOOL bDestroy = FALSE;
				theMaster->EndEditParams( (IObjParam*)createInterface, bDestroy, NULL );
				theMaster = NULL;
				node0 = NULL;
				CreateNewMaster();	
				attachedToNode = FALSE;
			}
		break;		
	} // end switch
	return REF_SUCCEED;
}

void RingMasterCreationManager::CreateNewMaster()
{
	theMaster = new RingMaster(RingMaster::dlgNum);
	
	// Start the edit params process
	theMaster->BeginEditParams( (IObjParam*)createInterface, BEGIN_EDIT_CREATE,NULL );
	lastPutCount = theHold.GetGlobalPutCount();
}

#define DUMSZ 20.0f
#define BOXSZ 20.0f

static BOOL needToss;

int RingMasterCreationManager::proc( 
	HWND hwnd,
	int msg,
	int point,
	int flag,
	IPoint2 m )
{	
	int res = TRUE;
	INode *newNode,*dummyNode;	
	float r;
	ViewExp *vpx = createInterface->GetViewport(hwnd); 
	DbgAssert(vpx );

	switch ( msg ) 
	{
		case MOUSE_POINT:
		{
			if (point==0) 
			{
				pt0 = m;	
				DbgAssert(theMaster);

				mat.IdentityMatrix();
				if ( createInterface->SetActiveViewport(hwnd) ) {
					return FALSE;
				}
				if (createInterface->IsCPEdgeOnInView()) { 
					return FALSE;
				}
				if ( attachedToNode ) 
				{
			  	// send this one on its way
			  	theMaster->EndEditParams( (IObjParam*)createInterface,0,NULL );
					
					// Get rid of the references.
					DeleteAllRefsFromMe();

					// new object
					CreateNewMaster();   // creates theMaster
				}

				needToss = theHold.GetGlobalPutCount()!=lastPutCount;

			  theHold.Begin();	 // begin hold for undo
				mat.IdentityMatrix();
				center = vpx->SnapPoint(m, m, NULL, SNAP_IN_PLANE);
				mat.SetTrans(center);

				// Create a dummy object & node
				DummyObject *dumObj = (DummyObject *)createInterface->CreateInstance(
					HELPER_CLASS_ID, Class_ID(DUMMY_CLASS_ID, 0)); 			
				DbgAssert(dumObj);					
				dummyNode = createInterface->CreateObjectNode(dumObj);
				dumObj->SetBox(Box3(Point3(-DUMSZ, -DUMSZ, -DUMSZ), Point3(DUMSZ, DUMSZ, DUMSZ)));

				// make a box object
				GenBoxObject *ob = (GenBoxObject *)createInterface->
					CreateInstance(GEOMOBJECT_CLASS_ID,Class_ID(BOXOBJ_CLASS_ID,0));
				ob->SetParams(BOXSZ,BOXSZ,BOXSZ,1,1,1,FALSE); 

				// Make a bunch of nodes, hook the box object to and a
				// slave controller of the master control to each
				for (int i = 0; i < theMaster->GetNum(createInterface->GetTime()); i++) 
				{
					newNode = createInterface->CreateObjectNode(ob);
					SlaveControl* slave = new SlaveControl(theMaster,i);
					newNode->SetTMController(slave);
					dummyNode->AttachChild(newNode);
					theMaster->SetSlaveNode(i,newNode);
				}

				// select the dummy node.
				attachedToNode = TRUE;

				// Reference the node so we'll get notifications.
				ReplaceReference( 0, theMaster->GetSlaveNode(0) );
				theMaster->SetRad(TimeValue(0), 0.0f);
				mat.SetTrans(vpx->SnapPoint(m, m, NULL, SNAP_IN_PLANE));
				createInterface->SetNodeTMRelConstPlane(dummyNode, mat);
				res = TRUE;
			}
			else 
			{
				// select a node so if go into modify branch, see params 
				ignoreSelectionChange = TRUE;
			 	createInterface->SelectNode( theMaster->GetSlaveNode(0) );
				ignoreSelectionChange = FALSE;
				theHold.Accept(IDS_DS_CREATE);
				res = FALSE;
			}
			createInterface->RedrawViews(createInterface->GetTime(), REDRAW_NORMAL, theMaster);  
		}
		break;
		case MOUSE_MOVE:
			if (node0) 
			{
				r = (float)fabs(vpx->SnapLength(vpx->GetCPDisp(center, Point3(0,1,0), pt0, m)));
				theMaster->SetRad(0, r);
				theMaster->radSpin->SetValue(r, FALSE );
				createInterface->RedrawViews(createInterface->GetTime(), REDRAW_NORMAL, theMaster);
			}
			res = TRUE;
			break;

		case MOUSE_FREEMOVE:
			SetCursor( LoadCursor( hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR) ) );
			break;

		case MOUSE_PROPCLICK:
			// right click while between creations
			createInterface->RemoveMode(NULL);
			break;

		case MOUSE_ABORT:
			DbgAssert(theMaster);
			theMaster->EndEditParams( (IObjParam*)createInterface, 0,NULL );
			theHold.Cancel();  // undo the changes
			// DS 8/21/97: If something has been put on the undo stack since this object was 
			// created, we have to flush the undo stack.
			if (needToss) {
				GetSystemSetting(SYSSET_CLEAR_UNDO);
			}
			DeleteAllRefsFromMe();
			CreateNewMaster();	
			createInterface->RedrawViews(createInterface->GetTime(),REDRAW_END,theMaster); 
			attachedToNode = FALSE;
			res = FALSE;						
			break;
	} // end switch
	
	createInterface->ReleaseViewport(vpx); 
	return res;
}

int RingMasterClassDesc::BeginCreate(Interface *i)
{
	SuspendSetKeyMode();
	IObjCreate *iob = i->GetIObjCreate();
	
	theRingMasterCreateMode.Begin( iob, this );
	iob->PushCommandMode( &theRingMasterCreateMode );
	
	return TRUE;
}

int RingMasterClassDesc::EndCreate(Interface *i)
{
	ResumeSetKeyMode();
	theRingMasterCreateMode.End();
	i->RemoveMode( &theRingMasterCreateMode );
	return TRUE;
}


//------------------------------------------------------------------------------
// Ringarray mxs exposure
//------------------------------------------------------------------------------
// --- Mixin RingArray Master interface ---
FPInterfaceDesc RingMaster::mFPInterfaceDesc(
  IID_RING_ARRAY_MASTER, 
	_T("IRingArrayMaster"),		// Interface name used by maxscript - don't localize it!
	0,												// Res ID of description string
	&theRingMasterClassDesc,	// Class descriptor
	FP_MIXIN,

	// - Methods -
	RingMaster::kRM_GetNodes, _T("GetNodes"), 0, TYPE_INT, 0, 2,
		_T("nodes"), 0, TYPE_INODE_TAB_BR, f_inOut, FPP_IN_OUT_PARAM, 
		_T("context"), 0, TYPE_ENUM, RingMaster::kfpSystemNodeContexts, 
	RingMaster::kRM_GetNode, _T("GetNode"), 0, TYPE_INODE, 0, 1,
		_T("nodeIdx"), 0, TYPE_INDEX, f_inOut, FPP_IN_PARAM, 
	
	// - Properties -
	properties,
		RingMaster::kRM_GetNodeNum, RingMaster::kRM_SetNodeNum, _T("numNodes"),	0, TYPE_INT, 
			f_range, RingMaster::kMIN_NODE_NUM, RingMaster::kMAX_NODE_NUM,
		RingMaster::kRM_GetCycle, RingMaster::kRM_SetCycle, _T("cycles"),	0, TYPE_FLOAT, 
			f_range, RingMaster::kMIN_CYCLES, RingMaster::kMAX_CYCLES,
		RingMaster::kRM_GetPhase, RingMaster::kRM_SetPhase, _T("phase"),	0, TYPE_FLOAT, 
			f_range, RingMaster::kMIN_PHASE, RingMaster::kMAX_PHASE,
		RingMaster::kRM_GetAmplitute, RingMaster::kRM_SetAmplitude, _T("amplitude"),	0, TYPE_FLOAT, 
			f_range, RingMaster::kMIN_AMPLITUDE, RingMaster::kMAX_AMPLITUDE,
		RingMaster::kRM_GetRadius, RingMaster::kRM_SetRadius, _T("radius"),	0, TYPE_FLOAT, 
			f_range, RingMaster::kMIN_RADIUS, RingMaster::kMAX_RADIUS,

	enums,
		RingMaster::kfpSystemNodeContexts, 4, 
		_T("Ctx_Clone"), kSNCClone,
		_T("Ctx_Delete"), kSNCDelete,
		_T("Ctx_FileMerge"), kSNCFileMerge,
		_T("Ctx_FileSave"), kSNCFileSave,

	end 
);

FPInterfaceDesc* RingMaster::GetDesc() 
{ 
	return &mFPInterfaceDesc; 
}

FPInterfaceDesc* RingMaster::GetDescByID(Interface_ID id)
{
	if (IID_RING_ARRAY_MASTER == id)
		return &mFPInterfaceDesc;
	else
		return FPMixinInterface::GetDescByID(id);
}

BaseInterface* RingMaster::GetInterface(Interface_ID id)
{
	if (IID_RING_ARRAY_MASTER == id) 
		return static_cast<RingMaster*>(this); 
	else 
		return FPMixinInterface::GetInterface(id); 
}


// --- Static Mxs interface for creating RingArrays -----------------------------
#define IID_RINGARRAY_SYSTEM Interface_ID(0x7a5a74ed, 0x52304621)
class FPRingArraySystem : public FPStaticInterface 
{
	public:
   DECLARE_DESCRIPTOR(FPRingArraySystem);

   enum fnID {
     kRA_Create, 
   };

   BEGIN_FUNCTION_MAP
     FN_6(kRA_Create, TYPE_INTERFACE, fpCreate, TYPE_POINT3_BR, TYPE_INT, 
			TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT);
	 END_FUNCTION_MAP

	FPInterface* fpCreate(
		const Point3& posOrigin, 
		int numNodes,
		float amplitude,
		float radius,
		float cycles,
		float phase);
};

namespace { 

static FPRingArraySystem fpRingArray (
	IID_RINGARRAY_SYSTEM, 
	_T("RingArray"), 
	NULL, NULL, 
	FP_CORE,

	FPRingArraySystem::kRA_Create, _T("Create"), 0, TYPE_INTERFACE, 0, 6,
		_T("posOrigin"), 0, TYPE_POINT3_BR, f_inOut, FPP_IN_PARAM, f_keyArgDefault, &Point3::Origin,
		_T("numNodes"), 0, TYPE_INT, 
			f_inOut, FPP_IN_PARAM, f_keyArgDefault, RingMaster::kDEF_NODE_NUM,
			f_range, RingMaster::kMIN_NODE_NUM, RingMaster::kMAX_NODE_NUM,
		_T("amplitude"), 0, TYPE_FLOAT, 
			f_inOut, FPP_IN_PARAM, f_keyArgDefault, RingMaster::kDEF_AMPLITUDE,
			f_range, RingMaster::kMIN_AMPLITUDE, RingMaster::kMAX_AMPLITUDE,
		_T("radius"), 0, TYPE_FLOAT, 
			f_inOut, FPP_IN_PARAM, f_keyArgDefault, RingMaster::kDEF_RADIUS,
			f_range, RingMaster::kMIN_RADIUS, RingMaster::kMAX_RADIUS,
		_T("cycles"), 0, TYPE_FLOAT, 
			f_inOut, FPP_IN_PARAM, f_keyArgDefault, RingMaster::kDEF_CYCLES,
			f_range, RingMaster::kMIN_CYCLES, RingMaster::kMAX_CYCLES,
		_T("phase"), 0, TYPE_FLOAT, 
			f_inOut, FPP_IN_PARAM, f_keyArgDefault, RingMaster::kDEF_PHASE,
			f_range, RingMaster::kMIN_PHASE, RingMaster::kMAX_PHASE,
	end
);

};

FPInterface* FPRingArraySystem::fpCreate(
	const Point3& posOrigin, 
	int numNodes,
	float amplitude,
	float radius,
	float cycles,
	float phase)
{
	Interface* ip = GetCOREInterface();
	RingMaster* theMaster = new RingMaster(numNodes);
	DbgAssert(theMaster != NULL);

	// The creation params used in interactive mode need to be kept up-to-date
	// with the param values used via maxscript creation in order to ensure that 
	// the UI displays the values actually used via maxscript creation
	RingMaster::dlgNum = numNodes;
	RingMaster::dlgAmplitude = amplitude;
	RingMaster::dlgRadius = radius;
	RingMaster::dlgCycles = cycles;
	RingMaster::dlgPhase = phase;

	// Create a dummy object & node
	DummyObject* dumObj = (DummyObject*)ip->CreateInstance(HELPER_CLASS_ID, 
		Class_ID(DUMMY_CLASS_ID, 0)); 			
	DbgAssert(dumObj);					
	INode* dummyNode = ip->CreateObjectNode(dumObj);
	dumObj->SetBox(Box3(Point3(-DUMSZ, -DUMSZ, -DUMSZ), Point3(DUMSZ, DUMSZ, DUMSZ)));

	// make a box object
	GenBoxObject* ob = (GenBoxObject*)ip->CreateInstance(GEOMOBJECT_CLASS_ID, 
		Class_ID(BOXOBJ_CLASS_ID, 0));
	DbgAssert(ob != NULL);
	ob->SetParams(BOXSZ, BOXSZ, BOXSZ, 1, 1, 1, FALSE); 

	// Make a bunch of nodes with a slave controllers. Hook the nodes to the box 
	// and the slave ctrls to the ring master
	for (int i = 0; i < theMaster->GetNum(ip->GetTime()); i++) 
	{
		INode* newNode = ip->CreateObjectNode(ob);
		SlaveControl* slave = new SlaveControl(theMaster, i);
		newNode->SetTMController(slave);
		dummyNode->AttachChild(newNode);
		theMaster->SetSlaveNode(i, newNode);
	}

	// Position the system's root node
	Matrix3 mat;
	mat.IdentityMatrix();
	mat.SetTrans(posOrigin);
	ip->SetNodeTMRelConstPlane(dummyNode, mat);

	// Set other parameters
	theMaster->SetAmp(ip->GetTime(), amplitude);
	theMaster->SetRad(ip->GetTime(), radius);
	theMaster->SetCyc(ip->GetTime(), cycles);
	theMaster->SetPhs(ip->GetTime(), phase);

	return static_cast<FPInterface*>(theMaster);
}

#endif // NO_OBJECT_RINGARRAY
// EOF
