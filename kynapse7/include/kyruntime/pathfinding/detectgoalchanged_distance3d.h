/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_DETECTGOALCHANDED_DISTANCE3D_H
#define KY_DETECTGOALCHANDED_DISTANCE3D_H

#include <kyruntime/pathfinding/idetectgoalchanged.h>


namespace Kaim
{
/*!	This implementation of the IDetectGoalChanged interface determines that the destination
	point has changed significantly when the distance between the original destination and
	the new destination exceeds the a configurable distance threshold.
	\pp This class also handles a special case: when the previous Path computation failed 
	because no vertex could be found that was reachable from the old destination
	point, DetectGoalChanged_Distance3D will consider even very small changes in the destination point to
	be significant, as that small change may cause a vertex to become reachable.
	For example, if the destination is set to the position of another Bot that is newly 
	spawned slightly above the valid altitude range of the spatial graph, no vertex 
	may be found for that position, and pathfinding will fail. However, when the 
	destination Bot falls a short distance to the ground, the small difference in distance
	is detected and triggers a re-computation of the path. To conserve CPU resources, this test occurs 
	only periodically: every 0.8 seconds when CPU time allows for path following, or every 1.2 
	seconds when no CPU time remains.
	\ingroup kyruntime_pathfinding */
class DetectGoalChanged_Distance3D: public IDetectGoalChanged
{
public:
	KY_DECLARE_DETECTGOALCHANGED(DetectGoalChanged_Distance3D)


public:
	/*! \label_constructor */
	DetectGoalChanged_Distance3D()
		:IDetectGoalChanged()
		,m_maxDist(1.0f)
	{}

	virtual void ReInit();
	virtual KyBool HasGoalChanged(const PointWrapper& oldGoal, const PointWrapper& newGoal);

	/*! Retrieves the value set through a call to MaxDist(KyFloat32). */
	KyFloat32 MaxDist() const { return m_maxDist; }

	/*! Determines how far the destination point may move before DetectGoalChanged_Distance3D re-calculates a new path to that destination.
		\acceptedvalues	any positive value
		\defaultvalue	\c 1.0f */
	void MaxDist(KyFloat32 val)
	{
		KY_FUNCTION("DetectGoalChanged_Distance3D::MaxDist");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'MaxDist' must be > 0.", val));
		m_maxDist = val; 
	}

	KyFloat32 m_lastExtraUpdate; /*!< Stores the last time at which the special case described in the class description was handled. */

protected:
	virtual void OnSetPathFinder();

	KyFloat32 m_maxDist; //< The value stored through MaxDist().

};

} // namespace Kaim

#endif // KY_DETECTGOALCHANDED_DISTANCE3D_H
