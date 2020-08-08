/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_attributes */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_ACTIONSHOOT_H
#define KY_ACTIONSHOOT_H

#include <kyruntime/core/action.h>

#include <kypathdata/math/vec3f.h>


namespace Kaim
{

/*!	This class is used by \SDKName components to indicate whether or not a character in the \gameOrSim engine
	should shoot a weapon, and where it should aim.
	\pp This ActionAttribute is used only by the ShootAgent when it has a clear line of fire
	to its target. You may also use it if you write a custom component that needs to make your character
	fire a weapon under a given set of circumstances.
	\ingroup kyruntime_attributes */
class ActionShoot: public ActionAttribute
{
public:
	KY_DECLARE_ACTIONATTRIBUTE(ActionShoot)


public:
	/*!	\label_baseconstructor */
	ActionShoot() {}
	
	/*!	\label_virtualdestructor */
	virtual ~ActionShoot() {}
	
	/*!	Copies the state of the specified ActionShoot instance to this, and marks this ActionAttribute as 
		having changed in the current frame. */
	virtual ActionAttribute& operator=(const ActionAttribute& attr);

	/*!	Sets the direction the character in the \gameOrSim engine should aim. This value must be a normalized vector. */
	void SetShotDir (const Vec3f& dir)
	{
#ifdef KYDEBUG
		KY_FUNCTION("ActionShoot::SetShotDir");
		const float squareMagnitude = dir.GetSquareLength();
		const float epsilon = 0.0201f;
		KY_COND_MESSAGE(KY_LOG_WARNING, fabs(squareMagnitude - 1.f) > epsilon, ("Bad parameter : %s.", "Shoot Direction should be normalized"));
#endif
		m_shotDir = dir;
		Update();
	}

	/*!	Sets the target point at which the character in the \gameOrSim engine should aim. */
	void SetShotTargetPoint(const Vec3f& targetPos) { m_targetPos = targetPos; }

	/*!	Sets whether or not the character in the \gameOrSim engine should fire. */
	void SetIsShooting(KyBool isShooting) { m_isShooting = isShooting; Update(); }

	/*!	Retrieves the direction in which the character in the \gameOrSim engine should aim. */
	const Vec3f& GetShotDir() const { return m_shotDir; }

	/*!	Retrieves the target point at which the character in the \gameOrSim engine should aim. */
	const Vec3f& GetShotTargetPoint() const { return m_targetPos;}

	/*!	Retrieves whether or not the character in the \gameOrSim engine should fire. */
	KyBool GetIsShooting() const { return m_isShooting; }


private:
	Vec3f m_shotDir; // exact direction of the shot (remember we can be clumsy)
	Vec3f m_targetPos; //Exact position of aimed point
	KyBool m_isShooting; // KY_TRUE if the bot is shooting, KY_FALSE otherwise
};

} // namespace Kaim

#endif // KY_ACTIONSHOOT_H
