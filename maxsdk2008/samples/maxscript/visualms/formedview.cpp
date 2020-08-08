// ============================================================================
// FormEdView.cpp
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#include "stdafx.h"
#include "VisualMS.h"
#include "VisualMSDoc.h"
#include "FormEdView.h"

// From the MAX SDK
#include "iColorMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ============================================================================
IMPLEMENT_DYNCREATE(CFormEdView, CScrollView)
BEGIN_MESSAGE_MAP(CFormEdView, CScrollView)
	//{{AFX_MSG_MAP(CFormEdView)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_NCPAINT()
	ON_WM_NCCALCSIZE()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ============================================================================
CFormEdView::CFormEdView() {}
CFormEdView::~CFormEdView() {}

// ============================================================================
BOOL CFormEdView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
//	if(m_formEd.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
//		return TRUE;
	
	return CScrollView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

// ============================================================================
BOOL CFormEdView::PreCreateWindow(CREATESTRUCT& cs)
{
	// Set views mapping mode for the scroll bars
	SetScrollSizes(MM_TEXT, CSize(0,0));

//	cs.dwExStyle = 0;
	cs.style |= WS_CLIPCHILDREN;

	return CScrollView::PreCreateWindow(cs);
}

// ============================================================================
void CFormEdView::OnDraw(CDC* pDC)
{
	CVisualMSDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

//	CGuiObj *pObj = pDoc->GetObj(0);
//	if(pObj && pObj->IsSelected())
//		pObj->m_sizer.Draw(pDC);
}

// ============================================================================
void CFormEdView::OnInitialUpdate()
{
	// BUG: why doesn't this function get called ?
//	MessageBox("OnInitialUpdate");
	CSize size;
	CVisualMSDoc::GetDoc()->GetFormSize(size);
	SetScrollSizes(MM_TEXT, size);
	CScrollView::OnInitialUpdate();
}

// ============================================================================
#ifdef _DEBUG
void CFormEdView::AssertValid() const { CScrollView::AssertValid(); }
void CFormEdView::Dump(CDumpContext& dc) const { CScrollView::Dump(dc); }
CVisualMSDoc* CFormEdView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CVisualMSDoc)));
	return (CVisualMSDoc*)m_pDocument;
}
#endif //_DEBUG

// ============================================================================
void CFormEdView::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	CScrollView::OnNcCalcSize(bCalcValidRects, lpncsp);

	lpncsp->rgrc[0].left += c_rulerSize;
	lpncsp->rgrc[0].top  += c_rulerSize;
	lpncsp->rgrc[0].right;//  -= 2;
	lpncsp->rgrc[0].bottom;// -= 2;
}

// ============================================================================
void CFormEdView::OnNcPaint()
{
	CScrollView::OnNcPaint();
	CWindowDC dc(this);

	CRect rect;
	GetWindowRect(rect);
	rect.right -= rect.left;
	rect.bottom -= rect.top;

	dc.FillRect(CRect(0, 0, rect.right, c_rulerSize),
		CBrush::FromHandle(GetCustSysColorBrush(COLOR_BTNFACE)));

	dc.FillRect(CRect(0, c_rulerSize, c_rulerSize, rect.bottom),
		CBrush::FromHandle(GetCustSysColorBrush(COLOR_BTNFACE)));

	HorizIndent(&dc, 0, 0, rect.right);
	VertIndent(&dc, 0, 1, rect.bottom);
	HorizIndent(&dc, 0, rect.bottom-2, c_rulerSize);
	VertIndent(&dc, rect.right-2, 1, c_rulerSize);

	rect.left = rect.top = c_rulerSize;
	DrawSunkenRect(&dc, rect);
}

