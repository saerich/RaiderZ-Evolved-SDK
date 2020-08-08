/**********************************************************************
 *<
	FILE: util.h

	DESCRIPTION: Utility classes

	CREATED BY: Hans Larsen

	HISTORY:

 *>	Copyright (c) 2007, All Rights Reserved.
 **********************************************************************/
// Some functions here are in an anonymous namespace (notably for 
// SmartPtr) for not exporting the symbols that could corrupt the final 
// obj symbol space.  Since most of them are small (often one-liners) 
// and inlined, it would be moot to put them in a separate cpp file. So 
// they are here for compiler optimization, and in an anonymous 
// namespace to not exporting them.
#pragma once

#include <algorithm>
#include <string>

extern HINSTANCE hInstance;

/////////////////////////////////////////////////////////////////////////////
#pragma region UNICODE
// Define _tstring as a unicode string if needed, otherwise a simple MB string.
// Due to standard nothing can be added in ::std so we add it to our namespace.
#ifdef UNICODE
   typedef std::wstring _tstring;
#else
   typedef std::string  _tstring;
#endif
typedef std::basic_ostringstream< _tstring::value_type > _tostringstream;
#pragma endregion
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
#pragma region Min Max Clamp Rotate
// Import min and max to permit overloading and specialization.
// This is as fast and more versatile than the standard C #define.
// Must undef here because #define will tamper the later use.
// Define NOMINMAX to be sure it won't be defined if windows.h is
// included later.
#define NOMINMAX
#undef min
#undef max

// Define a min/max tree for 3 args and import the standard min/max.
using std::min;
using std::max;
template< typename T >
   inline const T& min( const T& a, const T& b, const T& c ) 
   { return min(min(a, b), c); }
template< typename T >
   inline const T& max( const T& a, const T& b, const T& c ) 
   { return max(max(a, b), c); }

// Specialize min, max and clamp for colors.  min/max each attribute.
namespace {
   Color min( const Color& a, const Color& b )
   { return Color( ::min(a.r,b.r), 
                   ::min(a.g,b.g), 
                   ::min(a.b,b.b) ); }
   Color max( const Color& a, const Color& b )
   { return Color( ::max(a.r,b.r), 
                   ::max(a.g,b.g), 
                   ::max(a.b,b.b) ); }

   AColor min( const AColor& a, const AColor& b )
   { return AColor( ::min(a.r,b.r), 
                    ::min(a.g,b.g), 
                    ::min(a.b,b.b),
                    ::min(a.a,b.a)); }
   AColor max( const AColor& a, const AColor& b )
   { return AColor( ::max(a.r,b.r), 
                    ::max(a.g,b.g), 
                    ::max(a.b,b.b),
                    ::max(a.a,b.a) ); }
}

// Returns a, b or c such that b <= a <= c (clamping a value between two)
// If b > c, always returns c (which is expected but not ideal behaviour).
template< typename T >
   inline T clamp( T a, T b, T c ) { return min( max( a, b ), c ); }

// Returns (a) such that (b) <= (a) < (c), by confining (a) in a curved
// linear space (example, if b == 0, it's effectively a modulo c, and 
// rotate(3, 5, 8) == 6). This works for values that does not define
// a modulo operator (such as float).
// The specialization is due to a bug that was happening in float and doubles
// and needed to check for infinity.
template< typename T >
   inline T rotate( T a, T b, T c ) {
      if (c < b)
         return a;
      const T delta( c - b );
      while( a <  b ) a += delta;
      while( a >= c ) a -= delta;
      return a;
   }

// Specialization for floats/doubles with an optimization.
template< >
   inline double rotate( double a, double b, double c ) {
      if (c < b)
         return a;
      switch( _fpclass( a ) ) {
         case _FPCLASS_NINF: return b;
         case _FPCLASS_PINF: return b;
         case _FPCLASS_SNAN:
         case _FPCLASS_QNAN: return a;
      };
      const double delta( c - b );
      while( a <  b ) a += delta;
      while( a >= c ) a -= delta;
      return a;
   }

