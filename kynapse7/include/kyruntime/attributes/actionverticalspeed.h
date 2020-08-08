/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_attributes */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_ACTIONVERTICALSPEED_H
#define KY_ACTIONVERTICALSPEED_H

#include <kyruntime/core/action.h>


namespace Kaim
{

/*!	This class is used by \SDKName components to indicate the linear vertical speed that should be applied to a
	character in the \gameOrSim engine, in meters per second. 
	\pp This ActionAttribute is only used for flying characters. If a Bot can fly (indicated by the BodyCanFly 
	class of BodyAttribute), and flight-compatible PathData is available, the PathFinder updates the
	ActionVerticalSpeed value to make the Bot move in three dimensions. See :USERGUIDE:"Flying".
	\pp The value set for this ActionAttribute by the components supplied in this SDK is
	typically between \c –MaxSpeed and \c MaxSpeed, where \c MaxSpeed is the value of the BodyMaxSpeed
	class of BodyAttribute assigned to the Bot (if any). 
	\ingroup kyruntime_attributes */
class ActionVerticalSpeed: public ActionAttribute
{
public:
	KY_DECLARE_ACTIONATTRIBUTE(ActionVerticalSpeed)


public:
	/*!	\label_baseconstructor */
	ActionVerticalSpeed() {}
	
	/*!	\label_virtualdestructor */
	virtual ~ActionVerticalSpeed() {}
	
	/*!	Copies the state of the specified ActionVerticalSpeed instance to this, and marks this ActionAttribute as 
		having changed in the current frame. */
	virtual ActionAttribute& operator=(const ActionAttribute& attr);

	/*!	Sets the vertical speed that should be applied to the character in the \gameOrSim engine, in meters per second. */
	void SetVerticalSpeed(KyFloat32 verticalSpeed) { m_verticalSpeed = verticalSpeed; Update(); }

	/*!	Retrieves the vertical speed that should be applied to the character in the \gameOrSim engine, in meters per second. */
	KyFloat32 GetVerticalSpeed() const { return m_verticalSpeed; }


private:
	KyFloat32 m_verticalSpeed;
};

} // namespace Kaim

#endif // KY_ACTIONVERTICALSPEED_H
