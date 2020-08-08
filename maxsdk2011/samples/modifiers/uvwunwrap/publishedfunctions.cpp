/*

Copyright [2008] Autodesk, Inc.  All rights reserved.

Use of this software is subject to the terms of the Autodesk license agreement 
provided at the time of installation or download, or which otherwise accompanies 
this software in either electronic or hard copy form.   

*/
#include "unwrap.h"
#include "modstack.h"

static UnwrapClassDesc unwrapDesc;


ClassDesc* GetUnwrapModDesc() {return &unwrapDesc;}

static FPInterfaceDesc unwrap_interface6(
	UNWRAP_INTERFACE6, _T("unwrap6"), 0, &unwrapDesc, FP_MIXIN,

	unwrap_getvertexpositionByNode, _T("GetVertexPositionByNode"),0,TYPE_POINT3,0,3,
							_T("time"),0,TYPE_TIMEVALUE,
							_T("index"),0,TYPE_INT,
							_T("node"),0,TYPE_INODE,

	unwrap_numberverticesByNode, _T("numberVerticesByNode"),0,TYPE_INT,0,1,
							_T("node"),0,TYPE_INODE,
	unwrap_getselectedpolygonsByNode,_T("getSelectedPolygonsByNode"),0,TYPE_BITARRAY,0,1,
							_T("node"),0,TYPE_INODE,
	unwrap_selectpolygonsByNode, _T("selectPolygonsByNode"),0,TYPE_VOID,0,2,
							_T("selection"),0,TYPE_BITARRAY,
							_T("node"),0,TYPE_INODE,
	unwrap_ispolygonselectedByNode, _T("isPolygonSelectedByNode"),0,TYPE_BOOL,0,2,
							_T("index"),0,TYPE_INT,
							_T("node"),0,TYPE_INODE,
	unwrap_numberpolygonsByNode, _T("numberPolygonsByNode"),0, TYPE_INT, 0, 1,
							_T("node"),0,TYPE_INODE,

	unwrap_markasdeadByNode, _T("markAsDeadByNode"),0,TYPE_VOID,0,2,
							_T("index"), 0, TYPE_INT,
							_T("node"),0,TYPE_INODE,

	unwrap_numberpointsinfaceByNode, _T("numberPointsInFaceByNode"),0,TYPE_INT,0,2,
							_T("index"), 0, TYPE_INT,
							_T("node"),0,TYPE_INODE,
	unwrap_getvertexindexfromfaceByNode, _T("getVertexIndexFromFaceByNode"),0,TYPE_INT,0,3,
							_T("faceIndex"), 0, TYPE_INT,
							_T("ithVertex"), 0, TYPE_INT,
							_T("node"),0,TYPE_INODE,
	unwrap_gethandleindexfromfaceByNode, _T("getHandleIndexFromFaceByNode"),0,TYPE_INT,0,3,
							_T("faceIndex"), 0, TYPE_INT,
							_T("ithVertex"), 0, TYPE_INT,
							_T("node"),0,TYPE_INODE,
	unwrap_getinteriorindexfromfaceByNode, _T("getHandleIndexFromFaceByNode"),0,TYPE_INT,0,3,
							_T("faceIndex"), 0, TYPE_INT,
							_T("ithVertex"), 0, TYPE_INT,
							_T("node"),0,TYPE_INODE,
	unwrap_getvertexgindexfromfaceByNode, _T("getVertexGeomIndexFromFaceByNode"),0,TYPE_INT,0,3,
							_T("faceIndex"), 0, TYPE_INT,
							_T("ithVertex"), 0, TYPE_INT,
							_T("node"),0,TYPE_INODE,
	unwrap_gethandlegindexfromfaceByNode, _T("getHandleGeomIndexFromFaceByNode"),0,TYPE_INT,0,3,
							_T("faceIndex"), 0, TYPE_INT,
							_T("ithVertex"), 0, TYPE_INT,
							_T("node"),0,TYPE_INODE,
	unwrap_getinteriorgindexfromfaceByNode, _T("getInteriorGeomIndexFromFaceByNode"),0,TYPE_INT,0,3,
							_T("faceIndex"), 0, TYPE_INT,
							_T("ithVertex"), 0, TYPE_INT,
							_T("node"),0,TYPE_INODE,

	unwrap_addpointtofaceByNode, _T("setFaceVertexByNode"),0,TYPE_VOID,0,5,
							_T("pos"), 0, TYPE_POINT3,
							_T("faceIndex"), 0, TYPE_INT,
							_T("ithVertex"), 0, TYPE_INT,
							_T("sel"), 0, TYPE_BOOL,
							_T("node"),0,TYPE_INODE,
	unwrap_addpointtohandleByNode, _T("setFaceHandleByNode"),0,TYPE_VOID,0,5,
							_T("pos"), 0, TYPE_POINT3,
							_T("faceIndex"), 0, TYPE_INT,
							_T("ithVertex"), 0, TYPE_INT,
							_T("sel"), 0, TYPE_BOOL,
							_T("node"),0,TYPE_INODE,
	unwrap_addpointtointeriorByNode, _T("setFaceInteriorByNode"),0,TYPE_VOID,0,5,
							_T("pos"), 0, TYPE_POINT3,
							_T("faceIndex"), 0, TYPE_INT,
							_T("ithVertex"), 0, TYPE_INT,
							_T("sel"), 0, TYPE_BOOL,
							_T("node"),0,TYPE_INODE,

	unwrap_setfacevertexindexByNode, _T("setFaceVertexIndexByNode"),0,TYPE_VOID,0,4,
							_T("faceIndex"), 0, TYPE_INT,
							_T("ithVertex"), 0, TYPE_INT,
							_T("vertexIndex"), 0, TYPE_INT,
							_T("node"),0,TYPE_INODE,
	unwrap_setfacehandleindexByNode, _T("setFaceHandleIndexByNode"),0,TYPE_VOID,0,4,
							_T("faceIndex"), 0, TYPE_INT,
							_T("ithVertex"), 0, TYPE_INT,
							_T("vertexIndex"), 0, TYPE_INT,
							_T("node"),0,TYPE_INODE,
	unwrap_setfaceinteriorindexByNode, _T("setFaceInteriorIndexByNode"),0,TYPE_VOID,0,4,
							_T("faceIndex"), 0, TYPE_INT,
							_T("ithVertex"), 0, TYPE_INT,
							_T("vertexIndex"), 0, TYPE_INT,
							_T("node"),0,TYPE_INODE,

	unwrap_getareaByNode, _T("getAreaByNode"), 0, TYPE_VOID, 0, 4,
							_T("faceSelection"), 0, TYPE_BITARRAY,
							_T("areaUVW"), 0, TYPE_FLOAT_BR, f_inOut, FPP_OUT_PARAM,
							_T("areaGeom"), 0, TYPE_FLOAT_BR, f_inOut, FPP_OUT_PARAM,
							_T("node"),0,TYPE_INODE,

	unwrap_getboundsByNode, _T("getAreaByNode"), 0, TYPE_VOID, 0, 6,
							_T("faceSelection"), 0, TYPE_BITARRAY,
							_T("x"), 0, TYPE_FLOAT_BR, f_inOut, FPP_OUT_PARAM,
							_T("y"), 0, TYPE_FLOAT_BR, f_inOut, FPP_OUT_PARAM,
							_T("width"), 0, TYPE_FLOAT_BR, f_inOut, FPP_OUT_PARAM,
							_T("height"), 0, TYPE_FLOAT_BR, f_inOut, FPP_OUT_PARAM,
							_T("node"),0,TYPE_INODE,


	unwrap_pelt_getseamselectionByNode, _T("getPeltSelectedSeamsByNode"),0,TYPE_BITARRAY,0,1,
							_T("node"),0,TYPE_INODE,
	unwrap_pelt_setseamselectionByNode, _T("setPeltSelectedSeamsByNode"),0,TYPE_VOID,0,2,
							_T("selection"),0,TYPE_BITARRAY,
							_T("node"),0,TYPE_INODE,
	unwrap_selectpolygonsupdateByNode, _T("selectPolygonsUpdateByNode"),0,TYPE_VOID,0,3,
							_T("selection"),0,TYPE_BITARRAY,
							_T("update"),0,TYPE_BOOL,
							_T("node"),0,TYPE_INODE,

	unwrap_sketchByNode, _T("sketchByNode"),0, TYPE_VOID, 0, 3,
							_T("indexList"),0,TYPE_INT_TAB,
							_T("positionList"),0,TYPE_POINT3_TAB,
							_T("node"),0,TYPE_INODE,

	unwrap_getnormalByNode, _T("getNormalByNode"),0,TYPE_POINT3,0,2,
							_T("faceIndex"), 0, TYPE_INT,
							_T("node"),0,TYPE_INODE,

	unwrap_setvertexpositionByNode, _T("setVertexPositionByNode"),0,TYPE_VOID,0,4,
							_T("time"), 0, TYPE_TIMEVALUE,
							_T("index"), 0, TYPE_INT,
							_T("pos"), 0, TYPE_POINT3,
							_T("node"),0,TYPE_INODE,

	unwrap_setvertexposition2ByNode, _T("setVertexPosition2ByNode"),0,TYPE_VOID,0,6,
							_T("time"), 0, TYPE_TIMEVALUE,
							_T("index"), 0, TYPE_INT,
							_T("pos"), 0, TYPE_POINT3,
							_T("hold"), 0, TYPE_BOOL,
							_T("update"), 0, TYPE_BOOL,
							_T("node"),0,TYPE_INODE,

	unwrap_getvertexweightByNode, _T("getVertexWeightByNode"), 0, TYPE_FLOAT, 0, 2,
							_T("index"), 0, TYPE_INT,
							_T("node"),0,TYPE_INODE,
	unwrap_setvertexweightByNode, _T("setVertexWeightByNode"), 0, TYPE_VOID, 0, 3,
							_T("index"), 0, TYPE_INT,
							_T("weight"), 0, TYPE_FLOAT,
							_T("node"),0,TYPE_INODE,

	unwrap_isweightmodifiedByNode, _T("isWeightModifiedByNode"), 0, TYPE_BOOL, 0, 2,
							_T("index"), 0, TYPE_INT,
							_T("node"),0,TYPE_INODE,
	unwrap_modifyweightByNode, _T("modifyWeightByNode"), 0, TYPE_VOID, 0, 3,
							_T("index"), 0, TYPE_INT,
							_T("modify"), 0, TYPE_BOOL,
							_T("node"),0,TYPE_INODE,

	unwrap_getselectedvertsByNode, _T("getSelectedVerticesByNode"),0,TYPE_BITARRAY,0,1,
							_T("node"),0,TYPE_INODE,
	unwrap_selectvertsByNode, _T("selectVerticesByNode"),0,TYPE_VOID,0,2,
							_T("selection"),0,TYPE_BITARRAY,
							_T("node"),0,TYPE_INODE,

	unwrap_isvertexselectedByNode, _T("isVertexSelectedByNode"),0,TYPE_BOOL,0,2,
							_T("index"),0,TYPE_INT,
							_T("node"),0,TYPE_INODE,

	unwrap_getselectedfacesByNode, _T("getSelectedFacesByNode"),0,TYPE_BITARRAY,0,1,
							_T("node"),0,TYPE_INODE,
	unwrap_selectfacesByNode, _T("selectFacesByNode"),0,TYPE_VOID,0,2,
							_T("selection"),0,TYPE_BITARRAY,
							_T("node"),0,TYPE_INODE,
	unwrap_isfaceselectedByNode, _T("isFaceSelectedByNode"),0,TYPE_BOOL,0,2,
							_T("index"),0,TYPE_INT,
							_T("node"),0,TYPE_INODE,

	unwrap_getselectededgesByNode, _T("getSelectedEdgesByNode"),0,TYPE_BITARRAY,0,1,
							_T("node"),0,TYPE_INODE,
	unwrap_selectedgesByNode, _T("selectEdgesByNode"),0,TYPE_VOID,0,2,
							_T("selection"),0,TYPE_BITARRAY,
							_T("node"),0,TYPE_INODE,

	unwrap_isedgeselectedByNode, _T("isEdgeSelectedByNode"),0,TYPE_BOOL,0,2,
							_T("index"),0,TYPE_INT,
							_T("node"),0,TYPE_INODE,

	unwrap_getgeomvertexselectionByNode, _T("getSelectedGeomVertsByNode"),0,TYPE_BITARRAY,0,1,
						_T("node"),0,TYPE_INODE,
	unwrap_setgeomvertexselectionByNode, _T("setSelectedGeomVertsByNode"),0,TYPE_VOID,0,2,
						_T("selection"),0,TYPE_BITARRAY,
						_T("node"),0,TYPE_INODE,

	unwrap_getgeomedgeselectionByNode, _T("getSelectedGeomEdgesByNode"),0,TYPE_BITARRAY,0,1,
						_T("node"),0,TYPE_INODE,
	unwrap_setgeomedgeselectionByNode, _T("setSelectedGeomEdgesByNode"),0,TYPE_VOID,0,2,
						_T("selection"),0,TYPE_BITARRAY,
						_T("node"),0,TYPE_INODE,

	unwrap_getsg, _T("getSG"),0, TYPE_INT, 0, 0,
	unwrap_setsg, _T("setSG"),0, TYPE_VOID, 0, 1,
						_T("sgID"),0,TYPE_INT,

	unwrap_getMatIDSelect, _T("getSelectMatID"),0, TYPE_INT, 0, 0,
	unwrap_setMatIDSelect, _T("setSelectMatID"),0, TYPE_VOID, 0, 1,
						_T("matID"),0,TYPE_INT,


	unwrap_splinemap, _T("splineMap"),0, TYPE_VOID, 0, 0,

	unwrap_splinemap_selectSpline, _T("splineMap_selectSpline"),0,TYPE_VOID,0,2,
						_T("splineIndex"),0,TYPE_INDEX,
						_T("sel"),0,TYPE_BOOL,
	unwrap_splinemap_isSplineSelected, _T("splineMap_isSplineSelected"),0,TYPE_BOOL,0,1,
						_T("splineIndex"),0,TYPE_INDEX,	


	unwrap_splinemap_selectCrossSection, _T("splineMap_selectCrossSection"),0,TYPE_VOID,0,3,
						_T("splineIndex"),0,TYPE_INDEX,
						_T("crossSectionIndex"),0,TYPE_INDEX,
						_T("sel"),0,TYPE_BOOL,
	unwrap_splinemap_isCrossSectionSelected, _T("splineMap_isCrossSectionSelected"),0,TYPE_BOOL,0,2,
						_T("splineIndex"),0,TYPE_INDEX,	
						_T("crossSectionIndex"),0,TYPE_INDEX,


	unwrap_splinemap_clearSelectSpline, _T("splineMap_ClearSelectSpline"),0,TYPE_VOID,0,0,
	unwrap_splinemap_clearSelectCrossSection, _T("splineMap_ClearSelectCrossSection"),0,TYPE_VOID,0,0,


	unwrap_splinemap_numberOfSplines, _T("splineMap_numberSplines"),0,TYPE_INT,0,0,
	unwrap_splinemap_numberOfCrossSections, _T("splineMap_numberCrossSection"),0,TYPE_INT,0,1,
						_T("splineIndex"),0,TYPE_INDEX,	

	unwrap_splinemap_getCrossSection_Pos, _T("splineMap_GetCrossSection_Pos"),0,TYPE_POINT3,0,2,
						_T("splineIndex"),0,TYPE_INDEX,	
						_T("crossSectionIndex"),0,TYPE_INDEX,	
	unwrap_splinemap_getCrossSection_ScaleX, _T("splineMap_GetCrossSection_ScaleX"),0,TYPE_FLOAT,0,2,
						_T("splineIndex"),0,TYPE_INDEX,	
						_T("crossSectionIndex"),0,TYPE_INDEX,	
	unwrap_splinemap_getCrossSection_ScaleY, _T("splineMap_GetCrossSection_ScaleY"),0,TYPE_FLOAT,0,2,
						_T("splineIndex"),0,TYPE_INDEX,	
						_T("crossSectionIndex"),0,TYPE_INDEX,	
	unwrap_splinemap_getCrossSection_Twist, _T("splineMap_GetCrossSection_Twist"),0,TYPE_FLOAT,0,2,
						_T("splineIndex"),0,TYPE_INDEX,	
						_T("crossSectionIndex"),0,TYPE_INDEX,	

	unwrap_splinemap_setCrossSection_Pos, _T("splineMap_SetCrossSection_Pos"),0,TYPE_VOID,0,3,
						_T("splineIndex"),0,TYPE_INDEX,	
						_T("crossSectionIndex"),0,TYPE_INDEX,	
						_T("pos"),0,TYPE_POINT3,	
	unwrap_splinemap_setCrossSection_ScaleX, _T("splineMap_SetCrossSection_ScaleX"),0,TYPE_VOID,0,3,
						_T("splineIndex"),0,TYPE_INDEX,	
						_T("crossSectionIndex"),0,TYPE_INDEX,	
						_T("scaleX"),0,TYPE_FLOAT,	
 	unwrap_splinemap_setCrossSection_ScaleY, _T("splineMap_SetCrossSection_ScaleY"),0,TYPE_VOID,0,3,
						_T("splineIndex"),0,TYPE_INDEX,	
						_T("crossSectionIndex"),0,TYPE_INDEX,	
						_T("scaleY"),0,TYPE_FLOAT,	
	unwrap_splinemap_setCrossSection_Twist, _T("splineMap_SetCrossSection_Twist"),0,TYPE_VOID,0,3,
						_T("splineIndex"),0,TYPE_INDEX,	
						_T("crossSectionIndex"),0,TYPE_INDEX,	
						_T("Twist"),0,TYPE_FLOAT,	

	unwrap_splinemap_recomputeCrossSections, _T("splineMap_RecomputeCrossSections"),0, TYPE_VOID, 0, 0,

	unwrap_splinemap_fit, _T("splineMap_Fit"),0, TYPE_VOID, 0, 2,
						_T("fitAll"),0,TYPE_BOOL,
						_T("extraScale"),0,TYPE_FLOAT,


	unwrap_splinemap_align, _T("splineMap_Align"),0, TYPE_VOID, 0, 3,
						_T("splineIndex"),0,TYPE_INDEX,	
						_T("crossSectionIndex"),0,TYPE_INDEX,	
						_T("vec"),0,TYPE_POINT3,							
	unwrap_splinemap_alignSelected, _T("splineMap_AlignSelected"),0, TYPE_VOID, 0, 1,
						_T("vec"),0,TYPE_POINT3,							
	unwrap_splinemap_aligncommandmode, _T("splineMap_AlignCommandMode"),0, TYPE_VOID, 0, 0,
	unwrap_splinemap_alignsectioncommandmode, _T("splineMap_AlignSectionCommandMode"),0, TYPE_VOID, 0, 0,

	unwrap_splinemap_copy, _T("splineMap_Copy"),0, TYPE_VOID, 0, 0,
	unwrap_splinemap_paste, _T("splineMap_Paste"),0, TYPE_VOID, 0, 0,
	unwrap_splinemap_pasteToSelected, _T("splineMap_PasteToSelected"),0, TYPE_VOID, 0, 2,
						_T("splineIndex"),0,TYPE_INDEX,	
						_T("crossSectionIndex"),0,TYPE_INDEX,	

	unwrap_splinemap_delete, _T("splineMap_Delete"),0, TYPE_VOID, 0, 0,
	
	unwrap_splinemap_addCrossSectionCommandmode, _T("splineMap_AddCrossSectionMode"),0, TYPE_VOID, 0, 0,


	unwrap_splinemap_moveSelectedCrossSection, _T("splineMap_moveSelectedCrossSection"),0, TYPE_VOID, 0, 1,
						_T("u"),0,TYPE_FLOAT,	
	unwrap_splinemap_rotateSelectedCrossSection, _T("splineMap_rotateSelectedCrossSection"),0, TYPE_VOID, 0, 1,
						_T("twist"),0,TYPE_FLOAT,	
	unwrap_splinemap_scaleSelectedCrossSection, _T("splineMap_scaleSelectedCrossSection"),0, TYPE_VOID, 0, 1,
						_T("scale"),0,TYPE_POINT2,	

	unwrap_splinemap_insertCrossSection, _T("splineMap_InsertCrossSection"),0, TYPE_VOID, 0, 2,
						_T("splineIndex"),0,TYPE_INDEX,	
						_T("u"),0,TYPE_FLOAT,	


	unwrap_splinemap_dump, _T("splineMap_Dump"),0, TYPE_VOID, 0, 0,	

	unwrap_splinemap_resample, _T("splineMap_Resample"),0, TYPE_VOID, 0, 1,	
						_T("samples"),0,TYPE_INT,	


	unwrap_gettweakmode, _T("getTweakMode"),0, TYPE_BOOL, 0, 0,	
	unwrap_settweakmode, _T("setTweakMode"),0, TYPE_VOID, 0, 1,	
						_T("on"),0,TYPE_BOOL,	

	unwrap_uvloop, _T("uvLoop"),0, TYPE_VOID, 0, 1,	
						_T("mode"),0,TYPE_INT,	
	unwrap_uvring, _T("uvRing"),0, TYPE_VOID, 0, 1,	
						_T("mode"),0,TYPE_INT,	

	unwrap_align, _T("align"),0, TYPE_VOID, 0, 1,	
						_T("horizontal"),0,TYPE_BOOL,	
	unwrap_space, _T("space"),0, TYPE_VOID, 0, 1,	
						_T("horizontal"),0,TYPE_BOOL,	


	end
	);


class UnwrapPBAccessor : public PBAccessor
{ 
	public:
		void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
		{
			UnwrapMod* p = (UnwrapMod*)owner;
			if (id == unwrap_splinemap_node)
			{
				INode *node = (INode *) v.r;
				p->SetSplineMappingNode(node);
			}
			if ((id == unwrap_splinemap_node) ||
				(id == unwrap_splinemap_manualseams) ||
				(id == unwrap_splinemap_projectiontype) 
				)
			{
				p->fnSplineMap_UpdateUI();
			}
		}
};


static UnwrapPBAccessor unwrap_accessor;


static ParamBlockDesc2 unwrap_param_blk ( unwrap_params, _T("params"),  0, &unwrapDesc, 
										 P_AUTO_CONSTRUCT + + P_AUTO_UI+ P_MULTIMAP, PBLOCK_REF, 
										 // params
										 1,
										 unwrap_renderuv_params,  IDD_UNWRAP_RENDERUVS_PARAMS, IDS_RENDERUV_PARAMETERS, 0, 0, NULL,

										 unwrap_texmaplist, 	_T("texMapList"),		TYPE_TEXMAP_TAB, 		0,	P_VARIABLE_SIZE, 0,
										 end, 

										 unwrap_checkmtl,	_T("checkerMaterial"), 		TYPE_MTL, 	0, 	0, 
										 end,

										 unwrap_originalmtl,	_T("baseMaterial"), 		TYPE_MTL, 	0, 	0, 
										 end,

										 unwrap_texmapidlist, 	_T("texMapIDList"),		TYPE_INT_TAB, 		0,	P_VARIABLE_SIZE, 0,
										 end, 

										 unwrap_gridsnap,	_T("gridSnap"),		TYPE_BOOL, 		0,	 IDS_GRIDSNAP,
										 p_default, TRUE,
										 end, 
										 unwrap_vertexsnap,	_T("vertexSnap"),		TYPE_BOOL, 		0,	 IDS_VERTEXSNAP,
										 p_default, TRUE,
										 end, 
										 unwrap_edgesnap,	_T("edgeSnap"),		TYPE_BOOL, 		0,	 IDS_EDGESNAP,
										 p_default, TRUE,
										 end, 

										 unwrap_showimagealpha,	_T("showImageAlpha"),		TYPE_BOOL, 		0,	 IDS_SHOWIMAGEALPHA,
										 p_default, FALSE,
										 end, 

										 unwrap_renderuv_width, 	_T("renderuv_width"), 	TYPE_INT, 	 0, 	IDS_RENDERUV_WIDTH, 
										 p_default, 		1024, 
										 p_range, 		1, 10000, 
										 p_ui, 			unwrap_renderuv_params,TYPE_SPINNER, EDITTYPE_INT, IDC_WIDTH, IDC_WIDTHSPIN, SPIN_AUTOSCALE,  
										 end, 

										 unwrap_renderuv_height, 	_T("renderuv_height"), 	TYPE_INT, 	 0, 	IDS_RENDERUV_HEIGHT, 
										 p_default, 		1024, 
										 p_range, 		1, 10000, 
										 p_ui, 			unwrap_renderuv_params,TYPE_SPINNER, EDITTYPE_INT, IDC_HEIGHT, IDC_HEIGHTSPIN, SPIN_AUTOSCALE,  
										 end, 

										 unwrap_renderuv_edgecolor,	 _T("renderuv_edgeColor"),	TYPE_RGBA,		0,	IDS_EDGECOLOR,	
										 p_default,		Color(1,1,1), 
										 p_ui,			unwrap_renderuv_params,	TYPE_COLORSWATCH, IDC_UNWRAP_EDGECOLOR, 
										 end,

										 unwrap_renderuv_edgealpha, 	_T("renderuv_edgealpha"), 	TYPE_FLOAT, 	 0, 	IDS_RENDERUV_EDGEALPHA, 
										 p_default, 		1.0f, 
										 p_range, 		0.0f, 1.0f, 
										 p_ui, 			unwrap_renderuv_params,TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDGEALPHA, IDC_EDGEALPHASPIN, SPIN_AUTOSCALE,  
										 end, 

										 unwrap_renderuv_seamcolor,	 _T("renderuv_seamColor"),	TYPE_RGBA,		0,	IDS_SEAMCOLOR,	
										 p_default,		Color(0,1,0), 
										 p_ui,			unwrap_renderuv_params,TYPE_COLORSWATCH, IDC_UNWRAP_SEAMCOLOR, 
										 end,

										 unwrap_renderuv_visible,	_T("renderuv_visibleedges"),		TYPE_BOOL, 		0,	 IDS_VISIBLEEDGES,
										 p_default, TRUE,
										 p_ui,unwrap_renderuv_params,TYPE_SINGLECHEKBOX, IDC_VISIBLE_CHECK, 
										 end,
										 unwrap_renderuv_invisible,	_T("renderuv_invisibleedges"),		TYPE_BOOL, 		0,	 IDS_INVISIBLEEDGES,
										 p_default, FALSE,
										 p_ui,unwrap_renderuv_params,TYPE_SINGLECHEKBOX, IDC_INVISIBLE_CHECK, 
										 end,

										 unwrap_renderuv_seamedges,	_T("renderuv_seamedges"),		TYPE_BOOL, 		0,	 IDS_SEAMEDGES,
										 p_default, TRUE,
										 p_ui,unwrap_renderuv_params,TYPE_SINGLECHEKBOX, IDC_SEAM_CHECK, 
										 end,

										 unwrap_renderuv_showframebuffer,	_T("renderuv_showframebuffer"),		TYPE_BOOL, 		0,	 IDS_SHOWFRAMEBUFFER,
										 p_default, TRUE,
										 p_ui,unwrap_renderuv_params,TYPE_SINGLECHEKBOX, IDC_SHOWFRAMEBUFFER_CHECK, 
										 end,

										 unwrap_renderuv_force2sided,	_T("renderuv_force2sided"),		TYPE_BOOL, 		0,	 IDS_FORCE2SIDED,
										 p_default, TRUE,
										 p_ui,unwrap_renderuv_params,TYPE_SINGLECHEKBOX, IDC_FORCE2SIDE_CHECK, 
										 end,

										 unwrap_renderuv_fillmode, 			_T("renderuv_fillmode"), 		TYPE_INT, 	0, 	IDS_FILLMODE, 
										 p_default, 		0, 
										 p_ui,unwrap_renderuv_params, TYPE_INTLISTBOX, IDC_FILLMODECOMBO, 
										 4, IDS_NONE, IDS_SOLID, IDS_NORMAL, IDS_SHADED,		
										 end,

										 unwrap_renderuv_fillalpha, 	_T("renderuv_fillalpha"), 	TYPE_FLOAT, 	 0, 	IDS_RENDERUV_FILLALPHA, 
										 p_default, 		1.0f, 
										 p_range, 		0.0f, 1.0f, 
										 p_ui, 			unwrap_renderuv_params,TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FILLALPHA, IDC_FILLALPHASPIN, SPIN_AUTOSCALE,  
										 end, 

										 unwrap_renderuv_fillcolor,	 _T("renderuv_fillColor"),	TYPE_RGBA,		0,	IDS_FILLCOLOR,	
										 p_default,		Color(0.5,0.5,0.5), 
										 p_ui,			unwrap_renderuv_params,TYPE_COLORSWATCH, IDC_UNWRAP_FILLCOLOR, 
										 end,


										 unwrap_renderuv_overlap,	_T("renderuv_showoverlap"),		TYPE_BOOL, 		0,	 IDS_OVERLAP,
										 p_default, TRUE,
										 p_ui,unwrap_renderuv_params,TYPE_SINGLECHEKBOX, IDC_OVERLAP_CHECK, 
										 end,

										 unwrap_renderuv_overlapcolor,	 _T("renderuv_overlapColor"),	TYPE_RGBA,		0,	IDS_OVERLAPCOLOR,	
										 p_default,		Color(1.0,0.0,0.0), 
										 p_ui,			unwrap_renderuv_params,TYPE_COLORSWATCH, IDC_UNWRAP_OVERLAPCOLOR, 
										 end,


										 unwrap_qmap_preview,	_T("quick_map_preview"),		TYPE_BOOL, 		0,	 IDS_PREVIEW,
										 p_default, TRUE,
										 end,

										 unwrap_qmap_align,	_T("quick_map_align"),		TYPE_INT, 		0,	 IDS_QMAPALIGN,
										 p_default, 3,
										 end,

										 unwrap_originalmtl_list,	_T("baseMaterial_list"), 		TYPE_MTL_TAB, 		0,	P_VARIABLE_SIZE, 0,
										 end,


										 unwrap_splinemap_node,	_T("splinemap_node"), 		TYPE_INODE, 		0,	 0,
										 p_accessor,	&unwrap_accessor,
										 end,

										 unwrap_splinemap_manualseams,	_T("splinemap_manualseams"),		TYPE_BOOL, 		0,	 0,
										 p_default, FALSE,
										 p_accessor,	&unwrap_accessor,
										 end,

										 unwrap_splinemap_projectiontype,	_T("splinemap_projectiontype"),		TYPE_INT, 		0,	 0,
										 p_default, 0,
										 p_accessor,	&unwrap_accessor,
										 end,

										 unwrap_splinemap_display,	_T("splinemap_display"),		TYPE_BOOL, 		0,	 0,
										 p_default, TRUE,
										 end,


										 unwrap_splinemap_uscale,	_T("splinemap_uscale"),		TYPE_FLOAT, 		0,	 0,
										 p_default, 1.0f,
										 end,

										 unwrap_splinemap_vscale,	_T("splinemap_vscale"),		TYPE_FLOAT, 		0,	 0,
										 p_default, 1.0f,
										 end,

										 unwrap_splinemap_uoffset,	_T("splinemap_uoffset"),		TYPE_FLOAT, 		0,	 0,
										 p_default, 0.0f,
										 end,

										 unwrap_splinemap_voffset,	_T("splinemap_voffset"),		TYPE_FLOAT, 		0,	 0,
										 p_default, 0.0f,
										 end,

										 unwrap_localDistorion,	_T("localDistortion"),		TYPE_BOOL, 		0,	 0,
										 p_default, FALSE,
										 end,

										 unwrap_splinemap_iterations,	_T("splinemap_iterations"),		TYPE_INT, 		0,	 0,
										  p_range, 		1, 64, 
										 p_default, 16,
										 end,


										 unwrap_splinemap_resample_count,	_T("splinemap_resampleCount"),		TYPE_INT, 		0,	 0,
										 p_range, 		2, 256, 
										 p_default, 4,
										 end,

										 unwrap_splinemap_method,	_T("splinemap_advanceMethod"),		TYPE_INT, 		0,	 0,										 
										 p_default, 1,
										 end,





										 end


										 );

