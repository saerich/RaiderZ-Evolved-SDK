/**********************************************************************
 *<
	FILE: PatchDeformPW.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 
	bug resmaple not undoable

	add exposure to get the param space data

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "SkinWrapPatch.h"

static PatchDeformPWClassDesc PatchDeformPWDesc;
ClassDesc2* GetPatchDeformPWDesc() { return &PatchDeformPWDesc; }



IObjParam *PatchDeformPW::ip			= NULL;


static ParamBlockDesc2 patchdeformpw_param_blk ( IPatchDeformPWMod::patchdeformpw_params, _T("params"),  0, &PatchDeformPWDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_PANEL, IDS_PARAMS, 0, 0, NULL,
	// params
	IPatchDeformPWMod::pb_patch, 	_T("patch"),		TYPE_INODE, 		0,				IDS_PW_PATCH,
		p_ui, 			TYPE_PICKNODEBUTTON, 	IDC_PATCH, 
		p_classID, patchObjectClassID,
		end, 

	IPatchDeformPWMod::pb_samplerate, 			_T("sampleRate"), 		TYPE_INT, 	0, 	IDS_SAMPLERATE, 
		p_default, 		100, 
		p_range, 		10,100000, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_INT, IDC_SAMPLERATE,	IDC_IDC_SAMPLERATE_SPIN, 1.0f, 
		end,

	end
	);


//Function Publishing descriptor for Mixin interface
//*****************************************************
static FPInterfaceDesc patchdeformpw_interface(
   PATCHDEFORMPW_INTERFACE, _T("patchDeformOps"), 0, &PatchDeformPWDesc, FP_MIXIN,


		patchdeform_resample, _T("resample"), 0, TYPE_VOID, 0, 0,

      end
      );


int PatchEnumProc::proc(ReferenceMaker *rmaker) 
	{ 
		if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
		{
			Nodes.Append(1, (INode **)&rmaker);  
			return DEP_ENUM_SKIP;
		}

		return DEP_ENUM_CONTINUE;
	}


INT_PTR PatchDeformParamsMapDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
			
	{


	switch (msg) 
		{
		case WM_INITDIALOG:
			{
			mod->hWnd = hWnd;
			break;
			}
		case WM_COMMAND:
			{
			switch (LOWORD(wParam)) 
				{

				case IDC_RESAMPLE:
					{
					macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Wrap_Patch].patchDeformOps.resample"), 0, 0);

					mod->Resample();
					break;
					}
				}

			break;
			}



		}
	return FALSE;
	}


FPInterfaceDesc* IPatchDeformPWMod::GetDesc()
	{
	 return &patchdeformpw_interface;
	}

void *PatchDeformPWClassDesc::Create(BOOL loading)
		{
		AddInterface(&patchdeformpw_interface);
		return  new PatchDeformPW();
		}


//--- PatchDeformPW -------------------------------------------------------
PatchDeformPW::PatchDeformPW()
{
	pblock = NULL;
	PatchDeformPWDesc.MakeAutoParamBlocks(this);
	hWnd=NULL;
}

PatchDeformPW::~PatchDeformPW()
{
}

/*===========================================================================*\
 |	The validity of the parameters.  First a test for editing is performed
 |  then Start at FOREVER, and intersect with the validity of each item
\*===========================================================================*/
Interval PatchDeformPW::LocalValidity(TimeValue t)
{
	// if being edited, return NEVER forces a cache to be built 
	// after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;  
	//TODO: Return the validity interval of the modifier
	return NEVER;
}


BOOL RecursePipeAndMatch(LocalPatchData *smd, Object *obj)
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
				if (mc->localData == smd)
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
			if (RecursePipeAndMatch(smd, bobj)) return TRUE;
			}

		}

	return FALSE;
}


INode* PatchDeformPW::GetNodeFromModContext(LocalPatchData *smd)
	{

	int	i;

    PatchEnumProc dep;              
	DoEnumDependents(&dep);
	for ( i = 0; i < dep.Nodes.Count(); i++)
		{
		INode *node = dep.Nodes[i];
		BOOL found = FALSE;

		if (node)
			{
			Object* obj = node->GetObjectRef();
	
			if ( RecursePipeAndMatch(smd,obj) )
				{
				return node;
				}
// look at the SkinWrapMesh.cpp code here....
			}
		}
	return NULL;
	}


