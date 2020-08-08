/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_perceptions */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_BODYINFO_SIGHT_H
#define KY_BODYINFO_SIGHT_H

#include <kyruntime/bodyinfomanagement/bodyinfo.h>
#include <kyruntime/bodyinfomanagement/bodyinfofilter.h>
#include <kyruntime/bodyinfomanagement/concretebodyinfo.inl>
#include <kyruntime/bodyinfomanagement/twobodiesinfo.inl>


namespace Kaim
{

class BodyInfo_Visibility;


/*!	This implementation of BodyInfo computes and stores whether or not other \Bodies can be seen: i.e. 
	whether or not the other Body lies within the cone of vision, and for which the direct line-of-sight 
	is not blocked by any obstacles.
	\pp This class requires the BodyInfo_Visibility class to also be configured for its BodyProfile. You must
	set an instance of class by calling SetDependentBodyInfo().
	\pp For each Body in its BodyProfile, the BodyInfo_Sight class considers each other Body in the list of
	\BodyProfiles that you set by calling AddInputProfile(). For each of these other \Bodies, it calls the InViewCone()
	function to determine whether or not the other Body lies within the view cone defined by the values stored in the
	BodyVisualAcuteness class of BodyAttribute assigned to the subject Body. If the subject Body has not been assigned 
	BodyVisualAcuteness, its cone of visibility is assumed to be unlimited. If the other Body is within the view cone, the
	BodyInfo_Visibility is then consulted to determine whether or not a clear line of sight exists to the other Body.  
	\pp If the other Body is within the view cone and the line of sight is not blocked by any obstacles, the value 
	stored in the BodyInfo_Sight for that pair of \Bodies is #KY_TRUE. Otherwise, the value stored in the BodyInfo 
	for that pair of \Bodies is #KY_FALSE.
	\pp For more information on setting up and using BodyInfo classes and the BodyInfoManager, see :USERGUIDE:"Tracking Perception Data".
	\ingroup kyruntime_perceptions */
class BodyInfo_Sight : public TwoBodiesInfo<KyBool>
{
public:
	KY_DECLARE_BODYINFO(BodyInfo_Sight);


public:
	/*!	\label_baseconstructor */
	BodyInfo_Sight(const char* name, BodyInfoManager& bodyInfoManager, 
		BodyProfile& profile): 
		TwoBodiesInfo<KyBool>(name, bodyInfoManager, profile), m_visibilityInfo(KY_NULL) {}

	/*!	Specifies the BodyInfo_Visibility instance that the BodyInfo_Sight will use. This instance must be set up to compute and
		record values for the same \BodyProfiles as this instance of BodyInfo_Sight. */
	void SetDependentBodyInfos(BodyInfo_Visibility&);

protected:
	virtual KyBool ComputeInfo(const Body& watcherBody, 
		const Body& observedEntity, KyBool& value, KyBool& finished);

protected:
	// related BodyInfo
	BodyInfo_Visibility* m_visibilityInfo;
};

} // namespace Kaim

#endif // KY_BODYINFO_SIGHT_H
