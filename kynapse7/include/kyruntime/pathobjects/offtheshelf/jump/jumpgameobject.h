/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathobjects */

// primary contact: MAMU - secondary contact: None
#ifndef KYRUNTIME_JUMP_GAMEOBJECT_H
#define KYRUNTIME_JUMP_GAMEOBJECT_H

#include <kypathdata/graph/graphtypes.h>
#include <kyruntime/pathobjects/offtheshelf/jump/jumppathobject.h>
#include <kyruntime/pathobjects/offtheshelf/jump/jumppathobjectbehavior.h>
#include <kypathdata/pathobject/offtheshelf/jump/jumppathobjecttopology.h>
#include <kypathdata/pathobject/info/dynamicpathobjectinfo.h>

namespace Kaim
{

typedef KyUInt32 JumpGameObjectRenderMasks; /*!< Defines a type for a bitmask composed of elements from the #JumpGameObjectRenderFlags enumeration. \ingroup kyruntime_pathobjects */
/*! Enumerates the types of information that can be rendered in a call to JumpGameObject::Render(). \ingroup kyruntime_pathobjects */
enum JumpGameObjectRenderFlags
{
	JumpGameObjectRender_GameObject = 1, /*!< Indicates that the jump trajectory should be rendered. */
	JumpGameObjectRender_PathObject = 2, /*!< Indicates that the PathObject data should be rendered. */
	JumpGameObjectRender__dummy__forceEnumSize = KYFORCEENUMSIZEINT
};

/*!	The JumpGameObject class provides an interface for using an off-the-shelf PathObject that simulates
	a connection between two points that can only be crossed by jumping.
	\section jumpposetup Setting up a jump
	\subsection requirements Requirements
	-	Any Bot using the jump must have the ActionSpeed, ActionRotate and ActionForcePosition classes of 
		ActionAttribute assigned to its Action.
	\subsection jumppoadd Adding a jump to the World
	To set up a jump in your World:
	-#	Create a new instance of the JumpGameObject in your \gameOrSim engine. In the constructor, you will provide
		a variety of information about the location and behavior of the jump.
	-#	Call AddToKynapse() in order to add the jump to the specified World. The jump will be stitched in to the
		Graph in the Database that you specified in the constructor.
	-#	When you want to remove the jump from the World, call RemoveFromKynapse().
	\section jumppousing Using a jump
	Using a jump PathObject involves the following considerations
	\subsection jumppoqueue Multiple Bots
	Only one Bot can traverse a jump at any given time. Additional \Bots that try to use the jump while it is already
	in use will be rejected.
	\ingroup kyruntime_pathobjects */
class JumpGameObject
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:

	/*! \label_constructor */
	JumpGameObject():
		m_pathObject(KY_NULL),
		m_topology(KY_NULL),
		m_info(KY_NULL),
		m_behavior(KY_NULL) {}

	/*! \label_constructor
		\param databaseName		The name of the Database to which the PathObject will be connected. Only \Bots using this Database will be able
								to take the PathObject into account when planning and following paths.
		\param start			A starting position at one end of the jump. Should be on or close to the ground. Must be within
								the boundaries of the NavMesh.
		\param end				A target position at the other end of the jump. Should be on or close to the ground. Must be within
								the boundaries of the NavMesh.
		\param entityHeight		The height of the characters that use the Database specified by \c databaseName.
		\param halfWidth		Half the width of the characters that use the Database specified by \c databaseName.
		\param jumpHeight		The maximum difference in height between the apex of the jump and the higher of the start and end positions.
		\param bidirectional	Indicates whether or not the jump can be traversed in both directions. If \c true, \Bots will be able to pass from
								the start to the end, and from the end to the start. If \c false, \Bots will only be able to pass from the
								start to the end.
		\param poId				A unique ID for the PathObject. Used only to create a file name if you dump the PathObject to a file on disk. */
	JumpGameObject(
		const char* databaseName,
		const Kaim::Vec3f& start,
		const Kaim::Vec3f& end,
		const KyFloat32 entityHeight,
		const KyFloat32 halfWidth,
		const KyFloat32 jumpHeight,
		bool bidirectional,
		PathObjectId poId);

	/*! \label_destructor Removes the PathObject from its World, and deletes its topology, info and behavior. */
	~JumpGameObject();

public:
	/*! Adds the PathObject to the specified World.*/
	void AddToKynapse(World& world) { if (m_pathObject) m_pathObject->AttachToWorld(&world); }

	/*! Removes the PathObject from its World, but does not delete its topology, info and behavior. It can be
		re-added at any time. */
	void RemoveFromKynapse() { if (m_pathObject) m_pathObject->DetachFromWorld(); }

	/*! Renders the PathObject.
		\param renderMask		A bitmask composed of elements from the #JumpGameObjectRenderFlags enumeration
								that specifies what information to render. */
	void Render(JumpGameObjectRenderMasks renderMask);

protected:
	JumpPathObject* m_pathObject;
	JumpPathObjectTopology* m_topology;
	DynamicPathObjectInfo* m_info;
	JumpPathObjectBehavior* m_behavior;
};

} // namespace Kaim

#endif // KYRUNTIME_JUMP_GAMEOBJECT_H
