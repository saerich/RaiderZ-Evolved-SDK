/*==============================================================================

  file:     mrShaderListShader.cpp

  author:   Daniel Levesque

  created:  25 April 2003

  description:

      Implementation of a UI for the shader list shader.

  modified:	


© 2003 Autodesk
==============================================================================*/

#include "mrShaderListShader.h"
#include "resource.h"

#include <mentalray\imrShader.h>
#include <mentalray\imrShaderClassDesc.h>
#include <mentalray\mentalrayInMax.h>

extern TCHAR *GetString(int id);

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
// class mrShaderListShader
//==============================================================================

mrShaderListShader::mrShaderListShader_ClassDesc mrShaderListShader::m_classDescs[] = {
	mrShaderListShader_ClassDesc(imrShaderClassDesc::kApplyFlag_Default, IDS_TEXTURE, _T("Texture"), Class_ID(0x18176064, 0x720c6edc)),
	mrShaderListShader_ClassDesc(imrShaderClassDesc::kApplyFlag_Lens, IDS_LENS, _T("Lens"), Class_ID(0x31ab2269, 0x72a0409a)),
	mrShaderListShader_ClassDesc(imrShaderClassDesc::kApplyFlag_Shadow, IDS_SHADOW, _T("Shadow"), Class_ID(0x697a2aff, 0x28626df)),
	mrShaderListShader_ClassDesc(imrShaderClassDesc::kApplyFlag_Environment, IDS_ENVIRONMENT, _T("Environment"), Class_ID(0xc5d3a9a, 0x813618a)),
	mrShaderListShader_ClassDesc(imrShaderClassDesc::kApplyFlag_Volume, IDS_VOLUME, _T("Volume"), Class_ID(0x58b2269a, 0x2f151180)),
	mrShaderListShader_ClassDesc(imrShaderClassDesc::kApplyFlag_Displace, IDS_DISPLACEMENT, _T("Displacement"), Class_ID(0x7af956e2, 0x1f192c4c), false),	// Displacement shader is not public - It wouldn't even work.
	mrShaderListShader_ClassDesc(imrShaderClassDesc::kApplyFlag_Output, IDS_OUTPUT, _T("Output"), Class_ID(0x224051a9, 0x69a1360b)),
	mrShaderListShader_ClassDesc(imrShaderClassDesc::kApplyFlag_PhotonVol, IDS_PHOTONVOLUME, _T("Photon Volume"), Class_ID(0x45203748, 0x296612b6)),
	mrShaderListShader_ClassDesc(imrShaderClassDesc::kApplyFlag_Bump, IDS_BUMP, _T("Bump"), Class_ID(0x66d05c36, 0xf7349a2), false)		// Bump shader is not public - I don't think it's necessary since the texture shader can be used for bumps
};

int mrShaderListShader::GetNumClassDescs() {

	return sizeof(m_classDescs) / sizeof(m_classDescs[0]);

}

ClassDesc2& mrShaderListShader::GetClassDesc(int i) {

	if(i < GetNumClassDescs())
		return m_classDescs[i];
	else
		return m_classDescs[0];
}

mrShaderListShader::mrShaderListShader(
	mrShaderListShader_ClassDesc& classDesc,	// The class descriptor
	bool loading
)
: m_classDesc(classDesc),
  m_mainPB(NULL),
  m_translationShader(NULL),
  m_subMapFilter(classDesc.GetApplyTypes(), static_cast<ParamType2>(TYPE_RGBA), false),
  m_selectedShader(-1),
  m_dontCommitActiveShader(false)
{
	if(!loading) {
		m_classDesc.MakeAutoParamBlocks(this);
	}
}

mrShaderListShader::~mrShaderListShader() {

	DeleteAllRefs();
}

void mrShaderListShader::AppendShader(Texmap* shader, bool on) {

	if(m_mainPB != NULL) {
		m_dontCommitActiveShader = true;
		m_mainPB->Append(kMainPID_ShaderList, 1, &shader);
		BOOL onVal = on;
		m_mainPB->Append(kMainPID_ShaderOnList, 1, &onVal);
		m_dontCommitActiveShader = false;
	}
	else {
		DbgAssert(false);
	}
}

