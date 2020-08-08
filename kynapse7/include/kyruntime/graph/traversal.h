/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_TRAVERSAL_H
#define KY_TRAVERSAL_H

#include <kyruntime/core/metaclass.h>
#include <kyruntime/graph/traversalconfig.h>
#include <kypathdata/generic/memory.h>


namespace Kaim
{

class TraversalClass;
class GraphManager;
class GraphVertexPtr;
class GraphEdgePtr;



/*!	GraphTraversal is the base class for a component that analyzes the GraphCells maintained by a GraphManager
	in real time, finding vertices and edges that meet certain user-defined criteria. These \GraphTraversals
	form the basis of most Spatial Reasoning in \SDKName.
	\pp The GraphTraversal class cannot be used as-is; you must write your own class that derives either from the
	base GraphTraversal class, or from one of the "exploration" classes supplied with \SDKName: AstarTraversal and
	PropagationGraphTraversal.
	\pp For additional information, see :USERGUIDE:"Using GraphTraversals".
	\section writingcustomtraversal Writing a custom GraphTraversal class
	Writing a custom class that derives from GraphTraversal involves the following considerations.
	\subsection traversalconst Constructor
	All classes that derive from GraphTraversal must have a constructor that follows the #TraversalConstructor type definition. 
	\subsection traversalvm Virtual methods
	In order to be able to use a GraphTraversal class at runtime, its GetEdgeToProcess, ProcessEdge(), GetVertexToProcess()
	nd ProcessVertex() methods must all be implemented. 
	\pp If you derive your own class directly from GraphTraversal, you must implement all of these methods in your class.
	\pp However, the exploration classes supplied with \SDKName (AstarGraphTraversal and PropagationGraphTraversal) already 
	implement GetEdgeToProcess() and GetVertexToProcess(). If you derive your own class from either of those exploration
	classes, you only need to implement ProcessEdge() and ProcessVertex().
	\pp You may choose to re-implement other virtual methods, or use the default implementations if you prefer.
	\subsection traversalmetaclass MetaClass macros
	In order for \SDKName to recognize your GraphTraversal class, you must include the following macro definitions:
	-	#KY_DECLARE_TRAVERSAL: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_TRAVERSAL: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from GraphTraversal.
			For example, &Kaim::AstarTraversal::Class. If your new class derives directly from GraphTraversal,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_graph */
class GraphTraversal : public VirtualMemObject
{
	friend class TraversalManager;

public:
	
	/*!	\label_getclass */
	virtual const TraversalClass& GetClass()const = 0;

	/*!	Specifies the GraphManager that contains the set of \GraphCells to be processed by the GraphTraversal. */
	virtual void SetGraph(GraphManager& graphManager) { m_graphManager = &graphManager; }

	/*!	Specifies the ID of the aperiodic task used to manage the CPU time consumed by this GraphTraversal in each
		frame. If this value is TimeManager::InvalidTaskId, the GraphTraversal will not be time-sliced at all, and
		will continue to run within the current frame until it is finished. */
	void SetTimeSlicingConfig(KyInt32 aperiodicTaskId);
	
	/*!	Specifies the name of an aperiodic task used to manage the CPU time consumed by this GraphTraversal in each frame. */
	void SetTimeSlicingConfig(const char* aperiodicTaskName);
	
	/*!	Retrieves the ID of the aperiodic task used to manage the CPU time consumed by this GraphTraversal in each frame. */
	TimeManager::TaskId GetTimeSlicingConfig() const { return m_aperiodicTaskId; }

	/*!	Resets the GraphTraversal. This function must be called before launching a new calculation.
		Typically, after calling this method, you will also need to perform additional initializations
		that are specific to your GraphTraversal class. */
	virtual void Start();

	/*!	Stops the GraphTraversal immediately. */
	virtual void Stop();

	/*!	Indicates whether or not the GraphTraversal process has been successfully started. */
	KyBool IsProcessStarted() const;

	/*!	Indicates whether or not the GraphTraversal process has finished. */
	KyBool IsProcessOver() const { return m_finished; }

	/*!	Releases the GraphTraversal back to the pool maintained by the TraversalManager. This method is
		called automatically if the GraphTraversal is not used for several consecutive frames. However,
		in order to ensure that other components are never blocked from accessing a GraphTraversal, it is 
		recommended that you call this method explicitly whenever you have finished using a GraphTraversal. */
	void Release();

	/*!	Returns a pointer to the component using the GraphTraversal. */
	void* GetCurrentUser() const { return m_currentUser.addr; }

	/*!	\label_getworld */
	World& GetWorld()
	{
		return *m_world;
	}

	/*!	\label_getworld */
	const World& GetWorld() const
	{
		return *m_world;
	}

	/*!	\label_getengine */
	Engine& GetEngine();

	/*!	\label_getengine */
	const Engine& GetEngine() const;

protected:
	// Functions used to traverse graph edges and vertices under CPU constraints.
	// The function arguments are used mainly to manage time-slicing.
	// See TimeManager::RequestAperiodic() functions for details.
	virtual void TraverseEdges(void* owner);
	virtual void TraverseVertices(void* owner);
	virtual void TraverseEdges(const Bot& bot);
	virtual void TraverseVertices(const Bot& bot);
	virtual void TraverseEdges(const Team& team);
	virtual void TraverseVertices(const Team& team);

	//-------------------------------------------------------------
	// Pure virtual functions. You must define these in the child classes
	//-------------------------------------------------------------

