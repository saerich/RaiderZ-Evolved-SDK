/*==============================================================================

  file:     mrMaterial.cpp

  author:   Daniel Levesque

  created:  3mar2003

  description:

      Implementation of the generic mental ray material. The material is translated
	  using the custom shader translation interface, and is provided as a sample
	  for the use of that interface.

  modified:	


© 2003 Autodesk
==============================================================================*/

#include "mrMaterial.h"

#include "resource.h"

#include <iparamm2.h>
#include <mentalray\imrShader.h>
#include <mentalray\imrShaderClassDesc.h>
#include <mentalray\imrPreferences.h>
#include <mentalray\shared_src\mrShaderFilter.h>
#include <mentalray\mentalrayInMax.h>

extern TCHAR *GetString(int id);

//==============================================================================
// mtl/map browser filters for each button
//==============================================================================

static mrShaderFilter surfaceFilter(imrShaderClassDesc::kApplyFlag_Default, static_cast<ParamType2>(TYPE_RGBA), false);
static mrShaderFilter bumpFilter(imrShaderClassDesc::kApplyFlag_Bump, static_cast<ParamType2>(TYPE_RGBA), false);
static mrShaderFilter displaceFilter(imrShaderClassDesc::kApplyFlag_Displace, static_cast<ParamType2>(TYPE_FLOAT), false);
static mrShaderFilter shadowFilter(imrShaderClassDesc::kApplyFlag_Shadow, static_cast<ParamType2>(TYPE_RGBA), false);
static mrShaderFilter environmentFilter(imrShaderClassDesc::kApplyFlag_Environment, static_cast<ParamType2>(TYPE_RGBA), false);
static mrShaderFilter contourFilter(imrShaderClassDesc::kApplyFlag_ContourShader, TYPE_MAX_TYPE, false);
static mrShaderFilter photonFilter(imrShaderClassDesc::kApplyFlag_Photon, TYPE_MAX_TYPE, false);
static mrShaderFilter photonVolFilter(imrShaderClassDesc::kApplyFlag_PhotonVol, TYPE_MAX_TYPE, false);
static mrShaderFilter volumeFilter(imrShaderClassDesc::kApplyFlag_Volume, static_cast<ParamType2>(TYPE_RGBA), false);
static mrShaderFilter lightMapFilter(imrShaderClassDesc::kApplyFlag_LightMap, TYPE_MAX_TYPE, false);


//==============================================================================
// class PrivateResourceManager
//
// Takes care of initializing/deleting private resource.
//==============================================================================
class PrivateResourceManager {

public:

	static HIMAGELIST& GetBrowserIcons();

	~PrivateResourceManager();

private:

	PrivateResourceManager();

	static PrivateResourceManager m_theInstance;

	HIMAGELIST m_matBrowserIcons;
};

PrivateResourceManager PrivateResourceManager::m_theInstance;

PrivateResourceManager::PrivateResourceManager() 
: m_matBrowserIcons(NULL)
{
}

PrivateResourceManager::~PrivateResourceManager() {

	if(m_matBrowserIcons != NULL)
		ImageList_Destroy(m_matBrowserIcons);
}

HIMAGELIST& PrivateResourceManager::GetBrowserIcons() {

	return m_theInstance.m_matBrowserIcons;
}

//==============================================================================
// class mrMaterial
//==============================================================================

