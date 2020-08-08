#include "unwrap.h"

#include "3dsmaxport.h"

/****



******/


void UnwrapMod::fnRelax(int iteration, float str, BOOL lockEdges, BOOL matchArea)
{
   theHold.Begin();
   HoldPoints();

   for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
   {
	   mMeshTopoData[ldID]->Relax(TVSubObjectMode, iteration, str, lockEdges, matchArea,this);
   }

   theHold.Accept(_T(GetString(IDS_PW_MOVE_UVW)));
   NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
   InvalidateView();
   if (ip) ip->RedrawViews(ip->GetTime());

}




void UnwrapMod::fnFit(int iteration, float str)
{
	theHold.Begin();
	HoldPoints();
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		mMeshTopoData[ldID]->RelaxFit(TVSubObjectMode,iteration,str,this);
	}
	theHold.Accept(_T(GetString(IDS_PW_RELAX)));
	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	InvalidateView();
	if (ip) ip->RedrawViews(ip->GetTime());

}



float UnwrapMod::fnGetRelaxAmount()
{
   return relaxAmount;
}
void  UnwrapMod::fnSetRelaxAmount(float amount)
{
   relaxAmount = amount;
}


int      UnwrapMod::fnGetRelaxIter()
{
   return relaxIteration;
}
void  UnwrapMod::fnSetRelaxIter(int iter)
{
   if (iter < 1) iter = 1;
   relaxIteration = iter;
}


BOOL  UnwrapMod::fnGetRelaxBoundary()
{
   return relaxBoundary;
}
void  UnwrapMod::fnSetRelaxBoundary(BOOL boundary)
{
   relaxBoundary = boundary;
}

BOOL  UnwrapMod::fnGetRelaxSaddle()
{
   return relaxSaddle;
}
void  UnwrapMod::fnSetRelaxSaddle(BOOL saddle)
{
   relaxSaddle = saddle;
}


void  UnwrapMod::fnRelax2()
{

   theHold.Begin();
   HoldPoints();
   theHold.Accept(_T(GetString(IDS_PW_RELAX)));

   if (relaxType == 0)
      fnRelaxByFaceAngle(relaxIteration,relaxStretch,relaxAmount,relaxBoundary);
   else if (relaxType == 1)
      fnRelaxByEdgeAngle(relaxIteration,relaxStretch,relaxAmount,relaxBoundary);
   else if (relaxType == 2)
      RelaxVerts2(relaxAmount, relaxIteration, relaxBoundary, relaxSaddle);
   
}
void  UnwrapMod::fnRelax2Dialog()
{
//bring up the dialog
   if (hRelaxDialog == NULL)
   {
      hRelaxDialog = CreateDialogParam(   hInstance,
                     MAKEINTRESOURCE(IDD_RELAXDIALOG),
//                   GetCOREInterface()->GetMAXHWnd(),
                     hWnd,
                     UnwrapRelaxFloaterDlgProc,
                     (LPARAM)this );
      ShowWindow(hRelaxDialog,SW_SHOW);
   }


}







void  UnwrapMod::RelaxVerts2(float relax, int iter, BOOL boundary, BOOL saddle)

{
   for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
   {
	   mMeshTopoData[ldID]->RelaxVerts2(TVSubObjectMode, relax, iter, boundary, saddle, this);
   }
}


void  UnwrapMod::SetRelaxDialogPos()
   {
   if (relaxWindowPos.length != 0) 
      SetWindowPlacement(relaxHWND,&relaxWindowPos);
   }

void  UnwrapMod::SaveRelaxDialogPos()
   {
   relaxWindowPos.length = sizeof(WINDOWPLACEMENT); 
   GetWindowPlacement(relaxHWND,&relaxWindowPos);
   }


