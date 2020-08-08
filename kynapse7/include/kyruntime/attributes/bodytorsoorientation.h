/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_attributes */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_BODYTORSOORIENTATION_H
#define KY_BODYTORSOORIENTATION_H

#include <kyruntime/core/bot.h>


namespace Kaim
{

/*!	This class indicates to \SDKName components the facing direction of the body of a character in the
	\gameOrSim engine.
	\pp This BodyAttribute is intended for use in cases where you want to maintain the orientation of the 
	torso of a Body independent of its direction of movement. For example, you can use this 
	BodyAttribute to implement strafing movements, where the Body moves to the left or right while 
	looking straight ahead. See :USERGUIDE:"Strafing".
	\ingroup kyruntime_attributes */
class BodyTorsoOrientation: public BodyAttribute
{
public:
	KY_DECLARE_BODYATTRIBUTE(BodyTorsoOrientation)


public:
	/*!	\label_baseconstructor */
	BodyTorsoOrientation() {}
	
	/*!	\label_virtualdestructor */
	virtual ~BodyTorsoOrientation() {}
	
	/*!	Copies the state of the specified BodyTorsoOrientation instance to this. */
	virtual BodyAttribute& operator=(const BodyAttribute& attr);

	/*!	Sets the orientation of the torso of the character in the \gameOrSim engine. This value must be a normalized vector. */
	void SetTorsoOrientation(const Vec3f& orientation) { m_torsoOrientation = orientation; }

	/*!	Retrieves the orientation of the torso of the character in the \gameOrSim engine. */
	const Vec3f& GetTorsoOrientation() const { return m_torsoOrientation; }


private:
	Vec3f m_torsoOrientation;
};

} // namespace Kaim

#endif // KY_BODYTORSOORIENTATION_H
