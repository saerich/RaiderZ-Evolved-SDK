/**********************************************************************
 *<
	FILE: MeshDeformPW.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 

	fix the weight all check put a hold state that 
/

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "SkinWrapMesh.h"
#include <maxheapdirect.h>

SelectModBoxCMode *MeshDeformPW::selectMode      = NULL;
ICustButton			*MeshDeformPW::iPickButton   = NULL;

static MeshDeformPWClassDesc MeshDeformPWDesc;
ClassDesc2* GetMeshDeformPWDesc() { return &MeshDeformPWDesc; }

static GenSubObjType SOT_Points(13);


IObjParam *MeshDeformPW::ip			= NULL;

static PickControlNode thePickMode;

class MeshDeformPBAccessor : public PBAccessor
{ 
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		MeshDeformPW* p = (MeshDeformPW*)owner;
		switch (id)
		{

			case pb_falloff:
			case pb_distance:
			case pb_facelimit:
				p->Resample();
				break;
			case pb_engine:
				p->Resample();
				if (p->ip)
					p->UpdateUI();
				break;


		}
	}
	void TabChanged(tab_changes changeCode, Tab<PB2Value>* tab, ReferenceMaker* owner, ParamID id, int tabIndex, int count)
	{
		if (id == pb_meshlist)
		{
			if (changeCode == tab_delete)
			{
				MeshDeformPW* p = (MeshDeformPW*)owner;
				for (int i = tabIndex; i < tabIndex + count; i++)
				{
					if (i < p->wrapMeshData.Count())
					{
						if (p->wrapMeshData[i])
							p->wrapMeshData[i]->numFaces = 0;
					}
				}
			}
		}
	}

	
};


static MeshDeformPBAccessor meshdeform_accessor;


static ParamBlockDesc2 meshdeformpw_param_blk ( meshdeformpw_params, _T("params"),  0, &MeshDeformPWDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI+P_MULTIMAP, PBLOCK_REF, 
	3,
	//rollout
	meshdeformpw_params,IDD_MESHPANEL, IDS_PARAMS, 0, 0, NULL,
	meshdeformpw_advanceparams,IDD_MESHPANELADVANCE, IDS_PARAMSADVANCE, 0, 0, NULL,
	meshdeformpw_displayparams,IDD_MESHPANELDISPLAY, IDS_PARAMSDISPLAY, 0, 0, NULL,
	// params

	pb_mesh, 	_T("mesh"),		TYPE_INODE, 		0,				IDS_PW_MESH,
		end, 


	pb_threshold, 			_T("threshold"), 		TYPE_FLOAT, 	0, 	IDS_THRESHOLD, 
		p_default, 		5.0f, 
		p_range, 		0.001f,100000.0f, 
		p_ui, 			meshdeformpw_params,		TYPE_SPINNER,		EDITTYPE_UNIVERSE, IDC_THRESHOLD,	IDC_THRESHOLD_SPIN2, 1.0f, 
		end,

	pb_engine, 			_T("engine"), 		TYPE_INT, 	0, 	IDS_ENGINE, 
		p_default, 		1, 
		p_ui, meshdeformpw_params,		TYPE_INTLISTBOX, IDC_ENGINE_COMBO, 2, IDS_FACE, IDS_VERTEX,
		p_accessor,	&meshdeform_accessor,
		end,

	pb_falloff, 			_T("falloff"), 		TYPE_FLOAT, 	0, 	IDS_FALLOFF, 
		p_default, 		1.0f, 
		p_range, 		0.001f,10.0f, 
		p_ui, 			meshdeformpw_params,		TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_FALLOFF,	IDC_IDC_FALLOFF_SPIN, 0.1f, 
		p_accessor,	&meshdeform_accessor,
		end,

	pb_distance, 		_T("distance"), 		TYPE_FLOAT, 	0, 	IDS_DISTANCE, 
		p_default, 		1.2f, 
		p_range, 		0.001f,10.0f, 
		p_ui, 			meshdeformpw_params,		TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_DISTANCE,	IDC_IDC_DISTANCE_SPIN, 0.1f, 
		p_accessor,	&meshdeform_accessor,
		end,

	pb_facelimit, 		_T("faceLimit"), 		TYPE_INT, 	0, 	IDS_FACELIMIT, 
		p_default, 		3, 
		p_range, 		0,30, 
		p_ui, 			meshdeformpw_params,		TYPE_SPINNER,		EDITTYPE_INT, IDC_FACELIMIT,	IDC_IDC_FACELIMIT_SPIN, 1.0f, 
		p_accessor,	&meshdeform_accessor,
		end,

	pb_showloops, 	_T("showLoops"), 	TYPE_BOOL, 		0,				IDS_SHOWLOOPS,
	    p_default,		TRUE,
		p_ui, 			meshdeformpw_displayparams,TYPE_SINGLECHEKBOX, 	IDC_DISPLAYLOOPCHECK, 
		end, 
	pb_showaxis, 	_T("showAxis"), 	TYPE_BOOL, 		0,				IDS_SHOWAXIS,
	    p_default,		TRUE,
		p_ui, 			meshdeformpw_displayparams,TYPE_SINGLECHEKBOX, 	IDC_DISPLAYAXISCHECK, 
		end, 
	pb_showfacelimit, 	_T("showFaceLimit"), 	TYPE_BOOL, 		0,	IDS_SHOWFACELIMIT,
	    p_default,		TRUE,
		p_ui, 			meshdeformpw_displayparams,TYPE_SINGLECHEKBOX, 	IDC_DISPLAYFACELIMITCHECK, 
		end, 

	pb_mirrorshowdata, 	_T("showMirrorData"), 	TYPE_BOOL, 		0,	IDS_SHOWMIRRORDATA,
	    p_default,		FALSE,
		p_ui, 			meshdeformpw_advanceparams,TYPE_SINGLECHEKBOX, 	IDC_SHOWMIRRORCHECK, 
		end, 

	pb_mirrorplane,  _T("mirrorPlane"), TYPE_INT, 	P_RESET_DEFAULT, 	IDS_MIRRORPLANE, 
		p_default, 		0,	
		p_range, 		0, 2, 
		p_ui, 			meshdeformpw_advanceparams, TYPE_RADIO,  3, IDC_XRADIO, IDC_YRADIO, IDC_ZRADIO,
		end, 
	pb_mirroroffset, 		_T("mirrorOffset"), 	TYPE_FLOAT, 	0, 	IDS_MIRROROFFSET, 
		p_default, 		0.0f, 
		p_range, 		-10000.0f,10000.0f, 
		p_ui, 			meshdeformpw_advanceparams,		TYPE_SPINNER,		EDITTYPE_UNIVERSE, IDC_MIRROROFFSET,	IDC_MIRROROFFSET_SPIN, 0.1f, 
		end,


	pb_debugpoint, 		_T(""), 		TYPE_INT, 	0, 	IDS_FACELIMIT,
		p_default, 		0, 
		end,

	pb_meshlist,    _T("meshList"),  TYPE_INODE_TAB,		0,	P_AUTO_UI|P_VARIABLE_SIZE,	IDS_MESHLIST,
		p_ui,		meshdeformpw_params, TYPE_NODELISTBOX, IDC_LIST1,0,0,IDC_REMOVE,
		p_accessor,	&meshdeform_accessor,
		end,

	pb_showunassignedverts, _T("showUnassignedVerts"),  	TYPE_BOOL, 		0,	IDS_SHOWUNASSIGNEDVERTS,
	    p_default,		FALSE,
		p_ui, 			meshdeformpw_displayparams,TYPE_SINGLECHEKBOX, 	IDC_SHOWUNASSIGNEDVERTSCHECK, 
		end, 

	pb_showverts, _T("showVerts"),  	TYPE_BOOL, 		0,	IDS_SHOWVERTS,
	    p_default,		TRUE,
		p_ui, 			meshdeformpw_displayparams,TYPE_SINGLECHEKBOX, 	IDC_SHOWVERTSCHECK, 
		end, 


	pb_weightallverts, _T("weightAllVerts"),  	TYPE_BOOL, 		0,	IDS_WEIGHTALL,
	    p_default,		FALSE,
		p_ui, 			meshdeformpw_params,TYPE_SINGLECHEKBOX, 	IDC_WEIGHTALLCHECK, 
		end, 

	pb_mirrorthreshold, 		_T("mirrorThreshold"), 	TYPE_FLOAT, 	0, 	IDS_MIRRORTHRESHOLD, 
		p_default, 		0.5f, 
		p_range, 		0.0f,10000.0f, 
		p_ui, 			meshdeformpw_advanceparams,		TYPE_SPINNER,		EDITTYPE_UNIVERSE, IDC_MIRRORTHRESHOLD,	IDC_MIRRORTHRESHOLD_SPIN, 0.1f, 
		end,

	pb_useblend, _T("blend"),  	TYPE_BOOL, 		0,	IDS_BLEND,
	    p_default,		FALSE,
	   p_enable_ctrls,		1,	pb_blenddistance,
		p_ui, 			meshdeformpw_params,TYPE_SINGLECHEKBOX, 	IDC_BLENDCHECK, 
		end, 

	pb_blenddistance, 		_T("blendDistance"), 	TYPE_FLOAT, 	0, 	IDS_BLENDDISTANCE, 
		p_default, 		5.0f, 
		p_range, 		0.0f,10000.0f, 
		p_ui, 			meshdeformpw_params,		TYPE_SPINNER,		EDITTYPE_UNIVERSE, IDC_BLEND,	IDC_BLEND_SPIN, 0.1f, 
		end,	

	end
	);


int MeshEnumProc::proc(ReferenceMaker *rmaker) 
	{ 
		if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
		{
			Nodes.Append(1, (INode **)&rmaker);
			return DEP_ENUM_SKIP;
		}

		return DEP_ENUM_CONTINUE;
	}




INT_PTR MeshDeformParamsMapDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
			
	{
	static float sx = 0.0f;
	static float sy = 0.0f;
	static float sz = 0.0f;

	switch (msg) 
		{
		case WM_INITDIALOG:
			{
			mod->hWnd = hWnd;
			mod->hWndParamPanel = hWnd;
			mod->UpdateXYZSpinners();
			mod->UpdateUI();

			mod->iPickButton = GetICustButton(GetDlgItem(hWnd, IDC_ADD));
			mod->iPickButton->SetType(CBT_CHECK);
			mod->iPickButton->SetHighlightColor(GREEN_WASH);

			break;
			}

		case WM_CUSTEDIT_ENTER:	
			if (LOWORD(wParam) == IDC_SCALEMULT) 
			{
				//get the multiplier
				float mult = mod->multSpin->GetFVal();
				theHold.Accept(GetString(IDS_SCALE));

//				mod->MultScale(mult);
				//
				//set back to 1
				mod->multSpin->SetValue(1.0f,FALSE);
				mod->UpdateXYZSpinners();
			}
			//hold weights
			//set weights
			//start hold
			if (LOWORD(wParam) == IDC_X) 
			{

				theHold.Accept(GetString(IDS_SCALE));				
				//set weights
//				mod->UpdateScale(0,mod->xSpin->GetFVal());
				macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Wrap].meshDeformOps.setLocalScale"), 2, 0,
						mr_int,1,
						mr_float,mod->xSpin->GetFVal());

			}

			if (LOWORD(wParam) == IDC_Y) 
			{
				theHold.Accept(GetString(IDS_SCALE));				
				//set weights
//				mod->UpdateScale(1,mod->ySpin->GetFVal());
				macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Wrap].meshDeformOps.setLocalScale"), 2, 0,
						mr_int,2,
						mr_float,mod->ySpin->GetFVal());

			}

			if (LOWORD(wParam) == IDC_Z) 
			{
				theHold.Accept(GetString(IDS_SCALE));				
				//set weights
//				mod->UpdateScale(2,mod->zSpin->GetFVal());
				macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Wrap].meshDeformOps.setLocalScale"), 2, 0,
						mr_int,3,
						mr_float,mod->zSpin->GetFVal());

			}
			if (LOWORD(wParam) == IDC_STR) 
			{
				theHold.Accept(GetString(IDS_STR));				
				//set weights
//				mod->UpdateStr(mod->strSpin->GetFVal());
				macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Wrap].meshDeformOps.setLocalStr"), 1, 0,
						mr_float,mod->strSpin->GetFVal());

			}

			break;

		case CC_SPINNER_BUTTONDOWN:			{

			//start hold
			if  (LOWORD(wParam) == IDC_SCALEMULT_SPIN)
			{
				theHold.Begin();
				for (int i = 0; i < mod->pblock->Count(pb_meshlist); i++)
				{

					theHold.Put (new LocalScaleRestore (mod,i));
				}

			}

			if ( (LOWORD(wParam) == IDC_X_SPIN) ||
				 (LOWORD(wParam) == IDC_Y_SPIN) ||
				 (LOWORD(wParam) == IDC_Z_SPIN) )
				{
			//hold
				theHold.Begin();
				for (int i = 0; i < mod->pblock->Count(pb_meshlist); i++)
				{

					theHold.Put (new LocalScaleRestore (mod,i));
				}




				}
			if (LOWORD(wParam) == IDC_STR_SPIN) 
			{
				theHold.Begin();
				for (int i = 0; i < mod->pblock->Count(pb_meshlist); i++)
				{
					theHold.Put (new LocalStrRestore (mod,i));
				}

			}


			}

		case CC_SPINNER_CHANGE:
			if (LOWORD(wParam) == IDC_SCALEMULT_SPIN) 
			{
				if (!theHold.Holding())
				{
					theHold.Begin();
					for (int i = 0; i < mod->pblock->Count(pb_meshlist); i++)
					{
						theHold.Put (new LocalScaleRestore (mod,i));
					}
				}
				//restore
				theHold.Restore();
				//set weights
				mod->MultScale(mod->multSpin->GetFVal());
				mod->UpdateXYZSpinners(FALSE);
			}
			else if (LOWORD(wParam) == IDC_X_SPIN) 
			{
				if (!theHold.Holding())
				{
					theHold.Begin();
					for (int i = 0; i < mod->pblock->Count(pb_meshlist); i++)
					{
						theHold.Put (new LocalScaleRestore (mod,i));
					}
				}
				//restore
				theHold.Restore();
				//set weights
				mod->UpdateScale(0,mod->xSpin->GetFVal());
			}
			else if (LOWORD(wParam) == IDC_Y_SPIN) 
			{
				if (!theHold.Holding())
				{
					theHold.Begin();
					for (int i = 0; i < mod->pblock->Count(pb_meshlist); i++)
					{
						theHold.Put (new LocalScaleRestore (mod,i));
					}
				}
				//restore
				theHold.Restore();
				//set weights
				mod->UpdateScale(1,mod->ySpin->GetFVal());
			}
			else if (LOWORD(wParam) == IDC_Z_SPIN) 
			{
				if (!theHold.Holding())
				{
					theHold.Begin();
					for (int i = 0; i < mod->pblock->Count(pb_meshlist); i++)
					{
						theHold.Put (new LocalScaleRestore (mod,i));
					}
				}
				theHold.Restore();
				mod->UpdateScale(2,mod->zSpin->GetFVal());
			}
			else if (LOWORD(wParam) == IDC_STR_SPIN) 
			{
				if (!theHold.Holding())
				{
					theHold.Begin();
					for (int i = 0; i < mod->pblock->Count(pb_meshlist); i++)
					{
						theHold.Put (new LocalStrRestore (mod,i));
					}
				}
				theHold.Restore();
				mod->UpdateStr(mod->strSpin->GetFVal());
			}

			break;
		case CC_SPINNER_BUTTONUP:
			if (HIWORD(wParam) == FALSE)
			{
				if ( (LOWORD(wParam) == IDC_X_SPIN) ||
					 (LOWORD(wParam) == IDC_Y_SPIN) ||
					 (LOWORD(wParam) == IDC_Z_SPIN) ||
					 (LOWORD(wParam) == IDC_STR_SPIN) ||
					 (LOWORD(wParam) == IDC_SCALEMULT_SPIN)
					 )
					theHold.Cancel();
					mod->multSpin->SetValue(1.0f,FALSE);

			}
			else
			{
				if (LOWORD(wParam) == IDC_SCALEMULT_SPIN)
				{
					theHold.Accept(GetString(IDS_SCALE));
					mod->multSpin->SetValue(1.0f,FALSE);

				}
				if ( (LOWORD(wParam) == IDC_X_SPIN) ||
					 (LOWORD(wParam) == IDC_Y_SPIN) ||
					 (LOWORD(wParam) == IDC_Z_SPIN) 
					  )
				{
					theHold.Accept(GetString(IDS_SCALE));				
					if (LOWORD(wParam) == IDC_X_SPIN)
						macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Wrap].meshDeformOps.setLocalScale"), 2, 0,
								mr_int,1,
								mr_float,mod->xSpin->GetFVal());
					else if (LOWORD(wParam) == IDC_Y_SPIN)
						macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Wrap].meshDeformOps.setLocalScale"), 2, 0,
								mr_int,2,
								mr_float,mod->ySpin->GetFVal());
					else if (LOWORD(wParam) == IDC_Z_SPIN)
						macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Wrap].meshDeformOps.setLocalScale"), 2, 0,
								mr_int,3,
								mr_float,mod->zSpin->GetFVal());
				}
				if (LOWORD(wParam) == IDC_STR_SPIN)
				{
					macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Wrap].meshDeformOps.setLocalStr"), 1, 0,
						mr_float,mod->strSpin->GetFVal());
					theHold.Accept(GetString(IDS_STR));				
				}
			}
			break;

		case WM_COMMAND:
			{
			switch (LOWORD(wParam)) 
				{

				case IDC_ADD:
					{
					thePickMode.mod = mod;
					GetCOREInterface()->SetPickMode(&thePickMode);
					break;
					}

				case IDC_RESET:
					{
					macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Wrap].meshDeformOps.reset"), 0, 0);
					mod->Reset();
					break;
					}
				case IDC_CONVERTTOSKIN:
					{
			 			macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Wrap].meshDeformOps.convertToSkin"), 1, 0, 
												mr_bool, FALSE);

						mod->SkinWraptoSkin(FALSE);

					}
				}


			break;
			}



		}
	return FALSE;
	}

INT_PTR MeshDeformParamsAdvanceMapDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
			
	{
	switch (msg) 
		{
		case WM_INITDIALOG:
			{

			break;
			}


		case WM_COMMAND:
			{
			switch (LOWORD(wParam)) 
				{

				case IDC_MIRRORSELECTED:
					{
						macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Wrap].meshDeformOps.mirrorSelectedControlPoints"), 0, 0);
						mod->MirrorSelectedVerts();
						break;
					}
				case IDC_BAKE:
					{
						macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Wrap].meshDeformOps.bakeControlPoint"), 0, 0);
						mod->BakeControlPoints();
						break;
					}
				case IDC_RETRIEVE:
					{
						macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Wrap].meshDeformOps.retrieveControlPoint"), 0, 0);
						mod->RetreiveControlPoints();
						break;
					}
				}

			break;
			}



		}
	return FALSE;
	}


//Function Publishing descriptor for Mixin interface
//*****************************************************
static FPInterfaceDesc meshdeformpw_interface(
    MESHDEFORMPW_INTERFACE, _T("meshDeformOps"), 0, &MeshDeformPWDesc, FP_MIXIN,

		meshdeform_getselectedvertices, _T("getSelectedVertices"), 0, TYPE_BITARRAY, 0, 1,
			_T("wrapNodeIndex"),0,TYPE_INDEX,
		meshdeform_setselectedvertices, _T("setSelectVertices"), 0, TYPE_VOID, 0, 3,
			_T("wrapNodeIndex"),0,TYPE_INDEX,
			_T("sel"), 0, TYPE_BITARRAY,
			_T("update"), 0, TYPE_BOOL,

		meshdeform_getnumbercontrolpoints, _T("getNumberOfControlPoints"), 0, TYPE_INT, 0, 1,
			_T("wrapNodeIndex"),0,TYPE_INDEX,

		meshdeform_getcontrolpointscale, _T("getControlPointScale"), 0, TYPE_POINT3, 0, 2,
			_T("wrapNodeIndex"),0,TYPE_INDEX,
			_T("index"), 0, TYPE_INDEX,
		meshdeform_setcontrolpointscale, _T("setControlPointScale"), 0, TYPE_VOID, 0, 3,
		    _T("wrapNodeIndex"),0,TYPE_INDEX,
			_T("index"), 0, TYPE_INDEX,
			_T("scale"), 0, TYPE_POINT3,


		meshdeform_getcontrolpointstr, _T("getControlPointStr"), 0, TYPE_FLOAT, 0, 2,
		    _T("wrapNodeIndex"),0,TYPE_INDEX,
			_T("index"), 0, TYPE_INDEX,
		meshdeform_setcontrolpointstr, _T("setControlPointStr"), 0, TYPE_VOID, 0, 3,
		    _T("wrapNodeIndex"),0,TYPE_INDEX,
			_T("index"), 0, TYPE_INDEX,
			_T("str"), 0, TYPE_FLOAT,

		meshdeform_mirrorselectedcontrolpoints, _T("mirrorSelectedControlPoints"), 0, TYPE_VOID, 0, 0,
		meshdeform_bakecontrolpoints, _T("bakeControlPoint"), 0, TYPE_VOID, 0, 0,
		meshdeform_retrievecontrolpoints, _T("retrieveControlPoint"), 0, TYPE_VOID, 0, 0,
		meshdeform_resample, _T("reset"), 0, TYPE_VOID, 0, 0,

		meshdeform_setlocalscale, _T("setLocalScale"), 0, TYPE_VOID, 0, 2,
			_T("axis"), 0, TYPE_INDEX,
			_T("scale"), 0, TYPE_FLOAT,
		meshdeform_setlocalstr, _T("setLocalStr"), 0, TYPE_VOID, 0, 1,
			_T("str"), 0, TYPE_FLOAT,

		meshdeform_getcontrolpointinitialtm, _T("getControlPointInitialTM"), 0, TYPE_MATRIX3, 0, 2,
			_T("wrapNodeIndex"),0,TYPE_INDEX,
			_T("index"), 0, TYPE_INDEX,
		meshdeform_getcontrolpointcurrenttm, _T("getControlPointCurrentTM"), 0, TYPE_MATRIX3, 0, 2,
		    _T("wrapNodeIndex"),0,TYPE_INDEX, 
			_T("index"), 0, TYPE_INDEX,
		meshdeform_getcontrolpointdist, _T("getControlPointDist"), 0, TYPE_FLOAT, 0, 2,
		    _T("wrapNodeIndex"),0,TYPE_INDEX,
			_T("index"), 0, TYPE_INDEX,
		meshdeform_getcontrolpointxvert, _T("getControlPointXVert"), 0, TYPE_INT, 0, 2,
		    _T("wrapNodeIndex"),0,TYPE_INDEX,
			_T("index"), 0, TYPE_INDEX,

		meshdeform_converttoskin, _T("ConvertToSkin"), 0, TYPE_VOID, 0, 1,
			_T("silent"), 0, TYPE_BOOL,


		meshdeform_ouputvertexdata, _T("OutputVertexData"), 0, TYPE_VOID, 0, 2,
			_T("node"), 0, TYPE_INODE,
			_T("vertexIndex"), 0, TYPE_INT,

      end
      );

void *MeshDeformPWClassDesc::Create(BOOL loading)
		{
		AddInterface(&meshdeformpw_interface);
		return  new MeshDeformPW();
		}

FPInterfaceDesc* IMeshDeformPWMod::GetDesc()
	{
	 return &meshdeformpw_interface;
	}

//--- MeshDeformPW -------------------------------------------------------
MeshDeformPW::MeshDeformPW()
{
	oldFaceData = FALSE;
	pblock = NULL;
	MeshDeformPWDesc.MakeAutoParamBlocks(this);
	hWnd=NULL;
	xSpin = NULL;
	ySpin = NULL;
	zSpin = NULL;
	strSpin = NULL;
	multSpin = NULL;
	cageTopoChange = FALSE;
	resample = FALSE;
	oldLocalBaseTM = FALSE;
	loadOldFaceIndices = FALSE;
	useOldLocalBaseTM = FALSE;
}

MeshDeformPW::~MeshDeformPW()
{
	for (int i = 0; i < wrapMeshData.Count(); i++) 
	{ 
		delete wrapMeshData[i];
	}
}

/*===========================================================================*\
 |	The validity of the parameters.  First a test for editing is performed
 |  then Start at FOREVER, and intersect with the validity of each item
\*===========================================================================*/
Interval MeshDeformPW::LocalValidity(TimeValue t)
{
	// if being edited, return NEVER forces a cache to be built 
	// after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;  
	//TODO: Return the validity interval of the modifier
	return NEVER;
}


