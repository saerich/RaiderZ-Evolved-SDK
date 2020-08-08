/**********************************************************************
 *<
	FILE: jiggle.cpp

	DESCRIPTION:	A procedural Mass/Spring Position controller 
					Main source body

	CREATED BY:		Adam Felt

	HISTORY: 

 *>	Copyright (c) 1999-2000, All Rights Reserved.
 **********************************************************************/

#include "max.h"
#include "IPathConfigMgr.h"
#include "jiggle.h"
#include "springs.h"
#include "NodeSearchEnumProc.h"

//  Mixin Interface stuff
FPInterfaceDesc* IJiggle::GetDesc()
{
	return &spring_controller_interface;
}

//-----------------------------------------------------------------------
float Distance(Point3 p1, Point3 p2)
{
	p1 = p2-p1;
	return fabs((float)sqrt((p1.x*p1.x)+(p1.y*p1.y)+(p1.z*p1.z)));
}

Point3 p3abs(Point3 pt)
{
	Point3 p;
	p.x = fabs(pt.x);
	p.y = fabs(pt.y);
	p.z = fabs(pt.z);
	return p;
}

//-----------------------------------------------------------------------


ClassDesc* GetPoint3JiggleDesc() {return &point3JiggleDesc;}
ClassDesc* GetPosJiggleDesc() {return &PosJiggleDesc;}

//TODO: Should implement this method to reset the plugin params when Max is reset
void PosJiggleClassDesc::ResetClassParams (BOOL /*fileReset*/) 
{
	
}

//TODO: Should implement this method to reset the plugin params when Max is reset
void Point3JiggleClassDesc::ResetClassParams (BOOL /*fileReset*/) 
{

}

IObjParam		*Jiggle::ip					= NULL;
IObjParam		*JiggleDlg::ip				= NULL;
IRollupWindow	*JiggleDlg::TVRollUp		= NULL;

INT_PTR CALLBACK JiggleDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


void Jiggle::BeginEditParams(
		IObjParam *ip, ULONG flags,Animatable *prev)
{
	if(GetLocked()==false)
	{
		if (flags&BEGIN_EDIT_MOTION) 
		{
			this->ip = ip;
			posCtrl->BeginEditParams(ip, flags, NULL);
			PosJiggleDesc.BeginEditParams(ip, this, flags, prev);
			jig_param_blk.SetUserDlgProc(new DynMapDlgProc(this));
			jig_param_blk.ParamOption(jig_control_node,p_validator,&node_validator);

			jig_force_param_blk.SetUserDlgProc( new ForceMapDlgProc(this));
			jig_force_param_blk.ParamOption(jig_force_node,p_validator,&validator);
		}
	}
}

void Jiggle::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	if(this->ip!=NULL) //if not true then no params
	{
		posCtrl->EndEditParams(ip,flags,NULL);
		hParams1 = hParams2 = NULL; 
		//PosJiggleDesc.EndEditParams(ip, this, flags, next);
		PosJiggleDesc.EndEditParams(ip, this, flags | END_EDIT_REMOVEUI, next);
		this->ip = NULL;
	}
}

TSTR Jiggle::SvGetRelTip(IGraphObjectManager *gom, IGraphNode *gNodeTarger, int /*id*/, IGraphNode *gNodeMaker)
{
	return SvGetName(gom, gNodeMaker, false) + " -> " + gNodeTarger->GetAnim()->SvGetName(gom, gNodeTarger, false);
}

bool Jiggle::SvHandleRelDoubleClick(IGraphObjectManager *gom, IGraphNode* /*gNodeTarget*/, int /*id*/, IGraphNode *gNodeMaker)
{
	return Control::SvHandleDoubleClick(gom, gNodeMaker);
}

SvGraphNodeReference Jiggle::SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags)
{
	SvGraphNodeReference nodeRef = Control::SvTraverseAnimGraph( gom, owner, id, flags );

	if( nodeRef.stat == SVT_PROCEED ) {
		INode *pNode = NULL;
		if( dyn_pb ) {
			for( int i=0; i<dyn_pb->Count(jig_control_node); i++ )
			{
				Interval tempTime = FOREVER;
				dyn_pb->GetValue(jig_control_node, 0, pNode, tempTime, i);
				if( pNode )
					gom->AddRelationship( nodeRef.gNode, pNode, i, RELTYPE_CONTROLLER );
			}
		}
		if( force_pb ) {
			for( int i=0; i<force_pb->Count(jig_force_node); i++ )
			{
				Interval tempTime = FOREVER;
				force_pb->GetValue(jig_force_node, 0, pNode, tempTime, i);
				if( pNode )
					gom->AddRelationship( nodeRef.gNode, pNode, i, RELTYPE_CONTROLLER );
			}
		}
	}

	return nodeRef;
}

