#include "unwrap.h"



#include "unwrap.h"

#include "3dsmaxport.h"

//these are just debug globals so I can stuff data into to draw
#ifdef DEBUGMODE
//just some pos tabs to draw bounding volumes
extern Tab<float> jointClusterBoxX;
extern Tab<float> jointClusterBoxY;
extern Tab<float> jointClusterBoxW;
extern Tab<float> jointClusterBoxH;


extern float hitClusterBoxX,hitClusterBoxY,hitClusterBoxW,hitClusterBoxH;

extern int currentCluster, subCluster;

//used to turn off the regular display and only show debug display
extern BOOL drawOnlyBounds;
#endif





void UnwrapMod::BuildNormals(MeshTopoData *md, Tab<Point3> &normList)
{

	if (md == NULL) 
	{
		return;
	}



	if(md->GetMesh())
	{
		normList.SetCount(md->GetMesh()->numFaces);
		for (int i =0; i < md->GetMesh()->numFaces; i++)
		{
			//build normal
			Point3 p[3];
			for (int j =0; j < 3; j++)
			{
				p[j] = md->GetMesh()->verts[md->GetMesh()->faces[i].v[j]];
			}
			Point3 vecA,vecB, norm;
			vecA = Normalize(p[1] - p[0]);
			vecB = Normalize(p[2] - p[0]);
			norm = Normalize(CrossProd(vecA,vecB));


			normList[i] = norm;
		}
	}
	else if (md->GetMNMesh())
	{
		normList.SetCount(md->GetMNMesh()->numf);
		for (int i =0; i < md->GetMNMesh()->numf; i++)
		{
			Point3 norm =  md->GetMNMesh()->GetFaceNormal (i, TRUE);
			normList[i] = norm;
		}
	}
	else if (md->GetPatch())
	{
		normList.SetCount(md->GetPatch()->numPatches);
		for (int i =0; i < md->GetPatch()->numPatches; i++)
		{
			Point3 norm(0.0f,0.0f,0.0f);
			for (int j=0; j < md->GetPatch()->patches[i].type; j++)
			{
				Point3 vecA, vecB, p;
				p = md->GetPatch()->verts[md->GetPatch()->patches[i].v[j]].p;
				vecA = md->GetPatch()->vecs[md->GetPatch()->patches[i].vec[j*2]].p;
				if (j==0)
					vecB = md->GetPatch()->vecs[md->GetPatch()->patches[i].vec[md->GetPatch()->patches[i].type*2-1]].p;
				else vecB = md->GetPatch()->vecs[md->GetPatch()->patches[i].vec[j*2-1]].p;
				vecA = Normalize(p - vecA);
				vecB = Normalize(p - vecB);
				norm += Normalize(CrossProd(vecA,vecB));
			}
			normList[i] = Normalize(norm/(float)md->GetPatch()->patches[i].type);
		}
	}
}



Point3*  UnwrapMod::fnGetNormal(int faceIndex, INode *node)
{
	Point3 norm(0.0f,0.0f,0.0f);
	MeshTopoData *ld = GetMeshTopoData(node);
	if (ld)
	{
		Tab<Point3> objNormList;
		BuildNormals(ld,objNormList);
		if ((faceIndex >= 0) && (faceIndex < objNormList.Count()))
			norm = objNormList[faceIndex];
	}


	n = norm;
	return &n;
}


Point3*  UnwrapMod::fnGetNormal(int faceIndex)
{
	Point3 norm(0.0f,0.0f,0.0f);
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			Tab<Point3> objNormList;
			BuildNormals(ld,objNormList);
			if ((faceIndex >= 0) && (faceIndex < objNormList.Count()))
				norm = objNormList[faceIndex];
		}
	}


	n = norm;
	return &n;
}



