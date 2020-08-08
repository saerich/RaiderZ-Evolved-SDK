#include "stdafx.h"
#include "ConfigDlg.h"

// 다이얼로그 컨트롤 데이터 처리 매크로
#define ADD_STRING(i, s)		ComboBox_AddString(GetDlgItem(i), s)

#define SET_TEXT(i, s)			::SetWindowText(GetDlgItem(i), s)
#define GET_TEXT(i, s)			::GetWindowText(GetDlgItem(i), s, MAX_PATH)

#define SET_TEXT_INT(i, s)		::SetWindowText(GetDlgItem(i), _itot(s, buf, 10))
#define GET_TEXT_INT(i, s)		::GetWindowText(GetDlgItem(i), buf, MAX_PATH); s = _tstoi(buf)

#define SET_TEXT_FLOAT(i, s)	::swprintf(buf, MAX_PATH,  _T("%.3f"), s);	::SetWindowText(GetDlgItem(i), buf)
#define GET_TEXT_FLOAT(i, s)	::GetWindowText(GetDlgItem(i), buf, MAX_PATH); s = (float)_tstof(buf)


CConfigDlg::CConfigDlg(BCAP_CONFIG *cfg)
{
	m_cfg = *cfg;
	m_min_fps = 10;
	m_max_fps = 30;
	m_thread_priority = 0;

	GetCurrentDirectory(MAX_PATH, m_output_file);
	_tcscat(m_output_file, _T("\\test.avi"));
}


