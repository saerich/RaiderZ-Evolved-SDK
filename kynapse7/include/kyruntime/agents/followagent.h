/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_agents */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_FOLLOWAGENT_H
#define KY_FOLLOWAGENT_H

#include <kyruntime/pathfinding/pathfinder.h>


namespace Kaim
{

class ActionTorsoRotate;
class PointLockManager;


/*!	The FollowAgent makes a Bot follow another specified Body, at a given distance and at a given angle relative to the
	orientation of the target Body. 
	\pp For details on setting up and using this class, see :USERGUIDE:"Following Another Character".
	\ingroup kyruntime_agents */
class FollowAgent
{
	KY_DEFINE_NEW_DELETE_OPERATORS
	FollowAgent operator= (const FollowAgent&);

	/* The FollowAgent creates and initializes an instance of this class to ensure that any Body that is removed 
		from the World is also un-registered from the FollowAgent if it is the current target of the FollowAgent.
		\ingroup kyruntime_agents */
	class WorldObserver : public IWorldObserver
	{
	public:
		/* Default constructor. */
		WorldObserver() : m_followAgent(KY_NULL) {}

		/* Sets up the WorldObserver for use. 
			\param followAgent		The instance of the FollowAgent whose target will be updated. */
		void Initialize(FollowAgent& followAgent) { m_followAgent = &followAgent; }

		virtual void OnActivateBody(Body& ) {}
		/* This implementation of IWorldObserver::OnActivateBody() tests each Body removed from the World to see if
			it matches the target Body of the FollowAgent. If it is, the target Body of the FollowAgent is reset to #KY_NULL. */
		virtual void OnDeactivateBody(Body& body)
		{
			if (m_followAgent->GetTarget() == &body)
				m_followAgent->SetTarget(KY_NULL);
		}
	protected:
		FollowAgent* m_followAgent;
	};

public:
	//////////////////////////////////////////////////////////////////////////////////////////
	// STANDARD AGENT FUNCTIONS
	//////////////////////////////////////////////////////////////////////////////////////////

	/*! \label_constructor \label_mustinit */
	FollowAgent();

	/*! \label_destructor */
	~FollowAgent();

	/*! \label_init 
		\param bot							The Bot that will use the FollowAgent.
		\param computeRealDestinationTaskId	The unique ID of a aperiodic task configured for the TraversalManager, which will be used
											to determine whether or not enough time remains in the current frame for the FollowAgent
											to recompute its ideal destination. */
	KyResult Initialize(Bot& bot, TimeManager::TaskId computeRealDestinationTaskId);

	/*! \label_clear \label_mustinit */
	void Clear();

	/*! \label_reinit */
	void ReInit();
	
	/*!	Determines an appropriate destination point for the FollowAgent based on the position of the target Bot,
		the distance to the target, and the desired following distance and angle set for the FollowAgent. It
		then calls the PathFinder to move the FollowAgent to the destination. */
	KyBool Think();

	//////////////////////////////////////////////////////////////////////////////////////////
	// ACCESSOR FUNCTIONS
	//////////////////////////////////////////////////////////////////////////////////////////

	/*!	Sets the target Body that the FollowAgent will follow. */
	void SetTarget(Body* target);

	/*!	Retrieves the target Body that the FollowAgent is currently following. */
	Body* GetTarget() const { return m_targetBody;}

	/*!	Determines the distance the FollowAgent attempts to maintain between itself and its target Bot. 
		\units			meters
		\acceptedvalues	any positive value or \c 0.0f
		\defaultvalue	\c 1.0f */
	void SetDistFromTarget (KyFloat32 distFromTarget);

	/*!	Retrieves the value set through a call to SetDistFromTarget(). */
	KyFloat32 GetDistFromTarget() const {return m_distFromTarget;}

	/*!	Determines the angle the FollowAgent attempts to maintain between the orientation of the target Body and the
		preferred destination. For example, the default value of \c 180.0 degrees means the FollowAgent stays directly
		behind the target Body.
		\units			degrees
		\acceptedvalues	any negative or positive value
		\defaultvalue	\c 180.0f */
	void SetAngleFromTarget(KyFloat32 angleFromTarget);
	
