// ============================================================================
// StdObj.cpp
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#include "stdafx.h"
#include "VisualMS.h"
#include "VisualMSThread.h"
#include "VisualMSDoc.h"
#include "StdObj.h"
#include "resource.h"

#define RADIO_DOT_WIDTH   23
#define SPACING_BEFORE    5

// ============================================================================
CStdObj::CStdObj(CWnd *pParent, const CRect &rect)
	: CGuiObj(pParent)
{
	m_sizer.m_rect = rect;
	m_sizer.m_rect.right += rect.left;
	m_sizer.m_rect.bottom += rect.top;

	m_properties[IDX_XPOS] = rect.left;
	m_properties[IDX_YPOS] = rect.top;
	m_properties[IDX_WIDTH] = rect.right;
	m_properties[IDX_HEIGHT] = rect.bottom;
	m_properties[IDX_CLASS].m_flags |= FLAG_READONLY;

	m_selected = TRUE;
}

// ============================================================================
CStdObj::~CStdObj()
{
}

// ============================================================================
void CStdObj::MoveToTop(const CWnd* pWndInsertAfter)
{
	// TODO: Fix this, it's never worked ?
//	m_wnd.SetWindowPos(pWndInsertAfter, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_SHOWWINDOW);
//	m_wnd.ShowWindow(SW_SHOWNA);
}

// ============================================================================
void CStdObj::GetRect(CRect &rect) const
{
	rect.left = (int)m_properties[IDX_XPOS];
	rect.top = (int)m_properties[IDX_YPOS];
	rect.right = (int)m_properties[IDX_WIDTH];
	rect.bottom = (int)m_properties[IDX_HEIGHT];
}

// ============================================================================
void CStdObj::SetRect(const CRect &rect)
{
	CRect r;
	r = rect;
	r.right += r.left;
	r.bottom += r.top;
	m_wnd.MoveWindow(r, TRUE);

	m_wnd.GetWindowRect(r);
	m_wnd.GetParent()->ScreenToClient(r);
	m_sizer.m_rect = r;
	r.right -= r.left;
	r.bottom -= r.top;

	m_properties[IDX_XPOS] = r.left;
	m_properties[IDX_YPOS] = r.top;
	m_properties[IDX_WIDTH] = r.right;
	m_properties[IDX_HEIGHT] = r.bottom;

	InvalidateProp(IDX_XPOS);
	InvalidateProp(IDX_YPOS);
	InvalidateProp(IDX_WIDTH);
	InvalidateProp(IDX_HEIGHT);
}

// ============================================================================
int CStdObj::OnPropChange(int idx)
{
	CRect rect;
	switch(idx)
	{
		case IDX_CAPTION:
			m_wnd.SetWindowText(Caption());
			m_wnd.InvalidateRect(NULL, TRUE);
			return 1;
		case IDX_XPOS:
		case IDX_YPOS:
		case IDX_WIDTH:
		case IDX_HEIGHT:
			GetRect(rect);
			SetRect(rect);
			return 1;
	}
	return 0;
}

// ============================================================================
void CStdObj::Emit(CString &out)
{
	CGuiObj::Emit(out);
}


// ============================================================================
// CBitmapObj
// ----------------------------------------------------------------------------
int CBitmapObj::m_count = 0;

// ============================================================================
CBitmapObj::CBitmapObj(CWnd *pParent, const CRect &rect)
	: CStdObj(pParent, (rect.right&&rect.bottom ? rect : CRect(rect.left,rect.top,80,30)))
{
	Class() = _T("bitmap");
	Name().Format(_T("bmp%d"), ++m_count);
	Caption() = _T("Bitmap");

	idx_fileName = m_properties.Add(CProperty(_T("fileName"), CString("")));
	idx_bitmap   = m_properties.Add(CProperty(_T("bitmap"), CString("")));
	m_properties[idx_bitmap].m_type = PROP_LITERAL;

	m_handlers.RemoveAll();

	m_wnd.Create(
		_T("STATIC"),
		NULL,
		WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_SUNKEN,
		rect, pParent, -1);
	m_wnd.SendMessage(STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP)));
}


// ============================================================================
// CButtonObj
// ----------------------------------------------------------------------------
int CButtonObj::m_count = 0;

// ============================================================================
CButtonObj::CButtonObj(CWnd *pParent, const CRect &rect)
	: CStdObj(pParent, (rect.right&&rect.bottom ? rect : CRect(rect.left,rect.top,80,30)))
{
	Class() = _T("button");
	Name().Format(_T("btn%d"), ++m_count);
	Caption() = _T("Button");

	idx_images = m_properties.Add(CProperty(_T("images"), CStringArray()));
	idx_toolTip = m_properties.Add(CProperty(_T("toolTip"), CString("")));

	m_handlers.RemoveAll();
	m_handlers.Add(CHandler(false, _T("pressed"), _T(" do"), _T("")));

	m_wnd.Create(
		_T("BUTTON"),//_T("CustButton"),
		Caption(),
		WS_CHILD | WS_VISIBLE,
		rect, pParent, -1);
	m_wnd.SetFont(GetStdFont(), FALSE);
}


// ============================================================================
// CMapButtonObj
// ----------------------------------------------------------------------------
CMapButtonObj::CMapButtonObj(CWnd *pParent, const CRect &rect)
	: CStdObj(pParent, (rect.right&&rect.bottom ? rect : CRect(rect.left,rect.top,80,30)))
{
	Class() = _T("mapButton");
	Name().Format(_T("btn%d"), ++CButtonObj::m_count);
	Caption() = _T("MapButton");

	idx_map    = m_properties.Add(CProperty(_T("map"), CString("")));
	idx_images = m_properties.Add(CProperty(_T("images"), CStringArray()));
	idx_toolTip = m_properties.Add(CProperty(_T("toolTip"), CString("")));

	m_properties[idx_map].m_type = PROP_LITERAL;

	m_handlers.RemoveAll();
	m_handlers.Add(CHandler(false, _T("picked"), _T("texmap do"), _T("")));

	m_wnd.Create(
		_T("BUTTON"),//_T("CustButton"),
		Caption(),
		WS_CHILD | WS_VISIBLE,
		rect, pParent, -1);
	m_wnd.SetFont(GetStdFont(), FALSE);
}


// ============================================================================
// CMaterialButtonObj
// ----------------------------------------------------------------------------
CMaterialButtonObj::CMaterialButtonObj(CWnd *pParent, const CRect &rect)
	: CStdObj(pParent, (rect.right&&rect.bottom ? rect : CRect(rect.left,rect.top,80,30)))
{
	Class() = _T("materialButton");
	Name().Format(_T("btn%d"), ++CButtonObj::m_count);
	Caption() = _T("MaterialButton");

	idx_material = m_properties.Add(CProperty(_T("material"), CString("")));
	idx_images = m_properties.Add(CProperty(_T("images"), CStringArray()));
	idx_toolTip = m_properties.Add(CProperty(_T("toolTip"), CString("")));

	m_properties[idx_material].m_type = PROP_LITERAL;

	m_handlers.RemoveAll();
	m_handlers.Add(CHandler(false, _T("picked"), _T("mtl do"), _T("")));

	m_wnd.Create(
		_T("BUTTON"),//_T("CustButton"),
		Caption(),
		WS_CHILD | WS_VISIBLE,
		rect, pParent, -1);
	m_wnd.SetFont(GetStdFont(), FALSE);
}


