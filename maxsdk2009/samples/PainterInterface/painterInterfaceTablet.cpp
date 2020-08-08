
#include "painterInterface.h"
#include "IpainterInterface.h"

#include "3dsmaxport.h"

HCTX static NEAR TabletInit(HWND hWnd, PainterInterface *mod)
{
   LOGCONTEXT lcMine;

   if(mod->hWinTabDLL == NULL) return NULL;

   UINT p,rp; 
   rp = mod->PaintWTInfo(WTI_DEVICES, DVC_NPRESSURE, &p );


   /* get default region */
   mod->PaintWTInfo(WTI_DEFCONTEXT, 0, &lcMine);

   /* modify the digitizing region */
   wsprintf(lcMine.lcName, "PrsTest Digitizing %x", hInstance);
   lcMine.lcOptions |= CXO_MESSAGES|CXO_SYSTEM;
   lcMine.lcPktData = PACKETDATA;
   lcMine.lcPktMode = PACKETMODE;
   lcMine.lcMoveMask = PACKETDATA;
   lcMine.lcBtnUpMask = lcMine.lcBtnDnMask;

   /* output in 10000 x 10000 grid */
   lcMine.lcOutOrgX = lcMine.lcOutOrgY = 0;
   lcMine.lcOutExtX = 10000;
   lcMine.lcOutExtY = 10000;

   /* open the region */
   if (mod->hWinTabDLL)
      return mod->PaintWTOpen(hWnd, &lcMine, TRUE);
   else return NULL;

}

static HCTX hTab = NULL;

static INT_PTR CALLBACK TabletFloaterDlgProc(
      HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
   {

   
   PACKET pkt;
   static UINT prsOld, prsNew;

   PainterInterface *painter = DLGetWindowLongPtr<PainterInterface*>(hWnd);
   switch (msg) {
      case WM_INITDIALOG: {
         painter = (PainterInterface*)lParam;
         DLSetWindowLongPtr(hWnd, lParam);

         
//       hTab = TabletInit(hWnd,painter);

         break;
         }

      case WT_PACKET:

         if( painter!=NULL ) {
            if ((painter->hWinTabDLL) && (painter->PaintWTPacket((HCTX)lParam, wParam, &pkt)) )
               {
                  if ((painter->pblock!=NULL) && painter->GetPressureEnable())
                     painter->fpressure = (float)(pkt.pkNormalPressure)/1024.0f;
                  else painter->fpressure = 1.0f;
               }
            else painter->fpressure  = 1.0f;
         }
         break;


      

      default:
         return FALSE;
      }

   return TRUE;
   }



void PainterInterface::CreateTabletWindow()
   {
   if (painterTabletPressure==NULL)
   {
      painterTabletPressure = CreateDialogParam(hInstance,
                 MAKEINTRESOURCE(IDD_DIALOG1),
                 GetCOREInterface()->GetMAXHWnd(),
                 TabletFloaterDlgProc,
                 (LPARAM)this);
      InitTablet(GetPressureEnable());
   }
   }
void PainterInterface::DestroyTabletWindow()
   {
/*
   DestroyWindow(painterTabletPressure);
   painterTabletPressure = NULL;
   InitTablet(FALSE);
*/
   }
void PainterInterface::InitTablet(BOOL init)
   {
   
   if ((painterTabletPressure) && (init))
      {
      if (!loadedDLL)
         {
         LoadWinTabDLL();
         loadedDLL = TRUE;
         }
      if (hTab == NULL)
         hTab = TabletInit(painterTabletPressure,this);
      }
   else
      {
//    UnLoadWinTabDLL();
      }
   }

void PainterInterface::CloseTablet()
{
   if (hTab != NULL)
      PaintWTClose(hTab);
   hTab = NULL;
}