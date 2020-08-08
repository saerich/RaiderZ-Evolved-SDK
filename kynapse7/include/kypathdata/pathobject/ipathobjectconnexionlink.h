/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_pathobject */

#ifndef KyPathData_IPathObject_ConnexionLink_H
#define KyPathData_IPathObject_ConnexionLink_H

#include "kypathdata/pathobject/pathobjectdatatypes.h"
#include "kypathdata/generic/memory.h"

namespace Kaim
{

/*! Defines the base interface for a PathObject connection link, which is a one-way link between two connection spaces
	of the same PathObject.
	\pp This class cannot be used directly. Use DynamicPathObjectConnectionLink instead.
	\ingroup kypathdata_pathobject */
class IPathObjectConnexionLink : public VirtualMemObject
{
public:
	/*! \label_baseconstructor */
	IPathObjectConnexionLink() : m_userData(KY_NULL) {}

	/*! \label_virtualdestructor */
	virtual ~IPathObjectConnexionLink() {}

	/*! Retrieves the unique index of the PathObject connection space at the beginning of this link. */
	virtual const PathObjectConnexionSpaceIdx& GetStartConnexionSpaceIdx() const = 0;

	/*! Retrieves the unique index of the PathObject connection space at the end of this link. */
	virtual const PathObjectConnexionSpaceIdx& GetEndConnexionSpaceIdx() const = 0;

	/*! Stores a \c void pointer, typically used to identify an object in the \gameOrSim engine that corresponds
		to this connection link or to its PathObject. */
	void SetUserData(void* userData) { m_userData = userData; }

	/*! Retrieves the value set through a call to SetUserData(). */
	void* GetUserData() const { return m_userData; }

private:
	void* m_userData;
};

}

#endif // #ifndef KyPathData_IPathObject_ConnexionLink_H
