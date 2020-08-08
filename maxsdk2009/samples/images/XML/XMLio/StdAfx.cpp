// stdafx.cpp : source file that includes just the standard includes
//	XMLio.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
#ifdef KAHN_OR_EARLIER
	#include "xslregistry_i.c"
	#include "xmldocutil_i.c"
	#include "xmldomutil_i.c"
	#include "vizpointer_i.c"
	#include "domloader_i.c"
#else
	#include "vizpointer_i.c"
	#include "domloader_i.c"
	#include "xmlmtl_i.c"
	#include "xmlmapmods_i.c"
#endif
//#include "xmlcoercion_i.c"