void  UnwrapMod::fnUnfoldSelectedPolygons(int unfoldMethod, BOOL normalize)
{     

	// flatten selected polygons
	if (!ip) return;
	BailStart();

	theHold.Begin();
	HoldPointsAndFaces();   

	Point3 normal(0.0f,0.0f,1.0f);

	for (int ldID =0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		ld->HoldFaceSel();
	}

	BOOL bContinue = TRUE;
	for (int ldID =0; ldID < mMeshTopoData.Count(); ldID++)
	{

		Tab<Point3> mapNormal;
		mapNormal.SetCount(0);
		MeshTopoData *ld = mMeshTopoData[ldID];

		for (int ldIDPrep =0; ldIDPrep < mMeshTopoData.Count(); ldIDPrep++)
		{
			MeshTopoData *ldPrep = mMeshTopoData[ldIDPrep];
			if (ld != ldPrep)
				ldPrep->ClearFaceSelection();
			else
				ldPrep->RestoreFaceSel();
		}

		//hold our face selection
		//get our processed list 
		BitArray holdFaces = ld->GetFaceSelection();
		BitArray processedFaces = ld->GetFaceSelection();
		while (processedFaces.NumberSet())
		{
			//select the first one
			int seed = -1;
			for (int faceID = 0; faceID < processedFaces.GetSize(); faceID++)
			{
				if (processedFaces[faceID])
				{
					seed = faceID;
					faceID = processedFaces.GetSize();
				}
			}
			BitArray faceSel = ld->GetFaceSel();
			faceSel.ClearAll();
			//select the element the first one
			faceSel.Set(seed,TRUE);
			//select it
			ld->SetFaceSel(faceSel);
			SelectGeomElement(ld);
			faceSel = ld->GetFaceSel();
			int ns = faceSel.NumberSet();
//			ld->SelectElement(TVFACEMODE,FALSE);
			faceSel &= holdFaces;			
			//remove that from our process list
			for (int faceID = 0; faceID < faceSel.GetSize(); faceID++)
			{
				if (faceSel[faceID])
				{
					processedFaces.Set(faceID,FALSE);
				}
			}
			ld->SetFaceSel(faceSel);
			
			bContinue = BuildCluster( mapNormal, 5.0f, TRUE, TRUE);
			TSTR statusMessage;



			if (bContinue)
			{


				for (int i =0; i < clusterList.Count(); i++)
				{
					
					ld->ClearFaceSelection();
					for (int j = 0; j < clusterList[i]->faces.Count();j++)
						ld->SetFaceSelected(clusterList[i]->faces[j],TRUE);//	sel.Set(clusterList[i]->faces[j]);
					ld->PlanarMapNoScale(clusterList[i]->normal,this);


					int per = (i * 100)/clusterList.Count();
					statusMessage.printf("%s %d%%.",GetString(IDS_PW_STATUS_MAPPING),per);
					if (Bail(ip,statusMessage))
					{
						i = clusterList.Count();
						bContinue =  FALSE;
					}
				}

				DebugPrint ("Final Vct %d \n",ld->GetNumberTVVerts());

				if ( (bContinue) && (clusterList.Count() > 1) )
				{




						Tab<Point3> objNormList;
						BuildNormals(ld,objNormList);

						//remove internal edges
						Tab<int> clusterGroups;
						clusterGroups.SetCount(ld->GetNumberFaces());
						for (int i =0; i < clusterGroups.Count(); i++)
						{
							clusterGroups[i] = -1;
						}
						
						for (int i = 0; i < clusterList.Count(); i++)
						{
							for (int j = 0; j < clusterList[i]->faces.Count(); j++)
							{
								int faceIndex = clusterList[i]->faces[j];
								clusterGroups[faceIndex] = i;
							}
						}
						BitArray processedClusters;
						processedClusters.SetSize(clusterList.Count());
						processedClusters.ClearAll();

						Tab<BorderClass> edgesToBeProcessed;

						BOOL done = FALSE;
						int currentCluster = 0;

						processedClusters.Set(0);
						clusterList[0]->newX = 0.0f;
						clusterList[0]->newY = 0.0f;
						//    clusterList[0]->angle = 0.0f;
						for (int i = 0; i < clusterList[0]->borderData.Count(); i++)
						{
							int outerFaceIndex = clusterList[0]->borderData[i].outerFace;
							int connectedClusterIndex = clusterGroups[outerFaceIndex];
							if ((connectedClusterIndex != 0) && (connectedClusterIndex != -1))
							{
								edgesToBeProcessed.Append(1,&clusterList[0]->borderData[i]);
							}
						}
					
						BitArray seedFaceList;
						seedFaceList.SetSize(clusterGroups.Count());
						seedFaceList.ClearAll();
						for (int i = 0; i < seedFaces.Count(); i++)
						{
							seedFaceList.Set(seedFaces[i]);
						}

						while (!done)
						{
							Tab<int> clustersJustProcessed;
							clustersJustProcessed.ZeroCount();
							done = TRUE;

							int edgeToAlign = -1;
							float angDist = PI*2;
							if (unfoldMethod == 1)
								angDist =  PI*2;
							else if (unfoldMethod == 2) angDist = 0;
							int i;
							for (i = 0; i < edgesToBeProcessed.Count(); i++)
							{
								int outerFace = edgesToBeProcessed[i].outerFace;
								int connectedClusterIndex = clusterGroups[outerFace];
								if (!processedClusters[connectedClusterIndex])
								{
									int innerFaceIndex = edgesToBeProcessed[i].innerFace;
									int outerFaceIndex = edgesToBeProcessed[i].outerFace;
									//get angle
									Point3 innerNorm, outerNorm;
									innerNorm = objNormList[innerFaceIndex];
									outerNorm = objNormList[outerFaceIndex];
									float dot = DotProd(innerNorm,outerNorm);

									float angle = 0.0f;

									if (dot == -1.0f)
										angle = PI;
									else if (dot >= 1.0f)
										angle = 0.f;                  
									else angle = acos(dot);

									if (unfoldMethod == 1)
									{
										if (seedFaceList[outerFaceIndex])
											angle = 0.0f;
										if (angle < angDist)
										{
											angDist = angle;
											edgeToAlign = i;
										}
									}

									else if (unfoldMethod == 2)
									{
										if (seedFaceList[outerFaceIndex])
											angle = 180.0f;
										if (angle > angDist)
										{
											angDist = angle;
											edgeToAlign = i;
										}
									}

								}
							}
							if (edgeToAlign != -1)
							{
								int innerFaceIndex = edgesToBeProcessed[edgeToAlign].innerFace;
								int outerFaceIndex = edgesToBeProcessed[edgeToAlign].outerFace;
								int edgeIndex = edgesToBeProcessed[edgeToAlign].edge;


								int connectedClusterIndex = clusterGroups[outerFaceIndex];

								seedFaceList.Set(outerFaceIndex, FALSE);

								processedClusters.Set(connectedClusterIndex);
								clustersJustProcessed.Append(1,&connectedClusterIndex);	
								ld->AlignCluster(clusterList,i,connectedClusterIndex,innerFaceIndex, outerFaceIndex,edgeIndex,this);
								done = FALSE;
							}

							//build new cluster list
							for (int j = 0; j < clustersJustProcessed.Count(); j++)
							{
								int clusterIndex = clustersJustProcessed[j];
								for (int i = 0; i < clusterList[clusterIndex]->borderData.Count(); i++)
								{
									int outerFaceIndex = clusterList[clusterIndex]->borderData[i].outerFace;
									int connectedClusterIndex = clusterGroups[outerFaceIndex];
									if ((connectedClusterIndex != 0) && (connectedClusterIndex != -1) && (!processedClusters[connectedClusterIndex]))
									{
										edgesToBeProcessed.Append(1,&clusterList[clusterIndex]->borderData[i]);
									}
								}
							}
						}
					
				}

				ld->ClearSelection(TVVERTMODE);

				for (int i = 0; i < clusterList.Count(); i++)
				{
					MeshTopoData *ld = clusterList[i]->ld;
					ld->UpdateClusterVertices(clusterList);
					for (int j =0; j < clusterList[i]->faces.Count(); j++)
					{
						int faceIndex = clusterList[i]->faces[j];
						int degree = ld->GetFaceDegree(faceIndex);
						for (int k =0; k < degree; k++)
						{
							int vertexIndex = ld->GetFaceTVVert(faceIndex,k);//TVMaps.f[faceIndex]->t[k];
							ld->SetTVVertSelected(vertexIndex,TRUE);//vsel.Set(vertexIndex);
						}
					}
				}

				//now weld the verts
				if (normalize)
				{
					NormalizeCluster();
				}


				ld->WeldSelectedVerts(0.001f,this);

			}
	
			FreeClusterList();
		}
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

	
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{			
		mMeshTopoData[ldID]->BuildTVEdges();
		mMeshTopoData[ldID]->RestoreFaceSel();
	}

	theHold.Suspend();
	fnSyncGeomSelection();
	theHold.Resume();


	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	InvalidateView();

}
/*
void UnwrapMod::AlignCluster(int baseCluster, int moveCluster, int innerFaceIndex, int outerFaceIndex,int edgeIndex)
{



}
*/

void  UnwrapMod::fnUnfoldSelectedPolygonsNoParams()
{
	fnUnfoldSelectedPolygons(unfoldMethod, unfoldNormalize);
}     

void  UnwrapMod::fnUnfoldSelectedPolygonsDialog()
{
	//bring up the dialog
	DialogBoxParam(   hInstance,
		MAKEINTRESOURCE(IDD_UNFOLDDIALOG),
		GetCOREInterface()->GetMAXHWnd(),
		//                   hWnd,
		UnwrapUnfoldFloaterDlgProc,
		(LPARAM)this );


}

void  UnwrapMod::SetUnfoldDialogPos()
{
	if (unfoldWindowPos.length != 0) 
		SetWindowPlacement(unfoldHWND,&unfoldWindowPos);
}

void  UnwrapMod::SaveUnfoldDialogPos()
{
	unfoldWindowPos.length = sizeof(WINDOWPLACEMENT); 
	GetWindowPlacement(unfoldHWND,&unfoldWindowPos);
}



INT_PTR CALLBACK UnwrapUnfoldFloaterDlgProc(
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	UnwrapMod *mod = DLGetWindowLongPtr<UnwrapMod*>(hWnd);
	//POINTS p = MAKEPOINTS(lParam); commented out by sca 10/7/98 -- causing warning since unused.

	switch (msg) {
	  case WM_INITDIALOG:
		  {

			  mod = (UnwrapMod*)lParam;
			  mod->unfoldHWND = hWnd;

			  DLSetWindowLongPtr(hWnd, lParam);
			  ::SetWindowContextHelpId(hWnd, idh_unwrap_unfoldmap);

			  HWND hMethod = GetDlgItem(hWnd,IDC_METHOD_COMBO);
			  SendMessage(hMethod, CB_RESETCONTENT, 0, 0);

			  SendMessage(hMethod, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_FARTHESTFACE));
			  SendMessage(hMethod, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_CLOSESTFACE));

			  SendMessage(hMethod, CB_SETCURSEL, mod->unfoldMethod, 0L);

			  //set normalize cluster
			  CheckDlgButton(hWnd,IDC_NORMALIZE_CHECK,mod->unfoldNormalize);

			  break;
		  }

	  case WM_SYSCOMMAND:
		  if ((wParam & 0xfff0) == SC_CONTEXTHELP) 
		  {
			  DoHelp(HELP_CONTEXT, idh_unwrap_unfoldmap); 
		  }
		  return FALSE;
		  break;
	  case WM_COMMAND:
		  switch (LOWORD(wParam)) {
	  case IDC_OK:
		  {
			  mod->SaveUnfoldDialogPos();


			  BOOL tempNormalize;
			  int tempMethod;

			  tempNormalize = mod->unfoldNormalize;
			  tempMethod = mod->unfoldMethod;

			  HWND hMethod = GetDlgItem(hWnd,IDC_METHOD_COMBO);
			  mod->unfoldMethod = SendMessage(hMethod, CB_GETCURSEL, 0L, 0);


			  mod->unfoldNormalize = IsDlgButtonChecked(hWnd,IDC_NORMALIZE_CHECK);

			  mod->fnUnfoldSelectedPolygonsNoParams();

			  mod->unfoldNormalize = tempNormalize;
			  mod->unfoldMethod = tempMethod;


			  EndDialog(hWnd,1);

			  break;
		  }
	  case IDC_CANCEL:
		  {

			  mod->SaveFlattenDialogPos();

			  EndDialog(hWnd,0);

			  break;
		  }
	  case IDC_DEFAULT:
		  {

			  //get align
			  mod->unfoldNormalize = IsDlgButtonChecked(hWnd,IDC_NORMALIZE_CHECK);
			  HWND hMethod = GetDlgItem(hWnd,IDC_METHOD_COMBO);
			  mod->unfoldMethod = SendMessage(hMethod, CB_GETCURSEL, 0L, 0);

			  break;
		  }

		  }
		  break;

	  default:
		  return FALSE;
	}
	return TRUE;
}




