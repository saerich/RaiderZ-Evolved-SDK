/********************************************************************** *<
FILE: ToolSplineMapping_unwrapFunctions.cpp

DESCRIPTION: This contains all our spline map functions that are in 
Unwrap


HISTORY: 12/31/96
CREATED BY: Peter Watje


*>	Copyright (c) 2007, All Rights Reserved.
**********************************************************************/


#include "unwrap.h"

void UnwrapMod::fnSplineMap_UpdateUI()
{
	HWND hSplineMapWND = fnSplineMap_GetHWND();
	if (hSplineMapWND)
	{
		TimeValue t = GetCOREInterface()->GetTime();
		//update proj type
		int val = 0;
		pblock->GetValue(unwrap_splinemap_projectiontype,t,val,FOREVER);
		HWND hType = GetDlgItem(hSplineMapWND,IDC_PROJ_COMBO);
		SendMessage(hType, CB_SETCURSEL, val, 0L);


		//update manual seams		
		BOOL manualSeams = FALSE;
		pblock->GetValue(unwrap_splinemap_manualseams,t,manualSeams,FOREVER);
		SetCheckBox(hSplineMapWND,IDC_MANUALSEAMS_CHECK,manualSeams);
		//deactivate manual seams if proj type is planar
		if (val == 0)
			EnableWindow(GetDlgItem(hSplineMapWND,IDC_MANUALSEAMS_CHECK),TRUE);
		else
			EnableWindow(GetDlgItem(hSplineMapWND,IDC_MANUALSEAMS_CHECK),FALSE);


		INode *node;
		pblock->GetValue(unwrap_splinemap_node,t,node,FOREVER);
		//update spline name
		HWND hText = GetDlgItem(hSplineMapWND,IDC_NODENAME);
		if (node)
		{			
			SetWindowText(hText,node->GetName());
		}
		else
		{
			TSTR blank;
			blank.printf("-");
			SetWindowText(hText,blank);
		}

		//update the add remove faces buttons since these can only be active when one spline is active
		Tab<int> selSplines;
		mSplineMap.GetSelectedSplines(selSplines);

		BOOL enableState = FALSE;
		if (selSplines.Count() == 1) 
			enableState = TRUE;

		EnableWindow(GetDlgItem(hSplineMapWND,IDC_ADDFACES_BUTTON),enableState);
		EnableWindow(GetDlgItem(hSplineMapWND,IDC_REMOVEFACES_BUTTON),enableState);


		//update the command mode buttons
		int currentMode = fnSplineMap_GetMode();
		ICustButton *iButton = GetICustButton(GetDlgItem(hSplineMapWND, IDC_ADD_BUTTON));
		if (iButton)
		{	
			iButton->SetType(CBT_CHECK);
			if (currentMode == SplineData::kAddMode)
				iButton->SetCheck(TRUE);
			else
				iButton->SetCheck(FALSE);
			ReleaseICustButton(iButton);
		}	

		iButton = GetICustButton(GetDlgItem(hSplineMapWND, IDC_ALIGNSECTION_BUTTON));
		if (iButton)
		{	
			iButton->SetType(CBT_CHECK);
			if (currentMode == SplineData::kAlignSectionMode)
				iButton->SetCheck(TRUE);
			else
				iButton->SetCheck(FALSE);
			ReleaseICustButton(iButton);
		}	

		iButton = GetICustButton(GetDlgItem(hSplineMapWND, IDC_ALIGNFACE_BUTTON));
		if (iButton)
		{	
			iButton->SetType(CBT_CHECK);
			if (currentMode == SplineData::kAlignFaceMode)
				iButton->SetCheck(TRUE);
			else
				iButton->SetCheck(FALSE);
			ReleaseICustButton(iButton);
		}	



	}
}

HWND UnwrapMod::fnSplineMap_GetHWND()
{
	return mSplineMap.GetHWND();
}

void UnwrapMod::fnSplineMap_StartMapMode()
{

//keep a record of the state before operation so we can cancel it
	HoldCancelBuffer();

	if (GetIsMeshUp())
	{
		mSplineMap.SetPlanarMapDirection(TRUE);
	}
	else
	{
		mSplineMap.SetPlanarMapDirection(FALSE);
	}


	INode *node;
	TimeValue t = GetCOREInterface()->GetTime();

	pblock->GetValue(unwrap_splinemap_node,t,node,FOREVER);
//	see if we have a node
	if (node)
	{
		//see if the spline count has changed		
		Matrix3 tm = node->GetObjectTM(t);
		Matrix3 iTM = Inverse(tm);
		ObjectState nos = node->EvalWorldState(t);
		if (nos.obj->IsShapeObject()) 
		{
			ShapeObject *pathOb = (ShapeObject*)nos.obj;
			int numberOfCurves = pathOb->NumberOfCurves();
			//check if the spline topo changed
			BOOL curveCountChanged = numberOfCurves != mSplineMap.NumberOfSplines();
			if (!curveCountChanged)
			{
			//just update what we have
				UpdateSplineMappingNode(node);
			}
			else
			{
			//recompute from start
				SetSplineMappingNode(node);				
			}
			theHold.Begin();
			fnSplineMap();
			theHold.Accept(_T(GetString(IDS_PW_SPLINEMAP)));
		}
	}


	//bring up the dialog
	HWND parentHWND = hWnd;
	if (parentHWND == NULL)
		parentHWND = GetCOREInterface()->GetMAXHWnd();
	mSplineMap.SetHWND(CreateDialogParam(	hInstance,
		MAKEINTRESOURCE(IDD_UNWRAP_SPLINEMAPDIALOG),
		parentHWND,
		UnwrapSplineMapFloaterDlgProc,
		(LPARAM)this ));
	ShowWindow(mSplineMap.GetHWND() ,SW_SHOW);

	fnSplineMap_UpdateUI();

}
void UnwrapMod::fnSplineMap_EndMapMode()
{
	//bring down the dialog
	if (mSplineMap.GetHWND())
	{
		DestroyWindow(mSplineMap.GetHWND());
		GetCOREInterface()->SetStdCommandMode(CID_OBJMOVE);
	}
	mSplineMap.SetHWND(NULL);

	//delete our cancel buffer
	FreeCancelBuffer();
}

void UnwrapMod::fnSplineMap_Cancel()
{
	//restore the original mapping
	RestoreCancelBuffer();
	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());
	if (hView) InvalidateView();
}


