/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_DATABASEMANAGERCALLBACKS_H
#define KY_DATABASEMANAGERCALLBACKS_H

#include <kypathdata/generic/memory.h>
#include <kypathdata/containers/list.h>

namespace Kaim
{

class Graph;
class AdditionalGraph;
class NavMesh;
class Database;

/*! Used internally by the DatabaseManager to manage the lists of callback functions it should invoke when a Graph or NavMesh
	is added to or removed from a Database. */
class DatabaseManagerCallBacks
{
	KY_DEFINE_NEW_DELETE_OPERATORS

private:
	friend class DatabaseManager;

	/*! \label_constructor For internal use. */
	DatabaseManagerCallBacks() {}

public:
	/*! \label_clear */
	void Clear();

	/*!	Defines a type for a callback function that you can use to process notifications each time 
		the DatabaseManager loads a new Graph into a Database. See also DatabaseManager::RegisterOnAddGraphCB().
		\param graph		the Graph that has been added or that is about to be removed.
		\param db			the Database the Graph has been added to or removed from.
		\param userData		A void pointer that corresponds to the \c userData argument provided to the
							RegisterOnAddGraphCB() method when you added your callback function. */
	typedef void (*OnAddGraphCB)(const Graph& graph, Database& db, void* userData);

	/*!	Defines a type for a callback function that you can use to process notifications each time 
		the DatabaseManager removes a Graph from a Database. See also DatabaseManager::RegisterOnRemoveGraphCB().
		\param graph		the Graph that has been added or that is about to be removed.
		\param db			the Database the Graph has been added to or removed from.
		\param userData		A void pointer that corresponds to the \c userData argument provided to the
							RegisterOnRemoveGraphCB() method when you added your callback function. */
	typedef void (*OnRemoveGraphCB)(const Graph& graph, Database& db, void* userData);

	/*!	Defines a type for a callback function that you can use to process notifications each time 
		the DatabaseManager loads a new AdditionalGraph into a Database. See also DatabaseManager::RegisterOnAddAdditionalGraphCB().
		\param additionalGraph		the AdditionalGraph that has been added or that is about to be removed.
		\param db			the Database the AdditionalGraph has been added to or removed from.
		\param userData		A void pointer that corresponds to the \c userData argument provided to the
							RegisterOnAddAdditionalGraphCB() method when you added your callback function. */
	typedef void (*OnAddAdditionalGraphCB)(const AdditionalGraph& additionalGraph, Database& db, void* userData);

	/*!	Defines a type for a callback function that you can use to process notifications each time 
		the DatabaseManager removes a AdditionalGraph from a Database. See also DatabaseManager::RegisterOnRemoveAdditionalGraphCB().
		\param additionalGraph		the AdditionalGraph that has been added or that is about to be removed.
		\param db			the Database the AdditionalGraph has been added to or removed from.
		\param userData		A void pointer that corresponds to the \c userData argument provided to the
							RegisterOnRemoveAdditionalGraphCB() method when you added your callback function. */
	typedef void (*OnRemoveAdditionalGraphCB)(const AdditionalGraph& additionalGraph, Database& db, void* userData);

	/*!	Defines a type for a callback function that you can use to process notifications each time 
		the DatabaseManager loads a new NavMesh into a Database. See also DatabaseManager::RegisterOnAddNavMeshCB().
		\param navmesh		the NavMesh that has been added or that is about to be removed.
		\param db			the Database the NavMesh has been added to or removed from.
		\param userData		A void pointer that corresponds to the \c userData argument provided to the
							RegisterOnAddNavMeshCB() method when you added your callback function. */
	typedef void (*OnAddNavMeshCB)(const NavMesh& navmesh, Database& db, void* userData);

	/*!	Defines a type for a callback function that you can use to process notifications each time 
		the DatabaseManager removes a NavMesh from a Database. See also DatabaseManager::RegisterOnRemoveNavMeshCB().
		\param navmesh		the NavMesh that has been added or that is about to be removed.
		\param db			the Database the NavMesh has been added to or removed from.
		\param userData		A void pointer that corresponds to the \c userData argument provided to the
							RegisterOnRemoveNavMeshCB() method when you added your callback function. */
	typedef void (*OnRemoveNavMeshCB)(const NavMesh& navmesh, Database& db, void* userData);


	/*!	Called transparently by DatabaseManager::RegisterOnAddGraphCB(). */
	KyBool RegisterOnAddGraphCB(OnAddGraphCB cb, void* userData);

	/*!	Called transparently by DatabaseManager::RegisterOnRemoveGraphCB(). */
	KyBool RegisterOnRemoveGraphCB(OnRemoveGraphCB cb, void* userData);

	/*!	Called transparently by DatabaseManager::RegisterOnAddAdditionalGraphCB(). */
	KyBool RegisterOnAddAdditionalGraphCB(OnAddAdditionalGraphCB cb, void* userData);

	/*!	Called transparently by DatabaseManager::RegisterOnRemoveAdditionalGraphCB(). */
	KyBool RegisterOnRemoveAdditionalGraphCB(OnRemoveAdditionalGraphCB cb, void* userData);

	/*!	Called transparently by DatabaseManager::RegisterOnAddNavMeshCB(). */
	KyBool RegisterOnAddNavMeshCB(OnAddNavMeshCB cb, void* userData);

