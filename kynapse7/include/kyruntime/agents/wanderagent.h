/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_agents */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_WANDERAGENT_H
#define KY_WANDERAGENT_H


#include <kyruntime/agents/wandertraversal.h>
#include <kyruntime/graph/traversalmanager.h>

#include <kyruntime/core/pointwrapper.h>
#include <kyruntime/graph/pointinfo_vertexsafeptr.h>


namespace Kaim
{

class GraphManager;
class PathFinder;



/*!	The WanderAgent makes a Bot wander around the world at random. Each time it reaches a destination, it chooses a
	new vertex at random and moves there, repeating the process indefinitely.
	\pp For details on using the WanderAgent, see :USERGUIDE:"Wandering".
	\ingroup kyruntime_agents */
class WanderAgent
{
	KY_DEFINE_NEW_DELETE_OPERATORS

	friend class WanderTraversal;

public:
	/*! \label_constructor \label_mustinit */
	WanderAgent();

	/*! \label_destructor */
	~WanderAgent();

	/*! \label_init 
		\param bot							The Bot that will use the HideAgent.
		\param traversalConfigId			The unique ID of a WanderTraversal instance configured for the TraversalManager. */
	KyResult Initialize(Bot& bot, TraversalManager::ConfigId traversalConfigId);

	/*! \label_clear \label_mustinit */
	void Clear();

	/*! \label_reinit */
	void ReInit();
	
	/*!	Invokes the WanderTraversal to pick a destination at random, and invokes the PathFinder to move the Bot
		toward that destination. */
	KyBool Think();

	/*!	Sets the GraphManager that contains the set of \GraphCells that the WanderAgent should search for a 
		hidden destination point. */
	void SetGraph(GraphManager& graphManager);
	
	/*!	Retrieves the value set through a call to SetGraph(). */
	const GraphManager* GetGraph() const { return m_graphManager; }
	
	/*!	Retrieves the WanderTraversal currently in use by the WanderAgent, or #KY_NULL if the WanderAgent is not currently
		using a WanderTraversal. */
	WanderTraversal* GetTraversal() const { return m_traversal; }
	
	/*!	Determines the preferred speed at which the WanderAgent should move. This preferred speed is capped to
		the value of the BodyMaxSpeed attribute set for the Bot using the WanderAgent, if any. In 
		addition, it may be altered by the current set of IModifier classes used by the PathFinder.
		\units			meters per second
		\acceptedvalues	any positive value
		\defaultvalue	The value of the BodyMaxSpeed attribute of the Bot if available, or \c 1.0f otherwise. */
	void SetSpeed(const KyFloat32 speed);
	
	/*!	Retrieves the value set through a call to SetSpeed(). */
	KyFloat32 GetSpeed() const { return m_speed; }

	/*!	Retrieves the destination chosen by the WanderAgent, or #KY_NULL if it has not yet found a random destination. */
	const PointWrapper& GetDestination() const { return m_destination; }
	
	/*!	Indicates whether or not the WanderAgent has arrived at its chosen destination. */
	KyBool HasArrived() const { return m_hasArrived;}
	
	/*!	Retrieves the time at which the WanderAgent last calculated its destination. */
	KyFloat32 GetDestinationTimeStamp() const { return m_destinationTimeStamp; }

private:
	KyBool ComputeDestination();


private:
	Bot* m_bot;

	// Time-Sliced Traversal 
	WanderTraversal* m_traversal;
	TraversalManager::ConfigId m_traversalConfigId;

	// Chosen destination
	KyFloat32 m_destinationTimeStamp; //<Contains the date when destination was computed
	PointWrapper m_destination;
	PointInfo_VertexSafePtr m_destinationSafePtr;

	// Hiding graph and related FindNearestData
	GraphManager* m_graphManager; //<Currently used graph

	// Bias that is used to find a wander point
	PropagationBias_Wander m_bias;

	// State data
	KyBool m_hasArrived; //<Boolean flag activated when entity reached its target point
	KyFloat32 m_speed; //< Contains agents move speed
};

} //Kaim

#endif // KY_WANDERAGENT_H
