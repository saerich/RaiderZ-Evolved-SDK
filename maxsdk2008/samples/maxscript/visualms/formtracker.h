// ============================================================================
// FormTracker.h
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#ifndef __FORMTRACKER_H__
#define __FORMTRACKER_H__

class CFormEd;

// ============================================================================
// Transparent overlay window used with CFormEd. Used so messages are not sent
// to the GUI object child windows of CFormEd.
// ----------------------------------------------------------------------------
class CFormTracker : public CWnd
{
public:
	CFormEd *m_formEd;

	CFormTracker();
	virtual ~CFormTracker();
	BOOL Create(CWnd *pParentWnd);

	//{{AFX_VIRTUAL(CFormTracker)
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CFormTracker)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


//{{AFX_INSERT_LOCATION}}

#endif //__FORMTRACKER_H__
