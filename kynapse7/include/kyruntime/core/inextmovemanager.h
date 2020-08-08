/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_core */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_INEXTMOVEMANAGER_H
#define KY_INEXTMOVEMANAGER_H

#include <kyruntime/core/service.h>
#include <kyruntime/core/action.h>

#include <kypathdata/math/vec3futils.h>

#include <kyruntime/graph/graphvertexptr.h>
#include <kyruntime/graph/graphedgeptr.h>


namespace Kaim
{

class Vec3f;

class Bot;
class PointWrapper;


////////////////////////////////////////////////////////////////////////////////////////////////////////
//		Pathfinding Constraint and Heuristic
////////////////////////////////////////////////////////////////////////////////////////////////////////

class IPathFinder;
class IConstraint;
class IHeuristic;
class AstarContext;


/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from IConstraint. Each class that derives from 
	IConstraint must contain a constructor function that matches this type definition. The argument list
	of this type definition is empty, which indicates that the default constructor of your derived class
	will be invoked.
	\ingroup kyruntime_core */
typedef IConstraint* (*IConstraintConstructor)();

/*!	IConstraintClass is the MetaClass for IConstraint, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_core */
class IConstraintClass: public MetaClass<IConstraint, IConstraintConstructor>
{
public:
	/*!	Constructor for the IConstraint MetaClass. For internal use. */
	IConstraintClass(const char* className, IConstraintConstructor ctr, 
		IConstraintClass* pBaseClass = (IConstraintClass*) NULL);
};

/*!	IConstraint is the base class for objects that determine the cost of moving a Bot from one point 
	to another in order to control the characteristics of paths calculated and followed by the PathFinder. For example, 
	one class of IConstraint might produce the shortest path, another the stealthiest path, and another the path that 
	requires the least travel through enemy territory.
	\section constraintpathcalc Path calculation
	\pp Each time the A* algorithm considers an edge in the spatial graph, it passes the edge to an object that implements IConstraint, 
	which evaluates the cost of following that edge and returns a result. The costs of all edges in each possible path 
	are added together to determine which possible path offers the lowest cost according to the current IConstraint. 
	\pp See also :USERGUIDE:"Path Calculation".
	\section constraintpathfollow Path following
	The PathFinder also uses constraints to optimize trajectories while following a path. For example, when it 
	evaluates whether or not its Bot should shortcut directly to a PathNode farther along its Path, it will use 
	the current IConstraint to determine whether or not the direct path actually represents a shortcut relative to following 
	the pre-planned path through the spatial graph. 
	\pp For example, if a stealthy Bot uses an IConstraint class that places additional costs on moving through lighted areas, the 
	PathFinder will avoid taking shortcuts that make it leave the shadows. 
	\pp See also :USERGUIDE:"Path Following".
	\section writingcustomconstraint Writing a custom IConstraint class
	Writing a custom class that derives from IConstraint involves the following considerations.
	\subsection contraintconst Constructor
	All classes that derive from IConstraint must have a constructor that follows the #IConstraintConstructor type definition. 
	\subsection constraintvirtualmethods Virtual methods
	All IConstraint classes must implement the GetCost() method.
	\pp You may also implement any other virtual methods of this class if you choose, or use the default implementations.
	\pp If you choose to implement the OnStartAstar(), OnFinishAstar, OnPauseAstar() and/or OnResumeAstar() methods to carry out tasks at
	specific moments during the path calculation process, your implementations must take into account the fact that paused A* 
	calculations are not necessarily resumed. For example, if a path needs to be re-calculated before its initial A* calculation 
	has completed (for example, due to a GraphCell being unloaded), the calculation may be restarted instead. In this case, 
	OnFinishAstar() is called to notify the IConstraint that the paused calculation has been terminated, and OnStartAstar() is 
	called again when the next calculation is begun.
	\subsection constraintmetaclassmacros MetaClass macros
	In order for \SDKName to recognize your IConstraint class, you must include the following macro definitions:
	-	#KY_DECLARE_CONSTRAINT: Place this macro in the declaration of your new class. This macro takes one argument:
		the name of your new class.
	-	#KY_IMPLEMENT_CONSTRAINT: Place this macro in the implementation file of your new class. This macro takes the 
		following arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the IConstraint class from which your class derives, if different from 
			IConstraint. For example, &Kaim::SphereConstraint::Class. If your class derives directly from 
			IConstraint, the value of this argument should be #KY_NULL.
	\ingroup kyruntime_core */
class IConstraint: public VirtualMemObject
{
protected:
	/*!	\label_baseconstructor */
	IConstraint(): m_astarContext(KY_NULL) {}

public:
	/*!	\label_getclass */
	virtual IConstraintClass& GetClass() = 0;