BOOL RecursePipeAndMatch(LocalMeshData *smd, Object *obj)
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


INode* MeshDeformPW::GetNodeFromModContext(LocalMeshData *smd)
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

			if ( RecursePipeAndMatch(smd,obj) )
			{
				return node;
			}
			if (obj != NULL)
			{
				IXRefItem* xi = IXRefItem::GetInterface(*obj);
				if (xi != NULL) 
				{
					obj = (Object*)xi->GetSrcItem();
					if (RecursePipeAndMatch(smd,obj))
					{							
						return node;
					}
				}
			}
		}
	}
	return NULL;
}
//this builds our initial tm for each vertex in the wrap mesh
void MeshDeformPW::GetTMFromVertex(TimeValue t,int wrapIndex, Mesh *msh, Point3 *vnorms, Matrix3 meshToWorld)
{
	Matrix3 tm(1);
	Interval ivalid;
	

	if ((wrapMeshData[wrapIndex]->initialData.Count() != msh->numVerts) || (cageTopoChange))
	{
		wrapMeshData[wrapIndex]->initialData.SetCount(msh->numVerts);
	}
	else return;

	for (int vid = 0; vid < msh->numVerts; vid++)
	{
		wrapMeshData[wrapIndex]->initialData[vid].dist = 0.0f;
		wrapMeshData[wrapIndex]->initialData[vid].localScale = Point3(1.0f,1.0f,1.0f);
		wrapMeshData[wrapIndex]->initialData[vid].str = 1.0f;
	}
//get our align vec

	AdjEdgeList edgeList(*msh);

//	BitArray potentialEdges;
//	potentialEdges.SetSize(msh->numVerts);

	Tab<int> potentialEdges;

	for (int vid = 0; vid < msh->numVerts; vid++)
		{
		
		int vertIndex = vid;
		Point3 centerVert = msh->verts[vertIndex];

//		potentialEdges.ClearAll();
		potentialEdges.SetCount(0);

		int last = -1;
		int numberOfConnectedEdges = edgeList.list[vid].Count();
		for (int ei =0; ei < numberOfConnectedEdges; ei++)
		{
			int i = edgeList.list[vid][ei];
			BOOL vis = edgeList.edges[i].Visible(msh->faces);
			if (vid == edgeList.edges[i].v[0])
			{
				last = edgeList.edges[i].v[1];
				if (vis)
//					potentialEdges.Set(edgeList.edges[i].v[1],TRUE);
				{
					int vIndex = edgeList.edges[i].v[1];
					potentialEdges.Append(1,&vIndex,100);
				}
			}
			else if (vid == edgeList.edges[i].v[1])
			{
				last = edgeList.edges[i].v[0];
				if (vis)
				{
					int vIndex = edgeList.edges[i].v[0];
					potentialEdges.Append(1,&vIndex,100);
				}
//					potentialEdges.Set(edgeList.edges[i].v[0],TRUE);
			}
		}
/*
		for (int i =0; i < edgeList.edges.Count(); i++)
		{
			BOOL vis = edgeList.edges[i].Visible(msh->faces);
			if (vid == edgeList.edges[i].v[0])
			{
				last = edgeList.edges[i].v[1];
				if (vis)
					potentialEdges.Set(edgeList.edges[i].v[1],TRUE);
			}
			else if (vid == edgeList.edges[i].v[1])
			{
				last = edgeList.edges[i].v[0];
				if (vis)
					potentialEdges.Set(edgeList.edges[i].v[0],TRUE);
			}
		}
*/
		if (last == -1) 
			last = vid;
//		if (potentialEdges.NumberSet() == 0)
		if (potentialEdges.Count() == 0)
			wrapMeshData[wrapIndex]->initialData[vid].xPoints = last;
		else
		{
			float x = 0.0f,y = 0.0f,z = 0.0f;
//			for (int k = 0; k < potentialEdges.GetSize(); k++)
			for (int k = 0; k < potentialEdges.Count(); k++)
			{
//				if (potentialEdges[k])
				{
					Point3 vec = Normalize(msh->verts[potentialEdges[k]]-centerVert);
					if (vec.x > x) x = vec.x;
					if (vec.y > y) y = vec.y;
					if (vec.z > z) z = vec.z;
				}
			}

			int xPoint = -1;
			float d = 0.0f;

			if ((y > x) && (y > z))
			{
//				for (int k = 0; k < potentialEdges.GetSize(); k++)
				for (int k = 0; k < potentialEdges.Count(); k++)
				{
//					if (potentialEdges[k])
					{
						Point3 vec = Normalize(msh->verts[potentialEdges[k]]-centerVert);
						if ((vec.y > d) || (xPoint == -1))
						{
							xPoint = potentialEdges[k];
							d = vec.y;
						}
					}
				}
			}
			else if ((z > y) && (x > x))
			{
//				for (int k = 0; k < potentialEdges.GetSize(); k++)
				for (int k = 0; k < potentialEdges.Count(); k++)
				{
//					if (potentialEdges[k])
					{
						Point3 vec = Normalize(msh->verts[potentialEdges[k]]-centerVert);
						if ((vec.z > d) || (xPoint == -1))
						{
							xPoint = potentialEdges[k];
							d = vec.z;
						}
					}
				}
			}
			else
			{
//				for (int k = 0; k < potentialEdges.GetSize(); k++)
				for (int k = 0; k < potentialEdges.Count(); k++)
				{
//					if (potentialEdges[k])
					{

						Point3 vec = Normalize(msh->verts[potentialEdges[k]]-centerVert);
						if ((vec.x > d) || (xPoint == -1))
						{
							xPoint = potentialEdges[k];
							d = vec.x;
						}
					}
				}
			}
			if (xPoint == vid)
			{
				DebugPrint("error %d\n",vid);
			}
			wrapMeshData[wrapIndex]->initialData[vid].xPoints = xPoint;
		}

	}

	for (int i =0; i < edgeList.edges.Count(); i++)
		{
		int a,b;
		a = edgeList.edges[i].v[0];
		b = edgeList.edges[i].v[1];
		Point3 pa,pb;
		pa = msh->verts[a];
		pb = msh->verts[b];
		float dist = Length(pa-pb);
		if (dist > wrapMeshData[wrapIndex]->initialData[a].dist) wrapMeshData[wrapIndex]->initialData[a].dist = dist;
		if (dist > wrapMeshData[wrapIndex]->initialData[b].dist) wrapMeshData[wrapIndex]->initialData[b].dist = dist;
		}
//now build our matrices
//get our align vec
	for (int vid = 0; vid < msh->numVerts; vid++)
		{
		Point3 xvec,yvec,zvec, t;
		Point3 pole;
		t =  msh->verts[vid];
		zvec = vnorms[vid];
		pole = msh->verts[wrapMeshData[wrapIndex]->initialData[vid].xPoints];
		pole = Normalize(pole - t);
		xvec = Normalize(CrossProd(zvec,pole));
		yvec = Normalize(CrossProd(zvec,xvec));
		wrapMeshData[wrapIndex]->initialData[vid].initialVertexTMs.SetRow(0,xvec);
		wrapMeshData[wrapIndex]->initialData[vid].initialVertexTMs.SetRow(1,yvec);
		wrapMeshData[wrapIndex]->initialData[vid].initialVertexTMs.SetRow(2,zvec);
		wrapMeshData[wrapIndex]->initialData[vid].initialVertexTMs.SetRow(3,t);
		}



}

void MeshDeformPW::RebuildSelectedWeights(LocalMeshData *localData,int wrapIndex, Matrix3 meshTM,Matrix3 baseTM, float falloff, float distance, int faceLimit)
{

	if (wrapMeshData[wrapIndex]->mesh == NULL) return;

	int count = localData->initialPoint.Count();
	//put our base mesh data into wrap mesh space
	Matrix3 tmFromBaseToMesh = baseTM * Inverse(meshTM);

	Tab<Point3> objPoints;
	objPoints.SetCount(localData->initialPoint.Count());
	//loop through our local space points and put them in wrap mesh space
	for (int i = 0; i < objPoints.Count(); i++)
	{
		objPoints[i] = localData->initialPoint[i] * tmFromBaseToMesh;
	}


	//build our tms that put a point into space of the control mesh point
	Tab<Matrix3> defTMs;
	defTMs.SetCount(wrapMeshData[wrapIndex]->initialData.Count());
	for (int i = 0; i < wrapMeshData[wrapIndex]->initialData.Count(); i++)
	{

		Matrix3 pTM = wrapMeshData[wrapIndex]->initialData[i].initialVertexTMs;
		pTM.Scale(Point3(wrapMeshData[wrapIndex]->initialData[i].dist*distance*wrapMeshData[wrapIndex]->initialData[i].localScale.x,
			wrapMeshData[wrapIndex]->initialData[i].dist*distance*wrapMeshData[wrapIndex]->initialData[i].localScale.y,
			wrapMeshData[wrapIndex]->initialData[i].dist*distance*wrapMeshData[wrapIndex]->initialData[i].localScale.z));
		defTMs[i] =  Inverse(pTM);
	}
	Tab<int> vertList;
	BitArray processedVerts;
	processedVerts.SetSize(wrapMeshData[wrapIndex]->mesh->numVerts);

	BitArray newVerts;
	newVerts.SetSize(wrapMeshData[wrapIndex]->mesh->numVerts);


	for (int j =0; j < localData->paramData.Count(); j++)
	{
		if (localData->potentialWeightVerts[j])
		{



			vertList.SetCount(0);
			processedVerts.ClearAll();
			//get our seed face
			int seedFace = localData->faceIndex[wrapIndex]->f[j];//wrapMeshData[wrapIndex]->faceIndex[j];//localData->paramData[j].faceIndex;

			if (seedFace >= 0)
			{

				//get our 3 vertices
				int a,b,c;
				a = wrapMeshData[wrapIndex]->mesh->faces[seedFace].v[0];
				b = wrapMeshData[wrapIndex]->mesh->faces[seedFace].v[1];
				c = wrapMeshData[wrapIndex]->mesh->faces[seedFace].v[2];

				//add them to our list
				vertList.Append(1,&a,100);
				vertList.Append(1,&b,100);
				vertList.Append(1,&c,100);
				processedVerts.Set(a,TRUE);
				processedVerts.Set(b,TRUE);
				processedVerts.Set(c,TRUE);

				Tab<int> outerEdge;
				outerEdge.Append(1,&a,100);
				outerEdge.Append(1,&b,100);
				outerEdge.Append(1,&c,100);


				//loop through our limit
				for (int k = 0; k < faceLimit; k++)
				{
					//loop through our newvertices
					newVerts.ClearAll();
					Tab<int> newOuterEdge;
					for (int m = 0; m < outerEdge.Count(); m++)
					{
						int vertexID = outerEdge[m];
						int numberOfConnectedVerts = localData->adjEdgeList[wrapIndex]->list[vertexID].Count();
						for (int n = 0; n < numberOfConnectedVerts; n++)
						{
							int edgeID = localData->adjEdgeList[wrapIndex]->list[vertexID][n];
							int a,b;
							a = localData->adjEdgeList[wrapIndex]->edges[edgeID].v[0];
							b = localData->adjEdgeList[wrapIndex]->edges[edgeID].v[1];
							if (!processedVerts[a])
							{
								processedVerts.Set(a,TRUE);
								newOuterEdge.Append(1,&a,100);
							}
							if (!processedVerts[b])
							{
								processedVerts.Set(b,TRUE);
								newOuterEdge.Append(1,&b,100);
							}
						}				
					}
					if (newOuterEdge.Count() == 0)
						k = faceLimit;
					else
					{
						vertList.Append(newOuterEdge.Count(), newOuterEdge.Addr(0), 100);
						outerEdge = newOuterEdge;
					}
				}
			}

			//now weight these verts

			for (int k = 0; k < vertList.Count(); k++)
			{
				Point3 tp = objPoints[j] * defTMs[vertList[k]];
				float dist = Length(tp);

				dist = 1.0f - dist ;
				if (dist > 0.0f)
				{
					BoneInfo temp;
					temp.dist = pow(dist,falloff)* wrapMeshData[wrapIndex]->initialData[vertList[k]].str;
					temp.normalizedWeight = 0.0f;
					temp.owningBone = vertList[k];
					temp.whichNode = wrapIndex;
					localData->weightData[j]->data.Append(1,&temp);
				}
			}


		}
	}


//get our potential cage point list
	//need our list of selected control point
	//loop through them
//put our base mesh data into mesh space
/*
	Matrix3 tmFromBaseToMesh = baseTM * Inverse(meshTM);


	Tab<Point3> objPoints;
	objPoints.SetCount(localData->initialPoint.Count());


	for (int i = 0; i < objPoints.Count(); i++)
	{

		objPoints[i] = localData->initialPoint[i] * tmFromBaseToMesh;
	}


	Tab<Matrix3> defTMs;
	defTMs.SetCount(wrapMeshData[wrapIndex]->initialData.Count());
	for (i = 0; i < wrapMeshData[wrapIndex]->initialData.Count(); i++)
	{

		Matrix3 pTM = wrapMeshData[wrapIndex]->initialData[i].initialVertexTMs;
		pTM.Scale(Point3(wrapMeshData[wrapIndex]->initialData[i].dist*distance*wrapMeshData[wrapIndex]->initialData[i].localScale.x,
					     wrapMeshData[wrapIndex]->initialData[i].dist*distance*wrapMeshData[wrapIndex]->initialData[i].localScale.y,
						 wrapMeshData[wrapIndex]->initialData[i].dist*distance*wrapMeshData[wrapIndex]->initialData[i].localScale.z));
 		defTMs[i] =  Inverse(pTM);
	}

	BitArray invalidVerts;
	invalidVerts.SetSize(localData->initialPoint.Count());
	invalidVerts.ClearAll();

	for (i = 0; i < wrapMeshData[wrapIndex]->selectedVerts.GetSize(); i++)
	{
		if (wrapMeshData[wrapIndex]->selectedVerts[i] && (localData->lookupList[i]))
		{
			for (int j =0; j < localData->lookupList[i]->neighorbors.Count(); j++)
			{
				
				int vindex = localData->lookupList[i]->neighorbors[j];
				invalidVerts.Set(vindex,TRUE);
				Point3 tp = objPoints[vindex] * defTMs[i];
				float dist = Length(tp);
				dist = 1.0f - dist ;

				int index = -1;
				for (int k = 0; k < localData->weightData[vindex]->data.Count(); k++)
				{
					if ((localData->weightData[vindex]->data[k].owningBone == i) &&
						(localData->weightData[vindex]->data[k].whichNode == wrapIndex))

					{
						index = k;
						k = localData->weightData[vindex]->data.Count();
					}
				}
				if (dist > 0.0f)
					{
						if (index == -1)
						{
							BoneInfo temp;
							temp.dist = pow(dist,falloff) * wrapMeshData[wrapIndex]->initialData[i].str;
							temp.normalizedWeight = 0.0f;
							temp.owningBone = i;
							temp.whichNode = wrapIndex;
							localData->weightData[vindex]->data.Append(1,&temp);
						}
						else
						{
							localData->weightData[vindex]->data[index].normalizedWeight = 0.0f;
							localData->weightData[vindex]->data[index].dist = pow(dist,falloff) * wrapMeshData[wrapIndex]->initialData[i].str;

						}
				
					}
				else
				{
					if (index != -1)
					{
						localData->weightData[vindex]->data.Delete(index,1);
					}
				}

			}

		}


			

	}
		

	for (i = 0; i < localData->weightData.Count(); i++)
	{
		if (invalidVerts[i])
		{
			float total =0.0f;
			for (int j = 0; j < localData->weightData[i]->data.Count(); j++)
				{
				localData->weightData[i]->data[j].normalizedWeight = 0.0f;
				total += localData->weightData[i]->data[j].dist;
				}

			if (total != 0.0f)
				{
				for (int j = 0; j < localData->weightData[i]->data.Count(); j++)
					{
					localData->weightData[i]->data[j].normalizedWeight = localData->weightData[i]->data[j].dist/total;
					}
				}
		}

	}

*/
}

void MeshDeformPW::FreeWeightData(Object *obj,LocalMeshData *localData)
{
	///free all our previous data since this reweights all out vertices
	localData->FreeWeightData();
	localData->weightData.SetCount(obj->NumPoints());

	for (int i = 0; i < localData->weightData.Count(); i++)
		localData->weightData[i] = new WeightData();
}

