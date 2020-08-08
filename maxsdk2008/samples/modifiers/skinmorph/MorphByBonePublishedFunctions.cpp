#include "MorphByBone.h"

static MorphByBoneClassDesc MorphByBoneDesc;
ClassDesc* GetMorphByBoneDesc() { return &MorphByBoneDesc; }

class MorphByBoneAccessor : public PBAccessor
{ 
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		MorphByBone* p = (MorphByBone*)owner;

		switch (id)
		{
		case pb_lname:
			{
				if ((p) && (!p->pauseAccessor))
					p->SetMorphName(v.s,p->currentBone,p->currentMorph);				
				break;
			}
		case pb_linfluence:
			{
				if ((p) && (!p->pauseAccessor))
					p->SetMorphInfluence(v.f,p->currentBone,p->currentMorph);				
				break;
			}
		case pb_lfalloff:
			{
				if ((p) && (!p->pauseAccessor))
					p->SetMorphFalloff(v.i,p->currentBone,p->currentMorph);				
				break;
			}
		case pb_ljointtype:
			{
				if ((p) && (!p->pauseAccessor))
					p->SetJointType(v.i,p->currentBone);				
				break;
			}
		case pb_lenabled:
			{
				if ((p) && (!p->pauseAccessor))
					p->SetMorphEnabled(v.i,p->currentBone,p->currentMorph);				
				break;

			}

			/*
			case pb_ltargnode:
			{
			if ((p) && (!p->pauseAccessor))
			{
			INode *node = (INode*) v.r;
			p->SetNode(node,p->currentBone,p->currentMorph);				
			}
			break;
			}
			*/

		}
	}
	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid)
	{
		MorphByBone* p = (MorphByBone*)owner;

	}
};


static MorphByBoneAccessor morphbybone_accessor;

