/*===========================================================================*\
 |  Ishani's stuff for MAX Script R3
 |
 |  FILE:   ui-progbar.cpp
 |       UI Control : Progress Bar 
 | 
 |  AUTH:   Harry Denholm
 |       Copyright(c) KINETIX 1998
 |       All Rights Reserved.
 |
 |  HIST:   Started 7-7-98
 | 
\*===========================================================================*/
 
/*========================[ PROGRESS BAR CONTROL ]===========================*\
 |
 | use:
 |    progressBar [name] [title] <width:100> <height:20> <value:0> <color:[30,10,190]>
 |
 |
 | SETTINGS:
 |    value          -->      percentage complete (0-100)%
 |    color          -->      color of bar
 |    enabled        -->         active control on/off
 |
 | EVENTS:
 |    clicked <pos>     -->         called when user clicks on control. 
 |                               'pos' is percentage  value at 
 |                               the clicked point
 |
\*===========================================================================*/

//#include "pch.h"
#include "MAXScrpt.h"
#include "MAXObj.h"
#include "Numbers.h"
#include "rollouts.h"
#include "ColorVal.h"
#include "iColorMan.h"
#include "parser.h"

extern HINSTANCE g_hInst;

#include "resource.h"

#ifdef ScripterExport
   #undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "i3.h"
#include "MXSAgni.h"

#include "defextfn.h"
#  include "namedefs.h"

#include "definsfn.h"

#include "3dsmaxport.h"

// Change from an AColor -> COLORREF
COLORREF convertFrom(AColor in)
{
 return RGB(FLto255(in.r),FLto255(in.g), FLto255(in.b)); 
}

#define PBAR_WINDOWCLASS   _T("PBAR_WINDOWCLASS")

// Horizontal/Vertical type defs
#define TYPE_HORIZ      0
#define TYPE_VERT    1

/* -------------------- ish3 : ProgressBar  ------------------- */
visible_class (ish3_ProgressBar)

class ish3_ProgressBar : public RolloutControl
{
protected:
   int value;
   COLORREF colorPro;
   int type;
   HWND   progbar;
   
public:
   static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
      { return new ish3_ProgressBar (name, caption, keyparms, keyparm_count); }

   ish3_ProgressBar(Value* name, Value* caption, Value** keyparms, int keyparm_count);
            classof_methods (ish3_ProgressBar, RolloutControl);
   void     collect() { delete this; }
   void     sprin1(CharStream* s) { s->printf(_T("ProgressBar:%s"), name->to_string()); }

   void     add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y);
   LPCTSTR     get_control_class() { return PBAR_WINDOWCLASS; }
// void     compute_layout(Rollout *ro, layout_data* pos) { pos->width = 120; pos->height = 20; }

   BOOL     handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);

   Value*      get_property(Value** arg_list, int count);
   Value*      set_property(Value** arg_list, int count);
   void     set_enable();
};

            // Constructor
ish3_ProgressBar::ish3_ProgressBar(Value* name, Value* caption, Value** keyparms, int keyparm_count)
   : RolloutControl(name, caption, keyparms, keyparm_count)  
{ 
   tag = class_tag(ish3_ProgressBar);
   progbar = NULL;
}

