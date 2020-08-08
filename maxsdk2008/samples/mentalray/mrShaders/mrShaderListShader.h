/*==============================================================================

  file:     mrShaderListShader.h

  author:   Daniel Levesque

  created:  25 April 2003

  description:

      Implementation of a UI for the shader list shader.

  modified:	


© 2003 Autodesk
==============================================================================*/

#ifndef _MRSHADERLISTSHADER_H_
#define _MRSHADERLISTSHADER_H_

#include <max.h>
#include <imtl.h>
#include <iparamb2.h>
#include <iparamm2.h>
#include <IMtlRender_Compatibility.h>
#include <IMtlBrowserFilter.h>

#include <mentalray\imrShaderTranslation.h>
#include <mentalray\shared_src\mrShaderFilter.h>
#include <mentalray\shared_src\mrShaderButtonHandler.h>

#define MRSHADERLISTSHADER_CLASS_ID	Class_ID(0x76774922, 0x752b12a8)

//==============================================================================
// class mrShaderListShader
//
// Implementation of the shader list shader. This texmap plugin only serves
// the purpose of defining a nice UI for the shader list shader.
//==============================================================================
class mrShaderListShader : public Texmap, public imrShaderTranslation, public ISubMtlMap_BrowserFilter, public PostLoadCallback {

public:

	// Parameter block IDs
	enum ParamBlockID {
		kMainPBID = 0
	};

	// Main PB param IDs
	enum MainPBParamID {
		kMainPID_ShaderOnList = 0,
		kMainPID_ShaderList = 1,
		kMainPID_CurrentShader = 2
	};

	enum RefNum {
		kRef_MainPB = 0,		// Main param block
		kRef_mrShader = 1,		// the shader used for translation

		kRef_Count
	};

	class mrShaderListShader_ClassDesc;

	// Get the class descriptors
	static int GetNumClassDescs();
	static ClassDesc2& GetClassDesc(int i);

	mrShaderListShader(
		mrShaderListShader_ClassDesc& classDesc,	// The class descriptor
		bool loading
	);
	virtual ~mrShaderListShader();

	// Manipulate the shader list
	void AppendShader(Texmap* shader, bool on);
	void ReplaceShader(unsigned int i, Texmap* shader);
	void RemoveShader(unsigned int i);
	Texmap* GetShader(unsigned int i);
	bool GetShaderOn(unsigned int i);
	void SetShaderOn(unsigned int i, bool on);
	int GetNumShaders();
	void SwapShaders(unsigned int a, unsigned int b);	// Swaps shader positions int he list

	// Removes all NULL shaders from the list
	void RemoveNullShaders();

	// Get/set the index of the selected shader
	int GetSelectedItem();
	void SetSelectedItem(int i);

	// Copies the active shader slot back to the shader list
	void CommitActiveShaderToList();

	// Sets the active shader slot
	void SetActiveShader(Texmap* activeShader);

	// -- from PostLoadCallback
	virtual void proc(ILoad *iload);

	// -- from Texmap
	virtual	AColor EvalColor(ShadeContext& sc);
	virtual	Point3 EvalNormalPerturb(ShadeContext& sc);

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

	// -- from ISubMap
	virtual int NumSubTexmaps();
	virtual Texmap* GetSubTexmap(int i);
	virtual void SetSubTexmap(int i, Texmap *m);
	virtual int SubTexmapOn(int i);
	virtual TSTR GetSubTexmapSlotName(int i);

	// -- from ReferenceTarget
	virtual RefTargetHandle Clone(RemapDir &remap = DefaultRemapDir());

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

	// I/O chunk IDs
	enum ChunkID {
		kChunk_MtlBase = 0x0100
	};

	class mrShaderListShader_UserDlgProc;

	// -- Data Members --

	// The array of class descriptors
	// A class descriptor is created for each apply type of the shader list shader.
	// This allows creating shader lists which only accept certain types of sub-shaders.
	static mrShaderListShader_ClassDesc m_classDescs[];

	// The class descriptor for this instance
	mrShaderListShader_ClassDesc& m_classDesc;

	// Main param block
	IParamBlock2* m_mainPB;

	// Shader used for custom translation
	imrShader* m_translationShader;

	// The shader filter for the submaps of this class
	mrShaderFilter m_subMapFilter;

	// The index of the shader currently selected in the list
	int m_selectedShader;

	// When true, the active shader is not commited on paramblk changes
	bool m_dontCommitActiveShader;
};

//==============================================================================
// class mrShaderListShader::mrShaderListShader_UserDlgProc
//
// User dialog proc for the shader list's parameter map
//==============================================================================
class mrShaderListShader::mrShaderListShader_UserDlgProc : public ParamMap2UserDlgProc/*, public mrShaderButtonHandler*/ {

public:

	mrShaderListShader_UserDlgProc(unsigned int applyTypes);
	virtual ~mrShaderListShader_UserDlgProc();

	// -- from ParamMap2UserDlgProc
	virtual INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual void DeleteThis();
	virtual void Update(TimeValue t, Interval& valid, IParamMap2* pmap);
	virtual void SetThing(ReferenceTarget *m);

protected:

	// -- from mrShaderButtonHandler
	/*
	virtual void SetShader(Texmap* shader);
	virtual Texmap* GetShader();
	virtual const TCHAR* GetNoneString();
	*/

private:

	// -- Data members --

	// The apply types accepted for this shader
	unsigned int m_applyTypes;
	// The texmap plugin
	mrShaderListShader* m_theTexmap;
};

//==============================================================================
// class mrShaderListShader::mrShaderListShader_ClassDesc
//
// Class Descriptor for the mental ray material.
//==============================================================================
class mrShaderListShader::mrShaderListShader_ClassDesc : public ClassDesc2, public IMtlRender_Compatibility_MtlBase, public imrShaderTranslation_ClassInfo {

public:

    mrShaderListShader_ClassDesc(
		unsigned int applyTypes,		// The apply types for this shader list
		int typeStringResID,				// Resource ID for string that describes the apply type
		const TCHAR* internalTypeString,	// Internal (non-localized) type string
		Class_ID classID,				// The class ID for this class
		bool isPublic = true
	);
	virtual ~mrShaderListShader_ClassDesc();

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

	// -- from imrShaderTranslation_ClassInfo
	virtual unsigned int GetApplyTypes();

private:
	
	// The apply types for this shader list class
	unsigned int m_applyTypes;
	// Resource ID for string that describes the apply type
	int m_typeStringResID;
	// Class ID for this class
	Class_ID m_classID;
	// Internal apply type string description
	TSTR m_internalName;
	// The param block descriptor associated with this calss
	ParamBlockDesc2 m_mainPBDesc;
	// The user dialog proc handler
	mrShaderListShader_UserDlgProc m_userDlgProc;
	// Is this class public?
	bool m_isPublic;
};


#endif



