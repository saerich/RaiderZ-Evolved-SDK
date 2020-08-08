/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyRuntime_ConnectVertex_H
#define KyRuntime_ConnectVertex_H

#include "kypathdata/generic/types.h"
#include "kyruntime/graph/graphvertexptr.h"
#include "kyruntime/graph/nearestvertices.h"
#include "kyruntime/graph/nearestedges.h"
#include "kyruntime/mesh/navmeshqueryserverwrapper.h"
#include "kypathdata/math/vec3f.h"

namespace Kaim
{

class Vec3f;
class GraphManager;
class NavMeshManager;
class GraphVertexPtr;

/* For internal use by the GraphManager. */
class ConnectVertex // class with static functions only
{
public:
	/*
	Fills a list of vertex indices in the input graph that 
	could be connected to the input position.
	Note: This function does not test bidirectional accessibility (only test from inputPosition to candidates).
	\param inputPosition The position you want to connect.
	\param graphManager The GraphManager you want to connect to.
	\param navMeshManager The NavMesh manager with which you test connections.
	\param candidatesMaxSize The maximum number of candidates that can be stored in the candidate output list.
	\param nbCandidates The number of candidates stored in "candidates".
	\param candidates The best candidates given the "candidatesMaxSize".
	*/

	static KyUInt32 GetNearbyAccessibleVertices(
		const Vec3f& startPos,
		const GraphReference* startGraphReference,
		NavMeshQueryServerWrapper& navMeshQueryServer,
		const KyUInt32 nbNearbyVertices,
		const GraphVertexPtr* nearbyVertices,
		KyUInt32 nbMaxCandidates,
		GraphVertexPtr* candidates);

	static KyUInt32 GetConnectionCandidates(
		const Vec3f& startPos,
		const GraphReference* startGraphReference,
		NavMeshQueryServerWrapper& navMeshQueryServer,
		const KyUInt32 nbNearbyVertices,
		const GraphVertexPtr* nearbyVertices,
		KyUInt32 nbMaxCandidates,
		GraphVertexPtr* candidates);

	static bool IntersectNearbyEdges(
		const GraphVertexPtr& startVertex,
		const Vec3f& startPos,
		const NavTrianglePtr& startTrianglePtr,
		const GraphVertexPtr& endVertex,
		const Vec3f& endPos,
		const NavTrianglePtr& endTrianglePtr,
		const KyUInt32 endNearbyVerticesIdx,
		const KyUInt32 nbNearbyVertices,
		const GraphVertexPtr* nearbyVertices,
		const KyUInt8* nearbyVerticesConnectionFlags,
		NavMeshQueryServerWrapper& navMeshQueryServer);

	/*	Intersect nearest edges from pos.*/
	static bool IntersectNearbyEdges(
		const Vec3f& startPos,
		const Vec3f& endPos,
		const KyUInt32 endNearbyVerticesIdx,
		const KyUInt32 nbNearbyVertices,
		const GraphVertexPtr* nearbyVertices,
		NavMeshQueryServerWrapper& navMeshQueryServer);

	static KyResult GetConnectionCandidatesOld(
		const Vec3f& inputPosition,
		KyFloat32 vertexHeightAboveGround,
		const GraphReference* currentGraphReference,
		NavMeshQueryServerWrapper& navMeshQueryServer,
		const KyUInt32 maxNearestAccessibleVertices,
		NearestVerticesQuery::NearestVertex* nearestAccessibleVertices,
		const KyUInt32 maxNearestEdges,
		NearestEdges::NearestEdge* nearestEdges,
		const KyUInt32 nbNearbyVertices,
		GraphVertexPtr* nearbyVertices,
		KyUInt32 nbMaxCandidates,
		GraphVertexPtr* candidates,
		KyUInt32& nbCandidates);

	static void ComputeCandidatesNotIntersectingEdges(
		const Vec3f& inputPosition,
		KyUInt32 nbNearestAccessibleVertices,
		NearestVerticesQuery::NearestVertex* nearestVertices,
		KyUInt32 nbNearestAccessibleEdges,
		NearestEdges::NearestEdge* nearestEdges,
		KyUInt32 nbMaxCandidates,
		KyUInt32& nbCandidates,
		GraphVertexPtr* candidates);

private:
	KY_FORCE_INLINE static bool AreVerticesEquals(const Vec3f& vertex1, const Vec3f& vertex2)
	{
		return vertex1.x == vertex2.x && vertex1.y == vertex2.y && fabs(vertex1.z - vertex2.z) < 0.5f;
	}
};

} // namespace Kaim

#endif //#ifndef KyPathData_ConnectVertex_H
