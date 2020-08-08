// ============================================================================
// SizeBar.h
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#ifndef __SIZEBAR_H__
#define __SIZEBAR_H__


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// ============================================================================
class CSizeBar : public CDialogBar
{
	DECLARE_DYNAMIC(CSizeBar)

public:
	CSizeBar();
	virtual ~CSizeBar();
	BOOL Create(CWnd* pParentWnd, LPCTSTR lpszTemplateName,
			UINT nStyle, UINT nID);
	BOOL Create(CWnd* pParentWnd, UINT nIDTemplate,
			UINT nStyle, UINT nID);

	BOOL IsHorz() const;
	void StartTracking();
	void StopTracking(bool bAccept);
   void DoInvertTracker(const CRect& rect);
	CPoint& ClientToWnd(CPoint& point);

protected:
	CSize   m_sizeMin;
	CSize   m_sizeFloat;
	CSize   m_sizeHorz;
	CSize   m_sizeVert;
	CRect   m_rectBorder;
	CRect   m_rectTracker;
	UINT    m_nDockBarID;
	CPoint  m_ptOld;
	bool    m_bTracking;
	bool    m_bInRecalcNC;
	int     m_cxEdge;

	virtual BOOL OnInitDialog();
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);


	//{{AFX_DATA(CSizeBar)
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CSizeBar)
	public:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual CSize CalcDynamicLayout(int nLength, DWORD mMode);
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(CSizeBar)
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif //__SIZEBAR_H__


