/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_gapda */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_GOTO_GAPDYNAMICAVOIDANCE_H
#define KY_GOTO_GAPDYNAMICAVOIDANCE_H

#include <kypathdata/containers/list.h>

#include <kyruntime/pathfinding/igoto.h>
#include <kyruntime/gapda/gapmanager.h>

namespace Kaim
{

class Obstacle;
class ObstacleManager;
class BodyInfoManager;

 // The polar diagram records the sectors obstructed by surrounding entities that intersects the disc which radius is myHalfWidth+extraGap+ KY_POLAR_DIAGRAM_WIDTH_RATIO*myWidth
#define KY_POLAR_DIAGRAM_WIDTH_RATIO 0.5f

 /* The crowd diagram records the entities in the rectangle,
	using same axis than collision diagram, of length MaxLength
	and width 2.0*KY_CROWD_DIAGRAM_WIDTH_RATIO*myWidth,
	and of both nearest angles cut at an angle defined by KY_CROWD_DIAGRAM_TANGENT
*/
#define KY_CROWD_DIAGRAM_WIDTH_RATIO 2.0f
#define KY_CROWD_DIAGRAM_TANGENT 2.0f



/*!	This implementation of the IGoto interface is is most suitable for use with highly reactive, agile 
	characters such as bipeds and animals, who have a very small turning radius. It is suitable for use
	regardless of crowd density, as it adjusts its behavior automatically depending on the number of 
	nearby \Bodies and the extent to which those other \Bodies interfere with the subject’s trajectory.
	\pp You do not have to interact with Goto_GapDynamicAvoidance at runtime at all. Its avoidance system is entirely
	autonomous. However:
	-	Fine details of your Bot reactions can be adjusted through class members. 
	-	You can retrieve information about the current state of the object. See \ref gapdaavoidancestates.
	\section gapdaprocess The gap dynamic avoidance process
	\pp At each frame, this class uses a three-step process that is analogous to the high-level approach
	recommended for all Bot decisions.
	\subsection gapdaperception Perception
	In order to make its Bot successfully avoid collisions with other nearby \Bodies, Goto_GapDynamicAvoidance
	must first collect information about the sizes, trajectories and speeds of the surrounding \Bodies.
	Based on this information, it constructs three different diagrams:
	-	A collision diagram, which identifies the nearby \Bodies whose trajectories may cause conflicts, taking 
		into account their relative speeds and projected orientations of movement. This diagram is used primarily
		in order to find avoidance trajectories in low-density to medium-density crowds. 
	-	A crowd diagram, which identifies the nearby \Bodies that are moving in approximately the same direction,
		and those that are moving in approximately the opposite direction. This diagram is used primarily in order
		to find avoidance trajectories in dense crowds with less freedom of movement. 
	-	A polar diagram, which identifies the directions of movement that will produce imminent collisions with 
		other \Bodies. This diagram is used primarily by the GetCost() method.
	\subsection gapdadecision Decision
	At each frame, Goto_GapDynamicAvoidance uses its perceptual diagrams to determine whether it can
	move directly to the current target point computed by the PathFinder at its current speed, or whether it must
	introduce evasive actions into the trajectory of the Bot. 
	\pp When some modification to the trajectory of the Bot is necessary, Goto_GapDynamicAvoidance generates a set of candidate
	points from its perceptual diagrams, using an approach that depends on the density and trajectories of the 
	surrounding \Bodies. It evaluates these points to choose the most appropriate candidate as its temporary target
	point. Depending on the trajectories of the surrounding \Bodies and the way it is configured, Goto_GapDynamicAvoidance may
	also lower the target speed that should be applied to the Bot. For example, if the best action is to follow a
	slower-moving Body.
	\pp For details on the way Goto_GapDynamicAvoidance determines its target point and target speed,
	see \ref gapdaavoidancestates.
	\subsection gapdaaction Action
	Once Goto_GapDynamicAvoidance determines its target point and target speed, it invokes the ISteering
	modifier in use by the PathFinder to construct the Action that will determine the actual movements carried out 
	by the Bot. 
	\section gapdaavoidancestates Avoidance states
	Goto_GapDynamicAvoidance is essentially a finite state machine with five high-level states, identified
	by the elements in the #GlobalMode enumeration. Several of these global modes are further broken down into different
	refined modes, described in the #RefinedMode enumeration, which indicate the particular action decided on for the current frame. 
	\pp You can determine the current mode by calling the GetRefinedMode() and GetGlobalMode() methods.
	Alternatively, you can register a callback function that will be invoked each time the mode changes.
	For details, see the #OnDAModeChange type definition, and the SetDAModeChangeCB() method.
	\subsection gapdanormalmode Normal mode
	In this mode, the Bot is able to move directly to the target point determined by the PathFinder, without interference
	from any other \Bodies. Goto_GapDynamicAvoidance moves the Bot directly toward that target point at the target speed set for the
	PathFinder, and continues to update its perceptual diagrams in order to detect potential collisions. 
	\subsection gapdastandardmode Standard gap avoidance mode
	When a collision is detected with another Bot in an area that is not crowded with other \Bodies, Goto_GapDynamicAvoidance
	enters its #GlobalMode_Standard gap avoidance mode, which uses the collision diagram to come up with a new target point for the Bot.
	\pp The following image shows an example of a collision diagram, with candidate points rendered:
	\image html "graphics/dynamic_avoidance_gaps_diagram.png"
	\pp Goto_GapDynamicAvoidance calculates a row of candidate points, perpendicular to the direction of movement of the Bot, 
	that may serve as temporary intermediate target points. These candidates are closely spaced immediately in front of the Bot,
	and more widely spaced as the distance from the current orientation of the Bot increases. 
	\pp In order for a candidate point to be considered valid, it must be directly reachable from the current position of
	the Bot, and the target point of the PathFinder must be directly reachable from the candidate point. For best performance,
	these tests are not made against the NavMesh or the static geometry. Instead, the tests are made against a set of gaps,
	which divide the horizontal plane of the diagram into open areas (indicated in green) and areas that lie in the path of
	other moving \Bodies (indicated in yellow). The width of each yellow gap in the diagram should be approximately equal to
	the width of the corresponding Body.
	\pp Candidate points are stored in a sorted list with the best candidates first: those that offer the clearest path with
	the least deviation. The candidates are then tested against the NavMesh, to ensure that the lines are not blocked by
	static geometry or dynamic \Obstacles. The first gap that does not collide with any obstacles is adopted as the new target
	point. This step is spread across multiple frames, to avoid peaks in CPU consumption.
	\pp For example, in the image above, the candidate points that lie directly in the paths of other moving \Bots and \Bodies
	are not preferred (rendered in colors closer to red) relative to candidates that lie in open gaps (rendered in colors 
	closer to yellow). The best candidate is rendered in green. Note that the candidate on the far right is refused because
	it passes through the static geometry.
	\pp Several aspects of this standard mode can be adjusted through setting class members.
	\subsection gapdacrowdmode Crowded mode
	When the number of nearby \Bodies rises to the point that the standard gap collision avoidance algorithm is unable to
	produce good results, the Goto_GapDynamicAvoidance enters its #GlobalMode_Crowded mode.
	\pp In this mode, the crowd diagram is used to find a Body moving in the same direction that the subject can follow.
	If no such Body is available, or if the path to that Body is blocked, the subject attempts to move to the right, to
	allow oncoming Bodies to pass by. If it cannot move to the right either, it simply continues to attempt to move straight ahead. 
	\pp By default, crowd mode is disabled; you can enable it through a call to AllowCrowding(KyBool).
	\subsection gapdarejoining Rejoining the original path
	In #GlobalMode_Standard mode, the original target point of the PathFinder is always visible from the temporary target, so
	the original path can always be rejoined easily. However, in #GlobalMode_Crowded mode, the target point of the PathFinder
	is not always visible: for example, if the subject followed another Body moving in approximately the same direction.
	\pp When Goto_GapDynamicAvoidance leaves a crowded area, it switches away from #GlobalMode_Crowded and attempts to direct the Bot back
	to its original path. It enters the #GlobalMode_RejoiningOriginalPath mode, in which it computes a new set of candidates
	intended to lead it smoothly back to the original path. This set of points includes candidates at different directions around
	the subject Bot: straight ahead, 45 degrees to either side, 90 degrees to either side, and 135 degrees to either side. Each
	point is evaluated, and the best is selected.
	\subsection gapdastuck Stuck mode
	The modifier enters #GlobalMode_Stuck when it is completely unable to determine a viable target point for the Bot. The Bot
	stops, and periodically tries again to find a valid candidate point.
	\section gapdasetup Setting Up Goto_GapDynamicAvoidance
	Setting up this modifier involves the following considerations.
	\subsection requirements Requirements
	-	You must activate the GapManager in your World through a call to World::ActivateService().
	\subsection recommendations Recommendations
	-	For best performance, set up the BodyInfoManager for your World. When the BdyInfoManager
		is available, Goto_GapDynamicAvoidance Modifier it to determine which other
		\Bodies lie in close proximity to the subject Bot. If it is not available, 
		Goto_GapDynamicAvoidance has to iterate through the list of all \Bodies in the
		World in order to find the ones that are near enough to affect the trajectory of the subject Bot.
	\ingroup kyruntime_gapda */
class Goto_GapDynamicAvoidance: public IGoto
{
	/* Used internally by Goto_GapDynamicAvoidance to remove internal pointers to Bodies that are removed from the World. */
	class WorldObserver : public IWorldObserver
	{
	public:
		WorldObserver() : m_goto_gapDa(KY_NULL) {}
		void Init(Goto_GapDynamicAvoidance& goto_gapDa) { m_goto_gapDa = &goto_gapDa;}

