/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/




/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_PATHFINDER_H
#define KY_PATHFINDER_H


#include <kyruntime/core/inextmovemanager.h>
#include <kyruntime/graph/graphconstraints.h>
#include <kyruntime/lpf/lpfmodule.h>
#include <kyruntime/mesh/navmeshlayerlocation.h>
#include <kyruntime/mesh/pointinfo_navtrianglesafeptr.h>
#include <kyruntime/pathobjects/runtimepathobjecttopology/pointinfo_connexionspace.h>
#include <kyruntime/pathobjects/runtimepathobjecttopology/pointinfo_controlspace.h>
#include <kyruntime/pathfinding/modifiers.h>
#include <kyruntime/pathfinding/smoothablepathobjecttraverseinfo.h>


namespace Kaim
{
// interface to customization modifiers
class IRefineGoal;
class IDetectGoalReached;
class IDetectGoalChanged;
class IDetectAccident;
class IDetectPathNodeReached;
class ISelectPathNodeCandidate;
class IComputeTargetPoint;
class ICheckDirectWay;
class ICanGo;
class IGoto;
class ISteering;
class IFindNodesFromPositions;
class ILpfShortcut;
class IEdgeStatusAwareness;

class RefineGoalClass;
class DetectGoalReachedClass;
class DetectGoalChangedClass;
class DetectAccidentClass;
class DetectPathNodeReachedClass;
class SelectPathNodeCandidateClass;
class ComputeTargetPointClass;
class CheckDirectWayClass;
class CanGoClass;
class GotoClass;
class SteeringClass;
class FindNodesFromPositionsClass;
class LpfShortcutClass;
class EdgeStatusAwarenessClass;

class Modifiers;

class LpfModule;
class PathObjectManager;
class AstarContext;

/*!	Returned by the PathFinder::GetLastError() method to provide details about the last
	error encountered by the PathFinder. 
	\ingroup kyruntime_pathfinding */
enum PathFinderError
{
	/*!	Indicates that the PathFinder is operating as expected, and that no error occurred 
		during the last path calculation. */
	PathFinderError_None,
	
	/*!	Indicates that the PathFinder could not find a vertex reachable from 
		the start point of the last path calculation (the position of the Bot). If this error occurs, 
		you may be able to find more information by calling the 
		IFindNodesFromPositions::GetLastErrors_Start() method of the IFindNodesFromPositions modifier
		used by the PathFinder. */
	PathFinderError_NoNode_StartPoint,
	
	/*!	Indicates that the PathFinder could not find a vertex reachable from 
		the destination point of the last path calculation. If this error occurs, you may be able to find 
		more information by calling the IFindNodesFromPositions::GetLastErrors_Dest() method of 
		the IFindNodesFromPositions modifier used by the PathFinder. */
	PathFinderError_NoNode_Destination,
	
	/*!	Indicates that the PathFinder could not find a path to the destination point. Check that 
		your start point and destination do not lie in disconnected regions of the GraphManager, and that 
		the edges between the points are not entirely blocked by \PathObjects, \LpfAreas, or the 
		IConstraint::GetCost() method of your pathfinding Constraint. */
	PathFinderError_NoPath,
	
	/*!	Indicates that a problem exists with a pre-calculated path injected into the PathFinder. */
	PathFinderError_InvalidInjectedPath,
	
	/*!	Indicates that the PathFinder has encountered an unexpected internal error. */
	PathFinderError_InternalError,

	KY_DUMMY_PathFinderError = KYFORCEENUMSIZEINT
};

enum PathFinderFindStartNodeStatus
{
	PathFinderFindStartNodeStatus_Idle = 0,
	PathFinderFindStartNodeStatus_POs,
	PathFinderFindStartNodeStatus_Graph,
	PathFinderFindStartNodeStatus_dummy_forceenumsize = KYFORCEENUMSIZEINT
};

enum PathFinderFindStartNodeUser
{
	PathFinderFindStartNodeUser_None = 0,
	PathFinderFindStartNodeUser_ComputeNodes,
	PathFinderFindStartNodeUser_GetNearestValidPoint,
	PathFinderFindStartNodeUser_dummy_forceenumsize = KYFORCEENUMSIZEINT
};

enum DetectAccidentStatus
{
	DetectAccident_DeferDetection = 0,
	DetectAccident_AccidentDetected,
	DetectAccident_NoAccident,
	DetectAccident__dummy_forceenumsize = KYFORCEENUMSIZEINT
};

/*!	Returned by the PathFinder::EvaluateShortcut() method to provide details about the result of its computation. 
	\ingroup kyruntime_pathfinding */
enum EvaluateShortcutResult
{
	EvaluateShortcut_NoCurrentPathNode = 0, /*!< Indicates that the starting PathNode is not valid. */
	EvaluateShortcut_InvalidLastNode, /*!< Indicates that the ending PathNode is not valid, or not found in the same Path as the starting PathNode. */
	EvaluateShortcut_PathCostNotComputable, /*!< Indicates that the cost of following the path between the two \PathNodes could not be computed. */
	EvaluateShortcut_DirectCostNotComputable, /*!< Indicates that the cost of following the path between the two specified points could not be computed. */
	EvaluateShortcut_DirectShorter, /*!< Indicates that the cost of following the straight line is less than that of following the path through the Graph. */
	EvaluateShortcut_DirectLonger, /*!< Indicates that the cost of following the path through the Graph is less than that of following the straight line. */
	EvaluateShortcut__dummy_forceenumsize = KYFORCEENUMSIZEINT
};


/*!	The PathFinder provides a Bot with the ability to move to a specified destination point. 
	\pp Each Bot maintains an instance of the PathFinder in its Bot::m_pathFinder class member. You must
	call PathFinder::Initialize() to set up this PathFinder instance after you add your Bot to a World.
	\pp Each frame, you call the FindNextMove() method to provide your desired destination point.
	\pp At first, the PathFinder uses the spatial graph that you have loaded into a 
	Database to compute an initial rough path to the destination point. This calculation may be sliced
	over multiple frames if necessary. In the following frames, once the initial path through the 
	spatial graph has been calculated, a set of modifiers are invoked to make the Bot follow the 
	path. Each of these modifiers performs a specific activity, such as refining and smoothing the 
	path to produce more realistic trajectories, testing that the desired destination has not 
	changed, testing for accidents, etc. These modifiers often rely on the NavMesh loaded into the
	Database.
	\pp Finally, the PathFinder generates an Action intended to move the Bot along its path, which it
	stores in the #m_action member. If the FindNextMove() method returns #KY_TRUE, you can copy this 
	Action to the \c m_action member of your Bot.
	\pp For details, see :USERGUIDE:"PathFinding".
	\ingroup kyruntime_pathfinding */
class PathFinder : public Kaim::IPathFinder
{
public:
	static const Vec3f INVALID_POSITION; /*!< Used by the PathFinder to represent an invalid position in 3D space. */

public:
	/*!	\label_constructor For internal use. */
	PathFinder(Kaim::Bot * bot);
	
