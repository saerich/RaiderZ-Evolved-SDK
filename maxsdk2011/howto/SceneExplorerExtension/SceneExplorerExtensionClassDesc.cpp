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

#include "SceneExplorerExtensionClassDesc.h"
#include "SceneExplorerExtensionGup.h"

// the following resolve warning LNK4248: unresolved typeref token (01000016) 
// for 'ParamBlockDesc2'; image may not run
// and warning LNK4248: unresolved typeref token (01000017) for 
// 'IParamMap2'; image may not run
// and warning LNK4248: unresolved typeref token (01000018) for 
// 'ParamMap2UserDlgProc'; image may not run
#include <iparamb2.h>
#include <iparamm2.h>

// the following resolves warning LNK4248: unresolved typeref token (01000015) 
// for 'Manipulator'; image may not run
class Manipulator{};



namespace SceneExplorerExtension
{

const Class_ID SceneExplorerExtensionClassDesc::kClassId(0x99292919, 0x72A92AAA);

SceneExplorerExtensionClassDesc::~SceneExplorerExtensionClassDesc()
{ }

int SceneExplorerExtensionClassDesc::IsPublic()
{
	return 1;
}

void* SceneExplorerExtensionClassDesc::Create(BOOL)
{
	return SceneExplorerExtensionGup::GetInstance();
}

const MCHAR* SceneExplorerExtensionClassDesc::ClassName()
{
	static const MCHAR className[] = "SceneExplorerExtension";
	return className;
}

SClass_ID SceneExplorerExtensionClassDesc::SuperClassID()
{
	return GUP_CLASS_ID;
}

Class_ID SceneExplorerExtensionClassDesc::ClassID()
{
	return kClassId;
}

const MCHAR* SceneExplorerExtensionClassDesc::Category()
{
	static const MCHAR category[] = "SceneExplorerExtension";
	return category;
}

int SceneExplorerExtensionClassDesc::NumActionTables()
{
	return 0;
}

}

