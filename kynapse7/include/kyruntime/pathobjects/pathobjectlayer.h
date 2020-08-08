/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathobjects */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_PATHOBJECTLAYER_H
#define KY_PATHOBJECTLAYER_H

#include <kyruntime/mesh/obstaclelayer.h>


namespace Kaim
{

class ObstacleAccessor_PathObjectOutline;


/*! This implementation of ObstacleLayer is used internally to represent the control spaces of blocking \PathObjects.
	When a blocking PathObject is added to the World, the NavMeshLayerManager transparently creates and manages an
	instance of this class, accessible through NavMeshLayerManager::GetPathObjectLayer(). All blocking \PathObjects
	are projected into this PathObjectLayer, which is transparently set up as a blocking layer for the CanGo_NavMesh 
	modifier so that \Bots will not be able to smooth their paths through the control spaces of the blocking PathObjects. 
	You can also take the PathObjectLayer into account in perception queries that you make through the 
	NavMeshQueryServerWrapper.
	\ingroup kyruntime_pathobjects */
class PathObjectLayer : public ObstacleLayer
{
public:
	KY_DECLARE_NAVMESHLAYER(PathObjectLayer);


public:
	/*! \label_constructor For internal use. */
	PathObjectLayer(NavMeshLayerManager& manager, const char* name);

	/*! \label_virtualdestructor For internal use. */
	virtual ~PathObjectLayer();


	/*! For internal use. */
	void AddDatabase(Database* database);

	/*! Retrieves the instance of ObstacleAccessor_PathObjectOutline used by the PathObjectLayer to retrieve the
		outlines of blocking \PathObjects. For internal use. */
	ObstacleAccessor_PathObjectOutline* GetPathObjectOutlineAccessor() { return m_poOutlineAccessor;}


protected:
	ObstacleAccessor_PathObjectOutline* m_poOutlineAccessor;
};

} // namespace Kaim

#endif // KY_PATHOBJECTLAYER_H
