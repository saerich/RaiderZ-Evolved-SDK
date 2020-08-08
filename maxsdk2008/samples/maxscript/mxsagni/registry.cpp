/**********************************************************************
*<
FILE: registry.cpp

DESCRIPTION: 

CREATED BY: Larry Minton

HISTORY: Created 15 April 2007

*>	Copyright (c) 2007, All Rights Reserved.
**********************************************************************/

#include "MAXScrpt.h"
#include "Numbers.h"
#include "MAXObj.h"
#include "Strings.h"

#include <SHLWAPI.H>

#ifdef ScripterExport
#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "MXSAgni.h"
#include "agnidefs.h"

// ============================================================================

#include "defextfn.h"
#	include "namedefs.h"

#include "definsfn.h"
#	include "registry_wraps.h"

/* ------------------- HKey class instance ----------------------- */

#include "defimpfn.h"

class HKeyClass : public ValueMetaClass
{
public:	
	HKeyClass(TCHAR* name) : ValueMetaClass (name) { } 
	void		collect() { delete this; }
	Value*		apply(Value** arglist, int count, CallContext* cc=NULL);
};

HKeyClass HKey_class (_T("HKey"));

class HKey : public Value
{
public:
	HKEY		key;
	HKey		*parentKey;
	TSTR		keyName;
	REGSAM		accessRights;

	static long	lastErrorCode;

	HKey();
	HKey(HKEY key, HKey* parentKey, TCHAR* name, REGSAM accessRights);

	classof_methods(HKey, Value);
	void		collect();
	void		sprin1(CharStream* s);
	void		gc_trace();
	def_generic  ( copy,	"copy");
	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);
};

long HKey::lastErrorCode = 0;

// -------------------- HKey methods -------------------------------- 

Value*
HKeyClass::apply(Value** arg_list, int count,  CallContext* cc)
{
	// HKey <HKey parentKey> <string keyName> <int writeAccess>
	check_arg_count(HKey, 3, count);

	type_check(arg_list[0], HKey, "HKey");
	HKey *parentKeyVal = (HKey*)arg_list[0]->eval();
	HKEY hParentKey = parentKeyVal->key;
	if (hParentKey == NULL) 
	{
		HKey::lastErrorCode = -2;
		return &false_value;
	}
	TCHAR* keyName = arg_list[1]->eval()->to_string();

	REGSAM accessRights = arg_list[2]->eval()->to_int();
	HKEY hKey;
	long res = RegOpenKeyEx(hParentKey, keyName, 0, accessRights, &hKey);

	Value *result = new HKey(hKey, parentKeyVal, keyName, accessRights);
	return result;
}

HKey::HKey()
{
	tag = class_tag(HKey);
	key = NULL;
	parentKey = NULL;
	accessRights = KEY_READ;
}

HKey::HKey(HKEY key, HKey* parentKey, TCHAR* name, REGSAM accessRights) : key(key), parentKey(parentKey), accessRights(accessRights)
{
	tag = class_tag(HKey);
	if (name) 
		keyName= name;
}

void
HKey::sprin1(CharStream* s)
{
	s->puts("(HKey ");
	parentKey->sprin1(s);
	s->puts(" \"");
	s->puts(keyName);
	s->puts("\" ");
	s->printf(_T("0x%x"), accessRights);
	s->puts(")");	
}

void
HKey::gc_trace()
{
	// mark me & trace sub-objects
	Value::gc_trace();
	if (parentKey)
		parentKey->gc_trace();
}

void
HKey::collect()
{
	// mark me & trace sub-objects
	if (key)
		RegCloseKey(key);
	delete this;
}

Value*
HKey::copy_vf(Value** arg_list, int count)
{
	// copy <HKey>
	check_arg_count(copy, 1, count + 1);
	return this;
} 

Value*
HKey::get_property(Value** arg_list, int count)
{
	Value* prop = arg_list[0];
	if (prop == n_name)
		return new String(keyName);
	else if (prop == n_accessRights)
		return Integer::intern(accessRights);
	else if (prop == n_parentKey)
	{
		if (parentKey)
			return parentKey;
		else
			return &undefined;
	}
	else
		return Value::get_property(arg_list, count);
}

