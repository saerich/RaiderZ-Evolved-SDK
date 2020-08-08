/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/



/*!	\file
	\ingroup kyruntime_pathobjects */

// primary contact: LAPA - secondary contact: MAMU
#ifndef KY_IPATHOBJECT_H
#define KY_IPATHOBJECT_H

#include <kypathdata/containers/list.h>
#include <kypathdata/pathobject/ipathobjectinfo.h>
#include <kyruntime/core/pointwrapper.h>
#include <kyruntime/core/inextmovemanager.h>
#include <kyruntime/pathobjects/pathobjectsmoothquery.h>


namespace Kaim
{
class AdditionalStitchedGraph;
class PathObjectManager;
class DatabaseManager;
class Database;
class StitchedGraphCell;
class GraphVertexSafePtr;
class World;
class Bot;

class PointInfo_ConnexionSpace;
class PointInfo_ControlSpace;


/*! Defines a type for a value built up from elements of the #PathObjectRenderFlags enumeration. Passed in a call to
	IPathObject::Render().
	\ingroup kyruntime_pathobjects */
typedef KyUInt32 PathObjectRenderMasks;

/*! Enumerates the types of information that can be rendered about a PathObject. Passed in a call to IPathObject::Render().
	\ingroup kyruntime_pathobjects */
enum PathObjectRenderFlags
{
	// Graph rendering flags
	PathObjectRender_InternalGraph = 1<<0, /*!< Activates rendering of the Graph built internally by the PathObject to represent its topology. Not currently used. */
	PathObjectRender_ConnexionLinks = 1<<1, /*!< Activates rendering of the links between the internal Graph and the surrounding static Graph. Not currently used. */
	PathObjectRender_LinkedEdges = 1<<2, /*!< Activates rendering of the edges associated to the PathObject. Not currently used. */

	// Spaces rendering flags
	PathObjectRender_ConnexionSpaces = 1<<4, /*!< Activates rendering of the connection spaces assigned to this PathObject. Not currently used. */
	PathObjectRender_ControlSpaces = 1<<5, /*!< Activates rendering of the control spaces assigned to this PathObject. Not currently used. */

	// Misc rendering flags
	PathObjectRender_Obstacles = 1<<6, /*!< Activates rendering of blocking control spaces assigned to this PathObject. Not currently used. */
	PathObjectRender_ControledBots = 1<<7,  /*!< Activates rendering of the \Bots currently using this PathObject. */

	// Data used to create the PathObject
	PathObjectRender_Data = 1<<9, /*!< Activates rendering of the data used to create this PathObject. Not currently used. */

	PathObjectRender__dummy__forceEnumSize = KYFORCEENUMSIZEINT
};


/*! For internal use by FindStartNode().
	\ingroup kyruntime_pathobjects */
enum PathObject_FindStartNode_Status
{
	PathObject_FindStartNode_UnderProcess = 0,
	PathObject_FindStartNode_NodeFound,
	PathObject_FindStartNode_NodeNotFound,
	PathObject_FindStartNode_dummy_forceenumsize = KYFORCEENUMSIZEINT
};


/*!	IPathObject is an abstract base class for dynamic objects that make \Bots move from place to place in special ways, or
	that place special constraints over the way \Bots can move in defined areas.
	For instance, these objects can be used to simulate ladders, teleporters, doors, jumping points, and destructible
	objects such as walls, windows and bridges.
	\pp This class cannot be used as-is; you must use one of the off-the-shelf PathObject classes supplied with the \SDKName
	SDK, or write your own custom class that derives from IPathObject. Typically, you will derive your own class from
	BasePathObject, which manages a finite state machine that simplifies taking control over the way the Bot traverses the PathObject.
	\pp For details, see :USERGUIDE:"Using PathObjects for Special Movements".
	\pp Deriving a custom class directly from IPathObject is recommended only in specific cases where you cannot use the automatic 
	Graph linking approach provided by the derived PathObject class, you know at generation time the locations of the
	Graph edges that each instance of your IPathObject should control, and those edges will not move dynamically at runtime.
	\pp To set up a class that derives directly from IPathObject, you must:
	-#	Write your class, and implement the virtual methods that make up the IPathObject path following interface: e.g. GoTo(),
		CanManageApproach(), CanLeave(), FindStartNode(), etc.
	-#	In your PathData generation process, you must create additional graphs that contain edges at all topological connections
		you want to be managed by your \IPathObject. For information on creating additional graphs and adding them to a PathData
		sector, see :USERGUIDE:"Using Custom Additional Graphs".
	-#	At runtime, after you load the PathData sector that contains your additional graph, you must retrieve from the
		GraphManager the AdditionalStitchedGraph that represents your additional graph. You can use the GraphManager::GetAdditionalStitchedGraph()
		method to retrieve the additional graph from a GUID that you set for it at generation time.
	-#	Create instances of your IPathObject class, and link them to the edges in the additional graph that they should manage
		by calling AdditionalStitchedGraph::SetPathObject(). 
	\ingroup kyruntime_pathobjects */
class IPathObject : public VirtualMemObject
{
	friend class PathObjectManager;

public:
	/************************************************************************/
	/*                                                                      */
	/* Constructors, Destructor, Initialization                             */
	/*                                                                      */
	/************************************************************************/

