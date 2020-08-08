/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathobjects */

#ifndef KyPathData_Base_PathObject_Behavior_H
#define KyPathData_Base_PathObject_Behavior_H

#include <kyruntime/pathobjects/ipathobjectbehavior.h>

namespace Kaim
{

class IMove;
class BasePathObject;

/*!	The BasePathObjectBehavior class provides a basic implementation of IPathObjectBehavior that is
	suitable for use with simple PathObjects. You can use it as a starting point for your own custom
	implementations of IPathObjectBehavior: simply re-implement the methods that you need to in order to
	make your PathObject class behave the way you want.
	\pp This class adds one important element to the base IPathObjectBehavior: it uses a class that
	implements the IMove interface to generate an Action that makes the Bot move toward its target point. 
	You must provide an instance of a class that implements IMove in the constructor of BasePathObjectBehavior.
	\pp This base implementation makes the Bot use the PathObject as follows:
	-	When the Bot first adopts the PathObject, the PathFinder is used to drive it toward the position of its
		entrance connection space.
	-	When that position is reached, the IMove class is invoked at each frame to create an Action that moves the
		Bot toward the position of its exit connection space. The speed used when traversing and escaping the PathObject
		are derived by multiplying the target speed currently set for the PathFinder by the factors returned by the
		GetTraverseSpeedFactor() and GetEscapeSpeedFactor() methods. By default these methods return 1.0f, but you
		can customize the speed factors by re-implementing these methods in your own derived class.
	\pp Only one Bot is allowed to traverse the PathObject at a time. Other \Bots are forced to wait until the Bot
	currently traversing the PathObject is finished.
	\ingroup runtime_pathobjects */
class BasePathObjectBehavior : public IPathObjectBehavior
{
public:
	/*! \label_constructor
		Sets #m_move to #KY_NULL. Before this BasePathObjectBehavior can make a Bot traverse a PathObject, you
		must update #m_move yourself to a valid pointer. */
	BasePathObjectBehavior() : m_move(KY_NULL) {}

	/*! \label_constructor
		\param move			An instance of a class that derives from IMove, which the BasePathObjectBehavior
							will use to generate \Actions to make the Bot traverse the PathObject. The
							BasePathObjectBehavior does not take responsibility for the lifespan of this object;
							you must ensure that it remains in memory until after you have finished using your
							BasePathObjectBehavior. */
	explicit BasePathObjectBehavior(IMove* move) : m_move(move) {}
	
	/*! \label_virtualdestructor */
	virtual ~BasePathObjectBehavior() {}

	virtual KyBool GetCostFromEntranceToExit(
		Bot& /*bot*/,
		BasePathObject& /*pathObject*/,
		RuntimePathObjectConnexionSpace& /*entrance*/, RuntimePathObjectConnexionSpace& /*exit*/,
		const PointWrapper& /*from*/, const PointWrapper& /*to*/, IConstraint& /*currentConstraint*/,
		KyFloat32& /*cost*/);
	virtual KyBool GetCostFromCurrentPosToExit(
		Bot& /*bot*/,
		BasePathObject& /*pathObject*/,
		const Vec3f& /*currentPosition*/, RuntimePathObjectConnexionSpace& /*exit*/,
		const PointWrapper& /*from*/, const PointWrapper& /*to*/, IConstraint& /*currentConstraint*/,
		KyFloat32& /*cost*/);
	virtual KyBool GetCostFromCurrentPosToTargetPos(
		Bot& /*bot*/,
		BasePathObject& /*pathObject*/,
		const Vec3f& /*currentPosition*/, const Vec3f& /*targetPosition*/,
		const PointWrapper& /*from*/, const PointWrapper& /*to*/, IConstraint& /*currentConstraint*/,
		KyFloat32& /*cost*/);
	virtual KyBool GetCostFromEntranceToTargetPos(
		Bot& /*bot*/,
		BasePathObject& /*pathObject*/,
		RuntimePathObjectConnexionSpace& /*entrance*/, const Vec3f& /*targetPosition*/,
		const PointWrapper& /*from*/, const PointWrapper& /*to*/, IConstraint& /*currentConstraint*/,
		KyFloat32& /*cost*/);

	/*! \copydoc IPathObjectBehavior::CanByPassEntrance()
		This implementation returns \c false in all cases. */
	virtual bool CanByPassEntrance(Bot& /*bot*/, BasePathObject& /*pathObject*/) { return false; }

	/*! \copydoc IPathObjectBehavior::CanByPassEntrance()
		This implementation selects the connection space that lies closest to the destination point. */
	virtual IPathObjectBehavior_ComputeExitFromPos_Status ComputeExitFromPos(
		Bot& /*bot*/, BasePathObject& /*pathObject*/,
		const Vec3f& /*startPos*/, const PointWrapper& /*dest*/,
		const RuntimePathObjectConnexionSpace*& /*exit*/);

	virtual void OnEnterPathObject(PathObjectBehaviorContext& /*context*/) {}
	virtual void OnLeavePathObject(PathObjectBehaviorContext& /*context*/) {}

	virtual void OnStartApproaching(PathObjectBehaviorContext& /*context*/) {}