void UnwrapMod::fnSplineMap_InsertCrossSection(int splineIndex, float u)
{
	theHold.Begin();
	mSplineMap.HoldData();
	mSplineMap.InsertCrossSection(splineIndex,u);
	fnSplineMap();
	theHold.Accept(_T(GetString(IDS_PW_SPLINEMAP)));

	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());
	if (hView) InvalidateView();
}

BOOL UnwrapMod::fnSplineMap_HitTestCrossSection(GraphicsWindow *gw, HitRegion hr, SplineMapProjectionTypes projType,  Tab<int> &hitSplines, Tab<int> &hitCrossSections)
{
	return mSplineMap.HitTestCrossSection(gw,hr,  projType,  hitSplines, hitCrossSections);
}
BOOL UnwrapMod::fnSplineMap_HitTestSpline(GraphicsWindow *gw, HitRegion *hr, int &splineIndex, float &u)
{
	return mSplineMap.HitTestSpline(gw,hr,splineIndex,u);
}

void UnwrapMod::fnSplineMap_DeleteSelectedCrossSections()
{

	theHold.Begin();
	mSplineMap.HoldData();
	mSplineMap.DeleteSelectedCrossSection();

	fnSplineMap();
	theHold.Accept(_T(GetString(IDS_PW_SPLINEMAP)));

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());
	if (hView) InvalidateView();



}
void UnwrapMod::fnSplineMap_Copy()
{
	mSplineMap.Copy();
}
void UnwrapMod::fnSplineMap_Paste()
{	
	theHold.Begin();
	mSplineMap.HoldData();
	mSplineMap.Paste();
	fnSplineMap();
	theHold.Accept(_T(GetString(IDS_PW_SPLINEMAP)));

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());
	if (hView) InvalidateView();
}

void UnwrapMod::fnSplineMap_PasteToSelected(int splineIndex, int crossSectionIndex)
{
	theHold.Begin();
	mSplineMap.HoldData();
	mSplineMap.PasteToSelected(splineIndex,crossSectionIndex);
	fnSplineMap();
	theHold.Accept(_T(GetString(IDS_PW_SPLINEMAP)));

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());
	if (hView) InvalidateView();
}
void UnwrapMod::fnSplineMap_AddCrossSectionCommandMode()
{
	mSplineMap.AddCrossSectionCommandMode();
}

void UnwrapMod::fnSplineMap_AlignFaceCommandMode()
{
	mSplineMap.AlignCommandMode();
}

void UnwrapMod::fnSplineMap_AlignSectionCommandMode()
{
	mSplineMap.AlignSectionCommandMode();
}


int UnwrapMod::fnSplineMap_GetMode()
{
	return mSplineMap.CurrentMode();
}
void UnwrapMod::fnSplineMap_SetMode(int mode)
{
	mSplineMap.SetCurrentMode(mode);
}
void UnwrapMod::fnSplineMap_Align(int splineIndex, int crossSectionIndex, Point3 vec)
{
	mSplineMap.AlignCrossSection(splineIndex,crossSectionIndex,vec);
	mSplineMap.RecomputeCrossSections();
}

void UnwrapMod::fnSplineMap_AlignSelected(Point3 vec)
{
	theHold.Begin();
	mSplineMap.HoldData();

	int numberSplines = mSplineMap.NumberOfSplines();

	for (int splineIndex = 0; splineIndex < numberSplines; splineIndex++)
	{
		if (mSplineMap.IsSplineSelected(splineIndex))
		{
			int numberCrossSections = mSplineMap.NumberOfCrossSections(splineIndex);
			for (int crossSectionIndex = 0; crossSectionIndex < numberCrossSections; crossSectionIndex++)
			{
				SplineCrossSection *section = mSplineMap.GetCrossSection(splineIndex,crossSectionIndex);

				if (section->mIsSelected)
					mSplineMap.AlignCrossSection(splineIndex,crossSectionIndex,vec);
			}
		}
	}
	mSplineMap.RecomputeCrossSections();

	fnSplineMap();
	theHold.Accept(_T(GetString(IDS_PW_SPLINEMAP)));

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());
	if (hView) InvalidateView();
}


