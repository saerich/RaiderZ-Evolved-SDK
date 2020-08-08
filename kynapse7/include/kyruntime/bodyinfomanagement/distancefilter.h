/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kyruntime_bodyinfomanagement */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_DISTANCEFILTER_H
#define KY_DISTANCEFILTER_H

#include <kyruntime/bodyinfomanagement/bodyinfofilter.h>
#include <kypathdata/math/trigo.h>

namespace Kaim
{

class Vec3f;


/////////////////////////////////////////////////////////////////////////////////
// DistanceFilter
/////////////////////////////////////////////////////////////////////////////////

/*! This implementation of BodyInfoFilter blocks the BodyInfoManager from computing \BodyInfos
	for any \Bodies that are outside a sphere defined by a center point and a radius. The center point may
	be either a specified reference position in the 3D world, or the position of a specified reference Bot.
	\ingroup kyruntime_bodyinfomanagement */
class DistanceFilter : public BodyInfoFilter
{
public:
	KY_DECLARE_BODYINFOFILTER(DistanceFilter)


public:
	/*! \label_baseconstructor
		\param name			A unique name for the DistanceFilter, which is stored in the #m_name member. This name
							must be unique among all BodyInfoFilter objects assigned to the same BodyProfile. */
	DistanceFilter(const char* name);

	/*! \label_virtualdestructor For internal use. */
	~DistanceFilter() {}


	// ********************** Data accessor functions **********************

	/*! Sets a reference position for the DistanceFilter, used as the center of the sphere within which
		\BodyInfos are permitted to be calculated. If there is already a reference Bot
		set when this method is called, the DistanceFilter will no longer take that Bot into account. */
	void SetReferencePoint(const Vec3f& position)
	{
		m_pos = position;
		m_usePoint = KY_TRUE;
	}
	
	/*! Retrieves the reference position used for filtering.
		\param[out] vector			Stores the reference position set for the DistanceFilter, if any
		\return #KY_TRUE if the \c vector parameter was successfully updated, or #KY_FALSE if no reference point has
				been set for the DistanceFilter. */
	KyBool GetReferencePoint(Vec3f& vector) const
	{
		if (m_usePoint)
		{
			vector = m_pos;
			return KY_TRUE;
		}
		return KY_FALSE;
	}

	/*! Sets a reference Bot for the DistanceFilter. The position of the Bot will be used as the center of
		the sphere within which \BodyInfos are permitted to be calculated. If there is already a reference
		position set when this method is called, the DistanceFilter will no longer take that position into account. */
	void SetReferenceBody(const Body& body)
	{
		m_body = &body;
		m_usePoint = KY_FALSE;
	}

	/*! Retrieves the reference Bot used for filtering.
		\param[out] body			Used to store the reference Bot set for the DistanceFilter, if any.
		\return #KY_TRUE if the \c entity parameter was successfully updated, or #KY_FALSE if no reference Bot has
				been set for the DistanceFilter. */
	KyBool GetReferenceBody(const Body* &body) const
	{
		if (!m_usePoint)
		{
			body = m_body;
			return KY_TRUE;
		}
		return KY_FALSE;
	}

	/*! Sets the radius within which the DistanceFilter allows \BodyInfos to be calculated. */
	void SetDistance(const KyFloat32 distance)
	{
		KY_FUNCTION("DistanceFilter::SetDistance");
		KY_ASSERT(distance > 0.f, ("Bad parameter (%f) : distance for filter should be positive.", distance));

		m_squareDistance = Square(distance);
	}

	/*! Returns the radius within which the DistanceFilter allows \BodyInfos to be calculated. */
	KyFloat32 GetDistance() const
	{
		return sqrtf(m_squareDistance);
	}


	// ********************** Filtering functions **********************

	/*! \copydoc BodyInfoFilter::ForbidComputation(BodyInfo&)
		\pp This implementation of BodyInfoFilter::ForbidComputation(BodyInfo&) simply returns #KY_FALSE. */
	virtual KyBool ForbidComputation(BodyInfo& /*info*/) { return KY_FALSE; }

	/*! \copydoc BodyInfoFilter::ForbidComputation(BodyInfo&, const Bot&)
		\pp This implementation of BodyInfoFilter::ForbidComputation(BodyInfo&, const Body&)
		forbids the computation of the specified BodyInfo for the specified Body if that Body is at a
		greater distance from the reference position or reference Body than the radius set for the
		DistanceFilter. */
	virtual KyBool ForbidComputation(BodyInfo& info, const Body& body1);

	/*! \copydoc BodyInfoFilter::ForbidComputation(BodyInfo&, const Body&, const Body&)
		\pp This implementation of BodyInfoFilter::ForbidComputation(BodyInfo&, const Body&, const Body&)
		simply returns #KY_FALSE. Note that it does not forbid a computation between the two specified \Bodies
		even if distance between the reference position and the second Body is more
		than the distance threshold set for the DistanceFilter. */
	virtual KyBool ForbidComputation(BodyInfo& /*info*/, const Body& /*body1*/, const Body& /*body2*/) { return KY_FALSE; }




protected:
	KyBool m_usePoint; /*!< Stores whether the filter uses a reference point (#KY_TRUE) or a reference Bot (#KY_FALSE). */
	const Body* m_body; /*!< Stores a pointer to the reference Bot, if any. */
	Vec3f m_pos; /*!< Stores the reference point, if any. */
	KyFloat32 m_squareDistance; /*!< Stores the radius within which the DistanceFilter allows \BodyInfos to be calculated. */
};

} // namespace Kaim

#endif //KY_DISTANCEFILTER_H