UnwrapMod::UnwrapMod()
{
	mRelaxThreadHandle = NULL;	
	mPeltThreadHandle = NULL;	
	mGizmoTM = Matrix3(1);
	mHasMax9Data = FALSE;
	
	suppressWarning = FALSE;

	minimized = FALSE;
	renderUVWindow = NULL;
	showCounter = FALSE;
	enableActionItems = FALSE;
	xWindowOffset = 0;
	yWindowOffset = 0;
	maximizeHeight = 0;
	maximizeWidth = 0;
	CurrentMap = 0;
	relaxBySpringUseOnlyVEdges = FALSE;
	relaxBySpringIteration = 10;
	relaxBySpringStretch = 0.0f;

	edgeDistortionScale = 1.0f;
	showEdgeDistortion = FALSE;
	normalizeMap = TRUE;
	alwaysShowSeams = TRUE;
	mapMapMode = NOMAP;
	initialSnapState = 0;
	suspendNotify = FALSE;
	flattenMax5 = FALSE;
	absoluteTypeIn = TRUE;
	rotationsRespectAspect = TRUE;
	//5.1.06
	relaxAmount = 0.1f;
	relaxType = 1;
	relaxIteration = 100;
	relaxStretch = 0.0f;
	relaxBoundary = FALSE;
	relaxSaddle = FALSE;
	relaxWindowPos.length = 0;
	peltWindowPos.length = 0;

	popUpDialog = TRUE;
	//5.1.05
	autoBackground = TRUE;

	bringUpPanel = FALSE;
	modifierInstanced = FALSE;

	mMouseHitLocalData = NULL;
	mMouseHitVert = 0;

	loadDefaults = TRUE;
	subObjCount = 3;

	getFaceSelectionFromStack = FALSE;
	falloffStr = 0.0f;
	forceUpdate = TRUE;

	windowPos.length =0;
	mapScale = .0f;
	lockAspect = TRUE;
	move    = 0;
	zoom    = 0.9f;
	aspect	= 1.0f;
	xscroll = 0.0f;
	yscroll = 0.0f;

	image = NULL;
	iw = ih = 0;
	uvw = 0;
	scale =0;
	zoomext =0;
	showMap = TRUE;	
	rendW = 256;
	rendH = 256;
	channel = 0;
	//	CurrentMap = 0;
	isBitmap = 0;
	useBitmapRes = FALSE;
	pixelSnap =0;
	tmControl = NULL;

	flags = CONTROL_CENTER|CONTROL_FIT|CONTROL_INIT;

	version = CURRENTVERSION;
	lockSelected = 0;
	mirror = 0;
	filterSelectedFaces = 0;
	hide = 0;
	freeze = 0;
	incSelected = 0;
	falloff = 0;
	falloffSpace = 0;

	oldDataPresent = FALSE;

	updateCache = FALSE;

	//UNFOLD
	updateCache = TRUE;
	showVertexClusterList = FALSE;
	gDebugLevel = 1;

	normalSpacing = 0.02f;
	normalNormalize = TRUE;
	normalRotate = TRUE;
	normalAlignWidth = TRUE;
	normalMethod = 0;
	normalWindowPos.length = 0;
	normalHWND =  NULL;

	flattenAngleThreshold = 45.0f;
	flattenSpacing = 0.02f;
	flattenNormalize = TRUE;
	flattenRotate = TRUE;
	flattenCollapse = TRUE;
	flattenWindowPos.length = 0;
	flattenHWND =  NULL;

	unfoldWindowPos.length = 0;
	unfoldHWND =  NULL;
	unfoldNormalize = TRUE;
	unfoldMethod = 1;

	//STITCH
	bStitchAlign = TRUE;
	bStitchScale = TRUE;
	fStitchBias = 0.5f;
	stitchHWND = NULL;
	stitchWindowPos.length = 0;

	//tile stuff
	bTile = TRUE;
	fContrast = 0.5f;
	iTileLimit = 1;

	//put back after finish menu
	if (GetCOREInterface()->GetMenuManager()->RegisterMenuBarContext(kUnwrapMenuBar, _T("UVW Unwrap Menu Bar"))) 
	{
		IMenu* pMenu = GetCOREInterface()->GetMenuManager()->FindMenu(_T("UVW Unwrap Menu Bar"));

		if (!pMenu) 
		{
			pMenu = GetIMenu();
			pMenu->SetTitle("UVW Unwrap Menu Bar");
			GetCOREInterface()->GetMenuManager()->RegisterMenu(pMenu, 0);

			IMenuBarContext* pContext = (IMenuBarContext*) GetCOREInterface()->GetMenuManager()->GetContext(kUnwrapMenuBar);
			pContext->SetMenu(pMenu);
			pContext->CreateWindowsMenu();

		}
	}

	limitSoftSel = TRUE;
	limitSoftSelRange = 16;

	//elem mode
	geomElemMode = FALSE;

	planarThreshold = 15.0f;
	planarMode = FALSE;

	ignoreBackFaceCull = TRUE;
	oldSelMethod = FALSE;

	InitScriptUI();

	tvElementMode = FALSE;
	alwaysEdit = FALSE;

	packMethod = 0;
	packSpacing = 0.02f;
	packNormalize = TRUE;
	packRotate = FALSE;
	packFillHoles = FALSE;
	packWindowPos.length = 0;

	TVSubObjectMode = 0;
	fillMode = FILL_MODE_FDIAGONAL;

	displayOpenEdges = TRUE;
	thickOpenEdges = TRUE;
	viewportOpenEdges = TRUE;

	uvEdgeMode = FALSE;
	openEdgeMode = FALSE;
	displayHiddenEdges = FALSE;

	freeFormPivotOffset = Point3(0.0f,0.0f,0.0f);
	inRotation = FALSE;

	sketchSelMode = SKETCH_SELDRAG;
	sketchType = SKETCH_LINE;
	restoreSketchSettings = FALSE;
	sketchWindowPos.length = 0;

	sketchInteractiveMode = FALSE;
	sketchDisplayPoints = TRUE;

	hitSize = 4;

	resetPivotOnSel = TRUE;

	polyMode = TRUE;

	allowSelectionInsideGizmo = FALSE;

	showShared = TRUE;

	showIconList.SetSize(32);
	showIconList.SetAll();

	syncSelection = TRUE;
	brightCenterTile = TRUE;
	blendTileToBackGround = FALSE;

	paintSize = 8;

	sketchCursorSize = 8;
	tickSize = 2;

	//new
	gridSize = 0.1f;
	gridSnap = FALSE;
	gridVisible = TRUE;
	gridStr = 0.1f;
	autoMap = FALSE;
	gridColor = RGB(0,0,200);

	preventFlattening = FALSE;

	enableSoftSelection = FALSE;

	for (int i =0;i<16;i++)
		circleCur[i] = NULL;

	BOOL iret;
	TSTR name,category;

	name.printf("%s",GetString(IDS_PW_LINECOLOR));
	category.printf("%s",GetString(IDS_PW_UNWRAPCOLOR));
	iret = ColorMan()->RegisterColor(LINECOLORID,name,category, lineColor);

	name.printf("%s",GetString(IDS_PW_SELCOLOR));
	category.printf("%s",GetString(IDS_PW_UNWRAPCOLOR));
	iret = ColorMan()->RegisterColor(SELCOLORID, name, category, selColor);

	name.printf("%s",GetString(IDS_PW_OPENEDGECOLOR));
	category.printf("%s",GetString(IDS_PW_UNWRAPCOLOR));
	iret = ColorMan()->RegisterColor(OPENEDGECOLORID, name, category, openEdgeColor);

	name.printf("%s",GetString(IDS_PW_HANDLECOLOR));
	category.printf("%s",GetString(IDS_PW_UNWRAPCOLOR));
	iret = ColorMan()->RegisterColor(HANDLECOLORID, name,category, handleColor);

	name.printf("%s",GetString(IDS_PW_FREEFORMCOLOR));
	category.printf("%s",GetString(IDS_PW_UNWRAPCOLOR));
	iret = ColorMan()->RegisterColor(FREEFORMCOLORID, name, category, freeFormColor);

	name.printf("%s",GetString(IDS_PW_SHAREDCOLOR));
	category.printf("%s",GetString(IDS_PW_UNWRAPCOLOR));
	iret = ColorMan()->RegisterColor(SHAREDCOLORID, name, category, sharedColor);

	name.printf("%s",GetString(IDS_PW_BACKGROUNDCOLOR));
	category.printf("%s",GetString(IDS_PW_UNWRAPCOLOR));
	iret = ColorMan()->RegisterColor(BACKGROUNDCOLORID, name, category, backgroundColor);

	//new
	name.printf("%s",GetString(IDS_PW_GRIDCOLOR));
	category.printf("%s",GetString(IDS_PW_UNWRAPCOLOR));
	iret = ColorMan()->RegisterColor(GRIDCOLORID, name, category, gridColor);

	COLORREF geomEdgeColor = RGB(230,0,000);
	name.printf("%s",GetString(IDS_PW_GEOMEDGECOLOR));
	category.printf("%s",GetString(IDS_PW_UNWRAPCOLOR));
	iret = ColorMan()->RegisterColor(GEOMEDGECOLORID, name, category, geomEdgeColor);

	COLORREF geomVertexColor = RGB(230,0,000);
	name.printf("%s",GetString(IDS_PW_GEOMVERTEXCOLOR));
	category.printf("%s",GetString(IDS_PW_UNWRAPCOLOR));
	iret = ColorMan()->RegisterColor(GEOMVERTCOLORID, name, category, geomVertexColor);

	COLORREF peltSeamColor = RGB(13,178,255);
	name.printf("%s",GetString(IDS_PELT_SEAMCOLOR));
	category.printf("%s",GetString(IDS_PW_UNWRAPCOLOR));
	iret = ColorMan()->RegisterColor(PELTSEAMCOLORID, name, category, peltSeamColor);

	COLORREF edgeDistortionColor = RGB(255,0,0);
	name.printf("%s",GetString(IDS_PW_EDGEDISTORTIONGOALCOLOR));
	category.printf("%s",GetString(IDS_PW_UNWRAPCOLOR));
	iret = ColorMan()->RegisterColor(EDGEDISTORTIONGOALCOLORID, name, category, edgeDistortionColor);

	edgeDistortionColor = RGB(0,255,0);
	name.printf("%s",GetString(IDS_PW_EDGEDISTORTIONCOLOR));
	category.printf("%s",GetString(IDS_PW_UNWRAPCOLOR));
	iret = ColorMan()->RegisterColor(EDGEDISTORTIONCOLORID, name, category, edgeDistortionColor);

	edgeDistortionColor = RGB(0,255,0);
	name.printf("%s",GetString(IDS_PW_EDGEDISTORTIONCOLOR));
	category.printf("%s",GetString(IDS_PW_UNWRAPCOLOR));
	iret = ColorMan()->RegisterColor(EDGEDISTORTIONCOLORID, name, category, edgeDistortionColor);

	COLORREF checkerA = RGB(128,128,128);
	name.printf("%s",GetString(IDS_CHECKA));
	category.printf("%s",GetString(IDS_PW_UNWRAPCOLOR));
	iret = ColorMan()->RegisterColor(CHECKERACOLORID, name, category, checkerA);

	COLORREF checkerB = RGB(255,255,255);
	name.printf("%s",GetString(IDS_CHECKB));
	category.printf("%s",GetString(IDS_PW_UNWRAPCOLOR));
	iret = ColorMan()->RegisterColor(CHECKERBCOLORID, name, category, checkerB);

	//	fnLoadDefaults();

	floaterWindowActive = FALSE;

	optionsDialogActive = FALSE;
	applyToWholeObject = FALSE;

	for (int i = 0; i < 90; i++)
		map[i] = NULL;
	checkerMat = NULL;
	originalMat = NULL;

	pblock = NULL;
	unwrapDesc.MakeAutoParamBlocks(this);

	//create our texture reference

	StdMat *stdMat = NewDefaultStdMat();
	stdMat->SetName("UnwrapChecker");
	Texmap *checkerMap = (Texmap *) CreateInstance(TEXMAP_CLASS_ID, Class_ID(CHECKER_CLASS_ID,0));
	StdUVGen *uvGen = (StdUVGen*)checkerMap->GetTheUVGen();
	uvGen->SetUScl(10.0f, 0);
	uvGen->SetVScl(10.0f, 0);
	checkerMap->SetName("CheckerPattern");

	enum { 	checker_blur, checker_color1, checker_color2 };//hack here since we dont expose the param ids for checker but they should be static

	IParamBlock2 *checkerPBlock =  checkerMap->GetParamBlock(0);

	COLORREF cColor = ColorMan()->GetColor(CHECKERACOLORID);
	Point3 color( (float) GetRValue(cColor)/255.0f,
		(float) GetGValue(cColor)/255.0f,
		(float) GetBValue(cColor)/255.0f );

	checkerPBlock->SetValue(checker_color1,0,color);

	COLORREF c2Color = ColorMan()->GetColor(CHECKERBCOLORID);
	Point3 color2( (float) GetRValue(c2Color)/255.0f,
		(float) GetGValue(c2Color)/255.0f,
		(float) GetBValue(c2Color)/255.0f );
	checkerPBlock->SetValue(checker_color2,0,color2);

	stdMat->SetSubTexmap(1, checkerMap);
	pblock->SetValue(unwrap_checkmtl,0,stdMat);
	//	ReplaceReference(100,stdMat);

	matid = -1;
}



//Function Publishing descriptor for Mixin interface
//*****************************************************
static FPInterfaceDesc unwrap_interface(
										UNWRAP_INTERFACE, _T("unwrap"), 0, &unwrapDesc, FP_MIXIN,
										unwrap_planarmap, _T("planarMap"), 0, TYPE_VOID, 0, 0,
										unwrap_save, _T("save"), 0, TYPE_VOID, 0, 0,
										unwrap_load, _T("load"), 0, TYPE_VOID, 0, 0,

										unwrap_reset, _T("reset"), 0, TYPE_VOID, 0, 0,
										unwrap_edit, _T("edit"), 0, TYPE_VOID, 0, 0,

										unwrap_setMapChannel, _T("setMapChannel"),0, TYPE_VOID, 0, 1,
										_T("mapChannel"), 0, TYPE_INT,
										unwrap_getMapChannel, _T("getMapChannel"),0, TYPE_INT, 0, 0,

										unwrap_setProjectionType, _T("setProjectionType"),0, TYPE_VOID, 0, 1,
										_T("mapChannel"), 0, TYPE_INT,
										unwrap_getProjectionType, _T("getProjectionType"),0, TYPE_INT, 0, 0,

										unwrap_setVC, _T("setVC"),0, TYPE_VOID, 0, 1,
										_T("vertexColor"), 0, TYPE_BOOL,
										unwrap_getVC, _T("getVC"),0, TYPE_BOOL, 0, 0,

										unwrap_move, _T("move"),0, TYPE_VOID, 0, 0,
										unwrap_moveh, _T("moveh"),0, TYPE_VOID, 0, 0,
										unwrap_movev, _T("movev"),0, TYPE_VOID, 0, 0,

										unwrap_rotate, _T("rotate"),0, TYPE_VOID, 0, 0,

										unwrap_scale, _T("scale"),0, TYPE_VOID, 0, 0,
										unwrap_scaleh, _T("scaleh"),0, TYPE_VOID, 0, 0,
										unwrap_scalev, _T("scalev"),0, TYPE_VOID, 0, 0,

										unwrap_mirrorh, _T("mirrorH"),0, TYPE_VOID, 0, 0,
										unwrap_mirrorv, _T("mirrorV"),0, TYPE_VOID, 0, 0,

										unwrap_expandsel, _T("expandSelection"),0, TYPE_VOID, 0, 0,
										unwrap_contractsel, _T("contractSelection"),0, TYPE_VOID, 0, 0,

										unwrap_setFalloffType, _T("setFalloffType"),0, TYPE_VOID, 0, 1,
										_T("falloffType"), 0, TYPE_INT,
										unwrap_getFalloffType, _T("getFalloffType"),0, TYPE_INT, 0, 0,
										unwrap_setFalloffSpace, _T("setFalloffSpace"),0, TYPE_VOID, 0, 1,
										_T("falloffSpace"), 0, TYPE_INT,
										unwrap_getFalloffSpace, _T("getFalloffSpace"),0, TYPE_INT, 0, 0,
										unwrap_setFalloffDist, _T("setFalloffDist"),0, TYPE_VOID, 0, 1,
										_T("falloffDist"), 0, TYPE_FLOAT,
										unwrap_getFalloffDist, _T("getFalloffDist"),0, TYPE_FLOAT, 0, 0,

										unwrap_breakselected, _T("breakSelected"),0, TYPE_VOID, 0, 0,
										unwrap_weld, _T("weld"),0, TYPE_VOID, 0, 0,
										unwrap_weldselected, _T("weldSelected"),0, TYPE_VOID, 0, 0,

										unwrap_updatemap, _T("updateMap"),0, TYPE_VOID, 0, 0,
										unwrap_displaymap, _T("DisplayMap"),0, TYPE_VOID, 0, 1,
										_T("displayMap"), 0, TYPE_BOOL,
										unwrap_ismapdisplayed, _T("IsMapDisplayed"),0, TYPE_BOOL, 0, 0,

										unwrap_setuvspace, _T("setUVSpace"),0, TYPE_VOID, 0, 1,
										_T("UVSpace"), 0, TYPE_INT,
										unwrap_getuvspace, _T("getUVSpace"),0, TYPE_INT, 0, 0,

										unwrap_options, _T("options"),0, TYPE_VOID, 0, 0,

										unwrap_lock, _T("lock"),0, TYPE_VOID, 0, 0,

										unwrap_hide, _T("hide"),0, TYPE_VOID, 0, 0,
										unwrap_unhide, _T("unhide"),0, TYPE_VOID, 0, 0,

										unwrap_freeze, _T("freeze"),0, TYPE_VOID, 0, 0,
										unwrap_thaw, _T("unfreeze"),0, TYPE_VOID, 0, 0,
										unwrap_filterselected, _T("filterselected"),0, TYPE_VOID, 0, 0,

										unwrap_pan, _T("pan"),0, TYPE_VOID, 0, 0,
										unwrap_zoom, _T("zoom"),0, TYPE_VOID, 0, 0,
										unwrap_zoomregion, _T("zoomRegion"),0, TYPE_VOID, 0, 0,
										unwrap_fit, _T("fit"),0, TYPE_VOID, 0, 0,
										unwrap_fitselected, _T("fitselected"),0, TYPE_VOID, 0, 0,

										unwrap_snap, _T("snap"),0, TYPE_VOID, 0, 0,

										unwrap_getcurrentmap, _T("getCurrentMap"),0, TYPE_INT, 0, 0,
										unwrap_setcurrentmap, _T("setCurrentMap"),0, TYPE_VOID, 0, 1,
										_T("map"), 0, TYPE_INT,
										unwrap_numbermaps, _T("numberMaps"),0, TYPE_INT, 0, 0,

										unwrap_getlinecolor, _T("getLineColor"),0, TYPE_POINT3, 0, 0,
										unwrap_setlinecolor, _T("setLineColor"),0, TYPE_VOID, 0, 1,
										_T("color"), 0, TYPE_POINT3,

										unwrap_getselectioncolor, _T("getSelectionColor"),0, TYPE_POINT3, 0, 0,
										unwrap_setselectioncolor, _T("setSelectionColor"),0, TYPE_VOID, 0, 1,
										_T("color"), 0, TYPE_POINT3,



										unwrap_getrenderwidth, _T("getRenderWidth"),0, TYPE_INT, 0, 0,
										unwrap_setrenderwidth, _T("setRenderWidth"),0, TYPE_VOID, 0, 1,
										_T("width"), 0, TYPE_INT,
										unwrap_getrenderheight, _T("getRenderHeight"),0, TYPE_INT, 0, 0,
										unwrap_setrenderheight, _T("setRenderHeight"),0, TYPE_VOID, 0, 1,
										_T("height"), 0, TYPE_INT,

										unwrap_getusebitmapres, _T("getUseBitmapRes"),0, TYPE_BOOL, 0, 0,
										unwrap_setusebitmapres, _T("setUseBitmapRes"),0, TYPE_VOID, 0, 1,
										_T("useRes"), 0, TYPE_BOOL,

										unwrap_getweldtheshold, _T("getWeldThreshold"),0, TYPE_FLOAT, 0, 0,
										unwrap_setweldtheshold, _T("setWeldThreshold"),0, TYPE_VOID, 0, 1,
										_T("height"), 0, TYPE_FLOAT,

										unwrap_getconstantupdate, _T("getConstantUpdate"),0, TYPE_BOOL, 0, 0,
										unwrap_setconstantupdate, _T("setConstantUpdate"),0, TYPE_VOID, 0, 1,
										_T("update"), 0, TYPE_BOOL,

										unwrap_getshowselectedvertices, _T("getShowSelectedVertices"),0, TYPE_BOOL, 0, 0,
										unwrap_setshowselectedvertices, _T("setShowSelectedVertices"),0, TYPE_VOID, 0, 1,
										_T("show"), 0, TYPE_BOOL,

										unwrap_getmidpixelsnap, _T("getMidPixelSnap"),0, TYPE_BOOL, 0, 0,
										unwrap_setmidpixelsnap, _T("setMidPixelSnap"),0, TYPE_VOID, 0, 1,
										_T("snap"), 0, TYPE_BOOL,

										unwrap_getmatid, _T("getMatID"),0, TYPE_INT, 0, 0,
										unwrap_setmatid, _T("setMatID"),0, TYPE_VOID, 0, 1,
										_T("matid"), 0, TYPE_INT,
										unwrap_numbermatids, _T("numberMatIDs"),0, TYPE_INT, 0, 0,

										unwrap_getselectedverts, _T("getSelectedVertices"),0,TYPE_BITARRAY,0,0,
										unwrap_selectverts, _T("selectVertices"),0,TYPE_VOID,0,1,
										_T("selection"),0,TYPE_BITARRAY,

										unwrap_isvertexselected, _T("isVertexSelected"),0,TYPE_BOOL,0,1,
										_T("index"),0,TYPE_INT,

										unwrap_moveselectedvertices, _T("MoveSelectedVertices"),0,TYPE_VOID,0,1,
										_T("offset"),0,TYPE_POINT3,

										unwrap_rotateselectedverticesc, _T("RotateSelectedVerticesCenter"),0,TYPE_VOID,0,1,
										_T("angle"),0,TYPE_FLOAT,
										unwrap_rotateselectedvertices, _T("RotateSelectedVertices"),0,TYPE_VOID,0,2,
										_T("angle"),0,TYPE_FLOAT,
										_T("axis"),0,TYPE_POINT3,
										unwrap_scaleselectedverticesc, _T("ScaleSelectedVerticesCenter"),0,TYPE_VOID,0,2,
										_T("scale"),0,TYPE_FLOAT,
										_T("dir"),0,TYPE_INT,
										unwrap_scaleselectedvertices, _T("ScaleSelectedVertices"),0,TYPE_VOID,0,3,
										_T("scale"),0,TYPE_FLOAT,
										_T("dir"),0,TYPE_INT,
										_T("axis"),0,TYPE_POINT3,



										unwrap_getvertexposition, _T("GetVertexPosition"),0,TYPE_POINT3,0,2,
										_T("time"),0,TYPE_TIMEVALUE,
										_T("index"),0,TYPE_INT,
										unwrap_numbervertices, _T("numberVertices"),0,TYPE_INT,0,0,

										unwrap_movex, _T("moveX"),0,TYPE_VOID,0,1,
										_T("p"),0,TYPE_FLOAT,
										unwrap_movey, _T("moveY"),0,TYPE_VOID,0,1,
										_T("p"),0,TYPE_FLOAT,
										unwrap_movez, _T("moveZ"),0,TYPE_VOID,0,1,
										_T("p"),0,TYPE_FLOAT,

										unwrap_getselectedpolygons,_T("getSelectedPolygons"),0,TYPE_BITARRAY,0,0,
										unwrap_selectpolygons, _T("selectPolygons"),0,TYPE_VOID,0,1,
										_T("selection"),0,TYPE_BITARRAY,
										unwrap_ispolygonselected, _T("isPolygonSelected"),0,TYPE_BOOL,0,1,
										_T("index"),0,TYPE_INT,
										unwrap_numberpolygons, _T("numberPolygons"),0, TYPE_INT, 0, 0,
										unwrap_detachedgeverts, _T("detachEdgeVertices"),0, TYPE_VOID, 0, 0,

										unwrap_fliph, _T("flipHorizontal"),0, TYPE_VOID, 0, 0,
										unwrap_flipv, _T("flipVertical"),0, TYPE_VOID, 0, 0,

										unwrap_getlockaspect, _T("getLockAspect"),0, TYPE_BOOL, 0, 0,
										unwrap_setlockaspect, _T("setLockAspect"),0, TYPE_VOID, 0, 1,
										_T("aspect"), 0, TYPE_BOOL,

										unwrap_getmapscale, _T("getMapScale"),0, TYPE_FLOAT, 0, 0,
										unwrap_setmapscale, _T("setMapScale"),0, TYPE_VOID, 0, 1,
										_T("scale"), 0, TYPE_FLOAT,

										unwrap_getselectionfromface, _T("getSelectionFromFace"),0, TYPE_VOID, 0, 0,

										unwrap_forceupdate, _T("forceUpdate"),0, TYPE_VOID, 0, 1,
										_T("update"), 0, TYPE_BOOL,

										unwrap_zoomtogizmo, _T("zoomToGizmo"),0, TYPE_VOID, 0, 1,
										_T("all"), 0, TYPE_BOOL,

										unwrap_setvertexposition, _T("setVertexPosition"),0,TYPE_VOID,0,3,
										_T("time"), 0, TYPE_TIMEVALUE,
										_T("index"), 0, TYPE_INT,
										_T("pos"), 0, TYPE_POINT3,
										unwrap_markasdead, _T("markAsDead"),0,TYPE_VOID,0,1,
										_T("index"), 0, TYPE_INT,

										unwrap_numberpointsinface, _T("numberPointsInFace"),0,TYPE_INT,0,1,
										_T("index"), 0, TYPE_INT,

										unwrap_getvertexindexfromface, _T("getVertexIndexFromFace"),0,TYPE_INT,0,2,
										_T("faceIndex"), 0, TYPE_INT,
										_T("ithVertex"), 0, TYPE_INT,
										unwrap_gethandleindexfromface, _T("getHandleIndexFromFace"),0,TYPE_INT,0,2,
										_T("faceIndex"), 0, TYPE_INT,
										_T("ithVertex"), 0, TYPE_INT,
										unwrap_getinteriorindexfromface, _T("getInteriorIndexFromFace"),0,TYPE_INT,0,2,
										_T("faceIndex"), 0, TYPE_INT,
										_T("ithVertex"), 0, TYPE_INT,

										unwrap_getvertexgindexfromface, _T("getVertexGeomIndexFromFace"),0,TYPE_INT,0,2,
										_T("faceIndex"), 0, TYPE_INT,
										_T("ithVertex"), 0, TYPE_INT,
										unwrap_gethandlegindexfromface, _T("getHandleGeomIndexFromFace"),0,TYPE_INT,0,2,
										_T("faceIndex"), 0, TYPE_INT,
										_T("ithVertex"), 0, TYPE_INT,
										unwrap_getinteriorgindexfromface, _T("getInteriorGeomIndexFromFace"),0,TYPE_INT,0,2,
										_T("faceIndex"), 0, TYPE_INT,
										_T("ithVertex"), 0, TYPE_INT,

										unwrap_addpointtoface, _T("setFaceVertex"),0,TYPE_VOID,0,4,
										_T("pos"), 0, TYPE_POINT3,
										_T("faceIndex"), 0, TYPE_INT,
										_T("ithVertex"), 0, TYPE_INT,
										_T("sel"), 0, TYPE_BOOL,
										unwrap_addpointtohandle, _T("setFaceHandle"),0,TYPE_VOID,0,4,
										_T("pos"), 0, TYPE_POINT3,
										_T("faceIndex"), 0, TYPE_INT,
										_T("ithVertex"), 0, TYPE_INT,
										_T("sel"), 0, TYPE_BOOL,

										unwrap_addpointtointerior, _T("setFaceInterior"),0,TYPE_VOID,0,4,
										_T("pos"), 0, TYPE_POINT3,
										_T("faceIndex"), 0, TYPE_INT,
										_T("ithVertex"), 0, TYPE_INT,
										_T("sel"), 0, TYPE_BOOL,

										unwrap_setfacevertexindex, _T("setFaceVertexIndex"),0,TYPE_VOID,0,3,
										_T("faceIndex"), 0, TYPE_INT,
										_T("ithVertex"), 0, TYPE_INT,
										_T("vertexIndex"), 0, TYPE_INT,
										unwrap_setfacehandleindex, _T("setFaceHandleIndex"),0,TYPE_VOID,0,3,
										_T("faceIndex"), 0, TYPE_INT,
										_T("ithVertex"), 0, TYPE_INT,
										_T("vertexIndex"), 0, TYPE_INT,
										unwrap_setfaceinteriorindex, _T("setFaceInteriorIndex"),0,TYPE_VOID,0,3,
										_T("faceIndex"), 0, TYPE_INT,
										_T("ithVertex"), 0, TYPE_INT,
										_T("vertexIndex"), 0, TYPE_INT,
										unwrap_updateview, _T("updateView"),0,TYPE_VOID,0,0,
										unwrap_getfaceselfromstack, _T("getFaceSelectionFromStack"),0,TYPE_VOID,0,0,




										end
										);



