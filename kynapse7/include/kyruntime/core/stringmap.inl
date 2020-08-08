/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_STRINGMAP_INL
#define KY_STRINGMAP_INL

#include <string.h>
#include <kypathdata/generic/types.h>
#include <kyruntime/core/stringmap.h>

namespace Kaim
{

template <typename FunctionType, KyUInt32 MaxNumber, typename UniqueType>
StringMap<FunctionType, MaxNumber, UniqueType>::~StringMap()
{
}

template <typename FunctionType, KyUInt32 MaxNumber, typename UniqueType>
FunctionType StringMap<FunctionType, MaxNumber, UniqueType>::GetFunctionByName(const char* functionName)
{
	FunctionHolder& holder = GetHolder();
	for (KyInt32 i=0;i<holder.m_funcCount;++i)
	{
		KyUInt32 j=0;
		char* str = holder.m_classes[i]->m_functionName;
		while (str[j] == functionName[j])
		{
			if (functionName[j] == '\0')
			{
				return holder.m_classes[i]->m_function;
			}
			++j;
		}
	}
	return 0;
}


template <typename FunctionType, KyUInt32 MaxNumber, typename UniqueType>
KyBool StringMap<FunctionType, MaxNumber, UniqueType>::Register(StringMap<FunctionType, MaxNumber, UniqueType>& functionMap)
{
	const char* name = functionMap.GetName();
	FunctionHolder& holder = GetHolder();
	for (KyInt32 i=0 ; i<holder.m_funcCount ; ++i)
	{
		if (strcmp(holder.m_classes[i]->m_functionName, name)==0)
		{
			holder.m_classes[i] = &functionMap;
			return KY_TRUE;
		}
	}
	if (holder.m_funcCount == MaxNumber )
	{
		holder.m_MaxFuncExceeded = KY_TRUE;
		return KY_FALSE;
	}
	holder.m_classes[holder.m_funcCount] = &functionMap;
	++holder.m_funcCount;
	return KY_TRUE;
}

template <typename FunctionType, KyUInt32 MaxNumber, typename UniqueType>
void StringMap<FunctionType, MaxNumber, UniqueType>::UnRegisterAll()
{
	GetHolder().Clear();
}

template <typename FunctionType, KyUInt32 MaxNumber, typename UniqueType>
StringMap<FunctionType, MaxNumber, UniqueType>::StringMap(const char* functionName, FunctionType function)
{
	KY_STRCPY_S(m_functionName, sizeof(m_functionName), functionName);

	m_function = function;
	if (Register(*this) == KY_FALSE)
		m_id = -1;
	else
		m_id = GetHolder().m_funcCount-1;
}

} // namespace Kaim

#endif // KY_STRINGMAP_INL
