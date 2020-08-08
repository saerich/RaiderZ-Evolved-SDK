/**********************************************************************
 *<
	FILE: reactor.h

	DESCRIPTION: Header file for Reactor Controller

	CREATED BY: Adam Felt

	HISTORY:

 *>	Copyright (c) 1998-1999 Adam Felt, All Rights Reserved.
 **********************************************************************/

#ifndef __REACTOR__H
#define __REACTOR__H


#include "Max.h"
#include "resource.h"
#include "ActionTable.h"
#include "Maxscrpt.h"
#include "definsfn.h"

#include "iparamm.h"
#include "ReactAPI.h"
#include "macrorec.h"
#include "notify.h"

extern ClassDesc* GetFloatReactorDesc();
extern ClassDesc* GetPositionReactorDesc();
extern ClassDesc* GetPoint3ReactorDesc();
extern ClassDesc* GetRotationReactorDesc();
extern ClassDesc* GetScaleReactorDesc();
extern ClassDesc* GetReactionMasterDesc();

extern HINSTANCE hInstance;

//--------------------------------------------------------------------------

// Keyboard Shortcuts stuff
const ActionTableId kReactionMgrActions = 0x6bd55e20;
const ActionContextId kReactionMgrContext = 0x6bd55e20;

TCHAR *GetString(int id);
ActionTable* GetActions();
void GetAbsoluteControlValue(INode *node,int subNum, TimeValue t,void *pt,Interval &iv);

void __cdecl MaybeDeleteThisProc(void *param, NotifyInfo *info);

//-----------------------------------------------------------------------

#define REACTOR_DONT_SHOW_CREATE_MSG		(1<<1)  //don't display the Create reaction warning message
#define REACTOR_DONT_CREATE_SIMILAR			(1<<2)  //don't create a reaction if it has the same value as an existing reaction
#define REACTOR_BLOCK_CURVE_UPDATE			(1<<3)  //don't update the Curves
#define REACTOR_BLOCK_REACTION_DELETE		(1<<4)  //only delete one reaction if multiple keys are selected

#define REACTORDLG_LIST_CHANGED_SIZE		(1<<5)  //refresh the listbox and everything depending on selection
#define REACTORDLG_LIST_SELECTION_CHANGED   (1<<6)  //update the fields depending on the list selection
#define REACTORDLG_REACTTO_NAME_CHANGED		(1<<7)  
//#define REACTORDLG_REACTION_NAME_CHANGED	(1<<8)

#define REFMSG_REACTOR_SELECTION_CHANGED 0x00022000
#define REFMSG_REACTION_COUNT_CHANGED	 0x00023000
#define REFMSG_REACTTO_OBJ_NAME_CHANGED	 0x00024000
#define REFMSG_USE_CURVE_CHANGED		 0x00025000
#define REFMSG_REACTION_NAME_CHANGED	 0x00026000

#define NumElements(array) (sizeof(array) / sizeof(array[0]))
#define REACTION_MASTER_CLASSID Class_ID(0x29651db6, 0x2c8515c2)

template <class T> class ReactorActionCB;
class Reactor;

//Pick Mode Stuff
//******************************************

class PickObjectCallback
{
public:
	virtual TSTR GetName()=0;
	virtual bool LoopTest(ReferenceTarget* anim)=0;
	virtual void NodePicked(ReferenceTarget* anim)=0;
	virtual bool IncludeRoots()=0;
	virtual bool MultipleChoice()=0;
	virtual void PostPick()=0;
};

class ReactionPickMode : 
		public PickModeCallback,
		public PickNodeCallback, 
		public PickObjectCallback
{
		Tab<ReferenceTarget*> nodes;
public:		
		ReactionPickMode() {nodes.SetCount(0);}

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);
		BOOL PickAnimatable(Animatable* anim);
		BOOL RightClick(IObjParam *ip,ViewExp *vpt)	{return TRUE;}
		BOOL Filter(INode *node);		
		PickNodeCallback *GetFilter() {return this;}
		BOOL FilterAnimatable(Animatable *anim);
		BOOL AllowMultiSelect(){ return MultipleChoice(); }
		bool MultipleChoice() { return false; }
		void ExitMode(IObjParam *ip);
		virtual void PostPick() {};
};

//-----------------------------------------------

//Storage class for the animatable right-click style menu
//*******************************************************
struct SubAnimPair
{
	ReferenceTarget* parent;
	int id;
	SubAnimPair() {parent = NULL; id = -1;}
};

class AnimEntry
{ 
public:
	TCHAR*		pname;			// param name
	int			level;			// used during pop-up building, 0->same, 1->new level, -1->back level
	//ReferenceTarget* root;		// display root
	Tab<SubAnimPair*> pair;		// anim/subanim num pair

