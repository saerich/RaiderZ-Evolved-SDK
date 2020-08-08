
/*

Copyright [2008] Autodesk, Inc.  All rights reserved.

Use of this software is subject to the terms of the Autodesk license agreement 
provided at the time of installation or download, or which otherwise accompanies 
this software in either electronic or hard copy form.   

*/

#include "unwrap.h"


MeshTopoData *TweakMouseProc::HitTest(ViewExp *vpt, IPoint2 m, int &hitVert)
{
	int savedLimits;
	TimeValue t = GetCOREInterface()->GetTime();

	GraphicsWindow *gw = vpt->getGW();

	HitRegion hr;

	int crossing = TRUE;
	int type = HITTYPE_POINT;
	MakeHitRegion(hr,type, crossing,8,&m);
	gw->setHitRegion(&hr);
	hitVert = -1;
	MeshTopoData *hitLD = NULL;
	mHitLDIndex = -1;

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
		{
			hitVert = hindex;
			hitLD = ld;
			mHitLDIndex = ldID;
		}
	}

	gw->setRndLimits(savedLimits);

	return hitLD;

}

void TweakMouseProc::AverageUVW()
{
	if (mHitLD)
	{
		Point3 uvw(0.0f,0.0f,0.0f);
		int ct = 0;
		for (int i = 0; i < mHitLD->GetNumberTVEdges(); i++ )
		{
			if (!mHitLD->GetTVEdgeHidden(i))
			{
				if (mHitLD->GetTVEdgeVert(i,0) == mHitTVVert)
				{
					uvw += mHitLD->GetTVVert(mHitLD->GetTVEdgeVert(i,1));
					ct++;
				}
				else if (mHitLD->GetTVEdgeVert(i,1) == mHitTVVert)
				{
					uvw += mHitLD->GetTVVert(mHitLD->GetTVEdgeVert(i,0));
					ct++;
				}
			}
		}
		if (ct > 0)
		{
			uvw = uvw/(float)ct;

			mFinalUVW = uvw;
			mHitLD->SetTVVert(GetCOREInterface()->GetTime(),mHitTVVert,mFinalUVW,mod);

			mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
			mod->InvalidateView();
			if (mod->ip) 
				mod->ip->RedrawViews(mod->ip->GetTime());
		}
	}
}
void TweakMouseProc::ComputeNewUVW(ViewExp *vpt, IPoint2 m)
{
	float at = 0;
	Ray r;
	vpt->MapScreenToWorldRay ((float) m.x, (float) m.y, r);
	TimeValue t = GetCOREInterface()->GetTime();
	Matrix3 tm = mod->GetMeshTopoDataNode(mHitLDIndex)->GetObjectTM(t);
	Matrix3 itm = Inverse(tm);

	r.p = r.p * itm;
	r.dir = VectorTransform(r.dir,itm);

	//intersect our ray with that face and get our new bary coords

	int deg = mHitLD->GetFaceDegree(mHitFace);
	Point3 n = mHitLD->GetGeomFaceNormal(mHitFace);


	Point3 p1, p2, p3;


	p1 = mHitP[0];
	p2 = mHitP[1];
	p3 = mHitP[2];

	Point3  p, bry;
	float d, rn, a; 

	// See if the ray intersects the plane (backfaced)
	rn = DotProd(r.dir,n);
	if (rn > -0.0001) return;

	// Use a point on the plane to find d
	Point3 v1 = p1;
	d = DotProd(v1,n);

	// Find the point on the ray that intersects the plane
	a = (d - DotProd(r.p,n)) / rn;

	// Must be positive...
	if (a < 0.0f) return ;



	// The point on the ray and in the plane.
	p = r.p + a*r.dir;

	// Compute barycentric coords.
	bry = BaryCoords(p1,p2,p3,p);  // MaxAssert(bry.x + bry.y+ bry.z = 1.0) 

	Point3 uvw = mHitUVW[0] * bry.x + mHitUVW[1] * bry.y + mHitUVW[2] * bry.z;
	Point3 v = uvw - mSourceUVW;
	v *= -1.0f;
	mFinalUVW = mSourceUVW + v;
	mHitLD->SetTVVert(GetCOREInterface()->GetTime(),mHitTVVert,mFinalUVW,mod);

	mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	mod->InvalidateView();
	if (mod->ip) 
		mod->ip->RedrawViews(mod->ip->GetTime());

}

