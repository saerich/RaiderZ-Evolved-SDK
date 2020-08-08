#include "avg_maxver.h"  // Defines MAGMA

/////////////////////////////
// avg_dlx
/////////////////////////////

def_visible_primitive_debug_ok	( getSegLengths,		"getSegLengths");
def_visible_primitive_debug_ok	( subdivideSegment,		"subdivideSegment");
def_visible_primitive_debug_ok	( interpCurve3D,		"interpCurve3D");
def_visible_primitive_debug_ok	( tangentCurve3D,		"tangentCurve3D");
def_visible_primitive_debug_ok	( setMaterialID,		"setMaterialID");
def_visible_primitive_debug_ok	( getMaterialID,		"getMaterialID");

def_visible_primitive_debug_ok	( affectRegionVal,		"affectRegionVal");

def_visible_primitive_debug_ok	( getTransformAxis,		"getTransformAxis");
def_mapped_primitive_debug_ok	( invalidateTM,			"invalidateTM");
def_mapped_primitive_debug_ok	( invalidateTreeTM,		"invalidateTreeTM");
def_mapped_primitive_debug_ok	( invalidateWS,			"invalidateWS");
def_visible_primitive_debug_ok	( snapshotAsMesh,		"snapshotAsMesh");
def_visible_primitive_debug_ok	( getInheritanceFlags,	"getInheritanceFlags");
def_mapped_primitive			( setInheritanceFlags,	"setInheritanceFlags");
def_visible_primitive_debug_ok	( getTransformLockFlags,"getTransformLockFlags");
def_mapped_primitive			( setTransformLockFlags,"setTransformLockFlags");

// def_visible_primitive		( isDebugBuild,			"isDebugBuild");  // remove before ship
def_visible_primitive			( setFocus,				"setFocus");  

def_struct_primitive			( SoundScrub,			WAVsound,	"scrub");

def_visible_primitive_debug_ok	( getPropertyController,"getPropertyController");
def_visible_primitive			( setPropertyController,"setPropertyController");
def_visible_primitive_debug_ok	( isPropertyAnimatable, "isPropertyAnimatable");

def_visible_primitive			( replaceInstances,		"replaceInstances");

def_struct_primitive_debug_ok	( internetCheckConnection,	internet,	"checkConnection");

def_visible_primitive_debug_ok	( getNumSubMtls,		"getNumSubMtls");
def_visible_primitive_debug_ok	( getSubMtl,			"getSubMtl");
def_visible_primitive			( setSubMtl,			"setSubMtl");
def_visible_primitive_debug_ok	( getSubMtlSlotName,	"getSubMtlSlotName");
def_visible_primitive_debug_ok	( getNumSubTexmaps,		"getNumSubTexmaps");
def_visible_primitive_debug_ok	( getSubTexmap,			"getSubTexmap");
def_visible_primitive			( setSubTexmap,			"setSubTexmap");
def_visible_primitive_debug_ok	( getSubTexmapSlotName,	"getSubTexmapSlotName");


#ifndef WEBVERSION
def_struct_primitive_debug_ok	( getPinNode,			ik,			"getPinNode");
def_struct_primitive			( setPinNode,			ik,			"setPinNode");
def_struct_primitive_debug_ok	( getPrecedence,		ik,			"getPrecedence");
def_struct_primitive			( setPrecedence,		ik,			"setPrecedence");

def_struct_primitive_debug_ok	( getIsTerminator,		ik,			"getIsTerminator");
def_struct_primitive			( setIsTerminator,		ik,			"setIsTerminator");
def_struct_primitive_debug_ok	( getBindPos,			ik,			"getBindPos");
def_struct_primitive			( setBindPos,			ik,			"setBindPos");
def_struct_primitive_debug_ok	( getBindOrient,		ik,			"getBindOrient");
def_struct_primitive			( setBindOrient,		ik,			"setBindOrient");
#endif //WEBVERSION

def_visible_primitive_debug_ok	( getMTLMEditFlags,		"getMTLMEditFlags");
def_visible_primitive			( setMTLMEditFlags,		"setMTLMEditFlags");
def_visible_primitive_debug_ok	( getMTLMeditObjType,	"getMTLMeditObjType");
def_visible_primitive			( setMTLMeditObjType,	"setMTLMeditObjType");
def_visible_primitive_debug_ok	( getMTLMeditTiling,	"getMTLMeditTiling");
def_visible_primitive			( setMTLMeditTiling,	"setMTLMeditTiling");
def_visible_primitive			( updateMTLInMedit,		"updateMTLInMedit");

def_visible_primitive_debug_ok	( qsort,				"qsort");
def_visible_primitive_debug_ok	( insertItem,			"insertItem");

def_visible_primitive_debug_ok	( amin,					"amin");
def_visible_primitive_debug_ok	( amax,					"amax");

def_visible_primitive_debug_ok	( getXYZControllers,	"getXYZControllers");
def_visible_primitive			( displayControlDialog,	"displayControlDialog");

def_visible_primitive_debug_ok	( GetToolBtnState,		"GetToolBtnState");

def_visible_primitive_debug_ok	( eulerToQuat,			"eulerToQuat");
def_visible_primitive_debug_ok	( quatToEuler,			"quatToEuler");

#ifndef MAGMA
	def_struct_primitive		( tvw_pickTrackDlg2,	trackView,	"pickTrackDlg");
#endif // MAGMA

#ifndef MAGMA
	def_visible_primitive_debug_ok	( WSMSupportsCollision,	"WSMSupportsCollision");
	def_visible_primitive_debug_ok	( WSMSupportsForce,		"WSMSupportsForce");
	def_visible_primitive_debug_ok	( WSMApplyFC,			"WSMApplyFC");
#endif // MAGMA

def_visible_primitive_debug_ok	( getHashValue,			"getHashValue");

def_visible_primitive_debug_ok	( getPointControllers,	"getPointControllers");
def_visible_primitive_debug_ok	( getPointController,	"getPointController");

def_visible_primitive_debug_ok	( isStructDef,			"isStructDef");
def_visible_primitive_debug_ok	( isStruct,				"isStruct");
def_visible_primitive_debug_ok	( isController,			"isController");

def_struct_primitive_debug_ok	( formatValue,			units,	"formatValue");
def_struct_primitive_debug_ok	( decodeValue,			units,	"decodeValue");

def_visible_primitive_debug_ok	( isSelectionFrozen,	"isSelectionFrozen");
def_visible_primitive			( freezeSelection,		"freezeSelection");
def_visible_primitive			( thawSelection,		"thawSelection");

def_visible_primitive			( getBitmapOpenFileName,"getBitmapOpenFileName");
def_visible_primitive			( getBitmapSaveFileName,"getBitmapSaveFileName");
def_visible_primitive_debug_ok	( doesFileExist,		"doesFileExist");

def_visible_primitive			( getMAXSaveFileName,	"getMAXSaveFileName");
def_visible_primitive			( getMAXOpenFileName,	"getMAXOpenFileName");


def_visible_primitive			( getCurNameSelSet,		"getCurNameSelSet");

def_visible_primitive_debug_ok	( int,					"int");
def_visible_primitive_debug_ok	( stricmp,				"stricmp");

def_visible_primitive_debug_ok	( getCoreInterfaces,	"getCoreInterfaces");

