// mysample.cpp : Defines the entry point for the application.
//
#include "stdafx.h"

#pragma warning(disable : 4996)


#include "mysample.h"
#include <d3d9.h>
#include <mmsystem.h>


#include "base/utility.h"
#include "base/container.h"
#include "base/tu_file.h"
#include "base/tu_types.h"
#include "net/tu_net_file.h"
#include "gameswf/gameswf_types.h"
#include "gameswf/gameswf_impl.h"
#include "gameswf/gameswf_root.h"
#include "gameswf/gameswf_freetype.h"
#include "gameswf/gameswf_player.h"

static bool s_allow_http = false;

static void	message_log(const char* message)
// Process a log message.
{
	if (gameswf::get_verbose_parse())
	{
		fputs(message, stdout);
		fflush(stdout);
	}
}


static void	log_callback(bool error, const char* message)
// Error callback for handling gameswf messages.
{
	if (error)
	{
		// Log, and also print to stderr.
		message_log(message);
		fputs(message, stderr);
		fflush(stderr);
	}
	else
	{
		message_log(message);
	}
}


static tu_file*	file_opener(const char* url)
// Callback function.  This opens files for the gameswf library.
{
	if (s_allow_http) {
		return new_tu_net_file(url, "rb");
	} else {
		return new tu_file(url, "rb");
	}
}


static void	fs_callback(gameswf::character* movie, const char* command, const char* args)
// For handling notification callbacks from ActionScript.
{
//	message_log("fs_callback: '");
//	message_log(command);
//	message_log("' '");
//	message_log(args);
//	message_log("'\n");
}

BOOL InitFlash(IDirect3DDevice9* pDevice);

#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

DWORD SDL_GetTicks()
{
	return timeGetTime();
}

IDirect3DDevice9*		_pDevice = NULL;
IDirect3D9*				pD3D = NULL;
D3DDISPLAYMODE			g_d3ddm;
D3DPRESENT_PARAMETERS	g_d3dpp; 


BOOL InitD3D(HWND hWnd)
{
	pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	HRESULT hr;

	hr=pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &g_d3ddm );

	ZeroMemory( &g_d3dpp, sizeof(g_d3dpp) );
	g_d3dpp.BackBufferWidth = 800;
	g_d3dpp.BackBufferHeight = 600;
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferCount =  1;						// or 2 ??
	g_d3dpp.Windowed   = true;
	g_d3dpp.BackBufferFormat = g_d3ddm.Format;
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	g_d3dpp.Flags=NULL;//D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	g_d3dpp.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;

	HINSTANCE hInstance = GetModuleHandle(NULL);

	hr = pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &_pDevice );
	bool bOK = hr==D3D_OK;
	if(!InitFlash(_pDevice))
		return FALSE;

	return bOK;
}

int	width = 0;
int	height = 0;
float	movie_fps = 0;
float scale_x = 1.f;
float scale_y = 1.f;

static int s_aa_level = 1;

static bool s_antialiased = true;
static bool s_background = true;
static bool s_measure_performance = false;
	float	speed_scale = 1.0f;
	int	frame_counter = 0;
	int	last_logged_fps = 0;
	bool do_render = true;
		bool	do_loop = true;
		int     delay = 10;

	Uint32	last_ticks;

	gameswf::player* player;
smart_ptr<gameswf::root>	m;

	gameswf::sound_handler*	sound = NULL;
	gameswf::render_handler*	render = NULL;


		// Mouse state.
	int	mouse_x = 0;
	int	mouse_y = 0;
	int	mouse_buttons = 0;



BOOL InitFlash(IDirect3DDevice9* pDevice)
{
	gameswf::register_file_opener_callback(file_opener);
	gameswf::register_fscommand_callback(fs_callback);
	if (gameswf::get_verbose_parse())
	{
		gameswf::register_log_callback(log_callback);
	}

	///////////////////////////

	player = new gameswf::player();

	render = gameswf::create_render_handler_d3d(_pDevice);
	gameswf::set_render_handler(render);

	sound = gameswf::create_sound_handler_sdl();
	gameswf::set_sound_handler(sound);

	gameswf::create_glyph_provider();

	const char* infile = "»Ç»Ç»Ç.swf";
//	const char* infile = "segment1.swf";

	m = player->load_file(infile);
	if (m == NULL)
	{
		return FALSE;
//		exit(1);
	}

	if (width == 0 || height == 0)
	{
		width = m->get_movie_width();
		height = m->get_movie_height();
	}
	scale_x = (float) width / m->get_movie_width();
	scale_y = (float) height / m->get_movie_height();

	movie_fps = m->get_movie_fps();

	render->open();
	render->set_antialiased(s_antialiased);


	Uint32	start_ticks = 0;

	if (do_render)
	{
		start_ticks = SDL_GetTicks();

	}
	last_ticks = start_ticks;
	frame_counter = 0;
	last_logged_fps = last_ticks;

	s_antialiased = s_aa_level > 0 ? true : false;

	m->set_display_viewport(0, 0, width, height);
	m->set_background_alpha(s_background ? 1.0f : 0.05f);

	return TRUE;
}