void UnwrapMod::fnSplineMap_Fit(BOOL fitAll, float extraScale)
{
	theHold.Begin();
	mSplineMap.HoldData();

	TimeValue t = GetCOREInterface()->GetTime();

	int numSplines = mSplineMap.NumberOfSplines();

	SplineMapProjectionTypes projType = kCircular;
	int val = 0;
	pblock->GetValue(unwrap_splinemap_projectiontype,t,val,FOREVER);
	if (val == 1)
		projType = kPlanar;


	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		ld->PreIntersect();
	}

	float minScale = 0.0f;
	Box3 splineBounds = mSplineMap.GetWorldsBounds();
	minScale = Length(splineBounds.pmax-splineBounds.pmin);
	minScale = minScale * 0.001f;

	//loop through the splines
	for (int splineIndex = 0; splineIndex < numSplines; splineIndex++)
	{
		int numCrossSections = mSplineMap.NumberOfCrossSections(splineIndex);
		//loop through the cross scections
		BitArray hitCrossSection;
		hitCrossSection.SetSize(numCrossSections);
		hitCrossSection.ClearAll();
		
		for (int crossSectionIndex = 0; crossSectionIndex < numCrossSections; crossSectionIndex++)
		{
			SplineCrossSection *crossSection = mSplineMap.GetCrossSection(splineIndex,crossSectionIndex);
			//see if selected or fit all
			if (crossSection->mIsSelected || fitAll)
			{
				//get the cross Section TM
				Matrix3 tm = crossSection->mTM;
				Point3 p, rx,ry;
				p =  tm.GetRow(3);
				rx =  tm.GetRow(0);
				ry =  tm.GetRow(1);

				float xScaleA = -1.0f;
				float yScaleA = -1.0f;

				float xScaleB = -1.0f;
				float yScaleB = -1.0f;

				for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
				{
					//transform into mesh space
					MeshTopoData *ld = mMeshTopoData[ldID];
					Matrix3 meshTM = mMeshTopoData.GetNodeTM(t,ldID); 
					Matrix3 imeshTM = Inverse(meshTM);
					Point3 t_p = p * imeshTM;
					Point3 t_rx = VectorTransform(rx , imeshTM);
					Point3 t_ry = VectorTransform(ry , imeshTM);

					Ray rx_Pos;
					Ray rx_Neg;
					Ray ry_Pos, ry_Neg;
					rx_Pos.p = t_p;
					rx_Pos.dir = Normalize(t_rx);

					rx_Neg.p = t_p;
					rx_Neg.dir = Normalize(t_rx) * -1.0f;

					ry_Pos.p = t_p;
					ry_Pos.dir = Normalize(t_ry);

					ry_Neg.p = t_p;
					ry_Neg.dir = Normalize(t_ry) * -1.0f;

					//get closest intersection with the mesh
					float at = 1.0f;

					if (ld->Intersect(rx_Pos,true, true, at))
					{					
						if ((xScaleA == -1.0f) || (at < xScaleA))
							xScaleA = at;
					}
					if (ld->Intersect(rx_Neg,true, true, at))
					{					
						if ((xScaleB == -1.0f) || (at < xScaleB))
							xScaleB = at;
					}

					if (ld->Intersect(ry_Pos,true, true, at))
					{					
						if ((yScaleA == -1.0f) || (at < yScaleA))
							yScaleA = at;
					}
					if (ld->Intersect(ry_Neg,true, true, at))
					{					
						if ((yScaleB == -1.0f) || (at < yScaleB))
							yScaleB = at;
					}
				}
				//set that as our spline cross section scale

				if ( (xScaleA > minScale) && 
					(xScaleB > minScale) )
				{
					float xScale = xScaleA;
					if (xScaleB < xScale)
						xScale = xScaleB;
					crossSection->mScale.x = xScale * extraScale;

				}
				if  ((yScaleA > minScale) && 
					(yScaleB > minScale) )
				{

					float yScale = yScaleA;
					if (yScaleB < yScale)
						yScale = yScaleB;
					crossSection->mScale.y = yScale * extraScale;
					hitCrossSection.Set(crossSectionIndex,TRUE);
				}


			}
		}
		if (fitAll)
		{
			SplineCrossSection *lastCrossSection = NULL;
			for (int crossSectionIndex = 0; crossSectionIndex < numCrossSections; crossSectionIndex++)
			{
				if (hitCrossSection[crossSectionIndex])
				{
					lastCrossSection = mSplineMap.GetCrossSection(splineIndex,crossSectionIndex);
					crossSectionIndex = numCrossSections;
				}
			}

			if (lastCrossSection)
			{
				for (int crossSectionIndex = 0; crossSectionIndex < numCrossSections; crossSectionIndex++)
				{
					if (!hitCrossSection[crossSectionIndex])
					{
						SplineCrossSection *crossSection = mSplineMap.GetCrossSection(splineIndex,crossSectionIndex);
						crossSection->mScale = lastCrossSection->mScale;
					}
					else
					{
						lastCrossSection = mSplineMap.GetCrossSection(splineIndex,crossSectionIndex);
					}
				}
			}
		}


	}

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		ld->PostIntersect();
	}
	mSplineMap.RecomputeCrossSections();

	fnSplineMap();
	theHold.Accept(_T(GetString(IDS_PW_SPLINEMAP)));

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());
	if (hView) InvalidateView();



}

void UnwrapMod::fnSplineMap_RecomputeCrossSections()
{
	mSplineMap.RecomputeCrossSections();

}

Point3 UnwrapMod::fnSplineMap_GetCrossSection_Pos(int splineIndex, int crossSectionIndex)
{
	SplineCrossSection *crossSection = mSplineMap.GetCrossSection(splineIndex,crossSectionIndex);
	if (crossSection)
	{
		return crossSection->mOffset;
	}
	return Point3(0.0f,0.0f,0.0f);
}
float UnwrapMod::fnSplineMap_GetCrossSection_ScaleX(int splineIndex, int crossSectionIndex)
{
	SplineCrossSection *crossSection = mSplineMap.GetCrossSection(splineIndex,crossSectionIndex);
	if (crossSection)
	{
		return crossSection->mScale.x;
	}
	return 0.0f;
}
float UnwrapMod::fnSplineMap_GetCrossSection_ScaleY(int splineIndex, int crossSectionIndex)
{
	SplineCrossSection *crossSection = mSplineMap.GetCrossSection(splineIndex,crossSectionIndex);
	if (crossSection)
	{
		return crossSection->mScale.y;
	}
	return 0.0f;
}
Quat UnwrapMod::fnSplineMap_GetCrossSection_Twist(int splineIndex, int crossSectionIndex)
{
	SplineCrossSection *crossSection = mSplineMap.GetCrossSection(splineIndex,crossSectionIndex);
	if (crossSection)
	{
		return crossSection->mQuat;
	}
	Quat q;
	q.Identity();
	return q;
}

void UnwrapMod::fnSplineMap_SetCrossSection_Pos(int splineIndex, int crossSectionIndex, Point3 p)
{
	SplineCrossSection *crossSection = mSplineMap.GetCrossSection(splineIndex,crossSectionIndex);
	if (crossSection)
	{
		crossSection->mOffset = p;
	}
	mSplineMap.SortCrossSections();
}
void UnwrapMod::fnSplineMap_SetCrossSection_ScaleX(int splineIndex, int crossSectionIndex, float scaleX)
{
	SplineCrossSection *crossSection = mSplineMap.GetCrossSection(splineIndex,crossSectionIndex);
	if (crossSection)
	{
		crossSection->mScale.x = scaleX;
	}
}
void UnwrapMod::fnSplineMap_SetCrossSection_ScaleY(int splineIndex, int crossSectionIndex, float scaleY)
{
	SplineCrossSection *crossSection = mSplineMap.GetCrossSection(splineIndex,crossSectionIndex);
	if (crossSection)
	{
		crossSection->mScale.y = scaleY;
	}
}
void UnwrapMod::fnSplineMap_SetCrossSection_Twist(int splineIndex, int crossSectionIndex, Quat twist)
{
	SplineCrossSection *crossSection = mSplineMap.GetCrossSection(splineIndex,crossSectionIndex);
	if (crossSection)
	{
		crossSection->mQuat = twist;
	}
}


int UnwrapMod::fnSplineMap_NumberOfSplines()
{
	return mSplineMap.NumberOfSplines();
}
int UnwrapMod::fnSplineMap_NumberOfCrossSections(int splineIndex)
{
	return mSplineMap.NumberOfCrossSections(splineIndex);
}


void UnwrapMod::fnSplineMap_SelectSpline(int index, BOOL sel)
{
	if (sel)
		mSplineMap.SelectSpline(index,true);
	else
		mSplineMap.SelectSpline(index,false);
}
BOOL UnwrapMod::fnSplineMap_IsSplineSelected(int index)
{
	return mSplineMap.IsSplineSelected(index);
}

