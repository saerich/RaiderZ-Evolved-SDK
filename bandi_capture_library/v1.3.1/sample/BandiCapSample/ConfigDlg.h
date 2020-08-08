#include "resource.h"
#include <bandicap.h>

class CConfigDlg : public CDialogImpl<CConfigDlg>
{
public:
	enum { IDD = IDD_CONFIG };

	CConfigDlg(BCAP_CONFIG *cfg);

	BEGIN_MSG_MAP(CConfigDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnOk)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnCancel)
		COMMAND_HANDLER(IDC_BROWSE, BN_CLICKED, OnBrowse)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

protected:
	LRESULT OnOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void UpdateData(BOOL bUpdate);

public:
	BCAP_CONFIG m_cfg;
	int m_min_fps;
	int m_max_fps;
	int m_thread_priority;
	TCHAR m_output_file[MAX_PATH];
};