	/*!	\label_destructor */
	~PathFinder();

	/*!	\label_init
		\param modifiers				An instance of the Modifiers class that provides the PathFinder with the set of
										IModifier objects it should use during its path calculation and path following operations.
		\param database					The Database that the PathFinder will use to retrieve the PathData it should use during its
										path calculation and path following operations.
		\param taskId_ComputePath		The unique ID of an aperiodic task configured for the TimeManager that the PathFinder will consult
										when it needs to compute a new path, in order to determine whether or not time remains in the frame
										for the path computation. 
		\param taskId_PathFollowing		The unique ID of an aperiodic task configured for the TimeManager that the PathFinder will consult
										at specific moments during path following, in order to determine whether or not time remains in the
										frame for time-intensive computations.
		\param lpfMgrContext			An \LpfContext configured for the LpfManager that the PathFinder will use to retrieve dynamic
										\Obstacles that it must avoid. See :USERGUIDE:"Using Obstacles". */
	KyResult Initialize(Modifiers& modifiers, Kaim::Database& database, TimeManager::TaskId taskId_ComputePath, TimeManager::TaskId taskId_PathFollowing = TimeManager::InvalidTaskId, ILpfManager::LpfContext* lpfMgrContext=KY_NULL);

	/*! \label_clear */
	void Clear();

	/*! \label_reinit */
	virtual void ReInit();

	//////////////////////////////////////////////////////////////////////////////
	// IPathFinder Interface
	//////////////////////////////////////////////////////////////////////////////

	virtual KyBool FindNextMove(const PointWrapper& dest);

	virtual KyFloat32 GetDistanceLeft();

	virtual PathStatus GetPathStatus() const { return m_pathStatus; }

	/*!	\copydoc IPathFinder::CanGo()
		\pp This implementation of IPathFinder::CanGo() simply calls the ICanGo::CanGo() method of the ICanGo
		modifier currently set for the PathFinder. */
	KyBool CanGo(const PointWrapper& from, const PointWrapper& to);
	KyBool HasArrived() const { return m_goalReached; }

	/*!	\copydoc IPathFinder::GetHeuristicPathCost()
		\pp This implementation of IPathFinder::GetHeuristicPathCost() simply calls the IHeuristic::GetCost() 
		method of the IHeuristic currently set for the PathFinder. */
	KyBool GetHeuristicPathCost(KyFloat32& cost, const PointWrapper& from, const PointWrapper& to);

	/*!	\copydoc IPathFinder::GetNearestReachableValidPoint()
		\pp If the PathFinder has a current PathObject that implements the PathObject::FindStartNode() 
		virtual method, this implementation of GetNearestReachableValidPoint() simply returns the vertex returned 
		by PathObject::FindStartNode(). Otherwise, this method calls the IFindNodesFromPositions::FindStartNode() 
		method of the IFindNodesFromPositions modifier currently in use by the PathFinder. */
	GetValidPointStatus GetNearestReachableValidPoint(const Vec3f& startPosition, PointWrapper& validPoint);


	//////////////////////////////////////////////////////////////////////////////
	// Heuristics and constraints
	//////////////////////////////////////////////////////////////////////////////

	/*! Sets a bitmask that identifies the terrain types that the PathFinder will consider to be walkable. */
	void SetAllowedTerrainTypeMask(NavTerrainTypeMask mask) { m_allowedTerrainTypes = mask; }

	/*! Allows the PathFinder to traverse all different kinds of terrain types contained in the PathData. */
	void AllowedAllTerrainTypes() { m_allowedTerrainTypes = NavTerrainTypeMask_All; }

	/*! Clears the list of terrain types that the PathFinder considers navigable. */
	void ClearAllowedTerrainType() { m_allowedTerrainTypes = NavTerrainTypeMask_None; }

	/*! Adds the specified terrain type to the list of terrain types that the PathFinder considers to be navigable. */
	void AllowTerrainType(NavTerrainType terrainType) { m_allowedTerrainTypes = m_allowedTerrainTypes | terrainType; }

	/*! Removes the specified terrain type to the list of terrain types that the PathFinder considers to be navigable. */
	void ForbidTerrainType(NavTerrainType terrainType) { m_allowedTerrainTypes = NavTerrainTypeMask_All - ((NavTerrainTypeMask_All-m_allowedTerrainTypes) | terrainType); }

	/*! Retrieves a bitmask that identifies the list of terrain types that the PathFinder considers to be navigable. */
	NavTerrainTypeMask GetAllowedTerrainTypes() const { return m_allowedTerrainTypes; }


	//////////////////////////////////////////////////////////////////////////////
	// Path planning
	//////////////////////////////////////////////////////////////////////////////

