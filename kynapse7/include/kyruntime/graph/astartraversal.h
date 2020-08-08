/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_ASTARTRAVERSAL_H
#define KY_ASTARTRAVERSAL_H

#include <kyruntime/graph/astarcontext.h>
#include <kyruntime/graph/graphmanager.h>
#include <kyruntime/graph/traversal.h>
#include <kyruntime/graph/graphedgeptr.h>

namespace Kaim
{
class PropagationBias;
class AstarTraversalConfig;

/*!	AstarTraversal is a GraphTraversal "exploration class": it implements the GetEdgeToProcess() and GetVertexToProcess()
	methods to define the order in which the edges and vertices of the \GraphCells loaded into the GraphManager are processed.
	\pp This class cannot be used directly. You must derive a new class from AstarTraversal, or use one
	of the derived classes supplied with \SDKName.
	\pp Classes that derive from AstarTraversal start processing the \GraphCells at a specific vertex, set 
	through a call to SetStartingVertex(). The AstarTraversal then propagates outward from this starting vertex using an
	A* algorithm. 
	\pp For additional information, see :USERGUIDE:"Using GraphTraversals".
	\section astartravprop Controlling propagation
	\pp Like the A* algorithm used by the PathFinder, you can provide a heuristic cost that influences the way the A* calculation
	propagates through the \GraphCells: i.e. to control the order in which the edges and vertices are processed. 
	To do this, you must create an instance of the PropagationBias class, and provide it to the AstarTraversal in a
	call to SetPropagationBias(). This PropagationBias class, similar to the IHeuristic used in path calculations, assigns a 
	relative cost to candidate vertices. The A* operation then treats the candidates in ascending order of cost: 
	those with lower costs are treated first, followed by those with higher relative costs.
	\section astartravcosts Retrieving the path of least cost
	\pp You must also set an IConstraint for the AstarTraversal through a call to SetPropagationConstraint(). This IConstraint,
	exactly like the IConstraint used by the PathFinder, assigns a final cost to each edge considered by the AstarTraversal. 
	The cost is used to calculate the paths of least cost from the starting vertex to other vertices that have been processed
	by the AstarTraversal. At any time, you can retrieve such a path from the starting vertex to any other vertex by calling
	GetFirstPathNode(), then calling GetNextPathNode() in a loop. For example:
	\code
		AstarTraversal::PathNodeIterator it;
		for (it=trav->GetFirstPathNode(destination); it; it=trav->GetNextPathNode(it))
		{
			PathNode pn;
			it.Get(pn);
			...
		} 
	\endcode
	You can then make an Bot follow this retrieved path by injecting the path into its PathFinder. 
	See :USERGUIDE:"Using Pre-Computed Paths".
	\section writingcustomastartraversal Writing a custom AstarTraversal class
	Writing a custom class that derives from AstarTraversal involves the following considerations.
	\subsection astartraversalconst Constructor
	All classes that derive from AstarTraversal must have a constructor that follows the #TraversalConstructor type definition. 
	\subsection astartraversalvm Virtual methods
	In order to be able to use an AstarTraversal class at runtime, you must implement its ProcessEdge() and ProcessVertex() methods. 
	\pp You may choose to re-implement other virtual methods, or use the default implementations if you prefer.
	\subsection astartraversalmetaclass MetaClass macros
	In order for \SDKName to recognize your AstarTraversal class, you must include the following macro definitions:
	-	#KY_DECLARE_TRAVERSAL: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_TRAVERSAL: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from GraphTraversal.
			For example, &Kaim::AstarTraversal::Class.
	\ingroup kyruntime_graph */
class AstarTraversal: public GraphTraversal
{
public:

	/*!	Sets the starting vertex from which the AstarTraversal begins its propagation through the
		\GraphCells loaded into the GraphManager. Note that this function transparently sets the GraphManager
		used by the AstarTraversal to the GraphManager that contains the specified vertex, so you do not need
		to call SetGraph() directly. */
	void SetStartingVertex(const GraphVertexPtr& vertex);

	virtual void Start();

	virtual void Stop();

	/*!	Sets the Bot using the AstarTraversal, if any. The PathFinder of this Bot is queried to determine the terrain types
		that the AstarTraversal can and cannot traverse. It is also passed to the PropagationBias set for the AstarTraversal,
		if any. */
	void SetBot(Bot* bot) { m_bot = bot; }

