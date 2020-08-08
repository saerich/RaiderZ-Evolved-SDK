//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// AUTHOR: Nicolas Desjardins
// DATE: 2007-07-24
//***************************************************************************/

#pragma unmanaged
#include "SceneExplorerExtensionGup.h"

// Switching to managed to include the managed HitPointsProperty ref class.
#pragma managed
#include "HitPointsProperty.h"

// Managed types cannot be used in unmanaged code, so call into managed code
// from here.
namespace // anonymous namespace
{

void RegisterHitPointsProperty()
{
	SceneExplorerExtension::HitPointsProperty::RegisterProperty();
}

}

// Switching back to unmanaged code. the GUP class is an unmanaged, native type.
#pragma unmanaged

namespace SceneExplorerExtension
{

SceneExplorerExtensionGup* SceneExplorerExtensionGup::sInstance = nullptr;

SceneExplorerExtensionGup::SceneExplorerExtensionGup()
{ }

SceneExplorerExtensionGup::~SceneExplorerExtensionGup()
{ }

SceneExplorerExtensionGup* SceneExplorerExtensionGup::GetInstance()
{
	if(nullptr == sInstance)
	{
		sInstance = new SceneExplorerExtensionGup();
	}

	return sInstance;
}

void SceneExplorerExtensionGup::Destroy()
{
	delete sInstance;
	sInstance = nullptr;
}

DWORD SceneExplorerExtensionGup::Start()
{
	RegisterHitPointsProperty();
	return GUPRESULT_KEEP;
}

void SceneExplorerExtensionGup::Stop()
{ }

ClassDesc* SceneExplorerExtensionGup::GetClassDesc()
{
	return &mClassDesc;
}


}
