/*==============================================================================
Copyright 2010 Autodesk, Inc.  All rights reserved.

Use of this software is subject to the terms of the Autodesk license agreement provided
at the time of installation or download, or which otherwise accompanies this software
in either electronic or hard copy form.   

//**************************************************************************/
// DESCRIPTION: Class to expose extended mxs functions for epoly/editpoly
// AUTHOR: Michael Zyracki created September 2009
//***************************************************************************/

#pragma once

#include "iEPoly.h"
#include "iEPolyMod.h"
#include "maxapi.h"
#include "Object.h"
#include "IPathConfigMgr.h"
#include "EPolyManipulatorGrip.h"


class EPolyManipulatorGrip_Imp : public EPolyManipulatorGrip
{
public:
        
    static EPolyManipulatorGrip& GetInstance() { return _theInstance; }
    
	enum {
		kSetManipulateGrip =0,
		kGetManipulateGrip
	};


	void SetManipulateGrip(bool on, EPolyManipulatorGrip::ManipulateGrips item);
	bool GetManipulateGrip(EPolyManipulatorGrip::ManipulateGrips item) ;

	void fpSetManipulateGrip(bool on, int item);
	bool fpGetManipulateGrip(int item) ;


	BEGIN_FUNCTION_MAP		
		VFN_2(kSetManipulateGrip, fpSetManipulateGrip, TYPE_bool, TYPE_ENUM);
		FN_1(kGetManipulateGrip,TYPE_bool, fpGetManipulateGrip,TYPE_ENUM);
	END_FUNCTION_MAP

	~EPolyManipulatorGrip_Imp();
	void Init();
private:

	void SavePreferences ();
	void LoadPreferences ();


	static MSTR GetINIFileName();
	static const TCHAR* kINI_FILE_NAME;
	static const TCHAR* kINI_SECTION_NAME;
	static const TCHAR* kFALLOFF;
	static const TCHAR* kPINCH;
	static const TCHAR* kBUBBLE;
	static const TCHAR* kSETFLOW;
	static const TCHAR* kLOOPSHIFT;
	static const TCHAR* kRINGSHIFT;
	static const TCHAR* kEDGECREASE;
	static const TCHAR* kEDGEWEIGHT;
	static const TCHAR* kVERTEXWEIGHT;

	// Declares the constructor of this class. It is private so a single static instance of this class
    // may be instantiated.
    DECLARE_DESCRIPTOR( EPolyManipulatorGrip_Imp );
    
    // The single instance of this class
    static EPolyManipulatorGrip_Imp  _theInstance;

	BitArray mManipMask;
};


const TCHAR* EPolyManipulatorGrip_Imp::kINI_FILE_NAME = _T("EditPolyManipulators.ini");
const TCHAR* EPolyManipulatorGrip_Imp::kINI_SECTION_NAME= _T("Toggles");
const TCHAR* EPolyManipulatorGrip_Imp::kFALLOFF = _T("Falloff");
const TCHAR* EPolyManipulatorGrip_Imp::kPINCH = _T("Pinch");
const TCHAR* EPolyManipulatorGrip_Imp::kBUBBLE = _T("Bubble");
const TCHAR* EPolyManipulatorGrip_Imp::kSETFLOW = _T("SetFlow");
const TCHAR* EPolyManipulatorGrip_Imp::kLOOPSHIFT = _T("LoopShift");
const TCHAR* EPolyManipulatorGrip_Imp::kRINGSHIFT = _T("RingShift");
const TCHAR* EPolyManipulatorGrip_Imp::kEDGECREASE = _T("EdgeCrease");
const TCHAR* EPolyManipulatorGrip_Imp::kEDGEWEIGHT = _T("EdgeWeigt");
const TCHAR* EPolyManipulatorGrip_Imp::kVERTEXWEIGHT = _T("VertexWeight");


enum enumList{eManipulateGrips = 0};

