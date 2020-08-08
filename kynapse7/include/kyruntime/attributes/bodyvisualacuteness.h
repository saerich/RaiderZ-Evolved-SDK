/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_attributes */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_BODYVISUALACUTENESS_H
#define KY_BODYVISUALACUTENESS_H

#include <kyruntime/core/bot.h>

#include <kypathdata/basesystem/config.h>


namespace Kaim
{

/*!	This class indicates to \SDKName components the maximum extents of the cone of vision of a character
	in the \gameOrSim engine, defined by the intersection of the following areas:
	-	A circle around the Body, whose radius is equal to the maximum distance the Body can see, 
		expressed in meters.
	-	The angle of the cone of vision, expressed as a number of degrees on either side of 
		the facing orientation of the Body. For example, a value of 180 indicates that the Body can see 
		in all directions. Typically, the cone of vision is set to a value between 0 and 90.
	\pp If either the maximum distance or the cone of vision is set to 0, the Body cannot see at all. If 
	the BodyVisualAcuteness class is not assigned to a Body at all, the Body is considered to be 
	able to see infinitely far in all directions.
	\pp This BodyAttribute may be taken into account when computing whether or not the Body can see other 
	\Bodies and points in the world. For instance, this BodyAttribute is taken into account by the 
	BodyInfo_Sight class of BodyInfo, to track the cones of vision of different \Bodies. 
	\ingroup kyruntime_attributes */
class BodyVisualAcuteness: public BodyAttribute
{
public:
	KY_DECLARE_BODYATTRIBUTE(BodyVisualAcuteness)


public:
	/*!	\label_baseconstructor */
	BodyVisualAcuteness(): m_visibilityDistance(100.f), m_visibilityAngle(60.f) {}
	
	/*!	\label_virtualdestructor */
	virtual ~BodyVisualAcuteness() {}
	
	/*!	Copies the state of the specified BodyVisualAcuteness instance to this. */
	virtual BodyAttribute& operator=(const BodyAttribute& attr);

	/*!	Sets the maximum distance the character in the \gameOrSim engine is able to see, in meters. */
	void SetVisibilityDistance(KyFloat32 distance)
	{
		KY_FUNCTION("BodyVisualAcuteness::SetVisibilityDistance");

		KY_AFFECT_POSITIVE_FLOAT("VisibilityDistance", m_visibilityDistance, distance);
	}

	/*!	Sets the maximum angle of the character's cone of vision relative to its facing orientation.
		A value of 180 makes the character able to see in all directions. */
	void SetVisibilityAngle(KyFloat32 angle);

	/*!	Retrieves the maximum distance the character in the \gameOrSim engine is able to see, in meters. */
	KyFloat32 GetVisibilityDistance() const { return m_visibilityDistance; }

	/*!	Retrieves the maximum angle of the character's cone of vision relative to its facing orientation. */
	KyFloat32 GetVisibilityAngle() const { return m_visibilityAngle; }


private:
	KyFloat32 m_visibilityDistance; //< Distance to which I can see, in meters
	KyFloat32 m_visibilityAngle; //< Angle between the AXIS and the "border" of the cone (so 0 < angle < 90)
};

} // namespace Kaim

#endif // KY_BODYVISUALACUTENESS_H
