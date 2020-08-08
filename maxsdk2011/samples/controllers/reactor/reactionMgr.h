#include "reactor.h"
#include "tvnode.h"

#ifndef __REACTIONMGR__H
#define __REACTIONMGR__H

#define REACTION_MGR_CLASSID Class_ID(0x294a389c, 0x87906d7)
#define REACTIONSET_CLASSID Class_ID(0x55293bb4, 0x4ad13dad)

class MgrListSizeRestore;
class SlaveListSizeRestore;

extern ClassDesc* GetReactionManagerDesc();
extern ClassDesc* GetReactionSetDesc();
void RemoveSlaveFromScene(Reactor* r);

class ReactionSet : public ReferenceTarget
{
	friend class SlaveListSizeRestore;
	friend class ReactionManager;

	ReactionMaster* mMaster;
	Tab<Reactor*> mSlaves;

public:
	ReactionSet() { mMaster = NULL; mSlaves.Init(); }
	ReactionSet(ReactionMaster* master, Reactor* slave) 
	{ 
		mMaster = NULL;
		if (master != NULL)
			ReplaceReference(0, master); 

		mSlaves.Init(); 
		if (slave != NULL)
			AddSlave(slave);
	}
	~ReactionSet() 
	{ 
	}

	void DeleteThis() 
	{
		delete this;
	}		
	Class_ID ClassID() { return REACTIONSET_CLASSID; }  
	SClass_ID SuperClassID() { return REF_MAKER_CLASS_ID; } 
	void GetClassName(TSTR& s) {s = GetString(IDS_AF_REACTION_SET);}

	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);

	int NumRefs() { return mSlaves.Count()+1; }
	ReferenceTarget* GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID &partID, RefMessage message);
	BOOL IsRealDependency(ReferenceTarget* rtarg) 
	{ 
		if (rtarg == mMaster && mMaster->GetReference(0) != NULL) 
			return TRUE;
		return FALSE; 
	}

	ReactionMaster* GetReactionMaster() { return mMaster; }
	int SlaveCount() { return mSlaves.Count(); }
	Reactor* Slave(int i) { return mSlaves[i]; }
	void AddSlave(Reactor* r);
	void RemoveSlave(Reactor* r);
	void RemoveSlave(int i);

	void ReplaceMaster(ReferenceTarget* parent, int subAnim);

	int SlaveNumToRefNum(int i) { return i+1; }
};

// ---------  reaction manager  ---------

#define REACTION_MANAGER_INTERFACE  Interface_ID(0x100940fa, 0x43aa3a02)

class IReactionManager : public FPStaticInterface 
{
public:
	// function IDs 
	enum { openEditor,
		}; 

	virtual void OpenEditor()=0; // open the reaction manager dialog
};

class ReactionMgrImp : public IReactionManager
{
	void OpenEditor();

	DECLARE_DESCRIPTOR(ReactionMgrImp) 
	// dispatch map
	BEGIN_FUNCTION_MAP
	VFN_0(openEditor, OpenEditor); 
	END_FUNCTION_MAP 
};

class ReactionManager : public Control
{
	friend class MgrListSizeRestore;

	//Keep track of all masters
	//and a list of all slaves assigned to that master
	Tab<ReactionSet*> mReactionSets;
	int mSelected;
	int mSuspended;

public:
	
	ReactionManager();
	~ReactionManager();

