/********************************************************************** *<
FILE: ToolSplineMapping.cpp

DESCRIPTION: This contains all our spline cross section and spline element methods


HISTORY: 12/31/96
CREATED BY: Peter Watje

Spline mapping is using a spline to create our UVW projection.  Think of it as bending a 
cylindrical mapping along a spline.  The basics are pretty simple, we take our chape object
and sample it down to a polyline.  We then sample the polyline and at each sample we create
tm and cross section.  When we map the mesh we take each geom vertex and find the closest sample.
The distance along the curve is our V value, the U value is then either the local X if we are 
planar or the distance around the sample if we are circular.

*>	Copyright (c) 2007, All Rights Reserved.
**********************************************************************/


#include "unwrap.h"

/*

set the selected spline to the hit sub spline element




To Do 
Unit Test								5

*/



//just used to generate our random colors 
Random rndColor;


//simple dist form point to line segment
float LineToPoint(Point3 p1, Point3 l1, Point3 l2, float &u)
{
	Point3 VectorA,VectorB,VectorC;
	double Angle;
	double dist = 0.0f;
	VectorA = l2-l1;
	VectorB = p1-l1;
	float dot = DotProd(Normalize(VectorA),Normalize(VectorB));
	if (dot >= 1.0f) 
		Angle = 0.0f;
	else
		Angle =  acos(dot);

	if (Angle > (PI/2.0))
	{
		dist = Length(p1-l1);
		u = 0.0;
	}
	else
	{
		VectorA = l1-l2;
		VectorB = p1-l2;
		dot = DotProd(Normalize(VectorA),Normalize(VectorB));
		if (dot >= 1.0f) 
			Angle = 0.0f;
		else
			Angle = acos(dot);
		if (Angle > (PI/2.0))
		{
			dist = Length(p1-l2);
			u = 1.0f;
		}
		else
		{
			double hyp;
			hyp = Length(VectorB);
			dist =  sin(Angle) * hyp;
			double du =  (cos(Angle) * hyp);
			double a = Length(VectorA);
			if ( a== 0.0f)
				return 0.0f;
			else u = (float)((a-du) / a);
		}
	}
	return (float) dist;
}



SplineCrossSection::SplineCrossSection()
{
	mIsSelected = false;
	mScale.x = 10.0f;
	mScale.y = 10.0f;
	mOffset.x = 0.0f;
	mOffset.y = 0.0f;
	mOffset.z = 0.0f;
	mQuat.Identity();
}

float SplineCrossSection::GetLargestScale()
{
	if (mScale.x > mScale.y)
		return mScale.x;
	else
		return mScale.y;
}

void	SplineCrossSection::Dump()
{
	DebugPrint(_T("CrossSection DUMP Offset %f %f %f  XScale %f YScale %f  Quat %f %f %f   %f \n"),mOffset.x,mOffset.y,mOffset.z,mScale.x,mScale.y,mQuat.x,mQuat.y,mQuat.z,mQuat.w);
	DebugPrint(_T("              Tangent %f %f %f\n"),mTangent.x,mTangent.y,mTangent.z);
	DebugPrint(_T("              NTangent %f %f %f\n"),mTangentNormalized.x,mTangentNormalized.y,mTangentNormalized.z);

	for (int i = 0; i < 3; i++)
	{
		Point3 vec = mTM.GetRow(i);
		DebugPrint(_T("              %f %f %f\n"),vec.x,vec.y,vec.z);
	}
}


void SplineCrossSection::ComputeTransform(Point3 upVec)
{
	Point3 xVec(0.0f,0.0f,1.0f);
	Point3 yVec(0.0f,0.0f,1.0f);
//builds a transform based on the up vector and the tangent of the cross section
	yVec = Normalize(CrossProd(mTangentNormalized,upVec)) ;
	xVec = Normalize(CrossProd(yVec,mTangentNormalized)) ;

	mTM.SetRow(0,xVec);
	mTM.SetRow(1,yVec);
	mTM.SetRow(2,mTangentNormalized);

	mTM.SetRow(3,Point3(0.0f,0.0f,0.0f));

	//store off this initial transform
	mBaseTM = mTM;
	mIBaseTM = Inverse(mTM);

	//apply our offset
	Point3 vec(0.0f,0.0f,0.0f);
	vec += Normalize(mTM.GetRow(0)) * mOffset.x;
	vec += Normalize(mTM.GetRow(1)) * mOffset.y;

	mTM.SetRow(3,mP + vec);
	
	//rotate the matrix
	PreRotateMatrix(mTM, mQuat);

	//apply our scale
	mTM.SetRow(0,mTM.GetRow(0) * mScale.x);
	mTM.SetRow(1,mTM.GetRow(1) * mScale.y);
	mTM.SetRow(2,mTM.GetRow(2) * Length(mTangent));



	mITM = Inverse(mTM);
}

SplineCrossSection	SplineCrossSection::Interp(SplineCrossSection next, float per)
{
	SplineCrossSection interp;


	interp.mOffset = mOffset + (next.mOffset - mOffset) * per;
	interp.mScale =  mScale + (next.mScale-mScale)*per;
	interp.mQuat = Slerp(mQuat, next.mQuat, per);
	interp.mP = mP + (next.mP - mP) * per;

	interp.mTangent =  mTangent + (next.mTangent-mTangent)*per;
	interp.mTangentNormalized =  mTangentNormalized + (next.mTangentNormalized-mTangentNormalized)*per;
	interp.mTangentNormalized = Normalize(interp.mTangentNormalized);

	return interp;
}


SplineCrossSection	SplineCrossSection::InterpOnlyMatrix(SplineCrossSection next, float per)
{
	SplineCrossSection interp;

	Matrix3 currentTM = mTM;
	Matrix3 nextTM = next.mTM;
	Matrix3 newTM(1);
	for (int i = 0; i < 4; i++)
	{
		Point3 vec1 = currentTM.GetRow(i) * (1.0f-per) ;
		Point3 vec2 = nextTM.GetRow(i) * per;
		newTM.SetRow(i,vec1 + vec2);
	}
	float scales[3];
	for (int i = 0; i < 3; i++)
	{
		scales[i] = Length(newTM.GetRow(i));
	}

	newTM.Orthogonalize();

	for (int i = 0; i < 3; i++)
	{
		newTM.SetRow(i,newTM.GetRow(i) * scales[i]);
	}

	interp.mTM = newTM;
	interp.mITM = Inverse(newTM);
	



	interp.mOffset = mOffset + (next.mOffset - mOffset) * per;
	interp.mScale =  mScale + (next.mScale-mScale)*per;
	interp.mQuat = Slerp(mQuat, next.mQuat, per);
	interp.mP = mP + (next.mP - mP) * per;

	interp.mTangent =  mTangent + (next.mTangent-mTangent)*per;
	interp.mTangentNormalized =  mTangentNormalized + (next.mTangentNormalized-mTangentNormalized)*per;
	interp.mTangentNormalized = Normalize(interp.mTangentNormalized);


	return interp;
}



