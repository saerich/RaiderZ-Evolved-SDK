/**********************************************************************
 *<
	FILE: BrushPresets.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2004, All Rights Reserved.
 **********************************************************************/

#include "BrushPresets.h"

#include "3dsmaxport.h"

#define DEFAULTFILENAME			_T("DefaultUI.bpr")		//LOC_Notes: do not localize this
#define STARTFILENAME			_T("MaxStartUI.bpr")	//LOC_Notes: do not localize this

#define MAX_NUM_PRESETS			50
#define ID_TOOL_PRESET_MIN		10001	//Tool ID of the first possible preset button
#define ID_TOOL_PRESET_MAX		10050	//Tool ID of the last possible preset button
#define ID_TOOL_ADDPRESET		10051
#define ID_TOOL_PRESETMGR		10052

#define PRESETCFG_TYPE_WORLD	_T("WORLD")
#define PRESETCFG_TYPE_FLOAT	_T("FLOAT")
#define PRESETCFG_TYPE_INT		_T("INT")
#define PRESETCFG_TYPE_BOOL		_T("BOOL")
#define PRESETCFG_TYPE_POINT3	_T("POINT3")
#define PRESETCFG_TYPE_COLOR	_T("COLOR")
#define PRESETCFG_TYPE_STRING	_T("STRING")
#define PRESETCFG_TYPE_CURVEPTS _T("CURVEPTS")
#define PRESETCFG_ENABLED		_T("ENABLED")
#define PRESETCFG_DISABLED		_T("DISABLED")
#define PRESETCFG_NUMPRESETS	_T("NumPresets")
#define PRESETCFG_NUMCONTEXTS	_T("NumContexts")
#define PRESETCFG_ICONMIN		_T("IconMin")
#define PRESETCFG_ICONMAX		_T("IconMax")
#define PRESETCFG_PRESETID		_T("PresetID")
#define PRESETCFG_CONTEXTID		_T("ContextID")
#define PRESETCFG_CONTEXTNAME	_T("ContextName")
#define PRESETCFG_PLUGINCLASSID	_T("PluginClassID")
#define PRESETCFG_PLUGINSUPERCLASSID _T("PluginSuperclassID")
//#define PRESETCFG_NAME			_T("Name")
//#define PRESETCFG_BUTTONPOS		_T("ButtonPos")

#define CURVE_CLASS_ID Class_ID(0x5f2d3a0a, 0x4be75269)


//-----------------------------------------------------------------------------
// forward declarations

inline TCHAR* ReadToken( TCHAR* str, TCHAR* buffer, TCHAR delim  );


//-----------------------------------------------------------------------------
// static instances

static BrushPresetMgr theBrushPresetMgr;
BrushPresetMgr* GetBrushPresetMgr() {return &theBrushPresetMgr;}

static StdBrushPresetContext theStdBrushPresetContext;
StdBrushPresetContext* GetStdBrushPresetContext() {return &theStdBrushPresetContext;}

static IPainterInterface_V5* thePainterInterface = NULL;
IPainterInterface_V5*	GetPainterInterface() { return thePainterInterface; }


IPainterInterface_V5*	CreatePainterInterface()
{ return (IPainterInterface_V5*)(GetCOREInterface()->CreateInstance(REF_TARGET_CLASS_ID,PAINTERINTERFACE_CLASS_ID)); }

StdBrushPresetParams* GetStdBrushPresetParams( IBrushPreset* preset )
{ return (StdBrushPresetParams*)preset->GetParams(STDBRUSHPRESETCONTEXT_ID); }


//-----------------------------------------------------------------------------
//-- Utilities

#define MAX(a,b) ((a)<(b)? (b):(a))
#define MIN(a,b) ((a)>(b)? (b):(a))

inline float Clamp( float x, float lo, float hi ) {
	if( x>=hi ) return hi;
	if( x<=lo ) return lo;
	return x;
}

inline float Interp( float w, float lower, float upper ) {
	return (upper*w) + (lower*(1.0f-w));
}

inline RGBTRIPLE Interp( float w, RGBTRIPLE& lower, RGBTRIPLE& upper ) {
	RGBTRIPLE retval;
	retval.rgbtRed		= (upper.rgbtRed * w)	+ (lower.rgbtRed * (1.0f-w));
	retval.rgbtGreen	= (upper.rgbtGreen * w) + (lower.rgbtGreen * (1.0f-w));
	retval.rgbtBlue		= (upper.rgbtBlue * w)	+ (lower.rgbtBlue * (1.0f-w));
	return retval;
}

RGBTRIPLE COLORREFtoRGBTRIPLE( COLORREF c ) {
	RGBTRIPLE retval = { (c&0xFF0000)>>16, (c&0xFF00)>>8, c&0xFF };
	return retval;
}

RGBTRIPLE ColorToRGBTRIPLE( Color& c ) {
	RGBTRIPLE retval;
	retval.rgbtRed = c.r*255, retval.rgbtGreen = c.g*255, retval.rgbtBlue = c.b*255;
	return retval;
}

RGBTRIPLE GetStdColor( int id ) {
	IColorManager* colorman = ColorMan();
	return COLORREFtoRGBTRIPLE( colorman->GetColor(id) );
}

void SetWindowPos( HWND hWnd, int x, int y, HWND relativeTo ) {
	if( !hWnd ) return;

	RECT rect, offset;
	GetWindowRect( hWnd, &rect );
	if( relativeTo!=NULL )
		 GetWindowRect( relativeTo, &offset );
	else offset.left = offset.top = 0;
	int absX = x+offset.left,		absY = y+offset.top;
	int w = rect.right-rect.left,	h = rect.bottom-rect.top;

	//IMPORTANT: this works with WM_MOVING but not WM_MOVE messages, because WM_MOVE reports
	//the coordinates for the client space, and we need the coordiantes for the whole window
	::SetWindowPos( hWnd, HWND_TOP, absX, absY, w, h, SWP_NOACTIVATE );
}

ClassDesc* LoadClass( Class_ID classID, SClass_ID superclassID )
{
	ClassDirectory& cdir = GetCOREInterface()->GetDllDir().ClassDir();
	return cdir.FindClassEntry( superclassID, classID )->FullCD();
}

//-----------------------------------------------------------------------------
//-- Bitmap Utilitues
// FIXME: WinImageList does not work correctly with multiple images per list (only supports 1 image)   :(

LPBITMAPINFO CreateWinBitmap(int width, int height) {
	HDC hdc = GetDC(NULL);
	int biSizeImage = (3 * width * height);
	LPBITMAPINFO pbmi = (LPBITMAPINFO)malloc( sizeof(BITMAPINFOHEADER) + biSizeImage );
	if (pbmi) {
		BITMAPINFOHEADER& header = pbmi->bmiHeader;
		header.biSize			= sizeof(BITMAPINFOHEADER);
		header.biWidth			= width;
		header.biHeight			= height; //use negative height to specify top-down bitmap?
		header.biPlanes			= 1;
		header.biBitCount		= 24;
		header.biCompression	= BI_RGB;
		header.biSizeImage		= biSizeImage;
		header.biXPelsPerMeter	= GetDeviceCaps(hdc, LOGPIXELSX);
		header.biYPelsPerMeter	= GetDeviceCaps(hdc, LOGPIXELSY);
		header.biClrUsed		= 0;
		header.biClrImportant	= 0;
	}
	ReleaseDC(NULL,hdc);
	return pbmi;
}
void DeleteWinBitmap( LPBITMAPINFO bitmap ) {
	if( bitmap!=NULL ) free( bitmap );
}

void WinImageList::Init( int width, int height ) {
	if( imagelist!=NULL ) Free();
	imagelist = ImageList_Create( width, height, ILC_COLOR24 | ILC_MASK, 1, 1 );
	this->width = width;
	this->height = height;
}

void WinImageList::Free() {
	for( int i=0; i<images.Count(); i++ )	DeleteWinBitmap( images[i] );
	for( int i=0; i<masks.Count(); i++ )	DeleteWinBitmap( masks[i] );
	images.SetCount(0);
	masks.SetCount(0);
	ImageList_Destroy( imagelist );
	imagelist = NULL;
}

int WinImageList::Count() {
	if( imagelist==NULL ) return 0;
	return ImageList_GetImageCount( imagelist );
}

int WinImageList::AddImages( int n ) {
	HDC hdc = GetDC(NULL);
	HBITMAP hImage = CreateCompatibleBitmap( hdc, width, height );
	HBITMAP hMask = CreateCompatibleBitmap( hdc, width, height );
	int index = 0;

	for( int i=0; i<n; i++ ) {
		LPBITMAPINFO image	= CreateWinBitmap( width, height );
		LPBITMAPINFO mask	= CreateWinBitmap( width, height );
		images.Append( 1, &image );
		masks.Append( 1, &mask );

		index = ImageList_Add( imagelist, hImage, hMask );
	}

	DeleteObject( hImage );
	DeleteObject( hMask );
	ReleaseDC(NULL,hdc);
	return index;
}


void WinImageList::UpdateImage( int i ) {
	HDC hdc = GetDC(NULL);
	LPBITMAPINFO image = images[i];
	LPBITMAPINFO mask = masks[i];

	HBITMAP hImage = CreateCompatibleBitmap( hdc, width, height );
	HBITMAP hMask = CreateCompatibleBitmap( hdc, width, height );
	SetDIBits( hdc, hImage, 0, height, image->bmiColors, image, DIB_RGB_COLORS );
	SetDIBits( hdc, hMask, 0, height, mask->bmiColors, mask, DIB_RGB_COLORS );

	ImageList_Replace( imagelist, i, hImage, hMask );
	DeleteObject( hImage );
	DeleteObject( hMask );
	ReleaseDC(NULL,hdc);
}

RGBTRIPLE* WinImageList::GetImage( int i ) {
	if( (i<0) || (images.Count()<=i) )
		return NULL;
	return (RGBTRIPLE*)(&(images[i]->bmiColors[0]));
}

RGBTRIPLE* WinImageList::GetMask( int i ) {
	if( (i<0) || (masks.Count()<=i) )
		return NULL;
	return (RGBTRIPLE*)(&(masks[i]->bmiColors[0]));
}


//-----------------------------------------------------------------------------
//-- FPValueInterfaceType Methods

void AcquireIfInterface( FPValue& val ) {
	if( (val.type==TYPE_INTERFACE) && (val.fpi!=NULL) && (val.fpi->LifetimeControl()==BaseInterface::wantsRelease) )
		val.fpi->AcquireInterface();
}

void ReleaseIfInterface( FPValue& val ) {
	if( (val.type==TYPE_INTERFACE) && (val.fpi!=NULL) && (val.fpi->LifetimeControl()==BaseInterface::wantsRelease) )
		val.fpi->ReleaseInterface();
}

FPValueInterfaceType::FPValueInterfaceType( Interface_ID id ) {
	this->id = id;
	lifetime = BaseInterface::wantsRelease;
	refCount = 1; //Assume we are Aquired upon creation
}

BaseInterface* FPValueInterfaceType::GetInterface(Interface_ID id) {
	if( id==GetID() )
		return this;
	return NULL;
}

void FPValueInterfaceType::SetLifetime( LifetimeType lifetime ) {
	this->lifetime = lifetime;
}

BaseInterface* FPValueInterfaceType::AcquireInterface() {
	refCount++;
	return this;
}
void FPValueInterfaceType::ReleaseInterface() {
	refCount--;
	if( (refCount==0) && (lifetime==BaseInterface::wantsRelease) )  delete this;
}

//-----------------------------------------------------------------------------
//-- Curve Methods

//ICurve* CreateICurve() {
//	//FIXME: hack ... assuming that the superclass is "ref maker" is not reasonable
//	return (ICurve*)(GetCOREInterface()->CreateInstance( REF_MAKER_CLASS_ID, CURVE_CLASS_ID ));
//}
//void DeleteICurve( ICurve* curve ) {
//	delete curve;
//}

CurvePts::CurvePts()
	: FPValueInterfaceType(CURVEIOBJECT_INTERFACE_ID)
{ }

class BezierSegment {
	public:
		Point2 p1, p2, p3, p4;
		Point2 Val( float t );
		Point2 Deriv( float t );
		float SolveX( float x, float err=0.001f, float t=0.5f, float tMin=0, float tMax=1 );
};

Point2 BezierSegment::Val( float t ) {
	// Bezier function:  P1*_T^3  +  3*P2 * _T^2*T  +  3*P3*T^2*_T  +   P4*T^3
	float _t = (1.0f-t);
	float tSq = t*t, _tSq = _t*_t;

	Point2 retval	 =	( p1*(_t*_tSq) );	// First term:	P1*_T^3
	       retval	+=	( p2*(3*_tSq*t) );	// Second term:	3*P2*_T^2*T
	       retval	+=	( p3*(3*_t*tSq) );	// Third term:	3*P3*T^2*_T
	return retval	+	( p4*(tSq*t) );		// Fourth term:	P4*T^3
}

Point2 BezierSegment::Deriv( float t ) {
	Point2 retval	 =	  ( (p4-p1) + 3*(p2-p3) ) * 3*t*t;
	       retval   +=	3*( (p1+p3) - (2*p2) ) * 2*t;
	return retval	+	3*(  p2-p1 );
}

// Assumes the X value of the segment is monotonically increasing.
// Uses Newton's method, combined with a binary search to narrow down an upper & lower bound
float BezierSegment::SolveX( float x, float err, float t, float tMin, float tMax ) {
	float xNext = Val( t ).x;
	float delta = x-xNext;
	if( (delta<err) && ((-delta)<err) ) return t;

	float tLo, tHi;
	if( delta>0 )	tMin = tLo = t,		tHi = 0.5f * (tMin + tMax);
	else			tMax = tHi = t,		tLo = 0.5f * (tMin + tMax);

	float deriv = Deriv( t ).x;
	// FIXME: deriv==0?

	float tNext = t + (delta / deriv);
	if( tNext<tLo ) tNext = tLo;
	if( tNext>tHi ) tNext = tHi;

	return SolveX( x, err, tNext, tMin, tMax );
}

float CurvePts::GetValue( float x ) {
	int N = GetNumPts();
	if( N==0 ) return 0;

	if( x<=curvePts[0].p.x )	return curvePts[0].p.y;
	if( x>=curvePts[N-1].p.x )	return curvePts[N-1].p.y;

   int i;
	for( i=0; i<(N-1); i++ )
		if( (curvePts[i].p.x<=x) && (curvePts[i+1].p.x>=x) ) break;

	BezierSegment seg;
	seg.p1 = curvePts[i].p;
	seg.p2 = (curvePts[i].p + curvePts[i].out);
	seg.p3 = (curvePts[i+1].p + curvePts[i+1].in);
	seg.p4 = curvePts[i+1].p;

	float span = curvePts[i+1].p.x - curvePts[i].p.x;
	float guess = (x-curvePts[i].p.x) / span;
	float t = seg.SolveX( x, 0.001f, guess );
	return seg.Val( t ).y;
}

void Assign( CurvePts* curve_output, ICurve* curve_input ) {
	int numPoints = curve_input->GetNumPts();
	curve_output->SetNumPts( numPoints );

	for( int i=0; i<numPoints; i++ )
		curve_output->SetPoint( i, curve_input->GetPoint( 0, i ) );
}

void Assign( ICurve* curve_output, CurvePts* curve_input ) {
	int numPoints = curve_input->GetNumPts();
	curve_output->SetNumPts( numPoints );

	for( int i=0; i<numPoints; i++ ) 
		curve_output->SetPoint( 0, i, &(curve_input->GetPoint(i)), FALSE );
}

void Assign( CurvePts* curve_output, CurvePts* curve_input ) {
	int numPoints = curve_input->GetNumPts();
	curve_output->SetNumPts( numPoints );

	for( int i=0; i<numPoints; i++ ) 
		curve_output->SetPoint( i, curve_input->GetPoint(i) );
}

