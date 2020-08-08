/*	mouseTrack() function implementation
 *
 *	Adds a new function, mouseTrack(), that can be used to script mouse-driven
 *  interaction in MAX.  It implements a new MAX command mode that does the 
 *  mouse tracking.  Basic operation allows the specifying of a callback
 *  function that is called as the mouse is manipulated and, in particular, 
 *  can be conditioned to track over the surface of a mesh giving back normal, face number, 
 *  etc., for implementing MAYA Artisan-like features.
 *
 *  John Wainwright, 1998
 */
 
#include "MAXScrpt.h"
#include "Numbers.h"
#include "3DMath.h"
#include "MAXObj.h"
#include "resource.h"
#include <streams.h>
#include <mouseman.h>
#include <ref.h>

// define the new primitives using macros from SDK
#include <maxscript\macros\define_instantiation_functions.h>
	def_visible_primitive( mouse_track,		"mouseTrack");

/* ------------ trackMouse function Command Mode class ----------------*/

BOOL end_track_mouse;

// this struct maps the value local array kept in mouseTrack() for gc() purposes
typedef struct
{
	Value* result;			// trackMouse result
	Value* node_val;		// the node we are currently tracking over (if any)
	Value* ray_val;			// ray at current tracking point (on grid or node surface)
	Value* face_num_val;	// the face containing the tracking point
	Value* face_bary;		// the barycentric coord in the face
} tmvl;

// the mouseTrack MouseCallBack class - used by the mouseTrack command mode class
class TrackMouseCallBack : public MouseCallBack, ReferenceMaker
{
	public:
		tmvl*  vl;							// track mouse value local struct (kept by trackMouse() for collector to see)
		Value* snap_mode;					// #2D or 3D
		NodeTab nodeTab;					// node(s) we are tracking on
		INode* hit_node;					// intersect node
		int	   face_num;
		Ray	   r;							// intersect ray
		Value* track_callback;				// mouse track callback fn
		Value* track_callback_arg;			// mouse track callback arg
		BOOL   ref_deleted;					// flag set if node is deleted under me

		TrackMouseCallBack() {}
		int proc(HWND hwnd, int msg, int point, int flag, IPoint2 m);
		BOOL point_on_obj(ViewExp *vpt, IPoint2 m, Point3& pt, Point3 &norm);
		void draw_marker(ViewExp* vpx, Point3 p, Point3 norm);
		void start_track();
		void end_track();

		// ReferenceMaker
		int			NumRefs() { return nodeTab.Count(); }
		RefTargetHandle GetReference(int i) { return nodeTab[i]; }
		void		SetReference(int i, RefTargetHandle rtarg) { nodeTab[i] = (INode*)rtarg; }
		RefResult	NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message);
};

#define TRACK_MOUSE_COMMAND	3247
#define CID_TRACK_MOUSE		CID_USER + 25634

// the mosueTrack command mode class
class TrackMouseCommandMode : public CommandMode
{
	public:
		TrackMouseCallBack proc;

		int Class() { return TRACK_MOUSE_COMMAND; }
		int ID() { return CID_TRACK_MOUSE; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints = 1; return &proc; }
		ChangeForegroundCallback *ChangeFGProc() { return NULL; }
		BOOL ChangeFG(CommandMode *oldMode) { return FALSE; }
		void EnterMode() { 
			proc.vl->result = proc.vl->face_num_val = &undefined; 
			MAXScript_interface->PushPrompt(GetString(IDS_TRACK_MOUSE)); 
			SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
		}
		void ExitMode() { 
			end_track_mouse = TRUE;
			MAXScript_interface->PopPrompt(); 
			SetCursor(LoadCursor(NULL, IDC_ARROW));		
		}
	};

static TrackMouseCommandMode theTrackMouseCommandMode;

