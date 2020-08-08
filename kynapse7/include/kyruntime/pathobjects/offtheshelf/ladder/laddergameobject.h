/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathobjects */

// primary contact: MAMU - secondary contact: None
#ifndef KYRUNTIME_LADDER_GAMEOBJECT_H
#define KYRUNTIME_LADDER_GAMEOBJECT_H

#include <kypathdata/graph/graphtypes.h>
#include <kyruntime/pathobjects/offtheshelf/ladder/ladderpathobject.h>
#include <kyruntime/pathobjects/offtheshelf/ladder/ladderpathobjectbehavior.h>
#include <kypathdata/pathobject/offtheshelf/ladder/ladderpathobjecttopology.h>
#include <kypathdata/pathobject/offtheshelf/ladder/ladderpathobjectinfo.h>

namespace Kaim
{

/*!	The LadderGameObject class provides an interface for using an off-the-shelf PathObject that simulates
	a connection between two points that can only be crossed by climbing or descending a ladder.
	\section ladderposetup Setting up a ladder
	\subsection requirements Requirements
	-	Any Bot using the ladder must have the ActionForcePosition class of 
		ActionAttribute assigned to its Action.
	\subsection ladderpoadd Adding a ladder to the World
	To set up a ladder in your World:
	-#	Create a new instance of the LadderGameObject in your \gameOrSim engine. In the constructor, you will provide
		a variety of information about the location and behavior of the ladder.
	-#	Call AddToKynapse() in order to add the ladder to the specified World. The ladder will be stitched in to the
		Graph in the Database that you specified in the constructor.
	-#	When you want to remove the ladder from the World, call RemoveFromKynapse().
	\section ladderpousing Using a ladder
	Using a ladder PathObject involves the following considerations
	\subsection ladderpoqueue Multiple Bots
	Only one Bot can traverse a ladder at any given time. Additional \Bots that try to use the ladder while it is already
	in use will be rejected.
	\ingroup kyruntime_pathobjects */
class LadderGameObject
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor
		\param databaseName		The name of the Database to which the PathObject will be connected. Only \Bots using this Database will be able
								to take the PathObject into account when planning and following paths.
		\param entrance			The position at which a Bot should start using the ladder. Should be on or close to the ground. Must be within
								the boundaries of the NavMesh.
		\param exit				The position at which a Bot should stop using the ladder. Should be on or close to the ground. Must be within
								the boundaries of the NavMesh.
		\param entityHeight		The height of the characters that use the Database specified by \c databaseName.
		\param halfWidth		Half the width of the characters that use the Database specified by \c databaseName.
		\param ladderBottom		The position at which the ladder reaches the bottom floor, used to compute the position at which the Bot begins ascending.
								Should be on or close to the ground, but not necessarily within the boundaries of the NavMesh.
		\param ladderTop		The position at which the ladder reaches the top floor, used to compute the position at which the Bot stops ascending.
								Should be on or close to the ground, but not necessarily within the boundaries of the NavMesh.
		\param ladderBack		The facing orientation of a character while it is climbing the ladder.
		\param ladderRadius		The radius of the ladder. This is used to define a cylindrical control space along the length of the ladder, 
								around the line segment from \c ladderBottom to \c ladderTop.
		\param poId				A unique ID for the PathObject. Used only to create a file name if you dump the PathObject to a file on disk. */
	LadderGameObject(
		const char* databaseName,
		const Vec3f& entrance,
		const Vec3f& exit,
		KyFloat32 entityHeight,
		KyFloat32 halfWidth,
		const Vec3f& ladderBottom,
		const Vec3f& ladderTop,
		const Vec3f& ladderBack,
		KyFloat32 ladderRadius,
		PathObjectId poId);

	/*! \label_destructor Removes the PathObject from its World, and deletes its topology, info and behavior. */
	~LadderGameObject();

public:
	/*!	Projects the specified position into the ladder. For internal use. */
	void ComputeProjectionOnLadder(const Vec3f& position, Vec3f& out_ProjectionOnLadder);

	/*! Computes the position at which a character at the specified starting position can grab the ladder. For internal use. */
	void ComputeGrabPosition(const Vec3f& entityPosition, Vec3f& out_GrabPosition);

public:
	Vec3f m_ladderBottom; /*!< Stores the \c ladderBottom value set in the constructor. */
	Vec3f m_ladderTop; /*!< Stores the \c ladderTop value set in the constructor. */
	Vec3f m_ladderBack; /*!< Stores the \c ladderBack value set in the constructor. */
	KyFloat32 m_ladderRadius; /*!< Stores the \c ladderRadius value set in the constructor. */

//////////////////////////
// KYNAPSE
//////////////////////////

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

protected:
	LadderPathObject* m_pathObject;
	LadderPathObjectTopology* m_topology;
	LadderPathObjectBehavior* m_behavior;
	LadderPathObjectInfo* m_info;
	PathObjectQueue* m_ladderQueue;
};

} // namespace Kaim

#endif // KYRUNTIME_LADDER_GAMEOBJECT_H
