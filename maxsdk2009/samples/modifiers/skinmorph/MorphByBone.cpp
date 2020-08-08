/**********************************************************************
*<
FILE: MorphByBone.cpp

DESCRIPTION:	Appwizard generated plugin

CREATED BY: 

HISTORY: 


disable grow, shrink ,loop, ring, and edge limit if not mesh or poly

WishLists
	add load/save deltas

	Extract Selected Morph. Similar to Extract in the standard Morph Modifier.  Let the user just snap shoot
	the morph at 100% while all the others are at 0.


	add a get current morph index

*>	Copyright (c) 2000, All Rights Reserved.
**********************************************************************/

#include "MorphByBone.h"
#include "XRef\iXrefItem.h"

MoveModBoxCMode *MorphByBone::moveMode  = NULL;
RotateModBoxCMode	*MorphByBone::rotMode;

UScaleModBoxCMode	*MorphByBone::uscaleMode = NULL;
NUScaleModBoxCMode	*MorphByBone::nuscaleMode = NULL;
SquashModBoxCMode	*MorphByBone::squashMode = NULL;
SelectModBoxCMode	*MorphByBone::selectMode = NULL;
ICustButton			*MorphByBone::iEditButton   = NULL;
ICustButton			*MorphByBone::iNodeButton   = NULL;
ICustButton			*MorphByBone::iPickBoneButton   = NULL;
ICustButton			*MorphByBone::iMultiSelectButton   = NULL;
ICustButton			*MorphByBone::iDeleteMorphButton   = NULL;
ICustButton			*MorphByBone::iResetMorphButton   = NULL;
ICustButton			*MorphByBone::iClearVertsButton   = NULL;
ICustButton			*MorphByBone::iRemoveVertsButton   = NULL;
ICustButton			*MorphByBone::iCreateMorphButton   = NULL;
ICustButton			*MorphByBone::iReloadButton   = NULL;
ICustButton			*MorphByBone::iGraphButton   = NULL;
ICustEdit			*MorphByBone::iNameField = NULL;
ISpinnerControl		*MorphByBone::iInfluenceAngleSpinner = NULL;
HWND				MorphByBone::hwndFalloffDropList = NULL;
HWND				MorphByBone::hwndJointTypeDropList = NULL;
HWND				MorphByBone::hwndSelectedVertexCheckBox = NULL;
HWND				MorphByBone::hwndEnabledCheckBox = NULL;






IObjParam *MorphByBone::ip			= NULL;

int MeshEnumProc::proc(ReferenceMaker *rmaker) 
{ 
	if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
	{
		Nodes.Append(1, (INode **)&rmaker);  
		return DEP_ENUM_SKIP;
	}

	return DEP_ENUM_CONTINUE;
}



//--- MorphByBone -------------------------------------------------------

MorphByBone::~MorphByBone()
{
	for (int i = 0; i < boneData.Count(); i++)
	{
		delete boneData[i];
		boneData[i] = NULL;
	}
}

/*===========================================================================*\
|	The validity of the parameters.  First a test for editing is performed
|  then Start at FOREVER, and intersect with the validity of each item
\*===========================================================================*/
Interval MorphByBone::LocalValidity(TimeValue t)
{
	// if being edited, return NEVER forces a cache to be built 
	// after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;  
	//TODO: Return the validity interval of the modifier
	return NEVER;
}


/*************************************************************************************************
*
Between NotifyPreCollapse and NotifyPostCollapse, Modify is
called by the system.  NotifyPreCollapse can be used to save any plugin dependant data e.g.
LocalModData
*
\*************************************************************************************************/

void MorphByBone::NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index)
{
	//TODO:  Perform any Pre Stack Collapse methods here
}



/*************************************************************************************************
*
NotifyPostCollapse can be used to apply the modifier back onto to the stack, copying over the
stored data from the temporary storage.  To reapply the modifier the following code can be 
used

Object *bo = node->GetObjectRef();
IDerivedObject *derob = NULL;
if(bo->SuperClassID() != GEN_DERIVOB_CLASS_ID)
{
derob = CreateDerivedObject(obj);
node->SetObjectRef(derob);
}
else
derob = (IDerivedObject*) bo;

// Add ourselves to the top of the stack
derob->AddModifier(this,NULL,derob->NumModifiers());

*
\*************************************************************************************************/

void MorphByBone::NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index)
{
	//TODO: Perform any Post Stack collapse methods here.

}



BOOL RecursePipeAndMatch(ModContext *mcd, Object *obj)
{
	SClass_ID		sc;
	IDerivedObject* dobj;
	Object *currentObject = obj;

	if ((sc = obj->SuperClassID()) == GEN_DERIVOB_CLASS_ID)
	{
		dobj = (IDerivedObject*)obj;
		while (sc == GEN_DERIVOB_CLASS_ID)
		{
			for (int j = 0; j < dobj->NumModifiers(); j++)
			{
				ModContext *mc = dobj->GetModContext(j);
				if (mcd == mc)
				{
					return TRUE;
				}

			}
			dobj = (IDerivedObject*)dobj->GetObjRef();
			currentObject = (Object*) dobj;
			sc = dobj->SuperClassID();
		}
	}

	int bct = currentObject->NumPipeBranches(FALSE);
	if (bct > 0)
	{
		for (int bi = 0; bi < bct; bi++)
		{
			Object* bobj = currentObject->GetPipeBranch(bi,FALSE);
			if (RecursePipeAndMatch(mcd, bobj)) return TRUE;
		}

	}

	return FALSE;
}




INode* MorphByBone::GetNodeFromModContext(ModContext *mc)

{
	int	i;

	MeshEnumProc dep;              
	DoEnumDependents(&dep);
	for ( i = 0; i < dep.Nodes.Count(); i++)
	{
		INode *node = dep.Nodes[i];
		BOOL found = FALSE;

		if (node)
		{
			Object* obj = node->GetObjectRef();

			if ( RecursePipeAndMatch(mc,obj) )
			{
				return node;
			}
			if (obj != NULL)
			{
				IXRefItem* xi = IXRefItem::GetInterface(*obj);
				if (xi != NULL) 
				{
					obj = (Object*)xi->GetSrcItem();
					if (RecursePipeAndMatch(mc,obj))
					{							
						return node;
					}
				}
			}
		}
	}
	return NULL;

}

/*************************************************************************************************
*
ModifyObject will do all the work in a full modifier
This includes casting objects to their correct form, doing modifications
changing their parameters, etc
*
\************************************************************************************************/


