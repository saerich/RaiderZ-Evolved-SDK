// ============================================================================
// ArrayEdit.h
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#ifndef __ARRAYEDIT_H__
#define __ARRAYEDIT_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#define EN_RETURN 0x8888
#define LBN_DEL   0x8889

// ============================================================================
// CReturnEdit control
// sub classed edit control that catches the return key hit.
// ----------------------------------------------------------------------------
class CReturnEdit : public CEdit
{
public:
	bool m_bShiftDown;
	CReturnEdit();
	virtual ~CReturnEdit();

	//{{AFX_VIRTUAL(CReturnEdit)
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CReturnEdit)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

// ============================================================================
// CDelListBox control
// sub classed list box that catches the delete key hit
// ----------------------------------------------------------------------------
class CDelListBox : public CListBox
{
public:
	CDelListBox();

	//{{AFX_VIRTUAL(CDelListBox)
	//}}AFX_VIRTUAL

	virtual ~CDelListBox();

protected:
	//{{AFX_MSG(CDelListBox)
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSelChange();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};


// ============================================================================
// CArrayEdit dialog
// Array editor dialog box class
// ----------------------------------------------------------------------------
class CArrayEdit : public CDialog
{
public:
	CStringArray m_stringArray;
	CReturnEdit  m_edit;
	CDelListBox  m_lbxArray;

	CArrayEdit(CWnd* pParent = NULL);   // standard constructor
	CArrayEdit(const CStringArray &arr, CWnd* pParent = NULL);   // standard constructor

	//{{AFX_DATA(CArrayEdit)
	enum { IDD = IDD_ARRAYEDITOR };
	CString	m_string;
	//}}AFX_DATA


	//{{AFX_VIRTUAL(CArrayEdit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(CArrayEdit)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif //__ARRAYEDIT_H__


