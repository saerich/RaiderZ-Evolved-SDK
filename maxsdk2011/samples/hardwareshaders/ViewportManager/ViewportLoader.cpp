/**********************************************************************
 *<
	FILE: ViewportLoader.cpp

	DESCRIPTION:	Viewport Manager for loading up Effects

	CREATED BY:		Neil Hazzard

	HISTORY:		Created:  02/15/02
					

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/
#include <d3dx9.h>

#include "maxscrpt\MAXScrpt.h"
#include "maxscrpt\MAXObj.h"
#include "maxscrpt\3DMath.h"
#include "maxscrpt\msplugin.h"

#include "ViewportManager.h"
#include "ViewportLoader.h"
#include "IMaterialViewportShading.h"

#ifndef NO_ASHLI
#include "rtmax.h"
#include "ID3D9GraphicsWindow.h"
#pragma comment( lib, "rtmax.lib" )
#endif

#include "3dsmaxport.h"

class ViewportLoaderClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading) {return new ViewportLoader;}
	const TCHAR *	ClassName() { return GetString(IDS_VPMCLASSNAME); }

	SClass_ID		SuperClassID() {return CUST_ATTRIB_CLASS_ID;}
	Class_ID 		ClassID() {return VIEWPORTLOADER_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	const TCHAR*	InternalName() { return _T("ViewportManager"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
	};

static ViewportLoaderClassDesc theViewportLoaderClassDesc;
ClassDesc2* GetViewportLoaderDesc(){ return &theViewportLoaderClassDesc;}

static void GetSceneLights(Tab<INode*> & lights)
{
	Interface *ip	  = GetCOREInterface();
	TimeValue t  = ip->GetTime();
	INode * Root  = ip->GetRootNode();
	int Count = Root->NumberOfChildren();
	int i=0;

	for( i=0; i < Count; i++) 
	{
		INode * node = Root->GetChildNode(i);
		ObjectState Os   = node->EvalWorldState(t);

		if(Os.obj && Os.obj->SuperClassID() == LIGHT_CLASS_ID) 
		{
			lights.Append(1, &node);
		}
	}
}

static LPDIRECT3DDEVICE9 GetDevice()
{
#ifndef NO_ASHLI
	GraphicsWindow		*lpGW = NULL;
	ViewExp				*lpView = NULL;
	static LPDIRECT3DDEVICE9 lpDevice = NULL;

	if(lpDevice)
		return lpDevice;

	lpView = GetCOREInterface()->GetActiveViewport();
	if(lpView)
	{
		lpGW = lpView->getGW();
		if(lpGW)
		{
			ID3D9GraphicsWindow *lpD3DGW = (ID3D9GraphicsWindow *)lpGW->GetInterface(D3D9_GRAPHICS_WINDOW_INTERFACE_ID);
			if(lpD3DGW)
			{
				lpDevice = lpD3DGW->GetDevice();
			}
		}
		GetCOREInterface()->ReleaseViewport(lpView);
	}
	
	return lpDevice;
#else
	return NULL;
#endif
}

static ICustAttribContainer * GetOwnerContainer(ReferenceTarget * owner)
{
	if (NULL == owner) {
		return NULL;
	}

	//Lets find our Parent - which should be a custom Attribute Container
	DependentIterator di(owner);
	ReferenceMaker* maker = NULL;
	while ((maker = di.Next()) != NULL) 
	{
		if (maker->SuperClassID() == REF_MAKER_CLASS_ID &&
				maker->ClassID() == CUSTATTRIB_CONTAINER_CLASS_ID ) 
		{
			return (ICustAttribContainer*)maker;
		}
	}
	
	return NULL;
}

class EffectPBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		ViewportLoader* l_pViewportLoader = (ViewportLoader*)owner;

		switch (id)
		{
		case pb_effect:
			l_pViewportLoader->HandleEffectIndexChanged(t, v.i);
			break;
		case pb_enabled:
			l_pViewportLoader->HandleEffectStateChanged(t, v.i != FALSE);
			break;
		case pb_dxStdMat:
			l_pViewportLoader->HandleDxStdMtlChanged(t, v.i != FALSE);
			break;
		case pb_saveFX2:
			l_pViewportLoader->HandleSaveFxFile(t, v.s);
			break;
		}
	}
};

static EffectPBAccessor effectPBAccessor;


INT_PTR EffectsDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
		case WM_INITDIALOG:
		{

			HWND hwndeffect = GetDlgItem(hWnd, IDC_EFFECTLIST);
			SendMessage(hwndeffect, CB_RESETCONTENT, 0L, 0L);
			for (int i = 0; i < vl->NumShaders(); i++) {
				ClassDesc* pClassD = vl->GetEffectCD(i);
				int n = SendMessage(hwndeffect, CB_ADDSTRING, 0L, (LPARAM)(pClassD->ClassName()) );
			}
			SendMessage(hwndeffect, CB_INSERTSTRING, 0L, (LPARAM)GetString(IDS_STR_NONE));
			SendMessage(hwndeffect, CB_SETCURSEL, vl->effectIndex,0L);

			if (vl->IsCurrentEffectSupported()) {
				vl->EnableDlgItem(IDC_ENABLED, TRUE);
			}
			else {
				vl->SetCurrentEffectEnabled(false);
				vl->EnableDlgItem(IDC_ENABLED, FALSE);
			}

			vl->EnableDlgItem(IDC_SAVE_FX, vl->IsSaveFxSupported());

			return TRUE;
		}
	}
	return FALSE;
}




static ParamBlockDesc2 param_blk ( viewport_manager_params, _T("DirectX Manager"),  0, &theViewportLoaderClassDesc, 
								  P_AUTO_CONSTRUCT + P_AUTO_UI + P_HASCATEGORY, PBLOCK_REF, 
	//rollout
	IDD_CUSTATTRIB, IDS_VIEWPORT_MANAGER, 0, 0, NULL, ROLLUP_CAT_CUSTATTRIB, 
	// params

	pb_enabled, 	_T("enabled"),		TYPE_BOOL, 	0, 	IDS_ENABLED, 
		p_default, 		FALSE, 
		p_ui,			TYPE_SINGLECHEKBOX, IDC_ENABLED,
		p_accessor,		&effectPBAccessor,
		end,

	pb_effect,		_T("effect"),	TYPE_INT,		0, 	IDS_EFFECT, 	
		p_default, 		0, 
		p_ui, 			TYPE_INTLISTBOX, IDC_EFFECTLIST, 0,
		p_accessor,		&effectPBAccessor,
		end,

	pb_dxStdMat,	_T("dxStdMat"), TYPE_BOOL,	0,	IDS_DX_STDMAT,
		p_default,		FALSE,
		p_accessor,		&effectPBAccessor,
		end,

	pb_saveFX2,		_T("SaveFXFile"), TYPE_FILENAME, 0, IDS_SAVE_FX,
		p_ui,		TYPE_FILESAVEBUTTON,IDC_SAVE_FX,
		p_file_types,	IDS_FILE_TYPES,
		p_caption,		IDS_TITLE,
		p_accessor,		&effectPBAccessor,
		end,

	end
);


static FPInterfaceDesc viewport_manager_interface(
    VIEWPORT_SHADER_MANAGER_INTERFACE, _T("viewportmanager"), 0, &theViewportLoaderClassDesc, 0,

		ViewportLoader::get_num_effects2,		_T("getNumViewportEffects"),		0, TYPE_INT, 0, 0,
		ViewportLoader::get_active_effect2,		_T("getActiveViewportEffect"),		0, TYPE_REFTARG, 0, 0,
		ViewportLoader::set_effect2,				_T("setViewportEffect"),			0, TYPE_REFTARG,0,1,
			_T("effectindex"),	0,	TYPE_INT,
		ViewportLoader::get_effect_name2,		_T("getViewportEffectName"),		0,TYPE_STRING,0,1,
			_T("effectindex"), 0, TYPE_INT,
		ViewportLoader::activate_effect2,		_T("activateEffect"),				0,0,0,2,
			_T("material"), 0, TYPE_MTL,
			_T("state"),0,TYPE_BOOL,
		ViewportLoader::is_dxStdMat_enabled2, _T("isDxStdMatEnabled"),			0, TYPE_bool,0,0,
		ViewportLoader::set_dxStdMat2,		  _T("activateDxStdMat"),			0,TYPE_VOID,0,1,
			_T("active"), 0, TYPE_bool,	 
		ViewportLoader::saveFXfile2, _T("saveFXFile"),		0, TYPE_bool,0,1,
			_T("fileName"), 0 , TYPE_FILENAME,
		ViewportLoader::get_EffectName2,		_T("getActiveEffectName"),		0, TYPE_STRING, 0, 0,
		
	end
);


//  Get Descriptor method for Mixin Interface
//  *****************************************
FPInterfaceDesc* IViewportShaderManager::GetDesc()
{
     return &viewport_manager_interface;
}


ViewportLoader::ViewportLoader()
{
	pblock = NULL;
	effect = NULL;
	theViewportLoaderClassDesc.MakeAutoParamBlocks(this);
	LoadEffectsList();
	masterDlg = NULL;
	clientDlg = NULL;
	effectIndex = 0;
	mEdit =  NULL;
	mparam = NULL;
	oldEffect = NULL;
	undo = false;
}

ViewportLoader::~ViewportLoader()
{
}

ReferenceTarget *ViewportLoader::Clone(RemapDir &remap)
{
	ViewportLoader *pnew = new ViewportLoader;
	pnew->ReplaceReference(0,remap.CloneRef(pblock));
	if(effect)
		pnew->ReplaceReference(1,remap.CloneRef(effect));
	pnew->effectIndex = effectIndex;
	BaseClone(this, pnew, remap);
	return pnew;
}


ParamDlg *ViewportLoader::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp)
{

	mEdit = hwMtlEdit;
	mparam = imp;;
	masterDlg =  theViewportLoaderClassDesc.CreateParamDlgs(hwMtlEdit, imp, this);
	//TODO: suspend macro recorder to prevent: meditMaterials[3].DirectX_Manager.enabled = off
//	SuspendAll suspendAll (FALSE, TRUE); 
	param_blk.SetUserDlgProc(new EffectsDlgProc(this));

	if(effect){
		IDXDataBridge * vp = (IDXDataBridge*)effect->GetInterface(VIEWPORT_SHADER_CLIENT_INTERFACE);
		if(vp)
		{
			clientDlg = vp->CreateEffectDlg(hwMtlEdit,imp);
			masterDlg->AddDlg(clientDlg);
		}
	}

	theViewportLoaderClassDesc.RestoreRolloutState();

	return masterDlg;
}

void ViewportLoader::SetReference(int i, RefTargetHandle rtarg) 
{
	switch(i)
	{
		case PBLOCK_REF: 
			pblock = (IParamBlock2 *)rtarg;
			break;
		case 1:
			effect = (ReferenceTarget *)rtarg;
			break;
	}
}

RefTargetHandle ViewportLoader::GetReference(int i)
{
	switch(i)
	{
		case PBLOCK_REF: 
			return pblock;
		case 1:
			return effect;

		default: return NULL;
	}
}	

RefResult ViewportLoader::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,PartID &partID, RefMessage message)
{
	return REF_SUCCEED;
}


int ViewportLoader::NumSubs()  
{
	return 1;
}

Animatable* ViewportLoader::SubAnim(int i)
{
	if(effect)
	{
		IParamBlock2 * pblock2 =  effect->GetParamBlock(0);
		return pblock2; 
	}

	return NULL;
}

TSTR ViewportLoader::SubAnimName(int i)
{
	// we use the data from the Actual shader
	if(effect)
	{
		IDXDataBridge * vp = (IDXDataBridge*)effect->GetInterface(VIEWPORT_SHADER_CLIENT_INTERFACE);
		TCHAR * name = vp->GetName();
		return TSTR(name);

	}
	else
		return TSTR(_T(""));
}

SvGraphNodeReference ViewportLoader::SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags) 
{ 
	// Only continue traversal if an effect is present and active
	if( effect && GetDXShaderManager()->IsVisible())
		return SvStdTraverseAnimGraph(gom, owner, id, flags); 
	else
		return SvGraphNodeReference();
}

#define MANAGER_ACTIVENUM_CHUNK	0x1000
#define MANAGER_ENABLED_CHUNK	0x1001

IOResult ViewportLoader::Save(ISave *isave)
{
	ULONG nb;
	isave->BeginChunk(MANAGER_ACTIVENUM_CHUNK);
	isave->Write(&effectIndex, sizeof(effectIndex), &nb);			
	isave->EndChunk();
	return IO_OK;
}


// this is used in the case that the file is being opened on a system with more effects
// it will try to find the effect from the new list, otherwise it will force a "None" and no UI
class PatchEffect : public PostLoadCallback 
{
public:
	ViewportLoader*	v;
	PatchEffect(ViewportLoader* pv){ v = pv;}
	void proc(ILoad *iload)
	{
		v->effectIndex = v->FindEffectIndex(v->effect);
		v->pblock->SetValue(pb_effect,0,v->effectIndex);
		delete this;

	}
};


IOResult ViewportLoader::Load(ILoad *iload)
{
	ULONG nb;
	IOResult res;
	int id;
	PatchEffect* pe = new PatchEffect(this);
	iload->RegisterPostLoadCallback(pe);
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {

			case MANAGER_ACTIVENUM_CHUNK:
				res = iload->Read(&effectIndex, sizeof(effectIndex), &nb);
				break;

		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
	return IO_OK;
}




// compare function for sorting Shader Tab<>
static int classDescListCompare(const void *elem1, const void *elem2) 
{
	ClassDesc* s1 = *(ClassDesc**)elem1;
	ClassDesc* s2 = *(ClassDesc**)elem2;
	TSTR sn1 = s1->ClassName();  // need to snap name string, since both use GetString()
	TSTR sn2 = s2->ClassName();
	return _tcscmp(sn1.data(), sn2.data());
}



ClassDesc * ViewportLoader::FindandLoadDeferedEffect(ClassDesc * defered)
{
	SubClassList* scList = GetCOREInterface()->GetDllDir().ClassDir().GetClassList(REF_TARGET_CLASS_ID);
	for (long i = 0, j = 0; i < scList->Count(ACC_ALL); ++i) {
		if ( (*scList)[ i ].IsPublic() && ((*scList)[ i ].ClassID() == defered->ClassID()) ) {

			ClassDesc* pClassD = (*scList)[ i ].FullCD();
			LoadEffectsList();
			return pClassD;
		}
	}
	return NULL;

}

#define D3D9_GRAPHICS_WINDOW_INTERFACE_ID Interface_ID(0x56424386, 0x2151b83)

void ViewportLoader::LoadEffectsList()
{
	// loads static shader list with name-sorted Shader ClassDesc*'s
	bool bdx9 = false;

	ViewExp *vpt = NULL;
	GraphicsWindow *gw = NULL;

	vpt = GetCOREInterface()->GetActiveViewport();	

	if(vpt)
	{
		gw = vpt->getGW();
		if(gw && gw->GetInterface(D3D9_GRAPHICS_WINDOW_INTERFACE_ID))
	{
		bdx9 = true;
	}
		GetCOREInterface()->ReleaseViewport(vpt);
	}

	effectsList.ZeroCount();
	SubClassList* scList = GetCOREInterface()->GetDllDir().ClassDir().GetClassList(REF_TARGET_CLASS_ID);
	theHold.Suspend(); // LAM - 3/24/03 - defect 446356 - doing a DeleteThis on created effects, need to make sure hold is off
	for (long i = 0, j = 0; i < scList->Count(ACC_ALL); ++i) {
		if ( (*scList)[ i ].IsPublic() ) {
			ClassDesc* pClassD = (*scList)[ i ].FullCD();
			const TCHAR *cat = pClassD->Category();
			TCHAR *defcat = GetString(IDS_DX_VIEWPORT_EFFECT);
			if ((cat) && (_tcscmp(cat,defcat) == 0)) 
			{
			
				ReferenceTarget * effect = (ReferenceTarget *)pClassD->Create(TRUE);
				if(effect)
				{

					IDX9DataBridge * vp = (IDX9DataBridge*)effect->GetInterface(VIEWPORT_SHADER9_CLIENT_INTERFACE);
					if( vp)
					{
						if(bdx9)
						{
							
							if(vp->GetDXVersion() >=9.0f || vp->GetDXVersion() == 1.0f)
							{
								effectsList.Append(1, &pClassD);
							}
						}
						else
						{
							if(vp->GetDXVersion() < 9.0f)
							{
								effectsList.Append(1, &pClassD);
							}

						}
					}
					else
					{
						IDXDataBridge * vp = (IDXDataBridge*)effect->GetInterface(VIEWPORT_SHADER_CLIENT_INTERFACE);
						if(vp && !bdx9)
						{
							effectsList.Append(1, &pClassD);
						}
					}

					effect->MaybeAutoDelete();
				}
			}

		}
	}
	theHold.Resume();
	effectsList.Sort(&classDescListCompare);
}

int ViewportLoader::NumShaders()
{
	if (effectsList.Count() == 0)
		LoadEffectsList();
	return effectsList.Count();
}

ClassDesc* ViewportLoader::GetEffectCD(int i)
{
	if (effectsList.Count() == 0)
		LoadEffectsList();
	return (i >= 0 && i < effectsList.Count()) ? effectsList[i] : NULL;
}

int ViewportLoader::FindEffectIndex(ReferenceTarget * e)
{
	if(e==NULL)
		return 0;
	for(int i=0;i<effectsList.Count();i++)
	{
		if(e->ClassID()==effectsList[i]->ClassID())
			return i+1;		//take into account "None" at 0
	}
	return 0;	//none found put up the "None"
}


void ViewportLoader::LoadEffect(ClassDesc * pd)
{

	ReferenceTarget * newEffect;
	ReferenceTarget * oldEffect;

	if (theHold.Holding())
	{
		oldEffect = effect;

//		theHold.Suspend(); 


		if(pd == NULL){
			newEffect = NULL;
		}
		else{
			newEffect = (ReferenceTarget *)pd->Create(FALSE);
			if(!newEffect)
			{
				// maybe defered
				ClassDesc * def = FindandLoadDeferedEffect(pd);
				if(def)
				{
					newEffect = (ReferenceTarget *)def->Create(FALSE);
				}

			}

		}

//		theHold.Resume(); 

		if (theHold.Holding())
			theHold.Put(new AddEffectRestore(this,oldEffect,newEffect));

		SwapEffect(newEffect);
	}
}


bool IsLoaderActiveInMedit(CustAttrib * loader)
{
	IMtlEditInterface *mtlEdit = (IMtlEditInterface *)GetCOREInterface(MTLEDIT_INTERFACE);
	MtlBase * mtl = mtlEdit->GetCurMtl();

	ICustAttribContainer * cc = mtl->GetCustAttribContainer();
	if(cc)
	{

		for(int i = 0; i< cc->GetNumCustAttribs();i++)
		{
			if(loader == cc->GetCustAttrib(i))
				return true;
		}
	}
	return false;
}


void ViewportLoader::SwapEffect(ReferenceTarget *e)
{
	ReplaceReference(1,e);

//	theHold.Suspend(); 


	if(IsLoaderActiveInMedit(this))
	{
	
	if(clientDlg)
		clientDlg->DeleteThis();

	if(masterDlg)
	{
		masterDlg->DeleteDlg(clientDlg);
		masterDlg->ReloadDialog();
		clientDlg = NULL;
	}


	if(mEdit && mparam && effect)
	{
		IDXDataBridge * vp = (IDXDataBridge*)effect->GetInterface(VIEWPORT_SHADER_CLIENT_INTERFACE);
		clientDlg = vp->CreateEffectDlg(mEdit,mparam);
		masterDlg->AddDlg(clientDlg);
		
	}
//	theHold.Resume(); 
	}
	else
		clientDlg = NULL;		//need this as something changed us without being active.
	
	NotifyDependents(FOREVER,PART_ALL,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	undo = false;

}

/////////////////////////// IViewportShaderManager Function Publishing Methods/////////////////////////////////////

int ViewportLoader::GetNumEffects()
{
	return NumShaders();
}

//We only return the effect if we are active..
ReferenceTarget* ViewportLoader::GetActiveEffect()
{
	return effect;	//return our active effect.
}


TCHAR * ViewportLoader::GetEffectName(int i)
{

	ClassDesc* pClassD = GetEffectCD(i-1);
	return (TCHAR*)pClassD->ClassName();
}

ReferenceTarget * ViewportLoader::SetViewportEffect(int i)
{
	theHold.Begin();
	pblock->SetValue(pb_effect,0,i);	//"None" is at position 0;
	theHold.Accept(GetString(IDS_STR_UNDO));

	if(effect)
		return effect;
	else
		return NULL;
}

void ViewportLoader::ActivateEffect(MtlBase * mtl, BOOL state)
{

	ICustAttribContainer* cc = mtl->GetCustAttribContainer();
	if(!cc)
		return;
	MtlBase * mb = (MtlBase *) cc->GetOwner();
	if(!mb)
		return;

	if(state && effect)
		mb->SetMtlFlag(MTL_HW_MAT_ENABLED);
	else
		mb->ClearMtlFlag(MTL_HW_MAT_ENABLED);
	
	GetCOREInterface()->ForceCompleteRedraw();

}

bool ViewportLoader::IsDxStdMtlEnabled()
{
	bool active = false;
	if(GetDevice()==NULL)
		return active;

	if(pblock)
	{
		BOOL enabled;
		pblock->GetValue(pb_dxStdMat,0,enabled,FOREVER);
		if(enabled)
			active = true;
	}
	return active;
}

void ViewportLoader::SetDxStdMtlEnabled(bool state)
{
	if(pblock)
		pblock->SetValue(pb_dxStdMat,0,(BOOL)state);
}

TCHAR * ViewportLoader::GetActiveEffectName()
{
	ClassDesc* pCD = GetEffectCD(effectIndex-1);
	if(pCD)
		return (TCHAR*)pCD->ClassName();
	else
		return NULL;

}

bool ViewportLoader::SaveFXFile(TCHAR * fileName)
{
	if(!pblock)
		return false;

	pblock->SetValue(pb_saveFX2,0,fileName);
	return true;
}

BaseInterface* ViewportLoader::GetInterface(Interface_ID id) 
{ 
	if (id == VIEWPORT_SHADER_MANAGER_INTERFACE) 
		return (IViewportShaderManager*)this; 
	else if(id == VIEWPORT_SHADER_MANAGER_INTERFACE2)
		return (IViewportShaderManager2*)this;
	else if(id == VIEWPORT_SHADER_MANAGER_INTERFACE3)
		return (IViewportShaderManager3*)this;
	else
		return FPMixinInterface::GetInterface(id);
}

bool ViewportLoader::IsCurrentEffectEnabled()
{
	BOOL l_pluginEnabled = FALSE;
	this->pblock->GetValue(pb_enabled, 0, l_pluginEnabled, FOREVER);

	return l_pluginEnabled != FALSE;
}

bool ViewportLoader::SetCurrentEffectEnabled(bool enabled)
{
	if (NULL != this->pblock)
	{
		this->pblock->SetValue(pb_enabled, 0, (BOOL)enabled);
		return IsCurrentEffectEnabled() == enabled;
	}
	return false;
}

bool ViewportLoader::IsDxStdMtlSupported()
{
	MtlBase* l_pMaterial = this->FindOwnerMaterial();

	return 
		// should be standard material
		( NULL != l_pMaterial ) && 
		( Class_ID(DMTL_CLASS_ID, 0) == l_pMaterial->ClassID() ) && 
		// DirectX device should have been initialized successfully.
		( NULL != ::GetDevice() ) && 
		// if plugin effect have been turned on, user can not change the shading
		// model of the material. He should disable plugin effect first.
		( !IsCurrentEffectEnabled() );
}

bool ViewportLoader::IsSaveFxSupported()
{
	MtlBase* l_pMaterial = this->FindOwnerMaterial();
	// should be standard material
	if (( NULL != l_pMaterial ) && 
		( Class_ID(DMTL_CLASS_ID, 0) == l_pMaterial->ClassID() ) && 
		// DirectX device should have been initialized successfully.
		( NULL != ::GetDevice() )
		)
	{
		// if it is directX shading, return true.
		return this->IsDxStdMtlEnabled();
	}
	else
	{
		return false;
	}
}

bool ViewportLoader::IsCurrentEffectSupported()
{
	return 
		// should be a valid effect
		( NULL != this->GetEffectCD(this->effectIndex - 1) ) && 
		( NULL != this->effect ) && 
		// DirectX device should have been initialized successfully.
		( NULL != ::GetDevice() );
}

MtlBase* ViewportLoader::FindOwnerMaterial()
{
	ICustAttribContainer* l_pAttribContainer = NULL;
	l_pAttribContainer = ::GetOwnerContainer(const_cast<ViewportLoader*>(this));

	if (NULL != l_pAttribContainer)
	{
		return (MtlBase *)(l_pAttribContainer->GetOwner());
	}
	else
	{
		return NULL;
	}
}

void ViewportLoader::HandleEffectIndexChanged(TimeValue t, int effectIndex)
{
	if (effectIndex == this->effectIndex)
	{
		// nothing is needed to do.
		return;
	}

	this->effectIndex = effectIndex;
	this->LoadEffect(this->GetEffectCD(this->effectIndex - 1));

	// update the UI
	if (this->IsCurrentEffectSupported())
	{
		this->EnableDlgItem(IDC_ENABLED, TRUE);
	}
	else
	{
		this->SetCurrentEffectEnabled(false);
		this->EnableDlgItem(IDC_ENABLED, FALSE);
	}

	this->EnableDlgItem(IDC_SAVE_FX, this->IsSaveFxSupported());

	GetCOREInterface()->ForceCompleteRedraw();
}

void ViewportLoader::HandleEffectStateChanged(TimeValue t, bool enabled)
{
	if (!this->IsCurrentEffectSupported())
	{
		if (enabled)
		{
			// plugin effect is not supported, restore the value
			SetCurrentEffectEnabled(false);
		}
		return;
	}

	// update the UI
	this->EnableDlgItem(IDC_SAVE_FX, this->IsSaveFxSupported());

	MtlBase* l_pMaterial = this->FindOwnerMaterial();
	if (NULL != l_pMaterial)
	{
		if (enabled || this->IsDxStdMtlEnabled())
		{
			l_pMaterial->SetMtlFlag(MTL_HW_MAT_ENABLED);
		}
		else
		{
			l_pMaterial->ClearMtlFlag(MTL_HW_MAT_ENABLED);
		}
		// redraw views
		GetCOREInterface()->RedrawViews(t);
	}
}

void ViewportLoader::HandleDxStdMtlChanged(TimeValue t, bool enabled)
{
	if (!this->IsDxStdMtlSupported())
	{
		if (enabled)
		{
			// DirectX Shading is not supported at this moment, restore the value.
			SetDxStdMtlEnabled(false);
		}
		return;
	}

	// update the UI
	this->EnableDlgItem(IDC_SAVE_FX, this->IsSaveFxSupported());

	if(!this->effect && !enabled){
		this->EnableDlgItem(IDC_ENABLED, FALSE);
	}
	else{
		this->EnableDlgItem(IDC_ENABLED, !enabled);
	}
	this->EnableDlgItem(IDC_EFFECTLIST, !enabled);


	MtlBase* l_pMaterial = this->FindOwnerMaterial();
	if (NULL != l_pMaterial)
	{
		if (enabled)
		{
			l_pMaterial->SetMtlFlag(MTL_HW_MAT_ENABLED);
		}
		else
		{
			l_pMaterial->ClearMtlFlag(MTL_HW_MAT_ENABLED);
			GetCOREInterface()->ForceCompleteRedraw(FALSE);
		}
	}
}

void ViewportLoader::HandleSaveFxFile(TimeValue t, const TCHAR* fileName)
{
	if (!this->IsSaveFxSupported())
	{
		return;
	}
#ifndef NO_ASHLI
	MtlBase* l_pMaterial = this->FindOwnerMaterial();
	if (NULL == l_pMaterial)
	{
		return;
	}
	IEffectFile* l_pEffectFile = static_cast<IEffectFile*>(
		l_pMaterial->GetInterface(EFFECT_FILE_INTERFACE));

	if (NULL != l_pEffectFile)
	{
		l_pEffectFile->SaveEffectFile(const_cast<TCHAR*>(fileName));
	}
#endif // NO_ASHLI
}

void ViewportLoader::EnableDlgItem(INT dlgItem, BOOL enabled)
{
	IParamMap2* l_pParamMap = this->pblock->GetMap();
	if (NULL != l_pParamMap)
	{
		// instead of using IParamMap2::Enable, I used windows API here.
		// so we could use this function when the dialog is initializing.
		EnableWindow(GetDlgItem(l_pParamMap->GetHWnd(), dlgItem), enabled);
	}
}

#ifndef NO_ASHLI

class DXEffectActions : public FPStaticInterface {
protected:
	DECLARE_DESCRIPTOR(DXEffectActions)

	enum {
		kSaveEffectFile = 1,
		kDxDisplay=2,
	};

	void SaveEffectFile();
	void ActivateDXDisplay();
	BOOL IsChecked(FunctionID actionID);
	BOOL IsEnabled(FunctionID actionID);

	BEGIN_FUNCTION_MAP
		VFN_0(kSaveEffectFile, SaveEffectFile)
		VFN_0(kDxDisplay, ActivateDXDisplay)
	END_FUNCTION_MAP

	static DXEffectActions mDXEffectActions;
};

#define SAVEEFFECTFILE_ACTIONS		Interface_ID(0x26615cff, 0x26273586)

DXEffectActions DXEffectActions::mDXEffectActions(
	SAVEEFFECTFILE_ACTIONS, _T("SaveEffectFile"), IDS_DLL_CATEGORY, &theViewportLoaderClassDesc, FP_ACTIONS,
	kActionMaterialEditorContext,

	kSaveEffectFile, _T("SaveAsFXFile"), IDS_SAVEASEFFECTFILE, 0,
	f_menuText,	IDS_SAVEASEFFECTFILE,
	end,

	kDxDisplay, _T("DXDisplay"), IDS_DXDISPLAY, 0,
	f_menuText,	IDS_DXDISPLAY,
	end,


	end
	);

static bool BrowseOutFile(TSTR &file) {
	BOOL silent = TheManager->SetSilentMode(TRUE);
	BitmapInfo biOutFile;
	FilterList fl;

	Interface8 * imp = GetCOREInterface8();
	HWND   hWnd = GetCOREInterface()->GetMAXHWnd();
	TCHAR  file_name[260] = _T("");
	TSTR cap_str = GetString(IDS_TITLE);
	FilterList filterList;
	TSTR fileName, initDir;

	filterList.Append(GetString(IDS_FX_FILTER_NAME));
	filterList.Append(GetString(IDS_FX_FILTER_DESC));

	return imp->DoMaxSaveAsDialog(hWnd,cap_str,file, initDir  , filterList);
}

#define UI_MAKEBUSY			SetCursor(LoadCursor(NULL, IDC_WAIT));
#define UI_MAKEFREE			SetCursor(LoadCursor(NULL, IDC_ARROW));

void DXEffectActions::SaveEffectFile()
{
	IMtlEditInterface *mtlEdit = (IMtlEditInterface *)GetCOREInterface(MTLEDIT_INTERFACE);
	MtlBase *mtl = mtlEdit->GetCurMtl();
	if(!mtl)
		return;		

	TSTR File;
	if(!BrowseOutFile(File))
		return;
	UI_MAKEBUSY
	IEffectFile * effectFile = static_cast<IEffectFile*>(mtl->GetInterface(EFFECT_FILE_INTERFACE));
	if(effectFile)
		effectFile->SaveEffectFile(File.data());
	UI_MAKEFREE
}

void DXEffectActions::ActivateDXDisplay()
{
	IMtlEditInterface *mtlEdit = (IMtlEditInterface *)GetCOREInterface(MTLEDIT_INTERFACE);
	MtlBase *mtl = mtlEdit->GetCurMtl();
	if(!mtl)
		return;	
	MSPlugin* plugin = (MSPlugin*)((ReferenceTarget*)mtl)->GetInterface(I_MAXSCRIPTPLUGIN);
	ReferenceTarget * targ = NULL;
	if(plugin){
		targ = plugin->get_delegate();
		mtl = (MtlBase*)targ;
	}

	switch (IMaterialViewportShading::GetCurrentShadingModel(*mtl))
	{
	case IMaterialViewportShading::Standard:
		IMaterialViewportShading::SetCurrentShadingModel(
			*mtl, IMaterialViewportShading::Hardware);
		break;
	case IMaterialViewportShading::Hardware:
		IMaterialViewportShading::SetCurrentShadingModel(
			*mtl, IMaterialViewportShading::Standard);
		break;
	default:
		break;
	}

}

BOOL DXEffectActions::IsChecked(FunctionID actionID)
{
	if(actionID == kDxDisplay)
	{
		IMtlEditInterface *mtlEdit = (IMtlEditInterface *)GetCOREInterface(MTLEDIT_INTERFACE);
		MtlBase *mtl = mtlEdit->GetCurMtl();
		if(!mtl)
			return FALSE;		
		MSPlugin* plugin = (MSPlugin*)((ReferenceTarget*)mtl)->GetInterface(I_MAXSCRIPTPLUGIN);
		ReferenceTarget * targ = NULL;
		if(plugin){
			targ = plugin->get_delegate();
			mtl = (MtlBase*)targ;
		}

		return 
			IMaterialViewportShading::GetCurrentShadingModel(*mtl) == 
			IMaterialViewportShading::Hardware;
	}
	return FALSE;
}

BOOL DXEffectActions::IsEnabled(FunctionID actionID)
{
	BOOL active = TRUE;

	IMtlEditInterface *mtlEdit = (IMtlEditInterface *)GetCOREInterface(MTLEDIT_INTERFACE);
	MtlBase *mtl = mtlEdit->GetCurMtl();
	if(!mtl)
		return FALSE;

	MSPlugin* plugin = (MSPlugin*)((ReferenceTarget*)mtl)->GetInterface(I_MAXSCRIPTPLUGIN);
	ReferenceTarget * targ = NULL;
	if(plugin){
		targ = plugin->get_delegate();
		mtl = (MtlBase*)targ;
	}

	if(actionID == kDxDisplay)
	{
		return IMaterialViewportShading::IsShadingModelSupported(
			*mtl, IMaterialViewportShading::Hardware);
	}
	if(actionID == kSaveEffectFile)
	{
		IViewportShaderManager3* pViewportLoader = NULL;

		ICustAttribContainer* l_pCustAttribContainer = mtl->GetCustAttribContainer();
		if (NULL != l_pCustAttribContainer)
		{
			pViewportLoader = (IViewportShaderManager3*)
				(l_pCustAttribContainer->FindCustAttribInterface(VIEWPORT_SHADER_MANAGER_INTERFACE3));
		}

		if (NULL != pViewportLoader)
		{
			return (pViewportLoader->IsSaveFxSupported());
		}
		else
		{
			return FALSE;
		}
	}

	return active;
}
#endif
