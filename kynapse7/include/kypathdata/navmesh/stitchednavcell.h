/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_StitchedNavCell_H
#define KyPathData_StitchedNavCell_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/navmesh/stitchednavmesh.h"

#include <string.h> // for memset

namespace Kaim
{

class NavCell;
class NavMeshStitchDataManager;
class NavHalfEdgePtr;


typedef KyUInt32 ConnectedComponentId;
static const ConnectedComponentId ConnectComponentTraversal_NEW = KyUInt32MAXVAL;
static const ConnectedComponentId ConnectComponentTraversal_OPEN = KyUInt32MAXVAL - 1;


class StitchedNavCell
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	StitchedNavCell(const NavCell* navCell) : m_parentStitchedNavMesh(KY_NULL),
		m_idxInTheBufferOfStitchedNavCell(KyUInt32MAXVAL), m_IsStitched(false), m_navCell(navCell), m_navFloorConnectedComponent(KY_NULL)
	{
		memset(m_links, 0, 4 * (KyUInt32)sizeof(void*));
		memset(m_numberOfLinks, 0, 4 * sizeof(KyUInt32));
	}

	NavMeshReference* GetNavMeshReference() const { return m_parentStitchedNavMesh->m_reference; }
	StitchedNavCellIdx GetStitchedNavCellIdx() const { return m_idxInTheBufferOfStitchedNavCell; }

	KY_FORCE_INLINE const NavCell& GetNavCell() const
	{
		KY_FUNCTION("StitchedNavCell::GetNavCell");
		KY_ASSERTK(m_navCell != KY_NULL);
		return *m_navCell;
	}

	KY_FORCE_INLINE bool IsStitched() const { return m_IsStitched; }

	const GuidCompound& GetGuidCompound() const { return m_parentStitchedNavMesh->GetGuidCompound(); }


public:
	StitchedNavMesh* m_parentStitchedNavMesh;
	StitchedNavCellIdx m_idxInTheBufferOfStitchedNavCell;
	bool m_IsStitched;
	const NavCell* m_navCell;

	ConnectedComponentId* m_navFloorConnectedComponent;

	KyUInt32 m_numberOfLinks[4];
	NavHalfEdgePtr* m_links[4];
};


}

#endif //KyPathData_StitchedNavCell_H

