// ============================================================================
// StdObj.h
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#ifndef __STDOBJ_H__
#define __STDOBJ_H__

#include "GuiObj.h"

// ============================================================================
// CStdObj - basic single window GUI object class.
// ----------------------------------------------------------------------------
class CStdObj : public CGuiObj
{
protected:
	CWnd m_wnd;

public:
	CStdObj(CWnd *pParent, const CRect &rect);
	virtual ~CStdObj();
	virtual void MoveToTop(const CWnd* pWndInsertAfter);
	virtual void GetRect(CRect &rect) const;
	virtual void SetRect(const CRect &rect);
	virtual void Emit(CString &out);
	virtual int  OnPropChange(int idx);
};

// ============================================================================
class CBitmapObj : public CStdObj
{
protected:
	static int m_count;
	int idx_fileName;
	int idx_bitmap;

public:
	CBitmapObj(CWnd *pParent, const CRect &rect);
};

// ============================================================================
class CButtonObj : public CStdObj
{
protected:
	int idx_images;
	int idx_toolTip;

public:
	static int m_count;
	CButtonObj(CWnd *pParent, const CRect &rect);
};

// ============================================================================
class CMapButtonObj : public CStdObj
{
protected:
	int idx_map;
	int idx_images;
	int idx_toolTip;

public:
	CMapButtonObj(CWnd *pParent, const CRect &rect);
};

// ============================================================================
class CMaterialButtonObj : public CStdObj
{
protected:
	int idx_material;
	int idx_images;
	int idx_toolTip;

public:
	CMaterialButtonObj(CWnd *pParent, const CRect &rect);
};

// ============================================================================
class CPickButtonObj : public CStdObj
{
protected:
	int idx_message;
	int idx_filter;
	int idx_toolTip;

public:
	CPickButtonObj(CWnd *pParent, const CRect &rect);
};

// ============================================================================
class CCheckButtonObj : public CStdObj
{
protected:
	static int m_count;
	int idx_highlightColor;
	int idx_toolTip;
	int idx_checked;
	int idx_images;

public:
	CCheckButtonObj(CWnd *pParent, const CRect &rect);
	virtual void Emit(CString &out);
	virtual int OnPropChange(int idx);
};

// ============================================================================
class CColorPickerObj : public CStdObj
{
protected:
	static int m_count;
	CWnd m_labelWnd;
	int idx_color;
	int idx_fieldWidth;
	int idx_height;
	int idx_title;

public:
	CColorPickerObj(CWnd *pParent, const CRect &rect);
	virtual void SetRect(const CRect &rect);
	virtual int  OnPropChange(int idx);
};

// ============================================================================
class CComboBoxObj : public CStdObj
{
protected:
	static int m_count;
	CWnd m_labelWnd;
	int idx_items;
	int idx_selection;

public:
	CComboBoxObj(CWnd *pParent, const CRect &rect);
	virtual void Emit(CString &out);
	virtual CString GetPropStr(int idx) const;
	virtual void SetPropStr(int idx, const CString &sVal);
	virtual int  GetPropInt(int idx) const;
	virtual void SetPropInt(int idx, int iVal);
	virtual void SetRect(const CRect &rect);
	virtual int  OnPropChange(int idx);
};

// ============================================================================
class CDropDownListObj : public CStdObj
{
protected:
	static int m_count;
	CWnd m_labelWnd;
	int idx_items;
	int idx_selection;

public:
	CDropDownListObj(CWnd *pParent, const CRect &rect);
	virtual void Emit(CString &out);
	virtual void GetRect(CRect &rect) const;
	virtual void SetRect(const CRect &rect);
	virtual int  OnPropChange(int idx);
};

// ============================================================================
class CListBoxObj : public CStdObj
{
protected:
	static int m_count;
	CWnd m_labelWnd;
	int idx_items;
	int idx_selection;

public:
	CListBoxObj(CWnd *pParent, const CRect &rect);
	virtual void Emit(CString &out);
	virtual CString GetPropStr(int idx) const;
	virtual void SetPropStr(int idx, const CString &sVal);
	virtual int  GetPropInt(int idx) const;
	virtual void SetPropInt(int idx, int iVal);
	virtual void SetRect(const CRect &rect);
	virtual int  OnPropChange(int idx);
};

