// ============================================================================
// LinkCtrl.cpp
// Copyright ©1999 Discreet
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#include "MAXScrpt.h"
#include "MAXObj.h"
#include "Numbers.h"
#include "ColorVal.h"
#include "Parser.h"
#include <shellapi.h>
#include <WindowsDefines.h>
#include <windowsx.h>
#include <rollouts.h>
#include <icolorman.h>

extern HINSTANCE g_hInst;
#include "3dsmaxport.h"
#include "resource.h"

#ifdef ScripterExport
   #undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include <maxscript\macros\define_external_functions.h>
#  include "namedefs.h"

#include <maxscript\macros\define_instantiation_functions.h>



#define LINKCTRL_WINDOWCLASS _T("LINKCTRL_WINDOWCLASS")
#define TOOLTIP_ID 1

// ============================================================================
visible_class(LinkControl)

class LinkControl : public RolloutControl
{
protected:
   HWND     m_hWnd;
   HWND     m_hToolTip;
   HFONT    m_hFont;
   TSTR     m_address;
   COLORREF m_color;
   COLORREF m_hoverColor;
   COLORREF m_visitedColor;
   bool     m_colorSpecified, m_hoverColorSpecified;
   bool     m_bMouseOver;
   bool     m_bVisited;

public:
   // Static methods
   static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

   static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
      { return new LinkControl(name, caption, keyparms, keyparm_count); }

   void SetAddress(TCHAR *text);
   void SetText(TCHAR *text);
   void SetUnderline(BOOL bUnderline);
   void Invalidate();
   void FillToolInfo(TOOLINFO *ti);

   // Event handlers
   LRESULT LButtonDown(int xPos, int yPos, int fwKeys);
   LRESULT LButtonUp(int xPos, int yPos, int fwKeys);
   LRESULT MouseMove(int xPos, int yPos, int fwKeys);
   LRESULT EraseBackground(HDC hDC);
   LRESULT Paint(HDC hdc);

   // MAXScript event handlers
   void CallChangedHandler();

   LinkControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
   ~LinkControl();

            classof_methods(LinkControl, RolloutControl);
   void     collect() { delete this; }
   void     sprin1(CharStream* s) { s->printf(_T("HyperLinkControl:%s"), name->to_string()); }

   void     add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y);
   LPCTSTR  get_control_class() { return LINKCTRL_WINDOWCLASS; }
   void     compute_layout(Rollout *ro, layout_data* pos);
   BOOL     handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);

   Value*   get_property(Value** arg_list, int count);
   Value*   set_property(Value** arg_list, int count);
   void     set_enable();
};

// ============================================================================
LRESULT CALLBACK LinkControl::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   LinkControl *ctrl = DLGetWindowLongPtr<LinkControl*>(hWnd);
   if(ctrl == NULL && msg != WM_CREATE)
      return DefWindowProc(hWnd, msg, wParam, lParam);

   if(ctrl && ctrl->m_hToolTip)
   {
      MSG ttmsg;
      ttmsg.lParam = lParam;
      ttmsg.wParam = wParam;
      ttmsg.message = msg;
      ttmsg.hwnd = hWnd;
      SendMessage(ctrl->m_hToolTip, TTM_RELAYEVENT, 0, (LPARAM)(LPMSG)&ttmsg);
   }

   switch(msg)
   {
   case WM_CREATE:
      {
         LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
         ctrl = (LinkControl*)lpcs->lpCreateParams;
         DLSetWindowLongPtr(hWnd, ctrl);
         break;
      }

   case WM_KILLFOCUS:
      ctrl->m_bMouseOver = false;
      ctrl->SetUnderline(FALSE);
      break;

   case WM_LBUTTONDOWN:
      return ctrl->LButtonDown((short)LOWORD(lParam), (short)HIWORD(lParam), wParam);

   case WM_LBUTTONUP:
      return ctrl->LButtonUp((short)LOWORD(lParam), (short)HIWORD(lParam), wParam);

   case WM_MOUSEMOVE:
      return ctrl->MouseMove((short)LOWORD(lParam), (short)HIWORD(lParam), wParam);

   case WM_ERASEBKGND:
      return ctrl->EraseBackground((HDC)wParam);

   case WM_PAINT:
      PAINTSTRUCT ps;
      BeginPaint(hWnd,&ps);
      ctrl->Paint(ps.hdc);
      EndPaint(hWnd,&ps);
      return FALSE;

   default:
      return DefWindowProc(hWnd, msg, wParam, lParam);
   }
   return FALSE;
}