LRESULT CALLBACK ish3_ProgressBar::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
   // Get back at our stored class pointer
   ish3_ProgressBar *UGC = DLGetWindowLongPtr<ish3_ProgressBar*>(hWnd);
   if(UGC == NULL && message != WM_CREATE)
      return DefWindowProc(hWnd, message, wParam, lParam);

   switch ( message ) {
   
      case WM_CREATE:
         {
            LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
            UGC = (ish3_ProgressBar*)lpcs->lpCreateParams;
            DLSetWindowLongPtr(hWnd, UGC);
            break;
         }

      case WM_MOUSEMOVE:
         {
         if(wParam && MK_LBUTTON)
         {
            init_thread_locals();
            push_alloc_frame();
            one_value_local(arg);
            // work out a positional click value and fire a 'clicked' event
            int xPos = LOWORD(lParam);
            int yPos = HIWORD(lParam);
            Rect rect;
            GetClientRect(hWnd,&rect);
            float fDelta;

            if(UGC->type==TYPE_HORIZ)
            {
               int rWide = (rect.right-1)-(rect.left+2);

               float Delta = 100.0f/rWide;
               fDelta = Delta*xPos;
            }
            if(UGC->type==TYPE_VERT)
            {
               int rHigh = (rect.bottom)-(rect.top);

               float Delta = 100.0f/rHigh;
               fDelta = Delta*(rHigh-yPos);
            }
            
            vl.arg = Integer::intern((int)fDelta);
            UGC->run_event_handler(UGC->parent_rollout, n_clicked, &vl.arg, 1);
            pop_value_locals();
            pop_alloc_frame();
         }

         if(UGC->enabled)
            SetCursor(LoadCursor(NULL, IDC_ARROW));
         }
      break;

      case WM_LBUTTONDOWN:
         {
            init_thread_locals();
            push_alloc_frame();
            one_value_local(arg);
            // ditto - just wanted to handle first time clicks
            int xPos = LOWORD(lParam);
            int yPos = HIWORD(lParam);
            Rect rect;
            GetClientRect(hWnd,&rect);
            float fDelta;

            if(UGC->type==TYPE_HORIZ)
            {
               int rWide = (rect.right-1)-(rect.left+2);

               float Delta = 100.0f/rWide;
               fDelta = Delta*xPos;
            }
            if(UGC->type==TYPE_VERT)
            {
               int rHigh = (rect.bottom)-(rect.top);

               float Delta = 100.0f/rHigh;
               fDelta = Delta*(rHigh-yPos);
            }
            
            vl.arg = Integer::intern((int)fDelta);
            UGC->run_event_handler(UGC->parent_rollout, n_clicked, &vl.arg, 1);
            pop_value_locals();
            pop_alloc_frame();
         }
         break;

      case WM_PAINT:
         {
            // begin paint cycle
            PAINTSTRUCT ps;
            HDC hdc;
            hdc = BeginPaint( hWnd, &ps );

            Rect rect;
            GetClientRect(hWnd,&rect);


            // Setup pens, brushes and colours
            SetBkColor( hdc, GetCustSysColor( COLOR_BTNFACE ) );
            SetBkMode( hdc, TRANSPARENT );   
            HPEN hLight = CreatePen( PS_SOLID, 0, GetCustSysColor( COLOR_BTNHIGHLIGHT ) );
            HPEN hBlack = (HPEN)GetStockObject(BLACK_PEN);
            HPEN hDark  = CreatePen( PS_SOLID, 0, GetCustSysColor( COLOR_BTNSHADOW ) );   

            // setup the progress bar colour, taking enabled flag into account
            HBRUSH hProgress;
            if(UGC->enabled)
             hProgress = CreateSolidBrush( UGC->colorPro );
            else hProgress = CreatePatternBrush( LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_DISABLED)) );

            HPEN hPP;
            if(UGC->enabled)
               hPP = CreatePen( PS_SOLID, 0, UGC->colorPro );
            else hPP = CreatePen( PS_SOLID, 0, GetCustSysColor( COLOR_BTNFACE ) );

            // the 'blanker' definition
            HBRUSH hProgressTerm = CreateSolidBrush( GetCustSysColor( COLOR_BTNFACE ) );
            HPEN hPT = CreatePen( PS_SOLID, 0, GetCustSysColor( COLOR_BTNFACE ) );


            // Do some 3D control drawing (frame)
            SelectObject(hdc, hDark);

            MoveToEx(hdc, rect.right-1, 0, NULL);
            LineTo  (hdc, 0, 0);
            LineTo  (hdc, 0, rect.bottom-1);

            SelectObject(hdc, hBlack);

            MoveToEx(hdc, rect.right-2, 1, NULL);
            LineTo  (hdc, 1, 1);
            LineTo  (hdc, 1, rect.bottom-2);

            SelectObject(hdc, hLight);

            MoveToEx(hdc, 0, rect.bottom-1, NULL);
            LineTo  (hdc, rect.right-1, rect.bottom-1);
            LineTo  (hdc, rect.right-1, 0 );

      // HORIZONTAL DRAW
      if(UGC->type==TYPE_HORIZ)
      {

            // prepare progress bar
            float delta = (float)((rect.right-rect.left)-1)/100.0f;
            int amt     = UGC->value;
            int draw = (int)(amt*delta);

            // handle extremities
            if(amt==100)   draw = ((rect.right-rect.left)-1);
            if(amt==0)     draw = rect.left+1;
            if(draw<=2) draw=2;


            // blit out unused to background
            if(amt!=100)
            {

               HGDIOBJ hOldPen = SelectObject(hdc, hPT);
               HGDIOBJ hOldBrush = SelectObject(hdc, hProgressTerm);
               Rectangle(hdc,draw,2,rect.right-1,rect.bottom-2);
			   SelectObject(hdc, hOldPen);
			   SelectObject(hdc, hOldBrush);
          }

            // draw progress bar
            if(amt>0)
            {
               HGDIOBJ hOldPen = SelectObject(hdc, hPP);
               HGDIOBJ hOldBrush = SelectObject(hdc, hProgress);
               Rectangle(hdc,2,2,draw,rect.bottom-2);
			   SelectObject(hdc, hOldPen);
			   SelectObject(hdc, hOldBrush);
            }
      }
      // VERTICAL DRAW
      if(UGC->type==TYPE_VERT)
      {

            // prepare progress bar
            float delta = (float)((rect.bottom-rect.top)-4)/100.0f;
            int amt     = UGC->value;
            int draw = (int)(amt*delta);

            // handle extremities
            if(amt==100)   draw = ((rect.bottom-rect.top)-4);
            if(amt==0)     draw = 0;//rect.bottom-2;

            
            // blit out unused to background
            if(amt!=100)
            {
               HGDIOBJ hOldPen = SelectObject(hdc, hPT);
               HGDIOBJ hOldBrush = SelectObject(hdc, hProgressTerm);
               Rectangle(hdc,2,rect.top+2,rect.right-2,(rect.bottom-2)-draw);
			   SelectObject(hdc, hOldPen);
			   SelectObject(hdc, hOldBrush);
            }

            // draw progress bar
            if(amt>0)
            {
               HGDIOBJ hOldPen = SelectObject(hdc, hPP);
               HGDIOBJ hOldBrush = SelectObject(hdc, hProgress);
               Rectangle(hdc,2,rect.bottom-2,rect.right-2,(rect.bottom-2)-draw);
			   SelectObject(hdc, hOldPen);
			   SelectObject(hdc, hOldBrush);
            }
      }

            DeleteObject(hLight);
            DeleteObject(hDark);
            DeleteObject(hBlack);
            DeleteObject(hPP);
            DeleteObject(hPT);
            DeleteObject(hProgress);
            DeleteObject(hProgressTerm);

            // end paint cycle
            EndPaint( hWnd, &ps );
            return 0;
         }

   }

   return TRUE;
}

