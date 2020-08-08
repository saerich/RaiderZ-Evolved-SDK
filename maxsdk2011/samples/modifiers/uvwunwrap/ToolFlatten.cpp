#include "unwrap.h"

#include "3dsmaxport.h"

void  UnwrapMod::fnFlattenMapDialog()
   {
//bring up the dialog
      DialogBoxParam(   hInstance,
                     MAKEINTRESOURCE(IDD_FLATTENDIALOG),
                     GetCOREInterface()->GetMAXHWnd(),
//                   hWnd,
                     UnwrapFlattenFloaterDlgProc,
                     (LPARAM)this );


   }

void  UnwrapMod::SetFlattenDialogPos()
   {
   if (flattenHWND && (flattenWindowPos.length != 0) )
      SetWindowPlacement(flattenHWND,&flattenWindowPos);
   }

void  UnwrapMod::SaveFlattenDialogPos()
   {
	   if (flattenHWND)
	   {
		   flattenWindowPos.length = sizeof(WINDOWPLACEMENT); 
		   GetWindowPlacement(flattenHWND,&flattenWindowPos);
	   }
   }

INT_PTR CALLBACK UnwrapFlattenFloaterDlgProc(
      HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
   {
   UnwrapMod *mod = DLGetWindowLongPtr<UnwrapMod*>(hWnd);
   //POINTS p = MAKEPOINTS(lParam); commented out by sca 10/7/98 -- causing warning since unused.
   static ISpinnerControl *iAngle = NULL;
   static ISpinnerControl *iSpacing = NULL;

   switch (msg) {
      case WM_INITDIALOG:


         mod = (UnwrapMod*)lParam;
         mod->flattenHWND = hWnd;

         DLSetWindowLongPtr(hWnd, lParam);
		 ::SetWindowContextHelpId(hWnd, idh_unwrap_flattenmap);

//create spinners and set value
         iAngle = SetupFloatSpinner(
            hWnd,IDC_UNWRAP_ANGLESPIN,IDC_UNWRAP_ANGLE,
            0.0f,180.0f,mod->flattenAngleThreshold);  
         iSpacing = SetupFloatSpinner(
            hWnd,IDC_UNWRAP_SPACINGSPIN,IDC_UNWRAP_SPACING,
            0.0f,1.0f,mod->flattenSpacing);  

//set align cluster
         CheckDlgButton(hWnd,IDC_NORMALIZE_CHECK,mod->flattenNormalize);
         CheckDlgButton(hWnd,IDC_ROTATE_CHECK,mod->flattenRotate);
         CheckDlgButton(hWnd,IDC_COLLAPSE_CHECK,mod->flattenCollapse);
         mod->SetFlattenDialogPos();

         break;

	  case WM_SYSCOMMAND:
		  if ((wParam & 0xfff0) == SC_CONTEXTHELP) 
		  {
			  DoHelp(HELP_CONTEXT, idh_unwrap_flattenmap); 
		  }
		  return FALSE;
		  break;
      case WM_COMMAND:
         switch (LOWORD(wParam)) {
            case IDC_OK:
               {
               mod->SaveFlattenDialogPos();


               float tempSpacing, tempAngle;
               BOOL tempNormalize, tempRotate, tempCollapse;
               tempSpacing = mod->flattenSpacing;
               tempAngle = mod->flattenAngleThreshold;
               tempNormalize = mod->flattenNormalize;
               tempRotate = mod->flattenRotate;
               tempCollapse = mod->flattenCollapse;


               mod->flattenSpacing = iSpacing->GetFVal()*0.5f;
               mod->flattenAngleThreshold = iAngle->GetFVal();

               mod->flattenNormalize = IsDlgButtonChecked(hWnd,IDC_NORMALIZE_CHECK);
               mod->flattenRotate = IsDlgButtonChecked(hWnd,IDC_ROTATE_CHECK);
               mod->flattenCollapse = IsDlgButtonChecked(hWnd,IDC_COLLAPSE_CHECK); 

               BOOL byMatID = IsDlgButtonChecked(hWnd,IDC_BYMATIDS_CHECK); 

               if (byMatID)
                  mod->fnFlattenMapByMatIDNoParams();
               else mod->fnFlattenMapNoParams();

               mod->flattenSpacing = tempSpacing;
               mod->flattenAngleThreshold = tempAngle;
               mod->flattenNormalize = tempNormalize;
               mod->flattenRotate = tempRotate;
               mod->flattenCollapse= tempCollapse;


               ReleaseISpinner(iAngle);
               iAngle = NULL;
               ReleaseISpinner(iSpacing);
               iSpacing = NULL;

               EndDialog(hWnd,1);
               
               break;
               }
            case IDC_CANCEL:
               {
            
               mod->SaveFlattenDialogPos();
               ReleaseISpinner(iAngle);
               iAngle = NULL;
               ReleaseISpinner(iSpacing);
               iSpacing = NULL;

               EndDialog(hWnd,0);

               break;
               }
            case IDC_DEFAULT:
               {
//get bias
               mod->flattenSpacing = iSpacing->GetFVal();
               mod->flattenAngleThreshold = iAngle->GetFVal();

//get align
               mod->flattenNormalize = IsDlgButtonChecked(hWnd,IDC_NORMALIZE_CHECK);
               mod->flattenRotate = IsDlgButtonChecked(hWnd,IDC_ROTATE_CHECK);
               mod->flattenCollapse = IsDlgButtonChecked(hWnd,IDC_COLLAPSE_CHECK); 
               
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


void  UnwrapMod::fnFlattenMapNoParams()
   {
   Tab<Point3*> normList;
   normList.SetCount(6);

   normList[0] = new Point3(1.0f,0.0f,0.0f);
   normList[1] = new Point3(-1.0f,0.0f,0.0f);
   normList[2] = new Point3(0.0f,1.0f,0.0f);
   normList[3] = new Point3(0.0f,-1.0f,0.0f);
   normList[4] = new Point3(0.0f,0.0f,1.0f);
   normList[5] = new Point3(0.0f,0.0f,-1.0f);


   fnFlattenMap(flattenAngleThreshold, &normList, flattenSpacing, flattenNormalize, 2, flattenRotate, flattenCollapse);
   for (int i = 0; i < 6; i++)
      delete normList[i];
   }

void  UnwrapMod::fnFlattenMapByMatIDNoParams()
{
   fnFlattenMapByMatID(flattenAngleThreshold, flattenSpacing, flattenNormalize, 2, flattenRotate, flattenCollapse);
}

void  UnwrapMod::fnFlattenMapByMatID(float angleThreshold, float spacing, BOOL normalize, int layoutType, BOOL rotateClusters, BOOL fillHoles)
{

	int holdSubMode = fnGetTVSubMode();
	fnSetTVSubMode(TVVERTMODE);

//	vsel.SetAll();

	Tab<Point3*> normList;
	normList.SetCount(6);

	normList[0] = new Point3(1.0f,0.0f,0.0f);
	normList[1] = new Point3(-1.0f,0.0f,0.0f);
	normList[2] = new Point3(0.0f,1.0f,0.0f);
	normList[3] = new Point3(0.0f,-1.0f,0.0f);
	normList[4] = new Point3(0.0f,0.0f,1.0f);
	normList[5] = new Point3(0.0f,0.0f,-1.0f);



	int largestID = -1;
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];

		for (int i = 0; i < ld->GetNumberFaces(); i++)//TVMaps.f.Count(); i++)
		{
			int matID = ld->GetFaceMatID(i);
			if (matID > largestID)
				largestID = matID;
		}
	}
	BitArray usedMats;
	usedMats.SetSize(largestID+1);
	usedMats.ClearAll();
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		for (int i = 0; i < ld->GetNumberFaces(); i++)//TVMaps.f.Count(); i++)
		{
			int matID = ld->GetFaceMatID(i);
			usedMats.Set(matID,TRUE);
		}
	}


	Tab<int> matIDs;
	matIDs.SetCount(usedMats.NumberSet());
	int ct = 0;
	for (int i = 0; i < usedMats.GetSize(); i++)
	{
		if (usedMats[i])
		{
			matIDs[ct] = i;
			ct++;
		}

	}

	//loop through our mat ID
	Tab<ClusterClass*> matIDClusters;


	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		for (int i = 0; i < matIDs.Count(); i++)
		{

				int matID = matIDs[i];
				FreeClusterList();
				for (int ldIDSel = 0; ldIDSel < mMeshTopoData.Count(); ldIDSel++)
				{
					mMeshTopoData[ldIDSel]->ClearFaceSelection();
				}
				ld->SelectByMatID(matID);
				if (ld->GetFaceSelection().NumberSet())
				{

					ClusterClass *mcluster = new ClusterClass();

					mcluster->ld = ld;
					for (int j = 0; j < ld->GetNumberFaces(); j++)//TVMaps.f.Count(); j++)
					{
						if (ld->GetFaceSelected(j))//(TVMaps.f[j]->MatID == matIDs[i])
						{
							mcluster->faces.Append(1,&j,100);
						}
					}

					matIDClusters.Append(1,&mcluster,5);

					fnFlattenMap(flattenAngleThreshold, &normList, flattenSpacing, FALSE, 2, flattenRotate, flattenCollapse);
				}
			
		}
	}


	FreeClusterList();

	clusterList.SetCount(matIDClusters.Count());
	for (int i = 0; i < matIDClusters.Count(); i++)
	{
		clusterList[i] = new ClusterClass();
		clusterList[i]->ld = matIDClusters[i]->ld;
		clusterList[i]->faces.SetCount(matIDClusters[i]->faces.Count());
		for (int j = 0; j < matIDClusters[i]->faces.Count(); j++)
			clusterList[i]->faces[j] = matIDClusters[i]->faces[j];

	}

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		mMeshTopoData[ldID]->UpdateClusterVertices(clusterList);

	Pack(0, spacing, normalize, rotateClusters, fillHoles,FALSE,FALSE);


	FreeClusterList();

	TimeValue t = GetCOREInterface()->GetTime();
	if (normalize)
	{
		float per = 1.0f-(spacing*2.0f);
		float add = spacing;
		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			MeshTopoData *ld = mMeshTopoData[ldID];
			for (int i = 0; i < ld->GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++)
			{
				Point3 p = ld->GetTVVert(i);

				p *= per;
				p.x += add;
				p.y += add;

				ld->SetTVVert(t,i,p,this);

//				if (TVMaps.cont[i]) 
//					TVMaps.cont[i]->SetValue(0,&TVMaps.v[i].p,CTRL_ABSOLUTE);

			}
		}
	}

	for (int i = 0; i < matIDs.Count(); i++)
	{
		if (matIDClusters[i])
			delete matIDClusters[i];
	}
	for (int i = 0; i < 6; i++)
		delete normList[i];

	fnSetTVSubMode(holdSubMode);

}