	/*!	\label_baseconstructor \label_mustinit Use this constructor only if you want to manage the IPathObjectInfo
		internally within your PathObject class. */
	IPathObject();

	/*!	\label_baseconstructor Calls Initialize() transparently. */
	IPathObject(IPathObjectInfo* info);

	/*!	\label_virtualdestructor.*/
	virtual ~IPathObject() {}

	/*!	\label_init
		Stores the specified IPathObjectInfo.
		\param info				An instance of a class that derives from IPathObjectInfo that specifies information about the PathObject,
								such as the Database this PathObject should be connected to. */
	void Initialize(IPathObjectInfo* info);



	/************************************************************************/
	/*                                                                      */
	/* Generic PathObject utilities                                         */
	/*                                                                      */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	// World
	///////////////////////////////////////////////////////

	/*! Adds the PathObject to the specified World. The PathObject is registered with the PathObjectManager
		in the World, and the Graph constructed internally by the PathObject to represent its topology is added to the
		Database indicated by IPathObjectInfo::GetDataBaseName(). */
	KyResult AttachToWorld(World* world);

	/*! Removes the PathObject from its current World. The Graph that represents the topology of the PathObject is
		unlinked from its Database. */
	KyResult DetachFromWorld();


	//////////////////////////////////////////////////////////////////////////
	// Info
	///////////////////////////////////////////////////////

	/*! Sets the IPathObjectInfo for this PathObject. */
	void SetInfo(IPathObjectInfo* info) { m_info = info; }


	//////////////////////////////////////////////////////////////////////////
	// Activation
	///////////////////////////////////////////////////////

	/*! Indicates whether or not this PathObject is currently active within its World. */
	bool IsActive() const { return m_isActive; }

	/*! Activates the PathObject within its World.
		\pp If you derive your PathObject class from BasePathObject, its activation
		status is used by BasePathObjectBehavior to determine whether or not the PathObject should be usable by \Bots in the World. When
		activated, \Bots will be able to plan paths that traverse the PathObject. You can disable the PathObject at any time through a
		call to Deactivate(). \PathObjects are activated by default. */
	void Activate() { m_isActive = true; }

	/*! Temporarily de-activates the PathObject within its World.
		\pp If you derive your PathObject class from BasePathObject, its activation
		status is used by BasePathObjectBehavior to determine whether or not the PathObject should be usable by \Bots in the World. When
		de-activated, \Bots will not plan paths that traverse the PathObject. You can re-enable the PathObject at any time through a call
		to Activate(). \PathObjects are activated by default. */
	void Deactivate() { m_isActive = false; }


	//////////////////////////////////////////////////////////////////////////
	// Status (last status update time)
	///////////////////////////////////////////////////////

	/*!	Updates the status timestamp of the PathObject.
		\pp This notifies all instances of the PathFinder that use the specified PathObject in their paths or
		that considered the specified PathObject in their last A* computations that their \Paths may need to be
		re-calculated.*/
	void UpdateStatus();

	/*!	Returns the status timestamp of the PathObject. */
	KyFloat32 GetLastStatusUpdateTime() { return m_lastUpdateTime; }


	//////////////////////////////////////////////////////////////////////////
	// Database associated to the PathObject
	///////////////////////////////////////////////////////

	/*! Retrieves the Database this PathObject is connected to. */
	Database* GetDatabase() { return m_database; }



	/************************************************************************/
	/*                                                                      */
	/* IPathObject interface                                                */
	/*                                                                      */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	// PathObject Manager
	///////////////////////////////////////////////////////

	/*! Called transparently when the PathObject is registered with a PathObjectManager. For internal use. */
	virtual KyResult OnRegisterToPathObjectManager(PathObjectManager& pathObjectManager);
	/*! Called transparently when the PathObject is unregistered from a PathObjectManager. For internal use. */
	virtual void OnUnregisterFromPathObjectManager();


	//////////////////////////////////////////////////////////////////////////
	// Behavior
	///////////////////////////////////////////////////////

	/*! Registers the specified Bot as a user of this PathObject. Called transparently. For internal use. */
	virtual void RegisterBot(Bot* entity);