void mrShaderListShader::ReplaceShader(unsigned int i, Texmap* shader) {

	if(shader == NULL) {
		RemoveShader(i);
	}
	else if(i >= GetNumShaders()) {
		AppendShader(shader, TRUE);
	}
	else if(m_mainPB != NULL) {
		m_dontCommitActiveShader = true;
		m_mainPB->SetValue(kMainPID_ShaderList, 0, shader, i);
		m_dontCommitActiveShader = false;
	}
	else {
		DbgAssert(false);
	}
}

void mrShaderListShader::RemoveShader(unsigned int i) {

	if(m_mainPB != NULL) {
		if(i < m_mainPB->Count(kMainPID_ShaderList)) {
			DbgAssert(m_mainPB->Count(kMainPID_ShaderList) == m_mainPB->Count(kMainPID_ShaderOnList));
			m_dontCommitActiveShader = true;
			m_mainPB->Delete(kMainPID_ShaderList, i, 1);
			m_mainPB->Delete(kMainPID_ShaderOnList, i, 1);
			m_dontCommitActiveShader = false;
		}
	}
	else {
		DbgAssert(false);
	}
}

Texmap* mrShaderListShader::GetShader(unsigned int i) {

	if(m_mainPB != NULL) {
		if(i < m_mainPB->Count(kMainPID_ShaderList)) {
			Texmap* texmap = m_mainPB->GetTexmap(kMainPID_ShaderList, 0, i);
			return texmap;
		}
		else {
			return NULL;
		}
	}
	else {
		DbgAssert(false);
		return NULL;
	}
}

bool mrShaderListShader::GetShaderOn(unsigned int i) {

	if(m_mainPB != NULL) {
		if(i < m_mainPB->Count(kMainPID_ShaderOnList)) {
			BOOL val = m_mainPB->GetInt(kMainPID_ShaderOnList, 0, i);
			return (val != 0);
		}
		else {
			return false;
		}
	}
	else {
		DbgAssert(false);
		return false;
	}
}

void mrShaderListShader::SetShaderOn(unsigned int i, bool on) {

	if(m_mainPB != NULL) {
		int count = m_mainPB->Count(kMainPID_ShaderOnList);
		DbgAssert(m_mainPB->Count(kMainPID_ShaderList) == m_mainPB->Count(kMainPID_ShaderOnList));
		if(i < count) {
			m_dontCommitActiveShader = true;
			m_mainPB->SetValue(kMainPID_ShaderOnList, 0, static_cast<BOOL>(on), i);
			m_dontCommitActiveShader = false;
		}
	}
	else {
		DbgAssert(false);
	}
}

int mrShaderListShader::GetNumShaders() {

	if(m_mainPB != NULL) {
		int count = m_mainPB->Count(kMainPID_ShaderList);
		DbgAssert(count == m_mainPB->Count(kMainPID_ShaderOnList));
		return count;
	}
	else {
		DbgAssert(false);
		return 0;
	}
}

void mrShaderListShader::SwapShaders(unsigned int a, unsigned int b) {

	int count = GetNumShaders();
	if((a < 0) || (a >= count) || (b < 0) || (b >= count)) {
		DbgAssert(false);
	}
	else {
		Texmap* shaderA = GetShader(a);
		bool shaderAon = GetShaderOn(a);
		Texmap* shaderB = GetShader(b);
		bool shaderBon = GetShaderOn(b);

		// Append shader A so that it doesn't get deleted when we replace it
		AppendShader(shaderA, shaderAon);
		
		// Replace shader A by shader B
		ReplaceShader(a, shaderB);
		SetShaderOn(a, shaderBon);

		// Replace shader B by shader A
		ReplaceShader(b, shaderA);
		SetShaderOn(b, shaderAon);

		// Remove temporary shader A from the end of the list
		DbgAssert(GetNumShaders() == (count + 1));
		RemoveShader(count);
	}
}

void mrShaderListShader::RemoveNullShaders() {

	for(int i = 0; i < GetNumShaders(); ++i) {

		Texmap* shader = GetShader(i);
		if(shader == NULL) {
			RemoveShader(i);
			--i;
		}
	}
}

