/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_gapda */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_GOTO_ASYNCGAPDA_ASYNCMODULE_H
#define KY_GOTO_ASYNCGAPDA_ASYNCMODULE_H

#include <kyruntime/asyncmanagement/asyncmodule.h>
#include <kyruntime/gapda/goto_asyncgapda.h>


namespace Kaim
{

//
// Dynamic Avoidance Async Module, root class for platform-specific implementations
//

/*!	Goto_AsyncGapDA_AsyncModule provides an abstract base class for \AsyncModules that manage the 
	calculations performed by the Goto_AsyncGapDA pathfinding modifier. This base class is
	specialized for use on different platforms.
	\ingroup kyruntime_gapda */
class Goto_AsyncGapDA_AsyncModule: public AsyncModule
{
public:

	/*!	\label_baseconstructor */
	Goto_AsyncGapDA_AsyncModule(AsyncManager& asyncMgr):AsyncModule(asyncMgr) {};

	/**********************************************************************
	* Requests
	*
	* An periodic-aperiodic time-slicing is used to better spread the async
	* DA calls over all the frames. Typical use from the modifier is :
	* 1- Start with timer = 0
	* 2- if timer>PeriodicTime, call a GrantAperiodicTask until served
	* 3- reset timer to 0
	**********************************************************************/

	/*!	Registers a new request for an asynchronous calculation. Called automatically by
		Goto_AsyncGapDA when an instance of the PathFinder needs to calculate 
		a trajectory for a Bot.
		\return #KY_TRUE if the new request was successfully added, or #KY_FALSE if the request could
				not be added to the list maintained by the AsyncModule. #KY_FALSE may indicate that
				the size of the list has been exhausted. */
	virtual KyBool AddRequest(Goto_AsyncGapDA& owner) = 0;

	/*!	Indicates to Goto_AsyncGapDA whether it should carry out its full update
		asynchronously, or whether some computations such as CanGo() tests may be deferred for 
		completion during the next update of the \SDKName Engine. */
	virtual KyBool FullExternalUpdateMode() const = 0;

public:
	static AsyncModuleClass Class; /*!< The MetaClass of this AsyncModule. */ // Replaces the KY_DECLARE... for virtual classes
};

} // namespace Kaim

#endif // KY_GOTO_ASYNCGAPDA_ASYNCMODULE_H