	/*! Unregisters the specified Bot as a user of this PathObject. Called transparently. For internal use. */
	virtual void UnregisterBot(Bot* entity);


	//////////////////////////////////////////////////////////////////////////
	// Dump
	///////////////////////////////////////////////////////

	/*! Saves the current set of data maintained by this PathObject in the specified directory. The generated file can
		be dragged and dropped into the \SDKName Lab to visualize the data. */
	virtual KyResult Dump(const char* targetDirectory) const;


	//////////////////////////////////////////////////////////////////////////
	// Path-finding / Path-following interface
	///////////////////////////////////////////////////////

	/*!	Implemented by the BasePathObject class. Do not re-implement this method if you derive your custom class from BasePathObject.
		\pp This method is called during path calculations to determine the cost for a specified Bot to traverse an edge
		associated with this PathObject. This method is also called during path following, to evaluate the cost of following
		shortcuts from place to place.
		\pp The cost returned by this method is used instead of using the cost calculated by the current pathfinding IConstraint.
		Therefore, the default implementation of this method calls the IConstraint::GetCost() method of the IConstraint passed in
		the \c currentConstraint parameter, in order to take the IConstraint into account by default. If you choose to implement this
		method for your PathObject class, your implementation of this method should also take the IConstraint into account
		the same way, or should invoke the base implementation of IPathObject::GetCost().
		\pp If the state of your PathObject changes such that its GetCost() method begins returning a significantly different cost,
		it is typically necessary to update the edge status timestamp of the edges linked to the PathObject by calling the
		UpdateEdgeStatus() method of the PathObject for each linked edge. This announces to all instances of
		the PathFinder that the status of the edge has changed; if the PathFinder uses that edge in its current
		path or considered that edge during its last A* computation, it re-computes its path to ensure that the change to the cost
		is taken into account.
		\pp Note that the coordinates provided to this method as the start and end points of the edge may not actually match the
		coordinates of an edge linked to the PathObject. For example, the default implementation of the IGoto::GetCost() method for
		IGoto modifiers calls the IPathObject::GetCost() method of the current PathObject in use by the PathFinder in order
		to determine the cost of moving from the current position of the Bot to a specific destination. Your implementation of
		IPathObject::GetCost() should be able to handle cases such as this.
		\pp Your PathObject is expected to manage the use of its linked edges from beginning to end. Your implementation
		of this method must therefore test for the presence of other blocking obstacles at any point along the edge. This is particularly
		important when the edge controlled by your PathObject crosses the boundaries of the static NavMesh, such as a ladder or jumping
		point, as the areas outside the NavMesh cannot be handled by any dynamic obstacle detection methods built in to \SDKName. For
		example, if you have an edge that represents a jumping spot across a river, your PathObject can use the collision detection
		methods provided by the CanGo_NavMesh modifier or by the NavMesh to test for dynamic obstacles on the segments of the edge that
		lie within the static NavMesh. However, if a boat moving down the river might block the jumping edge at runtime, your PathObject
		must use a custom obstacle detection mechanism supplied by your \gameOrSim engine to detect the presence of the boat.
		\pp \param entity			The Bot currently evaluating the specified edge or line segment.
		\param from					The starting point of the edge or line segment to evaluate.
		\param to					The end point of the edge or line segment to evaluate.
		\param currentConstraint	The pathfinding IConstraint currently in use.
		\param[out] cost			Stores the cost calculated by this method.
		\return -	#KY_TRUE if the Bot can traverse the specified edge or line segment, and if the \c cost parameter has been
					successfully updated.
				-	#KY_FALSE if the Bot may not traverse the specified edge or line segment at all (for example,
					if a door is locked or a bridge has been destroyed) */
	virtual KyBool GetCost(Bot* entity, const PointWrapper& from, const PointWrapper& to,
		IConstraint& currentConstraint, KyFloat32& cost);