/*
class RangeRestore : public RestoreObj {
	public:
		Jiggle *cont;
		Interval ur, rr;
		RangeRestore(Jiggle *c) 
			{
			cont = c;
			ur   = cont->range;
			}   		
		void Restore(int isUndo) 
			{
			rr = cont->range;
			cont->range = ur;
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		void Redo()
			{
			cont->range = rr;
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}		
		void EndHold() 
			{ 
			cont->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T("Jiggle control range")); }
	};


void Jiggle::HoldRange()
{
	if (theHold.Holding() && !TestAFlag(A_HELD)) {
		SetAFlag(A_HELD);
		theHold.Put(new RangeRestore(this));
		}
}
*/
//#define PARTICLES_CHUCK	100
#define LOCK_CHUNK						0x2535  //the lock value
IOResult Jiggle::Save(ISave *isave)
{ 
	IOResult res;
	res = partsys->Save(isave);
	//AF (5/24/02) #427804: I have no ideas why this was here...
	//partsys->Invalidate();
	//ivalid.SetEmpty();
	ULONG nb;
	int on = (mLocked==true) ? 1 :0;
	isave->BeginChunk(LOCK_CHUNK);
	isave->Write(&on,sizeof(on),&nb);
	isave->EndChunk();
	return res;
}

IOResult Jiggle::Load(ILoad *iload)
{ 
	IOResult	res;
	res = partsys->Load(iload);
	if (res!=IO_OK) return res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk()))
	{
		ULONG curID = iload->CurChunkID();
		if (curID==LOCK_CHUNK)
		{
			int on;
			res=iload->Read(&on,sizeof(on),&nb);
			if(on)
				mLocked = true;
			else
				mLocked = false;
		}
		iload->CloseChunk();
	};

	partsys->Invalidate();
	return IO_OK;
}


Jiggle::Jiggle(int ctrlType, BOOL loading)
{
	TimeValue t;
	type = ctrlType;
	validator.cont = this;
	node_validator.cont = this;
	validStart = false;
	posCtrl = NULL;
	dyn_pb = force_pb = NULL;
	dlg = NULL;
	pmap = NULL;
	ivalid.SetEmpty();
	selfNode = NULL;
	pickNodeMode = NULL;
	hParams1 = hParams2 = NULL;
	flags = 0;

	//initialize the particle system
	partsys = new SpringSys(this, 1);

	if (!loading)
	{
		Control *cont;
		if (type == JIGGLEPOS) 
		{
			ClassDesc *desc = GetDefaultController(CTRL_POSITION_CLASS_ID);
			//we don't want it to be another spring controller
			if (desc && desc->ClassID()==JIGGLE_POS_CLASS_ID) 
				cont = (Control*)CreateInstance(CTRL_POSITION_CLASS_ID, Class_ID(HYBRIDINTERP_POSITION_CLASS_ID,0));			
			else 
				cont = NewDefaultPositionController();
		}
		else
		{
			ClassDesc *desc = GetDefaultController(CTRL_POINT3_CLASS_ID);
			if (desc && desc->ClassID()==JIGGLE_P3_CLASS_ID) 
				cont = (Control*)CreateInstance(CTRL_POINT3_CLASS_ID, Class_ID(HYBRIDINTERP_POINT3_CLASS_ID,0));			
			else 
				cont = NewDefaultPoint3Controller();
			

		}
		ReplaceReference(JIGGLE_CONTROL_REF,cont);
	}
	if (type == JIGGLEPOS) 
		GetPosJiggleDesc()->MakeAutoParamBlocks(this);
	else
		GetPoint3JiggleDesc()->MakeAutoParamBlocks(this);

	force_pb->GetValue(jig_start,0,t,ivalid);
	partsys->SetReferenceTime(t);
	ivalid.SetEmpty();
}

