/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_core */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_TIMEMANAGER_H
#define KY_TIMEMANAGER_H

#include <kypathdata/generic/macros.h>
#include <kypathdata/generic/memory.h>
#include <kypathdata/containers/array.h>
#include "kypathdata/basesystem/config.h"


namespace Kaim
{

class World;
class Team;
class Bot;
class Engine;

/*! This class manages the CPU time consumed each frame by the components of a World. 
	It maintains lists of periodic and aperiodic tasks, which allow you to defer time-consuming
	calculations when no time remains in the current frame.
	Each World creates and maintains one instance of this class, accessible through World::GetTimeManager().
	\pp For more information, see :USERGUIDE:"Deferring and Time-Slicing Tasks".
	\ingroup kyruntime_core */
class TimeManager
{
	KY_DEFINE_NEW_DELETE_OPERATORS

	friend class World;

	/*! \label_constructor No implementation. For internal use only. */
	TimeManager(const TimeManager&);

	/*! No implementation. For internal use only. */
	const TimeManager& operator=(const TimeManager&);

public:
	/*!	Type definition for the unique ID of an aperiodic or periodic task. */
	typedef KyUInt32 TaskId;

	/*!	The unique ID that identifies an aperiodic or periodic task that is not recognized by the TimeManager. */
	static const TaskId InvalidTaskId = KyUInt32MAXVAL;

	/*!	Used internally by the TimeManager to represent a managed Bot. You should not have to
		interact with this class directly. */
	class BotTimerInfo
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
	public:
		BotTimerInfo();
		~BotTimerInfo();

		void Clear();
		void Initialize(TimeManager&);
		void ReInit(const TimeManager&);

	public:
		KyFloat32* m_priorities;
		KyFloat32 m_priority; /*!< The absolute priority of the Bot. */
		KyFloat32 m_priorityFactor;
		KyFloat32* m_lastAperiodicTaskUse;
		KyFloat32* m_lastPeriodicTaskUse;
		KyFloat32* m_lastAperiodicTaskRequest;
		mutable Array<KyBool> m_isAperiodicTaskRunning; //< bit is set to true when we 'do' the task (between StartAperiodicTask and ProcessFeedback) (only updated when KYDEBUG is defined).
	};

	/*!	Used internally by the TimeManager to represent a managed Team. You should not have to
		interact with this class directly. */
	class TeamTimerInfo
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
	public:
		TeamTimerInfo() : m_lastAperiodicTaskUse(KY_NULL), m_lastPeriodicTaskUse(KY_NULL) {}
		~TeamTimerInfo();
		void Init(TimeManager&);
		void ReInit(const TimeManager& timeManager);
	public:
		KyFloat32* m_lastAperiodicTaskUse;
		KyFloat32* m_lastPeriodicTaskUse;
		mutable Array<KyBool> m_isAperiodicTaskRunning; //< bit is set to true when we 'do' the task (between StartAperiodicTask and ProcessFeedback).
	};