SplineElementData::SplineElementData()
{
	mIsClosed = false;
	mIsSelected = false;
	mColor = Point3(1.0f,1.0f,0.0f);

	static Point3 baseColor(0.0f,0.0f,0.0f);
	static int count = 0;
	//get our color
	BOOL done = FALSE;

	COLORREF egdeSelectedColor = ColorMan()->GetColor(GEOMEDGECOLORID);
	COLORREF seamColorRef = ColorMan()->GetColor(PELTSEAMCOLORID);
	COLORREF openEdgeColorRef = ColorMan()->GetColor(OPENEDGECOLORID);
	

	Color occuppiedColors[5];

	int numColors = 4;
	occuppiedColors[0] = Color(seamColorRef);
	occuppiedColors[1] = Color(egdeSelectedColor);
	occuppiedColors[2] = GetUIColor(COLOR_SEL_GIZMOS);
	occuppiedColors[3] = Color(openEdgeColorRef);

	Color sampleColors[13];
	sampleColors[0] = Color(0.0f,0.0f,0.0f);
	sampleColors[1] = Color(1.0f,0.0f,0.5f);
	sampleColors[2] = Color(0.0f,1.0f,0.0f);
	sampleColors[3] = Color(0.0f,0.0f,1.0f);
	sampleColors[4] = Color(0.0f,1.0f,1.0f);
	sampleColors[5] = Color(1.0f,1.0f,0.0f);
	sampleColors[6] = Color(1.0f,0.0f,1.0f);
	sampleColors[7] = Color(0.5f,0.0f,0.0f);
	sampleColors[8] = Color(0.0f,0.5f,0.0f);
	sampleColors[9] = Color(0.0f,0.0f,0.5f);
	sampleColors[10] = Color(0.5f,0.5f,0.0f);
	sampleColors[11] = Color(0.0f,0.5f,0.5f);
	sampleColors[12] = Color(0.5f,0.0f,0.5f);

	
	while (!done)
	{
		BOOL colorOK = TRUE;
		for (int i = 0; i < numColors; i++)
		{
			float d = Length(occuppiedColors[i]-baseColor);
			if (d < 0.25f)
				colorOK = FALSE;
		}
		if (colorOK)
		{			
			count++;
			done = TRUE;
			mColor = baseColor;
			if (count < 13)
				baseColor = sampleColors[count];
			else
			{
				baseColor.x = rndColor.getf();
				baseColor.y = rndColor.getf();
				baseColor.z = rndColor.getf();
			}
		}
		else
		{
			if (count >= 13)
			{
				mColor.x = rndColor.getf();
				mColor.y = rndColor.getf();
				mColor.z = rndColor.getf();
				done = TRUE;
			}
			else
			{
				baseColor = sampleColors[count];
			}
			count++;
		}
	}

	

	mLine = NULL;
	mSplineLength = 0.0f;
	mPlanarMapUp = TRUE;
	mAdvanceSplines = TRUE;
}

SplineElementData::SplineElementData( SplineElementData *splineData)
{
	mIsClosed = splineData->IsClosed();
	mIsSelected = splineData->IsSelected();
	mCrossSectionData.SetCount(splineData->GetNumberOfCrossSections());
	for (int i = 0; i < splineData->GetNumberOfCrossSections(); i++)
	{
		mCrossSectionData[i] = splineData->GetCrossSection(i);
	}

}

SplineElementData::~SplineElementData()
{
	for (int i = 0 ; i < mBorderLines.Count(); i++)
		delete mBorderLines[i];
}

float SplineElementData::GetSplineLength()
{
	return mSplineLength;
}
void SplineElementData::SetSplineLength(float value)
{
	mSplineLength = value;
}

bool	SplineElementData::IsClosed()
{
	return mIsClosed;
}
void	SplineElementData::SetClosed(bool closed)
{
	mIsClosed = closed;
}


Point3	SplineElementData::GetColor()
{
	return mColor;
}

int		SplineElementData::GetNumCurves()
{
	return mCurves;
}
void	SplineElementData::SetNumCurves(int num)
{
	mCurves = num;
}


void SplineElementData::Select(bool sel)
{
	mIsSelected = sel;
}
bool SplineElementData::IsSelected()
{
	return mIsSelected;	
}

void SplineElementData::CrossSectionSelect(int crossSectionIndex,bool sel)
{
	if ((crossSectionIndex < 0) || (crossSectionIndex >= mCrossSectionData.Count()))
	{
		DbgAssert(0);
	}
	else
		mCrossSectionData[crossSectionIndex].mIsSelected = sel;
}
bool SplineElementData::CrossSectionIsSelected(int crossSectionIndex)
{
	if ((crossSectionIndex < 0) || (crossSectionIndex >= mCrossSectionData.Count()))
	{
		DbgAssert(0);
		return false;
	}
	else
		return mCrossSectionData[crossSectionIndex].mIsSelected;
}

void	SplineElementData::SetNumberOfCrossSections(int count)
{
	mCrossSectionData.SetCount(count);
	for (int i = 0; i < count; i++)
		mCrossSectionData[i].mIsSelected = false;
}

int	SplineElementData::GetNumberOfCrossSections()
{
	return mCrossSectionData.Count();
}

SplineCrossSection	SplineElementData::GetCrossSection(int index)
{
	if ((index < 0) || (index >= mCrossSectionData.Count()))
	{
		DbgAssert(0);
		//! NH What to do here ??
		return mCrossSectionData[0];
	}
	return mCrossSectionData[index];
}

SplineCrossSection*	SplineElementData::GetCrossSectionPtr(int index)
{
	if ((index < 0) || (index >= mCrossSectionData.Count()))
	{
		DbgAssert(0);
		return NULL;
	}

	return &mCrossSectionData[index];
}

SplineCrossSection	SplineElementData::GetCrossSection(float u)
{
	if (u <= 0.0f)
	{
		return mCrossSectionData[0];
	}
	else if (u >= 1.0f)
	{
		return mCrossSectionData[mCrossSectionData.Count()-1];
	}
	else
	{
		int ct = 0;
		while ((ct < mCrossSectionData.Count()) && (mCrossSectionData[ct].mOffset.z < u))
			ct++;
		ct--;
		

		float startU = mCrossSectionData[ct].mOffset.z;
		float endU = mCrossSectionData[ct+1].mOffset.z;
		float total = endU-startU;
		float per = u - startU;
		per = per/total;

		SplineCrossSection interp =  mCrossSectionData[ct].Interp(mCrossSectionData[ct+1],per);


		return interp;
	}
}


