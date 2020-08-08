/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_GOTO_REPULSORDYNAMICAVOIDANCE_H
#define KY_GOTO_REPULSORDYNAMICAVOIDANCE_H

#include <kyruntime/pathfinding/goto_trivial.h>


namespace Kaim
{
class BodyInfoManager;


/*!	This implementation of the IGoto interface is most suitable for use with large characters or 
	vehicles that have wide turning radii. 
	\pp You do not have to interact with the modifier at runtime at all. Its avoidance system is entirely
	autonomous. However, fine details of your Bot reactions can be adjusted through configuration parameters
	that you pass to the constructor. 
	\section repulsordaprocess The Repulsor dynamic avoidance process
	\pp At each frame, the solver carries out the following calculations for the Bot:
	-#	It builds a list of other \Bodies that the Bot might collide with in the near future.
	-#	It assigns a repulsor weight to each Body in the list, determined by multiple factors such 
		as the time and distance remaining before the Bot collides with that Body. 
	-#	It assigns an attractor weight to the final destination.
	-#	It integrates all attractors and repulsors in the nearby area to generate a new target point
		that gets the Bot toward its destination point while satisfying as much as possible the fields
		of the repulsors. The weight of each repulsor determines how much effect that item will have
		on the final path. So, for example, if a collision is imminent with a fast-moving nearby Body 
		and a slow-moving Body far away, the nearby Body will cause more deviation in the path of the
		Bot than the far-away Body.
	-#	It invokes the ISteering modifier used by the PathFinder to compute the Action that will move
		the Bot toward the new target point.
	\pp For example, in the image below, the Bot threads a path through the three repulsor fields to 
	reach the final destination as quickly as possible without collisions.
	\image html "graphics/dynamic_avoidance_repulsors.jpg"
	\section repulsorsetup Setting up Goto_RepulsorDynamicAvoidance
	Setting up this modifier involves the following considerations.
	\subsection recommendations Recommendations
	-	For best performance, set up the BodyInfoManager for your World. When the BodyInfoManager
		is available, Goto_RepulsorDynamicAvoidance queries it to determine which other
		\Bodies lie in close proximity to the subject Bot. If it is not available, 
		Goto_RepulsorDynamicAvoidance has to iterate through the list of all \Bodies in the
		World in order to find the ones that are near enough to affect the trajectory of the subject Bot.
	\ingroup kyruntime_pathfinding */
class Goto_RepulsorDynamicAvoidance: public Goto_Trivial
{
public:
	KY_DECLARE_GOTO(Goto_RepulsorDynamicAvoidance)


public:
	/*!	\label_constructor
		\param paramRatioAttractionRepulsion		Determines the ratio between attractors (the current goal of the Bot) and repulsors (other \Bodies to be avoided). 
													Higher values (above 0.5) produce smoother avoidance maneuvers, as the Bot assigns more importance to reaching its goal
													than to avoiding the repulsors. Lower values (below 0.5) produce tighter turns in the avoidance maneuvers, as the Bot 
													assigns more importance to avoiding other \Bodies than to reaching its destination.
													\acceptedvalues	any positive value
													\defaultvalue	\c 0.5f
		\param paramDelayTime						Determines the minimum time that a dynamic avoidance maneuver may last. If you notice your Bot moving in a jerky, jittery
													fashion, it may be switching back and forth too rapidly between standard movement mode and dynamic avoidance movement mode. 
													Tuning this parameter can help prevent this condition.
													\units			seconds
													\acceptedvalues	any positive value
													\defaultvalue	\c 0.1f
		\param paramTimeMinTrigger					Determines the minimum time interval required before an anticipated collision in order for the solver to attempt to find an
													avoidance path. If the time remaining before the anticipated collision is less than this value, the solver considers that it
													is too late to avoid the collision.
													\units			seconds
													\acceptedvalues	any negative or positive value
													\defaultvalue	\c -1.0f
		\param paramTimeMaxTrigger					Determines the maximum time interval possible before an anticipated collision in order for the solver to attempt to find an
													avoidance path. If the time remaining before the anticipated collision is greater than this value, the solver considers that
													it is too soon to begin avoiding the collision.
													\units			seconds
													\acceptedvalues	any negative or positive value
													\defaultvalue	\c 10.0f
		\param paramSlowSpeedFactor					Determines the ratio the solver applies to the BodyMaximumSpeed value when it needs a speed that it can consider slow.
													\acceptedvalues	between \c 0.0f and \c 1.0f inclusive
													\defaultvalue	\c 0.5f
		\param paramVerySlowSpeedFactor				Determines the ratio the solver applies to the BodyMaximumSpeed value when it needs a speed that it can consider very slow.
													\acceptedvalues	between \c 0.0f and \c 1.0f inclusive
													\defaultvalue	\c 0.25f
		\param paramDetectionDistanceRatio			This value is multiplied by the width of the Bot in order to determine a distance threshold outside of which all other \Bodies are ignored.
													\acceptedvalues	any positive value
													\defaultvalue	\c 5.0f
		\param paramCosThreshold					Sets the cosine of an angular threshold. While the angle between the orientation of movement of the Bot and the orientation of the
													target point relative to the Bot is below this threshold, the Bot prefers to alter its trajectory than to lower its speed.
													\acceptedvalues	any positive value
													\defaultvalue	\c 0.7f
		\param m_paramTimeToCollisionThreshold		Sets a time threshold. While the time to impact is below this threshold, the Bot prefers to alter its trajectory than to lower its speed.
													\units			seconds
													\acceptedvalues	any positive value
													\defaultvalue	\c 5.0f */
	Goto_RepulsorDynamicAvoidance(KyFloat32 paramRatioAttractionRepulsion = 0.5f,
		KyFloat32 paramDelayTime = 0.1f, KyFloat32 paramTimeMinTrigger = -1.f, KyFloat32 paramTimeMaxTrigger = 10.f, 
		KyFloat32 paramSlowSpeedFactor = 0.5f, KyFloat32 paramVerySlowSpeedFactor = 0.25f, 
		KyFloat32 paramDetectionDistanceRatio = 5.0f, KyFloat32 paramCosThreshold = 0.7f, 
		KyFloat32 m_paramTimeToCollisionThreshold = 5.0f);