LRESULT CConfigDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow();

	// 비디오 가로 크기
	ADD_STRING(IDC_VIDEO_SIZEW,			_T("320"));
	ADD_STRING(IDC_VIDEO_SIZEW,			_T("640"));
	ADD_STRING(IDC_VIDEO_SIZEW,			_T("800"));
	ADD_STRING(IDC_VIDEO_SIZEW,			_T("1024"));
	ADD_STRING(IDC_VIDEO_SIZEW,			_T("1280"));
	ADD_STRING(IDC_VIDEO_SIZEW,			_T("1600"));
	ADD_STRING(IDC_VIDEO_SIZEW,			_T("1920"));
	ADD_STRING(IDC_VIDEO_SIZEW,			_T("2560"));

	// 비디오 세로 크기
	ADD_STRING(IDC_VIDEO_SIZEH,			_T("240"));
	ADD_STRING(IDC_VIDEO_SIZEH,			_T("480"));
	ADD_STRING(IDC_VIDEO_SIZEH,			_T("600"));
	ADD_STRING(IDC_VIDEO_SIZEH,			_T("720"));
	ADD_STRING(IDC_VIDEO_SIZEH,			_T("1080"));
	ADD_STRING(IDC_VIDEO_SIZEH,			_T("1600"));

	// 비디오 FPS
	ADD_STRING(IDC_VIDEO_FPS,			_T("15.000"));
	ADD_STRING(IDC_VIDEO_FPS,			_T("20.000"));
	ADD_STRING(IDC_VIDEO_FPS,			_T("24.000"));
	ADD_STRING(IDC_VIDEO_FPS,			_T("25.000"));
	ADD_STRING(IDC_VIDEO_FPS,			_T("29.976"));
	ADD_STRING(IDC_VIDEO_FPS,			_T("30.000"));
	ADD_STRING(IDC_VIDEO_FPS,			_T("60.000"));

	// 비디오 코덱
	ADD_STRING(IDC_VIDEO_CODEC,			_T("MJPG"));
	ADD_STRING(IDC_VIDEO_CODEC,			_T("XVID"));

	// 비디오 VBR/CBR
	ADD_STRING(IDC_VIDEO_RATECONTROL,	_T("0"));
	ADD_STRING(IDC_VIDEO_RATECONTROL,	_T("1"));

	// 비디오 비트레이트
	ADD_STRING(IDC_VIDEO_KBITRATE,		_T("300"));
	ADD_STRING(IDC_VIDEO_KBITRATE,		_T("500"));
	ADD_STRING(IDC_VIDEO_KBITRATE,		_T("700"));
	ADD_STRING(IDC_VIDEO_KBITRATE,		_T("1000"));
	ADD_STRING(IDC_VIDEO_KBITRATE,		_T("1500"));
	ADD_STRING(IDC_VIDEO_KBITRATE,		_T("2000"));
	ADD_STRING(IDC_VIDEO_KBITRATE,		_T("3000"));

	// 비디오 품질
	ADD_STRING(IDC_VIDEO_QUALITY,		_T("40"));
	ADD_STRING(IDC_VIDEO_QUALITY,		_T("50"));
	ADD_STRING(IDC_VIDEO_QUALITY,		_T("60"));
	ADD_STRING(IDC_VIDEO_QUALITY,		_T("70"));
	ADD_STRING(IDC_VIDEO_QUALITY,		_T("80"));
	ADD_STRING(IDC_VIDEO_QUALITY,		_T("90"));
	ADD_STRING(IDC_VIDEO_QUALITY,		_T("100"));

	// 키프레임 간격
	ADD_STRING(IDC_VIDEO_INTERVAL,		_T("0"));
	ADD_STRING(IDC_VIDEO_INTERVAL,		_T("10"));
	ADD_STRING(IDC_VIDEO_INTERVAL,		_T("30"));
	ADD_STRING(IDC_VIDEO_INTERVAL,		_T("60"));
	ADD_STRING(IDC_VIDEO_INTERVAL,		_T("120"));
	ADD_STRING(IDC_VIDEO_INTERVAL,		_T("300"));

	// 오디오 채널 수
	ADD_STRING(IDC_AUDIO_CHANNEL,		_T("1"));
	ADD_STRING(IDC_AUDIO_CHANNEL,		_T("2"));
	
	// FPS 최대값
	ADD_STRING(IDC_AUDIO_SAMPLERATE,	_T("22050"));
	ADD_STRING(IDC_AUDIO_SAMPLERATE,	_T("24000"));
	ADD_STRING(IDC_AUDIO_SAMPLERATE,	_T("44100"));
	ADD_STRING(IDC_AUDIO_SAMPLERATE,	_T("48000"));

	// 오디오 코덱
	ADD_STRING(IDC_AUDIO_CODEC,			_T("1"));
	
	// 오디오 VBR/CBR
	ADD_STRING(IDC_AUDIO_RATECONTROL,	_T("0"));
	ADD_STRING(IDC_AUDIO_RATECONTROL,	_T("1"));
	
	// 오디오 비트레이트
	ADD_STRING(IDC_AUDIO_KBITRATE,		_T("64"));
	ADD_STRING(IDC_AUDIO_KBITRATE,		_T("96"));
	ADD_STRING(IDC_AUDIO_KBITRATE,		_T("128"));
	ADD_STRING(IDC_AUDIO_KBITRATE,		_T("256"));

	// 오디오 품질
	ADD_STRING(IDC_AUDIO_QUALITY,		_T("40"));
	ADD_STRING(IDC_AUDIO_QUALITY,		_T("50"));
	ADD_STRING(IDC_AUDIO_QUALITY,		_T("60"));
	ADD_STRING(IDC_AUDIO_QUALITY,		_T("70"));
	ADD_STRING(IDC_AUDIO_QUALITY,		_T("80"));
	ADD_STRING(IDC_AUDIO_QUALITY,		_T("90"));
	ADD_STRING(IDC_AUDIO_QUALITY,		_T("100"));

	// FPS 최소값
	ADD_STRING(IDC_FPS_MIN,				_T("10"));
	ADD_STRING(IDC_FPS_MIN,				_T("15"));
	ADD_STRING(IDC_FPS_MIN,				_T("20"));
	ADD_STRING(IDC_FPS_MIN,				_T("30"));
	ADD_STRING(IDC_FPS_MIN,				_T("60"));

	// FPS 최대값
	ADD_STRING(IDC_FPS_MAX,				_T("10"));
	ADD_STRING(IDC_FPS_MAX,				_T("15"));
	ADD_STRING(IDC_FPS_MAX,				_T("20"));
	ADD_STRING(IDC_FPS_MAX,				_T("30"));
	ADD_STRING(IDC_FPS_MAX,				_T("60"));
	
	// 쓰레드 우선 순위
	ADD_STRING(IDC_THREAD_PRIORITY,		_T("-1"));
	ADD_STRING(IDC_THREAD_PRIORITY,		_T("0"));
	ADD_STRING(IDC_THREAD_PRIORITY,		_T("1"));
	ADD_STRING(IDC_THREAD_PRIORITY,		_T("2"));

	UpdateData(TRUE);
	return 1;
}