	/*!	Implemented by the BasePathObject class. Do not re-implement this method if you derive your custom class from BasePathObject.
		\pp The PathFinder uses this method to determine whether or not an edge linked to this PathObject can be bypassed
		during path following.
		\pp If this method returns #KY_TRUE, the Bot will be allowed to follow a shortcut that leads it directly from its current position
		to a PathNode farther along the Path, bypassing the linked edge.
		\pp If this method returns #KY_FALSE, the Bot will follow the linked edge exactly, from its starting vertex to its ending vertex.
		This has the following consequences:
		-	When using Local PathFinding, the A* algorithm will only consider following an edge linked to this PathObject if the starting
			point of that edge is considered reachable (because, if the starting point is unreachable, the Bot will not be able to follow
			the linked edge exactly). To be considered reachable, the starting point of the edge must lie outside of all
			known \LpfAreas: i.e. all \LpfAreas that are within the Local PathFinding radius. See :USERGUIDE:"The Local Pathfinding Process".
		-	During path following, if the PathFinder determines that the starting point of the linked edge has been covered or
			blocked by one or more \LpfAreas since the path was calculated, it deems the linked edge to be impassable. It then re-calculates
			the path from the current position of the Bot to its destination, without considering the linked edge in the A* calculation.
		-	Once the PathFinder has determined that the starting point of the linked edge is accessible, it does not test for the
			presence of any other \LpfAreas that intersect the linked edge. Since the PathObject has indicated that the Bot must follow
			its linked edge exactly, the PathFinder considers that it is the responsibility of the PathObject to take into account
			the passability and cost of any dynamic obstacles that intersect the edge. This ensures that a PathObject such as a teleporter,
			whose edge represents an abstract non-physical connection between two points, will not be blocked by the presence of \LpfAreas
			that intersect its linked edge. However, if you have a PathObject that returns #KY_FALSE from CanByPass() but that should take
			into account dynamic obstacles along its length (such as a very narrow bridge or corridor that can be blocked by \LpfAreas)
			it becomes the responsibility of the IPathObject::GetCost() and IPathObject::GoTo() methods to manage the presence of these dynamic obstacles.
		\pp The default implementation of this method returns #KY_TRUE in all cases.
		\param entity				The Bot using the PathObject.
		\param nodeId1				The starting vertex of the current edge. May be an invalid GraphVertexSafePtr(); see GoTo() for details.
		\param nodeId2				The ending vertex of the current edge. May be an invalid GraphVertexSafePtr(); see GoTo() for details.
		\return #KY_TRUE if the linked edge may be bypassed, or #KY_FALSE otherwise. */
	virtual KyBool CanByPass(Bot& entity, const GraphVertexSafePtr& nodeId1, const GraphVertexSafePtr& nodeId2);


	/*! Indicates to the PathFinder whether or not this PathObject is smoothable. For a detailed
		description of smoothable \PathObjects, see also :USERGUIDE:"PathObjects and Path Smoothing".
		\pp When this method returns #KY_TRUE to indicate that the PathObject is smoothable, you must also
		implement SmoothQuery() to define the conditions under which the PathObject permits path smoothing.
		\pp The default implementation of this method returns #KY_FALSE in all cases.
		\param Bot&					The Bot using the PathObject.
		\return #KY_TRUE if this PathObject is smoothable, or #KY_FALSE otherwise. */
	virtual KyBool CanSmooth(Bot& /*entity*/) { return KY_FALSE; }

	/*! When CanSmooth() indicates to the PathFinder that the Bot can follow a shortcut
		through the PathObject, the PathFinder calls this method during the path smoothing process each time it needs to 
		evaluate a candidate path node. 
		\pp The PathFinder sets up the query it passes to this method with a "from" point and a "to" point:
		-	The "from" point is typically set to the current position of the Bot.
		-	The "to" point is typically set to the position of the candidate path node.
		\pp This method is responsible for determining whether or not the PathObject should permit the Bot to pass from
		the "from" point to the "to" point. It must update the query to indicate its decision. If it determines that the
		Bot should be permitted to follow the proposed shortcut, it must also indicate in the query the entrance and exit
		points that the Bot should use to adopt and leave the PathObject: i.e. the entry point is the point at which the 
		PathFinder cedes control over the movement of the Bot to the PathObject, and the exit point is the point at which
		the PathObject ceases to move the Bot and returns control over to the Bot to the PathFinder.
		\pp This method only needs to consider passability in the segment between the entry and exit points. For the other
		segments of the proposed shortcut (before the entry point and after the exit point), the PathFinder is responsible
		for validating the shortcut.
		For a detailed description of smoothable \PathObjects, see also :USERGUIDE:"PathObjects and Path Smoothing".
		\param PathObjectSmoothQuery&		A structure that provides input values and stores output values. */
	virtual void SmoothQuery(PathObjectSmoothQuery& /*query*/) {}


	/*!	Implemented by the BasePathObject class. Do not re-implement this method if you derive your custom class from BasePathObject.
		\pp This function is called by the PathFinder when the Bot is currently targeting
		a vertex whose next edge (in the path) is linked to this PathObject. If this function
		returns #KY_TRUE, the PathFinder switches to the next vertex immediately, and consequently
		the Bot adopts the PathObject.
		\pp The default implementation returns #KY_TRUE when the distance to p1 is less than one meter.
		\param entity				The Bot using the PathObject.
		\param p1					The current path node. Contains a PointInfo_VertexSafePtr that identifies the Graph vertex.
		\param p2					The next path node. Contains a PointInfo_VertexSafePtr that identifies the Graph vertex.
		\return #KY_TRUE if the PathObject should take control over the Bot immediately. */
	virtual KyBool CanManageApproach(Bot& entity, const PointWrapper& p1, const PointWrapper& p2);