		virtual void OnActivateBody(Body& ) {}
		virtual void OnDeactivateBody(Body& body) { m_goto_gapDa->OnRemoveBody(body); }

	protected:
		Goto_GapDynamicAvoidance* m_goto_gapDa;
	};
	friend class WorldObserver;
public: // Enumerators

	/*! Enumerates the possible high-level global modes. See \ref gapdaavoidancestates. See also GetGlobalMode(). */
	enum GlobalMode {
		GlobalMode_Normal = 0,				/*!< No collision is imminent, and Goto_GapDynamicAvoidance does not change the target point and target speed of the PathFinder. */
		GlobalMode_Standard,				/*!< Goto_GapDynamicAvoidance is using its standard gap collision avoidance mode. */
		GlobalMode_Crowding,				/*!< Goto_GapDynamicAvoidance is using its crowd avoidance mode. */
		GlobalMode_RejoiningOriginalPath,	/*!< Goto_GapDynamicAvoidance has left its crowd avoidance mode and is returning to the original target point of the PathFinder. */
		GlobalMode_Stuck,					/*!< Goto_GapDynamicAvoidance was unable to find a good target point at all, and the Bot is stuck at its current location. */
		GlobalMode_dummy_ForceEnumSize = KYFORCEENUMSIZEINT
	};

