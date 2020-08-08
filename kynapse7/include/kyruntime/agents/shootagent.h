/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_agents */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_SHOOTAGENT_H
#define KY_SHOOTAGENT_H

#include <kyruntime/core/timemanager.h>
#include <kyruntime/core/world.h>

#include <kypathdata/containers/list.h>


namespace Kaim
{

class Body;
class PointLockManager;
class BodyInfoManager;
class BodyInfo_Visibility;
class ActionShoot;
class ActionTorsoRotate;
class BodyTeamSide;
class BodyGunPosition;
class PathFinder;


/*!	The ShootAgent makes a Bot approach a target Body and open fire once it has closed within firing range.
	\pp For details on setting up and using this class, see :USERGUIDE:"Shooting at Another Character".
	\ingroup kyruntime_agents */
class ShootAgent
{
	KY_DEFINE_NEW_DELETE_OPERATORS

protected:
	/* The ShootAgent creates and initializes an instance of this class to ensure that any Body that is removed 
		from the World is also un-registered from the ShootAgent if it is the current target of the ShootAgent.
		\ingroup kyruntime_agents */
	class WorldObserver : public IWorldObserver
	{
	public:
		/* Default constructor. For internal use. */
		WorldObserver() : m_shootAgent(KY_NULL) {}
		
		/* Sets up the WorldObserver for first use. For internal use.
			\param shootAgent		The instance of the ShootAgent whose target will be updated. */
		void Initialize(ShootAgent& shootAgent) { m_shootAgent = &shootAgent; }

		virtual void OnActivateBody(Body& ) {}
		/* This implementation of IWorldObserver::OnActivateBody() tests each Body removed from the World to see if
			it matches the target Body of the ShootAgent. If it is, the target Body of the ShootAgent is reset to #KY_NULL. */
		virtual void OnDeactivateBody(Body& body)
		{
			if (m_shootAgent->GetTargetBody() == &body)
				m_shootAgent->SetTargetBody(KY_NULL);
		}
	protected:
		ShootAgent* m_shootAgent;
	};

public:
	//////////////////////////////////////////////////////////////////////////////////////////
	// STANDARD AGENT FUNCTIONS
	//////////////////////////////////////////////////////////////////////////////////////////

	/*! \label_constructor \label_mustinit */
	ShootAgent();

	/*! \label_destructor */
	~ShootAgent();

	/*! \label_init 
		\param bot							The Bot that will use the ShootAgent.
		\param checkLineOfFireTaskId		The unique ID of an aperiodic task configured for the TraversalManager, which will be used
											to determine whether or not enough time remains in the current frame for the ShootAgent to 
											check whether or not friendly \Bodies have entered its line of fire.
		\param bodyInfoManager				The BodyInfoManager configured for the World. 
		\param pointLockManager				The PointLockManager configured for the World. Optional. */
	KyResult Initialize(Bot& bot, TimeManager::TaskId checkLineOfFireTaskId, BodyInfoManager& bodyInfoManager, PointLockManager* pointLockManager);

	/*! \label_clear \label_mustinit */
	void Clear();

	/*! \label_reinit */
	void ReInit();

	/*! Invokes the PathFinder to make the Bot close within firing range of a target, sidestepping other friendly \Bodies
		if necessary, and open fire at the target. */
	KyBool Think();

	//////////////////////////////////////////////////////////////////////////////////////////
	// ACCESSORS
	//////////////////////////////////////////////////////////////////////////////////////////

	/*!	Indicates whether or not the ShootAgent is waiting. #KY_TRUE indicates that the ShootAgent has not
		yet found a place from which it can fire at the target Body. */
	KyBool IsWaiting() const {return m_waiting;}

	/*!	Sets the target Body at whom the ShootAgent will shoot. */
	void SetTargetBody(Body* body);

	/*!	Retrieves the target Body currently set for the ShootAgent. */
	Body* GetTargetBody() const { return m_targetBody;}

	/*! Determines the range of the weapon used by the ShootAgent.
		\units			meters
		\acceptedvalues	Any positive value or \c 0.0f
		\defaultvalue	\c 30 */
	void SetGunRange (KyFloat32 gunRange)
	{
		KY_FUNCTION("ShootAgent::SetGunRange");
		KY_AFFECT_POSITIVE_FLOAT("GunRande", m_gunRange, gunRange);
	}

	/*!	Retrieves the value set through a call to SetGunRange(). */
	KyFloat32 GetGunRange () const { return m_gunRange; }

	/*!	Determines the angle of the "insecure cone". This cone's vertex is the position of the ShootAgent, and its axis
		is the orientation of the ShootAgent. If a non-enemy stands in this area, the ShootAgent will not shoot, but will
		nstead attempt to sidestep to a spot offering a clear shot at the target.
		\units			degrees
		\acceptedvalues	\c 0.0f to \c 180.0f
		\defaultvalue	\c 10.0f */
	void SetDangerousConeAngle(KyFloat32 coneAngle)
	{
		KY_FUNCTION("ShootAgent::SetDangerousConeAngle");
		KY_AFFECT_FLOAT_IN_RANGE("DangerousConeAngle", 0, 180, m_dangerousConeAngle, coneAngle);
	}

