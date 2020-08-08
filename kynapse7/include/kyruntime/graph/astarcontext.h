/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */

// primary contact: JUBA - secondary contact: LAPA
#ifndef KY_ASTARCONTEXT_H
#define KY_ASTARCONTEXT_H

#include <kyruntime/graph/graphmanager.h>
#include <kyruntime/graph/graphsafeptr.h>
#include <kyruntime/core/iworldobserver.h>


namespace Kaim
{

class AstarContext;


///////////////////////////////////////////////////////////////////////////////////////////
// AstarContext
///////////////////////////////////////////////////////////////////////////////////////////

/*!	For internal use by the AstarContext.
	\ingroup kyruntime_graph*/
class AStarContextObserver : public VirtualMemObject
{
public:
	virtual void OnResize(const AstarContext& astarContext, KyUInt32 oldSize) = 0;
};

/*!	This class contains temporary data related to an A* computation carried out by the GraphManager::Astar method. 
	It allows A* calculations to be paused and resumed in a succeeding frame.
	\pp Typically, use of this class is handled internally. 
	\pp The World creates an instance of this class at initialization time, and
	maintains it in its World::m_astarContext member. If you need an instance of AstarContext,
	you can borrow this one. However, keep in mind that the borrowed context may reflect the saved state of a 
	paused time-sliced calculation begun in the previous frame. In this case, your new A* calculation will 
	cancel the paused calculation.
	\ingroup kyruntime_graph */
class AstarContext : public VirtualMemObject
{
public:
	/*!	Used to indicate the status of each vertex that may be explored during an A* calculation. */
	enum VertexStatus
	{
		EVertexStatus_NEW = 0, /*!< Indicates that the vertex has not yet been considered. */
		EVertexStatus_OPENED,  /*!< Indicates that the vertex is being explored. */
		EVertexStatus_CLOSED,  /*!< Indicates that the vertex has been fully explored. */
		EVertexStatus_dummy_ = KYFORCEENUMSIZEINT
	};
protected:

public:
	// GraphVertexPtrAndAstarId
	/*!	For internal use.
		\ingroup kyruntime_graph */
	class GraphVertexPtrAndAstarId
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
	public :
		GraphVertexPtrAndAstarId() : m_graphVertexPtr(), m_astarId(KyUInt32MAXVAL) {}
		GraphVertexPtrAndAstarId(const GraphVertexPtr& vertexSafePtr, KyUInt32 astarId): m_graphVertexPtr(vertexSafePtr), m_astarId(astarId) {}

		KY_FORCE_INLINE const GraphVertexPtr& GetGraphVertexPtr() const { return m_graphVertexPtr; }
		KY_FORCE_INLINE GraphVertexPtr& GetGraphVertexPtr() { return m_graphVertexPtr; }

		KY_FORCE_INLINE KyUInt32 GetAstarId() const { return m_astarId; }

	private :
		GraphVertexPtr m_graphVertexPtr;
		KyUInt32 m_astarId;
	};

	// GraphCellSafePtrAndStitchedVersion
	/*!	For internal use.
		\ingroup kyruntime_graph */
	class GraphCellSafePtrAndStitchedVersion
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
	public :
		GraphCellSafePtrAndStitchedVersion() : m_graphCellSafePtr(), m_stitchedVersion(KyUInt32MAXVAL) {}
		GraphCellSafePtrAndStitchedVersion(const GraphCellSafePtr& graphCellSafePtr): m_graphCellSafePtr(graphCellSafePtr),
			m_stitchedVersion(graphCellSafePtr.GetStitchedGraphCell_UnSafe().m_stitchedVersion) {}

		KY_FORCE_INLINE const GraphCellSafePtr& GetGraphCellSafePtr() const { return m_graphCellSafePtr; }
		KY_FORCE_INLINE GraphCellSafePtr& GetGraphCellSafePtr() { return m_graphCellSafePtr; }

