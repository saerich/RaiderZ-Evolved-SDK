/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_GRAPHMANAGER_H
#define KY_GRAPHMANAGER_H

#include <kyruntime/graph/pointinfo_astarid.h>
#include <kyruntime/graph/pointinfo_vertexsafeptr.h>
#include <kyruntime/graph/path.h>
#include <kyruntime/graph/virtualedge.h>

#include <kyruntime/core/inextmovemanager.h>
#include <kyruntime/core/pointwrapper.h>

#include <kyruntime/mesh/navmeshqueryserverwrapper.h>

#include <kypathdata/graph/graphcell.h>
#include <kyruntime/graph/graphcellgrid.h>
#include <kyruntime/graph/graphstitchdatamanager.h>
#include <kypathdata/generic/referencemanager.h>
#include "kypathdata/generic/activeguids.h"

#include <kypathdata/shapes/ivolume.h>

#include <kyruntime/core/timemanager.h>

#include <kyruntime/graph/graphmanagervisualrepresentation.h>

#include "kypathdata/kycrt/math.h"

//0xFFFFFFF excluded because used for virtual edges
#define KyMaxSectorID (KyUInt32MAXVAL-2)

namespace Kaim
{

class PathObject;
class Database;
class AstarContext;
class PointInfo_VertexSafePtr;
class PointInfo_AstarId;
class PathObjectManager;
class NavMeshManager;
class StitchedGraphCell;
class GraphVertexPtr;
class GraphEdgePtr;
class VertexIteratorInWorld;
class EdgeIteratorInWorld;


/*!	Enumerates the errors that may be encountered during a search for nearby vertices carried out by calls to GraphManager::FindNearbyVertexSafePtrs(),
	GraphManager::FindNearbyVertexPtrs(), GraphManager::FindNearbySortedVertexSafePtrs() or GraphManager::FindNearbySortedVertexPtrs(). These calls may
	be made by the IFindNodesFromPositions pathfinding modifier, or on-demand by the user.
	\ingroup kyruntime_graph */
enum SearchNodeEvent
{
	/*!	Raised by GraphManager::FindNearbyVertexSafePtrs(), GraphManager::FindNearbyVertexPtrs(), GraphManager::FindNearbySortedVertexSafePtrs() and
		GraphManager::FindNearbySortedVertexPtrs() to indicate that no candidate vertices were found within 
		the specified bounding box. This may indicate that the DatabaseManager has not yet streamed the Graph for the 
		area into memory, or has already streamed the Graph out of memory. Alternatively, it may indicate that the provided 
		position lies at the wrong altitude, too far from the valid altitude of your PathData. */
	SearchNodeEvent_NoNodeFound = 1,
	/*!	Raised by GraphManager::FindNearbyVertexSafePtrs(), GraphManager::FindNearbyVertexPtrs(), GraphManager::FindNearbySortedVertexSafePtrs() and
		GraphManager::FindNearbySortedVertexPtrs() to indicate that the search found more vertices than could 
		be stored in the search array provided. When this occurs, the vertex that actually lies nearest to the provided position 
		may not be considered. */
	SearchNodeEvent_TooMuchNodeCollected = 2,
	/*!	Raised by GraphManager::FindNearbyVertexSafePtrs(), GraphManager::FindNearbyVertexPtrs(), GraphManager::FindNearbySortedVertexSafePtrs() or
		GraphManager::FindNearbySortedVertexPtrs() the IFindNodesFromPositions modifier to indicate that at 
		least one of the vertices collected by the search is invalid. */
	SearchNodeEvent_InvalidVertexId = 4,
	/*!	Raised by the IFindNodesFromPositions modifier to indicate that at least one of the vertices collected by the search cannot 
		be reached from the specified position due to static obstacles in the NavMesh, dynamic \Obstacles, or blocking \PathObjects.  */
	SearchNodeEvent_BlockedBy_CanGo = 8,
	/*!	Raised by GraphManager::FindNearbyVertexSafePtrs(), GraphManager::FindNearbyVertexPtrs(), GraphManager::FindNearbySortedVertexSafePtrs() and
		GraphManager::FindNearbySortedVertexPtrs() to indicate that at least one of the vertices collected by the 
		search cannot be reached from the specified position due to \Obstacles that cannot be bypassed.  */
	SearchNodeEvent_BlockedBy_LpfConstraint = 16,

