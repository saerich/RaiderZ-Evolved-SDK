/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_STRINGMAP_H
#define KY_STRINGMAP_H

#include <kypathdata/generic/types.h>
#include <kypathdata/generic/memory.h>


namespace Kaim
{

/*	For internal use. */
template <typename FunctionType, KyUInt32 MaxNumber, typename UniqueType>
class StringMap : public VirtualMemObject
{
public:
	StringMap(const char* functionName, FunctionType function);
	virtual ~StringMap();

	const char* GetName() const { return m_functionName; }
	FunctionType GetFunction() const { return m_function; }
	KyInt32 GetId() const { return m_id; }

	static FunctionType GetFunctionByName(const char* functionName);

public:
	// For internal use only
	class FunctionHolder
	{
		KY_DEFINE_NEW_DELETE_OPERATORS

	public:
		FunctionHolder()
		{
			static KyBool init = KY_TRUE;
			if (init)
				m_funcCount = 0;
			init = KY_FALSE;
		}

		~FunctionHolder() { Clear(); }

		void Clear()
		{
			m_funcCount = 0;
			m_MaxFuncExceeded = KY_FALSE;
		}

		StringMap* m_classes[MaxNumber];
		KyInt32 m_funcCount;
		KyBool m_MaxFuncExceeded;
	};

	// For internal use only
	static FunctionHolder& GetHolder()
	{
		static FunctionHolder s_functionHolder;
		return s_functionHolder;
	}

	// For internal use only
	static KyBool Register(StringMap& cls);

	// For internal use only
	static void UnRegisterAll();

protected:
	char m_functionName[256];
	FunctionType m_function;
	KyInt32 m_id;
};

#define KY_STRING_MAP_DUMP_KNOWN_FUNCTIONS(C) \
			KY_MESSAGE(KY_LOG_WARNING, ("  Known functions are:")); \
			KY_MESSAGE(KY_LOG_WARNING, ("   --- begin ---")); \
			for (KyInt32 i = 0; i < C::GetHolder().m_funcCount; ++i) \
				KY_MESSAGE(KY_LOG_WARNING, ("   [%s]", C::GetHolder().m_classes[i]->GetName())); \
			KY_MESSAGE(KY_LOG_WARNING, ("   --- end ---"));

} // namespace Kaim

#include <kyruntime/core/stringmap.inl>

#endif // KY_STRINGMAP_H
