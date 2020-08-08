/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: NOBODY
#ifndef KyRuntime_VirtualGraphVertexBlobBuilder_H
#define KyRuntime_VirtualGraphVertexBlobBuilder_H


#include "kypathdata/blob/baseblobbuilder.h"
#include "kyruntime/graph/blob/virtualgraphvertexblob.h"

namespace Kaim
{

/* A type of blob builder that creates a VirtualGraphVertexBlob for a single vertex in a VirtualGraph.
	\ingroup kyruntime_graph */
class VirtualGraphVertexBlobBuilder : public BaseBlobBuilder<VirtualGraphVertexBlob>
{
public:
	/*! \label_constructor
		\param position: The position of the VirtualGraph vertex. */
	VirtualGraphVertexBlobBuilder(const Vec3f& position) { m_position = position; }
	/*! \label_destructor */
	~VirtualGraphVertexBlobBuilder() {}

private:
	virtual void DoBuild();

	Vec3f m_position;
};


}


#endif
