/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_ConnectedComponentNavFloorTraversal_H
#define KyPathData_ConnectedComponentNavFloorTraversal_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"


namespace Kaim
{

class NavMesh;

class ConnectedComponentNavFloorTraversal
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public :
	ConnectedComponentNavFloorTraversal() {}

public :
	void ReInit() { m_openNodes.Clear(); }

	void search();

	KY_FORCE_INLINE void AddStartFloor(const NavFloorPtr& navFloorPtr)
	{
		navFloorPtr.SetConnectedComponentId(ConnectComponentTraversal_OPEN);
		m_openNodes.PushBack(navFloorPtr);
	}

	KY_FORCE_INLINE void SetCurrentComponentId(ConnectedComponentId currentConnectedComponentId)
	{
		m_currentConnectedComponentId = currentConnectedComponentId;
	}

private:

	KY_FORCE_INLINE void OpenNodeIfNew(const NavFloorPtr& navFloorPtr)
	{
		ConnectedComponentId& connectedComponentId = navFloorPtr.GetConnectedComponentId();

		if (connectedComponentId == ConnectComponentTraversal_NEW)
		{
			connectedComponentId = ConnectComponentTraversal_OPEN;
			m_openNodes.PushBack(navFloorPtr);
		}
	}

	KY_FORCE_INLINE void OpenNodeIfNew(const StitchedNavCell& stitchedNavCell, NavFloorIdx navFloorIdx)
	{
		ConnectedComponentId& connectedComponentId = stitchedNavCell.m_navFloorConnectedComponent[navFloorIdx];

		if (connectedComponentId == ConnectComponentTraversal_NEW)
		{
			connectedComponentId = ConnectComponentTraversal_OPEN;
			m_openNodes.PushBack(NavFloorPtr(stitchedNavCell, navFloorIdx));
		}
	}

public:
	List<NavFloorPtr> m_openNodes;
	ConnectedComponentId m_currentConnectedComponentId;
};

void ConnectedComponentNavFloorTraversal::search()
{
	NavFloorPtr currentFloorPtr;
	List<NavFloorPtr>::Iterator firstIterator = m_openNodes.GetFirst();
	while (m_openNodes.IsEmpty() == false)
	{
		currentFloorPtr = *firstIterator;
		m_openNodes.Erase(firstIterator);

		currentFloorPtr.SetConnectedComponentId(m_currentConnectedComponentId);

		const StitchedNavCell& stitchedNavCell = currentFloorPtr.GetStitchedNavCell();
		const NavFloor& currentFloor = currentFloorPtr.GetNavFloor();

		const KyUInt32 floorLinkCount = currentFloor.m_floorLinks.m_count;
		const FloorLink* floorLinks = currentFloor.m_floorLinks.Values();

		const KyUInt32 edgeCount = currentFloor.GetHalfEdgeCount();
		const NavHalfEdge* navHalfEdges = currentFloor.m_navHalfEdges.Values();

		for (KyUInt32 floorLinkIdx = 0; floorLinkIdx < floorLinkCount; ++floorLinkIdx )
		{
			const FloorLink& floorLink = floorLinks[floorLinkIdx];
			OpenNodeIfNew(stitchedNavCell, floorLink.m_floorIdx);
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

#endif // KyPathData_ConnectedComponentNavFloorTraversal_H

