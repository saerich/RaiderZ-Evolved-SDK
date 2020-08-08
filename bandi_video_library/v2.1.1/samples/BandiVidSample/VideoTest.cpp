////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// BandiVideoLibrary 2.1 Sample
/// 
/// Copyright(C) 2008-2010 BandiSoft.com All rights reserved.
///
/// Visit http://www.bandisoft.com for more information.
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "VideoTest.h"
#include "BandiVideoTexture_DX9.h"


static BOOL GetThisPath(LPCTSTR szFileName, LPTSTR buf, int nLenBuf)
{
	TCHAR szModuleName[MAX_PATH*4];
	::GetModuleFileName(NULL,szModuleName,MAX_PATH*4);
	*(_tcsrchr( szModuleName, '\\' ) + 1) = NULL;

	_sntprintf(buf, nLenBuf, _T("%s%s"), szModuleName, szFileName);
	return TRUE;
}


CVideoTest::CVideoTest(void)
{
	m_nWidth = 0;
	m_nHeight = 0;

	m_hWnd = NULL;
	m_hInstance = NULL;

	m_pd3d9 = NULL;
	m_pd3dDevice = NULL;

	m_bvt = NULL;

	m_current_time = 0;
	m_current_frame = 0;
}

CVideoTest::~CVideoTest(void)
{
	Close();
}

void CVideoTest::Init(HINSTANCE hInstance, HWND hWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hWnd;
	TCHAR	szPathName[MAX_PATH];

	GetThisPath(_T("sample.avi"), szPathName, MAX_PATH);

	// Load bandi video library dll
	if(FAILED(m_bvl.Create(BANDIVIDEO_DLL_FILE_NAME)))
		ASSERT(0);

	// verify
	if(FAILED(m_bvl.Verify("BANDISOFT-TRIAL-200909", "ea92bc5b")))
		ASSERT(0);

	if(Bvl_Open(szPathName) == FALSE)
		ASSERT(0);
}

void CVideoTest::Close()
{
	if(m_bvt)
	{
		delete m_bvt;
		m_bvt = NULL;
	}
}

static int vid_y = 0;
extern CDXUTDialog g_HUD;	// dialog for standard controls
void CVideoTest::InitializeDialogs(int iY)
{
	g_HUD.AddStatic(IDC_INFO,	L"[ Video file ]", 35, iY, 125, 22);
	g_HUD.AddButton(IDC_OPEN,	L"Open", 35, iY += 24, 125, 22, VK_F5);
	g_HUD.AddButton(IDC_CLOSE,	L"Close", 35, iY += 24, 125, 22, VK_F4);

	g_HUD.AddStatic(IDC_INFO,	L"[ Playback ]", 35, iY += 24, 125, 22);
	g_HUD.AddButton(IDC_PLAY,	L"Play", 35, iY += 24, 125, 22, VK_F6);
	g_HUD.AddButton(IDC_PAUSE,	L"Pause", 35, iY += 24, 125, 22, VK_F7);
	g_HUD.AddButton(IDC_STOP,	L"Stop", 35, iY += 24, 125, 22, VK_F8);

	g_HUD.AddButton(IDC_REW,	L"-10sec", 35, iY += 24, 125, 22, VK_F11);
	g_HUD.AddButton(IDC_FF,		L"+10sec", 35, iY += 24, 125, 22, VK_F12);
	g_HUD.AddButton(IDC_FRAME,	L"+frame", 35, iY += 24, 125, 22);

	g_HUD.AddStatic(IDC_INFO,	L"[ Small screen ]", 35, iY += 24, 125, 22);

	vid_y = iY + 24;
}