	/*! Enumerates the possible low-level refined modes. See GetRefinedMode(). */
	enum RefinedMode {
		// Mode_Normal sub-modes
		RefinedMode_Normal = 0,				/*!< The Bot is in the #GlobalMode_Normal mode, and is moving directly toward its target point. */

		// Mode_Standard sub-modes
		RefinedMode_Standard,				/*!< The Bot has entered the #GlobalMode_Standard mode. */
		RefinedMode_Standard_Avoiding,		/*!< The Bot is in the #GlobalMode_Standard mode, has successfully calculated a clear avoidance trajectory, and is following that trajectory. */
		RefinedMode_Standard_Slowing,		/*!< The Bot is in the #GlobalMode_Standard mode and has found a trajectory, but the time horizon before the collision is short, causing the Bot to slow down. */
		RefinedMode_Standard_Queueing,		/*!< The Bot is in the #GlobalMode_Standard mode and has not found a clear avoidance trajectory. The Bot waits until a trajectory opens up. If the timer set by QueueingDelay(KyFloat32) is reached, it switches to #RefinedMode_Standard_Pushing. */
		RefinedMode_Standard_Pushing,		/*!< The Bot is in the #GlobalMode_Standard mode and is attempting to follow the best trajectory it can find, even if there are some obstacles in the way. If the Bot cannot move before the timer set by PushingDelay(KyFloat32) is reached, it switches to #GlobalMode_Stuck. */

		// Mode_Crowding sub-modes
		RefinedMode_Crowding,				/*!< The Bot has entered the #GlobalMode_Crowding mode. */
		RefinedMode_Crowding_AvoidOnRight,	/*!< The Bot is in the #GlobalMode_Crowding mode, and is avoiding oncoming \Bodies by sidestepping to the right. */
		RefinedMode_Crowding_FollowingFlow,	/*!< The Bot is in the #GlobalMode_Crowding mode, and is following another Bot going in roughly the same direction. */
		RefinedMode_Crowding_Crowded,		/*!< The Bot is in the #GlobalMode_Crowding mode, and the crowd density is such that the Bot is unable to move around oncoming \Bodies or by following \Bodies going in the same direction. If the Bot is unable to move directly forward, it switches to #GlobalMode_Stuck. */

		// Mode_RejoiningOriginalPath sub-modes
		RefinedMode_RejoiningOriginalPath,	/*!< The Bot is in the #GlobalMode_RejoiningOriginalPath mode, attempting to return to its original path. */

		// Mode_Stuck sub-modes
		RefinedMode_Stuck,					/*!< The Bot is in the #GlobalMode_Stuck mode, unable to find a trajectory at all. It stops until the timer set by StuckDelay(KyFloat32) is reached, then requests a path recomputation from the PathFinder. */

		RefinedMode_dummy_ForceEnumSize = KYFORCEENUMSIZEINT
	};


	// Rendering flags enumerator
	typedef KyUInt32 RenderMasks; /*!< Defines a type for a bitmask composed of elements from the #RenderFlags enumeration. */
	/*! Determines the type of debug information drawn by the Render() method. */
	enum RenderFlags
	{
		RF_CrowdDiagram = 1<<0,				/*!< Renders the crowd diagram. */
		RF_CollisionDiagram = 1<<1,			/*!< Renders the collision diagram. */
		RF_PolarDiagram = 1<<2,				/*!< Renders the polar diagram. */
		RF_SameDirBodies = 1<<4,			/*!< Renders the \Bodies perceived to be moving in approximately the same direction. */
		RF_OppositeDirBodies = 1<<5,		/*!< Renders the \Bodies perceived to be moving in the opposite direction. */
		RF_Candidates = 1<<8,				/*!< Renders the possible candidate points generated by the modifier. */
		RF_Followee = 1<<9,					/*!< Renders the Bot being followed, if any. */
		RF_DATarget = 1<<12,				/*!< Renders the current temporary target point used by the modifier. */
		RF_Mode = 1<<16,					/*!< Renders the current global and refined modes. */
		RF_dummy_ForceEnumSize = KyUInt32MAXVAL
	};


public: // Internal classes

	/*!	Used internally by Goto_GapDynamicAvoidance to manage situations with high
		crowd density. Use of this class is entirely transparent. You should not need to interact 
		with this class directly.
		\ingroup kyruntime_gapda */
	class CrowdModule: public VirtualMemObject
	{
	public:
		class BodyRecord: public VirtualMemObject
		{
		public:
			BodyRecord():m_body(KY_NULL) {}
			~BodyRecord() {}

			// body infos
			// - body itself
			const Body* m_body;
			// - body shape attributes
			KyFloat32 m_width;
			KyFloat32 m_length;
			Vec3f m_torsoDir;

			// Relative infos
			KyFloat32 m_score;
		};

	public:
		CrowdModule();
		~CrowdModule();

		void ReInit();
		void ClearDiagram();

		void AddNearSameDirBody(const Body* body, const KyFloat32 hisWidth,
			const KyFloat32 hisLength, const Vec3f& entTorsoOri, const KyFloat32 score);

		void AddNearOppositeDirBody(const Body* body, const KyFloat32 hisWidth,
			const KyFloat32 hisLength, const Vec3f& entTorsoOri, const KyFloat32 score);

	public:
		// same dir bodies
		List<BodyRecord> m_nearSameDirBodies;
		List<BodyRecord>::Iterator m_followee;

		// opposite dir entities
		List<BodyRecord> m_nearOppositeDirBodies;
	};


	/*!	Used internally by Goto_GapDynamicAvoidance to represent areas within the
		polar diagram that are occupied by other \Bodies. Use of this class is entirely transparent. 
		You should not need to interact with this class directly.
		\ingroup kyruntime_gapda */
	class PolarDiagramSector
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
	public:
		PolarDiagramSector(): m_axis(0.f, 0.0f, 0.f), m_cosAngle(1.0f) {}
		Vec3f m_axis;
		KyFloat32 m_cosAngle;
	};

