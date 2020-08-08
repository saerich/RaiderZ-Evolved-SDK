/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*!	\file
	\ingroup kypathdata_graph */

#ifndef KyPathData_AdditionalGraphEdge_H
#define KyPathData_AdditionalGraphEdge_H

#include <kypathdata/generic/memory.h>
#include "kypathdata/graph/dynamicadditionalgraphvertex.h"

namespace Kaim
{

/*!	This class represents additional information associated with a single edge in a DynamicAdditionalGraph. 
	\ingroup kypathdata_graph */
struct DynamicAdditionalGraphEdgeInfo
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	/*! \label_constructor */
	DynamicAdditionalGraphEdgeInfo(): m_tag(0), m_terrainType(0), m_data(0) {}
	/*! \label_constructor
		\param tag				Sets the value of #m_tag.
		\param terrainType		Sets the value of #m_terrainType.
		\param data				Sets the value of #m_data. */
	DynamicAdditionalGraphEdgeInfo(KyUInt32 tag, KyUInt32 terrainType, KyUInt32 data): m_tag(tag), m_terrainType(terrainType), m_data(data) {}
	KyUInt32 m_tag; /*!< Can be used to store a custom tag to identify this edge. */
	KyUInt32 m_terrainType; /*!< Sets the terrain type mask to be assigned to this edge. */
	KyUInt32 m_data; /*!< Can be used to store custom data about this edge. */
};

/*
Contains all basic data of a graph edge.
*/
class DynamicAdditionalGraphEdge
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	DynamicAdditionalGraphEdge(): m_startVertex(KY_NULL), m_endVertex(KY_NULL) {}
	DynamicAdditionalGraphEdge(DynamicAdditionalGraphVertex* startVertex, DynamicAdditionalGraphVertex* endVertex, const DynamicAdditionalGraphEdgeInfo& info)
	{
		KY_FUNCTION("DynamicAdditionalGraphEdge::DynamicAdditionalGraphEdge");

		KY_ASSERT(&startVertex != &endVertex, ("Creation of an illegal circular edge."));
		m_startVertex = startVertex;
		m_endVertex = endVertex;
		startVertex->m_outgoingEdges.PushBack(this);
		endVertex->m_incomingEdges.PushBack(this);
		m_info = info;
	}

	~DynamicAdditionalGraphEdge()
	{
		Clear();
	}

	void Clear()
	{
		if (m_startVertex)
			m_startVertex->m_outgoingEdges.RemoveAllOccurrences(this);
		if (m_endVertex)
			m_endVertex->m_incomingEdges.RemoveAllOccurrences(this);
		m_startVertex = KY_NULL;
		m_endVertex = KY_NULL;
		m_index = KyUInt32MAXVAL;
	}

	DynamicAdditionalGraphVertex* GetStartVertex() { return m_startVertex; }
	DynamicAdditionalGraphVertex* GetEndVertex() { return m_endVertex; }
	const DynamicAdditionalGraphVertex* GetStartVertex() const { return m_startVertex; }
	const DynamicAdditionalGraphVertex* GetEndVertex() const { return m_endVertex; }
	const DynamicAdditionalGraphEdgeInfo& GetInfo() const { return m_info; }

	inline KyFloat32 GetSquareLength() const { return (m_startVertex->GetPosition() - m_endVertex->GetPosition()).GetSquareLength(); }
	inline KyFloat32 GetLength() const { return (m_startVertex->GetPosition() - m_endVertex->GetPosition()).GetLength(); }
	inline KyFloat32 GetSquareLength2d() const { return (m_startVertex->GetPosition() - m_endVertex->GetPosition()).GetSquareLength2d(); }
	inline KyFloat32 GetLength2d() const { return (m_startVertex->GetPosition() - m_endVertex->GetPosition()).GetLength2d(); }

public:
	DynamicAdditionalGraphVertex* m_startVertex;   //< start vertex.
	DynamicAdditionalGraphVertex* m_endVertex;     //< end vertex.
	DynamicAdditionalGraphEdgeInfo m_info;
	KyUInt32 m_index; // internal use
};

}

#endif // #ifndef KyPathData_AdditionalGraphEdge_H