void UnwrapMod::fnFlattenMap(float angleThreshold, Tab<Point3*> *normalList, float spacing, BOOL normalize, int layoutType, BOOL rotateClusters, BOOL fillHoles)

{

	BailStart();

	if (preventFlattening) return;
	/*
	if (TVMaps.f.Count() == 0) return;

	BitArray *polySel = fnGetSelectedPolygons();
	if (polySel == NULL) 
	return;
	*/

	theHold.Begin();
	HoldPointsAndFaces();   
	/*
	BitArray holdPolySel;
	holdPolySel.SetSize(polySel->GetSize());
	holdPolySel = *polySel;
	*/
	Point3 normal(0.0f,0.0f,1.0f);

	Tab<Point3> mapNormal;
	mapNormal.SetCount(normalList->Count());
	for (int i =0; i < mapNormal.Count(); i++)
	{
		mapNormal[i] = *(*normalList)[i];
	}

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

	TSTR statusMessage;
	BOOL bContinue = BuildCluster( mapNormal, angleThreshold, TRUE, TRUE);
	/*
	BitArray sel;
	sel.SetSize(TVMaps.f.Count());
	*/
	gBArea = 0.0f;

	int initialCluster = clusterList.Count();

	if (bContinue)
	{
		for (int i =0; i < clusterList.Count(); i++)
		{
			MeshTopoData *ld = clusterList[i]->ld;
			ld->ClearSelection(TVFACEMODE);//         sel.ClearAll();
			for (int j = 0; j < clusterList[i]->faces.Count();j++)
				ld->SetFaceSelected(clusterList[i]->faces[j],TRUE);//sel.Set(clusterList[i]->faces[j]);
			ld->PlanarMapNoScale(clusterList[i]->normal,this);

			int per = (i * 100)/clusterList.Count();
			statusMessage.printf("%s %d%%.",GetString(IDS_PW_STATUS_MAPPING),per);
			if (Bail(ip,statusMessage))
			{
				i = clusterList.Count();
				bContinue =  FALSE;
			}

		}

		//    if (0)
		if (bContinue)
		{
			for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
				mMeshTopoData[ldID]->UpdateClusterVertices(clusterList);

			if (layoutType == 1)
				bContinue = LayoutClusters( spacing, rotateClusters, TRUE, fillHoles);
			else
			{
				if (flattenMax5)
					bContinue = LayoutClusters3( spacing, rotateClusters, fillHoles);
				else bContinue = LayoutClusters2( spacing, rotateClusters, fillHoles);

			}

			//normalize map to 0,0 to 1,1
			if ((bContinue) && (normalize))
			{
				NormalizeCluster(spacing);
			}
		}

	}

	CleanUpDeadVertices();
	if (bContinue)
	{
		theHold.Accept(_T(GetString(IDS_PW_FLATTEN)));

		theHold.Suspend();
		fnSyncTVSelection();
		theHold.Resume();

	}
	else
	{
		theHold.Cancel();
		//    theHold.SuperCancel();
	}

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{	
		mMeshTopoData[ldID]->SetTVEdgeInvalid();
	}


	theHold.Suspend();
	fnSyncTVSelection();
	fnSyncGeomSelection();
	theHold.Resume();


	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	InvalidateView();


#ifdef DEBUGMODE 
	if (gDebugLevel >= 1)
	{
		int finalCluster = clusterList.Count();
		gEdgeHeight = 0.0f;
		gEdgeWidth = 0.0f;
		for (int i =0; i < clusterList.Count(); i++)
		{
			gEdgeHeight += clusterList[i]->h;
			gEdgeWidth += clusterList[i]->w;

		}

		ScriptPrint("Surface Area %f bounds area %f  per used %f\n",gSArea,gBArea,gSArea/gBArea); 
		ScriptPrint("Edge Height %f Edge Width %f\n",gEdgeHeight,gEdgeWidth); 
		ScriptPrint("Initial Clusters %d finalClusters %d\n",initialCluster,finalCluster); 
	}

#endif

	FreeClusterList();

	statusMessage.printf("Done, area coverage %3.2f",(gSArea/gBArea)*100.f);
	Bail(ip,statusMessage,0);


}


void  UnwrapMod::fnSetMax5Flatten(BOOL like5)
{
   flattenMax5 = like5;
}