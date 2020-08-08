// ============================================================================
// GuiObj.cpp
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#include "stdafx.h"
#include "resource.h"
#include "MainFrm.h"
#include "VisualMSDoc.h"
#include "VisualMS.h"
#include "GuiObj.h"
#include "StdObj.h"


// ============================================================================
CCtrlSizer::CCtrlSizer()
{
	m_lockMovement = false;
	m_oldRect.SetRectEmpty();
	m_pObj = NULL;
	m_handleMask = m_restoreMask = HM_ALL;
	m_nHandleSize = c_handleSize;
	m_nStyle = CRectTracker::resizeOutside | CRectTracker::solidLine;
}

// ============================================================================
// NOTE: make sure to set the m_oldRect member to the current object size in 
//       relative coordinates before calling Track() 
void CCtrlSizer::AdjustRect(int nHandle, LPRECT lpRect)
{
	CRect rect = *lpRect;
	rect.right -= rect.left;
	rect.bottom -= rect.top;

	CRect change;
	change.left = rect.left - m_oldRect.left;
	change.top = rect.top - m_oldRect.top;
	change.right = rect.right - m_oldRect.right;
	change.bottom = rect.bottom - m_oldRect.bottom;

	// translation axis restriction
	if(m_lockMovement)
	{
		CPoint mousePoint;
		::GetCursorPos(&mousePoint);
		mousePoint -= m_mouseStartPoint;

		if(abs(mousePoint.x) > abs(mousePoint.y))
		{
			lpRect->top = rect.top = m_oldRect.top;
			lpRect->bottom = m_oldRect.top + m_oldRect.bottom;
			rect.bottom = m_oldRect.bottom;
			change.top = change.bottom = 0;
		}
		else
		{
			lpRect->left = rect.left = m_oldRect.left;
			lpRect->right = m_oldRect.left + m_oldRect.right;
			rect.right = m_oldRect.right;
			change.left = change.right = 0;
		}
	}


	CVisualMSDoc *pDoc = CVisualMSDoc::GetDoc();
	if(pDoc)
	{
		// Snap the coordinates if grid is on
		if(pDoc->GetUseGrid())
		{
			int snap = pDoc->GetGridSpacing();

			// if change in both left and right then just the left side is being 
			// sized because this is based of relative coordinates. if there is change in just left
			// the left side then the whole object is moving, and if just change
			// in right side then the right side is being sized. same goes for verticle.
			if(change.left && change.right)
				lpRect->left = SnapValue(lpRect->left, snap);
			else if(change.left) { //
				lpRect->left = SnapValue(lpRect->left, snap);
				lpRect->right = lpRect->left + rect.right;
			}
			else if(change.right)
				lpRect->right = SnapValue(lpRect->right, snap);

			if(change.top && change.bottom)
				lpRect->top = SnapValue(lpRect->top, snap);
			else if(change.top) {
				lpRect->top = SnapValue(lpRect->top, snap);
				lpRect->bottom = lpRect->top + rect.bottom;
			}
			else if(change.bottom)
				lpRect->bottom = SnapValue(lpRect->bottom, snap);

			// recalc relative coordinates from snapped values for UI views width/height
			rect = *lpRect;
			rect.right -= rect.left;
			rect.bottom -= rect.top;
		}

		if(rect.right < 0) rect.right = 0;
		if(rect.bottom < 0) rect.bottom = 0;

		if(m_pObj)
		{
			// restrict size
			if(rect.right < m_pObj->m_minimumSize.cx)
				rect.right = m_pObj->m_minimumSize.cx;
			else if(rect.right > m_pObj->m_maximumSize.cx)
				rect.right = m_pObj->m_maximumSize.cx;

			if(rect.bottom < m_pObj->m_minimumSize.cy)
				rect.bottom = m_pObj->m_minimumSize.cy;
			else if(rect.bottom > m_pObj->m_maximumSize.cy)
				rect.bottom = m_pObj->m_maximumSize.cy;

			lpRect->right = lpRect->left + rect.right;
			lpRect->bottom = lpRect->top + rect.bottom;

			if(change.left)
			{
				m_pObj->m_properties[IDX_XPOS] = rect.left;
				pDoc->UpdateAllViews(NULL, HINT_CHANGEPROP, (CObject*)IDX_XPOS);
			}
			if(change.top)
			{
				m_pObj->m_properties[IDX_YPOS] = rect.top;
				pDoc->UpdateAllViews(NULL, HINT_CHANGEPROP, (CObject*)IDX_YPOS);
			}
			if(change.right)
			{
				m_pObj->m_properties[IDX_WIDTH] = rect.right;
				pDoc->UpdateAllViews(NULL, HINT_CHANGEPROP, (CObject*)IDX_WIDTH);
			}
			if(change.bottom)
			{
				m_pObj->m_properties[IDX_HEIGHT] = rect.bottom;
				pDoc->UpdateAllViews(NULL, HINT_CHANGEPROP, (CObject*)IDX_HEIGHT);
			}
		}
	}

	SetStatus(PANE_POS, "%d, %d", rect.left, rect.top);
	SetStatus(PANE_SIZE, "%d x %d", rect.right, rect.bottom);
	CRectTracker::AdjustRect(nHandle, lpRect); // must use absolute coordinates for width/height
}

