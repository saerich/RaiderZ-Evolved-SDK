/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_perceptions */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_BODYINFO_NEARESTVISIBLEENEMY_H
#define KY_BODYINFO_NEARESTVISIBLEENEMY_H

#include <kyruntime/bodyinfomanagement/bodyinfo.h>
#include <kyruntime/bodyinfomanagement/bodyinfofilter.h>
#include <kyruntime/bodyinfomanagement/concretebodyinfo.inl>
#include <kyruntime/bodyinfomanagement/onebodyinfo.inl>


namespace Kaim
{

class BodyInfo_Distance;
class BodyInfo_Hostility;
class BodyInfo_Visibility;


/*!	This implementation of BodyInfo computes and stores the nearest other hostile Body that is visible: i.e.
	for which the direct line-of-sight is not blocked by any obstacles. 
	\pp This class requires the BodyInfo_Hostility, BodyInfo_Visibility and BodyInfo_Distance 
	classes to also be configured for its BodyProfile. You must set instances of these classes by calling
	SetDependentBodyInfos().
	\pp For each Body in its BodyProfile, the BodyInfo_NearestVisibleEnemy class considers each other Body in the list of
	\BodyProfiles that you set by calling AddExploredProfile(). Each of those other \Bodies is considered to be hostile only
	if the BodyInfo_Hostility is available for the pair of \Bodies, and the value of the BodyInfo_Hostility is #KY_TRUE.
	For each hostile Body, the BodyInfo_Visibility is then consulted to determine whether or not the line of sight to that
	Body is clear, and the BodyInfo_Distance is consulted to determine which Body is nearest.
	\pp The value stored is a pointer to the nearest hostile Body that can be seen, or #KY_NULL if no other 
	Body was found with the BodyInfo values described above.
	\pp For more information on setting up and using BodyInfo classes and the BodyInfoManager, see :USERGUIDE:"Tracking Perception Data".
	\ingroup kyruntime_perceptions */
class BodyInfo_NearestVisibleEnemy : public OneBodyInfo<Body*>
{
public:
	KY_DECLARE_BODYINFO(BodyInfo_NearestVisibleEnemy);


public:
	/*!	\label_baseconstructor */
	BodyInfo_NearestVisibleEnemy(const char* name, BodyInfoManager& bodyInfoManager, BodyProfile& profile) :
		OneBodyInfo<Body*>(name, bodyInfoManager, profile),
		m_distanceInfo(KY_NULL),
		m_hostilityInfo(KY_NULL),
		m_visibilityInfo(KY_NULL),
		m_temporaryNearest(KY_NULL),
		m_maxLoopCount(15) {}

	/*!	Specifies the BodyInfo_Distance, BodyInfo_Hostility and BodyInfo_Visibility instances that the BodyInfo_NearestVisibleEnemy will use.
		These instances must be set up to compute and record values for the same \BodyProfiles as this instance of BodyInfo_NearestVisibleEnemy. */
	void SetDependentBodyInfos(BodyInfo_Distance&, BodyInfo_Hostility&, BodyInfo_Visibility&);

	// Accessors for MaxLoopCount
	/*!	Determines the maximum number of other \Bodies from the #m_exploredProfiles list that will be checked in
		each frame when computing this BodyInfo. After this limit is reached, the computation is paused temporarily,
		to allow other BodyInfo classes access to CPU resources. The computation is resumed where it left off in the
		following frame.
		\defaultvalue	\c 15 */
	void SetMaxLoopCount(const KyUInt32 count)
	{
		KY_FUNCTION("BodyInfo_NearestVisibleEnemy::SetMaxLoopCount");

		KY_ASSERT(count != 0, ("Invalid parameter"));
		m_maxLoopCount = count;
	}
	
	/*!	Retrieves the value set through a call to SetMaxLoopCount(). */
	KyUInt32 GetMaxLoopCount() const {return m_maxLoopCount;}

protected:
	virtual KyBool ComputeInfo(const Body& workingEntity, Body*& value,
		KyBool& finished);

	virtual void OnRemoveBody(const Body& body);


protected:
	// related BodyInfos
	BodyInfo_Distance* m_distanceInfo;
	BodyInfo_Hostility* m_hostilityInfo;
	BodyInfo_Visibility* m_visibilityInfo;

	// Computation Intercall data
	Body* m_temporaryNearest;
	KyFloat32 m_temporaryBestDistance;
	KyUInt32 m_maxLoopCount; // Max loop count for computation slicing
};

} // namespace Kaim

#endif // KY_BODYINFO_NEARESTVISIBLEENEMY_H
