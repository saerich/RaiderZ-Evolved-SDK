// GuideDlg.cpp : implementation file
//

#include "stdafx.h"
#include "visualms.h"
#include "GuideDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGuideDlg dialog


CGuideDlg::CGuideDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGuideDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGuideDlg)
	m_useGrid = FALSE;
	m_gridSpacing = 0;
	m_alignOffset = 0;
	//}}AFX_DATA_INIT
}

CGuideDlg::CGuideDlg(BOOL useGrid, int gridSpacing, int alignOffset)
	: CDialog(CGuideDlg::IDD, NULL)
{
	m_useGrid = useGrid;
	m_gridSpacing = gridSpacing;
	m_alignOffset = alignOffset;
}


void CGuideDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGuideDlg)
	DDX_Check(pDX, IDC_USE_GRID, m_useGrid);
	DDX_Text(pDX, IDC_GRID_SPACING, m_gridSpacing);
	DDV_MinMaxInt(pDX, m_gridSpacing, 0, 32767);
	DDX_Text(pDX, IDC_ALIGN_OFFSET, m_alignOffset);
	DDV_MinMaxInt(pDX, m_alignOffset, 0, 32767);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGuideDlg, CDialog)
	//{{AFX_MSG_MAP(CGuideDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGuideDlg message handlers
