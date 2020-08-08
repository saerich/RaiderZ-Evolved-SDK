// ============================================================================
// FormEd.cpp
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#include "stdafx.h"
#include "VisualMS.h"
#include "FormEd.h"
#include "VisualMSDoc.h"
#include "VisualMSThread.h"
#include "resource.h"

// From the MAX SDK
#include "iColorMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// ============================================================================
BEGIN_MESSAGE_MAP(CFormEd, CWnd)
	//{{AFX_MSG_MAP(CFormEd)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// ============================================================================
class CRubberRect : public CRectTracker
{
	void AdjustRect(int nHandle, LPRECT lpRect)
	{
		CRect rect = *lpRect;
		CheckRectInvert(rect);

		SetStatus(PANE_POS, "%d, %d", rect.left, rect.top);
		SetStatus(PANE_SIZE, "%d x %d", rect.right-rect.left, rect.bottom-rect.top);
		CRectTracker::AdjustRect(nHandle, lpRect);
	}
};

// ============================================================================
CFormEd::CFormEd()
{
	m_multiSizer.m_handleMask = HM_NONE;
	m_drawMultiSizer = false;
}

CFormEd::~CFormEd() {}

// ============================================================================
BOOL CFormEd::Create(CWnd *pParentWnd, const CRect &rect)
{
	BOOL res = CWnd::Create(
		AfxRegisterWndClass(
			CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW,
			LoadCursor(NULL, IDC_ARROW),
			NULL,
			NULL),
		NULL,
		WS_CHILD | WS_VISIBLE,//WS_CLIPSIBLINGS, // | WS_CLIPCHILDREN,
		rect,
		pParentWnd, 0, NULL
	);
	ASSERT(res != FALSE);

	SetFont(GetStdFont());
	GetDC()->SelectObject(GetStdFont());

	m_formTracker.Create(this);
	m_formTracker.UpdateWindow();
	m_formTracker.ShowWindow(SW_SHOW);

	return res;
}

// ============================================================================
void CFormEd::Invalidate()
{
	GetParent()->InvalidateRect(NULL, TRUE);
	InvalidateRect(NULL, TRUE);

	CVisualMSDoc *pDoc = CVisualMSDoc::GetDoc();
	pDoc->UpdateAllViews((CView*)GetParent(), HINT_CHANGESEL,
		(CObject*)pDoc->GetObj(pDoc->GetNextSel()));
}

// ============================================================================
void CFormEd::DrawRectTrackers(CDC *pDC)
{
	CVisualMSDoc *pDoc = CVisualMSDoc::GetDoc();

	if(m_drawMultiSizer)
		m_multiSizer.Draw(pDC);
	else
	{
		int idx = pDoc->GetNextSel(0);
		while(idx >= 0)
		{
			CGuiObj *pObj = pDoc->GetObj(idx);
			ASSERT(pObj != NULL);
			if(idx != pDoc->m_curSelBase)
			{
				pObj->ClearHandleMask();
				pObj->m_sizer.Draw(pDC);
				pObj->RestoreHandleMask();
			}
			else
				pObj->m_sizer.Draw(pDC);

			idx = pDoc->GetNextSel(idx);
		}
	}
}

