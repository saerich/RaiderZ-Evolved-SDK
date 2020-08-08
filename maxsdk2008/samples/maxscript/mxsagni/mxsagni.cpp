/**********************************************************************
 *<
	FILE: MXSAgni.cpp

	DESCRIPTION: Get/Set functions for system globals defined in MXSAgni

	CREATED BY: Ravi Karra, 1998

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

//#include "pch.h"
#include "MAXScrpt.h"
#include "Numbers.h"
#include "3DMath.h"
#include "MAXObj.h"
#include "Strings.h"

#include "randgenerator.h"

#include "resource.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "MXSAgni.h"
#include "ExtClass.h"

//#include "defextfn.h"
#include "definsfn.h"
#	include "namedefs.h"

#include "definsfn.h"
#	include "ExtFuncs.h"

Value*
get_MAX_version_cf(Value** arg_list, int count)
{
	check_arg_count(MAXVersion, 0, count);
	one_typed_value_local(Array* result); 
	vl.result = new Array(3);
	vl.result->append(Integer::intern(MAX_RELEASE));
	vl.result->append(Integer::intern(MAX_API_NUM));
	vl.result->append(Integer::intern(MAX_SDK_REV));
	return_value(vl.result);
}
 
Value*
get_root_node()
{
	return MAXRootNode::intern(MAXScript_interface->GetRootNode());
}

Value*
set_root_node(Value* val)
{
	throw RuntimeError (GetString(IDS_RK_ROOTNODE_IS_READ_ONLY));
    return &undefined;
}

Value*
set_rend_output_filename(Value* val)
{
	TSTR name = val->to_filename();
//	if (_tcscmp(name, _T("")) == 0)
	if (name.Length() == 0)
		MAXScript_interface->SetRendSaveFile(FALSE);
	BitmapInfo& bi = MAXScript_interface->GetRendFileBI();

	bi.SetName(name);
	// LAM - 8/4/04 - defects 483429, 493861 - reload PiData
	bi.SetDevice("");

	bi.ResetPiData();
	if (name.Length() != 0)
	{
		if (!TheManager->ioList.Listed())
			TheManager->ListIO();
		int idx = TheManager->ioList.ResolveDevice(&bi);
		if (idx != -1)
		{
			BitmapIO *IO = TheManager->ioList.CreateDevInstance(idx);
			if(IO) {
				DWORD pisize = IO->EvaluateConfigure();
				if (pisize) {
					bi.AllocPiData(pisize);
					IO->ReadCfg();
					IO->SaveConfigure(bi.GetPiData());
				}
				delete IO;
			}
		}
	}
	return val;
}

Value*
get_rend_output_filename()
{
	BitmapInfo& bi = MAXScript_interface->GetRendFileBI();
	return new String((TCHAR*)bi.Name());
}

Value*
get_rend_camNode()
{
	return MAXRootNode::intern(MAXScript_interface8->GetRendCamNode());
}

Value*
set_rend_camNode(Value* val)
{
	if( val==&undefined )
		 MAXScript_interface8->SetRendCamNode(NULL);
	else MAXScript_interface8->SetRendCamNode(val->to_node());
    return val;
}

Value*
set_rend_useImgSeq(Value* val)
{
	MAXScript_interface8->SetRendUseImgSeq(val->to_bool());
	return val;
}

Value*
get_rend_useImgSeq()
{
	return MAXScript_interface8->GetRendUseImgSeq() ? 
		&true_value : &false_value;
}

Value*
set_rend_imgSeqType(Value* val)
{
	MAXScript_interface8->SetRendImgSeqType(val->to_int());
	return val;
}

Value*
get_rend_imgSeqType()
{
	return Integer::intern(MAXScript_interface8->GetRendImgSeqType());
}

Value*
set_rend_preRendScript(Value* val)
{
	MAXScript_interface8->SetPreRendScript(val->to_string());
	return val;
}

Value*
get_rend_preRendScript()
{
	return new String( MAXScript_interface8->GetPreRendScript() );
}

Value*
set_rend_usePreRendScript(Value* val)
{
	MAXScript_interface8->SetUsePreRendScript(val->to_bool());
	return val;
}

Value*
get_rend_usePreRendScript()
{
	return MAXScript_interface8->GetUsePreRendScript() ? &true_value : &false_value;
}

Value*
set_rend_localPreRendScript(Value* val)
{
	MAXScript_interface8->SetLocalPreRendScript(val->to_bool());
	return val;
}

Value*
get_rend_localPreRendScript()
{
	return MAXScript_interface8->GetLocalPreRendScript() ? &true_value : &false_value;
}

Value*
set_rend_postRendScript(Value* val)
{
	MAXScript_interface8->SetPostRendScript(val->to_string());
	return val;
}

Value*
get_rend_postRendScript()
{
	return new String( MAXScript_interface8->GetPostRendScript() );
}

Value*
set_rend_usePostRendScript(Value* val)
{
	MAXScript_interface8->SetUsePostRendScript(val->to_bool());
	return val;
}

Value*
get_rend_usePostRendScript()
{
	return MAXScript_interface8->GetUsePostRendScript() ? &true_value : &false_value;
}

Value*
set_real_time_playback(Value* val)
{
	MAXScript_interface->SetRealTimePlayback(val->to_bool());
	return val;
}

Value*
get_real_time_playback()
{
	return MAXScript_interface->GetRealTimePlayback() ? 
		&true_value : &false_value;
}

Value*
set_playback_loop(Value* val)
{
	MAXScript_interface->SetPlaybackLoop(val->to_bool());
	return val;
}

Value*
get_playback_loop()
{
	return MAXScript_interface->GetPlaybackLoop() ? 
		&true_value : &false_value;
}

Value*
set_play_active_only(Value* val)
{
	MAXScript_interface->SetPlayActiveOnly(val->to_bool());
	return val;
}

Value*
get_play_active_only()
{
	return MAXScript_interface->GetPlayActiveOnly() ? 
		&true_value : &false_value;
}

Value*
set_enable_animate_button(Value* val)
{
	MAXScript_interface->EnableAnimateButton(val->to_bool());
	return val;
}

Value*
get_enable_animate_button()
{
	return MAXScript_interface->IsAnimateEnabled() ? 
		&true_value : &false_value;
}

Value*
set_animate_button_state(Value* val)
{
	MAXScript_interface->SetAnimateButtonState(val->to_bool());
	return val;
}

Value*
get_animate_button_state()
{
	return Animating() ? &true_value : &false_value;
}

Value*
set_fly_off_time(Value* val)
{
	MAXScript_interface->SetFlyOffTime(val->to_int());
	return val;
}

Value*
get_fly_off_time()
{
	return Integer::intern(MAXScript_interface->GetFlyOffTime());
}

Value*
set_xform_gizmos(Value *val)
{
	MAXScript_interface->SetUseTransformGizmo(val->to_bool());
	return val;
}

Value*
get_xform_gizmos()
{
	return MAXScript_interface->GetUseTransformGizmo() ? &true_value : &false_value;
}


Value*
set_constant_axis(Value *val)
{
	MAXScript_interface->SetConstantAxisRestriction(val->to_bool());
    return val;
}

Value*
get_constant_axis()
{
	return MAXScript_interface->GetConstantAxisRestriction() ? &true_value : &false_value;
}

#define ECO1144
// LAM - 8/29/03
#ifdef ECO1144
CoreExport void SetDontRepeatRefMsg(bool value, bool commitToIni);
CoreExport bool DontRepeatRefMsg();
Value*
set_DontRepeatRefMsg(Value *val)
{
	SetDontRepeatRefMsg(val->to_bool()==TRUE,false);
	return val;
}

Value*
get_DontRepeatRefMsg()
{
	return DontRepeatRefMsg() ? &true_value : &false_value;
}
#endif

CoreExport void SetInvalidateTMOpt(bool value, bool commitToIni);
CoreExport bool GetInvalidateTMOpt();
Value*
set_InvalidateTMOpt(Value *val)
{
	SetInvalidateTMOpt(val->to_bool()==TRUE,false);
	return val;
}

Value*
get_InvalidateTMOpt()
{
	return GetInvalidateTMOpt() ? &true_value : &false_value;
}

// mjm - 3.1.99 - use spinner precision for edit boxes linked to slider controls
/*
Value*
set_slider_precision(Value *val)
{
	SetSliderPrecision (val->to_int());
	return val;
}

Value*
get_slider_precision()
{
	return Integer::intern(GetSliderPrecision());
}
*/