EPolyManipulatorGrip_Imp EPolyManipulatorGrip_Imp::_theInstance(
  IEPOLYMANIPGRIP_INTERFACE, _T("EPolyManipGrip"), 0, NULL, FP_CORE,
   
	kSetManipulateGrip, _T("SetManipulateGrip"), 0, TYPE_VOID, 0, 2,
		_T("on"), 0, TYPE_bool,
		_T("gripItem"), 0, TYPE_ENUM, eManipulateGrips,
	kGetManipulateGrip, _T("GetManipulateGrip"), 0, TYPE_bool, 0, 1,
		_T("gripItem"), 0, TYPE_ENUM, eManipulateGrips,

	enums,
		eManipulateGrips, 9,

		_T("SSFalloff"), EPolyManipulatorGrip::eSoftSelFalloff,
		_T("SSBubble"), EPolyManipulatorGrip::eSoftSelBubble,
		_T("SSPinch"), EPolyManipulatorGrip::eSoftSelPinch,
		_T("SetFlow"), EPolyManipulatorGrip::eSetFlow,
		_T("LoopShift"), EPolyManipulatorGrip::eLoopShift,
		_T("RingShift"), EPolyManipulatorGrip::eRingShift,
		_T("EdgeCrease"), EPolyManipulatorGrip::eEdgeCrease,
		_T("EdgeWeight"), EPolyManipulatorGrip::eEdgeWeight,
		_T("VertexWeight"), EPolyManipulatorGrip::eVertexWeight,
	end 
);


BOOL GetModel(MNMesh *&mm, EPoly *&cd, EPolyMod13 *&ep, INode *&node, int &editpoly)
{
	Interface* ip = GetCOREInterface();
	BaseObject* base = ip->GetCurEditObject();
	if (!base || ip->GetSelNodeCount() != 1) return TRUE;
	if (base->ClassID() == EPOLYOBJ_CLASS_ID) editpoly = 0;
	else if (base->ClassID() == EDIT_POLY_MODIFIER_CLASS_ID) editpoly = 1;
	else return TRUE;
	node = ip->GetSelNode(0);
	if (editpoly)
	{
		ep = (EPolyMod13 *) base->GetInterface(EPOLY_MOD13_INTERFACE);
		mm = ep->EpModGetMesh();
	}
	else
	{
		cd = (EPoly *)(base->GetInterface(EPOLY_INTERFACE));
		mm = cd->GetMeshPtr();
	}
	return FALSE;
}



void  EPolyManipulatorGrip_Imp::Init()
{
	mManipMask.SetSize(EPolyManipulatorGrip::eVertexWeight+1 );
	mManipMask.SetAll();
	LoadPreferences();
}

EPolyManipulatorGrip_Imp::~EPolyManipulatorGrip_Imp()
{
	int toggles[EPolyManipulatorGrip::eVertexWeight+1 ];
	for(int i=0;i<mManipMask.GetSize();++i)
	{
		toggles[i] = mManipMask[i];
	}

	SavePreferences();
}


void EPolyManipulatorGrip_Imp::SetManipulateGrip(bool on, EPolyManipulatorGrip::ManipulateGrips item)
{
	
	int i = (int) item;
	if(i>=0 && i< mManipMask.GetSize())
	{
		if(on)
			mManipMask.Set(i);
		else
			mManipMask.Clear(i);
	}

	Interface* ip = GetCOREInterface();
	BaseObject* base = ip->GetCurEditObject();
	if (!base || ip->GetSelNodeCount() != 1) return;
	if (base->ClassID() == EPOLYOBJ_CLASS_ID)
	{
		EPoly13 *eP = dynamic_cast<EPoly13 *>(base->GetInterface(EPOLY_INTERFACE));
		if(eP)
		{
			EPoly13::ManipulateGrips cItem = (EPoly13::ManipulateGrips)(item);
			eP->SetManipulateGrip(on,cItem);
		}
	}
	else if (base->ClassID() == EDIT_POLY_MODIFIER_CLASS_ID)
	{
		if(item < (int) EPolyManipulatorGrip::eEdgeCrease)
		{
			EPolyMod13* eP = dynamic_cast<EPolyMod13 *>(base->GetInterface(EPOLY_MOD13_INTERFACE));
			if(eP)
			{
				EPolyMod13::ManipulateGrips cItem = (EPolyMod13::ManipulateGrips)(item);
				eP->SetManipulateGrip(on,cItem);
			}
		}
	}
}

bool EPolyManipulatorGrip_Imp::GetManipulateGrip(EPolyManipulatorGrip::ManipulateGrips val)
{
	int i = (int) val;
	if(i>=0 && i< mManipMask.GetSize())
		return mManipMask[i] == 0 ? false : true;
	return false;

}


