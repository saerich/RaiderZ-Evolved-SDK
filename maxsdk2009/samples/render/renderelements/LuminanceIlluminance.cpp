/*==============================================================================

  file:	        LuminanceIlluminance.cpp

  author:       Daniel Levesque

  created:	   12 May 2005

  description:
        Render elements for rendering luminance and illuminance values.

		Adapted from the luminance & illuminance render elements used by the
		pseudo-color exposure control.


  modified:	


© 2005 Autodesk
==============================================================================*/

#include "renElemPch.h"
#include "LuminanceIlluminance.h"
#include <ILightingUnits.h>

static const double M_PI = 3.14159265358979323846;

//same as in radiositysolution.h
#define DEFAULT_MAX_TO_PHYSICAL_LIGHTING_SCALE  1500

class LuminanceClassDesc:public ClassDesc2
{
public:
    int 			IsPublic() { return TRUE; }
    void *			Create(BOOL loading) { return new LuminanceRenderElement; }
    const TCHAR *	ClassName() { return GetString(IDS_LUMINANCE_CLASSNAME); }
    SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
    Class_ID 		ClassID() { return LUM_CLASS_ID; }
    const TCHAR* 	Category() { return _T(""); }
    const TCHAR*	InternalName() { return _T("LuminanceRenderElement"); } // hard-coded for scripter
    HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static LuminanceClassDesc lumCD;
ClassDesc2& GetLuminanceElementClassDesc() { 
	return lumCD; 
}

ParamBlockDesc2 LuminanceRenderElement::m_paramBlockDesc(
	PBLOCK_NUMBER, _T("LuminanceREParameters"), IDS_LUMINANCE_CLASSNAME, &lumCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,

	//rollout
	IDD_LUMINANCEILLUMINANCE_ELEMENT, IDS_LUMINANCE_PARAMS, 0, 0, NULL,

	// params
	BaseRenderElement::enableOn, _T("enabledOn"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, FALSE,
		end,
	LuminanceRenderElement::kPBParam_atmosphereOn, _T("atmosphereOn"), TYPE_BOOL, 0, IDS_ATMOSPHERE_ON,
		p_default, TRUE,
		end,
	LuminanceRenderElement::kPBParam_shadowOn, _T("shadowOn"), TYPE_BOOL, 0, IDS_SHADOW_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default,"",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	LuminanceRenderElement::kPBParam_scaleFactor, _T("scaleFactor"), TYPE_FLOAT, 0, IDS_SCALEFACTOR,
		p_default, 1.0f,
		p_range, 0.0f, 1e12f,
		p_ui, TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_EDIT_LUMILLUM_SCALEFACTOR, IDC_SPIN_LUMILLUM_SCALEFACTOR, SPIN_AUTOSCALE,
		end,
	end);


LuminanceRenderElement::LuminanceRenderElement() 
: m_scaleFactorMult(0)
{
    lumCD.MakeAutoParamBlocks(this);
    DbgAssert(mpParamBlk);
    SetElementName( GetName());
}


RefTargetHandle LuminanceRenderElement::Clone( RemapDir &remap )
{
    LuminanceRenderElement*	mnew = new LuminanceRenderElement();
    mnew->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, mnew, remap);
    return (RefTargetHandle)mnew;
}

IRenderElementParamDlg* LuminanceRenderElement::CreateParamDialog(IRendParams *ip)
{
	return lumCD.CreateParamDialogs(ip, this);
}

void LuminanceRenderElement::Update(TimeValue t, Interval& valid) {

	// Cache the scale factor
	float scaleFactor;
	mpParamBlk->GetValue(kPBParam_scaleFactor, t, scaleFactor, valid);
	m_scaleFactorMult = scaleFactor;

	// Apply the conversion multiplier for the lighting units. The internal units
	// are always in metric system, so we need to convert to the system selected by the user.
	ILightingUnits* ls = static_cast<ILightingUnits*> (GetCOREInterface(ILIGHT_UNITS_FO_INTERFACE));
	if(ls != NULL) {
		m_scaleFactorMult = ls->ConvertLuminanceToCurrSystem(m_scaleFactorMult);
	}

	// Must also divide by PI.
	m_scaleFactorMult /= M_PI;
}

int LuminanceRenderElement::RenderBegin(TimeValue t, ULONG flags) {

	// This is in case the renderer doesn't call Update() on render elements.
	Update(t, FOREVER);

	return 1;
}

void LuminanceRenderElement::SetApplyAtmosphere( BOOL on )
{
    mpParamBlk->SetValue( kPBParam_atmosphereOn, 0, on );
}

BOOL LuminanceRenderElement::AtmosphereApplied() const
{
    int	on;
    mpParamBlk->GetValue( kPBParam_atmosphereOn, 0, on, FOREVER );
    return on;
}

void LuminanceRenderElement::SetApplyShadows( BOOL on )
{
    mpParamBlk->SetValue( kPBParam_shadowOn, 0, on );
}

BOOL LuminanceRenderElement::ShadowsApplied() const
{
    int	on;
    mpParamBlk->GetValue( kPBParam_shadowOn, 0, on, FOREVER );
    return on;
}

void LuminanceRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
    //not corrected for human eye sensitivity
    Color lum = ip.finalC;//this is the colour that hits my eye luminance.

