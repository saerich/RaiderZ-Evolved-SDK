/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/




/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_ISELECTPATHNODECANDIDATE_H
#define KY_ISELECTPATHNODECANDIDATE_H

#include <kyruntime/pathfinding/imodifier.h>

#include <kyruntime/graph/path.h>
#include <kyruntime/core/pointwrapper.h>


namespace Kaim
{

class IPathFinder;
class ISelectPathNodeCandidate;


///////////////////////////////////////////////////////////////////////////////////////////
// ISelectPathNodeCandidate
///////////////////////////////////////////////////////////////////////////////////////////

/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from ISelectPathNodeCandidate. Each class that derives from 
	ISelectPathNodeCandidate must contain a constructor function that matches this type definition. The argument list
	of this type definition is empty, which indicates that the default constructor of your derived class
	will be invoked.
	\ingroup kyruntime_pathfinding */
typedef ISelectPathNodeCandidate* (*SelectPathNodeCandidateConstructor)();

/*!	SelectPathNodeCandidateClass is the MetsClass for ISelectPathNodeCandidate, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
class SelectPathNodeCandidateClass: public MetaClass<ISelectPathNodeCandidate, SelectPathNodeCandidateConstructor>
{
public:
	/*!	Constructor for an ISelectPathNodeCandidate MetaClass. For internal use. */
	SelectPathNodeCandidateClass(const char* className, SelectPathNodeCandidateConstructor ctr,
		SelectPathNodeCandidateClass* baseClass);
};

/*!	This macro must be included in the declaration of any IModifier class that derives
	from ISelectPathNodeCandidate, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_DECLARE_SELECTPATHNODECANDIDATE(ky_class) \
	static Kaim::SelectPathNodeCandidateClass Class; \
	static Kaim::ISelectPathNodeCandidate* KyCreate(); \
	Kaim::SelectPathNodeCandidateClass& GetClass() const { return ky_class::Class; }

/*!	This macro must be included in the implementation of any IModifier class that derives
	from ISelectPathNodeCandidate, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_IMPLEMENT_SELECTPATHNODECANDIDATE(ky_class,ky_base_class) \
	Kaim::SelectPathNodeCandidateClass ky_class::Class(#ky_class,ky_class::KyCreate,ky_base_class); \
	Kaim::ISelectPathNodeCandidate* ky_class::KyCreate() \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::ISelectPathNodeCandidate* ptr = (Kaim::ISelectPathNodeCandidate*) KY_NEW_CLASS(ky_class)(); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}


/*!	ISelectPathNodeCandidate is an abstract base class for a type of IModifier that the PathFinder uses to
	choose the PathNode in the Path that the Bot should target. Instead of proceeding all the way to each PathNode 
	in turn, the Bot can use this modifier to find a reachable PathNode farther along the Path, and skip 
	directly to that PathNode. Typically, skipping to the farthest reachable PathNode results in the smoothest, 
	most realistic trajectory.
	\pp The PathFinder calls the ISelectPathNodeCandidate::SelectPathNodeCandidate() method in a loop. As long 
	as SelectPathNodeCandidate() returns a valid PathNode along the Path, the following tests are carried out on the 
	returned candidate PathNode:
	-	If the edge leading to the candidate PathNode is linked to a PathObject, the 
		IPathObjectBehavior::CanByPassEntrance() method of that PathObject is called.
	-	If any \LpfArea lies on the direct line from the current position of the Bot to the candidate PathNode, the 
		passability and cost of bypassing that LpfArea is taken into account. 
	-	The ISelectPathNodeCandidate::EvaluateShortcut() method is called for the candidate PathNode. 
	-	The ISelectPathNodeCandidate::ValidateCandidate() method is called for the candidate PathNode. 
	\pp If all of these tests succeed, the candidate is considered to be valid.
	\pp When ISelectPathNodeCandidate::SelectPathNodeCandidate() returns #KY_NULL, the loop is terminated, and the last valid 
	candidate is adopted as the current PathNode candidate. The loop is also stopped if the first condition above is true: if any
	PathObject along the Path does not permit the Bot to bypass its entrance.
	\pp After the loop has been stopped, it will not be re-entered until a variable time interval has elapsed. By default, 
	the base period for this time interval is set to 0.1 seconds, but you can change this value by calling 
	PathFinder::SetPathNodeUpdatePeriod(). This base period is doubled if the PathFinder attempts to enter the 
	ISelectPathNodeCandidate loop when no time remains for path following in the current frame, in order to share CPU time 
	more evenly with other \Bots. If you have problems with your \Bots not smoothing their paths quickly enough after their 
	paths have been computed, you may need to lower the base period.
	\section writingcustomiselpnc Writing a custom ISelectPathNodeCandidate class
	Writing a custom class that derives from ISelectPathNodeCandidate involves the following considerations.
	\subsection iselpncconst Constructor
	All classes that derive from ISelectPathNodeCandidate must have a constructor that follows the #SelectPathNodeCandidateConstructor type definition. 
	\subsection iselpncvm Virtual methods
	All classes that derive from ISelectPathNodeCandidate must implement the SelectPathNodeCandidate(), EvaluateShortcut() and ValidateCandidate()
	methods.
	\pp You may also implement the other virtual methods, or use their default implementations if you prefer.
	\subsection iselpncmetaclass MetaClass macros
	In order for \SDKName to recognize your ISelectPathNodeCandidate class, you must include the following macro definitions:
	-	#KY_DECLARE_SELECTPATHNODECANDIDATE: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_SELECTPATHNODECANDIDATE: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from ISelectPathNodeCandidate.
			For example, &Kaim::SelectPathNodeCandidate_NextPathNode::Class. If your new class derives directly from ISelectPathNodeCandidate,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_pathfinding */