def_visible_primitive_debug_ok	( getTextExtent,		"getTextExtent");
def_visible_primitive_debug_ok	( isValidNode,			"isValidNode");
def_visible_primitive_debug_ok	( isValidObj,			"isValidObj");
def_visible_primitive			( updateToolbarButtons,	"updateToolbarButtons");

def_visible_primitive_debug_ok	( okToCreate,			"okToCreate");

def_visible_primitive_debug_ok	( getSelectionLevel,	"getSelectionLevel");
def_visible_primitive			( setSelectionLevel,	"setSelectionLevel");

def_struct_primitive			( renderSceneDialog_Open,		renderSceneDialog,	"open");
def_struct_primitive			( renderSceneDialog_Cancel,		renderSceneDialog,	"cancel");
def_struct_primitive			( renderSceneDialog_Close,		renderSceneDialog,	"close");
def_struct_primitive			( renderSceneDialog_Commit,		renderSceneDialog,	"commit");
def_struct_primitive			( renderSceneDialog_Update,		renderSceneDialog,	"update");
def_struct_primitive_debug_ok	( renderSceneDialog_isOpen,		renderSceneDialog,	"isOpen");

def_visible_primitive_debug_ok	( getLastRenderedImage,			"getLastRenderedImage");

def_visible_primitive			( getMAXWindowSize,				"getMAXWindowSize" );
def_visible_primitive			( getMAXWindowPos,				"getMAXWindowPos" );

def_visible_primitive_debug_ok	( getClassInstances,			"getClassInstances");


def_visible_primitive_debug_ok	( isMSPluginClass,				"isMSPluginClass"); // is_msplugin_class
def_visible_primitive_debug_ok	( isMSCustAttribClass,			"isMSCustAttribClass"); // is_attribute_def
def_visible_primitive_debug_ok	( isMSPlugin,					"isMSPlugin"); 
def_visible_primitive_debug_ok	( isMSCustAttrib,				"isMSCustAttrib");

def_visible_primitive_debug_ok	( CompareBitmaps,				"compareBitmaps");

def_visible_primitive_debug_ok	( formattedPrint,				"formattedPrint");

def_visible_primitive_debug_ok	( used_maps,					"usedMaps");

/////////////////////////////
// lam_ctrl
/////////////////////////////

def_struct_primitive_debug_ok	( getLinkCount,				LinkCtrl,	"getLinkCount");
def_struct_primitive_debug_ok	( getLinkTime,				LinkCtrl,	"getLinkTime");
def_struct_primitive			( setLinkTime,				LinkCtrl,	"setLinkTime");
def_struct_primitive			( addLink,					LinkCtrl,	"addLink");
def_struct_primitive			( deleteLink,				LinkCtrl,	"deleteLink");
def_struct_primitive_debug_ok	( getLinkNode,				LinkCtrl,	"getLinkNode");
def_struct_primitive			( setLinkNode,				LinkCtrl,	"setLinkNode");

def_struct_primitive_debug_ok	( Attachment_GetKey,		AttachCtrl,	"getKey");
def_struct_primitive			( Attachment_AddNewKey,		AttachCtrl,	"addNewKey");
def_struct_primitive			( Attachment_Update,		AttachCtrl,	"update");

def_struct_primitive_debug_ok	( ListController_GetName,	ListCtrl,	"getName");
def_struct_primitive			( ListController_SetName,	ListCtrl,	"setName");
def_struct_primitive_debug_ok	( ListController_GetActive,	ListCtrl,	"getActive");
def_struct_primitive			( ListController_SetActive,	ListCtrl,	"setActive");

///////////////////////////////////////////////////////////////////////////////////////
//                             meshop
///////////////////////////////////////////////////////////////////////////////////////

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// following can be getter/setters for properties
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