	/*!	\label_virtualdestructor */
	virtual ~IConstraint() {}

	/*!	This method is called once immediately before starting an A* computation that uses this IConstraint.
		\param GraphVertexPtr			The starting vertex for the computation.
		\param GraphVertexPtr			The ending vertex for the computation. */
	virtual void OnStartAstar(const GraphVertexPtr& /*start*/, const GraphVertexPtr& /*dest*/) {}
	
	/*!	This method is called once when an A* computation that uses this IConstraint is completed.
		\param GraphVertexPtr			The starting vertex for the computation.
		\param GraphVertexPtr			The ending vertex for the computation. */
	virtual void OnFinishAstar(const GraphVertexPtr& /*start*/, const GraphVertexPtr& /*dest*/) {}
	
	/*!	This method is called once when an A* computation that uses this IConstraint is paused in order to
		check its CPU usage.
		\pp Your implementation of this method must leave your IConstraint in a temporary idle state from which 
		its calculations may be either resumed or restarted.
		\param GraphVertexPtr			The starting vertex for the computation.
		\param GraphVertexPtr			The ending vertex for the computation. */
	virtual void OnPauseAstar(const GraphVertexPtr& /*start*/, const GraphVertexPtr& /*dest*/) {}
	
	/*!	This method is called once when an A* computation that uses this IConstraint is resumed after having
		paused in order to check its CPU usage.
		\param GraphVertexPtr			The starting vertex for the computation.
		\param GraphVertexPtr			The ending vertex for the computation. */
	virtual void OnResumeAstar(const GraphVertexPtr& /*start*/, const GraphVertexPtr& /*dest*/) {}

	/*!	Sets an AstarContext used by this IConstraint. For internal use. */
	virtual KyResult SetCurrentAstarContext(AstarContext* astarContext);

	/*!	Clears the current AstarContext used by this IConstraint. For internal use. */
	virtual KyResult UnsetCurrentAstarContext();

	/*!	Computes the cost for a specified Bot to move along a line segment defined by two points.
		\pp When calculating a path, this method is called to determine the relative cost
		of each candidate edge in the spatial graph. The final path will be the path with the lowest total
		cost.
		\pp When following a path, the PathFinder calls this method to determine whether it can shortcut
		to points further along its path.
		\param entity		The Bot using the IConstraint.
		\param from			The starting point of the line segment.
		\param to 			The ending point of the line segment.
		\param[out] cost	Stores the cost computed for the line segment.
		\return #KY_TRUE if the \c cost parameter has been successfully updated, or #KY_FALSE to indicate that
				the specified Bot is entirely forbidden to follow the specified line segment. */
	virtual KyBool GetCost(Bot* entity, const PointWrapper& from, const PointWrapper& to, KyFloat32& cost) = 0;

	AstarContext* GetAstarContext() { return m_astarContext; }

	/*! \label_getworld */
	World* GetWorld();