	// Scale color from RGB to physical using either the tone operator or
	// the default physical scale
	if((sc.globContext != NULL) && (sc.globContext->pToneOp != NULL)) {
		sc.ScaleRGB(lum);
	}
	else {
		lum *= DEFAULT_MAX_TO_PHYSICAL_LIGHTING_SCALE;
	}

	// Apply the scale factor
	lum *= m_scaleFactorMult;
  
    sc.out.elementVals[ mShadeOutputIndex ] = lum;
}


/************************************************************************
/*	
/*	The Illuminance Render Element Stuff.
/*	
/************************************************************************/

class IlluminanceClassDesc:public ClassDesc2
{
public:
    int 			IsPublic() { return TRUE; }
    void *			Create(BOOL loading) { return new IlluminanceRenderElement; }
    const TCHAR *	ClassName() { return GetString(IDS_ILLUMINANCE_CLASSNAME); }
    SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
    Class_ID 		ClassID() { return ILLUM_CLASS_ID; }
    const TCHAR* 	Category() { return _T(""); }
    const TCHAR*	InternalName() { return _T("IlluminanceRenderElement"); } // hard-coded for scripter
    HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static IlluminanceClassDesc illumCD;
ClassDesc2& GetIlluminanceElementClassDesc() { 
	return illumCD; 
}

ParamBlockDesc2 IlluminanceRenderElement::m_paramBlockDesc(
	PBLOCK_NUMBER, _T("IlluminanceREParameters"), IDS_ILLUMINANCE_CLASSNAME, &illumCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,

	//rollout
	IDD_LUMINANCEILLUMINANCE_ELEMENT, IDS_ILLUMINANCE_PARAMS, 0, 0, NULL,

	// params
	BaseRenderElement::enableOn, _T("enabledOn"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, FALSE,
		end,
	IlluminanceRenderElement::kPBParam_atmosphereOn, _T("atmosphereOn"), TYPE_BOOL, 0, IDS_ATMOSPHERE_ON,
		p_default, TRUE,
		end,
	IlluminanceRenderElement::kPBParam_shadowOn, _T("shadowOn"), TYPE_BOOL, 0, IDS_SHADOW_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default,"",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	IlluminanceRenderElement::kPBParam_scaleFactor, _T("scaleFactor"), TYPE_FLOAT, 0, IDS_SCALEFACTOR,
		p_default, 1.0f,
		p_range, 0.0f, 1e12f,
		p_ui, TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_EDIT_LUMILLUM_SCALEFACTOR, IDC_SPIN_LUMILLUM_SCALEFACTOR, SPIN_AUTOSCALE,
		end,
	end);


IlluminanceRenderElement::IlluminanceRenderElement() 
: m_scaleFactorMult(0)
{
    illumCD.MakeAutoParamBlocks(this);
    DbgAssert(mpParamBlk);
    SetElementName( GetName());
}


RefTargetHandle IlluminanceRenderElement::Clone( RemapDir &remap )
{
    IlluminanceRenderElement*	mnew = new IlluminanceRenderElement();
    mnew->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, mnew, remap);
    return (RefTargetHandle)mnew;
}

IRenderElementParamDlg* IlluminanceRenderElement::CreateParamDialog(IRendParams *ip)
{
	return illumCD.CreateParamDialogs(ip, this);
}

void IlluminanceRenderElement::Update(TimeValue t, Interval& valid) {

	// Cache the scale factor
	float scaleFactor;
	mpParamBlk->GetValue(kPBParam_scaleFactor, t, scaleFactor, valid);
	m_scaleFactorMult = scaleFactor;

	// Apply the conversion multiplier for the lighting units. The internal units
	// are always in metric system, so we need to convert to the system selected by the user.
	ILightingUnits* ls = static_cast<ILightingUnits*> (GetCOREInterface(ILIGHT_UNITS_FO_INTERFACE));
	if(ls != NULL) {
		m_scaleFactorMult = ls->ConvertIlluminanceToCurrSystem(m_scaleFactorMult);
	}
}

int IlluminanceRenderElement::RenderBegin(TimeValue t, ULONG flags) {

	// This is in case the renderer doesn't call Update() on render elements.
	Update(t, FOREVER);

	return 1;
}

void IlluminanceRenderElement::SetApplyAtmosphere( BOOL on )
{
    mpParamBlk->SetValue( kPBParam_atmosphereOn, 0, on );
}

BOOL IlluminanceRenderElement::AtmosphereApplied() const
{
    int	on;
    mpParamBlk->GetValue( kPBParam_atmosphereOn, 0, on, FOREVER );
    return on;
}

void IlluminanceRenderElement::SetApplyShadows( BOOL on )
{
    mpParamBlk->SetValue( kPBParam_shadowOn, 0, on );
}

BOOL IlluminanceRenderElement::ShadowsApplied() const
{
    int	on;
    mpParamBlk->GetValue( kPBParam_shadowOn, 0, on, FOREVER );
    return on;
}

void IlluminanceRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
    Color illum = Color(0,0,0);
  
