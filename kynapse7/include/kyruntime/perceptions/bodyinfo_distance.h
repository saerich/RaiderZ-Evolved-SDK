/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_perceptions */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_BODYINFO_DISTANCE_H
#define KY_BODYINFO_DISTANCE_H

#include <kyruntime/bodyinfomanagement/bodyinfo.h>
#include <kyruntime/bodyinfomanagement/bodyinfofilter.h>
#include <kyruntime/bodyinfomanagement/concretebodyinfo.inl>
#include <kyruntime/bodyinfomanagement/twobodiesinfo.inl>


namespace Kaim
{
/*!	This implementation of BodyInfo stores the distance between pairs of \Bodies.
	\pp For each Body in its BodyProfile, the BodyInfo_Distance class considers each other Body in the list of
	\BodyProfiles that you set by calling AddInputProfile().
	\pp The value stored in the BodyInfo_Distance is a KyFloat32 that expresses the distance between the pair of
	\Bodies in meters.
	\pp For more information on setting up and using BodyInfo classes and the BodyInfoManager, see :USERGUIDE:"Tracking Perception Data".
	\ingroup kyruntime_perceptions */
class BodyInfo_Distance : public TwoBodiesInfo<KyFloat32>
{
public:
	KY_DECLARE_BODYINFO( BodyInfo_Distance );


public:
	/*!	\label_baseconstructor */
	BodyInfo_Distance(const char* name, BodyInfoManager& bodyInfoManager, BodyProfile& profile):
		TwoBodiesInfo<KyFloat32>(name, bodyInfoManager, profile) {}

protected:
	KyBool ComputeInfo(const Body& body1, const Body& body2,
		KyFloat32& value, KyBool& finished);
};

} // namespace Kaim

#endif // KY_BODYINFO_DISTANCE_H