void  UnwrapMod::fnHideSelectedPolygons()
{
/*FIXPW
	if (hiddenPolygons.GetSize() != TVMaps.f.Count() )
	{
		hiddenPolygons.SetSize(TVMaps.f.Count());
	}
	//check for type
	ModContextList mcList;     
	INodeTab nodes;

	if (!ip) return;
	ip->GetModContexts(mcList,nodes);

	int objects = mcList.Count();




	if (objects != 0)
	{


		MeshTopoData *md = (MeshTopoData*)mcList[0]->localData;  

		if (md == NULL) 
		{
			return;
		}


		hiddenPolygons |= md->faceSel;
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		ip->RedrawViews(ip->GetTime());
		InvalidateView();
	}
*/

}
void  UnwrapMod::fnUnhideAllPolygons()
{
/*FIXPW
	if (hiddenPolygons.GetSize() != TVMaps.f.Count() )
	{
		hiddenPolygons.SetSize(TVMaps.f.Count());
	}

	hiddenPolygons.ClearAll();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
	InvalidateView();
*/
}

void  UnwrapMod::fnSetSeedFace()
{
/*FIXPW
	seedFaces.ZeroCount();
	for (int i =0; i < TVMaps.f.Count(); i++)
	{
		if (TVMaps.f[i]->flags & FLAG_SELECTED)
			seedFaces.Append(1,&i);

	}
*/

}
void UnwrapMod::fnShowVertexConnectionList()
{
	if (showVertexClusterList)
		showVertexClusterList = FALSE;
	else showVertexClusterList = TRUE;

	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	InvalidateView();

}


BOOL  UnwrapMod::fnGetShowConnection()
{
	return showVertexClusterList;
}

void  UnwrapMod::fnSetShowConnection(BOOL show)
{
	showVertexClusterList = show;
	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	InvalidateView();
}




