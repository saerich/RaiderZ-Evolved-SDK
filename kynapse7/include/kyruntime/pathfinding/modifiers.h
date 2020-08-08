/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: BRGR - secondary contact: NOBODY
#ifndef KY_MODIFIERS_H
#define KY_MODIFIERS_H

#include <kypathdata/generic/memory.h>
#include <kyruntime/lpf/ilpfmanager.h>

namespace Kaim
{

class World;
class PathFinder;

class IRefineGoal;
class IDetectGoalReached;
class IDetectGoalChanged;
class IDetectAccident;
class IDetectPathNodeReached;
class ISelectPathNodeCandidate;
class IComputeTargetPoint;
class ICheckDirectWay;
class ICanGo;
class IGoto;
class ISteering;
class IFindNodesFromPositions;
class ILpfShortcut;
class IEdgeStatusAwareness;

/*! The Modifiers class is a collection of all the objects that derive from IModifier that the PathFinder uses during its
	path calculation and path following calculations. You create an instance of IModifier, pass it an instance of each
	IModifier class that you want your PathFinder to use, and pass your Modifiers instance to the PathFinder in your call
	to PathFinder::Initialize().
	\pp The PathFinder checks that all the modifier classes you have set in the Modifiers instance are intercompatible, and
	takes control of managing the lifespan of the modifier objects you provide. */
class Modifiers
{
public:
	/*! Populates the specified instance of Modifiers with a default set of IModifier for a
		character that walks on the ground, chosen according to the services set up for the World.
		The following classes are used:
		-	For the IRefineGoal interface:
			-	If the LpfManager is set up for the World, RefineGoal_LpfCompatible.
			-	Otherwise, RefineGoal_NearestInsideNavMesh.
		-	For the IEdgeStatusAwareness interface:
			-	If the LpfManager is set up for the World, EdgeStatusAwareness_Distance2dAndTime.
			-	Otherwise, #KY_NULL.
		-	For the ISelectPathNodeCandidate modifier:
			-	If the AsyncManager is set up for the World, SelectPathNodeCandidate_AsyncNextPathNode.
			-	Otherwise, SelectPathNodeCandidate_NextPathNode.
		-	For the IFindNodesFromPositions modifier:
			-	If the AsyncManager is set up for the World, FindNodesFromPositions_AsyncNearestReachable.
			-	Otherwise, FindNodesFromPositions_Default.
		-	For the IGoto modifier:
			-	If the AsyncManager and GapManager are set up for the World, Goto_AsyncGapDA.
			-	If the GapManager is set up for the World but not the AsyncManager, Goto_GapDynamicAvoidance.
			-	If neither the GapManager nor the AsyncManager are set up for the World, Goto_Trivial.
		-	For the IDetectGoalReached interface: 
			-	DetectGoalReached_Distance2D5 in all cases.
		-	For the IDetectGoalChanged interface: 
			-	DetectGoalChanged_Distance3D in all cases.
		-	For the IDetectAccident interface: 
			-	DetectAccident_PredictMove in all cases.
		-	For the IDetectPathNodeReached interface: 
			-	DetectPathNodeReached_Distance2D5 in all cases.
		-	For the ICanGo interface: 
			-	CanGo_NavMesh in all cases.
		-	For the ICheckDirectWay interface: 
			-	CheckDirectWay_Trivial in all cases.
		-	For the IComputeTargetPoint interface: 
			-	ComputeTargetPoint_Queue in all cases.
		-	For the ISteering interface: 
			-	Steering_SimpleBiped in all cases.
		-	For the ILpfShortcut interface:
			-	#KY_NULL in all cases.
		\param World&				The World in which this Modifiers instance will be used.
		\param[out] Modifiers&		The instance of Modifiers to set up with the default IModifier objects. */
	static void DeduceDefaultModifiers(World&, Modifiers&);