	RefResult AutoDelete() { return REF_FAIL; } // cannot be deleted.
	Class_ID ClassID() { return REACTION_MGR_CLASSID; }  
	SClass_ID SuperClassID() { return REF_MAKER_CLASS_ID; } 
	void GetClassName(TSTR& s) {s = GetString(IDS_AF_REACTION_MANAGER);}

	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);

	int NumRefs() { return mReactionSets.Count(); }
	ReferenceTarget* GetReference(int i) { return (ReferenceTarget*)mReactionSets[i]; }
	void SetReference(int i, RefTargetHandle rtarg);
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID &partID, RefMessage message);
	BOOL IsRealDependency(ReferenceTarget* rtarg) 
	{ 
		ReactionSet* set = (ReactionSet*)rtarg;
		if (set->GetReactionMaster() != NULL && set->GetReactionMaster()->Owner() != NULL)
			return TRUE;
		return FALSE; 
	}
	BOOL BypassTreeView() {return TRUE;}

	void Copy(Control *){}
	BOOL IsLeaf() {return FALSE;}
	void GetValue(TimeValue,void *,Interval &,GetSetMethod) {}
	void SetValue(TimeValue,void *,int,GetSetMethod) {}

	ReactionMaster* AddReaction(ReferenceTarget* owner, int subAnimIndex, Reactor* slave = NULL);
	void RemoveSlave(ReferenceTarget* owner, int subAnimIndex, Reactor* slave);
	void RemoveReaction(ReferenceTarget* owner, int subAnimIndex, Reactor* slave = NULL);
	int ReactionCount() { return mReactionSets.Count(); }
	ReactionSet* GetReactionSet(int i) { return mReactionSets[i]; }
	void AddReactionSet(ReactionSet* set);
	void MaybeAddReactor(Reactor* slave);
	void RemoveTarget(ReferenceTarget* target);
	void RemoveReaction(int i);

	int GetSelected() { return mSelected; }
	void SetSelected(int index) { mSelected = index; }

private:
	void Suspend() { mSuspended++; }
	void Resume() { mSuspended--; }
	bool IsSuspended() { return mSuspended > 0; }
};

ReactionManager* GetReactionManager();

#define CHILDINDENT					2 //the amount to indent the name of a slaves

class ReactionListItem;
class StateListItem;

static enum
{
	kExpandCol = -1,			//pseudo column to contain the expand/collapse icon
	kNameCol = 0,

	kFromCol = 1,
	kToCol = 2,
	kCurveCol = 3,

	kValueCol = 1,
	kStrengthCol = 2,
	kInfluenceCol = 3,
	kFalloffCol = 4,
};

class ReactionDlg;

template <class T>
class ReactionDlgActionCB : public ActionCallback
{
	public:
		T* dlg;
		ReactionDlgActionCB(T* d) { dlg = d; }
		BOOL	ExecuteAction(int id);
};

ReactionDlg* GetReactionDlg();
void SetReactionDlg(ReactionDlg* dlg);

class ReactionDlg : public ReferenceMaker, public TimeChangeCallback {
	public:
		IObjParam *ip;
		HWND hWnd;
		bool valid, selectionValid, stateSelectionValid, reactionListValid;
		static HIMAGELIST mIcons;
		POINT* origPt;
		TimeValue origTime;
		float origValue;
		ICustEdit* floatWindow;
		ICurveCtl* iCCtrl;
		INodeTab selectedNodes;
		bool blockInvalidation;
		ReactionPickMode* mCurPickMode;
		RefTargetHandle reactionManager;

		float rListPos, sListPos;
        		
		WNDPROC reactionListWndProc;
		WNDPROC stateListWndProc;
		WNDPROC splitterWndProc;

		static const int kMaxTextLength;
		static const int kNumReactionColumns;
		static const int kNumStateColumns;

		ReactionDlgActionCB<ReactionDlg>	*reactionDlgActionCB;		// Actions handler 		
		
		ReactionDlg(IObjParam *ip, HWND hParent);
		~ReactionDlg();

		Class_ID ClassID() {return REACTIONDLG_CLASS_ID;}
		SClass_ID SuperClassID() {return REF_MAKER_CLASS_ID;}

		void TimeChanged(TimeValue t) {/*Invalidate();*/}
		static void PreSceneNodeDeleted(void *param, NotifyInfo *info);
		static void SelectionSetChanged(void *param, NotifyInfo *info);
		static void PreReset(void *param, NotifyInfo *info);
		static void PreOpen(void *param, NotifyInfo *info);
		static void PostOpen(void *param, NotifyInfo *info);


		void SetupUI(HWND hWnd);
		void PerformLayout();
		void SetupReactionList();
		void SetupStateList();
		void FillButton(HWND hWnd, int butID, int iconIndex, int disabledIconIndex, TSTR toolText, CustButType type);
		void BuildCurveCtrl();

