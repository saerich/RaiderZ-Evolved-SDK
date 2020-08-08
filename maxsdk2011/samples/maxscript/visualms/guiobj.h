// ============================================================================
// GuiObj.h
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#ifndef __GUIOBJ_H__
#define __GUIOBJ_H__

#include <afxtempl.h>
#include "Property.h"
#include "iVisualMS.h"

// Indicies for common properties that exist in all GUI objects.
#define IDX_CLASS    0
#define IDX_NAME     1
#define IDX_CAPTION  2
#define IDX_XPOS     3
#define IDX_YPOS     4
#define IDX_WIDTH    5
#define IDX_HEIGHT   6
#define IDX_ENABLED  7

// Item IDs for system items
#define ID_CODE		 1

class CGuiObj;
class CVisualMSDoc;


// flags for the CtrlSizer handle mask
#define HM_NONE         0x0000
#define HM_TOPLEFT      0x0001
#define HM_TOPRIGHT     0x0002
#define HM_BOTTOMRIGHT  0x0004
#define HM_BOTTOMLEFT   0x0008
#define HM_TOPMIDDLE    0x0010
#define HM_RIGHTMIDDLE  0x0020
#define HM_BOTTOMMIDDLE 0x0040
#define HM_LEFTMIDDLE   0x0080
#define HM_CORNERS (HM_TOPLEFT|HM_TOPRIGHT|HM_BOTTOMLEFT|HM_BOTTOMRIGHT)
#define HM_EDGES (HM_LEFTMIDDLE|HM_RIGHTMIDDLE|HM_TOPMIDDLE|HM_BOTTOMMIDDLE)
#define HM_ALL (HM_CORNERS|HM_EDGES)

// ============================================================================
// CCtrlSizer
// Customized rectangle tracking class, used to move/resize GUI objects.
// TODO:? It would be much nicer to mix this class in directly with the CGuiObj class
// ----------------------------------------------------------------------------
class CCtrlSizer : public CRectTracker
{
public:
	bool m_lockMovement;
	CPoint m_mouseStartPoint;  // initial mouse position for movement locking

	CGuiObj *m_pObj;         // GUI object that owns this tracker.
	UINT m_handleMask;
	UINT m_restoreMask;      // for restoring m_handleMask after multiple selections
	CRect m_oldRect;

	CCtrlSizer();
	BOOL SetCursor(CWnd* pWnd, UINT nHitTest) const;
	UINT GetHandleMask() const { return m_handleMask; }
	void AdjustRect(int nHandle, LPRECT lpRect);
	static void SetCursor(UINT hit);
};

// ============================================================================
// CGuiObj
// Base class for all user creatable/editable objects.
// ----------------------------------------------------------------------------
class CGuiObj : public IVisualMSItem
{
public:
	UINT	m_createID;
	BOOL	m_selected;
	CCtrlSizer m_sizer;
	CSize	m_defaultSize;
	CSize	m_minimumSize;
	CSize m_maximumSize;
	CString m_comment;
	CWnd *m_pParent;
	
	// existing source data if constructed from parsed source
	int		src_from;			// source start & end offsets
	int		src_to;			

	// Array of all properties in the object.
	CArray <CProperty, CProperty&> m_properties;

	// Array of all event handlers in the object.
	CArray <CHandler, CHandler&> m_handlers;

	CGuiObj();
	CGuiObj(CWnd *pParent);
	virtual ~CGuiObj();

	// Helpers
	CString& Class() { return m_properties[IDX_CLASS].m_string; }
	CString& Name()  { return m_properties[IDX_NAME].m_string; }
	CString& Caption()  { return m_properties[IDX_CAPTION].m_string; }
	virtual BOOL IsSelected() { return m_selected; }
	int FindProperty(TCHAR* propName);
	int FindHandler(TCHAR* eventName);

	// Move this object to the top of the Z order.
	virtual void MoveToTop(const CWnd* pWndInsertAfter) {}

	// Output text used to create the control.
	virtual void Emit(CString &out);

	// Output text for specified handlers function call and function body.
	virtual void EmitHandler(int idx, CString &hdr, CString &body);

	// Hit test the object.
	virtual bool HitTest(const CRect &rect);

	// Invalidate the entire control in all views.
	virtual void Invalidate() {}

	// Invalidate specifed property in all views.
	virtual void InvalidateProp(int idx);

	// Function to send the prop change message to this object.
	// Calling this is allowed from any thread!
	virtual int  PropChanged(int idx);

	// Called when a property changes via the property list.
	virtual int  OnPropChange(int idx) { return 0; }

	// Get/Set rectangle of object (in pixels)
	virtual void GetRect(CRect &rect) const {}
	virtual void SetRect(const CRect &rect) {}

	// Get/Set specific properties of the object (in object units)
	virtual CString GetPropStr(int idx) const;
	virtual void SetPropStr(int idx, const CString &sVal);
	virtual int GetPropInt(int idx) const;
	virtual void SetPropInt(int idx, int iVal);

	// Clear/Restore the rect tracker handle manipulators.
	virtual void SetHandleMask(UINT mask);
	virtual void ClearHandleMask();
	virtual void RestoreHandleMask();

	// Global creation/destruction
	static CGuiObj* Create(UINT nID, CWnd *pParent, const CRect &rect);
	static void     Destroy(CGuiObj *obj);
	
	// from IVisualMSItem FP interface
	void SetPropery(TCHAR* propName, float f);
	void SetPropery(TCHAR* propName, int i);
	void SetPropery(TCHAR* propName, bool i);
	void SetPropery(TCHAR* propName, TCHAR* s);
	void SetProperyLit(TCHAR* propName, TCHAR* s);
	void SetPropery(TCHAR* propName, Point2 pt);
	void SetPropery(TCHAR* propName, Tab<TCHAR*>* sa);
	void SetPropery(TCHAR* propName, Point3 pt);
	void GetShape(int& left, int& top, int& width, int& height);
	void SetShape(int left, int top, int width, int height);

	void SetHandler(TCHAR* eventName, TCHAR* source, int arg_start, int arg_end, int body_start, int body_end);
	void AddComment(TCHAR* code, int src_from, int src_to);

};

// generic intermediate code holder, used to store existing code between
// editable rollout items from parsed source
class CCodeObj : public CGuiObj
{
//	CString code;
	int idx_code;
public:
	CCodeObj(TCHAR* code);

	void Emit(CString &out);
};

#endif //__GUIOBJ_H__


