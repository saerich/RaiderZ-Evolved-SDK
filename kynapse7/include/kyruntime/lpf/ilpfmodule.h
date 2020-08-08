/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_lpf */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_ILPFMODULE_H
#define KY_ILPFMODULE_H

#include <kyruntime/lpf/ilpfmanager.h>

#include <kyruntime/core/action.h>
#include <kyruntime/core/pointwrapper.h>
#include <kyruntime/mesh/navmeshlayerquery.h>


namespace Kaim
{

class LpfAStarData;


/*!	ILpfModule represents an abstract base class for an object that can be used by the PathFinder to
	calculate "local Paths" that bypass aggregations of \Obstacles built by the LpfManager. 
	\pp This class cannot be used directly. See the derived LpfModule class.
	See also :USERGUIDE:"Using Obstacles".
	\ingroup kyruntime_lpf */
class ILpfModule: public VirtualMemObject
{
public:
	enum GetCostResult {
		GCR_Reachable,
		GCR_Unreachable,
		GCR_Unknown,
		GCR_DUMMY__ = KYFORCEENUMSIZEINT
	};

	enum LocalPathFindingState
	{
		LocalPathFindingState_DirectWay,
		LocalPathFindingState_ByPassNotFound,
		LocalPathFindingState_FollowingByPass,
		LocalPathFindingState_QuittingByPass,
		LocalPathFindingState_NoMoreValid,

		KY_DUMMY_LocalPathFindingState = KYFORCEENUMSIZEINT
	};


public:
	// Core interface
	explicit ILpfModule(World& world, ILpfManager::LpfContext& lpfMgrContext);
	~ILpfModule();
	virtual void ReInit();


	// CanGo interface
	virtual KyBool CanGo(const PointWrapper& from, const PointWrapper& to) = 0;

	virtual KyBool CanGo_Polyline(Vec3f* points, const KyUInt32 pointCount, KyUInt32* collisionEdgeIndex) = 0;

	virtual KyBool CanGo_SelectLayerTypes(
		const PointWrapper& start, const PointWrapper& dest, KyUInt32 layersToIgnore) = 0;

	virtual KyBool CanGo_IgnoreLPFLayers(const PointWrapper& from, const PointWrapper& to) = 0;

	virtual KyBool TraceLine_SelectLayerTypes(
		const PointWrapper& start, const PointWrapper& dest,
		PointWrapper& lastValidPoint, KyUInt32 layersToIgnore) = 0;

	/* Get nearest layerLocation in layerQuery between start and destPos ignoring NavMesh interior obstacles.
	Return KY_TRUE if the retrieval process did work (what ever there is actually an element LayerLocation or not);
	KY_FALSE otherwise (for example if the start position is outside the NavMesh). */
	virtual KyBool GetNearestLayerLocation(
		const PointWrapper& start, const Vec3f& destPos, const NavMeshLayerQuery& layerQuery,
		NavMeshLayerLocation& layerLocation, Vec3f *lastValidPosition=KY_NULL) = 0;

	/* Get the layerLocation of all elements encountered on the given segment in the layers specified in the layerQuery.
	Return KY_TRUE if the retrieval process did work,
	KY_FALSE otherwise (for example if the start position is outside the NavMesh). */
	virtual KyBool GetAllLayerLocations( const PointWrapper& start, const Vec3f& destPos,
		NavMeshLayerQuery& layerQuery, NavMeshLayerLocationList& layerLocations) = 0;


	// GetCost interface
	virtual GetCostResult GetCost_AStar(
		const Vec3f &curPos, const Vec3f &nextPos,
		LpfAStarData* currentSearchNodeData,
		KyFloat32 &cost, LpfAStarData* newSearchNodeData);


	// LocalPath computation interface
	virtual ILpfManager::LocalPathComputationResult GetLocalPath_Follow(
		const Vec3f &curPos, const Vec3f &destPos,
		ILpfManager::LpfArea* area,
		Vec3fList* localPath, KyBool& didShortCut) = 0;