		void Invalidate();
		void InvalidateReactionList()
		{
			reactionListValid = false;
			selectionValid = false;
			InvalidateRect(GetDlgItem(hWnd, IDC_REACTION_LIST), NULL, FALSE);
		}
		void InvalidateSelection()
		{
			selectionValid = false;
			InvalidateRect(hWnd, NULL, FALSE);
		}

		void InvalidateStateSelection()
		{
			stateSelectionValid = false;
			InvalidateRect(hWnd, NULL, FALSE);
		}

		void ClearCurves();

		void Update();
		void UpdateReactionList();
		void UpdateStateList();
		void UpdateCurveControl();
		void UpdateButtons();
		void UpdateStateButtons();
		void UpdateEditStates();
		void UpdateCreateStates();
		void SelectionChanged();
		void StateSelectionChanged(); 

		void Change(BOOL redraw=FALSE);

		bool ToggleReactionExpansionState(int rowNum);
		bool AddSlaveRow(HWND hList, ReactionListItem* listItem, int itemID);
		bool InsertListItem(ReactionListItem *pItem, int after);
		void InsertSlavesInList(ReactionSet *master, int after);
		void RemoveSlavesFromList(ReactionSet *master, int after);

		bool ToggleStateExpansionState(int rowNum);
		bool AddSlaveStateRow(HWND hList, StateListItem* listItem, int itemID);
		bool InsertListItem(StateListItem *pItem, int after);
		void InsertStatesInList(MasterState *state, ReactionSet* owner, int after);
		void RemoveStatesFromList(MasterState *state, int after);

		ReactionListItem* GetReactionDataAt(int where);
		StateListItem* GetStateDataAt(int where);
		ReactionListItem* GetSelectedMaster();
		Tab<ReactionListItem*> GetSelectedSlaves();
		Tab<StateListItem*> GetSelectedStates();
		Tab<StateListItem*> GetSelectedMasterStates();
		Tab<StateListItem*> GetSelectedSlaveStates();

		BOOL IsMasterStateSelected();
		BOOL IsSlaveStateSelected();

		POINT HitTest(POINT & p, UINT id);
		BOOL OnLMouseButtonDown(HWND hDlg, WPARAM wParam, POINT lParam, UINT id);
		void WMCommand(int id, int notify, HWND hCtrl);
		void SpinnerChange(int id, POINT p, int nRow, int nCol);
		void SpinnerEnd(int id, BOOL cancel, int nRow, int nCol);
		void SetMinInfluence();
		void SetMaxInfluence();

		void InvokeRightClickMenu();

		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	         PartID& partID,  RefMessage message);
		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {if (i == 0) return reactionManager; return NULL; }  //return iCCtrl;
		void SetReference(int i, RefTargetHandle rtarg) { reactionManager = rtarg; } //if (i == 1) iCCtrl = (ICurveCtl*)rtarg;
		BOOL IsRealDependency(ReferenceTarget* rtarg) { return FALSE; }
	};

class ReactionListItem 
{
public:
	enum ItemType { kReactionSet, kSlave, };

private:
	void* mItem;
	ItemType mType;
	bool mIsExpanded;
	ReferenceTarget* owner;
	int index;

public:
	ReactionListItem(ItemType type, void* obj, ReferenceTarget* o, int id)
	{
		mItem = obj;
		mType = type;
		mIsExpanded = true;
		owner = o;
		index = id;
	}

	ReactionSet* GetReactionSet() 
	{
		if (mType == kReactionSet)
			return (ReactionSet*) mItem;
		return NULL;
	}

	Reactor* Slave() 
	{
		if (mType == kSlave)
			return (Reactor*) mItem;
		return NULL;
	}

	TSTR GetName();

	ReferenceTarget* GetOwner() { return owner; }
	int GetIndex() { return index; }

	bool GetRange(TimeValue &t, bool start);
	void SetRange(TimeValue t, bool start);
	void GetValue(void* val);

