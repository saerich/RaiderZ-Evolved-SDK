/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_agents */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_HIDINGTRAVERSAL_H
#define KY_HIDINGTRAVERSAL_H

#include <kyruntime/graph/astartraversal.h>


namespace Kaim
{

class PointLockManager;
class HideAgent;
class HidingTraversalConfig;
class Body;

/*!	This implementation of the AstarTraversal is used by the HideAgent to select a random destination point
	in the World. 
	\pp This class is closely coupled with the HideAgent. For details, see :USERGUIDE:"Hiding".
	\ingroup kyruntime_agents */
class HidingTraversal: public AstarTraversal
{
public:
	KY_DECLARE_TRAVERSAL(HidingTraversal)

public:
	/*!	\label_destructor */
	~HidingTraversal();

	/*!	Sets the HideAgent using the HidingTraversal. */
	void SetHideAgent(const HideAgent& agent);

	virtual void Start();

	/*!	Launches the HidingTraversal to compute a destination point for the HideAgent.
		\return #KY_TRUE if the computation has been completed, or #KY_FALSE otherwise. */
	KyBool ComputeHidingPlaces();


	/*!	Retrieves the final hiding position decided on by the HidingTraversal. */
	GraphVertexPtr GetHidingPlace() const;

	/*!	Defines a type of callback function that the HidingTraversal can invoke in order to determine whether or not each 
		vertex that it processes is hidden from view by each of the dangerous points set for the HideAgent.
		\param dangerousPoint		One of the dangerous points set for the HideAgent.
		\param end					The position of the vertex currently under consideration by the HidingTraversal.
		\param body					The Body of the Bot using the HideAgent.
		\return #KY_TRUE if \c body is visible from \c dangerousPoint when standing at point \c end, or #KY_FALSE otherwise. */
	typedef KyBool (*CheckPointVisibility)(const Vec3f& dangerousPoint, const Vec3f& end, Body* body); 
	
	/*!	Defines a type of callback function that the HidingTraversal can invoke in order to determine whether or not each 
		vertex that it processes is hidden from view by each of the dangerous \Bodies set for the HideAgent.
		\param dangerousBody		One of the dangerous \Bodies set for the HideAgent.
		\param end					The position of the vertex currently under consideration by the HidingTraversal.
		\param body					The Body of the Bot using the HideAgent.
		\return #KY_TRUE if \c body is visible by \c dangerousBody when standing at point \c end, or #KY_FALSE otherwise. */
	typedef KyBool (*CheckBodyVisibility)(const Body& dangerousBody, const Vec3f& end, Body* body); 

	/*!	Sets the callback function that the HidingTraversal will invoke in order to determine whether each vertex
		that it processes is visible from each of the dangerous points set for the HideAgent. By default, 
		the HidingTraversal uses BasicPointVisibility(). */
	void SetPointVisibilityFunction(CheckPointVisibility func, Body* body) { 
		m_pointVisibilityFunc = func;
		m_pointVisibleParam = body;
	}
	
	/*!	Retrieves the callback function that the HidingTraversal invokes in order to determine whether each vertex
		that it processes is visible from each of the dangerous points set for the HideAgent. */
	CheckPointVisibility GetPointVisibilityFunction() const { return m_pointVisibilityFunc; }
	
	/*!	Sets the callback function that the HidingTraversal will invoke in order to determine whether each vertex
		that it processes is visible from each of the dangerous \Bodies set for the HideAgent. By default, 
		the HidingTraversal uses BasicBodyVisibility(). */
	void SetBodyVisibilityFunction(CheckBodyVisibility func, Body* body) { 
		m_bodyVisibleFunc = func;
		m_bodyVisibleParam = body;
	}
	
	/*!	Retrieves the callback function that the HidingTraversal invokes in order to determine whether each vertex
		that it processes is visible from each of the dangerous \Bodies set for the HideAgent. */
	CheckBodyVisibility GetBodyVisibilityFunction() const { return m_bodyVisibleFunc; }

