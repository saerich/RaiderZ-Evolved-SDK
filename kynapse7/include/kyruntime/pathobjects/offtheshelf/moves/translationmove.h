/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathobjects */

// primary contact: MAMU - secondary contact: None
#ifndef KYRUNTIME_TRANSLATION_MOVE_H
#define KYRUNTIME_TRANSLATION_MOVE_H

#include "kyruntime/pathobjects/offtheshelf/imove.h"
#include "kypathdata/containers/chunkbasedpool.h"

namespace Kaim
{

class TranslationMoveContext
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	Vec3f m_startPos;
	Vec3f m_targetPos;
};

/*!	This implementation of IMove generates an Action that moves a Bot toward its target position in three dimensions.
	\pp Requires the Bot using the PathObject to have the ActionRotate and ActionForcePosition classes of ActionAttribute.
	\ingroup kyruntime_pathobjects */
class TranslationMove : public IMove
{
public:
	/*! \label_constructor */
	TranslationMove();

	/*! \label_constructor
		\param maxBotsUsingThisMoveAtTheSameTime	Sets the number of \Bots that you expect to use this TranslationMove object at the same time.
													This value is used to reserve a pool of contexts that store information about the current state of the TranslationMove. 
													Each time a Bot uses this TranslationMove object, it retrieves a new context from the pool. */
	TranslationMove(KyUInt16 maxBotsUsingThisMoveAtTheSameTime);

	/*! \label_virtualdestructor */
	~TranslationMove();

	virtual void BeforeMove(const Vec3f& targetPos, const Vec3f& targetAngle, const Vec3f& botPos, const KyFloat32 speed, PathObjectBehaviorContext& context);
	virtual void Move(const Vec3f& targetPos, const Vec3f& targetAngle, const Vec3f& botPos, const KyFloat32 targetSpeed, Action* action, KyFloat32 deltaFrameTime, PathObjectBehaviorContext& context);
	virtual bool IsTargetReached(const Vec3f& targetPos, const Vec3f& targetAngle, const Vec3f& botPos, const KyFloat32 speed, PathObjectBehaviorContext& context);
	virtual void Render(KyUInt32 /*r*/, KyUInt32 /*g*/, KyUInt32 /*b*/, const PathObjectBehaviorContext& context) const;
	virtual void AfterMove(PathObjectBehaviorContext& context);

protected:
	void Init(KyFloat32 min_2D_dist_to_goal, KyFloat32 min_vertical_dist_to_goal, KyUInt16 maxBotsUsingThisMoveAtTheSameTime);

public:
	KyFloat32 m_min_2D_dist_to_goal;
	KyFloat32 m_min_vertical_dist_to_goal;

protected:
	ChunkBasedPool<TranslationMoveContext>* m_contextPool;
};

}// namespace Kaim

#endif // KYRUNTIME_TRANSLATION_MOVE_H
