////////////////////////////////////////////////////////////////////////
//
//	Standard TextureBake Render Elements 	
//
//	Created: Kells Elmquist, 2 December, 2001
//
//	Copyright (c) 2001, All Rights Reserved.
//
// local includes
#include "renElemPch.h"
#include "stdBakeElem.h"
#include "INodeBakeProperties.h"
#include "IProjection_WorkingModelInfo.h"

#ifndef	NO_RENDER_TO_TEXTURE

// Custom shader translation for mental ray
#include <mentalray\imrShaderTranslation.h>
#include <mentalray\imrShader.h>


Class_ID completeBakeElementClassID( COMPLETE_BAKE_ELEMENT_CLASS_ID , 0);
Class_ID specularBakeElementClassID( SPECULAR_BAKE_ELEMENT_CLASS_ID , 0);
Class_ID diffuseBakeElementClassID( DIFFUSE_BAKE_ELEMENT_CLASS_ID , 0);
//Class_ID emissionRenderElementClassID( EMISSION_BAKE_ELEMENT_CLASS_ID , 0);
Class_ID reflectRefractBakeElementClassID( REFLECT_REFRACT_BAKE_ELEMENT_CLASS_ID , 0);
Class_ID shadowBakeElementClassID( SHADOW_BAKE_ELEMENT_CLASS_ID , 0);
Class_ID blendBakeElementClassID( BLEND_BAKE_ELEMENT_CLASS_ID , 0);
Class_ID lightBakeElementClassID( LIGHT_BAKE_ELEMENT_CLASS_ID , 0);
Class_ID normalsBakeElementClassID( NORMALS_ELEMENT_CLASS_ID , 0);
Class_ID alphaBakeElementClassID( ALPHA_BAKE_ELEMENT_CLASS_ID , 0);
Class_ID heightBakeElementClassID( HEIGHT_BAKE_ELEMENT_CLASS_ID , 0);
#ifndef NO_AMBIENT_OCCLUSION
Class_ID ambientOcclusionBakeElementClassID( AMBIENTOCCLUSION_BAKE_ELEMENT_CLASS_ID , 0);
#endif //NO_AMBIENT_OCCLUSION

// Interface accessors
#define GetWorkingModelInfo( sc ) \
	((IProjection_WorkingModelInfo*)sc.GetInterface(IPROJECTION_WORKINGMODELINFO_INTERFACE_ID))
#define GetINodeBakeProperties(x) \
	((INodeBakeProperties*)(x)->GetInterface(NODE_BAKE_PROPERTIES_INTERFACE))
#define GetINodeBakeProjProperties(x) \
	((x)==NULL? NULL : (INodeBakeProjProperties*)(x)->GetInterface(NODE_BAKE_PROJ_PROPERTIES_INTERFACE))


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

#define CLAMP(x) ((x)<0? 0 : ((x)>1? 1 : (x)))

static TCHAR* shadowIllumOutStr = "shadowIllumOut";
static TCHAR* inkIllumOutStr = "Ink";
static TCHAR* paintIllumOutStr = "Paint";

namespace {

	bool GetParamIDByName(ParamID& paramID, const TCHAR* name, IParamBlock2* pBlock) {

		DbgAssert(pBlock != NULL);
		int count = pBlock->NumParams();
		for(int i = 0; i < count; ++i) {
			ParamID id = pBlock->IndextoID(i);
			const ParamDef& paramDef = pBlock->GetParamDef(id);
			if(_tcsicmp(name, paramDef.int_name) == 0) {
				paramID = id;
				return true;
			}
		}

		DbgAssert(false);
		return false;
	}

	template<typename T>
	void TranslateMRShaderParameter(IParamBlock2* paramBlock, TCHAR* paramName, T value) {

		ParamID paramID;
		if(GetParamIDByName(paramID, paramName, paramBlock)) {
			paramBlock->SetValue(paramID, 0, value);
		}
		else {
			DbgAssert(false);
		}
	}

	int ParamTypeToFPValueType( int t ) {
		switch( t ) {
		case 1:	return TYPE_BOOL;
		case 2:	return TYPE_INT;
		case 3:	return TYPE_FLOAT;
		case 4:	return TYPE_FRGBA_BV;
		}
		return TYPE_VOID;
	}

	int FPValueTypeToParamType( int t ) {
		switch( t ) {
		case TYPE_BOOL:		return 1;
		case TYPE_INT:		return 2;
		case TYPE_FLOAT:	return 3;
		case TYPE_FRGBA_BV:	return 4;
		}
		return 0;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//	Function publishing interface descriptor
//
FPInterfaceDesc BaseBakeElement::_fpInterfaceDesc(	
	BAKE_ELEMENT_INTERFACE,				// interface id
	_T("BakeElementProperties"),					// internal name
	0,											// res id of description string
	NULL,										// pointer to class desc of the publishing plugin
	FP_MIXIN + FP_CORE,							// flag bits; FP_CORE needed to allow maxscript to enumerate it 
												// (see src\dll\maxscrpt\maxclass.cpp - MAXSuperClass::show_interfaces)
	//item ID,  internal_name, description_string, return type, flags, num arguments
		//Paramter internal name,  description, type
	_nParams,		_T("nParams"), 0, TYPE_INT, 0, 0,
	_ParamName,	_T("paramName"), 0, TYPE_STRING, 0, 1,
		_T("paramIndex"), 0, TYPE_INT,
	_ParamType,	_T("paramType"), 0, TYPE_INT, 0, 1,
		_T("paramIndex"), 0, TYPE_INT,
	_GetParamValue,	_T("getParamValue"), 0, TYPE_INT, 0, 1,
		_T("paramIndex"), 0, TYPE_INT,
	_SetParamValue,	_T("setParamValue"), 0, TYPE_VOID, 0, 2,
		_T("paramIndex"), 0, TYPE_INT,
		_T("newValue"), 0, TYPE_INT,
	_GetParamFPValue,	_T("getParamFPValue"), 0, TYPE_FPVALUE_BV, 0, 1,
		_T("paramIndex"), 0, TYPE_INT,
	_SetParamFPValue,	_T("setParamFPValue"), 0, TYPE_VOID, 0, 2,
		_T("paramIndex"), 0, TYPE_INT,
		_T("newValue"), 0, TYPE_FPVALUE_BV,
	_GetParamFPValueMin, _T("getParamFPValueMin"), 0, TYPE_FPVALUE_BV, 0, 1,
		_T("paramIndex"), 0, TYPE_INT,
	_GetParamFPValueMax, _T("getParamFPValueMax"), 0, TYPE_FPVALUE_BV, 0, 1,
		_T("paramIndex"), 0, TYPE_INT,
	_FindParam,	_T("findParam"), 0, TYPE_INT, 0, 1,
		_T("paramName"), 0, TYPE_STRING,

	properties,
		_GetBackgroundColor, _SetBackgroundColor, _T("backgroundColor"), 0, TYPE_FRGBA_BV,

//	properties,
//		_GetFlags,  _SetFlags,  _T("flags"),  0, TYPE_INT,
	end 
);

FPInterfaceDesc* BaseBakeElement::GetDesc() {
	return & _fpInterfaceDesc; 
}

BaseInterface* BaseBakeElement::GetInterface(Interface_ID id) {

	if(id == BAKE_ELEMENT_INTERFACE) {
		return this;
	}
	else {
		return MaxRenderElement::GetInterface(id);
	}
}
///////////////////////////////////////////////////////////////////////////////
//
//	Diffuse render element
//
// parameters

class DiffuseBakeElement : public BaseBakeElement, public imrShaderTranslation {
public:
		enum{
			lightingOn = BaseBakeElement::fileType+1, // last element + 1
			shadowsOn,
			targetMapSlot
		};
		static int paramNames[];

		DiffuseBakeElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return diffuseBakeElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_DIFFUSE_BAKE_ELEMENT ); }

		IRenderElementParamDlg *CreateParamDialog(IRendParams *ip);

		// set/get element's light applied state
		void SetLightApplied(BOOL on){ 
			mpParamBlk->SetValue( lightingOn, 0, on ); 
		}
		BOOL IsLightApplied() const{
			int	on;
			mpParamBlk->GetValue( lightingOn, 0, on, FOREVER );
			return on;
		}

		// set/get element's shadow applied state
		void SetShadowApplied(BOOL on){
			mpParamBlk->SetValue( shadowsOn, 0, on ); 
		}
		BOOL IsShadowApplied() const{
			int	on;
			mpParamBlk->GetValue( shadowsOn, 0, on, FOREVER );
			return on;
		}

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);

		int  GetNParams() const { return 2; }
		// 1 based param numbering
		const TCHAR* GetParamName1( int nParam ) { 
			if( nParam > 0 && nParam <= GetNParams() )
				return GetString( paramNames[ nParam-1 ] );
			else 
				return NULL;
		}
		const int FindParamByName1( TCHAR*  name ) { 
			if( strcmp( name, GetString( paramNames[0] )) == 0 )
				return 1;
			if( strcmp( name, GetString( paramNames[1] )) == 0 )
				return 2;
			return 0; 
		}
		// currently only type 1 == boolean or 0 == undefined
		int  GetParamType1( int nParam ) { 
			return (nParam==1 || nParam==2)? 1 : 0;
		}

		int  GetParamValue1( int nParam ){
			if( nParam == 1 ) 
				return IsLightApplied();
			if( nParam == 2 ) 
				return IsShadowApplied();
			return -1;
		}
		void SetParamValue1( int nParam, int newVal ){
			if( nParam == 1 ) 
				SetLightApplied( newVal );
			else if( nParam == 2 ) 
				SetShadowApplied( newVal);
		}

		// -- from imrShaderTranslation
		virtual imrShader* GetMRShader(imrShaderCreation& shaderCreation);
		virtual void ReleaseMRShader();
		virtual bool NeedsAutomaticParamBlock2Translation();
		virtual void TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid);
		virtual void GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets);

		// -- from InterfaceServer
		virtual BaseInterface* GetInterface(Interface_ID id);
};

int DiffuseBakeElement::paramNames[] = { IDS_LIGHTING_ON, IDS_SHADOWS_ON };

// --------------------------------------------------
// Diffuse element class descriptor
// --------------------------------------------------
class DiffuseBakeElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { AddInterface(&BaseBakeElement::_fpInterfaceDesc); return new DiffuseBakeElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_DIFFUSE_BAKE_ELEMENT); }
	SClass_ID		SuperClassID() { return BAKE_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return diffuseBakeElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("DiffuseMap"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static DiffuseBakeElementClassDesc diffuseBakeCD;
#endif	// no_render_to_texture

ClassDesc* GetDiffuseBakeElementDesc()
{
#ifndef	NO_RENDER_TO_TEXTURE
	return &diffuseBakeCD;
#else	// no_render_to_texture
	return NULL;
#endif	// no_render_to_texture
}

#ifndef	NO_RENDER_TO_TEXTURE
IRenderElementParamDlg *DiffuseBakeElement::CreateParamDialog(IRendParams *ip)
{
	IRenderElementParamDlg * paramDlg = diffuseBakeCD.CreateParamDialogs(ip, this);
	SetLightApplied( IsLightApplied() );	//update controls
	return paramDlg;
}

class DiffusePBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		BaseBakeElement* m = (BaseBakeElement*)owner;
//		macroRecorder->Disable();
		switch (id)
		{
			case DiffuseBakeElement::lightingOn: {
				IParamMap2* map = m->GetMap();
				if ( map ) {
					map->Enable(DiffuseBakeElement::shadowsOn, v.i );
				}
			} break;
		}
//		macroRecorder->Enable();
	}
};

static DiffusePBAccessor diffusePBAccessor;


// ------------------------------------------------------
// Diffuse parameter block description - global instance
// ------------------------------------------------------
static ParamBlockDesc2 diffuseBakeParamBlk(PBLOCK_NUMBER, _T("Diffuse TextureBake Parameters"), 0, &diffuseBakeCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_LIGHT_SHADOW, IDS_DIFFUSE_BAKE_PARAMS, 0, 0, NULL,
	// params
	BaseBakeElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseBakeElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseBakeElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseBakeElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	BaseBakeElement::outputSzX, _T("outputSzX"), TYPE_INT, 0, IDS_OUTPUTSIZE_X,
		p_default, 256,
		end,
	BaseBakeElement::outputSzY, _T("outputSzY"), TYPE_INT, 0, IDS_OUTPUTSIZE_Y,
		p_default, 256,
		end,
	BaseBakeElement::autoSzOn, _T("autoSzOn"), TYPE_BOOL, 0, IDS_AUTOSIZE_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::fileName, _T("fileName"), TYPE_FILENAME, 0, IDS_FILE_NAME,
		p_default, "",
		end,
	BaseBakeElement::fileNameUnique, _T("filenameUnique"), TYPE_BOOL, 0, IDS_FILENAME_UNIQUE,
		p_default, FALSE,
		end,
	BaseBakeElement::fileType, _T("fileType"), TYPE_FILENAME, 0, IDS_FILE_TYPE,
		p_default, "",
		end,
	BaseBakeElement::backgroundColor, _T("backgroundColor"), TYPE_FRGBA, 0, IDS_BACKGROUNDCOLOR,
		p_default, AColor(0.0f, 0.0f, 0.0f, 0.0f),
		end,
	DiffuseBakeElement::lightingOn, _T("lightingOn"), TYPE_BOOL, 0, IDS_LIGHTING_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_LIGHTING_ON,
		p_default, FALSE,
		p_accessor,		&diffusePBAccessor,
		end,
	DiffuseBakeElement::shadowsOn, _T("shadowsOn"), TYPE_BOOL, 0, IDS_SHADOWS_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SHADOWS_ON,
		p_default, FALSE,
		end,
	DiffuseBakeElement::targetMapSlot, _T("targetMapSlotName"), TYPE_STRING, 0, IDS_TARGET_MAP_SLOT_NAME,
		p_default, "",
		end,
	end
	);

