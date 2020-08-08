/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// Primary contact: LASI - secondary contact: GUAL
#ifndef KyPdg_NavMeshBlobBuilder_H
#define KyPdg_NavMeshBlobBuilder_H


#include <kypathdata/blob/baseblobbuilder.h>
#include <kypathdata/navmesh/navmesh.h>
#include <kypdg/common/stlvector.h>


namespace Kaim
{

class PdgSystem;
class DynamicNavCell;
class PdgGuidCompound;


class NavMeshBlobBuilder : public BaseBlobBuilder<NavMesh>
{
public:
	NavMeshBlobBuilder(PdgSystem* sys, const PdgGuidCompound* guidCompound, const CellBox& cellBox, const StlVector<DynamicNavCell*>* dynamicNavCells, NavMesh::NavMeshFlags flags = NavMesh::NAVMESH_NO_FLAG);

private:
	virtual void DoBuild();

private:
	PdgSystem* m_sys;
	const PdgGuidCompound* m_guidCompound;
	CellBox m_cellBox;
	const StlVector<DynamicNavCell*>* m_dynamicNavCells;
	NavMesh::NavMeshFlags m_navmeshFlags;
};


}


#endif
