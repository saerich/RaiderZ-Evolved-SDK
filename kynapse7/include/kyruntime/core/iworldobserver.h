/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/



/*!	\file
	\ingroup kyruntime_core */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_IWORLDOBSERVER_H
#define KY_IWORLDOBSERVER_H

namespace Kaim
{

/*! IWorldObserver is an abstract base interface for an object that processes notifications each time a Body is added to or
	removed from a World.
	To use this class:
	-	Write your own class that implements IWorldObserver, and implement either or both of the OnActivateBody() or OnDeactivateBody() methods.
	-	After you create your World, create an instance of your IWorldObserver class.
	-	Pass your IWorldObserver object to your World in a call to World::RegisterWorldObserver().
	\pp Every time a Body is added to the World, the OnActivateBody() method of your IWorldObserver will be called automatically. Every
	time a Body is removed from the World, the OnDeactivateBody method of your IWorldObserver will be called automatically.
	\pp If you need to de-activate your IWorldObserver at any time, you can remove it by calling World::UnregisterWorldObserver().
	\ingroup kyruntime_core */
class IWorldObserver: public VirtualMemObject
{
public:
	/*! \label_virtualdestructor */
	virtual ~IWorldObserver() {}

	/*! Called by the World to notify the IWorldObserver that the specified Body has just been initialized in the World. */
	virtual void OnActivateBody(Body& body) = 0;

	/*! Called by the World to notify the IWorldObserver that the specified Body is about to be removed from the World. */
	virtual void OnDeactivateBody(Body& body) = 0;
};

} // namespace Kaim

#endif // KY_IWORLDOBSERVER_H
