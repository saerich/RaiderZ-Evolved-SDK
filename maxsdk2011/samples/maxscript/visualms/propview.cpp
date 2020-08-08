// ============================================================================
// PropView.cpp
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#include "stdafx.h"
#include "VisualMS.h"
#include "PropView.h"
#include "ArrayEdit.h"
#include "resource.h"

// from the maxsdk
#include "Max.h"
#include "CustCont.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ============================================================================
class CPropEdit : public CEdit
{
protected:
	CProperty *m_prop;
	int  m_iItem;
	int  m_iSubItem;
	CString m_text;
	BOOL m_bESC;
	CSpinButtonCtrl m_spinner;
	HWND m_hSpinner;

public:
	CPropEdit(int iItem, int iSubItem, CString text, CProperty *pProp);
	virtual ~CPropEdit();

public:
	//{{AFX_VIRTUAL(CPropEdit)
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CPropEdit)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnNcDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

// ============================================================================
class CPropDropDown : public CComboBox
{
protected:
	CProperty *m_prop;
	int 	m_iItem;
	int 	m_iSubItem;
	CStringList m_lstItems;
	int 	m_nSel;
	BOOL	m_bESC;				// To indicate whether ESC key was pressed

public:
	CPropDropDown(int iItem, int iSubItem, CProperty *pProp);
	virtual ~CPropDropDown();

	//{{AFX_VIRTUAL(CPropDropDown)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL


protected:
	//{{AFX_MSG(CPropDropDown)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnNcDestroy();
	afx_msg void OnCloseup();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};


// ============================================================================
// CPropEdit
// ----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CPropEdit, CEdit)
	//{{AFX_MSG_MAP(CPropEdit)
	ON_WM_CREATE()
	ON_WM_CHAR()
	ON_WM_KILLFOCUS()
	ON_WM_NCDESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ============================================================================
CPropEdit::CPropEdit(int iItem, int iSubItem, CString text, CProperty *pProp)
{
	m_prop = pProp;      // for number limits..
	m_iItem = iItem;
	m_iSubItem = iSubItem;
	m_bESC = FALSE;
	m_text = text;
}
CPropEdit::~CPropEdit() {}

// ============================================================================
BOOL CPropEdit::PreTranslateMessage(MSG* pMsg) 
{
	if(pMsg->message == WM_KEYDOWN)
	{
		if(pMsg->wParam == VK_RETURN ||
		   pMsg->wParam == VK_DELETE ||
		   pMsg->wParam == VK_ESCAPE ||
		   GetKeyState(VK_CONTROL))
		{
			::TranslateMessage(pMsg);
			::DispatchMessage(pMsg);
			return TRUE;		    	// DO NOT process further
		}
	}
	return CEdit::PreTranslateMessage(pMsg);
}

// ============================================================================
int CPropEdit::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if(CEdit::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetFont(GetParent()->GetFont());
	SetWindowText(m_text);
	SetFocus();
	SetSel(0, -1);

	DisableAccelerators();

	if(m_prop->m_type == PROP_INTEGER)
	{
		CRect rect;
		GetWindowRect(rect);
		rect.left += (rect.right-rect.left)-20;
//		m_hSpinner = CreateWindow(
//			SPINNERWINDOWCLASS,
//			_T("Spinner"),

//		m_pSpinner = new ISpinnerControl();
	}

/*	if(m_prop->m_type == PROP_INTEGER)
	{
		CRect rect;
		GetWindowRect(rect);
		rect.left += (rect.right-rect.left)-20;
		m_spinner.Create(UDS_ARROWKEYS | UDS_SETBUDDYINT | UDS_ALIGNRIGHT,
			rect, GetParent(), ID_SPINNER);
		m_spinner.SetRange(UD_MINVAL, UD_MAXVAL);
		m_spinner.SetPos(m_prop->m_integer);
		m_spinner.SetBuddy(this);
		m_spinner.ShowWindow(SW_SHOW);
		m_spinner.UpdateWindow();
	}
*/
	return 0;
}

