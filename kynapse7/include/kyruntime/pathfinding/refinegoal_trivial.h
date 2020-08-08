/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_REFINEGOAL_TRIVIAL_H
#define KY_REFINEGOAL_TRIVIAL_H

#include <kyruntime/pathfinding/irefinegoal.h>

namespace Kaim
{
/*! This implementation of the IRefineGoal interface adopts the same destination point
	provided to PathFinder::FindNextMove() as the refined goal, without adjusting the
	position of the destination point at all. If this destination point is outside the
	boundaries of the NavMesh, pathfinding will fail and the PathFinder::FindNextMove()
	method will return #KY_FALSE.
	\ingroup kyruntime_pathfinding */
class RefineGoal_Trivial: public IRefineGoal
{
public:
	KY_DECLARE_REFINEGOAL(RefineGoal_Trivial)
public:
	/*! \label_constructor */
	RefineGoal_Trivial():
		IRefineGoal() {}

	virtual void RefineGoal(const PointWrapper& goal, PointWrapper& refinedGoal);

protected:
	virtual void OnSetPathFinder() {}
};

} // namespace Kaim

#endif // KY_REFINEGOAL_TRIVIAL_H