	/*!	Implemented by the BasePathObject class. Do not re-implement this method if you derive your custom class from BasePathObject.
		\pp The PathFinder calls this method when a Bot currently using the PathObject needs to calculate a new path, in order
		to determine whether or not a Bot can leave the PathObject immediately.
		\pp If this method returns #KY_FALSE, the Bot will not re-calculate its path. Instead, it will continue to use the current PathObject
		until the GoTo() method indicates that the Bot has finished using the PathObject.
		\pp If this method returns #KY_TRUE, the Bot will be allowed to calculate a new path, using the vertex determined by the
		FindStartNode() method of the PathObject as the starting point. It is recommended that you implement the FindStartNode() method
		any time your implementation of CanLeave() returns #KY_TRUE, to ensure that the re-computation starts from the best possible
		vertex (typically the one closest to the eventual destination point).
		\pp When the CanLeave() method allows an Bot to re-calculate its Path, the OnLeave() method of the PathObject is also called
		transparently. See OnLeave().
		\pp In most cases, the Bot will stop using the current PathObject when this method returns #KY_TRUE. However, as with the GoTo() method,
		the Bot may still re-adopt the PathObject as its current PathObject under certain circumstances. For example, if the Bot
		re-calculates its path while within an ObstacleLayer outline that represents the PathObject, the Bot will re-adopt the PathObject
		the following frame.
		\pp If an Bot re-computes its path while using your PathObject and re-adopts the PathObject as described above, then the PathFinder
		will pass an invalid GraphVertexSafePtr() to the \c nodeId1 parameter of the PathObject::GoTo() method in subsequent frames. This
		parameter is expected to identify the previous PathNode in the Path of the Bot, but due to the path re-calculation, the Bot is on its
		way to its first PathNode, so no previous PathNode exists to provide to the GoTo() method. Your implementation of the GoTo() method
		must take this possibility into account if it uses the vertices provided by the \c nodeId1 and \c nodeId2 parameters in its calculations.
		\pp The default implementation of this method returns #KY_TRUE in all cases.
		\param entity				The Bot using the PathObject.
		\param nodeId1				The vertex at the start of the linked edge.
		\param nodeId2				The vertex at the end of the linked edge.
		\return #KY_TRUE if the Bot may recompute its Path and leave the PathObject immediately, or #KY_FALSE if it must finish traversing the
				PathObject before it can recompute its Path. */
	virtual KyBool CanLeave(const Bot& entity, const GraphVertexSafePtr& nodeId1, const GraphVertexSafePtr& nodeId2);

