/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathobjects */

// primary contact: LAPA - secondary contact: MAMU
#ifndef KY_PATHOBJECT_H
#define KY_PATHOBJECT_H

#include <kyruntime/pathobjects/ipathobject.h>

#include <kyruntime/graph/graphmanager.h>
#include <kyruntime/core/bot.h>
#include <kyruntime/pathobjects/ipathobjectbehavior.h>

#include <kypathdata/pathobject/ipathobjectconnexionspace.h>
#include <kypathdata/pathobject/ipathobjectconnexionlink.h>
#include <kypathdata/pathobject/ipathobjectcontrolspace.h>
#include <kypathdata/pathobject/ipathobjecttopology.h>

#include <kypathdata/containers/list.h>
#include <kypathdata/blob/blobhandler.h>



namespace Kaim
{

class DatabaseManager;
class NavMeshLayerManager;
class PathObjectManager;
class RuntimePathObjectTopology;
class RuntimePathObjectConnexionSpace;
class RuntimePathObjectControlSpace;
class RuntimePathObjectConnexionLink;
class StitchedGraphCell;
class PointInfo_ControlSpace;
class PointInfo_ConnexionSpace;
class GraphVertexSafePtr;



/*!	PathObject is an abstract class that derives from the base IPathObject interface, and adds the ability
	to stitch the PathObject to the surrounding PathData through a topology made up of connection spaces,
	connection links and control spaces.
	\pp This class cannot be used as-is; you must use one of the off-the-shelf
	PathObject classes supplied with the \SDKName SDK, or write your own custom
	class that derives from PathObject. Typically, you will derive your own
	class from BasePathObject, which also manages a finite state machine that simplifies
	taking control over the Bot using the PathObject. 
	\pp For details, see :USERGUIDE:"Using PathObjects for Special Movements".
	\pp Deriving a custom class directly from PathObject is recommended only in specific cases where you need to control
	the behavior of your \Bots in a way that cannot be done using the finite state machine and behavior interface provided
	by the BasePathObject class.
	\pp To set up a class that derives directly from PathObject, you must:
	-	Write your class, and implement the virtual methods that make up the IPathObject path following interface: e.g. GoTo(),
		CanManageApproach(), CanLeave(), FindStartNode(), etc.
	-	Create set up the topology that defines the way your PathObject links to the surrounding PathData, and the volumes
		within which it will take responsibility for moving your Bot. For more information, see :USERGUIDE:"The PathObject Framework" and 
		:USERGUIDE:"Setting Up a Custom BasePathObject".
	-	At runtime, create and initialize an instance of your PathObject class. You will need to provide an instance of a class that implements
		IPathObjectInfo, which defines some basic information about your PathObject, and you will need to provide your topology. 
		Your PathObject will be linked automatically to the Graph, and your \Bots will begin taking it into account in their
		path calculations and path following.
		\ingroup kyruntime_pathobjects */
class PathObject : public IPathObject
{
	friend class PathObjectManager;

public:
	/************************************************************************/
	/*                                                                      */
	/* Constructors, Destructor, Initialization                             */
	/*                                                                      */
	/************************************************************************/

	/*!	\label_baseconstructor \label_mustinit Use this constructor only if you want to manage the IPathObjectInfo
		and IPathObjectTopology internally within your PathObject class. */
	PathObject();

	/*!	\label_baseconstructor Calls Initialize() transparently. */
	PathObject(IPathObjectInfo* info, IPathObjectTopology* topology);

	/*!	\label_virtualdestructor.*/
	virtual ~PathObject();

	/*!	\label_init
		Stores the specified IPathObjectInfo and IPathObjectTopology, and creates a Graph that corresponds to the topology.
		\param info				An instance of a class that derives from IPathObjectInfo that specifies information about the PathObject,
								such as the Database this PathObject should be connected to.
		\param topology			An instance of a class that derives from IPathObjectTopology that specifies the connection spaces at
								which the PathObject should be connected to the static \Graphs in the Database, the connections between
								those connection spaces, and the control spaces within which the PathObject controls the movement of its \Bots. */
	void Initialize(IPathObjectInfo* info, IPathObjectTopology* topology);



