// Copyright 2009 Autodesk, Inc.  All rights reserved.
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law.  They may not
// be disclosed to, copied or used by any third party without the prior written
// consent of Autodesk, Inc.

#include "NatLightAssemblyClassDesc.h"
#include "natlight.h"
#include "sunlight.h"

#pragma managed

namespace 
{

bool checkStandin(Class_ID& id)
{
	return id.PartA() != STANDIN_CLASS_ID || id.PartB() != 0;
}

}

NatLightAssemblyClassDesc* NatLightAssemblyClassDesc::sInstance = nullptr;

bool NatLightAssemblyClassDesc::IsValidSun( ClassDesc& cd) 
{
	bool bValidSun = ((cd.ClassID() == NatLightAssembly::GetStandardSunClass()) != FALSE);
	if (!bValidSun)
	{
		INaturalLightClass* pNC = GetNaturalLightClassInterface(&cd);
		if (pNC != NULL && (pNC->GetDesc()->flags & FP_STATIC_METHODS)) 
		{
			bValidSun = (pNC->IsSun() != FALSE);
		}
	}
	return bValidSun;
}

bool NatLightAssemblyClassDesc::IsValidSky( ClassDesc& cd) 
{
	bool bValidSky = false;
	INaturalLightClass* pNC = GetNaturalLightClassInterface(&cd);
	if (pNC != NULL && (pNC->GetDesc()->flags & FP_STATIC_METHODS)) 
	{
		bValidSky = (pNC->IsSky() != FALSE);
	}
	return bValidSky;
}

NatLightAssemblyClassDesc::NatLightAssemblyClassDesc()
{ 
	ResetClassParams(true); 
}

NatLightAssemblyClassDesc::~NatLightAssemblyClassDesc()
{ }

NatLightAssemblyClassDesc* NatLightAssemblyClassDesc::GetInstance()
{
	if(nullptr == sInstance)
	{
		sInstance = new NatLightAssemblyClassDesc();
	}
	
	return sInstance;
}

void NatLightAssemblyClassDesc::Destroy()
{
	delete sInstance;
	sInstance = nullptr;
}


int NatLightAssemblyClassDesc::IsPublic()
{ 
	return 0; 
}

void* NatLightAssemblyClassDesc::Create(BOOL) 
{ 
	return new NatLightAssembly(); 
}

const TCHAR* NatLightAssemblyClassDesc::ClassName()
{ 
	return MaxSDK::GetResourceString(IDS_NAT_LIGHT_NAME); 
}

SClass_ID NatLightAssemblyClassDesc::SuperClassID()
{ 
	return HELPER_CLASS_ID; 
}

Class_ID NatLightAssemblyClassDesc::ClassID() 
{ 
	return NATLIGHT_HELPER_CLASS_ID; 
}

const TCHAR* NatLightAssemblyClassDesc::InternalName()
{ 
	return _T("DaylightAssemblyHead"); 
}

// The slave controllers don't appear in any of the drop down lists, 
// so they just return a null string.
const TCHAR* NatLightAssemblyClassDesc::Category() 
{ 
	return _T("");  
}

void NatLightAssemblyClassDesc::ResetClassParams(BOOL fileReset) 
{
	// IES Sun class ID - Defaults to directional light
	mSunID = GetMarketDefaults()->GetClassID(HELPER_CLASS_ID,
		NATLIGHT_HELPER_CLASS_ID, _T("sun"), NatLightAssembly::GetStandardSunClass(),
		checkStandin);
	// IES Sky class ID - Defaults to Skylight
	mSkyID = GetMarketDefaults()->GetClassID(HELPER_CLASS_ID,
		NATLIGHT_HELPER_CLASS_ID, _T("sky"),
		Class_ID(0x7bf61478, 0x522e4705), checkStandin);
	mSunValid = true;
	mSkyValid = true;
}

Object* NatLightAssemblyClassDesc::CreateSun(Interface* ip, const Class_ID* clsID)
{
	theHold.Suspend();
	Object* obj = NULL;
	if (NULL == clsID) 
	{
		obj = static_cast<Object*>(
			mSunValid ? ip->CreateInstance(NatLightAssembly::kValidSuperClass, mSunID) : NULL );
	}
	else 
	{
		ClassDirectory& dir = ip->GetDllDir().ClassDir();
		ClassDesc* cd =  dir.FindClass(NatLightAssembly::kValidSuperClass, *clsID);
		if (cd != NULL && IsValidSun(*cd))
		{
			obj = static_cast<Object*>(ip->CreateInstance(NatLightAssembly::kValidSuperClass, *clsID));
		}
	}
	theHold.Resume();
	return obj;
}

Object* NatLightAssemblyClassDesc::CreateSky(Interface* ip, const Class_ID* clsID) 
{
	theHold.Suspend();
	Object* obj = NULL;
	if (NULL == clsID)
	{
		obj = static_cast<Object*>(
			mSkyValid ? ip->CreateInstance(NatLightAssembly::kValidSuperClass, mSkyID) : NULL );
	}
	else 
	{
		ClassDirectory& dir = ip->GetDllDir().ClassDir();
		ClassDesc* cd =  dir.FindClass(NatLightAssembly::kValidSuperClass, *clsID);
		if (cd != NULL && IsValidSky(*cd))
		{
			obj = static_cast<Object*>(ip->CreateInstance(NatLightAssembly::kValidSuperClass, *clsID));
		}
	}
	theHold.Resume();
	return obj;
}

bool NatLightAssemblyClassDesc::IsPublicClass(const Class_ID& id) 
{
	if (id == Class_ID(0, 0))
		return true;

	ClassEntry* pE = GetCOREInterface()->GetDllDir().ClassDir()
		.FindClassEntry(NatLightAssembly::kValidSuperClass, id);
	return pE != NULL && pE->IsPublic() != 0;
}
