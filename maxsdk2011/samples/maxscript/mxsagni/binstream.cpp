// ============================================================================
// BinStream.cpp      By Simon Feltman [discreet]
// ----------------------------------------------------------------------------
#include "MAXScrpt.h"
#include "Numbers.h"
#include "Strings.h"
#include "MAXObj.h"
#include "Structs.h"
#include "Funcs.h"

#include "assetmanagement\AssetUser.h"
#include "AssetManagement\iassetmanager.h"
using namespace MaxSDK::AssetManagement;

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#	include "BinStream.h"

#include <maxscript\macros\define_external_functions.h>
#	include "namedefs.h"

// ============================================================================
#include <maxscript\macros\define_instantiation_functions.h>
#	include "BinStream_wraps.h"

visible_class_instance(BinStream, "BinStream");

// ============================================================================
BinStream::BinStream()
{
	tag = class_tag(BinStream);
	name = _T("");
	desc = NULL;
}


// ============================================================================
void BinStream::collect()
{
	if(desc != NULL) fclose(desc);
	delete this;
}


// ============================================================================
void BinStream::sprin1(CharStream* s)
{
	s->printf(_T("<BinStream:%s>"), (TCHAR*)name);
}


// ============================================================================
void BinStream::gc_trace()
{
	Value::gc_trace();
//	if(desc) fclose(desc); // LAM - defect 290069
}

// ============================================================================
// BinStream fopen <String> <String>
Value* fopen_cf(Value** arg_list, int count)
{
	check_arg_count(fopen, 2, count);

#ifdef USE_GMAX_SECURITY
	TSTR mode = arg_list[1]->to_string();
	if (!strcmp(mode, _T("rb"))==0) {
		NOT_SUPPORTED_BY_PRODUCT("fopen write mode");
	}
#endif

	BinStream *stream = new BinStream();
	if(!stream) return &undefined;

	stream->name = arg_list[0]->to_filename();
	AssetId assetId;
	if (IAssetManager::GetInstance()->StringToAssetId(stream->name, assetId))
		stream->name = IAssetManager::GetInstance()->GetAsset(assetId).GetFileName();

	stream->mode = arg_list[1]->to_string();
	checkFileOpenModeValidity(stream->mode);
	// if not explicitly specifying 't' or 'b' in the mode, append 'b' to open as binary
	if (_tcschr(stream->mode,_T('t')) == NULL && _tcschr(stream->mode,_T('b')) == NULL)
		stream->mode.append(_T("b"));
	stream->desc = fopen(stream->name, stream->mode);

	if(stream->desc) return stream;

	delete stream; // LAM - defect 290069

	return &undefined;
}


// ============================================================================
// Boolean fclose <BinStream>
Value* fclose_cf(Value** arg_list, int count)
{
	check_arg_count(fclose, 1, count);

	BinStream *stream = (BinStream*)arg_list[0];
	if(!is_binstream(stream)) return &false_value;
	stream->name = _T("");

	if(stream->desc) fclose(stream->desc);
	else return &false_value;
	stream->desc = NULL;

	return &true_value;
}


// ============================================================================
// Boolean fseek <BinStream> <Integer> <#seek_set | #seek_cur | #seek_end>
Value* fseek_cf(Value** arg_list, int count)
{
	check_arg_count(fseek, 3, count);

	BinStream *stream = (BinStream*)arg_list[0];
	if(!is_binstream(stream)) return &false_value;

	// get file descriptor
	if(stream->desc == NULL) return &false_value;

	// get offset
	long pos = arg_list[1]->to_int();

	// get origin
	int  seek_type = 0;
	Value *seek_constants[] = { n_seek_set, n_seek_cur, n_seek_end };
	for(seek_type = 0; seek_type < 3; seek_type++)
		if(arg_list[2] == seek_constants[seek_type]) break;
	if(seek_type == 3) return &false_value;

	if(!fseek(stream->desc, pos, seek_type))
		return &true_value;

	return &false_value;
}


