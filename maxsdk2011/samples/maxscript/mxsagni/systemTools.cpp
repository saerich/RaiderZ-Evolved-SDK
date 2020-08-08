#include "MAXScrpt.h"
#include "Numbers.h"
#include "Strings.h"
#include "MXSAgni.h"

#ifdef ScripterExport
#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

// ============================================================================

#include <maxscript\macros\define_external_functions.h>
#	include "namedefs.h"

#include <maxscript\macros\define_instantiation_functions.h>
#	include "systemTools_wraps.h"
// -----------------------------------------------------------------------------------------


Value* systemTools_IsDebugging_cf (Value** arg_list, int count)
{
	check_arg_count (IsDebugging, 0, count);
	return ( IsDebugging() ? &true_value : &false_value );
}

Value* systemTools_DebugPrint_cf(Value** arg_list, int count)
{
	check_arg_count(DebugPrint, 1, count);
	MCHAR* message = arg_list[0]->to_string();
	DebugPrint(_T("%s\n"), message);
	return &ok;
}

Value* systemTools_NumberOfProcessors_cf (Value** arg_list, int count)
{
	check_arg_count (NumberOfProcessors, 0, count);
	return Integer::intern (NumberOfProcessors());
}

Value* systemTools_IsWindows9x_cf (Value** arg_list, int count)
{
	check_arg_count (IsWindows9x, 0, count);
	return ( IsWindows9x() ? &true_value : &false_value );
}

Value* systemTools_IsWindows98or2000_cf (Value** arg_list, int count)
{
	check_arg_count (IsWindows98or2000, 0, count);
	return ( IsWindows98or2000() ? &true_value : &false_value );
}

Value* systemTools_GetScreenWidth_cf (Value** arg_list, int count)
{
	check_arg_count (GetScreenWidth, 0, count);
	return Integer::intern (GetScreenWidth());
}

Value* systemTools_GetScreenHeight_cf (Value** arg_list, int count)
{
	check_arg_count (GetScreenHeight, 0, count);
	return Integer::intern (GetScreenHeight());
}

Value*
systemTools_getEnvVar_cf(Value** arg_list, int count)
{
	// systemTools.getEnvVariable <string>
	check_arg_count(systemTools.getEnvVariable, 1, count);
	TCHAR *varName = arg_list[0]->to_string();
	DWORD bufSize = GetEnvironmentVariable(varName,NULL,0);
	if (bufSize == 0) return &undefined;
	TSTR varVal;
	varVal.Resize(bufSize);
	GetEnvironmentVariable(varName,varVal.data(),bufSize);
	return new String(varVal);
}

Value*
systemTools_setEnvVar_cf(Value** arg_list, int count)
{
	// systemTools.setEnvVariable <string> <string>
	check_arg_count(systemTools.setEnvVariable, 2, count);
	TCHAR *varName = arg_list[0]->to_string();
	TCHAR *varVal = NULL;
	if (arg_list[1] != &undefined)
		varVal = arg_list[1]->to_string();
	BOOL res = SetEnvironmentVariable(varName,varVal);
	return bool_result(res);
}