	AnimEntry() : pname(NULL) { pair.SetCount(0); }

	AnimEntry(TCHAR* n, int l, Tab<SubAnimPair*> p) :
		pname(n ? save_string(n) : n), level(l) 
		{ 
			pair.SetCount(0); 
			for(int i = 0; i < p.Count(); i++)
				pair.Append(1, &p[i]);
		}

	~AnimEntry() 
	{ 
		for(int i = 0; i < pair.Count(); i++)
			delete pair[i];
	}

	AnimEntry& operator=(const AnimEntry& from)
	{
		Clear();
		pname = from.pname ? save_string(from.pname) : NULL;
		level = from.level;
		pair.SetCount(0);
		for(int i = 0; i < from.pair.Count(); i++)
		{
			SubAnimPair *p = new SubAnimPair();
			p->id = from.pair[i]->id;
			p->parent = from.pair[i]->parent;
			pair.Append(1, &p);
		}
		return *this;
	}

	void Clear()
	{
		if (pname) free(pname);
		pname = NULL;
	}

	Animatable* Anim(int i) { return pair[i]->parent ? pair[i]->parent->SubAnim(pair[i]->id) : NULL; }
};

/* -----  viewport right-click menu ----------------- */

class AnimPopupMenu
{
private:
//	INode*		node;				// source node & param...
//	AnimEntry	entry;
	Tab<HMENU>  menus;

	void		add_hmenu_items(Tab<AnimEntry*>& wparams, HMENU menu, int& i);
	bool		wireable(Animatable* parent, int subnum);
	void		build_params(PickObjectCallback* cb, Tab<AnimEntry*>& wparams, Tab<ReferenceTarget*> root);
	bool		add_subanim_params(PickObjectCallback* cb, Tab<AnimEntry*>& wparams, TCHAR* name, Tab<ReferenceTarget*> parents, int level, Tab<ReferenceTarget*> roots);
	AnimEntry*	find_param(Tab<AnimEntry>& params, Animatable* anim, Animatable* parent, int subNum);
	bool		Filter(PickObjectCallback* cb, Animatable* anim, bool includeChildren=false);

public:
				AnimPopupMenu() { }
	void		DoPopupMenu(PickObjectCallback* cb, Tab<ReferenceTarget*> node);
	Control*	AssignDefaultController(Animatable* parent, int subnum);
};

static AnimPopupMenu theAnimPopupMenu;

class MasterState
{
public:
	TSTR	name;
	int		type;

	//The type used here is the same as the client track
	Point3	pvalue;		//current value if it is a point3
	float	fvalue;		//current value if it's a float
	Quat	qvalue;		//current value if it's a quat

	MasterState(MasterState* state)	
	{
		*this = *state;
	}

	MasterState(TSTR n, int t, void* val)	
	{ 
		name = n;
		type = t;
		if (val != NULL)
		{
			switch(type)
			{
				case FLOAT_VAR:
					fvalue = *(float*)val;
					break;
				case SCALE_VAR:
				case VECTOR_VAR:
					pvalue = *(Point3*)val;
					break;
				case QUAT_VAR:
					qvalue = *(Quat*)val;
					break;
			}
		}
	}

	MasterState& operator=(const MasterState& from){
		name	= from.name;
		type	= from.type;
		pvalue	= from.pvalue;
		fvalue	= from.fvalue;
		qvalue	= from.qvalue;
		return *this;
	}

	MasterState* Clone()
	{
		MasterState* state = new MasterState(this);
		return state;
	}

};

//Storage class for the track being referenced
//*********************************************
class ReactionMaster : public ReferenceTarget {
public:
	ReferenceTarget* client;
	int subnum;
	Tab<MasterState*> states;
	int type;
	NameMaker*		nmaker;

	ReactionMaster()	{ client = NULL; states.Init(); type = 0;nmaker = NULL;}
	ReactionMaster(ReferenceTarget* c, int id);	
	~ReactionMaster()
	{
		for(int i = states.Count()-1; i >= 0; i--)
			delete states[i];
		if(nmaker) delete nmaker;
	}

	ReactionMaster& operator=(const ReactionMaster& from){
		subnum = from.subnum;
		type = from.type;
		for(int i = states.Count()-1; i >= 0; i--)
			delete states[i];
		states.SetCount(0);

		if (nmaker) delete nmaker;
		nmaker = GetCOREInterface()->NewNameMaker(FALSE);
		for (int i=0;i<from.states.Count();i++)
		{
			nmaker->AddName(from.states[i]->name);
			MasterState* state = new MasterState(_T(""), 0, NULL);
			*(MasterState*)state = *(MasterState*)from.states[i];
			states.Append(1, &state);
		}
		return *this;
	}

