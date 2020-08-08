#include "StdAfx.h"
#include "CaptureTest.h"
#include "DemoMain.h"
#include "CaptureUpload.h"

BOOL GetThisPath(LPCTSTR szFileName, LPTSTR buf, int nLenBuf)
{
	TCHAR szModuleName[MAX_PATH*4];
	::GetModuleFileName(NULL,szModuleName,MAX_PATH*4);
	*(_tcsrchr( szModuleName, '\\' ) + 1) = NULL;

	_sntprintf(buf, nLenBuf, _T("%s%s"), szModuleName, szFileName);
	return TRUE;
}


CCaptureTest::CCaptureTest(void)
{
	m_pCapBtn = NULL;
	m_pCapStatic = NULL;

	m_hWnd = NULL;
	m_hInstance = NULL;
	m_pBaCon1 = NULL;
	m_pBaCon2 = NULL;
	m_tickBeep = 0;
	m_pd3dDevice = NULL;

	m_bCapture = FALSE;
	m_szCaptureFilePathName[0] = 0;

	m_bYoutubeUpload = FALSE;
}

CCaptureTest::~CCaptureTest(void)
{
	Close();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
///          Initialize bandicapture
/// @param   hInstance 
/// @param   hWnd 
/// @date    2008-09-02 14:31:54
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCaptureTest::Init(HINSTANCE hInstance, HWND hWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hWnd;
	TCHAR	szOggPathName[MAX_PATH];


	GetThisPath(_T("sound\\The Blue Danube.ogg"), szOggPathName, MAX_PATH);

	// audio initialize
	if(m_pBaCon1==NULL)
	{
		if(m_baFactory.Create(BANDIAUDIO_RELEASE_DLL_FILE_NAME)==FALSE) ASSERT(0);
		m_pBaCon1 = m_baFactory.CreateBaCon();
		if(m_pBaCon1)
		{
			m_pBaCon1->Create(m_hInstance, m_hWnd);
			m_pBaCon1->Play(szOggPathName);
			m_pBaCon1->SetVolume256(255);
			m_pBaCon1->SetOption(BAOPT_LOOP);
		}

		m_pBaCon2 = m_baFactory.CreateBaCon();
		if(m_pBaCon2)
		{
			m_pBaCon2->Create(m_hInstance, m_hWnd);
		}

		else ASSERT(0);
	}

	// LOAD BANDI CAPTURE LIBRARY DLL
	if(FAILED(m_bandiCaptureLibrary.Create(BANDICAP_DLL_FILE_NAME)))
		ASSERT(0);

	// verify
	if(FAILED(m_bandiCaptureLibrary.Verify("BANDISOFT-TRIAL-200809", "e1b03d86")))
		ASSERT(0);

}

void CCaptureTest::Close()
{
	SAFE_RELEASE(m_pBaCon1);
	SAFE_RELEASE(m_pBaCon2);
	m_baFactory.Destroy();
}


void CCaptureTest::InitializeDialogs(int iY)
{
	g_HUD.AddStatic(IDC_CAPTURE_STATE, L"Capturing...", 35, iY, 125, 22, false, &m_pCapStatic);
	g_HUD.AddButton(IDC_CAPTURE, L"Capture(F11,ScrLock)", 35, iY += 24, 125, 50, VK_F11, false, &m_pCapBtn);
	g_HUD.AddCheckBox(IDC_YOUTUBE, L"YouTube Upload", 35, iY += 52, 125, 22, false);

	g_HUD.AddButton(IDC_CONFIG, L"Config (F9)", 35, iY += 52, 125, 22, VK_F9);
	g_HUD.AddCheckBox(IDC_PLAYBEEP, L"Beep-effect", 35, iY += 24, 125, 22, false);

	g_HUD.AddButton(IDC_CAPTURE_JPEG, L"Capture JPEG (F8)", 35, iY += 52, 125, 22, VK_F8);
	g_HUD.AddButton(IDC_CAPTURE_PNG, L"Capture PNG (F7)", 35, iY += 52, 125, 22, VK_F7);

	g_HUD.AddButton(IDC_CAPTURE, L"", 0, 0, 0, 0, VK_F12, false, &m_pCapBtn);
	g_HUD.AddButton(IDC_CAPTURE, L"", 0, 0, 0, 0, VK_SCROLL, false, &m_pCapBtn);

	m_pCapStatic->SetVisible(false);
}


void CCaptureTest::OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl)
{
	switch( nControlID )
	{
	case IDC_CAPTURE :
		DoCapture(!m_bCapture);
		break;
	case IDC_CAPTURE_JPEG :
		DoCapture_Jpeg();
		break;
	case IDC_CAPTURE_PNG :
		DoCapture_PNG();
		break;
	case IDC_CONFIG :
		DoConfig();
		break;
	case IDC_YOUTUBE :
		m_bYoutubeUpload = !m_bYoutubeUpload;
		break;
	case IDC_PLAYBEEP :
		{
			CDXUTCheckBox* pCheck = pControl->m_pDialog->GetCheckBox(IDC_PLAYBEEP);
			if(pCheck->GetChecked()) SetTimer(m_hWnd, TIMER_BEEP, 5000, NULL);
			else KillTimer(m_hWnd, TIMER_BEEP);
			break;
		}
	}
}