void MeshDeformPW::WeightAllVerts(LocalMeshData *localData)
{


	for (int i = 0; i < localData->weightData.Count(); i++)
	{
		int numberOfBones = localData->weightData[i]->data.Count();
		if (numberOfBones == 0)
		{


			int faceID = localData->paramData[i].faceIndex;
			int wrapIndex = localData->paramData[i].whichNode;
			if ((faceID >= 0) && (wrapIndex >= 0) && (wrapIndex < wrapMeshData.Count()))
			{
				Face *faces = wrapMeshData[wrapIndex]->mesh->faces;
				
				localData->weightData[i]->data.SetCount(3);
				localData->weightData[i]->data[0].dist = localData->paramData[i].bary[0];
				localData->weightData[i]->data[1].dist = localData->paramData[i].bary[1];
				localData->weightData[i]->data[2].dist = localData->paramData[i].bary[2];

				localData->weightData[i]->data[0].normalizedWeight = localData->paramData[i].bary[0];
				localData->weightData[i]->data[1].normalizedWeight = localData->paramData[i].bary[1];
				localData->weightData[i]->data[2].normalizedWeight = localData->paramData[i].bary[2];

				localData->weightData[i]->data[0].owningBone = faces[faceID].v[0];
				localData->weightData[i]->data[1].owningBone = faces[faceID].v[1];
				localData->weightData[i]->data[2].owningBone = faces[faceID].v[2];

				localData->weightData[i]->data[0].whichNode = localData->paramData[i].whichNode;
				localData->weightData[i]->data[1].whichNode = localData->paramData[i].whichNode;
				localData->weightData[i]->data[2].whichNode = localData->paramData[i].whichNode;
			}
			else
			{
				int nonAsssignedface = 0;
			}

		}
	}
}
//this build our per vertex weight data, basically each vertex that we are deforming has a list of vertices/weights
//on the wrap meshes that affect it
void MeshDeformPW::BuildWeightData(Object *obj,LocalMeshData *localData,int wrapIndex, Mesh *msh,Matrix3 meshTM,Matrix3 baseTM, float falloff, float distance, int faceLimit, BitArray pointsToProcess)
{

	int engine;
	pblock->GetValue(pb_engine,GetCOREInterface()->GetTime(),engine,FOREVER);

	int count = obj->NumPoints();
	//put our base mesh data into wrap mesh space
	Matrix3 tmFromBaseToMesh = baseTM * Inverse(meshTM);


	Tab<Point3> objPoints;
	objPoints.SetCount(obj->NumPoints());

	//we cache off our undeformed point list so we can do a rebuild weights on any frame of the deformation
	//we nuke this cache if the topohanges
	BOOL rebuildInitialPoints = FALSE;
	if (localData->initialPoint.Count() != obj->NumPoints())
	{
		rebuildInitialPoints = TRUE;
		localData->initialPoint.SetCount(obj->NumPoints());
	}

	//loop through our local space points and put them in wrap mesh space
	for (int i = 0; i < objPoints.Count(); i++)
	{
		//if a topo chane use the new points and set that as out cache
		if (rebuildInitialPoints)
		{
			localData->initialPoint[i] = obj->GetPoint(i);
			objPoints[i] = obj->GetPoint(i) * tmFromBaseToMesh;
		}
		//use the cache
		else objPoints[i] = localData->initialPoint[i] * tmFromBaseToMesh;
	}

/*
	int tid = localData->paramData[0].faceIndex;
	int ta = msh->faces[tid].v[0];
	int tb = msh->faces[tid].v[1];
	int tc = msh->faces[tid].v[2];
*/

	//build our tms that put a point into space of the control mesh point
	Tab<Matrix3> defTMs;
	defTMs.SetCount(wrapMeshData[wrapIndex]->initialData.Count());
	for (int i = 0; i < wrapMeshData[wrapIndex]->initialData.Count(); i++)
	{

		Matrix3 pTM = wrapMeshData[wrapIndex]->initialData[i].initialVertexTMs;
		pTM.Scale(Point3(wrapMeshData[wrapIndex]->initialData[i].dist*distance*wrapMeshData[wrapIndex]->initialData[i].localScale.x,
			wrapMeshData[wrapIndex]->initialData[i].dist*distance*wrapMeshData[wrapIndex]->initialData[i].localScale.y,
			wrapMeshData[wrapIndex]->initialData[i].dist*distance*wrapMeshData[wrapIndex]->initialData[i].localScale.z));
		defTMs[i] =  Inverse(pTM);
	}
	if (engine == 0)
	{

		for (int j =0; j < localData->paramData.Count(); j++)
		{
			int seedFace = localData->paramData[j].faceIndex;

			int whichWrap = localData->paramData[j].whichNode;

			if (seedFace >= 0)
			{
				localData->weightData[j]->data.SetCount(3);
				Point3 bry = localData->paramData[j].bary;
				if (falloff != 0.001f)
				{
					if (bry.x < 0.0f) bry.x = 0.0f;
					if (bry.y < 0.0f) bry.y = 0.0f;
					if (bry.z < 0.0f) bry.z = 0.0f;

					bry[0] = pow(bry.x,falloff);
					bry[1] = pow(bry.y,falloff);
					bry[2] = pow(bry.z,falloff);

					float sum = bry[0]+bry[1]+bry[2];
					bry[0] = bry[0]/sum;
					bry[1] = bry[1]/sum;
					bry[2] = bry[2]/sum;
				}

				localData->weightData[j]->data[0].dist = bry[0];
				localData->weightData[j]->data[0].normalizedWeight = localData->weightData[j]->data[0].dist;
				localData->weightData[j]->data[0].owningBone = wrapMeshData[whichWrap]->mesh->faces[seedFace].v[0];
				localData->weightData[j]->data[0].whichNode = localData->paramData[j].whichNode;

				localData->weightData[j]->data[1].dist = bry[1];
				localData->weightData[j]->data[1].normalizedWeight = localData->weightData[j]->data[1].dist;
				localData->weightData[j]->data[1].owningBone = wrapMeshData[whichWrap]->mesh->faces[seedFace].v[1];
				localData->weightData[j]->data[1].whichNode =localData->paramData[j].whichNode;

				localData->weightData[j]->data[2].dist = bry[2];
				localData->weightData[j]->data[2].normalizedWeight = localData->weightData[j]->data[2].dist;
				localData->weightData[j]->data[2].owningBone = wrapMeshData[whichWrap]->mesh->faces[seedFace].v[2];
				localData->weightData[j]->data[2].whichNode = localData->paramData[j].whichNode;



			}
			else localData->weightData[j]->data.SetCount(0);

		}
		
	}
	else
	{
		if (wrapIndex >= localData->adjEdgeList.Count())
		{
			for (int i = 0; i < msh->numVerts; i++)
			{
				BitArray facesUsed;
				facesUsed.SetSize(msh->numFaces);
				facesUsed.ClearAll();
				//gather up all the faces that touch this vertex
				Point3 p = msh->verts[i];


				//if we dont have an adjface list go to the old slow method, this occurs when there is animated 
				//topolgy and we are not editing since we only keep the adjface data when the UI is up

				BitArray selVerts;
				selVerts.SetSize(msh->numVerts);
				selVerts.ClearAll();
				for (int j = 0; j < msh->numFaces; j++)
				{

					int a,b,c;
					a = msh->faces[j].v[0];
					b = msh->faces[j].v[1];
					c = msh->faces[j].v[2];

					if (a == i) 
					{
						selVerts.Set(a);
						facesUsed.Set(j);
					}
					if (b == i) 
					{
						selVerts.Set(b);
						facesUsed.Set(j);
					}
					if (c == i) 
					{
						selVerts.Set(c);
						facesUsed.Set(j);
					}
				}

				for (int j = 0; j < faceLimit; j++)
				{
					BitArray holdVertSel;
					holdVertSel.SetSize(selVerts.GetSize());
					holdVertSel = selVerts;

					for (int k = 0; k < msh->numFaces; k++)
					{
						int a,b,c;
						a = msh->faces[k].v[0];
						b = msh->faces[k].v[1];
						c = msh->faces[k].v[2];

						if (holdVertSel[a] || holdVertSel[b] || holdVertSel[c])
						{
							selVerts.Set(a);
							selVerts.Set(b);
							selVerts.Set(c);
							facesUsed.Set(k);
						}
					}


				}	
				for (int j =0; j < localData->paramData.Count(); j++)
				{
					int fid = localData->paramData[j].faceIndex;
					if (facesUsed[fid])
					{
						Point3 tp = objPoints[j] * defTMs[i];
						//				float dist = Length(objPoints[j]-p);
						float dist = Length(tp);

						dist = 1.0f - dist ;
						if (dist > 0.0f)
						{

							BoneInfo temp;
							temp.dist = pow(dist,falloff)* wrapMeshData[wrapIndex]->initialData[i].str;
							temp.normalizedWeight = 0.0f;
							temp.owningBone = i;
							temp.whichNode = wrapIndex;

							localData->weightData[j]->data.Append(1,&temp);
						}
					}
				}

			}
		}
		else
		{
			Tab<int> vertList;
			BitArray processedVerts;
			processedVerts.SetSize(wrapMeshData[wrapIndex]->mesh->numVerts);

			BitArray newVerts;
			newVerts.SetSize(wrapMeshData[wrapIndex]->mesh->numVerts);


			for (int j =0; j < localData->paramData.Count(); j++)
			{

				if (!pointsToProcess[j])
					continue;
				if ((ip) && (localData->paramData.Count() > 10000))
				{
					TSTR prompt;
					if ((j%200) == 0)
					{
						float per = (float)j/(float)localData->paramData.Count()*100.0f;
						prompt.printf("%s %d : %d(%d) (%0.0f%%)", GetString(IDS_STEP2),wrapIndex, j,localData->paramData.Count(),per);
						ip->ReplacePrompt( prompt);
					}
				}


				vertList.SetCount(0);
				processedVerts.ClearAll();
				//get our seed face
				if (j >= localData->faceIndex[wrapIndex]->f.Count()/*wrapMeshData[wrapIndex]->faceIndex.Count()*/) 
					continue;

				int seedFace = localData->faceIndex[wrapIndex]->f[j];//wrapMeshData[wrapIndex]->faceIndex[j];//localData->paramData[j].faceIndex;

/*
				if ((seedFace >= 0) &&(seedFace < wrapMeshData[wrapIndex]->mesh->numFaces) && (processOnlyZeroWeights))
				{
					vertList.SetCount(wrapMeshData[wrapIndex]->mesh->numVerts);
					for (int vi = 0; vi < wrapMeshData[wrapIndex]->mesh->numVerts; vi++)
						vertList[vi] = vi;
				}

				else */
				if ((seedFace >= 0) &&(seedFace < wrapMeshData[wrapIndex]->mesh->numFaces))
				{

					//get our 3 vertices
					int a,b,c;
					a = wrapMeshData[wrapIndex]->mesh->faces[seedFace].v[0];
					b = wrapMeshData[wrapIndex]->mesh->faces[seedFace].v[1];
					c = wrapMeshData[wrapIndex]->mesh->faces[seedFace].v[2];

					//add them to our list
					vertList.Append(1,&a,100);
					vertList.Append(1,&b,100);
					vertList.Append(1,&c,100);
					processedVerts.Set(a,TRUE);
					processedVerts.Set(b,TRUE);
					processedVerts.Set(c,TRUE);

					Tab<int> outerEdge;
					outerEdge.Append(1,&a,100);
					outerEdge.Append(1,&b,100);
					outerEdge.Append(1,&c,100);


					//loop through our limit
					for (int k = 0; k < faceLimit; k++)
					{
					//loop through our newvertices
						newVerts.ClearAll();
						Tab<int> newOuterEdge;
						for (int m = 0; m < outerEdge.Count(); m++)
						{
							int vertexID = outerEdge[m];
							int numberOfConnectedVerts = localData->adjEdgeList[wrapIndex]->list[vertexID].Count();
							for (int n = 0; n < numberOfConnectedVerts; n++)
							{
								int edgeID = localData->adjEdgeList[wrapIndex]->list[vertexID][n];
								int a,b;
								a = localData->adjEdgeList[wrapIndex]->edges[edgeID].v[0];
								b = localData->adjEdgeList[wrapIndex]->edges[edgeID].v[1];
								if (!processedVerts[a])
								{
									processedVerts.Set(a,TRUE);
									newOuterEdge.Append(1,&a,100);
								}
								if (!processedVerts[b])
								{
									processedVerts.Set(b,TRUE);
									newOuterEdge.Append(1,&b,100);
								}
							}				
						}
						if (newOuterEdge.Count() == 0)
							k = faceLimit;
						else
						{
							vertList.Append(newOuterEdge.Count(), newOuterEdge.Addr(0), 100);
							outerEdge = newOuterEdge;
						}
					}
				}
				else
				{
					if (seedFace!=-1)
						DebugPrint("Face %d out of bounds\n",j);
				}

			//now weight these verts
				for (int k = 0; k < vertList.Count(); k++)
				{
					Point3 tp = objPoints[j] * defTMs[vertList[k]];
					float dist = Length(tp);

					dist = 1.0f - dist ;
					if (dist > 0.0f)
					{
						BoneInfo temp;
						temp.dist = pow(dist,falloff)* wrapMeshData[wrapIndex]->initialData[vertList[k]].str;
						temp.normalizedWeight = 0.0f;
						temp.owningBone = vertList[k];
						temp.whichNode = wrapIndex;

						localData->weightData[j]->data.Append(1,&temp);
					}
				}
			}
		}

	}

	if ((ip) && (localData->paramData.Count() > 10000))
	{
		TSTR prompt;
		prompt.printf(" ");
		ip->ReplacePrompt( prompt);
	}


}

void MeshDeformPW::NormalizeWeightData(LocalMeshData *localData)
{
	for (int i = 0; i < localData->weightData.Count(); i++)
		{
		float total =0.0f;
		for (int j = 0; j < localData->weightData[i]->data.Count(); j++)
			{
			localData->weightData[i]->data[j].normalizedWeight = 0.0f;
			total += localData->weightData[i]->data[j].dist;
			}

		if (total != 0.0f)
			{

			for (int j = 0; j < localData->weightData[i]->data.Count(); j++)
				{
				localData->weightData[i]->data[j].normalizedWeight = localData->weightData[i]->data[j].dist/total;
				}
			}

		}
}

void MeshDeformPW::BuildCurrentTMFromVertex(TimeValue t, int wrapIndex, Mesh *msh, Point3 *vnorms, Matrix3 meshTM, Matrix3 baseTM, Matrix3 initialBaseTM)
{
	Matrix3 tm(1);
//get node
//get mesh
	Interval ivalid;

	wrapMeshData[wrapIndex]->currentVertexTMs.SetCount(msh->numVerts);


//now build our matrices
//get our align vec
	for (int vid = 0; vid < msh->numVerts; vid++)
		{
		Point3 xvec,yvec,zvec, t;
		Point3 pole;
		t =  msh->verts[vid];
		zvec = vnorms[vid];
		pole = msh->verts[wrapMeshData[wrapIndex]->initialData[vid].xPoints];
		pole = Normalize(pole - t);
		xvec = Normalize(CrossProd(zvec,pole));
		yvec = Normalize(CrossProd(zvec,xvec));
		wrapMeshData[wrapIndex]->currentVertexTMs[vid].SetRow(0,xvec);
		wrapMeshData[wrapIndex]->currentVertexTMs[vid].SetRow(1,yvec);
		wrapMeshData[wrapIndex]->currentVertexTMs[vid].SetRow(2,zvec);
		wrapMeshData[wrapIndex]->currentVertexTMs[vid].SetRow(3,t);
		}

	wrapMeshData[wrapIndex]->defTMs.SetCount(msh->numVerts);
	for (int vid = 0; vid < msh->numVerts; vid++)
		{
		//need to get the point from initial local to world
		//from world to mesh initial
		
		//back to world using current tm
		//back to local using current tm
//		wrapMeshData[wrapIndex]->defTMs[vid] = initialBaseTM * Inverse(wrapMeshData[wrapIndex]->initialMeshTM) * Inverse(wrapMeshData[wrapIndex]->initialData[vid].initialVertexTMs) * wrapMeshData[wrapIndex]->currentVertexTMs[vid] * meshTM * Inverse(baseTM );
 		wrapMeshData[wrapIndex]->defTMs[vid] = initialBaseTM * Inverse(wrapMeshData[wrapIndex]->initialMeshTM) * Inverse(wrapMeshData[wrapIndex]->initialData[vid].initialVertexTMs) * wrapMeshData[wrapIndex]->currentVertexTMs[vid] * meshTM * Inverse(baseTM );


		}

}

void MeshDeformPW::BuildParamData(TimeValue t, Object *obj, LocalMeshData *localData,int wrapIndex, INode *node,Matrix3 meshTM,Matrix3 baseTM, float threshold)
	{
//put our base mesh data into mesh space
	Matrix3 tmFromBaseToMesh = baseTM * Inverse(meshTM);

	Matrix3 tmFromMeshToBase = meshTM * Inverse(baseTM);
//get the engine
	ReferenceTarget *ref = (ReferenceTarget *) CreateInstance(REF_TARGET_CLASS_ID, RAYMESHGRIDINTERSECT_CLASS_ID);
	if (ref == NULL) return;


	IRayMeshGridIntersect_InterfaceV1 *rt =  (IRayMeshGridIntersect_InterfaceV1 *) ref->GetInterface(RAYMESHGRIDINTERSECT_V1_INTERFACE);
	IRayMeshGridIntersect_InterfaceV2 *rt2 =  (IRayMeshGridIntersect_InterfaceV2 *) ref->GetInterface(RAYMESHGRIDINTERSECT_V2_INTERFACE);

//if no engine bail
	if (rt == NULL) return;

	//initial engine with mesh
	rt->fnFree();
	rt->fnInitialize(100);
	//add node
	rt->fnAddNode(node);
	rt->fnBuildGrid(t);
	//build grid


	//loop through all our points and find the closest face


	int pointCount = obj->NumPoints();
//	localData->paramData.SetCount(pointCount);

	Matrix3 inverseTM = Inverse(baseTM);
	
	Tab<Point3> worldSpacePoints;
	worldSpacePoints.SetCount(pointCount);
	Box3 bounds;
	bounds.Init();
	

//	wrapMeshData[wrapIndex]->faceIndex.SetCount(pointCount);
	localData->faceIndex[wrapIndex]->f.SetCount(pointCount);
	
	for (int i = 0; i < pointCount; i++)
	{
		Point3 p = obj->GetPoint(i);
		worldSpacePoints[i] = p * baseTM;
		bounds += worldSpacePoints[i];
	}

//	TimeValue t = GetCOREInterface()->GetTime();
	Object *nodeObj = node->EvalWorldState(t).obj;
	Box3 nodeBoundingBox;
	Matrix3 nodeTM = node->GetObjectTM(t);
	nodeObj->GetDeformBBox(t, nodeBoundingBox,&nodeTM);
	nodeBoundingBox.EnlargeBy(threshold*1.2f);
//	threshold = Length(bounds.pmax- bounds.pmin)/200.0f;

	for (int i = 0; i < pointCount; i++)
	{
		localData->faceIndex[wrapIndex]->f[i] = -1;
//		wrapMeshData[wrapIndex]->faceIndex[i] = -1;
	}

	for (int i = 0; i < pointCount; i++)
		{

	

//		wrapMeshData[wrapIndex]->faceIndex[i] = -1;

		ParamSpaceData *pd = localData->paramData.Addr(i);

		Point3 p = worldSpacePoints[i];		

		if (!nodeBoundingBox.Contains(p)) continue;

		rt2->fnClosestFaceThreshold(p,threshold);
		int index =  rt->fnGetClosestHit();
		int faceIndex =  rt->fnGetHitFace(index)-1;
		Point3 bary = rt->fnGetHitBary(index);
//get bary, face index and  perp dist
		float offset = rt->fnGetPerpDist(index);
		float dist = rt->fnGetHitDist(index);



		if ((ip) && (pointCount > 1000))
		{
			TSTR prompt;
			if ((i%100) == 0)
			{
				float per = (float)i/(float)pointCount*100.0f;
				prompt.printf("%s %d : %d(%d) (%0.0f%%)", GetString(IDS_STEP1),wrapIndex, i,pointCount,per);
				ip->ReplacePrompt( prompt);
			}
		}
		
		
		localData->faceIndex[wrapIndex]->f[i] = faceIndex;
//		wrapMeshData[wrapIndex]->faceIndex[i] = faceIndex;

		if ((dist < pd->dist) || (pd->dist == -1.0f))
			{
				if (faceIndex != -1)
				{
					Point3 norm = rt->fnGetHitNorm(index);
					norm *= offset;
					norm = VectorTransform(norm,inverseTM);
					pd->bary = bary;
					pd->offset = offset;
					pd->dist = dist;
					pd->faceIndex = faceIndex;
					pd->whichNode = wrapIndex;
				}

			}

		SHORT iret = GetAsyncKeyState (VK_ESCAPE);
		if (iret==-32767)
		{
			i = pointCount;
			iret = GetAsyncKeyState (VK_ESCAPE);
		}


		}

	//rt->fnPrintStats();


	//delete the engine

	theHold.Suspend();
	ref->DeleteMe();
	theHold.Resume();

//build our weight data

		// Evaluate the node

	if (ip)
		{
		TSTR name;
		name.printf("Pick Mesh");
		SetWindowText(GetDlgItem(hWnd,IDC_STATUS),name);
		}

	if ((ip) && (localData->paramData.Count() > 1000))
	{
		TSTR prompt;
		prompt.printf(" ");
		ip->ReplacePrompt( prompt);
	}
	

	}

void MeshDeformPW::BuildParamDataForce(TimeValue t, Object *obj, LocalMeshData *localData,int wrapIndex, INode *node,Matrix3 meshTM,Matrix3 baseTM, float threshold, BitArray pointsToProcess)
	{

//put our base mesh data into mesh space
	Matrix3 tmFromBaseToMesh = baseTM * Inverse(meshTM);

	Matrix3 tmFromMeshToBase = meshTM * Inverse(baseTM);
//get the engine
	ReferenceTarget *ref = (ReferenceTarget *) CreateInstance(REF_TARGET_CLASS_ID, RAYMESHGRIDINTERSECT_CLASS_ID);
	if (ref == NULL) return;


	IRayMeshGridIntersect_InterfaceV1 *rt =  (IRayMeshGridIntersect_InterfaceV1 *) ref->GetInterface(RAYMESHGRIDINTERSECT_V1_INTERFACE);
	IRayMeshGridIntersect_InterfaceV2 *rt2 =  (IRayMeshGridIntersect_InterfaceV2 *) ref->GetInterface(RAYMESHGRIDINTERSECT_V2_INTERFACE);

//if no engine bail
	if (rt == NULL) return;

//initial engine with mesh
	rt->fnFree();
	rt->fnInitialize(100);
//add node
	rt->fnAddNode(node);
	rt->fnBuildGrid(t);
	//build grid


	//loop through all our points and find the closest face


	int pointCount = obj->NumPoints();
//	localData->paramData.SetCount(pointCount);

	Matrix3 inverseTM = Inverse(baseTM);
	
	Tab<Point3> worldSpacePoints;
	worldSpacePoints.SetCount(pointCount);
	Box3 bounds;
	bounds.Init();
	

//	wrapMeshData[wrapIndex]->faceIndex.SetCount(pointCount);
	localData->faceIndex[wrapIndex]->f.SetCount(pointCount);
	
	for (int i = 0; i < pointCount; i++)
	{
		Point3 p = obj->GetPoint(i);
		worldSpacePoints[i] = p * baseTM;
		bounds += worldSpacePoints[i];
	}

//	TimeValue t = GetCOREInterface()->GetTime();
	Object *nodeObj = node->EvalWorldState(t).obj;
	Box3 nodeBoundingBox;
	Matrix3 nodeTM = node->GetObjectTM(t);
	nodeObj->GetDeformBBox(t, nodeBoundingBox,&nodeTM);
	nodeBoundingBox.EnlargeBy(threshold*1.2f);
//	threshold = Length(bounds.pmax- bounds.pmin)/200.0f;

	for (int i = 0; i < pointCount; i++)
	{
		if (pointsToProcess[i])
			localData->faceIndex[wrapIndex]->f[i] = -1;
//		wrapMeshData[wrapIndex]->faceIndex[i] = -1;
	}

	for (int i = 0; i < pointCount; i++)
		{

		if (!pointsToProcess[i]) continue;


//		wrapMeshData[wrapIndex]->faceIndex[i] = -1;

		ParamSpaceData *pd = localData->paramData.Addr(i);

		Point3 p = worldSpacePoints[i];		

		if (!nodeBoundingBox.Contains(p)) continue;

		rt2->fnClosestFaceThreshold(p,threshold);
		int index =  rt->fnGetClosestHit();
		int faceIndex =  rt->fnGetHitFace(index)-1;
		Point3 bary = rt->fnGetHitBary(index);
//get bary, face index and  perp dist
		float offset = rt->fnGetPerpDist(index);
		float dist = rt->fnGetHitDist(index);



		if ((ip) && (pointCount > 1000))
		{
			TSTR prompt;
			if ((i%100) == 0)
			{
				float per = (float)i/(float)pointCount*100.0f;
				prompt.printf("%s %d : %d(%d) (%0.0f%%)", GetString(IDS_STEP1),wrapIndex, i,pointCount,per);
				ip->ReplacePrompt( prompt);
			}
		}
		
		
		localData->faceIndex[wrapIndex]->f[i] = faceIndex;
//		wrapMeshData[wrapIndex]->faceIndex[i] = faceIndex;

		if ((dist < pd->dist) || (pd->dist == -1.0f))
			{
				if (faceIndex != -1)
				{
					Point3 norm = rt->fnGetHitNorm(index);
					norm *= offset;
					norm = VectorTransform(norm,inverseTM);
					pd->bary = bary;
					pd->offset = offset;
					pd->dist = dist;
					pd->faceIndex = faceIndex;
					pd->whichNode = wrapIndex;
				}

			}

		SHORT iret = GetAsyncKeyState (VK_ESCAPE);
		if (iret==-32767)
		{
			i = pointCount;
			iret = GetAsyncKeyState (VK_ESCAPE);
		}


		}

//	rt->fnPrintStats();


	//delete the engine

	theHold.Suspend();
	ref->DeleteMe();
	theHold.Resume();

//build our weight data

		// Evaluate the node

	if (ip)
		{
		TSTR name;
		name.printf("Pick Mesh");
		SetWindowText(GetDlgItem(hWnd,IDC_STATUS),name);
		}

	if ((ip) && (localData->paramData.Count() > 1000))
	{
		TSTR prompt;
		prompt.printf(" ");
		ip->ReplacePrompt( prompt);
	}


	}

//just builds vertex and face normal list from a mesh
//note vertex norms are not exact since I dont weight them
void MeshDeformPW::BuildNormals(Mesh *mesh, Tab<Point3> &vnorms, Tab<Point3> &fnorms)
	{
		fnorms.SetCount(mesh->numFaces);
		for (int j = 0; j < mesh->numFaces; j++)
		{
			Point3 norm(0.0f,0.0f,0.0f);
			Point3 vecA,vecB;
			Point3 a,b,c;
			a = mesh->verts[mesh->faces[j].v[0]];
			b = mesh->verts[mesh->faces[j].v[1]];
			c = mesh->verts[mesh->faces[j].v[2]];
			vecA = (a - b);
			vecB = (c - b);
			norm = Normalize(CrossProd(vecA,vecB));
			fnorms[j] = norm;
		}

		Tab<int> vcount;
		vnorms.SetCount(mesh->numVerts);
		vcount.SetCount(mesh->numVerts);
		for (int j = 0; j < mesh->numVerts; j++)
		{
			vnorms[j] = Point3(0.0f,0.0f,0.0f);
			vcount[j] = 0;
		}
		for (int j = 0; j < mesh->numFaces; j++)
		{
			int a,b,c;
			a = mesh->faces[j].v[0];
			b = mesh->faces[j].v[1];
			c = mesh->faces[j].v[2];

			vnorms[a] += fnorms[j];
			vcount[a] += 1;
			vnorms[b] += fnorms[j];
			vcount[b] += 1;
			vnorms[c] += fnorms[j];
			vcount[c] += 1;
		}

		for (int j = 0; j < mesh->numVerts; j++)
		{
			if (vcount[j]!= 0)
			{
				vnorms[j] = vnorms[j]/(float)vcount[j];
				vnorms[j] = Normalize(vnorms[j]);
			}
		}

	}


/*************************************************************************************************
*
	ModifyObject will do all the work in a full modifier
    This includes casting objects to their correct form, doing modifications
	changing their parameters, etc
*
\************************************************************************************************/