static ParamBlockDesc2 
morphbybone_param_blk ( morphbybone_params, _T("params"),  0, &MorphByBoneDesc, 
					   P_AUTO_CONSTRUCT + P_AUTO_UI+ P_MULTIMAP, PBLOCK_REF, 
					   //rollout
					   5,
					   morphbybone_mapparams, IDD_PANEL, IDS_PARAMS, 0, 0, NULL,
					   morphbybone_mapparamsselection, IDD_PANELSOFTSELECTION, IDS_SELECTION, 0, 0, NULL,
					   morphbybone_mapparamsprop, IDD_PANELPROPS, IDS_PARAMSPROP, 0, 0, NULL,
					   morphbybone_mapparamscopy, IDD_PANELCOPY, IDS_PARAMSCOPY, 0, 0, NULL,
					   morphbybone_mapparamsoptions, IDD_PANELOPTIONS, IDS_OPTIONS, 0, 0, NULL,
					   // params

					   pb_bonelist,    _T("bones"),  TYPE_INODE_TAB,		0,	P_VARIABLE_SIZE,	IDS_BONE_NODES,
					   end,

					   pb_lname, 	_T("morphName"),		TYPE_STRING, 	0,  IDS_NAME,
					   p_ui,  morphbybone_mapparamsprop, TYPE_EDITBOX,  IDC_NAME,
					   p_accessor,		&morphbybone_accessor,
					   end, 

					   pb_linfluence, 			_T("influenceAngle"), 		TYPE_FLOAT, 	0, 	IDS_INFLUENCE, 
					   p_default, 		90.0f, 
					   p_range, 		0.0f,180.0f, 
					   p_ui, morphbybone_mapparamsprop, TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_ANGLE,	IDC_ANGLE_SPIN, 1.0f, 
					   p_accessor,		&morphbybone_accessor,
					   end,

					   pb_lfalloff, 			_T("falloff"), 		TYPE_INT, 	0, 	IDS_FALLOFF, 
					   p_default, 		0, 
					   p_ui, morphbybone_mapparamsprop, TYPE_INTLISTBOX, IDC_FALLOFF_COMBO, 5, 
					   IDS_LINEAR,
					   IDS_SINUAL,
					   IDS_FAST,
					   IDS_SLOW,
					   IDS_CUSTOMFALLOFF,
					   p_accessor,		&morphbybone_accessor,
					   end,

					   pb_showmirror, 	_T("showMirrorPlane"),		TYPE_BOOL, 		P_RESET_DEFAULT,IDS_SHOWMIRRORPLANE,
					   p_default, 		FALSE, 
					   p_ui,  morphbybone_mapparamscopy, TYPE_SINGLECHEKBOX, 	IDC_SHOWMIRRORPLANE_CHECK, 
					   end, 

					   pb_mirrorplane, 			_T("mirrorPlane"), 		TYPE_INT, 	0, 	IDS_MIRRORPLANE, 
					   p_default, 		0, 
					   p_ui, morphbybone_mapparamscopy,TYPE_INTLISTBOX, IDC_MIRROR_COMBO, 3, IDS_X,
					   IDS_Y,
					   IDS_Z,
					   end,

					   pb_mirroroffset, 			_T("mirrorOffset"), 		TYPE_FLOAT, 	0, 	IDS_MIRROROFFSET, 
					   p_default, 		0.0f, 
					   p_range, 		-1000000.0f,1000000.0f, 
					   p_ui, 			morphbybone_mapparamscopy, TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_MIRROROFFSET,	IDC_MIRROROFFSET_SPIN, 1.0f, 
					   end,


					   pb_mirrortm, 	_T(""),		TYPE_BOOL, 		P_RESET_DEFAULT,IDS_MIRRORTM,
					   p_default, 		FALSE, 
					   p_ui,  morphbybone_mapparamscopy,TYPE_SINGLECHEKBOX, 	IDC_MIRRORTM_CHECK, 
					   end, 

					   pb_mirrorthreshold, 			_T("mirrorThreshold"), 		TYPE_FLOAT, 	0, 	IDS_MIRRORTHRESHOLD, 
					   p_default, 		1.0f, 
					   p_range, 		0.0f,100.0f, 
					   p_ui, 			morphbybone_mapparamscopy, TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_THRESHOLD,	IDC_THRESHOLD_SPIN, 0.01f, 
					   end,

					   pb_mirrorpreviewverts, 	_T("previewVerts"),		TYPE_BOOL, 		P_RESET_DEFAULT,IDS_PREVIEWVERTS,
					   p_default, 		FALSE, 
					   p_ui,  morphbybone_mapparamscopy,TYPE_SINGLECHEKBOX, 	IDC_PREVIEWVERTICES_CHECK, 
					   end, 

					   pb_mirrorpreviewbone, 	_T("previewBone"),		TYPE_BOOL, 		P_RESET_DEFAULT,IDS_PREVIEWBONE,
					   p_default, 		FALSE, 
					   p_ui,  morphbybone_mapparamscopy,TYPE_SINGLECHEKBOX, 	IDC_PREVIEWBONE_CHECK, 
					   end, 


					   pb_ljointtype, 			_T("jointType"), 		TYPE_INT, 	0, 	IDS_JOINTTYPE, 
					   p_default, 		0, 
					   p_ui, morphbybone_mapparamsprop,TYPE_INTLISTBOX, IDC_JOINTTYPE_COMBO, 2, 
					   IDS_BALLJOINT,
					   IDS_PLANARJOINT,
					   p_accessor,		&morphbybone_accessor,

					   end,

					   /*	pb_ltargnode, 	_T("targetNode"),		TYPE_INODE, 		0,				IDS_TARGETNODE,
					   p_ui,	morphbybone_mapparamsprop,	TYPE_PICKNODEBUTTON, 	IDC_EXTERNALNODE, 
					   end, 
					   */
					   pb_targnodelist,    _T("targetNodes"),  TYPE_INODE_TAB,		0,	P_VARIABLE_SIZE,	IDS_TARGETNODES,
					   end,

					   pb_falloffgraphlist,    _T("falloffGraphs"),  TYPE_REFTARG_TAB,		0,	P_VARIABLE_SIZE,	IDS_FALLOFFGRAPHS,
					   end,

					   pb_reloadselected, 	_T("reloadSelected"),		TYPE_BOOL, 		P_RESET_DEFAULT,IDS_RELOADSELECTED,
					   p_default, 		FALSE, 
					   p_ui,  morphbybone_mapparamsprop,TYPE_SINGLECHEKBOX, 	IDC_RELOADSELECTED_CHECK, 
					   end, 

					   pb_safemode, 	_T("safemode"),		TYPE_BOOL, 		0,IDS_SAFEMODE,
					   p_default, 		TRUE, 
					   p_ui,  morphbybone_mapparamsoptions,TYPE_SINGLECHEKBOX, 	IDC_SAFEMODE_CHECK, 
					   end, 

					   pb_showdriverbone, 	_T("showDriverBone"),		TYPE_BOOL, 		0,IDS_SHOWDRIVERBONE,
					   p_default, 		TRUE, 
					   p_ui,  morphbybone_mapparamsoptions,TYPE_SINGLECHEKBOX, 	IDC_SHOWDRIVERBONE_CHECK, 
					   end, 

					   pb_showmorphbone, 	_T("showMorphBone"),		TYPE_BOOL, 		0,IDS_SHOWMORPHBONE,
					   p_default, 		TRUE, 
					   p_ui,  morphbybone_mapparamsoptions,TYPE_SINGLECHEKBOX, 	IDC_SHOWMORPHBONE_CHECK, 
					   end, 

					   pb_showcurrentangle, 	_T("showCurrentAngle"),		TYPE_BOOL, 		0,IDS_SHOWCURRENTANGLE,
					   p_default, 		TRUE, 
					   p_ui,  morphbybone_mapparamsoptions,TYPE_SINGLECHEKBOX, 	IDC_SHOWCURRENTANGLE_CHECK, 
					   end, 
					   pb_showanglelimit, 	_T("showLimitAngle"),		TYPE_BOOL, 		0,IDS_SHOWLIMITANGLE,
					   p_default, 		FALSE, 
					   //		p_ui,  morphbybone_mapparamsoptions,TYPE_SINGLECHEKBOX, 	IDC_SHOWLIMITANGLE_CHECK, 
					   end, 

					   pb_matrixsize,			_T("matrixSize"), 		TYPE_FLOAT, 	0, 	IDS_MATRIXSIZE, 
					   p_default, 		10.0f, 
					   p_range, 		0.0f,1000.0f, 
					   p_ui, 			morphbybone_mapparamsoptions, TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_MATRIXSIZE,	IDC_MATRIXSIZE_SPIN, 1.0f, 
					   end,

					   pb_showdeltas, 	_T("showDeltas"),		TYPE_BOOL, 		0,	IDS_SHOWDELTAS,
					   p_default, 		TRUE, 
					   p_ui,  morphbybone_mapparamsoptions,TYPE_SINGLECHEKBOX, 	IDC_SHOWDELTAS_CHECK, 
					   end, 

					   pb_drawx, 	_T("showX"),		TYPE_BOOL, 		0,	IDS_SHOWX,
					   p_default, 		TRUE, 
					   //		p_ui,  morphbybone_mapparamsoptions,TYPE_SINGLECHEKBOX, 	IDC_X_CHECK, 
					   end, 

					   pb_drawy, 	_T("showY"),		TYPE_BOOL, 		0,	IDS_SHOWY,
					   p_default, 		TRUE, 
					   //		p_ui,  morphbybone_mapparamsoptions,TYPE_SINGLECHEKBOX, 	IDC_Y_CHECK, 
					   end, 

					   pb_drawz, 	_T("showZ"),		TYPE_BOOL, 		0,	IDS_SHOWZ,
					   p_default, 		TRUE, 
					   //		p_ui,  morphbybone_mapparamsoptions,TYPE_SINGLECHEKBOX, 	IDC_Z_CHECK, 
					   end, 

					   pb_bonesize,			_T("boneSize"), 		TYPE_FLOAT, 	0, 	IDS_BONESIZE, 
					   p_default, 		1.0f, 
					   p_range, 		0.0001f,10.0f, 
					   p_ui, 			morphbybone_mapparamsoptions, TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_BONESIZE,	IDC_BONESIZE_SPIN, 0.10f, 
					   end,



					   pb_selectiongraph, _T("softSelectionGraph"),			TYPE_REFTARG, 	P_SUBANIM, 	IDS_SOFTSELECTIONGRAPH,
					   end, 

					   pb_usesoftselection, 	_T("useSoftSelection"),		TYPE_BOOL, 		0,IDS_USESOFTSELECTION,
					   p_default, 		FALSE, 
					   p_ui,  morphbybone_mapparamsselection,TYPE_SINGLECHEKBOX, 	IDC_USESOFTSELECTIONCHECK, 
					   end, 

					   pb_selectionradius,			_T("selectionRadius"), 		TYPE_FLOAT, 	0, 	IDS_SELECTIONRADIUS, 
					   p_default, 		10.0f, 
					   p_range, 		0.0001f,1000.0f, 
					   p_ui, 			morphbybone_mapparamsselection, TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_RADIUS,	IDC_RADIUS_SPIN, 0.10f, 
					   end,


					   pb_edgelimit,			_T("edgeLimit"), 		TYPE_INT, 	0, 	IDS_EDGELIMIT, 
					   p_default, 		10, 
					   p_range, 		1,1000, 
					   p_ui, 			morphbybone_mapparamsselection, TYPE_SPINNER,		EDITTYPE_INT, IDC_EDGELIMIT,	IDC_EDGELIMIT_SPIN, 1.0f, 
					   end,

					   pb_useedgelimit, 	_T("useEdgeLimit"),		TYPE_BOOL, 		0,IDS_USEEDGELIMIT,
					   p_default, 		FALSE, 
					   p_enable_ctrls,		1,	pb_edgelimit,
					   p_ui,  morphbybone_mapparamsselection,TYPE_SINGLECHEKBOX, 	IDC_USEEDGELIMITCHECK, 
					   end, 

					   pb_lenabled, 	_T("enabled"),		TYPE_BOOL, 		0,IDS_ENABLED,
					   p_default, 		TRUE, 
					   p_ui,  morphbybone_mapparamsprop,TYPE_SINGLECHEKBOX, 	IDC_ENABLED_CHECK, 
					   p_accessor,		&morphbybone_accessor,
					   end, 

					   
					   pb_showedges, 	_T("showEdges"),		TYPE_BOOL, 		0,IDS_SHOWEDGES,
					   p_default, 		TRUE, 
					   p_ui,  morphbybone_mapparamsoptions,TYPE_SINGLECHEKBOX, 	IDC_SHOWEDGES_CHECK, 
					   end, 

					   end
					   );






