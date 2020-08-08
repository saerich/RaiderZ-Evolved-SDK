/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: NOBODY
#ifndef KyRuntime_PathObjectBlobBuilder_H
#define KyRuntime_PathObjectBlobBuilder_H


#include "kypathdata/blob/baseblobbuilder.h"
#include "kyruntime/pathobjects/runtimepathobjecttopology/runtimepathobjectdata.h"

namespace Kaim
{

class RuntimePathObjectTopology;
class IPathObjectInfo;
class PathObjectManager;
class StitchedGraph;

class RuntimePathObjectBlobBuilder : public BaseBlobBuilder<RuntimePathObjectData>
{
public:
	RuntimePathObjectBlobBuilder(
		const RuntimePathObjectTopology* topology,
		const IPathObjectInfo* info,
		StitchedGraph* stitchedGraph = KY_NULL)
	{
		m_topology = topology;
		m_info = info;
		m_stitchedGraph = stitchedGraph;
	}
	~RuntimePathObjectBlobBuilder() {}

private:
	virtual void DoBuild();

	const RuntimePathObjectTopology* m_topology;
	const IPathObjectInfo* m_info;
	StitchedGraph* m_stitchedGraph;
};


}


#endif