void MeshDeformPW::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *incnode) 
{
	Interval iv = FOREVER;

	//TODO: Add the code for actually modifying the object
	//check is local data if not create it
	if (mc.localData == NULL)
		{
		LocalMeshData *localData = new LocalMeshData();
		mc.localData = localData;
		}

	LocalMeshData *localData = (LocalMeshData *)mc.localData;

	//if for some strnage reason we have no localdata bail
	if (localData == NULL ) return;

	//grab a pointer to the node that owns this local if we dont already have it
	if (localData->selfNode == NULL)
	{
		localData->selfNode = GetNodeFromModContext(localData);
		if (localData->selfNode == NULL) 
		{
			os->obj->PointsWereChanged();
			iv.SetInstant(t);
			os->obj->UpdateValidity(GEOM_CHAN_NUM,iv);
			return;
		}
		localData->initialBaseTM = localData->selfNode->GetObjectTM(t, &iv);;
	}

	if (useOldLocalBaseTM )
	{
		localData->initialBaseTM = oldLocalBaseTM;
		useOldLocalBaseTM = FALSE;
	}


	//grab all our param block data
	float threshold;
	pblock->GetValue(pb_threshold,t,threshold,iv);
	float falloff;
	pblock->GetValue(pb_falloff,t,falloff,iv);
	float distance;
	pblock->GetValue(pb_distance,t,distance,iv);
	int faceLimit;
	pblock->GetValue(pb_facelimit,t,faceLimit,iv);
	int engine;
	pblock->GetValue(pb_engine,t,engine,iv);

	BOOL weightAllVerts;
	pblock->GetValue(pb_weightallverts,t,weightAllVerts,iv);


	//if we have no wrap meshes bail
	if (pblock->Count(pb_meshlist) == 0 )
	{	
		os->obj->UpdateValidity(GEOM_CHAN_NUM,iv);	
		return;
	}
	else
	{
		//check to see if we have an empty wrap mesh list and if so bail
		int numberOfValidNodes = 0;
		for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
		{
			INode *node = NULL;
			pblock->GetValue(pb_meshlist,t,node,iv,wrapIndex);
			if (node) numberOfValidNodes++;
		}
		if (numberOfValidNodes == 0)
		{
			os->obj->UpdateValidity(GEOM_CHAN_NUM,iv);
			return;
		}
	}


	debugV[0] = 0;
	debugV[1] = 0;
	debugV[2] = 0;

	//if we need to resample set all our local data to resample
	if (resample)
	{
		SetResampleModContext();
		if (ip) UpdateXYZSpinners();
	}
	resample = FALSE;

	//check to see if our wrap mesh count has changed
	if (wrapMeshData.Count() != pblock->Count(pb_meshlist))
	{
		Tab<WrapMeshData*> tempData = wrapMeshData;
		wrapMeshData.SetCount(pblock->Count(pb_meshlist));
		for (int i = 0; i < wrapMeshData.Count(); i++)
		{
			wrapMeshData[i] = NULL;
			if (i < tempData.Count())
				wrapMeshData[i] = tempData[i];
		}
		cageTopoChange = TRUE;
		
	}

	Matrix3 baseTM = localData->selfNode->GetObjectTM(t, &iv);


	localData->currentBaseTM = localData->selfNode->GetNodeTM(t);
	localData->currentInvBaseTM = Inverse(localData->currentBaseTM);

	//loop through our wrap mesh and store off the mesh and norms etc

	for (int wrapIndex = 0; wrapIndex < wrapMeshData.Count(); wrapIndex++)
	{
		if (wrapMeshData[wrapIndex])
		{
			INode *node = NULL;
			if (wrapIndex < pblock->Count(pb_meshlist))
				pblock->GetValue(pb_meshlist,t,node,iv,wrapIndex);
			if (node == NULL)
				wrapMeshData[wrapIndex]->numFaces = 0;
			}
		}

	for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
	{
		INode *node = NULL;
		pblock->GetValue(pb_meshlist,t,node,iv,wrapIndex);



		//see if we have some wrap mesh data if not create it
		if (wrapMeshData[wrapIndex] == NULL)
			wrapMeshData[wrapIndex] = new WrapMeshData();


		//if no node bail
		if (node == NULL) 
		{
			wrapMeshData[wrapIndex]->mesh = NULL;
			wrapMeshData[wrapIndex]->numFaces = 0;
			continue;
		}

		//grab our object from the node
		ObjectState meshos = node->EvalWorldState(t);
		if (meshos.obj) 
		{
			iv &= meshos.obj->ObjectValidity(t);

			Matrix3 meshIdentTM(1);
			if (node->GetWSMDerivedObject())
			{
				BOOL identity = meshos.TMIsIdentity();
				if (!identity)
					 wrapMeshData[wrapIndex]->currentMeshTM = node->GetObjectTM(t, &iv);
				else wrapMeshData[wrapIndex]->currentMeshTM = meshIdentTM;
			}
			else wrapMeshData[wrapIndex]->currentMeshTM = node->GetObjectTM(t, &iv);
			

		//convert or collaspe to get our mesh
			Mesh *mesh = NULL;
			TriObject *collapsedtobj = NULL;
			if (meshos.obj->IsSubClassOf(triObjectClassID))
			{
				TriObject* tobj = (TriObject*)meshos.obj;
				mesh = &tobj->mesh;
			}
		//collapse it to a mesh
			else
			{
				if (meshos.obj->CanConvertToType(triObjectClassID))
				{
					collapsedtobj = (TriObject*) meshos.obj->ConvertToType(t,triObjectClassID);
					mesh = &collapsedtobj->mesh;
				}
			}

			//build vertex and face normals of our wrap mesh	
			if (mesh == NULL)
				wrapMeshData[wrapIndex]->mesh = NULL;
			else
			{
				BuildNormals(mesh, wrapMeshData[wrapIndex]->vnorms, wrapMeshData[wrapIndex]->fnorms);

				wrapMeshData[wrapIndex]->mesh = mesh;
				if ((meshos.obj != collapsedtobj) && (collapsedtobj)) 
				{
					wrapMeshData[wrapIndex]->collapsedtobj = collapsedtobj;
					wrapMeshData[wrapIndex]->deleteMesh = TRUE;
				}
				else
				{
					wrapMeshData[wrapIndex]->collapsedtobj = NULL;
					wrapMeshData[wrapIndex]->deleteMesh = FALSE;
				}
			}
		}
	}

	BOOL rebuilduildParamSpace = FALSE;
	if (localData->faceIndex.Count() <= wrapMeshData.Count())
	{
		while (localData->faceIndex.Count() < wrapMeshData.Count())
		{
			ClosestFaces *f = new ClosestFaces();
			localData->faceIndex.Append(1,&f);
			rebuilduildParamSpace = TRUE;
		}
	}
	if (oldFaceData)
	{
		for (int i = 0; i < localData->faceIndex.Count(); i++)
		{
			localData->faceIndex[i]->f = wrapMeshData[i]->faceIndex;
			wrapMeshData[i]->faceIndex.SetCount(0);
			wrapMeshData[i]->faceIndex.Resize(0);
			oldFaceData = FALSE;
		}

	}

	if (loadOldFaceIndices)
	{
		if ((wrapMeshData.Count() == 1) && (wrapMeshData[0]))
		{
			localData->faceIndex[0]->f.SetCount(localData->paramData.Count());
//			wrapMeshData[0]->faceIndex.SetCount(localData->paramData.Count());
			for (int i = 0; i < localData->paramData.Count(); i++)
			{
				localData->faceIndex[0]->f[i] = localData->paramData[i].faceIndex;
//				wrapMeshData[0]->faceIndex[i] = localData->paramData[i].faceIndex;
				
			}
			wrapMeshData[0]->numFaces = wrapMeshData[0]->mesh->numFaces;
		}
		
		loadOldFaceIndices = FALSE;
	}

	//see if the topology flowing up the stack has changed
	BOOL incomingObjectChanged = FALSE;
	if (os->obj->NumPoints() != localData->initialPoint.Count() )
		incomingObjectChanged = TRUE;

	BOOL cageChanged = FALSE;
	for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
	{
		if (wrapMeshData[wrapIndex]->mesh)
		{
			if ((wrapMeshData[wrapIndex]->numFaces != wrapMeshData[wrapIndex]->mesh->numFaces) || (cageTopoChange) )				
				cageChanged = TRUE;	
			wrapMeshData[wrapIndex]->numFaces = wrapMeshData[wrapIndex]->mesh->numFaces;
		}
	if ((wrapMeshData[wrapIndex]->mesh==NULL) && (localData->faceIndex[wrapIndex]->f.Count() != 0)
			 || (localData->faceIndex[wrapIndex]->f.Count() != localData->paramData.Count()))
		{
			cageChanged = TRUE;
			localData->faceIndex[wrapIndex]->f.SetCount(0);
			localData->faceIndex[wrapIndex]->f.Resize(0);
		}

	}

	//now build our param space data if needed

	if (cageChanged || incomingObjectChanged || (localData->reset))
		rebuilduildParamSpace = TRUE;

	if (rebuilduildParamSpace)
	{

		localData->initialBaseTM = localData->selfNode->GetObjectTM(t, &iv);;

		int pointCount = os->obj->NumPoints();
		localData->paramData.SetCount(pointCount);
		for (int vi = 0; vi < localData->paramData.Count(); vi++)
		{
			localData->paramData[vi].dist = -1.0f;
			localData->paramData[vi].faceIndex = -1;
			localData->paramData[vi].whichNode = -1;
		}

		localData->FreeAdjEdgeList();
		localData->adjEdgeList.SetCount(pblock->Count(pb_meshlist));
		for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
		{
			localData->adjEdgeList[wrapIndex] = NULL;
		}

		for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
		{
			INode *node = NULL;
			pblock->GetValue(pb_meshlist,t,node,iv,wrapIndex);

			if (node == NULL) continue;

			ObjectState meshos = node->EvalWorldState(t);
			Matrix3 meshIdentityTM(1);
			if (node->GetWSMDerivedObject())
			{
				BOOL identity = meshos.TMIsIdentity();
				if (!identity)
					wrapMeshData[wrapIndex]->initialMeshTM = node->GetObjectTM(t, &iv);
				else wrapMeshData[wrapIndex]->initialMeshTM = meshIdentityTM;
			}
			else wrapMeshData[wrapIndex]->initialMeshTM = node->GetObjectTM(t, &iv);

			//if no node bail
			if ((node != NULL) && (wrapMeshData[wrapIndex]->mesh))
			{
				BuildParamData(t,os->obj,localData,wrapIndex,node,wrapMeshData[wrapIndex]->initialMeshTM,localData->initialBaseTM,threshold);
				if (wrapMeshData[wrapIndex]->selectedVerts.GetSize() != wrapMeshData[wrapIndex]->mesh->numVerts)
				{
					wrapMeshData[wrapIndex]->selectedVerts.SetSize(wrapMeshData[wrapIndex]->mesh->numVerts);
					wrapMeshData[wrapIndex]->selectedVerts.ClearAll();

					if (ip)
					{
						localData->displayedVerts.SetSize(os->obj->NumPoints());
						localData->displayedVerts.ClearAll();
					}
					else localData->displayedVerts.SetSize(0);

				}
				//rebuild our adj edge list if we are editing
				if (ip && wrapMeshData[wrapIndex]->mesh)
				{
					localData->adjEdgeList[wrapIndex] = new AdjEdgeList(*wrapMeshData[wrapIndex]->mesh);
				}
			}
		}
	}


	localData->displayedVerts.SetSize(os->obj->NumPoints());
	localData->displayedVerts.ClearAll();

	for (int wrapIndex = 0; wrapIndex < wrapMeshData.Count(); wrapIndex++)
	{
		if (wrapMeshData[wrapIndex]->mesh)
		{
			if (wrapMeshData[wrapIndex]->selectedVerts.GetSize() != wrapMeshData[wrapIndex]->mesh->numVerts)
			{
				wrapMeshData[wrapIndex]->selectedVerts.SetSize(wrapMeshData[wrapIndex]->mesh->numVerts);
				wrapMeshData[wrapIndex]->selectedVerts.ClearAll();
			}
		}
	}


	BOOL rebuildWeights = FALSE;
	if (cageChanged || incomingObjectChanged || (localData->reset) || (localData->resample))
		rebuildWeights = TRUE;
	//check to make sure we have edge lists
	if (ip || rebuildWeights)
	{
		if (localData->adjEdgeList.Count() != wrapMeshData.Count())
		{
			localData->FreeAdjEdgeList();
			localData->adjEdgeList.SetCount(wrapMeshData.Count());
			for (int wrapIndex = 0; wrapIndex < wrapMeshData.Count(); wrapIndex++)
				localData->adjEdgeList[wrapIndex] = NULL;

		}
		for (int wrapIndex = 0; wrapIndex < wrapMeshData.Count(); wrapIndex++)
		{
			if ((localData->adjEdgeList[wrapIndex] == NULL) && wrapMeshData[wrapIndex]->mesh )
				localData->adjEdgeList[wrapIndex] = new AdjEdgeList(*wrapMeshData[wrapIndex]->mesh);
		}
	}
	//if not editing nuke them
	else
	{
		for (int wrapIndex = 0; wrapIndex < localData->adjEdgeList.Count(); wrapIndex++)
		{

			if (localData->adjEdgeList[wrapIndex])
				delete localData->adjEdgeList[wrapIndex];
			localData->adjEdgeList[wrapIndex] = NULL;
		}
	}



	if (rebuildWeights)
	{
		//free our old 
		FreeWeightData(os->obj,localData);
		//create a new weight data
		for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
		{

			
			//save our old data	we might be	able to	reuse some of it					
			if (wrapMeshData[wrapIndex]->mesh == NULL) continue;
			Tab<InitialTMData> oldInitialData;
			if (cageChanged)
			{
				oldInitialData.SetCount(wrapMeshData[wrapIndex]->initialData.Count());
				oldInitialData = wrapMeshData[wrapIndex]->initialData;
			}

			//if we	are	doing a	complete reset nuke	everything and start from scratch
			if (localData->reset)
				wrapMeshData[wrapIndex]->initialData.SetCount(0);
			//rebuild our initial vertex tms on the wrap mesh
			if ( wrapMeshData[wrapIndex]->vnorms.Count() > 0)
				GetTMFromVertex(t, wrapIndex, wrapMeshData[wrapIndex]->mesh, wrapMeshData[wrapIndex]->vnorms.Addr(0),wrapMeshData[wrapIndex]->initialMeshTM);
					
			//see if we can find any matches and if so reuse those
			//we just look for vertices that occupied the same space and copy over the scale and strength
			if (cageChanged)
			{
				for (int i = 0; i < oldInitialData.Count(); i++)
				{
					Point3 oldP = oldInitialData[i].initialVertexTMs.GetTrans();
					for (int j = 0; j < wrapMeshData[wrapIndex]->initialData.Count(); j++)
					{
						Point3 newP = wrapMeshData[wrapIndex]->initialData[j].initialVertexTMs.GetTrans();
						if (newP == oldP)
						{
							wrapMeshData[wrapIndex]->initialData[j].str = oldInitialData[i].str;
							wrapMeshData[wrapIndex]->initialData[j].localScale = oldInitialData[i].localScale;
						}
					}
				}
			}
			//rebuild our weight data
			BitArray pointsToProcess;
			pointsToProcess.SetSize(os->obj->NumPoints());
			pointsToProcess.SetAll();
			BuildWeightData(os->obj,localData,wrapIndex,wrapMeshData[wrapIndex]->mesh,wrapMeshData[wrapIndex]->initialMeshTM,localData->initialBaseTM,falloff,distance,faceLimit,pointsToProcess);

			///store off initial local data 
			localData->numFaces = wrapMeshData[wrapIndex]->mesh->numFaces;
//			localData->initialBaseTM = baseTM;
			localData->rebuildNeighorbordata = TRUE;

		}
		//normalize our weight data
		NormalizeWeightData(localData);
	}



	for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
	{
		if ((wrapMeshData[wrapIndex]->mesh) && (wrapMeshData[wrapIndex]->vnorms.Count() > 0))
			BuildCurrentTMFromVertex(t,wrapIndex, wrapMeshData[wrapIndex]->mesh,wrapMeshData[wrapIndex]->vnorms.Addr(0), wrapMeshData[wrapIndex]->currentMeshTM,baseTM,localData->initialBaseTM);
	}

	if (localData->rebuildNeighorbordata)
	{
		localData->potentialWeightVerts.SetSize(os->obj->NumPoints());
		localData->potentialWeightVerts.ClearAll();

		for (int wrapIndex = 0; wrapIndex < wrapMeshData.Count(); wrapIndex++)
		{
			if (wrapMeshData[wrapIndex]->mesh)
				BuildLookUpData(localData,wrapIndex);
		}
		localData->rebuildNeighorbordata = FALSE;
	}

	if (localData->rebuildSelectedWeights)
		{

		for (int i = 0; i < localData->weightData.Count(); i++)
		{
			if (localData->potentialWeightVerts[i])
				localData->weightData[i]->data.SetCount(0);
		}
		for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
		{
			if (wrapMeshData[wrapIndex]->mesh)
				RebuildSelectedWeights(localData,wrapIndex,wrapMeshData[wrapIndex]->initialMeshTM,baseTM,falloff,distance,faceLimit);	
		}
		NormalizeWeightData(localData);
		localData->rebuildSelectedWeights = FALSE;
		}

	if (weightAllVerts)
	{
		float tempThreshold = threshold;

		BOOL unassignedFaceFound = FALSE;
		BOOL unassignedWeightFound = FALSE;
		BitArray facesToProcess;
		facesToProcess.SetSize(localData->paramData.Count());
		facesToProcess.ClearAll();

		BitArray pointsWithNoWeights;
		pointsWithNoWeights.SetSize(localData->paramData.Count());
		pointsWithNoWeights.ClearAll();

		for (int i = 0; i < localData->paramData.Count(); i++)
		{
			if (localData->paramData[i].faceIndex < 0)
			{
				unassignedFaceFound = TRUE;								
				facesToProcess.Set(i);
//				DebugPrint("Point with no face %d\n",i);
			}
			if (localData->weightData[i]->data.Count() == 0)
			{
				unassignedWeightFound = TRUE;		
				pointsWithNoWeights.Set(i);
//				DebugPrint("Point with no weight %d\n",i);
			}

		}
		if (unassignedFaceFound)
		{
			float tThreshold = tempThreshold;
			while (facesToProcess.NumberSet() > 0)
			{

				tThreshold *= 2;
				int numFaces = 0;
				for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
				{
					INode *node = NULL;
					pblock->GetValue(pb_meshlist,t,node,iv,wrapIndex);

					if (node == NULL) continue;

				//if no node bail
					if ((node != NULL) && (wrapMeshData[wrapIndex]->mesh))
					{
						numFaces += wrapMeshData[wrapIndex]->numFaces;
						BuildParamDataForce(t,os->obj,localData,wrapIndex,node,wrapMeshData[wrapIndex]->initialMeshTM,baseTM,tThreshold,facesToProcess);
					}
				}				
				facesToProcess.ClearAll();

				for (int i = 0; i < localData->paramData.Count(); i++)
				{
					if (localData->paramData[i].faceIndex < 0)
					{
						facesToProcess.Set(i);
					}
				}
				if (numFaces == 0) facesToProcess.ClearAll();

			}

		}
		//fill any vertices that did not have a closest face
		if (unassignedWeightFound)
		{
			for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
			{
				INode *node = NULL;
				pblock->GetValue(pb_meshlist,t,node,iv,wrapIndex);

				if (node == NULL) continue;
				BuildWeightData(os->obj,localData,wrapIndex,wrapMeshData[wrapIndex]->mesh,wrapMeshData[wrapIndex]->initialMeshTM,localData->initialBaseTM,falloff,distance,faceLimit,pointsWithNoWeights);
			}
			NormalizeWeightData(localData);
		}
		//now force any others to have a weight
		WeightAllVerts(localData);
	}

/*





				if (localData->rebuildSelectedWeights)
				{
					RebuildSelectedWeights(localData,wrapIndex,meshTM,baseTM,falloff,distance);
					
					localData->rebuildSelectedWeights = FALSE;
				}


				//nukethe tri mesh if we had to collapse to get it

			}

		}

	}
*/

//setup the deformer
	cageTopoChange = FALSE;
	localData->resample= FALSE;
	localData->reset = FALSE;
	cageChanged = FALSE;
	incomingObjectChanged = FALSE;


	MeshDeformer deformer(this,localData,baseTM,engine);
	deformer.falloff = falloff;
	deformer.SetWrapDataCount(pblock->Count(pb_meshlist));
	for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
	{
		if (ip)
			localData->displayedVerts.ClearAll();
		if ((wrapMeshData[wrapIndex]->mesh) && (wrapMeshData[wrapIndex]->fnorms.Count() > 0))
			deformer.AddWrapData(wrapIndex, wrapMeshData[wrapIndex]->mesh,  wrapMeshData[wrapIndex]->currentMeshTM, wrapMeshData[wrapIndex]->fnorms.Addr(0));
		else deformer.AddWrapData(wrapIndex, wrapMeshData[wrapIndex]->mesh,  wrapMeshData[wrapIndex]->currentMeshTM, NULL);

	}
	os->obj->Deform(&deformer, TRUE);

	for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
	{
		if ((wrapMeshData[wrapIndex]->collapsedtobj != NULL) &&	wrapMeshData[wrapIndex]->deleteMesh)
			wrapMeshData[wrapIndex]->collapsedtobj->DeleteThis();
		wrapMeshData[wrapIndex]->vnorms.Resize(0);
		wrapMeshData[wrapIndex]->fnorms.Resize(0);
	}

	if (ip)
	{
		if (localData->localPoint.Count() != os->obj->NumPoints())
			localData->localPoint.SetCount(os->obj->NumPoints());
		for (int i = 0; i < localData->localPoint.Count(); i++)
			localData->localPoint[i] = os->obj->GetPoint(i);
	}
	else localData->localPoint.ZeroCount();
		
	os->obj->PointsWereChanged();
	os->obj->UpdateValidity(GEOM_CHAN_NUM,iv);
}


void MeshDeformPW::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;
	TimeValue t = ip->GetTime();

	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);

	MeshDeformPWDesc.BeginEditParams(ip, this, flags, prev);
	meshdeformpw_param_blk.SetUserDlgProc(meshdeformpw_params, new MeshDeformParamsMapDlgProc(this));
	meshdeformpw_param_blk.SetUserDlgProc(meshdeformpw_advanceparams,new MeshDeformParamsAdvanceMapDlgProc(this));

	TSTR name;
	name.printf("Pick Mesh");
	SetWindowText(GetDlgItem(hWnd,IDC_STATUS),name);


	selectMode = new SelectModBoxCMode(this,ip);
}

void MeshDeformPW::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	ip->DeleteMode(selectMode);
	if (selectMode) delete selectMode;
	selectMode = NULL;

	TimeValue t = ip->GetTime();
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

	MeshDeformPWDesc.EndEditParams(ip, this, flags, next);
	
	this->ip = NULL;

	if (xSpin != NULL)
		ReleaseISpinner(xSpin);
	if (ySpin != NULL)
		ReleaseISpinner(ySpin);
	if (zSpin != NULL)
		ReleaseISpinner(zSpin);

	if (strSpin != NULL)
		ReleaseISpinner(strSpin);

	if (multSpin != NULL)
		ReleaseISpinner(multSpin);

	if (iPickButton != NULL)
		ReleaseICustButton(iPickButton);
	iPickButton = NULL;

	multSpin = NULL;
	xSpin = NULL;
	ySpin = NULL;
	zSpin = NULL;
	strSpin = NULL;
	



}


