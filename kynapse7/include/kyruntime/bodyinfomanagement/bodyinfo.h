/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/




/*! \file
	\ingroup kyruntime_bodyinfomanagement */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_BODYINFO_H
#define KY_BODYINFO_H

#include <kyruntime/bodyinfomanagement/bodyinfomanager.h>

#include <kyruntime/core/bitarray.h>
#include <kyruntime/core/world.h>

#include <kypathdata/containers/array.h>
#include <kypathdata/containers/list.h>
#include <kypathdata/generic/memory.h>


namespace Kaim
{
class BodyInfoClass;
class BodyProfile;
class BodyInfoFilter;


/////////////////////////////////////////////////////////////////////////////////
// BodyInfo
/////////////////////////////////////////////////////////////////////////////////

/*! BodyInfo is the base class for all types of perceptual information that can be
	calculated and maintained for your \Bots and \Bodies by the BodyInfoManager.
	\pp Each BodyInfo instance is owned by an BodyProfile configured for the BodyInfoManager. Each class
	of BodyInfo calculates and maintains a specific type of perceptual information for each
	Body that has been assigned that BodyProfile. The data for each Body is stored in an independent
	"cell"; for the implementation of this data storage, see the derived ConcreteBodyInfo class.
	\pp You should never need to use this base class directly. Several classes that derive from
	BodyInfo are supplied with the \SDKName SDK to calculate and maintain commonly used kinds of
	perception data, such as the line-of-sight to other \Bodies. If you need to write your own class of
	BodyInfo to represent a different type of perception, you must derive your class from either
	OneBodyInfo or TwoBodiesInfo.
	\pp For more information on \BodyInfos and the BodyInfoManager, see :USERGUIDE:"Tracking Perception Data".
	\ingroup kyruntime_bodyinfomanagement */
class BodyInfo: public VirtualMemObject
{
public:
	/*!	Used to indicate how recently the value stored in an BodyInfo was last calculated, which in
		turn indicates whether or not the value is safe to use. */
	enum ValidityStatus {
		INVALID = 0     /*!< Indicates that the value of the BodyInfo must not be used, and can be corrupted (e.g. it may represent an invalid pointer). */
		, UNSAFE = 1    /*!< Indicates that the BodyInfo has not been updated recently, so its value may no longer accurately reflect the state of the World. */
		, VALID = 2     /*!< Indicates that the BodyInfo was updated recently, so its value should be safe to use. */
		, DUMMY_ValidityStatus = KYFORCEENUMSIZEINT
	};

	/*! Specifies the behavior to use when retrieving an BodyInfo through the OneBodyInfo::GetValue() and
		TwoBodiesInfo::GetValue() methods. */
	enum AccessMode {
		READ_ONLY				/*!< Retrieves the current value of the BodyInfo, regardless of its current status. */
		, RECOMPUTE_IF_TIME		/*!< Requests a recomputation of the BodyInfo if it is out-of-date and if CPU time remains in the current frame. */
		, FORCE_RECOMPUTATION	/*!< Requests a recomputation of the BodyInfo if it is out-of-date, even if no CPU time remains in the current frame. */
		, DUMMY_AccessMode = KYFORCEENUMSIZEINT
	};

public:
	/*! \label_constructor For internal use.
		\param name					A unique name for the BodyInfo.
		\param bodyInfoManager		The BodyInfoManager that will manage the calculations of this BodyInfo.
		\param profile				The BodyProfile that owns this BodyInfo. */
	BodyInfo(const char* name, BodyInfoManager& bodyInfoManager, BodyProfile& profile);

	/*! \label_virtualdestructor For internal use. */
	~BodyInfo();

	/*! \label_getclass */
	virtual BodyInfoClass& GetClass() const = 0;

	/*! \label_reinit */
	virtual void ReInit() {}

	/*! Returns the BodyProfile that owns this BodyInfo. */
	BodyProfile& GetProfile() const { return m_profile; }

	/*! Returns the list of all BodyInfoFilter objects that are currently applied to this BodyInfo. */
	const List<BodyInfoFilter*>& GetFilters() const { return m_filters; }

	/*! Adds a BodyInfoFilter object that will be applied when computing this BodyInfo, to disable
		computation under a specific set of circumstances.
		\param filter			A pointer to the BodyInfoFilter that should be applied.
		\return An iterator whose \c bool operator returns #KY_TRUE if the BodyInfoFilter was successfully added,
				or #KY_FALSE otherwise. */
	List<BodyInfoFilter*>::Iterator AddFilter(BodyInfoFilter* filter);

