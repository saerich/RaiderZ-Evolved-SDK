// ============================================================================
// VisualMSDll.cpp
// Copyright ©2000, Discreet
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#include "stdafx.h"
#include "VisualMS.h"
#include "VisualMSDll.h"
#include "VisualMSThread.h"
#include "resource.h"

// from the max sdk
#include "max.h"
#include "iparamm2.h"

HINSTANCE g_hInst;
extern ClassDesc2* GetVisualMSDesc();

// ============================================================================
// For MFC DLL's you must use a global instance of a CWinApp derived class to do
// initialization instead of using the DllEntry function.
CVisualMSDLL theDLL;

// ============================================================================
class CVisualMSExitCallback : public ExitMAXCallback
{
	BOOL Exit(HWND hWnd) { return theDLL.Exit(); }
};
static CVisualMSExitCallback theExitCallback;

// ============================================================================
BOOL CVisualMSDLL::InitInstance()
{
	g_hInst = AfxGetResourceHandle();
	InitCommonControls();
	AfxInitRichEdit();
	GetCOREInterface()->RegisterExitMAXCallback(&theExitCallback);

	return TRUE;
}

// ============================================================================
BOOL CVisualMSDLL::Exit()
{
	while(m_threads.GetSize())
	{
		CVisualMSThread *pThread = (CVisualMSThread*)m_threads[0];
		pThread->Destroy();
		::WaitForSingleObject(pThread->m_threadExit.m_hObject, INFINITE);
	}
	return TRUE;
}

// ============================================================================
void CVisualMSDLL::RegisterThread(CVisualMSThread *pThread)
{
	m_threads.Add(pThread);
}

// ============================================================================
void CVisualMSDLL::RemoveThread(CVisualMSThread *pThread)
{
	for(int i = 0; i < m_threads.GetSize(); i++)
	{
		if(pThread == (CVisualMSThread*)m_threads[i])
			m_threads.RemoveAt(i);
	}
}

// ============================================================================
DLLEXPORT const TCHAR* LibDescription() { return GetString(IDS_LIBDESCRIPTION); }
DLLEXPORT int LibNumberClasses() { return 1; }
DLLEXPORT ClassDesc* LibClassDesc(int i)
{
	switch(i)
	{
	case 0: return GetVisualMSDesc();
	default: return 0;
	}
}
DLLEXPORT ULONG LibVersion() { return VERSION_3DSMAX; }