	/*!	Sets an IConstraint for the AstarTraversal. */
	void SetPropagationConstraint(IConstraint& constraint) { m_constraint = &constraint; }
	
	/*!	Retrieves the IConstraint set for the AstarTraversal.*/
	IConstraint* GetPropagationConstraint() const {return m_constraint;}

	/*!	Sets a PropagationBias that guides the propagation of the AstarTraversal, similar to the heuristics
		used in pathfinding. */
	void SetPropagationBias(PropagationBias& bias) { m_bias = &bias; }
	
	/*!	Retrieves the PropagationBias set for the AstarTraversal. */
	PropagationBias* GetPropagationBias() const {return m_bias;}

	/*!	\copydoc AstarTraversalConfig::m_maxCost */
	void SetMaxCost(KyFloat32 cost) { m_maxCost = cost; }
	
	/*!	Retrieves the value set in a call to SetMaxCost(). */
	KyFloat32 GetMaxCost() const { return m_maxCost; }

	/*!	\copydoc AstarTraversalConfig::m_maxDistance */
	void SetMaxDistance(KyFloat32 distance) { m_maxDistance = distance; }
	
	/*!	Retrieves the value set in a call to SetMaxDistance(). */
	KyFloat32 GetMaxDistance() const { return m_maxDistance; }

	/*!	This class represents a bi-directional iterator that can be used to iterate through the paths of least cost
		calculated by an AstarTraversal.
		\pp You can retrieve an instance of this class by calling AstarTraversal::GetFirstPathNode(), and you can 
		iterate through the path by calling AstarTraversal::GetNextPathNode() in a loop.
		\ingroup kyruntime_graph */
	class PathNodeIterator: public VirtualMemObject
	{
	public:
		/*!	For internal use. */
		PathNodeIterator() {}

		/*!	Returns #KY_TRUE if this PathNodeIterator points to a valid vertex. */
		bool IsValid() const { return m_current.IsValid(); }

		/*!	Sets the specified PathNode to the vertex currently being pointed to by this iterator. */
		void Get(PathNode& pn) const
		{
			if (m_current.IsValid())
			{
				m_current.ConvertToGraphVertexSafePtr_UnSafe(pn.m_vertexSafePtr);
				pn.m_position = m_current.GetPosition();

				GraphEdgePtr edge;
				if(m_next.IsValid())
					edge = m_current.GetOutEdgePtr(m_next.GetAstarId());

				if (edge.IsValid())
					pn.m_nextEdgeSafePtr = edge.ConvertToSafeEdgePtr_UnSafe();

				if (m_previous.IsValid())
				{
					GraphEdgePtr prevEdge = m_previous.GetOutEdgePtr(m_current.GetAstarId());
					if (prevEdge.IsValid())
						pn.m_firstPathObject = prevEdge.GetPathObject();
				}
			}
		}

	public:
		GraphVertexPtr m_previous; /*!< Points to the previous vertex in the path. */
		GraphVertexPtr m_current; /*!< The vertex currently being pointed to by this iterator. */
		GraphVertexPtr m_next; /*!< Points to the next vertex in the path. */
	};

	/*!	Retrieves a PathNodeIterator that points to the first vertex in the path of least cost calculated 
		by the AstarTraversal between the starting vertex and the destination passed to this method. */
	PathNodeIterator GetFirstPathNode(const GraphVertexSafePtr& destination);
	
	/*!	Retrieves a PathNodeIterator that points to the vertex following the one indicated by \c it. Must be
		preceded by a call to GetFirstPathNode().
		See also GetFirstPathNode(). */
	PathNodeIterator GetNextPathNode(PathNodeIterator& it);

	/*!	Renders the vertices that have been processed by the AstarTraversal, for debugging purposes. 
		-	Light green indicates an open vertex (at least one vedge leading from the vertex is still under consideration).
		-	Light blue indicates a closed vertex (all edges leading from the vertex have been explored).
		-	Light grey indicates a new vertex (the vertex has not yet been considered). */
	virtual void Render() const;

	/*!	The MetaClass of the AstarTraversal. */
	static TraversalClass Class;

protected:
	/*!	\copydoc GraphTraversal::GetVertexToProcess()
		\pp This implementation of GraphTraversal::GetVertexToProcess() retrieves the vertex with the lowest cost
		that has not yet been processed. The relative cost of each vertex is determined by the PropagationBias
		set for the AstarTraversal, if any. */
	virtual GraphVertexPtr GetVertexToProcess();
	
