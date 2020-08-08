// ============================================================================
// MemStream.cpp
//
// History:
//  02.25.01 - Created by Simon Feltman
//
// Copyright ©2001, Discreet
// ----------------------------------------------------------------------------
#include "iMemStream.h"


// ============================================================================
//
// IMemStream Interface Descriptor
//
// ----------------------------------------------------------------------------
static FPInterfaceDesc memStreamInterfaceDesc(
	MEMSTREAM_INTERFACE, _T("memStream"), 0, NULL, FP_MIXIN,

	/*functions*/
		IMemStream::fnGetFileName, _T("getFileName"), 0, TYPE_FILENAME, 0, 0,
		IMemStream::fnSkipSpace, _T("skipSpace"), 0, TYPE_VOID, 0, 0,
		IMemStream::fnReadChar, _T("readChar"), 0, TYPE_STRING, 0, 0,
		IMemStream::fnPeekChar, _T("peekChar"), 0, TYPE_STRING, 0, 0,
#if MEMSTREAM_READSTRING	// russom - 08/24/01
		IMemStream::fnReadString, _T("readString"), 0, TYPE_STRING, 0, 0,
#endif
		IMemStream::fnReadLine, _T("readLine"), 0, TYPE_STRING, 0, 0,
		IMemStream::fnReadToken, _T("readToken"), 0, TYPE_STRING, 0, 0,
		IMemStream::fnPeekToken, _T("peekToken"), 0, TYPE_STRING, 0, 0,
		IMemStream::fnUnReadToken, _T("unReadToken"), 0, TYPE_VOID, 0, 0,
		IMemStream::fnReadBlock, _T("readBlock"), 0, TYPE_STRING, 0, 2,
			_T("open"), 0, TYPE_STRING,
			_T("close"), 0, TYPE_STRING,
		IMemStream::fnSkipBlock, _T("skipBlock"), 0, TYPE_VOID, 0, 2,
			_T("open"), 0, TYPE_STRING,
			_T("close"), 0, TYPE_STRING,
		IMemStream::fnPos, _T("pos"), 0, TYPE_INT, 0, 0,
		IMemStream::fnSeek, _T("seek"), 0, TYPE_VOID, 0, 2,
			_T("offset"), 0, TYPE_INT,
			_T("origin"), 0, TYPE_ENUM, MemStreamSeekMode,
		IMemStream::fnSize, _T("size"), 0, TYPE_INT, 0, 0,
		IMemStream::fnEOS, _T("eos"), 0, TYPE_bool, 0, 0,

	enums,
		MemStreamSeekMode, 3,
			_T("seek_set"), seek_set,
			_T("seek_cur"), seek_cur,
			_T("seek_end"), seek_end,

	end
);
FPInterfaceDesc* IMemStream::GetDesc() { return &memStreamInterfaceDesc; }


// ============================================================================
//
// class CMemStream
//
// ----------------------------------------------------------------------------
class CMemStream : public IMemStream
{
protected:
	TCHAR *m_start, *m_end, *m_last, *m_next, *m_cur;
	TSTR m_fname;

public:
	CMemStream( );
	~CMemStream( );
	bool OpenFile( const TCHAR *fname );
	bool OpenString( const TCHAR *str );
	void Close( );

	// From IMemStream
	TCHAR* GetFileName( );
	void SkipSpace( );
	TCHAR* ReadChar( );
	TCHAR* PeekChar( );
#if MEMSTREAM_READSTRING	// russom - 08/24/01
	TCHAR* ReadString( );
#endif
	TCHAR* ReadLine( );
	TCHAR* ReadToken( );
	TCHAR* PeekToken( );
	void UnReadToken( );
	TCHAR* ReadBlock( const TCHAR *open, const TCHAR *close );
	void SkipBlock( const TCHAR *open, const TCHAR *close );
	int Pos( );
	void Seek( int offset, int origin );
	int Size( );
	bool EOS( );
};

// ============================================================================
CMemStream::CMemStream( )
{
	m_start = m_end = m_last = m_next = m_cur = NULL;
}
CMemStream::~CMemStream( ) { Close(); }