	/*! Removes a BodyInfoFilter object currently applied to this BodyInfo.
		\param filter			A pointer to the BodyInfoFilter to be removed. You can retrieve this
								pointer through a call to GetFilters().
		\return #KY_TRUE if the BodyInfoFilter was successfully removed, or #KY_FALSE if no BodyInfoFilter matches the specified
				pointer. */
	KyBool RemoveFilter(BodyInfoFilter* filter);

	/*! Returns the number of cells in this BodyInfo that were updated in the current frame: i.e. the number
		of \Bodies for which this BodyInfo was calculated. This information may be useful in deterministic
		time management mode, in order to determine an accurate time estimation for each call to the ComputeInfo()
		method of the BodyInfo class. */
	KyUInt32 GetFrameComputationCount() const { return m_computationCount; }

	/*! Read accessor for #m_estimatedCpuConsumption. */
	KyFloat32 GetEstimatedCpuConsumption() const { return m_estimatedCpuConsumption; }

	// *** accessors ***

	/*! Retrieves the unique name of the BodyInfo set in the constructor. */
	const char* GetName() const { return m_name; }

	/*! Configures the way this BodyInfo evaluates the validity of its data.
		\param validTime			Determines the maximum length of time that this BodyInfo can remain in the #VALID state
									without being re-computed. After this time interval passes, if the BodyInfo is not updated,
									its state is downgraded to #UNSAFE.
									\units			milliseconds
									\defaultvalue	\c 200
		\param unsafeTime			Determines the maximum length of time that this BodyInfo can remain in the #UNSAFE state
									without being re-computed. After this time interval passes, if the BodyInfo is not updated,
									its state is downgraded to #INVALID.
									\units			milliseconds
									\defaultvalue	\c 1000
		\param immediateMode		Determines whether or not the BodyInfo should be recomputed automatically every time its
									value is requested by another component.
									\defaultvalue	#KY_FALSE */
	KyResult SetupTimeInfo(KyFloat32 validTime, KyFloat32 unsafeTime, KyBool immediateMode);

	/*! Indicates whether or not the BodyInfo is recomputed automatically every time its value is
		requested by another component. See SetupTimeInfo(). */
	KyBool ImmediateMode() const { return m_immediateMode; }

	/*! Retrieves the maximum length of time that this BodyInfo can remain in the #VALID state without being re-computed.
		See SetupTimeInfo(). */
	KyFloat32 GetValidTime() const { return m_validTime; }

	/*! Retrieves the maximum length of time that this BodyInfo can remain in the #UNSAFE state without being re-computed.
		See SetupTimeInfo(). */
	KyFloat32 GetUnsafeTime() const { return m_unsafeTime; }

	/*! Retrieves the BodyInfoManager that computes and stores this BodyInfo object. */
	BodyInfoManager& GetBodyInfoManager() { return m_bodyInfoManager; }

	/*! Retrieves the BodyInfoManager that computes and stores this BodyInfo object. */
	const BodyInfoManager& GetBodyInfoManager() const { return m_bodyInfoManager; }

	/*! \label_getworld */
	const World& GetWorld() const;

	/*! \label_getworld */
	World& GetWorld();

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

	// *** functions called by the BodyInfoManager class ***

	/*! Called by transparently by BodyInfoManager::Update() at the begining of each frame to reset the
		count of cells that have been treated in the current frame. */
	void SetFrameComputationCount(KyUInt32 c) { m_computationCount = c; }

	/*! Called by transparently by BodyInfoManager::Update() to update the values stored for the specified number of cells.
		\param[in,out] count		This parameter specifies the maximum number of cells whose values should be updated
									in the current frame. Implementations of this method must decrement this
									variable each time a cell is updated.
		\return #KY_TRUE when the update is over. */
	virtual KyBool Update(KyUInt32& count) = 0;

	/*! Called by transparently by BodyInfoManager::Update() to update the dates stored for the specified number of cells.
		\param[in,out] count		This parameter specifies the maximum number of cells whose dates should be updated
									in the current frame. Implementations of this method must decrement this
									variable each time a cell is updated.
		\return #KY_TRUE when the update is over. */
	virtual KyBool UpdateDates(KyUInt32& count) = 0;

	/*! Returns the current number of cells in this BodyInfo. */
	virtual KyUInt32 GetCellCount() const = 0;

	/*! Invalidates all BodyInfo values computed for the specified Body. */
	virtual void Invalidate(const Body& /*body*/) {}

	/*! This method is called transparently when the specified Body is added to the BodyProfile that
		owns this instance of BodyInfo. You can overload this method if your implementation of this
		class needs to perform additional steps to handle the addition of the new Body. */
	virtual void OnAddBody(const Body& /*body*/) {}

