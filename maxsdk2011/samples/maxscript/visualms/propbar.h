// ============================================================================
// PropBar.h
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#ifndef __PROPBAR_H__
#define __PROPBAR_H__

#include "SizeBar.h"
#include "resource.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// ============================================================================
// CPropBar
// Resizable/docable bar that contains a tab control.
// ----------------------------------------------------------------------------
class CPropBar : public CSizeBar
{
	DECLARE_DYNAMIC(CPropBar)

protected:
	CView *m_pActiveView;
	CPtrArray m_views;

public:
	CPropBar();

	// Add a view to tab control
	BOOL AddView(LPCTSTR lpszLabel, CRuntimeClass *pViewClass,
		CDocument *pDoc, CCreateContext *pContext = NULL);
	void SelectTab(int nSel);

	//{{AFX_DATA(CPropBar)
	enum { IDD = IDD_PROPBAR };
	CTabCtrl	m_tabCtrl;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CPropBar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:
	virtual BOOL OnInitDialog();

	//{{AFX_MSG(CPropBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSelChangePropTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif //__PROPBAR_H__
