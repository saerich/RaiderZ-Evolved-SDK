//**************************************************************************/
// Copyright (c) 1998-2007 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// AUTHOR: Nicolas Desjardins
// DATE: 2007-11-08 
//***************************************************************************/

// These includes from the max sdk are all grouped here and included in the rest
// of this project through this single header in order to prevent inconsistent 
// metadata problems when linking C++/CLI object files.
#pragma managed(push, off)
#pragma warning(push, 3)
// warning C4793, compiled as unmanaged, native code
#pragma warning(disable:4793)
// error C2220: warning treated as error - no 'object' file generated
// warning C4819: The file contains a character that cannot be represented in the current code page
#pragma warning(disable:4819)
// When compiled with /clr GetClassName is undefined in windows.h.  This causes
// a linker error against ReferenceTarget::GetClassName.  Core is compiled
// without /clr, so the evil macro replaces ReferenceTarget::GetClassName with
// either GetClassNameW or GetClassNameA.  The macro is forcibly applied here
// to maintain consistency with core and link successfully.
#ifndef GetClassName
	#ifdef UNICODE
	#define GetClassName  GetClassNameW
	#else
	#define GetClassName  GetClassNameA
	#endif // !UNICODE
#endif
#include <WindowsDefines.h>
#include <bitmap.h>
#include <iparamb2.h>
#include <IDaylightSystem.h>
#include <Noncopyable.h>
#include <INodeGIProperties.h>
#include <naturalLight.h>
#include <IAssemblyMgr.h>
#include <macrorec.h>
#include <maxscrpt/maxscrpt.h>
#include <marketDefaults.h>
#include <shadgen.h>
#include <icustattribcontainer.h>
#include <decomp.h>
#include <toneop.h>
#include <maxscrpt/maxclses.h>
#include <maxscrpt\name.h>
#include <notify.h>
#include <units.h>
#include <mouseman.h>
#include <control.h>
#include <genlight.h>
#include <istdplug.h>
#include <gfx.h>
#include <SingleWeakRefMaker.h>
#include <INodeMonitor.h>
#include <string>
#include "AssetManagement\IAssetAccessor.h"
#include "IPathConfigMgr.h"
#include <commdlg.h>
#include <maxicon.h>
#include <dllutilities.h>
#include <maxtypes.h>
#include <paramtype.h>
#include "IFileResolutionManager.h"
#include "AssetManagement\iassetmanager.h"
#include "assetmanagement\AssetType.h"
#include <time.h>
#include <trig.h>
#ifdef _DEBUG
#include <stdio.h>
#endif
#ifdef GLOBAL_COORD_SYS
#include <ICoordSys.h>
#include <ICoordSys_i.c>
#endif

#pragma warning(pop)
#pragma managed(pop)