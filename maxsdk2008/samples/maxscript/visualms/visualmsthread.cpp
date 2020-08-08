// ============================================================================
// VisualMSThread.cpp
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#include "stdafx.h"
#include "VisualMS.h"
#include "VisualMSThread.h"
#include "VisualMSDll.h"
#include "MainFrm.h"
#include "FormEdView.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ============================================================================
CVMSDocTemplate::CVMSDocTemplate(
	UINT nIDResource,
	CRuntimeClass* pDocClass,
	CRuntimeClass* pFrameClass,
	CRuntimeClass* pViewClass,
	CVisualMSThread *pThread)
   : CSingleDocTemplate(nIDResource, pDocClass, pFrameClass, pViewClass)
{
	m_pThread = pThread;

	m_pOnlyDoc = CreateNewDocument();
	ASSERT(m_pOnlyDoc != NULL);
	m_pThread->m_pMainDoc = (CVisualMSDoc*)m_pOnlyDoc;

	CFrameWnd *pFrame = CreateNewFrame(m_pOnlyDoc, NULL);
	ASSERT(pFrame != NULL);
	m_pThread->m_pMainWnd = pFrame;

	InitialUpdateFrame(pFrame, m_pOnlyDoc, FALSE);
	m_pOnlyDoc->OnNewDocument();
}

// ============================================================================
CVMSDocTemplate::~CVMSDocTemplate()
{
	if(m_pThread->m_pMainWnd) m_pThread->m_pMainWnd->DestroyWindow();
	delete m_pOnlyDoc;
}

// ============================================================================
CDocument* CVMSDocTemplate::OpenDocumentFile(LPCTSTR lpszPathName, BOOL bMakeVisible)
{
	return CSingleDocTemplate::OpenDocumentFile(lpszPathName, FALSE);
}

// ============================================================================
IMPLEMENT_DYNCREATE(CVisualMSThread, CWinThread)
BEGIN_MESSAGE_MAP(CVisualMSThread, CWinThread)
	//{{AFX_MSG_MAP(CVisualMSThread)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ============================================================================
CVisualMSThread* CVisualMSThread::Create(bool bVisible)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CVisualMSThread *pThread = (CVisualMSThread*)AfxBeginThread(RUNTIME_CLASS(CVisualMSThread));

	// Wait for the newly created thread to finish initializing before this
	// function continues
	::WaitForSingleObject(pThread->m_threadInit.m_hObject, INFINITE);

	// Register this thread with the DLL object so when the DLL is unloaded 
	// VMS threads can be destroyed and prompted for save
	theDLL.RegisterThread(pThread);

	if(bVisible) pThread->ShowApp();
	return pThread;
}

// ============================================================================
CVisualMSThread* CVisualMSThread::GetRunningThread()
{
	// Make sure this function is being called from within an executing 
	// CVisualMSThread by runtime checking the class.
	CWinThread *pThread = AfxGetThread();
	if(pThread && pThread->IsKindOf(RUNTIME_CLASS(CVisualMSThread)))
		return (CVisualMSThread*)pThread;
	return NULL;
}

// ============================================================================
CVisualMSThread::CVisualMSThread()
{
	m_bAutoDelete = FALSE;
	m_pDocTemplate = NULL;
	m_pMainDoc = NULL;
	m_pMainWnd = NULL;
}

// ============================================================================
CVisualMSThread::~CVisualMSThread()
{
}

// ============================================================================
BOOL CVisualMSThread::InitInstance()
{
	m_pDocTemplate = new CVMSDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CVisualMSDoc),
		RUNTIME_CLASS(CMainFrame),
		RUNTIME_CLASS(CFormEdView),
		this);

	// Set as child of max main window
	Interface *ip = GetCOREInterface();
	HWND hMainWnd = m_pMainWnd->GetSafeHwnd();
	SetWindowLongPtr(hMainWnd, GWLP_HWNDPARENT, (LONG_PTR)ip->GetMAXHWnd());
	ip->RegisterDlgWnd(hMainWnd);

	// Let the global vms creation function continue now that everything in
	// this thread has been initialized.
	m_threadInit.SetEvent();

	return TRUE;
}

// ============================================================================
int CVisualMSThread::ExitInstance()
{
	theDLL.RemoveThread(this);
	m_threadExit.SetEvent();
	delete m_pDocTemplate;
	delete this;
	return 0;
}

// ============================================================================
void CVisualMSThread::Destroy()
{
	::SendMessage(m_pMainWnd->GetSafeHwnd(), WM_CLOSE, 0, 0);
}

// ============================================================================
void CVisualMSThread::ShowApp()
{
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	UpdateViews();
}

// ============================================================================
void CVisualMSThread::HideApp()
{
	m_pMainWnd->ShowWindow(SW_HIDE);
}

// ============================================================================
void CVisualMSThread::UpdateViews(LPARAM lHint, CObject* pHint)
{
	::SendMessage(m_pMainWnd->GetSafeHwnd(), WM_UPDATEVIEWS, (WPARAM)pHint, lHint);
}

