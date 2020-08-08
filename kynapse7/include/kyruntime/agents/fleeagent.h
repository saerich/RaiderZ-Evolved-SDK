/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_agents */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_FLEEAGENT_H
#define KY_FLEEAGENT_H

#include <kyruntime/core/body.h>
#include <kyruntime/core/world.h>
#include <kyruntime/graph/graphconstraints.h>
#include <kyruntime/graph/pointinfo_vertexsafeptr.h>
#include <kyruntime/graph/astartraversal.h>

#include <kypathdata/generic/types.h>

namespace Kaim
{

//Forward declarations
class BodyInfo_Visibility;
class FleeAgent;
class FleeingTraversal;
class GraphManager;
class PathObjectManager;
class TraversalManager;


/*! This implementation of IConstraint is used by the FleeAgent in order to avoid dangerous points and dangerous \Bodies.
	-	When the FleeAgent computes a destination point, it uses the Constraint_FleeAgent in order to avoid having the
		initial path computed along the edges of the \GraphCells pass close to a dangerous point or dangerous Bot.
	-	When the FleeAgent is following its path, it uses the Constraint_FleeAgent in order to avoid following shortcuts
		that pass close to the dangerous points and dangerous \Bodies.
	\pp For details on using the FleeAgent, see :USERGUIDE:"Fleeing".
	\ingroup kyruntime_agents */
class Constraint_FleeAgent: public IConstraint
{
	friend class FleeAgent;
public:
	KY_DECLARE_CONSTRAINT(Constraint_FleeAgent)
public:
	/*! \label_constructor For internal use by the FleeAgent. */
	Constraint_FleeAgent();

	/*! Computes a cost scaling factor for the specified line segment. This factor is inversely proportional to the
		distance between the segment and a specified position, which typically represents a dangerous point or
		dangerous Body set for the FleeAgent. Segments that pass close to dangerous points and \Bodies receive a
		higher cost, while segments that are at a safe distance receive a lower cost.
		\param Bot*			A pointer to the Bot using the FleeAgent.
		\param position		The position of a dangerous point or dangerous Body.
		\param from			The starting point of the line segment.
		\param to			The ending point of the line segment. */
	KyFloat32 GetCostRatio(Bot*, const Vec3f& position, const Vec3f& from, const Vec3f& to) const;

	/*! \copydoc IConstraint::GetCost()
		\pp This implementation of IConstraint::GetCost() increases the cost of the line segment in inverse proportion
		to the distance between the segment and each dangerous point and each dangerous Bot set for the FleeAgent.
		Segments that pass close to dangerous points and Bodies receive a higher cost, while segments that are at a safe
		distance receive a lower cost. The extra factor applied to the cost of the segment for each dangerous point and
		each dangerous Bot is determined through iterative calls to GetCostRatio(). */
	KyBool GetCost(Bot* entity, const PointWrapper& from, const PointWrapper& to, KyFloat32& cost);
protected:
	FleeAgent* m_fleeAgent;
	KyFloat32 m_costRatioPerWatcher;
	KyFloat32 m_distanceSafe;
};


/*! The FleeAgent finds a hiding place for a Bot that is at a safe distance from a list of dangerous points and/or
	a list of dangerous \Bodies, and invokes the PathFinder to move the Bot toward that safe place.
	\pp For details on setting up and using this class, see :USERGUIDE:"Fleeing".
	\ingroup kyruntime_agents */
class FleeAgent
{
	KY_DEFINE_NEW_DELETE_OPERATORS
	friend class Constraint_FleeAgent;
	friend class FleeingTraversal;

	FleeAgent operator= (const FleeAgent&);

	/* The FleeAgent creates and initializes an instance of this class to ensure that any Body that is removed 
		from the World is also removed from the list of dangerous \Bodies maintained by the FleeAgent.
		\ingroup kyruntime_agents */
	class WorldObserver : public IWorldObserver
	{
	public:
		/* Default constructor. For internal use. */
		WorldObserver() : m_fleeAgent(KY_NULL) {}
		
		/* Sets up the WorldObserver for use. For internal use. 
			\param fleeAgent		The instance of the FleeAgent whose list of dangerous \Bodies will be updated. */
		void Initialize(FleeAgent& fleeAgent) { m_fleeAgent = &fleeAgent; }

