/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

// primary contact: BEHE - secondary contact: LASI

#ifndef KyPdg_DynamicPointGraphCell_H
#define KyPdg_DynamicPointGraphCell_H

#include <kypathdata/generic/memory.h>
#include "kypathdata/blob/blobhandler.h"
#include "kypathdata/blob/blobfilewriter.h"
#include "kypathdata/math/vec3f.h"
#include "kypathdata/math/box3f.h"
#include "kypathdata/navmesh/navcell.h"
#include "kypdg/generator/pdgsystem.h"
#include "kypdg/pointgraph/pdgpointgraph.h"
#include "kypdg/generator/celldesc.h"


namespace Kaim
{

class PdgGraphEdge;
class PdgGraphVertex;

/*	This class contains an editable pointgraph and provides all methods
	to create a pointgraph from scratch or from a navCell. */
class DynamicPointGraphCell
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	DynamicPointGraphCell(PdgSystem* sys, const CellDesc& cellDesc);

	~DynamicPointGraphCell();

public: 
	PdgPointGraph* GetGraph() { return m_graph; }
	const PdgPointGraph* GetGraph() const { return m_graph; }

	/* Get number of edges. */
	KyUInt32 GetEdgeCount() const { return m_graph->GetEdgeCount(); }

	/* Get number of vertices. */
	KyUInt32 GetVertexCount() const { return m_graph->GetVertexCount(); }

public:
	PdgSystem* m_sys;                 //< System (must be used to allocate and retrieve coordsytem)
	CellDesc m_cellDesc;
	CellPos m_cellPos;                //< the cell position of the pointGraphCell.
	PdgPointGraph* m_graph;
	bool m_useTerrainTypes;
};

}

#endif // #ifndef KyPdg_DynamicPointGraphCell_H
