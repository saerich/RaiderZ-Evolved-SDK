/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_IMODIFIER_H
#define KY_IMODIFIER_H

#include <kyruntime/core/world.h>
#include <kyruntime/lpf/ilpfmanager.h>

#include <kypathdata/generic/memory.h>
#include <kypathdata/basesystem/config.h>
#include <kypathdata/generic/cstdio.h>

namespace Kaim
{

class PathFinder;
class Modifiers;

///////////////////////////////////////////////////////////////////////////////////////////
// IModifier
///////////////////////////////////////////////////////////////////////////////////////////

/*!	IModifier represents an abstract base class for objects that affect the path computation and path following
	services provided by the PathFinder. 
	\pp Several different interfaces are derived from IModifier, each of which encapsulates a particular task that 
	the PathFinder needs to perform during its path calculation and path following computations. For 
	example, the PathFinder uses an object that implements the IFindNodesFromPositions interface to 
	find the starting and ending vertices for a path calculation, and an object that implements
	the IDetectPathNodeReached interface to determine whether or not the current target PathNode in its Path has
	been reached. 
	\pp For most of these interfaces, multiple different implementations are provided in the \SDKName SDK. To change
	the behavior of the PathFinder, you can substitute a different implementation, or write your own 
	custom implementation of any modifier interface. Your own implementation will not derive directly from 
	IModifier, but rather from one of the interfaces used by the PathFinder. See the documentation for the specific
	interfaces for detailed information on writing your own custom modifier classes.
	\pp For more detailed information on how IModifier classes are used during path computation and path following, 
	see :USERGUIDE:"The PathFinding Process".
	\ingroup kyruntime_pathfinding */
class IModifier: public VirtualMemObject
{
public:
	/*!	\label_constructor */
	IModifier() :
		m_pathFinder(KY_NULL)
	{
	}

	/*!	\label_virtualdestructor */
	virtual ~IModifier() = 0;


	/*!	Resets the state of the IModifier. This method is called transparently when PathFinder::Reset() is called. */
	virtual void ReInit() {}

	/*!	Retrieves a pointer to the PathFinder using this IModifier. */
	PathFinder* GetPathFinder() const { return m_pathFinder; }

	/*!	\label_getworld */
	const World& GetWorld() const;

	/*!	\label_getworld */
	World& GetWorld();

	/*!	Indicates whether or not the Bot using this IModifier has been initialized in a World. */
	bool IsAttachedToWorld() const;

	/*!	Indicates whether or not the Bot using this IModifier has been initialized in the specified World. */
	bool IsAttachedToWorld(const World& world) const;

	/*!	\label_getengine */
	Engine& GetEngine()
	{
		return GetWorld().GetEngine();
	}

	/*!	\label_getengine */
	const Engine& GetEngine() const
	{
		return GetWorld().GetEngine();
	}

protected:

	/*! Called automatically at the beginning of every call to PathFinder::Initialize() to check that this class of IModifier is compatible with
		the other IModifier classes set in the \c Modifiers& parameter, and with the \Obstacles system if an \LpfContext is provided.
		\param Modifiers&							The Modifiers object passed to PathFinder::Initialize().
		\param const Kaim::ILpfManager::LpfContext	The LpfContext passed to PathFinder::Initialize, if any.
		\return Kaim::Result::Success if this class of IModifier is compatible with the parameters provided. */
	virtual KyResult CheckModifierDependencies(const Modifiers&, const Kaim::ILpfManager::LpfContext*) { return Kaim::Result::Success; }


	friend class Modifiers;
	/*! Called automatically when the PathFinder is initialized. */
	void SetPathFinder(PathFinder& pf)
	{
		KY_FUNCTION("IModifier::Initialize");
		KY_ASSERT(m_pathFinder==KY_NULL, ("IModifier already affected to a pathFinder"));
		m_pathFinder = &pf;
		OnSetPathFinder();
	}

	/*! Notifies the IModifier that the PathFinder has been initialized, so that the IModifier can carry out any
		steps it needs in order to initialize itself. */
	virtual void OnSetPathFinder() = 0;

	PathFinder* m_pathFinder; /*!< A pointer to the PathFinder using this IModifier. Do not modify. */
};

} // namespace Kaim

#endif // KY_IMODIFIER_H
