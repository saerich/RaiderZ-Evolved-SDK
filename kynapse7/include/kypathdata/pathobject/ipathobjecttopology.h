/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_pathobject */

#ifndef KyPathData_IPathObjectTopology_H
#define KyPathData_IPathObjectTopology_H

#include "kypathdata/pathobject/pathobjectdatatypes.h"
#include "kypathdata/generic/memory.h"

namespace Kaim
{

class IPathObjectConnexionSpace;
class IPathObjectConnexionLink;
class IPathObjectControlSpace;

/*! Defines the base interface for a PathObject topology, which represents the connection spaces at which
	\Bots can enter and leave the PathObject, the links the Bot can follow between those connection spaces,
	and the control space (or spaces) that defines the area within which the PathObject controls the movements of the
	\Bots traversing it.
	\pp This class cannot be used directly. Use a class that derives from DynamicPathObjectTopology instead.
	\ingroup kypathdata_pathobject */
class IPathObjectTopology : public VirtualMemObject
{
public:
	/*! \label_baseconstructor */
	IPathObjectTopology() : m_userData(KY_NULL) {}

	/*! \label_virtualdestructor */
	virtual ~IPathObjectTopology() {}

	/*! Retrieves the number of connection spaces set up for the topology. */
	virtual KyUInt32 GetConnexionSpaceCount() const = 0;

	/*! Retrieves the connection space with the specified index. */
	virtual IPathObjectConnexionSpace* GetConnexionSpace(const PathObjectConnexionSpaceIdx& idx) const = 0;

	/*! Retrieves the number of connection links set up between the connection spaces in this topology. */
	virtual KyUInt32 GetConnexionLinkCount() const = 0;

	/*! Retrieves the connection link with the specified index. */
	virtual IPathObjectConnexionLink* GetConnexionLink(const PathObjectConnexionLinkIdx& idx) const = 0;

	/*! Retrieves the number of control spaces set up for the topology. */
	virtual KyUInt32 GetControlSpaceCount() const = 0;

	/*! Retrieves the control space with the specified index. */
	virtual IPathObjectControlSpace* GetControlSpace(const PathObjectControlSpaceIdx& idx) const = 0;

	/*! Indicates whether or not the control spaces and connection spaces will detect accidents if the Bot
		using the PathObject leaves their volumes unexpectedly.
		\return \c true to activate accident detection, or \c false to disable. */
	virtual bool CanDetectAccident() const = 0;

	/*! Renders the control space according to the specified flags. Called transparently by PathObject::Render().
		\param KyUInt32			A bitmask composed of elements from the #PathObjectRenderFlags enumeration. */
	virtual void Render(KyUInt32 /*renderFlags*/) const {}

	/*! Stores a \c void pointer, typically used to identify an object in the \gameOrSim engine that corresponds
		to this topology or to its PathObject. */
	void SetUserData(void* userData) { m_userData = userData; }

	/*! Retrieves the value set through a call to SetUserData(). */
	void* GetUserData() const { return m_userData; }

private:
	void* m_userData;
};

}

#endif // #ifndef KyPathData_IPathObjectTopology_H
