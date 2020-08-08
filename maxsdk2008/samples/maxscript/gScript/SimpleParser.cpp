// ============================================================================
// SimpleParser.cpp
//
// History:
//  02.16.01 - Created by Simon Feltman
//
// Copyright ©2001, Discreet
// ----------------------------------------------------------------------------
#include "MAXScrpt.h"
#include "Streams.h"
#include "Strings.h"
#include "Numbers.h"
#include "iMemStream.h"


// ============================================================================
#include "definsfn.h"
	def_visible_primitive_debug_ok	( isSpace,		"isSpace");
	def_visible_primitive_debug_ok	( trimLeft,		"trimLeft");
	def_visible_primitive_debug_ok	( trimRight,	"trimRight");
	def_visible_primitive_debug_ok	( readToken,	"readToken");
	def_visible_primitive_debug_ok	( peekToken,	"peekToken");
	def_visible_primitive_debug_ok	( skipSpace,	"skipSpace");
	def_visible_primitive_debug_ok	( getFileSize,	"getFileSize");

void SkipSpace( CharStream *stream )
{
	TCHAR c;
	while(!stream->at_eos())
	{
		c = stream->get_char();
		if(IsSpace(c))
			continue;
		else
		{
			if(c != _T('\0'))
				stream->unget_char(c);
			break;
		}
	}
}

// ============================================================================
// <Boolean> isSpace <String>
Value* isSpace_cf(Value** arg_list, int count)
{
	check_arg_count(isSpace, 1, count);
	TCHAR *str = arg_list[0]->to_string();
	return (IsSpace(str[0]) ? &true_value : &false_value);
}

// ============================================================================
// <String> trimLeft <String> [String trimChars]
Value* trimLeft_cf(Value** arg_list, int count)
{
	if(count < 1 || count > 2)
		check_arg_count(trimLeft, 1, count);

	TCHAR *trim;
	TCHAR *in = arg_list[0]->to_string();
	if(count == 2)
		trim = arg_list[1]->to_string();
	else
		trim = whitespace;

	while(*in && CharInStr(*in, trim)) in++;
	return new String(in);
}

// ============================================================================
// <String> trimRight <String> [String trimChars]
Value* trimRight_cf(Value** arg_list, int count)
{
	if(count < 1 || count > 2)
		check_arg_count(trimLeft, 1, count);

	TCHAR *trim;
	TCHAR *in = arg_list[0]->to_string();
	if(count == 2)
		trim = arg_list[1]->to_string();
	else
		trim = whitespace;

	TSTR str(in);
	TCHAR *data = str;
	TCHAR *end = data + str.length()-1;
	while(end >= data && CharInStr(*end, trim)) *end-- =  _T('\0');

	return new String(data);
}

// ============================================================================
// <String> readToken <CharStream inStream>
Value* readToken_cf(Value** arg_list, int count)
{
	check_arg_count(readToken, 1, count);
	if(arg_list[0]->is_kind_of(class_tag(CharStream)))
	{
		CharStream *stream = (CharStream*)arg_list[0];
		TCHAR token[512];
		TCHAR *t = token; *t = _T('\0');

retry:;
		// Skip leading space
		SkipSpace(stream);
		if(stream->at_eos()) return &undefined;

		// Skip '//' style comments
		TCHAR c = stream->get_char();
		if(c == _T('/') && !stream->at_eos() && stream->peek_char() == _T('/'))
		{
			while(!stream->at_eos() && !IsNewline(c=stream->get_char())) ;
			goto retry;
		}

		if(c == _T('"'))
		{
			while(!stream->at_eos() && (c=stream->get_char()) != _T('"'))
				*t++ = c;
		}
		else
		{
			do {
				*t++ = c;
			} while(!stream->at_eos() && !IsSpace(c=stream->get_char()));
		}
		*t = _T('\0');
		return new String(token);
	}
	return &undefined;
}

// ============================================================================
// <String> peekToken <CharStream inStream>
Value* peekToken_cf(Value** arg_list, int count)
{
	check_arg_count(peekToken, 1, count);
	if(arg_list[0]->is_kind_of(class_tag(CharStream)))
	{
		CharStream *stream = (CharStream*)arg_list[0];
		int pos = stream->pos();
		Value *res = readToken_cf(arg_list, count);
		stream->seek(pos);
		return res;
	}
	return &undefined;
}

// ============================================================================
// skipSpace <CharStream inStream>
Value* skipSpace_cf(Value** arg_list, int count)
{
	check_arg_count(skipSpace, 1, count);
	if(arg_list[0]->is_kind_of(class_tag(CharStream)))
	{
		CharStream *stream = (CharStream*)arg_list[0];
		SkipSpace(stream);
	}

	return &ok;
}

// ============================================================================
// getFileSize <String fileName>
Value* getFileSize_cf(Value** arg_list, int count)
{
	check_arg_count(getFileSize, 1, count);
	DWORD size = 0;

	HANDLE hFile = CreateFile(arg_list[0]->to_filename(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		size = GetFileSize(hFile, NULL);
		if(size == 0xFFFFFFFF)
			size = 0;
		CloseHandle(hFile);
	}
	return Integer::intern(size);
}

