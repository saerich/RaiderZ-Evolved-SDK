////////////////////////////////////////////////////////////////////////
//
//	Standard Render Elements 	
//
//	Created: Kells Elmquist, 24, June 2000
//
//	Copyright (c) 2000, All Rights Reserved.
//
// local includes
#include "renElemPch.h"
#include "stdRenElems.h"
#include "gbuf.h"

// maxsdk includes
#include <IShadeContextExtension8.h>
#include <hsv.h>
#include <notify.h>

#include "buildver.h"

Class_ID specularRenderElementClassID( SPECULAR_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID specularSelectRenderElementClassID( SPECULAR_SELECT_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID specularCompRenderElementClassID( SPECULAR_COMP_RENDER_ELEMENT_CLASS_ID , 0);
#ifndef NO_RENDER_ELEMENT_DIFFUSE
Class_ID diffuseRenderElementClassID( DIFFUSE_RENDER_ELEMENT_CLASS_ID , 0);
#endif	// no_render_element_diffuse
#ifndef NO_RENDER_ELEMENT_MATTE
Class_ID matteRenderElementClassID( MATTE_RENDER_ELEMENT_CLASS_ID , 0);
#endif	// no_render_element_matte
#ifndef NO_RENDER_ELEMENT_LIGHTING
Class_ID lightingRenderElementClassID( LIGHTING_RENDER_ELEMENT_CLASS_ID , 0);
#endif	// no_render_element_lighting
Class_ID emissionRenderElementClassID( EMISSION_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID reflectionRenderElementClassID( REFLECTION_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID refractionRenderElementClassID( REFRACTION_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID shadowRenderElementClassID( SHADOWS_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID atmosphereRenderElementClassID( ATMOSPHERE_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID atmosphere2RenderElementClassID( ATMOSPHERE2_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID blendRenderElementClassID( BLEND_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID ZRenderElementClassID( Z_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID alphaRenderElementClassID( ALPHA_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID clrShadowRenderElementClassID( CLR_SHADOW_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID bgndRenderElementClassID( BGND_RENDER_ELEMENT_CLASS_ID , 0);
#ifndef NO_RENDER_ELEMENT_MOTION
Class_ID motionRenderElementClassID( MOTION_RENDER_ELEMENT_CLASS_ID, 0);
#endif	// NO_RENDER_ELEMENT_MOTION
Class_ID MaterialIDRenderElementClassID( MATERIALID_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID ObjectIDRenderElementClassID( OBJECTID_RENDER_ELEMENT_CLASS_ID , 0);


////////////////////////////////////
// color utility
inline void ClampMax( Color& c ){
	if( c.r > 1.0f ) c.r = 1.0f;
	if( c.g > 1.0f ) c.g = 1.0f;
	if( c.b > 1.0f ) c.b = 1.0f;
}

inline void ClampMax( AColor& c ){
	if( c.r > 1.0f ) c.r = 1.0f;
	if( c.g > 1.0f ) c.g = 1.0f;
	if( c.b > 1.0f ) c.b = 1.0f;
	if( c.a > 1.0f ) c.a = 1.0f;
}


#ifndef NO_RENDER_ELEMENT_DIFFUSE
///////////////////////////////////////////////////////////////////////////////
//
//	Diffuse render element
//
class DiffuseRenderElement : public BaseRenderElement {
public:
		enum{
			lightingOn = BaseRenderElement::pbBitmap+1
		};
		
		DiffuseRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return diffuseRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_DIFFUSE_RENDER_ELEMENT ); }
		SFXParamDlg *CreateParamDialog(IRendParams *ip);

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);

		void SetLightApplied(BOOL on){ 
			mpParamBlk->SetValue( lightingOn, 0, on ); 
		}
		BOOL IsLightApplied() const{
			int	on;
			mpParamBlk->GetValue( lightingOn, 0, on, FOREVER );
			return on;
		}

};

// --------------------------------------------------
// Diffuse element class descriptor
// --------------------------------------------------
class DiffuseElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new DiffuseRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_DIFFUSE_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return diffuseRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("diffuseRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static DiffuseElementClassDesc diffuseCD;
#endif	// no_render_element_diffuse
ClassDesc* GetDiffuseElementDesc()
{
#ifndef NO_RENDER_ELEMENT_DIFFUSE
	return &diffuseCD;
#else	// no_render_element_diffuse
	return NULL;
#endif	// no_render_element_diffuse
}


#ifndef NO_RENDER_ELEMENT_DIFFUSE
// ------------------------------------------------------
// Diffuse parameter block description - global instance
// ------------------------------------------------------
static ParamBlockDesc2 diffuseParamBlk(PBLOCK_NUMBER, _T("Diffuse render element parameters"), 0, &diffuseCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_DIFFUSE_ELEMENT, IDS_DIFFUSE_ELEMENT_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	DiffuseRenderElement::lightingOn, _T("lightingOn"), TYPE_BOOL, 0, IDS_LIGHTING_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_LIGHTING,
		end,
	end
	);

//--- Diffuse Render Element ------------------------------------------------
DiffuseRenderElement::DiffuseRenderElement()
{
	diffuseCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}

IRenderElementParamDlg *DiffuseRenderElement::CreateParamDialog(IRendParams *ip)
{
	return (IRenderElementParamDlg *)diffuseCD.CreateParamDialogs(ip, this);
}

RefTargetHandle DiffuseRenderElement::Clone( RemapDir &remap )
{
	DiffuseRenderElement*	newEle = new DiffuseRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}


void DiffuseRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {

		if( IsLightApplied() ){
			sc.out.elementVals[ mShadeOutputIndex ] = ip.finalAttenuation * (ip.diffIllumOut+ip.ambIllumOut);
			float a = 1.0f - Intens( sc.out.t );
			if( a < 0.0f ) a = 0.0f;
			sc.out.elementVals[ mShadeOutputIndex ].a = a;

			if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
					&& sc.globContext->pToneOp->Active(sc.CurTime())) {
				sc.globContext->pToneOp->ScaledToRGB(sc.out.elementVals[ mShadeOutputIndex ]);
			}
		}
		else
		{
			// LAM - 7/11/03 - 507044 - some mtls leave stdIDToChannel NULL, use black instead 
			sc.out.elementVals[ mShadeOutputIndex ] = ip.stdIDToChannel ? ip.channels[ ip.stdIDToChannel[ ID_DI ]] : AColor(0,0,0);
			float a = 1.0f - Intens( sc.out.t );
			if( a < 0.0f ) a = 0.0f;
			sc.out.elementVals[ mShadeOutputIndex ].a = a;
		}
	}
}
#endif	// no_render_element_diffuse

#ifndef NO_RENDER_ELEMENT_MATTE
///////////////////////////////////////////////////////////////////////////////
//
//	Matte render element
//
class MatteRenderElement : public BaseRenderElement {
public:
		enum{
			mtlIDOn = BaseRenderElement::pbBitmap+1,
			gbufIDOn,
			mtlID,
			gbufID,
			IncludeOn
		};
		enum {
		   BASE_CHUNK = 0x4523,
		   MATTE_LE_CHUNK = 0x4524
		};
		
		MatteRenderElement();
		RefTargetHandle Clone( RemapDir &remap );
		ExclList			exclList;


		Class_ID ClassID() {return matteRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_MATTE_RENDER_ELEMENT ); }
		SFXParamDlg *CreateParamDialog(IRendParams *ip);

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);

		BOOL IsIncludeApplied() const{
			int	on;
			mpParamBlk->GetValue( IncludeOn, 0, on, FOREVER );
			return on;
		}
		BOOL IsMtlIDFilterApplied() const{
			int	on;
			mpParamBlk->GetValue( mtlIDOn, 0, on, FOREVER );
			return on;
		}
		BOOL IsGBufIDFilterApplied() const{
			int	on;
			mpParamBlk->GetValue( gbufIDOn, 0, on, FOREVER );
			return on;
		}
		int GBufID() const{
			int	id;
			mpParamBlk->GetValue( gbufID, 0, id, FOREVER );
			return id;
		}
		int MtlID() const{
			int	id;
			mpParamBlk->GetValue( mtlID, 0, id, FOREVER );
			return id;
		}


		IOResult Load(ILoad *iload)
		{
		   IOResult res;

		   res = BaseRenderElement::Load(iload);

		   while (IO_OK==(res=iload->OpenChunk())) {
			  switch(iload->CurChunkID())  {
				 case MATTE_LE_CHUNK: {
					res = exclList.Load(iload);;
				 } break;
			  }
			  iload->CloseChunk();
			  if (res!=IO_OK) 
				 return res;
		   }
		   return IO_OK;
		}



		IOResult Save(ISave *isave)
		{
		   IOResult res;

		   //Calling the parents save function first to make sure "render element merging" will work
		   res = BaseRenderElement::Save(isave);

		   if (res != IO_OK) {
			  return res;
		   }

		   isave->BeginChunk(MATTE_LE_CHUNK);
		   res = exclList.Save(isave);
		   isave->EndChunk();

		   return res;
		}

		// -- from Animatable
		virtual void* GetInterface(ULONG id);

};

// --------------------------------------------------
// Matte element class descriptor
// --------------------------------------------------
class MatteElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new MatteRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_MATTE_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return matteRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("matteRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static MatteElementClassDesc matteCD;
#endif	// no_render_element_matte
ClassDesc* GetMatteElementDesc() {
#ifndef NO_RENDER_ELEMENT_MATTE
	return &matteCD;
#else	// no_render_element_matte
	return NULL;
#endif	// no_render_element_matte
}


#ifndef NO_RENDER_ELEMENT_MATTE
//==============================================================================
// class MainPBDialogProc
//
// Custom dialog procedure for the main param block rollout. Necessary because
// of the combo boxes in the UI, no supported by PB2
//==============================================================================
class MainPBDialogProc : public ParamMap2UserDlgProc {

public:

    static MainPBDialogProc* GetInstance() { return &m_theInstance; }
    // notification callback for lighting unit system changes

    MainPBDialogProc();
    ~MainPBDialogProc();

    // -- from ParamMap2UserDlgProc
    virtual INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    virtual void SetThing(ReferenceTarget *m) { matterender = static_cast<MatteRenderElement*>(m); }
    void DeleteThis() { matterender = NULL; }
    virtual void Update(TimeValue t, Interval& valid, IParamMap2* pmap);
    
private:

    static MainPBDialogProc m_theInstance;

    MatteRenderElement* matterender;
};

MainPBDialogProc MainPBDialogProc::m_theInstance;


// ------------------------------------------------------
// Matte parameter block description - global instance
// ------------------------------------------------------
static ParamBlockDesc2 matteParamBlk(PBLOCK_NUMBER, _T("Matte render element parameters"), 0, &matteCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_MATTE_ELEMENT, IDS_MATTE_ELEMENT_PARAMS, 0, 0, MainPBDialogProc::GetInstance(),
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	MatteRenderElement::mtlIDOn, _T("mtlIDOn"), TYPE_BOOL, 0, IDS_MTLID_ON,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_MTL_ID,
		end,
	MatteRenderElement::gbufIDOn, _T("gbufIDOn"), TYPE_BOOL, 0, IDS_GBUFID_ON,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_GBUF_ID,
		end,
	MatteRenderElement::gbufID, _T("gbufID"), TYPE_INT, 0, IDS_GBUFID,
		p_default,		1,
		p_range, 		0, 65535, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_INT, IDC_GBUF_EDIT, IDC_GBUF_SPIN, SPIN_AUTOSCALE, 
		end, 
	MatteRenderElement::mtlID, _T("mtlID"), TYPE_INT, 0, IDS_MTLID,
		p_default,		1,
		p_range, 		0, 65535, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_INT, IDC_MTLID_EDIT, IDC_MTLID_SPIN, SPIN_AUTOSCALE, 
		end, 
	MatteRenderElement::IncludeOn, _T("includeOn"), TYPE_BOOL, 0, IDS_INCLUDE_ON,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_INCLUDE_CHECK,
		end,
	end
);
//--- Matte Render Element ------------------------------------------------
MatteRenderElement::MatteRenderElement()
{
	matteCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
	exclList.SetFlag(NT_INCLUDE);
}

IRenderElementParamDlg *MatteRenderElement::CreateParamDialog(IRendParams *ip)
{
	MainPBDialogProc::GetInstance()->SetThing(this);
	return (IRenderElementParamDlg *)matteCD.CreateParamDialogs(ip, this);
}

RefTargetHandle MatteRenderElement::Clone( RemapDir &remap )
{
	MatteRenderElement*	newEle = new MatteRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}


void MatteRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd

		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,1);


		if(IsIncludeApplied())
		{
			if (!exclList.TestFlag(NT_INCLUDE)) 
			{ // exclude list
			  //lets have the backrgoungas white
				sc.out.elementVals[ mShadeOutputIndex ] = AColor(1,1,1,1);
			}

		}

	} else {
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,1);
		if(IsMtlIDFilterApplied() && sc.out.gbufId==MtlID())
			sc.out.elementVals[ mShadeOutputIndex ] = AColor(1,1,1,1);
		int id = sc.NodeID();
		RenderInstance *inst = sc.globContext->GetRenderInstance(id);
		if(IsGBufIDFilterApplied() && inst && inst->GetINode()->GetGBufID()==GBufID())
			sc.out.elementVals[ mShadeOutputIndex ] = AColor(1,1,1,1);

		if(IsIncludeApplied())
		{
			if (exclList.TestFlag(NT_INCLUDE) && inst) 
			{ // include list
				if (exclList.FindNode(inst->GetINode()) >= 0)
					sc.out.elementVals[ mShadeOutputIndex ] = AColor(1,1,1,1);
			}
			else if(!exclList.TestFlag(NT_INCLUDE) && inst)
			{
				sc.out.elementVals[ mShadeOutputIndex ] = AColor(1,1,1,1);
				if (exclList.FindNode(inst->GetINode()) >= 0)
					sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,1);
			}

		}

	}
}

void* MatteRenderElement::GetInterface(ULONG id) {
	
	if(id == 0x5be67b75) {
		// [dl | 14may2003]
		// Return the exclusion list. This is used by the mental ray translator
		// to access to exclusion list.
		return &exclList;
	}
	else {
		return BaseRenderElement::GetInterface(id);
	}
}

MainPBDialogProc::MainPBDialogProc() : matterender(NULL) {
}


MainPBDialogProc::~MainPBDialogProc() {
}


INT_PTR MainPBDialogProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch(msg) {
    case WM_COMMAND:
        switch(LOWORD(wParam)) {
			case IDC_INCLUDE_BUTTON:
				{
					int a=0;
					GetCOREInterface()->DoExclusionListDialog(&matterender->exclList, FALSE);
					if(matterender->exclList.TestFlag(NT_INCLUDE))
					{
						SetWindowText(GetDlgItem(hWnd,IDC_INCLUDE_BUTTON), GetString(IDS_INCLUDE));
					}
					else
					{
						SetWindowText(GetDlgItem(hWnd,IDC_INCLUDE_BUTTON), GetString(IDS_EXCLUDE));
					}

					return true;
				}            
				break;
        default:
            return false;
        }
        break;
    default:
        return false;
    }
    return false;
}


void MainPBDialogProc::Update(TimeValue t, Interval& valid, IParamMap2* pmap) {

    HWND hDlg = pmap->GetHWnd();
    IParamBlock2* pBlock = pmap->GetParamBlock();
    TSTR unitString;
	if(matterender->exclList.TestFlag(NT_INCLUDE))
	{
		SetWindowText(GetDlgItem(hDlg,IDC_INCLUDE_BUTTON), GetString(IDS_INCLUDE));
	}
	else
	{
		SetWindowText(GetDlgItem(hDlg,IDC_INCLUDE_BUTTON), GetString(IDS_EXCLUDE));
	}
}
#endif	// no_render_element_matte


#ifndef NO_RENDER_ELEMENT_LIGHTING
///////////////////////////////////////////////////////////////////////////////
//
//	Light render element
//
class LightingRenderElement : public BaseRenderElement {
public:
		enum{
			directOn = BaseRenderElement::pbBitmap+1, 
			indirectOn,
			shadowsOn
		};
		
		LightingRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return lightingRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_LIGHTING_RENDER_ELEMENT ); }
		SFXParamDlg *CreateParamDialog(IRendParams *ip);

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);

		BOOL IsLightApplied() const{ return TRUE; }

		BOOL ShadowsApplied() const{ return IsShadowApplied(); }

		// set/get element's shadow applied state
		void SetShadowApplied(BOOL on){
			mpParamBlk->SetValue( shadowsOn, 0, on ); 
		}
		BOOL IsShadowApplied() const{
			int	on;
			mpParamBlk->GetValue( shadowsOn, 0, on, FOREVER );
			return on;
		}

		// set/get element's direct light applied state
		void SetDirectLightOn(BOOL on){
			mpParamBlk->SetValue( directOn, 0, on ); 
		}
		BOOL IsDirectLightOn() const{
			int	on;
			mpParamBlk->GetValue( directOn, 0, on, FOREVER );
			return on;
		}

		// set/get element's indirect light applied state
		void SetIndirectLightOn(BOOL on){
			mpParamBlk->SetValue( indirectOn, 0, on ); 
		}
		BOOL IsIndirectLightOn() const{
			int	on;
			mpParamBlk->GetValue( indirectOn, 0, on, FOREVER );
			return on;
		}

};

