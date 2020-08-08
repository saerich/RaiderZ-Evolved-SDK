// An example of an OpenGL animation loop using the Win32 API. Also
// demonstrates palette management for RGB and color index modes and
// general strategies for message handling. 


#include <windows.h>		// must include this before GL/gl.h 
#include <tchar.h>
#include <GL/gl.h>			// OpenGL header file 
#include <stdio.h>
#include <bandicap.h>
#include <math.h>

#define GL_PI (3.141592653589793f)

HDC hDC;						// device context 
HPALETTE hPalette = 0;			// custom palette (if needed) 
GLboolean animate = GL_TRUE;	// animation flag 

CBandiCaptureLibrary	 g_bandiCaptureLibrary;


// 캡처 하기
BOOL DoCapture(BOOL bCapture)
{
	if(g_bandiCaptureLibrary.IsCreated())
	{
		g_bandiCaptureLibrary.Stop();
	}
	else
	{
		if(FAILED(g_bandiCaptureLibrary.Create(BANDICAP_DLL_FILE_NAME)))
			ASSERT(0);

		// verify
		if(FAILED(g_bandiCaptureLibrary.Verify("BANDISOFT-TRIAL-200809", "e1b03d86")))
			ASSERT(0);
	}

	if(bCapture == TRUE)
	{
		HRESULT hr;

		BCAP_CONFIG cfg;

		cfg.VideoSizeW = -1;
		cfg.VideoSizeH = -1;
		
		g_bandiCaptureLibrary.CheckConfig(&cfg);			// 잘못된 설정 바로 잡기
		g_bandiCaptureLibrary.SetConfig(&cfg);				// 설정 적용

		// get (target directory + filename by date)
		TCHAR szCaptureFilePathName[MAX_PATH*2];
		g_bandiCaptureLibrary.MakePathnameByDate(_T("c:\\"), _T("Capture"), _T("avi"), szCaptureFilePathName, MAX_PATH*2);

		// start capture
		hr = g_bandiCaptureLibrary.Start(szCaptureFilePathName, NULL, BCAP_MODE_GL, NULL);
		if(FAILED(hr))
		{
			ASSERT(0);
			g_bandiCaptureLibrary.Stop();
		}
		else if(hr == BC_OK_ONLY_VIDEO)
		{
			ASSERT(0);
		}
		else if(hr == BC_OK_ONLY_AUDIO)
		{
			ASSERT(0);
		}
	}

	return g_bandiCaptureLibrary.IsCapturing();
}

void Render()
{
	// rotate a triangle around 
	glClear(GL_COLOR_BUFFER_BIT);
	if(animate)
		glRotatef(1.0f, 0.0f, 0.0f, 1.0f);

	glBegin(GL_TRIANGLES);
	glIndexi(1);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2i(0,  1);
	glIndexi(2);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex2i(-1, -1);
	glIndexi(3);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex2i(1, -1);
	glEnd();
	glFlush();

	// 캡처 처리
	if(g_bandiCaptureLibrary.IsCapturing())
	{
		g_bandiCaptureLibrary.Work(NULL);

		// 캡처중임을 알리는 메시지 출력
		RECT rect;
		::GetClientRect(::WindowFromDC(hDC), &rect);

		glPushAttrib(GL_ALL_ATTRIB_BITS);

		glViewport(0, rect.bottom - 64, 64, 64);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0.0, 64, 64, 0.0 , -1.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_1D);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_DEPTH_TEST);
		glShadeModel(GL_FLAT);
		glColor3f(1.0f, 0.0f, 0.0f);

		glBegin(GL_TRIANGLE_FAN);

		GLfloat x,y,angle; // 좌표값과 각도 저장
        for(angle = 0.0f; angle < (2.0f*GL_PI); angle += (GL_PI/8.0f))
        {
                x=12.0f*sin(angle) + 24.0f;
                y=12.0f*cos(angle) + 24.0f;
                glVertex2f(x,y);
        }

        x=12.0f*sin(angle) + 24.0f;
        y=12.0f*cos(angle) + 24.0f;
        glVertex2f(x,y);
		glEnd();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		glPopAttrib();
	}

	SwapBuffers(hDC);			// nop if singlebuffered 
}


