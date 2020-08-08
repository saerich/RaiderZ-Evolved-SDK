/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

/*! \file
	\ingroup kypathdata_graph */

// primary contact: LASI - secondary contact: NOBODY
#ifndef KyPdg_InputGraphVertexCell_H
#define KyPdg_InputGraphVertexCell_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/blob/blobarray.h"
#include "kypathdata/math/box3f.h"
#include "kypathdata/graph/inputgraphvertex.h"
#include "kypathdata/containers/array.h"
#include "kypathdata/generic/guidcompound.h"

namespace Kaim
{

class DynamicInputGraphVertexCell
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	CellPos m_cellPos;
	Array<InputGraphVertex> m_vertices;
};

/*! Records a list of the vertices provided as input to the PathData generation process for a single
	cell within a sector.
	\ingroup kypathdata_graph */
class InputGraphVertexCell
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! Retrieves the InputGraphVertex that was created during the PathData generation process to represent
		the specified input position. 
		\pp This method is needed because the vertex actually present in the Graph may not be at exactly the 
		same position that was originally provided as input to the PathData generation process. For details, see 
		:USERGUIDE:"Specifying Vertex Positions". */
	const InputGraphVertex* GetInputGraphVertexFromInitialPos(const Vec3f& pos) const;

public:
	CellPos m_cellPos; /*!< The position of this cell within the grid of cells in its InputGraphVertices. */
	Box3f m_boundingBox; /*!< The axis-aligned bounding box that represents the extents of this cell. */
	/*! The list of InputGraphVertex objects that contains the Graph vertices created to represent the 
		vertex positions provided as input to the PathData generation process. */
	BlobArray<InputGraphVertex> m_vertices; 
};

inline void SwapEndianness(Endianness::Target e, InputGraphVertexCell& self)
{
	SwapEndianness(e, self.m_cellPos);
	SwapEndianness(e, self.m_boundingBox);
	SwapEndianness(e, self.m_vertices);
}

/*! Records all vertices provided as input to the PathData generation process for a single
	sector, organized into cells.
	\ingroup kypathdata_graph */
class InputGraphVertices
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_blobtype */
	static const char* GetBlobType() { return "InputGraphVertices"; }
	// history:
	// 0: first iteration
	// 1: added a guidCompound and renamed to InputGraphVertices
	/*! \label_blobversion */
	static KyUInt32 GetBlobVersion() { return 1; }
	BlobArray<InputGraphVertexCell> m_cells; /*!< The list of cells in this sector. */
	GuidCompound m_guidCompound; /*!< For internal use. Do not modify. */
};

inline void SwapEndianness(Endianness::Target e, InputGraphVertices& self)
{
	SwapEndianness(e, self.m_cells);
	SwapEndianness(e, self.m_guidCompound);
}

} // namespace Kaim


#endif

