/********************************************************************** *<
FILE: ToolSplineMapping.cpp

DESCRIPTION: This contains all our spline undo methods


HISTORY: 12/31/96
CREATED BY: Peter Watje


*>	Copyright (c) 2007, All Rights Reserved.
**********************************************************************/


#include "unwrap.h"


SplineMapHoldObject::SplineMapHoldObject(SplineData *splineData,BOOL recomputeCrossSections) 
{
	mRecomputeCrossSections = recomputeCrossSections;
	m = splineData;
	splineData->CopyCrossSectionData(udata);

}

SplineMapHoldObject::~SplineMapHoldObject() 
{
	for (int i = 0; i < udata.Count(); i++)
	{
		if (udata[i])
		{
			delete udata[i];
		}
	}
	for (int i = 0; i < rdata.Count(); i++)
	{
		if (rdata[i])
		{
			delete rdata[i];
		}
	}

}


void SplineMapHoldObject::Restore(int isUndo) 
{
	if (isUndo) 
	{
		m->CopyCrossSectionData(rdata);
	}

	m->PasteCrossSectionData(udata);

	if (mRecomputeCrossSections)
		m->RecomputeCrossSections();

}

void SplineMapHoldObject::Redo() 
{
	m->PasteCrossSectionData(rdata);
	if (mRecomputeCrossSections)
		m->RecomputeCrossSections();

}
void SplineMapHoldObject::EndHold() 
{
}
TSTR SplineMapHoldObject::Description() 
{
	return TSTR(_T(GetString(IDS_PW_UVW_VERT_EDIT)));
}		


