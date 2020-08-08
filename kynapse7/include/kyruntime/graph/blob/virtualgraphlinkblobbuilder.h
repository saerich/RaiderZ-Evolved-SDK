/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: NOBODY
#ifndef KyRuntime_VirtualGraphLinkBlobBuilder_H
#define KyRuntime_VirtualGraphLinkBlobBuilder_H


#include "kypathdata/blob/baseblobbuilder.h"
#include "kyruntime/graph/blob/virtualgraphlinkblob.h"

namespace Kaim
{

/* A type of blob builder that creates a VirtualGraphLinkBlob for a single VirtualGraph.
	\ingroup kyruntime_graph */
class VirtualGraphLinkBlobBuilder : public BaseBlobBuilder<VirtualGraphLinkBlob>
{
public:
	VirtualGraphLinkBlobBuilder(
		const KyUInt32 virtualGraphVertexIdx, /*!< The index of the vertex of the VirtualGraph. */
		const KyUInt32 connectedGraphId, /*!< The ID of the Graph connected to the VirtualGraph. */
		const KyUInt32 connectedVertexIdx, /*!< The index of the vertex in the Graph that is connected to the VirtualGraph. */
		const Vec3f& connectedPosition, /*!< The position of the vertex in the connected Graph. */
		const KyBool startIsInVirtualGraph) /*!< #KY_TRUE if the start vertex belongs to the VirtualGraph. #KY_TRUE if the start vertex belongs to another Graph. */
	{
		m_virtualGraphVertexIdx = virtualGraphVertexIdx;
		m_connectedGraphId = connectedGraphId;
		m_connectedVertexIdx = connectedVertexIdx;
		m_connectedPosition = connectedPosition;
		m_startIsInVirtualGraph = startIsInVirtualGraph;
	}
	~VirtualGraphLinkBlobBuilder() {}

private:
	virtual void DoBuild();

	KyUInt32 m_virtualGraphVertexIdx;                     //< Index of the vertex of the VirtualGraph.
	KyUInt32 m_connectedGraphId;                          //< ID of the graph connected to the VirtualGraph.
	KyUInt32 m_connectedVertexIdx;                        //< Idx of the vertex in the graph connected to the VirtualGraph.
	Vec3f m_connectedPosition;                            //< Position of the vertex in the connected graph.
	KyBool m_startIsInVirtualGraph;                       //< True if the start vertex belongs to the VirtualGraph. False if the start vertex belongs to another graph.
};


}


#endif
