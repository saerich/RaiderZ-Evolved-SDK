/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathobjects */

// primary contact: MAMU - secondary contact: None
#ifndef KYRUNTIME_JUMP_MOVE_H
#define KYRUNTIME_JUMP_MOVE_H

#include "kyruntime/pathobjects/offtheshelf/imove.h"
#include "kypathdata/containers/chunkbasedpool.h"

namespace Kaim
{

class JumpMoveContext
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	Vec3f m_startPos;
	Vec3f m_targetPos;
	KyFloat32 m_initialSpeed;
	KyFloat32 m_angle;
	Vec2f m_axis;
	KyFloat32 m_axisLength;
	KyFloat32 m_axisSqrlength;
	KyFloat32 m_length;
};

/*!	This implementation of IMove generates an Action that makes a Bot jump toward its target position.
	\pp Requires the Bot using the PathObject to have the ActionSpeed, ActionRotate and ActionForcePosition classes of ActionAttribute.
	\ingroup kyruntime_pathobjects */
class JumpMove : public IMove
{
public:
	/*! \label_constructor */
	JumpMove();

	/*! \label_constructor
		\param jumpHeight							Sets the maximum difference in altitude, in meters, between the starting point of the jump
													and the apex of the jump trajectory.
		\param maxBotsUsingThisMoveAtTheSameTime	Sets the number of \Bots that you expect to use this JumpMove object at the same time.
													This value is used to reserve a pool of contexts that store information about the current state of the JumpMove. 
													Each time a Bot uses this JumpMove object, it retrieves a new context from the pool. */
	JumpMove(KyFloat32 jumpHeight, KyUInt16 maxBotsUsingThisMoveAtTheSameTime);

	/*! \label_virtualdestructor */
	virtual ~JumpMove();

	virtual void BeforeMove(const Vec3f& targetPos, const Vec3f& targetAngle, const Vec3f& botPos, const KyFloat32 speed, PathObjectBehaviorContext& context);
	virtual void Move(const Vec3f& targetPos, const Vec3f& targetAngle, const Vec3f& botPos, const KyFloat32 targetSpeed, Action* action, KyFloat32 deltaFrameTime, PathObjectBehaviorContext& context);
	virtual bool IsTargetReached(const Vec3f& targetPos, const Vec3f& targetAngle, const Vec3f& botPos, const KyFloat32 speed, PathObjectBehaviorContext& context);
	virtual void Render(KyUInt32 r, KyUInt32 g, KyUInt32 b, const PathObjectBehaviorContext& context) const;
	virtual void AfterMove(PathObjectBehaviorContext& context);

	/*! Renders the complete jump trajectory from beginning to end. */
	void Render(const Vec3f& start, const Vec3f& end, const KyUInt32 r, const KyUInt32 g, const KyUInt32 b) const;

protected:
	void Init(KyFloat32 min_2D_dist_to_goal, KyFloat32 min_vertical_dist_to_goal, KyFloat32 jumpHeight,
		KyUInt16 maxBotsUsingThisMoveAtTheSameTime);
	void RenderSegments(const Vec3f& start, const Vec3f& end, const Vec2f& axis, const KyUInt32 r, const KyUInt32 g, const KyUInt32 b, const KyFloat32 increment = 0.01f) const;
	void RenderSegment(const Vec3f& start, const Vec3f& end, Vec3f& segmentStart, const KyFloat32 nextCurvAbsisca, const Vec2f& axis, const KyUInt32 r, const KyUInt32 g, const KyUInt32 b) const;

public:
	KyFloat32 m_min_2D_dist_to_goal;
	KyFloat32 m_min_vertical_dist_to_goal;
	KyFloat32 m_jumpHeight;

protected:
	ChunkBasedPool<JumpMoveContext>* m_contextPool;
};

}// namespace Kaim

#endif // KYRUNTIME_JUMP_MOVE_H
