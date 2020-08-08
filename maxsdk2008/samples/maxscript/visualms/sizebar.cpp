// ============================================================================
// SizeBar.cpp
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#include "stdafx.h"
#include "afxpriv.h"
#include "VisualMS.h"
#include "SizeBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const CSize c_sizeDefault(350,600);

// ============================================================================
IMPLEMENT_DYNAMIC(CSizeBar, CDialogBar)
BEGIN_MESSAGE_MAP(CSizeBar, CDialogBar)
	//{{AFX_MSG_MAP(CSizeBar)
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_NCHITTEST()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_CAPTURECHANGED()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ============================================================================
CSizeBar::CSizeBar()
{
	//{{AFX_DATA_INIT(CSizeBar)
	//}}AFX_DATA_INIT

	m_sizeMin = CSize(32, 32);
	m_sizeHorz = CSize(200, 400);
	m_sizeVert = CSize(200, 400);
	m_sizeFloat = CSize(200, 400);
	m_bTracking = FALSE;
	m_bInRecalcNC = FALSE;
	m_cxEdge = 7;

}
CSizeBar::~CSizeBar() { }

// ============================================================================
BOOL CSizeBar::Create(CWnd *pParentWnd, LPCTSTR lpszTemplateName,
	UINT nStyle, UINT nID)
{
	m_dwStyle = nStyle & CBRS_ALL;  // BUGFIX

//	nStyle &= ~CBRS_ALL;
//	nStyle |= CCS_NOPARENTALIGN|CCS_NOMOVEY|CCS_NODIVIDER|CCS_NORESIZE;

	m_sizeHorz = c_sizeDefault;
	m_sizeVert = c_sizeDefault;
	m_sizeFloat = c_sizeDefault;

	if(!CDialogBar::Create(pParentWnd, lpszTemplateName, nStyle, nID))
		return FALSE;

	if(!OnInitDialog())
		return FALSE;

	SetBarStyle(nStyle & CBRS_ALL);   // BUGFIX to sample

	return TRUE;
}

// ============================================================================
BOOL CSizeBar::Create(CWnd *pParentWnd, UINT nIDTemplate,
	UINT nStyle, UINT nID)
{
	return Create(pParentWnd, MAKEINTRESOURCE(nIDTemplate), nStyle, nID);
}

// ============================================================================
BOOL CSizeBar::IsHorz() const
{
	return (m_nDockBarID == AFX_IDW_DOCKBAR_TOP || m_nDockBarID == AFX_IDW_DOCKBAR_BOTTOM);
}

// ============================================================================
CPoint& CSizeBar::ClientToWnd(CPoint& point)
{
	if (m_nDockBarID == AFX_IDW_DOCKBAR_BOTTOM)
		point.y += m_cxEdge;
	else if (m_nDockBarID == AFX_IDW_DOCKBAR_RIGHT)
		point.x += m_cxEdge;

	return point;
}

// ============================================================================
BOOL CSizeBar::OnInitDialog() 
{
	return TRUE;
}

// ============================================================================
void CSizeBar::OnUpdateCmdUI(class CFrameWnd *pTarget, int bDisableIfNoHndler)
{
    UpdateDialogControls(pTarget, bDisableIfNoHndler);
}

// ============================================================================
void CSizeBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSizeBar)
	//}}AFX_DATA_MAP
}

// ============================================================================
CSize CSizeBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	CRect rect;
	m_pDockSite->GetControlBar(AFX_IDW_DOCKBAR_TOP)->GetWindowRect(rect);
	int nHorzWidth = bStretch ? 32767 : rect.Width() + 4;
	m_pDockSite->GetControlBar(AFX_IDW_DOCKBAR_LEFT)->GetWindowRect(rect);
	int nVertHeight = bStretch ? 32767 : rect.Height() + 4;

	if(IsFloating())
		return m_sizeFloat;
	else if(bHorz)
		return CSize(nHorzWidth, m_sizeFloat.cy);
	else
		return CSize(m_sizeVert.cx, nVertHeight);

 }

// ============================================================================
CSize CSizeBar::CalcDynamicLayout(int nLength, DWORD nMode)
{
	if(nMode & (LM_HORZDOCK | LM_VERTDOCK))
	{
		SetWindowPos(NULL, 0, 0, 0, 0,
			SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER |
			SWP_NOACTIVATE | SWP_FRAMECHANGED);
	 	m_pDockSite->RecalcLayout();
		return CDialogBar::CalcDynamicLayout(nLength, nMode);
	}

	if(nMode & LM_MRUWIDTH)
		return m_sizeFloat;

	if(nMode & LM_COMMIT)
	{
		m_sizeFloat.cx = nLength;
		return m_sizeFloat;
	}

	if(nMode & LM_LENGTHY)
		m_sizeFloat.cy = nLength;
	else
		return CSize(nLength, m_sizeFloat.cy);

	return m_sizeFloat;
}

