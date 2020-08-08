/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_bodyinfomanagement */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_BODYINFOMANAGER_H
#define KY_BODYINFOMANAGER_H

#include <kyruntime/core/service.h>
#include <kyruntime/core/world.h>

#include <kypathdata/containers/array.h>
#include <kypathdata/math/box2i.h>

namespace Kaim
{

class BodyInfo;
class BodyInfoClass;
class BodyInfoFilter;
class PointWrapper;
class TimeManager;
class Body;

class BodyInfoManager;

/////////////////////////////////////////////////////////////////////////////////
// BodyProfile
/////////////////////////////////////////////////////////////////////////////////

/*! This class is used to identify to the BodyInfoManager the different types of perceptual
	data that should be calculated for different types of \Bodies. Each BodyProfile provides a link
	between a specific type of Body and the set of \BodyInfos that must be computed and maintained for that Body.
	\pp For details, see :USERGUIDE:"Tracking Perception Data".
	\section usingbodyinfoprofiles Using BodyInfoProfiles
	\pp To create an instance of the BodyProfile class, you call BodyInfoManager::RegisterProfile(). The lifespan of
	that BodyProfile object is managed by the BodyInfoManager. You then assign one or more BodyInfo classes to the newly
	created BodyProfile by calling BodyInfoManager::ActivateBodyInfoToProfile(). The BodyProfile creates instances of the
	specified BodyInfo classes and manages their lifespans.
	\ingroup kyruntime_bodyinfomanagement */
class BodyProfile
{
	KY_DEFINE_NEW_DELETE_OPERATORS

	friend class BodyInfoManager;
	friend class Body;
public:
	// *** body info ***

	/*! Retrieves a pointer to the first BodyInfo assigned to this BodyProfile whose metaclass matches or
		derives from the specified MetaClass.
		\param bodyInfoClass			The MetaClass of the BodyInfo to retrieve.
		\return A pointer to the BodyInfo, or #KY_NULL if none match the specified MetaClass. */
	BodyInfo* GetBodyInfo(const BodyInfoClass& bodyInfoClass);

	/*! Retrieves a pointer to the first BodyInfo assigned to this BodyProfile whose class matches or
		derives from the specified class.
		\tparam T			The class of the BodyInfo to retrieve.
		\return A pointer to the BodyInfo, or #KY_NULL if none match the specified class. */
	template < class T >
	T* GetBodyInfo() { return static_cast<T*>(GetBodyInfo(T::Class)); }

	/*! Retrieves a pointer to the first BodyInfo assigned to this BodyProfile whose MetaClass matches
		the specified MetaClass exactly.
		\param bodyInfoClass			The MetaClass of the BodyInfo to retrieve.
		\return A pointer to the BodyInfo, or #KY_NULL if none match the specified MetaClass. */
	BodyInfo* GetExactBodyInfo(const BodyInfoClass& bodyInfoClass);

	/*! Retrieves a pointer to the first BodyInfo assigned to this BodyProfile whose class matches the
		specified class exactly.
		\tparam T			The class of the BodyInfo to retrieve.
		\return A pointer to the BodyInfo, or #KY_NULL if none match the specified class. */
	template < class T >
	T* GetExactBodyInfo() { return static_cast<T*>(GetExactBodyInfo(T::Class)); }

	// *** accessors ***

	/*! Retrieves the name set for this BodyProfile at the time of its registration. */
	const char* GetName() const { return m_name; }

	/*! Retrieves the list of all \Bodies that have been set up with this BodyProfile. */
	const Array<Body*>& GetBodies() const { return m_bodies; }

	/*! Retrieves the list of all BodyInfo objects assigned to this BodyProfile. */
	const Array<BodyInfo*>& GetInfos() const { return m_infos; }

private:
	BodyProfile& operator=(const BodyProfile& eP); // No implementation.

	BodyProfile(BodyInfoManager& bodyInfoManager, const char* name);
	~BodyProfile();


	bool AddBody(Body& body);
	void RemoveBody(Body& body);


	BodyInfoManager& m_bodyInfoManager;
	Array<Body*> m_bodies;

	Array<BodyInfo*> m_infos; // index = BodyInfoClass id
	char* m_name;
};

/////////////////////////////////////////////////////////////////////////////////
// BodyInfoManager
/////////////////////////////////////////////////////////////////////////////////

/*!	The BodyInfoManager is a WorldService that manages the computation of perceptual
	information about \Bodies and their locations in the World.
	\pp For complete details, see :USERGUIDE:"Tracking Perception Data".
	\ingroup kyruntime_bodyinfomanagement */
class BodyInfoManager : public WorldService
{
	friend class BodyInfo;
protected:
	class Iterator;

