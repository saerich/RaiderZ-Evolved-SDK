// Some render globals

// RK: 5/17/99 Not valid in Shiva anymore
//define_struct_global ( "maxRayDepth",			"scanlineRender",		get_max_ray_depth,			set_max_ray_depth);

#ifndef WEBVERSION
define_struct_global ( "antiAliasFilter",		"scanlineRender",		get_anti_alias_filter,		set_anti_alias_filter);
define_struct_global ( "antiAliasFilterSize",	"scanlineRender",	get_scanRenderer_antiAliasFilterSz,		set_scanRenderer_antiAliasFilterSz);
define_struct_global ( "enablePixelSampler",	"scanlineRender",	get_scanRenderer_pixelSamplerEnable,	set_scanRenderer_pixelSamplerEnable);

// LAM: 9/10/01 More scanline renderer exposure

define_struct_global ( "mapping",				"scanlineRender",	get_scanRenderer_mapping,				set_scanRenderer_mapping);
define_struct_global ( "shadows", 				"scanlineRender",	get_scanRenderer_shadows,				set_scanRenderer_shadows);
define_struct_global ( "autoReflect", 			"scanlineRender",	get_scanRenderer_autoReflect,			set_scanRenderer_autoReflect);
define_struct_global ( "forceWireframe", 		"scanlineRender",	get_scanRenderer_forceWireframe,		set_scanRenderer_forceWireframe);
define_struct_global ( "wireThickness", 		"scanlineRender",	get_scanRenderer_wireThickness,			set_scanRenderer_wireThickness);
define_struct_global ( "antiAliasing", 			"scanlineRender",	get_scanRenderer_antiAliasing,			set_scanRenderer_antiAliasing);
define_struct_global ( "filterMaps", 			"scanlineRender",	get_scanRenderer_filterMaps,			set_scanRenderer_filterMaps);
define_struct_global ( "objectMotionBlur", 		"scanlineRender",	get_scanRenderer_objectMotionBlur,		set_scanRenderer_objectMotionBlur);
define_struct_global ( "objectBlurDuration", 	"scanlineRender",	get_scanRenderer_objectBlurDuration,	set_scanRenderer_objectBlurDuration); 
define_struct_global ( "objectBlurSamples", 	"scanlineRender",	get_scanRenderer_objectBlurSamples,		set_scanRenderer_objectBlurSamples);
define_struct_global ( "objectBlurSubdivisions","scanlineRender",	get_scanRenderer_objectBlurSubdivisions,set_scanRenderer_objectBlurSubdivisions);
define_struct_global ( "imageMotionBlur", 		"scanlineRender",	get_scanRenderer_imageMotionBlur,		set_scanRenderer_imageMotionBlur);
define_struct_global ( "imageBlurDuration", 	"scanlineRender",	get_scanRenderer_imageBlurDuration,		set_scanRenderer_imageBlurDuration);
define_struct_global ( "autoReflectLevels", 	"scanlineRender",	get_scanRenderer_autoReflectLevels,		set_scanRenderer_autoReflectLevels);
define_struct_global ( "colorClampType", 		"scanlineRender",	get_scanRenderer_colorClampType,		set_scanRenderer_colorClampType);
define_struct_global ( "imageBlurEnv", 			"scanlineRender",	get_scanRenderer_imageBlurEnv,			set_scanRenderer_imageBlurEnv);
define_struct_global ( "imageBlurTrans", 		"scanlineRender",	get_scanRenderer_imageBlurTrans,		set_scanRenderer_imageBlurTrans);
define_struct_global ( "conserveMemory", 		"scanlineRender",	get_scanRenderer_conserveMemory,		set_scanRenderer_conserveMemory);
define_struct_global ( "enableSSE",				"scanlineRender",	get_scanRenderer_enableSSE,				set_scanRenderer_enableSSE);
#ifdef	SINGLE_SUPERSAMPLE_IN_RENDER
define_struct_global ( "samplerQuality",		"scanlineRender",	get_scanRenderer_samplerQuality,		set_scanRenderer_samplerQuality);
#endif	// SINGLE_SUPERSAMPLE_IN_RENDER
#endif // WEBVERSION

