/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: LASI
#ifndef KyPdg_PathDataIndexDescriptorBlobBuilder_FromGenerator_H
#define KyPdg_PathDataIndexDescriptorBlobBuilder_FromGenerator_H


#include <kypathdata/blob/baseblobbuilder.h>
#include <kypathdata/pathdata/pathdataindex.h>


namespace Kaim
{


class Generator;


class PathDataIndexBlobBuilder_FromGenerator : public BaseBlobBuilder<PathDataIndex>
{
public:
	PathDataIndexBlobBuilder_FromGenerator(Generator* generator) { m_generator = generator; }

private:
	virtual void DoBuild();

	Generator* m_generator;
};


}


#endif