Value*
HKey::set_property(Value** arg_list, int count)
{
	Value* prop = arg_list[1];
	Value* val = arg_list[0];
	if (prop == n_name)
		throw RuntimeError (GetString(IDS_READ_ONLY_PROPERTY), prop);
	else if (prop == n_accessRights)
		throw RuntimeError (GetString(IDS_READ_ONLY_PROPERTY), prop);
	else if (prop == n_parentKey)
		throw RuntimeError (GetString(IDS_READ_ONLY_PROPERTY), prop);
	else
		return Value::set_property(arg_list, count);
	return val;
}

class ConstHKey : public HKey
{
public:
	String	*keyName;

	ConstHKey (HKEY key, String* keyName) 
		: HKey(key, NULL, NULL, KEY_ALL_ACCESS), keyName(keyName) { }

	BOOL		is_const() { return TRUE; }
	void		collect() { delete this; }
	void		sprin1(CharStream* s);
	void		gc_trace();
	def_generic  ( copy,	"copy");
};

void
ConstHKey::gc_trace()
{
	// mark me & trace sub-objects
	HKey::gc_trace();
	keyName->gc_trace();
}

void
ConstHKey::sprin1(CharStream* s)
{

	s->puts(keyName->to_string());

}

Value*
ConstHKey::copy_vf(Value** arg_list, int count)
{
	// copy <ConstHKey>
	check_arg_count(copy, 1, count + 1);
	return this;
} 

Value*
registry_closeKey_cf(Value** arg_list, int count)
{
	// registry.closeKey <HKey>
	check_arg_count(registry.closeKey, 1, count);
	type_check(arg_list[0], HKey, "HKey");
	HKey *keyVal = (HKey*)arg_list[0];
	HKEY hKey = keyVal->key;
	if (hKey == NULL) 
	{
		HKey::lastErrorCode = -1;
		return &false_value;
	}
	long res = RegCloseKey(hKey);
	if (res == ERROR_SUCCESS)
	{
		if (!keyVal->is_const()) 
			keyVal->key = NULL;
	}
	else
		HKey::lastErrorCode = res;
	return bool_result(res == ERROR_SUCCESS);
}

Value*
registry_flushKey_cf(Value** arg_list, int count)
{
	// registry.flushKey <HKey>
	check_arg_count(registry.flushKey, 1, count);
	type_check(arg_list[0], HKey, "HKey");
	HKey *keyVal = (HKey*)arg_list[0];
	HKEY hKey = keyVal->key;
	if (hKey == NULL) 
	{
		HKey::lastErrorCode = -1;
		return &false_value;
	}
	long res = RegFlushKey(hKey);
	if (res != ERROR_SUCCESS)
		HKey::lastErrorCode = res;
	return bool_result(res == ERROR_SUCCESS);
}

Value*
registry_deleteSubKey_cf(Value** arg_list, int count)
{
	// registry.deleteSubKey <HKey> <string subKeyName> recurse:<false>
	check_arg_count_with_keys(registry.deleteSubKey, 2, count);
	type_check(arg_list[0], HKey, "HKey");
	HKey *keyVal = (HKey*)arg_list[0];
	HKEY hKey = keyVal->key;
	if (hKey == NULL) 
	{
		HKey::lastErrorCode = -1;
		return &false_value;
	}
	Value *tmp;
	BOOL recurse = bool_key_arg(recurse,tmp,FALSE);
	long res;
	if (recurse)
		res = SHDeleteKey(hKey,arg_list[1]->to_string());
	else
		res = RegDeleteKey(hKey,arg_list[1]->to_string());
	if (res != ERROR_SUCCESS)
		HKey::lastErrorCode = res;
	return bool_result(res == ERROR_SUCCESS);
}

