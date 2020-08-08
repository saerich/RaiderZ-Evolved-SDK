/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kyruntime_obstaclemanagement */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_SHAPEPOLYLINE_H
#define KY_SHAPEPOLYLINE_H

#include "kypathdata/generic/types.h"
#include "kypathdata/generic/memory.h"


namespace Kaim
{

class Vec3f;


/*! This class represents a 2D5-polyline extruded along the vertical Z axis. You can pass an instance of this
	class to Obstacle::Update() in order to represent the physical dimensions of your Obstacle. 
	\pp The polyline is an array of Vec3f positions with the following requirements:
	-	The first Vec3f and the last Vec3f in the array must be identical.
	-	The points in the polyline are in clockwise order, as seen from above.
	\pp There are two ways to set the points in the polyline:
	-	You can provide the full array of Vec3f points at one time through a call to SetPolyline().
	-	You can set the maximum number of points through a call to SetupPointArray(), and then feed points one-by-one
		through calls to AddPoint().
	\ingroup kyruntime_obstaclemanagement */
class ShapePolyline
{
	KY_DEFINE_NEW_DELETE_OPERATORS


public:
	/*! \label_constructor */
	ShapePolyline(): m_points(KY_NULL), m_maxPointCount(0), m_pointCount(0), m_thickness(0.0f) {}
	ShapePolyline(KyUInt32 maxPointCount): m_points(KY_NULL), m_maxPointCount(0), m_pointCount(0), m_thickness(0.0f) { SetupPointArray(maxPointCount); }
	ShapePolyline(const Vec3f* points, KyUInt32 pointCount, KyFloat32 thickness);
	~ShapePolyline() { DeletePointArray(); }

	/*! Sets the maximum number of points that can be contained in the polyline. */
	KyResult SetupPointArray(KyUInt32 maxPointCount);
	
	/*! Adds a point to the end of the polyline. Always make sure that you have called SetupPointArray() to 
		set up the maximum size of the polyline before you call this method. */
	KyResult AddPoint(const Vec3f& point);

	/*! Sets the polyline of the ShapePolyline to copy the specified array of points. If the first and last points in
		the array are not identical, this method also copies the first point to the end of the array.
		\param points			The array of Vec3f objects that make up the polyline.
		\param pointCount		The number of points in the \c points array.
		\param thickness		The vertical thickness of the volume. */
	KyResult SetPolyline(const Vec3f* points, KyUInt32 pointCount, KyFloat32 thickness);

	/*! \label_clear */
	void Clear() { m_pointCount = 0; }

	/*! Deletes all points in the polyline. */
	void DeletePointArray();

private:
	ShapePolyline(const ShapePolyline&);
	ShapePolyline& operator=(const ShapePolyline&);


public:
	Vec3f* m_points; /*!< The array of points in the outline. Do not modify directly. */
	KyUInt32 m_maxPointCount; /*!< The maximum number of points in the outline. Do not modify directly. */
	KyUInt32 m_pointCount; /*!< The actual number of points in the outline. Do not modify. */
	KyFloat32 m_thickness; /*!< The vertical thickness of the volume. */
};

} // Kaim

#endif // KY_SHAPEPOLYLINE_H
