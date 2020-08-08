/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_COMPUTETARGETPOINT_QUEUE_H
#define KY_COMPUTETARGETPOINT_QUEUE_H

#include <kyruntime/pathfinding/icomputetargetpoint.h>


namespace Kaim
{

class BodyInfoManager;


/*!	This implementation of the IComputeTargetPoint interface chooses a target point that
	is the same as the current PathNode in most cases. However, when the Bot begins
	to target its final destination point, and that destination point is already occupied
	by another Body, this modifier calculates a target point that lies as close as possible 
	to the final destination but outside the bounding box of the other blocking \Bodies. 
	This tends to make \Bots form queues when other \Bots and \Bodies block their destinations,
	instead of trying to force their way to unreachable destinations.
	\pp For best performance, also set up the BodyInfoManager for your World. When this WorldService
	is available, ComputeTargetPoint_Queue queries it to determine which other
	\Bodies lie in close proximity to the destination. If it is not available, 
	ComputeTargetPoint_Queue has to iterate through the list of all \Bodies in the
	World in order to find the ones that are near enough to make the subject Bot queue.
	\ingroup kyruntime_pathfinding */
class ComputeTargetPoint_Queue: public IComputeTargetPoint
{
public:
	KY_DECLARE_COMPUTETARGETPOINT(ComputeTargetPoint_Queue)


protected:
	/*!	This class is used internally by the ComputeTargetPoint_Queue modifier. You should not need to
		interact directly with this class.
		\ingroup kyruntime_pathfinding*/
	class GetOverlapingParam
	{
		KY_DEFINE_NEW_DELETE_OPERATORS

	public:
		GetOverlapingParam(KyFloat32 myRadius, KyFloat32 myHalfHeight, Body* ignoredBody,
			const Vec3f& dest, KyFloat32 distToDest, const Vec3f& dirToDest):
			m_myRadius(myRadius),
			m_myHalfHeight(myHalfHeight),
			m_ignoredBody(ignoredBody),
			m_dest(dest),
			m_distToDest(distToDest),
			m_dirToDest(dirToDest) {}

	private:
		GetOverlapingParam operator=(const GetOverlapingParam&) { return *this; }

	public:
		KyFloat32 m_myRadius;
		KyFloat32 m_myHalfHeight;
		Body* m_ignoredBody;
		const Vec3f& m_dest;
		KyFloat32 m_distToDest;
		const Vec3f& m_dirToDest;
	};


public:
	/*!	\label_constructor */
	ComputeTargetPoint_Queue()
		:IComputeTargetPoint()
		,m_filterCB(KY_NULL)
		,m_bodyInfoManager(KY_NULL)
		,m_maxDist(10.0f)
		,m_maxBotDist(2.5f)
		,m_extraGap(0.25f)
		,m_maxDeltaHeightRatio(1.0f)
	{}

	virtual KyBool ComputeTargetPoint(const PointWrapper& currentPathNodePosition,
		PointWrapper& targetPoint);

	/*!	This method is called transparently by ComputeTargetPoint() to determine whether or not
		it should skip computing the target point in the current frame. This can help to save
		CPU resources by avoiding unnecessary calculations.
		\pp The default implementation of this method skips computing the target point in the
		following cases:
		-	When the PathFinder is targeting any PathNode except the final destination.
		-	When the distance to the final destination is larger than the value of #m_maxDist.
		\pp You can redefine this method in a derived class in order to customize the circumstances
		under which queueing positions are computed. For example, you could allow queueing 
		computations when a Bot targets a PathNode that lies at the start of an edge linked
		to a PathObject that represents an elevator. This would tend to make \Bots queue in front 
		of the elevator doors.
		\param dest				The current PathNode targeted by the PathFinder.
		\return #KY_FALSE if a new target point should be computed, or #KY_TRUE to use the position
				of the current PathNode as the target point. */
	virtual KyBool DisableQueueing(const PointWrapper& dest);

	/*!	Defines a type of callback function that can be passed to ComputeTargetPoint_Queue
		to determine which other \Bodies should be taken into account when computing
		a target point that lies outside the area occupied by other \Bodies.
		\param thisBot			The Bot using ComputeTargetPoint_Queue.
		\param targetBot		Another Bot that may be taken into account by ComputeTargetPoint_Queue in
								its calculations.
		\return #KY_TRUE if \c thisBot should take \c targetBot into account in its computations, 
				or #KY_FALSE otherwise. */
	typedef KyBool (*FilteringFunction)(Bot& thisBot, Bot& targetBot);

	/*!	Sets a callback function that ComputeTargetPoint_Queue will invoke to determine
		which other \Bots it should take into account in its computations. */
	virtual void SetFilteringCallBack(FilteringFunction func) { m_filterCB = func; }

	/*!	Retrieves a callback function previously set in a call to SetFilteringCallBack(). */
	virtual FilteringFunction GetFilteringCallBack() { return m_filterCB; }

protected:
	/*	Computes if there is any body in the way of 'entity' up to 'dest' position.
		If any is detected, it computes a 'correctedDest' and returns the pointer to the concerned body. */
	Body* GetOverlaping(const Vec3f& dest, Vec3f& correctedDest, Body* ignoredBody);

