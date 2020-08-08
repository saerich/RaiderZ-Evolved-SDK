// ============================================================================
//
// iMemStream.h
// Fast function published memory stream.
// Copyright ©2001, Discreet  -  www.discreet.com
// History:
//  02.25.01 - Created by Simon Feltman
//
// ----------------------------------------------------------------------------
#ifndef __IMEMSTREAM_H__
#define __IMEMSTREAM_H__

#include "max.h"
#include "iFnPub.h"

#define MEMSTREAM_INTERFACE     Interface_ID(0x667a0fc9, 0x41a81e59)
#define MEMSTREAMMGR_INTERFACE  Interface_ID(0x270517e7, 0x29f449e3)

// russom - 08/24/01
// Put 5k limit on line lengths
#define MEMSTREAM_MAXLINE	5120

// russom - 08/24/01
// Readstring was never fully implemented
#define MEMSTREAM_READSTRING	0

inline class IMemStreamMgr* GetMemStreamMgr( ) {
	return (IMemStreamMgr*)GetCOREInterface( MEMSTREAMMGR_INTERFACE );
}

// IMemStream function published enums
enum SeekMode {
	seek_set, seek_cur, seek_end,
};

enum MemStreamEnums {
	MemStreamSeekMode,
};

// ============================================================================
//
// class IMemStream
//
// ----------------------------------------------------------------------------
class IMemStream : public FPMixinInterface
{
public:
	// Basic token parsing.
	virtual TCHAR* GetFileName() = 0;
	virtual void SkipSpace( ) = 0;
	virtual TCHAR* ReadChar( ) = 0;
	virtual TCHAR* PeekChar( ) = 0;
#if MEMSTREAM_READSTRING	// russom - 08/24/01
	virtual TCHAR* ReadString( ) = 0;
#endif
	virtual TCHAR* ReadLine( ) = 0;
	virtual TCHAR* ReadToken( ) = 0;
	virtual TCHAR* PeekToken( ) = 0;
	virtual void UnReadToken( ) = 0;
	virtual TCHAR* ReadBlock( const TCHAR *open, const TCHAR *close ) = 0;
	virtual void SkipBlock( const TCHAR *open, const TCHAR *close ) = 0;

	// Position / size operations.
	virtual int Pos( ) = 0;
	virtual void Seek( int offset, int origin ) = 0;
	virtual int Size( ) = 0;
	virtual bool EOS( ) = 0;  // returns true if at end of stream

	virtual FPInterfaceDesc* GetDesc( );

	// function IDs 
	enum {
		fnGetFileName,
		fnSkipSpace,
		fnReadChar,
		fnPeekChar,
#if MEMSTREAM_READSTRING	// russom - 08/24/01
		fnReadString,
#endif
		fnReadLine,
		fnReadToken,
		fnPeekToken,
		fnUnReadToken,
		fnReadBlock,
		fnSkipBlock,
		fnPos,
		fnSeek,
		fnSize,
		fnEOS,
	};

	// dispatch map
	BEGIN_FUNCTION_MAP
		FN_0( fnGetFileName, TYPE_FILENAME, GetFileName );
		VFN_0( fnSkipSpace, SkipSpace );
		FN_0( fnReadChar, TYPE_STRING, ReadChar );
		FN_0( fnPeekChar, TYPE_STRING, PeekChar );
#if MEMSTREAM_READSTRING	// russom - 08/24/01
		FN_0( fnReadString, TYPE_STRING, ReadString );
#endif
		FN_0( fnReadLine, TYPE_STRING, ReadLine );
		FN_0( fnReadToken, TYPE_STRING, ReadToken );
		FN_0( fnPeekToken, TYPE_STRING, PeekToken );
		VFN_0( fnUnReadToken, UnReadToken );
		FN_2( fnReadBlock, TYPE_STRING, ReadBlock, TYPE_STRING, TYPE_STRING );
		VFN_2( fnSkipBlock, SkipBlock, TYPE_STRING, TYPE_STRING );
		FN_0( fnPos, TYPE_INT, Pos );
		VFN_2( fnSeek, Seek, TYPE_INT, TYPE_ENUM );
		FN_0( fnSize, TYPE_INT, Size );
		FN_0( fnEOS, TYPE_bool, EOS );
	END_FUNCTION_MAP
};


// ============================================================================
class IMemStreamMgr : public FPStaticInterface
{
public:
	virtual IMemStream* OpenFile( const TCHAR *fname ) = 0;
	virtual IMemStream* OpenString( const TCHAR *str ) = 0;
	virtual void Close( FPInterface *memStream ) = 0;

	// function IDs 
	enum {
		fnOpenFile, fnOpenString, fnClose,
	}; 
};

// ============================================================================
// Utility functions.
static TCHAR whitespace[] = _T(" \t\n\r");
inline bool CharInStr(TCHAR c, TCHAR *str)
{
	while(*str) if(c == *str++) return true;
	return false;
}
inline bool IsSpace(TCHAR c) { return CharInStr(c, whitespace); }
inline bool IsNewline(TCHAR c) { return (c=='\n'||c=='\r'); }


#endif //__IMEMSTREAM_H__
