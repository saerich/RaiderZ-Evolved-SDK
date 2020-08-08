// ============================================================================
// CustMenu.cpp
// Copyright ©1999 Discreet
// Created by Simon Feltman
// ----------------------------------------------------------------------------

#include "MAXScrpt.h"

#include "ICustMenu.h"

#include "Resource.h"

#include "3dsmaxport.h"

#define CUSTMENU_CLASS _T("CustMenuClass")

const int c_itemHeight = 16;
const int c_sepHeight  = 9;
const int c_textLead   = 20;
const int c_textTrail  = 20;
const int c_timerElapse = 200;

int g_hdrHeight = 10;

class CustMenu;

// ============================================================================
TCHAR* DupStr(const TCHAR *str)
{
   int len = static_cast<int>(_tcslen(str)+1);  // SR DCAST64: Downcast to 2G limit.
   TCHAR *dup = new TCHAR[len];
   _tcscpy(dup, str);
   return dup;
}

// ============================================================================
void DrawBitmap(HDC hDC, HBITMAP hBmp, int xPos, int yPos)
{
   BITMAP bm;
   GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bm);

   HDC hMemDC = CreateCompatibleDC(hDC);
   SelectObject(hMemDC, hBmp);
   BitBlt(hDC, xPos, yPos, bm.bmWidth, bm.bmHeight, hMemDC, 0, 0, SRCCOPY);
   DeleteDC(hMemDC);
}

// ============================================================================
#include <stdio.h>

void DebugPrintf(char *fmt, ...)
{
   static char msg[1024];
   va_list arglist;

   if(!fmt) return;
   va_start(arglist, fmt);
   vsprintf(msg, fmt, arglist);
   va_end(arglist);

   OutputDebugString(msg);
}

// ============================================================================
class MenuItemCustom
{
public:
   TCHAR *m_sText;
   UINT m_nID;
   bool m_bChecked;
   bool m_bEnabled;
   bool m_bIsSep;
   ICustMenu *m_pSubMenu;

   HICON m_hIcon;
   HICON m_hIconSel;

   MenuItemCustom() { memset(this, NULL, sizeof(MenuItemCustom)); }
   ~MenuItemCustom()
   {
      if(m_sText)
      {
         delete m_sText;
         m_sText = NULL;
      }
   }
};

// ============================================================================
class MenuData
{
protected:
   bool    m_bMouseOver;
   bool    m_bIsMenuBar;
   int     m_nSelected;
   int     m_nSubMenuUp;
   int     m_nSelStart;
   UINT    m_nTimerID;
   HWND    m_hWnd;
   HWND    m_hParentWnd;
   HWND    m_hRouterWnd;
   MenuData *m_pParent;
   CustMenu *m_cm;

public:
   static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

   MenuData(HWND hParentWnd, HWND hRouterWnd, MenuData *pParent, POINT pt, CustMenu *cm, bool bCreateAsBar = false);
   ~MenuData();
   void ConvertToBar();

   // Message Handlers
   LRESULT OnKillFocus(HWND hNewWnd);
   LRESULT OnCommand(WORD wNotifyCode, WORD wID, HWND hCtlWnd);
   LRESULT OnPaint(HDC hDC);
   LRESULT OnMouseMove(int xPos, int yPos, int fwKeys);
   LRESULT OnLButtonDown(int xPos, int yPos, int fwKeys);
   LRESULT OnLButtonUp(int xPos, int yPos, int fwKeys);
   LRESULT OnTimer(UINT wTimerID, TIMERPROC *tmprc);

   int HitTest(const POINT &in);
   int GetItemStart(int nItem);
   int SelectItem(int nItem);
   void GetSize(SIZE &size);
};

// ============================================================================
class CustMenu : public ICustMenu
{
public:
   TCHAR  *m_sTitle;
   bool    m_bCanConvertToBar;

   Tab <MenuItemCustom*> m_items;
   Tab <MenuData*> m_menus;

public:
   static HINSTANCE  hInstance;
   static HFONT      hFont;
   static HICON      hCheckIco;
   static HICON      hCheckSelIco;

   CustMenu(const TCHAR *sTitle, bool bCanConvertToBar);
   ~CustMenu();

   void Destroy();

   void PopupMenu(HWND hParentWnd, POINT pt);
   void PopupBar(HWND hParentWnd, POINT pt);