public:

	/*!	Registers a new periodic task for the TimeManager.
		\param name			A unique name for the new task. */
	TaskId RegisterPeriodicTask(const char* name);
	
	/*!	Registers a new aperiodic task for the TimeManager.
		\param name			A unique name for the new task. */
	TaskId RegisterAperiodicTask(const char* name);

	/*!	Retrieves the unique name of the aperiodic task with the specified numeric ID. */
	const char* GetAperiodicTaskName(TaskId i) const { return m_aperiodicTasks[i].m_aperiodicTaskNames; }

	/*!	\label_getworld */
	World* GetWorld() const
	{
		return m_world;
	}

	/*!	\label_getengine */
	Engine& GetEngine();

	/*!	\label_getengine */
	const Engine& GetEngine() const;

	/////////////////////////////////////////////////////////////////////////////////////////////
	// Init / construction / destruction / accessors for parameters
	/////////////////////////////////////////////////////////////////////////////////////////////

	/*!	\label_reinit For internal use. */
	void ReInit();

	/*!	Sets the maximum allotted time per frame for the World. */
	KyResult SetAverageTimePerFrame(KyFloat32 seconds);

	/*!	Retrieves the maximum allotted time per frame for the World. */
	KyFloat32 GetAverageTimePerFrame() const;

	/*!	Sets the maximum time per frame that can be spent performing the specified aperiodic task. By default, this
		value is unlimited, allowing the task to use the entire time per frame allotted to your World set in the call
		to World::Initialize(). Set this value between \c 0 and the maximum time per frame allotted to your World. 
		If you set a value in excess of the time per frame allotted for the World, the World-level setting takes precedence.
		\param taskId		The unique numeric ID of the aperiodic task. See GetAperiodicTaskByName().
		\param seconds		The maximum time per frame for the task, in seconds. */
	void SetAverageTaskTimePerFrame(TaskId taskId, KyFloat32 seconds);

	/*!	Retrieves the value set in a call to SetAverageTaskTimePerFrame() for the specified aperiodic task. */
	KyFloat32 GetAverageTaskTimePerFrame(TaskId taskId) const;

	/*!	Determines the maximum number of times the specified aperiodic task may be performed within a single frame.
		By default, this parameter has no limit, allowing the task to be called as many times as necessary each frame.
		\param taskId		The unique numeric ID of the aperiodic task. See GetAperiodicTaskByName().
		\param nbMax		The maximum number of times the task can be called in a single frame. */
	void SetMaxTaskCallPerFrame(TaskId taskId, KyInt32 nbMax);

	/*!	Retrieves the value set in a call to SetMaxTaskCallPerFrame() for the specified aperiodic task. */
	KyInt32 GetMaxTaskCallPerFrame(TaskId taskId) const;

	/*!	Determines the boost in priority that should be applied to each Bot that requests the specified aperiodic task
		and is turned down by the TimeManager. Having a higher priority makes the denied \Bots more likely to be given
		access to the aperiodic task in the following frame. The default value is 1. For details, see 
		:USERGUIDE:"Managing Bot Priorities". 
		\param taskId		The unique numeric ID of the aperiodic task. See GetAperiodicTaskByName().
		\param priority		The priority factor to set for the task. */
	void SetTaskPriority(TaskId taskId, KyFloat32 priority);

	/*!	Retrieves the value set in a call to SetTaskPriority() for the specified aperiodic task. */
	KyFloat32 GetTaskPriority(TaskId taskId) const;

	/*!	Determines the frequency at which the specified periodic task is carried out.
		\param taskId		The unique numeric ID of the periodic task. See GetAperiodicTaskByName().
		\param period		The frequency at which the task is carried out, in milliseconds. */
	void SetTaskPeriodicity(TaskId taskId, KyFloat32 period);

	/*!	Retrieves the value set in a call to SetTaskPeriodicity() for the specified aperiodic task. */
	KyFloat32 GetTaskPeriodicity(TaskId taskId) const;


	/*!	Retrieves the unique numeric ID of the aperiodic task with the specified name.
		\param name			The unique name of the aperiodic task. See GetAperiodicTaskByName().
		\return The integer ID of the task whose name matches \c name, or -1 if no matching task was found. */
	TaskId GetAperiodicTaskByName(const char* name);

	/*!	Retrieves the unique numeric ID of the periodic task with the specified name.
		\param name			The unique name of the periodic task. See GetAperiodicTaskByName().
		\return The integer ID of the task whose name matches \c name, or -1 if no matching task was found. */
	TaskId GetPeriodicTaskByName(const char* name);

	/////////////////////////////////////////////////////////////////////////////////////////////
	// Various accessor functions used to get consumption feedback on aperiodic tasks
	/////////////////////////////////////////////////////////////////////////////////////////////

	/*!	Retrieves the number of aperiodic tasks configured for the World.*/
	const KyUInt32& GetAperiodicTaskCount() const { return m_aperiodicCount; }

	/*!	Retrieves the CPU time consumed by the specified aperiodic task since the start of the current frame.
		\param taskId		The unique numeric ID of the aperiodic task. See GetAperiodicTaskByName().
		\return The time in seconds consumed by the task whose ID matches \c taskId, or -1.0f if no matching
				task could be found. */
	KyFloat32 GetFrameTimeConsumptionForTask(const TaskId taskId) const;

	/*!	Retrieves the number of times the specified aperiodic task has been called since the beginning of the
		current frame.
		\param taskId		The unique numeric ID of the aperiodic task. See GetAperiodicTaskByName().
		\return The number of calls to the task whose ID matches \c taskId, or #KyUInt32MAXVAL if no matching
				task could be found. */
	KyUInt32 GetFrameNbCallsForTask(const TaskId taskId) const;

	/////////////////////////////////////////////////////////////////////////////////////////////
	// Various runtime functions used in the update loop
	/////////////////////////////////////////////////////////////////////////////////////////////

	/*!	Retrieves the date at which the current frame began. */
	inline KyFloat32 GetCurrentFrameStart() const { return m_currentFrameStart; }

	/*!	Retrieves the date at which the previous frame began. */
	inline KyFloat32 GetLastFrameStart() const { return m_lastFrameStart; }

	/*!	Retrieves the \c deltaTime value specified in the last call to World::Update(). */
	inline KyFloat32 GetFrameDeltaTime() const { return m_frameDeltaTime; }

	/*!	Returns the time spent during the last update of the World. */
	inline KyFloat32 GetLastElapsedTime() const { return m_lastElapsedTime; }

	/*!	Retrieves the time spent in asynchronous calculations during the last asynchronous phase. */
	KyFloat32 GetLastAsyncElapsedTime() const;

	/*!	Indicates whether or not the global CPU time allotted to the World has elapsed
		in the current frame. When this method returns #KY_TRUE, the TimeManager will deny
		all further requests for aperiodic tasks within this World. */
	inline KyBool NoMoreTime() const { return m_noMoreTime; }

	/*!	Updates the value returned by NoMoreTime(). */
	void UpdateNoMoreTime();

	/*!	Retrieves the amount of CPU time remaining in the current frame for the execution of
		the specified aperiodic task.
		\param taskId			The unique numeric ID of the aperiodic task. See GetAperiodicTaskByName().
		\return The time left, in seconds, for the task whose ID matches \c taskId, or 0 if no matching
				task could be found. */
	KyFloat32 GetAvailableTime(TaskId taskId) const;

	/*!	Compares the priority of the two specified \Bots.
		\param timeManager		The TimeManager of the World that contains both \Bots to compare.
		\param arg1				The first Bot to compare.
		\param arg2				The second Bot to compare.
		\return -	-1 if the first Bot has a higher priority than the second.
				-	0 if both \Bots have the same priority.
				-	1 if the second Bot has a higher priority than the first. */
	static KyInt32 BotCompare(const TimeManager& timeManager, const Bot* arg1, const Bot* arg2);

	/*!	For internal use. */
	void BeginFrame(KyFloat32 deltaTime = 0.0167f);

	/*!	For internal use. */
	Bot* GetNextBotId();

	/*!	For internal use. */
	void SetPrioritaryBot(const Bot& bot);

	/*!	For internal use. */
	void EndFrame();

	/////////////////////////////////////////////////////////////////////////////////////////////
	// RequestPeriodic
	/////////////////////////////////////////////////////////////////////////////////////////////

	/*!	Requests authorization for the specified Bot to perform the specified
		periodic task. If the elapsed time since the last time that Bot performed
		that task is greater than the frequency of the task set through the last call
		to SetTaskPeriodicity(), authorization will be granted. If the task has been run
		more recently that that, authorization will be denied.
		\param taskId			The unique numeric ID of the periodic task. See GetAperiodicTaskByName().
		\param bot				The Bot performing the periodic task.
		\return #KY_TRUE if \c bot is authorized to perform the periodic task, or #KY_FALSE
				otherwise. */
	KyBool RequestPeriodic(TaskId taskId, Bot& bot);

	/*!	Requests authorization for the specified Team to perform the specified
		periodic task. If the elapsed time since the last time that Team performed
		that task is greater than the frequency of the task set through the last call
		to SetTaskPeriodicity(), authorization will be granted. If the task has been run
		more recently that that, authorization will be denied.
		\param taskId			The unique numeric ID of the periodic task. See GetAperiodicTaskByName().
		\param team				The Team performing the periodic task.
		\return #KY_TRUE if \c team is authorized to perform the periodic task, or #KY_FALSE
				otherwise. */
	KyBool RequestPeriodic(TaskId taskId, Team& team);

	/*!	Sets the time at which the specified Bot last performed the specified periodic
		task to the current time. If you need to run a periodic task immediately in your code, 
		without requesting authorization or waiting for the period to expire, you can call 
		this method immediately after running the task in order to notify the TimeManager 
		that the task has been performed. The period will be re-started from the current time.
		\param taskId			The unique numeric ID of the periodic task. See GetAperiodicTaskByName().
		\param bot				The Bot performing the periodic task. */
	void RecordPeriodicTaskUse(TaskId taskId, Bot& bot);

	/*!	Sets the time at which the specified Team last performed the specified periodic
		task to the current time. If you need to run a periodic task immediately in your code, 
		without requesting authorization or waiting for the period to expire, you can call 
		this method immediately after running the task in order to notify the TimeManager 
		that the task has been performed. The period will be re-started from the current time.
		\param taskId			The unique numeric ID of the periodic task. See GetAperiodicTaskByName().
		\param team				The Team performing the periodic task. */
	void RecordPeriodicTaskUse(TaskId taskId, Team& team);

	/////////////////////////////////////////////////////////////////////////////////////////////
	// RequestAperiodic
	/////////////////////////////////////////////////////////////////////////////////////////////

	/*!	Requests CPU time for the specified Bot or Team to run the specified aperiodic task,
		and updates the priority of the requester accordingly.
		\pp If this method returns #KY_TRUE, your code should perform the aperiodic task immediately. 
		When the task has been completed, you must call ProcessFeedback() in order to inform the
		TimeManager that the task has been finished. The TimeManager will use the elapsed time
		since the request was granted in order to manage the time taken by that task in the
		current frame. The priority of the requester for the task is reset automatically.
		\pp If this method returns #KY_FALSE, your code should not perform the aperiodic task. The
		priority of the requester for the task is increased automatically. This causes the requester
		to be updated earlier in the following frame, which results in a better chance for the
		requester to receive CPU time for the task.
		\pp This method calls several more granular methods of the TimeManager interface. If it is more
		convenient, you can call these methods directly from your own code. Use the inline implementation
		of RequestAperiodic() as a model.
		\param taskId			The unique numeric ID of the aperiodic task. See GetAperiodicTaskByName().
		\param requester		The Bot or Team requesting authorization for the aperiodic task.
		\return #KY_TRUE if \c requester may perform the aperiodic task, or #KY_FALSE otherwise. */
	template <class Requester>
	KyBool RequestAperiodic(TaskId taskId, Requester& requester)
	{
		KeepAperiodicTaskAlive(taskId, requester);
		if (GrantAperiodicTask(taskId, requester))
		{
			ResetPriority(taskId, requester);
			StartAperiodicTask(taskId, requester);
			return KY_TRUE;
		}
		else
		{
			IncreasePriority(taskId, requester);
			return KY_FALSE;
		}
	}

	/*!	Requests CPU time to run the specified aperiodic task, without managing the priority
		of the requester.
		\pp If this method returns #KY_TRUE, your code should perform the aperiodic task immediately. 
		When the task has been completed, you must call ProcessFeedback() in order to inform the
		TimeManager that the task has been finished. The TimeManager will use the elapsed time
		since the request was granted in order to manage the time taken by that task in the
		current frame.
		\pp If this method returns #KY_FALSE, your code should not perform the aperiodic task. 
		\pp This method calls several more granular methods of the TimeManager interface. If it is more
		convenient, you can call these methods directly from your own code. Use the inline implementation
		of RequestAperiodic() as a model.
		\param taskId			The unique numeric ID of the aperiodic task. See GetAperiodicTaskByName().
		\return #KY_TRUE if the aperiodic task may be performed, or #KY_FALSE otherwise. */
	KyBool RequestAperiodic(TaskId taskId)
	{
		if (GrantAperiodicTask(taskId) == KY_FALSE)
			return KY_FALSE;
		StartAperiodicTask(taskId);
		return KY_TRUE;
	}

	/*!	Indicates whether or not the specified Bot may perform the specified aperiodic task
		in the current frame. Authorization is given if all the following conditions are true:
		-	The global time per frame allotted for the World, set in the call to World::Initialize(), 
			has not been exceeded.
		-	The time per frame allotted for the aperiodic task, set by the last call to SetAverageTaskTimePerFrame()
			for that task, has not been exceeded.
		-	The number of times the aperiodic task has been granted in the current frame is less
			than the maximum number of calls set by the last call to SetMaxTaskCallPerFrame() for the task.
		\param taskId				The unique numeric ID of the aperiodic task. See GetAperiodicTaskByName().
		\param bot					The Bot requesting authorization for the aperiodic task.
		\return #KY_TRUE if authorization is granted for \c bot to perform the aperiodic task, or
				#KY_FALSE otherwise. */
	KyBool GrantAperiodicTask(TaskId taskId, const Bot& bot);

	/*!	Indicates whether or not the specified Team may perform the specified aperiodic task
		in the current frame. Authorization is given if all the following conditions are true:
		-	The global time per frame allotted for the World, set in the call to World::Initialize(), 
			has not been exceeded.
		-	The time per frame allotted for the aperiodic task, set by the last call to SetAverageTaskTimePerFrame()
			for that task, has not been exceeded.
		-	The number of times the aperiodic task has been granted in the current frame is less
			than the maximum number of calls set by the last call to SetMaxTaskCallPerFrame() for the task.
		\param taskId				The unique numeric ID of the aperiodic task. See GetAperiodicTaskByName().
		\param team					The Team requesting authorization for the aperiodic task.
		\return #KY_TRUE if authorization is granted for \c team to perform the aperiodic task, or
				#KY_FALSE otherwise. */
	KyBool GrantAperiodicTask(TaskId taskId, const Team& team);

	/*!	Indicates whether or not the specified aperiodic task may be performed
		in the current frame. Authorization is given if all the following conditions are true:
		-	The global time per frame allotted for the World, set in the call to World::Initialize(), 
			has not been exceeded.
		-	The time per frame allotted for the aperiodic task, set by the last call to SetAverageTaskTimePerFrame()
			for that task, has not been exceeded.
		-	The number of times the aperiodic task has been granted in the current frame is less
			than the maximum number of calls set by the last call to SetMaxTaskCallPerFrame() for the task.
		\param taskId				The unique numeric ID of the aperiodic task. See GetAperiodicTaskByName().
		\return #KY_TRUE if authorization is granted for the aperiodic task to be performed, or
				#KY_FALSE otherwise. */
	KyBool GrantAperiodicTask(TaskId taskId);

	/*!	Forbids the TimeManager to authorize the specified aperiodic task for the rest of the 
		current frame.
		\param taskId				The unique numeric ID of the aperiodic task. See GetAperiodicTaskByName(). */
	void ForbidAperiodicTask(TaskId taskId);

	/*!	Updates the last time at which the specified Bot requested the specified aperiodic task.
		\param taskId				The unique numeric ID of the aperiodic task. See GetAperiodicTaskByName().
		\param bot					The Bot requesting the aperiodic task. */
	void KeepAperiodicTaskAlive(TaskId taskId, Bot& bot);

	/*!	Updates the last time at which the specified Team requested the specified aperiodic task.
		\param taskId				The unique numeric ID of the aperiodic task. See GetAperiodicTaskByName().
		\param team					The Team requesting the aperiodic task. */
	void KeepAperiodicTaskAlive(TaskId taskId, Team& team);

	/*!	Resets the priority of the specified Bot for the specified aperiodic task.
		\param aperiodicTaskId		The unique numeric ID of the aperiodic task. See GetAperiodicTaskByName().
		\param bot					The Bot whose priority will be reset. */
	void ResetPriority(TaskId aperiodicTaskId, Bot& bot);

	/*!	Resets the priority of the specified Team for the specified aperiodic task.
		\param aperiodicTaskId		The unique numeric ID of the aperiodic task. See GetAperiodicTaskByName().
		\param team					The Team whose priority will be reset. */
	void ResetPriority(TaskId aperiodicTaskId, Team& team);

	/*!	Indicates that the specified Bot is about to begin the specified aperiodic task, to set
		the starting time for the task. The TimeManager considers all the time that passes between 
		the call to StartAperiodicTask() and the next call to ProcessFeedback() as time spent
		performing the aperiodic task.
		\param taskId				The unique numeric ID of the aperiodic task. See GetAperiodicTaskByName().
		\param bot					The Bot starting the aperiodic task. */
	void StartAperiodicTask(TaskId taskId, Bot& bot);

	/*!	Indicates that the specified Team is about to begin the specified aperiodic task, to set
		the starting time for the task. The TimeManager considers all the time that passes between 
		the call to StartAperiodicTask() and the next call to ProcessFeedback() as time spent
		performing the aperiodic task.
		\param taskId				The unique numeric ID of the aperiodic task. See GetAperiodicTaskByName().
		\param team					The Team starting the aperiodic task. */
	void StartAperiodicTask(TaskId taskId, Team& team);

	/*!	Indicates that the specified aperiodic task is about to be performed, to set
		the starting time for the task. The TimeManager considers all the time that passes between 
		the call to StartAperiodicTask() and the next call to ProcessFeedback() as time spent
		performing the aperiodic task.
		\param taskId				The unique numeric ID of the aperiodic task. See GetAperiodicTaskByName(). */
	void StartAperiodicTask(TaskId taskId);

	/*!	Increases the priority of the specified Bot for the specified aperiodic task by the value
		set by the last call to SetTaskPriority() for the task. This is
		typically called when an Bot is denied access to a task, in order to increase the
		chances of that Bot being authorized to perform the task in a succeeding frame.
		\param aperiodicTaskId		The unique numeric ID of the aperiodic task. See GetAperiodicTaskByName().
		\param bot					The Bot whose priority will be increased.
		\return #KY_TRUE if the priority of the Bot was successfully increased for the task, or #KY_FALSE otherwise. */
	KyBool IncreasePriority(TaskId aperiodicTaskId, Bot& bot);

	/*!	Increases the priority of the specified Bot for the specified aperiodic task by the value
		set by the last call to SetTaskPriority() for the task. This is
		typically called when an Team is denied access to a task, in order to increase the
		chances of that Team being authorized to perform the task in a succeeding frame.
		\param aperiodicTaskId		The unique numeric ID of the aperiodic task. See GetAperiodicTaskByName().
		\param team					The Team whose priority will be increased.
		\return #KY_TRUE if the priority of the Team was successfully increased for the task, or #KY_FALSE otherwise. */
	KyBool IncreasePriority(TaskId aperiodicTaskId, Team& team);

	/*!	Increases the absolute priority for the specified Bot by the specified amount. The increased
		priority is taken into account in the next frame. For internal use.
		\param bot					The Bot whose priority will be increased.
		\param inc					The amount to be added to the absolute priorit of the Bot.
		\return #KY_TRUE if the priority of the Bot was successfully increased, or #KY_FALSE otherwise. */
	KyBool IncreaseInstantPriority(Bot& bot, KyFloat32 inc);

	/*!	Increases the absolute priority for the specified Team by the specified amount. The increased
		priority is taken into account in the next frame. For internal use.
		\param team					The Team whose priority will be increased.
		\param inc					The amount to be added to the absolute priority of the Team.
		\return #KY_TRUE if the priority of the Team was successfully increased, or #KY_FALSE otherwise. */
	KyBool IncreaseInstantPriority(Team& team, KyFloat32 inc);

	/*!	Indicates that the specified Bot has completed performing the specified aperiodic task.
		\param taskId				The unique numeric ID of the aperiodic task. See GetAperiodicTaskByName().
		\param bot				The Bot performing the aperiodic task. */
	void ProcessFeedback(TaskId taskId, Bot& bot);

	/*!	Indicates that the specified Team has completed performing the specified aperiodic task.
		\param taskId				The unique numeric ID of the aperiodic task. See GetAperiodicTaskByName().
		\param team					The Team performing the aperiodic task. */
	void ProcessFeedback(TaskId taskId, Team& team);

	/*!	Indicates that the specified aperiodic task has been completed.
		\param taskId				The unique numeric ID of the aperiodic task. See GetAperiodicTaskByName(). */
	void ProcessFeedback(TaskId taskId);

	/*!	Sets whether or not the TimeManager will log warnings about potentially dangerous aperiodic task
		requests for debugging purposes.
		\pp It is generally recommended not to call GrantAperiodicTask(KyInt32, const Bot&) outside of Bot::Think()
		or other methods called during the course of Bot::Think() (e.g. the Think() method of an agent). The
		TimeManager maintains a flag that indicates whether the current time is considered dangerous
		to request an aperiodic task. This flag is set to #KY_TRUE most of the time. It is automatically set
		to #KY_FALSE just before calling the Bot::Think() method of each Bot, and automatically reset
		to #KY_TRUE after all Bot::Think() methods have been called. If you know that your usage of your
		aperiodic task is safe, you can set this flag to #KY_TRUE manually using this method in order to 
		disable the warning messages.
		\param status			#KY_TRUE in order to have the TimeManager log any requests to 
								GrantAperiodicTask(KyInt32, const Bot&) that happen outside of Bot::Think(),
								or #KY_FALSE to disable these log messages. */
	void WarnOnSuspiciousCpuRequests(KyBool status) { m_warnOnSuspiciousCpuRequests = status; }

	/*! Retrieves the maximum number of aperiodic tasks that can be managed by this TimeManager, as set in the
		call to World::Initialize(). */
	KyUInt32 GetMaxAperiodicTask() const
	{
		return m_maxAperiodicTaskCount;
	}

	/*! Retrieves the maximum number of periodic tasks that can be managed by this TimeManager, as set in the
		call to World::Initialize(). */
	KyUInt32 GetMaxPeriodicTask() const
	{
		return m_maxPeriodicTaskCount;
	}

