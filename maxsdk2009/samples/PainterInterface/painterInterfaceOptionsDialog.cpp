

#include "painterInterface.h"
#include "IpainterInterface.h"

#include "3dsmaxport.h"

static HIMAGELIST hParamImages = NULL;


static INT_PTR CALLBACK PainterOptionFloaterDlgProc(
      HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
   {



   PainterInterface *painter = DLGetWindowLongPtr<PainterInterface*>(hWnd);
   switch (msg) {
      case WM_INITDIALOG: {
         painter = (PainterInterface*)lParam;
         DLSetWindowLongPtr(hWnd, lParam);
         
         

         painter->InitDialog(hWnd);

         break;
         }


      
      case WM_DESTROY: 
         {
         painter->BringDownOptions();

         ReleaseISpinner(painter->spinMarker);
         painter->spinMarker = NULL;
         ReleaseISpinner(painter->spinNormalScale);
         painter->spinNormalScale = NULL;


         ReleaseISpinner(painter->spinTreeDepth);
         painter->spinTreeDepth = NULL;

         ReleaseISpinner(painter->spinLagRate);
         painter->spinLagRate = NULL;

         ReleaseISpinner(painter->spinMinStr);
         painter->spinMinStr = NULL;
         ReleaseISpinner(painter->spinMaxStr);
         painter->spinMaxStr = NULL;

         ReleaseISpinner(painter->spinMinSize);
         painter->spinMinSize = NULL;
         ReleaseISpinner(painter->spinMaxSize);
         painter->spinMaxSize = NULL;

         ReleaseISpinner(painter->spinMirrorOffset);
         painter->spinMirrorOffset = NULL;
         ReleaseISpinner(painter->spinMirrorGizmoSize);
         painter->spinMirrorGizmoSize = NULL;


         
         break;
         }
      case WM_CUSTEDIT_ENTER:
         {
         if ((LOWORD(wParam) == IDC_TREEDEPTH) || (LOWORD(wParam) == IDC_TREEDEPTH_SPIN))
            painter->SetTreeDepth(painter->spinTreeDepth->GetIVal());
         else if ((LOWORD(wParam) == IDC_MINSTR) || (LOWORD(wParam) == IDC_MINSTR_SPIN))
            painter->SetMinStr(painter->spinMinStr->GetFVal());
         else if ((LOWORD(wParam) == IDC_MAXSTR) || (LOWORD(wParam) == IDC_MAXSTR_SPIN))
            painter->SetMaxStr(painter->spinMaxStr->GetFVal());
         else if ((LOWORD(wParam) == IDC_MINSIZE) || (LOWORD(wParam) == IDC_MINSIZE_SPIN))
            painter->SetMinSize(painter->spinMinSize->GetFVal());
         else if ((LOWORD(wParam) == IDC_MAXSIZE) || (LOWORD(wParam) == IDC_MAXSIZE_SPIN))
            painter->SetMaxSize(painter->spinMaxSize->GetFVal());
         else if ((LOWORD(wParam) == IDC_MIRROROFFSET) || (LOWORD(wParam) == IDC_MIRROROFFSET_SPIN))
            painter->SetMirrorOffset(painter->spinMirrorOffset->GetFVal());
         else if ((LOWORD(wParam) == IDC_GIZMOSIZE) || (LOWORD(wParam) == IDC_GIZMOSIZE_SPIN))
            painter->SetMirrorGizmoSize(painter->spinMirrorGizmoSize->GetFVal());
         else if ((LOWORD(wParam) == IDC_LAGRATE) || (LOWORD(wParam) == IDC_LAGRATE_SPIN))
            painter->SetLagRate(painter->spinLagRate->GetIVal());

         else if ((LOWORD(wParam) == IDC_NORMALSCALE) || (LOWORD(wParam) == IDC_NORMALSCALE_SPIN))
            painter->SetNormalScale(painter->spinNormalScale->GetFVal());
         else if ((LOWORD(wParam) == IDC_MARKER) || (LOWORD(wParam) == IDC_MARKER_SPIN))
            painter->SetMarker(painter->spinMarker->GetFVal());

         break;
         }
      case CC_SPINNER_CHANGE:
         {
         if ((LOWORD(wParam) == IDC_MIRROROFFSET) || (LOWORD(wParam) == IDC_MIRROROFFSET_SPIN))
            painter->SetMirrorOffset(painter->spinMirrorOffset->GetFVal());
         else if ((LOWORD(wParam) == IDC_GIZMOSIZE) || (LOWORD(wParam) == IDC_GIZMOSIZE_SPIN))
            painter->SetMirrorGizmoSize(painter->spinMirrorGizmoSize->GetFVal());
         break;
         }
      case CC_SPINNER_BUTTONUP:
         {
         if (HIWORD(wParam))
            {
            if ((LOWORD(wParam) == IDC_TREEDEPTH) || (LOWORD(wParam) == IDC_TREEDEPTH_SPIN))
               painter->SetTreeDepth(painter->spinTreeDepth->GetIVal());
            else if ((LOWORD(wParam) == IDC_MINSTR) || (LOWORD(wParam) == IDC_MINSTR_SPIN))
               painter->SetMinStr(painter->spinMinStr->GetFVal());
            else if ((LOWORD(wParam) == IDC_MAXSTR) || (LOWORD(wParam) == IDC_MAXSTR_SPIN))
               painter->SetMaxStr(painter->spinMaxStr->GetFVal());
            else if ((LOWORD(wParam) == IDC_MINSIZE) || (LOWORD(wParam) == IDC_MINSIZE_SPIN))
               painter->SetMinSize(painter->spinMinSize->GetFVal());
            else if ((LOWORD(wParam) == IDC_MAXSIZE) || (LOWORD(wParam) == IDC_MAXSIZE_SPIN))
               painter->SetMaxSize(painter->spinMaxSize->GetFVal());
            else if ((LOWORD(wParam) == IDC_MIRROROFFSET) || (LOWORD(wParam) == IDC_MIRROROFFSET_SPIN))
               painter->SetMirrorOffset(painter->spinMirrorOffset->GetFVal());
            else if ((LOWORD(wParam) == IDC_GIZMOSIZE) || (LOWORD(wParam) == IDC_GIZMOSIZE_SPIN))
               painter->SetMirrorGizmoSize(painter->spinMirrorGizmoSize->GetFVal());
            else if ((LOWORD(wParam) == IDC_LAGRATE) || (LOWORD(wParam) == IDC_LAGRATE_SPIN))
               painter->SetLagRate(painter->spinLagRate->GetIVal());

            else if ((LOWORD(wParam) == IDC_NORMALSCALE) || (LOWORD(wParam) == IDC_NORMALSCALE_SPIN))
               painter->SetNormalScale(painter->spinNormalScale->GetFVal());
            else if ((LOWORD(wParam) == IDC_MARKER) || (LOWORD(wParam) == IDC_MARKER_SPIN))
               painter->SetMarker(painter->spinMarker->GetFVal());


            }
         break;
         }
      case WM_CC_SEL_CURVEPT: //Sent when a point is selected or deselected.
      case WM_CC_CHANGE_CURVEPT: //Sent when a point is changed.
      case WM_CC_CHANGE_CURVETANGENT: //Sent when a point's in or out tangent is changed.
      case WM_CC_DEL_CURVEPT: //Sent when a point is deleted.
      case WM_CC_INSERT_CURVEPT: //This option is available in release 4.0 and later only.
         {
         painter->UpdateDependents();
         return FALSE;
         }

      case WM_COMMAND:
         {
         
         switch (LOWORD(wParam)) 
            {

            case IDOK:
            case IDCANCEL:
               DestroyWindow(hWnd);
               break;
            default:
               painter->WMCommand(LOWORD(wParam), HIWORD(wParam));
               break;
            }

         break;
         }

      default:
         return FALSE;
      }

   return TRUE;
   }




BOOL  PainterInterface::BringDownOptions()
   {

   optionsWindowPos.length = sizeof(WINDOWPLACEMENT); 
   GetWindowPlacement(painterOptionsWindow,&optionsWindowPos);
   SaveWindowPos();

   ReleaseICustToolbar(iFalloffToolBar);
   iFalloffToolBar = NULL;
   GetCOREInterface()->UnRegisterDlgWnd(painterOptionsWindow);
   painterOptionsWindow = NULL;
   return TRUE;
   }

BOOL  PainterInterface::BringUpOptions()
   {
   if (painterOptionsWindow == NULL)
      {
      painterOptionsWindow = CreateDialogParam(hInstance,
                 MAKEINTRESOURCE(IDD_PAINTEROPTIONS_FLOATER),
                 GetCOREInterface()->GetMAXHWnd(),
                 PainterOptionFloaterDlgProc,
                 (LPARAM)this);
      GetCOREInterface()->RegisterDlgWnd(painterOptionsWindow);



      }
   else
      {
      DestroyWindow(painterOptionsWindow);
      painterOptionsWindow = NULL;
      }


   return TRUE;
   }

void  PainterInterface::CloseOptionsDialog()
{
   if (painterOptionsWindow != NULL)
   {
      DestroyWindow(painterOptionsWindow);
      painterOptionsWindow = NULL;
   }
}

void PainterInterface::InitDialog(HWND hWnd)
   {
   painterOptionsWindow = hWnd;

   if ((!firstTime) && (optionsWindowPos.rcNormalPosition.right != 0) && (optionsWindowPos.rcNormalPosition.bottom != 0))
   {
      optionsWindowPos.length = sizeof(WINDOWPLACEMENT); 
      SetWindowPlacement(painterOptionsWindow,&optionsWindowPos);    
   }
   firstTime = FALSE;

   CreateCurveControl();
   ReferenceTarget *ref;
   pblock->GetValue(painterinterface_falloffgraph,0,ref, FOREVER);
   if (ref)
      {
      curveCtl = (ICurveCtl *) ref;

//Initialize Curve Control
      curveCtl->SetCustomParentWnd(GetDlgItem(hWnd, IDC_CURVE));
      curveCtl->SetMessageSink(hWnd);     
      pCurve = curveCtl->GetControlCurve(0);
      }

   pblock->GetValue(painterinterface_predefinedstrgraph,0,ref, FOREVER);
   if (ref)
      {
      curvePredefinedStrCtl = (ICurveCtl *) ref;
      curvePredefinedStrCtl->SetMessageSink(hWnd);
      pPredefinedStrCurve = curvePredefinedStrCtl->GetControlCurve(0);;
      }

   pblock->GetValue(painterinterface_predefinedsizegraph,0,ref, FOREVER);
   if (ref)
      {
      curvePredefinedSizeCtl = (ICurveCtl *) ref;
      curvePredefinedSizeCtl->SetMessageSink(hWnd);
      pPredefinedSizeCurve = curvePredefinedSizeCtl->GetControlCurve(0);;
      }

   spinTreeDepth = SetupIntSpinner(painterOptionsWindow,IDC_TREEDEPTH_SPIN,IDC_TREEDEPTH,2,10,GetTreeDepth());

   spinLagRate = SetupIntSpinner(painterOptionsWindow,IDC_LAGRATE_SPIN,IDC_LAGRATE,0,100,GetLagRate());

   spinMinStr = SetupFloatSpinner(painterOptionsWindow,IDC_MINSTR_SPIN,IDC_MINSTR,-100000.0f,100000.0f,GetMinStr());
   spinMaxStr = SetupFloatSpinner(painterOptionsWindow,IDC_MAXSTR_SPIN,IDC_MAXSTR,-100000.0f,100000.0f,GetMaxStr());

   spinMinSize = SetupFloatSpinner(painterOptionsWindow,IDC_MINSIZE_SPIN,IDC_MINSIZE,0.0f,100000.0f,GetMinSize());
   spinMaxSize = SetupFloatSpinner(painterOptionsWindow,IDC_MAXSIZE_SPIN,IDC_MAXSIZE,0.0f,100000.0f,GetMaxSize());
   spinMinSize->LinkToEdit(GetDlgItem(painterOptionsWindow,IDC_MINSIZE), EDITTYPE_UNIVERSE);
   spinMaxSize->LinkToEdit(GetDlgItem(painterOptionsWindow,IDC_MAXSIZE), EDITTYPE_UNIVERSE);
   
   spinMirrorOffset = SetupFloatSpinner(painterOptionsWindow,IDC_MIRROROFFSET_SPIN,IDC_MIRROROFFSET,-100000.0f,100000.0f,GetMirrorOffset());
   spinMirrorGizmoSize = SetupFloatSpinner(painterOptionsWindow,IDC_GIZMOSIZE_SPIN,IDC_GIZMOSIZE,0.0f,100000.0f,GetMirrorGizmoSize());
   
   spinMirrorOffset->LinkToEdit(GetDlgItem(painterOptionsWindow,IDC_MIRROROFFSET), EDITTYPE_UNIVERSE);
   spinMirrorGizmoSize->LinkToEdit(GetDlgItem(painterOptionsWindow,IDC_GIZMOSIZE), EDITTYPE_POS_UNIVERSE);

   spinNormalScale = SetupFloatSpinner(painterOptionsWindow,IDC_NORMALSCALE_SPIN,IDC_NORMALSCALE,0.0f,100000.0f,GetNormalScale());
   spinMarker = SetupFloatSpinner(painterOptionsWindow,IDC_MARKER_SPIN,IDC_MARKER,0.0f,100000.0f,GetMarker());


   if (!hParamImages) 
      {
      HBITMAP hBitmap, hMask; 
      hParamImages = ImageList_Create(16, 15, TRUE, 5, 0);
      hBitmap = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_PARAMS));
      hMask   = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_PARAMS_MASK));
      ImageList_Add(hParamImages,hBitmap,hMask);
      DeleteObject(hBitmap);
      DeleteObject(hMask);
      }


   iFalloffToolBar = GetICustToolbar(GetDlgItem(painterOptionsWindow,IDC_FALLOFF_TOOLBAR));
   iFalloffToolBar->SetBottomBorder(FALSE);  
   iFalloffToolBar->SetImage(hParamImages);

   iFalloffToolBar->AddTool(ToolButtonItem(CTB_PUSHBUTTON,  0, 0, 1, 1, 16, 15, 23, 22, IDC_LINEAR));
   iFalloffToolBar->AddTool(ToolButtonItem(CTB_PUSHBUTTON, 2, 2, 3, 3, 16, 15, 23, 22, IDC_SMOOTH));
   iFalloffToolBar->AddTool(ToolButtonItem(CTB_PUSHBUTTON, 4, 4, 5, 5, 16, 15, 23, 22,    IDC_SLOW));
   iFalloffToolBar->AddTool(ToolButtonItem(CTB_PUSHBUTTON, 6, 6, 7, 7, 16, 15, 23, 22, IDC_FAST));
   iFalloffToolBar->AddTool(ToolButtonItem(CTB_PUSHBUTTON, 8, 8, 9, 9, 16, 15, 23, 22, IDC_FLAT));


   UpdateControls();
   }

