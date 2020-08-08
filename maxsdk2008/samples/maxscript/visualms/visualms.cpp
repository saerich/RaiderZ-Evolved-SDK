// ============================================================================
// VisualMS.cpp
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#include "stdafx.h"
#include "VisualMS.h"
#include "MainFrm.h"
#include "max.h"

// ============================================================================
void VertIndent(CDC *pDC, int x, int y, int h)
{
	pDC->SelectObject(new CPen(PS_SOLID, 0, GetSysColor(COLOR_3DSHADOW)));
	pDC->MoveTo(x, y);
	pDC->LineTo(x, y+h);

	delete pDC->SelectObject(new CPen(PS_SOLID, 0, GetSysColor(COLOR_3DHIGHLIGHT)));
	pDC->MoveTo(x+1, y);
	pDC->LineTo(x+1, y+h);

	delete pDC->SelectStockObject(BLACK_PEN);
}

// ============================================================================
void HorizIndent(CDC *pDC, int x, int y, int w)
{
	pDC->SelectObject(new CPen(PS_SOLID, 0, GetSysColor(COLOR_3DSHADOW)));
	pDC->MoveTo(x,   y);
	pDC->LineTo(x+w, y);

	delete pDC->SelectObject(new CPen(PS_SOLID, 0, GetSysColor(COLOR_3DHIGHLIGHT)));
	pDC->MoveTo(x,   y+1);
	pDC->LineTo(x+w, y+1);

	delete pDC->SelectStockObject(BLACK_PEN);
}

// ============================================================================
void DrawSunkenRect(CDC *pDC, LPRECT lpRect)
{
	pDC->SelectStockObject(WHITE_PEN);
	pDC->MoveTo(lpRect->left, lpRect->bottom-1);
	pDC->LineTo(lpRect->right-1, lpRect->bottom-1);
	pDC->LineTo(lpRect->right-1, lpRect->top-1);

	pDC->SelectStockObject(BLACK_PEN);
	pDC->MoveTo(lpRect->left+1, lpRect->bottom-2);
	pDC->LineTo(lpRect->left+1, lpRect->top+1);
	pDC->LineTo(lpRect->right-1, lpRect->top+1);

	pDC->SelectObject(new CPen(PS_SOLID, 0, RGB(128,128,128)));
	pDC->MoveTo(lpRect->left, lpRect->bottom-2);
	pDC->LineTo(lpRect->left, lpRect->top);
	pDC->LineTo(lpRect->right-1, lpRect->top);

	delete pDC->SelectObject(new CPen(PS_SOLID, 0, RGB(192,192,192)));
	pDC->MoveTo(lpRect->left+1, lpRect->bottom-2);
	pDC->LineTo(lpRect->right-2, lpRect->bottom-2);
	pDC->LineTo(lpRect->right-2, lpRect->top);

	delete pDC->SelectStockObject(BLACK_PEN);
}

// ============================================================================
void DrawGrid(CDC *pDC, LPRECT lpRect, int spacing)
{
	int xlim = lpRect->left + lpRect->right;
	int ylim = lpRect->top + lpRect->bottom;

	int x = lpRect->left + spacing;
	while(x < xlim)
	{
		int y = lpRect->top + spacing;
		while(y < ylim)
		{
			pDC->MoveTo(x, y);
			pDC->LineTo(x, y+1);
			y += spacing;
		}
		x += spacing;
	}
}

// ============================================================================
void CheckRectInvert(LPRECT lpRect)
{
	int i;
	if(lpRect->right < lpRect->left)
	{
		i = lpRect->left;
		lpRect->left = lpRect->right;
		lpRect->right = i;
	}

	if(lpRect->bottom < lpRect->top)
	{
		i = lpRect->top;
		lpRect->top = lpRect->bottom;
		lpRect->bottom = i;
	}
}

// ============================================================================
int SnapValue(int val, int snap)
{
	return ((val%snap) <= (snap/2) ? (val/snap)*snap : (val/snap)*snap+snap);
}

// ============================================================================
bool HasQuotes(const CString &str)
{
	if(str[0] == '\"' && str[str.GetLength()-1] == '\"')
		return true;
	return false;
}

// ============================================================================
void StripQuotes(CString &str)
{
	if(HasQuotes(str))
		str = str.Mid(1, str.GetLength()-2);
}

// ============================================================================
CFont* GetStdFont()
{
	static bool fontCreated = false;
	static CFont stdFont;

	if(!fontCreated)
	{
		LOGFONT lf;
		GetObject(GetStockObject(SYSTEM_FONT), sizeof(lf), &lf);
		lf.lfWeight = 400;
		lf.lfHeight = c_stdFontHeight;
		lf.lfWidth = 0;
		strcpy(lf.lfFaceName, _T("MS Sans Serif"));
		stdFont.CreateFontIndirect(&lf);
		fontCreated = true;
	}

	return &stdFont;
}

