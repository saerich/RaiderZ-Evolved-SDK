//
//	resourceOverride.h
//
//
//	This header file contains product specific resource override code.
//	It remaps max resource ID to derivative product alternatives.
//
//

#ifndef _RESOURCEOVERRIDE_H
#define _RESOURCEOVERRIDE_H

#include "buildver.h"

#ifdef USE_ORIGINAL_VERTEX_PAINT

	#undef	IDS_CLASS_NAME
	#define IDS_CLASS_NAME			IDS_CLASS_NAME_ORIGINAL

#endif	// vertex_paint_original_replaces_paintbox

#ifdef GAME_VER

	// dialogs

#endif	// GAME_VER

#ifdef WEBVERSION

	// dialogs

	// strings

#endif	// WEBVERSION

#ifdef RENDER_VER

	// dialogs

#endif

#endif	// _RESOURCEOVERRIDE_H