static FPInterfaceDesc unwrap_interface2(
	UNWRAP_INTERFACE2, _T("unwrap2"), 0, &unwrapDesc, FP_MIXIN,

	//UNFOLD STUFF

	unwrap_selectpolygonsupdate, _T("selectPolygonsUpdate"),0,TYPE_VOID,0,2,
	_T("selection"),0,TYPE_BITARRAY,
	_T("update"),0,TYPE_BOOL,
	unwrap_selectfacesbynormal, _T("selectFacesByNormal"), 0, TYPE_VOID, 0, 3,
	_T("normal"), 0, TYPE_POINT3,
	_T("threshold"), 0, TYPE_FLOAT,
	_T("update"),0,TYPE_BOOL,
	unwrap_selectclusterbynormal, _T("selectClusterByNormal"), 0, TYPE_VOID, 0, 4,
	_T("threshold"), 0, TYPE_FLOAT,
	_T("faceIndexSeed"), 0, TYPE_INT,
	_T("relative"),0,TYPE_BOOL,
	_T("update"),0,TYPE_BOOL,

	unwrap_flattenmap, _T("flattenMap"), 0, TYPE_VOID, 0, 7,
	_T("angleThreshold"), 0, TYPE_FLOAT,
	_T("normalList"), 0, TYPE_POINT3_TAB,
	_T("spacing"), 0, TYPE_FLOAT,
	_T("normalize"), 0, TYPE_BOOL,
	_T("layOutType"), 0, TYPE_INT,
	_T("rotateClusters"), 0, TYPE_BOOL,
	_T("fillHoles"), 0, TYPE_BOOL,


	unwrap_normalmap, _T("normalMap"),0, TYPE_VOID, 0, 6,
	_T("normalList"), 0, TYPE_POINT3_TAB,
	_T("spacing"), 0, TYPE_FLOAT,
	_T("normalize"), 0, TYPE_BOOL,
	_T("layOutType"), 0, TYPE_INT,
	_T("rotateClusters"), 0, TYPE_BOOL,
	_T("alignWidth"), 0, TYPE_BOOL,
	unwrap_normalmapnoparams, _T("normalMapNoParams"),0, TYPE_VOID, 0, 0,
	unwrap_normalmapdialog, _T("normalMapDialog"),0, TYPE_VOID, 0, 0,

	unwrap_unfoldmap, _T("unfoldMap"), 0, TYPE_VOID, 0, 1,
	_T("unfoldMethod"), 0, TYPE_INT,
	unwrap_unfoldmapnoparams, _T("unfoldMapNoParams"), 0, TYPE_VOID, 0, 0,
	unwrap_unfoldmapdialog, _T("unfoldMapDialog"), 0, TYPE_VOID, 0, 0,

	unwrap_hideselectedpolygons, _T("hideSelectedPolygons"), 0, TYPE_VOID, 0, 0,
	unwrap_unhideallpolygons, _T("unhideAllPolygons"), 0, TYPE_VOID, 0, 0,


	unwrap_getnormal, _T("getNormal"),0,TYPE_POINT3,0,1,
	_T("faceIndex"), 0, TYPE_INT,
	unwrap_setseedface, _T("setSeedFace"),0, TYPE_VOID, 0, 0,
	unwrap_showvertexconnectionlist, _T("toggleVertexConnection"),0, TYPE_VOID, 0, 0,
	//COPYPASTE STUF
	unwrap_copy, _T("copy"),0, TYPE_VOID, 0, 0,
	unwrap_paste, _T("paste"),0, TYPE_VOID, 0, 1,
	_T("rotate"), 0, TYPE_BOOL,

	unwrap_pasteinstance, _T("pasteInstance"),0, TYPE_VOID, 0, 0,

	unwrap_setdebuglevel, _T("setDebugLevel"), 0, TYPE_VOID, 0, 1,
	_T("level"), 0, TYPE_INT,

	unwrap_stitchverts, _T("stitchVerts"), 0, TYPE_VOID, 0, 2,
	_T("align"), 0, TYPE_BOOL,
	_T("bias"), 0, TYPE_FLOAT,

	unwrap_stitchvertsnoparams, _T("stitchVertsNoParams"), 0, TYPE_VOID, 0, 0,
	unwrap_stitchvertsdialog, _T("stitchVertsDialog"), 0, TYPE_VOID, 0, 0,

	unwrap_selectelement, _T("selectElement"), 0, TYPE_VOID, 0, 0,

	unwrap_flattenmapdialog, _T("flattenMapDialog"), 0, TYPE_VOID, 0, 0,
	unwrap_flattenmapnoparams, _T("flattenMapNoParams"), 0, TYPE_VOID, 0, 0,

	//TILE STUFF
	unwrap_gettilemap, _T("getTileMap"), 0, TYPE_BOOL, 0, 0,
	unwrap_settilemap, _T("setTileMap"), 0, TYPE_VOID, 0, 1,
	_T("tile"), 0, TYPE_BOOL,

	unwrap_gettilemaplimit, _T("getTileMapLimit"), 0, TYPE_INT, 0, 0,
	unwrap_settilemaplimit, _T("setTileMapLimit"), 0, TYPE_VOID, 0, 1,
	_T("limit"), 0, TYPE_INT,

	unwrap_gettilemapcontrast, _T("getTileMapBrightness"), 0, TYPE_FLOAT, 0, 0,
	unwrap_settilemapcontrast, _T("setTileMapBrightness"), 0, TYPE_VOID, 0, 1,
	_T("contrast"), 0, TYPE_FLOAT,

	unwrap_getshowmap, _T("getShowMap"), 0, TYPE_BOOL, 0, 0,
	unwrap_setshowmap, _T("setShowMap"), 0, TYPE_VOID, 0, 1,
	_T("showMap"), 0, TYPE_BOOL,


	unwrap_setlimitsoftsel, _T("getLimitSoftSel"), 0, TYPE_BOOL, 0, 0,
	unwrap_getlimitsoftsel, _T("setLimitSoftSel"), 0, TYPE_VOID, 0, 1,
	_T("limit"), 0, TYPE_BOOL,

	unwrap_setlimitsoftselrange, _T("getLimitSoftSelRange"), 0, TYPE_INT, 0, 0,
	unwrap_getlimitsoftselrange, _T("setLimitSoftSelRange"), 0, TYPE_VOID, 0, 1,
	_T("range"), 0, TYPE_INT,


	unwrap_getvertexweight, _T("getVertexWeight"), 0, TYPE_FLOAT, 0, 1,
	_T("index"), 0, TYPE_INT,
	unwrap_setvertexweight, _T("setVertexWeight"), 0, TYPE_VOID, 0, 2,
	_T("index"), 0, TYPE_INT,
	_T("weight"), 0, TYPE_FLOAT,

	unwrap_isweightmodified, _T("isWeightModified"), 0, TYPE_BOOL, 0, 1,
	_T("index"), 0, TYPE_INT,
	unwrap_modifyweight, _T("modifyWeight"), 0, TYPE_VOID, 0, 2,
	_T("index"), 0, TYPE_INT,
	_T("modify"), 0, TYPE_BOOL,


	unwrap_getgeom_elemmode, _T("getGeomSelectElementMode"), 0, TYPE_BOOL, 0, 0,
	unwrap_setgeom_elemmode, _T("setGeomSelectElementMode"), 0, TYPE_VOID, 0, 1,
	_T("mode"), 0, TYPE_BOOL,


	unwrap_getgeom_planarmode, _T("getGeomPlanarThresholdMode"), 0, TYPE_BOOL, 0, 0,
	unwrap_setgeom_planarmode, _T("setGeomPlanarThresholdMode"), 0, TYPE_VOID, 0, 1,
	_T("mode"), 0, TYPE_BOOL,

	unwrap_getgeom_planarmodethreshold, _T("getGeomPlanarThreshold"), 0, TYPE_FLOAT, 0, 0,
	unwrap_setgeom_planarmodethreshold, _T("setGeomPlanarThreshold"), 0, TYPE_VOID, 0, 1,
	_T("angle"), 0, TYPE_FLOAT,


	unwrap_getwindowx, _T("getWindowX"), 0, TYPE_INT, 0, 0,
	unwrap_getwindowy, _T("getWindowY"), 0, TYPE_INT, 0, 0,
	unwrap_getwindoww, _T("getWindowW"), 0, TYPE_INT, 0, 0,
	unwrap_getwindowh, _T("getWindowH"), 0, TYPE_INT, 0, 0,


	unwrap_getbackfacecull, _T("getIgnoreBackFaceCull"), 0, TYPE_BOOL, 0, 0,
	unwrap_setbackfacecull, _T("setIgnoreBackFaceCull"), 0, TYPE_VOID, 0, 1,
	_T("ignoreBackFaceCull"), 0, TYPE_BOOL,

	unwrap_getoldselmethod, _T("getOldSelMethod"), 0, TYPE_BOOL, 0, 0,
	unwrap_setoldselmethod, _T("setOldSelMethod"), 0, TYPE_VOID, 0, 1,
	_T("oldSelMethod"), 0, TYPE_BOOL,

	unwrap_selectbymatid, _T("selectByMatID"), 0, TYPE_VOID, 0, 1,
	_T("matID"), 0, TYPE_INT,

	unwrap_selectbysg, _T("selectBySG"), 0, TYPE_VOID, 0, 1,
	_T("sg"), 0, TYPE_INT,

	unwrap_gettvelementmode, _T("getTVElementMode"), 0, TYPE_BOOL, 0, 0,
	unwrap_settvelementmode, _T("setTVElementMode"), 0, TYPE_VOID, 0, 1,
	_T("mode"), 0, TYPE_BOOL,


	unwrap_geomexpandsel, _T("expandGeomFaceSelection"),0, TYPE_VOID, 0, 0,
	unwrap_geomcontractsel, _T("contractGeomFaceSelection"),0, TYPE_VOID, 0, 0,


	unwrap_getalwaysedit, _T("getAlwaysEdit"), 0, TYPE_BOOL, 0, 0,
	unwrap_setalwaysedit, _T("setAlwaysEdit"), 0, TYPE_VOID, 0, 1,
	_T("always"), 0, TYPE_BOOL,

	unwrap_getshowvertexconnectionlist, _T("getShowVertexConnections"), 0, TYPE_BOOL, 0, 0,
	unwrap_setshowvertexconnectionlist, _T("setShowVertexConnections"), 0, TYPE_VOID, 0, 1,
	_T("show"), 0, TYPE_BOOL,

	unwrap_getfilterselected, _T("getFilterSelected"), 0, TYPE_BOOL, 0, 0,
	unwrap_setfilterselected, _T("setFilterSelected"), 0, TYPE_VOID, 0, 1,
	_T("filter"), 0, TYPE_BOOL,

	unwrap_getsnap, _T("getSnap"), 0, TYPE_BOOL, 0, 0,
	unwrap_setsnap, _T("setSnap"), 0, TYPE_VOID, 0, 1,
	_T("snap"), 0, TYPE_BOOL,

	unwrap_getlock, _T("getLock"), 0, TYPE_BOOL, 0, 0,
	unwrap_setlock, _T("setLock"), 0, TYPE_VOID, 0, 1,
	_T("lock"), 0, TYPE_BOOL,

	unwrap_pack, _T("pack"), 0, TYPE_VOID, 0, 5,
	_T("method"), 0, TYPE_INT,
	_T("spacing"), 0, TYPE_FLOAT,
	_T("normalize"), 0, TYPE_BOOL,
	_T("rotate"), 0, TYPE_BOOL,
	_T("fillholes"), 0, TYPE_BOOL,

	unwrap_packnoparams, _T("packNoParams"), 0, TYPE_VOID, 0, 0,
	unwrap_packdialog, _T("packDialog"), 0, TYPE_VOID, 0, 0,


	unwrap_gettvsubobjectmode, _T("getTVSubObjectMode"), 0, TYPE_INT, 0, 0,
	unwrap_settvsubobjectmode, _T("setTVSubObjectMode"), 0, TYPE_VOID, 0, 1,
	_T("mode"), 0, TYPE_INT,


	unwrap_getselectedfaces, _T("getSelectedFaces"),0,TYPE_BITARRAY,0,0,
	unwrap_selectfaces, _T("selectFaces"),0,TYPE_VOID,0,1,
	_T("selection"),0,TYPE_BITARRAY,

	unwrap_isfaceselected, _T("isFaceSelected"),0,TYPE_BOOL,0,1,
	_T("index"),0,TYPE_INT,

	unwrap_getfillmode, _T("getFillMode"), 0, TYPE_INT, 0, 0,
	unwrap_setfillmode, _T("setFillMode"), 0, TYPE_VOID, 0, 1,
	_T("mode"), 0, TYPE_INT,

	unwrap_moveselected, _T("MoveSelected"),0,TYPE_VOID,0,1,
	_T("offset"),0,TYPE_POINT3,

	unwrap_rotateselectedc, _T("RotateSelectedCenter"),0,TYPE_VOID,0,1,
	_T("angle"),0,TYPE_FLOAT,
	unwrap_rotateselected, _T("RotateSelected"),0,TYPE_VOID,0,2,
	_T("angle"),0,TYPE_FLOAT,
	_T("axis"),0,TYPE_POINT3,
	unwrap_scaleselectedc, _T("ScaleSelectedCenter"),0,TYPE_VOID,0,2,
	_T("scale"),0,TYPE_FLOAT,
	_T("dir"),0,TYPE_INT,
	unwrap_scaleselected, _T("ScaleSelected"),0,TYPE_VOID,0,3,
	_T("scale"),0,TYPE_FLOAT,
	_T("dir"),0,TYPE_INT,
	_T("axis"),0,TYPE_POINT3,

	unwrap_getselectededges, _T("getSelectedEdges"),0,TYPE_BITARRAY,0,0,
	unwrap_selectedges, _T("selectEdges"),0,TYPE_VOID,0,1,
	_T("selection"),0,TYPE_BITARRAY,

	unwrap_isedgeselected, _T("isEdgeSelected"),0,TYPE_BOOL,0,1,
	_T("index"),0,TYPE_INT,


	unwrap_getdisplayopenedge, _T("getDisplayOpenEdges"), 0, TYPE_BOOL, 0, 0,
	unwrap_getdisplayopenedge, _T("setDisplayOpenEdges"), 0, TYPE_VOID, 0, 1,
	_T("displayOpenEdges"), 0, TYPE_BOOL,



	unwrap_getopenedgecolor, _T("getOpenEdgeColor"),0, TYPE_POINT3, 0, 0,
	unwrap_setopenedgecolor, _T("setOpenEdgeColor"),0, TYPE_VOID, 0, 1,
	_T("color"), 0, TYPE_POINT3,

	unwrap_getuvedgemode, _T("getUVEdgeMode"), 0, TYPE_BOOL, 0, 0,
	unwrap_setuvedgemode, _T("setUVEdgeMode"), 0, TYPE_VOID, 0, 1,
	_T("uvEdgeMode"), 0, TYPE_BOOL,

	unwrap_getopenedgemode, _T("getOpenEdgeMode"), 0, TYPE_BOOL, 0, 0,
	unwrap_setopenedgemode, _T("setOpenEdgeMode"), 0, TYPE_VOID, 0, 1,
	_T("uvOpenMode"), 0, TYPE_BOOL,


	unwrap_uvedgeselect, _T("uvEdgeSelect"), 0, TYPE_VOID, 0, 0,
	unwrap_openedgeselect, _T("openEdgeSelect"), 0, TYPE_VOID, 0, 0,


	unwrap_selectverttoedge, _T("vertToEdgeSelect"), 0, TYPE_VOID, 0, 0,
	unwrap_selectverttoface, _T("vertToFaceSelect"), 0, TYPE_VOID, 0, 0,

	unwrap_selectedgetovert, _T("edgeToVertSelect"), 0, TYPE_VOID, 0, 0,
	unwrap_selectedgetoface, _T("edgeToFaceSelect"), 0, TYPE_VOID, 0, 0,

	unwrap_selectfacetovert, _T("faceToVertSelect"), 0, TYPE_VOID, 0, 0,
	unwrap_selectfacetoedge, _T("faceToEdgeSelect"), 0, TYPE_VOID, 0, 0,

	unwrap_getdisplayhiddenedge, _T("getDisplayHiddenEdges"), 0, TYPE_BOOL, 0, 0,
	unwrap_setdisplayhiddenedge, _T("setDisplayHiddenEdges"), 0, TYPE_VOID, 0, 1,
	_T("displayHiddenEdges"), 0, TYPE_BOOL,

	unwrap_gethandlecolor, _T("getHandleColor"),0, TYPE_POINT3, 0, 0,
	unwrap_sethandlecolor, _T("setHandleColor"),0, TYPE_VOID, 0, 1,
	_T("color"), 0, TYPE_POINT3,

	unwrap_getfreeformmode, _T("getFreeFormMode"), 0, TYPE_BOOL, 0, 0,
	unwrap_setfreeformmode, _T("setFreeFormMode"), 0, TYPE_VOID, 0, 1,
	_T("freeFormMode"), 0, TYPE_BOOL,

	unwrap_getfreeformcolor, _T("getFreeFormColor"),0, TYPE_POINT3, 0, 0,
	unwrap_setfreeformcolor, _T("setFreeFormColor"),0, TYPE_VOID, 0, 1,
	_T("color"), 0, TYPE_POINT3,

	unwrap_scaleselectedxy, _T("ScaleSelectedXY"),0,TYPE_VOID,0,3,
	_T("scaleX"),0,TYPE_FLOAT,
	_T("scaleY"),0,TYPE_FLOAT,
	_T("axis"),0,TYPE_POINT3,

	unwrap_snappivot, _T("snapPivot"),0,TYPE_VOID,0,1,
	_T("pos"),0,TYPE_INT,

	unwrap_getpivotoffset, _T("getPivotOffset"),0, TYPE_POINT3, 0, 0,
	unwrap_setpivotoffset, _T("setPivotOffset"),0, TYPE_VOID, 0, 1,
	_T("offset"), 0, TYPE_POINT3,
	unwrap_getselcenter, _T("getSelCenter"),0, TYPE_POINT3, 0, 0,

	unwrap_sketch, _T("sketch"),0, TYPE_VOID, 0, 2,
	_T("indexList"),0,TYPE_INT_TAB,
	_T("positionList"),0,TYPE_POINT3_TAB,
	unwrap_sketchnoparams, _T("sketchNoParams"),0, TYPE_VOID, 0, 0,
	unwrap_sketchdialog, _T("sketchDialog"),0, TYPE_VOID, 0, 0,
	unwrap_sketchreverse, _T("sketchReverse"),0, TYPE_VOID, 0, 0,


	unwrap_gethitsize, _T("getHitSize"), 0, TYPE_INT, 0, 0,
	unwrap_sethitsize, _T("SetHitSize"), 0, TYPE_VOID, 0, 1,
	_T("size"), 0, TYPE_INT,


	unwrap_getresetpivotonsel, _T("getResetPivotOnSelection"), 0, TYPE_BOOL, 0, 0,
	unwrap_setresetpivotonsel, _T("SetResetPivotOnSelection"), 0, TYPE_VOID, 0, 1,
	_T("reset"), 0, TYPE_BOOL,

	unwrap_getpolymode, _T("getPolygonMode"), 0, TYPE_BOOL, 0, 0,
	unwrap_setpolymode, _T("setPolygonMode"), 0, TYPE_VOID, 0, 1,
	_T("mode"), 0, TYPE_BOOL,
	unwrap_polyselect, _T("PolygonSelect"), 0, TYPE_VOID, 0, 0,



	unwrap_getselectioninsidegizmo, _T("getAllowSelectionInsideGizmo"), 0, TYPE_BOOL, 0, 0,
	unwrap_setselectioninsidegizmo, _T("SetAllowSelectionInsideGizmo"), 0, TYPE_VOID, 0, 1,
	_T("select"), 0, TYPE_BOOL,



	unwrap_setasdefaults, _T("SaveCurrentSettingsAsDefault"), 0, TYPE_VOID, 0, 0,
	unwrap_loaddefaults, _T("LoadDefault"), 0, TYPE_VOID, 0, 0,



	unwrap_getshowshared, _T("getShowShared"), 0, TYPE_BOOL, 0, 0,
	unwrap_setshowshared, _T("setShowShared"), 0, TYPE_VOID, 0, 1,
	_T("select"), 0, TYPE_BOOL,


	unwrap_getsharedcolor, _T("getSharedColor"),0, TYPE_POINT3, 0, 0,
	unwrap_setsharedcolor, _T("setSharedColor"),0, TYPE_VOID, 0, 1,
	_T("color"), 0, TYPE_POINT3,


	unwrap_showicon, _T("showIcon"), 0, TYPE_VOID, 0, 2,
	_T("index"), 0, TYPE_INT,
	_T("show"), 0, TYPE_BOOL,


	unwrap_getsyncselectionmode, _T("getSyncSelectionMode"), 0, TYPE_BOOL, 0, 0,
	unwrap_setsyncselectionmode, _T("setSyncSelectionMode"), 0, TYPE_VOID, 0, 1,
	_T("sync"), 0, TYPE_BOOL,


	unwrap_synctvselection, _T("syncTVSelection"), 0, TYPE_VOID, 0, 0,
	unwrap_syncgeomselection, _T("syncGeomSelection"), 0, TYPE_VOID, 0, 0,

	unwrap_getbackgroundcolor, _T("getBackgroundColor"),0, TYPE_POINT3, 0, 0,
	unwrap_setbackgroundcolor, _T("setBackgroundColor"),0, TYPE_VOID, 0, 1,
	_T("color"), 0, TYPE_POINT3,

	unwrap_updatemenubar, _T("updateMenuBar"),0, TYPE_VOID, 0, 0,


	unwrap_getbrightcentertile, _T("getBrightnessAffectsCenterTile"), 0, TYPE_BOOL, 0, 0,
	unwrap_setbrightcentertile, _T("setBrightnessAffectsCenterTile"), 0, TYPE_VOID, 0, 1,
	_T("bright"), 0, TYPE_BOOL,

	unwrap_getblendtoback, _T("getBlendTileToBackground"), 0, TYPE_BOOL, 0, 0,
	unwrap_setblendtoback, _T("setBlendTileToBackground"), 0, TYPE_VOID, 0, 1,
	_T("blend"), 0, TYPE_BOOL,

	unwrap_getpaintmode, _T("getPaintSelectMode"), 0, TYPE_BOOL, 0, 0,
	unwrap_setpaintmode, _T("setPaintSelectMode"), 0, TYPE_VOID, 0, 1,
	_T("paint"), 0, TYPE_BOOL,

	unwrap_getpaintsize, _T("getPaintSelectSize"), 0, TYPE_INT, 0, 0,
	unwrap_setpaintsize, _T("setPaintSelectSize"), 0, TYPE_VOID, 0, 1,
	_T("size"), 0, TYPE_INT,

	unwrap_incpaintsize, _T("PaintSelectIncSize"), 0, TYPE_VOID, 0, 0,
	unwrap_decpaintsize, _T("PaintSelectDecSize"), 0, TYPE_VOID, 0, 0,


	unwrap_getticksize, _T("getTickSize"), 0, TYPE_INT, 0, 0,
	unwrap_setticksize, _T("setTickSize"), 0, TYPE_VOID, 0, 1,
	_T("size"), 0, TYPE_INT,


	//new
	unwrap_getgridsize, _T("getGridSize"), 0, TYPE_FLOAT, 0, 0,
	unwrap_setgridsize, _T("setGridSize"), 0, TYPE_VOID, 0, 1,
	_T("size"), 0, TYPE_FLOAT,

	unwrap_getgridsnap, _T("getGridSnap"), 0, TYPE_BOOL, 0, 0,
	unwrap_setgridsnap, _T("setGridSnap"), 0, TYPE_VOID, 0, 1,
	_T("snap"), 0, TYPE_BOOL,
	unwrap_getgridvisible, _T("getGridVisible"), 0, TYPE_BOOL, 0, 0,
	unwrap_setgridvisible, _T("setGridVisible"), 0, TYPE_VOID, 0, 1,
	_T("visible"), 0, TYPE_BOOL,

	unwrap_getgridcolor, _T("getGridColor"),0, TYPE_POINT3, 0, 0,
	unwrap_setgridcolor, _T("setGridColor"),0, TYPE_VOID, 0, 1,
	_T("color"), 0, TYPE_POINT3,

	unwrap_getgridstr, _T("getGridStr"), 0, TYPE_FLOAT, 0, 0,
	unwrap_setgridstr, _T("setGridStr"), 0, TYPE_VOID, 0, 1,
	_T("str"), 0, TYPE_FLOAT,

	unwrap_getautomap, _T("getAutoBackground"), 0, TYPE_BOOL, 0, 0,
	unwrap_setautomap, _T("setAutoBackground"), 0, TYPE_VOID, 0, 1,
	_T("enable"), 0, TYPE_BOOL,


	unwrap_getflattenangle, _T("getFlattenAngle"), 0, TYPE_FLOAT, 0, 0,
	unwrap_setflattenangle, _T("setFlattenAngle"), 0, TYPE_VOID, 0, 1,
	_T("angle"), 0, TYPE_FLOAT,

	unwrap_getflattenspacing, _T("getFlattenSpacing"), 0, TYPE_FLOAT, 0, 0,
	unwrap_setflattenspacing, _T("setFlattenSpacing"), 0, TYPE_VOID, 0, 1,
	_T("spacing"), 0, TYPE_FLOAT,

	unwrap_getflattennormalize, _T("getFlattenNormalize"), 0, TYPE_BOOL, 0, 0,
	unwrap_setflattennormalize, _T("setFlattenNormalize"), 0, TYPE_VOID, 0, 1,
	_T("normalize"), 0, TYPE_BOOL,

	unwrap_getflattenrotate, _T("getFlattenRotate"), 0, TYPE_BOOL, 0, 0,
	unwrap_setflattenrotate, _T("setFlattenRotate"), 0, TYPE_VOID, 0, 1,
	_T("rotate"), 0, TYPE_BOOL,

	unwrap_getflattenfillholes, _T("getFlattenFillHoles"), 0, TYPE_BOOL, 0, 0,
	unwrap_setflattenfillholes, _T("setFlattenFillHoles"), 0, TYPE_VOID, 0, 1,
	_T("fillHoles"), 0, TYPE_BOOL,

	unwrap_getpreventflattening, _T("getPreventFlattening"), 0, TYPE_BOOL, 0, 0,
	unwrap_setpreventflattening, _T("setPreventFlattening"), 0, TYPE_VOID, 0, 1,
	_T("preventFlattening"), 0, TYPE_BOOL,

	unwrap_getenablesoftselection, _T("getEnableSoftSelection"), 0, TYPE_BOOL, 0, 0,
	unwrap_setenablesoftselection, _T("setEnableSoftSelection"), 0, TYPE_VOID, 0, 1,
	_T("enable"), 0, TYPE_BOOL,

	unwrap_getapplytowholeobject, _T("getApplyToWholeObject"), 0, TYPE_BOOL, 0, 0,
	unwrap_setapplytowholeobject, _T("setApplyToWholeObject"), 0, TYPE_VOID, 0, 1,
	_T("whole"), 0, TYPE_BOOL,


	unwrap_setvertexposition2, _T("setVertexPosition2"),0,TYPE_VOID,0,5,
	_T("time"), 0, TYPE_TIMEVALUE,
	_T("index"), 0, TYPE_INT,
	_T("pos"), 0, TYPE_POINT3,
	_T("hold"), 0, TYPE_BOOL,
	_T("update"), 0, TYPE_BOOL,


	unwrap_relax, _T("relax"),0,TYPE_VOID,0,4,
	_T("iterations"), 0, TYPE_INT,
	_T("strength"), 0, TYPE_FLOAT,
	_T("lockEdges"), 0, TYPE_BOOL,
	_T("matchArea"), 0, TYPE_BOOL,

	unwrap_fitrelax, _T("fitRelax"),0,TYPE_VOID,0,2,
	_T("iterations"), 0, TYPE_INT,
	_T("strength"), 0, TYPE_FLOAT,


	end
	);

//5.1.05
static FPInterfaceDesc unwrap_interface3(
	UNWRAP_INTERFACE3, _T("unwrap3"), 0, &unwrapDesc, FP_MIXIN,

	//UNFOLD STUFF

	unwrap_getautobackground, _T("getAutoBackground"),0,TYPE_BOOL,0,0,
	unwrap_setautobackground, _T("setAutoBackground"), 0, TYPE_VOID, 0, 1,
	_T("autoBackground"), 0, TYPE_BOOL,

	unwrap_getrelaxamount, _T("getRelaxAmount"),0,TYPE_FLOAT,0,0,
	unwrap_setrelaxamount, _T("setRelaxAmount"), 0, TYPE_VOID, 0, 1,
	_T("amount"), 0, TYPE_FLOAT,

	unwrap_getrelaxiter, _T("getRelaxIteration"),0,TYPE_INT,0,0,
	unwrap_setrelaxiter, _T("setRelaxIteration"), 0, TYPE_VOID, 0, 1,
	_T("amount"), 0, TYPE_INT,


	unwrap_getrelaxboundary, _T("getRelaxBoundary"),0,TYPE_BOOL,0,0,
	unwrap_setrelaxboundary, _T("setRelaxBoundary"), 0, TYPE_VOID, 0, 1,
	_T("boundary"), 0, TYPE_BOOL,

	unwrap_getrelaxsaddle, _T("getRelaxSaddle"),0,TYPE_BOOL,0,0,
	unwrap_setrelaxsaddle, _T("setRelaxSaddle"), 0, TYPE_VOID, 0, 1,
	_T("saddle"), 0, TYPE_BOOL,


	unwrap_relax2, _T("relax2"), 0, TYPE_VOID, 0, 0,
	unwrap_relax2dialog, _T("relax2dialog"), 0, TYPE_VOID, 0, 0,

	end
	);

static FPInterfaceDesc unwrap_interface4(
	UNWRAP_INTERFACE4, _T("unwrap4"), 0, &unwrapDesc, FP_MIXIN,

	//UNFOLD STUFF

	unwrap_getthickopenedges, _T("getThickOpenEdges"),0,TYPE_BOOL,0,0,
	unwrap_setthickopenedges, _T("setThickOpenEdges"), 0, TYPE_VOID, 0, 1,
	_T("thick"), 0, TYPE_BOOL,

	unwrap_getviewportopenedges, _T("getViewportOpenEdges"),0,TYPE_BOOL,0,0,
	unwrap_setviewportopenedges, _T("setViewportOpenEdges"), 0, TYPE_VOID, 0, 1,
	_T("show"), 0, TYPE_BOOL,

	unwrap_selectinvertedfaces, _T("selectInvertedFaces"), 0, TYPE_VOID, 0, 0,

	unwrap_getrelativetypein, _T("getRelativeTypeIn"),0,TYPE_BOOL,0,0,
	unwrap_setrelativetypein, _T("setRelativeTypeIn"), 0, TYPE_VOID, 0, 1,
	_T("relative"), 0, TYPE_BOOL,

	unwrap_addmap, _T("addMap"), 0, TYPE_VOID, 0, 1,
	_T("map"), 0, TYPE_TEXMAP,


	unwrap_flattenmapbymatid, _T("flattenMapByMatID"), 0, TYPE_VOID, 0, 6,
	_T("angleThreshold"), 0, TYPE_FLOAT,
	_T("spacing"), 0, TYPE_FLOAT,
	_T("normalize"), 0, TYPE_BOOL,
	_T("layOutType"), 0, TYPE_INT,
	_T("rotateClusters"), 0, TYPE_BOOL,
	_T("fillHoles"), 0, TYPE_BOOL,


	unwrap_getarea, _T("getArea"), 0, TYPE_VOID, 0, 7,
	_T("faceSelection"), 0, TYPE_BITARRAY,
	_T("x"), 0, TYPE_FLOAT_BR, f_inOut, FPP_OUT_PARAM,
	_T("y"), 0, TYPE_FLOAT_BR, f_inOut, FPP_OUT_PARAM,
	_T("width"), 0, TYPE_FLOAT_BR, f_inOut, FPP_OUT_PARAM,
	_T("height"), 0, TYPE_FLOAT_BR, f_inOut, FPP_OUT_PARAM,
	_T("areaUVW"), 0, TYPE_FLOAT_BR, f_inOut, FPP_OUT_PARAM,
	_T("areaGeom"), 0, TYPE_FLOAT_BR, f_inOut, FPP_OUT_PARAM,


	unwrap_getrotationsrespectaspect, _T("getRotationsRespectAspect"),0,TYPE_BOOL,0,0,
	unwrap_setrotationsrespectaspect, _T("setRotationsRespectAspect"), 0, TYPE_VOID, 0, 1,
	_T("respect"), 0, TYPE_BOOL,
	unwrap_setmax5flatten, _T("setMax5Flatten"), 0, TYPE_VOID, 0, 1,
	_T("like5"), 0, TYPE_BOOL,



	

	end
	);


