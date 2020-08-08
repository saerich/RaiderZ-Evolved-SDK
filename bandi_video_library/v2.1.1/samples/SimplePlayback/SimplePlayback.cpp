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
#include "BandiVideoTexture_DX9.h"

#pragma warning (disable:4996)

#define WINDOW_TEXT	"BandiVideo Simple Playback"


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
		wc.lpszClassName = "SimplePlayback";

		if(!RegisterClass(&wc))
		{
			return( 0 );
		}
	}

	return CreateWindow("SimplePlayback", WINDOW_TEXT, 
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



int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HWND hwnd;
	MSG msg;
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
	CBandiVideoDevice*		bvd;
	CBandiVideoTexture*		bvt;

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

	bvd = new CBandiVideoDevice_DX9();
	if(!bvd || FAILED(bvd->Open(hwnd)))
	{
		MessageBox(NULL, "Error opening device...", WINDOW_TEXT, MB_OK | MB_ICONSTOP);
		DestroyWindow(hwnd);
		if(bvd) delete bvd;
		return -1;
	}

	bvt = new CBandiVideoTexture_DX9((CBandiVideoDevice_DX9*)bvd);
	if(!bvt || FAILED(bvt->Open(info.width , info.height)))
	{
		MessageBox(NULL, "Error opening texture...", WINDOW_TEXT, MB_OK | MB_ICONSTOP);
		DestroyWindow(hwnd);
		if(bvd) delete bvd;
		if(bvt) delete bvt;
		return -1;
	}

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
				INT		pitch;
				BYTE*	buf = bvt->Lock(pitch);
				if(buf)
				{
					// Get frame
					BVL_FRAME frame;
					frame.frame_buf = buf;
					frame.frame_buf_size = info.height*pitch;
					frame.pitch = pitch;
					frame.width = info.width;
					frame.height = info.height;
					frame.pixel_format = bvt->GetFormat();

					bvl.GetFrame(frame, FALSE);
					
					bvt->Unlock();

					// Show frame
					bvd->StartFrame();
					bvt->Draw(0, 0, info.width, info.height);
					bvd->EndFrame();
				}
			}
			else
			{
				Sleep(1);
			}
		}
	}
	while(1);

	bvl.Destroy();

	if(bvd) 
	{
		bvd->Close();
		delete bvd;
	}

	if(bvt)
	{
		bvt->Close();
		delete bvt;
	}

	return 0;
}