void PainterInterface::UpdateControls()
   {
   BOOL checkState;
   if (painterOptionsWindow==NULL) return;

   pblock->GetValue(painterinterface_drawring,0,checkState,FOREVER);
   if (checkState) CheckDlgButton(painterOptionsWindow, IDC_DRAWRING_CHECK, BST_CHECKED);
   else CheckDlgButton(painterOptionsWindow, IDC_DRAWRING_CHECK, BST_UNCHECKED);

   pblock->GetValue(painterinterface_drawnormal,0,checkState,FOREVER);
   if (checkState) CheckDlgButton(painterOptionsWindow, IDC_DRAWNORMAL_CHECK, BST_CHECKED);
   else CheckDlgButton(painterOptionsWindow, IDC_DRAWNORMAL_CHECK, BST_UNCHECKED);

   pblock->GetValue(painterinterface_drawtrace,0,checkState,FOREVER);
   if (checkState) CheckDlgButton(painterOptionsWindow, IDC_DRAWTRACE_CHECK, BST_CHECKED);
   else CheckDlgButton(painterOptionsWindow, IDC_DRAWTRACE_CHECK, BST_UNCHECKED);

   pblock->GetValue(painterinterface_updateonmouseup,0,checkState,FOREVER);
   if (checkState) CheckDlgButton(painterOptionsWindow, IDC_UPDATEONMOUSEUP_CHECK, BST_CHECKED);
   else CheckDlgButton(painterOptionsWindow, IDC_UPDATEONMOUSEUP_CHECK, BST_UNCHECKED);   

   pblock->GetValue(painterinterface_additivemode,0,checkState,FOREVER);
   if (checkState) CheckDlgButton(painterOptionsWindow, IDC_ADDITIVE_CHECK, BST_CHECKED);
   else CheckDlgButton(painterOptionsWindow, IDC_ADDITIVE_CHECK, BST_UNCHECKED); 
   if (curveCtl) {
      curveCtl->SetActive(TRUE);
      curveCtl->Redraw();
   }

//need to turn these off if no tablet
   pblock->GetValue(painterinterface_pressureenable,0,checkState,FOREVER);
   if (checkState) CheckDlgButton(painterOptionsWindow, IDC_PRESUREENABLE_CHECK, BST_CHECKED);
   else CheckDlgButton(painterOptionsWindow, IDC_PRESUREENABLE_CHECK, BST_UNCHECKED);  

   //load up pressure affects drop list
   HWND hPressureAffects = GetDlgItem(painterOptionsWindow,IDC_PRESSURE_AFFECTS_COMBO);
   SendMessage(hPressureAffects, CB_RESETCONTENT, 0, 0);
   SendMessage(hPressureAffects, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_NONE));
   SendMessage(hPressureAffects, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_STRENGTH));
   SendMessage(hPressureAffects, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_SIZE));
   SendMessage(hPressureAffects, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_BOTH));
   SendMessage(hPressureAffects, CB_SETCURSEL, (GetPressureAffects()), 0L);

