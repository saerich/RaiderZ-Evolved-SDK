/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file 
	\ingroup kyruntime_agents */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_AGENTUTILS_H
#define KY_AGENTUTILS_H

#include <kypathdata/generic/types.h>
#include <kypathdata/math/vec3f.h>


namespace Kaim
{

class Body;
class Bot;


/*!	Projects a point onto a line segment, and calculates the minimum distance from the point to the 
	line segment.
	\param segmentP1			The starting point of the line segment.
	\param segmentP2			The ending point of the line segment.
	\param testedPoint			The point to project onto the line segment.
	\param[out] distance		Stores the minimum distance between \c testedPoint and the line segment.
	\param projectionPoint		The point at which \c testedPoint is projected onto the line segment.
	\return #KY_TRUE if the point was successfully projected onto the line segment, or #KY_FALSE otherwise.
	\ingroup kyruntime_agents */
KyBool DistanceToSegment(const Kaim::Vec3f& segmentP1, const Kaim::Vec3f& segmentP2, 
					 const Kaim::Vec3f& testedPoint, KyFloat32& distance, 
					 Kaim::Vec3f &projectionPoint);


/*!	Calculates the area of intersection between two circles that lie on the same 2D plane.
	\param distanceBetweenCenters	The distance between the centers of the two circles.
	\param radius1					The radius of the first circle.
	\param radius2					The radius of the second circle.
	\return The area of intersection of the given two circles.
	\ingroup kyruntime_agents */
KyFloat32 GiveCoplanarCirclesIntersectionArea(const KyFloat32 distanceBetweenCenters, 
	const KyFloat32 radius1, const KyFloat32 radius2);

/*!	Determines whether or not a Body lies inside a cone of visibility: a conical segment of a sphere
	centered around a given vertex point. 
	\pp This function is typically more accurate than Kaim::InViewCone(), as it takes into account the 
	bounding box of the tested Body, calculated from the radius and height values set for the Body.
	\pp Note that this test is not a raycast; it does not take into account collision objects or 
	dynamic obstacles that may lie between the center and the tested Body. It simply tests whether 
	or not any part of the bounding box of the Body lies within the cone.
	\param body			The Body to test.
	\param position		The center of the sphere (i.e. the vertex of the visibility cone). This point is typically
						the position of the eyes or weapon of a viewer.
	\param direction	A normalized vector that indicates the direction of the axis of the cone.
	\param coneAngle	The angle between the orientation defined by \c direction and each "border" of the cone.
						This value should be greater than 0 and less than 90.
	\param length		The radius of the sphere (i.e. the length of the cone), in meters. This parameter determines
						the maximum distance at which \c body may be seen. A negative value indicates an infinitely 
						long cone.
	\return #KY_TRUE if the Body is in the view cone, or #KY_FALSE otherwise.
	\ingroup kyruntime_agents */
KyBool EnhancedInViewCone( Kaim::Body* body, const Kaim::Vec3f& position,
	const Kaim::Vec3f& direction, const KyFloat32 coneAngle, const KyFloat32 length);

/*!	This function determines whether or not a Body lies inside a cone of visibility: a conical segment of a
	sphere centered around a given vertex point. 
	\pp This function is typically more accurate than Kaim::InViewCone(), as it takes into account the 
	bounding box of the tested Bot, calculated from the radius and height values set for its Body.
	\pp Note that this test is not a raycast; it does not take into account collision objects or 
	dynamic obstacles that may lie between the center and the tested Bot. It simply tests whether 
	or not any part of the bounding box of the Bot lies within the cone.
	\param entity		The Bot to test.
	\param position		The center of the sphere (i.e. the vertex of the visibility cone). This point is typically
						the position of the eyes or weapon of a viewer.
	\param direction	A normalized vector that indicates the direction of the axis of the cone.
	\param coneAngle	The angle between the orientation defined by \c direction and each "border" of the cone.
						This value should be greater than 0 and less than 90.
	\param length		The radius of the sphere (i.e. the length of the cone), in meters. This parameter determines
						the maximum distance at which \c entity may be seen. A negative value indicates an infinitely 
						long cone.
	\return #KY_TRUE if the Bot is in the view cone, or #KY_FALSE otherwise.
	\ingroup kyruntime_agents */
KyBool EnhancedInViewCone( Kaim::Bot* entity, const Kaim::Vec3f& position,
	const Kaim::Vec3f& direction, const KyFloat32 coneAngle, const KyFloat32 length);

} // namespace Kaim

#endif // KY_AGENTUTILS_H