// ============================================================================
void CCtrlSizer::SetCursor(UINT hit)
{
	if(hit == CRectTracker::hitTopLeft || hit == CRectTracker::hitBottomRight)
		::SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
	else if(hit == CRectTracker::hitTop || hit == CRectTracker::hitBottom)
		::SetCursor(LoadCursor(NULL, IDC_SIZENS));
	else if(hit == CRectTracker::hitTopRight || hit == CRectTracker::hitBottomLeft)
		::SetCursor(LoadCursor(NULL, IDC_SIZENESW));
	else if(hit == CRectTracker::hitRight || hit == CRectTracker::hitLeft)
		::SetCursor(LoadCursor(NULL, IDC_SIZEWE));
//	else if(hit == CRectTracker::hitMiddle)
//		::SetCursor(LoadCursor(NULL, IDC_SIZEALL));
	else
		::SetCursor(LoadCursor(NULL, IDC_ARROW));
}

// ============================================================================
BOOL CCtrlSizer::SetCursor(CWnd* pWnd, UINT nHitTest) const
{
	return CRectTracker::SetCursor(pWnd, nHitTest);
}

// ============================================================================
CGuiObj::CGuiObj()
{
	m_createID = 0;
	m_selected = FALSE;
	m_pParent = NULL;

	m_minimumSize = CSize(0,0);
	m_maximumSize = CSize(INT_MAX, INT_MAX);

	src_from = -1;  // => not from source parse
	src_to = -1;
}

// ============================================================================
CGuiObj::CGuiObj(CWnd *pParent)
{
	m_selected = FALSE;
	m_properties.Add(CProperty(_T("class"), CString("")));
	m_properties.Add(CProperty(_T("name"), CString("")));
	m_properties.Add(CProperty(_T("caption"), CString("")));
	m_properties.Add(CProperty(_T("x-pos"), 0));
	m_properties.Add(CProperty(_T("y-pos"), 0));
	m_properties.Add(CProperty(_T("width"), 20));
	m_properties.Add(CProperty(_T("height"), 12));
	m_properties.Add(CProperty(_T("enabled"), true));

	// setup default size limitations for objects
	m_minimumSize = CSize(0,0);
	m_maximumSize = CSize(INT_MAX, INT_MAX);

	m_pParent = pParent;

	src_from = -1;  // => not from source parse
	src_to = -1;
}
CGuiObj::~CGuiObj() {}

// ============================================================================
void CGuiObj::SetHandleMask(UINT mask)
{
	m_sizer.m_restoreMask = m_sizer.m_handleMask;
	m_sizer.m_handleMask = mask;
}

