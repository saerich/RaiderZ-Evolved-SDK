// ============================================================================
// PropView.h
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#ifndef __PROPVIEW_H__
#define __PROPVIEW_H__

#include <afxcview.h>
#include "GuiObj.h"


// ============================================================================
class CPropView : public CListView
{
protected:
	CGuiObj *m_pCurObj;

	CPropView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPropView)

public:
	void UpdateProperty(int idx);
	void SetCurObj(CGuiObj *pCurObj);
	CEdit* EditSubLabel(int nItem, int nCol);
	CComboBox* DropDownLabel(int nItem, int nCol);

	//{{AFX_VIRTUAL(CPropView)
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

protected:
	virtual ~CPropView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//{{AFX_MSG(CPropView)
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif //__PROPVIEW_H__