Value*
registry_deleteKey_cf(Value** arg_list, int count)
{
	// registry.deleteKey <HKey> recurse:<false>
	check_arg_count_with_keys(registry.deleteKey, 1, count);
	type_check(arg_list[0], HKey, "HKey");
	HKey *keyVal = (HKey*)arg_list[0];
	if (keyVal->is_const()) 
	{
		HKey::lastErrorCode = -3;
		return &false_value;
	}

	HKEY hParentKey = keyVal->parentKey->key;
	if (hParentKey == NULL) 
	{
		HKey::lastErrorCode = -2;
		return &false_value;
	}
	Value *tmp;
	BOOL recurse = bool_key_arg(recurse,tmp,FALSE);
	long res;
	if (recurse)
		res = SHDeleteKey(hParentKey,keyVal->keyName);
	else
		res = RegDeleteKey(hParentKey,keyVal->keyName);
	if (res == ERROR_SUCCESS)
		keyVal->key = NULL;
	else
		HKey::lastErrorCode = res;
	return bool_result(res == ERROR_SUCCESS);
}

Value*
registry_createKey_cf(Value** arg_list, int count)
{
	// registry.createKey <HKey> <string subKeyName> accessRights:<{#readOnly | #writeOnly | #all | <int>}> newKeyCreated:<&bool> Key:<&HKey> volatile:<false>
	check_arg_count_with_keys(registry.createKey, 2, count);
	type_check(arg_list[0], HKey, "HKey");
	HKey *parentKeyVal = (HKey*)arg_list[0];
	HKEY hParentKey = parentKeyVal->key;
	if (hParentKey == NULL) 
	{
		HKey::lastErrorCode = -2;
		return &false_value;
	}
	TCHAR* keyName = arg_list[1]->to_string();

	REGSAM accessRights = KEY_ALL_ACCESS;
	Value *accessRightsVal = key_arg(accessRights);
	if (accessRightsVal == n_readOnly)
		accessRights = KEY_READ;
	else if (accessRightsVal == n_writeOnly)
		accessRights = KEY_WRITE;
	else if (accessRightsVal == n_all)
		accessRights = KEY_ALL_ACCESS;
	else if (accessRightsVal != &unsupplied)
		accessRights = accessRightsVal->to_int();

	Thunk* newKeyCreatedThunk = NULL;
	Value* newKeyCreatedValue = key_arg(newKeyCreated);
	if (newKeyCreatedValue != &unsupplied && newKeyCreatedValue->_is_thunk()) 
		newKeyCreatedThunk = newKeyCreatedValue->to_thunk();

	Thunk* HKeyThunk = NULL;
	Value* HKeyValue = key_arg(key);
	if (HKeyValue != &unsupplied && HKeyValue->_is_thunk()) 
		HKeyThunk = HKeyValue->to_thunk();

	Value *tmp;
	BOOL volatileOpt = bool_key_arg(volatile,tmp,FALSE);
	DWORD options = (volatileOpt) ? REG_OPTION_VOLATILE : REG_OPTION_NON_VOLATILE;

	HKEY hKey;
	DWORD disposition = REG_OPENED_EXISTING_KEY;
	long res = RegCreateKeyEx(hParentKey,keyName,0,_T(""),options,accessRights,NULL,&hKey,&disposition);
	if (res == ERROR_SUCCESS)
	{
		if (HKeyThunk)
			HKeyThunk->assign(new HKey(hKey, parentKeyVal, keyName, accessRights));
		if (newKeyCreatedThunk)
			newKeyCreatedThunk->assign(bool_result(disposition == REG_CREATED_NEW_KEY));
	}
	else
		HKey::lastErrorCode = res;
	return bool_result(res == ERROR_SUCCESS);
}