	/*! \label_getworld */
	const World* GetWorld() const;


protected:
	/*!	The AstarContext that contains the current state of the A* calculation. */
	mutable AstarContext* m_astarContext;
};

/*!	This macro must be included in the declaration of any IConstraint class, to set up the MetaClass 
	used by the \SDKName object factory.
	\ingroup kyruntime_core */
#define KY_DECLARE_CONSTRAINT(ky_class) \
	static Kaim::IConstraintClass Class; \
	static Kaim::IConstraint* KyCreate(); \
	Kaim::IConstraintClass&  GetClass() { return ky_class::Class; }

/*!	This macro must be included in the implementation of any IConstraint class., to set up the MetaClass 
	used by the \SDKName object factory.
	\ingroup kyruntime_core */
#define KY_IMPLEMENT_CONSTRAINT(ky_class,ky_base_class) \
	Kaim::IConstraintClass ky_class::Class(#ky_class,ky_class::KyCreate,(Kaim::IConstraintClass*)ky_base_class); \
	Kaim::IConstraint* ky_class::KyCreate() \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::IConstraint* ptr = (Kaim::IConstraint*) KY_NEW_CLASS(ky_class)(); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}


/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from IHeuristic. Each class that derives from 
	IHeuristic must contain a constructor function that matches this type definition. The argument list
	of this type definition is empty, which indicates that the default constructor of your derived class
	will be invoked.
	\ingroup kyruntime_core */
typedef IHeuristic* (*IHeuristicConstructor)();

/*!	IHeuristicClass is the MetaClass for IHeuristic, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_core */
class IHeuristicClass: public MetaClass<IHeuristic,IHeuristicConstructor>
{
public:
	/*!	Constructor for the IHeuristic MetaClass. For internal use. */
	IHeuristicClass(const char* className, IHeuristicConstructor ctr, 
		IHeuristicClass* pBaseClass = (IHeuristicClass*) NULL);
};

/*!	IHeuristic is the base class for objects that determine the cost of moving a Bot from one point to another
	in order to control the way in which the A* algorithm used by the PathFinder traverses the vertices and edges in a spatial graph. Although
	the IHeuristic typically has no effect upon the final path chosen, using a class of IHeuristic that closely approximates the cost
	given by the current IConstraint class can increase the performance of the A* algorithm. 
	\pp For details on how heuristics are used, see :USERGUIDE:"Path Calculation".
	\pp This class is not usable as-is; you must either use one of the derived classes supplied with \SDKName, or write your
	own custom class.
	\section writingcustomheuristic Writing a custom IHeuristic class
	Writing a custom class that derives from IHeuristic involves the following considerations.
	\subsection heuristicconst Constructor
	All classes that derive from IHeuristic must have a constructor that follows the #IHeuristicConstructor type definition. 
	\subsection heuristicvirtualmethods Virtual methods
	All IHeuristic classes must implement the GetCost() method.
	\pp You may also implement any other virtual methods of this class if you choose, or use the default implementations.
	\pp If you choose to implement the OnStartAstar(), OnFinishAstar, OnPauseAstar() and/or OnResumeAstar() methods to carry out tasks at
	specific moments during the path calculation process, your implementations must take into account the fact that paused A* 
	calculations are not necessarily resumed. For example, if a path needs to be re-calculated before its initial A* calculation 
	has completed (for example, due to a GraphCell being unloaded), the calculation may be restarted instead. In this case, 
	OnFinishAstar() is called to notify the IHeuristic that the paused calculation has been terminated, and OnStartAstar() is 
	called again when the next calculation is begun.
	\subsection heuristicmetaclassmacros MetaClass macros
	In order for \SDKName to recognize your IHeuristic class, you must include the following macro definitions:
	-	#KY_DECLARE_HEURISTIC: Place this macro in the declaration of your new class. This macro takes one argument:
		the name of your new class.
	-	#KY_IMPLEMENT_HEURISTIC: Place this macro in the implementation file of your new class. This macro takes the 
		following arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the IHeuristic class from which your class derives, if different from 
			IHeuristic. For example, &Kaim::HeuristicEuclidianDistance::Class. If your class derives directly from 
			IHeuristic, the value of this argument should be #KY_NULL.
	\ingroup kyruntime_core */
class IHeuristic: public VirtualMemObject
{
protected:
	/*!	\label_baseconstructor */
	IHeuristic(): m_astarContext(KY_NULL) {}

public:
	/*!	\label_getclass */
	virtual IHeuristicClass& GetClass() = 0;