	/* The BodyInfoManager creates and initializes an instance of this class to ensure that any Body that is removed 
		from the World is also un-registered from the BodyProfiles and BodyInfos maintained by the BodyInfoManager. */
	class WorldObserver : public IWorldObserver
	{
	public:
		WorldObserver() : m_bodyInfoManager(KY_NULL) {}
		void Initialize(BodyInfoManager& bodyInfoManager) { m_bodyInfoManager = &bodyInfoManager; }

		virtual void OnActivateBody(Body& ) {}
		virtual void OnDeactivateBody(Body& body) { m_bodyInfoManager->OnRemoveBody(body);}
	protected:
		BodyInfoManager* m_bodyInfoManager;
	};
	friend class WorldObserver;
public:
	KY_DECLARE_WORLDSERVICE(BodyInfoManager)

public:
	/*! \label_constructor \label_mustinit
		\param world			The World that contains the BodyInfoManager. */
	BodyInfoManager(Kaim::World& world);

	/*! \label_virtualdestructor */
	virtual ~BodyInfoManager();

	/*! Used internally by the BodyInfoManager to track the location of each Body in the World.
		\ingroup kyruntime_bodyinfomanagement */
	struct BodyPos
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
	public:
		BodyPos() : body(KY_NULL), m_gridPos(KyUInt32MAXVAL, KyUInt32MAXVAL) {}
		BodyPos(const Vec2i& gridPos) : body(KY_NULL), m_gridPos(gridPos) {}
	public:
		Kaim::Body* body;
		Vec2i m_gridPos;
	};

	/*! Used internally by the BodyInfoManager to partition the World into a grid used to track
		the locations of all \Bodies.
		\ingroup kyruntime_bodyinfomanagement */
	class Partitioner
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
		friend class BodyInfoManager;
		friend class Iterator;
	public:
		/*! \label_constructor
			\param gridPitch		
			\param gridUpdatePeriod	 */
		Partitioner(KyFloat32 gridPitch = 10.f, KyFloat32 gridUpdatePeriod = 0.3f);

		/*! \label_virtualdestructor */
		~Partitioner();

