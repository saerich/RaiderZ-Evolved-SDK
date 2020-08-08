/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_attributes */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_ACTIONSPEED_H
#define KY_ACTIONSPEED_H

#include <kyruntime/core/action.h>


namespace Kaim
{

/*!	This class is used by \SDKName components to indicate the linear speed at which a character in the \gameOrSim engine
	should move, in meters per second.
	\pp Every Bot must be assigned either ActionSpeed or ActionAcceleration, in order for the PathFinder to 
	indicate the speed or change in speed that should be applied to the Bot at each frame.
	\pp The value set for this ActionAttribute by the components supplied in this SDK is typically between 
	\c –MaxSpeed and \c MaxSpeed, where \c MaxSpeed is the value of the BodyMaxSpeed class of BodyAttribute
	assigned to the Bot (if any). 
	\ingroup kyruntime_attributes */
class ActionSpeed: public ActionAttribute
{
public:
	KY_DECLARE_ACTIONATTRIBUTE(ActionSpeed)


public:
	/*!	\label_baseconstructor */
	ActionSpeed() {}
	
	/*!	\label_virtualdestructor */
	virtual ~ActionSpeed() {}
	
	/*!	Copies the state of the specified ActionSpeed instance to this, and marks this ActionAttribute as 
		having changed in the current frame. */
	virtual ActionAttribute& operator=(const ActionAttribute& attr);

	/*!	Sets the speed that should be applied to the character in the \gameOrSim engine, in meters per second. */
	void SetSpeed(KyFloat32 speed) { m_speed = speed; Update(); }

	/*!	Retrieves the speed that should be applied to the character in the \gameOrSim engine, in meters per second. */
	KyFloat32 GetSpeed() const { return m_speed; }


private:
	KyFloat32 m_speed;
};

} // namespace Kaim

#endif // KY_ACTIONSPEED_H
