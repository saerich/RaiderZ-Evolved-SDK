#include "MAXScrpt.h"
#include "Numbers.h"
#include "Strings.h"
#include "MXSAgni.h"

#include "resource.h"

#ifdef ScripterExport
#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include <maxscript\macros\define_instantiation_functions.h>
#include "extclass.h"

// ============================================================================

#include <maxscript\macros\define_external_functions.h>
#	include "namedefs.h"

#include <maxscript\macros\define_instantiation_functions.h>
#	include "bit_wraps.h"
// -----------------------------------------------------------------------------------------

//  =================================================
//  Bit operations
//  =================================================

union shareIntFloat
{
	int iVal;
	INT64 i64Val;
	UINT i64Valpart[2];
	INT_PTR iptrVal;
	float fVal;
	double dVal;
	byte bVal[8];
};

Value* 
bit_and_cf(Value** arg_list, int count)
{
	//	and <integer> <integer>
	check_arg_count(and, 2, count);
	if (is_int64(arg_list[0]))
		return Integer64::intern(arg_list[0]->to_int64() & arg_list[1]->to_int64());
	else if (is_intptr(arg_list[0]))
		return IntegerPtr::intern(arg_list[0]->to_intptr() & arg_list[1]->to_intptr());
	else
		return Integer::intern(arg_list[0]->to_int() & arg_list[1]->to_int());
}

Value* 
bit_or_cf(Value** arg_list, int count)
{
	//	or <integer> <integer>
	check_arg_count(or, 2, count);
	if (is_int64(arg_list[0]))
		return Integer64::intern(arg_list[0]->to_int64() | arg_list[1]->to_int64());
	else if (is_intptr(arg_list[0]))
		return IntegerPtr::intern(arg_list[0]->to_intptr() | arg_list[1]->to_intptr());
	else
		return Integer::intern(arg_list[0]->to_int() | arg_list[1]->to_int());
}

Value* 
bit_xor_cf(Value** arg_list, int count)
{
	//	xor <integer> <integer>
	check_arg_count(xor, 2, count);
	if (is_int64(arg_list[0]))
		return Integer64::intern(arg_list[0]->to_int64() ^ arg_list[1]->to_int64());
	else if (is_intptr(arg_list[0]))
		return IntegerPtr::intern(arg_list[0]->to_intptr() ^ arg_list[1]->to_intptr());
	else
		return Integer::intern(arg_list[0]->to_int() ^ arg_list[1]->to_int());
}

Value* 
bit_not_cf(Value** arg_list, int count)
{
	//	not <integer>
	check_arg_count(not, 1, count);
	if (is_int64(arg_list[0]))
		return Integer64::intern(~arg_list[0]->to_int64());
	else if (is_intptr(arg_list[0]))
		return IntegerPtr::intern(~arg_list[0]->to_intptr());
	else
		return Integer::intern(~arg_list[0]->to_int());
}

Value* 
bit_shift_cf(Value** arg_list, int count)
{
	//	shift <integer> <integer> -- positive values shift to left
	check_arg_count(shift, 2, count);
	int shiftCount = arg_list[1]->to_int();
	if (is_int64(arg_list[0]))
	{
		UINT64 val = arg_list[0]->to_int64();
		int int64_num_bits = sizeof(INT64) * 8;
		range_check(shiftCount, -int64_num_bits, int64_num_bits, GetString(IDS_BIT_INDEX_OR_SHIFT_COUNT_OUT_OF_RANGE));
		if (shiftCount > 0)
			val <<= shiftCount;
		else if (shiftCount < 0)
			val >>= (UINT) abs(shiftCount);
		return Integer64::intern(val);
	}
	else if (is_intptr(arg_list[0]))
	{
		UINT_PTR val = arg_list[0]->to_intptr();
		int intptr_num_bits = sizeof(INT_PTR) * 8;
		range_check(shiftCount, -intptr_num_bits, intptr_num_bits, GetString(IDS_BIT_INDEX_OR_SHIFT_COUNT_OUT_OF_RANGE));
		if (shiftCount > 0)
			val <<= shiftCount;
		else if (shiftCount < 0)
			val >>= (UINT) abs(shiftCount);
		return IntegerPtr::intern(val);
	}
	else
	{
		UINT val = arg_list[0]->to_int();
		int int_num_bits = sizeof(int) * 8;
		range_check(shiftCount, -int_num_bits, int_num_bits, GetString(IDS_BIT_INDEX_OR_SHIFT_COUNT_OUT_OF_RANGE));
		if (shiftCount > 0)
			val <<= shiftCount;
		else if (shiftCount < 0)
			val >>= (UINT) abs(shiftCount);
		return Integer::intern(val);
	}
}