void PatchDeformPW::BuildParamData(Object *obj, LocalPatchData *localData,PatchMesh *patch,Matrix3 patchTM,Matrix3 baseTM)
	{
	localData->numPatches = patch->numPatches;
//put our base mesh data into patch space
	Matrix3 tmFromBaseToPatch = baseTM * Inverse(patchTM);

	Matrix3 tmFromPatchToBase = patchTM * Inverse(baseTM);

	Tab<Point3> pointData;
	int pointCount = obj->NumPoints();
	pointData.SetCount(pointCount);

	Tab<int> closestPoint;
	closestPoint.SetCount(pointCount);

	localData->paramData.SetCount(pointCount);

	for (int i = 0; i < pointCount; i++)
		{
		Point3 p = obj->GetPoint(i);
		localData->paramData[i].initialLocalPoint = p;
		p = p * tmFromBaseToPatch;
		pointData[i] = p;
		closestPoint[i] = -1;
		}

	int numPatches = patch->numPatches;

//split the patches into 10x10 chunks
	Tab<Point3> patchPoints;
	patchPoints.SetCount(numPatches * 100);


	int ct = 0;
	for (int i = 0; i < numPatches; i++)
		{
		Patch *p = &patch->patches[i];

		if (p->type == PATCH_QUAD)
			{
			float u,v;
			float inc = 1.0f/(10.0f+1.0f);
			u = inc;
			v = inc;
			for (int y = 0; y < 10; y++)
				{
				u = inc;
				for (int x = 0; x < 10; x++)
					{
					Point3 pt = p->interp(patch, u, v);
					patchPoints[ct] = pt;
					ct++;
					u += inc;
					}
				v += inc;
				}

			}
		else ct += 100;
		}

	for (int i = 0; i < pointCount; i++)
		{
		int closest = -1;
		float d= 0.0f;
		for (int j = 0; j <  ct; j++)
			{
			float len = LengthSquared(patchPoints[j]-pointData[i]);
			if ((closest == -1) || (len<d))
				{
				d = len;
				closest = j;
				}
			}
		closestPoint[i] = closest/100;		
		}

	BitArray usedPatches;
	usedPatches.SetSize(numPatches);
	usedPatches.ClearAll();
	for (int i = 0; i < pointCount; i++)
		{
		usedPatches.Set(closestPoint[i]);
		}

	int sampleRate;
	Interval iv;
	iv = FOREVER;

	pblock->GetValue(pb_samplerate,0,sampleRate,iv);
	patchPoints.SetCount(sampleRate*sampleRate);

	

	for (int i = 0; i < numPatches; i++)
		{
		Patch *p = &patch->patches[i];

		if (ip)
			{
			TSTR name;
			name.printf(GetString(IDS_COMPLETED_PCT),(float) i / numPatches *100.0f);
			SetWindowText(GetDlgItem(hWnd,IDC_STATUS),name);
			}

		float inc = 1.0f/(sampleRate+1.0f);

		if ( (usedPatches[i]) && (p->type == PATCH_QUAD))
			{
			float u,v;
			
			u = inc;
			v = inc;
			ct = 0;
			for (int y = 0; y < sampleRate; y++)
				{
				u = inc;
				for (int x = 0; x < sampleRate; x++)
					{
					Point3 pt = p->interp(patch, u, v);
					patchPoints[ct] = pt;
					ct++;
					u += inc;
					}
				v += inc;
				}

			
			for (int j = 0; j < pointCount; j++)
				{

				if ((ip) && ((j%10) == 0))
					{
					TSTR name;
					name.printf(GetString(IDS_COMPLETED_PCT_W_COUNT),(float) i / numPatches *100.0f,j,pointCount);
					SetWindowText(GetDlgItem(hWnd,IDC_STATUS),name);
					}
				if (closestPoint[j] == i)
					{
					int closest = -1;
					float d= 0.0f;

					for (int k = 0; k <  sampleRate*sampleRate; k++)
						{
						float len = LengthSquared(patchPoints[k]-pointData[j]);
						if ((closest == -1) || (len<d))
							{
							d = len;
							closest = k;							
							}
						}	
					
					localData->paramData[j].uv.y = float (closest/sampleRate);
					localData->paramData[j].uv.x = float (closest - (localData->paramData[j].uv.y*sampleRate));

					localData->paramData[j].uv.y = (localData->paramData[j].uv.y +1) * inc;
					localData->paramData[j].uv.x = (localData->paramData[j].uv.x +1) * inc;		
					localData->paramData[j].patchIndex = i;

//get the u vec
					float u = localData->paramData[j].uv.x;
					float v = localData->paramData[j].uv.y;
					float delta = 1.0f/(sampleRate+1.0f)*0.5f;
					float du = u-delta;
					float dv = v-delta;

if (du <= 0.0f) DebugPrint("error du 0.0f \n");
if (dv <= 0.0f) DebugPrint("error dv 0.0f \n");
if (du >= 1.0f) DebugPrint("error du 1.0f \n");
if (dv >= 1.0f) DebugPrint("error dv 1.0f \n");

					localData->incDelta = delta;

					Patch *p = &patch->patches[i];

					Point3 uVec = Normalize(p->interp(patch, du, v)-patchPoints[closest]);
//get the v vec
					Point3 vVec = Normalize(p->interp(patch, u, dv)-patchPoints[closest]);

					Point3 xAxis,yAxis,zAxis;
					xAxis = uVec;
					zAxis = CrossProd(uVec,vVec);
					yAxis = CrossProd(xAxis,zAxis);

					Point3 center = patchPoints[closest];

					Matrix3 tm(xAxis,yAxis,zAxis,center);
/*DebugPrint("init %d\n",j);*/
/*DebugPrint("%f %f %f\n",xAxis.x,xAxis.y,xAxis.z);
DebugPrint("%f %f %f\n",yAxis.x,yAxis.y,yAxis.z);
DebugPrint("%f %f %f\n",zAxis.x,zAxis.y,zAxis.z);
DebugPrint("%f %f %f\n",center.x,center.y,center.z);*/

					localData->paramData[j].initialPoint = pointData[j]*Inverse(tm);
//DebugPrint("init %d\n",j);

					}
				}
			
			}
		}

	if (ip)
		{
		TSTR name;
		name.printf("%s",GetString(IDS_PICK));
		SetWindowText(GetDlgItem(hWnd,IDC_STATUS),name);
		}

//split the patch into sub samples chunk
	}