// ============================================================================
void CPropEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// This was not working in the plug-in version ? so I'm handling it in OnKeyUp
	if(nChar == VK_ESCAPE || nChar == VK_RETURN)
	{
		if(nChar == VK_ESCAPE)
			m_bESC = TRUE;
		GetParent()->SetFocus();
		return;
	}

	CEdit::OnChar(nChar, nRepCnt, nFlags);

	// Resize edit control if needed

	// Get text extent
	CString str;

	GetWindowText(str);
	CWindowDC dc(this);
	CFont *pFont = GetParent()->GetFont();
	CFont *pFontDC = dc.SelectObject(pFont);
	CSize size = dc.GetTextExtent(str);
	dc.SelectObject(pFontDC);
	size.cx += 5;			   	// add some extra buffer

	// Get client rect
	CRect rect, parentrect;
	GetClientRect(&rect);
	GetParent()->GetClientRect(&parentrect);

	// Transform rect to parent coordinates
	ClientToScreen(&rect);
	GetParent()->ScreenToClient(&rect);

	// Check whether control needs to be resized
	// and whether there is space to grow
	if(size.cx > rect.Width())
	{
		if(size.cx + rect.left < parentrect.right)
			rect.right = rect.left + size.cx;
		else
			rect.right = parentrect.right;
		MoveWindow(&rect);
	}
}

// ============================================================================
void CPropEdit::OnKillFocus(CWnd* pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);

	if(pNewWnd && pNewWnd->GetParent() == &m_spinner)
		return;

	CString str;
	GetWindowText(str);

	// Send Notification to parent of ListView ctrl
	LV_DISPINFO dispinfo;
	dispinfo.hdr.hwndFrom = GetParent()->m_hWnd;
	dispinfo.hdr.idFrom = GetDlgCtrlID();
	dispinfo.hdr.code = LVN_ENDLABELEDIT;

	dispinfo.item.mask = LVIF_TEXT;
	dispinfo.item.iItem = m_iItem;
	dispinfo.item.iSubItem = m_iSubItem;
	dispinfo.item.pszText = m_bESC ? NULL : LPTSTR((LPCTSTR)str);
	dispinfo.item.cchTextMax = m_bESC ? -1 : str.GetLength();

	GetParent()->GetParent()->SendMessage(WM_NOTIFY,
		GetParent()->GetDlgCtrlID(), (LPARAM)&dispinfo);

	DestroyWindow();
	EnableAccelerators();
}

// ============================================================================
void CPropEdit::OnNcDestroy()
{
	CEdit::OnNcDestroy();
	delete this;
}

// ============================================================================
// CPropDropDown
// ----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CPropDropDown, CComboBox)
	//{{AFX_MSG_MAP(CPropDropDown)
	ON_WM_CREATE()
	ON_WM_KILLFOCUS()
	ON_WM_NCDESTROY()
	ON_CONTROL_REFLECT(CBN_CLOSEUP, OnCloseup)
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ============================================================================
CPropDropDown::CPropDropDown(int iItem, int iSubItem, CProperty *pProp)
{
	m_iItem = iItem;
	m_iSubItem = iSubItem;
	m_prop = pProp;
	m_bESC = FALSE;
}

// ============================================================================
CPropDropDown::~CPropDropDown() {}

// ============================================================================
int CPropDropDown::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CComboBox::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetFont(GetStdFont());
	SetFocus();
	DisableAccelerators();

	if(m_prop->m_type == PROP_BOOLEAN)
	{
		AddString(_T("true"));
		AddString(_T("false"));

		if(m_prop->m_boolean)
			SetCurSel(0);
		else
			SetCurSel(1);
	}
	else if(m_prop->m_type == PROP_OPTION)
	{
		for(int i = 0; i < m_prop->m_array.GetSize(); i++)
			AddString(m_prop->m_array[i]);
		SetCurSel(m_prop->m_integer);
	}

	return 0;
}