static FPInterfaceDesc unwrap_interface5(
	UNWRAP_INTERFACE5, _T("unwrap5"), 0, &unwrapDesc, FP_MIXIN,


	unwrap_pelt_getmapmode, _T("getPeltMapMode"),0,TYPE_BOOL,0,0,
	unwrap_pelt_setmapmode, _T("setPeltMapMode"), 0, TYPE_VOID, 0, 1,
	_T("mode"), 0, TYPE_BOOL,

	unwrap_pelt_geteditseamsmode, _T("getPeltEditSeamsMode"),0,TYPE_BOOL,0,0,
	unwrap_pelt_seteditseamsmode, _T("setPeltEditSeamsMode"), 0, TYPE_VOID, 0, 1,
	_T("mode"), 0, TYPE_BOOL,

	unwrap_pelt_getseamselection, _T("getPeltSelectedSeams"),0,TYPE_BITARRAY,0,0,
	unwrap_pelt_setseamselection, _T("setPeltSelectedSeams"),0,TYPE_VOID,0,1,
	_T("selection"),0,TYPE_BITARRAY,

	unwrap_pelt_getpointtopointseamsmode, _T("getPeltPointToPointSeamsMode"),0,TYPE_BOOL,0,0,
	unwrap_pelt_setpointtopointseamsmode, _T("setPeltPointToPointSeamsMode"), 0, TYPE_VOID, 0, 1,
	_T("mode"), 0, TYPE_BOOL,

	unwrap_pelt_expandtoseams, _T("peltExpandSelectionToSeams"),0,TYPE_VOID,0,0,
	unwrap_pelt_dialog, _T("peltDialog"),0,TYPE_VOID,0,0,

	unwrap_peltdialog_resetrig, _T("peltDialogResetStretcher"),0,TYPE_VOID,0,0,
	unwrap_peltdialog_selectrig, _T("peltDialogSelectStretcher"),0,TYPE_VOID,0,0,
	unwrap_peltdialog_selectpelt, _T("peltDialogSelectPelt"),0,TYPE_VOID,0,0,
	unwrap_peltdialog_snaprigtoedges, _T("peltDialogSnapToSeams"),0,TYPE_VOID,0,0,
	unwrap_peltdialog_mirrorrig, _T("peltDialogMirrorStretcher"),0,TYPE_VOID,0,0,


	unwrap_peltdialog_getstraightenmode, _T("getPeltDialogStraightenMode"),0,TYPE_BOOL,0,0,
	unwrap_peltdialog_setstraightenmode, _T("setPeltDialogStraightenMode"), 0, TYPE_VOID, 0, 1,
	_T("mode"), 0, TYPE_BOOL,

	unwrap_peltdialog_straighten, _T("peltDialogStraighten"), 0, TYPE_VOID, 0, 2,
	_T("vert1"), 0, TYPE_INDEX,
	_T("vert2"), 0, TYPE_INDEX,

	unwrap_peltdialog_run, _T("peltDialogRun"),0,TYPE_VOID,0,0,
	unwrap_peltdialog_relax1, _T("peltDialogRelaxLight"),0,TYPE_VOID,0,0,
	unwrap_peltdialog_relax2, _T("peltDialogRelaxHeavy"),0,TYPE_VOID,0,0,


	unwrap_peltdialog_getframes, _T("getPeltDialogFrames"),0,TYPE_INT,0,0,
	unwrap_peltdialog_setframes, _T("setPeltDialogFrames"), 0, TYPE_VOID, 0, 1,
	_T("frames"), 0, TYPE_INT,

	unwrap_peltdialog_getsamples, _T("getPeltDialogSamples"),0,TYPE_INT,0,0,
	unwrap_peltdialog_setsamples, _T("setPeltDialogSamples"), 0, TYPE_VOID, 0, 1,
	_T("samples"), 0, TYPE_INT,

	unwrap_peltdialog_getrigstrength, _T("getPeltDialogStretcherStrength"),0,TYPE_FLOAT,0,0,
	unwrap_peltdialog_setrigstrength, _T("setPeltDialogStretcherStrength"), 0, TYPE_VOID, 0, 1,
	_T("strength"), 0, TYPE_FLOAT,

	unwrap_peltdialog_getstiffness, _T("getPeltDialogStiffness"),0,TYPE_FLOAT,0,0,
	unwrap_peltdialog_setstiffness, _T("setPeltDialogStiffness"), 0, TYPE_VOID, 0, 1,
	_T("stiffness"), 0, TYPE_FLOAT,

	unwrap_peltdialog_getdampening, _T("getPeltDialogDampening"),0,TYPE_FLOAT,0,0,
	unwrap_peltdialog_setdampening, _T("setPeltDialogDampening"), 0, TYPE_VOID, 0, 1,
	_T("dampening"), 0, TYPE_FLOAT,

	unwrap_peltdialog_getdecay, _T("getPeltDialogDecay"),0,TYPE_FLOAT,0,0,
	unwrap_peltdialog_setdecay, _T("setPeltDialogDecay"), 0, TYPE_VOID, 0, 1,
	_T("decay"), 0, TYPE_FLOAT,

	unwrap_peltdialog_getmirroraxis, _T("getPeltDialogMirrorAxis"),0,TYPE_FLOAT,0,0,
	unwrap_peltdialog_setmirroraxis, _T("setPeltDialogMirrorAxis"), 0, TYPE_VOID, 0, 1,
	_T("axis"), 0, TYPE_FLOAT,

	unwrap_mapping_align, _T("mappingAlign"), 0, TYPE_VOID, 0, 1,
	_T("axis"), 0, TYPE_INT,

	unwrap_mappingmode, _T("mappingMode"), 0, TYPE_VOID, 0, 1,
	_T("mode"), 0, TYPE_INT,

	unwrap_setgizmotm, _T("setGizmoTM"), 0, TYPE_VOID, 0, 1,
	_T("tm"), 0, TYPE_MATRIX3,

	unwrap_getgizmotm, _T("getGizmoTM"), 0, TYPE_MATRIX3, 0, 0,

	unwrap_gizmofit, _T("mappingFit"), 0, TYPE_VOID, 0, 0,
	unwrap_gizmocenter, _T("mappingCenter"), 0, TYPE_VOID, 0, 0,
	unwrap_gizmoaligntoview, _T("mappingAlignToView"), 0, TYPE_VOID, 0, 0,

	unwrap_geomedgeselectionexpand, _T("expandGeomEdgeSelection"), 0, TYPE_VOID, 0, 0,
	unwrap_geomedgeselectioncontract, _T("contractGeomEdgeSelection"), 0, TYPE_VOID, 0, 0,

	unwrap_geomedgeloop, _T("geomEdgeLoopSelection"), 0, TYPE_VOID, 0, 0,
	unwrap_geomedgering, _T("geomEdgeRingSelection"), 0, TYPE_VOID, 0, 0,

	unwrap_geomvertexselectionexpand, _T("expandGeomVertexSelection"), 0, TYPE_VOID, 0, 0,
	unwrap_geomvertexselectioncontract, _T("contractGeomVertexSelection"), 0, TYPE_VOID, 0, 0,

	unwrap_getgeomvertexselection, _T("getSelectedGeomVerts"),0,TYPE_BITARRAY,0,0,
	unwrap_setgeomvertexselection, _T("setSelectedGeomVerts"),0,TYPE_VOID,0,1,
	_T("selection"),0,TYPE_BITARRAY,

	unwrap_getgeomedgeselection, _T("getSelectedGeomEdges"),0,TYPE_BITARRAY,0,0,
	unwrap_setgeomedgeselection, _T("setSelectedGeomEdges"),0,TYPE_VOID,0,1,
	_T("selection"),0,TYPE_BITARRAY,


	unwrap_pelt_getshowseam, _T("getPeltAlwaysShowSeams"),0,TYPE_BOOL,0,0,
	unwrap_pelt_setshowseam, _T("setPeltAlwaysShowSeams"), 0, TYPE_VOID, 0, 1,
	_T("show"), 0, TYPE_BOOL,

	unwrap_pelt_seamtosel, _T("peltSeamToEdgeSel"), 0, TYPE_VOID, 0, 1,
	_T("replace"), 0, TYPE_BOOL,

	unwrap_pelt_seltoseam, _T("peltEdgeSelToSeam"), 0, TYPE_VOID, 0, 1,
	_T("replace"), 0, TYPE_BOOL,

	unwrap_getnormalizemap, _T("getNormalizeMap"),0,TYPE_BOOL,0,0,
	unwrap_setnormalizemap, _T("setNormalizeMap"), 0, TYPE_VOID, 0, 1,
	_T("normalize"), 0, TYPE_BOOL,


	unwrap_getshowedgedistortion, _T("getShowEdgeDistortion"),0,TYPE_BOOL,0,0,
	unwrap_setshowedgedistortion, _T("setShowEdgeDistortion"), 0, TYPE_VOID, 0, 1,
	_T("show"), 0, TYPE_BOOL,


	unwrap_getlockedgesprings, _T("getPeltLockOpenEdges"),0,TYPE_BOOL,0,0,
	unwrap_setlockedgesprings, _T("setPeltLockOpenEdges"), 0, TYPE_VOID, 0, 1,
	_T("lock"), 0, TYPE_BOOL,

	unwrap_selectoverlap, _T("selectOverlappedFaces"), 0, TYPE_VOID, 0, 0,

	unwrap_gizmoreset, _T("mappingReset"), 0, TYPE_VOID, 0, 0,

	unwrap_getedgedistortionscale, _T("getEdgeDistortionScale"),0,TYPE_FLOAT,0,0,
	unwrap_setedgedistortionscale, _T("setEdgeDistortionScale"), 0, TYPE_VOID, 0, 1,
	_T("scale"), 0, TYPE_FLOAT,

	unwrap_relaxspring, _T("relaxBySpring"), 0, TYPE_VOID, 0, 3,
	_T("frames"), 0, TYPE_INT,
	_T("stretch"), 0, TYPE_FLOAT,
	_T("useOnlyVEdges"), 0, TYPE_BOOL,

	unwrap_relaxspringdialog, _T("relaxBySpringDialog"), 0, TYPE_VOID, 0, 0,

	unwrap_relaxspringdialog, _T("relaxBySpringDialog"), 0, TYPE_VOID, 0, 0,


	unwrap_getrelaxspringstretch, _T("getRelaxBySpringStretch"),0,TYPE_FLOAT,0,0,
	unwrap_setrelaxspringstretch, _T("setRelaxBySpringStretch"), 0, TYPE_VOID, 0, 1,
	_T("stretch"), 0, TYPE_FLOAT,

	unwrap_getrelaxspringiteration, _T("getRelaxBySpringIteration"),0,TYPE_INT,0,0,
	unwrap_setrelaxspringiteration, _T("setRelaxBySpringIteration"), 0, TYPE_VOID, 0, 1,
	_T("iteration"), 0, TYPE_INT,


	unwrap_getrelaxspringvedges, _T("getRelaxBySpringUseOnlyVEdges"),0,TYPE_BOOL,0,0,
	unwrap_setrelaxspringvedges, _T("setRelaxBySpringUseOnlyVEdges"), 0, TYPE_VOID, 0, 1,
	_T("useOnlyVEdges"), 0, TYPE_BOOL,

	unwrap_relaxbyfaceangle, _T("relaxByFaceAngle"), 0, TYPE_VOID, 0, 4,
	_T("iterations"), 0, TYPE_INT,
	_T("stretch"), 0, TYPE_FLOAT,
	_T("strength"), 0, TYPE_FLOAT,
	_T("lockBoundaries"), 0, TYPE_BOOL,

	unwrap_relaxbyedgeangle, _T("relaxByEdgeAngle"), 0, TYPE_VOID, 0, 4,
	_T("iterations"), 0, TYPE_INT,
	_T("stretch"), 0, TYPE_FLOAT,
	_T("strength"), 0, TYPE_FLOAT,
	_T("lockBoundaries"), 0, TYPE_BOOL,

	unwrap_setwindowxoffset, _T("setWindowXOffset"), 0, TYPE_VOID, 0, 1,
	_T("offset"), 0, TYPE_INT,
	unwrap_setwindowyoffset, _T("setWindowYOffset"), 0, TYPE_VOID, 0, 1,
	_T("offset"), 0, TYPE_INT,

	unwrap_frameselectedelement, _T("fitSelectedElement"), 0, TYPE_VOID, 0, 0,


	unwrap_getshowcounter, _T("getShowSubObjectCounter"),0,TYPE_BOOL,0,0,
	unwrap_setshowcounter, _T("setShowSubObjectCounter"), 0, TYPE_VOID, 0, 1,
	_T("show"), 0, TYPE_BOOL,

	unwrap_renderuv_dialog, _T("renderUVDialog"),0,TYPE_VOID,0,0,
	unwrap_renderuv, _T("renderUV"),0,TYPE_VOID,0,1,
	_T("fileName"), 0, TYPE_FILENAME,

	unwrap_ismesh, _T("isMesh"), 0, TYPE_BOOL, 0, 0,

	unwrap_qmap, _T("quickPlanarMap"),0,TYPE_VOID,0,0,

	unwrap_qmap_getpreview, _T("getQuickMapGizmoPreview"),0,TYPE_BOOL,0,0,
	unwrap_qmap_setpreview, _T("setQuickMapGizmoPreview"), 0, TYPE_VOID, 0, 1,
	_T("preview"), 0, TYPE_BOOL,

	unwrap_getshowmapseams, _T("getShowMapSeams"),0,TYPE_BOOL,0,0,
	unwrap_setshowmapseams, _T("setShowMapSeams"), 0, TYPE_VOID, 0, 1,
	_T("show"), 0, TYPE_BOOL,


	end
	);




//  Get Descriptor method for Mixin Interface
//  *****************************************


void *UnwrapClassDesc::Create(BOOL loading)
{

	AddInterface(&unwrap_interface);
	AddInterface(&unwrap_interface2);
	//5.1.05
	AddInterface(&unwrap_interface3);
	AddInterface(&unwrap_interface4);
	AddInterface(&unwrap_interface5);

	return new UnwrapMod;
}

FPInterfaceDesc* IUnwrapMod::GetDesc()
{
	return &unwrap_interface;
}

FPInterfaceDesc* IUnwrapMod2::GetDesc()
{
	return &unwrap_interface2;
}
//5.1.05
FPInterfaceDesc* IUnwrapMod3::GetDesc()
{
	return &unwrap_interface3;
}

FPInterfaceDesc* IUnwrapMod4::GetDesc()
{
	return &unwrap_interface4;
}


FPInterfaceDesc* IUnwrapMod5::GetDesc()
{
	return &unwrap_interface5;
}


FPInterfaceDesc* IUnwrapMod6::GetDesc()
{
	return &unwrap_interface6;
}

void UnwrapMod::ActivateActionTable()
{
	pCallback = new UnwrapActionCallback();
	pCallback->SetUnwrap(this);

	if  (GetCOREInterface()->GetActionManager()->ActivateActionTable(pCallback, kUnwrapActions) )
	{
		ActionTable *actionTable =  GetCOREInterface()->GetActionManager()->FindTable(kUnwrapActions);
		if (actionTable)
		{
			int actionCount = NumElements(spActions)/3;
			for (int i =0; i < actionCount; i++)
			{
				int id = spActions[i*3];
				UnwrapAction *action =  (UnwrapAction *) actionTable->GetAction(spActions[i*3]);
				if (action)
				{
					action->SetUnwrap(this);
				}

			}

		}
	}

}

//action Table methods
void UnwrapMod::DeActivateActionTable()
{
	ActionTable *actionTable =  GetCOREInterface()->GetActionManager()->FindTable(kUnwrapActions);
	if (actionTable)
	{
		int actionCount = NumElements(spActions)/3;
		for (int i =0; i < actionCount; i++)
		{
			UnwrapAction *action =  (UnwrapAction *) actionTable->GetAction(spActions[i*3]);
			if (action)
			{
				action->SetUnwrap(NULL);
			}
		}
	}

	if (pCallback) {
		GetCOREInterface()->GetActionManager()->DeactivateActionTable(pCallback, kUnwrapActions); 
		delete pCallback;
		pCallback = NULL;
	}
}