	/*!	Begins injecting a pre-computed path into the PathFinder. 
		This method should be followed by a loop of calls to InjectPath_AddNode() to add each
		PathNode, and a call to InjectPath_End() to complete the process.
		\return #KY_TRUE if the path injection process was successfully initiated, or #KY_FALSE 
				if it is not possible to inject a path right now. #KY_FALSE typically indicates
				that the either the current PathObject or the Local PathFinding system does not 
				allow the Bot to leave its current path. */
	KyBool InjectPath_Begin();

	/*!	Adds a PathNode to the end of the path currently being injected. This
		method must be preceded by a call to InjectPath_Begin(). When you have finished adding
		PathNodes to the injected path, you must call InjectPath_End().
		\param node		The PathNode to be appended to the path. Any PathNode that does not
						lie at the same position as a vertex (often the case
						for the final destination point) should have its GraphVertexSafePtr set to
						an invalid value.
		\return #KY_TRUE if the PathNode was successfully appended to the injected path, or #KY_FALSE
				otherwise. */
	KyBool InjectPath_AddNode(const PathNode& node);

	/*!	Ends the path injection process.
		\param autoRecompute	When set to #KY_TRUE, allows the PathFinder to automatically 
								overwrite the injected path with a newly calculated path to the destination
								when an accident is detected and/or when the state of an edge of the 
								injected path has changed (a new PathObject is linked to the edge or
								an existing PathObject is unlinked from the edge). See also
								InjectPath_PathEventWasRaised().
		\return #KY_TRUE if the path has been successfully injected, or #KY_FALSE otherwise. */
	KyBool InjectPath_End(KyBool autoRecompute=KY_TRUE);

	/*!	Indicates that an event has occurred that may invalidate the injected path currently being
		followed by the PathFinder. For example:
		- 	An accident was detected.
		-	A PathObject linked to an edge in the path changed.
		-	The ForcePathComputation() method was invoked.
		If the InjectPath_End() method was called with the \c autoRecompute argument set to #KY_TRUE (the
		default), this method is not called. Instead, the path is automatically re-calculated, and the 
		injected path is overwritten.
		\pp If the InjectPath_End() method was called with the \c autoReceompute argument set to #KY_FALSE, the 
		injected path is not recomputed. Instead, you must call InjectPath_PathEventWasRaised() to determine 
		whether or not a problem has been detected with the injected path. For example, if this method returns
		#KY_TRUE, you could inject a different path, send the Bot to a different destination, or stop the
		Bot.
		\return #KY_TRUE if an event was raised while following an injected path, or #KY_FALSE if the Bot is
				following the injected path normally. */
	KyBool InjectPath_PathEventWasRaised() const { return m_pathEventRaised; }

	/*!	Indicates whether or not the PathFinder is currently following an injected path. 
		\pp Note that having successfully injected a path does not guarantee that the PathFinder is still
		following that injected path. Under some circumstances the PathFinder may abandon an injected
		path; for details, see InjectPath_End() and InjectPath_PathEventWasRaised().
		\return #KY_TRUE if the PathFinder is following an injected path, or #KY_FALSE otherwise.
	*/
	KyBool IsUsingAnInjectedPath() const { return m_useInjectedPath; }

	/*!	Indicates whether or note the path maintained by the PathFinder has been recomputed within 
		the current frame. */
	KyBool CurrentPathChanged() const;

	/*!	Retrieves the timestamp of the frame in which the current path maintained by the PathFinder
		was computed. */
	KyFloat32 GetPathTimeStamp() const { return m_pathTimeStamp; }

	/*!	Forces the PathFinder to recompute a new path to the destination point as soon as possible. 
		\pp Note that the path is not guaranteed to be re-computed immediately, due to limitations on the time per
		frame allotted to the World and the PathFinder. You may have to wait several frames before the Bot using the PathFinder
		Service is given priority, and before the path calculation is completed. */
	void ForcePathComputation();


	//////////////////////////////////////////////////////////////////////////
	// Accessors
	//////////////////////////////////////////////////////////////////////////

	/*!	When the PathFinder is unable to find a path to the destination point, it periodically
		re-tries the path calculation. This method retrieves the minimum time interval that must elapse 
		between successive path recomputations when a path cannot be found. */
	KyFloat32 GetAStarNewAttemptPeriod() const { return m_astarNewAttemptPeriod; }

	/*!	Sets the minimum time interval that must elapse between successive path recomputations when a
		path cannot be found. */
	void SetAStarNewAttemptPeriod(KyFloat32 seconds) { m_astarNewAttemptPeriod = seconds; }

	/*!	Retrieves the date of the last time the PathFinder attempted to re-calculated its path
		following a failed path calculation. */
	KyFloat32 GetLastAStarAttemptDate() {return m_lastTime_AStarAttempt;}

	/*!	Read accessor for the \c NbEdgesPerAstar configuration parameter. */
	KyUInt32 GetMaxEdgesPerAstar() const { return m_nbEdgesPerAstar; }

	/*!	Write accessor for the \c NbEdgesPerAstar configuration parameter. */
	void SetMaxEdgesPerAstar(KyUInt32 nbEdgesPerAstar) { m_nbEdgesPerAstar = nbEdgesPerAstar; }

	const GraphManager* GetGraphManager() const { return m_graphManager; }
	GraphManager* GetGraphManager() { return m_graphManager; }


	// Modifiers

	/*!	Retrieves the instance of the specified class of IModifier currently in use by the PathFinder. For all classes
		of IModifier that are considered mandatory by the PathFinder. See the constructor of the Modifiers class.
		\tparam TModifier			The name of the interface to retrieve. For instance, ICanGo, IGoto, etc. */
	template <class TModifier>
	TModifier& GetModifier()
	{
		return static_cast<TModifier&>(this->GetMandatoryModifier(TModifier::Class));
	}