// ============================================================================
BOOL CPropDropDown::PreTranslateMessage(MSG* pMsg) 
{
	if(pMsg->message == WM_KEYDOWN)
	{
		if(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
		{
			::TranslateMessage(pMsg);
			::DispatchMessage(pMsg);
			return TRUE;				// DO NOT process further
		}
	}
	
	return CComboBox::PreTranslateMessage(pMsg);
}

// ============================================================================
void CPropDropDown::OnKillFocus(CWnd* pNewWnd) 
{
	CComboBox::OnKillFocus(pNewWnd);
	
	CString str;
	GetWindowText(str);

	// Send Notification to parent of ListView ctrl
	LV_DISPINFO dispinfo;
	dispinfo.hdr.hwndFrom = GetParent()->m_hWnd;
	dispinfo.hdr.idFrom = GetDlgCtrlID();
	dispinfo.hdr.code = LVN_ENDLABELEDIT;

	dispinfo.item.mask = LVIF_TEXT;
	dispinfo.item.iItem = m_iItem;
	dispinfo.item.iSubItem = m_iSubItem;
	if(m_bESC)
	{
		dispinfo.item.pszText = NULL;
		dispinfo.item.cchTextMax = -1;
	}
	else if(m_prop->m_type == PROP_OPTION)
	{
		dispinfo.item.pszText = NULL;
		dispinfo.item.cchTextMax = GetCurSel();
	}
	else
	{
		dispinfo.item.pszText = LPTSTR((LPCTSTR)str);
		dispinfo.item.cchTextMax = str.GetLength();
	}

	GetParent()->GetParent()->SendMessage(WM_NOTIFY,
		GetParent()->GetDlgCtrlID(), (LPARAM)&dispinfo);

	EnableAccelerators();
	PostMessage(WM_CLOSE);
}

// ============================================================================
void CPropDropDown::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(nChar == VK_ESCAPE || nChar == VK_RETURN)
	{
		if(nChar == VK_ESCAPE)
			m_bESC = TRUE;
		GetParent()->SetFocus();
		return;
	}

	CComboBox::OnChar(nChar, nRepCnt, nFlags);
}

// ============================================================================
void CPropDropDown::OnNcDestroy() 
{
	CComboBox::OnNcDestroy();
	delete this;
}

// ============================================================================
void CPropDropDown::OnCloseup() 
{
	GetParent()->SetFocus();
}

// ============================================================================
// CPropView
// ----------------------------------------------------------------------------
IMPLEMENT_DYNCREATE(CPropView, CListView)
BEGIN_MESSAGE_MAP(CPropView, CListView)
	//{{AFX_MSG_MAP(CPropView)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemChanged)
	ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, OnEndLabelEdit)
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ============================================================================
CPropView::CPropView()
{
	m_pCurObj = NULL;
}
CPropView::~CPropView() {}

// ============================================================================
#ifdef _DEBUG
void CPropView::AssertValid() const { CListView::AssertValid(); }
void CPropView::Dump(CDumpContext& dc) const { CListView::Dump(dc); }
#endif //_DEBUG


// ============================================================================
int CPropView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	lpCreateStruct->style = WS_VISIBLE | WS_CHILD | WS_VSCROLL |
		LVS_REPORT | LVS_SINGLESEL | LVS_NOSORTHEADER | LVS_SHOWSELALWAYS;
	lpCreateStruct->dwExStyle = WS_EX_CLIENTEDGE;

	if(CListView::OnCreate(lpCreateStruct) == -1)
		return -1;

	GetListCtrl().SetExtendedStyle(
		LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES |
		LVS_EX_ONECLICKACTIVATE | LVS_EX_UNDERLINEHOT);
	GetListCtrl().InsertColumn(0, GetString(IDS_PROPERTY), LVCFMT_LEFT, 80, -1);
	GetListCtrl().InsertColumn(1, GetString(IDS_VALUE), LVCFMT_LEFT, 80, -1);

	// Setup MAX custom colors
	if( ColorMan()->GetColorSchemeType() == IColorManager::CST_CUSTOM )
	{
		GetListCtrl().SetBkColor(GetCustSysColor(COLOR_WINDOW));
		GetListCtrl().SetTextBkColor(GetCustSysColor(COLOR_WINDOW));
		GetListCtrl().SetTextColor(GetCustSysColor(COLOR_WINDOWTEXT));
	}


	SetFont(GetParent()->GetFont());
	return 0;
}

// ============================================================================
void CPropView::SetCurObj(CGuiObj *pCurObj)
{
	GetListCtrl().DeleteAllItems();
	m_pCurObj = pCurObj;
	if(!pCurObj) return;

	for(int i = 0; i < m_pCurObj->m_properties.GetSize(); i++)
	{
		GetListCtrl().InsertItem(i, m_pCurObj->m_properties[i].m_name);
		GetListCtrl().SetItemText(i, 1, m_pCurObj->GetPropStr(i));
/*		DebugPrintf("%s  :  %s\n",
			(LPCTSTR)(m_pCurObj->m_properties[i].m_name),
			(LPCTSTR)(m_pCurObj->m_properties[i]));
*/	}
}