BOOL UnwrapMod::WtIsChecked(int id)
{
	BOOL iret = FALSE;
	switch (id)
	{
	case ID_SNAPGRID:
		{
			BOOL snap; 
			pblock->GetValue(unwrap_gridsnap,0,snap,FOREVER);
			iret = snap;
			break;
		}
	case ID_SNAPVERTEX:
		{
			BOOL snap; 
			pblock->GetValue(unwrap_vertexsnap,0,snap,FOREVER);
			iret = snap;
			break;
		}

	case ID_SNAPEDGE:
		{
			BOOL snap; 
			pblock->GetValue(unwrap_edgesnap,0,snap,FOREVER);
			iret = snap;
			break;
		}


	case ID_SHOWCOUNTER:
		iret = fnGetShowCounter();
		break;

	case ID_TWEAKUVW:
		iret = fnGetTweakMode();
		break;


	case ID_SHOWLOCALDISTORTION:
		{
		BOOL showLocalDistortion;
		TimeValue t = GetCOREInterface()->GetTime();
		pblock->GetValue(unwrap_localDistorion,t,showLocalDistortion,FOREVER);
		iret = showLocalDistortion;
		}
		break;

	case ID_PW_SHOWEDGEDISTORTION:
		iret = fnGetShowEdgeDistortion();
		break;

	case ID_PELT_ALWAYSSHOWSEAMS:
		iret = fnGetAlwayShowPeltSeams();
		break;

	case ID_PELT_MAP:
		if (fnGetMapMode()==PELTMAP)
			iret = TRUE;
		break;
	case ID_PLANAR_MAP:
		if (fnGetMapMode()==PLANARMAP)
			iret = TRUE;
		break;
	case ID_CYLINDRICAL_MAP:
		if (fnGetMapMode()==CYLINDRICALMAP)
			iret = TRUE;
		break;
	case ID_SPHERICAL_MAP:
		if (fnGetMapMode()==SPHERICALMAP)
			iret = TRUE;
		break;
	case ID_BOX_MAP:
		if (fnGetMapMode()==BOXMAP)
			iret = TRUE;
		break;
	case ID_SPLINE_MAP:
		if (fnGetMapMode()==SPLINEMAP)
			iret = TRUE;
		break;

	case ID_PELT_EDITSEAMS:
		if (fnGetPeltEditSeamsMode())
			iret = TRUE;
		break;
	case ID_PELT_POINTTOPOINTSEAMS:
		if (fnGetPeltPointToPointSeamsMode())
			iret = TRUE;
		break;
	case ID_PELTDIALOG_STRAIGHTENSEAMS:
		if (fnGetPeltDialogStraightenSeamsMode())
			iret = TRUE;
		break;


	case ID_SHOWOPENEDGESINVIEWPORT:
		if (fnGetViewportOpenEdges())
			iret = TRUE;
		break;

	case ID_FREEFORMMODE:
		if (mode ==ID_FREEFORMMODE)
		{
			iret = TRUE; 
		}
		break;
	case ID_MOVE:
		if ((mode ==ID_UNWRAP_MOVE) || (mode ==  ID_TOOL_MOVE))
		{
			iret = TRUE; 
		}
		break;
	case ID_ROTATE:
		if ((mode ==ID_UNWRAP_ROTATE) || (mode ==  ID_TOOL_ROTATE))
		{
			iret = TRUE; 
		}
		break;
	case ID_SCALE:
		if ((mode ==ID_UNWRAP_SCALE) || (mode ==  ID_TOOL_SCALE))
		{
			iret = TRUE; 
		}
		break;
	case ID_WELD:
		if ((mode ==ID_UNWRAP_WELD) || (mode ==  ID_TOOL_WELD))
		{
			iret = TRUE; 
		}
		break;
	case ID_PAN:
		if ((mode ==ID_UNWRAP_PAN) || (mode ==  ID_TOOL_PAN))
		{
			iret = TRUE; 
		}
		break;
	case ID_ZOOM:
		if ((mode ==ID_UNWRAP_ZOOM) || (mode ==  ID_TOOL_ZOOM))
		{
			iret = TRUE; 
		}
		break;
	case ID_ZOOMREGION:
		if ((mode ==ID_UNWRAP_ZOOMREGION) || (mode ==  ID_TOOL_ZOOMREG))
		{
			iret = TRUE; 
		}
		break;

	case ID_LOCK:
		iret = lockSelected;
		break;
	case ID_FILTERSELECTED:
		iret = filterSelectedFaces;
		break;

	case ID_SHOWMAP:
		iret = showMap;
		break;

	case ID_SNAP:
		iret = pixelSnap;
		break;
	case ID_LIMITSOFTSEL:
		iret = limitSoftSel;
		break;
	case ID_GEOMELEMMODE:
		iret = geomElemMode;
		break;
	case ID_PLANARMODE:
		{
			iret = fnGetGeomPlanarMode();
			break;
		}
	case ID_IGNOREBACKFACE:
		{
			iret = fnGetBackFaceCull();
			break;
		}
	case ID_ELEMENTMODE:
		{
			iret = fnGetTVElementMode();
			break;
		}
	case ID_SHOWVERTCONNECT:
		{
			iret = fnGetShowConnection();
			break;
		}
	case ID_TV_VERTMODE:
		{
			if (fnGetTVSubMode() == TVVERTMODE)
				iret = TRUE;
			else iret = FALSE;
			break;
		}
	case ID_TV_EDGEMODE:
		{
			if (fnGetTVSubMode() == TVEDGEMODE)
				iret = TRUE;
			else iret = FALSE;
			break;
		}
	case ID_TV_FACEMODE:
		{
			if (fnGetTVSubMode() == TVFACEMODE)
				iret = TRUE;
			else iret = FALSE;
			break;
		}
	case ID_UVEDGEMODE:
		{
			if (fnGetUVEdgeMode())
				iret = TRUE;
			else iret = FALSE;			
			break;
		}
	case ID_OPENEDGEMODE:
		{
			if (fnGetOpenEdgeMode())
				iret = TRUE;
			else iret = FALSE;			
			break;
		}
	case ID_DISPLAYHIDDENEDGES:
		{
			if (fnGetDisplayHiddenEdges())
				iret = TRUE;
			else iret = FALSE;			
			break;
		}
	case ID_RESETPIVOTONSEL:
		{
			if (fnGetResetPivotOnSel())
				iret = TRUE;
			else iret = FALSE;			
			break;
		}
	case ID_SKETCH:
		{
			if (mode == ID_SKETCHMODE)
				iret = TRUE;
			else iret = FALSE;			
			break;
		}
	case ID_SHOWHIDDENEDGES:
		{
			if (fnGetDisplayHiddenEdges())
				iret = TRUE;
			else iret = FALSE;			
			break;
		}
	case ID_POLYGONMODE:
		{
			if (fnGetPolyMode())
				iret = TRUE;
			else iret = FALSE;			
			break;
		}
	case ID_ALLOWSELECTIONINSIDEGIZMO:
		{
			if (fnGetAllowSelectionInsideGizmo())
				iret = TRUE;
			else iret = FALSE;			
			break;
		}
	case ID_SHOWSHARED:
		{
			if (fnGetShowShared())
				iret = TRUE;
			else iret = FALSE;			
			break;
		}
	case ID_ALWAYSEDIT:
		{
			if (fnGetAlwaysEdit())
				iret = TRUE;
			else iret = FALSE;			
			break;
		}
	case ID_SYNCSELMODE:
		{
			if (fnGetSyncSelectionMode())
				iret = TRUE;
			else iret = FALSE;			
			break;
		}
	case ID_SHOWOPENEDGES:
		{
			if (fnGetDisplayOpenEdges())
				iret = TRUE;
			else iret = FALSE;			
			break;
		}
	case ID_BRIGHTCENTERTILE:
		{
			if (fnGetBrightCenterTile())
				iret = TRUE;
			else iret = FALSE;			
			break;
		}
	case ID_BLENDTOBACK:
		{
			if (fnGetBlendToBack())
				iret = TRUE;
			else iret = FALSE;			
			break;
		}

	case ID_PAINTSELECTMODE:
		{
			if (fnGetPaintMode())
				iret = TRUE;
			else iret = FALSE;			
			break;
		}


	case ID_GRIDSNAP:
		{
			if (fnGetGridSnap())
				iret = TRUE;
			else iret = FALSE;			
			break;
		}

	case ID_GRIDVISIBLE:
		{
			if (fnGetGridVisible())
				iret = TRUE;
			else iret = FALSE;			
			break;
		}
	case ID_PREVENTREFLATTENING:
		{
			if (fnGetPreventFlattening())
				iret = TRUE;
			else iret = FALSE;			
			break;
		}

	}

	return iret;
}
BOOL UnwrapMod::WtIsEnabled(int id)
{
	BOOL iret = TRUE;

	switch (id)
	{
	case ID_ZOOMTOGIZMO:
		{
		if  (ip && (ip->GetSubObjectLevel() == 3) )
			iret = TRUE;
		else iret = FALSE;
		break;
		}

	case ID_QMAP:
		{
			if  (ip && (ip->GetSubObjectLevel() == 3) )
				iret = TRUE;
			else iret = FALSE;
			break;
		}
	case ID_DISPLAYHIDDENEDGES:
	case ID_SHOWHIDDENEDGES:
		{
			if (fnIsMesh())
				iret = TRUE;
			else iret = FALSE;
			break;
		}
	case ID_SAVE:
	case ID_LOAD:
	case ID_RESET:
		{
			if (fnGetMapMode() == PELTMAP)
				iret = FALSE;
			else iret = TRUE;
			break;
		}
		/*
		case ID_PELT_MAP:
		if (fnGetTVSubMode() != TVFACEMODE)
		iret = FALSE;
		break;
		*/
	case ID_EDGERINGSELECTION:
	case ID_EDGELOOPSELECTION:
		if  (ip && (ip->GetSubObjectLevel() == 2) )
			iret = TRUE;
		else iret = FALSE;

		break;

	case ID_MAPPING_ALIGNX:
	case ID_MAPPING_ALIGNY:
	case ID_MAPPING_ALIGNZ:
	case ID_MAPPING_NORMALALIGN:
	case ID_MAPPING_FIT:
	case ID_MAPPING_CENTER:
	case ID_MAPPING_ALIGNTOVIEW:
	case ID_MAPPING_RESET:
		if (fnGetMapMode() == NOMAP)
			iret = FALSE;
		break;
	case ID_PELTDIALOG:
	case ID_PELT_EDITSEAMS:
		if (peltData.GetPeltMapMode())
			iret = TRUE;
		else iret = FALSE;
		break;
		//		case ID_PELT_POINTTOPOINTSEAMS:	
	case ID_PW_SELTOSEAM:	
	case ID_PW_SELTOSEAM2:	
	case ID_PW_SEAMTOSEL:	
	case ID_PW_SEAMTOSEL2:			

		if (fnGetTVSubMode() == TVEDGEMODE)
			return TRUE;
		break;
	case ID_PELT_EXPANDSELTOSEAM:	
		if (fnGetTVSubMode() == TVFACEMODE)
			return TRUE;
		break;

	case ID_PELTDIALOG_RESETRIG:	
	case ID_PELTDIALOG_SELECTRIG:	
	case ID_PELTDIALOG_SELECTPELT:	
	case ID_PELTDIALOG_SNAPRIG:	
	case ID_PELTDIALOG_STRAIGHTENSEAMS:
	case ID_PELTDIALOG_MIRRORRIG:
	case ID_PELTDIALOG_RUN:
	case ID_PELTDIALOG_RELAX1:
	case ID_PELTDIALOG_RELAX2:
		if (peltData.GetPeltMapMode() && peltData.peltDialog.hWnd)
			iret = TRUE;
		else iret = FALSE;
		break;


	case ID_SELECT_OVERLAP:
		{
			if (fnGetTVSubMode() == TVFACEMODE)
				iret = TRUE;
			else iret = FALSE;			
			break;
		}
		break;

	case ID_PASTE:
		iret = copyPasteBuffer.CanPaste();
		break;
	case ID_PASTEINSTANCE:
		iret = copyPasteBuffer.CanPasteInstance(this);
		break;
	case ID_UVEDGEMODE:
		{
			if (fnGetTVSubMode() == TVEDGEMODE)
				iret = TRUE;
			else iret = FALSE;			
			break;
		}
	case ID_OPENEDGEMODE:
		{
			if (fnGetTVSubMode() == TVEDGEMODE)
				iret = TRUE;
			else iret = FALSE;			
			break;
		}
	case ID_UVEDGESELECT:
		{
			if (fnGetTVSubMode() == TVEDGEMODE)
				iret = TRUE;
			else iret = FALSE;			
			break;
		}
	case ID_OPENEDGESELECT:
		{
			if (fnGetTVSubMode() == TVEDGEMODE)
				iret = TRUE;
			else iret = FALSE;			
			break;
		}

	case ID_SNAPCENTER:
	case ID_SNAPLOWERLEFT:
	case ID_SNAPLOWERCENTER:
	case ID_SNAPLOWERRIGHT:
	case ID_SNAPUPPERLEFT:
	case ID_SNAPUPPERCENTER:
	case ID_SNAPUPPERRIGHT:
	case ID_SNAPRIGHTCENTER:
	case ID_SNAPLEFTCENTER:

		{
			if (mode == ID_FREEFORMMODE)
				iret = TRUE;
			else iret = FALSE;			
			break;
		}
	case ID_SKETCH:
	case ID_SKETCHDIALOG:
		{
			if (fnGetTVSubMode() == TVVERTMODE) 
				iret = TRUE;
			else iret = FALSE;
			break;
		}
	case ID_SKETCHREVERSE:
		{
			if ((sketchSelMode == SKETCH_SELCURRENT) && (mode == ID_SKETCHMODE))
				iret = TRUE;
			else iret = FALSE;
			break;
		}
	case ID_POLYGONMODE:
	case ID_POLYGONSELECT:

		{
			if (fnGetTVSubMode() == TVFACEMODE)
				iret = TRUE;
			else iret = FALSE;			
			break;
		}
	case ID_PAINTSELECTINC:
		{
			if (fnGetPaintMode() )
				iret = TRUE;
			else iret = FALSE;
			break;
		}
	case ID_PAINTSELECTDEC:
		{
			if (fnGetPaintMode() && (fnGetPaintSize() > 1))
				iret = TRUE;
			else iret = FALSE;
			break;
		}
	case ID_FLATTENMAP:
	case ID_FLATTENMAPDIALOG:
		{
			if (fnGetPreventFlattening())
				iret = FALSE;
			else
			{
				if (fnGetTVSubMode() == TVFACEMODE) 
					iret = TRUE; 
				else iret = FALSE;
			}
			break;
		}


	case ID_NORMALMAP:
	case ID_NORMALMAPDIALOG:
	case ID_UNFOLDMAP:
	case ID_UNFOLDMAPDIALOG:
		if (fnGetTVSubMode() == TVFACEMODE) 
			iret = TRUE; 
		else iret = FALSE;
		break;

	case ID_WELD:
		if (fnGetTVSubMode() == TVFACEMODE) 
			iret = FALSE; 
		else iret = TRUE; 
		break;
		/*
		case ID_RELAX:
		case ID_RELAXDIALOG:
		case ID_RELAXBYSPRING:
		case ID_RELAXBYSPRINGDIALOG:

		if (fnGetTVSubMode() == TVVERTMODE) 
		iret = TRUE; 
		else iret = FALSE; 
		break;
		*/
	case ID_SELECTINVERTEDFACES:
		{
			if (fnGetTVSubMode() == TVFACEMODE)
				iret = TRUE;
			else iret = FALSE;			
			break;
		}
	case ID_PACK:
		{
			if (fnGetTVSubMode() == TVFACEMODE)
				iret = TRUE;
			else iret = FALSE;			
			break;
		}



	}
	return iret;
}
BOOL UnwrapMod::WtExecute(int id, BOOL override)
{
	BOOL iret = FALSE;

	//	if ((!enableActionItems) && (!override))
	//		return iret;

	if (floaterWindowActive)
	{
		switch (id)
		{


		case ID_INCSEL:
			fnExpandSelection();
			iret = TRUE;
			break;
		case ID_DECSEL:
			fnContractSelection();
			iret = TRUE;
			break;


		case ID_LOCK:
			fnLock();
			iret = TRUE;
			break;			


		case ID_PAN:
			fnPan();
			iret = TRUE;
			break;			
		case ID_ZOOM:
			fnZoom();
			iret = TRUE;
			break;			
		case ID_ZOOMREGION:
			fnZoomRegion();
			iret = TRUE;
			break;			
		case ID_FIT:
			fnFit();
			iret = TRUE;
			break;			
		case ID_FITSELECTED:
			fnFitSelected();
			iret = TRUE;
			break;			
		case ID_FITSELECTEDELEMENT:
			fnFrameSelectedElement();
			iret = TRUE;
			break;

		case ID_MOVE:
			fnMove();
			iret = TRUE;
			break;
		case ID_ROTATE:
			fnRotate();
			iret = TRUE;
			break;
		case ID_SCALE:
			fnScale();
			iret = TRUE;
			break;

		}
	}
	switch (id)
	{
	case ID_QMAP:
		{
			fnQMap();
			iret = TRUE;

			break;
		}

	case ID_RENDERUV:
		{

			fnRenderUVDialog();
			iret = TRUE;
			break;
		}

	case ID_SNAPGRID:
		{
			BOOL snap; 
			pblock->GetValue(unwrap_gridsnap,0,snap,FOREVER);
			if (snap)
				snap = FALSE;
			else snap = TRUE;
			pblock->SetValue(unwrap_gridsnap,0,snap);
			iret = TRUE;
			break;
		}
	case ID_SNAPVERTEX:
		{
			BOOL snap; 
			pblock->GetValue(unwrap_vertexsnap,0,snap,FOREVER);
			if (snap)
				snap = FALSE;
			else snap = TRUE;
			pblock->SetValue(unwrap_vertexsnap,0,snap);
			iret = TRUE;
			break;
		}

	case ID_SNAPEDGE:
		{
			BOOL snap; 
			pblock->GetValue(unwrap_edgesnap,0,snap,FOREVER);
			if (snap)
				snap = FALSE;
			else snap = TRUE;
			pblock->SetValue(unwrap_edgesnap,0,snap);
			iret = TRUE;
			break;
		}
	case ID_SHOWCOUNTER:
		{
			if (fnGetShowCounter())
				fnSetShowCounter(FALSE);
			else fnSetShowCounter(TRUE);
			iret = TRUE;
			break;
		}

	case ID_SELECT_OVERLAP:
		{
			fnSelectOverlap();

			break;
		}
	case ID_PW_SHOWEDGEDISTORTION:
		if (fnGetShowEdgeDistortion())
			fnSetShowEdgeDistortion(FALSE);
		else fnSetShowEdgeDistortion(TRUE);
		iret = TRUE;
		break;

	case ID_SHOWLOCALDISTORTION:
		{
		BOOL showLocalDistortion;
		TimeValue t = GetCOREInterface()->GetTime();
		pblock->GetValue(unwrap_localDistorion,t,showLocalDistortion,FOREVER);

		if (showLocalDistortion)
			pblock->SetValue(unwrap_localDistorion,t,FALSE);
		else pblock->SetValue(unwrap_localDistorion,t,TRUE);
		
		iret = TRUE;
		}
		break;


	case ID_PELT_ALWAYSSHOWSEAMS:
		if  (fnGetAlwayShowPeltSeams())
			fnSetAlwayShowPeltSeams(FALSE);
		else fnSetAlwayShowPeltSeams(TRUE);
		iret = TRUE;

		break;
	case ID_EDGERINGSELECTION:
		fnGeomRingSelect();
		break;
	case ID_EDGELOOPSELECTION:
		fnGeomLoopSelect();
		break;
	case ID_PELT_MAP:
		fnSetMapMode(PELTMAP);
		break;
	case ID_PLANAR_MAP:
		fnSetMapMode(PLANARMAP);
		break;
	case ID_CYLINDRICAL_MAP:
		fnSetMapMode(CYLINDRICALMAP);
		break;
	case ID_SPHERICAL_MAP:
		fnSetMapMode(SPHERICALMAP);

		break;

	case ID_BOX_MAP:
		fnSetMapMode(BOXMAP);
		break;
	case ID_SPLINE_MAP:
		fnSetMapMode(SPLINEMAP);
		break;



	case ID_MAPPING_ALIGNX:
		fnAlignAndFit(0);
		break;				
	case ID_MAPPING_ALIGNY:
		fnAlignAndFit(1);
		break;				
	case ID_MAPPING_ALIGNZ:
		fnAlignAndFit(2);
		break;				
	case ID_MAPPING_NORMALALIGN:
		fnAlignAndFit(3);
		break;
	case ID_MAPPING_FIT:
		fnGizmoFit();
		break;
	case ID_MAPPING_CENTER:
		fnGizmoCenter();
		break;
	case ID_MAPPING_ALIGNTOVIEW:
		fnGizmoAlignToView();
		break;

	case ID_MAPPING_RESET:
		fnGizmoReset();
		break;

	case ID_TWEAKUVW:
		if (fnGetTweakMode())
			fnSetTweakMode(FALSE);
		else fnSetTweakMode(TRUE);
		iret = TRUE;

		break;

	case ID_PELT_EDITSEAMS:
		if (fnGetPeltEditSeamsMode())
			fnSetPeltEditSeamsMode(FALSE);
		else fnSetPeltEditSeamsMode(TRUE);
		iret = TRUE;

		break;
	case ID_PELT_POINTTOPOINTSEAMS:	
		if (GetCOREInterface()->GetCommandMode() == peltPointToPointMode)
			fnSetPeltPointToPointSeamsMode(FALSE);
		else fnSetPeltPointToPointSeamsMode(TRUE);
		iret = TRUE;

		break;
	case ID_PELT_EXPANDSELTOSEAM:	
		fnPeltExpandSelectionToSeams();
		break;

	case ID_PW_SELTOSEAM:	
		fnPeltSelToSeams(TRUE);
		break;
	case ID_PW_SELTOSEAM2:	
		fnPeltSelToSeams(FALSE);
		break;

	case ID_PW_SEAMTOSEL:	
		fnPeltSeamsToSel(TRUE);
		break;

	case ID_PW_SEAMTOSEL2:			
		fnPeltSeamsToSel(FALSE);
		break;

	case ID_PELTDIALOG:
		fnPeltDialog();
		break;
	case ID_PELTDIALOG_RESETRIG:
		fnPeltDialogResetRig();
		break;
	case ID_PELTDIALOG_SELECTRIG:
		fnPeltDialogSelectRig();
		break;
	case ID_PELTDIALOG_SELECTPELT:
		fnPeltDialogSelectPelt();
		break;

	case ID_PELTDIALOG_SNAPRIG:
		fnPeltDialogSnapRig();
		break;

	case ID_PELTDIALOG_STRAIGHTENSEAMS:
		if (fnGetPeltDialogStraightenSeamsMode())
			fnSetPeltDialogStraightenSeamsMode(FALSE);
		else fnSetPeltDialogStraightenSeamsMode(TRUE);
		iret = TRUE;

		break;
	case ID_PELTDIALOG_MIRRORRIG:
		fnPeltDialogMirrorRig();
		break;
	case ID_PELTDIALOG_RUN:
		fnPeltDialogRun();
		break;
	case ID_PELTDIALOG_RELAX1:
		fnPeltDialogRelax1();
		break;
	case ID_PELTDIALOG_RELAX2:
		fnPeltDialogRelax2();
		break;
	case ID_SHOWOPENEDGESINVIEWPORT:
		if (fnGetViewportOpenEdges())
			fnSetViewportOpenEdges(FALSE);
		else fnSetViewportOpenEdges(TRUE);
		InvalidateView();
		NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		iret = TRUE;
		break;
		/*
		case ID_PLANAR_MAP:
		fnPlanarMap();
		iret = TRUE;
		break;
		*/

	case ID_SAVE:
		fnSave();
		iret = TRUE;
		break;
	case ID_LOAD:
		fnLoad();
		iret = TRUE;
		break;
	case ID_RESET:
		fnReset();
		iret = TRUE;
		break;
	case ID_EDIT:
		fnEdit();
		iret = TRUE;
		break;
	case ID_PREVENTREFLATTENING:
		{
			if (fnGetPreventFlattening())
				fnSetPreventFlattening(FALSE);
			else fnSetPreventFlattening(TRUE);	
			iret = TRUE;
			break;
		}
	case ID_ZOOMTOGIZMO:
		fnZoomToGizmo(FALSE);
		iret = TRUE;
		break;
	case ID_FREEFORMMODE:
		fnSetFreeFormMode(TRUE);
		iret = TRUE;
		break;

	case ID_MOVEH:
		fnMoveH();
		iret = TRUE;
		break;
	case ID_MOVEV:
		fnMoveV();
		iret = TRUE;
		break;

	case ID_SCALEH:
		fnScaleH();
		iret = TRUE;
		break;
	case ID_SCALEV:
		fnScaleV();
		iret = TRUE;
		break;
	case ID_MIRRORH:
		fnMirrorH();
		iret = TRUE;
		break;
	case ID_MIRRORV:
		fnMirrorV();
		iret = TRUE;
		break;

	case ID_FLIPH:
		fnFlipH();
		iret = TRUE;
		break;
	case ID_FLIPV:
		fnFlipV();
		iret = TRUE;
		break;
	case ID_BREAK:
		fnBreakSelected();
		iret = TRUE;
		break;
	case ID_WELD:
		fnWeld();
		iret = TRUE;
		break;
	case ID_WELD_SELECTED:
		fnWeldSelected();
		iret = TRUE;
		break;
	case ID_UPDATEMAP:
		fnUpdatemap();
		iret = TRUE;
		break;			
	case ID_OPTIONS:
		fnOptions();
		iret = TRUE;
		break;	
	case ID_HIDE:
		fnHide();
		iret = TRUE;
		break;			
	case ID_UNHIDE:
		fnUnhide();
		iret = TRUE;
		break;			
	case ID_FREEZE:
		fnFreeze();
		iret = TRUE;
		break;			
	case ID_UNFREEZE:
		fnThaw();
		iret = TRUE;
		break;			
	case ID_FILTERSELECTED:
		fnFilterSelected();
		iret = TRUE;
		break;			

	case ID_SNAP:
		fnSnap();
		iret = TRUE;
		break;			
	case ID_FACETOVERTEX:
		iret = TRUE;
		fnGetSelectionFromFace();
		break;			

	case ID_DETACH:
		fnDetachEdgeVerts();
		iret = TRUE;
		break;


	case ID_GETFACESELFROMSTACK:
		fnGetFaceSelFromStack();
		iret = TRUE;
		break;
	case ID_STITCH:
		fnStitchVertsNoParams();
		iret = TRUE;
		break;
	case ID_STITCHDIALOG:
		fnStitchVertsDialog();
		iret = TRUE;
		break;
	case ID_SHOWMAP:
		fnShowMap();
		iret = TRUE;
		break;

	case ID_NORMALMAP:
		fnNormalMapNoParams();
		iret = TRUE;
		break;
	case ID_NORMALMAPDIALOG:
		fnNormalMapDialog();
		iret = TRUE;
		break;

	case ID_FLATTENMAP:
		fnFlattenMapNoParams();
		iret = TRUE;
		break;
	case ID_FLATTENMAPDIALOG:
		fnFlattenMapDialog();
		iret = TRUE;
		break;

	case ID_UNFOLDMAP:
		fnUnfoldSelectedPolygonsNoParams();
		iret = TRUE;
		break;
	case ID_UNFOLDMAPDIALOG:
		fnUnfoldSelectedPolygonsDialog();
		iret = TRUE;
		break;

	case ID_COPY:
		fnCopy();
		iret = TRUE;
		break;

	case ID_PASTE:
		fnPaste(TRUE);
		iret = TRUE;
		break;
	case ID_PASTEINSTANCE:
		fnPasteInstance();
		iret = TRUE;
		break;

	case ID_LIMITSOFTSEL:
		{
			BOOL limit = fnGetLimitSoftSel();

			if (limit)
				fnSetLimitSoftSel(FALSE);
			else fnSetLimitSoftSel(TRUE);
			iret = TRUE;
			break;
		}

	case ID_LIMITSOFTSEL1:
		fnSetLimitSoftSelRange(1);
		iret = TRUE;
		break;
	case ID_LIMITSOFTSEL2:
		fnSetLimitSoftSelRange(2);
		iret = TRUE;
		break;
	case ID_LIMITSOFTSEL3:
		fnSetLimitSoftSelRange(3);
		iret = TRUE;
		break;
	case ID_LIMITSOFTSEL4:
		fnSetLimitSoftSelRange(4);
		iret = TRUE;
		break;
	case ID_LIMITSOFTSEL5:
		fnSetLimitSoftSelRange(5);
		iret = TRUE;
		break;
	case ID_LIMITSOFTSEL6:
		fnSetLimitSoftSelRange(6);
		iret = TRUE;
		break;
	case ID_LIMITSOFTSEL7:
		fnSetLimitSoftSelRange(7);
		iret = TRUE;
		break;
	case ID_LIMITSOFTSEL8:
		fnSetLimitSoftSelRange(8);
		iret = TRUE;
		break;
	case ID_GEOMELEMMODE:
		{
			BOOL mode = fnGetGeomElemMode();
			if (mode)
				fnSetGeomElemMode(FALSE);
			else fnSetGeomElemMode(TRUE);
			iret = TRUE;
			break;
		}
	case ID_PLANARMODE:
		{
			BOOL pmode = fnGetGeomPlanarMode();

			if (pmode)
				fnSetGeomPlanarMode(FALSE);
			else fnSetGeomPlanarMode(TRUE);
			iret = TRUE;
			break;
		}
	case ID_IGNOREBACKFACE:
		{
			BOOL backface = fnGetBackFaceCull();

			if (backface)
				fnSetBackFaceCull(FALSE);
			else fnSetBackFaceCull(TRUE);
			iret = TRUE;
			break;
		}
	case ID_ELEMENTMODE:
		{
			BOOL mode = fnGetTVElementMode();

			if (mode)
				fnSetTVElementMode(FALSE);
			else fnSetTVElementMode(TRUE);
			iret = TRUE;
			break;
		}
	case ID_GEOMEXPANDFACESEL:
		if (ip)
		{
			if  (ip->GetSubObjectLevel() == 2)
				fnGeomExpandEdgeSel();
			else if  (ip->GetSubObjectLevel() == 3)
				fnGeomExpandFaceSel();

			iret = TRUE;
		}
		break;
	case ID_GEOMCONTRACTFACESEL:
		if (ip)
		{
			if  (ip->GetSubObjectLevel() == 2)
				fnGeomContractEdgeSel();
			else if  (ip->GetSubObjectLevel() == 3)
				fnGeomContractFaceSel();

			iret = TRUE;
		}
		break;
		//	 		fnGeomContractFaceSel();
		//			iret = TRUE;
		break;

	case ID_SHOWVERTCONNECT:
		{
			BOOL show = fnGetShowConnection();
			if (show)
				fnSetShowConnection(FALSE);
			else fnSetShowConnection(TRUE);
			iret = TRUE;
			break;
		}
	case ID_TV_VERTMODE:
		{
			fnSetTVSubMode(1);
			iret = TRUE;
			break;
		}
	case ID_TV_EDGEMODE:
		{
			fnSetTVSubMode(2);
			iret = TRUE;
			break;
		}
	case ID_TV_FACEMODE:
		{
			fnSetTVSubMode(3);
			iret = TRUE;
			break;
		}
	case ID_PACK:
		fnPackNoParams();
		iret = TRUE;
		break;
	case ID_PACKDIALOG:
		fnPackDialog();
		iret = TRUE;
		break;
	case ID_UVEDGEMODE:
		{
			if (fnGetUVEdgeMode())
				fnSetUVEdgeMode(FALSE);
			else fnSetUVEdgeMode(TRUE);
			iret = TRUE;
			break;
		}
	case ID_OPENEDGEMODE:
		{
			if (fnGetOpenEdgeMode())
				fnSetOpenEdgeMode(FALSE);
			else fnSetOpenEdgeMode(TRUE);
			iret = TRUE;
			break;
		}
	case ID_UVEDGESELECT:
		{
			fnUVEdgeSelect();
			iret = TRUE;
			break;
		}
	case ID_OPENEDGESELECT:
		{
			fnOpenEdgeSelect();
			iret = TRUE;
			break;
		}

	case ID_VERTTOEDGESELECT:
		{
			fnVertToEdgeSelect();
			fnSetTVSubMode(2);
			MoveScriptUI();
			iret = TRUE;
			break;
		}
	case ID_VERTTOFACESELECT:
		{
			fnVertToFaceSelect();
			fnSetTVSubMode(3);
			MoveScriptUI();
			iret = TRUE;
			break;
		}

	case ID_EDGETOVERTSELECT:
		{
			fnEdgeToVertSelect();
			fnSetTVSubMode(1);
			RebuildDistCache();
			MoveScriptUI();
			iret = TRUE;
			break;
		}

	case ID_EDGETOFACESELECT:
		{
			fnEdgeToFaceSelect();
			fnSetTVSubMode(3);
			MoveScriptUI();
			iret = TRUE;
			break;
		}

	case ID_FACETOVERTSELECT:
		{
			fnFaceToVertSelect();
			fnSetTVSubMode(1);
			RebuildDistCache();
			MoveScriptUI();
			iret = TRUE;
			break;
		}
	case ID_FACETOEDGESELECT:
		{
			fnFaceToEdgeSelect();
			fnSetTVSubMode(2);
			MoveScriptUI();
			iret = TRUE;
			break;
		}
	case ID_DISPLAYHIDDENEDGES:
		{
			if (fnGetDisplayHiddenEdges())
				fnSetDisplayHiddenEdges(FALSE);
			else fnSetDisplayHiddenEdges(TRUE);
			iret = TRUE;
			break;
		}
	case ID_SNAPCENTER:
		{
			fnSnapPivot(1);
			break;
		}
	case ID_SNAPLOWERLEFT:
		{
			fnSnapPivot(2);
			break;
		}
	case ID_SNAPLOWERCENTER:
		{
			fnSnapPivot(3);
			break;
		}
	case ID_SNAPLOWERRIGHT:
		{
			fnSnapPivot(4);
			break;
		}
	case ID_SNAPRIGHTCENTER:
		{
			fnSnapPivot(5);
			break;
		}

	case ID_SNAPUPPERLEFT:
		{
			fnSnapPivot(8);
			break;
		}
	case ID_SNAPUPPERCENTER:
		{
			fnSnapPivot(7);
			break;
		}
	case ID_SNAPUPPERRIGHT:
		{
			fnSnapPivot(6);
			break;
		}
	case ID_SNAPLEFTCENTER:
		{
			fnSnapPivot(9);
			break;
		}
	case ID_SKETCHREVERSE:
		{
			fnSketchReverse();
			break;
		}
	case ID_SKETCHDIALOG:
		{
			fnSketchDialog();
			iret = TRUE;
			break;
		}
	case ID_SKETCH:
		{
			fnSketchNoParams();
			iret = TRUE;
			break;
		}
	case ID_RESETPIVOTONSEL:
		{
			if (fnGetResetPivotOnSel())
				fnSetResetPivotOnSel(FALSE);
			else fnSetResetPivotOnSel(TRUE);
			iret = TRUE;
			break;
		}

	case ID_SHOWHIDDENEDGES:
		{
			if (fnGetDisplayHiddenEdges())
				fnSetDisplayHiddenEdges(FALSE);
			else fnSetDisplayHiddenEdges(TRUE);		
			iret = TRUE;
			break;
		}
	case ID_POLYGONMODE:
		{
			if (fnGetPolyMode())
				fnSetPolyMode(FALSE);
			else fnSetPolyMode(TRUE);		
			iret = TRUE;
			break;
		}
	case ID_POLYGONSELECT:
		{
			HoldSelection();
			fnPolySelect();
			break;
		}
	case ID_ALLOWSELECTIONINSIDEGIZMO:
		{
			if (fnGetAllowSelectionInsideGizmo())
				fnSetAllowSelectionInsideGizmo(FALSE);
			else fnSetAllowSelectionInsideGizmo(TRUE);		
			iret = TRUE;
			break;
		}
	case ID_SAVEASDEFAULT:
		{
			fnSetAsDefaults();
			iret = TRUE;
			break;
		}
	case ID_LOADDEFAULT:
		{
			fnLoadDefaults();
			iret = TRUE;
			break;
		}
	case ID_SHOWSHARED:
		{
			if (fnGetShowShared())
				fnSetShowShared(FALSE);
			else fnSetShowShared(TRUE);		
			iret = TRUE;
			break;
		}
	case ID_ALWAYSEDIT:
		{
			if (fnGetAlwaysEdit())
				fnSetAlwaysEdit(FALSE);
			else fnSetAlwaysEdit(TRUE);	
			iret = TRUE;
			break;
		}
	case ID_SYNCSELMODE:
		{
			if (fnGetSyncSelectionMode())
				fnSetSyncSelectionMode(FALSE);
			else fnSetSyncSelectionMode(TRUE);			
			iret = TRUE;
			break;
		}
	case ID_SYNCTVSELECTION:
		{
			fnSyncTVSelection();
			iret = TRUE;
			break;
		}
	case ID_SYNCGEOMSELECTION:
		{
			fnSyncGeomSelection();
			iret = TRUE;
			break;
		}
	case ID_SHOWOPENEDGES:
		{
			if (fnGetDisplayOpenEdges())
				fnSetDisplayOpenEdges(FALSE);
			else fnSetDisplayOpenEdges(TRUE);			
			iret = TRUE;
			break;
		}

	case ID_BRIGHTCENTERTILE:
		{
			if (fnGetBrightCenterTile())
				fnSetBrightCenterTile(FALSE);
			else fnSetBrightCenterTile(TRUE);			
			iret = TRUE;
			break;
		}
	case ID_BLENDTOBACK:
		{
			if (fnGetBlendToBack())
				fnSetBlendToBack(FALSE);
			else fnSetBlendToBack(TRUE);
			iret = TRUE;
			break;
		}

	case ID_PAINTSELECTMODE:
		{
			fnSetPaintMode(TRUE);
			iret = TRUE;
			break;
		}
	case ID_PAINTSELECTINC:
		{
			fnIncPaintSize();
			iret = TRUE;
			break;
		}
	case ID_PAINTSELECTDEC:
		{
			fnDecPaintSize();			
			iret = TRUE;
			break;
		}
	case ID_GRIDSNAP:
		{
			if (fnGetGridSnap())
				fnSetGridSnap(FALSE);
			else fnSetGridSnap(TRUE);			
			iret = TRUE;
			break;
		}

	case ID_GRIDVISIBLE:
		{
			if (fnGetGridVisible())
				fnSetGridVisible(FALSE);
			else fnSetGridVisible(TRUE);			
			iret = TRUE;
			break;
		}

	case ID_RELAX:
		{
			this->fnRelax2();
			break;
		}

	case ID_RELAXDIALOG:
		{
			this->fnRelax2Dialog();
			break;
		}

	case ID_RELAXBYSPRING:
		{
			float stretch = relaxBySpringStretch;
			stretch= 1.0f-stretch*0.01f;
			this->fnRelaxBySprings(relaxBySpringIteration,relaxBySpringStretch,relaxBySpringUseOnlyVEdges);
			break;
		}
	case ID_RELAXBYSPRINGDIALOG:
		{
			this->fnRelaxBySpringsDialog();
			break;
		}

	case ID_SELECTINVERTEDFACES:
		{
			this->fnSelectInvertedFaces();
			break;
		}
	case ID_TVRING:
		{
			fnUVRing(0);
			break;
		}
	case ID_TVRINGGROW:
		{
			fnUVRing(1);
			break;
		}
	case ID_TVRINGSHRINK:
		{
			fnUVRing(-1);
			break;
		}
	case ID_TVLOOP:
		{
			fnUVLoop(0);
			break;
		}
	case ID_TVLOOPGROW:
		{
			fnUVLoop(1);
			break;
		}
	case ID_TVLOOPSHRINK:
		{
			fnUVLoop(-1);
			break;
		}

	}


	if ( (ip) &&(hWnd) && iret)
	{	
		IMenuBarContext* pContext = (IMenuBarContext*) GetCOREInterface()->GetMenuManager()->GetContext(kUnwrapMenuBar);
		if (pContext)
			pContext->UpdateWindowsMenu();
	}

	return iret;
}






void UnwrapMod::fnPlanarMap()
{
	//align to normals
	//call fit
	fnAlignAndFit(3);
	

	flags |= CONTROL_FIT|CONTROL_CENTER|CONTROL_HOLD;
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
	ApplyGizmo();
	CleanUpDeadVertices();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
}

void UnwrapMod::fnSave()
{
	DragAcceptFiles(ip->GetMAXHWnd(), FALSE);
	SaveUVW(hWnd);
	DragAcceptFiles(ip->GetMAXHWnd(), TRUE);
}
void UnwrapMod::fnLoad()
{
	DragAcceptFiles(ip->GetMAXHWnd(), FALSE);
	LoadUVW(hWnd);
	DragAcceptFiles(ip->GetMAXHWnd(), TRUE);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
	InvalidateView();
}

void UnwrapMod::fnEdit()
{
	HWND hWnd = hParams;
	RegisterClasses();
	if (ip)
	{
		RebuildEdges();
		if (!this->hWnd) {
			HWND floaterHwnd = CreateDialogParam(
				hInstance,
				MAKEINTRESOURCE(IDD_UNWRAP_FLOATER),
				//			hWnd,
				ip->GetMAXHWnd (),
				UnwrapFloaterDlgProc,
				(LPARAM)this);
//			LoadMaterials();

			IMenuBarContext* pContext = (IMenuBarContext*) GetCOREInterface()->GetMenuManager()->GetContext(kUnwrapMenuBar);
			assert(pContext);
			pContext->CreateWindowsMenu();
			SetMenu(floaterHwnd, pContext->GetCurWindowsMenu());
			DrawMenuBar(floaterHwnd);
			pContext->UpdateWindowsMenu();
			LaunchScriptUI();
			SetFocus(floaterHwnd);
			if (ip->GetSubObjectLevel()== 0)
				fnSetTVSubMode(TVVERTMODE);
		} 
		else 
		{
			SetActiveWindow(this->hWnd);
			ShowWindow(this->hWnd,SW_RESTORE);
		}
	}
}

void UnwrapMod::fnReset()
{
	TSTR buf1 = GetString(IDS_RB_RESETUNWRAPUVWS);
	TSTR buf2 = GetString(IDS_RB_UNWRAPMOD);
	if (IDYES==MessageBox(ip->GetMAXHWnd(),buf1,buf2,MB_YESNO|MB_ICONQUESTION|MB_TASKMODAL)) 
	{
		theHold.Begin();
		Reset();
		theHold.Accept(GetString(IDS_RB_RESETUVWS));
		ip->RedrawViews(ip->GetTime());
		InvalidateView();

	}
}


void UnwrapMod::fnSetMapChannel(int incChannel)
{
	int tempChannel = incChannel;
	if (tempChannel == 1) tempChannel = 0;

	if (tempChannel != channel)
	{
		if (iMapID) iMapID->SetValue(incChannel,TRUE);

		if (loadDefaults)
		{
			fnLoadDefaults();
			loadDefaults = FALSE;
		}

		theHold.Begin();		
		channel = incChannel;
		if (channel == 1) channel = 0;
		theHold.Accept(GetString(IDS_RB_SETCHANNEL));					

		SetCheckerMapChannel();
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

		if (ip)
		{
			ip->RedrawViews(ip->GetTime());
			InvalidateView();
		}
	}

}
int	UnwrapMod::fnGetMapChannel()
{
	return channel;
}

void UnwrapMod::fnSetProjectionType(int proj)
{
	SetQMapAlign(proj-1);

	if (proj == 1)
	{
		CheckRadioButton(  hMapParams, IDC_RADIO1, IDC_RADIO4,IDC_RADIO1);
	}
	else if (proj == 2)
	{
		CheckRadioButton(  hMapParams, IDC_RADIO1, IDC_RADIO4,IDC_RADIO2);
	}	
	else if (proj == 3)
	{
		CheckRadioButton(  hMapParams, IDC_RADIO1, IDC_RADIO4,IDC_RADIO3);
	}
	else if (proj == 4)
	{
		CheckRadioButton(  hMapParams, IDC_RADIO1, IDC_RADIO4,IDC_RADIO4);
	}

}
int	UnwrapMod::fnGetProjectionType()
{
	return GetQMapAlign()+1;
}

void UnwrapMod::fnSetQMapPreview(BOOL preview)
{
	SetQMapPreview(preview);
	CheckDlgButton(hMapParams,IDC_PREVIEW_CHECK,GetQMapPreview());
}
BOOL UnwrapMod::fnGetQMapPreview()
{
	return GetQMapPreview();
}

void UnwrapMod::fnSetVC(BOOL vc)
{
	suppressWarning = TRUE;
	if (vc)
	{
		CheckRadioButton(  hParams, IDC_MAP_CHAN1, IDC_MAP_CHAN2,IDC_MAP_CHAN2);
		SendMessage(hParams,WM_COMMAND,IDC_MAP_CHAN2,0);
	}
	else 
	{
		CheckRadioButton(  hParams, IDC_MAP_CHAN1, IDC_MAP_CHAN2,IDC_MAP_CHAN1);
		SendMessage(hParams,WM_COMMAND,IDC_MAP_CHAN1,0);
	}	
	suppressWarning = FALSE;

}
BOOL UnwrapMod::fnGetVC()
{
	if (channel == 1)
		return TRUE;
	else return FALSE;
}


void UnwrapMod::fnMove()
{
	if (iMove) 
	{
		iMove->SetCurFlyOff(0,TRUE);
	}
	else
	{
		move = 0;
		SetMode(ID_TOOL_MOVE);
	}
}
void UnwrapMod::fnMoveH()
{
	if (iMove) 
	{
		iMove->SetCurFlyOff(1,TRUE);
	}
	else
	{
		move = 1;
		SetMode(ID_TOOL_MOVE);
	}

}
void UnwrapMod::fnMoveV()
{
	if (iMove) 
	{
		iMove->SetCurFlyOff(2,TRUE);
	}
	else
	{
		move = 2;
		SetMode(ID_TOOL_MOVE);
	}

}


void UnwrapMod::fnRotate()
{
	if (iRot) SetMode(ID_TOOL_ROTATE);
	else
	{
		SetMode(ID_TOOL_ROTATE);
	}
}	

void UnwrapMod::fnScale()
{
	if (iScale) 
		iScale->SetCurFlyOff(0,TRUE);//SetMode(ID_TOOL_SCALE);
	else
	{
		scale = 0;
		SetMode(ID_TOOL_SCALE);
	}
}
void UnwrapMod::fnScaleH()
{
	if (iScale) 
		iScale->SetCurFlyOff(1,TRUE);//SetMode(ID_TOOL_SCALE);
	else
	{
		scale = 1;
		SetMode(ID_TOOL_SCALE);
	}
}
void UnwrapMod::fnScaleV()
{
	if (iScale) 
		iScale->SetCurFlyOff(2,TRUE);//SetMode(ID_TOOL_SCALE);
	else
	{
		scale = 2;
		SetMode(ID_TOOL_SCALE);
	}

}



void UnwrapMod::fnMirrorH()
{
	if (iMirror) 
	{
		iMirror->SetCurFlyOff(0,TRUE);
		//		SendMessage(hWnd,WM_COMMAND,ID_TOOL_MIRROR,0);
	}
	else
	{
		mirror = 0;
		MirrorPoints( mirror);
	}
}

void UnwrapMod::fnMirrorV()
{
	if (iMirror) 
	{
		iMirror->SetCurFlyOff(1,TRUE);

		//		SendMessage(hWnd,WM_COMMAND,ID_TOOL_MIRROR,0);
	}
	else
	{
		mirror = 1;
		MirrorPoints( mirror);

	}

}


void UnwrapMod::fnExpandSelection()
{

	theHold.Begin();
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		theHold.Put(new TSelRestore(this,ld));
	}
	theHold.Accept(_T(GetString(IDS_PW_SELECT_UVW)));

	theHold.Suspend();
	if ((fnGetTVSubMode() == TVEDGEMODE)  )
	{
		BOOL openEdgeSel = FALSE;
		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			MeshTopoData *ld = mMeshTopoData[ldID];
			BitArray esel = ld->GetTVEdgeSelection();
			int edgeCount = esel.GetSize();
			for (int i = 0; i < edgeCount; i++)
			{
				if (esel[i])
				{
					int ct = ld->GetTVEdgeNumberTVFaces(i);//TVMaps.ePtrList[i]->faceList.Count();
					if (ct <= 1)
					{
						openEdgeSel = TRUE;
						i = edgeCount;
						ldID = mMeshTopoData.Count();
					}
				}
			}
		}
		if (openEdgeSel)
			GrowSelectOpenEdge();
		GrowUVLoop(FALSE);
	}
	else ExpandSelection(0);

	if (fnGetSyncSelectionMode()) 
		fnSyncGeomSelection();

	theHold.Resume();

	InvalidateView();
	UpdateWindow(hWnd);

	
}


void UnwrapMod::fnContractSelection()
{
	//	if (iIncSelected) iIncSelected->SetCurFlyOff(1,TRUE);

	if ((fnGetTVSubMode() == TVEDGEMODE)  )
		ShrinkSelectOpenEdge();
	else ExpandSelection(1);

	if (fnGetSyncSelectionMode()) 
		fnSyncGeomSelection();

	InvalidateView();
	UpdateWindow(hWnd);
}

void UnwrapMod::fnSetFalloffType(int falloff)
{
	if (iFalloff) 
		iFalloff->SetCurFlyOff(falloff-1,TRUE);
	else
	{
		this->falloff = falloff-1;
		RebuildDistCache();
		InvalidateView();
	}
}
int	UnwrapMod::fnGetFalloffType()
{
	if (iFalloff) 
		return iFalloff->GetCurFlyOff()+1;
	else
	{
		return falloff+1;
	}
	return -1;

}
void UnwrapMod::fnSetFalloffSpace(int space)
{
	if (iFalloffSpace) 
		iFalloffSpace->SetCurFlyOff(space-1,TRUE);
	else
	{
		falloffSpace = space -1;
		RebuildDistCache();
		InvalidateView();

	}
}
int	UnwrapMod::fnGetFalloffSpace()
{
	if (iFalloffSpace) 
		return iFalloffSpace->GetCurFlyOff()+1;
	else
	{
		return falloffSpace +1;
	}
	return -1;

}

