/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */

// primary contact: JUBA - secondary contact: NOBODY

#ifndef KY_DATABASE_H
#define KY_DATABASE_H

#include <kyruntime/graph/graphmanager.h>

#include <kyruntime/mesh/navmeshqueryserverwrapper.h>

#include <kypathdata/generic/types.h>
#include <kypathdata/navmesh/navmeshmanager.h>


namespace Kaim
{

class Graph;
class NavMesh;
class DatabaseManager;
class PathData;

///////////////////////////////////////////////////////////////////////////////////////////
// Database
///////////////////////////////////////////////////////////////////////////////////////////

/*!	This class is a runtime container for all PathData that represents the world from the point
	of view of a specific type of Bot. \Databases are created and maintained by the DatabaseManager.
	\pp You add and remove PathData in the Database by calling AddPathData() and RemovePathData().
	\pp Each Database maintains a GraphManager that is responsible for handling the \Graphs stored
	in the PathData, and a NavMeshManager that is responsible for handling the \NavMeshes stored
	in the Database.
	\pp The Database also maintains an instance of NavMeshQueryServerWrapper, which you can retrieve
	through a call to GetNavMeshQueryServer(). This object allows you to make runtime queries against the set of
	\NavMeshes managed by the NavMeshManager.
	\pp See also :USERGUIDE:"Using PathData at Runtime".
	\ingroup kyruntime_graph */
class Database
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! Retrieves the unique name set for the Database. */
	const char* GetName() const;

	/*! Retrieves the index of the Database in the list of \Databases maintained by the DatabaseManager. */
	KyUInt32 GetDatabaseIndex() const { return m_databaseIndex; }

	/*! Adds the specified PathData to the Database.
		\return A #KyResult that indicates the success or failure of the request. */
	KyResult AddPathData(const PathData& pathData);

	/*! Removes the specified PathData from the Database.
		\return A #KyResult that indicates the success or failure of the request. */
	KyResult RemovePathData(const PathData& pathData);

	/*!	Adds the specified Graph to the GraphManager maintained by this Database. 
		\return A pointer to a GraphReference for the new Graph, or #KY_NULL if an error occurred. */
	GraphReference* AddGraph(const Graph& graph);
	
	/*!	Removes the specified Graph from the GraphManager maintained by this Database.
		\return A #KyResult that indicates the success or failure of the request. */
	KyResult RemoveGraph(const Graph& graph);

	/*! For internal use. */
	GraphReference* AddAdditionalGraph(const AdditionalGraph& additionalGraph);

	/*! For internal use. */
	KyResult RemoveAdditionalGraph(const AdditionalGraph& additionalGraph);

	/*!	Adds the specified NavMesh to the NavMeshManager maintained by this Database. 
		\return A pointer to a NavMeshReference for the new NavMesh, or #KY_NULL if an error occurred. */
	NavMeshReference* AddNavMesh(const NavMesh& navMesh);
	
	/*!	Removes the specified NavMesh from the NavMeshManager maintained by this Database.
		\return A #KyResult that indicates the success or failure of the request. */
	KyResult RemoveNavMesh(const NavMesh& navMesh);

	/*!	Removes all \Graphs and \NavMeshes from the Database. */
	void RemoveAll();

	/*!	Retrieves the NavMeshQueryServerWrapper, which provides access to the \NavMeshes loaded into the Database. */
	NavMeshQueryServerWrapper& GetNavMeshQueryServer();

	/*!	Calls the GraphManager::Render() method of the GraphManager maintained by the Database. */
	void Render() const;

	/*! Retrieves the DatabaseManager that maintains this Database. */
	const DatabaseManager& GetDatabaseManager() const { return *m_databaseManager; }

	/*! Retrieves the DatabaseManager that maintains this Database. */
	DatabaseManager& GetDatabaseManager() { return *m_databaseManager; }

	/*! Retrieves the GraphManager maintained by this Database. */
	const GraphManager& GetGraphManager() const { return m_graphManager; }

	/*! Retrieves the GraphManager maintained by this Database. */
	GraphManager& GetGraphManager() { return m_graphManager; }

	/*!	\label_getengine */
	Engine& GetEngine();

	/*!	\label_getengine */
	const Engine& GetEngine() const;

	/*! Retrieves the NavMeshManager maintained by this Database. */
	const NavMeshManager& GetNavMeshManager() const { return m_navMeshManager; }

	/*! Retrieves the NavMeshManager maintained by this Database. */
	NavMeshManager& GetNavMeshManager() { return m_navMeshManager; }

	/*! Indicates whether or not the specified PathData object was created with the same generation parameters
		as the PathData objects that have already been loaded into this Database. */
	bool IsCompatibleWith(const PathData& pathData) const;

private:
	friend class DatabaseManager;
	
	Database(DatabaseManager& dm, KyUInt32 databaseIndex, const char* name);

	~Database();

	void Clear();

	mutable char* m_name; /*< A unique name assigned to this Database by the DatabaseManager Service from the EngineDefinition. */
	KyUInt32 m_databaseIndex; /*< A unique identifier assigned to this Database by the DatabaseManager Service. */

	GraphManager m_graphManager; /*< Contains a pointer to the GraphManager maintained by this Database. */
	NavMeshManager m_navMeshManager; /*< An object that manages the loading, removal and stitching of \NavMeshes. Do not modify. */

	NavMeshQueryServerWrapper m_navMeshQueryServer; /*< The NavMeshQueryServerWrapper instance. Do not modify. Access through GetNavMeshQueryServer(). */

	DatabaseManager* m_databaseManager; /*< Contains a pointer to the DatabaseManager Service. Do not modify. */
};

} // namespace Kaim

#endif // KY_DATABASE_H