//Function Publishing descriptor for Mixin interface
//*****************************************************
static FPInterfaceDesc morphbybone_interface(
    MORPHBYBONE_INTERFACE, _T("skinMorphOps"), 0, &MorphByBoneDesc, FP_MIXIN,
		morphbybone_addbone, _T("addBone"), 0, TYPE_VOID, 0, 1,
			_T("node"), 0, TYPE_INODE,
		morphbybone_removebone, _T("removeBone"), 0, TYPE_VOID, 0, 1,
			_T("node"), 0, TYPE_INODE,
		morphbybone_selectbone, _T("selectBone"), 0, TYPE_VOID, 0, 2,
			_T("node"), 0, TYPE_INODE,
			_T("morphName"), 0, TYPE_STRING,
		morphbybone_selectvertices, _T("selectVertices"), 0, TYPE_VOID, 0, 2,
			_T("node"), 0, TYPE_INODE,
			_T("sel"), 0, TYPE_BITARRAY,
		morphbybone_isselectedvertex, _T("isSelectedVertex"), 0, TYPE_BOOL, 0, 2,
			_T("node"), 0, TYPE_INODE,
			_T("vertexIndex"), 0, TYPE_INT,

		morphbybone_resetgraph, _T("resetSelectionGraph"), 0, TYPE_VOID, 0, 0,
		morphbybone_grow, _T("grow"), 0, TYPE_VOID, 0, 0,
		morphbybone_shrink, _T("shrink"), 0, TYPE_VOID, 0, 0,
		morphbybone_loop, _T("loop"), 0, TYPE_VOID, 0, 0,
		morphbybone_ring, _T("ring"), 0, TYPE_VOID, 0, 0,

		morphbybone_createmorph, _T("createMorph"), 0, TYPE_VOID, 0, 1,
			_T("node"), 0, TYPE_INODE,
		morphbybone_removemorph, _T("removeMorph"), 0, TYPE_VOID, 0, 2,
			_T("node"), 0, TYPE_INODE,
			_T("morphName"), 0, TYPE_STRING,
		morphbybone_editmorph, _T("editMorph"), 0, TYPE_VOID, 0, 1,
			_T("edit"), 0, TYPE_BOOL,

		morphbybone_clearvertices, _T("clearSelectedVertices"), 0, TYPE_VOID, 0, 0,
		morphbybone_deletevertices, _T("deleteSelectedVertices"), 0, TYPE_VOID, 0, 0,

		morphbybone_resetorientation, _T("resetOrientation"), 0, TYPE_VOID, 0, 2,
			_T("node"), 0, TYPE_INODE,
			_T("morphName"), 0, TYPE_STRING,

		morphbybone_reloadtarget, _T("reloadTarget"), 0, TYPE_VOID, 0, 2,
			_T("node"), 0, TYPE_INODE,
			_T("morphName"), 0, TYPE_STRING,

		morphbybone_mirrorpaste, _T("mirrorPaste"), 0, TYPE_VOID, 0, 1,
			_T("node"), 0, TYPE_INODE,

		morphbybone_editfalloffgraph, _T("editFalloffGraph"), 0, TYPE_VOID, 0, 2,
			_T("node"), 0, TYPE_INODE,
			_T("morphName"), 0, TYPE_STRING,

		morphbybone_setexternalnode, _T("setExternalNode"), 0, TYPE_VOID, 0, 3,
			_T("morphnode"), 0, TYPE_INODE,
			_T("morphName"), 0, TYPE_STRING,
			_T("externalnode"), 0, TYPE_INODE,

		morphbybone_moveverts, _T("moveVerts"), 0, TYPE_VOID, 0, 1,
			_T("vec"), 0, TYPE_POINT3,

		morphbybone_transformverts, _T("transformVerts"), 0, TYPE_VOID, 0, 2,
			_T("tmMotion"), 0, TYPE_MATRIX3,
			_T("tmToLocalSpace"), 0, TYPE_MATRIX3,

		morphbybone_numberofbones, _T("numberOfBones"), 0, TYPE_INT, 0, 0,

		morphbybone_bonegetinitialnodetm, _T("boneGetInitialNodeTM"), 0, TYPE_MATRIX3, 0, 1,
			_T("node"), 0, TYPE_INODE,
		morphbybone_bonesetinitialnodetm, _T("boneSetInitialNodeTM"), 0, TYPE_VOID, 0, 2,
			_T("node"), 0, TYPE_INODE,
			_T("tm"), 0, TYPE_MATRIX3,


		morphbybone_bonegetinitialobjecttm, _T("boneGetInitialObjectTM"), 0, TYPE_MATRIX3, 0, 1,
			_T("node"), 0, TYPE_INODE,
		morphbybone_bonesetinitialobjecttm, _T("boneSetInitialObjectTM"), 0, TYPE_VOID, 0, 2,
			_T("node"), 0, TYPE_INODE,
			_T("tm"), 0, TYPE_MATRIX3,

		morphbybone_bonegetinitialparenttm, _T("boneGetInitialParentTM"), 0, TYPE_MATRIX3, 0, 1,
			_T("node"), 0, TYPE_INODE,
		morphbybone_bonesetinitialparenttm, _T("boneSetInitialParentTM"), 0, TYPE_VOID, 0, 2,
			_T("node"), 0, TYPE_INODE,
			_T("tm"), 0, TYPE_MATRIX3,

		morphbybone_bonegetnumberofmorphs, _T("boneGetNumberOfMorphs"), 0, TYPE_INT, 0, 1,
			_T("node"), 0, TYPE_INODE,

		morphbybone_bonegetmorphname, _T("boneGetMorphName"), 0, TYPE_STRING, 0, 2,
			_T("node"), 0, TYPE_INODE,
			_T("index"), 0, TYPE_INT,
		morphbybone_bonesetmorphname, _T("boneSetMorphName"), 0, TYPE_VOID, 0, 3,
			_T("node"), 0, TYPE_INODE,
			_T("index"), 0, TYPE_INT,
			_T("name"), 0, TYPE_STRING,

		morphbybone_bonegetmorphangle, _T("boneGetMorphAngle"), 0, TYPE_FLOAT, 0, 2,
			_T("node"), 0, TYPE_INODE,
			_T("index"), 0, TYPE_INT,
		morphbybone_bonesetmorphangle, _T("boneSetMorphAngle"), 0, TYPE_VOID, 0, 3,
			_T("node"), 0, TYPE_INODE,
			_T("index"), 0, TYPE_INT,
			_T("angle"), 0, TYPE_FLOAT,


		morphbybone_bonegetmorphtm, _T("boneGetMorphTM"), 0, TYPE_MATRIX3, 0, 2,
			_T("node"), 0, TYPE_INODE,
			_T("index"), 0, TYPE_INT,
		morphbybone_bonesetmorphtm, _T("boneSetMorphTM"), 0, TYPE_VOID, 0, 3,
			_T("node"), 0, TYPE_INODE,
			_T("index"), 0, TYPE_INT,
			_T("tm"), 0, TYPE_MATRIX3,

		morphbybone_bonegetmorphparenttm, _T("boneGetMorphParentTM"), 0, TYPE_MATRIX3, 0, 2,
			_T("node"), 0, TYPE_INODE,
			_T("index"), 0, TYPE_INT,
		morphbybone_bonesetmorphparenttm, _T("boneSetMorphParentTM"), 0, TYPE_VOID, 0, 3,
			_T("node"), 0, TYPE_INODE,
			_T("index"), 0, TYPE_INT,
			_T("tm"), 0, TYPE_MATRIX3,

		morphbybone_bonegetmorphisdead, _T("boneGetMorphIsDead"), 0, TYPE_BOOL, 0, 2,
			_T("node"), 0, TYPE_INODE,
			_T("index"), 0, TYPE_INT,
		morphbybone_bonegetmorphsetdead, _T("boneSetMorphSetDead"), 0, TYPE_VOID, 0, 3,
			_T("node"), 0, TYPE_INODE,
			_T("index"), 0, TYPE_INT,
			_T("dead"), 0, TYPE_BOOL,

		morphbybone_bonegetmorphnumpoints, _T("boneGetMorphNumPoints"), 0, TYPE_BOOL, 0, 2,
			_T("node"), 0, TYPE_INODE,
			_T("index"), 0, TYPE_INT,
		morphbybone_bonesetmorphnumpoints, _T("boneSetMorphNumPoints"), 0, TYPE_VOID, 0, 3,
			_T("node"), 0, TYPE_INODE,
			_T("index"), 0, TYPE_INT,
			_T("numPoints"), 0, TYPE_INT,


		morphbybone_bonegetmorphvertid, _T("boneGetMorphVertID"), 0, TYPE_INT, 0, 3,
			_T("node"), 0, TYPE_INODE,
			_T("morphIndex"), 0, TYPE_INT,
			_T("ithVertIndex"), 0, TYPE_INT,
		morphbybone_bonegetmorphvertid, _T("boneSetMorphVertID"), 0, TYPE_VOID, 0, 4,
			_T("node"), 0, TYPE_INODE,
			_T("morphIndex"), 0, TYPE_INT,
			_T("ithVertIndex"), 0, TYPE_INT,
			_T("index"), 0, TYPE_INT,


		morphbybone_bonegetmorphvec, _T("boneGetMorphVec"), 0, TYPE_POINT3, 0, 3,
			_T("node"), 0, TYPE_INODE,
			_T("morphIndex"), 0, TYPE_INT,
			_T("ithVertIndex"), 0, TYPE_INT,
		morphbybone_bonesetmorphvec, _T("boneSetMorphVec"), 0, TYPE_VOID, 0, 4,
			_T("node"), 0, TYPE_INODE,
			_T("morphIndex"), 0, TYPE_INT,
			_T("ithVertIndex"), 0, TYPE_INT,
			_T("vec"), 0, TYPE_POINT3,

		morphbybone_bonegetmorphpvec, _T("boneGetMorphVecInParentSpace"), 0, TYPE_POINT3, 0, 3,
			_T("node"), 0, TYPE_INODE,
			_T("morphIndex"), 0, TYPE_INT,
			_T("ithVertIndex"), 0, TYPE_INT,
		morphbybone_bonesetmorphpvec, _T("boneSetMorphVecInParentSpace"), 0, TYPE_VOID, 0, 4,
			_T("node"), 0, TYPE_INODE,
			_T("morphIndex"), 0, TYPE_INT,
			_T("ithVertIndex"), 0, TYPE_INT,
			_T("vec"), 0, TYPE_POINT3,


		morphbybone_bonegetmorphop, _T("boneGetMorphBasePoint"), 0, TYPE_POINT3, 0, 3,
			_T("node"), 0, TYPE_INODE,
			_T("morphIndex"), 0, TYPE_INT,
			_T("ithVertIndex"), 0, TYPE_INT,
		morphbybone_bonesetmorphop, _T("boneSetMorphBasePoint"), 0, TYPE_VOID, 0, 4,
			_T("node"), 0, TYPE_INODE,
			_T("morphIndex"), 0, TYPE_INT,
			_T("ithVertIndex"), 0, TYPE_INT,
			_T("point"), 0, TYPE_POINT3,


		morphbybone_bonegetmorphowner, _T("boneGetMorphOwner"), 0, TYPE_INODE, 0, 3,
			_T("node"), 0, TYPE_INODE,
			_T("morphIndex"), 0, TYPE_INT,
			_T("ithVertIndex"), 0, TYPE_INT,
		morphbybone_bonesetmorphowner, _T("boneSetMorphOwner"), 0, TYPE_VOID, 0, 4,
			_T("node"), 0, TYPE_INODE,
			_T("morphIndex"), 0, TYPE_INT,
			_T("ithVertIndex"), 0, TYPE_INT,
			_T("node"), 0, TYPE_INODE,


		morphbybone_bonegetmorphfalloff, _T("boneGetMorphFalloff"), 0, TYPE_INT, 0, 2,
			_T("node"), 0, TYPE_INODE,
			_T("morphIndex"), 0, TYPE_INT,
		morphbybone_bonesetmorphfalloff, _T("boneSetMorphFalloff"), 0, TYPE_VOID, 0, 3,
			_T("node"), 0, TYPE_INODE,
			_T("morphIndex"), 0, TYPE_INT,
			_T("falloff"), 0, TYPE_INT,

		morphbybone_update, _T("update"), 0, TYPE_VOID, 0, 0,

		morphbybone_getweight, _T("getWeight"), 0, TYPE_FLOAT, 0, 2,
			_T("node"), 0, TYPE_INODE,
			_T("morphName"), 0, TYPE_STRING,

		morphbybone_bonegetmorphenabled, _T("boneGetMorphEnabled"), 0, TYPE_BOOL, 0, 2,
			_T("node"), 0, TYPE_INODE,
			_T("morphIndex"), 0, TYPE_INT,
		morphbybone_bonesetmorphenabled, _T("boneSetMorphEnabled"), 0, TYPE_VOID, 0, 3,
			_T("node"), 0, TYPE_INODE,
			_T("morphIndex"), 0, TYPE_INT,
			_T("enabled"), 0, TYPE_BOOL,

      end
      );