void CVideoTest::OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl)
{
	switch( nControlID )
	{
	case IDC_OPEN :
		{
			TCHAR file_path[MAX_PATH];

		    OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			file_path[0] = NULL;

			static const TCHAR szFilter[]  
									= TEXT("BVL Video Files (.AVI)\0*.AVI\0") \
									  TEXT("All Files (*.*)\0*.*\0\0");
			ofn.lStructSize         = sizeof(OPENFILENAME);
			ofn.hwndOwner           = m_hWnd;
			ofn.hInstance           = NULL;
			ofn.lpstrFilter         = szFilter;
			ofn.nFilterIndex        = 1;
			ofn.lpstrCustomFilter   = NULL;
			ofn.nMaxCustFilter      = 0;
			ofn.lpstrFile           = file_path;
			ofn.nMaxFile            = MAX_PATH;
			ofn.lpstrFileTitle      = NULL;
			ofn.nMaxFileTitle       = 0;
			ofn.lpstrInitialDir     = NULL;
			ofn.lpstrTitle          = TEXT("Select a video file to play...");
			ofn.Flags               = OFN_HIDEREADONLY;
			ofn.nFileOffset         = 0;
			ofn.nFileExtension      = 0;
			ofn.lpstrDefExt         = TEXT("AVI");
			ofn.lCustData           = 0L;
			ofn.lpfnHook            = NULL;
			ofn.lpTemplateName  = NULL; 
		    
			m_pd3dDevice->SetDialogBoxMode(TRUE);
			if(GetOpenFileName (&ofn))  // user specified a file
			{
				Bvl_Open(file_path);
			}
			m_pd3dDevice->SetDialogBoxMode(FALSE);
			break;
		}
	case IDC_PLAY :
		Bvl_Play();
		break;
	case IDC_PAUSE :
		Bvl_Pause();
		break;
	case IDC_STOP :
		Bvl_Stop();
		break;
	case IDC_REW :
		Bvl_REW();
		break;
	case IDC_FF :
		Bvl_FF();
		break;
	case IDC_FRAME :
		Bvl_Frame();
		break;
	case IDC_CLOSE :
		Bvl_Close();
		break;
	}
}

HRESULT CVideoTest::OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	m_pd3dDevice = pd3dDevice;
	m_pd3dDevice->GetDirect3D(&m_pd3d9);

    return S_OK;
}

HRESULT CVideoTest::OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	m_pd3dDevice = pd3dDevice;
	m_pd3dDevice->GetDirect3D(&m_pd3d9);

	m_nWidth = pBackBufferSurfaceDesc->Width;
	m_nHeight = pBackBufferSurfaceDesc->Height;

	return S_OK;
}

void CVideoTest::OnFrameMove( double fTime, float fElapsedTime)
{
}

LRESULT CVideoTest::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing)
{
	if(uMsg==WM_TIMER)
	{
		OnTimer(wParam);
	}

    return 0;
}

void CVideoTest::OnD3D9LostDevice()
{
	if(m_bvt)
	{
		delete m_bvt;
		m_bvt = NULL;
	}
}

void CVideoTest::OnD3D9DestroyDevice()
{

}