// --------------------------------------------------
// Light element class descriptor
// --------------------------------------------------
class LightingElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new LightingRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_LIGHTING_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return lightingRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("LightingRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static LightingElementClassDesc lightingCD;
#endif	// no_render_element_lighting
ClassDesc* GetLightingElementDesc()
{
#ifndef NO_RENDER_ELEMENT_LIGHTING
	return &lightingCD;
#else	// no_render_element_lighting
	return NULL;
#endif	// no_render_element_lighting
}


#ifndef NO_RENDER_ELEMENT_LIGHTING
// ------------------------------------------------------
// Light parameter block description - global instance
// ------------------------------------------------------
static ParamBlockDesc2 lightingParamBlk(PBLOCK_NUMBER, _T("Lighting render element parameters"), 0, &lightingCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_LIGHT_ELEMENT, IDS_LIGHTING_ELEMENT_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	LightingRenderElement::shadowsOn, _T("shadowsOn"), TYPE_BOOL, 0, IDS_SHADOWS_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SHADOWS_ON,
		p_default, TRUE,
		end,
	LightingRenderElement::directOn, _T("directOn"), TYPE_BOOL, 0, IDS_DIRECT_ILLUM_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_DIRECT_ON,
		p_default, TRUE,
		end,
	LightingRenderElement::indirectOn, _T("indirectOn"), TYPE_BOOL, 0, IDS_INDIRECT_ILLUM_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_INDIRECT_ON,
		p_default, TRUE,
		end,	end
	);

//--- Light Render Element ------------------------------------------------
LightingRenderElement::LightingRenderElement()
{
	lightingCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}

IRenderElementParamDlg *LightingRenderElement::CreateParamDialog(IRendParams *ip)
{
	return (IRenderElementParamDlg *)lightingCD.CreateParamDialogs(ip, this);
}

RefTargetHandle LightingRenderElement::Clone( RemapDir &remap )
{
	LightingRenderElement*	newEle = new LightingRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}


void LightingRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
		BOOL shadowSave = sc.shadow;
		sc.shadow = IsShadowApplied() ? TRUE : FALSE;

		sc.out.elementVals[ mShadeOutputIndex ] 
			= computeIlluminance( sc, IsIndirectLightOn(), IsDirectLightOn() );

		float a = 1.0f - Intens( sc.out.t );
		if( a < 0.0f ) a = 0.0f;
		sc.out.elementVals[ mShadeOutputIndex ].a = a;


		if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
				&& sc.globContext->pToneOp->Active(sc.CurTime())) {
			sc.globContext->pToneOp->ScaledToRGB(sc.out.elementVals[ mShadeOutputIndex ]);
		}

		sc.shadow = shadowSave; // restore shadow
	}
}
#endif	// no_render_element_lighting


