// ============================================================================
// VisualMSUtil.cpp
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#include "stdafx.h"
#include "VisualMS.h"
#include "VisualMSThread.h"

// from the max sdk
#include "Max.h"
#include "iparamm2.h"
#include "utilapi.h"

#define VISUALMS_CLASS_ID  Class_ID(0x376af827, 0xb5e60d3)


// ============================================================================
class CVisualMSUtil : public UtilityObj
{
public:
	Interface   *ip;
	IUtil       *iu;

	void BeginEditParams(Interface *ip, IUtil *iu);
	void EndEditParams(Interface *ip, IUtil *iu);
	void DeleteThis() {}

	CVisualMSUtil();
	~CVisualMSUtil();
};
static CVisualMSUtil g_theVisualMSUtil;


// ============================================================================
class CVisualMSClassDesc : public ClassDesc2
{
public:
	int            IsPublic() { return 1; }
	void*          Create(BOOL loading = FALSE) { return &g_theVisualMSUtil; }
	const TCHAR*   ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID      SuperClassID() { return UTILITY_CLASS_ID; }
	Class_ID       ClassID() { return VISUALMS_CLASS_ID; }
	const TCHAR*   Category() { return GetString(IDS_CATEGORY); }
	const TCHAR*   InternalName() { return _T("VisualMSUtil"); }
	HINSTANCE      HInstance() { return g_hInst; }
};
static CVisualMSClassDesc g_visualMSDesc;
ClassDesc2* GetVisualMSDesc() { return &g_visualMSDesc; }


// ============================================================================
CVisualMSUtil::CVisualMSUtil()
{
	iu = NULL;
	ip = NULL;
}

// ============================================================================
CVisualMSUtil::~CVisualMSUtil() {}

// ============================================================================
void CVisualMSUtil::BeginEditParams(Interface *ip, IUtil *iu) 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	this->iu = iu;
	this->ip = ip;

	CVisualMSThread::Create();
	iu->CloseUtility();
}

// ============================================================================
void CVisualMSUtil::EndEditParams(Interface *ip, IUtil *iu) {}