// ============================================================================
// CPickButtonObj
// ----------------------------------------------------------------------------
CPickButtonObj::CPickButtonObj(CWnd *pParent, const CRect &rect)
	: CStdObj(pParent, (rect.right&&rect.bottom ? rect : CRect(rect.left,rect.top,80,30)))
{
	Class() = _T("pickButton");
	Name().Format(_T("btn%d"), ++CButtonObj::m_count);
	Caption() = _T("PickButton");

	idx_message = m_properties.Add(CProperty(_T("message"), CString("")));
	idx_filter  = m_properties.Add(CProperty(_T("filter"), CString("")));
	idx_toolTip = m_properties.Add(CProperty(_T("toolTip"), CString("")));

	m_properties[idx_filter].m_type = PROP_LITERAL;

	m_handlers.RemoveAll();
	m_handlers.Add(CHandler(false, _T("picked"), _T("obj do"), _T("")));

	m_wnd.Create(
		_T("BUTTON"),//_T("CustButton"),
		Caption(),
		WS_CHILD | WS_VISIBLE,
		rect, pParent, -1);
	m_wnd.SetFont(GetStdFont(), FALSE);
}


// ============================================================================
// CCheckButtonObj
// ----------------------------------------------------------------------------
int CCheckButtonObj::m_count = 0;

// ============================================================================
CCheckButtonObj::CCheckButtonObj(CWnd *pParent, const CRect &rect)
	: CStdObj(pParent, rect)
{
	Class() = _T("checkButton");
	Name().Format(_T("ckb%d"), ++m_count);
	Caption() = _T("CheckButton");

	idx_highlightColor = m_properties.Add(CProperty(_T("highlightColor"), CString("")));
	idx_toolTip = m_properties.Add(CProperty(_T("toolTip"), CString("")));
	idx_checked = m_properties.Add(CProperty(_T("checked"), false));
	idx_images = m_properties.Add(CProperty(_T("images"), CStringArray()));

	m_properties[idx_highlightColor].m_type = PROP_LITERAL;

	m_handlers.RemoveAll();
	m_handlers.Add(CHandler(false, _T("changed"), _T("state do"), _T("")));

	m_wnd.Create(
		_T("BUTTON"),
		Caption(),
		WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_PUSHLIKE,
		m_sizer.m_rect, pParent, -1);
	m_wnd.SetFont(GetStdFont(), FALSE);
}

// ============================================================================
int CCheckButtonObj::OnPropChange(int idx)
{
	if(CStdObj::OnPropChange(idx))
		return 1;

	if(idx == idx_checked)
	{
		m_wnd.SendMessage(BM_SETCHECK, (int)(m_properties[idx]), 0);
		return 1;
	}

	return 0;
}

// ============================================================================
void CCheckButtonObj::Emit(CString &out)
{
	CStdObj::Emit(out);
//	CString str = _T("");
//	str.Format(" checked:%s", (LPCTSTR)(m_properties[idx_checked]));
//	out += str;
}


// ============================================================================
// CColorPickerObj
// ----------------------------------------------------------------------------
int CColorPickerObj::m_count = 0;

// ============================================================================
CColorPickerObj::CColorPickerObj(CWnd *pParent, const CRect &rect)
	: CStdObj(pParent, (rect.right&&rect.bottom ? rect : CRect(rect.left,rect.top,40,16)))
{
	Class() = _T("colorPicker");
	Name().Format(_T("cp%d"), ++m_count);
	Caption() = _T("ColorPicker");

	idx_color = m_properties.Add(CProperty(_T("color"), CString("(color 0 0 155)")));
	idx_fieldWidth = m_properties.Add(CProperty(_T("fieldWidth"), CString("")));
//	idx_height = m_properties.Add(CProperty(_T("height"), CString("")));
	idx_title = m_properties.Add(CProperty(_T("title"), CString(GetString(IDS_CHOOSE_A_COLOR))));

	m_properties[idx_color].m_type = PROP_LITERAL;
	m_properties[idx_fieldWidth].m_type = PROP_LITERAL;

	m_handlers.RemoveAll();
	m_handlers.Add(CHandler(false, _T("changed"), _T("col do"), _T("")));

	m_labelWnd.Create(
		_T("STATIC"), Caption(),
		WS_CHILD | WS_VISIBLE,
		m_sizer.m_rect, pParent, -1);
	m_labelWnd.SetFont(GetStdFont(), FALSE);

	m_wnd.Create(
		_T("STATIC"),
		NULL,
		WS_CHILD | WS_VISIBLE | SS_BITMAP,
		m_sizer.m_rect, pParent, -1);
	m_wnd.SendMessage(STM_SETIMAGE, (WPARAM)IMAGE_BITMAP,
		(LPARAM)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_COLORPICKER)));
}

// ============================================================================
void CColorPickerObj::SetRect(const CRect &rect)
{
	CRect r = rect;
	m_properties[IDX_XPOS] = r.left;
	m_properties[IDX_YPOS] = r.top;
	m_properties[IDX_WIDTH] = r.right;
	m_properties[IDX_HEIGHT] = r.bottom;

	m_sizer.m_rect = r;
	m_sizer.m_rect.right += r.left;
	m_sizer.m_rect.bottom += r.top;

	int textWidth = GetTextWidth(&m_labelWnd, Caption());
	textWidth += 4;

	m_labelWnd.MoveWindow(r.left, r.top + (r.bottom-c_stdFontHeight)/2, textWidth, c_stdFontHeight, TRUE);
	m_wnd.MoveWindow(r.left+textWidth, r.top, r.right-textWidth, r.bottom, TRUE);

	if(m_selected)
		m_wnd.GetParent()->InvalidateRect(NULL, TRUE);
}


// ============================================================================
int CColorPickerObj::OnPropChange(int idx)
{
	if(idx == IDX_CAPTION)
	{
		m_labelWnd.SetWindowText(Caption());
		CRect rect;
		GetRect(rect);
		SetRect(rect);
		return 1;
	}

	return CStdObj::OnPropChange(idx);
}


// ============================================================================
// CComboBoxObj
// ----------------------------------------------------------------------------
int CComboBoxObj::m_count = 0;

