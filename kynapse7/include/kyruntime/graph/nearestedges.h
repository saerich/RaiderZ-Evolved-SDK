/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyRuntime_NearestEdge_H
#define KyRuntime_NearestEdge_H


#include "kypathdata/generic/memory.h"
#include "kyruntime/graph/graphedgeptr.h"
#include "kyruntime/graph/graphvertexptr.h"

namespace Kaim
{

class NavMeshQueryServer;
class Vec3f;
class GraphManager;

class NearestEdges // class with static functions only
{
public:

	/* Class containing data used internally to compare nearest edges.
	The distance criteria is:
	-The ortho distance to the edge when the refPos can be orthogonally projected on the edge.
	-The distance to the closer of the start and end vertex.
	*/
	class NearestEdge
	{
		KY_DEFINE_NEW_DELETE_OPERATORS

	public:
		NearestEdge()
		{
			m_sqrDistance = 0.0f;
		}
		NearestEdge(const GraphEdgePtr& edgePtr, Vec3f refPos, Vec3f vertexPosA, Vec3f vertexPosB);

	public:
		GraphEdgePtr m_edgePtr; //< Id of the edge
		KyFloat32 m_sqrDistance; //< Distance from refPos to edge.
	};

	/*
	Fills a list of edges from the input graph that 
	are accessible from the input position.
	\param graphManager The GraphManager from which edges are extracted.
	\param queryServer The query server with which navmesh requests will be executed.
	\param inputPosition The position you want to connect.
	\param nbMaxEdges The maximum number of candidates that can be stored in the candidate output list.
	\param nearestEdges The output best candidates given the "candidatesMaxSize".
	\return The number of candidates stored in "nearestEdges".
	*/
	static KyUInt32 GetNearestAccessibleEdges(
		NavMeshQueryServer& queryServer,
		const Vec3f& inputPosition,
		const KyUInt32 nbNearestVertices,
		GraphVertexPtr* nearbyVertices,
		const KyUInt32 nbMaxEdges,
		NearestEdge* nearestEdges,
		KyFloat32 vertexHeightAboveGround);

};

}

#endif // #ifndef KyRuntime_NearestEdge_H
