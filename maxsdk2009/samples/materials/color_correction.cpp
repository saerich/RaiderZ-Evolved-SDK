/**********************************************************************
 *<
   FILE: color_correction.cpp
   DESCRIPTION: A color correction Texture map.
   CREATED BY: Hans Larsen
   HISTORY: Created
 *>   Copyright (c) 2007, All Rights Reserved.
 **********************************************************************/
#pragma region Includes

#include "mtlhdr.h"
#include "mtlres.h"
#include "mtlresOverride.h"
#include "stdmat.h"
#include <bmmlib.h>
#include "iparamm2.h"
#include "macrorec.h"

#include "util.h"

#include <d3dx9.h>
#include "IHardwareMaterial.h"
#include "3dsmaxport.h"

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
*******************************************************************************/
#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Exports

namespace HSL {
   static const float MIN_COLOR_DELTA = 0.00001f;
   float CalcHue( const Color& c ) {
      const float min_ ( ::min(c.r,c.g,c.b) );
      const float max_ ( ::max(c.r,c.g,c.b) );
      const float delta( max_ - min_ );
      if ( delta < MIN_COLOR_DELTA )
      {
         return 0.0f;
      }

      float h;
      if (c.r == max_)
      {
         h = (c.g - c.b) / delta;
      }
      else if (c.g == max_)
      {
         h = 2 + (c.b - c.r) / delta;
      }
      else 
      {
         h = 4 + (c.r - c.g) / delta;
      }

      return rotate( h * 60.0f, 0.0f, 360.0f );
   }
   float CalcLum( const Color& c ) {
      return clamp( (::max(c.r,c.g,c.b) + ::min(c.r,c.g,c.b)) / 2, 0, 1 );
   }
   float CalcSat( const Color& c ) {
      const float min_( ::min(c.r,c.g,c.b) );
      const float max_( ::max(c.r,c.g,c.b) );
      float lum_( CalcLum(c) );
      if (min_ == max_ || lum_ == 0)
         return 0;
      else
         if (lum_ <= 0.5f)
            return clamp((max_ - min_) / (2.0f * lum_), 0, 1);
         else
            return clamp((max_ - min_) / (2.0f - 2.0f * lum_), 0, 1);
   }
}

Color RGBtoHSL( const Color& in ) {
   return Color( HSL::CalcHue( in ), HSL::CalcSat( in ), HSL::CalcLum( in ) );
}

