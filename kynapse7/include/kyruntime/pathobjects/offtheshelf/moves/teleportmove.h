/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathobjects */

// primary contact: MAMU - secondary contact: None
#ifndef KYRUNTIME_TELEPORT_MOVE_H
#define KYRUNTIME_TELEPORT_MOVE_H

#include "kyruntime/pathobjects/offtheshelf/imove.h"

namespace Kaim
{

/*!	This implementation of IMove generates an Action that teleports a Bot directly to its target point.
	\pp Requires the Bot using the PathObject to have the ActionForcePosition class of ActionAttribute.
	\ingroup kyruntime_pathobjects */
class TeleportMove : public IMove
{
public:
	/*! \label_constructor */
	TeleportMove() : m_min_2D_dist_to_goal(0.2f), m_min_vertical_dist_to_goal(0.1f) {}

	/*! \label_virtualdestructor */
	~TeleportMove() {}

	virtual void BeforeMove(const Vec3f& /*targetPos*/, const Vec3f& /*targetAngle*/, const Vec3f& /*botPos*/, const KyFloat32 /*speed*/, PathObjectBehaviorContext& /*context*/) {}
	virtual void Move(const Vec3f& targetPos, const Vec3f& targetAngle, const Vec3f& botPos, const KyFloat32 targetSpeed, Action* action, KyFloat32 deltaFrameTime, PathObjectBehaviorContext& context);
	virtual bool IsTargetReached(const Vec3f& targetPos, const Vec3f& targetAngle, const Vec3f& botPos, const KyFloat32 speed, PathObjectBehaviorContext& context);
	virtual void Render(KyUInt32 /*r*/, KyUInt32 /*g*/, KyUInt32 /*b*/, const PathObjectBehaviorContext& /*context*/) const {}
	virtual void AfterMove(PathObjectBehaviorContext& /*context*/) {}

private:
	KyFloat32 m_min_2D_dist_to_goal;
	KyFloat32 m_min_vertical_dist_to_goal;
};

}// namespace Kaim

#endif // KYRUNTIME_TELEPORT_MOVE_H