int mrShaderListShader::GetSelectedItem() {

	return m_selectedShader;
}

void mrShaderListShader::SetSelectedItem(int i) {

	m_selectedShader = i;

	if(m_mainPB != NULL) {
		// Copy the shader from the list to the 'active' slot
		int count = GetNumShaders();
		if((i >= 0) && (i < count)) {
			Texmap* selectedShader = GetShader(i);
			m_dontCommitActiveShader = true;
			m_mainPB->SetValue(kMainPID_CurrentShader, 0, selectedShader);
			m_dontCommitActiveShader = false;
		}
		else {
			// No selection
			m_dontCommitActiveShader = true;
			m_mainPB->SetValue(kMainPID_CurrentShader, 0, static_cast<Texmap*>(NULL));
			m_dontCommitActiveShader = false;
		}
	}
	else {
		DbgAssert(false);
	}
}

void mrShaderListShader::CommitActiveShaderToList() {

	if(m_mainPB != NULL) {
		int count = GetNumShaders();
		Texmap* activeShader = m_mainPB->GetTexmap(kMainPID_CurrentShader);
		if((m_selectedShader >= 0) && (m_selectedShader < count)) {
			// Copy the active shader to the list
			ReplaceShader(m_selectedShader, activeShader);
			// Reset the selection
			SetSelectedItem(m_selectedShader);
		}
		else {
			// Add the shader to the list
			AppendShader(activeShader, true);
			// Reset the selection to the last item
			int count = GetNumShaders();
			SetSelectedItem(count - 1);
		}
	}
	else {
		DbgAssert(false);
	}
}

AColor mrShaderListShader::EvalColor(ShadeContext& sc) {

	// Return 60% grey
	return AColor(0.6,0.6,0.6);
}

Point3 mrShaderListShader::EvalNormalPerturb(ShadeContext& sc) {

	return Point3(0,0,0);
}

imrShader* mrShaderListShader::GetMRShader(imrShaderCreation& shaderCreation) {

	if(m_translationShader == NULL) {
		// Create the shader
		TSTR texmapName = GetFullName();
		imrShader* shader = shaderCreation.CreateShader(_T("max_ShaderList"), texmapName.data());

		ReplaceReference(kRef_mrShader, ((shader != NULL) ? &shader->GetReferenceTarget() : NULL));

		DbgAssert(m_translationShader != NULL);
	}

	return m_translationShader;
}

void mrShaderListShader::ReleaseMRShader() {

	ReplaceReference(kRef_mrShader, NULL);
}

bool mrShaderListShader::NeedsAutomaticParamBlock2Translation() {

	// The shader is translated manually
	return false;
}

void mrShaderListShader::TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid) {

	DbgAssert(shader == m_translationShader);

	IParamBlock2* pBlock = shader->GetParametersParamBlock();
	pBlock->ZeroCount(0);
	int count = GetNumShaders();
	pBlock->Resize(0, count);
	for(int i = 0; i < count; ++i) {
		Texmap* shader = GetShader(i);
		if((shader != NULL) && GetShaderOn(i))
			pBlock->Append(0, 1, &shader);
	}
}

void mrShaderListShader::GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets) {

	// none
}

IMtlBrowserFilter* mrShaderListShader::GetSubMapFilter(unsigned int i) {

	// Same filter for all submaps
	return &m_subMapFilter;
}

IMtlBrowserFilter* mrShaderListShader::GetSubMtlFilter(unsigned int i) {

	// No sub materials
	return NULL;
}

ParamDlg* mrShaderListShader::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {

	return m_classDesc.CreateParamDlgs(hwMtlEdit, imp, this);
}

void mrShaderListShader::Update(TimeValue t, Interval& valid) {

	// Update the sub textures
	int count = NumSubTexmaps();
	for(int i = 0; i < count; ++i) {
		Texmap* subMap = GetSubTexmap(i);
		if(subMap != NULL)
			subMap->Update(t, valid);
	}
}

void mrShaderListShader::Reset() {

	// Reset all PB2's
	m_classDesc.Reset(this);
}