// ============================================================================
// Integer ftell <BinStream>
Value* ftell_cf(Value** arg_list, int count)
{
	check_arg_count(ftell, 1, count);
	BinStream *stream = (BinStream*)arg_list[0];
	if(!is_binstream(stream)) return &undefined;
	if(!stream->desc) return &undefined;

	long res = ftell(stream->desc);
	if(res != -1L) return Integer::intern(res);
	return &undefined;
}

// ============================================================================
// Boolean fflush <BinStream>
Value* fflush_cf(Value** arg_list, int count)
{
	check_arg_count(fflush, 1, count);

	BinStream *stream = (BinStream*)arg_list[0];
	if(!is_binstream(stream)) return &false_value;

	if(stream->desc) fflush(stream->desc);
	else return &false_value;

	return &true_value;
}

// ============================================================================
// Boolean WriteByte <BinStream> <Integer> [#signed | #unsigned]
Value* WriteByte_cf(Value** arg_list, int count)
{
	if(count != 2 && count != 3) {
		check_arg_count(WriteByte, 2, count);
	}

	Value *type = &unsupplied;
	if(count == 3)
		type = arg_list[2];

	BinStream *stream = (BinStream*)arg_list[0];
	if(!is_binstream(stream)) return &false_value;
	if(!stream->desc) return &false_value;

	unsigned char c = 0;
	if(type == n_unsigned)
		c = (unsigned char)arg_list[1]->to_int();
	else if(type == n_signed)
		c = (signed char)arg_list[1]->to_int();
	else
		c = (char)arg_list[1]->to_int();

	if(fwrite(&c, sizeof(char), 1, stream->desc) == 1)
		return &true_value;

	return &false_value;
}


// ============================================================================
// Boolean WriteShort <BinStream> <Integer> [#signed | #unsigned]
Value* WriteShort_cf(Value** arg_list, int count)
{
	if(count != 2 && count != 3) {
		check_arg_count(WriteShort, 2, count);
	}

	Value *type = &unsupplied;
	if(count == 3)
		type = arg_list[2];

	BinStream *stream = (BinStream*)arg_list[0];
	if(!is_binstream(stream)) return &false_value;
	if(!stream->desc) return &false_value;

	unsigned short c = 0;
	if(type == n_unsigned)
		c = (unsigned short)arg_list[1]->to_int();
	else if(type == n_signed)
		c = (signed short)arg_list[1]->to_int();
	else
		c = (short)arg_list[1]->to_int();

	if(fwrite(&c, sizeof(short), 1, stream->desc) == 1)
		return &true_value;

	return &false_value;
}


// ============================================================================
// Boolean WriteLong <BinStream> <Integer> [#signed | #unsigned]
Value* WriteLong_cf(Value** arg_list, int count)
{
	if(count != 2 && count != 3) {
		check_arg_count(WriteLong, 2, count);
	}

	Value *type = &unsupplied;
	if(count == 3)
		type = arg_list[2];

	BinStream *stream = (BinStream*)arg_list[0];
	if(!is_binstream(stream)) return &false_value;
	if(!stream->desc) return &false_value;

	unsigned long c = 0;
	if(type == n_unsigned)
		c = (unsigned long)arg_list[1]->to_int();
	else if(type == n_signed)
		c = (signed long)arg_list[1]->to_int();
	else
		c = (long)arg_list[1]->to_int();

	if(fwrite(&c, sizeof(long), 1, stream->desc) == 1)
		return &true_value;

	return &false_value;
}

// ============================================================================
// Boolean WriteLongLong <BinStream> <Integer64> [#signed | #unsigned]
Value* WriteLongLong_cf(Value** arg_list, int count)
{
	if(count != 2 && count != 3) {
		check_arg_count(WriteLongLong, 2, count);
	}

	Value *type = &unsupplied;
	if(count == 3)
		type = arg_list[2];

	BinStream *stream = (BinStream*)arg_list[0];
	if(!is_binstream(stream)) return &false_value;
	if(!stream->desc) return &false_value;

	unsigned long long c = 0;
	if(type == n_unsigned)
		c = (unsigned long long)arg_list[1]->to_int64();
	else if(type == n_signed)
		c = (signed long long)arg_list[1]->to_int64();
	else
		c = (long long)arg_list[1]->to_int64();

	if(fwrite(&c, sizeof(long long), 1, stream->desc) == 1)
		return &true_value;

	return &false_value;
}