CurvePts* GetCurvePtsInterface( FPValue* val ) {
	if( (val==NULL) || (val->type!=TYPE_INTERFACE) || (val->fpi==NULL) )
		return NULL;

	BaseInterface* iface = val->fpi->GetInterface( CURVEIOBJECT_INTERFACE_ID );
	return (CurvePts*)iface;
}


//-----------------------------------------------------------------------------
//-- String helper functions

inline int StringToInt(TCHAR *cp) { return _tcstol(cp, NULL, 10); }
inline DWORD StringToDWORD(TCHAR *cp) { return _tcstol(cp, NULL, 10); }
inline float StringToFloat(TCHAR *cp) { return _tcstod(cp, NULL); }
inline Class_ID StringToClassID(TCHAR *cp) {
	ULONG a,b;
	int res = _stscanf(cp, "(0x%lX,0x%lX)", &a, &b);
	return (res==2?  Class_ID(a,b) : Class_ID(0,0));
}
inline void ClassIDToString( Class_ID classID, TCHAR* str ) {
	_stprintf( str, "(0x%lX,0x%lX)", classID.PartA(), classID.PartB() );
}


inline bool IsWhitespace( TCHAR c )	{ return (c==' ') || (c=='\t'); }
inline bool IsEndline( TCHAR c )		{ return (c=='\n') || (c=='\0'); }
inline bool IsDelimiter( TCHAR c )	{ return (c=='|'); }
inline bool IsQuote( TCHAR c )		{ return (c=='"'); }

// Given a string made of delimited tokens, read the next token.
// Returns the character after the delimiter (the start of the next token), or NULL for end of string
inline TCHAR* ReadToken( TCHAR* str, TCHAR* buffer, TCHAR delim  ) {
	while( IsWhitespace(*str) )  str++;
	if( IsQuote(*str) ) { //quoted token
		str++;
		// FIXME: Currently no support for escape sequences in quoted strings
		while( !IsQuote(*str) )	*buffer++ = *str++;
		while( (*str!=delim) && !IsEndline(*str) )	str++; //skip to the next token
	}
	else //non-quoted token
		while( (*str!=delim) && !IsEndline(*str) ) *buffer++ = *str++;

	*buffer = '\0';
	if( *str==delim ) return (str+1); //skip the delimiter character
	return NULL;
}

// Read a "context" entry from a Brush Preset config file
int ReadCfgContextString( TCHAR* str, Class_ID* contextID, TCHAR* contextName, int* numParams ) {
	TCHAR buf[1024], *next=str;

	next = ReadToken( next, buf, _T('|') );		// contextID
	*contextID = StringToClassID(buf);
	next = ReadToken( next, buf, _T('|') );		// contextName
	_tcscpy( contextName, buf );
	next = ReadToken( next, buf, _T('|') );		// numParams
	*numParams = StringToInt( buf );
	return TRUE;
}

// Read a "param" entry from a Brush Preset config file
int ReadCfgParamString( TCHAR* str, int* paramID, TCHAR* paramName, TCHAR* paramValue ) {
	TCHAR buf[1024], *next=str;

	next = ReadToken( next, buf, _T('|') );			// paramID
	*paramID = StringToInt( buf );
	next = ReadToken( next, buf, _T('|') );			// contextName
	_tcscpy( paramName, buf );
	//next = ReadToken( next, buf, _T('|') );			// paramToggle
	//if( !_tcsicmp( buf, PRESETCFG_ENABLED ) )			*paramToggle = TRUE;
	//if( !_tcsicmp( buf, PRESETCFG_DISABLED ) )		*paramToggle = FALSE;
	next = ReadToken( next, paramValue, _T('|') );	// paramValue
	return TRUE;
}


//-----------------------------------------------------------------------------
//-- Type conversion helper functions


void CurveToString( CurvePts* curve, TCHAR* str ) {
	TCHAR buf[256];
	str[0] = 0;

	int numPoints = curve->GetNumPts();
	_stprintf( str, "%d", numPoints );

	for( int i=0; i<numPoints; i++ ) {
		CurvePoint& point = curve->GetPoint( i );
		_stprintf( buf, ":(%.3f,%.3f),(%.3f,%.3f),(%.3f,%.3f),%d",
			point.p.x, point.p.y, point.in.x, point.in.y, point.out.x, point.out.y, point.flags );
		_tcscat( str, buf );
	}
}

void StringToCurve( TCHAR* str, CurvePts* curve ) {
	TCHAR* s, buf[256];
	s = ReadToken( str, buf, _T(':') );
	int numPoints = StringToInt(buf);
	curve->SetNumPts( numPoints );

	CurvePoint point;
	for( int i=0; i<numPoints; i++ ) {
		s = ReadToken( s, buf, _T(':') );
		_stscanf( buf, "(%f,%f),(%f,%f),(%f,%f),%d",
			&point.p.x, &point.p.y, &point.in.x, &point.in.y, &point.out.x, &point.out.y, &point.flags );
		curve->SetPoint( i, point );
	}
}

//int DecimalPlaces( float f, int max=5 ) {
//	int mult = 1;
//	for( int i=0; i<max; i++ ) {
//		if( (f*mult)==(int)(f*mult) ) return i;
//		mult *= 10;
//	}
//}

void ParamType2ToString( FPValue* val, TCHAR* str ) {
	TCHAR* s = NULL;
	switch( val->type ) {
	case TYPE_WORLD:		s = PRESETCFG_TYPE_WORLD;	break;
	case TYPE_FLOAT:		s = PRESETCFG_TYPE_FLOAT;	break;
	case TYPE_INT:			s = PRESETCFG_TYPE_INT;		break;
	case TYPE_BOOL:			s = PRESETCFG_TYPE_BOOL;	break;
	case TYPE_POINT3_BV:
	case TYPE_POINT3:		s = PRESETCFG_TYPE_POINT3;	break;
	case TYPE_COLOR_BV:
	case TYPE_COLOR:		s = PRESETCFG_TYPE_COLOR;	break;
	case TYPE_TSTR_BV:
	case TYPE_TSTR:			s = PRESETCFG_TYPE_STRING;	break;
	case TYPE_STRING:		s = PRESETCFG_TYPE_STRING;	break;
	case TYPE_INTERFACE:
		if( GetCurvePtsInterface( val )!=NULL )
			s = PRESETCFG_TYPE_CURVEPTS;
		break;
	}
	if( s==NULL ) str[0] = _T('\0');
	else _tcscpy( str, s );
}

void FPValueToUIString( FPValue* val, TCHAR* str ) {
	switch( val->type ) {
	case TYPE_WORLD:		_stprintf( str, "%s", FormatUniverseValue(val->f) ); break;
	case TYPE_FLOAT:		_stprintf( str, "%f", val->f );				break;
	case TYPE_INT:			_stprintf( str, "%i", val->i );				break;
	case TYPE_STRING:		_stprintf( str, "%s", val->s );				break;
	case TYPE_TSTR_BV:
	case TYPE_TSTR:			_stprintf( str, "%s", val->tstr->data() );	break;
	case TYPE_BOOL:
		_tcscpy( str, (val->i? GetString(IDS_ON) : GetString(IDS_OFF)) );
		break;
	default:
		str[0] = 0;
		break;
	}
}

void FPValueToString( FPValue* val, TCHAR* str ) {
	ParamType2ToString( val, str );
	str += _tcslen(str); //append to end of string

	switch( val->type ) {
	case TYPE_WORLD:
	case TYPE_FLOAT:		_stprintf( str, ":%f", val->f );
		break;
	case TYPE_INT:
	case TYPE_BOOL:			_stprintf( str, ":%i", val->i );
		break;
	case TYPE_POINT3_BV:
	case TYPE_POINT3:
	case TYPE_COLOR_BV:
	case TYPE_COLOR:		_stprintf( str, ":(%f,%f,%f)", val->p->x, val->p->y, val->p->z );
		break;
	case TYPE_TSTR_BV:
	case TYPE_TSTR:			_stprintf( str, ":%s", val->tstr->data() );
		break;
	case TYPE_STRING:		_stprintf( str, ":%s", val->s );
		break;
	case TYPE_INTERFACE:
		{
			CurvePts* curve = GetCurvePtsInterface( val );
			if( curve!=NULL ) {
				TCHAR buf[1024];
				CurveToString( curve, buf );
				_stprintf( str, ":%s", buf );
			}
		}
		break;
	default:
		str[0] = 0;
		break;
	}
}

void StringToFPValue( TCHAR* str, FPValue* val ) {
	if( str==NULL || str[0]==0 ) {
		val->type = TYPE_VOID;
		return;
	}

	TCHAR *s = str;
	TCHAR typeStr[256], valStr[1024];

	// read in the type string
   int i;
   for( i=0; (*s!=0) && (*s!=_T(':')); i++ )	typeStr[i] = *(s++);
   typeStr[i] = 0; // add end-of-string
   
	if( *s==_T(':') ) s++; //skip the delimiter

	// read in the value string
   for( i=0; (*s!=0); i++ )	valStr[i] = *(s++);
   valStr[i] = 0; // add end-of-string

	//FIXME: optimize this with bsearch()
	if(      !_tcsicmp( typeStr, PRESETCFG_TYPE_WORLD ) )		val->type = (ParamType2)TYPE_WORLD;
	else if( !_tcsicmp( typeStr, PRESETCFG_TYPE_FLOAT ) )		val->type = (ParamType2)TYPE_FLOAT;
	else if( !_tcsicmp( typeStr, PRESETCFG_TYPE_INT ) )			val->type = (ParamType2)TYPE_INT;
	else if( !_tcsicmp( typeStr, PRESETCFG_TYPE_BOOL ) )		val->type = (ParamType2)TYPE_BOOL;
	else if( !_tcsicmp( typeStr, PRESETCFG_TYPE_POINT3 ) )		val->type = (ParamType2)TYPE_POINT3_BV;
	else if( !_tcsicmp( typeStr, PRESETCFG_TYPE_COLOR ) )		val->type = (ParamType2)TYPE_COLOR_BV;
	else if( !_tcsicmp( typeStr, PRESETCFG_TYPE_STRING ) )		val->type = (ParamType2)TYPE_TSTR_BV;
	else if( !_tcsicmp( typeStr, PRESETCFG_TYPE_CURVEPTS ) )	{ // Special handling for "Curve Points" type
		CurvePts* curve = new CurvePts();
		StringToCurve( valStr, curve );
		val->type = TYPE_INTERFACE;
		val->fpi = curve;
	}
	else val->type = TYPE_VOID, val->ptr = NULL;

	switch( val->type ) {
	case TYPE_WORLD:
	case TYPE_FLOAT:		_stscanf( valStr, "%f", &val->f );
		break;
	case TYPE_INT:			_stscanf( valStr, "%i", &val->i );
		break;
	case TYPE_BOOL:			_stscanf( valStr, "%i", &val->i );
		break;
	case TYPE_POINT3_BV:	val->p = new Point3;
							_stscanf( valStr, "(%f,%f,%f)", &(val->p->x), &(val->p->y), &(val->p->z) );
		break;
	case TYPE_COLOR_BV:		val->clr = new Color;
							_stscanf( valStr, "(%f,%f,%f)", &(val->clr->r), &(val->clr->g), &(val->clr->b) );
		break;
	case TYPE_TSTR_BV:		val->tstr = new TSTR( valStr );
		break;
	}
}


//-----------------------------------------------------------------------------
//-- CUIFramePtr - Smart pointer

class CUIFramePtr {
	public:
		ICUIFrame* frame;
		CUIFramePtr( HWND hwnd )						{ frame = (hwnd==NULL? NULL : GetICUIFrame(hwnd)); }
		CUIFramePtr( ICUIFrame* frame )					{ this->frame = frame; }
		~CUIFramePtr()									{ if( frame!=NULL ) ReleaseICUIFrame( frame ); }
		ICUIFrame* operator->()							{ return frame; }
		ICUIFrame* operator=( ICUIFrame* ptr )			{ frame=ptr; return frame; }
		BOOL operator==( ICUIFrame* ptr )				{ return frame==ptr; }
		BOOL operator!=( ICUIFrame* ptr )				{ return frame!=ptr; }
};


//-----------------------------------------------------------------------------
//-- ToolbarPtr - Smart pointer

class ToolbarPtr {
	public:
		ICustToolbar* toolbar;
		ToolbarPtr( HWND hwnd )							{ toolbar = (hwnd==NULL? NULL : GetICustToolbar(hwnd)); }
		ToolbarPtr( ICustToolbar* toolbar )				{ this->toolbar = toolbar; }
		~ToolbarPtr()									{ if( toolbar!=NULL ) ReleaseICustToolbar( toolbar ); }
		ICustToolbar* operator->()						{ return toolbar; }
		ICustToolbar* operator=( ICustToolbar* ptr )	{ toolbar=ptr; return toolbar; }
		BOOL operator==( ICustToolbar* ptr )			{ return toolbar==ptr; }
		BOOL operator!=( ICustToolbar* ptr )			{ return toolbar!=ptr; }
};


//-----------------------------------------------------------------------------
//-- ButtonPtr - Smart pointer

class ButtonPtr {
	public:
		ICustButton* button;
		ButtonPtr( HWND hwnd )							{ button = (hwnd==NULL? NULL : GetICustButton(hwnd)); }
		ButtonPtr( ICustButton* button )				{ this->button=button; }
		~ButtonPtr()									{ if( button!=NULL ) ReleaseICustButton( button ); }
		ICustButton* operator->()						{ return button; }
		ICustButton* operator=( ICustButton* ptr )		{ button=ptr; return button; }
		BOOL operator==( ICustButton* ptr )				{ return button==ptr; }
		BOOL operator!=( ICustButton* ptr )				{ return button!=ptr; }
};


//-----------------------------------------------------------------------------
// class StandinBrushPresetParams

StandinBrushPresetParams::StandinBrushPresetParams( Class_ID contextID, BrushPresetMgr* mgr )
{
	this->mgr = mgr;
	this->parent = NULL;
	this->contextID = contextID;
}

StandinBrushPresetParams::~StandinBrushPresetParams() {
	int count = GetNumParams();
	for( int i=0; i < count; i++ ) {
		ReleaseIfInterface( paramList[i]->val );
		delete paramList[i]; 
		paramList[i] = NULL;
	}
	paramList.SetCount(0);
}

int StandinBrushPresetParams::GetParamIndex( int paramID ) {
	int count = GetNumParams();
	for( int i=0; i<count; i++ )
		if( paramList[i]->paramID == paramID ) return i;
	return -1;
}

void StandinBrushPresetParams::AddParam( int paramID, FPValue val ) {
	int index = GetParamIndex( paramID );
	if( index < 0 || index >= paramList.Count() ) {
		index = paramList.Count();
		ParamItem* paramItem = new ParamItem;
		paramItem->paramID = paramID;
		paramList.Append( 1, &paramItem );
	}
	ParamItem* paramItem = paramList[index];
	ReleaseIfInterface(paramItem->val);
	paramItem->val = val;
	AcquireIfInterface(paramItem->val);
}

IBrushPresetParams*	StandinBrushPresetParams::ResolveParams() {
	IBrushPresetContext* context = (mgr->GetContext(contextID));
	if( context==NULL ) return NULL;

	mgr->SetIgnoreUpdates(TRUE);
	IBrushPresetParams* params = context->CreateParams();
	DbgAssert(NULL != params);
	if (params == NULL) return NULL;
	
	params->SetParent( this->parent );
	int count = GetNumParams();
	for( int i=0; i<count; i++ )
		params->SetParamValue( paramList[i]->paramID, paramList[i]->val );
	mgr->SetIgnoreUpdates(FALSE);
	return params;
}


//-----------------------------------------------------------------------------
// class StandinBrushPresetContext

StandinBrushPresetContext::StandinBrushPresetContext(
	Class_ID contextID, TCHAR* name, Class_ID pluginClassID, SClass_ID pluginSClassID )
{
	this->contextID = contextID;
	this->pluginClassID = pluginClassID;
	this->pluginSClassID = pluginSClassID;
	this->name = name;
}

