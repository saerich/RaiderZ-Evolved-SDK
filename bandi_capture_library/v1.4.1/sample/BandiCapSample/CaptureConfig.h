#pragma once

#include "resource.h"
#include <bandicap.h>

class CCaptureConfig : public CDialogImpl<CCaptureConfig>
{
public:
	enum { IDD = IDD_CONFIG };

	CCaptureConfig(BCAP_CONFIG *cfg = NULL);

	BEGIN_MSG_MAP(CCaptureConfig)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnOk)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnCancel)
		COMMAND_HANDLER(IDC_BROWSE, BN_CLICKED, OnBrowse)
		COMMAND_HANDLER(IDC_CAPTURE_RECT_TEST, BN_CLICKED, OnCaptureRectTest)
		COMMAND_HANDLER(IDC_BTN_PRESET_YOUTUBE, BN_CLICKED, OnBnClickedBtnPresetYoutube)
		COMMAND_HANDLER(IDC_BTN_PRESET_NAVER_BLOG, BN_CLICKED, OnBnClickedBtnPresetNaverBlog)
		COMMAND_HANDLER(IDC_BTN_PRESET_DEFAULT, BN_CLICKED, OnBnClickedBtnPresetDefault)
		COMMAND_HANDLER(IDC_BTN_PRESET_HALF, BN_CLICKED, OnBnClickedBtnPresetHalf)
		COMMAND_HANDLER(IDC_BTN_PRESET_640x480, BN_CLICKED, OnBnClickedBtnPreset640x480)
		COMMAND_HANDLER(IDC_BTN_PRESET_800x600, BN_CLICKED, OnBnClickedBtnPreset800x600)
		COMMAND_HANDLER(IDC_BTN_PRESET_320x240, BN_CLICKED, OnBnClickedBtnPreset320x240)
		COMMAND_HANDLER(IDC_BTN_PRESET_400x300, BN_CLICKED, OnBnClickedBtnPreset400x300)
		COMMAND_HANDLER(IDC_BTN_PRESET_512x384, BN_CLICKED, OnBnClickedBtnPreset512x384)
		COMMAND_HANDLER(IDC_BTN_PRESET_576x432, BN_CLICKED, OnBnClickedBtnPreset576x432)
		COMMAND_HANDLER(IDC_BTN_PRESET_MJPEG, BN_CLICKED, OnBnClickedBtnPresetMjpeg)
		COMMAND_HANDLER(IDC_BTN_PRESET_MJPEG_HIGH, BN_CLICKED, OnBnClickedBtnPresetMjpegHigh)
		COMMAND_HANDLER(IDC_BTN_PRESET_VIDEO_EDTING, BN_CLICKED, OnBnClickedBtnPresetVideoEditing)
		COMMAND_HANDLER(IDC_BTN_PRESET_YOUTUBE_HQ, BN_CLICKED, OnBnClickedBtnPresetYoutubeHq)
		COMMAND_HANDLER(IDC_BTN_PRESET_MPEG1, BN_CLICKED, OnBnClickedBtnPresetMpeg1)
		COMMAND_HANDLER(IDC_BTN_PRESET_MPEG4, BN_CLICKED, OnBnClickedBtnPresetMpeg4)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

protected:
	LRESULT OnOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCaptureRectTest(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void UpdateData(BOOL bUpdate);

public:
	BCAP_CONFIG m_cfg;

	int m_cap_mode;
	int m_min_fps;
	int m_max_fps;
	int m_priority;
	TCHAR m_target_directory[MAX_PATH];
	LRESULT OnBnClickedBtnPresetYoutube(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBtnPresetNaverBlog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBtnPresetDefault(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBtnPresetHalf(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBtnPreset640x480(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBtnPreset800x600(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBtnPreset320x240(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBtnPreset400x300(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBtnPreset512x384(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBtnPreset576x432(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBtnPresetMjpeg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBtnPresetMjpegHigh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBtnPresetVideoEditing(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBtnPresetYoutubeHq(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBtnPresetMpeg1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBtnPresetMpeg4(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