	/*!	Implemented by the BasePathObject class. Do not re-implement this method if you derive your custom class from BasePathObject.
		\pp This method determines how an Bot traverses the PathObject. It can use either of the following approaches:
		-	It can determine a point to which the Bot must move, and pass this point to the PathFinder by updating the value
			of the \c targetPoint parameter. The PathFinder will take responsibility for moving the Bot to the specified point.
			In this case, the GoTo() method should return #KY_FALSE.
		-	It can take total control over the way the Bot crosses the edge by creating a new Action that represents the movements
			required, and saving that action in the \c action parameter. In this case, the GoTo() method should return #KY_TRUE. To create a
			new Action, you can compute it yourself from scratch, or you can invoke the ISteering::ComputeAction() method of the ISteering
			modifier currently in use by the PathFinder.
		\pp When the Bot has completely traversed the PathObject, this method should set the value of the \c quitPathObject parameter to
		#KY_TRUE. This indicates to the PathFinder that it should no longer consider this PathObject its current PathObject, and
		that it should no longer call the PathObject::GoTo() method in future frames. By default, when the Bot is released, it will
		continue to target the PathNode at the end of the edge linked to the PathObject. The PathFinder resumes the responsibility
		for moving the Bot to that PathNode, or skipping to another PathNode.
		\pp Each PathObject can use the \c skippedNodeCount parameter of the GoTo() method to control the PathNode that the Bot targets after
		quitting the PathObject. This argument indicates to the PathFinder how many nodes in the Path were bypassed while
		the GoTo() method of the PathObject was driving the Bot. For example, consider a Path through four \PathNodes, ABCD, where a single large
		PathObject is linked to both the A-B edge and the B-C edge. When the Bot first targets node B, it adopts the PathObject, whose
		GoTo() method moves the Bot past PathNode C (say by jumping), and sets \c quitPathObject to #KY_TRUE to release the Bot. By default,
		the current target PathNode of the Bot will still be B, which may cause the Bot to attempt to return to B, and to re-adopt the
		PathObject undesirably. In this case, the PathObject should set \c skippedNodeCount to 2, indicating to the PathFinder that
		it should skip two \PathNodes (node B and node C), and adopt node D as its current target PathNode.
		\pp Note that under certain circumstances, the Bot may still re-adopt the PathObject as its current PathObject. For example, if the
		GoTo() method sets \c quitPathObject to #KY_TRUE while the Bot is still within an ObstacleLayer outline that represents the PathObject,
		the Bot will re-adopt the PathObject the following frame.
		\pp The values of the \c nodeId1 and \c nodeId2 parameters identify the edge currently being followed by the Bot.
		-	If the PathFinder has adopted this PathObject as a result of the PathObject being linked to an edge that the Bot
			has included in its Path and that the Bot is currently attempting to follow, these arguments indicate, respectively, the
			vertex at the start of the edge currently being followed by the Bot, and at the end of the edge currently being
			followed by the Bot.
		-	If the PathFinder has adopted this PathObject as a result of the its trajectory passing within the boundaries of
			an ObstacleLayer outline that represents the PathObject, either of these GraphVertexSafePtr objects may contain invalid values under certain
			circumstances. For example, if the Bot adopts the PathObject when moving toward the first vertex contained in its
			Path, \c nodeId1 will be invalid. Similarly, if the Bot adopts the PathObject when moving to its final destination
			point, and that final destination point is not itself a vertex, the \c nodeId2 argument will be invalid.
			This is likely to happen if the Bot re-calculates its Path while using the PathObject. See also CanLeave().
		\pp The default implementation of this method returns #KY_FALSE, sets \c targetPoint to equal \c dest (thereby retaining the original target
		point of the PathFinder), and sets \c quitPathObject to #KY_TRUE when the Bot is closer than one meter to \c dest.
		\param bot						The Bot using the PathObject.
		\param dest						The current target point of the PathFinder.
		\param[out] action				An Action that will make the Bot use the PathObject. Read only when this method returns #KY_TRUE.
		\param[out] quitPathObject		Indicates whether or not the Bot has completed traversing this PathObject.
		\param[out] targetPoint			A new target point for the PathFinder. Read only when this method returns #KY_FALSE.
		\param nodeId1					The vertex at the start of the linked edge, or an invalid GraphVertexSafePtr.
		\param nodeId2					The vertex at the end of the linked edge, or an invalid GraphVertexSafePtr.
		\param[out] skippedNodeCount	The number of \PathNodes that were passed by the Bot while traversing the PathObject.
		\return #KY_TRUE if \c action has been updated to provide a specific set of movements for the Bot, or #KY_FALSE
				otherwise. */
	virtual KyBool GoTo(Bot& bot, const PointWrapper& dest, Action& action,
		KyBool& quitPathObject, PointWrapper& targetPoint, const GraphVertexSafePtr& nodeId1, const GraphVertexSafePtr& nodeId2,
		KyUInt32& skippedNodeCount);

	/*!	Implemented by the BasePathObject class. Do not re-implement this method if you derive your custom class from BasePathObject.
		\pp The PathFinder calls this method any time a Bot starts using the PathObject. The default implementation of this method
		is empty, but you can re-implement the method in your derived classes to perform any initialization tasks required to make your
		PathObject ready to manage the Bot.
		\param bot						The Bot using the PathObject.
		\param GraphVertexSafePtr&		The starting vertex of the edge that will be followed by the Bot.
		\param GraphVertexSafePtr&		The ending vertex of the edge that will be followed by the Bot. */
	virtual void OnEnter(Bot& bot, const GraphVertexSafePtr& /*entranceUid*/, const GraphVertexSafePtr& /*exitUid*/)
	{
		RegisterBot(&bot);
	}

	/*!	Implemented by the BasePathObject class. Do not re-implement this method if you derive your custom class from BasePathObject.
		\pp The PathFinder calls this method any time a Bot stops using the PathObject. The default implementation of this method
		is empty, but you can re-implement the method in your derived classes to perform any re-initialization tasks required to make your
		PathObject ready for the next Bot.
		\pp For example, \PathObjects that can only be used by one Bot at a time (such as a ladder, a narrow corridor, or a rope that an
		Bot must use to swing across a gap) typically keep track of whether or not they are currently in use. For such \PathObjects,
		you can implement this method to clear the "in use" flag when the current Bot stops using the PathObject for any reason.
		\param bot				The Bot using the PathObject. */
	virtual void OnLeave(Bot& bot)
	{
		UnregisterBot(&bot);
	}