template< >
   inline float rotate( float a, float b, float c ) {
      return rotate< double >(a, b, c);
   }

#pragma endregion
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
#pragma region Constant Colors
// Define some simple colors
const Color Black     ( 0, 0, 0 );
const Color White     ( 1, 1, 1 );
#pragma endregion
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Exception-safe guard against throw. Not Thread-safe.
// Basically, it takes a value and sets it to whatever you want.  Then when
// the guard goes out of scope it reverts the value.
template< typename Type >
class ValueGuard {
   Type& m_Ref;
   Type  m_Old;
public:
   ValueGuard(Type& r, Type NewValue) : m_Ref(r), m_Old(r) { m_Ref = NewValue; }
   ~ValueGuard() { m_Ref = m_Old; }
};

// Generic Smart manager for resources (ICust* or else).
// Reference-counted implementation (copy-construction safe).  
// You should NOT box or unbox this SmartResource in an implicit manner. Full 
// ownership of the resource should be passed to this class.  There's a lot 
// of troubling implications if you add membership management functions.  
// Normally you should only stay with unboxing (function move - free m_Pimpl 
// without deleting Type and return the pointer), although even that could 
// cause problems if other SmartResource are still pointing to impl.
// Want an advice? Use it like it was meant to.
template< class T > void DoNothing( T p ) {}
template< class Type, void (*Free)(Type) = DoNothing<Type> >
class SmartResource {
   // Implementation.  Contains a count and a value.  Once the count
   // reaches 0, free the value.  BTW, it is considered invalid to
   // actually change the value itself (for obvious reasons) - so if
   // you need to point elsewhere just dec and create another impl.
   // Support for move have been added but not implemented in the
   // SmartResource.
   // Anyway this is private to the helper.
   class impl {
      Type   m_Value;
      size_t m_Count; // Number of SmartResource pointing this
      ~impl() {}     // Private so that only we can delete.
      
      // Unimplemented members
      impl();
      impl(const impl&);
      impl& operator=(const impl&);
   public:
      impl( Type v ) : m_Value( v ), m_Count( 1 ) {}
      operator       Type()       { return m_Value; }
      operator const Type() const { return m_Value; }
      void inc()  throw() { m_Count++; }
      void dec()  throw() { if (0 == --m_Count) clean(); }
      // This member unbox the value and delete the Pimpl.
      // >>> INVALIDATES EVERY SmartResource POINTING TO this <<<
      Type move() throw() { Type value(m_Value); delete this; return value; }

      // Clean this resource, which means unbox the value and call Free.
      // This should be used wisely, as it invalidates every client of this
      // resource.
      void clean() throw() { Free(move()); }
   } *m_Pimpl;
   
public:
   SmartResource()
      : m_Pimpl( 0 )
   {}
   explicit SmartResource( Type ptr )
      : m_Pimpl( new impl( ptr ) )
   {}
   SmartResource( const SmartResource& src ) 
      : m_Pimpl( src.m_Pimpl )
   { if (m_Pimpl) m_Pimpl->inc(); }

   ~SmartResource()
   { if (m_Pimpl) m_Pimpl->dec(); }

   // Only swap pimpls.
   void swap( SmartResource& ptr ) throw() { std::swap( m_Pimpl, ptr.m_Pimpl ); }

   // Assignments
   // This is exception-safe.
   SmartResource& operator=( Type ptr )                 { return Assign(ptr); }
   SmartResource& operator=( const SmartResource& ptr ) { return Assign(ptr); }

   SmartResource& Assign( Type ptr )
      { SmartResource NewValue( ptr ); swap( NewValue ); return *this; }
   SmartResource& Assign( const SmartResource& ptr )
      { SmartResource NewValue( ptr ); swap( NewValue ); return *this; }

   bool IsNull() { return m_Pimpl == NULL; }

   // Accessors
   // If this is a nil resource, this gonna assert.
   operator       Type()       { assert(m_Pimpl); return *m_Pimpl; }
   operator const Type() const { assert(m_Pimpl); return *m_Pimpl; }

         Type get()       { assert(m_Pimpl); return (Type)(*this); }
   const Type get() const { assert(m_Pimpl); return (Type)(*this); }
};

