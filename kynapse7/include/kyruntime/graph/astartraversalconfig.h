/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */

#ifndef KY_ASTARTRAVERSALCONFIG_H
#define KY_ASTARTRAVERSALCONFIG_H

#include <kyruntime/graph/traversalconfig.h>
#include <kypathdata/basesystem/config.h>

namespace Kaim
{

/*!	AstarTraversalConfig is an implementation of GraphTraversalConfig that sets up the data required to create an AstarTraversal.
	\pp You cannot use this class as-is; you must use a class that derives from AstarTraversalConfig but that is
	specialized for a particular class of GraphTraversal. For example, the HidingTraversalConfig class derives from
	AstarTraversalConfig, but is specialized to create and set up an instance of the HidingTraversal class.
	If you create a custom class that derives from AstarTraversal, you will also have to write a custom class that
	derives from AstarTraversalConfig, which should be able to create a new instance of your AstarTraversal class.
	\pp For additional information, see :USERGUIDE:"Using GraphTraversals".
	\section astarconfigcustom Writing a custom AstarTraversalConfig class
	Writing a custom class that derives from AstarTraversalConfig involves the following considerations.
	\subsection astarconfigdataconst Constructors
	Each class that derives from AstarTraversalConfig must provide initialize default values for all data members of the
	AstarTraversalConfig class, including the #m_aperiodicTaskId class member. The easiest way to do this is for the
	constructor of your custom class to call one of the non-default AstarTraversalConfig constructors.
	\subsection astarconfigdatamembers Data members and accessors
	Your AstarTraversalConfig class may contain any number of additional data members that you can use to provide default 
	values for information needed by your AstarTraversal. You can set the values of these data members in the constructors
	that you write for your AstarTraversalConfig class, or you can set them after creating the instance of your 
	AstarTraversalConfig class but before you pass the configuration to the TraversalManager. The implementation of 
	AstarTraversal::Initialize() in your AstarTraversal class can then read the values of these data members and set up
	the AstarTraversal accordingly.
	\subsection astarconfigvm Virtual methods
	Every class that you derive from AstarTraversalConfig must implement the CreateGraphTraversal() method. Your implementation
	must create and initialize an instance of a specific class of AstarTraversal, and return a pointer to the new instance.
	The TraversalManager will call this method internally to get a new instance of the AstarTraversal class from your
	AstarTraversalConfig.
	\pp For example, your implementation is likely to be similar to the following: \code
	virtual GraphTraversal* CreateGraphTraversal(Kaim::TraversalManager& traversalManager) const
	{ 
		MyAstarTraversal* traversal = KY_NEW_CLASS(MyAstarTraversal);
		traversal->Initialize(traversalManager, *this);
		return traversal;
	} \endcode
	\ingroup kyruntime_graph */
	class AstarTraversalConfig : public GraphTraversalConfig
	{
	public:
	/*!	\label_constructor You should not create instances of this class directly, but you can call this constructor from
		the constructors of your derived classes. If you use this constructor, you must at a minimum configure the 
		#m_aperiodicTaskId class member before attempting to use your object. */
		AstarTraversalConfig();

	/*!	\label_constructor You should not create instances of this class directly, but you can call this constructor from
		the constructors of your derived classes.
		\param timeMgr				The TimeManager maintained by the World.
		\param aperiodicTaskName	Sets the value of #m_aperiodicTaskId from the unique name of the aperiodic task. 
		\param estimatedTaskName	Sets the value of #m_estimatedTaskId from the unique name of the estimation. Optional.
		\param maxDistance			Sets the value of #m_maxDistance. 
		\param maxCost				Sets the value of #m_maxCost. */
		AstarTraversalConfig(TimeManager& timeMgr, const char* aperiodicTaskName, const char* estimatedTaskName = KY_NULL,
			KyFloat32 maxDistance = KyFloat32MAXVAL, KyFloat32 maxCost = KyFloat32MAXVAL);

	/*!	\label_constructor You should not create instances of this class directly, but you can call this constructor from
		the constructors of your derived classes.
		\param aperiodicTaskId		Sets the value of #m_aperiodicTaskId. 
		\param estimatedTaskId		Sets the value of #m_estimatedTaskId. Optional.
		\param maxDistance			Sets the value of #m_maxDistance. 
		\param maxCost				Sets the value of #m_maxCost. */
		AstarTraversalConfig(TimeManager::TaskId aperiodicTaskId, TaskRegistry::TaskId estimatedTaskId = TaskRegistry::InvalidTaskId,
			KyFloat32 maxDistance = KyFloat32MAXVAL, KyFloat32 maxCost = KyFloat32MAXVAL);

		virtual KyBool IsValid() const;

	public:
		/*!	Determines the maximum distance any candidate vertex may be from the starting vertex. When a candidate vertex is farther
			away from the starting vertex than the value of this parameter, the AstarTraversal does not consider that vertex. 
			\units			meters
			\acceptedvalues	any positive value
			\defaultvalue	#KyFloat32MAXVAL */
		KyFloat32 m_maxDistance;

		/*!	Determines the maximum cost of any path from the starting vertex to a candidate vertex, according to the current 
			IConstraint used by the AstarTraversal. When the total path cost from the starting vertex to a candidate vertex exceeds
			the value of this parameter, the AstarTraversal does not consider that vertex.
			\units			meters
			\acceptedvalues	any positive value
			\defaultvalue	#KyFloat32MAXVAL */
		KyFloat32 m_maxCost;
	};

} // namespace Kaim

#endif // KY_ASTARTRAVERSALCONFIG_H