def_struct_primitive_debug_ok	( meshop_getHiddenVerts,			meshop, "getHiddenVerts");
def_struct_primitive			( meshop_setHiddenVerts,			meshop, "setHiddenVerts");
def_struct_primitive_debug_ok	( meshop_getHiddenFaces,			meshop, "getHiddenFaces");
def_struct_primitive			( meshop_setHiddenFaces,			meshop, "setHiddenFaces");
def_struct_primitive_debug_ok	( meshop_getOpenEdges,				meshop, "getOpenEdges");
def_struct_primitive_debug_ok	( meshop_getNumVerts,				meshop, "getNumVerts");
def_struct_primitive			( meshop_setNumVerts,				meshop, "setNumVerts");
def_struct_primitive_debug_ok	( meshop_getNumFaces,				meshop, "getNumFaces");
def_struct_primitive			( meshop_setNumFaces,				meshop, "setNumFaces");
def_struct_primitive_debug_ok	( meshop_getNumTVerts,				meshop, "getNumTVerts");
def_struct_primitive			( meshop_setNumTVerts,				meshop, "setNumTVerts");
def_struct_primitive_debug_ok	( meshop_getNumCPVVerts,			meshop, "getNumCPVVerts");
def_struct_primitive			( meshop_setNumCPVVerts,			meshop, "setNumCPVVerts");
#ifndef WEBVERSION
def_struct_primitive_debug_ok	( meshop_getDisplacementMapping,	meshop, "getDisplacementMapping");
def_struct_primitive			( meshop_setDisplacementMapping,	meshop, "setDisplacementMapping");
def_struct_primitive_debug_ok	( meshop_getSubdivisionDisplacement, meshop, "getSubdivisionDisplacement");
def_struct_primitive			( meshop_setSubdivisionDisplacement, meshop, "setSubdivisionDisplacement");
def_struct_primitive_debug_ok	( meshop_getSplitMesh,				meshop, "getSplitMesh");
def_struct_primitive			( meshop_setSplitMesh,				meshop, "setSplitMesh");
def_struct_primitive_debug_ok	( meshop_getSubdivisionMethod,		meshop, "getSubdivisionMethod");
def_struct_primitive			( meshop_setSubdivisionMethod,		meshop, "setSubdivisionMethod");
def_struct_primitive_debug_ok	( meshop_getSubdivisionStyle,		meshop, "getSubdivisionStyle");
def_struct_primitive			( meshop_setSubdivisionStyle,		meshop, "setSubdivisionStyle");
def_struct_primitive_debug_ok	( meshop_getSubdivisionView,		meshop, "getSubdivisionView");
def_struct_primitive			( meshop_setSubdivisionView,		meshop, "setSubdivisionView");
def_struct_primitive_debug_ok	( meshop_getSubdivisionSteps,		meshop, "getSubdivisionSteps");
def_struct_primitive			( meshop_setSubdivisionSteps,		meshop, "setSubdivisionSteps");
def_struct_primitive_debug_ok	( meshop_getSubdivisionEdge,		meshop, "getSubdivisionEdge");
def_struct_primitive			( meshop_setSubdivisionEdge,		meshop, "setSubdivisionEdge");
def_struct_primitive_debug_ok	( meshop_getSubdivisionDistance,	meshop, "getSubdivisionDistance");
def_struct_primitive			( meshop_setSubdivisionDistance,	meshop, "setSubdivisionDistance");
def_struct_primitive_debug_ok	( meshop_getSubdivisionAngle,		meshop, "getSubdivisionAngle");
def_struct_primitive			( meshop_setSubdivisionAngle,		meshop, "setSubdivisionAngle");
def_struct_primitive_debug_ok	( meshop_getSubdivisionMinLevels,	meshop, "getSubdivisionMinLevels");
def_struct_primitive			( meshop_setSubdivisionMinLevels,	meshop, "setSubdivisionMinLevels");
def_struct_primitive_debug_ok	( meshop_getSubdivisionMaxLevels,	meshop, "getSubdivisionMaxLevels");
def_struct_primitive			( meshop_setSubdivisionMaxLevels,	meshop, "setSubdivisionMaxLevels");
def_struct_primitive_debug_ok	( meshop_getSubdivisionMaxTriangles, meshop, "getSubdivisionMaxTriangles");
def_struct_primitive			( meshop_setSubdivisionMaxTriangles, meshop, "setSubdivisionMaxTriangles");
#endif // WEBVERSION
def_struct_primitive_debug_ok	( meshop_getSelByVertex,			meshop, "getSelByVertex");
def_struct_primitive			( meshop_setSelByVertex,			meshop, "setSelByVertex");
def_struct_primitive_debug_ok	( meshop_getIgnoreBackfacing,		meshop, "getIgnoreBackfacing");
def_struct_primitive			( meshop_setIgnoreBackfacing,		meshop, "setIgnoreBackfacing");
def_struct_primitive_debug_ok	( meshop_getIgnoreVisEdges,			meshop, "getIgnoreVisEdges");
def_struct_primitive			( meshop_setIgnoreVisEdges,			meshop, "setIgnoreVisEdges");
def_struct_primitive_debug_ok	( meshop_getSoftSel,				meshop, "getSoftSel");
def_struct_primitive			( meshop_setSoftSel,				meshop, "setSoftSel");
def_struct_primitive_debug_ok	( meshop_getSSUseEdgeDist,			meshop, "getSSUseEdgeDist");
def_struct_primitive			( meshop_setSSUseEdgeDist,			meshop, "setSSUseEdgeDist");
def_struct_primitive_debug_ok	( meshop_getSSEdgeDist,				meshop, "getSSEdgeDist");
def_struct_primitive			( meshop_setSSEdgeDist,				meshop, "setSSEdgeDist");
def_struct_primitive_debug_ok	( meshop_getAffectBackfacing,		meshop, "getAffectBackfacing");
def_struct_primitive			( meshop_setAffectBackfacing,		meshop, "setAffectBackfacing");
def_struct_primitive_debug_ok	( meshop_getWeldPixels,				meshop, "getWeldPixels");
def_struct_primitive			( meshop_setWeldPixels,				meshop, "setWeldPixels");
def_struct_primitive_debug_ok	( meshop_getExtrusionType,			meshop, "getExtrusionType");
def_struct_primitive			( meshop_setExtrusionType,			meshop, "setExtrusionType");
def_struct_primitive_debug_ok	( meshop_getShowVNormals,			meshop, "getShowVNormals");
def_struct_primitive			( meshop_setShowVNormals,			meshop, "setShowVNormals");
def_struct_primitive_debug_ok	( meshop_getShowFNormals,			meshop, "getShowFNormals");
def_struct_primitive			( meshop_setShowFNormals,			meshop, "setShowFNormals");
def_struct_primitive_debug_ok	( meshop_getPlanarThreshold,		meshop, "getPlanarThreshold");
def_struct_primitive			( meshop_setPlanarThreshold,		meshop, "setPlanarThreshold");
def_struct_primitive_debug_ok	( meshop_getFalloff,				meshop, "getFalloff");
def_struct_primitive			( meshop_setFalloff,				meshop, "setFalloff");
def_struct_primitive_debug_ok	( meshop_getPinch,					meshop, "getPinch");
def_struct_primitive			( meshop_setPinch,					meshop, "setPinch");
def_struct_primitive_debug_ok	( meshop_getBubble,					meshop, "getBubble");
def_struct_primitive			( meshop_setBubble,					meshop, "setBubble");
def_struct_primitive_debug_ok	( meshop_getWeldThreshold,			meshop, "getWeldThreshold");
def_struct_primitive			( meshop_setWeldThreshold,			meshop, "setWeldThreshold");
def_struct_primitive_debug_ok	( meshop_getNormalSize,				meshop, "getNormalSize");
def_struct_primitive			( meshop_setNormalSize,				meshop, "setNormalSize");

// get SO bitarray based on SO bitarray

def_struct_primitive_debug_ok	( meshop_getEdgesUsingVert,			meshop,	"getEdgesUsingVert");
def_struct_primitive_debug_ok	( meshop_getFacesUsingVert,			meshop,	"getFacesUsingVert");
def_struct_primitive_debug_ok	( meshop_getPolysUsingVert,			meshop,	"getPolysUsingVert");
def_struct_primitive_debug_ok	( meshop_getVertsUsingEdge,			meshop,	"getVertsUsingEdge");
def_struct_primitive_debug_ok	( meshop_getFacesUsingEdge,			meshop,	"getFacesUsingEdge");
def_struct_primitive_debug_ok	( meshop_getPolysUsingEdge,			meshop,	"getPolysUsingEdge");
def_struct_primitive_debug_ok	( meshop_getVertsUsingFace,			meshop,	"getVertsUsingFace");
def_struct_primitive_debug_ok	( meshop_getEdgesUsingFace,			meshop,	"getEdgesUsingFace");
def_struct_primitive_debug_ok	( meshop_getPolysUsingFace,			meshop,	"getPolysUsingFace");
def_struct_primitive_debug_ok	( meshop_getEdgesReverseEdge,		meshop,	"getEdgesReverseEdge");
def_struct_primitive_debug_ok	( meshop_getElementsUsingFace,		meshop, "getElementsUsingFace");
def_struct_primitive_debug_ok	( meshop_getVertsUsedOnlyByFaces,	meshop, "getVertsUsedOnlyByFaces");

// info utilities

def_struct_primitive_debug_ok	( meshop_getVertexAngles,			meshop, "getVertexAngles");
def_struct_primitive_debug_ok	( meshop_getFaceArea,				meshop, "getFaceArea");
def_struct_primitive_debug_ok	( meshop_getFaceCenter,				meshop, "getFaceCenter");
def_struct_primitive_debug_ok	( meshop_getBaryCoords,				meshop, "getBaryCoords");
def_struct_primitive_debug_ok	( meshop_getIsoVerts,				meshop, "getIsoVerts");
def_struct_primitive_debug_ok	( meshop_getFaceRNormals,			meshop, "getFaceRNormals");
def_struct_primitive_debug_ok	( meshop_minVertexDistanceFrom,		meshop, "minVertexDistanceFrom");
def_struct_primitive_debug_ok	( meshop_minVertexDistancesFrom,	meshop, "minVertexDistancesFrom");

// operations: mesh level

def_struct_primitive			( meshop_attach,					meshop,	"attach");
def_struct_primitive			( meshop_removeDegenerateFaces,		meshop, "removeDegenerateFaces");
def_struct_primitive			( meshop_removeIllegalFaces,		meshop, "removeIllegalFaces");
def_struct_primitive			( meshop_deleteIsoVerts,			meshop, "deleteIsoVerts");
def_struct_primitive			( meshop_optimize,					meshop, "optimize");
def_struct_primitive			( meshop_cut,						meshop, "cut");
def_struct_primitive			( meshop_explodeAllFaces,			meshop, "explodeAllFaces");

