/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_core */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_POINTINFO_BODY_H
#define KY_POINTINFO_BODY_H

#include <kyruntime/core/pointinfo.h>


namespace Kaim
{

class Body;


/*!	PointInfo_Body is an implementation of PointInfo that stores a pointer to
	a Body. This pointer is typically used to identify the Body that
	occupies the position stored in the PointWrapper. */
class PointInfo_Body: public PointInfo
{
public:
	KY_DECLARE_POINTINFO(PointInfo_Body)

	/*!	\label_constructor Points to #KY_NULL. */
	PointInfo_Body(): m_body(KY_NULL) {}

	/*!	\label_constructor
		\param body					The Body to set for the PointInfo. */
	PointInfo_Body(const Body* body): m_body(body) {}

	/*!	Resets the PointInfo to point to #KY_NULL. */
	virtual void Reset() { m_body = KY_NULL; }

	/*!	Retrieves the Body pointed to by this instance of PointInfo_Body.
		\param[out] body 			Used to store the pointer to the attached Body.
		\return #KY_TRUE */
	virtual KyBool GetBody(const Body* &body) const
	{
		body = m_body;
		return KY_TRUE;
	}

	/*!	Sets the Body pointed to by this instance of PointInfo_Body.
		\param body 				A pointer to the Body. */
	virtual void SetBody(const Body* body) { m_body = body; }


protected:
	const Body* m_body;
};

} // namespace Kaim

#endif // KY_POINTINFO_BODY_H
