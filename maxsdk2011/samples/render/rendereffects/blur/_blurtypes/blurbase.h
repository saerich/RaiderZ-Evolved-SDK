/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: blurBase.h

	 DESCRIPTION: base blur type - class declaration

	 CREATED BY: michael malone (mjm)

	 HISTORY: created November 4, 1998

	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

#if !defined(_BLURBASE_H_INCLUDED_)
#define _BLURBASE_H_INCLUDED_

#include "max.h"
#include "..\globals.h"

class BlurMgr;
class BitmapBuf;
class BitmapColorAccessor;
class BitmapAlphaAccessor;
class BitmapAccessor;

inline int MAX( int a, int b ) { return ((a>b)? a:b); }
inline int MIN( int a, int b ) { return ((a>b)? b:a); }


// ----------------------------------------
// base blur type - class declaration
// ----------------------------------------
class BlurBase
{
protected:
	BlurMgr *const mp_blurMgr;
	int m_bufSz, m_mapSz, m_imageSz, m_imageW, m_imageH, m_brightenType;
	bool m_blurValid;
	BOOL m_affectAlpha;
	AColor *mp_blurCache, *mp_scratchMap;
	const TCHAR *mp_nameCurProcess;
	DWORD m_lastBMModifyID;

	int findRotPixels(int x1, int y1, int x2, int y2, IPoint2* buf = NULL, int sz = 0);
	void calcGaussWts(float *buf, int bufSz);
	void calcGaussWts(float *buf, int radA, int radB);
	void blendPixel(int index, BitmapAccessor &bmFrom, BitmapAccessor &bmTo, AColor blendCol, float brighten, float blend);
	virtual bool doBlur(CompMap *pCompMap, BitmapAccessor &bmFrom, BitmapAccessor &bmTo) { return false; }

public:
	BlurBase() : mp_blurMgr(NULL),
		m_bufSz(0), m_mapSz(0), m_imageSz(0), m_imageW(0), m_imageH(0), m_brightenType(0), m_blurValid(false), m_affectAlpha(FALSE),
		mp_blurCache(NULL), mp_scratchMap(NULL), mp_nameCurProcess(NULL), m_lastBMModifyID(0xFFFFFFFF) { }
	BlurBase(BlurMgr *const mgr) : mp_blurMgr(mgr),
		m_bufSz(0), m_mapSz(0), m_imageSz(0), m_imageW(0), m_imageH(0), m_brightenType(0), m_blurValid(false), m_affectAlpha(FALSE),
		mp_blurCache(NULL), mp_scratchMap(NULL), mp_nameCurProcess(NULL), m_lastBMModifyID(0xFFFFFFFF) { }
	virtual ~BlurBase() { }
	virtual void notifyPrmChanged(int prmID) { }
	virtual void blur(TimeValue t, CompMap *compositeMap, Bitmap *bm, RenderGlobalContext *gc) { return; };
};


// ---------- ---------- ---------- ----------
// Bitmap Buffer
// Exposes pixels of a bitmap as if they were a contiguous array,
// while internally buffering one line of the bitmap at a time.
// ---------- ---------- ---------- ----------

class BitmapBuf
{
	public:
		typedef LONGLONG Index;
		inline BitmapBuf( Bitmap* map_in, BOOL readOnly_in );
		inline ~BitmapBuf();
		inline BMM_Color_64& GetPixel( Index pixelIndex );

	protected:
		// Allocation sizes for optimal performance, tested empirically:
		//	128 for uniform blur, 4 for radial blur, 16 for compromise
		static const int BUF_SIZE_MAX=16;	// buffer allocation size in pixels
		BMM_Color_64 buf[ BUF_SIZE_MAX ];	// buffer storage

		Index bufIndex;						// current buffer pixel index
		int bufSize, lineSize;				// current buffer size and bitmap line size in pixels
		int bufX;							// current buffer X coordinate
		int bufY;							// current buffer Y coordinate; buffer never spans multiple lines
		BOOL readOnly;
		Bitmap* map;

		inline void Read();					// copy from bitmap into buffer
		inline void Write();				// copy from buffer into bitmap
		inline void RemapBuf( Index pixelIndex ); // point buffer to a different location in the bitmap
};

inline BitmapBuf::BitmapBuf( Bitmap* map_in, BOOL readOnly_in )
: bufIndex(0), bufSize(0), lineSize(0), bufX(0), bufY(0),
  readOnly(readOnly_in), map(map_in)
{
	lineSize = map->Width();
	RemapBuf(0);
}