ParamBlockDesc2 mrMaterial::m_mainPBDesc(
	kPBID_Main, _T("Material Shaders"), IDS_MATERIAL_SHADERS, &mrMaterial::GetClassDesc(), (P_AUTO_CONSTRUCT | P_AUTO_UI), kRef_MainPB,

	// Rollup
	IDD_MRMATERIAL, IDS_MATERIAL_SHADERS, 0, 0, NULL,

	// Parameters
	kMainPB_Opaque, _T("Opaque"), TYPE_BOOL, P_ANIMATABLE, IDS_OPAQUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_CHECK_OPAQUE,
		p_default, FALSE,
		end,
	kMainPB_Surface, _T("Surface"), TYPE_TEXMAP, P_SUBANIM, IDS_SURFACE,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_SURFACE,
		p_validator, static_cast<PBValidator*>(&surfaceFilter),
		p_subtexno, kSubTex_Surface,
		end,
	kMainPB_Bump, _T("Bump"), TYPE_TEXMAP, P_SUBANIM, IDS_BUMP,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_BUMP,
		p_validator, static_cast<PBValidator*>(&bumpFilter),
		p_subtexno, kSubTex_Bump,
		end,
	kMainPB_Displace, _T("Displacement"), TYPE_TEXMAP, P_SUBANIM, IDS_DISPLACEMENT,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_DISPLACEMENT,
		p_validator, static_cast<PBValidator*>(&displaceFilter),
		p_subtexno, kSubTex_Displace,
		end,
	kMainPB_Shadow, _T("Shadow"), TYPE_TEXMAP, P_SUBANIM, IDS_SHADOW,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_SHADOW,
		p_validator, static_cast<PBValidator*>(&shadowFilter),
		p_subtexno, kSubTex_Shadow,
		end,
	kMainPB_Volume, _T("Volume"), TYPE_TEXMAP, P_SUBANIM, IDS_VOLUME,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_VOLUME,
		p_validator, static_cast<PBValidator*>(&volumeFilter),
		p_subtexno, kSubTex_Volume,
		end,
	kMainPB_Environment, _T("Environment"), TYPE_TEXMAP, P_SUBANIM, IDS_ENVIRONMENT,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_ENVIRONMENT,
		p_validator, static_cast<PBValidator*>(&environmentFilter),
		p_subtexno, kSubTex_Environment,
		end,
	kMainPB_Photon, _T("Photon"), TYPE_TEXMAP, P_SUBANIM, IDS_PHOTON,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_PHOTON,
		p_validator, static_cast<PBValidator*>(&photonFilter),
		p_subtexno, kSubTex_Photon,
		end,
	kMainPB_PhotonVol, _T("PhotonVolume"), TYPE_TEXMAP, P_SUBANIM, IDS_PHOTONVOL,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_PHOTONVOL,
		p_validator, static_cast<PBValidator*>(&photonVolFilter),
		p_subtexno, kSubTex_PhotonVol,
		end,
	
	kMainPB_SurfaceON, _T("SurfaceOn"), TYPE_BOOL, 0, IDS_SURFACE_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_CHECK_SURFACE,
		p_default, TRUE,
		end,
	kMainPB_BumpON, _T("BumpOn"), TYPE_BOOL, 0, IDS_BUMP_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_CHECK_BUMP,
		p_default, TRUE,
		end,
	kMainPB_DisplaceON, _T("DisplaceOn"), TYPE_BOOL, 0, IDS_DISPLACE_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_CHECK_DISPLACEMENT,
		p_default, TRUE,
		end,
	kMainPB_ShadowON, _T("ShadowOn"), TYPE_BOOL, 0, IDS_SHADOW_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_CHECK_SHADOW,
		p_default, TRUE,
		end,
	kMainPB_VolumeON, _T("VolumeOn"), TYPE_BOOL, 0, IDS_VOLUME_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_CHECK_VOLUME,
		p_default, TRUE,
		end,
	kMainPB_EnvironmentON, _T("EnvironmentOn"), TYPE_BOOL, 0, IDS_ENVIRONMENT_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_CHECK_ENVIRONMENT,
		p_default, TRUE,
		end,
	kMainPB_PhotonON, _T("PhotonOn"), TYPE_BOOL, 0, IDS_PHOTON_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_CHECK_PHOTON,
		p_default, TRUE,
		end,
	kMainPB_PhotonVolON, _T("PhotonVolOn"), TYPE_BOOL, 0, IDS_PHOTONVOL_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_CHECK_PHOTONVOL,
		p_default, TRUE,
		end,
	end
);

ParamBlockDesc2 mrMaterial::m_advancedPBDesc(
	kPBID_Advanced, _T("Advanced Shaders"), IDS_ADVANCED_SHADERS, &mrMaterial::GetClassDesc(), (P_AUTO_CONSTRUCT | P_AUTO_UI), kRef_AdvancedPB,

	// Rollup
	IDD_MRMATERIAL_ADVANCED, IDS_ADVANCED_SHADERS, 0, 0, NULL,

	// Parameters
	kAdvancedPB_Contour, _T("Contour"), TYPE_TEXMAP, P_SUBANIM, IDS_CONTOUR,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_CONTOUR,
		p_validator, static_cast<PBValidator*>(&contourFilter),
		p_subtexno, kSubTex_Contour,
		end,
	kAdvancedPB_LightMap, _T("LightMap"), TYPE_TEXMAP, P_SUBANIM, IDS_LIGHTMAP,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_LIGHTMAP,
		p_validator, static_cast<PBValidator*>(&lightMapFilter),
		p_subtexno, kSubTex_LightMap,
		end,
	kAdvancedPB_ContourON, _T("ContourOn"), TYPE_BOOL, 0, IDS_CONTOUR_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_CHECK_CONTOUR,
		p_default, TRUE,
		end,
	kAdvancedPB_LightMapON, _T("LightMapOn"), TYPE_BOOL, 0, IDS_LIGHTMAP_ON,
		p_ui, TYPE_SINGLECHEKBOX, IDC_CHECK_LIGHTMAP,
		p_default, TRUE,
		end,

	end
);

ClassDesc2& mrMaterial::GetClassDesc() {

	static mrMaterial_ClassDesc classDesc;
	
	return classDesc;
}