	/*! This method is called transparently just before removing the specified Body from the BodyProfile
		that owns this instance of BodyInfo. You can overload this method if your implementation of
		this class needs to perform additional steps to handle the removal of the Body. */
	virtual void OnRemoveBody(const Body& /*body*/) {}


protected:
	KyBool IsComputationForbidden();
	KyBool IsComputationForbidden(Body& body);
	KyBool IsComputationForbidden(Body& body1, Body& body2);

private:
	/* Copies the state of the specified BodyInfo to this. */
	BodyInfo& operator=(const BodyInfo& eI);

protected:
	BodyInfoManager& m_bodyInfoManager;
	BodyProfile& m_profile;
	char* m_name;

	KyFloat32 m_validTime;
	KyFloat32 m_unsafeTime;
	KyBool m_immediateMode;

	/*! Provides an estimation for the amount of CPU time that will be consumed calculating this BodyInfo value for
		a single cell. The default value set in the base BodyInfo class is 0.00005f, but you can alter this
		estimation in your custom BodyInfo classes in order to make the BodyInfoManager better able to predict the
		number of cells it has time to update in each frame. */
	KyFloat32 m_estimatedCpuConsumption;
	KyUInt32 m_computationCount;
	List<BodyInfoFilter*> m_filters;
};


/*! Macro that must be included in the declaration of any BodyInfo class, to set up the MetaClass
	used by the \SDKName object factory.
	\ingroup kyruntime_bodyinfomanagement */
#define KY_DECLARE_BODYINFO( infoClass ) \
	static Kaim::BodyInfoClass Class; \
	static Kaim::BodyInfo* KyCreate(const char* name, Kaim::BodyInfoManager& bodyInfoManager, Kaim::BodyProfile& profile); \
	Kaim::BodyInfoClass& GetClass() const { return infoClass::Class; }

/*! Macro that must be included in the implementation of any BodyInfo class, to set up the MetaClass
	used by the \SDKName object factory.
	\ingroup kyruntime_bodyinfomanagement */
#define KY_IMPLEMENT_BODYINFO( infoClass, baseClass) \
	Kaim::BodyInfoClass infoClass::Class( #infoClass, infoClass::KyCreate, baseClass); \
	Kaim::BodyInfo* infoClass::KyCreate(const char* name, Kaim::BodyInfoManager& bodyInfoManager, Kaim::BodyProfile& profile) \
	{ \
		KY_LOG_MEMORY(infoClass::Class.GetName()); \
		Kaim::BodyInfo* ptr = (Kaim::BodyInfo*) KY_NEW_CLASS(infoClass)(name, bodyInfoManager, profile); \
		KY_REGISTER_METACLASS_VPTR(infoClass::Class, (*(void**)ptr)); \
		return ptr; \
	}


/////////////////////////////////////////////////////////////////////////////////
// BodyInfoClass
/////////////////////////////////////////////////////////////////////////////////

/*!	Defines the signature for the constructor function that is called automatically each time \SDKName
	needs to create an instance of a class that derives from BodyInfo. Each class that derives from
	BodyInfo must contain a constructor function that matches this type definition, and that passes its
	arguments on to the constructor function of its parent class.
	\param name					A unique name for the Body Info.
	\param bodyInfoManager		The BodyInfoManager that will manage the calculations of this BodyInfo.
	\param profile				The BodyProfile that owns this BodyInfo.
	\ingroup kyruntime_bodyinfomanagement */
typedef BodyInfo* (*BodyInfoConstructor)(const char* name, BodyInfoManager& bodyInfoManager, BodyProfile& profile);

/*! BodyInfoClass is the MetaClass for BodyInfo, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_bodyinfomanagement */
class BodyInfoClass : public MetaClass<BodyInfo, BodyInfoConstructor>
{
public:
	/*! Constructor for the BodyInfo MetaClass. For internal use. */
	BodyInfoClass(const char* className, BodyInfoConstructor ctr, BodyInfoClass* baseClass = KY_NULL);
};


/////////////////////////////////////////////////////////////////////////////////
// ConcreteBodyInfo
/////////////////////////////////////////////////////////////////////////////////

/*! This class adds a data storage layer to the BodyInfo base class. It stores the value of the
	BodyInfo and the date at which that value was last updated, for each "cell" (i.e. for each
	Body for which the BodyInfo is computed). This data is stored in the #m_data and #m_datesId
	arrays respectively.
	\tparam T			The type of data stored for each BodyInfo value (i.e. a Boolean for
						line-of-sight tests, a floating-point value for distance information, etc.).
	\pp You should never need to use this base class or any of its methods and members directly. Several BodyInfo
	classes that derive from ConcreteBodyInfo are supplied with the \SDKName SDK to calculate and
	maintain commonly used kinds of perception data, such as the line-of-sight to other \Bodies. If you
	need to write your own class of BodyInfo to represent a different type of perception, you must derive
	your class from either OneBodyInfo or TwoBodiesInfo.
	\pp For more information on \BodyInfos and the BodyInfoManager, see :USERGUIDE:"Tracking Perception Data".
	\ingroup kyruntime_bodyinfomanagement */
template <typename T>
class ConcreteBodyInfo: public BodyInfo
{
public:
	/*! Invalidates all dates stored in #m_datesId. */
	void ReInit();

	/*! Sets the current date to 'now' for the cell at the specified index within #m_datesId. */
	void Refresh(KyUInt32 index);

protected:
	/*! Computes the value of the cell at the specified index within #m_data. */
	virtual KyBool ComputeInfoCell(KyUInt32 index, KyBool& finished) = 0;

protected:
	/*! \label_constructor For internal use. */
	ConcreteBodyInfo(const char* name, BodyInfoManager& bodyInfoManager, BodyProfile& profile);

	/*! \label_virtualdestructor For internal use. */
	~ConcreteBodyInfo();

	/*! For internal use. */
	KyFloat32 GetDate(KyUInt32 index) const;

	/*! For internal use. */
	BodyInfo::ValidityStatus GetValue(KyUInt32 index,
										T& value,
										BodyInfo::ValidityStatus requestedStatus = BodyInfo::UNSAFE,
										BodyInfo::AccessMode accessMode = BodyInfo::RECOMPUTE_IF_TIME);

	/*! For internal use. */
	void MoveData(KyUInt32 indexFrom, KyUInt32 indexTo);

	/*! For internal use. */
	void Resize(KyUInt32 newSize);
protected:
	/*! Stores the BodyInfo value for each Body that has been assigned the BodyProfile that owns this BodyInfo. */
	Array<T> m_data;

	/*! Stores the last computation date of each value stored in #m_data. To minimize memory consumption, these
		values are indices (by default, four bits in size) that refer to dates stored in a circular list managed
		by the BodyInfoManager. */
	BitArray* m_datesId;
};


/////////////////////////////////////////////////////////////////////////////////
// OneBodyInfo
/////////////////////////////////////////////////////////////////////////////////

/*! This class represents an iterator used to navigate through the list of \Bodies contained in all
	\BodyProfiles that an BodyInfo is configured to take into account during its computations.
	For an example that demonstrates how to use this iterator, see the BodyInfo_NearestEntity class.
	\ingroup kyruntime_bodyinfomanagement */
class OneBodyInfoExplorationIterator
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	OneBodyInfoExplorationIterator(); /*!< For internal use. */
	OneBodyInfoExplorationIterator(const OneBodyInfoExplorationIterator& it) { *this = it; } /*!< For internal use. */
	OneBodyInfoExplorationIterator(Array<BodyProfile*>* bodyInfoProfiles); /*!< For internal use. */

	/*! Resets the OneBodyInfoExplorationIterator to the beginning of the list. */
	void Reset();

	/*! Retrieves a pointer to the Body referred to by the OneBodyInfoExplorationIterator, or #KY_NULL if the OneBodyInfoExplorationIterator does not
		point to a valid Body. */
	Body* operator *();

	/*! Copies the state of the specified OneBodyInfoExplorationIterator to this. */
	OneBodyInfoExplorationIterator& operator=(const OneBodyInfoExplorationIterator& it);

	/*! Increments the OneBodyInfoExplorationIterator to refer to the next Body in the list. */
	OneBodyInfoExplorationIterator& operator++();

	/*! Indicates whether or not the OneBodyInfoExplorationIterator refers to a valid profile. When the OneBodyInfoExplorationIterator passes the end of the list,
		this operator will return #KY_FALSE. */
	operator KyBool() const { return (m_currentProfile == KyUInt32MAXVAL ? (KyBool)KY_FALSE : (KyBool)KY_TRUE); }

	/*! Indicates whether or not the OneBodyInfoExplorationIterator has just been reset. This method returns #KY_TRUE after a call to Reset() only
		until the \c ++ operator is invoked. */
	KyBool WasReset() const { return m_wasReset; }

public:
	// info cell id:
	Array<BodyProfile*>* m_bodyInfoProfiles; /*!< A pointer to the BodyInfo that created this OneBodyInfoExplorationIterator. */
	KyUInt32 m_index; /*!< The index of the cell referred to by this OneBodyInfoExplorationIterator within the BodyInfo. */
	// current related body:
	KyUInt32 m_currentProfile; /*!< The BodyProfile of the Body currently referred to by this OneBodyInfoExplorationIterator. */
	KyUInt32 m_currentBody; /*!< The Body currently referred to by this OneBodyInfoExplorationIterator. */
	// is it the first body to be explored since the last reset ?
	KyBool m_wasReset; /*!< See WasReset(). */
};

/* Checks if the specified instance of OneBodyInfoExplorationIterator is affected by the cell at the specified index within
	the specified \BodyProfiles, and resets the iterator if necessary. For internal use. */
void OneBodyInfoExplorationIteratorReset(OneBodyInfoExplorationIterator& it, Array<BodyProfile*>& bodyInfoProfiles, KyUInt32 index);

/*! OneBodyInfo is the base class for \BodyInfos that store perceptual information related
	to a single Body: for example, the nearest object of a certain type, the nearest other Body,
	the distance from the Body to specific objects, etc.
	\tparam T			The type of value computed and stored by the BodyInfo. For example,
						\c Body* to store a pointer to a Body, KyFloat32 to store a floating-point
						value, KyBool to store a Boolean value, etc.
	\pp For more information on \BodyInfos and the BodyInfoManager, see :USERGUIDE:"Tracking Perception Data".
	\section usingonebodyinfo Setting up a OneBodyInfo class
	All OneBodyInfo classes supplied with \SDKName search through the \Bodies contained in another set of \BodyProfiles, maintained
	in the #m_exploredProfiles list. If you use any of these ready-to-use OneBodyInfo classes, you must call AddExploredProfile() to
	add other \BodyProfiles to the #m_exploredProfiles list. If you write a custom OneBodyInfo class, you can use this list or not,
	depending on the type of data you need to compute. See the #m_exploredProfiles list.
	\section writingcustomonebodyinfo Writing a custom OneBodyInfo class
	Writing a custom class that derives from OneBodyInfo involves the following considerations.
	\subsection onebodyinfoconst Constructor
	All classes that derive from OneBodyInfo must have a constructor that follows the #BodyInfoConstructor type definition.
	\subsection onebodyinfovm Virtual methods
	All classes that derive from OneBodyInfo must implement the ComputeInfo() method.
	\subsection onebodyinfometaclass MetaClass macros
	In order for \SDKName to recognize your BodyInfo class, you must include the following macro definitions:
	-	#KY_DECLARE_BODYINFO: Place this macro in the declaration of your new class. This macro takes one argument: the
		name of your new class.
	-	#KY_IMPLEMENT_BODYINFO: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from BodyInfo.
			For example, &Kaim::OneBodyInfo::Class. If your new class derives directly from BodyInfo,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_bodyinfomanagement */
template <typename T>
class OneBodyInfo : public ConcreteBodyInfo<T>
{
public:
	/*! \label_baseconstructor
		\param name					A unique name for the BodyInfo.
		\param bodyInfoManager		The BodyInfoManager that will manage the calculations of this BodyInfo.
		\param profile				The BodyProfile that owns this BodyInfo. */
	OneBodyInfo(const char* name, BodyInfoManager& bodyInfoManager, BodyProfile& profile)
		: ConcreteBodyInfo<T>(name, bodyInfoManager, profile)
		,m_infoIt(0)
		,m_dateIt(0)
	{}