Value*
get_useVertexDots()
{
   return getUseVertexDots() ? &true_value : &false_value;
}


Value*
set_useVertexDots(Value *val)
{
	setUseVertexDots ( (val->to_bool()) ? 1 : 0);
	needs_redraw_set();
	return val;
}

Value*
get_vertexDotType()
{
   return getVertexDotType() ? &true_value : &false_value;
}


Value*
set_vertexDotType(Value *val)
{
	setVertexDotType ( (val->to_bool()) ? 1 : 0);
	needs_redraw_set();
	return val;
}


Value*
get_useTrackBar()
{
	return MAXScript_interface->GetKeyStepsUseTrackBar () ? &true_value : &false_value;
}


Value*
set_useTrackBar(Value *val)
{
	MAXScript_interface->SetKeyStepsUseTrackBar (val->to_bool() ); 
	return val;
}


Value*
get_renderDisplacements()
{
	return MAXScript_interface->GetRendDisplacement () ? &true_value : &false_value;
}


#ifndef WEBVERSION
Value*
set_renderDisplacements(Value *val)
{
	MAXScript_interface->SetRendDisplacement  (val->to_bool() ); 
	return val;
}
#endif // WEBVERSION


Value*
get_renderEffects()
{
	return MAXScript_interface->GetRendEffects () ? &true_value : &false_value;
}