/* -------------------- ish3_ProgressBar  ------------------- */

visible_class_instance (ish3_ProgressBar, "progressBar")

void
ish3_ProgressBar::add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y)
{
   caption = caption->eval();

   TCHAR*   text = caption->eval()->to_string();

   // We'll hang onto these...
   parent_rollout = ro;
   control_ID = next_id();

   // Load in some values from the user, if they were given...
   Value* Vtmp = control_param(value);
   if(Vtmp==&unsupplied) value = 0;
   else
      value = Vtmp->to_int();

   // Check orientation parameter
   type = TYPE_HORIZ;
   Value* Ttmp = control_param(orient);
   if(Ttmp!=&unsupplied&&Ttmp==n_vertical) type = TYPE_VERT;

   // Get the colour of the progress slider
   Value* Ctmp = control_param(color);
   if(Ctmp==&unsupplied) colorPro = RGB(30,10,190);
   else
      colorPro = convertFrom(Ctmp->to_acolor());

   // Pass the inpho back to MXS to 
   // let it calculate final position
   layout_data pos;
   compute_layout(ro, &pos, current_y);

   progbar = CreateWindow(
                     PBAR_WINDOWCLASS,
                     text,
                     WS_VISIBLE | WS_CHILD | WS_GROUP,
                     pos.left, pos.top, pos.width, pos.height,    
                     parent, (HMENU)control_ID, g_hInst, this);
   DWORD err = GetLastError();

}