//--- Diffuse Render Element ------------------------------------------------
DiffuseBakeElement::DiffuseBakeElement()
{
	diffuseBakeCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
	mpRenderBitmap = NULL;
}


RefTargetHandle DiffuseBakeElement::Clone( RemapDir &remap )
{
	DiffuseBakeElement*	newEle = new DiffuseBakeElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}


void DiffuseBakeElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
		// not bgnd, do we want shaded or unshaded textures?
		if( IsLightApplied() ){
			// shading is on
			sc.out.elementVals[ mShadeOutputIndex ] = ip.finalAttenuation * (ip.diffIllumOut+ip.ambIllumOut);
			int n;
			if( (n = ip.FindUserIllumName( paintIllumOutStr ))>=0 )
				sc.out.elementVals[ mShadeOutputIndex ] += ip.GetUserIllumOutput( n );

			if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
					&& sc.globContext->pToneOp->Active(sc.CurTime())) {
				sc.globContext->pToneOp->ScaledToRGB(sc.out.elementVals[ mShadeOutputIndex ]);
			}

			float a = 1.0f - Intens( sc.out.t );
			a = CLAMP( a );
			sc.out.elementVals[ mShadeOutputIndex ].a = a;

		}// end, want shaded output
		else {
			// unshaded, raw-but-blended texture
			// LAM - 7/11/03 - 507044 - some mtls leave stdIDToChannel NULL, use black instead
			if (ip.stdIDToChannel) {
				sc.out.elementVals[ mShadeOutputIndex ] = ip.channels[ ip.stdIDToChannel[ ID_DI ]];

				float a = 1.0f - Intens( sc.out.t );
				a = CLAMP( a );
				sc.out.elementVals[ mShadeOutputIndex ].a = a;
			}
			else
				sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
		}
	}
}

imrShader* DiffuseBakeElement::GetMRShader(imrShaderCreation& shaderCreation) {

	imrShader* shader = shaderCreation.CreateShader(_T("max_DiffuseBakeElement"), ElementName());
	return shader;
}

void DiffuseBakeElement::ReleaseMRShader() {
	
	// Do nothing (we're not holding a reference to the shader)
}

bool DiffuseBakeElement::NeedsAutomaticParamBlock2Translation() {

	// All translation is manual
	return false;
}

void DiffuseBakeElement::TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid) {

	if(shader != NULL) {
		IParamBlock2* paramsPBlock = shader->GetParametersParamBlock();
		if(paramsPBlock != NULL) {
			TranslateMRShaderParameter(paramsPBlock, _T("applyAtmosphere"), AtmosphereApplied());
			TranslateMRShaderParameter(paramsPBlock, _T("applyShadows"), ShadowsApplied());
			TranslateMRShaderParameter(paramsPBlock, _T("applyLighting"), IsLightApplied());
		}
	}
}

void DiffuseBakeElement::GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets) {

	// No additional dependencies
}

BaseInterface* DiffuseBakeElement::GetInterface(Interface_ID id) {

	if(id == IMRSHADERTRANSLATION_INTERFACE_ID) {
		return static_cast<imrShaderTranslation*>(this);
	}
	else {
		return BaseBakeElement::GetInterface(id);
	}
}
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
//
//	Specular bake element
//
class SpecularBakeElement : public BaseBakeElement, public imrShaderTranslation {
public:
		enum{
			lightingOn = BaseBakeElement::fileType+1, // last element + 1
			shadowsOn,
			targetMapSlot
		};

		static int paramNames[];

		SpecularBakeElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return specularBakeElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_SPECULAR_BAKE_ELEMENT ); }

		BOOL AtmosphereApplied() const{ return TRUE; } //override, always on

		// set/get element's light applied state
		void SetLightApplied(BOOL on){ 
			mpParamBlk->SetValue( lightingOn, 0, on ); 
		}
		BOOL IsLightApplied() const{
			int	on;
			mpParamBlk->GetValue( lightingOn, 0, on, FOREVER );
			return on;
		}

		// set/get element's shadow applied state
		void SetShadowApplied(BOOL on){
			mpParamBlk->SetValue( shadowsOn, 0, on ); 
		}
		BOOL IsShadowApplied() const{
			int	on;
			mpParamBlk->GetValue( shadowsOn, 0, on, FOREVER );
			return on;
		}

		IRenderElementParamDlg *CreateParamDialog(IRendParams *ip);

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
//		void PostAtmosphere(ShadeContext& sc, float z, float zPrev);

		int  GetNParams() const { return 2; }
		// 1 based param numbering
		const TCHAR* GetParamName1( int nParam ) { 
			if( nParam > 0 && nParam <= GetNParams() )
				return GetString( paramNames[ nParam-1 ] );
			else 
				return NULL;
		}
		const int FindParamByName1( TCHAR*  name ) { 
			if( strcmp( name, GetString( paramNames[0] )) == 0 )
				return 1;
			if( strcmp( name, GetString( paramNames[1] )) == 0 )
				return 2;
			return 0; 
		}
		// currently only type 1 == boolean or 0 == undefined
		int  GetParamType1( int nParam ) { 
			return (nParam==1 || nParam==2)? 1 : 0;
		}

		int  GetParamValue1( int nParam ){
			if( nParam == 1 ) 
				return IsLightApplied();
			if( nParam == 2 ) 
				return IsShadowApplied();
			return -1;
		}
		void SetParamValue1( int nParam, int newVal ){
			if( nParam == 1 ) 
				SetLightApplied( newVal );
			else if( nParam == 2 ) 
				SetShadowApplied( newVal);
		}
		
		// -- from imrShaderTranslation
		virtual imrShader* GetMRShader(imrShaderCreation& shaderCreation);
		virtual void ReleaseMRShader();
		virtual bool NeedsAutomaticParamBlock2Translation();
		virtual void TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid);
		virtual void GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets);

		// -- from InterfaceServer
		virtual BaseInterface* GetInterface(Interface_ID id);
};

int SpecularBakeElement::paramNames[] = {IDS_LIGHTING_ON, IDS_SHADOWS_ON };


// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class SpecularBakeElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { AddInterface(&BaseBakeElement::_fpInterfaceDesc); return new SpecularBakeElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_SPECULAR_BAKE_ELEMENT); }
	SClass_ID		SuperClassID() { return BAKE_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return specularBakeElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("SpecularMap"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static SpecularBakeElementClassDesc specularBakeCD;
#endif	// no_render_to_texture

ClassDesc* GetSpecularBakeElementDesc()
{
#ifndef	NO_RENDER_TO_TEXTURE
	return &specularBakeCD;
#else	// no_render_to_texture
	return NULL;
#endif	// no_render_to_texture
}


#ifndef	NO_RENDER_TO_TEXTURE
IRenderElementParamDlg *SpecularBakeElement::CreateParamDialog(IRendParams *ip)
{
	IRenderElementParamDlg * paramDlg = specularBakeCD.CreateParamDialogs(ip, this);
	SetLightApplied( IsLightApplied() );	//update controls
	return paramDlg;
}

class SpecularPBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		BaseBakeElement* m = (BaseBakeElement*)owner;
//		macroRecorder->Disable();
		switch (id)
		{
			case SpecularBakeElement::lightingOn: {
				IParamMap2* map = m->GetMap();
				if ( map ) {
					map->Enable(SpecularBakeElement::shadowsOn, v.i );
				}
			} break;
		}
//		macroRecorder->Enable();
	}
};

static SpecularPBAccessor specularPBAccessor;



// ---------------------------------------------
// Specular parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 specularBakeParamBlk(PBLOCK_NUMBER, _T("Specular bake element parameters"), 0, &specularBakeCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_LIGHT_SHADOW, IDS_SPECULAR_BAKE_PARAMS, 0, 0, NULL,
	// params
	BaseBakeElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseBakeElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseBakeElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseBakeElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	BaseBakeElement::outputSzX, _T("outputSzX"), TYPE_INT, 0, IDS_OUTPUTSIZE_X,
		p_default, 256,
		end,
	BaseBakeElement::outputSzY, _T("outputSzY"), TYPE_INT, 0, IDS_OUTPUTSIZE_Y,
		p_default, 256,
		end,
	BaseBakeElement::autoSzOn, _T("autoSzOn"), TYPE_BOOL, 0, IDS_AUTOSIZE_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::fileName, _T("fileName"), TYPE_FILENAME, 0, IDS_FILE_NAME,
		p_default, "",
		end,
	BaseBakeElement::fileNameUnique, _T("filenameUnique"), TYPE_BOOL, 0, IDS_FILENAME_UNIQUE,
		p_default, FALSE,
		end,
	BaseBakeElement::fileType, _T("fileType"), TYPE_FILENAME, 0, IDS_FILE_TYPE,
		p_default, "",
		end,
	BaseBakeElement::backgroundColor, _T("backgroundColor"), TYPE_FRGBA, 0, IDS_BACKGROUNDCOLOR,
		p_default, AColor(0.0f, 0.0f, 0.0f, 0.0f),
		end,
	SpecularBakeElement::lightingOn, _T("lightingOn"), TYPE_BOOL, 0, IDS_LIGHTING_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_LIGHTING_ON,
		p_default, FALSE,
		p_accessor,		&specularPBAccessor,
		end,
	SpecularBakeElement::shadowsOn, _T("shadowsOn"), TYPE_BOOL, 0, IDS_SHADOWS_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SHADOWS_ON,
		p_default, FALSE,
		end,
	SpecularBakeElement::targetMapSlot, _T("targetMapSlotName"), TYPE_STRING, 0, IDS_TARGET_MAP_SLOT_NAME,
		p_default, "",
		end,
	end
	);

//--- Specular Render Element ------------------------------------------------
SpecularBakeElement::SpecularBakeElement()
{
	specularBakeCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
	mpRenderBitmap = NULL;
}


RefTargetHandle SpecularBakeElement::Clone( RemapDir &remap )
{
	SpecularBakeElement*	newEle = new SpecularBakeElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}


void SpecularBakeElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
		// not bgnd, do we want shaded or unshaded textures?
		if( IsLightApplied() ){
			// shading is on
			Color c = ip.specIllumOut;

			if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
					&& sc.globContext->pToneOp->Active(sc.CurTime())) {
				sc.globContext->pToneOp->ScaledToRGB(c);
			}

			ClampMax( c );
			sc.out.elementVals[ mShadeOutputIndex ] = c;

			// always want full alpha w/ bake elements, for dilation
			float a = 1.0f - Intens( sc.out.t );
			a = CLAMP( a );
			sc.out.elementVals[ mShadeOutputIndex ].a = a;
		}// end, want shaded output
		else {
			// unshaded, raw-but-blended texture
			// could select among: specular color map, shininess & glossiness
			// LAM - 7/11/03 - 507044 - some mtls leave stdIDToChannel NULL, use black instead
			if (ip.stdIDToChannel) {
				sc.out.elementVals[ mShadeOutputIndex ] =  ip.channels[ ip.stdIDToChannel[ ID_SS ]];
				// copy 
				sc.out.elementVals[ mShadeOutputIndex ].g = sc.out.elementVals[ mShadeOutputIndex ].b = sc.out.elementVals[ mShadeOutputIndex ].r;

				float a = 1.0f - Intens( sc.out.t );
				a = CLAMP( a );
				sc.out.elementVals[ mShadeOutputIndex ].a = a;
			}
			else
				sc.out.elementVals[ mShadeOutputIndex ] =  AColor(0,0,0,0);
		}
	} // end, not bgnd

}

imrShader* SpecularBakeElement::GetMRShader(imrShaderCreation& shaderCreation) {

	imrShader* shader = shaderCreation.CreateShader(_T("max_SpecularBakeElement"), ElementName());
	return shader;
}

void SpecularBakeElement::ReleaseMRShader() {
	
	// Do nothing (we're not holding a reference to the shader)
}

bool SpecularBakeElement::NeedsAutomaticParamBlock2Translation() {

	// All translation is manual
	return false;
}

void SpecularBakeElement::TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid) {

	if(shader != NULL) {
		IParamBlock2* paramsPBlock = shader->GetParametersParamBlock();
		if(paramsPBlock != NULL) {
			TranslateMRShaderParameter(paramsPBlock, _T("applyAtmosphere"), AtmosphereApplied());
			TranslateMRShaderParameter(paramsPBlock, _T("applyShadows"), ShadowsApplied());
			TranslateMRShaderParameter(paramsPBlock, _T("applyLighting"), IsLightApplied());
		}
	}
}

void SpecularBakeElement::GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets) {

	// No additional dependencies
}

BaseInterface* SpecularBakeElement::GetInterface(Interface_ID id) {

	if(id == IMRSHADERTRANSLATION_INTERFACE_ID) {
		return static_cast<imrShaderTranslation*>(this);
	}
	else {
		return BaseBakeElement::GetInterface(id);
	}
}
//-----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
//
//	Illuminance bake render element -- Light
//
class LightBakeElement : public BaseBakeElement, public imrShaderTranslation {
public:
		enum{
			shadowsOn = BaseBakeElement::fileType+1, // last element + 1
			directOn,
			indirectOn,
			targetMapSlot
		};

		static int paramNames[];

		LightBakeElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return lightBakeElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_LIGHT_BAKE_ELEMENT ); }

		BOOL IsLightApplied() const{ return TRUE; }

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

		IRenderElementParamDlg *CreateParamDialog(IRendParams *ip);

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
//		void PostAtmosphere(ShadeContext& sc, float z, float zPrev);

		int  GetNParams() const { return 3; }
		// 1 based param numbering
		const TCHAR* GetParamName1( int nParam ) { 
			if( nParam > 0 && nParam <= GetNParams() )
				return GetString( paramNames[ nParam-1 ] );
			else 
				return NULL;
		}
		const int FindParamByName1( TCHAR*  name ) { 
			for( int i = 0; i < GetNParams(); ++i ){
				if( strcmp( name, GetString( paramNames[i] ) ) == 0 )
					return i+1;
			}
			return 0; 
		}
		// currently only type 1 == boolean or 0 == undefined
		int  GetParamType1( int nParam ) { 
			return (nParam > 0 && nParam <= GetNParams())? 1 : 0;
		}

		int  GetParamValue1( int nParam ){
			switch( nParam ) {
				case 1: return IsShadowApplied();
				case 2: return IsDirectLightOn();
				case 3: return IsIndirectLightOn();
				default: return -1;
			}
		}
		void SetParamValue1( int nParam, int newVal ){
			switch( nParam ) {
				case 1: SetShadowApplied( newVal ); break;
				case 2: SetDirectLightOn( newVal ); break;
				case 3: SetIndirectLightOn( newVal ); break;
			}
		}
		
		// -- from imrShaderTranslation
		virtual imrShader* GetMRShader(imrShaderCreation& shaderCreation);
		virtual void ReleaseMRShader();
		virtual bool NeedsAutomaticParamBlock2Translation();
		virtual void TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid);
		virtual void GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets);

		// -- from InterfaceServer
		virtual BaseInterface* GetInterface(Interface_ID id);

};

int LightBakeElement::paramNames[] = { IDS_SHADOWS_ON, IDS_DIRECT_ILLUM_ON, IDS_INDIRECT_ILLUM_ON };

// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class LightBakeElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { AddInterface(&BaseBakeElement::_fpInterfaceDesc); return new LightBakeElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_LIGHT_BAKE_ELEMENT); }
	SClass_ID		SuperClassID() { return BAKE_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return lightBakeElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("lightMap"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static LightBakeElementClassDesc lightBakeCD;
#endif	// no_render_to_texture

ClassDesc* GetLightBakeElementDesc()
{
#ifndef	NO_RENDER_TO_TEXTURE
	return &lightBakeCD;
#else	// no_render_to_texture
	return NULL;
#endif	// no_render_to_texture
}

#ifndef	NO_RENDER_TO_TEXTURE
IRenderElementParamDlg *LightBakeElement::CreateParamDialog(IRendParams *ip)
{
	return (IRenderElementParamDlg *)lightBakeCD.CreateParamDialogs(ip, this);
}


// ---------------------------------------------
// Illuminance parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 lightBakeParamBlk(PBLOCK_NUMBER, _T("Lighting bake element parameters"), 0, &lightBakeCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_LIGHT_BAKE, IDS_LIGHT_BAKE_PARAMS, 0, 0, NULL,
	// params
	BaseBakeElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseBakeElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseBakeElement::outputSzX, _T("outputSzX"), TYPE_INT, 0, IDS_OUTPUTSIZE_X,
		p_default, 256,
		end,
	BaseBakeElement::outputSzY, _T("outputSzY"), TYPE_INT, 0, IDS_OUTPUTSIZE_Y,
		p_default, 256,
		end,
	BaseBakeElement::autoSzOn, _T("autoSzOn"), TYPE_BOOL, 0, IDS_AUTOSIZE_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::fileName, _T("fileName"), TYPE_FILENAME, 0, IDS_FILE_NAME,
		p_default, "",
		end,
	BaseBakeElement::fileNameUnique, _T("filenameUnique"), TYPE_BOOL, 0, IDS_FILENAME_UNIQUE,
		p_default, FALSE,
		end,
	BaseBakeElement::fileType, _T("fileType"), TYPE_FILENAME, 0, IDS_FILE_TYPE,
		p_default, "",
		end,
	BaseBakeElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseBakeElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	BaseBakeElement::backgroundColor, _T("backgroundColor"), TYPE_FRGBA, 0, IDS_BACKGROUNDCOLOR,
		p_default, AColor(0.0f, 0.0f, 0.0f, 0.0f),
		end,
	LightBakeElement::shadowsOn, _T("shadowsOn"), TYPE_BOOL, 0, IDS_SHADOWS_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SHADOWS_ON,
		p_default, TRUE,
		end,
	LightBakeElement::directOn, _T("directOn"), TYPE_BOOL, 0, IDS_DIRECT_ILLUM_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_DIRECT_ON,
		p_default, TRUE,
		end,
	LightBakeElement::indirectOn, _T("indirectOn"), TYPE_BOOL, 0, IDS_INDIRECT_ILLUM_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_INDIRECT_ON,
		p_default, TRUE,
		end,
	LightBakeElement::targetMapSlot, _T("targetMapSlotName"), TYPE_STRING, 0, IDS_TARGET_MAP_SLOT_NAME,
		p_default, "",
		end,
	end
	);

//--- Illuminance Render Element ------------------------------------------------
LightBakeElement::LightBakeElement()
{
	lightBakeCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
	mpRenderBitmap = NULL;
}


RefTargetHandle LightBakeElement::Clone( RemapDir &remap )
{
	LightBakeElement*	newEle = new LightBakeElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}


void LightBakeElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL || ! IsLightApplied() ){ // bgnd or no lighting = black
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
		BOOL shadowSave = sc.shadow;
		sc.shadow = IsShadowApplied() ? TRUE : FALSE;

		if (IsIndirectLightOn() || IsDirectLightOn()) {
			sc.out.elementVals[ mShadeOutputIndex ] = computeIlluminance( sc, IsIndirectLightOn(), IsDirectLightOn() );

			if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
				&& sc.globContext->pToneOp->Active(sc.CurTime())) {
					sc.globContext->pToneOp->ScaledToRGB(sc.out.elementVals[ mShadeOutputIndex ]);
			}

			float a = 1.0f - Intens( sc.out.t );
			a = CLAMP( a );
			sc.out.elementVals[ mShadeOutputIndex ].a = a;

		}
		else
			sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);

		sc.shadow = shadowSave; // restore shadow
	}
}

imrShader* LightBakeElement::GetMRShader(imrShaderCreation& shaderCreation) {

	imrShader* shader = shaderCreation.CreateShader(_T("max_LightBakeElement"), ElementName());
	return shader;
}

void LightBakeElement::ReleaseMRShader() {
	
	// Do nothing (we're not holding a reference to the shader)
}

bool LightBakeElement::NeedsAutomaticParamBlock2Translation() {

	// All translation is manual
	return false;
}

void LightBakeElement::TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid) {

	if(shader != NULL) {
		IParamBlock2* paramsPBlock = shader->GetParametersParamBlock();
		if(paramsPBlock != NULL) {
			TranslateMRShaderParameter(paramsPBlock, _T("applyAtmosphere"), AtmosphereApplied());
			TranslateMRShaderParameter(paramsPBlock, _T("applyShadows"), ShadowsApplied());			
			TranslateMRShaderParameter(paramsPBlock, _T("isDirectOn"), IsDirectLightOn());	
			TranslateMRShaderParameter(paramsPBlock, _T("isIndirectOn"), IsIndirectLightOn());	
		}
	}
}

void LightBakeElement::GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets) {

	// No additional dependencies
}

BaseInterface* LightBakeElement::GetInterface(Interface_ID id) {

	if(id == IMRSHADERTRANSLATION_INTERFACE_ID) {
		return static_cast<imrShaderTranslation*>(this);
	}
	else {
		return BaseBakeElement::GetInterface(id);
	}
}

//-----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
//
//	Reflect / Refract bake render element
//
class ReflectRefractBakeElement : public BaseBakeElement, public imrShaderTranslation {
public:
		enum{
			targetMapSlot = BaseBakeElement::fileType+1, // last element + 1
		};
		ReflectRefractBakeElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return reflectRefractBakeElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_REFLECTREFRACT_BAKE_ELEMENT ); }

		BOOL IsLightApplied() const{ return TRUE; }
		BOOL IsShadowApplied() const{ return TRUE; }

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
//		void PostAtmosphere(ShadeContext& sc, float z, float zPrev);
		
		// -- from imrShaderTranslation
		virtual imrShader* GetMRShader(imrShaderCreation& shaderCreation);
		virtual void ReleaseMRShader();
		virtual bool NeedsAutomaticParamBlock2Translation();
		virtual void TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid);
		virtual void GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets);

		// -- from InterfaceServer
		virtual BaseInterface* GetInterface(Interface_ID id);
};

// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class ReflectRefractBakeElementClassDesc : public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { AddInterface(&BaseBakeElement::_fpInterfaceDesc); return new ReflectRefractBakeElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_REFLECTREFRACT_BAKE_ELEMENT); }
	SClass_ID		SuperClassID() { return BAKE_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return reflectRefractBakeElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("reflectRefractMap"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static ReflectRefractBakeElementClassDesc reflectRefractBakeCD;
#endif	// no_render_to_texture

ClassDesc* GetReflectRefractBakeElementDesc()
{
#ifndef	NO_RENDER_TO_TEXTURE
	return &reflectRefractBakeCD;
#else	// no_render_to_texture
	return NULL;
#endif	// no_render_to_texture
}


#ifndef	NO_RENDER_TO_TEXTURE
// ---------------------------------------------
// Reflect-Refract parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 reflectRefractBakeParamBlk(PBLOCK_NUMBER, _T("Reflect/Refract bake element parameters"), 0, &reflectRefractBakeCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseBakeElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseBakeElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseBakeElement::outputSzX, _T("outputSzX"), TYPE_INT, 0, IDS_OUTPUTSIZE_X,
		p_default, 256,
		end,
	BaseBakeElement::outputSzY, _T("outputSzY"), TYPE_INT, 0, IDS_OUTPUTSIZE_Y,
		p_default, 256,
		end,
	BaseBakeElement::autoSzOn, _T("autoSzOn"), TYPE_BOOL, 0, IDS_AUTOSIZE_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::fileName, _T("fileName"), TYPE_FILENAME, 0, IDS_FILE_NAME,
		p_default, "",
		end,
	BaseBakeElement::fileNameUnique, _T("filenameUnique"), TYPE_BOOL, 0, IDS_FILENAME_UNIQUE,
		p_default, FALSE,
		end,
	BaseBakeElement::fileType, _T("fileType"), TYPE_FILENAME, 0, IDS_FILE_TYPE,
		p_default, "",
		end,
	BaseBakeElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseBakeElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	BaseBakeElement::backgroundColor, _T("backgroundColor"), TYPE_FRGBA, 0, IDS_BACKGROUNDCOLOR,
		p_default, AColor(0.0f, 0.0f, 0.0f, 0.0f),
		end,
	ReflectRefractBakeElement::targetMapSlot, _T("targetMapSlotName"), TYPE_STRING, 0, IDS_TARGET_MAP_SLOT_NAME,
		p_default, "",
		end,
	end
	);


//--- Reflect / Refract Bake Element ------------------------------------------------
ReflectRefractBakeElement::ReflectRefractBakeElement()
{
	reflectRefractBakeCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
	mpRenderBitmap = NULL;
}


RefTargetHandle ReflectRefractBakeElement::Clone( RemapDir &remap )
{
	ReflectRefractBakeElement*	newEle = new ReflectRefractBakeElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}


void ReflectRefractBakeElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
		// reflection
 		Color c = ip.reflIllumOut;

		if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
				&& sc.globContext->pToneOp->Active(sc.CurTime())) {
			sc.globContext->pToneOp->ScaledToRGB(c);
		}

		ClampMax( c );
		sc.out.elementVals[ mShadeOutputIndex ] = c;

		// add in refraction
 		c = ip.transIllumOut;

		if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
				&& sc.globContext->pToneOp->Active(sc.CurTime())) {
			sc.globContext->pToneOp->ScaledToRGB(c);
		}

		ClampMax( c );
		sc.out.elementVals[ mShadeOutputIndex ] += c;

		float a = 1.0f - Intens( sc.out.t );
		a = CLAMP( a );
		sc.out.elementVals[ mShadeOutputIndex ].a = a;
	}
}

imrShader* ReflectRefractBakeElement::GetMRShader(imrShaderCreation& shaderCreation) {

	imrShader* shader = shaderCreation.CreateShader(_T("max_ReflectRefractBakeElement"), ElementName());
	return shader;
}

void ReflectRefractBakeElement::ReleaseMRShader() {
	
	// Do nothing (we're not holding a reference to the shader)
}

bool ReflectRefractBakeElement::NeedsAutomaticParamBlock2Translation() {

	// All translation is manual
	return false;
}

void ReflectRefractBakeElement::TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid) {

	if(shader != NULL) {
		IParamBlock2* paramsPBlock = shader->GetParametersParamBlock();
		if(paramsPBlock != NULL) {
			TranslateMRShaderParameter(paramsPBlock, _T("applyAtmosphere"), AtmosphereApplied());
			TranslateMRShaderParameter(paramsPBlock, _T("applyShadows"), ShadowsApplied());
		}
	}
}

void ReflectRefractBakeElement::GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets) {

	// No additional dependencies
}

BaseInterface* ReflectRefractBakeElement::GetInterface(Interface_ID id) {

	if(id == IMRSHADERTRANSLATION_INTERFACE_ID) {
		return static_cast<imrShaderTranslation*>(this);
	}
	else {
		return BaseBakeElement::GetInterface(id);
	}
}
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
//
//	Shadow baking element - high res, no color, alpha only
//
class ShadowBakeElement : public BaseBakeElement, public imrShaderTranslation {
public:
		enum{
			targetMapSlot = BaseBakeElement::fileType+1, // last element + 1
		};
		ShadowBakeElement();
		RefTargetHandle Clone( RemapDir &remap );

		// note, we don't override the ShadowsApplied(){ return FALSE; }
		// we control it ourselves

		Class_ID ClassID() {return shadowBakeElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_SHADOW_BAKE_ELEMENT ); }

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);

		BOOL IsLightApplied() const{ return TRUE; }
		BOOL IsShadowApplied() const{ return TRUE; }

		// -- from imrShaderTranslation
		virtual imrShader* GetMRShader(imrShaderCreation& shaderCreation);
		virtual void ReleaseMRShader();
		virtual bool NeedsAutomaticParamBlock2Translation();
		virtual void TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid);
		virtual void GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets);

		// -- from InterfaceServer
		virtual BaseInterface* GetInterface(Interface_ID id);
};

// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class ShadowBakeElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { AddInterface(&BaseBakeElement::_fpInterfaceDesc); return new ShadowBakeElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_SHADOW_BAKE_ELEMENT); }
	SClass_ID		SuperClassID() { return BAKE_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return shadowBakeElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("shadowsMap"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static ShadowBakeElementClassDesc shadowBakeCD;
#endif	// no_render_to_texture

ClassDesc* GetShadowBakeElementDesc()
{
#ifndef	NO_RENDER_TO_TEXTURE
	return &shadowBakeCD;
#else	// no_render_to_texture
	return NULL;
#endif	// no_render_to_texture
}


#ifndef	NO_RENDER_TO_TEXTURE
// ---------------------------------------------
// Shadow parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 shadowBakeParamBlk(PBLOCK_NUMBER, _T("Shadow render element parameters"), 0, &shadowBakeCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseBakeElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseBakeElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseBakeElement::outputSzX, _T("outputSzX"), TYPE_INT, 0, IDS_OUTPUTSIZE_X,
		p_default, 256,
		end,
	BaseBakeElement::outputSzY, _T("outputSzY"), TYPE_INT, 0, IDS_OUTPUTSIZE_Y,
		p_default, 256,
		end,
	BaseBakeElement::autoSzOn, _T("autoSzOn"), TYPE_BOOL, 0, IDS_AUTOSIZE_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::fileName, _T("fileName"), TYPE_FILENAME, 0, IDS_FILE_NAME,
		p_default, "",
		end,
	BaseBakeElement::fileNameUnique, _T("filenameUnique"), TYPE_BOOL, 0, IDS_FILENAME_UNIQUE,
		p_default, FALSE,
		end,
	BaseBakeElement::fileType, _T("fileType"), TYPE_FILENAME, 0, IDS_FILE_TYPE,
		p_default, "",
		end,
	BaseBakeElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseBakeElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	BaseBakeElement::backgroundColor, _T("backgroundColor"), TYPE_FRGBA, 0, IDS_BACKGROUNDCOLOR,
		p_default, AColor(0.0f, 0.0f, 0.0f, 0.0f),
		end,
	ShadowBakeElement::targetMapSlot, _T("targetMapSlotName"), TYPE_STRING, 0, IDS_TARGET_MAP_SLOT_NAME,
		p_default, "",
		end,
	end
	);

//--- Shadow Bake Element ------------------------------------------------
ShadowBakeElement::ShadowBakeElement()
{
	shadowBakeCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
	mpRenderBitmap = NULL;
}


RefTargetHandle ShadowBakeElement::Clone( RemapDir &remap )
{
	ShadowBakeElement* newEle = new ShadowBakeElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}

inline void Clamp( float& val ){ 
	if( val < 0.0f ) val = 0.0f;
	else if( val > 1.0f ) val = 1.0f;
}


void ShadowBakeElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
		Color sClr;
		float shadowAtten = IllumShadow( sc, sClr );
		float a = 1.0f-shadowAtten;

		float opac = 1.0f - Intens( sc.out.t );
		opac = CLAMP( opac );

		if(ip.pMtl->ClassID() != Class_ID(MATTE_CLASS_ID,0) )
			a *= opac;

		float a1 = 1.0f - a;

		a1 = CLAMP( a1 );

		sc.out.elementVals[ mShadeOutputIndex ]	= AColor( a1, a1, a1, opac );
	}
}

imrShader* ShadowBakeElement::GetMRShader(imrShaderCreation& shaderCreation) {

	imrShader* shader = shaderCreation.CreateShader(_T("max_ShadowBakeElement"), ElementName());
	return shader;
}

void ShadowBakeElement::ReleaseMRShader() {
	
	// Do nothing (we're not holding a reference to the shader)
}

bool ShadowBakeElement::NeedsAutomaticParamBlock2Translation() {

	// All translation is manual
	return false;
}

void ShadowBakeElement::TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid) {

	if(shader != NULL) {
		IParamBlock2* paramsPBlock = shader->GetParametersParamBlock();
		if(paramsPBlock != NULL) {
			TranslateMRShaderParameter(paramsPBlock, _T("applyAtmosphere"), AtmosphereApplied());
			TranslateMRShaderParameter(paramsPBlock, _T("applyShadows"), ShadowsApplied());
		}
	}
}

void ShadowBakeElement::GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets) {

	// No additional dependencies
}

BaseInterface* ShadowBakeElement::GetInterface(Interface_ID id) {

	if(id == IMRSHADERTRANSLATION_INTERFACE_ID) {
		return static_cast<imrShaderTranslation*>(this);
	}
	else {
		return BaseBakeElement::GetInterface(id);
	}
}
//-----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
//
//	Normals Bake element
//		- texture bake the normals
//
class NormalsBakeElement : public BaseBakeElement, public imrShaderTranslation {
	public:
		enum{
			targetMapSlot = BaseBakeElement::fileType+1, // last element + 1
			useNormalBump,
			useHeightAsAlpha,
		};
		static int paramNames[];

		NormalsBakeElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return normalsBakeElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_NORMALS_BAKE_ELEMENT ); }

		BOOL IsLightApplied() const{ return FALSE; }

		void Init( ShadeContext& sc );

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);

		int  GetNParams() const { return 2; }
		// 1 based param numbering
		const TCHAR* GetParamName1( int nParam ) { 
			if( nParam>0 && nParam<=GetNParams() ) return GetString( paramNames[ nParam-1 ] );
			else return NULL;
		}
		const int FindParamByName1( TCHAR*  name ) { 
			if( strcmp( name, GetString( paramNames[0] )) == 0 ) return 1;
			if( strcmp( name, GetString( paramNames[1] )) == 0 ) return 2;
			return 0; 
		}
		// currently only type 1 == boolean or 0 == undefined
		int  GetParamType1( int nParam ) { 
			return (nParam>0 && nParam<=GetNParams())? 1 : 0;
		}

		int  GetParamValue1( int nParam ){
			if( nParam == 1 ) return mpParamBlk->GetInt(useNormalBump);
			if( nParam == 2 ) return mpParamBlk->GetInt(useHeightAsAlpha);
			return -1;
		}
		void SetParamValue1( int nParam, int newVal ){
			if( nParam == 1 )		mpParamBlk->SetValue(useNormalBump,0,newVal);
			else if( nParam == 2 )	mpParamBlk->SetValue(useHeightAsAlpha,0,newVal);
		}

		// Helper method
		float GetProjectionHeight( ShadeContext& sc, IProjection_WorkingModelInfo* workingModelInfo );

		// -- from InterfaceServer
		virtual BaseInterface* GetInterface(Interface_ID id);

		// -- from Animatable
		int RenderBegin(TimeValue t, ULONG flags=0);
		int RenderEnd(TimeValue t) {return 0;}

		// -- from imrShaderTranslation
		virtual imrShader* GetMRShader(imrShaderCreation& shaderCreation);
		virtual void ReleaseMRShader();
		virtual bool NeedsAutomaticParamBlock2Translation();
		virtual void TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid);
		virtual void GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets);

	protected:
		BOOL render_init, render_projectionOn, render_useHeightAsAlpha;
		BOOL render_tangentYDown, render_tangentXLeft;
		int render_mapChannel, render_normalSpace;
		float render_heightMin, render_heightMax, render_heightMult;
};

int NormalsBakeElement::paramNames[] = { IDS_USENORMALBUMP, IDS_USEHEIGHTASALPHA };


// --------------------------------------------------
//texture bake the normals class descriptor 
class NormalsBakeElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() {
#ifdef NORMAL_MAPPING_RTT_BAKE_ELEMENT 
		return 1;
#else 
		return 0 ;
#endif
	}


	void *			Create(BOOL loading) { AddInterface(&BaseBakeElement::_fpInterfaceDesc); return new NormalsBakeElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_NORMALS_BAKE_ELEMENT); }
	SClass_ID		SuperClassID() { return BAKE_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return normalsBakeElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("normalsMap"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static NormalsBakeElementClassDesc normalsBakeCD;
#endif	// no_render_to_texture

ClassDesc* GetNormalsBakeElementDesc()
{
#ifndef	NO_RENDER_TO_TEXTURE
	return &normalsBakeCD;
#else	// no_render_to_texture
	return NULL;
#endif	// no_render_to_texture
}


#ifndef	NO_RENDER_TO_TEXTURE
// ---------------------------------------------
// normal map baker parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 normalsBakeParamBlk(
	PBLOCK_NUMBER, _T("Normal map bake element parameters"), 0, &normalsBakeCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseBakeElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseBakeElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::outputSzX, _T("outputSzX"), TYPE_INT, 0, IDS_OUTPUTSIZE_X,
		p_default, 256,
		end,
	BaseBakeElement::outputSzY, _T("outputSzY"), TYPE_INT, 0, IDS_OUTPUTSIZE_Y,
		p_default, 256,
		end,
	BaseBakeElement::autoSzOn, _T("autoSzOn"), TYPE_BOOL, 0, IDS_AUTOSIZE_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::fileName, _T("fileName"), TYPE_FILENAME, 0, IDS_FILE_NAME,
		p_default, "",
		end,
	BaseBakeElement::fileNameUnique, _T("filenameUnique"), TYPE_BOOL, 0, IDS_FILENAME_UNIQUE,
		p_default, FALSE,
		end,
	BaseBakeElement::fileType, _T("fileType"), TYPE_FILENAME, 0, IDS_FILE_TYPE,
		p_default, "",
		end,
	BaseBakeElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseBakeElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	BaseBakeElement::backgroundColor, _T("backgroundColor"), TYPE_FRGBA, 0, IDS_BACKGROUNDCOLOR,
		p_default, AColor(128.0f/255.0f, 128.0f/255.0f, 1.0f, 0.0f),
		end,
	NormalsBakeElement::targetMapSlot, _T("targetMapSlotName"), TYPE_STRING, 0, IDS_TARGET_MAP_SLOT_NAME,
		p_default, "",
		end,
	NormalsBakeElement::useNormalBump, _T("useNormalBump"), TYPE_BOOL, 0, IDS_USE_NORMAL_BUMP,
		p_default, FALSE,
		end,
	NormalsBakeElement::useHeightAsAlpha, _T("useHeightAsAlpha"), TYPE_BOOL, 0, IDS_USE_HEIGHT_AS_ALPHA,
		p_default, FALSE,
		end,
	end
	);


//--- Normal Map Baking Element ------------------------------------------------
NormalsBakeElement::NormalsBakeElement()
{
	normalsBakeCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
	mpRenderBitmap = NULL;
}


RefTargetHandle NormalsBakeElement::Clone( RemapDir &remap )
{
	NormalsBakeElement* newEle = new NormalsBakeElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}

void NormalsBakeElement::Init( ShadeContext& sc ) {
	render_useHeightAsAlpha = mpParamBlk->GetInt(useHeightAsAlpha);

	IProjection_WorkingModelInfo* workingModelInfo = GetWorkingModelInfo(sc);
	INode* node;
	if( workingModelInfo==NULL ) {
		render_projectionOn = FALSE;
		node = sc.Node();
	}
	else {
		RenderInstance* workingModelInst = workingModelInfo->GetRenderInstance();
		node = workingModelInst->GetINode();
	}

	INodeBakeProjProperties* projProps = GetINodeBakeProjProperties( node );
	if( projProps==NULL )
		render_projectionOn =	projProps->GetEnabled();
	else {
		render_projectionOn =	projProps->GetEnabled();
		render_normalSpace =	projProps->GetNormalSpace();
		render_tangentYDown =	(projProps->GetTangentYDir() == INodeBakeProjProperties::enumIdTangentDirYDown? TRUE:FALSE);
		render_tangentXLeft =	(projProps->GetTangentXDir() == INodeBakeProjProperties::enumIdTangentDirXLeft? TRUE:FALSE);
		render_heightMin =		projProps->GetHeightMapMin();
		render_heightMax =		projProps->GetHeightMapMax();
		// FIXME: what happens if render_heightMin > render_heightMax ?
		render_heightMult =		1.0f / (render_heightMax - render_heightMin);

		INodeBakeProperties* bakeProps = GetINodeBakeProperties( node );
		render_mapChannel =		bakeProps->GetBakeMapChannel();
	}
	render_init = TRUE;
}

int NormalsBakeElement::RenderBegin(TimeValue t, ULONG flags) {
	render_init = FALSE; //to be initialized on the first PostIllum call
	return 0;
}

void NormalsBakeElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ) { // background
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
		return;
	}
	
	if( !render_init ) Init( sc );

	if( !render_projectionOn ) { //normals, no projection
		Point3 delta = sc.OrigNormal() - sc.Normal();
		Point3 N(0.0,0.0,1.0);
		N = N +delta;
		N = N.Normalize();
		N = N + 1.0;
		N *= 0.5;

		float a = 1.0f - Intens( sc.out.t );
		a = CLAMP( a );
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(N.x, N.y, N.z, a);
	}
	else { // normal mapping with projection
		IProjection_WorkingModelInfo* workingModelInfo = GetWorkingModelInfo(sc);
		RenderInstance* workingModelInst = workingModelInfo->GetRenderInstance();
		Point3 normal = sc.Normal();

		switch( render_normalSpace ) {
		case INodeBakeProjProperties::enumIdNormalSpaceWorld:
			normal = sc.VectorTo( normal, REF_WORLD );	//camera to world space
			break;
		case INodeBakeProjProperties::enumIdNormalSpaceLocal:
			normal = VectorTransform( normal, workingModelInst->camToObj );
				sc.VectorTo( normal, REF_OBJECT );	//camera to object space
			break;
		case INodeBakeProjProperties::enumIdNormalSpaceScreen:
			//normal already in screen space	
			break;
		default: //default is tangent space
			Matrix3 basis;
			Point3* t = (Point3*)(basis.GetAddr());
			workingModelInfo->TangentBasisVectors( t, render_mapChannel  );
			t[2] = workingModelInfo->GetOrigNormal();
			if( workingModelInfo->GetBackFace() ) //FIXME: normal being reported as flipped for backfaces
				t[2].x = -t[2].x, t[2].y = -t[2].y, t[2].z = -t[2].z;
			basis.ClearIdentFlag(MAT_IDENT); //FIXME: performance penealty
			basis.Invert();

			//convert the normal to tangent space using the texture basis vectors
			normal = VectorTransform( basis, normal );
		}

		if( render_tangentXLeft ) normal.x = -(normal.x);
		if( render_tangentYDown ) normal.y = -(normal.y);

		normal = normal.Normalize();
		normal = normal + 1.0f; //convert the normal to a color representation...
		normal *= 0.5f;	//...as color, 0.5 means zero, 0 means -1, and 1 means +1 for each coord

		AColor retVal(0,0,0,1);
		retVal.r = normal.x, retVal.g = normal.y, retVal.b = normal.z;

		float a;
		if( render_useHeightAsAlpha )
			 a = GetProjectionHeight( sc, workingModelInfo );
		else a = 1.0f - Intens( sc.out.t );

		retVal.a = CLAMP(a);
		sc.out.elementVals[ mShadeOutputIndex ] = retVal;
	}
}

