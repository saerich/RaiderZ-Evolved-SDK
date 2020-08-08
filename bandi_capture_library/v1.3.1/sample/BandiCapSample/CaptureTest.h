#pragma once

#include "dxstdafx.h"
#include "DXUT.h"
#include "DXUTgui.h"
#include "DXUTmisc.h"
#include "DXUTSettingsDlg.h"
#include "DXUTMesh.h"
#include "sdkmesh.h"
#include "BandiAudio.h"
#include "CaptureConfig.h"
#include "YoutubeUploader.h"

///////////////////////////////////////////////////////////////////////////////////////////
// BANDI SDK :반디캡SDK 해더 추가
#include <bandicap.h>
///////////////////////////////////////////////////////////////////////////////////////////

#define TIMER_PROCESSMESSAGE	1
#define TIMER_BEEP				2

#define	IDC_CAPTURE_STATE		10
#define	IDC_CAPTURE				11
#define	IDC_CONFIG				12
#define	IDC_YOUTUBE				13
#define	IDC_PLAYBEEP			14
#define	IDC_CAPTURE_JPEG		15
#define IDC_CAPTURE_PNG			16


class CCaptureTest
{
public:
	CCaptureTest(void);
	~CCaptureTest(void);

public :
	void					Init(HINSTANCE hInstance, HWND hWnd);
	void					Close();

public :
	void					InitializeDialogs(int iY);
	void					RenderText(CDXUTTextHelper &txtHelper, BOOL bCaptured);
	void					OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl);
	HRESULT					OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc);
	HRESULT					OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc);
	void					OnFrameMove( double fTime, float fElapsedTime);
	LRESULT					MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing);
	void					OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown);
	void					OnD3D9LostDevice();
	void					OnD3D9DestroyDevice();
	void					RenderScene(IDirect3DDevice9* pd3dDevice);
	void					CaptureScene(IDirect3DDevice9* pd3dDevice);

private :
	void					DoConfig();
	BOOL					DoCapture(BOOL bCapture);
	BOOL					DoCapture_Jpeg();
	BOOL					DoCapture_PNG();
	void					Message(LPCTSTR szMsg, ...);
	void					OnTimer(int nTimerID);

private :
	int						m_nWidth;
	int						m_nHeight;

	CAtlList<CAtlString>	m_messages;

	CDXUTButton*			m_pCapBtn;
	CDXUTStatic*			m_pCapStatic;

	HWND					m_hWnd;
	DWORD					m_tickBeep;
	HINSTANCE				m_hInstance;

	IBaCon*					m_pBaCon1;
	IBaCon*					m_pBaCon2;
	CBandiAudioFactory		m_baFactory;
	LPDIRECT3DDEVICE9		m_pd3dDevice;


	CBandiCaptureLibrary	m_bandiCaptureLibrary;
	BOOL					m_bCapture;
	int						m_nCapMode;
	CCaptureConfig			m_Config;
	TCHAR					m_szCaptureFilePathName[MAX_PATH*2];

	BOOL					m_bYoutubeUpload;
	CYoutubeUploader		m_YoutubeUploader;
};

