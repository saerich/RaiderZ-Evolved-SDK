#if !defined(AFX_GUIDEDLG_H__5F2FACC0_05E3_43D0_9CBB_C45D86F16877__INCLUDED_)
#define AFX_GUIDEDLG_H__5F2FACC0_05E3_43D0_9CBB_C45D86F16877__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GuideDlg.h : header file
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CGuideDlg dialog

class CGuideDlg : public CDialog
{
// Construction
public:
	CGuideDlg(CWnd* pParent = NULL);   // standard constructor
	CGuideDlg(BOOL useGrid, int gridSpacing, int alignOffset);

// Dialog Data
	//{{AFX_DATA(CGuideDlg)
	enum { IDD = IDD_GUIDES };
	BOOL	m_useGrid;
	int		m_gridSpacing;
	int		m_alignOffset;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGuideDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGuideDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GUIDEDLG_H__5F2FACC0_05E3_43D0_9CBB_C45D86F16877__INCLUDED_)
