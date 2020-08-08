//
//	resOverride.h
//
//
//	This header file contains product specific resource override code.
//	It remaps max resource ID to derivative product alternatives.
//
//	This file is included by include/reslib.h
//

#ifndef _OVERRIDE_H
#define _OVERRIDE_H

#ifdef RENDER_VER_ABS

	#undef  IDR_MAXDOCUMENT
	#define IDR_MAXDOCUMENT	IDR_MAXDOCUMENT_ABS

#endif // RENDER_VER
#ifdef RENDER_VER_CIV3D

	#undef  IDR_MAXDOCUMENT
	#define IDR_MAXDOCUMENT	IDR_MAXDOCUMENT_CIV3D

#endif // RENDER_VER





#endif	// _OVERRIDE_H