namespace { template< class T > void DeletePtr( T* p ) { delete p; } }
// Specialization for pointers, namely smart pointers
// By default, delete the Type value (hoping it's a pointer), acting 
// exactly like a SmartPtr (not an auto_ptr!). You can specialize Free by
// specifying a function that receives a Type and do something. It could 
// even be nothing if the pointer is managed elsewhere. 
template< typename Type, void (*Free)(Type*) = DeletePtr<Type> >
struct SmartPtr : public SmartResource< Type*, Free >
{
   SmartPtr()
      : SmartResource()
   {}
   explicit SmartPtr( Type* ptr )
      : SmartResource( ptr )
   {}
   SmartPtr( const SmartPtr& src ) 
      : SmartResource( src )
   {}

   // Need to redefine those here.
   SmartPtr& operator=( Type* ptr )                { return Assign(ptr); }
   SmartPtr& operator=( const SmartResource& ptr ) { return Assign(ptr); }

   SmartPtr& Assign( Type* ptr )
      { SmartPtr NewValue( ptr ); swap( NewValue ); return *this; }
   SmartPtr& Assign( const SmartResource& ptr )
      { SmartPtr NewValue( ptr ); swap( NewValue ); return *this; }

         Type* operator->()       { return       (Type*)(*this); }
   const Type* operator->() const { return (const Type*)(*this); }
         Type& operator* ()       { return *this; }
   const Type& operator* () const { return *this; }

   bool  IsNull() { return SmartResource::IsNull() || get() == NULL; }
};

// Specialization for MAX interfaces.  Basically calls DeleteThis instead
// of deleting the pointer.
namespace { template< class T > void DeleteIface( T* p ) { p->DeleteThis(); } }
template< typename Type >
struct SmartInterface : public SmartPtr< Type, DeleteIface >
{
   SmartInterface()
      : SmartPtr()
   {}
   explicit SmartInterface( Type* ptr )
      : SmartPtr( ptr )
   {}
   SmartInterface( const SmartPtr& src ) 
      : SmartPtr( src )
   {}

   // Need to redefine those here.
   SmartInterface& operator=( Type* ptr )                { return Assign(ptr); }
   SmartInterface& operator=( const SmartResource& ptr ) { return Assign(ptr); }

   SmartInterface& Assign( Type* ptr )
      { SmartInterface NewValue( ptr ); swap( NewValue ); return *this; }
   SmartInterface& Assign( const SmartResource& ptr )
      { SmartInterface NewValue( ptr ); swap( NewValue ); return *this; }

         Type*operator->()       { return       (Type*)(*this); }
   const Type*operator->() const { return (const Type*)(*this); }
         Type&operator* ()       { return *this; }
   const Type&operator* () const { return *this; }
};

// Specialization for ICustButton.
struct ButtonPtr : public SmartPtr< ICustButton, ReleaseICustButton > {
   explicit ButtonPtr( ICustButton* p )
      : SmartPtr( p )
   {}
   ButtonPtr( HWND parent, int res_id )
      : SmartPtr( ::GetICustButton( ::GetDlgItem( parent, res_id ) ) )
   {}
   explicit ButtonPtr( HWND wnd )
      : SmartPtr( ::GetICustButton( wnd ) )
   {}
   explicit ButtonPtr( const ButtonPtr& ptr )
      : SmartPtr( ptr )
   {}
};

// Specialization for ICustEdit.
class EditPtr : public SmartPtr< ICustEdit, ReleaseICustEdit > {
public:
   explicit EditPtr( ICustEdit* p )
      : SmartPtr( p )
   {}
   EditPtr( HWND parent, int res_id )
      : SmartPtr( ::GetICustEdit( ::GetDlgItem( parent, res_id ) ) )
   {}
   explicit EditPtr( HWND wnd )
      : SmartPtr( ::GetICustEdit( wnd ) )
   {}
   explicit EditPtr( const EditPtr& ptr )
      : SmartPtr( ptr )
   {}
};

