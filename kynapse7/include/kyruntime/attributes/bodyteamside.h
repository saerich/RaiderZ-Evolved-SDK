/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_attributes */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_BODYTEAM_H
#define KY_BODYTEAM_H

#include <kyruntime/core/bot.h>


namespace Kaim
{

/*!	This class indicates to \SDKName components the team affiliation of a character in the \gameOrSim
	engine, using an integer ID value. 
	\pp This BodyAttribute is used by:
	-	The ShootAgent, in order to determine whether the line of fire to its target
		contains any other non-hostile \Bodies. 
	-	The BodyInfo_Hostility class of BodyInfo, in order to determine whether or not a given Body should be
		considered hostile to another given Body.
	\pp An BodyTeamSide value of –1 indicates a neutral Body that is not considered part of any team, 
	and is not considered hostile to any other \Bodies. 
	\pp Note that this BodyAttribute is not directly related to the Team class or to cooperative 
	team behaviors, although you may use this BodyAttribute class in your high-level team behaviors 
	if you so choose. See :USERGUIDE:"Writing Cooperative Behaviors".
	\ingroup kyruntime_attributes */
class BodyTeamSide: public BodyAttribute
{
public:
	KY_DECLARE_BODYATTRIBUTE(BodyTeamSide)


public:
	/*!	\label_baseconstructor */
	BodyTeamSide(): m_teamSide(-1) {}
	
	/*!	\label_virtualdestructor */
	virtual ~BodyTeamSide() {}
	
	/*!	Copies the state of the specified BodyTeamSide instance to this. */
	virtual BodyAttribute& operator=(const BodyAttribute& attr);

	/*!	Sets the team ID assigned to the character in the \gameOrSim engine. */
	void SetTeamSide(KyInt32 entityTeam) { m_teamSide = entityTeam; }

	/*!	Retrieves the team ID assigned to the character in the \gameOrSim engine. */
	KyInt32 GetTeamSide() const { return m_teamSide; }


private:
	KyInt32 m_teamSide;
};

} // namespace Kaim

#endif // KY_BODYTEAM_H
