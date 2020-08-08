/**********************************************************************
 *<
	FILE: ExposeControllers.cpp

	DESCRIPTION:	SpecialControllers

	CREATED BY: Michael Zyracki

	HISTORY: 

 *>	Copyright (c) 2004, All Rights Reserved.
 **********************************************************************/
#include "ExposeControllers.h"
#include "ExposeTransform.h"

static FloatExposeControlClassDesc FloatExposeControlDesc;
ClassDesc* GetFloatExposeControlDesc() { return &FloatExposeControlDesc; }

static Point3ExposeControlClassDesc Point3ExposeControlDesc;
ClassDesc* GetPoint3ExposeControlDesc() { return &Point3ExposeControlDesc; }

static EulerExposeControlClassDesc EulerExposeControlDesc;
ClassDesc* GetEulerExposeControlDesc() { return &EulerExposeControlDesc; }


BOOL BaseExposeControl::IsReplaceable()
{
	//this should never really happen. what's happening here is that if there's a floating
	//expose control that has an ExposeTransform that's been deleted/corrupted, then it can
	//be replaced. basically due to the unreplaceable controller interface that's been added.
	if(exposeTransform&&exposeTransform->pblock_display&&exposeTransform->pblock_expose
		&&exposeTransform->pblock_info&&exposeTransform->pblock_display
		&&exposeTransform->pblock_output)
		return FALSE;
	else
		return TRUE;
}

//recursive function to put parents in tab.
static void CollectParents(INodeTab &tab,INode *node)
{
	if(node)
	{
		INode *pNode = node->GetParentNode();
		if(pNode&&pNode!=GetCOREInterface()->GetRootNode())
		{
			tab.Append(1,&pNode);
			CollectParents(tab,pNode);
		}
	}
}


//we need to not only check the expose node and reference node to see if there flags are set, but 
//we also need to check their parents since a call on node->GetNodeTM may call node->parent->UpdateTM
//node->parent->parent->UpdateTM.. etc... So all of the parents need to get checked to.
BOOL BaseExposeControl::AreNodesOrParentsInTMUpdate()
{
	//collect expose node parents.
	if(exposeTransform)
	{
		INode *exposeNode = exposeTransform->GetExposeNode();
		if(exposeNode)
		{
			INodeTab nodes;
			nodes.Append(1,&exposeNode);
			CollectParents(nodes,exposeNode);
	
			//simple check to see if referenceNode isn't exposeNodeParent.. if not.. collect them too
			INode *refNode = exposeTransform->GetReferenceNode();
			if(refNode&&refNode!=exposeNode->GetParentNode())
			{
				nodes.Append(1,&refNode);
				CollectParents(nodes,refNode);
			}

			for(int i=0;i<nodes.Count();++i)
			{
				if(nodes[i]->TestAFlag(A_INODE_IN_UPDATE_TM)==TRUE)
					return TRUE;
			}
		}
	}

	return FALSE;
}