// if (hWinTabDLL == NULL)
   if (0)
      {
      SetPressureEnable( FALSE);
      EnableWindow(hPressureAffects,FALSE);
      EnableWindow(GetDlgItem(painterOptionsWindow,IDC_PRESUREENABLE_CHECK), FALSE);
      }
   else 
      {
      EnableWindow(GetDlgItem(painterOptionsWindow,IDC_PRESUREENABLE_CHECK), TRUE);
      EnableWindow(hPressureAffects,TRUE);
      }


   checkState = GetPredefinedStrEnable();
   if (checkState) CheckDlgButton(painterOptionsWindow, IDC_PREDEFINEDSTR_CHECK, BST_CHECKED);
   else CheckDlgButton(painterOptionsWindow, IDC_PREDEFINEDSTR_CHECK, BST_UNCHECKED);  
   EnableWindow(GetDlgItem(painterOptionsWindow,IDC_PREDEFINEDSTR_BUTTON),checkState);

   checkState = GetPredefinedSizeEnable();
   if (checkState) CheckDlgButton(painterOptionsWindow, IDC_PREDEFINEDSIZE_CHECK, BST_CHECKED);
   else CheckDlgButton(painterOptionsWindow, IDC_PREDEFINEDSIZE_CHECK, BST_UNCHECKED);
   EnableWindow(GetDlgItem(painterOptionsWindow,IDC_PREDEFINEDSIZE_BUTTON),checkState);


   if (spinTreeDepth) spinTreeDepth->SetValue(GetTreeDepth(),FALSE);

   if (spinLagRate) spinLagRate->SetValue(GetLagRate(),FALSE);

   if (spinMinStr) spinMinStr->SetValue(GetMinStr(),FALSE);
   if (spinMaxStr) spinMaxStr->SetValue(GetMaxStr(),FALSE);

   if (spinMinSize) spinMinSize->SetValue(GetMinSize(),FALSE);
   if (spinMaxSize) spinMaxSize->SetValue(GetMaxSize(),FALSE);

   if (spinNormalScale) spinNormalScale->SetValue(GetNormalScale(),FALSE);
   if (spinMarker) spinMarker->SetValue(GetMarker(),FALSE);