SplineCrossSection	SplineElementData::GetSubCrossSection(float u)
{
	if (u <= 0.0f)
	{
		return mSubBounds[0].mCrossSection;
	}
	else if (u >= 1.0f)
	{
		return mSubBounds[mSubBounds.Count()-1].mCrossSection;
	}
	else
	{
		
		int start = 0;
		int mid = mSubBounds.Count()/2;
		int end = mSubBounds.Count();

		BOOL done = FALSE;
		if (u < mSubBounds[1].mCrossSection.mOffset.z)
		{
			mid = 0;
		}
		else if (u >= mSubBounds[mSubBounds.Count()-2].mCrossSection.mOffset.z)
		{
			mid = mSubBounds.Count()-2;
		}
		else
		{
			while (!done)
			{
				if (u < mSubBounds[mid].mCrossSection.mOffset.z)
				{
					end = mid;
					mid = (start+end)/2;
				}
				else
				{
					start = mid;
					mid = (start+end)/2;
				}
				if ( (u < mSubBounds[mid+1].mCrossSection.mOffset.z) &&
					 ( u >= mSubBounds[mid].mCrossSection.mOffset.z) )
					 done = TRUE;

			}
		}
		

		int ct = mid;
		float startU = mSubBounds[ct].mCrossSection.mOffset.z;
		float endU = mSubBounds[ct+1].mCrossSection.mOffset.z;


		float total = endU-startU;
		float per = u - startU;
		per = per/total;

		SplineCrossSection interp;
		SplineCrossSection testInterp;

		if (mAdvanceSplines)
		{
			interp =  mSubBounds[ct].mCrossSection.InterpOnlyMatrix(mSubBounds[ct+1].mCrossSection,per);
		}
		else
			interp =  mSubBounds[ct].mCrossSection.Interp(mSubBounds[ct+1].mCrossSection,per);
		


		return interp;
	}
}

void	SplineElementData::SetCrossSection(int index, Point3 p,   Point2 scale, Quat q)
{
	if ((index >= 0) && (index < mCrossSectionData.Count()))
	{
		mCrossSectionData[index].mOffset = p;		
//		mCrossSectionData[index].mTwist = twist;		
		mCrossSectionData[index].mScale = scale;
		mCrossSectionData[index].mQuat = q;
	}
}

void	SplineElementData::ResetBoundingBox()
{
	mBounds.Init();
}
void	SplineElementData::SetBoundingBox(Box3 bounds)
{
	mBounds = bounds;
}
Box3	SplineElementData::GetBoundingBox()
{
	return mBounds;
}



void	SplineElementData::SetNumberOfSubBoundingBoxes(int ct)
{
	mSubBounds.SetCount(ct);
	for (int i = 0; i < ct; i++)
		mSubBounds[i].mBounds.Init();
}

int	SplineElementData::GetNumberOfSubBoundingBoxes()
{
	return mSubBounds.Count();
}
void	SplineElementData::SetSubBoundingBoxes(int index, Box3 bounds, float u, SplineCrossSection crossSection )
{
	if ((index >= 0) && (index< mSubBounds.Count()))
	{
		mSubBounds[index].mBounds = bounds;
		mSubBounds[index].mCrossSection = crossSection;
		mSubBounds[index].mCrossSection.mOffset.z = u;
	}
	else
	{
		DbgAssert(0);
	}
}
SubSplineBoundsInfo	SplineElementData::GetSubBoundingBoxes(int index)
{
	if ((index >= 0) && (index< mSubBounds.Count()))
	{
		return mSubBounds[index];
	}
	else
	{
		DbgAssert(0);
	}

	SubSplineBoundsInfo error;
	error.mBounds.Init();
	error.mCrossSection.mOffset.z = -1.0f;
	return error;
}

Point3 SplineElementData::GetAdvanceCenter(int seg,float u)
{
	if (u < 0.0f)
		u = 0.0f;
	if (u > 1.0f)
		u = 1.0f;
	int segment = 0;
	segment = seg*4;

	Point3 x1 = mBorderLines[segment]->InterpCurve3D(u,POLYSHP_INTERP_NORMALIZED);
	Point3 x2 = mBorderLines[segment+2]->InterpCurve3D(u,POLYSHP_INTERP_NORMALIZED);

	Point3 y1 = mBorderLines[segment+1]->InterpCurve3D(u,POLYSHP_INTERP_NORMALIZED);
	Point3 y2 = mBorderLines[segment+3]->InterpCurve3D(u,POLYSHP_INTERP_NORMALIZED);

	return ((x1+x2)/2.0f + (y1+y2)/2.0f) * 0.5f;

}

Point3 SplineElementData::GetAdvanceTangent(int seg,float u, Point3 &xVec, Point3 &yVec)
{
	if (u < 0.0f)
		u = 0.0f;
	if (u > 1.0f)
		u = 1.0f;

	int segment  = 0;
	segment = seg * 4;

	Point3 x1 = mBorderLines[segment + 0]->InterpCurve3D(u,POLYSHP_INTERP_NORMALIZED);
	Point3 x2 = mBorderLines[segment + 2]->InterpCurve3D(u,POLYSHP_INTERP_NORMALIZED);

	Point3 y1 = mBorderLines[segment + 1]->InterpCurve3D(u,POLYSHP_INTERP_NORMALIZED);
	Point3 y2 = mBorderLines[segment + 3]->InterpCurve3D(u,POLYSHP_INTERP_NORMALIZED);

	xVec = (x1-x2) * 0.5f;
	yVec = (y1-y2) * 0.5f;

	x1 = Normalize(x2 - x1);
	y1 = Normalize(y2 - y1);


	return Normalize(CrossProd(x1,y1));

}


class CrossSectionIntersect
{
public:
	CrossSectionIntersect()
	{
		Init();
	}
	void Init()
	{
		mTM.IdentityMatrix();
		mITM.IdentityMatrix();
		for (int i = 0; i < 4; i++)
		{
			mAxis[i] = Point3(0.0f,0.0f,0.0f);
			mSkip[i] = FALSE;
			mMajorCrossSection = FALSE;
		}
	}
	Matrix3 mTM;
	Matrix3 mITM;
	Point3 mAxis[4]; //+x,+y,-x,-y
	BOOL	mSkip[4];
	BOOL	mMajorCrossSection;

};