	/*!	Used internally by Goto_GapDynamicAvoidance to represent candidate target
		points for the Bot. Use of this class is entirely transparent. You should not need to 
		interact with this class directly.
		\ingroup kyruntime_gapda */
	class Candidate
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
	public:
		Candidate():
			m_traceLinePosition(KyFloat32MAXVAL, KyFloat32MAXVAL, KyFloat32MAXVAL),
			m_position(KyFloat32MAXVAL, KyFloat32MAXVAL, KyFloat32MAXVAL),
			m_cost(0.0f),
			m_abscissa(0.0f),
			m_speed(0.0f),
			m_gapTti(0.0f),
			m_gapFrontalSpeed(0.0f),
			m_nextMode(RefinedMode_Normal),
			m_crowdRecord()
			{}

	public:
		Vec3f m_traceLinePosition;
		Vec3f m_position;
		KyFloat32 m_cost;
		KyFloat32 m_abscissa;
		KyFloat32 m_speed;
		KyFloat32 m_gapTti;
		KyFloat32 m_gapFrontalSpeed;
		RefinedMode m_nextMode;
		List<CrowdModule::BodyRecord>::Iterator m_crowdRecord;
	};

	typedef List<Candidate> CandidateList;

public:
	KY_DECLARE_GOTO(Goto_GapDynamicAvoidance)


public: // methods
	/*! \label_constructor
		\param gapManager		The GapManager that you have set up for the World. */
	Goto_GapDynamicAvoidance(Kaim::GapManager* gapManager = KY_NULL);
	
	/*! \label_virtualdestructor */
	~Goto_GapDynamicAvoidance();

	/////////////////////////////////////////////////////////////////////////////////
	// Base class functions (see base class for details)
	/////////////////////////////////////////////////////////////////////////////////
	virtual void ReInit();
	
	/*!	\copydoc IGoto::Goto()
		\pp This implementation of IGoto::Goto() finds nearby \Bodies that may block the direct line to \c targetPoint,
		uses a collision diagram and a crowd diagram to calculate a temporary target point that will best avoid those 
		other \Bodies, and calls the ISteering modifier to construct an Action that moves the Bot toward the 
		temporary target point. */
	virtual KyBool Goto(const PointWrapper& targetPoint, Action& action);

	/*!	\copydoc IGoto::GetCost()
		\pp This implementation of IGoto::GetCost() assigns a very high cost (multiplied by 1000) to any segment whose
		end point lies within the polar diagram. */
	virtual KyBool GetCost(Bot* bot, const PointWrapper& from, const PointWrapper& to, 
		IConstraint& constraint, KyFloat32& cost);

	/*!	Defines a type of callback function that can be passed to Goto_GapDynamicAvoidance
		to determine which other \Bodies should be taken into account when computing
		a target point that lies outside the area occupied by other \Bodies.
		\param thisBody			The Bot using the Goto_GapDynamicAvoidance modifier.
		\param targetBody		Another Bot that may be taken into account by this modifier in
								its calculations.
		\return #KY_TRUE if \c thisBody should take \c targetBody into account in its computations, 
				or #KY_FALSE otherwise. */
	typedef KyBool (*FilteringFunction)(Body& thisBody, Body& targetBody);
	
	/*!	Sets a callback function that Goto_GapDynamicAvoidance will invoke to determine
		which other \Bodies it should take into account in its computations. */
	void SetFilteringCallBack(FilteringFunction func) { m_filterCB = func; }
	
	/*!	Retrieves a callback function previously set in a call to SetFilteringCallBack(). */
	FilteringFunction GetFilteringCallBack() { return m_filterCB; }


	/////////////////////////////////////////////////////////////////////////////////
	// Specific functions
	/////////////////////////////////////////////////////////////////////////////////

	// Debug rendering
	/*!	Renders the diagram that contains potential collisions with other \Bodies. */
	void RenderCollisionDiagram() const;
	
	/*!	Renders potential candidates (intermediate points) that are accessible from the current position of the Bot
		and from the target point. */
	void RenderCandidates(KyBool drawCandidateSpeed = KY_FALSE, KyBool drawCandidateCost = KY_FALSE, KyBool drawCandidateValidation = KY_FALSE) const;
	
	/*!	Renders the polar diagram, which indicates the directions that are occupied by other \Bodies. */
	void RenderPolarDiagram() const;
	
	/*!	Renders the specified information. Note that only information relevant to the current mode is rendered.
		For example, in standard mode, the collision diagram is rendered, while in crowd mode only the crowd diagram
		is rendered.
		\param flags			A bit mask composed of items in the #RenderFlags enumeration, which indicates
								the types of information that should be rendered. If omitted, all information 
								is rendered. */
	//void Render(const KyUInt32 flags=RF_CrowdDiagram|RF_Candidates|RF_Followee|RF_DATarget);
	void Render(const RenderMasks flags=0xFFFFFFFF) const;

	/*!	Retrieves the high-level global mode that corresponds to the specified low-level refined mode. 
		\param refinedMode		Specifies a low-level refined mode in the #RefinedMode enumeration. */
	static GlobalMode GetGlobalMode(RefinedMode refinedMode);
	
	/*!	Retrieves the current high-level global mode. */
	GlobalMode GetGlobalMode() const {return GetGlobalMode(m_currentMode);}
	
	/*!	Retrieves the current low-level refined mode. */
	RefinedMode GetRefinedMode() const { return m_currentMode; }

	/*!	Defines a type of callback function that can be passed to Goto_GapDynamicAvoidance
		in order to receive feedback each time it switches to a different low-level refined mode. 
		See also SetDAModeChangeCB().
		\param bot				The Bot using this modifier.
		\param previousMode		The low-level refined mode used by \c bot before the mode change.
		\param newMove			The low-level refined mode used by \c bot after the mode change.
	*/
	typedef void (*OnDAModeChange)(Bot* bot, RefinedMode previousMode, RefinedMode newMode);

	/*! Sets a callback function that Goto_GapDynamicAvoidance uses to provide feedback when it
		changes to a new low-level refined mode. */
	void SetDAModeChangeCB(OnDAModeChange onDAModeChangeCB) { m_onDAModeChange = onDAModeChangeCB; }

	/*!	Retrieves the point that is currently targeted by Goto_GapDynamicAvoidance (not necessarily 
		equivalent to the current target point maintained by the PathFinder). */
	const Vec3f& GetTargetPoint() const;

	/*!	Indicates whether or not Goto_GapDynamicAvoidance is currently altering the target point 
		of the Bot in order to avoid one or more other dynamic \Bodies. */
	KyBool IsAvoiding() const;

	/////////////////////////////////////////////////////////////////////////////////
	// Advanced use : do not call these if you don't know precisely what you're doing
	/////////////////////////////////////////////////////////////////////////////////

	/*!	Sets the target point of the modifier to the specified position. For internal use. */
	void SetDATargetPoint(const Vec3f& point) { m_daTargetPoint = point; }

	/*!	Clears all sectors from the polar diagram. For internal use. */
	void ClearPolarDiagram() { m_polarDiagram.Clear(); }

	/*!	Adds a new sector to the polar diagram. For internal use. */
	void AddPolarDiagramSector(const Vec3f& axis, KyFloat32 cosAgle);

	/*!	Set the next time at which the polar diagram will be updated. For internal use. */
	void SetNextPolarDiagramUpdateTime(KyFloat32 time) { m_nextPolarDiagramUpdateTime = time; }

	/*!	Retrieves the call counter for the \c UpdateDiagram_AddBodyestimated task. Used to determine an accurate estimation for 
		this task when in deterministic time management mode. */
	static KyUInt32 GetTaskCounter_UpdateDiagram_AddBody();