	/*! Retrieves the value of the BodyInfo stored for the specified Body. Do not call this method for
		any \Bodies for which this BodyInfo has not been configured to calculate and maintain values. See IsManaged().
		\param body					The Body whose data should be retrieved.
		\param[out] value			Stores the retrieved value.
		\param requestedStatus		The requested level of validity. May be any element from the
									BodyInfo::ValidityStatus enumeration.
		\param accessMode			Determines the behavior of this method when the status of the BodyInfo
									is older than the status identified in the \c requestedStatus
									parameter. This value may be any element from the BodyInfo::AccessMode
									enumeration.
		\return The validity status of the BodyInfo value written to the \c value parameter. See the
				BodyInfo::ValidityStatus enumeration for details. */
	BodyInfo::ValidityStatus GetValue(
		const Body& body, T& value,
		BodyInfo::ValidityStatus requestedStatus = BodyInfo::UNSAFE,
		BodyInfo::AccessMode accessMode = BodyInfo::RECOMPUTE_IF_TIME);

	/*! Sets the value of the BodyInfo recorded for the specified Body.
		\param body					The Body whose value is to be set.
		\param value				The value of the BodyInfo to set for the Body. */
	void SetValue(const Body& body, const T& value);

	/*! Returns the date at which the BodyInfo was last updated for the specified
		Body, or #KyFloat32MAXVAL to indicate that the BodyInfo is invalid. */
	KyFloat32 GetDate(const Body& body);