int TweakMouseProc::proc(
									HWND hwnd, 
									int msg, 
									int point, 
									int flags, 
									IPoint2 m )
{
	int res = TRUE;

	ViewExp *vpt = iObjParams->GetViewport(hwnd);   
	
	
	

	static IPoint2 cancelPoints[3];
	static HWND cancelHWND;


	switch ( msg ) 
	{
	case MOUSE_PROPCLICK:
		//reset start point

//kill the command mode
		SetCursor(LoadCursor(NULL,IDC_ARROW));
		GetCOREInterface()->SetStdCommandMode(CID_OBJSELECT);
		mHitLD = NULL;
		break;

	case MOUSE_ABORT:
		{
			theHold.Cancel();

			if (mHitLD)
			{
				mHitLD->SetTVVert(GetCOREInterface()->GetTime(),mHitTVVert,mSourceUVW,mod);

				mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
				mod->InvalidateView();
				if (mod->ip) 
					mod->ip->RedrawViews(mod->ip->GetTime());
			}
			break;
		}
	case MOUSE_POINT:
		{
//see if we hit a point
			if (point == 0)
			{		
				theHold.Begin();
				mod->HoldPoints();

				mHitLD = HitTest(vpt, m, mHitVert);
				if (mHitLD != NULL)
				{
					float at = 0;
					Ray r;
					vpt->MapScreenToWorldRay ((float) m.x, (float) m.y, r);
					TimeValue t = GetCOREInterface()->GetTime();
					Matrix3 tm = mod->GetMeshTopoDataNode(mHitLDIndex)->GetObjectTM(t);
					Matrix3 itm = Inverse(tm);

					r.p = r.p * itm;
					r.dir = VectorTransform(r.dir,itm);

					mHitLD->Intersect(r,true,false, at, mBary, mHitFace);
					if (mHitFace != -1)
					{
						int deg = mHitLD->GetFaceDegree(mHitFace);
						for (int j = 0; j < deg; j++)
						{
							if (mHitVert == mHitLD->GetFaceGeomVert(mHitFace,j))
							{
								mHitTVVert = mHitLD->GetFaceTVVert(mHitFace,j);
								mSourceUVW =  mHitLD->GetTVVert(mHitTVVert);
								int offset1 = j;
								int offset2 = j+1;
								if (j == 0)
								{
									offset1 = 1;
									offset2 = 2;
								}
								else if (j == deg-1)
								{
									offset1 = deg-1;
									offset2 = deg-2;			
								}
								mHitP[0] = mHitLD->GetGeomVert(mHitLD->GetFaceGeomVert(mHitFace,0));
								mHitP[1] = mHitLD->GetGeomVert(mHitLD->GetFaceGeomVert(mHitFace,offset1));
								mHitP[2] = mHitLD->GetGeomVert(mHitLD->GetFaceGeomVert(mHitFace,offset2));								

								mHitUVW[0] = mHitLD->GetTVVert(mHitLD->GetFaceTVVert(mHitFace,0));
								mHitUVW[1] = mHitLD->GetTVVert(mHitLD->GetFaceTVVert(mHitFace,offset1));
								mHitUVW[2] = mHitLD->GetTVVert(mHitLD->GetFaceTVVert(mHitFace,offset2));								

							}															
						}
					}
					else
					{
						mHitLD->Intersect(r,true,false, at, mBary, mHitFace);
						mHitLD = NULL;
					}
				}
			}
			else
			{
				//accept the undo
				if (mHitLD)
				{
					if (flags & MOUSE_SHIFT)
					{
						AverageUVW();
					}
					else
					{
						ComputeNewUVW(vpt, m);
					}	

					mHitLD = NULL;
					TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.setVertexPositionByNode"));
					macroRecorder->FunctionCall(mstr, 4, 0,	
								mr_time,GetCOREInterface()->GetTime(),
								mr_int,mHitTVVert+1,
								mr_point3, mFinalUVW,
								mr_reftarg,	mod->GetMeshTopoDataNode(mHitLDIndex)	);						
				}
				theHold.Accept(GetString (IDS_DS_MOVE2));
			}

		break;
		}
	case MOUSE_MOVE:
		{
//if we hit a point get the transform and compute the uv offset
			if (mHitLD)
			{
				if (flags & MOUSE_SHIFT)
				{
					AverageUVW();
				}
				else
				{
					ComputeNewUVW(vpt, m);
				}				
			}
		break;
		}		
	case MOUSE_FREEMOVE:
		{
	
		//see if hit and set cursor
		//hit test point
		int index = -1;
		
		float at = 0;
		Ray r;
		vpt->MapScreenToWorldRay ((float) m.x, (float) m.y, r);
		TimeValue t = GetCOREInterface()->GetTime();
		BOOL hitFace = FALSE;

		for (int ldID = 0; ldID < mod->GetMeshTopoDataCount(); ldID++)
		{
			Matrix3 tm = mod->GetMeshTopoDataNode(ldID)->GetObjectTM(t);
			Matrix3 itm = Inverse(tm);

			r.p = r.p * itm;
			r.dir = VectorTransform(r.dir,itm);

			MeshTopoData *ld = mod->GetMeshTopoData(ldID);
			Point3 bry;
			int hit = -1;
			if (ld)
			{
				if (ld->Intersect(r,true,false, at, bry, hit))
					hitFace = TRUE;
			}
		}


		if ((HitTest(vpt, m, index)!= NULL) && hitFace)
			SetCursor(GetCOREInterface()->GetSysCursor(SYSCUR_SELECT));
		else SetCursor(LoadCursor(NULL,IDC_ARROW));
		
		break;
		}
	}


	if ( vpt ) 
		iObjParams->ReleaseViewport(vpt);

	return res;

}



/*-------------------------------------------------------------------*/

void TweakMode::EnterMode()
{
	ICustButton *iTweakButton = GetICustButton(GetDlgItem(mod->hMapParams, IDC_TWEAKUVW));
	if (iTweakButton)
	{
		iTweakButton->SetCheck(TRUE);
		ReleaseICustButton(iTweakButton);

	}


	for (int ldID = 0; ldID < mod->GetMeshTopoDataCount(); ldID++)
	{
		MeshTopoData *ld = mod->GetMeshTopoData(ldID);
		ld->PreIntersect();
	}

}

void TweakMode::ExitMode()
{
	
	ICustButton *iTweakButton = GetICustButton(GetDlgItem(mod->hMapParams, IDC_TWEAKUVW));
	if (iTweakButton)
	{
		iTweakButton->SetCheck(FALSE);
		ReleaseICustButton(iTweakButton);
	}

	for (int ldID = 0; ldID < mod->GetMeshTopoDataCount(); ldID++)
	{
		MeshTopoData *ld = mod->GetMeshTopoData(ldID);
		ld->PostIntersect();
	}
}
