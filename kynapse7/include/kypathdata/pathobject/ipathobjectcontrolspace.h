/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_pathobject */

#ifndef KyPathData_IPathObject_ControlSpace_H
#define KyPathData_IPathObject_ControlSpace_H

#include "kypathdata/generic/memory.h"

namespace Kaim
{

class IVolume;

/*! Defines the base interface for a PathObject control space, which represents a volume within which
	the PathObject has sole responsibility over the movements of a Bot.
	\pp This class cannot be used directly. Use DynamicPathObjectControlSpace instead.
	\ingroup kypathdata_pathobject */
class IPathObjectControlSpace : public VirtualMemObject
{
public:
	/*! \label_baseconstructor */
	IPathObjectControlSpace() : m_userData(KY_NULL) {}

	/*! \label_virtualdestructor */
	virtual ~IPathObjectControlSpace() {}

	/*! Retrieves the volume of the control space. */
	virtual const IVolume* GetVolume() const  = 0;

	/*! Indicates whether or not the Graph edges that intersect this control space should be locked and
		considered unusable by the PathFinder. This forces \Bots to use the PathObject if they need to cross
		the control space. For example, a door typically locks the edges within its control space so that 
		when the door is closed and the PathObject prevents \Bots from traversing it, the \Bots will not
		attempt to go through the doorway anyway, using edges in the static Graph rather than using the
		PathObject.
		\return \c true if the control space should lock intersecting edges, or \c false otherwise. */
	virtual bool CanLockEdges() const = 0;

	/*! Indicates whether or not this control space should be considered as an Obstacle. This prevents other
		\Bots from passing through the control space when smoothing their paths. This is typically the case 
		for \PathObjects that represent physical objects. For example, a destructible bridge or plank will
		likely need to declare itself an Obstacle, so that other \Bots will not collide with it.
		\pp If this method returns \c true, the control space will be reflected in the PathObjectLayer created
		and maintained internally by the NavMeshLayerManager, which will make the CanGo_NavMesh modifier of the
		PathFinder avoid smoothing paths through the control space.
		\pp Note that this setting has no effect on the path smoothing of flying creatures. You will have to 
		prevent flying creatures from moving through the object as you would for any other physical object. 
		See :USERGUIDE:"Flying".
		\return \c true if the control space should be considered a physical obstacle, or \c false otherwise. */
	virtual bool IsAnObstacle() const = 0;

	/*! Indicates whether or not a Bot that starts or restarts its path inside this control space must adopt the
		PathObject.
		\return \c true if the PathObject must have total control over the movements of \Bots that are spawned or
				that re-calculate a path while inside the control space. */
	virtual bool IsASpaceToStartUsingThePathObject() const = 0;

	/*! Renders the control space according to the specified flags. Called transparently by PathObject::Render().
		\param KyUInt32			A bitmask composed of elements from the #PathObjectRenderFlags enumeration. */
	virtual void Render(KyUInt32 /*renderFlags*/) const = 0;

	/*! Stores a \c void pointer, typically used to identify an object in the \gameOrSim engine that corresponds
		to this control space or to its PathObject. */
	void SetUserData(void* userData) { m_userData = userData; }

	/*! Retrieves the value set through a call to SetUserData(). */
	void* GetUserData() const { return m_userData; }

private:
	void* m_userData;
};

}

#endif // #ifndef KyPathData_IPathObject_ControlSpace_H