void UnwrapMod::fnSetFalloffDist(float dist)
{
	if (iStr) iStr->SetValue(dist,TRUE);
}
float UnwrapMod::fnGetFalloffDist()
{
	if (iStr) return iStr->GetFVal();
	return -1.0f;
}

void UnwrapMod::fnBreakSelected()
{
	ClearAFlag(A_HELD);
	BreakSelected();
	InvalidateView();
}
void UnwrapMod::fnWeld()
{
	if ((mode == ID_UNWRAP_WELD)||(mode == ID_TOOL_WELD))
		SetMode(oldMode);
	else SetMode(ID_TOOL_WELD);
}
void UnwrapMod::fnWeldSelected()
{
	ClearAFlag(A_HELD);
	WeldSelected(TRUE,TRUE);
}


void UnwrapMod::fnUpdatemap()
{
	SendMessage(hWnd,WM_COMMAND,ID_TOOL_UPDATE,0);
}

void UnwrapMod::fnDisplaymap(BOOL update)
{
	if (iShowMap)
	{
		iShowMap->SetCheck(update);
		SendMessage(hWnd,WM_COMMAND,ID_TOOL_SHOWMAP,0);
	}
	else
	{
	}

}
BOOL UnwrapMod::fnIsMapDisplayed()
{
	return showMap;
}


void UnwrapMod::fnSetUVSpace(int space)
{
	if (iUVW) 
		iUVW->SetCurFlyOff(space-1,TRUE);
	else
	{
		uvw = space -1;
		InvalidateView();
	}	
}

int	UnwrapMod::fnGetUVSpace()
{
	return uvw+1;
}

void UnwrapMod::fnOptions()
{
	//	if (iProp) 
	SendMessage(hWnd,WM_COMMAND,ID_TOOL_PROP,0);
	//	else
	//		{
	//		}	
}


void UnwrapMod::fnLock()
{
	if (iLockSelected)
	{
		if (iLockSelected->IsChecked())
		{
			iLockSelected->SetCheck(FALSE);
			lockSelected = FALSE;
		}
		else 
		{
			iLockSelected->SetCheck(TRUE);
			lockSelected = TRUE;
		}

	}
	else
	{
		lockSelected = !lockSelected;
	}
	SendMessage(hWnd,WM_COMMAND,ID_TOOL_LOCKSELECTED,0);
}

BOOL	UnwrapMod::fnGetLock()
{
	return lockSelected;
}
void	UnwrapMod::fnSetLock(BOOL lock)
{
	lockSelected = lock;

	if (iLockSelected)
	{
		if (lockSelected)
		{
			iLockSelected->SetCheck(TRUE);
		}
		else 
		{
			iLockSelected->SetCheck(FALSE);
		}
	}
	SendMessage(hWnd,WM_COMMAND,ID_TOOL_LOCKSELECTED,0);
}

void UnwrapMod::fnHide()
{
	if (iHide) 
		iHide->SetCurFlyOff(0,TRUE);
	else
	{
		HideSelected();
		InvalidateView();
	}
}
void UnwrapMod::fnUnhide()
{
	if (iHide) 
		iHide->SetCurFlyOff(1,TRUE);
	else
	{
		UnHideAll();
		InvalidateView();
	}
}

void UnwrapMod::fnFreeze()
{
	if (iFreeze) 
		iFreeze->SetCurFlyOff(0,TRUE);
	else
	{
		FreezeSelected();
		InvalidateView();
	}	


}
void UnwrapMod::fnThaw()
{
	if (iFreeze) 
		iFreeze->SetCurFlyOff(1,TRUE);
	else
	{
		UnFreezeAll();
		InvalidateView();
	}	
}
void UnwrapMod::fnFilterSelected()
{
	if (iFilterSelected)
	{
		if (iFilterSelected->IsChecked())
		{
			iFilterSelected->SetCheck(FALSE);
			filterSelectedFaces = FALSE;
		}
		else
		{
			iFilterSelected->SetCheck(TRUE);
			filterSelectedFaces = TRUE;
		}
		SendMessage(hWnd,WM_COMMAND,ID_TOOL_FILTER_SELECTEDFACES,0);
	}
	else
	{
		filterSelectedFaces = !filterSelectedFaces;
		SendMessage(hWnd,WM_COMMAND,ID_TOOL_FILTER_SELECTEDFACES,0);
	}
	BuildFilterSelectedFacesData();
}

BOOL	UnwrapMod::fnGetFilteredSelected()
{
	return filterSelectedFaces;
}
void	UnwrapMod::fnSetFilteredSelected(BOOL filter)
{
	filterSelectedFaces = filter;
	if (iFilterSelected)
	{
		if (filterSelectedFaces)
		{
			iFilterSelected->SetCheck(TRUE);
		}
		else
		{
			iFilterSelected->SetCheck(FALSE);
		}

	}
	SendMessage(hWnd,WM_COMMAND,ID_TOOL_FILTER_SELECTEDFACES,0);
	BuildFilterSelectedFacesData();
}



void UnwrapMod::fnPan()
{
	//	if (iPan) 
	SetMode(ID_TOOL_PAN);
}
void UnwrapMod::fnZoom()
{
	//	if (iWeld) 
	SetMode(ID_TOOL_ZOOM);
}
void UnwrapMod::fnZoomRegion()
{
	//	if (iWeld) 
	SetMode(ID_TOOL_ZOOMREG);
}
void UnwrapMod::fnFit()
{
	if (iZoomExt) 		
		iZoomExt->SetCurFlyOff(0,TRUE);
	else
	{
		ZoomExtents();
	}
}
void UnwrapMod::fnFitSelected()
{
	if (iZoomExt) 
		iZoomExt->SetCurFlyOff(1,TRUE);
	else
	{
		ZoomSelected();
	}
}

void UnwrapMod::fnSnap()
{
	if (iSnap)
	{
		iSnap->SetCurFlyOff(1);
		if (iSnap->IsChecked())
			iSnap->SetCheck(FALSE);
		else iSnap->SetCheck(TRUE);
	}
	else
	{
		pixelSnap = !pixelSnap;
	}
	gridSnap = FALSE;

	SendMessage(hWnd,WM_COMMAND,ID_TOOL_SNAP,0);

}

BOOL	UnwrapMod::fnGetSnap()
{
	return pixelSnap;
}
void	UnwrapMod::fnSetSnap(BOOL snap)
{
	pixelSnap = snap;
	if (pixelSnap)
		gridSnap = FALSE;

	if (iSnap)
	{
		iSnap->SetCurFlyOff(1);
		if (pixelSnap)
			iSnap->SetCheck(TRUE);
		else iSnap->SetCheck(FALSE);
	}

	SendMessage(hWnd,WM_COMMAND,ID_TOOL_SNAP,0);

}


int	UnwrapMod::fnGetCurrentMap()
{
	return CurrentMap+1;
}
void UnwrapMod::fnSetCurrentMap(int map)
{
	map--;
	int ct = SendMessage( hTextures, CB_GETCOUNT, 0, 0 )-3;

	if ( (map < ct) && (CurrentMap!=map))
	{
		CurrentMap = map;
		SendMessage(hTextures, CB_SETCURSEL, map, 0 );
		SetupImage();
	}
}	
int	UnwrapMod::fnNumberMaps()
{
	int ct = SendMessage( hTextures, CB_GETCOUNT, 0, 0 )-3;
	return ct;
}

Point3 *UnwrapMod::fnGetLineColor()
{
	AColor c(lineColor);

	lColor.x = c.r;
	lColor.y = c.g;
	lColor.z = c.b;
	return &lColor;
}
void UnwrapMod::fnSetLineColor(Point3 color)
{
	AColor c;
	c.r = color.x;
	c.g = color.y;
	c.b = color.z;
	lineColor = c.toRGB();
	InvalidateView();
}

Point3 *UnwrapMod::fnGetSelColor()
{
	AColor c(selColor);
	lColor.x = c.r;
	lColor.y = c.g;
	lColor.z = c.b;
	return &lColor;
}

void UnwrapMod::fnSetSelColor(Point3 color)
{
	AColor c;
	c.r = color.x;
	c.g = color.y;
	c.b = color.z;

	selColor = c.toRGB();
	InvalidateView();
}

void UnwrapMod::fnSetRenderWidth(int dist)
{
	rendW = dist;
	if (rendW!=iw)
		SetupImage();
	InvalidateView();
}
int UnwrapMod::fnGetRenderWidth()
{
	return rendW;
}
void UnwrapMod::fnSetRenderHeight(int dist)
{
	rendH = dist;
	if (rendH!=ih)
		SetupImage();
	InvalidateView();
}
int UnwrapMod::fnGetRenderHeight()
{
	return rendH;
}

void UnwrapMod::fnSetWeldThreshold(float dist)
{
	weldThreshold = dist;
}
float UnwrapMod::fnGetWeldThresold()
{
	return weldThreshold;
}

void UnwrapMod::fnSetUseBitmapRes(BOOL useBitmapRes)
{
	BOOL change= FALSE;
	if (this->useBitmapRes != useBitmapRes)
		change = TRUE;
	this->useBitmapRes = useBitmapRes;
	if (change)
		SetupImage();
	InvalidateView();
}
BOOL UnwrapMod::fnGetUseBitmapRes()
{
	return useBitmapRes;
}



BOOL UnwrapMod::fnGetConstantUpdate()
{
	return update;
}
void UnwrapMod::fnSetConstantUpdate(BOOL constantUpdates)
{
	update = constantUpdates;
}

BOOL UnwrapMod::fnGetShowSelectedVertices()
{
	return showVerts;
}
void UnwrapMod::fnSetShowSelectedVertices(BOOL show)
{
	showVerts = show;
	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	InvalidateView();

}

BOOL UnwrapMod::fnGetMidPixelSnape()
{
	return midPixelSnap;
}

void UnwrapMod::fnSetMidPixelSnape(BOOL midPixel)
{
	midPixelSnap = midPixel;
}


int	UnwrapMod::fnGetMatID()
{
	return matid+2;
}
void UnwrapMod::fnSetMatID(int mid)
{
	mid--;
	int ct = SendMessage( hMatIDs, CB_GETCOUNT, 0, 0 );

	if ( (mid < ct) && (matid!=(mid-1)))
	{		
		SendMessage(hMatIDs, CB_SETCURSEL, mid, 0 );
		mid-=1;

		matid = mid;
		InvalidateView();
	}

}
int	UnwrapMod::fnNumberMatIDs()
{
	int ct = SendMessage( hMatIDs, CB_GETCOUNT, 0, 0 );
	return ct;
}


void UnwrapMod::fnMoveSelectedVertices(Point3 offset)
{
	Point2 pt;
	pt.x = offset.x;
	pt.y = offset.y;
	theHold.Begin();
	MovePoints(pt);
	theHold.Accept(_T(GetString(IDS_PW_MOVE_UVW)));
}
void UnwrapMod::fnRotateSelectedVertices(float angle, Point3 axis)
{
	theHold.Begin();
	//	centeron=TRUE;
	//	center.x = axis.x;
	//	center.y = axis.y;
	//	center.z = axis.z;
	RotateAroundAxis(tempHwnd, angle, axis);
	//	RotatePoints(tempHwnd, angle);
	theHold.Accept(_T(GetString(IDS_PW_ROTATE_UVW)));

}
void UnwrapMod::fnRotateSelectedVertices(float angle)
{
	theHold.Begin();
	centeron=FALSE;
	RotatePoints(tempHwnd, angle);
	theHold.Accept(_T(GetString(IDS_PW_ROTATE_UVW)));
}
void UnwrapMod::fnScaleSelectedVertices(float scale,int dir)
{
	theHold.Begin();
	centeron=FALSE;
	ScalePoints(tempHwnd, scale,dir);
	theHold.Accept(_T(GetString(IDS_PW_SCALE_UVW)));
}


void UnwrapMod::fnScaleSelectedVertices(float scale,int dir, Point3 axis)
{
	theHold.Begin();
	centeron=TRUE;
	center.x = axis.x;
	center.y = axis.y;
	ScalePoints(tempHwnd, scale,dir);
	theHold.Accept(_T(GetString(IDS_PW_SCALE_UVW)));
}

Point3* UnwrapMod::fnGetVertexPosition(TimeValue t,  int index, INode *node)
{
	index--;
	MeshTopoData *ld = GetMeshTopoData(node);
	if (ld)
	{
		tempVert = ld->GetTVVert(index);
	}	
	return &tempVert;
}

Point3* UnwrapMod::fnGetVertexPosition(TimeValue t,  int index)
{
	index--;
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];		
		if (ld)
		{
			tempVert = ld->GetTVVert(index);
		}
	}
	
	return &tempVert;
}


int	UnwrapMod::fnNumberVertices(INode *node)
{
	
	MeshTopoData *ld = GetMeshTopoData(node);
	if (ld)
	{
		return ld->GetNumberTVVerts();//TVMaps.v.Count();
	}
	
	return 0;
}

int	UnwrapMod::fnNumberVertices()
{
	
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
			return ld->GetNumberTVVerts();//TVMaps.v.Count();
	}
	
	return 0;
}


void UnwrapMod::fnMoveX(float p)
{
	ChannelChanged(0, p);

}
void UnwrapMod::fnMoveY(float p)
{
	ChannelChanged(1, p);
}
void UnwrapMod::fnMoveZ(float p)
{
	ChannelChanged(2, p);
}




// private namespace
namespace
{
	class sMyEnumProc : public DependentEnumProc 
	{
	public :
		virtual int proc(ReferenceMaker *rmaker); 
		INodeTab Nodes;              
	};

	int sMyEnumProc::proc(ReferenceMaker *rmaker) 
	{ 
		if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
		{
			Nodes.Append(1, (INode **)&rmaker);  
			return DEP_ENUM_SKIP;
		}

		return DEP_ENUM_CONTINUE;
	}
}



class ModDataOnStack : public GeomPipelineEnumProc
	{
public:  
   ModDataOnStack(ReferenceTarget *me) : mRef(me),mModData(NULL) {}
   PipeEnumResult proc(ReferenceTarget *object, IDerivedObject *derObj, int index);
   ReferenceTarget *mRef;
   MeshTopoData *mModData;
protected:
   ModDataOnStack(); // disallowed
   ModDataOnStack(ModDataOnStack& rhs); // disallowed
   ModDataOnStack& operator=(const ModDataOnStack& rhs); // disallowed
};

PipeEnumResult ModDataOnStack::proc(
   ReferenceTarget *object, 
   IDerivedObject *derObj, 
   int index)
{
   if ((derObj != NULL) && object == mRef) //found it!
   {
		ModContext *mc = derObj->GetModContext(index);
		mModData = (MeshTopoData*)mc->localData;
        return PIPE_ENUM_STOP;
   }
   return PIPE_ENUM_CONTINUE;
}



//new
MeshTopoData *UnwrapMod::GetModData()
{
	sMyEnumProc dep;              
	DoEnumDependents(&dep);
	if (dep.Nodes.Count() > 0)
	{
		INode *node = dep.Nodes[0];
		Object* obj = node->GetObjectRef();
		ModDataOnStack pipeEnumProc(this);
        EnumGeomPipeline(&pipeEnumProc, obj);
		return pipeEnumProc.mModData;
	}
	return NULL;
}


BitArray* UnwrapMod::fnGetSelectedPolygons(INode *node)
{

	MeshTopoData *ld = GetMeshTopoData(node);
	if (ld)
	{
		return ld->GetFaceSelectionPtr();
	}
	return NULL;
}

BitArray* UnwrapMod::fnGetSelectedPolygons()
{
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
			return ld->GetFaceSelectionPtr();
	}
	return NULL;
}


void UnwrapMod::fnSelectPolygons(BitArray *sel, INode *node)
{
		MeshTopoData *ld = GetMeshTopoData(node);
		if (ld)
		{
			ld->ClearFaceSelection();
			for (int i =0; i < ld->GetNumberFaces(); i++)
			{
				if (i < sel->GetSize())
				{
					if ((*sel)[i]) 
						ld->SetFaceSelected(i,TRUE);//md->faceSel.Set(i);
				}
			}
			fnSyncTVSelection();
			NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
			InvalidateView();
		}
}



void UnwrapMod::fnSelectPolygons(BitArray *sel)
{
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			ld->ClearFaceSelection();
			for (int i =0; i < ld->GetNumberFaces(); i++)
			{
				if (i < sel->GetSize())
				{
					if ((*sel)[i]) 
						ld->SetFaceSelected(i,TRUE);//md->faceSel.Set(i);
				}
			}
			fnSyncTVSelection();
			NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
			InvalidateView();
		}
	}
}


BOOL UnwrapMod::fnIsPolygonSelected(int index, INode *node)
{
	index--;
	MeshTopoData *ld = GetMeshTopoData(node);
	if (ld)
	{
		return ld->GetFaceSelected(index);
	}

	return FALSE;	
}

BOOL UnwrapMod::fnIsPolygonSelected(int index)
{
	index--;
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			return ld->GetFaceSelected(index);
		}
	}

	return FALSE;	
}

int	UnwrapMod::fnNumberPolygons(INode *node)
{
	MeshTopoData *ld = GetMeshTopoData(node);
	if (ld)
	{
		return ld->GetNumberFaces();
	}
	return 0;
}

int	UnwrapMod::fnNumberPolygons()
{
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			return ld->GetNumberFaces();
		}
	}
	return 0;
}

void UnwrapMod::fnDetachEdgeVerts()
{
	DetachEdgeVerts();
}

void UnwrapMod::fnFlipH()
{
	if (iMirror) 
		iMirror->SetCurFlyOff(2,TRUE);
	else
	{
		mirror = 2;
		FlipPoints(mirror-2);
	}

}
void UnwrapMod::fnFlipV()
{
	if (iMirror) iMirror->SetCurFlyOff(3,TRUE);
	else
	{
		mirror = 3;		
		FlipPoints(mirror-2);
	}
}

BOOL UnwrapMod::fnGetLockAspect()
{
	return lockAspect;
}
void UnwrapMod::fnSetLockAspect(BOOL a)
{
	lockAspect = a;
	InvalidateView();
}


float UnwrapMod::fnGetMapScale()
{
	return mapScale;
}
void UnwrapMod::fnSetMapScale(float sc)
{
	mapScale = sc;
	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	InvalidateView();

}

void UnwrapMod::fnGetSelectionFromFace()
{
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		if (ld)
		{
			BitArray vsel = ld->GetTVVertSelection();
			BitArray tempSel(vsel);
			tempSel.ClearAll();

			for (int i = 0; i < ld->GetNumberFaces(); i++)//TVMaps.f.Count(); i++)
			{
				if (!ld->GetFaceDead(i))//(TVMaps.f[i]->flags & FLAG_DEAD))
				{
					if (ld->GetFaceSelected(i))//TVMaps.f[i]->flags & FLAG_SELECTED)
					{
						int degree = ld->GetFaceDegree(i);
						for (int j = 0; j < degree; j++)
						{
							int id = ld->GetFaceTVVert(i,j);//TVMaps.f[i]->t[j];
							tempSel.Set(id,TRUE);
						}
					}
				}
			}
			if (fnGetTVSubMode() == TVVERTMODE)
			{
				ld->SetTVVertSelection(tempSel);//	vsel = tempSel;
			}
			else if (fnGetTVSubMode() == TVEDGEMODE)
			{
				BitArray holdSel(vsel);
				ld->SetTVVertSelection(tempSel);//vsel = tempSel;
				BitArray esel;
				ld->GetEdgeSelFromVert(esel,FALSE);
				ld->SetTVEdgeSelection(esel);
				ld->SetTVVertSelection(holdSel);//vsel = holdSel;
			}
			else if (fnGetTVSubMode() == TVFACEMODE)
			{
				BitArray holdSel(vsel);
				ld->SetTVVertSelection(tempSel);//vsel = tempSel;
				BitArray fsel;
				ld->GetFaceSelFromVert(fsel,FALSE);
				ld->SetFaceSelection(fsel);//
				ld->SetTVVertSelection(holdSel);//vsel = holdSel;
			}



		}
	}
	InvalidateView();
}

void UnwrapMod::fnForceUpdate(BOOL update)
{
	forceUpdate = update;
}

void UnwrapMod::fnZoomToGizmo(BOOL all)
{
	if (ip)
	{
		if (!gizmoBounds.IsEmpty() && ip && (ip->GetSubObjectLevel() == 3))
			ip->ZoomToBounds(all,gizmoBounds);
	}		
}

void UnwrapMod::fnSetVertexPosition(TimeValue t, int index, Point3 pos, INode *node)
{
	index --;
	MeshTopoData *ld = GetMeshTopoData(node);
	if (ld)
		SetVertexPosition(ld,t, index, pos);
	
}

void UnwrapMod::fnSetVertexPosition(TimeValue t, int index, Point3 pos)
{
	index --;
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
			SetVertexPosition(ld,t, index, pos);
	}
	
}

void UnwrapMod::fnSetVertexPosition2(TimeValue t, int index, Point3 pos, BOOL hold, BOOL update, INode *node)
{
	index --;
	MeshTopoData *ld = GetMeshTopoData(node);
	if (ld)
		SetVertexPosition(ld,t, index, pos, hold, update);

}

void UnwrapMod::fnSetVertexPosition2(TimeValue t, int index, Point3 pos, BOOL hold, BOOL update)
{
	index --;
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
			SetVertexPosition(ld,t, index, pos, hold, update);
	}

}

void UnwrapMod::fnMarkAsDead(int index, INode *node)
{
	index--;
		MeshTopoData *ld = GetMeshTopoData(node);
		if (ld)
		{
			ld->SetTVVertDead(index,TRUE);
		}

}

void UnwrapMod::fnMarkAsDead(int index)
{
	index--;
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			ld->SetTVVertDead(index,TRUE);
		}
	}

}


int UnwrapMod::fnNumberPointsInFace(int index, INode *node)
{
	index--;
		MeshTopoData *ld = GetMeshTopoData(node);
		if (ld)
		{
			return ld->GetFaceDegree(index);
		}
	return 0;
}


int UnwrapMod::fnNumberPointsInFace(int index)
{
	index--;
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			return ld->GetFaceDegree(index);
		}
	}
	return 0;
}

int UnwrapMod::fnGetVertexIndexFromFace(int index,int vertexIndex, INode *node)
{

	index--;
	vertexIndex--;
	MeshTopoData *ld = GetMeshTopoData(node);
	if (ld)
	{
		return ld->GetFaceTVVert(index,vertexIndex)+1;
	}

	return 0;
}

int UnwrapMod::fnGetVertexIndexFromFace(int index,int vertexIndex)
{

	index--;
	vertexIndex--;
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			return ld->GetFaceTVVert(index,vertexIndex)+1;
		}
	}

	return 0;
}


int UnwrapMod::fnGetHandleIndexFromFace(int index,int vertexIndex, INode *node)
{
	index--;
	vertexIndex--;

	MeshTopoData *ld = GetMeshTopoData(node);
	if (ld)
	{
		if (ld->GetFaceHasVectors(index))
			return ld->GetFaceTVHandle(index,vertexIndex)+1;
	}
	return 0;
}


int UnwrapMod::fnGetHandleIndexFromFace(int index,int vertexIndex)
{
	index--;
	vertexIndex--;

	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			if (ld->GetFaceHasVectors(index))
				return ld->GetFaceTVHandle(index,vertexIndex)+1;
		}
	}
	return 0;
}

int UnwrapMod::fnGetInteriorIndexFromFace(int index,int vertexIndex, INode *node)
{

	index--;
	vertexIndex--;

		MeshTopoData *ld = GetMeshTopoData(node);
		if (ld)
		{
			if (ld->GetFaceHasVectors(index))
				return ld->GetFaceTVInterior(index,vertexIndex)+1;
		}
	return 0;
}


int UnwrapMod::fnGetInteriorIndexFromFace(int index,int vertexIndex)
{

	index--;
	vertexIndex--;

	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			if (ld->GetFaceHasVectors(index))
				return ld->GetFaceTVInterior(index,vertexIndex)+1;
		}
	}
	return 0;
}


int UnwrapMod::fnGetVertexGIndexFromFace(int index,int vertexIndex, INode *node)
{

	index--;
	vertexIndex--;
		MeshTopoData *ld = GetMeshTopoData(node);
		if (ld)
		{
			return ld->GetFaceGeomVert(index,vertexIndex)+1;
		}
	return 0;
}


int UnwrapMod::fnGetVertexGIndexFromFace(int index,int vertexIndex)
{

	index--;
	vertexIndex--;
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			return ld->GetFaceGeomVert(index,vertexIndex)+1;
		}
	}
	return 0;
}

int UnwrapMod::fnGetHandleGIndexFromFace(int index,int vertexIndex, INode *node)
{

	index--;
	vertexIndex--;


		MeshTopoData *ld = GetMeshTopoData(node);
		if (ld)
		{
			if (ld->GetFaceHasVectors(index))
				return ld->GetFaceGeomHandle(index,vertexIndex)+1;
		}
	return 0;
}


int UnwrapMod::fnGetHandleGIndexFromFace(int index,int vertexIndex)
{

	index--;
	vertexIndex--;

	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			if (ld->GetFaceHasVectors(index))
				return ld->GetFaceGeomHandle(index,vertexIndex)+1;
		}
	}
	return 0;
}

int UnwrapMod::fnGetInteriorGIndexFromFace(int index,int vertexIndex, INode *node)
{

	index--;
	vertexIndex--;
		MeshTopoData *ld = GetMeshTopoData(node);
		if (ld)
		{
			if (ld->GetFaceHasVectors(index))
				return ld->GetFaceGeomInterior(index,vertexIndex)+1;
		}
	return 0;
}


int UnwrapMod::fnGetInteriorGIndexFromFace(int index,int vertexIndex)
{

	index--;
	vertexIndex--;
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			if (ld->GetFaceHasVectors(index))
				return ld->GetFaceGeomInterior(index,vertexIndex)+1;
		}
	}
	return 0;
}


void UnwrapMod::fnAddPoint(Point3 pos, int fIndex,int vIndex, BOOL sel, INode *node)
{

	fIndex--;
	vIndex--;
		MeshTopoData *ld = GetMeshTopoData(node);
		if (ld)
		{
			ld->AddTVVert(0, pos,fIndex,vIndex,this,sel);
		}
}

void UnwrapMod::fnAddPoint(Point3 pos, int fIndex,int vIndex, BOOL sel)
{

	fIndex--;
	vIndex--;
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			ld->AddTVVert(0, pos,fIndex,vIndex,this,sel);
		}
	}
}

void UnwrapMod::fnAddHandle(Point3 pos, int fIndex,int vIndex, BOOL sel,INode *node)
{


	fIndex--;
	vIndex--;
		MeshTopoData *ld = GetMeshTopoData(node);
		if (ld)
		{
			ld->AddTVHandle(0, pos,fIndex,vIndex,this,sel);
		}
}

void UnwrapMod::fnAddHandle(Point3 pos, int fIndex,int vIndex, BOOL sel)
{


	fIndex--;
	vIndex--;
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			ld->AddTVHandle(0, pos,fIndex,vIndex,this,sel);
		}
	}
}

void UnwrapMod::fnAddInterior(Point3 pos, int fIndex,int vIndex, BOOL sel, INode *node)
{

	fIndex--;
	vIndex--;
	MeshTopoData *ld = GetMeshTopoData(node);
	if (ld)
	{
		ld->AddTVInterior(0, pos,fIndex,vIndex,this,sel);
	}
}


void UnwrapMod::fnAddInterior(Point3 pos, int fIndex,int vIndex, BOOL sel)
{

	fIndex--;
	vIndex--;
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			ld->AddTVInterior(0, pos,fIndex,vIndex,this,sel);
		}
	}
}


void UnwrapMod::fnSetFaceVertexIndex(int fIndex,int ithV, int vIndex, INode *node)
{

	fIndex--;
	ithV--;
	vIndex--;

		MeshTopoData *ld = GetMeshTopoData(node);
		if (ld)
		{
			ld->SetFaceTVVert(fIndex,ithV,vIndex);
		}
}


void UnwrapMod::fnSetFaceVertexIndex(int fIndex,int ithV, int vIndex)
{

	fIndex--;
	ithV--;
	vIndex--;
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			ld->SetFaceTVVert(fIndex,ithV,vIndex);
		}
	}
}

void UnwrapMod::fnSetFaceHandleIndex(int fIndex,int ithV, int vIndex, INode *node)
{

	fIndex--;
	ithV--;
	vIndex--;
		MeshTopoData *ld = GetMeshTopoData(node);
		if (ld)
		{
			if (ld->GetFaceHasVectors(fIndex))
				ld->SetFaceTVHandle(fIndex,ithV,vIndex);
		}
}

void UnwrapMod::fnSetFaceHandleIndex(int fIndex,int ithV, int vIndex)
{

	fIndex--;
	ithV--;
	vIndex--;
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			if (ld->GetFaceHasVectors(fIndex))
				ld->SetFaceTVHandle(fIndex,ithV,vIndex);
		}
	}
}

void UnwrapMod::fnSetFaceInteriorIndex(int fIndex,int ithV, int vIndex, INode *node)
{

	fIndex--;
	ithV--;
	vIndex--;
	MeshTopoData *ld = GetMeshTopoData(node);
	if (ld)
	{
		if (ld->GetFaceHasVectors(fIndex))
			ld->SetFaceTVInterior(fIndex,ithV,vIndex);
	}
}


void UnwrapMod::fnSetFaceInteriorIndex(int fIndex,int ithV, int vIndex)
{

	fIndex--;
	ithV--;
	vIndex--;
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			if (ld->GetFaceHasVectors(fIndex))
				ld->SetFaceTVInterior(fIndex,ithV,vIndex);
		}
	}
}

void UnwrapMod::fnUpdateViews()
{
	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	InvalidateView();
}

void UnwrapMod::fnGetFaceSelFromStack()
{
	getFaceSelectionFromStack = TRUE;
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());
	InvalidateView();
}

void UnwrapMod::fnSetDebugLevel(int level)
{
	gDebugLevel = level;
}


BOOL	UnwrapMod::fnGetTile()
{
	return bTile;
}
void	UnwrapMod::fnSetTile(BOOL tile)
{
	bTile = tile;
	tileValid = FALSE;
	InvalidateView();
}

int		UnwrapMod::fnGetTileLimit()
{
	return iTileLimit;

}
void	UnwrapMod::fnSetTileLimit(int limit)
{
	iTileLimit = limit;
	if (iTileLimit < 0 ) iTileLimit = 0;
	if (iTileLimit > 50 ) iTileLimit = 50;
	tileValid = FALSE;
	InvalidateView();
}

float	UnwrapMod::fnGetTileContrast()
{
	return fContrast;
}
void	UnwrapMod::fnSetTileContrast(float fcontrast)
{
	this->fContrast = fcontrast;
	tileValid = FALSE;
	InvalidateView();
}



BOOL	UnwrapMod::fnGetShowMap()
{
	return showMap;
}	

void	UnwrapMod::fnSetShowMap(BOOL smap)
{
	showMap = smap;
	if (iShowMap) 
		iShowMap->SetCheck(smap);
	else InvalidateView();
}

void	UnwrapMod::fnShowMap()
{
	if (iShowMap)
	{
		if (iShowMap->IsChecked())
			iShowMap->SetCheck(FALSE);
		else iShowMap->SetCheck(TRUE);
		SendMessage(hWnd,WM_COMMAND,ID_TOOL_SHOWMAP,0);
	}
	else
	{
		showMap = !showMap;
		InvalidateView();

	}
}

#pragma warning (disable  : 4530)

void	UnwrapMod::InitScriptUI()
{
	if (executedStartUIScript==FALSE)
	{
		TSTR scriptUI;

		scriptUI.printf("mcrfile = openFile   \"UI\\MacroScripts\\Macro_UnwrapUI.mcr\" ; execute mcrfile");


		init_thread_locals();
		push_alloc_frame();
		one_typed_value_local(StringStream* util_script);
		save_current_frames();
		trace_back_active = FALSE;

		try 
		{
			vl.util_script = new StringStream (scriptUI);
			vl.util_script->execute_vf(NULL, 0);
		}
		catch (MAXScriptException& e)
		{
			clear_error_source_data();
			restore_current_frames();
			MAXScript_signals = 0;
			if (progress_bar_up)
				MAXScript_interface->ProgressEnd(), progress_bar_up = FALSE;
			error_message_box(e, _T("Unwrap UI Script Macro_UnwrapUI.mcr not found"));
		}
		catch (...)
		{
			clear_error_source_data();
			restore_current_frames();
			if (progress_bar_up)
				MAXScript_interface->ProgressEnd(), progress_bar_up = FALSE;
			error_message_box(UnknownSystemException (), _T("Unwrap UI Script Macro_UnwrapUI.mcr not found"));
		}
		vl.util_script->close();
		pop_value_locals();
		pop_alloc_frame();
		executedStartUIScript=TRUE;
	}

}

