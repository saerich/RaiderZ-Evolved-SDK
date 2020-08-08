/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_graph */

// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_GraphTypes_H
#define KyPathData_GraphTypes_H

#include <kypathdata/generic/types.h>
#include <kypathdata/blob/endianness.h>
#include <kypathdata/generic/kyguid.h>
#include <kypathdata/generic/guidcompound.h>

namespace Kaim
{

typedef KyUInt32 StitchedMode;

static const StitchedMode StitchedMode_OneToOne = 0;
static const StitchedMode StitchedMode_LinktoNearestAccessible = 1;


/*!	Represents a terrain type that can be assigned to a GraphVertex. \ingroup kypathdata_graph */
typedef KyUInt32 GraphVertexTerrainType;
/*!	Represents an invalid or non-existent terrain type. \ingroup kypathdata_graph */
static const GraphVertexTerrainType GraphVertexTerrainType_Unset = 0x00000000;
/*!	Represents the default terrain type. \ingroup kypathdata_graph */
static const GraphVertexTerrainType GraphVertexTerrainType_Default = 0xFFFFFFFF;

/*!	Represents the unique index of a GraphVertex within the list of vertices maintained by a GraphCell. \ingroup kypathdata_graph */
typedef KyUInt32 GraphVertexIdx;
/*!	Represents an invalid #GraphVertexIdx value. \ingroup kypathdata_graph */
static const GraphVertexIdx GraphVertexIdx_Invalid = KyUInt32MAXVAL;

/*!	Represents the unique index of a GraphEdge within the list of edges maintained by a GraphCell. \ingroup kypathdata_graph */
typedef KyUInt32 GraphEdgeIdx;
/*!	Represents an invalid #GraphEdgeIdx value. \ingroup kypathdata_graph */
static const GraphEdgeIdx GraphEdgeIdx_Invalid = KyUInt32MAXVAL;

/*!	Represents the unique index of a GraphEdge within the last A* computation. \ingroup kypathdata_graph */
typedef KyUInt32 AstarId;
/*!	Represents an invalid #AstarId value. \ingroup kypathdata_graph */
static const AstarId AstarId_Invalid = KyUInt32MAXVAL;

/*!	Represents the unique index of a virtual edge within the list of edges maintained by a GraphCell. \ingroup kypathdata_graph */
typedef KyUInt32 VirtualEdgeIdx;
/*!	Represents an invalid #VirtualEdgeIdx value. \ingroup kypathdata_graph */
static const VirtualEdgeIdx VirtualEdgeIdx_Invalid = KyUInt32MAXVAL;

/*!	Represents the unique index of a GraphCell within the list of cells maintained by a Graph. \ingroup kypathdata_graph */
typedef KyUInt32 GraphCellIdx;
/*!	Represents an invalid #GraphCellIdx value. \ingroup kypathdata_graph */
static const GraphCellIdx GraphCellIdx_Invalid = KyUInt32MAXVAL;
/*!	Indicates whether or not the specified #GraphCellIdx is invalid. \ingroup kypathdata_graph */
static KY_FORCE_INLINE bool IsGraphCellIdxValid(const GraphCellIdx idx) { return idx < GraphCellIdx_Invalid; }

/*	Represents the unique index of a StitchedGraphCell within the list of cells maintained by a StitchedGraph. */
typedef KyUInt32 StitchedGraphCellIdx;
/*	Represents an invalid StitchedGraphCellIdx value. */
static const StitchedGraphCellIdx StitchedGraphCellIdx_Invalid = KyUInt32MAXVAL;
/*	Indicates whether or not the specified StitchedGraphCellIdx is invalid. */
static KY_FORCE_INLINE bool IsStitchedGraphCellIdxValid(const StitchedGraphCellIdx idx) { return idx < StitchedGraphCellIdx_Invalid; }

}

#endif

