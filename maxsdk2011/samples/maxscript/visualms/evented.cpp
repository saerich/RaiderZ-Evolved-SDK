// ============================================================================
// EventEd.cpp
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#include "stdafx.h"
#include "VisualMS.h"
#include "EventEd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int c_dlgEdge = 3;

// ============================================================================
BEGIN_MESSAGE_MAP(CEventEd, CDialog)
	//{{AFX_MSG_MAP(CEventEd)
	ON_CBN_SELCHANGE(IDC_CONTROL, OnSelChangeControl)
	ON_CBN_SELCHANGE(IDC_EVENT, OnSelChangeEvent)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_CLOSE()
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_COMMAND(ID_EDIT_SELECTALL, OnEditSelectAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ============================================================================
CEventEd::CEventEd(CWnd* pParent /*=NULL*/)
	: CDialog(CEventEd::IDD, pParent)
{
}

// ============================================================================
CEventEd::CEventEd(CVisualMSDoc *pDoc, int startSel, CWnd* pParent /*=NULL*/)
	: CDialog(CEventEd::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEventEd)
	//}}AFX_DATA_INIT

	m_startSel = startSel;
	m_curSel = -1;
	m_pCurObj = NULL;
	m_pDoc = pDoc;
}

// ============================================================================
void CEventEd::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEventEd)
	DDX_Control(pDX, IDC_TXT_EVENT, m_txtEvent);
	DDX_Control(pDX, IDC_TXT_CONTROL, m_txtControl);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDC_EVENT, m_cbxEvents);
	DDX_Control(pDX, IDC_CONTROL, m_cbxControls);
	DDX_Control(pDX, IDC_EDITOR, m_editor);
	DDX_Control(pDX, IDC_DECLARATION, m_stcDeclaration);
	//}}AFX_DATA_MAP
}

// ============================================================================
void CEventEd::BuildEventList()
{
	m_cbxEvents.ResetContent();
	int size = m_pCurObj->m_handlers.GetSize();
	for(int i = 0; i < size; i++)
		m_cbxEvents.AddString(m_pCurObj->m_handlers[i].m_name);

	if(m_startSel >= size)
		m_startSel = 0;
	if(size == 0)
		m_startSel = -1;

	m_cbxEvents.SetCurSel(m_startSel);
	OnSelChangeEvent();
}

// ============================================================================
CFont* CEventEd::editor_font = NULL;
CFont  CEventEd::m_font;

BOOL CEventEd::OnInitDialog() 
{
	CDialog::OnInitDialog();
	LoadWindowPosition(_T("EventEditor"), m_hWnd);

	int sel = m_pDoc->GetNextSel();
	if(sel == -1) return FALSE;
	m_pCurObj = m_pDoc->GetObj(sel);

	m_cbxControls.ResetContent();
	for(int i = 0; i < m_pDoc->GetSize(); i++)
		m_cbxControls.AddString(m_pDoc->GetObj(i)->Name());
	m_cbxControls.SetCurSel(sel);


	// set up edit box  (copied from ScriptEd.cpp)
	// make font if not already
	if (editor_font == NULL)
	{
		BOOL res = m_font.CreateFont(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
								  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
								  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH,
								  _T("Courier New"));
		if (res == FALSE)
			m_font.CreateStockObject(ANSI_FIXED_FONT);
		editor_font = &m_font;
	}
	m_editor.SetFont(editor_font, FALSE); 

	// setup tabs
	PARAFORMAT	para_fmt;
	SIZE		size;
	CDC*		cdc;
	HDC			dc;
	memset(&para_fmt, 0, sizeof(PARAFORMAT));	// set tabstops 
	para_fmt.cbSize = sizeof(PARAFORMAT);
	para_fmt.dwMask = PFM_TABSTOPS;
	para_fmt.cTabCount = MAX_TAB_STOPS;
	cdc = m_editor.GetDC(); 
	dc = cdc->m_hDC; 
	SelectObject(dc, (HFONT)editor_font);
	GetTextExtentPoint32(dc, _T("01235678901234567890123456789012345678901234567890"), 4, &size); 
	int ppi = GetDeviceCaps(dc, LOGPIXELSX);		// convert log units to twips for tabset...
	size.cx = MulDiv(size.cx, 1440, ppi);
	for (int i = 0; i < MAX_TAB_STOPS; i++)
		para_fmt.rgxTabs[i] = size.cx * (i + 1);
	SetMapMode(dc, MM_TEXT);
	m_editor.ReleaseDC(cdc);
	m_editor.SetParaFormat(para_fmt);
	m_editor.SetOptions(ECOOP_OR, ECO_SELECTIONBAR);

	BuildEventList();

	return TRUE;
}