inline BitmapBuf::~BitmapBuf()
{
	if( !readOnly ) Write(); //flush any unwritten pixels
}

inline BMM_Color_64& BitmapBuf::GetPixel( Index pixelIndex )
{
	// Performance optimization: Checking (offset<0) is avoided by forcing to unsigned, causing wraparound.
	unsigned int offset = pixelIndex - bufIndex;
	if( offset>=bufSize )
	{
		RemapBuf( pixelIndex ); // point buffer at new location, update member variables
		offset = pixelIndex - bufIndex;
	}
	return buf[ offset ];
}

// Copy from bitmap into buffer
inline void BitmapBuf::Read()
{
	map->GetPixels( bufX, bufY, bufSize, buf );
}

// Copy from buffer into bitmap
inline void BitmapBuf::Write()
{
	if( bufSize>0 ) // Prevent write before buffer is initialized
	{
		map->PutPixels( bufX, bufY, bufSize, buf );
	}
}

// Point buffer to a different location in the bitmap
inline void BitmapBuf::RemapBuf( Index pixelIndex )
{
	// Copy old line from buffer to bitmap if necessary
	if( !readOnly ) Write();

	// Update variables
	bufIndex = pixelIndex;
	bufY = bufIndex / lineSize;
	bufX = bufIndex - (bufY * lineSize);

	int bufX_back = MIN( lineSize, (bufX + BUF_SIZE_MAX) );
	bufSize = (bufX_back - bufX);

	// Copy new line from bitmap to buffer
	Read();
}


// ---------- ---------- ---------- ----------
// Bitmap Color Accessor
// ---------- ---------- ---------- ----------

class BitmapColorAccessor
{
	public:
		typedef LONGLONG Index;
		inline BitmapColorAccessor( BitmapBuf* buf ) : buf(buf) {};
		inline WORD& operator[]( Index wordIndex );
		inline operator int() {return (buf==NULL? 0:1);} // returns zero if buffer is NULL

	protected:
		BitmapBuf* buf;
};

// Returns red, green, or blue component of a pixel.
inline WORD& BitmapColorAccessor::operator[]( Index wordIndex )
{
	// Color map is considered a flat array of words, each pixel containing three words.
	// See uses of fromMap[] and toMap[] in the code.

	// For red,    wordIndex = (pixelIndex*3)
	// For green,  wordIndex = (pixelIndex*3) + 1
	// For blue,   wordIndex = (pixelIndex*3) + 2

	// Pixel index is word index divided by three
	// Component index is word index modulo three (0 for red, 1 for green, 2 for blue)
	Index pixelIndex = wordIndex / 3;
	Index pixelComponentOffset = wordIndex - (pixelIndex*3);
	BMM_Color_64& pixel = buf->GetPixel( pixelIndex );
	WORD* pixelComponents = reinterpret_cast<WORD*>(&pixel);
	return *(pixelComponents + pixelComponentOffset);
}


// ---------- ---------- ---------- ----------
// Bitmap Alpha Accessor
// ---------- ---------- ---------- ----------

class BitmapAlphaAccessor
{
	public:
		typedef LONGLONG Index;
		inline BitmapAlphaAccessor( BitmapBuf* buf_in ) : buf(buf_in) {};
		inline WORD& operator[]( Index wordIndex );
		inline operator int() {return (buf==NULL? 0:1);} // returns zero if buffer is NULL

	protected:
		BitmapBuf* buf;
};

// Returns alpha component of a pixel.
inline WORD& BitmapAlphaAccessor::operator[]( Index wordIndex )
{
	// Alpha map is considered a flat array of words, each pixel containing one word.

	// Pixel index is equivalent to word index
	Index pixelIndex = wordIndex;
	BMM_Color_64& pixel = buf->GetPixel( pixelIndex );
	return pixel.a;
}

// ---------- ---------- ---------- ----------
// Bitmap Accessor
// ---------- ---------- ---------- ----------

class BitmapAccessor
{
	public:
		BitmapBuf buf;
		BitmapColorAccessor map;
		BitmapAlphaAccessor alpha;
		BitmapAccessor( Bitmap* map_in, BOOL readOnly_in, BOOL hasAlpha_in )
		: buf( map_in, readOnly_in ), map( &buf ), alpha( hasAlpha_in? &buf : NULL )
		{ }
};

#endif // !defined(_BLURBASE_H_INCLUDED_)
