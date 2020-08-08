/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_DETECTPATHNODEREACHED_DISTANCE3D_H
#define KY_DETECTPATHNODEREACHED_DISTANCE3D_H

#include <kyruntime/pathfinding/idetectpathnodereached.h>


namespace Kaim
{
/*!	This implementation of IDetectPathNodeReached considers the current PathNode to be
	reached if the straight-line distance between the current position of the Bot and the 
	position of the PathNode is less than a configurable distance threshold set through
	a call to MaxDist(KyFloat32).
	\ingroup kyruntime_pathfinding */
class DetectPathNodeReached_Distance3D: public IDetectPathNodeReached
{
public:
	KY_DECLARE_DETECTPATHNODEREACHED(DetectPathNodeReached_Distance3D)


public:
	/*!	\label_constructor */
	DetectPathNodeReached_Distance3D()
		:IDetectPathNodeReached()
		,m_maxDist(0.1f)
		{}

	virtual KyBool DetectPathNodeReached(const PointWrapper &pos, const PointWrapper &pathNodePosition);

	/*! Retrieves the value set through a call to MaxDist(KyFloat32). */
	KyFloat32 MaxDist() const { return m_maxDist; }

	/*! Determines the maximum distance from a PathNode at which DetectPathNodeReached_Distance3D considers that PathNode reached. If the distance
		between the Bot and its destination point is greater than this value, the PathNode is not considered reached.
		\acceptedvalues	any positive value
		\defaultvalue	\c 0.1f */
	void MaxDist(KyFloat32 val)
	{
		KY_FUNCTION("DetectPathNodeReached_Distance3D::MaxDist");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'MaxDist' must be > 0.", val));
		m_maxDist = val; 
	}

protected:
	virtual void OnSetPathFinder() {}

	KyFloat32 m_maxDist; //< The value set through MaxDist().
};

} // namespace Kaim

#endif // KY_DETECTPATHNODEREACHED_DISTANCE3D_H
