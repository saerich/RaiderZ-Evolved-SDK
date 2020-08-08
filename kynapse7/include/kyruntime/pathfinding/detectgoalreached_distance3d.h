/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_DETECTGOALREACHED_DISTANCE3D_H
#define KY_DETECTGOALREACHED_DISTANCE3D_H

#include <kyruntime/pathfinding/idetectgoalreached.h>


namespace Kaim
{

/*!	This implementation of IDetectGoalReached considers the final destination point to be
	reached if the straight-line distance between the current position of the Bot and the 
	position of the destination point is less than the value of the PathFinder::m_distMin
	class member. Once this condition is achieved, DetectGoalReached_Distance3D considers the destination
	to be reached as long as the distance from the Bot to its goal continues to be less
	than the value of the PathFinder::m_distMax class member. 
	\pp These complementary threshold values create a hysteresis that helps to prevent the 
	Bot from oscillating between stopping and starting when its position is very close to
	the threshold. See :USERGUIDE:"Eliminating Oscillation through Hysteresis".
	\ingroup kyruntime_pathfinding */
class DetectGoalReached_Distance3D: public IDetectGoalReached
{
public:
	KY_DECLARE_DETECTGOALREACHED(DetectGoalReached_Distance3D)


public:
	/*!	\label_constructor */
	DetectGoalReached_Distance3D():
		IDetectGoalReached() {}

	virtual void ReInit();

	virtual KyBool DetectGoalReached(const PointWrapper &pos, const PointWrapper &goal);


public:
	virtual void OnSetPathFinder();

	KyBool m_goalReached; /*!< Indicates the result of the last call to DetectGoalReached(). */
};

} // namespace Kaim

#endif // KY_DETECTGOALREACHED_DISTANCE3D_H