		/*!	Implemented by the BasePathObject class. Do not re-implement this method if you derive your custom class from BasePathObject.
		\pp The PathFinder calls this method when the path of a Bot is recomputed while the Bot uses this PathObject.
		\param Bot&				The Bot using the PathObject. */
	virtual void OnPathRecomputed(Bot& /*bot*/, const GraphVertexSafePtr& /*entranceUid*/, const GraphVertexSafePtr& /*exitUid*/) {}

		/*!	Implemented by the BasePathObject class. Do not re-implement this method if you derive your custom class from BasePathObject.
		\pp The PathFinder calls this method to determines whether or not a Bot is using the PathObject as expected.
		\pp Implementing this method involves a combination of the return value and the \c accidentDetected parameter.
		\pp The return value indicates whether the PathObject::CheckAccident() method performs accident detection, or whether it defers
		accident detection to the IDetectAccident modifier currently in use by the PathFinder.
		\pp To defer accident detection to the IDetectAccident modifier currently in use by the PathFinder, the CheckAccident() method
		should return #KY_FALSE. In this case, the PathFinder does not take the value of the \c accidentDetected parameter into account.
		\pp To make the PathObject responsible for detecting accidents, CheckAccident() should return #KY_TRUE. In this case, the value
		of the \c accidentDetected parameter determines whether or not an accident has occurred.
		-	If an accident has occurred -- for example, if the force of an explosion blows an Bot off an elevator platform -- the
			CheckAccident() method should set the value of the \c accidentDetected parameter to #KY_TRUE. In this case, the PathFinder
			leaves the PathObject and recomputes a new Path for the Bot to get to its destination. This new Path is computed using the
			vertex determined by the PathObject::FindStartNode() method as its starting point.
		-	If an accident has not occurred, and the Bot is following the PathObject as expected, the CheckAccident() method should set
			the value of the \c accidentDetected parameter to #KY_FALSE.
		\pp The default implementation of this method returns #KY_FALSE in all cases.
		\param entity					The Bot using the PathObject.
		\param[out] accidentDetected	Indicates whether or not the Bot has encountered an accident while traversing the PathObject.
										Read only when the method returns #KY_TRUE.
		\return #KY_TRUE if the method checked for accidents and updated the \c accidentDetected parameter accordingly, or #KY_FALSE to defer
				accident detection to the IDetectAccident modifier in use by the PathFinder. */
	virtual KyBool CheckAccident(const Bot& entity, KyBool& accidentDetected);

