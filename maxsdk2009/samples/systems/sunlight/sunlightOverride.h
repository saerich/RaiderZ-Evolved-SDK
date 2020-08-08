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
//
//	resourceOverride.h
//
//
//	This header file contains product specific resource override code.
//	It remaps max resource ID to derivative product alternatives.
//
//

#ifndef _SUNLIGHTOVERRIDE_H
#define _SUNLIGHTOVERRIDE_H

#ifdef RENDER_VER

	// dialogs
	#undef	IDD_DAYPARAM
	#define IDD_DAYPARAM		IDD_DAYPARAM_VIZR

	#undef	IDD_SUNPARAM
	#define IDD_SUNPARAM		IDD_SUNPARAM_VIZR

#endif	// RENDER_VER

#endif // _SUNLIGHTOVERRIDE_H