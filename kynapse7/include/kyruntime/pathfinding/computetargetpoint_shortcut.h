/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/




/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_COMPUTETARGETPOINT_SHORTCUT_H
#define KY_COMPUTETARGETPOINT_SHORTCUT_H

#include <kyruntime/pathfinding/icomputetargetpoint.h>


namespace Kaim
{
/*!	This implementation of the IComputeTargetPoint interface samples the edge
	leading from the current PathNode to the following PathNode, and selects a
	target point that is as far along that edge as possible. This results in your
	\Bots smoothing their \Paths more aggressively wherever possible, which
	typically increases the realism of their trajectories.
	\pp For example, in the image on the left below, a Bot standing at the green point 
	who is instructed to move to the red destination point will choose the vertex at the
	top of the image as the first PathNode in its Path. It cannot skip directly to the next 
	PathNode, as the next PathNode is blocked by the corner of the building. It will 
	therefore head toward the first PathNode until the IDetectPathNodeReached 
	modifier determines that the PathNode has been reached or until the following PathNode
	becomes reachable directly, then it will head down the edge toward the destination.
	\pp When using the ComputeTargetPoint_ShortCut modifier as in the image on the right, 
	the Bot will still adopt the vertex at the top of the image as the first PathNode in
	its Path. However, ComputeTargetPoint_ShortCut then samples locations along the edge leading from 
	the current PathNode to the succeeding PathNode at the bottom of the image, and
	selects as the current target point the last sampled point along the edge that is
	directly reachable by the Bot. The Bot therefore avoids the long detour up to
	the first PathNode.
	\pp \image html "graphics/modifiers_icomputetargetpoint_shortcut.png" */
class ComputeTargetPoint_ShortCut: public IComputeTargetPoint
{
public:
	KY_DECLARE_COMPUTETARGETPOINT(ComputeTargetPoint_ShortCut)


public:
	/*!	\label_constructor */
	ComputeTargetPoint_ShortCut()
		:IComputeTargetPoint()
		,m_samplingStep(0.5f)
		,m_squareDist(0.25f)
		,m_maxDeltaHeight(KyFloat32MAXVAL)
		,m_updatePeriod(0.5f)
	{}

	void ReInit();

	virtual KyBool ComputeTargetPoint(const PointWrapper& currentPathNodePosition, PointWrapper& targetPoint);

	/*! Retrieves the value set through a call to SamplingStep(KyFloat32). */
	KyFloat32 SamplingStep() const { return m_samplingStep; }

	/*! Determines the distance between each two sampling points along an edge. 
		\pp It is recommended that you exercise caution in setting this parameter. Setting a lower value may increase the
		accuracy of the shortcut evaluations, but it will also consume more CPU resources.
		\units			meters
		\acceptedvalues	any positive value
		\defaultvalue	\c 0.5f */
	void SamplingStep(KyFloat32 val)
	{ 
		KY_FUNCTION("ComputeTargetPoint_ShortCut::SamplingStep");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'SamplingStep' must be > 0.", val));
		m_samplingStep = val; 
	}

	/*! Retrieves the value set through a call to MaxDist(KyFloat32). */
	KyFloat32 MaxDist() const { return sqrtf(m_squareDist); }

	/*! Determines the maximum distance from a target point at which ComputeTargetPoint_ShortCut considers that target point to be reached.
		If the distance between the Bot and its current target point is greater than this value, the point is not considered reached.
		\units			meters
		\acceptedvalues	any positive value
		\defaultvalue	\c 0.5f */
	void MaxDist(KyFloat32 val) 
	{ 
		KY_FUNCTION("ComputeTargetPoint_ShortCut::MaxDist");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'MaxDist' must be > 0.", val));
		m_squareDist = val; 
		m_squareDist *= m_squareDist;
	}

	/*! Retrieves the value set through a call to MaxDeltaHeight(KyFloat32). */
	KyFloat32 MaxDeltaHeight() const { return m_maxDeltaHeight; }

	/*! Determines the maximum allowable height differential between a Bot and a target point in order for ComputeTargetPoint_ShortCut to consider
		that target point to be reached. 
		\units			meters
		\acceptedvalues	any positive value
		\defaultvalue	#KyFloat32MAXVAL */
	void MaxDeltaHeight(KyFloat32 val) 
	{
		KY_FUNCTION("ComputeTargetPoint_ShortCut::MaxDeltaHeight");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'MaxDeltaHeight' must be > 0.", val));
		m_maxDeltaHeight = val; 
	}

	/*! Retrieves the value set through a call to UpdatePeriod(KyFloat32). */
	KyFloat32 UpdatePeriod() const { return m_updatePeriod; }
	
	/*! Determines the time interval between successive target point updates. 
		\pp In most cases, it is unnecessary to update target points every frame; therefore, setting a higher value for this parameter can ease the
		burden on your CPU. However, setting the value too high may result in your \Bots being slow to recognize possible shortcuts.
		\units			seconds
		\acceptedvalues	any positive value
		\defaultvalue	\c 0.5 */
	void UpdatePeriod(KyFloat32 val) 
	{ 
		KY_FUNCTION("ComputeTargetPoint_ShortCut::UpdatePeriod");
		KY_ASSERT(val >= 0.f, ("Invalid value (%.2f), 'UpdatePeriod' must be >= 0.", val));
		m_updatePeriod = val;
	}

protected:
	KyBool EvaluateShortcut(const PathNodeIterator& candiatePathNode, const Vec3f& candidate) const;
	KyBool DidAdvanceTargetPoint(const KyFloat32 norm, const PathNodeIterator& current, const Vec3f& dir, const PointWrapper& currentPathNodePosition);

	virtual void OnSetPathFinder();

protected:
	KyFloat32 m_samplingStep; //< The value set through SamplingStep().
	KyFloat32 m_squareDist; //< Square of the value set through MaxDist().
	KyFloat32 m_maxDeltaHeight; //< The value set through MaxDeltaHeight().
	KyFloat32 m_updatePeriod; //< The value set through UpdatePeriod().

	Vec3f m_currentSubgoal;
	Vec3f m_nextSubgoal;
	PointWrapper m_targetPoint;
	KyFloat32 m_currentSamplingStep;
	KyFloat32 m_currentOffset;
	KyFloat32 m_lastUpdateTime;
};

} // namespace Kaim

#endif // KY_COMPUTETARGETPOINT_SHORTCUT_H