Jiggle::~Jiggle()
{
	DeleteAllRefsFromMe();
	delete partsys;
	dyn_pb = force_pb = NULL;
	hParams1 = hParams2 = NULL;
	dlg = NULL;
	pickNodeMode = NULL;
}

/*
void Jiggle::EditTimeRange(Interval range,DWORD flags)
{
	if(!(flags&EDITRANGE_LINKTOKEYS)){
		HoldRange();
		this->range = range;
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}
}

void Jiggle::MapKeys(TimeMap *map,DWORD flags)
	{
	if (flags&TRACK_MAPRANGE) {
		HoldRange();
		TimeValue t0 = map->map(range.Start());
		TimeValue t1 = map->map(range.End());
		range.Set(t0,t1);
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
	}
*/


RefTargetHandle Jiggle::GetReference(int i)
	{
	switch (i) {
		case JIGGLE_CONTROL_REF: return (RefTargetHandle)posCtrl;
		case JIGGLE_PBLOCK_REF1: return (RefTargetHandle)dyn_pb;
		case JIGGLE_PBLOCK_REF2: return (RefTargetHandle)force_pb;
		default: assert(0);
		}
	return NULL;
	}

void Jiggle::SetReference(int i, RefTargetHandle rtarg)
	{
	switch (i) {
		case JIGGLE_CONTROL_REF: posCtrl = (Control*)rtarg; break;
		case JIGGLE_PBLOCK_REF1: dyn_pb = (IParamBlock2*)rtarg; break;
		case JIGGLE_PBLOCK_REF2: force_pb = (IParamBlock2*)rtarg; break;
		default: assert(0);
		}
	if (( i == JIGGLE_PBLOCK_REF1 || i == JIGGLE_PBLOCK_REF2) && rtarg == NULL && dlg )
		PostMessage(dlg->hWnd,WM_CLOSE,0,0);
	}

void Jiggle::MouseCycleStarted(TimeValue )
{
	if (GetCOREInterface8()->GetSpringQuickEditMode()) {
		SpringSysGlobalSettings::SetAlwaysAccurate(false);
	}
}

void Jiggle::MouseCycleCompleted(TimeValue )
{
	if (GetCOREInterface8()->GetSpringQuickEditMode()) {
		SpringSysGlobalSettings::SetAlwaysAccurate();

		// Clear our cache
		partsys->Invalidate();
		ivalid.SetEmpty();
		validStart = FALSE;

		// I'd rather not have to notify, but in some cases there is no
		// evaluation triggered after a MouseCycleCompleted
		NotifyDependents(FOREVER, (PartID)PART_ALL, REFMSG_CHANGE);
	}
}

RefResult Jiggle::NotifyRefChanged(
		Interval /*iv*/, 
		RefTargetHandle hTarg, 
		PartID& /*partID*/, 
		RefMessage msg) 
{
	INode* node = NULL;
	int i;

	switch (msg) {
		//this gets sent when an object is moved, play is pressed, or the time slider is clicked on.
		//so far it seems to work good for triggering when an object might change.  
		//Not called as often as a RedrawViews notification
		case REFMSG_WANT_SHOWPARAMLEVEL:
			if (SuperClassID() == CTRL_POINT3_CLASS_ID) 
			{
				validStart = FALSE;
				partsys->Invalidate();
				ivalid.SetEmpty();
			}
			break;
		//case REFMSG_FLAGDEPENDENTS:
		case REFMSG_CHANGE:
		{
			if (!dyn_pb || !force_pb) break;
			if (hTarg == dyn_pb && dyn_pb->LastNotifyParamID() == jig_control_node)
			{
				UpdateNodeList();
			}
			partsys->Invalidate();
			ivalid.SetEmpty();
			validStart = FALSE;
		}
		break;
		case REFMSG_TAB_ELEMENT_NULLED:
		case REFMSG_REF_DELETED:
			if (hTarg == dyn_pb && dyn_pb->LastNotifyParamID() == jig_control_node)
			{
				for(i=dyn_pb->Count(jig_control_node)-1;i>0;i--)
				{
					Interval tempTime = FOREVER;
					dyn_pb->GetValue(jig_control_node, 0, node, tempTime, i);
					if (node == NULL) 
					{
//						theHold.Suspend();
						RemoveSpring(i);
//						theHold.Resume();
					}
				}
			}
			else if (hTarg == force_pb && force_pb->LastNotifyParamID() == jig_force_node && msg != REFMSG_REF_DELETED)
			{
				for(i=force_pb->Count(jig_force_node)-1;i>=0;i--)
				{
					Interval tempTime = FOREVER;
					force_pb->GetValue(jig_force_node, 0, node, tempTime, i);
					if (node == NULL) 
					{
						force_pb->Delete(jig_force_node, i, 1);
					}
				}
			}
			break;
		case REFMSG_GET_CONTROL_DIM: 
			break;
		}
	return REF_SUCCEED;
}


