/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathobjects */

#ifndef KyPathData_IPathObject_Behavior_H
#define KyPathData_IPathObject_Behavior_H


#include "kypathdata/pathobject/pathobjectdatatypes.h"
#include "kypathdata/pathobject/ipathobjectconnexionspace.h"

namespace Kaim
{

class RuntimePathObjectConnexionSpace;
class Bot;
class PathObjectBehaviorContext;
class PointWrapper;
class IConstraint;
class PathObject;
class BasePathObject;

/*! Enumerates the possible return values for implementations of the IPathObjectBehavior::ComputeExitFromPos() method.
	\ingroup kyruntime_pathobjects */
enum IPathObjectBehavior_ComputeExitFromPos_Status
{
	PathObjectBehavior_ComputeExitFromPos_UnderProcess = 0, /*!< Indicates that the calculation is still in progress. The IPathObjectBehavior::ComputeExitFromPos() method will be called again in successive frames until it returns another result. */ 
	PathObjectBehavior_ComputeExitFromPos_NodeFound, /*!< Indicates that an exit connection space was successfully determined. */
	PathObjectBehavior_ComputeExitFromPos_NodeNotFound, /*!< Indicates that no exit connection space can be found. The PathFinder will use its IFindNodesFromPositions modifier to find the nearest accessible vertex. */
	PathObjectBehavior_ComputeExitFromPos_dummy_forceenumsize = KYFORCEENUMSIZEINT
};


/*! Provides an abstract base interface called by the BasePathObject class to determine how the PathObject
	should act at various points in time as a Bot traverses it.
	\pp This class is not usable as-is; you must use a derived class that implements this interface. If you use one
	of the off-the-shelf \PathObjects supplied with the SDK, you should use the class of IPathObjectBehavior supplied
	for use with that class of off-the-shelf PathObject. If you write your own custom class of PathObject, you should write
	your own custom implementation of IPathObjectBehavior to go with it.
	\pp The IPathObjectBehavior interface is completely empty. However, the \SDKName SDK does include a concrete implementation,
	BasePathObjectBehavior, that provides a good starting point for use with your custom PathObject classes. Simple \PathObjects
	can use the BasePathObjectBehavior class almost as-is: for example, the DoorPathObject supplied with \SDKName uses a
	DoorPathObjectBehavior class that does not change the default implementations in BasePathObjectBehavior at all. You will
	likely want to use BasePathObjectBehavior for your own PathObject class, and re-implement only selected methods in order to 
	customize it for your PathObject.
	\ingroup kyruntime_pathobjects */
class IPathObjectBehavior : public VirtualMemObject
{
public:
	/*! \label_virtualdestructor */
	virtual ~IPathObjectBehavior() {}

	/*! This method is expected to return the cost for the specified Bot to traverse a PathObject from the 
		specified entrance to the specified exit.
		\param Bot&								The Bot to consider in the calculation.
		\param BasePathObject&					The PathObject being traversed.
		\param RuntimePathObjectConnexionSpace&	The connection space the Bot will use to enter the PathObject.
		\param RuntimePathObjectConnexionSpace&	The connection space the Bot will use to exit the PathObject.
		\param PointWrapper&					The starting point for the cost calculation.
		\param PointWrapper&					The ending point for the cost calculation.
		\param IConstraint&						An IConstraint to take into account in the calculation.
		\param[out] KyFloat32&					Stores the cost calculated by this method.
		\return #KY_TRUE if the Bot can traverse the PathObject from the starting point to the ending point, and
				the KyFloat32& parameter was updated. #KY_FALSE if the Bot cannot traverse the PathObject at all (for
				example, if a door is locked, a bridge has been destroyed, a teleporter is disabled, etc.). */
	virtual KyBool GetCostFromEntranceToExit(
		Bot& /*entity*/,
		BasePathObject& /*pathObject*/,
		RuntimePathObjectConnexionSpace& /*entrance*/, RuntimePathObjectConnexionSpace& /*exit*/,
		const PointWrapper& /*from*/, const PointWrapper& /*to*/, IConstraint& /*currentConstraint*/,
		KyFloat32& /*cost*/) = 0;

