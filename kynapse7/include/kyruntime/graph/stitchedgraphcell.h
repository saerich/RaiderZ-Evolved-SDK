/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// ---------- Primary contact: JUBA - secondary contact: MAMU
#ifndef KyPathData_StitchedGraphCell_H
#define KyPathData_StitchedGraphCell_H

#include <kypathdata/graph/graphcell.h>
#include <kyruntime/graph/stitchedgraph.h>
#include <kyruntime/graph/vertexdata.h>
#include <kyruntime/graph/edgedata.h>

#include <kypathdata/graph/graphtypes.h>
#include <kypathdata/containers/array.h>
#include <kypathdata/containers/list.h>

#include <kypathdata/generic/datareference.h>
#include <kypathdata/generic/referencemanager.h>
#include <kypathdata/generic/guidcompound.h>

namespace Kaim
{

class GraphVertexPtr;

/* For internal use. */
class StitchedGraphCell
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	StitchedGraphCell(const GraphCell* graphCell) : m_parentStitchedGraph(KY_NULL), m_idxInTheBufferOfStitchedGraphCell(KyUInt32MAXVAL),
		m_IsStitched(false), m_stitchedVersion(KyUInt32MAXVAL), m_graphCell(graphCell), m_vertexDatas(KY_NULL), m_edgeDatas(KY_NULL)
	{
		memset(m_numberOfLinks, 0, 4 * sizeof(KyUInt32));
		memset(m_links, 0, 4 * (KyUInt32)sizeof(void*));
	}

	StitchedGraphCell() : m_parentStitchedGraph(KY_NULL), m_idxInTheBufferOfStitchedGraphCell(KyUInt32MAXVAL),
		m_IsStitched(false), m_stitchedVersion(KyUInt32MAXVAL), m_graphCell(KY_NULL), m_vertexDatas(KY_NULL), m_edgeDatas(KY_NULL)
	{
		memset(m_numberOfLinks, 0, 4 * sizeof(KyUInt32));
		memset(m_links, 0, 4 * (KyUInt32)sizeof(void*));
	}



	GraphReference* GetGraphReference() const { return m_parentStitchedGraph->m_reference; }
	StitchedGraphCellIdx GetStitchedGraphCellIdx() const { return m_idxInTheBufferOfStitchedGraphCell; }

	KY_FORCE_INLINE const GraphCell& GetGraphCell() const
	{
		KY_FUNCTION("StitchedGraphCell::GetGraphCell");
		KY_ASSERTK(m_graphCell != KY_NULL);
		return *m_graphCell;
	}

	KY_FORCE_INLINE bool IsVirtual() const { return m_parentStitchedGraph->IsVirtual(); }
	KY_FORCE_INLINE bool IsStitched() const { return m_IsStitched; }

	const GuidCompound& GetGuidCompound() const { return m_parentStitchedGraph->GetGuidCompound(); }

	KY_FORCE_INLINE const GraphManager* GetGraphManager() { return m_parentStitchedGraph->GetGraphManager(); }
	KY_FORCE_INLINE const GraphManager* GetGraphManager() const { return m_parentStitchedGraph->GetGraphManager(); }

	KY_FORCE_INLINE void SetAstarId(GraphVertexIdx vertexIdx, KyUInt32 astarId) const { GetVertexData(vertexIdx).SetAstarId(astarId); }
	KY_FORCE_INLINE KyUInt32 GetAstarId(GraphVertexIdx vertexIdx) const { return GetVertexData(vertexIdx).GetAstarId(); }

	KY_FORCE_INLINE VertexData& GetVertexData(GraphVertexIdx vertexIdx) const
	{
		KY_FUNCTION("StitchedGraphCell:GetVertexData");
		KY_ASSERT(!IsVirtual(), ("You must not call function GetVertexData on a virtual graphCell"));
		KY_ASSERTK(vertexIdx < m_graphCell->GetVertexCount());
		return m_vertexDatas[vertexIdx];
	}

	KY_FORCE_INLINE EdgeData& GetEdgeData(GraphEdgeIdx edgeIdx) const
	{
		KY_FUNCTION("StitchedGraphCell:GetEdgeData");
		KY_ASSERT(!IsVirtual(), ("You must not call function GetEdgeData on a virtual graphCell"));
		KY_ASSERTK(edgeIdx < m_graphCell->GetEdgeCount());
		return m_edgeDatas[edgeIdx];
	}

	StitchedGraph& GeParentStitchedGraph() { return *m_parentStitchedGraph; }
	const StitchedGraph& GeParentStitchedGraph() const { return *m_parentStitchedGraph; }

	GraphVertexPtr GetGraphVertexPtrFromInputGraphVertexPos(const Vec3f& pos) const;

public:
	StitchedGraph* m_parentStitchedGraph;
	StitchedGraphCellIdx m_idxInTheBufferOfStitchedGraphCell;
	bool m_IsStitched;
	KyUInt32 m_stitchedVersion;		// a kind of stitched version to detect when a cell has been stitched and unstitched between two frames
	const GraphCell* m_graphCell;

	KyUInt32 m_numberOfLinks[4];
	GraphVertexPtr* m_links[4];

	mutable VertexData* m_vertexDatas;
	mutable EdgeData* m_edgeDatas;
};


}

#endif //KyPathData_StitchedGraphCell_H

