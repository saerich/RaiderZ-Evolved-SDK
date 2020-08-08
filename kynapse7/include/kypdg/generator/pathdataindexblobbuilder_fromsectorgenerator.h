/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: LASI - secondary contact: NONE
#ifndef KyPdg_PathDataIndexDescriptorBlobBuilder_FromSectorGenerator_H
#define KyPdg_PathDataIndexDescriptorBlobBuilder_FromSectorGenerator_H


#include <kypathdata/blob/baseblobbuilder.h>
#include <kypathdata/pathdata/pathdataindex.h>


namespace Kaim
{


class PdgSectorGenerator;


class PathDataIndexBlobBuilder_FromSectorGenerator : public BaseBlobBuilder<PathDataIndex>
{
public:
	PathDataIndexBlobBuilder_FromSectorGenerator(PdgSectorGenerator* sectorGenerator) { m_sectorGenerator = sectorGenerator; }

private:
	virtual void DoBuild();

	PdgSectorGenerator* m_sectorGenerator;
};


}


#endif