void UnwrapMod::fnSplineMap_SelectCrossSection(int index, int crossIndex, BOOL sel)
{
	if (sel)
		mSplineMap.CrossSectionSelect(index,crossIndex,true);
	else
		mSplineMap.CrossSectionSelect(index,crossIndex,false);
}
BOOL UnwrapMod::fnSplineMap_IsCrossSectionSelected(int index,int crossIndex)
{
	return mSplineMap.CrossSectionIsSelected(index,crossIndex);
}

void UnwrapMod::fnSplineMap_ClearSplineSelection()
{
	for (int i = 0; i < mSplineMap.NumberOfSplines(); i++)
	{
		mSplineMap.SelectSpline(i,FALSE);
	}
}
void UnwrapMod::fnSplineMap_ClearCrossSectionSelection()
{
	for (int i = 0; i < mSplineMap.NumberOfSplines(); i++)
	{
		for (int j = 0; j < mSplineMap.NumberOfCrossSections(i); j++)
			mSplineMap.CrossSectionSelect(i,j,FALSE);
	}
}

void	UnwrapMod::UpdateSplineMappingNode(INode *node)
{
	TimeValue t = GetCOREInterface()->GetTime();
	mSplineMap.UpdateNode(node);
	//update our map groups since the user may have added or removed faces
	//loop through the lds

}

void	UnwrapMod::SetSplineMappingNode(INode *node)
{

	TimeValue t = GetCOREInterface()->GetTime();

	//add the node to the spline map system
	mSplineMap.AddNode(node);
	fnSplineMap_Fit(TRUE,1.5f);
}


