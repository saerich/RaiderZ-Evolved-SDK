/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_IEDGESTATUSAWARENESS_H
#define KY_IEDGESTATUSAWARENESS_H

#include <kyruntime/pathfinding/imodifier.h>

#include <kyruntime/graph/graphvertexsafeptr.h>


namespace Kaim
{

class IPathFinder;
class IEdgeStatusAwareness;


///////////////////////////////////////////////////////////////////////////////////////////
// IEdgeStatusAwareness
///////////////////////////////////////////////////////////////////////////////////////////
/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from IEdgeStatusAwareness. Each class that derives from 
	IEdgeStatusAwareness must contain a constructor function that matches this type definition. The argument list
	of this type definition is empty, which indicates that the default constructor of your derived class
	will be invoked.
	\ingroup kyruntime_pathfinding */
typedef IEdgeStatusAwareness* (*EdgeStatusAwarenessConstructor)();

/*!	EdgeStatusAwarenessClass is the MetaClass for IEdgeStatusAwareness, created and used internally by
	the \SDKName object factory. 
	\ingroup kyruntime_pathfinding */
class EdgeStatusAwarenessClass: public MetaClass<IEdgeStatusAwareness, EdgeStatusAwarenessConstructor>
{
public:
	/*!	Constructor for an IEdgeStatusAwareness MetaClass. For internal use. */
	EdgeStatusAwarenessClass(const char* className, EdgeStatusAwarenessConstructor ctr, EdgeStatusAwarenessClass* baseClass);
};

/*!	This macro must be included in the declaration of any IModifier class that derives
	from IEdgeStatusAwareness, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_DECLARE_EDGESTATUSAWARENESS(ky_class) \
	static Kaim::EdgeStatusAwarenessClass Class; \
	static Kaim::IEdgeStatusAwareness* KyCreate(); \
	Kaim::EdgeStatusAwarenessClass& GetClass() const { return ky_class::Class; }

/*!	This macro must be included in the implementation of any IModifier class that derives
	from IEdgeStatusAwareness, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_IMPLEMENT_EDGESTATUSAWARENESS(ky_class,ky_base_class) \
	Kaim::EdgeStatusAwarenessClass ky_class::Class(#ky_class,ky_class::KyCreate,ky_base_class); \
	Kaim::IEdgeStatusAwareness* ky_class::KyCreate() \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::IEdgeStatusAwareness* ptr = (Kaim::IEdgeStatusAwareness*) KY_NEW_CLASS(ky_class)(); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}


/*!	IEdgeStatusAwareness is an abstract base class for a type of IModifier that the PathFinder uses to
	determine whether or not its Bot is aware of the passability status of edges that 
	were previously determined to be blocked by \LpfAreas. 
	\pp This type of IModifier is only effective in conjunction with the Local PathFinding system. See also
	:USERGUIDE:"Using Obstacles". 
	\section writingcustomiesw Writing a custom IEdgeStatusAwareness class
	Writing a custom class that derives from IEdgeStatusAwareness involves the following considerations.
	\subsection ieswconst Constructor
	All classes that derive from IEdgeStatusAwareness must have a constructor that follows the #EdgeStatusAwarenessConstructor type definition. 
	\subsection ieswvm Virtual methods
	All classes that derive from IEdgeStatusAwareness must implement the GetPerceivedEdgeStatus() method.
	\subsection ieswmetaclass MetaClass macros
	In order for \SDKName to recognize your IEdgeStatusAwareness class, you must include the following macro definitions:
	-	#KY_DECLARE_EDGESTATUSAWARENESS: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_EDGESTATUSAWARENESS: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from IEdgeStatusAwareness.
			For example, &Kaim::EdgeStatusAwareness_Distance2dAndTime::Class. If your new class derives directly from IEdgeStatusAwareness,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_pathfinding */
class IEdgeStatusAwareness: public IModifier
{
public:
	/*!	\label_constructor */
	IEdgeStatusAwareness():
		IModifier() {};
		
	/*!	\label_getclass */
	virtual EdgeStatusAwarenessClass& GetClass() const = 0;

	/*!	When planning a path for a Bot, the PathFinder calls this method for each edge 
		contained in its taboo edge list, to evaluate whether the specified Bot should consider 
		the specified edge to be blocked or passable.
		\param entity					The Bot using the PathFinder.
		\param edgeP1UID				The UID of the vertex at the start of the edge.
		\param edgeP2UID				The UID of the vertex at the end of the edge.
		\param realStatus				The real Local PathFinding passability status of the edge maintained
										internally by the PathFinder.
										-	If #KY_FALSE, an \LpfArea is known to still block the specified edge.
										-	If #KY_TRUE, the passability status of the edge is currently unknown 
											to the PathFinder, typically due to a recent change in the 
											LpfArea (addition or removal of one or more obstacles, destruction 
											of the LpfArea, etc.).
		\param lastStatusChangeDate		The date of the last known change to the Local PathFinding passability 
										of the specified edge.
		\return -	#KY_FALSE if the Bot should consider the edge to be blocked. In this case, the PathFinder 
					will not use the edge in its current path calculation. 
				-	#KY_TRUE if the Bot should consider the edge to be passable. In this case, the normal 
					Local Pathfinding process occurs: if the edge is farther from the Bot than the value of 
					the LpfModule::m_awarenessRadius value set for the PathFinder, the 
					Bot considers the edge to be passable; if the edge is closer to the Bot than the value 
					of that parameter, the LpfManager re-calculates its actual current passability. */
	virtual KyBool GetPerceivedEdgeStatus(const Kaim::Bot &entity,
		const GraphVertexSafePtr& edgeP1UID, const GraphVertexSafePtr& edgeP2UID, KyBool realStatus,
		KyFloat32 lastStatusChangeDate) = 0;
};

} // namespace Kaim

#endif // KY_IEDGESTATUSAWARENESS_H
