/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_COMPUTETARGETPOINT_TRIVIAL_H
#define KY_COMPUTETARGETPOINT_TRIVIAL_H

#include <kyruntime/pathfinding/icomputetargetpoint.h>


namespace Kaim
{
/*!	This implementation of the IComputeTargetPoint interface selects the current PathNode
	as the target point for the Bot, without attempting to smooth the trajectory.
	Although this class typically results in a more jagged trajectory as the Bot follows its 
	initial Path through the spatial graph very closely, it consumes negligible CPU resources
	at each frame.
	\ingroup kyruntime_pathfinding */
class ComputeTargetPoint_Trivial: public IComputeTargetPoint
{
public:
	KY_DECLARE_COMPUTETARGETPOINT(ComputeTargetPoint_Trivial)

public:
	/*!	\label_constructor */
	ComputeTargetPoint_Trivial(): IComputeTargetPoint() {}

	virtual KyBool ComputeTargetPoint(const PointWrapper& currentPathNodePosition, PointWrapper& targetPoint);

protected:
	virtual void OnSetPathFinder() {}
};

} // namespace Kaim

#endif // KY_COMPUTETARGETPOINT_TRIVIAL_H