// ============================================================================
void CGuiObj::ClearHandleMask() { SetHandleMask(HM_NONE); }

// ============================================================================
void CGuiObj::RestoreHandleMask() { m_sizer.m_handleMask = m_sizer.m_restoreMask; }

// ============================================================================
void CGuiObj::Emit(CString &out)
{

	out.Format(_T("%s %s \"%s\" pos:[%d,%d]"),
		Class(), Name(), Caption(),
		GetPropInt(IDX_XPOS),
		GetPropInt(IDX_YPOS));

	for (int i = IDX_WIDTH; i < m_properties.GetSize(); i++)
	{
		if ((m_properties[i].m_flags & FLAG_DONTEMIT) || m_properties[i].HasDefaultValue())
			continue;

		CString sVal = GetPropStr(i);
		if(m_properties[i].m_type == PROP_STRING)
			sVal = _T("\"") + sVal + _T("\"");

		// dont emit if empty string or array
		if (!sVal.IsEmpty() && sVal.Compare(_T("#()")))
			out += _T(" ") + m_properties[i].m_name + _T(":") + sVal;
	}

	out += m_comment;
}

// ============================================================================
void CGuiObj::EmitHandler(int idx, CString &hdr, CString &body)
{
	ASSERT(idx >= 0 && idx < m_handlers.GetSize());
	hdr.Format(_T("on %s %s %s"),
		(LPCTSTR)Name(),
		(LPCTSTR)m_handlers[idx].m_name,
		(LPCTSTR)m_handlers[idx].m_args);
	body = m_handlers[idx].m_text;
}

// ============================================================================
// fast rectangle intersection test. Expand the controls bbox by half of the 
// testing rectangles bbox in all directions, then just test if center of 
// testing rectangle is within the expanded bbox.
bool CGuiObj::HitTest(const CRect &rect)
{
	CRect r;

	int cx = (rect.right-rect.left) >> 1;
	int cy = (rect.bottom-rect.top) >> 1;

	CPoint point;
	point.x = rect.left + cx;
	point.y = rect.top + cy;

	// add 5 pixel border for selected objects
	if(IsSelected())
		cx += 5, cy += 5;

	GetRect(r);
	r.right = (r.left + r.right) + cx;
	r.left -= cx;
	r.bottom = (r.top + r.bottom) + cy;
	r.top -= cy;

	if(point.x < r.left ||
		point.x > r.right ||
		point.y < r.top ||
		point.y > r.bottom
	) return false;

	return true;
}

// ============================================================================
void CGuiObj::InvalidateProp(int idx)
{
	CVisualMSDoc *pDoc = CVisualMSDoc::GetDoc();
	if(pDoc) pDoc->UpdateAllViews(NULL, HINT_CHANGEPROP, reinterpret_cast<CObject*>(static_cast<UINT_PTR>(idx)));
}

// ============================================================================
int CGuiObj::PropChanged(int idx)
{
	HWND hFrameWnd = NULL;

	// PropChanged has been called from within an executing VMS thread.
	CMainFrame *pFrameWnd = CMainFrame::GetFrame();
	if(pFrameWnd != NULL)
		hFrameWnd = pFrameWnd->GetSafeHwnd();
	else if(m_pParent != NULL)
		hFrameWnd = CMainFrame::GetFrameHWnd(m_pParent->GetSafeHwnd());

	if(hFrameWnd != NULL)
		::SendMessage(hFrameWnd, WM_PROPCHANGE, (WPARAM)this, idx);
	return 0;
}

// ============================================================================
CString CGuiObj::GetPropStr(int idx) const
{
	if(idx >= 0 && idx < m_properties.GetSize())
	{	CProperty &val = const_cast <CProperty &>(m_properties[idx]);
		return CString((LPCTSTR)val);
	}
	return _T("");
}