//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
//
//	Specular render element
//
class SpecularRenderElement : public BaseRenderElement {
public:
		SpecularRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return specularRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_SPECULAR_RENDER_ELEMENT ); }

		BOOL AtmosphereApplied() const{ return TRUE; } //override, always on

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
		void SpecularRenderElement::PostAtmosphere(ShadeContext& sc, float z, float zPrev);
};

// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class SpecularElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new SpecularRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_SPECULAR_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return specularRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("specularRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static SpecularElementClassDesc specularCD;
ClassDesc* GetSpecularElementDesc() { return &specularCD; }


// ---------------------------------------------
// Specular parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 specularParamBlk(PBLOCK_NUMBER, _T("Specular render element parameters"), 0, &specularCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	end
	);

//--- Specular Render Element ------------------------------------------------
SpecularRenderElement::SpecularRenderElement()
{
	specularCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle SpecularRenderElement::Clone( RemapDir &remap )
{
	SpecularRenderElement*	newEle = new SpecularRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}


void SpecularRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
		Color c = ip.specIllumOut;

		if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
				&& sc.globContext->pToneOp->Active(sc.CurTime())) {
			sc.globContext->pToneOp->ScaledToRGB(c);
		}

		ClampMax( c );
		sc.out.elementVals[ mShadeOutputIndex ] = c;
//		sc.out.elementVals[ mShadeOutputIndex ].a = Intens( ip.specIllumOut );
		sc.out.elementVals[ mShadeOutputIndex ].a = 0;
	}
}

void SpecularRenderElement::PostAtmosphere(ShadeContext& sc, float z, float zPrev)
{
	// additive xparency can wipe out the a, so fill it in
	sc.out.elementVals[ mShadeOutputIndex ].a = Intens( sc.out.elementVals[ mShadeOutputIndex ] );
}

//-----------------------------------------------------------------------------

/*************
///////////////////////////////////////////////////////////////////////////////
//
//	Composited Specular render element
//
class SpecularCompRenderElement : public BaseRenderElement {
public:
		SpecularCompRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return specularCompRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_SPECULAR_COMP_RENDER_ELEMENT ); }

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
};

// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class SpecularCompElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new SpecularCompRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_SPECULAR_COMP_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return specularCompRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("specularCompRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static SpecularCompElementClassDesc specularCompCD;
ClassDesc* GetSpecularCompElementDesc() { return &specularCompCD; }


// ---------------------------------------------
// SpecularComp parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 specularCompParamBlk(PBLOCK_NUMBER, _T("SpecularComp render element parameters"), 0, &specularCompCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	end
	);

//--- Specular Comp Render Element ------------------------------------------------
SpecularCompRenderElement::SpecularCompRenderElement()
{
	specularCompCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle SpecularCompRenderElement::Clone( RemapDir &remap )
{
	SpecularCompRenderElement*	newEle = new SpecularCompRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}


void SpecularCompRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
		sc.out.elementVals[ mShadeOutputIndex ] = ip.specIllumOut;
		sc.out.elementVals[ mShadeOutputIndex ].a = Intens( ip.specIllumOut );
	}
}


//-----------------------------------------------------------------------------
***********/

///////////////////////////////////////////////////////////////////////////////
//
//	Emission render element
//
class EmissionRenderElement : public BaseRenderElement {
public:
		EmissionRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return emissionRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_EMISSION_RENDER_ELEMENT ); }

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
};

// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class EmissionElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new EmissionRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_EMISSION_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return emissionRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("emissionRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static EmissionElementClassDesc emissionCD;
ClassDesc* GetEmissionElementDesc() { return &emissionCD; }


// ---------------------------------------------
// Emission parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 emissionParamBlk(PBLOCK_NUMBER, _T("Emission render element parameters"), 0, &emissionCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	end
	);

//--- Emission Render Element ------------------------------------------------
EmissionRenderElement::EmissionRenderElement()
{
	emissionCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle EmissionRenderElement::Clone( RemapDir &remap )
{
	EmissionRenderElement*	newEle = new EmissionRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}


void EmissionRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
 		Color c = ip.selfIllumOut;
		ClampMax( c );

		if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
				&& sc.globContext->pToneOp->Active(sc.CurTime())) {
			sc.globContext->pToneOp->ScaledToRGB(c);
		}

		sc.out.elementVals[ mShadeOutputIndex ] = ip.finalAttenuation * c;
		float a = 1.0f - Intens( sc.out.t );
		if( a < 0.0f ) a = 0.0f;
		sc.out.elementVals[ mShadeOutputIndex ].a = a;
	}
}


//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
//
//	Background render element
//
class BgndRenderElement : public BaseRenderElement {
public:
		BgndRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return bgndRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_BGND_RENDER_ELEMENT ); }

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
};