	/*!	Retrieves the next edge that the GraphTraversal should process, or #KY_NULL if all edges in the GraphManager 
		have already been processed. ProcessEdge() will be called to treat the returned edge.
		\pp This method, in conjunction with GetVertexToProcess(), determines how the GraphTraversal explores the spatial graph. 
		If you derive your GraphTraversal from either of the exploration classes supplied with \SDKName, 
		you should not need to implement this method. */
	virtual GraphEdgePtr GetEdgeToProcess() = 0;

	/*!	Retrieves the next vertex that the GraphTraversal should process, or #KY_NULL if all vertices in the GraphManager 
		have already been processed. ProcessVertex() will be called to treat the returned vertex.
		\pp This method, in conjunction with GetEdgeToProcess(), determines how the GraphTraversal explores the spatial graph. 
		If you derive your GraphTraversal from either of the exploration classes supplied with \SDKName, 
		you should not need to implement this method. */
	virtual GraphVertexPtr GetVertexToProcess() = 0;

	/*!	This method is called to examine each edge considered by the GraphTraversal, in order to determine whether
		or not that edge meets the desired set of criteria.
		\pp If a candidate edge satisfies the examination conducted by this method, the method should return #KY_FALSE.
		Otherwise, the method should return #KY_TRUE.
		\param edge			The edge to be processed.
		\return #KY_TRUE if the GraphTraversal process should continue, or #KY_FALSE to stop the GraphTraversal. */
	virtual KyBool ProcessEdge(const GraphEdgePtr& edge) = 0;

	/*!	This method is called to examine each vertex considered by the GraphTraversal, in order to determine whether
		or not that vertex meets the desired set of criteria.
		\pp If you intend to use the vertex you select as a destination point for a ground-based Bot, your implementation of 
		this method should test that the vertex lies within a reachable area of the NavMesh, outside of all \Obstacles. 
		To verify that the point is clear, you can call the PathFinder::IsClearPosition() method or use the SpawnManager. 
		\pp If a candidate vertex satisfies the examination conducted by this method, the method should return #KY_FALSE. 
		Otherwise, the method should return #KY_TRUE. 
		\param vertex			The vertex to be processed.
		\return #KY_TRUE if the GraphTraversal process should continue, or #KY_FALSE to stop the GraphTraversal. */
	virtual KyBool ProcessVertex(const GraphVertexPtr& vertex) = 0;

	/*!	Updates the counter each time an edge or vertex is processed. The default implementation of this 
		method is blank. You may implement it in your derived classes. */
	virtual void UpdateCallCounter() {}

protected:
	/*!	\label_baseconstructor */
	GraphTraversal();

	KyResult Initialize(Kaim::World& world, const GraphTraversalConfig& config);

protected:
	GraphManager* m_graphManager; //< The Spatial Graph being processed by the GraphTraversal. Do not modify directly.
	TimeManager::TaskId m_aperiodicTaskId; //< The ID of the aperiodic task for the GraphTraversal. Do not modify directly.
	TaskRegistry::TaskId m_estimatedTaskId; //< The ID of the estimated task for the GraphTraversal. Do not modify directly.

	// Called internally to register that the GraphTraversal has been used in the current frame.
	void RecordComputation() { m_usageStatus = 0; }
	// Class used internally to represent the component using the GraphTraversal.
	struct CurrentUser
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
	public:
		CurrentUser() : m_type(CurrentUser::ADDR), addr(KY_NULL) {};

		enum EType { BOT, TEAM, ADDR, __DUMMY__ = KYFORCEENUMSIZEINT } m_type;
		void* addr;
	};
	World* m_world; //< Pointer to the AI World. Do not modify.
	CurrentUser m_currentUser; //< Pointer to the component using the GraphTraversal. Do not modify.
	KyInt32 m_usageStatus; //< For internal use. Do not modify.
	KyFloat32 m_lastRequest; // For internal use. Do not modify.
	KyBool m_restart; // For internal use. Do not modify.

private:
	KyBool m_finished;
};



/*!	Macro that must be called in the declaration of any GraphTraversal class, to set up the
	MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_graph */
#define KY_DECLARE_TRAVERSAL(ky_class) \
	static Kaim::TraversalClass Class; \
	static Kaim::GraphTraversal* KyCreate(); \
	const Kaim::TraversalClass& GetClass() const { return ky_class::Class; }


/*!	Macro that must be called in the implementation of any GraphTraversal class, to set up the
	MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_graph */
#define KY_IMPLEMENT_TRAVERSAL(ky_class, ky_base_class) \
	Kaim::TraversalClass ky_class::Class(#ky_class, ky_class::KyCreate, static_cast<Kaim::TraversalClass*>(ky_base_class)); \
	Kaim::GraphTraversal* ky_class::KyCreate() \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		return static_cast<Kaim::GraphTraversal*>(KY_NEW_CLASS(ky_class)()); \
	}


/*!	Defines the signature for the constructor function that is called automatically each time a class that
	derives from GraphTraversal is instantiated during the \SDKName initialization process. Each class 
	that derives from GraphTraversal must contain a constructor function that matches this type definition. 
	The argument list of this type definition is empty, which indicates that the default constructor of 
	your derived class will be invoked.
	\ingroup kyruntime_graph */
typedef GraphTraversal* (*TraversalConstructor)();

/*!	TraversalClass is the MetaClass for GraphTraversal, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_graph */
class TraversalClass: public MetaClass<GraphTraversal, TraversalConstructor>
{
public:
	/*!	Constructor for the GraphTraversal MetaClass. */
	TraversalClass(const char* className, TraversalConstructor ctr, TraversalClass* baseClass);
};


} // namespace Kaim

#endif // KY_TRAVERSAL_H
