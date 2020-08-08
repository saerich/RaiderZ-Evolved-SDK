//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: HashTables.h  : header for various helper HashTable classes
// AUTHOR: Larry.Minton - created Feb.6.2009
//***************************************************************************/
//

#pragma once

#include "MaxIncludes.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

namespace MXS_dotNet
{

/* -------------------- dotNetBaseDotNetLifetimeControlHashTable  ------------------- */
// 
#pragma unmanaged

invisible_class (dotNetBaseDotNetLifetimeControlledHashTable)
class dotNetBaseDotNetLifetimeControlledHashTable : public Value
{
public:
	HashTable	*m_pExisting_dotNetBaseDotNetLifetimeControlledValues;
	dotNetBaseDotNetLifetimeControlledHashTable();
	~dotNetBaseDotNetLifetimeControlledHashTable();
	void	gc_trace();
	void	collect() { delete this; }
	Value*	get(const void* key);
	Value*	put(const void* key, const void* val);
	void	remove(const void* key);
	int		num_entries();
};

/* -------------------- dotNetBaseDotNetLifetimeControlledHashTableMapper  ------------------- */
// 
#pragma unmanaged

class dotNetBaseDotNetLifetimeControlledHashTableMapper : public HashTabMapper 
{
public:
	dotNetBaseDotNetLifetimeControlledHashTableMapper() {}
	void map(const void* key, const void* val);
};

/* -------------------- dotNetMXSValueHashTable  ------------------- */
// 
#pragma unmanaged

invisible_class (dotNetMXSValueHashTable)
class dotNetMXSValueHashTable : public Value
{
public:
	HashTable	*m_pExisting_dotNetMXSValues;
	dotNetMXSValueHashTable();
	~dotNetMXSValueHashTable();
	void	gc_trace();
	void	collect() { delete this; }
	Value*	get(const void* key);
	Value*	put(const void* key, const void* val);
	void	remove(const void* key);
};

/* -------------------- dotNetMXSValueHashTabMapper  ------------------- */
// 
#pragma unmanaged

class dotNetMXSValueHashTabMapper : public HashTabMapper 
{
public:
	dotNetMXSValueHashTabMapper() {}
	void map(const void* key, const void* val);
};

/* -------------------- dotNetBaseDotNetLifetimeControlledHashTableMapperObjectFinder  ------------------- */
// 
#pragma unmanaged

class dotNetBaseDotNetLifetimeControlledHashTableMapperObjectFinder : public HashTabMapper 
{
public:
	Value* m_result;
	gcroot<System::Object^> m_objectToFind;
	gcroot<System::Type^> m_typeToFind;
	dotNetBaseDotNetLifetimeControlledHashTableMapperObjectFinder(gcroot<System::Object^> object, gcroot<System::Type^> type) : m_result(NULL), m_objectToFind(object), m_typeToFind(type) {}
	virtual ~dotNetBaseDotNetLifetimeControlledHashTableMapperObjectFinder() {}
	void map(const void* key, const void* val);
};

}  // end of namespace MXS_dotNet