	/*! Raised by GraphManager::FindNearbyVertexSafePtrs(), GraphManager::FindNearbyVertexPtrs(), GraphManager::FindNearbySortedVertexSafePtrs() and
		GraphManager::FindNearbySortedVertexPtrs() to indicate that at least one of the vertices collected by the
		search cannot be reached from the specified position due to \Obstacles that cannot be bypassed.  */
	SearchNodeEvent_BlockedBy_Constraint = 32,
	KY_DUMMY_SearchNodeEvent = KYFORCEENUMSIZEINT
};


///////////////////////////////////////////////////////////////////////////////////////////
// GraphManager
///////////////////////////////////////////////////////////////////////////////////////////

/*! Enumerates the possible status values of the last A* calculation carried out by the GraphManager.
	\ingroup kyruntime_graph */
enum AstarStatus 
{
	ASTAR_PATH_FOUND, /*!< Indicates that a Path was successfully calculated. */
	ASTAR_PATH_NOT_FOUND, /*!< Indicates that no Path could be computed between the specified vertices. */
	ASTAR_PATH_IN_CONSTRUCTION, /*!< Indicates that the A* calculation has been paused, and will resume in the next frame. */
	_AstarStatus_dummy = KYFORCEENUMSIZEINT
};


/*!	This class is a runtime container for the \Graphs that represent the individual sectors in your terrain. 
	It provides a single point of access to the current set of \Graphs loaded into memory. 
	\pp Each Database owned by the World creates and initializes one instance of the GraphManager class, accessible through
	Database::GetGraphManager(). Any time you add PathData to a Database, the Graph contained in the PathData is added
	transparently to the GraphManager. The GraphManager stitches these separate \Graphs together transparently at
	runtime by creating virtual edges between their vertices.
	\pp The GraphManager also stitches in \Graphs created dynamically by \PathObjects to represent their topologies: i.e.
	the connections between their connection spaces. For more information, see :USERGUIDE:"Using PathObjects for Special Movements". 
	\pp You can use the GraphManager:
	-	To find vertices and edges in the \Graphs that have been added to the GraphManager.
	-	To launch an A* calculation that finds the shortest path between two specified vertices. 
	\ingroup kyruntime_graph */
class GraphManager : public VirtualMemObject
{
	GraphManager& operator=(const GraphManager&);
public:
	/*!	\label_constructor For internal use. */
	GraphManager();
	
	/*!	\label_virtualdestructor For internal use. */
	~GraphManager();

	void Clear();
	void Reset();


public:
	/************************************************************************/
	/* VisualGeometry                                                       */
	/************************************************************************/

	/*! Sets the instance of IVisualGeometry to which this object will send its triangles for rendering when
		ComputeVisualGeometry() is called. */
	void SetVisualGeometry(IVisualGeometry* geometry) { m_visualRepresentation.m_geometry = geometry; }

	/*! Retrieves the instance of IVisualGeometry to which this object sends its triangles for rendering, 
		or #KY_NULL if none has been set. */
	IVisualGeometry* GetVisualGeometry() { return m_visualRepresentation.m_geometry; }

	/*! Retrieves the instance of the GraphManagerVisualRepresentation maintained by this object, which describes 
		the way its data is expressed as triangles for rendering. */
	GraphManagerVisualRepresentation& GetVisualRepresentation() { return m_visualRepresentation; }

	/*! Sends to the IVisualGeometry a set of triangles that express the data maintained by this object for rendering.
		See :USERGUIDE:"Rendering PathData". */
	void ComputeVisualGeometry() { m_visualRepresentation.ComputeGeometry(); }

public:
	/*! For internal use. */
	void StartMultipleInsertion(); //<! function to tell the GraphManager to not Reset the navCellGrid before the call of EndMultipleInsertion() Can be used to insert several graphes/navCells
	/*! For internal use. */
	void EndMultipleInsertion();   //<! function to tell the GraphManager the insertion is done, it can reset the navCellGrid

	/*! For internal use. */
	void StartMultipleRemoval(); //<! function to tell the GraphManager to not Reset the navCellGrid before the call of EndMultipleRemoval(). Can be used to remove several graphes/navCells
	/*! For internal use. */
	void EndMultipleRemoval();   //<! function to tell the GraphManager the insertion is done, it can reset the navCellGrid

	/*! For internal use. */
	GraphReference* AddGraph(const Graph& graph); //<! insert a Graph. If StartMultipleInsertion() has not been called before, the navCellGrid will be updated

	/*! For internal use. */
	GraphReference* AddAdditionalGraph(const AdditionalGraph& additionalGraph);

	/*! For internal use. */
	KyResult RemoveGraph(const Graph& graph); //<! remove a Graph. If StartMultipleRemoval() has not been called before, the navCellGrid will be reseted
	/*! For internal use. */
	KyResult RemoveGraph(StitchedGraph& graph); //<! remove a Graph. If StartMultipleRemoval() has not been called before, the navCellGrid will be reseted