Color HSLtoRGB( const Color& in ) {
   const struct {
      float operator()( float v1, float v2, float H ) const {
         const float tmpH( rotate( H, 0.0f, 1.0f ) );

              if ( tmpH < 1.0f/6.0f ) return v1 + (v2 - v1) * 6 * tmpH;
         else if ( tmpH < 1.0f/2.0f ) return v2;
         else if ( tmpH < 2.0f/3.0f ) return v1 + (v2 - v1) * ((2.0/3.0)-tmpH) * 6;
         else                         return v1;
      }
   } hue_to_rgb;

   const float H( in[0] );
   const float S( in[1] );
   const float L( in[2] );

   const float Q ( L < 0.5f ? (L * (1.0f + S)) 
                            : (L + S - L*S) );
   const float P ( 2.0f * L - Q );
   const float Hk( H / 360.0f );

   return Color( clamp( hue_to_rgb( P, Q, Hk + 1.0f/3.0f ), 0, 1 ),
                 clamp( hue_to_rgb( P, Q, Hk             ), 0, 1 ),
                 clamp( hue_to_rgb( P, Q, Hk - 1.0f/3.0f ), 0, 1 ) );
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////
#ifndef NO_MAPTYPE_COLORCORRECTION

#pragma region Constants

// ColorCorrection namespace and all internally used global variables.
namespace ColorCorrection {

   namespace BlockParam {
      // Param IDs
      enum { 
         // pblock ID
         ID           = 0
      };

      // Rollouts
      enum {
         ChannelRollout,
         TextureRollout,
         ColorRollout,
         LightnessStandardRollout,
         LightnessAdvancedRollout_NO_LONGER_USED //all these parameters have been moved into the lightness standard rollup to make it more stable
      };

      // Parameters
      enum {
         // Texture
         DefaultColor,
         Texture,

         // Rewiring
         RewireMode,
         RewireRed_Custom,
         RewireGreen_Custom,
         RewireBlue_Custom,
         RewireAlpha_Custom,

         // Color
         HueShift,
         Saturation,
         HueTint,
         HueTintStrength,

         // Lightness
         LightnessMode,

         // Lightness -> Standard
         Brightness,
         Contrast,

         // Lightness -> Advanced
         ExposureMode,
         EnableR,
         EnableG,
         EnableB,
         RGBGain,
         RGain,
         GGain,
         BGain,
         RGBGamma,
         RGamma,
         GGamma,
         BGamma,
         RGBPivot,
         RPivot,
         GPivot,
         BPivot,
         RGBLift,
         RLift,
         GLift,
         BLift,
         Printer
      };
   }

   namespace ExposureMode {
      enum {
         Gain = 0,
         FStops,
         Printer
      };
   }

   namespace RewireType {
      enum {
         Normal,
         Monochrome,
         Invert,
         Custom
      };
   }

   // Indices of the values inside the combo
   namespace RewireCombo {
      enum  {
         Red = 0,
         Green,
         Blue,
         Alpha,
         RedInverse,
         GreenInverse,
         BlueInverse,
         AlphaInverse,
         Mono,
         One,
         Zero
      };
   }

   // Default values for new layers
   namespace Default {
      namespace Rewire {
         enum  {
            Red   = RewireCombo::Red,
            Green = RewireCombo::Green,
            Blue  = RewireCombo::Blue,
            Alpha = RewireCombo::Alpha
         } ;
      };
   }
}

//this contains our information to rewire to different color channels
class RewireData
{
public:
	int mR,mG,mB,mA;

};

////////////////////////////////////////////////////////////////////////
// Static assertions
// These are going to generate compilation error if they are not valid.

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Channel Rewiring

namespace ColorCorrection {
   namespace Rewire {
      struct RewireBase : std::unary_function< const AColor&, float > {
         virtual float    operator()( const AColor& color ) const = 0;
         virtual _tstring Name()                            const = 0;
      };

      template< typename SubType, int ResourceID >
         struct RewireHelper : RewireBase {
            enum { NameResourceID = ResourceID };

            static const RewireBase* i() { 
               static SubType instance;
               return &instance;
            }

            _tstring Name() const { return _tstring( GetString( ResourceID ) ); }
         };

      struct Red : public RewireHelper< Red, IDS_CC_COMBO_RED > {
         float operator()( const AColor& color ) const
            { return color.r; }
      };
      struct Green : public RewireHelper< Green, IDS_CC_COMBO_GREEN > {
         float operator()( const AColor& color ) const
            { return color.g; }
      };
      struct Blue : public RewireHelper< Blue, IDS_CC_COMBO_BLUE > {
         float operator()( const AColor& color ) const
            { return color.b; }
      };
      struct Alpha : public RewireHelper< Alpha, IDS_CC_COMBO_ALPHA > {
         float operator()( const AColor& color ) const
            { return color.a; }
      };

      // Inverse functions.
      struct RedInverse : public RewireHelper< RedInverse, IDS_CC_COMBO_RED_INV > {
         float operator()( const AColor& color ) const
            { return 1.0f - color.r; }
      };
      struct GreenInverse : public RewireHelper< GreenInverse, IDS_CC_COMBO_GREEN_INV > {
         float operator()( const AColor& color ) const
            { return 1.0f - color.g; }
      };
      struct BlueInverse : public RewireHelper< BlueInverse, IDS_CC_COMBO_BLUE_INV > {
         float operator()( const AColor& color ) const
            { return 1.0f - color.b; }
      };
      struct AlphaInverse : public RewireHelper< AlphaInverse, IDS_CC_COMBO_ALPHA_INV > {
         float operator()( const AColor& color ) const
            { return 1.0f - color.a; }
      };

      // Mono
      struct Mono : public RewireHelper< Mono, IDS_CC_COMBO_MONO > {
         float operator()( const AColor& color ) const
            { return (color.r + color.g + color.b) / 3.0f; }
      };

      // One and Zero
      struct One : public RewireHelper< One, IDS_CC_COMBO_ONE > {
         float operator()( const AColor& color ) const
            { return 1.0f; }
      };
      struct Zero : public RewireHelper< Zero, IDS_CC_COMBO_ZERO > {
         float operator()( const AColor& color ) const
            { return 0.0f; }
      };

      /////////////////////////////////////////////////////////////////////////////
      // Creating and managing the list.
      const RewireBase* const RewireArray[] = {
         Red::i(), Green::i(), Blue::i(), Alpha::i(),
         RedInverse::i(), GreenInverse::i(), BlueInverse::i(), AlphaInverse::i(),
         Mono::i(), 
         One::i(), Zero::i()
      };
      struct RewireListType {
         typedef const RewireBase* const * const_iterator;
         const RewireBase& operator []( int Index ) const {
            return *(RewireArray[ Index ]);
         }
         const_iterator begin() const { return &RewireArray[0]; }
         const_iterator end()   const { return &RewireArray[size()]; }
         size_t         size()  const { return sizeof(RewireArray)/sizeof(*RewireArray); }
      private:
      } RewireList;
   }
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Lightness

namespace ColorCorrection {
   // Predicates for evaluating the Lightness correction in Standard or
   // Advanced mode.
   // You pass it a PBlockHelper in the constructor and it fetches the values
   // inside the parameter block.
   namespace Lightness {

      class Standard : std::unary_function< float, float > {
         float m_Brightness;
         float m_Contrast;

      public:
         Standard( float b, float c ) 
            : m_Brightness( b )
            , m_Contrast  ( c ) 
         {}

         Standard( IParamBlock2* pb, TimeValue  t    = 0 ) 
            : m_Brightness( pb->GetFloat( BlockParam::Brightness, t ) / 100.0f )
            , m_Contrast  ( pb->GetFloat( BlockParam::Contrast,   t ) / 100.0f ) 
         {}

         float operator()( float value ) const {
            return   (value - 0.5f) * (1.0f + m_Contrast)
                   + 0.5f
                   + m_Brightness;
         }
      };

      // This one takes a template parameter which contains enumeration of the
      // IDs to fetch from the parameter block.
      // See the RGB, R, G and B structures below.
      template< typename BPHelper >
         class Advanced : std::unary_function< float, float > {
            float    m_Gain;
            float    m_Gamma;
            float    m_Pivot;
            float    m_Lift;
            
            int      m_Exp;
            float    m_Printer;

         public:
            Advanced( IParamBlock2* pb, TimeValue  t = 0 ) 
               : m_Gain    ( pb->GetFloat( BPHelper::Gain,  t ) / 100.0f )
               , m_Gamma   ( pb->GetFloat( BPHelper::Gamma, t ) )
               , m_Pivot   ( pb->GetFloat( BPHelper::Pivot, t ) )
               , m_Lift    ( pb->GetFloat( BPHelper::Lift,  t ) )
               , m_Exp     ( pb->GetInt  ( BlockParam::ExposureMode, t ) )
               , m_Printer ( pb->GetFloat( BlockParam::Printer, t ) )
            {}

            float operator()( float value ) const {
               switch( m_Exp ) {
                  case ExposureMode::Gain:
                        return   m_Pivot 
                               * pow( value * m_Gain / m_Pivot, 
                                      1.0f / m_Gamma ) 
                               + m_Lift;
                  case ExposureMode::FStops:
                        return   m_Pivot 
                               * pow( value * pow( 2.0f, m_Gain ) / m_Pivot, 
                                      1.0f / m_Gamma ) 
                               + m_Lift;
                  case ExposureMode::Printer:
                        return   m_Pivot 
                               * pow( value * pow( m_Printer, m_Gain ) / m_Pivot, 
                                      1.0f / m_Gamma ) 
                               + m_Lift;
               }
               // Else, return all black.
               return 0.0f;
            }
         };

      // BlockParam helpers
      struct RGB { enum {
            Gain  = ColorCorrection::BlockParam::RGBGain,
            Gamma = BlockParam::RGBGamma,
            Pivot = BlockParam::RGBPivot,
            Lift  = BlockParam::RGBLift
      }; };

      struct R { enum {
            Gain  = BlockParam::RGain,
            Gamma = BlockParam::RGamma,
            Pivot = BlockParam::RPivot,
            Lift  = BlockParam::RLift
      }; };

      struct G { enum {
            Gain  = BlockParam::GGain,
            Gamma = BlockParam::GGamma,
            Pivot = BlockParam::GPivot,
            Lift  = BlockParam::GLift
      }; };

      struct B { enum {
            Gain  = BlockParam::BGain,
            Gamma = BlockParam::BGamma,
            Pivot = BlockParam::BPivot,
            Lift  = BlockParam::BLift
      }; };

   }
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Parameter Block
namespace ColorCorrection { namespace {
   class TextureAccessorClass : public PBAccessor
   {
   public:
      void Set( PB2Value      & v, 
                ReferenceMaker* owner, 
                ParamID         id, 
                int             tabIndex, 
                TimeValue       t );
   };

   class ChannelAccessorClass : public PBAccessor
   {
   public:
      void Set( PB2Value      & v, 
                ReferenceMaker* owner, 
                ParamID         id, 
                int             tabIndex, 
                TimeValue       t );
   };

   class ColorAccessorClass : public PBAccessor
   {
   public:
      void Set( PB2Value      & v, 
                ReferenceMaker* owner, 
                ParamID         id, 
                int             tabIndex, 
                TimeValue       t );
   };

   class LightnessAccessorClass : public PBAccessor
   {
   public:
      void Set( PB2Value      & v, 
                ReferenceMaker* owner, 
                ParamID         id, 
                int             tabIndex, 
                TimeValue       t );
   };

   TextureAccessorClass    TextureAccessor;
   ChannelAccessorClass    ChannelAccessor;
   ColorAccessorClass      ColorAccessor;
   LightnessAccessorClass  LightnessAccessor;
} }

// Parameter block (module private)
namespace ColorCorrection { namespace {
   /////////////////////////////////////////////////////////////////////////////
   // Main param block
   ParamBlockDesc2 ParamBlock ( 
      BlockParam::ID,   _T("parameters"),            // ID, int_name
                        0,                           // local_name
                        GetColorCorrectionDesc(),    // ClassDesc
                        P_AUTO_UI 
                      | P_MULTIMAP
                      | P_AUTO_CONSTRUCT 
                      | P_VERSION,                   // flags
                        1,                           // version
                        0,                           // construct_id

      // Rollouts      
      4,
      BlockParam::TextureRollout, IDD_CC_TEXTURE,
                                  IDS_DS_CC_TEXTURE_TITLE,
                                  0, 0,
                                  0,
      BlockParam::ChannelRollout, IDD_CC_CHANNELS, 
                                  IDS_DS_CC_CHANNELS_TITLE,
                                  0, 0,
                                  0,
      BlockParam::ColorRollout,   IDD_CC_COLOR,
                                  IDS_DS_CC_COLOR_TITLE,
                                  0, 0,
                                  0,
      BlockParam::LightnessStandardRollout, IDD_CC_LIGHTNESS_STANDARD,
                                  IDS_DS_CC_LIGHTNESS_TITLE,
                                  0, 0,
                                  0,

      //////////////////////////////////////////////////////////////////////////
      // params
      BlockParam::DefaultColor, _T("color"),          // ID, int_name
         TYPE_FRGBA,                                  // Type
         P_ANIMATABLE,                                // flags
         IDS_DS_COLOR1,                               // localized Name
         p_ui,                BlockParam::TextureRollout,
                              TYPE_COLORSWATCH,
                              IDC_CC_TEXTURE_COLOR,
         p_accessor,         &TextureAccessor,
         p_tooltip,           IDS_CC_TOOLTIP_COLOR,
      end,
      BlockParam::Texture,    _T("map"),              // ID, int_name
         TYPE_TEXMAP,                                 // Type
         P_OWNERS_REF,                                // flags
         IDS_DS_TEXMAP,                               // localized Name
	   	p_refno,		         1,
         p_ui,                BlockParam::TextureRollout,
                              TYPE_TEXMAPBUTTON,
                              IDC_CC_TEXTURE,
         p_accessor,         &TextureAccessor,
         p_tooltip,           IDS_CC_TOOLTIP_TEXTURE,
      end,

      // RewireMode Rewiring
      BlockParam::RewireMode, _T("rewireMode"),       // ID, int_name
         TYPE_INT,                                    // Type
         P_ANIMATABLE,                                // flags
         IDS_DS_REWIRE_MODE,                          // localized Name
         p_ui,                BlockParam::ChannelRollout,
                              TYPE_RADIO, 4,
                              IDC_CHANNEL_RADIO_NORMAL,
                              IDC_CHANNEL_RADIO_MONO,
                              IDC_CHANNEL_RADIO_INVERT,
                              IDC_CHANNEL_RADIO_CUSTOM,
         p_default,           0,
         p_accessor,         &ChannelAccessor,
         p_tooltip,           IDS_CC_TOOLTIP_REWIRE_MODE,
      end,
      BlockParam::RewireRed_Custom,  _T("rewireR"),          // ID, int_name
         TYPE_INT,                                    // Type
         P_ANIMATABLE,                                // flags
         IDS_DS_REWIRE_RED,                           // localized Name
         p_ui,                BlockParam::ChannelRollout,
                              TYPE_INT_COMBOBOX, 
                              IDC_COMBO_REWIRE_RED, Rewire::RewireList.size(),
                              Rewire::Red::NameResourceID,
                              Rewire::Green::NameResourceID,
                              Rewire::Blue::NameResourceID,
                              Rewire::Alpha::NameResourceID,
                              Rewire::RedInverse::NameResourceID,
                              Rewire::GreenInverse::NameResourceID,
                              Rewire::BlueInverse::NameResourceID,
                              Rewire::AlphaInverse::NameResourceID,
                              Rewire::Mono::NameResourceID,
                              Rewire::One::NameResourceID,
                              Rewire::Zero::NameResourceID,
         p_vals,              0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
         p_default,           Default::Rewire::Red,
         p_accessor,         &ChannelAccessor,
         p_tooltip,           IDS_CC_TOOLTIP_REWIRE_RED,
      end,
      BlockParam::RewireGreen_Custom,_T("rewireG"),          // ID, int_name
         TYPE_INT,                                    // Type
         P_ANIMATABLE,                                // flags
         IDS_DS_REWIRE_GREEN,                         // localized Name
         p_ui,                BlockParam::ChannelRollout,
                              TYPE_INT_COMBOBOX, 
                              IDC_COMBO_REWIRE_GREEN, Rewire::RewireList.size(),
                              Rewire::Red::NameResourceID,
                              Rewire::Green::NameResourceID,
                              Rewire::Blue::NameResourceID,
                              Rewire::Alpha::NameResourceID,
                              Rewire::RedInverse::NameResourceID,
                              Rewire::GreenInverse::NameResourceID,
                              Rewire::BlueInverse::NameResourceID,
                              Rewire::AlphaInverse::NameResourceID,
                              Rewire::Mono::NameResourceID,
                              Rewire::One::NameResourceID,
                              Rewire::Zero::NameResourceID,
         p_vals,              0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
         p_default,           Default::Rewire::Green,
         p_accessor,         &ChannelAccessor,
         p_tooltip,           IDS_CC_TOOLTIP_REWIRE_GREEN,
      end,
      BlockParam::RewireBlue_Custom, _T("rewireB"),          // ID, int_name
         TYPE_INT,                                    // Type
         P_ANIMATABLE,                                // flags
         IDS_DS_REWIRE_BLUE,                          // localized Name
         p_ui,                BlockParam::ChannelRollout,
                              TYPE_INT_COMBOBOX, 
                              IDC_COMBO_REWIRE_BLUE, Rewire::RewireList.size(),
                              Rewire::Red::NameResourceID,
                              Rewire::Green::NameResourceID,
                              Rewire::Blue::NameResourceID,
                              Rewire::Alpha::NameResourceID,
                              Rewire::RedInverse::NameResourceID,
                              Rewire::GreenInverse::NameResourceID,
                              Rewire::BlueInverse::NameResourceID,
                              Rewire::AlphaInverse::NameResourceID,
                              Rewire::Mono::NameResourceID,
                              Rewire::One::NameResourceID,
                              Rewire::Zero::NameResourceID,
         p_vals,              0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
         p_default,           Default::Rewire::Blue,
         p_accessor,         &ChannelAccessor,
         p_tooltip,           IDS_CC_TOOLTIP_REWIRE_BLUE,
      end,
      BlockParam::RewireAlpha_Custom,_T("rewireA"),          // ID, int_name
         TYPE_INT,                                    // Type
         P_ANIMATABLE,                                // flags
         IDS_DS_REWIRE_ALPHA,                         // localized Name
         p_ui,                BlockParam::ChannelRollout,
                              TYPE_INT_COMBOBOX, 
                              IDC_COMBO_REWIRE_ALPHA, Rewire::RewireList.size(),
                              Rewire::Red::NameResourceID,
                              Rewire::Green::NameResourceID,
                              Rewire::Blue::NameResourceID,
                              Rewire::Alpha::NameResourceID,
                              Rewire::RedInverse::NameResourceID,
                              Rewire::GreenInverse::NameResourceID,
                              Rewire::BlueInverse::NameResourceID,
                              Rewire::AlphaInverse::NameResourceID,
                              Rewire::Mono::NameResourceID,
                              Rewire::One::NameResourceID,
                              Rewire::Zero::NameResourceID,
         p_vals,              0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
         p_default,           Default::Rewire::Alpha,
         p_accessor,         &ChannelAccessor,
         p_tooltip,           IDS_CC_TOOLTIP_REWIRE_ALPHA,
      end,

      //////////////////////////////////////////////////////////////////////////
      // Color
      BlockParam::HueShift,   _T("hueShift"),
         TYPE_FLOAT,
         P_ANIMATABLE,
         IDS_DS_HUE_SHIFT,
         p_ui,                BlockParam::ColorRollout,
                              TYPE_SLIDER,
                              EDITTYPE_FLOAT,
                              IDC_COLOR_HUE_EDIT,
                              IDC_COLOR_HUE_SLIDER,
                              1,
         p_default,           0.0f,
         p_range,             -180.0f, 180.0f,
         p_accessor,         &ColorAccessor,
         p_tooltip,           IDS_CC_TOOLTIP_HUESHIFT,
      end,
      BlockParam::Saturation, _T("saturation"),
         TYPE_FLOAT,
         P_ANIMATABLE,
         IDS_DS_SATURATION,
         p_ui,                BlockParam::ColorRollout,
                              TYPE_SLIDER,
                              EDITTYPE_FLOAT,
                              IDC_COLOR_SAT_EDIT,
                              IDC_COLOR_SAT_SLIDER,
                              1,
         p_default,           0.0f,
         p_range,             -100.0f, 100.0f,
         p_accessor,         &ColorAccessor,
         p_tooltip,           IDS_CC_TOOLTIP_SATURATION,
      end,
      BlockParam::HueTint,    _T("tint"),
         TYPE_FRGBA,
         P_ANIMATABLE,
         IDS_DS_HUE_TINT,
         p_ui,                BlockParam::ColorRollout,
                              TYPE_COLORSWATCH,
                              IDC_COLOR_TINT_COLOR,
         p_accessor,         &ColorAccessor,
         p_tooltip,           IDS_CC_TOOLTIP_HUETINT,
      end,
      BlockParam::HueTintStrength, _T("tintStrength"),
         TYPE_FLOAT,
         P_ANIMATABLE,
         IDS_DS_HUE_STRENGTH,
         p_ui,                BlockParam::ColorRollout,
                              TYPE_SPINNER,
                              EDITTYPE_FLOAT,
                              IDC_COLOR_STRENGTH,
                              IDC_COLOR_STRENGTH_SPIN,
                              1.0f,
         p_default,           0.0f,
         p_range,             0.0f, 100.0f,
         p_accessor,         &ColorAccessor,
         p_tooltip,           IDS_CC_TOOLTIP_HUETINT_STRENGTH,
      end,

      //////////////////////////////////////////////////////////////////////////
      // Lightness - Mode
      BlockParam::LightnessMode, _T("lightnessMode"),
         TYPE_INT,
         0,
         0,
		 p_range,		0,	1,
		 p_ui,			BlockParam::LightnessStandardRollout, TYPE_RADIO, 2, IDC_LIGHTNESS_STANDARD, IDC_LIGHTNESS_ADVANCED,
		 p_default,           0,

		 p_accessor,   &LightnessAccessor,
      end,

      //////////////////////////////////////////////////////////
      // Lightness - Standard
      // Helper for easing declaration
      #undef DECLARE_HELPER__
      #define DECLARE_HELPER__( id, name, constant ) \
      BlockParam::id, _T(name), TYPE_FLOAT, P_ANIMATABLE, IDS_DS_LIGHTNESS_##constant, \
         p_ui,          BlockParam::LightnessStandardRollout, \
                        TYPE_SLIDER, EDITTYPE_FLOAT, \
                        IDC_LIGHTNESS_##constant##_EDIT, IDC_LIGHTNESS_##constant, 1, \
         p_default,     0.0f, \
         p_range,       -100.0f, 100.0f, \
         p_accessor,   &LightnessAccessor, \
         p_tooltip,     IDS_CC_TOOLTIP_##constant, \
      end,

      DECLARE_HELPER__( Contrast,   "contrast",    CONTRAST  )
      DECLARE_HELPER__( Brightness, "brightness",  BRIGHTNESS )

      #undef DECLARE_HELPER__      

      //////////////////////////////////////////////////////////
      // Lightness - Advanced
      BlockParam::ExposureMode,  _T("exposureMode"),
         TYPE_INT,
         0,
         IDS_DS_LIGHTNESS_MODE,
         p_ui,                BlockParam::LightnessStandardRollout,
                              TYPE_INT_COMBOBOX, 
                              IDC_LIGHTNESS_COMBO, 3,
                              IDS_CC_LIGHTNESS_GAIN,
                              IDS_CC_LIGHTNESS_FSTOP,
                              IDS_CC_LIGHTNESS_PRINTER_LIGHTS,
         p_vals,              0, 1, 2,
         p_default,           0,
         p_accessor,         &LightnessAccessor,
         p_tooltip,           IDS_CC_TOOLTIP_EXPOSURE_MODE,
      end,

      BlockParam::EnableR,       _T("enableR"), 
         TYPE_BOOL, 
         P_ANIMATABLE, 
         IDS_DS_LIGHTNESS_ENABLE_R,
         p_ui,          BlockParam::LightnessStandardRollout, 
                        TYPE_SINGLECHEKBOX, 
                        IDC_LIGHTNESS_ENABLE_R,
         p_default,     false,
         p_accessor,   &LightnessAccessor,
         p_tooltip,     IDS_CC_TOOLTIP_ENABLE_R,
      end,
      BlockParam::EnableG,       _T("enableG"), 
         TYPE_BOOL, 
         P_ANIMATABLE, 
         IDS_DS_LIGHTNESS_ENABLE_G,
         p_ui,          BlockParam::LightnessStandardRollout, 
                        TYPE_SINGLECHEKBOX, 
                        IDC_LIGHTNESS_ENABLE_G,
         p_default,     false,
         p_accessor,   &LightnessAccessor,
         p_tooltip,     IDS_CC_TOOLTIP_ENABLE_G,
      end,
      BlockParam::EnableB,       _T("enableB"), 
         TYPE_BOOL, 
         P_ANIMATABLE, 
         IDS_DS_LIGHTNESS_ENABLE_B,
         p_ui,          BlockParam::LightnessStandardRollout, 
                        TYPE_SINGLECHEKBOX, 
                        IDC_LIGHTNESS_ENABLE_B,
         p_default,     false,
         p_accessor,   &LightnessAccessor,
         p_tooltip,     IDS_CC_TOOLTIP_ENABLE_B,
      end,

      // Helper for easing declaration
      #undef DECLARE_HELPER__
      #define DECLARE_HELPER__( id, name, constant, default, min, max, step ) \
      BlockParam::id, _T(name), TYPE_FLOAT, P_ANIMATABLE, IDS_DS_LIGHTNESS_##constant, \
         p_ui,          BlockParam::LightnessStandardRollout, \
                        TYPE_SPINNER, EDITTYPE_FLOAT, \
                        IDC_LIGHTNESS_##constant, IDC_LIGHTNESS_##constant##_SPIN, step, \
         p_default,     default, \
         p_range,       min, max, \
         p_accessor,   &LightnessAccessor, \
         p_tooltip,     IDS_CC_TOOLTIP_##constant, \
      end,

      DECLARE_HELPER__( RGBGain,   "gainRGB",   GAIN_RGB,   100.0f, -1000.00f,   1000.0f, 0.10f )
      DECLARE_HELPER__( RGain,     "gainR",     GAIN_R,     100.0f, -1000.00f,   1000.0f, 0.10f )
      DECLARE_HELPER__( GGain,     "gainG",     GAIN_G,     100.0f, -1000.00f,   1000.0f, 0.10f )
      DECLARE_HELPER__( BGain,     "gainB",     GAIN_B,     100.0f, -1000.00f,   1000.0f, 0.10f )

      DECLARE_HELPER__( RGBGamma,  "gammaRGB",  GAMMA_RGB,    1.0f,     0.01f, 100000.0f, 0.01f )
      DECLARE_HELPER__( RGamma,    "gammaR",    GAMMA_R,      1.0f,     0.01f, 100000.0f, 0.01f )
      DECLARE_HELPER__( GGamma,    "gammaG",    GAMMA_G,      1.0f,     0.01f, 100000.0f, 0.01f )
      DECLARE_HELPER__( BGamma,    "gammaB",    GAMMA_B,      1.0f,     0.01f, 100000.0f, 0.01f )

      DECLARE_HELPER__( RGBPivot,  "pivotRGB",  PIVOT_RGB,    1.0f,     0.01f, 100000.0f, 0.01f )
      DECLARE_HELPER__( RPivot,    "pivotR",    PIVOT_R,      1.0f,     0.01f, 100000.0f, 0.01f )
      DECLARE_HELPER__( GPivot,    "pivotG",    PIVOT_G,      1.0f,     0.01f, 100000.0f, 0.01f )
      DECLARE_HELPER__( BPivot,    "pivotB",    PIVOT_B,      1.0f,     0.01f, 100000.0f, 0.01f )

      DECLARE_HELPER__( RGBLift,   "liftRGB",   LIFT_RGB,     0.0f,    -2.00f,      2.0f, 0.01f )
      DECLARE_HELPER__( RLift,     "liftR",     LIFT_R,       0.0f,    -2.00f,      2.0f, 0.01f )
      DECLARE_HELPER__( GLift,     "liftG",     LIFT_G,       0.0f,    -2.00f,      2.0f, 0.01f )
      DECLARE_HELPER__( BLift,     "liftB",     LIFT_B,       0.0f,    -2.00f,      2.0f, 0.01f )

      DECLARE_HELPER__( Printer,   "printerLights", PRINT,    5.0f,   -40.00f,   6000.0f, 0.01f )

      #undef DECLARE_HELPER__
   end );
} }
#pragma endregion

////////////////////////////////////////////////////////////////////////////////


// Define this here because the IRollupCallback interface doesn't declare neither
// a virtual destructor nor a DeleteThis function...
class RollupCallback : public IRollupCallback 
{

   BOOL HandleDrop( IRollupPanel *src, IRollupPanel *dst, bool before );
};


BOOL RollupCallback::HandleDrop( IRollupPanel *src, IRollupPanel *dst, bool before ) 
{
	   if (0 == GetColorCorrectionDesc()->GetMParamDlg())
		   return FALSE;

	   // Let the system handle it if we're not editing color correction.
	   ReferenceTarget* thing( GetColorCorrectionDesc()->GetMParamDlg()->GetThing() );
	   return (thing && (thing->ClassID() == GetColorCorrectionDesc()->ClassID())) ? TRUE : FALSE;
}



////////////////////////////////////////////////////////////////////////////////

#pragma region Texture

namespace ColorCorrection {

   class LightnessProc;

   class Texture : public Texmap {
   public:
      //////////////////////////////////////////////////////////////////////////
      // ctor & dtor
       Texture();
      ~Texture() { }

      // MtlBase virtuals
      ParamDlg* CreateParamDlg(HWND editor_wnd, IMtlParams *imp);

      // Texmap virtuals
      AColor EvalColor(ShadeContext& sc);
      AColor Correct( const AColor& c, TimeValue t );

      Point3 EvalNormalPerturb(ShadeContext& sc) {
         if (m_Map)
            return m_Map->EvalNormalPerturb(sc);
         else
            return Point3(0, 0, 0);
      }
      bool   IsLocalOutputMeaningful( ShadeContext& sc ) { 
         if (m_Map)
            return m_Map->IsLocalOutputMeaningful(sc); 
         else
            return false;
      }

      int NumSubs() { return 2; }
      Animatable*    SubAnim(int i) { 
         if (i == 0)
            return m_ParamBlock;
         else
            return GetSubTexmap(i-1); 
      }
      TSTR SubAnimName(int i) {
         if (i == 0)
            return TSTR(GetString(IDS_DS_PARAMETERS));
         else
            return GetSubTexmapTVName( i - 1 );
      }


      void Update(TimeValue t, Interval& valid) { 
         if (!m_Validity.InInterval(t)) {
            m_Validity.SetInfinite();

            m_ParamBlock->GetValidity( t, m_Validity );

            if (m_Map)
               m_Map->Update( t, m_Validity );

			UpdateChannelRewiring( t );

            m_ViewportIval.SetEmpty();  //TO DO this could force excessive viewport recomputation since in some cases it could not be instant???
         }

         valid &= m_Validity;
      }

      Interval Validity(TimeValue t) { 
         Interval v;
         Update(t,v);
         return m_Validity;
      }

      // From Submap
      void SetSubTexmap( int i, Texmap *m ) { 
         if( i == 0 ) {
            ReplaceReference(1,m);
            m_Validity.SetEmpty();
            m_ViewportIval.SetEmpty();
            GetColorCorrectionDesc()->GetParamBlockDesc(0)->InvalidateUI( BlockParam::Texture );
         }
      }

      Texmap* GetSubTexmap( int i ) { 
            if( i == 0 )
               return (Texmap*)GetReference(1); 
            
            return 0;
         }

      int NumSubTexmaps() { 
         return 1; 
      }

      // Class management
      void Init();
      void Reset();

	  // Accessors
	  LightnessProc*	ParamDialog( )					 const { return m_LightProc; }
	  void				ParamDialog(LightnessProc* v)          { m_LightProc = v; }

      Class_ID    ClassID()             { return GetColorCorrectionDesc()->ClassID(); }
      SClass_ID   SuperClassID()        { return GetColorCorrectionDesc()->SuperClassID(); }
      void        GetClassName(TSTR& s) { s = GetColorCorrectionDesc()->ClassName(); }
      void        DeleteThis()          { delete this; }

      // From ReferenceMaker
      int NumRefs() { return 2; }
      RefTargetHandle GetReference( int i ) 
         { switch( i ) {
            case 0:  return m_ParamBlock;
            case 1:  return m_Map;
            default: return 0;
         } }
      void SetReference( int i, RefTargetHandle v ) { 
         switch( i ) {
            case 0: m_ParamBlock = (IParamBlock2*)v; break;
            case 1: m_Map        = (Texmap      *)v; 
                    NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
                    break;
         }
      }

      RefTargetHandle Clone( RemapDir &remap );
      RefResult       NotifyRefChanged( Interval        changeInt, 
                                        RefTargetHandle hTarget, 
                                        PartID         &partID, 
                                        RefMessage      message );
     
      //this takes our channel wiring data and builds a mapping for each channel 
      //which is stored in mRewireData
      void UpdateChannelRewiring( TimeValue t);
      //this just updates the UI state of the custom rewire drop downs
      void UpdateChannelRewiringUI( TimeValue t);
      // this makes sure that the wiring mode switches to custom if we set values corresponding to custom.
      void UpdateChannelRewiringMode( TimeValue t);

      // IO - Everything is in the paramblock.
      IOResult Save(ISave *) { return IO_OK; }
      IOResult Load(ILoad *) { return IO_OK; }

      // return number of ParamBlocks in this instance
      int           NumParamBlocks   (     ) { return 1; }
      IParamBlock2 *GetParamBlock    (int i) { return m_ParamBlock; }
      
      // return id'd ParamBlock
      IParamBlock2 *GetParamBlockByID(BlockID id) 
         { 
            if (id == BlockParam::ID)
               return m_ParamBlock;

            return NULL;
         }

      // Multiple map in vp support
      BOOL SupportTexDisplay()                 { return TRUE; }
      BOOL SupportsMultiMapsInViewport()       { return TRUE; }

      void ActivateTexDisplay(BOOL onoff);
      void SetupGfxMultiMaps(TimeValue t, Material *mtl, MtlMakerCallback &cb);

      void InitMultimap      (     TimeValue  time, 
                            MtlMakerCallback &callback );
      void PrepareHWMultimap (     TimeValue  time, 
                           IHardwareMaterial *hw_material, 
                                  ::Material *material, 
                            MtlMakerCallback &callback  );
      void PrepareSWMultimap (     TimeValue  time, 
                                  ::Material *material, 
                            MtlMakerCallback &callback  );

   private:
      // Invalid calls.
      template< typename T > void operator=(T);
      template< typename T > void operator=(T) const;
      Texture(const Texture&);

      // Member variables
      IMtlParams           *m_MtlParam;
      IParamBlock2         *m_ParamBlock;
      Interval              m_Validity;
      Texmap               *m_Map;

      Interval              m_ViewportIval;
      TexHandlePtr          m_ViewportHandle;
      LightnessProc         *m_LightProc;

	  
      RewireData            mRewireData;
      bool                  mChangingRewireMode; // true when changing values because of a change of wiring mode
   };
}

static RollupCallback		g_RollupCallback;

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Dialog Procs

namespace ColorCorrection {

   class ColorProc : public ParamMap2UserDlgProc
   {
      static INT_PTR StaticWndProc( HWND h, UINT msg, WPARAM w, LPARAM l )
      {
         if (msg == WM_PAINT) {
            ColorProc* this_ = DLGetWindowLongPtr<ColorProc*>( h );
            CallWindowProc( this_->m_OldStaticWndProc, h, msg, w, l );
            this_->DrawSpectrum( h );
            return TRUE;
         }
         return FALSE;
      }

      static INT_PTR DynamicWndProc( HWND h, UINT msg, WPARAM w, LPARAM l )
      {
         if (msg == WM_PAINT) {
            ColorProc* this_ = DLGetWindowLongPtr<ColorProc*>( h );
            CallWindowProc( this_->m_OldDynamicWndProc, h, msg, w, l );

            TimeValue    t ( this_->m_MtlParam->GetTime() );

            float hue = this_->m_ParamBlock->GetFloat( BlockParam::HueShift,   t );
            float sat = this_->m_ParamBlock->GetFloat( BlockParam::Saturation, t );
            this_->DrawSpectrum( h, hue - 180.0f, 1.0f + sat / 100.0f, 0.5f );
            return TRUE;
         }
         return FALSE;
      }

	  void DrawSpectrum( HWND ctrl, float h = -180, float s = 1.0f, float l = 0.5f )
	  {
		  RECT rect_ctrl; GetClientRect( ctrl, &rect_ctrl );
		  int  width ( rect_ctrl.right  - rect_ctrl.left );
		  int  height( rect_ctrl.bottom - rect_ctrl.top  );

		  HDC      dc   ( ::GetDC( ctrl ) );
		  HBITMAP  membm( ::CreateCompatibleBitmap( dc, width, 1 ) );
		  HDC      memdc( ::CreateCompatibleDC( dc ) );
		  ::SelectObject( memdc, membm );

		  float inc( 360.0f / float(width) );
		  Color hsl( h, s, clamp(l,0.0f,1.0f) );
		  Color rgb;

		  for( int i = 0; i < width; i++, hsl[0] += inc ) {
			  hsl[0] = rotate( hsl[0], 0.0f, 360.0f );

			  rgb = HSLtoRGB( hsl );
			  ::SetPixelV( memdc, i, 0, RGB(int(255.0f*rgb[0]), 
				  int(255.0f*rgb[1]), 
				  int(255.0f*rgb[2])) );
		  }

         ::StretchBlt  ( dc,    rect_ctrl.left, rect_ctrl.top, width, height, 
                         memdc, 0,              0,             width, 1, 
                         SRCCOPY );
		  ::DeleteDC    ( memdc );
		  ::DeleteBitmap( membm );
		  ::ReleaseDC   ( ctrl, dc );
	  }

   public:
      ColorProc( IMtlParams *imp )
         : m_MtlParam( imp )
      {}

      INT_PTR DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
      {
         switch( msg ) {
            case WM_INITDIALOG:
                  m_ParamBlock = map->GetParamBlock();
                  m_hWnd       = hWnd;

                  // Subclass the static controls
                  DLSetWindowLongPtr( GetDlgItem( m_hWnd, IDC_CC_HUE_SPECTRUM  ), this );
                  m_OldStaticWndProc  = DLSetWindowProc( GetDlgItem( m_hWnd, IDC_CC_HUE_SPECTRUM ), 
                                                (WNDPROC)StaticWndProc  );

                  DLSetWindowLongPtr( GetDlgItem( m_hWnd, IDC_CC_HUE_SPECTRUM2 ), this );
                  m_OldDynamicWndProc = DLSetWindowProc( GetDlgItem( m_hWnd, IDC_CC_HUE_SPECTRUM2 ), 
                                                (WNDPROC)DynamicWndProc );
				  
                  ::InvalidateRect( m_hWnd, 0, TRUE );
                  break;
         }
         return FALSE;
      }

      void SetThing( ReferenceTarget *m ) {
         m_ParamBlock = m->GetParamBlock( 0 );
         ::InvalidateRect( m_hWnd, 0, TRUE );
		 if (m->ClassID() == GetColorCorrectionDesc()->ClassID())
		 {
			 Texture *colorCorrect = (Texture *)m;
			//make sure to update the Custom Color Channel UI since we swapped textures but not UI
			 colorCorrect->UpdateChannelRewiringUI(GetCOREInterface()->GetTime());
		 }
      }

	  void Update( TimeValue t ) 
	  {
		  HWND hwnd = GetDlgItem( m_hWnd, IDC_CC_HUE_SPECTRUM2 );
		  if (hwnd)
			::InvalidateRect( hwnd, 0, FALSE );
	  }

      void DeleteThis() 
	  { 
         delete this;
      }

   private:
      IMtlParams    * m_MtlParam;
      IParamBlock2  * m_ParamBlock;
      WNDPROC         m_OldStaticWndProc;
      WNDPROC         m_OldDynamicWndProc;
      HWND            m_hWnd;
   };

   class LightnessProc : public ParamMap2UserDlgProc
   {
      static INT_PTR StaticWndProc( HWND h, UINT msg, WPARAM w, LPARAM l )
      {
         if (msg != WM_PAINT)
            return FALSE;

         LightnessProc* this_ = DLGetWindowLongPtr<LightnessProc*>( h );
         if( this_ ) {
            CallWindowProc( this_->m_OldStaticWndProc, h, msg, w, l );
            this_->DrawSpectrum( h, Lightness::Standard( 0.0f, 0.0f ) );
         }
         return TRUE;
      }

      static INT_PTR DynamicWndProc( HWND h, UINT msg, WPARAM w, LPARAM l )
      {
         if (msg != WM_PAINT)
            return FALSE;

         using namespace Lightness;
         namespace BP = BlockParam;

         LightnessProc* this_ = DLGetWindowLongPtr<LightnessProc*>( h );
         if (!this_)
            return FALSE;

         CallWindowProc( this_->m_OldDynamicWndProc, h, msg, w, l );

         TimeValue     t( this_->m_MtlParam->GetTime() );
         int        mode( this_->m_ParamBlock->GetInt( BP::LightnessMode, t ) );
         
         if (0 == mode) {
            this_->DrawSpectrum( h, Standard( this_->m_ParamBlock, t ) );
         } else {
            this_->DrawSpectrum( h, Advanced<RGB>( this_->m_ParamBlock, t ) );
         }

         return TRUE;
      }

      template< typename Pred >
	  void DrawSpectrum( HWND ctrl, const Pred& pred )
	  {
		  RECT rect_ctrl; GetClientRect( ctrl, &rect_ctrl );
		  int  width ( rect_ctrl.right  - rect_ctrl.left );
		  int  height( rect_ctrl.bottom - rect_ctrl.top  );

		  HDC      dc   ( ::GetDC( ctrl ) );
		  HBITMAP  membm( ::CreateCompatibleBitmap( dc, width, 1 ) );
		  HDC      memdc( ::CreateCompatibleDC( dc ) );
		  ::SelectObject( memdc, membm );

		  float inc( 1.0f / float(width) );
		  float val( 0 );

		  for( int i = 0; i < width; i++, val += inc ) {
			  float rgb( pred(val) );
			  int   x  ( clamp( int(rgb * 255.0f), 0, 255 ) );
			  ::SetPixelV( memdc, i, 0, RGB(x, x, x) );
		  }

            ::StretchBlt  ( dc,    rect_ctrl.left, rect_ctrl.top, width, height, 
                            memdc, 0,              0,             width, 1, 
				  SRCCOPY );
		  ::DeleteDC    ( memdc );
		  ::DeleteBitmap( membm );
		  ::ReleaseDC   ( ctrl, dc );
         }
   public:
      LightnessProc( HWND editor_wnd, IMtlParams *imp, ColorCorrection::Texture *mtl )
         : m_EditorWnd( editor_wnd )
         , m_MtlParam ( imp )
         , m_Map      ( mtl )
      {}

      INT_PTR DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
      {
         int      id   ( LOWORD(wParam) );
         int      code ( HIWORD(wParam) );		
         switch( msg ) {
            case WM_INITDIALOG:
				  m_hWnd       = hWnd;         
                  m_ParamBlock = map->GetParamBlock();

                  // Subclass the static controls
                  DLSetWindowLongPtr( GetDlgItem( hWnd, IDC_LIGHTNESS_SPECTRUM  ), this );
                  m_OldStaticWndProc  = DLSetWindowProc( GetDlgItem( hWnd, IDC_LIGHTNESS_SPECTRUM ), 
                                                (WNDPROC)StaticWndProc  );

                  DLSetWindowLongPtr( GetDlgItem( hWnd, IDC_LIGHTNESS_SPECTRUM2 ), this );
                  m_OldDynamicWndProc = DLSetWindowProc( GetDlgItem( hWnd, IDC_LIGHTNESS_SPECTRUM2 ), 
                                                (WNDPROC)DynamicWndProc );
				//make sure to update the light UI based on whether it is standard or advance
				  SetLightUI( m_ParamBlock->GetInt( BlockParam::LightnessMode, GetCOREInterface()->GetTime()) );
                  ::InvalidateRect( hWnd, 0, TRUE );
                  break;
         }
         return FALSE;
      }

      void DeleteThis() 
	  { 
		  if (m_Map) 
			  m_Map->ParamDialog(NULL);
		  delete this; 
	  }

      void SetThing( ReferenceTarget *m )
      {
         if (m->ClassID() != GetColorCorrectionDesc()->ClassID())
            return;

         m_ParamBlock = m->GetParamBlock(0);
         m_Map        = (ColorCorrection::Texture*)m;
		 m_Map->ParamDialog(this);
		 //we swapped textures but not UI so we need to update the standard/advance light ui
		 SetLightUI(m_ParamBlock->GetInt(BlockParam::LightnessMode,GetCOREInterface()->GetTime()));
         ::InvalidateRect( m_hWnd, 0, TRUE );
      }

	  void Update( TimeValue t ) 
	  {
		  HWND hwnd = GetDlgItem( m_hWnd, IDC_LIGHTNESS_SPECTRUM2 );
		  if (hwnd)
			::InvalidateRect( hwnd, 0, FALSE );
	  }

	  //this toggles our advance or standard Light UI.  It used to be that this was seperated into 
	  //2 seperate rollups but the param map swapping code was problematic, so I just show/hide 
	  //ui instead to make it simplier and more stable.
	  //mode = 0 sets the UI to the Standard Light UI
	  //mode = 1 sets the UI to the Advance Light UI
	  void SetLightUI(int mode);

   private:
      IParamBlock2                    *m_ParamBlock;
      WNDPROC                          m_OldStaticWndProc;
      WNDPROC                          m_OldDynamicWndProc;
      HWND                             m_hWnd;

      HWND                             m_EditorWnd;
      IMtlParams                      *m_MtlParam;
      ColorCorrection::Texture        *m_Map;

   };

   void LightnessProc::SetLightUI(int mode)
   {
	   int showBasicUI = SW_SHOW;
	   int showAdvanceUI = SW_HIDE;
	   if (mode != 0)
	   {
		   showBasicUI = SW_HIDE;
		   showAdvanceUI = SW_SHOW;
	   }

	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_BRIGHTNESS ),showBasicUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_CONTRAST ),showBasicUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_BRIGHTNESS_EDIT ),showBasicUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_CONTRAST_EDIT ),showBasicUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_STATIC_STANDARD_1 ),showBasicUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_STATIC_STANDARD_2 ),showBasicUI);


	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_COMBO ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_ENABLE_R ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_ENABLE_G ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_ENABLE_B ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_GAIN_RGB ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_GAIN_R ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_GAIN_G ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_GAIN_B ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_GAIN_RGB_SPIN ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_GAIN_R_SPIN ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_GAIN_G_SPIN ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_GAIN_B_SPIN ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_STATIC_ADVANCE_1 ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_STATIC_ADVANCE_2 ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_STATIC_ADVANCE_3 ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_STATIC_ADVANCE_4 ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_STATIC_ADVANCE_5 ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_STATIC_ADVANCE_6 ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_STATIC_ADVANCE_7 ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_STATIC_ADVANCE_8 ),showAdvanceUI);

	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_GAMMA_RGB ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_GAMMA_R ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_GAMMA_G ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_GAMMA_B ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_GAMMA_RGB_SPIN ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_GAMMA_R_SPIN ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_GAMMA_G_SPIN ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_GAMMA_B_SPIN ),showAdvanceUI);

	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_PIVOT_RGB ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_PIVOT_R ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_PIVOT_G ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_PIVOT_B ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_PIVOT_RGB_SPIN ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_PIVOT_R_SPIN ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_PIVOT_G_SPIN ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_PIVOT_B_SPIN ),showAdvanceUI);

	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_LIFT_RGB ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_LIFT_R ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_LIFT_G ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_LIFT_B ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_LIFT_RGB_SPIN ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_LIFT_R_SPIN ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_LIFT_G_SPIN ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_LIFT_B_SPIN ),showAdvanceUI);

	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_PRINT ),showAdvanceUI);
	   ShowWindow(GetDlgItem( m_hWnd, IDC_LIGHTNESS_PRINT_SPIN ),showAdvanceUI);

   }

   // dialog stuff to get the ResetAll button message
   class TextureProc : public ParamMap2UserDlgProc
   {
   public:
      INT_PTR DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
      {
         switch (msg) {
            case WM_COMMAND:
               switch (LOWORD(wParam)) {
                  case IDC_CC_BUTTON_RESET_ALL:
                  {
                     IParamBlock2 * pblock = map->GetParamBlock();
                     pblock->ResetAll();
                  }
               }
         }
         return FALSE;
      }
      void DeleteThis() {delete this;}
   };
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Class Descriptors

