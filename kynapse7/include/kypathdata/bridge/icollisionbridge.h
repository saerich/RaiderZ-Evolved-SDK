/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_bridge */

#ifndef KY_ICOLLISIONBRIDGE_H
#define KY_ICOLLISIONBRIDGE_H

#include "kypathdata/generic/types.h"
#include "kypathdata/math/vec3f.h"

namespace Kaim
{
	class World;
	class Body;

	/*!	Defines an interface used by \SDKName components to conduct collision tests against
		the \gameOrSim terrain or physics mesh.
		\pp The Bridge can maintain an instance of a class that implements this interface, making the interface available to other
		\SDKName components. Because the implementation of this interface is specific to the \gameOrSim engine and/or third-party
		tools in use, no implementations of this interface are supplied by default with the SDK. You must write
		your own implementation of ICollisionBridge, and assign an instance of your class to the Bridge.
		\pp See :USERGUIDE:"Customizing Bridge Interfaces" and :USERGUIDE:"Initializing \SDKName".
		\ingroup kypathdata_bridge */
	class ICollisionBridge
	{
	public:

		/*!	Describes a raycast query carried out by ICollisionBridge::RayCast() or ICollisionBridge::HasRayCastHit().
			\ingroup kypathdata_bridge */
		struct RayCastQuery
		{
			RayCastQuery() : world(KY_NULL), body(KY_NULL) {}

			Vec3f start; /*!< The starting point for the ray. */ 
			Vec3f end; /*!< The ending point for the ray. */
			const World* world; /*!< The \SDKName World in which this query is being carried out, or NULL if none. */
			const Body* body; /*!< A Body associated with this query, or NULL if none. */
		};

		/*!	Describes the results of a raycast query carried out by ICollisionBridge::RayCast() or ICollisionBridge::HasRayCastHit().
			\ingroup kypathdata_bridge */
		struct RayCastResult
		{
			bool hasHit; /*!< Indicates whether or not the ray collides with an obstacle. */
			Vec3f hitPosition; /*!< Indicates the position of the first collision, if any. */
			Vec3f hitNormal; /*!< Indicates the normal of the first collision, if any. */
		};

	public:
		/*!	\label_virtualdestructor */
		virtual ~ICollisionBridge() {}

	public:
		/*!	Conducts a raycast with a zero width against the obstacles in the \gameOrSim engine.
			\param query			Describes the starting and ending points of the ray.
			\param[out] result		Describes the results of the test. */
		virtual void RayCast(const RayCastQuery& query, /*[out]*/RayCastResult& result) = 0;
		
		/*!	Indicates whether or not a ray with zero width collides with an obstacle in the \gameOrSim engine.
			\param query			Describes the starting and ending points of the ray.
			\return true if the ray collides with at least one obstacle, or false otherwise. */
		virtual bool HasRayCastHit(const RayCastQuery& query) = 0;

	public:
		/*!	Describes a sphere-cast query carried out by ICollisionBridge::SphereCast() or ICollisionBridge::HasSphereCastHit().
			\ingroup kypathdata_bridge */
		struct SphereCastQuery : RayCastQuery
		{
			KyFloat32 radius; /*!< The radius of the sphere. */
		};

		/*!	Describes the results of a sphere-cast query carried out by ICollisionBridge::SphereCast() or ICollisionBridge::HasSphereCastHit().
			\ingroup kypathdata_bridge */
		struct SphereCastResult : RayCastResult
		{
		};

	public:
		/*!	Casts a sphere against the obstacles in the \gameOrSim engine.
			\param query			Describes the starting and ending points of the sphere cast.
			\param[out] result		Describes the results of the test. */
		virtual void SphereCast(const SphereCastQuery& query, /*[out]*/SphereCastResult& result) = 0;
		
		/*!	Indicates whether or not a cast sphere collides with any obstacles in the \gameOrSim engine.
			\param query			Describes the starting and ending points of the sphere cast.
			\return true if the sphere collides with at least one obstacle, or false otherwise. */
		virtual bool HasSphereCastHit(const SphereCastQuery& query) = 0;
	};

} // namespace Kaim


#endif // KY_ICOLLISIONBRIDGE_H