#ifndef WEBVERSION
Value*
set_renderEffects(Value *val)
{
	MAXScript_interface->SetRendEffects  (val->to_bool() ); 
	return val;
}
#endif // WEBVERSION


Value*
get_showEndResult()
{
	return MAXScript_interface->GetShowEndResult () ? &true_value : &false_value;
}


Value*
set_showEndResult(Value *val)
{
	MAXScript_interface->SetShowEndResult  (val->to_bool() ); 
	return val;
}


Value*
get_skipRenderedFrames()
{
	return MAXScript_interface->GetSkipRenderedFrames () ? &true_value : &false_value;
}


#ifndef WEBVERSION
Value*
set_skipRenderedFrames(Value *val)
{
	MAXScript_interface->SetSkipRenderedFrames (val->to_bool() ); 
	return val;
}
#endif // WEBVERSION

Value*
set_quiet_mode(Value *val)
{
	LogSys* thelog = MAXScript_interface->Log();
	thelog->SetQuietMode(val->to_bool() ? 1 : 0);
	return val;
}


Value*
get_spinner_wrap()
{
	return GetSpinnerWrap() ? &true_value : &false_value;
}

Value*
set_spinner_wrap(Value *val)
{
	SetSpinnerWrap(val->to_bool() ? 1 : 0);
	return val;
}

Value*
get_spinner_precision()
{
	return Integer::intern(GetSpinnerPrecision());
}

Value*
set_spinner_precision(Value *val)
{
	SetSpinnerPrecision(val->to_int());
	return val;
}

Value*
get_spinner_snap()
{
	return Float::intern(GetSnapSpinValue());
}

Value*
set_spinner_snap(Value *val)
{
	SetSnapSpinValue(val->to_float());
	return val;
}

Value*
get_spinner_useSnap()
{
	return GetSnapSpinner() ? &true_value : &false_value;
}

