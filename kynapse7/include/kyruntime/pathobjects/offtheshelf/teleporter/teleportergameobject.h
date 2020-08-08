/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathobjects */

// primary contact: MAMU - secondary contact: None
#ifndef KYRUNTIME_TELEPORTER_GAMEOBJECT_H
#define KYRUNTIME_TELEPORTER_GAMEOBJECT_H

#include <kypathdata/graph/graphtypes.h>
#include <kyruntime/pathobjects/offtheshelf/teleporter/teleporterpathobject.h>
#include <kyruntime/pathobjects/offtheshelf/teleporter/teleporterpathobjectbehavior.h>
#include <kypathdata/pathobject/offtheshelf/teleporter/teleporterpathobjecttopology.h>
#include <kypathdata/pathobject/info/dynamicpathobjectinfo.h>

namespace Kaim
{

/*!	The TeleporterGameObject class provides an interface for using an off-the-shelf PathObject that simulates
	the behavior of a teleporter.
	\section teleporterposetup Setting up a teleporter
	\subsection requirements Requirements
	-	Any Bot using the teleporter must have the ActionForcePosition class of ActionAttribute assigned to its Action.
	\subsection teleporterpoadd Adding a teleporter to the World
	To set up a teleporter in your World:
	-#	Create a new instance of the TeleporterGameObject in your \gameOrSim engine. In the constructor, you will provide
		a variety of information about the location and behavior of the teleporter.
	-#	Call AddToKynapse() in order to add the teleporter to the specified World. The teleporter will be stitched in to the
		Graph in the Database that you specified in the constructor.
	-#	When you want to remove the teleporter from the World, call RemoveFromKynapse().
	\section teleporterpousing Using a teleporter
	Using a teleporter PathObject involves the following considerations
	\subsection teleporterpolocking Activating and de-activating the teleporter
	A teleporter PathObject can be either activated or de-activated.
	-	When activated, \Bots will be able to plan paths that use the teleporter.
	-	When de-activated, \Bots will not be able to plan paths that use the teleporter.
	\pp You can activate and de-activate the teleporter by calling Activate() and Deactivate(). The teleporter is activated by default. 
	\subsection doorpoqueue Multiple Bots
	Only one Bot can traverse a teleporter at any given time. Additional \Bots that try to use the teleporter while it is already
	in use will be rejected.
	\ingroup kyruntime_pathobjects */
class TeleporterGameObject
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor 
		\param databaseName			The name of the Database to which the PathObject will be connected. Only \Bots using this Database will be able
									to take the PathObject into account when planning and following paths.
		\param entrance				The position of the teleporter entrance. Should be on or close to the ground. 
									Must be within the boundaries of the NavMesh.
		\param exit					The position of the teleporter exit. Should be on or close to the ground. 
									Must be within the boundaries of the NavMesh.
		\param entityHeight			The height of the characters that use the Database specified by \c databaseName.
		\param halfWidth			Half the width of the characters that use the Database specified by \c databaseName.
		\param bidirectional		Indicates whether or not the teleporter can be traversed in both directions. If \c true, \Bots will be able to pass from
									the entrance to the exit, and from the exit to the entrance. If \c false, \Bots will only be able to pass from the
									entrance to the exit.
		\param poId					A unique ID for the PathObject. Used only to create a file name if you dump the PathObject to a file on disk. */
	TeleporterGameObject(
		const char* databaseName,
		Vec3f entrance,
		Vec3f exit,
		KyFloat32 entityHeight,
		KyFloat32 halfWidth,
		bool bidirectional,
		PathObjectId poId);

	/*! \label_destructor Removes the PathObject from its World, and deletes its topology, info and behavior. */
	~TeleporterGameObject();

public:
	/*! Adds the PathObject to the specified World. */
	void AddToKynapse(World& world) { if (m_pathObject) m_pathObject->AttachToWorld(&world); }

	/*! Removes the PathObject from its World, but does not delete its topology, info and behavior. It can be
		re-added at any time. */
	void RemoveFromKynapse() { if (m_pathObject) m_pathObject->DetachFromWorld(); }

	/*! Renders the PathObject.*/
	void Render()
	{
		static const KyUInt32 flags = 0
			+ Kaim::PathObjectRender_ConnexionSpaces
			+ Kaim::PathObjectRender_ControlSpaces
			+ Kaim::PathObjectRender_ConnexionLinks
			+ Kaim::PathObjectRender_ControledBots
			+ Kaim::PathObjectRender_InternalGraph
			+ Kaim::PathObjectRender_Obstacles
			;

		if (m_pathObject != KY_NULL)
			m_pathObject->Render(flags);
	}

	/* LOCK */
public:
	/*! Activates the teleporter, allowing \Bots to use it in their paths. */
	void Activate() { m_behavior->Activate(); }

	/*! Deactivates the teleporter, preventing \Bots from using it in their paths. */
	void Deactivate() { m_behavior->Deactivate(); }

protected:
	TeleporterPathObject* m_pathObject;
	TeleporterPathObjectTopology* m_topology;
	TeleporterPathObjectBehavior* m_behavior;
	DynamicPathObjectInfo* m_info;
};

} // namespace Kaim

#endif // KYRUNTIME_TELEPORTER_GAMEOBJECT_H
