/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_StitchedGraph_H
#define KyPathData_StitchedGraph_H

#include "kypathdata/graph/graphtypes.h"
#include "kypathdata/generic/datareference.h"
#include "kypathdata/generic/referencemanager.h"
#include "kypathdata/graph/graph.h"

#include <string.h> // for memset

namespace Kaim
{

class StitchedGraph;

typedef DataReference<StitchedGraph> GraphReference;
typedef ReferenceManager<GraphReference> GraphReferenceManager;

class StitchedGraphCell;
class GraphManager;
class IPathObject;
class InputGraphVertices;

/* For internal use. */
class StitchedGraph
{
	KY_DEFINE_NEW_DELETE_OPERATORS

	StitchedGraph& operator=(const StitchedGraph&);

friend class GraphStitchDataManager;
friend class AdditionalStitchedGraph;

	StitchedGraph(const Graph& graph);
	StitchedGraph();

public:
	KY_FORCE_INLINE void SetGraphReference(GraphReference& graphReference) { m_reference = &graphReference; }
	KY_FORCE_INLINE GraphReference* GetGraphReference() const { return m_reference; }

	const GuidCompound& GetGuidCompound() const { return m_graph->m_guidCompound; }

	KY_FORCE_INLINE const GraphManager* GetGraphManager() { return m_graphManager; }
	const GraphManager* GetGraphManager() const { return m_graphManager; }

	const Graph& GetGraph() const { return *m_graph; }

	KY_FORCE_INLINE bool IsVirtual() const { return m_graph == KY_NULL; }
	KY_FORCE_INLINE bool IsAdditionalGraph() const { return m_graph != KY_NULL && m_graph->m_flags & Graph::GRAPH_IS_ADDITIONAL_GRAPH; }

	KY_FORCE_INLINE IPathObject* GetPathObject(GraphEdgeIdx graphEdgeIdx) const { return IsAdditionalGraph() ? m_pathObjects[graphEdgeIdx] : KY_NULL; }

	KY_FORCE_INLINE void SetPathObject(GraphEdgeIdx graphEdgeIdx, IPathObject* IPathObject)
	{
		KY_FUNCTION("StitchedGraph::SetPathObject");
		KY_ASSERTK(IsAdditionalGraph());
		m_pathObjects[graphEdgeIdx] = IPathObject;
	}

	KY_FORCE_INLINE bool HasEdgeAdditionalData() const { return m_edgeAdditionalDatas != KY_NULL; }
	KY_FORCE_INLINE bool HasVertexAdditionalData() const { return m_vertexAdditionalDatas != KY_NULL; }
	KY_FORCE_INLINE bool HasInputGraphVertices() const { return m_inputGraphVertices != KY_NULL; }

public:
	KyUInt32 m_idxInTheBufferOfStitchedGraph;
	const Graph* m_graph;

	void* m_edgeAdditionalDatas;
	void* m_vertexAdditionalDatas;
	InputGraphVertices* m_inputGraphVertices;

	GraphReference* m_reference;
	const GraphManager* m_graphManager;

	KyUInt32 m_numberOfCells;

	mutable StitchedGraph* m_virtualStitchedGraph;

	StitchedGraphCell** m_stitchedCells;

	IPathObject** m_pathObjects;
};


}

#endif //KyPathData_StitchedGraph_H

