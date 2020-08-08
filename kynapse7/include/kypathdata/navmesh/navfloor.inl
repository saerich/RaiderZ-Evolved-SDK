/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_navmesh */

/*!	\file navfloor.inl */

// Primary contact: JUBA - secondary contact: NOBODY

KY_FORCE_INLINE const NavVertex& NavFloor::GetVertex(const NavVertexIdx idx) const
{
	KY_FUNCTION("NavFloor::GetVertex");
	KY_ASSERT(idx < GetVertexCount(), ("Invalid vertex index"));

	return m_navVertices.Values()[idx];
}

KY_FORCE_INLINE const NavHalfEdge& NavFloor::GetHalfEdge(NavHalfEdgeIdx idx) const
{
	KY_FUNCTION("NavFloor::GetHalfEdge");
	KY_ASSERT(idx < GetHalfEdgeCount(), ("Invalid half-edge index"));

	return m_navHalfEdges.Values()[idx];
}

KY_FORCE_INLINE const FloorLink& NavFloor::GetFloorLink(NavHalfEdgeIdx idx) const
{
	KY_FUNCTION("NavFloor::GetFloorLink");
	KY_ASSERT(GetHalfEdge(idx).GetFloorLinkIdx() < m_floorLinks.m_count, ("Invalid FloorLink index"));

	return m_floorLinks.Values()[GetHalfEdge(idx).GetFloorLinkIdx()];
}

// ---------- NavHalfEdgeType ----------
KY_FORCE_INLINE NavHalfEdgeType NavFloor::GetHalfEdgeType(NavHalfEdgeIdx idx) const { return GetHalfEdge(idx).GetHalfEdgeType(); }

// ---------- Edge to StartVertex ----------
KY_FORCE_INLINE const NavVertex& NavFloor::HalfEdgeIdxToStartVertex(NavHalfEdgeIdx idx) const { return GetVertex(HalfEdgeIdxToStartVertexIdx(idx)); }
KY_FORCE_INLINE NavVertexIdx NavFloor::HalfEdgeIdxToStartVertexIdx(NavHalfEdgeIdx idx) const { return GetHalfEdge(idx).GetStartVertexIdx(); }

// ---------- Edge to EndVertex ----------
KY_FORCE_INLINE const NavVertex& NavFloor::HalfEdgeIdxToEndVertex(NavHalfEdgeIdx idx) const { return GetVertex(HalfEdgeIdxToEndVertexIdx(idx)); }
KY_FORCE_INLINE NavVertexIdx NavFloor::HalfEdgeIdxToEndVertexIdx(NavHalfEdgeIdx idx) const { return HalfEdgeIdxToStartVertexIdx(HalfEdgeIdxToNextHalfEdgeIdx(idx)); }

// ---------- Edge to the third vertex of the triangle ----------
KY_FORCE_INLINE const NavVertex& NavFloor::HalfEdgeIdxToThirdVertex(NavHalfEdgeIdx idx) const { return GetVertex(HalfEdgeIdxToThirdVertexIdx(idx)); }
KY_FORCE_INLINE NavVertexIdx NavFloor::HalfEdgeIdxToThirdVertexIdx(NavHalfEdgeIdx idx) const { return HalfEdgeIdxToStartVertexIdx(HalfEdgeIdxToPrevHalfEdgeIdx(idx)); }

// ---------- Edge to PairEdge ----------
KY_FORCE_INLINE const NavHalfEdge& NavFloor::HalfEdgeIdxToPairHalfEdge(NavHalfEdgeIdx idx) const { return GetHalfEdge(HalfEdgeIdxToPairHalfEdgeIdx(idx)); }
KY_FORCE_INLINE NavHalfEdgeIdx NavFloor::HalfEdgeIdxToPairHalfEdgeIdx(NavHalfEdgeIdx idx) const { return GetHalfEdge(idx).GetPairHalfEdgeIdx(); }

// ---------- Edge to Next Edge ----------
KY_FORCE_INLINE const NavHalfEdge& NavFloor::HalfEdgeIdxToNextHalfEdge(NavHalfEdgeIdx idx) const { return GetHalfEdge(HalfEdgeIdxToNextHalfEdgeIdx(idx)); }

// ---------- Edge to Prev Edge ----------
KY_FORCE_INLINE const NavHalfEdge& NavFloor::HalfEdgeIdxToPrevHalfEdge(NavHalfEdgeIdx idx) const { return GetHalfEdge(HalfEdgeIdxToPrevHalfEdgeIdx(idx)); }

// ---------- triangle to halfEdge ----------
KY_FORCE_INLINE const NavHalfEdge& NavFloor::TriangleIdxToHalfEdge(NavTriangleIdx idx, const KyInt32 halfEdgeNumber) const { return GetHalfEdge(TriangleIdxToHalfEdgeIdx(idx, halfEdgeNumber)); }

// ---------- triangle to Vertices ----------
KY_FORCE_INLINE void NavFloor::TriangleIdxToVertexIndices(NavTriangleIdx triangleIdx, NavVertexIdx& v0Idx, NavVertexIdx& v1Idx, NavVertexIdx& v2Idx) const
{
	KY_FUNCTION("NavFloor::TriangleIdxToVertexIndices");
	KY_ASSERT(triangleIdx < GetTriangleCount(), ("Invalid triangle index"));

	const NavHalfEdgeIdx firstNavHalfEdgeIdx = TriangleIdxToHalfEdgeIdx(triangleIdx, 0);
	v0Idx = HalfEdgeIdxToStartVertexIdx(firstNavHalfEdgeIdx);
	v1Idx = HalfEdgeIdxToStartVertexIdx(firstNavHalfEdgeIdx + 1);
	v2Idx = HalfEdgeIdxToStartVertexIdx(firstNavHalfEdgeIdx + 2);
}

// ---------- triangle to vertex ----------
KY_FORCE_INLINE const NavVertex& NavFloor::TriangleIdxToVertex(NavTriangleIdx idx, const KyInt32 vertexNumber) const { return GetVertex(TriangleIdxToVertexIdx(idx, vertexNumber)); }
KY_FORCE_INLINE NavVertexIdx NavFloor::TriangleIdxToVertexIdx(NavTriangleIdx idx, const KyInt32 vertexNumber) const
{
	KY_FUNCTION("NavFloor::TriangleIdxToVertexIdx");
	KY_ASSERT(idx < GetTriangleCount(), ("Invalid triangle index"));
	KY_ASSERT(vertexNumber > -1 && vertexNumber < 3, ("Invalid vertex number in triangle"));

	return HalfEdgeIdxToStartVertexIdx(idx * 3 + vertexNumber);
}

