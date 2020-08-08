#include "stdafx.h"
#include "CaptureUpload.h"


CCaptureUpload::CCaptureUpload(CAtlString sFilePath)
: m_sFilePath(sFilePath)
{
	memset(&m_info, NULL, sizeof(m_info));
}


LRESULT CCaptureUpload::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow();
	
	GetDlgItem(IDC_ED_ID).SetWindowText(_T(""));
	GetDlgItem(IDC_ED_PWD).SetWindowText(_T(""));

	GetDlgItem(IDC_ED_TITLE).SetWindowText(_T("[BCL] Game Video"));
	GetDlgItem(IDC_ED_DESC).SetWindowText(_T("This video was created using BCL."));
	GetDlgItem(IDC_ED_KEYWORD).SetWindowText(_T("Game, BCL"));

	GetDlgItem(IDC_ED_FILE).SetWindowText(m_sFilePath);

	return 1;
}


LRESULT CCaptureUpload::OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	::GetWindowTextA(GetDlgItem(IDC_ED_ID).m_hWnd,		m_info.szID,		UP_MAX_ID);
	::GetWindowTextA(GetDlgItem(IDC_ED_PWD).m_hWnd,		m_info.szPWD,		UP_MAX_PWD);

	::GetWindowTextW(GetDlgItem(IDC_ED_TITLE).m_hWnd,	m_info.szTitle,		UP_MAX_TITLE);
	::GetWindowTextW(GetDlgItem(IDC_ED_DESC).m_hWnd,	m_info.szDesc,		UP_MAX_DESC);
	::GetWindowTextW(GetDlgItem(IDC_ED_KEYWORD).m_hWnd, m_info.szKeyword,	UP_MAX_KEYWORD);

	::GetWindowTextW(GetDlgItem(IDC_ED_FILE).m_hWnd,	m_info.szFilePath,		MAX_PATH);

	if(m_info.szID[0] == '\0')
	{
		MessageBox(_T("Please check your username."));
		return 1;
	}

	if(m_info.szPWD[0] == '\0')
	{
		MessageBox(_T("Please check your password."));
		return 1;
	}

	EndDialog(IDOK);
	return 1;
}


LRESULT CCaptureUpload::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(IDCANCEL);
	return 1;
}
