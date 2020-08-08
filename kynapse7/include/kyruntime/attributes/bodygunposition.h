/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_attributes */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_BODYGUNPOSITION_H
#define KY_BODYGUNPOSITION_H


#include <kyruntime/core/bot.h>


namespace Kaim
{

/*!	This class indicates to \SDKName components the position of a weapon held by a character in the \gameOrSim engine.
	\pp This BodyAttribute is used by the ShootAgent to determine whether or not it has a clear 
	line of fire to its target.
	\ingroup kyruntime_attributes */
class BodyGunPosition: public BodyAttribute
{
public:
	KY_DECLARE_BODYATTRIBUTE(BodyGunPosition)


public:
	/*!	\label_baseconstructor */
	BodyGunPosition() {}
	
	/*!	\label_virtualdestructor */
	virtual ~BodyGunPosition() {}
	
	/*!	Copies the state of the specified BodyGunPosition instance to this. */
	virtual BodyAttribute& operator=(const BodyAttribute& attr);

	/*!	Sets the coordinates of the weapon held by the character in the \gameOrSim engine. */
	void SetGunPosition(const Vec3f& gunPosition) { m_gunPosition = gunPosition; }

	/*!	Retrieves the coordinates of the weapon held by the character in the \gameOrSim engine. */
	const Vec3f& GetGunPosition() const { return m_gunPosition; }


private:
	Vec3f m_gunPosition;
};

} // namespace Kaim

#endif // KY_BODYGUNPOSITION_H
