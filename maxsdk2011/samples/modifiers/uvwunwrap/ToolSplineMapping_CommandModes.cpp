/********************************************************************** *<
FILE: ToolSplineMapping.cpp

DESCRIPTION: Thisc onatins all our spline map command modes and mouse procs
	Align
	Add Cross Section


HISTORY: 12/31/96
CREATED BY: Peter Watje


*>	Copyright (c) 2007, All Rights Reserved.
**********************************************************************/

#include "unwrap.h"

static PickSplineNode thePickSplineMode;


SplineMapAlignSectionMode::SplineMapAlignSectionMode(UnwrapMod* bmod, IObjParam *i) :
fgProc(bmod), eproc(bmod,i) 
{
	mod=bmod;
}

void SplineMapAlignSectionMode::EnterMode()
{
	mod->fnSplineMap_SetMode(SplineData::kAlignSectionMode);
	mod->fnSplineMap_UpdateUI();
}
void SplineMapAlignSectionMode::ExitMode()
{
	mod->fnSplineMap_SetMode(SplineData::kNoMode);
	mod->fnSplineMap_UpdateUI();
}


int SplineMapAlignSectionMouseProc::proc(
								  HWND hwnd, 
								  int msg, 
								  int point, 
								  int flags, 
								  IPoint2 m )
{
	int res = TRUE;
	if ( !mod->ip ) return FALSE;


	int crossing = TRUE;
	int type = HITTYPE_POINT;
	HitRegion hr;
	MakeHitRegion(hr,type, crossing,4,&m);	

	ViewExp *vpt = GetCOREInterface()->GetActiveViewport();

	GraphicsWindow *gw = vpt->getGW();

	TimeValue t = GetCOREInterface()->GetTime();

	Tab<int> hitSplines;
	Tab<int> hitCrossSections;
	SplineMapProjectionTypes projType = kCircular;
	int val = 0;
	mod->pblock->GetValue(unwrap_splinemap_projectiontype,t,val,FOREVER);
	if (val == 1)
		projType = kPlanar;



	switch ( msg ) 
	{
				case MOUSE_PROPCLICK:
					iObjParams->PopCommandMode();
					break;

				case MOUSE_POINT:
					//on mouse up do the align on the selected faces
					
					if (mod->fnSplineMap_HitTestCrossSection(gw,hr,projType,hitSplines,hitCrossSections))
					{
						mod->fnSplineMap_PasteToSelected(hitSplines[0], hitCrossSections[0]);
						macroRecorder->FunctionCall(_T("$.modifiers[#unwrap_uvw].unwrap6.splineMap_PasteToSelected"), 2, 0,
												mr_int, hitSplines[0]+1,
												mr_int, hitCrossSections[0]+1);
					}
					SetCursor(LoadCursor(NULL,IDC_ARROW));
					break;

				case MOUSE_FREEMOVE:
					//hit test the sections

					if (mod->fnSplineMap_HitTestCrossSection(gw,hr,projType,hitSplines,hitCrossSections))
					{
							//set cursor
						SetCursor(mod->selCur);
					}
					else
					{
						SetCursor(LoadCursor(NULL,IDC_ARROW));
					}

					break;

	}

	GetCOREInterface()->ReleaseViewport(vpt);
	return res;
}



SplineMapAlignFaceMode::SplineMapAlignFaceMode(UnwrapMod* bmod, IObjParam *i) :
fgProc(bmod), eproc(bmod,i) 
{
	mod=bmod;
}

void SplineMapAlignFaceMode::EnterMode()
{
	mod->fnSplineMap_SetMode(SplineData::kAlignFaceMode);
	mod->fnSplineMap_UpdateUI();
}
void SplineMapAlignFaceMode::ExitMode()
{
	mod->fnSplineMap_SetMode(SplineData::kNoMode);
	mod->fnSplineMap_UpdateUI();
}