// ============================================================================
bool CMemStream::OpenFile( const TCHAR *fname )
{
	Close();
	FILE *f = fopen(fname, _T("rb"));
	if(f == NULL)
		return false;

	m_fname = fname;
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	m_start = new TCHAR[size];

	fseek(f, 0, SEEK_SET);
	fread(m_start, 1, size, f);
	fclose(f);

	m_cur = m_start;
	m_end = m_start + size;
	return true;
}

bool CMemStream::OpenString( const TCHAR *str )
{
	Close();
	int size = static_cast<int>(_tcslen(str));	// SR DCAST64: Downcast to 2G limit.
	m_start = new TCHAR[size+1];
	if(m_start == NULL)
		return false;

	strcpy(m_start, str);
	m_cur = m_start;
	m_end = m_start + size;
	return true;
}

void CMemStream::Close( )
{
	if(m_start) delete[] m_start;
	m_start = m_end = m_last = m_next = m_cur = NULL;
}

// ============================================================================
TCHAR* CMemStream::GetFileName( )
{
	return (TCHAR*)m_fname;
}

void CMemStream::SkipSpace( )
{
	while(!EOS() && IsSpace(*m_cur)) m_cur++;
}

TCHAR* CMemStream::ReadChar( )
{
	static TCHAR ch[2];
	ch[0] = *m_cur++;
	ch[1] = _T('\0');
	return ch;
}
TCHAR* CMemStream::PeekChar( )
{
	static TCHAR ch[2];
	ch[0] = *m_cur;
	ch[1] = _T('\0');
	return ch;
}

#if MEMSTREAM_READSTRING	// russom - 08/24/01
TCHAR* CMemStream::ReadString( )
{
	return NULL;
}
#endif

TCHAR* CMemStream::ReadLine( )
{
	static TCHAR buf[MEMSTREAM_MAXLINE];
	TCHAR *b = buf;
	int iBufLen = 0;

	// russom - 07/11/01
	// reset m_next to NULL.  This will prevent
	// a call to ReadToken from using an obsolete
	// m_next value.
	m_next = NULL;

	TCHAR c = *m_cur++;
	while( !IsNewline(c) && ((iBufLen++) < MEMSTREAM_MAXLINE)) {
		*b++ = c;
		c = *m_cur++;
	}
	*b = _T('\0');

	// Make sure to skip CR/LF combos.
	if(c == _T('\r') && *m_cur == _T('\n'))
		m_cur++;

	return buf;
}

// ============================================================================
TCHAR* CMemStream::ReadToken( )
{
	static TCHAR token[MEMSTREAM_MAXLINE];
	int iBufLen = 0;

	m_last = m_cur;

	if(m_next)
	{
		m_cur = m_next;
		m_next = NULL;
		return token;
	}
	TCHAR *t = token; *t = _T('\0');

retry:;
	// Skip leading space
	SkipSpace();
	if(EOS()) return NULL;

	// Skip '//' style comments
	TCHAR c = *m_cur++;
	if(c == _T('/') && *m_cur == _T('/'))
	{
		while(!EOS() && !IsNewline(c=*m_cur++)) ;
		goto retry;
	}

	if(c == _T('"'))
	{
		while(!EOS() && ((c=*m_cur++) != _T('"')) && ((iBufLen++) < MEMSTREAM_MAXLINE))
			*t++ = c;
	}
	else
	{
		do {
			*t++ = c;
		} while(!EOS() && !IsSpace(c=*m_cur++) && ((++iBufLen) < MEMSTREAM_MAXLINE));
	}
	*t = _T('\0');
	return token;
}

TCHAR* CMemStream::PeekToken( )
{
	TCHAR *pos = m_cur;
	// russom - 08/23/01
	// restore value of m_last - it's reset in ReadToken
	TCHAR *last = m_last;	
	TCHAR *tok = ReadToken();
	m_last = last;
	m_next = m_cur;
	m_cur = pos;
	return tok;
}

void CMemStream::UnReadToken( )
{
	if(m_last) {
		// russom - 08/23/01
		// can't use m_next optimization with unreadtoken
		// the static token string stored in ReadToken is
		// now invalid
		// m_next = m_cur;
		m_next = NULL;
		m_cur = m_last;
		m_last = NULL;
	}
}