	/************************************************************************/
	/*                                                                      */
	/* IPathObject interface                                                */
	/*                                                                      */
	/************************************************************************/

	// PathObject Manager
	virtual KyResult OnRegisterToPathObjectManager(PathObjectManager& pathObjectManager);
	virtual void OnUnregisterFromPathObjectManager();

	// Behavior
	virtual void RegisterBot(Bot* entity);
	virtual void UnregisterBot(Bot* entity);

	// Dump
	virtual KyResult Dump(const char* targetDirectory) const;

	// Path-finding / Path-following interface
	virtual KyBool GetCost(Bot* entity, const PointWrapper& from, const PointWrapper& to,
		IConstraint& currentConstraint, KyFloat32& cost);
	virtual KyBool CanByPass(Bot& entity, const GraphVertexSafePtr& nodeId1, const GraphVertexSafePtr& nodeId2);
	virtual KyBool CanManageApproach(Bot& entity, const PointWrapper& p1, const PointWrapper& p2);
	virtual KyBool CanLeave(const Bot& entity, const GraphVertexSafePtr& nodeId1, const GraphVertexSafePtr& nodeId2);
	virtual KyBool GoTo(Bot& bot, const PointWrapper& dest, Action& action, 
		KyBool& quitPathObject, PointWrapper& targetPoint, const GraphVertexSafePtr& nodeId1, const GraphVertexSafePtr& nodeId2, 
		KyUInt32& skippedNodeCount);

	/* The following virtual methods are not reimplemented in PathObject class.
	   We let them here commented to not forgot them, namely when deriving this class. */

	//virtual void OnEnter(Bot& bot, const GraphVertexSafePtr& entranceUid, const GraphVertexSafePtr& exitUid)
	//virtual void OnLeave(Bot& bot)
	//virtual void OnPathRecomputed(Bot& bot, const GraphVertexSafePtr& entranceUid, const GraphVertexSafePtr& exitUid)

	virtual KyBool CheckAccident(const Bot& entity, KyBool& accidentDetected);
	virtual PathObject_FindStartNode_Status FindStartNode(Bot* entity, const PointWrapper& start, const PointWrapper& dest, GraphVertexSafePtr& bestNodeUid);
	virtual KyResult UpdatePointInfos(const Vec3f& position, PointInfo_ConnexionSpace* connexionSpacePI, PointInfo_ControlSpace* controlSpacePI);

	// Misc
	virtual void Render(PathObjectRenderMasks renderFlags) const;
	virtual void OnAddGraph(const Graph* graph);



	/************************************************************************/
	/*                                                                      */
	/* PathObject-specific methods                                          */
	/*                                                                      */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////////////////////
	// Update
	//////////////////////////////////////////////////////////////////////////////////////////

	/*! Call this method to indicate that the PathObject has begun to move. The control space of the PathObject is removed
		from its PathObjectLayer, and the \Bots using this PathObject are notified that the PathObject has changed. */
	void StartMoving();

	/*! Call this method to indicate that the PathObject has stopped moving. The control space of the PathObject is re-projected
		into its PathObjectLayer if necessary, and the \Bots using this PathObject are notified that the PathObject has changed. */
	void EndMoving();


	//////////////////////////////////////////////////////////////////////////////////////////
	// Topology
	//////////////////////////////////////////////////////////////////////////////////////////

	/*! Sets the IPathObjectTopology for this PathObject. */
	void SetTopology(IPathObjectTopology* topology);

	/*! Retrieves the IPathObjectTopology set for this PathObject. */
	RuntimePathObjectTopology* GetTopology() { return m_topology; }

	/*! Retrieves the number of connection links between the internal Graph that represents the topology of the
		PathObject and the surrounding static Graphs. */
	KyUInt32 GetConnexionLinkCount() const;