// ============================================================================
class CEditTextObj : public CStdObj
{
protected:
	static int m_count;
	CWnd m_labelWnd;

public:
	CEditTextObj(CWnd *pParent, const CRect &rect);
	virtual void SetRect(const CRect &rect);
	virtual void Emit(CString &out);
	virtual int  OnPropChange(int idx);
};

// ============================================================================
class CLabelObj : public CStdObj
{
protected:
	static int m_count;

public:
	CLabelObj(CWnd *pParent, const CRect &rect);
};

// ============================================================================
class CGroupBoxObj : public CStdObj
{
protected:
	static int m_count;

public:
	CGroupBoxObj(CWnd *pParent, const CRect &rect);
	virtual void Emit(CString &out);
	virtual bool HitTest(const CRect &rect);
};

// ============================================================================
class CCheckboxObj : public CStdObj
{
protected:
	static int m_count;
	int idx_checked;

public:
	CCheckboxObj(CWnd *pParent, const CRect &rect);
	virtual void Emit(CString &out);
	virtual int OnPropChange(int idx);
};

// ============================================================================
class CRadioButtonsObj : public CGuiObj
{
protected:
	static int m_count;
	CPtrArray  m_buttons;
	CWnd       m_captionWnd;

	int idx_labels;
	int idx_default;
	int idx_columns;

public:
	CRadioButtonsObj(CWnd *pParent, const CRect &rect);
	virtual ~CRadioButtonsObj();
	virtual void GetRect(CRect &rect) const;
	virtual void SetRect(const CRect &rect);
	virtual CString GetPropStr(int idx) const;
	virtual void SetPropStr(int idx, const CString &sVal);
	virtual void SetPropInt(int idx, int iVal);
	virtual void Emit(CString &out);
	virtual int  OnPropChange(int idx);

	virtual bool UpdateButtons();
};

// ============================================================================
class CSpinnerObj : public CStdObj
{
protected:
	static int m_count;
	CWnd m_labelWnd;
	CStatic m_spn;

	int idx_range;
	int idx_type;
	int idx_scale;
	int idx_controller;
	int idx_fieldWidth;


public:
	CSpinnerObj(CWnd *pParent, const CRect &rect);
	virtual void SetRect(const CRect &rect);
	virtual void Emit(CString &out);
	virtual int  OnPropChange(int idx);
};

// ============================================================================
class CProgressBarObj : public CStdObj
{
protected:
	static int m_count;
	int idx_value;
	int idx_color;
	int idx_orient;

public:
	CProgressBarObj(CWnd *pParent, const CRect &rect);
};

// ============================================================================
class CSliderObj : public CStdObj
{
protected:
	static int m_count;
	CWnd m_labelWnd;
	int idx_range;
	int idx_type;
	int idx_ticks;
	int idx_orient;

public:
	CSliderObj(CWnd *pParent, const CRect &rect);
	virtual void SetRect(const CRect &rect);
	virtual int  OnPropChange(int idx);
};

// ============================================================================
class CTimerObj : public CStdObj
{
protected:
	static int m_count;
	int idx_interval;
	int idx_active;

public:
	CTimerObj(CWnd *pParent, const CRect &rect);
	virtual void SetRect(const CRect &rect);
};

// ============================================================================
class CActiveXObj : public CStdObj
{
protected:
	static int m_count;

public:
	CActiveXObj(CWnd *pParent, const CRect &rect);
	virtual void SetRect(const CRect &rect);
};


// ============================================================================
class CCustomObj : public CStdObj
{
protected:
	static int m_count;

public:
	CCustomObj(CWnd *pParent, const CRect &rect);
	virtual void SetRect(const CRect &rect);
};


#endif //__STDOBJ_H__


