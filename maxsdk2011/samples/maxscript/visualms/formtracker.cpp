// ============================================================================
// FormTracker.cpp
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#include "stdafx.h"
#include "VisualMS.h"
#include "FormTracker.h"
#include "FormEd.h"


// ============================================================================
BEGIN_MESSAGE_MAP(CFormTracker, CWnd)
	//{{AFX_MSG_MAP(CFormTracker)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_RBUTTONUP()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// ============================================================================
CFormTracker::CFormTracker() {}
CFormTracker::~CFormTracker() {}

// ============================================================================
BOOL CFormTracker::Create(CWnd *pParentWnd)
{
	CRect rect;
	m_formEd = (CFormEd*)pParentWnd;
	pParentWnd->GetClientRect(rect);

	return CWnd::CreateEx(
		WS_EX_TRANSPARENT|WS_EX_TOPMOST,
		AfxRegisterWndClass(
			CS_HREDRAW|CS_VREDRAW,
			LoadCursor(NULL, IDC_CROSS),
			NULL,
			NULL),
		NULL,
		WS_CHILD | WS_VISIBLE,
		rect,
		m_formEd, 0, NULL
	);
}

// ============================================================================
void CFormTracker::OnPaint() 
{
	CPaintDC dc(this);
	m_formEd->DrawRectTrackers(&dc);
}

// ============================================================================
// Forward these messages to the editor.

void CFormTracker::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_formEd->OnLButtonDown(nFlags, point);
}

void CFormTracker::OnMouseMove(UINT nFlags, CPoint point) 
{
	m_formEd->OnMouseMove(nFlags, point);
}

BOOL CFormTracker::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	return m_formEd->OnSetCursor(pWnd, nHitTest, message);
}

void CFormTracker::OnRButtonUp(UINT nFlags, CPoint point) 
{
	m_formEd->OnRButtonUp(nFlags, point);
}
