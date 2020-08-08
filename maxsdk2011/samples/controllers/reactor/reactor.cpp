/**********************************************************************
 *<
	FILE: reactor.cpp

	DESCRIPTION: A Controller plugin that reacts to changes in other controllers

	CREATED BY: Adam Felt

	HISTORY: 

 *>	Copyright (c) 1998, All Rights Reserved.
***********************************************************************/
//-----------------------------------------------------------------------------
#include "reactor.h"
#include "restoreObjs.h"
#include "reactionMgr.h"

//-----------------------------------------------------------------------
float Distance(Point3 p1, Point3 p2)
{
	p1 = p2-p1;
	return (float)sqrt((p1.x*p1.x)+(p1.y*p1.y)+(p1.z*p1.z));
}

int MyEnumProc::proc(ReferenceMaker *rmaker) 
{ 
	if (rmaker == me) return DEP_ENUM_CONTINUE;
	anims.Append(1, &rmaker);
	return DEP_ENUM_SKIP;
}

//-----------------------------------------------------------------------------
		
Reactor::Reactor() 
{
	Init(FALSE);
}

Reactor::Reactor(int t, BOOL loading) 
{
	Init(loading);
	type = t;
	if (loading) { isCurveControlled = 0; }  
}

void Reactor::Init(BOOL loading)
{
	mNumRefs = kNUM_REFS;
	mClient = NULL;
	master = NULL;
	type = 0;
	range.Set(GetAnimStart(), GetAnimEnd());
	count = 0;
	selected = 0;
	editing = FALSE;
	curpval = Point3(0.0f, 0.0f, 0.0f);
	curfval = 0.0f;
	curqval.Identity();
	ivalid.SetEmpty();
	blockGetNodeName = FALSE;
	isCurveControlled = 1;
	iCCtrl = NULL;
	flags = 0;
	upVector = Point3(0,0,-1);
	pickReactToMode = NULL;
	createMode = FALSE;
	curvesValid = false;
	if (!loading && GetCOREInterface8()->GetControllerOverrideRangeDefault())
		SetAFlag(A_ORT_DISABLED);
}


Reactor::~Reactor()
{
	deleteAllVars();
	DeleteAllRefsFromMe();
}

void Reactor::deleteAllVars()
{
	slaveStates.SetCount(0);
	count = 0;
	selected = 0;
}

Reactor& Reactor::operator=(const Reactor& from)
{
	int i;
	type = from.type;
	
	slaveStates.SetCount(from.slaveStates.Count());
	for(i = 0;i < from.slaveStates.Count();i++)
	{
		memset(&slaveStates[i], 0, sizeof(SlaveState));
		slaveStates[i] = from.slaveStates[i];
	}
	
	count = from.count;	
	selected = from.selected;
	editing = from.editing;
	createMode = from.createMode;

	isCurveControlled = from.isCurveControlled;

	curfval = from.curfval;
	curpval = from.curpval;
	curqval = from.curqval;

	ivalid = from.ivalid;
	range = from.range;

	upVector = from.upVector;
	mLocked = from.mLocked;
	return *this;
}

int Reactor::NumSubs() 
	{
	return 0;
	}