	virtual ILpfManager::LocalPathComputationResult GetLocalPath_AStar(
		const Vec3f &curPos, const Vec3f &nextPos,
		LpfAStarData* curLpfData, ILpfManager::LpfArea* area,
		Vec3fList* localPath, KyFloat32& cost,
		LpfAStarData* nextNodeLpfData) = 0;


	// LpfContext interface
	/*! Indicates whether or not the specified position is inside an Obstacle that is currently recognized by
		the Local PathFinding system. */
	KyBool IsInsideLpfAreas(const Vec3f& pos);

	// LocalPath following interface
	/*! Indicates whether or not the Bot is currently following a local path that bypasses an aggregation of
		\Obstacles managed by the Local PathFinding system. */
	virtual KyBool IsFollowingLocalPath() {return (m_localPath_lpfArea != KY_NULL);}
	virtual KyBool GetLocalPathNode(Vec3f& localPathNode);
	virtual KyBool CanLeave(Bot& entity) = 0;
	virtual KyBool Goto(const PointWrapper& targetPoint, Action& action) = 0;
	virtual KyBool CheckAccident(const Bot& entity, KyBool& accidentDetected) = 0;
	virtual KyUInt32 SkipPathNodes(KyUInt32 skipNodeCount) = 0;
	virtual void StopBot() = 0;


	// Shortcut interface
	virtual KyBool GetShortcutCost(
		const Vec3f& from, const Vec3f& to, KyFloat32& cost) = 0;


	// Other PathFinder interface
	virtual Bot* GetBot() = 0;
	virtual Vec3f GetBotPosition() = 0;
	virtual KyBool GetPathfinderConstraintCost(const Vec3f& from, const Vec3f& to, KyFloat32& cost) = 0;
	virtual KyBool GetGotoCost(const PointWrapper& from, const PointWrapper& to, KyFloat32& cost) = 0;


	// Flag management
	void ResetComputationInhibition() {m_localPathComputationInhibitionDate = 0.0f;}
	KyBool DidChangeSinceLastComputation() {return m_localPath_computationDate < m_lpfMgrContext->GetLastMoveDate();}
	void ResetComputationDate() {m_localPath_computationDate = 0.0f;}
	void SetTimeStamp() {m_localPath_computationDate=m_lpfMgrContext->GetLastMoveDate();}
	KyBool ForceGlobalPathComputation() {return m_forceGlobalPathComputation;}
	void ResetFlag_ForcePathRecompute() {m_forceGlobalPathComputation = KY_FALSE;}

	/*!	\label_getworld */
	const World& GetWorld() const;

	/*!	\label_getworld */
	World& GetWorld();

	/*!	\label_getengine */
	Engine& GetEngine();

	/*!	\label_getengine */
	const Engine& GetEngine() const;

protected:
	World* m_world;

public:
	// LPF manager
	ILpfManager::LpfContext* m_lpfMgrContext;
	ILpfManager* m_lpfManager;

	// Current LocalPath management
	// - computation context
	Vec3f m_globalPath_currentNodePos; // The position of the GlobalPath targeted by the GlobalPathFinder when the current LocalPath has been computed
	KyFloat32 m_localPath_computationDate;
	// - LocalPath
	KyBool m_useShortcut;
	Vec3fList m_localPath;
	ILpfManager::LpfArea *m_localPath_lpfArea;
	// - following
	Vec3f m_localPath_currentNodePos; // The position of the next node targeted on the localpath
	Vec3f m_localPath_currentTargetPos; // The position on the localpath that is actually targeted (this position is on the segment ending to m_localPath_currentNodePos)
	KyFloat32 m_localPath_lastUpdateDate; // The date the local path has been updated last time
	// - localPath coherence control (used to retrieve previous LocalPath following status right after a new LocalPath computation)
	Vec3f m_localPath_lastNodePos;
	Vec3f m_localPath_lastTargetPos;