// ============================================================================
CComboBoxObj::CComboBoxObj(CWnd *pParent, const CRect &rect)
	: CStdObj(pParent, rect)
{
	Class() = _T("comboBox");
	Name().Format(_T("cbx%d"), ++m_count);
	Caption() = _T("ComboBox");

	idx_items = m_properties.Add(CProperty(_T("items"), CStringArray()));
	idx_selection = m_properties.Add(CProperty(_T("selection"), 1));

	m_handlers.RemoveAll();
	m_handlers.Add(CHandler(false, _T("selected"), _T("sel do"), _T("")));
	m_handlers.Add(CHandler(false, _T("doubleClicked"), _T("sel do"), _T("")));
	m_handlers.Add(CHandler(false, _T("entered"), _T("text do"), _T("")));
	m_handlers.Add(CHandler(false, _T("changed"), _T("text do"), _T("")));

	m_labelWnd.Create(
		_T("STATIC"), Caption(),
		WS_CHILD | WS_VISIBLE,
		m_sizer.m_rect, pParent, -1);
	m_labelWnd.SetFont(GetStdFont(), FALSE);

	m_wnd.Create(
		_T("COMBOBOX"), NULL,
		WS_CHILD | WS_VISIBLE | CBS_SIMPLE,
		m_sizer.m_rect, pParent, -1);
	m_wnd.SetFont(GetStdFont(), FALSE);
}

// ============================================================================
void CComboBoxObj::Emit(CString &out)
{
	CStdObj::Emit(out);
//	CString str = _T("");
//	str.Format(" items:%s", (LPCTSTR)(m_properties[idx_items]));
//	out += str;
}

// ============================================================================
CString CComboBoxObj::GetPropStr(int idx) const
{
	if(idx == IDX_HEIGHT)
	{
		CString str;
		str.Format("%d", GetPropInt(idx));
		return str;
	}

	return CStdObj::GetPropStr(idx);
}

// ============================================================================
void CComboBoxObj::SetPropStr(int idx, const CString &sVal)
{
	if(idx == IDX_HEIGHT)
	{
		SetPropInt(idx, atoi(sVal));
		return;
	}

	CStdObj::SetPropStr(idx, sVal);
}

// ============================================================================
int CComboBoxObj::GetPropInt(int idx) const
{
	if(idx == IDX_HEIGHT)
	{
		// convert from pixels to rows
		int pxHeight = CStdObj::GetPropInt(idx) - 6;
		pxHeight -= (GetPropStr(IDX_CAPTION).IsEmpty()) ? 0 : (c_stdFontHeight + SPACING_BEFORE);
		pxHeight -= ((CComboBox*)&m_wnd)->GetItemHeight(-1); // 17

		if(pxHeight <= 0)
			return 1;

		pxHeight -= 4;
		int itmHeight = ((CComboBox*)&m_wnd)->GetItemHeight(0); // 13
		return (pxHeight / itmHeight + 2);
	}
	return CStdObj::GetPropInt(idx);
}

// ============================================================================
void CComboBoxObj::SetPropInt(int idx, int iVal)
{
	if(idx == IDX_HEIGHT)
	{
		// convert from rows to pixels
		if(iVal <= 0)
			iVal = 1;

		int px = 4;
		if(iVal >= 1)
			px += ((CComboBox*)&m_wnd)->GetItemHeight(-1) + 2;
		if(iVal >= 2)
			px += 4;
		if(iVal >= 3)
			px += (iVal-2) * ((CComboBox*)&m_wnd)->GetItemHeight(0);

		px += (Caption().IsEmpty()) ? 0 : (c_stdFontHeight + SPACING_BEFORE);
		CStdObj::SetPropInt(idx, px);
	}
	else
		CStdObj::SetPropInt(idx, iVal);
}

// ============================================================================
void CComboBoxObj::SetRect(const CRect &rect)
{
	CRect r = rect;
	m_properties[IDX_XPOS] = r.left;
	m_properties[IDX_YPOS] = r.top;
	m_properties[IDX_WIDTH] = r.right;
	m_properties[IDX_HEIGHT] = r.bottom;

	m_sizer.m_rect = r;
	m_sizer.m_rect.right += r.left;
	m_sizer.m_rect.bottom += r.top;

	int textHeight = (Caption().IsEmpty()) ? 0 : c_stdFontHeight + SPACING_BEFORE;

	m_labelWnd.MoveWindow(r.left, r.top, r.right, textHeight, TRUE);
	m_wnd.MoveWindow(r.left, r.top+textHeight, r.right, r.bottom-textHeight, TRUE);

	if(m_selected)
		m_wnd.GetParent()->InvalidateRect(NULL, TRUE);
}


// ============================================================================
int CComboBoxObj::OnPropChange(int idx)
{
	if(idx == IDX_CAPTION)
	{
		m_labelWnd.SetWindowText(Caption());
		CRect rect;
		GetRect(rect);
		SetRect(rect);
		InvalidateProp(IDX_HEIGHT);
		return 1;
	}

	return CStdObj::OnPropChange(idx);
}

// ============================================================================
// CDropDownListObj
// ----------------------------------------------------------------------------
int CDropDownListObj::m_count = 0;

// ============================================================================
CDropDownListObj::CDropDownListObj(CWnd *pParent, const CRect &rect)
	: CStdObj(pParent, rect)
{
	Class() = _T("dropDownList");
	Name().Format(_T("ddl%d"), ++m_count);
	Caption() = _T("DropDownList");

	idx_items = m_properties.Add(CProperty(_T("items"), CStringArray()));
	idx_selection = m_properties.Add(CProperty(_T("selection"), 1));

	m_properties[IDX_HEIGHT] = 10;

	m_handlers.RemoveAll();
	m_handlers.Add(CHandler(false, _T("selected"), _T("sel do"), _T("")));

	m_labelWnd.Create(
		_T("STATIC"), Caption(),
		WS_CHILD | WS_VISIBLE,
		m_sizer.m_rect, pParent, -1);
	m_labelWnd.SetFont(GetStdFont(), FALSE);

	m_wnd.Create(
		_T("COMBOBOX"), NULL,
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
		m_sizer.m_rect, pParent, -1);
	m_wnd.SetFont(GetStdFont(), FALSE);
}

// ============================================================================
void CDropDownListObj::Emit(CString &out)
{
	CStdObj::Emit(out);
//	CString str = _T("");
//	str.Format(" items:%s", (LPCTSTR)(m_properties[idx_items]));
//	out += str;
}

// ============================================================================
void CDropDownListObj::GetRect(CRect &rect) const
{
	rect.left = (int)m_properties[IDX_XPOS];
	rect.top = (int)m_properties[IDX_YPOS];
	rect.right = (int)m_properties[IDX_WIDTH];
	rect.bottom = (int)m_properties[IDX_HEIGHT];

/*	CRect r;
	int textHeight = (GetPropStr(IDX_CAPTION).IsEmpty()) ? 0 : c_stdFontHeight + SPACING_BEFORE;
	m_wnd.GetWindowRect(r);
	m_wnd.GetParent()->ScreenToClient(r);
	r.bottom -= r.top;

	rect.bottom = r.bottom + textHeight;
*/
}

