/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: BEHE - secondary contact: NONE

#ifndef KyPdg_DynamicPointGraphSimplifier_H
#define KyPdg_DynamicPointGraphSimplifier_H

#include "kypathdata/generic/basetypes.h"
#include "kypdg/pointgraph/dynamicpointgraphcell.h"

namespace Kaim
{
class ICollisionInterface;

/* This is a rework of the classical pdg simplification algorithm,
	modified to work with PdgPointGraphs, and with two limitations:
	 - assume all edges are one sided. No double-sided management at all.
	 - perform the simplification for flygraphs, that is simplification
		midpoints are not projected on the ground.
*/
class DynamicPointGraphSimplifier
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	DynamicPointGraphSimplifier(const ICollisionInterface& collisionInterface);
	~DynamicPointGraphSimplifier() {}

	/*! Simplify a given graph by removing edges when a 2-edges similar connection exists
		with a distance not greater than maxDistRatio times the removed edge length.
		WARNING! for one-sided edges only. */
	KyResult SimplifyByUnneededEdgeRemoval(PdgPointGraph& graph, KyFloat32 maxDistRatio = 1.5f);

	/*! Simplify a given graph by edge collapsing. WARNING! for one-sided edges only. */
	KyResult SimplifyByEdgeCollapse(PdgPointGraph& graph, KyFloat32 maxEdgeLength, KyFloat32 entityRadius);

private:
	bool AllowInEdgeSimplification(const PdgGraphEdge* testedEdge, const Vec3f& midPos, KyFloat32 sqDistEdgeMax);
	bool AllowOutEdgeSimplification(const PdgGraphEdge* testedEdge, const Vec3f& midPos, KyFloat32 sqDistEdgeMax);

	bool CanSimplify(PdgGraphEdge& edge, const Vec3f& midPos, KyFloat32 sqDistEdgeMax);

protected:
	const ICollisionInterface* m_collisionInterface;
	KyFloat32 m_entityRadius;
};

} // namespace Kaim

#endif // KyPdg_DynamicPointGraphSimplifier_H