void MorphByBone::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
{
	//TODO: Add the code for actually modifying the object
	//check if no local data
	if (mc.localData == NULL)
	{
		MorphByBoneLocalData *ld = new MorphByBoneLocalData();
		//get self node
		ld->selfNode = GetNodeFromModContext(&mc);


		//get self  node and object tm
		ld->selfNodeTM = ld->selfNode->GetNodeTM(t);
		ld->selfObjectTM = ld->selfNode->GetObjectTM(t);
		//add it and set it to id based on the local data list

		localDataList.Append(1,&ld);
		ld->id = localDataList.Count()-1;
		mc.localData = ld;
		ld->originalPoints.SetCount(os->obj->NumPoints());
		for (int i = 0; i < os->obj->NumPoints(); i++)
		{
			ld->originalPoints[i] = os->obj->GetPoint(i);	
		}
	}
	MorphByBoneLocalData *ld  = (MorphByBoneLocalData *) mc.localData;

	ld->selfNode = GetNodeFromModContext(&mc);
	//get self  node and object tm

	if (ld->originalPoints.Count() != os->obj->NumPoints())
	{
		ld->originalPoints.SetCount(os->obj->NumPoints());
		for (int i = 0; i < os->obj->NumPoints(); i++)
		{
			ld->originalPoints[i] = os->obj->GetPoint(i);	
		}
	}

	//check to see if this is in our local list if not add it
	if (ld->id >= localDataList.Count()) 
	{

/*
		if (ld->selfNode == NULL)
			DebugPrint("Adding ld %d:null\n",ld->id);
		else DebugPrint("Adding ld %d:%s\n",ld->id,ld->selfNode->GetName());
*/

		Tab<MorphByBoneLocalData*> tempList;
		tempList.SetCount(ld->id+1);

		for (int i = 0; i < tempList.Count(); i++)
		{
			tempList[i] = NULL;
		}

		for (int i = 0; i < localDataList.Count(); i++)
		{
			tempList[i] = localDataList[i];
		}	
		tempList[ld->id] = ld;
		localDataList = tempList;		
	}
	else if (localDataList[ld->id] == NULL)
		localDataList[ld->id] = ld;

	/*
	DebugPrint("ld count %d\n",localDataList.Count());
	for (int ldIndex = 0; ldIndex < localDataList.Count(); ldIndex++)
	{
		if (localDataList[ldIndex] == NULL)
			DebugPrint("%d null \n",ldIndex);
		else DebugPrint("%d ld - %s\n",localDataList[ldIndex]->id,localDataList[ldIndex]->selfNode->GetName());
	}
	*/

	if (ld->rebuildSoftSelection)
	{
		BOOL useSoftSelection;
		pblock->GetValue(pb_usesoftselection,0,useSoftSelection,FOREVER);
		float radius;
		pblock->GetValue(pb_selectionradius,0,radius,FOREVER);
		
		BOOL useEdgeLimit;
		int edgeLimit;
		pblock->GetValue(pb_useedgelimit,0,useEdgeLimit,FOREVER);
		pblock->GetValue(pb_edgelimit,0,edgeLimit,FOREVER);

		if (useSoftSelection)
			ld->ComputeSoftSelection(radius,pCurve,useEdgeLimit,edgeLimit);
		else
		{
			for (int i = 0; i < ld->softSelection.Count(); i++)
			{
				if (ld->softSelection[i] != 1.0f)
					ld->softSelection[i] = 0.0f;
			}
		}
		ld->rebuildSoftSelection = FALSE;
	}

	if (TestAFlag(A_MOD_BEING_EDITED))
	{
		ld->BuildConnectionList(os->obj);
	}

	for (int i = 0; i < boneData.Count(); i++)
	{
		INode *node = GetNode(i);
		if (node)
		{
			boneData[i]->currentBoneNodeTM = node->GetNodeTM(t);
			boneData[i]->currentBoneObjectTM = node->GetObjectTM(t);

			if (node->IsRootNode())
			{
				boneData[i]->parentBoneNodeCurrentTM = Matrix3(1);
				boneData[i]->parentBoneObjectCurrentTM = Matrix3(1);
			}
			else
			{
				INode *pnode = node->GetParentNode();

				boneData[i]->parentBoneNodeCurrentTM = pnode->GetNodeTM(t);
				boneData[i]->parentBoneObjectCurrentTM = pnode->GetObjectTM(t);

			}
			if ((editMorph) && (i==currentBone) )
			{
				if ((currentMorph >= 0) && (currentMorph < boneData[i]->morphTargets.Count()))
				{
					Matrix3 boneTM;
					Matrix3 parentTM;
					parentTM = boneData[i]->morphTargets[currentMorph]->parentTM;
					boneTM = boneData[i]->morphTargets[currentMorph]->boneTMInParentSpace * parentTM;

					boneData[i]->currentBoneNodeTM = boneTM;
					boneData[i]->currentBoneObjectTM = boneTM;

					boneData[i]->parentBoneNodeCurrentTM = parentTM;
					boneData[i]->parentBoneObjectCurrentTM = parentTM;

					//					boneData[i]->currentBoneNodeTM = boneData[i]->morph
				}
			}



		}
	}



	//if no self node bail
	if (ld->selfNode == NULL) return;

	ld->selfNodeCurrentTM = ld->selfNode->GetNodeTM(t);
	ld->selfObjectCurrentTM = ld->selfNode->GetObjectTM(t);

	//get mirror data

	pblock->GetValue(pb_mirrorplane,t,mirrorPlane,FOREVER);

	pblock->GetValue(pb_mirroroffset,t,mirrorOffset,FOREVER);

	pblock->GetValue(pb_showmirror,t,showMirror,FOREVER);

	pblock->GetValue(pb_mirrorpreviewverts,t,previewVertices,FOREVER);
	pblock->GetValue(pb_mirrorpreviewbone,t,previewBones,FOREVER);

	pblock->GetValue(pb_safemode,t,safeMode,FOREVER);


	//mirror tm
	mirrorTM.IdentityMatrix();
	//get bounds center and use as our trans
	Point3 center;

	bounds.Init();
	os->obj->GetDeformBBox(t, bounds);
/*
	for (i = 0; i < os->obj->NumPoints(); i++)
	{
		if (os->obj->GetPoint(i).x < -100000)
			DebugPrint("Error\n");
		bounds += os->obj->GetPoint(i);
	}
*/

	//add our offset
	center  = Point3(0.0f,0.0f,0.0f);
	center[mirrorPlane] += mirrorOffset;

	Point3 xvec(1.0f,0.0f,0.0f);
	Point3 yvec(0.0f,1.0f,0.0f);
	Point3 zvec(0.0f,0.0f,1.0f);

	mirrorTM.SetRow(0,xvec);
	mirrorTM.SetRow(1,yvec);
	mirrorTM.SetRow(2,zvec);
	mirrorTM.SetRow(3,center);

	Matrix3 imirrorTM;

	if (mirrorPlane == 0)
		xvec = xvec * -1.0f;
	else if (mirrorPlane == 1)
		yvec = yvec * -1.0f;
	else if (mirrorPlane == 2)
		zvec = zvec * -1.0f;

	imirrorTM.SetRow(0,xvec);
	imirrorTM.SetRow(1,yvec);
	imirrorTM.SetRow(2,zvec);
	imirrorTM.SetRow(3,center);

	mirrorTM = Inverse(mirrorTM) * imirrorTM;



	int pointCount = os->obj->NumPoints();
	if (ld->postDeform.Count() != pointCount)
		ld->postDeform.SetCount(pointCount);
	if (ld->preDeform.Count() != pointCount)
		ld->preDeform.SetCount(pointCount);
	if (ld->softSelection.Count() != pointCount)
	{
		ld->softSelection.SetCount(pointCount);
		for (int i = 0; i < pointCount; i++)
			ld->softSelection[i] = 0.0f;
		
//		ld->selection.ClearAll();
	}

	for (int i = 0; i < pointCount; i++)
	{
		ld->preDeform[i] = os->obj->GetPoint(i);
	}

	int ldID= -1;

	for (int i = 0; i < this->localDataList.Count(); i++)
	{
		if (ld == localDataList[i])
			ldID = i;
	}

	if (ldID != -1)
		Deform(os->obj, ldID);

	if ( (ip) && (TestAFlag(A_MOD_BEING_EDITED)) )
	{
		static TimeValue lastTime = 0;

		if (t != lastTime)
		{
			BuildTreeList();
			int id = currentBone;
			if (currentMorph >= 0)
				id = (id+1) *1000 + currentMorph;
			currentMorph = -1;
			currentBone = -1;
			SetSelection(id, TRUE);
		}

		lastTime = t;

	}


	os->obj->UpdateValidity(GEOM_CHAN_NUM,Interval(t,t));


}





Interval MorphByBone::GetValidity(TimeValue t)
{
	Interval valid = FOREVER;
	//TODO: Return the validity interval of the modifier
	return valid;
}




RefTargetHandle MorphByBone::Clone(RemapDir& remap)
{
	MorphByBone* newmod = new MorphByBone();	
	newmod->ReplaceReference(PBLOCK_REF,remap.CloneRef(pblock));
	BaseClone(this, newmod, remap);

	newmod->boneData.SetCount(boneData.Count());
	for (int i = 0; i < boneData.Count(); i++)
	{
		newmod->boneData[i] = boneData[i]->Clone();

	}
	return(newmod);
}


//From ReferenceMaker 


/****************************************************************************************
*
NotifyInputChanged is called each time the input object is changed in some way
We can find out how it was changed by checking partID and message
*
\****************************************************************************************/

void MorphByBone::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc)
{

}



//From Object
BOOL MorphByBone::HasUVW() 
{ 
	//TODO: Return whether the object has UVW coordinates or not
	return TRUE; 
}

void MorphByBone::SetGenUVW(BOOL sw) 
{  
	if (sw==HasUVW()) return;
	//TODO: Set the plugin internal value to sw				
}



void MorphByBone::SetSelection(int id, BOOL updateWindow)
{
	int tempBone, tempMorph;

	if (id < 1000)
	{
		tempBone = id;
		tempMorph = -1;
	}
	else
	{
		tempBone = id /1000-1;
		tempMorph = id - (tempBone+1)*1000;
	}

	int lastMorph, lastBone;

	lastBone = currentBone;
	lastMorph = currentMorph;

	if ( (tempBone != currentBone) || (tempMorph != currentMorph))
	{
		currentBone = tempBone;
		currentMorph = tempMorph;

		if ((currentBone < 0) || (currentBone >= boneData.Count()) ||
			(currentMorph >= boneData[currentBone]->morphTargets.Count()))
		{
			return;
		}

		if (updateWindow)
		{
			HWND treeHWND = GetDlgItem(rollupHWND,IDC_TREE1);

			if (lastBone != -1)
			{
				if (lastMorph == -1)
				{
					if (lastBone < boneData.Count())
					{
						TreeView_SetItemState(treeHWND,boneData[lastBone]->treeHWND,0,TVIS_BOLD);
						TreeView_SetItemState(treeHWND,boneData[lastBone]->treeHWND,0,TVIS_SELECTED);
					}
				}
				else
				{
					if (lastBone < boneData.Count())
					{
						if (lastMorph < boneData[lastBone]->morphTargets.Count())
						{
							TreeView_SetItemState(treeHWND,boneData[lastBone]->morphTargets[lastMorph]->treeHWND,0,TVIS_BOLD);
							TreeView_SetItemState(treeHWND,boneData[lastBone]->morphTargets[lastMorph]->treeHWND,0,TVIS_SELECTED);			
						}
					}
				}
			}


			if (currentMorph == -1) 
			{
				if (currentBone != -1)
				{
					TreeView_SetItemState(treeHWND,boneData[currentBone]->treeHWND,TVIS_SELECTED,TVIS_SELECTED);
					TreeView_SetItemState(treeHWND,boneData[currentBone]->treeHWND,TVIS_BOLD,TVIS_BOLD);
				}

				//			SendMessage(treeHWND,TVM_SELECTITEM ,TVGN_CARET,(LPARAM) boneData[currentBone]->treeHWND);
			}
			else 
			{
				TreeView_SetItemState(treeHWND,boneData[currentBone]->morphTargets[currentMorph]->treeHWND,TVIS_SELECTED,TVIS_SELECTED);
				TreeView_SetItemState(treeHWND,boneData[currentBone]->morphTargets[currentMorph]->treeHWND,TVIS_BOLD,TVIS_BOLD);

				//			SendMessage(treeHWND,TVM_SELECTITEM ,TVGN_CARET,(LPARAM) boneData[currentBone]->morphTargets[currentMorph]->treeHWND);
			}

		}


		if (currentMorph == -1)
		{
			if (editMorph)
				fnEdit(FALSE);
		}
		UpdateLocalUI();

		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	}
}