   bool AppendHMenu(HMENU hMenu);
   int  AddItem(const TCHAR *sText, UINT nID, bool bChecked = false, bool bEnabled = true);
   int  AddSubMenu(ICustMenu *custMenu);
   int  AddSeparator();
   void DelItem(int nItem);
   int  GetItemCount();

   void SetItemCheck(int nItem, bool bChecked);
   bool GetItemCheck(int nItem);

   void SetItemText(int nItem, const TCHAR *sText);
   const TCHAR* GetItemText(int nItem);

   void SetItemEnabled(int nItem, bool bEnabled);
   bool GetItemEnabled(int nItem);
};
HINSTANCE CustMenu::hInstance = NULL;
HFONT CustMenu::hFont = NULL;
HICON CustMenu::hCheckIco = NULL;
HICON CustMenu::hCheckSelIco = NULL;


// ============================================================================
MenuData::MenuData(HWND hParentWnd, HWND hRouterWnd, MenuData *pParent, POINT pt, CustMenu *cm, bool bCreateAsBar)
{
   m_pParent = pParent;
   m_hParentWnd = hParentWnd;
   m_hRouterWnd = hRouterWnd;
   m_cm = cm;
   m_bIsMenuBar = (m_cm->m_bCanConvertToBar && bCreateAsBar) ? true : false;
   m_bMouseOver = false;
   m_nSelected  = -2;
   m_nTimerID = 1;
   m_nSelStart = 0;

   DWORD dwExStyle = 0;
   DWORD dwStyle = WS_VISIBLE | WS_POPUP | WS_DLGFRAME | WS_CLIPSIBLINGS;

   if(m_bIsMenuBar)
   {
      dwExStyle |= WS_EX_TOOLWINDOW;
      dwStyle   |= WS_CAPTION | WS_SYSMENU;
   }

   m_hWnd = CreateWindowEx(
      dwExStyle,
      CUSTMENU_CLASS,
      m_cm->m_sTitle,
      dwStyle,
      0, 0, 0, 0,
      (m_bIsMenuBar ? NULL : m_hParentWnd),
      NULL,
      CustMenu::hInstance,
      this);

   SendMessage(m_hWnd, WM_SETFONT, (WPARAM)CustMenu::hFont, 0);

   SIZE size;
   GetSize(size);
   MoveWindow(m_hWnd, pt.x, pt.y, size.cx, size.cy, TRUE);

   MenuData *md = this;
   m_cm->m_menus.Append(1, &md);
}

// ============================================================================
MenuData::~MenuData()
{
   if(m_hWnd)
   {
      DLSetWindowLongPtr(m_hWnd, NULL);
      DestroyWindow(m_hWnd);
   }
}

// ============================================================================
void MenuData::ConvertToBar()
{
   m_bIsMenuBar = true;
   m_bMouseOver = false;
   m_nSelected = -2;
   ReleaseCapture();

   RECT rect;
   GetWindowRect(m_hWnd, &rect);
   rect.right -= rect.left;
   rect.bottom -= rect.top;

   SetWindowLong(m_hWnd, GWL_STYLE, GetWindowLong(m_hWnd, GWL_STYLE) | WS_CAPTION | WS_SYSMENU);
   SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW);
   SetWindowPos(m_hWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_FRAMECHANGED);
   UpdateWindow(m_hWnd);
// SetParent(m_hWnd, m_hRouterWnd);
}

// ============================================================================
LRESULT CALLBACK MenuData::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   MenuData *md = DLGetWindowLongPtr<MenuData*>(hWnd);

   if(md == NULL && msg != WM_CREATE)
      return DefWindowProc(hWnd, msg, wParam, lParam);
   else if(msg == WM_CREATE)
   {
      LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
      md = (MenuData*)lpcs->lpCreateParams;
      DLSetWindowLongPtr(hWnd, md);    
      return 0;
   }

   switch(msg)
   {
   case WM_CLOSE:
      if(md->m_bIsMenuBar)
         delete md;
      break;

   case WM_KILLFOCUS:
      return md->OnKillFocus((HWND)wParam);

   case WM_COMMAND:
      return md->OnCommand(HIWORD(wParam), LOWORD(wParam), (HWND)lParam);

   case WM_MOUSEMOVE:
      return md->OnMouseMove((short)LOWORD(lParam), (short)HIWORD(lParam), wParam);

   case WM_LBUTTONDOWN:
      return md->OnLButtonDown((short)LOWORD(lParam), (short)HIWORD(lParam), wParam);

   case WM_LBUTTONUP:
      return md->OnLButtonUp((short)LOWORD(lParam), (short)HIWORD(lParam), wParam);

   case WM_PAINT:
      {
         PAINTSTRUCT ps;
         BeginPaint(hWnd,&ps);
         int res = md->OnPaint(ps.hdc);
         EndPaint(hWnd,&ps);
         return res;
      }

   case WM_TIMER:
      return md->OnTimer(wParam, (TIMERPROC*)lParam);

   default:
      return DefWindowProc(hWnd, msg, wParam, lParam);
   }
   return 0;
}