	/*!	Retrieves the value set through a call to SetAngleFromTarget(). */
	KyFloat32 GetAngleFromTarget() const { return m_angleFromTarget; }

	/*!	Determines the minimum distance the target Body must move in order to trigger a response from the FollowAgent. 
		If the target Body moves less than this value, the FollowAgent will not re-evaluate its ideal destination.
		\units			meters
		\acceptedvalues	any positive value
		\defaultvalue	\c 1.0f */
	void SetMinimalImportantMove(const KyFloat32 value);
	
	/*!	Retrieves the value set through a call to SetMinimalImportantMove(). */
	KyFloat32 GetMinimalImportantMove() const { return m_minimalImportantMove; }

	/*!	Determines the preferred speed at which the FollowAgent should move. This preferred speed is capped to
		the value of the BodyMaxSpeed attribute set for the Bot using the FollowAgent, if any. In 
		addition, it may be altered by the current set of IModifier classes used by the PathFinder.
		\units			meters per second
		\acceptedvalues	any positive value
		\defaultvalue	The value of the BodyMaxSpeed attribute of the Bot if available, or \c 1.0f otherwise. */
	void SetSpeed(const KyFloat32 speed);
	
	/*!	Retrieves the value set through a call to SetSpeed(). */
	KyFloat32 GetSpeed() const { return m_speed; }

	/*!	Specifies the ratios the FollowAgent uses to decide whether it should switch to Close mode or Relaxed mode.
		\param closeRatio		If the ratio between the distance to the target Body and the specified following distance
								is less than this ratio, the FollowAgent enters Close mode.
		\param farRatio			If the ratio between the distance to the target Body and the specified following distance
								is greater than this ratio, the FollowAgent enters Relaxed mode.
		\acceptedvalues	any positive values
		\defaultvalue	for \c close parameter: \c 1.5. For \c far parameter: \c 2.0 */
	void SetRelaxedModeEnterRatio(KyFloat32 closeRatio, KyFloat32 farRatio);

	/*!	Determines the maximum amount of time the FollowAgent may spend trying to reach its destination point.
		When this time interval is exceeded, the FollowAgent recalculates its ideal destination point. */
	void SetMaxTimeToReachIdealDestination(KyFloat32 interval);

	//////////////////////////////////////////////////////////////////////////////////////////
	// AGENT STATE FEEDBACK
	//////////////////////////////////////////////////////////////////////////////////////////

	/*!	Retrieves the current destination point of the FollowAgent. */
	const Vec3f& GetDestination() const { return m_destination.GetPosition(); }
	
	/*!	Indicates whether or not the FollowAgent has arrived at its current destination point.*/
	KyBool HasArrived() const { return m_hasArrived; }
	
	/*!	Computes the ideal destination point for the FollowAgent: the position that lies at the angle and distance
		from the target Bot that are currently configured for the FollowAgent. This method is called transparently
		by FollowAgent::Think() when necessary.
		\param[out] idealPos		Stores the computed destination point.
		\return #KY_TRUE if the calculated position is directly reachable from the current position of the FollowAgent
				(determined by calling the ICanGo modifier in use by the PathFinder), or #KY_FALSE 
				if the destination position is blocked by static or dynamic obstacles. */
	KyBool ComputeIdealDestination(Vec3f& idealPos);
	
	/*!	Indicates whether or not the FollowAgent is currently in Close mode. */
	KyBool IsInCloseMode() const {return m_currentlyInShortMode;}
	
	/*!	Retrieves the linear distance between the FollowAgent and the target Body. */
	KyFloat32 GetActualDistanceToTarget() const;

