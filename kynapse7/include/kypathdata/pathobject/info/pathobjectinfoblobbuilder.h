/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: NOBODY
#ifndef KyPdg_PathObjectInfoBlobBuilder_H
#define KyPdg_PathObjectInfoBlobBuilder_H


#include "kypathdata/blob/baseblobbuilder.h"
#include "kypathdata/pathobject/info/pathobjectinfodata.h"

namespace Kaim
{

class IPathObjectInfo;

class PathObjectInfoBlobBuilder : public BaseBlobBuilder<PathObjectInfoData>
{
public:
	PathObjectInfoBlobBuilder(const IPathObjectInfo* info) { m_info = info; }
	~PathObjectInfoBlobBuilder() {}

private:
	virtual void DoBuild();

	const IPathObjectInfo* m_info;
};


}


#endif
