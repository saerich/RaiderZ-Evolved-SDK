/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */

#ifndef KY_PROPAGATIONTRAVERSALCONFIG_H
#define KY_PROPAGATIONTRAVERSALCONFIG_H

#include <kyruntime/graph/traversalconfig.h>


namespace Kaim
{

/*!	PropagationGraphTraversalConfig is an implementation of GraphTraversalConfig that sets up the data required to create a 
	PropagationGraphTraversal.
	\pp You cannot use this class as-is; you must use a class that derives from PropagationGraphTraversalConfig but that is
	specialized for a particular class of GraphTraversal. If you create a custom class that derives from PropagationGraphTraversal,
	you will also have to write a custom class that derives from PropagationGraphTraversalConfig, which should be able to create 
	a new instance of your PropagationGraphTraversal class.
	\pp For additional information, see :USERGUIDE:"Using GraphTraversals".
	\section propconfigcustom Writing a custom PropagationGraphTraversalConfig class
	Writing a custom class that derives from PropagationGraphTraversalConfig involves the following considerations.
	\subsection propconfigdataconst Constructors
	Each class that derives from PropagationGraphTraversalConfig must provide initialize default values for all data members of the
	PropagationGraphTraversalConfig class, including the #m_aperiodicTaskId class member. The easiest way to do this is for the
	constructor of your custom class to call one of the non-default PropagationGraphTraversalConfig constructors.
	\subsection propconfigdatamembers Data members and accessors
	Your PropagationGraphTraversalConfig class may contain any number of additional data members that you can use to provide default 
	values for information needed by your PropagationGraphTraversal. You can set the values of these data members in the constructors
	that you write for your PropagationGraphTraversalConfig class, or you can set them after creating the instance of your 
	PropagationGraphTraversalConfig class but before you pass the configuration to the TraversalManager. The implementation of 
	PropagationGraphTraversal::Initialize() in your PropagationGraphTraversal class can then read the values of these data members and set up
	the PropagationGraphTraversal accordingly.
	\subsection propconfigvm Virtual methods
	Every class that you derive from PropagationGraphTraversalConfig must implement the CreateGraphTraversal() method. Your implementation
	must create and initialize an instance of a specific class of PropagationGraphTraversal, and return a pointer to the new instance.
	The TraversalManager will call this method internally to get a new instance of the PropagationGraphTraversal class from your
	PropagationGraphTraversalConfig.
	\pp For example, your implementation is likely to be similar to the following: \code
	virtual GraphTraversal* CreateGraphTraversal(Kaim::TraversalManager& traversalManager) const
	{ 
		MyPropagationGraphTraversal* traversal = KY_NEW_CLASS(MyPropagationGraphTraversal);
		traversal->Initialize(traversalManager, *this);
		return traversal;
	} \endcode
	\ingroup kyruntime_graph */
	class PropagationGraphTraversalConfig : public GraphTraversalConfig
	{
	public:
	/*!	\label_constructor You should not create instances of this class directly, but you can call this constructor from
		the constructors of your derived classes. If you use this constructor, you must at a minimum configure the 
		#m_aperiodicTaskId class member before attempting to use your object. */
		PropagationGraphTraversalConfig()
			: GraphTraversalConfig(),
			m_maxDistance(KyFloat32MAXVAL)
		{
		}

	/*!	\label_constructor You should not create instances of this class directly, but you can call this constructor from
		the constructors of your derived classes.
		\param timeMgr				The TimeManager maintained by the World.
		\param aperiodicTaskName	Sets the value of #m_aperiodicTaskId from the unique name of the aperiodic task. 
		\param estimatedTaskName	Sets the value of #m_estimatedTaskId from the unique name of the estimation. Optional.
		\param maxDistance			Sets the value of #m_maxDistance. */
		PropagationGraphTraversalConfig(TimeManager& timeMgr, const char* aperiodicTaskName, const char* estimatedTaskName=KY_NULL,
			KyFloat32 maxDistance = KyFloat32MAXVAL)
			: GraphTraversalConfig(timeMgr, aperiodicTaskName, estimatedTaskName),
			m_maxDistance(maxDistance)
		{
		}

	/*!	\label_constructor You should not create instances of this class directly, but you can call this constructor from
		the constructors of your derived classes.
		\param aperiodicTaskId		Sets the value of #m_aperiodicTaskId. 
		\param estimatedTaskId		Sets the value of #m_estimatedTaskId. Optional.
		\param maxDistance			Sets the value of #m_maxDistance. */
		PropagationGraphTraversalConfig(TimeManager::TaskId aperiodicTaskId, TaskRegistry::TaskId estimatedTaskId=TaskRegistry::InvalidTaskId,
			KyFloat32 maxDistance = KyFloat32MAXVAL)
			:GraphTraversalConfig(aperiodicTaskId, estimatedTaskId)
			,m_maxDistance(maxDistance)
		{
		}


		virtual KyBool IsValid() const
		{
			KY_FUNCTION("PropagationGraphTraversalConfig::IsValid");
			
			KyBool isDistanceValid = (m_maxDistance > 0.f);
			KY_COND_MESSAGE(KY_LOG_ERROR, !isDistanceValid, ("The maximal distance of a propagation traversal must be strictly positive."));

			return GraphTraversalConfig::IsValid() && isDistanceValid;
		}
	public:
		/*!	Determines the maximum distance any candidate edge may be from the starting vertex. When a candidate edge is farther
			away from the starting vertex than the value of this parameter, the GraphTraversal does not consider that edge. 
			\units			meters
			\acceptedvalues	any positive value
			\defaultvalue	#KyFloat32MAXVAL */
		KyFloat32 m_maxDistance;
	};

} // namespace Kaim

#endif // KY_PROPAGATIONTRAVERSAL_H
