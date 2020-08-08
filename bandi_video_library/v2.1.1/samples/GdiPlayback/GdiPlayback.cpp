////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// BandiVideoLibrary 2.1 Sample
/// 
/// Copyright(C) 2008-2010 BandiSoft.com All rights reserved.
///
/// Visit http://www.bandisoft.com for more information.
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <stdio.h>

#include "BandiVideoLibrary.h"

#pragma warning (disable:4996)

#define WINDOW_TEXT	"BandiVideo Gdi Playback"


LRESULT WINAPI WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_PAINT:
		{
			// 검은색으로 배경 칠하기
			PAINTSTRUCT ps;
			HDC dc;
			dc = BeginPaint(hwnd, &ps);
			PatBlt(dc, 0, 0, 4096, 4096, BLACKNESS);
			EndPaint(hwnd, &ps);
		}
		return 0;

	case WM_ERASEBKGND:
		return 1;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}



// 윈도우 생성
static HWND CreatePlayerWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance)
{
	if(!hPrevInstance)
	{
		WNDCLASS wc;

		wc.style = 0;
		wc.lpfnWndProc = WindowProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(101));
		wc.hCursor = LoadCursor(0, IDC_ARROW);;
		wc.hbrBackground = 0;
		wc.lpszMenuName = 0;
		wc.lpszClassName = "GdiPlayback";

		if(!RegisterClass(&wc))
		{
			return( 0 );
		}
	}

	return CreateWindow("GdiPlayback", WINDOW_TEXT, 
						WS_CAPTION | WS_POPUP| WS_CLIPCHILDREN |
						WS_SYSMENU | WS_MINIMIZEBOX | WS_SIZEBOX,
						64, 64,
						64, 64,
						0, 0, hInstance,0);
}


// 윈도우 보더 크기 계산
static void CalcWindowBorder(HWND hwnd, int &extra_width, int &extra_height)
{
	RECT r, c;

	GetWindowRect(hwnd, &r);
	GetClientRect(hwnd, &c);

	extra_width		= ( r.right - r.left ) - ( c.right - c.left );
	extra_height	= ( r.bottom - r.top ) - ( c.bottom - c.top );
}


static BOOL DrawHBITMAP(HWND hWnd, HBITMAP hBitmap)
{
	HDC hDC = ::GetDC(hWnd);
	
	if(hDC==NULL) {ASSERT(0); return FALSE;}

	// 화면 밖으로 안나가게 하기..
	RECT	rClient;
	int		sw, sh;

	::GetClientRect(hWnd, &rClient);
	sw = (rClient.right-rClient.left);
	sh = (rClient.bottom-rClient.top);

	HDC hdcSrc = CreateCompatibleDC(hDC);
	if(hdcSrc==NULL){ASSERT(0); ::ReleaseDC(hWnd, hDC); return FALSE;}

	HBITMAP hBitmapOld = (HBITMAP)SelectObject(hdcSrc, hBitmap);
	BitBlt(hDC, 0, 0, sw, sh, hdcSrc, 0, 0, SRCCOPY);
	SelectObject(hdcSrc, hBitmapOld);
	DeleteDC(hdcSrc);

	::ReleaseDC(hWnd, hDC);
	return TRUE;
}



int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HWND hwnd;
	MSG msg;
	BITMAPINFO bmpinfo;
	BYTE* bmpdata = NULL;
	int extra_width, extra_height;

	// 재생창 생성하기
	hwnd = CreatePlayerWindow(hInstance, hPrevInstance);
	if(!hwnd)
	{
		MessageBox(NULL, "Error creating window.", WINDOW_TEXT, MB_OK | MB_ICONSTOP);
		return 1;
	}

	CalcWindowBorder(hwnd, extra_width, extra_height);

	CBandiVideoLibrary		bvl;

	if(FAILED(bvl.Create(BANDIVIDEO_DLL_FILE_NAME, NULL, NULL)))
	{
		MessageBox(NULL, "Error creating BandiVideoLibrary.", WINDOW_TEXT, MB_OK | MB_ICONSTOP);
		DestroyWindow(hwnd);
		return 2;
	}

/*	// verify
	if(FAILED(bvl.Verify("BANDISOFT-TRIAL-200909", "ea92bc5b")))
	{
		// trial expired!!
		ASSERT(0);
	}*/

	if(FAILED(bvl.Open(lpCmdLine, FALSE)))
	{
		MessageBox(NULL, "Error opening file....", WINDOW_TEXT, MB_OK | MB_ICONSTOP);
		DestroyWindow(hwnd);
		return 3;
	}

	BVL_VIDEO_INFO info;
	if(FAILED(bvl.GetVideoInfo(info)))
	{
		MessageBox(NULL, "Error getting video info....", WINDOW_TEXT, MB_OK | MB_ICONSTOP);
		DestroyWindow(hwnd);
		return 4;
	}

	// 비디오 크기에 맞춰 창크기 변경
	SetWindowPos(hwnd, 0, 0, 0, info.width+extra_width, info.height+extra_height, SWP_NOMOVE);


	// BITMAPINFO 생성

	memset(&bmpinfo, NULL, sizeof(bmpinfo));

	bmpinfo.bmiHeader.biHeight = -info.height;
	bmpinfo.bmiHeader.biWidth =	info.width;
	bmpinfo.bmiHeader.biBitCount = 32;
	bmpinfo.bmiHeader.biClrUsed = 0;
	bmpinfo.bmiHeader.biClrImportant = 0;
	bmpinfo.bmiHeader.biCompression = BI_RGB;
	bmpinfo.bmiHeader.biXPelsPerMeter = 0;
	bmpinfo.bmiHeader.biYPelsPerMeter = 0;
	bmpinfo.bmiHeader.biPlanes = 1;
	bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

	DWORD dwRowByte = ((bmpinfo.bmiHeader.biWidth * (DWORD)32 + 31) & ~31) >> 3;
	bmpinfo.bmiHeader.biSizeImage = dwRowByte*bmpinfo.bmiHeader.biWidth;

	bmpdata = (BYTE*)malloc(bmpinfo.bmiHeader.biSizeImage);

	HBITMAP hbitmap;
	hbitmap = CreateDIBSection(NULL, &bmpinfo, DIB_RGB_COLORS, (LPVOID*)&bmpdata, NULL, 0);

	bvl.Play();

	ShowWindow(hwnd, nCmdShow);

	do
	{
		if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			BVL_STATUS status;
			bvl.GetStatus(status);
			if(status == BVL_STATUS_PLAYEND)
			{
				bvl.Seek(0, BVL_SEEK_TIME);
				bvl.Play();
			}
	
			// 새 프레임을 출력할 시간인가?
			if(bvl.IsNextFrame())
			{
				// Get frame
				BVL_FRAME frame;
				frame.frame_buf = bmpdata;
				frame.frame_buf_size = bmpinfo.bmiHeader.biSizeImage;
				frame.pitch = dwRowByte;
				frame.width = bmpinfo.bmiHeader.biWidth;
				frame.height = abs(bmpinfo.bmiHeader.biHeight);
				frame.pixel_format = BVLPF_X8R8G8B8;

				bvl.GetFrame(frame, FALSE);

				DrawHBITMAP(hwnd, hbitmap);
			}
			else
			{
				Sleep(1);
			}
		}
	}
	while(1);

	bvl.Destroy();

	if(hbitmap) DeleteObject(hbitmap);

	return 0;
}