void	UnwrapMod::LaunchScriptUI()
{
	TSTR scriptUI;

	scriptUI.printf("macros.run \"UVW Unwrap\" \"OpenUnwrapUI\" ");


	init_thread_locals();
	push_alloc_frame();
	one_typed_value_local(StringStream* util_script);
	save_current_frames();
	trace_back_active = FALSE;

	try 
	{
		vl.util_script = new StringStream (scriptUI);
		vl.util_script->execute_vf(NULL, 0);
	}
	catch (MAXScriptException& e)
	{
		clear_error_source_data();
		restore_current_frames();
		restore_current_frames();
		MAXScript_signals = 0;
		if (progress_bar_up)
			MAXScript_interface->ProgressEnd(), progress_bar_up = FALSE;
		error_message_box(e, _T("Unwrap UI Script OpenUnwrapUI Macro not found"));
	}
	catch (...)
	{
		clear_error_source_data();
		restore_current_frames();
		restore_current_frames();
		if (progress_bar_up)
			MAXScript_interface->ProgressEnd(), progress_bar_up = FALSE;
		error_message_box(UnknownSystemException (), _T("Unwrap UI Script  OpenUnwrapUI Macro not found"));
	}
	vl.util_script->close();
	pop_value_locals();
	pop_alloc_frame();
}

void	UnwrapMod::EndScriptUI()
{

	TSTR scriptUI;

	scriptUI.printf("macros.run \"UVW Unwrap\" \"CloseUnwrapUI\" ");

	init_thread_locals();
	push_alloc_frame();
	one_typed_value_local(StringStream* util_script);
	save_current_frames();
	trace_back_active = FALSE;

	try 
	{
		vl.util_script = new StringStream (scriptUI);
		vl.util_script->execute_vf(NULL, 0);
	}
	catch (MAXScriptException& e)
	{
		clear_error_source_data();
		restore_current_frames();
		MAXScript_signals = 0;
		if (progress_bar_up)
			MAXScript_interface->ProgressEnd(), progress_bar_up = FALSE;
		error_message_box(e, _T("Unwrap UI Script CloseUnwrapUI Macro not found"));
	}
	catch (...)
	{
		clear_error_source_data();
		restore_current_frames();
		if (progress_bar_up)
			MAXScript_interface->ProgressEnd(), progress_bar_up = FALSE;
		error_message_box(UnknownSystemException (), _T("Unwrap UI Script  CloseUnwrapUI Macro not found"));
	}
	vl.util_script->close();
	pop_value_locals();
	pop_alloc_frame();


}

void	UnwrapMod::MoveScriptUI()
{

	TSTR scriptUI;

	scriptUI.printf("macros.run \"UVW Unwrap\" \"MoveUnwrapUI\" ");

	init_thread_locals();
	push_alloc_frame();
	one_typed_value_local(StringStream* util_script);
	save_current_frames();
	trace_back_active = FALSE;

	try 
	{
		vl.util_script = new StringStream (scriptUI);
		vl.util_script->execute_vf(NULL, 0);
	}
	catch (MAXScriptException& e)
	{
		clear_error_source_data();
		restore_current_frames();
		MAXScript_signals = 0;
		if (progress_bar_up)
			MAXScript_interface->ProgressEnd(), progress_bar_up = FALSE;
		error_message_box(e, _T("Unwrap UI Script MoveUnwrapUI Macro not found"));
	}
	catch (...)
	{
		clear_error_source_data();
		restore_current_frames();
		if (progress_bar_up)
			MAXScript_interface->ProgressEnd(), progress_bar_up = FALSE;
		error_message_box(UnknownSystemException (), _T("Unwrap UI Script  MoveUnwrapUI Macro not found"));
	}
	vl.util_script->close();
	pop_value_locals();
	pop_alloc_frame();


}


#pragma warning (default  : 4530)

int		UnwrapMod::fnGetWindowX()
{
	WINDOWPLACEMENT floaterPos;
	GetWindowPlacement(hWnd,&floaterPos);



	WINDOWPLACEMENT maxPos;
	Interface *ip = GetCOREInterface();
	HWND maxHwnd = ip->GetMAXHWnd();
	GetWindowPlacement(maxHwnd,&maxPos);

	RECT rect;
	GetWindowRect(  hWnd ,&rect);

	if (floaterPos.showCmd == SW_MINIMIZE)
		return 0;
	else return rect.left;//return floaterPos.rcNormalPosition.left;
}
int		UnwrapMod::fnGetWindowY()
{
	WINDOWPLACEMENT floaterPos;
	GetWindowPlacement(hWnd,&floaterPos);

	RECT rect;
	GetWindowRect(  hWnd ,&rect);	

	if (floaterPos.showCmd == SW_MINIMIZE)
		return 0;
	else return rect.top;//floaterPos.rcNormalPosition.top;
}
int		UnwrapMod::fnGetWindowW()
{
	WINDOWPLACEMENT floaterPos;
	floaterPos.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(hWnd,&floaterPos);
	if (floaterPos.showCmd == SW_MAXIMIZE)
		return maximizeWidth-xWindowOffset-2;
	if (floaterPos.showCmd == SW_MINIMIZE)
		return 0;
	else return (floaterPos.rcNormalPosition.right-floaterPos.rcNormalPosition.left);

}
int		UnwrapMod::fnGetWindowH()
{
	WINDOWPLACEMENT floaterPos;
	floaterPos.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(hWnd,&floaterPos);

	if (floaterPos.showCmd == SW_MAXIMIZE)
		return maximizeHeight-yWindowOffset;
	if ((floaterPos.showCmd == SW_MINIMIZE) || (minimized))
		return 0;
	else return (floaterPos.rcNormalPosition.bottom-floaterPos.rcNormalPosition.top);
}


BOOL	UnwrapMod::fnGetBackFaceCull()
{
	return ignoreBackFaceCull;
}	

void	UnwrapMod::fnSetBackFaceCull(BOOL backFaceCull)
{
	ignoreBackFaceCull = backFaceCull;
	//update UI
	CheckDlgButton(hSelRollup,IDC_IGNOREBACKFACING_CHECK,ignoreBackFaceCull);
}

BOOL	UnwrapMod::fnGetOldSelMethod()
{
	return oldSelMethod;
}
void	UnwrapMod::fnSetOldSelMethod(BOOL oldSelMethod)
{
	this->oldSelMethod = oldSelMethod;
}

BOOL	UnwrapMod::fnGetAlwaysEdit()
{
	return alwaysEdit;
}
void	UnwrapMod::fnSetAlwaysEdit(BOOL always)
{
	this->alwaysEdit = always;
}


int		UnwrapMod::fnGetTVSubMode()
{
	return TVSubObjectMode+1;
}
void	UnwrapMod::fnSetTVSubMode(int smode)
{
	TVSubObjectMode = smode-1;

	if (ip)
	{
		if (smode != ip->GetSubObjectLevel())
			ip->SetSubObjectLevel(smode);
	}
	if (fnGetSyncSelectionMode())
	{
		theHold.Suspend();
		fnSyncGeomSelection();
		theHold.Resume();
	}

	if (smode == 3)
	{
		if ((mode == ID_UNWRAP_WELD)||(mode == ID_TOOL_WELD))
			SetMode(oldMode);
	}

	if ( (ip) &&(hWnd) )
	{	
		IMenuBarContext* pContext = (IMenuBarContext*) GetCOREInterface()->GetMenuManager()->GetContext(kUnwrapMenuBar);
		if (pContext)
			pContext->UpdateWindowsMenu();
	}

	InvalidateView();
}

int		UnwrapMod::fnGetFillMode()
{
	return fillMode;
}

void	UnwrapMod::fnSetFillMode(int mode)
{
	fillMode = mode;
	InvalidateView();
}

void UnwrapMod::fnMoveSelected(Point3 offset)
{
	Point2 pt;
	pt.x = offset.x;
	pt.y = offset.y;
	TransferSelectionStart();
	theHold.Begin();
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		theHold.Put(new TVertRestore(this,ld,FALSE));
	}
	theHold.Accept(_T(GetString(IDS_PW_MOVE_UVW)));
	MovePoints(pt);
	TransferSelectionEnd(FALSE,FALSE);
}
void UnwrapMod::fnRotateSelected(float angle, Point3 axis)
{
	TransferSelectionStart();
	theHold.Begin();
	//	centeron=TRUE;
	//	center.x = axis.x;
	//	center.y = axis.y;
	//	center.z = axis.z;
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		theHold.Put(new TVertRestore(this,ld,FALSE));
	}
	theHold.Accept(_T(GetString(IDS_PW_ROTATE_UVW)));

	RotateAroundAxis(tempHwnd, angle,axis);
	TransferSelectionEnd(FALSE,FALSE);

}
void UnwrapMod::fnRotateSelected(float angle)
{
	TransferSelectionStart();
	theHold.Begin();
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		theHold.Put(new TVertRestore(this,ld,FALSE));
	}
	theHold.Accept(_T(GetString(IDS_PW_ROTATE_UVW)));
	centeron=FALSE;
	RotatePoints(tempHwnd, angle);
	TransferSelectionEnd(FALSE,FALSE);
}
void UnwrapMod::fnScaleSelected(float scale,int dir)
{
	TransferSelectionStart();
	theHold.Begin();
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		theHold.Put(new TVertRestore(this,ld,FALSE));
	}
	theHold.Accept(_T(GetString(IDS_PW_SCALE_UVW)));
	centeron=FALSE;
	ScalePoints(tempHwnd, scale,dir);
	TransferSelectionEnd(FALSE,FALSE);
}

void UnwrapMod::fnScaleSelected(float scale,int dir, Point3 axis)
{
	TransferSelectionStart();
	theHold.Begin();
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		theHold.Put(new TVertRestore(this,ld,FALSE));
	}
	theHold.Accept(_T(GetString(IDS_PW_SCALE_UVW)));

	centeron=TRUE;
	center.x = axis.x;
	center.y = axis.y;

	ScalePoints(tempHwnd, scale,dir);
	TransferSelectionEnd(FALSE,FALSE);
}

void UnwrapMod::fnScaleSelectedXY(float scaleX,float scaleY, Point3 axis)
{
	TransferSelectionStart();
	theHold.Begin();
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		theHold.Put(new TVertRestore(this,ld,FALSE));
	}
	theHold.Accept(_T(GetString(IDS_PW_SCALE_UVW)));

	ScaleAroundAxis(tempHwnd, scaleX,scaleY,axis);
	TransferSelectionEnd(FALSE,FALSE);
}


BOOL	UnwrapMod::fnGetDisplayOpenEdges()
{
	return displayOpenEdges;
}
void	UnwrapMod::fnSetDisplayOpenEdges(BOOL openEdgeDisplay)
{
	displayOpenEdges = openEdgeDisplay;
	InvalidateView();
}

BOOL	UnwrapMod::fnGetThickOpenEdges()
{
	return thickOpenEdges;
}
void	UnwrapMod::fnSetThickOpenEdges(BOOL thick)
{
	if (theHold.Holding())
	{
		theHold.Put(new UnwrapSeamAttributesRestore(this));
	}

	thickOpenEdges = thick;

	CheckDlgButton(hParams,IDC_THICKSEAM,FALSE);
	CheckDlgButton(hParams,IDC_THINSEAM,FALSE);
	if (thick)
		CheckDlgButton(hParams,IDC_THICKSEAM,TRUE);
	else CheckDlgButton(hParams,IDC_THINSEAM,TRUE);

	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	if (ip) 
		ip->RedrawViews(ip->GetTime());

	InvalidateView();
}

BOOL	UnwrapMod::fnGetViewportOpenEdges()
{
	return viewportOpenEdges;
}
void	UnwrapMod::fnSetViewportOpenEdges(BOOL show)
{

	if (theHold.Holding())
	{
		theHold.Put(new UnwrapSeamAttributesRestore(this));
	}

	viewportOpenEdges = show;
	CheckDlgButton(hParams,IDC_SHOWMAPSEAMS_CHECK,show);
	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	if (ip) 
		ip->RedrawViews(ip->GetTime());
	InvalidateView();
}


Point3 *UnwrapMod::fnGetOpenEdgeColor()
{
	AColor c(openEdgeColor);

	lColor.x = c.r;
	lColor.y = c.g;
	lColor.z = c.b;
	return &lColor;
}
void UnwrapMod::fnSetOpenEdgeColor(Point3 color)
{
	AColor c;
	c.r = color.x;
	c.g = color.y;
	c.b = color.z;
	openEdgeColor = c.toRGB();
	InvalidateView();
}

BOOL	 UnwrapMod::fnGetDisplayHiddenEdges()
{
	return displayHiddenEdges;
}
void	 UnwrapMod::fnSetDisplayHiddenEdges(BOOL hiddenEdgeDisplay)
{
	displayHiddenEdges = hiddenEdgeDisplay;
	InvalidateView();
}



Point3 *UnwrapMod::fnGetHandleColor()
{
	AColor c(handleColor);

	lColor.x = c.r;
	lColor.y = c.g;
	lColor.z = c.b;
	return &lColor;
}
void UnwrapMod::fnSetHandleColor(Point3 color)
{
	AColor c;
	c.r = color.x;
	c.g = color.y;
	c.b = color.z;
	handleColor = c.toRGB();
	InvalidateView();
}


BOOL	UnwrapMod::fnGetFreeFormMode()
{
	if (mode ==ID_FREEFORMMODE)
		return TRUE; 
	else return FALSE; 

}
void	UnwrapMod::fnSetFreeFormMode(BOOL freeFormMode)
{
	if (freeFormMode)
	{
		SetMode(ID_FREEFORMMODE);
	}
}


Point3 *UnwrapMod::fnGetFreeFormColor()
{
	AColor c(freeFormColor);

	lColor.x = c.r;
	lColor.y = c.g;
	lColor.z = c.b;
	return &lColor;
}
void UnwrapMod::fnSetFreeFormColor(Point3 color)
{
	AColor c;
	c.r = color.x;
	c.g = color.y;
	c.b = color.z;
	freeFormColor = c.toRGB();
	InvalidateView();
}

void	UnwrapMod::fnSnapPivot(int pos)
{
	//snap to center
	if (pos == 1)
		freeFormPivotOffset = Point3(0.0f,0.0f,0.0f);
	else if (pos == 2)
		freeFormPivotOffset = selCenter - freeFormCorners[3];
	else if (pos == 3)
		freeFormPivotOffset = selCenter - freeFormEdges[2];
	else if (pos == 4)
		freeFormPivotOffset = selCenter - freeFormCorners[2];
	else if (pos == 5)
		freeFormPivotOffset = selCenter - freeFormEdges[3];
	else if (pos == 6)
		freeFormPivotOffset = selCenter - freeFormCorners[0];
	else if (pos == 7)
		freeFormPivotOffset = selCenter - freeFormEdges[0];
	else if (pos == 8)
		freeFormPivotOffset = selCenter - freeFormCorners[1];
	else if (pos == 9)
		freeFormPivotOffset = selCenter - freeFormEdges[1];

	InvalidateView();

}

Point3*	UnwrapMod::fnGetPivotOffset()
{
	return &freeFormPivotOffset;
}
void	UnwrapMod::fnSetPivotOffset(Point3 offset)
{
	freeFormPivotOffset = offset;
	InvalidateView();
}
Point3*	UnwrapMod::fnGetSelCenter()
{
	RebuildFreeFormData();
	return &selCenter;
}

BOOL	UnwrapMod::fnGetResetPivotOnSel()
{
	return resetPivotOnSel;
}
void	UnwrapMod::fnSetResetPivotOnSel(BOOL reset)
{
	resetPivotOnSel = reset;
}

BOOL	UnwrapMod::fnGetAllowSelectionInsideGizmo()
{
	return allowSelectionInsideGizmo;
}
void	UnwrapMod::fnSetAllowSelectionInsideGizmo(BOOL select)
{
	allowSelectionInsideGizmo = select;
}


void	UnwrapMod::fnSetSharedColor(Point3 color)
{
	AColor c;
	c.r = color.x;
	c.g = color.y;
	c.b = color.z;
	sharedColor = c.toRGB();
	InvalidateView();
}

Point3*	UnwrapMod::fnGetSharedColor()
{
	AColor c(sharedColor);

	lColor.x = c.r;
	lColor.y = c.g;
	lColor.z = c.b;
	return &lColor;
}

BOOL	UnwrapMod::fnGetShowShared()
{
	return showShared;
}

void	UnwrapMod::fnSetShowShared(BOOL share)
{
	showShared = share;
	InvalidateView();
}

void	UnwrapMod::fnShowIcon(int id,BOOL show)
{
	if ((id > 0) && (id < 30))
		showIconList.Set(id,show);
}


Point3* UnwrapMod::fnGetBackgroundColor()
{
	AColor c(backgroundColor);

	lColor.x = c.r;
	lColor.y = c.g;
	lColor.z = c.b;
	return &lColor;
}

void UnwrapMod::fnSetBackgroundColor(Point3 color)
{
	AColor c;
	c.r = color.x;
	c.g = color.y;
	c.b = color.z;
	backgroundColor = c.toRGB();
	if (iBuf) iBuf->SetBkColor(backgroundColor);
	if (iTileBuf) iTileBuf->SetBkColor(backgroundColor);
	ColorMan()->SetColor(BACKGROUNDCOLORID,  backgroundColor);
	InvalidateView();
}

void	UnwrapMod::fnUpdateMenuBar()
{
	if ( (ip) &&(hWnd) )
	{
		IMenuBarContext* pContext = (IMenuBarContext*) GetCOREInterface()->GetMenuManager()->GetContext(kUnwrapMenuBar);
		if (pContext)
			pContext->UpdateWindowsMenu();

	}
}

BOOL	UnwrapMod::fnGetBrightCenterTile()
{
	return brightCenterTile;
}

void	UnwrapMod::fnSetBrightCenterTile(BOOL bright)
{
	brightCenterTile = bright;
	tileValid = FALSE;
	InvalidateView();
}

BOOL	UnwrapMod::fnGetBlendToBack()
{
	return blendTileToBackGround;
}

void	UnwrapMod::fnSetBlendToBack(BOOL blend)
{
	blendTileToBackGround = blend;
	tileValid = FALSE;
	InvalidateView();
}

int		UnwrapMod::fnGetTickSize()
{
	return tickSize;
}
void	UnwrapMod::fnSetTickSize(int size)
{
	tickSize = size;
	if (tickSize <1 ) tickSize = 1;
	InvalidateView();
}


//new


float	UnwrapMod::fnGetGridSize()
{
	return gridSize;
}
void	UnwrapMod::fnSetGridSize(float size)
{
	gridSize = size;
	InvalidateView();
}

BOOL	UnwrapMod::fnGetGridSnap()
{
	return gridSnap;
}
void	UnwrapMod::fnSetGridSnap(BOOL snap)
{
	gridSnap = snap;

	if (gridSnap)
		pixelSnap = FALSE;

	if (iSnap)
	{
		iSnap->SetCurFlyOff(0);
		if (gridSnap)		
			iSnap->SetCheck(TRUE);
		else iSnap->SetCheck(FALSE);
	}
}
BOOL	UnwrapMod::fnGetGridVisible()
{
	return gridVisible;

}
void	UnwrapMod::fnSetGridVisible(BOOL visible)
{
	gridVisible = visible;
	InvalidateView();
}

void	UnwrapMod::fnSetGridColor(Point3 color)
{
	AColor c;
	c.r = color.x;
	c.g = color.y;
	c.b = color.z;
	gridColor = c.toRGB();
	if (gridVisible) InvalidateView();
}

Point3*	UnwrapMod::fnGetGridColor()
{
	AColor c(gridColor);

	lColor.x = c.r;
	lColor.y = c.g;
	lColor.z = c.b;
	return &lColor;
}

float	UnwrapMod::fnGetGridStr()
{

	return gridStr * 2.0f;
}
void	UnwrapMod::fnSetGridStr(float str)
{

	str = str *0.5f;

	if (str < 0.0f) str = 0.0f;
	if (str > 0.5f) str = 0.5f;

	gridStr = str;
}


BOOL	UnwrapMod::fnGetAutoMap()
{
	return autoMap;

}
void	UnwrapMod::fnSetAutoMap(BOOL autoMap)
{
	this->autoMap = autoMap;
}




float	UnwrapMod::fnGetFlattenAngle()
{
	return flattenAngleThreshold;
}
void	UnwrapMod::fnSetFlattenAngle(float angle)
{
	if (loadDefaults)
	{
		fnLoadDefaults();
		loadDefaults = FALSE;
	}

	if (angle < 0.0f) angle = 0.0f;
	if (angle > 180.0f) angle = 180.0f;

	flattenAngleThreshold = angle;

}

float	UnwrapMod::fnGetFlattenSpacing()
{
	return flattenSpacing;
}
void	UnwrapMod::fnSetFlattenSpacing(float spacing)
{
	if (loadDefaults)
	{
		fnLoadDefaults();
		loadDefaults = FALSE;
	}

	if (spacing < 0.0f) spacing = 0.0f;
	if (spacing > 1.0f) spacing = 1.0f;

	flattenSpacing = spacing;

}

BOOL	UnwrapMod::fnGetFlattenNormalize()
{
	return flattenNormalize;
}
void	UnwrapMod::fnSetFlattenNormalize(BOOL normalize)
{
	if (loadDefaults)
	{
		fnLoadDefaults();
		loadDefaults = FALSE;
	}
	flattenNormalize = normalize;
}

BOOL	UnwrapMod::fnGetFlattenRotate()
{
	return flattenRotate;
}
void	UnwrapMod::fnSetFlattenRotate(BOOL rotate)
{
	if (loadDefaults)
	{
		fnLoadDefaults();
		loadDefaults = FALSE;
	}
	flattenRotate = rotate;
}

BOOL	UnwrapMod::fnGetFlattenFillHoles()
{
	return flattenCollapse;
}
void	UnwrapMod::fnSetFlattenFillHoles(BOOL fillHoles)
{
	if (loadDefaults)
	{
		fnLoadDefaults();
		loadDefaults = FALSE;
	}
	flattenCollapse = fillHoles;
}


BOOL	UnwrapMod::fnGetPreventFlattening()
{
	return preventFlattening;
}
void	UnwrapMod::fnSetPreventFlattening(BOOL preventFlattening)
{

	if (loadDefaults)
	{
		fnLoadDefaults();
		loadDefaults = FALSE;
	}

	if (theHold.Holding())
	{
		theHold.Put(new UnwrapSeamAttributesRestore(this));
	}

	this->preventFlattening = preventFlattening;

	if ( (ip) &&(hWnd) )
	{	
		IMenuBarContext* pContext = (IMenuBarContext*) GetCOREInterface()->GetMenuManager()->GetContext(kUnwrapMenuBar);
		if (pContext)
			pContext->UpdateWindowsMenu();
		//update UI

	}

	CheckDlgButton(hParams,IDC_DONOTREFLATTEN_CHECK,preventFlattening);


}


BOOL	UnwrapMod::fnGetEnableSoftSelection()
{
	return enableSoftSelection;
}

void	UnwrapMod::fnSetEnableSoftSelection(BOOL enable)
{
	enableSoftSelection = enable;
	RebuildDistCache();
	InvalidateView();

}


BOOL	UnwrapMod::fnGetApplyToWholeObject()
{
	return applyToWholeObject;
}

void	UnwrapMod::fnSetApplyToWholeObject(BOOL whole)
{
	applyToWholeObject = whole;

}






//5.1.05
BOOL	UnwrapMod::fnGetAutoBackground()
{
	return this->autoBackground;
}
void	UnwrapMod::fnSetAutoBackground(BOOL autoBackground)
{
	this->autoBackground = autoBackground;
}

BOOL	UnwrapMod::fnGetRelativeTypeInMode()
{
	if (absoluteTypeIn)
		return FALSE; 
	else return TRUE;
}

void	UnwrapMod::SetAbsoluteTypeInMode(BOOL absolute)
{
	absoluteTypeIn = absolute;
	typeInsValid = FALSE;
	SetupTypeins();
}

void	UnwrapMod::fnSetRelativeTypeInMode(BOOL relative)
{
	if (relative)
		absoluteTypeIn = FALSE;
	else absoluteTypeIn = TRUE;
	typeInsValid = FALSE;
	SetupTypeins();
	if (iUVWSpinAbsoluteButton)
		iUVWSpinAbsoluteButton->SetCheck(relative);
}


void	UnwrapMod::fnAddMap(Texmap *map)
{
	this->AddMaterial(map);
}

extern float AreaOfTriangle(Point3 a, Point3 b, Point3 c);
extern float AreaOfPolygon(Tab<Point3> &points);


void UnwrapMod::GetArea(BitArray *faceSelection, 
						  float &x, float &y,
						  float &width, float &height,
						  float &uvArea, float &geomArea,
						  MeshTopoData *ld)
{
	Box3 bounds;
	bounds.Init();
	uvArea = 0.0f;
	geomArea = 0.0f;

		if (ld)
		{
			for (int i = 0; i < ld->GetNumberFaces(); i++)//TVMaps.f.Count(); i++)
			{
				if ( i < faceSelection->GetSize() && !ld->GetFaceDead(i))
				{
					if ((*faceSelection)[i])
					{
						int ct = ld->GetFaceDegree(i);//TVMaps.f[i]->count;

						Tab<Point3> plistGeom;
						plistGeom.SetCount(ct);
						Tab<Point3> plistUVW;
						plistUVW.SetCount(ct);

						for (int j = 0; j < ct; j++)
						{
							int index = ld->GetFaceTVVert(i,j);//TVMaps.f[i]->t[j];
							plistUVW[j] = ld->GetTVVert(index);//TVMaps.v[index].p;
							bounds += plistUVW[j];

							index = ld->GetFaceGeomVert(i,j);//TVMaps.f[i]->v[j];
							plistGeom[j] = ld->GetGeomVert(index);//TVMaps.geomPoints[index];
						}
						if (ct == 3)
						{
							geomArea += AreaOfTriangle(plistGeom[0],plistGeom[1],plistGeom[2]);
							uvArea += AreaOfTriangle(plistUVW[0],plistUVW[1],plistUVW[2]);
						}
						else
						{
							geomArea += AreaOfPolygon(plistGeom);
							uvArea += AreaOfPolygon(plistUVW);
						}

					}
				}
			}
			x = bounds.pmin.x;
			y = bounds.pmin.y;
			width = bounds.pmax.x - bounds.pmin.x;
			height = bounds.pmax.y - bounds.pmin.y;

		}
	

}

void UnwrapMod::fnGetArea(BitArray *faceSelection, 
						  float &x, float &y,
						  float &width, float &height,
						  float &uvArea, float &geomArea)
{
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		GetArea(faceSelection,x,y,width, height, uvArea, geomArea,ld);

	}
}

void UnwrapMod::fnGetArea(BitArray *faceSelection, 
						  float &uvArea, float &geomArea, INode *node)
{
	MeshTopoData *ld = GetMeshTopoData(node);
	float x,y,width,height;
	GetArea(faceSelection,x,y,width, height, uvArea, geomArea, ld);
}

void UnwrapMod::fnGetBounds(BitArray *faceSelection, 
						  float &x, float &y,
						  float &width, float &height,
						  INode *node)
{
	MeshTopoData *ld = GetMeshTopoData(node);
	float uvArea,geomArea;
	GetArea(faceSelection,x,y,width, height, uvArea, geomArea, ld);
}

BOOL UnwrapMod::fnGetRotationsRespectAspect()
{
	return rotationsRespectAspect;
}
void UnwrapMod::fnSetRotationsRespectAspect(BOOL respect)
{
	rotationsRespectAspect = respect;
}


BOOL UnwrapMod::fnGetPeltEditSeamsMode()
{
	return peltData.GetEditSeamsMode();
}
void UnwrapMod::fnSetPeltEditSeamsMode(BOOL mode)
{

	peltData.SetEditSeamsMode(this,mode);
}


BOOL UnwrapMod::GetPeltMapMode()
{
	return peltData.GetPeltMapMode();
}
void UnwrapMod::SetPeltMapMode(BOOL mode)
{
	peltData.SetPeltMapMode(this,mode);

	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());
}

BOOL UnwrapMod::fnGetPeltPointToPointSeamsMode()
{
	return peltData.GetPointToPointSeamsMode();
}
void UnwrapMod::fnSetPeltPointToPointSeamsMode(BOOL mode)
{
	peltData.SetPointToPointSeamsMode(this, mode);
	//	peltData.SetPeltMapMode(mode);
}

void UnwrapMod::fnPeltExpandSelectionToSeams()
{
	theHold.Begin();
	HoldSelection();
	theHold.Accept(_T(GetString(IDS_PELT_EXPANDSELTOSEAM)));

	peltData.ExpandSelectionToSeams(this);
	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());
}

void UnwrapMod::fnPeltDialogResetRig()
{
	peltData.ResetRig(this);
}

void UnwrapMod::fnPeltDialogSelectRig()
{
	theHold.Begin();
	HoldSelection();
	theHold.Accept(_T(GetString(IDS_PELTDIALOG_SELECTRIG)));
	SHORT iret = GetAsyncKeyState (VK_CONTROL);
	if (iret==-32767)
		peltData.SelectRig(this,FALSE);
	else peltData.SelectRig(this,TRUE);
}

void UnwrapMod::fnPeltDialogSelectPelt()
{
	theHold.Begin();
	HoldSelection();
	theHold.Accept(_T(GetString(IDS_PELTDIALOG_SELECTPELT)));
	SHORT iret = GetAsyncKeyState (VK_CONTROL);
	if (iret==-32767)
		peltData.SelectPelt(this,FALSE);
	else peltData.SelectPelt(this,TRUE);
}

void UnwrapMod::fnPeltDialogSnapRig()
{
	theHold.Begin();
	HoldPoints();	
	theHold.Accept(_T(GetString(IDS_PELTDIALOG_SNAPRIG)));
	peltData.SnapRig(this);

}


BOOL UnwrapMod::fnGetPeltDialogStraightenSeamsMode()
{
	return peltData.peltDialog.GetStraightenMode();
}
void UnwrapMod::fnSetPeltDialogStraightenSeamsMode(BOOL mode)
{
	peltData.peltDialog.SetStraightenMode(mode);
}

void UnwrapMod::fnPeltDialogMirrorRig()
{
	theHold.Begin();
	HoldPoints();	
	theHold.Accept(_T(GetString(IDS_PELTDIALOG_MIRRORRIG)));

	peltData.MirrorRig(this);
}


void UnwrapMod::fnPeltDialogRun()
{
	theHold.Begin();
	HoldPoints();
	theHold.Put (new UnwrapPeltVertVelocityRestore (this));
	peltData.Run(this);
	theHold.Accept(_T(GetString(IDS_PELTDIALOG_RUN)));

}
void UnwrapMod::fnPeltDialogRelax1()
{
	theHold.Begin();
	HoldPoints();
	theHold.Put (new UnwrapPeltVertVelocityRestore (this));
	peltData.RunRelax(this,0);
	theHold.Accept(_T(GetString(IDS_PELTDIALOG_RELAX1)));

}
void UnwrapMod::fnPeltDialogRelax2()
{
	theHold.Begin();
	HoldPoints();
	theHold.Put (new UnwrapPeltVertVelocityRestore (this));
	peltData.RunRelax(this,1);
	theHold.Accept(_T(GetString(IDS_PELTDIALOG_RELAX2)));

}

BitArray* UnwrapMod::fnGetSeamSelection(INode *node)
{

		MeshTopoData *ld = GetMeshTopoData(node);
		if (ld)
		{
			return &ld->mSeamEdges;
		}
	return NULL;

}

BitArray* UnwrapMod::fnGetSeamSelection()
{
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			return &ld->mSeamEdges;
		}
	}
	return NULL;
	
}