// Helper function
float NormalsBakeElement::GetProjectionHeight( ShadeContext& sc, IProjection_WorkingModelInfo* workingModelInfo ) {
	Point3 pointA = sc.PointTo( workingModelInfo->GetPoint(), REF_WORLD );
	Point3 pointB = sc.PointTo( sc.P(), REF_WORLD );
	Point3 normal = sc.VectorTo( workingModelInfo->GetOrigNormal(), REF_WORLD ).Normalize();
	float c, height = DotProd( normal, (pointB - pointA) );

	if( render_heightMult==0 ) c = (height<render_heightMin? 0:1);
	else c = (height<render_heightMin? 0 :(height>render_heightMax? 1 :
				//this is equivalent to (height - render_heightMin) / (render_heightMax - render_heightMin)
				((height-render_heightMin) * render_heightMult)));
	return c;
}

//***************************************************************************
// Mental Ray support
//***************************************************************************

imrShader* NormalsBakeElement::GetMRShader(imrShaderCreation& shaderCreation) {

	imrShader* shader = shaderCreation.CreateShader(_T("max_NormalsBakeElement"), ElementName());
	return shader;
}

void NormalsBakeElement::ReleaseMRShader() {
	
	// Do nothing (we're not holding a reference to the shader)
}

bool NormalsBakeElement::NeedsAutomaticParamBlock2Translation() {

	// All translation is manual
	return false;
}

void NormalsBakeElement::TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid) {

	if(shader != NULL) {
		IParamBlock2* paramsPBlock = shader->GetParametersParamBlock();
		if(paramsPBlock != NULL) {
			TranslateMRShaderParameter(paramsPBlock, _T("applyAtmosphere"), AtmosphereApplied());
			TranslateMRShaderParameter(paramsPBlock, _T("applyShadows"), ShadowsApplied());
		}
	}
}

void NormalsBakeElement::GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets) {

	// No additional dependencies
}

BaseInterface* NormalsBakeElement::GetInterface(Interface_ID id) {

	if(id == IMRSHADERTRANSLATION_INTERFACE_ID) {
		return static_cast<imrShaderTranslation*>(this);
	}
	else {
		return BaseBakeElement::GetInterface(id);
	}
}
//-----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
//
//	Complete Bake element
//
class CompleteBakeElement : public BaseBakeElement, public imrShaderTranslation {
public:
		enum{
			lightingOn = BaseBakeElement::fileType+1, // last element + 1
			shadowsOn,
			targetMapSlot
		};

		static int paramNames[];

		CompleteBakeElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return completeBakeElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_COMPLETE_BAKE_ELEMENT ); }

				// set/get element's light applied state
//		void SetLightApplied(BOOL on){ 
//			mpParamBlk->SetValue( lightingOn, 0, on ); 
//		}
//		BOOL IsLightApplied() const{
//			int	on;
//			mpParamBlk->GetValue( lightingOn, 0, on, FOREVER );
//			return on;
//		}
		BOOL IsLightApplied() const{ return TRUE; }


		// set/get element's shadow applied state
		void SetShadowApplied(BOOL on){
			mpParamBlk->SetValue( shadowsOn, 0, on ); 
		}
		BOOL IsShadowApplied() const{
			int	on;
			mpParamBlk->GetValue( shadowsOn, 0, on, FOREVER );
			return on;
		}

		IRenderElementParamDlg *CreateParamDialog(IRendParams *ip);

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
//		void PostAtmosphere(ShadeContext& sc, float z, float zPrev);

		int  GetNParams() const { return 1; }
		// 1 based param numbering
		const TCHAR* GetParamName1( int nParam ) { 
			if( nParam == 1 )
				return GetString( paramNames[ nParam-1 ] );
			else 
				return NULL;
		}
		const int FindParamByName1( TCHAR*  name ) { 
			if( strcmp( name, GetString( paramNames[0] )) == 0 )
				return 1;
			return 0; 
		}
		// currently only type 1 == boolean or 0 == undefined
		int  GetParamType1( int nParam ) { 
			return ( nParam==1 )? 1 : 0;
		}

		int  GetParamValue1( int nParam ){
			if( nParam == 1 ) 
				return IsShadowApplied();
			return -1;
		}
		void SetParamValue1( int nParam, int newVal ){
			if( nParam == 1 ) 
				SetShadowApplied( newVal);
		}

		// -- from imrShaderTranslation
		virtual imrShader* GetMRShader(imrShaderCreation& shaderCreation);
		virtual void ReleaseMRShader();
		virtual bool NeedsAutomaticParamBlock2Translation();
		virtual void TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid);
		virtual void GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets);

		// -- from InterfaceServer
		virtual BaseInterface* GetInterface(Interface_ID id);
};

int CompleteBakeElement::paramNames[] = { IDS_SHADOWS_ON };

// --------------------------------------------------
//texture bake the complete illumination class descriptor 
class CompleteBakeElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { AddInterface(&BaseBakeElement::_fpInterfaceDesc); return new CompleteBakeElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_COMPLETE_BAKE_ELEMENT); }
	SClass_ID		SuperClassID() { return BAKE_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return completeBakeElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("completeMap"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static CompleteBakeElementClassDesc completeBakeCD;
#endif	// no_render_to_texture

ClassDesc* GetCompleteBakeElementDesc()
{
#ifndef	NO_RENDER_TO_TEXTURE
	return &completeBakeCD;
#else	// no_render_to_texture
	return NULL;
#endif	// no_render_to_texture
}

#ifndef	NO_RENDER_TO_TEXTURE
IRenderElementParamDlg *CompleteBakeElement::CreateParamDialog(IRendParams *ip)
{
	return (IRenderElementParamDlg *)completeBakeCD.CreateParamDialogs(ip, this);
}

// ---------------------------------------------
// normal map baker parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 completeBakeParamBlk(PBLOCK_NUMBER, _T("Full Illumination bake element parameters"), 0, &completeBakeCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_SHADOW, IDS_COMPLETE_BAKE_PARAMS, 0, 0, NULL,
	// params
	BaseBakeElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseBakeElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseBakeElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseBakeElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	BaseBakeElement::outputSzX, _T("outputSzX"), TYPE_INT, 0, IDS_OUTPUTSIZE_X,
		p_default, 256,
		end,
	BaseBakeElement::outputSzY, _T("outputSzY"), TYPE_INT, 0, IDS_OUTPUTSIZE_Y,
		p_default, 256,
		end,
	BaseBakeElement::autoSzOn, _T("autoSzOn"), TYPE_BOOL, 0, IDS_AUTOSIZE_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::fileName, _T("fileName"), TYPE_FILENAME, 0, IDS_FILE_NAME,
		p_default, "",
		end,
	BaseBakeElement::fileNameUnique, _T("filenameUnique"), TYPE_BOOL, 0, IDS_FILENAME_UNIQUE,
		p_default, FALSE,
		end,
	BaseBakeElement::fileType, _T("fileType"), TYPE_FILENAME, 0, IDS_FILE_TYPE,
		p_default, "",
		end,
	BaseBakeElement::backgroundColor, _T("backgroundColor"), TYPE_FRGBA, 0, IDS_BACKGROUNDCOLOR,
		p_default, AColor(0.0f, 0.0f, 0.0f, 0.0f),
		end,
	CompleteBakeElement::shadowsOn, _T("shadowsOn"), TYPE_BOOL, 0, IDS_SHADOWS_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SHADOWS_ON,
		p_default, TRUE,
		end,
	CompleteBakeElement::targetMapSlot, _T("targetMapSlotName"), TYPE_STRING, 0, IDS_TARGET_MAP_SLOT_NAME,
		p_default, "",
		end,
	end
	);


//--- Complete Illumination Baking Element ------------------------------------------------
CompleteBakeElement::CompleteBakeElement()
{
	completeBakeCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
	mpRenderBitmap = NULL;
}


RefTargetHandle CompleteBakeElement::Clone( RemapDir &remap )
{
	CompleteBakeElement* newEle = new CompleteBakeElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}


void CompleteBakeElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);

	} else {
		AColor c(0,0,0,0);
		c += ip.diffIllumOut + ip.ambIllumOut + ip.selfIllumOut;
		c *= ip.finalAttenuation;
		c += ip.specIllumOut + ip.reflIllumOut + ip.transIllumOut;
		int n;
		if( IsShadowApplied() && (n = ip.FindUserIllumName( shadowIllumOutStr ))>=0 )
				c += ip.GetUserIllumOutput( n );

		// now add ink & paint
//		if( (n = ip.FindUserIllumName( inkIllumOutStr ))>=0 )
//				c += ip.GetUserIllumOutput( n );
		if( (n = ip.FindUserIllumName( paintIllumOutStr ))>=0 )
				c += ip.GetUserIllumOutput( n );

		if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
				&& sc.globContext->pToneOp->Active(sc.CurTime())) {
			sc.globContext->pToneOp->ScaledToRGB(c);
		}

		ClampMax( c );
		float a = 1.0f - Intens( sc.out.t );
		c.a = CLAMP( a );
		sc.out.elementVals[ mShadeOutputIndex ] = c;
	}
}

imrShader* CompleteBakeElement::GetMRShader(imrShaderCreation& shaderCreation) {

	imrShader* shader = shaderCreation.CreateShader(_T("max_CompleteBakeElement"), ElementName());
	return shader;
}

void CompleteBakeElement::ReleaseMRShader() {
	
	// Do nothing (we're not holding a reference to the shader)
}

bool CompleteBakeElement::NeedsAutomaticParamBlock2Translation() {

	// All translation is manual
	return false;
}

void CompleteBakeElement::TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid) {

	if(shader != NULL) {
		IParamBlock2* paramsPBlock = shader->GetParametersParamBlock();
		if(paramsPBlock != NULL) {
			TranslateMRShaderParameter(paramsPBlock, _T("applyAtmosphere"), AtmosphereApplied());
			TranslateMRShaderParameter(paramsPBlock, _T("applyShadows"), ShadowsApplied());
		}
	}
}

void CompleteBakeElement::GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets) {

	// No additional dependencies
}

BaseInterface* CompleteBakeElement::GetInterface(Interface_ID id) {

	if(id == IMRSHADERTRANSLATION_INTERFACE_ID) {
		return static_cast<imrShaderTranslation*>(this);
	}
	else {
		return BaseBakeElement::GetInterface(id);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//	Blend render element
//

class BlendBakeElement : public BaseBakeElement, public imrShaderTranslation {
public:
		enum{
			lightingOn = BaseBakeElement::fileType+1, // last element + 1
			shadowsOn, diffuseOn, ambientOn, specularOn, 
			emissionOn, reflectionOn, refractionOn, targetMapSlot
		};

		static int paramNames[];

		BlendBakeElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return blendBakeElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_BLEND_BAKE_ELEMENT ); }

		SFXParamDlg *CreateParamDialog(IRendParams *ip);

		//attributes of the blend
		// set/get element's light applied state
		void SetLightApplied(BOOL on){ 
			mpParamBlk->SetValue( lightingOn, 0, on ); 
		}
		BOOL IsLightApplied() const{
			int	on;
			mpParamBlk->GetValue( lightingOn, 0, on, FOREVER );
			return on;
		}

		// set/get element's shadow applied state
		void SetShadowApplied(BOOL on){
			mpParamBlk->SetValue( shadowsOn, 0, on ); 
		}
		BOOL IsShadowApplied() const{
			int	on;
			mpParamBlk->GetValue( shadowsOn, 0, on, FOREVER );
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

		// the compute functions
		void PostIllum(ShadeContext& sc, IllumParams& ip);
		void PostAtmosphere(ShadeContext& sc, float z, float zPrev);

		int  GetNParams() const { return 8; }
		// 1 based param numbering
		const TCHAR* GetParamName1( int nParam ) { 
			if( nParam > 0 && nParam <= GetNParams() )
				return GetString( paramNames[ nParam-1 ] );
			else 
				return NULL;
		}
		const int FindParamByName1( TCHAR*  name ) { 
			for( int i = 0; i < GetNParams(); ++i ){
				if( strcmp( name, GetString( paramNames[i] )) == 0 )
					return i+1;
			}
			return 0; 
		}
		// currently only type 1 == boolean or 0 == undefined
		int  GetParamType1( int nParam ) { 
			return ( nParam > 0 && nParam <= GetNParams() )? 1 : 0;
		}

		int  GetParamValue1( int nParam ){
			switch( nParam ) {
				case 1: return IsLightApplied();
				case 2: return IsShadowApplied();
				case 3: return IsDiffuseOn();
				case 4: return IsAmbientOn();
				case 5: return IsSpecularOn();
				case 6: return IsEmissionOn();
				case 7: return IsReflectionOn();
				case 8: return IsRefractionOn();
				default: return -1;
			}
		}
		void SetParamValue1( int nParam, int newVal ){
			switch( nParam ) {
				case 1: SetLightApplied( newVal ); break;
				case 2: SetShadowApplied( newVal ); break;
				case 3: SetDiffuseOn( newVal ); break;
				case 4: SetAmbientOn( newVal ); break;
				case 5: SetSpecularOn( newVal ); break;
				case 6: SetEmissionOn( newVal ); break;
				case 7: SetReflectionOn( newVal ); break;
				case 8: SetRefractionOn( newVal ); break;
			}
		}
		
		// -- from imrShaderTranslation
		virtual imrShader* GetMRShader(imrShaderCreation& shaderCreation);
		virtual void ReleaseMRShader();
		virtual bool NeedsAutomaticParamBlock2Translation();
		virtual void TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid);
		virtual void GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets);

		// -- from InterfaceServer
		virtual BaseInterface* GetInterface(Interface_ID id);

};