// Specialization for ISpinnerControl.
class SpinPtr : public SmartPtr< ISpinnerControl, ReleaseISpinner > {
public:
   explicit SpinPtr( ISpinnerControl* p )
      : SmartPtr( p )
   {}
   SpinPtr( HWND parent, int res_id )
      : SmartPtr( ::GetISpinner( ::GetDlgItem( parent, res_id ) ) )
   {}
   explicit SpinPtr( HWND wnd )
      : SmartPtr( ::GetISpinner( wnd ) )
   {}
   explicit SpinPtr( const SpinPtr& ptr )
      : SmartPtr( ptr )
   {}
};

// Specialization for IRollupWindow
class RollupWindowPtr : public SmartPtr< IRollupWindow, ReleaseIRollup > {
public:
   RollupWindowPtr()
      : SmartPtr()
   {}
   explicit RollupWindowPtr( IRollupWindow* p )
      : SmartPtr( p )
   {}
   RollupWindowPtr( HWND parent, int res_id )
      : SmartPtr( ::GetIRollup( ::GetDlgItem( parent, res_id ) ) )
   {}
   explicit RollupWindowPtr( HWND wnd )
      : SmartPtr( ::GetIRollup( wnd ) )
   {}
   explicit RollupWindowPtr( const RollupWindowPtr& ptr )
      : SmartPtr( ptr )
   {}

   SmartPtr& operator=( IRollupWindow* ptr )       { return SmartPtr::operator =(ptr); }
   SmartPtr& operator=( const SmartResource& ptr ) { return SmartPtr::operator =(ptr); }
};

// This is a weird use of the SmartPtr class
// Implements a HBITMAP RAII.  Can be used as an example
// for other "weird" need of the design pattern.
// Anonymous free-er compatible with interface of SmartPtr.
namespace { void DeleteHBITMAP( HBITMAP ptr ) { DeleteObject( ptr ); } }
class SmartHBITMAP : public SmartResource< HBITMAP, DeleteHBITMAP > {
public:
   explicit SmartHBITMAP( HBITMAP v )
      : SmartResource( v )
   {}
   SmartHBITMAP( const SmartHBITMAP& src ) 
      : SmartResource( src )
   {}
};

// Specialization for HIMAGELIST
namespace { void DeleteHIMAGELIST( HIMAGELIST ptr ) { ImageList_Destroy( ptr ); } }
class HIMAGELISTPtr : public SmartResource< HIMAGELIST, DeleteHIMAGELIST > {
public:
   explicit HIMAGELISTPtr( HIMAGELIST v )
      : SmartResource( v )
   {}
   HIMAGELISTPtr( const HIMAGELISTPtr& src ) 
      : SmartResource( src )
   {}

   HIMAGELISTPtr& operator=( HIMAGELIST v ) 
      { SmartResource::operator=( v ); return *this; }
};

// Specialization for TexHandle* (needed by viewport interactive rendering)
typedef SmartInterface< TexHandle > TexHandlePtr;

// Render a Texture and returns its HBITMAP.  If Texture is NULL,
// give back a big blacked HBITMAP.
// This is needed for the preview in the texture buttons.
SmartHBITMAP RenderBitmap( Texmap* Texture, TimeValue t, 
                           size_t  width,   size_t    height,
                           int     type = BMM_TRUE_32 );


/////////////////////////////////////////////////////////////////////////////
// Color operations and functions
// These are defined inside [color_correction.cpp] if they aren't here.

// Transform a RGB color using a unary or binary predicate.
template< typename Pred >
   Color Transform( const Color& x, const Color& y, const Pred& f )
      { return Color( f( x.r, y.r ), f( x.g, y.g ), f( x.b, y.b ) ); }
template< typename Pred >
   Color Transform( const Color& x, const Pred& f )
      { return Color( f( x.r ), f( x.g ), f( x.b ) ); }

// HSL color-space related functions...
// This is just a namespace with helper functions that doesn't exist
// in max.
namespace HSL {
   float CalcHue( const Color& c );
   float CalcLum( const Color& c );
}
Color RGBtoHSL( const Color& in );
Color HSLtoRGB( const Color& in );
