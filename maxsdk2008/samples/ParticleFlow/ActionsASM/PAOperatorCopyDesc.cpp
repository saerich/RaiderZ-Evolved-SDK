/**********************************************************************
 *<
	FILE:			PAOperatorCopyDesc.cpp

	DESCRIPTION:	SimpleSpin Operator Class Descriptor (definition)
											 
	CREATED FOR:	Oleg Bayborodin

	HISTORY: created 05-17-03

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#include "StdAfx.h"
#include "resource.h"

#include "PAOperatorCopyDesc.h"

#include "PartASM_GlobalVariables.h"
#include "PartASM_GlobalFunctions.h"
#include "PartASM_SysUtil.h"
#include "PAOperatorCopy.h"
#include "PFClassIDs.h"

namespace PartASM {

HBITMAP PAOperatorCopyDesc::m_depotIcon = NULL;
HBITMAP PAOperatorCopyDesc::m_depotMask = NULL;

PAOperatorCopyDesc::~PAOperatorCopyDesc()
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

int PAOperatorCopyDesc::IsPublic()
{
	return 0;
}

void* PAOperatorCopyDesc::Create(BOOL loading) 
{
	return new PAOperatorCopy();
}

const TCHAR* PAOperatorCopyDesc::ClassName() 
{
	return GetString(IDS_OPERATOR_COPY_CLASS_NAME);
}

SClass_ID PAOperatorCopyDesc::SuperClassID() 
{
	return HELPER_CLASS_ID;
}

Class_ID PAOperatorCopyDesc::ClassID()
{
	return PAOperatorCopy_Class_ID;
}

Class_ID PAOperatorCopyDesc::SubClassID()
{
	return PFOperatorSubClassID;
}

const TCHAR* PAOperatorCopyDesc::Category() 
{
	return GetString(IDS_PARTICLEASSEMBLER_CATEGORY);
}

const TCHAR* PAOperatorCopyDesc::InternalName()
{
	return _T("ASMCopy");
}

HINSTANCE PAOperatorCopyDesc::HInstance()
{
	return hInstance;
}

INT_PTR PAOperatorCopyDesc::Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3)
{
	TCHAR** res = NULL;
	HBITMAP* depotIcon;
	HBITMAP* depotMask;
	switch( cmd )
	{
	case kPF_GetActionDescription:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_OPERATOR_COPY_DESCRIPTION);
		break;
	case kPF_PViewCategory:
		if (arg1 == NULL) return 0;
		res = (TCHAR**)arg1;
		*res = GetString(IDS_PF_OPERATOR_CATEGORY);
		break;
	case kPF_PViewDepotIcon:
		if (arg1 == NULL) return 0;
		depotIcon = (HBITMAP*)arg1;
		if (arg2 == NULL) return 0;
		depotMask = (HBITMAP*)arg2;
		if (m_depotIcon == NULL)
			m_depotIcon = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_OPERATOR_DEPOTICON));
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