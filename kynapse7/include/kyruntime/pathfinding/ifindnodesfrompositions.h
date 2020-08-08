/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_IFINDNODESFROMPOSITIONS_H
#define KY_IFINDNODESFROMPOSITIONS_H

#include <kyruntime/pathfinding/imodifier.h>

#include <kyruntime/core/pointwrapper.h>
#include <kyruntime/graph/graphvertexsafeptr.h>


namespace Kaim
{

class IPathFinder;
class IFindNodesFromPositions;


///////////////////////////////////////////////////////////////////////////////////////////
// IFindNodesFromPositions
///////////////////////////////////////////////////////////////////////////////////////////

/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from IFindNodesFromPositions. Each class that derives from 
	IFindNodesFromPositions must contain a constructor function that matches this type definition. The argument list
	of this type definition is empty, which indicates that the default constructor of your derived class
	will be invoked.
	\ingroup kyruntime_pathfinding */
typedef IFindNodesFromPositions* (*FindNodesFromPositionsConstructor)();

/*!	FindNodesFromPositionsClass is the MetaClass for IFindNodesFromPositions, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
class FindNodesFromPositionsClass: public MetaClass<IFindNodesFromPositions, FindNodesFromPositionsConstructor>
{
public:
	/*!	Constructor for an IFindNodesFromPositions MetaClass. For internal use. */
	FindNodesFromPositionsClass(const char* className, FindNodesFromPositionsConstructor ctr,
		FindNodesFromPositionsClass* baseClass);
};

/*!	This macro must be included in the declaration of any IModifier class that derives
	from IFindNodesFromPositions, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_DECLARE_FINDNODESFROMPOSITIONS(ky_class) \
	static Kaim::FindNodesFromPositionsClass Class; \
	static Kaim::IFindNodesFromPositions* KyCreate(); \
	Kaim::FindNodesFromPositionsClass& GetClass() const { return ky_class::Class; }

/*!	This macro must be included in the implementation of any IModifier class that derives
	from IFindNodesFromPositions, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_IMPLEMENT_FINDNODESFROMPOSITIONS(ky_class,ky_base_class) \
	Kaim::FindNodesFromPositionsClass ky_class::Class(#ky_class,ky_class::KyCreate,ky_base_class); \
	Kaim::IFindNodesFromPositions* ky_class::KyCreate() \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::IFindNodesFromPositions* ptr = (Kaim::IFindNodesFromPositions*) KY_NEW_CLASS(ky_class)(); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}


/*!	IFindNodesFromPositions is an abstract base class for a type of IModifier that the PathFinder uses to
	determine the vertices in the spatial graph that it should use as the starting and ending points for a path
	calculation. Typically, the nearest vertex reachable from the current position of the Bot will be selected
	as the starting vertex, and the nearest vertex from which the final destination point is directly reachable
	will be selected as the ending vertex.
	\section writingcustomifn Writing a custom IFindNodesFromPositions class
	Writing a custom class that derives from IFindNodesFromPositions involves the following considerations.
	\subsection ifnconst Constructor
	All classes that derive from IFindNodesFromPositions must have a constructor that follows the #FindNodesFromPositionsConstructor type definition. 
	\subsection ifnvm Virtual methods
	All classes that derive from IFindNodesFromPositions must implement the FindStartNode() and FindDestNode() methods.
	\subsection ifnmetaclass MetaClass macros
	In order for \SDKName to recognize your IFindNodesFromPositions class, you must include the following macro definitions:
	-	#KY_DECLARE_FINDNODESFROMPOSITIONS: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_FINDNODESFROMPOSITIONS: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from IFindNodesFromPositions.
			For example, &Kaim::FindNodesFromPositions_NearestReachable::Class. If your new class derives directly from IFindNodesFromPositions,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_pathfinding */
class IFindNodesFromPositions: public IModifier
{
public:
	/*!	\label_constructor */
	IFindNodesFromPositions():
		IModifier() {}
		
	/*!	\label_getclass */
	virtual FindNodesFromPositionsClass& GetClass() const = 0;

	/*!	The PathFinder calls this method in order to determine the vertex in the spatial graph that it
		should use as the starting vertex for the A* algorithm.
		\param start					The starting position for the requested path computation, typically the
										current position of the Bot using the PathFinder. If this PointWrapper
										has an attached PointInfo_VertexSafePtr object that holds a valid GraphVertexSafePtr, it is already
										at the same position as a vertex. In this case, you can typically
										just set the \c startNodeSafePtr parameter to that value.
		\param[out] startNodeSafePtr	Stores a GraphVertexSafePtr that identifies the starting vertex for the A* algorithm. 
		\return #KY_TRUE if the calculation was successfully completed and the \c startNodeSafePtr argument was
				successfully updated, or #KY_FALSE if the computation is not yet finished. If #KY_FALSE is returned, path calculation
				is delayed, and this method is re-called at each frame until it returns #KY_TRUE. */
	virtual KyBool FindStartNode(const PointWrapper& start, GraphVertexSafePtr& startNodeSafePtr) = 0;

	/*!	The PathFinder calls this method in order to determine the vertex in the spatial graph that it
		should use as the goal vertex for the A* algorithm.
		\param dest						The final destination point for the requested path computation. If this PointWrapper
										has an attached PointInfo_VertexSafePtr object that holds a valid GraphVertexSafePtr, it is already
										at the same position as a vertex. In this case, you can typically
										just set the \c destNodeSafePtr parameter to that value.
		\param[out] destNodeSafePtr		Stores a GraphVertexSafePtr that identifies the goal vertex for the A* algorithm. 
		\return #KY_TRUE if the calculation was successfully completed and the \c destNodeSafePtr argument was
				successfully updated, or #KY_FALSE if the computation is not yet finished. If #KY_FALSE is returned, path calculation
				is delayed, and this method is re-called at each frame until it returns #KY_TRUE. */
	virtual KyBool FindDestNode(const PointWrapper& dest, GraphVertexSafePtr& destNodeSafePtr) = 0;

	/*!	Retrieves a bit mask that indicates any errors encountered during the last attempt to find a valid starting vertex. */
	KyUInt32 GetLastErrors_Start() {return m_lastErrors_start;}
	
	/*!	Retrieves a bit mask that indicates any errors encountered during the last attempt to find a valid goal vertex. */
	KyUInt32 GetLastErrors_Dest() {return m_lastErrors_dest;}


public:
	KyUInt32 m_lastErrors_start; /*!< See GetLastErrors_Start(). */
	KyUInt32 m_lastErrors_dest; /*!< See GetLastErrors_Dest(). */
};

} // namespace Kaim

#endif // KY_IFINDNODESFROMPOSITIONS_H
