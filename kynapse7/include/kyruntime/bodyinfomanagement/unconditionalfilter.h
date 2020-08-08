/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_bodyinfomanagement */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_UNCONDITIONALFILTER_H
#define KY_UNCONDITIONALFILTER_H

#include <kyruntime/bodyinfomanagement/bodyinfofilter.h>


namespace Kaim
{
	/////////////////////////////////////////////////////////////////////////////////
	// UnconditionalFilter
	/////////////////////////////////////////////////////////////////////////////////

	/*! This implementation of BodyInfoFilter blocks the BodyInfoManager Service from computing
		all \BodyInfos for all \Bodies at all times, unconditionally.
		\ingroup kyruntime_bodyinfomanagement */
	class UnconditionalFilter : public BodyInfoFilter
	{
	public:
		KY_DECLARE_BODYINFOFILTER(UnconditionalFilter)

	public:
		/*! \label_baseconstructor
			\param name			A unique name for the UnconditionalFilter, which is stored in the #m_name member. This name
								must be unique among all \BodyInfoFilters assigned to the same EntityProfile.
		*/
		UnconditionalFilter(const char* name):BodyInfoFilter(name) {}

		/*!	\label_virtualdestructor */
		~UnconditionalFilter() {}

		/*!	\copydoc BodyInfoFilter::ForbidComputation(BodyInfo&)
			\pp This implementation of BodyInfoFilter::ForbidComputation(BodyInfo&) simply returns #KY_TRUE, to
			forbid all BodyInfo computations for all \Bodies. */
		virtual KyBool ForbidComputation(BodyInfo& /*info*/) { return KY_TRUE; }

		/*!	\copydoc BodyInfoFilter::ForbidComputation(BodyInfo&, const Body&)
			This implementation of BodyInfoFilter::ForbidComputation(BodyInfo&, const Body&) simply returns 
			#KY_TRUE, to forbid all BodyInfo computations for the specified Body. */
		virtual KyBool ForbidComputation(BodyInfo& /*info*/, const Body& /*body1*/) { return KY_TRUE; }

		/*!	\copydoc BodyInfoFilter::ForbidComputation(BodyInfo&, const Body&, const Body&)
			This implementation of BodyInfoFilter::ForbidComputation(BodyInfo&, const Body&, const Body&)
			simply returns #KY_TRUE, to forbid all BodyInfo computations for the specified pair of \Bodies. */
		virtual KyBool ForbidComputation(BodyInfo& /*info*/, const Body& /*body1*/,
			const Body& /*body2*/) { return KY_TRUE; }
	};

} // namespace Kaim

#endif // KY_UNCONDITIONALFILTER_H