Value*
registry_openKey_cf(Value** arg_list, int count)
{
	// registry.openKey <HKey> <string subKeyName> accessRights:<{#readOnly | #writeOnly | #all | <int>}> key:<&HKey>
	check_arg_count_with_keys(registry.openKey, 2, count);
	type_check(arg_list[0], HKey, "HKey");
	HKey *parentKeyVal = (HKey*)arg_list[0];
	HKEY hParentKey = parentKeyVal->key;
	if (hParentKey == NULL) 
	{
		HKey::lastErrorCode = -2;
		return &false_value;
	}
	TCHAR* keyName = arg_list[1]->to_string();

	REGSAM accessRights = KEY_READ;
	Value *accessRightsVal = key_arg(accessRights);
	if (accessRightsVal == n_readOnly)
		accessRights = KEY_READ;
	else if (accessRightsVal == n_writeOnly)
		accessRights = KEY_WRITE;
	else if (accessRightsVal == n_all)
		accessRights = KEY_ALL_ACCESS;
	else if (accessRightsVal != &unsupplied)
		accessRights = accessRightsVal->to_int();

	Thunk* HKeyThunk = NULL;
	Value* HKeyValue = key_arg(key);
	if (HKeyValue != &unsupplied && HKeyValue->_is_thunk()) 
		HKeyThunk = HKeyValue->to_thunk();

	HKEY hKey;
	long res = RegOpenKeyEx(hParentKey,keyName,0,accessRights,&hKey);
	if (res == ERROR_SUCCESS)
	{
		if (HKeyThunk)
			HKeyThunk->assign(new HKey(hKey, parentKeyVal, keyName, accessRights));
	}
	else
		HKey::lastErrorCode = res;
	return bool_result(res == ERROR_SUCCESS);
}

Value*
registry_deleteValue_cf(Value** arg_list, int count)
{
	// registry.deleteValue <HKey> <string valueName>
	check_arg_count(registry.deleteValue, 2, count);
	type_check(arg_list[0], HKey, "HKey");
	HKey *keyVal = (HKey*)arg_list[0];
	HKEY hKey = keyVal->key;
	if (hKey == NULL) 
	{
		HKey::lastErrorCode = -1;
		return &false_value;
	}
	long res = RegDeleteValue(hKey,arg_list[1]->to_string());
	if (res != ERROR_SUCCESS)
		HKey::lastErrorCode = res;
	return bool_result(res == ERROR_SUCCESS);
}

Value*
registry_setValue_cf(Value** arg_list, int count)
{
	// registry.setValue <HKey> <string valueName> <name valueType> <value data>
	check_arg_count(registry.setValue, 4, count);
	type_check(arg_list[0], HKey, "HKey");
	HKey *keyVal = (HKey*)arg_list[0];
	HKEY hKey = keyVal->key;
	if (hKey == NULL) 
	{
		HKey::lastErrorCode = -1;
		return &false_value;
	}

	TCHAR* valName = arg_list[1]->to_string();

	Value* typeValue = arg_list[2];
	def_registry_value_types();
	DWORD type = GetID(registryValueTypes, elements(registryValueTypes), typeValue);
	Value* dataValue = arg_list[3];
	long res;

	if (type == REG_DWORD)
	{
		type_check(dataValue, Integer, "Integer");
		DWORD intData = dataValue->to_int();
		res = RegSetValueEx(hKey,valName,0,type,(BYTE *)&intData,sizeof(DWORD));
	}
	else if (type == REG_SZ || type == REG_EXPAND_SZ)
	{
		TCHAR* string = dataValue->to_string();
		int iTotLength = static_cast<int>(strlen(string)+1);
		res = RegSetValueEx(hKey,valName,0,type,(BYTE *)string,iTotLength * sizeof(TCHAR));
	}
	else if (type == REG_MULTI_SZ)
	{
		Array *dataArray;
		if (dataValue->is_kind_of(class_tag(Array))) {
			dataArray = (Array*)dataValue;
		}
		else  
			ConversionError (dataValue, _T("Array"));

		int i, n, iStart, iTotLength, iLength;
		TCHAR *szBuf;

		n = dataArray->size;
		iTotLength = 0;
		for (i = 0; i < n; i++)
			iTotLength += static_cast<int>(strlen(dataArray->data[i]->to_string()) + 1);
		iTotLength += 1;	// Double null terminates a string list
		if (iTotLength == 1)
			iTotLength+=1;
		szBuf = new TCHAR[iTotLength];

		iStart = 0;
		for (i = 0; i < n; i++)
		{
			iLength = static_cast<int>(strlen(dataArray->data[i]->to_string()) + 1);
			memcpy(szBuf + iStart, dataArray->data[i]->to_string(), iLength * sizeof(TCHAR));
			iStart += iLength;
		}
		for (i = iStart; i < iTotLength; i++)
			szBuf[i] = '\0';

		res = RegSetValueEx(hKey,valName,0,type,(BYTE *) szBuf, iTotLength * sizeof(TCHAR));
		delete[] szBuf;
	}
	else
	{
		HKey::lastErrorCode = -4;
		return &false_value;
	}

	if (res != ERROR_SUCCESS)
		HKey::lastErrorCode = res;
	return bool_result(res == ERROR_SUCCESS);
}

