/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathobjects */

// primary contact: MAMU - secondary contact: None
#ifndef KYRUNTIME_BASE_PATHOBJECT_H
#define KYRUNTIME_BASE_PATHOBJECT_H

#include <kyruntime/pathobjects/pathobject.h>
#include <kyruntime/pathobjects/finitestatemachine_pathobject/basepathobjectfsm.h>
#include <kyruntime/pathobjects/offtheshelf/pathobjectbehaviorcontext.h>

namespace Kaim
{

class PathObjectQueue;

/*!	BasePathObject is an abstract base class for dynamic objects that derives from PathObject, and that adds a finite
	state machine to manage the phases a Bot uses to traverse the PathObject.
	\pp This class cannot be used as-is. You must use one of the derived classes supplied with the \SDKName SDK, or
	write your own custom class that derives from BasePathObject.
	\pp See :USERGUIDE:"Using PathObjects for Special Movements", :USERGUIDE:"The PathObject Framework", and
	:USERGUIDE:"Setting Up a Custom BasePathObject".
	\section usingapo Phases of traversing a BasePathObject
	The BasePathObject uses a finite state machine that divides the process of traversing a PathObject into four possible states:
	-	The \b approach state, in which the Bot first adopts the PathObject and carries out any preliminary actions that
		may be necessary in order to begin traversing the control space. For example, in the approach phase a teleporter 
		PathObject might direct the Bot toward the center of the teleport platform, or an elevator might direct the Bot to 
		go press a button on the wall, wait until the platform arrives, then enter the platform.
	-	The \b traversing state, in which the Bot moves through the control space from the place at which the traverse phase
		ceded control to the final destination point.
	-	The \b escape state, which allows a Bot to escape from the control space. This state allows a Bot to recover from
		problems encountered during the course of traversing the PathObject. It can also find a way out for a Bot that recomputes
		its path while traversing the PathObject, or a Bot that is spawned inside the control space.
	-	The \b blocked state may be entered if the escape state is unable to get the Bot out of the PathObject successfully.
	\pp Your PathObject can intervene at various points in this process, and control the conditions under which one state may
	transition to another, through the virtual methods provided by the IPathObjectBehavior class.
	\ingroup runtime_pathobjects */
class BasePathObject : public PathObject
{
public:
	/*	Mode in which the OnEnter is called.
		When the bot arrives inside the connexionSpace of a PathObject, the PathFinder calls
		OnEnter with mode Approach.
		When the bot is already inside a PathObject, the PathFinder calls OnEnter with mode Escape.*/
	enum EnterMode
	{
		EnterMode_Approach,
		EnterMode_Escape
	};

public:
	//////////////////////////////////////////////////////////////////////////////////////////
	// Constructor, Destructor, Initialization
	//////////////////////////////////////////////////////////////////////////////////////////

	/*!	\label_baseconstructor
		\param info				An instance of a class that derives from IPathObjectInfo, which specifies metadata
								about the PathObject, such as a unique ID and the name of the Database that the 
								PathObject will connect to.
		\param topology			An instance of a class that derives from IPathObjectTopology, which contains
								connection spaces that link the PathObject to the static \Graphs contained in a
								Database, and control spaces that demarcate the areas in which the PathObject has
								control over Bot movements.
		\param behavior			An instance of a class that derives from IPathObjectBehavior (typically from the
								BasePathObjectBehavior class supplied with \SDKName), which allows the PathObject to
								control the actions that a Bot carries out as it traverses the PathObject. */
	BasePathObject(IPathObjectInfo* info, IPathObjectTopology* topology, IPathObjectBehavior* behavior)
		: PathObject(info, topology), m_gameObject(KY_NULL), m_queue(KY_NULL) { SetBehavior(behavior); }

	/*!	\label_virtualdestructor */
	virtual ~BasePathObject() {}
	virtual void Render(PathObjectRenderMasks renderFlags) const;


	//////////////////////////////////////////////////////////////////////////////////////////
	// Behavior
	//////////////////////////////////////////////////////////////////////////////////////////

	void SetBehavior(IPathObjectBehavior* behavior) { m_behavior = behavior; }
	IPathObjectBehavior* GetBehavior() { return m_behavior; }
	virtual void RegisterBot(Bot* bot);
	virtual void UnregisterBot(Bot* bot);

	/*!	Called when a new PathObjectBehaviorContext has been created for a Bot using this PathObject.
		Implement this method if you want to store a value in the PathObjectBehaviorContext::m_userData member 
		(eg: animation frame, movement parameters, state variables, etc.). */
	virtual void OnPathObjectBehaviorAttachedToBot(PathObjectBehaviorContext* /*context*/, Bot* /*bot*/) {}

	/*!	Called when a PathObjectBehaviorContext is about to be detached from a Bot and erased.
		Implement this method if you have implemented OnPathObjectBehaviorAttachedToBot, in order to clear the 
		PathObjectBehaviorContext::m_userData if necessary. */
	virtual void OnPathObjectBehaviorDetachedFromBot(PathObjectBehaviorContext* /*context*/, Bot* /*bot*/) {}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Queue
	//////////////////////////////////////////////////////////////////////////////////////////

	virtual void SetQueue(PathObjectQueue* queue) { m_queue = queue; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Implement PathObject Path-finding / Path-following interface
	//////////////////////////////////////////////////////////////////////////////////////////

	virtual KyBool GetCost(Bot* bot, const PointWrapper& p1, const PointWrapper& p2,
		IConstraint& currentConstraint, KyFloat32& cost);

	virtual KyBool CanByPass(Bot& bot, const GraphVertexSafePtr& nodeId1, const GraphVertexSafePtr& nodeId2);

	virtual KyBool GoTo(Bot& bot, const PointWrapper& dest, Action& action, 
		KyBool& quitPathObject, PointWrapper& targetPoint, const GraphVertexSafePtr& nodeId1, const GraphVertexSafePtr& nodeId2, 
		KyUInt32& skippedNodeCount);

	virtual KyBool CanLeave(const Bot& bot, const GraphVertexSafePtr& nodeId1, const GraphVertexSafePtr& nodeId2);

	virtual void OnLeave(Bot& bot);

	virtual void OnEnter(Bot& bot, const GraphVertexSafePtr& entranceUid, const GraphVertexSafePtr& exitUid);

	virtual void OnPathRecomputed(Bot& bot, const GraphVertexSafePtr& entranceUid, const GraphVertexSafePtr& exitUid);

	virtual KyBool CheckAccident(const Bot& bot, KyBool& accidentDetected);

	virtual PathObject_FindStartNode_Status FindStartNode(Bot* bot, const PointWrapper& start, 
		const PointWrapper& dest, GraphVertexSafePtr& nearestNodeSafePtr);

public:
	void* m_gameObject;                                 /*!< A pointer to the object in the \gameOrSim engine that this PathObject represents. Set this value in the constructor of your PathObject class, if desired. */
	PathObjectQueue* m_queue;                           /*!< A queue associated with this pathObject.*/

protected:
	IPathObjectBehavior* m_behavior;                    /*!< The behavior that will control the bot inside the PathObject.*/
	List<PathObjectBehaviorContext> m_behaviorContexts; /*!< The list PathObjectBehaviorContext objects that correspond to the \Bots traversing the PathObject. Do not modify.*/
	BasePathObjectFSM m_defaultFiniteStateMachine;      /*!< Default PathObject Finite state machine.*/

};


} // namespace Kaim

#endif // KYRUNTIME_BASE_PATHOBJECT_H