	KyBool GetOverlaping_ForEachBody(Body* body, Vec3f& correctedDest, const GetOverlapingParam& param);


public:
	FilteringFunction m_filterCB; /*!< Stores the callback set through a call to SetFilteringCallBack(). Do not modify directly. */
	BodyInfoManager* m_bodyInfoManager; /*!< A pointer to the BodyInfoManager for the World. Do not modify. */

	/*! Retrieves the value set through a call to MaxDist(KyFloat32). */
	KyFloat32 MaxDist() const { return m_maxDist; }

	/*! Determines the radius from the destination point in which the subject Bot will queue when blocked by other \Bots and \Bodies. 
		When the subject Bot is farther from its destination than this value, it will not queue.
		\pp It is recommended that you exercise caution in setting this parameter, as increasing its value will consume more CPU resources.
		\units			meters
		\acceptedvalues	any positive value
		\defaultvalue	\c 10.0f */
	void MaxDist(KyFloat32 val)
	{
		KY_FUNCTION("ComputeTargetPoint_Queue::MaxDist");
		KY_ASSERT(val >= 0.f, ("Invalid value (%.2f), 'MaxDist' must be >= 0.", val));
		m_maxDist = val;
	}

	/*! Retrieves the value set through a call to MaxBotDist(KyFloat32). */
	KyFloat32 MaxBotDist() const { return m_maxBotDist; }

	/*! Determines the maximum distance a Body may be from the current subject Bot in order for ComputeTargetPoint_Queue to consider that Body when
		determining if the destination is blocked. Used only if the BodyInfoManager is configured for the World.
		\units			meters
		\acceptedvalues	any positive value
		\defaultvalue	\c 2.5f */
	void MaxBotDist(KyFloat32 val) 
	{ 
		KY_FUNCTION("ComputeTargetPoint_Queue::MaxBotDist");
		KY_ASSERT(val >= 0.f, ("Invalid value (%.2f), 'MaxBotDist' must be >= 0.", val));
		m_maxBotDist = val; 
	}

	/*! Retrieves the value set through a call to ExtraGap(KyFloat32). */
	KyFloat32 ExtraGap() const { return m_extraGap; }

	/*! Determines the minimum distance ComputeTargetPoint_Queue attempts to leave between passing Bots. 
		\pp The default value should suffice for most purposes. It is recommended that you do not change the value of this parameter unless instructed by Autodesk support.
		\units			meters
		\acceptedvalues	any positive value
		\defaultvalue	\c 0.25f */
	void ExtraGap(KyFloat32 val) 
	{ 
		KY_FUNCTION("ComputeTargetPoint_Queue::ExtraGap");
		KY_ASSERT(val >= 0.f, ("Invalid value (%.2f), 'ExtraGap' must be >= 0.", val));
		m_extraGap = val; 
	}

	/*! Retrieves the value set through a call to MaxDeltaHeightRatio(KyFloat32). */
	KyFloat32 MaxDeltaHeightRatio() const { return m_maxDeltaHeightRatio; }

	/*! Determines the maximum allowable height differential between the current Bot and other \Bodies for ComputeTargetPoint_Queue to consider those other 
		\Bodies when computing target points. 
		\pp The vertical tolerance is determined by the following calculation:
		\pp <tt> m_maxDeltaHeightRatio * 0.5 * (height1 + height2) </tt>
		\pp where \c height1 represents the altitude of the current Bot in meters, and \c height2 represents the height of another Body in meters.
		\pp Decreasing the value of this parameter makes ComputeTargetPoint_Queue less inclusive of \Bodies on different levels. This may increase performance
		by causing your Bot to ignore \Bodies on different floors. Increasing the value of this parameter makes ComputeTargetPoint_Queue more inclusive of 
		Bots on different levels. This may resolve irregularities that can occur in terrains with large altitude differences.
		\pp The default value should suffice for most purposes. 
		\acceptedvalues	any positive value
		\defaultvalue	\c 1.0f */
	void MaxDeltaHeightRatio(KyFloat32 val) 
	{ 
		KY_FUNCTION("ComputeTargetPoint_Queue::MaxDeltaHeightRatio");
		KY_ASSERT(val >= 0.f, ("Invalid value (%.2f), 'MaxDeltaHeightRatio' must be >= 0.", val));
		m_maxDeltaHeightRatio = val; 
	}

protected:
	virtual void OnSetPathFinder();
	
	// parameters
	KyFloat32 m_maxDist; /*!< Stores the value set by MaxDist(KyFloat32). */
	KyFloat32 m_maxBotDist; /*!< Stores the value set by MaxBotDist(KyFloat32). */
	KyFloat32 m_extraGap; /*!< Stores the value set by ExtraGap(KyFloat32). */
	KyFloat32 m_maxDeltaHeightRatio; /*!< Stores the value set by MaxDeltaHeightRatio(KyFloat32). */
		
	// Internal stuff
	Bot *m_me; /*!< A pointer to the Bot using the PathFinder. Do not modify. */
	Vec3f m_upDir; /*!< For internal use. Do not modify. */
};

} // namespace Kaim

#endif // KY_COMPUTETARGETPOINT_QUEUE_H
