// ============================================================================
// EventView.h
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#ifndef __EVENTVIEW_H__
#define __EVENTVIEW_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxcview.h>
#include "GuiObj.h"

// ============================================================================
// CEventView
// Displays events for a GUI object
// ----------------------------------------------------------------------------
class CEventView : public CListView
{
protected:
	CGuiObj *m_pCurObj;

	CEventView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CEventView)

public:
	void SetCurObj(CGuiObj *pCurObj);
	void UpdateEvent(int idx);

	//{{AFX_VIRTUAL(CEventView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

protected:
	virtual ~CEventView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//{{AFX_MSG(CEventView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif //__EVENTVIEW_H__