int SplineMapAlignFaceMouseProc::proc(
								  HWND hwnd, 
								  int msg, 
								  int point, 
								  int flags, 
								  IPoint2 m )
{
	int res = TRUE;
	if ( !mod->ip ) return FALSE;


	int crossing = TRUE;
	int type = HITTYPE_POINT;
	HitRegion hr;
	MakeHitRegion(hr,type, crossing,4,&m);

	ViewExp *vpt = GetCOREInterface()->GetActiveViewport();

	GraphicsWindow *gw = vpt->getGW();

	TimeValue t = GetCOREInterface()->GetTime();




	switch ( msg ) {
				case MOUSE_PROPCLICK:
					iObjParams->PopCommandMode();
					break;

				case MOUSE_POINT:
					//on mouse up do the align on the selected faces
					{
						int ct = mod->GetMeshTopoDataCount();
						for (int ldID = 0; ldID < ct; ldID++)
						{
							MeshTopoData *ld = mod->GetMeshTopoData(ldID);
							Tab<int> hitFaces;
							INode *node = mod->GetMeshTopoDataNode(ldID);
							ld->HitTestFaces(gw,node,hr,hitFaces);
							if (hitFaces.Count())
							{
								//set cursor
								int faceIndex = hitFaces[0];
								Point3 norm = ld->GetGeomFaceNormal(faceIndex);
								INode *node = mod->GetMeshTopoDataNode(ldID);
								TimeValue t = GetCOREInterface()->GetTime();
								Matrix3 tm = node->GetObjectTM(t);
								norm = VectorTransform(tm,norm);

								mod->fnSplineMap_AlignSelected(norm);

								macroRecorder->FunctionCall(_T("$.modifiers[#unwrap_uvw].unwrap6.splineMap_AlignSelected"), 1, 0,
												mr_point3, &norm );

								res = FALSE;
							}
							else
							{
								SetCursor(LoadCursor(NULL,IDC_ARROW));
							}
						}

					}
					break;

				case MOUSE_FREEMOVE:
					//hit test the mesh faces
					int ct = mod->GetMeshTopoDataCount();
					for (int ldID = 0; ldID < ct; ldID++)
					{
						MeshTopoData *ld = mod->GetMeshTopoData(ldID);
						Tab<int> hitFaces;
						INode *node = mod->GetMeshTopoDataNode(ldID);
						ld->HitTestFaces(gw,node,hr,hitFaces);
						if (hitFaces.Count())
						{
							//set cursor
							SetCursor(mod->selCur);
						}
						else
						{
							SetCursor(LoadCursor(NULL,IDC_ARROW));
						}
					}

					break;

	}

	GetCOREInterface()->ReleaseViewport(vpt);
	return res;
}





SplineMapAddCrossSectionMode::SplineMapAddCrossSectionMode(UnwrapMod* bmod, IObjParam *i) :
fgProc(bmod), eproc(bmod,i) 
{
	mod=bmod;
}

void SplineMapAddCrossSectionMode::EnterMode()
{
	mod->fnSplineMap_SetMode(SplineData::kAddMode);
	mod->fnSplineMap_UpdateUI();
}
void SplineMapAddCrossSectionMode::ExitMode()
{
	mod->fnSplineMap_SetMode(SplineData::kNoMode);
	mod->fnSplineMap_UpdateUI();
}


