/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_agents */

// primary contact: JODA - secondary contact: LAPA
#ifndef KY_TESTAGENT_H
#define KY_TESTAGENT_H

#include <kypathdata/math/vec3f.h>
#include <kyruntime/core/bot.h>

namespace Kaim
{

class ActionAcceleration;
class ActionRotate;
class ActionSpeed;
class ActionSteering;

/*! The TestAgent makes its Bot follow a set of pre-determined movements intended to validate
	several points of integration between \SDKName and your \gameOrSim engine. This includes:
	-	The interfaces provided by the Bridge.
	-	The data flow that updates your Bot with current information about the state of the corresponding character in the
		\gameOrSim engine, and that updates the character in the \gameOrSim engine to apply the results of the Action
		calculated by the Bot.
	\pp The TestAgent indicates the success or failure of its tests through a combination of the movements of your Bot
	in the \gameOrSim engine, debug information drawn in the \gameOrSim engine, and informational messages
	written to the \SDKName log.
	\pp For details on setting up the TestAgent and interpreting the results of its tests,
	see :USERGUIDE:"Integration Phase 3: Adding Bots and Bodies".
	\ingroup kyruntime_agents */
class TestAgent
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	class TestPredictActionMove;
	class TestAction;

public:
	/*! \label_constructor \label_mustinit */
	TestAgent();

	/*! \label_destructor */
	~TestAgent();

	/*! \label_init
		\param bot			The Bot that will use the TestAgent. */
	KyResult Initialize(Bot& bot);

	/*! Makes the TestAgent carry out its preset list of movements. */
	KyBool Think();

	/*! Returns the name of the test currently being conducted by the TestAgent, or "Finish" if
		all tests have been completed. */
	const char* GetState() const;

protected:

	void DrawState() const;

	/* Called at initialization time to make sure that the Action class assigned to the Bot contains
		the necessary Action Attributes. */
	KyResult CheckInitActionIntegrity() const;

	/* Called at each frame to test whether or not the Bot correctly updates its Body Attributes. */
	KyBool CheckInput() const;

public:
	/*! Sets the speed of the Bot, in meters per second. Primarily intended for internal use during the
		course of the tests carried out by the TestAgent.
		\return #KY_TRUE if the current speed of the Bot is the same as the \c speed argument. */
	KyBool SetSpeed(KyFloat32 speed);

	/*! Sets the orientation of the Bot. Primarily intended for internal use during the course of the tests carried out by the TestAgent.
		\param angle		The desired angular orientation of the Bot, in degrees of absolute yaw. See :USERGUIDE:"Pitch, Roll and Yaw".
		\return #KY_TRUE if the current orientation of the Bot is the same as the \c angle argument. */
	KyBool SetOrientation(KyFloat32 angle);

	/*!	Retrieves the Body of the Bot using the TestAgent. */
	const Body& GetBody() const { return m_bot->GetBody(); }

	/*!	\label_getworld */
	World& GetWorld() { return m_bot->GetWorld(); }

protected:
	Bot* m_bot;
	KyUInt32 m_stateIndex;                      //< current state of the brain.
	Array<TestAction*> m_testActions;           //< Array of single action to do.
	ActionSpeed* m_speed;                       //< Bot Action attribute : speed
	ActionAcceleration* m_acceleration;         //< Bot Action attribute : acceleration
	ActionRotate* m_rotate;                     //< Bot Action attribute : rotation (direction)
	ActionSteering* m_steering;                 //< Bot Action attribute : steering
	TestPredictActionMove* m_predictActionMove; //< Class used to test if move seems to be correct.
	Vec3f m_lastPosition;                       //< Position of the bot in the previous frame.
};

} // namespace Kaim

#endif // KY_TESTAGENT_H