Interval mrShaderListShader::Validity(TimeValue t) {

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

int mrShaderListShader::NumSubTexmaps() {

	int count = GetNumShaders();
	return count + 1;		// count for the active shader
}

Texmap* mrShaderListShader::GetSubTexmap(int i) {

	if(i == 0) {
		// Return the active shader
		Texmap* texmap = m_mainPB->GetTexmap(kMainPID_CurrentShader);
		return texmap;
	}
	else {
		int count = GetNumShaders();
		int shaderIndex = i - 1;
		if(shaderIndex < count) {
			Texmap* shader = GetShader(shaderIndex);
			return shader;
		}
		else {
			return NULL;
		}
	}
}

void mrShaderListShader::SetSubTexmap(int i, Texmap *m) {

	if(i == 0) {
		// Set the active shader
		m_mainPB->SetValue(kMainPID_CurrentShader, 0, m);
	}
	else {
		int count = GetNumShaders();
		int shaderIndex = i - 1;
		if(shaderIndex < count) {
			if(m != NULL) {
				ReplaceShader(shaderIndex, m);
			}
			else {
				RemoveShader(shaderIndex);
			}
		}
		else {
			// Append to the list if the array is too small.
			AppendShader(m, true);
		}
	}
}

int mrShaderListShader::SubTexmapOn(int i) {

	if(i == 0) {
		// The active shader is never ON: it's not used for rendering, but only for UI
		return FALSE;
	}
	else {
		int count = GetNumShaders();
		int shaderIndex = i - 1;
		if(shaderIndex < count)
			return GetShaderOn(shaderIndex);
		else
			return FALSE;
	}
}

TSTR mrShaderListShader::GetSubTexmapSlotName(int i) {

	if(i == 0) {
		return GetString(IDS_SELECTED);
	}
	else {
		// The name is just the index of the texmap
		TSTR name;
		name.printf(_T("%d"), i);
		return name;
	}
}

RefTargetHandle mrShaderListShader::Clone(RemapDir &remap) {

	mrShaderListShader* pNew = new mrShaderListShader(m_classDesc, true);
    
	int count = NumRefs();
	for(int i = 0; i < count; ++i) {
		ReferenceTarget* refTarg = GetReference(i);
		pNew->ReplaceReference(i, ((refTarg != NULL) ? remap.CloneRef(refTarg) : NULL));
	}

	pNew->MtlBase::operator=(*this);

	BaseClone(this, pNew, remap);

	return pNew;
}

int mrShaderListShader::NumRefs() {

	return kRef_Count;
}

RefTargetHandle mrShaderListShader::GetReference(int i) {

	switch(i) {
	case kRef_MainPB:
		return m_mainPB;
	case kRef_mrShader:
		return ((m_translationShader != NULL) ? &m_translationShader->GetReferenceTarget() : NULL);
	default:
		return NULL;
	}
}

void mrShaderListShader::SetReference(int i, RefTargetHandle rtarg) {

	switch(i) {
	case kRef_MainPB:
		DbgAssert((rtarg == NULL) || (rtarg->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID));
		if((rtarg == NULL) || (rtarg->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID))
			m_mainPB = static_cast<IParamBlock2*>(rtarg);
		break;
	case kRef_mrShader:
		DbgAssert((rtarg == NULL) || IsIMRShader(rtarg));
		if((rtarg == NULL) || IsIMRShader(rtarg))
			m_translationShader = GetIMRShader(rtarg);
		break;
	}
}

RefResult mrShaderListShader::NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message ) {

	switch(message) {
	case REFMSG_CHANGE:
		if((hTarget != NULL) && (hTarget->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID)) {
			// Param block changed: invalidate UI
			IParamBlock2* pBlock = static_cast<IParamBlock2*>(hTarget);

			// If the main param block changes, copy the active shader back to the list - in case the active shader changed
			if((pBlock == m_mainPB) && !m_dontCommitActiveShader)
				CommitActiveShaderToList();

			IParamMap2* pMap = pBlock->GetMap();
			if(pMap != NULL)
				pMap->Invalidate();
		};
		break;
	}

	return REF_SUCCEED;
}