protected: // methods
	void OnRemoveBody(Body& oldBody);
	// perception functions (diagrams creation)
	virtual void UpdateDiagrams();
	virtual void UpdateDiagrams_AddBody(const Body& body);
	virtual void UpdateDiagrams_AddObstacle(const Obstacle* obstacle);
	virtual void UpdateDiagram_Collision_CircularBody(const Body& body, KyFloat32 hisWidth, KyFloat32 tti);
	virtual void UpdateDiagram_Collision_RectangularBody(const Vec3f& pos, const Vec3f& ori, KyFloat32 hisWidth, KyFloat32 hisLength, KyFloat32 frontalSpeed);
	virtual void UpdateDiagram_Collision_Obstacle(const Obstacle* obstacle);
	virtual void UpdateDiagram_Crowd_CircularBody(const Body& body, const Vec3f &deltaPos, const KyFloat32 hisWidth);
	virtual void UpdateDiagram_Crowd_RectangularBody(const Body& body, const Vec3f &deltaPos, const KyFloat32 hisWidth, KyFloat32 const hisLength);
	virtual void UpdateDiagram_Polar_CircularBody(const Body& body, const Vec3f &deltaPos, const KyFloat32 hisWidth);
	virtual void UpdateDiagram_Polar_RectangularBody(const Body& body, const Vec3f &deltaPos, const KyFloat32 hisWidth, KyFloat32 const hisLength);
	virtual void UpdateDiagram_Polar_Obstacle(const Obstacle* obstacle);
	virtual void UpdatePolarDiagram();

	// action functions (candidate generation, validation and selection and action update)
	virtual void ComputeTargetPoint();
	KyBool CheckCurrentDATargetPointValidity();

	void GenerateCandidates_Standard();
	void GenerateCandidates_Crowding();
	void GenerateCandidates_RejoiningOriginalPath();

	virtual Candidate ComputeCandidate(KyFloat32 abscissa, const List<Gap>::ConstIterator& gapIt, KyFloat32 limitingTti, KyBool abscissaBias);
	virtual KyFloat32 ComputeSpeed(KyFloat32 targetSpeed, KyFloat32 ttiMin, KyFloat32 minGapFrontalSpeed);

	virtual void ProcessNextCandidates();

	virtual KyBool ValidateNextCandidate_Standard();
	virtual KyBool ValidateNextCandidate_Crowding();
	virtual KyBool ValidateNextCandidate_RejoiningOriginalPath();
	virtual void SetNextCandidateAsDATargetPoint();

	void UpdateCurrentMode();
	void SetCurrentMode(RefinedMode newMode); // call the callback if necessary

	virtual KyBool UpdateAction(Action& action);
	KyFloat32 GetSpeed_Crowding(RefinedMode refinedMode); // computes the speed accordingly to the given crowding state
	KyFloat32 GetSpeed_RejoiningOriginalPath(); // computes the speed