void*          ColorCorrection::Desc::Create(BOOL)   { return new ColorCorrection::Texture(); }
int            ColorCorrection::Desc::IsPublic()     { return GetAppID() != kAPP_VIZR; }
SClass_ID      ColorCorrection::Desc::SuperClassID() { return TEXMAP_CLASS_ID; }
Class_ID       ColorCorrection::Desc::ClassID()      { return Class_ID( COLORCORRECTION_CLASS_ID, 0 ); }
HINSTANCE      ColorCorrection::Desc::HInstance()    { return hInstance; }
const TCHAR  * ColorCorrection::Desc::ClassName()    { return GetString( IDS_RB_COLORCORRECTION_CDESC ); }
const TCHAR  * ColorCorrection::Desc::Category()     { return TEXMAP_CAT_COLMOD; }
const TCHAR  * ColorCorrection::Desc::InternalName() { return _T("ColorCorrection"); }

ClassDesc2* GetColorCorrectionDesc() {
   static ColorCorrection::Desc ClassDescInstance;
   return &ClassDescInstance; 
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Accessors

namespace ColorCorrection { namespace {

   void TextureAccessorClass::Set( PB2Value      & v, 
                                   ReferenceMaker* owner, 
                                   ParamID         id, 
                                   int             tabIndex, 
                                   TimeValue       t )
   {
      if (   owner
          && owner->ClassID() == GetColorCorrectionDesc()->ClassID()
          && id == BlockParam::RewireMode) 
      {
         ((Texture*)owner)->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);  //TO DO Why do we need this the paramblock should take care of this
      }
   }

   void ChannelAccessorClass::Set( PB2Value      & v, 
                                   ReferenceMaker* owner, 
                                   ParamID         id, 
                                   int             tabIndex, 
                                   TimeValue       t )
   {
      if (   owner
          && owner->ClassID() == GetColorCorrectionDesc()->ClassID()) 
      {
         //we cannot update on these parameters since the UpdateChannelRewiringUI
         //calls set values on them which is circular
         if ((id != BlockParam::RewireRed_Custom) &&
            (id != BlockParam::RewireBlue_Custom) &&
            (id != BlockParam::RewireGreen_Custom) && 
            (id != BlockParam::RewireAlpha_Custom) )
         {
            ((Texture*)owner)->UpdateChannelRewiringUI( t );
            ((Texture*)owner)->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);  //TO DO Why do we need this the paramblock should take care of this
         }
         else
         {
            ((Texture*)owner)->UpdateChannelRewiringMode( t );
         }
      }
   }

   void ColorAccessorClass::Set( PB2Value      & v, 
                                 ReferenceMaker* owner, 
                                 ParamID         id, 
                                 int             tabIndex, 
                                 TimeValue       t )
   {
      if (   owner
          && owner->ClassID() == GetColorCorrectionDesc()->ClassID()) 
      {
         ((Texture*)owner)->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);  //TO DO Why do we need this the paramblock should take care of this
      }
   }

   void LightnessAccessorClass::Set( PB2Value      & v, 
                                     ReferenceMaker* owner, 
                                     ParamID         id, 
                                     int             tabIndex, 
                                     TimeValue       t )
   {
      if (   owner
          && owner->ClassID() == GetColorCorrectionDesc()->ClassID()) 
      {
		  if (id == BlockParam::LightnessMode)
		  {
			  //we have to look up the dlg proc off the class desc since we only have 1 and it
			  //can get swapped around
			  ParamBlockDesc2 *pbd( GetColorCorrectionDesc()->GetParamBlockDesc( BlockParam::ID ) );
			  LightnessProc *proc = (LightnessProc *)GetColorCorrectionDesc()->GetUserDlgProc(pbd, BlockParam::LightnessStandardRollout);
			  if (proc)
				  proc->SetLightUI(v.i);
		  }
         ((Texture*)owner)->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);  //TO DO Why do we need this the paramblock should take care of this
      }
   }
} }

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Implementation

