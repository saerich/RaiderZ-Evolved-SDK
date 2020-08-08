/**********************************************************************
 *<
	FILE:			PATestDesc.cpp

	DESCRIPTION:	Duration Test Class Descriptor (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 12-03-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/


#include "StdAfx.h"
#include "resource.h"

#include "PATestDesc.h"

#include "PartASM_GlobalVariables.h"
#include "PartASM_GlobalFunctions.h"
#include "PartASM_SysUtil.h"
#include "PATest.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PartASM {

HBITMAP PATestDesc::m_depotIcon = NULL;
HBITMAP PATestDesc::m_depotMask = NULL;

PATestDesc::~PATestDesc()
{
	if (m_depotIcon != NULL) {
		DeleteObject(m_depotIcon);
		m_depotIcon = NULL;
	}
	if (m_depotMask != NULL) {
		DeleteObject(m_depotMask);
		m_depotMask = NULL;
	}
}

int PATestDesc::IsPublic()
{
	return 0;
}

void* PATestDesc::Create(BOOL loading) 
{
	return new PATest();
}

const TCHAR* PATestDesc::ClassName() 
{
	return GetString(IDS_TEST_CLASS_NAME);
}

SClass_ID PATestDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PATestDesc::ClassID()
{
	return PATest_Class_ID;
}

Class_ID PATestDesc::SubClassID()
{
	return PFTestSubClassID;
}

const TCHAR* PATestDesc::Category() 
{
	return GetString(IDS_PARTICLEASSEMBLER_CATEGORY);
}

const TCHAR* PATestDesc::InternalName()
{
	return _T("ASM_Test");
}

HINSTANCE PATestDesc::HInstance()
{
	return hInstance;
}

INT_PTR PATestDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	bool* isPublic;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_TEST_DESCRIPTION);
		break;
	case kPF_PViewPublic:
		if (arg1 == NULL) return 0;
		isPublic = (bool*)arg1;
		*isPublic = true;
		break;
	case kPF_PViewCategory:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_PF_TEST_CATEGORY);
		break;
	case kPF_PViewDepotIcon:
		if (arg1 == NULL) return 0;
		depotIcon = (HBITMAP*)arg1;
		if (arg2 == NULL) return 0;
		depotMask = (HBITMAP*)arg2;
		if (m_depotIcon == NULL)
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_TEST_DEPOTICON));
		if (m_depotMask == NULL)
			m_depotMask = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_DEPOTICONMASK_TEST));
		*depotIcon = m_depotIcon;
		*depotMask = m_depotMask;
		break;
	default:
		return 0;
	}
	return 1;
}


} // end of namespace PartASM