// ============================================================================
void CEventEd::OnSelChangeControl() 
{
	if(m_curSel >= 0)
		m_editor.GetWindowText(m_pCurObj->m_handlers[m_curSel].m_text);

	m_pCurObj = m_pDoc->GetObj(m_cbxControls.GetCurSel());
	m_curSel = -1;
	BuildEventList();
}

// ============================================================================
void CEventEd::OnSelChangeEvent() 
{
	if(m_curSel >= 0)
		m_editor.GetWindowText(m_pCurObj->m_handlers[m_curSel].m_text);

	m_curSel = m_cbxEvents.GetCurSel();
	if(m_curSel >= 0)
	{
		CString hdr, body;
		m_pCurObj->EmitHandler(m_curSel, hdr, body);
		m_stcDeclaration.SetWindowText(hdr);
		m_editor.SetWindowText(body);
		m_editor.SetModify(FALSE);

		if(!m_pCurObj->m_handlers[m_curSel].m_use)
		{
			m_pCurObj->m_handlers[m_curSel].m_use = TRUE;
			m_pDoc->UpdateAllViews(NULL, HINT_CHANGEEVENT, reinterpret_cast<CObject*>(static_cast<ULONG_PTR>(m_curSel)));
		}
	}
}

// ============================================================================
void CEventEd::OnPaint() 
{
	CPaintDC dc(this);
	
	CRect rect;
	GetClientRect(rect);
	HorizIndent(&dc, rect.left, rect.top, rect.right);
}

// ============================================================================
void CEventEd::OnSize(UINT nType, int cx, int cy) 
{
	CRect rect;
	CWnd *ctrl = NULL;

	if((ctrl = GetDlgItem(IDOK)) != NULL) {
		ctrl->GetWindowRect(&rect);
		ctrl->MoveWindow(cx-rect.Width()-c_dlgEdge, cy-rect.Height()-c_dlgEdge, rect.Width(), rect.Height());
	}

	if((ctrl = GetDlgItem(IDC_CONTROL)) != NULL) {
		ctrl->GetWindowRect(&rect);
		ctrl->MoveWindow(c_dlgEdge, cy-rect.Height()-c_dlgEdge+1, rect.Width(), rect.Height());
	}

	if((ctrl = GetDlgItem(IDC_TXT_CONTROL)) != NULL) {
		ctrl->GetWindowRect(&rect);
		ctrl->MoveWindow(c_dlgEdge, cy-36-c_dlgEdge, rect.Width(), rect.Height());
	}

	if((ctrl = GetDlgItem(IDC_EVENT)) != NULL) {
		ctrl->GetWindowRect(&rect);
		ctrl->MoveWindow(c_dlgEdge+159, cy-rect.Height()-c_dlgEdge+1, rect.Width(), rect.Height());
	}

	if((ctrl = GetDlgItem(IDC_TXT_EVENT)) != NULL) {
		ctrl->GetWindowRect(&rect);
		ctrl->MoveWindow(c_dlgEdge+159, cy-36-c_dlgEdge, rect.Width(), rect.Height());
	}

	if((ctrl = GetDlgItem(IDC_EDITOR)) != NULL) {
		ctrl->GetWindowRect(&rect);
		ScreenToClient(&rect);
		ctrl->MoveWindow(rect.left, rect.top, cx-2*c_dlgEdge, cy-64-c_dlgEdge);
	}

	CDialog::OnSize(nType, cx, cy);
}

// ============================================================================
void CEventEd::OnClose() 
{
	SaveWindowPosition(_T("EventEditor"), m_hWnd);
	if(m_editor.GetModify())
	{
		int res = MessageBox(GetString(IDS_SAVE_CHANGES_TO_EVENT), NULL, MB_YESNOCANCEL|MB_ICONEXCLAMATION);
		switch(res)
		{
		case IDYES:
			OnOK();
			return;
		case IDCANCEL:
			return;
		}
	}

	CDialog::OnClose();
}

// ============================================================================
void CEventEd::OnOK() 
{
	if(m_curSel >= 0)
	{
		m_editor.GetWindowText(m_pCurObj->m_handlers[m_curSel].m_text);
		if (m_editor.GetModify())
			m_pDoc->SetModifiedFlag(TRUE);
	}

	CDialog::OnOK();
}

// ============================================================================
void CEventEd::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	lpMMI->ptMinTrackSize.x = 406;
	lpMMI->ptMinTrackSize.y = 200;
	
	CDialog::OnGetMinMaxInfo(lpMMI);
}

// ============================================================================
void CEventEd::OnEditUndo()  { m_editor.Undo(); }
void CEventEd::OnEditCut()   { m_editor.Cut(); }
void CEventEd::OnEditCopy()  { m_editor.Copy(); }
void CEventEd::OnEditPaste() { m_editor.Paste(); }
void CEventEd::OnEditClear() { m_editor.Clear(); }
void CEventEd::OnEditSelectAll() { m_editor.SetSel(0, -1); }
