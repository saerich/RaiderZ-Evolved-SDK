/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_REFINEGOAL_NEARESTINSIDENAVMESH_H
#define KY_REFINEGOAL_NEARESTINSIDENAVMESH_H

#include <kyruntime/pathfinding/irefinegoal.h>

namespace Kaim
{
/*! This implementation of the IRefineGoal interface tests the destination point provided to the
	PathFinder::FindNextMove() method against the static NavMesh. If the destination is outside 
	the boundaries of the NavMesh, this modifier calculates a refined goal that is as close as 
	possible to the specified destination but inside the NavMesh.
	\pp For example, in the image on the left below, the destination point specified for the PathFinder
	(represented by the flagpole) is outside the NavMesh boundaries (represented by the red
	outlines). It is therefore unreachable to the Bot. The RefineGoal_NearestInsideNavMesh modifier
	displaces the destination point just far enough to bring it inside the NavMesh, as shown in the
	image and inset on the right.
	\pp \image html "graphics/modifiers_irefinegoal.PNG"
	\pp This modifier collects vertices nearby the destination point, and performs an
	ICanGo::TraceLine() test for each to determine whether or not the direct line segment to that
	vertex is clear. If so, the first point along that direct line segment that lies within the NavMesh
	is kept as a candidate. The candidate closest to the original destination is adopted as the final
	refined goal.
	\ingroup kyruntime_pathfinding */
class RefineGoal_NearestInsideNavMesh : public IRefineGoal
{
public:
	KY_DECLARE_REFINEGOAL(RefineGoal_NearestInsideNavMesh)

public:
	/*! \label_constructor */
	RefineGoal_NearestInsideNavMesh();
	void ReInit();

	virtual void RefineGoal(const PointWrapper& goal, PointWrapper& refinedGoal);

protected:
	virtual void OnSetPathFinder() {}

	void AdjustAltitude(NavMeshQueryServerWrapper& navMeshQueryServer, const NavTrianglePtr& navTrianglePtr, PointWrapper& refinedGoal);
	virtual void RefineGoal_FromOutside(const PointWrapper& goal, PointWrapper& refinedGoal);
	virtual KyFloat32 GetCost(const Vec3f& intermediate, const PointWrapper& dest, KyFloat32 radius);

protected:
	Vec3f m_cacheGoal;
	Vec3f m_cacheRefinedGoal;
};

} // namespace Kaim

#endif // KY_REFINEGOAL_NEARESTINSIDENAVMESH_H
