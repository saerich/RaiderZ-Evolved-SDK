/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// Primary contact: JUBA - secondary contact: MAMU
#ifndef KyPathData_DataReference_H
#define KyPathData_DataReference_H

#include "kypathdata/generic/memory.h"
#include "kypathdata/basesystem/logger.h"


namespace Kaim
{

template<class Data>
class DataReference
{
	KY_DEFINE_NEW_DELETE_OPERATORS

	DataReference& operator=(const DataReference&);

public:
	DataReference() : m_data(KY_NULL), m_numberOfUsers(0) {}

	~DataReference()
	{
		KY_FUNCTION("DataReference::~DataReference");
		KY_ASSERT(m_numberOfUsers == 0, ("You are destroying a GraphReference that is still referenced by someone. A crash is going to occur"));
		
		if (m_data != KY_NULL)
		{
			m_data->m_reference = KY_NULL;
			m_data = KY_NULL;
		}
	}

	KY_FORCE_INLINE void Invalidate() { m_data = KY_NULL; }
	KY_FORCE_INLINE bool IsValid() const { return m_data != KY_NULL; }

	/*
		functions used by DataPtr
	*/
	KY_FORCE_INLINE KyUInt32 GetRefCount() const { return m_numberOfUsers; }
	KY_FORCE_INLINE bool IsStillUsed() const { return GetRefCount() != 0; }
	KY_FORCE_INLINE void IncrementUserNumber() { ++m_numberOfUsers; }
	KY_FORCE_INLINE void DecrementUserNumber()
	{
		KY_FUNCTION("DataReference::DecrementUserNumber");
		KY_ASSERTK(m_numberOfUsers != 0);
		--m_numberOfUsers;
	}

	Data* GetData() const { return m_data; }
	void SetData(Data* data) { m_data = data; }

private:
	Data* m_data;
	KyUInt32 m_numberOfUsers;
};

}

#endif // #ifndef KyPathData_DataReference_H