void Reactor::EditTimeRange(Interval in_range, DWORD in_flags)
{
	if(GetLocked()==false)
	{
		if(!(in_flags&EDITRANGE_LINKTOKEYS)){
			HoldRange();
			range = in_range;
			NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
	}
}

void Reactor::MapKeys(TimeMap *map,DWORD flags)
{
	if(GetLocked()==false)
	{
		if (flags&TRACK_MAPRANGE) {
			HoldRange();
			TimeValue t0 = map->map(range.Start());
			TimeValue t1 = map->map(range.End());
			range.Set(t0,t1);
			NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
	}
}

void Reactor::SetNumRefs(int newNumRefs)
{
	mNumRefs = newNumRefs;
}

int Reactor::NumRefs() 
{
	return mNumRefs;
}

RefTargetHandle Reactor::GetReference(int i) 
{
	switch (i)
	{
		case kREFID_MASTER:
			return master;
		case kREFID_CURVE_CTL:
			return iCCtrl;
		case kREFID_CLIENT_REMAP_PRE_R7: // required for loading pre-R7 files
			return mClient;
		default:
		{
			DbgAssert(false);
			return NULL;
		}
	}
}

void Reactor::SetReference(int i, RefTargetHandle rtarg) 
{
	switch (i)
	{
		case kREFID_MASTER:
			master = (ReactionMaster*)rtarg;
			break;
		case kREFID_CURVE_CTL:
			iCCtrl = (ICurveCtl*)rtarg;
			break;
		case kREFID_CLIENT_REMAP_PRE_R7: // required for loading pre-R7 files
			mClient = rtarg;
			break;
		default:
			DbgAssert(false);
			break;
	}
}

RefResult Reactor::NotifyRefChanged(
		Interval iv, 
		RefTargetHandle hTarg, 
		PartID& partID, 
		RefMessage msg) 
{
	switch (msg) {
		case REFMSG_CHANGE:
			if (hTarg == master && isCurveControlled)
				curvesValid = false;
			ivalid.SetEmpty();
			break;
		case REFMSG_GET_NODE_NAME:
		// RB 3/23/99: See comment at imp of getNodeName().
		if (blockGetNodeName) return REF_STOP;
		break;
		case REFMSG_TARGET_DELETED:
			if (hTarg==master)  //If it's a special case reference delete everything
			{
				HoldParams();
				count = 0;
				slaveStates.ZeroCount();
				setSelected(-1);
				RebuildFloatCurves();
				master = NULL;
				GetReactionManager()->AddReaction(NULL, 0, this);
				NotifyDependents(FOREVER,PART_ALL,REFMSG_REACTION_COUNT_CHANGED);
			}
			break;
		case REFMSG_REF_DELETED:
			break;
		case REFMSG_GET_CONTROL_DIM: 
			break;
		case REFMSG_NODE_NAMECHANGE:
			NotifyDependents(FOREVER,PART_ALL,REFMSG_REACTTO_OBJ_NAME_CHANGED);
			break;
		}
	return REF_SUCCEED;
}
 
void Reactor::Copy(Control *from)
{
	if(GetLocked()==false)
	{
		Point3 pointval;
		float floatval, f;
		Quat quatval;
		ScaleValue sv;
		
		switch (type)
		{
			case REACTORPOS:
			case REACTORP3:
				from->GetValue(GetCOREInterface()->GetTime(), &pointval, ivalid);
				curpval = pointval;
				break;
			case REACTORROT:
				from->GetValue(GetCOREInterface()->GetTime(), &quatval, ivalid);
				curqval = quatval;
				break;
			case REACTORFLOAT:
				from->GetValue(GetCOREInterface()->GetTime(), &floatval, ivalid);
				f = floatval;
				curfval = f;
				break;
			case REACTORSCALE:
				from->GetValue(GetCOREInterface()->GetTime(), &sv, ivalid);
				curpval = sv.s;
				break;
			default: break;
		}
		GetReactionManager()->AddReaction(NULL, 0, this);
	}
}

class SuperDuperFixAllPLCB : public PostLoadCallback
{
public:

	Reactor* cont;
	SuperDuperFixAllPLCB(Reactor* r) { cont = r; }
	virtual int Priority() { return 4; }  
	virtual void proc(ILoad *iload)
	{
		// fix up corrupt scene.
		bool changed = false;
		for(int i = cont->slaveStates.Count() - 1; i >= 0 ; i--)
		{
			if (cont->getMasterState(i) == NULL)
			{
				DbgAssert(0);
				cont->slaveStates.Delete(i, 1);
				changed = true;
			}
		}
		if (changed)
		{
			cont->RebuildFloatCurves();
		}
		cont->count = cont->slaveStates.Count();
		if ( cont->selected >= cont->count) 
			cont->selected = cont->count - 1;

		if (!cont->iCCtrl) 
			cont->BuildCurves();

#ifdef FIX_OWNERLESS_MASTERS_ON_LOAD
		// make sure it wasn't saved while pointing to an ownerless pblock or something.
		if (cont->GetMaster() != NULL && cont->GetMaster()->client != NULL)
			cont->GetMaster()->client->MaybeAutoDelete();
#endif
		// This is here to fix up preR7 and R7 beta files.  
		GetReactionManager()->MaybeAddReactor(cont);

		delete this;
	}
};

// Prior to Max 7, reactor was directly referencing its "client" controller.
// During Max 7 this has been changed so that Reactor references the ReactionMaster,
// which in turn reference the "client". This post-load call-back ensures that the
// reference a reactor instance held to its client are transfered properly to the
// reaction master.
class TransferReactorReferencePLCB : public PostLoadCallback, public MaxSDK::Util::Noncopyable
{
public:
	TransferReactorReferencePLCB(Reactor& r) : mReactor(r) 
	{ 
		mReactor.SetNumRefs(Reactor::kNUM_REFS_PRE_R7);
	} 
	virtual int Priority() { 
		// I think it's better to run this plcb before SuperDuperFixAllPLCB
		return 3; 
	}   
	virtual void proc(ILoad *iload)
	{
		DbgAssert(mReactor.NumRefs() == Reactor::kNUM_REFS_PRE_R7);
		DbgAssert(mReactor.GetReference(Reactor::kREFID_CLIENT_REMAP_PRE_R7) != NULL);

		RefTargetHandle master = mReactor.GetReference(Reactor::kREFID_MASTER);
		DbgAssert(master != NULL);
		master->ReplaceReference(0, mReactor.GetReference(Reactor::kREFID_CLIENT_REMAP_PRE_R7));
		mReactor.DeleteReference(Reactor::kREFID_CLIENT_REMAP_PRE_R7);
		mReactor.SetNumRefs(Reactor::kNUM_REFS);

		delete this;
	}

private:
	Reactor& mReactor;
};

#define REACTOR_VAR_RQUAT		0x5000
#define REACTOR_VAR_RVECTOR		0x5001
#define REACTOR_RTYPE_CHUNK		0x5002
#define REACTOR_VAR_STRENGTH	0x5003
#define REACTOR_VAR_FALLOFF		0x5004
#define REACTOR_ISBIPED_CHUNK	0x5005
#define REACTOR_UPVECTOR_CHUNK	0x5006
#define REACTOR_CURFVAL_CHUNK	0x5007
#define REACTOR_CURPVAL_CHUNK	0x5008
#define REACTOR_CURQVAL_CHUNK	0x5009
#define REACTOR_RANGE_CHUNK		0x6001
#define REACTOR_VREFS_REFCT		0x6002
#define REACTOR_VREFS_SUBNUM	0x6003
#define REACTOR_SVAR_TABSIZE	0x6004
#define REACTOR_VVAR_TABSIZE	0x6005
#define REACTOR_VAR_NAME		0x6006
#define REACTOR_VAR_VAL			0x6007
#define REACTOR_VAR_INF			0x6008
#define REACTOR_VAR_MULT		0x6009
#define REACTOR_VAR_FNUM		0x7000
#define REACTOR_VAR_POS			0x7300
#define REACTOR_VAR_QVAL		0x7600
#define REACTOR_USE_CURVE		0x7700
#define REACTOR_VAR_MASTERID	0x7701
#define REACTOR_SVAR_ENTRY0		0x8000
#define REACTOR_SVAR_ENTRYN		0x8fff
#define REACTOR_VVAR_ENTRY0		0x9000
#define REACTOR_VVAR_ENTRYN		0x9fff
#define LOCK_CHUNK				0x2535  //the lock value

IOResult Reactor::Save(ISave *isave)
{		
	ULONG 	nb;
	int		i, ct, intVar;
 
	isave->BeginChunk(REACTOR_RANGE_CHUNK);
	isave->Write(&range, sizeof(range), &nb);
 	isave->EndChunk();

	isave->BeginChunk(REACTOR_UPVECTOR_CHUNK);
	isave->Write(&upVector, sizeof(Point3), &nb);
 	isave->EndChunk();

	isave->BeginChunk(REACTOR_CURFVAL_CHUNK);
	isave->Write(&curfval, sizeof(float), &nb);
 	isave->EndChunk();

	isave->BeginChunk(REACTOR_CURPVAL_CHUNK);
	isave->Write(&curpval, sizeof(Point3), &nb);
 	isave->EndChunk();

	isave->BeginChunk(REACTOR_CURQVAL_CHUNK);
	isave->Write(&curqval, sizeof(Quat), &nb);
 	isave->EndChunk();

	isave->BeginChunk(REACTOR_USE_CURVE);
	isave->Write(&isCurveControlled, sizeof(int), &nb);
 	isave->EndChunk();

	isave->BeginChunk(REACTOR_SVAR_TABSIZE);
	intVar = slaveStates.Count();
	isave->Write(&intVar, sizeof(intVar), &nb);
 	isave->EndChunk();
	
	int on = (mLocked==true) ? 1 :0;
	isave->BeginChunk(LOCK_CHUNK);
	isave->Write(&on,sizeof(on),&nb);	
	isave->EndChunk();	

	ct = count;
	for(i = 0; i < ct; i++) {
	 	isave->BeginChunk(REACTOR_SVAR_ENTRY0+i);

	 	// isave->BeginChunk(REACTOR_VAR_NAME);
		 //isave->WriteCString(reaction[i].name);
 		// isave->EndChunk();
	 	// isave->BeginChunk(REACTOR_VAR_POS);
		 //isave->Write(&reaction[i].pvalue, sizeof(Point3), &nb);
 		// isave->EndChunk();
	 	// isave->BeginChunk(REACTOR_VAR_FNUM);
		 //isave->Write(&reaction[i].fvalue, sizeof(float), &nb);
 		// isave->EndChunk();
	 	// isave->BeginChunk(REACTOR_VAR_QVAL);
		 //isave->Write(&reaction[i].qvalue, sizeof(Quat), &nb);
 		// isave->EndChunk();

	 	 isave->BeginChunk(REACTOR_VAR_VAL);
		 isave->Write(&slaveStates[i].fstate, sizeof(float), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(REACTOR_VAR_RQUAT);
		 isave->Write(&slaveStates[i].qstate, sizeof(Quat), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(REACTOR_VAR_RVECTOR);
		 isave->Write(&slaveStates[i].pstate, sizeof(Point3), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(REACTOR_VAR_INF);
		 isave->Write(&slaveStates[i].influence, sizeof(float), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(REACTOR_VAR_STRENGTH);
		 isave->Write(&slaveStates[i].strength, sizeof(float), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(REACTOR_VAR_FALLOFF);
		 isave->Write(&slaveStates[i].falloff, sizeof(float), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(REACTOR_VAR_MULT);
		 isave->Write(&slaveStates[i].multiplier, sizeof(float), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(REACTOR_VAR_MASTERID);
		 isave->Write(&slaveStates[i].masterID, sizeof(int), &nb);
 		 isave->EndChunk();
		 

	 	isave->EndChunk();
	}

	return IO_OK;
}

IOResult Reactor::Load(ILoad *iload)
	{
	ULONG 	nb;
	int		id, i, varIndex, intVar = 0;
	IOResult res;
	ReactionMaster	dummyMaster;
	master = NULL;
	ReactionMaster* in_master = NULL;

	SuperDuperFixAllPLCB* plcb = new SuperDuperFixAllPLCB(this);
	iload->RegisterPostLoadCallback(plcb);
	
	bool bTransferReactorReferencesPLCBRegistered = false;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (id = iload->CurChunkID()) {
	
// these exist in Pre-R7 files 		
		case REACTOR_RTYPE_CHUNK:
			iload->Read(&dummyMaster.type, sizeof(int), &nb);
			if (!bTransferReactorReferencesPLCBRegistered)
			{
				// Registering the callback based on the file version alone is not
				// robust since the file version could change, while the reactor data
				// could stay in its pre-R7 state, if reactor is loaded as a standin
				/// and saved out from a post-R7 version of Max.
				iload->RegisterPostLoadCallback(new TransferReactorReferencePLCB(*this));
				bTransferReactorReferencesPLCBRegistered = true;
			}
			break;
		case REACTOR_VREFS_SUBNUM:
			iload->Read(&dummyMaster.subnum, sizeof(int), &nb);
			break;
// End of PreR7 items
		case LOCK_CHUNK:
				{
					int on;
					res=iload->Read(&on,sizeof(on),&nb);
					if(on)
						mLocked = true;
					else
						mLocked = false;
				}
				break;
		case REACTOR_RANGE_CHUNK:
			iload->Read(&range, sizeof(range), &nb);
			break;
		case REACTOR_UPVECTOR_CHUNK:
			iload->Read(&upVector, sizeof(Point3), &nb);
			break;
		case REACTOR_CURFVAL_CHUNK:
			iload->Read(&curfval, sizeof(float), &nb);
			break;
		case REACTOR_CURPVAL_CHUNK:
			iload->Read(&curpval, sizeof(Point3), &nb);
			break;
		case REACTOR_CURQVAL_CHUNK:
			iload->Read(&curqval, sizeof(Quat), &nb);
			break;
		case REACTOR_USE_CURVE:
			iload->Read(&isCurveControlled, sizeof(int), &nb);
			break;
		case REACTOR_SVAR_TABSIZE:
			iload->Read(&intVar, sizeof(intVar), &nb);
			//count = intVar;
			slaveStates.SetCount(intVar);
			for(i = 0; i < intVar; i++)
			{
				memset(&slaveStates[i], 0, sizeof(SlaveState));
			}
			break;
		}	
		if(id >= REACTOR_SVAR_ENTRY0 && id <= REACTOR_SVAR_ENTRYN) {
			varIndex = id - REACTOR_SVAR_ENTRY0;
			assert(varIndex < slaveStates.Count());
			while (IO_OK == iload->OpenChunk()) {
				switch (iload->CurChunkID()) {
				case REACTOR_VAR_VAL:
					iload->Read(&slaveStates[varIndex].fstate, sizeof(float), &nb);
					break;
				case REACTOR_VAR_RQUAT:
					iload->Read(&slaveStates[varIndex].qstate, sizeof(Quat), &nb);
					break;
				case REACTOR_VAR_RVECTOR:
					iload->Read(&slaveStates[varIndex].pstate, sizeof(Point3), &nb);
					break;
				case REACTOR_VAR_INF:
					iload->Read(&slaveStates[varIndex].influence, sizeof(float), &nb);
					break;
				case REACTOR_VAR_STRENGTH:
					iload->Read(&slaveStates[varIndex].strength, sizeof(float), &nb);
					break;
				case REACTOR_VAR_FALLOFF:
					iload->Read(&slaveStates[varIndex].falloff, sizeof(float), &nb);
					break;
				case REACTOR_VAR_MULT:
					iload->Read(&slaveStates[varIndex].multiplier, sizeof(float), &nb);
					break;
				case REACTOR_VAR_MASTERID:
					iload->Read(&slaveStates[varIndex].masterID, sizeof(int), &nb);
					break;

// these exist in Pre-R7 files 		
				case REACTOR_VAR_NAME:
					{
					// update the master if you found pre-R7 data.
					if (in_master == NULL)
					{
						in_master = new ReactionMaster();
						int count = slaveStates.Count();
						dummyMaster.states.SetCount(count);
						for(i = 0; i < count; i++)
							dummyMaster.states[i] = new MasterState(_T(""), dummyMaster.type, NULL);			
					}
					slaveStates[varIndex].masterID = varIndex;

					TCHAR* buf;
					iload->ReadCStringChunk(&buf);
					dummyMaster.states[varIndex]->name = _T("");
					dummyMaster.states[varIndex]->name.printf(_T("%s"), buf);
					break;
					}
				case REACTOR_VAR_FNUM:
					// update the master if you found pre-R7 data.
					if (in_master == NULL)
					{
						in_master = new ReactionMaster();
						int count = slaveStates.Count();
						dummyMaster.states.SetCount(count);
						for(i = 0; i < count; i++)
							dummyMaster.states[i] = new MasterState(_T(""), dummyMaster.type, NULL);			
					}
					iload->Read(&dummyMaster.states[varIndex]->fvalue, sizeof(float), &nb);
					break;
				case REACTOR_VAR_POS:
					// update the master if you found pre-R7 data.
					if (in_master == NULL)
					{
						in_master = new ReactionMaster();
						int count = slaveStates.Count();
						dummyMaster.states.SetCount(count);
						for(i = 0; i < count; i++)
							dummyMaster.states[i] = new MasterState(_T(""), dummyMaster.type, NULL);			
					}
					iload->Read(&dummyMaster.states[varIndex]->pvalue, sizeof(Point3), &nb);
					break;
				case REACTOR_VAR_QVAL:
					// update the master if you found pre-R7 data.
					if (in_master == NULL)
					{
						in_master = new ReactionMaster();
						int count = slaveStates.Count();
						dummyMaster.states.SetCount(count);
						for(i = 0; i < count; i++)
							dummyMaster.states[i] = new MasterState(_T(""), dummyMaster.type, NULL);			
					}
					iload->Read(&dummyMaster.states[varIndex]->qvalue, sizeof(Quat), &nb);
					break;
// End of PreR7 items
				}	
				iload->CloseChunk();
			}
		}
		iload->CloseChunk();
	}
	if (in_master != NULL)
	{
		*(ReactionMaster*)in_master = dummyMaster; // update the master if you actually found a preR7 master.
		ReplaceReference( kREFID_MASTER, in_master);
		if(slaveStates.Count() > 0)
		{
			// initialize the current values just in case you were previously 
			// reacting to an ownerless pblock or something.  These are now saved
			// with the file.
			curfval = slaveStates[0].fstate;
			curpval = slaveStates[0].pstate;
			curqval = slaveStates[0].qstate;
		}
	}

	updReactionCt(intVar);
	return IO_OK;
}

int Reactor::RemapRefOnLoad(int iref)
{ 
	if (iref == kREFID_CLIENT_PRE_R7 && master != NULL)
	{
		DbgAssert(kNUM_REFS_PRE_R7 == mNumRefs);
		return kREFID_CLIENT_REMAP_PRE_R7; 
	}
	return iref;
}
	
void Reactor::HoldAll()
	{
	if (theHold.Holding()) { 	
		theHold.Put(new FullRestore(this));
		}
	}

void Reactor::HoldParams()
{
	if (theHold.Holding()) {
		theHold.Put(new SpinnerRestore(this));
	}
}

void Reactor::HoldTrack()
	{
	if (theHold.Holding()&&!TestAFlag(A_HELD)) {		
		theHold.Put(new StateRestore(this));
		SetAFlag(A_HELD);
		}
	}

void Reactor::HoldRange()
{
	if (theHold.Holding() && !TestAFlag(A_HELD)) {
		SetAFlag(A_HELD);
		theHold.Put(new RangeRestore(this));
		}
}

// RB 3/23/99: To solve 75139 (the problem where a node name is found for variables that 
// are not associated with nodes such as globabl tracks) we need to block the propogation
// of this message through our reference to the client of the variable we're referencing.
// In the expression controller's imp of NotifyRefChanged() we're going to block the get
// node name message if the blockGetNodeName variable is TRUE.
void Reactor::getNodeName(ReferenceTarget *client, TSTR &name)
{
	blockGetNodeName = TRUE;
	if (client) client->NotifyDependents(FOREVER,(PartID)&name,REFMSG_GET_NODE_NAME);
	blockGetNodeName = FALSE;
}

void Reactor::setSelected(int i) 
{
	selected = i; 
	NotifyDependents(FOREVER,PART_ALL,REFMSG_REACTOR_SELECTION_CHANGED);
	if (editing) {
		curpval = slaveStates[i].pstate;
		curqval = slaveStates[i].qstate;
		curfval = slaveStates[i].fstate;
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE); 
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	}
	return;
}

void Reactor::ComputeMultiplier(TimeValue t)
{
	if (!iCCtrl) BuildCurves();

	float mtemp, normval, total;
	float m = 0.0f;
	int i, j;
	Tab<float> ftab, inftab;
	Point3 axis;

	ftab.ZeroCount();
	inftab.ZeroCount();
	total = 0.0f;
	if (!editing)
	{
		float mult = 0.0f;

		//Make sure there is always an influentual reaction
		//If not create a temp influence value that is large enough
		//First sum up all multiplier values
		for(i=0;i<count;i++){
			inftab.Append(1, &(slaveStates[i].influence)); 
			switch (getReactionType())
			{
				case FLOAT_VAR:
					mult = 1.0f-((float)fabs(getCurFloatValue(t)-getMasterState(i)->fvalue)/(slaveStates[i].influence));
					break;
				case VECTOR_VAR: 
					mult = 1.0f-(Distance(getMasterState(i)->pvalue, getCurPoint3Value(t))/(slaveStates[i].influence));
					break;
				case SCALE_VAR:
					mult = 1.0f-(Distance(getMasterState(i)->pvalue, (getCurScaleValue(t)).s)/(slaveStates[i].influence));
					break;
				case QUAT_VAR: 
					mult = 1.0f-(QangAxis(getMasterState(i)->qvalue, getCurQuatValue(t), axis)/slaveStates[i].influence);
					break;

				default: DbgAssert(0);
			}
			if (mult<0) mult = 0.0f;
			total += mult;
		}
		//Check to see if any are influentual, total > 0 if any influence
		if(total <= 0.0f) {
			//find the closest reaction
			int which;  
			float closest, closesttemp;
			closest = 10000000.0f;
			which = 0;
			for(i=0;i<count;i++)
			{
				switch (getReactionType())
				{
					case FLOAT_VAR:
						closesttemp = ((float)fabs(getCurFloatValue(t)-getMasterState(i)->fvalue)<closest ? (float)fabs(getCurFloatValue(t)-getMasterState(i)->fvalue) : closest);
						break;
					case VECTOR_VAR: 
						closesttemp = (Distance(getMasterState(i)->pvalue, getCurPoint3Value(t))<closest ? Distance(getMasterState(i)->pvalue, getCurPoint3Value(t)) : closest);
						break;
					case SCALE_VAR: 
						closesttemp = (Distance(getMasterState(i)->pvalue, (getCurScaleValue(t)).s)<closest ? Distance(getMasterState(i)->pvalue, (getCurScaleValue(t)).s) : closest);
						break;
					case QUAT_VAR: 
						closesttemp = (QangAxis(getMasterState(i)->qvalue, getCurQuatValue(t), axis)<closest ? QangAxis(getMasterState(i)->qvalue, getCurQuatValue(t), axis) : closest);
						break;
					default: closesttemp = 10000000.0f;
				}
				if (closesttemp < closest) 
				{
					which = i;	
					closest = closesttemp;
				}
			}
			if(count&&closest) 
			{
				inftab[which] = closest + 1.0f;  //make the influence a little more than the closest reaction
			}
		}

		//Get the initial multiplier by determining it's influence
		for(i=0;i<count;i++)
		{
			switch (getReactionType())
			{
				case FLOAT_VAR:
					m = 1.0f-((float)fabs(getCurFloatValue(t)-getMasterState(i)->fvalue)/inftab[i]);
					break;
				case VECTOR_VAR: 
					m = 1.0f-(Distance(getMasterState(i)->pvalue, getCurPoint3Value(t))/inftab[i]);
					break;
				case SCALE_VAR: 
					m = 1.0f-(Distance(getMasterState(i)->pvalue, (getCurScaleValue(t)).s)/inftab[i]);
					break;
				case QUAT_VAR: 
					m = 1.0f-(QangAxis(getMasterState(i)->qvalue, getCurQuatValue(t), axis)/inftab[i]);
					break;

				default: DbgAssert(0);
			}
			if(m<0) m=0;
			slaveStates[i].multiplier = m;
		}
		
		for(i=0;i<count;i++)
		{
			//add the strength
			slaveStates[i].multiplier  *= slaveStates[i].strength;
		}

		//Make an adjustment so that when a value is reached 
		//the state is also reached reguardless of the other influentual reactions 
		for(i=0;i<count;i++)
		{
			mtemp = 1.0f;
			for(j=0;j<count;j++)
			{
				BOOL is_same = false;
				switch (getReactionType())
				{
					case SCALE_VAR:
					case VECTOR_VAR:
						if((*((Point3*)getReactionValue(j))) == (*((Point3*)getReactionValue(i)))) 
							is_same = true;
						break;
					case QUAT_VAR:
						if((*((Quat*)getReactionValue(j))) == (*((Quat*)getReactionValue(i)))) 
							is_same = true;
						break;
					case FLOAT_VAR:
						if((*((float*)getReactionValue(j))) == (*((float*)getReactionValue(i)))) 
							is_same = true;
						break;
					default : is_same = false;
				}
				if (is_same ) mtemp *= slaveStates[j].multiplier; 
					else mtemp *= (1.0f - slaveStates[j].multiplier);
			}
			if(mtemp<0) mtemp = 0.0f;
			ftab.Append(1, &mtemp);
		}

		//update the ReactionSet multipliers
		for(i=0;i<count;i++)
		{
			slaveStates[i].multiplier = ftab[i];
			//compute the falloff
			if (!isCurveControlled) slaveStates[i].multiplier = (float)pow(slaveStates[i].multiplier, (1/slaveStates[i].falloff));
			else 
			{
				ICurve* curve = iCCtrl->GetControlCurve(i);
				if (curve) slaveStates[i].multiplier = curve->GetValue(0, slaveStates[i].multiplier, FOREVER, FALSE);
			}
		}
		//make sure they always add up to 1.0
		int valcount = 0;
		total = 0.0f;
		for(i=0;i<count;i++)
			total +=slaveStates[i].multiplier;
		if (!total) total = 1.0f; 
		normval = 1.0f/total;
		for(i=0;i<count;i++)
		slaveStates[i].multiplier *= normval;
	}
}

BOOL Reactor::reactTo(ReferenceTarget *anim, TimeValue t, bool createReaction)
{
	Animatable* nd;

	HoldAll();

	if (anim->SuperClassID()==BASENODE_CLASS_ID)
	{
		nd = (INode*)anim;

		Control *c = ((INode*)nd)->GetTMController();
		if (c->ClassID() == BIPSLAVE_CONTROL_CLASS_ID || c->ClassID() == IKSLAVE_CLASSID)
		{
			if (!(assignReactObj((INode*)nd, IKTRACK))) return FALSE;
		}else {
			if (!(assignReactObj((INode*)nd, WSPOSITION))) return FALSE;
		}
	} 
	else {
		MyEnumProc dep(anim);             
		anim->DoEnumDependents(&dep);

		for(int x=0; x<dep.anims.Count(); x++)
		{
			for(int i=0; i<dep.anims[x]->NumSubs(); i++)
			{
				Animatable* n = dep.anims[x]->SubAnim(i);
				if ((Control*)n == (Control*)anim)
				{
					if (!(assignReactObj((ReferenceTarget*)dep.anims[x], i))) return FALSE;
				}
			}
		}
	}
	iCCtrl = NULL;
	theHold.Suspend();
	BuildCurves();
	theHold.Resume();

	BOOL notified = FALSE;
	if (createReaction)
		notified = CreateReaction();
	
	if (!notified)
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_REACTTO_OBJ_NAME_CHANGED);
	
	return TRUE;
	//if (theHold.Holding()) theHold.Accept(GetString(IDS_ASSIGN_TO));
}

BOOL Reactor::assignReactObj(ReferenceTarget* client, int subNum)
{
	if (!client) return FALSE;

	slaveStates.ZeroCount();
	count = 0;
	selected = 0;
	
	ReactionMaster* originalClient = GetMaster();
	ReactionMaster* newClient = GetReactionManager()->AddReaction(client, subNum, this);
	
	if(ReplaceReference(0, newClient) != REF_SUCCEED) {
		master = NULL;
		count = 0;
		theHold.Cancel();
		GetReactionManager()->RemoveSlave(client, subNum, this);
		if (GetReactionDlg())
		{
			TSTR s = GetString(IDS_AF_CIRCULAR_DEPENDENCY);
			MessageBox(GetReactionDlg()->hWnd, s, GetString(IDS_AF_CANT_ASSIGN), 
				MB_ICONEXCLAMATION | MB_SYSTEMMODAL | MB_OK);
		}
		return FALSE;
	}

	GetReactionManager()->RemoveReaction(originalClient, originalClient!=NULL?originalClient->SubIndex():0, this);

	return TRUE;
}

void Reactor::updReactionCt(int val)
{
	count += val;
	if (getReactionType() == FLOAT_VAR) SortReactions();
}


SlaveState* Reactor::CreateReactionAndReturn(BOOL setDefaults, TCHAR *buf, TimeValue t, int masterID)
{
	if (!GetMaster()) return NULL;
	
	if (masterID < 0)
		GetMaster()->AddState(buf, masterID, true, t);

	HoldParams();  
	

	int i;
	SlaveState sv;
	sv.masterID = masterID;
	sv.strength = 1.0f;
	sv.falloff = 2.0f;
	sv.influence = 100.0f;
	if (setDefaults)
	{
		sv.fstate = curfval;
		sv.qstate = curqval;
		sv.pstate = curpval;
	}
	
	i = slaveStates.Append(1, &sv);
	selected = i;

	if (setDefaults)
	{		
		//Scheme to set influence to nearest reaction automatically (better defaults)
		if (i!=0)		//if its not the first reaction
		{
			if(i == 1)   //and if it is the second one update the first while your at it
				setMinInfluence(0);
			setMinInfluence(i);
		}
	}

	theHold.Suspend();

	if (iCCtrl) 
	{ 
		if (getReactionType() == FLOAT_VAR)
		{
			if (!(flags&REACTOR_BLOCK_CURVE_UPDATE))
			{
				iCCtrl->SetNumCurves(Elems(), TRUE); 
				RebuildFloatCurves();
			}
		}	
		else 
		{
			iCCtrl->SetNumCurves(slaveStates.Count(), TRUE); 
			NewCurveCreatedCallback(slaveStates.Count()-1, iCCtrl); 
		}
	}
	
	BOOL isSame = FALSE;
	int retVal = ( !(flags & REACTOR_DONT_CREATE_SIMILAR) );

	if (setDefaults && masterID < 0)
	{
		for (int x=0; x < i; x++)
		{
			void *iVal = getReactionValue(i);
			void *xVal = getReactionValue(x);	
			switch (getReactionType())
			{
				case FLOAT_VAR:
					if ( *((float*)iVal) == *((float*)xVal) )
						isSame = TRUE;
					break;

				case VECTOR_VAR:
				case SCALE_VAR:
					if ( *((Point3*)iVal) == *((Point3*)xVal) )
						isSame = TRUE;
					break;
				case QUAT_VAR:
					if ( *((Point3*)iVal) == *((Point3*)xVal) )
						isSame = TRUE;
					break;
				default: break;	
			}	
			if ( isSame )
			{
				//check to see if the reaction value is the same as that of an existing reaction.  If it is, post a message
				if ( !(flags&REACTOR_DONT_SHOW_CREATE_MSG) )
				{
					DWORD extRetVal;
					HWND hWnd = GetCOREInterface()->GetMAXHWnd();

					TSTR *question = new TSTR(GetString(IDS_SAME_REACTION));
					retVal = ( MaxMsgBox(hWnd, question->data(), GetString(IDS_AF_REACTOR),
								MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON1,
								MAX_MB_DONTSHOWAGAIN, &extRetVal) == IDYES );
					delete question;
					
					//don't show the dialog again (for this session anyway...)
					if ( extRetVal == MAX_MB_DONTSHOWAGAIN ) 
					{
						flags = (flags | REACTOR_DONT_SHOW_CREATE_MSG);
						if (retVal == 0)
						{
							flags = (flags|REACTOR_DONT_CREATE_SIMILAR);
						}
					}
				}
				break;
			}
		}
	}
	theHold.Resume();

	if (isSame && retVal == 0)
	{
		theHold.Cancel();
		return NULL;
	}

	updReactionCt(1);

	ivalid.SetEmpty();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_REACTION_COUNT_CHANGED);	
	return &(slaveStates[selected]);
}

BOOL Reactor::CreateReaction(TCHAR *buf, TimeValue t)
{
	if ( CreateReactionAndReturn(TRUE, buf, t) ) return TRUE;
	else return FALSE;
}

BOOL Reactor::DeleteReaction(int i)
{
	if(GetMaster() != NULL)
	{		
		if (i == -1) i = selected;
		if (TRUE) //getReactionCount()>1)  //can't delete the last reaction  // you can now...
		{
			HoldParams(); 

			if (getReactionType() != FLOAT_VAR && iCCtrl->GetNumCurves() >= slaveStates.Count())
			{
				iCCtrl->DeleteCurve(i);
			}

			slaveStates.Delete(i, 1);
			count = getReactionCount(); 
			if (selected >= count ) selected -=1;

			if (!(flags&REACTOR_BLOCK_CURVE_UPDATE))
				RebuildFloatCurves();

			ivalid.SetEmpty();
			NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			NotifyDependents(FOREVER,PART_ALL,REFMSG_REACTION_COUNT_CHANGED);	
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
			return TRUE;
		}

	}return FALSE;
}



TCHAR *Reactor::getReactionName(int i)
{
	if(i>=0&&i<count&&count>0)
		return getMasterState(i)->name;
	return "error";
}

void Reactor::setReactionName(int i, TSTR name)
{
	if (i>=0&&i<slaveStates.Count())
	{
		getMasterState(i)->name = name;
		NotifyDependents(FOREVER,i,REFMSG_REACTION_NAME_CHANGED);
	}
}

void* Reactor::getReactionValue(int i)
{
	if(getReactionCount() > 0 && i >= 0 && i < getReactionCount())
	{
		switch (getReactionType())
		{
			case FLOAT_VAR:
				return &(getMasterState(i)->fvalue);
			case QUAT_VAR:
				return &(getMasterState(i)->qvalue);
			case VECTOR_VAR:
			case SCALE_VAR:
				return &(getMasterState(i)->pvalue);
			default: assert(0);
		}
	}
	return NULL;
}

FPValue Reactor::fpGetReactionValue(int index)
{
	FPValue val = FPValue();
	if(getReactionCount() > 0 && index >= 0 && index < getReactionCount())
	{
		switch (getReactionType())
		{
			case FLOAT_VAR:
				val.type = (ParamType2)TYPE_FLOAT;
				val.f =  getMasterState(index)->fvalue;
				break;
			case QUAT_VAR:
				val.type = (ParamType2)TYPE_QUAT;
				val.q =  &(getMasterState(index)->qvalue);
				break;
			case VECTOR_VAR:
			case SCALE_VAR:
				val.type = (ParamType2)TYPE_POINT3;
				val.p = &(getMasterState(index)->pvalue);
				break;
			default: assert(0);
		}
	} else throw MAXException(GetString(IDS_NOT_IN_VALID_RANGE));
	return val;
}


BOOL Reactor::setReactionValue(int i, TimeValue t)
{
	if(GetLocked()==false)
	{
		float f;
		Quat q;
		Point3 p;
		ScaleValue s;
		Control *c;
		
		if (t == NULL) t = GetCOREInterface()->GetTime();
		if(getReactionCount() == 0 || i < 0 || i >= getReactionCount()) 
		{
			if (i == -1) i = selected;
				else return false;
		}

		if (GetMaster() != NULL) {

			//theHold.Begin();
			HoldParams();

			MasterState* masterState = getMasterState(i);
			
			masterState->fvalue = 0.0f;
			masterState->pvalue = Point3(0,0,0);
			masterState->qvalue.Identity();

			if (GetMaster()->subnum < 0 )
			{
				if (GetMaster()->subnum == IKTRACK)	
				{
					GetAbsoluteControlValue((INode*)GetMaster()->client, GetMaster()->subnum, t, &(masterState->qvalue), FOREVER);
				}
				else {
					GetAbsoluteControlValue((INode*)GetMaster()->client, GetMaster()->subnum, t, &(masterState->pvalue), FOREVER);
				}
			}
			else {
				if (GetMaster()->client) {
				c = (Control *)GetMaster()->client->SubAnim(GetMaster()->subnum);
				switch (getReactionType())
				{
					case FLOAT_VAR:
						c->GetValue(t, &f, FOREVER);
						masterState->fvalue = f;
						SortReactions();
						UpdateCurves();
						break;
					case VECTOR_VAR:
						c->GetValue(t, &p, FOREVER);
						masterState->pvalue = p;
						break;
					case SCALE_VAR:
						c->GetValue(t, &s, FOREVER);
						masterState->pvalue = s.s;
						break;
					case QUAT_VAR:
						c->GetValue(t, &q, FOREVER);
						masterState->qvalue = q;
						break;
				}
			}
			}

			ivalid.SetEmpty();
			NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
			//if (theHold.Holding()) theHold.Accept(GetString(IDS_UNSET_VALUE));
			return TRUE;
		}
	}
	return FALSE;
}


BOOL Reactor::setReactionValue(int i, float val)
{
	if(GetLocked()==false)
	{
		if (getReactionType() != FLOAT_VAR ) return false;
		if(getReactionCount() == 0 || i < 0 || i >= getReactionCount()) 
		{
			if (i == -1) i = selected;
				else return false;
		}

		if (GetMaster() != NULL) {

			//theHold.Begin();
			HoldParams();
			
			getMasterState(i)->fvalue = val;
			SortReactions();

			ivalid.SetEmpty();
			NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
			//if (theHold.Holding()) theHold.Accept(GetString(IDS_UNSET_VALUE));
			UpdateCurves();
			return TRUE;
		}
	}
	return FALSE;
}

BOOL Reactor::setReactionValue(int i, Point3 val)
{
	if(GetLocked()==false)
	{
		if (getReactionType() != VECTOR_VAR && getReactionType() != SCALE_VAR ) return false;
		if(getReactionCount() == 0 || i < 0 || i >= getReactionCount()) 
		{
			if (i == -1) i = selected;
				else return false;
		}

		if (GetMaster() != NULL) {

			//theHold.Begin();
			HoldParams();
			
			getMasterState(i)->pvalue = val;
			
			ivalid.SetEmpty();
			NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
			//if (theHold.Holding()) theHold.Accept(GetString(IDS_UNSET_VALUE));
			return TRUE;
		}
	}
	return FALSE;
}

BOOL Reactor::setReactionValue(int i, Quat val)
{
	if (getReactionType() != QUAT_VAR ) return false;
	if(getReactionCount() == 0 || i < 0 || i >= getReactionCount()) 
	{
		if (i == -1) i = selected;
			else return false;
	}

	if (GetMaster() != NULL) {

		//theHold.Begin();
		HoldParams();
		
		getMasterState(i)->qvalue = val;
		
		ivalid.SetEmpty();
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		//if (theHold.Holding()) theHold.Accept(GetString(IDS_UNSET_VALUE));
		return TRUE;
	}
	return FALSE;
}

float Reactor::getCurFloatValue(TimeValue t)
{
	float f = 0.0f;
	Control *c = NULL;

	if (GetMaster() && GetMaster()->client) {
		c = GetControlInterface(GetMaster()->client->SubAnim(GetMaster()->subnum));
		if (c != NULL)
			{
			c->GetValue(t, &f, FOREVER);
			return f;
			}
	}
	return 0.0f;
}

Point3 Reactor::getCurPoint3Value(TimeValue t)
{
	Point3 p = Point3(0,0,0);
	Control *c = NULL;

	if (GetMaster() && GetMaster()->client) {
		if (GetMaster()->subnum < 0 )
		{
			GetAbsoluteControlValue((INode*)GetMaster()->client, GetMaster()->subnum, t, &p, FOREVER);
		}else {
			c = GetControlInterface(GetMaster()->client->SubAnim(GetMaster()->subnum));
			if (c != NULL)
				c->GetValue(t, &p, FOREVER);
		}
	}
	return p;
}

ScaleValue Reactor::getCurScaleValue(TimeValue t)
{
	ScaleValue ss(Point3(1.0f, 1.0f, 1.0f));
	Control *c = NULL;

	if (GetMaster() && GetMaster()->client) {
		c = GetControlInterface(GetMaster()->client->SubAnim(GetMaster()->subnum));
		if (c != NULL)
			c->GetValue(t, &ss, FOREVER);
	}
	return ss;
}

Quat Reactor::getCurQuatValue(TimeValue t)
{
	Quat q;
	q.Identity();
	Control *c = NULL;

	if (GetMaster() != NULL && GetMaster()->client) {
		if (GetMaster()->subnum < 0 )
		{
			GetAbsoluteControlValue((INode*)GetMaster()->client, GetMaster()->subnum, t, &q, FOREVER);
		}else {
			c = GetControlInterface(GetMaster()->client->SubAnim(GetMaster()->subnum));
			if (c != NULL)
				c->GetValue(t, &q, FOREVER);
		}
	}
	return q;
}



float Reactor::getInfluence(int num)
{
	if (num >= 0 && num <slaveStates.Count())
		return slaveStates[num].influence;
	return 0;
}

BOOL Reactor::setInfluence(int num, float inf)
{
	if(GetLocked()==false)
	{
		BOOL hold_here = false;

		//if (!theHold.Holding()) { hold_here=true; theHold.Begin();}
		HoldParams();

		slaveStates[num].influence = inf;

		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		//if (hold_here && theHold.Holding() ) theHold.Accept(GetString(IDS_AF_CHANGEINFLUENCE));
		return TRUE;
	}
	return FALSE;
}

void Reactor::setMinInfluence(int x)
{
	if(GetLocked()==false)
	{
		//BOOL hold_here = false;
		//if (!theHold.Holding()) 
		//{ 
		//	hold_here=true; theHold.Begin(); 
		//}
		HoldParams();
		
		if ( x == -1 ) x = selected;
		float dist = 1000000.0f;
		float disttemp = 100.0f;
		Point3 axis;

		if ( slaveStates.Count() && x >= 0 )
		{
			for(int i=0;i<slaveStates.Count();i++)
			{
				if (i != x)
				{
					switch (getReactionType())
					{
						case FLOAT_VAR:
							disttemp = (float)fabs(getMasterState(x)->fvalue - getMasterState(i)->fvalue);
							break;
						case VECTOR_VAR: 
						case SCALE_VAR: 
							disttemp = Distance(getMasterState(i)->pvalue, getMasterState(x)->pvalue);
							break;
						case QUAT_VAR: 
							disttemp = QangAxis(getMasterState(i)->qvalue, getMasterState(x)->qvalue, axis);
							break;
						default: disttemp = 100.0f; break;
					} 
				}
				if (disttemp != 0.0f) 
				{
					dist = (dist <= disttemp && dist != 100 ? dist : disttemp);
				} else if (dist == 1000000.0f && i == slaveStates.Count()-1) dist = 100.0f;
			}
			slaveStates[x].influence = dist;
			NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			//if (theHold.Holding() && hold_here) theHold.Accept(GetString(IDS_AF_CHANGEINFLUENCE));
		}
	}
}

void Reactor::setMaxInfluence(int x)
{
	if(GetLocked()==false)
	{
		//BOOL hold_here = false;
		//if (!theHold.Holding()) { hold_here=true; theHold.Begin(); }
		HoldParams();
		if ( x == -1 ) x = selected;

		float dist = 0.0f;
		float disttemp;
		Point3 axis;

		if ( count && x >= 0 )
		{
			for(int i=0;i<count;i++)
			{
				switch (getReactionType())
				{
					case FLOAT_VAR:
						disttemp = (float)fabs(getMasterState(x)->fvalue - getMasterState(i)->fvalue);
						break;
					case VECTOR_VAR: 
					case SCALE_VAR: 
						disttemp = Distance(getMasterState(i)->pvalue, getMasterState(x)->pvalue);
						break;
					case QUAT_VAR: 
						disttemp = QangAxis(getMasterState(i)->qvalue, getMasterState(x)->qvalue, axis);
						break;
					default: disttemp = 100.0f; break;
				}
				if (i == 0) dist = disttemp;
				else dist = (dist >= disttemp ? dist : disttemp);
			}
			slaveStates[x].influence = dist;
			NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	//		if (theHold.Holding() && hold_here) theHold.Accept(GetString(IDS_AF_CHANGEINFLUENCE));
		}
	}
}



float Reactor::getStrength(int num)
{
	return slaveStates[num].strength;
}

BOOL Reactor::setStrength(int num, float inf)
{
	if(GetLocked()==false)
	{
		BOOL hold_here = false;

		//if (!theHold.Holding()) { hold_here=true; theHold.Begin();}
		HoldParams();

		slaveStates[num].strength = inf;

		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		//if(theHold.Holding() && hold_here) theHold.Accept(GetString(IDS_AF_CHANGESTRENGTH));
		return TRUE;
	}
	return FALSE;
}

float Reactor::getFalloff(int num)
{
	return slaveStates[num].falloff;
}

BOOL Reactor::setFalloff(int num, float inf)
{
	if(GetLocked()==false)
	{
		BOOL hold_here = false;

		//if (!theHold.Holding()) { hold_here=true; theHold.Begin();}
		HoldParams();

		slaveStates[num].falloff = inf;

		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		//if (theHold.Holding() && hold_here) theHold.Accept(GetString(IDS_AF_CHANGEFALLOFF));
		return TRUE;
	}
	return FALSE;
}

void Reactor::setEditReactionMode(BOOL ed)
{
	if(GetLocked()==false)
	{
		editing = ed;
		ivalid.SetEmpty();
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
}

void Reactor::setCreateReactionMode(BOOL creating)
{
	if(GetLocked()==false)
	{
		createMode = creating;
		ivalid.SetEmpty();
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
}

BOOL Reactor::getCreateReactionMode()
{
	return createMode;
}

BOOL Reactor::setState(int num, TimeValue t)
{
	if(GetLocked()==false)
	{
		if(getReactionCount() == 0 || num < 0 || num >= getReactionCount()) 
			if ( num == -1 ) num = selected;
				else return FALSE;

		HoldParams();

		if (t == NULL) t = GetCOREInterface()->GetTime();
		switch (type)
		{
			case REACTORFLOAT: 
				this->GetValue(t, &(slaveStates[num].fstate), FOREVER, CTRL_ABSOLUTE);
				break; 
			case REACTORROT: 
				this->GetValue(t, &(slaveStates[num].qstate), FOREVER, CTRL_ABSOLUTE);
				break; 
			case REACTORP3: 
			case REACTORSCALE: 
			case REACTORPOS: 
				this->GetValue(t, &(slaveStates[num].pstate), FOREVER, CTRL_ABSOLUTE);
				break; 
			default: return FALSE;
		}
		ivalid.SetEmpty();
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		return TRUE; 
	}
	return FALSE;
}


//Overload used by Float reactors
BOOL Reactor::setState(int num, float val)
{
	if(GetLocked()==false)
	{
		if (type != REACTORFLOAT) return false;
		if(getReactionCount() == 0 || num < 0 || num >= getReactionCount()) 
			if ( num == -1 ) num = selected;
			else return false;
		
		HoldParams();

		slaveStates[num].fstate = val;

		ivalid.SetEmpty();
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		UpdateCurves();
		return true; 
	}
	return FALSE;
}

//Overload used by Point3 reactors
BOOL Reactor::setState(int num, Point3 val)
{
	if(GetLocked()==false)
	{
		if (type != REACTORP3 && type != REACTORSCALE && type != REACTORPOS) return false;
		if(getReactionCount() == 0 || num < 0 || num >= getReactionCount()) 
			if ( num == -1 ) num = selected;
			else return false;

		HoldParams();
		
		slaveStates[num].pstate = val;

		ivalid.SetEmpty();
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		UpdateCurves();
		return true; 
	}
	return FALSE;
}

//Overload used by Rotation reactors
BOOL Reactor::setState(int num, Quat val)
{
	if(GetLocked()==false)
	{
		if (type != REACTORROT) return false;
		if(getReactionCount() == 0 || num < 0 || num >= getReactionCount()) 
			if ( num == -1 ) num = selected;
			else return false;
		
		HoldParams();

		slaveStates[num].qstate = val;

		ivalid.SetEmpty();
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		return true; 
	}
	return FALSE;
}

void* Reactor::getState(int num)
{
	if(getReactionCount() > 0 && num >= 0 && num < getReactionCount())
		switch (type)
		{
			case REACTORFLOAT: 
				return &slaveStates[num].fstate; 
			case REACTORROT: 
				return &slaveStates[num].qstate;
			case REACTORP3: 
			case REACTORSCALE: 
			case REACTORPOS: 
				return &slaveStates[num].pstate;

		}
	return NULL;
}

FPValue Reactor::fpGetState(int index)
{
	FPValue val = FPValue();

	if(getReactionCount() > 0 && index >= 0 && index < getReactionCount())
	{
		switch (type)
		{
			case REACTORFLOAT: 
				val.type = (ParamType2)TYPE_FLOAT;
				val.f = slaveStates[index].fstate;
				break;
			case REACTORROT: 
				val.type = (ParamType2)TYPE_QUAT;
				val.q = &slaveStates[index].qstate;
				break;
			case REACTORP3: 
			case REACTORSCALE: 
			case REACTORPOS: 
				val.type = (ParamType2)TYPE_POINT3;
				val.p = &slaveStates[index].pstate;
				break;
			default: assert(0);
		}
	}else throw MAXException(GetString(IDS_NOT_IN_VALID_RANGE));
	return val;
}

TSTR Reactor::SvGetRelTip(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker)
{
	return SvGetName(gom, gNodeMaker, false) + " -> " + gNodeTarget->GetAnim()->SvGetName(gom, gNodeTarget, false);
}

bool Reactor::SvHandleRelDoubleClick(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker)
{
	return Control::SvHandleDoubleClick(gom, gNodeMaker);
}

SvGraphNodeReference Reactor::SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags)
{
	SvGraphNodeReference nodeRef = Control::SvTraverseAnimGraph( gom, owner, id, flags );

	if( nodeRef.stat == SVT_PROCEED ) {
		if( GetMaster() != NULL)
			gom->AddRelationship( nodeRef.gNode, GetMaster()->client, 0, RELTYPE_CONTROLLER );
	}

	return nodeRef;
}

static int CompareMasterStates( const void *elem1, const void *elem2 ) {
	float a = ((MasterState *)elem1)->fvalue;
	float b = ((MasterState *)elem2)->fvalue;
	return (a<b)?-1:(a==b?0:1);
}

void Reactor::SortReactions()
{	
	if (slaveStates.Count() > 1)
	{
		Tab<SlaveState> slaves;
		slaves.SetCount(0);
		bool sorted = false;
		for(int i = 0; i < slaveStates.Count(); i++)
		{
			bool found = true;
			MasterState* masterState = getMasterState(i);
			if (masterState)
			{
				found = false;
				for(int x = 0; x < slaves.Count(); x++)
				{
					MasterState* masterState2 = getMasterState(x);
					if (masterState2)
					{
						if (masterState->fvalue < masterState2->fvalue)
						{
							slaves.Insert(x, 1, &slaveStates[i]);
							found = true;
							sorted = true;
							break;
						}
					}
				}
			}
			if (!found)
				slaves.Append(1, &slaveStates[i]);
		}
		slaveStates = slaves;
		count = slaveStates.Count();
		if (sorted)
			NotifyDependents(FOREVER,PART_ALL,REFMSG_REACTION_COUNT_CHANGED);	
	}
}


//--------------------------------------------------------------------

BOOL Reactor::ChangeParents(TimeValue t,const Matrix3& oldP,const Matrix3& newP,const Matrix3& tm)
	{
		HoldAll();
		// Position and rotation controllers need their path counter rotated to
		// account for the new parent.
		Matrix3 rel = oldP * Inverse(newP);
		// Modify the controllers current value (the controllers cache)
		*((Point3*)(&curpval)) = *((Point3*)(&curpval)) * rel;
		*((Quat*)(&curqval)) = *((Quat*)(&curqval)) * rel;

		//Modify each reaction state 
		for (int i=0;i<count;i++)
		{
			*((Point3*)(&slaveStates[i].pstate)) = *((Point3*)(&slaveStates[i].pstate)) * rel;
			*((Quat*)(&slaveStates[i].qstate)) = *((Quat*)(&slaveStates[i].qstate)) * rel;
		}
		ivalid.SetEmpty();
		return TRUE;
	}

void Reactor::Update(TimeValue t)
{
	if (!ivalid.InInterval(t))
	{
		ivalid = FOREVER;		
		if (GetMaster()!=NULL)
		{
			float f;
			Quat q;
			Point3 p;
			ScaleValue s;
			//update the validity interval
			if (GetMaster()->subnum < 0 )
			{
				if (GetMaster()->subnum == IKTRACK) GetAbsoluteControlValue((INode*)GetMaster()->client, GetMaster()->subnum, t, &q, ivalid);
					else GetAbsoluteControlValue((INode*)GetMaster()->client, GetMaster()->subnum, t, &p, ivalid);
			}else {
				Animatable *anim = GetMaster()->client? GetMaster()->client->SubAnim(GetMaster()->subnum): NULL;
				if (anim && GetControlInterface(anim)) {
				switch (getReactionType())
				{
				case FLOAT_VAR:
						GetControlInterface(anim)->GetValue(t, &f, ivalid);
					break;
				case VECTOR_VAR:
						GetControlInterface(anim)->GetValue(t, &p, ivalid);
					break;
				case SCALE_VAR:
						GetControlInterface(anim)->GetValue(t, &s, ivalid);
					break;
				case QUAT_VAR:
						GetControlInterface(anim)->GetValue(t, &q, ivalid);
					break;
				}
			}
			}

			if (!createMode && slaveStates.Count() > selected && selected >= 0)
			{
				curfval = slaveStates[selected].fstate;
				curpval = slaveStates[selected].pstate;
				curqval = slaveStates[selected].qstate;
			}
		}
	}
}


void Reactor::SetValue(TimeValue t, void *val, int commit, GetSetMethod method)
{
	if(GetLocked()==false)
	{
		if ((editing || createMode) && count) {
			if (!TestAFlag(A_SET)) {				
				HoldTrack();
				tmpStore.PutBytes(sizeof(Point3),&curpval,this);
				SetAFlag(A_SET);
				}
			if (method == CTRL_RELATIVE) curpval += *((Point3*)val);
			else curpval = *((Point3*)val);

			ivalid.SetInstant(t);	
			if (commit) CommitValue(t);
			if (!commit) NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			//UpdateCurve();
		}
	}
}


void Reactor::CommitValue(TimeValue t) {
	if (TestAFlag(A_SET)) {		
		if (ivalid.InInterval(t)) {
			Point3 old;
			tmpStore.GetBytes(sizeof(Point3),&old,this);					
			if (editing && count)
				slaveStates[selected].pstate = curpval;
			if (getReactionType() == FLOAT_VAR) UpdateCurves();

			tmpStore.Clear(this);
			ivalid.SetEmpty();
			NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
		ClearAFlag(A_SET);
	}
}

void Reactor::RestoreValue(TimeValue t) 
	{
	if (TestAFlag(A_SET)) {
		if (count) {
			tmpStore.GetBytes(sizeof(Point3),&curpval,this);
			if (editing && count)
				slaveStates[selected].pstate = curpval;
			tmpStore.Clear(this);
			ivalid.SetInstant(t);
			}
		ClearAFlag(A_SET);
		}
	}


//***************************************************************************
//**
//** ResourceMakerCallback implementation
//**
//***************************************************************************

void Reactor::ResetCallback(int curvenum,ICurveCtl *pCCtl)
{
	if (getReactionType() != FLOAT_VAR)
	{
		ICurve *pCurve = NULL;
		pCurve = pCCtl->GetControlCurve(curvenum);
		if(pCurve)
		{
			pCurve->SetNumPts(2);
			NewCurveCreatedCallback(curvenum, pCCtl);
		}
	}
}

void Reactor::NewCurveCreatedCallback(int curvenum,ICurveCtl *pCCtl)
{
	ICurve *newCurve = NULL;
	
	iCCtrl = pCCtl;
	newCurve = iCCtrl->GetControlCurve(curvenum);
	
	if(newCurve)
	{
		newCurve->SetCanBeAnimated(FALSE);
		CurvePoint pt;
		if (getReactionType() != FLOAT_VAR)
		{
			newCurve->SetNumPts(2);
			
			pt = newCurve->GetPoint(0,0);
			pt.p = Point2(0.0f,0.0f);
			pt.flags = (CURVEP_CORNER|CURVEP_BEZIER|CURVEP_LOCKED_Y|CURVEP_LOCKED_X);
			pt.out = Point2(0.25f, 0.25f);
			newCurve->SetPoint(0,0,&pt);
			
			pt = newCurve->GetPoint(0,1);
			pt.p = Point2(1.0f,1.0f);
			pt.flags = (CURVEP_CORNER|CURVEP_BEZIER|CURVEP_LOCKED_Y|CURVEP_LOCKED_X);
			pt.in = Point2(-0.25f, -0.25f);
			newCurve->SetPoint(0,1,&pt);
			newCurve->SetPenProperty( RGB(0,0,0));
			newCurve->SetDisabledPenProperty( RGB(128,128,128));
		}else
		{
			RebuildFloatCurves();
			switch (curvenum) 
			{
			case 0: newCurve->SetPenProperty( RGB(255,0,0)); break;
			case 1: newCurve->SetPenProperty( RGB(0,255,0)); break;
			case 2: newCurve->SetPenProperty( RGB(0,0,255)); break;
			default: break;
			}
			newCurve->SetDisabledPenProperty( RGB(128,128,128));
		}
	}
}

void Reactor::BuildCurves()
{
	int numCurves = 1;
	ICurve *pCurve = NULL;
	CurvePoint point;

	if (getReactionType() == FLOAT_VAR)
	{
		numCurves = Elems();
	}
	else numCurves = slaveStates.Count();

	if(!iCCtrl)
	{
		ReplaceReference( 1, (RefTargetHandle)CreateInstance(REF_MAKER_CLASS_ID,CURVE_CONTROL_CLASS_ID));
		if(!iCCtrl)
			return;
		
		//bug #271818
//		theHold.Suspend();

		BitArray ba;
		//iCCtrl->SetNumCurves(0, FALSE);
		iCCtrl->SetNumCurves(numCurves, FALSE);
		
		if (getReactionType() != FLOAT_VAR)
		{
			iCCtrl->SetXRange(0.0f,1.0f);
			iCCtrl->SetYRange(0.0f,1.0f);
			iCCtrl->SetZoomValues(492.727f, 137.857f);  
			iCCtrl->SetScrollValues(-24, -35);
		
			//LoadCurveControlResources();
			iCCtrl->SetTitle(GetString(IDS_FALLOFF_CURVE));
			ba.SetSize(1);
		}
		else
		{
			iCCtrl->SetTitle(GetString(IDS_REACTION_CURVE));
			iCCtrl->SetCCFlags(CC_SINGLESELECT|CC_DRAWBG|CC_DRAWGRID|CC_DRAWUTOOLBAR|CC_SHOWRESET|CC_DRAWLTOOLBAR|CC_DRAWSCROLLBARS|CC_AUTOSCROLL|
								CC_DRAWRULER|CC_ASPOPUP|CC_RCMENU_MOVE_XY|CC_RCMENU_MOVE_X|CC_RCMENU_MOVE_Y|CC_RCMENU_SCALE|
								CC_RCMENU_INSERT_CORNER|CC_RCMENU_INSERT_BEZIER|CC_RCMENU_DELETE|CC_SHOW_CURRENTXVAL);
			iCCtrl->SetXRange(0.0f,0.01f);
			iCCtrl->SetYRange(9999999.0f,9999999.0f);
			ba.SetSize(numCurves);
		}
		for (int i=0;i<numCurves;i++)
			NewCurveCreatedCallback(i, iCCtrl);
		
		ba.SetAll();
		iCCtrl->SetDisplayMode(ba);	
//		theHold.Resume();
	}
}

void Reactor::ShowCurveControl()
{
	if (!iCCtrl) BuildCurves();
	
	if(iCCtrl->IsActive())
		iCCtrl->SetActive(FALSE);
	else
	{
		iCCtrl->RegisterResourceMaker(this);
		iCCtrl->SetActive(TRUE);
		iCCtrl->ZoomExtents();
	}
}

void Reactor::RebuildFloatCurves()
{
	if (!iCCtrl) return;

	if (getReactionType() == FLOAT_VAR)
	{
		SortReactions();
		theHold.Suspend(); //leave this in.  Not suspending the hold results in an array deletion error in the curve control...
		for (int c =0;c<iCCtrl->GetNumCurves();c++)
		{
			ICurve * curve = iCCtrl->GetControlCurve(c);  //didn't return a valid curve here...
			curve->SetNumPts(slaveStates.Count());
			curve->SetOutOfRangeType(CURVE_EXTRAPOLATE_CONSTANT);
			curve->SetCanBeAnimated(FALSE);
			for(int i=0;i<slaveStates.Count();i++)
			{
				if (c == 0 && i == 0)
				{
					iCCtrl->SetXRange(getMasterState(i)->fvalue, getMasterState(i)->fvalue+1.0f, FALSE);
				}
				else{
					if (getMasterState(i)->fvalue < iCCtrl->GetXRange().x) 
						iCCtrl->SetXRange(getMasterState(i)->fvalue, iCCtrl->GetXRange().y, FALSE);
					if (getMasterState(i)->fvalue > iCCtrl->GetXRange().y) 
						iCCtrl->SetXRange(iCCtrl->GetXRange().x, getMasterState(i)->fvalue, FALSE);
				}
				CurvePoint pt = curve->GetPoint(0,i);
				//pt.flags = 0;
				pt.flags |= (CURVEP_NO_X_CONSTRAINT);  //CURVEP_CORNER|
				//pt.p.x = getMasterState(i)->fvalue;
				switch (type) 
				{
					case REACTORFLOAT:
						pt.p = Point2(getMasterState(i)->fvalue, slaveStates[i].fstate);
						break;
					case REACTORROT:
						float ang[3];
						QuatToEuler(slaveStates[i].qstate, ang);
						pt.p = Point2(getMasterState(i)->fvalue, RadToDeg(ang[c]));

						break;
					case REACTORSCALE:
					case REACTORPOS:
					case REACTORP3:
						pt.p = Point2(getMasterState(i)->fvalue, slaveStates[i].pstate[c]);
						//pt.in = Point2(0.0f,0.0f);
						//pt.in = Point2(0.0f,0.0f);
						break;
				}
				curve->SetPoint(0,i, &pt, TRUE, FALSE);
			}
		}
		theHold.Resume();
	}
	iCCtrl->ZoomExtents();
}

void Reactor::UpdateCurves(bool doZoom)
{
	curvesValid = true;
	if (!iCCtrl) return;
	if (flags&REACTOR_BLOCK_CURVE_UPDATE) return;
	int c = 0;
	ICurve* curve;
	if (getReactionType() != FLOAT_VAR)
	{
		//check for NULL curves and delete them
		for (c = 0;c<iCCtrl->GetNumCurves();c++)
		{
			curve = iCCtrl->GetControlCurve(c);
			if (curve == NULL)
			{
				theHold.Suspend();
				iCCtrl->DeleteCurve(c);
				theHold.Resume();
				continue;
			}
		}
		return;
	}

	CurvePoint pt;

	theHold.Suspend();
	SortReactions();
	for (c = 0;c<iCCtrl->GetNumCurves();c++)
	{
		curve = iCCtrl->GetControlCurve(c);
		for(int i=0;i<slaveStates.Count();i++)
		{
			MasterState* masterState = getMasterState(i);
			
			if (masterState != NULL)
			{
				if (c == 0 && i == 0)
				{
					iCCtrl->SetXRange(masterState->fvalue-0.001f, masterState->fvalue, FALSE);
				}
				else{
					
					if (masterState->fvalue < iCCtrl->GetXRange().x) 
						iCCtrl->SetXRange(masterState->fvalue, iCCtrl->GetXRange().y, FALSE);
					if (masterState->fvalue > iCCtrl->GetXRange().y) 
						iCCtrl->SetXRange(iCCtrl->GetXRange().x, masterState->fvalue, FALSE);
				}
				CurvePoint pt = curve->GetPoint(0,i);
				//pt.p.x = masterState->fvalue;
				switch (type) 
				{
					case REACTORFLOAT:
						pt.p = Point2(masterState->fvalue, slaveStates[i].fstate);
						break;
					case REACTORROT:
						float ang[3];
						QuatToEuler(slaveStates[i].qstate, ang);
						pt.p = Point2(masterState->fvalue, RadToDeg(ang[c]));
						break;
					case REACTORSCALE:
					case REACTORPOS:
					case REACTORP3:
						pt.p = Point2(masterState->fvalue, slaveStates[i].pstate[c]);
						//pt.in = Point2(0.0f,0.0f);
						//pt.in = Point2(0.0f,0.0f);
						break;
				}
				curve->SetPoint(0,i, &pt, FALSE, TRUE);
			}
			//DebugPrint(_T("CurveUpdated\n"));
		}
	}
	theHold.Resume();
	if (doZoom) iCCtrl->ZoomExtents();

}

MasterState* Reactor::getMasterState(int noSlave)
{
	ReactionMaster* master = GetMaster();
	DbgAssert(master);
	if (master)
		return master->GetState(slaveStates[noSlave].masterID);
	else
		return NULL;
}

void Reactor::GetCurveValue(TimeValue t, float* val, Interval &valid)
{
	if (!iCCtrl) BuildCurves();
	if (!iCCtrl) return;

	if (!curvesValid)
		UpdateCurves();

	int numCurves = iCCtrl->GetNumCurves();
	
	ICurve* curve;
	float f;
	for (int i=0;i<numCurves;i++,val++)
	{
		curve = iCCtrl->GetControlCurve(i);
		f = curve->GetValue(0, getCurFloatValue(t), FOREVER);
		*val = f;
	}
}

void Reactor::EditTrackParams(TimeValue t,ParamDimensionBase *dim,TCHAR *pname,
							  HWND hParent,IObjParam *ip,DWORD flags)
{
	//fine to open this dialog though locked, the locking happens in it!
	// open reaction manager dialog on selected objects
	if (GetReactionDlg() == NULL)
		SetReactionDlg(new ReactionDlg(ip, GetCUIFrameMgr()->GetAppHWnd()));
	ShowWindow(GetReactionDlg()->hWnd, SW_SHOWNORMAL);
}

/// *******************************************************
/// Reaction Master implementation
/// *******************************************************

ReactionMaster::ReactionMaster(ReferenceTarget* c, int id)
{ 
	client = NULL;
	ReplaceReference( 0, c); 
	nmaker = NULL;
	subnum = id; 
	states.Init(); 
	if (client != NULL)
	{
		if (subnum < 0)
		{
			if (subnum == IKTRACK)
				type = QUAT_VAR;
			else
				type = VECTOR_VAR;
		}
		else
		{
			switch ( client->SubAnim(subnum)->SuperClassID() )
			{
				case CTRL_FLOAT_CLASS_ID:
					type = FLOAT_VAR; break;
				case CTRL_POINT3_CLASS_ID:
				case CTRL_POSITION_CLASS_ID:
					type = VECTOR_VAR; break;
				case CTRL_SCALE_CLASS_ID:
					type = SCALE_VAR; break;
				case CTRL_ROTATION_CLASS_ID:
					type = QUAT_VAR; break;
				default: type = 0; 
			}
		}
	}
	else type = 0;
}

RefTargetHandle ReactionMaster::Clone(RemapDir& remap)
	{
	ReactionMaster *m = new ReactionMaster();
	m->ReplaceReference(0, remap.CloneRef(client));
	*m = *this;
	BaseClone(this, m, remap);
	return m;
	}


#define MASTER_INDEX_CHUNK			0x5000
#define MASTER_TYPE_CHUNK			0x5001
#define MASTER_STATE_COUNT_CHUNK	0x5002
#define MASTER_STATE_NAME_CHUNK		0x5003
#define MASTER_STATE_POS_CHUNK		0x5004
#define MASTER_STATE_FLOAT_CHUNK	0x5005
#define MASTER_STATE_QUAT_CHUNK		0x5006
#define MASTER_STATE_TYPE_CHUNK		0x5007
#define MASTER_STATE_ENTRY0			0x8000
#define MASTER_STATE_ENTRYN			0x8fff

IOResult ReactionMaster::Save(ISave *isave)
{		
	ULONG nb;
	isave->BeginChunk(MASTER_INDEX_CHUNK);
	isave->Write(&subnum, sizeof(int), &nb);
 	isave->EndChunk();

	isave->BeginChunk(MASTER_TYPE_CHUNK);
	isave->Write(&type, sizeof(int), &nb);
 	isave->EndChunk();

	int count = states.Count();
	isave->BeginChunk(MASTER_STATE_COUNT_CHUNK);
	isave->Write(&count, sizeof(int), &nb);
 	isave->EndChunk();

	for(int i = 0; i< count; i++)
	{
	 	isave->BeginChunk(MASTER_STATE_ENTRY0+i);

	 	isave->BeginChunk(MASTER_STATE_NAME_CHUNK);
		isave->WriteCString(states[i]->name);
 		isave->EndChunk();		
		isave->BeginChunk(MASTER_STATE_POS_CHUNK);
		isave->Write(&states[i]->pvalue, sizeof(Point3), &nb);
		isave->EndChunk();
		isave->BeginChunk(MASTER_STATE_FLOAT_CHUNK);
		isave->Write(&states[i]->fvalue, sizeof(float), &nb);
		isave->EndChunk();
		isave->BeginChunk(MASTER_STATE_QUAT_CHUNK);
		isave->Write(&states[i]->qvalue, sizeof(Quat), &nb);
		isave->EndChunk();
		isave->BeginChunk(MASTER_STATE_TYPE_CHUNK);
		isave->Write(&states[i]->type, sizeof(int), &nb);
		isave->EndChunk();
		

		isave->EndChunk();
	}
	return IO_OK;
}

RefResult ReactionMaster::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID &partID, RefMessage message) 
{ 
	BOOL wasLocked;
	switch(message)
	{
	case REFMSG_TARGET_DELETED:
		wasLocked = TestAFlag(A_LOCK_TARGET);
		if (!wasLocked)
			SetAFlag(A_LOCK_TARGET); // don't let me get deleted from under us
		GetReactionManager()->RemoveReaction(client, subnum);
		NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
		if (!wasLocked)
			ClearAFlag(A_LOCK_TARGET);
		client = NULL;
		return REF_AUTO_DELETE;
	case REFMSG_GET_NODE_NAME:
		return REF_STOP;
	}
	return REF_SUCCEED; 
}

IOResult ReactionMaster::Load(ILoad *iload)
	{
	ULONG nb;
	int id;
	TCHAR	*cp;

	while (iload->OpenChunk()==IO_OK) {
		switch (id = iload->CurChunkID()) {
			case MASTER_INDEX_CHUNK:
				iload->Read(&subnum, sizeof(int), &nb);
				break;
			case MASTER_TYPE_CHUNK:
				iload->Read(&type, sizeof(int), &nb);
				break;
			case MASTER_STATE_COUNT_CHUNK:
				{
					int count = 0;
					iload->Read(&count, sizeof(int), &nb);
					states.SetCount(count);
					for(int i = 0; i < count; i++)
						states[i] = new MasterState(_T(""), 0, NULL);
				}
				break;
		}
		if(id >= MASTER_STATE_ENTRY0 && id <= MASTER_STATE_ENTRYN) {
			int varIndex = id - MASTER_STATE_ENTRY0;
			assert(varIndex < states.Count());
			while (IO_OK == iload->OpenChunk()) {
				switch (iload->CurChunkID()) {
					case MASTER_STATE_NAME_CHUNK:
						iload->ReadCStringChunk(&cp);
						states[varIndex]->name = _T("");
						states[varIndex]->name.printf(_T("%s"), cp);
						if (nmaker == NULL)
							nmaker = GetCOREInterface()->NewNameMaker(FALSE);
						nmaker->AddName(states[varIndex]->name);
						break;
					case MASTER_STATE_FLOAT_CHUNK:
						iload->Read(&states[varIndex]->fvalue, sizeof(float), &nb);
						break;
					case MASTER_STATE_POS_CHUNK:
						iload->Read(&states[varIndex]->pvalue, sizeof(Point3), &nb);
						break;
					case MASTER_STATE_QUAT_CHUNK:
						iload->Read(&states[varIndex]->qvalue, sizeof(Quat), &nb);
						break;
					case MASTER_STATE_TYPE_CHUNK:
						iload->Read(&states[varIndex]->type, sizeof(int), &nb);
						break;

				}	
				iload->CloseChunk();
			}
		}
		iload->CloseChunk();
	}
	return IO_OK;
}

MasterState* ReactionMaster::AddState(TCHAR* buf, int &id, bool setDefaults, TimeValue t)
{
	if (theHold.Holding())
		theHold.Put(new MasterStateListRestore(this));

	TSTR mname(GetString(IDS_AF_VARNAME));
	if (buf == NULL)
	{
		if (!nmaker) 
			nmaker = GetCOREInterface()->NewNameMaker(FALSE);
		nmaker->MakeUniqueName(mname);
	}else mname = buf;


	void* val = NULL;
	float f;
	Point3 p;
	Quat q;

	if (setDefaults)
	{
		switch(type)
		{
		case FLOAT_VAR:
			GetValue(t, &f);
			val = &f;
			break;
		case SCALE_VAR:
		case VECTOR_VAR:
			GetValue(t, &p);
			val = &p;
			break;
		case QUAT_VAR:
			GetValue(t, &q);
			val = &q;
			break;
		}
	}

	MasterState* state = new MasterState(mname, type, val);
	id = states.Count();
	states.Append(1, &state);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_REACTION_COUNT_CHANGED);
	return state;
}

void ReactionMaster::DeleteState(int id)
{
	DependentIterator di(this);
	ReferenceMaker* maker = NULL;
	while ((maker = di.Next()) != NULL)
	{
		Reactor* r = (Reactor*)GetIReactorInterface(maker);
		if (r != NULL)
		{
			for(int x = r->slaveStates.Count()-1; x >= 0; x--)
			{
				if(r->getMasterState(x) == this->GetState(id))
				{
					r->DeleteReaction(x);
					break;
				}
			}
			for(int x = r->slaveStates.Count()-1; x >= 0; x--)
			{
				// this is a separate pass since DeleteReaction stores the entire undo record. 
				if (r->slaveStates[x].masterID > id)
					r->slaveStates[x].masterID--;
			}
			if (theHold.Holding())
				theHold.Put(new MasterIDRestore(r, id));

		}
	}

	if (theHold.Holding())
		theHold.Put(new MasterStateListRestore(this));

	delete states[id];
	states.Delete(id, 1);
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_REACTION_COUNT_CHANGED);
}

void GetAbsoluteControlValue(
		INode *node,int subNum, TimeValue t,void *pt,Interval &iv)
{
	if (node != NULL)
	{
		if (subNum == IKTRACK)
		{
			Matrix3 cur_mat = node->GetNodeTM(t,&iv);
			Matrix3 par_mat =  node->GetParentTM(t);
			Matrix3 relative_matrix = cur_mat * Inverse( par_mat);
			Quat q = Quat(relative_matrix);
			*(Quat*)pt = q;
		}else {	
			Matrix3 tm = node->GetNodeTM(t,&iv);
			*(Point3*)pt = tm.GetTrans();
		}
	}
}

void ReactionMaster::GetValue(TimeValue t, void* val)
{
	Control *c;
	float f = 0.0f;
	Point3 p = Point3(0,0,0);
	Quat q = Quat();
	q.Identity();

	if (subnum < 0 )
	{
		if (subnum == IKTRACK)	
		{
			GetAbsoluteControlValue((INode*)client, subnum, t, &q, FOREVER);
			*(Quat*)val = q;
		}
		else {
			GetAbsoluteControlValue((INode*)client, subnum, t, &p, FOREVER);
			*(Point3*)val = p;
		}
	}
	else if (client) {
		c = (Control *)client->SubAnim(subnum);
		switch (type)
		{
			case FLOAT_VAR:
				c->GetValue(t, &f, FOREVER);
				*(float*)val = f;
				break;
			case VECTOR_VAR:
				c->GetValue(t, &p, FOREVER);
				*(Point3*)val = p;
				break;
			case SCALE_VAR:
				{
				ScaleValue s;
				c->GetValue(t, &s, FOREVER);
				*(Point3*)val = s.s;
				break;
				}
			case QUAT_VAR:
				c->GetValue(t, &q, FOREVER);
				*(Quat*)val = q;
				break;
		}
	}
}

void ReactionMaster::SetState(int i, float val)
{
	if (client) {
	Control *c = (Control *)client->SubAnim(subnum);
	if (type == FLOAT_VAR)
	{
		if (i >=0 && i < states.Count())
		{
			theHold.Put(new MasterStateRestore(this, i));
			states[i]->fvalue = val;
			NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
		else
			throw MAXException(GetString(IDS_NOT_IN_VALID_RANGE));
	}
	else
		throw MAXException(GetString(IDS_NOT_A_VALID_VALUE));
}
}

void ReactionMaster::SetState(int i, Point3 val)
{
	if (client) {
	Control *c = (Control *)client->SubAnim(subnum);
	if (type == VECTOR_VAR || type == SCALE_VAR)
	{
		if (i >=0 && i < states.Count())
		{
			theHold.Put(new MasterStateRestore(this, i));
			states[i]->pvalue = val;
			NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
		else
			throw MAXException(GetString(IDS_NOT_IN_VALID_RANGE));
	}
	else
		throw MAXException(GetString(IDS_NOT_A_VALID_VALUE));
}
}

void ReactionMaster::SetState(int i, Quat val)
{
	if (client) {
	Control *c = (Control *)client->SubAnim(subnum);
	if (type == QUAT_VAR)
	{
		if (i >=0 && i < states.Count())
		{
			theHold.Put(new MasterStateRestore(this, i));
			states[i]->qvalue = val;
			NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
		else
			throw MAXException(GetString(IDS_NOT_IN_VALID_RANGE));
	}
	else
		throw MAXException(GetString(IDS_NOT_A_VALID_VALUE));
}
}

void ReactionMaster::CopyFrom(Reactor* r)
{
	ReactionMaster* from = r->GetMaster();
	for(int i = 0; i < from->states.Count(); i++)
	{
		bool found = false;
		int type =  from->states[i]->type;
		for(int x = 0; x < states.Count(); x++)
		{
			switch (type)
			{
				case FLOAT_VAR:
					if (from->states[i]->fvalue == states[x]->fvalue)
						found = true;
					break;
				case VECTOR_VAR:
				case SCALE_VAR:
					if (from->states[i]->pvalue == states[x]->pvalue)
						found = true;
					break;
				case QUAT_VAR:
					if (from->states[i]->qvalue == states[x]->qvalue)
						found = true;
					break;
			}
			if (found)
			{
				// slave states and master states should have a one-to-one correspondence when loading Pre R7 files
				// this is not guaranteed to be true when fixing R7 beta files.  These may have some problems...
				if (i < r->slaveStates.Count() &&
					r->slaveStates[i].masterID >= from->states.Count())
					r->slaveStates[i].masterID = x;
				break;
			}
		}
		if (!found)
		{
			MasterState* newState = new MasterState(from->states[i]);
			int index = states.Append(1, &newState);
			// slave states and master states should have a one-to-one correspondence when loading Pre R7 files
			// this is not guaranteed to be true when fixing R7 beta files.  These may have some problems...
			if (i < r->slaveStates.Count() &&
				r->slaveStates[i].masterID >= from->states.Count())
				r->slaveStates[i].masterID = index; 
		}
	}
}

class ReactionMasterClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return FALSE; }
	void *			Create(BOOL loading) 
					{ 
						return new ReactionMaster(); 
					}
	const TCHAR *	ClassName() { return GetString(IDS_AF_REACTION_MASTER); }
	SClass_ID		SuperClassID() { return REF_MAKER_CLASS_ID; }
	Class_ID		ClassID() { return REACTION_MASTER_CLASSID; }
	const TCHAR* 	Category() { return _T("");  }

	const TCHAR*	InternalName() { return _T("ReactionMaster"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
	};

static ReactionMasterClassDesc reactionMasterCD;
ClassDesc* GetReactionMasterDesc() { return &reactionMasterCD; }