ParamDlg* ColorCorrection::Texture::CreateParamDlg(HWND editor_wnd, IMtlParams *imp) 
{   
   using namespace BlockParam;
   LightnessProc* proc;

   // Create each rollouts manually
   ParamDlg* main( GetColorCorrectionDesc()->CreateParamDlg( ID, editor_wnd, imp, this, TextureRollout ) );
   GetColorCorrectionDesc()->CreateParamDlg( ID, editor_wnd, imp, this, ChannelRollout );
   GetColorCorrectionDesc()->CreateParamDlg( ID, editor_wnd, imp, this, ColorRollout );

   int lightMode = m_ParamBlock->GetInt( LightnessMode, imp->GetTime() );  
   GetColorCorrectionDesc()->CreateParamDlg( ID, editor_wnd, imp, this, 
                                                           LightnessStandardRollout );
   proc = new LightnessProc( editor_wnd, imp, this );
   proc->SetLightUI(lightMode);

   GetColorCorrectionDesc()->SetUserDlgProc( &ParamBlock, ColorRollout, new ColorProc( imp ) );
   GetColorCorrectionDesc()->SetUserDlgProc( &ParamBlock, TextureRollout, new TextureProc() );
   GetColorCorrectionDesc()->SetUserDlgProc( &ParamBlock, LightnessStandardRollout, proc);

	//only need to call this on the first map since it just returns the medit rollup
    IParamMap2 *map = m_ParamBlock->GetMap(ChannelRollout);
	map->GetIRollup()->RegisterRollupCallback(&g_RollupCallback);


   m_LightProc = proc;
   

	UpdateChannelRewiringUI(GetCOREInterface()->GetTime());

   return main;
}