int SplineMapAddCrossSectionMouseProc::proc(
	HWND hwnd, 
	int msg, 
	int point, 
	int flags, 
	IPoint2 m )
{
	int res = TRUE;
	if ( !mod->ip ) return FALSE;


	int crossing = TRUE;
	int type = HITTYPE_POINT;
	HitRegion hr;
	MakeHitRegion(hr,type, crossing,4,&m);

	ViewExp *vpt = GetCOREInterface()->GetActiveViewport();

	GraphicsWindow *gw = vpt->getGW();

	TimeValue t = GetCOREInterface()->GetTime();




	switch ( msg ) {
				case MOUSE_PROPCLICK:
					iObjParams->PopCommandMode();
					break;

				case MOUSE_POINT:
					//on mouse up do the align on the selected faces
					{
						float u = 0.0f;
						int splineIndex = 0;
						BOOL hit = mod->fnSplineMap_HitTestSpline(gw,&hr,splineIndex,u);
						if (hit)
						{
							mod->fnSplineMap_InsertCrossSection(splineIndex,u);
							TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap6.splineMap_InsertCrossSection"));
							macroRecorder->FunctionCall(mstr, 2, 0,
												mr_int, splineIndex+1,
												mr_float,u);

						}

					}
					break;

				case MOUSE_FREEMOVE:
					//hit test the mesh faces
					float u = 0.0f;
					int splineIndex = 0;					
					BOOL hit = mod->fnSplineMap_HitTestSpline(gw,&hr,splineIndex,u);
					if (hit)
					{
							//set cursor
						SetCursor(mod->selCur);
					}
					else
					{
						SetCursor(LoadCursor(NULL,IDC_ARROW));
					}

					break;

	}

	GetCOREInterface()->ReleaseViewport(vpt);
	return res;
}


