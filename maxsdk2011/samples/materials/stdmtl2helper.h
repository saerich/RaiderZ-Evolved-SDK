#pragma once

#include <ILockedTracks.h>

// IDs for all the ParamBlocks and their parameters.  One block UI per rollout.
enum { std2_shader, std2_extended, std2_sampling, std_maps, std2_dynamics, };  // pblock IDs
// std2_shader param IDs
enum 
{ 
	std2_shader_type, std2_wire, std2_two_sided, std2_face_map, std2_faceted,
	std2_shader_by_name,  // virtual param for accessing shader type by name
};
// std2_extended param IDs
enum 
{ 
	std2_opacity_type, std2_opacity, std2_filter_color, std2_ep_filter_map,
	std2_falloff_type, std2_falloff_amnt, 
	std2_ior,
	std2_wire_size, std2_wire_units,
	std2_apply_refl_dimming, std2_dim_lvl, std2_refl_lvl,
	std2_translucent_blur, std2_ep_translucent_blur_map,
	std2_translucent_color, std2_ep_translucent_color_map,
};

// std2_sampling param IDs
enum 
{ 
	std2_ssampler, std2_ssampler_qual, std2_ssampler_enable, 
	std2_ssampler_adapt_on, std2_ssampler_adapt_threshold, std2_ssampler_advanced,
	std2_ssampler_subsample_tex_on, std2_ssampler_by_name, 
	std2_ssampler_param0, std2_ssampler_param1, std2_ssampler_useglobal
};
// std_maps param IDs
enum 
{
	std2_map_enables, std2_maps, std2_map_amnts, std2_mp_ad_texlock, 
};
// std2_dynamics param IDs
enum 
{
	std2_bounce, std2_static_friction, std2_sliding_friction,
};


// paramblock2 block and parameter IDs for the standard shaders
enum { shdr_params, };
// shdr_params param IDs
enum 
{ 
	shdr_ambient, shdr_diffuse, shdr_specular,
	shdr_ad_texlock, shdr_ad_lock, shdr_ds_lock, 
	shdr_use_self_illum_color, shdr_self_illum_amnt, shdr_self_illum_color, 
	shdr_spec_lvl, shdr_glossiness, shdr_soften,
};


// Helper function to enable the control associated with the opacity.
// Only works when the material is a StdMtl2.
// Opacity is exposed through shader UI but is handled by material. StdMat2 does only expose
// the value of opacity, but does not say if the value is read only or not.
inline void EnableMtl2OpacityControl( StdMat2 * mat, ICustomControl * control )
{
	static Class_ID StdMtl2ClassID(DMTL_CLASS_ID, 0);
	if (mat->ClassID() == StdMtl2ClassID
		|| mat->IsSubClassOf( StdMtl2ClassID )) {
			IParamBlock2 * pb_extended = mat->GetParamBlockByID( std2_extended );
			if ( pb_extended ) {
				control->UpdateEnableState( pb_extended, pb_extended->GetAnimNum( std2_opacity ) );
			}
	}
}