void MeshDeformPW::ActivateSubobjSel(int level, XFormModes& modes) {
	// Set the meshes level


	// Fill in modes with our sub-object modes
	if (level!= 0) {
		modes = XFormModes(NULL,NULL,NULL,NULL,NULL,selectMode);
	}
	UpdateUI();
}

Interval MeshDeformPW::GetValidity(TimeValue t)
{
	Interval valid = FOREVER;
	//TODO: Return the validity interval of the modifier
	return valid;
}




RefTargetHandle MeshDeformPW::Clone(RemapDir& remap)
{
	MeshDeformPW* newmod = new MeshDeformPW();	
	newmod->ReplaceReference(PBLOCK_REF,remap.CloneRef(pblock));
	BaseClone(this, newmod, remap);

	newmod->wrapMeshData.SetCount(wrapMeshData.Count());
	for (int i = 0; i < wrapMeshData.Count(); i++)
	{
		newmod->wrapMeshData[i] = new WrapMeshData();		
		newmod->wrapMeshData[i]->selectedVerts = wrapMeshData[i]->selectedVerts;
		newmod->wrapMeshData[i]->initialMeshTM = wrapMeshData[i]->initialMeshTM;
		newmod->wrapMeshData[i]->initialData = wrapMeshData[i]->initialData;
//		newmod->wrapMeshData[i]->faceIndex = wrapMeshData[i]->faceIndex;
	}

	return(newmod);
}


//From ReferenceMaker 
RefResult MeshDeformPW::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	//TODO: Add code to handle the various reference changed messages
	if (message == REFMSG_CHANGE) 
		{
			ParamID changing_param = pblock->LastNotifyParamID();
			meshdeformpw_param_blk.InvalidateUI(changing_param);
//			if ((changing_param == pb_mesh) || (changing_param == pb_meshlist)) 
			if (changing_param == pb_meshlist)
			{
				if (partID == PART_TOPO)
				{
					cageTopoChange = TRUE;
					Resample();
				}
			}
		}

	return REF_SUCCEED ;
	return REF_SUCCEED;
}

/****************************************************************************************
*
 	NotifyInputChanged is called each time the input object is changed in some way
 	We can find out how it was changed by checking partID and message
*
\****************************************************************************************/

void MeshDeformPW::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc)
{

}



//From Object
BOOL MeshDeformPW::HasUVW() 
{ 
	//TODO: Return whether the object has UVW coordinates or not
	return TRUE; 
}

void MeshDeformPW::SetGenUVW(BOOL sw) 
{  
	if (sw==HasUVW()) return;
	//TODO: Set the plugin internal value to sw				
}

class PLCBFixupMeshPB : public  PostLoadCallback
{
public:
	MeshDeformPW      *s;
	
	PLCBFixupMeshPB(MeshDeformPW *r) {s=r;}
	void proc(ILoad *iload);
};

void PLCBFixupMeshPB::proc(ILoad *iload)
{
	INode *node = NULL;
	s->pblock->GetValue(pb_mesh,0,node,FOREVER);
	if (node != NULL)
	{
		s->pblock->SetCount(pb_meshlist,1);
		s->pblock->SetValue(pb_meshlist,0,node,0);
		node = NULL;
		s->pblock->SetValue(pb_mesh,0,node);
	}
	delete this;
}

#define INITIAL_DATA			0x2000
#define INITIALBASETM_DATA		0x2010
#define INITIALMESHTM_DATA		0x2020
#define INITIALV1_DATA			0x2030
#define NUMBEROFWRAPMESHES_DATA	0x2040
#define WRAPMESH_DATA			0x2050
#define WRAPMESHTM_DATA			0x2060
#define WRAPMESH2_DATA			0x2070

IOResult MeshDeformPW::Load(ILoad *iload)
{
	//TODO: Add code to allow plugin to load its data
	IOResult	res;
	ULONG		nb;

	Modifier::Load(iload);

	int currentWrapMesh = 0;
	useOldLocalBaseTM = FALSE;
	while (IO_OK==(res=iload->OpenChunk())) 
		{
		switch(iload->CurChunkID())  
			{

			case INITIALBASETM_DATA:
				{
				useOldLocalBaseTM = TRUE;
				res = oldLocalBaseTM.Load(iload);
				break;
				}

			//old data
			case INITIALMESHTM_DATA:
				{				
				if (wrapMeshData.Count() == 0)
				{
					wrapMeshData.SetCount(1);
					wrapMeshData[0] = new WrapMeshData();
				}

				res = wrapMeshData[0]->initialMeshTM.Load(iload);
				break;
				}
			//old data
			case INITIAL_DATA:
				{
				loadOldFaceIndices = TRUE;
				if (wrapMeshData.Count() == 0)
				{
					wrapMeshData.SetCount(1);
					wrapMeshData[0] = new WrapMeshData();
				}

				Tab<InitialTMDataOld> initialDataOld;
				int ct;
				iload->Read(&ct,sizeof(int), &nb);
				initialDataOld.SetCount(ct);
				if(ct>0)
				{
					res = iload->Read(initialDataOld.Addr(0),sizeof(InitialTMDataOld)*ct, &nb);
					wrapMeshData[0]->initialData.SetCount(initialDataOld.Count());
					for (int i =0; i < ct; i++)
					{
						wrapMeshData[0]->initialData[i].dist = initialDataOld[i].dist;
						wrapMeshData[0]->initialData[i].initialVertexTMs = initialDataOld[i].initialVertexTMs;
						wrapMeshData[0]->initialData[i].xPoints = initialDataOld[i].xPoints;
						wrapMeshData[0]->initialData[i].localScale = Point3(1.0f,1.0f,1.0f);						
					}
				}

				break;
				}
			//old data
			case INITIALV1_DATA:
				{
				loadOldFaceIndices = TRUE;
				if (wrapMeshData.Count() == 0)
				{
					wrapMeshData.SetCount(1);
					wrapMeshData[0] = new WrapMeshData();
				}

				int ct;
				Tab<InitialTMData> initialData;
				iload->Read(&ct,sizeof(int), &nb);
				wrapMeshData[0]->initialData.SetCount(ct);
				if(ct>0)
					res = iload->Read(wrapMeshData[0]->initialData.Addr(0),sizeof(InitialTMData)*ct, &nb);

				break;
				}

			case NUMBEROFWRAPMESHES_DATA:
				{
					int ct;
					iload->Read(&ct,sizeof(int), &nb);
					wrapMeshData.SetCount(ct);
					break;
				}
			case WRAPMESHTM_DATA:
				{
					wrapMeshData[currentWrapMesh] = new WrapMeshData();
					res = wrapMeshData[currentWrapMesh]->initialMeshTM.Load(iload);
					break;

				}
			case WRAPMESH_DATA:
				{
					iload->Read(&wrapMeshData[currentWrapMesh]->numFaces,sizeof(int), &nb);
					
					int numberVerts;
					iload->Read(&numberVerts,sizeof(int), &nb);
					wrapMeshData[currentWrapMesh]->initialData.SetCount(numberVerts);
					if (numberVerts > 0)
					{
						iload->Read(wrapMeshData[currentWrapMesh]->initialData.Addr(0),sizeof(InitialTMData)*numberVerts, &nb);
					}

					iload->Read(&numberVerts,sizeof(int), &nb);
					wrapMeshData[currentWrapMesh]->faceIndex.SetCount(numberVerts);
					if (numberVerts > 0)
					{
						iload->Read(wrapMeshData[currentWrapMesh]->faceIndex.Addr(0),sizeof(int)*numberVerts, &nb);
					}
						
					currentWrapMesh++;
					oldFaceData = TRUE;
					break;
				}
			case WRAPMESH2_DATA:
				{
					iload->Read(&wrapMeshData[currentWrapMesh]->numFaces,sizeof(int), &nb);
					
					int numberVerts;
					iload->Read(&numberVerts,sizeof(int), &nb);
					wrapMeshData[currentWrapMesh]->initialData.SetCount(numberVerts);
					if (numberVerts > 0)
					{
						iload->Read(wrapMeshData[currentWrapMesh]->initialData.Addr(0),sizeof(InitialTMData)*numberVerts, &nb);
					}

					
						
					currentWrapMesh++;
					break;
				}

			default:
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
		}

	PLCBFixupMeshPB* plcb = new PLCBFixupMeshPB(this);
	iload->RegisterPostLoadCallback(plcb);

	return IO_OK;
}





IOResult MeshDeformPW::Save(ISave *isave)
{
	//TODO: Add code to allow plugin to save its data
	IOResult	res;
	ULONG		nb;
	
	Modifier::Save(isave);

	isave->BeginChunk(NUMBEROFWRAPMESHES_DATA);
	int ct = wrapMeshData.Count();
	res = isave->Write(&ct, sizeof(ct), &nb);
	isave->EndChunk();

	//now save the wrap data
	for (int wrapIndex = 0; wrapIndex < ct; wrapIndex++)
	{
		isave->BeginChunk(WRAPMESHTM_DATA);
		res = wrapMeshData[wrapIndex]->initialMeshTM.Save(isave);
		isave->EndChunk();

		isave->BeginChunk(WRAPMESH2_DATA);		
		res = isave->Write(&wrapMeshData[wrapIndex]->numFaces, sizeof(wrapMeshData[wrapIndex]->numFaces), &nb);
		

		int ict = wrapMeshData[wrapIndex]->initialData.Count();
		res = isave->Write(&ict, sizeof(ict), &nb);
		if(ict>0)
			res = isave->Write(wrapMeshData[wrapIndex]->initialData.Addr(0), sizeof(InitialTMData)*ict, &nb);

/*
		ict = wrapMeshData[wrapIndex]->faceIndex.Count();
		res = isave->Write(&ict, sizeof(ict), &nb);
		if(ict>0)
			res = isave->Write(wrapMeshData[wrapIndex]->faceIndex.Addr(0), sizeof(int)*ict, &nb);
*/

		isave->EndChunk();
	}

/*
	res = initialBaseTM.Save(isave);
	isave->EndChunk();

	isave->BeginChunk(INITIALMESHTM_DATA);
	res = initialMeshTM.Save(isave);
	isave->EndChunk();

	isave->BeginChunk(INITIALV1_DATA);
	int ct = initialData.Count();
	res = isave->Write(&ct, sizeof(ct), &nb);
	if(ct>0)
		res = isave->Write(initialData.Addr(0), sizeof(InitialTMData)*ct, &nb);
	isave->EndChunk();
*/
	//save number of wrap mesh
	//save the data

	
	return IO_OK;
}

#define UVSPACE_DATA	0x1000
#define MESHCOUNT_DATA	0x1010
#define INCDELTA_DATA	0x1020

#define WEIGHT_DATA	0x1030
#define INITIALPOINT_DATA	0x1040

#define UVSPACE2_DATA	0x1050
#define WEIGHT2_DATA	0x1060
#define LINITIALBASETM_DATA	0x1070
#define FACEINDICES_DATA	0x1080

IOResult MeshDeformPW::SaveLocalData(ISave *isave, LocalModData *pld)
{
	LocalMeshData *p;
	IOResult	res;
	ULONG		nb;

	p = (LocalMeshData*)pld;

	int ct = p->paramData.Count();
	if (ct != 0)
	{
		ParamSpaceData *data = p->paramData.Addr(0);

		isave->BeginChunk(UVSPACE2_DATA);
	
		res = isave->Write(&ct, sizeof(ct), &nb);
		if(ct)
			res = isave->Write(data, sizeof(ParamSpaceData)*p->paramData.Count(), &nb);
		isave->EndChunk();
	}

	isave->BeginChunk(MESHCOUNT_DATA);
	ct = p->numFaces;
	res = isave->Write(&ct, sizeof(ct), &nb);
	isave->EndChunk();

/*
	isave->BeginChunk(FACEINDICES_DATA);
	ct = p->weightData.Count();
	res = isave->Write(&ct, sizeof(ct), &nb);
	for (int i = 0 ; i < ct; i++)
	{
		int ct2 = p->weightData[i]->data.Count();
		res = isave->Write(&ct2, sizeof(ct2), &nb);
		if (ct2)
		{
			res = isave->Write(p->weightData[i]->data.Addr(0), sizeof(BoneInfo)*ct2, &nb);
		}

	}
	isave->EndChunk();
*/

	isave->BeginChunk(INITIALPOINT_DATA);
	ct = p->initialPoint.Count();
	res = isave->Write(&ct, sizeof(ct), &nb);
	if (ct!=0)
		isave->Write(p->initialPoint.Addr(0), sizeof(Point3)*ct, &nb);		
	isave->EndChunk();

	isave->BeginChunk(LINITIALBASETM_DATA);
	res = p->initialBaseTM.Save(isave);
	isave->EndChunk();

	isave->BeginChunk(FACEINDICES_DATA);
	ct = p->faceIndex.Count();
	res = isave->Write(&ct, sizeof(ct), &nb);
	for (int i = 0 ; i < ct; i++)
	{
		int ct2 = p->faceIndex[i]->f.Count();
		res = isave->Write(&ct2, sizeof(ct2), &nb);
		if (ct2)
		{
			res = isave->Write(p->faceIndex[i]->f.Addr(0), sizeof(int)*ct2, &nb);
		}

	}
	isave->EndChunk();

	
	



return IO_OK;
}

IOResult MeshDeformPW::LoadLocalData(ILoad *iload, LocalModData **pld)

{
	IOResult	res;
	ULONG		nb;

	LocalMeshData *p= new LocalMeshData();
	p->resample = TRUE;
	while (IO_OK==(res=iload->OpenChunk())) 
		{
		switch(iload->CurChunkID())  
			{
			case INITIALPOINT_DATA:
				{
				int ct;
				iload->Read(&ct,sizeof(int), &nb);
				
				if (ct)
				{
					p->initialPoint.SetCount(ct);
					iload->Read(p->initialPoint.Addr(0),sizeof(Point3)*ct, &nb);
				}

				break;
				}

			case WEIGHT_DATA:
				{
				int ct;
				iload->Read(&ct,sizeof(int), &nb);
				p->weightData.SetCount(ct);
				for (int i = 0; i < ct; i++)
				{
					int ct2;
					p->weightData[i] = new WeightData();
					iload->Read(&ct2,sizeof(int), &nb);

					Tab<BoneInfoOld> data;
					data.SetCount(ct2);
					p->weightData[i]->data.SetCount(ct2);
					if (ct2)
					{
						iload->Read(data.Addr(0),sizeof(BoneInfoOld)*ct2, &nb);
						for (int j = 0; j < ct2; j++)
						{
							p->weightData[i]->data[j].normalizedWeight = data[j].normalizedWeight;
							p->weightData[i]->data[j].dist = data[j].dist;
							p->weightData[i]->data[j].owningBone = data[j].owningBone;
							p->weightData[i]->data[j].whichNode = 0;
						}
					}
				}
				break;
				}
			case UVSPACE_DATA:
				{
				int ct;
				iload->Read(&ct,sizeof(int), &nb);
				Tab<ParamSpaceDataOld> paramData;
				paramData.SetCount(ct);
				p->paramData.SetCount(ct);
				if(ct)
				{
					ParamSpaceDataOld *data = paramData.Addr(0);
					iload->Read(data,sizeof(ParamSpaceData)*p->paramData.Count(), &nb);
				}
				for (int i = 0; i < ct; i++)
				{
					p->paramData[i].bary = paramData[i].bary;
					p->paramData[i].offset = paramData[i].offset;
					p->paramData[i].faceIndex = paramData[i].faceIndex;
					p->paramData[i].whichNode = 0;
				}
				
				break;
				}

			case MESHCOUNT_DATA:
				{
				int ct;
				iload->Read(&ct,sizeof(int), &nb);
				p->numFaces = ct;
				break;
				}
			case LINITIALBASETM_DATA:
				{					
					res =p->initialBaseTM.Load(iload);
					break;
				}
			case WEIGHT2_DATA:
				{
				int ct;
				iload->Read(&ct,sizeof(int), &nb);
				p->weightData.SetCount(ct);
				for (int i = 0; i < ct; i++)
				{
					int ct2;
					p->weightData[i] = new WeightData();
					iload->Read(&ct2,sizeof(int), &nb);
					p->weightData[i]->data.SetCount(ct2);
					if (ct2)
						iload->Read(p->weightData[i]->data.Addr(0),sizeof(BoneInfo)*ct2, &nb);
				}
				break;
				}
			case UVSPACE2_DATA:
				{
				int ct;
				iload->Read(&ct,sizeof(int), &nb);
				p->paramData.SetCount(ct);
				if(ct)
				{
					ParamSpaceData *data = p->paramData.Addr(0);
					iload->Read(data,sizeof(ParamSpaceData)*p->paramData.Count(), &nb);
				}
				break;
				}
			case FACEINDICES_DATA:
				{
				int ct;
				iload->Read(&ct,sizeof(int), &nb);
				p->faceIndex.SetCount(ct);
				for (int i = 0; i < ct; i++)
				{
					int ct2;
					p->faceIndex[i] = new ClosestFaces();
					iload->Read(&ct2,sizeof(int), &nb);
					p->faceIndex[i]->f.SetCount(ct2);
					if (ct2)
						iload->Read(p->faceIndex[i]->f.Addr(0),sizeof(int)*ct2, &nb);
				}
				break;
				}
			}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
		}

	*pld = p;


return IO_OK;

}



BOOL RecursePipeAndMatch2(MeshDeformPW *pmod, Object *obj,int whichField)
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
					if (whichField == FIELD_RESAMPLE)
						((LocalMeshData*)(mc->localData))->resample = TRUE;
					else if (whichField == FIELD_REBUILDNEIGHBORS)
						((LocalMeshData*)(mc->localData))->rebuildNeighorbordata = TRUE;
					else if (whichField == FIELD_REBUILDSELECTEDWEIGHTS)
						((LocalMeshData*)(mc->localData))->rebuildSelectedWeights = TRUE;
					else if (whichField == FIELD_RESET)
						((LocalMeshData*)(mc->localData))->reset = TRUE;


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
			if (RecursePipeAndMatch2(pmod, bobj,whichField)) return TRUE;
			}

		}

	return FALSE;
}




void MeshDeformPW::SetResampleModContext()
{
    MeshEnumProc dep;              
	DoEnumDependents(&dep);
	for ( int i = 0; i < dep.Nodes.Count(); i++)
		{
		INode *node = dep.Nodes[i];
		BOOL found = FALSE;

		if (node)
			{
			Object* obj = node->GetObjectRef();
	
			 RecursePipeAndMatch2(this,obj,FIELD_RESAMPLE) ;
			}
		}
}





void MeshDeformPW::SetRebuildNeighborDataModContext()
{
    MeshEnumProc dep;              
	DoEnumDependents(&dep);
	for ( int i = 0; i < dep.Nodes.Count(); i++)
		{
		INode *node = dep.Nodes[i];
		BOOL found = FALSE;

		if (node)
			{
			Object* obj = node->GetObjectRef();
	
			 RecursePipeAndMatch2(this,obj,FIELD_REBUILDNEIGHBORS) ;
			}
		}
}

void MeshDeformPW::SetRebuildSelectedWeightDataModContext()
{
    MeshEnumProc dep;              
	DoEnumDependents(&dep);
	for ( int i = 0; i < dep.Nodes.Count(); i++)
		{
		INode *node = dep.Nodes[i];
		BOOL found = FALSE;

		if (node)
			{
			Object* obj = node->GetObjectRef();
	
			 RecursePipeAndMatch2(this,obj,FIELD_REBUILDSELECTEDWEIGHTS) ;
			}
		}
}



Matrix3 MeshDeformPW::CompMatrix(TimeValue t,INode *inode,ModContext *mc)
	{
	Interval iv;
	Matrix3 tm(1);	
	if (mc && mc->tm) tm = Inverse(*(mc->tm));
	if (inode) 
	{
#ifdef DESIGN_VER
		tm = tm * inode->GetObjTMBeforeWSM(GetCOREInterface()->GetTime(),&iv);
#else
		tm = tm * inode->GetObjTMBeforeWSM(t,&iv);
#endif // DESIGN_VER
	}
	return tm;
	}

void MeshDeformPW::DrawLoops(GraphicsWindow *gw, float size, Matrix3 tm)
{
				//draw our loops
	int segments = 64;
	float delta = 2.0f*(float)PI/(float)(segments);
	float angle = 0.0f;


	angle = 0.0f;
	Point3 p[2];
	Point3 prevP[3];
	for (int i = 0; i < (segments+1); i++)
	{
		float b = (float) (cos(angle));
		float c = (float) (sin(angle));
		p[0] = Point3(b, 0.0f,c) * tm;
		p[1] = prevP[0];
		prevP[0] = p[0];
		if (i != 0)
			gw->segment(p,1);				

		p[0] = Point3(0.0f,b,c) * tm;
		p[1] = prevP[1];
		prevP[1] = p[0];
		if (i != 0)
			gw->segment(p,1);				

		p[0] = Point3(b,c,0.0f) * tm;
		p[1] = prevP[2];
		prevP[2] = p[0];
		if (i != 0)
			gw->segment(p,1);

		angle += delta;			


	}
}


void MeshDeformPW::DrawBox(GraphicsWindow *gw, Box3 box)
	{
	Point3 pt[3];

	pt[0] = box[2];
	pt[1] = box[3];
	gw->polyline(2, pt, NULL, NULL, 0, NULL);

	pt[0] = box[3];
	pt[1] = box[1];
	gw->polyline(2, pt, NULL, NULL, 0, NULL);

	pt[0] = box[1];
	pt[1] = box[0];
	gw->polyline(2, pt, NULL, NULL, 0, NULL);
			
	pt[0] = box[0];
	pt[1] = box[2];
	gw->polyline(2, pt, NULL, NULL, 0, NULL);


				
	pt[0] = box[2+4];
	pt[1] = box[3+4];
	gw->polyline(2, pt, NULL, NULL, 0, NULL);

	pt[0] = box[3+4];
	pt[1] = box[1+4];
	gw->polyline(2, pt, NULL, NULL, 0, NULL);

	pt[0] = box[1+4];
	pt[1] = box[0+4];
	gw->polyline(2, pt, NULL, NULL, 0, NULL);

	pt[0] = box[0+4];
	pt[1] = box[2+4];
	gw->polyline(2, pt, NULL, NULL, 0, NULL);



				
	pt[0] = box[2];
	pt[1] = box[2+4];
	gw->polyline(2, pt, NULL, NULL, 0, NULL);

	pt[0] = box[3];
	pt[1] = box[3+4];
	gw->polyline(2, pt, NULL, NULL, 0, NULL);

	pt[0] = box[1];
	pt[1] = box[1+4];
	gw->polyline(2, pt, NULL, NULL, 0, NULL);

	pt[0] = box[2];
	pt[1] = box[2+4];
	gw->polyline(2, pt, NULL, NULL, 0, NULL);

	}