INT_PTR CALLBACK UnwrapSplineMapFloaterDlgProc(
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Interface *ip = GetCOREInterface();
	TimeValue t = ip->GetTime();
	UnwrapMod *mod = DLGetWindowLongPtr<UnwrapMod*>(hWnd);
	static ISpinnerControl *resampleSpinner = NULL;
	switch (msg) {
	  case WM_INITDIALOG:
		  {
			  mod = (UnwrapMod*)lParam;
			  DLSetWindowLongPtr(hWnd, lParam);
			  HWND hType = GetDlgItem(hWnd,IDC_PROJ_COMBO);
			  SendMessage(hType, CB_RESETCONTENT, 0, 0);
			  SendMessage(hType, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_CIRCULAR));
			  SendMessage(hType, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_PLANAR2));
				::SetWindowContextHelpId(hWnd, idh_unwrap_splinemap);
			 
				ip->SetCoordCenter(ORIGIN_SELECTION);
				ip->SetRefCoordSys(COORDS_LOCAL);
				ip->EnableRefCoordSys(FALSE);
				ip->EnableCoordCenter(FALSE);


				int sampleCount = 4;
				mod->pblock->GetValue(unwrap_splinemap_resample_count,0,sampleCount,FOREVER);
				if (sampleCount < 2)
					sampleCount = 2;
				resampleSpinner = SetupIntSpinner(
					hWnd,IDC_UNWRAP_RESAMPLESPIN,IDC_UNWRAP_RESAMPLE,
					2,256,sampleCount);  
				resampleSpinner->SetScale(1.0f);
		  }
		  break;

	  case WM_SYSCOMMAND:
		  if ((wParam & 0xfff0) == SC_CONTEXTHELP) 
		  {
			  DoHelp(HELP_CONTEXT, idh_unwrap_splinemap); 
		  }
		  return FALSE;
		  break;


	  case WM_DESTROY:
		  if (resampleSpinner)
		  {
			  ReleaseISpinner(resampleSpinner);
		  }
		  resampleSpinner = NULL;
		  break;

	  case WM_CLOSE:
		  mod->fnSplineMap_Cancel();
		  mod->fnSetMapMode(NOMAP);
		  ip->EnableRefCoordSys(TRUE);
		  ip->EnableCoordCenter(TRUE);

		  EndDialog(hWnd,0);
		  break;


	  case WM_COMMAND:
		  switch (LOWORD(wParam)) 
		  {
			  case IDC_PICKSPLINE_BUTTON:
				  {
					  
					  if (GetCOREInterface()->GetCommandMode() == (CommandMode *)&thePickSplineMode)
					  {
							GetCOREInterface()->PopCommandMode();
					  }
					  else
					  {					  
						  thePickSplineMode.mod  = mod;					
						  GetCOREInterface()->SetPickMode(&thePickSplineMode);
					  }
					  break;
				  }
			  case IDC_MANUALSEAMS_CHECK:
				  {
					  BOOL manualSeams = GetCheckBox(hWnd,IDC_MANUALSEAMS_CHECK);
					  theHold.Begin();
					  mod->pblock->SetValue(unwrap_splinemap_manualseams,t,manualSeams);
					  mod->fnSplineMap();
					  theHold.Accept(_T(GetString(IDS_PW_SPLINEMAP)));

					  mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
					  mod->InvalidateView();
					  if (ip) ip->RedrawViews(ip->GetTime());

					  break;
				  }
			  case IDC_FIT_BUTTON:
				  {
					  mod->fnSplineMap_Fit(FALSE,1.2f);
					  TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap6.splineMap_Fit"));
					  macroRecorder->FunctionCall(mstr, 2, 0,
							mr_bool,FALSE,
							mr_float,1.2f
							);

					  break;
				  }
			  case IDC_ADD_BUTTON:
				  {
					  mod->fnSplineMap_AddCrossSectionCommandMode();
					  break;
				  }
			  case IDC_REMOVE_BUTTON:
				  {
					  mod->fnSplineMap_DeleteSelectedCrossSections();
					  TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap6.splineMap_Delete"));
					  macroRecorder->FunctionCall(mstr, 0, 0);

					  break;
				  }

			  case IDC_ALIGNSECTION_BUTTON:
				  {
					  mod->fnSplineMap_AlignSectionCommandMode();
					  break;
				  }
			  case IDC_ALIGNFACE_BUTTON:
				  {
					  mod->fnSplineMap_AlignFaceCommandMode();
					  break;
				  }
			  case IDC_RESAMPLE_BUTTON:
				  {
					  int ct = resampleSpinner->GetIVal();
					  theHold.Begin();
					  mod->fnSplineMap_Resample(ct);
				  	  theHold.Accept(_T(GetString(IDS_PW_SPLINEMAP)));
				  }
				  break;
			  case IDC_COMMIT_BUTTON:
				  {
			
					  if (resampleSpinner)
					  {
						  int sampleCount = resampleSpinner->GetIVal();
						  mod->pblock->SetValue(unwrap_splinemap_resample_count,0,sampleCount);
						  ReleaseISpinner(resampleSpinner);
					  }
					  resampleSpinner = NULL;

					  mod->fnSetMapMode(NOMAP);
					  ip->EnableRefCoordSys(TRUE);
					  ip->EnableCoordCenter(TRUE);



					  EndDialog(hWnd,1);
					  break;
				  }
			  case IDC_CANCEL_BUTTON:
				  {
					  mod->fnSplineMap_Cancel();
					  mod->fnSetMapMode(NOMAP);
					  ip->EnableRefCoordSys(TRUE);
					  ip->EnableCoordCenter(TRUE);
					  if (resampleSpinner)
					  {
						  ReleaseISpinner(resampleSpinner);
					  }
					  resampleSpinner = NULL;


					  EndDialog(hWnd,0);
					  break;
				  }

			  case IDC_PROJ_COMBO:
				  if (HIWORD(wParam)== CBN_SELCHANGE)
				  {
					  HWND hType = GetDlgItem(hWnd,IDC_PROJ_COMBO);
					  int type = SendMessage(hType, CB_GETCURSEL, 0L, 0);
					  
					  theHold.Begin();
					  mod->pblock->SetValue(unwrap_splinemap_projectiontype,t,type);
					  mod->fnSplineMap();
					  theHold.Accept(_T(GetString(IDS_PW_SPLINEMAP)));

					  mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
					  mod->InvalidateView();
					  if (ip) ip->RedrawViews(ip->GetTime());

				  }
				  break;

			}
		  break;

	  default:
		  return FALSE;
	}
	return TRUE;
}





