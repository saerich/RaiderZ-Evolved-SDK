// ============================================================================
// VisualMSDoc.cpp
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#include "stdafx.h"
#include "VisualMS.h"
#include "VisualMSThread.h"
#include "VisualMSDoc.h"
#include "MainFrm.h"
#include "FormEdView.h"
#include "FormEd.h"
#include "GuideDlg.h"
#include "max.h"

#pragma warning(push)
#pragma warning(disable : 4800)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// ============================================================================
void LoadStringArray(CArchive &ar, CStringArray &strArr)
{
	int count;
	ar >> count;
	strArr.SetSize(count);

	for(int i = 0; i < count; i++)
		ar >> strArr[i];
}

// ============================================================================
void SaveStringArray(CArchive &ar, CStringArray &strArr)
{
	int count = strArr.GetSize();
	ar << count;

	for(int i = 0; i < count; i++)
		ar << strArr[i];
}

// ============================================================================
IMPLEMENT_DYNCREATE(CVisualMSDoc, CDocument)
BEGIN_MESSAGE_MAP(CVisualMSDoc, CDocument)
	//{{AFX_MSG_MAP(CVisualMSDoc)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_LAYOUT_ALIGN_LEFT, OnLayoutAlignLeft)
	ON_COMMAND(ID_LAYOUT_ALIGN_RIGHT, OnLayoutAlignRight)
	ON_COMMAND(ID_LAYOUT_ALIGN_TOP, OnLayoutAlignTop)
	ON_COMMAND(ID_LAYOUT_ALIGN_BOTTOM, OnLayoutAlignBottom)
	ON_COMMAND(ID_LAYOUT_ALIGN_HORZCENTER, OnLayoutAlignHorzCenter)
	ON_COMMAND(ID_LAYOUT_ALIGN_VERTCENTER, OnLayoutAlignVertCenter)
	ON_COMMAND(ID_LAYOUT_GUIDE_SETTINGS, OnLayoutGuideSettings)
	ON_COMMAND(ID_LAYOUT_SPACEEVENLY_ACROSS, OnLayoutSpaceEvenlyAcross)
	ON_COMMAND(ID_LAYOUT_SPACEEVENLY_DOWN, OnLayoutSpaceEvenlyDown)
	ON_COMMAND(ID_LAYOUT_MAKESAMESIZE_WIDTH, OnLayoutMakeSameSizeWidth)
	ON_COMMAND(ID_LAYOUT_MAKESAMESIZE_HEIGHT, OnLayoutMakeSameSizeHeight)
	ON_COMMAND(ID_LAYOUT_MAKESAMESIZE_BOTH, OnLayoutMakeSameSizeBoth)
	ON_COMMAND(ID_LAYOUT_CENTERINDIALOG_VERTICAL, OnLayoutCenterInDialogVertical)
	ON_COMMAND(ID_LAYOUT_CENTERINDIALOG_HORIZONTAL, OnLayoutCenterInDialogHorizontal)
	ON_COMMAND(ID_LAYOUT_FLIP, OnLayoutFlip)
	ON_COMMAND(ID_EDIT_MOVELEFT, OnEditMoveLeft)
	ON_COMMAND(ID_EDIT_MOVERIGHT, OnEditMoveRight)
	ON_COMMAND(ID_EDIT_MOVEUP, OnEditMoveUp)
	ON_COMMAND(ID_EDIT_MOVEDOWN, OnEditMoveDown)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ============================================================================
CVisualMSDoc* CVisualMSDoc::GetDoc()
{
	CVisualMSThread *pThread = CVisualMSThread::GetRunningThread();
	if(pThread) return pThread->GetDoc();
	return NULL;
}

// ============================================================================
CVisualMSDoc::CVisualMSDoc()
{
	m_editCallback = NULL;
	m_creating = false;
}

// ============================================================================
CVisualMSDoc::~CVisualMSDoc()
{
}

// ============================================================================
void CVisualMSDoc::SetTitle(LPCTSTR lpszTitle)
{
	m_sTitle = lpszTitle;

	m_strTitle = m_sTitle + _T(" - ") + GetString(IDS_APP_TITLE);
	CMainFrame::GetFrame()->SetWindowText(m_strTitle);
}

// ============================================================================
void CVisualMSDoc::SetFileName(LPCTSTR fname)
{
	m_sFileName = fname;
}

// ============================================================================
const CString& CVisualMSDoc::GetFileName()
{
	if(m_sFileName.IsEmpty())
		return GetFormObj()->Name();
	return m_sFileName;
}

// ============================================================================
CView* CVisualMSDoc::GetFormView()
{
	POSITION pos = GetFirstViewPosition();
	while(pos != NULL)
	{
		CView* pView = GetNextView(pos);
		if(pView->IsKindOf(RUNTIME_CLASS(CFormEdView)))
			return pView;
	}   
	return NULL;
}

// ============================================================================
CView* CVisualMSDoc::GetPropView()
{
	return NULL;
}

// ============================================================================
CView* CVisualMSDoc::GetEventView()
{
	return NULL;
}

// ============================================================================
#ifdef _DEBUG
void CVisualMSDoc::AssertValid() const
{
	CDocument::AssertValid();
}
void CVisualMSDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

// ============================================================================
void CVisualMSDoc::SetCreateMode(UINT createMode)
{
	m_createMode = createMode;
	CMainFrame *pFrameWnd= (CMainFrame*)AfxGetMainWnd();
	pFrameWnd->PressCtrlButton(m_createMode);
}
UINT CVisualMSDoc::GetCreateMode() { return m_createMode; }

// ============================================================================
void CVisualMSDoc::SetUseGrid(bool bUseGrid)
{
	if(bUseGrid != m_bUseGrid)
	{
		m_bUseGrid = bUseGrid;
		Invalidate();
	}
}
bool CVisualMSDoc::GetUseGrid() { return m_bUseGrid; }

