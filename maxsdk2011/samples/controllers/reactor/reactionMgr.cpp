#include "reactionMgr.h"
#include "custcont.h"

static ReactionManager theReactionMgr;
// Reaction Manager interface instance and descriptor
ReactionMgrImp theReactionMgrImp(REACTION_MANAGER_INTERFACE, _T("reactionMgr"), 0, NULL, FP_CORE, 
	IReactionManager::openEditor, _T("openEditor"), 0, TYPE_VOID, 0, 0, 
	end );

ReactionManager* GetReactionManager() { return &theReactionMgr; }
void AddReactionMgrToScene();

//**********************************************************************
// ReactionManager implementation
//**********************************************************************
void PostReset(void *param, NotifyInfo *info)
{
	AddReactionMgrToScene();
}

void PreReset(void *param, NotifyInfo *info)
{
	DbgAssert(info);

	// Do not empty the reaction manager if we're just loading a render preset file
	if (info != NULL && info->intcode == NOTIFY_FILE_PRE_OPEN && info->callParam) {
		FileIOType type = *(static_cast<FileIOType*>(info->callParam));
		if (type == IOTYPE_RENDER_PRESETS)
			return;
	}

	for(int i = GetReactionManager()->NumRefs()-1; i >= 0; i--)
		GetReactionManager()->RemoveReaction(i);
}

ReactionManager::ReactionManager() 
{ 
	mSelected = -1; 
	mSuspended = 0;
	RegisterNotification(PreReset, this, NOTIFY_SYSTEM_PRE_RESET);
	RegisterNotification(PreReset, this, NOTIFY_FILE_PRE_OPEN);
	RegisterNotification(PreReset, this, NOTIFY_SYSTEM_PRE_NEW);

	RegisterNotification(PreReset, this, NOTIFY_SYSTEM_SHUTDOWN);

	RegisterNotification(PostReset, this, NOTIFY_SYSTEM_POST_RESET);
	RegisterNotification(PostReset, this, NOTIFY_FILE_POST_OPEN);
	RegisterNotification(PostReset, this, NOTIFY_SYSTEM_POST_NEW);
}

ReactionManager::~ReactionManager() 
	{ 
	}

void ReactionManager::SetReference(int i, RefTargetHandle rtarg)
{
	if (i >= mReactionSets.Count())
	{
		if (rtarg != NULL)
		{
			if (theHold.Holding()) {
				theHold.Put(new MgrListSizeRestore(GetReactionManager(), true, i));
				}
			ReactionSet* m = (ReactionSet*)rtarg;
			mReactionSets.Append(1, &m);
		}
	}
	else
	{
		mReactionSets[i] = (ReactionSet*)rtarg;
	}
}

RefResult ReactionManager::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID &partID, RefMessage message)
{
	switch (message) {
		case REFMSG_TARGET_DELETED:
			RemoveTarget(hTarget);
			break;
		}
	return REF_SUCCEED;
}

void ReactionManager::RemoveTarget(ReferenceTarget* target)
{
	for(int i = mReactionSets.Count()-1; i>=0; i--)
		if (mReactionSets[i] == target)
		{
			RemoveReaction(i);
			break;
		}
}

ReactionMaster* ReactionManager::AddReaction(ReferenceTarget* owner, int subAnimIndex, Reactor* slave)
{
	if (!IsSuspended())
	{
      int i;
		for(i = 0; i < mReactionSets.Count(); i++)
		{
			if (!mReactionSets[i])
				continue;
			ReactionMaster* master = mReactionSets[i]->GetReactionMaster();
			if ((owner == NULL && master == NULL) || 
				(master != NULL && master->Owner() != NULL && // don’t add it to a set with a valid master and a null client.  This will soon be deleted.
				master->Owner() == owner && master->SubIndex() == subAnimIndex))
			{
				if (slave != NULL)
				{
					mReactionSets[i]->AddSlave(slave);
					mSelected = i;
				}
				return master;
			}
		}
		ReactionMaster* master = NULL;
		if (owner != NULL)
			master = new ReactionMaster(owner, subAnimIndex);

		if (theHold.Holding()) {
			theHold.Put(new MgrListSizeRestore(GetReactionManager(), true, i));
		}
		ReactionSet* null_ptr = NULL;
		mReactionSets.Append(1, &null_ptr);

		ReactionSet* rs = new ReactionSet(master, slave);
		ReplaceReference(i, (ReferenceTarget*)rs);
		mSelected = i;
		NotifyDependents(FOREVER, PART_ALL, REFMSG_SUBANIM_STRUCTURE_CHANGED);
		return master;
	}
	return NULL;
}