public:
	/*!	The unique numeric ID of the \c UpdateDiagram_AddBody estimation. Used only in deterministic time management mode. See :USERGUIDE:"Using Deterministic Time Measurement". Do not modify. */
	TaskRegistry::TaskId m_taskId_UpdateDiagram_AddBody;

	// parameters

	/*! Retrieves the value set through a call to DistMax(KyFloat32). */
	KyFloat32 DistMax() const { return m_distMax; }

	/*! Determines the maximum distance at which a Body may be considered for avoidance. The solver ignores all \Bodies farther
		away than this distance.
		\units			meters
		\acceptedvalues	any positive value
		\defaultvalue	\c 20.0f */
	void DistMax(KyFloat32 val) 
	{ 
		KY_FUNCTION("Goto_GapDynamicAvoidance::DistMax");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'DistMax' must be > 0.", val));
		KY_ASSERT(m_me && val > m_me->GetBody().GetWidth(), ("Invalid value (%.2f), 'DistMax' must be > body's width.", val));
		m_distMax = val;
	}
	
	/*! Retrieves the value set through a call to DiagramMaxLength(KyFloat32). */
	KyFloat32 DiagramMaxLength() const { return m_diagramMaxLength; }

	/*! Determines the maximum total length of the collision diagram.
		\units			meters
		\acceptedvalues	any positive value greater than one-half the current width of the Bot
		\defaultvalue	\c 5.0f */
	void DiagramMaxLength(KyFloat32 val) 
	{ 
		KY_FUNCTION("Goto_GapDynamicAvoidance::DiagramMaxLength");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'DiagramMaxLength' must be > 0.", val));
		KY_ASSERT(m_me && val > m_me->GetBody().GetLength(), ("Invalid value (%.2f), 'DiagramMaxLength' must be > body's length.", val));
		m_diagramMaxLength = val;
	}

	/*! Retrieves the value set through a call to DiagramHalfWidth(KyFloat32). */
	KyFloat32 DiagramHalfWidth() const { return m_diagramHalfWidth; }

	/*! Determines the width of half the collision diagram. 
		\pp A good value for this parameter is twice the width of the Bot using the PathFinder. 
		\pp It is recommended that you exercise caution when setting this parameter to a different value. If the value is too low, 
		\Bots may have trouble finding acceptable paths to avoid other \Bodies. If the value is too high, CPU consumption is increased, 
		and \Bots may make undesirably large detours.
		\units			meters
		\acceptedvalues	any positive value greater than the width of the Bot
		\defaultvalue	\c 3.0f */
	void DiagramHalfWidth(KyFloat32 val)
	{
		KY_FUNCTION("Goto_GapDynamicAvoidance::DiagramHalfWidth");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'DiagramHalfWidth' must be > 0.", val));
		KY_ASSERT(m_me && val > m_me->GetBody().GetWidth(), ("Invalid value (%.2f), 'DiagramHalfWidth' must be > body's width.", val));
		m_diagramHalfWidth = val;
		m_diagramInvHalfWidth = 1.f / m_diagramHalfWidth;
	}

	/*! Retrieves the value set through a call to DiagramRefreshPeriod(KyFloat32). */
	KyFloat32 DiagramRefreshPeriod() const { return m_diagramRefreshPeriod; }

	/*! Determines the maximum time that the collision diagram remains valid before it is recomputed.
		\pp It is recommended that you exercise caution in setting this parameter. If the value is too low, the accuracy of the path computations
		increases, but the CPU usage of the solver increases correspondingly. If the value is too high, CPU consumption is decreased, but \Bots 
		may have trouble finding up-to-date paths to avoid other \Bodies.
		\units			seconds
		\acceptedvalues	any positive value
		\defaultvalue	\c 0.1f */
	void DiagramRefreshPeriod(KyFloat32 val)
	{ 
		KY_FUNCTION("Goto_GapDynamicAvoidance::DiagramRefreshPeriod");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'DiagramRefreshPeriod' must be > 0.", val));
		m_diagramRefreshPeriod = val; 
	}

	/*! Retrieves the value set through a call to CandidateSpacing(KyFloat32). */
	KyFloat32 CandidateSpacing() const { return m_candidateSpacing; }

	/*! Determines the lateral distance between the two central candidate points. Spacing between adjacent candidates increases with their distance from the center point.
		\pp It is recommended that you exercise caution in setting this parameter. If the value is too low, the accuracy of the path computations increases, but the
		CPU usage of the solver increases correspondingly. If the value is too high, CPU consumption is decreased, but \Bots may have trouble finding acceptable paths
		to avoid other \Bodies.
		\units			meters
		\acceptedvalues	any positive value
		\defaultvalue	\c 0.1f */
	void CandidateSpacing(KyFloat32 val) 
	{ 
		KY_FUNCTION("Goto_GapDynamicAvoidance::CandidateSpacing");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'CandidateSpacing' must be > 0.", val));
		m_candidateSpacing = val;
	}

	/*! Retrieves the value set through a call to ExtraGap(KyFloat32). */
	KyFloat32 ExtraGap() const { return m_extraGap; }

	/*! Determines a minimum distance that must be maintained in between passing \Bodies. This parameter enhances the realism of the
		trajectories, as it prevents \Bots from approaching too close to other \Bodies. 
		\units			meters
		\acceptedvalues	any positive value
		\defaultvalue	\c 0.1f */
	void ExtraGap(KyFloat32 val) 
	{ 
		KY_FUNCTION("Goto_GapDynamicAvoidance::ExtraGap");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'ExtraGap' must be > 0.", val));
		m_extraGap = val;
	}

	/*! Retrieves the value set through a call to MinSpeed(KyFloat32). */
	KyFloat32 MinSpeed() const { return m_minSpeed; }

	/*! Determines the behavior of the Bot when approaching another Body that blocks its path.
		-	A value close to 0 makes the Bot slow down to follow the blocking Body if it is moving in the same direction. 
		-	A value close to the maximum speed of the Bot makes the Bot avoid the blocking Body by deviating from its original path. 
		-	Intermediate values set the preferred behavior on a scale between these two extremes. A lower value makes the Bot more likely to
			slow down or stop; a higher value makes the Bot more likely to move aside. 
		\pp Higher values are recommended in open spaces where Bots have room to maneuver. Bots will tend to walk around each other.
		\pp Lower values are recommended for use with dense crowds or in enclosed spaces such as sidewalks and corridors. Bots will tend to
		slow down and form queues and streams.
		\pp Used only when the ContinuousSpeed(KyBool) value is set to #KY_TRUE.
		\units			meters
		\acceptedvalues	any positive value
		\defaultvalue	\c 1.0f */
	void MinSpeed(KyFloat32 val) 
	{ 
		KY_FUNCTION("Goto_GapDynamicAvoidance::MinSpeed");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'MinSpeed' must be > 0.", val));
		m_minSpeed = val;
	}

	/*! Retrieves the value set through a call to Courtesy(KyFloat32). */
	KyFloat32 Courtesy() const { return m_courtesy; }

	/*! Determines the preferred behavior of the Bot on a scale of courtesy, where:
		-	A value of 1 indicates maximum courtesy. The Bot will stop when the most disruptive Bot in the alternate pathway is nearby. 
		-	A value of 0 indicates minimum courtesy. The Bot never stops; instead, it attempts to push other \Bodies out of the way to
			reach its destination. 
		-	Values between 0 and 1 set the preferred behavior on a scale between these two extremes. A higher value makes the Bot more likely
			to stop or take a large detour; a lower value makes the Bot more likely to push other \Bodies. 
		Changing this parameter at runtime can result in visible changes in behavior. For example, \Bots wandering in a quiet place might have their
		courtesy values initially set to 1. However, if an explosion occurs, making the \Bots flee, the courtesy value could be changed to 0 to
		simulate a panic reaction, where each Bot is trying to save itself at the expense of others.
		\acceptedvalues	any positive value
		\defaultvalue	\c 1.0f */
	void Courtesy(KyFloat32 val) 
	{ 
		KY_FUNCTION("Goto_GapDynamicAvoidance::Courtesy");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'Courtesy' must be > 0.", val));
		m_courtesy = val;
	}

	/*! Retrieves the value set through a call to QueueingDelay(KyFloat32). */
	KyFloat32 QueueingDelay() const { return m_queueingDelay; }

	/*! Determines the maximum length of time \Bots will spend in Queuing mode before switching to Pushing mode.
		Meanwhile, the solver continues to update diagrams and evaluate candidate points in order to find movement opportunities.
		\units			seconds
		\acceptedvalues	any positive value
		\defaultvalue	\c 5.0f */
	void QueueingDelay(KyFloat32 val) 
	{ 
		KY_FUNCTION("Goto_GapDynamicAvoidance::QueueingDelay");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'QueueingDelay' must be > 0.", val));
		m_queueingDelay = val;
	}

	/*! Retrieves the value set through a call to PushingDelay(KyFloat32). */
	KyFloat32 PushingDelay() const { return m_pushingDelay; }

	/*! Determines the maximum length of time \Bots will spend in Pushing mode before switching to Normal mode. 
		During this time, \Bots try to push their way through to their destination, disregarding other \Bodies.
		\units			seconds
		\acceptedvalues	any positive value
		\defaultvalue	\c 5.0f */
	void PushingDelay(KyFloat32 val) 
	{ 
		KY_FUNCTION("Goto_GapDynamicAvoidance::PushingDelay");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'PushingDelay' must be > 0.", val));
		m_pushingDelay = val;
	}

	/*! Retrieves the value set through a call to StuckDelay(KyFloat32). */
	KyFloat32 StuckDelay() const { return m_stuckDelay; }

	/*! Determines the maximum length of time \Bots will spend in Stuck mode before trying to find a new avoidance trajectory.
		\units			seconds
		\acceptedvalues	any positive value
		\defaultvalue	\c 5.0f */
	void StuckDelay(KyFloat32 val) 
	{ 
		KY_FUNCTION("Goto_GapDynamicAvoidance::StuckDelay");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'StuckDelay' must be > 0.", val));
		m_stuckDelay = val;
	}

	/*! Retrieves the value set through a call to MaxAngularSpeed(KyFloat32). */
	KyFloat32 MaxAngularSpeed() const { return m_maxAngularSpeed; }

	/*! Determines the maximum rotational speed of the Bot, expressed as the maximum number of degrees per second that the Bot can turn. This
		is typically determined by the animations and modeling of the character in the \gameOrSim engine. The dynamic avoidance solver uses this
		value in order to select the best candidate point and to adapt the speed of the Bot to the selected point.
		\units			degrees per second
		\acceptedvalues	any positive value
		\defaultvalue	\c 360.0f */
	void MaxAngularSpeed(KyFloat32 val)
	{
		KY_FUNCTION("Goto_GapDynamicAvoidance::MaxAngularSpeed");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'MaxAngularSpeed' must be > 0.", val));
		m_maxAngularSpeed = val; 
	}

	/*! Retrieves the value set through a call to MaxCollisionTestsPerFrame(KyUInt32). */
	KyUInt32 MaxCollisionTestsPerFrame() const { return m_maxCollisionTestsPerFrame; }

	/*! Determines the maximum number of collision tests with the static 3D world that can be made in a single frame.
		\acceptedvalues	any positive value
		\defaultvalue	\c 1 */
	void MaxCollisionTestsPerFrame(KyUInt32 val)
	{
		KY_FUNCTION("Goto_GapDynamicAvoidance::MaxCollisionTestsPerFrame");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'MaxCollisionTestsPerFrame' must be > 0.", val));
		m_maxCollisionTestsPerFrame = val; 
	}

	/*! Retrieves the value set through a call to AllowCrowding(KyBool). */
	KyBool IsCrowdingAllowed() const { return m_allowCrowding; }

	/*! Determines whether or not Crowd mode may be used.
		\defaultvalue	#KY_FALSE */
	void AllowCrowding(KyBool val) { m_allowCrowding = val; }

	/*! Retrieves the value set through a call to ContinuousSpeed(KyBool). */
	KyBool ContinuousSpeed() const { return m_continuousSpeed; }

	/*! Determines whether or not the speed of the Bot may be raised or lowered freely in order to better avoid other \Bodies. 
		\pp When this value is set to #KY_FALSE, the solver must set the speed of the Bot to either 0 meters per second or to the
		BodyMaximumSpeed value set for the Bot.
		\pp When this value is set to #KY_TRUE, the solver may set the speed of the Bot to either 0 meters per second, or to any
		value in between the minimum set by MinSpeed(KyFloat32) and the BodyMaximumSpeed value set for the Bot. 
		\pp If you set this value to #KY_TRUE, you must also set an appropriate value for MinSpeed(KyFloat32).

		\acceptedvalues	any positive value
		\defaultvalue	#KY_FALSE */
	void ContinuousSpeed(KyBool val) { m_continuousSpeed = val; }