void SplineElementData::UpdateCrossSectionData()
{
	for (int j = 0; j < GetNumberOfCrossSections(); j++)
	{
		float u = mCrossSectionData[j].mOffset.z;

		
			Point3 p = mLine->InterpCurve3D(u,POLYSHP_INTERP_NORMALIZED);

			if ( (u > 0.05f) && (u < 0.95f))
			{		
				Point3 tangentA = mLine->InterpCurve3D(u-0.005f,POLYSHP_INTERP_NORMALIZED);
				Point3 tangentB = mLine->InterpCurve3D(u+0.005f,POLYSHP_INTERP_NORMALIZED);
				mCrossSectionData[j].mTangent = Normalize(tangentB - tangentA);
				mCrossSectionData[j].mTangentNormalized = mCrossSectionData[j].mTangent;
			}
			else if (u <= 0.05f) 
			{
				Point3 tangentB = mLine->InterpCurve3D(u+0.005f,POLYSHP_INTERP_NORMALIZED);
				mCrossSectionData[j].mTangent = Normalize(tangentB - p);
				mCrossSectionData[j].mTangentNormalized = mCrossSectionData[j].mTangent;

			}

			else if (u >= 0.95f) 
			{
				Point3 tangentB = mLine->InterpCurve3D(u-0.005f,POLYSHP_INTERP_NORMALIZED);
				mCrossSectionData[j].mTangent = Normalize(p - tangentB);
				mCrossSectionData[j].mTangentNormalized = mCrossSectionData[j].mTangent;

			}

			mCrossSectionData[j].mP = p;
			
		mCrossSectionData[j].ComputeTransform(mUpVec);

		if (mIsClosed)
		{
			mCrossSectionData[mCrossSectionData.Count()-1] = mCrossSectionData[0];
			mCrossSectionData[mCrossSectionData.Count()-1].mOffset.z = 1.0f;
		}
	
	}


	//loop through segments
	int numSegments = GetNumberOfCrossSections()-1;
	int samples = 32;

	for (int j = 0; j < mBorderLines.Count(); j++)
		delete mBorderLines[j];
	mBorderLines.SetCount(0);

	Tab<CrossSectionIntersect> sectionTests;
	for (int j = 0; j < numSegments; j++)
	{
		//get our start cross section for this segment
		//get our end cross section for this segment

		SplineCrossSection startSection = GetCrossSection(j);
		SplineCrossSection endSection = GetCrossSection(j+1);
		float startU = startSection.mOffset.z;
		float endU = endSection.mOffset.z;
		//loop through our sample count
		//build our center poly line
		PolyLine centerLine;
		float u = startU;
		float inc = (endU-startU)/float(samples-1);

		PolyLine xPos,xNeg,yPos,yNeg;
//build a list of tm and border points;
//iterate through find a point that intersects the previous plane
		//tag it and the previous point as bad
		//go foward one sample and back one
		//see if anything intersects
		//if so repeat until no intersections
		int numSamples = (samples-1);
		if ( j == (numSegments-1))
			numSamples++;

		for (int i = 0; i < numSamples; i++)
		{
			//sample our center to a new polyline
			SplineCrossSection section;
			
			if (i == 0) 
			{
				section = GetCrossSection(j);
			}
			else
			{
				section = GetCrossSection(u);

				section.mP = mLine->InterpCurve3D(u,POLYSHP_INTERP_NORMALIZED);
				section.mTangent = mLine->TangentCurve3D(u,POLYSHP_INTERP_NORMALIZED);
				section.mTangentNormalized = Normalize(section.mTangent);

				section.ComputeTransform(mUpVec);
			}
			
			Point3 p[4];
			p[0] = section.mTM.GetRow(3) + section.mTM.GetRow(0);
			p[1] = section.mTM.GetRow(3) + section.mTM.GetRow(1);
			p[2] = section.mTM.GetRow(3) - section.mTM.GetRow(0);
			p[3] = section.mTM.GetRow(3) - section.mTM.GetRow(1);

			CrossSectionIntersect data;
			data.Init();
			data.mTM = section.mTM;
			data.mITM = Inverse(section.mTM);
			data.mAxis[0] = p[0];
			data.mAxis[1] = p[1];
			data.mAxis[2] = p[2];
			data.mAxis[3] = p[3];
			data.mMajorCrossSection = FALSE;
			if (i == 0)
				data.mMajorCrossSection = TRUE;
			sectionTests.Append(1,&data,100);
			u += inc;
		}
	}
	//now loop through looking for intersections

	for (int i = 1; i < sectionTests.Count(); i++)
	{
		Matrix3 currentITM = sectionTests[i].mITM;
		for (int k = -samples; k < samples; k++)
		{
			int sectionID = (i + k);//%sectionTests.Count();
			if ( (sectionID < sectionTests.Count() && sectionID >= 0 ) &&
				 (sectionID != i) )
			{
				BOOL hit = FALSE;
				//see if we intersected the previous cross section
				Point3 center = sectionTests[sectionID].mTM.GetRow(3) * currentITM;
				for (int j = 0; j < 4; j++)
				{
					{
						Point3 p = sectionTests[sectionID].mAxis[j] * currentITM;
						BOOL possibleHit = FALSE;
						if (center.z < 0.0f)
						{	
							if (p.z > 0.0f)
								possibleHit = TRUE;
						}
						else
						{
							if (p.z < 0.0f)
								possibleHit = TRUE;
						}

						if (possibleHit)
						{
							p.z = 0.0f;
							float d = Length(p);
							if (d < 1.0f)
							{
//we intersected now marker the points that intersected to be skipped
								sectionTests[i].mSkip[j] = TRUE;
								sectionTests[i].mMajorCrossSection = FALSE;
								sectionTests[sectionID].mSkip[j] = TRUE;
								sectionTests[sectionID].mMajorCrossSection = FALSE;
								
							}
						}
					}
				}
			}
		}
	}
//we need to go back and reset the major crossection where we deleted points
	for (int i = 0; i < sectionTests.Count()-1; i++)
	{
		int cCt = 0;
		int nCt = 0;
		for (int j = 0; j < 4; j++)
		{
			if (sectionTests[i].mSkip[j])
				cCt++;
			if (sectionTests[i+1].mSkip[j])
				nCt++;
		}
		if ((cCt == 0) && (nCt > 0))
			sectionTests[i].mMajorCrossSection = TRUE;

		if ((nCt == 0) && (cCt > 0))
			sectionTests[i+1].mMajorCrossSection = TRUE;
	}


	//nowbuild our boundary
	PolyLine*  bLines[4];
	for (int i = 0; i < 4; i++)
		bLines[i] = new PolyLine();

	int startIndex = 0;
	for (int i = 0; i < sectionTests.Count(); i++)
	{
		//add the points to the current list
		for (int j = 0; j < 4; j++)
		{
			PolyPt pt(PolyPt(sectionTests[i].mAxis[j]));
			pt.SetMatID(0);
			if (sectionTests[i].mSkip[j])
			{
				if (startIndex == 0)
					bLines[j]->Append(pt);
				pt.SetMatID(1);
			}
			else
				bLines[j]->Append(pt);
		}
		startIndex++;

		//see if we have a major section
		if ((sectionTests[i].mMajorCrossSection && (i!=0)) ||
			(i==sectionTests.Count()-1))
		{
		//if so create new poly lines
			for (int j = 0; j < 4; j++)
			{
				PolyLine* newPolyLine = bLines[j];
				mBorderLines.Append(1,&newPolyLine,40);
				if (i != sectionTests.Count() -1)
				{
					bLines[j] = new PolyLine();
					bLines[j]->Append(PolyPt(sectionTests[i].mAxis[j]));
				}
			}
			startIndex = 0;
		}
	}
	//loop from major section to major section
	if (mLine)
	{

		Box3 bounds;
		bounds.Init();
		float lineLength = mLine->CurveLength();
		for (int j = 0; j < mLine->numPts; j++)
		{
			bounds += mLine->pts[j].p;
		}

		float enlargeBy = 0.0f;
		for (int j = 0; j < GetNumberOfCrossSections(); j++)
		{
			SplineCrossSection section = GetCrossSection(j);

			Matrix3 tm = section.mTM;

			bounds += tm.GetRow(3);

			for (int m = 0; m < 3; m ++)
			{
				float l = Length(tm.GetRow(m));
				if (l > enlargeBy)
					enlargeBy = l; 
			}
		}

		bounds.EnlargeBy(enlargeBy);
		SetBoundingBox(bounds);

		//compute our sub section bounding boxes


		float runningLength =  0.0f;

		if (mAdvanceSplines)
		{
			int splineSegments = mBorderLines.Count()/4;//GetNumberOfCrossSections()-1;//[4].Segments();
			int interp = 32;
			int segments = interp * splineSegments;
			float inc = 1.0f/segments;
			float u = 0.0f;

			//compute our spline distance


			mSubBounds.SetCount(segments+1);
			int id = 0;
			
			for (int j = 0; j < splineSegments; j++)
			{
//compute our segment distance
				int segcount = interp;
				float segInc = 1.0f/segcount;
				if (j == splineSegments-1)
					segcount += 1;


				float segU = 0.0f;
				
				for (int i = 0; i < segcount; i++)
				{

					SplineCrossSection crossSection;// = GetCrossSection(u);
					Point3 center = GetAdvanceCenter(j,segU);
					Point3 xVec,yVec;
					Point3 tangent = GetAdvanceTangent(j,segU,xVec,yVec);
					
					
					crossSection.mP = center;
					crossSection.mTangentNormalized = tangent;

					float l = Length(center - GetAdvanceCenter(j,segU+segInc));
					if (l == 0.0f)
						l = 1.0f;
					crossSection.mTangent = tangent * l;				
					crossSection.mOffset = Point3(0.0f,0.0f,0.0f);
					crossSection.mQuat.Identity();

					Matrix3 tm;
					tm.SetRow(0,xVec);
					tm.SetRow(1,yVec);
					tm.SetRow(2,crossSection.mTangent);
					tm.SetRow(3,center);

					crossSection.mTM = tm;
					crossSection.mITM = Inverse(tm);


					mSubBounds[id].mCrossSection = crossSection;
					mSubBounds[id++].mCrossSection.mOffset.z = 0.0f;

					segU += segInc;					
				}
			}

			float runningLength = 0.0f;
			for (int j = 1; j < mSubBounds.Count(); j++)
			{
				Point3 p1 = mSubBounds[j].mCrossSection.mTM.GetRow(3);
				Point3 p2 = mSubBounds[j-1].mCrossSection.mTM.GetRow(3);
				runningLength += Length(p2-p1);
				mSubBounds[j].mCrossSection.mOffset.z = runningLength;
			}

			if (runningLength > 0.0f)
			{
				for (int j = 1; j < mSubBounds.Count(); j++)
				{
					float v = mSubBounds[j].mCrossSection.mOffset.z;
					v = v/runningLength;
					mSubBounds[j].mCrossSection.mOffset.z = v;
				}
			}


			mSegmentBounds.SetCount(0);
			Box3 segBounds;
			segBounds.Init();
			int s;
			s = 0;

			for (int i = 0; i <= segments; i++)
			{
				Point3 currentPoint = mSubBounds[i].mCrossSection.mP;
				Point3 nextPoint(0.0f,0.0f,0.0f);
				if (i == segments)
				{
					nextPoint = mSubBounds[i].mCrossSection.mP;
				}
				else
				{
					nextPoint = mSubBounds[i+1].mCrossSection.mP;
				}

				runningLength += Length(nextPoint-currentPoint);
				
				Box3 bounds;
				bounds.Init();
				bounds += nextPoint;
				bounds += currentPoint;

				SplineCrossSection currentCrossSection = mSubBounds[i].mCrossSection;

				
				Box3 b;
				b += Point3(1.0f,1.0f,1.0f);
				b += Point3(-1.0f,-1.0f,-1.0f);
				bounds += b * currentCrossSection.mTM;

				mSubBounds[i].mBounds = bounds;

				segBounds += bounds;
				int sampleRate = 16;
				if ((i % (sampleRate) == (sampleRate-1)))
				{
					SegmentBoundingBox d;
					d.mBounds = segBounds;
					d.mStart = s;
					d.mEnd = i;
					mSegmentBounds.Append(1,&d,100);
					segBounds.Init();
					s = i+1;
				}
				else if (i == segments)
				{
					SegmentBoundingBox d;
					d.mBounds = segBounds;
					d.mStart = s;
					d.mEnd = segments;
					mSegmentBounds.Append(1,&d,100);
					segBounds.Init();
				}

			}
			mSplineLength = runningLength;
		}
		else
		{
			int numberOfPoints = mLine->numPts;
			float u = 0.0f;
			Point3 lastPoint(0.0f,0.0f,0.0f);			
			SplineCrossSection lastCrossSection = GetCrossSection(0);

			lastCrossSection.mP = mLine->pts[0].p;
			lastCrossSection.mTangent = (mLine->pts[1].p - mLine->pts[0].p);
			lastCrossSection.mTangentNormalized = Normalize(lastCrossSection.mTangent);

			lastCrossSection.ComputeTransform(mUpVec);
			lastPoint = mLine->pts[0].p;

			int numberOfRealPoints = numberOfPoints;
			if (mIsClosed)
				numberOfPoints += 1;

			SetNumberOfSubBoundingBoxes(numberOfPoints-1);

			mSegmentBounds.SetCount(0);
			Box3 segBounds;
			segBounds.Init();
			int s,e;
			s = 0;
			e = 0;

			for (int j = 1; j < numberOfPoints; j++)
			{
				float lastU = u;
				
				Point3 currentPoint(0.0f,0.0f,0.0f);

				if (j >= numberOfRealPoints)
					currentPoint = mLine->pts[0].p;
				else
					currentPoint = mLine->pts[j].p;

				float d = Length(lastPoint - currentPoint);
				runningLength += d;
				
				u = runningLength/lineLength;

				SplineCrossSection currentCrossSection = GetCrossSection(u);



				currentCrossSection.mP = mLine->pts[(j) % numberOfRealPoints].p;
				currentCrossSection.mTangent = (mLine->pts[(j+1)% numberOfRealPoints].p - mLine->pts[(j)% numberOfRealPoints].p);
				currentCrossSection.mTangentNormalized = Normalize(currentCrossSection.mTangent);

				currentCrossSection.ComputeTransform(mUpVec);

				//build the bounding box
				Box3 bounds;
				bounds.Init();
				bounds += lastPoint;
				bounds += currentPoint;

				
				Box3 b;
				b += Point3(1.0f,1.0f,1.0f);
				b += Point3(-1.0f,-1.0f,-1.0f);
				bounds += b * lastCrossSection.mTM;


				if ((j < numberOfPoints-1) || mIsClosed)
					bounds += b * currentCrossSection.mTM;

				SetSubBoundingBoxes(j-1,bounds,lastU, lastCrossSection);

				segBounds += bounds;
				int sampleRate = 16;
				if ((j % (sampleRate) == 0))
				{
					SegmentBoundingBox d;
					d.mBounds = segBounds;
					d.mStart = s;
					d.mEnd = j-1;
					mSegmentBounds.Append(1,&d,100);
					segBounds.Init();
					s = j-1;
				}
				else if (j == numberOfPoints-1)
				{
					SegmentBoundingBox d;
					d.mBounds = segBounds;
					d.mStart = s;
					d.mEnd = j-1;
					mSegmentBounds.Append(1,&d,100);
					segBounds.Init();
					s = j-1;
				}
				lastPoint = currentPoint;
				lastCrossSection = currentCrossSection;
			}
			mSplineLength = runningLength;
	
		}
	}

}