// operations: vertex level

def_struct_primitive			( meshop_weldVertsByThreshold,		meshop,	"weldVertsByThreshold");
def_struct_primitive			( meshop_weldVertSet,				meshop,	"weldVertSet");
def_struct_primitive			( meshop_deleteVerts,				meshop, "deleteVerts");
def_struct_primitive			( meshop_breakVerts,				meshop, "breakVerts");
def_struct_primitive			( meshop_makeVertsPlanar,			meshop, "makeVertsPlanar");
def_struct_primitive			( meshop_moveVertsToPlane,			meshop, "moveVertsToPlane");
def_struct_primitive			( meshop_cloneVerts,				meshop, "cloneVerts");
def_struct_primitive			( meshop_detachVerts,				meshop, "detachVerts");
def_struct_primitive			( meshop_chamferVerts,				meshop, "chamferVerts");
def_struct_primitive_debug_ok	( meshop_getVert,					meshop, "getVert");
def_struct_primitive			( meshop_setVert,					meshop, "setVert");
def_struct_primitive			( meshop_moveVert,					meshop, "moveVert");

// operations: face level

def_struct_primitive			( meshop_createPolygon,				meshop, "createPolygon");
def_struct_primitive			( meshop_autoSmooth,				meshop,	"autoSmooth");
def_struct_primitive			( meshop_unifyNormals,				meshop,	"unifyNormals");
def_struct_primitive			( meshop_flipNormals,				meshop,	"flipNormals");
def_struct_primitive			( meshop_divideFaceByEdges,			meshop, "divideFaceByEdges");
def_struct_primitive			( meshop_deleteFaces,				meshop, "deleteFaces");
def_struct_primitive			( meshop_bevelFaces,				meshop, "bevelFaces");
def_struct_primitive			( meshop_makeFacesPlanar,			meshop, "makeFacesPlanar");
def_struct_primitive			( meshop_cloneFaces,				meshop, "cloneFaces");
def_struct_primitive			( meshop_collapseFaces,				meshop, "collapseFaces");
def_struct_primitive			( meshop_detachFaces,				meshop, "detachFaces");
def_struct_primitive			( meshop_divideFace,				meshop, "divideFace");
def_struct_primitive			( meshop_divideFaces,				meshop, "divideFaces");
def_struct_primitive			( meshop_explodeFaces,				meshop, "explodeFaces");
def_struct_primitive			( meshop_extrudeFaces,				meshop, "extrudeFaces");
def_struct_primitive			( meshop_slice,						meshop, "slice");

// operations: edge level

def_struct_primitive			( meshop_autoEdge,					meshop,	"autoEdge");
def_struct_primitive			( meshop_turnEdge,					meshop, "turnEdge");
def_struct_primitive			( meshop_deleteEdges,				meshop, "deleteEdges");
def_struct_primitive			( meshop_cloneEdges,				meshop, "cloneEdges");
def_struct_primitive			( meshop_collapseEdges,				meshop, "collapseEdges");
def_struct_primitive			( meshop_divideEdge,				meshop, "divideEdge");
def_struct_primitive			( meshop_divideEdges,				meshop, "divideEdges");
def_struct_primitive			( meshop_edgeTessellate,			meshop, "edgeTessellate");
def_struct_primitive			( meshop_extrudeEdges,				meshop, "extrudeEdges");
def_struct_primitive			( meshop_chamferEdges,				meshop, "chamferEdges");

// mapping

def_struct_primitive			( meshop_setNumMaps,				meshop,	"setNumMaps");
def_struct_primitive_debug_ok	( meshop_getNumMaps,				meshop,	"getNumMaps");
def_struct_primitive			( meshop_setMapSupport,				meshop,	"setMapSupport");
def_struct_primitive_debug_ok	( meshop_getMapSupport,				meshop,	"getMapSupport");

def_struct_primitive			( meshop_setNumMapVerts,			meshop,	"setNumMapVerts");
def_struct_primitive_debug_ok	( meshop_getNumMapVerts,			meshop,	"getNumMapVerts");
def_struct_primitive			( meshop_setNumMapFaces,			meshop,	"setNumMapFaces");
def_struct_primitive_debug_ok	( meshop_getNumMapFaces,			meshop,	"getNumMapFaces");

def_struct_primitive			( meshop_setMapVert,				meshop,	"setMapVert");
def_struct_primitive_debug_ok	( meshop_getMapVert,				meshop,	"getMapVert");
def_struct_primitive			( meshop_setMapFace,				meshop,	"setMapFace");
def_struct_primitive_debug_ok	( meshop_getMapFace,				meshop,	"getMapFace");

def_struct_primitive			( meshop_makeMapPlanar,				meshop,	"makeMapPlanar");
def_struct_primitive			( meshop_applyUVWMap,				meshop,	"applyUVWMap");
def_struct_primitive			( meshop_buildMapFaces,				meshop,	"buildMapFaces");
def_struct_primitive			( meshop_defaultMapFaces,			meshop,	"defaultMapFaces");

def_struct_primitive_debug_ok	( meshop_getIsoMapVerts,			meshop,	"getIsoMapVerts");
def_struct_primitive			( meshop_deleteIsoMapVerts,			meshop,	"deleteIsoMapVerts");
def_struct_primitive			( meshop_deleteIsoMapVertsAll,		meshop,	"deleteIsoMapVertsAll");
def_struct_primitive			( meshop_deleteMapVertSet,			meshop,	"deleteMapVertSet");
def_struct_primitive			( meshop_freeMapVerts,				meshop,	"freeMapVerts");
def_struct_primitive			( meshop_freeMapFaces,				meshop,	"freeMapFaces");
def_struct_primitive			( meshop_freeMapChannel,			meshop,	"freeMapChannel");

def_struct_primitive_debug_ok	( meshop_getMapFacesUsingMapVert,	meshop,	"getMapFacesUsingMapVert");
def_struct_primitive_debug_ok	( meshop_getMapVertsUsingMapFace,	meshop,	"getMapVertsUsingMapFace");

def_struct_primitive_debug_ok	( meshop_getVertsByColor,			meshop,	"getVertsByColor");

def_struct_primitive			( meshop_setFaceAlpha,				meshop,	"setFaceAlpha");
def_struct_primitive			( meshop_setVertAlpha,				meshop,	"setVertAlpha");
def_struct_primitive			( meshop_setFaceColor,				meshop,	"setFaceColor");
def_struct_primitive			( meshop_setVertColor,				meshop,	"setVertColor");

// vertex data

def_struct_primitive_debug_ok	( meshop_getVertWeight,				meshop,	"getVertWeight");
def_struct_primitive			( meshop_setVertWeight,				meshop,	"setVertWeight");
def_struct_primitive			( meshop_resetVertWeights,			meshop,	"resetVertWeights");
def_struct_primitive			( meshop_freeVertWeights,			meshop,	"freeVertWeights");
def_struct_primitive			( meshop_supportVertWeights,		meshop,	"supportVertWeights");

