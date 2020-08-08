/**********************************************************************
 *<
   FILE:        composite.cpp
   DESCRIPTION: A compositor Texture map.  Based on the ole' Composite
                Texture from Rolf Berteig.
   CREATED BY:  Hans Larsen
   HISTORY:     Created 01.aug.2007
 *>   Copyright (c) 2007, All Rights Reserved.
 **********************************************************************/

#pragma region Includes

// Max headers
#include "mtlhdr.h"
#include "mtlres.h"
#include "mtlresOverride.h"
#include "stdmat.h"
#include "iparamm2.h"
#include "macrorec.h"
#include "IHardwareMaterial.h"
#include "3dsmaxport.h"
#include "hsv.h"
#include "iFnPub.h"

// Custom headers
#include "util.h"

// Internal project dependencies
#include "color_correction.h"

// Standard headers
#include <vector>
#include <list>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <functional>
#include <map>

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Notes
/******************************************************************************
   Reference system:
      The reference Index works as follow for textures:
         [0]      - The parameter block
         [1..n(   - The Textures (1)
         [n..2n(  - The Masks    (1)
      This system is used to keep the same reference indices between the old
      version and the new one (so that loading the param block will set the
      same references, easing the backward-compatibility contract).
      (1) When using an index instead of a reference number, it is simply
          (as of the writing of this text) the index inside the
          [texture,mask( range of reference numbers.
          Ideally, if you were to add new references, you could add them at
          the beginning and change the indexing functions (see
          Texture::idx_to_lyr) so that the indexing system stays the same
          inside the range. This way the changes will stay minimal.
   ----------------------------------------------------------------------------
   A bit of explanation concerning the class diagram here.  Here it is in
   short (using ASCII UML. <>-> is association, <#>-> is composition):

      *   _____________   1  1   _______   1      *   ________________
      ,->| LayerDialog |<>----->| Layer |<#>-------->| LayerParam<Id> |
      |   ¯¯¯¯¯¯¯¯¯¯¯¯?         ¯¯¯¯¯¯?             ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
      |                             ^ *
      |                             `-----------.
      |  1   ________   1    1   __________   1 |
      `--<#>| Dialog |<-------<>| Texture |<#>-'
             ¯¯¯¯¯¯¯¯            ¯¯¯¯¯¯¯¯¯¯
   So basically, Dialog is a composition of LayerDialog which basically
   manage the update of the interface based on the properties of a Layer.
   Bear in mind that there is an operation (Rewire) that re-associate the
   LayerDialogs to the Layers, and add/remove LayerDialog if necessary. Since
   the only role of LayerDialog is to show information based on its Layer
   pointer, there is no black magic involved.
   ----------------------------------------------------------------------------
   A Layer is composited of multiple LayerParam. Due to backward compatibility,
   it was simpler to put a BlockParam2 that had many tables than one which had
   one table of BlockParam2. Basically, a Layer points to a BlockParam2 (of
   the Texture), has an index, and one LayerParam<Id> for each field
   inside this BlockParam2. The LayerParam<Id> exists only to encapsulate the
   GetValue and SetValue, the Id and a cache if necessary.
   To add a parameter to each layers, simply add the field in the BlockParam2,
   add a LayerParam<Id> to Layer, with the corresponding Id and type, and add
   accessors to this param inside the Layer class. Then, you have to update
   the functions ValidateParamBlock and UpdateParamBlock to ensure the count
   of the field inside the ParamBlock2 is still valid. If you want that
   parameter to show in the interface, change LayerDialog::Update and the
   resource file.
   ----------------------------------------------------------------------------
   If you are new to this code, understand some of the concepts in util.h,
   then start reading the simple code of LayerParam<Id>, then read on Layer
   and LayerDialog and understand them correctly. Blendmodes should be next,
   and after that you'll be able to check Texture and Dialog and their
   implementation.
   ----------------------------------------------------------------------------
   This is a gigantic file but it's separated in regions to ease reading and
   maintenance. The particular structure of the map couldn't let us use the
   param block to its full extent, and this has resulted in a lot of lines that
   mainly link the param block values to their UI elements. The design was
   separated and simplified to leave it easier to maintain.
   ----------------------------------------------------------------------------
   STL was used heavily in this file.  Be sure you have used or understand
   algorithms, streams and sequences.  Some methods from functional were used
   as well.
   ----------------------------------------------------------------------------
   Please note the use of SmartPtr for almost everything that require
   acquisition and release.  This include every GetICust... and ReleaseICust...
   for custom controls as well as many classes.
   This automates memory management and cleans the code and design.
   There's a few "new" in this file, but no delete . Don't be surprised, it's
   all done once the smart pointers pointing to that class are destroyed.
   Same goes for ReleaseICust.
   ----------------------------------------------------------------------------
   Hope this helped.
*******************************************************************************/
#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Color Blending

//CODE from hsv.cpp  the old RGBtoHSV would return black on colors < 0 or > 1
//this is a temporary fix to remove that clamp since we dont want to have to test
//all the RGBtoHSV at this stage.  This should be reevaluated for Renior.
#define MAX3(a,b,c) ((a)>(b)?((a)>(c)?(a):(c)):((b)>(c)?(b):(c)))
#define MIN3(a,b,c) ((a)<(b)?((a)<(c)?(a):(c)):((b)<(c)?(b):(c)))

Color RGBtoHSV2(Color rgb)
{
	float h, s, r, g, b, vmx, V, X;
	Color res;

	V = MAX3(rgb.r,rgb.g,rgb.b);
	X = MIN3(rgb.r,rgb.g,rgb.b);

	if (V==X) {
		h = (float)0.;
		s = (float)0.;
	} else {
		vmx = V - X;
		s = vmx/V;
		r = ( V - rgb.r )/vmx;
		g = ( V - rgb.g )/vmx;
		b = ( V - rgb.b )/vmx;
		if( rgb.r == V )
			h = ( rgb.g == X ) ?  (float)5.+b : (float)1.-g;
		else if( rgb.g == V )
			h = ( rgb.b == X ) ? (float)1.+r : (float)3.-b;
		else
			h = ( rgb.r == X ) ? (float)3.+g : (float)5.-r;
		h /= (float)6.;
	}
	res.r = h; 	// h
	res.g = s;	// s
	res.b = V;	// v
	return res;
}

// If you modify the blending modes, don't forget to adapt the ones in the 
// Mental Ray shader. Since it's only an integer that is passed, they MUST be
// in the same order in ModeArray.
namespace Blending {
   struct BaseBlendMode {
      virtual float    operator()( const float    fg,  const float     bg  ) const = 0;
      virtual ::Color  operator()( const ::Color& fg,  const ::Color & bg  ) const = 0;

      virtual _tstring Name      ( )                                      const = 0;
   };

   template< typename SubType >
   struct BlendMode : public BaseBlendMode {
      static const BaseBlendMode* i() { 
         static SubType instance;
         return &instance; 
      }

      // Builds a color, blend it, then return the mix of the result.
      // This might be used in some mono case.
      // THIS MIGHT BE INFINITELY RECURSIVE IF NEITHER OPERATOR IS DEFINED,
      // since they depend on one another, so you can define one operator
      // without the other.
      float operator()( const float fg,  const float bg ) const { 
         ::Color colfg ( fg, fg, fg ); 
         ::Color colbg ( bg, bg, bg );
         // Apply blending.
         ::Color result( (*(i()))(colfg, colbg) );
         return ( result.r + result.g + result.b ) / 3.0f;
      }

		::Color operator()( const Color& fg, const Color& bg ) const {
			return Transform( fg, bg, *(i()) ); }

		_tstring Name() const {
			return _tstring( GetString( SubType::NameResourceID ) ); }
   };

   /////////////////////////////////////////////////////////////////////////////
   // Full list of Blending modes
   // To add one, copy an existing mode, change the value of the NameResourceID
   // enum to the resource used for its Name and change the content of 
   // operator() so it applies the function and returns the value.
   // Finally, to make the Blending type appear in the combo box and be usable
   // in scripts, just add a new reference in the ModeArray initialization.
   // Don't forget to update the Mental Ray shader if you do so.  If the blend
   // mode is invalid in Mental Ray, it will return all black so you'll know.

   // If you define a [float operator()(...)], you won't need to define a
   // [Color operator()(...)] since it will call compose and call the float
   // operator on each component (RGB, A will be a mix of alphas).
   // Likewise, if you define a [Color operator()(...)], the other will only
   // apply it in cases where it needs a float (EvalMono).

   // We estimate that all colors are clamped PRIOR to the call.

   /////////////////////////////////////////////////////////////////////////////
   // Normal
   struct Normal : public BlendMode< Normal > {
      enum { NameResourceID = IDS_BLENDMODE_NORMAL };
		::Color operator()( const ::Color& fg, const ::Color& bg ) const {
			return fg; }
   };
   struct Average : public BlendMode< Average > {
      enum { NameResourceID = IDS_BLENDMODE_AVERAGE };
		float operator()( const float fg, const float bg ) const {
			return (fg + bg) / 2.0f; }
   };
   struct Add : public BlendMode< Add > {
      enum { NameResourceID = IDS_BLENDMODE_ADD };
		float operator()( const float fg, const float bg ) const {
			return  bg + fg; }
   };
   struct Subtract : public BlendMode< Subtract > {
      enum { NameResourceID = IDS_BLENDMODE_SUBTRACT };
		float operator()( const float fg, const float bg ) const {
			return bg - fg; }
   };

   /////////////////////////////////////////////////////////////////////////////
   // Darkening
   struct Darken : public BlendMode< Darken > {
      enum { NameResourceID = IDS_BLENDMODE_DARKEN };
		float operator()( const float fg, const float bg ) const {
			return min( fg, bg ); }
   };
   struct Multiply : public BlendMode< Multiply > {
      enum { NameResourceID = IDS_BLENDMODE_MULTIPLY };
		float operator()( const float fg, const float bg ) const {
			return bg * fg; }
   };
   struct ColorBurn : public BlendMode< ColorBurn > {
      enum { NameResourceID = IDS_BLENDMODE_COLORBURN };
		float operator()( const float fg, const float bg ) const {
			return (fg == 0.0f) ? 0.0f : max( 1.0f - (1.0f - bg) / fg, 0.0f ); }
   };
   struct LinearBurn : public BlendMode< LinearBurn > {
      enum { NameResourceID = IDS_BLENDMODE_LINEARBURN };
		float operator()( const float fg, const float bg ) const {
			return max(fg + bg - 1.0f, 0.0f); }
   };
   
   /////////////////////////////////////////////////////////////////////////////
   // Lightening
   struct Lighten : public BlendMode< Lighten > {
      enum { NameResourceID = IDS_BLENDMODE_LIGHTEN };
		float operator()( const float fg, const float bg ) const {
			return max( fg, bg ); }
   };
   struct Screen : public BlendMode< Screen > {
      enum { NameResourceID = IDS_BLENDMODE_SCREEN };
		float operator()( const float fg, const float bg ) const {
			return fg + bg - fg * bg; }
   };
   struct ColorDodge : public BlendMode< ColorDodge > {
      enum { NameResourceID = IDS_BLENDMODE_COLORDODGE };
		float operator()( const float fg, const float bg ) const {
			return (fg == 1.0f) ? 1.0f : min( bg / (1.0f - fg), 1.0f ); }
   };
   struct LinearDodge : public BlendMode< LinearDodge > {
      enum { NameResourceID = IDS_BLENDMODE_LINEARDODGE };
		float operator()( const float fg, const float bg ) const {
			return min( fg + bg, 1.0f ); }
   };

   /////////////////////////////////////////////////////////////////////////////
   // Spotlights
   struct Spotlight : public BlendMode< Spotlight > {
      enum { NameResourceID = IDS_BLENDMODE_SPOT };
		float operator()( const float fg, const float bg ) const {
			return min( 2.0f * fg * bg, 1.0f ); }
   };
   struct SpotlightBlend : public BlendMode< SpotlightBlend > {
      enum { NameResourceID = IDS_BLENDMODE_SPOTBLEND };
		float operator()( const float fg, const float bg ) const {
			return min( fg * bg + bg, 1.0f ); }
   };

   /////////////////////////////////////////////////////////////////////////////
   // Lighting
   struct Overlay : public BlendMode< Overlay > {
      enum { NameResourceID = IDS_BLENDMODE_OVERLAY };
		float operator()( const float fg, const float bg ) const {
			return clamp( bg <= 0.5f ? 2.0f * fg * bg
                                      : 1.0f - 2.0f * (1.0f-fg) * (1.0f-bg),
                         0.0f, 1.0f ); }
   };
   struct SoftLight : public BlendMode< SoftLight > {
      enum { NameResourceID = IDS_BLENDMODE_SOFTLIGHT };
		float operator()(const float a, const float b) const {
			return clamp( a <= 0.5f ? b * (b + 2.0f*a*(1.0f - b))
									: b + (2.0f*a - 1.0f)*(sqrt(b) - b),
						  0.0f, 1.0f ); }
   };
   struct HardLight : public BlendMode< HardLight > {
      enum { NameResourceID = IDS_BLENDMODE_HARDLIGHT };
		float operator()(const float a, const float b) const {
			return clamp( a <= 0.5f ? 2.0f*a*b
									: 1.0f - 2.0f*(1.0f - a)*(1.0f - b),
						  0.0f, 1.0f ); }
   };
   struct PinLight : public BlendMode< PinLight > {
      enum { NameResourceID = IDS_BLENDMODE_PINLIGHT };
		float operator()(const float a, const float b) const {
			return (a > 0.5f && a > b) || (a < 0.5f && a < b) ? a : b; }
   };
   struct HardMix : public BlendMode< HardMix > {
      enum { NameResourceID = IDS_BLENDMODE_HARDMIX };
		float operator()(const float a, const float b) const {
			return a + b <= 1.0f ? 0.0f : 1.0f; }
   };

   /////////////////////////////////////////////////////////////////////////////
   // Difference
   struct Difference : public BlendMode< Difference > {
      enum { NameResourceID = IDS_BLENDMODE_DIFFERENCE };
		float operator()(const float a, const float b) const {
			return abs(a - b); }
   };
   struct Exclusion : public BlendMode< Exclusion > {
      enum { NameResourceID = IDS_BLENDMODE_EXCLUSION };
		float operator()( const float fg, const float bg ) const {
			return fg + bg - 2.0f * fg * bg; }
   };

   // HSV
   struct Hue : public BlendMode< Hue > {
      enum { NameResourceID = IDS_BLENDMODE_HUE };
      ::Color operator()( const ::Color& fg, const ::Color& bg ) const {
			const ::Color hsv_fg( ::RGBtoHSV2( ::Color(fg.r, fg.g, fg.b) ) );
			const ::Color hsv_bg( ::RGBtoHSV2( ::Color(bg.r, bg.g, bg.b) ) );
         return ::Color(::HSVtoRGB( ::Color( hsv_fg[0], hsv_bg[1], hsv_bg[2] ) ));
      }
   };
   struct Saturation : public BlendMode< Saturation > {
      enum { NameResourceID = IDS_BLENDMODE_SATURATION };
      ::Color operator()( const ::Color& fg, const ::Color& bg ) const {
			const ::Color hsv_fg( ::RGBtoHSV2( ::Color(fg.r, fg.g, fg.b) ) );
			const ::Color hsv_bg( ::RGBtoHSV2( ::Color(bg.r, bg.g, bg.b) ) );
         return ::Color(HSVtoRGB( ::Color( hsv_bg[0], hsv_fg[1], hsv_bg[2] ) ));
      }
   };
   struct Color : public BlendMode< Color > {
      enum { NameResourceID = IDS_BLENDMODE_COLOR };
      ::Color operator()( const ::Color& fg, const ::Color& bg ) const {
			const ::Color hsv_fg( ::RGBtoHSV2( ::Color(fg.r, fg.g, fg.b) ) );
			const ::Color hsv_bg( ::RGBtoHSV2( ::Color(bg.r, bg.g, bg.b) ) );
         return ::Color(HSVtoRGB( ::Color( hsv_fg[0], hsv_fg[1], hsv_bg[2] ) ));
      }
   };
   struct Value : public BlendMode< Value > {
      enum { NameResourceID = IDS_BLENDMODE_VALUE };
      ::Color operator()( const ::Color& fg, const ::Color& bg ) const {
			const ::Color hsv_fg( ::RGBtoHSV2( ::Color(fg.r, fg.g, fg.b) ) );
			const ::Color hsv_bg( ::RGBtoHSV2( ::Color(bg.r, bg.g, bg.b) ) );
         return ::Color(HSVtoRGB( ::Color( hsv_bg[0], hsv_bg[1], hsv_fg[2] ) ));
      }
   };

   /////////////////////////////////////////////////////////////////////////////
   // Creating and managing the static list of blending modes.
   const BaseBlendMode* const ModeArray[] = {
         Normal    ::i(),
         Average   ::i(), Add            ::i(), Subtract  ::i(),
         Darken    ::i(), Multiply       ::i(), ColorBurn ::i(),  LinearBurn::i(),
         Lighten   ::i(), Screen         ::i(), ColorDodge::i(),  LinearDodge::i(),
         Spotlight ::i(), SpotlightBlend ::i(),
         Overlay   ::i(), SoftLight      ::i(), HardLight ::i(),  PinLight::i(), HardMix::i(),
         Difference::i(), Exclusion      ::i(),
         Hue       ::i(), Saturation     ::i(), Color     ::i(),  Value::i()
      };

   struct ModeListType {
      typedef const BaseBlendMode* const * const_iterator;
      const BaseBlendMode& operator []( int Index ) const {
         return *(ModeArray[ Index ]);
      }
      const_iterator begin() const { return &ModeArray[0]; }
      const_iterator end()   const { return &ModeArray[size()]; }
      size_t         size()  const { return sizeof(ModeArray)/sizeof(*ModeArray); }
   } ModeList;
}
#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Constants

// Composite namespace and all internally used global variables.
namespace Composite {

	// Chunk magic numbers.
	namespace Chunk {
		// For compatibility with the old COMPOSITE.
		namespace Compatibility {
			enum {
				Header         = 0x4000,
				Param2         = 0x4010,
				SubTexCount    = 0x0010,
				MapOffset      = 0x1000,
				MaxMapOffset   = 0x2000
			};
		};

		// We don't need the offsets and all that since everything
		// is now owned by the ParamBlock2.
		enum {
			Header            = 0x1000,
			Version           = 0x1001
		};
	};

	// Default values for new layers
	namespace Default {
		const float		Opacity			(  100.0f );
		const bool		Visible			( true );
		const bool		VisibleMask		( true );

		const float		OpacityMax		(  100.0f );
		const float		OpacityMin		(    0.0f );

		const int		BlendMode		(    0 );

		const bool		DialogOpened	( true );

		// Resource Id
		const _tstring Name				( _T("") );
	}

	namespace Param {
		// Maximum number of layers that can be constructed in ui or through FPS methods.
		const unsigned short LayerMax    ( 1024 );

		// Maximum number of layers that can exist, created by reading in from file
		// or by direct access to the PB2 properties via maxscript
		const unsigned int CategoryCounterMax ( 2<<30 );

		// The maximum number of Layer to show in the
		// interactive renderer.
		const unsigned short LayerMaxShown( 4 );
	}

	// Some interface constants
	namespace ToolBar {
		const int   IconHeight           ( 13 );
		const int   IconWidth            ( 13 );

		// Indices for the ToolBar icons in the image list
		const int   RemoveIcon           (  0 );
		const int   VisibleIconOn        (  1 );
		const int   VisibleIconOff       (  2 );
		const int   AddIcon              (  3 );
		const int   RenameIcon           (  4 );
		const int   DuplicateIcon        (  5 );
		const int   ColorCorrectionIcon  (  6 );
		const int   RemoveIconDis        (  7 );
		const int   AddIconDis           (  8 );
	}

}

////////////////////////////////////////////////////////////////////////
// Static assertions
// These are going to generate compilation error if they are not valid.
namespace {
	// LayerMax for interactive renderer must be under the LayerMax
	// because, obviously, you cannot have more Layer shown than the
	// maximum Layer available.
	C_ASSERT( Composite::Param::LayerMaxShown <= Composite::Param::LayerMax );
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Data Holders (Layers)

// Render a Texture and returns its HBITMAP.  If Texture is NULL,
// give back a big blacked HBITMAP.
// This is needed for the preview in the texture buttons.
SmartHBITMAP RenderBitmap( Texmap* Texture, TimeValue t,
						   size_t  width,   size_t    height,
						   int     type )
{
	BitmapInfo  bi;

	bi.SetWidth ( int(width)    );
	bi.SetHeight( int(height)   );
	bi.SetType  ( type          );
	bi.SetFlags ( MAP_HAS_ALPHA );
	bi.SetCustomFlag( 0 );

	Bitmap      *bm   ( TheManager->Create( &bi ) );
	HDC          hdc  ( GetDC( bm->GetWindow() )  );

	if (Texture) {
		// Unsure of why 50 works here, but it does (same as in the Bricks texture)
		Texture->RenderBitmap( t, bm, 50.0f, TRUE );
	}

	PBITMAPINFO  bitex( bm->ToDib() );
	HBITMAP      bmp  ( CreateDIBitmap( hdc, &bitex->bmiHeader,
										CBM_INIT,
										bitex->bmiColors,
										bitex,
										DIB_RGB_COLORS ) );

	// Freeing the allocated resource
	LocalFree( bitex );
	ReleaseDC( GetDesktopWindow(), hdc );
	bm->DeleteThis();

	// Return a managed resource of this...
	return SmartHBITMAP(bmp);
}

// A Layer description.
// Contains code to handle the Dialog as well.
namespace Composite {
	// Used to diff between a function which acts on cache and real data and
	// cache only.
	struct CacheOnly {};

