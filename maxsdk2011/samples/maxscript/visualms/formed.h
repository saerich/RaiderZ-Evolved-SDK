// ============================================================================
// FormEd.h
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#ifndef __FORMED_H__
#define __FORMED_H__

#include "GuiObj.h"
#include "FormTracker.h"

// ============================================================================
// CFormEd
// Container class for GUI objects, handles creation and editing.
// ----------------------------------------------------------------------------
class CFormEd : public CWnd
{
public:
	CFormTracker m_formTracker;
	CCtrlSizer   m_multiSizer;
	bool         m_drawMultiSizer;

	CFormEd();
	virtual ~CFormEd();
	BOOL Create(CWnd *pParentWnd, const CRect &rect);
	void Invalidate();
	void DrawRectTrackers(CDC *pDC);

	//{{AFX_VIRTUAL(CFormEd)
	//}}AFX_VIRTUAL

public:
	//{{AFX_MSG(CFormEd)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ============================================================================
// CFormObj
// GUI object wrapper for the form editor.
// ----------------------------------------------------------------------------
class CFormObj : public CGuiObj
{
public:
	CFormEd m_formEd;
	CFormObj(CWnd *pParent, const CRect &rect);
	virtual void GetRect(CRect &rect) const;
	virtual void SetRect(const CRect &rect);
	virtual void Emit(CString &out);
	virtual int  OnPropChange(int idx);
	virtual void Invalidate();
};


//{{AFX_INSERT_LOCATION}}

#endif //__FORMED_H__