	/*! For internal use. */
	KyResult RemoveAdditionalGraph(const AdditionalGraph& additionalGraph);
	/*! For internal use. */
	KyResult RemoveAdditionalGraph(AdditionalStitchedGraph& additionalStitchedGraph);

	/*! For internal use. */
	StitchedGraph* GetStitchedGraph(const Graph& graph) { return m_stitchDataManager.GetStitchedGraph(graph); }
	/*! For internal use. */
	AdditionalStitchedGraph* GetAdditionalStitchedGraph(const AdditionalGraph& additionalGraph) { return m_stitchDataManager.GetAdditionalStitchedGraph(additionalGraph); }

	/*! For internal use. */
	StitchedGraph* GetStitchedGraph(const GuidCompound& guidCompound) const { return m_stitchDataManager.GetStitchedGraph(guidCompound); }
	/*! Retrieves a pointer to the AdditionalStitchedGraph created by the GraphManager to manage the additional graph with the specified
		GUID, if any. If the specified GUID does not match any additional graphs currently being managed by the GraphManager, this method
		returns #KY_NULL.
		\pp See also :USERGUIDE:"Using Custom Additional Graphs". */
	AdditionalStitchedGraph* GetAdditionalStitchedGraph(const GuidCompound& guidCompound) const { return m_stitchDataManager.GetAdditionalStitchedGraph(guidCompound); }

public:
	/************************************************************************/
	/* access graph data                                                    */
	/************************************************************************/
	
	/*! Retrieves a GraphEdgePtr that identifies the virtual edge with the specified index. */
	GraphEdgePtr GetVirtualEdgePtr(VirtualEdgeIdx edgeIdx) const;

	/*! Retrieves a GraphEdgePtr that identifies the virtual edge with the specified index.
		\param edgeIdx				The index of the virtual edge to retrieve.
		\param[out] edgePtr			Updated to store the retrieved GraphEdgePtr. */
	void GetVirtualEdgePtr(VirtualEdgeIdx edgeIdx, GraphEdgePtr& edgePtr) const;

	/*! Retrieves the virtual edge with the specified index. */
	VirtualEdge& GetVirtualEdge(VirtualEdgeIdx edgeIdx) { return m_graphCellGrid.m_additionalGraphStitcher.GetVirtualEdge(edgeIdx); }

	/*! Retrieves the virtual edge with the specified index. */
	const VirtualEdge& GetVirtualEdge(VirtualEdgeIdx edgeIdx) const { return m_graphCellGrid.m_additionalGraphStitcher.GetVirtualEdge(edgeIdx); }

	/*! Retrieves the number of virtual edges currently maintained by the GraphManager. */
	KyUInt32 GetVirtualEdgeCount() const { return m_graphCellGrid.m_additionalGraphStitcher.GetVirtualEdgeCount(); }

	/*! Retrieves the vertex in the Graph that was created from the input position specified by \c pos, if any. If \c pos
		does not match a vertex position provided as input for this sector, returns an invalid GraphVertexPtr.
		\param pos				A position in 3D space originally provided as input to the PathData generation system. 
								Should be expressed in the \gameOrSim coordinate system. */
	GraphVertexPtr GetGraphVertexPtrFromInputGraphVertexPos(const Vec3f& pos) const;

public:
	/************************************************************************/
	/* Iterators                                                            */
	/************************************************************************/

	/*!	Retrieves an iterator that allows you to iterate through all vertices maintained by the GraphManager. */
	VertexIteratorInWorld GetVertexIterator() const;

	/*!	Retrieves an iterator that allows you to iterate through all edges maintained by the GraphManager. */
	EdgeIteratorInWorld GetEdgeIterator() const;


public:
	/************************************************************************/
	/* Astar                                                                */
	/************************************************************************/

