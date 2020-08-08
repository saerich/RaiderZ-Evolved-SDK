/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_attributes */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_ACTIONSTEERING_H
#define KY_ACTIONSTEERING_H

#include <kyruntime/core/action.h>
#include <kypathdata/basesystem/config.h>


namespace Kaim
{

/*!	This class is used by \SDKName components to indicate a steering value that should be applied to the forward movements of
	a character in the \gameOrSim engine.
	\pp Every Bot must be assigned either ActionRotate or ActionSteering, in order for the PathFinder to indicate the
	direction or change in direction that should be applied to the Bot at each frame.
	\pp The value set for this ActionAttribute by the components supplied in this SDK is always either 
	–1 or 1, where 1 represents steering to the right and –1 represents steering to the left.
	\ingroup kyruntime_attributes */
class ActionSteering: public ActionAttribute
{
public:
	KY_DECLARE_ACTIONATTRIBUTE(ActionSteering)


public:
	/*!	\label_baseconstructor */
	ActionSteering() {}
	
	/*!	\label_virtualdestructor */
	virtual ~ActionSteering() {}
	
	/*!	Copies the state of the specified ActionSteering instance to this, and marks this ActionAttribute as 
		having changed in the current frame. */
	virtual ActionAttribute& operator=(const ActionAttribute& attr);

	/*!	Sets the steering value that should be applied to the orientation of the character in the \gameOrSim engine. */
	void SetSteering(KyFloat32 steering)
	{
		KY_FUNCTION("ActionSteering::SetSteering");

		KY_AFFECT_FLOAT_IN_RANGE("Steering", -1.0f, 1.0f, m_steering, steering);
		Update();
	}

	/*!	Retrieves the steering value that should be applied to the orientation of the character in the \gameOrSim engine. */
	KyFloat32 GetSteering() const { return m_steering; }


private:
	KyFloat32 m_steering;
};

} // namespace Kaim

#endif // KY_ACTIONSTEERING_H