// --------------------------------------------------
// Bgnd element class descriptor
// --------------------------------------------------
class BgndElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new BgndRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_BGND_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return bgndRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("bgndRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static BgndElementClassDesc bgndCD;
ClassDesc* GetBgndElementDesc() { return &bgndCD; }


// ------------------------------------------------------
// Bgnd parameter block description - global instance
// ------------------------------------------------------
static ParamBlockDesc2 bgndParamBlk(PBLOCK_NUMBER, _T("Bgnd render element parameters"), 0, &bgndCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, FALSE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	end
	);

//--- Bgnd Render Element ------------------------------------------------
BgndRenderElement::BgndRenderElement()
{
	bgndCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
	SetFilterEnabled( FALSE );
}


RefTargetHandle BgndRenderElement::Clone( RemapDir &remap )
{
	BgndRenderElement*	newEle = new BgndRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}


void BgndRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	float a;
	Color bgClr, bgTrans;
	sc.GetBGColor( bgClr, bgTrans, FALSE);
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = bgClr;
	} else {
// >>>>>>>>> why is this dimmed??? bad for matte
		// dimmed for blending w/ transparent objects!
		a = 1.0f - Intens( sc.out.t );
		if( (ip.hasComponents & HAS_MATTE_MTL) )
			a = 1.0f;	// matte bgnd case
		if( a < 0.0f ) 
			a = 0.0f;
		sc.out.elementVals[ mShadeOutputIndex ] = bgClr * a;
//		sc.out.elementVals[ mShadeOutputIndex ] = bgClr;
	}
	a = 1.0f - Intens( bgTrans );
	if( a < 0.0f ) a = 0.0f;
	sc.out.elementVals[ mShadeOutputIndex ].a = a;

	if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
			&& sc.globContext->pToneOp->Active(sc.CurTime())
			&& sc.globContext->pToneOp->GetProcessBackground()) {
		sc.globContext->pToneOp->ScaledToRGB(sc.out.elementVals[ mShadeOutputIndex ]);
	}
}


///////////////////////////////////////////////////////////////////////////////
//
//	Reflection render element
//
class ReflectionRenderElement : public BaseRenderElement {
public:
		ReflectionRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return reflectionRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_REFLECTION_RENDER_ELEMENT ); }

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
};

// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class ReflectionElementClassDesc : public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new ReflectionRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_REFLECTION_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return reflectionRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("reflectionRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static ReflectionElementClassDesc reflectionCD;
ClassDesc* GetReflectionElementDesc() { return &reflectionCD; }


// ---------------------------------------------
// Reflection parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 reflectionParamBlk(PBLOCK_NUMBER, _T("Reflection render element parameters"), 0, &reflectionCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	end
	);

//--- Reflection Render Element ------------------------------------------------
ReflectionRenderElement::ReflectionRenderElement()
{
	reflectionCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle ReflectionRenderElement::Clone( RemapDir &remap )
{
	ReflectionRenderElement*	newEle = new ReflectionRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}


void ReflectionRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
 		Color c = ip.reflIllumOut;

		if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
				&& sc.globContext->pToneOp->Active(sc.CurTime())) {
			sc.globContext->pToneOp->ScaledToRGB(c);
		}

		ClampMax( c );
		sc.out.elementVals[ mShadeOutputIndex ] = c;
		float a = 1.0f - Intens( sc.out.t );
		if( a < 0.0f ) a = 0.0f;
		sc.out.elementVals[ mShadeOutputIndex ].a = a;
	}
}


//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//
//	Refraction render element
//
class RefractionRenderElement : public BaseRenderElement {
public:
		RefractionRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return refractionRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_REFRACTION_RENDER_ELEMENT ); }

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
};

// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class RefractionElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new RefractionRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_REFRACTION_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return refractionRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("refractionRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static RefractionElementClassDesc refractionCD;
ClassDesc* GetRefractionElementDesc() { return &refractionCD; }


// ----------------------------------------------------------
// Refraction parameter block description - global instance
// ----------------------------------------------------------
static ParamBlockDesc2 refractionParamBlk(PBLOCK_NUMBER, _T("Refraction render element parameters"), 0, &refractionCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
/*
	BaseRenderElement::pathName, _T("pathName"), TYPE_STRING, 0, IDS_PATH_NAME,
		p_default, "",
		end,
*/
	end
	);

//--- Refraction Render Element ------------------------------------------------
RefractionRenderElement::RefractionRenderElement()
{
	refractionCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle RefractionRenderElement::Clone( RemapDir &remap )
{
	RefractionRenderElement*	newEle = new RefractionRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}


void RefractionRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
 		Color c = ip.transIllumOut;

		if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
				&& sc.globContext->pToneOp->Active(sc.CurTime())) {
			sc.globContext->pToneOp->ScaledToRGB(c);
		}

		ClampMax( c );
		sc.out.elementVals[ mShadeOutputIndex ] = c;
//		sc.out.elementVals[ mShadeOutputIndex ] = ip.transIllumOut;
		float a = 1.0f - Intens( sc.out.t );
		if( a < 0.0f ) a = 0.0f;
		sc.out.elementVals[ mShadeOutputIndex ].a = a;
	}
}


//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//
//	Shadow render element
//
class ShadowRenderElement : public BaseRenderElement {
public:
		ShadowRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		// note, we don't override the ShadowsApplied(){ return FALSE; }
		// we control it ourselves

		Class_ID ClassID() {return shadowRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_SHADOW_RENDER_ELEMENT ); }

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
	
		BOOL ShadowsApplied() const{ return TRUE; }
};

// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class ShadowElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new ShadowRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_SHADOW_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return shadowRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("shadowRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static ShadowElementClassDesc shadowCD;
ClassDesc* GetShadowElementDesc() { return &shadowCD; }

  
// ---------------------------------------------
// Shadow parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 shadowParamBlk(PBLOCK_NUMBER, _T("Shadow render element parameters"), 0, &shadowCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, FALSE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
//	BaseRenderElement::colorOn, _T("colorOn"), TYPE_BOOL, 0, IDS_COLOR_ON,
//		p_default, TRUE,
//		end,
//	BaseRenderElement::alphaOn, _T("alphaOn"), TYPE_BOOL, 0, IDS_ALPHA_ON,
//		p_default, TRUE,
//		end,
	end
	);

//--- Shadow Render Element ------------------------------------------------
ShadowRenderElement::ShadowRenderElement()
{
	shadowCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle ShadowRenderElement::Clone( RemapDir &remap )
{
	ShadowRenderElement* newEle = new ShadowRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}

inline void Clamp( float& val ){ 
	if( val < 0.0f ) val = 0.0f;
	else if( val > 1.0f ) val = 1.0f;
}

/************
// returns shadow fraction & shadowClr
// move this util to shade context at first opportunity
float IllumShadow( ShadeContext& sc, Color& shadowClr ) 
{ 
	IlluminateComponents illumComp;
	IIlluminationComponents* pIComponents;	
	Color illumClr(0,0,0);
	Color illumClrNS(0,0,0);
	shadowClr.Black();
	Point3 N = sc.Normal();
	
	for (int i = 0; i < sc.nLights; i++) {
		LightDesc* l = sc.Light( i );
		pIComponents = (IIlluminationComponents*)l->GetInterface( IID_IIlluminationComponents );
		if( pIComponents ){
			// use component wise illuminate routines
			if (!pIComponents->Illuminate( sc, N, illumComp ))
				continue;

			illumClr += (illumComp.finalColor - illumComp.shadowColor ) * illumComp.geometricAtten;
			illumClrNS += illumComp.finalColorNS * illumComp.geometricAtten;

			if( illumComp.rawColor != illumComp.filteredColor ){
				// light is filtered by a transparent object, sum both filter & user shadow color
				shadowClr += illumComp.finalColor * illumComp.geometricAtten; //attenuated filterColor 
			} else {
				// no transparency, sum in just the shadow color
				shadowClr += illumComp.shadowColor * illumComp.geometricAtten;
			}

		} else {
			// no component interface, shadow clr is black
			Color lightCol;
			Point3 L;
			register float NL, diffCoef;
			if (!l->Illuminate(sc, N, lightCol, L, NL, diffCoef))
				continue;
			if (diffCoef <= 0.0f)	  
				continue;
			illumClr += diffCoef * lightCol;

			if( sc.shadow ){
				sc.shadow = FALSE;
				l->Illuminate(sc, N, lightCol, L, NL, diffCoef);
				illumClrNS += diffCoef * lightCol;
				sc.shadow = TRUE;
			} else {
				illumClrNS = illumClr;
			}
		}
	}// end, for each light

	float intensNS = Intens(illumClrNS);
//	Clamp( intensNS );
	if( intensNS < 0.0f ) intensNS = 0.0f;
	float intens = Intens(illumClr);
//	Clamp( intens );
	if( intens < 0.0f ) intens = 0.0f;
	float atten = (intensNS > 0.05f)? intens/intensNS : 1.0f;

	// correction for negative lights
	if (atten > 1.0f)
		atten = 1.0f/atten;

	return atten;
}
**************/

static TCHAR* shadowIllumOutStr = "shadowIllumOut";

void ShadowRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
//		sc.shadow = 0;
//		Color clrNS = sc.DiffuseIllum();
//		sc.shadow = 1;
//		Color clrS = sc.DiffuseIllum();
//		float ns = Intens(clrNS);
//		float atten = (ns > 0.0f)? Intens(clrS)/ns : 1.0f;
//		float atten = (ns > 0.01f)? Intens(clrS)/ns : 1.0f;
//		if (atten>1.0f) atten = 1.0f/atten;	// correction for negative lights

		Color sClr;
		float shadowAtten = IllumShadow( sc, sClr );
		float a = 1.0f-shadowAtten;



#ifndef NO_MTL_MATTESHADOW // orb 01-07-2001
		if(ip.pMtl->ClassID() != Class_ID(MATTE_CLASS_ID,0) )
			a *= 1.0f - Intens( sc.out.t );
#endif // NO_MTL_MATTESHADOW
		if( a < 0.0f ) a = 0.0f;

		int nUser = ip.FindUserIllumName( shadowIllumOutStr );
		if( nUser >= 0 )
			sClr = ip.GetUserIllumOutput( nUser );
		else
			sClr *= a;

		sc.out.elementVals[ mShadeOutputIndex ] 
			= AColor( sClr.r, sClr.g, sClr.b, a );
	}
}