	// Fore declaration of Layer
	class Layer;

	/////////////////////////////////////////////////////////////////////////////
#pragma region Layer Data

	// A class encapsulating one single parameter of one Layer
	template< typename Type, ParamID Id >
	class LayerParam {
	private:
		mutable Type m_Cache;
		void SetCache( const Type& value ) const {
			m_Cache = value;
		}

		Layer *m_Layer;

		LayerParam();
	public:
		LayerParam( Layer* aLayer             ) : m_Layer(aLayer), m_Cache(Type()) {}
		LayerParam( Layer* aLayer, Type value ) : m_Layer(aLayer) {
			operator=(value);
		}
		LayerParam( Layer* aLayer, const LayerParam<Type,Id>& value )
			: m_Layer( aLayer )
			, m_Cache( value.m_Cache )
		{}

		// We can make a copy-ction from a Param with a different ID,
		// as long as the type is same.
		template< ParamID IdSrc >
		LayerParam( const LayerParam<Type,IdSrc>& src )
			: m_Layer   ( src.m_Layer   )
			, m_Cache    ( src.m_Cache   )
		{}

		template< ParamID IdSrc >
		void swap( LayerParam< Type, IdSrc >& src ) {
			// Swap the values, the layers and the controllers.
			DbgAssert(m_Layer->ParamBlock() == src.m_Layer->ParamBlock());
			IParamBlock2 *iPB2 = m_Layer->ParamBlock();

			int       idx1 ( (int)    m_Layer->Index() );
			int       idx2 ( (int)src.m_Layer->Index() );
			iPB2->SwapControllers(Id, idx1, IdSrc, idx2);

			std::swap( m_Layer, src.m_Layer );
		}

		// Operators
		const LayerParam& operator=( const LayerParam& src ) {
			LayerParam cpy( src ); swap( cpy ); return *this;
		}

		operator Type() const { 
			UpdateCache(); return m_Cache; 
		}

		const LayerParam& operator =(const Type& value) {
			SetCache( value );
			UpdateParamBlock();
			return *this;
		}

		bool IsKeyFrame() const {
			return IsKeyFrame( m_Layer->CurrentTime() );
		}
		bool IsKeyFrame( TimeValue t ) const {
			return FALSE != m_Layer->ParamBlock()->KeyFrameAtTime( Id, t, int(m_Layer->Index()) );
		}

		// This function copy the tracks and is called notably during cloning of the layer.
		// This is templated since we can copy tracks from basically any params.
		template< typename TypeSrc, ParamID IdSrc >
		void SetController( const LayerParam<TypeSrc,IdSrc>* src, RemapDir &remap ) {
			IParamBlock2* pbsrc( src->m_Layer->ParamBlock() );
			IParamBlock2* pbdst(      m_Layer->ParamBlock() );
			size_t        isrc ( src->m_Layer->Index() );
			size_t        idst (      m_Layer->Index() );

			Control *ctrl = pbsrc->GetController( IdSrc, int(isrc) );
			pbdst->SetController( Id, int(idst), (Control*)remap.CloneRef(ctrl), false );
		}

		bool HasController( ) const {
			return m_Layer->ParamBlock()->GetController( Id, int(m_Layer->Index()) ) != NULL;
		}

		void Layer( Layer* layer ) { 
			m_Layer = layer;
		}

		void UpdateCache( ) const {
			m_Layer->Update(m_Layer->CurrentTime(), Interval());
		}

		void Update( TimeValue t, Interval& valid ) const {
			DbgAssert(m_Layer->ParamBlock() && m_Layer->ParamBlock()->Count( Id ) > m_Layer->Index());
			if (m_Layer->ParamBlock() && m_Layer->ParamBlock()->Count( Id ) > m_Layer->Index()) {
				m_Layer->ParamBlock()->GetValue( Id, t,
					m_Cache,
					valid,
					int(m_Layer->Index()) );
			}
		}

		void UpdateParamBlock() const {
			DbgAssert(m_Layer->ParamBlock() && m_Layer->ParamBlock()->Count( Id ) > m_Layer->Index());
			if (m_Layer->ParamBlock() && m_Layer->ParamBlock()->Count( Id ) > m_Layer->Index()) {
				m_Layer->ParamBlock()->SetValue( Id, m_Layer->CurrentTime(),
					m_Cache,
					int(m_Layer->Index()) );
			}
		}
	};

	/////////////////////////////////////////////////////////////////////////////
#pragma region LayerParam Specializations

	// Specialization for texmaps
	template< ParamID Id >
	class LayerParam< Texmap*, Id > {
	private:
		Layer             *m_Layer;
		mutable Texmap    *m_Cache;
		void SetCache( Texmap *value ) const {
			m_Cache = value;
		}

		LayerParam();
	public:
		LayerParam( Layer* aLayer                ) : m_Layer(aLayer), m_Cache(NULL) {}
		LayerParam( Layer* aLayer, Texmap* value ) : m_Layer(aLayer) {
			operator=(value);
		}
		LayerParam( Layer* aLayer, const LayerParam<Texmap*,Id>& value )
			: m_Layer( aLayer )
			, m_Cache( value.m_Cache )
		{}

		// We can make a copy-ction from a Param with a different ID,
		// as long as the type is same.
		template< ParamID IdSrc >
		LayerParam( const LayerParam<Texmap*,IdSrc>& src )
			: m_Layer    ( src.m_Layer   )
			, m_Cache    ( src.m_Cache   )
		{}

		template< ParamID IdSrc >
		void swap( LayerParam< Texmap*, IdSrc >& src ) {

			int       idx1 ( (int)    m_Layer->Index() );
			int       idx2 ( (int)src.m_Layer->Index() );
			Texmap  *map1 = NULL;
			m_Layer->ParamBlock()->GetValue( Id,0,map1, FOREVER,   idx1 );
			Texmap  *map2 = NULL;
			src.m_Layer->ParamBlock()->GetValue( IdSrc,0,map2, FOREVER, idx2 );
			if (map1) map1->SetAFlag(A_LOCK_TARGET);
			if (map2) map2->SetAFlag(A_LOCK_TARGET);
			m_Layer->ParamBlock()->SetValue(Id,0,map2,idx1);
			src.m_Layer->ParamBlock()->SetValue(IdSrc,0,map1,idx2);
			if (map1) map1->ClearAFlag(A_LOCK_TARGET);
			if (map2) map2->ClearAFlag(A_LOCK_TARGET);

			std::swap( m_Layer, src.m_Layer   );
		}

		// Operators
		const LayerParam& operator=( const LayerParam& src ) {
			LayerParam cpy( src ); swap( cpy ); return *this;
		}

		operator Texmap*()       { UpdateCache(); return m_Cache; }
		operator Texmap*() const { UpdateCache(); return m_Cache; }

		const LayerParam& operator =(Texmap* value) {
			SetCache(value);
			UpdateParamBlock();
			return *this;
		}

		void        Layer( Layer* l ) {
			m_Layer = l;
		}

		void UpdateCache( ) const {
			m_Layer->Update(m_Layer->CurrentTime(), Interval());
		}

		void Update( TimeValue t, Interval& valid ) const {
			DbgAssert(m_Layer->ParamBlock() && m_Layer->ParamBlock()->Count( Id ) > m_Layer->Index());
			if (m_Layer->ParamBlock() && m_Layer->ParamBlock()->Count( Id ) > m_Layer->Index()) {
				m_Layer->ParamBlock()->GetValue( Id,t,
					m_Cache,
					valid,
					int(m_Layer->Index()) );
			}
		}

		void UpdateParamBlock() const {
			DbgAssert(m_Layer->ParamBlock() && m_Layer->ParamBlock()->Count( Id ) > m_Layer->Index());
			if (m_Layer->ParamBlock() && m_Layer->ParamBlock()->Count( Id ) > m_Layer->Index()) {
				m_Layer->ParamBlock()->SetValue( Id, m_Layer->CurrentTime(),
					m_Cache,
					int(m_Layer->Index()) );
			}
		}
	};

	// Specialization for strings
	template< ParamID Id >
	class LayerParam< LPTSTR, Id > {
	private:
		mutable _tstring   m_Cache;
		void SetCache( const _tstring& value ) const {
			m_Cache = value;
		}

		Layer             *m_Layer;

		LayerParam();
	public:
		LayerParam( Layer* aLayer               ) : m_Layer(aLayer), m_Cache(_tstring()) {}
		LayerParam( Layer* aLayer, LPTSTR value ) : m_Layer(aLayer) {
			operator=(value);
		}
		LayerParam( Layer* aLayer, const LayerParam<LPTSTR,Id>& value )
			: m_Layer( aLayer )
			, m_Cache( value.m_Cache )
		{}

		// We can make a copy-ction from a Param with a different ID,
		// as long as the type is same.
		template< ParamID IdSrc >
		LayerParam( const LayerParam<LPTSTR,IdSrc>& src )
			: m_Layer    ( src.m_Layer   )
			, m_Cache    ( src.m_Cache   )
		{}

		template< ParamID IdSrc >
		void swap( LayerParam< LPTSTR, IdSrc >& src ) {
			// Make sure operators get called and
			// pblock stays valid.
			_tstring tmp( (LPCTSTR)*this );
			operator=((LPCTSTR)src);
			src = tmp.c_str();

			std::swap( m_Layer, src.m_Layer   );
		}

		// Operators
		const LayerParam& operator=( const LayerParam& src ) {
			LayerParam cpy( src ); swap( cpy ); return *this;
		}

		operator LPCTSTR() const {
			UpdateCache();
			return m_Cache.c_str();
		}
		operator const _tstring() const {
			UpdateCache();
			return m_Cache;
		}

		const LayerParam& operator =(const LPCTSTR value) {
			SetCache( _tstring(value) );
			UpdateParamBlock();
			return *this;
		}

		const LayerParam& operator =(const _tstring value) {
			SetCache( value );
			UpdateParamBlock();
			return *this;
		}

		void Layer( Layer* l ) {
			m_Layer = l;
			UpdateParamBlock();
		}

		void UpdateCache( ) const {
			m_Layer->Update(m_Layer->CurrentTime(), Interval());
		}

		void Update( TimeValue t, Interval& valid ) const {
			DbgAssert(m_Layer->ParamBlock() && m_Layer->ParamBlock()->Count( Id ) > m_Layer->Index());
			if (m_Layer->ParamBlock() && m_Layer->ParamBlock()->Count( Id ) > m_Layer->Index()) {
				const TCHAR* retVal;
				m_Layer->ParamBlock()->GetValue( Id, t,
					retVal,
					valid,
					int(m_Layer->Index()) );

				// Old version might not have had a value, so we keep default
				// if the pointer returned is NULL.
				if (retVal != NULL)
					m_Cache = retVal;
			}
		}

		void UpdateParamBlock() const {
			DbgAssert(m_Layer->ParamBlock() && m_Layer->ParamBlock()->Count( Id ) > m_Layer->Index());
			if (m_Layer->ParamBlock() && m_Layer->ParamBlock()->Count( Id ) > m_Layer->Index()) {
				m_Layer->ParamBlock()->SetValue( Id, m_Layer->CurrentTime(),
					const_cast<LPTSTR>(m_Cache.c_str()),
					int(m_Layer->Index()) );
			}
		}
	};

#pragma endregion
	/////////////////////////////////////////////////////////////////////////////

	class Layer {
	public:
		// Public typedefs
		typedef Texmap* TextureType;
		typedef BOOL    VisibleType;
		typedef float   OpacityType;
		typedef int     BlendType;
		typedef LPTSTR  NameType;
		typedef BOOL    DialogOpenedType;

	private:
		IParamBlock2	*m_ParamBlock;
		size_t			m_Index;
		TimeValue		m_CurrentTime;
		Interval		m_Validity;

		LayerParam< VisibleType,      BlockParam::Visible        > m_Visible;
		LayerParam< VisibleType,      BlockParam::VisibleMask    > m_VisibleMask;
		LayerParam< BlendType,        BlockParam::BlendMode      > m_Blend;
		LayerParam< NameType,         BlockParam::Name           > m_Name;
		LayerParam< DialogOpenedType, BlockParam::DialogOpened   > m_DialogOpened;
		LayerParam< OpacityType,      BlockParam::Opacity        > m_Opacity;
		LayerParam< TextureType,      BlockParam::Texture        > m_Texture;
		LayerParam< TextureType,      BlockParam::Mask           > m_Mask;

		// This cache is used specifically to return a pointer to a managed buffer
		// that stays in-scope.  Basically, there is the cache of the m_Name itself,
		// but since there is a modification to the Name (to include Texture Name,
		// etc), we need another cache.
		mutable _tstring m_NameCache;

	public:
		////////////////////////////////////////////////////////////////////////////////
		// Disabling the warning C4355 - this used in base member initializer list
		// The following code has been thought correctly and as such this warning may
		// be disabled.
#pragma warning( push )
#pragma warning( disable: 4355 )
		Layer( IParamBlock2 *pblock, size_t Index )
			: m_ParamBlock  ( pblock       )
			, m_Index       ( Index        )
			, m_Visible     ( this         )
			, m_Opacity     ( this         )
			, m_Texture     ( this         )
			, m_Mask        ( this         )
			, m_Blend       ( this         )
			, m_VisibleMask ( this         )
			, m_Name        ( this         )
			, m_DialogOpened( this         )
			, m_CurrentTime ( )
			, m_Validity    ( )
		{}

		// Copy-constructor
		Layer( const Layer& src )
			: m_ParamBlock  ( src.m_ParamBlock         )
			, m_Index       ( src.m_Index              )
			, m_Visible     ( this, src.m_Visible      )
			, m_Opacity     ( this, src.m_Opacity      )
			, m_Texture     ( this, src.m_Texture      )
			, m_Mask        ( this, src.m_Mask         )
			, m_Blend       ( this, src.m_Blend        )
			, m_VisibleMask ( this, src.m_VisibleMask  )
			, m_Name        ( this, src.m_Name         )
			, m_DialogOpened( this, src.m_DialogOpened )
			, m_CurrentTime ( src.m_CurrentTime )
			, m_Validity    ( src.m_Validity )
		{ }
#pragma warning( pop )
		////////////////////////////////////////////////////////////////////////////////

		// Accessors
		// Name
		NameType Name( )      const { return const_cast<LPTSTR>((LPCTSTR)m_Name); }
		void     Name( const NameType v ) {
			const _tstring tmp(v);
			// Trim the string, removing spaces at the beginning and the end (as well as tabs).
			if (tmp == _T(""))
				m_Name = _T("");
			else
				m_Name = _tstring( v + tmp.find_first_not_of( _T(" \t") ),
				v + tmp.find_last_not_of ( _T(" \t") ) + 1 ).c_str();
		}
		NameType ProcessName( ) const;

		// Texture
		TextureType Texture( )               const { return m_Texture; }
		void        Texture( const TextureType v ) { m_Texture = v;    }

		// Mask
		TextureType Mask( )               const { return m_Mask; }
		void        Mask( const TextureType v ) { m_Mask = v; }

		// Visible
		VisibleType Visible( )               const { return m_Visible; }
		void        Visible( const VisibleType v ) { m_Visible = v;    }
		void        ToggleVisible( )         { m_Visible = !m_Visible; }

		// VisibleMask
		VisibleType VisibleMask( )               const { return m_VisibleMask; }
		void        VisibleMask( const VisibleType v ) { m_VisibleMask = v;    }
		void        ToggleVisibleMask( )         { m_VisibleMask = !m_VisibleMask; }

		// Opacity
		// Here we clamp on read because the value in ParamBlock might not be
		// clamped (when using a Controller or MXS, for example), without our
		// setter being called.
		OpacityType Opacity( ) const {
			return clamp( m_Opacity, Default::OpacityMin, Default::OpacityMax );
		}
		void        Opacity( const OpacityType v ) { m_Opacity = v; }
		float       NormalizedOpacity() const {
			return   float( Opacity() - Default::OpacityMin ) / float(Default::OpacityMax);
		}
		bool        IsOpacityKeyFrame() const {
			return m_Opacity.IsKeyFrame();
		}

		// BlendMode
		// The Blend is not clamped, but instead defaulted to a value.
		BlendType Blend( ) const { return m_Blend; }
		void      Blend( const BlendType v ) {
			m_Blend = (v < 0 || v >= Blending::ModeList.size()) ? Default::BlendMode : v;
		}

		// Dialog Opened
		BlendType DialogOpened( )              const { return m_DialogOpened; }
		void      DialogOpened( const DialogOpenedType v ) { m_DialogOpened = v; }

		// Index
		size_t Index( )    const { return m_Index; }
		void   Index( const size_t v ) { m_Index = v; }

		// ParamBlock
		void          ParamBlock( IParamBlock2 *pblock ) { m_ParamBlock = pblock; }
		IParamBlock2 *ParamBlock( )                const { return m_ParamBlock; }

		// Validators for texture, mask and visibility
		bool HasTexture   () const { return Texture() != NULL; }
		bool HasMask      () const { return Mask() != NULL; }
		bool IsMaskVisible() const { return VisibleMask() && HasMask(); }
		bool IsVisible    () const { return Visible() && Opacity() > 0.0f && HasTexture(); }

		TimeValue CurrentTime(             ) const { return m_CurrentTime; }
		void      CurrentTime( const TimeValue v ) { m_CurrentTime = v; }

		// Common functions
		void swap( Layer& src ) {
			// create no new keys while setting values...
			AnimateSuspend as (TRUE, TRUE);

			std::swap( m_ParamBlock,   src.m_ParamBlock  );
			std::swap( m_Index,        src.m_Index       );
			std::swap( m_CurrentTime,  src.m_CurrentTime );
			std::swap( m_Validity,     src.m_Validity );
			SwapValue( src );
		}

		// Swap only the values of the layers.
		void SwapValue( Layer& src ) {
			m_Visible	  .swap ( src.m_Visible );		m_Visible	  .Layer( this ); src.m_Visible		.Layer( &src );
			m_VisibleMask .swap ( src.m_VisibleMask );	m_VisibleMask .Layer( this ); src.m_VisibleMask .Layer( &src );
			m_Blend		  .swap ( src.m_Blend );		m_Blend		  .Layer( this ); src.m_Blend		.Layer( &src );
			m_Name		  .swap ( src.m_Name );			m_Name		  .Layer( this ); src.m_Name		.Layer( &src );
			m_DialogOpened.swap ( src.m_DialogOpened );	m_DialogOpened.Layer( this ); src.m_DialogOpened.Layer( &src );
			m_Opacity	  .swap ( src.m_Opacity );		m_Opacity	  .Layer( this ); src.m_Opacity		.Layer( &src );
			m_Texture	  .swap ( src.m_Texture );		m_Texture	  .Layer( this ); src.m_Texture		.Layer( &src );
			m_Mask		  .swap ( src.m_Mask );			m_Mask		  .Layer( this ); src.m_Mask		.Layer( &src );
		}

		void CopyValue( const Layer& src ) {
			m_CurrentTime = src.m_CurrentTime;

			m_Blend			= BlendType (		src.m_Blend );
			m_Name          = (LPCTSTR)			src.m_Name;
			m_NameCache     = 					src.m_NameCache;
			m_DialogOpened  = DialogOpenedType( src.m_DialogOpened );

			RemapDir *remap = NewRemapDir();

			if (src.m_Visible.HasController())
				m_Visible.SetController( &src.m_Visible, *remap );
			else
				m_Visible = VisibleType( src.m_Visible );

			if (src.m_VisibleMask.HasController())
				m_VisibleMask.SetController( &src.m_VisibleMask, *remap );
			else
				m_VisibleMask = VisibleType( src.m_VisibleMask );

			if (src.m_Opacity.HasController())
				m_Opacity.SetController( &src.m_Opacity, *remap );
			else
				m_Opacity = OpacityType( src.m_Opacity );

			// Clone the textures.
			HoldSuspend hs;
			Texmap *map = (Texmap *) remap->CloneRef(src.m_Texture);
			Texmap *mask = (Texmap *) remap->CloneRef(src.m_Mask);
			hs.Resume();
			m_Texture = TextureType( map );
			m_Mask =    TextureType( mask );

			remap->DeleteThis();
		}

		// Operators
		const Layer& operator=( const Layer& src ) {
			Layer cpy( src ); swap( cpy ); return *this;
		}
		bool operator==( const Layer& rhv ) const {
			return rhv.m_Index      == m_Index
				&& rhv.m_ParamBlock == m_ParamBlock;
		}

		// Update the Texmaps.
		void Update( TimeValue t, Interval& valid ) {
			CurrentTime( t );
			if (!m_Validity.InInterval(t)) {
				m_Validity.SetInfinite();

				m_Visible		.Update( t, m_Validity );
				m_VisibleMask	.Update( t, m_Validity );
				m_Blend			.Update( t, m_Validity );
				m_Name			.Update( t, m_Validity );
				m_DialogOpened	.Update( t, m_Validity );
				m_Opacity		.Update( t, m_Validity );
				m_Texture		.Update( t, m_Validity );
				m_Mask			.Update( t, m_Validity );

				if (Texture())
					Texture()->Update(t,m_Validity);
				if (Mask())
					Mask()->Update(t,m_Validity);

			}
			valid &= m_Validity;
		}

		void Invalidate() {
			m_Validity.SetEmpty();
		};
	};