	/*!	Retrieves the instance of the specified class of IModifier currently in use by the PathFinder. For all classes
		of IModifier that are considered optional by the PathFinder. See the constructor of the Modifiers class.
		\tparam TModifier			The name of the interface to retrieve. For instance, ILpfShortcut, IEdgeStatusAwareness, etc.
		\return The instance of the specified class, or #KY_NULL if no instance of the specified class is used by the PathFinder. */
	template <class TModifier>
	TModifier* GetOptionalModifier()
	{
		return static_cast<TModifier*>(this->GetOptionalModifier(TModifier::Class));
	}

	//////////////////////////////////////////////////////////////////////////////
	// Path Following
	//////////////////////////////////////////////////////////////////////////////

	/*! Retrieves the refined destination calculated by the IRefineGoal modifier. */
	const Vec3f& GetRefinedGoal() const { return m_refinedGoal.GetPosition(); }

	/*! \copydoc GetRefinedGoal() */
	const PointWrapper& GetRefinedGoalWrapper() const { return m_refinedGoal; }

	/*! Indicates whether or not an accident has been detected in the current frame. This flag is reset to 
		#KY_FALSE at the end of FindNextMove(). Therefore, you should use this method only during the 
		execution of FindNextMove(), such as from within a custom modifier class. As an alternative, you can 
		call GetLastAccidentDate() to retrieve the timestamp of the last frame in which an accident was detected, 
		and compare that timestamp to the timestamp of the current frame. */
	KyBool IsAccidentDetected() const { return m_accidentDetected; }

	/*!	Retrieves the exact timestamp at which the PathFinder last detected an accident. 
		If GetLastAccidentDate() returns a value that matches GetWorld().GetAccumulatedTimeInUpdate(), 
		an accident was detected within the current frame. */
	KyFloat32 GetLastAccidentDate() const { return m_lastAccidentDate; }

	/*!	Indicates whether or not the current target point is valid. If this method returns #KY_FALSE, 
		do not rely on the position returned by GetTargetPoint() or GetTargetPointWrapper(). */
	KyBool IsTargetPointValid() const { return m_targetPointValid; }

	/*!	Retrieves the current target point: the position currently aimed at by the PathFinder. 
		Before using this position, check its validity by a call to IsTargetPointValid(). */
	const Vec3f& GetTargetPoint() const { return m_targetPoint.GetPosition(); }

	/*!	\copydoc GetTargetPoint() */
	const PointWrapper& GetTargetPointWrapper() const { return m_targetPoint; }

	/*!	Retrieves the position of the current PathNode being targeted by the PathFinder. */
	const Vec3f& GetCurrentPathNodePosition() const { return m_pathNodePosition; }

	/*!	Indicates whether or not the PathFinder was granted CPU time for path following during
		the current frame. */
	KyBool DidGrantCpuForPathFollowing() const { return m_grantCpuForPathFollowing; }

	/*!	Retrieves the base period between successive attempts to select a new PathNode. */
	KyFloat32 GetPathNodeUpdatePeriod() const { return m_pathNodeUpdatePeriod; }

	/*!	Sets the base period between successive attempts to select a new PathNode. 
		This base period is doubled if the PathFinder attempts to select a new PathNode 
		when no time remains for path following in the current frame, in order to share CPU time 
		more evenly with other \Bots. */
	void SetPathNodeUpdatePeriod(KyFloat32 seconds) { m_pathNodeUpdatePeriod = seconds; }

	/*!	Retrieves the base period between successive attempts to detect whether or not an accident has
		occurred. */
	KyFloat32 GetAccidentDetectionPeriod() const { return m_accidentDetectionPeriod; }

	/*!	Sets the base period between successive attempts to detect whether or not an accident has
		occurred. This base period is doubled if the PathFinder attempts to detect accidents 
		when no time remains for path following in the current frame, in order to share CPU time 
		more evenly with other \Bots. */
	void SetAccidentDetectionPeriod(KyFloat32 seconds) { m_accidentDetectionPeriod = seconds; }

	/*!	Returns the current PathNode being targeted by the PathFinder.*/
	PathNodeIterator GetCurrentPathNode() const { return m_currentPathNode; }

	/*!	Returns the UID of the last PathNode targeted by the PathFinder.*/
	const GraphVertexSafePtr& GetPreviousPathNodeUID() const { return m_previousPathNodeSafePtr; }

	/*!	Evaluates the costs of following two possible trajectories:
		-#	Moving from \c firstPosition to \c lastPosition in a straight line.
		-#	Moving from \c firstPathNode to \c LastPathNode along a path through the edges of the Graph.
		\pp The costs are evaluated in order to determine whether or not the straight-line trajectory
		represents a shortcut in comparison to the path through the Graph.
		\param firstPosition			The starting point for the potential shortcut.
		\param firstPathNode			The first PathNode in the path through the Graph.
		\param lastPathNode				The last PathNode in the path through the Graph.
		\param lastPosition				The ending point for the potential shortcut.
		\param[out] out_pathCost		The cost of following the path through the Graph from \c firstPathNode to \c lastPathNode.
		\param[out] out_directCost		The cost of moving in a straight line from \c firstPosition to \c lastPosition.
		\return An element from the #Kaim::EvaluateShortcutResult enumeration that indicates the result of its calculations. */
	EvaluateShortcutResult EvaluateShortcut(
		const PointWrapper& firstPosition, const PathNodeIterator& firstPathNode,
		const PathNodeIterator& lastPathNode, const PointWrapper& lastPosition,
		KyFloat32* out_pathCost=KY_NULL, KyFloat32* out_directCost=KY_NULL);


	//////////////////////////////////////////////////////////////////////////////
	// Local path finding
	//////////////////////////////////////////////////////////////////////////////

