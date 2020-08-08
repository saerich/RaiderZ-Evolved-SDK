/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPathData_SeedGraphDataTypes_H
#define KyPathData_SeedGraphDataTypes_H



namespace Kaim
{

typedef KyUInt32 SeedGraphDataId;
typedef KyUInt32 SeedGraphEdgeDataIdx;
typedef KyUInt32 SeedGraphVertexDataIdx;

static const SeedGraphDataId INVALID_SEEDGRAPH_DATA_ID = KyUInt32MAXVAL;                    //< Invalid seedgraph id.
static const SeedGraphEdgeDataIdx INVALID_SEEDGRAPH_EDGE_DATA_IDX = KyUInt32MAXVAL;         //< Invalid seedgraph edge idx.
static const SeedGraphVertexDataIdx INVALID_SEEDGRAPH_VERTEX_DATA_IDX = KyUInt32MAXVAL;     //< Invalid seedgraph vertex idx.

}

#endif // #ifndef KyPathData_SeedGraphDataTypes_H