void *MorphByBoneClassDesc::Create(BOOL loading)
		{
		AddInterface(&morphbybone_interface);
		return new MorphByBone;
		}

FPInterfaceDesc* IMorphByBone::GetDesc()
	{
	 return &morphbybone_interface;
	}

MorphByBone::MorphByBone()
{
	suspendUI = FALSE;
	currentBone = -1;
	currentMorph = -1;
	editMorph = FALSE;
	pauseAccessor = FALSE;
	pasteMode = FALSE;
	pblock = NULL;
	MorphByBoneDesc.MakeAutoParamBlocks(this);

	curveCtl = (ICurveCtl *) CreateInstance(REF_MAKER_CLASS_ID,CURVE_CONTROL_CLASS_ID);
	pblock->SetValue(pb_selectiongraph,0,curveCtl);

	curveCtl->SetXRange(0.0f,1.0f);
	curveCtl->SetYRange(-1.2f,1.2f);

 	curveCtl->RegisterResourceMaker((ReferenceMaker *)this);

	curveCtl->SetNumCurves(1);
	curveCtl->SetZoomValues(110.0f,70.4f);
	curveCtl->SetScrollValues(-14,-34);

	BitArray ba;
	ba.SetSize(1);
	ba.Set(0);//ba.Set(0);ba.Set(0);ba.Set(3);
	curveCtl->SetDisplayMode(ba);
	
	DWORD flags = CC_NONE;
	
	flags |=  CC_DRAWBG ;
	flags |=  CC_DRAWGRID ;
	flags |=  CC_DRAWUTOOLBAR ;
	flags |=  CC_AUTOSCROLL ;
	
	
	flags |=  CC_RCMENU_MOVE_XY ;
	flags |=  CC_RCMENU_MOVE_X ;
	flags |=  CC_RCMENU_MOVE_Y ;
	flags |=  CC_RCMENU_SCALE ;
	flags |=  CC_RCMENU_INSERT_CORNER ;
	flags |=  CC_RCMENU_INSERT_BEZIER ;
	flags |=  CC_RCMENU_DELETE ;

	curveCtl->SetCCFlags(flags);

	ICurve *pCurve = NULL;
	
	pCurve = curveCtl->GetControlCurve(0);

	CurvePoint pt;

	pt.flags = CURVEP_CORNER | CURVEP_BEZIER | CURVEP_ENDPOINT;
	pt.out.x = 0.33f;
	pt.out.y = 0.0f;
	pt.in.x = -0.33f;
	pt.in.y = 0.0f;

	pt.p.x = 0.0f;			
	pt.p.y = 1.0f;			
	pCurve->SetPoint(0,0,&pt);
		
	pt.p.x = 1.0f;			
	pt.p.y = 0.0f;			
	pt.in.x = -.33f;
	pt.in.y = 0.0f;
	pt.out.x = .33f;
	pt.out.y = 0.0f;

	pCurve->SetPoint(0,1,&pt);
	pCurve->SetLookupTableSize(100);

}


