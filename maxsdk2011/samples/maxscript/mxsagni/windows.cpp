/**********************************************************************
*<
FILE: windows.cpp

DESCRIPTION: 

CREATED BY: Larry Minton

HISTORY: Created 15 April 2007

*>	Copyright (c) 2007, All Rights Reserved.
**********************************************************************/

#include "MAXScrpt.h"
#include "Numbers.h"
#include "Strings.h"
#include "MXSAgni.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include <maxscript\macros\define_instantiation_functions.h>
#include "extclass.h"

// ============================================================================

#include <maxscript\macros\define_external_functions.h>
#	include "namedefs.h"

#include <maxscript\macros\define_instantiation_functions.h>
#	include "windows_wraps.h"
// -----------------------------------------------------------------------------------------

struct EnumProcArg
{
	Array* returnArray; // will contain the array to store results it
	bool checkParent; // if true, check window parent against parent member value below
	HWND parent; // parent window value to check against
	TCHAR* matchString; // string to test window text against
	EnumProcArg(Array* returnArray, bool checkParent, HWND parent, TCHAR* matchString = 0) 
		: returnArray(returnArray), checkParent(checkParent), parent(parent), matchString(matchString) {}
};

static const int CollectWindowData_DataSize = 8;

// function for collecting data for a specified window
static void CollectWindowData (HWND hWnd, Array *winData)
{
	TCHAR className[256] = {};
	TCHAR windowText[256] = {};
	int bufSize = _countof(windowText);

	// 1: HWND
	winData->append(IntegerPtr::intern((INT_PTR)hWnd));  // 1
	HWND ancestor = GetAncestor(hWnd,GA_PARENT);
	// 2: the parent HWND. This does not include the owner
	winData->append(IntegerPtr::intern((INT_PTR)ancestor)); // 2
	HWND par = GetParent(hWnd);
	// 3: the parent or owner HWND. If the window is a child window, the value is a handle to the parent window. 
	// If the window is a top-level window, the value is a handle to the owner window.
	winData->append(IntegerPtr::intern((INT_PTR)par)); // 3
	GetClassName(hWnd, className, bufSize);
	// 4: class name as a string – limit of 255 characters
	winData->append(new String(className)); // 4
	GetWindowText(hWnd, windowText, bufSize);
	// 5: window text as a string – limit of 255 characters
	winData->append(new String(windowText)); // 5
	HWND owner = GetWindow(hWnd,GW_OWNER);
	// 6: the owner's HWND 
	winData->append(IntegerPtr::intern((INT_PTR)owner)); // 6
	HWND root = GetAncestor(hWnd,GA_ROOT);
	// 7: root window HWND. The root window as determined by walking the chain of parent windows
	winData->append(IntegerPtr::intern((INT_PTR)root)); // 7
	HWND rootowner = GetAncestor(hWnd,GA_ROOTOWNER);
	// 8: owned root window HWND. The owned root window as determined by walking the chain of parent and owner 
	// windows returned by GetParent
	winData->append(IntegerPtr::intern((INT_PTR)rootowner)); // 8
}

Value*
getData_HWND_cf(Value** arg_list, int count)
{
	// windows.getHWNDData {<int_HWND>|#max} 
	check_arg_count(windows.getHWNDData, 1, count);
	HWND hWnd = NULL;
	if (arg_list[0] == n_max)  
		hWnd = MAXScript_interface->GetMAXHWnd();
	else
		hWnd = (HWND)arg_list[0]->to_intptr();
	if (hWnd == 0)
		hWnd = GetDesktopWindow();

	if (IsWindow(hWnd)) 
	{
		one_typed_value_local(Array* result);
		vl.result = new Array (CollectWindowData_DataSize);
		CollectWindowData(hWnd, vl.result);
		return_value(vl.result);
	}
	else
		return &undefined;
}

static BOOL CALLBACK CollectorEnumChildProc(HWND hWnd, LPARAM lParam)
{
	EnumProcArg *enumProcArg = (EnumProcArg*)lParam;

	HWND par = GetParent(hWnd);
	if (enumProcArg->checkParent && (par != enumProcArg->parent)) 
		return TRUE;

	Array *winData = new Array(CollectWindowData_DataSize);
	enumProcArg->returnArray->append(winData);
	CollectWindowData(hWnd, winData);

	return TRUE;
}

Value*
getChildren_HWND_cf(Value** arg_list, int count)
{
	// windows.getChildrenHWND {<int_HWND>|#max} parent:{<int_HWND>|#max}
	check_arg_count_with_keys(windows.getChildrenHWND, 1, count);
	HWND hWnd = NULL;
	if (arg_list[0] == n_max)  
		hWnd = MAXScript_interface->GetMAXHWnd();
	else
		hWnd = (HWND)arg_list[0]->to_intptr();

	Value *tmp;
	tmp = key_arg(parent);
	bool checkParent = false;
	HWND parent_hwnd = NULL;
	if (tmp == n_max)
	{
		parent_hwnd = MAXScript_interface->GetMAXHWnd();
		checkParent = true;
	}
	else if (tmp != &unsupplied)
	{
		parent_hwnd = (HWND)intptr_key_arg(parent,tmp,0);
		checkParent = true;
	}
	if (checkParent && parent_hwnd == 0)
		parent_hwnd = GetDesktopWindow();

	if (hWnd == 0 || IsWindow(hWnd)) 
	{
		one_typed_value_local(Array* result);
		vl.result = new Array (0);
		EnumProcArg enumProcArg(vl.result, checkParent, parent_hwnd);
		EnumChildWindows(hWnd, CollectorEnumChildProc, (LPARAM)&enumProcArg);
		return_value(vl.result);
	}
	else
		return &undefined;
}

