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
// DESCRIPTION: dotnet_wraps.h  : MAXScript dotNet method declares
// AUTHOR: Larry.Minton - created Mar.15.2006
//***************************************************************************/
//

def_struct_primitive_debug_ok	( dotNet_getType,	dotNet,	"getType");
def_struct_primitive_debug_ok	( showConstructors,	dotNet,	"showConstructors");
/*
// see comments in utils.cpp
def_struct_primitive_debug_ok	( showAttributes,	dotNet,	"showAttributes");
def_struct_primitive_debug_ok	( getAttributes,	dotNet,	"getAttributes");
*/

def_struct_primitive_debug_ok	( dotNet_add_event_handler,			dotNet,	"addEventHandler");
def_struct_primitive_debug_ok	( dotNet_remove_event_handler,		dotNet,	"removeEventHandler");
def_struct_primitive_debug_ok	( dotNet_remove_event_handlers,		dotNet,	"removeEventHandlers");
def_struct_primitive_debug_ok	( dotNet_remove_all_event_handlers,	dotNet,	"removeAllEventHandlers");

def_struct_primitive_debug_ok	( dotNet_combineEnums,	dotNet,	"combineEnums");
def_struct_primitive_debug_ok	( dotNet_compareEnums,	dotNet,	"compareEnums");

def_struct_primitive_debug_ok	( dotNet_loadAssembly,	dotNet,	"loadAssembly");