// ============================================================================
void CVisualMSDoc::SetGridSpacing(int gridSpacing)
{
	m_gridSpacing = gridSpacing;
}
int CVisualMSDoc::GetGridSpacing() { return m_gridSpacing; }

// ============================================================================
void CVisualMSDoc::SetAlignOffset(int alignOffset) { m_alignOffset = alignOffset; }
int  CVisualMSDoc::GetAlignOffset() { return m_alignOffset; }

// ============================================================================
int CVisualMSDoc::GetSize() { return m_objects.GetSize(); }

// ============================================================================
int CVisualMSDoc::AddObj(CGuiObj *pObj)
{
	ASSERT(pObj != NULL);
	pObj->m_sizer.m_pObj = pObj;

	SetModifiedFlag(TRUE);
	return m_objects.Add(pObj);
}

// ============================================================================
int CVisualMSDoc::InsertObj(int insertAt, CGuiObj *pObj)
{
	ASSERT(pObj != NULL);
	pObj->m_sizer.m_pObj = pObj;

	SetModifiedFlag(TRUE);
	m_objects.InsertAt(insertAt, pObj);
	return insertAt;
}

// ============================================================================
void CVisualMSDoc::RemoveObj(int idx)
{
	ASSERT(idx >= 0 && idx < m_objects.GetSize());
	CGuiObj::Destroy((CGuiObj*)m_objects[idx]);
	m_objects.RemoveAt(idx);
	SetModifiedFlag(TRUE);
}

// ============================================================================
void CVisualMSDoc::SetObj(int idx, CGuiObj *obj)
{
	ASSERT(idx >= 0 && idx < m_objects.GetSize());
	m_objects[idx] = obj;
	SetModifiedFlag(TRUE);
}

// ============================================================================
CGuiObj* CVisualMSDoc::GetObj(int idx)
{
	if(idx >= 0 && idx < m_objects.GetSize())
		return (CGuiObj*)m_objects[idx];
	return NULL;
}

// ============================================================================
CFormObj* CVisualMSDoc::GetFormObj()
{
	CGuiObj *pObj = (CGuiObj*)m_objects[0];
	ASSERT(pObj != NULL);
	return (CFormObj*)pObj;
}

// ============================================================================
int CVisualMSDoc::HitTest(int start, const CRect &rect)
{
	// first go through selected objects
	for(int i = start+1; i < m_objects.GetSize(); i++)
	{
		CGuiObj *obj = (CGuiObj*)m_objects[i];
		if(obj->IsSelected() && obj->HitTest(rect))
			return i;
	}

	// then go through un-selected objects
	for(int i = start+1; i < m_objects.GetSize(); i++)
	{
		CGuiObj *obj = (CGuiObj*)m_objects[i];
		if(obj->IsSelected() == false && obj->HitTest(rect))
			return i;
	}
	return 0;
}

// ============================================================================
int CVisualMSDoc::GetNextSel(int start /*=-1*/, bool bSel /*= true*/)
{
	for(int i = start+1; i < m_objects.GetSize(); i++)
	{
		if(IsSelected(i) == bSel)
			return i;
	}
	return -1;
}

// ============================================================================
void CVisualMSDoc::GetSelectionSet(CUIntArray &arr)
{
	arr.RemoveAll();

	for(int i = 0; i < m_objects.GetSize(); i++)
	{
		if(IsSelected(i))
			arr.Add(i);
	}
}

// ============================================================================
int CVisualMSDoc::GetSelectionCount()
{
	int count = 0;
	for(int i = 0; i < m_objects.GetSize(); i++)
	{
		if(IsSelected(i))
			count++;
	}
	return count;
}

// ============================================================================
void CVisualMSDoc::GetSelectionRect(CRect &rect) 
{
	rect.left = rect.top = 65535;
	rect.right = rect.bottom = -65536;

	int i = GetNextSel(0);
	while(i >= 0)
	{
		CGuiObj *pObj = GetObj(i);

		CRect r;
		pObj->GetRect(r);
		r.right += r.left;
		r.bottom += r.top;

		if(r.left < rect.left) rect.left = r.left;
		if(r.top < rect.top) rect.top = r.top;
		if(r.right > rect.right) rect.right = r.right;
		if(r.bottom > rect.bottom) rect.bottom = r.bottom;

		i = GetNextSel(i);
	}
}

// ============================================================================
void CVisualMSDoc::GetFormSize(CSize &size)
{
	CRect rect;
	GetObj(0)->GetRect(rect);
	size.cx = rect.right;
	size.cy = rect.bottom;
}

// ============================================================================
void CVisualMSDoc::Select(int idx)
{
	ASSERT(idx >= 0 && idx < m_objects.GetSize());
	CGuiObj *pObj = (CGuiObj*)m_objects[idx];
	pObj->m_selected = true;
	if(idx > 0)
		pObj->MoveToTop(&GetFormObj()->m_formEd.m_formTracker);//&CWnd::wndTopMost);//
	m_curSelBase = idx;
}

// ============================================================================
bool CVisualMSDoc::IsSelected(int idx)
{
	ASSERT(idx >= 0 && idx < m_objects.GetSize());
	return ((CGuiObj*)m_objects[idx])->IsSelected();
}

// ============================================================================
void CVisualMSDoc::DeSelect(int idx)
{
	ASSERT(idx >= 0 && idx < m_objects.GetSize());
	((CGuiObj*)m_objects[idx])->m_selected = false;

	if(idx == m_curSelBase)
		m_curSelBase = GetNextSel(0);
}

// ============================================================================
void CVisualMSDoc::SelectAll()
{
	CGuiObj *obj = (CGuiObj*)m_objects[0];
	obj->m_selected = FALSE;

	for(int i = 1; i < m_objects.GetSize(); i++)
		Select(i);
}

