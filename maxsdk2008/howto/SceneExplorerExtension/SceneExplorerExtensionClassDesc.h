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

#ifndef SCENE_EXPLORER_EXTENSION_CLASS_DESC_H
#define SCENE_EXPLORER_EXTENSION_CLASS_DESC_H

#pragma unmanaged
#include "nativeInclude.h"

namespace SceneExplorerExtension
{

// Implement ClassDesc to tell Max about this plug-in.  This is all native code
// since there's no 'ref' keyword and the pragma above puts the compiler in 
// unmanaged mode.
class SceneExplorerExtensionClassDesc : public ClassDesc
{
public:
	virtual ~SceneExplorerExtensionClassDesc();

	virtual int IsPublic();
	virtual void * Create(BOOL);
	virtual const MCHAR* ClassName();
	virtual SClass_ID SuperClassID();
	virtual Class_ID ClassID();
	virtual const MCHAR* Category();
	virtual int NumActionTables();

private:
	static const Class_ID kClassId;
};

}


#endif