void MorphByBone::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);	


	moveMode    = new MoveModBoxCMode(this,ip);

	rotMode = new RotateModBoxCMode(this,ip);
	uscaleMode     = new UScaleModBoxCMode(this,ip);
	nuscaleMode    = new NUScaleModBoxCMode(this,ip);
	squashMode     = new SquashModBoxCMode(this,ip);
	selectMode     = new SelectModBoxCMode(this,ip);



	MorphByBoneDesc.BeginEditParams(ip, this, flags, prev);
	morphbybone_param_blk.SetUserDlgProc(morphbybone_mapparams, new MorphByBoneParamsMapDlgProc(this));
	morphbybone_param_blk.SetUserDlgProc(morphbybone_mapparamsprop, new MorphByBoneParamsMapDlgProcProp(this));
	morphbybone_param_blk.SetUserDlgProc(morphbybone_mapparamscopy, new MorphByBoneParamsMapDlgProcCopy(this));

	morphbybone_param_blk.SetUserDlgProc(morphbybone_mapparamsselection, new MorphByBoneParamsMapDlgProcSoftSelection(this));


}


void MorphByBone::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	editMorph = FALSE;
	MorphByBoneDesc.EndEditParams(ip, this, flags, next);

	TimeValue t = ip->GetTime();
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

	ip->ClearPickMode();

	ip->DeleteMode(moveMode);	
	if (moveMode) delete moveMode;
	moveMode = NULL;	

	ip->DeleteMode(rotMode);	
	if (rotMode) delete rotMode;
	rotMode = NULL;

	ip->DeleteMode(uscaleMode);
	ip->DeleteMode(nuscaleMode);
	ip->DeleteMode(squashMode);
	ip->DeleteMode(selectMode);

	if (uscaleMode) delete uscaleMode;
	uscaleMode = NULL;
	if (nuscaleMode) delete nuscaleMode;
	nuscaleMode = NULL;
	if (squashMode) delete squashMode;
	squashMode = NULL;
	if (selectMode) delete selectMode;
	selectMode = NULL;


	ReleaseICustButton(iEditButton);
	iEditButton = NULL;

	ReleaseICustButton(iNodeButton);
	iNodeButton = NULL;

	ReleaseICustButton(iPickBoneButton);
	iPickBoneButton = NULL;

	ReleaseICustButton(iMultiSelectButton);
	iMultiSelectButton = NULL;


	ReleaseICustButton(iDeleteMorphButton);
	iDeleteMorphButton = NULL;

	ReleaseICustButton(iResetMorphButton);
	iResetMorphButton = NULL;

	ReleaseICustButton(iClearVertsButton);
	iClearVertsButton = NULL;

	ReleaseICustButton(iRemoveVertsButton);
	iRemoveVertsButton = NULL;

	ReleaseICustButton(iGraphButton);
	iGraphButton = NULL;

	ReleaseICustEdit(iNameField);
	iNameField = NULL;

	ReleaseISpinner(iInfluenceAngleSpinner);
	iInfluenceAngleSpinner = NULL;

	


	this->ip = NULL;

	//Defect 727409 If the user turn off the skin morph modifier, we need to get the 
	//localDataList for using the copy/paste mirror option.   
	if(!TestAFlag(A_MOD_DISABLED))
	{
		for (int i = 0; i < localDataList.Count(); i++)
		{
			if (localDataList[i])
				localDataList[i]->FreeConnectionList();
		}
		localDataList.ZeroCount();
	}
	

}



