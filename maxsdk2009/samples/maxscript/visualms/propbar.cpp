// ============================================================================
// PropBar.cpp
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#include "stdafx.h"
#include "VisualMS.h"
#include "PropBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ============================================================================
IMPLEMENT_DYNAMIC(CPropBar, CSizeBar)
BEGIN_MESSAGE_MAP(CPropBar, CSizeBar)
	//{{AFX_MSG_MAP(CPropBar)
	ON_WM_CREATE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_PROPTAB, OnSelChangePropTab)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// ============================================================================
void CPropBar::DoDataExchange(CDataExchange* pDX)
{
	CSizeBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropBar)
	DDX_Control(pDX, IDC_PROPTAB, m_tabCtrl);
	//}}AFX_DATA_MAP
}

// ============================================================================
CPropBar::CPropBar()
{
	//{{AFX_DATA_INIT(CPropBar)
	//}}AFX_DATA_INIT
}

// ============================================================================
BOOL CPropBar::OnInitDialog()
{
	CSizeBar::OnInitDialog();
	ModifyStyle(0, WS_CLIPCHILDREN);
	return TRUE;
}

// ============================================================================
int CPropBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if(CSizeBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rect, itemRect;
	GetClientRect(rect);
	rect.left += 4;
	rect.top += 100;
	rect.right -= rect.left + 4;
	rect.bottom -= rect.top + 4;

	// TODO: why are the tabs upside down
	m_tabCtrl.Create(
		WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | 
		TCS_HOTTRACK | TCS_FOCUSNEVER,/*TCS_BOTTOM| */
		rect, this, IDC_PROPTAB);
	m_tabCtrl.MoveWindow(rect.left, rect.top, rect.right, rect.bottom, TRUE);
	m_tabCtrl.SetFont(GetBigFont(), TRUE);

	m_pActiveView = NULL;

	return 0;
}

// ============================================================================
BOOL CPropBar::AddView(LPCTSTR lpszLabel, CRuntimeClass *pViewClass,
							  CDocument *pDoc, CCreateContext *pContext)
{	
	CCreateContext context;
	if(pContext == NULL)
		pContext = &context;

	CWnd *pWnd;
	pWnd = (CWnd*)pViewClass->CreateObject();
	if(pWnd == NULL)
		AfxThrowMemoryException();

	CRect rect;
	DWORD dwStyle = AFX_WS_DEFAULT_VIEW;
	if(!pWnd->Create(NULL, NULL, dwStyle, rect, &m_tabCtrl, 0, pContext))
	{
		TRACE0("Warning: couldn't create client pane for view.\n");
		return FALSE;
	}

	pWnd->EnableWindow(FALSE);
	pWnd->ShowWindow(SW_HIDE);
	pDoc->AddView((CView*)pWnd);

	int viewIdx = static_cast<int>(m_views.Add(pWnd));	// SR DCAST64: Downcast to 2G limit.
	m_tabCtrl.InsertItem(viewIdx, lpszLabel);

	return TRUE;
}

// ============================================================================
void CPropBar::SelectTab(int nSel)
{
	if(m_pActiveView)
	{
		m_pActiveView->EnableWindow(FALSE);
		m_pActiveView->ShowWindow(SW_HIDE);
	}

	m_pActiveView = (CView*)m_views[nSel];
	m_pActiveView->EnableWindow(TRUE);
	m_pActiveView->ShowWindow(SW_SHOW);
	m_pActiveView->SetFocus();

	((CFrameWnd*)GetParent())->SetActiveView((CView*)m_pActiveView);
}

// ============================================================================
void CPropBar::OnSelChangePropTab(NMHDR* pNMHDR, LRESULT* pResult) 
{
	SelectTab(m_tabCtrl.GetCurSel());
	*pResult = 0;
}

// ============================================================================
void CPropBar::OnSize(UINT nType, int cx, int cy) 
{
	CSizeBar::OnSize(nType, cx, cy);

	if(!IsFloating())
		cy -= 5;
	m_tabCtrl.MoveWindow(5, 5, cx-10, cy-10);

	CRect rect;
	m_tabCtrl.GetClientRect(rect);
	rect.top = 28;

	CWnd *pWnd;
	for(int i = 0; i < m_views.GetSize(); i++)
	{
		pWnd = (CWnd*)(m_views[i]);
		pWnd->MoveWindow(rect, TRUE);
	}
}