void MorphByBone::GetSubObjectCenters(
									  SubObjAxisCallback *cb,TimeValue t,
									  INode *node,ModContext *mc)
{

	MorphByBoneLocalData *ld = (MorphByBoneLocalData *) mc->localData;

	if (ld)
	{
		Matrix3 tm = ld->selfObjectCurrentTM;
		Box3 bb;
		bb.Init();
		for (int i = 0; i < ld->postDeform.Count(); i++)
		{
			if (ld->softSelection[i]>0.0f)
				bb += ld->postDeform[i];
		}
		Point3 pt = bb.Center();
		tm.PreTranslate(pt);
		cb->Center(tm.GetTrans(),0);
	}
}

void MorphByBone::GetSubObjectTMs(
								  SubObjAxisCallback *cb,TimeValue t,
								  INode *node,ModContext *mc)
{

	MorphByBoneLocalData *ld = (MorphByBoneLocalData *) mc->localData;

	if (ld)
	{
		Matrix3 tm = ld->selfObjectCurrentTM;
		cb->TM(tm,0);
	}
}


void MorphByBone::Rotate(
						 TimeValue t, 
						 Matrix3& partm, 
						 Matrix3& tmAxis, 
						 Quat& val, 
						 BOOL localOrigin)
{

	if ((safeMode) && (!editMorph)) return;

	scriptMoveMode = FALSE;
	Matrix3 tm  = partm * Inverse(tmAxis);
	Matrix3 itm = Inverse(tm);
	Matrix3 mat;
	val.MakeMatrix(mat);
	tm *= mat;

	this->scriptTMA = tm;
	this->scriptTMB = itm;
	SetMorph(tm,itm);


	NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

}

void MorphByBone::Scale(
						TimeValue t, 
						Matrix3& partm, 
						Matrix3& tmAxis, 
						Point3& val, 
						BOOL localOrigin)
{

	if ((safeMode) && (!editMorph)) return;

	scriptMoveMode = FALSE;

	Matrix3 tm  = partm * Inverse(tmAxis);
	Matrix3 itm = Inverse(tm);
	tm *= ScaleMatrix(val);;

	this->scriptTMA = tm;
	this->scriptTMB = itm;

	SetMorph(tm,itm);


	NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());


}


void MorphByBone::Move(TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
					   Point3& val, BOOL localOrigin)
{
	if ((safeMode) && (!editMorph)) return;

	scriptMoveMode = TRUE;

	Point3 vec = VectorTransform(tmAxis*Inverse(partm),val);
	this->scriptMoveVec = vec;

	SetMorph(vec/*VectorTransform(tmAxis*Inverse(partm),val)*/);
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

	/*	if (sel[0]) {
	p1->SetValue(
	t,VectorTransform(tmAxis*Inverse(partm),val),
	TRUE,CTRL_RELATIVE);
	}
	if (sel[1]) {
	p2->SetValue(
	t,VectorTransform(tmAxis*Inverse(partm),val),
	TRUE,CTRL_RELATIVE);
	}
 	*/
}

float MorphByBone::AngleBetweenTMs(Matrix3 tmA, Matrix3 tmB)
{
	float zangle,yangle;
	Point3 avec,bvec;
	float angle = 0.0f;
	avec = Normalize(tmA.GetRow(0));
	bvec = Normalize(tmB.GetRow(0));
	angle = DotProd(avec,bvec);
	if ((avec == bvec) || (angle = 1.0f))
		angle = 0.0f;
	else if ((avec*-1.0f) == bvec) 
		angle = PI;
	else angle = acos(angle);

	avec = Normalize(tmA.GetRow(1));
	bvec = Normalize(tmB.GetRow(1));
	yangle = DotProd(avec,bvec);
	if (avec == bvec) 
		yangle = 0.0f;
	else if ((avec*-1.0f) == bvec) 
		yangle = PI;
	else yangle = acos(yangle);

	if (yangle > angle) angle = yangle;


	avec = Normalize(tmA.GetRow(2));
	bvec = Normalize(tmB.GetRow(2));
	zangle = DotProd(avec,bvec);
	if (avec == bvec) 
		zangle = 0.0f;
	else if ((avec*-1.0f) == bvec) 
		zangle = PI;
	else zangle = acos(zangle);

	if (zangle > angle) angle = zangle;



	return angle;

}


void MorphByBone::DeleteMorph(int whichBone, int whichMorph)
{
	if ((whichBone < 0) || (whichBone >= boneData.Count()) ||
		(whichMorph < 0) || (whichMorph >= boneData[whichBone]->morphTargets.Count()))
	{
		return;
	}
	else boneData[whichBone]->morphTargets[whichMorph]->SetDead(TRUE);

	theHold.Begin();
	theHold.Put(new RemoveMorphRestore(this,boneData[whichBone]->morphTargets[whichMorph]));
	theHold.Accept(GetString(IDS_REMOVEMORPH));

	if (editMorph)
		fnEdit(FALSE);


	currentBone = 0;
	currentMorph = -1;

	BuildTreeList();
	SetSelection(currentBone*1000+currentMorph,TRUE);
	UpdateLocalUI();
	//just tag that morph dead
}

void MorphByBone::DeleteBone(int whichBone)
{
	if ((whichBone < 0) || (whichBone >= boneData.Count()) )
	{
		return;
	}
	for (int i = 0; i < boneData[whichBone]->morphTargets.Count(); i++)
	{
		boneData[whichBone]->morphTargets[i]->SetDead(true);
	}
	INode *node = NULL;
	//	boneData[whichBone]->node = NULL;
	pblock->SetValue(pb_bonelist,0,node,whichBone);
	currentBone = -1;
	currentMorph = -1;

	if (editMorph)
		fnEdit(FALSE);

	BuildTreeList();
	SetSelection((currentBone+1)*1000+currentMorph,TRUE);
	UpdateLocalUI();
	//just tag that morph dead
}


void MorphByBone::CreateMorph(INode *node,BOOL force)

{
	if (editMorph) return;
	//build our morph if not there
	//get our current bone and matrix
	int activeMorph = -1;

	int cBone = this->GetBoneIndex(node);
//	 = GetNode(currentBone);


	if ( (cBone < 0) || (cBone >= boneData.Count()) || 
		(node == NULL) ) return;
	//loop through all our current morphs and 
	for (int i = 0; i < boneData[cBone]->morphTargets.Count(); i++)
	{
		//get the morph matrix and compare it gainst the current
		if (!boneData[cBone]->morphTargets[i]->IsDead())
		{
			float angle = AngleBetweenTMs(boneData[cBone]->currentBoneNodeTM*Inverse(boneData[cBone]->parentBoneNodeCurrentTM), boneData[cBone]->morphTargets[i]->boneTMInParentSpace);
			//if within the threshold tag this as our morph
			if (angle < 0.001f)
			{
				activeMorph = i;
				i = boneData[cBone]->morphTargets.Count();
			}
		}

	}

	//if no morph found create one
	if ((activeMorph == -1) || (force))
	{
		MorphTargets *morphTarget = new MorphTargets() ;
		//		morphTarget->boneID = cBone;
		morphTarget->boneTMInParentSpace = boneData[cBone]->currentBoneNodeTM  * Inverse(boneData[cBone]->parentBoneNodeCurrentTM) ;
		morphTarget->parentTM = boneData[cBone]->parentBoneNodeCurrentTM;
		TSTR name;
		name.printf("Morph %d",boneData[cBone]->morphTargets.Count());
		morphTarget->name = name;
		morphTarget->influenceAngle = PI/2.0f;
		morphTarget->treeHWND = NULL;
		morphTarget->weight = 1.0f;
		//set our targets to ful list and zero then out
		int total = 0;
		for (int i = 0; i < localDataList.Count(); i++)
		{
			Matrix3 tm = localDataList[i]->selfObjectTM*Inverse(boneData[cBone]->currentBoneObjectTM);
			for (int j =0; j < localDataList[i]->preDeform.Count(); j++)
			{
				float length = Length(localDataList[i]->preDeform[j] - localDataList[i]->originalPoints[j]);
				if (length > 0.01f) total++;
			}
		}
/*
		for (int i = 0; i < localDataList.Count(); i++)
		{

			total += localDataList[i]->preDeform.Count();
		}
*/

		morphTarget->d.SetCount(total);

		int currentVert = 0;

		for (int i = 0; i < localDataList.Count(); i++)
		{
			Matrix3 tm = localDataList[i]->selfObjectTM*Inverse(boneData[cBone]->currentBoneObjectTM);
			for (int j =0; j < localDataList[i]->preDeform.Count(); j++)
			{
				float length = Length(localDataList[i]->preDeform[j] - localDataList[i]->originalPoints[j]);
				if (length > 0.01f)
				{
					morphTarget->d[currentVert].vertexID = j;
					morphTarget->d[currentVert].vec = Point3(0.0f,0.0f,0.0f);
					morphTarget->d[currentVert].vecParentSpace = Point3(0.0f,0.0f,0.0f);
					morphTarget->d[currentVert].originalPt = localDataList[i]->preDeform[j];//*tm;
					morphTarget->d[currentVert].localOwner = i;
					currentVert++;
				}
			}
		}
		//FIX need to look for empty spot is not append one
		boneData[cBone]->morphTargets.Append(1,&morphTarget);
		int activeMorph = boneData[cBone]->morphTargets.Count()-1;
		BuildTreeList();
		SetSelection((cBone+1)*1000+activeMorph,TRUE);

		theHold.Begin();
		theHold.Put(new CreateMorphRestore(this,morphTarget));
		theHold.Accept(GetString(IDS_CREATEMORPH));


	}
	else
	{
		//make sure we expand the morph target list to cover all our morphs
		currentMorph = activeMorph;
	}

}