void BaseExposeControl::PopupErrorMessage()
{
	if(exposeTransform) //should exist if not and this message is popping up then we're scewed.
	{
		INode *expNode = exposeTransform->GetExposeNode();
		INode *node = exposeTransform->GetMyNode();
		if(expNode&&node)
		{
			TSTR msg; msg.printf(GetString(IDS_ILLEGAL_SELF_REFERENCE),expNode->GetName(),node->GetName(),node->GetName()); 
			if (GetCOREInterface()->GetQuietMode()) 
			{
				 GetCOREInterface()->Log()->LogEntry(SYSLOG_WARN,NO_DIALOG,GetString(IDS_ILLEGAL_CYCLE),msg);
			}	
			else
			{
				// beep or no??MessageBeep(MB_ICONEXCLAMATION); 
				MessageBox(GetCOREInterface()->GetMAXHWnd(),msg,GetString(IDS_ILLEGAL_CYCLE), 
				MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
			}

		}
		//suspend the hold
		BOOL resume =FALSE;
		if(theHold.Holding())
		{
			theHold.Suspend();
			resume =TRUE;
		}
		exposeTransform->SetExposeNode(NULL);
		if(resume)
			theHold.Resume();
	}
}

FloatExposeControl::FloatExposeControl()
{
	ivalid.SetEmpty();
	curVal =0;
}
FloatExposeControl::FloatExposeControl(ExposeTransform *e,int id)
{
	exposeTransform = e;
	paramID = id;
	ivalid.SetEmpty();
	curVal =0;
}
FloatExposeControl::~FloatExposeControl()
{
	DeleteAllRefsFromMe();
}
void FloatExposeControl::Copy(Control *from)
{
	if (from->ClassID()==ClassID())
	{
		exposeTransform = ((FloatExposeControl*)from)->exposeTransform;
		paramID = ((FloatExposeControl*)from)->paramID;
		ivalid = ((FloatExposeControl*)from)->ivalid;
		curVal = ((FloatExposeControl*)from)->curVal;
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
}
void FloatExposeControl::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method)
{
	Update(t);
	valid  &= ivalid; //only for the current time
	if (method==CTRL_RELATIVE)
	{
		float *f = (float *)val;
		*f = *f + curVal;
	}
	else
		*((float*)val) = curVal;

}


void FloatExposeControl::Update(TimeValue t)
{
	if (!ivalid.InInterval(t))
	{
		ivalid = Interval(t,t);
		if(exposeTransform)
		{
			if((evaluating >=2 && t == evaluatingTime)
				||AreNodesOrParentsInTMUpdate()==TRUE)
			{
				PopupErrorMessage();
				return;
			}
			evaluating+=1;
			evaluatingTime = t;
			curVal = exposeTransform->GetExposedFloatValue(t,paramID,this);
			evaluating -=1;
		}
	}
}
//only the ExposeTransform can set it so this does nothing
void FloatExposeControl::SetValue(TimeValue t, void *val, int commit, GetSetMethod method)
{
}

RefTargetHandle FloatExposeControl::Clone(RemapDir& remap)
{
	FloatExposeControl *p = new FloatExposeControl();

	p->exposeTransform =exposeTransform;
	p->paramID =paramID;
	p->ivalid = ivalid;
	p->curVal = curVal;
	return p;
}


//from IUnReplaceableControl
Control * FloatExposeControl::GetReplacementClone()
{
	Control *control = NewDefaultFloatController();
	if(control)
	{
		// set key per frame
		Interval range =GetCOREInterface()->GetAnimRange();
		TimeValue tpf = GetTicksPerFrame();	
		SuspendAnimate();
		AnimateOn();
		float v;
		for(TimeValue t= range.Start(); t<=range.End();t+=tpf)
		{
			GetValue(t,&v,Interval(t,t));
			control->SetValue(t,&v,1,CTRL_ABSOLUTE);
			
		}
	
		ResumeAnimate();
	}
	return control;
}



Point3ExposeControl::Point3ExposeControl()
{
	ivalid.SetEmpty();
	curVal = Point3(0,0,0);
}
Point3ExposeControl::Point3ExposeControl(ExposeTransform *e,int id)
{
	exposeTransform = e;
	paramID = id;
	ivalid.SetEmpty();
	curVal = Point3(0,0,0);
}
Point3ExposeControl::~Point3ExposeControl()
{
	DeleteAllRefsFromMe();
}

void Point3ExposeControl::Copy(Control *from)
{
	if (from->ClassID()==ClassID())
	{
		exposeTransform = ((Point3ExposeControl*)from)->exposeTransform;
		paramID = ((Point3ExposeControl*)from)->paramID;
		ivalid = ((Point3ExposeControl*)from)->ivalid; 
		curVal = ((Point3ExposeControl*)from)->curVal;
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
}

void Point3ExposeControl::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method)
{
	Update(t);
	valid  &= ivalid; //only for the current time
    if (method==CTRL_RELATIVE)
	{	
		Point3 *p = (Point3 *)val;       
		*p = *p + curVal; 
	}
	else
        *((Point3*)val) = curVal;
}

void Point3ExposeControl::Update(TimeValue t)
{
	if (!ivalid.InInterval(t))
	{
		ivalid = Interval(t,t);
		if(exposeTransform)
		{
			if((evaluating >=2 && t == evaluatingTime)
				||AreNodesOrParentsInTMUpdate()==TRUE)
			{
				PopupErrorMessage();
				return;
			}
			evaluating+=1;
			evaluatingTime = t;
			curVal = exposeTransform->GetExposedPoint3Value(t,paramID,this);
			evaluating -=1;
		}
	}
}



//only the ExposeTransform can set it so this does nothing
void Point3ExposeControl::SetValue(TimeValue t, void *val, int commit, GetSetMethod method)
{
}


RefTargetHandle Point3ExposeControl::Clone(RemapDir& remap)
{
	Point3ExposeControl *p = new Point3ExposeControl();

	p->exposeTransform =exposeTransform;
	p->paramID =paramID;
	p->ivalid = ivalid;
	p->curVal = curVal;
	return p;
}

//from IUnReplaceableControl
Control * Point3ExposeControl::GetReplacementClone()
{
	Control *control = NewDefaultPoint3Controller();
	if(control)
	{
		// set key per frame
		Interval range =GetCOREInterface()->GetAnimRange();
		TimeValue tpf = GetTicksPerFrame();	
		SuspendAnimate();
		AnimateOn();
		Point3 v;
		for(TimeValue t= range.Start(); t<=range.End();t+=tpf)
		{
			GetValue(t,&v,Interval(t,t));
			control->SetValue(t,&v,1,CTRL_ABSOLUTE);
		}
	
		ResumeAnimate();
	}
	return control;
}

//euler expose control

EulerExposeControl::EulerExposeControl()
{
	ivalid.SetEmpty();
	curVal = Quat();
}
EulerExposeControl::EulerExposeControl(ExposeTransform *e,int id)
{
	exposeTransform = e;
	paramID = id;
	ivalid.SetEmpty();
	curVal = Quat();
}
EulerExposeControl::~EulerExposeControl()
{
	DeleteAllRefsFromMe();
}

void EulerExposeControl::Copy(Control *from)
{
	if (from->ClassID()==ClassID())
	{
		exposeTransform = ((EulerExposeControl*)from)->exposeTransform;
		paramID = ((EulerExposeControl*)from)->paramID;
		ivalid = ((EulerExposeControl*)from)->ivalid;
		curVal = ((EulerExposeControl*)from)->curVal;
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
}

void EulerExposeControl::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method)
{
	Update(t);
	valid  &= ivalid; //only for the current time
	if (method==CTRL_RELATIVE)
	{	
		Matrix3 *mat = (Matrix3*)val;       
		PreRotateMatrix(*mat,curVal); 
	}
	else
		*((Quat*)val) = curVal;
}

void EulerExposeControl::Update(TimeValue t)
{
	if (!ivalid.InInterval(t))
	{
		ivalid = Interval(t,t);
		if(exposeTransform)
		{
			if((evaluating >=2 && t == evaluatingTime)
				||AreNodesOrParentsInTMUpdate()==TRUE)
			{
				PopupErrorMessage();
				return;
			}
			evaluating+=1;
			evaluatingTime = t;
			curVal = exposeTransform->GetExposedEulerValue(t,paramID,this);
		
			evaluating -=1;
		}
	}
}

//only the ExposeTransform can set it so this does nothing
void EulerExposeControl::SetValue(TimeValue t, void *val, int commit, GetSetMethod method)
{
}


RefTargetHandle EulerExposeControl::Clone(RemapDir& remap)
{
	EulerExposeControl *p = new EulerExposeControl();

	p->exposeTransform =exposeTransform;
	p->paramID =paramID;
	p->ivalid = ivalid;
	p->curVal = curVal;
	return p;
}

//from IUnReplaceableControl
Control * EulerExposeControl::GetReplacementClone()
{
	Control *control = NewDefaultRotationController();
	if(control)
	{
		// set key per frame
		Interval range =GetCOREInterface()->GetAnimRange();
		TimeValue tpf = GetTicksPerFrame();	
		SuspendAnimate();
		AnimateOn();
		Quat v;
		for(TimeValue t= range.Start(); t<=range.End();t+=tpf)
		{
			GetValue(t,&v,Interval(t,t));
			control->SetValue(t,&v,1,CTRL_ABSOLUTE);
		}
	
		ResumeAnimate();
	}
	return control;
}