// ============================================================================
void CDropDownListObj::SetRect(const CRect &rect)
{
	CRect r = rect;

	r.bottom = m_wnd.SendMessage(CB_GETITEMHEIGHT, -1, 0) + 4; 
	int textHeight = (Caption().IsEmpty()) ? 0 : c_stdFontHeight + SPACING_BEFORE;

	m_labelWnd.MoveWindow(r.left, r.top, r.right, textHeight, TRUE);
	m_wnd.MoveWindow(r.left, r.top+textHeight, r.right, r.bottom, TRUE);

	m_wnd.GetWindowRect(r);
	m_wnd.GetParent()->ScreenToClient(r);
	r.top -= textHeight;

	m_sizer.m_rect = r;
	r.right -= r.left;
	r.bottom -= r.top;

	m_properties[IDX_XPOS] = r.left;
	m_properties[IDX_YPOS] = r.top;
	m_properties[IDX_WIDTH] = r.right;
	m_properties[IDX_HEIGHT] = r.bottom;

	if(m_selected)
		m_wnd.GetParent()->InvalidateRect(NULL, TRUE);
}


// ============================================================================
int CDropDownListObj::OnPropChange(int idx)
{
	if(idx == IDX_CAPTION)
	{
		m_labelWnd.SetWindowText(Caption());
		CRect rect;
		GetRect(rect);
		SetRect(rect);
		InvalidateProp(IDX_HEIGHT);
		return 1;
	}

	return CStdObj::OnPropChange(idx);
}


// ============================================================================
// CListBoxObj
// ----------------------------------------------------------------------------
int CListBoxObj::m_count = 0;

// ============================================================================
CListBoxObj::CListBoxObj(CWnd *pParent, const CRect &rect)
	: CStdObj(pParent, rect)
{
	Class() = _T("listBox");
	Name().Format(_T("lbx%d"), ++m_count);
	Caption() = _T("ListBox");

	idx_items = m_properties.Add(CProperty(_T("items"), CStringArray()));
	idx_selection = m_properties.Add(CProperty(_T("selection"), 1));

	m_handlers.RemoveAll();
	m_handlers.Add(CHandler(false, _T("selected"), _T("sel do"), _T("")));
	m_handlers.Add(CHandler(false, _T("doubleClicked"), _T("sel do"), _T("")));

	m_labelWnd.Create(
		_T("STATIC"), Caption(),
		WS_CHILD | WS_VISIBLE,
		m_sizer.m_rect, pParent, -1);
	m_labelWnd.SetFont(GetStdFont(), FALSE);

	m_wnd.CreateEx(
		WS_EX_CLIENTEDGE,
		_T("LISTBOX"), NULL,
		WS_CHILD | WS_VISIBLE,
		m_sizer.m_rect, pParent, -1);
	m_wnd.SetFont(GetStdFont(), FALSE);
}

// ============================================================================
void CListBoxObj::Emit(CString &out)
{
	CStdObj::Emit(out);
//	CString str = _T("");
//	str.Format(" items:%s", (LPCTSTR)(m_properties[idx_items]));
//	out += str;
}

// ============================================================================
CString CListBoxObj::GetPropStr(int idx) const
{
	if(idx == IDX_HEIGHT)
	{
		CString str;
		str.Format("%d", GetPropInt(idx));
		return str;
	}

	return CStdObj::GetPropStr(idx);
}

// ============================================================================
void CListBoxObj::SetPropStr(int idx, const CString &sVal)
{
	if(idx == IDX_HEIGHT)
	{
		SetPropInt(idx, atoi(sVal));
		return;
	}

	CStdObj::SetPropStr(idx, sVal);
}

// ============================================================================
int CListBoxObj::GetPropInt(int idx) const
{
	if(idx == IDX_HEIGHT)
	{
		int textHeight = (GetPropStr(IDX_CAPTION).IsEmpty()) ? 0 : (c_stdFontHeight + SPACING_BEFORE);
		return ((CStdObj::GetPropInt(idx) - textHeight - 4) / ((CListBox*)&m_wnd)->GetItemHeight(0));
	}

	return CStdObj::GetPropInt(idx);
}

// ============================================================================
void CListBoxObj::SetPropInt(int idx, int iVal)
{
	if(idx == IDX_HEIGHT)
	{
		int textHeight = (GetPropStr(IDX_CAPTION).IsEmpty()) ? 0 : (c_stdFontHeight + SPACING_BEFORE);
		iVal = iVal * ((CListBox*)&m_wnd)->GetItemHeight(0) + textHeight + 4;
	}

	CStdObj::SetPropInt(idx, iVal);
}

// ============================================================================
void CListBoxObj::SetRect(const CRect &rect)
{
	CRect r = rect;
	int textHeight = (Caption().IsEmpty()) ? 0 : c_stdFontHeight + SPACING_BEFORE;

	m_labelWnd.MoveWindow(r.left, r.top, r.right, textHeight, TRUE);
	m_wnd.MoveWindow(r.left, r.top+textHeight, r.right, r.bottom-textHeight, TRUE);

	m_wnd.GetWindowRect(r);
	m_wnd.GetParent()->ScreenToClient(r);
	r.top -= textHeight;

	m_sizer.m_rect = r;
	r.right -= r.left;
	r.bottom -= r.top;

	m_properties[IDX_XPOS] = r.left;
	m_properties[IDX_YPOS] = r.top;
	m_properties[IDX_WIDTH] = r.right;
	m_properties[IDX_HEIGHT] = r.bottom;

	if(m_selected)
		m_wnd.GetParent()->InvalidateRect(NULL, TRUE);
}


// ============================================================================
int CListBoxObj::OnPropChange(int idx)
{
	if(idx == IDX_CAPTION)
	{
		m_labelWnd.SetWindowText(Caption());
		CRect rect;
		GetRect(rect);
		SetRect(rect);
		InvalidateProp(IDX_HEIGHT);
		return 1;
	}

	return CStdObj::OnPropChange(idx);
}


// ============================================================================
// CEditTextObj
// ----------------------------------------------------------------------------
int CEditTextObj::m_count = 0;


// ============================================================================
CEditTextObj::CEditTextObj(CWnd *pParent, const CRect &rect)
	: CStdObj(pParent, rect)
{
	Class() = _T("editText");
	Name().Format(_T("edt%d"), ++m_count);
	Caption() = _T("");

	m_handlers.RemoveAll();
	m_handlers.Add(CHandler(false, _T("entered"), _T("text do"), _T("")));

	m_labelWnd.Create(
		_T("STATIC"), NULL,
		WS_CHILD | WS_VISIBLE,
		m_sizer.m_rect, pParent, -1);
	m_labelWnd.SetFont(GetStdFont(), FALSE);

	m_wnd.CreateEx(
		WS_EX_CLIENTEDGE,
		_T("EDIT"), NULL,
		WS_CHILD | WS_VISIBLE,
		m_sizer.m_rect, pParent, -1);
	m_wnd.SetFont(GetStdFont(), FALSE);

	OnPropChange(IDX_CAPTION);
}