BOOL Jiggle::AssignController(Animatable *control,int subAnim)
{
	if(GetLocked()==false)
	{
		if (control->ClassID()==ClassID() || subAnim != JIGGLE_CONTROL_REF) return FALSE;
		//make sure the jiggle control isn't locked.
		if(posCtrl&&GetLockedTrackInterface(posCtrl)&&GetLockedTrackInterface(posCtrl)->GetLocked()==true)
			return FALSE;
		ReplaceReference(JIGGLE_CONTROL_REF,(ReferenceTarget*)control);
		NotifyDependents(FOREVER,0,REFMSG_CHANGE);
		NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
		return TRUE;
	}
	return FALSE;
}

Jiggle& Jiggle::operator=(const Jiggle& from)
{
	type		= from.type;
	ctrlValid	= from.ctrlValid;
	range		= from.range;
	flags		= 0;

	initState.Init();
	for (int i=0;i<from.initState.Count();i++)
	{
		Matrix3 mat = from.initState[i];
		initState.Append(1, &mat);
	}
	partsys->Copy(from.partsys);
	partsys->Invalidate();
	validStart = FALSE;

	return *this;
}

void Jiggle::Copy(Control *from)
{
	if(GetLocked()==false)
	{
		//you shoulg get a fresh new Spring controller when replacing an existing one
		//no longer copying the existing one  (there was a potential for bugs here anyway because I don't think this was ever tested)
		
		if (from->ClassID()==ClassID()) {
			mLocked = ((Jiggle*)from)->mLocked;
			ReplaceReference(JIGGLE_CONTROL_REF,CloneRefHierarchy(((Jiggle*)from)->posCtrl));
			if (((Jiggle*)from)->dlg && ((Jiggle*)from)->dlg->hWnd)
				PostMessage(((Jiggle*)from)->dlg->hWnd,WM_CLOSE,0,0);
		} else { 
			if(from&&GetLockedTrackInterface(from))
			  mLocked = GetLockedTrackInterface(from)->GetLocked();
			if (from->ClassID() == Class_ID(DUMMY_CONTROL_CLASS_ID,0))
				ReplaceReference(JIGGLE_CONTROL_REF, (ReferenceTarget*)(GetDefaultController(this->SuperClassID())->Create(false)));
			else ReplaceReference(JIGGLE_CONTROL_REF,(ReferenceTarget*)CloneRefHierarchy(from));
			}
		NotifyDependents(FOREVER,0,REFMSG_CHANGE);
		NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	}
}

RefTargetHandle PosJiggle::Clone(RemapDir& remap)
{
	Jiggle *cont = new PosJiggle(true);
	*cont = *this;
	((PosJiggle*)cont)->mLocked= mLocked;
	cont->ReplaceReference(JIGGLE_CONTROL_REF, remap.CloneRef(posCtrl));
	cont->ReplaceReference(JIGGLE_PBLOCK_REF1,remap.CloneRef(dyn_pb));
	cont->ReplaceReference(JIGGLE_PBLOCK_REF2,remap.CloneRef(force_pb));

	CloneControl(cont,remap);
	BaseClone(this, cont, remap);
	return cont;
}	


RefTargetHandle Point3Jiggle::Clone(RemapDir& remap)
{
	Jiggle *cont = new Point3Jiggle(true);
	*cont = *this;
	((Point3Jiggle*)cont)->mLocked= mLocked;
	cont->ReplaceReference(JIGGLE_CONTROL_REF, remap.CloneRef(posCtrl));
	cont->ReplaceReference(JIGGLE_PBLOCK_REF1,remap.CloneRef(dyn_pb));
	cont->ReplaceReference(JIGGLE_PBLOCK_REF2,remap.CloneRef(force_pb));

	CloneControl(cont,remap);
	BaseClone(this, cont, remap);
	return cont;
}