	void ToggleUseCurve()
	{
		Reactor* slave = Slave();
		if (slave != NULL)
		{
			slave->useCurve(!slave->useCurve());
		}
	}

	BOOL IsUsingCurve()
	{
		Reactor* slave = Slave();
		if (slave != NULL)
			return slave->useCurve();
		return FALSE;
	}

	bool IsExpandable()
	{
		ReactionSet* set = GetReactionSet();
		if (set != NULL)
		{
			for(int j = 0; j < set->SlaveCount(); j++)
			{
				Reactor* r = set->Slave(j);
				if (!r)
					continue;
				MyEnumProc dep(r); 
				r->DoEnumDependents(&dep);

				for(int x=0; x<dep.anims.Count(); x++)
				{
					for(int i=0; i<dep.anims[x]->NumSubs(); i++)
					{
						Animatable* n = dep.anims[x]->SubAnim(i);
						if (n == r)
						{
							return true; // found one in the scene
						}
					}
				}
			}
		}
		return false;
	}

	bool IsExpanded() { return mIsExpanded; }
	void Expand(bool expanded) { if (expanded) mIsExpanded = true; else mIsExpanded = false; }

};

class StateListItem 
{
public:
	enum ItemType { kMasterState, kSlaveState, };

private:
	void* mItem;
	ReferenceTarget* owner;
	ItemType mType;
	bool mIsExpanded;
	int index;

public:
	StateListItem(ItemType type, void* obj, ReferenceTarget* o, int id)
	{
		mItem = obj;
		mType = type;
		mIsExpanded = true;
		owner = o;
		index = id;
	}

	MasterState* GetMasterState() 
	{
		if (mType == kMasterState)
			return (MasterState*) mItem;
		return NULL;
	}

	SlaveState* GetSlaveState() 
	{
		if (mType == kSlaveState)
			return (SlaveState*) mItem;
		return NULL;
	}

	ReferenceTarget* GetOwner() { return owner; }
	int GetIndex() { return index; }

	TSTR GetName();
	int GetType();
	void GetValue(void* val);
	void SetValue(float val);
	float GetInfluence();
	void SetInfluence(float val);

	bool IsExpandable()
	{
		MasterState* state = GetMasterState();
		// find any slaves that use this master state
		for (int j = 0; j < GetReactionManager()->ReactionCount(); j++)
		{
			ReactionSet* set = GetReactionManager()->GetReactionSet(j);
			for(int k = 0; k < set->SlaveCount(); k++)
			{
				Reactor* r = set->Slave(k);
				if (r) {
					for(int x = 0; x < r->slaveStates.Count(); x++)
					{
						if (r->getMasterState(x) == state )
						{
							return true;
						}
					}
				}
			}
		}
		return false;
	}

	bool IsExpanded() { return mIsExpanded; }
	void Expand(bool expanded) { if (expanded) mIsExpanded = true; else mIsExpanded = false; }

};

class ReactionMasterPickMode : public ReactionPickMode
{
	ICustButton* but;
public:
	ReactionMasterPickMode(ICustButton* b) {but = b;}
	~ReactionMasterPickMode() {	if (but) ReleaseICustButton(but); }
	TSTR GetName() { return GetString(IDS_ADD_MASTER); }
	bool LoopTest(ReferenceTarget* anim) { return true; }
	void NodePicked(ReferenceTarget* anim);
	bool IncludeRoots() { return true; }
	bool MultipleChoice() { return true; }
	void PostPick() 
	{
		//if(GetReactionDlg() != NULL)
		//	GetReactionDlg()->ip->GetActionManager()->FindTable(kReactionMgrActions)->GetAction(IDC_ADD_SLAVES)->ExecuteAction();
	}

	void EnterMode(IObjParam *ip) {	ReactionPickMode::EnterMode(ip); if (but) but->SetCheck(TRUE);}
	void ExitMode(IObjParam *ip) 
	{
		ReactionPickMode::ExitMode(ip);
		if (but) 
			but->SetCheck(FALSE);
	}
};