// ============================================================================
void CVisualMSDoc::DeSelectAll()
{
	for(int i = 0; i < m_objects.GetSize(); i++)
	{
		CGuiObj *obj = (CGuiObj*)m_objects[i];
		obj->m_selected = FALSE;
	}
}

// ============================================================================
void CVisualMSDoc::Invalidate(int idx)
{
	ASSERT(idx >= 0 && idx < m_objects.GetSize());
	((CGuiObj*)m_objects[idx])->Invalidate();
}

// ============================================================================
void CVisualMSDoc::UpdateAllViews(CView* pSender, LPARAM lHint, CObject* pHint)
{
//	if (!m_creating)
		CDocument::UpdateAllViews(pSender, lHint, pHint);
}

// ============================================================================
void CVisualMSDoc::OnEditCut() 
{
	OnEditCopy();
	OnEditClear();
}

// ============================================================================
void CVisualMSDoc::OnEditCopy() 
{
	static TCHAR fname[260];
	GetTempPath(sizeof(fname), fname);
	strcat(fname, _T("~TMPBUF.VMS"));
	SaveVMS(fname, true);
}

// ============================================================================
void CVisualMSDoc::OnEditPaste() 
{
	static TCHAR fname[260];
	GetTempPath(sizeof(fname), fname);
	strcat(fname, _T("~TMPBUF.VMS"));
	MergeVMS(fname, true);
}

// ============================================================================
void CVisualMSDoc::OnEditClear() 
{
	int i = GetNextSel(0);
	while(i >= 0)
	{
		DeSelect(i);
		RemoveObj(i);
		i = GetNextSel(i-1);
	}
	Select(0);
	Invalidate();
	SetModifiedFlag(TRUE);
	UpdateAllViews(NULL, HINT_CHANGESEL, (CObject*)GetFormObj());
}

// ============================================================================
bool CVisualMSDoc::CheckScriptLinkage() 
{
	if(m_editCallback != NULL)
	{
		if(YesNoBoxf(GetString(IDS_ASK_TO_UNLINK)) == IDYES)
		{
			m_editCallback->Close();
			SetEditCallback(NULL);
		}
		else
			return FALSE;
	}
	return TRUE;
}

// ============================================================================
void CVisualMSDoc::DeleteContents() 
{
	for(int i = 0; i < m_objects.GetSize(); i++)
		CGuiObj::Destroy((CGuiObj*)m_objects[i]);
	m_objects.RemoveAll();

	CDocument::DeleteContents();
}

// ============================================================================
BOOL CVisualMSDoc::DoFileSave()
{
	if(m_editCallback != NULL)
	{
		m_editCallback->Save();
		SetModifiedFlag(FALSE);
	}
	else if(m_sFileName.IsEmpty())
	{
		OnFileSaveAs();
	}
	else
	{
		SaveFile(m_sFileName);
		SetModifiedFlag(FALSE);
	}

	return TRUE;
}

// ============================================================================
BOOL CVisualMSDoc::SaveModified() 
{
	if(!IsModified())
		return TRUE;

	CString str;
	str.Format(IDS_SAVE_CHANGES_TO, GetFileName());
	int res = ::MessageBox(
		GetCOREInterface()->GetMAXHWnd(),
		(LPCTSTR)str,
		GetString(IDS_APP_TITLE),
		MB_YESNOCANCEL|MB_ICONEXCLAMATION);

//	int res = YesNoCancelBoxf(GetString(IDS_SAVE_CHANGES_TO), GetFileName());

	switch(res)
	{
	case IDYES:
		DoFileSave();
		break;
	case IDCANCEL:
		return FALSE;
	}
	return TRUE;
}

// ============================================================================
BOOL CVisualMSDoc::OnNewDocument()
{
	if(!CheckScriptLinkage())
		return FALSE;

	if(!CDocument::OnNewDocument())
		return FALSE;

	if(m_objects.GetSize() > 0)
		DeleteContents();

	SetCreateMode(ID_CTRL_SELECT);

	// TODO: LOAD THESE FROM INI FILE !
	m_bUseGrid = false;
	m_gridSpacing = 8;
	m_alignOffset = 8;

	CFormObj *formObj = new CFormObj(GetFormView(), CRect(0,0,162,300));
	ASSERT(formObj != NULL);

	formObj->m_createID = 0;
	Select(AddObj(formObj));
	UpdateAllViews(NULL, HINT_CHANGESEL, (CObject*)formObj);

	SetTitle(formObj->Name());
	SetFileName("");
	SetModifiedFlag(FALSE);

	return TRUE;
}

// ============================================================================
void CVisualMSDoc::OnCloseDocument()
{
	if(m_editCallback != NULL)
	{
		m_editCallback->Close();
		SetEditCallback(NULL);
	}
	SetModifiedFlag(FALSE);
}

// ============================================================================
BOOL CVisualMSDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	return TRUE;
}

// ============================================================================
void CVisualMSDoc::OnFileNew() 
{
	if(!SaveModified())
		return;
	OnNewDocument();
}

// ============================================================================
void CVisualMSDoc::OnFileOpen() 
{
	if(!SaveModified())
		return;

	if(!CheckScriptLinkage())
		return;

	CFileDialog dlg(TRUE, NULL, m_sFileName,
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ENABLESIZING,
		"Visual MAXScript Files (*.vms)|*.vms|All Files (*.*)|*.*||");

	if(dlg.DoModal() == IDOK)
		LoadFile(dlg.GetPathName());
}

// ============================================================================
void CVisualMSDoc::OnFileSave() 
{
	DoFileSave();
}

// ============================================================================
void CVisualMSDoc::OnFileSaveAs() 
{
	if(!CheckScriptLinkage())
		return;

	CFileDialog dlg(FALSE, "*.ms", NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ENABLESIZING,
		"Visual MAXScript Files (*.vms)|*.vms|MAXScript Files (*.ms;*.mcr)|*.ms; *.mcr|All Files (*.*)|*.*||");

	if(dlg.DoModal() == IDOK)
	{
		SaveFile(dlg.GetPathName());
		SetModifiedFlag(FALSE);
	}
}