// ============================================================================
void CFormEd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CGuiObj *pObj = NULL;
	CVisualMSDoc *pDoc = CVisualMSDoc::GetDoc();
	int snap = pDoc->GetGridSpacing();

	// Selection mode.
	if(pDoc->GetCreateMode() == ID_CTRL_SELECT)
	{
		// Test if directly hit an object.
		int hit = pDoc->HitTest(0, CRect(point,point));
		if(hit != 0)
		{
			CRect rect;
			GetWindowRect(rect);
			ClipCursor(&rect);

			// Adding / Subtracting from selection set.
			if(nFlags & MK_CONTROL)
			{
				if(pDoc->IsSelected(hit))
					pDoc->DeSelect(hit);
				else
					pDoc->Select(hit);
			}

			// Translation of multiple objects
			else if(pDoc->IsSelected(hit) && pDoc->GetSelectionCount() > 1)
			{
				CRect absRect;
				pDoc->GetSelectionRect(absRect);
				pDoc->Select(hit);
				m_multiSizer.m_rect = absRect;
				m_drawMultiSizer = true;
				Invalidate();

				// setup translation axis locking
				if(nFlags & MK_SHIFT &&
					m_multiSizer.HitTest(point) == CRectTracker::hitMiddle)
				{
					m_multiSizer.m_lockMovement = true;
					::GetCursorPos(&m_multiSizer.m_mouseStartPoint);
				}
				else
					m_multiSizer.m_lockMovement = false;

				m_multiSizer.m_oldRect = absRect;
				m_multiSizer.m_oldRect.right  -= absRect.left;
				m_multiSizer.m_oldRect.bottom -= absRect.top;

				if(m_multiSizer.Track(this, point, FALSE))
				{
					int ofsX = m_multiSizer.m_rect.left - absRect.left;
					int ofsY = m_multiSizer.m_rect.top  - absRect.top;

					// Now move everything selected
					int idx = pDoc->GetNextSel(0);
					while(idx >= 0)
					{
						CGuiObj *pObj = pDoc->GetObj(idx);
						pObj->GetRect(rect);
						rect.left += ofsX;
						rect.top  += ofsY;
						pObj->SetRect(rect);
						idx = pDoc->GetNextSel(idx);
					} 
				}
				m_drawMultiSizer = false;
			}
			else // Single object translation or scaling.
			{
				pDoc->DeSelectAll();
				pDoc->Select(hit);
				Invalidate();

				pObj = pDoc->GetObj(hit);
				pObj->GetRect(pObj->m_sizer.m_oldRect);

				// setup translation axis locking
				if(nFlags & MK_SHIFT &&
					pObj->m_sizer.HitTest(point) == CRectTracker::hitMiddle)
				{
					pObj->m_sizer.m_lockMovement = true;
					::GetCursorPos(&pObj->m_sizer.m_mouseStartPoint);
				}
				else
					pObj->m_sizer.m_lockMovement = false;

				// track the object
				if(pObj->m_sizer.Track(this, point, FALSE))
				{
					rect = pObj->m_sizer.m_rect;
					CheckRectInvert(rect);
					rect.right -= rect.left;
					rect.bottom -= rect.top;
					pObj->SetRect(rect);
				}
			}

			ClipCursor(NULL);
			Invalidate();

			return;
		}

		pDoc->DeSelectAll();
		Invalidate();

		// Track for multiple selection.
		bool hitSomething = false;
		CRubberRect rt;
		if(rt.TrackRubberBand(this, point, TRUE))
		{
			CPoint endPnt(rt.m_rect.right, rt.m_rect.bottom);
			CheckRectInvert(rt.m_rect);

			int dist = INT_MAX;
			int curSelBase = 0;
			int idx = 0;

			while((idx = pDoc->HitTest(idx, rt.m_rect)) != 0)
			{
				hitSomething = true;
				pDoc->Select(idx);

				// find the distance^2 between the center of the object and the end of the rectangle
				CRect rect;
				CGuiObj *pObj = pDoc->GetObj(idx);
				pObj->GetRect(rect);
				CPoint pnt(rect.left+rect.right/2, rect.top+rect.bottom/2);
				pnt = endPnt - pnt;
				int d = pnt.x*pnt.x + pnt.y*pnt.y;
				
				if(d < dist)
				{
					dist = d;
					curSelBase = idx;
				}
			}
			pDoc->m_curSelBase = curSelBase;
		}

		if(!hitSomething)
			pDoc->Select(0);

		Invalidate();
	}
	else // Creation mode
	{
		if(pDoc->GetUseGrid())
		{
			point.x = SnapValue(point.x, snap);
			point.y = SnapValue(point.y, snap);
		}

		CRect rect(point.x, point.y, 0, 0);

		// Track for new object creation
		CRubberRect rt;
		if(rt.TrackRubberBand(this, point, TRUE))
		{
			rect = rt.m_rect;

			// Snap the rectangle
			CheckRectInvert(rect);
			if(pDoc->GetUseGrid())
			{
				rect.right = SnapValue(rect.right, snap);
				rect.bottom = SnapValue(rect.bottom, snap);
			}

			rect.right -= rect.left;
			rect.bottom -= rect.top;
		}

		pObj = CGuiObj::Create(pDoc->GetCreateMode(), this, rect);
		if(pObj)
		{
			pDoc->DeSelectAll();
			int index = pDoc->m_objects.GetSize()-1;
			CGuiObj* last_obj = (CGuiObj*)pDoc->m_objects[index];
			if (last_obj->m_createID == ID_CODE)
				pDoc->InsertObj(index, pObj);
			else
				index = pDoc->AddObj(pObj);
			pDoc->Select(index);
			pObj->SetRect(rect);
			Invalidate();
		}

		pDoc->SetCreateMode(ID_CTRL_SELECT);
	}
}

