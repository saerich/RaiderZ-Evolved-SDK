/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_spawnmanagement */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_SPAWNQUERY_H
#define KY_SPAWNQUERY_H

#include <kypathdata/generic/types.h>

namespace Kaim
{

class Database;
class IVolume;
class Vec3f;
class SpawnManager;
class World;

/*!	This class is used in conjunction with the SpawnManager::GetSpawnPositions() method, 
	which retrieves random valid spawning points within specified areas. 
	\pp \section spawninputs Setting inputs
	This class sets up the information required by SpawnManager::GetSpawnPositions() to carry out its
	calculations. Before providing SpawnManager::GetSpawnPositions() with an instance of this class,
	you must set up your SpawnQuery with the following information:
	-	The NavMesh that represents the valid area for spawning points. See SetDatabase().
	-	An optional overall bounding box for the query. See SetControlVolume().
	-	One or more areas that contain NavMesh triangles within which the spawning points will be selected 
		at random. See SetSpawnVolumes().
	-	The minimum distance that each spawning point must be from all \Bodies, Obstacles and other
		spawning points. See SetMinDistance().
	-	A memory buffer that will be used to store the spawning points selected by 
		SpawnManager::GetSpawnPositions(). See SetResultBuffer().
	\pp \section spawnoutputs Retrieving outputs
	After you pass this object to SpawnManager::GetSpawnPositions(), the list of spawning positions selected by
	SpawnManager::GetSpawnPositions() is stored in #m_result_buffer. You can retrieve the generated spawning points
	directly from this array. The number of spawning points in the buffer is indicated by #m_result_count. 
	\ingroup kyruntime_spawnmanagement */
class SpawnQuery
{
public:
	/*!	\label_constructor */
	SpawnQuery();

	/*!	\label_destructor */
	~SpawnQuery();

	/*!	Resets the SpawnQuery to its default state, clearing all data members and nullifying
		all pointers. */
	void Reset();

	// Input management

	/*!	Sets the PathData Database for this SpawnQuery. The SpawnManager will use this Database
		to retrieve the NavMesh within which it will search for valid spawning points. If you do not
		set a Database before calling SpawnManager::GetSpawnPositions(), the first Database added to the
		DatabaseManager is used by default. */
	bool SetDatabase(World& world, const char* databaseName);

	/*!	Sets the minimum distance, in meters, that must exist between each spawning point and:
		-	Each Body in the World.
		-	Each Obstacle in the World.
		-	The control space of each PathObject in the World that is configured to lock its control spaces.
		-	Each other spawning point selected by SpawnManager::GetSpawnPositions(). */
	bool SetMinDistance(KyFloat32 value);
	
	/*!	Retrieves the minimum distance, in meter, that must exist between each spawning point and:
		-	Each Body in the World.
		-	Each Obstacle in the World.
		-	The control space of each PathObject in the World that is configured to lock its control spaces.
		-	Each other spawning point selected by SpawnManager::GetSpawnPositions(). */
	KyFloat32 GetMinDistance() const {return m_minDistance;}

	/*!	Sets an optional global bounding box for the search. All spawning points are guaranteed to be 
		within this bounding box.
		\return \c true if the bounding box was successfully set, or \c false otherwise. A \c false return typically
				indicates that a control volume has already been set. You can reset it through a call to Reset(). */
	bool SetControlVolume(IVolume* newControlVolume);
	
	/*!	Retrieves the global bounding box for the search, or #KY_NULL if no global bounding box has been set. */
	IVolume* GetControlVolume() {return m_controlVolume;}

	/*!	Sets one or more volumes within which spawning points will be selected. Only NavMesh triangles that
		lie within at least one of these areas will be taken into account by the SpawnManager. Your volumes 
		must therefore include the altitude of the NavMesh that represents the area within which you want to search,
		which is typically approximately the level of the ground or floor from which the NavMesh was originally
		generated.
		\param spawnVolumes				An array of objects that define volumes in 3D space.
		\param spawnVolumeCount			The number of elements in the \c spawnVolumes array.
		\return true if the spawn volumes were successfully set, or false otherwise. A false return typically
				indicates that at least one spawn volume has already been set. You can reset the list of spawn
				volumes through a call to ResetSpawnVolumes() or Reset(). */
	bool SetSpawnVolumes(IVolume** spawnVolumes, KyUInt32 spawnVolumeCount);
	
	/*!	Resets the list of spawn volumes. 
		\return true if the list of spawn volumes were successfully reset. */
	bool ResetSpawnVolumes();

	// Output management

	/*!	Specifies the address and size of the memory buffer used by this SpawnQuery to store the spawning points
		found by SpawnManager::GetSpawnPositions().
		\param buffer					The address of the memory buffer in which the generated list of
										spawning points will be stored.
		\param capacity					The maximum number of Vec3f objects that can be stored in \c buffer.
		\return \c true if the new buffer was successfully recorded. */
	bool SetResultBuffer(Vec3f* buffer, KyUInt32 capacity);
	
	/*!	Retrieves the number of spawning points generated during the last call to SpawnManager::GetSpawnPositions()
		and stored in SpawnQuery::m_result_buffer. */
	KyUInt32 GetResultCount() {return m_result_count;}

	void Render();


public:
	// Inputs
	Database* m_database; /*!< The Database set by SetDatabase(). Do not modify directly. */
	IVolume* m_controlVolume; /*!< The optional global bounding box set by SetControlVolume(). Do not modify directly. */ // optional overall volume
	IVolume** m_spawnVolumes; /*!< The list of spawn volumes set by SetSpawnVolumes(). Do not modify directly. */
	KyUInt32 m_spawnVolumeArraySize; /*!< The number of volumes in #m_spawnVolumes. Do not modify directly. */
	KyFloat32 m_minDistance; /*!< The minimum distance between spawn points set by SetMinDistance(). Do not modify directly. */

	// Outputs
	Vec3f* m_result_buffer; /*!< The list of spawning points selected by the last call to SpawnManager::GetSpawnPositions(). */
	KyUInt32 m_result_capacity; /*!< The maximum number of Vec3f objects that can be held in the #m_result_buffer array. */
	KyUInt32 m_result_count; /*!< The number of spawning points written to the #m_result_buffer by the last call to SpawnManager::GetSpawnPositions(). */
};

} // namespace Kaim

#endif // KY_SPAWNQUERY_H