IOResult mrShaderListShader::Save(ISave *isave) {

	IOResult res;

	isave->BeginChunk(kChunk_MtlBase);
	res = MtlBase::Save(isave);
	isave->EndChunk();

	return res;
}

IOResult mrShaderListShader::Load(ILoad *iload) {

	// Don't commit the active shader while loading
	m_dontCommitActiveShader = true;
	iload->RegisterPostLoadCallback(this);

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

void mrShaderListShader::proc(ILoad *iload) {

	// Accept changes in the active shader
	m_dontCommitActiveShader = false;
}

Class_ID mrShaderListShader::ClassID() {

	return m_classDesc.ClassID();
}

SClass_ID mrShaderListShader::SuperClassID() {

	return m_classDesc.SuperClassID();
}

void mrShaderListShader::GetClassName(TSTR& s) {

	s = m_classDesc.ClassName();
}

void mrShaderListShader::DeleteThis() {

	delete this;
}

int mrShaderListShader::NumSubs() {

	return 1;
}

Animatable* mrShaderListShader::SubAnim(int i) {

	switch(i) {
	case 0:
		return m_mainPB;
	default:
		return NULL;
	}
}

TSTR mrShaderListShader::SubAnimName(int i) {

	switch(i) {
	case 0:
		if(m_mainPB != NULL)
			return m_mainPB->GetLocalName();
		break;
	}

	return _T("");
}

void mrShaderListShader::BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev) {

	m_classDesc.BeginEditParams(ip, this, flags, prev);
}

void mrShaderListShader::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next) {

	m_classDesc.EndEditParams(ip, this, flags, next);
}

int	mrShaderListShader::NumParamBlocks() {

	return 1;
}

IParamBlock2* mrShaderListShader::GetParamBlock(int i) {

	switch(i) {
	case 0:
		return m_mainPB;
	default:
		return NULL;
	}
}

IParamBlock2* mrShaderListShader::GetParamBlockByID(BlockID id) {

	switch(id) {
	case kMainPBID:
		return m_mainPB;
	default:
		return NULL;
	}
}

BaseInterface* mrShaderListShader::GetInterface(Interface_ID id) {

	if(id == IMRSHADERTRANSLATION_INTERFACE_ID) {
		imrShaderTranslation* r = this;
		return r;
	}
	else if(id == ISUBMTLMAP_BROWSERFILTER_INTERFACEID) {
		ISubMtlMap_BrowserFilter* r = this;
		return r;
	}
	else {
		return Texmap::GetInterface(id);
	}
}

//==============================================================================
// class mrShaderListShader::mrShaderListShader_ClassDesc
//==============================================================================

mrShaderListShader::mrShaderListShader_ClassDesc::mrShaderListShader_ClassDesc(
	unsigned int applyTypes,		// The apply types for this shader list
	int typeStringResID,				// Resource ID for string that describes the apply type
	const TCHAR* internalTypeString,	// Internal (non-localized) type string
	Class_ID classID,				// The class ID for this class
	bool isPublic
)
: m_applyTypes(applyTypes),
  m_typeStringResID(typeStringResID),
  m_classID(classID),
  m_userDlgProc(applyTypes),
  m_isPublic(isPublic),
  m_mainPBDesc(
	kMainPBID, _T("Shader List Parameters"), IDS_SHADERLIST_PARAMS, NULL, (P_AUTO_CONSTRUCT | P_AUTO_UI), kRef_MainPB,

	// Rollup
	IDD_SHADERLIST, IDS_SHADERLIST_PARAMS, 0, 0, NULL,

	// Parameters
	kMainPID_ShaderOnList, _T("Shaders On"), TYPE_BOOL_TAB, 0, 0, IDS_SHADERS_ON,
		p_default, TRUE,
		end,
	kMainPID_ShaderList, _T("Shaders"), TYPE_TEXMAP_TAB, 0, 0, IDS_SHADERS,
		end,
	kMainPID_CurrentShader, _T("Current Shader"), TYPE_TEXMAP, 0, IDS_CURRENTSHADER,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_SHADER,
		p_subtexno, 0,
		end,
	end
  )
{
	m_internalName.printf(_T("Shader List (%s)"), internalTypeString);

	// Set the user dialog proc of the parameter block
	m_mainPBDesc.SetClassDesc(this);
	m_mainPBDesc.dlgProc = &m_userDlgProc;

	// Initial the base interfaces
	IMtlRender_Compatibility_MtlBase::Init(*this);
	imrShaderTranslation_ClassInfo::Init(*this);
}

