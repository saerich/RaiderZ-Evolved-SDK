#include "unwrap.h"



/*
static void LagXORDottedLine( HWND hwnd, IPoint2 p0, IPoint2 p1 )
	{
	HDC hdc;
	hdc = GetDC( hwnd );
	SetROP2( hdc, R2_XORPEN );
	SetBkMode( hdc, TRANSPARENT );
	SelectObject( hdc, CreatePen( PS_DOT, 0, ComputeViewportXORDrawColor() ) );
	MoveToEx( hdc, p0.x, p0.y, NULL );
	LineTo( hdc, p1.x, p1.y );		
	DeleteObject( SelectObject( hdc, GetStockObject( BLACK_PEN ) ) );
	ReleaseDC( hwnd, hdc );
	}
*/

int PeltPointToPointMouseProc::proc(
                        HWND hwnd, 
                        int msg, 
                        int point, 
                        int flags, 
                        IPoint2 m )
{
	ViewExp *vpt = iObjParams->GetViewport(hwnd);   
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;
	int res = TRUE;

	static IPoint2 cancelPoints[3];
	static HWND cancelHWND;

	switch ( msg ) 
	{
		case MOUSE_PROPCLICK:
			//reset start point
			if (mod->peltData.currentPointHit != -1) 
			{


				XORDottedPolyline(cancelHWND, 2, cancelPoints);

			}
			else mod->peltData.SetPointToPointSeamsMode(mod,FALSE);
			mod->peltData.currentPointHit = -1;
			break;

		case MOUSE_POINT:
			{
			mod->peltData.basep = m;
			//hit test point
			int index = -1;
			int savedLimits;
			TimeValue t = GetCOREInterface()->GetTime();


			int crossing = TRUE;
			int type = HITTYPE_POINT;
			MakeHitRegion(hr,type, crossing,4,&m);
			savedLimits = gw->getRndLimits();
			gw->setHitRegion(&hr);



			MeshTopoData *hitLD = NULL;
			Matrix3 hitTM;
			for (int ldID = 0; ldID < mod->GetMeshTopoDataCount(); ldID++)
			{			
				MeshTopoData *ld = mod->GetMeshTopoData(ldID);

				INode *inode = mod->GetMeshTopoDataNode(ldID);
				Matrix3 mat = inode->GetObjectTM(t);
				gw->setTransform(mat);	
				gw->setRndLimits((gw->getRndLimits() | GW_PICK) & ~GW_ILLUM);
				gw->setRndLimits(gw->getRndLimits() |  GW_BACKCULL);
				ModContext mc;

				int hindex = mod->peltData.HitTestPointToPointMode(mod,ld,vpt,gw,&m,hr,inode,&mc);
				if (hindex != -1)
				{
					index = hindex;
					hitLD = ld;
					hitTM = mat;
				}
			}
			//do selection
		//get the hit point

			if (index != -1)
			{

				Point3 previousAnchorPoint = mod->peltData.pointToPointAnchorPoint;
				mod->peltData.mBaseMeshTopoDataPrevious = mod->peltData.mBaseMeshTopoDataCurrent;
				mod->peltData.mBaseMeshTopoDataCurrent = hitLD;

				
				
				mod->peltData.pointToPointAnchorPoint = hitLD->GetGeomVert(index) * hitTM;//mod->TVMaps.geomPoints[index] * mat;

				if (hitLD->mSeamEdges.GetSize() != hitLD->GetNumberGeomEdges())//mod->TVMaps.gePtrList.Count())
				{
					hitLD->mSeamEdges.SetSize(mod->peltData.mBaseMeshTopoDataCurrent->GetNumberGeomEdges());//mod->TVMaps.gePtrList.Count());
					hitLD->mSeamEdges.ClearAll();
				}

				

				if (index != mod->peltData.currentPointHit)
				{
					//assign the current to previous
					mod->peltData.previousPointHit = mod->peltData.currentPointHit;
					//assign hit point to current
					mod->peltData.currentPointHit = index;
					Point3 vec = mod->peltData.viewZVec;

					mod->peltData.lastMouseClick = mod->peltData.currentMouseClick;
					mod->peltData.currentMouseClick = m;

					//if we have both a current and previous create seam
					if ( ((mod->peltData.previousPointHit!=-1) && (mod->peltData.currentPointHit!=-1)) && 
						((mod->peltData.previousPointHit!=mod->peltData.currentPointHit)) 
						&& (mod->peltData.mBaseMeshTopoDataPrevious == mod->peltData.mBaseMeshTopoDataCurrent)
						)
					{



						theHold.Begin();
						theHold.Put (new UnwrapPeltSeamRestore (mod,hitLD,&previousAnchorPoint));
						theHold.Accept (GetString (IDS_DS_SELECT));

						Tab<int> newSeamEdges;
						hitLD->EdgeListFromPoints(newSeamEdges, mod->peltData.previousPointHit, mod->peltData.currentPointHit,vec);
						for (int i = 0; i < newSeamEdges.Count(); i++)
						{
							int edgeIndex = newSeamEdges[i];
							hitLD->mSeamEdges.Set(edgeIndex,TRUE);

						}

						mod->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
						mod->ip->RedrawViews(t);
						mod->peltData.previousPointHit = -1;
						mod->peltData.currentPointHit = -1;
					}
					
						
				}

				INode *node = NULL;
				for (int ldID = 0; ldID < mod->GetMeshTopoDataCount(); ldID++)
				{
					if (hitLD == mod->GetMeshTopoData(ldID))
						node = mod->GetMeshTopoDataNode(ldID);
				}
			
				TSTR mstr;
				mstr.printf("$%s.modifiers[#unwrap_uvw].unwrap6.setPeltSelectedSeamsByNode",node->GetName());
				macroRecorder->FunctionCall(mstr, 2, 0,
					mr_bitarray,&(hitLD->mSeamEdges),
					mr_reftarg,node
					);
			}

			break;
			}
		case MOUSE_MOVE:
			break;
		case MOUSE_FREEMOVE:
			//see if hit and set cursor
			//hit test point
			int index = -1;
			int savedLimits;
			TimeValue t = GetCOREInterface()->GetTime();

/*
			ModContextList mcList;		
			INodeTab nodes;

			GetCOREInterface()->GetModContexts(mcList,nodes);

			int objects = mcList.Count();
			if (objects == 0) return 0;

			INode *inode = nodes[0];
*/
			int crossing = TRUE;
			int type = HITTYPE_POINT;
			MakeHitRegion(hr,type, crossing,4,&m);
			gw->setHitRegion(&hr);

//			else gw->setRndLimits(gw->getRndLimits() & ~GW_BACKCULL);

			savedLimits = gw->getRndLimits();
			for (int ldID = 0; ldID < mod->GetMeshTopoDataCount(); ldID++)
			{
				MeshTopoData *ld = mod->GetMeshTopoData(ldID);
				INode *inode = mod->GetMeshTopoDataNode(ldID);

				Matrix3 mat = inode->GetObjectTM(t);
				gw->setTransform(mat);	
				gw->setRndLimits(((gw->getRndLimits() | GW_PICK) & ~GW_ILLUM) |  GW_BACKCULL);
				ModContext mc;

				int hindex = mod->peltData.HitTestPointToPointMode(mod,ld,vpt,gw,&m,hr,inode,&mc);
				if (hindex != -1)
					index = hindex;
			}

			if (index != -1)
				SetCursor(GetCOREInterface()->GetSysCursor(SYSCUR_SELECT));
			else SetCursor(LoadCursor(NULL,IDC_ARROW));
			//draw the xor line



			if (mod->peltData.currentPointHit == -1) 
			{
				mod->peltData.basep = m;
			}
			mod->peltData.pp = mod->peltData.cp;
			mod->peltData.cp = m;
	//xor our last line
	//draw our new one

			static IPoint2 lastPoint(0,0);
			static HWND lastHWND;



			Point3 anchorScreen;

			GraphicsWindow *gw = vpt->getGW();	
			gw->setTransform(Matrix3(1));
			gw->transPoint(&mod->peltData.pointToPointAnchorPoint,&anchorScreen);


			IPoint2 points[3];
			points[0].x = (int) anchorScreen.x;
			points[0].y = (int) anchorScreen.y;

			HWND hWnd = gw->getHWnd();
			if (mod->peltData.currentPointHit != -1) 
			{

				

				//				points[0] = mod->peltData.basep;
				if (lastPoint == points[0])
				{
					points[1] = mod->peltData.pp;
					XORDottedPolyline(hWnd, 2, points);

				}
				if (hWnd != lastHWND)
				{

					IPoint2 tpoints[3];
					tpoints[0] = lastPoint;
					tpoints[1] = mod->peltData.pp;
					XORDottedPolyline(lastHWND, 2, tpoints);


				}

				//				points[0] = mod->peltData.basep;
				points[1] = mod->peltData.cp;
				XORDottedPolyline(hWnd, 2, points);
				cancelHWND = hWnd;
				cancelPoints[0] = points[0];
				cancelPoints[1] = points[1];


			}
			
			lastPoint = points[0];

			lastHWND = hWnd;
			

			break;
	}


	if ( vpt ) 
		iObjParams->ReleaseViewport(vpt);

	return res;

}