void ColorCorrection::Texture::UpdateChannelRewiringUI( TimeValue t)
{
   namespace BP = BlockParam;

   if (0 == m_ParamBlock)
      return;

   int type = m_ParamBlock->GetInt(BlockParam::RewireMode,t);
   mChangingRewireMode = true;

   int r = 0;
   int g = 0;
   int b = 0;
   int a = 0;
   switch( type ) {
	   using namespace RewireCombo;
	  case RewireType::Normal:
		  r = Red, g = Green, b = Blue, a = Alpha;
		  break;
	  case RewireType::Invert:
		  r = RedInverse, g = GreenInverse, b = BlueInverse, a = Alpha;
		  break;
	  case RewireType::Monochrome:
		  r = Mono, g = Mono, b = Mono, a = Alpha;
		  break;
	  case RewireType::Custom:
		  r = m_ParamBlock->GetInt( BP::RewireRed_Custom,   t ); 
		  g = m_ParamBlock->GetInt( BP::RewireGreen_Custom, t ); 
		  b = m_ParamBlock->GetInt( BP::RewireBlue_Custom,  t ); 
		  a = m_ParamBlock->GetInt( BP::RewireAlpha_Custom, t ); 
		  break;
	  default:
		  // Should not happen and you're in for trouble.
		  DbgAssert( FALSE );
		  break;
   };

   m_ParamBlock->SetValue( BP::RewireRed_Custom,   t, r );
   m_ParamBlock->SetValue( BP::RewireGreen_Custom, t, g );
   m_ParamBlock->SetValue( BP::RewireBlue_Custom,  t, b );
   m_ParamBlock->SetValue( BP::RewireAlpha_Custom, t, a );

   mChangingRewireMode = false;
}

