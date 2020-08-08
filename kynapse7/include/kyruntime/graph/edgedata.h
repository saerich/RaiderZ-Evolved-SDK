/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyRuntime_EdgeData_H
#define KyRuntime_EdgeData_H

// primary contact: MAMU - secondary contact: NOBODY
#include <kypathdata/generic/memory.h>

namespace Kaim
{

/* For internal use. */
class EdgeData
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	EdgeData() { Clear(); }
	void Clear() { m_lockCount = 0; }

	void IncreaseLockCount() { m_lockCount++; }
	void DecreaseLockCount();

	bool IsLocked() const { return (m_lockCount != 0); }
	bool IsNotLocked() const { return (m_lockCount == 0); }

private:
	KyUInt32 m_lockCount;
};

} // namespace Kaim

#endif
