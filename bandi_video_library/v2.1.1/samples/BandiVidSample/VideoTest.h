////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// BandiVideoLibrary 2.1 Sample
/// 
/// Copyright(C) 2008-2010 BandiSoft.com All rights reserved.
///
/// Visit http://www.bandisoft.com for more information.
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "dxstdafx.h"
#include "DXUT.h"
#include "DXUTgui.h"
#include "DXUTmisc.h"
#include "DXUTSettingsDlg.h"
#include "DXUTMesh.h"
#include "sdkmesh.h"

#include "BandiVideoLibrary.h"

#define TIMER_PROCESSMESSAGE	1

#define	IDC_INFO				10
#define	IDC_OPEN				11
#define	IDC_PLAY				12
#define	IDC_PAUSE				13
#define	IDC_STOP				14
#define	IDC_REW					15
#define	IDC_FF					16
#define	IDC_FRAME				17
#define	IDC_CLOSE				18


class CBandiVideoTexture;
class CVideoTest
{
public:
	CVideoTest(void);
	~CVideoTest(void);

public :
	void					Init(HINSTANCE hInstance, HWND hWnd);
	void					Close();

public :
	void					InitializeDialogs(int iY);
	void					RenderText(CDXUTTextHelper &txtHelper);
	void					OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl);
	HRESULT					OnD3D9CreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc);
	HRESULT					OnD3D9ResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc);
	void					OnFrameMove(double fTime, float fElapsedTime);
	LRESULT					MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing);
	void					OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown);
	void					OnD3D9LostDevice();
	void					OnD3D9DestroyDevice();
	void					RenderScene(IDirect3DDevice9* pd3dDevice);

	LPDIRECT3DTEXTURE9		GetVideoTexture();

private :
	void					Message(LPCTSTR szMsg, ...);
	void					OnTimer(int nTimerID);

	BOOL					Bvl_Open(LPCTSTR path);
	void					Bvl_Play();
	void					Bvl_Pause();
	void					Bvl_Stop();
	void					Bvl_FF();
	void					Bvl_REW();
	void					Bvl_Frame();
	void					Bvl_Close();

private :
	int						m_nWidth;
	int						m_nHeight;

	CAtlList<CAtlString>	m_messages;

	HWND					m_hWnd;
	HINSTANCE				m_hInstance;

	LPDIRECT3D9				m_pd3d9;
	LPDIRECT3DDEVICE9		m_pd3dDevice;

	CBandiVideoLibrary		m_bvl;
	CBandiVideoTexture*		m_bvt;

	LONGLONG				m_current_time;
	LONGLONG				m_current_frame;

	BVL_VIDEO_INFO			m_video_info;
};