// ============================================================================
// old behavior
/*
void CFormEd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CVisualMSDoc *pDoc = CVisualMSDoc::GetDoc(this);
	int snap = pDoc->GetGridSpacing();
	CGuiObj *obj = NULL;

	// Select object if directly hit.
	int hit = pDoc->HitTest(0, CRect(point,point));
	if(hit != 0)
	{
		pDoc->DeSelectAll();
		pDoc->Select(hit);
		Invalidate();
	}

	// Track selected objects for translation or scaling changes.
	for(int i = 1; i < pDoc->GetSize(); i++)
	{
		obj = pDoc->GetObj(i);
		if(obj->m_selected &&
			obj->m_sizer.HitTest(point) != CRectTracker::hitNothing)
		{
			if(obj->m_sizer.Track(this, point, FALSE))
			{
				CRect rect = obj->m_sizer.m_rect;
				CheckRectInvert(rect);
				rect.right -= rect.left;
				rect.bottom -= rect.top;
				obj->SetRect(rect);
			}
			Invalidate();
			return;
		}
	}

	pDoc->DeSelectAll();
	Invalidate();

	if(pDoc->GetUseGrid())
	{
		point.x = SnapValue(point.x, snap);
		point.y = SnapValue(point.y, snap);
	}

	// Track for multiple creation of new object or multiple selection.
	CRubberRect rt;
	if(rt.TrackRubberBand(this, point, TRUE))
	{
		// Snap the rectangle
		CheckRectInvert(rt.m_rect);
		if(pDoc->GetUseGrid())
		{
			rt.m_rect.right = SnapValue(rt.m_rect.right, snap);
			rt.m_rect.bottom = SnapValue(rt.m_rect.bottom, snap);
		}

		// Multiple selection.
		if(pDoc->GetCreateMode() == ID_CTRL_SELECT)
		{
			hit = 0;
			int i = 0;
			while((i = pDoc->HitTest(i, rt.m_rect)))
			{
				hit = 1;
				pDoc->Select(i);
			}
			if(hit) Invalidate();
		}
		else // Object Creation
		{
			rt.m_rect.right -= rt.m_rect.left;
			rt.m_rect.bottom -= rt.m_rect.top;
			CGuiObj *obj = CGuiObj::Create(pDoc->GetCreateMode(), this, rt.m_rect);
			if(obj)
			{
				obj->m_selected = TRUE;
				hit = pDoc->AddObj(obj);
				Invalidate();
			}
		}
	}

	if(hit == 0)
	{
		pDoc->DeSelectAll();
		pDoc->Select(0);
		Invalidate();
	}

	CWnd::OnLButtonDown(nFlags, point);
}
*/

// ============================================================================
void CFormEd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CWnd::OnLButtonUp(nFlags, point);
}

// ============================================================================
void CFormEd::OnRButtonUp(UINT nFlags, CPoint point) 
{
	CMenu menu;
	menu.LoadMenu(IDR_RCMENU);
	CMenu *pSubMenu = menu.GetSubMenu(0);

	CPoint pnt = point;
	ClientToScreen(&pnt);
	pSubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pnt.x, pnt.y, CMainFrame::GetFrame());