// ============================================================================
LinkControl::LinkControl(Value* name, Value* caption, Value** keyparms, int keyparm_count)
   : RolloutControl(name, caption, keyparms, keyparm_count)
{
   tag = class_tag(LinkControl);

   m_hWnd = NULL;
   m_hToolTip = NULL;
   m_hFont = NULL;
   m_address = GetString(IDS_GET_DEFAULT_MXS_HYPERLINK_URL);
   m_color = GetCustSysColor(COLOR_BTNTEXT);
   m_hoverColor = GetCustSysColor(COLOR_HIGHLIGHTTEXT);
   m_colorSpecified = false;
   m_hoverColorSpecified = false;
   m_visitedColor = RGB(0,0,192);
   m_bMouseOver = false;
   m_bVisited = false;
}

LinkControl::~LinkControl()
{
   if(m_hFont) DeleteObject(m_hFont);
}

// ============================================================================
void LinkControl::SetText(TCHAR *text)
{
   SIZE size;
   POINT pnt = {0,0};

   SetWindowText(m_hWnd, text);
   DLGetTextExtent(parent_rollout->rollout_dc, text, &size);
   MapWindowPoints(m_hWnd, parent_rollout->page, &pnt, 1);
   MoveWindow(m_hWnd, pnt.x, pnt.y, size.cx, parent_rollout->text_height+2, TRUE);

   TOOLINFO ti;
   FillToolInfo(&ti);
   SendMessage(m_hToolTip, TTM_SETTOOLINFO, 0, (LPARAM)&ti);
}

