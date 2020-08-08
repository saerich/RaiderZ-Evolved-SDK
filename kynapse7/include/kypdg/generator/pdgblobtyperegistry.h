/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_PdgBlobTypeRegistry_H
#define KyPdg_PdgBlobTypeRegistry_H


#include <kypathdata/pathdata/pathdatablobtyperegistry.h>


namespace Kaim
{


/* BlobTypeRegistry that registers all Pdg and PathData blobs */
class PdgBlobTypeRegistry : public PathDataBlobTypeRegistry
{
public:
	PdgBlobTypeRegistry();
};


/* BlobAggregate that uses a BlobTypeRegistry that registers all Pdg and PathData blobs */
class PdgBlobAggregate : public BlobAggregate
{
public:
	PdgBlobAggregate() : BlobAggregate(&m_pathDataBlobTypeRegistry) {}

public:
	PathDataBlobTypeRegistry m_pathDataBlobTypeRegistry;
};


}


#endif