// ============================================================================
bool CVisualMSDoc::SaveFile(const CString &fname)
{
	bool res = false;

	CString ext = fname.Mid(fname.ReverseFind('.'));
	if(!ext.CompareNoCase(".ms") || !ext.CompareNoCase(".mcr"))
		res = ExportScript(fname);
	else if(!ext.CompareNoCase(".vms"))
		res = SaveVMS(fname);

	if(res)
	{
		SetFileName(fname);
		SetModifiedFlag(FALSE);
	}

	return res;
}

// ============================================================================
bool CVisualMSDoc::LoadFile(const CString &fname)
{
	if(LoadVMS(fname))
	{
		SetFileName(fname);
		SetModifiedFlag(FALSE);
		return true;
	}

	return false;
}

// ============================================================================
bool CVisualMSDoc::ExportScript(const CString &fname)
{
	FILE *stream = fopen(fname, "wt");
	if(!stream)
	{
		MessageBoxf(GetString(IDS_CANT_OPEN__FOR_WRITING), fname);
		return FALSE;
	}

	CString out;
	if (GenScript(out))
	{
		fprintf(stream, (LPCTSTR)out);
		fclose(stream);
		return TRUE;
	}
	else
		return FALSE;
}

// ============================================================================

static int InsertTabs(CString &str, int numTabs, const TCHAR* indent, bool& bracketed)
{
	CString tabs = indent ? indent : _T("");
	for(int i = 0; i < numTabs; i++)
		tabs += _T("\t");

	str.TrimRight();
	str = tabs + str;
	bool sol = true; int line = 0;
	bracketed = false;
	for (int i = 0; i < str.GetLength(); i++)
	{
		if (str[i] == _T('\n') || str[i] == _T('\r'))
		{
			while (i < str.GetLength()-1 && str[++i] == _T('\n'));
			if (i < str.GetLength())
			{
				str.Insert(i, tabs);
				i += tabs.GetLength();
			}
			sol = true;
			continue;
		}
		if (str[i] == _T('(') && sol)
			bracketed = true;
		if (!isspace(str[i]) && sol)
			line++, sol = false;
	}
	return line;
}

static bool at_newline(CString& str)
{
	TCHAR c = str[str.GetLength()-1];
	return c == _T('\n') || c == _T('\r');
}

static bool at_spaced_newline(CString& str)
{
	int line = 0; TCHAR c;
	for (int i = str.GetLength()-1; i > 0; --i)
		if ((c = str[i]) == _T('\r')) continue;
		else if (c == _T('\n')) line++;
		else if (!isspace(c)) break;
	return line > 1;
}

bool CVisualMSDoc::GenScript(CString& script, TCHAR* indentp)
{
	CString tmp;
	CString indent;
	if (indentp != NULL) indent = indentp;
	CGuiObj *obj = (CGuiObj*)m_objects[0];
	CRect formSize;
	obj->GetRect(formSize);
	script.Format(_T("%s%s %s \"%s\" width:%d height:%d\n%s(\n"), (LPCTSTR)indent,
		(LPCTSTR)obj->Class(),
		(LPCTSTR)obj->Name(),
		(LPCTSTR)obj->Caption(),
		formSize.right,
		formSize.bottom, (LPCTSTR)indent);

	bool last_item_code;
	bool first = true, first_code = false;
	for (int i = 1; i < m_objects.GetSize(); i++)
	{
		last_item_code = false;
		obj = (CGuiObj*)m_objects[i];
		tmp = "";
		obj->Emit(tmp);
		if(tmp == "" || tmp.IsEmpty()) continue;

		if (obj->m_createID == ID_CODE)
		{
			if (!at_newline(script)) script += _T("\n");
//			if (first_code) script += "\n";
			script += tmp;
			last_item_code = true;
			first_code = false;
		}
		else
		{
//			if (!first) script += "\n";
			if (!at_newline(script)) script += _T("\n");
			script += indent + "\t" + tmp;
			first = false; first_code = true;
		}
	}

	first = true; bool bracketed; int lines;
	for (int i = 0; i < m_objects.GetSize(); i++)
	{
		obj = (CGuiObj*)m_objects[i];
		for (int j = 0; j < obj->m_handlers.GetSize(); j++)
		{
			if (obj->m_handlers[j].m_use)
			{
				if (!at_newline(script)) script += _T("\n");
				else if (!at_spaced_newline(script)) script += _T("\n");
				CString hdr, body;
				obj->EmitHandler(j, hdr, body);
				script += indent + _T("\t") + hdr + _T("\n");// + indent;
				body.TrimLeft();
				lines = InsertTabs(body, 1, indent, bracketed);
				if (lines == 1) script += _T("\t") + body;
				else if (!bracketed)
				{
					InsertTabs(body, 1, NULL, bracketed);
					script += indent + _T("(\n") + body + _T("\n") + indent + _T("\t)");
				}
				else
					script += body;
				first = false;
				last_item_code = false;
			}
		}
	}
	if (!at_newline(script)) script += _T("\n");
	script += indent + ")";

/*
	obj = (CGuiObj*)m_objects[0];
	CRect formSize;
	obj->GetRect(formSize);
	tmp.Format("nrf = NewRolloutFloater \"\" %d %d\nAddRollout %s nrf\n",
		formSize.right+30, formSize.bottom+50, (LPCTSTR)obj->Name());
	script += tmp;
*/

	return TRUE;
}