mrMaterial::mrMaterial(bool loading) 
: m_mainPB(NULL),
  m_advancedPB(NULL),
  m_mtlPhen(NULL),
  m_surfaceShaderListShader(NULL),
  m_photonShaderListShader(NULL)
{

	if(!loading) {

		GetClassDesc().MakeAutoParamBlocks(this);
	}
}

mrMaterial::~mrMaterial() {

	DeleteAllRefs();
}

Color mrMaterial::GetAmbient(int mtlNum, BOOL backFace) {

	return Color(0,0,0);
}

Color mrMaterial::GetDiffuse(int mtlNum, BOOL backFace) {

	// Show objects grey in the viewport
	return Color (0.75,0.75,0.75); 
}

Color mrMaterial::GetSpecular(int mtlNum, BOOL backFace) {

	return Color(0,0,0);
}

float mrMaterial::GetShininess(int mtlNum, BOOL backFace) {

	return 0;
}

float mrMaterial::GetShinStr(int mtlNum, BOOL backFace) {

	return 0;
}

float mrMaterial::GetXParency(int mtlNum, BOOL backFace) {

	return 0;
}

void mrMaterial::SetAmbient(Color c, TimeValue t) {	

	// Do nothing
}
	
void mrMaterial::SetDiffuse(Color c, TimeValue t) {

	// Do nothing
}
		
void mrMaterial::SetSpecular(Color c, TimeValue t) {

	// Do nothing
}

void mrMaterial::SetShininess(float v, TimeValue t) {

	// Do nothing
}

void mrMaterial::Shade(ShadeContext& sc) {

	// Do nothing
}

bool mrMaterial::SupportsMtlOverrides() {

	// No mtl overrides needed
	return false;
}

void mrMaterial::InitializeMtlOverrides(imrMaterialCustAttrib* mtlCustAttrib) {

	// do nothing
}

imrShader* mrMaterial::GetMRShader(imrShaderCreation& shaderCreation) {

	if(m_mtlPhen == NULL) {
		// Create a default material phenomenon
		TSTR mtlName = GetFullName();
		imrShader* mtlPhen = shaderCreation.CreateShader(_T("max_default_mtl_phen"), mtlName.data());
		imrShader* surfaceShaderListShader = shaderCreation.CreateShader(_T("max_ShaderList"), mtlName.data());
		imrShader* photonShaderListShader = shaderCreation.CreateShader(_T("max_ShaderList"), mtlName.data());

		ReplaceReference(kRef_MtlPhen, ((mtlPhen != NULL) ? &mtlPhen->GetReferenceTarget() : NULL));
		ReplaceReference(kRef_SurfaceShaderList, ((surfaceShaderListShader != NULL) ? &surfaceShaderListShader->GetReferenceTarget() : NULL));
		ReplaceReference(kRef_PhotonShaderList, ((photonShaderListShader != NULL) ? &photonShaderListShader->GetReferenceTarget() : NULL));

		DbgAssert((m_mtlPhen != NULL) && (m_surfaceShaderListShader != NULL) && (m_photonShaderListShader != NULL));
	}

	return m_mtlPhen;
}

void mrMaterial::ReleaseMRShader() {

	ReplaceReference(kRef_MtlPhen, NULL);
	ReplaceReference(kRef_SurfaceShaderList, NULL);
	ReplaceReference(kRef_PhotonShaderList, NULL);
}

bool mrMaterial::NeedsAutomaticParamBlock2Translation() {

	return false;
}

