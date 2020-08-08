/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_mesh */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_NAVMESHACCESSOR_DATABASE_H
#define KY_NAVMESHACCESSOR_DATABASE_H

#include <kyruntime/mesh/inavmeshaccessor.h>
#include <kyruntime/graph/databasemanager.h>


namespace Kaim
{

class Database;
class NavMesh;


/*!	This implementation of INavMeshAccessor provides its ObstacleLayer with all \NavMeshes streamed into in one
	or more \Databases maintained by the DatabaseManager.
	\pp This implementation of INavMeshAccessor is recommended for use in most cases. Once it has been configured in the
	WorldDefinition, its use is entirely transparent.
	\pp You configure the NavMeshAccessor_Database with one or more target Databases by including one or more \c Database 
	configuration parameters for the NavMeshAccessor_Database in the WorldDefinition. 
	\pp At its initialization, the NavMeshAccessor_Database registers its NavMeshCB() method as a callback for the 
	DatabaseManager Service, to be called each time a NavMesh is streamed in or out of memory. When the NavMeshCB() callback detects 
	that a NavMesh has been streamed into one of the target \Databases configured for the NavMeshAccessor_Database, it adds
	that NavMesh to its ObstacleLayer. Likewise, when the NavMeshCB() callback detects that a NavMesh has been streamed out of
	one of the target \Databases configured for the NavMeshAccessor_Database, it removes that NavMesh from its ObstacleLayer.
	\ingroup kyruntime_mesh */
class NavMeshAccessor_Database: public INavMeshAccessor
{
public:
	KY_DECLARE_NAVMESHACCESSOR(NavMeshAccessor_Database)


public:
	/*!	\label_constructor For internal use.
		\param layer				The ObstacleLayer that this object will supply with \NavMeshes. */
	NavMeshAccessor_Database(INavMeshLayer* layer);

	/*!	\label_virtualdestructor */
	~NavMeshAccessor_Database();

	/*!	\label_init
		\param maxDatabases			The maximum number of \Databases that this object will watch for newly streamed \NavMeshes. */
	KyResult Initialize(KyUInt32 maxDatabases);

	/*!	Configures the NavMeshAccessor_Database to retrieve \NavMeshes from the specified Database, and provide those \NavMeshes
		to its ObstacleLayer. */
	KyResult AddDatabase(Database& database);

	/*!	Indicates whether or not the specified Database has been added to this NavMeshAccessor_Database through a call to AddDatabase(). */
	bool ManagesDatabase(const Database* db);

	/*!	This method is transparently registered at initialization time with the DatabaseManager as a NavMesh streaming callback.
		When a NavMesh is streamed into memory, this method is called to evaluate what action needs to be taken in response.
		It calls OnAddNavMesh() to add the NavMesh to the ObstacleLayer.*/
	static void OnAddNavMeshCB(const NavMesh& navMesh, Database& database, void* userData);

	/*!	This method is transparently registered at initialization time with the DatabaseManager as a NavMesh streaming callback.
		When a NavMesh is streamed out of memory, this method is called to evaluate what action needs to be taken in response.
		It calls OnRemoveNavMesh() to remove the NavMesh from the ObstacleLayer.*/
	static void OnRemoveNavMeshCB(const NavMesh& navMesh, Database& database, void* userData);

	/*!	Adds the specified NavMesh to the ObstacleLayer that uses this NavMeshAccessor_Database by calling ObstacleLayer::AddMesh(). */
	virtual void OnAddNavMesh(const NavMesh& navMesh, Database* database);

	/*!	Removes the specified NavMesh from the ObstacleLayer that uses this NavMeshAccessor_Database by calling ObstacleLayer::RemoveMesh(). */
	virtual void OnRemoveNavMesh(const NavMesh& navMesh);


private:
	Array<const Database*> m_relevantDatabases;
};

} // namespace Kaim

#endif // KY_NAVMESHACCESSOR_DATABASE_H
