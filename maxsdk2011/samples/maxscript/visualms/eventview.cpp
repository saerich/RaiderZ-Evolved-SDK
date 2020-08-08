// ============================================================================
// EventView.cpp
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#include "stdafx.h"
#include "VisualMS.h"
#include "EventView.h"
#include "VisualMSDoc.h"
#include "EventEd.h"
#include "ArrayEdit.h"

#include "Max.h"
#include "iColorMan.h"

#pragma warning(push)
#pragma warning(disable : 4800)   // forcing value to bool 'true' or 'false'

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ============================================================================
IMPLEMENT_DYNCREATE(CEventView, CListView)
BEGIN_MESSAGE_MAP(CEventView, CListView)
	//{{AFX_MSG_MAP(CEventView)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemChanged)
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ============================================================================
CEventView::CEventView()
{
	m_pCurObj = NULL;
}

// ============================================================================
CEventView::~CEventView()
{
}

// ============================================================================
void CEventView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

// ============================================================================
#ifdef _DEBUG
void CEventView::AssertValid() const
{
	CListView::AssertValid();
}
void CEventView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

// ============================================================================
void CEventView::SetCurObj(CGuiObj *pCurObj)
{
	GetListCtrl().DeleteAllItems();
	m_pCurObj = pCurObj;
	if(!pCurObj) return;

	for(int i = 0; i < m_pCurObj->m_handlers.GetSize(); i++)
	{
		GetListCtrl().InsertItem(i, m_pCurObj->m_handlers[i].m_name);
		GetListCtrl().SetCheck(i, m_pCurObj->m_handlers[i].m_use);
	}
}

// ============================================================================
void CEventView::UpdateEvent(int idx)
{
	ASSERT(idx >= 0 && idx < m_pCurObj->m_handlers.GetSize());
	GetListCtrl().SetItemText(idx, 0, m_pCurObj->m_handlers[idx].m_name);
	GetListCtrl().SetCheck(idx, m_pCurObj->m_handlers[idx].m_use);
}

// ============================================================================
int CEventView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	lpCreateStruct->style = WS_VISIBLE | WS_CHILD | WS_VSCROLL |
		LVS_REPORT | LVS_SINGLESEL | LVS_NOSORTHEADER | LVS_SHOWSELALWAYS;
	lpCreateStruct->dwExStyle = WS_EX_CLIENTEDGE;

	if(CListView::OnCreate(lpCreateStruct) == -1)
		return -1;

	GetListCtrl().SetExtendedStyle(
		LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES |
		LVS_EX_ONECLICKACTIVATE | LVS_EX_UNDERLINEHOT);
	GetListCtrl().InsertColumn(0, GetString(IDS_EVENT_HANDLERS), LVCFMT_LEFT, 160, -1);
	SetFont(GetParent()->GetFont());

	// Setup MAX custom colors
	if( ColorMan()->GetColorSchemeType() == IColorManager::CST_CUSTOM )
	{
		GetListCtrl().SetBkColor(GetCustSysColor(COLOR_WINDOW));
		GetListCtrl().SetTextBkColor(GetCustSysColor(COLOR_WINDOW));
		GetListCtrl().SetTextColor(GetCustSysColor(COLOR_WINDOWTEXT));
	}

	return 0;
}

// ============================================================================
void CEventView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	switch(lHint)
	{
	case HINT_CHANGESEL:
		SetCurObj((CGuiObj*)pHint);
		break;
	case HINT_CHANGEEVENT:
		UpdateEvent(static_cast<int>(reinterpret_cast<ULONG_PTR>(pHint)));
		break;
	case HINT_CHANGEPROP:
		break;
	default:
		SetCurObj(m_pCurObj);
		break;
	}
}

// ============================================================================
void CEventView::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LPNMLISTVIEW pNMListView = (LPNMLISTVIEW)pNMHDR;
	*pResult = 0;

	if(pNMListView->uOldState == 0 && pNMListView->uNewState == 0)
		return;	// No change

	// get old state of the checkbox
	BOOL bPrevState = (BOOL)(((pNMListView->uOldState & LVIS_STATEIMAGEMASK)>>12)-1);
	if(bPrevState < 0) bPrevState = 0;

	// get the new state of the checkbox
	BOOL bChecked = (BOOL)(((pNMListView->uNewState & LVIS_STATEIMAGEMASK)>>12)-1);   
	if(bChecked < 0) bChecked = 0; 

	// Checkbox changed
	if(bPrevState != bChecked)
	{
		m_pCurObj->m_handlers[pNMListView->iItem].m_use = (bool)bChecked;
	}
}

// ============================================================================
void CEventView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CListView::OnLButtonDown(nFlags, point);

	int idx = GetListCtrl().HitTest(point);
	if(LB_ERR == idx) return;

	UINT flag = LVIS_FOCUSED;
	if((GetListCtrl().GetItemState(idx, flag) & flag) == flag)
	{
		GetListCtrl().SetCheck(idx, TRUE);

		CEventEd ed(CVisualMSDoc::GetDoc(), idx);
		ed.DoModal();
	}
	GetListCtrl().SetItemState(idx, 0, LVIS_SELECTED|LVIS_FOCUSED);
}

#pragma warning(pop)