def_struct_primitive_debug_ok	( meshop_getVSelectWeight,			meshop,	"getVSelectWeight");
def_struct_primitive			( meshop_setVSelectWeight,			meshop,	"setVSelectWeight");
def_struct_primitive			( meshop_resetVSelectWeights,		meshop,	"resetVSelectWeights");
def_struct_primitive			( meshop_freeVSelectWeights,		meshop,	"freeVSelectWeights");
def_struct_primitive			( meshop_supportVSelectWeights,		meshop,	"supportVSelectWeights");

def_struct_primitive_debug_ok	( meshop_getVertCorner,				meshop,	"getVertCorner");
def_struct_primitive			( meshop_setVertCorner,				meshop,	"setVertCorner");
def_struct_primitive			( meshop_resetVertCorners,			meshop,	"resetVertCorners");
def_struct_primitive			( meshop_freeVertCorners,			meshop,	"freeVertCorners");
def_struct_primitive			( meshop_supportVertCorners,		meshop,	"supportVertCorners");

def_struct_primitive_debug_ok	( meshop_getVAlpha,					meshop,	"getVAlpha");
def_struct_primitive			( meshop_setVAlpha,					meshop,	"setVAlpha");
def_struct_primitive			( meshop_resetVAlphas,				meshop,	"resetVAlphas");
def_struct_primitive			( meshop_freeVAlphas,				meshop,	"freeVAlphas");
def_struct_primitive			( meshop_supportVAlphas,			meshop,	"supportVAlphas");

def_struct_primitive			( meshop_setNumVDataChannels,		meshop,	"setNumVDataChannels");
def_struct_primitive_debug_ok	( meshop_getNumVDataChannels,		meshop,	"getNumVDataChannels");
def_struct_primitive			( meshop_setVDataChannelSupport,	meshop,	"setVDataChannelSupport");
def_struct_primitive_debug_ok	( meshop_getVDataChannelSupport,	meshop,	"getVDataChannelSupport");

def_struct_primitive_debug_ok	( meshop_getVDataValue,				meshop,	"getVDataValue");
def_struct_primitive			( meshop_setVDataValue,				meshop,	"setVDataValue");
def_struct_primitive			( meshop_freeVData,					meshop,	"freeVData");

// UI parameters

def_struct_primitive_debug_ok	( meshop_getUIParam,				meshop,	"getUIParam");
def_struct_primitive			( meshop_setUIParam,				meshop,	"setUIParam");



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// following can be getter/setters for properties
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

def_struct_primitive_debug_ok	( polyop_getVertSelection,			polyop,	"getVertSelection");
def_struct_primitive			( polyop_setVertSelection,			polyop,	"setVertSelection");
def_struct_primitive_debug_ok	( polyop_getEdgeSelection,			polyop,	"getEdgeSelection");
def_struct_primitive			( polyop_setEdgeSelection,			polyop,	"setEdgeSelection");
def_struct_primitive_debug_ok	( polyop_getFaceSelection,			polyop,	"getFaceSelection");
def_struct_primitive			( polyop_setFaceSelection,			polyop,	"setFaceSelection");

def_struct_primitive_debug_ok	( polyop_getNumVerts,				polyop,	"getNumVerts");
def_struct_primitive_debug_ok	( polyop_getNumEdges,				polyop,	"getNumEdges");
def_struct_primitive_debug_ok	( polyop_getNumFaces,				polyop,	"getNumFaces");

def_struct_primitive_debug_ok	( polyop_getHiddenVerts,			polyop,	"getHiddenVerts");
def_struct_primitive			( polyop_setHiddenVerts,			polyop,	"setHiddenVerts");
def_struct_primitive_debug_ok	( polyop_getHiddenFaces,			polyop,	"getHiddenFaces");
def_struct_primitive			( polyop_setHiddenFaces,			polyop,	"setHiddenFaces");

def_struct_primitive_debug_ok	( polyop_getOpenEdges,				polyop,	"getOpenEdges");

// get/set SO flags

def_struct_primitive_debug_ok	( polyop_getVertsByFlag,			polyop,	"getVertsByFlag");
def_struct_primitive_debug_ok	( polyop_getVertFlags,				polyop,	"getVertFlags");
def_struct_primitive			( polyop_setVertFlags,				polyop,	"setVertFlags");
def_struct_primitive_debug_ok	( polyop_getEdgesByFlag,			polyop,	"getEdgesByFlag");
def_struct_primitive_debug_ok	( polyop_getEdgeFlags,				polyop,	"getEdgeFlags");
def_struct_primitive			( polyop_setEdgeFlags,				polyop,	"setEdgeFlags");
def_struct_primitive_debug_ok	( polyop_getFacesByFlag,			polyop,	"getFacesByFlag");
def_struct_primitive_debug_ok	( polyop_getFaceFlags,				polyop,	"getFaceFlags");
def_struct_primitive			( polyop_setFaceFlags,				polyop,	"setFaceFlags");

// get dead SO

def_struct_primitive_debug_ok	( polyop_getDeadVerts,				polyop,	"getDeadVerts");
def_struct_primitive_debug_ok	( polyop_getDeadEdges,				polyop,	"getDeadEdges");
def_struct_primitive_debug_ok	( polyop_getDeadFaces,				polyop,	"getDeadFaces");
def_struct_primitive_debug_ok	( polyop_getHasDeadStructs,			polyop,	"getHasDeadStructs");
def_struct_primitive_debug_ok	( polyop_isFaceDead,				polyop,	"isFaceDead");
def_struct_primitive_debug_ok	( polyop_isEdgeDead,				polyop,	"isEdgeDead");
def_struct_primitive_debug_ok	( polyop_isVertDead,				polyop,	"isVertDead");

// get SO bitarray based on SO bitarray

def_struct_primitive_debug_ok	( polyop_getEdgesUsingVert,			polyop,	"getEdgesUsingVert");
def_struct_primitive_debug_ok	( polyop_getFacesUsingVert,			polyop,	"getFacesUsingVert");
def_struct_primitive_debug_ok	( polyop_getVertsUsingEdge,			polyop,	"getVertsUsingEdge");
def_struct_primitive_debug_ok	( polyop_getFacesUsingEdge,			polyop,	"getFacesUsingEdge");
def_struct_primitive_debug_ok	( polyop_getVertsUsingFace,			polyop,	"getVertsUsingFace");
def_struct_primitive_debug_ok	( polyop_getEdgesUsingFace,			polyop,	"getEdgesUsingFace");
def_struct_primitive_debug_ok	( polyop_getElementsUsingFace,		polyop, "getElementsUsingFace");
def_struct_primitive_debug_ok	( polyop_getVertsUsedOnlyByFaces,	polyop, "getVertsUsedOnlyByFaces");

// info utilities - mesh

def_struct_primitive_debug_ok	( polyop_isMeshFilledIn,			polyop, "isMeshFilledIn");

// info utilities - vertex


// info utilities - edge

def_struct_primitive_debug_ok	( polyop_getBorderFromEdge,			polyop,	"getBorderFromEdge");
def_struct_primitive_debug_ok	( polyop_getEdgeVerts,				polyop,	"getEdgeVerts");
def_struct_primitive_debug_ok	( polyop_getEdgeFaces,				polyop,	"getEdgeFaces");

// info utilities - face

