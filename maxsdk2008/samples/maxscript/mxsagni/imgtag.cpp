// ============================================================================
// ImgTag.cpp
// Copyright ©1999 Discreet
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#include "MAXScrpt.h"
#include "BitMaps.h"
#include "Numbers.h"
#include "ColorVal.h"
#include "3DMath.h"
#include "Parser.h"
#include "MXSAgni.h"

extern HINSTANCE g_hInst;

extern bool bRunningW2K;
// following are for methods defined in W2K, but not NT
extern transparentBlt TransparentBlt_i;
extern alphaBlend AlphaBlend_i;

#include "resource.h"

#ifdef ScripterExport
   #undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "defextfn.h"
#  include "namedefs.h"

#include "definsfn.h"

#include "3dsmaxport.h"

#define IMGTAG_WINDOWCLASS _T("IMGTAG_WINDOWCLASS")
#define TOOLTIP_ID 1


// ============================================================================
visible_class(ImgTag)

class ImgTag : public RolloutControl
{
protected:
   HWND     m_hWnd;
   HWND     m_hToolTip;
   TSTR     m_sToolTip;
   bool   m_bMouseOver;
   float    m_opacity;
   COLORREF m_transparent;

   HBITMAP     m_hBitmap;
   Value      *m_maxBitMap;
   Value      *m_style;

public:
   // Static methods
   static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

   static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
      { return new ImgTag(name, caption, keyparms, keyparm_count); }

   ImgTag(Value* name, Value* caption, Value** keyparms, int keyparm_count);
   ~ImgTag();

   int SetBitmap(Value *val);
   TOOLINFO* GetToolInfo();
   void Invalidate();

   enum button {left, middle, right};

   // Event handlers
   LRESULT ButtonDown(int xPos, int yPos, int fwKeys, button which);
   LRESULT ButtonUp(int xPos, int yPos, int fwKeys, button which);
   LRESULT ButtonDblClk(int xPos, int yPos, int fwKeys, button which);
   LRESULT MouseMove(int xPos, int yPos, int fwKeys);

   LRESULT EraseBkgnd(HDC hDC);
   LRESULT Paint(HDC hDC);

   void   run_event_handler (Value* event, int xPos, int yPos, int fwKeys, bool capture = true);

            classof_methods(ImgTag, RolloutControl);
   void     gc_trace();
   void     collect() { delete this; }
   void     sprin1(CharStream* s) { s->printf(_T("ImgTag:%s"), name->to_string()); }

   void     add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y);
   LPCTSTR  get_control_class() { return IMGTAG_WINDOWCLASS; }
   void     compute_layout(Rollout *ro, layout_data* pos) { }
   BOOL     handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);

   Value*   get_property(Value** arg_list, int count);
   Value*   set_property(Value** arg_list, int count);
   void     set_enable();
};

// ============================================================================
ImgTag::ImgTag(Value* name, Value* caption, Value** keyparms, int keyparm_count)
   : RolloutControl(name, caption, keyparms, keyparm_count)
{
   tag = class_tag(ImgTag);

   m_hWnd = NULL;
   m_hToolTip = NULL;
   m_sToolTip = _T("");
   m_hBitmap = NULL;
   m_maxBitMap = NULL;
   m_bMouseOver = false;
   m_style = n_bmp_stretch;
   m_opacity = 0.f;
   m_transparent = 0;
}

// ============================================================================
ImgTag::~ImgTag()
{
   if(m_hBitmap) DeleteObject(m_hBitmap);
}

void ImgTag::gc_trace()
{
   RolloutControl::gc_trace();
   if (m_maxBitMap && m_maxBitMap->is_not_marked())
      m_maxBitMap->gc_trace();
}

// ============================================================================
TOOLINFO* ImgTag::GetToolInfo()
{
   static TOOLINFO ti;

   memset(&ti, 0, sizeof(TOOLINFO));
   ti.cbSize = sizeof(TOOLINFO);
   ti.hwnd = m_hWnd;
   ti.uId = TOOLTIP_ID;
   ti.lpszText = (LPSTR)m_sToolTip;
   GetClientRect(m_hWnd, &ti.rect);

   return &ti;
}