Value*
registry_queryValue_cf(Value** arg_list, int count)
{
	// registry.queryValue <HKey> <string valueName> type:<&name> value:<&value> expand:<bool>
	check_arg_count_with_keys(registry.queryValue, 2, count);
	two_value_locals(type, value);
	type_check(arg_list[0], HKey, "HKey");
	HKey *keyVal = (HKey*)arg_list[0];
	HKEY hKey = keyVal->key;
	if (hKey == NULL) 
	{
		HKey::lastErrorCode = -1;
		return &false_value;
	}

	TCHAR* valName = arg_list[1]->to_string();

	long res;

	Thunk* typeThunk = NULL;
	Value* typeValue = key_arg(type);
	if (typeValue != &unsupplied && typeValue->_is_thunk()) 
		typeThunk = typeValue->to_thunk();

	Thunk* valueThunk = NULL;
	Value* valueValue = key_arg(value);
	if (valueValue != &unsupplied && valueValue->_is_thunk()) 
		valueThunk = valueValue->to_thunk();

	Value *tmp;
	BOOL expand = bool_key_arg(expand,tmp,FALSE);

	DWORD type, iBufLen;
	res = RegQueryValueEx(hKey, valName, 0, &type, NULL, &iBufLen);
	iBufLen += 1;

	def_registry_value_types();
	vl.type = GetName(registryValueTypes, elements(registryValueTypes), type);
	if (vl.type == NULL) vl.type = &undefined;

	vl.value = &undefined;

	if (type == REG_DWORD)
	{
		DWORD intData;
		DWORD nSize = sizeof(DWORD);
		res = RegQueryValueEx(hKey,valName,0,&type,(BYTE *)&intData,&nSize);
		if (res == ERROR_SUCCESS)
			vl.value = Integer::intern(intData);
	}
	else if (type == REG_SZ || (type == REG_EXPAND_SZ && !expand))
	{
		TCHAR *szBuf = new TCHAR[iBufLen];
		res = RegQueryValueEx(hKey,valName,0,&type,(BYTE *)szBuf,&iBufLen);
		if (res == ERROR_SUCCESS)
			vl.value = new String (szBuf);
		delete[] szBuf;
	}
	else if (type == REG_EXPAND_SZ && expand)
	{
		TCHAR *szBuf = new TCHAR[iBufLen];
		res = RegQueryValueEx(hKey,valName,0,&type,(BYTE *)szBuf,&iBufLen);
		if (res == ERROR_SUCCESS)
		{
			DWORD res_size = ExpandEnvironmentStrings(szBuf,NULL,0)+1;
			TCHAR *szBuf2 = new TCHAR[res_size];
			DWORD res2 = ExpandEnvironmentStrings(szBuf,szBuf2,res_size);
			if (res2)
				vl.value = new String (szBuf2);
			else
				vl.value = new String (szBuf);
			delete[] szBuf2;
		}
		delete[] szBuf;
	}
	else if (type == REG_MULTI_SZ)
	{

		TCHAR *szBuf = new TCHAR[iBufLen];
		res = RegQueryValueEx(hKey,valName,0,&type,(BYTE *)szBuf,&iBufLen);
		if (res == ERROR_SUCCESS)
		{
			vl.value = new Array(0);
			Array *resArray = (Array*)vl.value;
			int iPos = 0;
			while (szBuf[iPos] != '\0')
			{
				resArray->append(new String (&szBuf[iPos]));
				while (szBuf[iPos] != '\0')
					iPos++;
				iPos++;
			}
		}
		delete[] szBuf;
	}

	if (typeThunk)
		typeThunk->assign(vl.type);
	if (valueThunk)
		valueThunk->assign(vl.value);

	if (res != ERROR_SUCCESS)
		HKey::lastErrorCode = res;

	pop_value_locals();

	return bool_result(res == ERROR_SUCCESS);
}

