/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_DETECTPATHNODEREACHED_DISTANCE2D5_H
#define KY_DETECTPATHNODEREACHED_DISTANCE2D5_H

#include <kyruntime/pathfinding/idetectpathnodereached.h>


namespace Kaim
{
/*!	This implementation of IDetectPathNodeReached considers the current PathNode to be
	reached if both of the following conditions are true:
	-	The horizontal straight-line distance between the current position of the Bot and the 
		position of the current PathNode is less than a configurable distance threshold set through
		a call to MaxDist(KyFloat32).
	-	The difference in altitude between the current position of the Entity and the position of the 
		destination point is less than a configurable altitude threshold set through a call to 
		MaxDeltaHeight(KyFloat32).
	\ingroup kyruntime_pathfinding */
class DetectPathNodeReached_Distance2D5: public IDetectPathNodeReached
{
public:
	KY_DECLARE_DETECTPATHNODEREACHED(DetectPathNodeReached_Distance2D5)


public:
	/*!	\label_constructor */
	DetectPathNodeReached_Distance2D5()
		:IDetectPathNodeReached()
		,m_maxDist(0.1f)
		,m_maxDeltaHeight(KyFloat32MAXVAL)
		{}


	virtual KyBool DetectPathNodeReached(const PointWrapper &pos, const PointWrapper &pathNodePosition);

	/*! Retrieves the value set through a call to MaxDist(KyFloat32). */
	KyFloat32 MaxDist() const { return m_maxDist; }

	/*! Determines the maximum distance from a PathNode at which DetectPathNodeReached_Distance2D5 considers that PathNode reached. If the distance
		between the Bot and its destination point is greater than this value, the PathNode is not considered reached.
		\acceptedvalues	any positive value
		\defaultvalue	\c 0.1f */
	void MaxDist(KyFloat32 val)
	{
		KY_FUNCTION("DetectPathNodeReached_Distance2D5::MaxDist");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'MaxDist' must be > 0.", val));
		m_maxDist = val; 
	}

	/*! Retrieves the value set through a call to MaxDeltaHeight(KyFloat32). */
	KyFloat32 MaxDeltaHeight() const { return m_maxDeltaHeight; }

	/*! Determines the maximum allowable height differential between a Bot and a PathNode in order for DetectGoalReached_Distance2D5
		to consider that PathNode reached. 
		\acceptedvalues	any positive value
		\defaultvalue	half the height of the Bot */
	void MaxDeltaHeight(KyFloat32 val) 
	{
		KY_FUNCTION("DetectPathNodeReached_Distance2D5::MaxDeltaHeight");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'MaxDeltaHeight' must be > 0.", val));
		m_maxDeltaHeight = val; 
	}

protected:
	virtual void OnSetPathFinder();

	KyFloat32 m_maxDist; //< The value set through MaxDist().
	KyFloat32 m_maxDeltaHeight; //< The value set through MaxDeltaHeight().
};

} // namespace Kaim

#endif // KY_DETECTPATHNODEREACHED_DISTANCE2D5_H