def_struct_primitive_debug_ok	( polyop_getFaceVerts,				polyop,	"getFaceVerts");
def_struct_primitive_debug_ok	( polyop_getFaceEdges,				polyop,	"getFaceEdges");
def_struct_primitive_debug_ok	( polyop_getFaceDeg,				polyop,	"getFaceDeg");
def_struct_primitive_debug_ok	( polyop_getFaceCenter,				polyop, "getFaceCenter");
def_struct_primitive_debug_ok	( polyop_getSafeFaceCenter,			polyop, "getSafeFaceCenter");
def_struct_primitive_debug_ok	( polyop_getFaceNormal,				polyop, "getFaceNormal");
def_struct_primitive_debug_ok	( polyop_getFaceArea,				polyop, "getFaceArea");

// operations: mesh level

def_struct_primitive			( polyop_collapseDeadStructs,		polyop,	"collapseDeadStructs");
def_struct_primitive			( polyop_attach,					polyop,	"attach");
def_struct_primitive			( polyop_deleteIsoVerts,			polyop, "deleteIsoVerts");
def_struct_primitive			( polyop_forceSubdivision,			polyop, "forceSubdivision");
def_struct_primitive			( polyop_propagateFlags,			polyop, "propagateFlags");
def_struct_primitive			( polyop_fillInMesh,				polyop, "fillInMesh");
def_struct_primitive			( polyop_resetSlicePlane,			polyop, "resetSlicePlane");
def_struct_primitive_debug_ok	( polyop_getSlicePlane,				polyop, "getSlicePlane");
def_struct_primitive			( polyop_setSlicePlane,				polyop, "setSlicePlane");
def_struct_primitive_debug_ok	( polyop_inSlicePlaneMode,			polyop, "inSlicePlaneMode");

// operations: vertex level

def_struct_primitive_debug_ok	( polyop_getVert,					polyop, "getVert");
def_struct_primitive			( polyop_setVert,					polyop, "setVert");
def_struct_primitive			( polyop_moveVert,					polyop, "moveVert");
def_struct_primitive			( polyop_deleteVerts,				polyop, "deleteVerts");
def_struct_primitive			( polyop_weldVertsByThreshold,		polyop,	"weldVertsByThreshold");
def_struct_primitive			( polyop_weldVerts,					polyop, "weldVerts");
def_struct_primitive			( polyop_createVert,				polyop, "createVert");
def_struct_primitive			( polyop_unHideAllVerts,			polyop, "unHideAllVerts");
def_struct_primitive			( polyop_breakVerts,				polyop, "breakVerts");
def_struct_primitive			( polyop_collapseVerts,				polyop, "collapseVerts");
def_struct_primitive			( polyop_meshSmoothByVert,			polyop, "meshSmoothByVert");
def_struct_primitive			( polyop_tessellateByVert,			polyop, "tessellateByVert");
def_struct_primitive			( polyop_detachVerts,				polyop, "detachVerts");
def_struct_primitive			( polyop_cutVert,					polyop, "cutVert");
def_struct_primitive			( polyop_capHolesByVert,			polyop, "capHolesByVert");
def_struct_primitive			( polyop_makeVertsPlanar,			polyop, "makeVertsPlanar");
def_struct_primitive			( polyop_moveVertsToPlane,			polyop, "moveVertsToPlane");
def_struct_primitive			( polyop_chamferVerts,				polyop,	"chamferVerts");

// operations: face level

def_struct_primitive_debug_ok	( polyop_getFaceMatID,				polyop,	"getFaceMatID");
def_struct_primitive			( polyop_setFaceMatID,				polyop,	"setFaceMatID");
def_struct_primitive			( polyop_deleteFaces,				polyop, "deleteFaces");
def_struct_primitive			( polyop_autoSmooth,				polyop,	"autoSmooth");
def_struct_primitive			( polyop_flipNormals,				polyop, "flipNormals");
def_struct_primitive			( polyop_retriangulate,				polyop, "retriangulate");
def_struct_primitive			( polyop_setDiagonal,				polyop, "setDiagonal");
def_struct_primitive			( polyop_createPolygon,				polyop, "createPolygon");
def_struct_primitive			( polyop_unHideAllFaces,			polyop, "unHideAllFaces");
def_struct_primitive			( polyop_setFaceSmoothGroup,		polyop,	"setFaceSmoothGroup");
def_struct_primitive_debug_ok	( polyop_getFaceSmoothGroup,		polyop,	"getFaceSmoothGroup");
def_struct_primitive			( polyop_divideFace,				polyop, "divideFace");
def_struct_primitive			( polyop_collapseFaces,				polyop, "collapseFaces");
def_struct_primitive			( polyop_meshSmoothByFace,			polyop, "meshSmoothByFace");
def_struct_primitive			( polyop_tessellateByFace,			polyop, "tessellateByFace");
def_struct_primitive			( polyop_detachFaces,				polyop, "detachFaces");
def_struct_primitive			( polyop_slice,						polyop, "slice");
def_struct_primitive			( polyop_cutFace,					polyop, "cutFace");
def_struct_primitive			( polyop_capHolesByFace,			polyop, "capHolesByFace");
def_struct_primitive			( polyop_makeFacesPlanar,			polyop, "makeFacesPlanar");
def_struct_primitive			( polyop_moveFacesToPlane,			polyop, "moveFacesToPlane");
def_struct_primitive			( polyop_extrudeFaces,				polyop,	"extrudeFaces");
def_struct_primitive			( polyop_bevelFaces,				polyop,	"bevelFaces");

// operations: edge level

def_struct_primitive			( polyop_deleteEdges,				polyop, "deleteEdges");
def_struct_primitive			( polyop_createEdge,				polyop, "createEdge");
def_struct_primitive			( polyop_weldEdgesByThreshold,		polyop,	"weldEdgesByThreshold");
def_struct_primitive			( polyop_weldEdges,					polyop, "weldEdges");
def_struct_primitive			( polyop_divideEdge,				polyop, "divideEdge");
def_struct_primitive			( polyop_collapseEdges,				polyop, "collapseEdges");
def_struct_primitive			( polyop_splitEdges,				polyop, "splitEdges");
def_struct_primitive			( polyop_meshSmoothByEdge,			polyop, "meshSmoothByEdge");
def_struct_primitive			( polyop_tessellateByEdge,			polyop, "tessellateByEdge");
def_struct_primitive			( polyop_detachEdges,				polyop, "detachEdges");
def_struct_primitive			( polyop_cutEdge,					polyop, "cutEdge");
def_struct_primitive			( polyop_capHolesByEdge,			polyop, "capHolesByEdge");
def_struct_primitive			( polyop_makeEdgesPlanar,			polyop, "makeEdgesPlanar");
def_struct_primitive			( polyop_moveEdgesToPlane,			polyop, "moveEdgesToPlane");
def_struct_primitive			( polyop_createShape,				polyop, "createShape");
def_struct_primitive_debug_ok	( polyop_getEdgeVis,				polyop, "getEdgeVis");
def_struct_primitive			( polyop_setEdgeVis,				polyop, "setEdgeVis");
def_struct_primitive			( polyop_chamferEdges,				polyop,	"chamferEdges");

// mapping