	/*!	Retrieves the value set through a call to SetDangerousConeAngle(). */
	KyFloat32 GetDangerousConeAngle () const { return m_dangerousConeAngle; }

	/*!	Determines the maximum time interval between successive line-of-fire calculations.
		\units			seconds
		\acceptedvalues	Any positive value or \c 0.0f
		\defaultvalue	\c 0.5 */
	void SetLineOfFireValidityInterval (const KyFloat32 value)
	{
		KY_FUNCTION("ShootAgent::SetLineOfFireValidityInterval");
		KY_AFFECT_POSITIVE_FLOAT("LineOfFireValidityInterval", m_lineOfFireValidityInterval, value);
	}

	/*!	Retrieves the value set through a call to SetLineOfFireValidityInterval(). */
	KyFloat32 GetLineOfFireValidityInterval () const { return m_lineOfFireValidityInterval; }

	/*!	When the ShootAgent is prevented from shooting due to the presence of a non-enemy, this value determines the angle
		of the sidesteps taken by the ShootAgent between successive checks to determine whether it has re-established a
		clear line of fire to the target.
		\units			degrees
		\acceptedvalues	any positive value
		\defaultvalue	\c 10.0f */
	void SetSideStepAngle (const KyFloat32 value)
	{
		KY_FUNCTION("ShootAgent::SetSideStepAngle");
		KY_AFFECT_POSITIVE_FLOAT("SideStep", m_sideStepAngle, value);
	}

	/*!	Retrieves the value set through a call to SetSideStepAngle(). */
	KyFloat32 GetSideStepAngle () const { return m_sideStepAngle; }

	/*!	When the ShootAgent is prevented from shooting due to the presence of a non-enemy, this value determines the maximum
		number of sidesteps the ShootAgent will take to the left or right when attempting to find a clear line of fire.
		\acceptedvalues	any positive value
		\defaultvalue	\c 20 */
	void SetPotentialSideStepsSize (const KyInt32 value)
	{
		KY_FUNCTION("ShootAgent::SetPotentialSideStepsSize");
		KY_AFFECT_POSITIVE_INT("NumPotentialSideSteps", m_potentialSideStepsSize, value);
	}

	/*!	Retrieves the value set through a call to SetPotentialSideStepsSize(). */
	KyUInt32 GetPotentialSideStepsSize () const { return m_potentialSideStepsSize; }

	/*!	Determines the preferred speed at which the ShootAgent should move. This preferred speed is capped to
		the value of the BodyMaxSpeed attribute set for the Bot using the ShootAgent, if any. In 
		addition, it may be altered by the current set of IModifier classes used by the PathFinder.
		\units			meters per second
		\acceptedvalues	any positive value
		\defaultvalue	The value of the BodyMaxSpeed attribute of the Bot if available, or \c 1.0f otherwise. */
	void SetSpeed(const KyFloat32 speed);

	/*!	Retrieves the value set through a call to SetSpeed(). */
	KyFloat32 GetSpeed() const { return m_speed; }

	/*!	Renders debug information about the ShootAgent.
		\param drawDangerousCone			Determines whether or not to draw the dangerous cone set for the ShootAgent (its weapon range). 
											If the ShootAgent is close enough to the target Bot, and the target Bot is visible,
											the firing range will be drawn. If the cone is drawn in green, the ShootAgent is
											currently able to shoot at the target. If the cone is drawn in red, the ShootAgent is not
											able to shoot at the target (another Bot may be blocking the line of fire).
		\param drawPotentialSideStep		Determines whether or not potential side-steps are drawn. Each side step is indicated
											by a green line.
		\param drawIsWaiting				Determines whether or not the waiting status of the ShootAgent is rendered. If the ShootAgent is waiting,
											a red point is drawn above the head of the Bot using the ShootAgent. */
	void Render(KyBool drawDangerousCone = KY_TRUE, KyBool drawPotentialSideStep = KY_FALSE, KyBool drawIsWaiting = KY_FALSE) const;

private :
	// Returns KY_TRUE if the target is enough close to the agent's gun's position to be shooted
	KyBool IsTargetAtGunRange() const;

	// Returns KY_TRUE if the target is enough close to the 'gunPosition' parameter to be shooted
	KyBool IsTargetAtGunRange(const Vec3f& gunPosition)const;

	// Returns KY_TRUE if the target is visible from the agent's position.
	KyBool IsTargetVisible() const;

	// Returns KY_TRUE if the target is visible from the 'pos' parameter.
	KyBool IsTargetVisible(const Vec3f& pos) const;