private:
	KyResult Initialize(KyUInt32 maxAperiodicTask, KyUInt32 maxPeriodicTask, KyFloat32 globalTpf);//< called by the world

	void SetMaxAperiodicTask(KyUInt32 maxAperiodicTask);
	void SetMaxPeriodicTask(KyUInt32 maxPeriodicTask);
	void AddBot(Bot& bot);
	void RemoveBot(Bot& bot);
	void Clear();
	void DestructContent();
	void InitContent();

	class GlobalTimerInfo
	{
	public:
		GlobalTimerInfo() : m_timeManager(KY_NULL), m_lastAperiodicTaskUse(KY_NULL) {}
		~GlobalTimerInfo();

		void Initialize(TimeManager& timeManager) { m_timeManager = &timeManager; }
		void SetMaxAperiodicTask(KyUInt32 maxAperiodicTask);
		void ReInit();
		void Clear();

		void StartAperiodicTask(TaskId taskId);
		void ProcessFeedback(TaskId taskId);

#ifdef KYDEBUG
		void CheckEndFrame() const;
#endif
	protected:
		TimeManager* m_timeManager;
		KyFloat32* m_lastAperiodicTaskUse;
		mutable Array<KyBool> m_isAperiodicTaskRunning; //< bit is set to true when we 'do' the task (between StartAperiodicTask and ProcessFeedback).
	};

