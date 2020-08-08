////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// BandiVideoLibrary 2. Sample
/// 
/// Copyright(C) 2008-2010 BandiSoft.com All rights reserved.
///
/// Visit http://www.bandisoft.com for more information.
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
#define _WIN32_WINNT 0x400

#include <windows.h>
#include <stdio.h>
#include <math.h>

#define USE_NULL_SOUND	0

#include "BandiVideoLibrary.h"
#include "BandiVideoTexture_DX9.h"
#include "BandiVideoFileReader.h"

#if USE_NULL_SOUND
	#include "BandiVideoSound_NULL.h"
#else
	#include "BandiVideoSound_DS.h"
#endif


#pragma warning (disable:4996)

#define WINDOW_TEXT	"BandiVideo Custom Playback"

// global
CBandiVideoLibrary	g_bvl;
INT64				g_total_time = 0;
INT64				g_play_time = 0;
INT					g_volume = 255;


// 상태 정보 업데이트
static void UpdateStatistics(HWND hwnd)
{
	char buf[256];
	int psec = int(g_play_time/10000000);
	int tsec = int(g_total_time/10000000);

	sprintf(buf, "%s - %02d:%02d:%02d / %02d:%02d:%02d", WINDOW_TEXT, psec/3600, (psec/60)%60, psec%60, tsec/3600, (tsec/60)%60, tsec%60);

	SetWindowText(hwnd, buf);
}


LRESULT WINAPI WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CHAR:
		if(wParam == 27)		// esc
		{
			DestroyWindow(hwnd);
		}
		else if(wParam == 32)	// space
		{
			BVL_STATUS status;
			if(SUCCEEDED(g_bvl.GetStatus(status)))
				g_bvl.Pause((status != BVL_STATUS_PAUSED));
		}
		else if(wParam == ',' || wParam == '<')
		{
			g_play_time -= 100000000i64;
			g_bvl.Seek(g_play_time, BVL_SEEK_TIME);
		}
		else if(wParam == '.' || wParam == '>')
		{
			g_play_time += 100000000i64;
			g_bvl.Seek(g_play_time, BVL_SEEK_TIME);
		}
		break;

	case WM_MOUSEWHEEL:
		{
			g_volume += (GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? 10 : -10);
			g_volume = min(max(g_volume, 0), 255);
			g_bvl.SetVolume(g_volume);
		}
		break;

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

		// 재생 상태 표시용 타이머
	case WM_TIMER:
		UpdateStatistics(hwnd);
		break;

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
		wc.lpszClassName = "CustomPlayback";

		if(!RegisterClass(&wc))
		{
			return( 0 );
		}
	}

	return CreateWindow("CustomPlayback", WINDOW_TEXT, 
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

	CBandiVideoDevice*		bvd;
	CBandiVideoTexture*		bvt;

#if USE_NULL_SOUND
	if(FAILED(g_bvl.Create(BANDIVIDEO_DLL_FILE_NAME, new CBandiVideoFileReader(), new CBandiVideoSound_NULL())))
#else
	if(FAILED(g_bvl.Create(BANDIVIDEO_DLL_FILE_NAME, new CBandiVideoFileReader(), new CBandiVideoSound_DS())))
#endif
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

	if(FAILED(g_bvl.Open(lpCmdLine, FALSE)))
	{
		MessageBox(NULL, "Error opening file....", WINDOW_TEXT, MB_OK | MB_ICONSTOP);
		DestroyWindow(hwnd);
		return 3;
	}

	BVL_VIDEO_INFO info;
	if(FAILED(g_bvl.GetVideoInfo(info)))
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

	g_bvl.Play();
	g_total_time = info.total_time;

	ShowWindow(hwnd, nCmdShow);
	SetTimer(hwnd, 0, 1000, NULL);

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
			g_bvl.GetStatus(status);
			if(status == BVL_STATUS_PLAYEND)
			{
				g_bvl.Seek(0, BVL_SEEK_TIME);
				g_bvl.Play();
			}
	
			// 새 프레임을 출력할 시간인가?
			if(g_bvl.IsNextFrame())
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

					g_bvl.GetFrame(frame, TRUE);
					
					bvt->Unlock();

					// Show frame
					bvd->StartFrame();
					bvt->Draw(0, 0, info.width, info.height);
					bvd->EndFrame();

					g_play_time = frame.frame_time;
				}
			}
			else
			{
				Sleep(1);
			}
		}
	}
	while(1);

	g_bvl.Destroy();

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