	/*!	Computes the shortest path between two vertices in the GraphManager. This method is invoked by the 
		PathFinder and by the AstarTraversal exploration class. If you use these components for 
		your pathfinding and perception, you do not need to call this method directly. However, you may use
		this method if you write a custom component that needs to compute a path between two vertices.
		\param start				The starting vertex for the path calculation.
		\param end					The destination vertex for the path calculation.
		\param[out] path			Stores the Path computed between \c start and \c end.
		\param constraint			An object that derives from IConstraint, used to evaluate the relative cost of each edge. 
		\param heuristic			An object that derives from IHeuristic, used to guide the propagation of the A* algorithm. 
		\param entity 				A pointer to a Bot that will be considered the owner of this operation 
									for the purposes of managing time slicing and priority. This pointer may be 
									#KY_NULL if you do not wish to assign an Bot.
		\param astarContext			Stores temporary data during the computation. You can create a new AstarContext 
									from scratch, or borrow the one maintained by the World by calling World::GetAstarContext(). 
									However, if you borrow this AstarContext, keep in mind that it may reflect the saved state
									of a paused time-sliced calculation begun in the previous frame. In this case, your new A*
									calculation will cancel the paused calculation.
		\param allowedTerrainTypes	A bitmask that identifies the terrain types that will be considered navigable during the A* 
									calculation. By default, all terrain types are allowed.
		\param nbEdges				The number of edges to process before checking CPU consumption. Optional; by default,
									no limit is used.
		\param astarTaskId			The id of an aperiodic task used to determine whether or not the CPU time allotted
									for the A* calculation has expired for the current frame. Optional; by default, no
									time-slicing is performed.
		\return An element of the #AstarStatus enumeration that indicates the state of the calculation. When this
				method returns #ASTAR_PATH_FOUND, you can retrieve the path from the Path object you passed to the
				\c path parameter. */
	AstarStatus Astar(const GraphVertexPtr& start, const GraphVertexPtr& end, Path& path,
		IConstraint& constraint, IHeuristic& heuristic, Bot* entity,
		AstarContext& astarContext, NavTerrainTypeMask allowedTerrainTypes=NavTerrainTypeMask_All,
		KyUInt32 nbEdges=KyUInt32MAXVAL, TimeManager::TaskId astarTaskId=TimeManager::InvalidTaskId) const;

	/*!	Retrieves the number of A* loops carried out by all GraphManager objects in the current frame. Used to calculate
		an accurate estimation for the \c GraphManagerAstarLoop task when using deterministic time measurement. 
		See :USERGUIDE:"Using Deterministic Time Measurement". */
	static KyUInt32 GetAstarLoopCounter();

protected:
	KyBool Astar_ConstructPath(Path& path, const GraphVertexPtr& start, const GraphVertexPtr& end, AstarContext& astarContext) const;

	void Astar_ProcessEdge(const GraphEdgePtr& edge, const GraphVertexPtr& dest, AstarContext& astarContext,
		IConstraint& constraint, IHeuristic& heuristic, Bot* entity) const;


public:
	/************************************************************************/
	/* Rendering                                                            */
	/************************************************************************/

	/*!	Renders all vertices and edges contained in all \Graphs currently added to the GraphManager. 
		-	Vertices are drawn as red points.
		-	Vertices that lie outside the boundaries of the NavMesh are drawn as yellow points.
		-	Vertices in additional graphs are drawn in magenta if they are not linked to the surrounding Graph,
			or in white if they are linked to the surrounding Graph.
		-	Edges are drawn in green.
		-	Edges that are linked to \PathObjects are drawn in yellow.
		-	Edges that link two different \Graphs are drawn in cyan.
		-	Edges that are linked to \PathObjects \b and that link two different \Graphs are drawn in salmon. 
		-	Edges in additional graphs are drawn in darker yellow.
		\pp \param uidGraph		Specifies the UID of a single Graph. If you include this argument, only the vertices and
								edges in the specified Graph are rendered. If you omit the argument or provide #KY_NULL, 
								all \Graphs are rendered. */
	void Render(const KyUInt32* uidGraph = KY_NULL) const;


public:
	/************************************************************************/
	/* FindNearestData                                                      */
	/************************************************************************/

	/*---------------- Find vertices in a Bounding Box 3d ----------------*/
	/*!	Retrieves all vertices that lie within the specified axis-aligned bounding box.
		\param aabbMin			The minima of the bounding box: the corner with the lowest values on the X, Y and Z axes.
								Diagonally opposite to \c aabbMax.
		\param aabbMax			The maxima of the bounding box: the corner with the highest values on the X, Y and Z axes.
								Diagonally opposite to \c aabbMin.
		\param[out] vertices	Stores a GraphVertexSafePtr for each vertex found in the specified volume.
		\param maxVertices		The maximum number of elements that can be stored in the \c vertices array.
		\return The number of vertices found: i.e. the number of GraphVertexSafePtr objects written to the \c vertices array. */
	KyUInt32 FindVertexSafePtrsInBox3f(const Vec3f& aabbMin, const Vec3f& aabbMax, GraphVertexSafePtr* vertices, KyUInt32 maxVertices) const;

