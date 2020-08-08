// ============================================================================
// MainFrm.h
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#ifndef __MAINFRM_H__
#define __MAINFRM_H__

#include "PropBar.h"
#include "VmsToolBar.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define frameWndAtomID "__FRAMEWND_ATOM_ID__"


// ============================================================================
// CMainFrame - Main window frame class.
// ----------------------------------------------------------------------------
class CMainFrame : public CFrameWnd
{
	
protected:
	DECLARE_DYNCREATE(CMainFrame)

public:
	// ! this must be called from within an executing VMS thread !
	static CMainFrame* GetFrame();

	// This can be called from within other threads but the hWnd parameter 
	// requires an HWND that is a decendant of the frame window.
	static HWND GetFrameHWnd(HWND hWnd);

	void PressCtrlButton(UINT nID);
	int CreateMainToolBar();
	int CreateCtrlToolBar();
	int CreateStatusBar();
	int CreatePropBar();

	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void RecalcLayout(BOOL bNotify = TRUE);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	CMainFrame();
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CVmsToolBar   m_mainToolBar;
	CVmsToolBar   m_ctrlToolBar;
	CStatusBar    m_statusBar;
	CPropBar      m_propBar;

protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewGrid();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnClose();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnFileExit();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	afx_msg void OnChangeCreateMode(UINT nID);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif //__MAINFRM_H__
