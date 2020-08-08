/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_EDGESTATUSAWARENESS_DISTANCE2DANDTIME_H
#define KY_EDGESTATUSAWARENESS_DISTANCE2DANDTIME_H

#include <kyruntime/pathfinding/iedgestatusawareness.h>


namespace Kaim
{
/*!	This implementation of the IEdgeStatusAwareness interface considers a previously
	blocked edge to be passable if either of the following conditions is true:
	-	The status of the \LpfArea blocking the edge is currently unknown, and any part 
		of the edge is closer to the current position of the Bot than a configurable distance
		threshold set through a call to AwarenessDist(KyFloat32).
	-	The status of the \LpfArea blocking the edge is currently unknown, and the elapsed 
		time since the status of the edge became unknown to the Bot exceeds a configurable time
		threshold set through a call to MemoryDuration(KyFloat32).
	\pp In all other circumstances, the edge is considered to be impassable, and is retained
	in the taboo edge list. This modifier never removes from the taboo edge list any edges that are 
	known to still be blocked.
	\ingroup kyruntime_pathfinding */
class EdgeStatusAwareness_Distance2dAndTime: public IEdgeStatusAwareness
{
public:
	KY_DECLARE_EDGESTATUSAWARENESS(EdgeStatusAwareness_Distance2dAndTime)


public:
	/*!	\label_constructor */
	EdgeStatusAwareness_Distance2dAndTime():
		IEdgeStatusAwareness(),
		m_awarenessDist(25.0f),
		m_memoryDuration(10.0f) {}

	virtual KyBool GetPerceivedEdgeStatus(const Bot &bot,
		const GraphVertexSafePtr& edgeP1UID, const GraphVertexSafePtr& edgeP2UID, KyBool realStatus,
		KyFloat32 statusChangeDate);


	/*! Retrieves the value set through a call to AwarenessDist(KyFloat32). */
	KyFloat32 AwarenessDist() const { return m_awarenessDist; }

	/*! Determines the radius within which the Bot is aware of previously blocked edges that become passable. 
		When an edge stored in the taboo edge list becomes passable, CEdgeStatusAwareness_Distance2dAndTime instructs the PathFinder to remove
		that edge from the taboo edge list if the edge is closer to the Bot than the value of this parameter.
		\acceptedvalues	any positive value
		\defaultvalue	\c 25.0f */
	void AwarenessDist(KyFloat32 val)
	{
		KY_FUNCTION("EdgeStatusAwareness_Distance2dAndTime::AwarenessDist");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'AwarenessDist' must be > 0.", val));
		m_awarenessDist = val; 
	}

	/*! Retrieves the value set through a call to MemoryDuration(KyFloat32). */
	KyFloat32 MemoryDuration() const { return m_memoryDuration; }

	/*! Determines the maximum length of time that the PathFinder can retain an unchanged passable edge in its taboo edge list. 
		Once an edge held in the taboo edge list becomes passable, the PathFinder removes that edge from the list if the elapsed time since the
		last time the LpfArea changed exceeds the value of this parameter.
		\acceptedvalues	any positive value
		\defaultvalue	\c 10.0f */
	void MemoryDuration(KyFloat32 val)
	{ 
		KY_FUNCTION("EdgeStatusAwareness_Distance2dAndTime::MemoryDuration");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'MemoryDuration' must be > 0.", val));
		m_memoryDuration = val; 
	}

protected:
	virtual void OnSetPathFinder() {}

	// Parameters
	KyFloat32 m_awarenessDist; //< The value set through AwarenessDistance().
	KyFloat32 m_memoryDuration; //< The value set through MemoryDuration().

};

} // namespace Kaim

#endif // KY_EDGESTATUSAWARENESS_DISTANCE2DANDTIME_H