		/*!	Retrieves all vertices that lie within the specified axis-aligned bounding box.
		\param aabbMin			The minima of the bounding box: the corner with the lowest values on the X, Y and Z axes.
								Diagonally opposite to \c aabbMax.
		\param aabbMax			The maxima of the bounding box: the corner with the highest values on the X, Y and Z axes.
								Diagonally opposite to \c aabbMin.
		\param[out] vertices	Stores a GraphVertexPtr for each vertex found in the specified volume.
		\param maxVertices		The maximum number of elements that can be stored in the \c vertices array.
		\return The number of vertices found: i.e. the number of GraphVertexSafePtr objects written to the \c vertices array. */
	KyUInt32 FindVertexPtrsInBox3f(const Vec3f& aabbMin, const Vec3f& aabbMax, GraphVertexPtr* vertices, KyUInt32 maxVertices) const;


	/*---------------- Find vertices From a position ----------------*/

	/*!	Retrieves vertices that lie close to a specified three-dimensional position.
		\param pos				The reference position.
		\param[out] vertices	Stores a GraphVertexSafePtr for each vertex found near \c pos.
		\param maxVertices		The maximum number of elements that can be stored in the \c vertices array.
		\param searchNodeEvents	Bit mask that indicates errors encountered during the search. This is composed of
								elements of the #SearchNodeEvent enumeration.
		\return The number of vertices found: i.e. the number of GraphVertexSafePtr objects written to the \c vertices array. */
	KyUInt32 FindNearbyVertexSafePtrs(const Vec3f& pos, GraphVertexSafePtr* vertices, KyUInt32 maxVertices, KyUInt32* searchNodeEvents = 0) const;

		/*!	Retrieves vertices that lie close to a specified three-dimensional position.
		\param pos				The reference position.
		\param[out] vertices	Stores a GraphVertexPtr for each vertex found near \c pos.
		\param maxVertices		The maximum number of elements that can be stored in the \c vertices array.
		\param searchNodeEvents	Bit mask that indicates errors encountered during the search. This is composed of
								elements of the #SearchNodeEvent enumeration.
		\return The number of vertices found: i.e. the number of GraphVertexPtr objects written to the \c vertices array. */
	KyUInt32 FindNearbyVertexPtrs(const Vec3f& pos, GraphVertexPtr* vertices, KyUInt32 maxVertices, KyUInt32* searchNodeEvents = 0) const;

	/*---------------- Find vertices From a position with an altitude difference max----------------*/

	/*!	Retrieves vertices that lie close to a specified three-dimensional position, within a specified difference in altitude.
		\param pos				The reference position.
		\param diffAltitudeMax	Determines the maximum difference in altitude between \c pos and any returned vertex.
		\param[out] vertices	Stores a GraphVertexSafePtr for each vertex found near \c pos.
		\param maxVertices		The maximum number of elements that can be stored in the \c vertices array.
		\param searchNodeEvents	Bit mask that indicates errors encountered during the search. This is composed of
								elements of the #SearchNodeEvent enumeration.
		\return The number of vertices found: i.e. the number of GraphVertexSafePtr objects written to the \c vertices array. */
	KyUInt32 FindNearbyVertexSafePtrsInAltitudeRange(const Vec3f& pos, KyFloat32 diffAltitudeMax, GraphVertexSafePtr* vertices, KyUInt32 maxVertices, KyUInt32* searchNodeEvents = 0) const;

		/*!	Retrieves vertices that lie close to a specified three-dimensional position, within a specified difference in altitude.
		\param pos				The reference position.
		\param diffAltitudeMax	Determines the maximum difference in altitude between \c pos and any returned vertex.
		\param[out] vertices	Stores a GraphVertexPtr for each vertex found near \c pos.
		\param maxVertices		The maximum number of elements that can be stored in the \c vertices array.
		\param searchNodeEvents	Bit mask that indicates errors encountered during the search. This is composed of
								elements of the #SearchNodeEvent enumeration.
		\return The number of vertices found: i.e. the number of GraphVertexPtr objects written to the \c vertices array. */
	KyUInt32 FindNearbyVertexPtrsInAltitudeRange(const Vec3f& pos, KyFloat32 diffAltitudeMax, GraphVertexPtr* vertices, KyUInt32 maxVertices, KyUInt32* searchNodeEvents = 0) const;


	/*!	Defines a type of callback function that FindNearbySortedVertexSafePtrs() and FindNearbySortedVertexPtrs() can use to sort
		the vertices it finds. This function is called once for each vertex found. It should calculate a relative cost
		for each vertex, typically according to the cost to reach that vertex from a specified position, and should save
		the cost to the \c cost parameter.
		\param pos				The \c pos parameter passed to FindNearbySortedVertexSafePtrs() and FindNearbySortedVertexPtrs().
		\param vec				One of the vertices found by FindNearbySortedVertexSafePtrs() and FindNearbySortedVertexPtrs() near the specified position.
		\param[out] cost		The cost calculated for the \c vec vertex.
		\param userData			The \c userData parameter passed to FindNearbySortedVertexSafePtrs() and FindNearbySortedVertexPtrs()).
		\return #KY_TRUE if the calculation has been completed successfully and the \c cost parameter has been
				updated, or #KY_FALSE otherwise. */
	typedef KyBool (*GetCostFunction)(const Vec3f& pos, const GraphVertexPtr& vec, KyFloat32& cost, void* userData);

