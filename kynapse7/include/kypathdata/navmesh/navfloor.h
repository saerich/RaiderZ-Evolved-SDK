/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_navmesh */

// Primary contact: JUBA - secondary contact: LASI
#ifndef KyPathData_NavFloor_H
#define KyPathData_NavFloor_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/navmesh/navhalfedge.h"

#include "kypathdata/blob/blobarray.h"
#include "kypathdata/blob/endianness.h"
#include <kypathdata/basesystem/coordsystem.h>
#include "kypathdata/math/box2f.h"

namespace Kaim
{

/*!	Represents a link from one NavFloor to another.
	\ingroup kypathdata_navmesh */
struct FloorLink
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	NavHalfEdgeIdx m_halfEdgeIdx; /*!< The index of the triangle edge that borders the adjacent NavFloor. */
	NavFloorIdx m_floorIdx; /*!< The index of the neighboring NavFloor within the NavMesh. */
};
/*!	\label_swapendianness
	\ingroup kypathdata_navmesh */
KY_FORCE_INLINE void SwapEndianness(Endianness::Target e, FloorLink& self)
{
	SwapEndianness(e, self.m_halfEdgeIdx);
	SwapEndianness(e, self.m_floorIdx);
}

/*!	A NavFloor defines a contiguous area within a NavCell that is made up of connected, neighboring triangles
	that share the same terrain type.
	\pp Different \NavFloors within the same NavCell may represent separate floors in 3D that are not connected one
	to another. They may also represent areas with different terrain types, such as a sidewalk area and a road
	area, which may or may not be connected one to another.
	\ingroup kypathdata_navmesh */
