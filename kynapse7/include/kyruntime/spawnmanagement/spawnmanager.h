/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_spawnmanagement */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_SPAWNMANAGER_H
#define KY_SPAWNMANAGER_H

#include <kyruntime/core/service.h>

#include <kypathdata/containers/list.h>
#include <kypathdata/navmesh/navmeshutils.h>
#include <kypathdata/navmesh/navtriangleptr.h>

#include <kyruntime/mesh/navmeshlayerlocation.h>


namespace Kaim
{

class IVolume;
class NavMeshQueryServerWrapper;
class NavMeshManager;
class SpawnQuery;
class Vec3f;
class WorldService;


/*!	The SpawnManager class is an implementation of WorldService that finds and validates spawning
	positions for characters. These spawning points are guaranteed to be at least a specified distance from:
	-	Each other Body in the World.
	-	Each Obstacle in the World.
	-	The control space of each PathObject in the World that is configured to lock its control spaces.
	-	Each other spawning point selected by the SpawnManager.
	\section spawnmgrsetup Setting Up the SpawnManager
	To set up the SpawnManager in your World, call the World::ActivateService() method. For example:
	\code Kaim::SpawnManager* spawnmgr = m_world->ActivateService<Kaim::SpawnManager>(); \endcode
	\section spawnmgrgetpoints Finding valid positions
	To find valid positions in a defined area of the terrain:
	-#	Create a new instance of the SpawnQuery class, and set it up as described in the SpawnQuery class
		description.
	-#	Call GetSpawnPositions(), and pass your SpawnQuery object as a parameter.
	-#	Retrieve the generated spawn points from the SpawnQuery::m_result_buffer member of your SpawnQuery object.
	\section spawnmgrtest Validating a potential spawning position
	If you have a potential spawning position and you want to test whether or not it is clear of obstacles, call
	IsSpawnPointValid(). See the method description below.
	\ingroup kyruntime_spawnmanagement */
class SpawnManager : public Kaim::WorldService
{
public:
	/*!	Used to indicate the result of a call to GetSpawnPositions(). */
	enum QueryResult
	{
		SpawnQueryResult_Done = 0,			/*!< Indicates that the calculations have been completed successfully. */
		SpawnQueryResult_BufferOverflow,	/*!< Indicates that the calculations have produced more potential spawning points than the capacity of the array provided in the SpawnQuery. This case should never occur. */
		SpawnQueryResult_Working,			/*!< Indicates that the calculations are currently in progress. */
		SpawnQueryResult_Error,				/*!< Indicates that an internal error has prevented the calculations from being completed successfully. Check the log for error messages. */
		SpawnQueryResult_dummy_forceenumSize = KYFORCEENUMSIZEINT
	};


public:
	KY_DECLARE_WORLDSERVICE(SpawnManager)


public:
	/*!	\label_baseconstructor For internal use.
		\param world			The World that will contain this SpawnManager. */
	SpawnManager(World& world);

	/*!	\label_virtualdestructor */
	~SpawnManager();

	/*!	\label_reinit */
	void ReInit();

	/*!	\copydoc WorldService::Update()
		This implementation is empty. The SpawnManager does not conduct any operations at each frame; it only responds to
		the queries that you make through its IsSpawnPointValid() and GetSpawnPositions() methods. */
	void Update() {}

	//-------------------------------------------------
	// Public interface called by users
	//-------------------------------------------------
	/*!	Indicates whether or not a specified position is a valid spawning point for a character.
		\param position					The position to test.
		\param interEntityMinDistance	The minimum distance, in game units, that must exist between the 
										spawning point and all other nearby \Bodies and Obstacles in order 
										for the point to be considered valid. 
		\param databaseName				The name of the Database that contains the PathData used to validate the point.
										If not specified, the first Database added to the DatabaseManager is used.
		\return #KY_TRUE if the specified point is valid, or #KY_FALSE otherwise. */
	KyBool IsSpawnPointValid(const Vec3f& position, KyFloat32 interEntityMinDistance, const char* databaseName=KY_NULL);
	
	/*!	Retrieves a list of valid positions that may be used as spawning points for new characters. These 
		positions are selected at random within a set of volumes specified in the \c query.
		\param[in,out] query			A SpawnQuery object that contains input parameters for the query and
										stores the list of spawning points found by this method. See SpawnQuery.
		\return An element of the #QueryResult enumeration that indicates the results of the calculation. */
	QueryResult GetSpawnPositions(SpawnQuery& query);

protected:
	KyBool CollectNavTriangles(SpawnQuery& query);
	KyBool CollectVolumeTriangles(IVolume* controlVolume, IVolume* spawnVolume);

	QueryResult GenerateSpawnPositions(SpawnQuery& query);
	QueryResult GenerateSpawnPositions(SpawnQuery& query, const Vec3f* trianglePoints);
	KyBool AdjustSpawnCandidate(SpawnQuery& query, Vec3f& candidate, const Vec3f* trianglePoints);


	KyBool IntersectsAabbs(IVolume* volume0, IVolume* volume1);
	KyBool IntersectVolumeNavTriangle(IVolume* volume, NavTrianglePtr navTrianglePtr);

	void InsertTrianglePtr(NavTrianglePtr navTrianglePtr, List<NavTrianglePtr>& trianglePtrList);

	KyBool GetAabbFromVolume(IVolume* volume, Vec3f& aabbMin, Vec3f& aabbMax);
	KyBool CollectVolumeTriangles(IVolume* spawnVolume);
	KyBool IntersectVolumeTriangle(IVolume* volume, const Vec3f* trianglePoints);
	KyBool IsInsideVolume(IVolume* volume, const Vec3f& position);


protected:
	List<NavTrianglePtr> m_tmpTriangleList;
	List<NavTrianglePtr> m_finalTriangleList;
	NavMeshLayerLocationList m_tmpLayerLocations;
	NavMeshQueryServerWrapper* m_navMeshQueryServer;
	const NavMeshManager* m_navMeshManager;
};

} // namespace Kaim

#endif // KY_SPAWNMANAGER_H