// called by TrackMouseCallBack to apply the scripted callback function 
static BOOL
do_track_callback(TrackMouseCallBack* ppcb, Value* message, int flags)
{
	init_thread_locals();
	push_alloc_frame();
	save_current_frames();
	try 
	{
		// build arg list and apply callback fn
		// called as:  callBackFn message intersectRay obj faceNum shiftKey ctrlKey altKey [callback_arg]
		//    message = #mousePoint | #mouseMove | #freeMove | #mouseAbort
		// should return #continue to contine, any other value to stop which is passed back as result of mouseTrack fn
		Value* shiftKey = (flags & MOUSE_SHIFT) ? &true_value : &false_value;
		Value* ctrlKey = (flags & MOUSE_CTRL) ? &true_value : &false_value;
		Value* altKey = (flags & MOUSE_ALT) ? &true_value : &false_value;
		Value* args[8] = { message, ppcb->vl->ray_val, MAXNode::intern(ppcb->hit_node),
						   ppcb->vl->face_num_val, shiftKey, ctrlKey, altKey, 
						   ppcb->track_callback_arg 
						};
		ppcb->vl->result = ppcb->track_callback->apply(args, (ppcb->track_callback_arg ? 8 : 7));
	}
	catch (MAXScriptException)// & e)
	{
		clear_error_source_data();
		restore_current_frames();
		// any error disables the callback
		ppcb->track_callback = NULL;
		MAXScript_signals = 0;
		ppcb->vl->result = &undefined;
	}
	catch (...)
	{
		clear_error_source_data();
		restore_current_frames();
		// any error disables the callback
		ppcb->track_callback = NULL;
		ppcb->vl->result = &undefined;
	}

	if (thread_local(redraw_mode) && thread_local(needs_redraw))
	{
		MAXScript_interface->RedrawViews(MAXScript_interface->GetTime());
		needs_redraw_clear();
	}
	BOOL res = ppcb->vl->result == n_continue;
	pop_alloc_frame(ppcb->vl->result); // make sure stack-based values are handled correctly
	// callback fn returns #continue -> keep going, false -> stop pickpoint
	return res;
}

// mouse proc for mouseTrack, handles the actual tracking messages from the
// MAX command mode system
int
TrackMouseCallBack::proc(HWND hwnd, int msg, int point, int flag, IPoint2 m)
{	
	if (ref_deleted)
	{
		// bail if my tracking object is deleted under me
		end_track_mouse = TRUE; 
		return FALSE;		
	}
	
	int result = TRUE;
	ViewExp *vpx = MAXScript_interface->GetViewport(hwnd); 
	int snap = (snap_mode == n_2D) ? SNAP_IN_PLANE : SNAP_IN_3D;
	IPoint2 sm;
	Value* cbmsg = NULL;

	// determine what kind of mouse event
	switch (msg) 
	{
		case MOUSE_FREEMOVE:
			SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
			cbmsg = n_freeMove;
			break;

		case MOUSE_MOVE:
			cbmsg = n_mouseMove;
			break;

		case MOUSE_POINT:
			if (!MAXScript_interface->IsCPEdgeOnInView())
				cbmsg = n_mousePoint;
			break;

		case MOUSE_ABORT:
		case MOUSE_PROPCLICK:	// right click exit
			cbmsg = n_mouseAbort;
			break;
	}
	
	if (cbmsg != NULL)
	{
		//  we have an event to signal to the scripted callback
		Point3 wp;
		Point3 wdirp;
		BOOL   ok = TRUE;
		// valid mouse op, compute tracking point & normal and invoke callback
		if (nodeTab.Count() > 0)
		{
			// point on object surface, get the normal ray
			ok = point_on_obj(vpx, m, r.p, r.dir);
			if (ok == TRUE)
				draw_marker(vpx, r.p, r.dir);
		}
		else
		{
			// point on active grid, compute normal ray
			vpx->SnapPreview(m, sm, NULL, snap);
			Point3 cpp = vpx->SnapPoint(m, sm, NULL, snap);
			Point3 dirp = cpp + Point3(0.0f, 0.0f, 1.0f);
			wp = vpx->MapCPToWorld(cpp);
			wdirp = vpx->MapCPToWorld(dirp);
			r.p = wp; r.dir = wdirp - wp;
			draw_marker(vpx, r.p, r.dir);
		}
		
		// call the scripted callback
		vl->ray_val = ok ? (Value*)new RayValue (r) : (Value*)&undefined;
		if (track_callback != NULL)
			result = do_track_callback(this, cbmsg, flag);
	}

	MAXScript_interface->ReleaseViewport(vpx);
	if (!result)
		end_track_mouse = TRUE; 
	return result;
}

void 
TrackMouseCallBack::start_track()
{
	// set up MAX ref so we can track node deletions
	if (nodeTab.Count()>0)
	{
		HoldSuspend hs (theHold.Holding()); // LAM - 2/22/01
		for( int i=0; i<nodeTab.Count(); i++ )
		{
			INode* tmp = nodeTab[i];
			nodeTab[i] = NULL;
			theTrackMouseCommandMode.proc.ReplaceReference(i, tmp);
		}
		hs.Resume();
	}
	ref_deleted = FALSE;
}

void 
TrackMouseCallBack::end_track()
{
	DeleteAllRefsFromMe();
}

