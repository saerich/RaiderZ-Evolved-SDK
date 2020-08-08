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

#ifdef GAME_VER

#endif	// GAME_VER

#ifdef WEBVERSION


#endif	// WEBVERSION

#ifdef DESIGN_VER

	#undef IDD_BLEND_ELEMENT
	#define IDD_BLEND_ELEMENT			IDD_BLEND_ELEMENT_VIZ
	
	#ifdef RENDER_VER

	#endif

#endif

#endif	// _RESOURCEOVERRIDE_H
