/**********************************************************************
*<
FILE: sysinfo.cpp

DESCRIPTION: 

CREATED BY: Larry Minton

HISTORY: Created 15 April 2007

*>	Copyright (c) 2007, All Rights Reserved.
**********************************************************************/

#include "MAXScrpt.h"
#include "Numbers.h"
#include "3DMath.h"
#include "Strings.h"
#include "MXSAgni.h"

#ifdef ScripterExport
#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

// ============================================================================

#include "defextfn.h"
#	include "namedefs.h"

#include "definsfn.h"
#	include "sysInfo_wraps.h"

#include "agnidefs.h"

// -----------------------------------------------------------------------------------------

Value*
getProcessAffinity()
{	
	one_value_local(res);
	vl.res = &undefined;
	int processID = GetCurrentProcessId();
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processID);
	DWORD_PTR ProcessAffinityMask, SystemAffinityMask;
	if (GetProcessAffinityMask(hProcess,&ProcessAffinityMask, &SystemAffinityMask))
		vl.res = IntegerPtr::intern(ProcessAffinityMask);
	CloseHandle( hProcess );
	return_value (vl.res);
}
Value*
setProcessAffinity(Value* val)
{
	DWORD ProcessAffinityMask = val->to_int();
	int processID = GetCurrentProcessId();
	HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, processID);
	SetProcessAffinityMask(hProcess,ProcessAffinityMask);
	CloseHandle( hProcess );
	return val;
}

Value*
getSystemAffinity()
{
	Value *res = &undefined;
	int processID = GetCurrentProcessId();
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processID);
	DWORD_PTR ProcessAffinityMask, SystemAffinityMask;
	if (GetProcessAffinityMask(hProcess,&ProcessAffinityMask, &SystemAffinityMask))
		res = IntegerPtr::intern(SystemAffinityMask);
	CloseHandle( hProcess );
	return res;
}
Value*
setSystemAffinity(Value* val)
{
	throw RuntimeError (_T("Cannot set system affinity"));
	return val;
}

Value*
getDesktopSize()
{
	int wScreen = GetScreenWidth();
	int hScreen = GetScreenHeight();
	return new Point2Value((float)wScreen,(float)hScreen);
}

Value*
getDesktopBPP()
{
	HDC hdc = GetDC(GetDesktopWindow());
	int bits = GetDeviceCaps(hdc,BITSPIXEL);
	ReleaseDC(GetDesktopWindow(),hdc);
	return Integer::intern(bits);
}

Value*
getSystemMemoryInfo_cf(Value** arg_list, int count)
{
	check_arg_count(getSystemMemoryInfo, 0, count);
	MEMORYSTATUS ms;
	ms.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&ms);
	one_typed_value_local(Array* result);
	vl.result = new Array (7);
	vl.result->append(Integer64::intern(ms.dwMemoryLoad));   // percent of memory in use
	vl.result->append(Integer64::intern(ms.dwTotalPhys)); // bytes of physical memory
	vl.result->append(Integer64::intern(ms.dwAvailPhys)); // free physical memory bytes
	vl.result->append(Integer64::intern(ms.dwTotalPageFile));// bytes of paging file
	vl.result->append(Integer64::intern(ms.dwAvailPageFile));// free bytes of paging file
	vl.result->append(Integer64::intern(ms.dwTotalVirtual)); // user bytes of address space
	vl.result->append(Integer64::intern(ms.dwAvailVirtual)); // free user bytes
	return_value(vl.result);
}

//-----------------------------------------------------------------------------
// Structure for GetProcessMemoryInfo()
//
// This is from psapi.h which is only included in the Platform SDK

typedef struct _PROCESS_MEMORY_COUNTERS {
	DWORD cb;
	DWORD PageFaultCount;
	SIZE_T PeakWorkingSetSize;
	SIZE_T WorkingSetSize;
	SIZE_T QuotaPeakPagedPoolUsage;
	SIZE_T QuotaPagedPoolUsage;
	SIZE_T QuotaPeakNonPagedPoolUsage;
	SIZE_T QuotaNonPagedPoolUsage;
	SIZE_T PagefileUsage;
	SIZE_T PeakPagefileUsage;
} PROCESS_MEMORY_COUNTERS;

//-- This is in psapi.dll (NT4 and W2k only)

typedef BOOL (WINAPI *GetProcessMemoryInfo)(HANDLE Process,PROCESS_MEMORY_COUNTERS* ppsmemCounters,DWORD cb);

GetProcessMemoryInfo getProcessMemoryInfo;
//-----------------------------------------------------------------------------