HRESULT CCaptureTest::OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	m_pd3dDevice = pd3dDevice;
    return S_OK;
}

HRESULT CCaptureTest::OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	return S_OK;
}

void CCaptureTest::OnFrameMove( double fTime, float fElapsedTime)
{
}

LRESULT CCaptureTest::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing)
{
	if(uMsg==WM_TIMER)
	{
		OnTimer(wParam);
	}

    return 0;
}

void CCaptureTest::OnD3D9LostDevice()
{
}

void CCaptureTest::OnD3D9DestroyDevice()
{
	if(m_bandiCaptureLibrary.IsCreated())
	{
		m_bandiCaptureLibrary.Destroy();
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
///          start/stop capture
/// @param   bCapture - 1: start, 0: stop
/// @return  1: start, 0: stop
/// @date    2008-09-02 14:30:19
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CCaptureTest::DoCapture(BOOL bCapture)
{
	m_bCapture = bCapture;
	if(m_bandiCaptureLibrary.IsCapturing())
	{
		m_bandiCaptureLibrary.Stop();

		if(m_bYoutubeUpload)
		{
			m_pd3dDevice->SetDialogBoxMode(TRUE);

			CCaptureUpload dlg(m_szCaptureFilePathName);
			if(dlg.DoModal() == IDOK)
			{
				if(m_YoutubeUploader.Upload(&dlg.m_info) == FALSE)
				{
					if(m_YoutubeUploader.IsUploading())
						MessageBox(m_hWnd, _T("The last upload is still going."), _T("Error"), MB_OK);
				}
			}

			m_pd3dDevice->SetDialogBoxMode(FALSE);
		}
	}

	if(m_bCapture == TRUE)
	{
		HRESULT hr;

		if(m_bYoutubeUpload)
		{
			BCapConfigPreset(&m_Config.m_cfg, BCAP_PRESET_YOUTUBE_HIGH_QUALITY);
		}

		m_bandiCaptureLibrary.CheckConfig(&m_Config.m_cfg);				// 잘못된 설정 바로 잡기
		m_bandiCaptureLibrary.SetConfig(&m_Config.m_cfg);				// 설정 적용
		
		m_bandiCaptureLibrary.SetMinMaxFPS(m_Config.m_min_fps, m_Config.m_max_fps);
		m_bandiCaptureLibrary.SetPriority(m_Config.m_priority);

		// get (target directory + filename by date)
		m_bandiCaptureLibrary.MakePathnameByDate(m_Config.m_target_directory, _T("Capture"), 
								_T("avi"), 
								m_szCaptureFilePathName, MAX_PATH*2);

		LONG_PTR param;
		if(m_Config.m_cap_mode == BCAP_MODE_GDI) param =  (LONG_PTR)m_hWnd;
		else param = (LONG_PTR)m_pd3dDevice;

		// start capture
		hr = m_bandiCaptureLibrary.Start(m_szCaptureFilePathName, NULL, m_Config.m_cap_mode, param);
		if(FAILED(hr))
		{
			Message(_T("Failed to start capture (0x%08x)"), hr);

			ASSERT(0);
			m_bandiCaptureLibrary.Stop();
			m_bCapture = FALSE;
		}
		else if(hr == BC_OK_ONLY_VIDEO)
		{
			Message(_T("Only video capture"));
		}
		else if(hr == BC_OK_ONLY_AUDIO)
		{
			Message(_T("Only audio capture"));
		}
	}

	m_pCapBtn->SetText(m_bCapture ? L"Stop (F12 toggle)" : L"Capture (F12 toggle)");
	m_pCapStatic->SetVisible(!!m_bCapture);

	return m_bCapture;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
///          Capture as JPEG
/// @return  
/// @date    2009-01-19 12:21:49
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CCaptureTest::DoCapture_Jpeg()
{
	DWORD dwTick = GetTickCount();

	// get (target directory + filename by date)
	TCHAR szCaptureFilePathName[MAX_PATH*2];
	m_bandiCaptureLibrary.MakePathnameByDate(m_Config.m_target_directory, _T("Capture"), 
							_T("jpg"), 
							szCaptureFilePathName, MAX_PATH*2);

	if(FAILED(m_bandiCaptureLibrary.CaptureImage(szCaptureFilePathName, BCAP_IMAGE_JPG, 80, BCAP_MODE_D3D9, FALSE, (LONG_PTR)m_pd3dDevice)))
		return FALSE;

	Message(_T("%s, %dms"), szCaptureFilePathName, GetTickCount()-dwTick);

	return TRUE;
}

BOOL CCaptureTest::DoCapture_PNG()
{
	DWORD dwTick = GetTickCount();

	// get (target directory + filename by date)
	TCHAR szCaptureFilePathName[MAX_PATH*2];
	m_bandiCaptureLibrary.MakePathnameByDate(m_Config.m_target_directory, _T("Capture"), 
							_T("png"), 
							szCaptureFilePathName, MAX_PATH*2);

	if(FAILED(m_bandiCaptureLibrary.CaptureImage(szCaptureFilePathName, BCAP_IMAGE_PNG, 80, BCAP_MODE_D3D9, FALSE, (LONG_PTR)m_pd3dDevice)))
		return FALSE;

	Message(_T("%s, %dms"), szCaptureFilePathName, GetTickCount()-dwTick);

	return TRUE;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
///          show config dialog
/// @date    2008-09-02 14:30:01
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCaptureTest::DoConfig()
{
	m_pd3dDevice->SetDialogBoxMode(TRUE);
	m_Config.DoModal();
	m_pd3dDevice->SetDialogBoxMode(FALSE);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
///         message to screen
/// @date   Thursday, August 07, 2008  3:26:06 PM
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCaptureTest::Message(LPCTSTR szMsg, ...)
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
void CCaptureTest::OnTimer(int nTimerID)
{
	if(nTimerID==TIMER_PROCESSMESSAGE)
	{
		if(m_messages.GetCount())
			m_messages.RemoveAll();
	}
	TCHAR	szOggPathName[MAX_PATH];
	GetThisPath(_T("sound\\beep.ogg"), szOggPathName, MAX_PATH);

	if(nTimerID==TIMER_BEEP)
	{
		m_pBaCon2->Play(szOggPathName);
		m_tickBeep = GetTickCount();
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
///          render text after capture processing
/// @param   &txtHelper 
/// @param   bCaptured
/// @date    2008-09-02 14:29:34
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCaptureTest::RenderText(CDXUTTextHelper &txtHelper, BOOL bCaptured)
{
	if(bCaptured == TRUE)
	{
		if(m_tickBeep + 200 > GetTickCount())
		{
			txtHelper.DrawTextLine(_T("Beep!!!"));
		}
	}
	else 
	{
		if(m_bandiCaptureLibrary.IsCapturing() == TRUE)
		{
			TCHAR s[128];
			INT msec = m_bandiCaptureLibrary.GetCaptureTime();
			_stprintf(s, _T("[ %02d:%02d:%03d / %.1f MB] Capturing..."), 
				msec/(60*1000), msec%(60*1000)/1000, msec%1000, 
				m_bandiCaptureLibrary.GetCaptureFileSize()/1024./1024.);
			
			txtHelper.DrawTextLine(s);
		}

		TCHAR szStatus[STATUS_BUFFER_COUNT+1];
		int nBufferCnt = STATUS_BUFFER_COUNT;
		if(m_YoutubeUploader.GetStatusString(szStatus, nBufferCnt))
		{
			txtHelper.DrawTextLine(szStatus);
		}

		POSITION p = m_messages.GetHeadPosition();
		while(p)
		{
			txtHelper.DrawTextLine(m_messages.GetNext(p));
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
///          캡처 후 랜더 처리, 이곳에 랜더된 오브젝트들은 캡처된 영상에 표시되지 않음
/// @param   pd3dDevice 
/// @date    2008-09-02 14:28:26
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCaptureTest::RenderScene( IDirect3DDevice9* pd3dDevice)
{
	// 녹화 상태 표시

}


////////////////////////////////////////////////////////////////////////////////////////////////////
///          화면 캡처
/// @param   pd3dDevice 
/// @date    2008-09-02 14:27:37
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCaptureTest::CaptureScene( IDirect3DDevice9* pd3dDevice)
{
	if(m_bCapture == TRUE)
	{
		ASSERT(m_bandiCaptureLibrary.IsCreated()==TRUE);

		// 캡처 작업
		HRESULT hr = m_bandiCaptureLibrary.Work((LONG_PTR)pd3dDevice);
		if(FAILED(hr))
		{
			DoCapture(FALSE);

			if(hr == BCERR_FILE_CREATE)
			{
				Message(_T("Cannot create output file : %s"), m_szCaptureFilePathName);
			}
			else if(hr == BCERR_FILE_WRITE)
			{
				Message(_T("Cannot write output file : %s"), m_szCaptureFilePathName);
			}
			else if(hr == BCERR_NOT_ENOUGH_DISKSPACE)
			{
				Message(_T("Not enough free disk space"));
			}
			else if(hr == BCERR_VIDEO_CAPTURE)
			{
				Message(_T("Fail to capture video"));
			}
			else if(hr == BCERR_AUDIO_CAPTURE)
			{
				Message(_T("Fail to capture audio"));
			}
		}
	}
}