class NavFloor
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor For internal use. */
	NavFloor() {}

	/*! Retrieves an axis-aligned bounding box that encloses the volume occupied by the NavFloor. */
	KY_FORCE_INLINE const Box3f& GetAABB() const { return m_aabbox; }

	// ---------- static functions ----------
	/*! Retrieves the index of the triangle that contains the specified edge. */
	KY_FORCE_INLINE static NavTriangleIdx HalfEdgeIdxToTriangleIdx(NavHalfEdgeIdx idx) { return idx / 3; }

	/*! Retrieves one edge from the specified triangle.
		\param idx				The index of the triangle whose edge is to be retrieved. 
		\param halfEdgeNumber	The index of the edge to retrieve within the triangle. May be 0, 1 or 2. */
	KY_FORCE_INLINE static NavHalfEdgeIdx TriangleIdxToHalfEdgeIdx(NavTriangleIdx idx, KyInt32 halfEdgeNumber) { return idx * 3 + halfEdgeNumber; }

	/*! Retrieves the triangle edge that follows the specified edge within its triangle. */
	KY_FORCE_INLINE static NavHalfEdgeIdx HalfEdgeIdxToNextHalfEdgeIdx(NavHalfEdgeIdx idx) { return idx - (idx % 3) + ((idx + 1) % 3); }

	/*! Retrieves the triangle edge that precedes the specified edge within its triangle. */
	KY_FORCE_INLINE static NavHalfEdgeIdx HalfEdgeIdxToPrevHalfEdgeIdx(NavHalfEdgeIdx idx) { return idx - (idx % 3) + ((idx + 2) % 3); }

	/*! Retrieves the index of the specified edge within its triangle.
		\return 0, 1 or 2.*/
	KY_FORCE_INLINE static KyUInt32 HalfEdgeIdxToHalfEdgeNumberInTriangle(NavHalfEdgeIdx idx) { return idx % 3; }

	// ---------- accessors ----------

	/*! Retrieves the number of triangle vertices maintained in this NavFloor. */
	KY_FORCE_INLINE KyUInt32 GetVertexCount()   const { return m_navVertices.m_count; }

	/*! Retrieves the number of triangle edges maintained in this NavFloor. */
	KY_FORCE_INLINE KyUInt32 GetHalfEdgeCount() const { return m_navHalfEdges.m_count; }

	/*! Retrieves the number of triangles maintained in this NavFloor. */
	KY_FORCE_INLINE KyUInt32 GetTriangleCount() const { return GetHalfEdgeCount() / 3; }

	/*! Retrieves the terrain type of all triangles in the NavFloor. */
	KY_FORCE_INLINE NavTerrainType GetNavTerrainType() const { return m_terrainType; }

	/*! Retrieves the triangle vertex with the specified index within this NavFloor. */
	KY_FORCE_INLINE const NavVertex& GetVertex(const NavVertexIdx idx) const;

	/*! Retrieves the triangle edge with the specified index within this NavFloor. */
	KY_FORCE_INLINE const NavHalfEdge& GetHalfEdge(NavHalfEdgeIdx idx) const;

	//KY_FORCE_INLINE const FloorLink& GetFloorLink(const NavHalfEdgePtr& id) const;
	/*! Retrieves a link to an adjacent NavFloor that borders the specified triangle edge, if any. */
	KY_FORCE_INLINE const FloorLink& GetFloorLink(NavHalfEdgeIdx idx) const;

	// ---------- NavHalfEdgeType ----------
	/*! Retrieves the type of the specified triangle edge. */
	KY_FORCE_INLINE NavHalfEdgeType GetHalfEdgeType(NavHalfEdgeIdx idx) const;

	// ---------- Edge to StartVertex ----------
	/*! Retrieves the vertex at the start of the specified triangle edge. */
	KY_FORCE_INLINE const NavVertex& HalfEdgeIdxToStartVertex(NavHalfEdgeIdx idx) const;

	/*! Retrieves the vertex at the start of the specified triangle edge. */
	KY_FORCE_INLINE NavVertexIdx HalfEdgeIdxToStartVertexIdx(NavHalfEdgeIdx idx) const;

	// ---------- Edge to EndVertex ----------
	/*! Retrieves the vertex at the end of the specified triangle edge. */
	KY_FORCE_INLINE const NavVertex& HalfEdgeIdxToEndVertex(NavHalfEdgeIdx idx) const;

	/*! Retrieves the vertex at the end of the specified triangle edge. */
	KY_FORCE_INLINE NavVertexIdx HalfEdgeIdxToEndVertexIdx(NavHalfEdgeIdx idx) const;

	// ---------- Edge to the third vertex of the triangle ----------
	/*! Retrieves the vertex in the triangle that is not connected to the specified triangle edge. */
	KY_FORCE_INLINE const NavVertex& HalfEdgeIdxToThirdVertex(NavHalfEdgeIdx idx) const;

	/*! Retrieves the vertex in the triangle that is not connected to the specified triangle edge. */
	KY_FORCE_INLINE NavVertexIdx HalfEdgeIdxToThirdVertexIdx(NavHalfEdgeIdx idx) const;

	// ---------- Edge to PairEdge ----------
	/*! Retrieves the triangle edge that is adjacent to the specified triangle edge, if any. */
	KY_FORCE_INLINE const NavHalfEdge& HalfEdgeIdxToPairHalfEdge(NavHalfEdgeIdx idx) const;

	/*! Retrieves the triangle edge that is adjacent to the specified triangle edge, if any. */
	KY_FORCE_INLINE NavHalfEdgeIdx HalfEdgeIdxToPairHalfEdgeIdx(NavHalfEdgeIdx idx) const;

	// ---------- Edge to Next Edge ----------
	/*! Retrieves the edge that follows the specified triangle edge within its triangle. */
	KY_FORCE_INLINE const NavHalfEdge& HalfEdgeIdxToNextHalfEdge(NavHalfEdgeIdx idx) const;

	// ---------- Edge to Prev Edge ----------
	/*! Retrieves the edge that precedes the specified triangle edge within its triangle. */
	KY_FORCE_INLINE const NavHalfEdge& HalfEdgeIdxToPrevHalfEdge(NavHalfEdgeIdx idx) const;

	// ---------- triangle to halfEdge ----------
	/*! Retrieves one edge from the specified triangle.
		\param idx				The index of the triangle whose edge is to be retrieved. 
		\param halfEdgeNumber	The index of the edge to retrieve within the triangle. May be 0, 1 or 2. */
	KY_FORCE_INLINE const NavHalfEdge& TriangleIdxToHalfEdge(NavTriangleIdx idx, const KyInt32 halfEdgeNumber) const;

	// ---------- triangle to Vertices ----------
	/*! Retrieves the vertices that make up the specified triangle.
		\param triangleIdx		The index of the triangle whose vertices are to be retrieved.
		\param[out] v0Idx		The index of the first vertex in the triangle.
		\param[out] v1Idx		The index of the second vertex in the triangle.
		\param[out] v2Idx		The index of the third vertex in the triangle. */
	KY_FORCE_INLINE void TriangleIdxToVertexIndices(NavTriangleIdx triangleIdx, NavVertexIdx& v0Idx, NavVertexIdx& v1Idx, NavVertexIdx& v2Idx) const;

	// ---------- triangle to vertex ----------
	/*! Retrieves one vertex from the specified triangle.
		\param idx				The index of the triangle whose edge is to be retrieved. 
		\param vertexNumber		The index of the vertex to retrieve within the triangle. May be 0, 1 or 2. */
	KY_FORCE_INLINE const NavVertex& TriangleIdxToVertex(NavTriangleIdx idx, const KyInt32 vertexNumber) const;

	/*! Retrieves one vertex from the specified triangle.
		\param idx				The index of the triangle whose edge is to be retrieved. 
		\param vertexNumber		The index of the vertex to retrieve within the triangle. May be 0, 1 or 2. */
	KY_FORCE_INLINE NavVertexIdx TriangleIdxToVertexIdx(NavTriangleIdx idx, const KyInt32 vertexNumber) const;

	/*! Indicates whether or not this object represents a valid NavFloor: i.e. one made up of valid triangles. */
	bool IsValid() const;

	/*! Indicates whether or not the specified NavTriangleIdx is valid. */
	bool IsTriangleValid(NavTriangleIdx idx) const;

public:
	Box3f m_aabbox; /*!< The axis-aligned bounding box that encloses the NavFloor. Do not modify. */
	NavTerrainType m_terrainType; /*!< The terrain type of the NavFloor. Do not modify. */

	BlobArray<NavVertex> m_navVertices; /*!< The array of triangle vertices maintained by the NavFloor. Do not modify. */
	BlobArray<NavHalfEdge> m_navHalfEdges; /*!< The array of triangle edges maintained by the NavFloor. Do not modify. */
	BlobArray<FloorLink> m_floorLinks; /*!< The links to other NavFloors. For internal use. Do not modify. */

private:
	NavFloor& operator=(const NavFloor& navFloor);
	NavFloor(const NavFloor& navFloor);
};

/*! \label_swapendianness
	\ingroup kypathdata_navmesh */
KY_FORCE_INLINE void SwapEndianness(Endianness::Target e, NavFloor& self)
{
	SwapEndianness(e, self.m_aabbox);
	SwapEndianness(e, self.m_navVertices);
	SwapEndianness(e, self.m_navHalfEdges);
	SwapEndianness(e, self.m_floorLinks);
	SwapEndianness(e, self.m_terrainType);

}

#include "kypathdata/navmesh/navfloor.inl"

}


#endif //KyPathData_NavFloor_H