Value* 
bit_set_cf(Value** arg_list, int count)
{
	//	set <integer> <integer> <bool>
	check_arg_count(set, 3, count);
	int whichBit = arg_list[1]->to_int();
	if (is_int64(arg_list[0]))
	{
		INT64 val = arg_list[0]->to_int64();
		INT64 mask = 1;
		mask <<= (whichBit-1);
		int int64_num_bits = sizeof(INT64) * 8;
		range_check(whichBit, 1, int64_num_bits, GetString(IDS_BIT_INDEX_OR_SHIFT_COUNT_OUT_OF_RANGE));
		return Integer64::intern((arg_list[2]->to_bool()) ? val | mask : val & ~mask);
	}
	else if (is_intptr(arg_list[0]))
	{
		INT_PTR val = arg_list[0]->to_intptr();
		INT_PTR mask = 1;
		mask <<= (whichBit-1);
		int intptr_num_bits = sizeof(INT_PTR) * 8;
		range_check(whichBit, 1, intptr_num_bits, GetString(IDS_BIT_INDEX_OR_SHIFT_COUNT_OUT_OF_RANGE));
		return IntegerPtr::intern((arg_list[2]->to_bool()) ? val | mask : val & ~mask);
	}
	else
	{
		int val = arg_list[0]->to_int();
		int mask = 1 << (whichBit-1);
		int int_num_bits = sizeof(int) * 8;
		range_check(whichBit, 1, int_num_bits, GetString(IDS_BIT_INDEX_OR_SHIFT_COUNT_OUT_OF_RANGE));
		return Integer::intern((arg_list[2]->to_bool()) ? val | mask : val & ~mask);
	}
}

Value* 
bit_flip_cf(Value** arg_list, int count)
{
	//	flip <integer> <integer>
	check_arg_count(flip, 2, count);
	int whichBit = arg_list[1]->to_int();
	if (is_int64(arg_list[0]))
	{
		INT64 val = arg_list[0]->to_int64();
		INT64 mask = 1;
		mask <<= (whichBit-1);
		int int64_num_bits = sizeof(INT64) * 8;
		range_check(whichBit, 1, int64_num_bits, GetString(IDS_BIT_INDEX_OR_SHIFT_COUNT_OUT_OF_RANGE));
		return Integer64::intern(val ^ mask);
	}
	else if (is_intptr(arg_list[0]))
	{
		INT_PTR val = arg_list[0]->to_intptr();
		INT_PTR mask = 1;
		mask <<= (whichBit-1);
		int intptr_num_bits = sizeof(INT_PTR) * 8;
		range_check(whichBit, 1, intptr_num_bits, GetString(IDS_BIT_INDEX_OR_SHIFT_COUNT_OUT_OF_RANGE));
		return IntegerPtr::intern(val ^ mask);
	}
	else
	{
		int val = arg_list[0]->to_int();
		int mask = 1 << (whichBit-1);
		int int_num_bits = sizeof(int) * 8;
		range_check(whichBit, 1, int_num_bits, GetString(IDS_BIT_INDEX_OR_SHIFT_COUNT_OUT_OF_RANGE));
		return Integer::intern(val ^ mask);
	}
}