protected:
	TimeManager();
	~TimeManager();

	class AperiodicTask
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
	public:
		AperiodicTask() :
			m_aperiodicTaskNames(KY_NULL),
			m_taskTpf(KyFloat32MAXVAL),
			m_taskCallMax(KyInt32MAXVAL),
			m_taskPriority(1.0f)
		{}
		~AperiodicTask() { KY_FREE(m_aperiodicTaskNames); }

		void BeginFrame()
		{
			m_currentFrameTaskTime = 0.0f;
			m_currentFrameTaskCall = 0;
		}

		KyFloat32 GetAvailableTime() const { return Max(0.f, m_taskTpf - m_currentFrameTaskTime); }

		bool GrantAperiodicTask() const
		{
			return m_currentFrameTaskCall >= m_taskCallMax || m_currentFrameTaskTime > m_taskTpf;
		}

		void ForbidTask() { m_currentFrameTaskCall = m_taskCallMax + 1; }

	public:
		char* m_aperiodicTaskNames; /*< The unique names of the aperiodic Tasks configured for the World. Do not modify. */
		KyFloat32 m_taskTpf;
		KyInt32 m_taskCallMax;
		KyFloat32 m_taskPriority;
		KyFloat32 m_currentFrameTaskTime;
		KyInt32 m_currentFrameTaskCall;
	};

	World* m_world;

	KyUInt32 m_maxAperiodicTaskCount; /*< The maximum number of aperiodic Tasks that can be created in the World. Do not modify. */
	KyUInt32 m_maxPeriodicTaskCount; /*< The maximum number of periodic Tasks that can be created in the World. Do not modify. */

	KyUInt32 m_periodicCount; /*< The number of periodic Tasks configured for the World. Do not modify. */
	KyUInt32 m_aperiodicCount; /*< The number of aperiodic Tasks configured for the World. Do not modify. */

	AperiodicTask* m_aperiodicTasks;
	char** m_periodicTaskNames; /*< The unique names of the periodic Tasks configured for the World. Do not modify. */

	// Parameters set by user
	KyFloat32 m_kaimTpf;

	KyFloat32* m_taskPeriodicity;

	// Parameters used for time management over several frames
	GlobalTimerInfo m_globalInfo;

	// Parameters related to the current frame management
	KyFloat32 m_currentFrameStart;
	KyFloat32 m_maxFrameTime;
	KyFloat32 m_lastFrameStart;
	KyFloat32 m_frameDeltaTime;
	KyFloat32 m_lastElapsedTime;

	Array<Bot*> m_orderedBots;
	KyUInt32 m_nextBotId;
	KyBool m_noMoreTime;
	const Bot* m_prioritaryBotId;

	// Usually you should never call GrantAperiodicTask(id, bot) outside of the Brain::Think
	// function (or sub-fonctions). This flag is set to false before calling Brain::Think functions 
	// and is reset just after. When m_warnOnSuspiciousCpuRequests is true a warning message is 
	// triggered when GrantAperiodicTask(id, bot) is called.
	KyBool m_warnOnSuspiciousCpuRequests;
};


} // namespace Kaim

#endif // KY_TIMEMANAGER_H