LONG WINAPI WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{ 
	static PAINTSTRUCT ps;

	switch(uMsg) {
	case WM_PAINT: 
		Render();
		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		return 0;

	case WM_SIZE:
		glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
		PostMessage(hWnd, WM_PAINT, 0, 0);
		return 0;

	case WM_KEYDOWN:
		if(wParam == VK_SCROLL)
		{
			DoCapture(!g_bandiCaptureLibrary.IsCapturing());
		}
		break;

	case WM_CHAR:
		switch (wParam) {
			case 27:			// ESC key 
				PostQuitMessage(0);
				break;
			case ' ':
				animate = !animate;
				break;
		}
		return 0;

	case WM_ACTIVATE:
		if (IsIconic(hWnd))
			animate = GL_FALSE;
		else
			animate = GL_TRUE;
		return 0;

	case WM_PALETTECHANGED:
		if (hWnd == (HWND)wParam)
			break;
		// fall through to WM_QUERYNEWPALETTE 

	case WM_QUERYNEWPALETTE:
		if (hPalette) {
			UnrealizeObject(hPalette);
			SelectPalette(hDC, hPalette, FALSE);
			RealizePalette(hDC);
			return TRUE;
		}
		return FALSE;

	case WM_CLOSE:
		PostMessage(hWnd, WM_QUIT, NULL, NULL);
		return TRUE;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam); 
} 


HWND CreateOpenGLWindow(char* title, int x, int y, int width, int height, 
						BYTE type, DWORD flags)
{
	int         n, pf;
	HWND        hWnd;
	WNDCLASS    wc;
	LOGPALETTE* lpPal;
	PIXELFORMATDESCRIPTOR pfd;
	static HINSTANCE hInstance = 0;

	// only register the window class once - use hInstance as a flag. 
	if (!hInstance) {
		hInstance = GetModuleHandle(NULL);
		wc.style         = CS_OWNDC;
		wc.lpfnWndProc   = (WNDPROC)WindowProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.hInstance     = hInstance;
		wc.hIcon         = LoadIcon(NULL, IDI_WINLOGO);
		wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = "OpenGL";

		if (!RegisterClass(&wc)) {
			MessageBox(NULL, "RegisterClass() failed:  "
				"Cannot register window class.", "Error", MB_OK);
			return NULL;
		}
	}

	hWnd = CreateWindow("OpenGL", title, WS_OVERLAPPEDWINDOW |
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		x, y, width, height, NULL, NULL, hInstance, NULL);

	if (hWnd == NULL) {
		MessageBox(NULL, "CreateWindow() failed:  Cannot create a window.",
			"Error", MB_OK);
		return NULL;
	}

	hDC = GetDC(hWnd);

	// there is no guarantee that the contents of the stack that become
	// the pfd are zeroed, therefore _make sure_ to clear these bits. 
	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize        = sizeof(pfd);
	pfd.nVersion     = 1;
	pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | flags;
	pfd.iPixelType   = type;
	pfd.cColorBits   = 32;

	pf = ChoosePixelFormat(hDC, &pfd);
	if (pf == 0) {
		MessageBox(NULL, "ChoosePixelFormat() failed:  "
			"Cannot find a suitable pixel format.", "Error", MB_OK); 
		return 0;
	} 

	if (SetPixelFormat(hDC, pf, &pfd) == FALSE) {
		MessageBox(NULL, "SetPixelFormat() failed:  "
			"Cannot set format specified.", "Error", MB_OK);
		return 0;
	} 

	DescribePixelFormat(hDC, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

	if (pfd.dwFlags & PFD_NEED_PALETTE ||
		pfd.iPixelType == PFD_TYPE_COLORINDEX) {

			n = 1 << pfd.cColorBits;
			if (n > 256) n = 256;

			lpPal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) +
				sizeof(PALETTEENTRY) * n);
			memset(lpPal, 0, sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * n);
			lpPal->palVersion = 0x300;
			lpPal->palNumEntries = n;

			GetSystemPaletteEntries(hDC, 0, n, &lpPal->palPalEntry[0]);

			// if the pixel type is RGBA, then we want to make an RGB ramp,
			// otherwise (color index) set individual colors. 
			if (pfd.iPixelType == PFD_TYPE_RGBA) {
				int redMask = (1 << pfd.cRedBits) - 1;
				int greenMask = (1 << pfd.cGreenBits) - 1;
				int blueMask = (1 << pfd.cBlueBits) - 1;
				int i;

				// fill in the entries with an RGB color ramp. 
				for (i = 0; i < n; ++i) {
					lpPal->palPalEntry[i].peRed = 
						(((i >> pfd.cRedShift)   & redMask)   * 255) / redMask;
					lpPal->palPalEntry[i].peGreen = 
						(((i >> pfd.cGreenShift) & greenMask) * 255) / greenMask;
					lpPal->palPalEntry[i].peBlue = 
						(((i >> pfd.cBlueShift)  & blueMask)  * 255) / blueMask;
					lpPal->palPalEntry[i].peFlags = 0;
				}
			} else {
				lpPal->palPalEntry[0].peRed = 0;
				lpPal->palPalEntry[0].peGreen = 0;
				lpPal->palPalEntry[0].peBlue = 0;
				lpPal->palPalEntry[0].peFlags = PC_NOCOLLAPSE;
				lpPal->palPalEntry[1].peRed = 255;
				lpPal->palPalEntry[1].peGreen = 0;
				lpPal->palPalEntry[1].peBlue = 0;
				lpPal->palPalEntry[1].peFlags = PC_NOCOLLAPSE;
				lpPal->palPalEntry[2].peRed = 0;
				lpPal->palPalEntry[2].peGreen = 255;
				lpPal->palPalEntry[2].peBlue = 0;
				lpPal->palPalEntry[2].peFlags = PC_NOCOLLAPSE;
				lpPal->palPalEntry[3].peRed = 0;
				lpPal->palPalEntry[3].peGreen = 0;
				lpPal->palPalEntry[3].peBlue = 255;
				lpPal->palPalEntry[3].peFlags = PC_NOCOLLAPSE;
			}

			hPalette = CreatePalette(lpPal);
			if (hPalette) {
				SelectPalette(hDC, hPalette, FALSE);
				RealizePalette(hDC);
			}

			free(lpPal);
	}

	ReleaseDC(hWnd, hDC);

	MessageBox(NULL, _T("BCL Sample for OpenGL.\n\n") 
						_T("Press 'scroll-lock' that is toggle(start/stop) key,\n") 
						_T("and the output file will be created in the \"c:\\\".\n") 
						, _T("Animate"), MB_OK);
	return hWnd;
}    


