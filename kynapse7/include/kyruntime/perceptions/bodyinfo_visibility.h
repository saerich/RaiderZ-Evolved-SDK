/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_perceptions */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_BODYINFO_VISIBILITY_H
#define KY_BODYINFO_VISIBILITY_H

#include <kyruntime/bodyinfomanagement/bodyinfo.h>
#include <kyruntime/bodyinfomanagement/bodyinfofilter.h>
#include <kyruntime/bodyinfomanagement/concretebodyinfo.inl>
#include <kyruntime/bodyinfomanagement/twobodiesinfo.inl>


namespace Kaim
{
/*!	This implementation of BodyInfo computes and stores whether or not a clear line of sight exists between
	pairs of \Bodies, unblocked by any intervening obstacles in the \gameOrSim engine. 
	\pp For each Body in its BodyProfile, the BodyInfo_Visibility class considers each other Body in the list of
	\BodyProfiles that you set by calling AddInputProfile(). It uses the ICollisionBridge
	interface to conduct a collision test between each pair of \Bodies. For any Body that has been assigned 
	the BodyEyePosition class of BodyAttribute, the position stored in BodyEyePosition is used for the collision test; 
	for all other \Bodies, the Body::m_position value is used instead.  
	\pp If the line of sight is not blocked by any obstacles, the value stored in the BodyInfo_Visibility for that pair of 
	\Bodies is #KY_TRUE. If the line of sight collides with an obstacle, the value stored in the BodyInfo_Visibility
	for that pair of \Bodies is #KY_FALSE.
	\pp For more information on setting up and using BodyInfo classes and the BodyInfoManager, see :USERGUIDE:"Tracking Perception Data".
	\ingroup kyruntime_perceptions */
class BodyInfo_Visibility : public TwoBodiesInfo<KyBool>
{
public:
	KY_DECLARE_BODYINFO(BodyInfo_Visibility);


public:
	/*!	\label_baseconstructor */
	BodyInfo_Visibility(const char* name, 
		BodyInfoManager& bodyInfoManager, BodyProfile& entityProfile):
		TwoBodiesInfo<KyBool>(name, bodyInfoManager, entityProfile) {}

protected:
	virtual KyBool ComputeInfo(const Body& watcherBody, 
		const Body& observedEntity, KyBool& value, KyBool& finished);
};

} // namespace Kaim

#endif // KY_BODYINFO_VISIBILITY_H