	/*---------------- Find sorted vertices From a position ----------------*/

	/*!	Retrieves vertices that lie close to a specified three-dimensional position, and sorts them by increasing
		cost. The cost of each vertex is determined by calling a callback function specified in the \c getCost parameter.
		\param pos				The reference position.
		\param[out] vertices	Stores a GraphVertexPtr for each vertex found.
		\param maxVertices		The maximum number of elements that can be stored in the \c vertices array.
		\param getCost			A callback function used to calculate the cost of each vertex. This callback function 
								must satisfy the GetCostFunction typedef.
		\param userData			Passed to the \c userData parameter of the \c getCost function.
		\param searchNodeEvents	Bit mask that indicates errors encountered during the search. This is composed of
								elements of the #SearchNodeEvent enumeration.
		\return The number of vertices found: i.e. the number of GraphVertexPtr objects written to the \c vertices array. */
	KyUInt32 FindNearbySortedVertexPtrs(const Vec3f& pos, GraphVertexPtr* vertices, KyUInt32 maxVertices, 
		GetCostFunction getCost = 0, void* userData = 0, KyUInt32* searchNodeEvents = 0) const;

	/*!	Retrieves vertices that lie close to a specified three-dimensional position, and sorts them by increasing
		cost. The cost of each vertex is determined by calling a callback function specified in the \c getCost parameter.
		\param pos				The reference position.
		\param[out] vertices	Stores a GraphVertexSafePtr for each vertex found.
		\param maxVertices		The maximum number of elements that can be stored in the \c vertices array.
		\param getCost			A callback function used to calculate the cost of each vertex. This callback function 
								must satisfy the GetCostFunction typedef.
		\param userData			Passed to the \c userData parameter of the \c getCost function.
		\param searchNodeEvents	Bit mask that indicates errors encountered during the search. This is composed of
								elements of the #SearchNodeEvent enumeration.
		\param costs            (optional) Store given costs for each vertex found.
		\return The number of vertices found: i.e. the number of GraphVertexSafePtr objects written to the \c vertices array. */
	KyUInt32 FindNearbySortedVertexSafePtrs(const Vec3f& pos, GraphVertexSafePtr* vertices, KyUInt32 maxVertices, 
		GetCostFunction getCost = 0, void* userData = 0, KyUInt32* searchNodeEvents = 0, KyFloat32* costs = KY_NULL) const;

	/*---------------- Find sorted vertices From a position with an altitude difference max----------------*/

	/*!	Retrieves vertices that lie close to a specified three-dimensional position, within a specified difference in altitude, and sorts them by increasing
		cost. The cost of each vertex is determined by calling a callback function specified in the \c getCost parameter.
		\param pos				The reference position.
		\param diffAltitudeMax	Determines the maximum difference in altitude between \c pos and any returned vertex.
		\param[out] vertices	Stores a GraphVertexSafePtr for each vertex found.
		\param maxVertices		The maximum number of elements that can be stored in the \c vertices array.
		\param getCost			A callback function used to calculate the cost of each vertex. This callback function 
								must satisfy the GetCostFunction typedef.
		\param userData			Passed to the \c userData parameter of the \c getCost function.
		\param searchNodeEvents	Bit mask that indicates errors encountered during the search. This is composed of
								elements of the #SearchNodeEvent enumeration.
		\param costs            (optional) Store given costs for each vertex found.
		\return The number of vertices found: i.e. the number of GraphVertexSafePtr objects written to the \c vertices array. */
	KyUInt32 FindNearbySortedVertexSafePtrsInAltitudeRange(const Vec3f& pos, KyFloat32 diffAltitudeMax, GraphVertexSafePtr* vertices, KyUInt32 maxVertices, 
		GetCostFunction getCost = 0, void* userData = 0, KyUInt32* searchNodeEvents = KY_NULL, KyFloat32* costs = KY_NULL) const;