// ============================================================================
TCHAR* CMemStream::ReadBlock( const TCHAR *open, const TCHAR *close )
{
	static TCHAR buf[MEMSTREAM_MAXLINE];

	// Find the exact starting point of the next block.
	TCHAR *tok = ReadToken();
	if(tok == NULL || stricmp(tok, open) != 0)
	{
		UnReadToken();
		return NULL;
	}
	TCHAR *start = m_cur - (strlen(open)+1);
	m_cur = start;

	// Skip the block.
	SkipBlock(open, close);

	// Block might be to big for buffer so check.
	int size = m_cur - start;
	if(size >= MEMSTREAM_MAXLINE)
		return NULL;

	strncpy(buf, start, size-1);
	buf[size-1] = _T('\0');

	return buf;
}

void CMemStream::SkipBlock( const TCHAR *open, const TCHAR *close )
{
	int count = 0;
	TCHAR *tok = NULL;
	do {
		if((tok = ReadToken()) == NULL) break;
		if(stricmp(tok, open) == 0)
			count++;
		else if(stricmp(tok, close) == 0)
			count--;
	} while(count);
}


// ============================================================================
int CMemStream::Pos( ) { return m_cur-m_start; }

void CMemStream::Seek( int offset, int origin )
{
	switch(origin)
	{
	case seek_set:
		m_cur = m_start + offset;
		break;
	case seek_cur:
		m_cur = m_cur + offset;
		break;
	case seek_end:
		m_cur = m_end + offset;
		break;
	}

	// russom - 08/23/01
	// If the user reset the m_cur location, invalidate
	// the m_next pointer so the static token string in
	// ReadToken will not be used.  Also, set m_last
	// to null to make the next call to UnreadToken
	// a no-op
	m_next = NULL;
	m_last = NULL;
}

int CMemStream::Size( ) { return m_end-m_start; }

bool CMemStream::EOS( ) { return (m_cur >= m_end ? true : false); }


// ============================================================================
//
// class CMemStreamMgr
//
// ----------------------------------------------------------------------------
class CMemStreamMgr : public IMemStreamMgr 
{
public:
	IMemStream* OpenFile( const TCHAR *fname );
	IMemStream* OpenString( const TCHAR *str );
	void Close( FPInterface *memStream );

	DECLARE_DESCRIPTOR(CMemStreamMgr)
	BEGIN_FUNCTION_MAP
		FN_1(fnOpenFile, TYPE_INTERFACE, OpenFile, TYPE_FILENAME);
		FN_1(fnOpenString, TYPE_INTERFACE, OpenString, TYPE_STRING);
		VFN_1( fnClose, Close, TYPE_INTERFACE );
	END_FUNCTION_MAP
};

// ============================================================================
//
// IMemStreamMgr Interface Descriptor
//
// ----------------------------------------------------------------------------
CMemStreamMgr memStreamMgr(
	MEMSTREAMMGR_INTERFACE, _T("memStreamMgr"), 0, NULL, FP_CORE,

	/*functions*/
		IMemStreamMgr::fnOpenFile, _T("openFile"), 0, TYPE_INTERFACE, 0, 1,
			_T("fname"), 0, TYPE_FILENAME,
		IMemStreamMgr::fnOpenString, _T("openString"), 0, TYPE_INTERFACE, 0, 1,
			_T("string"), 0, TYPE_STRING,
		IMemStreamMgr::fnClose, _T("close"), 0, TYPE_VOID, 0, 1,
			_T("memStream"), 0, TYPE_INTERFACE,

	end 
);

// ============================================================================
IMemStream* CMemStreamMgr::OpenFile( const TCHAR *fname )
{
	CMemStream *memStream = new CMemStream;
	if(!memStream->OpenFile(fname))
	{
		delete memStream;
		return NULL;
	}
	return memStream;
}
IMemStream* CMemStreamMgr::OpenString( const TCHAR *str )
{
	CMemStream *memStream = new CMemStream;
	if(!memStream->OpenString(str))
	{
		delete memStream;
		return NULL;
	}
	return memStream;
}
void CMemStreamMgr::Close( FPInterface *memStream )
{
	delete memStream;
}
