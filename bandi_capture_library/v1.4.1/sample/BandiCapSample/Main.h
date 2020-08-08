////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// main object class
/// 
/// @author   park
/// @date     Thursday, August 07, 2008  10:23:55 AM
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "dxstdafx.h"
#include "DXUT.h"
#include "DXUTgui.h"
#include "DXUTmisc.h"
//#include "DXUTCamera.h"
#include "DXUTSettingsDlg.h"
//#include "misc.h"
//#include "Mesh10.h"
#include "DXUTMesh.h"
#include "sdkmesh.h"

#include "BandiAudio.h"

#define IDC_TOGGLEFULLSCREEN 1
#define IDC_TOGGLEREF        3
#define IDC_CHANGEDEVICE     4
#define IDC_CHECKBOX         5
#define IDC_LIGHTPERSPECTIVE 6
#define IDC_ATTACHLIGHTTOCAR 7
#define	IDC_TEST			 10

#define TIMER_PROCESSMESSAGE	1
#define TIMER_BEEP				2

#define NUM_OBJ 14

//-----------------------------------------------------------------------------
// Name: class CObj
// Desc: Encapsulates a mesh object in the scene by grouping its world matrix
//       with the mesh.
//-----------------------------------------------------------------------------
struct CObj
{
    CDXUTMesh m_Mesh;
    D3DXMATRIXA16 m_mWorld;
};




//-----------------------------------------------------------------------------
// Name: class CViewCamera
// Desc: A camera class derived from CFirstPersonCamera.  The arrow keys and
//       numpad keys are disabled for this type of camera.
//-----------------------------------------------------------------------------
class CViewCamera : public CFirstPersonCamera
{
protected:
    virtual D3DUtil_CameraKeys MapKey( UINT nKey )
    {
        // Provide custom mapping here.
        // Same as default mapping but disable arrow keys.
        switch( nKey )
        {
            case 'A':      return CAM_STRAFE_LEFT;
            case 'D':      return CAM_STRAFE_RIGHT;
            case 'W':      return CAM_MOVE_FORWARD;
            case 'S':      return CAM_MOVE_BACKWARD;
            case 'Q':      return CAM_MOVE_DOWN;
            case 'E':      return CAM_MOVE_UP;

            case VK_HOME:   return CAM_RESET;
        }

        return CAM_UNKNOWN;
    }
};




//-----------------------------------------------------------------------------
// Name: class CLightCamera
// Desc: A camera class derived from CFirstPersonCamera.  The letter keys
//       are disabled for this type of camera.  This class is intended for use
//       by the spot light.
//-----------------------------------------------------------------------------
class CLightCamera : public CFirstPersonCamera
{
protected:
    virtual D3DUtil_CameraKeys MapKey( UINT nKey )
    {
        // Provide custom mapping here.
        // Same as default mapping but disable arrow keys.
        switch( nKey )
        {
            case VK_LEFT:  return CAM_STRAFE_LEFT;
            case VK_RIGHT: return CAM_STRAFE_RIGHT;
            case VK_UP:    return CAM_MOVE_FORWARD;
            case VK_DOWN:  return CAM_MOVE_BACKWARD;
            case VK_PRIOR: return CAM_MOVE_UP;        // pgup
            case VK_NEXT:  return CAM_MOVE_DOWN;      // pgdn

            case VK_NUMPAD4: return CAM_STRAFE_LEFT;
            case VK_NUMPAD6: return CAM_STRAFE_RIGHT;
            case VK_NUMPAD8: return CAM_MOVE_FORWARD;
            case VK_NUMPAD2: return CAM_MOVE_BACKWARD;
            case VK_NUMPAD9: return CAM_MOVE_UP;        
            case VK_NUMPAD3: return CAM_MOVE_DOWN;      

            case VK_HOME:   return CAM_RESET;
        }

        return CAM_UNKNOWN;
    }
};


class CMain
{
public:
	CMain(void);
	~CMain(void);

public :
	void						InitApp(HINSTANCE hInstance);
	void						RenderText();
	void						ExitApp();

public :
	static void CALLBACK		_OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl);

public :
	void						OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl);
	HRESULT						OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc);
	HRESULT						OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc);
	void						OnFrameMove( double fTime, float fElapsedTime);
	void						OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime);
	LRESULT						MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing);
	void						OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown);
	void						OnD3D9LostDevice();
	void						OnD3D9DestroyDevice();

private :
	void						DoTest();
	void						Message(LPCTSTR szMsg, ...);
	void						OnTimer(int nTimerID);
	void						RenderScene( IDirect3DDevice9* pd3dDevice, bool bRenderShadow, float fElapsedTime, const D3DXMATRIX *pmView, const D3DXMATRIX *pmProj );


private :
	CDXUTTextHelper*            m_pTxtHelper;


	ID3DXSprite*                m_pSprite9;

	D3DMATERIAL9				m_teapotMtrl;
	LPD3DXMESH					g_pTeapotMesh;
	D3DLIGHT9					g_pLight;
	CDXUTXFileMesh				m_mesh1;
	CDXUTXFileMesh				m_mesh2;

	int							m_nWidth;
	int							m_nHeight;
	float						g_fSpinX;
	float						g_fSpinY;

	CAtlList<CAtlString>		m_messages;
	HWND						m_hWnd;
	BOOL						m_bNowBeep;
	HINSTANCE					m_hInstance;

	LPDIRECT3DVERTEXBUFFER9		g_pLineList_VB;

	IBaCon*						m_pBaCon1;
	CBandiAudioFactory			m_baFactory;


	ID3DXFont*              g_pFont;
	ID3DXFont*              g_pFontSmall;
	ID3DXSprite*            g_pTextSprite;
	ID3DXEffect*            g_pEffect;
	bool                    g_bShowHelp;
	CDXUTDialog             g_HUD;                  // dialog for standard controls
	CFirstPersonCamera      g_VCamera;              // View camera
	CFirstPersonCamera      g_LCamera;              // Camera obj to help adjust light
	CObj                    g_Obj[NUM_OBJ];         // Scene object meshes
	LPDIRECT3DVERTEXDECLARATION9 g_pVertDecl;
	LPDIRECT3DTEXTURE9      g_pTexDef;
	D3DLIGHT9               g_Light;                // The spot light in the scene
	CDXUTMesh               g_LightMesh;
	LPDIRECT3DTEXTURE9      g_pShadowMap;
	LPDIRECT3DSURFACE9      g_pDSShadow;
	float                   g_fLightFov;            // FOV of the spot light (in radian)
	D3DXMATRIXA16           g_mShadowProj;          // Projection matrix for shadow map

	bool                    g_bRightMouseDown;
	bool                    g_bCameraPerspective;
	bool                    g_bFreeLight;


};