	// Return the name with all the special variable replaced by their values.
	// This uses a stringstream to pipe every character except if they correspond
	// to the special tags. If so, it pipes the value instead. Returns the content
	// of the pipe in a cache to ensure the pointer stays valid outside the
	// scope of this function.
	Layer::NameType Layer::ProcessName() const {
		const _tstring   &name  ( m_Name );
		_tostringstream   oss;
		oss << m_Name << GetString(IDS_LAYER) << (Index() + 1);
		m_NameCache = oss.str();
		return Layer::NameType( m_NameCache.c_str() );
	}

#pragma endregion
	/////////////////////////////////////////////////////////////////////////////
#pragma region Layer Dialog

	// A class representing the view of Layer in a Dialog.
	// Does not need to be hooked to a rollup, but needs to have
	// a valid Layer class for parameters.
	// This means that you could pop a Dialog or put the Layer
	// in any window without modifying this class.
	class LayerDialog {
		// Internal class to ease tooltip management for the combo.
		struct ComboToolTipManager {
			ComboToolTipManager( HWND hWnd, int resTextId )
				: m_hComboToolTip( 0 ) {
				// Hack to add the tooltip to a combo box, which doesn't support (yet) a
				// CustControl interface.
				TOOLINFO ti = {0};
				ti.cbSize		= sizeof(TOOLINFO);
				ti.uFlags		= TTF_SUBCLASS | TTF_IDISHWND;
				ti.hwnd			= hWnd;							// Control's parent window
				ti.hinst		= hInstance;
				ti.uId			= (UINT_PTR)hWnd;				// Control's window handle
				ti.lpszText		= GetString( resTextId );

				// Create a ToolTip window if it not yet created
				// This assure that the tooltip window is created only once
				if (!m_hComboToolTip) {
					m_hComboToolTip = CreateWindowEx( NULL,
						TOOLTIPS_CLASS,
						NULL,
						WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						hWnd,
						NULL,
						hInstance,
						NULL );

					DbgAssert(m_hComboToolTip);
				}

				// Before registering a tool with a ToolTip control, we try to
				// delete it to ensure that the tooltip is added only once.
				SendMessage(m_hComboToolTip, TTM_DELTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
				SendMessage(m_hComboToolTip, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
			}

			~ComboToolTipManager() {
				if (m_hComboToolTip) {
					TOOLINFO ti = {0};
					ti.cbSize = sizeof(TOOLINFO);

					// Remove every registered ToolTip with m_hComboToolTip
					while ( SendMessage(m_hComboToolTip, TTM_ENUMTOOLS, 0, LPARAM(&ti)) ) {
							SendMessage(m_hComboToolTip, TTM_DELTOOL, 0, LPARAM(&ti) );

							ZeroMemory(&ti, sizeof(TOOLINFO));
							ti.cbSize = sizeof(TOOLINFO);
					}

					DestroyWindow(m_hComboToolTip);
					m_hComboToolTip = 0;
				}
			}
		private:
			HWND m_hComboToolTip;
		};

	public:
		LayerDialog( Layer* layer, IRollupPanel* panel, HWND handle, DADMgr* dadManager, HIMAGELISTPtr imlButtons )
			: m_Layer            ( layer  )
			, m_RollupPanel      ( panel  )
			, m_hWnd             ( handle )
			, m_TextureImageList ( 0 )
			, m_ButtonImageList  ( imlButtons )
			, m_ComboToolTip     ( new ComboToolTipManager( GetDlgItem( m_hWnd, IDC_COMP_BLEND_TYPE ), IDS_DS_COMP_TOOLTIP_BLEND_MODE ) )
		{
			init( dadManager );
		}

		LayerDialog( const LayerDialog& src )
			: m_Layer            ( src.m_Layer            )
			, m_RollupPanel      ( src.m_RollupPanel      )
			, m_hWnd             ( src.m_hWnd             )
			, m_TextureImageList ( src.m_TextureImageList )
			, m_ButtonImageList  ( src.m_ButtonImageList  )
			, m_ComboToolTip     ( src.m_ComboToolTip )
		{
			UpdateDialog();
		}

		// Accessors
		Layer          *Layer()                  const { return m_Layer; }
		void            Layer( Composite::Layer* v )   { m_Layer = v; }
		IRollupPanel   *Panel()                  const { return m_RollupPanel; }
		const _tstring  Title()                  const {
			if (m_Layer) return m_Layer->ProcessName();
			else return _tstring();
		}

		bool IsTextureButton( HWND handle ) const {
			HWND button ( ::GetDlgItem( m_hWnd, IDC_COMP_TEX  ) );
			return handle == button;
		}
		bool IsMaskButton( HWND handle ) const {
			HWND button( ::GetDlgItem( m_hWnd, IDC_COMP_MASK ) );
			return handle == button;
		}

		void swap( LayerDialog& src ) {
			std::swap( m_Layer,              src.m_Layer             );
			std::swap( m_RollupPanel,        src.m_RollupPanel       );
			std::swap( m_TextureImageList,   src.m_TextureImageList  );
			std::swap( m_ButtonImageList,    src.m_ButtonImageList   );
		}

		bool operator ==( HWND handle ) const {
			return m_hWnd == handle;
		}

		// Update the content of the fields
		void        UpdateDialog()                      const;
		void        UpdateOpacity( bool edit = true )   const;

		void        UpdateTexture( )                    const;
		void        UpdateMask   ( )                    const;

		void        UpdateVisibility( )                 const;
		void        UpdateMaskVisibility   ( )          const;

		void        SetTime(TimeValue t) {
			Layer()->CurrentTime( t );
		}

		void        DisableRemove() {
			ButtonPtr  button( m_hWnd, IDC_COMP_LAYER_REMOVE    );
			button->Disable();
		}
		void        EnableRemove() {
			ButtonPtr  button( m_hWnd, IDC_COMP_LAYER_REMOVE    );
			button->Enable();
		}
		bool        IsRemoveEnabled() {
			ButtonPtr  button( m_hWnd, IDC_COMP_LAYER_REMOVE    );
			return button->IsEnabled() != FALSE;
		}

	private:
		LayerDialog();

		Composite::Layer    *m_Layer;
		IRollupPanel        *m_RollupPanel;
		HWND                 m_hWnd;
		HIMAGELISTPtr        m_TextureImageList;
		HIMAGELISTPtr        m_ButtonImageList;
		SmartPtr<ComboToolTipManager> m_ComboToolTip;

		// operators
		LayerDialog& operator=( const LayerDialog& );
		bool operator ==( const LayerDialog& ) const;

		void init( DADMgr* dragManager ) {
			// Fill the combo box for blending modes.
			initCombo();

			// Set the Texture buttons and image list.
			initButton( dragManager );

			// Set the custom buttons.
			initToolbar();

			// Set the spinner for opacity
			SpinPtr OpacitySpin( m_hWnd, IDC_COMP_OPACITY_SPIN );
			EditPtr OpacityEdit( m_RollupPanel->GetHWnd(), IDC_COMP_OPACITY );
			OpacitySpin->SetLimits ( Default::OpacityMin, Default::OpacityMax  );
			OpacityEdit->WantReturn( TRUE );

			// Set tooltip.
			OpacitySpin->SetTooltip( TRUE, GetString( IDS_DS_COMP_TOOLTIP_OPACITY ) );
			OpacityEdit->SetTooltip( TRUE, GetString( IDS_DS_COMP_TOOLTIP_OPACITY ) );

			// Update the controls.
			UpdateDialog();
		}

		// Initialize the values inside the Combo.
		void initCombo( ) {
			// Set the combo box for Blending modes
			struct predicate : std::binary_function<HWND, const Blending::BaseBlendMode*, void> {
				void operator()( HWND ctrl, const Blending::BaseBlendMode* mode ) const {
					::SendMessage( ctrl, CB_ADDSTRING, 0, LPARAM( mode->Name().c_str() ) );
				};
			} ;

			std::for_each( Blending::ModeList.begin(),
				Blending::ModeList.end(),
				std::bind1st( predicate(),
				GetDlgItem( m_hWnd, IDC_COMP_BLEND_TYPE ) ) );
		}

		// Initialize the mask and texture buttons.
		void initButton( DADMgr* dragManager ) {
			ButtonPtr  tex_button( m_hWnd, IDC_COMP_TEX  );
			ButtonPtr  msk_button( m_hWnd, IDC_COMP_MASK );

			RECT rc;
			GetWindowRect( m_hWnd, &rc );

			int width ( rc.right - rc.left );
			int height( rc.bottom - rc.top );

			if (m_TextureImageList == NULL) {
				m_TextureImageList  = ImageList_Create( width, height, ILC_COLORDDB, 3, 1 );
				// Set the Texture maps.
				SmartHBITMAP tex_bmp ( RenderBitmap( NULL, 0, width, height ) );
				SmartHBITMAP mask_bmp( RenderBitmap( NULL, 0, width, height, BMM_GRAY_8 ) );
				ImageList_Add        ( m_TextureImageList, tex_bmp,  0 );
				ImageList_Add        ( m_TextureImageList, mask_bmp, 0 );
			}

			tex_button->SetDADMgr( dragManager );
			msk_button->SetDADMgr( dragManager );

			tex_button->SetImage ( m_TextureImageList, 0, 0, 0, 0, width, height );
			msk_button->SetImage ( m_TextureImageList, 1, 1, 1, 1, width, height );

			tex_button->SetTooltip( TRUE, GetString( IDS_DS_COMP_TOOLTIP_TEXTURE ) );
			msk_button->SetTooltip( TRUE, GetString( IDS_DS_COMP_TOOLTIP_MASK    ) );
		}

		// Initialize ALL the buttons.
		void initToolbar() {
			using namespace ToolBar;

			ButtonPtr  DeleteBtn       ( m_hWnd, IDC_COMP_LAYER_REMOVE    );
			ButtonPtr  VisibleBtn      ( m_hWnd, IDC_COMP_VISIBLE         );
			ButtonPtr  ColorCorrBtn    ( m_hWnd, IDC_COMP_CC              );
			ButtonPtr  MaskVisibleBtn  ( m_hWnd, IDC_COMP_MASK_VISIBLE    );
			ButtonPtr  MaskColorCorrBtn( m_hWnd, IDC_COMP_MASK_CC         );
			ButtonPtr  RenameBtn       ( m_hWnd, IDC_COMP_LAYER_RENAME    );
			ButtonPtr  DuplicateBtn    ( m_hWnd, IDC_COMP_LAYER_DUPLICATE );

			DeleteBtn      ->SetImage( m_ButtonImageList, RemoveIcon,    RemoveIcon,
				RemoveIconDis, RemoveIconDis,
				IconHeight,    IconWidth );
			VisibleBtn     ->SetImage( m_ButtonImageList, VisibleIconOn, VisibleIconOn,
				VisibleIconOn, VisibleIconOn,
				IconHeight,    IconWidth );
			ColorCorrBtn   ->SetImage( m_ButtonImageList, ColorCorrectionIcon, ColorCorrectionIcon,
				ColorCorrectionIcon, ColorCorrectionIcon,
				IconHeight,    IconWidth );
			MaskVisibleBtn ->SetImage( m_ButtonImageList, VisibleIconOn, VisibleIconOn,
				VisibleIconOn, VisibleIconOn,
				IconHeight,    IconWidth );
			MaskColorCorrBtn->SetImage(m_ButtonImageList, ColorCorrectionIcon, ColorCorrectionIcon,
				ColorCorrectionIcon, ColorCorrectionIcon,
				IconHeight,    IconWidth );
			RenameBtn      ->SetImage( m_ButtonImageList, RenameIcon,    RenameIcon,
				RenameIcon,    RenameIcon,
				IconHeight,    IconWidth );
			DuplicateBtn   ->SetImage( m_ButtonImageList, DuplicateIcon, DuplicateIcon,
				DuplicateIcon, DuplicateIcon,
				IconHeight,    IconWidth );

			DeleteBtn->Execute         ( I_EXEC_CB_NO_BORDER );
			VisibleBtn->Execute        ( I_EXEC_CB_NO_BORDER );
			ColorCorrBtn->Execute      ( I_EXEC_CB_NO_BORDER );
			MaskVisibleBtn->Execute    ( I_EXEC_CB_NO_BORDER );
			MaskColorCorrBtn->Execute  ( I_EXEC_CB_NO_BORDER );
			RenameBtn->Execute         ( I_EXEC_CB_NO_BORDER );
			DuplicateBtn->Execute      ( I_EXEC_CB_NO_BORDER );

			DeleteBtn->Execute         ( I_EXEC_BUTTON_DAD_ENABLE, 0 );
			VisibleBtn->Execute        ( I_EXEC_BUTTON_DAD_ENABLE, 0 );
			ColorCorrBtn->Execute      ( I_EXEC_BUTTON_DAD_ENABLE, 0 );
			MaskVisibleBtn->Execute    ( I_EXEC_BUTTON_DAD_ENABLE, 0 );
			MaskColorCorrBtn->Execute  ( I_EXEC_BUTTON_DAD_ENABLE, 0 );
			RenameBtn->Execute         ( I_EXEC_BUTTON_DAD_ENABLE, 0 );
			DuplicateBtn->Execute      ( I_EXEC_BUTTON_DAD_ENABLE, 0 );

			DeleteBtn->SetTooltip         ( TRUE, GetString( IDS_DS_COMP_TOOLTIP_DELETE ) );
			RenameBtn->SetTooltip         ( TRUE, GetString( IDS_DS_COMP_TOOLTIP_RENAME ) );
			DuplicateBtn->SetTooltip      ( TRUE, GetString( IDS_DS_COMP_TOOLTIP_DUPLICATE ) );
			ColorCorrBtn->SetTooltip      ( TRUE, GetString( IDS_DS_COMP_TOOLTIP_COLOR ) );
			MaskColorCorrBtn->SetTooltip  ( TRUE, GetString( IDS_DS_COMP_TOOLTIP_MASK_COLOR ) );
		}
	};

#pragma region Layer Dialog Implementations

	// Update the texture button (render the texmap and reshow it)
	void LayerDialog::UpdateTexture( ) const {
		ButtonPtr  tex_button( m_hWnd, IDC_COMP_TEX );
		if ( false == tex_button )
			return;

		// No Update when no Texture
		if (m_Layer && !m_Layer->Texture()) {
			tex_button->SetIcon( 0, 0, 0 );
			tex_button->SetText( GetString( IDS_DS_NO_TEXTURE ) );
			// button tool-tip is updated automatically when SetText is called (TexDADMgr::AutoTooltip returns TRUE),
			// We want to force the tooltip to something different.
			tex_button->SetTooltip( TRUE, GetString( IDS_DS_COMP_TOOLTIP_TEXTURE ) );
		} else {
			// Create the image.
			HWND button( tex_button->GetHwnd() );
			RECT rc; ::GetWindowRect( button, &rc );
			int width  = rc.right - rc.left;
			int height = rc.bottom - rc.top;

			SmartHBITMAP tex_bmp  ( RenderBitmap( m_Layer->Texture(),
				m_Layer->CurrentTime(),
				width, height ) );
			ImageList_Replace     ( m_TextureImageList, 0, tex_bmp, 0 );
			tex_button->SetImage  ( m_TextureImageList, 0, 0, 0, 0, width, height );
			// Add the name of Texture to the tooltip.
			_tstring ttip( GetString( IDS_DS_COMP_TOOLTIP_TEXTURE ) );
			ttip += _T(": ");
			ttip += m_Layer->Texture()->GetName();
			tex_button->SetTooltip( TRUE, const_cast<LPTSTR>(ttip.c_str()) );
		}
	}

	// Update the mask button (render the texmap and reshow it).
	void LayerDialog::UpdateMask( ) const {
		ButtonPtr mask_button( m_hWnd, IDC_COMP_MASK );
		if ( false == mask_button )
			return;

		// No Update when no Texture
		if (m_Layer && !m_Layer->Mask()) {
			mask_button->SetIcon( 0, 0, 0 );
			mask_button->SetText( GetString( IDS_DS_NO_MASK ) );
			// button tool-tip is updated automatically when SetText is called (TexDADMgr::AutoTooltip returns TRUE),
			// We want to force the tooltip to something different.
			mask_button->SetTooltip( TRUE, GetString( IDS_DS_COMP_TOOLTIP_MASK ) );
		} else {
			// Create the image.
			HWND button( mask_button->GetHwnd() );
			RECT rc; ::GetWindowRect( button, &rc );
			int width  = rc.right - rc.left;
			int height = rc.bottom - rc.top;

			SmartHBITMAP mask_bmp( RenderBitmap( m_Layer->Mask(),
				m_Layer->CurrentTime(),
				width, height,
				BMM_GRAY_16 ) );
			ImageList_Replace    ( m_TextureImageList, 1, mask_bmp, 0 );
			mask_button->SetImage( m_TextureImageList, 1, 1, 1, 1, width, height );
			// Add the name of Texture to the tooltip.
			_tstring ttip( GetString( IDS_DS_COMP_TOOLTIP_MASK ) );
			ttip += _T(": ");
			ttip += m_Layer->Mask()->GetName();
			mask_button->SetTooltip( TRUE, const_cast<LPTSTR>(ttip.c_str()) );
		}
	}

	// Update the opacity edit and spinner.
	void LayerDialog::UpdateOpacity( bool edit ) const {
		// Set the edit boxes
		EditPtr Opacity(m_RollupPanel->GetHWnd(), IDC_COMP_OPACITY     );
		SpinPtr spin   (m_RollupPanel->GetHWnd(), IDC_COMP_OPACITY_SPIN);
		if (m_Layer && Opacity && spin) {
			if (edit) {
				Opacity->SetText ( m_Layer->Opacity() );
			}

			spin   ->SetValue( m_Layer->Opacity(), FALSE );
			// Set the red border if spin is on key frame.
			spin   ->SetKeyBrackets( Layer()->IsOpacityKeyFrame() );
		}
	}

	// Update the texture visibility button.
	void LayerDialog::UpdateVisibility( ) const {
		using namespace ToolBar;
		ButtonPtr VisibleBtn( m_RollupPanel->GetHWnd(), IDC_COMP_VISIBLE );
		if (m_Layer && VisibleBtn) {
			if (m_Layer->Visible()) {
				VisibleBtn->SetImage( m_ButtonImageList,  VisibleIconOn,  VisibleIconOn,
					VisibleIconOn,  VisibleIconOn,
					IconWidth, IconHeight );
				VisibleBtn->SetTooltip( TRUE, GetString( IDS_DS_COMP_TOOLTIP_VISIBLE ) );
			} else {
				VisibleBtn->SetImage( m_ButtonImageList,  VisibleIconOff, VisibleIconOff,
					VisibleIconOff, VisibleIconOff,
					IconWidth, IconHeight );
				VisibleBtn->SetTooltip( TRUE, GetString( IDS_DS_COMP_TOOLTIP_HIDDEN ) );
			}
		}
	}

	// Update the mask visibility button.
	void LayerDialog::UpdateMaskVisibility( ) const {
		using namespace ToolBar;
		ButtonPtr MaskVisibleBtn( m_RollupPanel->GetHWnd(), IDC_COMP_MASK_VISIBLE );
		if (m_Layer && m_Layer->VisibleMask()) {
			MaskVisibleBtn->SetImage( m_ButtonImageList,  VisibleIconOn,  VisibleIconOn,
				VisibleIconOn,  VisibleIconOn,
				IconWidth, IconHeight );
			MaskVisibleBtn->SetTooltip( TRUE, GetString( IDS_DS_COMP_TOOLTIP_MASK_VISIBLE ) );
		} else {
			MaskVisibleBtn->SetImage( m_ButtonImageList, VisibleIconOff, VisibleIconOff,
				VisibleIconOff, VisibleIconOff,
				IconWidth, IconHeight );
			MaskVisibleBtn->SetTooltip( TRUE, GetString( IDS_DS_COMP_TOOLTIP_MASK_HIDDEN ) );
		}
	}

	// Updates all the fields in the dialog.
	void LayerDialog::UpdateDialog()  const {
		if (m_Layer == NULL)
			return;

		// Set the visibility buttonz
		UpdateVisibility();
		UpdateMaskVisibility();

		HWND combo( ::GetDlgItem( m_RollupPanel->GetHWnd(), IDC_COMP_BLEND_TYPE ) );
		::SendMessage( combo, CB_SETCURSEL, m_Layer->Blend(), 0 );

		UpdateOpacity( );
		if (m_Layer) {
			UpdateTexture( );
			UpdateMask   ( );
		}
	}
#pragma endregion

#pragma endregion
	/////////////////////////////////////////////////////////////////////////////
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Parameter Block
namespace Composite {
	namespace {
		class AccessorClass : public PBAccessor
		{
		public:
			void TabChanged( tab_changes     changeCode,
				Tab<PB2Value>  *tab,
				ReferenceMaker *owner,
				ParamID         id,
				int             tabIndex,
				int             count );
		};

