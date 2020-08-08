// ============================================================================
// FormEdView.h
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#ifndef __FORMEDVIEW_H__
#define __FORMEDVIEW_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ============================================================================
// CFormEdView
// Main view class, parent of the form editor.
// ----------------------------------------------------------------------------
class CFormEdView : public CScrollView
{
protected: // create from serialization only
	CFormEdView();
	DECLARE_DYNCREATE(CFormEdView)

// Attributes
public:
	CVisualMSDoc* GetDocument();

// Operations
public:

// Overrides
	//{{AFX_VIRTUAL(CFormEdView)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFormEdView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif


protected:
	//{{AFX_MSG(CFormEdView)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnNcPaint();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in FormEdView.cpp
inline CVisualMSDoc* CFormEdView::GetDocument()
   { return (CVisualMSDoc*)m_pDocument; }
#endif


//{{AFX_INSERT_LOCATION}}

#endif //__FORMEDVIEW_H__
