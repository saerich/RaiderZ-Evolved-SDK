/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KyRuntime_DetectGoalReached_DontQueue_H
#define KyRuntime_DetectGoalReached_DontQueue_H

#include <kyruntime/pathfinding/detectgoalreached_distance2d5.h>


namespace Kaim
{
/*!	This implementation of IDetectGoalReached is intended for use in conjunction with
	ComputeTargetPoint_Queue. DetectGoalReached_DontQueue derives from 
	DetectGoalReached_Distance2D5, and uses the same horizontal and vertical distance
	thresholds to determine whether or not the destination has been reached. However, 
	it will also consider the destination to be reached if both of the following 
	conditions are true:
	-	The Bot has reached the target point computed by ComputeTargetPoint_Queue, which indicates
		that the Bot has gotten as close to its destination point as it possibly can, and has been 
		blocked by other \Bodies.
	-	The Bot is closer to the target point than the value set through MaxDist().
	\pp When DetectGoalReached_DontQueue considers the destination point to be reached due to these circumstances,
	it periodically returns #KY_FALSE, in case the other \Bodies blocking the destination point 
	have moved out of the way. This check is performed once per second by default; this period
	can be changed by changing the value of the #m_timeLimit member.
	\ingroup kyruntime_pathfinding */
class DetectGoalReached_DontQueue: public DetectGoalReached_Distance2D5
{
public:
	KY_DECLARE_DETECTGOALREACHED(DetectGoalReached_DontQueue)


public:
	/*!	\label_constructor */
	DetectGoalReached_DontQueue():
		DetectGoalReached_Distance2D5(),
		m_maxDist(10.f)
	{}

	virtual void ReInit();

	virtual KyBool DetectGoalReached(const PointWrapper &pos, const PointWrapper &goal);

	KyFloat32 MaxDist() const { return m_maxDist; }
	void MaxDist(KyFloat32 val)
	{
		KY_FUNCTION("DetectGoalReached_DontQueue::MaxDist");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'MaxDist' must be > 0.", val));
		m_maxDist = val; 
	}

protected:
	virtual KyResult CheckModifierDependencies(const Modifiers&, const Kaim::ILpfManager::LpfContext*);
	virtual void OnSetPathFinder();

	KyFloat32 m_maxDist; //< The value set through MaxDist().
	KyFloat32 m_timeLimit; /*!< The period at which this modifier returns #KY_FALSE when other \Bodies block the destination point. */
};

} // namespace Kaim

#endif // KyRuntime_DetectGoalReached_DontQueue_H
