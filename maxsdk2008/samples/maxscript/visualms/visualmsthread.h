// ============================================================================
// VisualMSThread.h
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#ifndef __VISUALMSTHREAD_H__
#define __VISUALMSTHREAD_H__

#include <afxmt.h>
#include "VisualMSDoc.h"
#include "MainFrm.h"

class CVisualMSThread;

// ============================================================================
class CVMSDocTemplate : public CSingleDocTemplate
{
public:
	CVisualMSThread *m_pThread;

	CVMSDocTemplate(
		UINT nIDResource,
		CRuntimeClass* pDocClass,
		CRuntimeClass* pFrameClass,
		CRuntimeClass* pViewClass,
		CVisualMSThread *pThread);
	~CVMSDocTemplate();

	CDocument* OpenDocumentFile(LPCTSTR lpszPathName, BOOL bMakeVisible);
};

// ============================================================================
class CVisualMSThread : public CWinThread
{
	DECLARE_DYNCREATE(CVisualMSThread)

protected:
	CVisualMSThread();           // protected constructor used by dynamic creation

public:
	CEvent m_threadInit;
	CEvent m_threadExit;

	CVMSDocTemplate *m_pDocTemplate;
	CVisualMSDoc *m_pMainDoc;

	// NOTE: m_pMainWnd is a member of CWinThread

	static CVisualMSThread* Create(bool bVisible = true);

   // ! This should only be called from within an executing VMS thread !
	static CVisualMSThread* GetRunningThread();

	void Destroy();
	void ShowApp();
	void HideApp();

	// Can be called anywhere, uses message to communicate with thread.
	void UpdateViews(LPARAM lHint = 0L, CObject* pHint = NULL);

	CVisualMSDoc* GetDoc() { return m_pMainDoc; }
	CMainFrame* GetFrame() { return (CMainFrame*)m_pMainWnd; }

	//{{AFX_VIRTUAL(CVisualMSThread)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

protected:
	virtual ~CVisualMSThread();

	//{{AFX_MSG(CVisualMSThread)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif //__VISUALMSTHREAD_H__