	/*! This method is expected to return the cost for the specified Bot to traverse a PathObject from its 
		current position inside the PathObject to the specified ending point.
		\param Bot&								The Bot to consider in the calculation.
		\param BasePathObject&					The PathObject being traversed.
		\param Vec3f&							The current position of the Bot.
		\param RuntimePathObjectConnexionSpace&	The connection space the Bot will use to exit the PathObject.
		\param PointWrapper&					The starting point for the cost calculation.
		\param PointWrapper&					The ending point for the cost calculation.
		\param IConstraint&						An IConstraint to take into account in the calculation.
		\param[out] KyFloat32&					Stores the cost calculated by this method.
		\return #KY_TRUE if the Bot can traverse the PathObject from the starting point to the ending point, and
				the KyFloat32& parameter was updated. #KY_FALSE if the Bot cannot traverse the PathObject at all (for
				example, if a door is locked, a bridge has been destroyed, a teleporter is disabled, etc.). */
	virtual KyBool GetCostFromCurrentPosToExit(
		Bot& /*entity*/,
		BasePathObject& /*pathObject*/,
		const Vec3f& /*currentPosition*/, RuntimePathObjectConnexionSpace& /*exit*/,
		const PointWrapper& /*from*/, const PointWrapper& /*to*/, IConstraint& /*currentConstraint*/,
		KyFloat32& /*cost*/) = 0;

	/*! This method is expected to return the cost for the specified Bot to traverse a PathObject from its 
		current position inside the PathObject to another position inside the same PathObject.
		\param Bot&								The Bot to consider in the calculation.
		\param BasePathObject&					The PathObject being traversed.
		\param Vec3f&							The current position of the Bot.
		\param Vec3f&							The target position inside the PathObject.
		\param PointWrapper&					The starting point for the cost calculation.
		\param PointWrapper&					The ending point for the cost calculation.
		\param IConstraint&						An IConstraint to take into account in the calculation.
		\param[out] KyFloat32&					Stores the cost calculated by this method.
		\return #KY_TRUE if the Bot can traverse the PathObject from the starting point to the ending point, and
				the KyFloat32& parameter was updated. #KY_FALSE if the Bot cannot traverse the PathObject at all (for
				example, if a door is locked, a bridge has been destroyed, a teleporter is disabled, etc.). */
	virtual KyBool GetCostFromCurrentPosToTargetPos(
		Bot& /*entity*/,
		BasePathObject& /*pathObject*/,
		const Vec3f& /*currentPosition*/, const Vec3f& /*targetPosition*/,
		const PointWrapper& /*from*/, const PointWrapper& /*to*/, IConstraint& /*currentConstraint*/,
		KyFloat32& /*cost*/) = 0;

	/*! This method is expected to return the cost for the specified Bot to traverse a PathObject from the 
		specified entrance to a position inside the same PathObject.
		\param Bot&								The Bot to consider in the calculation.
		\param BasePathObject&					The PathObject being traversed.
		\param RuntimePathObjectConnexionSpace&	The connection space the Bot will use to enter the PathObject.
		\param Vec3f&							The target position inside the PathObject.
		\param PointWrapper&					The starting point for the cost calculation.
		\param PointWrapper&					The ending point for the cost calculation.
		\param IConstraint&						An IConstraint to take into account in the calculation.
		\param[out] KyFloat32&					Stores the cost calculated by this method.
		\return #KY_TRUE if the Bot can traverse the PathObject from the starting point to the ending point, and
				the KyFloat32& parameter was updated. #KY_FALSE if the Bot cannot traverse the PathObject at all (for
				example, if a door is locked, a bridge has been destroyed, a teleporter is disabled, etc.). */
	virtual KyBool GetCostFromEntranceToTargetPos(
		Bot& /*entity*/,
		BasePathObject& /*pathObject*/,
		RuntimePathObjectConnexionSpace& /*entrance*/, const Vec3f& /*targetPosition*/,
		const PointWrapper& /*from*/, const PointWrapper& /*to*/, IConstraint& /*currentConstraint*/,
		KyFloat32& /*cost*/) = 0;

	/*!	This method is called during path following in order to determine whether or not the Bot can skip 
		entering the PathObject through its entrance connection space, and proceed directly to the exit
		connection space. This can be useful to make a PathObject send a Bot across the borders of the 
		NavMesh without supplying a custom trajectory.
		\pp If PathObject::CanSmoothIndependentlyOfEdge() returns #KY_TRUE to indicate that the PathObject
		is smoothable, this method is not called. See also :USERGUIDE:"PathObjects and Path Smoothing".
		\param Bot&				The Bot attempting to skip to the exit.
		\param BasePathObject&	The PathObject being traversed.
		\return \c true if the Bot can skip directly to the exit, or \c false if the Bot must enter the
				PathObject through the connection space. */
	virtual bool CanByPassEntrance(Bot& /*entity*/, BasePathObject& /*pathObject*/) = 0;