void MorphByBone::TransformStart(TimeValue t)
{
	if ((safeMode) && (!editMorph)) return;
	CreateMorph(GetNode(currentBone));

//compress morph
	for (int i = 0; i < localDataList.Count(); i++)
	{
		MorphByBoneLocalData *ld = localDataList[i];
		if (ld)
			ld->tempPoints = ld->postDeform;
	}
}
//	void TransformHoldingFinish (TimeValue t);
//	void TransformFinish(TimeValue t);
//	void TransformCancel(TimeValue t);

void MorphByBone::TransformFinish(TimeValue t)
{

	if (this->scriptMoveMode)
		macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Morph].skinMorphOps.moveVerts"), 1, 0,
								mr_point3,&scriptMoveVec
								);
	else macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Morph].skinMorphOps.transformVerts"), 2, 0,
								mr_matrix3,&scriptTMA,
								mr_matrix3,&scriptTMB
								);

	for (int i = 0; i < localDataList.Count(); i++)
	{
		MorphByBoneLocalData *ld = localDataList[i];
//		ld->tempPoints.Resize(0);
	}
//decompress morph
}

void MorphByBone::SetMorph(Matrix3 tm, Matrix3 itm)
{

	if ((currentBone < 0) || (currentBone >= boneData.Count()) ||
		(currentMorph < 0) || (currentMorph >= boneData[currentBone]->morphTargets.Count()))
	{
		return;
	}
	//convert our vector to bone space
	//add it to our vector list
	if ( ( theHold.Holding() )  &&  (!TestAFlag(A_HELD)) )
	{
		theHold.Put(new MorphRestore(this,currentBone, currentMorph));
		SetAFlag(A_HELD);	
		//build our matching index list
	}
	//add the vec to all our selected vertices


	Tab<int> hitLD;
	Tab<int> hitVerts;

	for (int i = 0; i < boneData[currentBone]->morphTargets[currentMorph]->d.Count(); i++)
	{
		int ldID = boneData[currentBone]->morphTargets[currentMorph]->d[i].localOwner;
		MorphByBoneLocalData *ld = localDataList[ldID];

		int vertID = boneData[currentBone]->morphTargets[currentMorph]->d[i].vertexID;


		if (ld->softSelection[vertID]> 0.0f)
		{

			Point3 p = ld->tempPoints[vertID];
			Point3 vec = ((tm*p)*itm)-ld->tempPoints[vertID];

			Point3 tvec = VectorTransform(ld->selfNodeTM * Inverse(boneData[currentBone]->currentBoneNodeTM),vec);
			boneData[currentBone]->morphTargets[currentMorph]->d[i].vec += tvec  * ld->softSelection[vertID];

			Point3 pvec = VectorTransform(ld->selfNodeTM * Inverse(boneData[currentBone]->parentBoneNodeCurrentTM),vec);
			boneData[currentBone]->morphTargets[currentMorph]->d[i].vecParentSpace += pvec  * ld->softSelection[vertID];

			hitLD.Append(1,&ldID,5000);
			hitVerts.Append(1,&vertID,5000);
		}
	}

//add any missed verts
	for (int i = 0;  i < localDataList.Count(); i++)
	{
		MorphByBoneLocalData *ld = localDataList[i];
		BitArray holdSelection;
		holdSelection.SetSize(ld->softSelection.Count());
		for (int j = 0; j < ld->softSelection.Count(); j++)
		{
			if (ld->softSelection[j] == 0.0f)
				holdSelection.Set(j,FALSE);
			else holdSelection.Set(j,TRUE);
			
		}

		for (int j = 0; j < hitVerts.Count(); j++)
		{	
			if (hitLD[j] == i)
			{
				holdSelection.Set(hitVerts[j],false);
			}
		}
		
		if (holdSelection.NumberSet() > 0)
		{
			for (int j = 0; j < holdSelection.GetSize(); j++)
			{
				if (holdSelection[j])
				{
				
					int ldID = i;
					MorphByBoneLocalData *ld = localDataList[ldID];

					int vertID = j;
//create a new morph delta		
					MorphData tempData;
					tempData.vertexID = vertID; 
		

					tempData.vertexID = j;
					tempData.vec = Point3(0.0f,0.0f,0.0f);
					tempData.vecParentSpace = Point3(0.0f,0.0f,0.0f);
					tempData.originalPt = ld->preDeform[j];//*tm;
					tempData.localOwner = i;
					
					boneData[currentBone]->morphTargets[currentMorph]->d.Append(1,&tempData,50);

					int index = boneData[currentBone]->morphTargets[currentMorph]->d.Count()-1;

					Point3 p = ld->tempPoints[vertID];
					Point3 vec = ((tm*p)*itm)-ld->tempPoints[vertID];

					Point3 tvec = VectorTransform(ld->selfNodeTM * Inverse(boneData[currentBone]->currentBoneNodeTM),vec);
					boneData[currentBone]->morphTargets[currentMorph]->d[index].vec += tvec  * ld->softSelection[vertID];

					Point3 pvec = VectorTransform(ld->selfNodeTM * Inverse(boneData[currentBone]->parentBoneNodeCurrentTM),vec);
					boneData[currentBone]->morphTargets[currentMorph]->d[index].vecParentSpace += pvec  * ld->softSelection[vertID];

				}
			}
		}

	}



}



void MorphByBone::SetMorph(Point3 vec)

{

	if ((currentBone < 0) || (currentBone >= boneData.Count()) ||
		(currentMorph < 0) || (currentMorph >= boneData[currentBone]->morphTargets.Count()))
	{
		return;
	}
	//convert our vector to bone space
	//add it to our vector list
	if ( ( theHold.Holding() )  &&  (!TestAFlag(A_HELD)) )
	{
		theHold.Put(new MorphRestore(this,currentBone, currentMorph));
		SetAFlag(A_HELD);	
		//build our matching index list
	}
	//add the vec to all our selected vertices

	Tab<int> hitLD;
	Tab<int> hitVerts;

	for (int i = 0; i < boneData[currentBone]->morphTargets[currentMorph]->d.Count(); i++)
	{
		int ldID = boneData[currentBone]->morphTargets[currentMorph]->d[i].localOwner;
 		MorphByBoneLocalData *ld = localDataList[ldID];

		int vertID = boneData[currentBone]->morphTargets[currentMorph]->d[i].vertexID;

		if (ld->softSelection[vertID] > 0.0f)
		{
			Point3 tvec = VectorTransform(ld->selfNodeTM * Inverse(boneData[currentBone]->currentBoneNodeTM),vec);
			boneData[currentBone]->morphTargets[currentMorph]->d[i].vec += tvec * ld->softSelection[vertID];

			Point3 pvec = VectorTransform(ld->selfNodeTM * Inverse(boneData[currentBone]->parentBoneNodeCurrentTM),vec);
			boneData[currentBone]->morphTargets[currentMorph]->d[i].vecParentSpace += pvec * ld->softSelection[vertID];

			hitLD.Append(1,&ldID,5000);
			hitVerts.Append(1,&vertID,5000);
		}
	}

//add any missed verts
	for (int i = 0;  i < localDataList.Count(); i++)
	{
		MorphByBoneLocalData *ld = localDataList[i];
		BitArray holdSelection;

		holdSelection.SetSize(ld->softSelection.Count());
		for (int j = 0; j < ld->softSelection.Count(); j++)
		{
			if (ld->softSelection[j] == 0.0f)
				holdSelection.Set(j,FALSE);
			else holdSelection.Set(j,TRUE);
			
		}
//		holdSelection = ld->selection;

		for (int j = 0; j < hitVerts.Count(); j++)
		{	
			if (hitLD[j] == i)
			{
				holdSelection.Set(hitVerts[j],false);
			}
		}
		
		if (holdSelection.NumberSet() > 0)
		{
			for (int j = 0; j < holdSelection.GetSize(); j++)
			{
				if (holdSelection[j])
				{
				
					int ldID = i;
					MorphByBoneLocalData *ld = localDataList[ldID];

					int vertID = j;
//create a new morph delta		
					MorphData tempData;
					tempData.vertexID = vertID; 
		

					tempData.vertexID = j;
					tempData.vec = Point3(0.0f,0.0f,0.0f);
					tempData.vecParentSpace = Point3(0.0f,0.0f,0.0f);
					tempData.originalPt = ld->preDeform[j];//*tm;
					tempData.localOwner = i;
					
					boneData[currentBone]->morphTargets[currentMorph]->d.Append(1,&tempData,50);

					int index = boneData[currentBone]->morphTargets[currentMorph]->d.Count()-1;

					Point3 tvec = VectorTransform(ld->selfNodeTM * Inverse(boneData[currentBone]->currentBoneNodeTM),vec);
					boneData[currentBone]->morphTargets[currentMorph]->d[index].vec += tvec  * ld->softSelection[vertID];

					Point3 pvec = VectorTransform(ld->selfNodeTM * Inverse(boneData[currentBone]->parentBoneNodeCurrentTM),vec);
					boneData[currentBone]->morphTargets[currentMorph]->d[index].vecParentSpace += pvec * ld->softSelection[vertID];
				}
			}
		}

	}



}