/*	CMenu menu;

	if(menu.LoadMenu(IDR_POPUP))
	{
		CPoint pnt = point;
		ClientToScreen(&pnt);
		CMenu *pSubMenu = menu.GetSubMenu(0);
		if(pSubMenu)
		{
			CGuiObj *hit = NULL;
			CGuiObj *obj = NULL;
			for(int i = 0; i < m_objects.GetSize(); i++)
			{
				obj = (CGuiObj*)m_objects[i];
				if(obj->m_selected && obj->HitTest(CRect(point, point)))
					hit = obj;
			}

			if(!hit)
			{
				for(int i = 0; i < m_objects.GetSize(); i++)
				{
					obj = (CGuiObj*)m_objects[i];
					obj->m_selected = FALSE;
					if(obj->HitTest(CRect(point, point)))
						hit = obj;
				}

				if(hit) hit->m_selected = TRUE;
				SelChanged();
			}

			if(hit)
			{
				for(int i = 0; i < hit->m_handlers.GetSize(); i++)
				{
					if(hit->m_handlers[i].m_use)
						pSubMenu->AppendMenu(MF_STRING, 0,
							(_T(" on ") + hit->m_handlers[i].m_name + _T(" ...")));
				}
			}

			pSubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pnt.x, pnt.y, this);
		}
	}
*/
	CWnd::OnRButtonUp(nFlags, point);
}

// ============================================================================
void CFormEd::OnMouseMove(UINT nFlags, CPoint point) 
{
	CVisualMSDoc *pDoc = CVisualMSDoc::GetDoc();
	CRect rect;
	pDoc->GetFormObj()->GetRect(rect);
	SetStatus(PANE_POS, "%d, %d", point.x, point.y);
	SetStatus(PANE_SIZE, "%d x %d", rect.right, rect.bottom);

	CWnd::OnMouseMove(nFlags, point);
}

// ============================================================================
void CFormEd::OnPaint() 
{
	CPaintDC dc(this);
	CVisualMSDoc *pDoc = CVisualMSDoc::GetDoc();

	if(pDoc->GetUseGrid())
	{
		CRect rect;
		GetClientRect(rect);
		DrawGrid(&dc, rect, pDoc->GetGridSpacing());
	}
}

// ============================================================================
BOOL CFormEd::OnEraseBkgnd(CDC* pDC) 
{
	CRect rect;
	GetClientRect(rect);
	pDC->FillRect(rect, CBrush::FromHandle(GetCustSysColorBrush(COLOR_BTNFACE)));
	return TRUE;
}

// ============================================================================
HBRUSH CFormEd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);

	switch(nCtlColor)
	{
	case CTLCOLOR_STATIC:
		pDC->SetBkColor(GetCustSysColor(COLOR_BTNFACE));
		pDC->SetTextColor(GetCustSysColor(COLOR_BTNTEXT));
		hbr = GetCustSysColorBrush(COLOR_BTNFACE);
		break;

	case CTLCOLOR_EDIT:
		pDC->SetBkColor(GetCustSysColor( COLOR_WINDOW ) );
		pDC->SetTextColor(GetCustSysColor( COLOR_WINDOWTEXT ) );
		hbr = GetCustSysColorBrush(COLOR_WINDOW) ;
		break;

	case CTLCOLOR_LISTBOX:
		pDC->SetBkColor(GetCustSysColor( COLOR_WINDOW ) );
		pDC->SetTextColor(GetCustSysColor( COLOR_WINDOWTEXT ) );
		hbr = GetCustSysColorBrush(COLOR_WINDOW) ;
		break;

	case CTLCOLOR_SCROLLBAR:
		pDC->SetBkColor(GetCustSysColor(COLOR_BTNFACE));
		hbr = GetCustSysColorBrush(COLOR_BTNFACE);
		break;
	}

	return hbr;
}

// ============================================================================
void CFormEd::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	if(::IsWindow(m_formTracker.m_hWnd))
		m_formTracker.MoveWindow(CRect(0,0,cx,cy));	
}