//Mirror Options
   if (spinMirrorOffset) spinMirrorOffset->SetValue(GetMirrorOffset(),FALSE);
   if (spinMirrorGizmoSize) spinMirrorGizmoSize->SetValue(GetMirrorGizmoSize(),FALSE);
   
   if (GetMarkerEnable()) 
      {
      CheckDlgButton(painterOptionsWindow, IDC_MARKER_CHECK, BST_CHECKED);
      if (spinMarker) spinMarker->Enable(TRUE);
      }
   else 
      {
      CheckDlgButton(painterOptionsWindow, IDC_MARKER_CHECK, BST_UNCHECKED);
      if (spinMarker) spinMarker->Enable(FALSE);
      }

   if (GetMirrorEnable()) CheckDlgButton(painterOptionsWindow, IDC_MIRROR_CHECK, BST_CHECKED);
   else CheckDlgButton(painterOptionsWindow, IDC_MIRROR_CHECK, BST_UNCHECKED);

   HWND hMirrorAxis = GetDlgItem(painterOptionsWindow,IDC_MIRRORAXIS_COMBO);
   SendMessage(hMirrorAxis, CB_RESETCONTENT, 0, 0);
   SendMessage(hMirrorAxis, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_X));
   SendMessage(hMirrorAxis, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_Y));
   SendMessage(hMirrorAxis, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_Z));
   SendMessage(hMirrorAxis, CB_SETCURSEL, (GetMirrorAxis()), 0L);

   EnableWindow(hMirrorAxis,GetMirrorEnable());
   EnableWindow(GetDlgItem(painterOptionsWindow,IDC_MIRRORREPROJECT_CHECK),GetMirrorEnable());
   if (spinMirrorOffset) spinMirrorOffset->Enable(GetMirrorEnable());
   if (spinMirrorGizmoSize) spinMirrorGizmoSize->Enable(GetMirrorEnable());



   
   }