void EPolyManipulatorGrip_Imp::fpSetManipulateGrip(bool on, int item)
{

	EPolyManipulatorGrip::ManipulateGrips cItem = static_cast<EPolyManipulatorGrip::ManipulateGrips>(item);
	SetManipulateGrip(on,cItem);
}


bool EPolyManipulatorGrip_Imp::fpGetManipulateGrip(int item) 
{
	EPolyManipulatorGrip::ManipulateGrips cItem = static_cast<EPolyManipulatorGrip::ManipulateGrips>(item);
	return GetManipulateGrip(cItem);
}


MSTR EPolyManipulatorGrip_Imp::GetINIFileName()
{
	IPathConfigMgr* pPathMgr = IPathConfigMgr::GetPathConfigMgr();
	MSTR iniFileName(pPathMgr->GetDir(APP_PLUGCFG_DIR));
	iniFileName += _T("\\");
	iniFileName += kINI_FILE_NAME;
	return iniFileName;
}



void EPolyManipulatorGrip_Imp::SavePreferences ()
{
	MSTR iniFileName = GetINIFileName();
	TCHAR buf[MAX_PATH] = {0};
	// The ini file should always be saved and read with the same locale settings
	MaxLocaleHandler localeGuard;
	_stprintf_s(buf, sizeof(buf), "%d", mManipMask[EPolyManipulatorGrip::eSoftSelFalloff]);
	BOOL res = WritePrivateProfileString(kINI_SECTION_NAME, kFALLOFF, buf, iniFileName);
	DbgAssert(res);

	_stprintf_s(buf, sizeof(buf), "%d", mManipMask[EPolyManipulatorGrip::eSoftSelPinch]);
	res = WritePrivateProfileString(kINI_SECTION_NAME, kPINCH, buf, iniFileName);
	DbgAssert(res);

	_stprintf_s(buf, sizeof(buf), "%d", mManipMask[EPolyManipulatorGrip::eSoftSelBubble]);
	res = WritePrivateProfileString(kINI_SECTION_NAME, kBUBBLE, buf, iniFileName);
	DbgAssert(res);

	_stprintf_s(buf, sizeof(buf), "%d", mManipMask[EPolyManipulatorGrip::eSetFlow]);
	res = WritePrivateProfileString(kINI_SECTION_NAME, kSETFLOW, buf, iniFileName);
	DbgAssert(res);

	_stprintf_s(buf, sizeof(buf), "%d", mManipMask[EPolyManipulatorGrip::eLoopShift]);
	res = WritePrivateProfileString(kINI_SECTION_NAME, kLOOPSHIFT, buf, iniFileName);
	DbgAssert(res);

	_stprintf_s(buf, sizeof(buf), "%d", mManipMask[EPolyManipulatorGrip::eRingShift]);
	res = WritePrivateProfileString(kINI_SECTION_NAME, kRINGSHIFT, buf, iniFileName);
	DbgAssert(res);

	_stprintf_s(buf, sizeof(buf), "%d", mManipMask[EPolyManipulatorGrip::eEdgeCrease]);
	res = WritePrivateProfileString(kINI_SECTION_NAME, kEDGECREASE, buf, iniFileName);
	DbgAssert(res);

	_stprintf_s(buf, sizeof(buf), "%d", mManipMask[EPolyManipulatorGrip::eEdgeWeight]);
	res = WritePrivateProfileString(kINI_SECTION_NAME, kEDGEWEIGHT, buf, iniFileName);
	DbgAssert(res);

	_stprintf_s(buf, sizeof(buf), "%d", mManipMask[EPolyManipulatorGrip::eVertexWeight]);
	res = WritePrivateProfileString(kINI_SECTION_NAME, kVERTEXWEIGHT, buf, iniFileName);
	DbgAssert(res);
}