Value*
set_spinner_useSnap(Value *val)
{
	SetSnapSpinner(val->to_bool() ? 1 : 0);
	return val;
}



Value*
get_quiet_mode()
{
   LogSys* thelog = MAXScript_interface->Log();
   return (thelog->GetQuietMode() ? &true_value : & false_value);
}

Value*
get_maxGBufferLayers()
{
	return Integer::intern (GetMaximumGBufferLayerDepth());
}

Value*
set_maxGBufferLayers(Value *val)
{
	SetMaximumGBufferLayerDepth(val->to_int());
	return val;
}


//mcr_global_imp

// called in Parser::setup(), MXSAgni system globals & all name interns 
// are registered here
void MXSAgni_init1()
{
#	include "ExtGlbls.h"
#include "defimpfn.h"
#	include "namedefs.h"
}

#define INI_BUFFER_LEN 8192

Value*
get_INI_setting_cf(Value** arg_list, int count)
{
	TCHAR val[INI_BUFFER_LEN];
	if (count == 1 || count == 2) // get sections or section keys
	{
		GetPrivateProfileString(
			(count == 2) ? arg_list[1]->to_string() : NULL, 
			NULL, 
			_T(""),	val, INI_BUFFER_LEN, 
			arg_list[0]->to_filename());
		one_typed_value_local(Array* result); 
		TCHAR *cp = val;
		TCHAR tmpSec[INI_BUFFER_LEN] = {0};
		int i;
		vl.result = new Array(0);
		while(*cp) {
			i = 0;
			while((tmpSec[i++] = *cp++) != _T('\0'));
			vl.result->append(new String(tmpSec));
			}
		return_value(vl.result);
	}
	else
	{
		check_arg_count(getIniSetting, 3, count);
		GetPrivateProfileString(
			arg_list[1]->to_string(), 
			arg_list[2]->to_string(), 
			_T(""),	val, INI_BUFFER_LEN, 
			arg_list[0]->to_filename());
		return new String(val);	
	}
}

Value*
del_INI_setting_cf(Value** arg_list, int count)
{
	BOOL res;
	if (count == 2 || count == 3) // delete section or section key
		res = WritePrivateProfileString(
			arg_list[1]->to_string(), 
			(count == 3) ? arg_list[2]->to_string() : NULL, 
			NULL,
			arg_list[0]->to_filename());
	else
		check_arg_count(delIniSetting, 3, count);
	return &ok;	
}

#ifndef USE_GMAX_SECURITY

Value*
set_INI_setting_cf(Value** arg_list, int count)
{
	check_arg_count(setINISetting, 4, count);

	return WritePrivateProfileString(
		arg_list[1]->to_string(), 
		arg_list[2]->to_string(), 
		arg_list[3]->to_string(), 
		arg_list[0]->to_filename()) ? &true_value : &false_value;
}

#else // USE_GMAX_SECURITY

#define MAX_INI_FILESIZE 10000

