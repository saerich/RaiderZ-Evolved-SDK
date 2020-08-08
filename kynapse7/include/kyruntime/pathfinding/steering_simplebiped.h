/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_STEERING_SIMPLEBIPED_H
#define KY_STEERING_SIMPLEBIPED_H

#include <kyruntime/pathfinding/isteering.h>

#include <kypathdata/math/trigo.h>


namespace Kaim
{
/*!	This implementation of the ISteering interface is appropriate for use with most biped \Bots. 
	It writes the following output information to direct your Bot along its path: 
	-	A target forward speed. See \ref steeringbipedspeed.
	-	A target orientation. See \ref steeringbipedrotation.
	-	A facing orientation for characters able to strafe. See \ref steeringbipedstrafing.
	It applies the following constraints to the target speed and target orientation computed earlier 
	in the pathfinding process. 
	\section steeringbipedspeed Speed management
	\pp In order to make abrupt changes of direction less jarring, this modifier reduces the target speed
	of the PathFinder in proportion to the difference between the current orientation of the Bot and the
	orientation required in order for the Bot to move toward its target point.
	\pp The Steering_SimpleBiped class outputs the speed value in one of the following attributes:
	-	If your Bot has been assigned the ActionSpeed attribute, it is updated to indicate the 
		target speed that your character should move in the current frame.
	-	If your Bot has been assigned the ActionAcceleration attribute, it is updated to indicate whether
		your Bot should slow down (a value of \c -1) or speed up (a value of \c 1).
	\pp Note that this speed is in two dimensions, along the X,Y plane. Typically, your gameOrSim engine 
	needs a way to keep walking characters clamped to the actual altitude of the ground using a physics
	engine or a height map. KynProductName itself does not manage the altitude of characters that travel
	along the ground. The altitude information maintained in the NavMesh is typically not sufficiently 
	precise to guarantee good placement for your characters in all situations. 
	\section steeringbipedrotation
	The Steering_SimpleBiped class outputs the orientation value in one of the following attributes:
	-	If your Bot has been assigned the ActionRotate attribute, it is updated to indicate the target
		orientation that your character should reach, expressed in degrees of absolute yaw. For details
		on absolute yaw in the \SDKName coordinate system, see :USERGUIDE:"Pitch, Roll and Yaw".
	-	If your Bot has been assigned the ActionSteering attribute, it is updated to indicate whether the
		character should turn left (a value of \c -1) or right (a value of \c 1).
	\section steeringbipedstrafing Strafing
	If the Bot is able to strafe, this modifier will update the ActionTorsoRotate attribute to indicate a 
	facing direction for the character that is different from its direction of movement under certain conditions.
	\subsection steeringbipedstrafingreqs Strafing requirements
	In order for the Steering_SimpleBiped class to make your Bot strafe, your Bot must be assigned:
	-	The BodyTorsoOrientation class of BodyAttribute, which provides \SDKName with the current facing
		direction of your character. At each frame, before you launch the \SDKName update sequence, you must
		update this attribute with the current facing direction of your character in degrees of absolute yaw.
	-	The ActionTorsoRotate class of ActionAttribute, which indicates the target facing direction for your
		character determined by Steering_SimpleBiped. At each frame, when you apply the Action 
		of your Bot to the character in your \gameOrSim engine, if this attribute has been updated, you must
		retrieve its value and make your \gameOrSim engine character face the target rotation. The value is 
		expressed in degrees of absolute yaw.
 	\pp For details on absolute yaw in the \SDKName coordinate system, see :USERGUIDE:"Pitch, Roll and Yaw".
	\subsection steeringbipedstrafingcond Strafing conditions
	The Steering_SimpleBiped class will make your Bot strafe any time both of the following conditions are true:
	-	The angle between the current facing direction of the Bot and the target point is greater than the 
		value set through a call to StrafeAngle(KyFloat32). The current orientation of the Bot is taken 
		from the value of its BodyTorsoOrientation attribute, if defined; otherwise, by calling Bot::GetOriVector(). 
	-	The distance between the current position of the Bot and the position of the target point is less 
		than the value set through a call to StrafeRadius(KyFloat32). 
	\ingroup kyruntime_pathfinding */
class Steering_SimpleBiped: public ISteering
{
public:
	KY_DECLARE_STEERING(Steering_SimpleBiped)


public:
	/*!	\label_constructor */
	Steering_SimpleBiped():
		ISteering(),
		m_paramStrafeRadius(2.0f),
		m_paramStrafeCosAngle(cosf(5.0f*KY_PI_DIVIDED_BY_180)),
		m_strafing(KY_FALSE)
	{}

