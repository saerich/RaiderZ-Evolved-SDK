/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavMeshStitchDataManager_H
#define KyPathData_NavMeshStitchDataManager_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/navmesh/stitchednavmesh.h"
#include "kypathdata/navmesh/stitchednavcell.h"

namespace Kaim
{

class StitchedNavMesh;
class StitchedNavCell;
class NavMesh;
class NavCell;

class NavMeshStitchDataManager
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	NavMeshStitchDataManager(KyUInt32 bufferSizeAtInit = 50);
	~NavMeshStitchDataManager();

	void Init(KyUInt32 bufferSizeAtInit = 50);
	void Clear();
	void Reset() { Clear(); Init(); }

	StitchedNavMesh* AddNavMesh(const NavMesh& navMesh); //<! allocate memory and create a new StitchedNavMesh

	void RemoveNavMesh(StitchedNavMesh* stitchedNavMesh); //<! free memory of the StitchedNavMesh

	StitchedNavMesh* GetStitchedNavMesh(const NavMesh& navMesh); //<! retrieve the StitchedNavMesh corresponding to  navMesh

private:
	void RemoveNavMesh(KyUInt32 idx);

	bool ResizeStitchedNavMeshBuffer(KyUInt32 newSize = 20);

	void PlaceStitchedNavMeshInBuffer(const NavMesh& navMesh, StitchedNavMesh*& stitchNavMesh,  char*& currentMemoryPlace);
	void PlaceStitchedNavCellInBuffer(const NavCell* navCell, StitchedNavCell*& stitchNavCell,  char*& currentMemoryPlace);

public:
	StitchedNavMesh** m_bufferOfStitchedNavMesh;
	KyUInt32 m_numberOfStitchedNavMesh;
	KyUInt32 m_sizeOfStitchedNavMeshBuffer;
};


}

#endif //KyPathData_NavMeshStitchDataManager_H