// ============================================================================
LRESULT MenuData::OnKillFocus(HWND hNewWnd)
{
   if (hNewWnd == m_hWnd) return 0;
   if(!m_bIsMenuBar && (GetParent(hNewWnd) != m_hWnd)) {
      MenuData *parent=NULL;
      if (m_pParent && m_pParent->m_hWnd != hNewWnd) parent=m_pParent;
      delete this;
      for (;parent;) {
         MenuData *nextparent=NULL;
         if (parent->m_pParent && parent->m_pParent->m_hWnd != hNewWnd) nextparent=parent->m_pParent;
         delete parent;
         parent=nextparent;
      }
   }
   else
   {
      m_bMouseOver = false;
      ReleaseCapture();
   }

   return 0;
}

// ============================================================================
LRESULT MenuData::OnCommand(WORD wNotifyCode, WORD wID, HWND hCtlWnd)
{
   return SendMessage(m_hRouterWnd, WM_COMMAND, MAKEWPARAM(wID, wNotifyCode), (LPARAM)hCtlWnd);
}

// ============================================================================
LRESULT MenuData::OnPaint(HDC hDC)
{
   RECT rect;
   GetClientRect(m_hWnd, &rect);
   rect.right += rect.left;
   rect.bottom += rect.top;

   SetBkMode(hDC, TRANSPARENT);
   SelectObject(hDC, CustMenu::hFont);

   int x = c_textLead, y = 0;
   if(!m_bIsMenuBar && m_cm->m_bCanConvertToBar)
   {
      if(m_nSelected == -1)
      {
         RECT r;
         r.left = 0;
         r.top  = m_nSelStart;
         r.right = rect.right;
         r.bottom = r.top + g_hdrHeight;

         HBRUSH hBrush = CreateSolidBrush(RGB(10,36,106));
         FillRect(hDC, &r, hBrush);
         DeleteObject(hBrush);
      }

      int y2 = g_hdrHeight / 2 - 3;

      for(int i = 0; i < 2; i++, y2+=4)
      {
         SelectObject(hDC, CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DHILIGHT)));
         MoveToEx(hDC, rect.left+4, y2+1, NULL);
         LineTo(hDC, rect.left+4, y2);
         LineTo(hDC, rect.right-4, y2);

         DeleteObject(SelectObject(hDC, CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DSHADOW))));
         MoveToEx(hDC, rect.left+4, y2+2, NULL);
         LineTo(hDC, rect.right-4, y2+2);
         LineTo(hDC, rect.right-4, y2-1);

         DeleteObject(SelectObject(hDC, CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DFACE))));
         MoveToEx(hDC, rect.left+5, y2+1, NULL);
         LineTo(hDC, rect.right-4, y2+1);

         DeleteObject(SelectObject(hDC, GetStockObject(BLACK_PEN)));
      }
      y += g_hdrHeight;
   }

   int count = m_cm->m_items.Count();
   for(int i = 0; i < count; i++)
   {
      MenuItemCustom *mi = m_cm->m_items[i];
      if(mi->m_bIsSep)
      {
         SelectObject(hDC, CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DSHADOW)));
         MoveToEx(hDC, rect.left+4, y+4, NULL);
         LineTo(hDC, rect.right-4, y+4);
         DeleteObject(SelectObject(hDC, CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DHILIGHT))));
         MoveToEx(hDC, rect.left+4, y+5, NULL);
         LineTo(hDC, rect.right-4, y+5);
         DeleteObject(SelectObject(hDC, GetStockObject(BLACK_PEN)));
         y += c_sepHeight;
      }
      else
      {
         if(i == m_nSelected)
         {
            RECT r;
            r.left = 0;
            r.top  = m_nSelStart;
            r.right = rect.right;
            r.bottom = r.top + c_itemHeight;

            if(mi->m_hIcon || mi->m_hIconSel)
               r.left += c_textLead-2;

            HBRUSH hBrush = CreateSolidBrush(RGB(10,36,106));
            FillRect(hDC, &r, hBrush);
            SetTextColor(hDC, RGB(255,255,255));
            DLTextOut(hDC, x, y+2, mi->m_sText);
            DeleteObject(hBrush);
         }
         else
         {
            SetTextColor(hDC, RGB(0,0,0));
            DLTextOut(hDC, x, y+2, mi->m_sText);
         }

         if(i == m_nSelected && mi->m_hIconSel)
            DrawIconEx(hDC, 0, y, mi->m_hIconSel, 0, 0, 0 , NULL, DI_NORMAL);
         else if(mi->m_hIcon)
            DrawIconEx(hDC, 0, y, mi->m_hIcon, 0, 0, 0 , NULL, DI_NORMAL);

         if(mi->m_pSubMenu)
         {
            HICON hIcon = (HICON)LoadImage(CustMenu::hInstance,
               MAKEINTRESOURCE((i==m_nSelected?IDI_SUBMENUSEL:IDI_SUBMENU)),
               IMAGE_ICON, c_itemHeight, c_itemHeight, LR_DEFAULTCOLOR);
            DrawIconEx(hDC, rect.right-c_itemHeight, y, hIcon, 0, 0, 0 , NULL, DI_NORMAL);
            DeleteObject(hIcon);
         }
         
         y += c_itemHeight;
      }
   }

   return 0;
}

