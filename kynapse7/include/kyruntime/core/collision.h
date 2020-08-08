/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_core */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_COLLISION_H
#define KY_COLLISION_H

#include <kyruntime/core/engine.h>
#include <kyruntime/core/pointwrapper.h>
#include <kypathdata/basesystem/profilermacro.h>

#include <kypathdata/bridge/icollisionbridge.h>
#include <kypathdata/math/vec3futils.h>


//////////////////////////////////////////////////////////////////////////////////////
// This file contains the functions used by Kynapse for collision detection.        //
//////////////////////////////////////////////////////////////////////////////////////

namespace Kaim
{

/*!	Tests for collisions along the line segment between two specified points by invoking
	the ICollisionBridge() interface provided by the \SDKName Bridge.
	\param begin			The starting point of the line segment, in the \SDKName coordinate system.
	\param end				The ending point of the line segment, in the \SDKName coordinate system.
	\param world			A \SDKName World associated with this request, or #KY_NULL if none.
	\param body				A Body associated with this request, or #KY_NULL if none.
	\return true if a collision was detected, or false otherwise.
	\ingroup kyruntime_core */
inline bool RayCast(const Vec3f& begin, const Vec3f& end, const World* world, const Body* body)
{
	KY_PROFILE("RayCast");
	ICollisionBridge* collisionBridge = GetBaseSystem().GetBridge().GetCollisionBridge();
	if (collisionBridge == KY_NULL)
		return KY_FALSE;

	Kaim::ICollisionBridge::RayCastQuery query;

	query.start = begin;
	query.end = end;
	query.world = world;
	query.body = body;

	return collisionBridge->HasRayCastHit(query);
}

/*!	Tests for collisions along the line segment between two specified points by invoking
	the ICollisionBridge() interface served by the \SDKName Bridge, and retrieves information
	about the point at which the first collision is detected.
	\param begin				The starting point of the line segment, in the \SDKName coordinate system.
	\param end					The ending point of the line segment, in the \SDKName coordinate system.
	\param world				A \SDKName World associated with this request, or #KY_NULL if none.
	\param body					A Body associated with this request, or #KY_NULL if none.
	\param[out] collisionPoint	The point on the line segment at which the first collision was detected, 
								in the \SDKName coordinate system.
	\param[out] normal			The normal of the first collision detected along the line segment, in the
								\SDKName coordinate system.
	\return true if a collision was detected, or false otherwise.
	\ingroup kyruntime_core */
inline bool RayCastWithHitInfo(const Vec3f& begin, const Vec3f& end,
							   const World* world, const Body* body,
							   Vec3f& collisionPoint, Vec3f& normal)
{
	KY_PROFILE("RayCastWithHitInfo");
	ICollisionBridge* collisionBridge = GetBaseSystem().GetBridge().GetCollisionBridge();
	if(collisionBridge == KY_NULL)
		return KY_FALSE;

	Kaim::ICollisionBridge::RayCastQuery query;
	query.start = begin;
	query.end = end;
	query.world = world;
	query.body = body;

	Kaim::ICollisionBridge::RayCastResult result;

	collisionBridge->RayCast(query, result);

	collisionPoint = result.hitPosition;
	normal = result.hitNormal;

	return result.hasHit;
}

/*!	Tests for collisions by sweeping a sphere along the line segment between two specified points by invoking
	the ICollisionBridge() interface provided by the \SDKName Bridge.
	\param begin			The starting point of the line segment, in the \SDKName coordinate system.
	\param end				The ending point of the line segment, in the \SDKName coordinate system.
	\param radius			The radius of the sphere.
	\param world			A \SDKName World associated with this request, or #KY_NULL if none.
	\param body				A Body associated with this request, or #KY_NULL if none.
	\return true if a collision was detected, or false otherwise.
	\ingroup kyruntime_core */
inline bool SphereCast(const Vec3f& begin, const Vec3f& end, KyFloat32 radius, const World* world, const Body* body)
{
	KY_PROFILE("SphereCast");
	ICollisionBridge* collisionBridge = GetBaseSystem().GetBridge().GetCollisionBridge();
	if (collisionBridge == KY_NULL)
		return KY_FALSE;

	Kaim::ICollisionBridge::SphereCastQuery query;

	query.start = begin;
	query.end = end;
	query.world = world;
	query.body = body;
	query.radius = radius;

	return collisionBridge->HasSphereCastHit(query);
}

/*!	Tests for collisions along the line segment between two specified points by invoking
	the ICollisionBridge() interface served by the \SDKName Bridge, and retrieves information
	about the point at which the first collision is detected.
	\param begin				The starting point of the line segment, in the \SDKName coordinate system.
	\param end					The ending point of the line segment, in the \SDKName coordinate system.
	\param radius				The radius of the sphere.
	\param world				A \SDKName World associated with this request, or #KY_NULL if none.
	\param body					A Body associated with this request, or #KY_NULL if none.
	\param[out] collisionPoint	The point on the line segment at which the first collision was detected, 
								in the \SDKName coordinate system.
	\param[out] normal			The normal of the first collision detected along the line segment, in the
								\SDKName coordinate system.
	\return #KY_TRUE if a collision was detected, or #KY_FALSE otherwise.
	\ingroup kyruntime_core */
inline bool SphereCastWithHitInfo(const Vec3f& begin, const Vec3f& end, KyFloat32 radius, 
								  const World* world, const Body* body,
								  Vec3f& collisionPoint, Vec3f& normal)
{
	KY_PROFILE("SphereCastWithHitInfo");
	ICollisionBridge* collisionBridge = GetBaseSystem().GetBridge().GetCollisionBridge();
	if (collisionBridge == KY_NULL)
		return KY_FALSE;

	Kaim::ICollisionBridge::SphereCastQuery query;
	query.start = begin;
	query.end = end;
	query.world = world;
	query.body = body;
	query.radius = radius;

	Kaim::ICollisionBridge::SphereCastResult result;

	GetBaseSystem().GetBridge().GetCollisionBridge()->SphereCast(query, result);

	collisionPoint = result.hitPosition;
	normal = result.hitNormal;

	return result.hasHit;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
// intersection function call counter accessors
////////////////////////////////////////////////////////////////////////////////////////////////////////

/*!	Retrieves the total number of calls to Intersect2D_Segment_Segment() since the Engine
	was opened. This function can help to determine an accurate value for the \c Intersect2D_Segment_Segment
	estimated task when using deterministic time measurement mode. 
	See also :USERGUIDE:"Using Deterministic Time Measurement".
	\ingroup kyruntime_core */
KyUInt32 GetCallCounter_Intersect2D_Segment_Segment();

/*!	Retrieves the total number of calls to Intersect2D_Capsule_Segment() since the Engine
	was opened. This function can help to determine an accurate value for the \c Intersect2D_Capsule_Segment
	estimated task when using deterministic time measurement mode. 
	See also :USERGUIDE:"Using Deterministic Time Measurement".
	\ingroup kyruntime_core */
KyUInt32 GetCallCounter_Intersect2D_Capsule_Segment();

/*!	Retrieves the total number of calls to Intersect2D_Rectangle_Rectangle() since the Engine
	was opened. This function can help to determine an accurate value for the \c Intersect2D_Rectangle_Rectangle
	estimated task when using deterministic time measurement mode. 
	See also :USERGUIDE:"Using Deterministic Time Measurement".
	\ingroup kyruntime_core */
KyUInt32 GetCallCounter_Intersect2D_Rectangle_Rectangle();


////////////////////////////////////////////////////////////////////////////////////////////////////////
// 2D collision functions
////////////////////////////////////////////////////////////////////////////////////////////////////////

/*!	Tests whether or not two specified line segments intersect in two dimensions, ignoring their altitude, 
	and retrieves the first collision point along each line segment.
	\param p1					The starting point of the first line segment.
	\param p2					The ending point of the first line segment.
	\param p3					The starting point of the second line segment.
	\param p4					The ending point of the second line segment.
	\param[out] collPoint34		If an intersection is detected between the line segments, this parameter
								is updated with the position of the intersection along the second line
								segment. The altitude of this collision point is interpolated between the
								altitudes of \c p3 and \c p4.
	\param[out] collPoint12		If an intersection is detected between the line segments, this parameter
								is updated with the position of the intersection along the first line
								segment. The altitude of this collision point is interpolated between the 
								altitudes of \c p1 and \c p2. This position will always be the same as 
								\c collPoint34 along the x and y axes, but its altitude may be 
								different. If the altitude of \c collPoint12 matches the altitude of 
								\c collPoint34, the segments likely collide in 3D as well.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kyruntime_core */
KyBool Intersect2D_Segment_Segment(const Vec3f& p1, const Vec3f& p2, const Vec3f& p3, const Vec3f& p4, Vec3f* collPoint34, Vec3f* collPoint12=KY_NULL);

/*!	Tests whether or not a specified line segment intersects the area of a specified
	capsule in two dimensions, ignoring their altitude, and retrieves the first
	collision point along the line segment.
	\param p1					The starting point of the capsule.
	\param p2					The ending point of the capsule.
	\param p3					The starting point of the line segment.
	\param p4					The ending point of the line segment.
	\param halfWidth			The radius of the capsule.
	\param[out] collPoint		If an intersection is detected between the line segment and the capsule, 
								this parameter is updated with the first position along the capsule axis
								where the moving circle intersects the segment. The altitude of this
								collision point is interpolated between the altitudes of \c p3 and \c p4.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kyruntime_core */
KyBool Intersect2D_Capsule_Segment(const Vec3f& p1, const Vec3f& p2, const Vec3f& p3, const Vec3f& p4, KyFloat32 halfWidth, Vec3f* collPoint);

/*!	Tests whether or not a specified line segment intersects any segment in a specified
	polyline in two dimensions, ignoring their altitude, and retrieves the first
	collision point along the line segment.
	\param p1					The starting point of the line segment.
	\param p2					The ending point of the line segment.
	\param points				The list of points in the polyline.
	\param count				The number of points in the \c points array.
	\param[out] collPoint		If an intersection is detected between the line segment and the polyline, 
								this parameter is updated with the position of the intersection. The 
								altitude of this collision point is interpolated between the altitudes of 
								\c p1 and \c p2.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kyruntime_core */
KyBool Intersect2D_Segment_Polyline(const Vec3f& p1, const Vec3f& p2, const Vec3f* points, KyUInt32 count, Vec3f* collPoint=KY_NULL);

/*!	Tests whether or not a specified polyline intersects the area of a specified
	capsule in two dimensions, ignoring their altitude, and retrieves the first
	collision point along the polyline.
	\param p1					The starting point of the capsule.
	\param p2					The ending point of the capsule.
	\param halfWidth			The radius of the capsule.
	\param points				The list of points in the polyline.
	\param count				The number of points in the \c points array.
	\param[out] collisionPoint	If an intersection is detected between the polyline and the capsule, 
								this parameter is updated with the first position along the capsule axis
								where the moving circle intersects the polyline. The altitude of this
								collision point is interpolated between the altitudes of the two polyline
								points that define the intersecting line segment.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kyruntime_core */
KyBool Intersect2D_Capsule_Polyline(const Vec3f& p1, const Vec3f& p2, KyFloat32 halfWidth,
								  const Vec3f* points, KyUInt32 count, Vec3f* collisionPoint=KY_NULL);

/*!	Tests whether or not two rectangles intersect in two dimensions, ignoring their altitude.
	\param r1_center			The center of the first rectangle.
	\param r1_main2DAxis		The two-dimensional orientation of the main axis of the first rectangle.
	\param r1_length			The extent of the first rectangle along its main axis.
	\param r1_width				The extent of the first rectangle along the direction orthogonal 
								to its main axis.
	\param r2_center			The center of the second rectangle.
	\param r2_main2DAxis		The two-dimensional orientation of the main axis of the second rectangle.
	\param r2_length			The extent of the second rectangle along its main axis.
	\param r2_width				The extent of the second rectangle along the direction orthogonal 
								to its main axis.
	\return #KY_TRUE if an intersection was detected, or #KY_FALSE otherwise.
	\ingroup kyruntime_core */
KyBool Intersect2D_Rectangle_Rectangle(const Vec3f& r1_center, const Vec2f& r1_main2DAxis, KyFloat32 r1_length, KyFloat32 r1_width,
							 const Vec3f& r2_center, const Vec2f& r2_main2DAxis, KyFloat32 r2_length, KyFloat32 r2_width);

} //namespace Kaim

#endif // KY_COLLISION_H