// ============================================================================
LRESULT CALLBACK ImgTag::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   ImgTag *ctrl = DLGetWindowLongPtr<ImgTag*>(hWnd);
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
         ctrl = (ImgTag*)lpcs->lpCreateParams;
         DLSetWindowLongPtr(hWnd, ctrl);
         break;
      }

   case WM_KILLFOCUS:
      if (ctrl->m_bMouseOver)
      {
         ctrl->m_bMouseOver = false;
         ctrl->run_event_handler(n_mouseout, (short)LOWORD(lParam), (short)HIWORD(lParam), wParam, false);
      }
      break;

   case WM_MOUSEMOVE:
      return ctrl->MouseMove((short)LOWORD(lParam), (short)HIWORD(lParam), wParam);

   case WM_LBUTTONDOWN:
      return ctrl->ButtonDown((short)LOWORD(lParam), (short)HIWORD(lParam), wParam, left);

   case WM_LBUTTONUP:
      return ctrl->ButtonUp((short)LOWORD(lParam), (short)HIWORD(lParam), wParam, left);

   case WM_LBUTTONDBLCLK:
      return ctrl->ButtonDblClk((short)LOWORD(lParam), (short)HIWORD(lParam), wParam, left);

   case WM_RBUTTONDOWN:
      return ctrl->ButtonDown((short)LOWORD(lParam), (short)HIWORD(lParam), wParam, right);

   case WM_RBUTTONUP:
      return ctrl->ButtonUp((short)LOWORD(lParam), (short)HIWORD(lParam), wParam, right);

   case WM_RBUTTONDBLCLK:
      return ctrl->ButtonDblClk((short)LOWORD(lParam), (short)HIWORD(lParam), wParam, right);

   case WM_MBUTTONDOWN:
      return ctrl->ButtonDown((short)LOWORD(lParam), (short)HIWORD(lParam), wParam, middle);

   case WM_MBUTTONUP:
      return ctrl->ButtonUp((short)LOWORD(lParam), (short)HIWORD(lParam), wParam, middle);

   case WM_MBUTTONDBLCLK:
      return ctrl->ButtonDblClk((short)LOWORD(lParam), (short)HIWORD(lParam), wParam, middle);

   case WM_ERASEBKGND:
      return ctrl->EraseBkgnd((HDC)wParam);

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


void ImgTag::run_event_handler (Value* event, int xPos, int yPos, int fwKeys, bool capture)
{
   if (capture) SetFocus(m_hWnd);
   HashTable* event_handlers= (HashTable*)parent_rollout->handlers->get(event);
   Value*   handler = NULL;
   if (event_handlers && (handler = event_handlers->get(name)) != NULL && (parent_rollout->flags & RO_CONTROLS_INSTALLED))
   {
      init_thread_locals();
      push_alloc_frame();
      push_plugin(parent_rollout->plugin);
      one_value_local(ehandler);
      vl.ehandler = handler->eval_no_wrapper();
      if (vl.ehandler->tag == class_tag(MAXScriptFunction) && ((MAXScriptFunction*)vl.ehandler)->parameter_count == 2)
      {
         Value** args;
         value_local_array(args, 2);
         args[0] = new Point2Value(xPos, yPos);
         args[1] = Integer::intern(fwKeys);
         RolloutControl::run_event_handler(parent_rollout, event, args, 2);
         pop_value_local_array(args);
      }
      else
         RolloutControl::run_event_handler(parent_rollout, event, NULL, 0);
      pop_value_locals();
      pop_plugin();
      pop_alloc_frame();
   }
   if (capture) SetCapture(m_hWnd);
}

// ============================================================================
LRESULT ImgTag::ButtonDown(int xPos, int yPos, int fwKeys, button which)
{
   switch (which)
   {
   case left:
      run_event_handler (n_mousedown, xPos, yPos, fwKeys);
      run_event_handler (n_lbuttondown, xPos, yPos, fwKeys);
      break;
   case middle:
      run_event_handler (n_mbuttondown, xPos, yPos, fwKeys);
      break;
   case right:
      run_event_handler (n_rightClick, xPos, yPos, fwKeys);
      run_event_handler (n_rbuttondown, xPos, yPos, fwKeys);
      break;
   }
   return TRUE;
}

// ============================================================================
LRESULT ImgTag::ButtonUp(int xPos, int yPos, int fwKeys, button which)
{
   switch (which)
   {
   case left:
      run_event_handler (n_mouseup, xPos, yPos, fwKeys);
      run_event_handler (n_click, xPos, yPos, fwKeys);
      run_event_handler(n_lbuttonup, xPos, yPos, fwKeys);
      break;
   case middle:
      run_event_handler(n_mbuttonup, xPos, yPos, fwKeys);
      break;
   case right:
      run_event_handler(n_rbuttonup, xPos, yPos, fwKeys);
      break;
   }
   return TRUE;
}

