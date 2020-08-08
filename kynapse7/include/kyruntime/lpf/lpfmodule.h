/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_lpf */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_LPFMODULE_H
#define KY_LPFMODULE_H

#include <kyruntime/lpf/ilpfmodule.h>

#include <kyruntime/graph/graphvertexsafeptr.h>
#include <kyruntime/graph/path.h>

namespace Kaim
{

class PathFinder;


/*!	This class is a concrete implementation of ILpfModule used internally by the PathFinder.
	Its usage is entirely transparent. You should not need to interact directly with this class.
	Most of the methods and data members of this class are intended for internal use. If you provide an \LpfContext in
	your call to PathFinder::Initialize(), the PathFinder transparently initializes and uses an instance of LpfModule, which
	you can access through PathFinder::GetLpfModule(). 
	Any interaction with local paths and \Obstacles in your own code should typically be done through the PathFinder interface.
	However, the LpfModule class does offer some data members that you can configure in order to control certain aspects of the
	Local PathFinding system, and some methods and members that provide information about the current state of the Local
	PathFinding system for the Bot. Only these members are exposed in this documentation.
	\pp See also :USERGUIDE:"Using Obstacles".
	\ingroup kyruntime_lpf */
class LpfModule: public ILpfModule
{
public:
	/* Used by LpfModule to represent edges in the spatial graph known by the Bot to be blocked by \Obstacles managed
		by the LpfManager. For internal use. */
	class LpfTabooEdge: public VirtualMemObject
	{
	public:
		LpfTabooEdge():
			m_edgeP1SafePtr(),
			m_edgeP2SafePtr(),
			m_blockingArea(KY_NULL),
			m_timeStamp(0.0f),
			m_passability(KY_FALSE) {}

		LpfTabooEdge(const GraphVertexSafePtr& p1SafePtr, const GraphVertexSafePtr& p2SafePtr, ILpfManager::LpfArea* area, KyFloat32 timeStamp, KyBool passability):
			m_edgeP1SafePtr(p1SafePtr),
			m_edgeP2SafePtr(p2SafePtr),
			m_blockingArea(area),
			m_timeStamp(timeStamp),
			m_passability(passability) {}

		~LpfTabooEdge()
		{
			m_edgeP1SafePtr.Invalidate();
			m_edgeP2SafePtr.Invalidate();
		}

	public:
		GraphVertexSafePtr m_edgeP1SafePtr;
		GraphVertexSafePtr m_edgeP2SafePtr;
		ILpfManager::LpfArea *m_blockingArea;
		KyFloat32 m_timeStamp;
		KyBool m_passability;
	};
	typedef List<LpfTabooEdge> LpfTabooEdgeList;

public:
	// Inherited interface
	// - core interface
	LpfModule(World& world, ILpfManager::LpfContext& lpfMgrContext, PathFinder* pathFinder);
	~LpfModule();

	virtual void ReInit();

	// - CanGo interface
	virtual KyBool CanGo(const PointWrapper& from, const PointWrapper& to);
	virtual KyBool CanGo_Polyline(Vec3f* points, const KyUInt32 pointCount,
		KyUInt32* collisionEdgeIndex);
	virtual KyBool CanGo_SelectLayerTypes(
		const PointWrapper& start, const PointWrapper& dest, KyUInt32 layersToIgnore);
	virtual KyBool CanGo_IgnoreLPFLayers(const PointWrapper& from, const PointWrapper& to);
	virtual KyBool TraceLine_SelectLayerTypes(
		const PointWrapper& start, const PointWrapper& dest,
		PointWrapper& lastValidPoint, KyUInt32 layersToIgnore);
	virtual KyBool GetNearestLayerLocation(
		const PointWrapper& start, const Vec3f& destPos, const NavMeshLayerQuery& layerQuery,
		NavMeshLayerLocation& layerLocation, Vec3f *lastValidPosition=KY_NULL);
	virtual KyBool GetAllLayerLocations( const PointWrapper& start, const Vec3f& destPos,
		NavMeshLayerQuery& layerQuery, NavMeshLayerLocationList& layerLocations);

	// - GetCost interface
	/* GetCost methods works on PathFinder-independent data and don't
	need to be overriden here.
	*/

	// - LocalPath computation interface
	virtual ILpfManager::LocalPathComputationResult GetLocalPath_Follow(
		const Vec3f &curPos, const Vec3f &destPos,
		ILpfManager::LpfArea* area,
		Vec3fList* localPath, KyBool& didShortCut);
	virtual ILpfManager::LocalPathComputationResult GetLocalPath_AStar(
		const Vec3f &curPos, const Vec3f &nextPos,
		LpfAStarData* curLpfData, ILpfManager::LpfArea* area,
		Vec3fList* localPath, KyFloat32& cost,
		LpfAStarData* nextNodeLpfData);

	// LocalPath following interface
	virtual KyBool CanLeave(Bot& entity);
	virtual KyBool Goto(const PointWrapper& targetPoint, Action& action);
	virtual KyBool CheckAccident(const Bot& entity, KyBool& accidentDetected);
	virtual KyUInt32 SkipPathNodes(KyUInt32 skipNodeCount);
	virtual void StopBot();