		AccessorClass Accessor;
	}
}

// Parameter block (module private)
namespace Composite {
	namespace {
		ParamBlockDesc2 ParamBlock (
				BlockParam::ID, _T("parameters"),              // ID, int_name
			                    0,                             // local_name
			                    GetCompositeDesc(),            // ClassDesc
			                    P_AUTO_CONSTRUCT | P_VERSION,  // flags
			                    3,                             // version
			                    0,                             // construct_id

      //////////////////////////////////////////////////////////////////////////
      // params
      // For compatibility while loading with the old Composite map, the two
      // first parameters have the same Name and are in the same order as before.
      BlockParam::Texture_DEPRECATED,    _T(""),          // ID, int_name
         TYPE_TEXMAP_TAB,     0,                      // Type, tab_size
         P_VARIABLE_SIZE | P_OWNERS_REF | P_OBSOLETE,              // flags
         IDS_DS_TEXMAPS,                              // localized Name
         p_accessor,         &Accessor,               // params
         p_refno,             1,
      end,
      BlockParam::Visible,    _T("mapEnabled"),
         TYPE_BOOL_TAB,       1,
         P_VARIABLE_SIZE | P_ANIMATABLE,
         IDS_JW_MAP1ENABLE,
         p_default,           Default::Visible,
         p_accessor,         &Accessor,
      end,
      BlockParam::Mask_DEPRECATED,       _T(""),
         TYPE_TEXMAP_TAB,     0,
         P_VARIABLE_SIZE | P_OWNERS_REF  | P_OBSOLETE,
         IDS_JW_MASKMAP,
         p_accessor,         &Accessor,
         p_refno,             1,
      end,
      BlockParam::VisibleMask,_T("maskEnabled"),
         TYPE_BOOL_TAB,       1,
         P_VARIABLE_SIZE | P_ANIMATABLE,
         IDS_JW_MAP2ENABLE,
         p_default,           Default::VisibleMask,
         p_accessor,         &Accessor,
      end,
      BlockParam::Opacity_DEPRECATED,    _T(""),
         TYPE_INT_TAB,        0,
         P_VARIABLE_SIZE | P_ANIMATABLE | P_OBSOLETE,
         0,
      end,
      BlockParam::BlendMode,  _T("blendMode"),
         TYPE_INT_TAB,        1,
         P_VARIABLE_SIZE,
         IDS_JW_MAP_BLENDMODE,
         p_default,           Default::BlendMode,
         p_range,             0, Blending::ModeList.size(),
         p_accessor,          &Accessor,
      end,
      BlockParam::Name,       _T("layerName"),
         TYPE_STRING_TAB,     1,
         P_VARIABLE_SIZE,
         IDS_JW_NAME,
		 p_default,          Default::Name.c_str(),
        p_accessor,          &Accessor,
      end,
      BlockParam::DialogOpened, _T("dlgOpened"),
         TYPE_BOOL_TAB,       1,
         P_VARIABLE_SIZE,
         IDS_JW_DIALOG_OPENED,
         p_default,           Default::DialogOpened,
         p_accessor,          &Accessor,
      end,
	  BlockParam::Opacity,    _T("opacity"),
         TYPE_FLOAT_TAB,        1,
         P_VARIABLE_SIZE | P_ANIMATABLE,
         IDS_JW_MAPOPACITY,
         p_default,           Default::Opacity,
         p_range,             Default::OpacityMin,
							  Default::OpacityMax,
         p_accessor,          &Accessor,
	  end,

	  BlockParam::Texture,    _T("mapList"),          // ID, int_name
         TYPE_TEXMAP_TAB,     1,                      // Type, tab_size
         P_VARIABLE_SIZE | P_SUBANIM,              // flags
         IDS_DS_TEXMAPS,                              // localized Name
         p_accessor,         &Accessor,               // params
	  end,
	  BlockParam::Mask,       _T("mask"),
         TYPE_TEXMAP_TAB,     1,
         P_VARIABLE_SIZE | P_SUBANIM ,
         IDS_JW_MASKMAP,
         p_accessor,         &Accessor,
	  end,

   end );
}
}
#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Function Publishing

namespace Composite {

   FPInterfaceDesc InterfaceDesc(
      COMPOSITE_INTERFACE,             _T("layers"),        // ID, int_name
                                       0,                   // local_name
                                       GetCompositeDesc(),  // ClassDesc
                                       FP_MIXIN,            // flags

		Function::CountLayers, _T("count"), 0, TYPE_INT, 0, 0, // ID, int_name, local_name, return type, flags, num args.
		Function::AddLayer, _T("add"), 0, TYPE_VOID, 0, 0, 
		Function::DeleteLayer, _T("delete"), 0, TYPE_VOID, 0, 1, 
			_T("index"), 0, TYPE_INDEX,
		Function::DuplicateLayer, _T("duplicate"), 0, TYPE_VOID, 0, 1, 
			_T("index"), 0, TYPE_INDEX,
		Function::MoveLayer, _T("move"), 0, TYPE_VOID, 0, 3, 
			_T("from index"), 0, TYPE_INDEX,
			_T("to index"),   0, TYPE_INDEX,
			_T("before"),     0, TYPE_BOOL, f_keyArgDefault, false,
      end
   );

   FPInterfaceDesc* Interface::GetDesc() {
      return &InterfaceDesc;
   }
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Composite Dialog

namespace Composite {
	class Texture;

	class Dialog : public ParamDlg {
	public:
		typedef LayerDialog                    value_type;
		typedef std::list<value_type>          list_type;
		typedef list_type::iterator            iterator;
		typedef list_type::const_iterator      const_iterator;
		typedef list_type::reverse_iterator    reverse_iterator;

		// Public STD Sequence methods
		iterator          begin()        { return m_DialogList.begin();  }
		iterator          end()          { return m_DialogList.end();    }
		const_iterator    begin()  const { return m_DialogList.begin();  }
		const_iterator    end()    const { return m_DialogList.end();    }
		reverse_iterator  rbegin()       { return m_DialogList.rbegin(); }
		reverse_iterator  rend()         { return m_DialogList.rend();   }
		size_t            size()         { return m_DialogList.size();   }
		void              push_back ( const value_type& value ) {
			m_DialogList.push_back( value ); }
		void              push_front( const value_type& value ) {
			m_DialogList.push_front( value ); }
		void              erase     ( iterator it ) {
			m_DialogList.erase( it ); }
		void              insert ( iterator where, const value_type& value ) {
			m_DialogList.insert( where, value ); }

		Dialog(HWND editor_wnd, IMtlParams *imp, Texture *m);
		~Dialog();

		// Find a layerdialog by its HWND.
		iterator find( HWND handle ) {
			struct {
				HWND m_hWnd;
				bool operator()( LayerDialog& d ) { return d == m_hWnd; }
			} predicate = {handle};
			if (!handle)
				return end();
			else
				return find_if( begin(), end(), predicate );
		}
		// Find a layerdialog by its layer.
		iterator find( Layer* l ) {
			struct {
				Layer *m_Layer;
				bool operator()( LayerDialog& d ) { return *d.Layer() == *m_Layer; }
			} predicate = {l};
			if (!l)
				return end();
			else
				return find_if( begin(), end(), predicate );
		}
		//find the layer by its index
		iterator find( int index ) {
			struct {
				int m_Index;
				bool operator()( LayerDialog& d ) { return d.Layer()->Index() == m_Index; }
			} predicate = {index};
			if (index < 0)
				return end();
			else
				return find_if( begin(), end(), predicate );
		}

		// Public methods
		// Create a new rollup for a layer. If the corresponding dialoglayer already exists
		// does nothing.
		void                  CreateRollup  ( Layer* l );
		// Delete the rollup for the layer, if it exists.
		void                  DeleteRollup  ( Layer* l );
		// Rename the layer (using the dialog box).
		void                  RenameLayer   ( LayerDialog* l );
		// Rewire all rollups to conform to m_Texture internal list of layers, adding
		// and deleting rollups if necessary.
		void                  Rewire        ( );

		Texture*              GetTexture    ( ) { return m_Texture; }

		// ParamDlg virtuals
		INT_PTR               PanelProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
		void                  ReloadDialog();
		void                  UpdateMtlDisplay();
		void                  ActivateDlg(BOOL onOff) {}

		void                  Invalidate() {
			m_Redraw = true;
			::InvalidateRect( m_EditorWnd, 0, FALSE );
		}
		void                  Destroy(HWND hWnd) {}

		// methods inherited from ParamDlg:
		Class_ID              ClassID() { return GetCompositeDesc()->ClassID(); }
		void                  SetThing(ReferenceTarget *m);
		ReferenceTarget*      GetThing();
		void                  DeleteThis() { delete this;  }
		void                  SetTime(TimeValue t);
		int                   FindSubTexFromHWND(HWND handle);

		void                  EnableDisableInterface();

	private:
		// Invalid calls.
		template< typename T > void operator=(T);
		template< typename T > void operator=(T) const;
		Dialog();
		Dialog(const Dialog&);

		static INT_PTR CALLBACK PanelDlgProc(HWND hWnd, UINT msg,
			WPARAM wParam,
			LPARAM lParam);

		bool						m_Redraw;
		bool						m_Reloading;

		// Texture Editor properties.
		IMtlParams					*m_MtlParams;
		HWND						m_EditorWnd;

		// Rollup Window for Layer list
		RollupWindowPtr				m_LayerRollupWnd;
		HWND						m_DialogWindow;

		// Drag&Drop Texture manager.
		TexDADMgr					m_DragManager;

		// The Composite Texture being edited.
		Texture						*m_Texture;

		list_type					m_DialogList;
		unsigned int				m_CategoryCounter;

		HIMAGELISTPtr				m_ButtonImageList;

		SmartPtr<IRollupCallback>	m_RollupCallback;

		TSTR						m_oldOpacityVal;

		// Private methods
		void OnOpacitySpinChanged( HWND handle, iterator LayerDialog );
		void OnOpacityEditChanged( HWND handle );
		void OnOpacityFocusLost  ( HWND handle );

		// Private utility methods
		void StartChange() { theHold.Begin(); }
		void StopChange( int resource_id ) { theHold.Accept( ::GetString( resource_id ) ); }
		void CancelChange() { theHold.Cancel(); }
	};
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Restore Objects

namespace Composite {
	class PopLayerBeforeRestoreObject: public RestoreObj {
		Texture			*m_owner;
		IParamBlock2	*m_paramblock;
		size_t			m_layer_index;
		PopLayerBeforeRestoreObject() { }
	public:
		PopLayerBeforeRestoreObject(Texture *owner, IParamBlock2 *paramblock, size_t layer_index);
		~PopLayerBeforeRestoreObject() { }
		void Restore(int isUndo);
		void Redo();
		TSTR Description() {
			return(TSTR(_T("CompositeTexmapPopLayerBeforeRestore")));
		}
	};

	class PopLayerAfterRestoreObject: public RestoreObj {
		Texture			*m_owner;
		IParamBlock2	*m_paramblock;
		size_t			m_layer_index;
		PopLayerAfterRestoreObject() { }
	public:
		PopLayerAfterRestoreObject(Texture *owner, IParamBlock2 *paramblock, size_t layer_index);
		~PopLayerAfterRestoreObject() { }
		void Restore(int isUndo);
		void Redo();
		TSTR Description() {
			return(TSTR(_T("CompositeTexmapPopLayerAfterRestore")));
		}
	};

	class PushLayerRestoreObject: public RestoreObj {
		Texture			*m_owner;
		IParamBlock2	*m_paramblock;
		bool			m_setName;
		PushLayerRestoreObject() { }
	public:
		PushLayerRestoreObject(Texture *owner, IParamBlock2 *paramblock, bool setName);
		~PushLayerRestoreObject(){}
		void Restore(int isUndo);
		void Redo();
		TSTR Description() {
			return(TSTR(_T("CompositeTexmapPushLayerRestore")));
		}
	};

	class InsertLayerBeforeRestoreObject: public RestoreObj {
		Texture			*m_owner;
		IParamBlock2	*m_paramblock;
		size_t			m_layer_index;
		InsertLayerBeforeRestoreObject() { }
	public:
		InsertLayerBeforeRestoreObject(Texture *owner, IParamBlock2 *paramblock, size_t layer_index);
		~InsertLayerBeforeRestoreObject() { }
		void Restore(int isUndo);
		void Redo();
		TSTR Description() {
			return(TSTR(_T("CompositeTexmapInsertLayerBeforeRestore")));
		}
	};

	class InsertLayerAfterRestoreObject: public RestoreObj {
		Texture			*m_owner;
		IParamBlock2	*m_paramblock;
		size_t			m_layer_index;
		InsertLayerAfterRestoreObject() { }
	public:
		InsertLayerAfterRestoreObject(Texture *owner, IParamBlock2 *paramblock, size_t layer_index);
		~InsertLayerAfterRestoreObject() { }
		void Restore(int isUndo);
		void Redo();
		TSTR Description() {
			return(TSTR(_T("CompositeTexmapInsertLayerAfterRestore")));
		}
	};

}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Composite Texture
namespace Composite {

	class Texture : public MultiTex, public Composite::Interface {
	public:
		// Public Sequence Types
		typedef Layer                       value_type;
		typedef std::list<value_type>       list_type;
		typedef list_type::iterator         iterator;
		typedef list_type::const_iterator   const_iterator;
		typedef list_type::reverse_iterator reverse_iterator;
		typedef list_type::difference_type  difference_type;

		// Public Sequence methods
		iterator          begin()        { return m_LayerList.begin();  }
		iterator          end()          { return m_LayerList.end();    }
		const_iterator    begin()  const { return m_LayerList.begin();  }
		const_iterator    end()    const { return m_LayerList.end();    }
		reverse_iterator  rbegin()       { return m_LayerList.rbegin(); }
		reverse_iterator  rend()         { return m_LayerList.rend();   }
		size_t            size()   const { return m_LayerList.size();   }

		void              push_back( const value_type& value ) {
			m_LayerList.push_back( value ); }
		// does not drop refs
		void              erase    ( iterator it ) { m_LayerList.erase( it ); }
		void              insert   ( iterator where, const value_type& value ) { 
			m_LayerList.insert( where, value ); }

		//////////////////////////////////////////////////////////////////////////
		// Utility Functions
		// There's a lot here. Mainly, if you want to change the reference index
		// system, you would just change the following functions.
		size_t nb_layer  (            ) const throw() { return size(); }
		size_t nb_texture(            ) const throw() { return nb_layer() * 2; }

		// Return true if the index is the one of a texture or a mask.
		bool   is_mask   ( size_t idx ) const throw() { return idx >= nb_layer(); }
		bool   is_tex    ( size_t idx ) const throw() { return idx <  nb_layer(); }

		// Is it a valid texture index?
		bool   is_valid  ( size_t idx ) const throw() {
			return is_mask(idx) ? (mask_idx_to_lyr(idx)< nb_layer())
								: (tex_idx_to_lyr(idx) < nb_layer());
		}

		// Return the index of a texture/mask from the index of a layer
		size_t lyr_to_tex_idx  ( size_t lyr ) const throw() { return lyr; }
		size_t lyr_to_mask_id  ( size_t lyr ) const throw() { return lyr + nb_layer(); }

		// Return the index of a layer from the index of a texture/mask.
		size_t tex_idx_to_lyr ( size_t idx ) const throw() { return idx; }
		size_t mask_idx_to_lyr( size_t idx ) const throw() { return tex_idx_to_lyr( idx - nb_layer() ); }

		// Return the index of a layer from an index (generic)
		size_t idx_to_lyr ( size_t idx ) const throw() {
			return is_mask(idx) ? mask_idx_to_lyr(idx)
								: tex_idx_to_lyr (idx); }

		// Return true if the index is the one of a texture or a mask.
		bool   is_subtexmap_idx_mask   ( size_t idx ) const throw() { return ((idx%2) == 1); }
		bool   is_subtexmap_idx_tex    ( size_t idx ) const throw() { return !is_subtexmap_idx_mask(idx); }

		// Is it a valid SubTexmap index?
		bool   is_subtexmap_idx_valid  ( size_t idx ) const throw() {
			return idx < (nb_layer()*2);
		}

		// Return the index of a layer from a SubTexmap index
		size_t subtexmap_idx_to_lyr ( size_t idx ) const throw() {
			return idx/2; }

		// Get an iterator to an indice. This is to prevent problems if changing the
		// type of iterator to a BidirectionalIterator instead of a RandomIterator.
		iterator LayerIterator( size_t lyr ) throw() {
			iterator ret( begin() );
			std::advance( ret, lyr );
			return ret;
		}
		const_iterator LayerIterator( size_t lyr ) const throw() {
			const_iterator ret( begin() );
			std::advance( ret, lyr );
			return ret;
		}

		// Return the number of Visible layers
		size_t CountVisibleLayer() const throw() {
			struct {
				bool operator()( const value_type& l ) const {
					return l.IsVisible(); }
			} const predicate;
			return std::count_if( begin(), end(), predicate );
		}

		// Return the number of masks
		size_t CountMaskedLayer() const throw() {
			struct {
				bool operator()( const value_type& l ) const {
					return l.IsMaskVisible(); }
			} const predicate;
			return std::count_if( begin(), end(), predicate );
		}

		//////////////////////////////////////////////////////////////////////////
		// ctor & dtor
		Texture(BOOL loading = FALSE);
		// No need for a destructor since everything is composited or managed
		// by smart pointers.
		//~Texture();

		// Accessors
		_tstring      DefaultLayerName( )       const { return m_DefaultLayerName; }
		Dialog*       ParamDialog( )            const { return m_ParamDialog; }
		void          ParamDialog(Dialog* v)          { m_ParamDialog = v; }

		// MultiTex virtuals
		void           SetNumSubTexmaps(int n)        { SetNumMaps(n); }

		// MtlBase virtuals
		ParamDlg*      CreateParamDlg(HWND editor_wnd, IMtlParams *imp);

		// ISubMap virtuals
		void           SetSubTexmap(int i, Texmap *m);
		int            NumSubTexmaps() { return int(nb_texture()); }
		Texmap*        GetSubTexmap(int i);
		TSTR           GetSubTexmapSlotName(int i);

		// Animatable virtuals
		int            SubNumToRefNum( int subNum ) { return subNum ; }
		int            NumSubs( ) { return 1;}
		Animatable*    SubAnim(int i) {
			if (i == 0)
				return m_ParamBlock;
			if (m_FileVersion != kMax2009)
			{
				DbgAssert(0);  //I dont think we should get here
				return NULL;
			}
			return NULL;
		}
		TSTR SubAnimName(int i) {
				return TSTR(GetString(IDS_DS_PARAMETERS));
		}

		// Texmap virtuals
		AColor         EvalColor(ShadeContext& sc);
		Point3         EvalNormalPerturb(ShadeContext& sc);
		bool           IsLocalOutputMeaningful( ShadeContext& sc );

		// Implementing functions for MAXScript.
		BaseInterface* GetInterface(Interface_ID id) {
			if (id == COMPOSITE_INTERFACE)
				return (Interface*)this;
			else
				return MultiTex::GetInterface(id);
		}

		void MXS_AddLayer() {
			if (nb_layer() >= Param::LayerMax)
				throw MAXException(GetString( IDS_ERROR_MAXLAYER ));
			AddLayer();
		}

		int MXS_Count() {
			return (int)nb_layer();
		}

		void MXS_DeleteLayer( int index ) {
			if (index < 0 || index >= nb_layer())
				throw MAXException(GetString( IDS_ERROR_LAYER_INDEX ));
			if (nb_layer() == 1)
				throw MAXException(GetString( IDS_ERROR_MINLAYER ));

			DeleteLayer( size_t( index ) );
		}

		void MXS_DuplicateLayer( int index ) {
			if (nb_layer() >= Param::LayerMax)
				throw MAXException(GetString( IDS_ERROR_MAXLAYER ));
			if (index < 0 || index >= nb_layer())
				throw MAXException(GetString( IDS_ERROR_LAYER_INDEX ));
			DuplicateLayer( size_t( index ) );
		}

		void MXS_MoveLayer ( int from_index, int to_index, bool before ) {
			if (from_index < 0 || from_index > nb_layer())
				throw MAXException(GetString( IDS_ERROR_LAYER_INDEX ));
			if (to_index < 0 || to_index >= nb_layer())
				throw MAXException(GetString( IDS_ERROR_LAYER_INDEX ));
			MoveLayer( size_t( from_index ), size_t( to_index ), !before );
		}

		// Public functions
		void AddLayer( );
		void DeleteLayer( size_t Index = 0 );
		void InsertLayer( size_t Index = 0 );

		void MoveLayer( size_t Index, size_t dest, bool before );
		void DuplicateLayer( size_t Index );

		void Update(TimeValue t, Interval& valid);
		void Init();
		void Reset();
		Interval GetValidity() { return m_Validity; }
		Interval Validity(TimeValue t) {
			Interval v;
			Update(t,v);
			return m_Validity;
		}
		void NotifyChanged();
		void SetNumMaps( int n,  bool setName = true );

		void UpdateParamBlock   ( size_t count );
		void OnParamBlockChanged( ParamID id );
		void ColorCorrection    ( Composite::Layer* layer, bool mask );

		void ValidateParamBlock();

		// Class management
		Class_ID    ClassID()             { return GetCompositeDesc()->ClassID();      }
		SClass_ID   SuperClassID()        { return GetCompositeDesc()->SuperClassID(); }
		void        GetClassName(TSTR& s) { s = GetCompositeDesc()->ClassName();       }
		void        DeleteThis()          { delete this;                               }

		// From ReferenceMaker
		int             NumRefs();
		RefTargetHandle GetReference( int i );
		void            SetReference( int i, RefTargetHandle v );
		RefTargetHandle Clone( RemapDir &remap );
		RefResult       NotifyRefChanged( Interval        changeInt,
			RefTargetHandle hTarget,
			PartID         &partID,
			RefMessage      message );

		// IO
		IOResult Save(ISave *);
		IOResult Load(ILoad *);

		// return number of ParamBlocks in this instance
		int           NumParamBlocks   (     ) { return 1; }
		IParamBlock2 *GetParamBlock    (int i) { return m_ParamBlock; }