// ============================================================================
void CEditTextObj::SetRect(const CRect &rect)
{
	CRect r = rect;
//	r.bottom = c_stdFontHeight + 2;  // fix the height of the edit control

	m_properties[IDX_XPOS] = r.left;
	m_properties[IDX_YPOS] = r.top;
	m_properties[IDX_WIDTH] = r.right;
	m_properties[IDX_HEIGHT] = r.bottom;

	m_sizer.m_rect = r;
	m_sizer.m_rect.right += r.left;
	m_sizer.m_rect.bottom += r.top;

	int textWidth = GetTextWidth(&m_labelWnd, Caption());
//	if(textWidth) textWidth += 4;
	textWidth += 4;

	m_labelWnd.MoveWindow(r.left, r.top, textWidth, r.bottom, TRUE);
	m_wnd.MoveWindow(r.left+textWidth, r.top, r.right-textWidth, r.bottom, TRUE);

	if(m_selected)
		m_wnd.GetParent()->InvalidateRect(NULL, TRUE);
}


// ============================================================================
int CEditTextObj::OnPropChange(int idx)
{
	if(idx == IDX_CAPTION)
	{
		m_labelWnd.SetWindowText(Caption());
		CRect rect;
		GetRect(rect);
		SetRect(rect);
		return 1;
	}

	return CStdObj::OnPropChange(idx);
}

// ============================================================================
void CEditTextObj::Emit(CString &out)
{
	CGuiObj::Emit(out);
}


// ============================================================================
// CLabelObj
// ----------------------------------------------------------------------------
int CLabelObj::m_count = 0;

// ============================================================================
CLabelObj::CLabelObj(CWnd *pParent, const CRect &rect)
	: CStdObj(pParent, rect)
{
	Class() = _T("label");
	Name().Format(_T("lbl%d"), ++m_count);
	Caption() = _T("Label");

	m_handlers.RemoveAll();

	m_wnd.Create(
		_T("static"),
		Caption(),
		WS_CHILD | WS_VISIBLE,
		m_sizer.m_rect, pParent, -1);
	m_wnd.SetFont(GetStdFont(), FALSE);
}


// ============================================================================
// CGroupBoxObj
// ----------------------------------------------------------------------------
int CGroupBoxObj::m_count = 0;

// ============================================================================
CGroupBoxObj::CGroupBoxObj(CWnd *pParent, const CRect &rect)
	: CStdObj(pParent, rect)
{
	Class() = _T("groupBox");
	Name().Format(_T("grp%d"), ++m_count);
	Caption() = _T("GroupBox");

	m_handlers.RemoveAll();

	m_wnd.Create(
		_T("button"),
		Caption(),
		BS_GROUPBOX | WS_VISIBLE | WS_CHILD,
		m_sizer.m_rect, pParent, -1);
	m_wnd.SetFont(GetStdFont(), FALSE);
}

// ============================================================================
bool CGroupBoxObj::HitTest(const CRect &rect)
{
	CRect r;

	int cx = (rect.right-rect.left) >> 1;
	int cy = (rect.bottom-rect.top) >> 1;

	CPoint point;
	point.x = rect.left + cx;
	point.y = rect.top + cy;

	GetRect(r);
	r.right = (r.left + r.right) + (cx + 5);
	r.left -= cx + 5;
	r.bottom = (r.top + r.bottom) + (cy + 5);
	r.top -= cy + 5;

	if(point.x < r.left ||
		point.x > r.right ||
		point.y < r.top ||
		point.y > r.bottom
	) return false;

	GetRect(r);
	r.right = (r.left + r.right) - (cx + 5);
	r.left += cx + 5;
	r.bottom = (r.top + r.bottom) - (cy + 5);
	r.top += cx + 15;

	if(point.x < r.left ||
		point.x > r.right ||
		point.y < r.top ||
		point.y > r.bottom
	) return true;

	return false;
}

// ============================================================================
void CGroupBoxObj::Emit(CString &out)
{
	CStdObj::Emit(out);
}

// ============================================================================
// CCheckboxObj
// ----------------------------------------------------------------------------
int CCheckboxObj::m_count = 0;

// ============================================================================
CCheckboxObj::CCheckboxObj(CWnd *pParent, const CRect &rect)
	: CStdObj(pParent, (rect.right&&rect.bottom ? rect : CRect(rect.left,rect.top,80,30)))
{
	Class() = _T("checkbox");
	Name().Format(_T("chk%d"), ++m_count);
	Caption() = _T("Checkbox");

	idx_checked = m_properties.Add(CProperty(_T("checked"), false));

	m_handlers.RemoveAll();
	m_handlers.Add(CHandler(false, _T("changed"), _T("state do"), _T("")));

	m_wnd.Create(
		_T("BUTTON"),
		Caption(),
		WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
		m_sizer.m_rect, pParent, -1);
	m_wnd.SetFont(GetStdFont(), FALSE);
}

// ============================================================================
int CCheckboxObj::OnPropChange(int idx)
{
	if(CStdObj::OnPropChange(idx))
		return 1;

	if(idx == idx_checked)
	{
		m_wnd.SendMessage(BM_SETCHECK, (int)(m_properties[idx]), 0);
		return 1;
	}

	return 0;
}

// ============================================================================
void CCheckboxObj::Emit(CString &out)
{
	CStdObj::Emit(out);
//	CString str = _T("");
//	str.Format(" checked:%s", (LPCTSTR)(m_properties[idx_checked]));
//	out += str;
}


// ============================================================================
// CRadioButtonsObj
// ----------------------------------------------------------------------------
int CRadioButtonsObj::m_count = 0;

// ============================================================================
CRadioButtonsObj::CRadioButtonsObj(CWnd *pParent, const CRect &rect)
	: CGuiObj(pParent)
{
	m_pParent = pParent;
	m_sizer.m_rect = rect;
	m_sizer.m_rect.right += rect.left;
	m_sizer.m_rect.bottom += rect.top;

	m_properties[IDX_XPOS] = rect.left;
	m_properties[IDX_YPOS] = rect.top;
	m_properties[IDX_WIDTH] = rect.right;
	m_properties[IDX_HEIGHT] = rect.bottom;
	m_properties[IDX_CLASS].m_flags |= FLAG_READONLY;
	m_properties[IDX_WIDTH].m_flags |= FLAG_READONLY;
	m_properties[IDX_HEIGHT].m_flags |= FLAG_READONLY;

	Class() = _T("radioButtons");
	Name().Format(_T("rdo%d"), ++m_count);
	Caption() = _T("RadioButtons");

	m_captionWnd.Create(
		_T("STATIC"),
		Caption(),
		WS_CHILD,
		CRect(0,0,0,0), pParent, -1);
	m_captionWnd.SetFont(GetStdFont(), FALSE);

	idx_labels = m_properties.Add(CProperty(_T("labels"), CStringArray()));
	idx_default = m_properties.Add(CProperty(_T("default"), 1));
	idx_columns = m_properties.Add(CProperty(_T("columns"), 1));

	m_properties[idx_labels].m_array.Add(_T("\"RadioButton\""));

	m_handlers.RemoveAll();
	m_handlers.Add(CHandler(false, _T("changed"), _T("stat do"), _T("")));

	m_selected = TRUE;

	UpdateButtons();
}

