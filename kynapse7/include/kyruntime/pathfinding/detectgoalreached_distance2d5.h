/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_DETECTGOALREACHED_DISTANCE2D5_H
#define KY_DETECTGOALREACHED_DISTANCE2D5_H

#include <kyruntime/pathfinding/idetectgoalreached.h>


namespace Kaim
{
/*!	This implementation of IDetectGoalReached considers the final destination point to be
	reached if both of the following conditions are true:
	-	The horizontal straight-line distance between the current position of the Bot and the 
		position of the destination point is less than the value of the PathFinder::m_distMin
		class member.
	-	The difference in altitude between the current position of the Bot and the position of the 
		destination point is less than a configurable altitude threshold set through MaxDeltaHeight(KyFloat32),
		OR a call to ICanGo::CanGo() indicates that the Bot can move directly to its destination
		point.
	\pp Once these conditions are achieved, DetectGoalReached_Distance2D5 considers the destination to be reached as 
	long as both of the following conditions are true:
	-	The horizontal distance from the Bot to its goal continues to be less than the value of 
		the PathFinder::m_distMax class member.
	-	The difference in altitude is less than the configurable altitude threshold set through MaxDeltaHeight(KyFloat32). 
	\pp These complementary threshold values create a hysteresis that helps to prevent the 
	Bot from oscillating between stopping and starting when its position is very close to
	the distance threshold. See also :USERGUIDE:"Eliminating Oscillation through Hysteresis".
	\ingroup kyruntime_pathfinding */
class DetectGoalReached_Distance2D5: public IDetectGoalReached
{
public:
	KY_DECLARE_DETECTGOALREACHED(DetectGoalReached_Distance2D5)


public:
	/*!	\label_constructor */
	DetectGoalReached_Distance2D5(): IDetectGoalReached() {}
	virtual KyBool DetectGoalReached(const PointWrapper &pos, const PointWrapper &goal);

	/*! Retrieves the value set through a call to MaxDeltaHeight(KyFloat32). */
	KyFloat32 MaxDeltaHeight() const { return m_maxDeltaHeight; }

	/*! Determines the maximum allowable height differential between a Bot and a goal point in order for DetectGoalReached_Distance2D5
		to consider that goal reached. 
		\acceptedvalues	any positive value
		\defaultvalue	approximately half the height of the Bot, plus a small error margin */
	void MaxDeltaHeight(KyFloat32 val)
	{
		KY_FUNCTION("DetectGoalReached_Distance2D5::MaxDeltaHeight");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'MaxDeltaHeight' must be > 0.", val));
		m_maxDeltaHeight = val;
	}

protected:
	virtual void OnSetPathFinder();


protected:
	KyFloat32 m_maxDeltaHeight; //< \The value set through MaxDeltaHeight().
};

} // namespace Kaim

#endif // KY_DETECTGOALREACHED_DISTANCE2D5_H