	/*! Populates the specified instance of Modifiers with a default set of IModifier for a
		flying character, chosen according to the services set up for the World.
		The following classes are used:
		-	For the ISelectPathNodeCandidate modifier:
			-	If the AsyncManager is set up for the World, SelectPathNodeCandidate_AsyncNextPathNode.
			-	Otherwise, SelectPathNodeCandidate_NextPathNode.
		-	For the IFindNodesFromPositions modifier:
			-	If the AsyncManager is set up for the World, FindNodesFromPositions_AsyncNearestReachable.
			-	Otherwise, FindNodesFromPositions_NearestReachable.
		-	For the IGoto modifier:
			-	Goto_RepulsorDynamicAvoidance in all cases.
		-	For the IRefineGoal interface:
			-	RefineGoal_Trivial in all cases.
		-	For the IDetectGoalReached interface: 
			-	DetectGoalReached_Distance3D in all cases.
		-	For the IDetectGoalChanged interface: 
			-	DetectGoalChanged_Distance3D in all cases.
		-	For the IDetectAccident interface: 
			-	DetectAccident_PredictMove in all cases.
		-	For the IDetectPathNodeReached interface: 
			-	DetectPathNodeReached_Distance3D in all cases.
		-	For the ICanGo interface: 
			-	CanGo_RayCast in all cases.
		-	For the ICheckDirectWay interface: 
			-	CheckDirectWay_Trivial in all cases.
		-	For the IComputeTargetPoint interface: 
			-	ComputeTargetPoint_Trivial in all cases.
		-	For the ISteering interface: 
			-	Steering_Fly in all cases.
		-	For the IEdgeStatusAwareness interface:
			-	#KY_NULL in all cases.
		-	For the ILpfShortcut interface:
			-	#KY_NULL in all cases.
		\param World&				The World in which this Modifiers instance will be used.
		\param[out] Modifiers&		The instance of Modifiers to set up with the default IModifier objects. */
	static void DeduceDefaultFlyModifiers(World&, Modifiers&);

	/*! \label_constructor Sets up the new Modifiers instance with the set of IModifier objects that will be used
		by the PathFinder. Parameters passed by reference indicate mandatory objects. Allocate these objects on the
		heap. The PathFinder will take ownership of the objects and manage their destruction transparently.
		Pointers may be set to #KY_NULL to set no object for the corresponding interface. */
	Modifiers(	IRefineGoal& iRefineGoal,
				IDetectGoalReached& iDetectGoalReached,
				IDetectGoalChanged& iDetectGoalChanged,
				IDetectAccident& iDetectAccident,
				ISelectPathNodeCandidate& iSelectPathNodeCandidate,
				IDetectPathNodeReached& iDetectPathNodeReached,
				ICanGo& iCanGo,
				ICheckDirectWay& iCheckDirectWay,
				IComputeTargetPoint& iComputeTargetPoint,
				IGoto& iGoto,
				ISteering& iSteering,
				IFindNodesFromPositions& iFindNodesFromPositions,
				ILpfShortcut* iLpfShortcut = KY_NULL,
				IEdgeStatusAwareness* iEdgeStatusAwareness = KY_NULL)
		:m_refineGoal(&iRefineGoal)
		,m_detectGoalReached(&iDetectGoalReached)
		,m_detectGoalChanged(&iDetectGoalChanged)
		,m_detectAccident(&iDetectAccident)
		,m_selectPathNodeCandidate(&iSelectPathNodeCandidate)
		,m_detectPathNodeReached(&iDetectPathNodeReached)
		,m_canGo(&iCanGo)
		,m_checkDirectWay(&iCheckDirectWay)
		,m_computeTargetPoint(&iComputeTargetPoint)
		,m_goto(&iGoto)
		,m_steering(&iSteering)
		,m_findNodesFromPositions(&iFindNodesFromPositions)
		,m_lpfShortcut(iLpfShortcut)
		,m_edgeStatusAwareness(iEdgeStatusAwareness)
	{
	}

	/*! \label_constructor If you use this constructor, you must call Set() for each type of IModifier. */
	Modifiers();

	/*! \label_destructor */
	~Modifiers();

	/*! Sets the instance of IRefineGoal maintained in this Modifiers object. */
	void Set(IRefineGoal& iRefineGoal);

	/*! Sets the instance of IDetectGoalReached maintained in this Modifiers object. */
	void Set(IDetectGoalReached& iDetectGoalReached);

	/*! Sets the instance of IDetectGoalChanged maintained in this Modifiers object. */
	void Set(IDetectGoalChanged& iDetectGoalChanged);

	/*! Sets the instance of IDetectAccident maintained in this Modifiers object. */
	void Set(IDetectAccident& iDetectAccident);

	/*! Sets the instance of ISelectPathNodeCandidate maintained in this Modifiers object. */
	void Set(ISelectPathNodeCandidate& iSelectPathNodeCandidate);

	/*! Sets the instance of IDetectPathNodeReached maintained in this Modifiers object. */
	void Set(IDetectPathNodeReached& iDetectPathNodeReached);

	/*! Sets the instance of ICanGo maintained in this Modifiers object. */
	void Set(ICanGo& iCanGo);

	/*! Sets the instance of ICheckDirectWay maintained in this Modifiers object. */
	void Set(ICheckDirectWay& iCheckDirectWay);

	/*! Sets the instance of IComputeTargetPoint maintained in this Modifiers object. */
	void Set(IComputeTargetPoint& iComputeTargetPoint);

	/*! Sets the instance of IGoto maintained in this Modifiers object. */
	void Set(IGoto& iGoto);

	/*! Sets the instance of ISteering maintained in this Modifiers object. */
	void Set(ISteering& iSteering);

