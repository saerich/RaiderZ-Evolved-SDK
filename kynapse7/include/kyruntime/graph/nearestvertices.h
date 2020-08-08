/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyRuntime_NearestVertex_H
#define KyRuntime_NearestVertex_H


#include "kypathdata/generic/memory.h"
#include "kypathdata/math/vec3f.h"
#include "kyruntime/graph/graphvertexptr.h"
#include "kyruntime/mesh/navmeshqueryserverwrapper.h"

namespace Kaim
{

class GraphManager;
class NavMeshQueryServer;
class NavTrianglePtr;

/* For internal use. */
class NearestVerticesQuery // class with static functions only
{
public:
	class NearestVertex
	{
		KY_DEFINE_NEW_DELETE_OPERATORS

	public:
		NearestVertex()
		{
			m_sqrDistance = 0.0f;
		}
		NearestVertex(const GraphVertexPtr& vertexPtr, Vec3f refPos, Vec3f vertexPos)
		{
			m_vertexPtr = vertexPtr;
			Vec3f vect = vertexPos - refPos;
			m_sqrDistance = vect.GetSquareLength();
		}

	public:
		GraphVertexPtr m_vertexPtr;
		KyFloat32 m_sqrDistance;
	};

	static KyUInt32 GetNearestAccessibleVertices(
		NavMeshQueryServerWrapper& navMeshQueryServer,
		const Vec3f& position,                            //< Position should be near the ground
		const GraphReference* currentGraphReference,
		KyUInt32 nbNearbyVertices,                  //< Number of nearvy vertices in nearbyVertices.
		GraphVertexPtr* nearbyVertices,                   //< Array filled with nbNearbyVertices nearby vertices (use GraphManger::FindNearbySortedVertexPtrs()).
		KyUInt32 nbMaxNearestAccessibleVertices,
		NearestVertex* nearestAccessibleVertices,
		KyFloat32 vertexHeightAboveGround);         //< This is used to offset the graph vertices positions to the ground.(usually around half entity height)

	static KyUInt32 GetNearestAccessibleVertices(
		NavMeshQueryServerWrapper& navMeshQueryServer,
		const Vec3f& position,                            //< Position should be near the ground
		const NavTrianglePtr& refTrianglePtr,
		KyUInt32 nbNearbyVertices,                  //< Number of nearvy vertices in nearbyVertices.
		GraphVertexPtr* nearbyVertices,                //< Array filled with nbNearbyVertices nearby vertices (use GraphManger::FindNearbySortedVertexPtrs())
		KyUInt32 nbMaxNearestAccessibleVertices,
		NearestVertex* nearestAccessibleVertices,
		KyFloat32 vertexHeightAboveGround);         //< This is used to offset the graph vertices positions to the ground.(usually around half entity height)
};

}

#endif // #ifndef KyRuntime_NearestVertex_H