		/*!	Implemented by the BasePathObject class. Do not re-implement this method if you derive your custom class from BasePathObject.
		\pp The PathFinder calls this method any time a Bot re-calculates its path while traversing the PathObject,
		in order to determine the vertex that should be used as the starting point for the path re-computation. This method is intended
		primarily for \PathObjects that take the Bot to a position from which it will not be able to access any nearby vertices
		directly, such as a ladder or a jumping point that takes control of the Bot and moves it outside of the NavMesh. If an
		accident occurs while the Bot is using a PathObject that takes it outside the boundaries of the NavMesh, the Bot may
		not be able to find a starting vertex for its path recalculation by itself. In this case, you can use this method to
		provide the PathFinder with a vertex that you consider to be reachable by the Bot at its current location.
		\pp When this method successfully calculates a starting vertex, this PathObject is retained as the current PathObject for the
		PathFinder. Therefore, the PathObject::GoTo() method will still be called at each frame to direct the Bot along
		the PathObject. In addition, the vertex returned by this method is adopted by the PathFinder as its current PathNode
		after the re-calculation.
		\pp When the PathFinder calls this method, it provides in the \c dest parameter the final destination point requested during
		the path re-computation. Your implementation of FindStartNode() can use this destination point in order to select the best starting
		vertex for the re-calculation.
		\pp Typically, an implementation of FindStartNode() returns either the starting or ending vertex of the current linked edge,
		whichever is reachable from the \c dest point without requiring the Bot to backtrack all the way through the PathObject again.
		For example, if your Bot is using a ladder to move between two floors, and a path re-calculation is requested to a new destination on the
		upper floor, the FindStartNode() method should typically return the vertex at the upper end of the ladder. That way, the vertex at the top
		of the ladder becomes the target point passed to the PathObject::GoTo() method in subsequent frames, and the Bot will climb
		to the top (regardless of whether it had previously been climbing or descending the ladder). If FindStartNode() returns the vertex
		at the bottom end of the ladder, but the destination is on the upper floor, the path of the Bot will lead it first down to the bottom
		of the ladder, then, when that PathNode is reached, it will climb back up the ladder again. In this case, a heuristic test that
		evaluates the altitude of the destination point with respect to the altitude of the start and end vertices of the edge controlled
		by the ladder would help determine the better candidate.
		\pp If this method returns #KY_FALSE, the PathFinder does not take its calculations into account at all. Instead, the PathFinder
		will attempt to find a starting vertex for the next path computation by calling the IFindNodesFromPositions modifier as usual.
		\pp If this method returns #KY_TRUE, the PathFinder continues to call FindStartNode() each frame until the method indicates
		that it has finished its calculations by setting the value of its \c computationIsOver parameter to #KY_TRUE. At this point, the
		PathFinder retrieves the GraphVertexSafePtr stored in the \c bestNodeUid parameter, and uses this vertex as the starting vertex
		for the new path. Note that in this case the IFindNodesFromPositions modifier is never called to determine the starting vertex for
		the next path computation. Therefore, if you store an invalid GraphVertexSafePtr in the \c bestNodeUid parameter, the PathFinder 
		will not be able to calculate a path at all. It will issue a warning to the log, and your Bot will freeze.
		\pp The default implementation of this method simply returns #KY_FALSE.
		\param entity					The Bot using the PathObject.
		\param start					The starting position for the path recalculation: typically, the current position of the Bot.
										If a PointInfo_VertexSafePtr object is attached to this PointWrapper, indicating that the Bot is at the
										position of a vertex already, you can set \c bestNodeUid to the value stored in that
										PointInfo_VertexSafePtr object (as long as it is not #KY_NULL).
		\param dest						The destination point for the path recalculation.
		\param[out] bestNodeUid			Stores the nearest vertex computed by this method.
		\return #PathObject_FindStartNode_UnderProcess if the computation is not complete,
				#PathObject_FindStartNode_NodeFound if the computation is done and the PathObject proposes a node to start the new path from, or
				#PathObject_FindStartNode_NodeNotFound if the computation is done and the PathObject lets the PathFinder find the node itself using the IFindNodesFromPositions modifier. */
	virtual PathObject_FindStartNode_Status FindStartNode(Bot* entity, const PointWrapper& start, const PointWrapper& dest, GraphVertexSafePtr& bestNodeUid);

	/*!	For internal use. */
	virtual KyResult UpdatePointInfos(const Vec3f& /*position*/, PointInfo_ConnexionSpace* /*connexionSpacePI*/, PointInfo_ControlSpace* /*controlSpacePI*/) { return KY_SUCCESS; }


	//////////////////////////////////////////////////////////////////////////
	// Misc
	///////////////////////////////////////////////////////

	/*!	Renders debugging information about the PathObject.
		\param renderFlags		A bitmask built up of elements from the #PathObjectRenderFlags enumeration that indicates
								the data that should be rendered. */
	virtual void Render(PathObjectRenderMasks renderFlags =
			PathObjectRender_InternalGraph +
			PathObjectRender_ConnexionLinks +
			PathObjectRender_ConnexionSpaces +
			PathObjectRender_ControlSpaces +
			PathObjectRender_Obstacles +
			PathObjectRender_ControledBots
		) const;

	/*!	Each time the DatabaseManager streams a Graph into memory, it calls this method automatically. You can
		implement this method in your custom PathObject class if you need to perform any specific operations each time a
		new Graph is streamed.*/
	virtual void OnAddGraph(const Graph* graph);


public:
	KyUInt32 m_uid;                               /*!< A unique ID for this PathObject. Set internally by the PathObjectManager. Do not modify. */
	IPathObjectInfo* m_info;                      /*!< The IPathObjectInfo set for this PathObject. Do not modify directly. */
	SharedPoolList<Bot*> m_registeredBots;        /*!< The list of \Bots traversing this PathObject. Do not modify. */

protected:
	World* m_world;                               /*!< The World this PathObject belongs to,if any. Do not modify. */
	Database* m_database;                         /*!< The Database this PathObject is connected to, if any. Do not modify. */
	PathObjectManager* m_pathObjectManager;       /*!< The PathObjectManager for the World. Do not modify. */
	GraphManager* m_graphManager;                 /*!< The GraphManager maintained by #m_database. */

public:
	AdditionalStitchedGraph* m_additionalStitchedGraph;               /*!< For internal use. Do not modify. */

protected:
	bool m_isActive;                              /*!< Stores whether or not the PathObject is currently activated in the World. */

	KyFloat32 m_lastUpdateTime;                   /*!	The last time the status of this PathObject changed. See UpdateStatus(). */

protected:
	void Init();
};


} // namespace Kaim

#endif // KY_IPATHOBJECT_H