	/*!	Retrieves vertices that lie close to a specified three-dimensional position, within a specified difference in altitude, and sorts them by increasing
		cost. The cost of each GraphVertex is determined by calling a callback function specified in the \c getCost parameter.
		\param pos				The reference position.
		\param diffAltitudeMax	Determines the maximum difference in altitude between \c pos and any returned vertex.
		\param[out] vertices	Stores a GraphVertexPtr for each vertex found.
		\param maxVertices		The maximum number of elements that can be stored in the \c vertices array.
		\param getCost			A callback function used to calculate the cost of each vertex. This callback function 
								must satisfy the GetCostFunction typedef.
		\param userData			Passed to the \c userData parameter of the \c getCost function.
		\param searchNodeEvents	Bit mask that indicates errors encountered during the search. This is composed of
								elements of the #SearchNodeEvent enumeration.
		\return The number of vertices found: i.e. the number of GraphVertexPtr objects written to the \c vertices array. */
	KyUInt32 FindNearbySortedVertexPtrsInAltitudeRange(const Vec3f& pos, KyFloat32 diffAltitudeMax, GraphVertexPtr* vertices, KyUInt32 maxVertices, 
		GetCostFunction getCost = 0, void* userData = 0, KyUInt32* searchNodeEvents = 0) const;

private:
	KyUInt32 FindVertexSafePtrsInBox2f(const Vec2f& aabbMin, const Vec2f& aabbMax, GraphVertexSafePtr* vertices, KyUInt32 maxVertices) const;
	KyUInt32 FindVertexPtrsInBox2f(const Vec2f& aabbMin, const Vec2f& aabbMax, GraphVertexPtr* vertices, KyUInt32 maxVertices) const;

public:
	/*!	Retrieves the number of \Graphs treated by the GraphManager::FindNearbyVertexSafePtrs(), GraphManager::FindNearbyVertexPtrs(),
		GraphManager::FindNearbySortedVertexSafePtrs() or GraphManager::FindNearbySortedVertexPtrs() methods. 
		Used to calculate an accurate estimation for the \c GraphManagerFindNearbyNodes estimation when using deterministic time measurement.
		See :USERGUIDE:"Using Deterministic Time Measurement". */
	static KyUInt32 GetFindNearbyVerticesCounter();

	/*! Retrieves a vertex at random from one of the \Graphs currently loaded into the GraphManager, or #KY_NULL 
		if no \Graphs have been loaded yet. */
	GraphVertexPtr GetRandomVertexPtr() const;

	/*!	Retrieves a pointer to the Database that owns this GraphManager, if any. */
	Database* GetDatabase() const { return m_database; }

	/*!	Sets the NavMeshQueryServerWrapper that will be used to perform CanGo queries during graph stitching. Set transparently by the Database. */
	void SetNavMeshQueryServer(NavMeshQueryServerWrapper& queryServer)
	{
		m_graphCellGrid.m_additionalGraphStitcher.SetNavMeshQueryServer(queryServer);
	}

	/*!	Retrieves the maximum number of vertices that can be loaded into this GraphManager. */
	KyUInt32 GetMaxVertexCount() const { return m_graphCellGrid.GetMaxVertexCount(); }

	/*!	Retrieves the number of static \Graphs currently loaded into this GraphManager. */
	KyUInt32 GetGraphLoadedCount() const { return m_stitchDataManager.m_numberOfStitchedGraph; }

public:
	/*! @{ */
	/*! For internal use. */
	const GraphReferenceManager& GetGraphReferenceManager() const { return m_graphReferenceManager; }
	GraphReferenceManager& GetGraphReferenceManager() { return m_graphReferenceManager; }

	const EdgeLocker& GetEdgeLocker() const { return m_graphCellGrid.m_edgeLocker; }
	EdgeLocker& GetEdgeLocker() { return m_graphCellGrid.m_edgeLocker; }



	KY_FORCE_INLINE CellPos ComputeGraphCellPos(const Vec3f& pos) const
	{
		KY_FUNCTION("GraphManager::ComputeGraphCellPos");
		KY_ASSERT(m_cellSize != KyFloat32MAXVAL, ("You should insert at least a valid graph first"));

		// Ceilf - 1 on x and Floorf on y to be IsInsideTriangle compatible
		return CellPos((KyInt32) Ceilf((pos.x * m_cellSizeInv)) - 1, (KyInt32) Floorf((pos.y * m_cellSizeInv)));
	}
	KY_FORCE_INLINE void ComputeGraphCellPos(const Vec3f& pos, CellPos& cellPos) const
	{
		KY_FUNCTION("GraphManager::ComputeGraphCellPos");
		KY_ASSERT(m_cellSize != KyFloat32MAXVAL, ("You should insert at least a valid graph first"));

		// Ceilf - 1 on x and Floorf on y to be IsInsideTriangle compatible
		cellPos.Set((KyInt32) Ceilf((pos.x * m_cellSizeInv)) - 1, (KyInt32) Floorf((pos.y * m_cellSizeInv)));
	}