class ReplaceMasterPickMode : public ReactionPickMode
{
	ICustButton* but;
	ReactionSet* mSet;
public:
	ReplaceMasterPickMode(ReactionSet* set, ICustButton* b) {mSet = set; but = b;}
	~ReplaceMasterPickMode() {	if (but) ReleaseICustButton(but); }
	TSTR GetName() { return GetString(IDS_REPLACE_MASTER); }
	bool LoopTest(ReferenceTarget* anim);
	void NodePicked(ReferenceTarget* anim);
	bool IncludeRoots() { return true; }

	void EnterMode(IObjParam *ip) {	ReactionPickMode::EnterMode(ip); if (but) but->SetCheck(TRUE);}
	void ExitMode(IObjParam *ip) 
	{
		ReactionPickMode::ExitMode(ip);
		if (but) 
			but->SetCheck(FALSE);
	}
};


class ReactionSlavePickMode : public ReactionPickMode
{
	ReactionSet* mMaster;
	Tab<int> mMasterIDs;
	ICustButton* but;
public:
	ReactionSlavePickMode(ReactionSet* m, Tab<int> masterIDs, ICustButton* b) 
		{ mMaster = m; mMasterIDs = masterIDs; but = b; }
	~ReactionSlavePickMode() {	if (but) ReleaseICustButton(but); }

	TSTR GetName() { return GetString(IDS_ADD_SLAVE); }
	bool LoopTest(ReferenceTarget* anim);
	void NodePicked(ReferenceTarget* anim);
	bool IncludeRoots() { return false; }
	bool MultipleChoice() { return true; }
	bool FilterAnimatable(Animatable *anim);

	void EnterMode(IObjParam *ip) {	ReactionPickMode::EnterMode(ip); if (but) but->SetCheck(TRUE);}
	void ExitMode(IObjParam *ip)
	{
		ReactionPickMode::ExitMode(ip);
		if (but) 
			but->SetCheck(FALSE);
	}
};

class SlaveListSizeRestore : public RestoreObj {
	public:
		ReactionSet *mMaster;	
		bool mIncrease;
		int mIndex;

		SlaveListSizeRestore(ReactionSet *m, bool increase, int index) {
			mMaster = m;
			mIncrease = increase;
			mIndex = index;
			}
		void Restore(int isUndo) {			
			if (mIncrease)
				mMaster->mSlaves.Resize(mMaster->mSlaves.Count()-1);
			else {
				Reactor* dummyEntry = NULL;
				mMaster->mSlaves.Insert(mIndex, 1, &dummyEntry);
				}
			mMaster->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
			}
		void Redo() {
			if (mIncrease) {
				mMaster->mSlaves.SetCount(mMaster->mSlaves.Count()+1);
				mMaster->mSlaves[mMaster->mSlaves.Count()-1] = NULL;
				}
			else 
				mMaster->mSlaves.Delete(mIndex, 1);
			mMaster->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
			}
		TSTR Description() {return TSTR(_T("Slave Count"));}
	};


class MgrListSizeRestore : public RestoreObj {
	public:
		ReactionManager *mMgr;	
		bool mIncrease;
		int mIndex;

		MgrListSizeRestore(ReactionManager *m, bool increase, int index) {
			mMgr = m;
			mIncrease = increase;
			mIndex = index;
			}
		void Restore(int isUndo) {			
			if (mIncrease)
				mMgr->mReactionSets.SetCount(mMgr->mReactionSets.Count()-1);
			else {
				ReactionSet* dummyEntry = NULL;
				mMgr->mReactionSets.Insert(mIndex, 1, &dummyEntry);
				}
			mMgr->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
			}
		void Redo() {
			if (mIncrease) {
				mMgr->mReactionSets.SetCount(mMgr->mReactionSets.Count()+1);
				mMgr->mReactionSets[mMgr->mReactionSets.Count()-1] = NULL;
				}
			else 
				mMgr->mReactionSets.Delete(mIndex, 1);
			mMgr->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
			}
		TSTR Description() {return TSTR(_T("Manager Size"));}
	};


#endif //__REACTIONMGR__H
