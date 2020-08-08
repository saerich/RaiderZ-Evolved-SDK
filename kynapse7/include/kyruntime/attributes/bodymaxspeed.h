/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_attributes */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_BODYMAXSPEED_H
#define KY_BODYMAXSPEED_H

#include <kyruntime/core/bot.h>

#include <kypathdata/basesystem/config.h>


namespace Kaim
{

/*!	This class indicates to \SDKName components the maximum speed at which a character in the \gameOrSim
	engine can move, in meters per second.
	\pp The value of this BodyAttribute is used by most agent classes supplied in the \SDKName SDK as the default
	speed for the Body. 
	\ingroup kyruntime_attributes */
class BodyMaxSpeed: public BodyAttribute
{
public:
	KY_DECLARE_BODYATTRIBUTE(BodyMaxSpeed)


public:
	/*!	\label_baseconstructor */
	BodyMaxSpeed(): m_maxSpeed(0.5f) {}
	
	/*!	\label_virtualdestructor */
	virtual ~BodyMaxSpeed() {}
	
	/*!	Copies the state of the specified BodyMaxSpeed instance to this. */
	virtual BodyAttribute& operator=(const BodyAttribute& attr);

	/*!	Sets the maximum speed of the character in the \gameOrSim engine, in meters per second. */
	void SetMaxSpeed(KyFloat32 maxSpeed)
	{
		KY_FUNCTION("BodyMaxSpeed::SetMaxSpeed");

		KY_AFFECT_POSITIVE_FLOAT("MaxSpeed", m_maxSpeed, maxSpeed);
	}

	/*!	Retrieves the maximum speed of the character in the \gameOrSim engine, in meters per second. */
	KyFloat32 GetMaxSpeed() const { return m_maxSpeed; }


private:
	KyFloat32 m_maxSpeed;
};

} // namespace Kaim

#endif // KY_BODYMAXSPEED_H