Value*
registry_queryInfoKey_cf(Value** arg_list, int count)
{
	// registry.queryInfoKey <HKey> numSubKeys:<&int> numValues:<&int>
	check_arg_count_with_keys(registry.queryInfoKey, 1, count);
	type_check(arg_list[0], HKey, "HKey");
	HKey *keyVal = (HKey*)arg_list[0];
	HKEY hKey = keyVal->key;
	if (hKey == NULL) 
	{
		HKey::lastErrorCode = -1;
		return &false_value;
	}

	Thunk* numSubKeysThunk = NULL;
	Value* numSubKeysValue = key_arg(numSubKeys);
	if (numSubKeysValue != &unsupplied && numSubKeysValue->_is_thunk()) 
		numSubKeysThunk = numSubKeysValue->to_thunk();

	Thunk* numValuesThunk = NULL;
	Value* numValuesValue = key_arg(numValues);
	if (numValuesValue != &unsupplied && numValuesValue->_is_thunk()) 
		numValuesThunk = numValuesValue->to_thunk();

	DWORD numSubKeys, numValues;
	long res = RegQueryInfoKey(hKey,NULL,NULL,NULL,&numSubKeys,NULL,NULL,&numValues,NULL,NULL,NULL,NULL);
	if (res == ERROR_SUCCESS)
	{
		if (numSubKeysThunk)
			numSubKeysThunk->assign(Integer::intern(numSubKeys));
		if (numValuesThunk)
			numValuesThunk->assign(Integer::intern(numValues));
	}
	else
		HKey::lastErrorCode = res;
	return bool_result(res == ERROR_SUCCESS);
}

Value*
registry_isKeyOpen_cf(Value** arg_list, int count)
{
	// registry.isKeyOpen <HKey> 
	check_arg_count(registry.isKeyOpen, 1, count);
	type_check(arg_list[0], HKey, "HKey");
	HKey *keyVal = (HKey*)arg_list[0];
	HKEY hKey = keyVal->key;
	return bool_result(hKey != NULL);
}

Value*
registry_isParentKeyOpen_cf(Value** arg_list, int count)
{
	// registry.isParentKeyOpen <HKey> 
	check_arg_count(registry.isParentKeyOpen, 1, count);
	type_check(arg_list[0], HKey, "HKey");
	HKey *keyVal = (HKey*)arg_list[0];
	if (keyVal->is_const())
		return &false_value;
	HKEY hKey = keyVal->parentKey->key;
	return bool_result(hKey != NULL);
}

Value*
registry_isKeyConstant_cf(Value** arg_list, int count)
{
	// registry.isKeyOpen <HKey> 
	check_arg_count(registry.isKeyConstant, 1, count);
	type_check(arg_list[0], HKey, "HKey");
	HKey *keyVal = (HKey*)arg_list[0];
	return bool_result(keyVal->is_const());
}

Value*
registry_getParentKey_cf(Value** arg_list, int count)
{
	// registry.getParentKey <HKey> 
	check_arg_count(registry.getParentKey, 1, count);
	type_check(arg_list[0], HKey, "HKey");
	HKey *keyVal = (HKey*)arg_list[0];
	if (keyVal->is_const())
		return &undefined;
	return keyVal->parentKey;
}

Value*
registry_getSubKeyName_cf(Value** arg_list, int count)
{
	// registry.getSubKeyName <HKey> <index> name:<&name>
	check_arg_count_with_keys(registry.getSubKeyName, 2, count);
	type_check(arg_list[0], HKey, "HKey");
	HKey *keyVal = (HKey*)arg_list[0];
	HKEY hKey = keyVal->key;
	if (hKey == NULL) 
	{
		HKey::lastErrorCode = -1;
		return &false_value;
	}
	int index = arg_list[1]->to_int()-1;

	Thunk* nameThunk = NULL;
	Value* nameValue = key_arg(name);
	if (nameValue != &unsupplied && nameValue->_is_thunk()) 
		nameThunk = nameValue->to_thunk();

	DWORD numSubKeys, numValues, maxSubKeyLen, maxValueLen;
	long res = RegQueryInfoKey(hKey,NULL,NULL,NULL,&numSubKeys,&maxSubKeyLen,NULL,&numValues,&maxValueLen,NULL,NULL,NULL);
	{
		range_check(index+1,1,numSubKeys,_T("subKey index out of range"));
		maxSubKeyLen += 1;
		TCHAR *szBuf = new TCHAR[maxSubKeyLen];
		res = RegEnumKeyEx(hKey,index,szBuf,&maxSubKeyLen,NULL,NULL,NULL,NULL);
		if (res == ERROR_SUCCESS)
		{
			if (nameThunk)
				nameThunk->assign(new String (szBuf));
		}
		delete[] szBuf;
	}
	if (res != ERROR_SUCCESS)
		HKey::lastErrorCode = res;
	return bool_result(res == ERROR_SUCCESS);
}

