/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: NOBODY
#ifndef KYRUNTIME_EDGELOCKER_H
#define KYRUNTIME_EDGELOCKER_H

#include "kypathdata/basesystem/basesystem.h"
#include "kypathdata/math/box2f.h"
#include "kyruntime/graph/graphedgeptr.h"
#include "kyruntime/graph/graphvertexptr.h"


#include <kypathdata/containers/list.h>

namespace Kaim
{

class IPathObjectControlSpace;
class IVolume;
class EdgeLockerVolume;
class GraphCellGrid;
class StitchedGraphCell;

///////////////////////////////////////////////////////////////////////////////////////////
// EdgeLocker
///////////////////////////////////////////////////////////////////////////////////////////

/*	This class is used internally in order to dynamically lock edges of a static graph.
	This component is per world and per database.
	Its responsibilities are:
	-Store the pool of locked edges for a given world and a given database.
	-Compute edges intersecting a given volume.
	-Add locked edges intersecting a PathObject control space to the PathObject.
	-Answer to the question "Is this edge is locked?" that an external component could ask
	(eg: PathFinder, A*).
*/
class EdgeLocker
{
	KY_DEFINE_NEW_DELETE_OPERATORS

	EdgeLocker& operator=(const EdgeLocker&);

friend class GraphCellGrid;
	EdgeLocker();
	KY_FORCE_INLINE void SetGraphManager(GraphManager& graphManager) { m_graphManager = &graphManager; }
	KY_FORCE_INLINE void SetGraphCelGrid(GraphCellGrid& graphCellGrid) { m_graphCellGrid = &graphCellGrid; }

public:

	~EdgeLocker();
	void Clear();

	/////////////////////////
	// REGISTER
	/////////////////////////

	List<EdgeLockerVolume>::Iterator AddEdgeLockerVolume(const IVolume* volume);
	void RemoveEdgeLockerVolume(List<EdgeLockerVolume>::Iterator& edgeLockerVolumeIterator);
	void ClearEdgeLockerVolume(EdgeLockerVolume& edgeLockerVolume);

	/////////////////////////
	// RENDER
	/////////////////////////

	/*!	Render locked edges */
	void RenderLockedEdges() const;

private:
	void RenderEdge(const Vec3f& start, const Vec3f& end, KyUInt32 r, KyUInt32 g, KyUInt32 b) const;

	/////////////////////////
	// LOCK
	/////////////////////////

	void LockEdgesOfEdgeLockerVolume(EdgeLockerVolume& edgeLockerVolume);

	/////////////////////////
	// UNLOCK
	/////////////////////////

	/*	Unlock edges in the specified pool list.*/
	void UnlockAllEdgesOfEdgeLockerVolumes(EdgeLockerVolume& edgeLockerVolume);



public:
	SharedPoolList<GraphEdgePtr>::Pool m_lockedSaticEdgePtrs; //<! Pool of locked edges.
	SharedPoolList<VirtualEdgeIdx>::Pool m_lockedVirtualEdgeIndices; //<! Pool of locked edges.

	List<EdgeLockerVolume> m_edgeLockerVolumes;

	GraphManager* m_graphManager;
	GraphCellGrid* m_graphCellGrid;
};

} // namespace Kaim

#endif // KYRUNTIME_EDGELOCKER_H
