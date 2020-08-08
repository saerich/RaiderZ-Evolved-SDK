/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kyruntime_perceptions */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_POINTLOCKMANAGER_H
#define KY_POINTLOCKMANAGER_H

#include <kyruntime/core/service.h>
#include <kyruntime/core/world.h>
#include <kypathdata/math/trigo.h>
#include <kypathdata/math/vec2i.h>

namespace Kaim
{

class Bot;
class PointLockManager;
class Body;

/*! This class is used internally by the PointLockManager to record each position locked by a Body.
	You should not need to interact directly with this class.
	\ingroup kyruntime_perceptions */
class PointLock
{
	friend class PointLockManager;
public:
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! Retrieves a pointer to the Body that locked this point. */
	Body* GetBody() const { return m_body; }

	/*! Retrieves the exact coordinates of the locked position. */
	const Vec3f& GetPosition() const { return  m_position; }

	/*! Retrieves the time at which this PointLock will expire. */
	KyFloat32 GetExpirationDate() const { return m_expirationDate; }

	/*! For internal use. */
	const Vec2i& GetGridPos() const { return m_gridPos; }
public:
	/*! For internal use. */
	PointLock() :
		m_body(KY_NULL),
		m_position(Vec3f::Zero()),
		m_expirationDate(KyFloat32MAXVAL),
		m_gridPos(KyInt32MAXVAL, KyInt32MAXVAL)
	{}
	
	/*! For internal use. */
	PointLock(Body* body, const Vec2i& gridPos) :
		m_body(body),
		m_position(Vec3f::Zero()),
		m_expirationDate(KyFloat32MAXVAL),
		m_gridPos(gridPos)
	{}

	/*! Indicates whether or not the lock is valid. For internal use. */
	bool IsValid() const { return m_body != KY_NULL; }

	/*! Clears the lock. For internal use. */
	void Invalidate()
	{
		m_body = KY_NULL;
		m_position = Vec3f::Zero();
		m_expirationDate = KyFloat32MAXVAL;
		m_gridPos = Vec2i(KyInt32MAXVAL, KyInt32MAXVAL);
	}

protected:
	Body* m_body;
	Vec3f m_position;
	KyFloat32 m_expirationDate;
	Vec2i m_gridPos;
};


/* An instance of this class is maintained by each Body to track the positions it has locked with the PointLockManager. For
	internal use. */
class PointLockDataPerBody
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	PointLockDataPerBody() : m_locks(KY_NULL) {}
	~PointLockDataPerBody() { KY_DELETE_CLASS_ARRAY(m_locks); }
	void Init(const PointLockManager& pointLockManager);
public:
	PointLock m_autoLock;
	PointLock* m_locks;
};


