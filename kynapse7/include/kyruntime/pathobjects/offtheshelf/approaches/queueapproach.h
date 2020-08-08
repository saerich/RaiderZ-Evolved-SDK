/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathobjects */

#ifndef KYRUNTIME_PATHOBJECT_QUEUE_APPROACH_H
#define KYRUNTIME_PATHOBJECT_QUEUE_APPROACH_H

#include <kypathdata/generic/memory.h>
#include <kypathdata/math/vec3f.h>

namespace Kaim
{

class PathObjectBehaviorContext;
class IConnexionSpaceQueue;

/*!
Approach functions to enter a PathObject queuing.
*/
class QueueApproach
{
public:
	static void OnStartApproaching(PathObjectBehaviorContext& /*context*/);
	static void Approach(PathObjectBehaviorContext& /*context*/);
	static void OnEndApproaching(PathObjectBehaviorContext& /*context*/);
	static bool ApproachFailed(PathObjectBehaviorContext& /*context*/);
	static bool CanStartTraverse(PathObjectBehaviorContext& /*context*/);
};

/*!
Queue to enter a PathObject.
After construction, you must create the queues for each connexion space.
*/
class PathObjectQueue
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	PathObjectQueue(KyUInt32 maxBotQueueing, KyUInt32 nbConnexionSpaces);
	~PathObjectQueue();

public:

	/*!	Return true if an entity located at specified connexionSpace bot could be registered in the pathobject queue.*/
	bool CanRegister(KyUInt32 connexionSpaceIdx);

	/*!	Register a bot in the queue.*/
	bool RegisterBot(void* entity, KyUInt32 connexionSpaceIdx);

	/*!	Unregister a bot from the queue.*/
	void UnregisterBot(void* entity);

	/*!	Get the position a registered bot should go to queue.
		If success, return true, and queue position is set inside parameter out_queuePosition.
		If failure return false.*/
	bool GetQueuePosition(void* entity, Vec3f& out_queuePosition) const;

	/*!	Return true if the specified bot is the first one in the queue.*/
	bool IsBotFirst(void* entity) const;

	/*!	Return true if the bot is registered, else false.*/
	bool IsBotRegistered(void* entity) const;

private:
	/*!	Return queue offset of specified entity. Return -1 if the entity was not found.*/
	KyInt32 GetQueueOffset(void* entity) const;

public:
	KyUInt32 m_maxQueueSize;
	KyUInt32 m_registeredEntityCount;
	void** m_registeredEntities;
	KyUInt32 m_connexionSpaceQueueCount;
	IConnexionSpaceQueue** m_connexionSpaceQueues;
};

/*!
Queue to enter a PathObject.from a connexionSpace.
The first entity will go to the first position, the second to the second and so on.
*/
class IConnexionSpaceQueue
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	virtual ~IConnexionSpaceQueue() {}

	virtual bool CanRegister() = 0;

	/*!	Register a bot in the queue.*/
	virtual bool RegisterBot(void* entity) = 0;

	/*!	Unregister a bot from the queue.*/
	virtual void UnregisterBot(void* entity) = 0;

	/*!	Get the position a registered bot should go to queue.
		If success, return true, and queue position is set inside parameter out_queuePosition.
		If failure return false.*/
	virtual bool GetQueuePosition(void* entity, Vec3f& out_queuePosition) const = 0;

	virtual KyUInt32 GetQueuePositionCount() const = 0;
	virtual bool GetQueuePosition(KyUInt32 queuePositionIndex, Vec3f& out_queuePosition) const = 0;

	/*!	Return true if the specified bot is the first one in the queue.*/
	virtual bool IsBotFirst(void* entity) const = 0;
};

/*!
Queue to enter a PathObject.from a connexionSpace.
The first entity will go to the first position, the second to the second and so on.
*/
class ConnexionSpaceQueue_empty
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	ConnexionSpaceQueue_empty() {}
	virtual ~ConnexionSpaceQueue_empty() {}

	virtual bool CanRegister() { return false; }

	/*!	Register a bot in the queue.*/
	virtual bool RegisterBot(void* /*entity*/) { return true; }

	/*!	Unregister a bot from the queue.*/
	void UnregisterBot(void* /*entity*/) {}

	/*!	Get the position a registered bot should go to queue.
		If success, return true, and queue position is set inside parameter out_queuePosition.
		If failure return false.*/
	bool GetQueuePosition(void* /*entity*/, Vec3f& /*out_queuePosition*/) const { return false; }

	KyUInt32 GetQueuePositionCount() const { return 0; }
	bool GetQueuePosition(KyUInt32 /*queuePositionIndex*/, Vec3f& /*out_queuePosition*/) const { return false; }

	/*!	Return true if the specified bot is the first one in the queue.*/
	bool IsBotFirst(void* /*entity*/) const { return false; }
};

/*!
Queue to enter a PathObject.from a connexionSpace.
After construction, you must fill the 3D positions for each seat in the queue.
You can do this using the providing methods (FillQueuePositions_XXX) or create 
your own shape filling yourself the queue positions.
The first entity will go to the first position, the second to the second and so on.
*/
class ConnexionSpaceQueue_Line : public IConnexionSpaceQueue
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	ConnexionSpaceQueue_Line(KyUInt32 maxQueueSize, const Vec3f& start, const Vec3f& end);
	~ConnexionSpaceQueue_Line();

	void FillQueuePositions_line(const Vec3f& start, const Vec3f& end);

	/*! WARNING: startOrientation must be normalized.*/
	void FillQueuePositions_circle(const Vec3f& center, const Vec2f& startOrientation, KyFloat32 radius, KyFloat32 angleInDegrees);

public:

	bool CanRegister();

	/*!	Register a bot in the queue.*/
	bool RegisterBot(void* entity);

	/*!	Unregister a bot from the queue.*/
	void UnregisterBot(void* entity);

	/*!	Get the position a registered bot should go to queue.
		If success, return true, and queue position is set inside parameter out_queuePosition.
		If failure return false.*/
	bool GetQueuePosition(void* entity, Vec3f& out_queuePosition) const;

	KyUInt32 GetQueuePositionCount() const { return m_maxQueueSize; }
	bool GetQueuePosition(KyUInt32 queuePositionIndex, Vec3f& out_queuePosition) const { out_queuePosition = m_queuePositions[queuePositionIndex]; return true; }

	/*!	Return true if the specified bot is the first one in the queue.*/
	bool IsBotFirst(void* entity) const;

private:
	/*!	Return queue offset of specified entity. Return -1 if the entity was not found.*/
	KyInt32 GetQueueOffset(void* entity) const;

public:
	KyUInt32 m_maxQueueSize;

	Vec3f* m_queuePositions;

	KyUInt32 m_registeredEntityCount;
	void** m_registeredEntities;
};

}

#endif // #ifndef KYRUNTIME_PATHOBJECT_QUEUE_APPROACH_H
