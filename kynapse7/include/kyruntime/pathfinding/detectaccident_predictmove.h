/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_DETECTACCIDENT_PREDICTMOVE_H
#define KY_DETECTACCIDENT_PREDICTMOVE_H

#include <kyruntime/pathfinding/idetectaccident.h>


namespace Kaim
{
/*!	This implementation of the IDetectAccident interface detects accidents in the two following
	cases.
	\section predictmovemismatch Bot position not as expected
	At the end of each frame, the DetectAccident_PredictMove::Update() method predicts an expected position 
	for the Bot in the next frame, based on the target speed and orientation calculated for the Bot. If the
	ActionSpeed value of the Bot was updated, its value is used for the target speed; if not, 
	PathFinder::GetTargetSpeed() is called. If the ActionRotate value of the Bot was updated, its value is
	used for the target orientation; if not, the orientation of the target point of the PathFinder relative to
	the current position of the Bot is used. 
	\pp When the DetectAccident_PredictMove::DetectAccident() method is invoked, it compares the current 
	position of the Bot to the position predicted for it in the previous frame. If these positions are not
	within a certain margin of error, an accident is assumed to have occurred.
	\pp The margin of error is expressed as a ratio. If the distance between the predicted position and the 
	actual position is greater than this fraction of the distance the Bot was predicted to travel, an accident
	is detected (provided also that the distance the Bot was predicted to travel is greater than 0.1 meters). 
	\pp For example, the image below demonstrates the behavior of the accident detection when this parameter is
	set to its default value of \c 0.5: half of the distance the Bot was originally predicted to travel. When 
	the actual position of the Bot in the next frame lies inside the green area surrounding its predicted 
	position, no accident is detected. When the actual position of the Bot lies anywhere outside of the green 
	area, an accident is detected.
	\image html "graphics/accidentratio.png"
	\pp You can change this ratio freely by calling AccidentRatio(KyFloat32). Increasing the value makes 
	DetectAccident_PredictMove less sensitive to accidents, and decreasing the value makes it
	more sensitive to accidents. 
	\section predictmovewrongway Bot moves away from current PathNode
	If the Bot appears to be moving away from its current target PathNode, DetectAccident_PredictMove
	will assume that it cannot proceed as expected. 
	\pp At the end of each frame, if the current PathNode maintained by the PathFinder has changed, the 
	Update() method stores the distance to that PathNode. If the Bot travels a total
	distance that is more than four times greater than that initial distance, an accident is deemed to have occurred.
	\ingroup kyruntime_pathfinding */
class DetectAccident_PredictMove: public IDetectAccident
{
public:
	KY_DECLARE_DETECTACCIDENT(DetectAccident_PredictMove)

public:
	/*!	\label_baseconstructor */
	DetectAccident_PredictMove() :
		IDetectAccident(),
		m_accidentRatio(0.5f),
		m_predictionDate(0.0f){}

	virtual void ReInit();

	virtual KyBool DetectAccident();
	virtual void Update();

	/*! Retrieves the value set through a call to AccidentRatio(KyFloat32). */
	KyFloat32 AccidentRatio() const { return m_accidentRatio; }

	/*! Determines how responsive DetectAccident_PredictMove is to mismatches between the actual and expected positions of the Bot.
		See the class description above for details.
		\acceptedvalues	any positive value
		\defaultvalue	\c 0.5f */
	void AccidentRatio(KyFloat32 val)
	{
		KY_FUNCTION("DetectAccident_PredictMove::AccidentRatio");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'AccidentRatio' must be > 0.", val));
		m_accidentRatio = val;
	}


protected:
	virtual void OnSetPathFinder();

	KyFloat32 m_accidentRatio;

	Vec3f m_predictedPosition;
	KyFloat32 m_predictionDate;
	KyFloat32 m_straightDistToSubgoal;
	KyFloat32 m_coverDistToSubgoal;
	Vec3f m_currentNodePosition;
	Vec3f m_previousPosition;
	KyFloat32 m_predictedDistance;
};

} // namespace Kaim

#endif // KY_DETECTACCIDENT_PREDICTMOVE_H