void MorphByBone::UpdateLocalUI()
{
	if (ip==NULL) return;

	if ((currentBone < 0) || (currentBone >= boneData.Count()))
		iCreateMorphButton->Enable(FALSE);
	else iCreateMorphButton->Enable(TRUE);

	if ((currentBone < 0) || (currentBone >= boneData.Count()) ||
		(currentMorph < 0) || (currentMorph >= boneData[currentBone]->morphTargets.Count()))
	{
		//disable properties		
		iNodeButton->Enable(FALSE);
		iEditButton->Enable(FALSE);
		iDeleteMorphButton->Enable(FALSE);
		iResetMorphButton->Enable(FALSE);
		iClearVertsButton->Enable(FALSE);
		iRemoveVertsButton->Enable(FALSE);
		iReloadButton->Enable(FALSE);
		iGraphButton->Enable(FALSE);
		iNameField->Enable(FALSE);
		iInfluenceAngleSpinner->Enable(FALSE);
		EnableWindow(hwndFalloffDropList,FALSE);
		EnableWindow(hwndJointTypeDropList,FALSE);
		EnableWindow(hwndSelectedVertexCheckBox,FALSE);
		EnableWindow(hwndEnabledCheckBox,FALSE);
	}
	else
	{		
		iNodeButton->Enable(TRUE);
		iEditButton->Enable(TRUE);
		iDeleteMorphButton->Enable(TRUE);
		iResetMorphButton->Enable(TRUE);
		iClearVertsButton->Enable(TRUE);
		iRemoveVertsButton->Enable(TRUE);
		iReloadButton->Enable(TRUE);

		if (boneData[currentBone]->morphTargets[currentMorph]->falloff == FALLOFF_CUSTOM)
			iGraphButton->Enable(TRUE);		
		else iGraphButton->Enable(FALSE);		
		iNameField->Enable(TRUE);
		iInfluenceAngleSpinner->Enable(TRUE);
		EnableWindow(hwndFalloffDropList,TRUE);
		EnableWindow(hwndJointTypeDropList,TRUE);
		EnableWindow(hwndSelectedVertexCheckBox,TRUE);

		EnableWindow(hwndEnabledCheckBox,TRUE);

		this->pauseAccessor = TRUE;
		theHold.Suspend();
		SuspendAnimate();
		AnimateOff();
		macroRecorder->Disable();

		pblock->SetValue(pb_lname,0,boneData[currentBone]->morphTargets[currentMorph]->name);
		pblock->SetValue(pb_linfluence,0,boneData[currentBone]->morphTargets[currentMorph]->influenceAngle * 180.0f/PI);
		pblock->SetValue(pb_lfalloff,0,boneData[currentBone]->morphTargets[currentMorph]->falloff);

		pblock->SetValue(pb_lenabled,0,!boneData[currentBone]->morphTargets[currentMorph]->IsDisabled());

		if (boneData[currentBone]->IsBallJoint())
			pblock->SetValue(pb_ljointtype,0,0);
		else pblock->SetValue(pb_ljointtype,0,1);

		macroRecorder->Enable();
		theHold.Resume();
		ResumeAnimate();

		int id = boneData[currentBone]->morphTargets[currentMorph]->externalNodeID;
		if ((id >=0) && (id<pblock->Count(pb_targnodelist)))
		{
			INode *node;
			pblock->GetValue(pb_targnodelist,0,node,FOREVER,id);
			TSTR name;
			if (node == NULL)
				name.printf("%s",GetString(IDS_NONE));
			else
			{
				name.printf("%s",node->GetName());
			}
			if (iNodeButton)
				iNodeButton->SetText(name);
		}
		else
		{
			TSTR name;
			name.printf("%s",GetString(IDS_NONE));
			if (iNodeButton)
				iNodeButton->SetText(name);

		}
		this->pauseAccessor = FALSE;
	}

}

void MorphByBone::SetMorphName(TCHAR *s,int whichBone, int whichMorph)
{
	if ((currentBone < 0) || (currentBone >= boneData.Count()) ||
		(currentMorph < 0) || (currentMorph >= boneData[currentBone]->morphTargets.Count()))
	{
		return;
	}	
	boneData[currentBone]->morphTargets[currentMorph]->name.printf(s);
	BuildTreeList();
	int id = currentBone;
	if (currentMorph >= 0)
		id = (id+1) *1000 + currentMorph;
	currentMorph = -1;
	currentBone = -1;
	SetSelection(id, TRUE);

}


void MorphByBone::SetMorphInfluence(float f,int whichBone, int whichMorph)
{
	if ((currentBone < 0) || (currentBone >= boneData.Count()) ||
		(currentMorph < 0) || (currentMorph >= boneData[currentBone]->morphTargets.Count()))
	{
		return;
	}	
	boneData[currentBone]->morphTargets[currentMorph]->influenceAngle = f * PI/180.0f;
	BuildTreeList();
	int id = currentBone;
	if (currentMorph >= 0)
		id = (id+1) *1000 + currentMorph;
	currentMorph = -1;
	currentBone = -1;
	SetSelection(id, TRUE);

}

float MorphByBone::GetFalloff(float val, int falloff, ICurve *curve)
{

	if ((falloff == FALLOFF_CUSTOM) && (curve))
	{
		val = curve->GetValue(0,val);
	}
	else if (falloff == FALLOFF_LINEAR)
		return val;
	else if (falloff == FALLOFF_SINUAL)
	{
		val = (val * PI) - (PI*0.5f);
		val = (sin(val) + 1.0f)*0.5f;

	}
	else if (falloff == FALLOFF_FAST)
	{
		val =  val * val * val;

	}
	else if (falloff == FALLOFF_SLOW)
	{
		val = val - 1.0f;
		val = val * val * val + 1.0f;

	}
	return val;
}

void MorphByBone::SetMorphFalloff(int falloff,int whichBone, int whichMorph)
{
	if ((currentBone < 0) || (currentBone >= boneData.Count()) ||
		(currentMorph < 0) || (currentMorph >= boneData[currentBone]->morphTargets.Count()))
	{
		return;
	}	
	boneData[currentBone]->morphTargets[currentMorph]->falloff = falloff;
	BuildTreeList();
	int id = currentBone;
	if (currentMorph >= 0)
		id = (id+1) *1000 + currentMorph;
	currentMorph = -1;
	currentBone = -1;
	SetSelection(id, TRUE);

	//build graph if need be
	if (falloff == FALLOFF_CUSTOM)
	{
		//see if we need to create a custom graph
		if (boneData[currentBone]->morphTargets[currentMorph]->externalFalloffID==-1)
		{
			boneData[currentBone]->morphTargets[currentMorph]->CreateGraph(this,pblock);
		}
	}

}

void MorphByBone::SetMorphEnabled(BOOL enabled,int whichBone, int whichMorph)
{
	if ((currentBone < 0) || (currentBone >= boneData.Count()) ||
		(currentMorph < 0) || (currentMorph >= boneData[currentBone]->morphTargets.Count()))
	{
		return;
	}	
	boneData[currentBone]->morphTargets[currentMorph]->SetDisabled(!enabled);
	BuildTreeList();
	int id = currentBone;
	if (currentMorph >= 0)
		id = (id+1) *1000 + currentMorph;
	currentMorph = -1;
	currentBone = -1;
	SetSelection(id, TRUE);



}

void MorphByBone::SetJointType(int type,int whichBone)
{
	if ((currentBone < 0) || (currentBone >= boneData.Count()))

	{
		return;
	}	

	if (type == 0)
		boneData[currentBone]->SetBallJoint();
	else boneData[currentBone]->SetPlanarJoint();

	BuildTreeList();
	int id = currentBone;
	if (currentMorph >= 0)
		id = (id+1) *1000 + currentMorph;
	currentMorph = -1;
	currentBone = -1;
	SetSelection(id, TRUE);

}



