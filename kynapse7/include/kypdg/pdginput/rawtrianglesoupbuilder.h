/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPdg_RawTriangleSoupBuilder_H
#define KyPdg_RawTriangleSoupBuilder_H


#include "kypathdata/blob/baseblobbuilder.h"
#include "kypdg/pdginput/rawtrianglesoup.h"


namespace Kaim
{


class RawTriangleSoupBuilder : public BaseBlobBuilder<RawTriangleSoup>
{
public:
	RawTriangleSoupBuilder(KyUInt32 verticesCount, const Vec3f* vertices, KyUInt32 indicesCount, const KyUInt32* indices)
	{
		m_verticesCount = verticesCount;
		m_vertices = vertices;
		m_indicesCount = indicesCount;
		m_indices = indices;
	}

private:
	virtual void DoBuild()
	{
		BLOB_ARRAY_COPY(m_blob->m_vertices, m_vertices, m_verticesCount);
		BLOB_ARRAY_COPY(m_blob->m_indices , m_indices , m_indicesCount);
	}

	KyUInt32 m_verticesCount;
	const Vec3f* m_vertices;
	KyUInt32 m_indicesCount;
	const KyUInt32* m_indices;
};


}


#endif
