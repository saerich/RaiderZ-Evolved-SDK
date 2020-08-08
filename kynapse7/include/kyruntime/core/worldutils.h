/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_core */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_WORLDUTILS_H
#define KY_WORLDUTILS_H

#include <kyruntime/spawnmanagement/spawnmanager.h>
#include <kyruntime/spawnmanagement/spawnquery.h>


namespace Kaim
{

class World;

namespace WorldUtils
{
	/*!	This function provides a shortcut to invoke the SpawnManager::IsSpawnPointValid() method 
		for the SpawnManager within the specified World. See the SpawnManager::IsSpawnPointValid() description
		for details.
		\pp Note that you must have already set up the SpawnManager
		for the World. See the SpawnManager class description for details.
		\param world					The World whose SpawnManager will be invoked.
		\param position					Passed to the \c position parameter of SpawnManager::IsSpawnPointValid().
		\param interEntityMinDistance	Passed to the \c interEntityMinDistance parameter of SpawnManager::IsSpawnPointValid().
		\param databaseName				Passed to the \c databaseName parameter of SpawnManager::IsSpawnPointValid().
		\ingroup kyruntime_core */
	KyBool IsSpawnPointValid(World* world, const Vec3f& position, KyFloat32 interEntityMinDistance, const char* databaseName=KY_NULL);

	/*!	This function provides a shortcut to invoke the SpawnManager::GetSpawnPositions() method 
		for the SpawnManager within the specified World. See the SpawnManager::GetSpawnPositions() description
		for details.
		\pp Note that you must have already set up the SpawnManager
		for the World. See the SpawnManager class description for details.
		\param world					The World whose SpawnManager will be invoked.
		\param query					Passed to the \c query parameter of SpawnManager::GetSpawnPositions().
		\ingroup kyruntime_core */
	SpawnManager::QueryResult GetSpawnPositions(World* world, SpawnQuery& query);

} // namespace WorldUtils

} // namespace Kaim

#endif // KY_WORLDUTILS_H