void mrMaterial::TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid) {

	DbgAssert((m_mainPB != NULL) && (m_mtlPhen != NULL) && (m_mtlPhen == shader));

	if((m_mainPB != NULL) && (m_mtlPhen != NULL)) {
		// Get all the parameters
		BOOL opaque;
		Texmap* surface;
		Texmap* bump;
		Texmap* shadow;
		Texmap* displace;
		Texmap* environment;
		Texmap* contour;
		Texmap* photon;
		Texmap* photonVol;
		Texmap* volume;
		Texmap* lightMap;

		m_mainPB->GetValue(kMainPB_Opaque, t, opaque, valid);

		surface = SubTexmapOn(kSubTex_Surface) ? GetSubTexmap(kSubTex_Surface) : NULL;
		bump = SubTexmapOn(kSubTex_Bump) ? GetSubTexmap(kSubTex_Bump) : NULL;
		shadow = SubTexmapOn(kSubTex_Shadow) ? GetSubTexmap(kSubTex_Shadow) : NULL;
		displace = SubTexmapOn(kSubTex_Displace) ? GetSubTexmap(kSubTex_Displace) : NULL;
		environment = SubTexmapOn(kSubTex_Environment) ? GetSubTexmap(kSubTex_Environment) : NULL;
		photon = SubTexmapOn(kSubTex_Photon) ? GetSubTexmap(kSubTex_Photon) : NULL;
		photonVol = SubTexmapOn(kSubTex_PhotonVol) ? GetSubTexmap(kSubTex_PhotonVol) : NULL;
		volume = SubTexmapOn(kSubTex_Volume) ? GetSubTexmap(kSubTex_Volume) : NULL;
		lightMap = SubTexmapOn(kSubTex_LightMap) ? GetSubTexmap(kSubTex_LightMap) : NULL;
		contour = SubTexmapOn(kSubTex_Contour) ? GetSubTexmap(kSubTex_Contour) : NULL;

		// Get the param block which contains the shader's parameters
		IParamBlock2* paramsPB = m_mtlPhen->GetParametersParamBlock();
		DbgAssert(paramsPB != NULL);

		if(paramsPB != NULL) {

			// Set the parameters in the phenomenon
			ParamID paramID;
			if(GetParamIDByName(paramID, _T("opaque"), paramsPB))
				paramsPB->SetValue(paramID, 0, opaque);
			if(GetParamIDByName(paramID, _T("displace"), paramsPB))
				paramsPB->SetValue(paramID, 0, displace);
			if(GetParamIDByName(paramID, _T("shadow"), paramsPB))
				paramsPB->SetValue(paramID, 0, shadow);
			if(GetParamIDByName(paramID, _T("volume"), paramsPB))
				paramsPB->SetValue(paramID, 0, volume);
			if(GetParamIDByName(paramID, _T("environment"), paramsPB))
				paramsPB->SetValue(paramID, 0, environment);
			if(GetParamIDByName(paramID, _T("contour"), paramsPB))
				paramsPB->SetValue(paramID, 0, contour);
			//if(GetParamIDByName(paramID, _T("photon"), paramsPB))
			//	paramsPB->SetValue(paramID, 0, photon);
			if(GetParamIDByName(paramID, _T("photonVol"), paramsPB))
				paramsPB->SetValue(paramID, 0, photonVol);
			if(GetParamIDByName(paramID, _T("lightMap"), paramsPB))
				paramsPB->SetValue(paramID, 0, lightMap);

			// Surface shader with its shader list
			if(GetParamIDByName(paramID, _T("surface"), paramsPB)) {

				bool shaderListSuccess = false;
				
				// Create a shader list with bump->surface if possible
				// Don't bother to create a list if there is no bump shader
				if((m_surfaceShaderListShader != NULL) && (bump != NULL)) {

					// Shaders are created as texmaps, so the cast should be safe
					DbgAssert(m_surfaceShaderListShader->GetReferenceTarget().SuperClassID() == TEXMAP_CLASS_ID);
					Texmap* shaderListTexmap = static_cast<Texmap*>(&m_surfaceShaderListShader->GetReferenceTarget());

					IParamBlock2* shaderListParamsPB = m_surfaceShaderListShader->GetParametersParamBlock();

					ParamID shaderListParamID;
					if(GetParamIDByName(shaderListParamID, _T("shaderList"), shaderListParamsPB)) {

						// Clear the list
						shaderListParamsPB->SetCount(shaderListParamID, 0);

						// Add the bump shader
						if(bump != NULL) {
							shaderListParamsPB->Append(shaderListParamID, 1, &bump);
						}

						// Add the surface shader
						if(surface != NULL) {
							shaderListParamsPB->Append(shaderListParamID, 1, &surface);
						}

						paramsPB->SetValue(paramID, 0, shaderListTexmap);
						shaderListSuccess = true;
					}
				}

				if(!shaderListSuccess) {
					// No shader list shader could be created: use surface only
					paramsPB->SetValue(paramID, 0, surface);
				}
			}			

			// Photon shader with its shader list
			if(GetParamIDByName(paramID, _T("photon"), paramsPB)) {

				bool shaderListSuccess = false;
				
				// Create a shader list with photon->bump if possible
				// Don't bother to create a list if there is no bump shader
				if((m_photonShaderListShader != NULL) && (bump != NULL)) {

					// Shaders are created as texmaps, so the cast should be safe
					DbgAssert(m_photonShaderListShader->GetReferenceTarget().SuperClassID() == TEXMAP_CLASS_ID);
					Texmap* shaderListTexmap = static_cast<Texmap*>(&m_photonShaderListShader->GetReferenceTarget());

					IParamBlock2* shaderListParamsPB = m_photonShaderListShader->GetParametersParamBlock();

					ParamID shaderListParamID;
					if(GetParamIDByName(shaderListParamID, _T("shaderList"), shaderListParamsPB)) {

						// Clear the list
						shaderListParamsPB->SetCount(shaderListParamID, 0);

						// Add the bump shader
						if(bump != NULL) {
							shaderListParamsPB->Append(shaderListParamID, 1, &bump);
						}

						// Add the photon shader
						if(photon != NULL) {
							shaderListParamsPB->Append(shaderListParamID, 1, &photon);
						}

						paramsPB->SetValue(paramID, 0, shaderListTexmap);
						shaderListSuccess = true;
					}
				}

				if(!shaderListSuccess) {
					// No shader list shader could be created: use surface only
					paramsPB->SetValue(paramID, 0, photon);
				}
			}		
		}
	}
}