BOOL PickSplineNode::Filter(INode *node)
{
	node->BeginDependencyTest();
	mod->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) 
	{		
		return FALSE;
	} 

	else 
	{
		TimeValue t = GetCOREInterface()->GetTime();
		ObjectState os = node->EvalWorldState(t);
		if (os.obj->SuperClassID()==SHAPE_CLASS_ID)
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL PickSplineNode::HitTest(
							  IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
{	
	if (ip->PickNode(hWnd,m,this)) 
	{
		mLastHitPoint = m;
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOL PickSplineNode::Pick(IObjParam *ip,ViewExp *vpt)
{
	INode *node = vpt->GetClosestHit();
	if (node) 
	{

		//figure out which spline element we hit
		TimeValue t = ip->GetTime();
		ObjectState os = node->EvalWorldState(t);
		if (os.obj->IsShapeObject()) 
		{
			ShapeObject *pathOb = (ShapeObject*)os.obj;
			int numberOfCurves = pathOb->NumberOfCurves();
			//get number of splines
			if (numberOfCurves != 0) 
			{
				PolyShape shapeCache;
				pathOb->MakePolyShape(t, shapeCache,32,FALSE);
				//hit test
				ViewExp *vpt = GetCOREInterface()->GetActiveViewport();
				Matrix3 tm = node->GetObjectTM(t);
				if (vpt)
				{
					GraphicsWindow *gw = vpt->getGW();


					

					DWORD limit = gw->getRndLimits();
					gw->setTransform(tm);

					int crossing = TRUE;
					int type = HITTYPE_POINT;
					HitRegion hr;
					MakeHitRegion(hr,type, crossing,4,&mLastHitPoint);	
					gw->setHitRegion(&hr);

					gw->setRndLimits(( limit | GW_PICK) & ~GW_ILLUM);

					
					Point3 l[3];


					for (int i = 0; i < numberOfCurves; i++)
					{
						PolyLine *line = &shapeCache.lines[i];
						int numPts = line->numPts;
						if (line->IsClosed())
							numPts+=1;
						gw->startSegments();
						for (int j = 0; j < numPts; j++)
						{
							int a = j;
							int b = (j+1)%line->numPts;

							l[0] = line->pts[a].p;
							l[1] = line->pts[b].p;

							gw->clearHitCode();
							gw->segment(l,1);
							if (gw->checkHitCode())
							{
								//sel spline = i;
								mod->fnSplineMap_SectActiveSpline(i);
								i = numberOfCurves;
								j = numPts; 
							}								
						}
						gw->endSegments();
						
					}
					gw->setRndLimits(limit);
					GetCOREInterface()->ReleaseViewport(vpt);
				}
			}
		}

		theHold.Begin();		
		mod->pblock->SetValue(unwrap_splinemap_node,t,node);
		
//		mod->SetSplineMappingNode(node);
		mod->fnSplineMap_UpdateUI();
		mod->fnSplineMap();

		TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap6.splineMap"));
		macroRecorder->FunctionCall(mstr, 0, 0);

		theHold.Accept(_T(GetString(IDS_PW_SPLINEMAP)));
		return TRUE;
	}
	return FALSE;
}

void PickSplineNode::EnterMode(IObjParam *ip)
{

	HWND hWND = mod->fnSplineMap_GetHWND();
	ICustButton *iButton = GetICustButton(GetDlgItem(hWND, IDC_PICKSPLINE_BUTTON));
	if (iButton)
	{	
		iButton->SetType(CBT_CHECK);
		iButton->SetCheck(TRUE);
		ReleaseICustButton(iButton);
	}
	

}

void PickSplineNode::ExitMode(IObjParam *ip)
{
	HWND hWND = mod->fnSplineMap_GetHWND();
	ICustButton *iButton = GetICustButton(GetDlgItem(hWND, IDC_PICKSPLINE_BUTTON));
	if (iButton)
	{	
		iButton->SetType(CBT_CHECK);
		iButton->SetCheck(FALSE);
		ReleaseICustButton(iButton);
	}
	
}

HCURSOR PickSplineNode::GetDefCursor(IObjParam *ip)
{
	return LoadCursor(NULL,IDC_ARROW);
}

HCURSOR PickSplineNode::GetHitCursor(IObjParam *ip)
{
	return mod->selCur;
}