Value* 
bit_get_cf(Value** arg_list, int count)
{
	//	get <integer> <integer>
	check_arg_count(get, 2, count);
	int whichBit = arg_list[1]->to_int();
	if (is_int64(arg_list[0]))
	{
		INT64 val = arg_list[0]->to_int64();
		INT64 mask = 1;
		mask <<= (whichBit-1);
		int int64_num_bits = sizeof(INT64) * 8;
		range_check(whichBit, 1, int64_num_bits, GetString(IDS_BIT_INDEX_OR_SHIFT_COUNT_OUT_OF_RANGE));
		return (val & mask) ? &true_value : &false_value;
	}
	else if (is_intptr(arg_list[0]))
	{
		INT_PTR val = arg_list[0]->to_intptr();
		INT_PTR mask = 1;
		mask <<= (whichBit-1);
		int intptr_num_bits = sizeof(INT_PTR) * 8;
		range_check(whichBit, 1, intptr_num_bits, GetString(IDS_BIT_INDEX_OR_SHIFT_COUNT_OUT_OF_RANGE));
		return (val & mask) ? &true_value : &false_value;
	}
	else
	{
		int val = arg_list[0]->to_int();
		int mask = 1 << (whichBit-1);
		int int_num_bits = sizeof(int) * 8;
		range_check(whichBit, 1, int_num_bits, GetString(IDS_BIT_INDEX_OR_SHIFT_COUNT_OUT_OF_RANGE));
		return (val & mask) ? &true_value : &false_value;
	}
}

Value* 
bit_intAsChar_cf(Value** arg_list, int count)
{
	//	intAsChar <integer> 
	check_arg_count(intAsChar, 1, count);
	TCHAR buf[2] = {0};
	int val = arg_list[0]->to_int();
	buf[0]= val & ((sizeof(TCHAR) == 1) ? 0x0FF : 0x0FFFF);
	return new String(buf);
}

Value* 
bit_charAsInt_cf(Value** arg_list, int count)
{
	//	charAsInt <string> 
	check_arg_count(charAsInt, 1, count);
	int val = (arg_list[0]->to_string())[0];
	return Integer::intern(val & ((sizeof(TCHAR) == 1) ? 0x0FF : 0x0FFFF));
}


Value* 
bit_intAsHex_cf(Value** arg_list, int count)
{
	//	intAsHex <int> 
	check_arg_count(intAsHex, 1, count);
	const int BUFLEN = 32;
	TCHAR buf[BUFLEN] = {0};
	if (is_int64(arg_list[0]))
	{
		shareIntFloat ifUnion;
		ifUnion.i64Val = arg_list[0]->to_int64();
		if (ifUnion.i64Valpart[1] == 0)
			wsprintf (buf, "%xL", ifUnion.i64Valpart[0]);
		else
			wsprintf (buf, "%x%08xL", ifUnion.i64Valpart[1], ifUnion.i64Valpart[0]);
	}
	else if (is_intptr(arg_list[0]))
		wsprintf (buf, "%pP", arg_list[0]->to_intptr());
	else
		wsprintf (buf, "%x", arg_list[0]->to_int());
	return new String(buf);
}

Value* 
bit_intAsFloat_cf(Value** arg_list, int count)
{
	//	intAsFloat <integer> 
	check_arg_count(intAsFloat, 1, count);
	shareIntFloat ifUnion;
	ifUnion.iVal = arg_list[0]->to_int();
	return Float::intern(ifUnion.fVal);
}

Value* 
bit_floatAsInt_cf(Value** arg_list, int count)
{
	//	floatAsInt <float> 
	check_arg_count(floatAsInt, 1, count);
	shareIntFloat ifUnion;
	ifUnion.fVal = arg_list[0]->to_float();
	return Integer::intern(ifUnion.iVal);
}

