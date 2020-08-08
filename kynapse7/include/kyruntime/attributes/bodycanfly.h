/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_attributes */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_BODYCANFLY_H
#define KY_BODYCANFLY_H

#include <kyruntime/core/bot.h>


namespace Kaim
{

/*!	This class indicates to \SDKName components whether or not a character in the \gameOrSim engine can fly. 
	\pp The BodyCanFly value is read by the Steering_Fly modifier of the PathFinder. If it indicates
	that a Bot can fly, the modifier automatically generates flight paths for that Bot in three
	dimensions, provided that appropriate PathData is available and that the Action owned by the Bot
	has also been assigned the ActionVerticalSpeed class of ActionAttribute.
	\pp See :USERGUIDE:"Flying".
	\ingroup kyruntime_attributes */
class BodyCanFly: public BodyAttribute
{
public:
	KY_DECLARE_BODYATTRIBUTE(BodyCanFly)


public:
	/*!	\label_baseconstructor */
	BodyCanFly(): m_canFly(KY_FALSE) {}
	
	/*!	\label_virtualdestructor */
	virtual ~BodyCanFly() {}
	
	/*!	Copies the state of the specified BodyCanFly instance to this. */
	virtual BodyAttribute& operator=(const BodyAttribute& attr);

	/*!	Sets whether or not the character in the \gameOrSim engine is able to fly.
		\param canFly			#KY_TRUE if the character can fly, or #KY_FALSE otherwise. */
	void SetCanFly(KyBool canFly) { m_canFly = canFly; }

	/*!	Retrieves whether or not the character in the \gameOrSim engine is able to fly.
		\return #KY_TRUE if the character can fly, or #KY_FALSE otherwise. */
	KyBool GetCanFly() const { return m_canFly; }


private:
	KyBool m_canFly;
};

} // namespace Kaim

#endif // KY_BODYCANFLY_H
