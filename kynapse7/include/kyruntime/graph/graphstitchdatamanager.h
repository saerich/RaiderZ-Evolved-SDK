/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyRuntime_GraphStitchDataManager_H
#define KyRuntime_GraphStitchDataManager_H

#include "kypathdata/graph/graphtypes.h"
#include "kypathdata/navmesh/stitchednavmesh.h"
#include "kypathdata/navmesh/stitchednavcell.h"

namespace Kaim
{

class StitchedGraph;
class StitchedGraphCell;
class Graph;
class GraphCell;

class AdditionalStitchedGraph;
class AdditionalGraph;

/* For internal use. */
class GraphStitchDataManager
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	GraphStitchDataManager(KyUInt32 bufferSizeAtInit = 50);
	~GraphStitchDataManager();

	void Clear();

	StitchedGraph* AddGraph(const Graph& graph); //<! allocate memory and create a new StitchedGraph
	AdditionalStitchedGraph* AddAdditionalGraph(const AdditionalGraph& additionalGraph); //<! allocate memory, create a new StitchedGraph and a corresponding virtualStitchedGraph

	void RemoveStitchedGraph(StitchedGraph* stitchedGraph); //<! free memory of the StitchedGraph

	StitchedGraph* GetStitchedGraph(const Graph& graph) const; //<! retrieve the StitchedGraph corresponding to graph
	AdditionalStitchedGraph* GetAdditionalStitchedGraph(const AdditionalGraph& additionalGraph) const; //<! retrieve the AdditionalStitchedGraph corresponding to additionalGraph

	/*! For internal use. */
	StitchedGraph* GetStitchedGraph(const GuidCompound& guidCompound) const;
	/*! For internal use. */
	AdditionalStitchedGraph* GetAdditionalStitchedGraph(const GuidCompound& guidCompound) const;

private:
	void RemoveGraph(KyUInt32 idx);

	bool ResizeStitchedGraphBuffer(KyUInt32 newSize = 20);

	void PlaceStitchedGraphInBuffer(const Graph& graph, StitchedGraph*& stitchGraph,  char*& currentMemoryPlace);
	void PlaceAdditionalStitchedGraphInBuffer(const AdditionalGraph& additionalGraph, AdditionalStitchedGraph*& additionalStitchedGraph,  char*& currentMemoryPlace);
	void PlaceStitchedGraphCellInBuffer(const GraphCell* graphCell, StitchedGraphCell*& stitchGraphCell,  char*& currentMemoryPlace);

public:
	StitchedGraph** m_bufferOfStitchedGraph;	//<! In this buffer, all the static graph are at the beginning, the first dynamic graph is at the index m_firstAdditionalGraphIndex and the last is at m_numberOfStitchedGraph-1.
	KyUInt32 m_numberOfStitchedGraph;			//<! Total number of stitched graph (static + additional). Corresponds to the next position after the one of the last stitchedGraph (static or additional if any recorded).
	KyUInt32 m_firstAdditionalGraphIndex;		//<! Position of the first additionalStitchedGraph in m_bufferOfStitchedGraph
	KyUInt32 m_sizeOfStitchedGraphBuffer;		//<! Internal use only (all stitchedGraph from m_numberOfStitchedGraph to m_sizeOfStitchedGraphBuffer are empty).
};


}

#endif //KyRuntime_GraphStitchDataManager_H