//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
//
//	Atmosphere render element
//
class AtmosphereRenderElement : public BaseRenderElement {
public:
		AtmosphereRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return atmosphereRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_ATMOS_RENDER_ELEMENT ); }

		BOOL AtmosphereApplied() const{ return TRUE; } //override, always on

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
		void PostAtmosphere(ShadeContext& sc, float z, float zPrev);
};

// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class AtmosphereElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new AtmosphereRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_ATMOS_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return atmosphereRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("atmosphereRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static AtmosphereElementClassDesc atmosphereCD;
ClassDesc* GetAtmosphereElementDesc() { return &atmosphereCD; }


// ---------------------------------------------
// Atmosphere parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 atmosphereParamBlk(PBLOCK_NUMBER, _T("Atmosphere render element parameters"), 0, &atmosphereCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	end
	);

//--- Atmosphere Render Element ------------------------------------------------
AtmosphereRenderElement::AtmosphereRenderElement()
{
	atmosphereCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle AtmosphereRenderElement::Clone( RemapDir &remap )
{
	AtmosphereRenderElement*	newEle = new AtmosphereRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}


void AtmosphereRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
}

void AtmosphereRenderElement::PostAtmosphere(ShadeContext& sc, float z, float zPrev)
{
	sc.out.elementVals[ mShadeOutputIndex ] = sc.out.c;
	sc.out.elementVals[ mShadeOutputIndex ].a = 1.0f - Intens( sc.out.t );

	if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
			&& sc.globContext->pToneOp->Active(sc.CurTime())
			&& sc.globContext->pToneOp->GetProcessBackground()) {
		sc.globContext->pToneOp->ScaledToRGB(sc.out.elementVals[ mShadeOutputIndex ]);
		// This is a hack to make the alpha better for compositing. Since we
		// are changing the brightness, we change the alpha the same way.
		// The problem was that the low alphas were causing the bright pixels
		// to be truncated to the alpha values.
		float a = sc.globContext->pToneOp->ScaledToRGB(
			sc.out.elementVals[ mShadeOutputIndex ].a);
		if (a >= 1.0f)
			a = 1.0f;
		sc.out.elementVals[ mShadeOutputIndex ].a = a;
	}
}

//-----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
//
//	Alpha render element
//
class AlphaRenderElement : public BaseRenderElement {
public:
		AlphaRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return alphaRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_ALPHA_RENDER_ELEMENT ); }
		BOOL AtmosphereApplied() const{ return TRUE; } //we get z info from atmosphere

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
		void PostAtmosphere(ShadeContext& sc, float z, float zPrev);
};

// --------------------------------------------------
// alpha element class descriptor 
class AlphaElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new AlphaRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_ALPHA_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return alphaRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("alphaRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static AlphaElementClassDesc alphaCD;
ClassDesc* GetAlphaElementDesc() { return &alphaCD; }


// ---------------------------------------------
// Alpha parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 alphaParamBlk(PBLOCK_NUMBER, _T("Alpha render element parameters"), 0, &alphaCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	end
	);

//--- Alpha Render Element ------------------------------------------------
AlphaRenderElement::AlphaRenderElement()
{
	alphaCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle AlphaRenderElement::Clone( RemapDir &remap )
{
	AlphaRenderElement* newEle = new AlphaRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}


void AlphaRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);

	} else {
		float a = 1.0f - Intens( sc.out.t );
		if( a < 0.0f ) a = 0.0f; else if( a > 1.0f ) a = 1.0f;
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0.0f,0.0f,0.0f,a);
//		sc.out.elementVals[ mShadeOutputIndex ].Black();
	}
}

void AlphaRenderElement::PostAtmosphere(ShadeContext& sc, float z, float zPrev)
{
		float a = 1.0f - Intens( sc.out.t );
		sc.out.elementVals[ mShadeOutputIndex ] = AColor( a, a, a, a );
}



///////////////////////////////////////////////////////////////////////////////
//
//	Blend render element
//
class BlendRenderElement : public BaseRenderElement {
public:
		enum{
//			diffuseOn = BaseRenderElement::pathName+1, 
			diffuseOn = BaseRenderElement::pbBitmap+1, 
		    atmosphereOn, shadowOn,
			ambientOn, specularOn, emissionOn, reflectionOn, refractionOn,
			inkOn, paintOn
		};

		BlendRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return blendRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_BLEND_RENDER_ELEMENT ); }

		SFXParamDlg *CreateParamDialog(IRendParams *ip);

		//attributes of the blend
		void SetApplyAtmosphere(BOOL on){ mpParamBlk->SetValue( atmosphereOn, 0, on ); }
		BOOL AtmosphereApplied() const{
			int	on;
			mpParamBlk->GetValue( atmosphereOn, 0, on, FOREVER );
			return on;
		}

		void SetApplyShadows(BOOL on){ mpParamBlk->SetValue( shadowOn, 0, on ); }
		BOOL ShadowsApplied() const{
			int	on;
			mpParamBlk->GetValue( shadowOn, 0, on, FOREVER );
			return on;
		}

		void SetDiffuseOn(BOOL on){ mpParamBlk->SetValue( diffuseOn, 0, on ); }
		BOOL IsDiffuseOn() const{
			int	on;
			mpParamBlk->GetValue( diffuseOn, 0, on, FOREVER );
			return on;
		}

		void SetAmbientOn(BOOL on){ mpParamBlk->SetValue( ambientOn, 0, on ); }
		BOOL IsAmbientOn() const{
			int	on;
			mpParamBlk->GetValue( ambientOn, 0, on, FOREVER );
			return on;
		}

		void SetSpecularOn(BOOL on){ mpParamBlk->SetValue( specularOn, 0, on ); }
		BOOL IsSpecularOn() const{
			int	on;
			mpParamBlk->GetValue( specularOn, 0, on, FOREVER );
			return on;
		}

		void SetEmissionOn(BOOL on){ mpParamBlk->SetValue( emissionOn, 0, on ); }
		BOOL IsEmissionOn() const{
			int	on;
			mpParamBlk->GetValue( emissionOn, 0, on, FOREVER );
			return on;
		}

		void SetReflectionOn(BOOL on){ mpParamBlk->SetValue( reflectionOn, 0, on ); }
		BOOL IsReflectionOn() const{
			int	on;
			mpParamBlk->GetValue( reflectionOn, 0, on, FOREVER );
			return on;
		}
		void SetRefractionOn(BOOL on){ mpParamBlk->SetValue( refractionOn, 0, on ); }
		BOOL IsRefractionOn() const{
			int	on;
			mpParamBlk->GetValue( refractionOn, 0, on, FOREVER );
			return on;
		}

		void SetInkOn(BOOL on){ mpParamBlk->SetValue( inkOn, 0, on ); }
		BOOL IsInkOn() const{
			int	on;
			mpParamBlk->GetValue( inkOn, 0, on, FOREVER );
			return on;
		}
		void SetPaintOn(BOOL on){ mpParamBlk->SetValue( paintOn, 0, on ); }
		BOOL IsPaintOn() const{
			int	on;
			mpParamBlk->GetValue( paintOn, 0, on, FOREVER );
			return on;
		}

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
		void PostAtmosphere(ShadeContext& sc, float z, float zPrev);
};

// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class BlendElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new BlendRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_BLEND_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return blendRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("blendRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static BlendElementClassDesc BlendCD;
ClassDesc* GetBlendElementDesc() { return &BlendCD; }

IRenderElementParamDlg *BlendRenderElement::CreateParamDialog(IRendParams *ip)
{
	return (IRenderElementParamDlg *)BlendCD.CreateParamDialogs(ip, this);
}

// ---------------------------------------------
// Blend parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 BlendParamBlk(PBLOCK_NUMBER, _T("Blend render element parameters"), 0, &BlendCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_BLEND_ELEMENT, IDS_BLEND_ELEMENT_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	BlendRenderElement::atmosphereOn, _T("atmosphereOn"), TYPE_BOOL, 0, IDS_ATMOSPHERE_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_ATMOS_ON,
		end,
	BlendRenderElement::shadowOn, _T("shadowOn"), TYPE_BOOL, 0, IDS_SHADOW_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SHADOW_ON,
		end,
	BlendRenderElement::diffuseOn, _T("diffuseOn"), TYPE_BOOL, 0, IDS_DIFFUSE_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_DIFFUSE_ON,
		end,
	BlendRenderElement::ambientOn, _T("ambientOn"), TYPE_BOOL, 0, IDS_AMBIENT_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_AMBIENT_ON,
		end,
	BlendRenderElement::specularOn, _T("specularOn"), TYPE_BOOL, 0, IDS_SPECULAR_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SPECULAR_ON,
		end,
	BlendRenderElement::emissionOn, _T("emissionOn"), TYPE_BOOL, 0, IDS_EMISSION_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_EMISSION_ON,
		end,
	BlendRenderElement::reflectionOn, _T("reflectionOn"), TYPE_BOOL, 0, IDS_REFLECTION_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_REFLECTION_ON,
		end,
	BlendRenderElement::refractionOn, _T("refractionOn"), TYPE_BOOL, 0, IDS_REFRACTION_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_REFRACTION_ON,
		end,
	BlendRenderElement::inkOn, _T("inkOn"), TYPE_BOOL, 0, IDS_INK_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_INK_ON,
		end,
	BlendRenderElement::paintOn, _T("paintOn"), TYPE_BOOL, 0, IDS_PAINT_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_PAINT_ON,
		end,

	end
	); 

//--- Blend Render Element ------------------------------------------------
BlendRenderElement::BlendRenderElement()
{
	BlendCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle BlendRenderElement::Clone( RemapDir &remap )
{
	BlendRenderElement*	newEle = new BlendRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}


void BlendRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	AColor c(0,0,0,0);
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = c;
	} else {
		if( IsDiffuseOn() ) c += ip.diffIllumOut;
		if( IsAmbientOn() ) c += ip.ambIllumOut;
		if( IsEmissionOn() ) c += ip.selfIllumOut;
		c *= ip.finalAttenuation;
		if( IsSpecularOn() ) c += ip.specIllumOut;
		if( IsReflectionOn() ) c += ip.reflIllumOut;
		if( IsRefractionOn() ) c += ip.transIllumOut;
		int n;
		if( ShadowsApplied() && (n = ip.FindUserIllumName( shadowIllumOutStr ))>=0 )
			c += ip.GetUserIllumOutput( n );

		n = ip.FindUserIllumName("Paint");
		if(IsPaintOn() && n>=0 )
			c += ip.finalAttenuation * (ip.GetUserIllumOutput(n));
		
		n = ip.FindUserIllumName("Ink");
		if( IsInkOn() && n>=0 )
			c += ip.finalAttenuation * (ip.GetUserIllumOutput(n));


		c.a = Intens( sc.out.t );

		if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
				&& sc.globContext->pToneOp->Active(sc.CurTime())) {
			sc.globContext->pToneOp->ScaledToRGB(c);
		}

		ClampMax( c );

		sc.out.elementVals[ mShadeOutputIndex ] = c;
		float a = 1.0f - Intens( sc.out.t );
		if( a < 0.0f ) a = 0.0f;
		sc.out.elementVals[ mShadeOutputIndex ].a = a;
	}
}

void BlendRenderElement::PostAtmosphere(ShadeContext& sc, float z, float zPrev)
{
	sc.out.elementVals[ mShadeOutputIndex ] = sc.out.c;

	if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
			&& sc.globContext->pToneOp->Active(sc.CurTime())
			&& sc.globContext->pToneOp->GetProcessBackground()) {
		sc.globContext->pToneOp->ScaledToRGB(sc.out.elementVals[ mShadeOutputIndex ]);
	}

	sc.out.elementVals[ mShadeOutputIndex ].a = 1.0f - Intens( sc.out.t );
}

//-----------------------------------------------------------------------------------------
#ifndef NO_RENDER_ELEMENT_MOTION
///////////////////////////////////////////////////////////////////////////////
//
//	Motion render element
//
class MotionRenderElement : public BaseRenderElement, public IRenderElementRequirements {
private:

	CRITICAL_SECTION mCriticalSection;
	float mRenderMaxVelocity;	// max velocity of current rendering
	float mLocalMaxVelocity;	// cached max velocity parameter
	BOOL mLocalVelocityUpdate;	// cached update parameter
	bool mMaxVelocityWasUpdated;

	static void RenderCallback(void *param, NotifyInfo *info);
public:
	enum { 
		velocityMax = BaseRenderElement::pbBitmap + 1,
		velocityUpdate = BaseRenderElement::pbBitmap + 2
	};

	MotionRenderElement();
	virtual ~MotionRenderElement();
	RefTargetHandle Clone( RemapDir &remap );

	Class_ID ClassID() {return motionRenderElementClassID;}	
	TSTR GetName() { return GetString(IDS_MOTION_RENDER_ELEMENT); }

	IRenderElementParamDlg *CreateParamDialog(IRendParams *ip);

	BOOL AtmosphereApplied() const{ return FALSE; }
	BOOL BlendOnMultipass() const { return TRUE; }

	// the compute functions
	void PostIllum(ShadeContext& sc, IllumParams& ip);
	void PostAtmosphere(ShadeContext& sc, float z, float zPrev) {};
	BOOL ShadowsApplied() const{ return FALSE; }

	void SetMaxVelocityParam(float v){
		if( mpParamBlk != NULL ) {
			mpParamBlk->SetValue( velocityMax, 0, v ); 
		}
	}
	float GetMaxVelocityParam() const {
		if( mpParamBlk != NULL ) {
			float v;
			mpParamBlk->GetValue( velocityMax, 0, v, FOREVER );
			return v;
		}
		else {
			return 1.0f;
		}
	}

	void SetVelocityUpdateParam(BOOL on){ 
		if( mpParamBlk != NULL ) {
			mpParamBlk->SetValue( velocityUpdate, 0, on ); 
		}
	}
	BOOL GetVelocityUpdateParam() const{
		if( mpParamBlk != NULL ) {
			int	on;
			mpParamBlk->GetValue( velocityUpdate, 0, on, FOREVER );
			return on;
		}
		else {
			return FALSE;
		}
	}

	// -- from IRenderElementRequirements
	virtual bool HasRequirement(Requirement requirement);

	// -- from InterfaceServer
	virtual BaseInterface* GetInterface(Interface_ID id);
};