		virtual void OnActivateBody(Body& ) {}
		/* This implementation of IWorldObserver::OnActivateBody() tests each Body removed from the World to see if
			it is in the list of dangerous \Bodies maintained by the FleeAgent. If so, the Body is removed from the list. */
		virtual void OnDeactivateBody(Body& body)
		{
			m_fleeAgent->RemoveDangerousBody(body);
		}
	protected:
		FleeAgent* m_fleeAgent;
	};

public:
	/*! The FleeAgent uses this class internally to represent the areas of danger registered through 
		calls to FleeAgent::AddDangerousPoint(). You should not need to interact with this class directly.
		\ingroup kyruntime_agents */
	class DangerousArea
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
	public:
		/* Default constructor. For internal use. */
		DangerousArea() : m_sqRadius(0.f) {}
		
		/* Constructor. For internal use.
			\param pos			Sets the value of #m_pos.
			\param sqRadius		Sets the value of #m_sqRadius. */
		DangerousArea(const Vec3f& pos, KyFloat32 sqRadius) : m_pos(pos), m_sqRadius(sqRadius) {}
	public:
		/* Indicates whether or not the specified position lies within this DangerousArea. */
		bool IsInArea(const Vec3f& pos) const;

		/* Renders this DangerousArea. The position is drawn using the specified colors, and the radius is drawn in dark red.
			\param r	\label_redval
			\param g	\label_greenval
			\param b	\label_blueval */
		void Render(KyUInt32 r, KyUInt32 g, KyUInt32 b) const;
	public:
		Vec3f m_pos; /*< The coordinates of the dangerous point. */
		KyFloat32 m_sqRadius; /*< A squared radius value that indicates the danger radius of the point. */
	};

public:
	//////////////////////////////////////////////////////////////////////////////////////////
	// STANDARD AGENT FUNCTIONS
	//////////////////////////////////////////////////////////////////////////////////////////

	/*! \label_constructor \label_mustinit */
	FleeAgent();

	/*! \label_destructor */
	~FleeAgent();

	/*! \label_init 
		\param bot					The Bot that will use the FleeAgent.
		\param traversalConfigId	The unique ID of a FleeingTraversal instance configured for the TraversalManager.
		\param checkDangerVisibleId	The unique ID of a periodic task configured for the TraversalManager, which will be used
									to determine the frequency with which the FleeAgent tests the safety of its current position. */
	KyResult Initialize(Bot& bot, TraversalManager::ConfigId traversalConfigId, TimeManager::TaskId checkDangerVisibleId);

	/*! \label_clear \label_mustinit */
	void Clear();

	/*! \label_reinit */
	void ReInit();

	/*! Computes a destination hidden from the list of dangerous points and dangerous \Bodies, and invokes
		the PathFinder to generate an Action that moves the Bot toward that destination. */
	KyBool Think();

	//////////////////////////////////////////////////////////////////////////////////////////
	// SPECIFIC FUNCTIONS
	//////////////////////////////////////////////////////////////////////////////////////////

	/*! Clears the list of dangerous points. */
	void ClearDangerousPoints();

	/*! Adds the specified point to the list of dangerous points maintained by the FleeAgent.
		\param point				The coordinates of the dangerous point.
		\param radius				A radius around \c point that defines the limit of the dangerous area. */
	void AddDangerousPoint(const Vec3f& point, const KyFloat32 radius = KyFloat32MAXVAL);

	/*! Retrieves the number of dangerous points currently in the list maintained by the FleeAgent. */
	KyUInt32 GetDangerousPointsCount() const { return m_points.GetCount(); }

	/*! Retrieves the dangerous point at a specified index within the list maintained by the FleeAgent.
		\param index						The index of the dangerous point to retrieve.
		\param[out] point					Used to store the position of the retrieved danger point.
		\param[out] squareHazardousRadius	Used to store the danger radius of the retrieved danger point. */
	void GetDangerousPoint(KyUInt32 index, Vec3f& point, KyFloat32& squareHazardousRadius) const
	{
		KY_FUNCTION("FleeAgent::GetDangerousPoint");
		KY_ASSERT((index < m_points.GetCount()),("Index is bigger than number of dangerous points"));

		point = m_points[index].m_pos;
		squareHazardousRadius = m_points[index].m_sqRadius;
	}

	/*! Clears the list of dangerous \Bodies. */
	void ClearDangerousBodies();

	/*! Adds the specified Body to the list of dangerous \Bodies maintained by the FleeAgent.
		\return \c true if the Body was successfully added, or \c false otherwise. \c false may indicate that
				the specified \c body is the same Body owned by the Bot using the FleeAgent. */
	bool AddDangerousBody(const Body& body);

	/*! Removes the specified Body from the list of dangerous \Bodies maintained by the FleeAgent.
		\return \c true if the Body was successfully removed, or \c false otherwise. */
	bool RemoveDangerousBody(const Body& body);

