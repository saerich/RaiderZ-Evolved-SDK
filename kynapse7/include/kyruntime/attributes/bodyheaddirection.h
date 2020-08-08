/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_attributes */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_BODYHEADDIRECTION_H
#define KY_BODYHEADDIRECTION_H

#include <kyruntime/core/bot.h>


namespace Kaim
{

/*!	This class indicates to \SDKName components the facing direction of the head of a character in the
	\gameOrSim engine (and therefore the direction of its eyes). 
	\pp This BodyAttribute is intended for use in cases where you want to maintain the head orientation 
	and visual plane of a Body distinct from its direction of movement and/or its torso orientation. 
	\pp This BodyAttribute may be taken into account when computing whether or not the Body can see other 
	\Bodies and points in the world. For instance, this BodyAttribute is used by the BodyInfo_Sight 
	class to determine the cone of vision of different \Bodies. 
	\ingroup kyruntime_attributes */
class BodyHeadDirection: public BodyAttribute
{
public:
	KY_DECLARE_BODYATTRIBUTE(BodyHeadDirection)


public:
	/*!	\label_baseconstructor */
	BodyHeadDirection() {}
	
	/*!	\label_virtualdestructor */
	virtual ~BodyHeadDirection() {}
	
	/*!	Copies the state of the specified BodyHeadDirection instance to this. */
	virtual BodyAttribute& operator=(const BodyAttribute& attr);

	/*!	Sets the orientation of the head of the character in the \gameOrSim engine. This vector must be normalized. */
	void SetHeadDirection(const Vec3f& headDirection)
	{
		KY_FUNCTION("BodyHeadDirection::SetHeadDirection");

#ifdef KYDEBUG
		const float squareMagnitude = headDirection.GetSquareLength();
		const float epsilon = 0.0201f; // = (1 + 0.01f)^2 - 1;
		KY_ASSERT(fabs(squareMagnitude - 1.f) <= epsilon, ("Invalid value for parameter 'headDirection', should be normalized"));
#endif

		m_headDirection = headDirection;
	}

	/*!	Retrieves the orientation of the head of the character in the \gameOrSim engine. */
	const Vec3f& GetHeadDirection() const { return m_headDirection; }


private:
	Vec3f m_headDirection;
};

} // namespace Kaim

#endif // KY_BODY_HEADDIRECTION_H