Value*
getMAXMemoryInfo_cf(Value** arg_list, int count)
{
	check_arg_count(getMAXMemoryInfo, 0, count);

	one_typed_value_local(Array* result);
	HMODULE hPsapi = LoadLibrary("psapi.dll");
	BOOL resOK = FALSE;
	if (hPsapi) {
		getProcessMemoryInfo = (GetProcessMemoryInfo)GetProcAddress(hPsapi,"GetProcessMemoryInfo");
		if (getProcessMemoryInfo) {
			PROCESS_MEMORY_COUNTERS pmc;
			int processID = GetCurrentProcessId();
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
			if (getProcessMemoryInfo(hProcess,&pmc,sizeof(pmc))) {
				vl.result = new Array (9);
				vl.result->append(Integer64::intern(pmc.PageFaultCount));
				vl.result->append(Integer64::intern(pmc.PeakWorkingSetSize));
				vl.result->append(Integer64::intern(pmc.WorkingSetSize));
				vl.result->append(Integer64::intern(pmc.QuotaPeakPagedPoolUsage));
				vl.result->append(Integer64::intern(pmc.QuotaPagedPoolUsage));
				vl.result->append(Integer64::intern(pmc.QuotaPeakNonPagedPoolUsage));
				vl.result->append(Integer64::intern(pmc.QuotaNonPagedPoolUsage));
				vl.result->append(Integer64::intern(pmc.PagefileUsage));
				vl.result->append(Integer64::intern(pmc.PeakPagefileUsage));
				resOK=TRUE;
			}
			CloseHandle( hProcess );
		}
		FreeLibrary(hPsapi);
	}
	if (resOK) 	return_value(vl.result);
	return_value(&undefined);// LAM - 5/18/01 - was: return &undefined;

}

Value*
getMAXPriority()
{
	def_process_priority_types();
	int processID = GetCurrentProcessId();
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processID);
	int priority=GetPriorityClass(hProcess);
	CloseHandle( hProcess );
	return GetName(processPriorityTypes, elements(processPriorityTypes), priority);
}

Value*
setMAXPriority(Value* val)
{
	def_process_priority_types();
	int type = GetID(processPriorityTypes, elements(processPriorityTypes), val);
	int processID = GetCurrentProcessId();
	HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, processID);
	SetPriorityClass(hProcess,type);
	CloseHandle( hProcess );
	return val;
}

Value*
getusername()
{
	TCHAR username[MAX_PATH];
	DWORD namesize = MAX_PATH;
	GetUserName(username,&namesize);
	return new String (username);
}

Value*
getcomputername()
{
	TCHAR computername[MAX_COMPUTERNAME_LENGTH+1];
	DWORD namesize = MAX_COMPUTERNAME_LENGTH+1;
	GetComputerName(computername,&namesize);
	return new String (computername);
}

Value*
getSystemDirectory()
{
	TCHAR sysDir[MAX_PATH];
	GetSystemDirectory(sysDir,MAX_PATH);
	return new String (sysDir);
}

Value*
getWinDirectory()
{
	TCHAR winDir[MAX_PATH];
	GetWindowsDirectory(winDir,MAX_PATH);
	return new String (winDir);
}

Value*
getTempDirectory()
{
	TCHAR tempDir[MAX_PATH];
	GetTempPath(MAX_PATH,tempDir);
	return new String (tempDir);
}

Value*
getCurrentDirectory()
{
	TCHAR currentDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH,currentDir);
	return new String (currentDir);
}

Value*
setCurrentDirectory(Value* val)
{
	DispInfo info;
	GetUnitDisplayInfo(&info);
	if (!SetCurrentDirectory(val->to_filename()))
		throw RuntimeError (GetString(IDS_ERROR_SETTING_CURRENT_DIRECTORY), val);
	return val;
}

Value*
getCPUcount()
{
	SYSTEM_INFO     si;
	GetSystemInfo(&si);
	return Integer::intern(si.dwNumberOfProcessors);
}

Value*
getLanguage_cf(Value** arg_list, int count)
{
	// sysinfo.getLanguage user:true
	check_arg_count_with_keys(getLanguage, 0, count);
	one_typed_value_local(Array* result);
	vl.result = new Array (3);
	TCHAR tmpString[256];
	LANGID wLang;
	Value* tmp;
	BOOL user = bool_key_arg(user,tmp,FALSE);
	if (user)
		wLang = GetUserDefaultLangID();
	else
		wLang = GetSystemDefaultLangID();
	VerLanguageName(wLang,tmpString,256);
	vl.result->append(Integer::intern(PRIMARYLANGID(wLang)));
	vl.result->append(Integer::intern(SUBLANGID(wLang)));
	vl.result->append(new String(tmpString));
	return_value(vl.result);
}


/* --------------------- plug-in init --------------------------------- */
// this is called by the dlx initializer, register the global vars here
void sysInfo_init()
{
#include "sysInfo_glbls.h"
}