// ============================================================================
CRadioButtonsObj::~CRadioButtonsObj()
{
	for(int i = 0; i < m_buttons.GetSize(); i++)
	{
		CWnd *wnd = (CWnd*)m_buttons.GetAt(i);
		delete wnd;
	}
}

// ============================================================================
int CRadioButtonsObj::OnPropChange(int idx)
{
	if(idx == idx_labels || idx == idx_columns || idx == idx_default)
	{
		UpdateButtons();
		return 1;
	}

	CRect rect;
	switch(idx)
	{
	case IDX_CAPTION:
		UpdateButtons();
		return 1;
	case IDX_XPOS:
	case IDX_YPOS:
	case IDX_WIDTH:
	case IDX_HEIGHT:
		GetRect(rect);
		SetRect(rect);
		return 1;
	}
	return 0;
}

// ============================================================================
void CRadioButtonsObj::Emit(CString &out)
{
	CGuiObj::Emit(out);
}

// ============================================================================
bool CRadioButtonsObj::UpdateButtons()
{
	int numBtns = m_properties[idx_labels].m_array.GetSize();
	int count = numBtns - m_buttons.GetSize();

	// add new buttons
	if(count > 0)
	{
		for(int i = 0; i < count; i++)
		{
			CWnd *wnd = new CWnd;
			wnd->Create(
				_T("BUTTON"),
				_T(""),
				BS_RADIOBUTTON | WS_CHILD | WS_VISIBLE,
				CRect(0,0,0,0), m_pParent, -1);
			wnd->SetFont(GetStdFont(), FALSE);
			m_buttons.Add(wnd);
		}
	}

	// remove buttons
	else if(count < 0)
	{
		count = -count;
		for(int i = 0; i < count; i++)
		{
			CWnd *wnd = (CWnd*)m_buttons.GetAt(numBtns+i);
			wnd->DestroyWindow();
			delete wnd;
		}
		m_buttons.SetSize(numBtns);
	}

	int xPos = GetPropInt(IDX_XPOS);
	int yPos = GetPropInt(IDX_YPOS);

	int capHeight = (Caption().IsEmpty()) ? 0 : c_stdFontHeight;

	if(capHeight)
	{
		int capWidth = GetTextWidth(&m_captionWnd, Caption());
		m_captionWnd.SetWindowText(Caption());
		m_captionWnd.ShowWindow(SW_SHOW);
		m_captionWnd.MoveWindow(xPos, yPos, capWidth, capHeight);
		yPos += capHeight;
	}
	else
		m_captionWnd.ShowWindow(SW_HIDE);

	int btnWidth = 0;
	int btnHeight = c_stdFontHeight + 2;

	// get maximum button width
	for(int i = 0; i < numBtns; i++)
	{
		CString str = m_properties[idx_labels].m_array[i];
		if(HasQuotes(str))
			StripQuotes(str);
		else
			str = _T("! ") + str; // NOTE: its a literal in a radio button items list!

		int width = GetTextWidth(&m_captionWnd, str) + RADIO_DOT_WIDTH;
		if(width > btnWidth)
			btnWidth = width;
	}

	// position the buttons
	int totalWidth = 0, totalHeight = 0;
	int columns = GetPropInt(idx_columns);
	for(int i = 0; i < numBtns; i++)
	{
		CWnd *wnd = (CWnd*)m_buttons.GetAt(i);

		CString str = m_properties[idx_labels].m_array[i];
		if(HasQuotes(str))
			StripQuotes(str);
		else
			str = _T("! ") + str; // NOTE: its a literal in a radio button items list!
		wnd->SetWindowText(str);

		if(i+1 == GetPropInt(idx_default))
			wnd->SendMessage(BM_SETCHECK, BST_CHECKED, 0);
		else
			wnd->SendMessage(BM_SETCHECK, BST_UNCHECKED, 0);

		if(columns == 0 || columns == numBtns)
		{
			CSize size = m_pParent->GetDC()->GetTextExtent(str);
			btnWidth = size.cx + RADIO_DOT_WIDTH;
			wnd->MoveWindow(xPos, yPos, btnWidth, btnHeight);
			xPos += btnWidth;
			totalWidth += btnWidth;
		}
		else
		{
			int x = xPos + (i % columns) * btnWidth;
			wnd->MoveWindow(x, yPos, btnWidth, btnHeight);
			if(i % columns == columns - 1)
				yPos += btnHeight;
		}
	}

	// calc the total bounding box of all buttons
	if(columns == 0 || columns == numBtns)
		totalHeight = btnHeight;
	else
	{
		totalWidth = columns * btnWidth;
		totalHeight = (numBtns / columns) * btnHeight;
		if(numBtns % columns)
			totalHeight += btnHeight;
	}
	totalHeight += capHeight;

	// set the tracker rect
	m_properties[IDX_WIDTH] = totalWidth;
	m_properties[IDX_HEIGHT] = totalHeight;
	m_minimumSize = m_maximumSize = CSize(totalWidth, totalHeight);

	m_sizer.m_rect.left = (int)m_properties[IDX_XPOS];
	m_sizer.m_rect.top  = (int)m_properties[IDX_YPOS];
	m_sizer.m_rect.right  = m_sizer.m_rect.left + totalWidth;
	m_sizer.m_rect.bottom = m_sizer.m_rect.top  + totalHeight;

	if(m_selected)
		m_pParent->InvalidateRect(NULL, TRUE);

	return true;
}

// ============================================================================
void CRadioButtonsObj::SetRect(const CRect &rect)
{
	m_properties[IDX_XPOS] = rect.left;
	m_properties[IDX_YPOS] = rect.top;
	UpdateButtons();
}

// ============================================================================
void CRadioButtonsObj::GetRect(CRect &rect) const
{
	rect.left = (int)m_properties[IDX_XPOS];
	rect.top = (int)m_properties[IDX_YPOS];
	rect.right = (int)m_properties[IDX_WIDTH];
	rect.bottom = (int)m_properties[IDX_HEIGHT];
}

// ============================================================================
CString CRadioButtonsObj::GetPropStr(int idx) const
{
/* NOTE: this code auto adds quotes
	if(idx == idx_labels)
	{
		CString str = _T("#(");
		CProperty prop = m_properties.GetAt(idx_labels);
		for(int i = 0; i < prop.m_array.GetSize(); i++)
		{
			str += _T("\"") + prop.m_array[i] + _T("\"");
			if(i != prop.m_array.GetSize()-1)
				str += _T(", ");
		}
		str += _T(")");
		return str;
	}
*/

	return CGuiObj::GetPropStr(idx);
}

// ============================================================================
void CRadioButtonsObj::SetPropStr(int idx, const CString &sVal)
{
	if(idx == IDX_WIDTH || idx == IDX_HEIGHT)
		return;

	CGuiObj::SetPropStr(idx, sVal);
}

