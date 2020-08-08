/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_pathobject */

#ifndef KyPathData_IPathObject_ConnexionSpace_H
#define KyPathData_IPathObject_ConnexionSpace_H

#include "kypathdata/generic/memory.h"
#include "kypathdata/graph/graphtypes.h"
#include "kypathdata/math/vec3f.h"

namespace Kaim
{

class IVolume;

/*! Defines the base interface for a PathObject connection space, which is a transition zone between the
	PathFinder and the PathObject.
	\pp This class cannot be used directly. Use a class that derives from DynamicPathObjectConnectionSpace instead.
	\ingroup kypathdata_pathobject */
class IPathObjectConnexionSpace : public VirtualMemObject
{
public:
	/*! \label_baseconstructor */
	IPathObjectConnexionSpace() : m_userData(KY_NULL) {}

	/*! \label_virtualdestructor */
	virtual ~IPathObjectConnexionSpace() {}

	/*! Retrieves the position at which this connection space will be connected to the surrounding Graph.*/
	virtual const Vec3f& GetConnexionPosition() const = 0;

	/*! Retrieves the volume of the connection space. */
	virtual const IVolume* GetVolume() const = 0;

	/*! Renders the connection space according to the specified flags. Called transparently by PathObject::Render().
		\param KyUInt32			A bitmask composed of elements from the #PathObjectRenderFlags enumeration. */
	virtual void Render(KyUInt32 /*renderFlags*/) const {}

	/*! Stores a \c void pointer, typically used to identify an object in the \gameOrSim engine that corresponds
		to this connection space or to its PathObject. */
	void SetUserData(void* userData) { m_userData = userData; }

	/*! Retrieves the value set through a call to SetUserData(). */
	void* GetUserData() const { return m_userData; }

private:
	void* m_userData;
};

}

#endif // #ifndef KyPathData_IPathObject_ConnexionSpace_H
