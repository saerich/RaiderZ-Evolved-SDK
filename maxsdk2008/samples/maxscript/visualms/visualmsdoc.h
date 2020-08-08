// ============================================================================
// VisualMSDoc.h : interface of the CVisualMSDoc class
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#ifndef __VISUALMSDOC_H__
#define __VISUALMSDOC_H__

#include "GuiObj.h"
#include "FormEd.h"

#if _MSC_VER > 1000
#pragma once
#endif

#define UG_FORMED     0x0001L
#define UG_PROPLIST   0x0002L
#define UG_EVENTLIST  0x0004L
#define UG_PROPBAR    (UG_PROPLIST|UG_EVENTLIST)
#define UG_ALL        0x00FFL

#define VMS_VERSION 1
#define VMS_MAGIC (MAKEINT('V', 'M', 'S', '1'))

// ============================================================================
struct VmsHeader
{
	UINT magic;
	int version;
	int numObj;
};

// ============================================================================
class CVisualMSDoc : public CDocument
{
protected:
	CVisualMSDoc();
	DECLARE_DYNCREATE(CVisualMSDoc)

// Attributes
public:
	CPtrArray m_objects;     // list of all GUI objects in the document.
	UINT     m_createMode;   // current creation mode.
	int      m_curSelBase;   // base obj index for multiple selection operations

	CString m_sTitle;
	CString m_sFileName;

	bool   m_bUseGrid;
	int    m_gridSpacing;
	int    m_alignOffset;
	bool   m_creating;		// form under creation, not yet opened

	IVisualMSCallback* m_editCallback;	// supplied if invoked by a system that wants edit op callbacks

// Operations
public:
	// ! This should only be called from within an executing VMS thread !
	static CVisualMSDoc* GetDoc();

	CView* GetFormView();
	CView* GetPropView();
	CView* GetEventView();

	void SetFileName(LPCTSTR fname);
	const CString& GetFileName();

	void SetCreateMode(UINT createMode);
	UINT GetCreateMode();

	void SetUseGrid(bool bUseGrid);
	bool GetUseGrid();

	void SetGridSpacing(int gridSpacing);
	int  GetGridSpacing();

	void SetAlignOffset(int alignOffset);
	int  GetAlignOffset();

	void SetEditCallback(IVisualMSCallback* cb) { m_editCallback = cb; }
	IVisualMSCallback* GetEditCallback() { return m_editCallback; }

	// GUI object operations.
	int  GetSize();
	int  AddObj(CGuiObj *obj);
	void RemoveObj(int idx);
	void SetObj(int idx, CGuiObj *obj);
	int  InsertObj(int insertAt, CGuiObj *pObj);
	CGuiObj* GetObj(int idx);
	CFormObj* GetFormObj();
	int ItemIDFromName(TCHAR* itemName);
	int PropertyIDFromName(TCHAR* propName);

	// This can only be called from within a VMS thread
	void UpdateAllViews(CView* pSender, LPARAM lHint = 0L, CObject* pHint = NULL);

	// Hit tests all objects from start+1 to the end, returns 0 if nothing.
	int HitTest(int start, const CRect &rect);

	// Searches for objects with a selection state of bSel
	// from start+1 to the end, returns -1 if nothing.
	int GetNextSel(int start = -1, bool bSel = true);

	// Fills arr with indices to all selected objects.
	void GetSelectionSet(CUIntArray &arr);

	// GUI object selection operations.
	void Select(int idx);
	bool IsSelected(int idx);
	void DeSelect(int idx);
	void SelectAll();
	void DeSelectAll();
	int  GetSelectionCount();
	void GetSelectionRect(CRect &rect);
	void GetFormSize(CSize &size);
	void Invalidate(int idx = 0);

	// script generation
	bool GenScript(CString& script, TCHAR* indent = NULL);
	bool CheckScriptLinkage();

	// file I/O
	bool SaveFile(const CString &fname);
	bool LoadFile(const CString &fname);
	bool ExportScript(const CString &fname);
	bool SaveVMS(const CString &fname, bool bSelOnly = false);
	bool LoadVMS(const CString &fname);
	bool MergeVMS(const CString &fname, bool bPaste = false);

// Overrides
	//{{AFX_VIRTUAL(CVisualMSDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();
	virtual BOOL DoFileSave();
	virtual void OnCloseDocument();
	virtual void SetTitle(LPCTSTR lpszTitle);
	virtual BOOL SaveModified();
	//}}AFX_VIRTUAL

public:
	virtual ~CVisualMSDoc();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//{{AFX_MSG(CVisualMSDoc)
	afx_msg void OnEditClear();
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditPaste();
	afx_msg void OnLayoutAlignLeft();
	afx_msg void OnLayoutAlignRight();
	afx_msg void OnLayoutAlignTop();
	afx_msg void OnLayoutAlignBottom();
	afx_msg void OnLayoutAlignHorzCenter();
	afx_msg void OnLayoutAlignVertCenter();
	afx_msg void OnLayoutGuideSettings();
	afx_msg void OnLayoutSpaceEvenlyAcross();
	afx_msg void OnLayoutSpaceEvenlyDown();
	afx_msg void OnLayoutMakeSameSizeWidth();
	afx_msg void OnLayoutMakeSameSizeHeight();
	afx_msg void OnLayoutMakeSameSizeBoth();
	afx_msg void OnLayoutCenterInDialogVertical();
	afx_msg void OnLayoutCenterInDialogHorizontal();
	afx_msg void OnLayoutFlip();
	afx_msg void OnEditMoveLeft();
	afx_msg void OnEditMoveRight();
	afx_msg void OnEditMoveUp();
	afx_msg void OnEditMoveDown();
	afx_msg void OnFileNew();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


//{{AFX_INSERT_LOCATION}}

#endif //__VISUALMSDOC_H__
