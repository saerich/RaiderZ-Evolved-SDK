// ============================================================================
// SyntaxEditCtrl.cpp
// Copyright ©2000, Asylum Software
// Parts of this were taken from: Juraj Rojko jrojko@twist.cz
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#include "stdafx.h"
#include "visualms.h"
#include "SyntaxEditCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// ============================================================================
BEGIN_MESSAGE_MAP(CSyntaxEditCtrl, CRichEditCtrl)
	//{{AFX_MSG_MAP(CSyntaxEditCtrl)
	ON_WM_GETDLGCODE()
	ON_WM_CHAR()
	ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ============================================================================
CSyntaxEditCtrl::CSyntaxEditCtrl()
{
}

// ============================================================================
CSyntaxEditCtrl::~CSyntaxEditCtrl()
{
}

// ============================================================================
UINT CSyntaxEditCtrl::OnGetDlgCode() 
{
	// TODO: Add your message handler code here and/or call default
	
	return CRichEditCtrl::OnGetDlgCode();
}

// ============================================================================
void CSyntaxEditCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	CRichEditCtrl::OnChar(nChar, nRepCnt, nFlags);
}

// ============================================================================
void CSyntaxEditCtrl::OnChange() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CRichEditCtrl::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
}