	/*!	\label_virtualdestructor */
	virtual ~IHeuristic() {}

	/*!	This method is called once immediately before starting an A* computation that uses this IHeuristic.
		\param GraphVertexPtr			The starting vertex for the computation.
		\param GraphVertexPtr			The ending vertex for the computation. */
	virtual void OnStartAstar(const GraphVertexPtr& /*start*/, const GraphVertexPtr& /*dest*/) {}

	/*!	This method is called once when an A* computation that uses this IHeuristic is completed.
		\param GraphVertexPtr			The starting vertex for the computation.
		\param GraphVertexPtr			The ending vertex for the computation. */
	virtual void OnFinishAstar(const GraphVertexPtr& /*start*/, const GraphVertexPtr& /*dest*/) {}

	/*!	This method is called once when an A* computation that uses this IHeuristic is paused in order to
		check its CPU usage.
		\pp Your implementation of this method must leave your IHeuristic in a temporary idle state from which 
		its calculations may be either resumed or restarted.
		\param GraphVertexPtr			The starting vertex for the computation.
		\param GraphVertexPtr			The ending vertex for the computation. */
	virtual void OnPauseAstar(const GraphVertexPtr& /*start*/, const GraphVertexPtr& /*dest*/) {}

	/*!	This method is called once when an A* computation that uses this IHeuristic is resumed after having
		paused in order to check its CPU usage.
		\param GraphVertexPtr			The starting vertex for the computation.
		\param GraphVertexPtr			The ending vertex for the computation. */
	virtual void OnResumeAstar(const GraphVertexPtr& /*start*/, const GraphVertexPtr& /*dest*/) {}

	/*!	Sets an AstarContext used by this IHeuristic. For internal use. */
	virtual KyResult SetCurrentAstarContext(AstarContext* astarContext);

	/*!	Clears the current AstarContext used by this IHeuristic. For internal use. */
	virtual KyResult UnsetCurrentAstarContext();

	/*!	Computes the approximate cost for a specified Bot to move along a line segment defined by two points, and
		stores the value in the \c cost parameter. 
		\pp This method is called during the path calculation process to determine the order in which candidate 
		vertices and edges should be explored. The \c from point will be the position of a candidate vertex, and the 
		\c to point will be the final desired destination. Vertices with lower costs are considered before edges with 
		higher costs. 
		\pp In general, you want this method to approximate the cost calculated by the IConstraint, without over-estimating. As long
		as the cost calculated by IHeuristic::GetCost() is lower than the sum of the values eventually calculated 
		by IConstraint::GetCost() for each edge in the spatial graph between the two points, the A* algorithm will be 
		guaranteed to find the shortest path to the destination. As the approximation made by the IHeuristic gets closer 
		to the cost value calculated by the IConstraint, the A* algorithm discards fewer vertices. Consequently, it finds
		the path more quickly.  
		\pp As soon as the IHeuristic cost can over-estimate the IConstraint cost, you lose the guarantee that the shortest path
		will be found. However, in some cases, you may want to sacrifice this guarantee in order to prioritize calculation speed
		in your implementation of IHeuristic::GetCost(). In other words, a very fast IHeuristic that occasionally over-costs 
		may be better than a very slow IHeuristic that never over-costs.
		\param entity		The Bot using this IHeuristic.
		\param from			The starting point of the line segment.
		\param to 			The ending point of the line segment.
		\param[out] cost	Stores the cost computed for the line segment.
		\return #KY_TRUE if the \c cost parameter has been successfully updated, or #KY_FALSE to indicate that
				the specified Bot is entirely forbidden to follow the specified line segment. */
	virtual KyBool GetCost(Bot* entity, const PointWrapper& from, const PointWrapper& to, KyFloat32& cost) = 0;


protected:
	/*!	The AstarContext that contains the current state of the A* calculation. */
	mutable AstarContext* m_astarContext;
};

/*!	This macro must be included in the declaration of any class that derives from IHeuristic, to set up the MetaClass 
	used by the \SDKName object factory.
	\ingroup kyruntime_core */
#define KY_DECLARE_HEURISTIC(ky_class) \
	static Kaim::IHeuristicClass Class; \
	static Kaim::IHeuristic* KyCreate(); \
	Kaim::IHeuristicClass&  GetClass() { return ky_class::Class; }

/*!	This macro must be included in the implementation of any class that derives from IHeuristic, to set up the MetaClass 
	used by the \SDKName object factory.
	\ingroup kyruntime_core */
#define KY_IMPLEMENT_HEURISTIC(ky_class,ky_base_class) \
	Kaim::IHeuristicClass ky_class::Class(#ky_class,ky_class::KyCreate,(Kaim::IHeuristicClass*)ky_base_class); \
	Kaim::IHeuristic* ky_class::KyCreate() \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::IHeuristic* ptr = (Kaim::IHeuristic*) KY_NEW_CLASS(ky_class)(); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}


////////////////////////////////////////////////////////////////////////////////////////////////////////
//		IPathFinder
////////////////////////////////////////////////////////////////////////////////////////////////////////

/*!	This class provides a basic abstract interface for an object that calculates a path for a Bot to 
	reach a given destination point, and that progressively refines that path at each frame according to the 
	immediate surroundings of the Bot.
	\pp You should not need to interact with this base class. Use the derived PathFinder class instead.
	\pp For more details on pathfinding and path following in \SDKName, see :USERGUIDE:"PathFinding".
	\ingroup kyruntime_core */
class IPathFinder : public VirtualMemObject
{
public:
	// definitions