mrShaderListShader::mrShaderListShader_ClassDesc::~mrShaderListShader_ClassDesc() {

}

int mrShaderListShader::mrShaderListShader_ClassDesc::IsPublic() {

	return m_isPublic;
}

void* mrShaderListShader::mrShaderListShader_ClassDesc::Create(BOOL loading) {

	Texmap* texmap = new mrShaderListShader(*this, (loading != 0));
	return texmap;
}

const TCHAR* mrShaderListShader::mrShaderListShader_ClassDesc:: ClassName() {

	// The class name is preceded by the prefix
	static TSTR name;
	name = GetString(IDS_SHADERLIST_CLASSNAME);
	name += GetString(m_typeStringResID);
	return name;
}

SClass_ID mrShaderListShader::mrShaderListShader_ClassDesc::SuperClassID() {

	return TEXMAP_CLASS_ID;
}

Class_ID mrShaderListShader::mrShaderListShader_ClassDesc::ClassID() {

	return m_classID;
}

const TCHAR* mrShaderListShader::mrShaderListShader_ClassDesc::Category() {

	return GetString(IDS_CATEGORY);
}

const TCHAR* mrShaderListShader::mrShaderListShader_ClassDesc::InternalName() {

	return m_internalName.data();
}

HINSTANCE mrShaderListShader::mrShaderListShader_ClassDesc::HInstance() {

	return hInstance;
}

bool mrShaderListShader::mrShaderListShader_ClassDesc::IsCompatibleWithRenderer(ClassDesc& rendererClassDesc) {

	// Only compatible with mental ray
	return ((rendererClassDesc.ClassID() == MRRENDERER_CLASSID) != 0);
}

bool mrShaderListShader::mrShaderListShader_ClassDesc::GetCustomMtlBrowserIcon(HIMAGELIST& hImageList, int& inactiveIndex, int& activeIndex, int& disabledIndex) {

	if(PrivateResourceManager::GetBrowserIcons() == NULL) {
		// Load the icons image list
		PrivateResourceManager::GetBrowserIcons() = ImageList_Create(12, 12, ILC_MASK, 4, 0);
		HBITMAP hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_MATICONS));
		HBITMAP hMask = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_MATICONS_MASK));
		int res = ImageList_Add(PrivateResourceManager::GetBrowserIcons(), hBitmap, hMask);
	}

	hImageList = PrivateResourceManager::GetBrowserIcons();

	inactiveIndex = 1;
	activeIndex = 2;
	disabledIndex = 3;

	return true;
}

unsigned int mrShaderListShader::mrShaderListShader_ClassDesc::GetApplyTypes() {

	return m_applyTypes;
}


//==============================================================================
// class mrShaderListShader::mrShaderListShader_UserDlgProc
//==============================================================================

mrShaderListShader::mrShaderListShader_UserDlgProc::mrShaderListShader_UserDlgProc(unsigned int applyTypes)
: m_applyTypes(applyTypes),
  m_theTexmap(NULL)
{
	// nothing
}

mrShaderListShader::mrShaderListShader_UserDlgProc::~mrShaderListShader_UserDlgProc() {

	// nothing
}