void	UnwrapMod::fnSplineMap()
{
	if (mSplineMap.NumberOfSplines() == 0)
		return;

	int adv;
	pblock->GetValue(unwrap_splinemap_method,0,adv,FOREVER);
	mSplineMap.SetMappingType(adv);


	if (theHold.Holding())
	{
		this->HoldPointsAndFaces();
	}
	TimeValue t = GetCOREInterface()->GetTime();

	BOOL manualSeams = FALSE;
	pblock->GetValue(unwrap_splinemap_manualseams,t,manualSeams,FOREVER);

	float uScale, vScale, uGOffset, vGOffset;
	pblock->GetValue(unwrap_splinemap_uscale,t,uScale,FOREVER);
	pblock->GetValue(unwrap_splinemap_vscale,t,vScale,FOREVER);

	pblock->GetValue(unwrap_splinemap_uoffset,t,uGOffset,FOREVER);
	pblock->GetValue(unwrap_splinemap_voffset,t,vGOffset,FOREVER);

	int maxIterations;
	pblock->GetValue(unwrap_splinemap_iterations,0,maxIterations,FOREVER);

	BOOL noralizeMap = fnGetNormalizeMap();

	float uTile = 1.0f;
	if (uScale != 0.0f)
		uScale = 1.0f/uScale;
	float vTile = 1.0f;
	if (vScale != 0.0f)
		vScale = 1.0f/vScale;

	int mCT = mMeshTopoData.Count();
	BOOL useAllFaces = TRUE;
	for (int ldID = 0; ldID < mCT; ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		if (ld->GetFaceSel().NumberSet())
			useAllFaces = FALSE;
	}

	//loop through our instances	
	for (int ldID = 0; ldID < mCT; ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		int numGeoPoints = ld->GetNumberGeomVerts();
		Matrix3 tm = mMeshTopoData.GetNodeTM(t,ldID); 


		Tab<Point3> uvw;	
		uvw.SetCount(numGeoPoints);

		//build a potential list of points to map
		float globalXOffset = 0.0f;

		int splineIndex = mSplineMap.GetActiveSpline();
		{


			//find all the points that are not part of the selection
			BitArray facesToBeMapped;
			facesToBeMapped = ld->GetFaceSelection();
			if (useAllFaces)
				facesToBeMapped.SetAll();
				

			BitArray dontTouch;
			dontTouch.SetSize(ld->GetNumberTVVerts());
			dontTouch.ClearAll();

			BitArray mapTheseVerts;
			mapTheseVerts.SetSize(ld->GetNumberGeomVerts());

			for (int i = 0; i < ld->GetNumberFaces(); i++)
			{

				if (!ld->GetFaceDead(i)  )
				{		
					if (!facesToBeMapped[i])
						ld->GetFaceTVPoints(i,dontTouch);
					else
						ld->GetFaceGeomPoints(i,mapTheseVerts);
				}
			}

			Tab<int> geoToUVW;
			geoToUVW.SetCount(numGeoPoints);
			for (int i = 0; i < numGeoPoints; i++)
			{
				geoToUVW[i] = -1;
			}


			//map the points

			SplineMapProjectionTypes projType = kCircular;
			int val = 0;
			pblock->GetValue(unwrap_splinemap_projectiontype,t,val,FOREVER);
			if (val == 1)
				projType = kPlanar;

			float largestD = 0.0f;

			//loop through the points
			for (int i = 0; i < numGeoPoints; i++)
			{
				ld->SetGeomVertSelected(i, FALSE);

				uvw[i] = Point3(0.0f,0.0f,0.0f);
				//only map the ones we need
				if (mapTheseVerts[i])
				{			
					Point3 p = ld->GetGeomVert(i);
					p = p * tm;
					float u = 0.0f;
					int s = 0;
					Point3 hitPoint(0.0f,0.0f,0.0f);
					float d = 0.0f;
					if (mSplineMap.ProjectPoint(splineIndex,p,uvw[i],d,hitPoint,projType,false,maxIterations))
					{
						if (d > largestD)
							largestD = d;
						ld->SetGeomVertSelected(i, TRUE);
					}
				}
			}


			Tab<int> deadVerts;
			for (int m = 0; m < ld->GetNumberTVVerts();m++)
			{
				if (ld->GetTVVertDead(m))
				{
					deadVerts.Append(1,&m,1000);						
				}
			}
			//get our uvw to geom points
			//now looop through the faces
			for (int i = 0; i < ld->GetNumberFaces(); i++)
			{
				//if the face needs to be mapped repoint the indices
				if (!ld->GetFaceDead(i) && facesToBeMapped[i])
				{			
					int degree = ld->GetFaceDegree(i);
					for (int j = 0 ; j < degree; j++)
					{
						int geoIndex = ld->GetFaceGeomVert(i,j);
						if (geoToUVW[geoIndex] == -1)
						{
							geoToUVW[geoIndex] = ld->AddTVVert(t,uvw[geoIndex],i,j,this,FALSE,&deadVerts);
						}
						else
						{
							ld->SetFaceTVVert(i,j,geoToUVW[geoIndex]);
						}


						if (ld->GetFaceCurvedMaping(i))
						{
							if (ld->GetFaceHasVectors(i))
							{

								int geoIndex = ld->GetFaceGeomHandle(i,j*2);
								if (geoToUVW[geoIndex] == -1)
								{
									geoToUVW[geoIndex] = ld->AddTVHandle(t,uvw[geoIndex],i,j*2,this,FALSE,&deadVerts);
								}
								else
								{
									ld->SetFaceTVHandle(i,j*2,geoToUVW[geoIndex]);
								}

								geoIndex = ld->GetFaceGeomHandle(i,j*2+1);
								if (geoToUVW[geoIndex] == -1)
								{
									geoToUVW[geoIndex] = ld->AddTVHandle(t,uvw[geoIndex],i,j*2+1,this,FALSE,&deadVerts);
								}
								else
								{
									ld->SetFaceTVHandle(i,j*2+1,geoToUVW[geoIndex]);
								}


								if (ld->GetFaceHasInteriors(i))
								{
									int geoIndex = ld->GetFaceGeomInterior(i,j);
									if (geoToUVW[geoIndex] == -1)
									{
										geoToUVW[geoIndex] = ld->AddTVInterior(t,uvw[geoIndex],i,j,this,FALSE,&deadVerts);
									}
									else
									{
										ld->SetFaceTVInterior(i,j,geoToUVW[geoIndex]);
									}

								}
							}
						}
					}			
				}
			}

//now compute our seams
//first fix the V seam
			if (mSplineMap.SplineClosed(splineIndex))
			{
				//if not manual seams then break along the seams
				//first break at the V seams
				for (int i = 0; i < ld->GetNumberFaces(); i++)
				{
					if (!ld->GetFaceDead(i) && facesToBeMapped[i])
					{			
						int degree = ld->GetFaceDegree(i);
						BOOL inQuad1 = FALSE;
						BOOL inQuad4 = FALSE;
						for (int j = 0 ; j < degree; j++)
						{
							int tvIndex = ld->GetFaceTVVert(i,j);
							Point3 uvw = ld->GetTVVert(tvIndex);
							if (uvw.y < 0.25f)
								inQuad1 = TRUE;
							else if (uvw.y > 0.75f)
								inQuad4 = TRUE;					
						}
						if (inQuad1 && inQuad4)
						{
							for (int j = 0 ; j < degree; j++)
							{
								int tvIndex = ld->GetFaceTVVert(i,j);
								Point3 uvw = ld->GetTVVert(tvIndex);

								if (uvw.y < 0.25f)
								{
									uvw.y += 1.0f;

									ld->AddTVVert(t,uvw,i,j,this,FALSE,&deadVerts);

									if (ld->GetFaceCurvedMaping(i))
									{
										if (ld->GetFaceHasVectors(i))
										{

											int tvIndex = ld->GetFaceTVHandle(i,j*2);
											Point3 uvw = ld->GetTVVert(tvIndex);
											if (uvw.y < 0.25f)
											{
												uvw.y += 1.0f;
												ld->AddTVHandle(t,uvw,i,j*2,this,FALSE,&deadVerts);
											}

											tvIndex = ld->GetFaceTVHandle(i,j*2+1);
											uvw = ld->GetTVVert(tvIndex);
											if (uvw.y < 0.25f)
											{
												uvw.y += 1.0f;
												ld->AddTVHandle(t,uvw,i,j*2+1,this,FALSE,&deadVerts);
											}

											if (ld->GetFaceHasInteriors(i))
											{
												int tvIndex = ld->GetFaceTVInterior(i,j);
												Point3 uvw = ld->GetTVVert(tvIndex);
												if (uvw.y < 0.25f)
												{
													uvw.y += 1.0f;
													ld->SetTVVert(t,tvIndex,uvw,this);
												}
											}
										}
									}
								}
							}			
						}
					}
				}
			}
			if (projType != kPlanar)
			{

				if (manualSeams && ld->mSeamEdges.GetSize())
				{
					ld->SetTVEdgeInvalid();
					ld->BuildTVEdges();
					//get our map seams
					BitArray splitEdges = ld->mSeamEdges;
					splitEdges.ClearAll();
					//remove any edges that are not part of our mapping
					for (int i = 0; i < ld->GetNumberGeomEdges(); i++)
					{
						if (ld->mSeamEdges[i])
						{
							int fct = ld->GetGeomEdgeNumberOfConnectedFaces(i);
							for (int j = 0; j < fct; j++)
							{
								int faceIndex = ld->GetGeomEdgeConnectedFace(i,j);
								if (facesToBeMapped[faceIndex])
								{
									splitEdges.Set(i,TRUE);
								}
							}
						}

					}



					BitArray uvSplitEdges = ld->GetTVEdgeSelection();
					uvSplitEdges.ClearAll();
					ld->ConvertGeomEdgeSelectionToTV(splitEdges, uvSplitEdges );


					//build our edge from u 0.0 to 1.0
					Tab<float> boundarySeam;
					int samples = 10001;
					float sampleSize = 10000.0f;
					boundarySeam.SetCount(samples);
					for (int i = 0; i < samples; i++)
						boundarySeam[i] = -1.0f;

					//build our map seam edge
					BitArray leftEdge;
					leftEdge.SetSize(ld->GetNumberTVVerts());
					leftEdge.ClearAll();

					BitArray rightEdge;
					rightEdge.SetSize(ld->GetNumberTVVerts());
					rightEdge.ClearAll();

					//first we need to check to see if our edge crosses a seam
					Point3 tvXMax(0.0f,0.0f,0.0f);
					int tvXMaxCT = 0;
					BOOL edgeCrossesSeam = FALSE;
					//loop through the edges
					for (int i = 0; i < ld->GetNumberTVEdges(); i++)
					{
						//if it is an edge we are about to split it needs to checked
						if (uvSplitEdges[i])
						{
							int a = ld->GetTVEdgeVert(i,0);
							int b = ld->GetTVEdgeVert(i,1);

							Point3 pa = ld->GetTVVert(a);
							Point3 pb = ld->GetTVVert(b);
							float d =  fabs(pa.x - pb.x);
							//if the distance is > 0.5 that edge crossed the seam
							if (d > 0.5f)
							{			
								//find the center of that edge and store it off
								tvXMax += (pa+pb) * 0.5f;
								tvXMaxCT++;
								edgeCrossesSeam = TRUE;
							}
						}
					}

					//if we the edge crosses a seam we need to fix things up
					if (edgeCrossesSeam)
					{
						//we need to slide the UVWs over to prevent the edge sel from crossing the map seam
						tvXMax = tvXMax/(float)tvXMaxCT;
						BitArray processedVerts;
						processedVerts.SetSize(ld->GetNumberTVVerts());
						processedVerts.ClearAll();
						Tab<int> vIndex;
						TimeValue t = GetCOREInterface()->GetTime();
						//loop through our faces
						for (int i = 0; i < ld->GetNumberFaces(); i++)
						{
							if (facesToBeMapped[i] && !ld->GetFaceDead(i))
							{
								ld->GetFaceTVPoints(i,vIndex);
								for (int j = 0; j < vIndex.Count(); j++)
								{
									int id = vIndex[j];
									//make sure we have not already set this vert
									if (!processedVerts[id])
									{
										processedVerts.Set(id,TRUE);
										Point3 tv = ld->GetTVVert(id);
										//if the vert is greater than our new seam put it on the other side
										if (tv.x >= tvXMax.x)
										{
											tv.x -= 1.0f;
											ld->SetTVVert(t,id,tv,this);
										}
									}
								}
							}			
						}
					}



					for (int i = 0; i < ld->GetNumberTVEdges(); i++)
					{
						if (uvSplitEdges[i])
						{
							int a = ld->GetTVEdgeVert(i,0);
							int b = ld->GetTVEdgeVert(i,1);

							leftEdge.Set(a,TRUE);
							leftEdge.Set(b,TRUE);
							Point3 pa = ld->GetTVVert(a);
							Point3 pb = ld->GetTVVert(b);

							if (pa.y > pb.y)
							{
								Point3 temp = pa;
								pa = pb;
								pb = temp;
							}

							float startY = (pa.y*sampleSize);
							float endY = (pb.y*sampleSize);
							float samples = (endY-startY);
							Point3 inc = (pb - pa)/(samples);
							Point3 p = pa;

							int isamples = (int) samples;
							while (p.y <= pb.y)
							{
								int cy = (int)(p.y*sampleSize);	
								float x = p.x;
								//need to first see if out range since the spline can exist past the mesh
								if (cy < boundarySeam.Count()) 
								{
									//next see if the cell has already been set to something 
									if ( (boundarySeam[cy] == -1.0f) || (x < boundarySeam[cy]))
										boundarySeam[cy] = x;
								}
								p += inc;

							}
							int y = (int)(pa.y*sampleSize);
							float x = pa.x;
							if (y < boundarySeam.Count())
								boundarySeam[y] = x;

							y = (int)(pb.y*sampleSize);
							x = pb.x;
							if (y < boundarySeam.Count())
								boundarySeam[y] = x;
						}
					}
//fill in any blanks
					int bi = 0;
					float lowerLimit = 0.0f;
					while ((bi < boundarySeam.Count()) && (boundarySeam[bi] == -1.0f ) )
					{						
						bi++;
						if (bi < boundarySeam.Count())
							lowerLimit = boundarySeam[bi];
					}
					for (int j = 0; j < bi; j++)
					{
						if ((j < boundarySeam.Count()) && (boundarySeam[j] == -1.0f))
							boundarySeam[j] = lowerLimit;
					}
					if (boundarySeam.Count())
					{
						float lastLimit = boundarySeam[0];
						for (int j = 0; j < boundarySeam.Count(); j++)
						{
							if (boundarySeam[j] == -1.0f)
								boundarySeam[j] = lastLimit;
							else if (boundarySeam[j] != -1.0f)
								lastLimit = boundarySeam[j];

						}
					}


					//break those edge

					int numberVerts = ld->GetNumberTVVerts();
					ld->BreakEdges(uvSplitEdges);
					int numberNewVerts = ld->GetNumberTVVerts();

					leftEdge.SetSize(numberNewVerts,1);
					rightEdge.SetSize(numberNewVerts,1);

					ld->SetTVEdgeInvalid();
					ld->BuildTVEdges();
					if (splitEdges.NumberSet())
						fnSplineMap_GetSplitEdges(ld, splitEdges, leftEdge, rightEdge);

					ld->SetTVVertSelection(rightEdge);


					//if on left of the edge add 1.0
					//if not manual seams then break along the seams

					BitArray vertsToMove;
					vertsToMove.SetSize(ld->GetNumberTVVerts());
					vertsToMove.ClearAll();


					BitArray processedFaces;
					processedFaces.SetSize(ld->GetNumberFaces());
					processedFaces.ClearAll();

					//move all the obivous ones
					//get all vertice pretty far away from the edge					
					for (int i = 0; i < ld->GetNumberFaces(); i++)
					{
						if (!ld->GetFaceDead(i) && facesToBeMapped[i])
						{	

							int degree = ld->GetFaceDegree(i);
							int moveFace = 0;
							int staticFace = 0;

							for (int j = 0 ; j < degree; j++)
							{
								int tvIndex = ld->GetFaceTVVert(i,j);

								if (rightEdge[tvIndex])
								{
									processedFaces.Set(i,TRUE);
									j = degree;
								}
								else
								{
									Point3 uvw = ld->GetTVVert(tvIndex);
									int y = (int)(uvw.y*sampleSize);
									if (y >= boundarySeam.Count())
										y =  boundarySeam.Count()-1;

									if (uvw.x < (boundarySeam[y]-0.01f))
									{
										vertsToMove.Set(tvIndex,TRUE);
										moveFace++;
									}
									if (uvw.x > (boundarySeam[y]+0.01f))
									{
										staticFace++;
									}
								}
							}
							//if we moved all the verts of this face it has been processed
							//we dont need to process this face anymore
							if ( (moveFace == degree) ||
								 (staticFace == degree) )
							{
								processedFaces.Set(i,TRUE);
							}
						}
					}

					//expand our selection
					BitArray tempVerts;
					tempVerts.SetSize(ld->GetNumberTVVerts());
					tempVerts.ClearAll();
					
					BitArray borderFaces;
					borderFaces.SetSize(ld->GetNumberFaces());
					borderFaces.ClearAll();

					//add our border verts to be moved
					//vertsToMove |= leftEdge;
					//now check all our ambigous verts
					BOOL done = FALSE;
					int pass = 0;
					while (!done)
					{			
						tempVerts.ClearAll();
						done = TRUE;
						//loop through the faces
						pass++;
						for (int i = 0; i < ld->GetNumberFaces(); i++)
						{
							//if the face is not processed look at it
							if (!ld->GetFaceDead(i) && facesToBeMapped[i] && !processedFaces[i])
							{
								int degree = ld->GetFaceDegree(i);
								BOOL boundaryFace = FALSE;
								//see if we are connected to any boundary vertices
								for (int j = 0 ; j < degree; j++)
								{
									int tvIndex = ld->GetFaceTVVert(i,j);
									if ((tvIndex < leftEdge.GetSize()) && leftEdge[tvIndex])
									{
										boundaryFace = TRUE;
										processedFaces.Set(i,TRUE);
									}
								}
								//if we touch a boundary vertex now look to see what verts need
								//to be moved
								if (boundaryFace)
								{
									done = FALSE;
									for (int j = 0 ; j < degree; j++)
									{
										int tvIndex = ld->GetFaceTVVert(i,j);
										Point3 uvw = ld->GetTVVert(tvIndex);
										int y = (int)(uvw.y*sampleSize);
										if (y >= boundarySeam.Count())
											y =  boundarySeam.Count()-1;

		
										tempVerts.Set(tvIndex,TRUE);
										if (uvw.x < (boundarySeam[y]+0.01f))
										{
											vertsToMove.Set(tvIndex,TRUE);
										}
									}
								}
							}
						}
						leftEdge |= tempVerts;
						
					}



					for (int i = 0; i < ld->GetNumberTVVerts(); i++)
					{
						if (vertsToMove[i])
						{
							Point3 uvw = ld->GetTVVert(i);
							uvw.x += 1.0f;
							ld->SetTVVert(t,i,uvw,this);

						}
					}
				}
				else if (!manualSeams)  // not manual seams
				{
					//if not manual seams then break along the seams
					//now break the U seams
					for (int i = 0; i < ld->GetNumberFaces(); i++)
					{
						if (!ld->GetFaceDead(i) && facesToBeMapped[i])
						{			
							int degree = ld->GetFaceDegree(i);
							BOOL inQuad1 = FALSE;
							BOOL inQuad4 = FALSE;
							for (int j = 0 ; j < degree; j++)
							{
								int tvIndex = ld->GetFaceTVVert(i,j);
								Point3 uvw = ld->GetTVVert(tvIndex);
								//I use .2499 and 0.7501 instead of 0.25 and 0.75
								//this is because if you are mapping a strip 1 faces wide you get value of ~0.250 and ~0.750 if
								//the cross sections are aligned to the surface.  this causes them to flip back and forth
								if (uvw.x < 0.2499f)
									inQuad1 = TRUE;
								else if (uvw.x > 0.7501f)
									inQuad4 = TRUE;					
							}

							if (inQuad1 && inQuad4)
							{
								for (int j = 0 ; j < degree; j++)
								{
									int tvIndex = ld->GetFaceTVVert(i,j);
									Point3 uvw = ld->GetTVVert(tvIndex);

									if (uvw.x < 0.2499f)
									{
										uvw.x += 1.0f;

										ld->AddTVVert(t,uvw,i,j,this,FALSE,&deadVerts);

										if (ld->GetFaceCurvedMaping(i))
										{
											if (ld->GetFaceHasVectors(i))
											{

												int tvIndex = ld->GetFaceTVHandle(i,j*2);
												Point3 uvw = ld->GetTVVert(tvIndex);
												if (uvw.x < 0.2499f)
												{
													uvw.x += 1.0f;
													ld->AddTVHandle(t,uvw,i,j*2,this,FALSE,&deadVerts);
												}

												tvIndex = ld->GetFaceTVHandle(i,j*2+1);
												uvw = ld->GetTVVert(tvIndex);
												if (uvw.x < 0.2499f)
												{
													uvw.x += 1.0f;
													ld->AddTVHandle(t,uvw,i,j*2+1,this,FALSE,&deadVerts);
												}

												if (ld->GetFaceHasInteriors(i))
												{
													int tvIndex = ld->GetFaceTVInterior(i,j);
													Point3 uvw = ld->GetTVVert(tvIndex);
													if (uvw.x < 0.2499f)
													{
														uvw.x += 1.0f;
														ld->SetTVVert(t,tvIndex,uvw,this);
													}
												}
											}
										}
									}
								}			
							}
						}
					}
				}

			}

			BitArray affectedVerts;
			affectedVerts.SetSize(ld->GetNumberTVVerts());
			affectedVerts.ClearAll();

			//center them
			//then slide them along the offset
			//add our offset
			//add the tile
			for (int i = 0; i < ld->GetNumberFaces(); i++)
			{
				if (!ld->GetFaceDead(i) && facesToBeMapped[i])
				{	
					ld->GetFaceTVPoints(i,affectedVerts);
				}
			}
			Box3 bounds;
			float uNScale = 1.0f;
			float vNScale = 1.0f;
			if (!normalizeMap)
			{
				//get the spline length
				vNScale = mSplineMap.SplineLength(splineIndex);
				//get the largest cross section
				if (largestD > 0.0f)
					uNScale = largestD;
			}

			//now apply our tiling and offsets
			for (int i = 0; i < ld->GetNumberTVVerts(); i++)
			{
				if (affectedVerts[i])
				{
					Point3 p = ld->GetTVVert(i);
					p.x *= uTile * uNScale;
					p.y *= vTile * vNScale;
					ld->SetTVVert(t,i,p,this);
					bounds += p;
				}
			}
			float minU = 0.0f; 
			float minV = 0.0f; 
			float maxU = 0.0f; 
			if (!bounds.IsEmpty())
			{
				minU = bounds.pmin.x;
				minV = bounds.pmin.y;
				maxU = bounds.pmax.x - minU;
			}

			float xOffset = 0.0f-minU + globalXOffset;
			float yOffset = 0.0f-minV;

			for (int i = 0; i < ld->GetNumberTVVerts(); i++)
			{
				if (affectedVerts[i])
				{
					Point3 p = ld->GetTVVert(i);
					p.x += xOffset + uGOffset;
					p.y += yOffset + vGOffset;;
					ld->SetTVVert(t,i,p,this);
				}
			}
			globalXOffset += (int)(maxU + 1.0f);
		}

		ld->SetTVEdgeInvalid();
		ld->BuildTVEdges();
		ld->BuildVertexClusterList();

	}
}