StandinBrushPresetContext::~StandinBrushPresetContext()
{
	
}

//-----------------------------------------------------------------------------
// Brush Param Descriptors

// NOTE: Enum ordering must match the paramDesc ordering
enum {
	paramFalloffCurve=1,
	paramName,
	paramMinSize, paramMaxSize,
	paramMinStr, paramMaxStr,
	paramPressureSenseOn, paramPressureAffects,
	paramMirrorOn, paramMirrorAxis, paramMirrorOffset, paramMirrorGizmoSize,
	// transient params for display only
	paramStrength, paramSize, paramMirror,
};

StdBrushPresetContext::ParamDesc StdBrushPresetContext::paramDescs[] =
{
	{ (ParamType2)TYPE_INTERFACE,	IDS_PARAM_FALLOFFCURVE,			_T(""),	-1	 }, //Note: these two params have
	{ (ParamType2)TYPE_STRING,		IDS_PARAM_NAME,					_T(""),	-1	 }, //special handling for display

	{ (ParamType2)TYPE_FLOAT,		IDS_PARAM_MINSIZE,				_T(""),	-1	 },
	{ (ParamType2)TYPE_WORLD,		IDS_PARAM_MAXSIZE,				_T(""),	-1	 },
	{ (ParamType2)TYPE_FLOAT,		IDS_PARAM_MINSTR,				_T(""),	-1	 },
	{ (ParamType2)TYPE_WORLD,		IDS_PARAM_MAXSTR,				_T(""),	-1	 },

	{ (ParamType2)TYPE_BOOL,		IDS_PARAM_PRESSURESENSEON,		_T(""),	-1	 },
	{ (ParamType2)TYPE_INT,			IDS_PARAM_PRESSUREAFFEECTS,		_T(""),	-1	 },

	{ (ParamType2)TYPE_BOOL,		IDS_PARAM_MIRRORON,				_T(""),	-1	 },
	{ (ParamType2)TYPE_INT,			IDS_PARAM_MIRRORAXIS,			_T(""),	-1	 },
	{ (ParamType2)TYPE_FLOAT,		IDS_PARAM_MIRROROFFSET,			_T(""),	-1	 },
	{ (ParamType2)TYPE_FLOAT,		IDS_PARAM_MIRRORGIZMOSIZE,		_T(""),	-1	 },

	{ (ParamType2)TYPE_VOID,		IDS_PARAM_STRENGTH,				_T(""),	0	 },
	{ (ParamType2)TYPE_VOID,		IDS_PARAM_SIZE,					_T(""),	1	 },
	{ (ParamType2)TYPE_VOID,		IDS_PARAM_MIRROR,				_T(""),	2	 },

	{ (ParamType2)TYPE_VOID, 0, _T(""), 0}
};


//-----------------------------------------------------------------------------
// class StdBrushPresetParams

StdBrushPresetParams::StdBrushPresetParams() {
	int numParams = GetStdBrushPresetContext()->GetNumParams();
	toggles.SetSize( numParams );
	toggles.SetAll();

	name[0] = 0;
	minSize = maxSize = 0;
	minStr  = maxStr  = 0;
	pressureSenseOn = mirrorOn = FALSE;
	pressureAffects = 0;
	mirrorAxis = 0;
	mirrorOffset = 0;
	mirrorGizmoSize = 0;
	falloffCurve.SetLifetime( BaseInterface::noRelease );
}

StdBrushPresetParams::~StdBrushPresetParams() {
}

int StdBrushPresetParams::GetNumParams() {
	return GetStdBrushPresetContext()->GetNumParams();
}

int StdBrushPresetParams::GetParamID( int paramIndex ) {
	if( (paramIndex<0) || (paramIndex>=GetNumParams()) ) return 0;
	return paramIndex+1;
}

int StdBrushPresetParams::GetParamIndex( int paramID ) {
	if( (paramID<1) || (paramID>GetNumParams()) ) return -1;
	return paramID-1;
}

void StdBrushPresetParams::ApplyParams() {
	IPainterInterface_V5* painter = GetPainterInterface();
	if( painter!=NULL ) {
		// FIXME: hack; set the falloff graph first...
		// updating the curve doesn't refresh the UI but the other Set() calls will.
		Assign( painter->GetFalloffGraph(), &falloffCurve );

		// FIXME: currently no handling for the param toggles
		painter->SetMinSize(minSize);
		painter->SetMaxSize(maxSize);
		painter->SetMinStr(minStr);
		painter->SetMaxStr(maxStr);
		painter->SetPressureEnable(pressureSenseOn);
		painter->SetPressureAffects(pressureAffects);
		painter->SetMirrorEnable(mirrorOn);
		painter->SetMirrorAxis(mirrorAxis);
		painter->SetMirrorOffset(mirrorOffset);
		//painter->SetMirrorSize(mirrorSize);
	}
}

void StdBrushPresetParams::FetchParams() {
	IPainterInterface_V5* painter = GetPainterInterface();
	if( painter!=NULL ) {
		minSize			= painter->GetMinSize();
		maxSize			= painter->GetMaxSize();
		minStr			= painter->GetMinStr();
		maxStr			= painter->GetMaxStr();
		pressureSenseOn	= painter->GetPressureEnable();
		pressureAffects	= painter->GetPressureAffects();
		mirrorOn		= painter->GetMirrorEnable();
		mirrorAxis		= painter->GetMirrorAxis();
		mirrorOffset	= painter->GetMirrorOffset();
		//mirrorGizmoSize = painter->GetMirrorSize();
		Assign( &falloffCurve, painter->GetFalloffGraph() );
	}
}

BOOL StdBrushPresetParams::GetParamToggle( int paramID ) {
	int paramIndex = GetParamIndex( paramID );
	return toggles[ paramIndex ];
}

void StdBrushPresetParams::SetParamToggle( int paramID, BOOL onOff ) {
	int paramIndex = GetParamIndex( paramID );
	toggles.Set( paramIndex, onOff );
}

FPValue StdBrushPresetParams::GetParamValue( int paramID ) {
	FPValue val;
	val.type = GetStdBrushPresetContext()->GetParamType(paramID);

	switch( paramID ) {
	case paramFalloffCurve:		val.fpi = &falloffCurve;		break;
	case paramName:				val.s = name;					break;
	case paramMinSize:			val.f = minSize;				break;
	case paramMaxSize:			val.f = maxSize;				break;
	case paramMinStr:			val.f = minStr;					break;
	case paramMaxStr:			val.f = maxStr;					break;
	case paramPressureSenseOn:	val.i = pressureSenseOn;		break;
	case paramPressureAffects:	val.i = pressureAffects;		break;
	case paramMirrorOn:			val.i = mirrorOn;				break;
	case paramMirrorAxis:		val.i = mirrorAxis;				break;
	case paramMirrorOffset:		val.f = mirrorOffset;			break;
	case paramMirrorGizmoSize:	val.f = mirrorGizmoSize;		break;
	}

	return val;
}

void StdBrushPresetParams::SetParamValue( int paramID, FPValue val ) {
	//FIXME: restore this check ... was removed because the Name parameter accepts several types
	//if( val.type != GetStdBrushPresetContext()->GetParamType(paramID) )
	//	return;

	switch( paramID ) {
	case paramFalloffCurve:
		{
			BaseInterface* iface = val.fpi->GetInterface( CURVEIOBJECT_INTERFACE_ID );
			if( iface!=NULL ) {
				Assign( &falloffCurve, (CurvePts*)val.fpi );
				GetBrushPresetMgr()->OnPresetIconUpdated( parentID );
			}
			break;
		}
	case paramName:
		name[0] = 0;
		if( ((val.type==TYPE_TSTR_BV) || (val.type==TYPE_TSTR)) && (val.tstr!=NULL) )
			_tcscpy( name, val.tstr->data() );
		if( (val.type==TYPE_STRING) && (val.s!=NULL) )
			_tcscpy( name, val.s );
		break;
	case paramMinSize:			minSize = val.f;				break;
	case paramMaxSize:			maxSize = val.f;				break;
	case paramMinStr:			minStr = val.f;					break;
	case paramMaxStr:			maxStr = val.f;					break;
	case paramPressureSenseOn:	pressureSenseOn = val.i;		break;
	case paramPressureAffects:	pressureAffects = val.i;		break;
	case paramMirrorOn:			mirrorOn = val.i;				break;
	case paramMirrorAxis:		mirrorAxis = val.i;				break;
	case paramMirrorOffset:		mirrorOffset = val.f;			break;
	case paramMirrorGizmoSize:	mirrorGizmoSize = val.f;		break;
	}
}

FPValue StdBrushPresetParams::GetDisplayParamValue( int paramID ) {
	if( (paramID==paramFalloffCurve) || (paramID==paramName) )
		return GetParamValue( paramID );

	TCHAR buf[256], *text=buf;
	buf[0] = 0;

	switch( paramID ) {
		case paramStrength:		_stprintf( buf, "%.2f - %.2f", minStr, maxStr );	break;
		case paramSize:	{
				buf[0] = _T('\0');
				_tcscat( buf, FormatUniverseValue(minSize) );
				_tcscat( buf, _T(" - ") );
				_tcscat( buf, FormatUniverseValue(maxSize) );
				break;
			}
		case paramMirror: {
				if( !mirrorOn ) _stprintf( buf, "-" );
				else {
					static TCHAR* axisStr[] = {_T("X"), _T("Y"), _T("Z") };
					_stprintf( buf, "%s: %s", axisStr[mirrorAxis], FormatUniverseValue(mirrorOffset) );
				}
				break;
			}
	}

	FPValue val;
	val.type = TYPE_TSTR_BV;
	val.tstr = new TSTR( text );
	return val;
}

void StdBrushPresetParams::SetParent( IBrushPreset* parent ) {
	this->parentID = parent->PresetID();
}

TCHAR* StdBrushPresetParams::GetName() {
	return name;
}

void StdBrushPresetParams::SetName( TCHAR* name ) {
	if( name==NULL )	this->name[0] = 0;
	else				_tcscpy( this->name, name );
}


HIMAGELIST StdBrushPresetParams::GetIcon() {
	return icon.GetImageList();
}

void StdBrushPresetParams::UpdateIcon( float iconMin, float iconMax ) {
	CUIFrameMgr* frameMgr = GetCUIFrameMgr();
	int iconSize  = frameMgr->GetImageSize();
	if( (icon.Count()<4) || (icon.Width()!=iconSize) ) {
		icon.Init( iconSize, iconSize );
		icon.AddImages( 4 );
	}

	RGBTRIPLE fgColor, bgColor, textColor = GetStdColor( kText );
	RGBTRIPLE black = {0,0,0};
	int width = iconSize, height = iconSize;
	float xMid = ((width/2.0f)-0.5f), yMid = ((height/2.0f)-0.5f);

	float radiusMax = MAX( xMid, yMid );
	float w = Clamp( maxSize, iconMin, iconMax );
	w = (w - iconMin) / (iconMax - iconMin);
	float radius = Interp( w, 3, radiusMax );
	float distMax = radius;

	int index = 0;
	for( int i=0; i<2; i++ ) {
		for( int j=0; j<2; j++, index++ ) {
			bgColor = GetStdColor( (j==0? kBackground:kItemHilight) );
			if( i==0 )	fgColor = textColor;
			else		fgColor = Interp( 0.7f, textColor, bgColor );
			RGBTRIPLE* pImage = icon.GetImage( index );
			RGBTRIPLE* pMask  = icon.GetMask( index );

			for( int y=0; y<height; y++ ) {
				float distY = y - yMid;
				for( int x=0; x<width; x++ ) {
					*pImage = fgColor;

					float distX = x - xMid;
					float dist = sqrt( distX*distX + distY*distY );
					float u = (dist>=distMax? 1.0f : dist/distMax);
					float w = (dist> distMax? 0.0f : falloffCurve.GetValue( u ));
					w = Clamp( w, 0, 1 );
					*pImage = Interp( w, bgColor, fgColor );
					*pMask = black;
					//pMask->rgbtRed = pMask->rgbtGreen = pMask->rgbtBlue = (w*255);
					pImage++, pMask++;
				}
			}

			icon.UpdateImage( index );
		}
	}

}


//-----------------------------------------------------------------------------
// class StdBrushPresetContext

StdBrushPresetContext::StdBrushPresetContext() {
}

void StdBrushPresetContext::Init() {
	IBrushPresetMgr* mgr = GetIBrushPresetMgr();
	paramCount = displayCount = 0;

	GetString( IDS_STDBRUSHPRESETCONTEXT, contextName, sizeof(contextName) );

	ParamDesc* desc = &(paramDescs[0]);
	for( int i=0; desc->nameResID!=0; i++ ) {
		GetString( desc->nameResID, desc->nameStr, sizeof(desc->nameStr) );
		if( desc->displayIndex>=0 ) displayCount++;
		paramCount++;

		desc = &(paramDescs[i+1]);
	}

	mgr->RegisterContext( this );
}


void StdBrushPresetContext::DeleteParams( IBrushPresetParams* params ) {
	if( (params==NULL) || (params->ContextID()!=STDBRUSHPRESETCONTEXT_ID) )
		return; //invalid params
	delete ((StdBrushPresetParams*)params);
}

int StdBrushPresetContext::GetNumParams() {
	return paramCount;
}

int StdBrushPresetContext::GetParamID( int paramIndex ) {
	// ID numbers can be arbitrary but nonzero; this code uses the range [1..N]
	if( paramIndex<0 || paramIndex>GetNumParams() ) return 0;
	return (paramIndex + 1); //add one to the index to get the ID
}

int StdBrushPresetContext::GetParamIndex( int paramID ) {
	// ID numbers can be arbitrary but nonzero; this code uses the range [1..N]
	if( paramID<1 || paramID>(GetNumParams()+1) ) return -1;
	return (paramID - 1);  //subtract one from the ID to get the index
}

TCHAR* StdBrushPresetContext::GetParamName( int paramID ) {
	int index = GetParamIndex(paramID);
	if( index<0 ) return NULL;
	return paramDescs[ index ].nameStr; //subtract one from the ID to get the index
}

ParamType2 StdBrushPresetContext::GetParamType( int paramID ) {
	int index = GetParamIndex(paramID);
	if( index<0 ) return TYPE_VOID;
	return paramDescs[ index ].type; //subtract one from the ID to get the index
}

int StdBrushPresetContext::GetNumDisplayParams() {
	return displayCount;
}

int StdBrushPresetContext::GetDisplayParamID( int displayIndex ) {
	for( int i=0; i<paramCount; i++ )
		if( paramDescs[i].displayIndex == displayIndex )
			return GetParamID(i);
	return 0;
}

int StdBrushPresetContext::GetDisplayParamIndex( int paramID ) {
	int paramIndex = GetParamIndex( paramID );
	if( paramIndex<0 ) return -1;
	return paramDescs[ paramIndex ].displayIndex;
}

BOOL StdBrushPresetContext::IsDisplayParam( int paramID ) {
	int paramIndex = GetParamIndex( paramID );
	if( paramIndex<0 ) return FALSE;
	return (paramDescs[ paramIndex ].displayIndex >= 0? TRUE:FALSE);
}

BOOL StdBrushPresetContext::IsTransientParam( int paramID ) {
	int paramIndex = GetParamIndex( paramID );
	if( paramIndex<0 ) return -1;
	return (paramDescs[ paramIndex ].type == TYPE_VOID? TRUE:FALSE);
}



//-----------------------------------------------------------------------------
// class StdBrushPreset

StdBrushPreset::StdBrushPreset( int presetID, BrushPresetMgr* mgr ) {
	this->presetID = presetID;
	this->mgr = mgr;
	DbgAssert( mgr!=NULL );

	int numContexts = mgr->GetNumContexts();
	for( int i=0; i<numContexts; i++ ) {
		Class_ID contextID = mgr->GetContextID( i );
		ParamsItem paramsItem( contextID );
		paramsList.Append( 1, &paramsItem );
	}
}