	/*!	Called transparently by DatabaseManager::RegisterOnRemoveNavMeshCB(). */
	KyBool RegisterOnRemoveNavMeshCB(OnRemoveNavMeshCB cb, void* userData);


	/*!	Called transparently by DatabaseManager::UnRegisterOnAddGraphCB(). */
	KyBool UnRegisterOnAddGraphCB(OnAddGraphCB cb, void* userData);

	/*!	Called transparently by DatabaseManager::UnRegisterOnRemoveGraphCB(). */
	KyBool UnRegisterOnRemoveGraphCB(OnRemoveGraphCB cb, void* userData);

	/*!	Called transparently by DatabaseManager::UnRegisterOnAddAdditionalGraphCB(). */
	KyBool UnRegisterOnAddAdditionalGraphCB(OnAddAdditionalGraphCB cb, void* userData);

	/*!	Called transparently by DatabaseManager::UnRegisterOnRemoveAdditionalGraphCB(). */
	KyBool UnRegisterOnRemoveAdditionalGraphCB(OnRemoveAdditionalGraphCB cb, void* userData);

	/*!	Called transparently by DatabaseManager::UnRegisterOnAddNavMeshCB(). */
	KyBool UnRegisterOnAddNavMeshCB(OnAddNavMeshCB cb, void* userData);

	/*!	Called transparently by DatabaseManager::UnRegisterOnRemoveNavMeshCB(). */
	KyBool UnRegisterOnRemoveNavMeshCB(OnRemoveNavMeshCB cb, void* userData);


	/*!	Called transparently by DatabaseManager::CallOnAddGraphCBs(). */
	void CallOnAddGraphCBs(const Graph& graph, Database& db) const
	{
		for (List<CbRecord<OnAddGraphCB> >::ConstIterator it = m_onAddGraphCallbacks.GetFirst(); it.IsValid(); ++it)
			it->m_cb(graph, db, it->m_userData);
	}

	/*!	Called transparently by DatabaseManager::CallOnRemoveGraphCBs(). */
	void CallOnRemoveGraphCBs(const Graph& graph, Database& db) const
	{
		for (List<CbRecord<OnRemoveGraphCB> >::ConstIterator it = m_onRemoveGraphCallbacks.GetFirst(); it.IsValid(); ++it)
			it->m_cb(graph, db, it->m_userData);
	}


	/*!	Called transparently by DatabaseManager::CallOnAddAdditionalGraphCBs(). */
	void CallOnAddAdditionalGraphCBs(const AdditionalGraph& additionalAdditionalGraph, Database& db) const
	{
		for (List<CbRecord<OnAddAdditionalGraphCB> >::ConstIterator it = m_onAddAdditionalGraphCallbacks.GetFirst(); it.IsValid(); ++it)
			it->m_cb(additionalAdditionalGraph, db, it->m_userData);
	}

	/*!	Called transparently by DatabaseManager::CallOnRemoveAdditionalGraphCBs(). */
	void CallOnRemoveAdditionalGraphCBs(const AdditionalGraph& additionalAdditionalGraph, Database& db) const
	{
		for (List<CbRecord<OnRemoveAdditionalGraphCB> >::ConstIterator it = m_onRemoveAdditionalGraphCallbacks.GetFirst(); it.IsValid(); ++it)
			it->m_cb(additionalAdditionalGraph, db, it->m_userData);
	}


	/*!	Called transparently by DatabaseManager::CallOnAddNavMeshCBs(). */
	void CallOnAddNavMeshCBs(const NavMesh& navmesh, Database& db) const
	{
		for (List<CbRecord<OnAddNavMeshCB> >::ConstIterator it = m_onAddNavmeshCallbacks.GetFirst(); it.IsValid(); ++it)
			it->m_cb(navmesh, db, it->m_userData);
	}

	/*!	Called transparently by DatabaseManager::CallOnRemoveNavMeshCBs(). */
	void CallOnRemoveNavMeshCBs(const NavMesh& navmesh, Database& db) const
	{
		for (List<CbRecord<OnRemoveNavMeshCB> >::ConstIterator it = m_onRemoveNavmeshCallbacks.GetFirst(); it.IsValid(); ++it)
			it->m_cb(navmesh, db, it->m_userData);
	}


protected:

	~DatabaseManagerCallBacks();

	template <typename CB> struct CbRecord: public VirtualMemObject
	{
		CbRecord(): m_cb(0), m_userData(0) {}
		CbRecord(CB cb, void* userData): m_cb(cb), m_userData(userData) {}
		CB m_cb;
		void* m_userData;
	};

protected:
	List<CbRecord<OnAddGraphCB> > m_onAddGraphCallbacks;
	List<CbRecord<OnRemoveGraphCB> > m_onRemoveGraphCallbacks;

	List<CbRecord<OnAddAdditionalGraphCB> > m_onAddAdditionalGraphCallbacks;
	List<CbRecord<OnRemoveAdditionalGraphCB> > m_onRemoveAdditionalGraphCallbacks;

	List<CbRecord<OnAddNavMeshCB> > m_onAddNavmeshCallbacks;
	List<CbRecord<OnRemoveNavMeshCB> > m_onRemoveNavmeshCallbacks;
};

} // namespace Kaim

#endif // KY_DATABASEMANAGERCALLBACKS_H