RefResult MorphByBone::NotifyRefChanged(
										Interval changeInt, RefTargetHandle hTarget,
										PartID& partID,  RefMessage message) 
{
	//TODO: Add code to handle the various reference changed messages
	switch (message) 
	{
	//watje Defect 683932 we need to prevent this from propogating since we only are dependant on the tm and this
	//can cause an objects cache to get flushed and crash the renderer.
	case REFMSG_OBJECT_CACHE_DUMPED:
		return REF_STOP;
		break;
	case REFMSG_CHANGE:
		// see if this message came from a changing parameter in the pblock,
		// if so, limit rollout update to the changing item and update any active viewport texture
		if (hTarget == pblock)
		{
			ParamID changing_param = pblock->LastNotifyParamID();

			morphbybone_param_blk.InvalidateUI(changing_param);
			if ( (changing_param == pb_usesoftselection) ||
				 (changing_param == pb_selectionradius) ||
				 (changing_param == pb_useedgelimit) ||
				 (changing_param == pb_edgelimit) ||
				 (changing_param == pb_selectiongraph) )
			{
				for (int i = 0; i < localDataList.Count(); i++)
				{
					if (localDataList[i])
						localDataList[i]->rebuildSoftSelection = TRUE;
				}
				if ( changing_param == pb_selectiongraph )
				{
					NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
					GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
				}

			}

		}

		break;
	}
	return REF_SUCCEED;

}

void MorphByBone::fnAddBone(INode *node)
{
	AddBone(node);
	UpdateLocalUI();
	BuildTreeList();
}

void MorphByBone::fnRemoveBone(INode *node)
{
	int boneIndex = GetBoneIndex(node);
	this->DeleteBone(boneIndex);
	UpdateLocalUI();
	BuildTreeList();
}

void	MorphByBone::fnSelectBone(INode *node,TCHAR* morphName)
{
	int boneIndex = GetBoneIndex(node);
	int morphIndex = GetMorphIndex(node,morphName);
	this->currentBone = boneIndex;
	this->currentMorph = morphIndex;
	UpdateLocalUI();
	BuildTreeList();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

}

INode*	MorphByBone::fnGetSelectedBone()
{
	INode *pnode = GetNode(currentBone);

	return pnode;
}

TCHAR*		MorphByBone::fnGetSelectedMorph()
{
	return GetMorphName(GetNode(currentBone),currentMorph);
}


