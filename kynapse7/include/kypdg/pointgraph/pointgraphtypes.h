/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPathData_PointGraphTypes_H
#define KyPathData_PointGraphTypes_H



namespace Kaim
{

typedef KyUInt32 GraphVertexIdx;
typedef KyUInt32 GraphEdgeIdx;
typedef KyUInt32 PointGraphId;

static const GraphVertexIdx INVALID_GRAPHVERTEX_IDX = KyUInt32MAXVAL;           //< Invalid vertex idx.
static const GraphEdgeIdx INVALID_GRAPHEDGE_IDX = KyUInt32MAXVAL;               //< Invalid edge idx.
static const PointGraphId INVALID_GRAPH_ID = KyUInt32MAXVAL;                    //< Invalid edge data idx.

}

#endif // #ifndef KyPathData_PointGraphTypes_H
