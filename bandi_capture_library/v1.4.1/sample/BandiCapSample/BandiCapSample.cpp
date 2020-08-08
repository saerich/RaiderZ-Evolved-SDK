#include "stdafx.h"
#include "BandiCapSample.h"
#include "Main.h"

#pragma comment(lib, "dxerr9.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "dsound.lib")

CMain	g_main;

bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed );
void CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps );
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc );
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc );
void CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime );
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime );
void RenderText();
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing );
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown );
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl );
void CALLBACK OnLostDevice();
void CALLBACK OnDestroyDevice();


int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    DXUTSetCallbackDeviceCreated( OnCreateDevice );
    DXUTSetCallbackDeviceReset( OnResetDevice );
    DXUTSetCallbackDeviceLost( OnLostDevice );
    DXUTSetCallbackDeviceDestroyed( OnDestroyDevice );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( KeyboardProc );
    DXUTSetCallbackFrameRender( OnFrameRender );
    DXUTSetCallbackFrameMove( OnFrameMove );

    // Show the cursor and clip it when in full screen
    DXUTSetCursorSettings( true, true );

    DXUTInit( true, true, true ); // Parse the command line, handle the default hotkeys, and show msgboxes
    DXUTCreateWindow( L"BandiCapSample App" );
    DXUTCreateDevice( D3DADAPTER_DEFAULT, true, 1024, 768, IsDeviceAcceptable, ModifyDeviceSettings );
	g_main.InitApp(hInstance);

    DXUTMainLoop();

	g_main.ExitApp();

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning false.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed )
{
    // Skip backbuffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3DObject(); 
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                    AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
                    D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;

    // Must support pixel shader 2.0
    if( pCaps->PixelShaderVersion < D3DPS_VERSION( 2, 0 ) )
        return false;

    // need to support D3DFMT_R32F render target
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                    AdapterFormat, D3DUSAGE_RENDERTARGET, 
                    D3DRTYPE_CUBETEXTURE, D3DFMT_R32F ) ) )
        return false;

    // need to support D3DFMT_A8R8G8B8 render target
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                    AdapterFormat, D3DUSAGE_RENDERTARGET, 
                    D3DRTYPE_CUBETEXTURE, D3DFMT_A8R8G8B8 ) ) )
        return false;

    return true;
}


//--------------------------------------------------------------------------------------
// This callback function is called immediately before a device is created to allow the 
// application to modify the device settings. The supplied pDeviceSettings parameter 
// contains the settings that the framework has selected for the new device, and the 
// application can make any desired changes directly to this structure.  Note however that 
// the sample framework will not correct invalid device settings so care must be taken 
// to return valid device settings, otherwise IDirect3D9::CreateDevice() will fail.  
//--------------------------------------------------------------------------------------
void CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps )
{
    // If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
    // then switch to SWVP.
    if( (pCaps->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 ||
         pCaps->VertexShaderVersion < D3DVS_VERSION(1,1) )
    {
        pDeviceSettings->BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }
    else
    {
        pDeviceSettings->BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
    }

    // This application is designed to work on a pure device by not using 
    // IDirect3D9::Get*() methods, so create a pure device if supported and using HWVP.
    if ((pCaps->DevCaps & D3DDEVCAPS_PUREDEVICE) != 0 && 
        (pDeviceSettings->BehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING) != 0 )
        pDeviceSettings->BehaviorFlags |= D3DCREATE_PUREDEVICE;

    // Debugging vertex shaders requires either REF or software vertex processing 
    // and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
    if( pDeviceSettings->DeviceType != D3DDEVTYPE_REF )
    {
        pDeviceSettings->BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
        pDeviceSettings->BehaviorFlags &= ~D3DCREATE_PUREDEVICE;                            
        pDeviceSettings->BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }
#endif
#ifdef DEBUG_PS
    pDeviceSettings->DeviceType = D3DDEVTYPE_REF;
#endif

}

HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc )
{
	return g_main.OnD3D9CreateDevice(pd3dDevice, pBackBufferSurfaceDesc);
}

HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice,  const D3DSURFACE_DESC* pBackBufferSurfaceDesc )
{
	return g_main.OnD3D9ResetDevice(pd3dDevice, pBackBufferSurfaceDesc);
}

void CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime )
{
	g_main.OnFrameMove(fTime, fElapsedTime);
}

void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime )
{
	g_main.OnD3D9FrameRender(pd3dDevice, fTime, fElapsedTime );
}

void RenderText()
{
	g_main.RenderText();
}

LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing )
{
	return g_main.MsgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing );
}

void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown )
{
	g_main.OnKeyboard(nChar, bKeyDown, bAltDown);
}


void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl )
{
	g_main.OnGUIEvent(nEvent, nControlID, pControl);
}

void CALLBACK OnLostDevice()
{
	g_main.OnD3D9LostDevice();
}

void CALLBACK OnDestroyDevice()
{
	g_main.OnD3D9DestroyDevice();
}
