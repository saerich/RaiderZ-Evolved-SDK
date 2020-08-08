// ============================================================================
// VmsToolBar.h
// Copyright ©2000, Discreet
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#ifndef __VMSTOOLBAR_H__
#define __VMSTOOLBAR_H__
#pragma once

// ============================================================================
class CVmsToolBar : public CToolBar
{
public:
	CVmsToolBar();
	virtual ~CVmsToolBar();
	BOOL LoadToolBar(LPCTSTR lpszResourceName);

	//{{AFX_VIRTUAL(CVmsToolBar)
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CVmsToolBar)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif //__VMSTOOLBAR_H__


