/*==============================================================================

  file:	        LuminanceIlluminance.h

  author:       Daniel Levesque

  created:	    12 May 2005

  description:
        Render elements for rendering luminance and illuminance values.

        Adapted from the luminance & illuminance render elements used by the
		pseudo-color exposure control.


  modified:	


© 2005 Autodesk
==============================================================================*/

#ifndef _LUMINANCEILLUMINANCE_H_
#define _LUMINANCEILLUMINANCE_H_

class LuminanceRenderElement;
class IlluminanceRenderElement;

#include "StdRenElems.h"

extern ClassDesc2& GetLuminanceElementClassDesc();
extern ClassDesc2& GetIlluminanceElementClassDesc();

//Class ID for this Render Element
#define LUM_CLASS_ID	Class_ID(LUMINANCE_RENDER_ELEMENT_CLASS_ID, 0)

class LuminanceRenderElement: public BaseRenderElement {

public:
    
    LuminanceRenderElement();
    RefTargetHandle Clone( RemapDir &remap );
    
    Class_ID ClassID() {return LUM_CLASS_ID;};
    TSTR GetName() { return GetString( IDS_LUMINANCE_CLASSNAME ); }
    void GetClassName(TSTR& s) { s = GetName(); }

	IRenderElementParamDlg *CreateParamDialog(IRendParams *ip);
	virtual void Update(TimeValue t, Interval& valid);
	virtual int RenderBegin(TimeValue t, ULONG flags);
    
    void SetApplyAtmosphere(BOOL applyAtmosphere);
    BOOL AtmosphereApplied() const;
    void SetApplyShadows(BOOL applyShadows);
    BOOL ShadowsApplied() const;
    
    // the compute functions
    void PostIllum(ShadeContext& sc, IllumParams& ip);
    
    // called after atmospheres are computed, to allow elements to handle atmospheres
    void PostAtmosphere(ShadeContext& sc, float z, float zPrev){};

private:

	enum{
		kPBParam_atmosphereOn = BaseRenderElement::pbBitmap+1, 
		kPBParam_shadowOn,
		kPBParam_scaleFactor
	};

	static ParamBlockDesc2 m_paramBlockDesc;

	// Scale factor cached in the call to Update()
	float m_scaleFactorMult;
};


/************************************************************************
/*	
/*	Illumination version of the same
/*	
/************************************************************************/

#define ILLUM_CLASS_ID	Class_ID(ILLUMINANCE_RENDER_ELEMENT_CLASS_ID, 0)

class IlluminanceRenderElement: public BaseRenderElement  {

public:

    IlluminanceRenderElement();
    RefTargetHandle Clone( RemapDir &remap );
    
    Class_ID ClassID() {return ILLUM_CLASS_ID;};
    TSTR GetName() { return GetString( IDS_ILLUMINANCE_CLASSNAME ); }
    void GetClassName(TSTR& s) { s = GetName(); }

	IRenderElementParamDlg *CreateParamDialog(IRendParams *ip);
	virtual void Update(TimeValue t, Interval& valid);
	virtual int RenderBegin(TimeValue t, ULONG flags);
    
    void SetApplyAtmosphere(BOOL applyAtmosphere);
    BOOL AtmosphereApplied() const;    
    void SetApplyShadows(BOOL applyShadows);
    BOOL ShadowsApplied() const;
    
    // the compute functions
    void PostIllum(ShadeContext& sc, IllumParams& ip);
    
    // called after atmospheres are computed, to allow elements to handle atmospheres
    void PostAtmosphere(ShadeContext& sc, float z, float zPrev){};

private:

	enum{
		kPBParam_atmosphereOn = BaseRenderElement::pbBitmap+1, 
		kPBParam_shadowOn,
		kPBParam_scaleFactor
	};

	static ParamBlockDesc2 m_paramBlockDesc;

	// Scale factor cached in the call to Update()
	float m_scaleFactorMult;
};

#endif