/*************************************************************************************************
*
	ModifyObject will do all the work in a full modifier
    This includes casting objects to their correct form, doing modifications
	changing their parameters, etc
*
\************************************************************************************************/


void PatchDeformPW::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *incnode) 
{
	Interval iv = FOREVER;

	//TODO: Add the code for actually modifying the object
	//check is local data if not create it
	if (mc.localData == NULL)
		{
		LocalPatchData *localData = new LocalPatchData();
		mc.localData = localData;
		}

	LocalPatchData *localData = (LocalPatchData *)mc.localData;

	if (localData == NULL ) return;

	if (localData->selfNode == NULL)
		{
		localData->selfNode = GetNodeFromModContext(localData);
		}


	//check if patch count has changed if so update our uv space
	//if rebuild then 

	INode *node = NULL;
	pblock->GetValue(pb_patch,t,node,iv);


	if (node == NULL)
		{	
		os->obj->UpdateValidity(GEOM_CHAN_NUM,iv);	
		return;
		}

	ObjectState patchos = node->EvalWorldState(t);
	if ((patchos.obj) && (localData->selfNode))
		{
		iv &= patchos.obj->ObjectValidity(t);

		Matrix3 patchTM = node->GetObjectTM(t, &iv);
		Matrix3 baseTM = localData->selfNode->GetObjectTM(t, &iv);

		PatchObject *patchObj = (PatchObject *) patchos.obj;
		PatchMesh *patch = &patchObj->patch;
		BOOL patchValid = TRUE;
		for (int i =0; i < patch->numPatches; i++)
			{
			if (patch->patches[i].type != PATCH_QUAD)
				{
				patchValid = FALSE;
				}
			}

		if ((!patchValid) && (ip))
			{
			TSTR name;
			name.printf(GetString(IDS_ERROR_TRIPATCH));
			SetWindowText(GetDlgItem(hWnd,IDC_STATUS),name);
			}

		if ((patch->numPatches > 0) && patchValid)
			{
			
			if ( (patch->numPatches != localData->numPatches) ||  (localData->resample))
				{
//rebuild the param data
				BuildParamData(os->obj,localData,patch,patchTM,baseTM);
				
				}
			localData->resample= FALSE;
//setup deformer
			PatchDeformer deformer(localData,patch,baseTM,patchTM);
			os->obj->Deform(&deformer, TRUE);

			}

		}

	os->obj->UpdateValidity(GEOM_CHAN_NUM,iv);
}