def_struct_primitive			( polyop_setNumMaps,				polyop,	"setNumMaps");
def_struct_primitive_debug_ok	( polyop_getNumMaps,				polyop,	"getNumMaps");
def_struct_primitive			( polyop_setMapSupport,				polyop,	"setMapSupport");
def_struct_primitive_debug_ok	( polyop_getMapSupport,				polyop,	"getMapSupport");

def_struct_primitive			( polyop_setNumMapVerts,			polyop,	"setNumMapVerts");
def_struct_primitive_debug_ok	( polyop_getNumMapVerts,			polyop,	"getNumMapVerts");
def_struct_primitive			( polyop_setNumMapFaces,			polyop,	"setNumMapFaces");
def_struct_primitive_debug_ok	( polyop_getNumMapFaces,			polyop,	"getNumMapFaces");

def_struct_primitive			( polyop_setMapVert,				polyop,	"setMapVert");
def_struct_primitive_debug_ok	( polyop_getMapVert,				polyop,	"getMapVert");
def_struct_primitive			( polyop_setMapFace,				polyop,	"setMapFace");
def_struct_primitive_debug_ok	( polyop_getMapFace,				polyop,	"getMapFace");

def_struct_primitive			( polyop_defaultMapFaces,			polyop,	"defaultMapFaces");
def_struct_primitive			( polyop_applyUVWMap,				polyop,	"applyUVWMap");
def_struct_primitive_debug_ok	( polyop_getVertsByColor,			polyop,	"getVertsByColor");

def_struct_primitive			( polyop_setFaceColor,				polyop,	"setFaceColor");
def_struct_primitive			( polyop_setVertColor,				polyop,	"setVertColor");

def_struct_primitive			( polyop_setNumVDataChannels,		polyop,	"setNumVDataChannels");
def_struct_primitive_debug_ok	( polyop_getNumVDataChannels,		polyop,	"getNumVDataChannels");
def_struct_primitive			( polyop_setVDataChannelSupport,	polyop,	"setVDataChannelSupport");
def_struct_primitive_debug_ok	( polyop_getVDataChannelSupport,	polyop,	"getVDataChannelSupport");
def_struct_primitive_debug_ok	( polyop_getVDataValue,				polyop,	"getVDataValue");
def_struct_primitive			( polyop_setVDataValue,				polyop,	"setVDataValue");
def_struct_primitive			( polyop_freeVData,					polyop,	"freeVData");
def_struct_primitive			( polyop_resetVData,				polyop,	"resetVData");

def_struct_primitive			( polyop_setNumEDataChannels,		polyop,	"setNumEDataChannels");
def_struct_primitive_debug_ok	( polyop_getNumEDataChannels,		polyop,	"getNumEDataChannels");
def_struct_primitive			( polyop_setEDataChannelSupport,	polyop,	"setEDataChannelSupport");
def_struct_primitive_debug_ok	( polyop_getEDataChannelSupport,	polyop,	"getEDataChannelSupport");
def_struct_primitive_debug_ok	( polyop_getEDataValue,				polyop,	"getEDataValue");
def_struct_primitive			( polyop_setEDataValue,				polyop,	"setEDataValue");
def_struct_primitive			( polyop_freeEData,					polyop,	"freeEData");
def_struct_primitive			( polyop_resetEData,				polyop,	"resetEData");

def_struct_primitive			( theHold_Begin,				theHold,	"Begin");
def_struct_primitive			( theHold_Suspend,				theHold,	"Suspend");
def_struct_primitive			( theHold_IsSuspended,			theHold,	"IsSuspended");
def_struct_primitive			( theHold_Resume,				theHold,	"Resume");
def_struct_primitive			( theHold_Holding,				theHold,	"Holding");
def_struct_primitive			( theHold_Restoring,			theHold,	"Restoring");
def_struct_primitive			( theHold_Redoing,				theHold,	"Redoing");
def_struct_primitive			( theHold_RestoreOrRedoing,		theHold,	"RestoreOrRedoing");
def_struct_primitive			( theHold_DisableUndo,			theHold,	"DisableUndo");
def_struct_primitive			( theHold_EnableUndo,			theHold,	"EnableUndo");
def_struct_primitive			( theHold_Restore,				theHold,	"Restore");
def_struct_primitive			( theHold_Release,				theHold,	"Release"); 

		// 3 ways to terminate the Begin()...
def_struct_primitive			( theHold_End,					theHold,	"End");
def_struct_primitive			( theHold_Accept,				theHold,	"Accept");
def_struct_primitive			( theHold_Cancel,				theHold,	"Cancel");

		//		
		// Group several Begin-End lists into a single Super-group.
def_struct_primitive			( theHold_SuperBegin,			theHold,	"SuperBegin");
def_struct_primitive			( theHold_SuperAccept,			theHold,	"SuperAccept");
def_struct_primitive			( theHold_SuperCancel,			theHold,	"SuperCancel");
def_struct_primitive			( theHold_GetSuperLevel,		theHold,	"SuperLevel");

		// Get the number of times Put() has been called in the current session of MAX
def_struct_primitive_debug_ok	( theHold_GetGlobalPutCount,	theHold,	"GetGlobalPutCount");

def_mapped_primitive			( ResetXForm,			"ResetXForm");

def_mapped_primitive			( CenterPivot,			"CenterPivot");
def_mapped_primitive			( CenterObject,			"CenterObject");
def_mapped_primitive			( WorldAlignPivot,		"WorldAlignPivot");
def_mapped_primitive			( WorldAlignObject,		"WorldAlignObject");
def_mapped_primitive			( AlignPivot,			"AlignPivot");
def_mapped_primitive			( AlignObject,			"AlignObject");
def_mapped_primitive			( AlignToParent,		"AlignToParent");
def_mapped_primitive			( ResetTransform,		"ResetTransform");
def_mapped_primitive			( ResetScale,			"ResetScale");
def_mapped_primitive			( ResetPivot,			"ResetPivot");

def_struct_primitive			( logsys_logEntry,		logsystem,	"logEntry");
def_struct_primitive			( logsys_logName,		logsystem,	"logName");

def_visible_primitive_debug_ok	( areMtlAndRendererCompatible,	"areMtlAndRendererCompatible");

def_struct_primitive			 ( HideByCategory_all,		hideByCategory,	"all");
def_struct_primitive			 ( HideByCategory_none,		hideByCategory,	"none");

def_visible_primitive_debug_ok	( GetQuietMode, 			"GetQuietMode");
def_visible_primitive			( SetQuietMode, 			"SetQuietMode");
def_visible_primitive_debug_ok	( GetMAXIniFile, 			"GetMAXIniFile");

def_visible_primitive			( SuspendEditing, 			"SuspendEditing");
def_visible_primitive			( ResumeEditing, 			"ResumeEditing");
def_visible_primitive_debug_ok	( getClassName, 			"getClassName");

def_visible_primitive_debug_ok	( isMaxFile, 				"isMaxFile");

def_visible_primitive_debug_ok	( is64bitApplication, 		"is64bitApplication");


#ifndef NO_RENDEREFFECTS 
def_struct_primitive			( envEffectsDialog_Open,			envEffectsDialog,	"Open");
def_struct_primitive			( envEffectsDialog_Close,			envEffectsDialog,	"Close");
def_struct_primitive_debug_ok	( envEffectsDialog_isOpen,			envEffectsDialog,	"isOpen");
#endif // NO_RENDEREFFECTS

