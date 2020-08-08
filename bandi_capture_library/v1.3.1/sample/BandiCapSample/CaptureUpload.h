#pragma once

#include "resource.h"
#include "YoutubeUploader.h"

class CCaptureUpload : public CDialogImpl<CCaptureUpload>
{
public:
	enum { IDD = IDD_UPLOAD };

	CCaptureUpload(CAtlString sFilePath);

	BEGIN_MSG_MAP(CCaptureUpload)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnOk)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnCancel)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

protected:
	LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	CAtlString	m_sFilePath;

public:
	SUploadInfo m_info;
};
