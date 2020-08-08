/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathobjects */

// primary contact: MAMU - secondary contact: None
#ifndef KYRUNTIME_DOOR_GAMEOBJECT_H
#define KYRUNTIME_DOOR_GAMEOBJECT_H

#include <kypathdata/graph/graphtypes.h>
#include <kyruntime/pathobjects/offtheshelf/door/doorpathobject.h>
#include <kyruntime/pathobjects/offtheshelf/door/doorpathobjectbehavior.h>
#include <kypathdata/pathobject/offtheshelf/door/doorpathobjectinfo.h>
#include <kypathdata/pathobject/offtheshelf/door/doorpathobjecttopology.h>

namespace Kaim
{

/*!	The DoorGameObject class provides an interface for using an off-the-shelf PathObject that simulates
	the behavior of a typical door.
	\section doorposetup Setting up a door
	\subsection requirements Requirements
	-	Any Bot using the door must have the ActionForcePosition class of ActionAttribute assigned to its Action.
	\subsection doorpoadd Adding a door to the World
	To set up a door in your World:
	-#	Create a new instance of the DoorGameObject in your \gameOrSim engine. In the constructor, you will provide
		a variety of information about the location and behavior of the door.
	-#	Call AddToKynapse() in order to add the door to the specified World. The door will be stitched in to the
		Graph in the Database that you specified in the constructor.
	-#	When you want to remove the door from the World, call RemoveFromKynapse().
	\section doorpousing Using a door
	Using a door PathObject involves the following considerations
	\subsection doorpolocking Locking and unlocking the door
	A door PathObject can be either locked or unlocked.
	-	When unlocked, \Bots will be able to plan paths that use the door.
	-	When locked, \Bots will not be able to plan paths that use the door.
	\pp You can lock and unlock the door by calling LockDoor() and UnlockDoor(). The door is unlocked by default. 
	\subsection doorpoqueue Multiple Bots
	Only one Bot can traverse a door at any given time. Additional \Bots that try to use the door while it is already
	in use will be rejected.
	\ingroup kyruntime_pathobjects */
class DoorGameObject
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor 
		\param poId					A unique ID for the PathObject. Used only to create a file name if you dump the PathObject to a file on disk.
		\param databaseName			The name of the Database to which the PathObject will be connected. Only \Bots using this Database will be able
									to take the PathObject into account when planning and following paths.
		\param averageEntityHeight	The height of the characters that use the Database specified by \c databaseName.
		\param entrance				A position on the entrance side of the door, on the ground, in approximately the middle of the doorframe. 
									Must be within the boundaries of the NavMesh.
		\param exit					A position on the exit side of the door, on the ground in approximately the middle of the doorframe.
									Must be within the boundaries of the NavMesh.
		\param height				The height of the doorframe.
		\param halfWidth			Half the width of the doorframe.
		\param bidirectional		Indicates whether or not the door can be traversed in both directions. If \c true, \Bots will be able to pass from
									the entrance to the exit, and from the exit to the entrance. If \c false, \Bots will only be able to pass from the
									entrance to the exit. */
	DoorGameObject(
		PathObjectId poId,
		const char* databaseName,
		KyFloat32 averageEntityHeight,
		Vec3f entrance,
		Vec3f exit,
		KyFloat32 height,
		KyFloat32 halfWidth,
		bool bidirectional);

	/*! \label_destructor Removes the PathObject from its World, and deletes its topology, info and behavior. */
	~DoorGameObject();

public:
	/*! Adds the PathObject to the specified World.*/
	void AddToKynapse(World& world) { if (m_pathObject) m_pathObject->AttachToWorld(&world); }

	/*! Removes the PathObject from its World, but does not delete its topology, info and behavior. It can be
		re-added at any time. */
	void RemoveFromKynapse() { if (m_pathObject) m_pathObject->DetachFromWorld(); }

	/*! Renders the PathObject. */
	void Render()
	{
		static const KyUInt32 flags = 0
			+ Kaim::PathObjectRender_ConnexionSpaces
			+ Kaim::PathObjectRender_ControlSpaces
			//+ Kaim::PathObjectRender_ConnexionLinks
			+ Kaim::PathObjectRender_ControledBots
			//+ Kaim::PathObjectRender_InternalGraph
			+ Kaim::PathObjectRender_Obstacles
			;

		if (m_pathObject != KY_NULL)
			m_pathObject->Render(flags);
	}

	/* LOCK */
public:
	/*! Locks the door, preventing \Bots from using it in their paths. */
	void LockDoor() { m_pathObject->Deactivate(); }

	/*! Unlocks the door, allowing \Bots to use it in their paths. */
	void UnlockDoor() { m_pathObject->Activate(); }

protected:
	DoorPathObject* m_pathObject;
	DoorPathObjectTopology* m_topology;
	DoorPathObjectInfo* m_info;
	DoorPathObjectBehavior* m_behavior;
};

} // namespace Kaim

#endif // KYRUNTIME_DOOR_PATHOBJECT_H
