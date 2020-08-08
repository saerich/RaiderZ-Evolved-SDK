/********************************************************************** *<
FILE: ToolSplineMapping.cpp

DESCRIPTION: This conatins all SplineData Methods


HISTORY: 12/31/96
CREATED BY: Peter Watje


*>	Copyright (c) 2007, All Rights Reserved.
**********************************************************************/


#include "unwrap.h"


SplineData::SplineData()
{
	mSampleRate = 10;
	mHasCopyBuffer = FALSE;
	mSplineDataLoaded = false;
	mHWND = NULL;
	mCurrentMode = kNoMode;
	mActiveSpline = 0;
	mReset = false;
}



int	SplineData::NumberOfCrossSections(int splineIndex)
{
	if ((splineIndex >= 0) && (splineIndex < mSplineElementData.Count()))
		return mSplineElementData[splineIndex]->GetNumberOfCrossSections();	
	return 0;
}


int	SplineData::NumberOfSplines()
{
	return mSplineElementData.Count();
}

bool SplineData::UpdateNode(INode *node)
{
	if (mReset)
	{
		mReset = false;
		return AddNode(node);
	}
	else
	{
		TimeValue t  = GetCOREInterface()->GetTime();
		Interval valid;
		Matrix3 tm = node->GetObjectTM(t,&valid);
		ObjectState nos = node->EvalWorldState(t);
		//make sure it is a shape
		if (nos.obj->IsShapeObject()) 
		{
			ShapeObject *pathOb = (ShapeObject*)nos.obj;
			pathOb->MakePolyShape(t, mShapeCache,32,FALSE);
			mShapeCache.Transform(tm);
			int numberOfCurves = pathOb->NumberOfCurves();
			for (int i = 0; i < numberOfCurves; i++)
			{
				//just rebuild our cache info
				mSplineElementData[i]->ResetBoundingBox();
				mSplineElementData[i]->SetLineCache(&mShapeCache.lines[i]);
				mSplineElementData[i]->ComputePlanarUp(mPlanarMapUp);
				if (pathOb->CurveClosed(t,i))
					mSplineElementData[i]->SetClosed(true);
				else mSplineElementData[i]->SetClosed(false);
				float curveLength = pathOb->LengthOfCurve(t,i);
				mSplineElementData[i]->Select(FALSE);
			}
			//now rebuild our derived cross sections
			RecomputeCrossSections();
			if (mActiveSpline >= numberOfCurves)
				mActiveSpline = 0;
			mSplineElementData[mActiveSpline]->Select(TRUE);
			return true;
		}
	}
	return false;
}

bool SplineData::AddNode(INode *node)
{
	//see if the node is a shape object
	if (node != NULL) 
	{
		TimeValue t  = GetCOREInterface()->GetTime();
		Interval valid;
		Matrix3 tm = node->GetObjectTM(t,&valid);
		ObjectState nos = node->EvalWorldState(t);
		if (nos.obj->IsShapeObject()) 
		{
			ShapeObject *pathOb = (ShapeObject*)nos.obj;
			int numberOfCurves = pathOb->NumberOfCurves();
			//get number of splines
			if (numberOfCurves != 0) 
			{
				pathOb->MakePolyShape(t, mShapeCache,32,FALSE);
				mShapeCache.Transform(tm);
				mSplineElementData.SetCount(numberOfCurves);
				for (int i = 0; i < numberOfCurves; i++)
					mSplineElementData[i] = NULL;

				for (int i = 0; i < numberOfCurves; i++)
				{

					Box3 bounds;
					bounds.Init();

					//sample the shape object
					int numberOfCurves = pathOb->NumberOfPieces(t, i);
					if (numberOfCurves != 0)
					{

						mSplineElementData[i] = new SplineElementData();
						
						mSplineElementData[i]->ResetBoundingBox();
						mSplineElementData[i]->SetLineCache(&mShapeCache.lines[i]);
						mSplineElementData[i]->ComputePlanarUp(mPlanarMapUp);
						mSplineElementData[i]->SetNumCurves(numberOfCurves);
						mSplineElementData[i]->SetNumberOfCrossSections(numberOfCurves+1);
						if (pathOb->CurveClosed(t,i))
							mSplineElementData[i]->SetClosed(true);
						else mSplineElementData[i]->SetClosed(false);
						float curveLength = pathOb->LengthOfCurve(t,i);

						float runningLength = 0.0f;
						Quat q;
						q.Identity();
						mSplineElementData[i]->SetCrossSection(0,Point3(0.0f,0.0f,0.0f),Point2(20.0f,20.0f),q);
						for (int j = 0; j < numberOfCurves; j++)
						{
							float u = runningLength/curveLength;
							mSplineElementData[i]->SetCrossSection(j,Point3(0.0f,0.0f,u),Point2(20.0f,20.0f),q);
							Point3 lastPoint = pathOb->InterpPiece3D(t,i,j,0.0f,PARAM_NORMALIZED);
							bounds += lastPoint;
							float runningU = 0.0f;
							float inc = 1.0f/(float)(mSampleRate-1);
							for (int k = 1; k < mSampleRate; k++)
							{								
								runningU += inc;
								Point3 p = pathOb->InterpPiece3D(t,i,j,runningU,PARAM_NORMALIZED);
								runningLength += Length(p-lastPoint);
								lastPoint = p;
								bounds += lastPoint;
							}


						}
						float u = runningLength/curveLength;


						mSplineElementData[i]->SetCrossSection(numberOfCurves,Point3(0.0f,0.0f,1.0f),Point2(20.0f,20.0f),q);
						mSplineElementData[i]->Select(FALSE);

					}
				}

				if (mActiveSpline >= numberOfCurves)
					mActiveSpline = 0;
				mSplineElementData[mActiveSpline]->Select(TRUE);

				//build our bounding box
				RecomputeCrossSections();

				return true;
			}
		}
	}

	return false;

}