void MeshDeformPW::DisplayMirrorData(GraphicsWindow *gw, TimeValue t)
{

	BOOL showMirrorData;
	int axis;
	float offset;


	pblock->GetValue(pb_mirrorshowdata, t, showMirrorData, FOREVER);

	if (showMirrorData)
	{
		pblock->GetValue(pb_mirrorplane, t, axis, FOREVER);
		pblock->GetValue(pb_mirroroffset, t, offset, FOREVER);

		INode *node = NULL;

		int savedLimits;

		gw->setRndLimits((savedLimits = gw->getRndLimits()) & ~GW_ILLUM);
		BOOL first = TRUE;

		//build out mirror tm
		Matrix3 mirrorTM;
		for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
		{
			pblock->GetValue(pb_meshlist,t,node,FOREVER,wrapIndex);

			if ((node != NULL) && (ip && ip->GetSubObjectLevel() == 1) && wrapMeshData[wrapIndex]->mesh)
			{
				if (first)
				{
					mirrorTM = node->GetObjectTM(t);
					first = FALSE;
				}
			}
		}

		Point3 vec = mirrorTM.GetRow(axis);
		vec = Normalize(vec) * offset;
		Point3 trans = mirrorTM.GetRow(3);
		trans += vec;
		mirrorTM.SetRow(3,trans);

		Matrix3 imirrorTM = Inverse(mirrorTM);

		//draw the plane
		Box3 bounds;
		bounds.Init();
		for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
		{
			Interval iv;
			pblock->GetValue(pb_meshlist,t,node,iv,wrapIndex);

			if ((node != NULL) && (ip && ip->GetSubObjectLevel() == 1) && wrapMeshData[wrapIndex]->mesh)
			{
				Matrix3 tm = node->GetObjectTM(t);
				for (int i = 0; i < wrapMeshData[wrapIndex]->initialData.Count(); i++)
				{
					Point3 p = wrapMeshData[wrapIndex]->initialData[i].initialVertexTMs.GetTrans();
					p = p * tm;
					p = p * imirrorTM;
					bounds +=  p;
				}
			}
		}
		Point3 c = GetUIColor (COLOR_GIZMOS);
		c = c + (c*0.25f);
		if ((ip && ip->GetSubObjectLevel() == 1))
		{
			float d = Length(bounds.pmax - bounds.pmin);
			d *= 0.2f;
			bounds.EnlargeBy(d);
			bounds.pmax[axis] = 0.0f;
			bounds.pmin[axis] = 0.0f;
			gw->setTransform(mirrorTM);

			gw->setColor (LINE_COLOR, c);
			gw->startSegments();
			DrawBox(gw, bounds);
			gw->endSegments();
		}

		//draw the mirrored points

		for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
		{
			Interval iv;
			pblock->GetValue(pb_meshlist,t,node,iv,wrapIndex);

			if ((node != NULL) && (ip && ip->GetSubObjectLevel() == 1) && wrapMeshData[wrapIndex]->mesh)
			{
				Matrix3 tm = node->GetObjectTM(t);			

				gw->setColor (LINE_COLOR, c);


		//draw mirror verts
				gw->startMarkers();
				for (int i = 0; i < wrapMeshData[wrapIndex]->initialData.Count(); i++)
				{
					if (wrapMeshData[wrapIndex]->selectedVerts[i])
					{
						Point3 p = wrapMeshData[wrapIndex]->currentVertexTMs[i].GetTrans();
						p = p * tm;
						p = p * imirrorTM;
						p[axis] *= -1.0f;
						gw->marker(&p,CIRCLE_MRKR);
					}
				}
				gw->endMarkers();
			}
		}

	}


}

int MeshDeformPW::Display(
		TimeValue t, INode* inode, ViewExp *vpt, 
		int flagst, ModContext *mc)
	{

	int engine;
	pblock->GetValue(pb_engine, t, engine, FOREVER);
//	if (engine == 0)
//		return 0;

	BOOL showFaceLimit,showLoop,showAxis,showUnassignedVerts,showVerts;

	pblock->GetValue(pb_showloops, t, showLoop, FOREVER);
	pblock->GetValue(pb_showaxis, t, showAxis, FOREVER);
	pblock->GetValue(pb_showfacelimit, t, showFaceLimit, FOREVER);
	pblock->GetValue(pb_showunassignedverts, t, showUnassignedVerts, FOREVER);
	pblock->GetValue(pb_showverts, t, showVerts, FOREVER);

	GraphicsWindow *gw = vpt->getGW();

	int savedLimits;
	gw->setRndLimits((savedLimits = gw->getRndLimits()) & ~GW_ILLUM);

	Point3 pt[4];
	int numberOfWrapMeshes = pblock->Count(pb_meshlist);

	LocalMeshData *localData = (LocalMeshData *)mc->localData;

	BitArray weightedVerts;
	weightedVerts.SetSize(localData->localPoint.Count());
	weightedVerts.ClearAll();

	for (int wrapIndex = 0; wrapIndex < numberOfWrapMeshes; wrapIndex++)
	{
		INode *node = NULL;
		Interval iv;
		pblock->GetValue(pb_meshlist,t,node,iv,wrapIndex);
		if (node == NULL) continue;

		int debugPoint;
		pblock->GetValue(pb_debugpoint,t,debugPoint,iv);

		if ((node != NULL) && (ip && ip->GetSubObjectLevel() == 1) )
		{
			Matrix3 tm = node->GetObjTMAfterWSM(t);//node->GetObjectTM(t);

			gw->setTransform(tm);

			Point3 colSel=GetSubSelColor();
			Point3 colTicks=GetUIColor (COLOR_VERT_TICKS);
			
			// Draw start point
			if (ip ) 
			{
				gw->startMarkers();
				int ct = wrapMeshData[wrapIndex]->currentVertexTMs.Count();

				for (int i = 0; i < ct; i++)
				{
					if (wrapMeshData[wrapIndex]->selectedVerts[i])
					{
						gw->setColor (LINE_COLOR, GetUIColor (COLOR_SEL_GIZMOS));

                  Point3 pt(wrapMeshData[wrapIndex]->currentVertexTMs[i].GetTrans());
						gw->marker(&pt,SM_DOT_MRKR);
					}
					else if (showVerts)
					{
						gw->setColor (LINE_COLOR, GetUIColor (COLOR_GIZMOS));

                  Point3 pt(wrapMeshData[wrapIndex]->currentVertexTMs[i].GetTrans());
						gw->marker(&pt,SM_DOT_MRKR);
					}
					
				}


				gw->endMarkers();

			}

			//draw our loops
			if (ip ) 
			{
				if (showLoop || showAxis)
				{
					float distance;
					pblock->GetValue(pb_distance,t,distance,iv);
					int ct = wrapMeshData[wrapIndex]->currentVertexTMs.Count();
					
					

					
					if (localData->selfNode)
					{

						gw->setColor (LINE_COLOR, GetUIColor (COLOR_SEL_GIZMOS));
						gw->setTransform(tm);
						gw->startSegments();
						for (int i = 0; i < ct; i++)
						{
							if (wrapMeshData[wrapIndex]->selectedVerts[i])
							{
								Point3 l[3];
								l[0] = wrapMeshData[wrapIndex]->currentVertexTMs[i].GetTrans();
								int xedge = wrapMeshData[wrapIndex]->initialData[i].xPoints;
								l[1] = wrapMeshData[wrapIndex]->currentVertexTMs[xedge].GetTrans();
								gw->segment(l,1);	
							}
						}
						gw->endSegments();
					}
					
					Matrix3 identtm(1);
					gw->setTransform( identtm);
					gw->startSegments();
					for (int i = 0; i < ct; i++)
					{
						if (wrapMeshData[wrapIndex]->selectedVerts[i])
						{
							//draw loop
							Matrix3 pTM = wrapMeshData[wrapIndex]->currentVertexTMs[i];
							pTM.Scale(Point3(wrapMeshData[wrapIndex]->initialData[i].dist*distance*wrapMeshData[wrapIndex]->initialData[i].localScale.x,
											wrapMeshData[wrapIndex]->initialData[i].dist*distance*wrapMeshData[wrapIndex]->initialData[i].localScale.y,
											wrapMeshData[wrapIndex]->initialData[i].dist*distance*wrapMeshData[wrapIndex]->initialData[i].localScale.z));
 					
							//gw->setTransform( pTM * tm);
							Matrix3 loopTM = pTM * tm;
							
							if (showLoop && (engine == 1))
							{
								gw->setColor (LINE_COLOR, colSel);
								DrawLoops(gw, 1.0f,loopTM);
							}
							if (showAxis)
							{
 								Point3 axis[3];
//								axis[0] = Point3(0.0f,0.0f,0.0f);
 								axis[0] = loopTM.GetRow(3);
								
								gw->setColor (LINE_COLOR, Point3(1.0f,0.0f,0.0f));
								axis[1] = loopTM.GetRow(3)+(loopTM.GetRow(0)*0.2f);
								gw->segment(axis,1);

								gw->setColor (LINE_COLOR, Point3(0.0f,1.0f,0.0f));
								axis[1] = loopTM.GetRow(3)+(loopTM.GetRow(1)*0.2f);
								gw->segment(axis,1);

								gw->setColor (LINE_COLOR, Point3(0.0f,0.0f,1.0f));
 								axis[1] = loopTM.GetRow(3)+(loopTM.GetRow(2)*-0.2f);
								gw->segment(axis,1);						
							}
							
						}
					}
					gw->endSegments();
				}

			}
		}


		if ((mc->localData) && (ip && ip->GetSubObjectLevel() == 1) )
		{
			LocalMeshData *localData = (LocalMeshData *)mc->localData;

			Matrix3 tm;
			if (localData->selfNode)
				tm = localData->selfNode->GetObjectTM(t);
			gw->setTransform(tm);


			Point3 selSoft = GetUIColor(COLOR_SUBSELECTION_SOFT);
			Point3 selMedium = GetUIColor(COLOR_SUBSELECTION_MEDIUM);
			Point3 selHard = GetUIColor(COLOR_SUBSELECTION_HARD);
			Point3 subSelColor = GetUIColor(COLOR_SUBSELECTION);

			gw->startMarkers();
			int i = 0;

			if (showUnassignedVerts)
			{
				gw->setColor (LINE_COLOR, Point3(1.0f,0.0f,0.0f));
				for (int i = 0; i < localData->localPoint.Count(); i++)
				{
					int numberOfBones = localData->weightData[i]->data.Count();

					if (numberOfBones == 0)
					{

						gw->marker(&localData->localPoint[i],HOLLOW_BOX_MRKR);
					}
					
				}
			}
			gw->endMarkers();


			gw->startMarkers();
			i = 0;

			if (showUnassignedVerts)
			{
				gw->setColor (LINE_COLOR, Point3(1.0f,0.0f,0.0f));
				for (int i = 0; i < localData->localPoint.Count(); i++)
				{
					int numberOfBones = localData->weightData[i]->data.Count();
					if (localData->paramData[i].faceIndex < 0)
					{
						gw->marker(&localData->localPoint[i],CIRCLE_MRKR);
					}

					
				}
			}
			gw->endMarkers();



			gw->startMarkers();
			if (engine == 0)
			{
				gw->setColor(LINE_COLOR, subSelColor);
				
				for (int i = 0; i < localData->localPoint.Count(); i++)
				{
					if (localData->displayedVerts[i])
						gw->marker(&localData->localPoint[i],SM_DOT_MRKR);
						
				}

			}
			else
			{

				for (int i = 0; i < localData->localPoint.Count(); i++)
				{


					//loop through our weights
					int numberOfBones = localData->weightData[i]->data.Count();
					float w = 0.0f;
					for (int j = 0; j < numberOfBones; j++)
					{
						int id = localData->weightData[i]->data[j].owningBone;					
						if (wrapMeshData[wrapIndex]->selectedVerts[id] &&
							(localData->weightData[i]->data[j].whichNode == wrapIndex))
						{
							float tw = localData->weightData[i]->data[j].normalizedWeight;
							if (tw > w)
								w = tw;
						}
					}




					if (w != 0.0f)
					{
						Point3 selColor(0.0f,0.0f,0.0f);

						float infl = w;
						if (infl > 0.0f)
						{
							if ( (infl<0.33f) && (infl > 0.0f))
							{
								selColor = selSoft + ( (selMedium-selSoft) * (infl/0.33f));
							}
							else if (infl <.66f)
							{
								selColor = selMedium + ( (selHard-selMedium) * ((infl-0.1f)/0.66f));
							}
							else if (infl < 0.99f)
							{
								selColor = selHard + ( (subSelColor-selHard) * ((infl-0.66f)/0.33f));
							}

							else 
							{
								selColor = subSelColor;
							}



							gw->setColor(LINE_COLOR, selColor);

							gw->marker(&localData->localPoint[i],SM_DOT_MRKR);
							weightedVerts.Set(i,true);
						}


					//see if any of the bone wieghts is selected if so tag it

					//see if this point is selected
					
					}
/*
					else if (localData->potentialWeightVerts[i] && showFaceLimit)
					{
						gw->setColor (LINE_COLOR, Point3(1.0f,0.0f,0.0f));
						gw->marker(&localData->localPoint[i],SM_DOT_MRKR);
					}
*/


						
				}
			}
			gw->endMarkers();
			
		}
	}

	if (showFaceLimit)
	{
		gw->startMarkers();
		for (int i = 0; i < localData->localPoint.Count(); i++)
		{
			if (!weightedVerts[i])
			{
				if (localData->potentialWeightVerts[i])
				{
					gw->setColor (LINE_COLOR, Point3(1.0f,0.0f,0.0f));
					gw->marker(&localData->localPoint[i],SM_DOT_MRKR);

				}
			}
		}
		gw->endMarkers();
	}

	DisplayMirrorData(gw, t);

	gw->setRndLimits(savedLimits);



	return 0;
	}


void MeshDeformPW::GetWorldBoundBox(
		TimeValue t,INode* inode, ViewExp *vpt, 
		Box3& box, ModContext *mc)
	{

	Matrix3 tm = CompMatrix(t,inode,mc);
	box.Init();
	for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
	{
		if (wrapIndex < wrapMeshData.Count()) 
		{
			INode *wnode = NULL;
			pblock->GetValue(pb_meshlist,t,wnode,FOREVER,wrapIndex);
			if (wnode)
			{
				Matrix3 wtm = wnode->GetObjTMAfterWSM(t);
				int ct = wrapMeshData[wrapIndex]->currentVertexTMs.Count();
				for (int i = 0; i < ct; i++)
				{
					box += wrapMeshData[wrapIndex]->currentVertexTMs[i].GetTrans() * wtm;
				}
			}

			if (ip ) 
			{
				float distance;
 				Interval iv;
				pblock->GetValue(pb_distance,t,distance,iv);
				int ct = wrapMeshData[wrapIndex]->currentVertexTMs.Count();

				INode *mnode = NULL;

				pblock->GetValue(pb_meshlist,t,mnode,iv,wrapIndex);

				if (mnode != NULL)
				{

					Matrix3 tm = mnode->GetObjectTM(t);

					for (int i = 0; i < ct; i++)
					{
						if (wrapMeshData[wrapIndex]->selectedVerts[i])
						{
							Matrix3 pTM = wrapMeshData[wrapIndex]->currentVertexTMs[i];
							Point3 p1(Point3(wrapMeshData[wrapIndex]->initialData[i].dist*distance*wrapMeshData[wrapIndex]->initialData[i].localScale.x,
											wrapMeshData[wrapIndex]->initialData[i].dist*distance*wrapMeshData[wrapIndex]->initialData[i].localScale.y,
											wrapMeshData[wrapIndex]->initialData[i].dist*distance*wrapMeshData[wrapIndex]->initialData[i].localScale.z));
		//					Point3 p1(1.5f,1.5f,1.5f);
							Point3 p2(p1*-1.0f);
							p1 = wrapMeshData[wrapIndex]->currentVertexTMs[i].GetTrans() + p1;
							p2 = wrapMeshData[wrapIndex]->currentVertexTMs[i].GetTrans() + p2;
 							box += p1*tm; 
 							box += p2*tm;
						}
					}
				}
			}

		}
	}

	box.Scale(1.2f);
	worldBounds = box;
	}


int MeshDeformPW::NumSubObjTypes() 
{ 
	return 1;
}

ISubObjType *MeshDeformPW::GetSubObjType(int i) 
{

	static bool initialized = false;
	if(!initialized)
	{
		initialized = true;
		SOT_Points.SetName(GetString(IDS_POINTS));
	}

	switch(i)
	{
	case 0:
		return &SOT_Points;
	}
	return NULL;
}

int MeshDeformPW::HitTest(
		TimeValue t, INode* inode, 
		int type, int crossing, int flags, 
		IPoint2 *p, ViewExp *vpt, ModContext* mc)
	{
	GraphicsWindow *gw = vpt->getGW();
	Point3 pt;
	HitRegion hr;
	int savedLimits, res = 0;


	MakeHitRegion(hr,type, crossing,4,p);
	gw->setHitRegion(&hr);	
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);	

	INode *node = NULL;
	Interval iv;

	for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
	{
		pblock->GetValue(pb_meshlist,t,node,iv,wrapIndex);

		if (node != NULL)
		{

			Matrix3 tm = node->GetObjTMAfterWSM(t);//CompMatrix(t,inode,mc);
			gw->setTransform(tm);

			gw->startMarkers();
			int ct = wrapMeshData[wrapIndex]->currentVertexTMs.Count();
			for (int i = 0; i < ct; i++)
			{
			// Hit test start point
				if ((flags&HIT_SELONLY   &&  wrapMeshData[wrapIndex]->selectedVerts[i]) ||
					(flags&HIT_UNSELONLY && !wrapMeshData[wrapIndex]->selectedVerts[i]) ||
					!(flags&(HIT_UNSELONLY|HIT_SELONLY)) ) 
				{
				
					gw->clearHitCode();

               Point3 pt(wrapMeshData[wrapIndex]->currentVertexTMs[i].GetTrans());
					gw->marker(&pt, SM_DOT_MRKR);
					if (gw->checkHitCode()) 
					{
						EnvHitData *eData = new EnvHitData(i,wrapIndex);
						vpt->LogHit(inode, mc, gw->getHitDistance(), i, eData); 
						res = 1;
					}
				}
			}
			gw->endMarkers();
		}
	}

	gw->setRndLimits(savedLimits);
	return res;
	}

void MeshDeformPW::SelectSubComponent(
		HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert)
	{
	if (theHold.Holding())  
	{
		for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
			theHold.Put (new SelectRestore (this,wrapIndex));
	}

	while (hitRec) 
	{
	
		int wrapIndex = ((EnvHitData *)hitRec->hitData)->whichNode;
		int vertIndex = ((EnvHitData *)hitRec->hitData)->whichVertex;
		BOOL state = selected;
		if (invert) state = !wrapMeshData[wrapIndex]->selectedVerts[vertIndex];
		if (state) wrapMeshData[wrapIndex]->selectedVerts.Set(vertIndex,1);
		else       wrapMeshData[wrapIndex]->selectedVerts.Set(vertIndex,0);
		if (!all) break;
		hitRec = hitRec->Next();
	}	


	SetRebuildNeighborDataModContext();

	for (int wrapIndex = 0; wrapIndex <  wrapMeshData.Count(); wrapIndex++)
	{
		if (wrapMeshData[wrapIndex]->mesh)
 			macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Wrap].meshDeformOps.setSelectVertices"), 3, 0, 
												mr_int, wrapIndex+1,
												mr_bitarray,&(wrapMeshData[wrapIndex]->selectedVerts),
												mr_bool, TRUE);
	}

	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	UpdateXYZSpinners();
	}

void MeshDeformPW::ClearSelection(int selLevel) 
{

	if (selLevel != 0) 
	{

		for (int wrapIndex = 0; wrapIndex < wrapMeshData.Count(); wrapIndex++)
		{
			if (wrapMeshData[wrapIndex])
			{	
				if (theHold.Holding())  
					theHold.Put (new SelectRestore (this,wrapIndex));
				wrapMeshData[wrapIndex]->selectedVerts.ClearAll ();
			}
		}
		SetRebuildNeighborDataModContext();
		UpdateXYZSpinners();
		NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	}
}

void MeshDeformPW::SelectAll(int selLevel) 
{

	if (selLevel != 0) 
	{
		for (int wrapIndex = 0; wrapIndex < wrapMeshData.Count(); wrapIndex++)
		{
			if (wrapMeshData[wrapIndex])
			{	
			if (theHold.Holding())  
				theHold.Put (new SelectRestore (this,wrapIndex));
			wrapMeshData[wrapIndex]->selectedVerts.SetAll ();
			}
		}
		SetRebuildNeighborDataModContext();
		UpdateXYZSpinners();
		NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

	}

}

void MeshDeformPW::InvertSelection(int selLevel) 
{

	if (selLevel != 0) 
	{
		for (int wrapIndex = 0; wrapIndex < wrapMeshData.Count(); wrapIndex++)
		{
			if (wrapMeshData[wrapIndex])
			{
				if (theHold.Holding())  
					theHold.Put (new SelectRestore (this,wrapIndex));
				wrapMeshData[wrapIndex]->selectedVerts = ~wrapMeshData[wrapIndex]->selectedVerts;
			}
		}
		SetRebuildNeighborDataModContext();
		UpdateXYZSpinners();
		NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

	}

}



void	MeshDeformPW::SelectVertices(int whichWrapMesh,BitArray *selList, BOOL updateViews)
{

	if ((whichWrapMesh < 0) || (whichWrapMesh >= wrapMeshData.Count())) return;

	wrapMeshData[whichWrapMesh]->selectedVerts.ClearAll();
	for (int i = 0; i < wrapMeshData[whichWrapMesh]->selectedVerts.GetSize(); i++)
	{
		if (i < selList->GetSize())
		{
			if ((*selList)[i])
				wrapMeshData[whichWrapMesh]->selectedVerts.Set(i,TRUE);
		}
	}
	SetRebuildNeighborDataModContext();
	if (updateViews)
	{
		NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		UpdateXYZSpinners();
	}
}

BitArray *MeshDeformPW::GetSelectedVertices(int whichWrapMesh)
{

	if ((whichWrapMesh < 0) || (whichWrapMesh >= wrapMeshData.Count())) return NULL;

	return &wrapMeshData[whichWrapMesh]->selectedVerts;
}

void MeshDeformPW::fnSetSelectVertices(int whichWrapMesh,BitArray *selList, BOOL updateViews)
{
	SelectVertices(whichWrapMesh,selList, updateViews);
}
BitArray *MeshDeformPW::fnGetSelectedVertices(int whichWrapMesh)
{
	if ((whichWrapMesh < 0) || (whichWrapMesh >= wrapMeshData.Count())) return NULL;
	return GetSelectedVertices(whichWrapMesh);
}


int MeshDeformPW::GetNumberControlPoints(int whichWrapMesh)
{
	if ((whichWrapMesh < 0) || (whichWrapMesh >= wrapMeshData.Count())) return 0;
	return wrapMeshData[whichWrapMesh]->initialData.Count();
}
int MeshDeformPW::fnGetNumberControlPoints(int whichWrapMesh)
{
	return GetNumberControlPoints(whichWrapMesh);
}


Point3* MeshDeformPW::fnGetPointScale(int whichWrapMesh, int index)
{	
	return GetPointScale(whichWrapMesh,index);
}