/*! The PointLockManager is an implementation of WorldService that maintains information about the positions and
	destinations of the \Bodies in the World. 
	\Bodies can use the PointLockManager to register or "lock" their current positions and/or their chosen
	destination positions, and to determine whether other given points are locked. This is typically used to
	prevent multiple \Bodies from deciding to move to the same destination points at the same time.
	\pp For example, when the HideAgent supplied with \SDKName attempts to find a destination point,
	it invokes the PointLockManager (if available in the World) to test whether or not each candidate hiding place has
	already been locked by another Body. If the point has already been locked, the HideAgent discards
	the candidate. If the point is not locked, the HideAgent invokes the PointLockManager to lock the point,
	thereby notifying other \Bodies that it intends to occupy that destination.
	\pp Each lock is represented by an instance of the PointLock class.
	\section pointlockersetup Setting up the PointLockManager
	To set up the PointLockManager in your World:
	-#	Call the World::ActivateService() method. For example:
		\code Kaim::SpawnManager* spawnmgr = m_world->ActivateService<Kaim::SpawnManager>(); \endcode
	-#	Call the Initialize() method.
	\section pointlockerusing Using the PointLockManager
	You can use the PointLockManager directly at runtime to lock and unlock points, and to evaluate the locks that exist
	in a given area.
	\section pointlocklocking Locking points on-demand
	To lock a point, call the Lock() method. If the specified Body has already reached its maximum number of locked points,
	the PointLockManager first discards the oldest lock belonging to that Body.
	\section pointlockerautolock Locking Body positions automatically
	To ensure that locks are not discarded prematurely, and to prevent conflicts between \Bodies who do not explicitly lock
	their positions, the PointLockManager can also be configured to automatically lock the positions or all stationary \Bodies 
	in your World.
	\pp In order to improve performance when automatic locking is activated, the PointLockManager uses distance and time thresholds
	to determine when a Body has moved a significant enough distance to make it worthwhile to update the lock. Any Body that
	moves less than the distance threshold within the time threshold is considered stationary, and its lock is not updated. 
	\pp This feature is activated by default. You can turn it on or off and configure its thresholds in either of two ways:
	-	In an AutoLockData object that you pass in the call to Initialize().
	-	By calling the SetAutomaticLockStatus(), SetImmobilityDistance() and SetImmobilityDelay() methods at runtime.
	\section pointlocktesting Testing positions and retrieving locks
	To test whether a Bot can move to a given destination point, it is not sufficient to test simply whether a lock exists on
	that exact point in three-dimensional space. If the point you test is open, but another point one centimeter away is locked,
	your Bot will still be in conflict with the Body owning the lock, even if the destination points are not exactly the same.
	\pp Therefore, when testing whether a position is locked, you provide the PointLockManager with a region in three-dimensional
	space defined by a position and a squared radius. The PointLockManager will return a list of all locks found within this region.
	\pp To retrieve locks, call the GetFirstLock() method, and provide the area within which you want to look.
	-	If this method returns #KY_NULL, no locks exist in the specified area and your destination is free.
	-	If this method returns a valid pointer, you can use the API provided by the PointLock class to test the exact position of
		the lock, and the Body that owns the lock. You can also call GetNextLock() in a loop to retrieve additional locks in the
		area. When GetNextLock() returns #KY_NULL, you have retrieved all locks in the area.
	\section pointlockunlock Unlocking points
	You can unlock points programmatically at any time to indicate to other \Bodies when those positions are no longer occupied. Call
	one of the versions of Unlock() to unlock a single point or all points locked by a single Body, or call UnlockAll() to remove
	all locks for all \Bodies.
	\pp In addition, the PointLockManager automatically discards points that have been locked for longer than a configurable time
	threshold. This threshold can be set in the \c defaultExpirationDelay parameter you pass to Initialize(), or through a call
	to SetExpirationDelay().
	\ingroup kyruntime_perceptions */
class PointLockManager: public WorldService
{
public:
	KY_DECLARE_WORLDSERVICE(PointLockManager)
protected:
	/* Used internally by the PointLockManager to detect when Bodies are added to and removed from the World, in order to 
		reserve and clear a buffer used by each Body to store its list of locks. */
	class WorldObserver : public IWorldObserver
	{
	public:
		WorldObserver() : m_pointLockManager(KY_NULL) {}
		void Init(PointLockManager& pointLockManager) { m_pointLockManager = &pointLockManager; }

		virtual void OnActivateBody(Body& body) { PointLockDataPerBody*& data = m_pointLockManager->GetPointLockData(body); data = KY_NEW_CLASS(PointLockDataPerBody); data->Init(*m_pointLockManager); }
		virtual void OnDeactivateBody(Body& body) { m_pointLockManager->Unlock(body); PointLockDataPerBody*& data = m_pointLockManager->GetPointLockData(body); KY_DELETE_CLASS(data); }
	protected:
		PointLockManager* m_pointLockManager;
	};

	/* Used internally by the PointLockManager to iterate through the locked positions. */
	class ConstIterator
	{
	public:
		ConstIterator();

		const PointLock* GetFirstLock(const PointLockManager* pointLockManager, const Vec3f& position, KyFloat32 sqDistMax);
		const PointLock* GetNextLock();
	protected:
		const PointLock* GetNextLock_Linear();
		const PointLock* GetNextLock_Dichotomic();
	protected:
		const PointLockManager* m_pointLockManager;
		Vec2i m_abMin, m_abMax, m_ab;
		KyInt32 m_iMin, m_iMax;
		KyInt32 m_i;
		KyBool m_negativeInc;
		Vec3f m_position;
		KyFloat32 m_sqDistMax;
		KyBool m_dichotomicSearch;
	};