protected:
	// m_diagramHalfWidth is protected because some internal "constants" are computed
	// from its value at init. You can change it at runtime only through accessors.

	/*	Half the width (in meters) of the diagram. If it is too small, entities will not find 
		avoiding trajectories. If it is too large, CPU consumption will be increased and entities 
		will possibly make big detours.
	*/
	KyFloat32 m_diagramHalfWidth;

	KyFloat32 m_distMax; /*< \c DistMax configuration parameter. */
	KyFloat32 m_diagramMaxLength; /*< \c DiagramMaxLength configuration parameter. */
	KyFloat32 m_diagramRefreshPeriod; /*< \c DiagramRefreshPeriod configuration parameter. */
	KyFloat32 m_candidateSpacing; /*< \c CandidateSpacing configuration parameter. */
	KyFloat32 m_extraGap; /*< \c ExtraGap configuration parameter. */
	KyFloat32 m_minSpeed; /*< \c minSpeed configuration parameter. */
	KyFloat32 m_courtesy; /*< \c Courtesy configuration parameter. */
	KyFloat32 m_queueingDelay; /*< \c QueueingDelay configuration parameter. */
	KyFloat32 m_pushingDelay; /*< \c PushingDelay configuration parameter. */
	KyFloat32 m_stuckDelay; /* < \c StuckDelay configuration parameter. */
	KyUInt32 m_maxCollisionTestsPerFrame;/*< Accessor for the \c MaxCollisionTestsPerFrame configuration parameter. */
	KyFloat32 m_maxAngularSpeed; /*<Accessor for the \c MaxAngularSpeed configuration parameter. */

	KyBool m_allowCrowding; /*< \c Crowding configuration parameter. */
	KyBool m_continuousSpeed; /*< ContinuousSpeed configuration parameter. */

	// internal stuff
	static Bot* s_lastComputedBot;
	BodyInfoManager* m_bodyInfoManager;
	ObstacleManager* m_obstacleManager;
	GapManager* m_gapManager;
	FilteringFunction m_filterCB;
	Bot* m_me;
	Vec3f m_myPos;
	KyFloat32 m_myWidth;
	KyFloat32 m_myLength;
	KyBool m_isRectangularShaped;

	Vec3f m_targetPoint;
	Vec3f m_daTargetPoint;
	Vec3f m_originalPath_start; // used to get a intermediary point to rejoin the original path

	RefinedMode m_currentMode;
	RefinedMode m_nextMode;

	Vec3f m_diagramRight;
	Vec3f m_diagramUp;
	Vec3f m_diagramFront;
	KyFloat32 m_diagramTimeDepth;
	KyFloat32 m_diagramHalfHeight;
	KyFloat32 m_distToTarget;
	KyFloat32 m_nextDiagramUpdateTime;
	KyFloat32 m_nextPolarDiagramUpdateTime;

	KyFloat32 m_lastAbscissa;
	KyFloat32 m_currentAbscissa;
	KyFloat32 m_currentCost;
	KyFloat32 m_currentTti;
	KyFloat32 m_currentSpeed;
	KyFloat32 m_currentMinGapFrontalSpeed;
	CandidateList m_candidates;
	CandidateList::Iterator m_nextCandidateIt;

	KyBool m_forceDiagramGeneration;
	KyBool m_doCandidateGeneration;
	KyBool m_invalidateDATarget;

	KyFloat32 m_modeTimer;
	KyFloat32 m_diagramInvHalfWidth;
	KyFloat32 m_invPathfinderTargetSpeed;
	KyFloat32 m_myHalfLengthTimeHorizon;
	KyFloat32 m_courtesyTimeHorizon;

	OnDAModeChange m_onDAModeChange; //< CallBack used to notify the user of a mode change. Can be null.

	// Polar diagram: stores informations about imminent potential colliders in all directions
	List<PolarDiagramSector> m_polarDiagram;
	WorldObserver m_worldObserver;

public:
	// crowd module
	/*!	The module used to manage situations of high crowd density. For internal use. Do not modify. */
	CrowdModule m_crowdModule;

protected:
	virtual void OnSetPathFinder();
};

} // namespace Kaim

#endif // KY_GOTO_GAPDYNAMICAVOIDANCE_H
