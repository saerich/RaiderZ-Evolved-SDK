// ============================================================================
// VmsToolBar.cpp
// Copyright ©2000, Discreet
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#include "stdafx.h"
#include "VmsToolBar.h"

// From the MAX SDK
#include "Max.h"
#include "iColorMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ============================================================================
/*
	DIBs use RGBQUAD format:
		0xbb 0xgg 0xrr 0x00

	Reasonably efficient code to convert a COLORREF into an
	RGBQUAD is byte-order-dependent, so we need different
	code depending on the byte order we're targeting.
*/
#define RGB_TO_RGBQUAD(r,g,b)   (RGB(b,g,r))
#define CLR_TO_RGBQUAD(clr)     (RGB(GetBValue(clr), GetGValue(clr), GetRValue(clr)))

struct CUST_COLORMAP
{
	// use DWORD instead of RGBQUAD so we can compare two RGBQUADs easily
	DWORD rgbqFrom;
	int iSysColorTo;
};

AFX_STATIC_DATA const CUST_COLORMAP custSysColorMap[] =
{
	// mapping from color in DIB to system color
	{ RGB_TO_RGBQUAD(0x00, 0x00, 0x00),  COLOR_BTNTEXT },       // black
	{ RGB_TO_RGBQUAD(0x80, 0x80, 0x80),  COLOR_BTNSHADOW },     // dark gray
	{ RGB_TO_RGBQUAD(0xC0, 0xC0, 0xC0),  COLOR_BTNFACE },       // bright gray
	{ RGB_TO_RGBQUAD(0xFF, 0xFF, 0xFF),  COLOR_BTNHIGHLIGHT }   // white
};

// ============================================================================
HBITMAP LoadCustSysColorBitmap(HINSTANCE hInst, HRSRC hRsrc, BOOL bMono)
{
	HGLOBAL hglb;
	if ((hglb = LoadResource(hInst, hRsrc)) == NULL)
		return NULL;

	LPBITMAPINFOHEADER lpBitmap = (LPBITMAPINFOHEADER)LockResource(hglb);
	if (lpBitmap == NULL)
		return NULL;

	// make copy of BITMAPINFOHEADER so we can modify the color table
	const int nColorTableSize = 16;
	UINT nSize = lpBitmap->biSize + nColorTableSize * sizeof(RGBQUAD);
	LPBITMAPINFOHEADER lpBitmapInfo = (LPBITMAPINFOHEADER)::malloc(nSize);
	if (lpBitmapInfo == NULL)
		return NULL;
	memcpy(lpBitmapInfo, lpBitmap, nSize);

	// color table is in RGBQUAD DIB format
	DWORD* pColorTable =
		(DWORD*)(((LPBYTE)lpBitmapInfo) + (UINT)lpBitmapInfo->biSize);

	for (int iColor = 0; iColor < nColorTableSize; iColor++)
	{
		// look for matching RGBQUAD color in original
		for (int i = 0; i < 4/*_countof(custSysColorMap)*/; i++)
		{
			if (pColorTable[iColor] == custSysColorMap[i].rgbqFrom)
			{
				if (bMono)
				{
					// all colors except text become white
					if (custSysColorMap[i].iSysColorTo != COLOR_BTNTEXT)
						pColorTable[iColor] = RGB_TO_RGBQUAD(255, 255, 255);
				}
				else
					pColorTable[iColor] =
						CLR_TO_RGBQUAD(GetCustSysColor(custSysColorMap[i].iSysColorTo));
				break;
			}
		}
	}

	int nWidth = (int)lpBitmapInfo->biWidth;
	int nHeight = (int)lpBitmapInfo->biHeight;
	HDC hDCScreen = ::GetDC(NULL);
	HBITMAP hbm = ::CreateCompatibleBitmap(hDCScreen, nWidth, nHeight);

	if (hbm != NULL)
	{
		HDC hDCGlyphs = ::CreateCompatibleDC(hDCScreen);
		HBITMAP hbmOld = (HBITMAP)::SelectObject(hDCGlyphs, hbm);

		LPBYTE lpBits;
		lpBits = (LPBYTE)(lpBitmap + 1);
		lpBits += static_cast<UINT>(1 << (lpBitmapInfo->biBitCount)) * sizeof(RGBQUAD);

		StretchDIBits(hDCGlyphs, 0, 0, nWidth, nHeight, 0, 0, nWidth, nHeight,
			lpBits, (LPBITMAPINFO)lpBitmapInfo, DIB_RGB_COLORS, SRCCOPY);
		SelectObject(hDCGlyphs, hbmOld);
		::DeleteDC(hDCGlyphs);
	}
	::ReleaseDC(NULL, hDCScreen);

	// free copy of bitmap info struct and resource itself
	::free(lpBitmapInfo);
	::FreeResource(hglb);

	return hbm;
}


// ============================================================================
BEGIN_MESSAGE_MAP(CVmsToolBar, CToolBar)
	//{{AFX_MSG_MAP(CVmsToolBar)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ============================================================================
CVmsToolBar::CVmsToolBar() {}
CVmsToolBar::~CVmsToolBar() {}

// ============================================================================
BOOL CVmsToolBar::OnEraseBkgnd(CDC* pDC) 
{
	CRect rect;
	GetClientRect(rect);
	pDC->FillRect(rect, CBrush::FromHandle(GetCustSysColorBrush(COLOR_BTNFACE)));
	return TRUE;
}

// ============================================================================
BOOL CVmsToolBar::LoadToolBar(LPCTSTR lpszResourceName)
{
	CToolBar::LoadToolBar(lpszResourceName);

	// determine location of the bitmap in resource fork
	HINSTANCE hInstImageWell = AfxFindResourceHandle(lpszResourceName, RT_BITMAP);
	HRSRC hRsrcImageWell = ::FindResource(hInstImageWell, lpszResourceName, RT_BITMAP);
	if (hRsrcImageWell == NULL)
		return FALSE;

	// load the bitmap
	HBITMAP hbmImageWell;
	hbmImageWell = LoadCustSysColorBitmap(hInstImageWell, hRsrcImageWell, FALSE);

	// tell common control toolbar about the new bitmap
	if (!AddReplaceBitmap(hbmImageWell))
		return FALSE;

	// remember the resource handles so the bitmap can be recolored if necessary
	m_hInstImageWell = hInstImageWell;
	m_hRsrcImageWell = hRsrcImageWell;

	return TRUE;
}

