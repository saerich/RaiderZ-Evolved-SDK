// Copyright 2009 Autodesk, Inc. All rights reserved.
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law. They may not
// be disclosed to, copied or used by any third party without the prior written
// consent of Autodesk, Inc.

#pragma once
#include "NativeInclude.h"

#pragma managed(push, on)

class NatLightAssemblyClassDesc : public ClassDesc2, MaxSDK::Util::Noncopyable
{
public:
	int IsPublic();
	void * Create(BOOL loading = FALSE);
	const TCHAR * ClassName();
	SClass_ID SuperClassID();
	Class_ID ClassID();

	const TCHAR* InternalName();

	// The slave controllers don't appear in any of the drop down lists, 
	// so they just return a null string.
	const TCHAR* Category();

	void ResetClassParams(BOOL fileReset);

	Object* CreateSun(Interface* ip, const Class_ID* clsID = NULL);
	Object* CreateSky(Interface* ip, const Class_ID* clsID = NULL);

	static bool IsValidSun(ClassDesc& cd);
	static bool IsValidSky(ClassDesc& cd);
	
	static NatLightAssemblyClassDesc* GetInstance();
	static void Destroy();
	
private:
	NatLightAssemblyClassDesc();
	virtual ~NatLightAssemblyClassDesc();
	
	bool IsPublicClass(const Class_ID& id);

	Class_ID mSunID;
	Class_ID mSkyID;
	bool mSunValid;
	bool mSkyValid;
	
	static NatLightAssemblyClassDesc* sInstance;
};

#pragma managed(pop)