	/*!	This implementation of the #CheckPointVisibility type invokes the IsColliding() function to perform a simple
		visibility check between the dangerous point and the vertex. */
	static KyBool BasicPointVisibility(const Vec3f& dangerousPoint, const Vec3f& end, Body* body);
	
	/*!	This implementation of the #CheckBodyVisibility type invokes the IsColliding() function to perform a simple
		visibility check between the dangerous Body and the vertex. */
	static KyBool BasicBodyVisibility(const Body& dangerousBody, const Vec3f& end, Body* body);

	/*!	This implementation of the #CheckPointVisibility type invokes the IsColliding() function to test for visibility 
		between the dangerous point and the Body using the HideAgent, taking into account the width, length
		and height of the Bot using the HideAgent. */
	static KyBool EnhancedPointVisibility(const Vec3f& dangerousPoint, const Vec3f& end, Body* body);
	
	/*!	This implementation of the #CheckPointVisibility type invokes the IsColliding() function to test for visibility 
		between the dangerous Body and the Bot using the HideAgent, taking into account the width, length
		and height of the Bot using the HideAgent. */
	static KyBool EnhancedBodyVisibility(const Body& dangerousBody, const Vec3f& end, Body* body);

	/*	Not used. */
	virtual KyBool EvaluateEdge(const GraphEdgePtr& edge, KyFloat32& cost);
	
	/*	Not used. */
	void SetStealthMode(KyBool stealthMode) { m_stealthMode = stealthMode; }
	
	/*	Not used. */
	KyBool GetStealthMode() const { return m_stealthMode; }
	
	/*	Not used. */
	void SetStealthModeFactor(KyFloat32 factor) { m_stealthModeFactor = factor; }
	
	/*	Not used. */
	KyFloat32 GetStealthModeFactor() const { return m_stealthModeFactor; }

	/*!	Retrieves the total number of edges and vertices processed by the HidingTraversal since its
		initialization. Used to compute an estimation for deterministic time measurement mode.
		See :USERGUIDE:"Using Deterministic Time Measurement". */
	static KyUInt32 GetCallCounter();

protected:
	friend class HidingTraversalConfig;

	HidingTraversal();

	KyResult Initialize(Kaim::TraversalManager& traversalManager, const HidingTraversalConfig & config);


	/*!	\copydoc GraphTraversal::ProcessEdge()
		\pp This implementation of GraphTraversal::ProcessEdge() simply calls ProcessVertex() for the vertex
		that lies at the end of the specified edge. No evaluation of the edge itself is performed. */
	virtual KyBool ProcessEdge(const GraphEdgePtr& edge);

	/*!	\copydoc GraphTraversal::ProcessVertex()
		\pp This implementation of GraphTraversal::ProcessVertex() tests:
		-	whether or not the specified vertex is visible from each dangerous point set for the HideAgent, using the 
			#CheckPointVisibility callback set for the HidingTraversal.
		-	whether or not the specified vertex is visible from each dangerous Body set for the HideAgent, using the 
			#CheckBodyVisibility callback set for the HidingTraversal.
		\pp If the vertex is not visible from any dangerous point or dangerous Body, the vertex is accepted and
		adopted by the HideAgent as its destination. */
	virtual KyBool ProcessVertex(const GraphVertexPtr& vertex);

	virtual void UpdateCallCounter();

	GraphVertexSafePtr m_hidingPlaceVertexSafePtr;

	const HideAgent* m_hideAgent;

	CheckPointVisibility m_pointVisibilityFunc;
	CheckBodyVisibility m_bodyVisibleFunc;
	Body* m_pointVisibleParam;
	Body* m_bodyVisibleParam;
	KyBool m_stealthMode;
	KyFloat32 m_stealthModeFactor;

	PointLockManager* m_pointLockManager;
};

} //Kaim

#endif // KY_HIDINGTRAVERSAL_H