void MorphByBone::SetNode(INode *node,int whichBone, int whichMorph)
{
	if ((whichBone < 0) || (whichBone >= boneData.Count()) ||
		(whichMorph < 0) || (whichMorph >= boneData[whichBone]->morphTargets.Count()))
	{
		return;
	}	

	if (node == NULL)
		return;

	//find an empty node in the paramblock
	int slot = -1;

	theHold.Begin();
	theHold.Put(new MorphUIRestore(this));  //stupid Hack to handle UI update after redo

	for (int i = 0; i < pblock->Count(pb_targnodelist);i++)
	{
		INode *targNode = NULL;
		pblock->GetValue(pb_targnodelist,0,targNode,FOREVER,i);

		if (targNode == NULL)
			slot = i;
	}
	if (slot != -1)
	{
		pblock->SetValue(pb_targnodelist,0,node,slot);
	}
	//if none found add one
	else
	{
		pblock->Append(pb_targnodelist,1,&node);
		slot = pblock->Count(pb_targnodelist)-1;
	}


	

	//set our index to it
	boneData[whichBone]->morphTargets[whichMorph]->externalNodeID = slot;
	ReloadMorph(whichBone,whichMorph);
	theHold.Accept(GetString(IDS_SETEXTERNALNODE));

}

INode *MorphByBone::GetNode(int id)
{
	if (id < 0) return NULL;
	if (id >= pblock->Count(pb_bonelist)) return NULL;
	INode *node = NULL;

	pblock->GetValue(pb_bonelist,0,node,FOREVER,id);

	return node;
}


void MorphByBone::ClearSelectedVertices(bool deleteVerts)
{
	if ((currentBone < 0) || (currentBone >= boneData.Count()) ||
		(currentMorph < 0) || (currentMorph >= boneData[currentBone]->morphTargets.Count()))
	{
		return;
	}	

	theHold.Begin();
	theHold.Put(new MorphUIRestore(this));  //stupid Hack to handle UI update after redo
	theHold.Put(new MorphRestore(this,currentBone, currentMorph));
	theHold.Put(new MorphUIRestore(this));  //stupid Hack to handle UI update after redo
	theHold.Accept(GetString(IDS_CLEARVERTS));

	for (int i = 0; i < boneData[currentBone]->morphTargets[currentMorph]->d.Count(); i++)
	{
		int ldID = boneData[currentBone]->morphTargets[currentMorph]->d[i].localOwner;
		int vertexID = boneData[currentBone]->morphTargets[currentMorph]->d[i].vertexID;

		if ((ldID >= 0) && (ldID < localDataList.Count()))
		{
			if ((vertexID >= 0) && (vertexID < localDataList[ldID]->softSelection.Count()))
			{
				if (localDataList[ldID]->softSelection[vertexID] > 0.0f)
				{
					if (deleteVerts)
					{
						boneData[currentBone]->morphTargets[currentMorph]->d.Delete(i,1);
						i--;
					}	
					else
					{
						boneData[currentBone]->morphTargets[currentMorph]->d[i].vec = Point3(0.0f,0.0f,0.0f);
						boneData[currentBone]->morphTargets[currentMorph]->d[i].vecParentSpace = Point3(0.0f,0.0f,0.0f);
					}
				}
			}
		}

	}

}

void MorphByBone::ReloadMorph(int whichBone, int whichMorph)
{

	if ((whichBone < 0) || (whichBone >= boneData.Count()) ||
		(whichMorph < 0) || (whichMorph >= boneData[whichBone]->morphTargets.Count()))
	{
		return;
	}	

	BOOL forceAccept = FALSE;

	if (!theHold.Holding())
	{
		theHold.Begin();
		forceAccept = TRUE;
	}
	theHold.Put(new MorphUIRestore(this));  //stupid Hack to handle UI update after redo
	theHold.Put(new MorphRestore(this,currentBone, currentMorph));
	theHold.Put(new MorphUIRestore(this));  //stupid Hack to handle UI update after redo

	if (forceAccept)
		theHold.Accept(GetString(IDS_RELOADTARGET));


	int id = boneData[whichBone]->morphTargets[whichMorph]->externalNodeID;
	if ((id < 0) || (id > pblock->Count(pb_targnodelist))) return;


	INode *node = NULL;
	pblock->GetValue(pb_targnodelist,0,node,FOREVER,id);

	BOOL reloadOnlySelected;
	pblock->GetValue(pb_reloadselected,0,reloadOnlySelected,FOREVER);

	if (node == NULL) return;

	TimeValue t = GetCOREInterface()->GetTime();
	ObjectState os = node->EvalWorldState(t);
	Object *obj = os.obj;

	MorphTargets *morph = boneData[whichBone]->morphTargets[whichMorph];
	
	// Make sure the morph target still has the same number of points as the morph, before processing it. 
	// This will prevent the problem where the user changes the morph target topology after they add it.
	int objNumPoints = obj->NumPoints();
	int originalNumPoints = localDataList[0]->originalPoints.Count();
	if (objNumPoints == originalNumPoints) // Easier to debug
	{
		//now loop through points
		for (int i = 0; i < objNumPoints; i++)
		{
			Point3 p = obj->GetPoint(i);

			Point3 initialP = Point3(0.0f,0.0f,0.0f);
			//		if (i < morph->d.Count())
			{
				BOOL process = TRUE;
				if (reloadOnlySelected)
				{
					if (localDataList[0]->softSelection[i] > 0.0f)
						process = TRUE; //localDataList[0]->selection[i];
					else process = FALSE;
				}
				if (process)
				{
					//see if we have an index already
					int index = -1;
					for (int j = 0; j < morph->d.Count();j++)
					{
						if (i == morph->d[j].vertexID)
						{
							index = j;
							j = morph->d.Count();
						}
					}

					if (index != -1)
						initialP = morph->d[index].originalPt;
					else 
					{
						int originalPointsCount = localDataList[0]->originalPoints.Count();
						if (i < originalPointsCount) //Defect 865440
						{
							initialP = localDataList[0]->originalPoints[i];
						}
					}
					float dist = Length(p - initialP);

					if (dist > 0.0001f)
					{

						Point3 vec = p - initialP;  //these are in local space
						//transform into world space 
						Point3 worldVec = VectorTransform(localDataList[0]->selfNodeTM,vec);
						Point3 vecInParentSpace;
						vecInParentSpace = VectorTransform(Inverse(morph->parentTM),worldVec);
						Point3 vecInBoneSpace;
						vecInBoneSpace = VectorTransform(Inverse(morph->boneTMInParentSpace*morph->parentTM),worldVec);
						if (index != -1)
						{
							morph->d[index].vec = vecInBoneSpace;
							morph->d[index].vecParentSpace = vecInParentSpace;
						}
						else
						{
							MorphData d;
							d.localOwner = 0;
							d.originalPt = initialP;
							d.vec = vecInBoneSpace;
							d.vecParentSpace = vecInParentSpace;
							d.vertexID = i;
							morph->d.Append(1,&d);
						}
					}
				}
			}
		}
	}
	else
	{
		DbgAssert(FALSE);
	}
}


void MorphByBone::ResetMorph(int whichBone, int whichMorph)
{

	if ((whichBone < 0) || (whichBone >= boneData.Count()) ||
		(whichMorph < 0) || (whichMorph >= boneData[whichBone]->morphTargets.Count()))
	{
		return;
	}	



	MorphTargets *morph = boneData[whichBone]->morphTargets[whichMorph];
	theHold.Begin();
	theHold.Put(new ResetOrientationMorphRestore(this,morph));
	theHold.Accept(GetString(IDS_RESETORIENTATION));
	morph->boneTMInParentSpace = boneData[whichBone]->currentBoneNodeTM * Inverse(boneData[whichBone]->parentBoneNodeCurrentTM);
}


void MorphByBone::ResetSelectionGraph()
	{

	ReferenceTarget *ref;

	pblock->GetValue(pb_selectiongraph,0,ref, FOREVER);
	curveCtl = (ICurveCtl *) ref;

	curveCtl->SetXRange(0.0f,1.0f);
	curveCtl->SetYRange(-1.2f,1.2f);

	curveCtl->SetNumCurves(1);

	curveCtl->SetZoomValues(110.0f,70.4f);
	curveCtl->SetScrollValues(-14,-34);

	BitArray ba;
	ba.SetSize(1);
	ba.Set(0);//ba.Set(0);ba.Set(0);ba.Set(3);
	curveCtl->SetDisplayMode(ba);
	

	ICurve *pCurve = NULL;
	

	pCurve = curveCtl->GetControlCurve(0);

	pCurve->SetNumPts(2);
	CurvePoint pt;

	pt.flags = CURVEP_CORNER | CURVEP_BEZIER | CURVEP_ENDPOINT;
	pt.out.x = 0.33f;
	pt.out.y = 0.0f;
	pt.in.x = -0.33f;
	pt.in.y = 0.0f;

	pt.p.x = 0.0f;			
	pt.p.y = 1.0f;			
	pCurve->SetPoint(0,0,&pt);
		
	pt.p.x = 1.0f;			
	pt.p.y = 0.0f;			
	pt.in.x = -.33f;
	pt.in.y = 0.0f;
	pt.out.x = .33f;
	pt.out.y = 0.0f;

	pCurve->SetPoint(0,1,&pt);
	pCurve->SetLookupTableSize(100);

	curveCtl->Redraw();
	}