Value* 
bit_int64AsDouble_cf(Value** arg_list, int count)
{
	//	int64AsDouble <integer64> 
	check_arg_count(int64AsDouble, 1, count);
	shareIntFloat ifUnion;
	ifUnion.i64Val = arg_list[0]->to_int64();
	return Double::intern(ifUnion.dVal);
}

Value* 
bit_doubleAsInt64_cf(Value** arg_list, int count)
{
	//	doubleAsInt64 <double> 
	check_arg_count(doubleAsInt64, 1, count);
	shareIntFloat ifUnion;
	ifUnion.dVal = arg_list[0]->to_double();
	return Integer64::intern(ifUnion.i64Val);
}

Value* 
bit_swapBytes_cf(Value** arg_list, int count)
{
	//	swapBytes <integer> <integer> <integer>
	check_arg_count(floatAsInt, 3, count);
	shareIntFloat ifUnion;
	ifUnion.i64Val = arg_list[0]->to_int64();
	int from = arg_list[1]->to_int();
	int to = arg_list[2]->to_int();
	int max_num_bytes = sizeof(int);
	if (is_int64(arg_list[0]))
		max_num_bytes = sizeof(INT64);
	else if (is_intptr(arg_list[0]))
		max_num_bytes = sizeof(INT_PTR);
	range_check(from, 1, max_num_bytes, _T("byte index out of range: "));
	range_check(to, 1, max_num_bytes, _T("byte index out of range: "));
	from--;
	to--;
	byte tmp = ifUnion.bVal[to];
	ifUnion.bVal[to] = ifUnion.bVal[from];
	ifUnion.bVal[from] = tmp;
	if (is_int64(arg_list[0]))
		return Integer64::intern(ifUnion.i64Val);
	else if (is_intptr(arg_list[0]))
		return IntegerPtr::intern(ifUnion.iptrVal);
	return Integer::intern(ifUnion.iVal);
}

Value* 
bit_isNAN_cf(Value** arg_list, int count)
{
	//	isNAN <float|double> 
	check_arg_count(isNAN, 1, count);
	if (is_double(arg_list[0]))
	{
		double dVal = arg_list[0]->to_double();
		return (_isnan(dVal)) ? &true_value : &false_value;
	}
	float fVal = arg_list[0]->to_float();
	return (_isnan(fVal)) ? &true_value : &false_value;
}

Value* 
bit_isFinite_cf(Value** arg_list, int count)
{
	//	isFinite <float|double> 
	check_arg_count(isFinite, 1, count);
	if (is_double(arg_list[0]))
	{
		double dVal = arg_list[0]->to_double();
		return (_finite(dVal)) ? &true_value : &false_value;
	}
	float fVal = arg_list[0]->to_float();
	return (_finite(fVal)) ? &true_value : &false_value;
}

Value* 
bit_hexAsInt_cf(Value** arg_list, int count)
{
	//	hexAsInt <string> 
	check_arg_count(hexAsInt, 1, count);
	// see if whether int64 or intptr by whether last character is an L or a P. If neither, its an int.
	TCHAR* buf = arg_list[0]->to_string();
	INT64 val = 0;
	int i = 0;
	int len = static_cast<int>(_tcslen(buf));
	if (len > 2 && buf[0] == _T('0') && buf[1] == _T('x')) i = 2;
	for (; i < len; i++)
	{
		TCHAR c = buf[i];
		if (isdigit(c))
			val = (val * 16) + (c - _T('0'));
		else if (c >= _T('a') && c <= _T('f'))
			val = (val * 16) + 10 + (c - _T('a'));
		else if (c >= _T('A') && c <= _T('F'))
			val = (val * 16) + 10 + (c - _T('A'));
		else if (c == _T('L'))
			return Integer64::intern(val);
		else if (c == _T('P'))
			return IntegerPtr::intern(val);
		else
			throw RuntimeError (GetString(IDS_INVALID_HEX_STRING), arg_list[0]);
	}
	return Integer::intern(val);
}
