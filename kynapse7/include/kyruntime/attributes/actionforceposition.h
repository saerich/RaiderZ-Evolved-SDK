/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_attributes */

// primary contact: MAMU - secondary contact: NOBODY
#ifndef KY_ACTIONFORCEPOSITION_H
#define KY_ACTIONFORCEPOSITION_H

#include <kyruntime/core/action.h>
#include <kypathdata/math/vec3f.h>


namespace Kaim
{

/*!	This class is used by \SDKName components to indicate the absolute position at which a character in the
	\gameOrSim engine should be placed.
	\pp This ActionAttribute is used by the off-the-shelf classes of PathObject in order to place the
	Bot at specific positions as it traverses the PathObject. If you use any of the off-the-shelf PathObject
	classes, you must assign this ActionAttribute to your Bot.
	\ingroup kyruntime_attributes */
class ActionForcePosition: public Kaim::ActionAttribute
{
public:
	KY_DECLARE_ACTIONATTRIBUTE(ActionForcePosition)

public:
	/*!	\label_baseconstructor */
	ActionForcePosition() {}
	
	/*!	\label_virtualdestructor */
	virtual ~ActionForcePosition() {}
	
	/*!	Copies the state of the specified ActionForcePosition instance to this, and marks this ActionAttribute as 
		having changed in the current frame. */
	virtual Kaim::ActionAttribute& operator=(const Kaim::ActionAttribute& attr); //< Copies attribute state and launches Update().

	/*!	Sets the target position for a character in the \gameOrSim engine. */
	void SetPosition(const Vec3f& position)	{ m_position = position; Update(); }

	/*!	Retrieves the target position for a character in the \gameOrSim engine. */
	const Vec3f& GetPosition() const { return m_position; }


private:
	Vec3f m_position; // Target position
};

} // namespace Kaim

#endif // KY_ACTIONFORCEPOSITION_H