	/*!	Retrieves the list of \ObstacleLayers used by the LpfManager to build the \LpfAreas contained in the
		\LpfContext used by the PathFinder. */
	INavMeshLayerPtrList* GetLpfLayers();

	/*!	Read accessor for the \c LpfModule/AStarRadius configuration parameter. */
	KyFloat32 GetLpfRadius();

	/*!	Retrieves the LpfModule instance used by the PathFinder to calculate local paths. */
	LpfModule* GetLpfModule() {return m_lpfModule;}
	
	/*!	Retrieves the LpfModule instance used by the PathFinder to calculate local paths. */
	const LpfModule* GetLpfModule() const {return m_lpfModule;}

	/*!	Returns the LpfConstraint in use by the PathFinder. 
		If the PathFinder is not correctly configured to use Local PathFinding, this method instead 
		returns the current IConstraint in use by the PathFinder, as the GetConstraint() method. */
	IConstraint* GetLpfConstraint();


	//////////////////////////////////////////////////////////////////////////////
	// Misc
	//////////////////////////////////////////////////////////////////////////////

	/*!	Returns an element of the PathFinderError enumeration, indicating the last error
		encountered when planning a path. When the GetStatus() method of the PathFinder 
		returns #PathNotFound, you can use this method to retrieve additional information about
		the cause of the error. */
	PathFinderError GetLastError() const { return m_lastError; }

	/*!	\copydoc PathFinder::HasArrived() */
	KyBool IsGoalReached() const { return m_goalReached; }

	/*!	Retrieves the current position of the Bot using the PathFinder. */
	const Vec3f& GetBotPosition() const { return m_botPositionWrapper.GetPosition(); }

	/*!	\copydoc GetBotPosition() */
	const PointWrapper& GetBotPositionWrapper();

	/*!	Retrieves the NavMesh triangle that corresponds to the current position of the Bot. For internal use. */
	PointInfo_NavTriangleSafePtr& GetPointInfo_NavTriangleSafePtr() { return m_botPositionInfo_NavTriangleSafePtr; }
	/*!	For internal use. */
	KyBool BotPositionInfos_NeedUpdate() { return m_botPositionInfos_NeedUpdate; }


	/*!	Retrieves the current destination point, as passed in the last call to FindNextMove(). */
	const Vec3f& GetDestination() const { return m_goal.GetPosition(); }

	/*!	\copydoc GetDestination() */
	const PointWrapper& GetDestinationWrapper() const { return m_goal; }

	/*!	Updates NavMesh information for the Bot using the PathFinder. For internal use. */
	void UpdateBotPositionNavMeshInfo();

	/*!	Updates PathObject ConnexionSpace and ControlSpace information for the Bot using the PathFinder. For internal use. */
	void UpdateBotPositionPOSpacesInfo();

	/*!	Tests the specified point to see if it lies within an area of the NavMesh that is a valid place 
		for this Bot to stand. 
		\pp To be considered valid, the (X,Y) coordinates of the point must lie within the boundaries of the static
		NavMesh, outside the boundaries of of all blocking \ObstacleLayers and PathObject layers configured for the 
		CanGo_NavMesh modifier, and outside the boundaries of all \LpfAreas in the \LpfContext configured 
		for the PathFinder.
		\pp The NavMesh is retrieved as follows:
		-	If the PointWrapper has a PointInfo_NavTriangleSafePtr attached, the NavMesh that contains that triangle is used.
		-	If not, the NavMesh that is nearest in altitude to the altitude of the input point is used.
		\pp This method requires that your PathFinder use the CanGo_NavMesh modifier, or a modifier 
		that derives from CanGo_NavMesh. If you use any other class of ICanGo modifier, this method 
		will always return #KY_TRUE. 
		\pp Note that this method does not test for reachability: i.e. even if this method returns #KY_TRUE, the Bot may not be
		able to actually move to the point due to the intervention of dynamic \Obstacles, \PathObjects, etc. */
	KyBool IsClearPosition(const PointWrapper& pw);


	//////////////////////////////////////////////////////////////////////////
	// PathObject
	//////////////////////////////////////////////////////////////////////////

	/*!	Retrieves the current PathObject in use by the PathFinder. */
	IPathObject* GetCurrentPathObject() { return m_currentPathObject; }

	/*!	Sets the current PathObject to the specified PathObject, and calls the PathObject::OnLeave() method of the 
		previous PathObject if different from \c newPO. For internal use. */
	void SetCurrentPathObject(IPathObject* newPO, const GraphVertexSafePtr& entranceUid, const GraphVertexSafePtr& exitUid);

	/*!	Updates the current PathObject. For internal use. */
	void UpdateCurrentPathObject(KyBool forceRetrieval, const GraphVertexSafePtr& entranceUid, const GraphVertexSafePtr& exitUid);

	/*!	Indicates whether or not the specified IPathObject is contained in the #m_smoothablePathObjectTraverseInfos list. For internal use. */
	bool IsListedInSmoothablePathObjectTraverseInfos(IPathObject* pathobject);

	/*!	Adds the specified IPathObject to the #m_smoothablePathObjectTraverseInfos list. For internal use. */
	void InsertInSmoothablePathObjectTraverseInfos(const SmoothablePathObjectTraverseInfo& newSmoothablePOTraverseInfo);

	/*! Called when a PathObject has been removed from World. For internal use. */
	void OnPathObjectRemoved(IPathObject* /*pathObject*/) {}

	/*! Called when a PathObject starts moving. For internal use. */
	void OnPathObjectStartsMoving(IPathObject* /*pathObject*/) {}

	/*! Called when a PathObject ends moving. For internal use. */
	void OnPathObjectEndsMoving(IPathObject* /*pathObject*/) {}