StdBrushPreset::~StdBrushPreset() {
	for( int i=0; i<paramsList.Count(); i++ ) RemoveParams(i);
}

void StdBrushPreset::Apply() {
	BrushPresetMgr* mgr = GetBrushPresetMgr();
	for( int index=0; index<paramsList.Count(); index++ ) {
		ParamsItem& paramsItem = paramsList[index];
		if( mgr->IsContextActive( paramsItem.contextID ) ) {
			if( (paramsItem.isFinal && paramsItem.params!=NULL) || ResolveParams(index) )
				// is not a standin, or was standin but resolved
				paramsItem.params->ApplyParams();
		}
	}
}

void StdBrushPreset::Fetch() {
	BrushPresetMgr* mgr = GetBrushPresetMgr();
	for( int index=0; index<paramsList.Count(); index++ ) {
		ParamsItem& paramsItem = paramsList[index];
		if( mgr->IsContextActive( paramsItem.contextID ) ) {
			if( (paramsItem.isFinal && paramsItem.params!=NULL) || ResolveParams(index) )
				// is not a standin, or was standin but resolved
				paramsItem.params->FetchParams();
		}
	}
}

IBrushPresetParams* StdBrushPreset::GetParams( Class_ID contextID ) {
	int index = GetContextIndex(contextID);
	if( index>=0 && index<paramsList.Count() ) {
		ParamsItem& paramsItem = paramsList[index];
		if( (paramsItem.isFinal && paramsItem.params!=NULL) || ResolveParams(index) )
			// is not a standin, or was standin but resolved
			return paramsItem.params;
	}
	return NULL;
}

Class_ID StdBrushPreset::GetContextID( int paramsIndex ) {
	if( (paramsIndex<0) || (paramsIndex>=paramsList.Count()) ) return Class_ID(0,0);
	return paramsList[paramsIndex].contextID;
}

int StdBrushPreset::GetContextIndex( Class_ID contextID ) {
	for( int i=0; i<paramsList.Count(); i++ )
		if( paramsList[i].contextID==contextID )
			return i;
	return -1;
}

void StdBrushPreset::Copy( StdBrushPreset* src ) {
	int numContexts = src->GetNumContexts();
	for( int i=0; i<numContexts; i++ ) {
		Class_ID contextID = src->GetContextID(i);
		// Don't force a resolve on the src params
		IBrushPresetParams* srcParams = src->GetParams( i );
		if( srcParams==NULL ) continue;

		// Don't force a resolve on the dest params; possibly create proxy
		IBrushPresetParams* destParams = GetParams( GetContextIndex(contextID) );
		StandinBrushPresetParams* destStandin = NULL;
		if( destParams==NULL ) {
			destStandin = CreateStandinParams(contextID);
			AddParams( destStandin );
		}

		int numParams = srcParams->GetNumParams();
		for( int j=0; j<numParams; j++ ) {
			int paramID = srcParams->GetParamID( j );
			FPValue paramValue = srcParams->GetParamValue(paramID);

			if( destParams!=NULL ) {
				if( destParams->GetParamIndex(paramID) < 0 ) continue;
				destParams->SetParamValue( paramID, paramValue );
			}
			else destStandin->AddParam( paramID, paramValue );
		}
	}
}

StandinBrushPresetParams* StdBrushPreset::CreateStandinParams( Class_ID contextID ) {
	StandinBrushPresetParams* standin = new StandinBrushPresetParams( contextID, mgr );
	AddParams( standin );
	return standin;
}

IBrushPresetParams* StdBrushPreset::GetParams( int index ) {
	if( index>=0 && index<paramsList.Count() )
		return paramsList[index].params;
	return NULL;
}

int StdBrushPreset::AddParams( IBrushPresetParams* params ) {
	if( params==NULL ) return -1;
	int index = GetContextIndex( params->ContextID() );
	if( index>=0 && index<paramsList.Count() ) {
		if( paramsList[index].params!=params ) RemoveParams(index);
	}
	else {
		index = paramsList.Count();
		ParamsItem paramsItem( params->ContextID() );
		paramsList.Append( 1, &paramsItem );
	}
	ParamsItem& paramsItem = paramsList[index];
	paramsItem.params = params;
	paramsItem.isStandin = FALSE;
	paramsItem.isFinal = FALSE;
	if( params!=NULL ) params->SetParent(this);
	return index;
}

int StdBrushPreset::AddParams( StandinBrushPresetParams* standin ) {
	// Use method for regular params, but set isStandin flag
	int index = AddParams( (IBrushPresetParams*)standin );
	if( index>=0 && index<paramsList.Count() )
		paramsList[index].isStandin = TRUE;
	return index;
}

void StdBrushPreset::RemoveParams( int index ) {
	if( index >= 0 && index < paramsList.Count() ) {
		ParamsItem& paramsItem = paramsList[index];
		if( paramsItem.params!=NULL ) {
			if( paramsItem.isStandin ) delete (StandinBrushPresetParams*)(paramsItem.params);
			else {
				IBrushPresetContext* context = mgr->GetContext( paramsItem.contextID );
				if( context!=NULL ) context->DeleteParams(paramsItem.params);
			}
			paramsItem.params = NULL;
		}
		paramsItem.isStandin = FALSE;
		paramsItem.isFinal = FALSE;
	}
}

BOOL StdBrushPreset::ResolveParams( int index ) {
	if( index>=0 && index<paramsList.Count() ) {
		ParamsItem& paramsItem = paramsList[index];

		// Handle two special cases for unresolved params...
		if( !paramsItem.isFinal ) {
			// Case 1: params is null, assign a standin
			if( paramsItem.params==NULL  ) {
				paramsItem.isStandin = TRUE;
				paramsItem.params = CreateStandinParams( paramsItem.contextID );
			}
			// Case 2: params is not null or a standin, just mark it as resolved
			else if( !paramsItem.isStandin ) paramsItem.isFinal = TRUE;
		}

		// Don't resolve if we already resolved before (isFinal)
		if( paramsItem.isFinal ) return TRUE;

		// Attempt to resolve the standin
		StandinBrushPresetParams* standin = (StandinBrushPresetParams*)(paramsItem.params);
		IBrushPresetParams* params = standin->ResolveParams();
		paramsItem.isFinal = TRUE;
		if( params!=NULL && params!=paramsItem.params )  {
			paramsItem.params = params;
			paramsItem.isStandin = FALSE;
			delete standin;
		}

		return (paramsItem.params==NULL?  FALSE : TRUE);
	}
	return FALSE;
}

StdBrushPreset::ParamsItem::ParamsItem( Class_ID contextID )
{ this->contextID = contextID, this->isStandin = FALSE, this->params = NULL, this->isFinal = FALSE; }


//-----------------------------------------------------------------------------
// class BrushPresetMgr

LRESULT CALLBACK BrushPresetMgr::ToolbarWndProc(
		HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_COMMAND: {
			int id = LOWORD(wParam);
			if( (id>=ID_TOOL_PRESET_MIN) && (id<=ID_TOOL_PRESET_MAX) )
				theBrushPresetMgr.OnApplyPresetBtn( id );
			else if( id==ID_TOOL_ADDPRESET )
				theBrushPresetMgr.OnAddPresetBtn();
			else if( id==ID_TOOL_PRESETMGR )
				theBrushPresetMgr.OnPresetMgrBtn();
			break;
		}

		default: {
			WNDPROC wndproc = theBrushPresetMgr.defaultToolbarWndProc;
			if( wndproc==NULL )	return FALSE;
			return CallWindowProc( wndproc, hwnd, msg, wParam, lParam );
		}
	}
	return TRUE;
}

BrushPresetMgr::BrushPresetMgr()
  :	IBrushPresetMgr
	(	//Interface descriptor for IBrushPresetMgr
#ifndef NO_BRUSH_PRESETS
		IBRUSHPRESETMGR_INTERFACE_ID, _T("BrushPresetMgr"), 0, NULL, FP_CORE,
#else //NO_BRUSH_PRESETS
		IBRUSHPRESETMGR_INTERFACE_ID, _T("BrushPresetMgr"), 0, NULL, FP_CORE + FP_TEST_INTERFACE,
#endif //NO_BRUSH_PRESETS
		fnIdIsActive, _T("IsActive"), 0, TYPE_BOOL, 0, 0,
		fnIdOpenPresetMgr, _T("OpenPresetMgr"), 0, TYPE_VOID, 0, 0,
		fnIdAddPreset, _T("AddPreset"), 0, TYPE_VOID, 0, 0,
		fnIdLoadPresetFile, _T("LoadPresetFile"), 0, TYPE_VOID, 0, 1,
			_T("file"), NULL, TYPE_FILENAME,
		fnIdSavePresetFile, _T("SavePresetFile"), 0, TYPE_VOID, 0, 1,
			_T("file"), NULL, TYPE_FILENAME,
		end
	)
{
	initialized = FALSE;

	activePresetID = 0;
	ignoreUpdates = FALSE;
	iconMin = 5, iconMax = 20;
	iconMinSpinner = iconMaxSpinner = NULL;
	icon1 = icon2 = icon3 = NULL;
	hToolWindow = hToolbar = hDialog = NULL;
	dialogPosX = dialogPosY = dialogWidth = dialogHeight = -1;
	defaultToolbarWndProc = NULL;
#ifndef NO_BRUSH_PRESETS
	RegisterNotification( OnSystemStartup,		this, NOTIFY_SYSTEM_STARTUP );
	RegisterNotification( OnSystemShutdown,		this, NOTIFY_SYSTEM_SHUTDOWN );
	RegisterNotification( OnColorChange,		this, NOTIFY_COLOR_CHANGE );
	RegisterNotification( OnUnitsChange,		this, NOTIFY_UNITS_CHANGE );
	RegisterNotification( OnToolbarsPreLoad,	this, NOTIFY_TOOLBARS_PRE_LOAD );
	RegisterNotification( OnToolbarsPostLoad,	this, NOTIFY_TOOLBARS_POST_LOAD );
#endif //NO_BRUSH_PRESETS
}

BrushPresetMgr::~BrushPresetMgr() {
	if( icon1!=NULL ) { delete icon1; icon1 = NULL; }
	if( icon2!=NULL ) { delete icon2; icon2 = NULL; }
	if( icon3!=NULL ) { delete icon3; icon3 = NULL; }

	for (int i = 0; i < presets.Count(); i++)
	{
		delete presets[i];
		presets[i] = NULL;
	}

	for( int i = GetNumContexts()-1; i >= 0; i-- )
	{
		RemoveContext(i);
	}
}

#define NOTIFY_SYSTEM_SHUTDOWN			0x00000049	// Max is about to exit,  (system shutdown starting)
#define NOTIFY_SYSTEM_STARTUP			0x00000050	// Max just went live

void BrushPresetMgr::Init( IPainterInterface_V5* painterInterface ) {
	//FIXME: is setting a global static like this a hack?  Or is it valid a singleton pattern?
	thePainterInterface = painterInterface;
}

void BrushPresetMgr::Reset() {
	SetActivePreset(0);

	while( presets.Count()>0 )
		DeletePreset( presets[0] );

	if( hToolWindow!=NULL ) {
		DestroyWindow( hToolWindow );
		GetCUIFrameMgr()->RecalcLayout(TRUE);
	}
	hToolWindow = NULL;
	hToolbar = NULL;
}

void BrushPresetMgr::RegisterContext( IBrushPresetContext* context ) {
#ifdef NO_BRUSH_PRESETS
	return;
#endif //NO_BRUSH_PRESETS
	if( context == NULL ) return; //invalid input
	AddContext( context );

	int numContexts = contextList.Count();
	activeContexts.SetSize( numContexts, TRUE );
	activeContexts.Set( (numContexts-1), FALSE );
}

void BrushPresetMgr::UnRegisterContext( IBrushPresetContext* context ) {
	if( context == NULL ) return; //invalid input
	int index = GetContextIndex( context->ContextID() );
	if( index<0 ) return; //context is null, or not registered

	int numContexts = contextList.Count();
	for( int i=index; i<(numContexts-1); i++ )
		activeContexts.Set( i, activeContexts[i+1] );
	activeContexts.SetSize( (numContexts-1), TRUE );
}

IBrushPresetContext* BrushPresetMgr::GetContext( Class_ID contextID ) {
	int index = GetContextIndex(contextID);
	DbgAssert(index >= 0);
	DbgAssert(index < contextList.Count());
	if( index >= 0 && index < contextList.Count() ) 
	{
		ContextItem& contextItem = contextList[index];
		if( (contextItem.isFinal) || (!contextItem.isStandin) || ResolveStandinContext(index) )
		{
			// is not a standin, or was standin but resolved
			DbgAssert(NULL != contextItem.context);
			return contextItem.context;
		}
	}
	return NULL;
}

Class_ID BrushPresetMgr::GetContextID( int contextIndex ) {
	if( (contextIndex<0) || (contextIndex>=contextList.Count()) ) return Class_ID(0,0);
	return contextList[contextIndex].contextID;
}

int BrushPresetMgr::GetContextIndex( Class_ID contextID ) {
	for( int i=0; i<contextList.Count(); i++ )
		if( contextList[i].contextID==contextID )
			return i;
	return -1;
}

int BrushPresetMgr::BeginContext( Class_ID contextID ) {
	int index = GetContextIndex(contextID);
	if( index<0 ) return 0;
	if( activeContexts[index]==TRUE ) return 0;

	//NOTE: Begin the standard paint context, before any other context launches.
	//FIXME: It's possible that a Painter object does not exist yet.
	// Creating the painter calls the Init() method; must create the painter here.
	Class_ID stdID = GetStdBrushPresetContext()->ContextID();
	if( (contextID!=stdID) && !IsContextActive(stdID) )		BeginContext( stdID );
	if( (contextID==stdID) && !initialized )				CreatePainterInterface();

	//if( activeContexts.IsEmpty() ) { //starting up the first context?
	//	ReadConfig(); //load the config data
	//	if( !IsToolbarVisible() ) ShowToolbar();
	//}

	activeContexts.Set( GetContextIndex(contextID), TRUE );

	SetFocusContextID( contextID );
	if( IsDialogVisible() ) UpdateDialog();
	if( contextID==stdID  ) UpdateToolbar( updateToolbar_Toggles );
	return 1;
}

int BrushPresetMgr::EndContext( Class_ID contextID ) {
	int index = GetContextIndex(contextID);
	if( index<0 ) return 0;
	if( activeContexts[index]==FALSE ) return 0;

	activeContexts.Set(index, FALSE);

	Class_ID stdID = GetStdBrushPresetContext()->ContextID();
	int activeCount = activeContexts.NumberSet();
	//if( activeCount==0 ) { //closing down the last context?
	//	WriteConfig(); //save the config data
	//	HideToolbar();
	//}
	if( activeCount==1 ) {
		//NOTE: End the standard paint context, after all other contexts shut down.
		SetActivePreset(0);
		EndContext( stdID );
		HideDialog();
		UpdateToolbar( updateToolbar_Toggles );
	}

	return 1;
}

BOOL BrushPresetMgr::IsContextActive( Class_ID contextID ) {
	int index = GetContextIndex(contextID);
	if( index<0 ) return FALSE;
	return activeContexts[index];
}

StdBrushPreset* BrushPresetMgr::GetStdPreset( int presetID ) {
	int index = GetPresetIndex(presetID);
	return (index<0? NULL : presets[ index ]);	
}

StdBrushPreset* BrushPresetMgr::CreateStdPreset( int presetID ) {
	StdBrushPreset* preset = new StdBrushPreset( presetID, this );
	presets.Append(1,&preset);
	return preset;
}

StandinBrushPresetContext* BrushPresetMgr::CreateStandinContext(
	Class_ID contextID, TCHAR* contextName, Class_ID pluginClassID, SClass_ID pluginSClassID )
{
	StandinBrushPresetContext* standin =
		new StandinBrushPresetContext( contextID, contextName, pluginClassID, pluginSClassID );
	AddContext( standin );
	return standin;
}