int BlendBakeElement::paramNames[] =	{ IDS_LIGHTING_ON, IDS_SHADOWS_ON,
										  IDS_DIFFUSE_ON, IDS_AMBIENT_ON,
										  IDS_SPECULAR_ON, IDS_EMISSION_ON,
										  IDS_REFLECTION_ON, IDS_REFRACTION_ON
										};


// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class BlendBakeElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { AddInterface(&BaseBakeElement::_fpInterfaceDesc); return new BlendBakeElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_BLEND_BAKE_ELEMENT); }
	SClass_ID		SuperClassID() { return BAKE_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return blendBakeElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("blendMap"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static BlendBakeElementClassDesc blendBakeCD;
#endif	// no_render_to_texture

ClassDesc* GetBlendBakeElementDesc()
{
#ifndef	NO_RENDER_TO_TEXTURE
	return &blendBakeCD;
#else	// no_render_to_texture
	return NULL;
#endif	// no_render_to_texture
}

#ifndef	NO_RENDER_TO_TEXTURE
IRenderElementParamDlg *BlendBakeElement::CreateParamDialog(IRendParams *ip)
{
	IRenderElementParamDlg * paramDlg = blendBakeCD.CreateParamDialogs(ip, this);
	SetLightApplied( IsLightApplied() );	//update controls
	return paramDlg;
}

class BlendPBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		BaseBakeElement* m = (BaseBakeElement*)owner;
//		macroRecorder->Disable();
		switch (id)
		{
			case BlendBakeElement::lightingOn: {
				IParamMap2* map = m->GetMap();
				if ( map ) {
					map->Enable(BlendBakeElement::shadowsOn, v.i );
				}
			} break;
		}
//		macroRecorder->Enable();
	}
};

static BlendPBAccessor blendPBAccessor;



// ---------------------------------------------
// Blend parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 blendBakeParamBlk(PBLOCK_NUMBER, _T("Blend bake element parameters"), 0, &blendBakeCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_BLEND_BAKE_ELEMENT, IDS_BLEND_BAKE_PARAMS, 0, 0, NULL,
	// params
	// these from the base bake element
	BaseBakeElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseBakeElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseBakeElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	BaseBakeElement::outputSzX, _T("outputSzX"), TYPE_INT, 0, IDS_OUTPUTSIZE_X,
		p_default, 256,
		end,
	BaseBakeElement::outputSzY, _T("outputSzY"), TYPE_INT, 0, IDS_OUTPUTSIZE_Y,
		p_default, 256,
		end,
	BaseBakeElement::autoSzOn, _T("autoSzOn"), TYPE_BOOL, 0, IDS_AUTOSIZE_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::fileName, _T("fileName"), TYPE_FILENAME, 0, IDS_FILE_NAME,
		p_default, "",
		end,
	BaseBakeElement::fileNameUnique, _T("filenameUnique"), TYPE_BOOL, 0, IDS_FILENAME_UNIQUE,
		p_default, FALSE,
		end,
	BaseBakeElement::fileType, _T("fileType"), TYPE_FILENAME, 0, IDS_FILE_TYPE,
		p_default, "",
		end,
	BaseBakeElement::backgroundColor, _T("backgroundColor"), TYPE_FRGBA, 0, IDS_BACKGROUNDCOLOR,
		p_default, AColor(0.0f, 0.0f, 0.0f, 0.0f),
		end,

	// blend specific
	BlendBakeElement::lightingOn, _T("lightingOn"), TYPE_BOOL, 0, IDS_LIGHTING_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_LIGHTING_ON,
		p_default, TRUE,
		p_accessor,		&blendPBAccessor,
		end,
	BlendBakeElement::shadowsOn, _T("shadowsOn"), TYPE_BOOL, 0, IDS_SHADOWS_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SHADOWS_ON,
		p_default, TRUE,
		end,
	BlendBakeElement::diffuseOn, _T("diffuseOn"), TYPE_BOOL, 0, IDS_DIFFUSE_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_DIFFUSE_ON,
		end,
	BlendBakeElement::ambientOn, _T("ambientOn"), TYPE_BOOL, 0, IDS_AMBIENT_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_AMBIENT_ON,
		end,
	BlendBakeElement::specularOn, _T("specularOn"), TYPE_BOOL, 0, IDS_SPECULAR_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SPECULAR_ON,
		end,
	BlendBakeElement::emissionOn, _T("emissionOn"), TYPE_BOOL, 0, IDS_EMISSION_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_EMISSION_ON,
		end,
	BlendBakeElement::reflectionOn, _T("reflectionOn"), TYPE_BOOL, 0, IDS_REFLECTION_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_REFLECTION_ON,
		end,
	BlendBakeElement::refractionOn, _T("refractionOn"), TYPE_BOOL, 0, IDS_REFRACTION_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_REFRACTION_ON,
		end,
	BlendBakeElement::targetMapSlot, _T("targetMapSlotName"), TYPE_STRING, 0, IDS_TARGET_MAP_SLOT_NAME,
		p_default, "",
		end,
	end
	); 

//--- Blend Render Element ------------------------------------------------
BlendBakeElement::BlendBakeElement()
{
	blendBakeCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
	mpRenderBitmap = NULL;
}


RefTargetHandle BlendBakeElement::Clone( RemapDir &remap )
{
	BlendBakeElement*	newEle = new BlendBakeElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}


void BlendBakeElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	AColor c(0,0,0,0);
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = c;
	} else {
		bool resultCalculated = false;
		if( IsLightApplied() ){
			if( IsDiffuseOn() ) { c += ip.diffIllumOut; resultCalculated = true; }
			if( IsAmbientOn() ) { c += ip.ambIllumOut; resultCalculated = true; }
			if( IsEmissionOn() ) { c += ip.selfIllumOut; resultCalculated = true; }
			c *= ip.finalAttenuation;
			if( IsSpecularOn() ) { c += ip.specIllumOut; resultCalculated = true; }
			if( IsReflectionOn() ) { c += ip.reflIllumOut; resultCalculated = true; }
			if( IsRefractionOn() ) { c += ip.transIllumOut; resultCalculated = true; }
			int n;
			if( ShadowsApplied() && (n = ip.FindUserIllumName( shadowIllumOutStr ))>=0 )
				{ c += ip.GetUserIllumOutput( n ); resultCalculated = true; }

			if (resultCalculated) {
				if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
						&& sc.globContext->pToneOp->Active(sc.CurTime())) {
					sc.globContext->pToneOp->ScaledToRGB(c);
				}
				ClampMax( c );
			}
		} // end, lighting applied
		else {
			// no lighting, use sum of unshaded channels
			// >>>>>> shd these be overs rather than sums???
			// LAM - 7/11/03 - 507044 - some mtls leave stdIDToChannel NULL, ignore if so
			if( IsDiffuseOn() && ip.stdIDToChannel)
				{ c += ip.channels[ ip.stdIDToChannel[ ID_DI ]]; resultCalculated = true; }
			else if( IsAmbientOn() )
				{ c += ip.channels[ ip.stdIDToChannel[ ID_AM ]]; resultCalculated = true; }
			else if( IsEmissionOn() ) 
				{ c += ip.channels[ ip.stdIDToChannel[ ID_SI ]]; resultCalculated = true; }
			else if( IsSpecularOn() )
				{ c += ip.channels[ ip.stdIDToChannel[ ID_SP ]]; resultCalculated = true; }
			else if( IsReflectionOn() )
				{ c += ip.channels[ ip.stdIDToChannel[ ID_RL ]]; resultCalculated = true; }
			else if( IsRefractionOn() )
				{ c += ip.channels[ ip.stdIDToChannel[ ID_RR ]]; resultCalculated = true; }
		}

		// apply alpha regardless of lighting on/off as long as we calcuated some result
		if (resultCalculated) {
			float a = 1.0f - Intens( sc.out.t );
			c.a = CLAMP( a );
		}

		sc.out.elementVals[ mShadeOutputIndex ] = c;
	}
}

void BlendBakeElement::PostAtmosphere(ShadeContext& sc, float z, float zPrev)
{
	sc.out.elementVals[ mShadeOutputIndex ] = sc.out.c;

	if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
			&& sc.globContext->pToneOp->Active(sc.CurTime())
			&& sc.globContext->pToneOp->GetProcessBackground()) {
		sc.globContext->pToneOp->ScaledToRGB(sc.out.elementVals[ mShadeOutputIndex ]);
	}

	float a = 1.0f - Intens( sc.out.t );
	a = CLAMP( a );
	sc.out.elementVals[ mShadeOutputIndex ].a = a;
}

imrShader* BlendBakeElement::GetMRShader(imrShaderCreation& shaderCreation) {

	imrShader* shader = shaderCreation.CreateShader(_T("max_BlendBakeElement"), ElementName());
	return shader;
}

void BlendBakeElement::ReleaseMRShader() {
	
	// Do nothing (we're not holding a reference to the shader)
}

bool BlendBakeElement::NeedsAutomaticParamBlock2Translation() {

	// All translation is manual
	return false;
}

void BlendBakeElement::TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid) {

	if(shader != NULL) {
		IParamBlock2* paramsPBlock = shader->GetParametersParamBlock();
		if(paramsPBlock != NULL) {
			TranslateMRShaderParameter(paramsPBlock, _T("applyAtmosphere"), AtmosphereApplied());
			TranslateMRShaderParameter(paramsPBlock, _T("applyShadows"), ShadowsApplied());
			TranslateMRShaderParameter(paramsPBlock, _T("applyLight"), IsLightApplied());
			TranslateMRShaderParameter(paramsPBlock, _T("diffuseOn"), IsDiffuseOn());
			TranslateMRShaderParameter(paramsPBlock, _T("ambientOn"), IsAmbientOn());
			TranslateMRShaderParameter(paramsPBlock, _T("emissionOn"), IsEmissionOn());
			TranslateMRShaderParameter(paramsPBlock, _T("specularOn"), IsSpecularOn());
			TranslateMRShaderParameter(paramsPBlock, _T("reflectionOn"), IsReflectionOn());
			TranslateMRShaderParameter(paramsPBlock, _T("refractionOn"), IsRefractionOn());
		}
	}
}

void BlendBakeElement::GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets) {

	// No additional dependencies
}

BaseInterface* BlendBakeElement::GetInterface(Interface_ID id) {

	if(id == IMRSHADERTRANSLATION_INTERFACE_ID) {
		return static_cast<imrShaderTranslation*>(this);
	}
	else {
		return BaseBakeElement::GetInterface(id);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//	Alpha Bake element
//		- texture bake the alpha channel
//
class AlphaBakeElement : public BaseBakeElement, public imrShaderTranslation {
public:
		enum{
			targetMapSlot = BaseBakeElement::fileType+1, // last element + 1
		};
		AlphaBakeElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return alphaBakeElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_ALPHA_BAKE_ELEMENT ); }

		BOOL IsLightApplied() const{ return FALSE; }

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
		
		// -- from imrShaderTranslation
		virtual imrShader* GetMRShader(imrShaderCreation& shaderCreation);
		virtual void ReleaseMRShader();
		virtual bool NeedsAutomaticParamBlock2Translation();
		virtual void TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid);
		virtual void GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets);

		// -- from InterfaceServer
		virtual BaseInterface* GetInterface(Interface_ID id);
};

// --------------------------------------------------
//texture bake the Alpha class descriptor 
class AlphaBakeElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { AddInterface(&BaseBakeElement::_fpInterfaceDesc); return new AlphaBakeElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_ALPHA_BAKE_ELEMENT); }
	SClass_ID		SuperClassID() { return BAKE_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return alphaBakeElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("alphaMap"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static AlphaBakeElementClassDesc alphaBakeCD;
#endif	// no_render_to_texture

ClassDesc* GetAlphaBakeElementDesc()
{
#ifndef	NO_RENDER_TO_TEXTURE
	return &alphaBakeCD;
#else	// no_render_to_texture
	return NULL;
#endif	// no_render_to_texture
}

#ifndef	NO_RENDER_TO_TEXTURE

// ---------------------------------------------
// normal map baker parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 alphaBakeParamBlk(PBLOCK_NUMBER, _T("Alpha map bake element parameters"), 0, &alphaBakeCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseBakeElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseBakeElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::outputSzX, _T("outputSzX"), TYPE_INT, 0, IDS_OUTPUTSIZE_X,
		p_default, 256,
		end,
	BaseBakeElement::outputSzY, _T("outputSzY"), TYPE_INT, 0, IDS_OUTPUTSIZE_Y,
		p_default, 256,
		end,
	BaseBakeElement::autoSzOn, _T("autoSzOn"), TYPE_BOOL, 0, IDS_AUTOSIZE_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::fileName, _T("fileName"), TYPE_FILENAME, 0, IDS_FILE_NAME,
		p_default, "",
		end,
	BaseBakeElement::fileNameUnique, _T("filenameUnique"), TYPE_BOOL, 0, IDS_FILENAME_UNIQUE,
		p_default, FALSE,
		end,
	BaseBakeElement::fileType, _T("fileType"), TYPE_FILENAME, 0, IDS_FILE_TYPE,
		p_default, "",
		end,
	BaseBakeElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseBakeElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	BaseBakeElement::backgroundColor, _T("backgroundColor"), TYPE_FRGBA, 0, IDS_BACKGROUNDCOLOR,
		p_default, AColor(0.0f, 0.0f, 0.0f, 0.0f),
		end,
	AlphaBakeElement::targetMapSlot, _T("targetMapSlotName"), TYPE_STRING, 0, IDS_TARGET_MAP_SLOT_NAME,
		p_default, "",
		end,
	end
	);


//--- Normal Map Baking Element ------------------------------------------------
AlphaBakeElement::AlphaBakeElement()
{
	alphaBakeCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
	mpRenderBitmap = NULL;
}


RefTargetHandle AlphaBakeElement::Clone( RemapDir &remap )
{
	AlphaBakeElement* newEle = new AlphaBakeElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}


void AlphaBakeElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);

	} else {
		float a = 1.0f - Intens( sc.out.t );
		a = CLAMP( a );
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(a,a,a,a);
	}
}