		KY_FORCE_INLINE KyUInt32 GetStitchedVersion() const { return m_stitchedVersion; }

	private :
		GraphCellSafePtr m_graphCellSafePtr;
		KyUInt32 m_stitchedVersion;
	};

protected:
	/* Used internally by AstarContext to remove internal pointers to Bots that are removed from the World. */
	class WorldObserver : public IWorldObserver
	{
	public:
		WorldObserver() : m_astarContext(KY_NULL) {}
		void Init(AstarContext& astarContext) { m_astarContext = &astarContext;}

		virtual void OnActivateBody(Body& ) {}
		virtual void OnDeactivateBody(Body& body) { m_astarContext->OnRemoveBody(body); }

	protected:
		AstarContext* m_astarContext;
	};
	friend class WorldObserver;

public:
	/*! \label_constructor */
	AstarContext();

	/*! \label_destructor */
	~AstarContext();

	/*! \label_clear */
	void Clear();

	/*! \label_reinit */
	void ReInit();

	/*! Retrieves the maximum number of vertices that can be treated by an A* calculation that uses this AstarContext. */
	KY_FORCE_INLINE KyUInt32 GetMaxVertexCount() const { return m_maxVertexCount; }

	/*! Retrieves the previous vertex in the best path found so far. This method should be called
		only during the course of an A* calculation. For example, it may be used in the 
		IConstraint::GetCost() method of the Constraint used during the A* calculation. */
	inline const GraphVertexPtr& GetAstarPredecessor(KyUInt32 vertexAstarId) const
	{
		KY_FUNCTION("AstarContext::GetAstarPredecessor");
		KY_ASSERT(m_predecessor != 0, ("invalid A* buffer"));
		KY_ASSERT(vertexAstarId <= m_maxVertexCount, ("invalid parameter"));
		KY_ASSERT(m_corruptedPredecessors == KY_FALSE, ("It seems you did not use AstarTraversal::GetFirstPathNode/GetNextPathNode as expected, "
			"please refer to AstarTraversal::GetFirstPathNode for details."));
		return m_predecessor[vertexAstarId];
	}


	/*! Retrieves the cost of the best path so far from the starting vertex up to a specified vertex.
		This method will only return a usable value if the specified vertex has been explored already
		(i.e. if the status of the vertex is not #EVertexStatus_NEW). This method should be called only
		during an A* computation. */
	inline KyFloat32 GetCostFromStart(KyUInt32 vertexAstarId) const
	{
		KY_FUNCTION("AstarContext::GetCostFromStart");
		KY_ASSERT(m_costFromStart != 0, ("invalid A* buffer"));
		KY_ASSERT(vertexAstarId <= m_maxVertexCount, ("invalid parameter"));
		return m_costFromStart[vertexAstarId];
	}

	/*! Retrieves an estimation of the cost of the best path found so far between a specified vertex and the final
		destination of the A* computation. This method will only return a usable value if the specified
		vertex has been explored already (i.e. if the status of the vertex is not #EVertexStatus_NEW).
		This method should be called only during an A* computation. */
	inline KyFloat32 GetEstimatedCostToDest(KyUInt32 vertexAstarId) const
	{
		KY_FUNCTION("AstarContext::GetEstimatedCostToDest");
		KY_ASSERT(m_estimatedCostToDest != 0, ("invalid A* buffer"));
		KY_ASSERT(vertexAstarId <= m_maxVertexCount, ("invalid parameter"));

		return m_estimatedCostToDest[vertexAstarId];
	}

	/*! Retrieves the current A* status of the specified vertex. This may be any value from the #VertexStatus
		enumeration. This method should be called only during an A* computation. */
	VertexStatus GetVertexStatus(KyUInt32 vertexAstarId) const { return m_twoBitsVerticesStatus.GetStatusByIndex(vertexAstarId); }
	inline void SetVertexStatus(KyUInt32 vertexAstarId, VertexStatus status) { m_twoBitsVerticesStatus.SetStatusByIndex(status, vertexAstarId); }

	/* Called transparently to manage A* computations sliced across several frames. */
	/*!	For internal use. */
	KyBool ReInitAstar(const GraphVertexSafePtr& start, const GraphVertexSafePtr& dest, const GraphManager& gr,
		IConstraint* constraint, void* heuristic, Bot* entity, NavTerrainTypeMask allowedTerrainTypes);

	/* Tells, after a call to ReInitAstar, if the Astar can be started from this astarContext */
	/*!	For internal use. */
	KY_FORCE_INLINE KyUInt32 CanStart() const { return m_graphCellSafePtrs.GetCount() != 0; }


	/*	Called transparently to manage A* computations done over several frames. */
	/*!	For internal use. */
	void AddExploredStitchedGraphCell(const GraphCellSafePtr& graphCellSafePtr);

	/*	Called transparently to retrieve the best vertex candidate so far. */
	/*!	For internal use. */
	List<GraphVertexPtrAndAstarId>::Iterator GetBestCandidate();

	/*!	For internal use. */
	void AstarProcessCandidate(const GraphVertexPtr& candidate, KyFloat32 estimatedCostFromCandidateToDest, KyFloat32 costToCandidate, const GraphVertexPtr& predecessor);

	/*!	For internal use. */
	bool AStar_ConstructPath(Path& path, const GraphVertexPtr& start, const GraphVertexPtr& end) const;

	/*!	For internal use. */
	void RegisterOnResize(AStarContextObserver* observer);
	/*!	For internal use. */
	void UnregisterOnResize(AStarContextObserver* observer);

	/*! Sets the World using this object. For internal use. */
	void SetWorld(World& world);
	/*! Sets the World using this object. For internal use. */
	void UnsetWorld();

	/*! \label_getworld */
	const World* GetWorld() const { return m_world; }
	/*! \label_getworld */
	World* GetWorld() { return m_world; }

	/*!	Retrieves the GraphManager currently using the AstarContext. */
	const GraphManager* GetGraphManager() const { return m_graphManager; }

	/*!	Retrieves the Bot currently using the AstarContext, if any. */
	const Bot* GetBot() const { return m_bot; }
	/*!	Retrieves the Bot currently using the AstarContext, if any. */
	Bot* GetBot() { return m_bot; }

	/*!	Retrieves the starting vertex for the current path computation. */
	const GraphVertexSafePtr& GetStart() const { return m_start; }

	/*!	Retrieves the destination vertex for the current path computation. */
	const GraphVertexSafePtr& GetDest() const { return m_dest; }

	/*!	Retrieves the terrain types allowed for the current path computation. */
	NavTerrainTypeMask GetAllowedTerrainTypes() { return m_allowedTerrainTypes; }