	// Debug
	/*!	Draws information about the current Path for debugging purposes. All edges in the path are
		drawn in bright green. All other information rendered is controlled by the bitmask you specify
		in the \c flags parameter. See the #PathRenderFlags enumeration for details on the types of
		information you can render. 
		\pp If you use the default value for \c flags, all available information is rendered except for
		the index values of the PathData vertex that corresponds to each PathNode.
		\param flags		A bitmask composed of flags from the #PathRenderFlags enumeration. */
	void Render(KyUInt32 flags=KyUInt32MAXVAL-PathRender_NodeUids) const;

protected: // functions

	//////////////////////////////////////////////////////////////////////////////
	// Path planning
	//////////////////////////////////////////////////////////////////////////////

	virtual void ComputePath();
	void UpdatePath(const PointWrapper& startPosition, const PointWrapper& endPosition, KyBool forceRecompute = KY_FALSE);
	virtual KyBool UpdatePath_MustRecompute(const PointWrapper& endPosition, KyBool forceRecompute);
	virtual KyBool UpdatePath_ComputeNodes(const PointWrapper& startPosition, const PointWrapper& endPosition);
	virtual void UpdatePath_Astar(const Vec3f& endPosition, KyBool forceRecompute);
	virtual void ManagePathDataRemoval();
	virtual void ManagePathObjectStatusUpdate();


	//////////////////////////////////////////////////////////////////////////////
	// Path following
	//////////////////////////////////////////////////////////////////////////////

	virtual KyBool FollowPath();
	virtual KyBool UpdateSubgoal();
	virtual void UpdateSubgoal_Init();
	virtual KyBool UpdateSubgoal_LocalPath();
	virtual KyBool UpdateSubgoal_ManagePathObjects();
	virtual KyBool UpdateSubgoal_ManageReachedSubgoal();
	virtual void UpdateSubgoal_ImproveCurrentSubgoal();
	inline KyBool ComputePathNodeFromSubgoal(const Vec3f& oldSubgoal, PathNodeIterator& node, GraphVertexSafePtr& prevNodeUID);
	virtual KyBool Goto_PO(PointWrapper& poTargetPoint);
	virtual KyBool Goto_SmoothablePO(PointWrapper& poTargetPoint);
	virtual void FollowPath_SkipUnifiedPathNodes(KyUInt32 skipNodeCount);
	virtual void FlushObsoletePathNodes();


	//////////////////////////////////////////////////////////////////////////////
	// CallBacks
	//////////////////////////////////////////////////////////////////////////////

	static void OnDeActivateGraphCellCB(StitchedGraphCell& stitchedGraphCell, void* userData);
	virtual void OnDeActivateGraphCell(StitchedGraphCell& stitchedGraphCell);
	static void OnRemovePathObjectCB(IPathObject* po, Database* db, void* userData);
	virtual void OnRemovePathObject(IPathObject* po, Database* db);

	//////////////////////////////////////////////////////////////////////////////
	// Misc
	//////////////////////////////////////////////////////////////////////////////

	KyBool IsPoLinkedToEdge(const IPathObject *po, GraphEdgeSafePtr& edgeSafePtr);

	IFindNodesFromPositions& GetMandatoryModifier(FindNodesFromPositionsClass& modifierClass) const; 
	IRefineGoal& GetMandatoryModifier(RefineGoalClass& modifierClass) const; 
	IDetectGoalReached& GetMandatoryModifier(DetectGoalReachedClass& modifierClass) const; 
	IDetectGoalChanged& GetMandatoryModifier(DetectGoalChangedClass& modifierClass) const; 
	IDetectAccident& GetMandatoryModifier(DetectAccidentClass& modifierClass) const; 
	ISelectPathNodeCandidate& GetMandatoryModifier(SelectPathNodeCandidateClass& modifierClass) const; 
	IDetectPathNodeReached& GetMandatoryModifier(DetectPathNodeReachedClass& modifierClass) const; 
	ICanGo& GetMandatoryModifier(CanGoClass& modifierClass) const; 
	ISteering& GetMandatoryModifier(SteeringClass& modifierClass) const;
	IComputeTargetPoint& GetMandatoryModifier(ComputeTargetPointClass& modifierClass) const; 
	ICheckDirectWay& GetMandatoryModifier(CheckDirectWayClass& modifierClass) const;
	IGoto& GetMandatoryModifier(GotoClass& modifierClass) const; 
	IEdgeStatusAwareness* GetOptionalModifier(EdgeStatusAwarenessClass& modifierClass) const; 
	ILpfShortcut* GetOptionalModifier(LpfShortcutClass& modifierClass) const; 



public: // attributes

	//////////////////////////////////////////////////////////////////////////////
	// Path planning
	//////////////////////////////////////////////////////////////////////////////

	GraphManager* m_graphManager; /*!< A pointer to the GraphManager used by the PathFinder to compute its paths. Do not modify. */
	Database* m_pathDataDatabase; /*!< A pointer to the Database used by the PathFinder to access PathData. */
	AstarContext* m_astarContext; /*!< Context used by the PathFinder when calculating paths. Retrieved automatically from the World. Do not modify. */
	KyUInt32 m_nbEdgesPerAstar; /*!< The number of edges that should be processed during a path computation before checking CPU consumption. See :USERGUIDE:"Time-slicing A* Calculations". */

	Path m_path; /*!< The initial Path through the GraphManager calculated by the PathFinder. Do not modify. */
	KyFloat32 m_pathTimeStamp; /*!< The time at which #m_path was last modified. Do not modify. */
	GraphVertexSafePtr m_pathStartVertexSafePtr; /*!< The unique ID of the starting vertex for the last path calculation. Do not modify. */
	GraphVertexSafePtr m_pathDestVertexSafePtr; /*!< The unique ID of the destination vertex for the last path calculation. Do not modify. */
	KyFloat32 m_lastPathFlush; /*!< The time at which obsolete \PathNodes were last deleted from the Path. Do not modify. */