void Jiggle::HoldAll()
{
	if (theHold.Holding()) 
		theHold.Put(new SpringRestore(this));
}


Animatable* Jiggle::SubAnim(int i) 
{
	switch (i) { 
	case JIGGLE_CONTROL_REF: return posCtrl; break;
	case JIGGLE_PBLOCK_REF1: return dyn_pb; break;
	case JIGGLE_PBLOCK_REF2: return force_pb; break;
	default: return NULL;}
}

TSTR Jiggle::SubAnimName(int i) 
{ 
	switch (i) {
	case JIGGLE_CONTROL_REF: return GetString(IDS_POS_CONTROL);
	case JIGGLE_PBLOCK_REF1: return GetString(IDS_DYN_PARAMS); 
	case JIGGLE_PBLOCK_REF2: return GetString(IDS_FORCE_PARAMS);
	default: return _T("");
	}
}
		
int Jiggle::SubNumToRefNum(int subNum) {return subNum;}

//don't check lock for this
BOOL Jiggle::ChangeParents(TimeValue t,const Matrix3& oldP,const Matrix3& newP,const Matrix3& tm)
{
	flags &= ~HAS_NO_PARENT;
	if ( dyn_pb->Count(jig_control_node) )
		dyn_pb->SetValue(jig_control_node, 0, ((INode*)NULL), 0);
	partsys->Invalidate();
	validStart = false;
	posCtrl->ChangeParents(t, oldP, newP, tm);
	return TRUE;
}


Point3 Jiggle::ComputeValue(TimeValue t)
{
	int steps;
	float xeffect, yeffect, zeffect;
	Interval for_ever = FOREVER;
	force_pb->GetValue(jig_tolerence,0,steps,for_ever);
	force_pb->GetValue(jig_xeffect,t,xeffect,ivalid);
	force_pb->GetValue(jig_yeffect,t,yeffect,ivalid);
	force_pb->GetValue(jig_zeffect,t,zeffect,ivalid);

	//Set the start time.  This doesn't need to be done here.
	int start; 
	int tpf = GetTicksPerFrame();
	for_ever = FOREVER;
	force_pb->GetValue(jig_start,0,start,for_ever);
	partsys->SetReferenceTime((float)start * tpf);
	int ReferenceFrame = (float)start * tpf;

	if ( t >= ReferenceFrame && steps != 0 && !(xeffect == 0.0f && yeffect == 0.0f && zeffect == 0.0f))
	{
		int DeltaT = tpf/(pow(2.0f, steps));
		
		//Get the bones from the Paramblock and store them
		INodeTab bones;
		INode *bone;
		for (int z=0;z<dyn_pb->Count(jig_control_node);z++)
		{
			for_ever = FOREVER;
			dyn_pb->GetValue(jig_control_node, t, bone, for_ever, z);
			bones.Append(1, &bone);
		}
		
		if (!validStart)
		{
			// Use the rolling start, but don't go before ReferenceFrame
			TimeValue startTime = SpringSysGlobalSettings::IsAlwaysAccurate()?
				ReferenceFrame: max(ReferenceFrame, 
					t - (int)GetCOREInterface8()->GetSpringRollingStart()*tpf);
			partsys->SetInitialPosition (GetCurrentTM(startTime).GetTrans(), 0); 
			partsys->SetInitialVelocity (Point3::Origin, 0);
			initState = GetForceMatrices(startTime);
			partsys->SetInitialBoneStates(initState);
			validStart = true;
		}

		partsys->Solve(t, DeltaT);	
		
		Point3 curval = partsys->GetParticle(0)->GetPos();

		// dampen the X, Y, and Z effects
		if (xeffect != 100.0f || yeffect != 100.0f || zeffect != 100.0f)
		{
			curval -= GetCurrentTM(t).GetTrans();
			curval.x *= (xeffect/100.0f);
			curval.y *= (yeffect/100.0f);
			curval.z *= (zeffect/100.0f);
			curval += GetCurrentTM(t).GetTrans();
		}

		return curval;
	} 
	return GetCurrentTM(t).GetTrans();
}