	/*! Retrieves the number of dangerous \Bodies currently in the list maintained by the FleeAgent. */
	KyUInt32 GetDangerousBodiesCount() const {return m_bodies.GetCount();}

	/*! Retrieves the dangerous Body at a specified index within the list maintained by the FleeAgent.
		\param index				The index of the dangerous Body to retrieve.*/
	const Body* GetDangerousBody(KyUInt32 index) const
	{
		KY_FUNCTION("FleeAgent::GetDangerousBody");
		KY_ASSERT((index < m_bodies.GetCount()),("Index is bigger than number of dangerous bodies"));
		return m_bodies[index];
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// ACCESSORS
	//////////////////////////////////////////////////////////////////////////////////////////

	/*!	Determines the interval between successive calculations of new destinations.
		\units			seconds
		\acceptedvalues	any positive value
		\defaultvalue	\c 1.0*/
	void SetComputeDestInterval (KyFloat32 computeDestInterval)
	{
		KY_FUNCTION("SetComputeDestInterval");
		KY_AFFECT_STRICT_POSITIVE_FLOAT("ComputeDestInterval", m_computeDestInterval, computeDestInterval);
	}

	/*!	Retrieves the value set through a call to SetComputeDestInterval(). */
	KyFloat32 GetComputeDestInterval() const { return m_computeDestInterval; }

	/*!	Determines whether the FleeAgent stops fleeing as soon as it reaches a safe position, or whether it
		always continues fleeing until it reaches the destination point chosen by the FleeingTraversal.
		\defaultvalue \c true */
	void SetStopWhenHidden(KyBool stopWhenHidden) { m_stopWhenHidden = stopWhenHidden; }

	/*!	Retrieves the value set through a call to SetStopWhenHidden(). */
	KyBool GetStopWhenHidden() const { return m_stopWhenHidden; }

	/*!	Determines the minimum distance that the destination point must be from each dangerous Body, and
		the minimum distance that the FleeAgent must be from each dangerous Body in order for it to 
		consider its current position safe.
		\units			meters
		\acceptedvalues	any positive value
		\defaultvalue	\c 50.0f */
	void SetDangerRadius(KyFloat32 radius);

	/*!	Retrieves the value set through a call to SetDangerRadius(). */
	KyFloat32 GetDangerRadius() const {return m_dangerRadius;}

	/*!	Determines the maximum distance that the FleeingTraversal will propagate from the current position
		of the FleeAgent when searching for a hidden destination point. In other worlds, this value 
		determines the farthest that any potential destination point may be from the FleeAgent.
		\units			meters
		\acceptedvalues	any positive value
		\defaultvalue	\c 50.0f */
	void SetExplorationRadius(KyFloat32 explorationRadius) { m_explorationRadius = explorationRadius; }

	/*!	Retrieves the value set through a call to SetExplorationRadius(). */
	KyFloat32 GetExplorationRadius() const { return m_explorationRadius; }

	/*!	Sets the GraphManager that contains the set of \GraphCells that the FleeAgent should search for a 
		hidden destination point. */
	void SetGraph(GraphManager& graphManager);

	/*!	Retrieves the value set through a call to SetGraph(). */
	const GraphManager* GetGraph() const { return m_graphManager; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// SPEED MANAGEMENT
	//////////////////////////////////////////////////////////////////////////////////////////

	/*!	Determines the preferred speed at which the FleeAgent should move. This preferred speed is capped to
		the value of the BodyMaxSpeed attribute set for the Bot using the FleeAgent, if any. In 
		addition, it may be altered by the current set of IModifier classes used by the PathFinder.
		\units			meters per second
		\acceptedvalues	any positive value
		\defaultvalue	The value of the BodyMaxSpeed attribute of the Bot if available, or \c 1.0f otherwise. */
	void SetSpeed(const KyFloat32 speed);

	/*!	Retrieves the value set through a call to SetSpeed(). */
	KyFloat32 GetSpeed() const { return m_speed; }

	//////////////////////////////////////////////////////////////////////////////////////////
	// AGENT STATE FEEDBACK
	//////////////////////////////////////////////////////////////////////////////////////////

	/*!	Retrieves the current destination point of the FleeAgent. */
	PointWrapper GetDestination() const { return m_destination; }

	/*!	Indicates whether or not the FleeAgent has arrived at its destination point. */
	KyBool HasArrived() const {return m_hasArrived;}

	/*!	Indicates whether or not the FleeAgent is currently computing a new destination point. */
	KyBool IsComputing() const {return m_traversal != KY_NULL;}

	/*!	Indicates whether or not the FleeAgent currently considers itself in danger. */
	KyBool IsInDanger() const {return !m_safe;}

	/*!	Returns the date at which the FleeAgent last checked whether or not it is in danger. */
	KyFloat32 GetLastDangerUpdateTime() const {return m_lastDangerUpdateTime;}

	/*!	Renders debug information about the FleeAgent.
		\param drawTargetPoint		Determines whether or not the destination point of the FleeAgent is rendered. If #KY_TRUE,
									draws a green point at the destination, and a blue line between the FleeAgent and the 
									destination.
		\param drawUnsafeArea		Determines whether or not the danger areas of the FleeAgent are rendered. If #KY_TRUE, draws
									a red point at the position of each dangerous Body and each dangerous point, and a
									red circle to mark the radius of each dangerous Body and each dangerous point. Also
									draws a green point above the head of the Bot using the FleeAgent if it considers itself safe, or a red point
									if it considers itself in danger.
		\param drawTraversal		Determines whether or not the FleeingTraversal used by the FleeAgent is drawn. Note that
									the FleeingTraversal is often too fast to be seen. */
	void Render(KyBool drawTargetPoint,
				KyBool drawUnsafeArea,
				KyBool drawTraversal) const;


	//////////////////////////////////////////////////////////////////////////////////////////
	// SPECIFIC FUNCTIONS
	//////////////////////////////////////////////////////////////////////////////////////////
private:
	KyBool ComputeDestination();
	KyBool IsCurrentPositionSafe() const;
	void ReleaseTraversal();

	/* Returns visibility between the entity driven by the agent and
		given entity. This function tries to use BodyInfoManager if possible, or
		uses ray cast test otherwise.*/
	KyBool IsVisible(const Body& body) const;

	/* Returns distance between the entity driven by the agent and
		given entity.*/
	KyFloat32 GetDistanceFrom(const Body& body) const
	{
		return Distance(m_bot->GetBody().GetPosition(), body.GetPosition());
	}

	/* Returns square distance between the entity driven by the agent and
		given entity.*/
	KyFloat32 GetSquareDistanceFrom(const Body& body) const
	{
		return SquareDistance(m_bot->GetBody().GetPosition(), body.GetPosition());
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// AGENT MEMBER VALUES
	//////////////////////////////////////////////////////////////////////////////////////////
private:
	WorldObserver m_worldObserver;

	Bot* m_bot;

	// Behaviour data
	KyBool m_stopWhenHidden; //< Boolean flag. When activated, the agent will make the entity stop as soon as it is hidden from its dangerous items
	KyFloat32 m_dangerRadius; //< Contains the threshold distance beyond which dangerous points/bodies are ignored

	// Information classes
	BodyInfo_Visibility* m_visibilityInfo;

	// Time-Sliced Traversal
	FleeingTraversal* m_traversal;
	TraversalManager::ConfigId m_traversalConfigId;
	KyFloat32 m_explorationRadius;

	// Dangerous items
	Array<DangerousArea> m_points; //< Contains dangerous static points (e.g. a biohazard area)
	Array<const Body*> m_bodies; //< Contains dynamic dangerous bodies (e.g. an enemy guard)
	Constraint_FleeAgent m_constraintFlee;

	// Chosen destination
	KyFloat32 m_minDangerLevel; //< The danger level of chosen point 
	PointWrapper m_destination;
	PointInfo_VertexSafePtr m_destinationSafePtr;
	KyBool m_hasADestination; //< Boolean flag activated when agent chose a goal point
	Vec3f m_oldPosition; //< A 3D vector where entity position is stored

	// Timers
	KyFloat32 m_nextComputationDate; //< Contains the date when a new destination computation should be made
	KyFloat32 m_computeDestInterval; //< A minimal time interval between 2 destination computations

	// Time-slicing task IDs
	TimeManager::TaskId m_checkDangerVisibleId; //!< The ID of periodic task "FleeAgent::CheckDangerVisible"

	// State data
	KyBool m_safe; //< Boolean flag activated when entity's position is safe
	KyBool m_hasArrived; //< Boolean flag activated when entity reached its target point
	KyFloat32 m_lastDangerUpdateTime; //< Time when danger status was last updated
	KyFloat32 m_speed; //< Contains agents move speed

	// Fleeing graph
	GraphManager* m_graphManager;//< Currently used graph

	// Bias that is used to find a fleeing point
	PropagationBias_Zero m_bias;
};

} // namespace Kaim

#endif // KY_FLEEAGENT_H
