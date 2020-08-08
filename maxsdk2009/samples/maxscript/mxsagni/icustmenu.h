// ============================================================================
// ICustMenu.h
// Copyright ©1999 Discreet
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#ifndef __ICUSTMENU_H__
#define __ICUSTMENU_H__


// ============================================================================
class ICustMenu
{
public:
	static bool Init(HINSTANCE hInstance, HFONT hFont);
	static ICustMenu* Create(const TCHAR *sTitle, bool bCanConvertToBar = true);

	virtual void Destroy() = 0;

	// Display methods
	virtual void PopupMenu(HWND hParentWnd, POINT pt) = 0;
	virtual void PopupBar(HWND hParentWnd, POINT pt) = 0;

	// Menu construction methods
	virtual bool AppendHMenu(HMENU hMenu) = 0;
	virtual int  AddItem(const TCHAR *sText, UINT nID, bool bChecked = false, bool bEnabled = true) = 0;
	virtual int  AddSubMenu(ICustMenu *custMenu) = 0;
	virtual int  AddSeparator() = 0;
	virtual void DelItem(int nItem) = 0;
	virtual int  GetItemCount() = 0;

	// State methods
	virtual void SetItemCheck(int nItem, bool bChecked) = 0;
	virtual bool GetItemCheck(int nItem) = 0;

	virtual void SetItemText(int nItem, const TCHAR *sText) = 0;
	virtual const TCHAR* GetItemText(int nItem) = 0;

	virtual void SetItemEnabled(int nItem, bool bEnabled) = 0;
	virtual bool GetItemEnabled(int nItem) = 0;
};


#endif //__ICUSTMENU_H__