Value*
set_INI_setting_cf(Value** arg_list, int count)
{
	check_arg_count(setINISetting, 4, count);

	// russom - 06/07/01
	//
	// New 06/07/01:
	//
	// We have removed setINIsettings from gmax.  But for
	// QE purposes, we will allow setINIsetting to only
	// write to a file called smoketest.err, which is limited
	// to 10k.  Everytime this method is called, we verify that
	// the filesize is increasing.  This will prevent someone
	// from writing 10k, moving the file, and writing another
	// 5k.  This also means that once the 10k limit is reached
	// within a gmax session, setINIsetting will no longer work.
	// ALL of this ONLY works when ENABLE_GMAX_SMOKETEST is defined
	// as an environment variable.
	//
	// Perform three security checks here:
	// 1. Check if the strings being written are greater than
	//    MAX_INI_FILESIZE
	// 2. Make sure the file size is increating
	// 3. Check the size of the ini file being written to and
	//    verify it is not larger than MAX_INI_FILESIZE
	//
	// #1:
	// Add the string lengths of section, key, and string.
	// If it is greater than MAX_INI_FILESIZE, throw an exception.
	//
	// #2:
	//
	// #3:
	// Get the file size of ini file the user is trying to write to.
	// If the file size exceeds MAX_INI_FILESIZE, throw an exception.
	// 
	// Don't check for file size limitations if the user is deleting
	// a key.  This is done when either arg2 or arg3 is NULL.
	//
	// POSSIBLE LIMITATION:  We are purposely not checking if this
	// is a new or existing key.  So, if the user creates an ini file
	// that exceeds MAX_INI_FILESIZE, they will not be able to change
	// key values even if doing so would reduce the file size.  They
	// could possibly exploit logic that does so.

#if !defined(WEBVERSION)
	if( _tgetenv(_T("ENABLE_GMAX_SMOKETEST")) == NULL ) {
#else
	if( _tgetenv(_T("ENABLE_PLASMA_SMOKETEST")) == NULL ) {
#endif
		NOT_SUPPORTED_BY_PRODUCT("setINIsetting");
		return 0;
	}

	// verify that the filename passed contains smokeTestLog.err
	if( _tcsstr( arg_list[0]->file_name(), _T("smokeTestLog.err") ) == NULL ) {
		NOT_SUPPORTED_BY_PRODUCT("setINIsetting");
		return 0;
	}

	TSTR strSmokeTestFile;
	strSmokeTestFile.printf("%s%s", MAXScript_interface->GetDir(APP_MAX_SYS_ROOT_DIR), _T("smokeTestLog.err") );

	static DWORD s_dwPrevFileSize = 0;

	// If the user is deleting keys, don't bother to checking anything
	if( (arg_list[2]->to_string() != NULL) 
		&& (arg_list[3]->to_string() != NULL) ) 
	{
		// Check string lengths
		DWORD dwStringLengths = 0;
		if( arg_list[1]->to_string() != NULL )
			dwStringLengths = _tcslen(arg_list[1]->to_string());

		// arg_list[2] and arg_list[3] were already checked for non-null
		if( dwStringLengths < MAX_INI_FILESIZE )
			dwStringLengths += _tcslen(arg_list[2]->to_string());
		if( dwStringLengths < MAX_INI_FILESIZE )
			dwStringLengths += _tcslen(arg_list[3]->to_string());

		if( dwStringLengths > MAX_INI_FILESIZE ) {
			// Throw exception
			throw RuntimeError (GetString(IDS_INI_FILESIZE_EXCEEDED));
			return 0;
		}

		// Check file size
		HANDLE hFile;
		hFile = CreateFile(strSmokeTestFile,
					GENERIC_READ,
					0,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
					NULL );
		
		if( hFile != INVALID_HANDLE_VALUE ) {
			DWORD dwFileSize = GetFileSize(hFile, NULL);
			CloseHandle(hFile);
			// Check filesize against MAX_INI_FILESIZE
			if( dwFileSize > MAX_INI_FILESIZE ) {
				// Throw exception
				throw RuntimeError (GetString(IDS_INI_FILESIZE_EXCEEDED));
				return 0;
			}
			// Make sure the filesize isn't decreasing
			if( s_dwPrevFileSize > dwFileSize ) {
				// Throw exception
				throw RuntimeError (GetString(IDS_INI_FILESIZE_EXCEEDED));
				return 0;
			}
			if( s_dwPrevFileSize == 0 )
				s_dwPrevFileSize = dwStringLengths;
			else
				s_dwPrevFileSize = dwFileSize;
		}
	}

	return WritePrivateProfileString(
		arg_list[1]->to_string(), 
		arg_list[2]->to_string(), 
		arg_list[3]->to_string(), 
		strSmokeTestFile) ? &true_value : &false_value;

}
#endif // USE_GMAX_SECURITY

Value*
has_INI_setting_cf(Value** arg_list, int count)
{
	TCHAR val[INI_BUFFER_LEN];
	if (count == 2 || count == 3) // get sections or section keys
	{
		GetPrivateProfileString(
			(count == 3) ? arg_list[1]->to_string() : NULL, 
			NULL, 
			_T(""),	val, INI_BUFFER_LEN, 
			arg_list[0]->to_filename());
		TCHAR *cp = val;
		TCHAR *key = arg_list[count-1]->to_string();
		while(*cp) {
			if (_tcsicmp(cp, key) == 0)
				return &true_value;
			while(*cp++);
		}
	}
	else
		check_arg_count(hasINISetting, 2, count);
	return &false_value;
}

Value*
get_file_version_cf(Value** arg_list, int count)
{
	check_arg_count(getFileVersion, 1, count);
	
	DWORD	tmp; 
	TSTR	fileName = arg_list[0]->to_filename();
	LPTSTR	file = fileName;
	DWORD	size = GetFileVersionInfoSize(file, &tmp);	
	if (!size) return &undefined;
	
	TCHAR*	data = (TCHAR*)malloc(size);	
	if(data && GetFileVersionInfo(file, NULL, size, data))
	{
		UINT len;
		VS_FIXEDFILEINFO *qbuf;
		TCHAR buf[256];
		if (VerQueryValue(data, "\\", (void**)&qbuf, &len))
		{
			DWORD fms = qbuf->dwFileVersionMS;
            DWORD fls = qbuf->dwFileVersionLS;
			DWORD pms = qbuf->dwProductVersionMS;
            DWORD pls = qbuf->dwProductVersionLS;
            	
			free(data);
			sprintf(buf, _T("%i,%i,%i,%i\t\t%i,%i,%i,%i"), 
				HIWORD(pms), LOWORD(pms), HIWORD(pls), LOWORD(pls),
				HIWORD(fms), LOWORD(fms), HIWORD(fls), LOWORD(fls));
			
			return new String(buf);
		}
		free(data);
	}
	return &undefined;
}

Value*
gen_class_id_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(genClassID, 0, count);
	
	CharStream* out = thread_local(current_stdout); 
	//Generating a unique ClassID
	ulong a = mxs_rand();
	ulong b = ClassIDRandGenerator->rand();

	Value *tmp;
	if (bool_key_arg(returnValue,tmp,FALSE) == FALSE)
	{

		a &= 0x7fffffff; // don't want highest order bit set, outputting as hex,
		b &= 0x7fffffff; // if convert that hex to number, MXS makes it a float
		out->printf(_T("#(0x%x, 0x%x)\n"), a, b);
		return &ok;
	}

	one_typed_value_local(Array* result); 
	vl.result = new Array(2);
	vl.result->append(Integer::intern(a));
	vl.result->append(Integer::intern(b));

	return_value(vl.result);
}

Value*
logsys_logEntry_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(logEntry, 1, count);
	DWORD type = 0;
	Value *tmp;
	if (bool_key_arg(error,tmp,FALSE)) type |= SYSLOG_ERROR;
	if (bool_key_arg(warning,tmp,FALSE)) type |= SYSLOG_WARN;
	if (bool_key_arg(info,tmp,FALSE)) type |= SYSLOG_INFO;
	if (bool_key_arg(debug,tmp,FALSE)) type |= SYSLOG_DEBUG;
	if (type == 0) type = SYSLOG_DEBUG;
	MAXScript_interface->Log()->LogEntry(type, NO_DIALOG, NULL, _T("%s\n"), arg_list[0]->to_string());
	return &ok;
}

Value*
logsys_logName_cf(Value** arg_list, int count)
{
	check_arg_count(logName, 1, count);
	MAXScript_interface->Log()->SetSessionLogName(arg_list[0]->to_filename());
	TCHAR * val = MAXScript_interface->Log()->GetSessionLogName();
	return new String(val);
}
//mcr_func_imp


