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

#ifndef NATIVE_INCLUDE_H
#define NATIVE_INCLUDE_H

// To avoid errors due to conflicting metadata definitions, native code #include
// directives are gathered in this file and included as a set.  That way, any
// metadata directives will be applied consistently across the assembly.

#pragma managed(push, off)
#include <windows.h>
#include <max.h>
#include <bmmlib.h>
#include <guplib.h>
#include <gup.h>
#include <noncopyable.h>
// allow methods taking an INode* parameter to be visible outside this assembly
#pragma make_public(INode)
#pragma managed(pop)

#endif