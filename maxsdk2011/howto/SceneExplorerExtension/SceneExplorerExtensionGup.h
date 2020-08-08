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

#ifndef SCENE_EXPLORER_EXTENSION_GUP_H
#define SCENE_EXPLORER_EXTENSION_GUP_H

#pragma unmanaged
#include "nativeInclude.h"
#include "SceneExplorerExtensionClassDesc.h"

namespace SceneExplorerExtension
{

// Entry point Singleton for this General Utility Plug-in.  Note that this is 
// unmanaged, native code.  It is in a #pragma unmanaged block and is not a 
// 'ref' class.
class SceneExplorerExtensionGup : public GUP, MaxSDK::Util::Noncopyable
{
public:
	static SceneExplorerExtensionGup* GetInstance();
	
	// Destroy must be called in LibShutdown to clean up the Singleton instance.
	static void Destroy();

	// Registers the HitPointsProperty in the Scene Explorer's property registry. 
	virtual DWORD Start();

	virtual void Stop();
	
	virtual ClassDesc* GetClassDesc();

private:
	SceneExplorerExtensionGup();
	~SceneExplorerExtensionGup();

	static SceneExplorerExtensionGup* sInstance;
	
	SceneExplorerExtensionClassDesc mClassDesc;
};

}
#endif