void UnwrapMod::fnSplineMap_MoveSelectedCrossSections(Point3 p)
{
	if (theHold.Holding())
		mSplineMap.HoldData();
	mSplineMap.MoveSelectedCrossSections(p);
}


void UnwrapMod::fnSplineMap_RotateSelectedCrossSections(Quat twist)
{
	if (theHold.Holding())
		mSplineMap.HoldData();
	mSplineMap.RotateSelectedCrossSections(twist);
}


void UnwrapMod::fnSplineMap_ScaleSelectedCrossSections(Point2 scale)
{
	if (theHold.Holding())
		mSplineMap.HoldData();
	mSplineMap.ScaleSelectedCrossSections(scale);
}

void UnwrapMod::fnSplineMap_Dump()
{
	for (int j = 0; j < mSplineMap.NumberOfSplines(); j++)
	{
		for (int i = 0; i < mSplineMap.NumberOfCrossSections(j); i++)
		{
			SplineCrossSection *cross = mSplineMap.GetCrossSection(j,i);
			cross->Dump();
		}
	}
}


void UnwrapMod::fnSplineMap_Resample(int sample)
{
	mSplineMap.Resample(sample);

	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());
	if (hView) InvalidateView();

}


void UnwrapMod::fnSplineMap_SectActiveSpline(int index)
{
	mSplineMap.SetActiveSpline(index);
}