	/*! Sets the instance of IFindNodesFromPositions maintained in this Modifiers object. */
	void Set(IFindNodesFromPositions& iFindNodesFromPositions);

	/*! Sets the instance of ILpfShortcut maintained in this Modifiers object. */
	void Set(ILpfShortcut* iLpfShortcut = KY_NULL);

	/*! Sets the instance of IEdgeStatusAwareness maintained in this Modifiers object. */
	void Set(IEdgeStatusAwareness* iEdgeStatusAwareness = KY_NULL);

	/*! Transfers the IModifier objects maintained in \c m to this instance of Modifiers. The current objects
		maintained in this instance are cleared, then overwritten by the objects maintained in \c m. Finally, the
		objects maintained in \c m are cleared. This instance of Modifiers will be responsible for managing the
		lifespan of the transferred objects. */
	void TransferOwnership(Modifiers& m); 


	/*! Retrieves the instance of IRefineGoal maintained in this Modifiers object. */
    const IRefineGoal& GetRefineGoal() const;

	/*! Retrieves the instance of IDetectGoalReached maintained in this Modifiers object. */
	const IDetectGoalReached& GetDetectGoalReached() const;

	/*! Retrieves the instance of IDetectGoalChanged maintained in this Modifiers object. */
	const IDetectGoalChanged& GetDetectGoalChanged() const;

	/*! Retrieves the instance of IDetectAccident maintained in this Modifiers object. */
	const IDetectAccident& GetDetectAccident() const;

	/*! Retrieves the instance of ISelectPathNodeCandidate maintained in this Modifiers object. */
	const ISelectPathNodeCandidate& GetSelectPathNodeCandidate() const;

	/*! Retrieves the instance of IDetectPathNodeReached maintained in this Modifiers object. */
	const IDetectPathNodeReached& GetDetectPathNodeReached() const;

	/*! Retrieves the instance of ICanGo maintained in this Modifiers object. */
	const ICanGo& GetCanGo() const;

	/*! Retrieves the instance of ICheckDirectWay maintained in this Modifiers object. */
	const ICheckDirectWay& GetCheckDirectWay() const;

	/*! Retrieves the instance of IComputeTargetPoint maintained in this Modifiers object. */
	const IComputeTargetPoint& GetComputeTargetPoint() const;

	/*! Retrieves the instance of IGoto maintained in this Modifiers object. */
	const IGoto& GetGoto() const;

	/*! Retrieves the instance of ISteering maintained in this Modifiers object. */
	const ISteering& GetSteering() const;

	/*! Retrieves the instance of IFindNodesFromPositions maintained in this Modifiers object. */
	const IFindNodesFromPositions& GetFindNodesFromPositions() const;

	/*! Retrieves the instance of ILpfShortcut maintained in this Modifiers object. */
	const ILpfShortcut* GetLpfShortcut() const;

	/*! Retrieves the instance of IEdgeStatusAwareness maintained in this Modifiers object. */
	const IEdgeStatusAwareness* GetEdgeStatusAwareness() const;

private:
	friend class PathFinder;

	Modifiers(Modifiers&); //< call TransferOwnership
	Modifiers& operator=(Modifiers&); //< call TransferOwnership

	void Erase(); //< KY_DELETE_CLASS on all modifiers
	void Remove(); //< pass all members to KY_NULL

	KyResult CheckModifierDependencies(const Kaim::ILpfManager::LpfContext*);
	void SetPathFinder(Kaim::PathFinder&);
	void ReInit();

	IRefineGoal* m_refineGoal; //< must not be modified once passed to the pathfinder
	IDetectGoalReached* m_detectGoalReached; //< must not be modified once passed to the pathfinder
	IDetectGoalChanged* m_detectGoalChanged; //< must not be modified once passed to the pathfinder
	IDetectAccident* m_detectAccident; //< must not be modified once passed to the pathfinder
	ISelectPathNodeCandidate* m_selectPathNodeCandidate; //< must not be modified once passed to the pathfinder
	IDetectPathNodeReached* m_detectPathNodeReached; //< must not be modified once passed to the pathfinder
	ICanGo* m_canGo; //< must not be modified once passed to the pathfinder
	ICheckDirectWay* m_checkDirectWay; //< must not be modified once passed to the pathfinder
	IComputeTargetPoint* m_computeTargetPoint; //< must not be modified once passed to the pathfinder
	IGoto* m_goto; //< must not be modified once passed to the pathfinder
	ISteering* m_steering; //< must not be modified once passed to the pathfinder
	IFindNodesFromPositions* m_findNodesFromPositions; //< must not be modified once passed to the pathfinder

	ILpfShortcut* m_lpfShortcut; //< must not be modified once passed to the pathfinder
	IEdgeStatusAwareness* m_edgeStatusAwareness; //< must not be modified once passed to the pathfinder
};


}

#endif // KY_MODIFIERS_H
