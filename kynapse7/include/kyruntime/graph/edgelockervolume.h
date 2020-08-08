/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: NOBODY
#ifndef KY_EDGE_LOCKER_VOLUME_H
#define KY_EDGE_LOCKER_VOLUME_H

#include <kypathdata/containers/list.h>
#include "kyruntime/graph/graphedgeptr.h"
#include "kypathdata/navmesh/navmeshtypes.h"

namespace Kaim
{

class EdgeLocker;
class IVolume;
class GraphManager;

///////////////////////////////////////////////////////////////////////////////////////////
// EdgeLockerVolume
///////////////////////////////////////////////////////////////////////////////////////////

/*	Used internally by the EdgeLocker to represent areas of 3D space in which it should lock edges. */
class EdgeLockerVolume
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	EdgeLockerVolume() : m_volume(KY_NULL) {}
	EdgeLockerVolume(IVolume* volume) { m_volume = volume; }
	~EdgeLockerVolume(); 

	void lockAllEdgesOfStitchedGraphCell(StitchedGraphCell* stitchedGraphCell);
	void lockAllStaticEdgesOfStitchedGraphCell(StitchedGraphCell* stitchedGraphCell);
	void UnlockStaticEdgesofStitchedGraphCell(StitchedGraphCell* stitchedGraphCell);

	void AddVirtualEdge(VirtualEdgeIdx virtualEdgeIdx);
	void AddVirtualEdgeIfInVolume(const Vec3f& startPos, const Vec3f& endPos, VirtualEdge& virtualEdge, VirtualEdgeIdx virtualEdgeIdx);

	void RemoveVirtualEdgeIfPresent(VirtualEdge& virtualEdge, VirtualEdgeIdx virtualEdgeIdx);

public:
	const IVolume* m_volume;                            /*!< Interface describing the volume. */
	CellBox m_cellBox;
	SharedPoolList<VirtualEdgeIdx> m_lockedVirtualEdgeIndices;
	SharedPoolList<GraphEdgePtr> m_lockedStaticEdgePtrs;
};

} // namespace Kaim

#endif
