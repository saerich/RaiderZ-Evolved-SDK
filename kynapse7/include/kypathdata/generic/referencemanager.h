/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: LAPA - secondary contact: NOBODY
#ifndef KyPathData_ReferenceManager_H
#define KyPathData_ReferenceManager_H


#include <kypathdata/containers/list.h>


namespace Kaim
{

///////////////////////////////////////////////////////////////////////////////////////////
// ReferenceManager
///////////////////////////////////////////////////////////////////////////////////////////

/* Used internally to manage references to a specific type. */
template <typename Reference>
class ReferenceManager
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*!	\label_baseconstructor */
	ReferenceManager() {}
	
	/*!	\label_virtualdestructor */
	~ReferenceManager() { Clear(); }

	void Clear()
	{
		m_references.Clear();
	}

	/*! Create a new Reference. Should be called when adding a subData is ReferenceManager */
	Reference* GetNewReference()
	{
		typename List<Reference>::Iterator referenceIt = m_references.Insert();
		Reference& reference = *referenceIt;
		return &reference;
	}

	/*! browse all the References and removed invalid ones that are no more used by an Uid (their nnumber of user is 0) */
	void ReleaseUnusedReferences()
	{
		for(typename List<Reference>::Iterator it = m_references.GetFirst(); it.IsValid(); )
		{
			Reference& reference = *it;

			if (reference.IsValid() || reference.IsStillUsed())
				++it;
			else
				it = m_references.Erase(it);
		}
	}
public:
	List<Reference> m_references;
};


} // namespace Kaim

#endif // KyPathData_ReferenceManager_H