Value*
registry_getSubKeyNames_cf(Value** arg_list, int count)
{
	// registry.getSubKeyNames <HKey> names:<&array>
	check_arg_count_with_keys(registry.getSubKeyNames, 1, count);
	type_check(arg_list[0], HKey, "HKey");
	HKey *keyVal = (HKey*)arg_list[0];
	HKEY hKey = keyVal->key;
	if (hKey == NULL) 
	{
		HKey::lastErrorCode = -1;
		return &false_value;
	}

	Thunk* nameThunk = NULL;
	Value* nameValue = key_arg(names);
	if (nameValue != &unsupplied && nameValue->_is_thunk()) 
		nameThunk = nameValue->to_thunk();


	DWORD numSubKeys, numValues, maxSubKeyLen,maxValueLen;
	long res = RegQueryInfoKey(hKey,NULL,NULL,NULL,&numSubKeys,&maxSubKeyLen,NULL,&numValues,&maxValueLen,NULL,NULL,NULL);
	{
		one_typed_value_local(Array* result);
		vl.result = new Array (numSubKeys);
		maxSubKeyLen += 1;
		TCHAR *szBuf = new TCHAR[maxSubKeyLen];
		for (int i = 0; i < numSubKeys; i++)
		{
			DWORD tmp = maxSubKeyLen;
			res = RegEnumKeyEx(hKey,i,szBuf,&tmp,NULL,NULL,NULL,NULL);
			if (res == ERROR_SUCCESS)
			{
				vl.result->append(new String (szBuf));
			}
		}
		delete[] szBuf;
		if (nameThunk)
			nameThunk->assign(vl.result);
		pop_value_locals();
	}
	if (res != ERROR_SUCCESS)
		HKey::lastErrorCode = res;
	return bool_result(res == ERROR_SUCCESS);
}

Value*
registry_getValueName_cf(Value** arg_list, int count)
{
	// registry.getValueName <HKey> <index> name:<&name>
	check_arg_count_with_keys(registry.getValueName, 2, count);
	type_check(arg_list[0], HKey, "HKey");
	HKey *keyVal = (HKey*)arg_list[0];
	HKEY hKey = keyVal->key;
	if (hKey == NULL) 
	{
		HKey::lastErrorCode = -1;
		return &false_value;
	}
	int index = arg_list[1]->to_int()-1;

	Thunk* nameThunk = NULL;
	Value* nameValue = key_arg(name);
	if (nameValue != &unsupplied && nameValue->_is_thunk()) 
		nameThunk = nameValue->to_thunk();

	DWORD numSubKeys, numValues, maxSubKeyLen,maxValueLen;
	long res = RegQueryInfoKey(hKey,NULL,NULL,NULL,&numSubKeys,&maxSubKeyLen,NULL,&numValues,&maxValueLen,NULL,NULL,NULL);
	{
		range_check(index+1,1,numValues,_T("value index out of range"));
		maxValueLen += 1;
		TCHAR *szBuf = new TCHAR[maxValueLen];
		res = RegEnumValue(hKey,index,szBuf,&maxValueLen,NULL,NULL,NULL,NULL);
		if (res == ERROR_SUCCESS)
		{
			if (nameThunk)
				nameThunk->assign(new String (szBuf));
		}
		delete[] szBuf;
	}
	if (res != ERROR_SUCCESS)
		HKey::lastErrorCode = res;
	return bool_result(res == ERROR_SUCCESS);
}