def_struct_primitive			( MatEditor_Open,					MatEditor,		"Open");
def_struct_primitive			( MatEditor_Close,					MatEditor,		"Close");
def_struct_primitive_debug_ok	( MatEditor_isOpen,					MatEditor,		"isOpen");

def_struct_primitive_debug_ok	( tabbedDialogs_isOpen,				tabbedDialogs,	"isOpen");
def_struct_primitive_debug_ok	( tabbedDialogs_getDialogID,		tabbedDialogs,	"getDialogID");
def_struct_primitive			( tabbedDialogs_invalidate,			tabbedDialogs,	"invalidate");
def_struct_primitive			( tabbedDialogs_OkToCommit,			tabbedDialogs,	"OkToCommit");
def_struct_primitive			( tabbedDialogs_CommitPages,		tabbedDialogs,	"CommitPages");
def_struct_primitive			( tabbedDialogs_CloseDialog,		tabbedDialogs,	"CloseDialog");
def_struct_primitive			( tabbedDialogs_CancelDialog,		tabbedDialogs,	"CancelDialog");
def_struct_primitive_debug_ok	( tabbedDialogs_getNumPages,		tabbedDialogs,	"getNumPages");
def_struct_primitive_debug_ok	( tabbedDialogs_getPageID,			tabbedDialogs,	"getPageID");
def_struct_primitive_debug_ok	( tabbedDialogs_getCurrentPage,		tabbedDialogs,	"getCurrentPage");
def_struct_primitive			( tabbedDialogs_setCurrentPage,		tabbedDialogs,	"setCurrentPage");
def_struct_primitive			( tabbedDialogs_invalidatePage,		tabbedDialogs,	"invalidatePage");
def_struct_primitive_debug_ok	( tabbedDialogs_getPageTitle,		tabbedDialogs,	"getPageTitle");
def_struct_primitive_debug_ok	( tabbedDialogs_isPage,				tabbedDialogs,	"isPage");

// Dependency Tests
def_struct_primitive			( DependencyLoopTest,		refs,			"DependencyLoopTest");

// snap system
def_struct_primitive_debug_ok	( snap_getOSnapName,		snapMode,		"getOSnapName");
def_struct_primitive_debug_ok	( snap_getOSnapNumItems,	snapMode,		"getOSnapNumItems");
def_struct_primitive_debug_ok	( snap_getOSnapItemName,	snapMode,		"getOSnapItemName");
def_struct_primitive_debug_ok	( snap_getOSnapItemToolTip,	snapMode,		"getOSnapItemToolTip");
def_struct_primitive_debug_ok	( snap_getOSnapItemActive,	snapMode,		"getOSnapItemActive");
def_struct_primitive_debug_ok	( snap_setOSnapItemActive,	snapMode,		"setOSnapItemActive");

// camera functions

def_struct_primitive_debug_ok	( cameraFOV_MMtoFOV,		cameraFOV,		"MMtoFOV");
def_struct_primitive_debug_ok	( cameraFOV_FOVtoMM,		cameraFOV,		"FOVtoMM");
def_struct_primitive_debug_ok	( cameraFOV_CurFOVtoFOV,	cameraFOV,		"CurFOVtoFOV");
def_struct_primitive_debug_ok	( cameraFOV_FOVtoCurFOV,	cameraFOV,		"FOVtoCurFOV");

// r10 avg_dlx integration

def_visible_primitive	( WSMSupportsCollision,										"WSMSupportsCollision");
def_visible_primitive	( WSMSupportsForce,											"WSMSupportsForce");
def_visible_primitive	( WSMApplyFC,												"WSMApplyFC");

def_visible_primitive	( quatToEuler2,												"quatToEuler2");
def_visible_primitive	( quatArrayToEulerArray,									"quatArrayToEulerArray");

def_visible_primitive	( close_enough,												"close_enough");
def_visible_primitive	( appendIfUnique,											"appendIfUnique");
def_visible_primitive	( pasteBitmap,												"pasteBitmap");

def_visible_primitive	( weldSpline,												"weldSpline");
def_visible_primitive	( FindPathSegAndParam,										"FindPathSegAndParam");
def_visible_primitive	( FindLengthSegAndParam,									"FindLengthSegAndParam");
def_visible_primitive	( interpBezier3D,											"interpBezier3D");
def_visible_primitive	( tangentBezier3D,											"tangentBezier3D");

def_visible_primitive	( getLocalTime,												"getLocalTime");
def_visible_primitive	( getUniversalTime,											"getUniversalTime");

def_visible_primitive	( isDeformable,												"isDeformable");
def_visible_primitive	( numPoints,												"numPoints");
def_visible_primitive	( getPointPos,												"getPointPos");

def_visible_primitive	( nodeGetBoundingBox,										"nodeGetBoundingBox");
def_visible_primitive	( nodeLocalBoundingBox,										"nodeLocalBoundingBox");

def_visible_primitive	( intersectRayScene,										"intersectRayScene");
def_visible_primitive	( setNeedsRedraw,											"setNeedsRedraw");

def_visible_primitive	( makeUniqueArray,											"makeUniqueArray");

def_visible_primitive	( toUpper,													"toUpper" );
def_visible_primitive	( toLower,													"toLower" );
def_visible_primitive	( substituteString,											"substituteString" );
def_visible_primitive	( replace_LF_with_CRLF,										"replace_LF_with_CRLF" );
def_visible_primitive	( replace_CRLF_with_LF,										"replace_CRLF_with_LF" );

def_visible_primitive	( clearControllerNewFlag,									"clearControllerNewFlag" );
def_visible_primitive	( scan_for_new_plugins,										"scanForNewPlugins" );

def_visible_primitive	( colorPickerDlg,											"colorPickerDlg" );

def_visible_primitive	( SetGridSpacing, 											"SetGridSpacing");
def_visible_primitive	( SetGridMajorLines,									 	"SetGridMajorLines");

def_visible_primitive	( getBitmapInfo,										 	"getBitmapInfo");
def_visible_primitive	( getNodeTM,										 		"getNodeTM");
def_visible_primitive	( isAnimPlaying,											"isAnimPlaying");

def_visible_primitive	( particleLife,												"particleLife");
def_visible_primitive	( particleCenter,											"particleCenter");
def_visible_primitive	( particleSize2,											"particleSize2");

def_visible_primitive	( areNodesInstances,										"areNodesInstances");

def_visible_primitive	( HiddenDOSCommand,											"HiddenDOSCommand");

def_visible_primitive	( pathIsNetworkPath,										"pathIsNetworkPath");

def_struct_primitive	( LayerManager_getLayerObject,		ILayerManager,			"getLayerObject");

def_struct_primitive	( NodeExposureInterface_create,		NodeExposureInterface,	"create");
def_struct_primitive	( NodeExposureInterface_get,		NodeExposureInterface,	"get");

def_struct_primitive	( GetDraftRenderer,					renderers,				"GetDraftRenderer");
def_struct_primitive	( ClearDraftRenderer,				renderers,				"ClearDraftRenderer");


//////////////////////////
// defect test routines //
//////////////////////////

// def_visible_primitive	( tester470888 ,		"tester470888");

////////////////