		/*! Indicates whether or not this Partitioner instance has been correctly set up. */
		bool IsValid() const
		{
			KY_FUNCTION("BodyInfoManager::Partitioner::IsValid");
			bool isValid = m_gridPitch > 0 && m_gridUpdatePeriod > 0;
			KY_COND_MESSAGE(KY_LOG_ERROR, !isValid, ("m_gridPitch and m_gridUpdatePeriod must be strictly positive"));
			return isValid;
		}
	protected:
		void UpdateAreas();
		Vec2i GetGridPosition(const Vec3f& v) const;
		void Render() const;
	protected:
		World* m_world;
		Array<BodyPos> m_areas;
		KyFloat32 m_gridPitch;
		KyFloat32 m_gridUpdatePeriod;
		KyFloat32 m_nextUpdateAreaTime;
	};

	/*! Used internally by the BodyInfoManager to manage the timestamps used to track the last update times
		for each BodyInfo.
		\ingroup kyruntime_bodyinfomanagement */
	struct DatePacking
	{
		/*! Determines the time interval between each successive timestamp recorded by the BodyInfoManager.
			\units			seconds
			\defaultvalue	\c 0.5f */
		KyFloat32 m_timerAccuracy;

		/*! Determines the period at which the BodyInfoManager updates the date IDs it has assigned to each BodyInfo.
			It is recommended that you do not change the default value of this parameter unless instructed by Autodesk
			support, or unless the BodyInfoManager::GetDatesUpdateStatus() method returns #KY_FALSE.
			\units			seconds
			\acceptedvalues	any value greater than #m_timerAccuracy
			\defaultvalue	\c 5.0f */
		KyFloat32 m_dateUpdatePeriod;

		/*! Determines the size in memory of the date ID the BodyInfoManager records for each BodyInfo when that BodyInfo is updated.
			Larger values increase the precision of the date IDs, but also increase the memory consumption of the BodyInfoManager. 
			\units			bits
			\acceptedvalues	between 1 and 16
			\defaultvalue	\c 4 */
		KyUInt32 m_bitsPerDateId;

		/*! Indicates whether or not this DatePacking instance has been correctly set up. */
		bool IsValid() const;

		/*! Retrieves the maximum number of unique timestamps that can be stored. */
		KyUInt32 GetMaxDates() const { return 1 << m_bitsPerDateId; }

		/*! Retrieves the unique ID used to identify invalid timestamps. */
		KyUInt32 GetInvalidId() const { return GetMaxDates() - 1; }

		/*! Retrieves the maximum total span of time that can be covered by the timestamps recorded by this DatePacking object. */
		KyFloat32 GetMaxPeriod() const { return GetInvalidId() * m_timerAccuracy; }

		/*! Retrieves the maximum date timestamp value that this DatePacking object can record. */
		KyFloat32 GetMaxTime() const { return m_dateUpdatePeriod - m_timerAccuracy; }

		/*! \label_constructor Sets all data members of this class to their default values. */
		DatePacking()
			:m_timerAccuracy(0.5f)
			,m_dateUpdatePeriod(5.f)
			,m_bitsPerDateId(4)
		{}
	};

	/*! \label_init
		\param allowedAverageWorkTimePerFrameInSeconds	The maximum amount of time that the BodyInfoManager::Update() method may spend updating
														BodyInfo values in each frame.
														\units			seconds
		\param estimationsReliabilty					Indicates the extent to which the BodyInfoManager trusts the estimated CPU consumption provided
														by each BodyInfo, retrieved by calling BodyInfo::GetEstimatedCpuConsumption(). This is used to predict
														how many BodyInfo values the BodyInfoManager should attempt to update in each frame.
														\defaultvalue	\c 0.9f
		\param partitionerTemplate						An instance of the Partitioner() class that defines the way the BodyInfoManager divides up the
														World into a grid used to track the positions of the Bodies.
		\param dateInfo									An instance of the DatePacking() class that defines the way the BodyInfoManager records timestamps
														for the last update time of each BodyInfo value.
		\param forceValidDates							When set to #KY_TRUE, the BodyInfoManager will automatically update all date timestamps for all BodyInfos
														if it detects that the circular list of possible timestamps has been exceeded before all BodyInfo values
														could be re-computed. This avoids the possibility of temporary corruptions in the status of the older
														BodyInfo values, but may cause CPU spikes.
														\defaultvalue	#KY_FALSE */
	KyResult Initialize(KyFloat32 allowedAverageWorkTimePerFrameInSeconds, KyFloat32 estimationsReliabilty=0.9f, const Partitioner& partitionerTemplate = Partitioner(), const DatePacking& dateInfo = DatePacking(), KyBool forceValidDates=KY_FALSE);

	/*! Pre-allocates the memory buffer used to store BodyInfo objects created by the BodyInfoManager. If this buffer is
		exceeded at runtime, additional memory will be allocated dynamically. */
	void ReserveBodyInfos(KyUInt32 size) { m_infos.Reserve(size); }

	/*! Pre-allocates the memory buffer used to store BodyProfile objects created by the BodyInfoManager. If this buffer is
		exceeded at runtime, additional memory will be allocated dynamically. */
	void ReserveProfiles(KyUInt32 size) { m_profiles.Reserve(size); }

	/*! Pre-allocates the memory buffer used to store BodyInfoFilter objects created by the BodyInfoManager. If this buffer is
		exceeded at runtime, additional memory will be allocated dynamically. */
	void ReserveFilters(KyUInt32 size) { m_filters.Reserve(size); }

	/*! \label_reinit */
	virtual void ReInit();

	virtual void Update();

	/*! Draws a green circle around the head of each Body, and writes in white the ID number of the partition cell that contains
		that Body. */
	void Render();

	// ********************** Body tracking **********************

	/*! Retrieves a pointer to the first Body found that is within the area described by a
		specified central position and a radius around that central position. To retrieve other
		\Bodies within this area, follow a call to this method by a loop of calls to GetNextBody().
		\param position			The center of the area to be tested.
		\param distMax			The radius around the central position.
		\return  A pointer to the first Body found in the specified area, or #KY_NULL if no Body
				lies within that area. */
	Kaim::Body* GetFirstBody(const Kaim::Vec3f& position, KyFloat32 distMax);

	/*! Retrieves the next Body in the area specified in the last call to GetFirstBody().
		This method must be preceded by a call to GetFirstBody(). See GetFirstBody() for details.
		\return A pointer to the next Body found within the area, or #KY_NULL if all \Bodies in the
				area have already been returned. */
	Kaim::Body* GetNextBody();

	/*! Computes the area within a specified two-dimensional capsule that is currently occupied by \Bodies.
		\param start				The starting point for the capsule.
		\param end					The end point for the capsule.
		\param bot					A pointer to the Bot for which the computation has been requested, used to
									determine the width of the capsule test.
		\param[out] capsuleSurface	The total surface area covered by the capsule.
		\param[out] crowdedSurface	The total surface area within the boundaries of the capsule that is occupied
									by \Bodies other than \c bot.
		\return #KY_TRUE if the computation succeeded, or #KY_FALSE otherwise. */
	KyBool ComputeCrowdDensity(const PointWrapper& start, const PointWrapper& end, const Bot* bot, KyFloat32& capsuleSurface, KyFloat32& crowdedSurface);

	// ********************** BodyInfo management ********************** 

	// *** profiles ***

	/*! Retrieves a pointer to the BodyProfile registered with the specified name.
		\param profileName			The name of the BodyProfile to retrieve.
		\return A pointer to the specified BodyProfile, or #KY_NULL if no BodyProfile
				registered with the BodyInfoManager matches the specified name. */
	BodyProfile* GetProfileByName(const char* profileName) const;

	/*! Retrieves the list of all BodyProfile objects registered with the BodyInfoManager. */
	const Array<BodyProfile*>& GetProfiles() const { return m_profiles; }

	// *** filters ***

	/*! Retrieves the list of all BodyInfoFilter objects registered with the BodyInfoManager. */
	const Array<BodyInfoFilter*>& GetFilters() const { return m_filters; }

	/*! Instructs the BodyInfoManager to create and register a new BodyInfoFilter of the specified class.
		\tparam TBodyInfoFilter		The name of the BodyInfoFilter class to create and register.
		\param name					A unique name for the new BodyInfoFilter.
		\return A pointer to the newly created BodyInfoFilter object. */
	template <class TBodyInfoFilter>
	TBodyInfoFilter* RegisterFilter(const char* name)
	{
		TBodyInfoFilter* filter = KY_NEW_CLASS(TBodyInfoFilter)(name);
		m_filters.PushBack(filter);
		return filter;
	}

	/*! Instructs the BodyInfoManager to create and register a new BodyProfile of the specified class. 
		\param name					A unique name for the new BodyProfile.
		\return A pointer to the newly created BodyProfile object. */
	BodyProfile* RegisterProfile(const char* name);

	/*! Instructs the BodyInfoManager to create a new BodyInfo object of the specified class, and assign it to the
		specified BodyProfile.
		\tparam TBodyInfo			The name of the BodyInfo class to create.
		\param name					A unique name for the new BodyInfo.
		\param profile				The BodyProfile to which the new BodyInfo will be assigned.
		\return A pointer to the newly created BodyInfo object. */
	template < class TBodyInfo >
	TBodyInfo* ActivateBodyInfoToProfile(const char* name, BodyProfile& profile)
	{
		KY_FUNCTION("BodyProfile::ActivateBodyInfo");

		KY_ASSERT(&(profile.m_bodyInfoManager)==this, ("Given profile doesn't belong to this BodyInfoManager"));
		KY_ASSERT(profile.m_infos[TBodyInfo::Class.m_id] == KY_NULL, ("This info class [%s] appears several times in the profile [%s]", TBodyInfo::Class.GetName(), profile.GetName()));

		TBodyInfo* info = KY_NEW_CLASS(TBodyInfo)(name, *this, profile);
		info->ReInit();
		m_infos.PushBack(info);
		profile.m_infos[TBodyInfo::Class.m_id] = info;

		return info;
	}

	/*! Retrieves a pointer to the BodyInfoFilter with the specified name.
		\param filterName			The name of the BodyInfoFilter to retrieve.
		\return A pointer to the specified BodyInfoFilter, or #KY_NULL if no BodyInfoFilter registered with the
				BodyInfoManager matches the specified name. */
	BodyInfoFilter* GetFilterByName(const char* filterName) const;

	// *** dates ***

	/*! Returns the current timestamp used by the BodyInfoManager. Note that
		this date may differ from the date returned by GetWorld().GetAccumulatedTimeInUpdate(),
		depending on the value of the DatePacking::m_timerAccuracy value used to initialize the BodyInfoManager. */
	KyFloat32 GetCurrentDate() const { return m_dates[m_currentDateId]; }

	/*! Returns the position of the current date within the circular list of dates maintained by the
		BodyInfoManager. This value is always between 0 and the value returned by GetDateCount(). */
	KyUInt32 GetCurrentDateId() const { return m_currentDateId; }

	/*! Returns the special ID the BodyInfoManager uses to represent an invalid date.
		This value depends on the DatePacking::m_bitsPerDateId value used to initialize the BodyInfoManager, and is equivalent
		to: \c 2^m_bitsPerDateId. */
	KyUInt32 GetInvalidDateId() const { return m_invalidDateId; }

	/*! Returns a 32-bit absolute date calculated from the specified date ID. The accuracy of this date
		depends on the DatePacking::m_timerAccuracy value used to initialize the BodyInfoManager. */
	KyFloat32 GetDate(KyUInt32 dateId) const;

	/*! Retrieves the DatePacking object used to initialize the BodyInfoManager. */
	DatePacking GetDateInfo() const { return m_dateInfo; }

	// *** debug / deterministic timer tuning ***

	/*! Returns the CPU time consumed during the last update of the BodyInfoManager. */
	KyFloat32 GetLastUpdateDuration() const { return m_lastUpdateDuration; }

	/*! Returns the total CPU time needed to update all \BodyInfos managed by the BodyInfoManager
		during the last full BodyInfo update loop. */
	KyFloat32 GetDataUpdateLoopDuration() const { return m_dataUpdateLoopDuration; }

	/*! Returns the total CPU time needed to update all dates managed by the BodyInfoManager
		during the last full date update loop. */
	KyFloat32 GetDatesUpdateLoopDuration() const { return m_datesUpdateLoopDuration; }

	/*! Indicates whether or not the BodyInfoManager has enough date stamps to handle the
		time taken in the date status update loop. If this method returns #KY_FALSE, reduce the value of
		the DatePacking::m_dateUpdatePeriod value used to initialize the BodyInfoManager. */
	KyBool GetDatesUpdateStatus() const { return m_datesUpdateStatus; }

	/*! Retrieves the value set by a call to SetForceValidDates(). */
	KyBool GetForceValidDates() const { return m_forceValidDates; }

	/*! When set to #KY_TRUE, the BodyInfoManager will automatically update all date timestamps for all BodyInfos
		if it detects that the circular list of possible timestamps has been exceeded before all BodyInfo values
		could be re-computed. This avoids the possibility of temporary corruptions in the status of the older
		BodyInfo values, but may cause CPU spikes. */
	void SetForceValidDates(KyBool b) { m_forceValidDates = b; }

	// *** misc ***

	/*! Returns the total number of \BodyInfos managed by the BodyInfoManager:
		i.e. the total number of \BodyInfos computed for all \Bodies. */
	KyUInt32 GetCurrentInfoCellCount() const { return m_infoCellCount; }

	/*! Returns the number of types of BodyInfo configured for the BodyInfoManager:
		i.e. the number of calls made to ActivateBodyInfoToProfile(). */
	const Array<BodyInfo*>& GetInfos() const { return m_infos; }

	/*! Retrieves the amount of CPU time allotted to the update of the BodyInfoManager at each frame, as set in the
		call to Initialize(). */
	KyFloat32 GetMaxUpdateTime() const;

	/*! This method is called transparently whenever a Body is added or removed from the World, in order to
		update the number of \BodyInfos that need to be computed by the BodyInfoManager. */
	void UpdateInfoCount();

