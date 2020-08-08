/**********************************************************************
 *<
	FILE:			PAOperatorDisplayDesc.cpp

	DESCRIPTION:	Display Operator Class Descriptor (definition)
											 
	CREATED FOR:		Oleg Bayborodin

	HISTORY:		created 05-17-03

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#include "StdAfx.h"
#include "resource.h"

#include "PAOperatorDisplayDesc.h"

#include "PartASM_GlobalVariables.h"
#include "PartASM_GlobalFunctions.h"
#include "PartASM_SysUtil.h"
#include "PAOperatorDisplay.h"
#include "PFClassIDs.h"
#include "IPFAction.h"

namespace PartASM {

HBITMAP PAOperatorDisplayDesc::m_depotIcon = NULL;
HBITMAP PAOperatorDisplayDesc::m_depotMask = NULL;

PAOperatorDisplayDesc::~PAOperatorDisplayDesc()
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

int PAOperatorDisplayDesc::IsPublic()
{
	return 0;
}

void* PAOperatorDisplayDesc::Create(BOOL loading) 
{
	return new PAOperatorDisplay();
}

const TCHAR* PAOperatorDisplayDesc::ClassName() 
{
	return GetString(IDS_OPERATOR_DISPLAY_CLASS_NAME);
}

SClass_ID PAOperatorDisplayDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PAOperatorDisplayDesc::ClassID()
{
	return PAOperatorDisplay_Class_ID;
}

Class_ID PAOperatorDisplayDesc::SubClassID()
{
	return PFOperatorSubClassID;
}

const TCHAR* PAOperatorDisplayDesc::Category() 
{
	return GetString(IDS_PARTICLEASSEMBLER_CATEGORY);
}

const TCHAR* PAOperatorDisplayDesc::InternalName()
{
	return _T("ASMDisplay");
}

HINSTANCE PAOperatorDisplayDesc::HInstance()
{
	return hInstance;
}

INT_PTR PAOperatorDisplayDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	bool* isPublic;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	bool* isNonExecutable;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_DISPLAY_DESCRIPTION);
		break;
	case kPF_GetActionName:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_DISPLAY_PVIEW_NAME);
		break;
	case kPF_PViewPublic:
		if (arg1 == NULL) return 0;
		isPublic = (bool*)arg1;
		*isPublic = true;
		break;
	case kPF_PViewCategory:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_PF_OPERATOR_CATEGORY);
		break;
	case kPF_IsNonExecutable:
		if (arg1 == NULL) return 0;
		isNonExecutable = (bool*)arg1;
		*isNonExecutable = true;
		break;
	case kPF_PViewDepotIcon:
		if (arg1 == NULL) return 0;
		depotIcon = (HBITMAP*)arg1;
		if (arg2 == NULL) return 0;
		depotMask = (HBITMAP*)arg2;
		if (m_depotIcon == NULL)
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_DISPLAY_DEPOTICON));
		if (m_depotMask == NULL)
			m_depotMask = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_DEPOTICONMASK_OPERATOR));
		*depotIcon = m_depotIcon;
		*depotMask = m_depotMask;
		break;
	default:
		return 0;
	}
	return 1;
}



} // end of namespace PartASM