	/*!	Indicates whether or not the FollowAgent is in Relaxed mode. */
	KyBool IsRelaxedMode() const { return m_useRelaxedMode; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// SPECIFIC FUNCTIONS
	//////////////////////////////////////////////////////////////////////////////////////////
protected:
	/*	Updates the destination (m_destination).
		If the agent is too far from the followed Bot or if the IdealDestination is unreachable
		(already locked or not in NavMesh), the destination is set to the target position
		and agent enter in "Relaxed Mode" (regardless to given distance and angle). */
	void ComputeDestination();

	/*	Hysteresis check that sets m_currentlyInShortMode variable to KY_TRUE when
 		agent enters CLOSE mode and to KY_FALSE when it enters FAR mode. Otherwise,
 		it does nothing.*/
	KyBool ProcessModeChanges();

	/*	Check if the agent need to recompute its destination.
		If the agent is in CLOSE mode, check if the entity followed
		has not move more than the Minimal Important Move.
		If the agent is not in CLOSE mode, check if the entity followed
		has not move more than DistToTarget * ratio (20 % by default) */
	KyBool NeedRecomputation(KyBool forceRecomputation);

	// "Relaxed Mode" state changes functions
	void EnterRelaxedMode(){m_useRelaxedMode = KY_TRUE;}
	void ExitRelaxedMode(){m_useRelaxedMode = KY_FALSE;}



	//////////////////////////////////////////////////////////////////////////////////////////
	// AGENT MEMBER VALUES
	//////////////////////////////////////////////////////////////////////////////////////////
protected:
	Bot* m_bot;

	WorldObserver m_worldObserver;
	// Behaviour data
	Body* m_targetBody; //< Accessor to target entity
	KyFloat32 m_distFromTarget; //< Follow instruction (distance from target)
	KyFloat32 m_angleFromTarget; //< Follow instruction (angle from target)

	// Services
	PointLockManager* m_pointLockManager;	//< Pointer to point lock manager world service

	// Time-slicing configuration
	TimeManager::TaskId m_computeRealDestinationTaskId; //< Id of ComputeRealDistance time-managed task
	KyFloat32 m_computeRealDistTimer; //< This timer is used to force a minimal delay between two calls
	KyFloat32 m_maximalIntervalToReevaluate; //< Maximal time without update to force a destination reevaluation
	KyFloat32 m_minimalIntervalToReevaluate; //< Minimal time to wait between two destination update
	KyFloat32 m_maximalIntervalToReachDestination; //< Maximal time to reach an Ideal Destination
	KyFloat32 m_timerToReach; //< time used to force a recomputation when agent take too muche time to reach its ideal destination

	// Target entity information
	Vec3f m_targetLastPos; //< Target entity position when agent last updated its destination point.
	Vec3f m_targetLastOri; //< Target entity orientation when agent last updated its destination point.

	ActionTorsoRotate* m_torsoRotateAttr; //< Bot Torso Rotate Action, used to aim at the same direction than the entity followed

	// Chosen destination
	PointWrapper m_destination; //< Agent destination point.
	PointInfo_NavTriangleSafePtr m_idealDestPointInfo; //< Ideal destination Mesh Point Info
	KyBool m_hasArrived; //< A flag to know if the agent has arrived

	// State data
	KyFloat32 m_speed;//< Contains agents move speed
	KyBool m_currentlyInShortMode;//< Boolean flag activated if agent is in 'short Mode'.
	KyBool m_useRelaxedMode;//< Agent should go to its target regardless to given angle order

	/* Ratio applied to m_distFromTarget. It determines an area where agent
		enters 'CLOSE mode' when getting closer to target entity than
		m_closeModeEnterRatio*m_distFromTarget. */
	KyFloat32 m_closeModeEnterRatio;

	/* Ratio applied to m_distFromTarget. It determines an area where
		agent enters 'FAR mode' when getting farther than
		m_farModeEnterRatio*m_distFromTarget. */
	KyFloat32 m_farModeEnterRatio;

	/* FAR mode only : Ratio applied to real distance to target.
		When target moves more than obtained length from
		m_targetLastPos, new objective point is set to current
		target entity position. It leads to an A* recomputation. */
	static const KyFloat32 s_farModeRatioToUpdateDestination;

	/* MinimalImportantMove is a distance >= 0. It determines an area in which, in CLOSE mode,
		any move of target entity will not affect agent. The smaller this value is, the more reactive
		the AI-driven entity will be, but it will also perform more computations to keep update its goal. */
	KyFloat32 m_minimalImportantMove;

};

} // namespace Kaim

#endif // KY_FOLLOWAGENT_H