// ============================================================================
LRESULT MenuData::OnMouseMove(int xPos, int yPos, int fwKeys)
{
   POINT pt = {xPos,yPos};

   if(m_pParent && m_pParent->m_nSelected != m_pParent->m_nSubMenuUp)
      m_pParent->SelectItem(m_pParent->m_nSubMenuUp);

   int nSel = HitTest(pt);
   if(m_bMouseOver)
   {
      RECT rect;
      GetClientRect(m_hWnd, &rect);

      if(!PtInRect(&rect, pt))
      {
         nSel = -2;
         m_bMouseOver = false;
         ReleaseCapture();
      }
   }
   else
   {
      m_bMouseOver = true;
//    SetFocus(m_hWnd);
      SetCapture(m_hWnd);
   }

   SelectItem(nSel);

   return 0;
}

// ============================================================================
LRESULT MenuData::OnLButtonDown(int xPos, int yPos, int fwKeys)
{
   SetFocus(m_hWnd);

   if(m_nSelected == -1)
   {
      POINT pt = {xPos, yPos};
      ClientToScreen(m_hWnd, &pt);
      ConvertToBar();

      SendMessage(m_hWnd, WM_NCLBUTTONDOWN, (WPARAM)HTCAPTION, MAKELPARAM(pt.x,pt.y));
   }
   return 0;
}

// ============================================================================
LRESULT MenuData::OnLButtonUp(int xPos, int yPos, int fwKeys)
{
   if(m_nSelected >= 0)
   {
      CustMenu *cm = (CustMenu*)(m_cm->m_items[m_nSelected]->m_pSubMenu);
      if(cm)
      {
/*       if(cm->m_items.Count() > 0)
         {
            RECT rect;
            GetClientRect(m_hWnd, &rect);

            POINT pt;
            pt.y = m_nSelStart;
            pt.x = rect.right;
            ClientToScreen(m_hWnd, &pt);

            cm->PopupMenu(m_hWnd, pt);
         }
*/    }
      else
      {
         OnCommand(0, m_cm->m_items[m_nSelected]->m_nID, NULL);

         if(!m_bIsMenuBar) {
            MenuData *parent=m_pParent;
            delete this;
            for (;parent;) {
               MenuData *nextparent=parent->m_pParent;
               delete parent;
               parent=nextparent;
            }
//          delete this;
         }
      }
   }

   return 0;
}

