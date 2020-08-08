/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_agents */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_GOTOAGENT_H
#define KY_GOTOAGENT_H

#include <kyruntime/pathfinding/pathfinder.h>


////////////////////////////////////////////////////////////////////////////////
//		GotoAgent
////////////////////////////////////////////////////////////////////////////////


namespace Kaim
{

class PointLockManager;


/*!	The GotoAgent moves a Bot to a specified position in the terrain, moving at a specified speed. 
	It is essentially a thin wrapper around the PathFinder.
	\pp For details on setting up and using this class, see :USERGUIDE:"Moving to a Preset Location".
	\ingroup kyruntime_agents */
class GotoAgent
{
	KY_DEFINE_NEW_DELETE_OPERATORS

	GotoAgent operator= (const GotoAgent& );

public:
	//////////////////////////////////////////////////////////////////////////////////////////
	// STANDARD AGENT FUNCTIONS
	//////////////////////////////////////////////////////////////////////////////////////////

	/*! \label_constructor \label_mustinit */
	GotoAgent();
	
	/*! \label_destructor */
	~GotoAgent();

	/*! \label_init 
		\param bot					The Bot that will use the GotoAgent.
		\param pointLockManager		The PointLockManager configured for the World. Optional. */
	KyResult Initialize(Bot& bot, PointLockManager* pointLockManager=KY_NULL);

	/*! \label_clear \label_mustinit */
	void Clear();

	/*!	Calls the PathFinder to move the Bot toward the destination set in the last call to SetDestination().
		If a PointLockManager has been set in the call to Initialize(), the GotoAgent also locks that 
		destination point automatically. */
	KyBool Think();


	//////////////////////////////////////////////////////////////////////////////////////////
	// SPECIFIC FUNCTIONS
	//////////////////////////////////////////////////////////////////////////////////////////

	/*!	Sets the current destination point for the GotoAgent. */
	void SetDestination(const Kaim::Vec3f& destination);


	//////////////////////////////////////////////////////////////////////////////////////////
	// SPEED MANAGEMENT
	//////////////////////////////////////////////////////////////////////////////////////////

	/*!	Determines the preferred speed at which the GotoAgent should move. This preferred speed is capped to
		the value of the BodyMaxSpeed attribute set for the Bot using the GotoAgent, if any. In 
		addition, it may be altered by the current set of IModifier classes used by the PathFinder.
		\units			meters per second
		\acceptedvalues	any positive value
		\defaultvalue	The value of the BodyMaxSpeed attribute of the Bot if available, or \c 1.0f otherwise. */
	void SetSpeed(KyFloat32 value);

	/*!	Retrieves the value set through a call to SetSpeed(). */
	KyFloat32 GetSpeed() const { return m_speed; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// AGENT STATE FEEDBACK
	//////////////////////////////////////////////////////////////////////////////////////////

	/*!	Retrieves the current destination point set for the GotoAgent. */
	const Vec3f& GetDestination() const { return m_targetPos; }


private:
	Bot* m_bot;
	PointLockManager* m_pointLockManager;	//< Pointer to point lock manager world service
	Vec3f m_targetPos;		//< Destination position
	KyFloat32 m_speed;				//< The speed at which entity goes (stored in meters per second)
};

} // Kaim

#endif // KY_GOTOAGENT_H