RefResult
TrackMouseCallBack::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message)
{
	// my node deleted, terminate tracking
	if (message == REFMSG_TARGET_DELETED ) {
		for( int i=0; i<nodeTab.Count(); i++ )
			if( hTarget == nodeTab[i] )
				ref_deleted = TRUE;
	}

	return REF_SUCCEED;
}

BOOL
TrackMouseCallBack::point_on_obj(ViewExp *vpt, IPoint2 m, Point3& pt, Point3 &norm)
{
	// computes the normal ray at the point of intersection
	Ray ray, world_ray;
	float at, best_dist = 0.0f;
	TimeValue t = MAXScript_time();	
	Object *obj = NULL;
	Matrix3 obtm, iobtm;
	Point3 testNorm;

	BOOL found_hit = FALSE;
	
	vl->face_num_val = vl->face_bary = &undefined;
	hit_node = NULL;

	// Calculate a ray from the mouse point
	vpt->MapScreenToWorldRay(float(m.x), float(m.y), world_ray);

	for( int i=(nodeTab.Count()-1); i>=0; i-- ) {
		// Get the object from the node
		INode* node = nodeTab[i];
		ObjectState os = node->EvalWorldState(t);
		obj = os.obj;	

		// Back transform the ray into object space.
		obtm	= node->GetObjectTM(t);
		iobtm	= Inverse(obtm);
		ray.p   = iobtm * world_ray.p;
		ray.dir = VectorTransform(iobtm, world_ray.dir);
		
		// See if we hit the object
		if (obj->IsSubClassOf(triObjectClassID))
		{
			TriObject*  tobj = (TriObject*)obj;
			DWORD		fi;
			Point3		bary;
			if (tobj->mesh.IntersectRay(ray, at, testNorm, fi, bary)  &&
			   ((!found_hit) || (at<=best_dist)) )
			{
				// Calculate the hit point and transform everything back into world space.
				// record the face index and bary coord
				best_dist = at;
				pt = ray.p + ray.dir * at;
				pt = pt * obtm;
				norm = Normalize(VectorTransform(obtm, testNorm));
				vl->face_num_val = Integer::intern(fi + 1);
				vl->face_bary = new Point3Value(bary);
				hit_node = node;
				found_hit = TRUE;
			}
		}
		else if (obj->IntersectRay(t, ray, at, testNorm)  &&
				((!found_hit) || (at<=best_dist)) )
		{
			// Calculate the hit point and transform everything back into world space.
			best_dist = at;
			pt = ray.p + ray.dir * at;
			pt = pt * obtm;
			norm = Normalize(VectorTransform(obtm, testNorm));
			hit_node = node;
			found_hit = TRUE;
		}
	}
	if( found_hit ) return TRUE;

	// Failed to find a hit on any node, look at the Normal Align Vector for the first node
	if ((obj!=NULL) && obj->NormalAlignVector(t, pt, testNorm)) // See if a default NA vector is provided
	{
		pt   = pt * obtm;
		norm = Normalize(VectorTransform(obtm, testNorm));
		return TRUE;
	}
	else
		return FALSE;
}

#define MARKER_COLOR	0.0f,0.0f,1.0f
#define MARKER_SCALE	5.0f

void
TrackMouseCallBack::draw_marker(ViewExp* vpt, Point3 p, Point3 norm)
{
return;   // sorry, this doesn't work yet - I'll post it later
	// set GW tm to orientation specified by norm and draw a circle
	Matrix3 tm;
	Point3 zdir, ydir, xdir;
	// compute the direction of the z axis to be.
	// the positive z axis points AWAY from the target.
	zdir = Normalize(norm);
	// compute direction of the X axis before roll.
	xdir = Normalize(CrossProd(zdir.x > 0 ? Point3(0, 0, 1) : Point3(0, 0, -1), zdir));
	// compute direction of the Y axis before roll.
	ydir = Normalize(CrossProd(zdir, xdir));
	tm.SetRow(0, xdir);
	tm.SetRow(1, ydir);
	tm.SetRow(2, zdir);

	vpt->getGW()->setTransform(tm);
	vpt->getGW()->setColor(LINE_COLOR, MARKER_COLOR);
	vpt->getGW()->marker(&p, CIRCLE_MRKR);
}

/* -------------- mouse tracker function ---------------------- */

