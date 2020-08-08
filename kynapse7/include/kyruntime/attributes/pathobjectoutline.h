/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_attributes */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_PATHOBJECTOUTLINE_H
#define KY_PATHOBJECTOUTLINE_H

#include <kyruntime/core/bot.h>


namespace Kaim
{

class RuntimePathObjectControlSpace;
class NavFloorPtr;

/*!	This class is used internally by the PathObjectManager to represent an outline around the
	control space of each PathObject that is configured to block \Bots from crossing its control space.
	You should not have to interact with this class directly.
	\ingroup kyruntime_attributes */
class PathObjectOutline: public VirtualMemObject
{

public:
	/*!	\label_baseconstructor */
	PathObjectOutline();

	/*!	\label_virtualdestructor */
	~PathObjectOutline();

	/*!	Called internally at initialization time.
		\param controlSpace			The control space of the PathObject. 
		\param outlineMaxSize		The maximum number of points in the outline around the control space, used to allocate
									memory for the #m_outline array.
		\param navFloorMaxCount		The maximum number of \NavFloors the control space can overlap, used to allocate
									memory for the #m_navFloorPtrs array. */
	virtual void Init(RuntimePathObjectControlSpace* controlSpace, KyUInt32 outlineMaxSize, KyUInt32 navFloorMaxCount);



public:
	RuntimePathObjectControlSpace* m_controlSpace; /*!< A pointer to the control space of the PathObject. */
	Vec3f* m_outline; /*!< The array containing the outline around the control space. This outline must be closed. */
	NavFloorPtr* m_navFloorPtrs; /*!< The array containing the \NavFloors overlapped by the outline. Updated automatically at each frame by Update(). */
	KyUInt32 m_outlineMaxSize; /*!< The maximum number of points in the outline: i.e. the size of the #m_outline array. */
	KyUInt32 m_navFloorMaxCount; /*!< The maximum number of \NavFloors that can be overlapped by the outline: i.e. the size of the #m_navFloorPtrs array. */
	KyUInt32 m_outlineSize; /*!< The current size of the outline: i.e. the number of vectors used in the #m_outline array. */
	KyFloat32 m_thickness; /*!< The thickness of the PathObject. */
	KyBool m_hasChanged; /*!< Indicates whether or not the outline has changed in the current frame. */
};


} // namespace Kaim

#endif // KY_PATHOBJECTOUTLINE_H
