/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/



// primary contact: MAMU - secondary contact: NOBODY
#ifndef KyRuntime_AdditionalGraphStitcher_H
#define KyRuntime_AdditionalGraphStitcher_H

#include "kypathdata/basesystem/basesystem.h"
#include "kypathdata/graph/graphtypes.h"
#include "kypathdata/generic/basetypes.h"
#include "kypathdata/containers/list.h"

#include "kyruntime/graph/virtualedge.h"

#include "kyruntime/graph/nearestedges.h"
#include "kyruntime/graph/nearestvertices.h"
#include "kypathdata/navmesh/nearesttrianglefromposquery.h"

namespace Kaim
{

class GraphEdgePtr;
class GraphVertexPtr;
class GraphVertexSafePtr;
class GraphManager;
class GraphCellGrid;
class StitchedGraph;
class NavMeshQueryServerWrapper;
class AdditionalGraph;

///////////////////////////////////////////////////////////////////////////////////////////
// AdditionalGraphStitcher
///////////////////////////////////////////////////////////////////////////////////////////

/* Used internally by the GraphManager to stitch Graphs together at runtime. */
class AdditionalGraphStitcher
{
	KY_DEFINE_NEW_DELETE_OPERATORS

	AdditionalGraphStitcher& operator=(const AdditionalGraphStitcher&);

friend class GraphCellGrid;
friend class GraphManager;

	AdditionalGraphStitcher();
	KY_FORCE_INLINE void SetNavMeshQueryServer(NavMeshQueryServerWrapper& queryServer) { m_navMeshQueryServer = &queryServer; }
	KY_FORCE_INLINE void SetGraphCellGrid(GraphCellGrid& graphCellGrid) { m_graphCellGrid = &graphCellGrid; }
	void SetGraphManager(GraphManager& graphManager);
public:

	~AdditionalGraphStitcher();
	void Clear();

	KY_FORCE_INLINE KyUInt32 GetVirtualEdgeCount() const { return m_virtualEdges.GetCount(); }
	KY_FORCE_INLINE KyUInt32 GetUsedVirtualEdgeCount() const { return m_virtualEdges.GetCount() - m_freeVirtualEdgeIndices.GetCount(); }
	KY_FORCE_INLINE GraphManager& GetGraphManager() const { return *m_graphManager; }

	// LINK
	// we call this function when we add a PO
	KyResult LinkAdditionalGraphVertex(const GraphVertexPtr& vertexToConnect, const AdditionalGraph& additionalGraph, const CellPos& cellPosOfVertex);
	KyResult LinkAdditionalGraphVertex(const GraphVertexPtr& vertexToConnect);
	KyResult UnLinkAdditionalGraphVertex(const GraphVertexPtr& vertexptr);

	// ACCESS VIRTUAL EDGES
	// return a reference the the virtual edge
	KY_FORCE_INLINE VirtualEdge& GetVirtualEdge(VirtualEdgeIdx virtualEdgeIdx)
	{
		KY_FUNCTION("AdditionalGraphStitcher::GetVirtualEdge");
		KY_ASSERTK(virtualEdgeIdx < m_virtualEdges.GetCount());

		return m_virtualEdges[virtualEdgeIdx];
	}

	KY_FORCE_INLINE const VirtualEdge& GetVirtualEdge(VirtualEdgeIdx virtualEdgeIdx) const
	{
		KY_FUNCTION("AdditionalGraphStitcher::GetVirtualEdge");
		KY_ASSERTK(virtualEdgeIdx < m_virtualEdges.GetCount());

		return m_virtualEdges[virtualEdgeIdx];
	}

	void RenderConnections() const;

private:
	KyResult LinkAdditionalGraphVertex_InputVertex(const GraphVertexPtr& vertexToConnect, const CellPos& cellPosOfVertex);
	KyResult LinkAdditionalGraphVertex_GraphVertex(const GraphVertexPtr& vertexToConnect, const CellPos& cellPosOfVertex);
	KyResult LinkAdditionalGraphVertex_RuntimeStitch(const GraphVertexPtr& vertexToConnect);

	KyResult RemoveAllLinks(StitchedGraph& stitchedGraph);
	KyResult RemoveAllLinksAndUpdateNeighBourPo(StitchedGraph& stitchedGraph);

	// return the number of VirtualEdges successfully created (0, 1 or 2)
	KyUInt32 CreateLinksBetweenVertices(const GraphVertexPtr& vertexToConnect, const GraphVertexPtr& targetVertex, const CellPos& targetCellPos);

	VirtualEdgeIdx CreateVirtualEdge(const GraphVertexPtr& start, const GraphVertexPtr& end, StitchedGraph* virtualStitchedGraph);
	void ReleaseVirtualEdge(VirtualEdgeIdx virtualEdgeIdx);

	// get a free virtual edge (that is not in use), and set returnEdgeIndex to the of the this virtual edge
	VirtualEdge* GetFreeVirtualEdge(VirtualEdgeIdx& returnEdgeIndex);

	KyResult RemoveVirtualEdge(VirtualEdgeIdx virtualEdgeIdx);
	SharedPoolList<VirtualEdgeIdx>::Iterator RemoveOutVirtualEdge(VirtualEdgeIdx virtualEdgeIdx, VertexData& startVertexData, const SharedPoolList<VirtualEdgeIdx>::Iterator& it);
	SharedPoolList<VirtualEdgeIdx>::Iterator RemoveInVirtualEdge(VirtualEdgeIdx virtualEdgeIdx, VertexData& endVertexData, const SharedPoolList<VirtualEdgeIdx>::Iterator& it);

	KyUInt32 LinkToNearbyVertices(const GraphVertexPtr& startConnectionPtr, const Vec3f& startVertexPos, const NavTrianglePtr& startTrianglePtr,
		KyUInt32 nbNearbyVertices, const GraphVertexPtr* nearbyVertices);

	bool CanConnectTwoVertices(const GraphVertexPtr& startVertex, const Vec3f& startVertexPos, const NavTrianglePtr& startTrianglePtr,
		const GraphVertexPtr& endvertex, const Vec3f& endVertexPos,
		KyUInt32 nbNearbyVertices, const GraphVertexPtr* nearbyVertices, KyUInt32 nearbyVertexIdx);

public:
	Array<VirtualEdge> m_virtualEdges;

private:
	List<VirtualEdgeIdx> m_freeVirtualEdgeIndices;

	// Pool of virtual edge indices linking a graph to another one
	// each stitched graph stores its link indices in a SharedPoolList targeting this pool
	SharedPoolList<VirtualEdgeIdx>::Pool m_PoolOfVirtualEdgeIndices;
	List<VirtualEdgesIndicesForVertex> m_virtualEdgesIndicesForVertexList;

public:
	NavMeshQueryServerWrapper* m_navMeshQueryServer;
	GraphCellGrid* m_graphCellGrid;
	GraphManager* m_graphManager;
private:
	// used to link graphs
	KyUInt32 m_maxNearbyVerticesCount;
	GraphVertexPtr* m_nearbyVertices;
	KyUInt8* m_nearbyVerticesConnectionState; //!< At index i, if 0, then the nearby vertex from m_nearbyVertices at index i was not yet connected to the current vertex. Else 1.
};

} // namespace Kaim

#endif // KyRuntime_AdditionalGraphStitcher_H
