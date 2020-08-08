/**********************************************************************
 *<
	FILE: jiggleFunctions.cpp

	DESCRIPTION:	A procedural Mass/Spring Position controller 
					Additional functions for the Spring controller

	CREATED BY:		Adam Felt

	HISTORY: 

 *>	Copyright (c) 1999-2000, All Rights Reserved.
 **********************************************************************/

#include "jiggle.h"


void Jiggle::SetTension(int index, float tension, int absolute, bool update)
{
	if (index < 0 || index >= dyn_pb->Count(jig_control_node))
	{
		MAXException(GetString(IDS_OUT_OF_RANGE));
		return;
	}
	
	for (int b=0;b<partsys->GetParticle(0)->GetSprings()->Count();b++)
	{
		if (index == partsys->GetParticle(0)->GetSpring(b)->GetPointConstraint()->GetIndex())
		{
			if (absolute == 0) 
				tension += partsys->GetParticle(0)->GetSpring(b)->GetTension();
			partsys->GetParticle(0)->GetSpring(b)->SetTension(tension);
		}
	}
	partsys->Invalidate();
	if (update)
	{
		UpdateNodeList();
		NotifyDependents(FOREVER, (PartID)PART_ALL, REFMSG_CHANGE);
		if(ip) ip->RedrawViews(ip->GetTime());
	}
}

void Jiggle::SetMass(float mass, bool update)
{
	partsys->GetParticle(0)->SetMass(mass);
	partsys->Invalidate();
	if (update)
	{
		NotifyDependents(FOREVER, (PartID)PART_ALL, REFMSG_CHANGE);
		if(ip) ip->RedrawViews(ip->GetTime());
	}
}

void Jiggle::SetDampening(int index, float dampening, int absolute, bool update)
{
	if (index < 0 || index >= dyn_pb->Count(jig_control_node))
	{
		MAXException(GetString(IDS_OUT_OF_RANGE));
		return;
	}

	for (int b=0;b<partsys->GetParticle(0)->GetSprings()->Count();b++)
	{
		if (index == partsys->GetParticle(0)->GetSpring(b)->GetPointConstraint()->GetIndex())
		{
			if (absolute == 0) 
				dampening += partsys->GetParticle(0)->GetSpring(b)->GetDampening();
			partsys->GetParticle(0)->GetSpring(b)->SetDampening(dampening);
		}
	}
	partsys->Invalidate();
	if (update)
	{
		UpdateNodeList();
		NotifyDependents(FOREVER, (PartID)PART_ALL, REFMSG_CHANGE);
		if(ip) ip->RedrawViews(ip->GetTime());
	}
}

void Jiggle::SetDrag(float drag, bool update)
{
	partsys->GetParticle(0)->SetDrag(drag);
	partsys->Invalidate();
	if (update)
	{
		NotifyDependents(FOREVER, (PartID)PART_ALL, REFMSG_CHANGE);
		if(ip) ip->RedrawViews(ip->GetTime());

	}
}

float Jiggle::GetMass()
{
	return partsys->GetParticle(0)->GetMass();
}

float Jiggle::GetDrag()
{
	return partsys->GetParticle(0)->GetDrag();
}

float Jiggle::GetTension(int index)
{
	if (index >=0 && index < dyn_pb->Count(jig_control_node))
		return partsys->GetParticle(0)->GetSpring(index)->GetTension();
	MAXException(GetString(IDS_OUT_OF_RANGE));
	return 0;
}

float Jiggle::GetDampening(int index)
{
	if (index >=0 && index < dyn_pb->Count(jig_control_node))
		return partsys->GetParticle(0)->GetSpring(index)->GetDampening();
	MAXException(GetString(IDS_OUT_OF_RANGE));
	return 0;
}