    // [dl | 1oct2001] Ignore background. (pMtl == NULL) => processing background.
    // This was causing a problem when this function was called on the background. The contents
    // of the ShadeContext (INode* and location) were still indicating the last location where
    // an actual hit took place. These cases need to be ignored or the color from the object would
    // leak to the background.
    if(ip.pMtl != NULL) {
        Color illumIncr;
        LightDesc* l;
        Point3 dummyDir;
        float dotnl;
        float dummyDiffuseCoef;
        for (int i=0; i<sc.nLights; i++) 
        {
            l = sc.Light(i);
            DbgAssert(l);
            if (l->Illuminate(sc, sc.Normal(), illumIncr, dummyDir,
					dotnl, dummyDiffuseCoef)) {
				// Don't add in illumIncr if the light doesn't illuminate the point.
				if ( !l->ambientOnly ) {
					if ( !l->affectDiffuse )
						continue;
					illumIncr *= dotnl;
				}
				illum += illumIncr;
			}
        }
    } 

    // Scale color from RGB to physical using either the tone operator or
	// the default physical scale
	if((sc.globContext != NULL) && (sc.globContext->pToneOp != NULL)) {
        sc.ScaleRGB(illum);
	}
	else {
		illum *= DEFAULT_MAX_TO_PHYSICAL_LIGHTING_SCALE;
	}
    
	// Apply the scale factor
	illum *= m_scaleFactorMult;

    sc.out.elementVals[ mShadeOutputIndex ] = illum; //illuminance.
} 