// ============================================================================
void LinkControl::SetAddress(TCHAR *text)
{
   m_address = text;

   TOOLINFO ti;
   FillToolInfo(&ti);
   SendMessage(m_hToolTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
}

// ============================================================================
void LinkControl::SetUnderline(BOOL bUnderline)
{
   LOGFONT lf;
   GetObject(m_hFont, sizeof(LOGFONT), &lf);
   lf.lfUnderline = bUnderline;
   DeleteObject(m_hFont);
   m_hFont = CreateFontIndirect(&lf);
}

// ============================================================================
void LinkControl::Invalidate()
{
   if (m_hWnd == NULL) return;
   RECT rect;
   GetClientRect(m_hWnd, &rect);
   MapWindowPoints(m_hWnd, parent_rollout->page, (POINT*)&rect, 2);
   InvalidateRect(parent_rollout->page, &rect, TRUE);
   InvalidateRect(m_hWnd, NULL, TRUE);
}

// ============================================================================
void LinkControl::FillToolInfo(TOOLINFO *ti)
{
   memset(ti, 0, sizeof(TOOLINFO));
   ti->cbSize = sizeof(TOOLINFO);
   ti->hwnd = m_hWnd;
   ti->uId = TOOLTIP_ID;
   ti->lpszText = (LPSTR)m_address;
   GetClientRect(m_hWnd, &ti->rect);
}

// ============================================================================
LRESULT LinkControl::LButtonDown(int xPos, int yPos, int fwKeys)
{
   SetFocus(m_hWnd);
   return TRUE;
}

// ============================================================================
LRESULT LinkControl::LButtonUp(int xPos, int yPos, int fwKeys)
{
   HINSTANCE hShell = ShellExecute(MAXScript_interface->GetMAXHWnd(), _T("open"), (TCHAR*)m_address, NULL, NULL, SW_SHOW);
   if(reinterpret_cast<INT_PTR>(hShell) > HINSTANCE_ERROR)
   {
      m_bVisited = true;
      Invalidate();
   }

/* init_thread_locals();
   push_alloc_frame();
   one_value_local(arg);
   vl.arg = new Point2Value(xPos, yPos);
   run_event_handler(parent_rollout, n_clicked, &vl.arg, 1);
   pop_value_locals();
   pop_alloc_frame();
*/
   return TRUE;
}

// ============================================================================
LRESULT LinkControl::MouseMove(int xPos, int yPos, int fwKeys)
{
   if(m_bMouseOver)
   {
      RECT rect;
      POINT pnt = {xPos,yPos};
      GetClientRect(m_hWnd, &rect);

      if(!PtInRect(&rect, pnt))
      {
         m_bMouseOver = false;
         SetUnderline(FALSE);
         ReleaseCapture();
         Invalidate();
      }
   }
   else
   {
      m_bMouseOver = true;
      SetUnderline(TRUE);
      SetFocus(m_hWnd);
      Invalidate();
      SetCapture(m_hWnd);
   }

   return TRUE;
}

// ============================================================================
LRESULT LinkControl::EraseBackground(HDC hDC)
{
// HBRUSH hbr = (HBRUSH)SendMessage(GetParent(m_hWnd), WM_CTLCOLORSTATIC, (WPARAM)hDC, (LPARAM)m_hWnd);
// SetClassLong(m_hWnd, GCL_HBRBACKGROUND, (long)hbr);
// return DefWindowProc(m_hWnd, WM_ERASEBKGND, (WPARAM)hDC, 0);
   return 1;
}

// ============================================================================
LRESULT LinkControl::Paint(HDC hdc)
{
   TCHAR text[256];
   GetWindowText(m_hWnd, text, sizeof(text));

   if(m_bMouseOver)
   {
      COLORREF t_hoverColor = (m_hoverColorSpecified) ? m_hoverColor : GetCustSysColor(COLOR_HIGHLIGHTTEXT);
      SetTextColor(hdc, t_hoverColor);
   }
   else if(m_bVisited)
      SetTextColor(hdc, m_visitedColor);
   else // if(IsWindowEnabled(m_hWnd))
   {
      COLORREF t_color = (m_colorSpecified) ? m_color : GetCustSysColor(COLOR_BTNTEXT);
      SetTextColor(hdc, t_color);
   }
// else
//    SetTextColor(hdc, RGB(128,128,128));

   SetBkMode(hdc, TRANSPARENT);
   SelectFont(hdc, m_hFont);
   DLTextOut(hdc, 0, 0, text);

   return FALSE;
}


// ============================================================================
visible_class_instance (LinkControl, "LinkControl")

void
LinkControl::compute_layout(Rollout *ro, layout_data* pos)
{
   SIZE  size;
   TCHAR*  text = caption->eval()->to_string();

   DLGetTextExtent(ro->rollout_dc, text, &size);   
   pos->width = size.cx; pos->height = ro->text_height + 2;
}

void LinkControl::add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y)
{
   caption = caption->eval();

   TCHAR *text = caption->eval()->to_string();
   control_ID = next_id();
   parent_rollout = ro;

   Value *val = control_param(color);
   if(val != &unsupplied)
   {
      m_color = val->to_colorref();
      m_colorSpecified = true;
   }

   val = control_param(hovercolor);
   if(val != &unsupplied)
   {
      m_hoverColor = val->to_colorref();
      m_hoverColorSpecified = true;
   }

   val = control_param(visitedcolor);
   if(val != &unsupplied)
      m_visitedColor = val->to_colorref();

   val = control_param(address);
   if(val != &unsupplied)
      m_address = val->to_string();

   layout_data pos;
   RolloutControl::compute_layout(ro, &pos, current_y);

   m_hWnd = CreateWindow(
      LINKCTRL_WINDOWCLASS,
      text,
      WS_VISIBLE | WS_CHILD | WS_GROUP,
      pos.left, pos.top, pos.width, pos.height,
      parent, (HMENU)control_ID, g_hInst, this);

   m_hToolTip = CreateWindow(TOOLTIPS_CLASS, NULL, WS_POPUP, //TTS_ALWAYSTIP,
      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
      m_hWnd, (HMENU)NULL, g_hInst, NULL);

   TOOLINFO ti;
   FillToolInfo(&ti);
   SendMessage(m_hToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);

   LOGFONT lf;
   GetObject(ro->font, sizeof(LOGFONT), &lf);
   m_hFont = CreateFontIndirect(&lf);
   SendDlgItemMessage(parent, control_ID, WM_SETFONT, (WPARAM)m_hFont, 0L);
}