	/*! Indicates whether or not this BodyInfo is calculated for the specified Body, by testing
		whether or not that Body has been assigned the same BodyProfile that owns this BodyInfo.
		If this method returns #KY_TRUE, it is safe to call GetValue() for that Body. */
	KyBool IsManaged(const Body& body) const;

	/*! Returns the #m_exploredProfiles list. */
	const Array<BodyProfile*>& GetExploredProfiles() const { return m_exploredProfiles; }

	/*! Adds the specified BodyProfile to the #m_exploredProfiles list. */
	void AddExploredProfile(BodyProfile* profile);

protected:
	/*! This method must be implemented in any class that derives from OneBodyInfo. It computes
		the value of the BodyInfo for the specified Body, and stores the computed value in the
		\c value parameter.
		\param body					Specifies the Body for which the BodyInfo must be calculated.
		\param[out] value			Used to store the calculated value.
		\param[out] finished		Indicates whether or not the calculation was completed successfully.
									If this calculation is being carried out in response to a call to GetValue(),
									this method will be called repeatedly until this parameter is set to #KY_TRUE,
									or until the CPU time allotted to the BodyInfoManager in the current frame is
									exhausted. See the \c accessMode parameter of the GetValue() method.
		\return #KY_TRUE if the value was successfully computed and stored in the \c value parameter, or
				#KY_FALSE otherwise. */
	virtual KyBool ComputeInfo(const Body& body, T& value, KyBool& finished) = 0;

protected:

	virtual KyBool Update(KyUInt32& count);
	virtual KyBool UpdateDates(KyUInt32& infoCount);
	virtual KyBool ComputeInfoCell(KyUInt32 index, KyBool& finished);
	virtual KyUInt32 GetCellCount() const;
	virtual void OnAddBody(const Body& body);
	virtual void OnRemoveBody(const Body& body);
	virtual void Invalidate(const Body& body);
protected:
	KyUInt32 m_infoIt; /*!< This iterator identifies the next Body whose data needs to be updated. */
	KyUInt32 m_dateIt; /*!< This iterator identifies the next date that needs to be updated. */

	/*! An iterator that navigates through the #m_exploredProfiles list. */
	OneBodyInfoExplorationIterator m_explorationIterator;

	/*! A list of other \BodyProfiles that this BodyInfo can take into account when computing its values.
		For example, the BodyInfo_Nearest class is intended to find and record the nearest other Body to each Body
		that has been given the BodyProfile that owns this BodyInfo. However, it restricts the search to only include
		other \Bodies that have been assigned one of the \BodyProfiles in the #m_exploredProfiles list.
		\pp You add \BodyProfiles to this list by calling AddExploredProfile().
		\pp Your custom BodyInfo classes can navigate through the \Bodies in this list using #m_explorationIterator. You
		do not have to use this list at all if you do not need to examine another set of \Bodies to calculate values for
		your custom type of BodyInfo. */
	Array<BodyProfile*> m_exploredProfiles;
};

extern BodyInfoClass OneBodyInfoClass; /*!< The MetaClass for OneBodyInfo. \ingroup kyruntime_bodyinfomanagement */


template <typename T>
class TwoBodiesInfoIterator;

/////////////////////////////////////////////////////////////////////////////////
// TwoBodiesInfo
/////////////////////////////////////////////////////////////////////////////////

/*! TwoBodiesInfo is the base class for \BodyInfos that store perceptual information related
	to the relationship between two \Bodies: for example, the distance between two \Bodies, whether
	or not a line of sight exists between them, etc.
	\tparam T					The type of value computed and stored by the BodyInfo. For example,
								KyFloat32 to store a floating-point value, KyBool to store a Boolean value, etc.
	\pp For more information on \BodyInfos and the BodyInfoManager, see :USERGUIDE:"Tracking Perception Data".
	\section usingtwobodyinfo Setting up a TwoBodiesInfo class
	After you activate a TwoBodiesInfo class for a BodyProfile by calling BodyInfoManager::ActivateBodyInfoToProfile(), you
	must identify each other BodyProfile that contains \Bodies you want the TwoBodiesInfo to be calculated for by calling
	AddInputProfile().
	\pp For example, suppose that you want to use the BodyInfo_Visibility class to calculate the visibility between \Bodies that have
	been assigned the BodyProfile named "GoodGuys" and \Bodies that have been assigned the BodyProfile named "BadGuys". You must
	pass one of those \BodyProfiles in the call to BodyInfoManager::ActivateBodyInfoToProfile(). That BodyProfile will own the newly
	created BodyInfo. You then have to call AddInputProfile() and pass the other BodyProfile in order to identify the \Bodies that the
	BodyInfo should be computed for.
	\section writingcustomtwobodyinfo Writing a custom TwoBodiesInfo class
	Writing a custom class that derives from OneBodyInfo involves the following considerations.
	\subsection twobodyinfoconst Constructor
	All classes that derive from TwoBodiesInfo must have a constructor that follows the #BodyInfoConstructor type definition.
	\subsection twobodyinfovm Virtual methods
	All classes that derive from TwoBodiesInfo must implement the ComputeInfo() method.
	\subsection twobodyinfometaclass MetaClass macros
	In order for \SDKName to recognize your TwoBodiesInfo class, you must include the following macro definitions:
	-	#KY_DECLARE_BODYINFO: Place this macro in the declaration of your new class. This macro takes one argument: the
		name of your new class.
	-	#KY_IMPLEMENT_BODYINFO: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from BodyInfo.
			For example, &Kaim::TwoBodiesInfo::Class. If your new class derives directly from BodyInfo,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_bodyinfomanagement */
template <typename T>
class TwoBodiesInfo : public ConcreteBodyInfo<T>
{
public:
	/*! \label_baseconstructor
		\param name					A unique name for the BodyInfo.
		\param bodyInfoManager		The BodyInfoManager that will manage the calculations of this BodyInfo.
		\param profile				The BodyProfile that owns this BodyInfo. */
	TwoBodiesInfo(const char* name, BodyInfoManager& bodyInfoManager, BodyProfile& profile);