// ============================================================================
void CFormEdView::OnPaint()
{
	CPaintDC dc(this);
	CGuiObj *pObj = GetDocument()->GetObj(0);
	if(pObj)
	{
		// don't draw handles if it's not selected
		if(!pObj->IsSelected())
			pObj->SetHandleMask(HM_NONE);

		// offset the tracker if the view is scrolled then draw it
		CPoint ofs = GetScrollPosition();
		CRect rect = pObj->m_sizer.m_rect; // store old rect
		pObj->m_sizer.m_rect -= ofs;       // calc scrolled rect
		pObj->m_sizer.Draw(&dc);
		pObj->m_sizer.m_rect = rect;  // restore old rect

		// restore the handle mask
		if(!pObj->IsSelected())
			pObj->RestoreHandleMask();
	}
}

// ============================================================================
BOOL CFormEdView::OnEraseBkgnd(CDC* pDC) 
{
	CRect rect;
	GetClientRect(rect);
	pDC->FillRect(rect, CBrush::FromHandle(GetCustSysColorBrush(COLOR_WINDOW)));
	return TRUE;
}

// ============================================================================
void CFormEdView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CVisualMSDoc *pDoc = GetDocument();
	ASSERT(pDoc != NULL);
	CGuiObj *pObj = pDoc->GetObj(0);
	ASSERT(pObj != NULL);

	CPoint ofs = GetScrollPosition();
	if(pObj->IsSelected() && !ofs.x && !ofs.y)
	{
		int hit = pObj->m_sizer.HitTest(point);
		if(hit != CRectTracker::hitNothing && hit != CRectTracker::hitMiddle)
		{
			CCtrlSizer::SetCursor(hit);
			ModifyStyle(WS_CLIPCHILDREN, 0);
			pObj->GetRect(pObj->m_sizer.m_oldRect);
			if(pObj->m_sizer.Track(this, point, FALSE))
			{
				CRect rect = pObj->m_sizer.m_rect;
				rect.right -= rect.left;
				rect.bottom -= rect.top;

				if(pDoc->GetUseGrid())
				{
					int snap = pDoc->GetGridSpacing();
					rect.right = SnapValue(rect.right, snap);
					rect.bottom = SnapValue(rect.bottom, snap);
				}

				pObj->SetRect(rect);
			}

			pDoc->UpdateAllViews(this, 0, NULL);
			ModifyStyle(0, WS_CLIPCHILDREN);
		}
		else
		{
			pDoc->DeSelectAll();
			pDoc->UpdateAllViews(NULL, HINT_CHANGESEL, NULL);
			CScrollView::OnLButtonDown(nFlags, point);
		}
	}

	if(!pObj->IsSelected())
	{
		pDoc->DeSelectAll();
		pDoc->UpdateAllViews(NULL, HINT_CHANGESEL, NULL);
	}
}

// ============================================================================
void CFormEdView::OnMouseMove(UINT nFlags, CPoint point)
{
	CGuiObj *pObj = GetDocument()->GetObj(0);
	ASSERT(pObj != NULL);

	CPoint ofs = GetScrollPosition();
	if(pObj->IsSelected() && !ofs.x && !ofs.y)
	{
		// Hit test the forms rect tracker if it is selected.
		int hit = pObj->m_sizer.HitTest(point);
		CCtrlSizer::SetCursor(hit);

		CRect rect;
		pObj->GetRect(rect);
		SetStatus(PANE_POS, "0, 0");
		SetStatus(PANE_SIZE, "%d x %d", rect.right, rect.bottom);
	}

	CScrollView::OnMouseMove(nFlags, point);
}

// ============================================================================
void CFormEdView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if(lHint == HINT_CHANGESEL)
	{
		InvalidateRect(NULL, TRUE);
		GetDocument()->Invalidate();
	}

	// Recalc the scroll sizes incase the form was resized
	int extra = c_formOffset + 2*c_handleSize;
	CSize size;
	GetDocument()->GetFormSize(size);
	size.cx += extra;
	size.cy += extra;
	SetScrollSizes(MM_TEXT, size);
}

// ============================================================================
BOOL CFormEdView::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll)
{
//	Invalidate();
	return CScrollView::OnScroll(nScrollCode, nPos, bDoScroll);
}

