/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_attributes */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_BODYEYEPOSITION_H
#define KY_BODYEYEPOSITION_H

#include <kyruntime/core/bot.h>


namespace Kaim
{

/*!	This class indicates to \SDKName components the absolute position of the eyes of a character in the \gameOrSim engine. 
	\pp This value may be taken into account when computing whether or not a Body can see other 
	\Bodies and points in the world. For instance, the value stored in this class is taken into account by:
	-	The BodyInfo_Sight class, used by the BodyInfoManager to track the line of 
		sight between \Bodies. 
	-	The HidingTraversal::EnhancedBodyVisibility() function, used by the HidingTraversal to
		test whether or not a Bot using the HideAgent is visible to its dangerous \Bodies.
	\ingroup kyruntime_attributes */
class BodyEyePosition: public BodyAttribute
{
public:
	KY_DECLARE_BODYATTRIBUTE(BodyEyePosition)


public:
	/*!	\label_baseconstructor */
	BodyEyePosition() {}
	
	/*!	\label_virtualdestructor */
	virtual ~BodyEyePosition() {}
	
	/*!	Copies the state of the specified BodyEyePosition instance to this. */
	virtual BodyAttribute& operator=(const BodyAttribute& attr);

	/*!	Sets the coordinates of the eyes of the character in the \gameOrSim engine. */
	void SetEyePosition(const Vec3f& eyePosition) { m_eyePosition = eyePosition; }

	/*!	Retrieves the coordinates of the eyes of the character in the \gameOrSim engine. */
	const Vec3f& GetEyePosition() const { return m_eyePosition; }


private:
	Vec3f m_eyePosition;
};

} // namespace Kaim

#endif // KY_BODYEYEPOSITION_H