	/*! Retrieves the value of the BodyInfo stored for the specified pair of \Bodies. Do not call this method for
		any pair of \Bodies for which this BodyInfo has not been configured to calculate and maintain values.
		See IsManaged().
		\param body1				The Body whose data should be retrieved.
		\param body2				The other Body in the pair.
		\param[out] value			Stores the retrieved value.
		\param requestedStatus		The requested level of validity. May be any element from the
									BodyInfo::ValidityStatus enumeration.
		\param accessMode			Determines the behavior of this method when the status of the BodyInfo
									is older than the status identified in the \c requestedStatus
									parameter. This value may be any element from the BodyInfo::AccessMode
									enumeration.
		\return The validity status of the BodyInfo value written to the \c value parameter. See the
				BodyInfo::ValidityStatus enumeration for details. */
	BodyInfo::ValidityStatus GetValue(
		const Body& body1, const Body& body2, T& value,
		BodyInfo::ValidityStatus requestedStatus = BodyInfo::UNSAFE,
		BodyInfo::AccessMode accessMode = BodyInfo::RECOMPUTE_IF_TIME);

	/*! Sets the value of the BodyInfo recorded for the specified pair of \Bodies.
		\param body1				The Body whose value is to be set.
		\param body2				The other Body in the pair.
		\param value				The value of the BodyInfo to set for the Body. */
	void SetValue(const Body& body1, const Body& body2, const T& value);

	/*! Returns the date at which the BodyInfo was last updated for the specified
		pair of \Bodies, or #KyFloat32MAXVAL to indicate that the BodyInfo is invalid. */
	KyFloat32 GetDate(const Body& body1, const Body& body2);

	/*! Indicates whether or not this BodyInfo is calculated for the specified pair of
		\Bodies. If this method returns #KY_TRUE, it is safe to call GetValue() for that pair
		of \Bodies. */
	KyBool IsManaged(const Body& body1, const Body& body2) const;

	/*! Returns the index within the #m_data array at which the data for the specified pair
		of \Bodies is recorded. If this method returns #KyUInt32MAXVAL, then one of both of the
		specified \Bodies is not managed by this BodyInfo. */
	KyUInt32 GetIndex(const Body& body1, const Body& body2) const;

	/*! Returns the #m_inputProfiles list. */
	const Array<BodyProfile*>& GetInputProfiles() const { return m_inputProfiles; }

	/*! Adds a new BodyProfile to the #m_inputProfiles list. */
	void AddInputProfile(BodyProfile* inputProfile);

	/*! Returns the maximum number of \Bodies that can be assigned to all other \BodyProfiles
		this BodyInfo is computed with. */
	KyUInt32 GetInputBodyMax() const { return m_inputBodyMax; }

protected:
	/*! This method must be implemented in any class that derives from TwoBodiesInfo. It computes
		the value of the BodyInfo for the specified pair of \Bodies, and stores the computed value in the
		\c value parameter.
		\param body1				Specifies the Body for which the BodyInfo must be calculated.
		\param body2				Specifies the other Body in the pair.
		\param[out] value			Used to store the calculated value.
		\param[out] finished		Indicates whether or not the calculation was completed successfully.
									If this calculation is being carried out in response to a call to GetValue(),
									this method will be called repeatedly until this parameter is set to #KY_TRUE,
									or until the CPU time allotted to the BodyInfoManager in the current frame is
									exhausted. See the \c accessMode parameter of the GetValue() method.
		\return #KY_TRUE if the value was successfully computed and stored in the \c value parameter,
				or #KY_FALSE otherwise. */
	virtual KyBool ComputeInfo(const Body& body1, const Body& body2, T& value, KyBool& finished) = 0;

protected:
	KyFloat32 GetDate(KyUInt32 index1, KyUInt32 index2);
	virtual KyBool Update(KyUInt32& count);
	virtual KyBool UpdateDates(KyUInt32& infoCount);
	virtual KyBool ComputeInfoCell(KyUInt32 index, KyBool& finished);
	KyBool ComputeInfoCell(Body& body1, Body& body2, KyUInt32 index, KyBool& finished);
	KyBool GetBodiesFromIndex(KyUInt32 index, Body*& body1, Body*& body2);
	virtual KyUInt32 GetCellCount() const;
	virtual void OnAddBody(const Body& body);
	virtual void OnRemoveBody(const Body& body);
	virtual void Invalidate(const Body& body);

protected:
	/*! For internal use. */
	KyUInt32 m_inputBodyMax;