	friend class WorldObserver;
	friend class ConstIterator;
	friend class PointLock;
	friend class PointLockDataPerBody;
public:

	/*! \label_baseconstructor */
	PointLockManager(Kaim::World& world);

	/*! \label_virtualdestructor */
	virtual ~PointLockManager();

	/*! The PointLockManager uses an instance of this class to determine whether or not it should lock Body positions
		automatically, and the configuration parameters that control automatic point locking. You can pass an instance of this
		class when you call Initialize().
		\ingroup kyruntime_perceptions */
	struct AutoLockData
	{
		/*! \label_constructor Sets all data members of this class to their default values. */
		AutoLockData() : m_squareEpsilon(0.025f), m_immobilityDelay(0.5f), m_isActive(KY_TRUE) {}

		/*! Determines the maximum squared distance a Body may move within the time period specified by the #m_immobilityDelay member in
			order for the PointLockManager to consider that Body to be stationary.
			\units			meters (squared)
			\defaultvalue	\c 0.025f */
		KyFloat32 m_squareEpsilon;

		/*! Determines the length of time after which the PointLockManager considers a Body to be stationary if the Body has not moved more
			than the distance specified by the #m_squareEpsilon member.
			\units			seconds
			\defaultvalue	\c 0.5f */
		KyFloat32 m_immobilityDelay;

		/*! Determines whether or not the PointLockManager automatically locks the current positions of all stationary \Bodies.
			\defaultvalue	#KY_TRUE */
		KyBool m_isActive;
	};

	/*! \label_init
		\param maxLockPerBody			Sets the maximum number of points that any Body can lock at any given time.
										\defaultvalue	\c 5
		\param gridPitch				Determines the edge length of internal space partitions. Do not change the default value of this
										parameter unless instructed by Autodesk support.
										\defaultvalue	\c 5.0f
		\param defaultExpirationDelay	Determines the maximum time a point may remain locked before the PointLockManager considers the lock
										expired and unlocks the point. 
										\units			seconds
										\defaultvalue	\c 0.25f
		\param autolock					An instance of the AutoLockData class that configures whether or not the PointLockManager should lock
										the current position of each stationary Body automatically. */
	KyResult Initialize(KyUInt32 maxLockPerBody=5, KyFloat32 gridPitch=5.f, KyFloat32 defaultExpirationDelay=0.25f, AutoLockData autolock = AutoLockData());

	virtual void Update();
	virtual void ReInit();

	/*! Locks a position for use by a specified Body. If the Body has already locked its maximum number of points (set in the
		PointLockManager constructor), the oldest lock belonging to that Body is overwritten.
		\param lockingBody 		The Body locking the specified position.
		\param position			The coordinates of the position to be locked.
		\param expirationDelay 	The maximum time this PointLock can remain valid. When this interval is 
								exceeded, the PointLock is automatically removed. If this value is set to 0,
								the default value is used (set in the constructor and accessble through 
								GetExpirationDelay() and SetExpirationDelay()).
		\return A pointer to the PointLock, or #KY_NULL if the PointLock could not be set. */
	PointLock* Lock(Body& lockingBody, const Vec3f& position, KyFloat32 expirationDelay = 0.f);

	/*! Removes the specified PointLock. */
	void Unlock(PointLock& lock);

	/*! Removes a PointLock defined by the specified position and the specified Body, if one exists. */
	void Unlock(Body& lockingBody, const Vec3f& position);

	/*! Removes all \PointLocks related to the specified Body. */
	void Unlock(Body& lockingBody);

	/*! Removes all \PointLocks related to all \Bodies. */
	void UnlockAll();

