/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_PROPAGATIONTRAVERSAL_H
#define KY_PROPAGATIONTRAVERSAL_H

#include <kyruntime/graph/graphmanager.h>
#include <kyruntime/graph/traversal.h>


namespace Kaim
{
/*!	PropagationGraphTraversal is an "exploration class" of GraphTraversal: it implements the GetEdgeToProcess() 
	and GetVertexToProcess() methods to define the order in which the edges and vertices of the \GraphCells loaded
	into the GraphManager are processed. 
	\pp This class cannot be used directly. You must derive a new class from PropagationGraphTraversal, or use one
	of the derived classes supplied with \SDKName.
	\pp Classes that derive from PropagationGraphTraversal start processing the \GraphCells at a specific
	vertex, set through a call to SetStartingVertex(). The PropagationGraphTraversal then propagates outward from this starting vertex, 
	treating edges and vertices in ascending order of cost: edges and vertices with lower costs are treated first, 
	followed by edges and vertices with higher relative costs. 
	\pp The cost of each edge and vertex is assigned by calls to the EvaluateEdge() method. The default implementation
	of EvaluateEdge() returns a cost for each edge equal to its distance from the starting vertex, which makes the
	PropagationGraphTraversal radiate outwards from the starting vertex equally in all directions. If you want your
	PropagationGraphTraversal to propagate in a particular direction rather than radiating in all directions equally,
	you can re-implement EvaluateEdge() to assign a relative cost to each edge based on different criteria.
	\pp For additional information, see :USERGUIDE:"Using GraphTraversals".
	\section writingcustomastartraversal Writing a custom PropagationGraphTraversal class
	Writing a custom class that derives from PropagationGraphTraversal involves the following considerations.
	\subsection proptraversalconst Constructor
	All classes that derive from PropagationGraphTraversal must have a constructor that follows the #TraversalConstructor type definition. 
	\subsection proptraversalvm Virtual methods
	In order to be able to use an PropagationGraphTraversal class at runtime, you must implement its ProcessEdge() and ProcessVertex() methods. 
	\pp You may choose to re-implement other virtual methods, or use the default implementations if you prefer. In particular,
	you may want to re-implement EvaluateEdge() to control the way the PropagationGraphTraversal propagates through the \GraphCells.
	\subsection proptraversalmetaclass MetaClass macros
	In order for \SDKName to recognize your PropagationGraphTraversal class, you must include the following macro definitions:
	-	#KY_DECLARE_TRAVERSAL: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_TRAVERSAL: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from GraphTraversal.
			For example, &Kaim::PropagationGraphTraversal::Class.
	\ingroup kyruntime_graph*/
class PropagationGraphTraversal : public GraphTraversal
{
public:
	PropagationGraphTraversal();
	
	~PropagationGraphTraversal();

	/*!	Sets the starting vertex from which the AstarTraversal begins its propagation through the
		\GraphCells loaded into the GraphManager. Note that this function transparently sets the GraphManager
		used by the AstarTraversal to the GraphManager that contains the specified vertex, so you do not need
		to call SetGraph() directly. */
	void SetStartingVertex(const GraphVertexPtr& vertex);

	virtual void Start();

	/*!	Calculates the cost of an edge.
		\pp The PropagationGraphTraversal uses this method to weigh the relative cost of each edge, to decide the order in
		which it should process the edges. It always processes edges with the least cost first. The default implementation
		returns the length of each edge, which causes the PropagationGraphTraversal to process edges closer to the starting
		vertex before exploring edges farther from the starting vertex.
		\pp You can override this method in a derived class to influence the way the PropagationGraphTraversal chooses
		the next edge it will process.
		\param edge				The edge to be evaluated.
		\param[out] cost		Used to store the cost calculated by this method.
		\return #KY_TRUE if the \c cost parameter has been updated with the cost of the edge, or #KY_FALSE 
				to indicate that the edge cannot be traversed. */
	virtual KyBool EvaluateEdge(const GraphEdgePtr& edge, KyFloat32& cost);

	/* not used */
	void SetMaxEdgeCount(KyUInt32 count);

	/*!	\copydoc PropagationGraphTraversalConfig::m_maxDistance */
	void SetMaxDistance(KyFloat32 distance);
	
	/*!	Retrieves the value set through a call to SetMaxDistance(). */
	KyFloat32 GetMaxDistance() const;

	/*!	Draws all edges treated by the PropagationGraphTraversal in white. */
	virtual void Render() const;

protected:
	/*!	\copydoc GraphTraversal::GetEdgeToProcess()
		\pp This implementation of GraphTraversal::GetEdgeToProcess() retrieves the edge with the lowest cost
		that has not yet been processed. The relative cost of each edge is determined through calls to 
		EvaluateEdge(). */
	virtual GraphEdgePtr GetEdgeToProcess();
	
	/*!	\copydoc GraphTraversal::GetVertexToProcess()
		\pp This implementation of GraphTraversal::GetVertexToProcess() calls GetEdgeToProcess() to retrieve the
		edge with the lowest cost that has not yet been explored, and returns the vertex that lies at the end
		of this edge. */
	virtual GraphVertexPtr GetVertexToProcess();

	// For internal use only
	GraphEdgePtr GetEdgeToProcess(KyBool allowDuplicateVertices);
	void Propagate(KyBool allowDuplicateVertices);

public:
	/*!	The MetaClass of the PropagationGraphTraversal. */
	static TraversalClass Class; //<! base class for derived classes

protected:
	Array<GraphEdgeSafePtr> m_edgeSafePtrs; /*< For internal use. Do not modify. */
	Array<KyFloat32> m_edgeDistances; /*< For internal use. Do not modify. */
	KyUInt32 m_edgeSafePtrsSize; /*< For internal use. Do not modify. */
	KyUInt32 m_edgeIdsCount; /*< For internal use. Do not modify. */
	KyUInt32 m_currentIndex; /*< For internal use. Do not modify. */
	KyUInt32 m_propagationId; /*< For internal use. Do not modify. */
	GraphVertexSafePtr m_startingVertexSafePtr; /*< For internal use. Do not modify. */
	KyFloat32 m_squareDistanceMax; /*< For internal use. Do not modify. */
};

} // namespace Kaim

#endif // KY_PROPAGATIONTRAVERSAL_H
