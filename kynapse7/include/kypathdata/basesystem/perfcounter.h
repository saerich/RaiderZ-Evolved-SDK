/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	



#ifndef KyPathData_PerfCounter_H
#define KyPathData_PerfCounter_H


#include "kypathdata/generic/types.h"
#include <kypathdata/basesystem/basesystem.h>

namespace Kaim
{

/*	Used internally by the BaseSystem to store performance data. */
class PerfCounter
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	PerfCounter()
		:name(KY_NULL), value(0.f)
	{
	}

	explicit PerfCounter(const char* _name)
		:name(_name), value(0.f)
	{
	}

public:
	bool IsEnabled() const
	{
		return name != KY_NULL;
	}

public:
	const char* GetName() const
	{
		return name;
	}

	KyFloat32 GetValue() const
	{
		return value;
	}

	void SetValue(KyFloat32 _value)
	{
		if (IsEnabled())
		{
			value = _value;

			if (GetBaseSystem().GetBridge().HasPerfCounterBridge())
				GetBaseSystem().GetBridge().GetPerfCounterBridge()->AddCounter(name, value);
		}
	}

protected:
	const char* name;
	KyFloat32 value;
};

} // Kaim




#endif // KyPathData_PerfCounter_H