Point3* MeshDeformPW::GetPointScale( int whichWrapMesh,int index)
{
	if ((whichWrapMesh < 0) || (whichWrapMesh >= wrapMeshData.Count())) return NULL;

	if ( (index >= 0) && (index < wrapMeshData[whichWrapMesh]->initialData.Count()) )
	{
		return &wrapMeshData[whichWrapMesh]->initialData[index].localScale;
	}
	return NULL;
}


void MeshDeformPW::fnSetPointScale(int whichWrapMesh,int index, Point3 scale)
{
	SetPointScale(whichWrapMesh,index,scale);
}

void MeshDeformPW::SetPointScale(int whichWrapMesh,int index, Point3 scale)
{
	if ((whichWrapMesh < 0) || (whichWrapMesh >= wrapMeshData.Count())) return;
	if ( (index >= 0) && (index < wrapMeshData[whichWrapMesh]->initialData.Count()) )
	{
		wrapMeshData[whichWrapMesh]->initialData[index].localScale = scale;
	}

}


void MeshDeformPW::UpdateXYZSpinners(BOOL updateMult)
{
//get our spinners
	if (xSpin == NULL)
	{
		xSpin = GetISpinner(GetDlgItem(hWndParamPanel,IDC_X_SPIN));
		xSpin->LinkToEdit(GetDlgItem(hWndParamPanel,IDC_X),EDITTYPE_FLOAT);
		xSpin->SetLimits(0.001f, 100.0f, FALSE);
		xSpin->SetAutoScale();	
	}

	if (ySpin == NULL)
	{
		ySpin = GetISpinner(GetDlgItem(hWndParamPanel,IDC_Y_SPIN));
		ySpin->LinkToEdit(GetDlgItem(hWndParamPanel,IDC_Y),EDITTYPE_FLOAT);
		ySpin->SetLimits(0.001f, 100.0f, FALSE);
		ySpin->SetAutoScale();	
	}

	if (zSpin == NULL)
	{
		zSpin = GetISpinner(GetDlgItem(hWndParamPanel,IDC_Z_SPIN));
		zSpin->LinkToEdit(GetDlgItem(hWndParamPanel,IDC_Z),EDITTYPE_FLOAT);
		zSpin->SetLimits(0.001f, 100.0f, FALSE);
		zSpin->SetAutoScale();	
	}

	if (strSpin == NULL)
	{
		strSpin = GetISpinner(GetDlgItem(hWndParamPanel,IDC_STR_SPIN));
		strSpin->LinkToEdit(GetDlgItem(hWndParamPanel,IDC_STR),EDITTYPE_FLOAT);
		strSpin->SetLimits(-100.0f, 100.0f, FALSE);
		strSpin->SetAutoScale();	
	}

	if (multSpin == NULL)
	{
		multSpin = GetISpinner(GetDlgItem(hWndParamPanel,IDC_SCALEMULT_SPIN));
		multSpin->LinkToEdit(GetDlgItem(hWndParamPanel,IDC_SCALEMULT),EDITTYPE_FLOAT);
		multSpin->SetLimits(-100.0f, 100.0f, FALSE);
		multSpin->SetAutoScale();	
	}

	BOOL xValid,yValid,zValid,strValid;
	float x=1.0f,y=1.0f,z=1.0f,str = 1.0f;

	xValid = TRUE;
	yValid = TRUE;
	zValid = TRUE;
	strValid = TRUE;

	int ct = 0;
	ct = 0;
	for (int wrapIndex = 0; wrapIndex < wrapMeshData.Count(); wrapIndex++)
	{
		for (int i = 0; i < wrapMeshData[wrapIndex]->initialData.Count(); i++)
		{
			if ((i < wrapMeshData[wrapIndex]->selectedVerts.GetSize()) && wrapMeshData[wrapIndex]->selectedVerts[i])
			{

				if (ct == 0)
				{
					x = wrapMeshData[wrapIndex]->initialData[i].localScale.x;
					y = wrapMeshData[wrapIndex]->initialData[i].localScale.y;
					z = wrapMeshData[wrapIndex]->initialData[i].localScale.z;
					str = wrapMeshData[wrapIndex]->initialData[i].str;
				}
				else
				{
					float tx = wrapMeshData[wrapIndex]->initialData[i].localScale.x;
					float ty = wrapMeshData[wrapIndex]->initialData[i].localScale.y;
					float tz = wrapMeshData[wrapIndex]->initialData[i].localScale.z;
					float tstr = wrapMeshData[wrapIndex]->initialData[i].str;
					if (str != tstr)
						strValid = FALSE;
					if (x != tx)
						xValid = FALSE;
					if (y != ty)
						xValid = FALSE;
					if (z != tz)
						xValid = FALSE;

				}
				ct++;
			}
		}
	}


//see if all the X/Y/Z are the same
//if not set as indeterminate
//else set them to the value
	if (xValid)
	{
		xSpin->SetIndeterminate(FALSE);
		xSpin->SetValue(x,FALSE);
		
	}
	else xSpin->SetIndeterminate(TRUE);

	if (yValid)
	{
		ySpin->SetIndeterminate(FALSE);
		ySpin->SetValue(y,FALSE);
		
	}
	else ySpin->SetIndeterminate(TRUE);

	if (zValid)
	{
		zSpin->SetIndeterminate(FALSE);
		zSpin->SetValue(z,FALSE);		
	}
	else zSpin->SetIndeterminate(TRUE);

	if (strValid)
	{
		strSpin->SetIndeterminate(FALSE);
		strSpin->SetValue(str,FALSE);		
	}
	else strSpin->SetIndeterminate(TRUE);

	if (updateMult)
		multSpin->SetValue(1.0f,FALSE);

}

void MeshDeformPW::fnSetLocalScale(int axis,float scale)
{
	UpdateScale(axis,scale);
	UpdateXYZSpinners();
}
void MeshDeformPW::fnSetLocalStr(float str)
{
	UpdateStr(str);
	UpdateXYZSpinners();
}


void MeshDeformPW::UpdateScale(int axis,float scale)
{
	

	for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
	{

		int ct = 0;
		ct = wrapMeshData[wrapIndex]->selectedVerts.GetSize();
		for (int i = 0; i < ct; i++)
		{
			if (wrapMeshData[wrapIndex]->selectedVerts[i])
			{
				wrapMeshData[wrapIndex]->initialData[i].localScale[axis] = scale;
			}
		}
	}

	SetRebuildSelectedWeightDataModContext();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

void MeshDeformPW::MultScale(float scale)
{
	

 	for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
	{

		int ct = 0;
		ct = wrapMeshData[wrapIndex]->selectedVerts.GetSize();
		for (int i = 0; i < ct; i++)
		{
			if (wrapMeshData[wrapIndex]->selectedVerts[i])
			{
//				DebugPrint("%f scale %2.3f\n",scale,wrapMeshData[wrapIndex]->initialData[i].localScale.x);
				wrapMeshData[wrapIndex]->initialData[i].localScale *= scale;
			}
		}
	}

	SetRebuildSelectedWeightDataModContext();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}


void MeshDeformPW::UpdateStr(float str)
{
	for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
	{

		int ct = 0;
		ct = wrapMeshData[wrapIndex]->selectedVerts.GetSize();
		for (int i = 0; i < ct; i++)
		{
			if (wrapMeshData[wrapIndex]->selectedVerts[i])
			{
				wrapMeshData[wrapIndex]->initialData[i].str = str;
			}
		}
	}
	SetRebuildSelectedWeightDataModContext();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

}


float MeshDeformPW::fnGetPointStr(int whichWrapMesh, int index)
{
	return GetPointStr(whichWrapMesh,index);
}

void MeshDeformPW::fnSetPointStr(int whichWrapMesh, int index, float str)
{
	SetPointStr(whichWrapMesh,index,str);
}

float MeshDeformPW::GetPointStr(int whichWrapMesh, int index)
{
	if ((whichWrapMesh < 0) || (whichWrapMesh >= wrapMeshData.Count())) return 0.0f;
	if ( (index >= 0) && (index < wrapMeshData[whichWrapMesh]->initialData.Count()) )
	{
		return wrapMeshData[whichWrapMesh]->initialData[index].str;
	}
	return 0.0f;

}
void MeshDeformPW::SetPointStr(int whichWrapMesh, int index, float str)
{
	if ((whichWrapMesh < 0) || (whichWrapMesh >= wrapMeshData.Count())) return;
	if ( (index >= 0) && (index < wrapMeshData[whichWrapMesh]->initialData.Count()) )
	{
		wrapMeshData[whichWrapMesh]->initialData[index].str = str;
	}	

}


void MeshDeformPW::BuildLookUpData(LocalMeshData *ld, int wrapIndex)
{
//free our lookup list
	

	TimeValue t = GetCOREInterface()->GetTime();


	Interval iv;


	INode *node = NULL;
	pblock->GetValue(pb_meshlist,t,node,iv,wrapIndex);

	int faceLimit;
	pblock->GetValue(pb_facelimit,t,faceLimit,iv);


	if (node == NULL)
		{	
		return;
		}

	
	if (ld->selfNode)
	{
		Matrix3 meshTM = node->GetObjectTM(t, &iv);
		Matrix3 baseTM = ld->selfNode->GetObjectTM(t, &iv);

	//convert or collasp
		Mesh *mesh = wrapMeshData[wrapIndex]->mesh;

 		if (mesh == NULL) return;

		for (int i = 0; i < wrapMeshData[wrapIndex]->selectedVerts.GetSize(); i++)
		{
	//loop through
			if (wrapMeshData[wrapIndex]->selectedVerts[i])
			{
				//get our neightboring faces
				//bitarray
				BitArray faceList;
				faceList.SetSize(mesh->numFaces);

				BitArray vertList;
				vertList.SetSize(mesh->numVerts);

				faceList.ClearAll();
				vertList.ClearAll();
				vertList.Set(i);

				for (int j = 0; j < (faceLimit+1); j++)
				{
					//hold our verts
					BitArray holdSel = vertList;
					for (int k = 0; k < mesh->numFaces; k++)
					{
						int a = mesh->faces[k].v[0];
						int b = mesh->faces[k].v[1];
						int c = mesh->faces[k].v[2];

						//get our verts if any are originally selected
						if (holdSel[a] || holdSel[b] || holdSel[c])
						{

							faceList.Set(k,TRUE);
							vertList.Set(a,TRUE);
							vertList.Set(b,TRUE);
							vertList.Set(c,TRUE);
						}
						
					}
					//copy new selection over
				}
				
				//now copy the selection over
				
				vertList.SetSize(ld->paramData.Count());
				vertList.ClearAll();
				for (int j = 0; j < ld->paramData.Count(); j++)
				{
					int faceIndex = ld->faceIndex[wrapIndex]->f[j];//wrapMeshData[wrapIndex]->faceIndex[j];//ld->paramData[j].faceIndex;
//					int nodeIndex = ld->paramData[j].whichNode;
					if (( faceIndex >= 0) && faceList[faceIndex] /*&& (nodeIndex==wrapIndex)*/)
					{
						vertList.Set(j,TRUE);
					}
				}
				for (int j = 0; j < vertList.GetSize(); j++)
				{
					if (vertList[j])
					{
						ld->potentialWeightVerts.Set(j,TRUE);
					}
				}

			}

		}
	}

}

void MeshDeformPW::UpdateUI()
{
	
	Interval iv;
	TimeValue t = GetCOREInterface()->GetTime();
	int engine;
	pblock->GetValue(pb_engine,t,engine,iv);

	int subObjectLevel = 0;
	subObjectLevel = GetCOREInterface()->GetSubObjectLevel();
	
	//if face engine hide all UI
	if (engine == 0)
	{		
		if (xSpin)
			xSpin->Enable(FALSE);
		if (ySpin)
			ySpin->Enable(FALSE);
		if (zSpin)
			zSpin->Enable(FALSE);
		if (strSpin)
			strSpin->Enable(FALSE);

		if (multSpin)
			multSpin->Enable(FALSE);

		ISpinnerControl *spin;
/*
		spin = GetISpinner(GetDlgItem(hWndParamPanel,IDC_IDC_FALLOFF_SPIN));
		spin->LinkToEdit(GetDlgItem(hWndParamPanel,IDC_FALLOFF),EDITTYPE_FLOAT);
		spin->Enable(FALSE);
		ReleaseISpinner(spin);
*/
		spin = GetISpinner(GetDlgItem(hWndParamPanel,IDC_IDC_DISTANCE_SPIN));
		spin->LinkToEdit(GetDlgItem(hWndParamPanel,IDC_DISTANCE),EDITTYPE_FLOAT);
		spin->Enable(FALSE);
		ReleaseISpinner(spin);

		spin = GetISpinner(GetDlgItem(hWndParamPanel,IDC_IDC_FACELIMIT_SPIN));
		spin->LinkToEdit(GetDlgItem(hWndParamPanel,IDC_FACELIMIT),EDITTYPE_FLOAT);
		spin->Enable(FALSE);
		ReleaseISpinner(spin);


	}
	else
	{
		if (subObjectLevel == 0)
		{
			if (xSpin)
				xSpin->Enable(FALSE);
			if (ySpin)
				ySpin->Enable(FALSE);
			if (zSpin)
				zSpin->Enable(FALSE);
			if (strSpin)
				strSpin->Enable(FALSE);
			if (multSpin)
				multSpin->Enable(FALSE);

		}
		else
		{
			if (xSpin)
				xSpin->Enable(TRUE);
			if (ySpin)
				ySpin->Enable(TRUE);
			if (zSpin)
				zSpin->Enable(TRUE);
			if (strSpin)
				strSpin->Enable(TRUE);
			if (multSpin)
				multSpin->Enable(TRUE);


		}
	//unhide all the controls
		ISpinnerControl *spin;
		spin = GetISpinner(GetDlgItem(hWndParamPanel,IDC_IDC_FALLOFF_SPIN));
		spin->LinkToEdit(GetDlgItem(hWndParamPanel,IDC_FALLOFF),EDITTYPE_FLOAT);
		spin->Enable(TRUE);
		ReleaseISpinner(spin);

		spin = GetISpinner(GetDlgItem(hWndParamPanel,IDC_IDC_DISTANCE_SPIN));
		spin->LinkToEdit(GetDlgItem(hWndParamPanel,IDC_DISTANCE),EDITTYPE_FLOAT);
		spin->Enable(TRUE);
		ReleaseISpinner(spin);

		spin = GetISpinner(GetDlgItem(hWndParamPanel,IDC_IDC_FACELIMIT_SPIN));
		spin->LinkToEdit(GetDlgItem(hWndParamPanel,IDC_FACELIMIT),EDITTYPE_FLOAT);
		spin->Enable(TRUE);
		ReleaseISpinner(spin);
	//if subobject mode enable local controls
	}
}

void MeshDeformPW::fnMirrorSelectedVerts()
{
	MirrorSelectedVerts();
}


		//draw the mirrored points
class MirroredPoints
{
public:
	Point3 p;
	int whichVertex;
	int whichWrapMesh;
};

void MeshDeformPW::MirrorSelectedVerts()
{

	int axis;
	float offset;

	TimeValue t = GetCOREInterface()->GetTime();

	pblock->GetValue(pb_mirrorplane, t, axis, FOREVER);
	pblock->GetValue(pb_mirroroffset, t, offset, FOREVER);

	float threshold = 0.5f;
	pblock->GetValue(pb_mirrorthreshold, t, threshold, FOREVER);

	INode *node = NULL;


	//build out mirror tm
	BOOL first = TRUE;
	Matrix3 mirrorTM;
	for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
	{
		pblock->GetValue(pb_meshlist,t,node,FOREVER,wrapIndex);

		if ((node != NULL) && wrapMeshData[wrapIndex]->mesh)
		{
			if (first)
			{
				mirrorTM = node->GetObjectTM(t);
				first = FALSE;
			}
		}
	}

	Point3 vec = mirrorTM.GetRow(axis);
	vec = Normalize(vec) * offset;
	Point3 trans = mirrorTM.GetRow(3);
	trans += vec;
	mirrorTM.SetRow(3,trans);

	Matrix3 imirrorTM = Inverse(mirrorTM);



	Tab<MirroredPoints> mirroredPoints;

	for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
	{
		Interval iv;
		pblock->GetValue(pb_meshlist,t,node,iv,wrapIndex);

		if ((node != NULL) && wrapMeshData[wrapIndex]->mesh)
		{
			Matrix3 tm = node->GetObjectTM(t);			

			for (int i = 0; i < wrapMeshData[wrapIndex]->initialData.Count(); i++)
			{
				if (wrapMeshData[wrapIndex]->selectedVerts[i])
				{
					Point3 p = wrapMeshData[wrapIndex]->currentVertexTMs[i].GetTrans();
					p = p * tm;
					p = p * imirrorTM;
					p[axis] *= -1.0f;
					MirroredPoints d;
					d.p = p;
					d.whichVertex = i;
					d.whichWrapMesh = wrapIndex;
					mirroredPoints.Append(1,&d,100);
				}
			}
		}
	}

	theHold.Begin();
	for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
	{
		Interval iv;
		pblock->GetValue(pb_meshlist,t,node,iv,wrapIndex);

		if ((node != NULL) && wrapMeshData[wrapIndex]->mesh)
		{
			theHold.Put (new LocalScaleRestore (this,wrapIndex));
			theHold.Put (new LocalStrRestore (this,wrapIndex));
		}
	}
	theHold.Accept(GetString(IDS_MIRROR));	
	//loop through our points
	//loop through mirrored points and see if any one is close
	for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
	{
		Interval iv;
		pblock->GetValue(pb_meshlist,t,node,iv,wrapIndex);

		if ((node != NULL) && wrapMeshData[wrapIndex]->mesh)
		{
			Matrix3 tm = node->GetObjectTM(t);
			for (int i = 0; i < wrapMeshData[wrapIndex]->initialData.Count(); i++)
			{
				int closest = -1;
				float closestDist = 0.0f;
				Point3 p = wrapMeshData[wrapIndex]->initialData[i].initialVertexTMs.GetTrans() * tm;
				p = p * imirrorTM;
				for (int k = 0; k < mirroredPoints.Count(); k++)
				{
					float dist = Length(mirroredPoints[k].p - p);
					if ((dist < closestDist) || (closest == -1))
					{
						closestDist = dist;
						closest = k;
					}
				}					
				if ((closest != -1) && (closestDist < threshold))
				{
					int vi = mirroredPoints[closest].whichVertex;
					int wi = mirroredPoints[closest].whichWrapMesh;
					wrapMeshData[wrapIndex]->initialData[i].localScale = wrapMeshData[wi]->initialData[vi].localScale;
					wrapMeshData[wrapIndex]->initialData[i].str = wrapMeshData[wi]->initialData[vi].str;
				}
			}	

		}
	}

	SetRebuildNeighborDataModContext();
	SetRebuildSelectedWeightDataModContext();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

}


void MeshDeformPW::fnBakeControlPoints()
{
	BakeControlPoints();
}
void MeshDeformPW::fnRetreiveControlPoints()
{
	RetreiveControlPoints();
}

#define FALLOFF_APPDATA		0x2000
#define DISTANCE_APPDATA	0x2010
#define FACELIMIT_APPDATA	0x2020
#define ENGINE_APPDATA		0x2030
#define CPCOUNT_APPDATA		0x2040
#define CPDATA_APPDATA		0x2050


void MeshDeformPW::BakeControlPoints()
{
	//get our node

	INode *node = NULL;
	TimeValue t = GetCOREInterface()->GetTime();
	Interval iv;

	for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
	{
		pblock->GetValue(pb_meshlist,t,node,iv);


		float falloff;
		pblock->GetValue(pb_falloff,t,falloff,iv);

		float distance;
		pblock->GetValue(pb_distance,t,distance,iv);

		int faceLimit;
		pblock->GetValue(pb_facelimit,t,faceLimit,iv);

		int engine;
		pblock->GetValue(pb_engine,t,engine,iv);

		if (node == NULL) continue;
		if (wrapMeshData[wrapIndex] == NULL) continue;


		float* pFalloff = static_cast<float*>(MAX_malloc(sizeof(float)));
		float* pDistance = static_cast<float*>(MAX_malloc(sizeof(float)));
		int* pEngine = static_cast<int*>(MAX_malloc(sizeof(int)));
		int* pFaceLimit = static_cast<int*>(MAX_malloc(sizeof(int)));

		*pFalloff = falloff;
		*pDistance = distance;
		*pEngine = engine;
		*pFaceLimit = faceLimit;

		//attache the app data
		//falloff
		node->RemoveAppDataChunk(MESHDEFORMPW_CLASS_ID,OSM_CLASS_ID,FALLOFF_APPDATA);
		node->AddAppDataChunk(MESHDEFORMPW_CLASS_ID,OSM_CLASS_ID,FALLOFF_APPDATA,sizeof(falloff),pFalloff);

		//get the engine
		node->RemoveAppDataChunk(MESHDEFORMPW_CLASS_ID,OSM_CLASS_ID,ENGINE_APPDATA);
		node->AddAppDataChunk(MESHDEFORMPW_CLASS_ID,OSM_CLASS_ID,ENGINE_APPDATA,sizeof(engine),pEngine);
		//dist
		node->RemoveAppDataChunk(MESHDEFORMPW_CLASS_ID,OSM_CLASS_ID,DISTANCE_APPDATA);
		node->AddAppDataChunk(MESHDEFORMPW_CLASS_ID,OSM_CLASS_ID,DISTANCE_APPDATA,sizeof(distance),pDistance);

		//face limit
		node->RemoveAppDataChunk(MESHDEFORMPW_CLASS_ID,OSM_CLASS_ID,FACELIMIT_APPDATA);
		node->AddAppDataChunk(MESHDEFORMPW_CLASS_ID,OSM_CLASS_ID,FACELIMIT_APPDATA,sizeof(faceLimit),pFaceLimit);

		//number of control points
		int ct = this->wrapMeshData[wrapIndex]->initialData.Count();
		int *pCt = static_cast<int*>(MAX_malloc(sizeof(int)));
		*pCt = ct;
		node->RemoveAppDataChunk(MESHDEFORMPW_CLASS_ID,OSM_CLASS_ID,CPCOUNT_APPDATA);
		node->AddAppDataChunk(MESHDEFORMPW_CLASS_ID,OSM_CLASS_ID,CPCOUNT_APPDATA,sizeof(ct),pCt);

		//control point data
		node->RemoveAppDataChunk(MESHDEFORMPW_CLASS_ID,OSM_CLASS_ID,CPDATA_APPDATA);
		InitialTMData* initialDataTemp = static_cast<InitialTMData*>(MAX_malloc(sizeof(InitialTMData)*ct));
		for (int i = 0; i < ct; i++) {
			initialDataTemp[i] = wrapMeshData[wrapIndex]->initialData[i];
		}
 		node->AddAppDataChunk(MESHDEFORMPW_CLASS_ID,OSM_CLASS_ID,CPDATA_APPDATA,sizeof(InitialTMData)*ct,initialDataTemp);
	}
}
void MeshDeformPW::RetreiveControlPoints()
{

	INode *node = NULL;
	TimeValue t = GetCOREInterface()->GetTime();
	Interval iv;

	theHold.Begin();
	for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
	{
		pblock->GetValue(pb_meshlist,t,node,iv,wrapIndex);

		if (node == NULL) continue;
		if (wrapMeshData[wrapIndex]->mesh == NULL) continue;

		theHold.Put (new LocalScaleRestore (this,wrapIndex));
		theHold.Put (new LocalStrRestore (this,wrapIndex));
	}
	theHold.Accept(GetString(IDS_RETREIVE));	

	for (int wrapIndex = 0; wrapIndex < pblock->Count(pb_meshlist); wrapIndex++)
	{
		pblock->GetValue(pb_meshlist,t,node,iv,wrapIndex);

		if (node == NULL) continue;
		if (wrapMeshData[wrapIndex]->mesh == NULL) continue;

		float falloff = 1.0f;
		float distance = 1.2f;
		int faceLimit = 3;
		int engine = 1;

		AppDataChunk *chunk;

		chunk = node->GetAppDataChunk(MESHDEFORMPW_CLASS_ID,OSM_CLASS_ID,FALLOFF_APPDATA);
		if (chunk)
		{
			float *f;
			f = (float*)chunk->data;
			falloff = *f;
		}
		
		chunk = node->GetAppDataChunk(MESHDEFORMPW_CLASS_ID,OSM_CLASS_ID,ENGINE_APPDATA);
		if (chunk)
		{
			int *i;
			i = (int *) chunk->data;
			engine = *i;
		}
			
		chunk = node->GetAppDataChunk(MESHDEFORMPW_CLASS_ID,OSM_CLASS_ID,DISTANCE_APPDATA);
		if (chunk)
		{
			float *f;
			f = (float*)chunk->data;
			distance = *f;
		}
			
		chunk = node->GetAppDataChunk(MESHDEFORMPW_CLASS_ID,OSM_CLASS_ID,FACELIMIT_APPDATA);
		if (chunk)
		{
			int *i;
			i = (int *) chunk->data;
			faceLimit = *i;
		}

		int ct = 0;

		chunk = node->GetAppDataChunk(MESHDEFORMPW_CLASS_ID,OSM_CLASS_ID,CPCOUNT_APPDATA);
		if (chunk)
		{
			int *i;
			i = (int *) chunk->data;
			ct = *i;
		}

		InitialTMData *initialDataTemp = NULL;

		chunk = node->GetAppDataChunk(MESHDEFORMPW_CLASS_ID,OSM_CLASS_ID,CPDATA_APPDATA);
		if (chunk)
		{
			initialDataTemp = (InitialTMData *) chunk->data;
		}

		if (ct == this->wrapMeshData[wrapIndex]->initialData.Count())
		{
			for (int i = 0; i < ct; i++)
			{
				wrapMeshData[wrapIndex]->initialData[i].localScale = initialDataTemp[i].localScale;
				wrapMeshData[wrapIndex]->initialData[i].str = initialDataTemp[i].str;
			}

			SuspendAnimate();
			AnimateOff();
			pblock->SetValue(pb_falloff,t,falloff);
			pblock->SetValue(pb_distance,t,distance);
			pblock->SetValue(pb_facelimit,t,faceLimit);
			pblock->SetValue(pb_engine,t,engine);
			ResumeAnimate();
		}
	}
	SetRebuildNeighborDataModContext();
	Resample();

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());


}