// ============================================================================
// Boolean WriteFloat <BinStream> <Float>
Value* WriteFloat_cf(Value** arg_list, int count)
{
	check_arg_count(WriteFloat, 2, count);

	BinStream *stream = (BinStream*)arg_list[0];
	if(!is_binstream(stream)) return &false_value;
	if(!stream->desc) return &false_value;

	float f = arg_list[1]->to_float(); // LAM - 5/6/01
	if(fwrite(&f, sizeof(float), 1, stream->desc) == 1)
		return &true_value;

	return &false_value;
}

// ============================================================================
// Boolean WriteDouble <BinStream> <Double>
Value* WriteDouble_cf(Value** arg_list, int count)
{
	check_arg_count(WriteDouble, 2, count);

	BinStream *stream = (BinStream*)arg_list[0];
	if(!is_binstream(stream)) return &false_value;
	if(!stream->desc) return &false_value;

	double d = arg_list[1]->to_double(); 
	if(fwrite(&d, sizeof(double), 1, stream->desc) == 1)
		return &true_value;

	return &false_value;
}


// ============================================================================
// Boolean WriteString <BinStream> <String>
Value* WriteString_cf(Value** arg_list, int count)
{
	check_arg_count(WriteString, 2, count);

	BinStream *stream = (BinStream*)arg_list[0];
	if(!is_binstream(stream)) return &false_value;
	if(!stream->desc) return &false_value;

	TCHAR *s = arg_list[1]->to_string();
	do {
		if(fputc(*s, stream->desc) == EOF)
			return &false_value;
	} while(*s++);

	return &true_value;
}


// ============================================================================
// Integer ReadByte <BinStream> [#signed | #unsigned]
Value* ReadByte_cf(Value** arg_list, int count)
{
	if(count != 1 && count != 2) {
		check_arg_count(ReadByte, 1, count);
	}

	Value *type = &unsupplied;
	if(count == 2)
		type = arg_list[1];

	BinStream *stream = (BinStream*)arg_list[0];
	if(!is_binstream(stream)) return &undefined;
	if(!stream->desc) return &undefined;

	unsigned char c = -1;
	if(fread(&c, sizeof(unsigned char), 1, stream->desc) == 1)
	{
		if(type == n_unsigned)
			return Integer::intern((unsigned char)c);
		else if(type == n_signed)
			return Integer::intern((signed char)c);
		else
			return Integer::intern((char)c);
	}

	return &undefined;
}


// ============================================================================
// Integer ReadShort <BinStream> [#signed | #unsigned]
Value* ReadShort_cf(Value** arg_list, int count)
{
	if(count != 1 && count != 2) {
		check_arg_count(ReadShort, 1, count);
	}

	Value *type = &unsupplied;
	if(count == 2)
		type = arg_list[1];

	BinStream *stream = (BinStream*)arg_list[0];
	if(!is_binstream(stream)) return &undefined;
	if(!stream->desc) return &undefined;

	unsigned short c = -1;
	if(fread(&c, sizeof(unsigned short), 1, stream->desc) == 1)
	{
		if(type == n_unsigned)
			return Integer::intern((unsigned short)c);
		else if(type == n_signed)
			return Integer::intern((signed short)c);
		else
			return Integer::intern((short) c);
	}

	return &undefined;
}


// ============================================================================
// Integer ReadLong <BinStream> [#signed | #unsigned]
Value* ReadLong_cf(Value** arg_list, int count)
{
	if(count != 1 && count != 2) {
		check_arg_count(ReadLong, 1, count);
	}

	Value *type = &unsupplied;
	if(count == 2)
		type = arg_list[1];

	BinStream *stream = (BinStream*)arg_list[0];
	if(!is_binstream(stream)) return &undefined;
	if(!stream->desc) return &undefined;

	unsigned long c = -1;
	if(fread(&c, sizeof(unsigned long), 1, stream->desc) == 1)
	{
		if(type == n_unsigned)
			return Integer::intern((unsigned long)c);
		else if(type == n_signed)
			return Integer::intern((signed long)c);
		else
			return Integer::intern((long)c);
	}
	return &undefined;
}