		// return id'd ParamBlock
		IParamBlock2 *GetParamBlockByID(BlockID id) {
			return	(m_ParamBlock && m_ParamBlock->ID() == id)
					? m_ParamBlock
					: NULL;
		}

		// Multiple map in vp support -- DS 5/4/00
		BOOL SupportTexDisplay()                 { return TRUE; }
		BOOL SupportsMultiMapsInViewport()       { return TRUE; }

		void ActivateTexDisplay(BOOL onoff);
		void SetupGfxMultiMaps(TimeValue t, ::Material *material, MtlMakerCallback &cb);

		//this is used to load old file
		//2008 files just get remapped ok
		//	the maps gets mapped into the owner ref ID of the param block and there are no masks
		//JohnsonBeta files up to J049 are hosed up and need to have major reference reworked
		//	the maps still go into the owner ref of the param block
		//	mask get moved from owner ref to paramblock controlled
		//this flag is set on load when a johnson beta file is loaded so we can get all the references
		//then it is unset in the post load call back after all the mask have been moved into the correct
		//paramblock
		enum FileVersion {
			kMax2008 = 0,
			kJohnsonBeta,
			kMax2009
		};
		FileVersion	m_FileVersion;
		//this is the list of owner ref maps we load from older files
		int		    m_OldMapCount_2008;
		int		    m_OldMapCount_JohnsonBeta;

		//on load if we are loading a 2008 or JohnsonBeta file all the maps and masks will end up 
		//here and in the post load call back we will move them into the paramblock
		Tab<Texmap*>  m_oldMapList;


	private:
		// Invalid calls.
		template< typename T > void operator=(T);
		template< typename T > void operator=(T) const;
		Texture(const Texture&);

		// Member variables
		IParamBlock2         *m_ParamBlock;
		Interval              m_Validity;

		list_type             m_LayerList;
		Dialog               *m_ParamDialog;

		// Saved members
		_tstring              m_DefaultLayerName;

		// Guards
		bool                  m_Updating;
		bool                  m_SettingNumMaps;

		// Pushing and removing layers from the vector (not the param block)
		void PushLayer( bool setName );
		void PopLayer ( size_t Index = 0 );

		/////////////////////////////////////////////////////////////////////////
		// For the interactive renderer
		struct viewport_texture {
			viewport_texture( TexHandlePtr h, iterator l, bool mask = false )
				: m_hWnd ( h )
				, m_Layer( l )
				, m_Mask ( mask )
			{}

			TexHandlePtr  handle() { return m_hWnd;  }
			iterator      Layer()  { return m_Layer; }
			bool          IsMask() { return m_Mask;  }

		private:
			TexHandlePtr  m_hWnd;  // The handle to the Texture
			bool          m_Mask;  // The mask

			// Yes, adding or removing a layer to/from the list is going to
			// invalidate this iterator, but it will trigger a REFMSG_CHANGE
			// (if using the methods) that will destroy the reference.
			iterator      m_Layer; // The Layer shown
		};

		typedef std::vector< viewport_texture > ViewportTextureListType;

		ViewportTextureListType m_ViewportTextureList;
		Interval                m_ViewportIval;

		void InitMultimap      ( TimeValue  time,
								 MtlMakerCallback &callback );
		void PrepareHWMultimap ( TimeValue  time,
								 IHardwareMaterial *hw_material,
								 ::Material *material,
								 MtlMakerCallback &callback  );
		void PrepareSWMultimap ( TimeValue  time,
								 ::Material *material,
								 MtlMakerCallback &callback  );
	};
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Class Descriptors

void*          Composite::Desc::Create(BOOL loading)   { return new Composite::Texture(loading); }
int            Composite::Desc::IsPublic()     { return GetAppID() != kAPP_VIZR; }
SClass_ID      Composite::Desc::SuperClassID() { return TEXMAP_CLASS_ID; }
Class_ID       Composite::Desc::ClassID()      { return Class_ID( COMPOSITE_CLASS_ID, 0 ); }
HINSTANCE      Composite::Desc::HInstance()    { return hInstance; }
const TCHAR  * Composite::Desc::ClassName()    { return GetString( IDS_RB_COMPOSITE_CDESC ); }
const TCHAR  * Composite::Desc::Category()     { return TEXMAP_CAT_COMP; }
const TCHAR  * Composite::Desc::InternalName() { return _T("CompositeMap"); }

ClassDesc2* GetCompositeDesc() {
	static Composite::Desc ClassDescInstance;
	return &ClassDescInstance;
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Accessors

namespace Composite {

	void AccessorClass::TabChanged( tab_changes     changeCode,
		Tab<PB2Value>  *tab,
		ReferenceMaker *owner,
		ParamID         id,
		int             tabIndex,
		int             count ) {
			if ( (owner != NULL) &&
				 (id != BlockParam::Texture_DEPRECATED) &&
				 (id != BlockParam::Mask_DEPRECATED) &&
				 (id != BlockParam::Opacity_DEPRECATED) )
				static_cast<Texture*>(owner)->OnParamBlockChanged( id );
	}
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Composite Dialog Method Implementation

INT_PTR Composite::Dialog::PanelDlgProc(HWND hWnd, UINT msg,
										WPARAM wParam,
										LPARAM lParam)
{
	Dialog *dlg = DLGetWindowLongPtr<Dialog*>( hWnd );
	if (msg == WM_INITDIALOG && dlg == 0) {
		dlg = (Dialog*)lParam;
		DLSetWindowLongPtr( hWnd, lParam );
	}

	if (dlg)
		return dlg->PanelProc( hWnd, msg, wParam, lParam );
	else
		return FALSE;
}

int Composite::Dialog::FindSubTexFromHWND(HWND handle) {
	struct {
		HWND m_hWnd;
		bool operator()( LayerDialog& d ) {
			return d.IsTextureButton( m_hWnd ) || d.IsMaskButton( m_hWnd ); }
	} predicate = {handle};

	iterator it(find_if( begin(), end(), predicate ));
	if (it == end())
		return int(-1);

	if (it->IsTextureButton( handle ))
		return int(it->Layer()->Index() * 2 );
	else
		return int(it->Layer()->Index() * 2 + 1);
}

void Composite::Dialog::OnOpacitySpinChanged( HWND handle, iterator LayerDialog ) {
	SpinPtr spin( handle, IDC_COMP_OPACITY_SPIN );
	LayerDialog->Layer()->Opacity( spin->GetFVal() );
	LayerDialog->UpdateOpacity( );
	m_Texture->NotifyChanged();
}

// When the opacity edit box change, update the layer if it was a return.
void Composite::Dialog::OnOpacityEditChanged( HWND handle ) {
	EditPtr edit( handle, IDC_COMP_OPACITY );
	if (edit->GotReturn()) {
		TSTR newOpacityVal;
		if ( m_oldOpacityVal == newOpacityVal )
			return;
		OnOpacityFocusLost( handle );
		edit->GetText(m_oldOpacityVal);
	}
}

// Update the opacity when edit lost focus
void Composite::Dialog::OnOpacityFocusLost( HWND handle ) {
	EditPtr edit( handle, IDC_COMP_OPACITY );
	iterator it ( find( handle ) );

	DbgAssert( it != end() );
	if (it == end())
		return;

	BOOL valid  ( TRUE );
	float  Opacity( edit->GetFloat( &valid ) );
	// Update the opacity if it's valid.
	if (valid)
		it->Layer()->Opacity( Opacity );

	it->UpdateOpacity( );
	m_Texture->NotifyChanged();
}

namespace {
	// Register a drag'n'drop callback to call the MoveLayer function when dragging.
	struct RollupCallback : public IRollupCallback {
		RollupCallback( Composite::Dialog& c ) : m_Dialog(c) {}
		// On drop, if we find both destination and source in our layers, we
		// move the layer
		BOOL HandleDrop( IRollupPanel *src, IRollupPanel *dst, bool before ) {
			Composite::Dialog::iterator src_it( m_Dialog.find( src->GetHWnd() ) );
			Composite::Dialog::iterator dst_it( m_Dialog.find( dst->GetHWnd() ) );

			if( src_it != m_Dialog.end() && dst_it != m_Dialog.end() && src_it != dst_it) {
				size_t s( src_it->Layer()->Index() );
				size_t d( dst_it->Layer()->Index() );
				theHold.Begin();
				m_Dialog.GetTexture()->MoveLayer( s, d, before );
				theHold.Accept( GetString(IDS_DS_UNDO_MOVE) );
				GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
			}

			// Never let the system handle this, since we need to completely refresh
			// the interface and we re-hooked a lot of things...
			return TRUE;
		}

	private:
		Composite::Dialog& m_Dialog;
	};
}

Composite::Dialog::Dialog(HWND editor_wnd, IMtlParams *mtl_param, Texture *comp)
							: m_MtlParams        ( mtl_param       )
							, m_Texture          ( comp            )
							, m_Redraw           ( false           )
							, m_EditorWnd        ( editor_wnd      )
							, m_CategoryCounter  ( Param::CategoryCounterMax )
							, m_Reloading        ( false           )
							, m_ButtonImageList  ( ImageList_Create( 13, 12, ILC_MASK, 0, 1 ) )
{
	m_DragManager.Init( this );
	m_DialogWindow = m_MtlParams->AddRollupPage( hInstance, MAKEINTRESOURCE(IDD_COMPOSITEMAP),
		PanelDlgProc,
		GetString( IDS_COMPOSITE_LAYERS ),
		(LPARAM)this );

	// We cannot directly construct m_LayerRollupWnd since we need to call
	// AddRollupPage before.
	// Note that the corresponding ReleaseIRollup is called when m_LayerRollupWnd is
	// destroyed.
	m_LayerRollupWnd = ::GetIRollup( ::GetDlgItem(m_DialogWindow, IDC_COMP_LAYER_LIST) );

	// Create a new managed callback and register it.
	m_RollupCallback = new RollupCallback( *this );
	m_LayerRollupWnd->RegisterRollupCallback( m_RollupCallback );

	// Create the button image list
	HBITMAP hBitmap, hMask;
	hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_COMP_BUTTONS));
	hMask   = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_COMP_MASKBUTTONS));
	ImageList_Add( m_ButtonImageList, hBitmap, hMask );
	DeleteObject(hBitmap);
	DeleteObject(hMask);

	// Create rollups and dialog classes for all layers already available
	for( Texture::iterator it  = m_Texture->begin(); it != m_Texture->end(); ++it )
		CreateRollup( &*it );

	// Disable the number of layers
	EditPtr( m_DialogWindow, IDC_COMP_TOTAL_LAYERS )->Disable();

	// Set the Add layer button.
	using namespace ToolBar;
	ButtonPtr add_button( m_DialogWindow, IDC_COMP_ADD );
	EditPtr   add_edit  ( m_DialogWindow, IDC_COMP_TOTAL_LAYERS );
	add_button->Execute( I_EXEC_CB_NO_BORDER );
	add_button->Execute( I_EXEC_BUTTON_DAD_ENABLE, 0 );
	add_button->SetTooltip( TRUE, GetString( IDS_DS_COMP_TOOLTIP_ADD_LAYER ) );
	add_edit  ->SetTooltip( TRUE, GetString( IDS_DS_COMP_TOOLTIP_ADD_LAYER ) );

	add_button->SetImage( m_ButtonImageList,	AddIcon,    AddIcon,
												AddIconDis, AddIconDis,
												IconWidth,  IconHeight );

	// Validate or change the control state
	EnableDisableInterface();
}

// Create a rollout.
void Composite::Dialog::CreateRollup( Layer* layer ) {
	// Does NOT add a rollup if one is already present.
	if (find(layer) != end())
		return;

	// see if we are inserting layer. This happens on undo/redo.
	int category = -1;
	bool insertLayer = false;
	iterator it = begin();
	if (layer != NULL && it != end() && begin()->Layer()->Index() > layer->Index()) {
		for( ; it != end(); ++it ) {
			if (it->Layer()->Index() > layer->Index()) {
				category = it->Panel()->GetCategory()+1;
				insertLayer = true;
			}
		}
	}

	if (!insertLayer)
		category = --m_CategoryCounter;

	// Add the rollup to the window
	int i = m_LayerRollupWnd->AppendRollup( hInstance,
		MAKEINTRESOURCE( IDD_COMPOSITEMAP_LAYER ),
		PanelDlgProc,
		_T("Layer"),
		LPARAM( this ),
		0,
		category );
	HWND            wnd  ( m_LayerRollupWnd->GetPanelDlg(i) );
	::IRollupPanel* panel( m_LayerRollupWnd->GetPanel( wnd ) );

	// Set opening of the rollup if necessary.
	if (layer != NULL)
		m_LayerRollupWnd->SetPanelOpen( i, layer->DialogOpened() );

	// Reposition categories if we have too many used (since deleting a Layer
	// doesn't free the category).  Shouldn't happen too often
	if (m_CategoryCounter < 1) {
		m_CategoryCounter = Param::CategoryCounterMax;
		// Re-categorize the layers.  Should not happen often
		for( iterator it  = begin(); it != end(); ++it, --m_CategoryCounter )
			it->Panel()->SetCategory( m_CategoryCounter );
	}

	m_LayerRollupWnd->Show( i );
	m_LayerRollupWnd->UpdateLayout();

	// Create and push the new Layer on the list
	if (insertLayer)
		insert(it, LayerDialog( layer, panel, wnd, &m_DragManager, m_ButtonImageList ) );
	else
		push_front( LayerDialog( layer, panel, wnd, &m_DragManager, m_ButtonImageList ) );
	Invalidate();
}

// Delete the first layer or the specified one.
void Composite::Dialog::DeleteRollup( Layer* layer ) {
	iterator it( layer ? find(layer) : begin() );

	if (it == end())
		return;

	HWND wnd  ( it->Panel()->GetHWnd() );
	int  index( m_LayerRollupWnd->GetPanelIndex( wnd ) );
	m_LayerRollupWnd->DeleteRollup( index, 1 );
	erase( it );
}

// Show the rename dialog to the user and change the layer's name.
void Composite::Dialog::RenameLayer( LayerDialog* layerDialog ) {
	// This is the dialog's handler class.
	// Basically it encapsulates the WinProc and call back Texture's members
	// to set or reset the layer name.
	struct rename_dialog {
		Layer     *m_Layer;
		Texture   *m_Texture;
		HWND       m_hWnd;

		static INT_PTR CALLBACK proc( HWND h, UINT msg, WPARAM w, LPARAM l ) {
			rename_dialog* Dialog(DLGetWindowLongPtr< rename_dialog* >( h ));

			switch( msg ) {
			// On init, set the user data to the rename_dialog class and
			// set the text inside, then select it all.
			case WM_INITDIALOG:
				DLSetWindowLongPtr( h, l, GWLP_USERDATA );
				Dialog = DLGetWindowLongPtr< rename_dialog* >( h );
				Dialog->m_hWnd = h;
				SetWindowText( GetDlgItem( Dialog->m_hWnd, IDC_LAYERNAME ),
					Dialog->m_Layer->Name() );
				SetFocus( GetDlgItem( Dialog->m_hWnd, IDC_LAYERNAME ) );
				SendMessage(GetDlgItem( Dialog->m_hWnd, IDC_LAYERNAME ),
					EM_SETSEL,
					0, MAKELONG(0, _tcslen(Dialog->m_Layer->Name())) );
				return FALSE;
				break;

			case WM_COMMAND:
				switch (LOWORD(w)) {
					// On OK, set the name to the editbox text.
				case IDOK: {
						TCHAR buff[ 1024 ];
						GetWindowText( GetDlgItem( Dialog->m_hWnd, IDC_LAYERNAME ),
							buff, sizeof( buff ) / sizeof( *buff ) );
						Dialog->m_Layer->Name( buff );
						EndDialog( Dialog->m_hWnd, 0 );
						break;
					}
				}
				break;

			case WM_CLOSE:
				EndDialog( Dialog->m_hWnd, 0 );
				break;

			default:
				return FALSE;
			}
			return TRUE;
		}
	} proc = { layerDialog->Layer(), m_Texture, 0 };

	// Call the dialog.
	DialogBoxParam( hInstance, MAKEINTRESOURCE(IDD_COMPOSITEMAP_RENAME),
		m_DialogWindow,
		rename_dialog::proc,
		LPARAM(&proc) );

	// Update the Name of this Layer...
	HWND wnd  ( layerDialog->Panel()->GetHWnd() );
	int  index( m_LayerRollupWnd->GetPanelIndex( wnd ) );
	m_LayerRollupWnd->SetPanelTitle( index, const_cast<LPTSTR>(layerDialog->Title().c_str()) );
}

void Composite::Dialog::Rewire( ) {
	DbgAssert( size() == m_Texture->size() );
	if (size() != m_Texture->size())
		return;

	ValueGuard<bool>   reloading_guard( m_Reloading, true );
	reverse_iterator   dlg_it( rbegin() );
	Texture::iterator  lay_it( m_Texture->begin() );

	m_CategoryCounter = Param::CategoryCounterMax;

	for( ; dlg_it != rend(); ++dlg_it, ++lay_it ) {
		dlg_it->Layer( &*lay_it );
		dlg_it->Panel()->SetCategory( --m_CategoryCounter );
	}

	m_LayerRollupWnd->UpdateLayout();
	m_Texture->NotifyChanged();
}

void Composite::Dialog::ReloadDialog() {
	// Ensure m_Reloading returns to false when we go out.
	ValueGuard<bool> reloading_guard( m_Reloading, true );

	Interval valid;
	m_Texture->Update(m_MtlParams->GetTime(), valid);

	// Update number of layers
	EditPtr layer_edit( m_DialogWindow, IDC_COMP_TOTAL_LAYERS );
	DbgAssert(false == layer_edit.IsNull());

	if (layer_edit.IsNull())
		return;

	_tostringstream oss;
	oss << m_Texture->nb_layer();
	layer_edit->SetText( const_cast<LPTSTR>(oss.str().c_str()) );

	// Update sub-dialogs (in reverse order so that the Name is correct)
	for( iterator it  = begin(); it != end(); ++it ) {
		Layer* layer( it->Layer() );
		if ( layer == NULL )
			continue;

		int Index( m_LayerRollupWnd->GetPanelIndex( it->Panel()->GetHWnd() ) );
		m_LayerRollupWnd->SetPanelOpen ( Index, layer->DialogOpened() );
		m_LayerRollupWnd->SetPanelTitle( Index, const_cast<LPTSTR>(it->Title().c_str()) );

		it->UpdateDialog();
	}
}

void Composite::Dialog::SetTime(TimeValue t) {
	//we need to check the opacity, mask, and map for any animation and if so update
	//those specific controls so we dont get excess ui redraw
	//get our map validity
//   Interval valid;
//   valid.SetInfinite();
//   m_Texture->GetParamBlock(0)->GetValidity(t,valid);

	int ct =  m_Texture->GetParamBlock(0)->Count( BlockParam::Opacity);
	for(iterator it = begin(); it != end(); it++) {
		it->SetTime( t );

		Interval iv;
		iv.SetInfinite();
		Texmap *map = it->Layer()->Texture();
		if (map) {
			iv.SetInfinite();
			iv = map->Validity(t);
			//if it is animated we need to invalidate the swatch on time change
			if (!(iv == FOREVER)) {
				it->UpdateTexture();
			}
		}
		iv.SetInfinite();
		map = it->Layer()->Mask();
		if (map) {
			iv.SetInfinite();
			iv = map->Validity(t);
			//if it is animated we need to invalidate the swatch on time change
			if (!(iv == FOREVER)) {
				it->UpdateMask();
			}
		}

		int index = (int)it->Layer()->Index();
		if ((index >= 0) && (index < ct)) {
			iv.SetInfinite();
			float opac = 0.0f;
			m_Texture->GetParamBlock(0)->GetValue(BlockParam::Opacity,t,opac,iv,index);
			if (!(iv == FOREVER)) {
				//if it is animated we need to invalidate the opacity spinnner on time change
				it->UpdateOpacity();
			}
			iv.SetInfinite();
			BOOL visible = TRUE;
			m_Texture->GetParamBlock(0)->GetValue(BlockParam::Visible,t,visible,iv,index);
			if (!(iv == FOREVER)) {
				//if it is animated we need to invalidate the button on time change
				it->UpdateVisibility();
			}
			iv.SetInfinite();
			visible = TRUE;
			m_Texture->GetParamBlock(0)->GetValue(BlockParam::VisibleMask,t,visible,iv,index);
			if (!(iv == FOREVER)) {
				//if it is animated we need to invalidate the button on time change
				it->UpdateMaskVisibility();
			}
		}
	}

	if (!m_Texture->GetValidity().InInterval(t)) {
		UpdateMtlDisplay();
	}
}

// Nothing to do here since everything is managed by SmartPtr or composited.
Composite::Dialog::~Dialog() {
	m_Texture->ParamDialog( NULL );

	// Delete rollup from the material editor
	m_MtlParams->DeleteRollupPage( m_DialogWindow );
}

// Window Proc for all the layers and the main dialog.  This could really use a clean
// up and add a method by event, then calling them. Ideally put those in a map
// outside this function, thus reducing it to less than a couple of lines.
INT_PTR Composite::Dialog::PanelProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	int      id   ( LOWORD(wParam) );
	int      code ( HIWORD(wParam) );
	bool updateViewPort = false;

