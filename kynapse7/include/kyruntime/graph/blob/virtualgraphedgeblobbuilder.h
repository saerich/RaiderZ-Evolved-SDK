/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: NOBODY
#ifndef KyRuntime_VirtualGraphEdgeBlobBuilder_H
#define KyRuntime_VirtualGraphEdgeBlobBuilder_H


#include "kypathdata/blob/baseblobbuilder.h"
#include "kyruntime/graph/blob/virtualgraphedgeblob.h"

namespace Kaim
{

/* A type of blob builder that creates a VirtualGraphEdgeBlob for a single VirtualGraph edge.
	\ingroup kyruntime_graph */
class VirtualGraphEdgeBlobBuilder : public BaseBlobBuilder<VirtualGraphEdgeBlob>
{
public:
	/*! \label_constructor
		\param startVertexIdx			The index of the starting vertex of the VirtualGraph edge.
		\param endVertexIdx				The index of the ending vertex of the VirtualGraph edge.
		*/
	VirtualGraphEdgeBlobBuilder(const KyUInt32 startVertexIdx, const KyUInt32 endVertexIdx)
	{
		m_startVertexIdx = startVertexIdx;
		m_endVertexId = endVertexIdx;
	}
	/*! \label_destructor */
	~VirtualGraphEdgeBlobBuilder() {}

private:
	virtual void DoBuild();

	KyUInt32 m_startVertexIdx;
	KyUInt32 m_endVertexId;
};


}


#endif
