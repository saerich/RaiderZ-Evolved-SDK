#include "unwrap.h"

#include "3dsmaxport.h"

void  UnwrapMod::fnStitchVerts(BOOL bAlign, float fBias)
{
	StitchVerts(bAlign, fBias,FALSE);
}

void  UnwrapMod::fnStitchVerts2(BOOL bAlign, float fBias, BOOL bScale)
{
	StitchVerts(bAlign, fBias,bScale);
}

void  UnwrapMod::StitchVerts(BOOL bAlign, float fBias, BOOL bScale)
{
	//build my connection list first

	TransferSelectionStart();
	int oldSubObjectMode = fnGetTVSubMode();
	fnSetTVSubMode(TVVERTMODE);

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		ld->Stitch(bAlign, fBias, bScale, this);
	}

	//now weld the verts
	float tempWeld = weldThreshold;
	weldThreshold = 0.001f;
	// theHold.Suspend();
	WeldSelected(FALSE);
	// theHold.Resume();
	weldThreshold = tempWeld;

	CleanUpDeadVertices();
//	vsel = oldSel;

	fnSetTVSubMode(oldSubObjectMode);
	TransferSelectionEnd(FALSE,TRUE);

	RebuildDistCache();
	RebuildEdges();
	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	if (ip) ip->RedrawViews(currentTime);
}

void  UnwrapMod::fnStitchVertsNoParams()
{
	theHold.Begin();
	HoldPointsAndFaces();   
	fnStitchVerts(bStitchAlign,fStitchBias);
	theHold.Accept(_T(GetString(IDS_PW_STITCH)));
	InvalidateView();

}
void  UnwrapMod::fnStitchVertsDialog()
{
	//bring up the dialog
	DialogBoxParam(   hInstance,
		MAKEINTRESOURCE(IDD_STICTHDIALOG),
		GetCOREInterface()->GetMAXHWnd(),
		//                   hWnd,
		UnwrapStitchFloaterDlgProc,
		(LPARAM)this );


}

void  UnwrapMod::SetStitchDialogPos()
{
	if (stitchWindowPos.length != 0) 
		SetWindowPlacement(stitchHWND,&stitchWindowPos);
}

void  UnwrapMod::SaveStitchDialogPos()
{
	stitchWindowPos.length = sizeof(WINDOWPLACEMENT); 
	GetWindowPlacement(stitchHWND,&stitchWindowPos);
}




