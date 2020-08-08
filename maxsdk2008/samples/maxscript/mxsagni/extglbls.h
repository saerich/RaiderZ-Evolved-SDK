define_system_global_replace ( "rootNode",				get_root_node,				set_root_node);
define_system_global ( "rendOutputFilename",	get_rend_output_filename,	set_rend_output_filename);
define_system_global ( "rendCamNode",			get_rend_camNode,			set_rend_camNode);
define_system_global ( "rendUseImgSeq",			get_rend_useImgSeq,			set_rend_useImgSeq);
define_system_global ( "rendImgSeqType",		get_rend_imgSeqType,		set_rend_imgSeqType);

define_system_global ( "preRendScript",			get_rend_preRendScript,		set_rend_preRendScript);
define_system_global ( "usePreRendScript",		get_rend_usePreRendScript,	set_rend_usePreRendScript);
define_system_global ( "localPreRendScript",	get_rend_localPreRendScript,set_rend_localPreRendScript);
define_system_global ( "postRendScript",		get_rend_postRendScript,	set_rend_postRendScript);
define_system_global ( "usePostRendScript",		get_rend_usePostRendScript,	set_rend_usePostRendScript);

define_system_global ( "realTimePlayback",		get_real_time_playback,		set_real_time_playback);
define_system_global ( "playActiveOnly",		get_play_active_only,		set_play_active_only);
define_system_global ( "playbackLoop",			get_playback_loop,			set_playback_loop);
define_system_global ( "animButtonEnabled",		get_enable_animate_button,	set_enable_animate_button);
define_system_global ( "animButtonState",		get_animate_button_state,	set_animate_button_state);
define_system_global ( "flyOffTime",			get_fly_off_time,			set_fly_off_time);
//define_system_global( "sliderPrecision",		get_slider_precision,		set_slider_precision);

#ifndef WEBVERSION
define_system_global ( "renderDisplacements",	get_renderDisplacements,	set_renderDisplacements);
define_system_global_replace ( "renderEffects",			get_renderEffects,			set_renderEffects);
define_system_global ( "skipRenderedFrames",	get_skipRenderedFrames,		set_skipRenderedFrames);
#endif // WEBVERSION

define_system_global ( "showEndResult",			get_showEndResult,			set_showEndResult);

define_struct_global ( "useTransformGizmos",	"preferences",			get_xform_gizmos,     set_xform_gizmos);			
define_struct_global ( "constantReferenceSystem", "preferences",		get_constant_axis,	  set_constant_axis);
define_struct_global ( "useVertexDots",			"preferences",			get_useVertexDots,	  set_useVertexDots);
define_struct_global ( "useLargeVertexDots",	"preferences",			get_vertexDotType,	  set_vertexDotType);
define_struct_global ( "flyOffTime",			"preferences",			get_fly_off_time,	  set_fly_off_time);
define_struct_global ( "spinnerWrap",			"preferences",			get_spinner_wrap,	  set_spinner_wrap);

define_struct_global ( "spinnerPrecision",		"preferences",			get_spinner_precision,set_spinner_precision);
define_struct_global ( "spinnerSnap",			"preferences",			get_spinner_snap,	  set_spinner_snap);
define_struct_global ( "useSpinnerSnap",		"preferences",			get_spinner_useSnap,  set_spinner_useSnap);

#ifndef WEBVERSION	// russom - 04/16/02
define_struct_global ( "maximumGBufferLayers"	, "preferences",		get_maxGBufferLayers, set_maxGBufferLayers);
#endif

// LAM - 8/29/03
#ifdef ECO1144
define_struct_global ( "DontRepeatRefMsg"	, "preferences",			get_DontRepeatRefMsg, set_DontRepeatRefMsg);
#endif

define_struct_global ( "InvalidateTMOpt"	, "preferences",	get_InvalidateTMOpt, set_InvalidateTMOpt);

define_struct_global ( "useTrackBar",			"timeConfiguration",	get_useTrackBar,			set_useTrackBar);		    
define_struct_global ( "playActiveOnly",		"timeConfiguration",	get_play_active_only,		set_play_active_only);
define_struct_global ( "realTimePlayback",		"timeConfiguration",	get_real_time_playback,		set_real_time_playback);
define_struct_global ( "playbackLoop",			"timeConfiguration",	get_playback_loop,			set_playback_loop);

// Log System
define_struct_global ( "quietMode",				"logsystem",			get_quiet_mode,				set_quiet_mode);

//mcr_global_def


