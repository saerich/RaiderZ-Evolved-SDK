/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

// Primary contact: LASI - secondary contact: NONE
#ifndef KyPdg_DynamicNavFloorHeightField_H
#define KyPdg_DynamicNavFloorHeightField_H


#include <kypathdata/math/vec3f.h>
#include <kypdg/common/stlvector.h>
#include "kypdg/generator/squarepartitiontypes.h"

namespace Kaim
{

class PdgSystem;
class DynamicRasterCell;
class DynamicTriangulation;


enum NavFloorHeightFieldErrorUpdateStatus
{
	NavFloorHeightFieldErrorStatus_Updated,
	NavFloorHeightFieldErrorStatus_NoProjections
};

enum NavFloorHeightFieldPointStatus
{
	NavFloorHeightFieldPointStatus_Valid,
	NavFloorHeightFieldPointStatus_Disabled
};

struct NavFloorHeightFieldPoint
{
	NavFloorHeightFieldPoint()
		: m_pos(0.0f,0.0f),
		  m_minLocalAlt(KyFloat32MAXVAL),
		  m_maxLocalAlt(-KyFloat32MAXVAL),
		  m_error(0.0f),
		  m_status(NavFloorHeightFieldPointStatus_Valid) {}

	NavFloorHeightFieldPoint(const Vec2f& position2D, KyFloat32 alt)
		: m_pos(position2D),
		  m_minLocalAlt(alt),
		  m_maxLocalAlt(alt),
		  m_error(0.0f),
		  m_status(NavFloorHeightFieldPointStatus_Valid){}

	bool operator == (const NavFloorHeightFieldPoint& other)
	{
		// for pos & alts are enough
		bool same = other.m_pos == m_pos
				 && other.m_maxLocalAlt == m_maxLocalAlt
				 && other.m_minLocalAlt == m_minLocalAlt;

		return same;
	}

	Vec2f m_pos;
	KyFloat32 m_minLocalAlt;
	KyFloat32 m_maxLocalAlt;

	KyFloat32 m_error;
	NavFloorHeightFieldPointStatus m_status;
};

// --------------------------------------
// ----- DynamicNavFloorHeightField -----
// --------------------------------------
class DynamicNavFloorHeightField
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	DynamicNavFloorHeightField(KyInt32 originalPixelSize, KyInt32 stride, const PixelBox& navPixelBox);

	void AddPoint(const PixelPos& coord, KyFloat32 alt);
	NavFloorHeightFieldErrorUpdateStatus UpdateErrors(const DynamicRasterCell* rasterCell);
	NavFloorHeightFieldPoint* GetWorstErrorPoint();
	NavFloorHeightFieldPoint* GetPointAtLocalHeightFieldPos(const PixelPos& pos);
	NavFloorHeightFieldPoint* GetPointAtAbsoluteHeightFieldPos(const PixelPos& pos);

public:
	KyInt32 m_originalPixelSize; // must be >0
	KyInt32 m_stride; // must be >0
	PixelBox m_navPixelBox; // from the raster (exclusive + 1 pixel overlap)
	PixelBox m_heightFieldPixelBox; // same but with pixelSize = m_stride * m_sys->Partition().m_pixelSize
	PixelBox m_heightFieldBoundingPixelBox; // updated when adding new point
	StlVector<NavFloorHeightFieldPoint> m_points;
	StlVector<KyUInt32> m_grid;

};

}


#endif //KyPdg_DynamicNavFloor_H

