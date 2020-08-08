/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_DATABASEMANAGER_H
#define KY_DATABASEMANAGER_H

#include <kyruntime/graph/traversalmanager.h>
#include <kyruntime/graph/databasemanagercallbacks.h>
#include <kyruntime/graph/database.h>


#include <kypathdata/navmesh/navmeshutils.h>
#include <kypathdata/navmesh/navcellgridcallbacks.h>
#include <kyruntime/graph/graphcellgridcallbacks.h>


namespace Kaim
{

class NavMesh;

///////////////////////////////////////////////////////////////////////////////////////////
// DatabaseManager
///////////////////////////////////////////////////////////////////////////////////////////

/*!	The DatabaseManager is responsible for managing a set of \Databases, each of which contains the PathData that
	represents the terrain from the point of view of a specific type of Bot. 
	\pp Each World contains a DatabaseManager, which you can access by calling World::GetDatabaseManager().
	\pp In order to load PathData into your World, you must call the DatabaseManager::Initialize() method of the
	DatabaseManager owned by your World, and specify the maximum number of \Databases it will need to manage. You then
	must call AddDatabase() to create each Database. 
	\pp When you need to add or remove PathData in your World, you retrieve a Database from the DatabaseManager, and use
	the methods provided by the Database class.
	\pp See also :USERGUIDE:"Using PathData at Runtime".
	\ingroup kyruntime_graph */
class DatabaseManager
{
	/*! Copies the state of the specified DatabaseManager to this object. */
	DatabaseManager& operator=(const DatabaseManager&);

	friend class Database;
	friend class World;
	KY_DEFINE_NEW_DELETE_OPERATORS

	/*! \label_constructor For internal use. */
	DatabaseManager() : m_world(KY_NULL) {}

	/*! Sets the World that owns this DatabaseManager. For internal use. */
	void SetWorld(World& world) { m_world = &world; }

public:

	/*! \label_init
		\param numDatabasesToReserve			Specifies the maximum number of \Databases this this DatabaseManager will be able to manage.
		\return A #KyResult that indicates the success or failure of the request. */
	KyResult Initialize(const KyUInt32& numDatabasesToReserve = 1);
	
	/*! Adds a new Database to the DatabaseManager. The lifespan of the new object is managed by the DatabaseManager.
		\param name				A unique name that identifies the new Database.
		\return A pointer to the newly created Database, or #KY_NULL if an error occurred. */
	Database* AddDatabase(const char* name);
	
	/*! \label_clear */
	void Clear();

	/*! \label_reinit */
	void ReInit();

	/*! Calls Database::Render() for each managed Database. */
	void Render() const;

	/*! Retrieves a pointer to the specified Database.
		\param name		The name of the Database to retrieve. */
	Database* GetDatabaseByName(const char* name) const;
	
	/*! Retrieves a pointer to the specified Database.
		\param idx		The index of the Database to retrieve. */
	Database* GetDatabase(KyUInt32 idx) const;

	/*! Retrieves the current number of \Databases managed by the DatabaseManager. */
	KyUInt32 GetDatabaseCount() const { return m_databases.GetCount(); }

	/*! \label_getworld */
	World& GetWorld() const { return *m_world; }

	////////// CALLBACKS /////////////

	/*!	Registers a callback function that the DatabaseManager will call each time it adds a
		Graph to a Database. This callback function must satisfy the DatabaseManagerCallBacks::OnAddGraphCB type definition.
		\param cb			The callback function to be invoked.
		\param userData		A void pointer that will be passed to the callback function.
		\return #KY_TRUE if the callback was successfully registered, or #KY_FALSE otherwise.
				#KY_FALSE typically indicates no more memory. */
	KyBool RegisterOnAddGraphCB(DatabaseManagerCallBacks::OnAddGraphCB cb, void* userData) { return m_callBacks.RegisterOnAddGraphCB(cb, userData); }

	/*!	Registers a callback function that the DatabaseManager will call each time it removes a
		Graph from a Database. This callback function must satisfy the DatabaseManagerCallBacks::OnRemoveGraphCB type definition.
		\param cb			The callback function to be invoked.
		\param userData		A void pointer that will be passed to the callback function.
		\return #KY_TRUE if the callback was successfully registered, or #KY_FALSE otherwise.
				#KY_FALSE typically indicates no more memory. */
	KyBool RegisterOnRemoveGraphCB(DatabaseManagerCallBacks::OnRemoveGraphCB cb, void* userData) { return m_callBacks.RegisterOnRemoveGraphCB(cb, userData); }

	/*!	Registers a callback function that the DatabaseManager will call each time it adds a
		AdditionalGraph to a Database. This callback function must satisfy the DatabaseManagerCallBacks::OnAddAdditionalGraphCB type definition.
		\param cb			The callback function to be invoked.
		\param userData		A void pointer that will be passed to the callback function.
		\return #KY_TRUE if the callback was successfully registered, or #KY_FALSE otherwise.
				#KY_FALSE typically indicates no more memory. */
	KyBool RegisterOnAddAdditionalGraphCB(DatabaseManagerCallBacks::OnAddAdditionalGraphCB cb, void* userData) { return m_callBacks.RegisterOnAddAdditionalGraphCB(cb, userData); }

