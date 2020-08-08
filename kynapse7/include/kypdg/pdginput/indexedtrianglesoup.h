/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPdg_IndexedTriangleSoup_H
#define KyPdg_IndexedTriangleSoup_H

#include "kypathdata/navmesh/navmeshtypes.h"

#include <kypdg/common/stlvector.h>


namespace Kaim
{

class IndexedTriangleSoup
{
public:
	IndexedTriangleSoup() {}

	void Clear()
	{
		m_vertices.clear();
		m_triangles.clear();
	}

public:
	StlVector<Vec3f> m_vertices;
	StlVector<KyUInt32> m_triangles;
};

}

#endif //KyPdg_IndexedTriangle_H