// ============================================================================
void CPropView::UpdateProperty(int idx)
{
	if(m_pCurObj == NULL) return; // safety
	ASSERT(idx >= 0 && idx < m_pCurObj->m_properties.GetSize());
	GetListCtrl().SetItemText(idx, 0, m_pCurObj->m_properties[idx].m_name);
	GetListCtrl().SetItemText(idx, 1, m_pCurObj->GetPropStr(idx));
}

// ============================================================================
CEdit* CPropView::EditSubLabel(int nItem, int nCol)
{
	// The returned pointer should not be saved

	// Make sure that the item is visible
	if(!GetListCtrl().EnsureVisible(nItem, TRUE)) return NULL;

	// Make sure that nCol is valid
	CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
	int nColumnCount = pHeader->GetItemCount();
	if(nCol >= nColumnCount || GetListCtrl().GetColumnWidth(nCol) < 5)
		return NULL;

	// Get the column offset
	int offset = 0;
	for(int i = 0; i < nCol; i++)
		offset += GetListCtrl().GetColumnWidth(i);

	CRect rect;
	GetListCtrl().GetItemRect(nItem, &rect, LVIR_BOUNDS);

	// Now scroll if we need to expose the column
	CRect rcClient;
	GetClientRect(&rcClient);
	if(offset + rect.left < 0 || offset + rect.left > rcClient.right)
	{
		CSize size;
		size.cx = offset + rect.left;
		size.cy = 0;
		GetListCtrl().Scroll(size);
		rect.left -= size.cx;
	}

	// Get Column alignment
	LV_COLUMN lvcol;
	lvcol.mask = LVCF_FMT;
	GetListCtrl().GetColumn(nCol, &lvcol);
	DWORD dwStyle ;
	if((lvcol.fmt&LVCFMT_JUSTIFYMASK) == LVCFMT_LEFT)
		dwStyle = ES_LEFT;
	else if((lvcol.fmt&LVCFMT_JUSTIFYMASK) == LVCFMT_RIGHT)
		dwStyle = ES_RIGHT;
	else dwStyle = ES_CENTER;

	rect.left += offset + 6;
	rect.right = rect.left + GetListCtrl().GetColumnWidth(nCol) - 6;
	if(rect.right > rcClient.right) rect.right = rcClient.right;
	rect.bottom -= 1;

	CProperty *prop = &m_pCurObj->m_properties[nItem];
	if(prop->m_type == PROP_INTEGER || prop->m_type == PROP_REAL)
		dwStyle |= ES_NUMBER;

	dwStyle |= WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL;
	CEdit *pEdit = new CPropEdit(nItem, nCol, m_pCurObj->GetPropStr(nItem), prop);
	pEdit->Create(dwStyle, rect, this, ID_PROPEDIT);

	return pEdit;
}

// ============================================================================
CComboBox* CPropView::DropDownLabel(int nItem, int nCol)
{
	if(!GetListCtrl().EnsureVisible(nItem, TRUE))
		return NULL;

	// Make sure that nCol is valid 
	CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
	int nColumnCount = pHeader->GetItemCount();
	if(nCol >= nColumnCount || GetListCtrl().GetColumnWidth(nCol) < 10)
		return NULL;

	// Get the column offset
	int offset = 0;
	for(int i = 0; i < nCol; i++)
		offset += GetListCtrl().GetColumnWidth(i);

	CRect rect;
	GetListCtrl().GetItemRect(nItem, &rect, LVIR_BOUNDS);

	// Now scroll if we need to expose the column
	CRect rcClient;
	GetClientRect(&rcClient);
	if(offset + rect.left < 0 || offset + rect.left > rcClient.right)
	{
		CSize size;
		size.cx = offset + rect.left;
		size.cy = 0;
		GetListCtrl().Scroll( size );
		rect.left -= size.cx;
	}

	rect.left += offset;
	rect.right = rect.left + GetListCtrl().GetColumnWidth(nCol) + 1;
	rect.top -= 1;
	rect.bottom -= 5;

	int height = rect.bottom-rect.top;
	rect.bottom += 5*height;

	if(rect.right > rcClient.right)
		rect.right = rcClient.right;

	DWORD dwStyle = WS_CHILD|WS_VISIBLE|//WS_VSCROLL|
					CBS_DROPDOWNLIST|CBS_DISABLENOSCROLL;
	CComboBox *pList = new CPropDropDown(nItem, nCol, &m_pCurObj->m_properties[nItem]);
	pList->Create(dwStyle, rect, this, ID_PROPEDIT);
	pList->SetItemHeight(-1, height);
	pList->SetHorizontalExtent(GetListCtrl().GetColumnWidth(nCol));

	return pList;
}

