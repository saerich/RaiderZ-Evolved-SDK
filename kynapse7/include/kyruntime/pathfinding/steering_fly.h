/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_STEERING_FLY_H
#define KY_STEERING_FLY_H

#include <kyruntime/pathfinding/isteering.h>


namespace Kaim
{
/*!	This implementation of the ISteering interface supports the movement characteristics of flying 
	\Bots, producing a desired vertical speed in addition to a desired orientation and linear speed. 
	\pp Note that if you use this modifier, the Bot using the PathFinder must have the 
	BodyCanFly class of BodyAttribute, and its Action class must have the ActionSpeed, ActionVerticalSpeed 
	and ActionRotate classes of ActionAttribute.
	\pp This modifier does not manage strafing movements in three dimensions.
	\pp See also :USERGUIDE:"Flying".
	\ingroup kyruntime_pathfinding */
class Steering_Fly: public ISteering
{
public:
	KY_DECLARE_STEERING(Steering_Fly);


public:
	/*!	\label_constructor */
	Steering_Fly():
		ISteering() {}

	virtual KyBool ComputeAction(const Bot& entity, KyFloat32 maxSpeed,
		const PointWrapper& targetPoint,  KyFloat32 targetSpeed, const Vec3f* targetOrientation,
		Action& action);

	virtual KyBool GetTime(const Bot& entity, const PointWrapper& startingPosition,
		KyFloat32 startingSpeed, const Vec3f& startingOrientation, KyFloat32 maxSpeed,
		const PointWrapper& targetPoint, KyFloat32& targetSpeed, const Vec3f* targetOrientation,
		KyFloat32& time);

protected:
	virtual void OnSetPathFinder() {}
};

} // namespace Kaim

#endif // KY_STEERING_FLY_H