// ============================================================================
LRESULT ImgTag::ButtonDblClk(int xPos, int yPos, int fwKeys, button which)
{
   switch (which)
   {
   case left:
      run_event_handler(n_dblclick, xPos, yPos, fwKeys);
      run_event_handler(n_lbuttondblclk, xPos, yPos, fwKeys);
      break;
   case middle:
      run_event_handler(n_mbuttondblclk, xPos, yPos, fwKeys);
      break;
   case right:
      run_event_handler(n_rbuttondblclk, xPos, yPos, fwKeys);
      break;
   }
   return TRUE;
}

// ============================================================================
LRESULT ImgTag::MouseMove(int xPos, int yPos, int fwKeys)
{
   if(m_bMouseOver)
   {
      RECT rect;
      POINT pnt = {xPos,yPos};
      GetClientRect(m_hWnd, &rect);
      if(!PtInRect(&rect, pnt))
      {
         m_bMouseOver = false;
         ReleaseCapture();
         run_event_handler(n_mouseout, xPos, yPos, fwKeys, false);
      }
   }
   else
   {
      m_bMouseOver = true;
//    SetFocus(m_hWnd);
      run_event_handler(n_mouseover, xPos, yPos, fwKeys, false);
      SetCapture(m_hWnd);
   }

   return TRUE;
}

// ============================================================================
LRESULT ImgTag::EraseBkgnd(HDC hDC)
{
   return 1;
}

// ============================================================================
LRESULT ImgTag::Paint(HDC hDC)
{
   SetBkMode(hDC, TRANSPARENT);

   if(!m_maxBitMap || !m_hBitmap)
      return TRUE;

   MAXBitMap* mbm = (MAXBitMap*)m_maxBitMap;
   RECT rect;
   GetClientRect(m_hWnd, &rect);
   int width  = mbm->bi.Width();
   int height = mbm->bi.Height();

   HDC hMemDC = CreateCompatibleDC(hDC);
   SelectObject(hMemDC, m_hBitmap);

   BLENDFUNCTION bf;
   bf.BlendOp = AC_SRC_OVER;
   bf.BlendFlags = 0;
   bf.SourceConstantAlpha = (BYTE)((1.f - m_opacity) * 255.f);
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// quick patch to fix build breakage. Need to fix..
// bf.AlphaFormat = AC_SRC_NO_PREMULT_ALPHA|AC_DST_NO_ALPHA;
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   bf.AlphaFormat = 0x03;

   if(m_style == n_bmp_tile)
   {
      for(int x = 0; x < rect.right; x+=width)
         for(int y = 0; y < rect.bottom; y+=height) {
            if (bRunningW2K) {
               TransparentBlt_i(hDC, x, y, width, height, hMemDC, 0, 0, width, height, m_transparent);
               AlphaBlend_i(hDC, x, y, width, height, hMemDC, 0, 0, width, height, bf);
            }
            else
               BitBlt(hDC, x, y, width, height, hMemDC, 0, 0, SRCCOPY);
         }
   }
   else if(m_style == n_bmp_center)
   {
      if (bRunningW2K) {
         TransparentBlt_i(hDC, (rect.right-width)>>1, (rect.bottom-height)>>1,
            width, height, hMemDC, 0, 0, width, height, m_transparent);
         AlphaBlend_i(hDC, (rect.right-width)>>1, (rect.bottom-height)>>1,
            width, height, hMemDC, 0, 0, width, height, bf);
      }
      else
         BitBlt(hDC, (rect.right-width)>>1, (rect.bottom-height)>>1,
            width, height, hMemDC, 0, 0, SRCCOPY);
   }
   else /* n_stretch */
   {
      if (bRunningW2K) {
         TransparentBlt_i(hDC, 0, 0, rect.right, rect.bottom, hMemDC, 0, 0, width, height, m_transparent);
         AlphaBlend_i(hDC, 0, 0, rect.right, rect.bottom, hMemDC, 0, 0, width, height, bf);
      }
      else
         StretchBlt(hDC, 0, 0, rect.right, rect.bottom, hMemDC, 0, 0, width, height, SRCCOPY);
   }

   DeleteDC(hMemDC);

   return FALSE;
}

// ============================================================================
void ImgTag::Invalidate()
{
   if (m_hWnd == NULL) return;
   RECT rect;
   GetClientRect(m_hWnd, &rect);
   MapWindowPoints(m_hWnd, parent_rollout->page, (POINT*)&rect, 2);
   InvalidateRect(parent_rollout->page, &rect, TRUE);
   InvalidateRect(m_hWnd, NULL, TRUE);
}

