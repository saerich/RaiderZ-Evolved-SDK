/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_TRAVERSALMANAGER_H
#define KY_TRAVERSALMANAGER_H

#include <kypathdata/basesystem/config.h>
#include <kyruntime/graph/traversalconfig.h>
#include <kyruntime/graph/traversal.h>
#include <kypathdata/basesystem/memorylog.h>

namespace Kaim
{
class World;

/*!	The TraversalPoolConfig class maintains the information needed by the TraversalManager to create a pool of
	\GraphTraversals of a particular class. In order to create a pool, you must create an instance
	of this class for the class of GraphTraversal that you want the TraversalManager to make available in your
	World, and pass it in a call to TraversalManager::CreateTraversalPool().
	\pp For additional information, see :USERGUIDE:"Using GraphTraversals".
	\ingroup kyruntime_graph */
class TraversalPoolConfig
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	/*!	\label_constructor \label_mustinit */
	TraversalPoolConfig() : m_cfgName(KY_NULL), m_traversalConfig(KY_NULL), m_instanceCount(0) {}

	/*!	\label_constructor
		\param cfgName			Sets the value of #m_cfgName.
		\param traversalConfig	Sets the value of #m_traversalConfig. 
		\param instanceCount	Sets the value of #m_instanceCount.  */
	TraversalPoolConfig(const char* cfgName, const GraphTraversalConfig& traversalConfig, KyUInt32 instanceCount = 1)
		 : m_cfgName(KY_NULL), m_traversalConfig(KY_NULL), m_instanceCount(0) 
	{ Initialize(cfgName, traversalConfig, instanceCount); }

	/*!	\label_constructor Copies the state of the specified TraversalPoolConfig to the newly constructed object. */
	TraversalPoolConfig(const TraversalPoolConfig& t)
	{
		Initialize(t.m_cfgName, *t.m_traversalConfig, t.m_instanceCount);
	}

	/*!	Copies the state of the specified TraversalPoolConfig to this object. */
	TraversalPoolConfig& operator=(const TraversalPoolConfig& t)
	{
		Initialize(t.m_cfgName, *t.m_traversalConfig, t.m_instanceCount);
		return *this;
	}

	/*! \label_destructor */
	~TraversalPoolConfig()
	{
		Clear();
	}

	/*!	\label_init
		\param cfgName			Sets the value of #m_cfgName.
		\param traversalConfig	Sets the value of #m_traversalConfig. 
		\param instanceCount	Sets the value of #m_instanceCount. */
	void Initialize(const char* cfgName, const GraphTraversalConfig& traversalConfig, KyUInt32 instanceCount=1)
	{
		Clear();

		KY_LOG_MEMORY("TraversalManager");

		m_cfgName = Strdup(cfgName);
		m_traversalConfig = traversalConfig.Clone();
		m_instanceCount = instanceCount;
	}

	/*! \label_clear */
	void Clear()
	{
		KY_DELETE_CLASS(m_traversalConfig);
		m_traversalConfig = KY_NULL;
		KY_FREE(m_cfgName);
		m_cfgName = KY_NULL;
		m_instanceCount = 0;
	}

	/*! Indicates whether or not this object has been correctly initialized. */
	KyBool IsValid() const { return m_cfgName != KY_NULL && m_instanceCount != 0 && m_traversalConfig != KY_NULL && m_traversalConfig->IsValid(); }

	/*!	Retrieves the value of #m_cfgName. */
	const char* GetName() const { return m_cfgName; }

	/*!	Retrieves the value of #m_traversalConfig. */
	const GraphTraversalConfig* GetGraphTraversalConfig() const { return m_traversalConfig; }

	/*!	Retrieves the value of #m_instanceCount. */
	KyUInt32 GetInstanceCount() const { return m_instanceCount; }

private:
	/*!	A unique name for this type of GraphTraversal pool. You can use this name to retrieve the unique ID assigned to
		the pool of GraphTraversals created for this GraphTraversal pool by the TraversalManager. */
	char* m_cfgName;