	/*! This method is called when a Bot re-calculates its path while traversing a PathObject. It is expected to 
		determine the best exit for the Bot to use in order to leave the PathObject heading toward the specified destination.
		\param Bot&										The Bot that needs to exit the PathObject.
		\param BasePathObject&							The PathObject being traversed.
		\param Vec3f&									The current position of the Bot inside the PathObject.
		\param PointWrapper&							The ultimate destination point the Bot is heading for.
		\param[out] RuntimePathObjectConnexionSpace*&	Stores the exit connection space calculated by this method.
		\return An element from the #IPathObjectBehavior_ComputeExitFromPos_Status enumeration that indicates the current
				status of the calculation. */
	virtual IPathObjectBehavior_ComputeExitFromPos_Status ComputeExitFromPos(
		Bot& /*entity*/, BasePathObject& /*pathObject*/,
		const Vec3f& /*startPos*/, const PointWrapper& /*dest*/,
		const RuntimePathObjectConnexionSpace*& /*exit*/) = 0;

	/*!	Called once when a Bot first begins using the PathObject, whether by entering through a connection space or by
		attempting to escape the control space of the PathObject.
		\param[in, out] PathObjectBehaviorContext&	The context that maintains the current set of information about the
													Bot, the PathObject, and the way the Bot traverses the PathObject. */
	virtual void OnEnterPathObject(PathObjectBehaviorContext& /*context*/) = 0;

	/*!	Called once when a Bot finally leaves the PathObject.
		\param[in, out] PathObjectBehaviorContext&	The context that maintains the current set of information about the
													Bot, the PathObject, and the way the Bot traverses the PathObject. */
	virtual void OnLeavePathObject(PathObjectBehaviorContext& /*context*/) = 0;

	/*!	Called once when a Bot begins its approach state.
		\param[in, out] PathObjectBehaviorContext&	The context that maintains the current set of information about the
													Bot, the PathObject, and the way the Bot traverses the PathObject. */
	virtual void OnStartApproaching(PathObjectBehaviorContext& /*context*/) = 0;

	/*!	Called at each frame while a Bot is in its approach state, to carry out whatever movements are needed in order to
		approach the PathObject. This method is called until one of the following conditions is true:
		-	CanStartTraverse() returns true. 
		-	ApproachFailed() returns true. 
		\pp \param[in, out] PathObjectBehaviorContext&	The context that maintains the current set of information about the
														Bot, the PathObject, and the way the Bot traverses the PathObject. */
	virtual void Approach(PathObjectBehaviorContext& /*context*/) = 0;

	/*!	Called once when the behavior switches out of its approach state.
		\param[in, out] PathObjectBehaviorContext&	The context that maintains the current set of information about the
													Bot, the PathObject, and the way the Bot traverses the PathObject. */
	virtual void OnEndApproaching(PathObjectBehaviorContext& /*context*/) = 0;

	/*!	Called at each frame while the Bot is in its approach state, to detect problems with the approach. If this method
		returns \c true, OnEndApproaching() is called, then the Bot quits the PathObject and requests a path re-computation
		from the PathFinder.
		\param[in, out] PathObjectBehaviorContext&	The context that maintains the current set of information about the
													Bot, the PathObject, and the way the Bot traverses the PathObject. */
	virtual bool ApproachFailed(PathObjectBehaviorContext& /*context*/) = 0;

	/*!	Called at each frame while the Bot is in its approach state, to detect whether or not the Bot can begin its
		traversing state. If this methods returns \c true, OnEndApproaching() is called, then the behavior switches to 
		the traversing state.
		\param[in, out] PathObjectBehaviorContext&	The context that maintains the current set of information about the
													Bot, the PathObject, and the way the Bot traverses the PathObject. */
	virtual bool CanStartTraverse(PathObjectBehaviorContext& /*context*/) = 0;

	/*!	Called at each frame while a Bot is in its traversing state, to carry out whatever movements are needed in order to
		actually traverse the PathObject. This method is called until the Bot reaches its final destination. 
		\pp \param[in, out] PathObjectBehaviorContext&	The context that maintains the current set of information about the
														Bot, the PathObject, and the way the Bot traverses the PathObject. */
	virtual void Traverse(PathObjectBehaviorContext& /*context*/) = 0;

	/*!	Called once when the behavior enters its traversing state.
		\param[in, out] PathObjectBehaviorContext&	The context that maintains the current set of information about the
													Bot, the PathObject, and the way the Bot traverses the PathObject. */
	virtual void OnStartTraversing(PathObjectBehaviorContext& /*context*/) = 0;

	/*!	Called once when the behavior switches out of its traversing state.
		\param[in, out] PathObjectBehaviorContext&	The context that maintains the current set of information about the
													Bot, the PathObject, and the way the Bot traverses the PathObject. */
	virtual void OnEndTraversing(PathObjectBehaviorContext& /*context*/) = 0;