	RefTargetHandle Clone(RemapDir& remap);

	void DeleteThis() {
		delete this;
	}		
	Class_ID ClassID() { return REACTION_MASTER_CLASSID; }  
	SClass_ID SuperClassID() { return REF_MAKER_CLASS_ID; } 
	void GetClassName(TSTR& s) {s = GetString(IDS_AF_REACTION_MASTER);}

	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);

	int NumRefs() { return 1; }
	ReferenceTarget* GetReference(int i) { return client; }
	void SetReference(int i, RefTargetHandle rtarg) { client = rtarg; }
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID &partID, RefMessage message);
	BOOL IsRealDependency(ReferenceTarget* rtarg) { return FALSE; }

	ReferenceTarget* GetMaster() 
	{ 
		if (subnum < 0 )
			return client;
		else
			return (ReferenceTarget*)client->SubAnim(subnum);
	}

	ReferenceTarget* Owner() { return client; }
	int SubIndex() { return subnum; }

	MasterState* GetState(int i) { if (i >= states.Count()) return NULL; return states[i]; }
	MasterState* AddState(TCHAR* name, int &id, bool setDefaults, TimeValue t);
	void DeleteState(int i);
	int GetType(){ return type; }
	void GetValue(TimeValue t, void* val);
	void SetState(int i, float val);
	void SetState(int i, Point3 val);
	void SetState(int i, Quat val);

	void CopyFrom(Reactor* r);
};

// ReactionSet variables
//*****************************************
class SlaveState {
public:

	int		masterID;
	float	influence;
	float	multiplier; 
	float	strength;
	float	falloff;

	//The type used here is the same as the controller type
	float	fstate;		//reaction state if it's a float
	Quat	qstate;		//reaction state if it's a quat
	Point3	pstate;		//reaction state if it's a point3
	
	SlaveState& operator=(const SlaveState& from){
		masterID	= from.masterID;
		multiplier	= from.multiplier;
		influence	= from.influence;
		strength	= from.strength;
		falloff		= from.falloff;
		fstate		= from.fstate;
		qstate		= from.qstate;
		pstate		= from.pstate;
		return *this;
	}
};

class Reactor : public IReactor, public ResourceMakerCallback {

	ReactionMaster* master;

	public:
		int				type, selected, count;
		int				isCurveControlled;
		BOOL			editing;  //editing the reaction state
		BOOL			createMode;
		Tab<SlaveState>	slaveStates;
		Interval		ivalid;
		Interval		range;
		ICurveCtl*		iCCtrl;
		UINT			flags;
		bool			curvesValid;

		Point3 curpval;
		Point3 upVector;
		float curfval;
		Quat curqval;
		BOOL blockGetNodeName; // RB 3/23/99: See imp of getNodeName()

		ReactorActionCB<Reactor >	*reactorActionCB;		// Actions handler 		
		
		ReactionPickMode *pickReactToMode;

		virtual int Elems(){return 0;}
		
		Reactor();
		Reactor(int t, BOOL loading);
		Reactor& operator=(const Reactor& from);
		~Reactor();
		void Init(BOOL loading = FALSE);

		BOOL	assignReactObj(ReferenceTarget* client, int subnum);
		BOOL	reactTo(ReferenceTarget* anim, TimeValue t = GetCOREInterface()->GetTime(), bool createReaction = true);
		ReactionMaster* GetMaster() { return master; }
		void	updReactionCt(int val);
		SlaveState*	CreateReactionAndReturn(BOOL setDefaults = TRUE, TCHAR *buf=NULL, TimeValue t = GetCOREInterface()->GetTime(), int masterID = -1);
		BOOL	CreateReaction(TCHAR *buf=NULL, TimeValue t = GetCOREInterface()->GetTime());
		BOOL	DeleteReaction(int i=-1);
		int		getReactionCount() { return slaveStates.Count(); }
		void	deleteAllVars();
		int		getSelected() {return selected;}
		void	setSelected(int i); 
		int 	getType(){ return type; }
		int		getReactionType() { if (master == NULL) return 0; return master->GetType(); }
		TCHAR*	getReactionName(int i);
		void	setReactionName(int i, TSTR name);
		void*	getReactionValue(int i);
		BOOL	setReactionValue(int i=-1, TimeValue t=NULL);
		BOOL	setReactionValue(int i, float val);
		BOOL	setReactionValue(int i, Point3 val);
		BOOL	setReactionValue(int i, Quat val);
		float	getCurFloatValue(TimeValue t);
		Point3	getCurPoint3Value(TimeValue t);
		ScaleValue	getCurScaleValue(TimeValue t);
		Quat	getCurQuatValue(TimeValue t);
		BOOL	setInfluence(int num, float inf);
		float	getInfluence(int num);
		void	setMinInfluence(int x=-1);
		void	setMaxInfluence(int x=-1);
		BOOL	setStrength(int num, float inf);
		float	getStrength(int num);
		BOOL	setFalloff(int num, float inf);
		float	getFalloff(int num);
		void	setEditReactionMode(BOOL edit);
		BOOL	getEditReactionMode(){return editing;}
		void	setCreateReactionMode(BOOL edit);
		BOOL	getCreateReactionMode();

