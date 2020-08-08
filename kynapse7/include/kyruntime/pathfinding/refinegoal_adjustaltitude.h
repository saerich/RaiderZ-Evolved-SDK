/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_REFINEGOAL_ADJUSTALTITUDE_H
#define KY_REFINEGOAL_ADJUSTALTITUDE_H

#include <kyruntime/pathfinding/irefinegoal.h>

namespace Kaim
{
/*! This implementation of the IRefineGoal interface adjusts the altitude of the destination point to match the
	altitude of a NavMesh triangle that is contained in the PointInfo_NavTriangleSafePtr attached to the PointWrapper. 
	This class is intended for use when altitude of the destination point that you pass to PathFinder::FindNextMove()
	may be far enough from the altitude of the NavMesh that the PathFinder is unable to detect automatically the correct
	NavMesh triangle corresponding to the destination. This may be particularly useful when the IDetectPathNodeReached modifier
	is sensitive to small differences in altitude, such as when using the DetectPathNodeReached_3D class, or when using the
	DetectPathNodeReached_2D5 class with a small altitude threshold.
	\pp In order to take advantage of this class, you must retrieve the NavTriangleSafePtr that represents the NavMesh
	triangle for your destination position, and set this NavTriangleSafePtr in a PointInfo_NavTriangleSafePtr that you
	add to the PointWrapper for your destination before you pass that PointWrapper to PathFinder::FindNextMove().
	\ingroup kyruntime_pathfinding */
class RefineGoal_AdjustAltitude: public IRefineGoal
{
public:
	KY_DECLARE_REFINEGOAL(RefineGoal_AdjustAltitude)
public:
	/*! \label_constructor */
	RefineGoal_AdjustAltitude():
		IRefineGoal() {}

	virtual void RefineGoal(const PointWrapper& goal, PointWrapper& refinedGoal);

protected:
	virtual void OnSetPathFinder() {}

	void AdjustAltitude(PointWrapper& refinedGoal);
	void AdjustAltitude(NavMeshQueryServerWrapper& navMeshQueryServer, const NavTrianglePtr& navTrianglePtr, PointWrapper& refinedGoal);
};

} // namespace Kaim

#endif // KY_REFINEGOAL_ADJUSTALTITUDE_H