// ============================================================================
void CGuiObj::SetPropStr(int idx, const CString &sVal)
{
	if(idx >= 0 && idx < m_properties.GetSize())
	{
		m_properties[idx] = sVal;
		CVisualMSDoc *pDoc = CVisualMSDoc::GetDoc();
		if(pDoc) pDoc->UpdateAllViews(NULL, HINT_CHANGEPROP, reinterpret_cast<CObject*>(static_cast<UINT_PTR>(idx)));
	}
}

// ============================================================================
int CGuiObj::GetPropInt(int idx) const
{
	if(idx >= 0 && idx < m_properties.GetSize())
		return (int)m_properties[idx];
	return 0;
}

// ============================================================================
void CGuiObj::SetPropInt(int idx, int iVal)
{
	if(idx >= 0 && idx < m_properties.GetSize())
	{
		m_properties[idx] = iVal;
		CVisualMSDoc *pDoc = CVisualMSDoc::GetDoc();
		if(pDoc) pDoc->UpdateAllViews(NULL, HINT_CHANGEPROP, reinterpret_cast<CObject*>(static_cast<UINT_PTR>(idx)));
	}
}

// ============================================================================
CGuiObj* CGuiObj::Create(UINT nID, CWnd *pParent, const CRect &rect)
{
	if(pParent == NULL)
	{
		CVisualMSDoc *pDoc = CVisualMSDoc::GetDoc();
		if(pDoc)
			pParent = &pDoc->GetFormObj()->m_formEd;
	}

	CGuiObj* obj = NULL;
	switch(nID)
	{
	case ID_CODE:
		obj = new CCodeObj(_T(""));
		break;
	case ID_CTRL_BITMAP:
		obj = new CBitmapObj(pParent, rect);
		break;
	case ID_CTRL_BUTTON:
		obj = new CButtonObj(pParent, rect);
		break;
	case ID_CTRL_MAPBUTTON:
		obj = new CMapButtonObj(pParent, rect);
		break;
	case ID_CTRL_MATERIALBUTTON:
		obj = new CMaterialButtonObj(pParent, rect);
		break;
	case ID_CTRL_PICKBUTTON:
		obj = new CPickButtonObj(pParent, rect);
		break;
	case ID_CTRL_CHECKBUTTON:
		obj = new CCheckButtonObj(pParent, rect);
		break;
	case ID_CTRL_COLORPICKER:
		obj = new CColorPickerObj(pParent, rect);
		break;
	case ID_CTRL_COMBOBOX:
		obj = new CComboBoxObj(pParent, rect);
		break;
	case ID_CTRL_DROPDOWNLIST:
		obj = new CDropDownListObj(pParent, rect);
		break;
	case ID_CTRL_LISTBOX:
		obj = new CListBoxObj(pParent, rect);
		break;
	case ID_CTRL_EDITTEXT:
		obj = new CEditTextObj(pParent, rect);
		break;
	case ID_CTRL_LABEL:
		obj = new CLabelObj(pParent, rect);
		break;
	case ID_CTRL_GROUPBOX:
		obj = new CGroupBoxObj(pParent, rect);
		break;
	case ID_CTRL_CHECKBOX:
		obj = new CCheckboxObj(pParent, rect);
		break;
	case ID_CTRL_RADIOBUTTONS:
		obj = new CRadioButtonsObj(pParent, rect);
		break;
	case ID_CTRL_SPINNER:
		obj = new CSpinnerObj(pParent, rect);
		break;
	case ID_CTRL_PROGRESSBAR:
		obj = new CProgressBarObj(pParent, rect);
		break;
	case ID_CTRL_SLIDER:
		obj = new CSliderObj(pParent, rect);
		break;
	case ID_CTRL_TIMER:
		obj = new CTimerObj(pParent, rect);
		break;
	case ID_CTRL_ACTIVEX:
		obj = new CActiveXObj(pParent, rect);
		break;
	case ID_CTRL_CUSTOM:
		obj = new CCustomObj(pParent, rect);
		break;
	}

	if(obj)
	{
		obj->m_createID = nID;
	}

	return obj;
}