// ============================================================================
bool CVisualMSDoc::SaveVMS(const CString &fname, bool bSelOnly)
{
	CFile f;
	if(!f.Open(fname, CFile::modeCreate | CFile::modeWrite))
	{
		MessageBoxf(GetString(IDS_ERROR_SAVING_FILE_), (LPCTSTR)fname);
		return false;
	}
	CArchive ar(&f, CArchive::store);

	VmsHeader hdr;
	hdr.magic = VMS_MAGIC;
	hdr.version = VMS_VERSION;

	CUIntArray selArr;

	if(bSelOnly)
	{
		GetSelectionSet(selArr);
		hdr.numObj = GetSelectionCount();
	}
	else
		hdr.numObj = m_objects.GetSize();

	// store the whole header with one write
	ar.Write(&hdr, sizeof(VmsHeader));

	for(int i = 0; i < hdr.numObj; i++)
	{
		CGuiObj *obj;
		if(bSelOnly)
			obj = (CGuiObj*)m_objects[selArr[i]];
		else
			obj = (CGuiObj*)m_objects[i];

		ar << obj->m_createID;

		int numProps = obj->m_properties.GetSize();
		ar << numProps;

		for(int j = 0; j < numProps; j++)
		{
			CProperty *prop = &obj->m_properties[j];
			ar << prop->m_name;
			ar << prop->m_type;
			ar << prop->m_flags;

			switch(prop->m_type)
			{
			case PROP_INTEGER:
			case PROP_OPTION:
				ar << prop->m_integer;
				break;
			case PROP_BOOLEAN:
				ar << (BOOL)prop->m_boolean;
				break;
			case PROP_REAL:
				ar << prop->m_real;
				break;
			case PROP_ARRAY:
				SaveStringArray(ar, prop->m_array);
				break;
			case PROP_STRING:
			case PROP_FILENAME:
				ar << prop->m_string;
				break;

			// if the property type is not supported by the VMS file I/O
			// write it as a string
			default:
				{
					CString str = (LPCTSTR)*prop;
					ar << str;
				}
				break;
			}
		}

		int numEvents = obj->m_handlers.GetSize();
		ar << numEvents;

		for(int j = 0; j < numEvents; j++)
		{
			CHandler *hndlr = &obj->m_handlers[j];
			ar << (BOOL)hndlr->m_use;
			ar << hndlr->m_name;
			ar << hndlr->m_args;
			ar << hndlr->m_text;
		}
	}

	// close the archive and file handle
	ar.Close();
	f.Close();

	return true;
}

// ============================================================================
bool CVisualMSDoc::LoadVMS(const CString &fname)
{
	DeleteContents();
	OnNewDocument();

	return MergeVMS(fname);
}

// ============================================================================
bool CVisualMSDoc::MergeVMS(const CString &fname, bool bPaste)
{
	CFile f;
	if(f.Open(fname, CFile::modeRead) == FALSE)
	{
		MessageBoxf(GetString(IDS_ERROR_LOADING_FILE_), (LPCTSTR)fname);
		return false;
	}
	DeSelectAll();
	CArchive ar(&f, CArchive::load);

	VmsHeader hdr;
	ar.Read(&hdr, sizeof(VmsHeader));

	if(hdr.magic != VMS_MAGIC)
	{
		MessageBoxf(GetString(IDS__IS_NOT_A_VALID_VISUAL_MAXSCRIPT_BINARY), (LPCTSTR)fname);
		ar.Close();
		f.Close();
		return false;
	}

	if(hdr.version != VMS_VERSION)
	{
		MessageBoxf(GetString(IDS__HAS_AN_UNSUPPORTED_FILE_VERSION), (LPCTSTR)fname);
		ar.Close();
		f.Close();
		return false;
	}

	CFormEd *formEd = &GetFormObj()->m_formEd;

	for(int i = 0; i < hdr.numObj; i++)
	{
		UINT nID;
		int numProps;

		ar >> nID;
		ar >> numProps;

		CGuiObj *obj = NULL;
		if(nID == 0)
			obj = (CGuiObj*)m_objects[0];
		else
		{
			obj = CGuiObj::Create(nID, formEd, CRect(0,0,60,40));
			if(obj)
			{
				obj->m_selected = FALSE;
				AddObj(obj);
			}
		}

		if(numProps > obj->m_properties.GetSize())
		{
		}

		for(int j = 0; j < numProps; j++)
		{
			// Its possible for there to be more properties then the defualt VMS
			// constructors added so dynamically add these
			if(j >= obj->m_properties.GetSize())
				obj->m_properties.Add(CProperty(_T("dynamic"), CString("")));

			// if were loading the form and its a paste operation
			// dont paste the forms properties, put in a dummy
			CProperty dummyProp;
			CProperty *prop = NULL;
			if(bPaste && nID == 0)
				prop = &dummyProp;
			else
				prop = &obj->m_properties[j];

			ar >> prop->m_name;
			ar >> prop->m_type;
			ar >> prop->m_flags;

			switch(prop->m_type)
			{
			case PROP_STRING:
				// if pasting we want to use the default control names
				if(bPaste && j == IDX_NAME)
				{
					CString name;
					ar >> name;
				}
				else
					ar >> prop->m_string;
				break;
			case PROP_INTEGER:
			case PROP_OPTION:
				ar >> prop->m_integer;
				break;
			case PROP_BOOLEAN:
				{
					BOOL b;
					ar >> b;
					prop->m_boolean = (bool)b;
					break;
				}
			case PROP_REAL:
				ar >> prop->m_real;
				break;
			case PROP_ARRAY:
				LoadStringArray(ar, prop->m_array);
				break;
			case PROP_FILENAME:
				ar >> prop->m_string;
				break;

			// if the property type is not supported by the VMS file I/O
			// read it as a string
			default:
				{
					CString str;
					ar >> str;
					*prop = (LPCTSTR)str;
				}
				break;
			}

			obj->PropChanged(j);
		}

		// if pasting offset position by [10,10] and select
		if(bPaste)
		{
			CRect rect;
			obj->GetRect(rect);
			rect.left += 10;
			rect.top += 10;
			obj->SetRect(rect);
			obj->m_selected = true;
		}
		obj->Invalidate();

		int numEvents;
		ar >> numEvents;

		if(numEvents > obj->m_handlers.GetSize())
		{

		}

		for(int j = 0; j < numEvents; j++)
		{
			// Its possible for there to be more handlers then the defualt VMS
			// constructors added so dynamically add these
			if(j >= obj->m_handlers.GetSize())
				obj->m_handlers.Add(CHandler(false, _T("dynamic"), _T(" do"), _T("")));

			// if were loading the form and its a paste operation
			// dont paste the forms handlers, put in a dummy
			CHandler dummyHndlr;
			CHandler *hndlr = NULL;
			if(bPaste && nID == 0)
				hndlr = &dummyHndlr;
			else
				hndlr = &obj->m_handlers[j];

			BOOL b;
			ar >> b;
			hndlr->m_use = (bool)b;

			ar >> hndlr->m_name;
			ar >> hndlr->m_args;
			ar >> hndlr->m_text;
		}
	}

	ar.Close();
	f.Close();
	Invalidate();

	return true;
}