BOOL Destroy()
{
//			delete player;

			gameswf::set_sound_handler(NULL);
			delete sound;

			gameswf::set_render_handler(NULL);
			delete render;

	SAFE_RELEASE(_pDevice);
	SAFE_RELEASE(pD3D);

	return true;
}


BOOL FlashRender()
{
	Uint32	ticks;
	if (do_render)
	{
		ticks = SDL_GetTicks();
	}
	else
	{
		// Simulate time.
		ticks = last_ticks + (Uint32) (1000.0f / movie_fps);
	}
	int	delta_ticks = ticks - last_ticks;
	float	delta_t = delta_ticks / 1000.f;
	last_ticks = ticks;

	//// Check auto timeout counter.
	//if (exit_timeout > 0
	//	&& ticks - start_ticks > (Uint32) (exit_timeout * 1000))
	//{
	//	// Auto exit now.
	//	return FALSE;
	//}

	bool ret = true;

	m = player->get_root();

	m->notify_mouse_state(mouse_x, mouse_y, mouse_buttons);

	Uint32 t_advance = SDL_GetTicks();
	m->advance(delta_t * speed_scale);
	t_advance = SDL_GetTicks() - t_advance;

	// clear

	//if (do_render)
	//{
	//	glDisable(GL_DEPTH_TEST);	// Disable depth testing.
	//	glDrawBuffer(GL_BACK);
	//}


	Uint32 t_display = SDL_GetTicks();
	m->display();
	t_display = SDL_GetTicks() - t_display;

	frame_counter++;

	if (do_render)
	{
		Uint32 t_swap = SDL_GetTicks();
//					SDL_GL_SwapBuffers();
		t_swap = SDL_GetTicks() - t_swap;
		//glPopAttrib ();

		// for perfomance testing
//					printf("advance time: %d, display time %d, swap buffers time = %d\n",
//						t_advance, t_display, t_swap);
//					char buffer[8];
//					snprintf(buffer, 8, "%03d", t_advance);
//					m->set_variable("t_Advance", buffer);
//					snprintf(buffer, 8, "%03d", t_display);
//					m->set_variable("t_Display", buffer);
//					snprintf(buffer, 8, "%03d", t_swap);
//					m->set_variable("t_SwapBuffers", buffer);

		if (s_measure_performance == false)
		{
			// Don't hog the CPU.
//						SDL_Delay(delay);
			Sleep(delay);
		}
		else
		{
			// Log the frame rate every second or so.
			if (last_ticks - last_logged_fps > 1000)
			{
				float	delta = (last_ticks - last_logged_fps) / 1000.f;

				if (delta > 0)
				{
					printf("fps = %3.1f\n", frame_counter / delta);
				}
				else
				{
					printf("fps = *inf*\n");
				}

				last_logged_fps = last_ticks;
				frame_counter = 0;
			}
		}
	}

	// TODO: clean up this interface and re-enable.
	//		gameswf_tesselate_dump_shape = false;  ///xxxxx

	// See if we should exit.
	if (do_loop == false && m->get_current_frame() + 1 == m->get_frame_count())
	{
		// We're reached the end of the movie; exit.
//					break;

		return FALSE;
	}

	return TRUE;
}

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void RAdjustWindow(HWND hWnd, int nWidth, int nHeight, bool bFullScreen)
{
	if((GetWindowLong( hWnd, GWL_STYLE) & WS_CHILD) !=0)
		return;

	if(bFullScreen)
	{
		SetWindowLong( hWnd, GWL_STYLE, WS_POPUP | WS_SYSMENU );
	}
	else
		SetWindowLong( hWnd, GWL_STYLE, WS_POPUP | WS_CAPTION | WS_SYSMENU );

	RECT rt;
	GetClientRect(hWnd,&rt);

	RECT winrt;
	GetWindowRect(hWnd,&winrt);

	MoveWindow(hWnd,winrt.left,winrt.top,
		winrt.right-winrt.left+(nWidth-(rt.right-rt.left)),
		winrt.bottom-winrt.top+(nHeight-(rt.bottom-rt.top)),FALSE);
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MYSAMPLE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		Destroy();
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MYSAMPLE));

    do
    {
		BOOL bGotMsg = PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE );
        if( bGotMsg ) {
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		if(!FlashRender())
			break;
    }while( WM_QUIT != msg.message  );

	Destroy();
	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYSAMPLE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MYSAMPLE);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   RAdjustWindow(hWnd,1024,768,false);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   if(!InitD3D( hWnd))
	   return FALSE;

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_MOUSEMOVE:
		{
			int fwKeys = (int)wParam; 
			int xPos = LOWORD(lParam); 
			int yPos = HIWORD(lParam);
			mouse_x = (int) (xPos / scale_x);
			mouse_y = (int) (yPos / scale_y);
		}
		break;
// 	case SDL_MOUSEBUTTONDOWN:
// 	case SDL_MOUSEBUTTONUP:
// 		{
// 			int	mask = 1 << (event.button.button - 1);
// 			if (event.button.state == SDL_PRESSED)
// 			{
// 				mouse_buttons |= mask;
// 			}
// 			else
// 			{
// 				mouse_buttons &= ~mask;
// 			}
// 			break;
// 		}

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