void SplineElementData::Contains(Point3 p,Tab<int> &hits, float scale)
{
	hits.SetCount(0,FALSE);
	int lastHit = -1;
	for (int i = 0; i < mSegmentBounds.Count(); i++)
	{
		Box3 b = mSegmentBounds[i].mBounds;
		b.Scale(scale);
		if (b.Contains(p))
		{
			int s = mSegmentBounds[i].mStart;
			int e = mSegmentBounds[i].mEnd;
			for (int j = s; j <= e; j++)
			{
				if (j != lastHit)
				{		
					Box3 subB = mSubBounds[j].mBounds;
					subB.Scale(scale);
					if (subB.Contains(p))
					{
						hits.Append(1,&j,100);
						lastHit = j;
					}
				}
			}
		}
	}	
}

bool SplineElementData::ProjectPoint(Point3 p, int subSample, Point3 &uvw, float &d, Point3 &hitPoint, SplineMapProjectionTypes projectionType, float scale, int maxIterations)
{
	bool hit = false;

	BOOL done = FALSE;
	float splineU = 0.0f;
	float perFromSpline = 0.0f;

	BOOL pastEnd = FALSE;

	int iter = 0;
	
	SplineCrossSection	startSection = GetSubBoundingBoxes(subSample).mCrossSection;
	SplineCrossSection	endSection;
	Point3 startP, endP,midP;
	if (subSample+1 < mSubBounds.Count())
		endSection = GetSubBoundingBoxes(subSample+1).mCrossSection;
	else
	{
		if (!mIsClosed)
		{
			endSection = GetSubBoundingBoxes(mSubBounds.Count()-1).mCrossSection;

			//we area at the end need to extend our end section
			Point3 tempP = p * endSection.mITM;
			midP = tempP;
			tempP.x = 0.0f;
			tempP.y = 0.0f;
			tempP = tempP * endSection.mTM;
			float d = Length(tempP - endSection.mP);
			splineU = 1.0f + d/mSplineLength;
			if (splineU > (1.01f*scale))
				return false;
			done = TRUE;
			pastEnd = TRUE;
		}
		else
		{
			endSection = GetSubBoundingBoxes(0).mCrossSection;
			endSection.mOffset.z = 1.0f;
		}
	}

	float startU = startSection.mOffset.z;
	float endU = endSection.mOffset.z;
	float midU = startU + (endU - startU) * 0.5f;
	SplineCrossSection	midSection = GetSubCrossSection(midU);

	if (!mAdvanceSplines)
		midSection.ComputeTransform(mUpVec);


	
	startP =  p * startSection.mITM;
	endP = p * endSection.mITM;
	midP = p * midSection.mITM;

	BOOL flip = FALSE;
	if (startP.z <= 0.0f)
		flip = TRUE;

	if ((startP.z < 0.0f) && (endP.z < 0.0f))
	{
		if (subSample == 0)
		{
			if (!mIsClosed)
			{


				Point3 tempP = p * startSection.mITM;
				midP = tempP;
				tempP.x = 0.0f;
				tempP.y = 0.0f;
				tempP = tempP * startSection.mTM;
				float d = Length(tempP - startSection.mP);
				splineU = -d/mSplineLength;
				if (splineU < (-0.01f)*scale)
					return false;
				done = TRUE;
				pastEnd = TRUE;
			}
			else
			{
				return false;
			}
		}
		else
			return false;
	}
	if ((startP.z > 0.0f) && (endP.z > 0.0f) && !done)
		return false;

	while (!done && iter < maxIterations)
	{
		//move down or move up
		if (fabs(startU-endU) < 0.0001f)
			done = TRUE;
		else
		{
			if (!flip)
			{
				if (midP.z < 0.0f)
				{
					endSection = midSection;
					float startU = startSection.mOffset.z;
					float endU = endSection.mOffset.z;
					float midU = startU + (endU - startU) * 0.5f;
					midSection = GetSubCrossSection(midU);
				}
				else
				{
					startSection = midSection;
					float startU = startSection.mOffset.z;
					float endU = endSection.mOffset.z;
					float midU = startU + (endU - startU) * 0.5f;
					midSection = GetSubCrossSection(midU);
				}
				if (!mAdvanceSplines)
					midSection.ComputeTransform(mUpVec);
				midP = p * midSection.mITM;
			}
			else
			{
				if (midP.z < 0.0f)
				{
					startSection = midSection;
					float startU = startSection.mOffset.z;
					float endU = endSection.mOffset.z;
					float midU = startU + (endU - startU) * 0.5f;
					midSection = GetSubCrossSection(midU);
				}
				else
				{
					endSection = midSection;
					float startU = startSection.mOffset.z;
					float endU = endSection.mOffset.z;
					float midU = startU + (endU - startU) * 0.5f;
					midSection = GetSubCrossSection(midU);
				}
				if (!mAdvanceSplines)
					midSection.ComputeTransform(mUpVec);
				midP = p * midSection.mITM;
			}
		}
		iter++;
		splineU = midSection.mOffset.z;
	}

	Point3 sectionP = midSection.mTM.GetRow(3);
	Point3 tp = midP;
	tp.z = 0.0f;
	float per = Length(tp);
	
	{
		d = per;

		if (projectionType == kCircular)
		{				
			uvw.z = 1.0f;					

			Point3 v = Point3(tp.x, tp.y, 0.0f);
			float a1 = (float)atan2(v.x,-v.y);
			uvw.x = (a1+PI)/TWOPI;
			uvw.y = splineU;

			hit = true;
		}
		else if (projectionType == kPlanar)
		{
			if (this->mPlanarMapUp)
				uvw.x = tp.y;
			else
				uvw.x = tp.x;
			uvw.x *= 0.5f;
			uvw.y = splineU;
			uvw.z = 1.0f;	
			hit = true;
		}

	}

	return true;

}
void SplineElementData::SetLineCache(PolyLine *line)
{
	mLine = line;

	mUpVec = Point3(0.0f,0.0f,1.0f);

	int numPts = line->numPts;
	Point3 testVecA = Normalize(line->pts[numPts-1].p-line->pts[0].p);
	Point3 testVecB = Normalize(line->pts[numPts/2].p-line->pts[0].p);

	float length = Length(testVecA-testVecB);

	if (length > 0.01f)
	{
		Point3 testVec = CrossProd(testVecA,testVecB);
		testVec = Normalize(testVec);
		float z = fabs(testVec.z);
		if (z < 0.9f)
			mUpVec = testVec;
	}
	else
	{
		mUpVec.x = testVecA.z;
		mUpVec.y = testVecA.y;
		mUpVec.z = testVecA.x;
	}
}

