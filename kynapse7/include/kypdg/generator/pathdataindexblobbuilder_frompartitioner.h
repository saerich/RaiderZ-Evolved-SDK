/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: LASI - secondary contact: NONE
#ifndef KyPdg_PathDataIndexBlobBuilder_FromPartitioner_H
#define KyPdg_PathDataIndexBlobBuilder_FromPartitioner_H


#include <kypathdata/blob/baseblobbuilder.h>
#include <kypathdata/pathdata/pathdataindex.h>


namespace Kaim
{


class PdgSectorPartitioner;


class PathDataIndexBlobBuilder_FromPartitioner : public BaseBlobBuilder<PathDataIndex>
{
public:
	PathDataIndexBlobBuilder_FromPartitioner(PdgSectorPartitioner* partitioner) { m_partitioner = partitioner; }

private:
	virtual void DoBuild();

	PdgSectorPartitioner* m_partitioner;
};


}


#endif