BOOL
ish3_ProgressBar::handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam)
{
   // All messages processing is done in our dlg proc
   return FALSE;
}

Value*
ish3_ProgressBar::get_property(Value** arg_list, int count)
{
   Value* prop = arg_list[0];

   if (prop == n_value)
      return Integer::intern((int)value) ;
   else if (prop == n_color)
      return new ColorValue((COLORREF)colorPro) ;
   else if (prop == n_enabled)
      return enabled ? &true_value : &false_value;
   else if (prop == n_orient)
      return type ? n_vertical : n_horizontal;
   else if (prop == n_clicked)
      return get_wrapped_event_handler(n_clicked);
   else
      return RolloutControl::get_property(arg_list, count);
}

Value*
ish3_ProgressBar::set_property(Value** arg_list, int count)
{
   Value* val = arg_list[0];
   Value* prop = arg_list[1];

   if (prop == n_value)
   {
      if (parent_rollout != NULL && parent_rollout->page != NULL)
      {
         value = val->to_int();
         if(value>100) value=100;
         if(value<0) value=0;

         // rebuild the dialog item
         // have to do this to update in loops on a page
         InvalidateRect(progbar,NULL,FALSE);
            SendMessage(progbar, WM_PAINT, 0, 0);
      }
   }
   else if (prop == n_color)
   {
      if (parent_rollout != NULL && parent_rollout->page != NULL)
      {
         colorPro = convertFrom(val->to_acolor());

         InvalidateRect(progbar,NULL,FALSE);
            SendMessage(progbar, WM_PAINT, 0, 0);
      }
   }
   else if (prop == n_orient)
   {
      if (parent_rollout != NULL && parent_rollout->page != NULL)
      {
         if(val==n_horizontal) type = TYPE_HORIZ;
         if(val==n_vertical) type = TYPE_VERT;

         InvalidateRect(progbar,NULL,FALSE);
            SendMessage(progbar, WM_PAINT, 0, 0);
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

void
ish3_ProgressBar::set_enable()
{
   // Enable/disable the window item and refresh 
   if (progbar)
   {
      EnableWindow(progbar, enabled);
      InvalidateRect(progbar,NULL,FALSE);
         SendMessage(progbar, WM_PAINT, 0, 0);
   }

}


// ============================================================================
void ProgressBarInit()
{
   static BOOL registered = FALSE;
   if(!registered)
   {
      // Draggable Progress Bar
      // PBAR_WINDOWCLASS
      WNDCLASSEX wcex;
      wcex.style         = CS_HREDRAW | CS_VREDRAW;
      wcex.hInstance     = hInstance;
      wcex.hIcon         = NULL;
      wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
      wcex.hbrBackground = GetCustSysColorBrush(COLOR_WINDOW);
      wcex.lpszMenuName  = NULL;
      wcex.cbClsExtra    = 0;
      wcex.cbWndExtra    = 0;
      wcex.lpfnWndProc   = ish3_ProgressBar::WndProc;
      wcex.lpszClassName = PBAR_WINDOWCLASS;
      wcex.cbSize        = sizeof(WNDCLASSEX);
      wcex.hIconSm       = NULL;

      if(!RegisterClassEx(&wcex))
         return;
      registered = TRUE;
   }

   install_rollout_control(Name::intern("progressBar"), ish3_ProgressBar::create);
}




