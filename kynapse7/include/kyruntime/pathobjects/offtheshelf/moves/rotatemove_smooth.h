/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathobjects */

// primary contact: MAMU - secondary contact: None
#ifndef KYRUNTIME_ROTATE_MOVE_SMOOTH_H
#define KYRUNTIME_ROTATE_MOVE_SMOOTH_H

#include "kyruntime/pathobjects/offtheshelf/imove.h"
#include "kypathdata/containers/chunkbasedpool.h"

namespace Kaim
{

class RotateMoveSmoothContext
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	Vec3f m_startAngle;
	Vec3f m_targetAngle;
	Vec3f m_startPos;
};

/*!	This implementation of IMove generates an Action that rotates a Bot toward a target orientation with a given rotation speed.
	\pp Requires the Bot using the PathObject to have the ActionForcePosition class of ActionAttribute.
	\ingroup kyruntime_pathobjects */
class RotateMove_Smooth : public IMove
{
public:
	/*! \label_constructor */
	RotateMove_Smooth();

	/*! \label_constructor
		\param maxBotsUsingThisMoveAtTheSameTime	Sets the number of \Bots that you expect to use this RotateMove_Smooth object at the same time.
													This value is used to reserve a pool of contexts that store information about the current state of the RotateMove_Smooth. 
													Each time a Bot uses this RotateMove_Smooth object, it retrieves a new context from the pool. */
	RotateMove_Smooth(KyUInt16 maxBotsUsingThisMoveAtTheSameTime);

	/*! \label_virtualdestructor */
	virtual ~RotateMove_Smooth();

	virtual void BeforeMove(const Vec3f& /*targetPos*/, const Vec3f& /*targetAngle*/, const Vec3f& /*botPos*/, const KyFloat32 /*speed*/, PathObjectBehaviorContext& /*context*/);
	virtual void Move(const Vec3f& targetPos, const Vec3f& targetAngle, const Vec3f& botPos, const KyFloat32 targetSpeed, Action* action, KyFloat32 deltaFrameTime, PathObjectBehaviorContext& context);
	virtual bool IsTargetReached(const Vec3f& targetPos, const Vec3f& targetAngle, const Vec3f& botPos, const KyFloat32 speed, PathObjectBehaviorContext& context);
	virtual void Render(KyUInt32 /*r*/, KyUInt32 /*g*/, KyUInt32 /*b*/, const PathObjectBehaviorContext& /*context*/) const;
	virtual void AfterMove(PathObjectBehaviorContext& /*context*/);

protected:
	virtual void Init(KyFloat32 rotationSpeed, KyFloat32 min_angle_to_target_angle, KyUInt16 maxBotsUsingThisMoveAtTheSameTime);
	virtual KyFloat32 SmoothOrientation(KyFloat32 currentYawInDegree, KyFloat32 wantedYawInDegree, KyFloat32 deltaFrameTime);

public:
	KyFloat32 m_rotationSpeed;				//!< The speed of rotation in number of degrees per second.
	KyFloat32 m_min_angle_to_target_angle;	//!< The angle delta with which we consider that the target angle has been reached.

protected:
	ChunkBasedPool<RotateMoveSmoothContext>* m_contextPool;
};

}// namespace Kaim

#endif // KYRUNTIME_ROTATE_MOVE_SMOOTH_H