// ============================================================================
void CVisualMSDoc::OnLayoutGuideSettings() 
{
	CGuideDlg dlg(GetUseGrid(), GetGridSpacing(), GetAlignOffset());
	if(dlg.DoModal())
	{
		SetUseGrid(dlg.m_useGrid);
		SetGridSpacing(dlg.m_gridSpacing);
		SetAlignOffset(dlg.m_alignOffset);
		Invalidate();
	}
}

// ============================================================================
void CVisualMSDoc::OnLayoutAlignLeft() 
{
	if(m_curSelBase <= 0 || m_curSelBase >= GetSize())
		return;

	CRect rect;
	CGuiObj *pObj = GetObj(m_curSelBase);
	pObj->GetRect(rect);
	int left = rect.left;

	int i = GetNextSel(0);
	while(i >= 0)
	{
		pObj = GetObj(i);
		pObj->GetRect(rect);
		rect.left = left;
		pObj->SetRect(rect);
		i = GetNextSel(i);
	}
	Invalidate();
	SetModifiedFlag(TRUE);
}

// ============================================================================
void CVisualMSDoc::OnLayoutAlignRight() 
{
	if(m_curSelBase <= 0 || m_curSelBase >= GetSize())
		return;

	CRect rect;
	CGuiObj *pObj = GetObj(m_curSelBase);
	pObj->GetRect(rect);
	int right = rect.left + rect.right;

	int i = GetNextSel(0);
	while(i >= 0)
	{
		pObj = GetObj(i);
		pObj->GetRect(rect);
		rect.left = right - rect.right;
		pObj->SetRect(rect);
		i = GetNextSel(i);
	}
	Invalidate();
	SetModifiedFlag(TRUE);
}

// ============================================================================
void CVisualMSDoc::OnLayoutAlignTop() 
{
	if(m_curSelBase <= 0 || m_curSelBase >= GetSize())
		return;

	CRect rect;
	CGuiObj *pObj = GetObj(m_curSelBase);
	pObj->GetRect(rect);
	int top = rect.top;

	int i = GetNextSel(0);
	while(i >= 0)
	{
		pObj = GetObj(i);
		pObj->GetRect(rect);
		rect.top = top;
		pObj->SetRect(rect);
		i = GetNextSel(i);
	}
	Invalidate();
	SetModifiedFlag(TRUE);
}

// ============================================================================
void CVisualMSDoc::OnLayoutAlignBottom() 
{
	if(m_curSelBase <= 0 || m_curSelBase >= GetSize())
		return;

	CRect rect;
	CGuiObj *pObj = GetObj(m_curSelBase);
	pObj->GetRect(rect);
	int bottom = rect.top + rect.bottom;

	int i = GetNextSel(0);
	while(i >= 0)
	{
		pObj = GetObj(i);
		pObj->GetRect(rect);
		rect.top = bottom - rect.bottom;
		pObj->SetRect(rect);
		i = GetNextSel(i);
	}
	Invalidate();
	SetModifiedFlag(TRUE);
}

// ============================================================================
void CVisualMSDoc::OnLayoutAlignHorzCenter() 
{
	if(m_curSelBase <= 0 || m_curSelBase >= GetSize())
		return;

	CRect rect;
	int horzCenter = 0;

	CGuiObj *pObj = GetObj(m_curSelBase);
	ASSERT(pObj != NULL);
	pObj->GetRect(rect);
	horzCenter = rect.top + (rect.bottom / 2);

	int idx = GetNextSel(0);
	while(idx >= 0)
	{
		pObj = GetObj(idx);
		ASSERT(pObj != NULL);
		pObj->GetRect(rect);

		rect.top = horzCenter - (rect.bottom / 2);
		pObj->SetRect(rect);

		idx = GetNextSel(idx);
	}
	Invalidate();
	SetModifiedFlag(TRUE);
}

// ============================================================================
void CVisualMSDoc::OnLayoutAlignVertCenter() 
{
	if(m_curSelBase <= 0 || m_curSelBase >= GetSize())
		return;

	CRect rect;
	int vertCenter = 0;

	CGuiObj *pObj = GetObj(m_curSelBase);
	ASSERT(pObj != NULL);
	pObj->GetRect(rect);
	vertCenter = rect.left + (rect.right / 2);

	int idx = GetNextSel(0);
	while(idx >= 0)
	{
		pObj = GetObj(idx);
		ASSERT(pObj != NULL);
		pObj->GetRect(rect);

		rect.left = vertCenter - (rect.right / 2);
		pObj->SetRect(rect);

		idx = GetNextSel(idx);
	}
	Invalidate();
	SetModifiedFlag(TRUE);
}

