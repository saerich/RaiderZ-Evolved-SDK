/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

/*!	\file
	\ingroup kypathdata_graph */

#ifndef KyPathData_DynamicAdditionalGraphVertex_H
#define KyPathData_DynamicAdditionalGraphVertex_H

#include "kypathdata/containers/list.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/containers/array.h"
#include "kypathdata/graph/additionalgraphtypes.h"

namespace Kaim
{

class DynamicAdditionalGraphEdge;

/*!	This class represents additional information associated with a single vertex in a DynamicAdditionalGraph. 
	\ingroup kypathdata_graph */
struct DynamicAdditionalGraphVertexInfo
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	/*! \label_constructor
		\param type			Sets the value of #m_connectionType. */
	DynamicAdditionalGraphVertexInfo(KyUInt32 type = AdditionalGraph_No_Connection): m_connectionType(type) {}
	KyUInt32 m_tag; /*!< Can be used to store a custom tag to identify this vertex. */
	KyUInt32 m_data; /*!< Can be used to store custom data about this vertex. */
	KyUInt32 m_connectionType; /*!< An element from the #AdditionalGraphVertexConnectionType enumeration that indicates the way this vertex should be stitched to the surrounding Graph at runtime. */
};

class DynamicAdditionalGraphVertex
{
	KY_DEFINE_NEW_DELETE_OPERATORS
	
public:
	DynamicAdditionalGraphVertex(const Vec3f& position, const DynamicAdditionalGraphVertexInfo& info = DynamicAdditionalGraphVertexInfo())
	{
		m_position = position;
		m_info = info;
	}

	inline const Vec3f& GetPosition() const { return m_position; }
	inline const DynamicAdditionalGraphVertexInfo& GetInfo() const { return m_info; }
	inline DynamicAdditionalGraphVertexInfo& GetInfo() { return m_info; }
	void SetConnection(KyUInt32 type)
	{
		m_info.m_connectionType = type;
	}

	void SetConnectionToRuntimeStitch()
	{
		SetConnection(AdditionalGraph_Multiple_Connections_NearbyVertices);
	}

public:
	Vec3f m_position; //< Position of the vertex.
	DynamicAdditionalGraphVertexInfo m_info;
	List<DynamicAdditionalGraphEdge*> m_incomingEdges;   //< incoming edges.
	List<DynamicAdditionalGraphEdge*> m_outgoingEdges;   //< outgoing edges.

	KyUInt32 m_index; // internal use
};

}

#endif // #ifndef KyPathData_DynamicAdditionalGraphVertex_H