void mrMaterial::GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets) {

	// No additional references
}

IMtlBrowserFilter* mrMaterial::GetSubMapFilter(unsigned int i) {

	switch(i) {
	case kSubTex_Surface:
		return &surfaceFilter;
	case kSubTex_Bump:
		return &bumpFilter;
	case kSubTex_Displace:
		return &displaceFilter;
	case kSubTex_Shadow:
		return &shadowFilter;
	case kSubTex_Volume:
		return &volumeFilter;
	case kSubTex_Environment:
		return & environmentFilter;
	case kSubTex_Contour:
		return &contourFilter;
	case kSubTex_Photon:
		return &photonFilter;
	case kSubTex_PhotonVol:
		return &photonVolFilter;
	case kSubTex_LightMap:
		return &lightMapFilter;
	default:
		return NULL;
	}
}

IMtlBrowserFilter* mrMaterial::GetSubMtlFilter(unsigned int i) {

	// No sub-materials
	return NULL;
}

ParamDlg* mrMaterial::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {

	ParamDlg* dlg = GetClassDesc().CreateParamDlgs(hwMtlEdit, imp, this);

	return dlg;
}

void mrMaterial::Update(TimeValue t, Interval& valid) {

	int count = NumSubTexmaps();
	for(int i = 0; i < count; ++i) {
		Texmap* subMap = GetSubTexmap(i);
		if(subMap != NULL)
			subMap->Update(t, valid);
	}
}

void mrMaterial::Reset() {

	// Reset all PB2's
	GetClassDesc().Reset(this);
}

Interval mrMaterial::Validity(TimeValue t) {

	Interval valid = FOREVER;

	int count = NumParamBlocks();
	int i;
	for(i = 0; i < count; ++i) {
		IParamBlock2* pBlock = GetParamBlock(i);
		if(pBlock != NULL)
			pBlock->GetValidity(t, valid);
	}

	count = NumSubTexmaps();
	for(i = 0; i < count; ++i) {
		Texmap* subMap = GetSubTexmap(i);
		if(subMap != NULL)
			valid &= subMap->Validity(t);
	}

	return valid;
}

ULONG mrMaterial::LocalRequirements(int subMtlNum) {

	// Always two-sided, possibly transparent
	ULONG requirements = MTLREQ_2SIDE | MTLREQ_TRANSP;

	// Check if the material has a displacement map
	if(SubTexmapOn(kSubTex_Displace) && (GetSubTexmap(kSubTex_Displace) != NULL)) {
		requirements |= MTLREQ_DISPLACEMAP;
	}

	return requirements;
}

int mrMaterial::NumSubTexmaps() {

	return kSubTex_Count;
}

Texmap* mrMaterial::GetSubTexmap(int i) {

	ParamID paramID;
	IParamBlock2* pBlock;

	switch(i) {
	case kSubTex_Surface:
		paramID = kMainPB_Surface;
		pBlock = m_mainPB;
		break;
	case kSubTex_Bump:
		paramID = kMainPB_Bump;
		pBlock = m_mainPB;
		break;
	case kSubTex_Displace:
		paramID = kMainPB_Displace;
		pBlock = m_mainPB;
		break;
	case kSubTex_Shadow:
		paramID = kMainPB_Shadow;
		pBlock = m_mainPB;
		break;
	case kSubTex_Volume:
		paramID = kMainPB_Volume;
		pBlock = m_mainPB;
		break;
	case kSubTex_Environment:
		paramID = kMainPB_Environment;
		pBlock = m_mainPB;
		break;
	case kSubTex_Contour:
		paramID = kAdvancedPB_Contour;
		pBlock = m_advancedPB;
		break;
	case kSubTex_Photon:
		paramID = kMainPB_Photon;
		pBlock = m_mainPB;
		break;
	case kSubTex_PhotonVol:
		paramID = kMainPB_PhotonVol;
		pBlock = m_mainPB;
		break;
	case kSubTex_LightMap:
		paramID = kAdvancedPB_LightMap;
		pBlock = m_advancedPB;
		break;
	default:
		return NULL;
	}

	DbgAssert(pBlock != NULL);
	if(pBlock != NULL) {
		Texmap* subMap = pBlock->GetTexmap(paramID);
		return subMap;
	}

	return NULL;
}