void MorphByBone::GrowVerts(BOOL grow)
{
	HoldSelections();
	BOOL useSoftSelection;
	pblock->GetValue(pb_usesoftselection,0,useSoftSelection,FOREVER);
	float radius;
	pblock->GetValue(pb_selectionradius,0,radius,FOREVER);
	
	for (int i = 0; i < localDataList.Count(); i++)
	{	
		MorphByBoneLocalData *ld = localDataList[i];
		
		if (ld)
		{
			BitArray sel;
			sel.SetSize(ld->softSelection.Count());
			sel.ClearAll();
			for (int i = 0; i < ld->softSelection.Count(); i++)
			{
				if (ld->softSelection[i] == 1.0f)
					sel.Set(i,TRUE);
			}
			if (grow)
				GrowVertSel(sel, ld->selfNode,GROW_SEL);
			else GrowVertSel(sel, ld->selfNode,SHRINK_SEL);

			for (int i = 0; i < ld->softSelection.Count(); i++)
			{
				if (sel[i])
					ld->softSelection[i] = 1.0f;
				else ld->softSelection[i] = 0.0f;
					
			}

		//recompute softselection
			if (useSoftSelection)
			{
				BOOL useEdgeLimit;
				int edgeLimit;
				pblock->GetValue(pb_useedgelimit,0,useEdgeLimit,FOREVER);
				pblock->GetValue(pb_edgelimit,0,edgeLimit,FOREVER);
				ld->ComputeSoftSelection(radius, pCurve, useEdgeLimit, edgeLimit);
			}
		}
	}
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

BOOL MorphByBone::GrowVertSel(BitArray &sel, INode *node, int mode)
{
	//get mesh
	ObjectState sos = node->EvalWorldState(GetCOREInterface()->GetTime());
	int numberVerts = sel.GetSize();

	if (numberVerts != sos.obj->NumPoints()) return FALSE;
		

//get the mesh
	//check if mesh or poly
	if (sos.obj->IsSubClassOf(triObjectClassID))
	{

		TriObject *tobj = (TriObject*)sos.obj;
		Mesh *tmsh = &tobj->GetMesh();

		if (tmsh->numVerts != numberVerts) return FALSE;

		BitArray origSel(sel);
		
		for (int i = 0; i < tmsh->numFaces; i++)
		{
			int deg = 3;
			int hit = 0;
			for (int j = 0; j < deg; j++)
			{	
				int a = tmsh->faces[i].v[j];
				if (origSel[a])
					hit ++;
			}
			if ((hit>0) && (mode == GROW_SEL))
			{
				for (int j = 0; j < deg; j++)
				{	
					int a = tmsh->faces[i].v[j];
					
					sel.Set(a,TRUE);	
										
				}			
			}
			else if ((hit!= deg) && (mode == SHRINK_SEL))
			{
				for (int j = 0; j < deg; j++)
				{	
					int a = tmsh->faces[i].v[j];
					
					sel.Set(a,FALSE);						
				}			
			}			

		}
	}
	//if poly object we can use that
	else if (sos.obj->IsSubClassOf(polyObjectClassID))
		{
		PolyObject *pobj = (PolyObject*)sos.obj;
		MNMesh *msh = &pobj->GetMesh();

		//convert vert sel to edge sel
		BitArray vsel, esel;

		if (msh->numv != numberVerts) return FALSE;

		BitArray origSel(sel);
		
		for (int i = 0; i < msh->numf; i++)
		{
			int deg = msh->f[i].deg;
			int hit = 0;
			for (int j = 0; j < deg; j++)
			{	
				int a = msh->f[i].vtx[j];
				if (origSel[a])
					hit++;
			}
			if ((hit>0) && (mode == GROW_SEL))
			{
				for (int j = 0; j < deg; j++)
				{	
					int a = msh->f[i].vtx[j];
					
					sel.Set(a,TRUE);
						
				}			
			}
			else if ((hit!= deg) && (mode == SHRINK_SEL))
			{
				for (int j = 0; j < deg; j++)
				{	
					int a = msh->f[i].vtx[j];
					
					sel.Set(a,FALSE);						
				}			
			}				


		}
	}
	return TRUE;
}


void MorphByBone::EdgeSel(BOOL loop)
{

	HoldSelections();
	BOOL useSoftSelection;
	pblock->GetValue(pb_usesoftselection,0,useSoftSelection,FOREVER);
	float radius;
	pblock->GetValue(pb_selectionradius,0,radius,FOREVER);
	
	for (int i = 0; i < localDataList.Count(); i++)
	{	
		MorphByBoneLocalData *ld = localDataList[i];
		
		if (ld)
		{
			BitArray sel;
			sel.SetSize(ld->softSelection.Count());
			sel.ClearAll();
			for (int i = 0; i < ld->softSelection.Count(); i++)
			{
				if (ld->softSelection[i] == 1.0f)
					sel.Set(i,TRUE);
			}
			if (loop)
				EdgeSel(sel, ld->selfNode,LOOP_SEL);
			else EdgeSel(sel, ld->selfNode,RING_SEL);

			for (int i = 0; i < ld->softSelection.Count(); i++)
			{
				if (sel[i])
					ld->softSelection[i] = 1.0f;
				else ld->softSelection[i] = 0.0f;
					
			}

		//recompute softselection
			if (useSoftSelection)
			{
				BOOL useEdgeLimit;
				int edgeLimit;
				pblock->GetValue(pb_useedgelimit,0,useEdgeLimit,FOREVER);
				pblock->GetValue(pb_edgelimit,0,edgeLimit,FOREVER);

				ld->ComputeSoftSelection(radius, pCurve, useEdgeLimit, edgeLimit);
			}
		}
	}
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}


BOOL MorphByBone::EdgeSel(BitArray &sel, INode *node, int mode)
{
	//get mesh
	ObjectState sos = node->EvalWorldState(GetCOREInterface()->GetTime());
	int numberVerts = sel.GetSize();
		

//get the mesh
	//check if mesh or poly
	if (sos.obj->IsSubClassOf(triObjectClassID))
		{

		TriObject *tobj = (TriObject*)sos.obj;
		Mesh *tmsh = &tobj->GetMesh();

		Object *obj = sos.obj->ConvertToType(GetCOREInterface()->GetTime(), polyObjectClassID);
		PolyObject *pobj = (PolyObject*)obj;
		MNMesh *msh = &pobj->GetMesh();

		if (tmsh->numVerts != numberVerts) return FALSE;


		Tab<int> vertIndexIntoPolyMesh;
		vertIndexIntoPolyMesh.SetCount(msh->numv);


		for (int i = 0; i < msh->numv; i++)
			{
			Point3 mnP = msh->v[i].p;
			for (int j = 0; j < tmsh->numVerts; j++)
				{
				Point3 p = tmsh->verts[j];
				if (p == mnP)
					{
					vertIndexIntoPolyMesh[i] = j;
					}
				}
			}

		//convert vert sel to edge sel
		BitArray vsel, esel;

		vsel.SetSize(numberVerts);
		vsel.ClearAll();
	//loop through the selection
		for ( int i =0; i < numberVerts; i++)
			{
			if (sel[i])
				vsel.Set(i);
			}

		esel.SetSize(msh->nume);
		esel.ClearAll();

		for (int i = 0; i < msh->nume; i++)
			{
			int a,b;
			a = msh->e[i].v1;
			b = msh->e[i].v2;
			a = vertIndexIntoPolyMesh[a];
			b = vertIndexIntoPolyMesh[b];
			if (vsel[a] && vsel[b])
				esel.Set(i);
			}


		//do loop
		if (mode == LOOP_SEL)
			msh->SelectEdgeLoop (esel);
		else if (mode == RING_SEL)
			msh->SelectEdgeRing (esel);


		//convert back to vert sel
		
		//pass that back
		sel.ClearAll();
		for (int i = 0; i < msh->nume; i++)
			{
			if (esel[i])
				{
				int a,b;
				a = msh->e[i].v1;
				b = msh->e[i].v2;

				a = vertIndexIntoPolyMesh[a];
				b = vertIndexIntoPolyMesh[b];

				sel.Set(a);
				sel.Set(b);
				}
			}
		obj->DeleteThis();

		}
	//if poly object we can use that
	else if (sos.obj->IsSubClassOf(polyObjectClassID))
		{
		PolyObject *pobj = (PolyObject*)sos.obj;
		MNMesh *msh = &pobj->GetMesh();

		//convert vert sel to edge sel
		BitArray vsel, esel;

		if (msh->numv != numberVerts) return FALSE;

		vsel.SetSize(numberVerts);
		vsel.ClearAll();
	//loop through the selection
		for (int i =0; i < numberVerts; i++)
			{
			if (sel[i])
				vsel.Set(i);
			}

		esel.SetSize(msh->nume);
		esel.ClearAll();

		for (int i = 0; i < msh->nume; i++)
			{
			int a,b;
			a = msh->e[i].v1;
			b = msh->e[i].v2;
			if (vsel[a] && vsel[b])
				esel.Set(i);
			}


		//do loop
		if (mode == LOOP_SEL)
			msh->SelectEdgeLoop (esel);
		else if (mode == RING_SEL)
		{
			BitArray tempGeSel;
			BitArray gesel = esel;
			tempGeSel = gesel;
			tempGeSel.ClearAll();


			//get the selected edge
			for (int i = 0; i < msh->nume; i++)
			{
				if (gesel[i])
				{
					//get the face atatched to that edge
					for (int j = 0; j < 2; j++)
					{
						//get all the visible edges attached to that face
						int currentFace = -1;
						if (j == 0)
							currentFace = msh->e[i].f1;
						else currentFace = msh->e[i].f2;

						if (currentFace != -1)
						{							
							int currentEdge = i;

							BOOL done = FALSE;
							int startEdge = currentEdge;
							BitArray edgesForThisFace;
							edgesForThisFace.SetSize(msh->nume);


							Tab<int> facesToProcess;
							BitArray processedFaces;
							processedFaces.SetSize(msh->numf);
							processedFaces.ClearAll();


							while (!done)
							{


								edgesForThisFace.ClearAll();
								facesToProcess.Append(1,&currentFace,100);
								while (facesToProcess.Count() > 0)
								{
									//pop the stack
									currentFace = facesToProcess[0];
									facesToProcess.Delete(0,1);

									processedFaces.Set(currentFace,TRUE);
									int numberOfEdges = msh->f[currentFace].deg;
									//loop through the edges
									for (int k = 0; k < numberOfEdges; k++)
									{
										//if edge is invisisble add the edges of the cross face
										int a = msh->f[currentFace].vtx[k];
										int b = msh->f[currentFace].vtx[(k+1)%numberOfEdges];
										if (a!=b)
										{
											int eindex = msh->f[currentFace].edg[k];
											edgesForThisFace.Set(eindex,TRUE);
										}

									}
								}
								//if odd edge count we are done
								if ( ((edgesForThisFace.NumberSet()%2) == 1) || (edgesForThisFace.NumberSet() <= 2))
									done = TRUE;
								else
								{
									//get the mid edge 
									//start at the seed
									int goal = edgesForThisFace.NumberSet()/2;
									int currentGoal = 0;
									int vertIndex = msh->e[currentEdge].v1;
									Tab<int> edgeList;
									for (int m = 0; m < edgesForThisFace.GetSize(); m++)
									{
										if (edgesForThisFace[m])
											edgeList.Append(1,&m,100);

									}
									while (currentGoal != goal)
									{
										//find next edge
										for (int i = 0; i < edgeList.Count(); i++)
										{
											int potentialEdge = edgeList[i];
											if (potentialEdge != currentEdge)
											{
												int a = msh->e[potentialEdge].v1;
												int b = msh->e[potentialEdge].v1;
												if (a == vertIndex)
												{
													vertIndex = b;
													currentEdge = potentialEdge;
													i = edgeList.Count();

													//increment current
													currentGoal++;
												}
												else if (b == vertIndex)
												{
													vertIndex = a;
													currentEdge = potentialEdge;
													i = edgeList.Count();

													//increment current
													currentGoal++;
												}
											}
										}


									}

								}

								if (tempGeSel[currentEdge])
									done = TRUE;
								tempGeSel.Set(currentEdge,TRUE);

								for (int m = 0; m < 2; m++)
								{

									int faceIndex = -1;
									if (m == 0)
										msh->e[currentEdge].v1;
									else msh->e[currentEdge].v2;

									if (faceIndex != -1)
									{									
										
										if ((faceIndex != currentFace) && (!processedFaces[faceIndex]))
										{
											currentFace = faceIndex;
											m = 2;
										}
									}
								}
								if ((msh->e[currentEdge].f1 == -1) || (msh->e[currentEdge].f2 == -1))
									done = TRUE;
								//if we hit the start egde we are done
								if (currentEdge == startEdge) 
									done = TRUE;
							}
						}
					}
				}
			}

		}

		//convert back to vert sel
		
		//pass that back
		sel.ClearAll();
		for (int i = 0; i < msh->nume; i++)
			{
			if (esel[i])
				{
				int a,b;
				a = msh->e[i].v1;
				b = msh->e[i].v2;

				sel.Set(a);
				sel.Set(b);
				}
			}


		}

	return TRUE;
}

void MorphByBone::HoldSelections()
{
	theHold.Begin();
	for (int i = 0; i < localDataList.Count(); i++)
	{	
		MorphByBoneLocalData *ld = localDataList[i];
		if (ld)
		{
			theHold.Put(new SelectionRestore(ld,this));
		}
	}
	theHold.Accept(GetString(IDS_PW_SELECT));
}

void MorphByBone::HoldBoneSelections()
{
	theHold.Begin();
	theHold.Put(new BoneSelectionRestore(this));
	theHold.Accept(GetString(IDS_PW_SELECT));
}


void MorphByBoneLocalData::BuildConnectionList(Object *obj)
{

	if (obj->NumPoints() == connectionList.Count()) return;

	FreeConnectionList();
	//loop through connection and allocate the data
	connectionList.SetCount(obj->NumPoints());
	for (int i = 0; i < obj->NumPoints();i++)
	{
		connectionList[i] = new EdgeConnections();
	}
	//see if mesh
	if (obj->IsSubClassOf(triObjectClassID))
	{

		TriObject *tobj = (TriObject*)obj;
		Mesh *msh = &tobj->GetMesh();
		for (int i = 0; i < msh->numFaces; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				int a = msh->faces[i].v[j];
				int b;
				if (j == 2)
					b = msh->faces[i].v[0];
				else b = msh->faces[i].v[j+1];

				connectionList[a]->verts.Append(1,&b,10);
				connectionList[b]->verts.Append(1,&a,10);
			}

		}


	}
	//else see if poly
	else if (obj->IsSubClassOf(polyObjectClassID))
		{
		PolyObject *pobj = (PolyObject*)obj;
		MNMesh *msh = &pobj->GetMesh();
		for (int i = 0; i < msh->numf; i++)
		{
			int deg = msh->f[i].deg;
			for (int j = 0; j < deg; j++)
			{
				int a = msh->f[i].vtx[j];
				int b;
				if (j == (deg-1))
					b = msh->f[i].vtx[0];
				else b = msh->f[i].vtx[j+1];

				connectionList[a]->verts.Append(1,&b,10);
				connectionList[b]->verts.Append(1,&a,10);
			}
		}
	}	
	
}
void MorphByBoneLocalData::FreeConnectionList()
{
	for (int i = 0; i < connectionList.Count(); i++)
	{
		if (connectionList[i])
			delete connectionList[i];
		connectionList[i] = NULL;
	}
	connectionList.ZeroCount();
}