	/*! Returned by the GetPathStatus() method to indicate the status of the path. */
	enum PathStatus
	{
		/*!	Indicates that the PathFinder is in the process of finding a path for its Bot, but has 
			not had enough time to complete its calculations. If this status occurs frequently, your time 
			management settings may be too strict. Try increasing the time per frame and maximum calls per frame
			for the time management tasks used by the PathFinder to manage its CPU consumption. */
		PATH_IN_CONSTRUCTION, 
		
		/*!	Indicates that the PathFinder has found a valid path to the specified destination, and is 
			currently directing its Bot along that path. */
		PATH_FOUND, 
		
		/*!	Indicates that the PathFinder was unable to find a valid path to the specified destination. 
			If this status occurs with PathFinder, you may be able to retrieve more information 
			on the cause of the error by calling the PathFinder::GetLastError() method. */
		PATH_NOT_FOUND, 
		
		/*!	Indicates that the PathFinder has detected a problem with the current path, which has forced 
			it to re-calculate a new path to the specified destination. For example, when an \LpfArea is discovered 
			to block an edge in the previously computed path. */
		PATH_RECOMPUTATION, 
		PathStatus__dummy__ = KYFORCEENUMSIZEINT
	};

	/*! Returned by GetNearestReachableValidPoint() to indicate the status of the computation. */
	enum GetValidPointStatus
	{
		/*!	The calculation is in progress, but has been time-sliced for completion in an upcoming frame. */
		GETVALIDPOINT_UNDER_PROCESS, 
		
		/*!	The \c validPoint argument passed to GetNearestReachableValidPoint() has been successfully updated with 
			a reachable valid point. */
		GETVALIDPOINT_FOUND, 
		
		/*!	No reachable vertex could be found in the spatial graph. */
		GETVALIDPOINT_NOT_FOUND, 
		GetValidPointStatus__dummy__ = KYFORCEENUMSIZEINT
	};

public:
	/*!	\label_baseconstructor */
	IPathFinder(Bot* bot);
	
	/*!	\label_virtualdestructor */
	~IPathFinder();
	void ReInit();

	/*! Retrieves the nearest vertex in the spatial graph that is directly reachable by an Bot located at the 
		specfied position and using this instance of the PathFinder. 
		\param startPosition	The coordinates for which to retrieve the nearest accessible vertex.
		\param[out] validPoint	Stores the nearest accessible vertex calculated by this method.
		\return An element from the #GetValidPointStatus enumeration that indicates the state of the calculation. */
	virtual GetValidPointStatus GetNearestReachableValidPoint(const Vec3f& startPosition, PointWrapper& validPoint) =0;