INT_PTR CALLBACK UnwrapRelaxFloaterDlgProc(
      HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
   {
   UnwrapMod *mod = DLGetWindowLongPtr<UnwrapMod*>(hWnd);
   //POINTS p = MAKEPOINTS(lParam); commented out by sca 10/7/98 -- causing warning since unused.
   
   static ISpinnerControl *iAmount = NULL;
   static ISpinnerControl *iIterations = NULL;
   static ISpinnerControl *iStretch = NULL;

   static BOOL bBoundary = TRUE;
   static BOOL bCorner = FALSE;
   static float amount = 1.0f;
   static int iterations = 1;

   switch (msg) {
      case WM_INITDIALOG:
         {


         mod = (UnwrapMod*)lParam;
         mod->relaxHWND = hWnd;

         DLSetWindowLongPtr(hWnd, lParam);

//save our points

//create relax amount spinner and set value
         iAmount = SetupFloatSpinner(
            hWnd,IDC_RELAX_AMOUNTSPIN,IDC_RELAX_AMOUNT,
            0.0f,1.0f,mod->relaxAmount);  
         iAmount->SetScale(0.01f);
         amount = mod->relaxAmount;

         iIterations = SetupIntSpinner(
            hWnd,IDC_RELAX_ITERATIONSSPIN,IDC_RELAX_ITERATIONS,
            0,100000,mod->relaxIteration);   
         iIterations->SetScale(1.f);
         iterations = mod->relaxIteration;

         iStretch = SetupFloatSpinner(
            hWnd,IDC_RELAX_STRETCHSPIN,IDC_RELAX_STRETCH,
            0.0f,1.0f,mod->relaxStretch); 
         iStretch->SetScale(0.001f);
         
         HWND hType = GetDlgItem(hWnd,IDC_RELAXTYPE_COMBO);
         SendMessage(hType, CB_RESETCONTENT, 0, 0);
         SendMessage(hType, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_RELAX_FACE));
         SendMessage(hType, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_RELAX_EDGE));
         SendMessage(hType, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_RELAX_CENTERS));
         SendMessage(hType, CB_SETCURSEL, (mod->relaxType), 0L);

//set align cluster
         bBoundary = mod->relaxBoundary;
         bCorner = mod->relaxSaddle;
         CheckDlgButton(hWnd,IDC_BOUNDARY_CHECK,bBoundary);
         CheckDlgButton(hWnd,IDC_CORNERS_CHECK,bCorner);
         
//restore window pos
         mod->SetRelaxDialogPos();
//start the hold begin

         if (mod->relaxType == 2)
         {           
            iStretch->Enable(FALSE);
            HWND hCheck = GetDlgItem(hWnd,IDC_BOUNDARY_CHECK);
            EnableWindow(hCheck,TRUE);
            hCheck = GetDlgItem(hWnd,IDC_CORNERS_CHECK);
            EnableWindow(hCheck,TRUE);
         }
         else
         {           
            iStretch->Enable(TRUE);
            HWND hCheck = GetDlgItem(hWnd,IDC_BOUNDARY_CHECK);
            EnableWindow(hCheck,TRUE);
            hCheck = GetDlgItem(hWnd,IDC_CORNERS_CHECK);
            EnableWindow(hCheck,FALSE);
         }

         break;
         }
      case CC_SPINNER_BUTTONDOWN:
         if (LOWORD(wParam) == IDC_UNWRAP_BIASSPIN) 
            {
            }
         break;



      case WM_CLOSE:

         if (iAmount) ReleaseISpinner(iAmount);
         if (iIterations) ReleaseISpinner(iIterations);
         if (iStretch) ReleaseISpinner(iStretch);
         iAmount = NULL;
         iIterations = NULL;
         iStretch = NULL;

         mod->hRelaxDialog = NULL;
         EndDialog(hWnd,0);
         break;

      case WM_COMMAND:
         switch (LOWORD(wParam)) {
            case IDC_APPLY:
               {
                              
               theHold.Begin();
               mod->HoldPoints();
               theHold.Accept(_T(GetString(IDS_PW_RELAX)));

               bBoundary = IsDlgButtonChecked(hWnd,IDC_BOUNDARY_CHECK);
               bCorner = IsDlgButtonChecked(hWnd,IDC_CORNERS_CHECK);
               
               amount = iAmount->GetFVal();
               float stretch = iStretch->GetFVal();
               iterations = iIterations->GetIVal();

               HWND hType = GetDlgItem(hWnd,IDC_RELAXTYPE_COMBO);
               int type = SendMessage(hType, CB_GETCURSEL, 0L, 0);

               HWND hStatus = GetDlgItem(hWnd,IDC_STATUSTEXT);

               if (type == 0)
                  mod->fnRelaxByFaceAngle(iterations,stretch,amount,bBoundary,hStatus);
               else if (type == 1)
                  mod->fnRelaxByEdgeAngle(iterations,stretch,amount,bBoundary,hStatus);
               else if (type == 2)
                  mod->RelaxVerts2(amount,iterations,bBoundary,bCorner);

               mod->InvalidateView();
               TimeValue t = GetCOREInterface()->GetTime();
               mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
               GetCOREInterface()->RedrawViews(t);



               break;
               }

            case IDC_REVERT:
               {
               mod->hRelaxDialog = NULL;
               EndDialog(hWnd,0);
               break;
               }
            case IDC_RELAXTYPE_COMBO:
               if (HIWORD(wParam)== CBN_SELCHANGE)
               {
                  HWND hType = GetDlgItem(hWnd,IDC_RELAXTYPE_COMBO);
                  int type = SendMessage(hType, CB_GETCURSEL, 0L, 0);
                  if (type == 2)
                  {           
                     iStretch->Enable(FALSE);
                     HWND hCheck = GetDlgItem(hWnd,IDC_BOUNDARY_CHECK);
                     EnableWindow(hCheck,TRUE);
                     hCheck = GetDlgItem(hWnd,IDC_CORNERS_CHECK);
                     EnableWindow(hCheck,TRUE);
                  }
                  else
                  {           
                     iStretch->Enable(TRUE);
                     HWND hCheck = GetDlgItem(hWnd,IDC_BOUNDARY_CHECK);
                     EnableWindow(hCheck,TRUE);
                     hCheck = GetDlgItem(hWnd,IDC_CORNERS_CHECK);
                     EnableWindow(hCheck,FALSE);
                  }
                  
               }
               break;

            case IDC_DEFAULT:
               {
//get bias
               amount = iAmount->GetFVal();
               iterations = iIterations->GetIVal();
               float stretch = iStretch->GetFVal();
               HWND hType = GetDlgItem(hWnd,IDC_RELAXTYPE_COMBO);
               int type = SendMessage(hType, CB_GETCURSEL, 0L, 0);

               mod->relaxAmount = amount;
               mod->relaxIteration = iterations;
               mod->relaxStretch = stretch;
               mod->relaxType = type;


//get align
               bCorner = IsDlgButtonChecked(hWnd,IDC_CORNERS_CHECK);
               bBoundary = IsDlgButtonChecked(hWnd,IDC_BOUNDARY_CHECK);
               mod->relaxBoundary = bBoundary;
               mod->relaxSaddle = bCorner;
//set as defaults
               break;
               }
            }
         break;

      default:
         return FALSE;
      }
   return TRUE;
   }


float UnwrapMod::fnGetRelaxStretch()
{
   return relaxStretch;
}
void UnwrapMod::fnSetRelaxStretch(float stretch)
{
   if (stretch < 0.0f) stretch = 0.0f;
   if (stretch > 1.0f) stretch = 1.0f;
   relaxStretch = stretch;

}

int UnwrapMod::fnGetRelaxType()
{
   return relaxType;
}
void UnwrapMod::fnSetRelaxType(int type)
{
   if (type < 0) type = 0;
   if (type > 2) type = 2;
   relaxType = type;

}