	/*!	A configuration object that the TraversalManager will use to get each instance of the GraphTraversal for the pool. */
	GraphTraversalConfig* m_traversalConfig;

	/*!	The number of GraphTraversal objects that the TraversalManager will create from #m_traversalConfig and put in the pool. 
		You should always use the default value of 1 for this member unless instructed otherwise by Autodesk Support. Creating a
		pool with more than one GraphTraversal uses up extra memory without any gain in performance. */
	KyUInt32 m_instanceCount;
};

/*!	The TraversalManager provides other components in the World with access to \GraphTraversals
	that they can use to analyze the terrain dynamically at runtime. The World creates and maintains
	an instance of this class, which you can access at any time by calling World::GetTraversalManager().
	\section travmgrsetup Setting Up the TraversalManager
	In order to analyze the terrain using a GraphTraversal, you must first set up the TraversalManager to
	create a pool of GraphTraversal objects of a certain class. To do this, you must create an instance of
	TraversalPoolConfig to identify the class of GraphTraversal that should be created, and pass that
	TraversalPoolConfig in a call to CreateTraversalPool().
	\section travmgrusing Getting a Traversal
	Your custom components such as \Bots, \Teams, agents or services can retrieve an instance of a GraphTraversal
	from any of the pools maintained by the TraversalManager at any time by calling one of the different
	versions of GetTraversal(). Your component can then set up and use the GraphTraversal as required to
	analyze the terrain. You should call GetTraversal() at each frame as long as you are using the GraphTraversal.
	\pp When finished, your component should call ReleaseTraversal() to free the GraphTraversal
	and return it to the pool maintained by the TraversalManager. If you do not call ReleaseTraversal()
	explicitly, but you stop calling GetTraversal(), the TraversalManager will assume that your component is
	no longer using the GraphTraversal, and will force its release back into the pool.
	\pp Note that if you have set any data in the GraphTraversal you retrieve, that data will still be 
	maintained by the GraphTraversal when you release it. The next component that retrieves that GraphTraversal
	will use the new values. For that reason, it is recommended that you reset any values you set in the
	GraphTraversal before you release it.
	\pp For additional information, see :USERGUIDE:"Using GraphTraversals".
	\ingroup kyruntime_graph */
class TraversalManager
{
	friend class World;

	TraversalManager& operator=(const TraversalManager&);

	KY_DEFINE_NEW_DELETE_OPERATORS

private:
	TraversalManager() : m_world(KY_NULL) {}
	void SetWorld(World& world) { m_world = &world; }

public:

	/*!	\label_init
		\param traversalCfg			An array of TraversalPoolConfig objects, each of which configures a pool of \GraphTraversals
									of a specific type.
		\param traversalCfgSize		The number of elements in the \c traversalCfg array. */
	KyResult CreateTraversalPool(const TraversalPoolConfig& traversalCfg);

	/*!	\label_clear \label_mustinit */
	void Clear();


	/*!	\label_getworld */
	World& GetWorld() const { return *m_world; }

	/////////////////////////////////////////////////////////////////////////////////////////////
	// Traversal API
	/////////////////////////////////////////////////////////////////////////////////////////////

	/*!	Type definition for the unique ID of a type of \GraphTraversals. */
	typedef KyUInt32 ConfigId;
	
	/*!	Represents a type of GraphTraversal that is not recognized by the TraversalManager. */
	static const ConfigId InvalidConfigId = KyUInt32MAXVAL;

	/*!	Retrieves a unique ID for a specific type of GraphTraversal. 
		\param traversalConfigName	The unique name for the GraphTraversal type as specified in the
									GraphTraversalConfig passed to CreateTraversalPool().
		\return The unique ID of the specified type of GraphTraversal, or -1 if no matching configuration was found. */
	ConfigId GetTraversalConfigId(const char* traversalConfigName) const;