void  MeshDeformPW::fnResample()
{
	Resample();
}
void  MeshDeformPW::Resample()
{

	resample = TRUE;
//	SetResampleModContext();
	NotifyDependents(FOREVER,GEOM_CHANNEL,REFMSG_CHANGE);
	
}


Matrix3* MeshDeformPW::fnGetPointInitialTM(int whichWrapMesh, int index)
{

	if ((whichWrapMesh < 0) || (whichWrapMesh >= wrapMeshData.Count())) return NULL;
	if ((index < 0) || (index >= wrapMeshData[whichWrapMesh]->initialData.Count()))
		return NULL;
	else return &wrapMeshData[whichWrapMesh]->initialData[index].initialVertexTMs;
	
}
Matrix3* MeshDeformPW::fnGetPointCurrentTM(int whichWrapMesh, int index)
{
	if ((whichWrapMesh < 0) || (whichWrapMesh >= wrapMeshData.Count())) return NULL;

	if ((index < 0) || (index >= wrapMeshData[whichWrapMesh]->currentVertexTMs.Count()))
		return NULL;
	else return &wrapMeshData[whichWrapMesh]->currentVertexTMs[index];

}
float MeshDeformPW::fnGetPointDist(int whichWrapMesh, int index)
{
	return GetPointDist(whichWrapMesh,index);
}
int MeshDeformPW::fnGetPointXVert(int whichWrapMesh, int index)
{
	return GetPointXVert(whichWrapMesh,index)+1;
}
Matrix3 MeshDeformPW::GetPointInitialTM(int whichWrapMesh, int index)
{
	if ((whichWrapMesh < 0) || (whichWrapMesh >= wrapMeshData.Count())) return Matrix3(1);
	
	if ((index < 0) || (index >= wrapMeshData[whichWrapMesh]->initialData.Count()))
		return Matrix3(1);
	else return wrapMeshData[whichWrapMesh]->initialData[index].initialVertexTMs;
	
}
Matrix3 MeshDeformPW::GetPointCurrentTM(int whichWrapMesh, int index)
{
	if ((whichWrapMesh < 0) || (whichWrapMesh >= wrapMeshData.Count())) return Matrix3(1);
	if ((index < 0) || (index >= wrapMeshData[whichWrapMesh]->currentVertexTMs.Count()))
		return Matrix3(1);
	else return wrapMeshData[whichWrapMesh]->currentVertexTMs[index];
}
float MeshDeformPW::GetPointDist(int whichWrapMesh, int index)
{
	if ((whichWrapMesh < 0) || (whichWrapMesh >= wrapMeshData.Count())) return 0.0f;
	
	if ((index < 0) || (index >= wrapMeshData[whichWrapMesh]->initialData.Count()))
		return 0;
	else return wrapMeshData[whichWrapMesh]->initialData[index].dist;
	
}
int MeshDeformPW::GetPointXVert(int whichWrapMesh, int index)
{
	if ((whichWrapMesh < 0) || (whichWrapMesh >= wrapMeshData.Count())) return 0;
	
	if ((index < 0) || (index >= wrapMeshData[whichWrapMesh]->initialData.Count()))
		return 0;
	else return wrapMeshData[whichWrapMesh]->initialData[index].xPoints;
	
}


int MeshDeformPW::fnNumberOfVertices(INode *node)
{
	return NumberOfVertices(node);
}

int MeshDeformPW::fnVertNumberWeights(INode *node, int vindex)
{
	
	return VertNumberWeights(node, vindex);
}
float MeshDeformPW::fnVertGetWeight(INode *node, int vindex, int windex)
{
	return VertGetWeight(node, vindex, windex);

}
float MeshDeformPW::fnVertGetDistance(INode *node, int vindex, int windex)
{
	return VertGetDistance(node, vindex, windex);

}
int MeshDeformPW::fnVertGetControlPoint(INode *node, int vindex, int windex)
{
	return VertGetControlPoint(node, vindex, windex);

}

LocalMeshData *RecursePipeAndFindLD(Modifier *thisMod, Object *obj)
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
					if (dobj->GetModifier(j) == thisMod)
					{
						ModContext *mc = dobj->GetModContext(j);
						return (LocalMeshData *) mc->localData;
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
			return RecursePipeAndFindLD(thisMod, bobj);
			}

		}

	return NULL;
}


LocalMeshData *MeshDeformPW::GetLocalData(INode *node)
{
	if (node)
	{
		Object* obj = node->GetObjectRef();
	
		return RecursePipeAndFindLD(this,obj);
	}
	return NULL;

}

int MeshDeformPW::NumberOfVertices(INode *node)
{
	LocalMeshData *ld = GetLocalData(node);
	if (ld)
		return ld->weightData.Count();

	return 0;
}
int MeshDeformPW::VertNumberWeights(INode *node, int vindex)
{
	LocalMeshData *ld = GetLocalData(node);

	if (ld)
	{
		if ((vindex >= 0) && (vindex < ld->weightData.Count()))
			return ld->weightData[vindex]->data.Count();
	}
	return 0;
}
float MeshDeformPW::VertGetWeight(INode *node, int vindex, int windex)
{
	LocalMeshData *ld = GetLocalData(node);

	if (ld)
	{
		if ((vindex >= 0) && (vindex < ld->weightData.Count()))
		{
			if ((windex >= 0) && (windex < ld->weightData[vindex]->data.Count()))
			{
				return ld->weightData[vindex]->data[windex].normalizedWeight;
			}
		}
	}
	return 0.0f;
}
float MeshDeformPW::VertGetDistance(INode *node, int vindex, int windex)
{
	LocalMeshData *ld = GetLocalData(node);

	if (ld)
	{
		if ((vindex >= 0) && (vindex < ld->weightData.Count()))
		{
			if ((windex >= 0) && (windex < ld->weightData[vindex]->data.Count()))
			{
				return ld->weightData[vindex]->data[windex].dist;
			}
		}
	}
	return 0.0f;
}
int MeshDeformPW::VertGetControlPoint(INode *node, int vindex, int windex)
{
	LocalMeshData *ld = GetLocalData(node);

	if (ld)
	{
		if ((vindex >= 0) && (vindex < ld->weightData.Count()))
		{
			if ((windex >= 0) && (windex < ld->weightData[vindex]->data.Count()))
			{
				return ld->weightData[vindex]->data[windex].owningBone;
			}
		}
	}
	return 0;
}


void MeshDeformPW::fnReset()
{
	Reset();
}
void MeshDeformPW::Reset()
{
    MeshEnumProc dep;              
	DoEnumDependents(&dep);
	for ( int i = 0; i < dep.Nodes.Count(); i++)
		{
		INode *node = dep.Nodes[i];
		BOOL found = FALSE;

		if (node)
			{
			Object* obj = node->GetObjectRef();
	
			 RecursePipeAndMatch2(this,obj,FIELD_RESET) ;
			}
		}
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

}


int MeshDeformPW::fnVertGetWrapNode(INode *node, int vindex, int windex)
{
	return VertGetWrapNode(node, vindex, windex);
}
int MeshDeformPW::VertGetWrapNode(INode *node, int vindex, int windex)
{
	LocalMeshData *ld = GetLocalData(node);

	if (ld)
	{
		if ((vindex >= 0) && (vindex < ld->weightData.Count()))
		{
			if ((windex >= 0) && (windex < ld->weightData[vindex]->data.Count()))
			{
				return ld->weightData[vindex]->data[windex].whichNode;
			}
		}
	}
	return 0;
}


Modifier* FindSkin(Object *obj)
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
					Modifier *mod = dobj->GetModifier(j);
					if (mod->ClassID() == SKIN_CLASSID)
						return mod;

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
			return FindSkin( bobj);
			}

		}

	return NULL;
}


class PerSkinData
{
public:
	Modifier *skinMod;
	ISkin *iskin;
	Tab<INode*> bones;
	Tab<int> toNewBoneIndex;
};

void MeshDeformPW::ConvertToSkin(BOOL silent)
{
	SkinWraptoSkin(silent);
}
void MeshDeformPW::fnConvertToSkin(BOOL silent)
{
	SkinWraptoSkin(silent);
}


BOOL MeshDeformPW::SkinWraptoSkin( BOOL silent)
{
	worldBounds.Init();
	worldBounds += Point3(0.0f,0.0f,0.0f);

	INode *selfNode = NULL;

    MeshEnumProc dep;              
	DoEnumDependents(&dep);
	LocalMeshData *localData = NULL;
	int ct = 0;
	for (int i = 0; i < dep.Nodes.Count(); i++)
	{
		INode *node = dep.Nodes[i];
		LocalMeshData *ld = GetLocalData(node);
		if (ld)
		{
			localData = ld;
			selfNode = node;		
			ct++;
		}
	}

	if  (ct > 1)
	{
		TSTR errorMsg;
		errorMsg.printf("%s",GetString(IDS_ERROR3));
		MessageBox(this->hWnd,errorMsg,errorMsg,MB_OK|MB_ICONWARNING);
		return FALSE;
	}
	if (selfNode == NULL)
		return FALSE;
	if (localData == NULL) return FALSE;


	Tab<INode*> wrapNodes;
	TimeValue t = GetCOREInterface()->GetTime();

	for (int i = 0; i < pblock->Count(pb_meshlist); i++)
	{
		INode *node;
		pblock->GetValue(pb_meshlist,t,node,FOREVER,i);
		wrapNodes.Append(1,&node,100);
	}

	Tab<PerSkinData*> skinData;
	skinData.SetCount(wrapNodes.Count());
	for (int i = 0; i < wrapNodes.Count(); i++)
		skinData[i] = new PerSkinData();
	for (int i = 0; i < wrapNodes.Count(); i++)
	{		
		Modifier *mod;
		if (wrapNodes[i])
		{
			Object* obj = wrapNodes[i]->GetObjectRef();

			mod = FindSkin(obj);
			if (mod == NULL)
			{
				if (!silent)
				{
					//pop message box here and bail
					TSTR errorMsg;
					errorMsg.printf("%s %s",GetString(IDS_ERROR1),wrapNodes[i]->GetName());
					MessageBox(this->hWnd,errorMsg,errorMsg,MB_OK|MB_ICONWARNING);
				}
				for (int k = 0; k < skinData.Count(); k++)
					delete skinData[k];
				return FALSE;
			}
			skinData[i]->skinMod = mod;
			skinData[i]->iskin = (ISkin*) mod->GetInterface(I_SKIN);
		}
		else
		{
			skinData[i]->skinMod = NULL;
			skinData[i]->iskin = NULL;
		}
	}

	//gather all the bones
	Tab<INode *> boneNodeList;
	for (int i = 0; i < wrapNodes.Count(); i++)
	{
		if (skinData[i]->iskin)
		{
			int numberOfBones = skinData[i]->iskin->GetNumBones();
			skinData[i]->bones.SetCount(numberOfBones);
			skinData[i]->toNewBoneIndex.SetCount(numberOfBones);
			for (int boneIndex = 0; boneIndex < numberOfBones; boneIndex++)
			{
				INode *boneNode = skinData[i]->iskin->GetBone(boneIndex);
				skinData[i]->bones[boneIndex] = boneNode;
				if (boneNode != NULL)
				{
					BOOL found = FALSE;
					for (int k = 0; k < boneNodeList.Count(); k++)
					{
						if (boneNode == boneNodeList[k])
							found = TRUE;
					}
					if (!found)
						boneNodeList.Append(1,&boneNode, 100);
				}
			}
		}
	}

	//now build our connection data
	for (int i = 0; i < wrapNodes.Count(); i++)
	{
		for (int j = 0; j < skinData[i]->bones.Count(); j++)
		{
			INode *boneNode = skinData[i]->bones[j];
			for (int k = 0; k < boneNodeList.Count(); k++)
			{
				if (boneNode == boneNodeList[k])
					skinData[i]->toNewBoneIndex[j] = k;
			}
		}
	}


	theHold.Begin();
	//create a skin modifier and add it
	Modifier *skinMod = (Modifier*) CreateInstance(OSM_CLASS_ID, SKIN_CLASSID);

	TSTR prompt;
	prompt.printf("%s", GetString(IDS_STEP5));
	ip->ReplacePrompt( prompt);

	Object* obj = selfNode->GetObjectRef();	
	ISkinImportData *iskinImport = NULL;
	if (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		IDerivedObject *dobj = (IDerivedObject *) obj;

		dobj->SetAFlag(A_LOCK_TARGET);
		dobj->AddModifier(skinMod);
		dobj->ClearAFlag(A_LOCK_TARGET);

		ISkin *iskin = (ISkin*) skinMod->GetInterface(I_SKIN);
		iskinImport = (ISkinImportData*) skinMod->GetInterface(I_SKINIMPORTDATA);



		if ((iskin) && (iskinImport))
		{
			int numberOfBones = boneNodeList.Count();
			for (int i = 0; i < numberOfBones; i++)
			{
				if (i == (numberOfBones-1))
					iskinImport->AddBoneEx(boneNodeList[i],TRUE);
				else iskinImport->AddBoneEx(boneNodeList[i],FALSE);
			}
		}
		else
		{
			TSTR errorMsg;
			errorMsg.printf("%s",GetString(IDS_ERROR2));
			MessageBox(this->hWnd,errorMsg,errorMsg,MB_OK|MB_ICONWARNING);

			for (int k = 0; k < skinData.Count(); k++)
				delete skinData[k];
			return FALSE;
		}

	}
	else
	{

		TSTR errorMsg;
		errorMsg.printf("%s",GetString(IDS_ERROR2));
		MessageBox(this->hWnd,errorMsg,errorMsg,MB_OK|MB_ICONWARNING);

		//pop message box here and bail
		for (int k = 0; k < skinData.Count(); k++)
			delete skinData[k];
		return FALSE;
	}

	for (int i = 0; i < localData->weightData.Count(); i++)
	{
		int numberOfBones = localData->weightData[i]->data.Count();
		for (int j = 0; j < numberOfBones; j++)
		{
			int whichVert = localData->weightData[i]->data[j].owningBone;
			int whichWrapMesh = localData->weightData[i]->data[j].whichNode;
			if (skinData[whichWrapMesh]->iskin)
			{
				ISkinContextData *sdata = skinData[whichWrapMesh]->iskin->GetContextInterface(wrapNodes[whichWrapMesh]);
				if (sdata && (whichVert >= sdata->GetNumPoints()))
				{
					TSTR errorMsg;
					errorMsg.printf("%s",GetString(IDS_ERROR4));
					MessageBox(this->hWnd,errorMsg,errorMsg,MB_OK|MB_ICONWARNING);

					//pop message box here and bail
					for (int k = 0; k < skinData.Count(); k++)
						delete skinData[k];
					return FALSE;
				}
			}
		}
	}

	
	GetCOREInterface()->ForceCompleteRedraw();

	//add our weights;
	Tab<double> weights;
	weights.SetCount(boneNodeList.Count());	
	for (int i = 0; i < localData->weightData.Count(); i++)
	{
		for (int j = 0; j < weights.Count(); j++)
			weights[j] = 0.0;
		int numberOfBones = localData->weightData[i]->data.Count();

		if ((ip) && (localData->weightData.Count() > 10000))
		{
			TSTR prompt;
			if ((i%200) == 0)
			{
				float per = (float)i/(float)localData->weightData.Count()*100.0f;
				prompt.printf("%s : %d(%d) (%0.0f%%)", GetString(IDS_STEP4),i,localData->weightData.Count(),per);
				ip->ReplacePrompt( prompt);
			}
		}


		for (int j = 0; j < numberOfBones; j++)
		{
			float vertw = localData->weightData[i]->data[j].normalizedWeight;
			int whichVert = localData->weightData[i]->data[j].owningBone;
			int whichWrapMesh = localData->weightData[i]->data[j].whichNode;			

			//get our skin weights
			if (skinData[whichWrapMesh]->iskin)
			{
				ISkinContextData *sdata = skinData[whichWrapMesh]->iskin->GetContextInterface(wrapNodes[whichWrapMesh]);
				if (sdata->GetNumPoints()>0)
				{
					int numberOfWeights = sdata->GetNumAssignedBones(whichVert);
					for (int k = 0; k < numberOfWeights; k++)
					{
						double weight = (double)sdata->GetBoneWeight(whichVert,k) * (double)vertw;
						int boneIndex = sdata->GetAssignedBone(whichVert,k);
						int newindex = skinData[whichWrapMesh]->toNewBoneIndex[boneIndex];
						weights[newindex] += weight;
					}
				}
			}
		}
		double sum = 0.0;
		for (int j = 0; j < weights.Count(); j++)
		{
			sum += weights[j];
		}
		if (sum != 0.0f)
		{
			for (int j = 0; j < weights.Count(); j++)
			{
//DebugPrint("Bone Weights %d %0.8f normalized %0.5f\n",j,weights[j],weights[j]/sum);

				weights[j] = weights[j]/sum;
			}
		}
		Tab<INode*> newBones;
		Tab<float> newWeights;
		for (int j = 0; j < weights.Count(); j++)
		{
			if (weights[j] > 0.0f)
			{
				newBones.Append(1,&boneNodeList[j],100);
				float w = weights[j];
				newWeights.Append(1,&w,100);
			}
		}



		iskinImport->AddWeights(selfNode,i,newBones,newWeights);
	}

	//add the bonesle
	theHold.Put (new EnableRestore (this));
	this->DisableMod();

	theHold.Accept(GetString(IDS_CONVERT));

	ip->DeSelectNode(selfNode);
	GetCOREInterface()->SelectNode(selfNode); 	

	prompt;
	prompt.printf(" ");
	GetCOREInterface()->ReplacePrompt( prompt);


	return TRUE;

/*
	check to see if all our wrapped objects are skinned
	get all the bones froms those skinned objects
	loop through our localdata weights
		loop through our bones
		get the wrap mesh bone
		get the skin weights for that bone
		add them to our list

	add a skin modifier to wrap object
	add the bones
	add the weights
	disable our skin wrap modifier
*/

}


BOOL PickControlNode::Filter(INode *node)
	{
	node->BeginDependencyTest();
	mod->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) 
	{		
		return FALSE;
	} 


	for (int i = 0;i < mod->pblock->Count(pb_meshlist); i++)
	{
		INode *tnode = NULL;
		mod->pblock->GetValue(pb_meshlist,0,tnode,FOREVER,i);	
		if  (node == tnode)
			return FALSE;


	}

	TimeValue t = GetCOREInterface()->GetTime();
	ObjectState meshos = node->EvalWorldState(t);
	if (meshos.obj->IsSubClassOf(triObjectClassID))
		return TRUE;
	else if (meshos.obj->CanConvertToType(triObjectClassID))
		return TRUE;
	else return FALSE;

	}

BOOL PickControlNode::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{	
	if (ip->PickNode(hWnd,m,this)) {
		return TRUE;
	} else {
		return FALSE;
		}
	}

BOOL PickControlNode::Pick(IObjParam *ip,ViewExp *vpt)
	{
	INode *node = vpt->GetClosestHit();
	if (node) 
		{
			theHold.Begin();
			mod->pblock->Append(pb_meshlist,1,&node,1);
			theHold.Accept(GetString(IDS_ADDWRAP));
			return FALSE;		
		}
	return FALSE;
	}

void PickControlNode::EnterMode(IObjParam *ip)
	{

		mod->iPickButton->SetCheck(TRUE);

	}


void PickControlNode::ExitMode(IObjParam *ip)
	{
		mod->iPickButton->SetCheck(FALSE);
	}


void MeshDeformPW::fnOutputVertexData(INode *whichNode, int whichVert)
{

	whichVert--;
 	LocalMeshData *ld = GetLocalData(whichNode);
	if (ld)
	{
		if (whichVert < ld->weightData.Count())
		{
			ScriptPrint("Vertex %d\n",whichVert);
			ScriptPrint("Number of weights %d\n",ld->weightData[whichVert]->data.Count());
			for (int i = 0; i < ld->weightData[whichVert]->data.Count(); i++)
			{
				int nindex = ld->weightData[whichVert]->data[i].owningBone;
				float f = ld->weightData[whichVert]->data[i].normalizedWeight;
				INode *node;
				pblock->GetValue(pb_meshlist,0,node,FOREVER,nindex);
				if (node)
					ScriptPrint("Weight %f Owner %s\n",f,node->GetName());
				else ScriptPrint("Weight %f Owner NULL\n",f);
			}
		}
	}
}