	/*!	Called at each frame while a Bot is in its traversing and escaping states, to determine whether or not the Bot has
		reached its final destination. If this method returns \c true, the OnEndTraversing() or OnEndEscaping() method is
		called depending on the current state, and the Bot quits the PathObject.
		\pp \param[in, out] PathObjectBehaviorContext&	The context that maintains the current set of information about the
														Bot, the PathObject, and the way the Bot traverses the PathObject. */
	virtual bool ExitReached(PathObjectBehaviorContext& /*context*/) = 0;

	/*!	Called at each frame while a Bot is in its escaping state, to lead the Bot toward an exit. 
		\pp \param[in, out] PathObjectBehaviorContext&	The context that maintains the current set of information about the
														Bot, the PathObject, and the way the Bot traverses the PathObject. */
	virtual void Escape(PathObjectBehaviorContext& /*context*/) = 0;

	/*!	Called once when the behavior enters its escaping state.
		\param[in, out] PathObjectBehaviorContext&	The context that maintains the current set of information about the
													Bot, the PathObject, and the way the Bot traverses the PathObject. */
	virtual void OnStartEscaping(PathObjectBehaviorContext& /*context*/) = 0;

	/*!	Called once when the behavior switches out of its escaping state.
		\param[in, out] PathObjectBehaviorContext&	The context that maintains the current set of information about the
													Bot, the PathObject, and the way the Bot traverses the PathObject. */
	virtual void OnEndEscaping(PathObjectBehaviorContext& /*context*/) = 0;

	/*!	Called at each frame while a Bot is in its escaping state, to determine whether or not the Bot can escape the
		PathObject by using its current exit point. If this method returns \c false and FindAnotherExit() returns \c
		false, then the OnEndEscaping() method is called and the behavior enters the blocked state.
		\param[in, out] PathObjectBehaviorContext&	The context that maintains the current set of information about the
													Bot, the PathObject, and the way the Bot traverses the PathObject. */
	virtual bool CanEscapeThroughExit(PathObjectBehaviorContext& /*context*/) = 0;

	/*!	Called at each frame while a Bot is in its escaping state and CanEscapeThroughExit() returns \c false, in order 
		to find a different exit point for the Bot to target in order to leave the control space of the PathObject. If
		this method returns \c false, then the OnEndEscaping() method is called and the behavior enters the blocked state.
		\param[in, out] PathObjectBehaviorContext&	The context that maintains the current set of information about the
													Bot, the PathObject, and the way the Bot traverses the PathObject. */
	virtual bool FindAnotherExit(PathObjectBehaviorContext& /*context*/) = 0;

	/* \param[in, out] PathObjectBehaviorContext&	The context that maintains the current set of information about the
													Bot, the PathObject, and the way the Bot traverses the PathObject. */
	virtual bool EscapeTargetReached(PathObjectBehaviorContext& /*context*/) = 0;

	/*!	Called once when the behavior enters its blocked state.
		\param[in, out] PathObjectBehaviorContext&	The context that maintains the current set of information about the
													Bot, the PathObject, and the way the Bot traverses the PathObject. */
	virtual void OnBotBlocked(PathObjectBehaviorContext& /*context*/) = 0;

	/*!	Called at each frame while a Bot is in its blocked state.
		\param[in, out] PathObjectBehaviorContext&	The context that maintains the current set of information about the
													Bot, the PathObject, and the way the Bot traverses the PathObject. */
	virtual void BlockBot(PathObjectBehaviorContext& /*context*/) = 0;

	/*!	Called once when the behavior switches out of its blocked state.
		\param[in, out] PathObjectBehaviorContext&	The context that maintains the current set of information about the
													Bot, the PathObject, and the way the Bot traverses the PathObject. */
	virtual void OnUnblockBot(PathObjectBehaviorContext& /*context*/) = 0;

	/*!	Called at each frame while the Bot is in its blocked state, to determine whether or not the Bot has been unblocked.
		If this method returns \c true, the Bot exits the PathObject. If this method returns \c false, the Bot stays in its
		blocked state until the next frame.
		\param[in, out] PathObjectBehaviorContext&	The context that maintains the current set of information about the
													Bot, the PathObject, and the way the Bot traverses the PathObject. */
	virtual bool UnblockedOutsidePathObject(PathObjectBehaviorContext& /*context*/) = 0;

	/*!	Renders data about the behavior.*/
	virtual void Render(const PathObjectBehaviorContext& /*context*/) const = 0;
};

}

#endif // #ifndef KyPathData_IPathObject_Behavior_H