	/*!	Registers a callback function that the DatabaseManager will call each time it removes a
		AdditionalGraph from a Database. This callback function must satisfy the DatabaseManagerCallBacks::OnRemoveAdditionalGraphCB type definition.
		\param cb			The callback function to be invoked.
		\param userData		A void pointer that will be passed to the callback function.
		\return #KY_TRUE if the callback was successfully registered, or #KY_FALSE otherwise.
				#KY_FALSE typically indicates no more memory. */
	KyBool RegisterOnRemoveAdditionalGraphCB(DatabaseManagerCallBacks::OnRemoveAdditionalGraphCB cb, void* userData) { return m_callBacks.RegisterOnRemoveAdditionalGraphCB(cb, userData); }

	/*!	Registers a callback function that the DatabaseManager will call each time it adds a
		NavMesh to a Database. This callback function must satisfy the DatabaseManagerCallBacks::OnAddNavMeshCB type definition.
		\param cb			The callback function to be invoked.
		\param userData		A void pointer that will be passed to the callback function.
		\return #KY_TRUE if the callback was successfully registered, or #KY_FALSE otherwise.
				#KY_FALSE typically indicates no more memory. */
	KyBool RegisterOnAddNavMeshCB(DatabaseManagerCallBacks::OnAddNavMeshCB cb, void* userData) { return m_callBacks.RegisterOnAddNavMeshCB(cb, userData); }

	/*!	Registers a callback function that the DatabaseManager will call each time it removes a
		NavMesh from a Database. This callback function must satisfy the DatabaseManagerCallBacks::OnRemoveNavMeshCB type definition.
		\param cb			The callback function to be invoked.
		\param userData		A void pointer that will be passed to the callback function.
		\return #KY_TRUE if the callback was successfully registered, or #KY_FALSE otherwise.
				#KY_FALSE typically indicates no more memory. */
	KyBool RegisterOnRemoveNavMeshCB(DatabaseManagerCallBacks::OnRemoveNavMeshCB cb, void* userData) { return m_callBacks.RegisterOnRemoveNavMeshCB(cb, userData); }

	/*!	Removes a callback function previously added by a call to RegisterOnAddGraphCB().
		\param cb			The callback function to remove.
		\param userData		The \c userData parameter passed to the original call to RegisterOnAddGraphCB().
		\note Both the \c cb and \c userData parameters must match the original values passed to RegisterSOnAddubGraphCB().
		\return #KY_TRUE if the callback was successfully removed, or #KY_FALSE otherwise. */
	KyBool UnRegisterOnAddGraphCB(DatabaseManagerCallBacks::OnAddGraphCB cb, void* userData) { return m_callBacks.UnRegisterOnAddGraphCB(cb, userData); }

	/*!	Removes a callback function previously added by a call to RegisterOnRemoveGraphCB().
		\param cb			The callback function to remove.
		\param userData		The \c userData parameter passed to the original call to RegisterOnRemoveGraphCB().
		\note Both the \c cb and \c userData parameters must match the original values passed to RegisterOnRemoveGraphCB().
		\return #KY_TRUE if the callback was successfully removed, or #KY_FALSE otherwise. */
	KyBool UnRegisterOnRemoveGraphCB(DatabaseManagerCallBacks::OnRemoveGraphCB cb, void* userData) { return m_callBacks.UnRegisterOnRemoveGraphCB(cb, userData); }

	/*!	Removes a callback function previously added by a call to RegisterOnAddAdditionalGraphCB().
		\param cb			The callback function to remove.
		\param userData		The \c userData parameter passed to the original call to RegisterOnAddAdditionalGraphCB().
		\note Both the \c cb and \c userData parameters must match the original values passed to RegisterSOnAddubAdditionalGraphCB().
		\return #KY_TRUE if the callback was successfully removed, or #KY_FALSE otherwise. */
	KyBool UnRegisterOnAddAdditionalGraphCB(DatabaseManagerCallBacks::OnAddAdditionalGraphCB cb, void* userData) { return m_callBacks.UnRegisterOnAddAdditionalGraphCB(cb, userData); }

	/*!	Removes a callback function previously added by a call to RegisterOnRemoveAdditionalGraphCB().
		\param cb			The callback function to remove.
		\param userData		The \c userData parameter passed to the original call to RegisterOnRemoveAdditionalGraphCB().
		\note Both the \c cb and \c userData parameters must match the original values passed to RegisterOnRemoveAdditionalGraphCB().
		\return #KY_TRUE if the callback was successfully removed, or #KY_FALSE otherwise. */
	KyBool UnRegisterOnRemoveAdditionalGraphCB(DatabaseManagerCallBacks::OnRemoveAdditionalGraphCB cb, void* userData) { return m_callBacks.UnRegisterOnRemoveAdditionalGraphCB(cb, userData); }