void SplineElementData::DrawBox(GraphicsWindow *gw,Box3 b)
{
	Point3 p[3];
	p[0] = b[0];
	p[1] = b[1];
	gw->segment(p,1);

	p[0] = b[1];
	p[1] = b[3];
	gw->segment(p,1);

	p[0] = b[3];
	p[1] = b[2];
	gw->segment(p,1);

	p[0] = b[2];
	p[1] = b[0];
	gw->segment(p,1);


	p[0] = b[4];
	p[1] = b[5];
	gw->segment(p,1);

	p[0] = b[5];
	p[1] = b[7];
	gw->segment(p,1);

	p[0] = b[7];
	p[1] = b[6];
	gw->segment(p,1);

	p[0] = b[6];
	p[1] = b[4];
	gw->segment(p,1);


	p[0] = b[0];
	p[1] = b[4];
	gw->segment(p,1);

	p[0] = b[1];
	p[1] = b[5];
	gw->segment(p,1);

	p[0] = b[3];
	p[1] = b[7];
	gw->segment(p,1);

	p[0] = b[2];
	p[1] = b[6];
	gw->segment(p,1);

}


void SplineCrossSection::Display(GraphicsWindow *gw)
{

	Point3 plist[32+1];
	Point3 mka,mkb,mkc,mkd;

	Point3 align(0.0f,0.0f,1.0f);

	int ct = 0;
	float angle = TWOPI/float(32) ;
	Matrix3 rtm = RotAngleAxisMatrix(align, angle);
	Point3 p(1.0f,0.0f,0.0f);

	for (int i=0; i<32; i++) 
	{
		p = p * rtm;
		plist[ct++] = p;
	}

	p = p * rtm;
	plist[ct++] = p;


	for (int i = 0; i < 32; i++)
		gw->segment(&plist[i],1);

}