// ============================================================================
int ImgTag::SetBitmap(Value *val)
{
   if(val == &undefined)
   {
      if(m_hBitmap) DeleteObject(m_hBitmap);
      m_hBitmap = NULL;
      m_maxBitMap = NULL;
   }
   else
   {
      HWND hWnd = MAXScript_interface->GetMAXHWnd();

      MAXBitMap *mbm = (MAXBitMap*)val;
      type_check(mbm, MAXBitMap, _T("set .bitmap"));
      m_maxBitMap = val;

      HDC hDC = GetDC(hWnd);
      PBITMAPINFO bmi = mbm->bm->ToDib(32);
      if(m_hBitmap) DeleteObject(m_hBitmap);
      m_hBitmap = CreateDIBitmap(hDC, &bmi->bmiHeader, CBM_INIT, bmi->bmiColors, bmi, DIB_RGB_COLORS);
      LocalFree(bmi);
      ReleaseDC(hWnd, hDC);
   }

   Invalidate();
   return 1;
}

// ============================================================================
visible_class_instance (ImgTag, "ImgTag")

void ImgTag::add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y)
{
   caption = caption->eval();

   TCHAR *text = caption->eval()->to_string();
   control_ID = next_id();
   parent_rollout = ro;

   Value *val = control_param(tooltip);
   if(val != &unsupplied)
      m_sToolTip = val->to_string();

   val = control_param(style);
   if(val != &unsupplied)
      m_style = val;

   val = control_param(bitmap);
   if(val != &unsupplied)
      SetBitmap(val);

   val = control_param(transparent);
   if(val != &unsupplied)
      m_transparent = val->to_colorref();
   else
      m_transparent = RGB(0,0,0);

   val = control_param(opacity);
   if(val != &unsupplied)
   {
      m_opacity = val->to_float();
      if(m_opacity < 0.f) m_opacity = 0.f;
      if(m_opacity > 1.f) m_opacity = 1.f;
   }

   layout_data pos;
   setup_layout(ro, &pos, current_y);

   if(m_maxBitMap)
   {
      MAXBitMap *mbm = (MAXBitMap*)m_maxBitMap;
      pos.width = mbm->bi.Width();
      pos.height = mbm->bi.Height();
   }

   process_layout_params(ro, &pos, current_y);

   m_hWnd = CreateWindow(
      IMGTAG_WINDOWCLASS,
      text,
      WS_VISIBLE | WS_CHILD | WS_GROUP,
      pos.left, pos.top, pos.width, pos.height,
      parent, (HMENU)control_ID, g_hInst, this);

   SendDlgItemMessage(parent, control_ID, WM_SETFONT, (WPARAM)ro->font, 0L);

   m_hToolTip = CreateWindow(
      TOOLTIPS_CLASS,
      TEXT(""), WS_POPUP,
      CW_USEDEFAULT, CW_USEDEFAULT,
      CW_USEDEFAULT, CW_USEDEFAULT,
      m_hWnd, (HMENU)NULL, g_hInst, NULL);

   SendMessage(m_hToolTip, TTM_ADDTOOL, 0, (LPARAM)GetToolInfo());
}


// ============================================================================
BOOL ImgTag::handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam)
{
   return FALSE;
}


// ============================================================================
Value* ImgTag::get_property(Value** arg_list, int count)
{
   Value* prop = arg_list[0];

   if(prop == n_width)
   {
      if(parent_rollout && parent_rollout->page)
      {
         HWND hWnd = GetDlgItem(parent_rollout->page, control_ID);
         RECT rect;
         GetWindowRect(hWnd, &rect);
         MapWindowPoints(NULL, parent_rollout->page, (LPPOINT)&rect, 2);
         return Integer::intern(rect.right-rect.left);
      }
      else return &undefined;
   }
   else if(prop == n_height)
   {
      if(parent_rollout && parent_rollout->page)
      {
         HWND hWnd = GetDlgItem(parent_rollout->page, control_ID);
         RECT rect;
         GetWindowRect(hWnd, &rect);
         MapWindowPoints(NULL, parent_rollout->page, (LPPOINT)&rect, 2);
         return Integer::intern(rect.bottom-rect.top);
      }
      else return &undefined;
   }
   else if(prop == n_tooltip)
   {
      if(parent_rollout && parent_rollout->page)
         return new String(m_sToolTip);
      else
         return &undefined;
   }
   else if(prop == n_style)
   {
      if(parent_rollout && parent_rollout->page)
         return m_style;
      else
         return &undefined;
   }
   else if(prop == n_bitmap)
   {
      if(parent_rollout && parent_rollout->page)
         return m_maxBitMap ? m_maxBitMap : &undefined;
      else
         return &undefined;
   }
   else if(prop == n_opacity)
   {
      if(parent_rollout && parent_rollout->page)
         return Float::intern(m_opacity);
      else
         return &undefined;
   }
   else if(prop == n_transparent)
   {
      if(parent_rollout && parent_rollout->page)
         return new ColorValue(m_transparent);
      else
         return &undefined;
   }

   return RolloutControl::get_property(arg_list, count);
}