	/*! Contains the edges linked to \PathObjects that were considered when performing the last A*.
		The PathFinder periodically tests whether or not the status of any edges stored in
		the #m_poEdges list has changed (see #m_poEdgedStatusCheckPeriod). If the status of at least
		one of these edges changes, the current path is no longer guaranteed to be the shortest possible
		path. Therefore, a new path calculation is launched. */
	List<GraphEdgeSafePtr> m_poEdges;

	/*!	Contains the timestamp of the last time the status of the edges in #m_poEdges list was
		checked. Do not modify. */
	KyFloat32 m_lastTime_PoEdgedStatusCheck;

	/*!	Determines the minimum time interval that must elapse between successive tests of the status
		of the edges stored in the #m_poEdges list. Do not modify. */
	KyFloat32 m_poEdgedStatusCheckPeriod;

	/*!	Contains the timestamp of the last time the PathFinder checked whether or not any
		\GraphCells whose vertices are contained in the current path were streamed out of memory.
		See #m_usedPathdataUnstreamedCheckPeriod. Do not modify. */
	KyFloat32 m_lastTime_UsedPathdataUnstreamedCheck;

	/*!	Determines the minimum time interval that must elapse between successive tests to determine
		whether or not any GraphCells whose vertices are contained in the current path were streamed 
		out of memory. If at least one GraphCell was unloaded that contained a vertex used in the
		current path, the PathFinder launches a path re-computation. Do not modify. */
	KyFloat32 m_usedPathdataUnstreamedCheckPeriod;

	/*!	The unique numeric ID of the \c PathFinder::ComputePath aperiodic Task. Do not modify. */
	TimeManager::TaskId m_taskId_ComputePath;

	/*!	Indicates whether or not the PathFinder is allowed to recompute an injected path. See
		InjectPath_End(). Do not modify. */
	KyBool m_injectPathAutoRecompute;
	/*!	Indicates whether or not an event has been detected that will cause the PathFinder to
		recompute its path. Do not modify. */
	KyBool m_pathEventRaised;
	/*!	Indicates whether or not the PathFinder is using an injected path. Do not modify. */
	KyBool m_useInjectedPath;
	/*!	Indicates whether or not the PathFinder is following the direct way to the destination point, validated
		by the ICheckDirectWay modifier. Do not modify. */
	KyBool m_useDirectPath;

	/*!	Indicates whether or not the PathFinder must recompute its path as soon as possible.
		Do not modify. */
	KyBool m_forcePathRecomputation;

	/*!	Stores the status of the current path. Do not modify. */
	IPathFinder::PathStatus m_pathStatus;

	/*!	Stores the default Constraint used by the PathFinder. Do not modify. */
	ConstraintShortestPath m_defaultConstraint;
	/*!	Stores the default Heuristic used by the PathFinder. Do not modify. */
	HeuristicEuclidianDistance m_defaultHeuristic;

	/*!	Stores the NavTerrainTypes on which the bot is allowed to walk. */
	NavTerrainTypeMask m_allowedTerrainTypes;


	/*!	Stores the base time period at which the PathFinder attempts to re-compute the path
		when it is unable to find a path from the starting point to the destination. Do not modify. */
	KyFloat32 m_astarNewAttemptPeriod;
	/*!	Stores the last time the PathFinder attempted to compute a path. Do not modify. */
	KyFloat32 m_lastTime_AStarAttempt;

	/*!	Stores the destination point used for the previous A* computation. Do not modify. */
	Vec3f m_previousGoal;


	//////////////////////////////////////////////////////////////////////////////
	// Path following
	//////////////////////////////////////////////////////////////////////////////
protected:
	Modifiers m_modifiers; //< must not be modified once initialized

	/*	The final destination point provided in the last call to FindNextMove(). Do not modify. */
	PointWrapper m_goal;
	/*	The destination point refined by the IRefineGoal modifier. Do not modify. */
	PointWrapper m_refinedGoal;

public:
	/*!	The last PathNode in the Path. Do not modify. */
	PointWrapper m_pathGoal;
	/*!	Stores whether or not the final goal is considered to be reached. Do not modify. */
	KyBool m_goalReached;

	/*!	The PathNode currently being targeted by the PathFinder. Do not modify. */
	PathNodeIterator m_currentPathNode;
	/*!	The position of the current PathNode. Do not modify. */
	Vec3f m_pathNodePosition;

	/*!	The PathNode previously targeted by the PathFinder. Do not modify. */
	GraphVertexSafePtr m_previousPathNodeSafePtr;

	/*!	The target point of the Bot: the point at which the PathFinder is aiming the Bot
		in the current frame. Do not modify. */
	PointWrapper m_targetPoint;
	/*!	Indicates whether or not #m_targetPoint is reachable. Do not modify. */
	KyBool m_targetPointValid;

	/*!	Indicates whether or not CPU time remains for path following in the current frame. Do not modify. */
	KyBool m_grantCpuForPathFollowing;
	/*!	The unique numeric ID of the \c PathFinder::FollowPath aperiodic Task. Do not modify. */
	TimeManager::TaskId m_taskId_PathFollowing;

	/*!	Stores the base time period at which the PathFinder attempts to update its current PathNode. 
		Do not modify. */
	KyFloat32 m_pathNodeUpdatePeriod;
	/*!	Stores the time at which the PathFinder last updated its current PathNode. Do not modify. */
	KyFloat32 m_lastTime_UpdatePathNode;

	/*!	Stores the current PathNode across path recomputations. If the previous current PathNode is still in the
		new Path, it is kept as the current PathNode in order to keep the Bot moving smoothly. Do not modify. */
	Vec3f m_currentPathNodeInPreviousPath;
	/*!	Indicates whether or not #m_currentPathNodeInPreviousPath is reachable. Do not modify. */
	KyBool m_currentPathNodeInPreviousPathIsValid;