	/*!	\copydoc GraphTraversal::GetEdgeToProcess()
		\pp This implementation of GraphTraversal::GetEdgeToProcess() retrieves the next vertex to be processed
		by calling GetVertexToProcess(), and returns the edge leading to that vertex. */
	virtual GraphEdgePtr GetEdgeToProcess();
	virtual void UpdateCallCounter() { /*Implement if you need*/ }

	void Astar_ProcessEdge(const GraphEdgePtr& edge, IConstraint& constraint, PropagationBias& bias, Bot* entity);

	virtual void TraverseEdges(void* owner);
	virtual void TraverseVertices(void* owner);
	virtual void TraverseEdges(const Bot& entity);
	virtual void TraverseVertices(const Bot& entity);
	virtual void TraverseEdges(const Team& team);
	virtual void TraverseVertices(const Team& team);

protected:
	AstarTraversal();

	~AstarTraversal();

	KyResult Initialize(Kaim::TraversalManager& traversalManager, const Kaim::AstarTraversalConfig& config);

public:
	GraphVertexPtr m_startingVertexPtr; /*!< For internal use. Do not modify. */
	GraphVertexSafePtr m_startingVertexSafePtr; /*!< For internal use. Do not modify. */
	Vec3f m_startingVertexPosition; /*!< For internal use. Do not modify. */
	KyFloat32 m_maxCost; /*!< For internal use. Do not modify. */
	KyFloat32 m_maxDistance; /*!< For internal use. Do not modify. */
	IConstraint* m_constraint; /*!< For internal use. Do not modify. */
	PropagationBias* m_bias; /*!< For internal use. Do not modify. */
	Bot* m_bot; /*!< For internal use. Do not modify. */
	KyBool m_begin; /*!< For internal use. Do not modify. */

	AstarContext m_astarContext; /*!< For internal use. Do not modify. */

	mutable PointWrapper m_pw1, m_pw2; /*!< For internal use. Do not modify. */
	mutable PointInfo_VertexSafePtr *m_vertexSafePtrInfo1, *m_vertexSafePtrInfo2; /*!< For internal use. Do not modify. */
	mutable PointInfo_AstarId *m_astarIdInfo1, *m_astarIdInfo2; /*!< For internal use. Do not modify. */
};


/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from PropagationBias. Each class that derives from 
	PropagationBias must contain a constructor function that matches this type definition. The argument list
	of this type definition is empty, which indicates that the default constructor of your derived class
	will be invoked.
	\ingroup kyruntime_graph */
typedef PropagationBias* (*PropagationBiasConstructor)();

/*!	PropagationBiasClass is the MetaClass for PropagationBias, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_graph */
class PropagationBiasClass: public MetaClass<PropagationBias,PropagationBiasConstructor>
{
public:
	/*!	Constructor for the MetaClass of PropagationBias. For internal use. */
	PropagationBiasClass(const char* className, PropagationBiasConstructor ctr, 
		PropagationBiasClass* pBaseClass = KY_NULL);
};

/*!	PropagationBias is the base class for all classes used by the AstarTraversal to control the way in which the 
	A* calculation propagates through the \GraphCells loaded into the GraphManager: i.e. the order in which the
	vertices and edges are processed by the AstarTraversal. The AstarTraversal uses an instance of a class that 
	derives from PropagationBias to determine a heuristic cost for each candidate vertex. It then sorts the 
	candidate vertices in ascending order of cost, and processes those with the lowest cost first.
	\pp See :USERGUIDE:"Using GraphTraversals".
	\pp This is very similar to the way the PathFinder uses heuristics to control the propagation of the A* calculation.
	The principal difference is that the PropagationBias does not have a destination point to use in its cost calculations. 
	\section writingcustombias Writing a custom PropagationBias class
	Writing a custom class that derives from PropagationBias involves the following considerations.
	\subsection biasconst Constructor
	All classes that derive from PropagationBias must have a constructor that follows the #PropagationBiasConstructor type definition. 
	\subsection biasvm Virtual methods
	All PropagationBias classes must implement the GetCost() method.
	\pp You may also implement any other virtual methods of this class if you choose, or use the default implementations.
	\pp If you choose to implement the OnStartAstar(), OnFinishAstar, OnPauseAstar() and/or OnResumeAstar to carry out tasks at
	specific moments during the path calculation process, your implementations must take into account the fact that paused A* 
	calculations are not necessarily resumed. For example, if a path needs to be re-calculated before its initial A* calculation 
	has completed (for example, due to a GraphCell being unloaded), the calculation may be restarted instead. In this case, 
	OnFinishAstar() is called to notify the PropagationBias that the paused calculation has been terminated, and OnStartAstar() is 
	called again when the next calculation is begun.
	\subsection biasmetaclass MetaClass macros
	In order for \SDKName to recognize your PropagationBias class, you must include the following macro definitions:
	-	#KY_DECLARE_PROPAGATIONBIAS: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_PROPAGATIONBIAS: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from PropagationBias.
			For example, &Kaim::PropagationBias_Zero::Class. If your new class derives directly from PropagationBias,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_graph */
class PropagationBias : public VirtualMemObject
{
public:
	/*!	\label_baseconstructor */
	PropagationBias(): m_astarContext(KY_NULL) {}