		void*	getState(int num);
		BOOL	setState(int num=-1, TimeValue t=NULL);
		BOOL	setState(int num, float val);
		BOOL	setState(int num, Point3 val);
		BOOL	setState(int num, Quat val);
		void	getNodeName(ReferenceTarget *client, TSTR &name);
		Point3	getUpVector() {return upVector;}
		void	setUpVector(Point3 up) {upVector = up;}
		BOOL	useCurve(){return isCurveControlled;}
		void	useCurve(BOOL use) {isCurveControlled = use; NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);}
		ICurveCtl*	getCurveControl() {return iCCtrl;}
		void	SortReactions();

		void	Update(TimeValue t);
		void	ComputeMultiplier(TimeValue t);
		BOOL	ChangeParents(TimeValue t,const Matrix3& oldP,const Matrix3& newP,const Matrix3& tm);
		void	ShowCurveControl();
		void	BuildCurves();
		void	RebuildFloatCurves();
		void	UpdateCurves(bool doZoom = true);
		void	GetCurveValue(TimeValue t, float* val, Interval &valid);

		FPValue fpGetReactionValue(int index);
		FPValue fpGetState(int index);
		
		MasterState* getMasterState(int i);

		// Animatable methods		
		void DeleteThis() {delete this;}		
		int IsKeyable() {if (!createMode && !editing) return 0; else return 1;}		
		BOOL IsAnimated() {if (slaveStates.Count() > 1 && !createMode && !editing) return true; else return false;}
		Interval GetTimeRange(DWORD flags) { return range; }

		void EditTimeRange(Interval range,DWORD flags);
		void MapKeys(TimeMap *map,DWORD flags);

		void HoldTrack();
		void HoldAll();
		void HoldParams();
		void HoldRange();

		int NumSubs();
		BOOL AssignController(Animatable *control,int subAnim) {return false;}
		Animatable* SubAnim(int i){return NULL;}
		TSTR SubAnimName(int i){ return "";}

		// Animatable's Schematic View methods
		SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags);
		TSTR SvGetRelTip(IGraphObjectManager *gom, IGraphNode *gNodeTarger, int id, IGraphNode *gNodeMaker);
		bool SvHandleRelDoubleClick(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker);

		void EditTrackParams(
			TimeValue t,
			ParamDimensionBase *dim,
			TCHAR *pname,
			HWND hParent,
			IObjParam *ip,
			DWORD flags);
		int TrackParamsType() {return TRACKPARAMS_WHOLE;}

		// Reference methods
		int NumRefs();
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		// The reaction master is a weak reference
		BOOL IsRealDependency(ReferenceTarget* rtarg) { return TRUE; }
		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage);
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
		int RemapRefOnLoad(int iref);

		// Control methods				
		void Copy(Control *from);
		BOOL IsLeaf() {return FALSE;}

		//These three default implementation are shared by Position, Point3 and Scale controllers
		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
		void CommitValue(TimeValue t);
		void RestoreValue(TimeValue t);		

		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method){} //overwritten by everyone


		//From ResourceMakerCallback
		void ResetCallback(int curvenum, ICurveCtl *pCCtl);
		void NewCurveCreatedCallback(int curvenum, ICurveCtl *pCCtl);
		void* GetInterface(ULONG id)
		{
			if(id == I_RESMAKER_INTERFACE)
				return (void *) (ResourceMakerCallback *) this;
			else
				return (void *) Control::GetInterface(id);
		}

		//Function Publishing method (Mixin Interface)
		//******************************
		BaseInterface* GetInterface(Interface_ID id) 
		{ 
			if (id == REACTOR_INTERFACE) 
				return (IReactor*)this; 
			else 
				return FPMixinInterface::GetInterface(id);
		} 
		//******************************
};

//Enumeration Proc
//Used to determine the proper reference object....
//******************************************
class MyEnumProc : public DependentEnumProc 
	{
      public :
      MyEnumProc(ReferenceTarget *m) { me = m; }
      virtual int proc(ReferenceMaker *rmaker); 
	  Tab<ReferenceMaker*> anims;
      ReferenceTarget *me;
	};


#endif // __REACTOR__H