// ============================================================================

int CGuiObj::FindProperty(TCHAR* propName)
{
	for (int i = 0; i < m_properties.GetSize(); i++)
	{
		if (_tcsicmp(m_properties[i].m_name, propName) == 0)
			return i;
	}

	// map 'text' to 'caption', synonmous general rollout item properties in MAXScript 
	if (_tcsicmp(propName, _T("text")) == 0)	
		return IDX_CAPTION;

	// ignore align and offset parameters
	if (_tcsicmp(propName, _T("align")) == 0 ||
		_tcsicmp(propName, _T("offset")) == 0)
		return -1;

	// not found, for now we will add a new one, even though this might be a typo,
	// it gives us a way to handle new props not yet known by the editor without losing them
	// over an edit
	return m_properties.Add(CProperty(propName, 0));

	// return -1;
}

int CGuiObj::FindHandler(TCHAR* eventName)
{
	for (int i = 0; i < m_handlers.GetSize(); i++)
		if (_tcsicmp(m_handlers[i].m_name, eventName) == 0)
			return i;
	return -1;
}

// ============================================================================
void CGuiObj::Destroy(CGuiObj *obj)
{
	ASSERT(obj != NULL);
	delete obj;
}

// ----- IVisualMSItem methods -------------------------

void CGuiObj::SetPropery(TCHAR* propName, float f)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	int i = FindProperty(propName);
	if (i >= 0) 
	{
		m_properties[i].m_type = PROP_REAL;
		m_properties[i] = f;
		PropChanged(i);
	}
}

void CGuiObj::SetPropery(TCHAR* propName, int i)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	int j = FindProperty(propName);
	if (j >= 0) 
	{
		m_properties[j].m_type = PROP_INTEGER;
		SetPropInt(j, i);
		PropChanged(j);
	}
}

void CGuiObj::SetPropery(TCHAR* propName, bool b)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	int j = FindProperty(propName);
	if (j >= 0) 
	{
		m_properties[j].m_type = PROP_BOOLEAN;
		m_properties[j] = b;
		PropChanged(j);
	}
}

void CGuiObj::SetPropery(TCHAR* propName, TCHAR* s)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	int i = FindProperty(propName);
	if (i >= 0) 
	{
		m_properties[i].m_type = PROP_STRING;
		m_properties[i] = s;
		PropChanged(i);
	}
}

void CGuiObj::SetProperyLit(TCHAR* propName, TCHAR* s)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	int i = FindProperty(propName);
	if (i >= 0) 
	{
		// handle options
		if(m_properties[i].m_type != PROP_OPTION)
			m_properties[i].m_type = PROP_LITERAL;

		m_properties[i] = s;
		PropChanged(i);
	}
}

void CGuiObj::SetPropery(TCHAR* propName, Point2 p2)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (_stricmp(propName, _T("pos")) == 0 || _stricmp(propName, _T("position")) == 0)
	{
		// hack to handle pos:, turn into pos_x, pos_y  (fix this)
		m_properties[IDX_XPOS] = p2.x;
		m_properties[IDX_YPOS] = p2.y;
		PropChanged(IDX_XPOS);
	}
	else
	{
		int j = FindProperty(propName);
		if (j >= 0) 
		{
			m_properties[j].m_type = PROP_POINT2;
			m_properties[j] = p2;
			PropChanged(j);
		}
	}
}

void CGuiObj::SetPropery(TCHAR* propName, Point3 p3)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	int j = FindProperty(propName);
	if (j >= 0) 
	{
		m_properties[j].m_type = PROP_POINT3;
		m_properties[j] = p3;
		PropChanged(j);
	}
}