////////////////////////////////////////////////////////////////////////////////////////////////////
///         message to screen
/// @date   Thursday, August 07, 2008  3:26:06 PM
////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoTest::Message(LPCTSTR szMsg, ...)
{
	CAtlString sMsg;
	va_list args;
	va_start(args, szMsg);
	sMsg.FormatV(szMsg, args);
	va_end(args);

	m_messages.AddHead(sMsg);
	SetTimer(m_hWnd, TIMER_PROCESSMESSAGE, 2000, NULL);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
///         process timer
/// @date   Thursday, August 07, 2008  3:26:00 PM
////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoTest::OnTimer(int nTimerID)
{
	if(nTimerID == TIMER_PROCESSMESSAGE)
	{
		if(m_messages.GetCount())
			m_messages.RemoveAll();
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
///          render text after capture processing
/// @param   &txtHelper 
/// @param   bCaptured
/// @date    2008-09-02 14:29:34
////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoTest::RenderText(CDXUTTextHelper &txtHelper)
{
	BVL_STATUS status;
	m_bvl.GetStatus(status);
	if(status >= BVL_STATUS_PLAYING)
	{
		TCHAR s[128];
		INT cmsec = INT(m_current_time/10000i64);
		INT tmsec = INT(m_video_info.total_time/10000i64);
		
		_stprintf(s, _T("[ %02d:%02d:%03d (%I64d) / %02d:%02d:%03d ]"), 
			cmsec/(60*1000), cmsec%(60*1000)/1000, cmsec%1000, m_current_frame,
			tmsec/(60*1000), tmsec%(60*1000)/1000, tmsec%1000);
		
		txtHelper.DrawTextLine(_T(""));
		txtHelper.DrawTextLine(s);
	}

	POSITION p = m_messages.GetHeadPosition();
	while(p)
	{
		txtHelper.DrawTextLine(m_messages.GetNext(p));
	}

}


////////////////////////////////////////////////////////////////////////////////////////////////////
///          render scene 
/// @param   pd3dDevice 
/// @date    2008-09-02 14:28:26
////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoTest::RenderScene( IDirect3DDevice9* pd3dDevice)
{
	// Draw small screen.
	if(m_bvt) m_bvt->Draw(m_nWidth - 125 - 10, vid_y, 125, 100);
}


LPDIRECT3DTEXTURE9 CVideoTest::GetVideoTexture()
{
	if(m_bvl.IsCreated() == FALSE)
		return NULL;

	BVL_STATUS status;
	m_bvl.GetStatus(status);
	if(status == BVL_STATUS_PLAYEND)
	{
		m_bvl.Seek(0, BVL_SEEK_TIME);
		m_bvl.Play();
	}

	if(m_bvl.IsNextFrame())
	{
		if(m_bvt == NULL)
		{
			BV_DEVICE_DX9 bvd = { m_pd3d9, m_pd3dDevice, m_hWnd };
			m_bvt = new CBandiVideoTexture_DX9(&bvd);
			if(!m_bvt || FAILED(m_bvt->Open(m_video_info.width , m_video_info.height)))
			{
				if(m_bvt) delete m_bvt; m_bvt = NULL;
				return NULL;
			}
		}

		INT		pitch;
		BYTE*	buf = m_bvt->Lock(pitch);
		if(buf)
		{
			// Get frame
			BVL_FRAME frame;
			frame.frame_buf = buf;
			frame.frame_buf_size = m_video_info.height*pitch;
			frame.pitch = pitch;
			frame.width = m_video_info.width;
			frame.height = m_video_info.height;
			frame.pixel_format = m_bvt->GetFormat();

			m_bvl.GetFrame(frame, TRUE);
			
			m_bvt->Unlock();

			m_current_time = frame.frame_time;
			m_current_frame = frame.frame_number;
		}
	}

	return m_bvt ? (LPDIRECT3DTEXTURE9)m_bvt->GetObject() : NULL;
}


BOOL CVideoTest::Bvl_Open(LPCTSTR path)
{
	Bvl_Close();

	if(FAILED(m_bvl.Open(path, FALSE)))
	{
		ASSERT(0);
		return FALSE;
	}

	if(FAILED(m_bvl.GetVideoInfo(m_video_info)))
	{
		ASSERT(0);
		return FALSE;
	}

	return SUCCEEDED(m_bvl.Play());
}


void CVideoTest::Bvl_Play()
{
	m_bvl.Seek(0, BVL_SEEK_TIME);
	m_bvl.Play();
}


void CVideoTest::Bvl_Pause()
{
	BVL_STATUS	status;
	if(SUCCEEDED(m_bvl.GetStatus(status)))
	{
		m_bvl.Pause(status != BVL_STATUS_PAUSED);
	}
}


void CVideoTest::Bvl_Stop()
{
	m_bvl.Stop();
}


void CVideoTest::Bvl_FF()
{
	m_bvl.Seek(m_current_time + 100000000i64, BVL_SEEK_TIME);
}


void CVideoTest::Bvl_REW()
{
	m_bvl.Seek(m_current_time - 100000000i64, BVL_SEEK_TIME);
}


void CVideoTest::Bvl_Close()
{
	if(m_bvt)
	{
		delete m_bvt;
		m_bvt = NULL;
	}

	m_bvl.Close();
}


void CVideoTest::Bvl_Frame()
{
	m_bvl.Pause(TRUE);
	m_bvl.Seek(m_current_frame + 1, BVL_SEEK_FRAME);
}