void SplineElementData::DisplayCrossSections(GraphicsWindow *gw, int crossSectionIndex,SplineMapProjectionTypes projType )
{
	Point3 l[3];

	Color selColor = GetUIColor(COLOR_SEL_GIZMOS);	

	COLORREF seamColorRef = ColorMan()->GetColor(OPENEDGECOLORID);
	Color seamColor(seamColorRef);
	Point3 lastPoint(0.0f,0.0f,0.0f);

	if (projType == kCircular)
	{
		l[0] = Point3 (0.0f,0.0f,0.0f);
		l[1] = Point3 (0.0f,1.0f,0.0f);
		gw->setColor(LINE_COLOR,seamColor);	
		gw->startSegments();
		gw->segment(l,1);

		l[0] = Point3 (1.0f,0.0f,0.0f);
		l[1] = Point3 (0.0f,0.0f,0.0f);
		gw->setColor(LINE_COLOR,Point3(1.0f,0.0f,0.0f));	
		gw->segment(l,1);


		if (GetCrossSection(crossSectionIndex).mIsSelected)
			gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
		else
			gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));

		GetCrossSection(crossSectionIndex).Display(gw);

		gw->endSegments();
	}
	else
	{
		l[0] = Point3 (0.0f,0.0f,0.0f);
		l[1] = Point3 (0.0f,0.5f,0.0f);
		gw->setColor(LINE_COLOR,seamColor);	
		gw->startSegments();
		gw->segment(l,1);

		if (GetCrossSection(crossSectionIndex).mIsSelected)
			gw->setColor(LINE_COLOR,selColor);
		else
			gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));			

		if (this->mPlanarMapUp)
		{
			l[1] = Point3 (0.0f,1.0f,0.0f);
			gw->segment(l,1);
			l[1] = Point3 (0.0f,-1.0f,0.0f);
			gw->segment(l,1);
		}
		else
		{
			l[1] = Point3 (1.0f,0.0f,0.0f);
			gw->segment(l,1);
			l[1] = Point3 (-1.0f,0.0f,0.0f);
			gw->segment(l,1);
		}


		gw->endSegments();
	}
}