int APIENTRY WinMain(HINSTANCE hCurrentInst, HINSTANCE hPreviousInst,
					 LPSTR lpszCmdLine, int nCmdShow)
{
	HGLRC hRC;				// opengl context 
	HWND  hWnd;				// window 
	MSG   msg;				// message 
	DWORD buffer = PFD_DOUBLEBUFFER;	// buffering type 
	BYTE  color  = PFD_TYPE_RGBA;		// color type 

	if (strstr(lpszCmdLine, "-sb")) {
		buffer = 0;
	} 
	if (strstr(lpszCmdLine, "-ci")) {
		color = PFD_TYPE_COLORINDEX;
	} 
	if (strstr(lpszCmdLine, "-h")) {
		MessageBox(NULL, "animate [-ci] [-sb]\n"
			"  -sb   single buffered\n"
			"  -ci   color index\n",
			"Usage help", MB_ICONINFORMATION);
		exit(0);
	}

	hWnd = CreateOpenGLWindow("animate", 0, 0, 800, 630, color, buffer);
	if (hWnd == NULL)
		exit(1);

	hDC = GetDC(hWnd);
	hRC = wglCreateContext(hDC);
	wglMakeCurrent(hDC, hRC);

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	while (1) 
	{
		while(PeekMessage(&msg, hWnd, 0, 0, PM_NOREMOVE)) 
		{
			if(WM_QUIT == msg.message)
				goto quit;

			if(GetMessage(&msg, hWnd, 0, 0)) 
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			} 
			else 
			{
				goto quit;
			}
		}

		Render();
	}

quit:
	g_bandiCaptureLibrary.Destroy();

	wglMakeCurrent(NULL, NULL);
	ReleaseDC(hWnd, hDC);
	wglDeleteContext(hRC);
	DestroyWindow(hWnd);
	if (hPalette)
		DeleteObject(hPalette);

	return 0;
}