void PainterInterface::SetCurveShape(ICurve *curve, int type)
   {

   if (type == IDC_LINEAR)
      {
      CurvePoint pt;
      curve->SetNumPts(2);

      pt.flags = CURVEP_CORNER | CURVEP_BEZIER | CURVEP_ENDPOINT;
      pt.out.x = 0.33f;
      pt.out.y = -0.33f;
      pt.in.x = -0.33f;
      pt.in.y = -0.33f;

      pt.p.x = 0.0f;       
      pt.p.y = 1.0f;       
      curve->SetPoint(0,0,&pt);
      
      pt.p.x = 1.0f;       
      pt.p.y = 0.0f;       
      pt.in.x = -.33f;
      pt.in.y = 0.33f;
      pt.out.x = .33f;
      pt.out.y = 0.33f;

      curve->SetPoint(0,1,&pt);
      curve->SetLookupTableSize(1000);
      }

   else if (type == IDC_SMOOTH)
      {
      CurvePoint pt;
      curve->SetNumPts(2);

      pt.flags = CURVEP_CORNER | CURVEP_BEZIER | CURVEP_ENDPOINT;
      pt.out.x = 0.33f;
      pt.out.y = 0.0f;
      pt.in.x = -0.33f;
      pt.in.y = 0.0f;

      pt.p.x = 0.0f;       
      pt.p.y = 1.0f;       
      curve->SetPoint(0,0,&pt);
      
      pt.p.x = 1.0f;       
      pt.p.y = 0.0f;       
      pt.in.x = -.33f;
      pt.in.y = 0.0f;
      pt.out.x = .33f;
      pt.out.y = 0.0f;

      curve->SetPoint(0,1,&pt);
      curve->SetLookupTableSize(1000);
      }
   else if (type == IDC_SLOW)
      {
      CurvePoint pt;
      curve->SetNumPts(2);

      pt.flags = CURVEP_CORNER | CURVEP_BEZIER | CURVEP_ENDPOINT;
      pt.out.x = 0.33f;
      pt.out.y = 0.0f;
      pt.in.x = -0.33f;
      pt.in.y = 0.0f;

      pt.p.x = 0.0f;       
      pt.p.y = 1.0f;       
      curve->SetPoint(0,0,&pt);
      
      pt.p.x = 1.0f;       
      pt.p.y = 0.0f;       
      pt.in.x = -.0f;
      pt.in.y = 1.f;
      pt.out.x = .33f;
      pt.out.y = 0.0f;

      curve->SetPoint(0,1,&pt);
      curve->SetLookupTableSize(1000);
      }
   else if (type == IDC_FAST)
      {
      CurvePoint pt;
      curve->SetNumPts(2);

      pt.flags = CURVEP_CORNER | CURVEP_BEZIER | CURVEP_ENDPOINT;
      pt.out.x = 0.0f;
      pt.out.y = -1.f;
      pt.in.x = -0.33f;
      pt.in.y = 0.0f;

      pt.p.x = 0.0f;       
      pt.p.y = 1.0f;       
      curve->SetPoint(0,0,&pt);
      
      pt.p.x = 1.0f;       
      pt.p.y = 0.0f;       
      pt.in.x = -.33f;
      pt.in.y = 0.0f;
      pt.out.x = .33f;
      pt.out.y = 0.0f;

      curve->SetPoint(0,1,&pt);
      curve->SetLookupTableSize(1000);
      }
   else if (type == IDC_FLAT)
      {
      CurvePoint pt;
      curve->SetNumPts(2);

      pt.flags = CURVEP_CORNER | CURVEP_BEZIER | CURVEP_ENDPOINT;
      pt.out.x = 0.33f;
      pt.out.y = 0.0f;
      pt.in.x = -0.33f;
      pt.in.y = 0.0f;

      pt.p.x = 0.0f;       
      pt.p.y = 1.0f;       
      curve->SetPoint(0,0,&pt);
      
      pt.p.x = 1.0f;       
      pt.p.y = 1.0f;       
      pt.in.x = -.33f;
      pt.in.y = 0.0f;
      pt.out.x = .33f;
      pt.out.y = 0.0f;

      curve->SetPoint(0,1,&pt);
      curve->SetLookupTableSize(1000);
      }

   }


