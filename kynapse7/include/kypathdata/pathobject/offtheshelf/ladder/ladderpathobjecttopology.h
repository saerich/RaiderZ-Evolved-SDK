/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: None
#ifndef KYPATHDATA_LADDER_PATHOBJECT_TOPOLOGY_H
#define KYPATHDATA_LADDER_PATHOBJECT_TOPOLOGY_H

#include "kypathdata/pathobject/topology/dynamicpathobjecttopology.h"
#include "kypathdata/pathobject/offtheshelf/ladder/ladderpathobjectconnexionspace.h"
#include "kypathdata/pathobject/topology/controlspaces/dynamicpathobjectcontrolspace_box.h"

namespace Kaim
{

/*	LadderPathObjectTopology describes the topology of a basic ladder pathobject.
	It contains:
	-All accesses to the ladder
	-A control space describing the ladder space.
*/
class LadderPathObjectTopology : public DynamicPathObjectTopology
{
public:
	LadderPathObjectTopology(
		const Vec3f& entrance,
		const Vec3f& exit,
		const KyFloat32 height,
		const KyFloat32 halfWidth,
		const Vec3f& ladderBottom,
		const Vec3f& ladderTop,
		const Vec3f& ladderBack,
		const KyFloat32 ladderRadius);

	~LadderPathObjectTopology() {}

	/*!	Compute the projection of a position into the ladder.*/
	static void ComputeProjectionOnLadder(
		const Vec3f& position,
		const Vec3f& ladderBottom,
		const Vec3f& ladderTop,
		Vec3f& out_ProjectionOnLadder);

	/*! Compute the position (out_GrabPosition) where the entity can grab the ladder given the entity position.*/
	static void ComputeGrabPosition(
		const Vec3f& entityPosition,
		const Vec3f& ladderBottom,
		const Vec3f& ladderTop,
		const Vec3f& ladderBack,
		const KyFloat32 ladderRadius,
		Vec3f& out_GrabPosition);

protected:
	Kaim::LadderPathObjectConnexionSpace m_entrance;
	Kaim::LadderPathObjectConnexionSpace m_exit;
	Kaim::DynamicPathObjectControlSpace_Box m_ladderControlSpace;
};

} // namespace Kaim

#endif // KYPATHDATA_LADDER_PATHOBJECT_TOPOLOGY_H
