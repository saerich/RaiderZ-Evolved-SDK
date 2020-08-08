#ifndef PX_CONVEXMESH_H
#define PX_CONVEXMESH_H

#include "px_config.h"
#include "Opcode.h"

struct PxEdgeListData;
struct PxCollisionHullData;
struct PxSupportVertexMapData;

//Data
#pragma PXD_PUSH_PACK(1)

typedef struct PxConvexMeshData_
{
	PxEdgeListData*				edgeListData;
// PXD_SUPPORT_CONVEX_OPCODE_MODEL
	Opcode::HybridModel			mOpcodeModel;
//~PXD_SUPPORT_CONVEX_OPCODE_MODEL
	PxCollisionHullData*		hullData;
	PxSupportVertexMapData*		vertexMapData;
	PxdVector					aabbMin, aabbMax; //local bounding box
	PxFloat						geomEpsilon;
} PxConvexMeshData;

#pragma PXD_POP_PACK

#endif