// ============================================================================
void CRadioButtonsObj::SetPropInt(int idx, int iVal)
{
	if(idx == IDX_WIDTH || idx == IDX_HEIGHT)
		return;

	CGuiObj::SetPropInt(idx, iVal);
}


// ============================================================================
// CSpinnerObj
// ----------------------------------------------------------------------------
int CSpinnerObj::m_count = 0;


// ============================================================================
CSpinnerObj::CSpinnerObj(CWnd *pParent, const CRect &rect)
	: CStdObj(pParent, rect)
{
	Class() = _T("spinner");
	Name().Format(_T("spn%d"), ++m_count);
	Caption() = _T("");

	idx_range = m_properties.Add(CProperty(_T("range"), Point3(0.f,100.f,0.f)));
	idx_type = m_properties.Add(CProperty(_T("type"), CStringArray(), 0));
	idx_scale = m_properties.Add(CProperty(_T("scale"), 0.1f));
	idx_controller = m_properties.Add(CProperty(_T("controller"), CString("")));
//	idx_fieldWidth = m_properties.Add(CProperty(_T("fieldWidth"), 0));

	m_properties[idx_type].m_array.Add(_T("#float"));
	m_properties[idx_type].m_array.Add(_T("#integer"));
	m_properties[idx_type].m_array.Add(_T("#worldunits"));

	m_handlers.RemoveAll();
	m_handlers.Add(CHandler(false, _T("changed"), _T("val do"), _T("")));
	m_handlers.Add(CHandler(false, _T("entered"), _T("text do"), _T("")));
	m_handlers.Add(CHandler(false, _T("buttondown"), _T(" do"), _T("")));
	m_handlers.Add(CHandler(false, _T("buttonup"), _T(" do"), _T("")));

	m_labelWnd.Create(
		_T("STATIC"), NULL,
		WS_CHILD | WS_VISIBLE,
		m_sizer.m_rect, pParent, -1);
	m_labelWnd.SetFont(GetStdFont(), FALSE);

	m_wnd.CreateEx(
		WS_EX_CLIENTEDGE,
		_T("EDIT"), NULL,
		WS_CHILD | WS_VISIBLE,
		m_sizer.m_rect, pParent, -1);
	m_wnd.SetFont(GetStdFont(), FALSE);

	m_spn.Create(
		NULL,
		WS_CHILD | WS_VISIBLE | SS_BITMAP,
		m_sizer.m_rect,
		pParent);
	m_spn.SetBitmap(LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_SPINNER)));

	OnPropChange(IDX_CAPTION);
}

// ============================================================================
void CSpinnerObj::SetRect(const CRect &rect)
{
	CRect r = rect;
	r.bottom = c_stdFontHeight + 2;
	m_minimumSize.cy = m_maximumSize.cy = r.bottom;

	m_properties[IDX_XPOS] = r.left;
	m_properties[IDX_YPOS] = r.top;
	m_properties[IDX_WIDTH] = r.right;
	m_properties[IDX_HEIGHT] = r.bottom;

	m_sizer.m_rect = r;
	m_sizer.m_rect.right += r.left;
	m_sizer.m_rect.bottom += r.top;

	int textWidth = GetTextWidth(&m_labelWnd, Caption());
	r.left += 1;

	// MXS BUG ?
	if(textWidth)
	{
		r.left -= 10;
		r.right += 10;
		textWidth += 2;
	}

	m_labelWnd.MoveWindow(r.left, r.top, textWidth, r.bottom, TRUE);
	m_wnd.MoveWindow(r.left+textWidth, r.top, r.right-textWidth-13, r.bottom, TRUE);
	m_spn.MoveWindow(r.left+r.right-13, r.top, 12, 16, TRUE);

	if(m_selected)
		m_wnd.GetParent()->InvalidateRect(NULL, TRUE);
}

// ============================================================================
int CSpinnerObj::OnPropChange(int idx)
{
	if(idx == IDX_CAPTION)
	{
		m_labelWnd.SetWindowText(Caption());
		CRect rect;
		GetRect(rect);
		SetRect(rect);
		return 1;
	}

	return CStdObj::OnPropChange(idx);
}

// ============================================================================
void CSpinnerObj::Emit(CString &out)
{
	CGuiObj::Emit(out);
}


// ============================================================================
// CProgressBarObj
// ----------------------------------------------------------------------------
int CProgressBarObj::m_count = 0;

// ============================================================================
CProgressBarObj::CProgressBarObj(CWnd *pParent, const CRect &rect)
	: CStdObj(pParent, (rect.right&&rect.bottom ? rect : CRect(rect.left,rect.top,80,30)))
{
	Class() = _T("progressBar");
	Name().Format(_T("pb%d"), ++m_count);
	Caption() = _T("ProgressBar");

	idx_value = m_properties.Add(CProperty(_T("value"), 0));
	idx_color = m_properties.Add(CProperty(_T("color"), CString("(color 30 10 190)")));
	idx_orient = m_properties.Add(CProperty(_T("orient"), CStringArray(), 0));

	m_properties[idx_color].m_type = PROP_LITERAL;
	m_properties[idx_orient].m_array.Add(_T("#horizontal"));
	m_properties[idx_orient].m_array.Add(_T("#vertical"));

	m_handlers.RemoveAll();
	m_handlers.Add(CHandler(false, _T("clicked"), _T("val do"), _T("")));

	m_wnd.Create(
		_T("STATIC"),
		NULL,
		WS_CHILD | WS_VISIBLE | SS_SUNKEN,
		rect, pParent, -1);
}


// ============================================================================
// CSliderObj
// ----------------------------------------------------------------------------
int CSliderObj::m_count = 0;

// ============================================================================
CSliderObj::CSliderObj(CWnd *pParent, const CRect &rect)
	: CStdObj(pParent, (rect.right&&rect.bottom ? rect : CRect(rect.left,rect.top,80,30)))
{
	Class() = _T("slider");
	Name().Format(_T("sld%d"), ++m_count);
	Caption() = _T("Slider");

	idx_range = m_properties.Add(CProperty(_T("range"), Point3(0.f,100.f,0.f)));
	idx_type  = m_properties.Add(CProperty(_T("type"), CStringArray(), 0));
	idx_orient = m_properties.Add(CProperty(_T("orient"), CStringArray(), 0));
	idx_ticks = m_properties.Add(CProperty(_T("ticks"), 10));

	m_properties[idx_type].m_array.Add(_T("#float"));
	m_properties[idx_type].m_array.Add(_T("#integer"));

	m_properties[idx_orient].m_array.Add(_T("#horizontal"));
	m_properties[idx_orient].m_array.Add(_T("#vertical"));

	m_handlers.RemoveAll();
	m_handlers.Add(CHandler(false, _T("changed"), _T("val do"), _T("")));
	m_handlers.Add(CHandler(false, _T("buttondown"), _T(" do"), _T("")));
	m_handlers.Add(CHandler(false, _T("buttonup"), _T(" do"), _T("")));

	m_labelWnd.Create(
		_T("STATIC"), Caption(),
		WS_CHILD | WS_VISIBLE,
		m_sizer.m_rect, pParent, -1);
	m_labelWnd.SetFont(GetStdFont(), FALSE);

	m_wnd.Create(
		TRACKBAR_CLASS,
		NULL,
		WS_CHILD | WS_VISIBLE | TBS_HORZ,
		rect, pParent, -1);
}