void CGuiObj::SetPropery(TCHAR* propName, Tab<TCHAR*>* sa)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	int i = FindProperty(propName);
	if (i >= 0) 
	{
		CStringArray csa;
		csa.SetSize(sa->Count());
		for (int j = 0; j < sa->Count(); j++)
			csa[j] = (*sa)[j];

		m_properties[i].m_type = PROP_ARRAY;
		m_properties[i] = csa;
		PropChanged(i);
	}
}

void CGuiObj::GetShape(int& left, int& top, int& width, int& height)
{
	left = GetPropInt(IDX_XPOS);
	top  = GetPropInt(IDX_YPOS);
	width  = GetPropInt(IDX_WIDTH);
	height = GetPropInt(IDX_HEIGHT);
}

void CGuiObj::SetShape(int left, int top, int width, int height)
{
	SetPropInt(IDX_XPOS, left);
	SetPropInt(IDX_YPOS, top);
	SetPropInt(IDX_WIDTH, width);
	SetPropInt(IDX_HEIGHT, height);
	PropChanged(IDX_XPOS);
	PropChanged(IDX_YPOS);
	PropChanged(IDX_WIDTH);
	PropChanged(IDX_HEIGHT);
}

void CGuiObj::SetHandler(TCHAR* eventName, TCHAR* source, int arg_start, int arg_end, int body_start, int body_end)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	int h = FindHandler(eventName);

	if (h < 0)
	{
		// for now, if event not recognized, will simply add as a new event in case it 
		// is a new one VMs doesnt know about and so it doesn't get dropped
		// over an edit
		h = m_handlers.Add(CHandler(false, eventName, _T("do"), _T("")));
	}

	// if (h >= 0)
	{
		m_handlers[h].m_use = TRUE;
		// body
		TCHAR c = source[body_end];
		source[body_end] = _T('\0');
		CString body = &source[body_start], &trimBody = m_handlers[h].m_text;
		source[body_end] = c;
		// clean up body, remove leading indents
		body.TrimRight();
		int i, j, k, indent; 
		for (i = 0; i < body.GetLength() && (body[i] == _T('\n') || body[i] == _T('\r')); i++) ;  // span linefeeds
		for (indent = 0, j = i; j < body.GetLength(); j++)  // compute indent
			if (body[j] == _T('\t')) indent += 4;
			else if (body[j] == _T(' ')) indent += 1;
			else break;
		for (i = j; i < body.GetLength(); i++)				// trim out leading common indents
			if (body[i] == _T('\n') || body[i] == _T('\r'))
			{
				while (body[i] == _T('\n') || body[i] == _T('\r')) i++;
				trimBody += body.Mid(j, i - j);
				for (j = i, k = 0; j < body.GetLength() && k < indent; j++)
					if (body[j] == _T('\t')) k += 4;
					else if (body[j] == _T(' ')) k += 1;
					else break;
			}
		if (j < body.GetLength())
			trimBody += body.Mid(j, body.GetLength() - j);
		// args do
		c = source[arg_end];
		source[arg_end] = _T('\0');
		m_handlers[h].m_args = &source[arg_start];
		m_handlers[h].m_args.TrimRight();
		source[arg_end] = c;
	}
}

void    
CGuiObj::AddComment(TCHAR* code, int src_from, int src_to)
{
	if (src_from < src_to)
	{
		// up to line end or eos
		for (int i = src_from; i < src_to; i++)
			if (code[i] == _T('\n') || code[i] == _T('\r'))
			{
				src_to = i;
				break;
			}
		TCHAR c = code[src_to];
		code[src_to] = _T('\0');
		m_comment = &code[src_from];
		code[src_to] = c;
	}
}

// ------- CCodeObj ----------------------------
CCodeObj::CCodeObj(TCHAR* code) : CGuiObj(NULL) // : code(code)
{
	m_createID = ID_CODE;
	m_selected = FALSE;
	idx_code = m_properties.Add(CProperty(_T("code"), CString(code)));
}


void CCodeObj::Emit(CString &out)
{
	out.Format(_T("%s"), m_properties[idx_code].m_string);
}
