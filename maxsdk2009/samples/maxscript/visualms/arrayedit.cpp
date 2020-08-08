// ============================================================================
// ArrayEdit.cpp
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#include "stdafx.h"
#include "VisualMS.h"
#include "ArrayEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// ============================================================================
// CReturnEdit control
// ----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CReturnEdit, CEdit)
	//{{AFX_MSG_MAP(CReturnEdit)
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CReturnEdit::CReturnEdit()
{
	m_bShiftDown = false;
}
CReturnEdit::~CReturnEdit() {}

void CReturnEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(nChar == '\r')
	{
		GetParent()->SendMessage(WM_COMMAND,
			MAKEWPARAM(IDC_EDT_ADDSTRING, EN_RETURN), (LPARAM)m_hWnd);
	}
	else
		CEdit::OnChar(nChar, nRepCnt, nFlags);
}

// ============================================================================
void CReturnEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(nChar == VK_SHIFT)
		m_bShiftDown = true;

	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

// ============================================================================
void CReturnEdit::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(nChar == VK_SHIFT)
		m_bShiftDown = false;

	CEdit::OnKeyUp(nChar, nRepCnt, nFlags);
}

// ============================================================================
// CDelListBox control
// ----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CDelListBox, CListBox)
	//{{AFX_MSG_MAP(CDelListBox)
	ON_WM_KEYUP()
	ON_CONTROL_REFLECT(LBN_SELCHANGE, OnSelChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CDelListBox::CDelListBox() {}
CDelListBox::~CDelListBox() {}

void CDelListBox::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(nChar == VK_DELETE)
	{
		GetParent()->SendMessage(WM_COMMAND,
			MAKEWPARAM(IDC_LBX_ARRAY, LBN_DEL), (LPARAM)m_hWnd);
	}
	else
		CListBox::OnKeyUp(nChar, nRepCnt, nFlags);
}


void CDelListBox::OnSelChange() 
{
		
}

// ============================================================================
// CArrayEdit dialog
// ----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CArrayEdit, CDialog)
	//{{AFX_MSG_MAP(CArrayEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CArrayEdit::CArrayEdit(CWnd* pParent /*=NULL*/)
	: CDialog(CArrayEdit::IDD, pParent)
{
}

CArrayEdit::CArrayEdit(const CStringArray &arr, CWnd* pParent /*=NULL*/)
	: CDialog(CArrayEdit::IDD, pParent)
{
	//{{AFX_DATA_INIT(CArrayEdit)
	m_string = _T("");
	//}}AFX_DATA_INIT

	m_stringArray.RemoveAll();
	m_stringArray.Copy(arr);
}

void CArrayEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CArrayEdit)
	DDX_Text(pDX, IDC_EDT_ADDSTRING, m_string);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CArrayEdit message handlers
BOOL CArrayEdit::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_edit.SubclassDlgItem(IDC_EDT_ADDSTRING, this);
	m_lbxArray.SubclassDlgItem(IDC_LBX_ARRAY, this);

	for(int i = 0; i < m_stringArray.GetSize(); i++)
		m_lbxArray.AddString(m_stringArray[i]);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CArrayEdit::OnOK() 
{
	m_stringArray.RemoveAll();
	for(int i = 0; i < m_lbxArray.GetCount(); i++)
	{
		CString str;
		m_lbxArray.GetText(i, str);
		m_stringArray.Add(str);
	}

	CDialog::OnOK();
}


BOOL CArrayEdit::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch(HIWORD(wParam))
	{
	case EN_RETURN:
		if(LOWORD(wParam) == IDC_EDT_ADDSTRING)
		{
			UpdateData(TRUE);

			// add quotes to the string if the shift key is being held
			CString str;
			if(m_edit.m_bShiftDown)
				str = _T("\"") + m_string + _T("\"");
			else
				str = m_string;

			if(m_lbxArray.GetSelCount() > 0)
			{
				int cnt = m_lbxArray.GetCount();
				for(int i = 0; i < cnt; i++)
				{
					if(m_lbxArray.GetSel(i))
					{
						m_lbxArray.DeleteString(i);
						m_lbxArray.InsertString(i, str);
					}
				}
			}
			else
				m_lbxArray.AddString(str);

			m_string = _T("");
			UpdateData(FALSE);
			return TRUE;
		}
		break;

	case LBN_DEL:
		if(LOWORD(wParam) == IDC_LBX_ARRAY)
		{
			for(int i = 0; i < m_lbxArray.GetCount(); i++)
			{
				if(m_lbxArray.GetSel(i))
					m_lbxArray.DeleteString(i--);
			}
			return TRUE;
		}
		break;
	}

	return CDialog::OnCommand(wParam, lParam);
}

