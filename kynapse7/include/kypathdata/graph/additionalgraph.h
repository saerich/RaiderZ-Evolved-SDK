/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: LASI - secondary contact: NOBODY
#ifndef KyPathData_AdditionalGraph_H
#define KyPathData_AdditionalGraph_H

#include <kypathdata/generic/basetypes.h>
#include <kypathdata/graph/graphtypes.h>
#include <kypathdata/graph/additionalgraphtypes.h>
#include <kypathdata/graph/graph.h>

namespace Kaim
{

class AdditionalGraph
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_blobtype */
	static const char* GetBlobType() { return "AdditionalGraph"; }
	// version history: 
	// -----------------
	// 0- First Iteration
	// 1- Added VertexConnection infos
	// 2- Added ConnectionRadius to AdditionalGraphExplicitVertexPositionConnection
	// 3- Simplified connections specs. Only 1 to 1 connection allowed. Removed m_explicitVertexPositions and m_explicitVertexIndexes
	// 4- Removed ConnectionRadius from AdditionalGraphVertexInfo
	/*! \label_blobversion */
	static KyUInt32 GetBlobVersion() { return 4; }

public:
	/*!	\label_constructor For internal use. */
	AdditionalGraph() {}

	const AdditionalGraphVertexInfo& GetAdditionalGraphVertexInfo(KyUInt32 vertexIdx) const
	{
		KY_FUNCTION("AdditionalGraph::GetAdditionalGraphVertexInfo");
		KY_ASSERT(vertexIdx < m_verticesInfo.m_count, ("Invalid vertex index"));

		return m_verticesInfo.Values()[vertexIdx];
	}

	const AdditionalGraphEdgeInfo& GetAdditionalGraphEdgeInfo(KyUInt32 edgeIdx) const
	{
		KY_FUNCTION("AdditionalGraph::GetAdditionalGraphEdgeInfo");
		KY_ASSERT(edgeIdx < m_edgesInfo.m_count, ("Invalid edge index"));

		return m_edgesInfo.Values()[edgeIdx];
	}

	const KyGuid& GetGuid() const { return m_graph.m_guidCompound.GetMainGuid(); }

private:
	AdditionalGraph& operator=(const AdditionalGraph& additionalGraph);
	AdditionalGraph(const AdditionalGraph& additionalGraph);

public:
	Graph m_graph;
	BlobArray<AdditionalGraphVertexInfo> m_verticesInfo;
	BlobArray<AdditionalGraphEdgeInfo> m_edgesInfo;
};

/* \label_swapendianness
	\ingroup kypathdata_graph */
inline void SwapEndianness(Endianness::Target e, AdditionalGraph& self)
{
	SwapEndianness(e, self.m_graph);
	SwapEndianness(e, self.m_verticesInfo);
	SwapEndianness(e, self.m_edgesInfo);
}

} // namespace Kaim


#endif // KyPathData_AdditionalGraph_H