INT_PTR CALLBACK UnwrapStitchFloaterDlgProc(
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	UnwrapMod *mod = DLGetWindowLongPtr<UnwrapMod*>(hWnd);
	//POINTS p = MAKEPOINTS(lParam); commented out by sca 10/7/98 -- causing warning since unused.
	static ISpinnerControl *iBias = NULL;
	static BOOL bAlign = TRUE;
	static BOOL bScale = TRUE;
	static float fBias= 0.0f;
	static float fSoftSel = 0.0f;
	static BitArray sel;
	static BOOL syncGeom = TRUE;
	switch (msg) {
	  case WM_INITDIALOG:


		  mod = (UnwrapMod*)lParam;
		  mod->stitchHWND = hWnd;

		  DLSetWindowLongPtr(hWnd, lParam);
		  ::SetWindowContextHelpId(hWnd, idh_unwrap_stitch);

		  syncGeom = mod->fnGetSyncSelectionMode();
		  mod->fnSetSyncSelectionMode(FALSE);

		  //create bias spinner and set value
		  iBias = SetupFloatSpinner(
			  hWnd,IDC_UNWRAP_BIASSPIN,IDC_UNWRAP_BIAS,
			  0.0f,1.0f,mod->fStitchBias);  
		  iBias->SetScale(0.01f);
		  //set align cluster
		  CheckDlgButton(hWnd,IDC_ALIGN_CHECK,mod->bStitchAlign);
		  CheckDlgButton(hWnd,IDC_SCALE_CHECK,mod->bStitchScale);
		  bAlign = mod->bStitchAlign;
		  fBias = mod->fStitchBias;
		  bScale = mod->bStitchScale;

		  //restore window pos
		  mod->SetStitchDialogPos();
		  //start the hold begin
		  if (!theHold.Holding())
		  {
			  theHold.SuperBegin();
			  theHold.Begin();
		  }
		  //hold the points and faces
		  mod->HoldPointsAndFaces();
		  //stitch initial selection
		  mod->fnStitchVerts2(bAlign,fBias,bScale);
		  mod->InvalidateView();

		  break;
	  case WM_SYSCOMMAND:
		  if ((wParam & 0xfff0) == SC_CONTEXTHELP) 
		  {
			  DoHelp(HELP_CONTEXT, idh_unwrap_stitch); 
		  }
		  return FALSE;
		  break;
	  case CC_SPINNER_BUTTONDOWN:
		  if (LOWORD(wParam) == IDC_UNWRAP_BIASSPIN) 
		  {
		  }
		  break;


	  case CC_SPINNER_CHANGE:
		  if (LOWORD(wParam) == IDC_UNWRAP_BIASSPIN) 
		  {
			  //get align
			  fBias = iBias->GetFVal();
			  //revert hold
			  theHold.Restore();

			  //call stitch again
			  mod->fnStitchVerts2(bAlign,fBias,bScale);
			  mod->InvalidateView();
		  }
		  break;

	  case WM_CUSTEDIT_ENTER:
	  case CC_SPINNER_BUTTONUP:
		  if ( (LOWORD(wParam) == IDC_UNWRAP_BIAS) || (LOWORD(wParam) == IDC_UNWRAP_BIASSPIN) )
		  {
		  }
		  break;


	  case WM_COMMAND:
		  switch (LOWORD(wParam)) {
	  case IDC_APPLY:
		  {
			  theHold.Accept(_T(GetString(IDS_PW_STITCH)));
			  theHold.SuperAccept(_T(GetString(IDS_PW_STITCH)));
			  mod->SaveStitchDialogPos();

			  ReleaseISpinner(iBias);
			  iBias = NULL;


			  mod->fnSetSyncSelectionMode(syncGeom);


			  EndDialog(hWnd,1);

			  break;
		  }
	  case IDC_REVERT:
		  {
			  theHold.Restore();
			  theHold.Cancel();
			  theHold.SuperCancel();

			  mod->SaveStitchDialogPos();
			  ReleaseISpinner(iBias);
			  iBias = NULL;

			  mod->fnSetSyncSelectionMode(syncGeom);

			  EndDialog(hWnd,0);

			  break;
		  }
	  case IDC_DEFAULT:
		  {
			  //get bias
			  fBias = iBias->GetFVal();
			  mod->fStitchBias = fBias;

			  //get align
			  bAlign = IsDlgButtonChecked(hWnd,IDC_ALIGN_CHECK);
			  mod->bStitchAlign = bAlign;

			  bScale = IsDlgButtonChecked(hWnd,IDC_SCALE_CHECK);
			  mod->bStitchScale = bScale;
			  //set as defaults
			  break;
		  }
	  case IDC_ALIGN_CHECK:
		  {
			  //get align
			  bAlign = IsDlgButtonChecked(hWnd,IDC_ALIGN_CHECK);
			  //revert hold
			  theHold.Restore();


			  //call stitch again
			  mod->fnStitchVerts2(bAlign,fBias,bScale);
			  mod->InvalidateView();
			  break;
		  }
	  case IDC_SCALE_CHECK:
		  {
			  //get align
			  bScale = IsDlgButtonChecked(hWnd,IDC_SCALE_CHECK);
			  //revert hold
			  theHold.Restore();

			  //call stitch again
			  mod->fnStitchVerts2(bAlign,fBias,bScale);
			  mod->InvalidateView();
			  break;
		  }

		  }
		  break;

	  default:
		  return FALSE;
	}
	return TRUE;
}