	/*! Retrieves the connection link with the specified index. */
	RuntimePathObjectConnexionLink* GetConnexionLink(KyUInt32 idx) const;

	/*! Retrieves the number of connection spaces set for this PathObject. */
	KyUInt32 GetConnexionSpaceCount() const;

	/*! Retrieves the connection space with the specified index. */
	RuntimePathObjectConnexionSpace* GetConnexionSpace(KyUInt32 idx) const;

	/*! Retrieves the connection space that corresponds to the specified vertex. */
	RuntimePathObjectConnexionSpace* GetConnexionSpaceFromVertexSafePtr(const GraphVertexSafePtr& vertexSafePtr) const;

	/*! Retrieves the number of control spaces set for this PathObject. */
	KyUInt32 GetControlSpaceCount() const;

	/*! Retrieves the control space with the specified index. */
	RuntimePathObjectControlSpace* GetControlSpace(KyUInt32 idx) const;


	//////////////////////////////////////////////////////////////////////////////////////////
	// Behavior
	//////////////////////////////////////////////////////////////////////////////////////////

	/*! Retrieves the IPathObjectBehavior used by this PathObject, if any. */
	virtual IPathObjectBehavior* GetBehavior() { return KY_NULL; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Graph associated to the PathObject
	//////////////////////////////////////////////////////////////////////////////////////////

	/*! Called transparently when the PathObject is registered. For internal use. */
	KyResult LinkToGraph();

	/*! Called transparently when the PathObject is unregistered. For internal use. */
	KyResult UnlinkFromGraph();

	/*! Called transparently when the PathObject is registered. For internal use. */
	void CreateGraph();

	/*! Called transparently when the PathObject is unregistered. For internal use. */
	void DestroyGraph();

	/*! Retrieves the Graph constructed internally to represent the topology of the PathObject. */
	const AdditionalGraph* GetGraph () const;


	//////////////////////////////////////////////////////////////////////////////////////////
	// Locked edges
	//////////////////////////////////////////////////////////////////////////////////////////

	/*! Called transparently when the PathObject is registered. For internal use. */
	KyResult LockEdgesInControlSpaces();

	/*! Called transparently when the PathObject is unregistered. For internal use. */
	void UnlockEdgesInControlSpaces();


	//////////////////////////////////////////////////////////////////////////////////////////
	// ObstacleProjectionVolume management
	//////////////////////////////////////////////////////////////////////////////////////////

	/*! Called transparently when the PathObject is registered. For internal use. */
	KyResult AttachToLayerManager();

	/*! Called transparently when the PathObject is unregistered. For internal use. */
	KyResult DetachFromLayerManager();

	/*! Called transparently when the PathObject is registered. For internal use. */
	KyResult RegisterProjectionVolumesToLayerManager();

	/*! Called transparently when the PathObject is unregistered. For internal use. */
	KyResult UnregisterProjectionVolumesFromLayerManager();


	void SetUserData(void* userData) { m_userData = userData; }
	void* GetUserData() const { return m_userData; }

protected:
	RuntimePathObjectTopology* m_topology;        /*!< A RuntimePathObjectTopology object created by this PathObject from the IPathObjectTopology set for it. Do not modify. */
	NavMeshLayerManager* m_layerManager;          /*!< The NavMeshLayerManager for the World. Do not modify. */

public:
	const AdditionalGraph* m_additionalGraph;                         /*!< A Graph created internally by the PathObject to represent its topology. This Graph is stitched automatically to the other \Graphs in the Database configured for this PathObject. Do not modify. */
	BlobHandler<AdditionalGraph> m_additionalGraphHandler;            /*!< For internal use. Do not modify. */

	bool m_isMoving;                              /*!< Stores whether or not the PathObject is currently moving. */

private:
	void* m_userData;

private:
	void Init();
};


} // namespace Kaim

#endif // KY_PATHOBJECT_H