class MotionRenderElementClassDesc : public ClassDesc2 {
public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new MotionRenderElement(); }
	const TCHAR *	ClassName() { return GetString(IDS_MOTION_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID		ClassID() { return motionRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("MotionRenderElement"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
};

// global instance
static MotionRenderElementClassDesc motionRenderElementCD;
#endif NO_RENDER_ELEMENT_MOTION
ClassDesc* GetMotionElementDesc() {
#ifndef NO_RENDER_ELEMENT_MOTION
	return &motionRenderElementCD;
#else	// NO_RENDER_ELEMENT_MOTION
	return NULL;
#endif	// NO_RENDER_ELEMENT_MOTION
}
#ifndef NO_RENDER_ELEMENT_MOTION
IRenderElementParamDlg* MotionRenderElement::CreateParamDialog(IRendParams *irp)
{
	return (IRenderElementParamDlg*) motionRenderElementCD.CreateParamDialogs(irp, this);
}


// ---------------------------------------------
// motion render element parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 MotionRenderElementParamBlk(PBLOCK_NUMBER, _T("Motion render element parameters"), 0, &motionRenderElementCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
		//rollout
		IDD_MOTION_RENDER_ELEMENT, IDS_MOTION_ELEMENT_PARAMS, 0, 0, NULL,
		// params
		BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
		BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, FALSE,
		end,
		BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
		BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
		MotionRenderElement::velocityMax, _T("velocityMax"), TYPE_FLOAT, 0, IDS_VMAX,
		p_default,		1.0f,
		p_range, 		0.01f, 5000.0f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_VMAX_EDIT, IDC_VMAX_SPIN, SPIN_AUTOSCALE, 
		end,
		MotionRenderElement::velocityUpdate, _T("velocityOn"), TYPE_BOOL, 0, IDS_VELOCITY_UPDATE,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_VELOCITY_UPDATE,
		end,
		end
		);

//--- Motion Render Element ------------------------------------------------
MotionRenderElement::MotionRenderElement()
: mMaxVelocityWasUpdated(false)
{
	motionRenderElementCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName(GetName());
	InitializeCriticalSectionAndSpinCount(&mCriticalSection, 10000); 
	RegisterNotification(RenderCallback, this, NOTIFY_PRE_RENDER);
	RegisterNotification(RenderCallback, this, NOTIFY_POST_RENDER);
}

MotionRenderElement::~MotionRenderElement() 
{
	DeleteCriticalSection(&mCriticalSection);
	UnRegisterNotification(RenderCallback, this);
}

RefTargetHandle MotionRenderElement::Clone( RemapDir &remap )
{
	MotionRenderElement *mnew = new MotionRenderElement();
	mnew->ReplaceReference(0, remap.CloneRef(mpParamBlk));
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}

void MotionRenderElement::RenderCallback(void *param, NotifyInfo *info)
{
	switch( info->intcode ) 
	{
	case NOTIFY_PRE_RENDER:
		{			
			MotionRenderElement *rElem = (MotionRenderElement *)param;
			rElem->mRenderMaxVelocity = 0.0f;
			// cache local copies of parameters so we don't have to fetch them each call to PostIllum()
			rElem->mLocalMaxVelocity = rElem->GetMaxVelocityParam();
			rElem->mLocalVelocityUpdate = rElem->GetVelocityUpdateParam();
			rElem->mMaxVelocityWasUpdated = false;
		}
		break;
	case NOTIFY_POST_RENDER:
		{			
			MotionRenderElement *rElem = (MotionRenderElement *)param;
			if( rElem->mLocalVelocityUpdate && rElem->mMaxVelocityWasUpdated ) {
				rElem->SetMaxVelocityParam( rElem->mRenderMaxVelocity ? rElem->mRenderMaxVelocity : 1.0f );
			}			
		}
		break;
	default:
		break;
	}
}

void MotionRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl != NULL ){	
		IShadeContextExtension8 *iShadeContext = Get_IShadeContextExtension8( sc );
		if( iShadeContext != NULL ) {
			Point2 p = iShadeContext->MotionVector();
			float x = ( p.x / mLocalMaxVelocity + 1 ) * 0.5;

			if( x < 0 ) x = 0;
			else if( x > 1 ) x = 1;

			float y = ( - p.y / mLocalMaxVelocity + 1 ) * 0.5;

			if( y < 0 ) y = 0;
			else if( y > 1 ) y = 1;

			// The BLUE component is set to 0.5 in order to specify zero movement in Z.
			sc.out.elementVals[ mShadeOutputIndex ] = AColor( x, y, 0.5f, 1.0f );

			if( mLocalVelocityUpdate )
			{				
				float length = p.Length();
				if( length > mRenderMaxVelocity ) {
					EnterCriticalSection( &mCriticalSection ); 
					if( length > mRenderMaxVelocity ) {
						mMaxVelocityWasUpdated = true;
						mRenderMaxVelocity = length;
					}
					LeaveCriticalSection( &mCriticalSection ); 
				}
			}
			return;
		}
	}

	// Store values of 0.5 in order to indicate zero movement
	sc.out.elementVals[ mShadeOutputIndex ] = AColor(0.5f,0.5f,0.5f,0.0f);    
}

//--- IRenderElementRequirements--------------------------------------------
bool MotionRenderElement::HasRequirement(Requirement requirement)
{
	if( requirement == kRequirement_MotionData ) {
		return true;
	}
	else {
		return false;
	}
}

//--- InterfaceServer ------------------------------------------------------
BaseInterface* MotionRenderElement::GetInterface ( Interface_ID id )
{
	if ( id == IRENDERELEMENTREQUIREMENTS_INTERFACE_ID ) {
		return static_cast<IRenderElementRequirements*>(this);
	}
	else {
		return BaseRenderElement::GetInterface ( id );
	}
}
#endif	// NO_RENDER_ELEMENT_MOTION
//-----------------------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
//
//	Z render element
//
class ZRenderElement : public BaseRenderElement {
private:
	CRITICAL_SECTION mCriticalSection;
	float mRenderMaxZ;	// max Z depth of current rendering
	float mRenderMinZ;
	float mLocalMaxZ;	// cached max Z depth parameter
	float mLocalMinZ;
	BOOL mLocalZUpdate;	// cached update parameter
	bool mMaxMinZWasUpdated;
	static void RenderCallback(void *param, NotifyInfo *info);
public:
		enum{
			ZMin = BaseRenderElement::pbBitmap+1, 
		    ZMax,
			ZUpdate
		};

		ZRenderElement();
		virtual ~ZRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return ZRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_Z_RENDER_ELEMENT ); }

		SFXParamDlg *CreateParamDialog(IRendParams *ip);

		// override shared attributes
//		BOOL IsFilterEnabled() const{ return FALSE; } // can't filter z
		BOOL AtmosphereApplied() const{ return TRUE; } //we get z info from atmosphere
		BOOL BlendOnMultipass() const { return FALSE; } // don't blend z's

		//attributes of the z element
		void SetZMin(float z){
			if( mpParamBlk != NULL ) {
				mpParamBlk->SetValue( ZMin, 0, z ); 
			}
		}
		float GetZMin() const {
			float z=0;
			if( mpParamBlk != NULL ) {
				mpParamBlk->GetValue( ZMin, 0, z, FOREVER );
			}
			return z;
		}

		void SetZMax(float z){
			if( mpParamBlk != NULL ) {
				mpParamBlk->SetValue( ZMax, 0, z ); 
			}
		}
		float GetZMax() const {
			float z=0;
			if( mpParamBlk != NULL ) {
				mpParamBlk->GetValue( ZMax, 0, z, FOREVER );
			}
			return z;
		}

		void SetZUpdate(BOOL on){ 
			if( mpParamBlk != NULL ) {
				mpParamBlk->SetValue( ZUpdate, 0, on ); 
			}
		}
		BOOL GetZUpdate() const{
			if( mpParamBlk != NULL ) {
				int	on;
				mpParamBlk->GetValue( ZUpdate, 0, on, FOREVER );
				return on;
			}
			else {
				return FALSE;
			}
		}
		// the compute functions
		void PostIllum(ShadeContext& sc, IllumParams& ip);
		void PostAtmosphere(ShadeContext& sc, float z, float zPrev);
};
 
// --------------------------------------------------
// element class descriptor - class declaration
//
class ZElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new ZRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_Z_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return ZRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("ZRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static ZElementClassDesc zElementCD;
ClassDesc* GetZElementDesc() { return &zElementCD; }

IRenderElementParamDlg* ZRenderElement::CreateParamDialog(IRendParams *irp)
{
	return (IRenderElementParamDlg*) zElementCD.CreateParamDialogs(irp, this);
}

// ---------------------------------------------
// z parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 ZParamBlk(PBLOCK_NUMBER, _T("Z render element parameters"), 0, &zElementCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_Z_ELEMENT, IDS_Z_ELEMENT_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, FALSE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	ZRenderElement::ZMin, _T("zMin"), TYPE_FLOAT, 0, IDS_ZMIN,
		p_default,		100.0,
		p_range, 		0.0, 250000.0, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_ZMIN_EDIT, IDC_ZMIN_SPIN, SPIN_AUTOSCALE, 
//		p_accessor,		&extendedPBAccessor,
		end, 
	ZRenderElement::ZMax, _T("zMax"), TYPE_FLOAT, 0, IDS_ZMAX,
		p_default,		300.0,
		p_range, 		0.0, 250000.0, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_ZMAX_EDIT, IDC_ZMAX_SPIN, SPIN_AUTOSCALE, 
		end,
	ZRenderElement::ZUpdate, _T("zUpdate"), TYPE_BOOL, 0, IDS_ZDEPTH_UPDATE,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_ZDEPTH_UPDATE,
		end,
	end
	);