void ReactionManager::AddReactionSet(ReactionSet* set)
{
	int count = mReactionSets.Count();
	for(int i = 0; i < count; i++)
		if (mReactionSets[i] == set)
			return;

	if (theHold.Holding()) {
		theHold.Put(new MgrListSizeRestore(GetReactionManager(), true, count));
	}
	ReactionSet* null_ptr = NULL;
	mReactionSets.Append(1, &null_ptr);

	ReplaceReference(count, set);
	mSelected = count;
	NotifyDependents(FOREVER, PART_ALL, REFMSG_SUBANIM_STRUCTURE_CHANGED);
}

void ReactionManager::MaybeAddReactor(Reactor* slave)
{
	for(int i = 0; i < mReactionSets.Count(); i++)
	{
		ReactionSet* set = GetReactionSet(i);
		if (set->GetReactionMaster() == slave->GetMaster())
		{
			for(int x = 0; x < set->SlaveCount(); x++)
			{
				if (set->Slave(x) == slave)
				{
					mSelected = i;
					return; //already added
				}
			}
			// if a set has this master, but not this slave add the slave.
			// this can happen with R7 beta files.
			set->AddSlave(slave); 
			return;
		}
	}

	// if you made it this far you are probably loading an PreR7 or R7 beta file.
	ReactionMaster* slavesMaster = slave->GetMaster();
	if (slavesMaster != NULL)
	{
		ReactionSet* rs = new ReactionSet(slavesMaster, slave);
		if (rs)
			AddReactionSet(rs);
	}
	else
		GetReactionManager()->AddReaction(NULL, 0, slave);
}

void ReactionManager::RemoveSlave(ReferenceTarget* owner, int subAnimIndex, Reactor* slave)
{
	for(int i = 0; i < mReactionSets.Count(); i++)
	{
		ReactionMaster* master = mReactionSets[i]->GetReactionMaster();
		if ((master == NULL && owner == NULL) ||
			(master != NULL && master->Owner() == owner && master->SubIndex() == subAnimIndex))
		{
			if (slave != NULL)
				mReactionSets[i]->RemoveSlave(slave);
			return;
		}
	}
}

void ReactionManager::RemoveReaction(ReferenceTarget* owner, int subAnimIndex, Reactor* slave)
{
	for(int i = 0; i < mReactionSets.Count(); i++)
	{
		ReactionMaster* master = mReactionSets[i]->GetReactionMaster();
		if ((master == NULL && owner == NULL) ||
			(master != NULL && master->Owner() == owner && master->SubIndex() == subAnimIndex))
		{
			if (slave != NULL)
				mReactionSets[i]->RemoveSlave(slave);
			if (mReactionSets[i]->SlaveCount() == 0)
			{
				RemoveReaction(i);
			}
			return;
		}
	}
}

void ReactionManager::RemoveReaction(int i)
{
	if (i >= mReactionSets.Count())
		return;

	Suspend();
	DeleteReference(i);
	if (theHold.Holding())
		theHold.Put(new MgrListSizeRestore(GetReactionManager(), false, i));
	// Deleting the reference above may have triggered other reaction set deletions
	if (i < mReactionSets.Count())
		mReactionSets.Delete(i, 1);
	if (mSelected >= i)
		mSelected--;
	NotifyDependents(FOREVER, PART_ALL, REFMSG_SUBANIM_STRUCTURE_CHANGED);
	Resume();
}

IOResult ReactionManager::Save(ISave *isave)
{		
	return IO_OK;
}

IOResult ReactionManager::Load(ILoad *iload)
{
	return IO_OK;
}

void ReactionMgrImp::OpenEditor()
{
	// open reaction manager dialog on selected objects
	if (GetReactionDlg() == NULL)
	{
		SetReactionDlg(new ReactionDlg((IObjParam*)GetCOREInterface(), GetCUIFrameMgr()->GetAppHWnd()));
		ShowWindow(GetReactionDlg()->hWnd, SW_SHOWNORMAL);
	}
	else if (IsIconic(GetReactionDlg()->hWnd))
		ShowWindow(GetReactionDlg()->hWnd, SW_SHOWNORMAL);
	else
		DestroyWindow(GetReactionDlg()->hWnd);
}