	virtual void ReInit();

	virtual KyBool ComputeAction(const Bot& entity,  KyFloat32 maxSpeed,
		const PointWrapper& targetPoint,  KyFloat32 targetSpeed,
		const Vec3f* targetOrientation, Action& action);

	virtual KyBool GetTime(const Bot& entity, const PointWrapper& startingPosition, 
		KyFloat32 startingSpeed, const Vec3f& startingOrientation, KyFloat32 maxSpeed, 
		const PointWrapper& targetPoint, KyFloat32& targetSpeed,
		const Vec3f* targetOrientation, KyFloat32& time);


	// Strafe parameter accessors
	/*! Retrieves the value set through a call to StrafeRadius(KyFloat32). */
	KyFloat32 StrafeRadius() const { return m_paramStrafeRadius; }

	/*! Determines the maximum distance that the target point may be from the Bot in order for the Bot to move to that point
		by strafing instead of by turning and moving straight ahead. If the target point is closer to the Bot than this value,
		and it lies at an angle from the current orientation of the Bot that is greater than that specified by StrafeAngle(), 
		the Bot will strafe to the point.
		\units			meters
		\acceptedvalues	any positive value
		\defaultvalue	\c 2.0f */
	void StrafeRadius(KyFloat32 val)
	{
		KY_FUNCTION("Steering_SimpleBiped::StrafeRadius");
		KY_ASSERT(val >= 0.f, ("Invalid value (%.2f), 'StrafeRadius' must be >= 0.", val));
		m_paramStrafeRadius = val;
	}
	
	
	/*! Retrieves the value set through a call to StrafeAngle(KyFloat32). */
	KyFloat32 StrafeAngle() const { return (acosf(m_paramStrafeCosAngle)/KY_PI_DIVIDED_BY_180); }

	/*! Determines the minimum angle that the target point may be offset from the current orientation of the Bot in order for
		the Bot to move to that point by strafing instead of by turning and moving straight ahead. If the target point is
		utside of this cone, and closer to the Bot than the StrafeRadius() value, the Bot will strafe to the point.
		\units			degrees
		\acceptedvalues	any positive value less than \c 180.0
		\defaultvalue	\c 5.0f */
	void StrafeAngle(KyFloat32 val)
	{
		KY_FUNCTION("Steering_SimpleBiped::StrafeAngle");
		KY_ASSERT(val >= 0.0f && val <= 180.0f, ("Invalid value (%.2f), 'StrafeAngle' must be be an angle in degrees in the range [0, 180]", val));

		m_paramStrafeCosAngle = cosf(val*KY_PI_DIVIDED_BY_180);
		if (m_paramStrafeCosAngle > 1.0f)
			m_paramStrafeCosAngle = 1.0f;
		else if (m_paramStrafeCosAngle < -1.0f)
			m_paramStrafeCosAngle = -1.0f;
	}
	


protected:
	virtual void OnSetPathFinder() {}

	KyFloat32 m_paramStrafeRadius;
	KyFloat32 m_paramStrafeCosAngle;
	KyBool m_strafing;
};

} // namespace Kaim

#endif // KY_STEERING_SIMPLEBIPED_H