static BOOL CALLBACK MatchEnumChildProc(HWND hWnd, LPARAM lParam)
{
	EnumProcArg *enumProcArg = (EnumProcArg*)lParam;

	TCHAR windowText[256];
	int bufSize = _countof(windowText);

	HWND par = GetParent(hWnd);
	if (enumProcArg->checkParent && (par != enumProcArg->parent)) 
		return TRUE;

	int res2 = GetWindowText(hWnd, windowText, bufSize);

	if (res2)
	{
		if (strnicmp(windowText, enumProcArg->matchString, bufSize) == 0)
		{
			CollectWindowData(hWnd, enumProcArg->returnArray);
			return FALSE;
		}
	}

	return TRUE;
}

Value*
getChild_HWND_cf(Value** arg_list, int count)
{
	// windows.getChildHWND {<int_HWND>|#max} <string> parent:{<int_HWND>|#max}
	check_arg_count_with_keys(windows.getChildHWND, 2, count);
	HWND hWnd = NULL;
	if (arg_list[0] == n_max)  
		hWnd = MAXScript_interface->GetMAXHWnd();
	else
		hWnd = (HWND)arg_list[0]->to_intptr();

	Value *tmp;
	tmp = key_arg(parent);
	bool checkParent = false;
	HWND parent_hwnd = NULL;
	if (tmp == n_max)
	{
		parent_hwnd = MAXScript_interface->GetMAXHWnd();
		checkParent = true;
	}
	else if (tmp != &unsupplied)
	{
		parent_hwnd = (HWND)intptr_key_arg(parent,tmp,0);
		checkParent = true;
	}
	if (checkParent && parent_hwnd == 0)
		parent_hwnd = GetDesktopWindow();

	if (hWnd == 0 || IsWindow(hWnd)) 
	{
		one_typed_value_local(Array* result);
		vl.result = new Array (0);
		EnumProcArg enumProcArg(vl.result, checkParent, parent_hwnd, arg_list[1]->to_string());
		BOOL notFound = EnumChildWindows(hWnd, MatchEnumChildProc, (LPARAM)&enumProcArg);
		if (notFound)
			return_value(&undefined)
		else
			return_value(vl.result);
	}
	else
		return &undefined;
}

Value*
getMAX_HWND_cf(Value** arg_list, int count)
{
	// windows.getMAXHWND() 
	check_arg_count(windows.getMAXHWND, 0, count);
	return IntegerPtr::intern((INT_PTR)MAXScript_interface->GetMAXHWnd());
}

Value*
getDesktop_HWND_cf(Value** arg_list, int count)
{
	// windows.getDesktopHWND() 
	check_arg_count(windows.getDesktopHWND, 0, count);
	return IntegerPtr::intern((INT_PTR)GetDesktopWindow());
}

Value*
send_message_cf(Value** arg_list, int count)
{
	// windows.sendMessage <int HWND> <int message> <int messageParameter1> <int messageParameter2>
	check_arg_count(windows.sendMessage, 4, count);

#ifdef USE_GMAX_SECURITY
	NOT_SUPPORTED_BY_PRODUCT("windows.sendMessage");
#else

	HRESULT hRes = SendMessage(
		(HWND)arg_list[0]->to_intptr(),
		(UINT)arg_list[1]->to_int(),
		(WPARAM)arg_list[2]->to_intptr(),
		(LPARAM)arg_list[3]->to_intptr());
	return Integer::intern(hRes);
#endif // USE_GMAX_SECURITY
}

Value*
post_message_cf(Value** arg_list, int count)
{
	// windows.postMessage <int HWND> <int message> <int messageParameter1> <int messageParameter2>
	check_arg_count(windows.postMessage, 4, count);

#ifdef USE_GMAX_SECURITY
	NOT_SUPPORTED_BY_PRODUCT("windows.postMessage");
#else

	BOOL res = PostMessage(
		(HWND)arg_list[0]->to_intptr(),
		(UINT)arg_list[1]->to_int(),
		(WPARAM)arg_list[2]->to_intptr(),
		(LPARAM)arg_list[3]->to_intptr());
	return bool_result(res);
#endif // USE_GMAX_SECURITY
}

Value*
process_messages_cf(Value** arg_list, int count)
{
	// windows.processPostedMessages() 
	check_arg_count(windows.processPostedMessages, 0, count);
	MSG wmsg;
	while (PeekMessage(&wmsg, NULL, 0 , 0, PM_REMOVE)) 
	{
		MAXScript_interface->TranslateAndDispatchMAXMessage(wmsg); 
	}
	return &ok;
}
