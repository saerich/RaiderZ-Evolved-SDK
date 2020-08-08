/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathobjects */

// primary contact: MAMU - secondary contact: None
#ifndef KYRUNTIME_IMOVE_H
#define KYRUNTIME_IMOVE_H

#include "kypathdata/math/vec3f.h"
#include "kyruntime/pathobjects/offtheshelf/pathobjectbehaviorcontext.h"
#include "kyruntime/core/action.h"

namespace Kaim
{

/*!	This class provides an abstract base interface for an object that a BasePathObjectBehavior can use to
	generate an Action for a Bot that is traversing a PathObject. This allows the PathObject to
	take total control over the movements the Bot performs while traversing the PathObject.
	\pp This class cannot be used as-is. You must use one of the derived classes supplied with the \SDKName SDK, or
	write your own implementation.
	\ingroup kyruntime_pathobjects */
class IMove : public VirtualMemObject
{
public:
	/*! \label_baseconstructor */
	IMove() {}

	/*! \label_virtualdestructor */
	virtual ~IMove() {}

public:
	/*! Called by the BasePathObjectBehavior to initialize a movement.
		\param targetPos			The target position toward which the IMove should direct the Bot.
		\param targetAngle			The preferred orientation for the Bot when it reaches its target position, expressed in degrees of absolute yaw.
		\param botPos				The current position of the Bot.
		\param speed				The preferred speed at which the Bot should move, in meters per second.
		\param context				The PathObjectBehaviorContext that contains information about how the Bot is using the PathObject. */
	virtual void BeforeMove(const Vec3f& targetPos, const Vec3f& targetAngle, const Vec3f& botPos, const KyFloat32 speed, PathObjectBehaviorContext& context) = 0;

	/*! Called by the BasePathObjectBehavior to generate an Action that moves a Bot toward its target point.
		\param targetPos			The target position toward which the IMove should direct the Bot.
		\param targetAngle			The preferred orientation for the Bot when it reaches its target position, expressed in degrees of absolute yaw.
		\param botPos				The current position of the Bot.
		\param targetSpeed			The preferred speed at which the Bot should move, in meters per second.
		\param[out] action			The Action that should be updated.
		\param deltaFrameTime		The time in between each two frames in the \gameOrSim loop, in seconds.
		\param context				The PathObjectBehaviorContext that contains information about how the Bot is using the PathObject. */
	virtual void Move(const Vec3f& targetPos, const Vec3f& targetAngle, const Vec3f& botPos, const KyFloat32 targetSpeed, Action* action, KyFloat32 deltaFrameTime, PathObjectBehaviorContext& context) = 0;

	/*! Called by the BasePathObjectBehavior to determine whether the Bot using the IMove has reached its target point.
		\param targetPos			The target position toward which the IMove should direct the Bot.
		\param targetAngle			The preferred orientation for the Bot when it reaches its target position, expressed in degrees of absolute yaw.
		\param botPos				The current position of the Bot.
		\param speed				The preferred speed at which the Bot should move, in meters per second.
		\param context				The PathObjectBehaviorContext that contains information about how the Bot is using the PathObject. */
	virtual bool IsTargetReached(const Vec3f& targetPos, const Vec3f& targetAngle, const Vec3f& botPos, const KyFloat32 speed, PathObjectBehaviorContext& context) = 0;

	/*! Called by the BasePathObjectBehavior to render information about the movement.
		\param r					\label_redval
		\param g					\label_greenval
		\param b					\label_blueval
		\param context				The PathObjectBehaviorContext that contains information about how the Bot is using the PathObject. */
	virtual void Render(KyUInt32 r, KyUInt32 g, KyUInt32 b, const PathObjectBehaviorContext& context) const = 0;

	/*! Called by the BasePathObjectBehavior to finalize a movement when the Bot reaches its target point.
		\param context				The PathObjectBehaviorContext that contains information about how the Bot is using the PathObject. */
	virtual void AfterMove(PathObjectBehaviorContext& context) = 0;
};

}// namespace Kaim

#endif // KYRUNTIME_IMOVE_H