class ISelectPathNodeCandidate: public IModifier
{
public:
	/*!	\label_constructor */
	ISelectPathNodeCandidate():
		IModifier() {}
		
	/*!	\label_getclass */
	virtual SelectPathNodeCandidateClass& GetClass() const = 0;

	/*!	The PathFinder calls this method in a loop in order to obtain a candidate PathNode 
		for the Bot to target instead of the current PathNode. 
		\param path					The current Path maintained by the PathFinder.
		\param currentPathNode		An iterator that points to the current PathNode being targeted by the PathFinder, 
									or #KY_NULL if no target PathNode currently exists.
		\return An iterator that points to the next candidate PathNode to check within the specified Path, or #KY_NULL if no 
				more candidate \PathNodes remain to be checked in the current Path. */
	virtual PathNodeIterator SelectPathNodeCandidate(Path& path, 
		PathNodeIterator& currentPathNode) = 0;

	/*!	The PathFinder calls this method in order to determine whether or not moving straight 
		to the \c candidatePosition represents a shortcut for the Bot. If so, the \c candidatePathNode 
		becomes the current PathNode of the Path. 
		\param currentPosition		The current position of the Bot.
		\param currentPathNode		The current PathNode being targeted by the PathFinder.
		\param candidatePathNode	The last PathNode on the path before the position to test.
		\param candidatePosition	The candidate position to test.
		\return #KY_TRUE if the Bot should adopt the candidate PathNode as its current PathNode, or 
				#KY_FALSE if the Bot should ignore the candidate and continue moving toward the current PathNode. */
	virtual KyBool EvaluateShortcut(const PointWrapper& currentPosition, const PathNodeIterator& currentPathNode,
		const PathNodeIterator& candidatePathNode, const PointWrapper& candidatePosition) = 0;

	/*!	The PathFinder calls this method in order to determine whether or not any obstacles exist in 
		between the current position of the Bot and the candidate PathNode. If any obstacles are found, the candidate 
		PathNode is ignored, and the Bot continues moving toward its current PathNode.
		\pp If you write a custom ISelectPathNodeCandidate modifier, your implementation of ValidateCandidate() 
		does not need to take \PathObjects or \Obstacles into account, as these types of obstacles are handled 
		internally within the PathFinder.
		\return #KY_TRUE if the Bot can move directly to the candidate PathNode, or #KY_FALSE otherwise. */
	virtual KyBool ValidateCandidate(const PointWrapper& currentPosition,
		const PointWrapper& currentPathNode, const PointWrapper& candidate) = 0;

	/*!	The PathFinder calls this method in order to apply a candidate point selected in an 
		asynchronous process. It is recommended that you do not change the default implementation of 
		this method unless aided by Autodesk support.
		\param path					The current Path maintained by the PathFinder.
		\param currentPathNode		The current PathNode being targeted by the PathFinder, or #KY_NULL if 
									no target PathNode currently exists. */
	virtual PathNodeIterator CheckLastPathNodeCandidate(const Path& path,
			const PathNodeIterator& currentPathNode)
	{
		(void)path;
		(void)currentPathNode;
		return PathNodeIterator();
	}
};

} // namespace Kaim

#endif // KY_ISELECTPATHNODECANDIDATE_H
