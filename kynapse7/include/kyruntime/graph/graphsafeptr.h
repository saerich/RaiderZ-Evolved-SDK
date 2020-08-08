/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	

/*!	\file
	\ingroup kyruntime_graph */

// Primary contact: JUBA - secondary contact: LAPA
#ifndef KyPathData_GraphSafePtr_H
#define KyPathData_GraphSafePtr_H

#include "kypathdata/containers/array.h"
#include "kypathdata/graph/graphtypes.h"
#include "kypathdata/navmesh/navmeshtypes.h"
#include "kyruntime/graph/stitchedgraphcell.h"


namespace Kaim
{

class Graph;

///////////////////////////////////////////////////////////////////////////////////////////
// GraphSafePtr
///////////////////////////////////////////////////////////////////////////////////////////

/*! This class uniquely identifies a Graph loaded into a GraphManager.
	\pp Instances of this class can be safely stored and used in subsequent frames.
	\pp When you are finished with an instance of this class, you must call Invalidate() to free its allocated resources. This
	must be done before you destroy the World.
	\ingroup kyruntime_graph */
class GraphSafePtr
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*!	\label_constructor Creates an invalid GraphSafePtr. */
	KY_FORCE_INLINE GraphSafePtr(): m_graphReference(KY_NULL) {}

	/*!	\label_constructor
		\param graphReference		A pointer to the GraphReference that refers to this Graph. */
	KY_FORCE_INLINE GraphSafePtr(GraphReference* graphReference) : m_graphReference(KY_NULL)
	{
		if (graphReference != KY_NULL)
			Set(*graphReference);
	}

	/*!	\label_constructor
		\param graphReference		A reference to the GraphReference that refers to this Graph. */
	KY_FORCE_INLINE GraphSafePtr(GraphReference& graphReference) : m_graphReference(&graphReference)
	{
		graphReference.IncrementUserNumber();
	}

	/*! \label_constructor Copies the state of the specified GraphSafePtr to this object. */
	GraphSafePtr(const GraphSafePtr& graphSafePtr) : m_graphReference(graphSafePtr.m_graphReference)
	{
		if (graphSafePtr.m_graphReference != KY_NULL)
			graphSafePtr.m_graphReference->IncrementUserNumber();
	}

	/*! Copies the state of the specified GraphSafePtr to the newly created object. */
	KY_FORCE_INLINE GraphSafePtr& operator=(const GraphSafePtr& graphSafePtr)
	{
		Set(graphSafePtr.m_graphReference);
		return *this;
	}

	/*! \label_destructor */
	~GraphSafePtr()
	{
		if (m_graphReference != KY_NULL)
			m_graphReference->DecrementUserNumber();
	}

	/*!	\label_reinit
		\param graphReference		A pointer to the GraphReference that refers to this Graph. */
	KY_FORCE_INLINE void Set(GraphReference* graphReference)
	{
		if (m_graphReference != KY_NULL)
		{
			m_graphReference->DecrementUserNumber();
			m_graphReference = KY_NULL;
		}

		if (graphReference != KY_NULL)
			graphReference->IncrementUserNumber();

		m_graphReference = graphReference;
	}

	/*!	\label_reinit
		\param graphReference		A reference to the GraphReference that refers to this Graph. */
	KY_FORCE_INLINE void Set(GraphReference& graphReference)
	{
		if (m_graphReference != KY_NULL)
			m_graphReference->DecrementUserNumber();

		m_graphReference = &graphReference;
		graphReference.IncrementUserNumber();
	}

	/*! Returns true if this object refers to a valid runtime Graph: i.e. a Graph that is currently loaded into a GraphManager. */
	KY_FORCE_INLINE bool IsValid() const
	{
		if (m_graphReference != KY_NULL)
		{
			if (m_graphReference->IsValid())
				return true;

			m_graphReference->DecrementUserNumber();
			m_graphReference = KY_NULL;
		}

		return false;
	}

	/*! Invalidates this object. */
	KY_FORCE_INLINE void Invalidate()
	{
		if (m_graphReference != KY_NULL)
		{
			m_graphReference->DecrementUserNumber();
			m_graphReference = KY_NULL;
		}
	}

	/*!	Returns \c true if this object identifies the same Graph as \c graphSafePtr, or if both are invalid. */
	KY_FORCE_INLINE bool operator==(const GraphSafePtr& graphSafePtr) const { return m_graphReference == graphSafePtr.m_graphReference; }

	/*!	Returns \c true if this object identifies a Graph different from the one identified by \c graphSafePtr, or if only one of them is invalid. */
	KY_FORCE_INLINE bool operator!=(const GraphSafePtr& graphSafePtr) const { return m_graphReference != graphSafePtr.m_graphReference; }

	KY_FORCE_INLINE bool operator < (const GraphSafePtr& graphSafePtr) const { return m_graphReference < graphSafePtr.m_graphReference; }
	KY_FORCE_INLINE bool operator > (const GraphSafePtr& graphSafePtr) const { return m_graphReference > graphSafePtr.m_graphReference; }
	KY_FORCE_INLINE bool operator <=(const GraphSafePtr& graphSafePtr) const { return !(*this > graphSafePtr); }
	KY_FORCE_INLINE bool operator >=(const GraphSafePtr& graphSafePtr) const { return !(*this < graphSafePtr); }


	/*!	Returns a const pointer to the StitchedGraph that handles the Graph identified by this object.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE StitchedGraph* GetStitchedGraph() const { return IsValid() ? m_graphReference->GetData() : KY_NULL; }

	/*!	Returns a const reference to the StitchedGraph that handles the Graph identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE StitchedGraph& GetStitchedGraph_UnSafe() const { return *m_graphReference->GetData(); }

	/*!	Returns a const pointer to the Graph identified by this object.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const Graph* GetGraph() const { return IsValid() ? m_graphReference->GetData()->m_graph : KY_NULL; }

	/*!	Returns a const pointer to the Graph identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const Graph* GetGraph_UnSafe() const { return m_graphReference->GetData()->m_graph; }

	/*!	Returns a pointer to the GraphReference that refers to the Graph identified by this object. */
	KY_FORCE_INLINE GraphReference* GetGraphReference() const { return m_graphReference; }

	/*!	Returns a const pointer to the GraphManager that contains the Graph identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const GraphManager* GetGraphManager_UnSafe() const { return m_graphReference->GetData()->GetGraphManager(); }

	/*!	Returns \c true if the Graph identified by this object is a VirtualGraph: a Graph that has been dynamically created to link two static \Graphs.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE bool IsVirtualGraph() const
	{
		KY_FUNCTION("GraphEdgePtr::IsVirtualEdge");
		KY_ASSERT(IsValid(), ("You cannot call GraphEdgePtr::IsVirtualEdge on an invalid GraphEdgePtr"));
		return m_graphReference->GetData()->IsVirtual();
	}

	/*!	Returns \c true if the Graph identified by this object is a static Graph.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE bool IsStaticGraph() const
	{
		KY_FUNCTION("GraphEdgePtr::IsStaticGraph");
		KY_ASSERT(IsValid(), ("You cannot call GraphEdgePtr::IsStaticEdge on an invalid GraphEdgePtr"));

		return !m_graphReference->GetData()->IsVirtual();
	}

private :
	mutable GraphReference* m_graphReference;
};


}

#endif // #ifndef KyPathData_GraphSafePtr_H