// ============================================================================
CFont* GetBigFont()
{
	static bool fontCreated = false;
	static CFont bigFont;

	if(!fontCreated)
	{
		LOGFONT lf;
		GetObject(GetStockObject(SYSTEM_FONT), sizeof(lf), &lf);
		lf.lfWeight = 400;
		lf.lfHeight = c_bigFontHeight;
		lf.lfWidth = 0;
		strcpy(lf.lfFaceName, _T("MS Sans Serif"));
		bigFont.CreateFontIndirect(&lf);
		fontCreated = true;
	}

	return &bigFont;
}

// ============================================================================
int GetTextWidth(CWnd *pWnd, const CString &str)
{
	CDC *pDC = pWnd->GetDC();
	pDC->SelectObject(pWnd->GetFont());
	CSize size = pDC->GetTextExtent(str);
	return size.cx;
}

// ============================================================================
void DebugPrintf(char *fmt, ...)
{
	static char msg[1024];
	va_list arglist;

	if(!fmt) return;
	va_start(arglist, fmt);
	vsprintf(msg, fmt, arglist);
	va_end(arglist);

	OutputDebugString(msg);
}

// ============================================================================
TCHAR *GetString(int nID)
{
	static TCHAR buf[256];

	if(g_hInst)
		return LoadString(g_hInst, nID, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

// ============================================================================
void SetStatus(int pane, char *fmt, ...)
{
	static TCHAR msg[1024];
	va_list arglist;

	CWnd *pWnd = AfxGetMainWnd();

	if(!pWnd)
		return;

	if(fmt == NULL)
	{
		pWnd->SendMessage(WM_SETSTATUS, (WPARAM)pane, (LPARAM)(""));
		return;
	}

	va_start(arglist, fmt);
	vsprintf(msg, fmt, arglist);
	va_end(arglist);

	pWnd->SendMessage(WM_SETSTATUS, (WPARAM)pane, (LPARAM)msg);
}

// ============================================================================
void  MessageBoxf(char *fmt, ...)
{
	static char msg[1024];
	va_list arglist;

	if(!fmt) return;
	va_start(arglist, fmt);
	vsprintf(msg, fmt, arglist);
	va_end(arglist);

	CMainFrame::GetFrame()->MessageBox(msg);
}

// ============================================================================
int YesNoBoxf(char *fmt, ...)
{
	static char msg[1024];
	va_list arglist;

	if(!fmt) return IDNO;
	va_start(arglist, fmt);
	vsprintf(msg, fmt, arglist);
	va_end(arglist);

	return CMainFrame::GetFrame()->MessageBox(msg, NULL, MB_YESNO);
}

// ============================================================================
int YesNoCancelBoxf(char *fmt, ...)
{
	static char msg[1024];
	va_list arglist;

	if(!fmt) return IDNO;
	va_start(arglist, fmt);
	vsprintf(msg, fmt, arglist);
	va_end(arglist);

	return CMainFrame::GetFrame()->MessageBox(msg, NULL, MB_YESNOCANCEL|MB_ICONEXCLAMATION);
}

// ============================================================================
void SaveWindowPosition( LPCTSTR keyName, HWND hWnd )
{
	WINDOWPLACEMENT wp;
	wp.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(hWnd, &wp);

	CString posStr;
	posStr.Format(
		_T("%i %i %i %i"),
		wp.rcNormalPosition.left,
		wp.rcNormalPosition.top,
		wp.rcNormalPosition.right,
		wp.rcNormalPosition.bottom);

	CString fname = GetCOREInterface()->GetDir(APP_PLUGCFG_DIR);
	fname += _T("\\visualms.ini");

	WritePrivateProfileString(_T("WindowPositions"), keyName, (LPCTSTR)posStr, (LPCTSTR)fname);
}

// ============================================================================
void LoadWindowPosition( LPCTSTR keyName, HWND hWnd )
{
	CString fname = GetCOREInterface()->GetDir(APP_PLUGCFG_DIR);
	fname += _T("\\visualms.ini");

	static TCHAR buf[100];
	GetPrivateProfileString(_T("WindowPositions"), keyName, _T(""), buf, 100, (LPCTSTR)fname);

	RECT rect;
	if( sscanf(buf, _T("%i %i %i %i"), &rect.left, &rect.top, &rect.right, &rect.bottom) == 4 )
	{
		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);
		wp.showCmd = SW_SHOWNORMAL;
		wp.rcNormalPosition = rect;
		SetWindowPlacement(hWnd, &wp);
	}
}