Value*
registry_getValueNames_cf(Value** arg_list, int count)
{
	// registry.getValueNames <HKey> names:<&array>
	check_arg_count_with_keys(registry.getValueNames, 1, count);
	type_check(arg_list[0], HKey, "HKey");
	HKey *keyVal = (HKey*)arg_list[0];
	HKEY hKey = keyVal->key;
	if (hKey == NULL) 
	{
		HKey::lastErrorCode = -1;
		return &false_value;
	}

	Thunk* nameThunk = NULL;
	Value* nameValue = key_arg(names);
	if (nameValue != &unsupplied && nameValue->_is_thunk()) 
		nameThunk = nameValue->to_thunk();


	DWORD numSubKeys, numValues, maxSubKeyLen,maxValueLen;
	long res = RegQueryInfoKey(hKey,NULL,NULL,NULL,&numSubKeys,&maxSubKeyLen,NULL,&numValues,&maxValueLen,NULL,NULL,NULL);
	{
		one_typed_value_local(Array* result);
		vl.result = new Array (numValues);
		maxValueLen += 1;
		TCHAR *szBuf = new TCHAR[maxValueLen];
		for (int i = 0; i < numValues; i++)
		{
			DWORD tmp = maxValueLen;
			res = RegEnumValue(hKey,i,szBuf,&tmp,NULL,NULL,NULL,NULL);
			if (res == ERROR_SUCCESS)
			{
				vl.result->append(new String (szBuf));
			}
		}
		delete[] szBuf;
		if (nameThunk)
			nameThunk->assign(vl.result);
		pop_value_locals();
	}
	if (res != ERROR_SUCCESS)
		HKey::lastErrorCode = res;
	return bool_result(res == ERROR_SUCCESS);
}

Value*
registry_getLastError_cf(Value** arg_list, int count)
{
	// registry.getLastError()
	check_arg_count(registry.getLastError, 0, count);
	one_value_local(result);
	if (HKey::lastErrorCode == -1)
		vl.result = new String(_T("key is not open"));
	else if (HKey::lastErrorCode == -2)
		vl.result = new String(_T("parent key is not open"));
	else if (HKey::lastErrorCode == -3)
		vl.result = new String(_T("attempt to delete constant key"));
	else if (HKey::lastErrorCode == -4)
		vl.result = new String(_T("unsupported registry value type"));
	else 
	{
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
			HKey::lastErrorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf, 0, NULL);
		if (lpMsgBuf == NULL)
			vl.result = new String(_T("unknown error"));
		else
			vl.result = new String((LPTSTR)lpMsgBuf);
		LocalFree(lpMsgBuf);
	}
	return_value(vl.result)
}

/* --------------------- plug-in init --------------------------------- */
// this is called by the dlx initializer, register the global vars here
void registry_init()
{

#define def_global(n, v)		\
	vl.name = Name::intern(n);	\
	globals->put_new(vl.name, (vl.thunk = new ConstGlobalThunk (vl.name, (vl.val = (v)))))

	three_value_locals(name, val, thunk);
	def_global( "HKEY_CLASSES_ROOT",		new (GC_STATIC) ConstHKey(HKEY_CLASSES_ROOT, new String(_T("HKEY_CLASSES_ROOT"))) );
	def_global( "HKEY_CURRENT_CONFIG",		new (GC_STATIC) ConstHKey(HKEY_CURRENT_CONFIG, new String(_T("HKEY_CURRENT_CONFIG"))) );
	def_global( "HKEY_CURRENT_USER",		new (GC_STATIC) ConstHKey(HKEY_CURRENT_USER, new String(_T("HKEY_CURRENT_USER"))) );
	def_global( "HKEY_LOCAL_MACHINE",		new (GC_STATIC) ConstHKey(HKEY_LOCAL_MACHINE, new String(_T("HKEY_LOCAL_MACHINE"))) );
	def_global( "HKEY_USERS",				new (GC_STATIC) ConstHKey(HKEY_USERS, new String(_T("HKEY_USERS"))) );
	def_global( "HKEY_PERFORMANCE_DATA",	new (GC_STATIC) ConstHKey(HKEY_PERFORMANCE_DATA, new String(_T("HKEY_PERFORMANCE_DATA"))) );
	pop_value_locals();

}