// ============================================================================
void CSliderObj::SetRect(const CRect &rect)
{
	CRect r = rect;
	int textHeight = (Caption().IsEmpty()) ? 0 : c_stdFontHeight + SPACING_BEFORE;

	if(GetPropStr(idx_orient).Compare(_T("#vertical")) == 0)
		r.bottom = 80; // TODO: why ??
	else
		r.bottom = c_stdFontHeight + SPACING_BEFORE - 2 + 8;

	m_labelWnd.MoveWindow(r.left, r.top, r.right, textHeight, TRUE);
	m_wnd.MoveWindow(r.left, r.top+textHeight, r.right, r.bottom, TRUE);

	m_wnd.GetWindowRect(r);
	m_wnd.GetParent()->ScreenToClient(r);
	r.top -= textHeight;

	m_sizer.m_rect = r;
	r.right -= r.left;
	r.bottom -= r.top;

	m_properties[IDX_XPOS] = r.left;
	m_properties[IDX_YPOS] = r.top;
	m_properties[IDX_WIDTH] = r.right;
	m_properties[IDX_HEIGHT] = r.bottom;

	m_minimumSize.cy = m_maximumSize.cy = r.bottom;

	if(m_selected)
		m_wnd.GetParent()->InvalidateRect(NULL, TRUE);
}


// ============================================================================
int CSliderObj::OnPropChange(int idx)
{
	if(idx == IDX_CAPTION)
	{
		m_labelWnd.SetWindowText(Caption());
		CRect rect;
		GetRect(rect);
		SetRect(rect);
		InvalidateProp(IDX_HEIGHT);
		return 1;
	}
	else if(idx == idx_orient)
	{
		CRect rect;
		GetRect(rect);
		CWnd *pParent = m_wnd.GetParent();
		m_wnd.DestroyWindow();

		UINT flag = TBS_HORZ;
		if(GetPropStr(idx_orient).Compare(_T("#vertical")) == 0)
			flag = TBS_VERT;

		m_wnd.Create(
			TRACKBAR_CLASS,
			NULL,
			WS_CHILD | WS_VISIBLE | flag,
			rect, pParent, -1);
		SetRect(rect);
	}

	return CStdObj::OnPropChange(idx);
}


// ============================================================================
// CTimerObj
// ----------------------------------------------------------------------------
int CTimerObj::m_count = 0;

// ============================================================================
CTimerObj::CTimerObj(CWnd *pParent, const CRect &rect)
	: CStdObj(pParent, CRect(rect.left,rect.top,24,24))
{
	Class() = _T("timer");
	Name().Format(_T("tmr%d"), ++m_count);
	Caption() = _T("Timer");

	idx_interval = m_properties.Add(CProperty(_T("interval"), 1000));
	idx_active   = m_properties.Add(CProperty(_T("active"), true));
	m_properties[IDX_WIDTH].m_flags |= FLAG_READONLY;
	m_properties[IDX_HEIGHT].m_flags |= FLAG_READONLY;

	m_handlers.RemoveAll();
	m_handlers.Add(CHandler(false, _T("tick"), _T(" do"), _T("")));

	m_minimumSize = m_maximumSize = CSize(24,24);

	m_wnd.Create(
		_T("STATIC"),
		NULL,
		WS_CHILD | WS_VISIBLE | SS_ICON | SS_REALSIZEIMAGE | SS_CENTERIMAGE,
		rect, pParent, -1);
	m_wnd.SendMessage(STM_SETIMAGE, (WPARAM)IMAGE_ICON,
		(LPARAM)LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_TIMER)));
}

// ============================================================================
void CTimerObj::SetRect(const CRect &rect)
{
	CRect r = rect;
	r.right = r.bottom = 24;
	CStdObj::SetRect(r);
}

// ============================================================================
// CActiveXObj
// ----------------------------------------------------------------------------
int CActiveXObj::m_count = 0;

// ============================================================================
CActiveXObj::CActiveXObj(CWnd *pParent, const CRect &rect)
	: CStdObj(pParent, CRect(rect.left,rect.top,24,24))
{
	Class() = _T("activeXControl");
	Name().Format(_T("actx%d"), ++m_count);
	Caption() = _T("???");

	m_properties[IDX_CAPTION].m_name = _T("controlType");

	m_properties[IDX_WIDTH].m_flags |= FLAG_READONLY;
	m_properties[IDX_HEIGHT].m_flags |= FLAG_READONLY;

	m_handlers.RemoveAll();

	m_wnd.CreateEx(
		WS_EX_CLIENTEDGE,
		_T("STATIC"),
		NULL,
		WS_CHILD | WS_VISIBLE | SS_ICON | SS_REALSIZEIMAGE | SS_CENTERIMAGE,
		rect, pParent, -1);
	m_wnd.SendMessage(STM_SETIMAGE, (WPARAM)IMAGE_ICON,
		(LPARAM)LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ACTIVEX)));
}

// ============================================================================
void CActiveXObj::SetRect(const CRect &rect)
{
	CRect r = rect;
//	r.right = r.bottom = 24;
	CStdObj::SetRect(r);
}

// ============================================================================
// CCustomObj
// ----------------------------------------------------------------------------
int CCustomObj::m_count = 0;

// ============================================================================
CCustomObj::CCustomObj(CWnd *pParent, const CRect &rect)
	: CStdObj(pParent, CRect(rect.left,rect.top,24,24))
{
	Class() = _T("???");
	Name().Format(_T("cust%d"), ++m_count);
	Caption() = _T("???");

	m_properties[IDX_CLASS].m_flags &= ~FLAG_READONLY;   // enable class name editing for custom type

	m_properties[IDX_WIDTH].m_flags |= FLAG_READONLY;
	m_properties[IDX_HEIGHT].m_flags |= FLAG_READONLY;

	m_handlers.RemoveAll();

	m_wnd.CreateEx(
		WS_EX_CLIENTEDGE,
		_T("STATIC"),
		NULL,
		WS_CHILD | WS_VISIBLE | SS_ICON | SS_REALSIZEIMAGE | SS_CENTERIMAGE,
		rect, pParent, -1);
	m_wnd.SendMessage(STM_SETIMAGE, (WPARAM)IMAGE_ICON,
		(LPARAM)LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_CUSTOM)));
}

// ============================================================================
void CCustomObj::SetRect(const CRect &rect)
{
	CRect r = rect;
//	r.right = r.bottom = 24;
	CStdObj::SetRect(r);
}