	switch (msg) {
	case WM_PAINT:
		if (m_Redraw && hWnd == m_DialogWindow) {
			m_Redraw = false;
			ReloadDialog();
		}
		break;
	// When recalculating the layout of the rollups (happening when a rollup is opened/closed
	// by the user), we update the state of the rollup in the ParamBlock.
	case WM_CUSTROLLUP_RECALCLAYOUT:
		if (!m_Reloading) {
			StartChange();
			for(iterator it = begin(); it != end(); it++) {
				int i( m_LayerRollupWnd->GetPanelIndex( it->Panel()->GetHWnd() ) );
				if (it->Layer()->DialogOpened() != m_LayerRollupWnd->IsPanelOpen( i )) {
					it->Layer()->DialogOpened( m_LayerRollupWnd->IsPanelOpen( i ) );
				}
			}
			StopChange( IDS_DS_UNDO_DLG_OPENED );
		}
		break;
	case WM_COMMAND: {
		iterator it( find( hWnd ) );
		switch( id ) {
		// Texture button.
		case IDC_COMP_TEX:
			DbgAssert( it != end() );
			if (it == end())
				break;

			// max doesn't support undo of subtexture assignment. As a matter of fact it
			// flushes the undo system if doing so. Here we aren't necessarily doing an
			// assignment - if subtexture exists, we go to its ui 
//			StartChange(); 
			SendMessage(m_EditorWnd, WM_TEXMAP_BUTTON,
				(int)it->Layer()->Index()*2 ,
				LPARAM(m_Texture));
//			StopChange( IDS_DS_UNDO_TEXMAP );
			updateViewPort = true;
			break;
		// Mask Texture button.
		case IDC_COMP_MASK:
			DbgAssert( it != end() );
			if (it == end())
				break;

			// max doesn't support undo of subtexture assignment. As a matter of fact it
			// flushes the undo system if doing so. Here we aren't necessarily doing an
			// assignment - if subtexture exists, we go to its ui 
//			StartChange();
			SendMessage(m_EditorWnd, WM_TEXMAP_BUTTON,
				(int)it->Layer()->Index() * 2 + 1,
				LPARAM(m_Texture));
//			StopChange( IDS_DS_UNDO_MASK );
			updateViewPort = true;
			break;
		// Visible button in toolbar.
		case IDC_COMP_VISIBLE:
			DbgAssert( it != end() );
			if (it == end())
				break;

			// Toggle the visibility
			StartChange();
			it->Layer()->ToggleVisible( );
			it->UpdateDialog( );
			StopChange( IDS_DS_UNDO_VISIBLE );
			updateViewPort = true;
			break;
		// Mask Visible button in toolbar.
		case IDC_COMP_MASK_VISIBLE:
			DbgAssert( it != end() );
			if (it == end())
				break;

			// Toggle the visibility of the Mask
			StartChange();
			it->Layer()->ToggleVisibleMask( );
			it->UpdateDialog( );
			StopChange( IDS_DS_UNDO_MASK_VISIBLE );
			updateViewPort = true;
			break;
		// Delete button in toolbar.
		case IDC_COMP_LAYER_REMOVE:
			DbgAssert( it != end() );
			if (it == end())
				break;

			StartChange();
			m_Texture->DeleteLayer( it->Layer()->Index() );
			StopChange( IDS_DS_UNDO_REMOVE );
			DbgAssert( m_Texture->nb_layer() > 0 );
			updateViewPort = true;
			break;
		// Rename button in toolbar.
		case IDC_COMP_LAYER_RENAME:
			DbgAssert( it != end() );
			if (it == end())
				break;

			StartChange();
			RenameLayer( &*it );
			StopChange( IDS_DS_UNDO_RENAME );
			break;
		// Duplicate button in toolbar.
		case IDC_COMP_LAYER_DUPLICATE:
			DbgAssert( it != end() );
			if (it == end())
				break;

			StartChange();
			m_Texture->DuplicateLayer( it->Layer()->Index() );
			StopChange( IDS_DS_UNDO_DUPLICATE );
			updateViewPort = true;
			break;
		// Blending mode combo in toolbar.
		case IDC_COMP_BLEND_TYPE:
			DbgAssert( it != end() );
			if (it == end())
				break;

			if (code == CBN_SELCHANGE) {
				int selection( SendMessage( HWND(lParam), CB_GETCURSEL, 0, 0 ) );
				StartChange();
				it->Layer()->Blend( Layer::BlendType( selection ) );
				StopChange( IDS_DS_UNDO_BLENDMODE );
			}
			updateViewPort = true;
			break;
		// Opacity edit in toolbar.
		case IDC_COMP_OPACITY:
			if (code == EN_SETFOCUS) {
				EditPtr edit( hWnd, IDC_COMP_OPACITY );
				edit->GetText(m_oldOpacityVal);
			}
			else if( code == EN_KILLFOCUS || code == EN_CHANGE ) {
				DbgAssert( it != end() );
				if (it == end())
					break;


				StartChange();
				if (code == EN_KILLFOCUS) {
					EditPtr edit( hWnd, IDC_COMP_OPACITY );
					TSTR newOpacityVal;
					edit->GetText(newOpacityVal);
					if ( m_oldOpacityVal != newOpacityVal )
						OnOpacityFocusLost( hWnd );
				}
				else {
					OnOpacityEditChanged( hWnd );
				}
				StopChange( IDS_DS_UNDO_OPACITY );
				updateViewPort = true;
			}
			break;
		// Insert the ColorCorrection map between this and the texture
		case IDC_COMP_CC:
			DbgAssert( it != end() );
			if (it != end()) {
				// max doesn't support undo of subtexture assignment. As a matter of fact it
				// flushes the undo system if doing so. Here we aren't necessarily doing an
				// assignment - if subtexture exists, we go to its ui 
//				StartChange();
				m_Texture->ColorCorrection( it->Layer(), false );
				// Edit the new texture
				SendMessage( m_EditorWnd, WM_TEXMAP_BUTTON,
					(int)it->Layer()->Index() * 2,
					LPARAM(m_Texture) );
//				StopChange( IDS_DS_UNDO_COLOR_CORRECTION );
				updateViewPort = true;
			}
			break;
		// Insert the ColorCorrection map between this and the mask
		case IDC_COMP_MASK_CC:
			DbgAssert( it != end() );
			if (it != end()) {
//				StartChange();
				m_Texture->ColorCorrection( it->Layer(), true );
				// Edit the new texture
				SendMessage( m_EditorWnd, WM_TEXMAP_BUTTON,
					 (int)it->Layer()->Index() * 2 + 1,
					LPARAM(m_Texture) );
//				StopChange( IDS_DS_UNDO_MASK_COLOR_CORRECTION );
				updateViewPort = true;
			}
			break;

		// Generic buttons
		// Add button (add a layer)
		case IDC_COMP_ADD:
			StartChange();
			m_Texture->AddLayer();
			StopChange( IDS_DS_UNDO_ADD_LAYER );
			updateViewPort = true;
			break;
		default:
			break;
			}
		}
		break;

	// When a spinner is pressed
	case CC_SPINNER_BUTTONDOWN:
		StartChange();
		break;

	// When a spinner is released, check if it's the opacity spinner and make change.
	case CC_SPINNER_BUTTONUP:
		switch( id ) {
		case IDC_COMP_OPACITY_SPIN:
			StopChange( IDS_DS_UNDO_OPACITY );
			//we only update the viewport on spinner up since it slows things down alot
			//on very simple cases
			updateViewPort = true;
			break;
		}
		break;

	// If value was changed, update layer.
	case CC_SPINNER_CHANGE: {
			iterator it( find( hWnd ) );
			DbgAssert( it != end() );
			if (it == end())
				break;

			switch( id ) {
			case IDC_COMP_OPACITY_SPIN:
				OnOpacitySpinChanged( hWnd, it );
				break;
			}
		}
		break;

	case WM_DESTROY:
		Destroy(hWnd);
		break;
	}

