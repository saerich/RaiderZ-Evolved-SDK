/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_attributes */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_ACTIONJUMP_H
#define KY_ACTIONJUMP_H

#include <kyruntime/core/action.h>


namespace Kaim
{

/*!	This class is used by \SDKName components to indicate whether or not a character in the \gameOrSim engine should jump.
	\pp This ActionAttribute is not currently used by any components in the SDK. You may use it if you write
	a custom component that needs to make your character jump under a given set of circumstances.
	\ingroup kyruntime_attributes */
class ActionJump: public Kaim::ActionAttribute
{
public:
	KY_DECLARE_ACTIONATTRIBUTE(ActionJump)


public:
	/*!	\label_baseconstructor */
	ActionJump() {}
	
	/*!	\label_virtualdestructor */
	virtual ~ActionJump() {}
	
	/*!	Copies the state of the specified ActionJump instance to this, and marks this ActionAttribute as 
		having changed in the current frame. */
	virtual Kaim::ActionAttribute& operator=(const Kaim::ActionAttribute& attr); //< Copies attribute state and launches Update().

	/*!	Sets whether or not the character in the \gameOrSim engine should jump. */
	void SetIsJumping(KyBool isJumping)	{ m_isJumping = isJumping; Update(); }

	/*!	Retrieves whether or not the character in the \gameOrSim engine should jump. */
	KyBool GetIsJumping() const { return m_isJumping; }


private:
	KyBool m_isJumping; // KY_TRUE if the bot is jumping, KY_FALSE otherwise
};

} // namespace Kaim

#endif // KY_ACTIONJUMP_H
