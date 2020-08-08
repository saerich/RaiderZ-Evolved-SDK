/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_agents */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_HIDEAGENT_H
#define KY_HIDEAGENT_H

#include <kyruntime/agents/hidingtraversal.h>
#include <kyruntime/core/world.h>

namespace Kaim
{

class PathFinder;
class TraversalManager;


/*!	The HideAgent makes a Bot locate and move to the nearest unoccupied point that is hidden from view from a list of
	dangerous points and dangerous \Bodies.
	\pp For details on setting up and using this class, see :USERGUIDE:"Hiding".
	\ingroup kyruntime_agents */
class HideAgent
{
	KY_DEFINE_NEW_DELETE_OPERATORS

	// We give friendship to hiding traversal class to allow it access agent data
	friend class HidingTraversal;

	HideAgent operator= (const HideAgent&);

	/* The HideAgent creates and initializes an instance of this class to ensure that any Body that is removed 
		from the World is also removed from the list of dangerous \Bodies maintained by the HideAgent.
		\ingroup kyruntime_agents */
	class WorldObserver : public IWorldObserver
	{
	public:
		/* Default constructor. */
		WorldObserver() : m_hideAgent(KY_NULL) {}
		
		/* Sets up the WorldObserver for use. 
			\param hideAgent		The instance of the HideAgent whose list of dangerous \Bodies will be updated. */
		void Initialize(HideAgent& hideAgent) { m_hideAgent = &hideAgent; }

		void OnActivateBody(Body& ) {}
		/* This implementation of IWorldObserver::OnActivateBody() tests each Body removed from the World to see if
			it is in the list of dangerous \Bodies maintained by the HideAgent. If so, the Body is removed from the list. */
		void OnDeactivateBody(Body& body)
		{
			m_hideAgent->RemoveDangerousBody(body);
		}
	protected:
		HideAgent* m_hideAgent;
	};


public:
	//////////////////////////////////////////////////////////////////////////////////////////
	// STANDARD AGENT FUNCTIONS
	//////////////////////////////////////////////////////////////////////////////////////////

	/*! \label_constructor \label_mustinit */
	HideAgent();
	
	/*! \label_destructor */
	~HideAgent();

	/*! \label_init 
		\param bot							The Bot that will use the HideAgent.
		\param traversalConfigId			The unique ID of a HidingTraversal instance configured for the TraversalManager.
		\param reconsiderHidingPlaceTaskId	The unique ID of an aperiodic task configured for the TraversalManager, which will be used
											to determine whether or not enough time remains in the current frame for the HideAgent to 
											check that its chosen hiding place is still hidden.
		\param pointLockManager				The PointLockManager configured for the World. Optional. */
	KyResult Initialize(Bot& bot, TraversalManager::ConfigId traversalConfigId, TimeManager::TaskId reconsiderHidingPlaceTaskId, PointLockManager* pointLockManager=KY_NULL);

	/*! \label_reinit */
	void ReInit();

	/*! \label_clear \label_mustinit */
	void Clear();

	/*!	Invokes the HidingTraversal to compute a destination point that is hidden from view by all dangerous
		points and dangerous \Bodies configured for the HideAgent, and invokes the PathFinder to move the Bot
		using the HideAgent toward that destination. */
	KyBool Think();

	//////////////////////////////////////////////////////////////////////////////////////////
	// SPECIFIC FUNCTIONS
	//////////////////////////////////////////////////////////////////////////////////////////

	/*!	Clears the list of dangerous points. */
	void ClearDangerousPoints();
	
	/*!	Adds a dangerous point to the list maintained by the HideAgent.
		\param point				The coordinates of the dangerous point.
		\return #KY_TRUE if the point was successfully added, or #KY_FALSE otherwise. #KY_FALSE may indicate that
				the list of dangerous points is full. */
	KyBool AddDangerousPoint(const Vec3f& point);
	
	/*!	Retrieves the number of dangerous points currently in the list maintained by the HideAgent. */
	KyUInt32 GetDangerousPointsCount() const {return m_points.GetCount();}
	
	/*!	Retrieves the dangerous point at a specified index within the list maintained by the HideAgent.
		\param index				The index of the dangerous point to retrieve.
		\param[out] point			The position of the dangerous point that matches the specified index. */
	void GetDangerousPoint(KyUInt32 index, Vec3f& point) const
	{
		KY_FUNCTION("HideAgent::GetDangerousPoint");
		KY_ASSERT((index < m_points.GetCount()),("Index is bigger than number of dangerous points"));
		point = m_points[index];
	}

	/*!	Clears the list of dangerous \Bodies. */
	void ClearDangerousBodies();

	/*! Adds a dangerous Body to the list maintained by the HideAgent.
		\return #true if the Body was successfully added, or #false otherwise. */
	bool AddDangerousBody(const Body& body);

	/*! Removes a dangerous Body to the list maintained by the HideAgent.
		\return #true if the Body was successfully removeded, or #false otherwise. */
	bool RemoveDangerousBody(const Body& body);

	/*!	Retrieves the number of dangerous \Bodies currently in the list maintained by the HideAgent. */
	KyUInt32 GetDangerousBodiesCount() const { return m_bodies.GetCount(); }
	
	/*!	Retrieves the dangerous Body at a specified index within the list maintained by the HideAgent.
		\param index				The index of the dangerous Body to retrieve. */
	const Body* GetDangerousBody(KyUInt32 index) const
	{
		KY_FUNCTION("HideAgent::GetDangerousBody");
		KY_ASSERT((index < m_bodies.GetCount()),("Index is bigger than number of dangerous bodies"));
		return m_bodies[index];
	}