protected:
	/* Allocates memory for the AstarContext.
		\param maxVertexCount		The maximum number of \GraphVertices that may be explored
									by an A* calculation that uses this AstarContext. */
	void Allocate(KyUInt32 maxVertexCount);

	void UpdateClosedStatus(const GraphVertexPtr& candidate, KyFloat32 estimatedCostFromCandidateToDest, KyFloat32 costToCandidate, const GraphVertexPtr& predecessor);
	void UpdateOpenedStatus(const GraphVertexPtr& candidate, KyFloat32 estimatedCostFromCandidateToDest, KyFloat32 costToCandidate, const GraphVertexPtr& predecessor);
	void UpdateNewStatus(const GraphVertexPtr& candidate, KyFloat32 estimatedCostFromCandidateToDest, KyFloat32 costToCandidate, const GraphVertexPtr& predecessor);

	void OnRemoveBody(Body& oldBody);


protected:
	class TwoBitsVerticesStatus
	{
	public:
		TwoBitsVerticesStatus() : m_verticesStatus(KY_NULL), m_entriesCount(0) {}
		~TwoBitsVerticesStatus() { KY_FREE(m_verticesStatus); }

		void Allocate(KyUInt32 maxVertexCount)
		{
			const KyUInt32 oldVerticeStatusSize = GetVerticesStatusSize();
			TwoBitsValues* oldVerticesStatus = m_verticesStatus;

			m_entriesCount = maxVertexCount;
			m_verticesStatus = KY_MALLOC(TwoBitsValues, GetVerticesStatusSize());

			Copy(oldVerticesStatus, oldVerticesStatus + oldVerticeStatusSize, m_verticesStatus);
			KY_FREE(oldVerticesStatus);
		}
		void Deallocate()
		{
			KY_FREE(m_verticesStatus);
			m_verticesStatus = NULL;
			m_entriesCount = 0;
		}
		inline void SetStatusByIndex(const VertexStatus& status, KyUInt32 idx)
		{
			TwoBitsValues& cell = m_verticesStatus[idx / 4];
			switch (idx % 4)
			{
			case 0: cell.value0 = status; break;
			case 1: cell.value1 = status; break;
			case 2: cell.value2 = status; break;
			case 3: cell.value3 = status; break;
			}
		}
		inline VertexStatus GetStatusByIndex(KyUInt32 idx) const
		{
			TwoBitsValues cell = m_verticesStatus[idx / 4];
			switch (idx % 4)
			{
			case 0: return (VertexStatus)cell.value0;
			case 1: return (VertexStatus)cell.value1;
			case 2: return (VertexStatus)cell.value2;
			case 3: return (VertexStatus)cell.value3;
			}
			return EVertexStatus_NEW;
		}
		inline void Reset()
		{
			const KyUInt32 sizeOfTwoBitsValues = sizeof (TwoBitsValues);
			memset(m_verticesStatus, '\0', GetVerticesStatusSize()* sizeOfTwoBitsValues);
		}
	protected:
		KyUInt32 GetVerticesStatusSize() const { return (m_entriesCount + 3) / 4; }

		struct TwoBitsValues
		{
			KyUInt8 value0:2;
			KyUInt8 value1:2;
			KyUInt8 value2:2;
			KyUInt8 value3:2;
		};
	protected:
		TwoBitsValues* m_verticesStatus;
		KyUInt32 m_entriesCount;
	};

	typedef List<AStarContextObserver*> ObserverContainer;