	/*!	Retrieves an instance of the specified GraphTraversal from the pool maintained by the TraversalManager. 
		To specify the type of GraphTraversal you want to retrieve, you must provide the 
		unique identification number returned by GetTraversalConfigId().
		\pp Each time you call this method, you must also specify the identity of the component that is intended 
		to use the GraphTraversal. This method is overloaded to accept an Bot, a Team, or a void pointer as 
		the requester. If you specify an Bot or a Team, the GraphTraversal will automatically increase the 
		priority of the requester whenever no time remains in the current frame for the GraphTraversal to perform 
		its calculations, increasing the chances that CPU time will be provided to the requester in the 
		following frame.
		\pp You must call this method each frame until you have finished using the GraphTraversal. The GraphTraversal 
		will not perform any requested calculations in a frame until you have called this method. 
		\pp If the TraversalManager detects that you have not called this method for several 
		frames, but have not released your GraphTraversal by calling ReleaseTraversal(), it will 
		automatically force the release of your GraphTraversal in order to make it available to other 
		requesters.
		\param traversalConfigId	The unique ID of the GraphTraversal configuration that defines the type
									of GraphTraversal you want to retrieve. See GetTraversalConfigId().
		\param requester 			The component requesting the GraphTraversal. 
		\return a pointer to a GraphTraversal of the specified type, or #KY_NULL if no GraphTraversal is available for the 
				requested configuration. */
	GraphTraversal* GetTraversal(ConfigId traversalConfigId, Bot& requester);
	
	/*! \copydoc GetTraversal(ConfigId,Bot&) */
	GraphTraversal* GetTraversal(ConfigId traversalConfigId, Team& requester);
	
	/*! \copydoc GetTraversal(ConfigId,Bot&) */
	GraphTraversal* GetTraversal(ConfigId traversalConfigId, void* requester);

	/*!	Releases the specified GraphTraversal back into the pool maintained by the TraversalManager,
		for future re-use by other requesters.
		\pp You should call this method when you have no further need of the GraphTraversal. If you 
		do not, the TraversalManager will release the GraphTraversal automatically as soon as it 
		recognizes that you have stopped calling GetTraversal() at each frame. However, in this 
		case the GraphTraversal may be locked unnecessarily for up to five frames. */
	void ReleaseTraversal(GraphTraversal& traversal);

	/*!	The TraversalManager uses an instance of TraversalPool to represent each pool of \GraphTraversals of a given type. 
		For internal use.
		\ingroup kyruntime_graph */
	class TraversalPool
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
	public:
		TraversalPool(): m_pool(), m_configName(KY_NULL) {};
		~TraversalPool();

		void Initialize(const TraversalPoolConfig& config, TraversalManager& traversalManager);

		Array<GraphTraversal*>& Traversals() {return m_pool; }
		const char* Name() const { return m_configName; }

	private:
		TraversalPool& operator=(const TraversalPool& t);
		Array<GraphTraversal*> m_pool;
		char* m_configName;
	};

	/*!	Returns the array of TraversalPool objects that represent the pools of GraphTraversals maintained by the
		TraversalManager. For internal use. */
	const Array<TraversalPool*>& GetTraversalData() { return m_traversalData; }

	/*!	Returns the number of GraphTraversal pools maintained by the TraversalManager. For internal use in conjunction
		with GetTraversalData(). */
	KyUInt32 GetTraversalDataSize() { return m_traversalData.GetCount(); }


protected:
	~TraversalManager();

	GraphTraversal* GetTraversal(ConfigId traversalConfigId, void* requester, GraphTraversal::CurrentUser::EType type);

	/* Stores the pools of \GraphTraversals maintained by the TraversalManager Service. 
		Do not use this member directly. Call GetTraversal() instead. */
	Array<TraversalPool*> m_traversalData;

	World* m_world; //<! the world the TraversalManager belongs to
};

} // namespace Kaim

#endif // KY_TRAVERSALMANAGER_H