	/*!	Sets a callback function that will be invoked to test for visibility between the position of the HideAgent
		and the position of each dangerous point. By default, the HideAgent is initialized to
		use HidingTraversal::BasicPointVisibility(). */
	void SetPointVisibilityFunction(HidingTraversal::CheckPointVisibility func, Body* body) { 
		m_pointVisibilityFunc = func;
		m_pointVisibleParam = body;
	}
	
	/*!	Retrieves the callback function invoked to test for visibility between the position of the HideAgent
		and the position of each dangerous point. */
	HidingTraversal::CheckPointVisibility GetPointVisibilityFunction() const { return m_pointVisibilityFunc; }

	/*!	Sets a callback function that will be invoked to test for visibility between the position of the HideAgent
		and the position of each dangerous Body. By default, the HideAgent is initialized to
		use HidingTraversal::BasicBodyVisibility(). */
	void SetBodyVisibilityFunction(HidingTraversal::CheckBodyVisibility func, Body* body) { 
		m_bodyVisibleFunc = func;
		m_bodyVisibleParam = body;
	}
	
	/*!	Retrieves the callback function invoked to test for visibility between the position of the HideAgent
		and the position of each dangerous Body. */
	HidingTraversal::CheckBodyVisibility GetBodyVisibilityFunction() const { return m_bodyVisibleFunc; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// ACCESSORS
	//////////////////////////////////////////////////////////////////////////////////////////

	/*!	Sets the GraphManager that contains the \GraphCells that the HideAgent should search for a 
		hidden destination point. */
	void SetGraph(GraphManager& graphManager);

	/*!	Retrieves the value set through a call to SetGraph(). */
	const GraphManager* GetGraph() const { return m_graphManager; }

	/*!	Retrieves the HidingTraversal currently in use by the HideAgent, or #KY_NULL if the HideAgent is not currently
		using a HidingTraversal. */
	HidingTraversal* GetTraversal() const { return m_traversal; }

	/*!	Determines the preferred speed at which the HideAgent should move. This preferred speed is capped to
		the value of the BodyMaxSpeed attribute set for the Bot using the HideAgent, if any. In 
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

	/*!	Returns the date at which the HideAgent last updated its chosen hiding place. */
	KyFloat32 GetHidingPlaceDate() const { return m_hidingPlaceDate; }
	
	/*!	Retrieves the hiding place chosen by the HideAgent, or #KY_NULL if it has not yet found a safe hiding place. */
	PointWrapper GetDestination() const { return m_destination; }
	
	/*!	Indicates whether or not the HideAgent is currently hidden from view by all dangerous points and dangerous \Bodies. */
	KyBool IsHidden() const;
	
	/*!	Indicates whether or not the HideAgent has arrived at its chosen hiding place. */
	KyBool HasArrived() const { return m_hasArrived;}

	/*!	Renders debug information about the HideAgent.
		\param drawTargetPoint		Determines whether or not the destination point of the HideAgent is rendered. If #KY_TRUE,
									draws a green point at the destination, and a blue line between the HideAgent and the 
									destination.
		\param drawDangerousArea	Determines whether or not the visibility of the dangerous points and dangerous \Bodies
									are rendered. If #KY_TRUE, draws a line from the position of each dangerous Body and 
									each dangerous point to the position of the HideAgent. If the HideAgent is considered visible to
									that point or Body, the line is drawn in red, otherwise the line is drawn in green.
		\param drawTraversal		Determines whether or not the HidingTraversal used by the HideAgent is drawn. Note that
									the HidingTraversal is often too fast to be seen. */
	void Render(KyBool drawTargetPoint = KY_TRUE, KyBool drawDangerousArea = KY_TRUE, KyBool drawTraversal = KY_FALSE) const;

private:
	//////////////////////////////////////////////////////////////////////////////////////////
	// SPECIFIC FUNCTIONS
	//////////////////////////////////////////////////////////////////////////////////////////

	/*	Computes m_hidingPlace, using a hiding traversal. */
	KyBool ComputeHidingPlace();
	/*	Indicates whether the current hiding place is still valid (hidden). */
	KyBool StillSafe();

	//////////////////////////////////////////////////////////////////////////////////////////
	// AGENT MEMBER VALUES
	//////////////////////////////////////////////////////////////////////////////////////////
private:
	Bot* m_bot;

	WorldObserver m_worldObserver;
	// Visibility functions
	HidingTraversal::CheckPointVisibility m_pointVisibilityFunc;
	HidingTraversal::CheckBodyVisibility m_bodyVisibleFunc;
	Body* m_pointVisibleParam;
	Body* m_bodyVisibleParam;

	// Services
	PointLockManager* m_pointLockManager;

	// Time-Sliced Traversal 
	HidingTraversal* m_traversal;
	TraversalManager::ConfigId m_traversalConfigId;
	TimeManager::TaskId m_reconsiderHidingPlaceTaskId;

	// Dangerous items
	Array<Vec3f> m_points; //< Contains dangerous static points (e.g. a gun nest)
	Array<Body*> m_bodies; //<Contains dynamic dangerous entities (e.g. an enemy guard)

	// Chosen hiding place
	KyFloat32 m_hidingPlaceDate;//<Contains the date when destination was last computed
	PointWrapper m_destination;
	PointInfo_VertexSafePtr m_destinationSafePtr;

	// Hiding graph and related FindNearestData
	GraphManager* m_graphManager;//<Currently used graph

	// Heuristic that is used to find a hiding point
	PropagationBias_Zero m_bias;

	// State data
	KyBool m_hasArrived; //<Boolean flag activated when body reached its target point
	KyFloat32 m_speed; //< Contains agents move speed
};

} // Kaim

#endif // KY_HIDEAGENT_H
