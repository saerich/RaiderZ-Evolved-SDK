/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_bodyinfomanagement */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_FRUSTUMFILTER_H
#define KY_FRUSTUMFILTER_H

#include <kyruntime/bodyinfomanagement/bodyinfofilter.h>

#include <kypathdata/basesystem/config.h>


namespace Kaim
{
	/////////////////////////////////////////////////////////////////////////////////
	// FrustumFilter
	/////////////////////////////////////////////////////////////////////////////////

	/*!	This implementation of BodyInfoFilter blocks the BodyInfoManager from computing \BodyInfos
		for any \Bodies that are outside a conical frustum: a truncated three-dimensional cone defined
		by an apex position, an axis vector, a near clip distance, a far clip distance, and an angle of
		width.
		\pp The following image shows the relevant configuration parameters, and how their values are
		taken together to define the frustum (shown by the green area).
		\pp \image html "graphics\frustumfilter.png"
		\ingroup kyruntime_bodyinfomanagement */
	class FrustumFilter : public BodyInfoFilter
	{
	public:
		KY_DECLARE_BODYINFOFILTER(FrustumFilter)

		/*! \label_baseconstructor
			\param name			A unique name for the FrustumFilter, which is stored in the #m_name member. This name
								must be unique among all \BodyInfoFilters assigned to the same EntityProfile.
		*/
		FrustumFilter(const char* name);

		/*! \label_virtualdestructor */
		~FrustumFilter() {}

		// ********************** Data accessor functions **********************

		/*! Write accessor for the \c EyePosition of the frustum. See the image in the class description above. */
		void SetEyePosition(const Vec3f& position) {m_eyePos = position;}

		/*! Read accessor for the \c EyePosition of the frustum. See the image in the class description above. */
		const Vec3f& GetEyePosition() const { return m_eyePos;}

		/*! Write accessor for the \c At position of the frustum. See the image in the class description above. */
		void SetAt(const Vec3f& at)
		{
			KY_FUNCTION("FrustumFilter::SetLookAt");
			KY_ASSERT(at.IsZero() == false, ("The 'Look at' direction should not be null"));

			at.GetNormalized(m_at);
		}

		/*! Read accessor for the \c At position of the frustum. See the image in the class description above. */
		const Vec3f& GetAt() const { return m_at;}

		/*! Write accessor for the \c NearClip and \c FarClip positions of the frustum. See the image in the class description above. */
		void SetClipping(const KyFloat32 nearClip, const KyFloat32 farClip)
		{
			KY_FUNCTION("FrustumFilter::SetClipping");
			KY_ASSERT(nearClip < farClip, ("nearClip (%f) should be less than farClip (%f)", nearClip, farClip));

			m_near = nearClip;
			m_far = farClip;
		}

		/*! Read accessor for the \c NearClip and \c FarClip positions of the frustum. See the image in the class description above. */
		void GetClipping(KyFloat32& nearClip, KyFloat32& farClip) const
		{
			nearClip = m_near;
			farClip = m_far;
		}

		/*! Write accessor for the \c Angle of the frustum. See the image in the class description above. */
		void SetAngle(const KyFloat32 angleDeg) {
			KY_FUNCTION("FrustumFilter::SetAngle");

			KY_AFFECT_FLOAT_IN_RANGE("Angle (in degree)", 0.f, 90.f, m_angleDeg, angleDeg);
		}

		/*! Read accessor for the \c Angle of the frustum. See the image in the class description above. */
		KyFloat32 GetAngle() const {return m_angleDeg;}

		// ********************** Filtering functions **********************

		/*! \copydoc BodyInfoFilter::ForbidComputation(BodyInfo&)
			\pp This implementation of BodyInfoFilter::ForbidComputation(BodyInfo&) simply returns #KY_FALSE. */
		virtual KyBool ForbidComputation(BodyInfo& /*info*/) { return KY_FALSE; }

		/*! \copydoc BodyInfoFilter::ForbidComputation(BodyInfo&, const Body&)
			\pp This implementation of BodyInfoFilter::ForbidComputation(BodyInfo&, const Body&)
			forbids the computation of the specified BodyInfo for the specified Body if that Body is
			outside of the frustum set for this instance of FrustumFilter. */
		virtual KyBool ForbidComputation(BodyInfo& info, const Body& entity1);

		/*! \copydoc BodyInfoFilter::ForbidComputation(BodyInfo&, const Body&, const Body&)
			\pp This implementation of BodyInfoFilter::ForbidComputation(BodyInfo&, const Body&, const Body&)
			simply returns #KY_FALSE. Note that it does not forbid a computation between the two specified \Bodies
			even if the second Body is outside of the frustum set for this instance of FrustumFilter. */
		virtual KyBool ForbidComputation(BodyInfo& /*info*/, const Body& /*body1*/, const Body& /*body2*/) { return KY_FALSE; }

	protected:
		Vec3f m_eyePos; // Eye position
		Vec3f m_at; // "At" vector
		KyFloat32 m_near; // Clipping values
		KyFloat32 m_far; // Clipping values
		KyFloat32 m_angleDeg; // Cone angle
	};

} // namespace Kaim

#endif //KY_FRUSTUMFILTER_H