// ============================================================================
BOOL LinkControl::handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_CONTEXTMENU)
	{
		call_event_handler(ro, n_rightClick, NULL, 0);
		return TRUE;
	}
   return FALSE;
}


// ============================================================================
Value* LinkControl::get_property(Value** arg_list, int count)
{
   Value* prop = arg_list[0];

   if(prop == n_color)
   {
      if(parent_rollout && parent_rollout->page)
      {
         COLORREF t_color = (m_colorSpecified) ? m_color : GetCustSysColor(COLOR_BTNTEXT);
         return ColorValue::intern(AColor(t_color));
      }
      else
         return &undefined;
   }
   else if(prop == n_hovercolor)
   {
      if(parent_rollout && parent_rollout->page)
      {
         COLORREF t_hoverColor = (m_hoverColorSpecified) ? m_hoverColor : GetCustSysColor(COLOR_HIGHLIGHTTEXT);
         return ColorValue::intern(AColor(t_hoverColor));
      }
      else
         return &undefined;
   }
   else if(prop == n_visitedcolor)
   {
      if(parent_rollout && parent_rollout->page)
         return ColorValue::intern(AColor(m_visitedColor));
      else
         return &undefined;
   }
   else if(prop == n_address)
   {
      if(parent_rollout && parent_rollout->page)
         return new String(m_address);
      else
         return &undefined;
   }

   return RolloutControl::get_property(arg_list, count);
}


// ============================================================================
Value* LinkControl::set_property(Value** arg_list, int count)
{
   Value* val = arg_list[0];
   Value* prop = arg_list[1];

   if(prop == n_color)
   {
      if(parent_rollout && parent_rollout->page) {
         m_color = val->to_colorref();
         m_colorSpecified = true;
         InvalidateRect(m_hWnd, NULL, FALSE);
      }
   }
   else if(prop == n_hovercolor)
   {
      if(parent_rollout && parent_rollout->page) {
         m_hoverColor = val->to_colorref();
         m_hoverColorSpecified = true;
         InvalidateRect(m_hWnd, NULL, FALSE);
      }
   }
   else if(prop == n_visitedcolor)
   {
      if(parent_rollout && parent_rollout->page) {
         m_visitedColor = val->to_colorref();
         InvalidateRect(m_hWnd, NULL, FALSE);
      }
   }
   else if(prop == n_address)
   {
      if(parent_rollout && parent_rollout->page)
         SetAddress(val->to_string());
   }
   else if(prop == n_text)
   {
      if(parent_rollout && parent_rollout->page)
         SetText(val->to_string());
   }
   else
      return RolloutControl::set_property(arg_list, count);

   return val;
}


// ============================================================================
void LinkControl::set_enable()
{
   if(parent_rollout != NULL && parent_rollout->page != NULL)
   {
      EnableWindow(m_hWnd, enabled);
      InvalidateRect(m_hWnd, NULL, TRUE);
   }
}


// ============================================================================
void LinkCtrlInit()
{
   static BOOL registered = FALSE;
   if(!registered)
   {
      WNDCLASSEX wcex;
      wcex.cbSize        = sizeof(WNDCLASSEX);
      wcex.style         = CS_HREDRAW | CS_VREDRAW;
      wcex.lpfnWndProc   = LinkControl::WndProc;
      wcex.cbClsExtra    = 0;
      wcex.cbWndExtra    = 0;
      wcex.hInstance     = g_hInst;
      wcex.hIcon         = NULL;
      wcex.hIconSm       = NULL;
      wcex.hCursor       = LoadCursor(g_hInst, MAKEINTRESOURCE(IDC_HAND_LINKCONTROL));
      wcex.hbrBackground = NULL;
      wcex.lpszMenuName  = NULL;
      wcex.lpszClassName  = LINKCTRL_WINDOWCLASS;

      if(!RegisterClassEx(&wcex))
         return;
      registered = TRUE;
   }

   install_rollout_control(Name::intern("HyperLink"), LinkControl::create);
}