void PatchDeformPW::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;
	PatchDeformPWDesc.BeginEditParams(ip, this, flags, prev);
	patchdeformpw_param_blk.SetUserDlgProc(new PatchDeformParamsMapDlgProc(this));

	TSTR name;
	name.printf("%s",GetString(IDS_PICK));
	SetWindowText(GetDlgItem(hWnd,IDC_STATUS),name);

}

void PatchDeformPW::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	PatchDeformPWDesc.EndEditParams(ip, this, flags, next);
	this->ip = NULL;
}



Interval PatchDeformPW::GetValidity(TimeValue t)
{
	Interval valid = FOREVER;
	//TODO: Return the validity interval of the modifier
	return valid;
}




RefTargetHandle PatchDeformPW::Clone(RemapDir& remap)
{
	PatchDeformPW* newmod = new PatchDeformPW();	
	newmod->ReplaceReference(PBLOCK_REF,remap.CloneRef(pblock));
	BaseClone(this, newmod, remap);
	return(newmod);
}


//From ReferenceMaker 
RefResult PatchDeformPW::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	//TODO: Add code to handle the various reference changed messages
	return REF_SUCCEED;
}

/****************************************************************************************
*
 	NotifyInputChanged is called each time the input object is changed in some way
 	We can find out how it was changed by checking partID and message
*
\****************************************************************************************/

void PatchDeformPW::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc)
{

}



//From Object
BOOL PatchDeformPW::HasUVW() 
{ 
	//TODO: Return whether the object has UVW coordinates or not
	return TRUE; 
}

void PatchDeformPW::SetGenUVW(BOOL sw) 
{  
	if (sw==HasUVW()) return;
	//TODO: Set the plugin internal value to sw				
}

IOResult PatchDeformPW::Load(ILoad *iload)
{
	//TODO: Add code to allow plugin to load its data
	
	return IO_OK;
}

IOResult PatchDeformPW::Save(ISave *isave)
{
	//TODO: Add code to allow plugin to save its data
	
	return IO_OK;
}

#define UVSPACE_DATA	0x1000
#define PATCHCOUNT_DATA	0x1010
#define INCDELTA_DATA	0x1020

IOResult PatchDeformPW::SaveLocalData(ISave *isave, LocalModData *pld)
{
LocalPatchData *p;
IOResult	res;
ULONG		nb;

p = (LocalPatchData*)pld;

isave->BeginChunk(UVSPACE_DATA);
int ct = p->paramData.Count();
res = isave->Write(&ct, sizeof(ct), &nb);
if (ct>0) 
{
	ParamSpaceData*	data = p->paramData.Addr(0);
	res = isave->Write(data, sizeof(ParamSpaceData)*p->paramData.Count(), &nb);
}
isave->EndChunk();

isave->BeginChunk(PATCHCOUNT_DATA);
ct = p->numPatches;
res = isave->Write(&ct, sizeof(ct), &nb);
isave->EndChunk();

isave->BeginChunk(INCDELTA_DATA);
float d = p->incDelta;
res = isave->Write(&d, sizeof(d), &nb);
isave->EndChunk();




return IO_OK;
}

IOResult PatchDeformPW::LoadLocalData(ILoad *iload, LocalModData **pld)

{
	IOResult	res;
	ULONG		nb;

	LocalPatchData *p= new LocalPatchData();
	while (IO_OK==(res=iload->OpenChunk())) 
		{
		switch(iload->CurChunkID())  
			{
			case UVSPACE_DATA:
				{
				int ct;
				iload->Read(&ct,sizeof(int), &nb);
				p->paramData.SetCount(ct);
				if (ct>0)
				{
					ParamSpaceData *data = p->paramData.Addr(0);
				
					iload->Read(data,sizeof(ParamSpaceData)*p->paramData.Count(), &nb);
				}
				break;
				}
			case PATCHCOUNT_DATA:
				{
				int ct;
				iload->Read(&ct,sizeof(int), &nb);
				p->numPatches = ct;
				break;
				}
			case INCDELTA_DATA:
				{
				float d;
				iload->Read(&d,sizeof(d), &nb);
				p->incDelta = d;
				break;
				}
			}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
		}

	*pld = p;


return IO_OK;

}



