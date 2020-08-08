/*===========================================================================*\
 |  Ishani's stuff for MAX Script R3
 |
 |  FILE:	ish3_func.cpp
 |			New functions and variable definitions
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
#include "Strings.h"
#include "rollouts.h"

#include "shellapi.h"

#include "resource.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "i3.h"
#include "MXSAgni.h"

#include "defextfn.h"
#	include "namedefs.h"

#include "definsfn.h"

Value* launch_exec_cf(Value** arg_list, int count)
{
	check_arg_count(ShellLaunch, 2, count);

#ifdef USE_GMAX_SECURITY
	NOT_SUPPORTED_BY_PRODUCT("ShellLaunch");
#else

	type_check(arg_list[0], String, "ShellLaunch [Path and Filename] [Parameters]");
	type_check(arg_list[1], String, "ShellLaunch [Path and Filename] [Parameters]");

	Interface *ip = MAXScript_interface;
	HWND maxWnd = ip->GetMAXHWnd();

	TSTR file_name = arg_list[0]->to_filename();
	TCHAR tpath[MAX_PATH];
//	TCHAR tfile[MAX_PATH];
//	TCHAR textn[MAX_PATH];
//	char fnl[255];

//	BMMSplitFilename(arg_list[0]->to_string(), tpath, tfile, textn);
	BMMSplitFilename(file_name, tpath, NULL, NULL);
	BMMAppendSlash(tpath);
//	sprintf(fnl,"%s",tpath);

	HINSTANCE o = ShellExecute(maxWnd,"open",file_name,arg_list[1]->to_string(),tpath,SW_SHOWNORMAL);
	if((INT_PTR)o<=32) return &false_value;
	else return &true_value;
#endif // USE_GMAX_SECURITY

}

Value* filterstring_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(FilterString, 2, count);
	type_check(arg_list[0], String, "FilterString [String to filter] [Tokens]");
	type_check(arg_list[1], String, "FilterString [String to filter] [Tokens]");

	Value *dmy;
	BOOL splitEmptyTokens = bool_key_arg(splitEmptyTokens,dmy,FALSE);

	one_typed_value_local(Array* result);
	vl.result = new Array(0);

	char *cmds = arg_list[0]->to_string();
	char *seps = arg_list[1]->to_string();
	char *token;

	char*	rPtr;
	char*	ePtr = save_string(cmds);
	rPtr = ePtr;

	if (splitEmptyTokens)
	{
		char *nextCharIsSep = _tcschr(seps, *rPtr);
		while (nextCharIsSep)
		{
			vl.result->append(new String(_T("")));
			nextCharIsSep = _tcschr(seps, *(++rPtr));
		}
	}

	token = strtok( rPtr, seps );
	while( token != NULL )  
	{
		// While there are tokens in "string"
		vl.result->append(new String(token));
		if (splitEmptyTokens)
		{
			char *nextChar = token + _tcslen(token) + 1;
			char *nextCharIsSep = _tcschr(seps, *nextChar);
			while (nextCharIsSep)
			{
				vl.result->append(new String(_T("")));
				nextCharIsSep = _tcschr(seps, *(++nextChar));
			}
		}
	    token = strtok( NULL, seps );
	}

	free(ePtr);

	return_value (vl.result); // LAM - 5/18/01 - was: return vl.result;
}