	// LocalPath computation control
	KyFloat32 m_localPathComputationInhibitionDate;

	// Flags
	KyBool m_localPathRecomputed;
	KyBool m_forceLpfDetectionOnGlobalPath;
	KyBool m_forceGlobalPathComputation;

	// Tmp for LPF compatibility computation
	Vec3fList m_tmpLocalPath;

	// Parameters
	/*! Determines the maximum distance that an Obstacle managed by the LpfManager can be from the Bot in order for that Obstacle
		to be taken into consideration when planning a path. 
		\pp \Obstacles outside of this radius will still be taken into account and avoided if the Bot encounters them while following
		its path. However, the limit helps to reduce CPU usage by ignoring \Obstacles during path calculation that are far enough away
		from the Bot that they may be out of date by the time the Bot reaches them. 
		\pp This parameter also controls the "omniscience" of the Bot: how much knowledge the Bot has about the current state of distant
		\Obstacles in the World. This can increase the realism of the system. 
		\units			meters
		\acceptedvalues	any positive value
		\defaultvalue	\c 25.0f */
	KyFloat32 m_awarenessRadius;

	/*! Determines the time interval between each successive calculation to determine whether or not the Bot can skip directly to a
		point farther along its local path, thereby smoothing and simplifying its trajectory.
		\pp Setting this parameter to a lower value may make your \Bots more responsive and "smarter" when going around \Obstacles
		managed by the LpfManager, but also increases CPU requirements.
		\units			seconds
		\acceptedvalues	any positive value
		\defaultvalue	\c 0.1f */
	KyFloat32 m_smoothingPeriod;

	/*! Determines the distance between successive candidate points when simplifying local paths. The first candidate
		tested is at a point on the local path at this distance from the current position of the Bot. Each successive
		candidate tested is at a point on the local path at this distance from the previous candidate. 
		\units			meters
		\acceptedvalues	any positive value
		\defaultvalue	\c 2.0f */
	KyFloat32 m_smoothingDistance;

	/*! Determines the maximum number of candidate points that may be tested while smoothing a local path. When this maximum
		value is reached, the PathFinder changes the target point of the Bot to the last valid candidate, and changes the current
		PathNode of the Bot to the PathNode immediately following the last valid candidate. 
		\pp A candidate is considered valid if the direct line from the current position of the Bot to the candidate point is not
		blocked by any obstacles in the static NavMesh, any blocking \Obstacles, or any blocking \PathObjects. 
		\acceptedvalues	any positive value
		\defaultvalue	\c 10 */
	KyUInt32 m_smoothingMaxNodeSkip;
};


/*!	Used by ILpfModule to maintain temporary data during path computations. For internal use.
	\ingroup kyruntime_lpf */
class LpfAStarData: public VirtualMemObject
{
public:
	LpfAStarData():
		m_timeStamp(0.0f),
		m_area(KY_NULL),
		m_entryPoint(Vec3f(KyFloat32MAXVAL, KyFloat32MAXVAL, KyFloat32MAXVAL)),
		m_plIt(),
		m_plEntryEdgeEndIt() {}

	void ReInit()
	{
		m_timeStamp = 0.0f;
		m_area = KY_NULL;
		m_entryPoint.x = KyFloat32MAXVAL;
		m_entryPoint.y = KyFloat32MAXVAL;
		m_entryPoint.z = KyFloat32MAXVAL;
		m_plIt.Invalidate();
		m_plEntryEdgeEndIt.Invalidate();
	}


public:
	KyFloat32 m_timeStamp;
	ILpfManager::LpfArea *m_area;
	Vec3f m_entryPoint;
	SharedPoolList<ILpfManager::LpfPolylinePtr>::Iterator m_plIt;
	SharedPoolList<ILpfManager::LpfPolylineNode>::Iterator m_plEntryEdgeEndIt;
};

} // namespace Kaim

#endif // KY_ILPFMODULE_H