// ============================================================================
LRESULT MenuData::OnTimer(UINT wTimerID, TIMERPROC *tmprc)
{
   if(m_nTimerID == wTimerID && m_nSelected != m_nSubMenuUp)
   {
      SetFocus(m_hWnd);
      m_nSubMenuUp = -1;

      if(m_nSelected >= 0)
      {
         CustMenu *cm = (CustMenu*)(m_cm->m_items[m_nSelected]->m_pSubMenu);
         if(cm && cm->m_items.Count() > 0)
         {
            RECT rect;
            GetClientRect(m_hWnd, &rect);

            POINT pt;
            pt.y = m_nSelStart;
            pt.x = rect.right;
            ClientToScreen(m_hWnd, &pt);

            MenuData *md = new MenuData(m_hWnd, m_hRouterWnd, this, pt, cm);
            m_nSubMenuUp = m_nSelected;
         }
      }
   }
   KillTimer(m_hWnd, m_nTimerID);

   return 0;
}

// ============================================================================
int MenuData::HitTest(const POINT &in)
{
   int y = 0;

   if(!m_bIsMenuBar && m_cm->m_bCanConvertToBar)
   {
      if(in.y >= y && in.y < y+g_hdrHeight)
         return -1;
      y += g_hdrHeight;
   }

   int count = m_cm->m_items.Count();
   for(int i = 0; i < count; i++)
   {
      MenuItemCustom *mi = m_cm->m_items[i];

      if(mi->m_bIsSep)
         y += c_sepHeight;
      else
      {
         if(in.y >= y && in.y < y+c_itemHeight)
            return i;

         y += c_itemHeight;
      }
   }
   return -2;
}

// ============================================================================
int MenuData::SelectItem(int nItem)
{
   RECT rect;
   GetClientRect(m_hWnd, &rect);

   int oldSel = m_nSelected;
   int oldStart = m_nSelStart;

   m_nSelected = nItem;
   m_nSelStart = GetItemStart(m_nSelected);

   if(m_nSelected != oldSel)
   {
//    KillTimer(m_hWnd, m_nTimerID);
      m_nTimerID = SetTimer(m_hWnd, m_nTimerID, c_timerElapse, NULL);

      if(oldSel >= -1)
      {
         rect.top  = oldStart;
         rect.bottom = oldStart + (oldSel == -1 ? g_hdrHeight : c_itemHeight);
         InvalidateRect(m_hWnd, &rect, TRUE);
      }

      if(m_nSelected >= -1)
      {
         rect.top  = m_nSelStart;
         rect.bottom = rect.top + (m_nSelected == -1 ? g_hdrHeight : c_itemHeight);
         InvalidateRect(m_hWnd, &rect, TRUE);
      }
   }

   return oldSel;
}

// ============================================================================
int MenuData::GetItemStart(int nItem)
{
   if(nItem < 0 || nItem >= m_cm->m_items.Count())
      return 0;

   int y = 0;

   if(!m_bIsMenuBar && m_cm->m_bCanConvertToBar)
      y += g_hdrHeight;

   for(int i = 0; i < nItem; i++)
   {
      MenuItemCustom *mi = m_cm->m_items[i];
      if(mi->m_bIsSep)
         y += c_sepHeight;
      else
         y += c_itemHeight;
   }
   return y;
}

// ============================================================================
void MenuData::GetSize(SIZE &size)
{
   size.cx = GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
   size.cy = GetSystemMetrics(SM_CYFIXEDFRAME) * 2;

   size.cx += c_textLead;
// size.cx += c_textTrail;

   if(m_cm->m_bCanConvertToBar)
   {
      g_hdrHeight = GetSystemMetrics(SM_CYSMCAPTION);
      size.cy += g_hdrHeight;
   }

   int textSize = 0;
   int count = m_cm->m_items.Count();
   for(int i = 0; i < count; i++)
   {
      MenuItemCustom *mi = m_cm->m_items[i];

      if(mi->m_bIsSep)
         size.cy += c_sepHeight;
      else
      {
         SIZE tmp;
         SelectObject(GetDC(m_hWnd), CustMenu::hFont);
         DLGetTextExtent(GetDC(m_hWnd), mi->m_sText, &tmp);
         LPtoDP(GetDC(m_hWnd), (LPPOINT)&tmp, 1);
         if(tmp.cx > textSize) textSize = tmp.cx;
         size.cy += c_itemHeight;
      }
   }
   size.cx += textSize;
}

