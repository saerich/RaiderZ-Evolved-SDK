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

#include "definsfn.h"
#include "extclass.h"

// ============================================================================

#include "defextfn.h"
#	include "namedefs.h"

#include "definsfn.h"
#	include "windows_wraps.h"
// -----------------------------------------------------------------------------------------

static HWND EnumImmediateChildWindowsOfOnly = NULL;

static BOOL CALLBACK CollectorEnumChildProc(HWND hWnd, LPARAM lParam)
{
	static TCHAR className[256];
	static TCHAR windowText[256];
	int bufSize = sizeof(TCHAR)*256;

	HWND par = GetParent(hWnd);
	if (EnumImmediateChildWindowsOfOnly != NULL && (par != EnumImmediateChildWindowsOfOnly)) return TRUE;

	int res1 = GetClassName(hWnd, className, bufSize);
	int res2 = GetWindowText(hWnd, windowText, bufSize);

	{
		Array *result = (Array*)lParam;
		Array *winData = new Array(5);
		result->append(winData);
		winData->append(IntegerPtr::intern((INT_PTR)hWnd));
		HWND ancestor = GetAncestor(hWnd,GA_PARENT);
		winData->append(IntegerPtr::intern((INT_PTR)ancestor));
		winData->append(IntegerPtr::intern((INT_PTR)par));
		winData->append(new String(className));
		winData->append(new String(windowText));
	}

	return TRUE;
}


Value*
getChildren_HWND_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(windows.getChildrenHWND, 1, count);
	HWND par;
	if (arg_list[0] == n_max)  
		par = MAXScript_interface->GetMAXHWnd();
	else
		par = (HWND)arg_list[0]->to_intptr();

	Value *tmp;
	tmp = key_arg(parent);
	if (tmp == n_max)
		EnumImmediateChildWindowsOfOnly = MAXScript_interface->GetMAXHWnd();
	else
		EnumImmediateChildWindowsOfOnly = (HWND)intptr_key_arg(parent,tmp,0);

	if (par == 0 || IsWindow(par)) 
	{

		one_typed_value_local(Array* result);
		vl.result = new Array (0);
		if (par == 0)
			EnumWindows(CollectorEnumChildProc, (LPARAM)vl.result);
		else
			EnumChildWindows(par, CollectorEnumChildProc, (LPARAM)vl.result);
		return_value(vl.result);
	}
	else
		return &undefined;
}

static BOOL CALLBACK MatchEnumChildProc(HWND hWnd, LPARAM lParam)
{
	static TCHAR className[256];
	static TCHAR windowText[256];
	int bufSize = sizeof(TCHAR)*256;

	HWND par = GetParent(hWnd);
	if (EnumImmediateChildWindowsOfOnly != NULL && (par != EnumImmediateChildWindowsOfOnly)) return TRUE;

	int res1 = GetClassName(hWnd, className, bufSize);
	int res2 = GetWindowText(hWnd, windowText, bufSize);

	if (res2)
	{
		DataPair *result = (DataPair*)lParam;

		if (strnicmp(windowText, result->v1->to_string(), bufSize) == 0)
		{
			Array *winData = new Array(5);
			result->v2 = winData;
			winData->append(IntegerPtr::intern((INT_PTR)hWnd));
			HWND ancestor = GetAncestor(hWnd,GA_PARENT);
			winData->append(IntegerPtr::intern((INT_PTR)ancestor));
			HWND par = GetParent(hWnd);
			winData->append(IntegerPtr::intern((INT_PTR)par));
			winData->append(new String(className));
			winData->append(new String(windowText));
			return FALSE;
		}
	}

	return TRUE;
}


Value*
getChild_HWND_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(windows.getChildHWND, 2, count);
	HWND par;
	if (arg_list[0] == n_max)  
		par = MAXScript_interface->GetMAXHWnd();
	else
		par = (HWND)arg_list[0]->to_intptr();

	Value *tmp;
	tmp = key_arg(parent);
	if (tmp == n_max)
		EnumImmediateChildWindowsOfOnly = MAXScript_interface->GetMAXHWnd();
	else
		EnumImmediateChildWindowsOfOnly = (HWND)intptr_key_arg(parent,tmp,0);

	if (par == 0 || IsWindow(par)) 
	{
		one_typed_value_local(DataPair* result);
		vl.result = new DataPair();
		vl.result->v1 = new String(arg_list[1]->to_string());
		if (par == 0)
			EnumWindows(MatchEnumChildProc, (LPARAM)vl.result);
		else
			EnumChildWindows(par, MatchEnumChildProc, (LPARAM)vl.result);
		return_value(vl.result->v2);
	}
	else
		return &undefined;
}


Value*
getMAX_HWND_cf(Value** arg_list, int count)
{
	check_arg_count(windows.getMAXHWND, 0, count);
	return IntegerPtr::intern((INT_PTR)MAXScript_interface->GetMAXHWnd());
}

Value*
getDesktop_HWND_cf(Value** arg_list, int count)
{
	check_arg_count(windows.getDesktopHWND, 0, count);
	return IntegerPtr::intern((INT_PTR)GetDesktopWindow());
}

Value*
send_message_cf(Value** arg_list, int count)
{
	check_arg_count(sendMessage, 4, count);

#ifdef USE_GMAX_SECURITY
	NOT_SUPPORTED_BY_PRODUCT("sendMessage");
#else

	HRESULT hRes = SendMessage(
		(HWND)arg_list[0]->to_intptr(),
		(UINT)arg_list[1]->to_int(),
		(WPARAM)arg_list[2]->to_intptr(),
		(LPARAM)arg_list[3]->to_intptr());
	return Integer::intern(hRes);
#endif // USE_GMAX_SECURITY
}