//see if we are a split mesh or continouse
void UnwrapMod::fnSplineMap_GetSplitEdges(MeshTopoData *ld, BitArray geomEdgeSel, BitArray &left, BitArray &right)
{

	//get our edge sel
	//find the end points
	Tab<int> vertCount;
	vertCount.SetCount(ld->GetNumberGeomVerts());
	for (int i = 0; i < vertCount.Count(); i++)
		vertCount[i] = 0;

	//we just see count how many selected edges touch a vert
	//anyone with with just a count of 1 is end vert
	for (int i = 0; i < ld->GetNumberGeomEdges(); i++)
	{
		if (geomEdgeSel[i])
		{
			int a = ld->GetGeomEdgeVert(i,0);
			int b = ld->GetGeomEdgeVert(i,1);
			vertCount[a] += 1;
			vertCount[b] += 1;
		}
	}

	int startEdge = -1;

	Tab<int> edgeList;
	int startVert = -1;
	int secondVert = -1;
	Point3 startTV(0.0f,0.0f,0.0f);

	//loop through the edges finding a starting edge and vert
	for (int i = 0; i < ld->GetNumberGeomEdges(); i++)
	{
		if (geomEdgeSel[i])
		{
			int a = ld->GetGeomEdgeVert(i,0);
			int b = ld->GetGeomEdgeVert(i,1);

			if ( (vertCount[a] == 1) ||
				(vertCount[b] == 1) )
			{
				if (startEdge == -1)
				{
					startEdge = i;
					if (vertCount[a] == 1)
					{
						startVert = a;
						secondVert = b;
					}
					else
					{
						startVert = b;
						secondVert = a;
					}
				}
				else
				{
					edgeList.Append(1,&i,1000);
				}
			}
			else
			{
				edgeList.Append(1,&i,1000);
			}
		}
	}


	//build our geom edge list
	Tab<int> orderEdgeList;
	Tab<int> orderVertList;
	BOOL done = FALSE;
	orderEdgeList.Append(1,&startEdge,1000);
	orderVertList.Append(1,&startVert,1000);
	orderVertList.Append(1,&secondVert,1000);
	//now order our edges and verts from start to end of the selected edges
	while (!done)
	{
		int previousEdge = orderEdgeList[orderEdgeList.Count()-1];
		int a = ld->GetGeomEdgeVert(previousEdge,0);
		int b = ld->GetGeomEdgeVert(previousEdge,1);
		BOOL hit = FALSE;
		for (int i = 0; i < edgeList.Count(); i++)
		{
			int id = edgeList[i];
			int ea = ld->GetGeomEdgeVert(id,0);
			int eb = ld->GetGeomEdgeVert(id,1);

			if ( (ea == a) || (eb == b) || 
				 (ea == b) || (eb == a))
			{
				hit = TRUE;
				orderEdgeList.Append(1,&id,1000);
				int lastvert = orderVertList[orderVertList.Count()-1];
				if (ea == lastvert)
					orderVertList.Append(1,&eb,1000);
				else
					orderVertList.Append(1,&ea,1000);

				edgeList.Delete(i,1);
				i = edgeList.Count();
			}
		}
		if (!hit)
		{
			done = TRUE;
		}
	}



	left.ClearAll();
	right.ClearAll();
	int leftVert = -1;
	int rightVert = -1;

	
	Point3 tvStart(0.0f,0.0f,0.0f),tvEnd(0.0f,0.0f,0.0f);

	//now see which side the edges are.  I use face winding compared against the direction of the edge
	//to set which side the face/verts are on
	for (int i = 0; i < orderEdgeList.Count(); i++)
	{
		int id = orderEdgeList[i];
		int vert1 = orderVertList[i];
		int vert2 = orderVertList[i+1];

		for (int j = 0; j < 2; j++)
		{
			int face = ld->GetGeomEdgeConnectedFace(id,j);
			if (face != -1)
			{
				int deg = ld->GetFaceDegree(face);
				for (int k = 0; k < deg; k++)
				{
					int a = ld->GetFaceGeomVert(face,k);
					int b = ld->GetFaceGeomVert(face,(k+1)%deg);

					int ta = ld->GetFaceTVVert(face,k);
					int tb = ld->GetFaceTVVert(face,(k+1)%deg);


					if ((a == vert2) && (b == vert1))
					{
						left.Set(ta,TRUE);
						left.Set(tb,TRUE);
						k = deg;
						if (i == 0)
							tvStart = ld->GetTVVert(ta);
						else if (i == orderEdgeList.Count()-1)
							tvEnd = ld->GetTVVert(ta);


					}
					else if ((a == vert1) && (b == vert2))
					{
						right.Set(ta,TRUE);
						right.Set(tb,TRUE);
						k = deg;
					}
				}				
			}			
		}
	}

	//see if we need to flip the edges since we can be going from bottom up or vice versa
	if (tvStart.y < tvEnd.y)
	{	
		BitArray temp = left;
		left = right;
		right = temp;
	}


}