void EPolyManipulatorGrip_Imp::LoadPreferences ()
{
	MSTR iniFileName = GetINIFileName();
	TCHAR buf[MAX_PATH] = {0};
	TCHAR defaultVal[MAX_PATH] = {0};
	IPathConfigMgr* pathMgr = IPathConfigMgr::GetPathConfigMgr();
	if (pathMgr->DoesFileExist(iniFileName)) 
	{

		// The ini file should always be saved and read with the same locale settings
		MaxLocaleHandler localeGuard;
		_stprintf_s(defaultVal, sizeof(defaultVal), "%d", mManipMask[EPolyManipulatorGrip::eSoftSelFalloff]);
		if (GetPrivateProfileString(kINI_SECTION_NAME, kFALLOFF, defaultVal, buf, sizeof(buf), iniFileName))
		{
			int val = atoi(buf);
			mManipMask.Set((int) mManipMask[EPolyManipulatorGrip::eSoftSelFalloff],val);
		}
		_stprintf_s(defaultVal, sizeof(defaultVal), "%d", mManipMask[EPolyManipulatorGrip::eSoftSelPinch]);
		if (GetPrivateProfileString(kINI_SECTION_NAME,kPINCH, defaultVal, buf, sizeof(buf), iniFileName))
		{
			int val = atoi(buf);
			mManipMask.Set((int)mManipMask[EPolyManipulatorGrip::eSoftSelPinch],val);
		}
		_stprintf_s(defaultVal, sizeof(defaultVal), "%d", mManipMask[EPolyManipulatorGrip::eSoftSelBubble]);
		if (GetPrivateProfileString(kINI_SECTION_NAME, kBUBBLE, defaultVal, buf, sizeof(buf), iniFileName))
		{
			int val = atoi(buf);
			mManipMask.Set((int)mManipMask[EPolyManipulatorGrip::eSoftSelBubble],val);
		}
		_stprintf_s(defaultVal, sizeof(defaultVal), "%d", mManipMask[EPolyManipulatorGrip::eSetFlow]);
		if (GetPrivateProfileString(kINI_SECTION_NAME, kSETFLOW, defaultVal, buf, sizeof(buf), iniFileName))
		{
			int val = atoi(buf);
			mManipMask.Set((int)mManipMask[EPolyManipulatorGrip::eSetFlow],val);
		}
		_stprintf_s(defaultVal, sizeof(defaultVal), "%d", mManipMask[EPolyManipulatorGrip::eLoopShift]);
		if (GetPrivateProfileString(kINI_SECTION_NAME,kLOOPSHIFT, defaultVal, buf, sizeof(buf), iniFileName))
		{
			int val = atoi(buf);
			mManipMask.Set((int)mManipMask[EPolyManipulatorGrip::eLoopShift],val);
		}
		_stprintf_s(defaultVal, sizeof(defaultVal), "%d", mManipMask[EPolyManipulatorGrip::eRingShift]);
		if (GetPrivateProfileString(kINI_SECTION_NAME, kRINGSHIFT, defaultVal, buf, sizeof(buf), iniFileName))
		{
			int val = atoi(buf);
			mManipMask.Set((int)EPolyManipulatorGrip::eRingShift,val);
		}
		_stprintf_s(defaultVal, sizeof(defaultVal), "%d", mManipMask[EPolyManipulatorGrip::eEdgeCrease]);
		if (GetPrivateProfileString(kINI_SECTION_NAME, kEDGECREASE, defaultVal, buf, sizeof(buf), iniFileName))
		{
			int val = atoi(buf);
			mManipMask.Set((int)EPolyManipulatorGrip::eEdgeCrease,val);
		}
		_stprintf_s(defaultVal, sizeof(defaultVal), "%d", mManipMask[EPolyManipulatorGrip::eEdgeWeight]);
		if (GetPrivateProfileString(kINI_SECTION_NAME,kEDGEWEIGHT, defaultVal, buf, sizeof(buf), iniFileName))
		{
			int val = atoi(buf);
			mManipMask.Set((int)EPolyManipulatorGrip::eEdgeWeight,val);
		}
		_stprintf_s(defaultVal, sizeof(defaultVal), "%d", mManipMask[EPolyManipulatorGrip::eVertexWeight]);
		if (GetPrivateProfileString(kINI_SECTION_NAME, kVERTEXWEIGHT, defaultVal, buf, sizeof(buf), iniFileName))
		{
			int val = atoi(buf);
			mManipMask.Set((int)EPolyManipulatorGrip::eVertexWeight,val);
		}


	}
}

