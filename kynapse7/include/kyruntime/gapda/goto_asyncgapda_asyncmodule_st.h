/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_gapda */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_GOTO_ASYNCGAPDA_ASYNCMODULE_ST_H
#define KY_GOTO_ASYNCGAPDA_ASYNCMODULE_ST_H

#include <kyruntime/gapda/goto_asyncgapda_asyncmodule.h>


namespace Kaim
{

//
// Asynchroneous CGoto_GapDA, running in the Single Thread (ST) created by the AsyncManager service
//

/*!	This implementation of the Goto_AsyncGapDA_AsyncModule class manages
	the calculations performed by the Goto_AsyncGapDA pathfinding 
	modifier on Windows and Xbox 360 platforms, using a secondary thread.
	\ingroup kyruntime_gapda */
class Goto_AsyncGapDA_AsyncModule_ST: public Goto_AsyncGapDA_AsyncModule
{
public:

	/*!	\label_baseconstructor */
	Goto_AsyncGapDA_AsyncModule_ST(AsyncManager& asyncMgr);
	
	/*!	\label_virtualdestructor */
	virtual ~Goto_AsyncGapDA_AsyncModule_ST();
	KY_DECLARE_ASYNC_MODULE(Goto_AsyncGapDA_AsyncModule_ST)
	

	/*!	label_init
		\param maxSimultaneousRequests			Determines the maximum number of simultaneous requests this AsyncModule can accept. */
	KyResult Initialize(KyUInt32 maxSimultaneousRequests);
	//virtual void ReInit();

	virtual void BeginAsyncProcessing();

	/**********************************************************************
	* Requests
	*
	* An periodic-aperiodic time-slicing is used to better spread the async
	* DA calls over all the frames. Typical use from the modifier is :
	* 1- Start with timer = 0
	* 2- if timer>PeriodicTime, call a GrantAperiodicTask until served
	* 3- reset timer to 0
	**********************************************************************/

	virtual KyBool AddRequest(Goto_AsyncGapDA& owner);

	/*!	\copydoc Goto_AsyncGapDA_AsyncModule::FullExternalUpdateMode()
		\pp This implementation of Goto_AsyncGapDA_AsyncModule::FullExternalUpdateMode() returns #KY_FALSE, which
		carries out CanGo() tests in a succeeding frame rather than performing them in the asynchronous thread. */
	virtual KyBool FullExternalUpdateMode() const { return KY_FALSE; }

protected:
	//
	// Configuration settings
	//

	/* Request queue size - default to MAX_ENTITIES */
	KyUInt32 m_maxSimultaneousRequests;

	//
	// Work variables
	//
	Goto_AsyncGapDA** m_requests;
	KyUInt32 m_requestCount;
};

} // namespace Kaim

#endif // KY_GOTO_ASYNCGAPDA_ASYNCMODULE_ST_H