// ============================================================================
BOOL CFormEd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CVisualMSDoc *pDoc = CVisualMSDoc::GetDoc();
	if(pDoc->GetCreateMode() == ID_CTRL_SELECT)
	{
		for(int i = 1; i < pDoc->GetSize(); i++)
		{
			CGuiObj *obj = pDoc->GetObj(i);
			if(obj->IsSelected())
			{
				if(obj->m_sizer.SetCursor(pWnd, nHitTest))
					return TRUE;
			}
		}

		CPoint point;
		GetCursorPos(&point);
		ScreenToClient(&point);

		int hit = pDoc->HitTest(0, CRect(point,point));
		if(hit != 0 && !pDoc->IsSelected(hit))
		{
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			return TRUE;
		}
	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

// ============================================================================
CFormObj::CFormObj(CWnd *pParent, const CRect &rect)
	: CGuiObj(pParent)
{
	CRect newRect = rect;
	newRect.left += c_formOffset;
	newRect.top  += c_formOffset;

	if(!m_formEd.Create(pParent, newRect))
	{
		TRACE0("Failed to create form editor window\n");
		return;
	}

	SetHandleMask(HM_BOTTOMRIGHT|HM_BOTTOMMIDDLE|HM_RIGHTMIDDLE);
	SetRect(newRect);

	Class() = _T("rollout");
	Name()  = GetString(IDS_UNNAMED_ROLLOUT);
	Caption() = GetString(IDS_UNTITLED);

	m_properties[IDX_CLASS].m_flags |= FLAG_READONLY;
	m_properties[IDX_XPOS].m_flags |= FLAG_READONLY;
	m_properties[IDX_YPOS].m_flags |= FLAG_READONLY;
	m_properties[IDX_ENABLED].m_flags |= FLAG_READONLY;

	m_handlers.RemoveAll();
	m_handlers.Add(CHandler(false, _T("open"), _T(" do"), _T("")));
	m_handlers.Add(CHandler(false, _T("close"), _T(" do"), _T("")));
	m_handlers.Add(CHandler(false, _T("okToClose"), _T(" do"), _T("")));
	m_handlers.Add(CHandler(false, _T("resized"), _T("size do"), _T("")));
	m_handlers.Add(CHandler(false, _T("moved"), _T("pos do"), _T("")));

	m_selected = TRUE;
}

// ============================================================================
void CFormObj::GetRect(CRect &rect) const
{
	rect.left = c_formOffset;
	rect.top = c_formOffset;
	rect.right = (int)m_properties[IDX_WIDTH];
	rect.bottom = (int)m_properties[IDX_HEIGHT];
}

// ============================================================================
void CFormObj::SetRect(const CRect &rect)
{
	CRect r;
	r = rect;
	r.left = c_formOffset;
	r.top = c_formOffset;
	r.right += r.left;
	r.bottom += r.top;
	m_formEd.MoveWindow(r, TRUE);

	m_formEd.GetWindowRect(r);
	m_formEd.GetParent()->ScreenToClient(r);
	m_sizer.m_rect = r;
	r.right -= r.left;
	r.bottom -= r.top;

	m_properties[IDX_XPOS] = r.left;
	m_properties[IDX_YPOS] = r.top;
	m_properties[IDX_WIDTH] = r.right;
	m_properties[IDX_HEIGHT] = r.bottom;

	if(m_selected)
		m_formEd.GetParent()->InvalidateRect(NULL, TRUE);
}

// ============================================================================
int CFormObj::OnPropChange(int idx)
{
	CRect rect;
	switch(idx)
	{
		case IDX_WIDTH:
		case IDX_HEIGHT:
			GetRect(rect);
			SetRect(rect);
			return 1;
		case IDX_NAME:
			{
				CVisualMSDoc *pDoc = CVisualMSDoc::GetDoc();
				if(pDoc) pDoc->SetTitle(Name());
				return 1;
			}
	}
	return 0;
}

// ============================================================================
void CFormObj::Emit(CString &out)
{
	out.Format(_T("%s %s \"%s\"\n(\n"), Class(), Name(), Caption());
}

// ============================================================================
void CFormObj::Invalidate()
{
	m_formEd.GetParent()->InvalidateRect(NULL, TRUE);
	m_formEd.InvalidateRect(NULL, TRUE);
}

