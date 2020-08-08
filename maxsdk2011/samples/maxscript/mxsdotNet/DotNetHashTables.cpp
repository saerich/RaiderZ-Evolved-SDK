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
// DESCRIPTION: HashTables.cpp  : various helper HashTable classes
// AUTHOR: Larry.Minton - created Feb.6.2009
//***************************************************************************/
//

#include "DotNetHashTables.h"
#include "dotNetControl.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

using namespace MXS_dotNet;

/* -------------------- dotNetBaseDotNetLifetimeControlledHashTable  ------------------- */
// 
#pragma unmanaged

#pragma warning(disable:4835)
invisible_class_instance (MXS_dotNet::dotNetBaseDotNetLifetimeControlledHashTable, _M("dotNetBaseDotNetLifetimeControlledHashTable"))
#pragma warning(default:4835)

dotNetBaseDotNetLifetimeControlledHashTable::dotNetBaseDotNetLifetimeControlledHashTable()
{
	tag = class_tag(dotNetBaseDotNetLifetimeControlledHashTable);
	m_pExisting_dotNetBaseDotNetLifetimeControlledValues = new HashTable(17, default_eq_fn, default_hash_fn, 0); // don't trace key or value
}

dotNetBaseDotNetLifetimeControlledHashTable::~dotNetBaseDotNetLifetimeControlledHashTable()
{
	m_pExisting_dotNetBaseDotNetLifetimeControlledValues = NULL;
}

void
dotNetBaseDotNetLifetimeControlledHashTable::gc_trace()
{
	Value::gc_trace();
	// look at each item in hash table, and if it is still alive trace it.
	dotNetBaseDotNetLifetimeControlledHashTableMapper htm;
	m_pExisting_dotNetBaseDotNetLifetimeControlledValues->map_keys_and_vals(&htm);
	m_pExisting_dotNetBaseDotNetLifetimeControlledValues->gc_trace();
}

Value*
dotNetBaseDotNetLifetimeControlledHashTable::get(const void* key)
{
	return m_pExisting_dotNetBaseDotNetLifetimeControlledValues->get(key);
}

Value*
dotNetBaseDotNetLifetimeControlledHashTable::put(const void* key, const void* value)
{
	return m_pExisting_dotNetBaseDotNetLifetimeControlledValues->put(key, value);
}

void
dotNetBaseDotNetLifetimeControlledHashTable::remove(const void* key)
{
	m_pExisting_dotNetBaseDotNetLifetimeControlledValues->remove(key);
}

int
dotNetBaseDotNetLifetimeControlledHashTable::num_entries()
{
	return m_pExisting_dotNetBaseDotNetLifetimeControlledValues->num_entries();
}

/* -------------------- dotNetBaseDotNetLifetimeControlledHashTableMapper  ------------------- */
// 
#pragma unmanaged

void dotNetBaseDotNetLifetimeControlledHashTableMapper::map(const void* key, const void* val) 
{
	dotNetBase *the_dotNetBaseValue = (dotNetBase*)key;
	if (the_dotNetBaseValue && the_dotNetBaseValue->is_not_marked())
	{
		DotNetObjectWrapper *the_dotNetObjectWrapper = the_dotNetBaseValue->GetDotNetObjectWrapper();
		DbgAssert(the_dotNetObjectWrapper->GetLifetimeControlledByDotnet());
		if (the_dotNetObjectWrapper->GetLifetimeControlledByDotnet() && the_dotNetObjectWrapper->IsWeakRefObjectAlive())
			the_dotNetBaseValue->gc_trace();
	}
}

/* -------------------- dotNetMXSValueHashTable  ------------------- */
// 
#pragma unmanaged

#pragma warning(disable:4835)
invisible_class_instance (MXS_dotNet::dotNetMXSValueHashTable, _M("dotNetMXSValueHashTable"))
#pragma warning(default:4835)

dotNetMXSValueHashTable::dotNetMXSValueHashTable()
{
	tag = class_tag(dotNetMXSValueHashTable);
	m_pExisting_dotNetMXSValues = new HashTable(17, default_eq_fn, default_hash_fn, 0); // don't trace key or value
}

dotNetMXSValueHashTable::~dotNetMXSValueHashTable()
{
	m_pExisting_dotNetMXSValues->make_collectable();
	m_pExisting_dotNetMXSValues = NULL;
}

void
dotNetMXSValueHashTable::gc_trace()
{
	Value::gc_trace();
	// look at each item in hash table, and if it is still alive trace it.
	dotNetMXSValueHashTabMapper htm;
	m_pExisting_dotNetMXSValues->map_keys_and_vals(&htm);
	m_pExisting_dotNetMXSValues->gc_trace();
}

Value*
dotNetMXSValueHashTable::get(const void* key)
{
	return m_pExisting_dotNetMXSValues->get(key);
}

Value*
dotNetMXSValueHashTable::put(const void* key, const void* value)
{
	return m_pExisting_dotNetMXSValues->put(key, value);
}

void
dotNetMXSValueHashTable::remove(const void* key)
{
	m_pExisting_dotNetMXSValues->remove(key);
}

/* -------------------- dotNetMXSValueHashTabMapper  ------------------- */
// 
#pragma unmanaged

void dotNetMXSValueHashTabMapper::map(const void* key, const void* val) 
{
	Value *the_mxsValue = (Value*)key;
	dotNetMXSValue *the_dotNetMXSValue = (dotNetMXSValue*)val;
	DotNetMXSValue *the_DotNetMXSValue = the_dotNetMXSValue->m_pDotNetMXSValue;
	if (the_DotNetMXSValue && the_DotNetMXSValue->IsAlive())
	{
		Value* theVal = the_DotNetMXSValue->Get_dotNetMXSValue();
		if (theVal && theVal->is_not_marked())
			theVal->gc_trace();
	}
}

/* -------------------- dotNetBaseDotNetLifetimeControlledHashTableMapperObjectFinder  ------------------- */
// 
#pragma managed

void dotNetBaseDotNetLifetimeControlledHashTableMapperObjectFinder::map(const void* key, const void* val) 
{
	if (m_result == NULL)
	{
		dotNetBase *the_dotNetBaseValue = (dotNetBase*)key;
		if (the_dotNetBaseValue)
		{
			DotNetObjectWrapper *the_dotNetObjectWrapper = the_dotNetBaseValue->GetDotNetObjectWrapper();
			DbgAssert(the_dotNetObjectWrapper->GetLifetimeControlledByDotnet());
			if (m_objectToFind)
			{
				if (the_dotNetObjectWrapper->GetObject() == m_objectToFind)
					m_result = the_dotNetBaseValue;
			}
			else if (m_typeToFind)
			{
				if (the_dotNetObjectWrapper->GetObject() == nullptr && 
					the_dotNetObjectWrapper->GetType() == m_typeToFind)
					m_result = the_dotNetBaseValue;
			}
		}
	}
}

