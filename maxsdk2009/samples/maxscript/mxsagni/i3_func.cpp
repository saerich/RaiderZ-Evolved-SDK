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
	// filterstring <string> <delimter string> [splitEmptyTokens: boolean]
	check_arg_count_with_keys(FilterString, 2, count);
	type_check(arg_list[0], String, "FilterString [String to filter] [Tokens]");
	type_check(arg_list[1], String, "FilterString [String to filter] [Tokens]");

	Value* dummy = NULL;
	BOOL splitEmptyTokens = bool_key_arg(splitEmptyTokens,dummy,FALSE);

	one_typed_value_local(Array* result);
	vl.result = new Array(0);

	MCHAR* cmds       = arg_list[0]->to_string();
	MCHAR* separators = arg_list[1]->to_string();

	MCHAR* ePtr = save_string(cmds);
	DbgAssert(ePtr);
	MCHAR* rPtr = ePtr;
	MCHAR* eosPtr = ePtr + _tcslen(ePtr);

	if (splitEmptyTokens)
	{
		// Find a character in a string. returns a pointer to the first occurrence of c in str, or NULL if c is not found.
		MCHAR* nextCharIsSep = _tcschr(separators, *rPtr);
		while (nextCharIsSep && *nextCharIsSep)
		{
			vl.result->append(new String(_T("")));
			++rPtr;
			// handle case where separator is last character of line
			if (rPtr == eosPtr )
			{
				vl.result->append(new String(_T("")));
				break;
			}
			nextCharIsSep = _tcschr(separators, *rPtr);
		}
	}

	if (rPtr != eosPtr)
	{
		MCHAR* token = _tcstok( rPtr, separators );
		while( token != NULL )  
		{
			// While there are tokens in "string"
			vl.result->append(new String(token));
			if (splitEmptyTokens)
			{
				MCHAR *nextChar = token + _tcslen(token) + 1;
				if (nextChar <= eosPtr)
				{
					MCHAR *nextCharIsSep = _tcschr(separators, *nextChar);
					while (nextCharIsSep)
					{
						vl.result->append(new String(_T("")));
						++nextChar;
						// handle case where separator is last character of line
						if (nextChar > eosPtr )
							break;
						if (nextChar == eosPtr )
						{
							vl.result->append(new String(_T("")));
							break;
						}
						nextCharIsSep = _tcschr(separators, *nextChar);
					}
				}
			}
			token = _tcstok( NULL, separators );
		}
	}
	free(ePtr);

	return_value (vl.result);
}