void	MorphByBone::fnSelectVertices(INode *node,BitArray *sel)
{
	BOOL useSoftSelection;
	pblock->GetValue(pb_usesoftselection,0,useSoftSelection,FOREVER);
	float radius;
	pblock->GetValue(pb_selectionradius,0,radius,FOREVER);
		
	BOOL useEdgeLimit;
	int edgeLimit;
	pblock->GetValue(pb_useedgelimit,0,useEdgeLimit,FOREVER);
	pblock->GetValue(pb_edgelimit,0,edgeLimit,FOREVER);


	for (int i = 0; i < this->localDataList.Count(); i++)
	{
		if (localDataList[i] && (localDataList[i]->selfNode == node))
		{
			for (int j = 0; j < sel->GetSize(); j++)
			{
				if (j < localDataList[i]->softSelection.Count())
				{
					if ((*sel)[j])
						localDataList[i]->softSelection[j] = 1.0f;
					else localDataList[i]->softSelection[j] = 0.0f;
				}
			}
			if (useSoftSelection)
				localDataList[i]->ComputeSoftSelection(radius,pCurve,useEdgeLimit,edgeLimit);

		}
	}
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

BOOL	MorphByBone::fnIsSelectedVertex(INode *node,int index)
{


	index--;
	for (int i = 0; i < this->localDataList.Count(); i++)
	{
		if (localDataList[i] && (localDataList[i]->selfNode == node))
		{
			
			if (index < localDataList[i]->softSelection.Count())
			{
				if (localDataList[i]->softSelection[index] == 1.0f)
					return TRUE;
				else return FALSE;
			}						
		}
	}

	return FALSE;
}


void	MorphByBone::fnResetGraph()
{
	ResetSelectionGraph();
}


void	MorphByBone::fnShrink()
{
	this->GrowVerts(FALSE);
}
void	MorphByBone::fnGrow()
{
	this->GrowVerts(TRUE);
}

void	MorphByBone::fnLoop()
{
	this->EdgeSel(TRUE);
}
void	MorphByBone::fnRing()
{
	this->EdgeSel(FALSE);
}

void	MorphByBone::fnCreateMorph(INode *node)
{
	this->CreateMorph(node);
}

void	MorphByBone::fnRemoveMorph(INode *node, TCHAR *name)
{
	int whichBone = this->GetBoneIndex(node); 
	int whichMorph = this->GetMorphIndex(node,name);
	DeleteMorph(whichBone, whichMorph);
}


void	MorphByBone::fnEdit(BOOL edit)
{
	if (iEditButton)
		iEditButton->SetCheck(edit);

	editMorph = edit;
	if (edit)
		GetCOREInterface()->SetSubObjectLevel(1);
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

}


void	MorphByBone::fnClearSelectedVertices()
{
	ClearSelectedVertices(FALSE);
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

}
void	MorphByBone::fnDeleteSelectedVertices()
{
	ClearSelectedVertices(TRUE);
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

}


void	MorphByBone::fnResetOrientation(INode *node, TCHAR *name)
{
	int whichBone = this->GetBoneIndex(node); 
	int whichMorph = this->GetMorphIndex(node,name);
	ResetMorph(whichBone, whichMorph);
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

void	MorphByBone::fnReloadTarget(INode *node, TCHAR *name)
{
	int whichBone = this->GetBoneIndex(node); 
	int whichMorph = this->GetMorphIndex(node,name);
	ReloadMorph(whichBone, whichMorph);
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

void	MorphByBone::fnMirrorPaste(INode *node)
{
	int whichBone = this->GetBoneIndex(node); 

	int whichMirrorBone = GetMirrorBone();
	this->MirrorMorph(whichBone,whichMirrorBone,TRUE);

	UpdateLocalUI();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

void	MorphByBone::fnEditFalloffGraph(INode *node, TCHAR *name)
{

	int whichBone = this->GetBoneIndex(node); 
	int whichMorph = this->GetMorphIndex(node,name);
	BringUpGraph(whichBone, whichMorph);
}

void	MorphByBone::fnSetExternalNode(INode *node, TCHAR *name,INode *exnode)
{
		int whichBone = this->GetBoneIndex(node); 
		int whichMorph = this->GetMorphIndex(node,name);
		SetNode(exnode,whichBone,whichMorph);
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		UpdateLocalUI();

}

void	MorphByBone::fnMoveVerts(Point3 vec)
{
	SetMorph(vec);
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

}

void	MorphByBone::fnTransFormVerts(Matrix3 a, Matrix3 b)
{
	SetMorph(a,b);
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}


int		MorphByBone::fnNumberOfBones()
{
	return boneData.Count();
}


Matrix3*	MorphByBone::fnBoneGetInitialNodeTM(INode *node)
{
	int whichBone = this->GetBoneIndex(node);
	Matrix3 tm(1);
	if (whichBone < 0) return NULL;
	return &boneData[whichBone]->intialBoneNodeTM;
}
void	MorphByBone::fnBoneSetInitialNodeTM(INode *node, Matrix3 tm)
{
	int whichBone = this->GetBoneIndex(node);
	if (whichBone >= 0)
		boneData[whichBone]->intialBoneNodeTM = tm;

}


Matrix3*	MorphByBone::fnBoneGetInitialObjectTM(INode *node)
{
	int whichBone = this->GetBoneIndex(node);
	Matrix3 tm(1);
	if (whichBone < 0) return NULL;
	return &boneData[whichBone]->intialBoneObjectTM;
}
void	MorphByBone::fnBoneSetInitialObjectTM(INode *node, Matrix3 tm)
{
	int whichBone = this->GetBoneIndex(node);
	if (whichBone >= 0)
		boneData[whichBone]->intialBoneObjectTM = tm;

}

Matrix3*	MorphByBone::fnBoneGetInitialParentTM(INode *node)
{
	int whichBone = this->GetBoneIndex(node);
	Matrix3 tm(1);
	if (whichBone < 0) return NULL;
	return &boneData[whichBone]->intialParentNodeTM;
}
void	MorphByBone::fnBoneSetInitialParentTM(INode *node, Matrix3 tm)
{
	int whichBone = this->GetBoneIndex(node);
	if (whichBone >= 0)
		boneData[whichBone]->intialParentNodeTM = tm;

}


int		MorphByBone::fnBoneGetNumberOfMorphs(INode *node)
{
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return 0;
	return boneData[whichBone]->morphTargets.Count();

}


TCHAR*	MorphByBone::fnBoneGetMorphName(INode *node, int morphIndex)
{
	morphIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return NULL;
	if (whichBone >= boneData.Count()) return NULL;

	if (morphIndex < 0) return NULL;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return NULL;

	return boneData[whichBone]->morphTargets[morphIndex]->name;

}
void	MorphByBone::fnBoneSetMorphName(INode *node, int morphIndex,TCHAR* name)
{
	morphIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return;
	if (whichBone >= boneData.Count()) return;

	if (morphIndex < 0) return;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return;

	boneData[whichBone]->morphTargets[morphIndex]->name = name;
}


float	MorphByBone::fnBoneGetMorphAngle(INode *node, int morphIndex)
{
	morphIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return 0.0f;
	if (whichBone >= boneData.Count()) return 0.0f;

	if (morphIndex < 0) return 0.0f;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return 0.0f;

	return boneData[whichBone]->morphTargets[morphIndex]->influenceAngle * 180.0f/PI;

}
void	MorphByBone::fnBoneSetMorphAngle(INode *node, int morphIndex,float angle)
{
	morphIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return;
	if (whichBone >= boneData.Count()) return;

	if (morphIndex < 0) return;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return;

	boneData[whichBone]->morphTargets[morphIndex]->influenceAngle = angle * PI/180.0f;
}


Matrix3* MorphByBone::fnBoneGetMorphTM(INode *node, int morphIndex)
{
	morphIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return NULL;
	if (whichBone >= boneData.Count()) return NULL;

	if (morphIndex < 0) return NULL;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return NULL;

	return &boneData[whichBone]->morphTargets[morphIndex]->boneTMInParentSpace;
}
void	 MorphByBone::fnBoneSetMorphTM(INode *node, int morphIndex,Matrix3 tm)
{
	morphIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return;
	if (whichBone >= boneData.Count()) return;

	if (morphIndex < 0) return;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return;

	boneData[whichBone]->morphTargets[morphIndex]->boneTMInParentSpace = tm;
}

Matrix3* MorphByBone::fnBoneGetMorphParentTM(INode *node, int morphIndex)
{
	morphIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return NULL;
	if (whichBone >= boneData.Count()) return NULL;

	if (morphIndex < 0) return NULL;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return NULL;

	return &boneData[whichBone]->morphTargets[morphIndex]->parentTM;
}
void	 MorphByBone::fnBoneSetMorphParentTM(INode *node, int morphIndex,Matrix3 tm)
{
	morphIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return;
	if (whichBone >= boneData.Count()) return;

	if (morphIndex < 0) return;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return;

	boneData[whichBone]->morphTargets[morphIndex]->parentTM = tm;
}



BOOL	 MorphByBone::fnBoneGetMorphIsDead(INode *node, int morphIndex)
{
	morphIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return TRUE;
	if (whichBone >= boneData.Count()) return TRUE;

	if (morphIndex < 0) return TRUE;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return TRUE;

	return boneData[whichBone]->morphTargets[morphIndex]->IsDead();
}
void	 MorphByBone::fnBoneSetMorphSetDead(INode *node, int morphIndex,BOOL dead)
{
	morphIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return;
	if (whichBone >= boneData.Count()) return;

	if (morphIndex < 0) return;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return;

	boneData[whichBone]->morphTargets[morphIndex]->SetDead(dead);
}

int		 MorphByBone::fnBoneGetMorphNumPoints(INode *node, int morphIndex)
{
	morphIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return 0;
	if (whichBone >= boneData.Count()) return 0;

	if (morphIndex < 0) return 0;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return 0;

	return boneData[whichBone]->morphTargets[morphIndex]->d.Count();
}

void	 MorphByBone::fnBoneSetMorphNumPoints(INode *node, int morphIndex, int numberPoints)
{
	morphIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return;
	if (whichBone >= boneData.Count()) return;

	if (morphIndex < 0) return;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return;

	boneData[whichBone]->morphTargets[morphIndex]->d.SetCount(numberPoints);
}

int		 MorphByBone::fnBoneGetMorphVertID(INode *node, int morphIndex, int ithIndex)
{
	morphIndex--;
	ithIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return -1;
	if (whichBone >= boneData.Count()) return -1;

	if (morphIndex < 0) return -1;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return -1;

	if (ithIndex < 0) return -1;
	if (ithIndex >= boneData[whichBone]->morphTargets[morphIndex]->d.Count()) return -1;

	return boneData[whichBone]->morphTargets[morphIndex]->d[ithIndex].vertexID+1;
}
void	 MorphByBone::fnBoneSetMorphVertID(INode *node, int morphIndex, int ithIndex, int vertIndex)
{
	morphIndex--;
	ithIndex--;
	vertIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return ;
	if (whichBone >= boneData.Count()) return ;

	if (morphIndex < 0) return ;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return ;

	if (ithIndex < 0) return ;
	if (ithIndex >= boneData[whichBone]->morphTargets[morphIndex]->d.Count()) return ;

	boneData[whichBone]->morphTargets[morphIndex]->d[ithIndex].vertexID = vertIndex;
}

Point3*	 MorphByBone::fnBoneGetMorphVec(INode *node, int morphIndex, int ithIndex)
{
	morphIndex--;
	ithIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return NULL;
	if (whichBone >= boneData.Count()) return NULL;

	if (morphIndex < 0) return NULL;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return NULL;

	if (ithIndex < 0) return NULL;
	if (ithIndex >= boneData[whichBone]->morphTargets[morphIndex]->d.Count()) return NULL;

	return &boneData[whichBone]->morphTargets[morphIndex]->d[ithIndex].vec;
}
void	 MorphByBone::fnBoneSetMorphVec(INode *node, int morphIndex, int ithIndex, Point3 vec)
{
	morphIndex--;
	ithIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return ;
	if (whichBone >= boneData.Count()) return ;

	if (morphIndex < 0) return ;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return ;

	if (ithIndex < 0) return ;
	if (ithIndex >= boneData[whichBone]->morphTargets[morphIndex]->d.Count()) return ;

	boneData[whichBone]->morphTargets[morphIndex]->d[ithIndex].vec = vec;
}

Point3*	 MorphByBone::fnBoneGetMorphPVec(INode *node, int morphIndex, int ithIndex)
{
	morphIndex--;
	ithIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return NULL;
	if (whichBone >= boneData.Count()) return NULL;

	if (morphIndex < 0) return NULL;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return NULL;

	if (ithIndex < 0) return NULL;
	if (ithIndex >= boneData[whichBone]->morphTargets[morphIndex]->d.Count()) return NULL;

	return &boneData[whichBone]->morphTargets[morphIndex]->d[ithIndex].vecParentSpace;
}
void	 MorphByBone::fnBoneSetMorphPVec(INode *node, int morphIndex, int ithIndex, Point3 vec)
{
	morphIndex--;
	ithIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return ;
	if (whichBone >= boneData.Count()) return ;

	if (morphIndex < 0) return ;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return ;

	if (ithIndex < 0) return ;
	if (ithIndex >= boneData[whichBone]->morphTargets[morphIndex]->d.Count()) return ;

	boneData[whichBone]->morphTargets[morphIndex]->d[ithIndex].vecParentSpace = vec;
}

Point3*	 MorphByBone::fnBoneGetMorphOP(INode *node, int morphIndex, int ithIndex)
{
	morphIndex--;
	ithIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return NULL;
	if (whichBone >= boneData.Count()) return NULL;

	if (morphIndex < 0) return NULL;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return NULL;

	if (ithIndex < 0) return NULL;
	if (ithIndex >= boneData[whichBone]->morphTargets[morphIndex]->d.Count()) return NULL;

	return &boneData[whichBone]->morphTargets[morphIndex]->d[ithIndex].originalPt;
}
void	 MorphByBone::fnBoneSetMorphOP(INode *node, int morphIndex, int ithIndex, Point3 vec)
{
	morphIndex--;
	ithIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return ;
	if (whichBone >= boneData.Count()) return ;

	if (morphIndex < 0) return ;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return ;

	if (ithIndex < 0) return ;
	if (ithIndex >= boneData[whichBone]->morphTargets[morphIndex]->d.Count()) return ;

	boneData[whichBone]->morphTargets[morphIndex]->d[ithIndex].originalPt = vec;
}

INode*	 MorphByBone::fnBoneGetMorphOwner(INode *node, int morphIndex, int ithIndex)
{
	morphIndex--;
	ithIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return NULL;
	if (whichBone >= boneData.Count()) return NULL;

	if (morphIndex < 0) return NULL;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return NULL;

	if (ithIndex < 0) return NULL;
	if (ithIndex >= boneData[whichBone]->morphTargets[morphIndex]->d.Count()) return NULL;

	int id = boneData[whichBone]->morphTargets[morphIndex]->d[ithIndex].localOwner;
	return localDataList[id]->selfNode;
}
void	 MorphByBone::fnBoneSetMorphOwner(INode *node, int morphIndex, int ithIndex, INode *onode)
{
	morphIndex--;
	ithIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return ;
	if (whichBone >= boneData.Count()) return ;

	if (morphIndex < 0) return ;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return ;

	if (ithIndex < 0) return ;
	if (ithIndex >= boneData[whichBone]->morphTargets[morphIndex]->d.Count()) return ;

	int id = boneData[whichBone]->morphTargets[morphIndex]->d[ithIndex].localOwner;
	
	for (int i = 0; i < localDataList.Count(); i++)
	{
		if (localDataList[i]->selfNode == onode)
			boneData[whichBone]->morphTargets[morphIndex]->d[ithIndex].localOwner = i;
	}
}


int		 MorphByBone::fnBoneGetMorphFalloff(INode *node, int morphIndex)
{
	morphIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return -1;
	if (whichBone >= boneData.Count()) return -1;

	if (morphIndex < 0) return -1;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return -1;

	return boneData[whichBone]->morphTargets[morphIndex]->falloff;

}

void	 MorphByBone::fnBoneSetMorphFalloff(INode *node, int morphIndex, int falloff)
{
	morphIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return;
	if (whichBone >= boneData.Count()) return;

	if (morphIndex < 0) return;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return;

	boneData[whichBone]->morphTargets[morphIndex]->falloff = falloff;
}

int		 MorphByBone::fnBoneGetJointType(INode *node)
{
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return -1;
	if (whichBone >= boneData.Count()) return -1;
	if (boneData[whichBone]->IsBallJoint())
		return 0;
	else
		return 1;
}

void	 MorphByBone::fnBoneSetJointType(INode *node, int jointType)
{
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return;
	if (whichBone >= boneData.Count()) return;
	if (jointType == 0)
		boneData[whichBone]->SetBallJoint();
	else boneData[whichBone]->SetPlanarJoint();
		

}

void	  MorphByBone::fnUpdate()
{
	UpdateLocalUI();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}


float	MorphByBone::fnGetWeight(INode *node, TCHAR*name)
{
	int whichBone = this->GetBoneIndex(node); 
	int whichMorph = this->GetMorphIndex(node,name);

	if ((whichBone <0) || (whichBone >= boneData.Count())) return 0.0f;
	if ((whichMorph <0) || (whichMorph >= boneData[whichBone]->morphTargets.Count())) return 0.0f;

	BOOL ballJoint = boneData[whichBone]->IsBallJoint();

	MorphTargets *morph = boneData[whichBone]->morphTargets[whichMorph];
	if (morph == NULL) return 0.0f;
	return morph->weight;

}
//		FN_2(morphbybone_getweight, TYPE_FLOAT, fnGetWeight,TYPE_INODE,TYPE_STRING);

BOOL	 MorphByBone::fnBoneGetMorphEnabled(INode *node, int morphIndex)
{
	morphIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return FALSE;
	if (whichBone >= boneData.Count()) return FALSE;

	if (morphIndex < 0) return FALSE;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return FALSE;

	return (!boneData[whichBone]->morphTargets[morphIndex]->IsDisabled());
}

void	 MorphByBone::fnBoneSetMorphEnabled(INode *node, int morphIndex, BOOL enabled)
{
	morphIndex--;
	int whichBone = this->GetBoneIndex(node);
	
	if (whichBone < 0) return;
	if (whichBone >= boneData.Count()) return;

	if (morphIndex < 0) return;
	if (morphIndex >= boneData[whichBone]->morphTargets.Count()) return;

	boneData[whichBone]->morphTargets[morphIndex]->SetDisabled(!enabled);
}