// ============================================================================
void CSizeBar::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CDialogBar::OnWindowPosChanged(lpwndpos);

	if(m_bInRecalcNC)
		return;

	UINT nDockBarID = GetParent()->GetDlgCtrlID();

	// Return if dropped at same location
	if(nDockBarID == m_nDockBarID &&  // no docking side change
		(lpwndpos->flags & SWP_NOSIZE) &&  // no size change
		((m_dwStyle & CBRS_BORDER_ANY) != CBRS_BORDER_ANY)
	) return;

	m_nDockBarID = nDockBarID;

	// Force recalc the non-client area
	m_bInRecalcNC = TRUE;
	SetWindowPos(NULL, 0, 0, 0, 0,
		SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER |
		SWP_NOACTIVATE | SWP_FRAMECHANGED);
	m_bInRecalcNC = FALSE;
}

// ============================================================================
void CSizeBar::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	GetWindowRect(m_rectBorder);
	m_rectBorder = CRect(0, 0, m_rectBorder.Width(), m_rectBorder.Height());

	DWORD dwBorderStyle = m_dwStyle | CBRS_BORDER_ANY;

	switch(m_nDockBarID)
	{
	case AFX_IDW_DOCKBAR_TOP:
		dwBorderStyle &= ~CBRS_BORDER_BOTTOM;
		lpncsp->rgrc[0].bottom += -m_cxEdge;
		m_rectBorder.top = m_rectBorder.bottom - m_cxEdge;
		break;
	case AFX_IDW_DOCKBAR_BOTTOM:
		dwBorderStyle &= ~CBRS_BORDER_TOP;
		lpncsp->rgrc[0].top += m_cxEdge;
		m_rectBorder.bottom = m_rectBorder.top + m_cxEdge;
		break;
	case AFX_IDW_DOCKBAR_RIGHT:
		dwBorderStyle &= ~CBRS_BORDER_LEFT;
		lpncsp->rgrc[0].left += m_cxEdge;
		m_rectBorder.right = m_rectBorder.left + m_cxEdge;
		break;
	case AFX_IDW_DOCKBAR_LEFT:
		dwBorderStyle &= ~CBRS_BORDER_RIGHT;
		lpncsp->rgrc[0].right += -m_cxEdge;
		m_rectBorder.left = m_rectBorder.right - m_cxEdge;
		break;
	default:
		m_rectBorder.SetRectEmpty();
		break;
	}
	SetBarStyle(dwBorderStyle);
}

// ============================================================================
void CSizeBar::OnNcPaint() 
{
	EraseNonClient();

	CDC *pDC = GetWindowDC();

	pDC->SelectStockObject(WHITE_PEN);
	pDC->MoveTo(m_rectBorder.left+1, m_rectBorder.top);
	pDC->LineTo(m_rectBorder.left+1, m_rectBorder.bottom);

	CPen pen(PS_SOLID, 0, GetSysColor(COLOR_BTNSHADOW));
	pDC->SelectObject(&pen);
	pDC->MoveTo(m_rectBorder.right-2, m_rectBorder.top);
	pDC->LineTo(m_rectBorder.right-2, m_rectBorder.bottom);

	pDC->SelectStockObject(BLACK_PEN);
	pDC->MoveTo(m_rectBorder.right-1, m_rectBorder.top);
	pDC->LineTo(m_rectBorder.right-1, m_rectBorder.bottom);

	ReleaseDC(pDC);
}

// ============================================================================
LRESULT CSizeBar::OnNcHitTest(CPoint point) 
{
	if (IsFloating())
		return CDialogBar::OnNcHitTest(point);

	CRect rc;
	GetWindowRect(rc);
	point.Offset(-rc.left, -rc.top);
	if (m_rectBorder.PtInRect(point))
		return HTSIZE;
	else
		return CDialogBar::OnNcHitTest(point);
}

// ============================================================================
void CSizeBar::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
	if(m_bTracking)
		return;

	if((nHitTest == HTSIZE) && !IsFloating())
		StartTracking();
	else    
		CDialogBar::OnNcLButtonDown(nHitTest, point);
}

// ============================================================================
BOOL CSizeBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if((nHitTest != HTSIZE) || m_bTracking)
		return CDialogBar::OnSetCursor(pWnd, nHitTest, message);

	if(IsHorz())
		SetCursor(LoadCursor(NULL, IDC_SIZENS));
	else
		SetCursor(LoadCursor(NULL, IDC_SIZEWE));

	return TRUE;
}

// ============================================================================
void CSizeBar::StartTracking()
{
	SetCapture();

	// make sure no updates are pending
	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_UPDATENOW);
	m_pDockSite->LockWindowUpdate();

	m_ptOld = m_rectBorder.CenterPoint();
	m_bTracking = TRUE;

	m_rectTracker = m_rectBorder;
	if(!IsHorz()) m_rectTracker.bottom -= 4;

	DoInvertTracker(m_rectTracker);
}

