/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file 
	\ingroup kyruntime_debugger */

// primary contact: ? - secondary contact: LAPA
#ifndef KYDEBUGGERSHAREDCAMERA_H
#define KYDEBUGGERSHAREDCAMERA_H

#include <kypathdata/math/vec3f.h>


namespace Kaim
{
	namespace Debugger
	{
		namespace Shared
		{
			/*!	This structure stores the position and orientation of an observer's point of view in three-
				dimensional space. The RemoteDebugging service uses an instance of this structure to store 
				the position and orientation of the camera in the \gameOrSim engine. This information is sent
				to the remote %Debugger interface at each frame.
				\ingroup kyruntime_debugger */
			struct Camera
			{
				Kaim::Vec3f m_position; /*!< The position of the camera in global space. */
				Kaim::Vec3f m_right; /*!< The Right vector of the camera. */
				Kaim::Vec3f m_up; /*!< The Up vector of the camera. */
				Kaim::Vec3f m_forward; /*!< The Front vector of the camera. */

				/*!	Sets the position and orientation of the camera.
					\param position			The position of the camera in global space.
					\param right			The Right vector of the camera.
					\param up				The Up vector of the camera.
					\param forward			The Front vector of the camera. */
				void Set(const Kaim::Vec3f& position, const Kaim::Vec3f& right, const Kaim::Vec3f& up, const Kaim::Vec3f& forward)
				{
					m_position = position;
					m_right = right;
					m_up = up;
					m_forward = forward;
				}
			};
		}
	}
}

#endif // KYDEBUGGERSHAREDCAMERA_H
