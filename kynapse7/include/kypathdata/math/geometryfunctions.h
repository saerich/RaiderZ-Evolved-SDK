/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*!	\file
	\ingroup kypathdata_math */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_GEOMETRYFUNCTIONS_H
#define KY_GEOMETRYFUNCTIONS_H

#include <kypathdata/generic/basetypes.h>

namespace Kaim
{
class Vec2f;
class Vec3f;

class Circle;
class Segment;
class Capsule;
class HorizontalRectangle;

class AxisAlignedBox;
class ExtrudedRectangle;
class ExtrudedPolyline;
class VerticalCylinder;


////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////////////////////////////////////////////////


/*!	Retrieves the minimum distance between \c point and any point on the line segment that extends between
	\c segStart and \c segEnd.
	\ingroup kypathdata_math */
KyFloat32 GetDistance_PointToSegment(const Vec2f& segStart, const Vec2f& segEnd, const Vec2f& point);

/*!	Retrieves the minimum distance between \c point and any point on the line segment that extends between
	\c segStart and \c segEnd, ignoring the Z axis.
	\ingroup kypathdata_math */
KyFloat32 GetDistance2d_PointToSegment(const Vec3f& segStart, const Vec3f& segEnd, const Vec3f& point);

KyFloat32 GetVerticalProjectionAltitudeOnSemiHorizontalPlane(
	const Vec3f& planeOrigin, const Vec3f& planeFirstAxis, const Vec3f& point);


////////////////////////////////////////////////////////////////////////////////////////////////////////
// IsInside functions
////////////////////////////////////////////////////////////////////////////////////////////////////////

/*!	Tests whether or not a specified point is inside the area defined by a polyline in two 
	dimensions, ignoring their altitude.
	\param p					The point to test.
	\param points				The list of points in the polyline. This list must be in clockwise order,
								as seen from above. The list must also be closed: its last point must be the
								same as its first point.
	\param count				The number of points in the \c points array.
	\return #KY_TRUE if the point is inside the polyline or on the borders of the polyline, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool IsInside2d_Polyline(const Vec3f& p, const Vec3f* points, KyUInt32 count);


////////////////////////////////////////////////////////////////////////////////////////////////////////
// 2D interseciton functions
////////////////////////////////////////////////////////////////////////////////////////////////////////

/*!	Tests whether or not two specified line segments intersect in two dimensions, ignoring their altitude, 
	and retrieves the first collision point along each line segment.
	\param seg1					The first line segment.
	\param seg2					The second line segment.
	\param[out] collPoint_seg1	If an intersection is detected between the line segments, this parameter
								is updated with the position of the intersection along the second line
								segment. The altitude of this collision point is interpolated between the
								altitudes of the points in the second line segment.
	\param[out] collPoint_seg2	If an intersection is detected between the line segments, this parameter
								is updated with the position of the intersection along the first line
								segment. The altitude of this collision point is interpolated between the 
								altitudes of the points in the firs tline segment. This position will always be the same as 
								\c collPointseg1 along the Right and Front axes, but its altitude may be 
								different. If the altitude of \c collPointseg1 matches the altitude of 
								\c collPointseg2, the segments likely collide in 3D as well.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect2d_Segments(const Segment& seg1, const Segment& seg2, Vec3f* collPoint_seg1=KY_NULL, Vec3f* collPoint_seg2=KY_NULL);

/*!	Tests whether or not a specified line segment intersects the area of a specified
	circle in two dimensions, ignoring their altitude, and retrieves the first
	and last collision points along the line segment.
	\param segment				The line segment to test.
	\param circle				The circle to test.
	\param[out] entryPoint		If an intersection is detected between the line segment and the circle, 
								this parameter is updated with the first position along the line segment
								that is inside the area of the circle.
	\param[out] exitPoint		If an intersection is detected between the line segment and the circle, 
								this parameter is updated with the last position along the line segment
								that is inside the area of the circle.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect2d_Segment_Circle(const Segment& segment, const Circle& circle, Vec3f* entryPoint=KY_NULL, Vec3f* exitPoint=KY_NULL);

/*!	Tests whether or not a specified line segment intersects the area of a specified
	capsule in two dimensions, ignoring their altitude, and retrieves the first
	collision point along the line segment.
	\param capsule				The capsule to test.
	\param segment				The line segment to test.
	\param[out] collPoint		If an intersection is detected between the line segment and the capsule, 
								this parameter is updated with the first position along the capsule axis
								where the moving circle intersects the segment. The altitude of this
								collision point is interpolated accordingly to segment altitude.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect2d_Capsule_Segment(const Capsule& capsule, const Segment& segment, Vec3f* collPoint=KY_NULL);

/*!	Tests whether or not a specified line segment intersects any segment in a specified
	polyline in two dimensions, ignoring their altitude, and retrieves the first
	collision point along the line segment.
	\param segment				The line segment to test.
	\param points				The list of points in the polyline.
	\param count				The number of points in the \c points array.
	\param[out] collPoint		If an intersection is detected between the line segment and the polyline, 
								this parameter is updated with the position of the intersection. The 
								altitude of this collision point is interpolated accordingly to segment altitude.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect2d_Segment_Polyline(const Segment& segment, const Vec3f* points, KyUInt32 count, Vec3f* collPoint=KY_NULL);

/*!	Tests whether or not a specified polyline intersects the area of a specified
	capsule in two dimensions, ignoring their altitude, and retrieves the first
	collision point along the polyline.
	\param capsule				The capsule to test.
	\param points				The list of points in the polyline.
	\param count				The number of points in the \c points array.
	\param[out] collisionPoint	If an intersection is detected between the polyline and the capsule, 
								this parameter is updated with the first position along the capsule axis
								where the moving circle intersects the segment. The altitude of this
								collision point is interpolated between the altitudes of the two polyline
								points that define the intersecting line segment.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect2d_Capsule_Polyline(const Capsule& capsule, const Vec3f* points, KyUInt32 count, Vec3f* collisionPoint=KY_NULL);

/*!	Tests whether or not a specified polyline intersects the area of a specified
	circle in two dimensions, ignoring their altitude.
	\param circle				The circle to test.
	\param points				The list of points in the polyline.
	\param count				The number of points in the \c points array.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect2d_Circle_Polyline(const Circle& circle, const Vec3f* points, KyUInt32 count);

/*!	Tests whether or not two polylines intersect in two dimensions, ignoring their altitude.
	\param pl1_points			The points in the first polyline.
	\param pl1_count			The number of points in \c pl1_points.
	\param pl2_points			The points in the second polyline.
	\param pl2_count			The number of points in \c pl2_points.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect2d_Polylines(const Vec3f* pl1_points, KyUInt32 pl1_count, const Vec3f* pl2_points, KyUInt32 pl2_count);

/*!	Tests whether or not two rectangles intersect in two dimensions, ignoring their altitude.
	\param rectangle1			The first rectangle.
	\param rectangle2			The second rectangle.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect2d_HorizontalRectangles(const HorizontalRectangle& rectangle1, const HorizontalRectangle& rectangle2);


/*!	Tests whether or not a line segment intersects an axis-aligned bounding box in two dimensions, ignoring their altitudes,
	and retrieves the first and last collision points along the segment.
	\param segment				The line segment to test.
	\param aabox				The axis-aligned bounding box to test.
	\param[out] entryPoint		If an intersection is detected between the line segment and the box, 
								this parameter is updated with the first position along the line segment
								that is inside the area of the box.
	\param[out] exitPoint		If an intersection is detected between the line segment and the box, 
								this parameter is updated with the last position along the line segment
								that is inside the area of the box.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect2d_Segment_AxisAlignedBox(const Segment& segment, const AxisAlignedBox& aabox, Vec3f* entryPoint=KY_NULL, Vec3f* exitPoint=KY_NULL);


//////////////////////////////////////////////////////////////////////////
// 3D intersection functions
//////////////////////////////////////////////////////////////////////////
// - complex shapes vs segment
/*!	Tests whether or not a line segment intersects an axis-aligned bounding box in three dimensions,
	and retrieves the first and last collision points along the segment.
	\param segment				The line segment to test.
	\param aabox				The axis-aligned bounding box to test.
	\param[out] entryPoint		If an intersection is detected between the line segment and the box, 
								this parameter is updated with the first position along the line segment
								that is inside the area of the box.
	\param[out] exitPoint		If an intersection is detected between the line segment and the box, 
								this parameter is updated with the last position along the line segment
								that is inside the area of the box.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect3d_Segment_AxisAlignedBox(const Segment& segment, const AxisAlignedBox& aabox, Vec3f* entryPoint=KY_NULL, Vec3f* exitPoint=KY_NULL);

/*!	Tests whether or not a line segment intersects a cylinder in three dimensions.
	\param segment				The line segment to test.
	\param verticalCylinder		The cylinder to test.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect3d_Segment_VerticalCylinder(const Segment& segment, const VerticalCylinder& verticalCylinder);

/*!	Tests whether or not a line segment intersects an extruded rectangle in three dimensions.
	\param segment				The line segment to test.
	\param extrudedRectangle	The extruded rectangle to test.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect3d_Segment_ExtrudedRectangle(const Segment& segment, const ExtrudedRectangle& extrudedRectangle);

/*!	Tests whether or not a line segment intersects an extruded polyline in three dimensions.
	\param segment				The line segment to test.
	\param extrudedPolyline		The extruded polyline to test.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect3d_Segment_ExtrudedPolyline(const Segment& segment, const ExtrudedPolyline& extrudedPolyline);


// - complex shapes vs triangle
/*!	Tests whether or not a triangle intersects an axis-aligned bounding box in three dimensions.
	\param p1					The first point of the triangle.
	\param p2					The second point of the triangle.
	\param p3					The third point of the triangle.
	\param aabox				The axis-aligned bounding box to test.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect3d_Triangle_AxisAlignedBox(const Vec3f& p1, const Vec3f& p2, const Vec3f& p3, const AxisAlignedBox& aabox);

/*!	Tests whether or not a triangle intersects a cylinder in three dimensions.
	\param p1					The first point of the triangle.
	\param p2					The second point of the triangle.
	\param p3					The third point of the triangle.
	\param verticalCylinder		The cylinder to test.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect3d_Triangle_VerticalCylinder(const Vec3f& p1, const Vec3f& p2, const Vec3f& p3, const VerticalCylinder& verticalCylinder);

/*!	Tests whether or not a triangle intersects an extruded rectangle in three dimensions.
	\param p1					The first point of the triangle.
	\param p2					The second point of the triangle.
	\param p3					The third point of the triangle.
	\param extrudedRectangle	The extruded rectangle to test.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect3d_Triangle_ExtrudedRectangle(const Vec3f& p1, const Vec3f& p2, const Vec3f& p3, const ExtrudedRectangle& extrudedRectangle);

/*!	Tests whether or not a triangle intersects an extruded polyline in three dimensions.
	\param p1					The first point of the triangle.
	\param p2					The second point of the triangle.
	\param p3					The third point of the triangle.
	\param extrudedPolyline		The extruded polyline to test.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect3d_Triangle_ExtrudedPolyline(const Vec3f& p1, const Vec3f& p2, const Vec3f& p3, const ExtrudedPolyline& extrudedPolyline);


// - complex shapes vs complex shapes
/*!	Tests whether or not two axis-aligned bounding boxes intersect in three dimensions.
	\param aabox1				The first axis-aligned bounding box.
	\param aabox2				The second axis-aligned bounding box.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect3d_AxisAlignedBoxes(const AxisAlignedBox& aabox1, const AxisAlignedBox& aabox2);

/*!	Tests whether or not a cylinder intersects an axis-aligned bounding box in three dimensions.
	\param verticalCylinder		The cylinder to test.
	\param aabox				The axis-aligned bounding box to test.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect3d_VerticalCylinder_AxisAlignedBox(const VerticalCylinder& verticalCylinder, const AxisAlignedBox& aabox);

/*!	Tests whether or not two cylinders intersect in three dimensions.
	\param verticalCylinder1	The first cylinder to test.
	\param verticalCylinder2	The second cylinder to test.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect3d_VerticalCylinders(const VerticalCylinder& verticalCylinder1, const VerticalCylinder& verticalCylinder2);

/*!	Tests whether or not an extruded rectangle intersects an axis-aligned bounding box in three dimensions.
	\param extrudedRectangle	The extruded rectangle to test.
	\param aabox				The axis-aligned bounding box to test.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect3d_ExtrudedRectangle_AxisAlignedBox(const ExtrudedRectangle& extrudedRectangle, const AxisAlignedBox& aabox);

/*!	Tests whether or not an extruded rectangle intersects a cylinder in three dimensions.
	\param extrudedRectangle	The extruded rectangle to test.
	\param verticalCylinder		The cylinder to test.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect3d_ExtrudedRectangle_VerticalCylinder(const ExtrudedRectangle& extrudedRectangle, const VerticalCylinder& verticalCylinder);

/*!	Tests whether or not two extruded rectangles intersect in three dimensions.
	\param er1					The first extruded rectangle to test.
	\param er2					The second extruded rectangle to test.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect3d_ExtrudedRectangles(const ExtrudedRectangle& er1, const ExtrudedRectangle& er2);

/*!	Tests whether or not an extruded polyline intersects an axis-aligned bounding box in three dimensions.
	\param extrudedPolyline		The extruded polyline to test.
	\param aabox				The axis-aligned bounding box to test.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect3d_ExtrudedPolyline_AxisAlignedBox(const ExtrudedPolyline& extrudedPolyline, const AxisAlignedBox& aabox);

/*!	Tests whether or not an extruded polyline intersects a cylinder in three dimensions.
	\param extrudedPolyline		The extruded polyline to test.
	\param verticalCylinder		The cylinder to test.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect3d_ExtrudedPolyline_VerticalCylinder(const ExtrudedPolyline& extrudedPolyline, const VerticalCylinder& verticalCylinder);

/*!	Tests whether or not an extruded polyline intersects an extruded rectangle in three dimensions.
	\param extrudedPolyline		The extruded polyline to test.
	\param extrudedRectangle	The extruded rectangle to test.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect3d_ExtrudedPolyline_ExtrudedRectangle(const ExtrudedPolyline& extrudedPolyline, const ExtrudedRectangle& extrudedRectangle);

/*!	Tests whether or not two extruded polylines intersect in three dimensions.
	\param ep1					The first extruded polyline to test.
	\param ep2					The second extruded polyline to test.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kypathdata_math */
KyBool Intersect3d_ExtrudedPolylines(const ExtrudedPolyline& ep1, const ExtrudedPolyline& ep2);

} //namespace Kaim

#endif // KY_GEOMETRYFUNCTIONS_H
