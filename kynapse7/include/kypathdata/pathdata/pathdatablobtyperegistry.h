/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPathData_PathDataBlobTypeRegistry_H
#define KyPathData_PathDataBlobTypeRegistry_H


#include <kypathdata/blob/blobtyperegistry.h>
#include <kypathdata/blob/blobaggregate.h>
#include <kypathdata/navmesh/navmesh.h>
#include <kypathdata/graph/graphcell.h>
#include <kypathdata/graph/graph.h>
#include <kypathdata/graph/jumpedgecandidatearray.h>
#include <kypathdata/graph/additionalgraph.h>
#include "kypathdata/graph/inputgraphvertexcell.h"

namespace Kaim
{


/* BlobTypeRegistry that registers all PathData blobs */
class PathDataBlobTypeRegistry : public BlobTypeRegistry
{
public:
	PathDataBlobTypeRegistry()
	{
		AddBlobType<BlobFieldArray>();
		AddBlobType<NavCell>();
		AddBlobType<NavMesh>();
		AddBlobType<AdditionalGraph>();
		AddBlobType<GraphCell>();
		AddBlobType<Graph>();
		AddBlobType<JumpEdgeCandidateArray>();
		AddBlobType<CoordSystem>();
		AddBlobType<InputGraphVertices>();
	}
};


/* BlobAggregate that uses a BlobTypeRegistry that registers all PathData blobs */
class PathDataBlobAggregate : public BlobAggregate
{
public:
	PathDataBlobAggregate() : BlobAggregate(&m_pathDataBlobTypeRegistry) {}

public:
	PathDataBlobTypeRegistry m_pathDataBlobTypeRegistry;
};


}


#endif