	KY_FORCE_INLINE CellPos ComputeGraphCellPos(const Vec2f& pos) const
	{
		KY_FUNCTION("GraphManager::ComputeGraphCellPos");
		KY_ASSERT(m_cellSize != KyFloat32MAXVAL, ("You should insert at least a valid graph first"));

		// Ceilf - 1 on x and Floorf on y to be IsInsideTriangle compatible
		return CellPos((KyInt32) Ceilf((pos.x * m_cellSizeInv)) - 1, (KyInt32) Floorf((pos.y * m_cellSizeInv)));
	}
	KY_FORCE_INLINE void ComputeGraphCellPos(const Vec2f& pos, CellPos& cellPos) const
	{
		KY_FUNCTION("GraphManager::ComputeGraphCellPos");
		KY_ASSERT(m_cellSize != KyFloat32MAXVAL, ("You should insert at least a valid graph first"));

		// Ceilf - 1 on x and Floorf on y to be IsInsideTriangle compatible
		cellPos.Set((KyInt32) Ceilf((pos.x * m_cellSizeInv)) - 1, (KyInt32) Floorf((pos.y * m_cellSizeInv)));
	}

public:
	/*! Indicates whether or not the specified Graph was created with the same generation parameters
		as the \Graphs that have already been loaded into this GraphManager. */
	bool IsCompatibleWith(const Graph& graph) const;

	/*! Retrieves the value set by a call to SetCoverageDistance(). */
	KyFloat32 GetCoverageDistance() const { return m_coverageDistance != KyFloat32MAXVAL ? m_coverageDistance : m_cellSize; }

	/*! Sets the value used by the GraphManager to determine the size of the volume used to retrieve Graph vertices
		in calls to the FindNearby...() methods. 
		\pp By default, the GraphManager uses a value determined automatically by the PathData generation system and
		saved in the PathData. This default value should be appropriate for most projects. Lowering the default value
		may increase performance in some cases by reducing the search space and lowering the number of candidate vertices
		to check. However, lowering the value too far can result in not finding any valid candidate vertices.
		\acceptedvalues	Any positive value. */
	void SetCoverageDistance(KyFloat32 coverageDistance)
	{
		KY_FUNCTION("GraphManager::SetCoverageDistance");

		if (coverageDistance <= 0.f)
		{
			KY_MESSAGE(KY_LOG_WARNING, ("Incorect argument, keeping old value (%f)", m_coverageDistance));
			return;
		}
		m_coverageDistance = coverageDistance;
	}

	void SetDefaultValues();

	/*! @} */
private:
	void LinkWaitingAdditionalGraphes();
	void FindVertexSafePtrsInBox3fFromCellPos(const CellPos& cellPos, const CellBox& cellBox, const Box3f& box3f, GraphVertexSafePtr* vertices, KyUInt32 maxVertices, KyUInt32& numberOfVertexFound) const;
	void FindVertexPtrsInBox3fFromCellPos(const CellPos& cellPos, const CellBox& cellBox, const Box3f& box3f, GraphVertexPtr* vertices, KyUInt32 maxVertices, KyUInt32& numberOfVertexFound) const;

public:
	/*! @{ */
	/*! For internal use. Do not modify. */
	Database* m_database;

	GraphStitchDataManager m_stitchDataManager;
	GraphReferenceManager m_graphReferenceManager;
	GraphCellGrid m_graphCellGrid;

	KyFloat32 m_cellSize;
	KyFloat32 m_cellSizeInv;
private:
	KyFloat32 m_coverageDistance; /*< coverage distance of the graph (used as hint to find nearby vertices) */
public:
	ActiveGuids m_activatedGuids;
	/*! @} */
private:
	friend class GraphCellGrid;
	KyUInt32 m_numberOfGraphToUpdate;
	CellBox m_cellBox;

	typedef KyUInt32 InsertionMode;
	static const InsertionMode SINGLE_INSERTION = 0;
	static const InsertionMode MULTIPLE_INSERTION = 1;
	InsertionMode m_insertionMode;

	typedef KyUInt32 RemovalMode;
	static const RemovalMode SINGLE_REMOVAL = 0;
	static const RemovalMode MULTIPLE_REMOVAL = 1;
	RemovalMode m_removalMode;

	List<AdditionalStitchedGraph*> m_waitingAdditionalGraphs;

	GraphManagerVisualRepresentation m_visualRepresentation;

protected:
	mutable PointWrapper m_pointWrapper1, m_pointWrapper2;
	mutable PointInfo_VertexSafePtr m_vertexSafePtrInfo1, m_vertexSafePtrInfo2;
	mutable PointInfo_AstarId m_astarIdInfo1, m_astarIdInfo2;
};


} // namespace Kaim

#endif // KY_GRAPHMANAGER_H