	if (updateViewPort) {
		TimeValue t = GetCOREInterface()->GetTime();
		GetCOREInterface()->RedrawViews(t);
	}
	return FALSE;
}

ReferenceTarget* Composite::Dialog::GetThing() {
	return (ReferenceTarget*)m_Texture;
}

void Composite::Dialog::SetThing(ReferenceTarget *m) {
	if (   m                   == NULL
		|| m->ClassID()        != m_Texture->ClassID()
		|| m->SuperClassID()   != m_Texture->SuperClassID())
	{
		DbgAssert( false );
		return;
	}

	// Change current Composite, then switch.
	m_Texture->ParamDialog( NULL );

	m_Texture = static_cast<Texture*>(m);
	size_t nb( m_Texture->nb_layer() );

	// Update the number of rollup shown
	while( size() != nb ) {
		if (size() < nb)
			CreateRollup( NULL );
		else
			DeleteRollup( NULL );
	}

	// Finally set the paramdialog of the new material.
	m_Texture->ParamDialog( this );

	// Validate or change the control state
	Rewire();
	EnableDisableInterface();
	Invalidate();
}

void Composite::Dialog::UpdateMtlDisplay() {
	m_MtlParams->MtlChanged();
}

void Composite::Dialog::EnableDisableInterface() {
	if (!m_Texture)
		return;

	// If we're adding a layer after the last, re-enable the remove button.
	if (m_Texture->nb_layer() == 1) {
		const struct {
			void operator()( LayerDialog& dlg ) const { dlg.DisableRemove(); }
		} disabler;
		std::for_each( begin(), end(), disabler );
	}
	else if (m_Texture->nb_layer() == 2) {
		const struct {
			void operator()( LayerDialog& dlg ) const { dlg.EnableRemove(); }
		} enabler;
		std::for_each( begin(), end(), enabler );
	}

	if (m_Texture->nb_layer() >= Param::LayerMax) {
		ButtonPtr add_button( m_DialogWindow, IDC_COMP_ADD );
		add_button->Disable();
	} else {
		ButtonPtr add_button( m_DialogWindow, IDC_COMP_ADD );
		add_button->Enable();
	}
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Composite Texture Method Implementation

// Update the number of layers inside the ParamBlock and set the vector
// accordingly.
void Composite::Texture::UpdateParamBlock( size_t count ) {
	if (m_Updating)
		return;

	if (theHold.RestoreOrRedoing())
		return;

	if (m_ParamBlock == NULL)
		return;

	// When this guard goes out of scope, it returns Updating to its old
	// value.
	ValueGuard<bool> updating_guard( m_Updating, true );

	m_ParamBlock ->SetCount( BlockParam::Visible,        int(count) );
	macroRec->Disable();  // only record one count change
	m_ParamBlock ->SetCount( BlockParam::VisibleMask,    int(count) );
	m_ParamBlock ->SetCount( BlockParam::BlendMode,      int(count) );
	m_ParamBlock ->SetCount( BlockParam::Name,           int(count) );
	m_ParamBlock ->SetCount( BlockParam::DialogOpened,   int(count) );
	m_ParamBlock ->SetCount( BlockParam::Opacity,        int(count) );
	m_ParamBlock ->SetCount( BlockParam::Texture,        int(count) );
	m_ParamBlock ->SetCount( BlockParam::Mask,           int(count) );
	macroRec->Enable();

	SetNumMaps(int(count));
	ValidateParamBlock();
}

// When param block is changed, we readjust it to make sure we are ok.
void Composite::Texture::OnParamBlockChanged( ParamID id ) {
	if (m_ParamBlock) {
		int count( m_ParamBlock->Count( id ) );
		UpdateParamBlock( count );
	}
}

// Add a color correction map between us and the texture/mask
void Composite::Texture::ColorCorrection( Composite::Layer* layer, bool mask ) {

	int index;
	if (mask)
		index = (int)layer->Index() * 2 + 1;
	else
		index = (int)layer->Index() * 2;

	ClassDesc2 * ccdesc( GetColorCorrectionDesc() );
	Texmap     * ref   ( GetSubTexmap( index ) );

	// We don't duplicate color correction here
	if (ref == NULL || (ref->ClassID() != ccdesc->ClassID())) {
		Texmap     * cc    ( (Texmap*)CreateInstance( ccdesc->SuperClassID(), ccdesc->ClassID() ) );

		cc->SetName( ccdesc->ClassName() );

		// Set the texture
		HoldSuspend hs; // don't need to be able to undo setting of the subtexture here
		cc->SetSubTexmap( 0, ref );
		hs.Resume();
		SetSubTexmap( index, cc );
	}
}

void Composite::Texture::AddLayer() {
	// If we've attained our maximum number of layers, we don't add a new one.
	if (nb_layer() >= Param::LayerMax) {
		MessageBox( 0, GetString( IDS_ERROR_MAXLAYER ),
			GetString( IDS_ERROR_TITLE ),
			MB_APPLMODAL | MB_OK | MB_ICONERROR );
		return;
	}

	UpdateParamBlock( nb_layer() + 1 );
}

void Composite::Texture::DeleteLayer( size_t Index ) {
	// If there is only one layer, or the index is outside of range,
	// we do not delete it.
	if (nb_layer() == 1) {
		MessageBox( 0, GetString( IDS_ERROR_MINLAYER ),
			GetString( IDS_ERROR_TITLE ),
			MB_APPLMODAL | MB_OK | MB_ICONERROR );
		return;
	} else if (Index < 0 || Index >= nb_layer()) {
		return;
	}

	PopLayer( Index );
}

// Add a new layer at the end. If loading is false, sets its name to the default one.
// If we are loading a file, don't set the name as it would overwrite the name in the
// pb2 we are loading
void Composite::Texture::PushLayer( bool setName ) {
	if (theHold.Holding())
		theHold.Put(new PushLayerRestoreObject(this, m_ParamBlock, setName));

	// We create the Layer at the end...
	push_back( Composite::Layer( m_ParamBlock, nb_layer() ) );
	Composite::Layer &new_layer( *LayerIterator( nb_layer()-1 ) );

	if (setName)
		new_layer.Name( Layer::NameType( m_DefaultLayerName.c_str() ) );

	// Create and push the new Layer on the list
	if (m_ParamDialog) {
		m_ParamDialog->EnableDisableInterface();
		m_ParamDialog->CreateRollup( &new_layer );
	}

	ValidateParamBlock();
}

// Delete a layer.
void Composite::Texture::PopLayer( size_t Index ) {
	iterator it = LayerIterator( Index );

	if( it != end() ) {
		// Update the interface
		if (m_ParamDialog)
			m_ParamDialog->DeleteRollup( &*it );

		erase( it );
		// invalidates it, so we need to refetch it.
		it = LayerIterator( Index ); // if Index is size() it will be end()

		// Update all indexes in the paramblock...
		for( ; it != end(); ++it ) {
			it->Index( it->Index() - 1 );
		}

		if (theHold.Holding())
			theHold.Put(new PopLayerBeforeRestoreObject(this, m_ParamBlock, Index));

		if (m_ParamBlock && !m_Updating) {
			// This is to prevent UpdateParamBlock from recalling us.
			// if we wrapped around, we are pretty well screwed because of everything else happening in this method.
			ValueGuard<bool> updating_guard( m_Updating, true );

			// Update the paramblock
			m_ParamBlock->Delete( BlockParam::Visible,      int(Index), 1 );
			m_ParamBlock->Delete( BlockParam::VisibleMask,  int(Index), 1 );
			m_ParamBlock->Delete( BlockParam::BlendMode,    int(Index), 1 );
			m_ParamBlock->Delete( BlockParam::Name,         int(Index), 1 );
			m_ParamBlock->Delete( BlockParam::DialogOpened, int(Index), 1 );
			m_ParamBlock->Delete( BlockParam::Opacity,      int(Index), 1 );
			m_ParamBlock->Delete( BlockParam::Texture,      int(Index), 1 );
			m_ParamBlock->Delete( BlockParam::Mask,         int(Index), 1 );
		}

		if (theHold.Holding())
			theHold.Put(new PopLayerAfterRestoreObject(this, m_ParamBlock, Index));

		// Redraw
		if (m_ParamDialog) {
			m_ParamDialog->EnableDisableInterface();
			m_ParamDialog->Invalidate();
		}
	}

	ValidateParamBlock();
}

// Insert a layer.
void Composite::Texture::InsertLayer( size_t Index ) {
	// Create and push the new Layer on the list
	iterator layer = LayerIterator ( Index );
	Composite::Layer &new_layer( Composite::Layer( m_ParamBlock, Index ) );
	insert( layer, new_layer );

	// Update all indexes in the paramblock...
	for( ; layer != end(); ++layer ) {
		layer->Index( layer->Index() + 1 );
	}

	if (theHold.Holding())
		theHold.Put(new InsertLayerBeforeRestoreObject(this, m_ParamBlock, Index));

	if (m_ParamBlock && !m_Updating) {
		// This is to prevent UpdateParamBlock from recalling us.
		ValueGuard<bool> updating_guard( m_Updating, true );

		Texmap *nullTexmap = NULL;
		BOOL visible = Default::Visible;
		BOOL visibleMask = Default::VisibleMask;
		float opacity = Default::Opacity;
		int blendMode = Default::BlendMode;
		BOOL dialogOpened = Default::DialogOpened;
		TCHAR *name = const_cast<TCHAR*>(m_DefaultLayerName.c_str());

		// Update the paramblock
		m_ParamBlock->Insert( BlockParam::Visible,      int(Index), 1, &visible );
		m_ParamBlock->Insert( BlockParam::VisibleMask,  int(Index), 1, &visibleMask );
		m_ParamBlock->Insert( BlockParam::BlendMode,    int(Index), 1, &blendMode );
		m_ParamBlock->Insert( BlockParam::Name,         int(Index), 1, &name );
		m_ParamBlock->Insert( BlockParam::DialogOpened, int(Index), 1, &dialogOpened );
		m_ParamBlock->Insert( BlockParam::Opacity,      int(Index), 1, &opacity );
		m_ParamBlock->Insert( BlockParam::Texture,      int(Index), 1, &nullTexmap );
		m_ParamBlock->Insert( BlockParam::Mask,         int(Index), 1, &nullTexmap );

		if (theHold.Holding())
			theHold.Put(new InsertLayerAfterRestoreObject(this, m_ParamBlock, Index));

		// Redraw
		if (m_ParamDialog) {
			m_ParamDialog->EnableDisableInterface();
			iterator new_layer = LayerIterator ( Index );
			m_ParamDialog->CreateRollup( &(*new_layer) );
			m_ParamDialog->Invalidate();
		}
	}

	ValidateParamBlock();
}

// Move a layer by inserting a layer, copying current layer data to it, and then deleting current layer.
// Tricky part is we need to send out a REFMSG_SUBANIM_NUMBER_CHANGED message with correct mapping
// TODO: PB2 isn't sending a REFMSG_SUBANIM_NUMBER_CHANGED for many cases, including setting a 
// reftarg-derived value or causing an animation controller to be assigned.
// before arg here is in terms of ui, which is opposite of layer order
void Composite::Texture::MoveLayer( size_t index, size_t dest, bool before ) {

	// create no new keys while setting values...
	AnimateSuspend as (TRUE, TRUE);

	if (before)
		dest++;

	iterator          it_src( LayerIterator( index ) );
	iterator          it_dst( LayerIterator( dest  ) );
	int               i     ( (int)index );

	if (it_src == it_dst)
		return;

	if (it_dst != begin() && it_src == --it_dst)
		return;

	// insert new, copy data, delete original
	InsertLayer( dest );
	it_dst = LayerIterator( dest  );
	it_dst->CopyValue( *it_src );
	PopLayer(it_src->Index());

	if (m_ParamDialog) {
		// Re-wire the dialogs...
		m_ParamDialog->Rewire();
		NotifyChanged();
	}
}

// Basically a duplicated Layer is a new Layer copied from the layer to duplicate
// and moved to the right position.  This is to re-use the code.
void Composite::Texture::DuplicateLayer ( size_t Index ) {
	// If we've attained our maximum number of layers, we don't add a new one.
	if (nb_layer() >= Param::LayerMax) {
		MessageBox( 0, GetString( IDS_ERROR_MAXLAYER ),
			GetString( IDS_ERROR_TITLE ),
			MB_APPLMODAL | MB_OK | MB_ICONERROR );
		return;
	}

	DbgAssert( Index < nb_layer() );
	if (Index >= nb_layer())
		return;

	// suspend macro recorder, animate, set key
	SuspendAll suspendAll (FALSE, TRUE, TRUE, TRUE);

	// Insert the layer directly below me and copy values from me to new layer 
	iterator it     ( LayerIterator( Index ) );
	InsertLayer(Index);
	iterator new_lyr( LayerIterator( Index ) );
	new_lyr->CopyValue( *it );

	// Give the layer a new name so we can distinguish it from the original
	TSTR newName;
	TSTR origName = new_lyr->Name(); //
	if (origName.isNull())
		origName = GetString(IDS_UNNAMED_LAYER);
	newName.printf(_T("%s%s"),GetString(IDS_DUPLICATE_OF), origName.data());
	new_lyr->Name(newName);

	if (m_ParamDialog) {
		// Re-wire the dialogs...
		m_ParamDialog->Rewire();
		NotifyChanged();
	}
}

void Composite::Texture::Init() {
	macroRecorder->Disable();
	m_Validity.SetEmpty();
	m_ViewportIval.SetEmpty();
	m_ViewportTextureList.swap( ViewportTextureListType() );
	UpdateParamBlock( 1 );
	macroRecorder->Enable();
}

void Composite::Texture::Reset() {
	DeleteAllRefsFromMe();
	GetCompositeDesc()->MakeAutoParamBlocks(this);   // make and intialize paramblock2
	Init();
}

void Composite::Texture::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

Composite::Texture::Texture(BOOL loading)
	: m_ParamDialog   ( NULL )
	, m_ParamBlock    ( NULL )
	, m_Updating      ( false )
	, m_SettingNumMaps( false )
{
	// Init default values to members
	m_DefaultLayerName = Default::Name;
	m_FileVersion = kMax2009;
	m_OldMapCount_2008 = 0;
	m_OldMapCount_JohnsonBeta = 0;
	if (!loading)
		GetCompositeDesc()->MakeAutoParamBlocks( this );   // make and intialize paramblock2
	Init();
}

// Update the number of layers, either adding or deleting the last one.
void Composite::Texture::SetNumMaps( int n,  bool setName ) {

	// guard against wrap around. Pushing/popping a layer causes a REFMSG_CHANGE message to be 
	// sent which hits Texture::NotifyRefChanged which calls UpdateParamBlock, which calls SetNumMaps
	if (m_SettingNumMaps)
		return;

	ValueGuard< bool >   change( m_SettingNumMaps, true );

	while( n != nb_layer() )
		if( n < nb_layer() )
			PopLayer( nb_layer()-1 );
		else
			PushLayer( setName );
}

void Composite::Texture::ValidateParamBlock() {
	// Check the count of every parameters so that they are coherent with internal
	// data.
	// If we are setting the number of maps, this may differ...
	if( m_ParamBlock && !m_SettingNumMaps ) {
		DbgAssert( m_ParamBlock->Count(BlockParam::Visible      ) == nb_layer() );
		DbgAssert( m_ParamBlock->Count(BlockParam::VisibleMask  ) == nb_layer() );
		DbgAssert( m_ParamBlock->Count(BlockParam::BlendMode    ) == nb_layer() );
		DbgAssert( m_ParamBlock->Count(BlockParam::Name         ) == nb_layer() );
		DbgAssert( m_ParamBlock->Count(BlockParam::DialogOpened ) == nb_layer() );
		DbgAssert( m_ParamBlock->Count(BlockParam::Opacity      ) == nb_layer() );
		DbgAssert( m_ParamBlock->Count(BlockParam::Texture      ) == nb_layer() );
		DbgAssert( m_ParamBlock->Count(BlockParam::Mask         ) == nb_layer() );
	}
}

// Accumulate the layers and return the result.
// Please note that EvalMono was not implemented because calling EvalMono
// of the children would not get the Alpha channel necessary for compositing.
// That leads to weird assumptions about blending and compositing.
AColor Composite::Texture::EvalColor(ShadeContext& sc) {
	AColor res;
	if ( sc.GetCache( this, res ) )
		return res;

	if ( gbufID )
		sc.SetGBufferID(gbufID);

	// Simple accumulator that blends the layers
	class accumulator {
	  ShadeContext&  sc;

	public:
	  accumulator( ShadeContext& shade ) : sc(shade) { }

		AColor operator()( const AColor& bg, const value_type& Layer ) {
		 // If the foreground is invisible, return the background.
		 if (false == Layer.IsVisible())
			return bg;

		 // Take color, apply Opacity, check Mask, Blend, rinse and re-do
		 AColor fg( Layer.Texture()->EvalColor( sc ) );

		 //we need to pull out the premultiplied alpha coming from a texture
		 if ((fg.a != 1.0f) && (fg.a != 0.0f)) {
			 fg.r /= fg.a;
			 fg.g /= fg.a;
			 fg.b /= fg.a;
		 }

		 if (Layer.IsMaskVisible()) {
			const float mask_alpha( Layer.Mask()->EvalMono( sc ) );
			fg.a *= mask_alpha;
		 }

		 fg.a *= Layer.NormalizedOpacity();

		 // If the background is invisible, this is equal to the foreground.
		 // Technically this means that the last visible layer is ALWAYS
		 // blended with Normal.
		 if (bg.a == 0) 
			return fg;

		 // AColor has no conversion to (const Color), only non-const. So we have
		 // to do it manually.
		 const Color bg_temp( bg.r, bg.g, bg.b );
		 // Apply Blending here.  fg is fore, bg is back.
		 const Color blended( Blending::ModeList[ Layer.Blend() ]( fg, bg_temp ) );
		 const float alpha  ( fg.a + (1 - fg.a) * bg.a );

			// We composite.  This is an "over" mode of composition, same as Combustion
			// and photoshop uses.
			// On blending mode: http://www.pegtop.net/delphi/articles/blendmodes/index.htm
			// Normal, add, subtract, average and multiple are not clamped
			const float retR( (  blended.r * (fg.a *      bg.a)
				+ fg.r      * (fg.a * (1 - bg.a))
				+ bg.r      * ((1 - fg.a) * bg.a)) / alpha );
			const float retG( (  blended.g * (fg.a *      bg.a)
				+ fg.g      * (fg.a * (1 - bg.a))
				+ bg.g      * ((1 - fg.a) * bg.a)) / alpha );
			const float retB( (  blended.b * (fg.a *      bg.a)
				+ fg.b      * (fg.a * (1 - bg.a))
				+ bg.b      * ((1 - fg.a) * bg.a)) / alpha );
	     
		 return AColor( retR, retG, retB, alpha );
	  }
	};

   // We don't want to accumulate from real Black
   res = std::accumulate( begin(), end(), AColor(0, 0, 0, 0), accumulator(sc) );


   
   //we now need to premultiple our apha into the results
   if (res.a != 1.0f)
   {
	   res.r *= res.a;
	   res.g *= res.a;
	   res.b *= res.a;
   }

   sc.PutCache( this, res );
   return res;
}

inline float BlendOver( float fg, float fgAlpha, float bg, float bgAlpha, float blended, float alpha  )
// Alpha should be ( fgAlpha + (1 - fgAlpha) * bgAlpha )
{
	// This is an "over" mode of composition, same as Combustion
	// and photoshop uses.
	// On blending mode: http://www.pegtop.net/delphi/articles/blendmodes/index.htm
	// Since everything is clamped between [0,1], there is no need to clamp
	// this color.
	return ( blended	* (fgAlpha *      bgAlpha)
			 + fg       * (fgAlpha * (1 - bgAlpha))
			 + bg       * ((1 - fgAlpha) * bgAlpha)) / alpha ;
}

float ClampZeroOne( float f ) { return (f<0? 0 : (f>1? 1 : f)); }
Color ClampZeroOne( const Color& c ) { return Color( ClampZeroOne(c.r), ClampZeroOne(c.g), ClampZeroOne(c.b) ); }
Color AddGreyscale( const Color& c, float f ) { return Color( c.r+f, c.g+f, c.b+f ); }
float  GetGreyscale( const Color& c ) { return ((c.r + c.g + c.b) / 3.0f); }


//  ---------- ---------- ---------- ---------- ---------- ---------- ---------- ----------
//  Composite Bump - Determines the bump value of the Composite Map

Point3 Composite::Texture::EvalNormalPerturb(ShadeContext& sc)  {
	Point3 result;

	if (!sc.doMaps) 
		return Point3(0,0,0);
	if (gbufID) 
		sc.SetGBufferID(gbufID);
	if ( sc.GetCache( this, result ) )
		return result;

	struct BumpModeInfo {
		Point3 deriv;
		Color  color;
		float  alpha;
		BumpModeInfo() : deriv(0,0,0), color(0,0,0), alpha(0) {}
	};

	class accumulator  {
	  ShadeContext&  sc;

	public:
	  accumulator( ShadeContext& shade ) : sc(shade) { }


	  BumpModeInfo operator()( const BumpModeInfo& bg, const value_type& Layer )
	  {
		 // If the foreground is invisible, return the background.
		 if (false == Layer.IsVisible())
			return bg;

		 BumpModeInfo bumpModeInfoTemp;	 
		 BumpModeInfo& fg = bumpModeInfoTemp;

		 AColor fgColor = Layer.Texture()->EvalColor( sc );
		 fg.color = fgColor;
		 fg.alpha = fgColor.a;

		 //we need to pull out the premultiplied alpha coming from a texture
		 if ((fg.alpha != 1.0f) && (fg.alpha != 0.0f)) {
			 fg.color.r /= fg.alpha;
			 fg.color.g /= fg.alpha;
			 fg.color.b /= fg.alpha;
		 }

		 if (Layer.IsMaskVisible())
			fg.alpha *= Layer.Mask()->EvalMono( sc );

		 fg.alpha *= Layer.NormalizedOpacity();

		 fg.deriv = Layer.Texture()->EvalNormalPerturb( sc );

		 // If the background is invisible, this is equal to the foreground.
		 // Technically this means that the last visible layer is ALWAYS
		 // blended with Normal.
		 if (bg.alpha == 0) 
			return fg;

		 float  alpha  ( fg.alpha + bg.alpha * (1 - fg.alpha) );


		 // Determine Blended Color.  fg is fore, bg is back.  
		 Color colorBlended( Blending::ModeList[ Layer.Blend() ]( fg.color, bg.color ) );


		 // Compute interpolated texture colors, three each for foreground and background.
		 // The distance across which we interpolate is scaled down by a constant factor.
		 // This gives a derivative that is more accurate locally.
		 // The magnitude of the final derivate must be scaled up by an inverse factor.
		 const float magicNumber = 0.1f; // Chosen through empirical tests; from 1/4 to 1/16 is reasonable.

		 float stepScale = magicNumber; 
		 float stepScaleInv = 1.0f / magicNumber;

		 Color fgInterpX = ClampZeroOne( AddGreyscale(fg.color, -(fg.deriv.x * stepScale)) );
		 Color fgInterpY = ClampZeroOne( AddGreyscale(fg.color, -(fg.deriv.y * stepScale)) );
		 Color fgInterpZ = ClampZeroOne( AddGreyscale(fg.color, -(fg.deriv.z * stepScale)) );

		 Color bgInterpX = ClampZeroOne( AddGreyscale(bg.color, -(bg.deriv.x * stepScale)) );
		 Color bgInterpY = ClampZeroOne( AddGreyscale(bg.color, -(bg.deriv.y * stepScale)) );
		 Color bgInterpZ = ClampZeroOne( AddGreyscale(bg.color, -(bg.deriv.z * stepScale)) );

		 // Compute the interpolated composite color for each pair of interpolated texture colors
		 Color colorBlendedInterpX( Blending::ModeList[ Layer.Blend() ]( fgInterpX, bgInterpX ) );
		 Color colorBlendedInterpY( Blending::ModeList[ Layer.Blend() ]( fgInterpY, bgInterpY ) );
		 Color colorBlendedInterpZ( Blending::ModeList[ Layer.Blend() ]( fgInterpZ, bgInterpZ ) );

		 float greyInterpX = GetGreyscale( colorBlendedInterpX );
		 float greyInterpY = GetGreyscale( colorBlendedInterpY );
		 float greyInterpZ = GetGreyscale( colorBlendedInterpZ );

		 float greyOrigin = GetGreyscale( colorBlended );

		 // Compute the derivative as the difference between the original and interpolated composite colors
		 Point3 derivBlended( greyOrigin - greyInterpX, greyOrigin - greyInterpY, greyOrigin - greyInterpZ );

		 derivBlended *= stepScaleInv; //scale back up


		 // We composite.
		 // retval and fg point to the same memory - operations are ordered so each value is overwritten only when not needed later

		 BumpModeInfo& retval = bumpModeInfoTemp;
		 retval.deriv.x = ( BlendOver( fg.deriv.x, fg.alpha, bg.deriv.x, bg.alpha, derivBlended.x, alpha ) );
		 retval.deriv.y = ( BlendOver( fg.deriv.y, fg.alpha, bg.deriv.y, bg.alpha, derivBlended.y, alpha ) );
		 retval.deriv.z = ( BlendOver( fg.deriv.z, fg.alpha, bg.deriv.z, bg.alpha, derivBlended.z, alpha ) );

		 retval.color.r = ( BlendOver( fg.color.r, fg.alpha, bg.color.r, bg.alpha, colorBlended.r, alpha ) );
		 retval.color.g = ( BlendOver( fg.color.g, fg.alpha, bg.color.g, bg.alpha, colorBlended.g, alpha ) );
		 retval.color.b = ( BlendOver( fg.color.b, fg.alpha, bg.color.b, bg.alpha, colorBlended.b, alpha ) );

		 retval.alpha = alpha;

		 return retval;
	  }
	};


	BumpModeInfo bumpInfo;
	bumpInfo = BumpModeInfo(std::accumulate( begin(), end(), bumpInfo, accumulator(sc) ));
	result = bumpInfo.deriv;

	sc.PutCache( this, result );
	return result;
}

int Composite::Texture::NumRefs() {
	if (m_FileVersion == kMax2008)
		return 1 + m_OldMapCount_2008;	//just the parm block + maps
	else if (m_FileVersion == kJohnsonBeta)
		return 1 + m_OldMapCount_JohnsonBeta;	//just the parm block + maps + masks
	else 
		return 1;  //just the parm block
}

// See reference system in the {Notes} region.
RefTargetHandle Composite::Texture::GetReference(int i) {
	if (m_FileVersion == kMax2009) {
		if (i == 0)
			return m_ParamBlock;
	}
	else {
		if (i == 0)
			return m_ParamBlock;
		else {
			int mapID = i -1;
			if (mapID < m_oldMapList.Count())
				return m_oldMapList[mapID];
		}
	}
	return NULL;

}

void Composite::Texture::SetReference(int i, RefTargetHandle v) {
	if (m_FileVersion == kMax2009) {
		//we only have 1 reference now the paramblock
		if (i == 0) {
			IParamBlock2* v_ParamBlock = (IParamBlock2*)(v);

			// we are guaranteed to have one of the following 3 cases:
			//		v_ParamBlock		m_ParamBlock
			//		NULL				NULL
			//		NULL				non-NULL
			//		non-NULL			NULL
			// we will never have both v_ParamBlock and m_ParamBlock non-NULL

			DbgAssert (!(v_ParamBlock != NULL && m_ParamBlock != NULL));

			if (v_ParamBlock) {
				// Create the number of layers, don't set new layer names
				m_ParamBlock = v_ParamBlock;
				SetNumMaps( m_ParamBlock->Count( BlockParam::Texture ), false );
			}
			else {
				SetNumMaps( 0 );
				m_ParamBlock = v_ParamBlock;
			}

			// Update param blocks for all layers
			struct {
				IParamBlock2 *m_ParamBlock;
				void operator()( value_type& Layer ) {
					Layer.ParamBlock( m_ParamBlock );
				}
			} predicate = {m_ParamBlock};
			for_each( begin(), end(), predicate );
		}
		else {
			DbgAssert(0);
		}
	}
	else {
		if ( i == 0 ) {
			m_ParamBlock = (IParamBlock2*)(v);

			if (m_ParamBlock)
				m_OldMapCount_JohnsonBeta = m_ParamBlock->Count(BlockParam::Texture_DEPRECATED) * 2;
			else
				m_OldMapCount_JohnsonBeta = 0;

			struct {
				IParamBlock2 *m_ParamBlock;
				void operator()( value_type& Layer ) {
					Layer.ParamBlock( m_ParamBlock );
				}
			} predicate = {m_ParamBlock};
			for_each( begin(), end(), predicate );
		}
		else {
			int mapID = i-1;
			while (mapID >= m_oldMapList.Count()) {
				Texmap *map = NULL;
				m_oldMapList.Append(1,&map,100);
			}
			m_oldMapList[mapID] = (Texmap*) v;
		}
	}
}

RefTargetHandle Composite::Texture::Clone(RemapDir &remap) {
	Texture *NewMtl = new Texture( TRUE ); // don't need the PB2 to be created
	*((MtlBase*)NewMtl) = *((MtlBase*)this);  // copy superclass stuff

	// Replace the ParamBlock
	NewMtl->ReplaceReference( 0, remap.CloneRef( m_ParamBlock ) );

	// Copy the internal variables
	NewMtl->m_DefaultLayerName = m_DefaultLayerName;

	// Empty the interactive rendering variables
	NewMtl->m_ViewportIval.SetEmpty();
	NewMtl->m_ViewportTextureList.swap( ViewportTextureListType() );

	BaseClone(this, NewMtl, remap);
	return (RefTargetHandle)NewMtl;
}

ParamDlg* Composite::Texture::CreateParamDlg(HWND editor_wnd, IMtlParams *imp) {
	m_ParamDialog = new Dialog( editor_wnd, imp, this );
	return m_ParamDialog;
}

void Composite::Texture::Update(TimeValue t, Interval& valid) {
	if (!m_Validity.InInterval(t)) {
		m_Validity.SetInfinite();

		for (iterator it = begin(); it != end(); ++it)
			it->Update( t, m_Validity );

		Interval iv;
		iv.SetInfinite();
		m_ParamBlock->GetValidity(t,iv);
		m_Validity &= iv;
	}

	valid &= m_Validity;

	int numTextures		= m_ParamBlock->Count(BlockParam::Texture);
	int numMasks		= m_ParamBlock->Count(BlockParam::Mask);
	int numOpacities	= m_ParamBlock->Count(BlockParam::Opacity);
	int numBlendModes	= m_ParamBlock->Count(BlockParam::BlendMode);
	int numDlgOpen		= m_ParamBlock->Count(BlockParam::DialogOpened);
	int numNames		= m_ParamBlock->Count(BlockParam::Name);
	int numVisible		= m_ParamBlock->Count(BlockParam::Visible);
	int numVisibleMask	= m_ParamBlock->Count(BlockParam::VisibleMask);

	int layerCount = (int)nb_layer();
	DbgAssert( layerCount == numTextures &&
			   layerCount == numMasks &&
			   layerCount == numOpacities &&
			   layerCount == numBlendModes &&
			   layerCount == numDlgOpen &&
			   layerCount == numNames &&
			   layerCount == numVisible &&
			   layerCount == numVisibleMask
		);

	int id = 0;
	for (iterator it = begin(); it != end(); ++it) 
	{
		Interval iv;		
		DbgAssert((int)it->Index() == id);

		BOOL visible = FALSE;
		m_ParamBlock->GetValue(BlockParam::Visible,t,visible,FOREVER,id);
		DbgAssert( visible == it->Visible());

		BOOL maskVisible = FALSE;
		m_ParamBlock->GetValue(BlockParam::VisibleMask,t,maskVisible,FOREVER,id);
		DbgAssert( maskVisible == it->VisibleMask());

		int blendMode = 0;
		m_ParamBlock->GetValue(BlockParam::BlendMode,t,blendMode,FOREVER,id);
		DbgAssert( blendMode == it->Blend());

		TSTR name = m_ParamBlock->GetStr(BlockParam::Name,t,id);
		DbgAssert( name == TSTR (it->Name()));

		BOOL dialogOpened = FALSE;
		m_ParamBlock->GetValue(BlockParam::DialogOpened,t,dialogOpened,FOREVER,id);
		DbgAssert( dialogOpened == it->DialogOpened());

		float f1 = it->Opacity();
		float f2 = m_ParamBlock->GetFloat(BlockParam::Opacity,t,id);
		DbgAssert(f1==f2);

		Texmap *map = NULL;
		m_ParamBlock->GetValue(BlockParam::Texture,t,map,FOREVER,id);
		DbgAssert(map==it->Texture());

		m_ParamBlock->GetValue(BlockParam::Mask,t,map,FOREVER,id);
		DbgAssert(map==it->Mask());

		id++;
	}
}

void Composite::Texture::SetSubTexmap( int i, Texmap *m ) {
	if (m_FileVersion == kMax2009) {
		if (!is_subtexmap_idx_valid(i)) {
			DbgAssert(false);
			return ;
		}

		iterator layer = LayerIterator( subtexmap_idx_to_lyr(i) );
		DbgAssert( layer != end() );
		if (is_subtexmap_idx_tex(i))
			layer->Texture(m);
		else
			layer->Mask   (m);
	}
	else
		DbgAssert(false);
}

Texmap* Composite::Texture::GetSubTexmap(int i) {

	if (m_FileVersion == kMax2009) {
		if (!is_subtexmap_idx_valid(i)) {
			DbgAssert(false);
			return NULL;
		}

		iterator layer = LayerIterator ( subtexmap_idx_to_lyr(i) );
		DbgAssert( layer != end() );
		return is_subtexmap_idx_tex(i) ? layer->Texture() : layer->Mask();
	}
	else {
		DbgAssert(false);
		return NULL;
	}
}

TSTR Composite::Texture::GetSubTexmapSlotName( int i ) {
	if (m_FileVersion == kMax2009) {
		if (false == is_subtexmap_idx_valid(i)) {
			DbgAssert(0);
			return TSTR();
		}

		iterator layer = LayerIterator ( subtexmap_idx_to_lyr(i) );
		DbgAssert( layer != end() );
		if (is_subtexmap_idx_tex(i))
			return TSTR( layer->ProcessName() );
		else
			return TSTR( layer->ProcessName() ) + TSTR( _T(" (Mask)") );
	}
	else {
		DbgAssert(0);
		return TSTR();
	}
}

RefResult Composite::Texture::NotifyRefChanged( Interval        changeInt,
											    RefTargetHandle hTarget,
											    PartID         &partID,
											    RefMessage      message )
{
	switch (message) {
	case REFMSG_CHANGE:
		// invalidate the texture
		m_Validity.SetEmpty();

		// invalidate the layer
		if (hTarget == m_ParamBlock) {
			int index = -1;
			//get which parameter is changing 
			ParamID changing_param = m_ParamBlock->LastNotifyParamID(index);
			if (index >= 0 && index < nb_layer()) {
				iterator it = LayerIterator(index);
				if (it != end())
					it->Invalidate();
			}
		}

		// Clean up the interactive renderer internals.
		m_ViewportIval.SetEmpty();
		// Empty the vector
		m_ViewportTextureList.swap( ViewportTextureListType() );

		// Recalculate the number of layers
		if (m_ParamBlock && nb_layer() != m_ParamBlock->Count( BlockParam::Texture ))
			UpdateParamBlock( m_ParamBlock->Count( BlockParam::Texture ) );

		// Re-show everything
		if (m_ParamDialog && (hTarget == m_ParamBlock)) {
			int index = -1;
			//get which parameter is changing and do specific ui updateing
			//if we can
			ParamID changing_param = m_ParamBlock->LastNotifyParamID(index);

			if (index >= 0 && index < m_ParamDialog->size() ) {
				Composite::Dialog::iterator it = m_ParamDialog->find(index);
				if (it != m_ParamDialog->end())
				{
					if (changing_param == BlockParam::Opacity) {
						if (index < m_ParamBlock->Count(BlockParam::Opacity))
							it->UpdateOpacity();
					}
					else if (changing_param == BlockParam::Texture) {
						if (index < m_ParamBlock->Count(BlockParam::Texture))
							it->UpdateTexture();
					}
					else if (changing_param == BlockParam::Mask) {
						if (index < m_ParamBlock->Count(BlockParam::Mask))
							it->UpdateMask();
					}
					else // Re-show everything
						m_ParamDialog->Invalidate();
				}
			}
			else
				m_ParamDialog->Invalidate();

		}
		break;

	case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			gpn->name = GetSubTexmapSlotName(gpn->index);
			return REF_HALT;
		}

	case REFMSG_SUBANIM_STRUCTURE_CHANGED:
		if (hTarget == m_ParamBlock && theHold.RestoreOrRedoing()) {
			for( Texture::iterator it  = begin(); it != end(); ++it )
				it->Invalidate();
		}
		break;

	}
	return(REF_SUCCEED);
}

// This map is meaningful iif all of its submaps are on.
bool Composite::Texture::IsLocalOutputMeaningful( ShadeContext& sc ) {
	struct {
		bool operator()( const value_type& l ) {
			return TRUE == l.IsVisible();
		}
	} predicate;

	return std::find_if( begin(), end(), predicate ) == end();
}

#pragma region Load/Save serializers

IOResult Composite::Texture::Save(ISave *ostream) {
	IOResult res( IO_OK );

	// Header and version to ensure that it is us. If those fields are missing,
	// loading will not work correctly.
	ostream->BeginChunk     ( Chunk::Header );
	res = MtlBase::Save  ( ostream );
	if (res != IO_OK) return res;
	ostream->EndChunk       ( );

	ULONG	nBytes = 0;
	ostream->BeginChunk     ( Chunk::Version );
	res = ostream->Write( &m_FileVersion, sizeof(m_FileVersion), &nBytes );	
	if (res != IO_OK) return res;
	ostream->EndChunk       ( );

	return res;
}

IOResult Composite::Texture::Load(ILoad *istream) {
	// Register a callback that will make sure the new layers have
	// all parameters.
	struct callback : public PostLoadCallback {
		Texture *m_Texture;

		callback( Texture* mtl )
			: m_Texture(mtl) {}

		void proc(ILoad *iload) {
			// Don't leak...  we are the sole responsible here.
			// Doing this prevents this class from leaking on exception.
			SmartPtr< callback > exception_guard( this );

			//FIX for 2008 to 2009 to ref structure
			//just need to copy owner ref maps to the paramblock and then drop the Texture's reference
			if (m_Texture->m_FileVersion == kMax2008) {
				int ct = m_Texture->m_oldMapList.Count();
				if (ct > 0) {
					m_Texture->UpdateParamBlock( ct );
					for (int i = 0; i < ct; i++) {
						Texmap *texmap = m_Texture->m_oldMapList[i];
						if (texmap) {
							m_Texture->GetParamBlock(0)->SetValue(BlockParam::Texture,0,texmap,i);	
							m_Texture->ReplaceReference(i+1, NULL);
						}
					}
				}
				else { // need at least 1 layer
					m_Texture->UpdateParamBlock( 1 );
				}

				m_Texture->GetParamBlock(0)->SetCount(BlockParam::Texture_DEPRECATED,0);
			}
			//FIX for JohnsonBeta to 2009 ref structure
			//just need to copy owner ref maps and masks to the paramblock
			else if (m_Texture->m_FileVersion == kJohnsonBeta) {
				int ct = m_Texture->m_OldMapCount_JohnsonBeta/2;
				m_Texture->UpdateParamBlock( ct );

				int maskID = ct;
				for (int i = 0; i < ct; i++) {
					if (i < m_Texture->m_oldMapList.Count()) {
						Texmap *texmap = m_Texture->m_oldMapList[i];
						if (texmap) {
							m_Texture->GetParamBlock(0)->SetValue(BlockParam::Texture,0,texmap,i);
							m_Texture->ReplaceReference(i+1, NULL);
						}
					}
					if (maskID < m_Texture->m_oldMapList.Count()) {
						Texmap *texmap = m_Texture->m_oldMapList[maskID];
						if (texmap) {
							m_Texture->GetParamBlock(0)->SetValue(BlockParam::Mask,0,texmap,i);	
							m_Texture->ReplaceReference(maskID+1, NULL);
						}
					}
					maskID++;
				}
				m_Texture->GetParamBlock(0)->SetCount(BlockParam::Texture_DEPRECATED,0);
				m_Texture->GetParamBlock(0)->SetCount(BlockParam::Mask_DEPRECATED,0);
			}

			DbgAssert(m_Texture->GetParamBlock(0)->Count(BlockParam::Texture_DEPRECATED) == 0);
			DbgAssert(m_Texture->GetParamBlock(0)->Count(BlockParam::Mask_DEPRECATED) == 0);
			
			m_Texture->m_oldMapList.SetCount(0);
			
			m_Texture->m_FileVersion = kMax2009;

			 //FIX*** for changing the opacity from int to float
			int count( m_Texture->GetParamBlock( 0 )->Count( BlockParam::Texture ) );

			//see if we have old integer based opacity values
			int oldOpacityParamCount = m_Texture->GetParamBlock( 0 )->Count( BlockParam::Opacity_DEPRECATED ) ;
			int opacityParamCount = m_Texture->GetParamBlock( 0 )->Count( BlockParam::Opacity ) ;
			if (opacityParamCount >= oldOpacityParamCount)
				oldOpacityParamCount = 0;

			if (oldOpacityParamCount > 0) {
				m_Texture->GetParamBlock( 0 )->SetCount( BlockParam::Opacity,oldOpacityParamCount ) ;
				//loop through each opac value
				for (int i = 0; i < oldOpacityParamCount; i++) {
					//see if it is animated if so we need to copy the controller
					Control *c =  m_Texture->GetParamBlock( 0 )->GetController(BlockParam::Opacity_DEPRECATED , i);
					if (c)
						m_Texture->GetParamBlock( 0 )->SetController(BlockParam::Opacity,i,c,FALSE);
					//otherwise we can just copy the value
					else {
						float opac = (float) m_Texture->GetParamBlock( 0 )->GetInt(BlockParam::Opacity_DEPRECATED,0,i);
						m_Texture->GetParamBlock( 0 )->SetValue(BlockParam::Opacity,0,opac,i);
					}
				}

				//make sure to remove those old values				
			}
			m_Texture->GetParamBlock( 0 )->SetCount( BlockParam::Opacity_DEPRECATED,0 ) ;

			
			//we have to make sure to update the pointer caches to the maps and masks are updated 
			m_Texture->UpdateParamBlock( count );

			int maskCount = m_Texture->m_ParamBlock->Count(BlockParam::Mask);			
			for(int i = 0; i < maskCount; i++) {
				 Texmap *map = NULL;
				 m_Texture->GetParamBlock(0)->GetValue(BlockParam::Mask,0,map,FOREVER,i);
				 m_Texture->LayerIterator(i)->Mask( map );
			}
			int mapCount = m_Texture->m_ParamBlock->Count(BlockParam::Texture);
			for(int i = 0; i < mapCount; i++) {
				Texmap *map = NULL;
				m_Texture->GetParamBlock(0)->GetValue(BlockParam::Texture,0,map,FOREVER,i);
				m_Texture->LayerIterator(i)->Texture( map );
			}

			m_Texture->NotifyChanged();
		}
	};

	IOResult res( IO_OK );
	bool     head_loaded( false );
	bool     compat_mode( false );

	//we set the version to JohnsonBeta since it did not have a flag
	// 2008 we can check by seeing if there is subtex count
	// 2009 will have the flag
	m_FileVersion = kJohnsonBeta;

	while( IO_OK == (res=istream->OpenChunk()) ) {
		int   chunk_id = istream->CurChunkID();
		ULONG tmp;

		switch( chunk_id ) {
		// Latest Version chunks
		case Chunk::Header:
			// Should not contain two headers or a new header
			// and a legacy tag.
			if (compat_mode || head_loaded) {
				return IO_ERROR;
			}
			istream->RegisterPostLoadCallback( new callback( this ) );

			res         = MtlBase::Load( istream );
			head_loaded = true;
			break;
		case Chunk::Version:   
			ULONG nb;
			res = istream->Read(&m_FileVersion, sizeof(m_FileVersion), &nb);
			break;
		// Legacy chunks
		case Chunk::Compatibility::Header:
			res         = MtlBase::Load( istream );
			head_loaded = true;
			compat_mode = true;
			break;
		case Chunk::Compatibility::SubTexCount: {
			compat_mode = true;
			int count;
			m_FileVersion = kMax2008;
			res = istream->Read( &count, sizeof(count), &tmp );
			m_OldMapCount_2008 = count;
			if (res != IO_OK) return res;

			istream->RegisterPostLoadCallback( new callback( this ) );
			break;
			}

		 default:
			 // Unknown chunk.  You may want to break here for debugging.
			 res = IO_OK;
		}

		if (res != IO_OK) return res;
		res = istream->CloseChunk();
		if (res != IO_OK) return res;
	}
	return IO_OK;
}

#pragma endregion

#pragma region Viewport Interactive Renderer

void Composite::Texture::ActivateTexDisplay(BOOL onoff) {}

// Init the interactive multi-map rendering
// Creates bitmaps from the sub-maps.  Modify those bitmaps to include
// the mask and opacity, then create handle and push it on the vector.
void Composite::Texture::InitMultimap( TimeValue  time,
									   MtlMakerCallback &callback ) {
	// Emptying the viewport vector.
	m_ViewportTextureList.swap( ViewportTextureListType() );

	// Determine how many textures we're going to show.
	int nb_tex( min<int>( int(CountVisibleLayer()),
		Param::LayerMaxShown,
		callback.NumberTexturesSupported() ) );

	iterator it( begin() );
	for( int i = 0; i < nb_tex && it != end(); ++it ) {
		if (it->IsVisible()) {
			const unsigned o   ( unsigned( it->NormalizedOpacity() * 255.0f ) );

			PBITMAPINFO bmi = it->Texture()->GetVPDisplayDIB(time, callback, GetValidity(), false);
			if(!bmi){
				continue;
			}
			unsigned char *data( (unsigned char*)bmi + sizeof( BITMAPINFOHEADER ) );
			const int      w   (bmi->bmiHeader.biWidth);
			const int      h   (bmi->bmiHeader.biHeight);

			if (it->IsMaskVisible()) {
				PBITMAPINFO pMaskBmi = it->Mask()->GetVPDisplayDIB(time, callback, GetValidity(), true, w, h);
				if(pMaskBmi){
					UBYTE* pMaskPixel = ((UBYTE *)((BYTE *)(pMaskBmi) + sizeof(BITMAPINFOHEADER)));
				for( int y = 0; y < h; ++y ) {
						for( int x = 0; x < w;  ++x,data += 4,pMaskPixel += 4 ) { 
							const unsigned long alpha(data[3]);
							const unsigned long mask ( pMaskPixel[0]); 
						data[3] = (unsigned char)( alpha * mask * o / 255 / 255 );
					}
				}
					free(pMaskBmi); // free the mask map bitmap.
				}

			} else {
				for( int y = 0; y < h; ++y ) {
					for( int x = 0; x < w; ++x, data += 4 ) { 
						const unsigned long alpha(data[3]);
						data[3] = (unsigned char)(alpha * o / 255 ); // alpha[0,255]* o[0,255] , so don't ">>8"
					}
				}
			}

			TexHandlePtr handle( callback.MakeHandle( bmi ) );
			m_ViewportTextureList.push_back( viewport_texture( handle, it ) );

			++i;
		}
	}
}

// Configure max to show the texture in DirectX.
void Composite::Texture::PrepareHWMultimap( TimeValue  time,
										    IHardwareMaterial *hw_material,
										    ::Material *material,
										    MtlMakerCallback &callback ) {
	hw_material->SetNumTexStages( DWORD(m_ViewportTextureList.size()) );

	ViewportTextureListType::iterator it( m_ViewportTextureList.begin() );
	for (int i = 0; it != m_ViewportTextureList.end(); ++it, ++i) {
		hw_material->SetTexture( i, it->handle()->GetHandle() );

		material->texture[0].useTex = i;
		Texmap* pSubMap = it->Layer()->Mask();
		if(!pSubMap){
			pSubMap = it->Layer()->Texture();
		}

		callback.GetGfxTexInfoFromTexmap( time, material->texture[0], pSubMap?pSubMap:this);

		// Set the operations and the arguments for the Texture
		if (0 == i)
			hw_material->SetTextureColorOp    ( i, D3DTOP_MODULATE );
		else
			hw_material->SetTextureColorOp    ( i, D3DTOP_BLENDTEXTUREALPHA );

		hw_material->SetTextureColorArg      ( i, 1, D3DTA_TEXTURE  );
		hw_material->SetTextureColorArg      ( i, 2, D3DTA_CURRENT  );

		hw_material->SetTextureAlphaOp       ( i, D3DTOP_SELECTARG2 );
		hw_material->SetTextureAlphaArg      ( i, 1, D3DTA_TEXTURE  );
		hw_material->SetTextureAlphaArg      ( i, 2, D3DTA_CURRENT  );

		hw_material->SetTextureTransformFlag ( i, D3DTTFF_COUNT2    );
	}
}

// Configure max to show the texture, either in OpenGL or software.
void Composite::Texture::PrepareSWMultimap( TimeValue  time,
										    ::Material *material,
										    MtlMakerCallback &callback ) {
	material->texture.SetCount( int(m_ViewportTextureList.size()) );

	ViewportTextureListType::iterator it( m_ViewportTextureList.begin() );
	for( int i = 0; it != m_ViewportTextureList.end(); ++i, ++it ) {
		Texmap* pSubMap = it->Layer()->Mask();
		if(!pSubMap){
			pSubMap = it->Layer()->Texture();
		}
		callback.GetGfxTexInfoFromTexmap( time, material->texture[i], pSubMap?pSubMap:this );
		material->texture[i].textHandle = it->handle()->GetHandle();

		material->texture[i].colorOp          = GW_TEX_ALPHA_BLEND;
		material->texture[i].colorAlphaSource = GW_TEX_TEXTURE;
		material->texture[i].colorScale       = GW_TEX_SCALE_1X;
		material->texture[i].alphaOp          = GW_TEX_MODULATE;
		material->texture[i].alphaAlphaSource = GW_TEX_TEXTURE;
		material->texture[i].alphaScale       = GW_TEX_SCALE_1X;
	}
}

void Composite::Texture::SetupGfxMultiMaps( TimeValue  t,
										    ::Material *mtl,
										    MtlMakerCallback &cb ) {
	// Are we still valid?  If not, initialize...
	if ( false == m_ViewportIval.InInterval(t) ) {
		// Set the interval and initialize the map.
		m_ViewportIval = GetValidity();
		InitMultimap( t, cb );
	}

	IHardwareMaterial *HWMaterial = (IHardwareMaterial *)GetProperty(PROPID_HARDWARE_MATERIAL);

	// Are we using hardware?
	if (HWMaterial)
		PrepareHWMultimap( t, HWMaterial, mtl, cb );
	else
		PrepareSWMultimap( t, mtl, cb );
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

Composite::PopLayerBeforeRestoreObject::PopLayerBeforeRestoreObject(Texture *owner, IParamBlock2 *paramblock, size_t layer_index) 
	: m_layer_index(layer_index), m_owner(owner), m_paramblock(paramblock)
{ }

void Composite::PopLayerBeforeRestoreObject::Redo() {
	Texture::iterator it = m_owner->LayerIterator ( m_layer_index );
	if (m_owner->ParamDialog())
		m_owner->ParamDialog()->DeleteRollup( &*it );

	m_owner->erase( it );
	// acquire new layer at index value
	it = m_owner->LayerIterator ( m_layer_index );

	// Update all indexes in the paramblock...
	for( ; it != m_owner->end(); ++it )
		it->Index( it->Index() - 1 );
}

void Composite::PopLayerBeforeRestoreObject::Restore(int isUndo) {
	if (isUndo) {
		// Create and push the new Layer on the list
		if (m_owner->ParamDialog()) {
			m_owner->ParamDialog()->EnableDisableInterface();
			Texture::iterator it = m_owner->LayerIterator ( m_layer_index );
			m_owner->ParamDialog()->CreateRollup( &*it );
		}

		m_owner->ValidateParamBlock();
	}
}

Composite::PopLayerAfterRestoreObject::PopLayerAfterRestoreObject(Texture *owner, IParamBlock2 *paramblock, size_t layer_index) 
	: m_layer_index(layer_index), m_owner(owner), m_paramblock(paramblock)
{ }

void Composite::PopLayerAfterRestoreObject::Restore(int isUndo) {
	if (isUndo) {
		Composite::Layer &new_layer( Composite::Layer( m_paramblock, m_layer_index ) );
		Texture::iterator it = m_owner->LayerIterator ( m_layer_index );
		m_owner->insert( it, new_layer );

		// Update all indexes in the paramblock...
		for( ; it != m_owner->end(); ++it )
			it->Index( it->Index() + 1 );
	}
}

void Composite::PopLayerAfterRestoreObject::Redo() {
	m_owner->ValidateParamBlock();

	if (m_owner->ParamDialog()) {
		// Re-wire the dialogs...
		m_owner->ParamDialog()->Rewire();
		m_owner->NotifyChanged();
	}
}

Composite::PushLayerRestoreObject::PushLayerRestoreObject(Texture *owner, IParamBlock2 *paramblock, bool setName) 
	: m_setName(setName), m_owner(owner), m_paramblock(paramblock)
{ }

void Composite::PushLayerRestoreObject::Restore(int isUndo) {
	if (isUndo) {
		Texture::iterator it = m_owner->LayerIterator ( m_owner->nb_layer()-1 );
		if (m_owner->ParamDialog())
			m_owner->ParamDialog()->DeleteRollup( &*it );

		m_owner->erase( it );
	}
}

void Composite::PushLayerRestoreObject::Redo() {
	// We create the Layer at the end...
	m_owner->push_back( Composite::Layer( m_paramblock, m_owner->nb_layer() ) );
	Composite::Layer &new_layer( *m_owner->LayerIterator( m_owner->nb_layer()-1 ) );

	if (m_setName)
		new_layer.Name( Layer::NameType( m_owner->DefaultLayerName().c_str() ) );

	// Create and push the new Layer on the list
	if (m_owner->ParamDialog()) {
		m_owner->ParamDialog()->EnableDisableInterface();
		m_owner->ParamDialog()->CreateRollup( &new_layer );
	}

	m_owner->ValidateParamBlock();
}

Composite::InsertLayerBeforeRestoreObject::InsertLayerBeforeRestoreObject(Texture *owner, IParamBlock2 *paramblock, size_t layer_index) 
	: m_layer_index(layer_index), m_owner(owner), m_paramblock(paramblock)
{ }

void Composite::InsertLayerBeforeRestoreObject::Redo() {
	Composite::Layer &new_layer( Composite::Layer( m_paramblock, m_layer_index ) );
	Texture::iterator it = m_owner->LayerIterator ( m_layer_index );
	m_owner->insert( it, new_layer );

	// Update all indexes in the paramblock...
	for( ; it != m_owner->end(); ++it )
		it->Index( it->Index() + 1 );
}

void Composite::InsertLayerBeforeRestoreObject::Restore(int isUndo) {
	if (isUndo) {
		Texture::iterator it = m_owner->LayerIterator ( m_layer_index );
		m_owner->erase( it );
		// acquire new layer at index value
		it = m_owner->LayerIterator ( m_layer_index );

		// Update all indexes in the paramblock...
		for( ; it != m_owner->end(); ++it )
			it->Index( it->Index() - 1 );

		m_owner->ValidateParamBlock();

		if (m_owner->ParamDialog()) {
			// Re-wire the dialogs...
			m_owner->ParamDialog()->Rewire();
			m_owner->NotifyChanged();
		}

	}
}

Composite::InsertLayerAfterRestoreObject::InsertLayerAfterRestoreObject(Texture *owner, IParamBlock2 *paramblock, size_t layer_index) 
	: m_layer_index(layer_index), m_owner(owner), m_paramblock(paramblock)
{ }

void Composite::InsertLayerAfterRestoreObject::Restore(int isUndo) {
	if (isUndo) {
		Texture::iterator it = m_owner->LayerIterator ( m_layer_index );
		if (m_owner->ParamDialog())
			m_owner->ParamDialog()->DeleteRollup( &*it );
	}
}

void Composite::InsertLayerAfterRestoreObject::Redo() {
	// Create and push the new Layer on the list
	if (m_owner->ParamDialog()) {
		m_owner->ParamDialog()->EnableDisableInterface();
		Texture::iterator it = m_owner->LayerIterator ( m_layer_index );
		m_owner->ParamDialog()->CreateRollup( &*it );
	}

	m_owner->ValidateParamBlock();
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////