	/*! Instructs the PathFinder to generate an Action that will move the Bot from its current
		position towards the specified destination. This is the primary method of the PathFinder.
		\param dest				The desired destination point for the Bot using the PathFinder.
		\return #KY_TRUE if an Action was successfully computed, or #KY_FALSE otherwise. To get further information 
				about the state of the path, you can call GetPathStatus(). */
	virtual KyBool FindNextMove(const PointWrapper& dest) = 0;

	/*!	Sets the desired speed for the Bot using the PathFinder, in meters per second. 
		The PathFinder will never make the Bot move faster than this value, but may make the 
		Bot move at a reduced speed when steering the Bot through a turn or when avoiding dynamic 
		obstacles. */
	virtual void SetTargetSpeed(KyFloat32 speed) { m_targetSpeed = speed; }

	/*!	Retrieves the desired speed set for the Bot using the PathFinder, in meters per second. */
	KyFloat32 GetTargetSpeed() const { return m_targetSpeed; }

	/*!	Sets the minimum and maximum distances that the Bot must be from its final destination point in 
		order for the PathFinder to consider that the final destination has been reached.
		\pp When the distance to the destination is less than \c distMin, the destination is considered reached. Once 
		this occurs, the destination is considered reached while the distance to the goal remains less than 
		\c distMax. As soon as this occurs, the destination is no longer considered reached, and the Bot will 
		resume trying to reach the destination.
		\param distMin		The distance threshold at which the destination is considered reached, in meters.
		\param distMax		The distance threshold at which the destination is no longer considered reached,
							in meters. */
	virtual void SetDistanceToTarget(KyFloat32 distMin, KyFloat32 distMax);
	
	/*!	Retrieves the distance thresholds used to determine whether or not the final destination is reached. See 
		SetDistanceToTarget() for details.
		\param[out] distMin		Used to store the distance threshold at which the destination is considered reached, 
								in meters.
		\param[out] distMax		Used to store the distance threshold at which the destination is no longer considered 
								reached, in meters. */
	void GetDistanceToTarget(KyFloat32& distMin, KyFloat32& distMax) const {
		distMin = m_distToGoalMin;
		distMax = m_distToGoalMax;
	}

	/*!	Computes the approximate cost for a specified Bot to move along a line segment defined by two points.
		\param[out] cost	Stores the cost computed for the line segment.
		\param from			The starting point of the line segment.
		\param to 			The ending point of the line segment.
		\return #KY_TRUE if the \c cost parameter has been successfully updated, or #KY_FALSE to indicate that
				the specified Bot is entirely forbidden to follow the specified line segment. */
	virtual KyBool GetHeuristicPathCost(KyFloat32& cost, const PointWrapper& from, const PointWrapper& to) = 0;

	/*! Returns the current IConstraint set for the PathFinder, or #KY_NULL if no IConstraint is set. */
	virtual IConstraint* GetConstraint() const { return m_constraint; }

	/*!	Sets the current IConstraint for the PathFinder.
		\pp The PathFinder checks the value of the specified pointer at each frame to determine whether or not it 
		differs from the pointer currently in use. If it does, the PathFinder stops and re-launches any pathfinding 
		calculations currently in progress in order to take the new IConstraint into account. It is therefore not 
		recommended to create the IConstraint object on the stack at each frame before passing it to the SetConstraint() 
		method, as the memory pointer will very likely be different from the previous frame, causing time-sliced A* 
		calculations to be interrupted and restarted. Instead, consider creating a class member of your Bot class that 
		maintains an instance of your IConstraint class, and passing the SetConstraint() method a pointer to that class member.
		\return #KY_TRUE if the IConstraint was successfully set, or #KY_FALSE otherwise. */
	virtual KyBool SetConstraint(IConstraint* constraint) { m_constraint = constraint; return KY_TRUE; }

	/*! Returns the current IHeuristic set for the PathFinder, or #KY_NULL if no Heuristic is set. */
	virtual IHeuristic* GetHeuristic() const { return m_heuristic; }

