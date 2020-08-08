/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathobjects */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_RUNTIMEPATHOBJECTTOPOLOGY_H
#define KY_RUNTIMEPATHOBJECTTOPOLOGY_H

#include <kypathdata/containers/list.h>
#include "kypathdata/pathobject/pathobjectdatatypes.h"
#include "kypathdata/containers/array.h"


namespace Kaim
{

class Vec3f;
class PathObject;
class GraphVertexSafePtr;
class NavMeshLayerManager;
class RuntimePathObjectConnexionSpace;
class RuntimePathObjectControlSpace;
class RuntimePathObjectConnexionLink;
class PointInfo_ControlSpace;
class PointInfo_ConnexionSpace;
class IPathObjectTopology;


/*! A runtime representation of a DynamicPathObjectTopology, created transparently when a PathObject
	is added to or removed from a World. The methods specific to this class are mostly intended for internal
	use.
	\ingroup kyruntime_pathobject */
class RuntimePathObjectTopology
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	RuntimePathObjectTopology();
	~RuntimePathObjectTopology();


	//////////////////////////////////////////////////////////////////////////
	// Initialization
	//////////////////////////////////////////////////////////////////////////

	void Init();
	void AttachToPathObject(PathObject* pathObject) { m_pathObject = pathObject; }
	void DetachFromPathObject() { m_pathObject = KY_NULL; }
	void Reset();


	//////////////////////////////////////////////////////////////////////////
	// Misc
	//////////////////////////////////////////////////////////////////////////

	/*! Read accessor for #m_userPathObjectTopology. */
	IPathObjectTopology* GetUserPathObjectTopology() { return m_userPathObjectTopology; }

	/*! Returns a pointer to the PathObject using this topology. */
	PathObject* GetPathObject() const { return m_pathObject; }

	/*! \copydoc IPathObjectTopology::CanDetectAccident()
		This implementation returns the value stored in #m_canDetectAccident. */
	bool CanDetectAccident() const { return m_canDetectAccident; }

	/*! Renders debugging information according to the flags in the \c renderFlags bitmask.
		-	If \c renderFlags includes the #PathObjectRender_ControlSpaces flag, the
			RuntimePathObjectControlSpace::Render() method of each control space is called, and is passed
			the same \c renderFlags value.
		-	If \c renderFlags includes the #PathObjectRender_ConnexionSpaces flag, the
			RuntimePathObjectConnexionSpace::Render() method of each connection space is called, and is passed
			the same \c renderFlags value. */
	void Render(KyUInt32 renderFlags) const;


	//////////////////////////////////////////////////////////////////////////
	// Spaces management
	//////////////////////////////////////////////////////////////////////////

	void SetUp(IPathObjectTopology* topology);

	void DeleteRegisteredConnexionLinks();
	void DeleteRegisteredConnexionSpaces();
	void DeleteRegisteredControlSpaces();

	KyUInt32 GetConnexionLinkCount() const;
	RuntimePathObjectConnexionLink* GetConnexionLink(const PathObjectConnexionLinkIdx& idx) const;
	
	KyUInt32 GetConnexionSpaceCount() const;
	RuntimePathObjectConnexionSpace* GetConnexionSpace(const PathObjectConnexionSpaceIdx& idx) const;

	KyUInt32 GetControlSpaceCount() const;
	RuntimePathObjectControlSpace* GetControlSpace(const PathObjectControlSpaceIdx& idx) const;

	PathObjectControlSpaceIdx AddControlSpace(RuntimePathObjectControlSpace* controlSpace);
	PathObjectConnexionSpaceIdx AddConnexionSpace(RuntimePathObjectConnexionSpace* connexionSpace);
	PathObjectConnexionLinkIdx AddConnexionLink(RuntimePathObjectConnexionLink* connexionLink);

	RuntimePathObjectConnexionSpace* GetConnexionSpaceFromVertexSafePtr(const GraphVertexSafePtr& vertexSafePtr);


	//////////////////////////////////////////////////////////////////////////
	// Localization
	//////////////////////////////////////////////////////////////////////////

	KyResult UpdatePointInfos(const Vec3f& position, PointInfo_ConnexionSpace* connexionSpacePI, PointInfo_ControlSpace* controlSpacePI);
	/*! Returns #KY_TRUE if the specified position is inside any control space or connection space set up for this topology. */
	KyBool IsInside(const Vec3f& position);
	/*! Returns #KY_TRUE if the specified position is inside any control space set up for this topology. */
	KyBool IsInsideControlSpace(const Vec3f& position);


	//////////////////////////////////////////////////////////////////////////
	// Obstacle management
	//////////////////////////////////////////////////////////////////////////

	KyResult AttachToLayerManager(NavMeshLayerManager* layerManager);
	KyResult DetachFromLayerManager(NavMeshLayerManager* layerManager);
	KyResult RegisterProjectionVolumesToLayerManager(NavMeshLayerManager* layerManager);
	KyResult UnregisterProjectionVolumesFromLayerManager(NavMeshLayerManager* layerManager);

public:
	Array<RuntimePathObjectConnexionSpace*> m_connexionSpaces; /*!< The list of connection spaces. Do not modify directly. */
	Array<RuntimePathObjectControlSpace*> m_controlSpaces; /*!< The list of control spaces. Do not modify directly. */
	Array<RuntimePathObjectConnexionLink*> m_connexionLinks; /*!< The list of connection links. Do not modify directly. */
	bool m_canDetectAccident; /*!< Determines the value returned by CanDetectAccident(). */
	IPathObjectTopology* m_userPathObjectTopology; /*!< Stores a pointer to the IPathObjectTopology used to construct this object. */

protected:
	PathObject* m_pathObject;
};

} // namespace Kaim


#endif // #ifndef KY_RUNTIMEPATHOBJECTTOPOLOGY_H