INT_PTR mrShaderListShader::mrShaderListShader_UserDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch(msg) {
	case WM_INITDIALOG:
		{
			ReferenceMaker* owner = map->GetParamBlock()->GetOwner();
			if(owner->IsRefTarget()) {
				SetThing(static_cast<ReferenceTarget*>(owner));
			}
			else {
				DbgAssert(false);
			}
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_BUTTON_ADD:
			if(HIWORD(wParam) == BN_CLICKED) {
				if(m_theTexmap != NULL) {
					// Set the selection to -1 to cause the call to the texture button to add the shader
					int oldSelectedItem = m_theTexmap->GetSelectedItem();
					m_theTexmap->SetSelectedItem(-1);
					int oldCount = m_theTexmap->GetNumShaders();

					// Generate a command on the texmap button to cause a new shader to be created
					HWND buttonHWnd = GetDlgItem(hWnd, IDC_BUTTON_SHADER);
					SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_SHADER, BN_CLICKED), reinterpret_cast<LPARAM>(buttonHWnd));
				}
				else {
					DbgAssert(false);
					return FALSE;
				}
			}
			else {
				return FALSE;
			}
			break;

		case IDC_BUTTON_REMOVE:
			if(HIWORD(wParam) == BN_CLICKED) {
				if(m_theTexmap != NULL) {
					// Remove the selected shader
					int selectedItem = m_theTexmap->GetSelectedItem();
					int count = m_theTexmap->GetNumShaders();
					if((selectedItem >= 0) && (selectedItem < count)) {
						m_theTexmap->RemoveShader(selectedItem);
						m_theTexmap->SetSelectedItem(selectedItem);
						Update(t, FOREVER, map);
					}
				}
				else {
					DbgAssert(false);
					return FALSE;
				}
			}
			else {
				return FALSE;
			}
			break;

		case IDC_LIST_SHADERS:
			if(HIWORD(wParam) == LBN_SELCHANGE) {
				if(m_theTexmap != NULL) {
					// Update the button when the selection changes
					int selectedItem = SendMessage(reinterpret_cast<HWND>(lParam), LB_GETCURSEL, 0, 0);
					m_theTexmap->SetSelectedItem(selectedItem);
					Update(t, FOREVER, map);
					// Give the focus back to the listbox
					SendMessage(reinterpret_cast<HWND>(lParam), LB_SETCURSEL, selectedItem, 0);
				}
				else {
					DbgAssert(false);
					return FALSE;
				}
			}
			else if(HIWORD(wParam) == LBN_DBLCLK) {
				// List was double-clicked: edit clicked shader
				// Generate a command on the shader button to cause it to be edited
				HWND buttonHWnd = GetDlgItem(hWnd, IDC_BUTTON_SHADER);
				SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_SHADER, BN_CLICKED), reinterpret_cast<LPARAM>(buttonHWnd));
			}
			else {
				return FALSE;
			}
			break;

		case IDC_SHADER_ON:
			{
				if(m_theTexmap != NULL) {
					int selectedItem = m_theTexmap->GetSelectedItem();
					if(selectedItem >= 0) {
						bool isOn = (IsDlgButtonChecked(hWnd, IDC_SHADER_ON) == BST_CHECKED);
						m_theTexmap->SetShaderOn(selectedItem, isOn);
					}
				}
				else {
					DbgAssert(false);
				}
			}
			return FALSE;

		case IDC_BUTTON_UP:
			if(HIWORD(wParam) == BN_CLICKED) {
				if(m_theTexmap != NULL) {
					int selectedItem = m_theTexmap->GetSelectedItem();
					if(selectedItem >= 1) {
						m_theTexmap->SwapShaders(selectedItem, selectedItem - 1);
						--selectedItem;
						m_theTexmap->SetSelectedItem(selectedItem);
						Update(t, FOREVER, map);
					}
				}
				else {
					DbgAssert(false);
				}
			}
			break;

		case IDC_BUTTON_DOWN:
			if(HIWORD(wParam) == BN_CLICKED) {
				if(m_theTexmap != NULL) {
					int count = m_theTexmap->GetNumShaders();
					int selectedItem = m_theTexmap->GetSelectedItem();
					if(selectedItem < (count - 1)) {
						m_theTexmap->SwapShaders(selectedItem, selectedItem + 1);
						++selectedItem;
						m_theTexmap->SetSelectedItem(selectedItem);
						Update(t, FOREVER, map);
					}
				}
				else {
					DbgAssert(false);
				}
			}
			break;

		default:
			return FALSE;
		}
		break;

	case WM_CLOSE:
		m_theTexmap = NULL;
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

