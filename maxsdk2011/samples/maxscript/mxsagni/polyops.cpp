#include "MAXScrpt.h"
#include "MAXObj.h"
#include "modstack.h"
#include "iEPoly.h"
#include "XREF\IXREFITEM.H"

#include "resource.h"

#include <maxscript\macros\define_instantiation_functions.h>

static EPoly*
setup_polyops(Value* obj)
{
	// make sure we are in Modifier panel, and obj is current and
	// get IPatchOps interface
	BaseInterface* ip;	
	if (MAXScript_interface->GetCommandPanelTaskMode() == TASK_MODE_MODIFY)
	{
		deletion_check((MAXWrapper*)obj);
		if (is_node(obj))
		{
			INode* node = obj->to_node();
			Object* obj = Get_Object_Or_XRef_BaseObject(node->GetObjectRef());
			if ((BaseObject*)obj != MAXScript_interface->GetCurEditObject())
				throw RuntimeError (GetString(IDS_EDITABLEPOLY_NOT_ACTIVE), obj->GetObjectName());
			if (obj->GetInterface(IID_XREF_ITEM))
				throw RuntimeError (GetString(IDS_POLYOP_ON_NON_POLY), obj->GetObjectName());
			if ((ip = ((Animatable*)obj)->GetInterface(EPOLY_INTERFACE)) == NULL)
				throw RuntimeError (GetString(IDS_POLYOP_ON_NON_POLY), obj->GetObjectName());
			return (EPoly*)ip;
		}
	}
	return NULL;
}


#define def_poly_mode(_cfn, _op, _socode)						\
	Value*														\
	_cfn##_cf(Value** arg_list, int count)						\
	{															\
		check_arg_count(_op, 1, count);							\
		EPoly* po = setup_polyops(arg_list[0]);					\
		if (po) po->EpActionToggleCommandMode(_socode);			\
		return &ok;												\
	}															\
	def_struct_primitive( _cfn,		polyOps, _T(#_op));

#define def_poly_op(_cfn, _op, _socode)							\
	Value*														\
	_cfn##_cf(Value** arg_list, int count)						\
	{															\
		check_arg_count(_op, 1, count);							\
		EPoly* po = setup_polyops(arg_list[0]);					\
		if (po) po->EpActionButtonOp(_socode);					\
		return &ok;												\
	}															\
	def_struct_primitive( _cfn,		polyOps, _T(#_op));



// Editable Poly Button operations:
// (Use with EpActionButtonOp)

def_poly_op( po_hide,				hide,				epop_hide )
def_poly_op( po_unhide,				unhide,				epop_unhide )
def_poly_op( po_nsCopy,				NamedSelCopy,		epop_ns_copy )
def_poly_op( po_nsPaste,			NamedSelPaste,		epop_ns_paste )
def_poly_op( po_cap,				cap,				epop_cap )
def_poly_op( po_delete,				delete,				epop_delete )
def_poly_op( po_detach,				detach,				epop_detach )
def_poly_op( po_attachList,			attachList,			epop_attach_list )
def_poly_op( po_split,				split,				epop_split )
def_poly_op( po_break,				break,				epop_break )
def_poly_op( po_collapse,			collapse,			epop_collapse )
def_poly_op( po_resetPlane,			resetPlane,			epop_reset_plane )
def_poly_op( po_slice,				slice,				epop_slice )
def_poly_op( po_weldSel,			weld,				epop_weld_sel )
def_poly_op( po_createShape,		createShapeFromEdges,	epop_create_shape )
def_poly_op( po_makePlanar,			makePlanar,			epop_make_planar )
def_poly_op( po_alignGrid,			gridAlign,			epop_align_grid )
def_poly_op( po_alignView,			viewAlign,			epop_align_view )
def_poly_op( po_removeIsoVerts,		removeIsolatedVerts,epop_remove_iso_verts )
def_poly_op( po_meshsmooth,			meshsmooth,			epop_meshsmooth )
def_poly_op( po_tessellate,			tessellate,			epop_tessellate )
def_poly_op( po_update,				update,				epop_update )
def_poly_op( po_selbyVC,			selectByColor,		epop_selby_vc )
def_poly_op( po_retriangulate,		retriangulate,		epop_retriangulate )
def_poly_op( po_flipNormals,		flipNormal,			epop_flip_normals )
def_poly_op( po_selbyMatid,			selectByID,			epop_selby_matid )
def_poly_op( po_selbySmg,			selectBySG,			epop_selby_smg )
def_poly_op( po_autosmooth,			autosmooth,			epop_autosmooth )
def_poly_op( po_clearSmg,			clearAllSG,			epop_clear_smg )

// Editable Poly Command modes:
// (Use with EpActionToggleCommandMode.)
def_poly_mode( po_createVertex,		startCreateVertex,	epmode_create_vertex )
def_poly_mode( po_createEdge,		startCreateEdge,	epmode_create_edge )
def_poly_mode( po_createFace,		startCreateFace,	epmode_create_face )
def_poly_mode( po_divideEdge,		startDivideEdge,	epmode_divide_edge )
def_poly_mode( po_divideFace,		startDivideFace,	epmode_divide_face )
def_poly_mode( po_extrudeVertex,	startExtrudeVertex,	epmode_extrude_vertex )
def_poly_mode( po_extrudeEdge,		startExtrudeEdge,	epmode_extrude_edge )
def_poly_mode( po_extrudeFace,		startExtrudeFace,	epmode_extrude_face )
def_poly_mode( po_chamferVertex,	startChamferVertex,	epmode_chamfer_vertex )
def_poly_mode( po_chamferEdge,		startChamferEdge,	epmode_chamfer_edge )
def_poly_mode( po_bevel,			startBevel,			epmode_bevel )
def_poly_mode( po_sliceplane,		startSlicePlane,	epmode_sliceplane )
def_poly_mode( po_cutVertex,		startCutVertex,		epmode_cut_vertex )
def_poly_mode( po_cutEdge,			startCutEdge,		epmode_cut_edge )
def_poly_mode( po_cutFace,			startCutFace,		epmode_cut_face )
def_poly_mode( po_weld,				startWeldTarget,	epmode_weld )
def_poly_mode( po_editTri,			startEditTri,		epmode_edit_tri )


