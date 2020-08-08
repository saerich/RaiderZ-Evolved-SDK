/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathobjects */

// primary contact: LAPA - secondary contact: MAMU
#ifndef KY_PATHOBJECTMANAGER_H
#define KY_PATHOBJECTMANAGER_H

#include <kyruntime/core/service.h>

#include <kyruntime/pathobjects/runtimepathobjecttopology/runtimepathobjectconnexionspace.h>
#include <kyruntime/pathobjects/runtimepathobjecttopology/runtimepathobjectcontrolspace.h>

#include <kypathdata/containers/list.h>

namespace Kaim
{

class DatabaseManager;
class Bot;
class GraphManager;
class IPathObject;
class PathObject;
class GraphEdge;
class Database;
class AdditionalGraph;
class GraphCell;
class PointInfo_ConnexionSpace;
class PointInfo_ControlSpace;


///////////////////////////////////////////////////////////////////////////////////////////
// PathObjectManager
///////////////////////////////////////////////////////////////////////////////////////////

/*!	This PathObjectManager is a class of WorldService that manages the \PathObjects that have been attached to the World.
	\pp In most cases, you will not need to use this class directly. Once you configure this Service for your 
	World, its use should be entirely transparent. Instead, you interact primarily with your \PathObjects 
	themselves, which in turn invoke this Service when required. 
	\pp See :USERGUIDE:"Using PathObjects for Special Movements".
	\ingroup kyruntime_pathobjects */
class PathObjectManager: public WorldService
{
	friend class IPathObject;

public:
	////////////////////////////////////////////////////////////////////////
	// CallBacks
	////////////////////////////////////////////////////////////////////////
	struct CbRecord
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
	public:
		CbRecord(): m_cb(0), m_userData(0) {}
		typedef void (*OnRemovePathObjectCB)(IPathObject* po, Database* db, void* userData);
		CbRecord(OnRemovePathObjectCB cb, void* userData): m_cb(cb), m_userData(userData) {}
		bool operator == (const CbRecord& rhs) const { return m_cb == rhs.m_cb && m_userData == rhs.m_userData; }
	public:
		OnRemovePathObjectCB m_cb;
		void* m_userData;
	};

public:
	KY_DECLARE_WORLDSERVICE(PathObjectManager)

public:
	/*! \label_baseconstructor */
	explicit PathObjectManager(World& world);

	/*! \label_baseconstructor */
	PathObjectManager();

	/*! \label_virtualdestructor */
	~PathObjectManager();

	/*! Called transparently when the world is destroyed.
		Removes all remaining registered PathObjects from the world.
		IMPORTANT: It is highly recommended for the user to remove himself all PathObjects from the world before destroying it! */
	void Clear();

	/*! Called transparently when a PathObject is attached to the World. For internal use. */
	KyResult AddPathObject(IPathObject* pathObject);

	/*! Called transparently when a PathObject is removed from the World. For internal use. */
	void RemovePathObject(IPathObject* pathObject);

	/*! Saves the current set of data maintained by all PathObjects in the specified directory. The generated files can
		be dragged and dropped into the \SDKName Lab to visualize the data. */
	KyResult DumpAllPathObjects(const char* targetDirectory) const;

	/*! Retrieves the list of \PathObjects attached to the World. For internal use. */
	const List<IPathObject*>& GetPathObjects() const;

	virtual void ReInit();
	virtual void Update();

	/*! Calls PathObject::Render() for each managed PathObject. */
	virtual void Render() const;

	/*! Defines a type of callback function that can be called each time a PathObject is removed from the World.
		This callback function must be registered through a call to RegisterPathObjectRemovalCB().
		\param po				A pointer to the PathObject being removed.
		\param db				The PathData Database passed in the \c db parameter of RegisterPathObjectRemovalCB().
		\param userData			A void pointer passed in the \c userData parameter of RegisterPathObjectRemovalCB(). */
	typedef void (*OnRemovePathObjectCB)(IPathObject* po, Database* db, void* userData);

	/*! Registers a callback function that will be invoked each time a PathObject is removed from the World.
		You can use this callback to receive notification of new deactivations, or to perform any additional
		steps that must be taken in order to remove the PathObject from your \gameOrSim.
		\param cb				The callback function to invoke.
		\param userData			A void pointer passed to your callback function in its \c userData parameter.*/
	KyBool RegisterPathObjectRemovalCB(OnRemovePathObjectCB cb, void* userData);

	/*! Unregisters a callback function previously registered using RegisterPathObjectRemovalCB().
		\param cb				The callback function to invoke.
		\param userData			The void pointer passed in the original call to RegisterPathObjectRemovalCB().*/
	KyBool UnregisterPathObjectRemovalCB(OnRemovePathObjectCB cb, void* userData);

	/*! Called transparently when a PathObject starts moving. */
	void OnPathObjectStartsMoving(IPathObject* /*pathObject*/) {}

	/*! Called transparently when a PathObject stops moving. */
	void OnPathObjectEndsMoving(IPathObject* /*pathObject*/) {}

	/*! Called transparently when an AdditionalGraph is about to be removed from Database. */
	static void OnRemoveAdditionalGraphCB(const AdditionalGraph& additionalGraph, Database& db, void* userData);

	/*! For internal use. */
	SharedPoolList<RuntimePathObjectConnexionSpace*>::Pool* GetConnexionSpacePointerPool() { return &m_connexionSpacePointerPool; }

	/*! For internal use. */
	SharedPoolList<RuntimePathObjectControlSpace*>::Pool* GetControlSpacePointerPool() { return &m_controlSpacePointerPool; }

	/*! For internal use. */
	SharedPoolList<Bot*>::Pool* GetBotPointerPool() { return &m_botPointerPool; }

	/*! For internal use. */
	KyResult UpdatePointInfos(const Vec3f& position, PointInfo_ConnexionSpace* connexionSpacePI, PointInfo_ControlSpace* controlSpacePI);

private:
	void OnPathObjectRemoved(IPathObject* pathObject);

protected:
	SharedPoolList<RuntimePathObjectConnexionSpace*>::Pool m_connexionSpacePointerPool;
	SharedPoolList<RuntimePathObjectControlSpace*>::Pool m_controlSpacePointerPool;
	SharedPoolList<Bot*>::Pool m_botPointerPool;
	List<IPathObject*> m_pathObjects; /*< List of registered PathObjects */
	List<CbRecord> m_poRemovalCallBacks;
};

} // namespace Kaim

#endif // KY_PATHOBJECTMANAGER_H
