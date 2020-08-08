/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_attributes */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_ACTIONCROUCH_H
#define KY_ACTIONCROUCH_H

#include <kyruntime/core/action.h>


namespace Kaim
{

/*!	This class is used by \SDKName components to indicate whether or not a character in the \gameOrSim engine should crouch.
	\pp This ActionAttribute is not currently used by any components in the SDK. You may use it if you write
	a custom component that needs to make your character crouch under a given set of circumstances.
	\ingroup kyruntime_attributes */
class ActionCrouch: public Kaim::ActionAttribute
{
public:
	KY_DECLARE_ACTIONATTRIBUTE(ActionCrouch)


public:
	/*!	\label_baseconstructor */
	ActionCrouch() {}
	
	/*!	\label_virtualdestructor */
	virtual ~ActionCrouch() {}
	
	/*!	Copies the state of the specified ActionCrouch instance to this, and marks this ActionAttribute as 
		having changed in the current frame. */
	virtual Kaim::ActionAttribute& operator=(const Kaim::ActionAttribute& attr); //< Copies attribute state and launches Update().

	/*!	Sets whether or not the character in the \gameOrSim engine should crouch. */
	void SetIsCrouching(KyBool isCrouching)	{ m_isCrouching = isCrouching; Update(); }

	/*!	Retrieves whether or not the character in the \gameOrSim engine should crouch. */
	KyBool GetIsCrouching() const { return m_isCrouching; }


private:
	KyBool m_isCrouching; //KY_TRUE if the bot is crouching, KY_FALSE otherwise
};

} // namespace Kaim

#endif // KY_ACTIONCROUCH_H