protected:
	void UpdateDates(KyUInt32& maxCount);

	void OnRemoveBody(Body&);


protected:
	class Iterator
	{
	public:
		Iterator();

		Body* GetFirstBody(BodyInfoManager* bodyInfoManager, const Vec3f& position, KyFloat32 distMax);
		Kaim::Body* GetNextBody();

	private:
		BodyInfoManager* m_bodyInfoManager;
		BodyPos* m_it;
		Box2i m_abBox;
		Vec2i m_ab; // iterator for the box
		Vec3f m_position;
		KyFloat32 m_sqDistMin;
		BodyPos* m_end;
	};


protected:
	Partitioner m_partitioner;
	Iterator m_it;

	// ********************** BodyInfo management ********************** 

	Array<BodyProfile*> m_profiles;
	Array<BodyInfoFilter*> m_filters;
	Array<BodyInfo*> m_infos;
	KyUInt32 m_nextComputationId; // next info to be computed
	KyUInt32 m_infoCellCount;
	KyBool m_forceValidDates;

	// *** time management ***
	KyFloat32 m_averageTpf;
	KyUInt32 m_nextDateId; // id of the next info whose date will be updated
	DatePacking m_dateInfo;
	KyFloat32 m_maxPeriod;
	KyFloat32* m_dates;
	KyUInt32 m_invalidDateId;
	KyUInt32 m_currentDateId;
	KyFloat32 m_estimationsReliabilty;

	// *** debug / tuning ***
	KyFloat32 m_lastUpdateDuration; // CPU consumption inside the last update
	KyFloat32 m_dataUpdateLoopTime;
	KyFloat32 m_dataUpdateLoopDuration;
	KyFloat32 m_datesUpdateLoopTime;
	KyFloat32 m_datesUpdateLoopDuration;
	KyBool m_datesUpdateStatus;
	WorldObserver m_worldObserver;
};



} // namespace Kaim

#endif // KY_BODYINFOMANAGER_H
