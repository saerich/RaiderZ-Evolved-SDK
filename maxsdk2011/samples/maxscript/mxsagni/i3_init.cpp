/*===========================================================================*\
 |  Ishani's stuff for MAX Script R3
 |
 |  FILE:	ish3_init.cpp
 |			Initialize this mess!
 | 
 |  AUTH:   Harry Denholm
 |			Copyright(c) KINETIX 1998
 |			All Rights Reserved.
 |
 |  HIST:	Started 7-7-98
 | 
\*===========================================================================*/

//#include "pch.h"
#include "MAXScrpt.h"
#include "MAXObj.h"
#include "parser.h"

#include "resource.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "i3.h"
#include "MXSAgni.h"

#include <maxscript\macros\define_external_functions.h>
#	include "namedefs.h"

#include <maxscript\macros\define_instantiation_functions.h>

extern void ProgressBarInit();

def_visible_primitive( launch_exec,			"ShellLaunch");
def_visible_primitive( filterstring,		"FilterString");

// called from MXSAgni_main.cpp
// Installs all rollout controls
void install_i3_custom_controls()
{
	
	// Progress Bar CC
	ProgressBarInit();

	// Timer Event CC
	install_rollout_control(
		Name::intern(s_timer),
		ish3_Timer::create
		);

}