void PainterInterface::WMCommand(int attrib,int attribHigh)
   {
   BOOL checkState;
            

   switch (attrib)
      {
      case IDC_SMOOTH:
      case IDC_LINEAR:
      case IDC_FAST:
      case IDC_SLOW:
      case IDC_FLAT:
         {
         theHold.Begin();
         SetCurveShape(pCurve,attrib);
         theHold.Accept(GetString(IDS_CURVECHANGE));
         curveCtl->Redraw();
         }
      case IDC_MIRROR_CHECK:
         {
         checkState = IsDlgButtonChecked(painterOptionsWindow,IDC_MIRROR_CHECK);
         SetMirrorEnable(checkState);
         break;
         }

      case IDC_MIRRORAXIS_COMBO:
         {
         if (attribHigh == CBN_SELCHANGE)
            {
            HWND hMirrorAxis = GetDlgItem(painterOptionsWindow,IDC_MIRRORAXIS_COMBO);              
            SetMirrorAxis(SendMessage(hMirrorAxis, CB_GETCURSEL, 0, 0L));

            }
         break;
         }
      case IDC_PREDEFINEDSIZE_BUTTON:
         {
         if (curvePredefinedSizeCtl)
            {
            if (curvePredefinedSizeCtl->IsActive())
               curvePredefinedSizeCtl->SetActive(FALSE);
            else 
               {
               curvePredefinedSizeCtl->SetActive(TRUE);
               curvePredefinedSizeCtl->ZoomExtents();
               }
            }
         break;
         }
      case IDC_PREDEFINEDSTR_BUTTON:
         {
         if (curvePredefinedStrCtl)
            {
            if (curvePredefinedStrCtl->IsActive())
               curvePredefinedStrCtl->SetActive(FALSE);
            else 
               {
               curvePredefinedStrCtl->SetActive(TRUE);
               curvePredefinedStrCtl->ZoomExtents();
               }
            }
         break;
         }

      case IDC_PREDEFINEDSTR_CHECK:
         {
         checkState = IsDlgButtonChecked(painterOptionsWindow,IDC_PREDEFINEDSTR_CHECK);
         SetPredefinedStrEnable(checkState);
         break;
         }

      case IDC_PREDEFINEDSIZE_CHECK:
         {
         checkState = IsDlgButtonChecked(painterOptionsWindow,IDC_PREDEFINEDSIZE_CHECK);
         SetPredefinedSizeEnable(checkState);
         break;
         }

      case IDC_PRESSURE_AFFECTS_COMBO:
         {
         if (attribHigh == CBN_SELCHANGE)
            {
            HWND hPressureAffects = GetDlgItem(painterOptionsWindow,IDC_PRESSURE_AFFECTS_COMBO);               
            SetPressureAffects(SendMessage(hPressureAffects, CB_GETCURSEL, 0, 0L));
            }
         break;
         }
      case IDC_PRESUREENABLE_CHECK:
         {
         checkState = IsDlgButtonChecked(painterOptionsWindow,IDC_PRESUREENABLE_CHECK);
         SetPressureEnable(checkState);
         break;
         }
      case IDC_ADDITIVE_CHECK:
         {
         checkState = IsDlgButtonChecked(painterOptionsWindow,IDC_ADDITIVE_CHECK);
         SetAdditiveMode(checkState);
         break;
         }
      case IDC_UPDATEONMOUSEUP_CHECK:
         {
         checkState = IsDlgButtonChecked(painterOptionsWindow,IDC_UPDATEONMOUSEUP_CHECK);
         pblock->SetValue(painterinterface_updateonmouseup,0,checkState);
         break;
         }
      case IDC_DRAWRING_CHECK:
         {
         checkState = IsDlgButtonChecked(painterOptionsWindow,IDC_DRAWRING_CHECK);
         SetDrawRing(checkState);
//       pblock->SetValue(painterinterface_drawring,0,checkState);
         break;
         }
      case IDC_DRAWNORMAL_CHECK:
         {
         checkState = IsDlgButtonChecked(painterOptionsWindow,IDC_DRAWNORMAL_CHECK);
         SetDrawNormal(checkState);
//       pblock->SetValue(painterinterface_drawnormal,0,checkState);
         break;
         }
      case IDC_DRAWTRACE_CHECK:
         {
         checkState = IsDlgButtonChecked(painterOptionsWindow,IDC_DRAWTRACE_CHECK);
         SetDrawTrace(checkState);
//       pblock->SetValue(painterinterface_drawtrace,0,checkState);
         break;
         }
      case IDC_MARKER_CHECK:
         {
         checkState = IsDlgButtonChecked(painterOptionsWindow,IDC_MARKER_CHECK);
         SetMarkerEnable(checkState);
//       pblock->SetValue(painterinterface_drawtrace,0,checkState);
         break;
         }
      }
   }

