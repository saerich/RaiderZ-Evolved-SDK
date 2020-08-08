/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_REFINEGOAL_OUTSIDENAVMESH_H
#define KY_REFINEGOAL_OUTSIDENAVMESH_H

#include <kyruntime/pathfinding/refinegoal_lpfcompatible.h>


namespace Kaim
{
/*!	This implementation of the IRefineGoal modifier is derived from RefineGoal_LpfCompatible modifier,
	and uses the same process to find a valid, reachable destination within the bounds of the NavMesh and outside any 
	\Obstacles reflected in the \LpfContext set for the PathFinder. However, this modifier goes further by actually
	directing the PathFinder outside the bounds of the NavMesh to make the Bot reach the actual destination point
	originally passed to PathFinder::FindNextMove().
	\pp When the destination point is outside of the NavMesh, this modifier uses the same process as
	RefineGoal_LpfCompatible to refine the goal point to the nearest valid location. The PathFinder moves its Bot 
	toward this point. Then, when the Bot gets very close to the refined goal point inside the NavMesh, this modifier 
	switches the refined goal back to your original destination outside the NavMesh.
	\pp The distance threshold at which this modifier switches the refined goal back to the original destination is set 
	through a call to ThresholdDist(KyFloat32). 
	\pp Once your Bot is outside of the NavMesh, it is outside the boundaries generally considered valid by the PathFinder 
	It therefore becomes your responsibility to guide the Bot back into the bounds of the NavMesh before you 
	can calculate a new path for your Bot. In addition, \SDKName cannot take responsibility for guiding your Bot 
	while outside of the NavMesh. See also :USERGUIDE:"Moving Outside of the NavMesh".
	\ingroup kyruntime_pathfinding */
class RefineGoal_ToOutsideNavMesh: public RefineGoal_LpfCompatible
{
public:
	KY_DECLARE_REFINEGOAL(RefineGoal_ToOutsideNavMesh)


public:
	/*!	\label_constructor */
	RefineGoal_ToOutsideNavMesh();
	
	void ReInit();

	virtual void RefineGoal(const PointWrapper& goal, PointWrapper& refinedGoal);

	/*! Retrieves the value set through a call to ThresholdDist(KyFloat32). */
	KyFloat32 ThresholdDist() const { return m_thresholdDist; }

	/*! Determines the distance from the refined goal (the temporary goal inside the NavMesh) at which the PathFinder switches
		back to the original goal outside the NavMesh.
		\pp Note that if you set a very small value for this parameter, the IDetectGoalReached modifier may consider the refined
		goal to be reached before the ThresholdDist trigger is met. In this case, the Bot will simply stop the moment that its
		goal is considered reached, and will never proceed to the original destination outside of the NavMesh. To avoid this
		behavior, the value of the IDetectGoalReached_Distance3D::DistMin() or IDetectGoalReached_Distance2D5::DistMin() value is used
		instead of the ThresholdDist() value if it is lower than ThresholdDist(). If you write a custom IDetectGoalReached
		class that uses an alternative mechanism to determine when the final goal is reached, you must ensure that your custom
		IDetectGoalReached class is compatible with RefineGoal_OutsideAiMesh class. See also Fpd::IDetectGoalReached. 
		\units			meters
		\acceptedvalues	any positive value
		\defaultvalue	\c 0.75f */
	void ThresholdDist(KyFloat32 val)
	{ 
		KY_FUNCTION("RefineGoal_ToOutsideNavMesh::ThresholdDist");
		KY_ASSERT(val >= 0.f, ("Invalid value (%.2f), 'ThresholdDist' must be >= 0.", val));
		m_thresholdDist = val; 
	}

	KyBool m_goingToRG; /*!< Indicates whether or not the Entity is using the refined goal inside the NavMesh. Do not modify. */

protected:
	/*!	\c ThresholdDist configuration parameter. */
	KyFloat32 m_thresholdDist;
	
	Vec3f m_nearestNavMeshRG; /*!< For internal use. Do not modify. */
	
};

} // namespace Kaim

#endif // KY_REFINEGOAL_OUTSIDENAVMESH_H