//**********************************************************************
// ReactionSet implementation
//**********************************************************************

ReferenceTarget* ReactionSet::GetReference(int i) 
{ 
	if (i == 0)
		return mMaster;
	else return mSlaves[i-1];
}

void ReactionSet::SetReference(int i, RefTargetHandle rtarg)
{
	if (i == 0)
		mMaster = (ReactionMaster*)rtarg;
	else 
	{
		i--;
		if (i >= mSlaves.Count())
		{
			if (rtarg != NULL)
			{
				if (theHold.Holding()) {
					theHold.Put(new SlaveListSizeRestore(this, true, i));
					}
				Reactor* r = (Reactor*)rtarg;
				mSlaves.Append(1, &r);
			}
		}
		else
		{
			mSlaves[i] = (Reactor*)rtarg;
		}
	}
}

RefResult ReactionSet::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID &partID, RefMessage message)
{
	switch (message) {
		case REFMSG_TARGET_DELETED:
			{
				if (hTarget == this->mMaster)
				{
					mMaster = NULL;
					return REF_AUTO_DELETE;
				}
				else
				{
					for(int i = mSlaves.Count()-1; i >= 0 ; i--)
						if (hTarget == mSlaves[i])
						{
							if (theHold.Holding())
								theHold.Put(new SlaveListSizeRestore(this, false, i));
							mSlaves.Delete(i, 1);
							NotifyDependents(FOREVER, PART_ALL, REFMSG_SUBANIM_STRUCTURE_CHANGED);
							break;
						}
				}
			}

			break;
		}
	return REF_SUCCEED;
}

void ReactionSet::AddSlave(Reactor* r)
{
    int i;
	for(i = 0; i < mSlaves.Count(); i++)
	{
		if (mSlaves[i] == r)
			return;
	}

	if (theHold.Holding()) {
		theHold.Put(new SlaveListSizeRestore(this, true, i));
	}
	Reactor* null_ptr = NULL;
	mSlaves.Append(1, &null_ptr);

	ReplaceReference( SlaveNumToRefNum(i), r);
	NotifyDependents(FOREVER, PART_ALL, REFMSG_SUBANIM_STRUCTURE_CHANGED);
}

void ReactionSet::RemoveSlave(Reactor* r)
{
	for(int i = 0; i < mSlaves.Count(); i++)
	{
		if (mSlaves[i] == r)
		{
			RemoveSlave(i);
			return;
		}
	}
}

void ReactionSet::RemoveSlave(int i)
{
	DeleteReference(SlaveNumToRefNum(i));
	mSlaves.Delete(i, 1);
	if (theHold.Holding()) {
		theHold.Put(new SlaveListSizeRestore(this, false, i));
		}
	NotifyDependents(FOREVER, PART_ALL, REFMSG_SUBANIM_STRUCTURE_CHANGED);
}

void ReactionSet::ReplaceMaster(ReferenceTarget* parent, int subAnim)
{
	int defaultID = -1;
	bool aborted = false;

	for(int i = 0; i < GetReactionManager()->ReactionCount(); i ++)
	{
		ReactionSet* set = GetReactionManager()->GetReactionSet(i);
		if (set != this)
		{
			ReactionMaster* master = set->GetReactionMaster();
			if (master != NULL && master->Owner() == parent && master->subnum == subAnim)
			{
				aborted = true;
				break;
			}
		}
	}

	if (!aborted)
	{
		ReactionMaster* master = new ReactionMaster(parent, subAnim);
		//master->AddState(NULL, defaultID, true, GetCOREInterface()->GetTime());
		ReplaceReference(0, master);
	}

	if (SlaveCount())
	{
		for(int i = SlaveCount()-1; i >=0 ; i--)
		{
			Reactor* r = Slave(i);
			BOOL succeed = false;
			if (r) {
				if (parent->SuperClassID()==BASENODE_CLASS_ID)
					succeed = r->reactTo(parent, GetCOREInterface()->GetTime(), false);
				else
					succeed = r->reactTo((ReferenceTarget*)parent->SubAnim(subAnim), GetCOREInterface()->GetTime(), false);

				if (succeed)
					r->CreateReactionAndReturn(true, NULL, GetCOREInterface()->GetTime(), defaultID);
				else
					return;
			}
		}
	NotifyDependents(FOREVER, PART_ALL, REFMSG_SUBANIM_STRUCTURE_CHANGED);
	}
	else
	{
		NotifyDependents(FOREVER, PART_ALL, REFMSG_SUBANIM_STRUCTURE_CHANGED);
		MaybeAutoDelete();
	}
}