	virtual void ReInit();

	/*!	\copydoc IGoto::Goto()
		\pp For details on this implementation of IGoto::Goto(), see the class description. */
	virtual KyBool Goto(const PointWrapper& targetPoint, Action& action);

	/*!	Defines a type of callback function that can be passed to Goto_RepulsorDynamicAvoidance
		to determine which other \Bodies should be taken into account. If you have \Bodies in
		your World that you do not want your Bot to avoid, you can write a function that satisfies this
		type definition and pass your function to SetFilteringCallback().
		\param thisBody			The Bot using Goto_RepulsorDynamicAvoidance.
		\param targetBody		Another Body that may or may not be taken into account by Goto_RepulsorDynamicAvoidance in
								its calculations.
		\return #KY_TRUE if \c thisBody should take \c targetBody into account in its computations, 
				or #KY_FALSE otherwise. */
	typedef KyBool (*FilteringFunction)(Body& thisBody, Body& targetBody);
	
	/*!	Sets a callback function that Goto_RepulsorDynamicAvoidance will invoke to determine
		which other \Bodies it should take into account in its computations. */
	void SetFilteringCallBack(FilteringFunction func) { m_filterCB = func; }
	
	/*!	Retrieves a callback function previously set in a call to SetFilteringCallBack(). */
	FilteringFunction GetFilteringCallBack() { return m_filterCB; }


protected:
	void ManageCollision(const Vec3f& dest);
	void ForecastCollision(const Vec3f& dest);
	void UpdateCollisionForecast(Body* targetBody, const Vec3f& dest);
	KyBool ComputeAction(Action& action, const Vec3f& targetPoint);


protected:
	FilteringFunction m_filterCB;

	virtual void OnSetPathFinder();

	// Bot informations
	Bot* m_me;
	BodyInfoManager* m_bodyInfoManager;
	KyFloat32 m_myWidth;
	KyFloat32 m_myHalfWidth;
	KyFloat32 m_myMaxSpeed;

	// Returned commands
	Vec3f m_torsoAngles;
	KyBool m_strafe;
	KyFloat32 m_speed;

	// runtime parameters
	KyBool m_collisionForecast;	//< Says if a collision will occur
	KyFloat32 m_timeCollision;	//< Time left before collision
	KyInt32 m_nbColliders;	//< number of entities in m_DA_tab_colliders
	#define REPULSOR_DA_NB_MAX_COLLIDERS 25
	Body* m_tabColliders[REPULSOR_DA_NB_MAX_COLLIDERS];	//< Body we could collide with
	Vec3f m_avoidingDirection ;	//< Direction to avoid collision
	KyBool m_inCollision;		//< True if we are during a delay collision time.
	KyFloat32 m_avoidLimitTime;	//< Time since last collision detected.
	KyFloat32 m_bodyDistanceMax; //< Distance above which the body is not considered.

	KyFloat32 m_paramRatioAttractionRepulsion;	//< Ratio between attraction and repulsion. The higher, the more attractive, the lower the more repulsive.
	KyFloat32 m_paramDelayTime;	//< Delay while we want to consider still collision.
	KyFloat32 m_paramTimeMinTrigger; //< Under this threshold, collisions are ignored.
	KyFloat32 m_paramTimeMaxTrigger; //< Above that this threshold, collisions are ignored.
	KyFloat32 m_paramSlowSpeedFactor; //< Ratio by which MaxSpeed is reduced to have a slow speed.
	KyFloat32 m_paramVerySlowSpeedFactor; //< Ratio by which MaxSpeed is reduced to have a very slow speed.
	KyFloat32 m_paramDetectionDistanceRatio; //< Ratio by which multiply the width to get the distance outside which entities are systematically ignored
	KyFloat32 m_paramCosThreshold; //< Cos Angle between orientation and targer under which it is preferable to avoid than to break
	KyFloat32 m_paramTimeToCollisionThreshold; //< time to collision in seconds under which it is preferable to avoid than to break

};

} // namespace Kaim

#endif // KY_GOTO_REPULSORDYNAMICAVOIDANCE_H