// ============================================================================
// Integer64 ReadLongLong <BinStream> [#signed | #unsigned]
Value* ReadLongLong_cf(Value** arg_list, int count)
{
	if(count != 1 && count != 2) {
		check_arg_count(ReadLongLong, 1, count);
	}

	Value *type = &unsupplied;
	if(count == 2)
		type = arg_list[1];

	BinStream *stream = (BinStream*)arg_list[0];
	if(!is_binstream(stream)) return &undefined;
	if(!stream->desc) return &undefined;

	unsigned long long c = -1;
	if(fread(&c, sizeof(long long), 1, stream->desc) == 1)
	{
		if(type == n_unsigned)
			return Integer64::intern((unsigned long long)c);
		else if(type == n_signed)
			return Integer64::intern((signed long long)c);
		else
			return Integer64::intern((long long)c);
	}
	return &undefined;
}


// ============================================================================
// Float ReadFloat <BinStream>
Value* ReadFloat_cf(Value** arg_list, int count)
{
	check_arg_count(ReadFloat, 1, count);

	BinStream *stream = (BinStream*)arg_list[0];
	if(!is_binstream(stream)) return &undefined;
	if(!stream->desc) return &undefined;

	float f = 0.f;
	if(fread(&f, sizeof(float), 1, stream->desc) == 1)
		return Float::intern(f);

	return &undefined;
}

// ============================================================================
// Float ReadDouble <BinStream>
Value* ReadDouble_cf(Value** arg_list, int count)
{
	check_arg_count(ReadDouble, 1, count);

	BinStream *stream = (BinStream*)arg_list[0];
	if(!is_binstream(stream)) return &undefined;
	if(!stream->desc) return &undefined;

	double d = 0.f;
	if(fread(&d, sizeof(double), 1, stream->desc) == 1)
		return Double::intern(d);

	return &undefined;
}


// ============================================================================
// String ReadString <BinStream>
Value* ReadString_cf(Value** arg_list, int count)
{
	int i;
	static char buf[4096];

	check_arg_count(ReadString, 1, count);

	BinStream *stream = (BinStream*)arg_list[0];
	if(!is_binstream(stream)) return &undefined;
	if(!stream->desc) return &undefined;

	// this is probly pretty slow, but i need to read until a zero not a newline.
	for(i = 0; i < sizeof(buf); i++)
	{
		buf[i] = fgetc(stream->desc);
		if(buf[i] == EOF) return &undefined;
		if(buf[i] == 0) break;
	}
	buf[i] = 0;

	return new String(buf);
}

// ============================================================================
// Float ReadDoubleAsFloat <BinStream>
Value* 
ReadDoubleAsFloat_cf(Value** arg_list, int count)
{
	check_arg_count(ReadDoubleAsFloat, 1, count);

	BinStream *stream = (BinStream*)arg_list[0];
	if(!is_binstream(stream)) return &undefined;
	if(!stream->desc) return &undefined;

	double d = 0.;
	if(fread(&d, sizeof(double), 1, stream->desc) == 1)
		return Float::intern(d);

	return &undefined;
}

// ============================================================================
// Boolean WriteFloatAsDouble <BinStream> <Float>
Value* 
WriteFloatAsDouble_cf(Value** arg_list, int count)
{
	check_arg_count(WriteFloatAsDouble, 2, count);

	BinStream *stream = (BinStream*)arg_list[0];
	if(!is_binstream(stream)) return &false_value;
	if(!stream->desc) return &false_value;

	double f = (double)arg_list[1]->to_float(); // LAM - 5/6/01
	if(fwrite(&f, sizeof(double), 1, stream->desc) == 1)
		return &true_value;

	return &false_value;
}

