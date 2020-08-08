/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPdg_DynamicPointGraphTypes_H
#define KyPdg_DynamicPointGraphTypes_H



namespace Kaim
{

typedef KyUInt32 DynamicGraphVertexIdx;
typedef KyUInt32 DynamicGraphEdgeIdx;
typedef KyUInt32 DynamicGraphVertexDataIdx;
typedef KyUInt32 DynamicGraphEdgeDataIdx;
typedef KyUInt32 DynamicPointGraphId;

static const DynamicGraphVertexIdx INVALID_DYNAMICGRAPHVERTEX_IDX = KyUInt32MAXVAL;           //< Invalid vertex idx.
static const DynamicGraphEdgeIdx INVALID_DYNAMICGRAPHEDGE_IDX = KyUInt32MAXVAL;               //< Invalid edge idx.
static const DynamicGraphVertexDataIdx INVALID_DYNAMICGRAPHVERTEXDATA_IDX = KyUInt32MAXVAL;   //< Invalid vertex data idx.
static const DynamicGraphEdgeDataIdx INVALID_DYNAMICGRAPHEDGEDATA_IDX = KyUInt32MAXVAL;       //< Invalid edge data idx.
static const DynamicGraphEdgeDataIdx INVALID_DYNAMICPOINTGRAPH_ID = KyUInt32MAXVAL;           //< Invalid graph ID.

}

#endif // #ifndef KyPdg_DynamicPointGraphTypes_H
