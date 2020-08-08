/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPdg_IndexedTriangleSoupToDynamicNavMesh_H
#define KyPdg_IndexedTriangleSoupToDynamicNavMesh_H


#include "kypathdata/math/vec2f.h"

namespace Kaim
{

class PdgSystem;
class IndexedTriangleSoup;
class DynamicNavMesh;
class DynamicNavCell;
class DynamicNavFloor;
class Box3f;

class IndexedTriangleSoupToDynamicNavMesh
{
public:
	IndexedTriangleSoupToDynamicNavMesh(PdgSystem* sys = KY_NULL) : m_sys(sys) {}

	KyResult FillDynamicNavMeshFromIndexedTriangleSoup(const IndexedTriangleSoup& indexedTriangleSoup, DynamicNavMesh& dynamicNavMesh, KyFloat32 cellSize);

private:
	KyResult FillDynamicNavCellFromIndexedTriangleSoup(const IndexedTriangleSoup& indexedTriangleSoup, DynamicNavCell& dynamicNavCell);
	KyResult FillDynamicNavFloorFromIndexedTriangleSoup(const IndexedTriangleSoup& indexedTriangleSoup, DynamicNavFloor& dynamicNavFloor);

public:
	PdgSystem* m_sys;
};

}


#endif //KyPdg_IndexedTriangleSoupToDynamicNavMesh_H

