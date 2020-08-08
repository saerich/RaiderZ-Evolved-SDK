/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*!	\file
	\ingroup kypathdata_graph */

// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_Graph_H
#define KyPathData_Graph_H

#include "kypathdata/graph/graphtypes.h"
#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/graph/graphcell.h"
#include "kypathdata/blob/blobarray.h"
#include "kypathdata/blob/endianness.h"
#include <kypathdata/basesystem/coordsystem.h>
#include "kypathdata/generic/guidcompound.h"

#include "kypathdata/kycrt/math.h"

namespace Kaim
{

class InputGraphVertex;
class InputGraphVertices;

/*! The Graph is one of the main data types that represent the terrain of your \gameOrSim world from the point of
	view of your characters. Each instance of the Graph class represents the topology of a single sector of your terrain. 
	\pp Each Graph is made up of a regular grid of GraphCell objects, each of which contains vertices and edges 
	within a localized, square area of the sector.
	\pp For more details on \Graphs, including how to create them from your terrain, how to stream them at runtime, and
	how to access them, see the :USERGUIDE:"PathData" chapter.
	\ingroup kypathdata_navmesh */
class Graph
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public: 
	enum GraphFlags
	{
		GRAPH_NO_FLAG		   = 0,
		GRAPH_FROM_PARTITIONER = 1, 
		GRAPH_IS_ADDITIONAL_GRAPH = 2,
		// GRAPH_ADDITIONAL_FLAG = 1<<1; etc

		GraphFlags_FORCE32 = 0xFFFFFFFF
	};

public:
	/*! \label_blobtype */
	static const char* GetBlobType() { return "Graph"; }

	// version history: 
	// -----------------
	// 0- Initial version.
	// 1- m_guidCompound now has a timeStamp
	// 2- ??
	// 3- Added GraphFlags
	/*! \label_blobversion */
	static KyUInt32 GetBlobVersion() { return 3; }

public:
	/*! \label_constructor For internal use. */
	Graph() : m_cellSize(0.0f) {}

	/*! Retrieves the number of \GraphCells in this Graph. */
	KY_FORCE_INLINE KyUInt32 GetCellCount() const { return m_cells.m_count; }

	/*! Retrieves the GraphCell at the specified index. */
	KY_FORCE_INLINE const GraphCell& GetGraphCell(KyUInt32 idx) const { return m_cells.Values()[idx]; }

	/*! Retrieves the Box2i that reflects the Graph partitioning scheme. */
	KY_FORCE_INLINE const Box2i& GetCellBox() const { return m_cellBox; }

	/*! Indicates whether or not the specified Graph was created with the same generation parameters
		as this object. */
	bool IsCompatibleWith(const Graph& graph) const;

public:
	// functions to be used during PDG where GraphManager is not available

	/*! Computes the CellPos that corresponds to the specified position in 3D space, taking into account the
		cell size of this Graph. For internal use during PathData generation. */
	KY_FORCE_INLINE CellPos ComputeGraphCellPos(const Vec3f& pos) const
	{
		const KyFloat32 cellSizeInv = 1.f / m_cellSize;
		// Ceilf - 1 on x and Floorf on y to be IsInsideTriangle compatible
		return CellPos((KyInt32) Ceilf((pos.x * cellSizeInv)) - 1, (KyInt32) Floorf((pos.y * cellSizeInv)));
	}

	/*! Retrieves the index of the cell at the specified CellPos. For internal use during PathData generation. */
	GraphCellIdx GetGraphCellIdxFromCellPos(const CellPos& cellPos) const;

	/*! Retrieves a pointer to the InputGraphVertex within \c inputGraphVertices that was created for the input position specified by \c clientPos, if any. 
		If \c pos does not match a vertex position provided as input for this sector, returns #KY_NULL.
		\pp See also :USERGUIDE:"Specifying Vertex Positions".
		\pp This method is typically called during a post-processing phase of the PathData generation process. See also
		the ISectorGeneratorPostProcess class.
		\pp At runtime, you can call GraphManager::GetGraphVertexPtrFromInputGraphVertexPos() instead.
		\param clientPos			The position of the input vertex as initially provided to the PathData generation system, in the \gameOrSim coordinate system. 
		\param inputGraphVertices	The InputGraphVertices instance that records the input vertices created within this Graph. */
	const InputGraphVertex* GetInputVertexFromClientInitialPos(const Vec3f& clientPos, const InputGraphVertices& inputGraphVertices) const;

	/*! Retrieves a pointer to the GraphVertex within this Graph that was created for the input position specified by \c clientPos, if any.
		If \c pos does not match a vertex position provided as input for this sector, returns #KY_NULL.
		\pp See also :USERGUIDE:"Specifying Vertex Positions".
		\pp This method is typically called during a post-processing phase of the PathData generation process. See also
		the ISectorGeneratorPostProcess class.
		\pp At runtime, you can call GraphManager::GetGraphVertexPtrFromInputGraphVertexPos() instead.
		\param clientPos			The position of the input vertex as initially provided to the PathData generation system, in the \gameOrSim coordinate system. 
		\param inputGraphVertices	The InputGraphVertices instance that records the input vertices created within this Graph. */
	const GraphVertex* GetGraphVertexFromClientInitialPos(const Vec3f& clientPos, const InputGraphVertices& inputGraphVertices) const;


public:
	CoordSystem m_coordSystem; /*!< The coordinate system used by the Graph. Do not modify. */
	GuidCompound m_guidCompound; /*!< For internal use. Do not modify. */
	Box2i m_cellBox; /*!< The partitioning of the Graph into GraphCells. Do not modify. */
	KyFloat32 m_cellSize; /*!< The width of each GraphCell. Do not modify. */
	BlobArray<GraphCell> m_cells; /*!< The list of \GraphCells. Do not modify. */
	KyUInt32 m_flags; /*!< Maintains extra information about this Graph, set at generation time. For internal use. Do not modify. */

private:
	Graph& operator=(const Graph& graph);
	Graph(const Graph& graph);
};

/*! \label_swapendianness
	\ingroup kypathdata_graph */
inline void SwapEndianness(Endianness::Target e, Graph& self)
{
	SwapEndianness(e, self.m_coordSystem);
	SwapEndianness(e, self.m_guidCompound);
	SwapEndianness(e, self.m_cellBox);
	SwapEndianness(e, self.m_cellSize);
	SwapEndianness(e, self.m_cells);
	SwapEndianness(e, self.m_flags);

}

}


#endif //KyPathData_Graph_H