void mrMaterial::SetSubTexmap(int i, Texmap *m) {

	ParamID paramID;
	IParamBlock2* pBlock;

	switch(i) {
	case kSubTex_Surface:
		paramID = kMainPB_Surface;
		pBlock = m_mainPB;
		break;
	case kSubTex_Bump:
		paramID = kMainPB_Bump;
		pBlock = m_mainPB;
		break;
	case kSubTex_Displace:
		paramID = kMainPB_Displace;
		pBlock = m_mainPB;
		break;
	case kSubTex_Shadow:
		paramID = kMainPB_Shadow;
		pBlock = m_mainPB;
		break;
	case kSubTex_Volume:
		paramID = kMainPB_Volume;
		pBlock = m_mainPB;
		break;
	case kSubTex_Environment:
		paramID = kMainPB_Environment;
		pBlock = m_mainPB;
		break;
	case kSubTex_Contour:
		paramID = kAdvancedPB_Contour;
		pBlock = m_advancedPB;
		break;
	case kSubTex_Photon:
		paramID = kMainPB_Photon;
		pBlock = m_mainPB;
		break;
	case kSubTex_PhotonVol:
		paramID = kMainPB_PhotonVol;
		pBlock = m_mainPB;
		break;
	case kSubTex_LightMap:
		paramID = kAdvancedPB_LightMap;
		pBlock = m_advancedPB;
		break;
	default:
		return;
	}

	DbgAssert(pBlock != NULL);
	if(pBlock != NULL) {
		pBlock->SetValue(paramID, 0, m);
	}
}

int mrMaterial::SubTexmapOn(int i) {

	ParamID paramID;
	IParamBlock2* pBlock;

	switch(i) {
	case kSubTex_Surface:
		paramID = kMainPB_SurfaceON;
		pBlock = m_mainPB;
		break;
	case kSubTex_Bump:
		paramID = kMainPB_BumpON;
		pBlock = m_mainPB;
		break;
	case kSubTex_Displace:
		paramID = kMainPB_DisplaceON;
		pBlock = m_mainPB;
		break;
	case kSubTex_Shadow:
		paramID = kMainPB_ShadowON;
		pBlock = m_mainPB;
		break;
	case kSubTex_Volume:
		paramID = kMainPB_VolumeON;
		pBlock = m_mainPB;
		break;
	case kSubTex_Environment:
		paramID = kMainPB_EnvironmentON;
		pBlock = m_mainPB;
		break;
	case kSubTex_Contour:
		paramID = kAdvancedPB_ContourON;
		pBlock = m_advancedPB;
		break;
	case kSubTex_Photon:
		paramID = kMainPB_PhotonON;
		pBlock = m_mainPB;
		break;
	case kSubTex_PhotonVol:
		paramID = kMainPB_PhotonVolON;
		pBlock = m_mainPB;
		break;
	case kSubTex_LightMap:
		paramID = kAdvancedPB_LightMapON;
		pBlock = m_advancedPB;
		break;
	default:
		return FALSE;
	}

	DbgAssert(pBlock != NULL);
	if(pBlock != NULL) {
		BOOL on = pBlock->GetInt(paramID);
		return on;
	}

	return FALSE;
}

TSTR mrMaterial::GetSubTexmapSlotName(int i) {

	int stringID;
	switch(i) {
	case kSubTex_Surface:
		stringID = IDS_SURFACE;
		break;
	case kSubTex_Bump:
		stringID = IDS_BUMP;
		break;
	case kSubTex_Displace:
		stringID = IDS_DISPLACEMENT;
		break;
	case kSubTex_Shadow:
		stringID = IDS_SHADOW;
		break;
	case kSubTex_Volume:
		stringID = IDS_VOLUME;
		break;
	case kSubTex_Environment:
		stringID = IDS_ENVIRONMENT;
		break;
	case kSubTex_Contour:
		stringID = IDS_CONTOUR;
		break;
	case kSubTex_Photon:
		stringID = IDS_PHOTON;
		break;
	case kSubTex_PhotonVol:
		stringID = IDS_PHOTONVOL;
		break;
	case kSubTex_LightMap:
		stringID = IDS_LIGHTMAP;
		break;
	default:
		return _T("");
	}

	return GetString(stringID);
}

int mrMaterial::MapSlotType(int i) {

	switch(i) {
		case kSubTex_Environment:
			return MAPSLOT_ENVIRON;
		default:
			return MAPSLOT_TEXTURE;
	}
}

RefTargetHandle mrMaterial::Clone(RemapDir &remap) {

	mrMaterial* newMtl = new mrMaterial(true);

	int count = NumRefs();
	for(int i = 0; i < count; ++i) {
		ReferenceTarget* refTarg = GetReference(i);
		if(refTarg != NULL) {
			newMtl->ReplaceReference(i, remap.CloneRef(refTarg));
		}
	}

	newMtl->MtlBase::operator=(*this);

	BaseClone(this, newMtl, remap);

	return newMtl;
}

