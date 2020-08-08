/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/




#ifndef KyPdg_PdgGraphVertex_H
#define KyPdg_PdgGraphVertex_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include <kypathdata/generic/memory.h>
#include <kypathdata/math/vec3f.h>
#include <kypathdata/math/cardinaldir.h>
#include <kypathdata/containers/multiset.h>
#include <kypathdata/containers/array.h>

namespace Kaim
{
class DynamicNavHalfEdge;
class EdgeToCover;
class PdgGraphEdge;
class PdgPointGraph;

/*
Contains all basic data of a graph vertex, namely:
-The unique ID of the vertex.
-The position of the vertex.
-The indices of the outgoing edges.
-The indices of the incoming edges.
*/
class PdgGraphVertex
{
	KY_DEFINE_NEW_DELETE_OPERATORS

	friend class PdgPointGraph;
public:
	typedef EdgeToCover** EdgeToCoverPtrIterator;
	typedef EdgeToCover* const* EdgeToCoverPtrConstIterator;

	enum Flags
	{
		CellBoundary = 1 << 0,
		FloorBoundary = 1 << 1,
		Fixed = 1 << 2, // can't be simplified / moved
		Found = 1 << 3, // used during mark/search phases
		ToBeRemoved = 1 << 4, // For lazy remove.
		Dummy_Flag = KYFORCEENUMSIZEINT
	};

public:
	PdgGraphVertex(MultiSet<PdgGraphEdge*>::PoolType& pool, const Vec3f& position, KyUInt32 terrainType = 0);
	explicit PdgGraphVertex(MultiSet<PdgGraphEdge*>::PoolType& pool);

	~PdgGraphVertex();

	void Clear();

	void SetAsCellBoundaryVertex(CardinalDir cellBoundaryCardinalDir, const PdgPointGraph& graph);
	inline bool IsCellBoundaryVertex() const { return IsFlagSet(CellBoundary); }

	void SetAsFloorBoundary() { SetFlag(FloorBoundary); }
	bool IsFloorBoundary() const { return IsFlagSet(FloorBoundary); }

	inline void SetAsFixedVertex() { SetFlag(Fixed); }
	inline bool IsFixedVertex() const { return IsFlagSet(Fixed); }

	inline void SetFound(bool enable) { if (enable) SetFlag(Found); else UnsetFlag(Found); }
	inline bool IsFound() const { return IsFlagSet(Found); }

	void SetRemoved() { SetFlag(ToBeRemoved); }
	bool IsRemoved() const { return IsFlagSet(ToBeRemoved); }

	inline const Vec3f& GetPosition() const { return m_position; }

	bool IsConnectedTo(const PdgGraphVertex& other) const;

	/* Returns the sum of the shortest two-edges road from this vertex to another.
		KyFloat32MAXVAL if no two-connections found. One-connection not checked. */
	KyFloat32 FindShortestTwoConnectionsTo(const PdgGraphVertex& other) const; 

	void AddSeenEdgeToCover(EdgeToCover& obs) { m_coverageEdges.PushBack(&obs); }
	void ClearCoverage() { m_coverageEdges.Clear(); }

	EdgeToCoverPtrConstIterator EdgeToCoverBegin() const { return m_coverageEdges.Begin(); }
	EdgeToCoverPtrIterator EdgeToCoverBegin() { return m_coverageEdges.Begin(); }
	EdgeToCoverPtrConstIterator EdgeToCoverEnd() const { return m_coverageEdges.End(); }
	EdgeToCoverPtrIterator EdgeToCoverEnd() { return m_coverageEdges.End(); }

	void AddNavFloor(NavFloorIdx navFloorIdx, const DynamicNavHalfEdge* halfEdge = KY_NULL);
	NavFloorIdx GetLastNavFloorIdx() const;
	bool IsInNavFloor(NavFloorIdx navFloorIdx) const;
	bool IsExactlyInSameNavFloorAs(const PdgGraphVertex& rhs) const;
	const DynamicNavHalfEdge* GetDynamicHalfEdge(NavFloorIdx navFloorIdx) const;

protected:
	inline void SetFlag(Flags flag) { m_flags |= flag; }
	inline void UnsetFlag(Flags flag) { m_flags &= (0xFFFFFFFF ^ flag); }
	inline bool IsFlagSet(Flags flag) const { return (m_flags & flag) != 0; }

	void CopyAllButCoverageAndEdges(const PdgGraphVertex& rhs);

public:
	class NavFloorAndHalfEdge;

protected:
	typedef Array<NavFloorAndHalfEdge>::Iterator NavFloorAndHalfEdgeIterator;
	typedef Array<NavFloorAndHalfEdge>::ConstIterator NavFloorAndHalfEdgeConstIterator;

public:
	Vec3f m_position; //< Position of the vertex.
	KyUInt32 m_terrainType;

	MultiSet<PdgGraphEdge*> m_outgoingEdges;   //< outgoing edges.
	MultiSet<PdgGraphEdge*> m_incomingEdges;   //< incoming egdes.

	CardinalDir m_boundaryCardinalDir; // is CardinalDir_INVALID if interior vertex

	KyUInt32 m_index;

private:
	Array<NavFloorAndHalfEdge> m_floorIdxs;
	KyUInt32 m_flags;
	Array<EdgeToCover*> m_coverageEdges;
};

}

#endif // #ifndef KyPdg_PdgGraphVertex_H