BOOL RecursePipeAndMatch2(PatchDeformPW *pmod, Object *obj,BOOL resample)
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
				if (pmod == dobj->GetModifier(j))
					{
					ModContext *mc = dobj->GetModContext(j);
					((LocalPatchData*)(mc->localData))->resample = resample;
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
			if (RecursePipeAndMatch2(pmod, bobj,resample)) return TRUE;
			}

		}

	return FALSE;
}


void PatchDeformPW::SetResampleModContext(BOOL resample)
{
    PatchEnumProc dep;              
	DoEnumDependents(&dep);
	for ( int i = 0; i < dep.Nodes.Count(); i++)
		{
		INode *node = dep.Nodes[i];
		BOOL found = FALSE;

		if (node)
			{
			Object* obj = node->GetObjectRef();
	
			 RecursePipeAndMatch2(this,obj,resample) ;
			}
		}
}

LocalPatchData * RecursePipeAndMatchLD(Modifier *mod, Object *obj)
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

					if (mod == dobj->GetModifier(j))
					{
						ModContext *mc = dobj->GetModContext(j);
						return (LocalPatchData *) mc->localData;
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
			return RecursePipeAndMatchLD(mod, bobj);
			}

		}

	return NULL;
}


LocalPatchData* PatchDeformPW::GetLocalDataFromNode(INode *node)
	{

	if (node)
		{
		Object* obj = node->GetObjectRef();
	
		return RecursePipeAndMatchLD(this,obj);
		}
	return NULL;
	}


void PatchDeformPW::fnResample()
{
	Resample();
}

void PatchDeformPW::Resample()
{
	SetResampleModContext(TRUE);
	NotifyDependents(FOREVER,GEOM_CHANNEL,REFMSG_CHANGE);
}

int PatchDeformPW::fnGetNumberOfPoints(INode *node)
{
	return GetNumberOfPoints(node);
}
Point3* PatchDeformPW::fnGetPointUVW(INode *node, int index)
{
	LocalPatchData *ld = GetLocalDataFromNode(node);
	index--;
	if ((index < 0) || (index >= ld->paramData.Count()))
		return NULL;
	return &ld->paramData[index].uv;
}
Point3* PatchDeformPW::fnGetPointLocalSpace(INode *node, int index)
{
	LocalPatchData *ld = GetLocalDataFromNode(node);
	index--;
	if ((index < 0) || (index >= ld->paramData.Count()))
		return NULL;
	return &ld->paramData[index].initialPoint;

}
Point3* PatchDeformPW::fnGetPointPatchSpace(INode *node, int index)
{
	LocalPatchData *ld = GetLocalDataFromNode(node);
	index--;
	if ((index < 0) || (index >= ld->paramData.Count()))
		return NULL;
	return &ld->paramData[index].initialLocalPoint;

}
int PatchDeformPW::fnGetPointPatchIndex(INode *node, int index)
{
	index -= 1;
	return GetPointPatchIndex(node,index);
}

int PatchDeformPW::GetNumberOfPoints(INode *node)
{
	LocalPatchData *ld = GetLocalDataFromNode(node);
	return ld->paramData.Count();
}
Point3 PatchDeformPW::GetPointUVW(INode *node, int index)
{
	LocalPatchData *ld = GetLocalDataFromNode(node);
	if ((index < 0) || (index >= ld->paramData.Count()))
		return Point3(0.0f,0.0f,0.0f);
	return ld->paramData[index].uv;
}
Point3 PatchDeformPW::GetPointLocalSpace(INode *node, int index)
{
	LocalPatchData *ld = GetLocalDataFromNode(node);
	if ((index < 0) || (index >= ld->paramData.Count()))
		return Point3(0.0f,0.0f,0.0f);
	return ld->paramData[index].initialPoint;

}
Point3 PatchDeformPW::GetPointPatchSpace(INode *node, int index)
{
	LocalPatchData *ld = GetLocalDataFromNode(node);
	if ((index < 0) || (index >= ld->paramData.Count()))
		return Point3(0.0f,0.0f,0.0f);
	return ld->paramData[index].initialLocalPoint;

}
int PatchDeformPW::GetPointPatchIndex(INode *node, int index)
{
	LocalPatchData *ld = GetLocalDataFromNode(node);
	if ((index < 0) || (index >= ld->paramData.Count()))
		return -1;
	return ld->paramData[index].patchIndex;

}