// ============================================================================
void CVisualMSDoc::OnLayoutSpaceEvenlyDown() 
{
	if(GetSelectionCount() <= 1)
		return;

	CUIntArray idxArr, posArr;
	int topIdx, bottomIdx;
	int top = 65535;
	int bottom = -65536;
	int height = 0;

	// Get the top and bottom most objects selected.
	int i = GetNextSel(0);
	while(i >= 0)
	{
		idxArr.Add(i);
		CGuiObj *pObj = GetObj(i);

		CRect r;
		pObj->GetRect(r);
		height += r.bottom;
		r.bottom += r.top;
		posArr.Add(r.top);

		if(r.top < top)
		{
			top = r.top;
			topIdx = i;
		}
		if(r.bottom > bottom)
		{
			bottom = r.bottom;
			bottomIdx = i;
		}

		i = GetNextSel(i);
	}

	int selCount = idxArr.GetSize();

	// Bubble sort the indices
	for(i = 0; i < selCount; i++)
	{
		for(int j = i+1; j < selCount; j++)
		{
			if(posArr[j] < posArr[i])
			{
				int tmp = posArr[i];
				posArr[i] = posArr[j];
				posArr[j] = tmp;
				tmp = idxArr[i];
				idxArr[i] = idxArr[j];
				idxArr[j] = tmp;
			}
		}
	}

	// spacing = (distance - total_height) / object_count
	int spacing = ((bottom-top) - height) / (selCount-1);

	int yPos = top;
	for(i = 0; i < selCount; i++, yPos += spacing)
	{
		CGuiObj *pObj = GetObj(idxArr[i]);
		CRect r;
		pObj->GetRect(r);
		r.top = yPos;
		pObj->SetRect(r);
		yPos += r.bottom;
	}

	Invalidate();
	SetModifiedFlag(TRUE);
}

// ============================================================================
void CVisualMSDoc::OnLayoutSpaceEvenlyAcross() 
{
	if(GetSelectionCount() <= 1)
		return;

	CUIntArray idxArr, posArr;
	int leftIdx, rightIdx;
	int left = 65535;
	int right = -65536;
	int width = 0;

	// Get the left and right most objects selected.
	int i = GetNextSel(0);
	while(i >= 0)
	{
		idxArr.Add(i);
		CGuiObj *pObj = GetObj(i);

		CRect r;
		pObj->GetRect(r);
		width += r.right;
		r.right += r.left;
		posArr.Add(r.left);

		if(r.left < left)
		{
			left = r.left;
			leftIdx = i;
		}
		if(r.right > right)
		{
			right = r.right;
			rightIdx = i;
		}

		i = GetNextSel(i);
	}

	int selCount = idxArr.GetSize();

	// Bubble sort the indices
	for(i = 0; i < selCount; i++)
	{
		for(int j = i+1; j < selCount; j++)
		{
			if(posArr[j] < posArr[i])
			{
				int tmp = posArr[i];
				posArr[i] = posArr[j];
				posArr[j] = tmp;
				tmp = idxArr[i];
				idxArr[i] = idxArr[j];
				idxArr[j] = tmp;
			}
		}
	}

	// spacing = (distance - total_height) / object_count
	int spacing = ((right-left) - width) / (selCount-1);

	int xPos = left;
	for(i = 0; i < selCount; i++, xPos += spacing)
	{
		CGuiObj *pObj = GetObj(idxArr[i]);
		CRect r;
		pObj->GetRect(r);
		r.left = xPos;
		pObj->SetRect(r);
		xPos += r.right;
	}

	Invalidate();
	SetModifiedFlag(TRUE);
}

// ============================================================================
void CVisualMSDoc::OnLayoutMakeSameSizeWidth() 
{
	if(m_curSelBase <= 0 || m_curSelBase >= GetSize())
		return;

	CRect rect;
	CGuiObj *pObj = GetObj(m_curSelBase);
	ASSERT(pObj != NULL);
	pObj->GetRect(rect);
	int width = rect.right;

	int idx = GetNextSel(0);
	while(idx >= 0)
	{
		pObj = GetObj(idx);
		ASSERT(pObj != NULL);
		pObj->GetRect(rect);
		rect.right = width;
		pObj->SetRect(rect);

		idx = GetNextSel(idx);
	}
	Invalidate();
	SetModifiedFlag(TRUE);
}

// ============================================================================
void CVisualMSDoc::OnLayoutMakeSameSizeHeight() 
{
	if(m_curSelBase <= 0 || m_curSelBase >= GetSize())
		return;

	CRect rect;
	CGuiObj *pObj = GetObj(m_curSelBase);
	ASSERT(pObj != NULL);
	pObj->GetRect(rect);
	int height = rect.bottom;

	int idx = GetNextSel(0);
	while(idx >= 0)
	{
		pObj = GetObj(idx);
		ASSERT(pObj != NULL);
		pObj->GetRect(rect);
		rect.bottom = height;
		pObj->SetRect(rect);

		idx = GetNextSel(idx);
	}
	Invalidate();
	SetModifiedFlag(TRUE);
}

// ============================================================================
void CVisualMSDoc::OnLayoutMakeSameSizeBoth() 
{
	if(m_curSelBase <= 0 || m_curSelBase >= GetSize())
		return;

	CRect rect;
	CGuiObj *pObj = GetObj(m_curSelBase);
	ASSERT(pObj != NULL);
	pObj->GetRect(rect);
	int width  = rect.right;
	int height = rect.bottom;

	int idx = GetNextSel(0);
	while(idx >= 0)
	{
		pObj = GetObj(idx);
		ASSERT(pObj != NULL);
		pObj->GetRect(rect);
		rect.right  = width;
		rect.bottom = height;
		pObj->SetRect(rect);

		idx = GetNextSel(idx);
	}
	Invalidate();
	SetModifiedFlag(TRUE);
}

