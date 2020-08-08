/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_agents */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_WANDERTRAVERSAL_H
#define KY_WANDERTRAVERSAL_H

#include <kyruntime/graph/astartraversal.h>
#include <kyruntime/graph/pointinfo_vertexsafeptr.h>


namespace Kaim
{

class WanderAgent;
class World;
class WanderTraversalConfig;

/*!	This implementation of the AstarTraversal is used by the WanderAgent to select a random destination point
	in the World. 
	\pp This class is closely coupled with the WanderAgent. For details, see :USERGUIDE:"Wandering".
	\ingroup kyruntime_agents */
class WanderTraversal: public AstarTraversal
{
public:
	KY_DECLARE_TRAVERSAL(WanderTraversal)


public:
	/*!	\label_constructor \label_mustinit For internal use. */
	WanderTraversal();
	
	/*!	\label_destructor */
	~WanderTraversal();

	/*!	\label_init For internal use. */
	KyResult Initialize(Kaim::TraversalManager& traversalManager, const WanderTraversalConfig& config);

	virtual void Start();

	/*!	Launches the WanderTraversal to compute a destination point for the WanderAgent.
		\return #KY_TRUE if the computation has been completed, or #KY_FALSE otherwise. */
	KyBool ComputeDestination();

	/*!	Retrieves the final destination decided on by the WanderTraversal. */
	const PointWrapper& GetDestination() const { return m_destination; }

	/* Not used. */
	virtual KyBool EvaluateEdge(const GraphEdgePtr& edge, KyFloat32& cost);

	/*!	Retrieves the total number of edges and vertices processed by the WanderTraversal since its
		initialization. Used to compute an estimation for deterministic time measurement mode.
		See :USERGUIDE:"Using Deterministic Time Measurement". */
	static KyUInt32 GetCallCounter();

	/*!	\copydoc WanderTraversalConfig::m_wanderDistance
		\return #KY_TRUE if the value was successfully set, or #KY_FALSE otherwise. #KY_FALSE may indicate that the
				specified value is a negative number. */
	KyBool SetWanderDistance(KyFloat32 distance);

	/*!	Retrieves the value set through a call to SetWanderDistance(). */
	KyFloat32 GetWanderDistance() const;

	/*!	\copydoc WanderTraversalConfig::m_wanderPropagationAngle */
	void SetPropagationAngle(KyFloat32 angle);
	
	/*!	Retrieves the value set through a call to SetPropagationAngle(). */
	KyFloat32 GetPropagationAngle() const;

protected:
	/*!	\copydoc GraphTraversal::ProcessEdge()
		\pp This implementation of GraphTraversal::ProcessEdge() simply calls ProcessVertex() for the vertex
		that lies at the end of the specified edge. No evaluation of the edge itself is performed. */
	virtual KyBool ProcessEdge(const GraphEdgePtr& edge);

	/*!	\copydoc GraphTraversal::ProcessVertex()
		\pp This implementation of GraphTraversal::ProcessVertex() evaluates each vertex as follows:
		-	If the vertex is farther from the starting vertex than the value set through the last call to 
			SetWanderDistance(), and it is not blocked by any dynamic obstacles, the candidate vertex is
			accepted.
		-	If not, the vertex is assigned a score based on its distance from the starting vertex, whether it
			lies within the cone defined by the last call to SetPropagationAngle(), and a random
			factor. If no ideal vertex is found, the vertex with the best score is chosen. */
	virtual KyBool ProcessVertex(const GraphVertexPtr& vertex);

	/* Increase the call counter */
	virtual void UpdateCallCounter();

	KyFloat32 ComputeScore(const Vec3f& position, KyFloat32 sqDistance) const;
	KyBool IsInCone(const Vec3f& position) const;
	KyBool IsClearPosition(const Vec3f& position) const;


public:
	PointWrapper m_destination; /*!< Stores the chosen destination point. Do not modify. */
	PointInfo_VertexSafePtr m_destinationSafePtr; /*!< Stores the UID of the chosen vertex. Do not modify. */
	KyFloat32 m_destinationScore; /*!< The score assigned to the destination by ProcessVertex(), if any. Do not modify. */ 
	KyFloat32 m_distance; /*!< Stores the value set by SetWanderDistance(). Do not modify directly. */

	/*!	Cosine of the angle for the cone of propagation. Do not modify directly.
		\pp This value is stored as the half of the actual width of the cone, because it is used as follows:
		[-m_propagationCosAngle, m_propagationCosAngle] */
	KyFloat32 m_propagationCosAngle;
};


/*!	This implementation of PropagationBias is intended specifically for use by the WanderTraversal. It prioritizes
	candidates that lie within the angle of propagation set for the WanderTraversal. When outside this cone of 
	propagation, candidate vertices that lie far away from the WanderAgent are prioritized over closer candidates.
	\ingroup kyruntime_agents */
class PropagationBias_Wander : public PropagationBias
{
public:
	KY_DECLARE_PROPAGATIONBIAS(PropagationBias_Wander)

public:
	/*!	\copydoc PropagationBias::GetCost()
		\pp This implementation of PropagationBias::GetCost() returns 0 for any vertex that lies within the cone
		of propagation set for the WanderTraversal. For vertices outside the cone of propagation, larger costs
		are assigned to those that are closer to the Bot, and lower costs are assigned to those that are farther away. */
	virtual KyBool GetCost(Bot* entity, const GraphVertexPtr& position, KyFloat32& cost);
	
	const WanderTraversal* m_traversal; /*!< A pointer to the WanderTraversal using this object. Do not modify. */
};

} // namespace Kaim

#endif // KY_WANDERTRAVERSAL_H