	/*!	Removes a callback function previously added by a call to RegisterOnAddNavMeshCB().
		\param cb			The callback function to remove.
		\param userData		The \c userData parameter passed to the original call to RegisterOnAddNavMeshCB().
		\note Both the \c cb and \c userData parameters must match the original values passed to RegisterOnAddNavMeshCB().
		\return #KY_TRUE if the callback was successfully removed, or #KY_FALSE otherwise. */
	KyBool UnRegisterOnAddNavMeshCB(DatabaseManagerCallBacks::OnAddNavMeshCB cb, void* userData) { return m_callBacks.UnRegisterOnAddNavMeshCB(cb, userData); }

	/*!	Removes a callback function previously added by a call to RegisterOnRemoveNavMeshCB().
		\param cb			The callback function to remove.
		\param userData		The \c userData parameter passed to the original call to RegisterOnRemoveNavMeshCB().
		\note Both the \c cb and \c userData parameters must match the original values passed to RegisterOnRemoveNavMeshCB().
		\return #KY_TRUE if the callback was successfully removed, or #KY_FALSE otherwise. */
	KyBool UnRegisterOnRemoveNavMeshCB(DatabaseManagerCallBacks::OnRemoveNavMeshCB cb, void* userData) { return m_callBacks.UnRegisterOnRemoveNavMeshCB(cb, userData); }

	/*! Invokes all callback functions previously added through calls to RegisterOnAddGraphCB(). */
	void CallOnAddGraphCBs(const Graph& graph, Database& db)    const { m_callBacks.CallOnAddGraphCBs(graph, db); }

	/*! Invokes all callback functions previously added through calls to RegisterOnRemoveGraphCB(). */
	void CallOnRemoveGraphCBs(const Graph& graph, Database& db) const { m_callBacks.CallOnRemoveGraphCBs(graph, db); }

	/*! Invokes all callback functions previously added through calls to RegisterOnAddAdditionalGraphCB(). */
	void CallOnAddAdditionalGraphCBs(const AdditionalGraph& graph, Database& db)    const { m_callBacks.CallOnAddAdditionalGraphCBs(graph, db); }

	/*! Invokes all callback functions previously added through calls to RegisterOnRemoveAdditionalGraphCB(). */
	void CallOnRemoveAdditionalGraphCBs(const AdditionalGraph& graph, Database& db) const { m_callBacks.CallOnRemoveAdditionalGraphCBs(graph, db); }

	/*! Invokes all callback functions previously added through calls to RegisterOnAddNavMeshCB(). */
	void CallOnAddNavMeshCBs(const NavMesh& navmesh, Database& db)    const { m_callBacks.CallOnAddNavMeshCBs(navmesh, db); }

	/*! Invokes all callback functions previously added through calls to RegisterOnRemoveNavMeshCB(). */
	void CallOnRemoveNavMeshCBs(const NavMesh& navmesh, Database& db) const { m_callBacks.CallOnRemoveNavMeshCBs(navmesh, db); }


	/*!	Called transparently by Database::RegisterOnActivateNavCellCB(). */
	void RegisterOnActivateNavCellCB(NavCellGridCallBacks::OnActivateNavCellCB cb, void* userData);

	/*!	Called transparently by Database::RegisterOnDeActivateNavCellCB(). */
	void RegisterOnDeActivateNavCellCB(NavCellGridCallBacks::OnDeActivateNavCellCB cb, void* userData);

	/*!	Called transparently by Database::RegisterOnActivateNavCellCB(). */
	void UnRegisterOnActivateNavCellCB(NavCellGridCallBacks::OnActivateNavCellCB cb, void* userData);

	/*!	Called transparently by Database::RegisterOnDeActivateNavCellCB(). */
	void UnRegisterOnDeActivateNavCellCB(NavCellGridCallBacks::OnDeActivateNavCellCB cb, void* userData);


	/*!	Called transparently by Database::RegisterOnActivateGraphCellCB(). */
	void RegisterOnActivateGraphCellCB(GraphCellGridCallBacks::OnActivateGraphCellCB cb, void* userData);

	/*!	Called transparently by Database::RegisterOnDeActivateGraphCellCB(). */
	void RegisterOnDeActivateGraphCellCB(GraphCellGridCallBacks::OnDeActivateGraphCellCB cb, void* userData);

	/*!	Called transparently by Database::RegisterOnActivateGraphCellCB(). */
	void UnRegisterOnActivateGraphCellCB(GraphCellGridCallBacks::OnActivateGraphCellCB cb, void* userData);

	/*!	Called transparently by Database::RegisterOnDeActivateGraphCellCB(). */
	void UnRegisterOnDeActivateGraphCellCB(GraphCellGridCallBacks::OnDeActivateGraphCellCB cb, void* userData);

	void AddRegisteredCellCallacksToDatabase(Database& database);

protected:

	/*! \label_virtualdestructor */
	~DatabaseManager();

protected:
	DatabaseManagerCallBacks m_callBacks;
	NavCellGridCallBacks m_navCellCallBacks;
	GraphCellGridCallBacks m_graphCellCallBacks;

	Array<Database*> m_databases;

	World* m_world; //<! the world the databaseManager belongs to
};

} // namespace Kaim

#endif // KY_DATABASEMANAGER_H