IBrushPreset* BrushPresetMgr::CreatePreset() {
	// Create the brush preset object
	int presetID = CreatePresetID();
	StdBrushPreset* preset = CreateStdPreset( presetID ); 
	presetIndices.SetCount(0); //reset the lookup; will be updated when needed

	// Initialize all the params for the preset
	preset->Fetch();

	return preset;
}

void BrushPresetMgr::DeletePreset( IBrushPreset* preset ) {
	if( preset==NULL ) return;
	int presetID = preset->PresetID();
	int presetIndex = GetPresetIndex( presetID );
	if( presetIndex>=0 ) {
		delete presets[ presetIndex ];
		presets.Delete( presetIndex, 1 );
		presetIndices.SetCount(0); //reset the lookup; will be updated when needed
	}
}

IBrushPreset* BrushPresetMgr::GetPreset( int presetID ) {
	int index = GetPresetIndex(presetID);
	return (index<0? NULL : presets[ index ]);
}

int BrushPresetMgr::GetPresetID( int presetIndex ) {
	if( (presetIndex<0) || (presetIndex>=presets.Count()) ) return 0;
	if( presets[presetIndex] == NULL ) return 0;
	return presets[presetIndex]->PresetID();
}

int BrushPresetMgr::GetPresetIndex( int presetID ) {
	if( presetID<1 || presetID>MAX_NUM_PRESETS ) return -1;
	if( presetIndices.Count() != GetNumPresets() )
		UpdatePresetIndices();
	return presetIndices[ presetID ];
}

int BrushPresetMgr::GetActivePreset() {
	return activePresetID;
}

void BrushPresetMgr::SetActivePreset( int presetID ) {
	activePresetID = presetID;
}

//FIXME: Call this ActivatePreset() ?
void BrushPresetMgr::ApplyPreset( int presetID ) {
	SetIgnoreUpdates(TRUE);

	int toolID = GetPresetToolID( presetID );
	ToolbarPtr toolbar( hToolbar );
	ButtonPtr btn( toolbar->GetICustButton( toolID ) );

	// Update all the params for the active preset, before deactivating
	if( activePresetID!=0 )
		GetPreset(activePresetID)->Fetch();

	if( presetID!=GetActivePreset() ) {
		IBrushPreset* preset = GetPreset( presetID );
		SetActivePreset( presetID );
		if( preset!=NULL )
			preset->Apply();
	}
	else SetActivePreset( 0 );

	SetIgnoreUpdates(FALSE);
}

// Called by a context when its brush parameters have changed
void BrushPresetMgr::OnContextUpdated( Class_ID contextID ) {
	if( GetIgnoreUpdates() ) return;

	IBrushPreset* preset = GetPreset( GetActivePreset() );
	if( (preset!=NULL) && IsContextActive(contextID) ) {
		IBrushPresetParams* params = preset->GetParams( contextID );
		params->FetchParams();

		StdBrushPresetParams* stdParams = GetStdBrushPresetParams(preset);
		if( contextID == stdParams->ContextID() )
			UpdateToolbarIcon( preset->PresetID() );
		UpdateDialogItem( preset->PresetID() );
	}
}

// Called by a preset when its icon image is changed
void BrushPresetMgr::OnPresetIconUpdated( int presetID ) {
	if( GetIgnoreUpdates() ) return;

	UpdateToolbarIcon( presetID );
}

int BrushPresetMgr::ReadConfig(TCHAR *cfg) {
	int retval = 0;

	// Find config file, if necessary
	TCHAR cfgBuf[MAX_PATH];
	if( (cfg==NULL) || (cfg[0]==0) || !DoesFileExist(cfg)) {
		cfg = cfgBuf;
		if( !GetReadFileName(cfg) )
			 cfg = NULL;
	}

	if( cfg!=NULL ) {
		retval = ReadPresets( cfg );
		if( retval && IsDialogVisible() ) UpdateDialog();
	}

	if( cfg==NULL || retval!=0 ) {
		//If cfg is NULL, this will create a new default toolbar
		retval = ReadToolbar( cfg );
		if( retval ) UpdateToolbar();
	}

	return retval;
}

int BrushPresetMgr::WriteConfig(TCHAR *cfg) {
	// Find config file, if necessary
	TCHAR cfgBuf[MAX_PATH];
	if( (cfg==NULL) || (cfg[0]==0) ) {
		if( !GetWriteFileName(cfgBuf) ) return FALSE;
		cfg = cfgBuf;
	}

	WritePresets( cfg );
	WriteToolbar( cfg );

	return TRUE;
}

Class_ID BrushPresetMgr::GetFocusContextID() {
	while( focusContexts.Count()>0 ) {
		int i = focusContexts.Count()-1;
		return focusContexts[i];
		//int contextID = focusContexts[i];
		//if( !IsContextActive(contextID) )
		//	focusContexts.Delete( i, 1 );
		//else return contextID;
	}
	return Class_ID(0,0);
}

void BrushPresetMgr::SetFocusContextID( Class_ID contextID ) {
	for( int i=0; i<focusContexts.Count(); ) {
		if( focusContexts[i]==contextID )
			focusContexts.Delete(i,1);
		else i++;
	}
	focusContexts.Append( 1, &contextID );
}

//int BrushPresetMgr::GetFocusPresetID() {
//	if( GetPreset(focusPresetID)==NULL ) {
//		focusPresetIndex = (GetNumPresets()-1);
//		focusPresetID = GetPresetID( focusPresetIndex );			
//	}
//	return focusPresetID;
//}
//
//void BrushPresetMgr::SetFocusPresetID( int presetID ) {
//	focusPresetID = presetID;
//	focusPresetIndex = GetPresetIndex( presetID );
//}

IBrushPresetContext* BrushPresetMgr::GetContext( int index ) {
	if( index>=0 && index<contextList.Count() )
		return contextList[index].context;
	return NULL;
}

int BrushPresetMgr::AddContext( IBrushPresetContext* context ) {
	if( context==NULL ) return -1;
	int index = GetContextIndex( context->ContextID() );
	if( index>=0 && index<contextList.Count() ) {
		RemoveContext(index);
	}
	else {
		index = contextList.Count();
		ContextItem contextItem( context->ContextID() );
		contextList.Append( 1, &contextItem );
	}

	int numContexts = contextList.Count();
	activeContexts.SetSize( numContexts, TRUE );
	activeContexts.Set( index, FALSE );

	ContextItem& contextItem = contextList[index];
	contextItem.context = context;
	contextItem.isStandin = FALSE;
	contextItem.isFinal = FALSE;
	return index;
}

int BrushPresetMgr::AddContext( StandinBrushPresetContext* standin ) {
	// Use method for regular params, but set isStandin flag
	int index = AddContext( (IBrushPresetContext*)standin );
	if( index>=0 && index<contextList.Count() )
		contextList[index].isStandin = TRUE;
	return index;
}

void BrushPresetMgr::RemoveContext( int index ) {
	DbgAssert( index >= 0 );
	DbgAssert( index < contextList.Count() );
	if( index>=0 && index<contextList.Count() ) {
		ContextItem& contextItem = contextList[index];
		if( contextItem.context!=NULL ) {
			if( contextItem.isStandin ) {
				delete (StandinBrushPresetContext*)(contextItem.context);
			}
		}
		contextItem.context = NULL;
		contextItem.isStandin = FALSE;
		contextItem.isFinal = FALSE;
	}
}

BOOL BrushPresetMgr::IsStandinContext( int index ) {
	if( index>=0 && index<contextList.Count() )
		return contextList[index].isStandin;
	return FALSE;
}

BOOL BrushPresetMgr::ResolveStandinContext( int index ) {
	if( index>=0 && index<contextList.Count() ) {
		ContextItem& contextItem = contextList[index];

		// Special cases for unresolved params: if not null or a standin, just mark as resolved 
		if( !contextItem.isFinal && !contextItem.isStandin ) contextItem.isFinal = TRUE;

		// Don't resolve if we already resolved before (isFinal)
		if( contextItem.isFinal ) return TRUE;

		// Attempt to resolve the standin by loading the standin's plugin
		StandinBrushPresetContext* standin = (StandinBrushPresetContext*)(contextItem.context);
		LoadClass( standin->PluginClassID(), standin->PluginSuperClassID() );
		contextItem.isFinal = TRUE;

		// Loaded plugin should call RegisterContext(), to replace standin with the real value.
		// Success if the context is no longer a standin
		return (IsStandinContext(index)?  FALSE : TRUE);
	}
	return FALSE;
}


void BrushPresetMgr::ShowToolbar() {
	if( hToolWindow==NULL ) {
		ReadToolbar(NULL);
		UpdateToolbar();
	}

	CUIFramePtr toolWindow( hToolWindow );
	if( toolWindow!=NULL ) {
		toolWindow->Hide(FALSE);
		UpdateToolbar( updateToolbar_Toggles );
	}
}

void BrushPresetMgr::HideToolbar() {
	CUIFramePtr toolWindow( hToolWindow );
	if( toolWindow!=NULL ) toolWindow->Hide(TRUE);
}

BOOL BrushPresetMgr::IsToolbarVisible() {
	CUIFramePtr toolWindow( hToolWindow );
	if( toolWindow==NULL ) return FALSE;
	return (toolWindow->IsHidden()? FALSE:TRUE);
}

void BrushPresetMgr::ShowDialog() {
	if( !IsDialogVisible() )
		OpenDialog();
}

void BrushPresetMgr::HideDialog() {
	if( IsDialogVisible() ) {
		::EndDialog( hDialog, 0 );
		DestroyWindow( hDialog );
		listView.Free();
		listViewIcons.Free();
		ReleaseISpinner( iconMinSpinner );
		ReleaseISpinner( iconMaxSpinner );
		iconMinSpinner = iconMaxSpinner = NULL;
		hDialog = NULL;

		UpdateToolbar( updateToolbar_Toggles );
	}
}

BOOL BrushPresetMgr::IsDialogVisible() {
	return (hDialog==NULL? FALSE:TRUE);
}