void ColorCorrection::Texture::UpdateChannelRewiring( TimeValue t )
{
	namespace BP = BlockParam;

	if (0 == m_ParamBlock)
		return;

	int type = m_ParamBlock->GetInt(BlockParam::RewireMode,t);

	int r = 0;
	int g = 0;
	int b = 0;
	int a = 0;
	switch( type ) {
		using namespace RewireCombo;
		case RewireType::Normal:
			r = Red, g = Green, b = Blue, a = Alpha;
			break;
		case RewireType::Invert:
			r = RedInverse, g = GreenInverse, b = BlueInverse, a = Alpha;
			break;
		case RewireType::Monochrome:
			r = Mono, g = Mono, b = Mono, a = Alpha;
			break;
		case RewireType::Custom:
			r = m_ParamBlock->GetInt( BP::RewireRed_Custom,   t ); 
			g = m_ParamBlock->GetInt( BP::RewireGreen_Custom, t ); 
			b = m_ParamBlock->GetInt( BP::RewireBlue_Custom,  t ); 
			a = m_ParamBlock->GetInt( BP::RewireAlpha_Custom, t ); 
			break;
		default:
			// Should not happen and you're in for trouble.
			DbgAssert( FALSE );
			break;
	};

	mRewireData.mR = r;
	mRewireData.mG = g;
	mRewireData.mB = b;
	mRewireData.mA = a;
}

