/**********************************************************************
 *<
	FILE: UIExtend.cpp

	DESCRIPTION: MaxScript user interface extensions

	CREATED BY: Ravi Karra, 1998

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

//#include "pch.h"
#include "MAXScrpt.h"
#include "MAXObj.h"

#include "resource.h"

#include "arcdlg.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "mxsagni.h"
#include "agnidefs.h"

#include "defextfn.h"
#	include "namedefs.h"

#include "definsfn.h"

/* ---------------------  Various callbacks ---------------------- */


class TestAxisChangeCallback:public AxisChangeCallback
{
	void proc(Interface *MAXScript_interface) { 
		CharStream* out = thread_local(current_stdout); 
		out->printf(_T("NumAxis %i\n"), MAXScript_interface->GetNumAxis()); 
	}
};

static TestAxisChangeCallback axisChangeCB;

Value* 
RegAxisChangeCB_cf(Value** arg_list, int count)
{
	check_arg_count(RegAxisChangeCB, 0, count);
	MAXScript_interface->RegisterAxisChangeCallback(&axisChangeCB);
	return &ok;
}

Value* 
UnRegAxisChangeCB_cf(Value** arg_list, int count)
{
	check_arg_count(UnRegAxisChangeCB, 0, count);
	MAXScript_interface->UnRegisterAxisChangeCallback(&axisChangeCB);
	return &ok;
}


class TestEventUser:public EventUser
{
	void Notify(){ 
		CharStream* out = thread_local(current_stdout); 
		out->printf(_T("Notified\n")); 
	}
};

static TestEventUser testEventUser;
			  
Value* 
RegDeleteUser_cf(Value** arg_list, int count)
{
	check_arg_count(RegDeleteUser, 0, count);
	MAXScript_interface->RegisterDeleteUser(&testEventUser);
	return &ok;
}

Value* 
UnRegDeleteUser_cf(Value** arg_list, int count)
{
	check_arg_count(UnRegDeleteUser, 0, count);
	MAXScript_interface->UnRegisterDeleteUser(&testEventUser);
	return &ok;
}

class TestNameEnumCallback:public NameEnumCallback
{
	void RecordName(TCHAR *name) {
			CharStream* out = thread_local(current_stdout); 
			out->printf(_T("name: %s\n"), name);
		}
};

TestNameEnumCallback nameEnum;

Value*
EnumAuxFiles_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(EnumAuxFiles, 1, count);	
	def_enumfile_types();
	MAXScript_interface->EnumAuxFiles(nameEnum, 
		GetID(enumfileTypes, elements(enumfileTypes), arg_list[0]));
	return &ok;
}

class TestExitMAXCallback:public ExitMAXCallback {
public:
	BOOL state;
	BOOL Exit(HWND hWnd) { 
		MessageBox(hWnd, _T("Max is exiting:"), "TestApp Info", MB_OK);
		return state;
	};
};


static TestExitMAXCallback exitCB;

Value*
RegExitMAXCallback_cf(Value** arg_list, int count)
{
	check_arg_count(RegExitMAXCallback, 1, count);
	exitCB.state = arg_list[0]->to_bool();
	MAXScript_interface->RegisterExitMAXCallback(&exitCB);
	return &ok;
}

Value*
UnRegExitMAXCallback_cf(Value** arg_list, int count)
{
	check_arg_count(UnRegExitMAXCallback, 0, count);
	MAXScript_interface->UnRegisterExitMAXCallback(&exitCB);
	return &ok;
}

class TestViewportDisplayCallback:public ViewportDisplayCallback {
	
	void Display(TimeValue t, ViewExp *vpt, int flags) 
		{ 
			CharStream* out = thread_local(current_stdout);
			out->printf(_T("ViewportDisplayCallback::Display\n"));
		}
	
	void GetViewportRect(TimeValue t, ViewExp *vpt, Rect *rect)
		{ CharStream* out = thread_local(current_stdout);
			out->printf(_T("ViewportDisplayCallback::GetViewportRect\n")); }

	BOOL Foreground()
		{ CharStream* out = thread_local(current_stdout);
			out->printf(_T("ViewportDisplayCallback::Foreground\n")); return FALSE;}
};

static TestViewportDisplayCallback vpDispCB;

Value*
NotifyVPDisplayCBChanged_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(NotifyVPDisplayCBChanged, 0, count);
	MAXScript_interface->NotifyViewportDisplayCallbackChanged(
		key_arg_or_default(preScene, &true_value)->to_bool(), &vpDispCB);
	return &ok;
}


class TestArcballCallback : public ArcballCallback{
public:
	int cnt;
	ArcballDialog *abDlg;
	CharStream* out;
	void StartDrag(){out->printf(_T("TestArcballCallback::StartDrag\n"));}
	void Drag(Quat q, BOOL buttonUp) {out->printf(_T("TestArcballCallback::Drag\n"));}
	void CancelDrag(){out->printf(_T("TestArcballCallback::CancelDrag\n"));}
	void BeingDestroyed() {out->printf(_T("%i TestArcballCallback::BeingDestroyed\n"), ++cnt); abDlg->DeleteThis();}
	void EndDrag(){out->printf(_T("TestArcballCallback::EndDrag\n"));}
};

static TestArcballCallback arcBall;

Value*
CreateArcballDialog_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(CreateArcballDialog, 0, count);
		
	arcBall.cnt = 0;
	arcBall.out = thread_local(current_stdout); 
	arcBall.abDlg = CreateArcballDialog(&arcBall, MAXScript_interface->GetMAXHWnd(), 
		key_arg(title)->to_string());	
	return &ok;
}