int mrMaterial::NumRefs() {

	return kRef_Count;
}

RefTargetHandle mrMaterial::GetReference(int i) {

	switch(i) {
	case kRef_MainPB:
		return m_mainPB;
	case kRef_MtlPhen:
		return ((m_mtlPhen != NULL) ? &m_mtlPhen->GetReferenceTarget() : NULL);
	case kRef_SurfaceShaderList:
		return ((m_surfaceShaderListShader != NULL) ? &m_surfaceShaderListShader->GetReferenceTarget() : NULL);
	case kRef_GBufferShader:
		return NULL;
	case kRef_AdvancedPB:
		return m_advancedPB;
	case kRef_PhotonShaderList:
		return ((m_photonShaderListShader != NULL) ? &m_photonShaderListShader->GetReferenceTarget() : NULL);
	default:
		return NULL;
	}
}

void mrMaterial::SetReference(int i, RefTargetHandle rtarg) {

	switch(i) {
	case kRef_MainPB:
		DbgAssert((rtarg == NULL) || (rtarg->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID));
		m_mainPB = static_cast<IParamBlock2*>(rtarg);
		break;
	case kRef_MtlPhen:
		DbgAssert((rtarg == NULL) || IsIMRShader(rtarg));
		m_mtlPhen = GetIMRShader(rtarg);
		break;
	case kRef_SurfaceShaderList:
		DbgAssert((rtarg == NULL) || IsIMRShader(rtarg));
		m_surfaceShaderListShader = GetIMRShader(rtarg);
		break;
	case kRef_GBufferShader:
		// No longer used - ignore this reference
		break;
	case kRef_AdvancedPB:
		DbgAssert((rtarg == NULL) || (rtarg->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID));
		m_advancedPB = static_cast<IParamBlock2*>(rtarg);
		break;
	case kRef_PhotonShaderList:
		DbgAssert((rtarg == NULL) || IsIMRShader(rtarg));
		m_photonShaderListShader = GetIMRShader(rtarg);
		break;
	}
}

RefResult mrMaterial::NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message ) {

	switch(message) {
	case REFMSG_CHANGE:
		if((hTarget != NULL) && (hTarget->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID)) {
			// Param block changed: invalidate UI
			IParamBlock2* pBlock = static_cast<IParamBlock2*>(hTarget);
			IParamMap2* pMap = pBlock->GetMap();
			if(pMap != NULL)
				pMap->Invalidate();
		};
		break;
	}

	return REF_SUCCEED;
}

IOResult mrMaterial::Save(ISave *isave) {

	IOResult res;

	isave->BeginChunk(kChunk_MtlBase);
	res = MtlBase::Save(isave);
	isave->EndChunk();

	return res;
}

IOResult mrMaterial::Load(ILoad *iload) {

	IOResult res = IO_OK;
	while(iload->OpenChunk() == IO_OK) {
		switch(iload->CurChunkID()) {
		case kChunk_MtlBase:
			res = MtlBase::Load(iload);
			break;
		}

		iload->CloseChunk();

		if(res != IO_OK)
			break;
	}

	return res;
}

Class_ID mrMaterial::ClassID() {

	return MRMATERIAL_CLASS_ID;
}

SClass_ID mrMaterial::SuperClassID() {

	return MATERIAL_CLASS_ID;
}

void mrMaterial::GetClassName(TSTR& s) {

	s = GetClassDesc().ClassName();
}

void mrMaterial::DeleteThis() {

	delete this;
}

int mrMaterial::NumSubs() {

	return 2;
}

Animatable* mrMaterial::SubAnim(int i) {

	switch(i) {
	case 0:
		return m_mainPB;
	case 1:
		return m_advancedPB;
	default:
		return NULL;
	}
}

TSTR mrMaterial::SubAnimName(int i) {

	switch(i) {
	case 0:
		if(m_mainPB != NULL)
			return m_mainPB->GetLocalName();
		break;
	case 1:
		if(m_advancedPB != NULL)
			return m_advancedPB->GetLocalName();
		break;
	}

	return _T("");
}

void mrMaterial::BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev) {

	GetClassDesc().BeginEditParams(ip, this, flags, prev);
}

void mrMaterial::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next) {

	GetClassDesc().EndEditParams(ip, this, flags, next);
}

int	mrMaterial::NumParamBlocks() {

	return 1;
}

IParamBlock2* mrMaterial::GetParamBlock(int i) {

	switch(i) {
	case 0:
		return m_mainPB;
	case 1:
		return m_advancedPB;
	default:
		return NULL;
	}
}

