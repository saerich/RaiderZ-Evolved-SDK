#include "unwrap.h"

#include "3dsmaxport.h"

void  UnwrapMod::fnNormalMapDialog()
{
	//bring up the dialog
	DialogBoxParam(   hInstance,
		MAKEINTRESOURCE(IDD_NORMALMAPDIALOG),
		GetCOREInterface()->GetMAXHWnd(),
		//                   hWnd,
		UnwrapNormalFloaterDlgProc,
		(LPARAM)this );


}

void  UnwrapMod::SetNormalDialogPos()
{
	if (normalWindowPos.length != 0) 
		SetWindowPlacement(normalHWND,&normalWindowPos);
}

void  UnwrapMod::SaveNormalDialogPos()
{
	normalWindowPos.length = sizeof(WINDOWPLACEMENT); 
	GetWindowPlacement(normalHWND,&normalWindowPos);
}

INT_PTR CALLBACK UnwrapNormalFloaterDlgProc(
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	UnwrapMod *mod = DLGetWindowLongPtr<UnwrapMod*>(hWnd);
	//POINTS p = MAKEPOINTS(lParam); commented out by sca 10/7/98 -- causing warning since unused.
	static ISpinnerControl *iSpacing = NULL;

	switch (msg) {
	  case WM_INITDIALOG:

		  {
			  mod = (UnwrapMod*)lParam;
			  mod->normalHWND = hWnd;

			  DLSetWindowLongPtr(hWnd, lParam);

			  HWND hMethod = GetDlgItem(hWnd,IDC_METHOD_COMBO);
			  SendMessage(hMethod, CB_RESETCONTENT, 0, 0);

			  SendMessage(hMethod, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_BACKFRONT));
			  SendMessage(hMethod, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_LEFTRIGHT));
			  SendMessage(hMethod, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_TOPBOTTOM));
			  SendMessage(hMethod, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_BOXNOTOP));
			  SendMessage(hMethod, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_BOX));
			  SendMessage(hMethod, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_DIAMOND));

			  SendMessage(hMethod, CB_SETCURSEL, mod->normalMethod, 0L);


			  //create spinners and set value
			  iSpacing = SetupFloatSpinner(
				  hWnd,IDC_UNWRAP_SPACINGSPIN,IDC_UNWRAP_SPACING,
				  0.0f,1.0f,mod->normalSpacing);   

			  //set align cluster
			  CheckDlgButton(hWnd,IDC_NORMALIZE_CHECK,mod->normalNormalize);
			  CheckDlgButton(hWnd,IDC_ROTATE_CHECK,mod->normalRotate);
			  CheckDlgButton(hWnd,IDC_ALIGNWIDTH_CHECK,mod->normalAlignWidth);
			  mod->SetNormalDialogPos();

			  break;
		  }


	  case WM_COMMAND:
		  switch (LOWORD(wParam)) {
	  case IDC_OK:
		  {
			  mod->SaveNormalDialogPos();


			  float tempSpacing;
			  BOOL tempNormalize, tempRotate, tempAlignWidth;
			  int tempMethod;
			  tempSpacing = mod->normalSpacing;
			  tempNormalize = mod->normalNormalize;
			  tempRotate = mod->normalRotate;
			  tempAlignWidth = mod->normalAlignWidth;
			  tempMethod = mod->normalMethod;


			  mod->normalSpacing = iSpacing->GetFVal();

			  mod->normalNormalize = IsDlgButtonChecked(hWnd,IDC_NORMALIZE_CHECK);
			  mod->normalRotate = IsDlgButtonChecked(hWnd,IDC_ROTATE_CHECK);
			  mod->normalAlignWidth = IsDlgButtonChecked(hWnd,IDC_ALIGNWIDTH_CHECK); 

			  HWND hMethod = GetDlgItem(hWnd,IDC_METHOD_COMBO);
			  mod->normalMethod = SendMessage(hMethod, CB_GETCURSEL, 0, 0L);

			  mod->fnNormalMapNoParams();

			  mod->normalSpacing = tempSpacing;
			  mod->normalNormalize = tempNormalize;
			  mod->normalRotate = tempRotate;
			  mod->normalAlignWidth= tempAlignWidth;
			  mod->normalMethod = tempMethod;


			  ReleaseISpinner(iSpacing);
			  iSpacing = NULL;



			  EndDialog(hWnd,1);

			  break;
		  }
	  case IDC_CANCEL:
		  {

			  mod->SaveNormalDialogPos();
			  ReleaseISpinner(iSpacing);
			  iSpacing = NULL;

			  EndDialog(hWnd,0);

			  break;
		  }
	  case IDC_DEFAULT:
		  {
			  //get bias
			  mod->normalSpacing = iSpacing->GetFVal();

			  //get align
			  mod->normalNormalize = IsDlgButtonChecked(hWnd,IDC_NORMALIZE_CHECK);
			  mod->normalRotate = IsDlgButtonChecked(hWnd,IDC_ROTATE_CHECK);
			  mod->normalAlignWidth = IsDlgButtonChecked(hWnd,IDC_ALIGNWIDTH_CHECK); 

			  HWND hMethod = GetDlgItem(hWnd,IDC_METHOD_COMBO);
			  mod->normalMethod = SendMessage(hMethod, CB_GETCURSEL, 0, 0L);

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


void  UnwrapMod::fnNormalMapNoParams()
{
	Tab<Point3*> normList;

	if (normalMethod==0)  //front/back
	{
		normList.SetCount(2);

		normList[0] = new Point3(0.0f,1.0f,0.0f);
		normList[1] = new Point3(0.0f,-1.0f,0.0f);

	}
	else if (normalMethod==1)  //left/right
	{
		normList.SetCount(2);

		normList[0] = new Point3(1.0f,0.0f,0.0f);
		normList[1] = new Point3(-1.0f,0.0f,0.0f);

	}
	else if (normalMethod==2)  //top/bottom
	{
		normList.SetCount(2);

		normList[0] = new Point3(0.0f,0.0f,1.0f);
		normList[1] = new Point3(0.0f,0.0f,-1.0f);

	}

	else if (normalMethod==3)//box no top
	{
		normList.SetCount(4);

		normList[0] = new Point3(1.0f,0.0f,0.0f);
		normList[1] = new Point3(-1.0f,0.0f,0.0f);
		normList[2] = new Point3(0.0f,1.0f,0.0f);
		normList[3] = new Point3(0.0f,-1.0f,0.0f);
		//    normList[4] = new Point3(0.0f,0.0f,1.0f);
		//    normList[5] = new Point3(0.0f,0.0f,-1.0f);
	}

	else if (normalMethod==4)//box
	{
		normList.SetCount(6);

		normList[0] = new Point3(1.0f,0.0f,0.0f);
		normList[1] = new Point3(-1.0f,0.0f,0.0f);
		normList[2] = new Point3(0.0f,1.0f,0.0f);
		normList[3] = new Point3(0.0f,-1.0f,0.0f);
		normList[4] = new Point3(0.0f,0.0f,1.0f);
		normList[5] = new Point3(0.0f,0.0f,-1.0f);
	}
	else if (normalMethod==5)//box
	{
		normList.SetCount(8);

		normList[0] = new Point3(0.57735f, 0.57735f, 0.57735f);
		normList[1] = new Point3(-0.57735f, -0.57735f, 0.57735f);
		normList[2] = new Point3(0.57735f, -0.57735f, 0.57735f);
		normList[3] = new Point3(-0.57735f, 0.57735f, 0.57735f);
		normList[4] = new Point3(-0.57735f, 0.57735f, -0.57735f);
		normList[5] = new Point3(-0.57735f, -0.57735f, -0.57735f);
		normList[6] = new Point3(0.57735f, -0.57735f, -0.57735f);
		normList[7] = new Point3(0.57735f, 0.57735f, -0.57735f);
	}
	else return;


	fnNormalMap( &normList, normalSpacing, normalNormalize, 1, normalRotate, normalAlignWidth);
	for (int i = 0; i < normList.Count(); i++)
		delete normList[i];
}


void UnwrapMod::fnNormalMap(Tab<Point3*> *normalList, float spacing, BOOL normalize, int layoutType, BOOL rotateClusters, BOOL alignWidth)
{

	if (!ip) return;



	theHold.Begin();
	HoldPointsAndFaces();   

	BailStart();
	BOOL bContinue = TRUE;
	FreeClusterList();
	BOOL noSelection = TRUE;
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		if (mMeshTopoData[ldID]->GetFaceSelection().NumberSet())
			noSelection = FALSE;
	}

	if (noSelection)
	{
		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			BitArray fsel = mMeshTopoData[ldID]->GetFaceSelection();
			fsel.SetAll();
			mMeshTopoData[ldID]->SetFaceSelection(fsel);
		}
	}

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{		
		if (bContinue)
			bContinue = mMeshTopoData[ldID]->NormalMap(normalList,clusterList, this );
	}

	if (bContinue)
	{  
		if (layoutType == 1)
			bContinue = LayoutClusters( spacing, rotateClusters, alignWidth, FALSE);
		else bContinue = LayoutClusters2( spacing, rotateClusters, FALSE);
		if (bContinue)
		{
			NormalizeCluster();
		}
	}

	FreeClusterList();
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{	
		mMeshTopoData[ldID]->BuildTVEdges();
		mMeshTopoData[ldID]->BuildVertexClusterList();
	}

	if (bContinue)
	{
		theHold.Accept(_T(GetString(IDS_PW_PLANARMAP)));

		theHold.Suspend();
		fnSyncTVSelection();
		theHold.Resume();
	}
	else
	{
		theHold.Cancel();
	}

	CleanUpDeadVertices();

	theHold.Suspend();
	fnSyncGeomSelection();
	theHold.Resume();


	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	InvalidateView();

#ifdef DEBUGMODE 
	gEdgeHeight = 0.0f;
	gEdgeWidth = 0.0f;
	for (int i =0; i < clusterList.Count(); i++)
	{
		gEdgeHeight += clusterList[i]->h;
		gEdgeWidth += clusterList[i]->w;

	}
	ScriptPrint("Layout Type %d Rotate Cluster %d  Align Width %d\n",layoutType, rotateClusters, alignWidth); 

	ScriptPrint("Surface Area %f bounds area %f  per used %f\n",gSArea,gBArea,gSArea/gBArea); 
	ScriptPrint("Edge Height %f Edge Width %f\n",gEdgeHeight,gEdgeWidth); 
#endif


}


