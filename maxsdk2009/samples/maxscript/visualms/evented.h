// ============================================================================
// EventEd.h
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#ifndef __EVENTED_H__
#define __EVENTED_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GuiObj.h"
#include "VisualMSDoc.h"
#include "resource.h"

// ============================================================================
// CEventEd - event handler editor dialog class
// ----------------------------------------------------------------------------
class CEventEd : public CDialog
{
protected:
	CVisualMSDoc *m_pDoc;
	CGuiObj  *m_pCurObj;
	int       m_startSel;
	int       m_curSel;
	static CFont m_font;
	static	  CFont* editor_font;

public:
	CEventEd(CWnd* pParent = NULL);
	CEventEd(CVisualMSDoc *pDoc, int startSel, CWnd* pParent = NULL);
	void BuildEventList();

	//{{AFX_DATA(CEventEd)
	enum { IDD = IDD_EVENTED };
	CStatic	m_txtEvent;
	CStatic	m_txtControl;
	CButton	m_btnOk;
	CComboBox	m_cbxEvents;
	CComboBox	m_cbxControls;
	CRichEditCtrl	m_editor;
	CStatic	m_stcDeclaration;
	//}}AFX_DATA


	//{{AFX_VIRTUAL(CEventEd)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(CEventEd)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangeControl();
	afx_msg void OnSelChangeEvent();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	virtual void OnOK();
	afx_msg void OnClose();
	afx_msg void OnEditUndo();
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditPaste();
	afx_msg void OnEditClear();
	afx_msg void OnEditSelectAll();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif //__EVENTED_H__