	/*!	\label_getclass */
	virtual PropagationBiasClass& GetClass() const = 0;

	/*!	Calculates the relative cost of the specified vertex, according to the specified Bot.
		\param entity				The Bot using the AstarTraversal that is invoking this method.
		\param position				The candidate vertex.
		\param[out] cost			Used to store the calculated cost.
		\return #KY_FALSE if the \c cost argument has been successfully updated, or #KY_FALSE if the specified vertex
				should be discarded by the A* calculation. */
	virtual KyBool GetCost(Bot* entity, const GraphVertexPtr& position, KyFloat32& cost) = 0;

	/*!	This method is called transparently by the AstarTraversal immediately before starting an A* computation. */
	virtual void OnStartAstar(const GraphVertexPtr& /*start*/) {}
	
	/*!	This method is called transparently by the AstarTraversal immediately after completing an A* computation. */
	virtual void OnFinishAstar(const GraphVertexPtr& /*start*/) {}
	
	/*!	This method is called transparently by the AstarTraversal each time the A* computation is paused. See also
		:USERGUIDE:"Time-slicing A* Calculations". Your implementation of this method must leave your PropagationBias 
		in a temporary idle state from which its calculations may be either resumed or restarted. For example,
		any data required to calculate costs in future frames should be saved into class members. */
	virtual void OnPauseAstar(const GraphVertexPtr& /*start*/) {}
	
	/*!	This method is called transparently by the AstarTraversal each time a paused A* computation is resumed. See also
		:USERGUIDE:"Time-slicing A* Calculations". Your implementation of this method must restart your PropagationBias
		from the idle state entered by the OnPauseAstar() method. */
	virtual void OnResumeAstar(const GraphVertexPtr& /*start*/) {}


	mutable AstarContext* m_astarContext; /*!< A pointer to the AstarContext in use by the AstarTraversal. Do not modify. */
};

/*!	Macro that must be called in the declaration of any PropagationBias class intended for use in
	an AstarTraversal, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_graph */
#define KY_DECLARE_PROPAGATIONBIAS(ky_class) \
	static Kaim::PropagationBiasClass Class; \
	static Kaim::PropagationBias* KyCreate(); \
	Kaim::PropagationBiasClass& GetClass() const { return ky_class::Class; }

/*!	Macro that must be called in the implementation of any PropagationBias class intended for use in
	an AstarTraversal, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_graph */
#define KY_IMPLEMENT_PROPAGATIONBIAS(ky_class,ky_base_class) \
	Kaim::PropagationBiasClass ky_class::Class(#ky_class,ky_class::KyCreate,(Kaim::PropagationBiasClass*)ky_base_class); \
	Kaim::PropagationBias* ky_class::KyCreate() \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::PropagationBias* ptr = (Kaim::PropagationBias*) KY_NEW_CLASS(ky_class)(); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}

/*!	This implementation of PropagationBias assigns a zero cost to all \GraphVertices. 
	This makes an AstarTraversal using PropagationBias_Zero propagate equally in all directions.
	\ingroup kyruntime_graph */
class PropagationBias_Zero : public PropagationBias
{
public:
	KY_DECLARE_PROPAGATIONBIAS(PropagationBias_Zero)

	/*!	\copydoc PropagationBias::GetCost()
		\pp This implementation of PropagationBias::GetCost() simply returns 0 in all cases. */
	virtual KyBool GetCost(Bot* /*entity*/, const GraphVertexPtr& /*position*/, KyFloat32& cost)
	{
		cost = 0.f;
		return KY_TRUE;
	}
};

} // namespace Kaim

#endif // KY_ASTARTRAVERSAL_H
