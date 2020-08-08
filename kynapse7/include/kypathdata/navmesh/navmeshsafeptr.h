/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_navmesh */

// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavMeshSafePtr_H
#define KyPathData_NavMeshSafePtr_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/navmesh/stitchednavmesh.h"


namespace Kaim
{

class NavMesh;


///////////////////////////////////////////////////////////////////////////////////////////
// NavMeshSafePtr
///////////////////////////////////////////////////////////////////////////////////////////

/*! Each instance of this class uniquely identifies a single NavMesh.
	\pp Instances of this class can be safely stored and used in subsequent frames.
	\pp When you are finished with an instance of this class, you must call Invalidate() to free its allocated resources. This
	must be done before you destroy the World.
	\ingroup kypathdata_navmesh */
class NavMeshSafePtr
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*!	\label_constructor Creates an invalid NavCellSafePtr. */
	KY_FORCE_INLINE NavMeshSafePtr(): m_navMeshReference(KY_NULL) {}

	/*!	\label_constructor
		\param navMeshReference		A pointer to the NavMeshReference associated with this NavMesh. */
	KY_FORCE_INLINE NavMeshSafePtr(NavMeshReference* navMeshReference) : m_navMeshReference(KY_NULL)
	{
		if (navMeshReference != KY_NULL)
			Set(*navMeshReference);
	}

	/*!	\label_constructor
		\param navMeshReference		A reference to the NavMeshReference associated with this NavMesh. */
	KY_FORCE_INLINE NavMeshSafePtr(NavMeshReference& navMeshReference) : m_navMeshReference(&navMeshReference) { navMeshReference.IncrementUserNumber(); }

	/*! \label_constructor Copies the state of the specified NavMeshSafePtr to the newly created object.  */
	NavMeshSafePtr(const NavMeshSafePtr& navMeshSafePtr) : m_navMeshReference(navMeshSafePtr.m_navMeshReference)
	{
		if (navMeshSafePtr.m_navMeshReference != KY_NULL)
			navMeshSafePtr.m_navMeshReference->IncrementUserNumber();
	}

	/*! Copies the state of the specified NavMeshSafePtr to this object.  */
	KY_FORCE_INLINE NavMeshSafePtr& operator=(const NavMeshSafePtr& navMeshSafePtr)
	{
		Set(navMeshSafePtr.m_navMeshReference);
		return *this;
	}

	/*! \label_destructor */
	~NavMeshSafePtr()
	{
		if (m_navMeshReference != KY_NULL)
			m_navMeshReference->DecrementUserNumber();
	}

	/*!	\label_reinit
		\param navMeshReference		A pointer to the NavMeshReference associated with this NavMesh. */
	KY_FORCE_INLINE void Set(NavMeshReference* navMeshReference)
	{
		if (m_navMeshReference != KY_NULL)
		{
			m_navMeshReference->DecrementUserNumber();
			m_navMeshReference = KY_NULL;
		}

		if (navMeshReference != KY_NULL)
			navMeshReference->IncrementUserNumber();

		m_navMeshReference = navMeshReference;
	}

	/*!	\label_constructor
		\param navMeshReference		A reference to the NavMeshReference associated with this NavMesh. */
	KY_FORCE_INLINE void Set(NavMeshReference& navMeshReference)
	{
		if (m_navMeshReference != KY_NULL)
			m_navMeshReference->DecrementUserNumber();

		m_navMeshReference = &navMeshReference;
		navMeshReference.IncrementUserNumber();
	}

	/*! Returns \c true if this object refers to a valid NavMesh: i.e. a NavMesh that is currently loaded in memory. */
	KY_FORCE_INLINE bool IsValid() const
	{
		if (m_navMeshReference != KY_NULL)
		{
			if (m_navMeshReference->IsValid())
				return true;

			m_navMeshReference->DecrementUserNumber();
			m_navMeshReference = KY_NULL;
		}

		return false;
	}

	/*! Invalidates this object. */
	KY_FORCE_INLINE void Invalidate()
	{
		if (m_navMeshReference != KY_NULL)
		{
			m_navMeshReference->DecrementUserNumber();
			m_navMeshReference = KY_NULL;
		}
	}

	/*!	Returns \c true if this object identifies the same NavMesh as \c navMeshSafePtr, or if both are invalid. */
	KY_FORCE_INLINE bool operator==(const NavMeshSafePtr& navMeshSafePtr) const { return m_navMeshReference == navMeshSafePtr.m_navMeshReference; }

	/*!	Returns \c true if this object identifies a NavMesh different from the one identified by \c navMeshSafePtr, or if only one of them is invalid. */
	KY_FORCE_INLINE bool operator!=(const NavMeshSafePtr& navMeshSafePtr) const { return m_navMeshReference != navMeshSafePtr.m_navMeshReference; }

	KY_FORCE_INLINE bool operator < (const NavMeshSafePtr& navMeshSafePtr) const { return m_navMeshReference < navMeshSafePtr.m_navMeshReference; }
	KY_FORCE_INLINE bool operator > (const NavMeshSafePtr& navMeshSafePtr) const { return m_navMeshReference > navMeshSafePtr.m_navMeshReference; }
	KY_FORCE_INLINE bool operator <=(const NavMeshSafePtr& navMeshSafePtr) const { return !(*this > navMeshSafePtr); }
	KY_FORCE_INLINE bool operator >=(const NavMeshSafePtr& navMeshSafePtr) const { return !(*this < navMeshSafePtr); }


	/*!	Returns a pointer to the StitchedNavMesh that handles this NavMesh.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE StitchedNavMesh* GetStitchedNavMesh() const { return IsValid() ? m_navMeshReference->GetData() : KY_NULL; }

	/*!	Returns a pointer to the NavMesh identified by this object.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const NavMesh* GetNavMesh() const { return IsValid() ? &m_navMeshReference->GetData()->m_navMesh : KY_NULL; }

	/*!	Returns a const reference to the StitchedNavMesh that handles this NavMesh.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE StitchedNavMesh& GetStitchedNavMesh_UnSafe() const { return *m_navMeshReference->GetData(); }

	/*!	Returns a const reference to the NavMesh identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavMesh& GetNavMesh_UnSafe() const { return m_navMeshReference->GetData()->m_navMesh; }

	/*!	Returns a pointer to the NavMeshReference associated with this NavMesh. */
	KY_FORCE_INLINE NavMeshReference* GetNavMeshReference() const { return m_navMeshReference; }

private :
	mutable NavMeshReference* m_navMeshReference;
};

}

#endif // KyPathData_NavMeshSafePtr_H

