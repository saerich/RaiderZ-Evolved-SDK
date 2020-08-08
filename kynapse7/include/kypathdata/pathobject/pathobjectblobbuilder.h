/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: NOBODY
#ifndef KyPathData_PathObjectBlobBuilder_H
#define KyPathData_PathObjectBlobBuilder_H


#include "kypathdata/blob/baseblobbuilder.h"
#include "kypathdata/pathobject/pathobjectdata.h"

namespace Kaim
{

class IPathObjectTopology;
class IPathObjectInfo;

class PathObjectBlobBuilder : public BaseBlobBuilder<PathObjectData>
{
public:
	PathObjectBlobBuilder(const IPathObjectTopology* topology, const IPathObjectInfo* info)
	{
		m_topology = topology;
		m_info = info;
	}
	~PathObjectBlobBuilder() {}

private:
	virtual void DoBuild();

	const IPathObjectTopology* m_topology;
	const IPathObjectInfo* m_info;
};


}


#endif