	/*! Retrieves the first PointLock found within an area defined by a given central position and
		a radius around that central position. To retrieve other \PointLocks contained within this area,
		call GetNextLock() in a loop.
		\pp The GetFirstLock() and GetNextLock() functions are designed to provide efficient results
		for relatively small \c sqDistMax values, such as those close to the square of the width of an Body.
		If you need to retrieve all \PointLocks in the World, you can set \c sqDistMax to #KyFloat32MAXVAL,
		but the calculation may adversely affect performance.
		\param position   The center of the area to be tested.
		\param sqDistMax  The square of the radius around the center position.
		\return The first PointLock found in the specified area, or #KY_NULL if no PointLock exists in the
				specified area. */
	const PointLock* GetFirstLock(const Vec3f& position, KyFloat32 sqDistMax) const { return m_it.GetFirstLock(this, position, sqDistMax); }

	/*! Retrieves the next PointLock in the specified area. Must be preceded by a call to GetFirstLock().
		See GetFirstLock() for details.
		\return The next PointLock found in the area originally given to the GetFirstLock() method,
				or #KY_NULL if all \PointLocks in the area have already been returned. */
	const PointLock* GetNextLock() const { return m_it.GetNextLock(); }

	// accessors
	/*!	Retrieves the value set through a call to SetImmobilityDelay(). */
	KyFloat32 GetImmobilityDelay() const { return m_autoLock.m_immobilityDelay; }

	/*! Determines the length of time after which the PointLockManager considers a Body to be stationary if the Body has not moved more
		than the distance returned by GetImmobilityDistance(). */
	void SetImmobilityDelay(KyFloat32 immobilityDelay) { m_autoLock.m_immobilityDelay = immobilityDelay; }

	/*!	Retrieves the value set through a call to SetImmobilityDistance(). */
	KyFloat32 GetImmobilityDistance() const { return sqrtf(m_autoLock.m_squareEpsilon); }

	/*! Determines the maximum distance, in meters, that a Body may move within the time period returned by GetImmobilityDelay() in
		order for the PointLockManager to consider that Body to be stationary. */
	void SetImmobilityDistance(KyFloat32 epsilon) { m_autoLock.m_squareEpsilon = Square(epsilon); }

	/*!	Retrieves the value set through a call to SetExpirationDelay(). */
	KyFloat32 GetExpirationDelay() const { return m_defaultExpirationDelay; }

	/*! Determines the maximum time, in seconds, that a point may remain locked before the PointLockManager considers the lock
		expired and unlocks the point. */
	void SetExpirationDelay(KyFloat32 expirationDelay) { m_defaultExpirationDelay = expirationDelay; }

	/*! Returns #KY_TRUE if automatic \PointLocks are generated for stationary \Bodies, or #KY_FALSE otherwise. */
	KyBool GetAutomaticLockStatus() const { return m_autoLock.m_isActive; }

	/*! Determines whether or not the PointLockManager will automatically lock the positions of all stationary Bodies. 
		\pp If you invoke this method to change the automatic PointLock status to #KY_FALSE at runtime, 
		all previously recorded automatic \PointLocks are immediately removed. */
	void SetAutomaticLockStatus(KyBool status);

	/*! Renders all active \PointLocks for debugging purposes. Each PointLock is rendered by a vertical line
		that extends upwards from the locked point. The length and color of this line is controlled 
		by a delay threshold:
		- 	If the time remaining before the expiration date of a PointLock is greater than the 
			delay threshold, the line is drawn in red, and is one meter in length. 
		- 	Once the time remaining before the expiration date becomes less than the specified delay, 
			the color of the line shifts progressively from red to green. In addition, the line 
			becomes progressively shorter, until it finally disappears at the moment that the PointLock 
			reaches its expiration date.
		\param delay  Specifies the delay threshold, in seconds. The default value is 3. */
	void Render(KyFloat32 delay = 3.f) const;

private:
	PointLockDataPerBody*& GetPointLockData(Body& body) { return body.m_pointLockData; }
	void Lock(PointLock& lock);
	static inline Vec2i GetABFromPosition(const Vec3f& v, const KyFloat32 pitch);
private:

	Array<PointLock*> m_sortedLocks;

	KyUInt32 m_maxLockPerBody;
	KyFloat32 m_defaultExpirationDelay;

	KyFloat32 m_gridPitch;
	AutoLockData m_autoLock;
	WorldObserver m_worldObserver;
	mutable ConstIterator m_it;
};

} // namespace Kaim

#endif // KY_POINTLOCKMANAGER_H