void ColorCorrection::Texture::UpdateChannelRewiringMode( TimeValue t )
{
	namespace BP = BlockParam;

	if (0 == m_ParamBlock || mChangingRewireMode)
		return;

	int type = m_ParamBlock->GetInt( BP::RewireMode, t );
	int r = m_ParamBlock->GetInt( BP::RewireRed_Custom, t ); 
	int g = m_ParamBlock->GetInt( BP::RewireGreen_Custom, t ); 
	int b = m_ParamBlock->GetInt( BP::RewireBlue_Custom, t ); 
	int a = m_ParamBlock->GetInt( BP::RewireAlpha_Custom, t ); 

	switch( type ) {
		using namespace RewireCombo;
		case RewireType::Normal:
			if ( r != Red || g != Green || b != Blue || a != Alpha )
			{
				m_ParamBlock->SetValue( BP::RewireMode, t, RewireType::Custom );
			}
			break;
		case RewireType::Invert:
			if ( r != RedInverse || g != GreenInverse || b != BlueInverse || a != Alpha )
			{
				m_ParamBlock->SetValue( BP::RewireMode, t, RewireType::Custom );
			}
			break;
		case RewireType::Monochrome:
			if ( r != Mono || g != Mono || b != Mono || a != Alpha )
			{
				m_ParamBlock->SetValue( BP::RewireMode, t, RewireType::Custom );
			}
			break;
		case RewireType::Custom:
			break;
		default:
			// Should not happen and you're in for trouble.
			DbgAssert( FALSE );
			break;
	};

}

RefResult ColorCorrection::Texture::NotifyRefChanged( Interval        changeInt, 
                                                      RefTargetHandle hTarget, 
                                                      PartID         &partID, 
                                                      RefMessage      message )
{
	using namespace BlockParam;

	switch (message) {
	  case REFMSG_CHANGE:
		  if ((hTarget == m_ParamBlock) && GetColorCorrectionDesc()) 
		  {
			  // Update the visual for the color and lightness (those who have custom
			  // WM_PAINT).
			  ParamBlockDesc2 *pbd( GetColorCorrectionDesc()->GetParamBlockDesc( ID ) );
			  TimeValue        t  ( GetCOREInterface()->GetTime() );

			  ParamID changing_param = m_ParamBlock->LastNotifyParamID();
			  ParamBlock.InvalidateUI(changing_param);

			  if ( ((changing_param == BlockParam::HueShift) || (changing_param == BlockParam::Saturation))
				  && pbd) 
			  {
				  if (pbd->GetUserDlgProc( ColorRollout ))
					  pbd->GetUserDlgProc( ColorRollout )->Update( t );
			  }
			  if ( ((changing_param == BlockParam::Contrast) || (changing_param == BlockParam::Brightness))
				  && m_LightProc)
				  m_LightProc->Update( t );

		  }

		  NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);  //TO DO  we are already notifiying plus the SDK doc say we should not do a notifydep in here??

		  // Reset the validity interval.
		  m_Validity.SetEmpty();     //TO DO the textures validity should be computed in the update, just because we have notification does not mean we are invalid???
		  m_ViewportIval.SetEmpty(); //TO DO see above
		  break;
	}


	return REF_SUCCEED;
}

void ColorCorrection::Texture::Init()
{
   macroRecorder->Disable();
   m_Validity.SetEmpty();
   m_ViewportIval.SetEmpty();
   UpdateChannelRewiring( GetCOREInterface()->GetTime() ); // sets mRewireData
   mChangingRewireMode = false;
   macroRecorder->Enable();
}

void ColorCorrection::Texture::Reset()
{  
   DeleteAllRefsFromMe();
   GetColorCorrectionDesc()->MakeAutoParamBlocks( this );
   Init();
}

RefTargetHandle ColorCorrection::Texture::Clone( RemapDir &remap )
{
   Texture* NewMtl( new Texture() );
   *((MtlBase*)NewMtl) = *((MtlBase*)this);  // copy superclass stuff  

   // Replace the ParamBlock and Map
   NewMtl->ReplaceReference( 0, remap.CloneRef( m_ParamBlock ) );
   NewMtl->ReplaceReference( 1, remap.CloneRef( m_Map        ) );

   // Set properties
   NewMtl->m_MtlParam       = m_MtlParam;
   NewMtl->m_Validity.SetEmpty();
   NewMtl->m_ViewportIval.SetEmpty();

   BaseClone(this, NewMtl, remap);
   return (RefTargetHandle)NewMtl;
}

