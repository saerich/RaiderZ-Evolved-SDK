/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_math */

// primary contact: LAPA - secondary contact: LASI
#ifndef KY_EXTRUDEDPOLYLINE_H
#define KY_EXTRUDEDPOLYLINE_H

#include <kypathdata/math/blob/extrudedpolylinedata.h>
#include <kypathdata/blob/blobhandler.h>


namespace Kaim
{
/*!	Represents a three-dimensional volume using:
	-	A polyline that contains the two-dimensional contours of the box on the horizontal plane.
	-	A minimum altitude value.
	-	A maximum altitude value.
	\ingroup kypathdata_math */
class ExtrudedPolyline
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	/*!	\label_constructor. If you use this constructor, you must call Setup() to set the dimensions of the extruded
		polyline before it can be used. */
	ExtrudedPolyline(): m_extrudedPolylineData(KY_NULL) {}

	/*!	\label_constructor Copies the state of the specified ExtrudedPolyline to the newly created object. */
	ExtrudedPolyline(const ExtrudedPolyline& other)
	{
		m_extrudedPolylineData = other.m_extrudedPolylineData;
		m_aabbMin = other.m_aabbMin;
		m_aabbMax = other.m_aabbMax;
	}

	/*!	Copies the state of the specified ExtrudedPolyline to this object. */
	ExtrudedPolyline& operator=(const ExtrudedPolyline& other)
	{
		m_extrudedPolylineData = other.m_extrudedPolylineData;
		m_aabbMin = other.m_aabbMin;
		m_aabbMax = other.m_aabbMax;
		return *this;
	}

	/*!	\label_constructor
		\param polylinePoints		An array that contains the coordinates of the points in the polyline. The points
									must be specified in clockwise order, as seen from above. The list 
									must be closed: i.e. the last point in the polyline must be the same 
									as the first.
		\param pointCount			The number of elements in the \c polylinePoints array.
		\param minAltitude			The altitude of the bottom of the volume.
		\param maxAltitude			The altitude of the top of the volume. */
	ExtrudedPolyline(const Vec3f* polylinePoints, KyUInt32 pointCount, KyFloat32 minAltitude, KyFloat32 maxAltitude);

	/*! Indicates whether or not this object has been successfully set up and is ready for use. */
	KyBool IsValid() const { return m_extrudedPolylineData != KY_NULL; }

	/*!	Sets the extents of the volume. 
		\param polylinePoints		An array that contains the coordinates of the points in the polyline. The points
									must be specified in clockwise order, as seen from above. The list 
									must be closed: i.e. the last point in the polyline must be the same 
									as the first.
		\param pointCount			The number of elements in the \c polylinePoints array.
		\param minAltitude			The altitude of the bottom of the volume.
		\param maxAltitude			The altitude of the top of the volume. */
	KyResult Setup(const Vec3f* polylinePoints, KyUInt32 pointCount, KyFloat32 minAltitude, KyFloat32 maxAltitude);

	/*!	Sets the extents of the volume from the specified ExtrudedPolylineData. */
	KyResult Setup(const ExtrudedPolylineData* data);

	/*!	Computes and stores the smallest axis-aligned bounding box that can enclose this volume. */
	KyResult ComputeAABB();
	

	/*!	Retrieves the altitude of the bottom of this volume. */
	KyFloat32 GetMinAlt() const { return m_extrudedPolylineData ? m_extrudedPolylineData->m_altMin : KyFloat32MAXVAL; }

	/*!	Retrieves the altitude of the top of this volume. */
	KyFloat32 GetMaxAlt() const { return m_extrudedPolylineData ? m_extrudedPolylineData->m_altMax : -KyFloat32MAXVAL; }

	/*!	Retrieves the point at the specified index within the polyline of this volume. */
	const Vec3f* GetPoint(KyUInt32 index) const
	{
		return m_extrudedPolylineData && index < m_extrudedPolylineData->m_polylinePoints.m_count ?  
			&(m_extrudedPolylineData->m_polylinePoints.Values()[index]) 
			: KY_NULL; 
	}

	/*!	Retrieves the array of points that make up the polyline of this volume. */
	const Vec3f* GetPointArray() const { return m_extrudedPolylineData ? m_extrudedPolylineData->m_polylinePoints.Values() : KY_NULL; }

	/*!	Retrieves the number of points that make up the polyline of this volume. */
	KyUInt32 GetPointCount() const 
	{
		return m_extrudedPolylineData ? m_extrudedPolylineData->m_polylinePoints.m_count : 0;
	}

	/*!	Retrieves the minima, or the corner with the smallest coordinate values on the X, Y and Z axes, of the
		smallest axis-aligned bounding box that can enclose this volume. */
	const Vec3f& GetAabbMin() const { return m_aabbMin; }

	/*!	Retrieves the maxima, or the corner with the largest coordinate values on the X, Y and Z axes, of the
		smallest axis-aligned bounding box that can enclose this volume. */
	const Vec3f& GetAabbMax() const { return m_aabbMax; }

	/*! Indicates whether or not the specified position is inside the area occupied by the polyline, ignoring altitude. */
	KyBool IsInside2d(const Vec3f& point) const;

	/*! Indicates whether or not the specified position is inside the three-dimensional volume occupied by the extruded polyline. */
	KyBool IsInside3d(const Vec3f& point) const;

	/*! Indicates whether or not the polyline array is in clockwise order, seen from above. */
	KyBool IsClockwise() const;

	/*! Indicates whether or not the polyline array is twisted: i.e. two or more of its edges intersect in two dimensions. */
	KyBool IsTwisted() const;


private:
	BlobHandler<ExtrudedPolylineData> m_dataHandler; //store the actual data as a blob.
	const ExtrudedPolylineData* m_extrudedPolylineData; //pointer to the data held by the handler. Cleanup is done upon destruction of the handler
	Vec3f m_aabbMin;
	Vec3f m_aabbMax;
};

} // namespace Kaim

#endif // KY_EXTRUDEDPOLYLINE_H
