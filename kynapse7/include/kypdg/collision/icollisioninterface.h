/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

// primary contact: JODA - secondary contact: NONE

/*! \file
	\ingroup kypdg */

#ifndef KyPdg_ICollisionInterface_H
#define KyPdg_ICollisionInterface_H

#include "kypathdata/math/vec3f.h"
#include "kypathdata/containers/array.h"

namespace Kaim
{

struct InputTriangle;
class Box3f;
class FlyCellDesc;
class PdgSystem;
class PdgInputCell;

/*! The ICollisionInterface class provides an abstract base interface that conducts swept-sphere tests against
	the triangles in a collision world. It is used by the PathData generation system to conduct collision tests
	when generating PathData for flying \Bots, and when generating "jump edge candidates" for ground-based \Bots: 
	places where a Bot can drop down from a vertical height that it is not allowed to climb.
	\pp You cannot use this class as-is; you must write your own implementation that typically hooks in to your own
	physics or collision system. You must also write a class that derives from CollisionInterfaceFactory, which
	the PathData generation framework will invoke to create and destroy instances of your ICollisionInterface
	class, and provide an instance of that CollisionInterfaceFactory to the Generator.
	\pp The PathData generation system invokes this class as follows: \code
	ICollisionInterface* collisionInterface = collisionInterfaceFactory->NewCollisionInterface()
	for (...)
	{
		collisionInterface.DoInit(...);
		collisionInterface.DoTriangleGeometry(...)
		...
		collisionInterface.DoIsColliding(...) // several calls
		...
		collisionInterface.DoClearGeometry()
	}
	collisionInterfaceFactory->DeleteCollisionInterface(collisionInterface); \endcode
	\ingroup kypdg */
class ICollisionInterface : public VirtualMemObject
{
public:
	/*! \label_baseconstructor */
	ICollisionInterface() : m_floatTriangles(KY_NULL) {}

	/*! \label_virtualdestructor */
	virtual ~ICollisionInterface() { KY_FREE(m_floatTriangles); }

	void ClearGeometry();
	void Init(const Box3f& box) { DoInit(box); }
	void Init(PdgSystem& sys, const FlyCellDesc& cellDesc);
	void AddPdgInputGeometry(PdgSystem& sys, const Array<const PdgInputCell*>& inputCells);
	bool IsColliding(const Vec3f& start, const Vec3f& end, KyFloat32 halfwidth) const { return DoIsColliding(start, end, halfwidth); };

protected:
	void AddTriangleGeometry(PdgSystem& sys, const InputTriangle triangles[], KyUInt32 nbTriangles);
protected:
	/*! Called by the PathData generation system to set up the bounding box around the collision world. */
	virtual void DoInit(const Box3f& box) = 0;

	/*! Called by the PathData generation system to add triangles to the collision world.
		\param pVertexArray		The list of vertices that make up the triangles to add to collision world. Each 3 consecutive elements
								from this list make up one triangle.
		\param nbVertices		The number of vertices in \c pVertexArray. Should be a multiple of 3. */
	virtual void DoAddTriangleGeometry(const KyFloat32 pVertexArray[], KyUInt32 nbVertices) = 0;

	/*! Called by the PathData generation system to conduct a swept-sphere test between the specified positions.
		\param startPosition	The starting point for the sphere.
		\param endPosition		The end point for the sphere.
		\param halfwidth		The radius of the sphere.
		\return \c true if the sphere collides with the collision world, or \c false if the sphere reached its end point without colliding. */
	virtual bool DoIsColliding(const Vec3f& startPosition, const Vec3f& endPosition, KyFloat32 halfwidth) const = 0;

	/*! Called by the PathData generation system to clear all resources reserved in DoInit() and DoAddTriangleGeometry(). */
	virtual void DoClearGeometry() = 0;
protected:
	KyFloat32* m_floatTriangles;
};

/*! The CollisionInterfaceFactory provides an abstract base class for an object that the PathData generation
	system uses to retrieve an instance of a class that implements the ICollisionInterface interface.
	\pp This class cannot be used as-is; you must write your own thread-safe implementation that returns an instance of your
	own custom implementation of ICollisionInterface, and provide an instance of your CollisionInterfaceFactory to the
	Generator.
	\ingroup kypdg */
class CollisionInterfaceFactory : public VirtualMemObject
{
public:
	/*! Called by the PathData generation system to retrieve a new instance of a class that implements ICollisionInterface. */
	virtual ICollisionInterface* NewCollisionInterface() = 0;

	/*! Called by the PathData generation system to destroy an existing instance of a class that implements ICollisionInterface. */
	virtual void DeleteCollisionInterface(ICollisionInterface* collisionInterface) = 0;
};


/* Raii Class to manage CollisionInterface memory. */
class AutoCollisionInterface
{
KY_DEFINE_NEW_DELETE_OPERATORS
public:
	explicit AutoCollisionInterface(CollisionInterfaceFactory& factory) :
		m_factory(&factory),
		m_collisionInterface(factory.NewCollisionInterface())
	{}
	~AutoCollisionInterface() { m_factory->DeleteCollisionInterface(m_collisionInterface); }

	const ICollisionInterface* operator -> () const { return m_collisionInterface; }
	ICollisionInterface* operator -> () { return m_collisionInterface; }

	const ICollisionInterface& operator * () const { return *m_collisionInterface; }
	ICollisionInterface& operator * () { return *m_collisionInterface; }

private:
	AutoCollisionInterface& operator = (ICollisionInterface* collisionInterface);
	AutoCollisionInterface(const AutoCollisionInterface&);
	AutoCollisionInterface& operator = (const AutoCollisionInterface&);
private:
	CollisionInterfaceFactory* m_factory;
	ICollisionInterface* m_collisionInterface;
};

} // namespace Kaim

#endif