Value*
mouse_track_cf(Value** arg_list, int count)
{
	// mouseTrack [on:node|#any] [prompt:"msg"] [snap:#2D|#3D] [trackCallback:fn|#(fn,arg)]

	// set up value local array to hold mouse tracker command mode Value*'s
	Value** vavl;
	value_local_array(vavl, sizeof(tmvl) / sizeof(Value*));
	tmvl* vl = (tmvl*)vavl;
	// show any prompt
	Value*	pv = key_arg(prompt);
	TCHAR*	prompt = NULL;
	if  (pv != &unsupplied)
	{
		prompt = pv->to_string();
		mprintf("%s ", prompt);
		mflush();
	}
	// setup snap
	Value* sv = key_arg_or_default(snap, n_2D);
	if (sv != n_2D && sv != n_3D)
		throw RuntimeError (GetString(IDS_BAD_SNAP_MODE), sv);
	// setup track callback fn
	Value* tcb = NULL;
	Value* tcbarg = NULL;
	Value *tcbv = key_arg(trackCallback);
	if (tcbv != &unsupplied)
	{
		if (is_array(tcbv))  // an array,  #(fn, arg), dig out fn & arg
		{
			Array* tcba = (Array*)tcbv;
			if (tcba->size != 2)
				throw RuntimeError (GetString(IDS_BAD_MOVE_CALLBACK_ARGS), tcbv);
			tcb = tcba->data[0];
			if (!is_function(tcb))
				throw RuntimeError (GetString(IDS_BAD_MOVE_CALLBACK_ARGS), tcb);
			tcbarg = tcba->data[1];
		}
		else  // just the fn
		{
			tcb = tcbv;
			if (!is_function(tcb))
				throw RuntimeError (GetString(IDS_BAD_MOVE_CALLBACK_ARGS), tcbv);
		}
	}

	// pickup any node to track on
	vl->node_val = key_arg(on);

	NodeTab& nodeTab = theTrackMouseCommandMode.proc.nodeTab;
	nodeTab.SetCount(0);
	if( vl->node_val->_is_collection() ) { //Multiple nodes specified
		Value* args[2] = { NULL, (Value*)&nodeTab };
		node_map m = { NULL, collect_nodes, args, 2 };
		vl->node_val->map(m);
	}
	else if( vl->node_val != &unsupplied ) { //Single node specified
		INode* node = vl->node_val->to_node();
		nodeTab.Append( 1, &node );
	}
	for( int i=(nodeTab.Count()-1); i>=0; i--)
		if( nodeTab[i]==NULL ) nodeTab.Delete(i,1); //Delete null nodes

	// set up pickpoint options, enter trackmouse command mode & wait for it to signal completion
	end_track_mouse = FALSE;
	theTrackMouseCommandMode.proc.vl = vl;
	theTrackMouseCommandMode.proc.snap_mode = sv;
	theTrackMouseCommandMode.proc.track_callback = tcb;
	theTrackMouseCommandMode.proc.track_callback_arg = tcbarg;
	theTrackMouseCommandMode.proc.start_track();
	//Changed from Set to Push to prevent an exception when the maxscript code changes the command mode
	//Similar to fix done to MouseTool.cpp - line 661
	MAXScript_interface->PushCommandMode(&theTrackMouseCommandMode);

	// process windows messages until point picked or canceled or keyboard message
    MSG wmsg;
    while (GetMessage(&wmsg, NULL, 0,0))
	{		
		if (wmsg.message == WM_KEYUP && (TCHAR)wmsg.wParam == VK_ESCAPE)
			end_track_mouse = TRUE;
		else if (wmsg.message == WM_KEYUP || wmsg.message == WM_KEYDOWN)
			continue;
		else if (wmsg.message != WM_RBUTTONUP)  // ignore rmousebuttonup's - they mess focus
			MAXScript_interface->TranslateAndDispatchMAXMessage(wmsg);

		if (end_track_mouse)
			break;
	}
	if (!end_track_mouse)
	{
		// we came out of message loop because of a quit, repost the quit and throw a SignalException
		theTrackMouseCommandMode.proc.end_track();
		MAXScript_interface->DeleteMode(&theTrackMouseCommandMode);
		throw SignalException();
	}

	// pickup result value & dismiss input modes
	theTrackMouseCommandMode.proc.end_track();
	MAXScript_interface->DeleteMode(&theTrackMouseCommandMode);
	Value* result = vl->result;
	pop_value_local_array(vavl);
	return_protected(result);
}
/*
// plug-in init
void mousetrack_init()
{
}

// get a string out of string resources
TCHAR *GetString(int id)
{
	static TCHAR buf1[128], buf2[128];
	static TCHAR* bp = buf1;
	TCHAR* result = NULL;
	if (hInstance)
		result = LoadString(hInstance, id, bp, sizeof(buf1)) ? bp : NULL;
	bp = (bp == buf1) ? buf2 : buf1;   // twiddle between two buffers to help multi-getstring users
	return result;
}
*/