SplineData::~SplineData()
{
	FreeData();
}

void SplineData::FreeData()
{
	for (int i = 0; i < mSplineElementData.Count(); i++)
	{
		if (mSplineElementData[i])
			delete mSplineElementData[i];
	}
}


void SplineData::RecomputeCrossSections()
{
	for (int i = 0; i < mSplineElementData.Count(); i++)
	{
		if (mSplineElementData[i])
		{
//			mSplineElementData[i]->ComputeBoundingBoxes(mSampleRate);			
			mSplineElementData[i]->UpdateCrossSectionData();
		}
	}
}


int	SplineData::FindClosestSpline(Point3 p)
{
	float closestD = -1.0f;
	int closestSpline = -1;
	for (int i = 0; i < mSplineElementData.Count(); i++)
	{
		int numberCrossSections = mSplineElementData[i]->GetNumberOfCrossSections();
		for (int j = 0; j < numberCrossSections; j++)
		{
			SplineCrossSection crossSection = mSplineElementData[i]->GetCrossSection(j);
			float testDist = Length(p-crossSection.mP);
			if ((testDist < closestD) || (closestSpline == -1))
			{
				closestD = testDist;
				closestSpline = i;
			}
		}
	}

	return closestSpline;

}
bool SplineData::ProjectPoint(int splineIndex, Point3 p,  Point3 &hitUVW, float &d, Point3 &hitPoint, SplineMapProjectionTypes projectionType, bool onlyInsideEnvelope, int maxIterations)
{
	bool hit = false;



	float scale = 1.0f;	//scale is used to expand our envelopes on each pass so we find at least a close point
	float limit = 128.0f;

	if (onlyInsideEnvelope)
		limit = 2.0f;

	mSplineElementData[splineIndex]->mClosestSubs.SetSize(mSplineElementData[splineIndex]->GetNumberOfSubBoundingBoxes());
	mSplineElementData[splineIndex]->mClosestSubs.ClearAll();
	while (!hit && scale < limit)
	{
		Tab<int> bounds;
		//start by getting a list of samples that contain this point
		mSplineElementData[splineIndex]->Contains(p,bounds,scale);	

		if (bounds.Count())
		{
			Point3 uvw = Point3(0.0f,0.0f,0.0f);
			float closestD = -1.0f;

			int closestSeg = 0;
			for (int j = 0; j < bounds.Count(); j++)
			{	
				//get our sample index
				int index = bounds[j];

				Point3 testUVW(0.0f,0.0f,0.0f);
				float testD = 0.0f;
				
				//find the closest sample
				if (mSplineElementData[splineIndex]->ProjectPoint(p, index, testUVW, testD, hitPoint,projectionType,scale,maxIterations))
				{
					if ((testD < closestD) || (closestD == -1.0f))
					{
						hitUVW = testUVW;
						closestD = testD;
						d = testD;
						hit =  true;
						closestSeg = j;
						mSplineElementData[splineIndex]->mClosestSubs.Set(closestSeg,TRUE);
					}		
					else
					{
					}
				}
			}		
				
			
		}		
		//increase our scale in case we did not find a hit
		scale *= 2.0f;
	}
	return hit;
}