void CConfigDlg::UpdateData(BOOL bUpdate)
{
	if(bUpdate)
	{
		TCHAR buf[MAX_PATH];
		TCHAR fourcc[5] = { (m_cfg.VideoCodec)&0xff, (m_cfg.VideoCodec>>8)&0xff, 
							(m_cfg.VideoCodec>>16)&0xff, (m_cfg.VideoCodec>>24)&0xff, '\0' };

		for(int i = 0 ; i < 3 ; i++)
			Button_SetCheck(GetDlgItem(IDC_MODE_GDI+i), m_cfg.nCaptureMode == i);
		
		SET_TEXT_INT	(IDC_VIDEO_SIZEW,		m_cfg.VideoSizeW);
		SET_TEXT_INT	(IDC_VIDEO_SIZEH,		m_cfg.VideoSizeH);
		SET_TEXT_FLOAT	(IDC_VIDEO_FPS,			m_cfg.VideoFPS);

		SET_TEXT		(IDC_VIDEO_CODEC,		fourcc);
		SET_TEXT_INT	(IDC_VIDEO_RATECONTROL,	m_cfg.VideoRateControl);
		SET_TEXT_INT	(IDC_VIDEO_KBITRATE,	m_cfg.VideoKBitrate);
		SET_TEXT_INT	(IDC_VIDEO_QUALITY,		m_cfg.VideoQuality);
		SET_TEXT_INT	(IDC_VIDEO_INTERVAL,	m_cfg.VideoKeyframeInterval);
		SET_TEXT_INT	(IDC_AUDIO_CHANNEL,		m_cfg.AudioChannels);
		SET_TEXT_INT	(IDC_AUDIO_SAMPLERATE,	m_cfg.AudioSampleRate);
		SET_TEXT_INT	(IDC_AUDIO_CODEC,		m_cfg.AudioCodec);
		SET_TEXT_INT	(IDC_AUDIO_RATECONTROL,	m_cfg.AudioRateControl);
		SET_TEXT_INT	(IDC_AUDIO_KBITRATE,	m_cfg.AudioKBitrate);
		SET_TEXT_INT	(IDC_AUDIO_QUALITY,		m_cfg.AudioQuality);
		SET_TEXT_INT	(IDC_FPS_MIN,			m_min_fps);
		SET_TEXT_INT	(IDC_FPS_MAX,			m_max_fps);
		SET_TEXT_INT	(IDC_THREAD_PRIORITY,	m_thread_priority);
		SET_TEXT		(IDC_OUTPUT_FILE,		m_output_file);
	}
	else
	{
		TCHAR buf[MAX_PATH];
		TCHAR fourcc[5];
	
		GET_TEXT_INT	(IDC_VIDEO_SIZEW,		m_cfg.VideoSizeW);
		GET_TEXT_INT	(IDC_VIDEO_SIZEH,		m_cfg.VideoSizeH);
		GET_TEXT_FLOAT	(IDC_VIDEO_FPS,			m_cfg.VideoFPS);

		GET_TEXT		(IDC_VIDEO_CODEC,		fourcc); 
		m_cfg.VideoCodec = fourcc[0]<<24 || fourcc[1]<<24 || fourcc[2]<<24 || fourcc[3];
		GET_TEXT_INT	(IDC_VIDEO_RATECONTROL,	m_cfg.VideoRateControl);
		GET_TEXT_INT	(IDC_VIDEO_KBITRATE,	m_cfg.VideoKBitrate);
		GET_TEXT_INT	(IDC_VIDEO_QUALITY,		m_cfg.VideoQuality);
		GET_TEXT_INT	(IDC_VIDEO_INTERVAL,	m_cfg.VideoKeyframeInterval);
		GET_TEXT_INT	(IDC_AUDIO_CHANNEL,		m_cfg.AudioChannels);
		GET_TEXT_INT	(IDC_AUDIO_SAMPLERATE,	m_cfg.AudioSampleRate);
		GET_TEXT_INT	(IDC_AUDIO_CODEC,		m_cfg.AudioCodec);
		GET_TEXT_INT	(IDC_AUDIO_RATECONTROL,	m_cfg.AudioRateControl);
		GET_TEXT_INT	(IDC_AUDIO_KBITRATE,	m_cfg.AudioKBitrate);
		GET_TEXT_INT	(IDC_AUDIO_QUALITY,		m_cfg.AudioQuality);
		GET_TEXT_INT	(IDC_FPS_MIN,			m_min_fps);
		GET_TEXT_INT	(IDC_FPS_MAX,			m_max_fps);
		GET_TEXT_INT	(IDC_THREAD_PRIORITY,	m_thread_priority);
		GET_TEXT		(IDC_OUTPUT_FILE,		m_output_file);
	}
}


LRESULT CConfigDlg::OnBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	OPENFILENAME ofn;
	TCHAR sFile[MAX_PATH];

	memset(&ofn, NULL, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFile = sFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(sFile);
	ofn.lpstrFilter = _T("AVI\0*.avi\0All\0*.*\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = _T("avi");

	if(GetSaveFileName(&ofn))
	{
		_tcscpy(m_output_file, sFile);
		UpdateData(TRUE);
	}

	return 1;
}


LRESULT CConfigDlg::OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	UpdateData(FALSE);

	EndDialog(IDOK);
	return 1;
}


LRESULT CConfigDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(IDCANCEL);
	return 1;
}