BOOL Jiggle::AddSpring(INode *node)
{
	if (node) 
	{
		theHold.Begin();
		HoldAll();

		int start;
		Interval for_ever = FOREVER;
		force_pb->GetValue(jig_start, 0, start, for_ever);

		int ct = dyn_pb->Count(jig_control_node);
		
		Point3 initPos = GetCurrentTM(start).GetTrans();
		//create a new bone for the spring
		SSConstraintPoint * bone = new SSConstraintPoint(); //spring->GetBone();
		bone->SetIndex(ct);
		bone->SetPos( initPos ); //node->GetNodeTM(start).GetTrans() );
		bone->SetVel( Point3(0,0,0) );
		
		//the particles world space pos - the bones world space pos
		Point3 length = initPos * Inverse(node->GetNodeTM(start));
		
		//add the spring to the particle
		partsys->GetParticle(0)->AddSpring(bone, length, JIGGLE_DEFAULT_TENSION, JIGGLE_DEFAULT_DAMPENING);
		dyn_pb->Append(jig_control_node, 1, &node);

		theHold.Accept(GetString(IDS_UNDO_ADD_SPRING));

		partsys->Invalidate();
		validStart = false;
		UpdateNodeList();
		if (dlg) SendDlgItemMessage(dlg->dynDlg, IDC_LIST2, LB_SETCURSEL, ct, 0);
		if (hParams1) SendDlgItemMessage(hParams1, IDC_LIST2, LB_SETCURSEL, ct, 0);


		NotifyDependents(FOREVER, (PartID)PART_ALL, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		

		return TRUE;
	}
	return FALSE;
}

int Jiggle::GetSpringCount( )
{
	return dyn_pb->Count(jig_control_node);
}

void Jiggle::RemoveSpring(int sel)
{
	
 
	if (sel == 0) 
	{
		MAXException("Can't remove the first spring");
		return;
	}else if (sel >= GetSpringCount() || sel < 0 )
		MAXException(GetString(IDS_OUT_OF_RANGE));

	Interface* my_ip = GetCOREInterface();
	
	if (theHold.Holding()) HoldAll();

	partsys->GetParticle(0)->DeleteSpring(sel);
	dyn_pb->Delete(jig_control_node, sel, 1);
	
	partsys->Invalidate();
	validStart = false;
	UpdateNodeList();
	NotifyDependents(FOREVER, (PartID)PART_ALL, REFMSG_CHANGE);
	my_ip->RedrawViews(my_ip->GetTime());
}

void Jiggle::RemoveSpring(INode* node)
{
	if (!node) return;
	Interface* my_ip = GetCOREInterface();
	
	theHold.Begin();
	HoldAll();
	int exists = 0;

   int i;
	for (i = 1; i< dyn_pb->Count(jig_control_node); i++)
	{
		INode* ctrlNode = NULL;
		Interval for_ever = FOREVER;
		dyn_pb->GetValue(jig_control_node, 0, ctrlNode, for_ever, i);
		if ( ctrlNode == node)
		{
			exists = 1;
			break;
		}
	}
	if (!exists) { theHold.Cancel(); return; }

	partsys->GetParticle(0)->DeleteSpring(i);
	dyn_pb->Delete(jig_control_node, i, 1);
	
	theHold.Accept(GetString(IDS_UNDO_REMOVE_SPRING));

	partsys->Invalidate();
	validStart = false;
	UpdateNodeList();
	NotifyDependents(FOREVER, (PartID)PART_ALL, REFMSG_CHANGE);
	my_ip->RedrawViews(my_ip->GetTime());
}



Matrix3 Jiggle::GetCurrentTM(TimeValue t)
{
	INode* baseNode = NULL;
	baseNode = selfNode;

	Matrix3 selfTM;
	Point3 pos = Point3(0.0f,0.0f,0.0f);
	Interval for_ever = FOREVER;
	posCtrl->GetValue(t, pos, for_ever, CTRL_ABSOLUTE);
	if (baseNode != NULL )
	{
		selfTM = baseNode->GetNodeTM(t);
		selfTM.SetTrans(selfTM * pos);
	}
	else 
	{	
		selfTM.IdentityMatrix();
		selfTM.SetTrans(pos);
	}
	return selfTM;
}

//From SpringSysClient
//*************************************************
Tab<Matrix3> Jiggle::GetForceMatrices(TimeValue t)
{
	Tab<Matrix3> tms;
	INode* node;
	Matrix3 parentTM;
	
	parentTM = GetCurrentTM(t);

	tms.Append(1, &parentTM);
	
	for (int x=1;x<dyn_pb->Count(jig_control_node); x++)
	{
		Interval for_ever = FOREVER;
		dyn_pb->GetValue(jig_control_node, 0, node, for_ever, x);
		if (node)
		{
			Matrix3 mat = node->GetNodeTM(t);
			tms.Append(1, &mat);
		}
	}
	return tms;
}


Point3 Jiggle::GetDynamicsForces(TimeValue t, Point3 pos, Point3 vel)
{
	INode *aforce;
	Point3 f(0.0f,0.0f,0.0f);

	for( int i=0;i<force_pb->Count(jig_force_node);i++)
	{
		Interval for_ever = FOREVER;
		force_pb->GetValue(jig_force_node, 0, aforce, for_ever, i);
		
		if (aforce)
		{
			Object* obref = aforce->GetObjectRef();

			ForceField* ff = NULL;
			if (obref != NULL)
			{
				WSMObject* wsmObj = static_cast<WSMObject*>(obref->GetInterface(I_WSMOBJECT));
				if (NULL == wsmObj && WSM_OBJECT_CLASS_ID == obref->SuperClassID()) {
					wsmObj = static_cast<WSMObject*>(obref);
				}

				if (wsmObj != NULL) 
				{
					ff = wsmObj->GetForceField(aforce);
				}
			}

		
			if (ff)
			{
				f += ff->Force(t, pos, vel, 0);
				ff->DeleteThis();
			}
		}
	}
	return f;
}

void RegisterJiggleWindow(HWND hWnd, HWND hParent, Control *cont)
{
	JiggleWindow rec(hWnd,hParent,cont);
	jiggleWindows.Append(1,&rec);
}

void UnRegisterJiggleWindow(HWND hWnd)
{	
	for (int i=0; i<jiggleWindows.Count(); i++) 
	{
		if (hWnd==jiggleWindows[i].hWnd) 
		{
			jiggleWindows.Delete(i,1);
			return;
		}
	}	
}

HWND FindOpenJiggleWindow(HWND hParent,Control *cont)
{	
	for (int i=0; i<jiggleWindows.Count(); i++) 
	{
		if (hParent == jiggleWindows[i].hParent &&
			cont    == jiggleWindows[i].cont) 
		{
			return jiggleWindows[i].hWnd;
		}
	}
	return NULL;
}