ColorCorrection::Texture::Texture()
   : m_ParamBlock    ( 0 )
   , m_Map           ( 0 )
   , m_MtlParam      ( 0 )
   , mChangingRewireMode( false )
   , m_LightProc       ( 0 )
{
   GetColorCorrectionDesc()->MakeAutoParamBlocks( this );
   Init(); // uses info in pb2
}

AColor ColorCorrection::Texture::Correct( const AColor& c, TimeValue t )
{
   namespace BP = BlockParam;
   using namespace Rewire;
   using namespace Lightness;
   using namespace HSL;

   // Perform Rewiring of the color.
   int RewireR = mRewireData.mR;
   int RewireG = mRewireData.mG;
   int RewireB = mRewireData.mB;
   int RewireA = mRewireData.mA ;
   
   AColor res( RewireList[ RewireR ]( c ), RewireList[ RewireG ]( c ),
               RewireList[ RewireB ]( c ), RewireList[ RewireA ]( c ) );

   // Perform HSL variation (Color rollout)
   Color hsl( RGBtoHSL( Color(res) ) );
   // Hue Shift
   hsl[0] = rotate( hsl[0] + m_ParamBlock->GetFloat( BP::HueShift, t ),
                    0.0f, 360.0f );
   hsl[1] = clamp ( hsl[1] + m_ParamBlock->GetFloat( BP::Saturation, t ) / 100.0f,
                    0.0f, 1.0f );

   // Tint calculation
   AColor tint     = m_ParamBlock->GetAColor( BP::HueTint,         t );
   float  strength = m_ParamBlock->GetFloat ( BP::HueTintStrength, t );

   float hue_tint = CalcHue( tint );

   // Verify that it is not infinity or undefined (could happened if monochrome - no hue)
   if (hue_tint >= 0.0f && hue_tint <= 360.0f)
      hsl[0] = (hsl[0] + (hue_tint - hsl[0]) * (strength/100.0f));

   res = AColor( HSLtoRGB( hsl ), res.a );

   // Perform Lightness variation
   // Standard?
   int mode = m_ParamBlock->GetInt( BP::LightnessMode, t );
   if (0 == mode) {
      res = AColor( Transform( res, Standard( m_ParamBlock, t ) ), res.a );
   } else {
      res = AColor( Transform( res, Advanced<RGB>( m_ParamBlock, t ) ), res.a );

      if (FALSE != m_ParamBlock->GetInt( BlockParam::EnableR, t )) {
         res.r = Advanced<R>( m_ParamBlock, t )( res.r );
      }
      if (FALSE != m_ParamBlock->GetInt( BlockParam::EnableG, t )) {
         res.g = Advanced<G>( m_ParamBlock, t )( res.g );
      }
      if (FALSE != m_ParamBlock->GetInt( BlockParam::EnableB, t )) {
         res.b = Advanced<B>( m_ParamBlock, t )( res.b );
      }
   }

   return clamp( res, AColor(0,0,0,0), AColor(1,1,1,1) );
}

AColor ColorCorrection::Texture::EvalColor( ShadeContext& sc )
{
   AColor res;
   if ( sc.GetCache( this, res ) )
      return res;

   if ( gbufID )
      sc.SetGBufferID(gbufID);

   TimeValue    t    ( sc.CurTime());
   AColor       color( m_Map ? m_Map->EvalColor( sc ) 
                             : m_ParamBlock->GetAColor( BlockParam::DefaultColor, t ) );

   res = Correct( color, t );

   sc.PutCache( this, res );
   return res;
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Viewport Interactive Rendering

// Init the interactive multi-map rendering
// Creates a bitmap from the sub-map by calling SetupGfxMultiMaps.  
// Color correct that bitmap.
void ColorCorrection::Texture::InitMultimap( TimeValue  time, 
                                      MtlMakerCallback &callback )
{
   PBITMAPINFO bmi;

   if (m_Map) {
      bmi = m_Map->GetVPDisplayDIB( time, callback, m_ViewportIval );
      if (!bmi)
         return;

      unsigned char *data( (unsigned char*)bmi + sizeof( BITMAPINFOHEADER ) );
      const int      w   ( bmi->bmiHeader.biWidth  );
      const int      h   ( bmi->bmiHeader.biHeight );

      for( int y = 0; y < h; ++y ) {
         for( int x = 0; x < w; ++x, data += 4 ) {
            // Don't forget bitmaps are BGRA, not RGBA.
            const float r( float(data[2]) / 255.0f );
            const float g( float(data[1]) / 255.0f );
            const float b( float(data[0]) / 255.0f );
            const float a( float(data[3]) / 255.0f );
            AColor res = Correct( AColor(r,g,b,a), time );
            data[2] = (unsigned char)( res.r * 0xFF );
            data[1] = (unsigned char)( res.g * 0xFF );
            data[0] = (unsigned char)( res.b * 0xFF );
            data[3] = (unsigned char)( res.a * 0xFF );
         }
      }
   } else {
      // If no map, creates a one pixel bitmap containing the color.
      bmi = (PBITMAPINFO)(new char[sizeof(BITMAPINFOHEADER) + 4]);
      if (!bmi)
         return;

      BITMAPINFOHEADER &bmih( bmi->bmiHeader );
      unsigned char    *data( (unsigned char*)bmi + sizeof( BITMAPINFOHEADER ) );
      
      bmih.biSize        = sizeof( BITMAPINFOHEADER );
      bmih.biWidth       = 1;
      bmih.biHeight      = 1;
      bmih.biPlanes      = 1;
      bmih.biBitCount    = 32;
      bmih.biCompression = BI_RGB;
      bmih.biSizeImage   = 4;

      BMM_Color_fl color( Correct( m_ParamBlock->GetAColor( BlockParam::DefaultColor, time ), time ) );
      data[ 2 ] = (unsigned char)(color.r * 0xFF);
      data[ 1 ] = (unsigned char)(color.g * 0xFF);
      data[ 0 ] = (unsigned char)(color.b * 0xFF);
      data[ 3 ] = (unsigned char)(color.a * 0xFF);
   }

   m_ViewportHandle = callback.MakeHandle( bmi );
}

// Configure max to show the texture in DirectX.
void ColorCorrection::Texture::PrepareHWMultimap(     TimeValue  time, 
                                              IHardwareMaterial *hw_material, 
                                                     ::Material *material, 
                                               MtlMakerCallback &callback )
{
   hw_material->SetNumTexStages( 1 );

   hw_material->SetTexture( 0, m_ViewportHandle->GetHandle() );
   material->texture[0].useTex = 0;
   callback.GetGfxTexInfoFromTexmap( time, material->texture[0], m_Map );

   // Set the operations and the arguments for the Texture
   hw_material->SetTextureColorOp       ( 0, D3DTOP_MODULATE );

   hw_material->SetTextureColorArg      ( 0, 1, D3DTA_TEXTURE  );
   hw_material->SetTextureColorArg      ( 0, 2, D3DTA_CURRENT  );

   hw_material->SetTextureAlphaOp       ( 0, D3DTOP_SELECTARG2 );
   hw_material->SetTextureAlphaArg      ( 0, 1, D3DTA_TEXTURE  );
   hw_material->SetTextureAlphaArg      ( 0, 2, D3DTA_CURRENT  );

   hw_material->SetTextureTransformFlag ( 0, D3DTTFF_COUNT2    );
}

// Configure max to show the texture, either in OpenGL or software.
void ColorCorrection::Texture::PrepareSWMultimap(      TimeValue  time, 
                                                      ::Material *material, 
                                                MtlMakerCallback &callback )
{
   material->texture.SetCount( 1 );
   callback.GetGfxTexInfoFromTexmap( time, material->texture[0], m_Map );
   material->texture[0].textHandle       = m_ViewportHandle->GetHandle();
   material->texture[0].colorOp          = GW_TEX_ALPHA_BLEND;
   material->texture[0].colorAlphaSource = GW_TEX_TEXTURE;
   material->texture[0].colorScale       = GW_TEX_SCALE_1X;
   material->texture[0].alphaOp          = GW_TEX_MODULATE;
   material->texture[0].alphaAlphaSource = GW_TEX_TEXTURE;
   material->texture[0].alphaScale       = GW_TEX_SCALE_1X;
}

void ColorCorrection::Texture::ActivateTexDisplay(BOOL onoff) {}

void ColorCorrection::Texture::SetupGfxMultiMaps(TimeValue t, Material *mtl, MtlMakerCallback &cb)
{ 
   // Are we still valid?  If not, initialize...
   if ( false == m_ViewportIval.InInterval(t) ) {
      // Set the interval and initialize the map.
      m_ViewportIval.SetInfinite(); 
      InitMultimap( t, cb );
   }

   IHardwareMaterial *HWMaterial = (IHardwareMaterial *)GetProperty(PROPID_HARDWARE_MATERIAL);
   
   // Are we using hardware?
   if (HWMaterial) {
      PrepareHWMultimap( t, HWMaterial, mtl, cb );
   } else {
      PrepareSWMultimap( t, mtl, cb );
   }
}



#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#endif