// ============================================================================
Value* ImgTag::set_property(Value** arg_list, int count)
{
   Value* val = arg_list[0];
   Value* prop = arg_list[1];

   if(prop == n_bitmap)
   {
      if(parent_rollout && parent_rollout->page)
         SetBitmap(val);
   }
   else if(prop == n_width)
   {
      if(parent_rollout && parent_rollout->page)
      {
         int width = val->to_int();
         HWND  hWnd = GetDlgItem(parent_rollout->page, control_ID);
         RECT  rect;
         GetWindowRect(hWnd, &rect);
         MapWindowPoints(NULL, parent_rollout->page,  (LPPOINT)&rect, 2);
         SetWindowPos(hWnd, NULL, rect.left, rect.top, width, rect.bottom-rect.top, SWP_NOZORDER);
         SendMessage(m_hToolTip, TTM_SETTOOLINFO, 0, (LPARAM)GetToolInfo());
      }
   }
   else if(prop == n_height)
   {
      if(parent_rollout && parent_rollout->page)
      {
         int height = val->to_int();
         HWND  hWnd = GetDlgItem(parent_rollout->page, control_ID);
         RECT  rect;
         GetWindowRect(hWnd, &rect);
         MapWindowPoints(NULL, parent_rollout->page,  (LPPOINT)&rect, 2);
         SetWindowPos(hWnd, NULL, rect.left, rect.top, rect.right-rect.left, height, SWP_NOZORDER);
         SendMessage(m_hToolTip, TTM_SETTOOLINFO, 0, (LPARAM)GetToolInfo());
      }
   }
   else if(prop == n_tooltip)
   {
      if(parent_rollout && parent_rollout->page)
      {
         m_sToolTip = val->to_string();
         SendMessage(m_hToolTip, TTM_UPDATETIPTEXT, 0, (LPARAM)GetToolInfo());
      }
   }
   else if(prop == n_style)
   {
      if(parent_rollout && parent_rollout->page)
      {
         if(val == n_bmp_tile ||
            val == n_bmp_stretch ||
            val == n_bmp_center)
         {
            m_style = val;
            Invalidate();
         }
      }
   }
   else if(prop == n_opacity)
   {
      if(parent_rollout && parent_rollout->page)
      {
         m_opacity = val->to_float();
         if(m_opacity < 0.f) m_opacity = 0.f;
         if(m_opacity > 1.f) m_opacity = 1.f;
         Invalidate();
      }
   }
   else if(prop == n_transparent)
   {
      if(parent_rollout && parent_rollout->page)
      {
         m_transparent = val->to_colorref();
         Invalidate();
      }
   }
   else if (prop == n_text || prop == n_caption) // not displayed
   {
      TCHAR *text = val->to_string(); // will throw error if not convertable
      caption = val->get_heap_ptr();
   }
   else
      return RolloutControl::set_property(arg_list, count);

   return val;
}


// ============================================================================
void ImgTag::set_enable()
{
   if(parent_rollout != NULL && parent_rollout->page != NULL)
   {
      EnableWindow(m_hWnd, enabled);
      InvalidateRect(m_hWnd, NULL, TRUE);
   }
}


// ============================================================================
void ImgTagInit()
{
   static BOOL registered = FALSE;
   if(!registered)
   {
      WNDCLASSEX wcex;
      wcex.cbSize        = sizeof(WNDCLASSEX);
      wcex.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
      wcex.lpfnWndProc   = ImgTag::WndProc;
      wcex.cbClsExtra    = 0;
      wcex.cbWndExtra    = 0;
      wcex.hInstance     = g_hInst;
      wcex.hIcon         = NULL;
      wcex.hIconSm       = NULL;
      wcex.hCursor       = LoadCursor(0, MAKEINTRESOURCE(IDC_ARROW));
      wcex.hbrBackground = NULL;
      wcex.lpszMenuName  = NULL;
      wcex.lpszClassName  = IMGTAG_WINDOWCLASS;

      if(!RegisterClassEx(&wcex))
         return;
      registered = TRUE;
   }

   install_rollout_control(Name::intern("ImgTag"), ImgTag::create);
}