	/*!	The time at which the PathFinder last attempted to detect an accident. Do not modify. */
	KyFloat32 m_lastTime_AccidentDetection;
	/*!	The base time period at which the PathFinder checks to determine whether or not an accident
		has occurred. Do not modify. */
	KyFloat32 m_accidentDetectionPeriod;

	/*!	Indicates whether or not an accident has been detected. Do not modify. */
	KyBool m_accidentDetected;
	/*!	The time at which the last accident was detected. Do not modify. */
	KyFloat32 m_lastAccidentDate;
	/*!	Indicates the time at which the last accident to be detected was taken into account. Do not modify. */
	KyFloat32 m_lastAccidentConsideredInSubgoalInit;



	//////////////////////////////////////////////////////////////////////////////
	// Local path finding module
	//////////////////////////////////////////////////////////////////////////////

	/*!	For internal use. Do not modify. Access through GetLpfModule(). */
	LpfModule* m_lpfModule;


	//////////////////////////////////////////////////////////////////////////////
	// Misc
	//////////////////////////////////////////////////////////////////////////////

	// Error handling
	/*!	The last error encountered by the PathFinder. Do not modify. */
	PathFinderError m_lastError;

	// Bot position and attached infos
	/*!	The current position of the Bot using the PathFinder. Do not modify. */
	PointWrapper m_botPositionWrapper;
	/*!	Stores the PointInfo_NavTriangleSafePtr of the Bot using the PathFinder. Do not modify. */
	PointInfo_NavTriangleSafePtr m_botPositionInfo_NavTriangleSafePtr;
	/*!	Indicates whether or not the PointInfos of the Bot using the PathFinder
		needs to be updated. Do not modify. */
	KyBool m_botPositionInfos_NeedUpdate;

	// PathObjects
	PathObjectManager* m_pathObjectManager; /*!< A pointer to the PathObjectManager for the World. Do not modify. */
	SmoothablePathObjectTraverseInfo m_smoothablePathObjectCandidateTraverseInfos; /*!< Maintains temporary information about smoothable PathObjects during smoothing. Do not modify. */
	List<SmoothablePathObjectTraverseInfo> m_smoothablePathObjectTraverseInfos; /*!< Maintains a list of the smoothable \PathObjects that lie between the current position of the Bot and its target point. Do not modify. */
	IPathObject* m_currentPathObject; /*!< The current PathObject adopted by the PathFinder. Do not modify. */
	IPathObject* m_startingPathObject; /*!< The PathObject in which the bot as been detected at path planning time. Do not modify. */
	PointInfo_ControlSpace* m_botPositionInfo_ControlSpace;/*!< Stores the PointInfo_ControlSpace of the Bot using the PathFinder. Do not modify. */
	PointInfo_ConnexionSpace* m_botPositionInfo_ConnexionSpace;/*!< Stores the PointInfo_ConnexionSpace of the Bot using the PathFinder. Do not modify. */

	PathFinderFindStartNodeUser m_findStartNodeUser;
	PathFinderFindStartNodeStatus m_findStartNodeStatus;
	SharedPoolList<RuntimePathObjectControlSpace*>::ConstIterator m_findStartNode_ctrlSpaceIt;

	// other
	NavMeshLayerLocationList m_tmpLayerLocations; /*!< Temporary data used to locate PathObjects by their NavMeshLayer outlines. Do not modify. */
	KyFloat32 m_lastFindNextMoveCall; /*!< The time at which FindNextMove() was last called. Do not modify. */

protected:
	bool m_graphCellCallBackIsRegistered; /*!< to avoid warning when clearing the pathFinder twice. */
};

/*
	template specialization in order to access the modifiers through the interface type.
*/
template<> KY_FORCE_INLINE IFindNodesFromPositions& PathFinder::GetModifier() { return *(this->m_modifiers.m_findNodesFromPositions); }
template<> KY_FORCE_INLINE IRefineGoal& PathFinder::GetModifier() { return *(this->m_modifiers.m_refineGoal); }
template<> KY_FORCE_INLINE IDetectGoalReached& PathFinder::GetModifier() { return *(this->m_modifiers.m_detectGoalReached); }
template<> KY_FORCE_INLINE IDetectGoalChanged& PathFinder::GetModifier() { return *(this->m_modifiers.m_detectGoalChanged); }
template<> KY_FORCE_INLINE IDetectAccident& PathFinder::GetModifier() { return *(this->m_modifiers.m_detectAccident); }
template<> KY_FORCE_INLINE ISelectPathNodeCandidate& PathFinder::GetModifier() { return *(this->m_modifiers.m_selectPathNodeCandidate); }
template<> KY_FORCE_INLINE IDetectPathNodeReached& PathFinder::GetModifier() { return *(this->m_modifiers.m_detectPathNodeReached); }
template<> KY_FORCE_INLINE ICanGo& PathFinder::GetModifier() { return *(this->m_modifiers.m_canGo); }
template<> KY_FORCE_INLINE ISteering& PathFinder::GetModifier() { return *(this->m_modifiers.m_steering); }
template<> KY_FORCE_INLINE IComputeTargetPoint& PathFinder::GetModifier() { return *(this->m_modifiers.m_computeTargetPoint); }
template<> KY_FORCE_INLINE ICheckDirectWay& PathFinder::GetModifier() { return *(this->m_modifiers.m_checkDirectWay); }
template<> KY_FORCE_INLINE IGoto& PathFinder::GetModifier() { return *(this->m_modifiers.m_goto); }

template<> KY_FORCE_INLINE IEdgeStatusAwareness* PathFinder::GetOptionalModifier() { return this->m_modifiers.m_edgeStatusAwareness; }
template<> KY_FORCE_INLINE ILpfShortcut* PathFinder::GetOptionalModifier() { return this->m_modifiers.m_lpfShortcut; }

} // namespace Kaim

#endif // KY_PATHFINDER_H