	// - Shortcut interface
	virtual KyBool GetShortcutCost(const Vec3f& from, const Vec3f& to, KyFloat32& cost);

	// - Other PathFinder module interface
	virtual Bot* GetBot();
	virtual Vec3f GetBotPosition();
	virtual KyBool GetPathfinderConstraintCost(const Vec3f& from, const Vec3f& to, KyFloat32& cost);
	virtual KyBool GetGotoCost(const PointWrapper& from, const PointWrapper& to, KyFloat32& cost);


	// PathFinder-specific

	// - LPF compatibility check

	/* This method checks if the bot can directly target "to"
	taking the LPF stuff into account.
	/param pos the arbitrary 3D position to check.
	/return KY_TRUE if the bot can navigate to "to" (directly or
	using a local path), KY_FALSE otherwise (there are some LPF obstacle that
	can't be bypassed on the way).
	*/
	KyBool CheckLpfCompatibility(const Vec3f& to);

	/* This method checks if the bot can use "cdtNode" as currentPathNode
	taking the LPF stuff into account.
	/param cdtNode the PathNode to check.
	/return KY_TRUE if the bot can use cdtNode as currentPathNode (directly or
	using a local path), KY_FALSE otherwise (there are some LPF obstacle that
	can't be bypassed on the way).
	*/
	KyBool CheckLpfCompatibility(PathNodeIterator& cdtNode);

	/* This method checks if the bot currentPathNode is still valid
	taking the LPF stuff into account.
	/return KY_TRUE if the bot can go to PathFinder::m_currentPathNode
	(directly or using a local path), KY_FALSE otherwise (there are some
	LPF obstacle that can't be bypassed on the way).
	*/
	KyBool CheckCurrentNodeCompatibility();


	// - LocalPath computation

	/*	This method computes, updates and follows the localPath the bot has
	to follow to avoid the dynamic obstacles laying on the global path.
	It is called each frame, and first checks if a new local path has to be
	computed, eventually computes it and calls FollowLocalPath function.
	*/
	LocalPathFindingState UpdateLocalPath();


	// - LocalPath follow

	/*	Updates the localPath following informations and computes
	m_lpfTargetPoint.
	*/
	LocalPathFindingState FollowLocalPath();

	/*	Releases Lpf following stuff. Two modes are allowed accordingly to the
	reason we reset LPF. Called with forceGlobalRecompute to KY_FALSE, we simply
	quit normally the local path and continue on the remainning part of the
	global path. When called with forceGlobalRecompute to KY_TRUE, we ask for a
	new global path computation and prepare the LPF for the intermediary period
	during which the new global path will be computed.
	*/
	void ReleaseLocalPath(KyBool forceGlobalRecompute=KY_FALSE);


	//- Taboo edge management and query

	/* Adds a graph edge into LpfTabooEdge list.
	\param blockingArea is the pointer on the LpfArea that blocks the edge
	\param edgeP1SafePtr the UID of the starting point of the edge to insert
	\param edgeP2SafePtr the UID of the end point of the edge to insert
	*/
	void AddLpfTabooEdge(ILpfManager::LpfArea *blockingArea, const GraphVertexSafePtr& edgeP1SafePtr, const GraphVertexSafePtr& edgeP2SafePtr);

	/* Removes, from LpfTabooEdge list, the edges that are no more impassable
	and not perceived as such.
	*/
	void RemoveUnlockedEdgesFromLpfTabooList();

	/* Get the passability status relativelly to Lpf has perceived by the bot.
	This checks if the given edge is in LpfTabooEdge. If so, it asks the
	EdgeAwareness modifier to retrieve the perceived status.
	\param edgeP1SafePtr the UID of the starting point of the edge to test
	\param edgeP2SafePtr the UID of the end point of the edge to test
	\return KY_TRUE if the edge is passable or of unknown status relativelly to
	LPF obstacles, KY_FALSE if the edge is blocked (or perceived as blocked) by
	LPF obstacles.
	*/
	KyBool GetLpfPerceivedEdgeStatus(const GraphVertexSafePtr& edgeP1SafePtr, const GraphVertexSafePtr& edgeP2SafePtr);


	// - Other utility methods
	/*! Retrieves the total distance left in the local path, measured from the current position of the Bot
		to the point at which the local path exits the aggreation of \Obstacles. */
	void GetDistanceLeft(Vec3f& currentPosition, KyFloat32& distance);

	/* Called transparently by PathFinder::Render(). */
	Vec3f Render(const Vec3f& botPos, PathNodeIterator currentNode, KyUInt32 mask);

public:
	PathFinder* m_pathFinder;
	IConstraint* m_lpfConstraint;

	LpfTabooEdgeList m_tabooEdges; 
	PathNodeIterator m_globalPath_exitNode;

	PathNodeIterator m_tmpCurNode;
	PathNodeIterator m_tmpNextNode;
};

} // namespace Kaim

#endif // KY_LPFMODULE_H