void SplineElementData::DisplaySpline(GraphicsWindow *gw, Matrix3 splineTM,SplineMapProjectionTypes projType )
{

	gw->setTransform(Matrix3(1));

	Color selColor = GetUIColor(COLOR_SEL_GIZMOS);	

	COLORREF seamColorRef = ColorMan()->GetColor(OPENEDGECOLORID);
	Color seamColor(seamColorRef);
	Point3 lastPoint(0.0f,0.0f,0.0f);

	

	DWORD rndLimits = gw->getRndLimits();


	gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));

	//draw our center spline


	gw->setRndLimits(rndLimits & ~GW_Z_BUFFER);
	gw->setColor(LINE_COLOR,Point3(1.0f,0.0f,1.0f));	

	BOOL debugDisplay = FALSE;
	if (mAdvanceSplines && debugDisplay)
	{
		gw->startSegments();


		for (int j = 0; j < mBorderLines.Count(); j++)
		{
			int numPts = mBorderLines[j]->numPts;
			for (int k = 1; k < numPts; k++)
			{
				Point3 l[3];
				l[0] = mBorderLines[j]->pts[k-1].p;
				l[1] = mBorderLines[j]->pts[k].p;
				gw->segment(l,1);
			}
		}
		
		
		for (int j = 0; j < mBorderLines.Count()/4; j++)
		{
			int numPts1 = mBorderLines[j*4+0]->numPts;
			BOOL draw = TRUE;

			for (int k = 1; k < 4; k++)
			{
				int numPts2 = mBorderLines[j*4+k]->numPts;
				if (numPts2 != numPts1)
					draw = FALSE;
			}
			

			if (draw)
			{
				for (int k = 0; k < numPts1; k++)
				{
					if (k == 0)
						gw->setColor(LINE_COLOR,Point3(0.0f,1.0f,1.0f));
					else if (mBorderLines[j*4]->pts[k].GetMatID() == 0)
						gw->setColor(LINE_COLOR,Point3(1.0f,0.0f,1.0f));
					else
						gw->setColor(LINE_COLOR,Point3(0.0f,0.0f,1.0f));
					
					Point3 p[3];

					p[0] = mBorderLines[j*4]->pts[k].p;
					p[1] = mBorderLines[j*4+2]->pts[k].p;
					gw->segment(p,1);

					p[0] = mBorderLines[j*4+1]->pts[k].p;
					p[1] = mBorderLines[j*4+3]->pts[k].p;
					gw->segment(p,1);

	//				gw->marker(&mBorderLines[j]->pts[k].p,DOT_MRKR);
				}
			}
		}

		gw->endSegments();

		gw->startMarkers();
		for (int j = 0; j < mBorderLines.Count(); j++)
		{
			int numPts = mBorderLines[j]->numPts;
				for (int k = 0; k < numPts; k++)
				{
//					if (k == 0)
//						gw->setColor(LINE_COLOR,Point3(0.0f,1.0f,0.0f));
//					else 
					if (mBorderLines[j]->pts[k].GetMatID() == 0)
						gw->setColor(LINE_COLOR,Point3(0.0f,0.0f,1.0f));
					else
						gw->setColor(LINE_COLOR,Point3(1.0f,0.0f,0.0f));
					

					gw->marker(&mBorderLines[j]->pts[k].p,DOT_MRKR);
				}
		}

		gw->endMarkers();

		
/*
		int segments = mBorderSplines[0].KnotCount()*15;
		float inc = 1.0f/(segments-1);
		

		for (int j = 0; j < 4; j++)
		{
			if (projType == kPlanar)
			{
				if ((j == 1) || (j==3))
					continue;
			}

			float u = 0.0f;
			for (int i = 0; i < segments; i++)
			{
				Point3 p = mBorderSplines[j].InterpCurve3D(u,SPLINE_INTERP_NORMALIZED);
				if (i != 0)
				{
					Point3 l[3];
					l[0] = p;
					l[1] = lastPoint;
					gw->segment(l,1);
				}

				lastPoint = p;
				u += inc;
			}
		}
*/
		
	}

	else
	{
		gw->startSegments();
		for (int i = 0; i < mSubBounds.Count(); i++)
		{
			if (i == 0)
			{
				lastPoint = Point3(0.0f,0.0f,0.0f) * mSubBounds[i].mCrossSection.mTM * splineTM;
			}
			else
			{		
				Point3 currentPoint =  Point3(0.0f,0.0f,0.0f) * mSubBounds[i].mCrossSection.mTM  * splineTM;

				Point3 l[3];
				l[0] = currentPoint;
				l[1] = lastPoint;
				gw->segment(l,1);
				lastPoint = currentPoint;
			}
		}
		gw->endSegments();
		gw->setRndLimits(rndLimits);

		if (mIsSelected)
		{
			gw->setTransform(Matrix3(1));


			gw->setRndLimits(rndLimits );

			Point3 zero(0.0f,0.0f,0.0f);
			Point3 x(1.0f,0.0f,0.0f);
			Point3 nx(-1.0f,0.0f,0.0f);
			Point3 y(0.0f,1.0f,0.0f);

			Point3 l[3];
			l[0] = x;
			l[1] = nx;

			gw->setTransform(Matrix3(1));
			gw->startSegments();
			for (int j = 0; j < 4; j++)
			{
				if (projType == kPlanar)
				{
					if (this->mPlanarMapUp)
					{
						if ((j == 0) || (j==2))
							continue;
					}
					else
					{
						if ((j == 1) || (j==3))
							continue;
					}
				}
				Point3 axis(0.0f,0.0f,0.0f);
				if (j < 2)
					axis[j] = 1.0f;
				else
					axis[j%2] = -1.0f;

				if (j == 1)
				{
					//get the map seam color
					gw->setColor(LINE_COLOR,seamColor);	
				}
				else
				{
					gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));				
				}


				for (int i = 0; i < mSubBounds.Count(); i++)
				{
					if (i == 0)
					{
						lastPoint = axis * mSubBounds[i].mCrossSection.mTM * splineTM;

					}
					else
					{		
						Point3 currentPoint =  axis * mSubBounds[i].mCrossSection.mTM  * splineTM;

						Point3 l[3];
						l[0] = currentPoint;
						l[1] = lastPoint;
						gw->segment(l,1);
						lastPoint = currentPoint;
					}
				}


				Point3 currentPoint =  axis * mCrossSectionData[mCrossSectionData.Count()-1].mTM  * splineTM;
				Point3 l[3];
				l[0] = currentPoint;
				l[1] = lastPoint;
				gw->segment(l,1);
				lastPoint = currentPoint;
			}
			gw->endSegments();
		}
	}

}


//selected spline we draw the cross section, point, spline, and envelope
//not selected just draw the spline
void SplineElementData::Display(GraphicsWindow *gw, Matrix3 splineTM,SplineMapProjectionTypes projType )
{

	
	Matrix3 viewTM = gw->getTransform();

	

	if (mLine == NULL)
		return;

	DisplaySpline(gw,splineTM,projType);



	//draw the cross sections
	if (mIsSelected)
	{
		
		int crossSectionCount = mCrossSectionData.Count();
		for (int j = 0; j < crossSectionCount; j++)
		{

			Matrix3 crossSectionTM = mCrossSectionData[j].mTM;
			Matrix3 tm = crossSectionTM * splineTM;
			gw->setTransform(tm);
			DisplayCrossSections(gw, j,projType );

		}
	}

	




}


void	SplineElementData::DeleteCrossSection(int crossSectionIndex)
{
	if ((crossSectionIndex > 0) && (crossSectionIndex < mCrossSectionData.Count()))
	{
		mCrossSectionData.Delete(crossSectionIndex,1);
	}
}


void	SplineElementData::InsertCrossSection(float u)
{
	for (int i = 0; i < mCrossSectionData.Count(); i++)
	{
			float currentU = mCrossSectionData[i].mOffset.z;
			if (u < currentU)
			{
				int insertIndex = i - 1;
				if (i > 0)
				{	
					SplineCrossSection section = GetCrossSection(u);

					mCrossSectionData.Insert(i,1,&section);					
					return;
				}			
			}
	}
}

void	SplineElementData::ComputePlanarUp(BOOL planarMapUp)
{
	if ((mUpVec.z > 0.5) || (mUpVec.z < -0.5))
		mPlanarMapUp =  planarMapUp;
	else
	{
		if (planarMapUp)
			mPlanarMapUp =  FALSE;
		else
			mPlanarMapUp =  TRUE;
	}
}

static int CrossSectionUSort( const void *elem1, const void *elem2 ) {
	SplineCrossSection *ta = (SplineCrossSection *)elem1;
	SplineCrossSection *tb = (SplineCrossSection *)elem2;


	if (ta->mOffset.z == tb->mOffset.z) return 0;
	else if (ta->mOffset.z < tb->mOffset.z) return -1;
	else return 1;

}

void SplineElementData::SortCrossSections()
{
	mCrossSectionData.Sort(CrossSectionUSort);
}
