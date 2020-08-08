/*-------------------------------------------------------------------*//*!
 *
 * Umbra Occlusion Booster
 * -----------------------------------------
 *
 * (C) 2009-2010 Umbra Software Ltd. 
 * All Rights Reserved.
 *
 *  
 * This file consists of unpublished, proprietary source code of
 * Umbra Software, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irreparable harm to 
 * Umbra Software and legal action against the party in breach.
 *
 * \file
 * \brief     Direct3D 10 entry point
 *
 *//*-------------------------------------------------------------------*/

#include <windows.h>
#include <windowsx.h>
#include <map>
#include "platform.hpp"

#pragma warning(disable:4996) // function was declared deprecated

// Global Variables:
HINSTANCE hInst;                        
TCHAR* szTitle          = TEXT("Umbra Sample");
TCHAR* szWindowClass    = TEXT("SampleClassName");

// Keyboard state
extern unsigned int keyboardAlphabetBitmap;
extern unsigned int keyboardSymbolsBitmap;

// Mouse buttons bitmap
extern unsigned int mouseButtonBitmap;

// True if window is active
extern bool	windowActive;

// Mappings from virtual keys to platform constants
extern std::map<unsigned int, Platform::KeyboardSymbol> symbolVirtualKeys;
extern std::map<unsigned int, Platform::KeyboardAlphabet> alphabetVirtualKeys;

/*****************************************************************************
 *
 * Function:        WndProc()
 *
 * Description:     Window-procedure
 *
 * Parameters:      hWnd        = window handle
 *                  message     = message 
 *                  wParam      = WORD param
 *                  lParam      = LONG param
 *
 * Returns:         result
 *
 *****************************************************************************/

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) 
    {

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

	case WM_KEYDOWN: 
	{

		// Try mapping the key value to a KeyboardSymbol enum constant.
        std::map<unsigned int, Platform::KeyboardSymbol>::iterator symbol = symbolVirtualKeys.find((unsigned int)wParam);
        if(symbol != symbolVirtualKeys.end()) 
        {
            // If a mapping was found, toggle the corresponding bit.
            keyboardSymbolsBitmap |= symbol->second;
			return 0;
        }

        // Similarly for alphabet.
        std::map<unsigned int, Platform::KeyboardAlphabet>::iterator alphabet = alphabetVirtualKeys.find((unsigned int)wParam);
        if(alphabet != alphabetVirtualKeys.end())
            keyboardAlphabetBitmap |= alphabet->second;

		return 0;
	}

	case WM_KEYUP: 
	{

		// Try mapping the key value to a KeyboardSymbol enum constant.
        std::map<unsigned int, Platform::KeyboardSymbol>::iterator symbol = symbolVirtualKeys.find((unsigned int)wParam);
        if(symbol != symbolVirtualKeys.end()) 
        {
            // If a mapping was found, toggle the corresponding bit.
            keyboardSymbolsBitmap &= ~symbol->second;
            return 0;
        }

        // Similarly for alphabet.
        std::map<unsigned int, Platform::KeyboardAlphabet>::iterator alphabet = alphabetVirtualKeys.find((unsigned int)wParam);
        if(alphabet != alphabetVirtualKeys.end())
            keyboardAlphabetBitmap &= ~alphabet->second;

		return 0;
	}

	case WM_ACTIVATE:
	{
		if(wParam == WA_INACTIVE)
		{
			ShowCursor(TRUE);
			windowActive = false;
		}
		else
		{
			ShowCursor(FALSE);
			windowActive = true;
		}

		return 0;
	}

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
   }
}

/*****************************************************************************
 *
 * Function:        MyRegisterClass()
 *
 * Description:     Registers the window class
 *
 * Parameters:      hInstance
 *
 * Returns:         Window class atom   
 *
 * Notes:           This function and its usage is only necessary if you want this code
 *                  to be compatible with Win32 systems prior to the 'RegisterClassEx'
 *                  function that was added to Windows 95. It is important to call this function
 *                  so that the application will get 'well formed' small icons associated
 *                  with it.
 *
 *****************************************************************************/

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX); 

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = NULL;
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = NULL;

    return RegisterClassEx(&wcex);
}

/*****************************************************************************
 *
 * Function:        InitInstance()
 *
 * Description:     Saves instance handle and creates main window
 *
 * Parameters:      videoMode
 *                  hInstance
 *                  nCmdShow
 *
 * Returns:         window handle of the created window
 *
 * Notes:           In this function, we save the instance handle in a global variable and
 *                  create and display the main program window.
 *
 *****************************************************************************/

HWND InitInstance(const Platform::VideoMode &videoMode, HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, videoMode.width, videoMode.height, NULL, NULL, hInstance, NULL);

   if(!hWnd)
      return NULL;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return hWnd;
}

/*****************************************************************************
 *
 * Function:        ParseCommandLine()
 *
 * Description:     Converts the command line from GetCommandLine to a string
					vector.
 *
 * Parameters:      args
 *
 *****************************************************************************/

static void ParseCommandLine(std::vector<std::string> &args) 
{
	// Pre-parse command line using CommandLineToArgvW, since it handles
	// quotation marks correctly. CommandLineToArgvW is ony available 
	// as wide version, so we need to convert the output to ansi
	// using WideCharToMultiByte.

	LPWSTR*		argv;
    int			argc;
	
	argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	for(int i = 0; i < argc; i++)
	{
		int lenW = (int)wcslen(argv[i]);
		// Get ansi string length
		int lenA = WideCharToMultiByte(CP_ACP, 0, argv[i], lenW, 0, 0, NULL, NULL);
		std::string arg;

		if(lenA > 0)
		{
			char *ansi = new char[lenA+1];
			ansi[lenA] = '\0';
			// Do the actual conversion
			WideCharToMultiByte(CP_ACP, 0, argv[i], lenW, ansi, lenA, NULL, NULL);
			arg = std::string(ansi);
			delete [] ansi;
		}

		args.push_back(arg);
	}

	LocalFree(argv);
}

/*****************************************************************************
 *
 * Function:        WinMain()
 *
 * Description:     entry point
 *
 *****************************************************************************/

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE /*hPrevInstance*/,
                     LPSTR     /*lpCmdLine*/,
                     int       nCmdShow)
{   
    MSG msg;
    HACCEL hAccelTable = NULL;
	Platform::VideoMode videoMode;

	getVideoMode(videoMode);

	// Initialize global strings

    MyRegisterClass(hInstance);

    HWND hWnd = InitInstance (videoMode, hInstance, nCmdShow);
    if(!hWnd)
        return FALSE;

	std::vector<std::string> args;
	ParseCommandLine(args);

	Platform::setCommandLine(args);

	// Perform platform initialization

	Platform::init(videoMode, hWnd);

    // Perform application initialization

    initApplication();

    // Main message loop

    for(;;)
    {
        if(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
        {
            if(GetMessage(&msg, NULL, 0, 0))
            {
                if(!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
            else
            {
                break;
            }
        } 
        else
        {
            if(!updateApplication())
                break;
        }
    }

    exitApplication();

	Platform::exit(); 

    return (int)msg.wParam;
}
//------------------------------------------------------------------------
