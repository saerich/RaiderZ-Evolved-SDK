/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_attributes */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_ACTIONTORSOROTATE_H
#define KY_ACTIONTORSOROTATE_H

#include <kyruntime/core/action.h>


namespace Kaim
{

/*!	This class is used by \SDKName components to indicate the orientation that should be applied to the torso of a character
	in the \gameOrSim engine. Its values are expressed in degrees of absolute yaw and absolute pitch. For details on
	absolute yaw and pitch, see :USERGUIDE:"The \SDKName Coordinate System" and :USERGUIDE:"Pitch, Roll and Yaw".
	\pp Use this ActionAttribute in cases where you want to manipulate the torso orientation of a character independently 
	of its direction of movement. For example, you can use this ActionAttribute to implement strafing movements, 
	where the character moves left to the left or right while its torso remains oriented straight ahead. 
	See :USERGUIDE:"Strafing".
	\pp This ActionAttribute can be used by the following components, if it is assigned to your Bot:
	-	The Steering_SimpleBiped modifier of the PathFinder. 
	-	The FollowAgent. When the FollowAgent arrives at its destination, it turns to face the
		same direction as its target Bodys.
	-	The ShootAgent. When the ShootAgent opens fire, it rotates to face its target.
	\ingroup kyruntime_attributes */
class ActionTorsoRotate: public Kaim::ActionAttribute
{
public:
	KY_DECLARE_ACTIONATTRIBUTE(ActionTorsoRotate)


public:
	/*!	\label_baseconstructor */
	ActionTorsoRotate() {}
	
	/*!	\label_virtualdestructor */
	virtual ~ActionTorsoRotate() {}
	
	/*!	Copies the state of the specified ActionTorsoRotate instance to this, and marks this 
		ActionAttribute as having changed in the current frame. */
	virtual Kaim::ActionAttribute& operator=(const Kaim::ActionAttribute& attr);

	/*!	Sets the absolute pitch that should be applied to the torso of the character in the \gameOrSim engine, in degrees. */
	void SetPitch(KyFloat32 pitch) { m_torsoPitch = pitch; Update(); }

	/*!	Sets the absolute yaw that should be applied to the torso of the character in the \gameOrSim engine, in degrees. */
	void SetYaw(KyFloat32 yaw)   { m_torsoYaw = yaw; Update(); }

	/*!	Retrieves the absolute pitch that should be applied to the torso of the character in the \gameOrSim engine, in degrees. */
	KyFloat32 GetPitch() const { return m_torsoPitch; }

	/*!	Retrieves the absolute yaw that should be applied to the torso of the character in the \gameOrSim engine, in degrees. */
	KyFloat32 GetYaw() const { return m_torsoYaw; }


private:
	KyFloat32 m_torsoPitch;
	KyFloat32 m_torsoYaw;
};

} // namespace Kaim

#endif // KY_ACTIONTORSOROTATE_H