IParamBlock2* mrMaterial::GetParamBlockByID(BlockID id) {

	switch(id) {
	case kPBID_Main:
		return m_mainPB;
	case kPBID_Advanced:
		return m_advancedPB;
	default:
		return NULL;
	}
}

BaseInterface* mrMaterial::GetInterface(Interface_ID id) {

	if(id == IMRMTLPHENTRANSLATION_INTERFACE_ID) {
		return static_cast<imrMaterialPhenomenonTranslation*>(this);
	}
	else if(id == ISUBMTLMAP_BROWSERFILTER_INTERFACEID) {
		return static_cast<ISubMtlMap_BrowserFilter*>(this);
	}
	else {
		return Mtl::GetInterface(id);
	}
}

bool mrMaterial::GetParamIDByName(ParamID& paramID, const TCHAR* name, IParamBlock2* pBlock) {

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


//==============================================================================
// class mrMaterial::mrMaterial_ClassDesc
//==============================================================================

mrMaterial::mrMaterial_ClassDesc::mrMaterial_ClassDesc() 
: m_preferenceCallbackRegistered(false)
{
	IMtlRender_Compatibility_MtlBase::Init(*this);
}

mrMaterial::mrMaterial_ClassDesc::~mrMaterial_ClassDesc() {

}

void mrMaterial::mrMaterial_ClassDesc::PreferencesChangeCallback(void* param) {

    DbgAssert(param != NULL);
    mrMaterial_ClassDesc* classDesc = static_cast<mrMaterial_ClassDesc*>(param);
    
    ClassEntry* classEntry = GetCOREInterface()->GetDllDir().ClassDir().FindClassEntry(classDesc->SuperClassID(), classDesc->ClassID());
    DbgAssert(classEntry != NULL);
    if(classEntry != NULL) {
        // Update the 'isPublic' state of the class entry. Do this by completely
        // updating the class entry
        classEntry->Set(classDesc, classEntry->DllNumber(), classEntry->ClassNumber(), classEntry->IsLoaded());
    }
}

int mrMaterial::mrMaterial_ClassDesc::IsPublic() {

	// [dl | 20may2003] Disregard the 'mr extensions active' flag.
	/*
	// Is public if and only if the mental ray extensions are ON
    imrPreferences* preferences = GetMRPreferences();
    DbgAssert(preferences != NULL);
    if(preferences != NULL) {

        // Register the preference change callback once
        if(!m_preferenceCallbackRegistered) {
            preferences->RegisterChangeCallback(PreferencesChangeCallback, this);
            m_preferenceCallbackRegistered = true;
        }
        
        // Public when the 'enable mental ray extensions' preference is ON
        return preferences->GetMRExtensionsActive();
    }
    else {
        return TRUE;
    }
	*/

	return TRUE;
}

void* mrMaterial::mrMaterial_ClassDesc::Create(BOOL loading) {

	Mtl* mtl = new mrMaterial(loading != 0);
	return mtl;
}

const TCHAR* mrMaterial::mrMaterial_ClassDesc::ClassName() {

	return GetString(IDS_CLASS_NAME);
}

SClass_ID mrMaterial::mrMaterial_ClassDesc::SuperClassID() {

	return MATERIAL_CLASS_ID;
}

Class_ID mrMaterial::mrMaterial_ClassDesc::ClassID() {

	return MRMATERIAL_CLASS_ID;
}

const TCHAR* mrMaterial::mrMaterial_ClassDesc::Category() {

	return GetString(IDS_CATEGORY);
}

const TCHAR* mrMaterial::mrMaterial_ClassDesc::InternalName() {

	return _T("mrMaterial");
}

HINSTANCE mrMaterial::mrMaterial_ClassDesc::HInstance() {

	extern HINSTANCE hInstance;
	return hInstance;
}

bool mrMaterial::mrMaterial_ClassDesc::IsCompatibleWithRenderer(ClassDesc& rendererClassDesc) {

	// Only compatible with mental ray
	return ((rendererClassDesc.ClassID() == MRRENDERER_CLASSID) != 0);
}

bool mrMaterial::mrMaterial_ClassDesc::GetCustomMtlBrowserIcon(HIMAGELIST& hImageList, int& inactiveIndex, int& activeIndex, int& disabledIndex) {

	if(PrivateResourceManager::GetBrowserIcons() == NULL) {
		// Load the icons image list
		PrivateResourceManager::GetBrowserIcons() = ImageList_Create(12, 12, ILC_MASK, 4, 0);
		HBITMAP hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_MATICONS));
		HBITMAP hMask = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_MATICONS_MASK));
		int res = ImageList_Add(PrivateResourceManager::GetBrowserIcons(), hBitmap, hMask);
	}

	hImageList = PrivateResourceManager::GetBrowserIcons();

	inactiveIndex = 0;
	activeIndex = 0;
	disabledIndex = 2;

	return true;
}

