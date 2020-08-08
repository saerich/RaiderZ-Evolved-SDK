/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_attributes */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_ACTIONROTATE_H
#define KY_ACTIONROTATE_H

#include <kyruntime/core/action.h>


namespace Kaim
{

/*!	This class is used by \SDKName components to indicate the orientation to which a character in the \gameOrSim engine
	should rotate, expressed in degrees of absolute yaw. For details on absolute yaw, see 
	:USERGUIDE:"The \SDKName Coordinate System" and :USERGUIDE:"Pitch, Roll and Yaw".
	\pp Every Bot must be assigned either ActionRotate or ActionSteering, in order for the PathFinder to
	indicate the direction or change in direction that should be applied to the Bot at each frame.
	\ingroup kyruntime_attributes */
class ActionRotate: public ActionAttribute
{
public:
	KY_DECLARE_ACTIONATTRIBUTE(ActionRotate)


public:
	/*!	\label_baseconstructor */
	ActionRotate() {}
	
	/*!	\label_virtualdestructor */
	virtual ~ActionRotate() {}
	
	/*!	Copies the state of the specified ActionRotate instance to this, and marks this ActionAttribute as 
		having changed in the current frame. */
	virtual ActionAttribute& operator=(const ActionAttribute& attr);

	/*!	Sets the target yaw value for the character in the \gameOrSim engine. */
	void SetYaw(KyFloat32 yaw) { m_yaw = yaw; Update(); }

	/*!	Retrieves the target yaw value for the character in the \gameOrSim engine. */
	KyFloat32 GetYaw() const { return m_yaw; }


private:
	KyFloat32 m_yaw;
};

} // namespace Kaim

#endif // KY_ACTIONROTATE_H