BOOL SplineData::HitTestSpline(GraphicsWindow *gw,HitRegion *hr, int &hitSplineIndex, float &u)
{
	int res = 0;
	
	DWORD limit = gw->getRndLimits();
	gw->setRndLimits(( limit | GW_PICK) & ~GW_ILLUM);
	//loop through splines
	gw->setTransform(Matrix3(1));
	int numLines = mShapeCache.numLines;
	gw->setHitRegion(hr);
	gw->clearHitCode();
	
	for (int splineIndex = 0; splineIndex < numLines;splineIndex++)
	{
		if (mSplineElementData[splineIndex]->IsSelected())
		{		
			PolyLine *line = &mShapeCache.lines[splineIndex];
			Point3 plist[2];
			
			u = 0.0f;		
			for (int i = 0; i < 100; i++)
			{
				plist[0] = line->InterpCurve3D(u,POLYSHP_INTERP_NORMALIZED);
				plist[1] = line->InterpCurve3D(u+0.01f,POLYSHP_INTERP_NORMALIZED);
				u += 0.01f;
				gw->polyline(2, plist, NULL, NULL, 0);
				if (gw->checkHitCode()) 
				{
					hitSplineIndex = splineIndex;
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

BOOL SplineData::HitTestCrossSection(GraphicsWindow *gw, HitRegion hr,  SplineMapProjectionTypes projType, Tab<int> &hitSplines, Tab<int> &hitCrossSections)
{
	hitSplines.SetCount(0);
	hitCrossSections.SetCount(0);
	DWORD limit = gw->getRndLimits();
	gw->setRndLimits(( limit | GW_PICK) & ~GW_ILLUM);
	gw->setHitRegion(&hr);
	//loop through splines

	for (int splineIndex = 0; splineIndex < mSplineElementData.Count();splineIndex++)
	{

		if (mSplineElementData[splineIndex]->IsSelected())
		{			
			for (int crossSectionIndex = 0; crossSectionIndex <  NumberOfCrossSections(splineIndex); crossSectionIndex++)
			{
				SplineCrossSection section = mSplineElementData[splineIndex]->GetCrossSection(crossSectionIndex);
				Matrix3 crossSectionTM = section.mTM;
				gw->setTransform(crossSectionTM);

				gw->clearHitCode();
				mSplineElementData[splineIndex]->DisplayCrossSections(gw, crossSectionIndex,projType );
				if (gw->checkHitCode())
				{
					hitSplines.Append(1,&splineIndex,10);
					hitCrossSections.Append(1,&crossSectionIndex,10);
				}

			}
		}

	}

	return hitSplines.Count();
}

int SplineData::HitTest(ViewExp *vpt,INode *node, ModContext *mc, Matrix3 tm, HitRegion hr, int flags, SplineMapProjectionTypes projType, BOOL selectCrossSection )
{
	int res = 0;

	GraphicsWindow *gw = vpt->getGW();
	DWORD limit = gw->getRndLimits();
	gw->setRndLimits(( limit | GW_PICK) & ~GW_ILLUM);
	if (selectCrossSection)
	{
		Tab<int> hitSplines;
		Tab<int> hitCrossSections;

		if (HitTestCrossSection(gw, hr,  projType, hitSplines,hitCrossSections))
		{
			for (int i = 0; i < hitSplines.Count(); i++)
			{
				int splineIndex = hitSplines[i];
				int crossSectionIndex = hitCrossSections[i];
				if ( (mSplineElementData[splineIndex]->CrossSectionIsSelected(crossSectionIndex) && (flags&HIT_SELONLY)) ||
					!(flags&(HIT_UNSELONLY|HIT_SELONLY)))
					vpt->LogHit(node,mc,crossSectionIndex,splineIndex,NULL);
				else if ( (!mSplineElementData[splineIndex]->CrossSectionIsSelected(crossSectionIndex) && (flags&HIT_UNSELONLY)) ||
					!(flags&(HIT_UNSELONLY|HIT_SELONLY)))
					vpt->LogHit(node,mc,crossSectionIndex,splineIndex,NULL);
			}
		}
	}
	else
	{
		for (int splineIndex = 0; splineIndex < mSplineElementData.Count();splineIndex++)
		{
			gw->clearHitCode();
			mSplineElementData[splineIndex]->Display(gw,Matrix3(1),projType );
			if (gw->checkHitCode())
			{
				if ( (mSplineElementData[splineIndex]->IsSelected() && (flags&HIT_SELONLY)) ||
					!(flags&(HIT_UNSELONLY|HIT_SELONLY)))
					vpt->LogHit(node,mc,0,splineIndex,NULL);
				else if ( (!mSplineElementData[splineIndex]->IsSelected() && (flags&HIT_UNSELONLY)) ||
					!(flags&(HIT_UNSELONLY|HIT_SELONLY)))
					vpt->LogHit(node,mc,0,splineIndex,NULL);
			}

		}

	}

	gw->setRndLimits(limit);
	return res;
}

void SplineData::Display(GraphicsWindow *gw, MeshTopoData *ld, Matrix3 tm,Tab<Point3> &faceCenters,SplineMapProjectionTypes projType )
{





	Matrix3 viewTM(1);
	gw->setTransform(viewTM);

	DWORD limits = gw->getRndLimits();

	for (int i = 0; i < mSplineElementData.Count(); i++)
	{
		if (i == mActiveSpline)
		{
			gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));
			if (i < mShapeCache.numLines)
			{
				mSplineElementData[i]->Display(gw,Matrix3(1),projType );
			}
		}
	}

	gw->setRndLimits(limits);

}

void SplineData::SelectSpline(int splineIndex,bool sel)
{
	if ((splineIndex >= 0) && (splineIndex < mSplineElementData.Count()))
	{		
		mSplineElementData[splineIndex]->Select(sel);
	}
	else
	{
		DbgAssert(0);
	}
}
bool SplineData::IsSplineSelected(int splineIndex)
{
	if ((splineIndex >= 0) && (splineIndex < mSplineElementData.Count()))
	{		
		return mSplineElementData[splineIndex]->IsSelected();
	}
	return false;
}
void SplineData::CrossSectionSelect(int splineIndex,int crossSectionIndex,bool sel)
{
	if ((splineIndex >= 0) && (splineIndex < mSplineElementData.Count()))
	{		
		if (mSplineElementData[splineIndex]->IsClosed())
		{
			mSplineElementData[splineIndex]->CrossSectionSelect(crossSectionIndex,sel);
			int endSection = mSplineElementData[splineIndex]->GetNumberOfCrossSections()-1;
			if (crossSectionIndex == 0)
				mSplineElementData[splineIndex]->CrossSectionSelect(endSection,sel);
			if (crossSectionIndex == endSection)
				mSplineElementData[splineIndex]->CrossSectionSelect(0,sel);

		}
		else
			mSplineElementData[splineIndex]->CrossSectionSelect(crossSectionIndex,sel);
	}
}
bool SplineData::CrossSectionIsSelected(int splineIndex,int crossSectionIndex)
{
	if ((splineIndex >= 0) && (splineIndex < mSplineElementData.Count()))
	{		
		return mSplineElementData[splineIndex]->CrossSectionIsSelected(crossSectionIndex);
	}
	return false;
}

SplineCrossSection* SplineData::GetCrossSection(int splineIndex, int crossSectionIndex)
{
	if ((splineIndex >= 0) && (splineIndex < mSplineElementData.Count()))
	{		
		return mSplineElementData[splineIndex]->GetCrossSectionPtr(crossSectionIndex);
	}
	return NULL;
}




float	SplineData::SplineLength(int splineIndex)
{
	if ((splineIndex >= 0) && (splineIndex < mSplineElementData.Count()))
	{		
		return mSplineElementData[splineIndex]->GetSplineLength();
	}
	return 0.0f;
}

bool	SplineData::SplineClosed(int splineIndex)
{
	if ((splineIndex >= 0) && (splineIndex < mSplineElementData.Count()))
	{		
		return mSplineElementData[splineIndex]->IsClosed();
	}
	return false;
}


void SplineData::AlignCrossSection(int splineIndex, int crossSectionIndex,Point3 vec)
{
	if ((splineIndex >= 0) && (splineIndex < mSplineElementData.Count()))
	{
		int numCrossSections = NumberOfCrossSections(splineIndex);

		if ((crossSectionIndex >= 0) && (crossSectionIndex < numCrossSections))
		{

			//put the vec in spline space
			SplineCrossSection *crossSection = GetCrossSection(splineIndex,crossSectionIndex);


			Matrix3 crossSectionTM = crossSection->mTM;
			crossSectionTM.NoScale();
			crossSectionTM.NoTrans();
			Matrix3 icrossSectionTM = Inverse(crossSectionTM);


			Point3 crossSectionZVec = crossSection->mTangentNormalized;


			Point3 yvec = Normalize(vec);

			Point3 xvec = Normalize(CrossProd(yvec,crossSectionZVec));
			Point3 zvec = Normalize(CrossProd(xvec,yvec));

			Matrix3 rtm(1);
			rtm.SetRow(0,xvec);
			rtm.SetRow(1,yvec);
			rtm.SetRow(2,zvec);
			rtm.SetRow(3,Point3(0.0f,0.0f,0.0f));

			Matrix3 relativeTM =  crossSection->mIBaseTM * rtm;

			Quat q(relativeTM);


			q = TransformQuat(crossSection->mIBaseTM,q);

			crossSection->mQuat = q;

			return;
		}
	}
	DbgAssert(0);


}


void	SplineData::CreateCommandModes(UnwrapMod *mod, IObjParam *i)
{
	mAlignCommandMode = new SplineMapAlignFaceMode(mod,i);
	mAlignSectionCommandMode = new SplineMapAlignSectionMode(mod,i);
	mAddCrossSectionCommandMode = new SplineMapAddCrossSectionMode(mod,i);
}
void	SplineData::DestroyCommandModes()
{	
	if (mAlignCommandMode)
		delete mAlignCommandMode;

	if (mAlignSectionCommandMode)
		delete mAlignSectionCommandMode;

	if (mAddCrossSectionCommandMode)
		delete mAddCrossSectionCommandMode;

	mAddCrossSectionCommandMode = NULL;
	mAlignCommandMode = NULL;
	mAlignSectionCommandMode = NULL;
}

void	SplineData::AlignCommandMode()
{
	Interface *ip = GetCOREInterface();
	CommandMode *mode = ip->GetCommandMode();

	if (mode == mAlignCommandMode)
		ip->PopCommandMode();
	else
		ip->PushCommandMode(mAlignCommandMode);
}


void	SplineData::AlignSectionCommandMode()
{
	Interface *ip = GetCOREInterface();
	CommandMode *mode = ip->GetCommandMode();

	if (mode == mAlignSectionCommandMode)
		ip->PopCommandMode();
	else
		ip->PushCommandMode(mAlignSectionCommandMode);
}

void	SplineData::AddCrossSectionCommandMode()
{
	Interface *ip = GetCOREInterface();
	CommandMode *mode = ip->GetCommandMode();

	if (mode == mAddCrossSectionCommandMode)
		ip->PopCommandMode();
	else
		ip->PushCommandMode(mAddCrossSectionCommandMode);
}




BOOL	SplineData::HasCopyBuffer()
{
	return mHasCopyBuffer;
}
void	SplineData::Copy()
{
	for (int i = 0; i < NumberOfSplines(); i++ )
	{
		if (IsSplineSelected(i))
		{
			for (int j = 0; j < NumberOfCrossSections(i); j++)
			{
				if (CrossSectionIsSelected(i,j))
				{
					mCopyBuffer = *GetCrossSection(i,j);
					mHasCopyBuffer = TRUE;
					return;
				}
			}
		}
	}
}
void	SplineData::Paste()
{
	for (int i = 0; i < NumberOfSplines(); i++ )
	{
		if (IsSplineSelected(i))
		{
			for (int j = 0; j < NumberOfCrossSections(i); j++)
			{
				if (CrossSectionIsSelected(i,j))
				{
					SplineCrossSection *crossSection = GetCrossSection(i,j);
					crossSection->mScale = mCopyBuffer.mScale;
					crossSection->mQuat = mCopyBuffer.mQuat;
				}
			}
		}
	}
	RecomputeCrossSections();
}

void SplineData::PasteToSelected(int splineIndex, int crossSectionIndex)
{
	if ((splineIndex < 0) || (splineIndex >= mSplineElementData.Count()))
	{
		DbgAssert(0);
		return;
	}
	int numCross = NumberOfCrossSections(splineIndex);
	if ((crossSectionIndex < 0) || (crossSectionIndex > numCross))
	{
		DbgAssert(0);
		return;
	}

	SplineCrossSection section = mSplineElementData[splineIndex]->GetCrossSection(crossSectionIndex);
	for (int i = 0; i < NumberOfSplines(); i++ )
	{
		if (IsSplineSelected(i))
		{
			for (int j = 0; j < NumberOfCrossSections(i); j++)
			{
				if (CrossSectionIsSelected(i,j))
				{
					SplineCrossSection *crossSection = GetCrossSection(i,j);
					crossSection->mScale = section.mScale;
					crossSection->mQuat = section.mQuat;
				}
			}
		}
	}
	RecomputeCrossSections();
}


void	SplineData::DeleteCrossSection(int splineIndex, int crossSectionIndex)
{
	if ((splineIndex < 0) || (splineIndex >= mSplineElementData.Count()))
	{
		DbgAssert(0);
		return;
	}
	if ((crossSectionIndex <= 0) || (crossSectionIndex >= NumberOfCrossSections(splineIndex)-1))
	{
		DbgAssert(0);
		return;
	}

	mSplineElementData[splineIndex]->DeleteCrossSection(crossSectionIndex);

	RecomputeCrossSections();

}
void	SplineData::DeleteSelectedCrossSection()
{
	for (int i = 0; i < NumberOfSplines(); i++ )
	{
		if (IsSplineSelected(i))
		{
			for (int j = 1; j < NumberOfCrossSections(i)-1; j++)
			{
				if (CrossSectionIsSelected(i,j))
				{
					mSplineElementData[i]->DeleteCrossSection(j);
					j--;
				}
			}
		}
	}

	RecomputeCrossSections();
}





void	SplineData::InsertCrossSection(int splineIndex, float u)
{
	if ((splineIndex < 0) || (splineIndex >= mSplineElementData.Count()))
	{
		DbgAssert(0);
		return;
	}
	mSplineElementData[splineIndex]->InsertCrossSection(u);
	RecomputeCrossSections();

}


void	SplineData::SortCrossSections()
{
	for (int i = 0; i < NumberOfSplines(); i++ )
	{
		mSplineElementData[i]->SortCrossSections();
	}
	RecomputeCrossSections();
}

void	SplineData::HoldData()
{
	if (theHold.Holding())
	{
		theHold.Put(new SplineMapHoldObject(this));
	}
}

void	SplineData::CopyCrossSectionData(Tab<SplineElementData*> &data)
{
	data.SetCount(mSplineElementData.Count());
	for (int i = 0; i < mSplineElementData.Count(); i++)
	{
		if (mSplineElementData[i])
		{
			data[i] = new SplineElementData(mSplineElementData[i]);			
		}
	}
}
void	SplineData::PasteCrossSectionData(Tab<SplineElementData*> data)
{
	for (int i = 0; i < mSplineElementData.Count(); i++)
	{
		if (i < data.Count())
		{
			mSplineElementData[i]->Select(data[i]->IsSelected());
			mSplineElementData[i]->SetNumberOfCrossSections(data[i]->GetNumberOfCrossSections());
			for (int j = 0; j < data[i]->GetNumberOfCrossSections(); j++)
			{
				SplineCrossSection section = data[i]->GetCrossSection(j);
				mSplineElementData[i]->SetCrossSection(j,section.mOffset,section.mScale,section.mQuat);			
				mSplineElementData[i]->CrossSectionSelect(j,section.mIsSelected);
			}			
		}		
	}
	RecomputeCrossSections();
}


IOResult SplineData::Save(ISave *isave)
{
//save our spline element data
	ULONG nb = 0;
	int numSplines = mSplineElementData.Count();
	isave->Write(&numSplines, sizeof(numSplines), &nb);

	for (int i = 0; i < numSplines; i++)
	{
		//save selected state
		BOOL sel = mSplineElementData[i]->IsSelected();
		//save closed states
		BOOL closed = mSplineElementData[i]->IsClosed();

		int numSections = mSplineElementData[i]->GetNumberOfCrossSections();
		//save cross section count

		isave->Write(&sel, sizeof(sel), &nb);
		isave->Write(&closed, sizeof(closed), &nb);
		isave->Write(&numSections, sizeof(numSections), &nb);


		for (int j = 0; j < numSections; j++)
		{
			//save selected state
			SplineCrossSection section = mSplineElementData[i]->GetCrossSection(j);
			//save our cross section date
			isave->Write(&section, sizeof(section), &nb);

		}
		
	}

	//we need to to tell the system that we have spline data already so not to nuke it when the 
	//node and spline cache is recomputed
	mSplineDataLoaded = true;

	return IO_OK;
}
IOResult SplineData::Load(ILoad *iload, int version)
{

	IOResult res =IO_OK;
	ULONG nb = 0;



	int numSplines = 0;
	iload->Read(&numSplines, sizeof(numSplines), &nb);	

	mSplineElementData.SetCount(numSplines);
	for (int i = 0; i < numSplines; i++)
	{
		BOOL sel = FALSE;
		BOOL closed = FALSE;
		int numSections = 0;

		iload->Read(&sel, sizeof(sel), &nb);
		iload->Read(&closed, sizeof(closed), &nb);
		iload->Read(&numSections, sizeof(numSections), &nb);
		mSplineElementData[i] = new SplineElementData();
		if (closed)
			mSplineElementData[i]->SetClosed(true);
		else
			mSplineElementData[i]->SetClosed(false);
		if (sel)
			mSplineElementData[i]->Select(true);
		else
			mSplineElementData[i]->Select(false);
		mSplineElementData[i]->SetNumberOfCrossSections(numSections);		

		for (int j = 0; j < numSections; j++)
		{
			if (version == 1)
			{
				SplineCrossSectionOldV1 section;
				Quat q;
				q.Identity();
				mSplineElementData[i]->SetCrossSection(j,Point3(0.0f,0.0f,section.mU),section.mScale,q);
				mReset = true;

			}
			else
			{
				SplineCrossSection section;
				iload->Read(&section, sizeof(section), &nb);
				mSplineElementData[i]->SetCrossSection(j,section.mOffset,section.mScale,section.mQuat);
			}

		}
	}

	return res;
}


void SplineData::SetHWND(HWND hWND)
{
	mHWND = hWND;
}
HWND SplineData::GetHWND()
{
	return mHWND;
}



void	SplineData::GetSelectedSplines(Tab<int> &selSplines)
{
	selSplines.SetCount(0);
	int numSelectedSplines = 0;
	int selSpline = 0;
	for (int i = 0; i < mSplineElementData.Count(); i++)
	{
		if (mSplineElementData[i]->IsSelected())
		{
			selSplines.Append(1,&i,10);
		}
	}
}


void	SplineData::GetSelectedCrossSections(Tab<int> &selSplines, Tab<int> &selCrossSections)
{
	selSplines.SetCount(0);
	int numSelectedSplines = 0;
	int selSpline = 0;
	for (int i = 0; i < mSplineElementData.Count(); i++)
	{
		if (mSplineElementData[i]->IsSelected())
		{
			for (int j = 0; j < NumberOfCrossSections(i); j++)
			{
				if (CrossSectionIsSelected(i,j))
				{
					selCrossSections.Append(1,&j,10);
					selSplines.Append(1,&i,10);
				}
			}			
		}
	}
}

int SplineData::CurrentMode()
{
	return mCurrentMode;
}

void  SplineData::SetCurrentMode(int mode)
{
	mCurrentMode = mode;
}

CommandMode *SplineData::GetAlignSectionMode()
{
	return NULL;
}

CommandMode *SplineData::GetAlignFaceMode()
{
	return mAlignCommandMode;
}
CommandMode *SplineData::GetAddCrossSectionMode()
{
	return mAddCrossSectionCommandMode;
}


void	SplineData::MoveSelectedCrossSections(Point3 vec)
{
	

 	Tab<int> selSplines;
	Tab<int> selCrossSections;
	GetSelectedCrossSections(selSplines,selCrossSections);
	if (selSplines.Count() == 0) return;
//get the spline length
	float l = mSplineElementData[selSplines[0]]->GetSplineLength();
//get the sel cross u
	SplineCrossSection *section = GetCrossSection(selSplines[0],selCrossSections[0]);

	//transform into spline space
	Matrix3 sTM = section->mTM;
	sTM.NoScale();
	Point3 tv = VectorTransform(sTM,vec);
	//no back into our initial space
	Point3 v = VectorTransform(section->mIBaseTM,tv);

	float u = section->mOffset.z;
//compute how much u to move
	
	float incU = v.z/l;
	//move the cross sections
	for (int i = 0; i < mSplineElementData.Count(); i++)
	{
		if (mSplineElementData[i]->IsSelected())
		{
			for (int j = 0; j < (NumberOfCrossSections(i)); j++)
			{
				if (CrossSectionIsSelected(i,j))
				{
					SplineCrossSection *section = GetCrossSection(i,j);

					Matrix3 sTM = section->mTM;
					sTM.NoScale();

					tv = VectorTransform(sTM,vec);
					v = VectorTransform(section->mIBaseTM,tv);
					float tempU = section->mOffset.z + incU;
					
					if (tempU > 1.0f)
						tempU = 1.0f;
					if (tempU < 0.0f)
						tempU = 0.0f;
					float prevU = 0.0f;
					if (j != 0)
						prevU = GetCrossSection(i,j-1)->mOffset.z;
					float nextU = 1.0;
					if ((j+1) < NumberOfCrossSections(i))
						nextU = GetCrossSection(i,j+1)->mOffset.z;

					if ((tempU < nextU) && (tempU > prevU))
					{
						//we cant slide the end/start cross sections
						if ((j != 0) && (j != NumberOfCrossSections(i)-1))
							section->mOffset.z = tempU;
					}

					

					section->mOffset.x += v.x;
					section->mOffset.y += v.y;


				}
			}
		}
	}

	RecomputeCrossSections();	
}



void	SplineData::RotateSelectedCrossSections(Quat q)
{
	Tab<int> selSplines;
	Tab<int> selCrossSections;
	GetSelectedCrossSections(selSplines,selCrossSections);

	//move the cross sections
	for (int i = 0; i < selSplines.Count(); i++)
	{
		int splineIndex = selSplines[i];
		int crossSectionIndex = selCrossSections[i];
		SplineCrossSection *section = GetCrossSection(splineIndex,crossSectionIndex);

		Matrix3 sTM = section->mTM;
		sTM.NoScale();
		sTM.NoTrans();

		Quat tq = TransformQuat(sTM,q);
		//no back into our initial space
		tq = TransformQuat(section->mIBaseTM,tq);

		section->mQuat += tq;


	}

	RecomputeCrossSections();	
}


void	SplineData::ScaleSelectedCrossSections(Point2 v)
{
 	Tab<int> selSplines;
	Tab<int> selCrossSections;
	GetSelectedCrossSections(selSplines,selCrossSections);

	//move the cross sections
	for (int i = 0; i < selSplines.Count(); i++)
	{
		int splineIndex = selSplines[i];
		int crossSectionIndex = selCrossSections[i];
		SplineCrossSection *section = GetCrossSection(splineIndex,crossSectionIndex);
		section->mScale.x *= v.x;
		section->mScale.y *= v.y;
	}

	RecomputeCrossSections();	
}


Box3 SplineData::GetWorldsBounds()
{
	Box3 bounds;
	bounds.Init();

	mShapeCache.BuildBoundingBox();
	bounds = mShapeCache.bdgBox;
	float extraPadding = 0.0f;
	for (int i = 0; i < mSplineElementData.Count(); i++)
	{
		for (int j = 0; j < NumberOfCrossSections(i); j++)
		{
			SplineCrossSection *section = GetCrossSection(i,j);
			float d = section->GetLargestScale();
			if (d > extraPadding)
				extraPadding = d;
		}
	}
	
	bounds.EnlargeBy(extraPadding);


	return bounds;
}

void SplineData::Resample(int sampleCount)
{
	if (theHold.Holding())
	  HoldData();

	if (sampleCount < 2)
		sampleCount = 2;

	

	
	for (int j = 0; j < NumberOfSplines(); j++)
	{
		float u = 0.0f;
		int numSections = mSplineElementData[j]->GetNumberOfCrossSections();
		for (int i = 0; i < numSections-2; i++)
		{
			mSplineElementData[j]->DeleteCrossSection(1);
		}

		int finalSample = sampleCount;
		if (mSplineElementData[j]->IsClosed())
			finalSample = sampleCount +1;
		float inc = 1.0f/(float)(finalSample - 1);

		for (int i = 0; i < finalSample-2; i++)
		{
			u += inc;
			mSplineElementData[j]->InsertCrossSection(u);
		}
		mSplineElementData[j]->UpdateCrossSectionData();


	}

	
}

void	SplineData::SetMappingType(int type)
{
	for (int j = 0; j < NumberOfSplines(); j++)
	{		
		mSplineElementData[j]->mAdvanceSplines = type;
	}
}


void	SplineData::SetPlanarMapDirection(BOOL up)
{
	mPlanarMapUp = up;
	
}