// ============================================================================
void CVisualMSDoc::OnLayoutCenterInDialogVertical() 
{
	CRect rect;
	GetSelectionRect(rect);
	rect.bottom -= rect.top;

	CSize size;
	GetFormSize(size);
	int offset = (size.cy - rect.bottom) / 2 - rect.top;

	int idx = GetNextSel(0);
	while(idx >= 0)
	{
		CGuiObj *pObj = GetObj(idx);
		ASSERT(pObj != NULL);

		pObj->GetRect(rect);
		rect.top += offset;
		pObj->SetRect(rect);

		idx = GetNextSel(idx);
	}
	Invalidate();
	SetModifiedFlag(TRUE);
}

// ============================================================================
void CVisualMSDoc::OnLayoutCenterInDialogHorizontal() 
{
	CRect rect;
	GetSelectionRect(rect);
	rect.right -= rect.left;

	CSize size;
	GetFormSize(size);
	int offset = (size.cx - rect.right) / 2 - rect.left;

	int idx = GetNextSel(0);
	while(idx >= 0)
	{
		CGuiObj *pObj = GetObj(idx);
		ASSERT(pObj != NULL);

		pObj->GetRect(rect);
		rect.left += offset;
		pObj->SetRect(rect);

		idx = GetNextSel(idx);
	}
	Invalidate();
	SetModifiedFlag(TRUE);
}

// ============================================================================
void CVisualMSDoc::OnLayoutFlip() 
{
	CRect rect, selRect;
	GetSelectionRect(selRect);

	int idx = GetNextSel(0);
	while(idx >= 0)
	{
		CGuiObj *pObj = GetObj(idx);
		ASSERT(pObj != NULL);

		pObj->GetRect(rect);
		rect.top = selRect.top + (selRect.bottom - (rect.top+rect.bottom));
		pObj->SetRect(rect);

		idx = GetNextSel(idx);
	}
	Invalidate();
	SetModifiedFlag(TRUE);
}

// ============================================================================
void CVisualMSDoc::OnEditMoveLeft() 
{
	int idx = GetNextSel(0);
	while(idx >= 0)
	{
		CGuiObj *pObj = GetObj(idx);
		ASSERT(pObj != NULL);

		CRect rect;
		pObj->GetRect(rect);
		rect.left -= 1;
		pObj->SetRect(rect);

		idx = GetNextSel(idx);
	}
	Invalidate();
	SetModifiedFlag(TRUE);
}

// ============================================================================
void CVisualMSDoc::OnEditMoveRight() 
{
	int idx = GetNextSel(0);
	while(idx >= 0)
	{
		CGuiObj *pObj = GetObj(idx);
		ASSERT(pObj != NULL);

		CRect rect;
		pObj->GetRect(rect);
		rect.left += 1;
		pObj->SetRect(rect);

		idx = GetNextSel(idx);
	}
	Invalidate();
	SetModifiedFlag(TRUE);
}

// ============================================================================
void CVisualMSDoc::OnEditMoveUp() 
{
	int idx = GetNextSel(0);
	while(idx >= 0)
	{
		CGuiObj *pObj = GetObj(idx);
		ASSERT(pObj != NULL);

		CRect rect;
		pObj->GetRect(rect);
		rect.top -= 1;
		pObj->SetRect(rect);

		idx = GetNextSel(idx);
	}
	Invalidate();
	SetModifiedFlag(TRUE);
}

// ============================================================================
void CVisualMSDoc::OnEditMoveDown() 
{
	int idx = GetNextSel(0);
	while(idx >= 0)
	{
		CGuiObj *pObj = GetObj(idx);
		ASSERT(pObj != NULL);

		CRect rect;
		pObj->GetRect(rect);
		rect.top += 1;
		pObj->SetRect(rect);

		idx = GetNextSel(idx);
	}
	Invalidate();
	SetModifiedFlag(TRUE);
}

// ============================================================================

static struct { TCHAR* name; int id; } itemCodes[] =
{
	_T("bitmap"),			ID_CTRL_BITMAP,
	_T("button"),			ID_CTRL_BUTTON,
	_T("mapbutton"),		ID_CTRL_MAPBUTTON,
	_T("materialbutton"),	ID_CTRL_MATERIALBUTTON,
	_T("pickbutton"),		ID_CTRL_PICKBUTTON,
	_T("checkbutton"),		ID_CTRL_CHECKBUTTON,
	_T("colorpicker"),		ID_CTRL_COLORPICKER,
	_T("combobox"),			ID_CTRL_COMBOBOX,
	_T("dropdownlist"),		ID_CTRL_DROPDOWNLIST,
	_T("listbox"),			ID_CTRL_LISTBOX,
	_T("edittext"),			ID_CTRL_EDITTEXT,
	_T("label"),			ID_CTRL_LABEL,
	_T("groupbox"),			ID_CTRL_GROUPBOX,
	_T("checkbox"),			ID_CTRL_CHECKBOX,
	_T("radiobuttons"),		ID_CTRL_RADIOBUTTONS,
	_T("spinner"),			ID_CTRL_SPINNER,
	_T("progressbar"),		ID_CTRL_PROGRESSBAR,
	_T("slider"),			ID_CTRL_SLIDER,
	_T("timer"),			ID_CTRL_TIMER,
	_T("activeXControl"),			ID_CTRL_ACTIVEX,
	_T("<custom>"),			ID_CTRL_CUSTOM,
	//
	_T("<code>"),			ID_CODE,
	NULL,
};

int CVisualMSDoc::ItemIDFromName(TCHAR* itemName)
{
	for (int i = 0; itemCodes[i].name != NULL; i++)
		if (_tcsicmp(itemCodes[i].name, itemName) == 0)
			return itemCodes[i].id;
	return ID_CTRL_CUSTOM;
}

#pragma warning(pop) 