void mrShaderListShader::mrShaderListShader_UserDlgProc::DeleteThis() {

	// don't delete - not dynamically allocated
}

void mrShaderListShader::mrShaderListShader_UserDlgProc::Update(TimeValue t, Interval& valid, IParamMap2* pmap) {
	
	HWND hWnd = pmap->GetHWnd();
	DbgAssert(hWnd != NULL);

	if(m_theTexmap != NULL) {

		// Update the list box - add all the shaders in the list
		HWND listHWnd = GetDlgItem(hWnd, IDC_LIST_SHADERS);
		int topindex = SendMessage(listHWnd, LB_GETTOPINDEX, 0, 0);

		SendMessage(listHWnd, LB_RESETCONTENT, 0, 0);
		
		int count = m_theTexmap->GetNumShaders();
		for(int i = 0; i < count; ++i) {
			Texmap* shader = m_theTexmap->GetShader(i);
			if(shader != NULL) {
				TSTR fullName = shader->GetFullName();
				SendMessage(listHWnd, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(fullName.data()));
			}
			else {
				// MUST insert a string, otherwise the list indices won't be correct
				SendMessage(listHWnd, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(GetString(IDS_NONE)));
			}
		}

		int selectedItem = m_theTexmap->GetSelectedItem();

		// Update the ON state of the shader
		if((selectedItem >= 0) && (selectedItem < count)) {
			bool isShaderOn = m_theTexmap->GetShaderOn(selectedItem);
			CheckDlgButton(hWnd, IDC_SHADER_ON, (isShaderOn ? BST_CHECKED : BST_UNCHECKED));
		}

		// Enable/disable the shader button/checkbox
		{
			bool enable = (selectedItem >= 0) && (selectedItem < count);
			EnableWindow(GetDlgItem(hWnd, IDC_SHADER_ON), enable);
			ICustButton* shaderButton = GetICustButton(GetDlgItem(hWnd, IDC_BUTTON_SHADER));
			if(shaderButton != NULL) {
				shaderButton->Enable(enable);
				ReleaseICustButton(shaderButton);
			}
		}		

		// Give the focus back to the listbox
		SendMessage(GetDlgItem(hWnd, IDC_LIST_SHADERS), LB_SETCURSEL, selectedItem, 0);
		if(topindex>=0)
			SendMessage(listHWnd, LB_SETTOPINDEX, topindex, 0);
	}

	// Enable/Disable items
	int selectedItem = SendMessage(GetDlgItem(hWnd, IDC_LIST_SHADERS), LB_GETCURSEL, 0, 0);

	if(selectedItem <= 0)
		EnableWindow ( GetDlgItem( hWnd, IDC_BUTTON_UP ), FALSE );
	else
		EnableWindow ( GetDlgItem( hWnd, IDC_BUTTON_UP ), TRUE );

	if(selectedItem==m_theTexmap->GetNumShaders()-1)
		EnableWindow ( GetDlgItem( hWnd, IDC_BUTTON_DOWN ), FALSE );
	else
		EnableWindow ( GetDlgItem( hWnd, IDC_BUTTON_DOWN ), TRUE );

	if(selectedItem<0)
		EnableWindow ( GetDlgItem( hWnd, IDC_BUTTON_REMOVE ), FALSE );
	else
		EnableWindow ( GetDlgItem( hWnd, IDC_BUTTON_REMOVE ), TRUE );
}

void mrShaderListShader::mrShaderListShader_UserDlgProc::SetThing(ReferenceTarget *m)
{
	ReferenceMaker* owner = m;
	if((owner != NULL) && (owner->SuperClassID() == TEXMAP_CLASS_ID))
		m_theTexmap = static_cast<mrShaderListShader*>(owner);
	else
		m_theTexmap = NULL;

	DbgAssert(m_theTexmap != NULL);
	if(m_theTexmap != NULL) {
		// Scan the list of shaders and remove any NULL shaders. NULL shaders
		// can be added through Maxscript, and possibly through other ways also.
		// This is to solve bug #525506.
		m_theTexmap->RemoveNullShaders();
	}
}