// ============================================================================
void CSizeBar::OnCaptureChanged(CWnd *pWnd) 
{
	if(m_bTracking && pWnd != this)
		StopTracking(FALSE); // cancel tracking

	CDialogBar::OnCaptureChanged(pWnd);
}

// ============================================================================
void CSizeBar::StopTracking(bool bAccept)
{
	DoInvertTracker(m_rectTracker);
	m_pDockSite->UnlockWindowUpdate();
	m_bTracking = FALSE;
	ReleaseCapture();

	if(!bAccept)
		return;

	int maxsize, minsize, newsize;
	CRect rcc;
	m_pDockSite->GetClientRect(rcc);

	newsize = IsHorz() ? m_sizeHorz.cy : m_sizeVert.cx;
	maxsize = newsize + (IsHorz() ? rcc.Height() : rcc.Width());
	minsize = IsHorz() ? m_sizeMin.cy : m_sizeMin.cx;

	CPoint point = m_rectTracker.CenterPoint();
	switch (m_nDockBarID)
	{
	case AFX_IDW_DOCKBAR_TOP:
		newsize += point.y - m_ptOld.y; break;
	case AFX_IDW_DOCKBAR_BOTTOM:
		newsize += -point.y + m_ptOld.y; break;
	case AFX_IDW_DOCKBAR_LEFT:
		newsize += point.x - m_ptOld.x; break;
	case AFX_IDW_DOCKBAR_RIGHT:
		newsize += -point.x + m_ptOld.x; break;
	}

	newsize = max(minsize, min(maxsize, newsize));

	if(IsHorz())
		m_sizeHorz.cy = newsize;
	else
		m_sizeVert.cx = newsize;

	m_pDockSite->DelayRecalcLayout();
}

// ============================================================================
void CSizeBar::DoInvertTracker(const CRect& rect)
{
	ASSERT_VALID(this);
	ASSERT(!rect.IsRectEmpty());
	ASSERT(m_bTracking);

	CRect rct = rect, rcc, rcf;
	GetWindowRect(rcc);
	m_pDockSite->GetWindowRect(rcf);

	rct.OffsetRect(rcc.left - rcf.left, rcc.top - rcf.top);
	rct.DeflateRect(1, 1);

	CDC *pDC = m_pDockSite->GetDCEx(NULL, DCX_WINDOW|DCX_CACHE|DCX_LOCKWINDOWUPDATE);

	CBrush* pBrush = CDC::GetHalftoneBrush();
	HBRUSH hOldBrush = NULL;
	if(pBrush != NULL)
		hOldBrush = (HBRUSH)SelectObject(pDC->m_hDC, pBrush->m_hObject);

	pDC->PatBlt(rct.left, rct.top, rct.Width(), rct.Height(), PATINVERT);

	if(hOldBrush != NULL)
		SelectObject(pDC->m_hDC, hOldBrush);

	m_pDockSite->ReleaseDC(pDC);
}

// ============================================================================
void CSizeBar::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// only start dragging if clicked in "void" space
	if (m_pDockBar != NULL)
	{
		// start the drag
		ASSERT(m_pDockContext != NULL);
		ClientToScreen(&point);
		m_pDockContext->StartDrag(point);
	}
	else
	{
		CDialogBar::OnLButtonDown(nFlags, point);
	}
}

// ============================================================================
void CSizeBar::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (!m_bTracking)
		CDialogBar::OnLButtonUp(nFlags, point);
	else
	{
		ClientToWnd(point);
		StopTracking(TRUE);
	}
}

// ============================================================================
void CSizeBar::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// only toggle docking if clicked in "void" space
	if(m_pDockBar != NULL)
	{
		// toggle docking
		ASSERT(m_pDockContext != NULL);
		m_pDockContext->ToggleDocking();
	}
	else
	{
		CDialogBar::OnLButtonDblClk(nFlags, point);
	}
}

// ============================================================================
void CSizeBar::OnMouseMove(UINT nFlags, CPoint point) 
{
	if(IsFloating() || !m_bTracking)
	{
		CDialogBar::OnMouseMove(nFlags, point);
		return;
	}

	CPoint cpt = m_rectTracker.CenterPoint();
	ClientToWnd(point);

	if(IsHorz())
	{
		if(cpt.y != point.y)
		{
			DoInvertTracker(m_rectTracker);
			m_rectTracker.OffsetRect(0, point.y - cpt.y);
			DoInvertTracker(m_rectTracker);
		}
	}
	else 
	{
		if(cpt.x != point.x)
		{
			DoInvertTracker(m_rectTracker);
			m_rectTracker.OffsetRect(point.x - cpt.x, 0);
			DoInvertTracker(m_rectTracker);
		}
	}
}