// ============================================================================
bool ICustMenu::Init(HINSTANCE hInstance, HFONT hFont)
{
   CustMenu::hInstance = hInstance;
   CustMenu::hFont = hFont;

   static bool bRegistered = false;
   if(!bRegistered)
   {
      WNDCLASSEX wcex;
      wcex.cbSize        = sizeof(WNDCLASSEX);
      wcex.style         = CS_HREDRAW | CS_VREDRAW;
      wcex.lpfnWndProc   = MenuData::WndProc;
      wcex.cbClsExtra    = 0;
      wcex.cbWndExtra    = 0;
      wcex.hInstance     = CustMenu::hInstance;
      wcex.hIcon         = NULL;
      wcex.hIconSm       = NULL;
      wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);;
      wcex.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
      wcex.lpszMenuName  = NULL;
      wcex.lpszClassName  = CUSTMENU_CLASS;

      if(!RegisterClassEx(&wcex))
         return false;

      CustMenu::hCheckIco = (HICON)LoadImage(CustMenu::hInstance,
            MAKEINTRESOURCE(IDI_CHECK), IMAGE_ICON,
            c_itemHeight, c_itemHeight, LR_DEFAULTCOLOR);

      CustMenu::hCheckSelIco = (HICON)LoadImage(CustMenu::hInstance,
            MAKEINTRESOURCE(IDI_CHECKSEL), IMAGE_ICON,
            c_itemHeight, c_itemHeight, LR_DEFAULTCOLOR);

      bRegistered = true;
   }
   return true;
}

// ============================================================================
ICustMenu* ICustMenu::Create(const TCHAR *sTitle, bool bCanConvertToBar)
{
   return new CustMenu(sTitle, bCanConvertToBar);
}

// ============================================================================
void CustMenu::Destroy()
{
   delete this;
}

// ============================================================================
CustMenu::CustMenu(const TCHAR *sTitle, bool bCanConvertToBar)
{
   m_sTitle = DupStr(sTitle);
   m_bCanConvertToBar = bCanConvertToBar;
}

// ============================================================================
CustMenu::~CustMenu()
{
   for(int i = 0, count = m_menus.Count(); i < count; i++)
   {
      MenuData *md = m_menus[i];
      delete md;
   }

   for(int i = 0, count = m_items.Count(); i < count; i++)
   {
      MenuItemCustom *mi = m_items[i];
      if(mi->m_pSubMenu)
         delete mi->m_pSubMenu;

      delete mi;
   }

   if(m_sTitle)
   {
      delete m_sTitle;
      m_sTitle = NULL;
   }
}

// ============================================================================
void CustMenu::PopupMenu(HWND hParentWnd, POINT pt)
{
   MenuData *md = new MenuData(hParentWnd, hParentWnd, NULL, pt, this);
}

// ============================================================================
void CustMenu::PopupBar(HWND hParentWnd, POINT pt)
{
   MenuData *md = new MenuData(hParentWnd, hParentWnd, NULL, pt, this, true);
}

// ============================================================================
bool CustMenu::AppendHMenu(HMENU hMenu)
{
   static char dwTypeData[1024];

   int count = GetMenuItemCount(hMenu);
   for(int i = 0; i < count; i++)
   {
      MENUITEMINFO mii;
      memset(&mii, 0, sizeof(MENUITEMINFO));
      mii.cbSize = sizeof(MENUITEMINFO);
      mii.fMask = MIIM_DATA|MIIM_ID|MIIM_STATE|MIIM_SUBMENU|MIIM_TYPE;
      mii.dwTypeData = dwTypeData;
      mii.cch = sizeof(dwTypeData);

      GetMenuItemInfo(hMenu, i, TRUE, &mii);

      if(mii.fType == MFT_SEPARATOR)
         AddSeparator();
      else if(mii.fType == MFT_STRING)
      {
         if(mii.hSubMenu)
         {
            CustMenu *cm = new CustMenu(mii.dwTypeData, false);
            cm->m_bCanConvertToBar = m_bCanConvertToBar;
            cm->AppendHMenu(mii.hSubMenu);
            AddSubMenu(cm);
         }
         else
         {
            AddItem(mii.dwTypeData, mii.wID,
               ((mii.fState&MFS_CHECKED) != 0),
               ((mii.fState&MFS_ENABLED) != 0));
         }
      }
   }

   return true;
}

