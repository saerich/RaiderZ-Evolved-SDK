/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_core */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_GLOBAL_H
#define KY_GLOBAL_H

#include <kypathdata/generic/types.h>


namespace Kaim
{

class Vec3f;

/*!	Tests whether or not a given target point is contained inside a cone of visibility: a 
	conical segment of a sphere centered around a given vertex point.
	\pp This function may be useful to test whether an object or point is within sight or firing
	range of a Bot. 
	\pp Note that this test is not a raycast; it does not take into account collision objects or 
	dynamic obstacles that may lie between the center and target points. It simply tests whether 
	the target point lies within the cone.
	\param target 		Coordinates of the target point to test.
	\param coneVertex 	Center of the sphere (or vertex of the cone). This point is typically
						the position of the eyes or weapon of an Bot.
	\param coneAxis 	Normalized vector that gives the direction of the axis of the cone.
	\param angleDeg 	Angle between the orientation defined by \c coneAxis and each "border" of the cone.
						This value should be greater than 0 and less than 90. 
	\param coneHeight 	Radius of the sphere (or length of the cone), in world units. 
						A negative value indicates an infinitely long cone.
	\return #KY_TRUE if the point is inside the cone, or #KY_FALSE otherwise.
	\ingroup kyruntime_core */
KyBool InViewCone( const Vec3f& target, const Vec3f& coneVertex, const Vec3f& coneAxis, 
				  KyFloat32 angleDeg, KyFloat32 coneHeight = -1.f);

} //namespace Kaim


#endif // KY_GLOBAL_H