void UnwrapMod::fnSetSeamSelection(BitArray *sel, INode *node)
{


		MeshTopoData *ld = GetMeshTopoData(node);
		if (ld)
		{
			ld->mSeamEdges.ClearAll();
			for (int i = 0 ; i < (*sel).GetSize(); i++)
			{
				if ((i < ld->mSeamEdges.GetSize()) && ((*sel)[i]))
					ld->mSeamEdges.Set(i,TRUE);
			}
			NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
			if (ip) ip->RedrawViews(ip->GetTime());
		}

}

void UnwrapMod::fnSetSeamSelection(BitArray *sel)
{

	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			ld->mSeamEdges.ClearAll();
			for (int i = 0 ; i < (*sel).GetSize(); i++)
			{
				if ((i < ld->mSeamEdges.GetSize()) && ((*sel)[i]))
					ld->mSeamEdges.Set(i,TRUE);
			}
			NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
			if (ip) ip->RedrawViews(ip->GetTime());
		}
	}

}


void UnwrapMod::fnPeltDialogStraighten(int a, int b)
{
	peltData.StraightenSeam(this,a,b);

	InvalidateView();
	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());
}

int UnwrapMod::fnGetPeltDialogFrames()
{
	return peltData.GetFrames();
}
void UnwrapMod::fnSetPeltDialogFrames(int frames)
{
	peltData.SetFrames(frames);
	peltData.peltDialog.UpdateSpinner(IDC_PELT_FRAMESSPIN, frames);

}

int UnwrapMod::fnGetPeltDialogSamples()
{
	return peltData.GetSamples();
}
void UnwrapMod::fnSetPeltDialogSamples(int samples)
{
	peltData.SetSamples(samples);
	peltData.peltDialog.UpdateSpinner(IDC_PELT_SAMPLESSPIN, samples);
}

float UnwrapMod::fnGetPeltDialogRigStrength()
{
	return peltData.GetRigStrength();
}
void UnwrapMod::fnSetPeltDialogRigStrength(float strength)
{
	peltData.SetRigStrength(strength);
	peltData.peltDialog.UpdateSpinner(IDC_PELT_RIGSTRENGTHSPIN, strength);
}

float UnwrapMod::fnGetPeltDialogStiffness()
{
	return peltData.GetStiffness();
}
void UnwrapMod::fnSetPeltDialogStiffness(float stiffness)
{
	peltData.SetStiffness(stiffness);
	peltData.peltDialog.UpdateSpinner(IDC_PELT_STIFFNESSSPIN, stiffness);
}

float UnwrapMod::fnGetPeltDialogDampening()
{
	return peltData.GetDampening();
}
void UnwrapMod::fnSetPeltDialogDampening(float dampening)
{
	peltData.SetDampening(dampening);
	peltData.peltDialog.UpdateSpinner(IDC_PELT_DAMPENINGSPIN, dampening);
}

float UnwrapMod::fnGetPeltDialogDecay()
{
	return peltData.GetDecay();
}
void UnwrapMod::fnSetPeltDialogDecay(float decay)
{
	peltData.SetDecay(decay);
	peltData.peltDialog.UpdateSpinner(IDC_PELT_DECAYSPIN, decay);
}

float UnwrapMod::fnGetPeltDialogMirrorAxis()
{
	return peltData.GetMirrorAngle();
}
void UnwrapMod::fnSetPeltDialogMirrorAxis(float axis)
{
	peltData.SetMirrorAngle(axis*PI/180.0f);
	peltData.peltDialog.UpdateSpinner(IDC_PELT_MIRRORAXISSPIN, axis);
	InvalidateView();

}

int UnwrapMod::fnGetMapMode()
{
	return mapMapMode;
}
void UnwrapMod::fnSetMapMode(int mode)
{
	//turn off the old mode

	if (mapMapMode == PELTMAP)
	{
		SetPeltMapMode(FALSE);
	}

	if (mapMapMode == SPLINEMAP)
	{
		peltData.EnablePeltButtons(hMapParams, TRUE);
		fnSplineMap_EndMapMode();
	}


	ICustButton *iButton = GetICustButton(GetDlgItem(hMapParams, MapButtons[mapMapMode]));
	if (iButton)
	{
		iButton->SetCheck(FALSE);
		ReleaseICustButton(iButton);
	}


	if ((mode == mapMapMode) || (mode == NOMAP))
	{
		mapMapMode = NOMAP;
		EnableAlignButtons(FALSE);
		if (ip->GetSubObjectLevel() == 3)
			ip->SetSubObjectLevel(3,TRUE);
		return;
	}
	else
	{

		fnSetTweakMode(FALSE);
		mapMapMode = mode;
		//press the buttons
		iButton = GetICustButton(GetDlgItem(hMapParams, MapButtons[mapMapMode]));
		if (iButton)
		{
			iButton->SetCheck(TRUE);
			ReleaseICustButton(iButton);
		}

		if (mapMapMode == PELTMAP)
			SetPeltMapMode(TRUE);
		if (mapMapMode == SPLINEMAP)
			fnSplineMap_StartMapMode();
			
		if ((fnGetMapMode() == PLANARMAP) || (fnGetMapMode() == CYLINDRICALMAP) || (fnGetMapMode() == SPHERICALMAP) || (fnGetMapMode() == BOXMAP))
			ApplyGizmo();

		if (mapMapMode == SPLINEMAP)
		{
			ip->SetSubObjectLevel(3,TRUE);
			EnableAlignButtons(FALSE);
			peltData.EnablePeltButtons(hMapParams, FALSE);
			peltData.SetPointToPointSeamsMode(this,FALSE);
			peltData.SetEditSeamsMode(this,FALSE);

		}
		else if (mapMapMode != PELTMAP) 
		{
			ip->SetSubObjectLevel(3,TRUE);
			EnableAlignButtons(TRUE);
		}
	}

}
void UnwrapMod::fnQMap()
{
	ClearAFlag(A_HELD);
	ApplyQMap();
}

void UnwrapMod::ApplyQMap()
{
	int holdMap = mapMapMode;
	mapMapMode = PLANARMAP;
	TimeValue t = GetCOREInterface()->GetTime();
	HoldPointsAndFaces();

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		Matrix3 tm(1);
		Matrix3 gizmoTM = mMapGizmoTM;
		if (!fnGetNormalizeMap())
		{
			for (int i = 0; i < 3; i++)
			{
				Point3 vec = gizmoTM.GetRow(i);
				vec = Normalize(vec);
				gizmoTM.SetRow(i,vec);
			}
		}
		tm = mMeshTopoData.GetNodeTM(t,ldID)* Inverse(gizmoTM);
		MeshTopoData *ld = mMeshTopoData[ldID];
		ld->ApplyMap(PLANARMAP, fnGetNormalizeMap(), tm, this);		
	}
	mapMapMode = holdMap;
	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	if (ip) 
		ip->RedrawViews(ip->GetTime());
	InvalidateView();
}



void UnwrapMod::fnSetGizmoTM(Matrix3 tm)
{
	SetXFormPacket pckt(tm);
	TimeValue t = GetCOREInterface()->GetTime();
	tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
	if ((fnGetMapMode() == PLANARMAP) || (fnGetMapMode() == CYLINDRICALMAP) || (fnGetMapMode() == SPHERICALMAP) || (fnGetMapMode() == BOXMAP))
		ApplyGizmo();
}

Matrix3* UnwrapMod::fnGetGizmoTM()
{
	TimeValue t = GetCOREInterface()->GetTime();

	mGizmoTM.IdentityMatrix();
	tmControl->GetValue(t,&mGizmoTM,FOREVER,CTRL_RELATIVE);
	return &mGizmoTM;
}


void UnwrapMod::fnSetNormalizeMap(BOOL normalize)
{
	if (theHold.Holding())
	{
		theHold.Put(new UnwrapMapAttributesRestore(this));
	}
	normalizeMap = normalize;
	CheckDlgButton(hMapParams,IDC_NORMALIZEMAP_CHECK2,normalizeMap);

	if ((fnGetMapMode() == PLANARMAP) || (fnGetMapMode() == CYLINDRICALMAP) || (fnGetMapMode() == SPHERICALMAP) || (fnGetMapMode() == BOXMAP))
	{
		theHold.Begin();
		ApplyGizmo();

		theHold.Accept(GetString(IDS_MAPPING_ALIGN));

		NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
		if (ip) ip->RedrawViews(ip->GetTime());
		InvalidateView();


	}


}
BOOL UnwrapMod::fnGetNormalizeMap()
{
	return normalizeMap;
}


void UnwrapMod::fnSetShowEdgeDistortion(BOOL show)
{
	showEdgeDistortion = show;
	if (show)
		BuildEdgeDistortionData();
	InvalidateView();
	NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

}
BOOL UnwrapMod::fnGetShowEdgeDistortion()
{
	return showEdgeDistortion;
}

void UnwrapMod::fnSetLockSpringEdges(BOOL lock)
{
	lockSpringEdges = lock;
}
BOOL UnwrapMod::fnGetLockSpringEdges()
{
	return lockSpringEdges;
}


void UnwrapMod::fnSetEdgeDistortionScale(float scale)
{
	edgeDistortionScale = scale;
	if (edgeDistortionScale <= 0.0f) edgeDistortionScale = 0.0001f;
	InvalidateView();
}
float UnwrapMod::fnGetEdgeDistortionScale()
{
	return edgeDistortionScale;
}

void UnwrapMod::fnSetShowCounter(BOOL show)
{
	showCounter = show;
	InvalidateView();
}
BOOL UnwrapMod::fnGetShowCounter()
{
	return showCounter;
}


void UnwrapMod::fnSetShowMapSeams(BOOL show)
{
	fnSetViewportOpenEdges(show);
	CheckDlgButton(hParams,IDC_SHOWMAPSEAMS_CHECK,show);

}
BOOL UnwrapMod::fnGetShowMapSeams()
{
	return fnGetViewportOpenEdges();
}

BOOL UnwrapMod::fnIsMesh()
{
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];		
		if (ld)
		{
			if (ld->GetMesh())
				return TRUE;
			else return FALSE;
		}
	}
	return TRUE;

}


int	UnwrapMod::fnGetSG()
{
	if (iSG)
		return iSG->GetIVal();
	return 0;
}

void UnwrapMod::fnSetSG(int sg)
{
	if (iSG)
		iSG->SetValue(sg,FALSE);
}


int		UnwrapMod::fnGetMatIDSelect()
{
	if (iSG)
		return iMatID->GetIVal();
	return 0;
}

void	UnwrapMod::fnSetMatIDSelect(int matID)
{
	if (iMatID)
		iMatID->SetValue(matID,FALSE);

}


BOOL UnwrapMod::GetIsMeshUp()
{
	//first see if we have a selection if not use the whole mesh
	BOOL hasSelection = FALSE;
	for (int i = 0; i < mMeshTopoData.Count(); i++)
	{
		MeshTopoData *ld = mMeshTopoData[i];
		if (ld->GetFaceSelection().NumberSet())
			hasSelection = TRUE;
	}
	TimeValue t = GetCOREInterface()->GetTime();

	float sideArea = 0.0f;
	float upArea = 0.0f;
	//loop through our meshes
	for (int i = 0; i < mMeshTopoData.Count(); i++)
	{
		
		MeshTopoData *ld = mMeshTopoData[i];
		INode *node = GetMeshTopoDataNode(i);
		Matrix3 tm = node->GetObjectTM(t);
		BitArray upFaces;
		BitArray sideFaces;

		int numFaces = ld->GetNumberFaces();
		upFaces.SetSize(numFaces);
		upFaces.ClearAll();
		sideFaces.SetSize(numFaces);
		sideFaces.ClearAll();
		BitArray faceSel = ld->GetFaceSelection();
		if (!hasSelection)
			faceSel.SetAll();

		//loop through our faces
		for (int j = 0; j < numFaces; j++)
		{
			if (faceSel[j])
			{
				Point3 norm = ld->GetGeomFaceNormal(j);
				norm = VectorTransform(tm,norm);
				//if the vector is pointing up or down add it to our up list
				if ((norm.z > 0.5) || (norm.z < -0.5f))			
				{
					upFaces.Set(j,TRUE);
				}
				//otherwise add it to our sideways list
				else
				{
					sideFaces.Set(j,TRUE);
				}
			}
		}

		float x,y,width,height;
		float uvArea,geomArea;
		geomArea = 0.0f;
		uvArea = 0.0f;
		x = 0.0f;
		y = 0.0f;
		width = 0.0f;
		height = 0.0f;
		//compute our up area and store it off
		GetArea(&upFaces,x,y,width, height, uvArea, geomArea, ld);
		upArea += geomArea;
		//compute our side area and store it off
		geomArea = 0.0f;
		uvArea = 0.0f;
		GetArea(&sideFaces,x,y,width, height, uvArea, geomArea, ld);
		sideArea += geomArea;
	}

	if (sideArea > upArea)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}

}

BOOL	UnwrapMod::fnGetTweakMode()
{
	Interface *ip = GetCOREInterface();
	CommandMode *mode = ip->GetCommandMode();

	if (mode == tweakMode)
		return TRUE;
	else
		return FALSE;
}
void	UnwrapMod::fnSetTweakMode(BOOL mode)
{
	Interface *ip = GetCOREInterface();
	CommandMode *tmode = ip->GetCommandMode();
	if (mode)
	{
		if (tmode != tweakMode)
		{
			fnSetMapMode(NOMAP);
			fnSetPeltEditSeamsMode(FALSE);
			ip->SetCommandMode(tweakMode);
		}
	}
	else
	{
		ip->SetStdCommandMode  ( CID_OBJSELECT ) ;
	}
}


void	UnwrapMod::fnUVLoop(int mode)
{
//check if vert or face if so convert to edge
	
//save the old edge
	//loop
	BOOL holding = theHold.Holding();
	if (!holding)
		theHold.Begin();
	HoldSelection();
	if (!holding)
		theHold.Accept(GetString(IDS_DS_SELECT));

	
	theHold.Suspend();
	if (fnGetTVSubMode() == TVVERTMODE)
	{
		for (int i = 0; i < GetMeshTopoDataCount(); i++)
		{
			MeshTopoData *ld = GetMeshTopoData(i);
			BitArray esel;
			ld->GetEdgeSelFromVert(esel,FALSE);
			ld->SetTVEdgeSelection(esel);
		}
		
	}
	else if (fnGetTVSubMode() == TVFACEMODE)
	{
		for (int i = 0; i < GetMeshTopoDataCount(); i++)
		{
			MeshTopoData *ld = GetMeshTopoData(i);
			ld->ClearTVEdgeSelection();
			BitArray fsel = ld->GetFaceSelection();
			BitArray vsel;
			vsel.SetSize(ld->GetNumberTVVerts());
			vsel.ClearAll();
			BitArray esel;
			esel.SetSize(ld->GetNumberTVEdges());
			esel.ClearAll();

			BitArray skipsel;
			skipsel.SetSize(ld->GetNumberTVEdges());
			skipsel.ClearAll();


			for (int i = 0; i < ld->GetNumberTVEdges(); i++)
			{
				if (ld->GetTVEdgeHidden(i))
				{
					skipsel.Set(i,TRUE);
				}
				else
				{
					int ct = ld->GetTVEdgeNumberTVFaces(i);
					bool sel = false;
					bool unsel = false;
					for (int j = 0; j < ct; j++)
					{
						int fid = ld->GetTVEdgeConnectedTVFace(i,j);
						if (fsel[fid])
							sel = true;
						if (!fsel[fid])
							unsel = true;
					}
					if (sel && !unsel)
					{
						skipsel.Set(i,TRUE);
						int tid1 = ld->GetTVEdgeVert(i,0);
						int tid2 = ld->GetTVEdgeVert(i,1);
						vsel.Set(tid1,TRUE);
						vsel.Set(tid2,TRUE);
					}
					else if (sel && unsel)
						esel.Set(i,TRUE);
				}
			}
			for (int i = 0; i < ld->GetNumberTVEdges(); i++)
			{
				if (esel[i] && !skipsel[i])
				{
					int tid1 = ld->GetTVEdgeVert(i,0);
					int tid2 = ld->GetTVEdgeVert(i,1);
					if (!vsel[tid1] && !vsel[tid2])
					{
						esel.Clear(i);
					}
				}
			}
			ld->SetTVEdgeSelection(esel);
		}
	}

	if (mode == 1)
		GrowUVLoop(TRUE);
	else if (mode == -1)
		ShrinkUVLoop();
	else
	{
		SelectUVEdge(TRUE);
	}

	if (fnGetTVSubMode() == TVVERTMODE)
	{
		for (int i = 0; i < GetMeshTopoDataCount(); i++)
		{
			MeshTopoData *ld = GetMeshTopoData(i);
			BitArray vsel;
			ld->GetVertSelFromEdge(vsel);
			ld->SetTVVertSelection(vsel);
		}

	}
	else if (fnGetTVSubMode() == TVFACEMODE)
	{
		theHold.Suspend();
		fnEdgeToFaceSelect();
		theHold.Resume();
		for (int i = 0; i < GetMeshTopoDataCount(); i++)
		{
			MeshTopoData *ld = GetMeshTopoData(i);
			ld->ClearTVEdgeSelection();
			BitArray fsel = ld->GetFaceSelection();
			BitArray vsel;
			vsel.SetSize(ld->GetNumberTVVerts());
			vsel.ClearAll();
			BitArray esel;
			esel.SetSize(ld->GetNumberTVEdges());
			esel.ClearAll();
			for (int i = 0; i < ld->GetNumberTVEdges(); i++)
			{
				if (ld->GetTVEdgeHidden(i))
				{	
					int ct = ld->GetTVEdgeNumberTVFaces(i);
					bool sel = false;
					bool unsel = false;
					for (int j = 0; j < ct; j++)
					{
						int fid = ld->GetTVEdgeConnectedTVFace(i,j);
						if (fsel[fid])
							sel = true;
						if (!fsel[fid])
							unsel = true;
					}
					if (sel && unsel)
					{
						for (int j = 0; j < ct; j++)
						{
							int fid = ld->GetTVEdgeConnectedTVFace(i,j);
							ld->SetFaceSelected(fid,FALSE);
						}

					}
				}
			}

		}

	}


	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();
	InvalidateView();			

	theHold.Resume();
//restore the selections
}
void	UnwrapMod::fnUVRing(int mode)
{
	BOOL holding = theHold.Holding();
	if (!holding)
		theHold.Begin();
	HoldSelection();
	if (!holding)
		theHold.Accept(GetString(IDS_DS_SELECT));

	theHold.Suspend();

	if (fnGetTVSubMode() == TVVERTMODE)
	{
		for (int i = 0; i < GetMeshTopoDataCount(); i++)
		{
			MeshTopoData *ld = GetMeshTopoData(i);
			BitArray esel;
			ld->GetEdgeSelFromVert(esel,FALSE);
			ld->SetTVEdgeSelection(esel);
		}

	}
	else if (fnGetTVSubMode() == TVFACEMODE)
	{
		for (int i = 0; i < GetMeshTopoDataCount(); i++)
		{
			MeshTopoData *ld = GetMeshTopoData(i);
			ld->ClearTVEdgeSelection();
			BitArray fsel = ld->GetFaceSelection();
			BitArray vsel;
			vsel.SetSize(ld->GetNumberTVVerts());
			vsel.ClearAll();
			BitArray esel;
			esel.SetSize(ld->GetNumberTVEdges());
			esel.ClearAll();
			for (int i = 0; i < ld->GetNumberTVEdges(); i++)
			{
				if (!ld->GetTVEdgeHidden(i))
				{
					int ct = ld->GetTVEdgeNumberTVFaces(i);				
					for (int j = 0; j < ct; j++)
					{
						int fid = ld->GetTVEdgeConnectedTVFace(i,j);
						if (fsel[fid])
							esel.Set(i,TRUE);
					}
				}
			}
			//select only ring edges
			Tab<int> edgesAtVertex;
			edgesAtVertex.SetCount(ld->GetNumberTVVerts());
			for (int i = 0; i < ld->GetNumberTVVerts(); i++)
				edgesAtVertex[i] = 0;
			for (int i =0; i < ld->GetNumberTVEdges(); i++)
			{
				if (esel[i])
				{
					int a = ld->GetTVEdgeVert(i, 0);
					int b = ld->GetTVEdgeVert(i, 1);
					edgesAtVertex[a] += 1;
					edgesAtVertex[b] += 1;
				}
			}

			//remove our interior edges
			for (int i =0; i < ld->GetNumberTVEdges(); i++)
			{

				if (esel[i])
				{
					int ct = ld->GetTVEdgeNumberTVFaces(i);
					if ( ct == 2 )
					{
						int a = ld->GetTVEdgeConnectedTVFace(i, 0);
						int b = ld->GetTVEdgeConnectedTVFace(i, 1);
						if (fsel[a] && fsel[b])
						{
							esel.Clear(i);
						}
					}
				}
			}

			BitArray ringVerts;
			ringVerts.SetSize(ld->GetNumberTVVerts());
			ringVerts.ClearAll();
			BitArray ringEdges;
			ringEdges.SetSize(ld->GetNumberTVEdges());
			ringEdges.ClearAll();
			for (int i =0; i < ld->GetNumberTVEdges(); i++)
			{
				if (esel[i])
				{
					int a = ld->GetTVEdgeVert(i, 0);
					int b = ld->GetTVEdgeVert(i, 1);
					if ( (edgesAtVertex[a] == 3) || (edgesAtVertex[b] == 3))
					{
						ringVerts.Set(a,TRUE);
						ringVerts.Set(b,TRUE);
						ringEdges.Set(i,TRUE);
					}
				}
			}

			//remove loop edges between rings
			for (int i =0; i < ld->GetNumberTVEdges(); i++)
			{
				if (esel[i] && (!ringEdges[i]))
				{
					int a = ld->GetTVEdgeVert(i, 0);
					int b = ld->GetTVEdgeVert(i, 1);
					if (ringVerts[a] && ringVerts[b])
					{
						esel.Clear(i);
					}
				}
			}

			ld->SetTVEdgeSelection(esel);
		}
	}

  	if (mode == 1)
		GrowUVRing(false);
	else if (mode == -1)
		ShrinkUVRing();
	else
	{
		GrowUVRing(true);
	}

	if (fnGetTVSubMode() == TVVERTMODE)
	{
		for (int i = 0; i < GetMeshTopoDataCount(); i++)
		{
			MeshTopoData *ld = GetMeshTopoData(i);
			BitArray vsel;
			ld->GetVertSelFromEdge(vsel);
			ld->SetTVVertSelection(vsel);
		}

	}
	else if (fnGetTVSubMode() == TVFACEMODE)
	{
		for (int i = 0; i < GetMeshTopoDataCount(); i++)
		{
			MeshTopoData *ld = GetMeshTopoData(i);
			BitArray vsel;
			ld->GetVertSelFromEdge(vsel);
			ld->SetTVVertSelection(vsel);
			BitArray fsel;
			ld->GetFaceSelFromVert(fsel,FALSE);
			
			//clean up faces that have a hidden edge with on side selected and the other not
			for (int i = 0; i < ld->GetNumberTVEdges(); i++)
			{
				if (ld->GetTVEdgeHidden(i))
				{				
					int ct = ld->GetTVEdgeNumberTVFaces(i);
					bool sel = false;
					bool unsel = false;
					for (int j = 0; j < ct; j++)
					{
						int fid = ld->GetTVEdgeConnectedTVFace(i,j);
						if (fsel[fid])
							sel = true;
						else if (!fsel[fid])
							unsel = true;
					}
					if (sel && unsel)
					{
						for (int j = 0; j < ct; j++)
						{
							int fid = ld->GetTVEdgeConnectedTVFace(i,j);
							fsel.Set(fid,FALSE);
						}
					}
				}
			}
			ld->SetFaceSelection(fsel);
		}
	}

	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();
	InvalidateView();	

	theHold.Resume();

}



void	UnwrapMod::SpaceOrAlign(BOOL align, BOOL horizontal)
{
//if face mode bail
	TimeValue t = GetCOREInterface()->GetTime();
	if (fnGetTVSubMode() == TVFACEMODE)
	{
		return;
	}

//if vertex mode convert vertex selection to edge selection
	if (fnGetTVSubMode() == TVVERTMODE)
	{
		for (int i = 0; i < GetMeshTopoDataCount(); i++)
		{
			MeshTopoData *ld = GetMeshTopoData(i);
			ld->ClearTVEdgeSelection();
			BitArray esel;
			ld->GetEdgeSelFromVert(esel,FALSE);
			ld->SetTVEdgeSelection(esel);
		}
	}

//mark all selected vertices
	else if (fnGetTVSubMode() == TVEDGEMODE)
	{
		for (int i = 0; i < GetMeshTopoDataCount(); i++)
		{
			MeshTopoData *ld = GetMeshTopoData(i);
			ld->ClearTVVertSelection();
			BitArray vsel;
			ld->GetVertSelFromEdge(vsel);
			ld->SetTVVertSelection(vsel);
		}
	}

	for (int i = 0; i < GetMeshTopoDataCount(); i++)
	{
		MeshTopoData *ld = GetMeshTopoData(i);
		Tab<int> numberOfConnectedEdges;
		numberOfConnectedEdges.SetCount(ld->GetNumberTVVerts());
		for (int j = 0; j < numberOfConnectedEdges.Count(); j++)
			numberOfConnectedEdges[j] = 0;

		//build number of edges at each vert
		DWORDTab *vertConnectedToEdge;
		vertConnectedToEdge = new DWORDTab[ld->GetNumberTVVerts()];

		BitArray esel;
		esel = ld->GetTVEdgeSelection();

		for (DWORD j = 0; j < ld->GetNumberTVEdges(); j++)
		{
			if (esel[j])
			{			
				DWORD tid0 = ld->GetTVEdgeVert(j,0);
				DWORD tid1 = ld->GetTVEdgeVert(j,1);
				vertConnectedToEdge[tid0].Append(1,&j,10);
				vertConnectedToEdge[tid1].Append(1,&j,10);
			}
		}
		Tab<DWORD> processEdges;
		BitArray processedVerts;		
		processedVerts.SetSize(ld->GetNumberTVVerts());

		BitArray skipVerts;		
		skipVerts.SetSize(ld->GetNumberTVVerts());
		skipVerts.ClearAll();


		Tab<DWORD> currentVerts;
		Tab<DWORD> newVerts;

		for (DWORD j = 0; j < ld->GetNumberTVVerts(); j++)
		{

			if (vertConnectedToEdge[j].Count() > 0 && !skipVerts[j])
			{
//gather up all the connected verts
				processedVerts.ClearAll();
				processEdges.SetCount(0,FALSE);
				
				bool done = false;
				currentVerts.SetCount(0,FALSE);
				newVerts.SetCount(0,FALSE);;

				currentVerts.Append(1,&j,1000);
				skipVerts.Set(currentVerts[0]);
				while (!done)
				{
					BOOL hit = false;
					newVerts.SetCount(0,FALSE);
					for (int m = 0; m < currentVerts.Count(); m++)
					{
						for (int k = 0; k < vertConnectedToEdge[currentVerts[m]].Count(); k++)
					{
							DWORD eindex =  vertConnectedToEdge[currentVerts[m]][k];
						if (esel[eindex])
						{
							processEdges.Append(1,&eindex);

							DWORD tid0 = ld->GetTVEdgeVert(eindex,0);
							DWORD tid1 = ld->GetTVEdgeVert(eindex,1);

								DWORD nextVert = -1;
								if (tid0 != currentVerts[m])
									nextVert = tid0;
								else if (tid1 != currentVerts[m])
									nextVert = tid1;
								if (!skipVerts[nextVert])
									newVerts.Append(1,&nextVert);
							esel.Clear(eindex);
								skipVerts.Set(currentVerts[m]);								
							hit = true;
						}
					}
					}
					currentVerts = newVerts;
					if (!hit)
						done = true;
				}
				if (processEdges.Count() > 0)
				{
					int i1, i2;
					GetUVWIndices(i1,i2);

					if (align)
					{					
						processedVerts.ClearAll();
						//compute the center
						float center = 0.0f;
						int centerCT = 0;

						if (horizontal)
							i1 = i2;
						for (int k = 0; k < processEdges.Count(); k++)
						{
							for (int m = 0; m < 2; m++)
							{
								int id = ld->GetTVEdgeVert(processEdges[k],m);
								if (!processedVerts[id])
								{
									Point3 p = ld->GetTVVert(id);
									center += p[i1];
									centerCT++;
									processedVerts.Set(id,TRUE);
								}
							}
						}

						if (centerCT)
						{
							center = center/(float)centerCT;
							processedVerts.ClearAll();
							for (int k = 0; k < processEdges.Count(); k++)
							{
								for (int m = 0; m < 2; m++)
								{
									int id = ld->GetTVEdgeVert(processEdges[k],m);
									if (!processedVerts[id])
									{
										Point3 p = ld->GetTVVert(id);
										p[i1] = center;
										ld->SetTVVert(t,id,p,this);
									}
								}
							}
						}
					}
//do space
					else
					{
						processedVerts.ClearAll();
						//compute the center
						float l = 0.0f;
						int lCT = 0;

						if (!horizontal)
							i1 = i2;


						if (processEdges.Count())
						{
							
							processedVerts.ClearAll();
							//find the left most vert/edge
							int leftMostEdge = -1;
							int leftMostVert = -1;
							float leftMostValue = 0.0f;
							Box3 b;
							b.Init();
							for (int k = 0; k < processEdges.Count(); k++)
							{
								for (int m = 0; m < 2; m++)
								{
									int id = ld->GetTVEdgeVert(processEdges[k],m);
									Point3 p = ld->GetTVVert(id);
									b += p;
									if ((p[i1] < leftMostValue) || (leftMostEdge==-1))
									{
										leftMostVert = id;
										leftMostValue = p[i1];
										leftMostEdge = k;
									}
								}
							}
							int tempID = processEdges[0];
							processEdges[0] = processEdges[leftMostEdge];
							processEdges[leftMostEdge] = tempID;
							l = (b.Max()-b.Min())[i1]/(float)processEdges.Count();
							

							//start with that edge and work our way to the end
							for (int k = 0; k < processEdges.Count()-2; k++)
							{
								int anchor1 = ld->GetTVEdgeVert(processEdges[k],0);
								int anchor2 = ld->GetTVEdgeVert(processEdges[k],1);
								for (int m = k+1 ; m < processEdges.Count()-1; m++)
								{
									for (int n = 0; n < 2; n++)
									{
										int edgeID = processEdges[m];
										int testID = ld->GetTVEdgeVert(edgeID,n);
										if ((testID == anchor1) || (testID == anchor2))
										{
											int tempID = processEdges[k+1];
											processEdges[k+1] = processEdges[m];
											processEdges[m] = tempID;
											m = processEdges.Count();
											n = 2;
										}
									}
								}
							}
							//process the edges
							for (int k = 0; k < processEdges.Count(); k++)
							{
								int id0 = ld->GetTVEdgeVert(processEdges[k],0);
								int id1 = ld->GetTVEdgeVert(processEdges[k],1);
								int moveVert = id1;
								int baseVert  = id0;
								if (id1 == leftMostVert)
								{
									moveVert = id0;
									baseVert = id1;
								}

								Point3 pm = ld->GetTVVert(moveVert);
								Point3 pb = ld->GetTVVert(baseVert);
								pm[i1] =  pb[i1] + l;
								ld->SetTVVert(t,moveVert,pm,this);
								leftMostVert = moveVert;
							}
						}
					}
					
				}
			}
		}

		delete [] vertConnectedToEdge;
	}

	
}

void	UnwrapMod::fnAlign(BOOL horizontal)
{
	BOOL holding = theHold.Holding();
	if (!holding)
		theHold.Begin();
	HoldPoints();
	if (!holding)
		theHold.Accept(GetString(IDS_DS_MOVE));

	theHold.Suspend();
	SpaceOrAlign(TRUE,horizontal);
	

	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();
	InvalidateView();

	theHold.Resume();
}

void	UnwrapMod::fnSpace(BOOL horizontal)
{
	BOOL holding = theHold.Holding();
	if (!holding)	
		theHold.Begin();
	HoldPoints();
	if (!holding)
		theHold.Accept(GetString(IDS_DS_MOVE));

	theHold.Suspend();
	SpaceOrAlign(FALSE,horizontal);
	

	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();
	InvalidateView();
	theHold.Resume();
}