class ReactionSetPostLoadCallback : public PostLoadCallback {
	ReactionSet* mSet;
	public:
		ReactionSetPostLoadCallback(ReactionSet* s) { mSet = s; }
		int Priority() { return 3; }  
		// CleanUpReactionSet removes null slaves from the slaves list
		int CleanUpReactionSet(ReactionSet* in_set) {
			int nbCleaned = 0;
			if (in_set) {
				for (int i = in_set->SlaveCount() - 1; i >= 0; i--) {
					if (!in_set->Slave(i)) {
						in_set->RemoveSlave(i);
						nbCleaned++;
					}
				}
			}
			return nbCleaned;
		}
		void proc(ILoad *iload)
		{
			CleanUpReactionSet(mSet);
			GetReactionManager()->AddReactionSet(mSet);
		}
	};

#define SLAVE_COUNT_CHUNK		0x5001

IOResult ReactionSet::Save(ISave *isave)
{		
	ULONG 	nb;

	isave->BeginChunk(SLAVE_COUNT_CHUNK);
	int count = mSlaves.Count();
	isave->Write(&count, sizeof(int), &nb);
 	isave->EndChunk();

	return IO_OK;
}

IOResult ReactionSet::Load(ILoad *iload)
	{
	ULONG 	nb;
	int id;

	while (iload->OpenChunk()==IO_OK) {
		switch (id = iload->CurChunkID()) {
			case SLAVE_COUNT_CHUNK:
				{
				int count = 0;
				iload->Read(&count, sizeof(int), &nb);
				mSlaves.SetCount(count);
				for(int i = 0; i < count; i++)
					mSlaves[i] = NULL;

				break;
				}
		}
		iload->CloseChunk();
	}

	ReactionSetPostLoadCallback *plc = new ReactionSetPostLoadCallback(this);
	iload->RegisterPostLoadCallback(plc);

	return IO_OK;
}

void AddReactionMgrToScene()
{
	Interface *ip = GetCOREInterface();
	if (ip == NULL)
		return;

	ITrackViewNode* tvRoot = ip->GetTrackViewRootNode();
	if (tvRoot->FindItem(REACTION_MGR_CLASSID) < 0)
		tvRoot->AddController(GetReactionManager(), _T("Reaction Manager"), REACTION_MGR_CLASSID);
}

///*****************************************************************
/// Class descriptors
///*****************************************************************

class ReactionManagerClassDesc:public ClassDesc2 {
	public:
	ReactionManagerClassDesc(){ AddReactionMgrToScene(); }

	int 			IsPublic() { return FALSE; }
	void *			Create(BOOL loading) 
					{ 
						return GetReactionManager(); 
					}
	const TCHAR *	ClassName() { return GetString(IDS_AF_REACTION_MANAGER); }
	SClass_ID		SuperClassID() { return REF_MAKER_CLASS_ID; }
	Class_ID		ClassID() { return REACTION_MGR_CLASSID; }
	const TCHAR* 	Category() { return _T("");  }

	const TCHAR*	InternalName() { return _T("ReactionManager"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
	void			ResetClassParams(BOOL fileReset) 
		{ 
		}

	int             NumActionTables() { return 1; }
	ActionTable*	GetActionTable(int i) { return GetActions(); }

	};

class ReactionSetClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return FALSE; }
	void *			Create(BOOL loading) 
					{ 
						return new ReactionSet(); 
					}
	const TCHAR *	ClassName() { return GetString(IDS_AF_REACTION_SET); }
	SClass_ID		SuperClassID() { return REF_MAKER_CLASS_ID; }
	Class_ID		ClassID() { return REACTIONSET_CLASSID; }
	const TCHAR* 	Category() { return _T("");  }

	const TCHAR*	InternalName() { return _T("ReactionSet"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
	};

static ReactionManagerClassDesc reactionMgrCD;
static ReactionSetClassDesc reactionSetCD;

ClassDesc* GetReactionManagerDesc() { return &reactionMgrCD; }
ClassDesc* GetReactionSetDesc() { return &reactionSetCD; }