//--- Z Render Element ------------------------------------------------
ZRenderElement::ZRenderElement()
{
	zElementCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
	InitializeCriticalSectionAndSpinCount(&mCriticalSection, 10000); 
	RegisterNotification(RenderCallback, this, NOTIFY_PRE_RENDER);
	RegisterNotification(RenderCallback, this, NOTIFY_POST_RENDER);
}

ZRenderElement::~ZRenderElement()
{
	DeleteCriticalSection(&mCriticalSection);
	UnRegisterNotification(RenderCallback, this);
}


RefTargetHandle ZRenderElement::Clone( RemapDir &remap )
{
	ZRenderElement*	newEle = new ZRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}

void ZRenderElement::RenderCallback(void *param, NotifyInfo *info)
{
	switch( info->intcode ) 
	{
	case NOTIFY_PRE_RENDER:
		{			
			ZRenderElement *rElem = (ZRenderElement *)param;
			rElem->mRenderMaxZ = 0.0f;
			rElem->mRenderMinZ = 0.0f;
			// cache local copies of parameters so we don't have to fetch them each call to PostIllum()
			rElem->mLocalMaxZ = rElem->GetZMax();
			rElem->mLocalMinZ = rElem->GetZMin();
			rElem->mLocalZUpdate = rElem->GetZUpdate();
			rElem->mMaxMinZWasUpdated = false;
		}
		break;
	case NOTIFY_POST_RENDER:
		{			
			ZRenderElement *rElem = (ZRenderElement *)param;
			if( rElem->mLocalZUpdate && rElem->mMaxMinZWasUpdated ) {
				rElem->SetZMax( rElem->mRenderMaxZ ? rElem->mRenderMaxZ : 0.0f );
				rElem->SetZMin( rElem->mRenderMinZ ? rElem->mRenderMinZ : 0.0f );
			}			
		}
		break;
	default:
		break;
	}
}

void ZRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	sc.out.elementVals[ mShadeOutputIndex ].Black();
	sc.out.elementVals[ mShadeOutputIndex ].a = 1.0f - Intens( sc.out.t );
}

void ZRenderElement::PostAtmosphere(ShadeContext& sc, float z, float zPrev)
{
	if( zPrev <= 0.0f ){
		// first level z
		float zOut;

		z = -z;

		if( z < mLocalMinZ ) zOut = 1.0f;
		else if( z > mLocalMaxZ ) zOut = 0.0f;	
		else zOut = 1.0 - ((z - mLocalMinZ) / (mLocalMaxZ - mLocalMinZ )) ;
//		AColor c( zOut, zOut, zOut, 1.0f - Intens( sc.out.t ) );

		sc.out.elementVals[ mShadeOutputIndex ].r = 
		sc.out.elementVals[ mShadeOutputIndex ].g = 
		sc.out.elementVals[ mShadeOutputIndex ].b = zOut;

		if( mLocalZUpdate )
		{				
			if( z > mRenderMaxZ ) {
				EnterCriticalSection( &mCriticalSection ); 
				if( z > mRenderMaxZ ) {
					mMaxMinZWasUpdated = true;
					if(z < INFINITE)
						mRenderMaxZ = z;
				}
				LeaveCriticalSection( &mCriticalSection ); 
			}
			else if( z < mRenderMinZ ) {
				EnterCriticalSection( &mCriticalSection ); 
				if( z < mRenderMinZ ) {
					mMaxMinZWasUpdated = true;
					mRenderMinZ = z;
				}
				LeaveCriticalSection( &mCriticalSection ); 
			}
		}
	}
}

//-----------------------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
//
//	Material Id render element
//
class MaterialIDRenderElement : public BaseRenderElement {
public:
	MaterialIDRenderElement();
	RefTargetHandle Clone( RemapDir &remap );

	Class_ID ClassID() {return MaterialIDRenderElementClassID;}
	TSTR GetName() { return GetString( IDS_MATERIALID_RENDER_ELEMENT ); }

	// the compute function
	void PostIllum(ShadeContext& sc, IllumParams& ip);

};

// --------------------------------------------------
// Material ID element class descriptor
// --------------------------------------------------
class MaterialIDElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new MaterialIDRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_MATERIALID_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return MaterialIDRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("materialIDRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static MaterialIDElementClassDesc materialIDCD;
ClassDesc* GetMaterialIDElementDesc() { return &materialIDCD; }


// ---------------------------------------------
// Material ID parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 MaterialIDParamBlk(PBLOCK_NUMBER, _T("Material ID render element parameters"), 0, &materialIDCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, FALSE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	end
	);

//--- Material ID Render Element ------------------------------------------------
MaterialIDRenderElement::MaterialIDRenderElement()
{
	materialIDCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle MaterialIDRenderElement::Clone( RemapDir &remap )
{
	MaterialIDRenderElement* newEle = new MaterialIDRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}

void MaterialIDRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd

		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,1);

	} else {
		int mtlID = ip.pMtl->GetGBufID();

		//Get the color corresponding to the id
		BMM_Color_24 c;
		GBuffer::ColorById( mtlID , c );

		sc.out.elementVals[ mShadeOutputIndex ] = AColor( c );
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//	Object Id render element
//
class ObjectIDRenderElement : public BaseRenderElement {
public:
	enum{
		colorMode = BaseRenderElement::pbBitmap+1
	};

	enum ColorMode{
		kOBJECTCOLOR,
		kOBJECTID,
		kMAXMODE
	};

	ObjectIDRenderElement();
	RefTargetHandle Clone( RemapDir &remap );

	Class_ID ClassID() {return ObjectIDRenderElementClassID;}
	TSTR GetName() { return GetString( IDS_OBJECTID_RENDER_ELEMENT ); }
	SFXParamDlg *CreateParamDialog(IRendParams *ip);

	// the compute function
	void PostIllum(ShadeContext& sc, IllumParams& ip);

	void SetColorMode(int mode){ 
		mpParamBlk->SetValue( colorMode, 0, mode ); 
	}
	int GetColorMode() const{
		int	mode;
		mpParamBlk->GetValue( colorMode, 0, mode, FOREVER );
		return mode;
	}
protected:
	
};

// --------------------------------------------------
// Object ID element class descriptor
// --------------------------------------------------
class ObjectIDElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new ObjectIDRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_OBJECTID_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return ObjectIDRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("objectIDRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static ObjectIDElementClassDesc objectIDCD;
ClassDesc* GetObjectIDElementDesc() { return &objectIDCD; }


// ---------------------------------------------
// Object ID parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 ObjectIDParamBlk(PBLOCK_NUMBER, _T("Object ID render element parameters"), 0, &objectIDCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//Roll out
	IDD_OBJECTID_ELEMENT, IDS_OBJECTID_ELEMENT_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, FALSE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	ObjectIDRenderElement::colorMode, _T("colorMode"), TYPE_INT, 0, IDS_OBJECTID_MODE,
		p_default, TRUE,
		p_ui, TYPE_RADIO, 2,  IDC_RADIO_OBJECTCOL,  IDC_RADIO_OBJECTID,
		p_vals, ObjectIDRenderElement::kOBJECTCOLOR, ObjectIDRenderElement::kOBJECTID,
		end,
	end
);

//--- Object ID Render Element ------------------------------------------------
ObjectIDRenderElement::ObjectIDRenderElement()
{
	objectIDCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}

IRenderElementParamDlg *ObjectIDRenderElement::CreateParamDialog(IRendParams *ip)
{
	return (IRenderElementParamDlg *)objectIDCD.CreateParamDialogs(ip, this);
}

RefTargetHandle ObjectIDRenderElement::Clone( RemapDir &remap )
{
	ObjectIDRenderElement* newEle = new ObjectIDRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}

void ObjectIDRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd

		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,1);

	} else {
		//Get the color mode set in the UI
		int mode = GetColorMode();
		int id = sc.NodeID();

		//Get the render instance corresponding to the node id
		RenderInstance *inst = sc.globContext->GetRenderInstance(id);

		int mtlID = 0;
		DWORD wireColor = 0;


		//Lets get the id and wire color
		if( inst != NULL )
		{
			INode* node = inst->GetINode();
			if( node != NULL )
			{
				mtlID = node->GetGBufID();
				wireColor = node->GetWireColor();
			}
		}

		switch(mode) {
		case kOBJECTCOLOR:
			//Set the output color to the wire color
			sc.out.elementVals[ mShadeOutputIndex ] = AColor( wireColor );
			break;
		case kOBJECTID:
			//Get the color corresponding to the id
			BMM_Color_24 c;
			GBuffer::ColorById( mtlID , c );
			sc.out.elementVals[ mShadeOutputIndex ] = AColor( c );
			break;
		}
		
	}
}
