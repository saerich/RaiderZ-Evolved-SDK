/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_StitchedNavMesh_H
#define KyPathData_StitchedNavMesh_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/datareference.h"
#include "kypathdata/generic/referencemanager.h"
#include "kypathdata/navmesh/navmesh.h"

#include <string.h> // for memset

namespace Kaim
{

class StitchedNavMesh;

typedef DataReference<StitchedNavMesh> NavMeshReference;
typedef ReferenceManager<NavMeshReference> NavMeshReferenceManager;

class StitchedNavCell;

class StitchedNavMesh
{
	KY_DEFINE_NEW_DELETE_OPERATORS

	StitchedNavMesh& operator=(const StitchedNavMesh&);
public:
	StitchedNavMesh(const NavMesh& navMesh);

	KY_FORCE_INLINE void SetNavMeshReference(NavMeshReference& navMeshReference) { m_reference = &navMeshReference; }
	KY_FORCE_INLINE NavMeshReference* GetNavMeshReference() const { return m_reference; }

	const GuidCompound& GetGuidCompound() const { return m_navMesh.m_guidCompound; }
public:
	NavMeshReference* m_reference;
	KyUInt32 m_idxInTheBufferOfStitchedNavMesh;


	const NavMesh& m_navMesh;
	KyUInt32 m_numberOfCells;
	StitchedNavCell** m_stitchedCells;
};


}

#endif //KyPathData_StitchedNavMesh_H

