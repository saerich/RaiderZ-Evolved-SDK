/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_perceptions */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_BODYINFO_HOSTILITY_H
#define KY_BODYINFO_HOSTILITY_H

#include <kyruntime/bodyinfomanagement/bodyinfo.h>
#include <kyruntime/bodyinfomanagement/bodyinfofilter.h>
#include <kyruntime/bodyinfomanagement/concretebodyinfo.inl>
#include <kyruntime/bodyinfomanagement/twobodiesinfo.inl>


namespace Kaim
{

class BodyInfoManager;

/*!	This implementation of BodyInfo computes and stores whether or not given pairs of \Bodies are hostile
	to each other. 
	\pp For each Body in its BodyProfile, the BodyInfo_Hostility class considers each other Body in the list of
	\BodyProfiles that you set by calling AddInputProfile(). It considers a pair of 
	\Bodies to be hostile only if both \Bodies have been assigned the BodyTeamSide
	class of BodyAttribute, and the BodyTeamSide valuesare different for the two
	\Bodies. If either Body has not been assigned BodyTeamSide, or if the BodyTeamSide value is the same for both 
	\Bodies, they are not considered hostile. 
	\pp If a pair of \Bodies is considered hostile, the value stored in the BodyInfo_Hostility for that pair of 
	\Bodies is #KY_TRUE. If a pair of \Bodies is considered not hostile, the value stored in the BodyInfo_Hostility
	for that pair of \Bodies is #KY_FALSE.
	\pp For more information on setting up and using BodyInfo classes and the BodyInfoManager, see :USERGUIDE:"Tracking Perception Data".
	\ingroup kyruntime_perceptions */
class BodyInfo_Hostility : public TwoBodiesInfo<KyBool>
{
public:
	KY_DECLARE_BODYINFO( BodyInfo_Hostility );


public:
	/*!	\label_baseconstructor */
	BodyInfo_Hostility(const char* name, BodyInfoManager& bodyInfoManager, BodyProfile& profile) :
		TwoBodiesInfo<KyBool>(name, bodyInfoManager, profile) {}

protected:
	KyBool ComputeInfo(const Body& body1, const Body& body2,
		KyBool& value, KyBool& finished);
};

} // namespace Kaim

#endif // KY_BODYINFO_HOSTILITY_H