BOOL TestForSubLoop(Animatable* anim, Control* ctrl)
{
	if (anim == ctrl)
		return TRUE;
	else {
		for (int i = 0;i< anim->NumSubs();i++) {
			if (anim->SubAnim(i) == ctrl)
				return TRUE;
			else if (TestForSubLoop(anim->SubAnim(i), ctrl))
				return TRUE;
		}
	}
	return FALSE;
}

BOOL Jiggle::SetSelfReference()
{	
	//Find out who you are assigned to
	if ( !(flags & HAS_NO_PARENT) && 
		(!dyn_pb->Count(jig_control_node) || 
		  dyn_pb->GetINode(jig_control_node, 0, 0) == NULL || 
		selfNode == NULL) )
	{
		dyn_pb->EnableNotifications(FALSE);

		NodeSearchEnumProc dep(this);
		DoEnumDependents(&dep);

		selfNode = dep.GetNode();

		if (selfNode == NULL) 
		{
			selfNode = GetCOREInterface()->GetRootNode();
		}

		if (!selfNode->IsRootNode() && this->SuperClassID() == CTRL_POSITION_CLASS_ID )
		{
			selfNode = selfNode->GetParentNode();
		}
		
		//check for an IK controller and iterate through parents until you don't have one
		INode* tempNode = selfNode;
		while (!tempNode->IsRootNode() && tempNode->GetTMController()->GetInterface(I_IKCONTROL))
		{
			tempNode = tempNode->GetParentNode();
		}

		//if this is a Point3 controller you cannot make a reference to it so...
		if (this->SuperClassID() == CTRL_POINT3_CLASS_ID )
		{
			tempNode = GetCOREInterface()->GetRootNode();
			while (selfNode && !selfNode->IsRootNode() && TestForSubLoop(selfNode->GetTMController(), this))
				selfNode = selfNode->GetParentNode();
		}

		if (!dyn_pb->Count(jig_control_node) )
			dyn_pb->Append(jig_control_node, 1, &tempNode);
		else
		{
			dyn_pb->Delete(jig_control_node, 0, 1);
			dyn_pb->Insert(jig_control_node, 0, 1, &tempNode);
			//dyn_pb->SetValue(jig_control_node, 0, ((INode*)selfNode), 0);
		}
		

		if (tempNode->IsRootNode()) 
		{
			dyn_pb->SetValue(jig_control_node, 0, ((INode*)NULL), 0); 
			if (selfNode->IsRootNode())
				selfNode = NULL;
			flags |= HAS_NO_PARENT;
		}

		//Add the selfInfluence spring
		if ( !partsys->GetParticle(0)->GetSprings()->Count() )
		{
			//create a new constraint for the spring
			SSConstraintPoint* newbone = new SSConstraintPoint(); 
			newbone->SetIndex(0);
			newbone->SetPos( GetCurrentTM(partsys->GetReferenceTime()).GetTrans() );
			newbone->SetVel( Point3(0.0f,0.0f,0.0f) );
			Point3 length = Point3(0.0f,0.0f,0.0f);
			//add the spring to the particle
			partsys->GetParticle(0)->AddSpring(newbone, length, JIGGLE_DEFAULT_TENSION, JIGGLE_DEFAULT_DAMPENING);
			validStart = false;
		}
		dyn_pb->EnableNotifications(TRUE);

	}
	return TRUE;
}
void Point3Jiggle::GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method)
{
	SetSelfReference();
	Point3 curval = ComputeValue(t);
	ivalid.SetInstant(t);
	valid &= ivalid;
	if (method==CTRL_RELATIVE) 
	{
		*((Point3*)val) += curval;
	} else 
	{
		*((Point3*)val) = curval;
	}
}


void PosJiggle::GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method)
{
	ivalid.SetInfinite();
	Point3 curval;
	SetSelfReference();
	curval = ComputeValue(t);
	ivalid.SetInstant(t);
	valid &= ivalid;
	if (method==CTRL_RELATIVE) 
	{
  		Matrix3 *mat = (Matrix3*)val;	
		mat->SetTrans(curval);
	}
	else 
	{
		*((Point3*)val) = curval;
	}
}


void Jiggle::SetValue(TimeValue t, void *val, int commit, GetSetMethod method)
{
	if(GetLocked()==false)
	{
		validStart = false;
		if (t <= partsys->GetReferenceTime()) 
			partsys->Invalidate();
		posCtrl->SetValue(t, val, commit, method); 
	}
}
