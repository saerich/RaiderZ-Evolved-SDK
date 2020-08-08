// Copyright 2009 Autodesk, Inc.  All rights reserved.
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law.  They may not
// be disclosed to, copied or used by any third party without the prior written
// consent of Autodesk, Inc.

#include "AssemblyEntry.h"
#include "DaylightSystemFactory.h"
#include "DaylightSystemFactory2.h"
#include "natLight.h"
#include "NatLightAssemblyClassDesc.h"
#include "SunMasterCreateMode.h"
#include "CityList.h"

#pragma managed

namespace SunlightSystem
{

void AssemblyEntry::AssemblyMain()
{
	NatLightAssembly::InitializeStaticObjects();
	DaylightSystemFactory::RegisterInstance();
	DaylightSystemFactory2::RegisterInstance();
}

void AssemblyEntry::AssemblyShutdown()
{
	SunMasterCreateMode::Destroy();
	DaylightSystemFactory2::Destroy();
	DaylightSystemFactory::Destroy();
	NatLightAssembly::DestroyStaticObjects();
	NatLightAssemblyClassDesc::Destroy();
	CityList::Destroy();
}

}
