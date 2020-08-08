// ============================================================================
// SyntaxEditCtrl.h
// Copyright ©2000, Asylum Software
// Parts of this were taken from: Juraj Rojko jrojko@twist.cz
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#ifndef __SYNTAXEDITCTRL_H__
#define __SYNTAXEDITCTRL_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// ============================================================================
class CSyntaxEditCtrl : public CRichEditCtrl
{
public:
	COLORREF m_colKeyword;
	COLORREF m_colComment;
	COLORREF m_colNumber;
	COLORREF m_colString;
	COLORREF m_colOperator;

public:
	CSyntaxEditCtrl();
	virtual ~CSyntaxEditCtrl();

	bool AddKeywords(LPCTSTR keywords);
	bool AddComment(LPCTSTR start, LPCTSTR end);
	bool AddOperators(LPCTSTR operators);

public:
	//{{AFX_VIRTUAL(CSyntaxEditCtrl)
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CSyntaxEditCtrl)
	afx_msg void OnChange();
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif //__SYNTAXEDITCTRL_H__
