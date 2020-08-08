/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathobjects */

#ifndef KyPathData_PathObject_Behavior_Context_H
#define KyPathData_PathObject_Behavior_Context_H

#include "kypathdata/math/vec3f.h"

namespace Kaim
{

class Action;
class Bot;
class PathFinder;
class PathObject;
class IState;
class RuntimePathObjectConnexionSpace;

/*! Each instance of this class stores a record of the way a single Bot is using a single BasePathObject.
	It is an intermediary between the BasePathObject and the IPathObjectBehavior. 
	\pp The BasePathObject internally creates an instance of this class for each Bot that adopts the PathObject.
	It sets some information in this object, such as pointers to the Bot and the PathObject, the connection spaces
	at which the Bot enters and exits the PathObject, etc. It then passes the context to the methods of the
	IPathObjectBehavior class. These methods in turn can update certain flags and data members in the context
	to instruct the PathObject how it should control the Bot.
	\ingroup kyruntime_pathobjects */
class PathObjectBehaviorContext : public VirtualMemObject
{
public:
	/*! \label_constructor For internal use. */
	PathObjectBehaviorContext() :
		m_userData(KY_NULL),
		m_bot(KY_NULL),
		m_pathFinder(KY_NULL),
		m_pathObject(KY_NULL),
		m_startPosition(),
		m_targetPosition(),
		m_startConnexionSpace(KY_NULL),
		m_targetConnexionSpace(KY_NULL),
		m_pathFinderTargetPosReached(false),
		m_currentState(KY_NULL),
		m_quitPathObject(false),
		m_pathFinderDrives(false),
		m_requestedAction(KY_NULL),
		m_moveContext(KY_NULL),
		m_currentActionIsInterruptible(true),
		m_canDetectAccident(false),
		m_accidentDetected(false)
		{}
	~PathObjectBehaviorContext() {}

public:
	/*! Stores information added to this context by the user. This value is typically updated and cleared in
		custom implementations of BasePathObject::OnPathObjectBehaviorAttachedToBot() and 
		BasePathObject::OnPathObjectBehaviorDetachedFromBot(). */
	void* m_userData;

//////////////////////////////////////////////////////////////////////////
// Input Data that Kynapse maintains up to date.
//////////////////////////////////////////////////////////////////////////

	Bot* m_bot;                         /*!< Set by the BasePathObject. Identifies the Bot using the PathObject. Do not modify. */
	PathFinder* m_pathFinder;           /*!< Set by the BasePathObject. Identifies the PathFinder of the Bot using the PathObject. Do not modify. */
	PathObject* m_pathObject;           /*!< Set by the BasePathObject. Identifies the PathObject in use. Do not modify. */

	Vec3f m_startPosition;  /*!< Set by the BasePathObject. Identifies the position at which the Bot started using the PathObject. Do not modify. */
	Vec3f m_targetPosition; /*!< Set by the BasePathObject. Identifies the target position at which the Bot will quit the PathObject. Do not modify. */
	RuntimePathObjectConnexionSpace* m_startConnexionSpace;  /*!< The connection space in which the Bot entered the PathObject, or #KY_NULL if the Bot started inside the control space. */
	RuntimePathObjectConnexionSpace* m_targetConnexionSpace; /*!< The connection space in which the Bot will leave the PathObject, or #KY_NULL if the Bot is going to a location inside the control space. */

	/*! Set by the BasePathObject. When #m_pathFinderDrives is set to \c true, this value indicates whether or not the PathFinder
		has reached #m_pathFinderTargetPos. This is evaluated by using the IDetectPathNodeReached modifier configured for the
		PathFinder of the Bot using the context. Do not modify. */
	bool m_pathFinderTargetPosReached;  

	IState* m_currentState;             /*!< Set by the BasePathObject. Indicates the current state of the PathObject finite state machine. Do not modify. */

//////////////////////////////////////////////////////////////////////////
// Output data that behavior/user must set to tell Kynapse what to do.
//////////////////////////////////////////////////////////////////////////

	/*! Set by the IPathObjectBehavior, to indicate whether or not the Bot has finished using the PathObject and
		should return to the control of the PathFinder. 
		-	If \c true, the Bot will leave the PathObject and attempt to rejoin its original path through the Graph.
		-	If \c false, the Bot will continue using the PathObject in the next frame. */
	bool m_quitPathObject;

	///// PathFinder drives
	/*! Set by the IPathObjectBehavior, to indicate whether or not the PathFinder should be used to move the Bot. 
		-	If \c true, the PathFinder will be invoked to generate an Action that moves the Bot to the position specified in #m_pathFinderTargetPos. 
		-	If \c false, the Action stored in #m_requestedAction will be applied to the Bot. */
	bool m_pathFinderDrives;

	/*!	Set by the IPathObjectBehavior, to indicate the position the PathFinder should direct the Bot to when #m_pathFinderDrives is set to \c true. */
	Vec3f m_pathFinderTargetPos;

	///// User Drives
	/*! Set by the IPathObjectBehavior, to provide an Action that should be applied to the Bot in order to make it traverse
		the PathObject in the current frame. The IPathObjectBehavior is responsible for setting up the ActionAttributes of this
		Action. Read only when #m_pathFinderDrives is \c false. */
	Action* m_requestedAction;

	/*! Set by the IPathObjectBehavior, to provide additional contextual information if necessary. For example, the
		initial position of a jump. */
	void* m_moveContext;

	///// Path recomputation
	/*! Set by the IPathObjectBehavior, to indicate whether or not the Bot can leave this PathObject in order to smooth its path ro
		a path node farther along the path, or to carry out a path re-computation request. 
		-	If \c true, the PathFinder of the Bot will be allowed to re-compute its path if needed, or to smooth to a later path node.
		-	If \c false, path re-computations and smoothing are not allowed until the Bot quits the PathObject or until this value changes
			to \c true. 
		\pp In order for the Bot to be able to leave the PathObject by smoothing its path to a path node farther along the path, 
		IPathObjectBehavior::CanByPassEntrance() must also return \c true. See also :USERGUIDE:"PathObjects and Path Smoothing". */
	bool m_currentActionIsInterruptible;

	///// Accident
	/*! Set by the IPathObjectBehavior, to indicate whether or not the behavior has the ability to detect accidents that occur
		as the Bot traverses the PathObejct.
		-	If \c true, the BasePathObject checks #m_accidentDetected to determine whether or not an accident has occurred. If
			an accident has occurred, the Bot will re-calculate its path.
		-	If \c false, the PathFinder will carry out its own accident detection using its IDetectAccident modifier. */
	bool m_canDetectAccident;

	/*! Set by the IPathObjectBehavior, to indicate whether or not an accident has occurred. Read only when #m_canDetectAccident is \c true. */
	bool m_accidentDetected;
};

} // namespace Kaim

#endif // #ifndef KyPathData_PathObject_Behavior_Context_H
