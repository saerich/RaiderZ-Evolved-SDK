/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_PdgConnectComponentTraversal_H
#define KyPathData_PdgConnectComponentTraversal_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypdg/generator/pdgpathdatafilter.h"
#include "kypathdata/navmesh/navfloorptr.h"
#include "kypathdata/navmesh/navhalfedgeptr.h"

namespace Kaim
{

class NavMesh;

class PdgConnectComponentTraversal
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public :
	PdgConnectComponentTraversal(PdgPathDataFilter::NavFloorConnectedComponentIdTable& navFloorConnectedComponentIdTable) : m_navFloorConnectedComponentIdTable(&navFloorConnectedComponentIdTable){}

public :
	void ReInit() { m_openNodes.Clear(); }

	void Search();

	KY_FORCE_INLINE void AddStartFloor(const NavFloorPtr& navFloorPtr)
	{
		const NavCellIdx cellIdx = navFloorPtr.GetStitchedNavCell().GetStitchedNavCellIdx();
		const NavFloorIdx floorIdx = navFloorPtr.GetFloorIdx();
		(*m_navFloorConnectedComponentIdTable)[cellIdx][floorIdx] = m_currentComponentId;
		m_openNodes.PushBack(navFloorPtr);
	}

	KY_FORCE_INLINE void SetCurrentComponentId(PdgConnectedComponentId currentComponentId)
	{
		m_currentComponentId = currentComponentId;
	}


private:

	KY_FORCE_INLINE void OpenNodeIfNew(const NavFloorPtr& navFloorPtr)
	{
		const NavCellIdx cellIdx = navFloorPtr.GetStitchedNavCell().GetStitchedNavCellIdx();
		const NavFloorIdx floorIdx = navFloorPtr.GetFloorIdx();

		PdgConnectedComponentId& connectedComponentId = (*m_navFloorConnectedComponentIdTable)[cellIdx][floorIdx];

		if (connectedComponentId == PdgConnectComponentTraversal_NEW)
		{
			connectedComponentId = m_currentComponentId;
			m_openNodes.PushBack(navFloorPtr);
		}
	}

public:
	List<const NavFloorPtr> m_openNodes;
	PdgConnectedComponentId m_currentComponentId;
	PdgPathDataFilter::NavFloorConnectedComponentIdTable* m_navFloorConnectedComponentIdTable;
};

void PdgConnectComponentTraversal::Search()
{
	NavFloorPtr currentFloorPtr;
	List<const NavFloorPtr>::Iterator firstIterator = m_openNodes.GetFirst();
	while (m_openNodes.IsEmpty() == false)
	{
		currentFloorPtr = *firstIterator;
		m_openNodes.Erase(firstIterator);

		const StitchedNavCell& stitchedNavCell = currentFloorPtr.GetStitchedNavCell();
		const NavFloor& currentFloor = currentFloorPtr.GetNavFloor();

		const KyUInt32 floorLinkCount = currentFloor.m_floorLinks.m_count;
		const FloorLink* floorLinks = currentFloor.m_floorLinks.Values();

		const KyUInt32 edgeCount = currentFloor.GetHalfEdgeCount();
		const NavHalfEdge* navHalfEdges = currentFloor.m_navHalfEdges.Values();

		for (KyUInt32 floorLinkIdx = 0; floorLinkIdx < floorLinkCount; ++floorLinkIdx )
		{
			const FloorLink& floorLink = floorLinks[floorLinkIdx];
			OpenNodeIfNew(NavFloorPtr(stitchedNavCell, floorLink.m_floorIdx));
		}

		for (NavHalfEdgeIdx edgeIdx = 0; edgeIdx < edgeCount; ++edgeIdx)
		{
			const NavHalfEdge& halfEdge = navHalfEdges[edgeIdx];

			if (halfEdge.GetHalfEdgeType() != CELL_BOUNDARY)
				continue;

			// edgeType == CELL_BOUNDARY
			const CardinalDir dir = halfEdge.GetCellBoundaryDir();
			const KyUInt32 linkIdx = halfEdge.GetCellLinkIdx();

			NavHalfEdgePtr& edgeLinkedPtr = stitchedNavCell.m_links[dir][linkIdx];
			if (!edgeLinkedPtr.IsValid())
				continue;

			OpenNodeIfNew(edgeLinkedPtr.GetNavFloorPtr());
		}

		firstIterator = m_openNodes.GetFirst();
	}
}


}

#endif // KyPathData_PdgConnectComponentTraversal_H