// ============================================================================
int CustMenu::AddItem(const TCHAR *sText, UINT nID, bool bChecked, bool bEnabled)
{
   MenuItemCustom *menuItem = new MenuItemCustom;
   if(!menuItem) return -1;

   menuItem->m_sText = DupStr(sText);
   menuItem->m_nID = nID;
   menuItem->m_bChecked = bChecked;
   menuItem->m_bEnabled = bEnabled;
   menuItem->m_bIsSep = false;
   menuItem->m_pSubMenu = NULL;
   if(bChecked)
   {
      menuItem->m_hIcon = CustMenu::hCheckIco;
      menuItem->m_hIconSel = CustMenu::hCheckSelIco;
   }

   return m_items.Append(1, &menuItem);
}

// ============================================================================
int CustMenu::AddSubMenu(ICustMenu *icustMenu)
{
   MenuItemCustom *menuItem = new MenuItemCustom;
   if(!menuItem) return -1;

   CustMenu *custMenu = (CustMenu*)icustMenu;

   menuItem->m_sText = DupStr(custMenu->m_sTitle);
   menuItem->m_nID = 0;
   menuItem->m_bChecked = false;
   menuItem->m_bEnabled = true;
   menuItem->m_bIsSep = false;
   menuItem->m_pSubMenu = icustMenu;

   return m_items.Append(1, &menuItem);
}

// ============================================================================
int CustMenu::AddSeparator()
{
   MenuItemCustom *sep = new MenuItemCustom;
   if(!sep) return -1;

   sep->m_sText = NULL;
   sep->m_nID = 0;
   sep->m_bChecked = false;
   sep->m_bEnabled = true;
   sep->m_bIsSep = true;
   sep->m_pSubMenu = NULL;

   return m_items.Append(1, &sep);
}

// ============================================================================
void CustMenu::DelItem(int nItem)
{
   delete m_items[nItem];
   m_items.Delete(nItem, 1);
}

// ============================================================================
int CustMenu::GetItemCount()
{
   return m_items.Count();
}

// ============================================================================
void CustMenu::SetItemCheck(int nItem, bool bChecked)
{
   assert(nItem >= 0 && nItem < m_items.Count());
   MenuItemCustom *pItem = m_items[nItem];
   if(pItem && !pItem->m_pSubMenu)
   {
      pItem->m_bChecked = bChecked;

      if(bChecked)
      {
         pItem->m_hIcon = CustMenu::hCheckIco;
         pItem->m_hIconSel = CustMenu::hCheckSelIco;
      }
      else
      {
         pItem->m_hIcon = NULL;
         pItem->m_hIconSel = NULL;
      }
   }
}

// ============================================================================
bool CustMenu::GetItemCheck(int nItem)
{
   assert(nItem >= 0 && nItem < m_items.Count());
   MenuItemCustom *pItem = m_items[nItem];
   if(pItem && !pItem->m_pSubMenu)
      return pItem->m_bChecked;
   return false;
}

// ============================================================================
void CustMenu::SetItemText(int nItem, const TCHAR *sText)
{
   assert(nItem >= 0 && nItem < m_items.Count());
   MenuItemCustom *pItem = m_items[nItem];
   if(pItem && !pItem->m_pSubMenu)
      pItem->m_sText = DupStr(sText);
}

// ============================================================================
const TCHAR* CustMenu::GetItemText(int nItem)
{
   assert(nItem >= 0 && nItem < m_items.Count());
   MenuItemCustom *pItem = m_items[nItem];
   if(pItem && !pItem->m_pSubMenu)
      return pItem->m_sText;
   return NULL;
}

// ============================================================================
void CustMenu::SetItemEnabled(int nItem, bool bEnabled)
{
   assert(nItem >= 0 && nItem < m_items.Count());
   MenuItemCustom *pItem = m_items[nItem];
   if(pItem && !pItem->m_pSubMenu)
      pItem->m_bEnabled = bEnabled;
}

// ============================================================================
bool CustMenu::GetItemEnabled(int nItem)
{
   assert(nItem >= 0 && nItem < m_items.Count());
   MenuItemCustom *pItem = m_items[nItem];
   if(pItem && !pItem->m_pSubMenu)
      return pItem->m_bEnabled;
   return false;
}