imrShader* AlphaBakeElement::GetMRShader(imrShaderCreation& shaderCreation) {

	imrShader* shader = shaderCreation.CreateShader(_T("max_AlphaBakeElement"), ElementName());
	return shader;
}

void AlphaBakeElement::ReleaseMRShader() {
	
	// Do nothing (we're not holding a reference to the shader)
}

bool AlphaBakeElement::NeedsAutomaticParamBlock2Translation() {

	// All translation is manual
	return false;
}

void AlphaBakeElement::TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid) {

	if(shader != NULL) {
		IParamBlock2* paramsPBlock = shader->GetParametersParamBlock();
		if(paramsPBlock != NULL) {
			TranslateMRShaderParameter(paramsPBlock, _T("applyAtmosphere"), AtmosphereApplied());
			TranslateMRShaderParameter(paramsPBlock, _T("applyShadows"), ShadowsApplied());
		}
	}
}

void AlphaBakeElement::GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets) {

	// No additional dependencies
}

BaseInterface* AlphaBakeElement::GetInterface(Interface_ID id) {

	if(id == IMRSHADERTRANSLATION_INTERFACE_ID) {
		return static_cast<imrShaderTranslation*>(this);
	}
	else {
		return BaseBakeElement::GetInterface(id);
	}
}

//-----------------------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
//
//	Height baking element - for projection mapping only
//
class HeightBakeElement :
		public BaseBakeElement,
		public imrShaderTranslation
{
	public:
		enum{
			targetMapSlot = BaseBakeElement::fileType+1, // last element + 1
		};
		HeightBakeElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return heightBakeElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_HEIGHT_BAKE_ELEMENT ); }

		void Init( ShadeContext& sc );

		// Helper method
		float GetProjectionHeight( ShadeContext& sc, IProjection_WorkingModelInfo* workingModelInfo );

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);

		BOOL IsLightApplied() const{ return TRUE; }
		BOOL IsShadowApplied() const{ return TRUE; }

		// -- from InterfaceServer
		virtual BaseInterface* GetInterface(Interface_ID id);

		// -- from Animatable
		int RenderBegin(TimeValue t, ULONG flags=0);
		int RenderEnd(TimeValue t) {return 0;}

		// -- from imrShaderTranslation
		virtual imrShader* GetMRShader(imrShaderCreation& shaderCreation);
		virtual void ReleaseMRShader();
		virtual bool NeedsAutomaticParamBlock2Translation();
		virtual void TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid);
		virtual void GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets);

	protected:
		BOOL render_init, render_projectionOn;
		float render_heightMin, render_heightMax, render_heightMult;
};

// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class HeightBakeElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() {
#ifdef NORMAL_MAPPING_RTT_BAKE_ELEMENT 
		return 1 ;
#else 
		return 0 ;
#endif
	}

	void *			Create(BOOL loading) { AddInterface(&BaseBakeElement::_fpInterfaceDesc); return new HeightBakeElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_HEIGHT_BAKE_ELEMENT); }
	SClass_ID		SuperClassID() { return BAKE_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return heightBakeElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("heightMap"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static HeightBakeElementClassDesc heightBakeCD;
#endif	// no_render_to_texture

ClassDesc* GetHeightBakeElementDesc()
{
#ifndef	NO_RENDER_TO_TEXTURE
	return &heightBakeCD;
#else	// no_render_to_texture
	return NULL;
#endif	// no_render_to_texture
}


#ifndef	NO_RENDER_TO_TEXTURE
// ---------------------------------------------
// Height parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 heightBakeParamBlk(PBLOCK_NUMBER, _T("Height render element parameters"), 0, &heightBakeCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseBakeElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseBakeElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseBakeElement::outputSzX, _T("outputSzX"), TYPE_INT, 0, IDS_OUTPUTSIZE_X,
		p_default, 256,
		end,
	BaseBakeElement::outputSzY, _T("outputSzY"), TYPE_INT, 0, IDS_OUTPUTSIZE_Y,
		p_default, 256,
		end,
	BaseBakeElement::autoSzOn, _T("autoSzOn"), TYPE_BOOL, 0, IDS_AUTOSIZE_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::fileName, _T("fileName"), TYPE_FILENAME, 0, IDS_FILE_NAME,
		p_default, "",
		end,
	BaseBakeElement::fileNameUnique, _T("filenameUnique"), TYPE_BOOL, 0, IDS_FILENAME_UNIQUE,
		p_default, FALSE,
		end,
	BaseBakeElement::fileType, _T("fileType"), TYPE_FILENAME, 0, IDS_FILE_TYPE,
		p_default, "",
		end,
	BaseBakeElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseBakeElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	BaseBakeElement::backgroundColor, _T("backgroundColor"), TYPE_FRGBA, 0, IDS_BACKGROUNDCOLOR,
		p_default, AColor(127.0f/255.0f, 127.0f/255.0f, 127.0f/255.0f, 0.0f),
		end,
	HeightBakeElement::targetMapSlot, _T("targetMapSlotName"), TYPE_STRING, 0, IDS_TARGET_MAP_SLOT_NAME,
		p_default, "",
		end,
	end
	);

//--- Height Bake Element ------------------------------------------------
HeightBakeElement::HeightBakeElement()
{
	heightBakeCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
	mpRenderBitmap = NULL;
}


RefTargetHandle HeightBakeElement::Clone( RemapDir &remap )
{
	HeightBakeElement* newEle = new HeightBakeElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}

int HeightBakeElement::RenderBegin(TimeValue t, ULONG flags) {
	render_init = FALSE; //to be initialized on the first PostIllum call
	return 0;
}

void HeightBakeElement::Init( ShadeContext& sc ) {
	IProjection_WorkingModelInfo* workingModelInfo = GetWorkingModelInfo(sc);
	INode* node;
	if( workingModelInfo==NULL ) {
		render_projectionOn = FALSE;
		node = sc.Node();
	}
	else {
		RenderInstance* workingModelInst = workingModelInfo->GetRenderInstance();
		node = workingModelInst->GetINode();
	}

	INodeBakeProjProperties* projProps = GetINodeBakeProjProperties( node );
	if( projProps==NULL )
		render_projectionOn = FALSE;
	else {
		render_projectionOn =	projProps->GetEnabled();
		render_heightMin = projProps->GetHeightMapMin();
		render_heightMax = projProps->GetHeightMapMax();
		// FIXME: what happens if render_heightMin > render_heightMax ?
		render_heightMult = 1.0f / (render_heightMax - render_heightMin);
	}

	render_init = TRUE;
}


void HeightBakeElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
		return;
	}

	if( !render_init ) Init( sc );

	if( !render_projectionOn )
		// Height map is only supported with projection mapping
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	else {
		IProjection_WorkingModelInfo* workingModelInfo = GetWorkingModelInfo(sc);
		RenderInstance* workingModelInst = workingModelInfo->GetRenderInstance();

		float c = GetProjectionHeight( sc, workingModelInfo );
		AColor retVal(c,c,c,1.0f);

		retVal.a = 1.0f - Intens( sc.out.t );
		retVal.a = CLAMP( retVal.a );
		sc.out.elementVals[ mShadeOutputIndex ] = retVal;
	}
}

// Helper function
float HeightBakeElement::GetProjectionHeight( ShadeContext& sc, IProjection_WorkingModelInfo* workingModelInfo ) {
	Point3 pointA = sc.PointTo( workingModelInfo->GetPoint(), REF_WORLD );
	Point3 pointB = sc.PointTo( sc.P(), REF_WORLD );
	Point3 normal = sc.VectorTo( workingModelInfo->GetOrigNormal(), REF_WORLD ).Normalize();
	float c, height = DotProd( normal, (pointB - pointA) );

	if( render_heightMult==0 ) c = (height<render_heightMin? 0:1);
	else c = (height<render_heightMin? 0 :(height>render_heightMax? 1 :
				//this is equivalent to (height - render_heightMin) / (render_heightMax - render_heightMin)
				((height-render_heightMin) * render_heightMult)));
	return c;
}

//***************************************************************************
// Mental Ray support
//***************************************************************************

imrShader* HeightBakeElement::GetMRShader(imrShaderCreation& shaderCreation) {

	imrShader* shader = shaderCreation.CreateShader(_T("max_HeightBakeElement"), ElementName());
	return shader;
}

void HeightBakeElement::ReleaseMRShader() {
	
	// Do nothing (we're not holding a reference to the shader)
}

bool HeightBakeElement::NeedsAutomaticParamBlock2Translation() {

	// All translation is manual
	return false;
}

void HeightBakeElement::TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid) {

	if(shader != NULL) {
		IParamBlock2* paramsPBlock = shader->GetParametersParamBlock();
		if(paramsPBlock != NULL) {
			TranslateMRShaderParameter(paramsPBlock, _T("applyAtmosphere"), AtmosphereApplied());
			TranslateMRShaderParameter(paramsPBlock, _T("applyShadows"), ShadowsApplied());
		}
	}
}

void HeightBakeElement::GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets) {

	// No additional dependencies
}

BaseInterface* HeightBakeElement::GetInterface(Interface_ID id) {

	if(id == IMRSHADERTRANSLATION_INTERFACE_ID) {
		return static_cast<imrShaderTranslation*>(this);
	}
	else {
		return BaseBakeElement::GetInterface(id);
	}
}


//-----------------------------------------------------------------------------
#ifndef NO_AMBIENT_OCCLUSION
///////////////////////////////////////////////////////////////////////////////
//
//	Ambient Occlusion baking element - with mental ray only
//

class AmbientOcclusionBakeElement : public BaseBakeElement, public imrShaderTranslation {

public:
	enum ParameterID{
		kParameterID_targetMapSlot = BaseBakeElement::fileType+1, // last element + 1
		kParameterID_samples,
		kParameterID_bright,
		kParameterID_dark,
		kParameterID_spread,
		kParameterID_maxDistance,
		kParameterID_falloff,
	};

	static int paramNames[];

	AmbientOcclusionBakeElement();
	RefTargetHandle Clone( RemapDir &remap );

	Class_ID	ClassID() {return ambientOcclusionBakeElementClassID;}
	TSTR		GetName() { return GetString( IDS_AMBIENTOCCLUSION_BAKE_ELEMENT_UI ); }

	void		SetSamples(int samples)				{ mpParamBlk->SetValue( kParameterID_samples, 0, samples ); }
	int			GetSamples() const					{ return mpParamBlk->GetInt( kParameterID_samples ); }

	void		SetBright(AColor bright)			{ mpParamBlk->SetValue( kParameterID_bright, 0, bright ); }
	AColor		GetBright() const					{ return mpParamBlk->GetAColor( kParameterID_bright ); }

	void		SetDark(AColor dark)				{ mpParamBlk->SetValue( kParameterID_dark, 0, dark ); }
	AColor		GetDark() const						{ return mpParamBlk->GetAColor( kParameterID_dark ); }

	void		SetSpread(float spread)				{ mpParamBlk->SetValue( kParameterID_spread, 0, spread ); }
	float		GetSpread() const					{ return mpParamBlk->GetFloat( kParameterID_spread ); }

	void		SetMaxDistance(float maxDistance)	{ mpParamBlk->SetValue( kParameterID_maxDistance, 0, maxDistance ); }
	float		GetMaxDistance() const			{ return mpParamBlk->GetFloat( kParameterID_maxDistance ); }

	void		SetFalloff(float falloff)			{ mpParamBlk->SetValue( kParameterID_falloff, 0, falloff ); }
	float		GetFalloff() const				{ return mpParamBlk->GetFloat( kParameterID_falloff ); }


	// --from MaxBakeElement and MaxBakeElement8
	BOOL		IsEnabled() const;
	int			GetNParams() const { return 6; }
	const TCHAR* GetParamName1( int nParam );
	const int	FindParamByName1( TCHAR*  name );
	int			GetParamFPValueType( int nParam );
	int			GetParamType1( int nParam );
	FPValue		GetParamFPValue1( int nParam );
	void		SetParamFPValue1( int nParam, FPValue newVal );
	FPValue		GetParamFPValueMin1( int nParam );
	FPValue		GetParamFPValueMax1( int nParam );


	// the compute function
	void PostIllum(ShadeContext& sc, IllumParams& ip);

	BOOL IsLightApplied() const{ return TRUE; }
	BOOL IsShadowApplied() const{ return TRUE; }

	// -- from InterfaceServer
	virtual BaseInterface* GetInterface(Interface_ID id);

	// -- from imrShaderTranslation
	virtual imrShader* GetMRShader(imrShaderCreation& shaderCreation);
	virtual void ReleaseMRShader();
	virtual bool NeedsAutomaticParamBlock2Translation();
	virtual void TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid);
	virtual void GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets);

private:

};