#define PAINTERINTERFACECONFIGNAME _T("painterInterface.ini")
#define WSDSECTION    _T("Default State")
#define WINDOWPOSX1                 _T("WindposX1")
#define WINDOWPOSY1                 _T("WindposY1")
#define WINDOWPOSX2                 _T("WindposX2")
#define WINDOWPOSY2                 _T("WindposY2")

void PainterInterface::GetCfgFilename( TCHAR *filename ) 
   {
   _tcscpy(filename,TheManager->GetDir(APP_PLUGCFG_DIR));
   size_t len = _tcslen(filename);
   if (len) 
      {
      if (_tcscmp(&filename[len-1],_T("\\")))
       _tcscat(filename,_T("\\"));

      }
   _tcscat(filename,PAINTERINTERFACECONFIGNAME);
   }

void PainterInterface::SaveWindowPos()
{

   TCHAR filename[MAX_PATH];
   TSTR str;

   GetCfgFilename(filename);
   
   char* saved_lc = NULL;
   char* lc = ::setlocale(LC_NUMERIC, NULL); // query  
   if (lc)  
      saved_lc = ::strdup(lc);  
   lc = ::setlocale(LC_NUMERIC, "C");  
   
   
   str.printf("%d",optionsWindowPos.rcNormalPosition.left);
   WritePrivateProfileString(WSDSECTION,WINDOWPOSX1,str,filename);
   str.printf("%d",optionsWindowPos.rcNormalPosition.bottom);
   WritePrivateProfileString(WSDSECTION,WINDOWPOSY1,str,filename);
   str.printf("%d",optionsWindowPos.rcNormalPosition.right);
   WritePrivateProfileString(WSDSECTION,WINDOWPOSX2,str,filename);
   str.printf("%d",optionsWindowPos.rcNormalPosition.top);
   WritePrivateProfileString(WSDSECTION,WINDOWPOSY2,str,filename);

   if (saved_lc)  
   {  
      lc = ::setlocale(LC_NUMERIC, saved_lc);  
      free(saved_lc);  
      saved_lc = NULL;  
   }     
   
}
void PainterInterface::LoadWindowPos()
{
   TCHAR filename[MAX_PATH];
   GetCfgFilename(filename);
   TCHAR str[MAX_PATH];
   TSTR def("DISCARD");


   char* saved_lc = NULL;
   char* lc = ::setlocale(LC_NUMERIC, NULL); // query  
   if (lc)  
      saved_lc = ::strdup(lc);  
   lc = ::setlocale(LC_NUMERIC, "C");  
   

   int full = 0;
   int res = GetPrivateProfileString(WSDSECTION,WINDOWPOSX1,  def,str,MAX_PATH,filename);
   if ((res) && (_tcscmp(str,def))) 
      {
      sscanf(str,"%d",&optionsWindowPos.rcNormalPosition.left);
      full++;
      }
   res = GetPrivateProfileString(WSDSECTION,WINDOWPOSY1,  def,str,MAX_PATH,filename);
   if ((res) && (_tcscmp(str,def))) 
      {
      sscanf(str,"%d",&optionsWindowPos.rcNormalPosition.bottom);
      full++;
      }
   res = GetPrivateProfileString(WSDSECTION,WINDOWPOSX2,  def,str,MAX_PATH,filename);
   if ((res) && (_tcscmp(str,def))) 
      {  
      sscanf(str,"%d",&optionsWindowPos.rcNormalPosition.right);
      full++;
      }
   res = GetPrivateProfileString(WSDSECTION,WINDOWPOSY2,  def,str,MAX_PATH,filename);
   if ((res) && (_tcscmp(str,def))) 
      {
      sscanf(str,"%d",&optionsWindowPos.rcNormalPosition.top);
      full++;
      }

   if (full != 4) 
      firstTime = TRUE;
   else firstTime = FALSE;

   if (saved_lc)  
   {  
      lc = ::setlocale(LC_NUMERIC, saved_lc);  
      free(saved_lc);  
      saved_lc = NULL;  
   }     


}