// ============================================================================
void CPropView::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO *pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM	*pItem = &pDispInfo->item;

	if(pItem->pszText != NULL)
	{
		GetListCtrl().SetItemText(pItem->iItem, pItem->iSubItem, pItem->pszText);
		m_pCurObj->SetPropStr(pItem->iItem, pItem->pszText);
		m_pCurObj->PropChanged(pItem->iItem);
	}
	else if(pItem->pszText == NULL && pItem->cchTextMax != -1)
	{
		GetListCtrl().SetItemText(pItem->iItem, pItem->iSubItem, m_pCurObj->GetPropStr(pItem->iItem));
		m_pCurObj->SetPropInt(pItem->iItem, pItem->cchTextMax);
		m_pCurObj->PropChanged(pItem->iItem);
	}

	*pResult = FALSE;
}

// ============================================================================
void CPropView::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	*pResult = 0;

	if(!(pNMListView->uOldState & LVIS_SELECTED) && (pNMListView->uNewState & LVIS_SELECTED))
	{
	/*	EditSubLabel(pNMListView->iItem, 1);
	
		CRect rect;
		GetSubItemRect(1, pNMListView->iItem, LVIR_BOUNDS, rect);
		CEdit *edit = new CEdit();
		edit->Create(WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL, rect, this, ID_PROPEDIT);
		edit->ModifyStyleEx(0, WS_EX_CLIENTEDGE);
		edit->SetWindowText(CListCtrl::GetItemText(pNMListView->iItem, 1));
		edit->ShowWindow(SW_SHOW);
		edit->SetFocus();
	*/
	}
}

// ============================================================================
void CPropView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CListView::OnLButtonDown(nFlags, point);

	int idx = GetListCtrl().HitTest(point);
	if(LB_ERR == idx) return;

	UINT flag = LVIS_FOCUSED;
	if((GetListCtrl().GetItemState(idx, flag) & flag) == flag)
	{
		CProperty *prop = &m_pCurObj->m_properties[idx];
		if(!prop->IsReadOnly())
		{
			switch(prop->m_type)
			{
			case PROP_STRING:
			case PROP_INTEGER:
			case PROP_REAL:
			case PROP_NAME: 
			case PROP_LITERAL:
			case PROP_POINT2:
			case PROP_POINT3:
				EditSubLabel(idx, 1);
				break;
			case PROP_BOOLEAN:
			case PROP_OPTION:
				DropDownLabel(idx, 1);
				break;
			case PROP_ARRAY:
				{
					CArrayEdit ae(prop->m_array);
					if(ae.DoModal() == IDOK)
					{
						m_pCurObj->m_properties[idx] = ae.m_stringArray;
						UpdateProperty(idx);
						m_pCurObj->PropChanged(idx);
					}
				}
				break;
			}
		}
	}
	else
		GetListCtrl().SetItemState(idx, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}

// ============================================================================
void CPropView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
	if(GetFocus() != this) SetFocus();
	CListView::OnVScroll(nSBCode, nPos, pScrollBar);
}

// ============================================================================
void CPropView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
	if(GetFocus() != this) SetFocus();
	CListView::OnHScroll(nSBCode, nPos, pScrollBar);
}

// ============================================================================
void CPropView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	switch(lHint)
	{
	case HINT_CHANGEPROP:
		UpdateProperty(static_cast<int>(reinterpret_cast<ULONG_PTR>(pHint)));
		break;
	case HINT_CHANGESEL:
		SetCurObj((CGuiObj*)pHint);
		break;
	default:
		SetCurObj(m_pCurObj);
		break;
	}
}

// ============================================================================
void CPropView::OnSize(UINT nType, int cx, int cy) 
{
	CListView::OnSize(nType, cx, cy);
	
	int w = cx - GetListCtrl().GetColumnWidth(0);
	GetListCtrl().SetColumnWidth(1, w-10);
}