void BrushPresetMgr::SetDialogSize( int width, int height ) {
	::SetWindowPos( hDialog, 0, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
	dialogWidth=width;
	dialogHeight=height;
}

void BrushPresetMgr::SetDialogPos( int x, int y, HWND relativeTo ) {
	::SetWindowPos( hDialog, x, y, relativeTo );

	RECT rect;
	::GetWindowRect( hDialog, &rect );
	dialogPosX=rect.left;
	dialogPosY=rect.top;
}

int BrushPresetMgr::ReadPresets(TCHAR *cfg) {
	TCHAR cfgBuf[1024];
	TCHAR key[1024];
	TCHAR name[1024];
	TCHAR contextSection[1024], presetSection[1024], contextName[1024];
	TCHAR* mgrSection = _T("BrushPresets_Config");

	if( (cfg==NULL) || (cfg[0]==0) || !DoesFileExist(cfg)) {
		cfg = cfgBuf;
		if( !GetReadFileName(cfg) )
			return FALSE;
	}

	Reset();

	TCHAR* prevLocale = _tsetlocale(LC_ALL, NULL); //Get and copy the locale string
	if( prevLocale!=NULL ) prevLocale = _tcsdup(prevLocale);
	_tsetlocale(LC_ALL, _T("C")); //Set the new locale

	int numPresets = 0, numContexts = 0;
	if( GetPrivateProfileString( mgrSection, PRESETCFG_NUMPRESETS, _T(""), key, sizeof(key), cfg ) )
		numPresets = StringToInt(key);

	if( GetPrivateProfileString( mgrSection, PRESETCFG_NUMCONTEXTS, _T(""), key, sizeof(key), cfg ) )
		numContexts = StringToInt(key);

	if( GetPrivateProfileString( mgrSection, PRESETCFG_ICONMIN, _T(""), key, sizeof(key), cfg ) )
		iconMin = StringToFloat( key );
	if( GetPrivateProfileString( mgrSection, PRESETCFG_ICONMAX, _T(""), key, sizeof(key), cfg ) )
		iconMax = StringToFloat( key );

	for( int i=0; i<numContexts; i++ ) {
		_stprintf( contextSection, _T("Context%d"), i+1 );

		GetPrivateProfileString( contextSection, PRESETCFG_CONTEXTID, _T(""), key, sizeof(key), cfg );
		Class_ID contextID = StringToClassID( key );

		// bad contextID or already have info about context? don't create standin
		if( (contextID==Class_ID(0,0)) || (GetContextIndex(contextID)>=0) ) continue;

		GetPrivateProfileString( contextSection, PRESETCFG_CONTEXTNAME, _T(""), key, sizeof(key), cfg );
		ReadToken( key, contextName, _T('\0') );

		GetPrivateProfileString( contextSection, PRESETCFG_PLUGINCLASSID, _T(""), key, sizeof(key), cfg );
		Class_ID pluginClassID = StringToClassID( key );

		GetPrivateProfileString( contextSection, PRESETCFG_PLUGINSUPERCLASSID, _T(""), key, sizeof(key), cfg );
		SClass_ID pluginSClassID = StringToDWORD( key );

		StandinBrushPresetContext* standin = CreateStandinContext( contextID, contextName, pluginClassID, pluginSClassID );
	}

	// For each preset, find one section in the config file
	for( int i=0; i<numPresets; i++ ) {
		//_stprintf( name, _T("Preset%d"), i+1 );
		//GetPrivateProfileString( mgrSection, name, _T(""), key, sizeof(key), cfg );
		//_tcscpy( presetSection, key );
		_stprintf( presetSection, _T("Preset%d"), i+1 );

		// Find entries for the presetID, presetName, presetButtonPos, and number of parameter sets (contexts)
		GetPrivateProfileString( presetSection, PRESETCFG_PRESETID, _T(""), key, sizeof(key), cfg );
		int presetID = StringToInt( key );

		StdBrushPreset* preset = CreateStdPreset( presetID );

		GetPrivateProfileString( presetSection, PRESETCFG_NUMCONTEXTS, _T(""), key, sizeof(key), cfg );
		int numContexts = StringToInt( key );

		for( int j=0; j<numContexts; j++ ) {
			TCHAR paramName[1024], paramValueStr[1024];
			Class_ID contextID;
			int numParams;

			_stprintf( name, _T("Context%d"), j+1 );
			if( !GetPrivateProfileString( presetSection, name, _T(""), key, sizeof(key), cfg ) )
				continue; //preset has no params defined for the context

			// Context entry indicates the contextID, contextName and number or params...
			ReadCfgContextString( key, &contextID, contextName, &numParams );

			Class_ID pluginClassID(0,0);
			SClass_ID pluginSClassID(0);
			StandinBrushPresetParams* standin = preset->CreateStandinParams( contextID );
			preset->AddParams( standin );

			for( int k=0; k<numParams; k++ ) {
				int paramID;
				//BOOL paramToggle;
				FPValue paramVal;

				_stprintf( name, _T("Context%d_Param%d"), j+1, k+1 );

				// Param entry indicates the paramID, paramName, and the param value as a quoted string
				GetPrivateProfileString( presetSection, name, _T(""), key, sizeof(key), cfg );
				ReadCfgParamString( key, &paramID, paramName, paramValueStr );

				StringToFPValue( paramValueStr, &paramVal );
				standin->AddParam( paramID, paramVal );
				//standin->SetParamValue( paramID, paramVal );
				//params->SetParamToggle( paramID, paramToggle );
				ReleaseIfInterface( paramVal );
			}
		}
	}

	_tsetlocale(LC_ALL, prevLocale); //Restore previous locale
	if( prevLocale!=NULL ) free(prevLocale);

	return TRUE;
}

int BrushPresetMgr::WritePresets(TCHAR *cfg) {
	TCHAR cfgBuf[1024];
	TCHAR key[1024];
	TCHAR name[1024];
	TCHAR contextSection[1024], presetSection[1024], paramValueStr[1024];
	TCHAR* mgrSection = _T("BrushPresets_Config");

	if( (cfg==NULL) || (cfg[0]==0) ) {
		cfg = cfgBuf;
		if( !GetWriteFileName(cfg) )
			 return FALSE;
	}

	TCHAR* prevLocale = _tsetlocale(LC_ALL, NULL); //Get and copy the locale string
	if( prevLocale!=NULL ) prevLocale = _tcsdup(prevLocale);
	_tsetlocale(LC_ALL, _T("C")); //Set the new locale

	int numPresets = GetNumPresets();
	int numContexts = GetNumContexts();

	_stprintf( key, _T("%d"), numPresets );
	WritePrivateProfileString( mgrSection, PRESETCFG_NUMPRESETS, key, cfg );

	_stprintf( key, _T("%d"), numContexts );
	WritePrivateProfileString( mgrSection, PRESETCFG_NUMCONTEXTS, key, cfg );

	_stprintf( key, "%f", iconMin );
	WritePrivateProfileString( mgrSection, PRESETCFG_ICONMIN, key, cfg );
	_stprintf( key, "%f", iconMax );
	WritePrivateProfileString( mgrSection, PRESETCFG_ICONMAX, key, cfg );

	for( int i=0; i<numContexts; i++ ) {
		IBrushPresetContext* context = GetContext( i ); // Context or standin
		if( context==NULL ) continue;
		_stprintf( contextSection, _T("Context%d"), i+1 );

		_stprintf( key, "\"%s\"", context->GetContextName() );
		WritePrivateProfileString( contextSection, PRESETCFG_CONTEXTNAME, key, cfg );

		Class_ID contextID = context->ContextID();
		ClassIDToString( contextID, key );
		WritePrivateProfileString( contextSection, PRESETCFG_CONTEXTID, key, cfg );

		Class_ID pluginClassID = context->PluginClassID();
		ClassIDToString( pluginClassID, key );
		WritePrivateProfileString( contextSection, PRESETCFG_PLUGINCLASSID, key, cfg );

		SClass_ID pluginSClassID = context->PluginSuperClassID();
		_stprintf( key, "%u", pluginSClassID );
		WritePrivateProfileString( contextSection, PRESETCFG_PLUGINSUPERCLASSID, key, cfg );
	}

	// For each preset, create one section in the config file
	for( int i=0; i<numPresets; i++ ) {
		StdBrushPreset* preset = GetStdPreset( GetPresetID(i) );
		if( preset==NULL ) continue;
		//_stprintf( name, _T("P%04d"), i );
		//_stprintf( key, _T("Preset%d"), preset->PresetID() );
		//WritePrivateProfileString( mgrSection, name, key, cfg );

		_stprintf( presetSection, _T("Preset%d"), i+1 );
		//_tcscpy( presetSection, key );

		// Create entries for the presetID, presetName, presetButtonPos, and number of parameter sets (contexts)
		_stprintf( key, "%d", preset->PresetID() );
		WritePrivateProfileString( presetSection, PRESETCFG_PRESETID, key, cfg );

		int numContexts = preset->GetNumContexts();
		_stprintf( key, "%d", numContexts );
		WritePrivateProfileString( presetSection, PRESETCFG_NUMCONTEXTS, key, cfg );

		DbgAssert( numContexts>1 );

		// Create entries for each set of params held by the preset (each context)
		for( int j=0; j<numContexts; j++ ) {
			// TO DO: handling for context and preset names with Quotes and Slashes in the name and preset value
			Class_ID contextID = preset->GetContextID( j );
			IBrushPresetContext* context = GetContext( GetContextIndex(contextID) );
			IBrushPresetParams* params = preset->GetParams( preset->GetContextIndex(contextID) );
			if( params==NULL ) continue;

			TCHAR* contextName = context->GetContextName();
			int numParams = params->GetNumParams();

			int numEntries = 0;
			for( int k=0; k<numParams; k++ )
				if( !context->IsTransientParam( params->GetParamID( k ) ) )
					numEntries++; //one entry for each non-transient parameter

			_stprintf( name, _T("Context%d"), j+1 );
			_stprintf( key, _T("(0x%lX,0x%lX)|\"%s\"|%d"), contextID.PartA(), contextID.PartB(), contextName, numEntries );

			// Context entry indicates the contextID, contextName and number or params...
			WritePrivateProfileString( presetSection, name, key, cfg );
			// ...Then one entry decribing each of params in this context
			int entryIndex = 0;
			for( int k=0; k<numParams; k++ ) {
				int paramID = params->GetParamID( k );
				if( context->IsTransientParam( paramID ) )
					continue; //transient param; do not write param or increment the entryIndex
				TCHAR* paramName = context->GetParamName( paramID );
				//BOOL paramToggle = params->GetParamToggle( paramID );
				//TCHAR* paramToggleStr = (paramToggle? PRESETCFG_ENABLED : PRESETCFG_DISABLED );
				FPValue val = params->GetParamValue( paramID );
				FPValueToString( &val, paramValueStr );
				_stprintf( name, _T("Context%d_Param%d"), j+1, entryIndex+1 );
				// Param entry indicates the paramID, paramName, and the param value as a quoted string
				_stprintf( key, _T("%d|\"%s\"|\"%s\""), paramID, paramName, paramValueStr );
				//_stprintf( key, _T("%d|\"%s\"|%s|\"%s\""), paramID, paramName, paramToggleStr, paramValueStr );
				WritePrivateProfileString( presetSection, name, key, cfg );
				entryIndex++;
			}
		}
	}

	_tsetlocale(LC_ALL, prevLocale); //Restore previous locale
	if( prevLocale!=NULL ) free(prevLocale);

	return TRUE;
}

int BrushPresetMgr::ReadToolbar(TCHAR *cfg) {
	//TO DO: get notification when the toolbar is closed

	Interface* ip = GetCOREInterface();
	CUIFrameMgr* frameMgr = GetCUIFrameMgr();


	//***************************************************************************
	// Create icons
	int btnWidth  = frameMgr->GetButtonWidth();
	int btnHeight = frameMgr->GetButtonHeight();
	int iconSize  = frameMgr->GetImageSize();

	if( icon1==NULL ) icon1 = new MaxBmpFileIcon( _T("LayerToolbar"), 2 );
	if( icon2==NULL ) icon2 = new MaxBmpFileIcon( _T("Radiosity"), 2 );
	if( icon3==NULL ) icon3 = new MaxBmpFileIcon( _T("Maxscript"), 1 );


	//***************************************************************************
	// Create the foating Tool Window for the Toolbar
	HWND hWnd_cui = frameMgr->GetAppHWnd();
	HWND hParent = hWnd_cui;
	TCHAR* toolWindowName = GetString(IDS_BRUSHPRESETS_TOOLBAR);	

	if( hToolWindow==NULL )
		hToolWindow = CreateCUIFrameWindow( hParent, toolWindowName, 0, 0, 0, 0 );
	CUIFramePtr toolWindow( hToolWindow );

	if(hToolWindow == NULL)	{
		// the rest of this code assumes this toolWindow exists
		return FALSE;
	}
	
	toolWindow->SetPosType( (~CUI_DONT_SAVE) & toolWindow->GetPosType() );

	BOOL cfgLoaded;
	if( DoesFileExist(cfg) )
		 cfgLoaded = toolWindow->ReadConfig( cfg, 1 );
	else cfgLoaded = FALSE;

	if( (!cfgLoaded) && (hToolbar==NULL) ) { //setup a default toolbar instead
		SIZE size = {40,60};
		RECT rect = {150,110,190,170};

		toolWindow->SetContentType(CUI_TOOLBAR);
		toolWindow->SetPosType( CUI_ALL_DOCK | CUI_FLOATABLE | CUI_SM_HANDLES );
		hToolbar = CreateWindow( CUSTTOOLBARWINDOWCLASS,
			toolWindowName, WS_CHILD | WS_VISIBLE, 0, 0, size.cx, size.cy, hToolWindow, NULL, hInstance, NULL );

		ToolbarPtr toolbar( hToolbar );
		toolbar->LinkToCUIFrame( hToolWindow, NULL ); //Put the toolbar into the frame window
		MoveWindow(hToolWindow, 0, 0, size.cx, size.cy, FALSE); //Resize the window
		::SetWindowPos( hToolWindow, rect.left, rect.top, hParent ); //Adjust the window position
		ShowWindow( hToolWindow, SW_HIDE );
	}
	toolWindow->SetPosType( CUI_DONT_SAVE | toolWindow->GetPosType() );
	toolWindow->SetSystemWindow( TRUE );

	//***************************************************************************
	// Get the toolbar & set its WndProc
	hToolbar = toolWindow->GetContentHandle();
	ToolbarPtr toolbar( hToolbar );

	WNDPROC proc = DLGetWindowProc( hToolbar);
	if( proc != ToolbarWndProc ) {
		defaultToolbarWndProc = proc;
		DLSetWindowLongPtr( hToolbar, ToolbarWndProc);
	}

	//***************************************************************************
	// Add buttons to the toolbar

	ButtonPtr mgrButton( toolbar->GetICustButton( ID_TOOL_PRESETMGR ) );
	if( mgrButton==NULL ) {
		toolbar->AddTool( ToolButtonItem(CTB_CHECKBUTTON, icon3, iconSize, iconSize, btnWidth, btnHeight, ID_TOOL_PRESETMGR) );
		mgrButton = toolbar->GetICustButton( ID_TOOL_PRESETMGR );
		mgrButton->SetIcon( icon3, iconSize, iconSize );
		mgrButton->SetInIcon( icon3, iconSize, iconSize );
	}
	mgrButton->SetTooltip( TRUE, GetString( IDS_OPENBRUSHPRESETMGR ) );

	ButtonPtr createButton( toolbar->GetICustButton( ID_TOOL_ADDPRESET ) );
	if( createButton==NULL ) {
		toolbar->AddTool( ToolButtonItem(CTB_PUSHBUTTON, icon1, iconSize, iconSize, btnWidth, btnHeight, ID_TOOL_ADDPRESET) );
		createButton = toolbar->GetICustButton( ID_TOOL_ADDPRESET );
		createButton->SetIcon( icon1, iconSize, iconSize );
		createButton->SetInIcon( icon1, iconSize, iconSize );
	}
	createButton->SetTooltip( TRUE, GetString( IDS_ADDBRUSHPRESET ) );

	for( int i=0; i<toolbar->GetNumItems(); i++ ) {
		int toolID = toolbar->GetItemID(i);
		if( toolID>=ID_TOOL_PRESET_MIN && toolID<=ID_TOOL_PRESET_MAX ) {
			ButtonPtr btn( toolbar->GetICustButton(toolID) );
			btn->Execute( I_EXEC_BUTTON_DAD_ENABLE, FALSE ); // Disable button drag & drop within the toolbar
		}
	}

	ResizeFloatingTB(hToolWindow);
	GetCUIFrameMgr()->RecalcLayout(TRUE);

	//***************************************************************************
	// Setup the button icons
	UpdateToolbar();

	return TRUE;
}

int BrushPresetMgr::WriteToolbar( TCHAR* cfg ) {
	TCHAR cfgBuf[MAX_PATH];

	if( (cfg==NULL) || (cfg[0]==0) ) {
		cfg = cfgBuf;
		if( !GetWriteFileName(cfg) )
			return FALSE;
	}

	ICUIFrame *toolWindow = GetICUIFrame( hToolWindow );
	if( toolWindow!=NULL ) {
		hToolbar = toolWindow->GetContentHandle();
		// (obsolete) Always save toolbar as hidden.  After loading, context objects may optionally display the toolbar using ShowToolbar()
		//toolWindow->Hide(TRUE);
		toolWindow->SetPosType( (~CUI_DONT_SAVE) & toolWindow->GetPosType() );
		toolWindow->WriteConfig( cfg );
		toolWindow->SetPosType(   CUI_DONT_SAVE  | toolWindow->GetPosType() );

		ReleaseICUIFrame( toolWindow );
	}

	return TRUE;
}

void BrushPresetMgr::UpdateToolbar( int updateChannels ) {
	//if( !IsToolbarVisible() ) return;

	int count = GetNumPresets();
	if( updateChannels & updateToolbar_Icons ) {
		for( int i=0; i<count; i++ )
			UpdateToolbarIcon( GetPresetID(i) );
	}

	if( updateChannels & updateToolbar_Toggles )
		UpdateToolbarToggles();

	if( updateChannels & updateToolbar_Size )
		UpdateToolbarSize();
}

void BrushPresetMgr::UpdateToolbarSize() {
	//if( !IsToolbarVisible() ) return;
	CUIFrameMgr* frameMgr = GetCUIFrameMgr();
	CUIFramePtr  toolWindow( hToolWindow );

	if( toolWindow!=NULL ) {
		if( toolWindow->IsFloating() )
			ResizeFloatingTB(hToolWindow);
		else frameMgr->RecalcLayout(TRUE);
	}
}

void BrushPresetMgr::UpdateToolbarIcon( int presetID ) {
	//if( !IsToolbarVisible() ) return;
	int toolID = GetPresetToolID( presetID );
	ToolbarPtr toolbar( hToolbar );
	if( toolbar==NULL )	return;

	ButtonPtr btn( toolbar->GetICustButton( toolID ) );
	if( btn==NULL )		return;

	StdBrushPreset* preset = GetStdPreset( presetID );
	StdBrushPresetParams* params = GetStdBrushPresetParams( preset );
	int iconSize  = GetCUIFrameMgr()->GetImageSize();

	//FIXME: add blank icons, otherwise the system thinks it's a text button
	btn->SetIcon( icon2, iconSize, iconSize );
	btn->SetInIcon( icon2, iconSize, iconSize );

	// Create and draw the icon image,
	// and assign the image to the button
	params->UpdateIcon( iconMin, iconMax );
	btn->SetImage( params->GetIcon(), 0, 1, 2, 3, iconSize, iconSize );

	btn->SetTooltip( TRUE, params->GetName() );
	btn->SetCaptionText( _T("") );
}

void BrushPresetMgr::UpdateToolbarToggles() {
	//if( !IsToolbarVisible() ) return;
	ToolbarPtr toolbar( hToolbar );
	if( toolbar==NULL )	return;

	BOOL enabledState = (activeContexts.IsEmpty()? FALSE:TRUE);

	ButtonPtr presetMgrBtn( toolbar->GetICustButton( ID_TOOL_PRESETMGR ) );
	BOOL checkedState = IsDialogVisible();
	if( checkedState != presetMgrBtn->IsChecked() ) presetMgrBtn->SetCheck( checkedState );
	if( enabledState != presetMgrBtn->IsEnabled() ) presetMgrBtn->Enable( enabledState );

	ButtonPtr addPresetBtn( toolbar->GetICustButton( ID_TOOL_ADDPRESET ) );
	if( enabledState != addPresetBtn->IsEnabled() ) addPresetBtn->Enable( enabledState );

	for( int i=0; i<GetNumPresets(); i++ ) {
		int presetID = GetPresetID(i);
		int toolID = GetPresetToolID( presetID );
		ButtonPtr btn( toolbar->GetICustButton( toolID ) );

		if( btn!=NULL ) {
			BOOL checkedState = (presetID==GetActivePreset()? TRUE:FALSE);
			if(  checkedState != btn->IsChecked() ) btn->SetCheck(checkedState);
			if(  enabledState != btn->IsEnabled() ) btn->Enable(enabledState);
		}
	}
}

void BrushPresetMgr::AddPresetButton( int presetID ) {
	IBrushPreset* preset = GetPreset( presetID );

	ToolbarPtr toolbar( hToolbar );
	if( toolbar==NULL ) return;

	CUIFrameMgr* frameMgr = GetCUIFrameMgr();
	int btnWidth  = frameMgr->GetButtonWidth();
	int btnHeight = frameMgr->GetButtonHeight();
	int iconSize  = frameMgr->GetImageSize();
	int toolID = GetPresetToolID( presetID );
	int helpID = idh_dialog_createbrushpreset;

	ToolButtonItem item(CTB_CHECKBUTTON, NULL, iconSize, iconSize, btnWidth, btnHeight, toolID, helpID );
	toolbar->AddTool( item );

	ButtonPtr btn( toolbar->GetICustButton( toolID ) );
	btn->Execute( I_EXEC_BUTTON_DAD_ENABLE, FALSE ); // Disable button drag & drop within the toolbar

	UpdateToolbarIcon( presetID );
	UpdateToolbar( updateToolbar_Size );
}

void BrushPresetMgr::RemovePresetButton( int presetID ) {
	ToolbarPtr toolbar( hToolbar );
	if( toolbar==NULL ) return;

	int toolID = GetPresetToolID( presetID );
	int toolIndex = toolbar->FindItem( toolID );
	toolbar->DeleteTools( toolIndex, 1 );

	UpdateToolbar( updateToolbar_Size );
}

HWND BrushPresetMgr::GetPresetButton( int presetID ) {
	if( hToolbar==NULL ) return NULL;
	int toolID = GetPresetToolID( presetID );

	ToolbarPtr toolbar( hToolbar );
	ButtonPtr btn( toolbar->GetICustButton( toolID ) );

	HWND hWnd = (btn==NULL? NULL : btn->GetHwnd());
	return hWnd;
}

int BrushPresetMgr::GetPresetToolID( int presetID ) {
	return (presetID + ID_TOOL_PRESET_MIN) - 1;
}

int BrushPresetMgr::GetToolPresetID( int toolID ) {
	return (toolID - ID_TOOL_PRESET_MIN) + 1;
}

BOOL BrushPresetMgr::PromptFileName( TCHAR* buf, int type ) {
	TCHAR drive[MAX_PATH], dir[MAX_PATH], fname[MAX_PATH], ext[MAX_PATH];
	TCHAR path[MAX_PATH], file[MAX_PATH];

	if( promptFileName.isNull() ) {
		GetWriteFileName(file);
		promptFileName = file;
	}
	_tsplitpath( promptFileName.data(), drive, dir, fname, ext);
	_stprintf( path, "%s%s", drive, dir );
	_stprintf( file, "%s%s", fname, ext );
    
	FilterList filterList;
    filterList.Append( GetString(IDS_PRESET_FILES_BPR) );
    filterList.Append( GetString(IDS_BPR_EXTENTION) );

	OPENFILENAME ofn;
	memset( &ofn, 0, sizeof(ofn) );
	ofn.lStructSize = sizeof(OPENFILENAME);  // No OFN_ENABLEHOOK
	ofn.hwndOwner = GetCOREInterface()->GetMAXHWnd();
	ofn.hInstance = hInstance;
	ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.FlagsEx = OFN_EX_NOPLACESBAR;
	ofn.lpstrFilter = filterList;
	ofn.nFilterIndex = 1;
	ofn.lpstrDefExt = GetString(IDS_BPR_EXTENTION);
	ofn.lpstrInitialDir = path;
    ofn.lpstrFile = file;
	ofn.nMaxFile = MAX_PATH;
	if( type==promptFileRead )	ofn.lpstrTitle = GetString(IDS_LOAD_PRESET_FILE);
	if( type==promptFileWrite )	ofn.lpstrTitle = GetString(IDS_SAVE_PRESET_FILE);

	BOOL retval = FALSE;
	if( type==promptFileRead )	retval = GetOpenFileName(&ofn);
	if( type==promptFileWrite )	retval = GetSaveFileName(&ofn);

	if( retval ) {
		promptFileName = ofn.lpstrFile;
		_tcscpy( buf, ofn.lpstrFile );
	}
	return retval;
}

// The config file to load from;
// Either MaxStartUI.bpr or DefaultUI.bpr,
// or returns FALSE if neither exist
BOOL BrushPresetMgr::GetReadFileName( TCHAR* buf ) {
	
	TSTR filename(STARTFILENAME);
	TSTR fullPath; 
	if( !GetCUIFrameMgr()->ResolveReadPath(filename, fullPath)) {
		filename = DEFAULTFILENAME;
		if( !GetCUIFrameMgr()->ResolveReadPath(filename, fullPath) )
			return FALSE;
	}
	_tcscpy( buf, fullPath.data());
	return TRUE;
}

// The config file to save into; MaxStartUI.bpr
BOOL BrushPresetMgr::GetWriteFileName( TCHAR* buf ) {
	TSTR filename(STARTFILENAME);
	TSTR fullPath; 

	// this really shouldn't fail
	bool result = GetCUIFrameMgr()->ResolveWritePath(filename, fullPath);
	DbgAssert(result);
	_tcscpy( buf, fullPath.data());
	return TRUE;
}

void BrushPresetMgr::OpenDialog() {
	if( IsDialogVisible() ) return;

	HWND hParent = GetCOREInterface()->GetMAXHWnd();
	hDialog = CreateDialogParam( hInstance, MAKEINTRESOURCE(IDD_PRESETMGR), hParent, DialogWndProc, (LPARAM)this );
	
	iconMinSpinner = GetISpinner( GetDlgItem(hDialog, IDC_PRESET_ICONMIN_SPIN) );
	HWND hIconMinEdit = GetDlgItem( hDialog, IDC_PRESET_ICONMIN_EDIT );
	iconMinSpinner->SetLimits( 0.0f, 9999999.0f );
	iconMinSpinner->SetAutoScale( TRUE );
	iconMinSpinner->SetValue( iconMin, FALSE );
	iconMinSpinner->LinkToEdit( hIconMinEdit, EDITTYPE_UNIVERSE );

	iconMaxSpinner = GetISpinner( GetDlgItem(hDialog, IDC_PRESET_ICONMAX_SPIN) );
	HWND hIconMaxEdit = GetDlgItem( hDialog, IDC_PRESET_ICONMAX_EDIT );
	iconMaxSpinner->SetLimits( 0.0f, 9999999.0f );
	iconMaxSpinner->SetAutoScale( TRUE );
	iconMaxSpinner->SetValue( iconMax, FALSE );
	iconMaxSpinner->LinkToEdit( hIconMaxEdit, EDITTYPE_UNIVERSE );

	UpdateDialog();
}

void SetCell( ListViewCell* listCell, FPValue* value, WinImageList* imageList ) {
	BOOL isColor = ((value->type==TYPE_COLOR) || (value->type==TYPE_COLOR_BV));
	BOOL isCurve = (GetCurvePtsInterface(value)!=NULL);
	BOOL isWorld = (value->type==TYPE_WORLD);
	int imageWidth = 0;
	int imageHeight = 0;
	int imageIndex = 0;
	RGBTRIPLE *image = NULL;
	RGBTRIPLE *mask = NULL;

	listCell->SetEditable( FALSE );

	if( isColor || isCurve ) {
		imageIndex = imageList->AddImages(1);
		image = imageList->GetImage( imageIndex );
		mask = imageList->GetMask( imageIndex );
		imageWidth = imageList->Width(), imageHeight = imageList->Height();
		listCell->SetImage( imageIndex );
	}

	if( isColor ) {
		RGBTRIPLE white = {255,255,255}, black = {0,0,0};
		RGBTRIPLE borderColor = GetStdColor( kText );
		RGBTRIPLE outerColor = GetStdColor( kWindow );
		RGBTRIPLE centerColor = ColorToRGBTRIPLE( *(value->clr) );

		int left=1, right=imageWidth-2, top=1, bottom=imageHeight-2;
		RGBTRIPLE *pImage = image, *pMask = mask;
		for( int y=0; y<imageHeight; y++ ) {
			for( int x=0; x<imageWidth; x++ ) {
				if( x<left || x>right || y<top || y>bottom )
					*pMask = black, *pImage = outerColor;
					 //*pMask = white, *pImage = black;
				else if( x==left || x==right || y==top || y==bottom )
					 *pMask = black, *pImage = borderColor;
				else *pMask = black, *pImage = centerColor;
				pImage++, pMask++;
			}
		}
		imageList->UpdateImage(imageIndex);
	}
	else if( isCurve ) {
		CurvePts* curve = GetCurvePtsInterface( value );

		RGBTRIPLE fgColor = GetStdColor( kText );
		RGBTRIPLE bgColor = GetStdColor( kWindow );
		RGBTRIPLE *pImage = image, *pMask = mask;
		float xMid = ((imageWidth/2.0f)-0.5f), yMid = ((imageHeight/2.0f)-0.5f);
		float distMax = (xMid>yMid? xMid:yMid);

		for( int y=0; y<imageHeight; y++ ) {
			float distY = y - yMid;
			for( int x=0; x<imageWidth; x++ ) {
				float distX = x - xMid;
				float dist = sqrt( distX*distX + distY*distY );
				float u = (dist>=distMax? 1.0f : dist/distMax);
				float w = (dist> distMax? 0.0f : curve->GetValue( u ));
				w = Clamp( w, 0, 1 );
				
				*pImage = Interp( w, bgColor, fgColor );
				//*pImage = fgColor;
				//pMask->rgbtRed = pMask->rgbtGreen = pMask->rgbtBlue = ((1.0f-w)*255);
				pImage++, pMask++;
			}
		}
		imageList->UpdateImage(imageIndex);
	}
	else if( isWorld ) {
		listCell->SetText( FormatUniverseValue(value->f) );
	}
	else {
		TCHAR valueStr[256];
		FPValueToUIString( value, valueStr );
		listCell->SetText( valueStr );
	}
}

void BrushPresetMgr::UpdateDialog( int updateChannels ) {
	if( !IsDialogVisible() ) return;

	IBrushPresetContext* stdContext = GetStdBrushPresetContext();
	IBrushPresetContext* focusContext = GetContext( GetFocusContextID() );
	DbgAssert(NULL != stdContext);
	DbgAssert(NULL != focusContext);
	int numStdParams = stdContext->GetNumDisplayParams();
	int numFocusParams = focusContext->GetNumDisplayParams();
	int numContexts = GetNumContexts();
	int numPresets = GetNumPresets();

	if( updateChannels & updateDialog_Spinners ) {
		iconMinSpinner->SetValue( iconMin, FALSE );
		iconMaxSpinner->SetValue( iconMax, FALSE );
	}

	if( updateChannels & updateDialog_ComboBox ) {
		HWND hComboBox = GetDlgItem( hDialog, IDC_PRESET_CONTEXT );
		int comboIndex=0, comboSel = 0;
		SendMessage( hComboBox, CB_RESETCONTENT, 0, 0 );
		for( int i=0; i<numContexts; i++ ) {
			IBrushPresetContext* context = GetContext( i );
			if( context->ContextID() == stdContext->ContextID() )
				continue; //don't display the standard context in the dropdown
			comboIndex = SendMessage( hComboBox, CB_INSERTSTRING, -1, (LPARAM)(context->GetContextName()) );
			SendMessage( hComboBox, CB_SETITEMDATA, comboIndex, (LPARAM)context );
			if( context->ContextID() == focusContext->ContextID() )
				comboSel = comboIndex;
		}
		SendMessage( hComboBox, CB_SETCURSEL, comboSel, 0 ); 
	} //end updateComboBox


	if( updateChannels & updateDialog_ListView ) {
		int iconWidth = GetSystemMetrics(SM_CXSMICON), iconHeight = GetSystemMetrics(SM_CYSMICON);
		listViewIcons.Init( iconWidth, iconHeight );

		HWND hListView = GetDlgItem( hDialog, IDC_PRESET_LIST );
		listView.Init( hListView );
		listView.SetNotifyProc( ListViewNotifyProc, this );

		listView.SetNumColumns( numStdParams + numFocusParams + 1 ); //Special handling for the first column
		listView.SetNumRows( numPresets );
		listView.SetImageList( listViewIcons.GetImageList() );

		int offset = 0;
		for( int i=0; i<2; i++ ) {
			IBrushPresetContext* context = (i==0? stdContext:focusContext);
			int numParams = context->GetNumDisplayParams();

			if( i==0 ) { //Special handling for the first column
				listView.GetColumn(0)->SetName( GetString(IDS_PARAM_NAME) );
				offset = 1;
			}

			for( int displayIndex=0; displayIndex<numParams; displayIndex++ ) {
				int paramID = context->GetDisplayParamID( displayIndex );
				TCHAR* name = context->GetParamName( paramID );
				ListViewColumn* column = listView.GetColumn( displayIndex + offset );
				column->SetName(name);
			}
			offset+=numParams;
		}

		for( int presetIndex=0; presetIndex<numPresets; presetIndex++ )
			UpdateDialogItem( GetPresetID(presetIndex) );

		listView.Update();
	} //end updateListView

	if( updateChannels & (updateDialog_ListView | updateDialog_ListViewSel) ) {
		int activePreset = GetActivePreset();
			listView.SetSelRow( GetPresetIndex(activePreset) );

		BOOL enable = activePreset>0? TRUE:FALSE;
		ButtonPtr duplicateBtn( GetDlgItem( hDialog, IDC_PRESET_DUPLICATE ) );
		ButtonPtr deleteBtn( GetDlgItem( hDialog, IDC_PRESET_DELETE ) );
		duplicateBtn->Enable( enable );
		deleteBtn->Enable( enable );
	}
}

void BrushPresetMgr::UpdateDialogItem( int presetID ) {
	if( !IsDialogVisible() ) return;

	IBrushPresetContext* stdContext = GetStdBrushPresetContext();
	IBrushPresetContext* focusContext = GetContext( GetFocusContextID() );

	int offset = 0;
	int row = GetPresetIndex( presetID );
	IBrushPreset* preset = GetPreset( presetID );

	for( int i=0; i<2; i++ ) {
		IBrushPresetContext* context = (i==0? stdContext:focusContext);
		Class_ID contextID = context->ContextID();
		int numParams = context->GetNumDisplayParams();

		IBrushPresetParams* params = preset->GetParams( contextID );
		ListViewRow* listRow = listView.GetRow( row );

		if( i==0 ) {
			//Special handling for the stdContext, first column
			ListViewCell* listCell = listRow->GetCell( 0 );
			FPValue value1 = params->GetParamValue( 1 );
			SetCell( listCell, &value1, &listViewIcons );
			FPValue value2 = params->GetParamValue( 2 );
			SetCell( listCell, &value2, &listViewIcons );
			listCell->SetEditable( TRUE );
			offset = 1;
		}

		for( int displayIndex=0; displayIndex<numParams; displayIndex++ ) {
			int col = displayIndex + offset;
			int paramID = context->GetDisplayParamID( displayIndex );
			ListViewCell* listCell = listRow->GetCell( col );

			FPValue value = params->GetDisplayParamValue( paramID );
			SetCell( listCell, &value, &listViewIcons );
		}

		offset += numParams;
	}

	listView.UpdateRow( row );
}

TCHAR invalidChars[] = {
	'\"', '\t',
	0
};

bool BrushPresetMgr::ListViewNotifyProc( ListView::NotifyInfo& info, void* param ) {
	BrushPresetMgr* parent = (BrushPresetMgr*)param;

	if( info.message==ListView::notifySelCell ) {
		int presetID = parent->GetPresetID(info.row);
		//parent->SetFocusPresetID( presetID );
		if( presetID != parent->GetActivePreset() ) {
			parent->ApplyPreset( presetID );
			parent->UpdateToolbar( updateToolbar_Toggles );
			parent->UpdateDialog( updateDialog_ListViewSel );
		}
	}
	if( info.message==ListView::notifyEditing ) {
		for( int i=0; invalidChars[i]!=0; i++ )
			if( info.editChar==invalidChars[i] )
				return false;
	}
	if( info.message==ListView::notifyEndEdit ) {
		TSTR& text = info.editText;
		int count, index = 0;
		while ( text[index] == ' ' || text[index] == '\t' )
			index++;
		text.remove(0, index);	// trim leading spaces
		count = text.length();
		while (count && (text[count-1] == ' ' || text[count-1] == '\t'))
			count--;
		text.remove(count);		// trim trailing spaces

		int presetID = parent->GetPresetID(info.row);
		IBrushPreset* preset = parent->GetPreset( presetID );
		StdBrushPresetParams* params = GetStdBrushPresetParams( preset );
		params->SetName( text.data() );
		parent->UpdateToolbarIcon( presetID );
	}
	return true;
}

IBrushPreset* BrushPresetMgr::CreatePreset( IBrushPreset* src ) {
	if( GetNumPresets()>=MAX_NUM_PRESETS ) {
		MessageBox( NULL, GetString(IDS_TOOMANYPRESETS), GetString(IDS_ERROR), MB_ICONERROR );
		return NULL;
	}

	TSTR name = CreatePresetName( src );
	if( name.isNull() ) return NULL;

	IBrushPreset* preset = CreatePreset();
	if( preset==NULL ) return NULL;

	if( src!=NULL ) {
		StdBrushPreset* stdSrc = GetStdPreset( src->PresetID() );
		StdBrushPreset* stdDest = GetStdPreset( preset->PresetID() );
		stdDest->Copy( stdSrc );
	}

	StdBrushPresetParams* params = GetStdBrushPresetParams( preset );
	params->SetName( name.data() );

	AddPresetButton( preset->PresetID() );
	ApplyPreset( preset->PresetID() );

	UpdateToolbar( updateToolbar_Toggles );
	UpdateDialog( updateDialog_ListView );
	return preset;
}

int IntegerComparator( const void* left, const void* right ) {
	int ileft = *(int*)left, iright = *(int*)right;
	if( ileft<iright ) return -1;
	if( ileft>iright ) return 1;
	return 0;
}
int BrushPresetMgr::CreatePresetID() {
	int count = GetNumPresets();
	if( count==0 ) return 1;
	if( count==1 ) return (GetPresetID(0)==1?  2:1);

	BitArray bits( MAX_NUM_PRESETS );
	bits.ClearAll();
   int i;
	for( i=0; i<count; i++ )
		bits.Set( GetPresetID(i), TRUE );

	for( i=0; i<MAX_NUM_PRESETS; i++ )
		if( !bits[i+1] ) break;

	return i+1;
}

// Parses an expresstion in the form "xxx copy yy"
// Where xxx is any text (the name of the preset) and yy is decimal digits (the copy number)
void ParseExpression_CopiedName( TCHAR* prefix, int* copyNum, const TCHAR* src ) {
	//NOTE: should maybe use CAtlRegExp Class

	TCHAR buf[256], copyDigits[256], copyLabel[256];
	TCHAR* str = buf;
	int start=0, end, cur, length;

	_tcscpy( buf, src );

	// Strip leading & trailing whitespace
	start = 0, cur = end = length = static_cast<int>(_tcslen(str));
	while( start<end && _istspace(str[start]) )	start++, length--;
	while( end>start && _istspace(str[end-1]) )	end--, length--;

	cur = end;
	str[end] = _T('\0');

	// Extract trailing digits
	while( cur>start && _istdigit(str[cur-1]) )	cur--;
	_tcscpy(copyDigits, str+cur);
	str[cur] = _T('\0');

	// Strip whitespace before the trailing digits
	while( cur>start && _istspace(str[cur-1]) )	cur--;
	str[cur] = _T('\0');

	// Look for the trailing word "copy"
	_tcscpy( copyLabel, GetString(IDS_COPY) );
	cur -= static_cast<int>(_tcslen(copyLabel));

	if( (cur>=start) && _tcsicmp(str+cur,copyLabel)==0) {
		// Found trailing word "copy", parse the digits
		if( _stscanf( copyDigits, "%i", copyNum ) != 1 ) *copyNum = 1;
		// Strip remaining whitespace
		end = cur;
		while( end>start && _istspace(str[end-1]) ) end--;
	}
	else *copyNum = -1; // No trailing word "copy", ignore the digits

	// Prefix is the [start,end] substring
	if( end>start ) memcpy( prefix, src+start, (end-start) * sizeof(TCHAR) );
	prefix[end-start] = _T('\0');
}

TSTR BrushPresetMgr::CreatePresetName( IBrushPreset* src ) {
	TCHAR buf[256];
	int numPresets = GetNumPresets();

	// Make a copied name if applicable
	if( src!=NULL ) {
		TCHAR srcPrefix[256], curPrefix[256], *name;
		int srcCopyNum, curCopyNum;

		name = GetStdBrushPresetParams( src )->GetName();
		ParseExpression_CopiedName( srcPrefix, &srcCopyNum, name );

		int maxCopyNum = srcCopyNum;
		for( int i=0; i<numPresets; i++ ) {
			name = GetStdBrushPresetParams( presets[i] )->GetName();
			ParseExpression_CopiedName( curPrefix, &curCopyNum, name );
			if( _tcsicmp( curPrefix, srcPrefix )==0 ) maxCopyNum = MAX(maxCopyNum, curCopyNum);
		}

		if( maxCopyNum>=0 )	_stprintf( buf, "%s %s %i", srcPrefix, GetString(IDS_COPY), maxCopyNum + 1 );
		else				_stprintf( buf, "%s %s", srcPrefix, GetString(IDS_COPY) );
	}
	else // Make a generic, non-copied name
		_stprintf( buf, "%s%i", GetString(IDS_PRESET), numPresets+1 );

	// Throw up the Create Brush Preset dialog
	CreatePresetDialog createPresetDialog;
	createPresetDialog.SetPresetName( buf );
	int result = createPresetDialog.DoDialog();
	if( result<=0 ) return TSTR(); //bail out if the user hits cancel
	_tcscpy( buf, createPresetDialog.GetPresetName() );

	return buf;
}

void BrushPresetMgr::UpdatePresetIndices() {
	presetIndices.SetCount(MAX_NUM_PRESETS+1);

	int lookupCount = presetIndices.Count();
	for( int i=0; i<lookupCount; i++ )
		presetIndices[i] = -1;

	int presetCount = presets.Count();
	for( int i=0; i<presetCount; i++ )
		presetIndices[ presets[i]->PresetID() ] = i;
}


// Event handlers
void BrushPresetMgr::OnPresetMgrBtn() {
	BOOL enabledState = (activeContexts.IsEmpty()? FALSE:TRUE);
	
	if( enabledState ) { // Prevent access to the dialog when manager is disabled
		if( !IsDialogVisible() )
			ShowDialog();
		else HideDialog();
	}
}

void BrushPresetMgr::OnLoadPresetsBtn() {
	TCHAR file[MAX_PATH];
	if( !PromptFileName( file, promptFileRead ) ) return;
	ReadConfig( file );
}

void BrushPresetMgr::OnSavePresetsBtn() {
	TCHAR file[MAX_PATH];
	if( !PromptFileName( file, promptFileWrite ) ) return;
	WriteConfig( file );
}

void BrushPresetMgr::OnAddPresetBtn() {
	IBrushPreset* preset = CreatePreset( NULL );
}

void BrushPresetMgr::OnDuplicatePresetBtn() {
	StdBrushPreset* src = GetStdPreset( GetActivePreset() ); //GetStdPreset( GetFocusPresetID() );
	if( src==NULL ) return;

	IBrushPreset* preset = CreatePreset( src );
}

void BrushPresetMgr::OnRemovePresetBtn() {
	// This message should only come from the dialog,
	// because the preset for deletion can only be selected in the dialog
	DbgAssert( IsDialogVisible() );

	int presetID = GetActivePreset(); //GetFocusPresetID();
	if( presetID == 0 ) return; //Nothing to do
	if( presetID == GetActivePreset() ) SetActivePreset(0);

	int presetIndex = GetPresetIndex( presetID );
	//SetFocusPresetID( GetPresetID(presetIndex-1) );

	DeletePreset( GetPreset(presetID) );

	RemovePresetButton(presetID);
	UpdateToolbar( updateToolbar_Toggles );
	UpdateDialog( updateDialog_ListView );
}


void BrushPresetMgr::OnApplyPresetBtn( int toolID ) {
	int presetID = GetToolPresetID( toolID );
	ApplyPreset( presetID );
	UpdateToolbar( updateToolbar_Toggles );
	UpdateDialog( updateDialog_ListViewSel );
}

void BrushPresetMgr::OnSpinnerChange() {
	if( iconMinSpinner==NULL || iconMaxSpinner==NULL )
		return; //Can happen during dialog shutdown
	float newIconMin = iconMinSpinner->GetFVal();
	float newIconMax = iconMaxSpinner->GetFVal();
	if( newIconMin>=newIconMax ) {
		iconMinSpinner->SetValue(iconMin,FALSE);
		iconMaxSpinner->SetValue(iconMax,FALSE);
	}
	else {
		iconMin=newIconMin, iconMax=newIconMax;
		UpdateToolbar();
	}
}

void BrushPresetMgr::OnInitDialog(HWND hWnd) {
	hDialog = hWnd;

	SendMessage( hDialog, WM_SETICON, ICON_SMALL, GetClassLongPtr( GetCOREInterface()->GetMAXHWnd(), GCLP_HICONSM ) );

	GetCOREInterface()->RegisterDlgWnd( hDialog );


	if( dialogPosX>=0 && dialogPosY>=0 )
		SetDialogPos( dialogPosX, dialogPosY );
	else 
		CenterWindow( hDialog, GetCOREInterface()->GetMAXHWnd() );

	if( dialogWidth>=0 && dialogHeight>=0 )
		SetDialogSize( dialogWidth, dialogHeight );

	RECT rect;
	::GetWindowRect( hDialog, &rect );
	dialogPosX = rect.left;
	dialogPosY = rect.top;
	dialogWidth = rect.right - rect.left;
	dialogHeight = rect.bottom - rect.top;
}

void BrushPresetMgr::OnDestroyDialog(HWND hWnd) {
	if( hDialog==hWnd ) {
		GetCOREInterface()->UnRegisterDlgWnd( hDialog );
		hDialog=NULL;
	}
}

void BrushPresetMgr::OnDialogResized() {
	RECT rect;
	::GetWindowRect( hDialog, &rect );
	dialogWidth = rect.right - rect.left;
	dialogHeight = rect.bottom - rect.top;
}

void BrushPresetMgr::OnDialogMoved() {
	RECT rect;
	::GetWindowRect( hDialog, &rect );
	dialogPosX = rect.left;
	dialogPosY = rect.top;
}

void BrushPresetMgr::OnSystemStartup(void *param, NotifyInfo *info) {
	BrushPresetMgr* parent = (BrushPresetMgr*)param;
	StdBrushPresetContext* context = GetStdBrushPresetContext();
	context->Init();

	if( parent->ReadConfig() )
		parent->initialized = TRUE;
	//FIXME: hack... we're actually registered to a viewport change,
	//which happens earlier than system startup.  Must remove the notification
	UnRegisterNotification( OnSystemStartup, param, NOTIFY_VIEWPORT_CHANGE );
}

void BrushPresetMgr::OnSystemShutdown(void *param, NotifyInfo *info) {
	BrushPresetMgr* parent = (BrushPresetMgr*)param;
	if( parent->initialized ) //Don't write unless we successfully read a config
		parent->WriteConfig();
}

void BrushPresetMgr::OnColorChange(void *param, NotifyInfo *info) {
	BrushPresetMgr* parent = (BrushPresetMgr*)param;
	int count = parent->GetNumPresets();
	for( int i=0; i<count; i++ )
		parent->UpdateToolbarIcon( parent->GetPresetID(i) );
}

void BrushPresetMgr::OnUnitsChange(void *param, NotifyInfo *info) {
	BrushPresetMgr* parent = (BrushPresetMgr*)param;
	if( parent->IsDialogVisible() )
		parent->UpdateDialog();
}

void BrushPresetMgr::OnToolbarsPreLoad(void *param, NotifyInfo *info) {
	BrushPresetMgr* parent = (BrushPresetMgr*)param;
	if( parent->hToolWindow==NULL ) return;

	// All system toolbars are destroyed and rebuilt during toolbar load.
	// Temporarily mark the preset toolbar as non-system, since it has a separate loading scheme
	CUIFramePtr toolWindow( parent->hToolWindow );
	if(toolWindow != NULL)	{
		toolWindow->SetSystemWindow( FALSE );
	}
}

void BrushPresetMgr::OnToolbarsPostLoad(void *param, NotifyInfo *info) {
	BrushPresetMgr* parent = (BrushPresetMgr*)param;
	if( parent->hToolWindow==NULL ) return;

	CUIFramePtr toolWindow( parent->hToolWindow );
	if(toolWindow != NULL )	{
		toolWindow->SetSystemWindow( TRUE );
	}
}

// Function publishing
BaseInterface* BrushPresetMgr::GetInterface(Interface_ID id) {
	if( id==IBRUSHPRESETMGR_INTERFACE_ID )
		return this;
	return NULL;
}

BOOL BrushPresetMgr::fnIsActive() {
	return (activeContexts.IsEmpty()? FALSE:TRUE);
}

void BrushPresetMgr::fnOpenPresetMgr() {
	OnPresetMgrBtn();
}

void BrushPresetMgr::fnAddPreset() {
	OnAddPresetBtn();
}

void BrushPresetMgr::fnLoadPresetFile( TCHAR* file ) {
	ReadConfig( file );
}

void BrushPresetMgr::fnSavePresetFile( TCHAR* file ) {
	WriteConfig( file );
}

BrushPresetMgr::ContextItem::ContextItem( Class_ID contextID )
{ this->contextID = contextID, this->isStandin = FALSE, this->isFinal = FALSE, this->context = NULL; }


/*
Michael Russo says:
		mhBookmarkWnd = CreateCUIFrameWindow(Max()->GetMAXHWnd(), GetString(IDS_FRAME_NAME), 0, 0,
							mpBookmarkPosData->GetWidth(0,0), mpBookmarkPosData->GetHeight(0,0));
Michael Russo says:
	mhBookmarkDialog = CreateDialogParam( hInstance, MAKEINTRESOURCE(IDD_BOOKMARK), 
											mhBookmarkWnd, BookmarkDlgProc, (LPARAM)this);
*/


//-----------------------------------------------------------------------------
// class CreatePresetDialog

void CreatePresetDialog::SetPresetName( TCHAR* name ) {
	if( name==NULL )	presetName[0] = 0;
	else				_tcscpy( presetName, name );
}

INT_PTR CALLBACK CreatePresetDialog::DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CreatePresetDialog* parent;

	if(msg == WM_INITDIALOG) {
		parent = (CreatePresetDialog*)(lParam);
		parent->hWnd = hWnd;
		DLSetWindowLongPtr(hWnd, lParam);

		TSTR name = parent->GetPresetName();
		SetDlgItemText( hWnd, IDC_PRESET_NAME, name );

		CenterWindow( hWnd, GetWindow( hWnd, GW_OWNER ));
		SetFocus( GetDlgItem( hWnd,IDC_PRESET_NAME ) );
		SetWindowContextHelpId( hWnd, idh_dialog_createbrushpreset );
	}
	else
		parent = DLGetWindowLongPtr<CreatePresetDialog*>(hWnd);

    switch (msg)  {
	case WM_SYSCOMMAND:
		if((wParam & 0xfff0) == SC_CONTEXTHELP) {
			DoHelp(HELP_CONTEXT, idh_dialog_createbrushpreset);
			return 0;
		}
		break;

    case WM_COMMAND:
    	switch (LOWORD(wParam))  {
        	case IDCANCEL:
				parent->EndDialog( FALSE );
				break;
        	case IDOK: {
				TCHAR buf[256];
				GetDlgItemText( hWnd, IDC_PRESET_NAME, buf, 256 );

				parent->SetPresetName( buf );
				parent->EndDialog( TRUE );
				break;
			}
        }
        break;
    }

    return FALSE;
}

int CreatePresetDialog::DoDialog() {
	//int numPresets = GetIBrushPresetMgr()->GetNumPresets();
	//_stprintf( presetName, "%s%i", GetString(IDS_PRESET), numPresets+1 );


	HWND hParent = GetCOREInterface()->GetMAXHWnd();
	return DialogBoxParam(
		hInstance, 
		MAKEINTRESOURCE(IDD_CREATEPRESET), 
		hParent, 
		DlgProc,
		(LPARAM)this);
}

void CreatePresetDialog::EndDialog( BOOL result ) {
	::EndDialog( hWnd, result );
}