void MorphByBone::AddBone(INode *node)
{
//find an empty param block
	int id = -1;

	Box3 bounds;
	bounds.Init();
	TimeValue t = GetCOREInterface()->GetTime();

	ObjectState os = node->EvalWorldState(t);
	Object *obj = os.obj;

	obj->GetDeformBBox(t, bounds);




	for (int j = 0; j < pblock->Count(pb_bonelist); j++)
		{
		INode *pnode = GetNode(j);
		if (pnode == node) return;
		if (pnode == NULL)
			{
			id = j;
			j = boneData.Count();
			}
		}
	
	if (id == -1)
		{
		MorphByBoneData *temp = new (MorphByBoneData);
		//add node		
		//get the self tm
		temp->intialBoneNodeTM = node->GetNodeTM(t);
		temp->intialBoneObjectTM = node->GetObjectTM(t);
		temp->localBounds = bounds;

		if (node->IsRootNode())
			{
			temp->intialParentNodeTM = Matrix3(1);
			}
		else
			{
			INode *pnode = node->GetParentNode();
			temp->intialParentNodeTM = pnode->GetNodeTM(t);
			}


		//add it
		boneData.Append(1,&temp);
		pblock->Append(pb_bonelist,1,&node);
		}
	else
		{
		
		boneData[id]->intialBoneNodeTM = node->GetNodeTM(t);
		boneData[id]->intialBoneObjectTM = node->GetObjectTM(t);

		if (node->IsRootNode())
			{
			boneData[id]->intialParentNodeTM = Matrix3(1);
			}
		else
			{
			INode *pnode = node->GetParentNode();
			boneData[id]->intialParentNodeTM = pnode->GetNodeTM(t);
			}

		

		boneData[id]->localBounds = bounds;
		pblock->SetValue(pb_bonelist,t,node,id);
		}

}


int	MorphByBone::GetBoneIndex(INode *node)
{

	for (int j = 0; j < pblock->Count(pb_bonelist); j++)
	{
		INode *pnode = GetNode(j);
		if (pnode == node) return j;
	}
	return -1;
}

TCHAR*	MorphByBone::GetMorphName(INode *node, int index)
{
	for (int j = 0; j < pblock->Count(pb_bonelist); j++)
	{
		INode *pnode = GetNode(j);
		if (pnode == node)
		{
			if ((index >= 0) && (index < boneData[j]->morphTargets.Count()))
				return boneData[j]->morphTargets[index]->name;
		}
	}

	return NULL;
}
int		MorphByBone::GetMorphIndex(INode *node, TCHAR *name)
{
	for (int j = 0; j < pblock->Count(pb_bonelist); j++)
	{
		INode *pnode = GetNode(j);
		if (pnode == node)
		{
			for (int k = 0; k < boneData[j]->morphTargets.Count(); k++)
			{
				if (strcmp(name,boneData[j]->morphTargets[k]->name) == 0)
					return k;
			}
		}
	}

	return -1;
}


