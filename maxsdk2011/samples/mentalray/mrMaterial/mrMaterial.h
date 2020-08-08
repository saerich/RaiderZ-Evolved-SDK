/*==============================================================================

  file:     mrMaterial.h

  author:   Daniel Levesque

  created:  3mar2003

  description:

      Implementation of the generic mental ray material. The material is translated
	  using the custom shader translation interface, and is provided as a sample
	  for the use of that interface.

  modified:	


© 2003 Autodesk
==============================================================================*/

#ifndef _MRMATERIAL_H_
#define _MRMATERIAL_H_

#include <max.h>
#include <imtl.h>
#include <iparamb2.h>
#include <IMtlBrowserFilter.h>
#include <IMtlRender_Compatibility.h>

#include <mentalray\imrShaderTranslation.h>

#define MRMATERIAL_CLASS_ID	Class_ID(0x6926ba21, 0x7a10aca5)

class imrShader;
class ClassDesc2;

//==============================================================================
// class mrMaterial
//
// Generic mental ray material. The material exposes the properties of the 
// mental ray material (the 'opaque' flag, and all the shader slots).
//==============================================================================
class mrMaterial : public Mtl, public imrMaterialPhenomenonTranslation, public ISubMtlMap_BrowserFilter {

public:

	static ClassDesc2& GetClassDesc();

	explicit mrMaterial(bool loading);
	virtual ~mrMaterial();

	// -- from Mtl
	virtual Color GetAmbient(int mtlNum=0, BOOL backFace=FALSE);
	virtual Color GetDiffuse(int mtlNum=0, BOOL backFace=FALSE);
	virtual Color GetSpecular(int mtlNum=0, BOOL backFace=FALSE);
	virtual float GetShininess(int mtlNum=0, BOOL backFace=FALSE);
	virtual float GetShinStr(int mtlNum=0, BOOL backFace=FALSE);
	virtual float GetXParency(int mtlNum=0, BOOL backFace=FALSE);
	virtual void SetAmbient(Color c, TimeValue t);		
	virtual void SetDiffuse(Color c, TimeValue t);		
	virtual void SetSpecular(Color c, TimeValue t);
	virtual void SetShininess(float v, TimeValue t);
	virtual void Shade(ShadeContext& sc);

	// -- from imrMaterialPhenomenonTranslation
	virtual bool SupportsMtlOverrides();
	virtual void InitializeMtlOverrides(imrMaterialCustAttrib* mtlCustAttrib);

	// -- from imrShaderTranslation
	virtual imrShader* GetMRShader(imrShaderCreation& shaderCreation);
	virtual void ReleaseMRShader();
	virtual bool NeedsAutomaticParamBlock2Translation();
	virtual void TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid);
	virtual void GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets);

	// -- from ISubMtlMap_BrowserFilter
	virtual IMtlBrowserFilter* GetSubMapFilter(unsigned int i);
	virtual IMtlBrowserFilter* GetSubMtlFilter(unsigned int i);

	// -- from MtlBase
	virtual ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
	virtual void Update(TimeValue t, Interval& valid);
	virtual void Reset();
	virtual Interval Validity(TimeValue t);
	virtual ULONG LocalRequirements(int subMtlNum);
	
	// -- from ISubMap
	virtual int NumSubTexmaps();
	virtual Texmap* GetSubTexmap(int i);
	virtual void SetSubTexmap(int i, Texmap *m);
	virtual int SubTexmapOn(int i);
	virtual TSTR GetSubTexmapSlotName(int i);
	virtual int MapSlotType(int i);

	// -- from ReferenceTarget
	virtual RefTargetHandle Clone(RemapDir &remap);

	// -- from ReferenceMaker
 	virtual int NumRefs();
	virtual RefTargetHandle GetReference(int i);
	virtual void SetReference(int i, RefTargetHandle rtarg);
	virtual RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message );
	virtual IOResult Save(ISave *isave);
	virtual IOResult Load(ILoad *iload);
	
	// -- from Animatable
	virtual Class_ID ClassID();
	virtual SClass_ID SuperClassID();
	virtual void GetClassName(TSTR& s);
	virtual void DeleteThis();
	virtual int NumSubs();
	virtual Animatable* SubAnim(int i);
	virtual TSTR SubAnimName(int i);
	virtual void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev=NULL);
	virtual void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next=NULL);
	virtual int	NumParamBlocks();
	virtual IParamBlock2* GetParamBlock(int i);
	virtual IParamBlock2* GetParamBlockByID(BlockID id);

	// -- from InterfaceServer
	virtual BaseInterface* GetInterface(Interface_ID id);