/*-------------------------------------------------------------------*/

void PeltPointToPointMode::EnterMode()
{
	ICustButton *iEditSeamsButton = GetICustButton(GetDlgItem(mod->hMapParams, IDC_UNWRAP_EDITSEAMS));
 	ICustButton *iEditSeamsByPointButton = GetICustButton(GetDlgItem(mod->hMapParams, IDC_UNWRAP_SEAMPOINTTOPOINT));

	if (iEditSeamsButton)
	{
		iEditSeamsButton->SetCheck(FALSE);
		ReleaseICustButton(iEditSeamsButton);
	}
	if (iEditSeamsByPointButton)
	{
		iEditSeamsByPointButton->SetCheck(TRUE);
		ReleaseICustButton(iEditSeamsByPointButton);
	}
}

void PeltPointToPointMode::ExitMode()
{
	ICustButton *iEditSeamsButton = GetICustButton(GetDlgItem(mod->hMapParams, IDC_UNWRAP_EDITSEAMS));
 	ICustButton *iEditSeamsByPointButton = GetICustButton(GetDlgItem(mod->hMapParams, IDC_UNWRAP_SEAMPOINTTOPOINT));

	if (iEditSeamsButton)
	{
		iEditSeamsButton->SetCheck(FALSE);
		ReleaseICustButton(iEditSeamsButton);
	}
	if (iEditSeamsByPointButton)
	{
		iEditSeamsByPointButton->SetCheck(FALSE);
		ReleaseICustButton(iEditSeamsByPointButton);
	}	
//	mod->fnSetPeltPointToPointSeamsMode(FALSE);
}
