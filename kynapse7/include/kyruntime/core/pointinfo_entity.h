/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_core */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_POINTINFO_BOT_H
#define KY_POINTINFO_BOT_H

#include <kyruntime/core/pointinfo.h>


namespace Kaim
{

class Bot;


/*!	PointInfo_Bot is an implementation of PointInfo that stores a pointer to
	a Bot. This pointer is typically used to identify the Bot that
	occupies the position stored in the PointWrapper. */
class PointInfo_Bot: public PointInfo
{
public:
	KY_DECLARE_POINTINFO(PointInfo_Bot)

	/*!	\label_constructor Points to #KY_NULL. */
	PointInfo_Bot(): m_bot(KY_NULL) {}

	/*!	\label_constructor
		\param bot					The Bot to set for the PointInfo. */
	PointInfo_Bot(const Bot* bot): m_bot(bot) {}

	/*!	Resets the PointInfo to point to #KY_NULL. */
	virtual void Reset() { m_bot = KY_NULL; }

	/*!	Retrieves the Bot pointed to by this instance of PointInfo_Bot.
		\param[out] bot 			Used to store the pointer to the attached Bot.
		\return #KY_TRUE */
	virtual KyBool GetBot(const Bot* &bot) const
	{
		bot = m_bot;
		return KY_TRUE;
	}

	/*!	Sets the Bot pointed to by this instance of PointInfo_Bot.
		\param bot 				A pointer to the Bot. */
	virtual void SetBot(const Bot* bot) { m_bot = bot; }


protected:
	const Bot* m_bot;
};

} // namespace Kaim

#endif // KY_POINTINFO_BOT_H