	/*! A list of the other \BodyProfiles that this BodyInfo will take into account when computing its values.
		For example, the BodyInfo_Visibility class is intended to find and record the visibility between each Body
		that has been given the BodyProfile that owns this BodyInfo, and each Body that has been given one of the
		\BodyProfiles in the #m_inputProfiles list.
		\pp You add \BodyProfiles to this list by calling AddInputProfile(). */
	Array<BodyProfile*> m_inputProfiles;

	TwoBodiesInfoIterator<T> m_infoIt; /*!< This iterator identifies the next Body whose data needs to be updated. */
	KyUInt32 m_dateIt; /*!< This iterator identifies the next date that needs to be updated. */
};

extern BodyInfoClass TwoBodyInfoClass; /*!< The MetaClass for TwoBodiesInfo. \ingroup kyruntime_bodyinfomanagement */

/*! This class represents an iterator that navigates through the list of cells managed by
	classes that derive from TwoBodiesInfo. This iterator is very similar to
	the OneBodyInfoExplorationIterator used for OneBodyInfo classes, but it manages a
	two-dimensional list that refers to pairs of \Bodies instead of a one-dimensional list
	that refers to single \Bodies.
	\ingroup kyruntime_bodyinfomanagement */
template <typename T>
class TwoBodiesInfoIterator
{
	KY_DEFINE_NEW_DELETE_OPERATORS
	friend class TwoBodiesInfo<T>;
public:
	inline TwoBodiesInfoIterator(); /*! For internal use. */

	/*! Increments the TwoBodiesInfoIterator to refer to the next item in the list. */
	void IncrementLineIndex();

	/*! Increments the TwoBodiesInfoIterator to refer to the next item in the list. */
	void IncrementColumnIndex();

	void Increment(KyBool columnOrLine)
	{
		if (columnOrLine)
		{
			IncrementColumnIndex();
		}
		else
		{
			IncrementLineIndex();
		}
	}

	/*! Indicates whether or not the TwoBodiesInfoIterator refers to a valid item. When the TwoBodiesInfoIterator passes the end of the list,
		this operator will return #KY_FALSE. */
	inline bool IsValid() const { if (m_finished) return false; return true; }

	/*! Compares two instances of TwoBodiesInfoIterator, and returns #KY_TRUE if both point to the same item. */
	inline bool operator==(const TwoBodiesInfoIterator& it);

	/*! Compares two instances of TwoBodiesInfoIterator, and returns #KY_TRUE if they point to different items. */
	inline bool operator!=(const TwoBodiesInfoIterator& it);

	/*! Resets the TwoBodiesInfoIterator to the beginning of the list. */
	inline void Reset();

	/*! Retrieves the pair of \Bodies currently referred to by the TwoBodiesInfoIterator. */
	inline KyBool GetBodies(Body*& body1, Body*& body2) const;

	/*! Retrieves the index of the current cell referred to by this TwoBodiesInfoIterator within the BodyInfo. */
	inline KyUInt32 GetIndex() const { return m_index; }

private:
	KyUInt32 m_curEnt1It;
	KyUInt32 m_curProfileId;
	KyUInt32 m_colOffset;
	KyUInt32 m_lineOffset;
	KyUInt32 m_curEnt2It;
	KyBool m_finished;
	KyUInt32 m_index;
	TwoBodiesInfo<T>* m_info;
};


} // namespace Kaim


#define KY_REGISTER_BODYINFO(className, baseClassName, templateType)\
typedef templateType DummyType_##className##_templateType;\
class DummyRegisterClassFor_##className##_##baseClassName##_##__FILE__##_##__LINE__##End : public baseClassName<templateType>\
{\
public:\
	DummyRegisterClassFor_##className##_##baseClassName##_##__FILE__##_##__LINE__##End\
		(const char* name, Kaim::BodyInfoManager& bodyInfoManager,\
		Kaim::BodyProfile& profile)\
		:baseClassName<templateType>(name, bodyInfoManager, profile)\
	{\
	}\
};

#endif // KY_BODYINFO_H
