/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: NOBODY
#ifndef KyPdg_PathObjectTopologyBlobBuilder_H
#define KyPdg_PathObjectTopologyBlobBuilder_H


#include "kypathdata/blob/baseblobbuilder.h"
#include "kypathdata/pathobject/pathobjectdata.h"

namespace Kaim
{

class IPathObjectTopology;

class PathObjectTopologyBlobBuilder : public BaseBlobBuilder<PathObjectTopologyData>
{
public:
	PathObjectTopologyBlobBuilder(const IPathObjectTopology* topology) { m_topology = topology; }
	~PathObjectTopologyBlobBuilder() {}

private:
	virtual void DoBuild();

	const IPathObjectTopology* m_topology;
};


}


#endif
