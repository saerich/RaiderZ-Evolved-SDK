/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_attributes */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_ACTIONACCELERATION_H
#define KY_ACTIONACCELERATION_H

#include <kyruntime/core/action.h>
#include <kypathdata/generic/macros.h>
#include <kypathdata/basesystem/config.h>


namespace Kaim
{

/*!	This class is used by \SDKName components to indicate the linear acceleration value that should be applied to a
	character in the \gameOrSim engine.
	\pp Every Bot must be assigned either ActionSpeed or ActionAcceleration, in order for the PathFinder to indicate
	the absolute speed or the relative change in speed that should be applied to the Bot at each frame.
	\pp The value set for this ActionAttribute by the components supplied in this SDK is always either –1 or 1, 
	where 1 represents acceleration in the orientation of the Bot and –1 represents acceleration in the opposite
	orientation (i.e. deceleration). If the speed of the Bot is a negative, a negative acceleration should increase
	its absolute value.
	\ingroup kyruntime_attributes */
class ActionAcceleration: public ActionAttribute
{
public:
	KY_DECLARE_ACTIONATTRIBUTE(ActionAcceleration)


public:
	/*!	\label_baseconstructor */
	ActionAcceleration() {}
	
	/*!	\label_virtualdestructor */
	virtual ~ActionAcceleration() {}
	
	/*!	Copies the state of the specified ActionAcceleration instance to this, and marks this ActionAttribute as 
		having changed in the current frame. */
	virtual ActionAttribute& operator=(const ActionAttribute& attr);

	/*!	Sets the acceleration value that should be applied to the character in the \gameOrSim engine. */
	void SetAcceleration(KyFloat32 acc)
	{
		KY_FUNCTION("ActionAcceleration::SetAcceleration");
		KY_AFFECT_FLOAT_IN_RANGE("Acceleration", -1.0f, 1.0f, m_acc, acc);
		Update(); 
	}

	/*!	Retrieves the acceleration value  that should be applied to the character in the \gameOrSim engine. */
	KyFloat32 GetAcceleration() const { return m_acc; }


private:
	KyFloat32 m_acc;
};

} // namespace Kaim

#endif // KY_ACTIONACCELERATION_H
