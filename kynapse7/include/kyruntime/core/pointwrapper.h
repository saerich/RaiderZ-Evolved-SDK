/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_core */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_POINTWRAPPER_H
#define KY_POINTWRAPPER_H

#include <kypathdata/generic/memory.h>
#include <kypathdata/math/vec3f.h>


//	The maximum number of types of additional information that can be added to any PointWrapper. For internal use. 
#define KY_MAX_POINT_INFO 16


namespace Kaim
{

class PointInfo;
class PointInfoClass;


////////////////////////////////////////////////////////////////////////////////
// PointWrapper
////////////////////////////////////////////////////////////////////////////////

/*!	This class is a wrapper around a Vec3f object that contains additional information
	related to the position stored in the Vec3f. For example, if the Vec3f represents the location
	of a vertex in a Graph, the PointWrapper may also contain a GraphVertexSafePtr that refers to that vertex. Or, if the
	Vec3f represents the location of a Bot, the PointWrapper may also contain a pointer to that
	Bot. PointWrapper instances are typically used in \SDKName API functions whenever such additional
	information may be useful to speed up computations.
	\pp Additional information of a specific type can be added to a PointWrapper by writing a new class that derives from PointInfo to 
	represent the desired type of information, and adding an instance of that custom PointInfo class to your PointWrapper using 
	the SetInfo() method. 
	\ingroup kyruntime_core */
class PointWrapper: public VirtualMemObject
{
public:
	/*!	\label_constructor Points to the origin. */
	PointWrapper(): m_position(0.0f, 0.0f, 0.0f) { ClearPointInfo(); }

	/*!	\label_constructor
		\param position				The position to set for the PointWrapper. */
	PointWrapper(const Vec3f& position): m_position(position) { ClearPointInfo(); }

	/*!	\label_constructor
		\param pw					A PointWrapper whose data will be copied to the new instance. */
	PointWrapper(const PointWrapper& pw) { *this = pw; }

	/*!	Copies the state of the specified PointWrapper to this. */
	inline PointWrapper& operator=(const PointWrapper& pw);

	// Accessors for the position.
	/*!	Sets the Vec3f position wrapped by this instance of PointWrapper. */
	void SetPosition(const Vec3f& position) { m_position = position; }

	/*!	Retrieves the Vec3f position wrapped by this instance of PointWrapper. */
	const Vec3f& GetPosition() const { return m_position; }

	/*!	Retrieves additional information attached to the PointWrapper if its MetaClass matches the specified MetaClass.
		\param infoClass		The MetaClass of the point information to be retrieved.
		\return A pointer to the requested information, or #KY_NULL if no information that matches the specified
				MetaClass has been set for this instance of PointWrapper. */
	PointInfo* GetInfo(PointInfoClass& infoClass) const;

	/*!	Retrieves additional information attached to the PointWrapper if its class matches the specified class.
		\tparam T				The class of the point information to be retrieved.
		\return A pointer to the requested information, or #KY_NULL if no information that matches the specified
				class has been set for this instance of PointWrapper. */
	template < class T >
	T* GetInfo() const { return static_cast<T*>(GetInfo(T::Class)); }

	/*!	Sets additional information about the point. This information is not copied; only a pointer to
		the specified object is stored. You must ensure that this pointer remains valid as long as this
		PointWrapper is used, or until you call ClearPointInfo().
		\param info				An instance of a class that derives from PointInfo and that contains
								additional information about the point wrapped by this instance of 
								PointWrapper. */
	KyResult SetInfo(PointInfo& info);

	/*!	Removes all additional information stored for the point. */
	inline void ClearPointInfo();

	/*! Resets  all additional information stored for the point. */
	inline void ResetPointInfos();


protected:
	PointInfo* m_info[KY_MAX_POINT_INFO];
	Vec3f m_position;
};

} // namespace Kaim


#include <kyruntime/core/pointwrapper.inl>


#endif // KY_POINTWRAPPER_H