	/*! \copydoc IPathObjectBehavior::Approach()
		This implementation makes the Bot use the PathFinder to move toward the vertex created at the center of the 
		entrance connection space. */
	virtual void Approach(PathObjectBehaviorContext& /*context*/);
	virtual void OnEndApproaching(PathObjectBehaviorContext& /*context*/) {}
	virtual bool ApproachFailed(PathObjectBehaviorContext& /*context*/) { return false; }

	/*! \copydoc IPathObjectBehavior::CanStartTraverse()
		This implementation allows the Bot to enter its traversing state once the PathFinder has completed
		directing the Bot to the vertex at the center of the entrance connection space. */
	virtual bool CanStartTraverse(PathObjectBehaviorContext& /*context*/);

	/*! \copydoc IPathObjectBehavior::Traverse()
		This implementation invokes the IMove::Move() method of the IMove in order to move the Bot toward 
		the vertex at the center of the exit connection space at half of the target speed set for the PathFinder. */
	virtual void Traverse(PathObjectBehaviorContext& /*context*/);

	/*! \copydoc IPathObjectBehavior::OnStartTraversing()
		This implementation invokes the IMove::BeforeMove() method of the IMove in order to begin moving the Bot toward 
		the vertex at the center of the exit connection space at half of the target speed set for the PathFinder. */
	virtual void OnStartTraversing(PathObjectBehaviorContext& /*context*/);

	/*! \copydoc IPathObjectBehavior::OnEndTraversing()
		This implementation invokes the IMove::AfterMove() method of the IMove in order to finish moving the Bot. */
	virtual void OnEndTraversing(PathObjectBehaviorContext& /*context*/);

	/*! \copydoc IPathObjectBehavior::ExitReached()
		This implementation returns \c true when the Bot reaches its target point, whether its movements are controlled by the
		PathFinder or by the IMove. */
	virtual bool ExitReached(PathObjectBehaviorContext& /*context*/);

	/*! \copydoc IPathObjectBehavior::Escape()
		This implementation invokes the IMove::Move() method of the IMove in order to move the Bot toward 
		the vertex at the center of the exit connection space at twice the target speed set for the PathFinder. */
	virtual void Escape(PathObjectBehaviorContext& /*context*/);

	/*! \copydoc IPathObjectBehavior::OnStartEscaping()
		This implementation invokes the IMove::BeforeMove() method of the IMove in order to begin moving the Bot toward 
		the vertex at the center of the exit connection space at twice the target speed set for the PathFinder. */
	virtual void OnStartEscaping(PathObjectBehaviorContext& /*context*/);

	/*! \copydoc IPathObjectBehavior::OnEndTraversing()
		This implementation invokes the IMove::AfterMove() method of the IMove in order to finish moving the Bot. */
	virtual void OnEndEscaping(PathObjectBehaviorContext& /*context*/);

	/*! \copydoc IPathObjectBehavior::CanEscapeThroughExit()
		This implementation returns \c false if any \Obstacles are blocking the exit connection space. */
	virtual bool CanEscapeThroughExit(PathObjectBehaviorContext& /*context*/);

	/*! \copydoc IPathObjectBehavior::FindAnotherExit()
		This implementation attempts to target the entrance connection space. It returns \c false if any \Obstacles are 
		blocking the entrance connection space. */
	virtual bool FindAnotherExit(PathObjectBehaviorContext& /*context*/);

	/*! \copydoc IPathObjectBehavior::EscapeTargetReached()
		This implementation returns \c true when the Bot reaches its target point, whether its movements are controlled by the
		PathFinder or by the IMove. */
	virtual bool EscapeTargetReached(PathObjectBehaviorContext& /*context*/);

	virtual void OnBotBlocked(PathObjectBehaviorContext& /*context*/) {}
	virtual void BlockBot(PathObjectBehaviorContext& /*context*/) {}
	virtual void OnUnblockBot(PathObjectBehaviorContext& /*context*/) {}
	virtual bool UnblockedOutsidePathObject(PathObjectBehaviorContext& /*context*/) { return false; }

	virtual void Render(const PathObjectBehaviorContext& context) const;

	/*! Returns a factor by which the target speed of the PathFinder will be multiplied while the Bot is traversing the PathObject. 
		If you want to customize the speed at which your Bot traverses the PathObject, you can re-implement this method in a
		derived class. */
	virtual KyFloat32 GetTraverseSpeedFactor() const { return 1.0f; }

	/*! Returns a factor by which the target speed of the PathFinder will be multiplied while the Bot is escaping the PathObject. 
		If you want to customize the speed at which your Bot traverses the PathObject, you can re-implement this method in a
		derived class. */
	virtual KyFloat32 GetEscapeSpeedFactor() const { return 1.0f; }

public:
	/*! Read accessor for #m_move. */
	IMove* GetMove() { return m_move; }

public:
	IMove* m_move; /*!< Stores a pointer to the IMove object that this BasePathObjectBehavior will use to generate an Action that makes a Bot traverse the PathObject. */
};

}

#endif // #ifndef KyPathData_Base_PathObject_Behavior_H