	/* Returns KY_TRUE if the given body is an enemy. 
		This function uses the team attribut : 
			the target and the agent have to be from different team to be enemy */
	KyBool IsEnemy(const Body& body) const;

	/* This function is called to make entity aim/look at its target, updating agent's Action and gunDirection.
		\param shoot : entity is shooting or not */
	void AimAtTarget(KyBool shoot);

	// This function generates potentially usable side steps positions. It updates m_potentialSideSteps container.
	void GenerateSideStepPosition();

	/* This function forces to evaluate the next side step.
		Increment the m_currentStepIndex and clean the vector if m_currentStepIndex was the last one */
	void ForceNextSideStepEvaluation();

	/* This function removes the potential side step, reset the current step index and set m_validSideStepFound to false. */
	void CleanUpSideStepPosition();

	/* This functions check the line of fire from a position with the agent parameter (dangerous cone angle...)
	*	\param pos : the position from where we compute the line of fire.
	*	\param recomputed : a flag to know is the line of fire has been recomputed (manage by the aperiodic task)	
	*	\return :
	*		-# KY_TRUE if dangerous cone angle is zero or if nobody stand in weapon "dangerous cone"
	*		-# KY_FALSE if there is non-enemy entity stands in weapon "dangerous cone" or if the line of fire has not been recomputed */
	KyBool CheckLineOfFire(const Vec3f& pos, KyBool& recomputed);

	/* Check id the Agent can fire
		Call the CheckLineOfFire function with agent position.
		\return 
			-# KY_FALSE if line of fire has not been recomputed and line of fire is not valid anymore (according to m_isLineOfFireValid)
			-# previous m_canShoot value if the line of fire has not been recomputed but is still valid
			-# CheckLineOfFire result otherwise */
	KyBool CanFire();

	/* Try to find a valid shoot place  in the side step vector.
		\return 
			-# KY_TRUE if a place has been found (m_currentStepIndex is the goo place)
			-# KY_FALSE otherwise
		When m_currentStepIndex == m_potentialSideStepsSize, there is no valid place in all the side step vector */
	KyBool FindAValidSideStep();

	/* This function tries to find a valid place to shoot.
		\param destination the place to shoot found
		\return :
			-# KY_TRUE, a place has been found.
			-# KY_FALSE, a place as not been found yet. If all potential side step have been tested,
			the flag m_waiting is set to true. */
	KyBool FindAValidPlaceToShoot(Vec3f& destination);

	//////////////////////////////////////////////////////////////////////////////////////////
	// Attributes
	//////////////////////////////////////////////////////////////////////////////////////////
private:
	WorldObserver m_worldObserver;
	Bot* m_bot;
	// Services
	PointLockManager* m_pointLockManager;
	BodyInfoManager* m_bodyInfoManager;

	// Information classes
	BodyInfo_Visibility* m_visibilityInfo; //< Contains a pointer to a Visibility entity info class

	// Kaim action and entity attributes
	ActionShoot* m_shootAttr;
	ActionTorsoRotate* m_torsoRotateAttr;
	BodyTeamSide* m_teamAttr;
	BodyGunPosition* m_gunPosAttr;

	Body* m_targetBody; //< The target

	// The destination, when the agent is not shooting
	Vec3f m_destination;

	// Waiting flag
	KyBool m_waiting; //< If there isn't place to shoot, the agent is waiting.
					  //< When the agent is waiting, user should change the brain behavior.

	// Gun range
	KyFloat32 m_gunRange;

	Vec3f m_gunPosition; //<Absolute position of the gun (can differ slightly from the entity's position; see CBotGunPosition attribute)

	// Dangerous cone, entity check if non enemy entity inside
	KyFloat32 m_dangerousConeAngle;

	KyFloat32 m_speed;//< Contains agent's move speed

	KyBool m_canShoot; //< Can the agent shoot ?

	// Value and flag for the validity of the line of fire
	KyBool m_isLineOfFireValid; //< Updated in CheckLineOfFire and if the agent move
	KyFloat32 m_lineOfFireValidityInterval; //<Longest time the line of fire stay valid without recomputation
	KyFloat32 m_lastLineOfFireCheck; //< Last time the Line of Fire (for agent only, not for side step validation) has been recomputed
	TimeManager::TaskId m_checkLineOfFireTaskId; //< The aperiodic task id for the Line of Fire computation

	// Container to portantial side steps that agent can do when there are non enemy entity in his dangerous cone
	List<Vec3f> m_potentialSideSteps;
	// The number of potential side steps build
	KyUInt32 m_potentialSideStepsSize;

	// The current iterator in potential side steps array to check before going
	List<Vec3f>::Iterator m_currentStepIndex;
	// the angle of the side step
	KyFloat32 m_sideStepAngle;

	// A flag to know if the agent has found a valid side step
	KyBool m_validSideStepFound;

};

} // namespace Kaim

#endif //KY_SHOOTAGENT_H