public:
	List<GraphEdgeSafePtr>* m_poEdges; /*!< Contains a list of all edges hit during the last computation that are linked to one or more \PathObjects. */

	// The following variables are public for debug purposes only. They should not be modified.
protected:
	KyUInt32 m_maxVertexCount;
public:
	GraphVertexPtr* m_predecessor;  /*!	For internal use. Do not modify. */
	KyBool m_corruptedPredecessors; /*!	For internal use. Do not modify. */
protected:
	KyFloat32* m_costFromStart;
public:
	KyFloat32* m_estimatedCostToDest;  /*!	For internal use. Do not modify. */
protected:
	TwoBitsVerticesStatus m_twoBitsVerticesStatus;
public:
	List<GraphVertexPtrAndAstarId> m_opened;           /*! For internal use. Do not modify. */
	KyBool m_paused;                    /*!	For internal use. Do not modify. */
protected:
	GraphVertexSafePtr m_start;
	GraphVertexSafePtr m_dest;
	IConstraint* m_constraint;
	void* m_heuristic;
	Bot* m_bot;
	NavTerrainTypeMask m_allowedTerrainTypes;
	const GraphManager* m_graphManager;
	List<GraphCellSafePtrAndStitchedVersion> m_graphCellSafePtrs;
	ObserverContainer m_observers;
	WorldObserver m_worldObserver;
	World* m_world;
};


} // namespace Kaim

#endif // KY_ASTARCONTEXT_H
