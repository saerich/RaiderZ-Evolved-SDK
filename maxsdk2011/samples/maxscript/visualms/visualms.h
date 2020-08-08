// ============================================================================
// VisualMS.h
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#ifndef __VISUALMS_H__
#define __VISUALMS_H__

/* Defines */
#define DLLEXPORT __declspec(dllexport)

#define MAKEINT(a,b,c,d) ((long)(char)(a) | ((long)(char)(b) << 8) | ((long)(char)(c) << 16) | ((long)(char)(d) << 24))

// custom message API
#define WM_SETSTATUS (WM_USER+1)         /* panel = (int)wParam, string = (TCHAR*)lParam */
#define WM_CREATEOBJ (WM_SETSTATUS+1)    /* createID = (int)wParam, pRect = (CRect*)lParam */
#define WM_PROPCHANGE (WM_CREATEOBJ+1)   /* pObj = (CGuiObj*)wParam, propIdx = (int)lParam */
#define WM_UPDATEVIEWS (WM_PROPCHANGE+1) /* pHint = (CObject*)wParam, lHint = (LPARAM)lParam */

// SetStatus types
#define PANE_TEXT 0
#define PANE_POS  1
#define PANE_SIZE 2

// hints for UpdateAllViews
#define HINT_NONE       0
#define HINT_CHANGESEL  1
#define HINT_CHANGEPROP 2
#define HINT_CHANGEEVENT 3

/* Functions */
void  VertIndent(CDC *pDC, int x, int y, int h);
void  HorizIndent(CDC *pDC, int x, int y, int w);
void  DrawSunkenRect(CDC *pDC, LPRECT lpRect);
void  DrawGrid(CDC *pDC, LPRECT lpRect, int spacing);
void  CheckRectInvert(LPRECT lpRect);
int   SnapValue(int val, int snap);
void  DebugPrintf(char *fmt, ...);
void  SetStatus(int pane, char *fmt, ...);
void  MessageBoxf(char *fmt, ...);
int   YesNoBoxf(char *fmt, ...);
int   YesNoCancelBoxf(char *fmt, ...);
CFont* GetStdFont();
CFont* GetBigFont();
int GetTextWidth(CWnd *pWnd, const CString &str);
TCHAR *GetString(int nID);
bool  HasQuotes(const CString &str);
void  StripQuotes(CString &str);
void SaveWindowPosition( LPCTSTR keyName, HWND hWnd );
void LoadWindowPosition( LPCTSTR keyName, HWND hWnd );


/* Constants */
static const int c_stdFontHeight = 14;
static const int c_bigFontHeight = 16;
static const int c_rulerSize  = 20;
static const int c_formOffset = 10;
static const int c_handleSize = 6;

/* Externs */
extern HINSTANCE g_hInst;




#endif //__VISUALMS_H__
