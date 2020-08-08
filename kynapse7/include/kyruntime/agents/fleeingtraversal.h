/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_agents */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_FLEEINGTRAVERSAL_H
#define KY_FLEEINGTRAVERSAL_H

#include <kyruntime/graph/astartraversal.h>


namespace Kaim
{

class FleeAgent;


/*!	This implementation of AstarTraversal is used internally by the FleeAgent to find a destination point
	that lies at a safe distance from all dangerous points and dangerous \Bodies maintained by the FleeAgent.
	\pp This class is closely coupled with the FleeAgent. For details, see :USERGUIDE:"Fleeing".
	\ingroup kyruntime_agents */
class FleeingTraversal : public AstarTraversal
{
public:
	KY_DECLARE_TRAVERSAL(FleeingTraversal)

public:
	///////////////////////////////////////////////////////////////////
	// STANDARD TRAVERSAL FUNCTIONS
	///////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////
	// SPECIFIC FUNCTIONS
	///////////////////////////////////////////////////////////////////

	/*!	Sets the FleeAgent using the FleeingTraversal. */
	void SetFleeAgent(const FleeAgent& agent);

	/*!	Verifies that this FleeingTraversal has been correctly initialized. Call this function 
		after initializing the FleeingTraversal, but before launching a computation.
		\return #KY_TRUE if the FleeingTraversal is correctly initialized, or #KY_FALSE otherwise. */
	KyBool IsProperlyInitialized();

	/*!	Launches the FleeingTraversal to compute a destination point for the FleeAgent.
		\return #KY_TRUE if the computation has been completed, or #KY_FALSE otherwise. */
	KyBool ComputeFleeingDestination();

	// Result accessor functions

	/*!	Retrieves the lowest danger level computed for any vertex processed so far. 
		The lower the value, the safer the point. */
	KyFloat32 GetMinDangerLevel() const { return m_minDangerLevel; }

	/*!	Retrieves the safest vertex processed so far. */
	GraphVertexPtr GetSafestDestination() const;

	/*!	Resets the FleeingTraversal. Call this function after initializing the FleeingTraversal, but before
		launching the FleeingTraversal. */
	void Start();

	/*!	Retrieves the total number of edges and vertices processed by the FleeingTraversal since its
		initialization. Used to compute an estimation for deterministic time measurement mode.
		See :USERGUIDE:"Using Deterministic Time Measurement". */
	static KyUInt32 GetCallCounter();

protected:
	~FleeingTraversal();


	// Danger level evaluation functions

	/*	Called internally by ProcessVertex() to assign a score to each candidate vertex.
		This method determines the number of outgoing edges that lead from the vertex, and calls
		EvaluateDestination(const Vec3f&, KyBool) to determine the final score. */
	KyFloat32 EvaluateDestination(const GraphVertexPtr& destination) const;

	/*	Called internally by EvaluateDestination(const GraphVertexPtr*) to assign a score to each candidate vertex.
		This method prohibits vertices with only one outgoing edge, to prevent the Agent from going into
		dead ends. */
	KyFloat32 EvaluateDestination(const Vec3f& destination, KyBool oneOutEdge) const;


	/*!	\copydoc GraphTraversal::ProcessEdge()
		\pp This implementation of GraphTraversal::ProcessEdge() simply calls ProcessVertex() on the vertex
		that lies at the end of the edge. No other evaluation of the edge is performed. */
	virtual KyBool ProcessEdge(const GraphEdgePtr& edge);

	/*!	\copydoc GraphTraversal::ProcessVertex()
		\pp This implementation of GraphTraversal::ProcessVertex() calculates a score for each candidate vertex 
		based on its distance from the dangerous points and dangerous \Bodies set for the FleeAgent using 
		this FleeingTraversal. The vertex with the best score is kept. */
	virtual KyBool ProcessVertex(const GraphVertexPtr& vertex);

	// Increase the call counter
	virtual void UpdateCallCounter();

protected:
	friend class FleeingTraversalConfig;

	/*	For internal use. */
	FleeingTraversal();


	// Input data
	const FleeAgent* m_fleeAgent;//<The agent requiring the traversal computation.
	KyBool m_initialized;//<This flag is activated during IsProperlyInitialized() call.

	// Output data
	KyFloat32 m_minDangerLevel;//<The danger level to the most interesting destination.
	GraphVertexSafePtr m_safestDestination;//< The safest destination.

	// Computation data
	KyFloat32 m_currentPositionDangerLevel;//< This value contains current entity position danger level.
	KyFloat32 m_startingVertexDangerLevel;//< This value contains traversal starting vertex danger level.
	GraphVertexSafePtr m_safestTraversedVertex;//< This value is updated when evaluating graph vertices

};

} // namespace Kaim

#endif // KY_FLEEINGTRAVERSAL_H
