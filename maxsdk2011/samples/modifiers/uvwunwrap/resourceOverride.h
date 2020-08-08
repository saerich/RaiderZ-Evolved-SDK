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

#ifdef DESIGN_VER

	#undef  IDD_RELAXDIALOG
	#define IDD_RELAXDIALOG			IDD_RELAXDIALOG_VIZ

	#undef  IDD_UNWRAP_MAPPARAMS
	#define IDD_UNWRAP_MAPPARAMS	IDD_UNWRAP_MAPPARAMS_VIZ

#endif // design_ver

#endif	// _RESOURCEOVERRIDE_H

