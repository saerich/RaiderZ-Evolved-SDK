/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_pathobject */

#ifndef KyPathData_IPathObjectInfo_H
#define KyPathData_IPathObjectInfo_H

#include "kypathdata/pathobject/pathobjectdatatypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/graph/graphtypes.h"

namespace Kaim
{

class CoordSystem;

/*! Defines the base interface for retrieving information about a PathObject, such as its type, unique ID,
	and the Database it should connect to.
	\pp This class cannot be used directly. Use a class that derives from DynamicPathObjectInfo instead.
	\ingroup kypathdata_pathobject */
class IPathObjectInfo : public VirtualMemObject
{
public:
	/*! \label_virtualdestructor */
	virtual ~IPathObjectInfo() {}

	/*!	Returns the name of the Database that this PathObject will connect to. For example, "Bipeds", "Giants", etc. */
	virtual const char* GetDataBaseName() const = 0;

	/*!	Returns the average height of the characters that will traverse this PathObject. */
	virtual KyFloat32 GetAverageEntityHeight() const = 0;

	/*! Renders the info stored in this object according to the specified flags. Called transparently by PathObject::Render().
		\param KyUInt32			A bitmask composed of elements from the #PathObjectRenderFlags enumeration. */
	virtual void Render(KyUInt32 /*renderFlags*/) const = 0;

	/*!	Returns a unique ID for the PathObject within its Database.
		This unique ID is used only to create a file name if you dump your \PathObjects to a file. */
	virtual PathObjectId GetId() const = 0;

	/*!	Returns a descriptive type for the PathObject, such as "WoodenDoor", "MetalDoor", "Transporter", etc.
		This type is used only to create a file name if you dump your \PathObjects to a file. */
	virtual const char* GetType() const = 0;
};

}

#endif // #ifndef KyPathData_IPathObjectInfo_H