int AmbientOcclusionBakeElement::paramNames[] =	{
	IDS_AMBIENTOCCLUSION_SAMPLES, IDS_AMBIENTOCCLUSION_BRIGHT, IDS_AMBIENTOCCLUSION_DARK,
	IDS_AMBIENTOCCLUSION_SPREAD, IDS_AMBIENTOCCLUSION_MAXDISTANCE, IDS_AMBIENTOCCLUSION_FALLOFF,
};

// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class AmbientOcclusionBakeElementClassDesc : public ClassDesc2
{
public:
	int IsPublic() {
		return 1 ;
	}

	void *			Create(BOOL loading) { AddInterface(&BaseBakeElement::_fpInterfaceDesc); return new AmbientOcclusionBakeElement; }
	const TCHAR *	ClassName() { return GetString(IDS_AMBIENTOCCLUSION_BAKE_ELEMENT); }
	SClass_ID		SuperClassID() { return BAKE_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return ambientOcclusionBakeElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("AmbientOcclusionBakeElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static AmbientOcclusionBakeElementClassDesc ambientOcclusionBakeCD;
#endif //NO_AMBIENT_OCCLUSION
#endif	// no_render_to_texture
ClassDesc* GetAmbientOcclusionBakeElementDesc()
{
#if defined(NO_AMBIENT_OCCLUSION) || defined(NO_RENDER_TO_TEXTURE)
	return NULL;
#else	//NO_AMBIENT_OCCLUSION || NO_RENDER_TO_TEXTURE
	return &ambientOcclusionBakeCD;
#endif //NO_AMBIENT_OCCLUSION || NO_RENDER_TO_TEXTURE
}
#ifndef NO_AMBIENT_OCCLUSION
#ifndef	NO_RENDER_TO_TEXTURE
// ---------------------------------------------
// Height parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 ambientOcclusionBakeParamBlk(PBLOCK_NUMBER, _T("Ambient Occlusion render element parameters"), 0, &ambientOcclusionBakeCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseBakeElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseBakeElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseBakeElement::outputSzX, _T("outputSzX"), TYPE_INT, 0, IDS_OUTPUTSIZE_X,
		p_default, 256,
		end,
	BaseBakeElement::outputSzY, _T("outputSzY"), TYPE_INT, 0, IDS_OUTPUTSIZE_Y,
		p_default, 256,
		end,
	BaseBakeElement::autoSzOn, _T("autoSzOn"), TYPE_BOOL, 0, IDS_AUTOSIZE_ON,
		p_default, FALSE,
		end,
	BaseBakeElement::fileName, _T("fileName"), TYPE_FILENAME, 0, IDS_FILE_NAME,
		p_default, "",
		end,
	BaseBakeElement::fileNameUnique, _T("filenameUnique"), TYPE_BOOL, 0, IDS_FILENAME_UNIQUE,
		p_default, FALSE,
		end,
	BaseBakeElement::fileType, _T("fileType"), TYPE_FILENAME, 0, IDS_FILE_TYPE,
		p_default, "",
		end,
	BaseBakeElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseBakeElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	BaseBakeElement::backgroundColor, _T("backgroundColor"), TYPE_FRGBA, 0, IDS_BACKGROUNDCOLOR,
		p_default, AColor(1.0f, 1.0f, 1.0f, 0.0f),
		end,
	AmbientOcclusionBakeElement::kParameterID_targetMapSlot, _T("targetMapSlotName"), TYPE_STRING, 0, IDS_TARGET_MAP_SLOT_NAME,
		p_default, "",
		end,
	AmbientOcclusionBakeElement::kParameterID_samples, _T("samples"), TYPE_INT, 0, IDS_AMBIENTOCCLUSION_SAMPLES,
		p_default, 16,
		end,
	AmbientOcclusionBakeElement::kParameterID_bright, _T("bright"), TYPE_FRGBA, 0, IDS_AMBIENTOCCLUSION_BRIGHT,
		p_default, AColor(1.0f,1.0f,1.0f,1.0f),
		end,
	AmbientOcclusionBakeElement::kParameterID_dark, _T("dark"), TYPE_FRGBA, 0, IDS_AMBIENTOCCLUSION_DARK,
		p_default, AColor(0.0f, 0.0f, 0.0f, 0.0f),
		end,
	AmbientOcclusionBakeElement::kParameterID_spread, _T("spread"), TYPE_FLOAT, 0, IDS_AMBIENTOCCLUSION_SPREAD,
		p_default, 0.8f,
		end,
	AmbientOcclusionBakeElement::kParameterID_maxDistance, _T("maxDistance"), TYPE_FLOAT, 0, IDS_AMBIENTOCCLUSION_MAXDISTANCE,
		p_default, 0.0f,
		end,
	AmbientOcclusionBakeElement::kParameterID_falloff, _T("falloff"), TYPE_FLOAT, 0, IDS_AMBIENTOCCLUSION_FALLOFF,
		p_default, 1.0f,
		end,
	end
);

//--- Height Bake Element ------------------------------------------------
AmbientOcclusionBakeElement::AmbientOcclusionBakeElement()
{
	ambientOcclusionBakeCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk != NULL);
	SetElementName( GetName() );
	mpRenderBitmap = NULL;
}


RefTargetHandle AmbientOcclusionBakeElement::Clone( RemapDir &remap )
{
	AmbientOcclusionBakeElement* newEle = new AmbientOcclusionBakeElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	BaseClone(this, newEle, remap);
	return (RefTargetHandle)newEle;
}

BOOL AmbientOcclusionBakeElement::IsEnabled() const {
	// MAB - 3/08/05 - Special case: Ambient Occlusion not supported in the scanline renderer
	// There is no up-to-date system to check Render Element compatibility ...
	// Originally, Render Elements would indicate Scanline compatibility by providing a MaxBakeElement interface.
	// But that interface is now used for RTT display of all bake elements, and does not imply any compatibility.
	Renderer* renderer = GetCOREInterface()->GetCurrentRenderer();
	if( (renderer!=NULL) && (renderer->ClassID()==Class_ID(SREND_CLASS_ID,0)) )
		return FALSE;

	return mpParamBlk->GetInt( enableOn );
}

const TCHAR* AmbientOcclusionBakeElement::GetParamName1( int nParam ) { 
	if( nParam > 0 && nParam <= GetNParams() )
		return GetString( paramNames[ nParam-1 ] );
	else 
		return NULL;
}

const int AmbientOcclusionBakeElement::FindParamByName1( TCHAR*  name ) { 
	for( int i = 0; i < GetNParams(); ++i ){
		if( strcmp( name, GetString( paramNames[i] )) == 0 )
			return i+1;
	}
	return 0; 
}

int AmbientOcclusionBakeElement::GetParamFPValueType( int nParam ) {
	switch( nParam ) {
		case 1: //kParameterID_samples
			return TYPE_INT;
		case 2: //kParameterID_bright
		case 3: //kParameterID_dark
			return TYPE_FRGBA_BV;
		case 4: //kParameterID_spread
		case 5: //kParameterID_maxDistance
		case 6: //kParameterID_falloff
			return TYPE_FLOAT;
	}
	return 0;
}

int AmbientOcclusionBakeElement::GetParamType1( int nParam ) {
	return FPValueTypeToParamType( GetParamFPValueType( nParam ) );
}

FPValue AmbientOcclusionBakeElement::GetParamFPValue1( int nParam ) {
	FPValue retval;
	if( nParam > 0 && nParam <= GetNParams() ) {
		retval.type = (ParamType2)GetParamFPValueType( nParam );
		if( nParam==2 || nParam==3 )
			retval.aclr = new AColor; //Will be deleted by the FPValue system

		switch( nParam ) {
			case 1: retval.i = mpParamBlk->GetInt( kParameterID_samples );					break;
			case 2: *(retval.aclr) = mpParamBlk->GetAColor( kParameterID_bright );			break;
			case 3: *(retval.aclr) = mpParamBlk->GetAColor( kParameterID_dark );			break;
			case 4: retval.f = mpParamBlk->GetFloat( kParameterID_spread );					break;
			case 5: retval.f = mpParamBlk->GetFloat( kParameterID_maxDistance );			break;
			case 6: retval.f = mpParamBlk->GetFloat( kParameterID_falloff );				break;
		}
	}
	return retval;
}

void AmbientOcclusionBakeElement::SetParamFPValue1( int nParam, FPValue newVal ) {
	if( nParam > 0 && nParam <= GetNParams() ) {
		switch( nParam ) {
			case 1: mpParamBlk->SetValue( kParameterID_samples, 0, newVal.i );				break;
			case 2: mpParamBlk->SetValue( kParameterID_bright, 0, (AColor)*(newVal.p4) );	break;
			case 3: mpParamBlk->SetValue( kParameterID_dark, 0, (AColor)*(newVal.p4) );		break;
			case 4: mpParamBlk->SetValue( kParameterID_spread, 0, newVal.f );				break;
			case 5: mpParamBlk->SetValue( kParameterID_maxDistance, 0, newVal.f );			break;
			case 6: mpParamBlk->SetValue( kParameterID_falloff, 0, newVal.f );				break;
		}
	}
}

FPValue AmbientOcclusionBakeElement::GetParamFPValueMin1( int nParam ) {
	if( nParam==1 || nParam==4 || nParam==5 || nParam==6 )
		return FPValue( TYPE_INT, (void*)0 );
	return BaseBakeElement::GetParamFPValueMin1( nParam );
}

FPValue AmbientOcclusionBakeElement::GetParamFPValueMax1( int nParam ) {
	if( nParam==1 || nParam==4 || nParam==5 || nParam==6 )
		return FPValue( TYPE_INT, (void*)999999999 );
	return BaseBakeElement::GetParamFPValueMin1( nParam );
}



void AmbientOcclusionBakeElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	// Do nothing - this render element is only supported in mental ray,
	// using the ambient occlusion mental ray shader.
	
	// Set to red to indicate an error.
	sc.out.elementVals[ mShadeOutputIndex ] = AColor(1.0f,0.0f,0.0f,0.0f);
}

//***************************************************************************
// Mental Ray support
//***************************************************************************

imrShader* AmbientOcclusionBakeElement::GetMRShader(imrShaderCreation& shaderCreation) {

	// Create an instance of the generic render element shader.
	imrShader* genericElementShader = shaderCreation.CreateShader(_T("max_GenericBakeElement"), ElementName());

	if(genericElementShader != NULL) {

		// Create an instance of the ambient occlusion shader, which is assigned as a sub-shader
		// of the generic render element shader.
		imrShader* ambientOcclusionShader = shaderCreation.CreateShader(_T("max_amb_occlusion"), ElementName());
		if(ambientOcclusionShader != NULL) {
			IParamBlock2* paramsPBlock = genericElementShader->GetParametersParamBlock();
			if(paramsPBlock != NULL) {
				DbgAssert(IsTex(&(ambientOcclusionShader->GetReferenceTarget())));
				Texmap* ambientOcclusionTexmap = static_cast<Texmap*>(&(ambientOcclusionShader->GetReferenceTarget()));
				TranslateMRShaderParameter(paramsPBlock, _T("subShader"), ambientOcclusionTexmap);
			}
		}
	}

	return genericElementShader;
}

void AmbientOcclusionBakeElement::ReleaseMRShader() {
	
	// Do nothing (we're not holding a reference to the shader)
}

bool AmbientOcclusionBakeElement::NeedsAutomaticParamBlock2Translation() {

	// All translation is manual
	return false;
}

void AmbientOcclusionBakeElement::TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid) {

	// The parameters need to be passed to the ambient occlusion shader, which was already assigned as
	// a sub-shader of the render element shader.
	bool success = false;
	if((shader != NULL) && (mpParamBlk != NULL)) {
		IParamBlock2* paramsPBlock = shader->GetParametersParamBlock();
		if(paramsPBlock != NULL) {
			ParamID subShaderParamID;
			if(GetParamIDByName(subShaderParamID, _T("subShader"), paramsPBlock)) {
				Texmap* ambientOcclusionTexmap = paramsPBlock->GetTexmap(subShaderParamID);
				if((ambientOcclusionTexmap != NULL) && IsIMRShader(ambientOcclusionTexmap)) {
					
					imrShader* ambientOcclusionShader = GetIMRShader(ambientOcclusionTexmap);
					IParamBlock2* ambientOcclusionPBlock = ambientOcclusionShader->GetParametersParamBlock();
					if(ambientOcclusionPBlock != NULL) {

						// Set the parameter values in the shader
						TranslateMRShaderParameter(ambientOcclusionPBlock, _T("samples"), GetSamples());
						TranslateMRShaderParameter(ambientOcclusionPBlock, _T("bright"), GetBright());
						TranslateMRShaderParameter(ambientOcclusionPBlock, _T("dark"), GetDark());
						TranslateMRShaderParameter(ambientOcclusionPBlock, _T("spread"), GetSpread());
						TranslateMRShaderParameter(ambientOcclusionPBlock, _T("max_distance"), GetMaxDistance());
						TranslateMRShaderParameter(ambientOcclusionPBlock, _T("falloff"), GetFalloff());

						// The following parameters are not exposed, we just provide a default value.
						TranslateMRShaderParameter(ambientOcclusionPBlock, _T("reflective"), FALSE);
						TranslateMRShaderParameter(ambientOcclusionPBlock, _T("output_mode"), 0);
						TranslateMRShaderParameter(ambientOcclusionPBlock, _T("occlusion_in_alpha"), FALSE);

						success = true;
					}
				}
			}
		}
	}

	DbgAssert(success);
}

void AmbientOcclusionBakeElement::GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets) {

	// No additional dependencies
}

BaseInterface* AmbientOcclusionBakeElement::GetInterface(Interface_ID id) {

	if(id == IMRSHADERTRANSLATION_INTERFACE_ID) {
		return static_cast<imrShaderTranslation*>(this);
	}
	else {
		return BaseBakeElement::GetInterface(id);
	}
}


//-----------------------------------------------------------------------------


#endif	// no_render_to_texture
#endif //NO_AMBIENT_OCCLUSION