/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathobjects */

// primary contact: MAMU - secondary contact: None
#ifndef KYRUNTIME_TRANSLATION_MOVE_2D_H
#define KYRUNTIME_TRANSLATION_MOVE_2D_H

#include "kyruntime/pathobjects/offtheshelf/imove.h"
#include "kypathdata/containers/chunkbasedpool.h"

namespace Kaim
{

class TranslationMove_2DContext
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	Vec3f m_startPos;
	Vec3f m_targetPos;
};

/*!	This implementation of IMove generates an Action that moves a Bot two-dimensionally toward its target position.
	\pp Requires the Bot using the PathObject to have the ActionSpeed and ActionRotate classes of ActionAttribute.
	\ingroup kyruntime_pathobjects */
class TranslationMove_2D : public IMove
{
public:
	/*! \label_constructor */
	TranslationMove_2D();

	/*! \label_constructor
		\param maxBotsUsingThisMoveAtTheSameTime	Sets the number of \Bots that you expect to use this TranslationMove_2D object at the same time.
													This value is used to reserve a pool of contexts that store information about the current state of the TranslationMove_2D. 
													Each time a Bot uses this TranslationMove_2D object, it retrieves a new context from the pool. */
	TranslationMove_2D(KyUInt16 maxBotsUsingThisMoveAtTheSameTime);

	/*! \label_virtualdestructor */
	~TranslationMove_2D();

	virtual void BeforeMove(const Vec3f& targetPos, const Vec3f& targetAngle, const Vec3f& botPos, const KyFloat32 speed, PathObjectBehaviorContext& context);
	virtual void Move(const Vec3f& targetPos, const Vec3f& targetAngle, const Vec3f& botPos, const KyFloat32 targetSpeed, Action* action, KyFloat32 deltaFrameTime, PathObjectBehaviorContext& context);
	virtual bool IsTargetReached(const Vec3f& targetPos, const Vec3f& targetAngle, const Vec3f& botPos, const KyFloat32 speed, PathObjectBehaviorContext& context);
	virtual void Render(KyUInt32 /*r*/, KyUInt32 /*g*/, KyUInt32 /*b*/, const PathObjectBehaviorContext& context) const;
	virtual void AfterMove(PathObjectBehaviorContext& context);

protected:
	virtual void Init(KyFloat32 min_2D_dist_to_goal, KyUInt16 maxBotsUsingThisMoveAtTheSameTime);

public:
	KyFloat32 m_min_2D_dist_to_goal;

public:
	ChunkBasedPool<TranslationMove_2DContext>* m_contextPool;
};

}// namespace Kaim

#endif // KYRUNTIME_TRANSLATION_MOVE_H