private:

	// Subtexmap numbers
	enum SubTexmapNum {
		kSubTex_Surface,
		kSubTex_Bump,
		kSubTex_Displace,
		kSubTex_Shadow,
		kSubTex_Volume,
		kSubTex_Environment,
		kSubTex_Contour,
		kSubTex_Photon,
		kSubTex_PhotonVol,
		kSubTex_LightMap,

		kSubTex_Count
	};

	// Param Block IDs
	enum PBlockID {
		kPBID_Main = 0,
		kPBID_Advanced = 1
	};

	// Main PB param IDs
	enum MainPBParamID {
		kMainPB_Opaque = 0,
		kMainPB_Surface = 1,
		kMainPB_Bump = 2,
		kMainPB_Displace = 3,
		kMainPB_Shadow = 4,
		kMainPB_Volume = 5,
		kMainPB_Environment = 6,
		//kMainPB_Contour = 7,		// defunct
		kMainPB_Photon = 8,
		kMainPB_PhotonVol = 9,
		//kMainPB_LightMap = 10,	// defunct
		
		kMainPB_SurfaceON = 11,
		kMainPB_BumpON = 12,
		kMainPB_DisplaceON = 13,
		kMainPB_ShadowON = 14,
		kMainPB_VolumeON = 15,
		kMainPB_EnvironmentON = 16,
		kMainPB_PhotonON = 17,
		kMainPB_PhotonVolON = 18
	};

	// Advanced PB param IDs
	enum AdvancedPBParamID {
		kAdvancedPB_Contour = 0,
		kAdvancedPB_LightMap = 1,
		kAdvancedPB_ContourON = 2,
		kAdvancedPB_LightMapON = 3
	};

	enum RefNum {
		kRef_MainPB = 0,
		kRef_MtlPhen = 1,
		kRef_SurfaceShaderList = 2,
		kRef_GBufferShader = 3,		// No longer used
		kRef_AdvancedPB = 4,
		kRef_PhotonShaderList = 5,

		kRef_Count
	};

	// I/O chunk IDs
	enum ChunkID {
		kChunk_MtlBase = 0x0100
	};

	class mrMaterial_ClassDesc;
	class mrMaterialPLCB;

	// Searches a parameter by name in a given param block
	bool GetParamIDByName(ParamID& paramID, const TCHAR* name, IParamBlock2* pBlock);

	// -- Data Members --

	// The main param block descriptor
	static ParamBlockDesc2 m_mainPBDesc;
	// The advanced param block descriptor
	static ParamBlockDesc2 m_advancedPBDesc;
	
	// The main param block
	IParamBlock2* m_mainPB;
	// The advanced param block
	IParamBlock2* m_advancedPB;

	// The material phenomenon for custom translation
	imrShader* m_mtlPhen;
	// The shader list shader for surface custom translation
	imrShader* m_surfaceShaderListShader;
	// The shader list shader for photon custom translation
	imrShader* m_photonShaderListShader;

	// no longer used, but needed for loading old files. 
	RefTargetHandle m_gbufferShader; 
};

//==============================================================================
// class mrMaterial::mrMaterial_ClassDesc
//
// Class Descriptor for the mental ray material.
//==============================================================================
class mrMaterial::mrMaterial_ClassDesc : public ClassDesc2, public IMtlRender_Compatibility_MtlBase {

public:

    mrMaterial_ClassDesc();
	virtual ~mrMaterial_ClassDesc();

	virtual int IsPublic();
	virtual void* Create(BOOL loading);
	virtual const TCHAR* ClassName();
	virtual SClass_ID SuperClassID();
	virtual Class_ID ClassID();
	virtual const TCHAR* Category();
	virtual const TCHAR* InternalName();
	virtual HINSTANCE HInstance();

	// -- from IMtlRender_Compability_MtlBase
	virtual bool IsCompatibleWithRenderer(ClassDesc& rendererClassDesc);
	virtual bool GetCustomMtlBrowserIcon(HIMAGELIST& hImageList, int& inactiveIndex, int& activeIndex, int& disabledIndex);

private:
	
    // Prefernces change callback. Used to detect when the 'mr extensions'
	// are turned ON/OFF and enable/disable the mental ray material.
    static void PreferencesChangeCallback(void* param);

    bool m_preferenceCallbackRegistered;

};

//==============================================================================
// class mrMaterial::mrMaterialPLCB
//
// Postload callback used to remove obsolete reftargs
//==============================================================================
class mrMaterial::mrMaterialPLCB : public PostLoadCallback 
{
public:
	mrMaterialPLCB( mrMaterial* pMtl )	{ mpMtl = pMtl; }
	virtual int Priority() { return 8; }  // we are in no hurry...

	void proc(ILoad *iload);

private:
	mrMaterial* mpMtl;
};


#endif