	/*!	Sets the current IHeuristic for the PathFinder.
		\pp The PathFinder checks the value of the specified pointer at each frame to determine whether or not it 
		differs from the pointer currently in use. If it does, the PathFinder stops and re-launches any pathfinding 
		calculations currently in progress in order to take the new IHeuristic into account. It is therefore not 
		recommended to create the IHeuristic object on the stack at each frame before passing it to the SetHeuristic() 
		method, as the memory pointer will very likely be different from the previous frame, causing time-sliced A* 
		calculations to be interrupted and restarted. Instead, consider creating a class member of your Bot class that 
		maintains an instance of your IHeuristic class, and passing the SetHeuristic() method a pointer to that class member.
		\return #KY_TRUE if the IHeuristic was successfully set, or #KY_FALSE otherwise. */
	virtual KyBool SetHeuristic(IHeuristic* heuristic) { m_heuristic = heuristic; return KY_TRUE; }

	/*!	Indicates whether or not the Bot has reached the destination point provided in the last call to
		FindNextMove(). */
	virtual KyBool HasArrived() const = 0;

	/*! Retrieves the remaining distance, in meters, between the Bot and the destination point 
		provided in the last call to FindNextMove(), measured along the path (not in a straight line). */
	virtual KyFloat32 GetDistanceLeft() = 0;

	/*!	Retrieves the current status of the path maintained by the PathFinder.
		\return An element of the #PathStatus enumeration. */
	virtual PathStatus GetPathStatus() const = 0;

	/*!	Indicates whether or not the Bot using this PathFinder can move in a straight line between two specified points. 
		\param from		The starting point for the test.
		\param to		The ending point of the test.
		\return	#KY_TRUE if the Bot using the PathFinder can move directly from the starting point
				to the ending point, or #KY_FALSE if the direct path is blocked by static or dynamic obstacles.
		\pp Note that this method does not necessarily indicate whether or not a path exists
		between the \c start point and the \c dest point. It is simply a collision test that determines whether or not the 
		straight line between the two points is blocked by any obstacles. */
	virtual KyBool CanGo(const PointWrapper& from, const PointWrapper& to) = 0;

	/*!	Retrieves the number of calls to the FindNextMove() method of all instances of the PathFinder
		since the initialization of the Engine. */
	static KyUInt32 GetFindNextMoveCallCounter()
	{
		return s_findNextMoveCallCounter;
	}


public:
	/*!	Stores the movements decided on by FindNextMove(). If FindNextMove() returns #KY_TRUE, copy this 
		Action back to the Bot::m_action member of the Bot that owns this PathFinder. */
	Action m_action;
	Bot* m_bot; /*!< The Bot that owns this PathFinder. */
	IConstraint* m_constraint; /*!< Stores the IConstraint used by the PathFinder. Do not modify or use directly. */
	IHeuristic* m_heuristic; /*!< Stores the IHeuristic used by the PathFinder. Do not modify or use directly. */

	KyFloat32 m_targetSpeed; /*!< Stores the target speed of the PathFinder. Do not modify or use directly. */
	KyFloat32 m_distToGoalMin; /*!< Stores the distance threshold at which a goal may be considered reached. Do not modify or use directly. */
	KyFloat32 m_distToGoalMax; /*!< Stores the distance threshold at which a goal is no longer considered reached. Do not modify or use directly. */

	KyInt32 m_findNextMoveEstimatedTaskId; /*!< Stores the task ID of the FindNextMove estimation, for use in deterministic time measurement mode. Do not modify. See :USERGUIDE:"Using Deterministic Time Measurement". */

	/*!	Stores the total number of calls to FindNextMove() in the current frame. Used to determine accurate
		time estimations when using deterministic time measurement mode. Do not modify.
		\pp See :USERGUIDE:"Using Deterministic Time Measurement". */
	static KyUInt32 s_findNextMoveCallCounter; 
};

} // namespace Kaim

#endif // KY_INEXTMOVEMANAGER_H
