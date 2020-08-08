/**********************************************************************
 *<
	FILE: section.cpp

	DESCRIPTION:  A section implementation

	CREATED BY: Jon Barber (based on the grid helper object)

	HISTORY: created Dec 4, 1996
			 Changed to ParamMaps for the UI  (12/10)
			 Feb 5:  major reworking - section is now a subclass of shape, 
						with the spline as a part of the object			 
			 June 11: Christer Janson
						Port to MAX Release 2.0	(or rather, port to Visual C++ 5.0)
						Enabled Create Section Spline code
						Added new spline Name prompt
			 June 16: Christer Janson
						Changed it to create Editable splines (instead of linearShapes)
						Added option to set section scope (All, Within Section, or Off)
			 June 19: Christer Janson
						Changed spline creation to use the current TM (not from last update)
			 May 2007: Chris Johnson / Jean-Francois Yelle
						Fixed up the parameter blocks to use the correct reference ID's. This defect had
						been in place ever since the section code was originally written. The error was discovered
						years later and a patch was put in place to that really just masked the problem. I 
						believe the patch was necassary because they had to preserve file compatibility at the time.
						For the upcoming release of Max 10, we are under no such guideline and we can really
						fix the problem, which was apparently this: The SectionObject class has a member Parameter
						block which it was getting and setting using index 0. It should have been getting and
						setting it's parent (ShapeObject Class's) parameter block using that index. This all is explained
						in the version control comments for this file. The problem in just fixing it, was preserving
						backwards file compatibility (i.e. reading old files). Thus, The fix was to attach a static 
						versioning member to the SectionObject class to help sort out when to use the old 
						behavior, and when to use the new proper behavior. With much help from Jean-Francois Yelle. 
	
	The following came from a note I sent to MaxSDK tech support about how 
	this works:


	Here's a rough outline of Section's algorithm, of how it intersects the 
	cutting plane with an object's mesh.

		1.  Determine the transformation matrix for the cutting 
		plane - call this M.

		2.  Transform the vertices in the mesh by the Inverse of M, 
		which effectively puts the points in the coordinate system 
		of the cutting plane.

		3.  Traverse the face list of the mesh.  For each face,

			4.  Trivially reject the face if all three of its (transformed) 
			vertice z-values are above or below zero (within a floating 
			point tolerance) (meaning the face does not intersect the plane).

			5.  Otherwise, count N the number of vertices that are on the 
			plane, i.e. whose z-values are within a tolerance of zero.  
			This count will be 0, 1, 2 or 3 (since there are only 3 vertices 
			per mesh face).

			6.  switch on the above count N

				case 3:  all the vertices of the face are on the cutting 
				plane.  Add all the edges to the section spline shape.

				case 2:  two vertices on the plane.  Add the single 
				edge between those two vertices to the section spline shape.

				case 1:  1 vertex on the plane - call this point P1.  
				If the other two vertices are on opposite sides of the 
				plane, do a linear interpolation between those two points 
				to find the point P2 that is the intersection of that edge 
				with the plane.  Add the segment P1-P2 to the section 
				spline shape.  If the other two vertices are on both sides 
				of the plane, ignore this face.

				case 0:  no vertices are on the plane, so there must be two 
				edges that intersect the plane.  Find the intersection 
				points of those two edges with the plane, and add the segment 
				between them to the section spline shape.

		7.  The result will be a shape that is a set of disconnected line 
		segment splines.  I also implemented a welding algorithm that throws 
		out trivial and duplicate segments, and concatenates adjacent line 
		segments into single splines.

		There are other possible algoritms, of course!  And you have to be 
		careful about floating point zero equality stuff.  And the welding 
		algorithm turned out to be harder than I expected, which makes it 
		tempting to use a different algorithm all together, one that doesn't 
		create so many independent spline pieces (but this could be a bit 
		hard, too).



 *>	Copyright (c) 1995, 1997, All Rights Reserved.
 **********************************************************************/
//#define DEBUG_SLICE
//#define DEBUG_WELD
//#define DEBUG_MERGE
//#define DEBUG_BBOX
//#define DEBUG_UPDATES
//#define DEBUG_WITH_AFX

//#define ADJ_EDGES			// use an adjacency list for computing slices - slower
#define CREATE_SECTION		// define this when the Create Section button is approved
								// (will have to change the resource dialog, too)

#include "Max.h"
#include "iparamm.h"
#include "LIMITS.H"
#include "float.h"
#include "sect_res.h"
#include "linshape.h"
#include "splshape.h"
#include "modstack.h"

#ifdef ADJ_EDGES
#include "meshadj.h"
#endif

#include <assert.h>

#include "3dsmaxport.h"

#ifndef MAX_FLOAT 
#define MAX_FLOAT 1e30f
#endif

#define SIGN(f) ( (int) ( fabs(double(f)) <= FLT_EPSILON ? 0 : ((f) > float(0) ? 1 : -1) ) )
#define WELD_SIGN(f) ( (int) ( fabs(double(f)) <= 1e-4 ? 0 : ((f) > float(0) ? 1 : -1) ) )


// From mods.h
#define EDITSPLINE_CLASS_ID                     0x00060

// Flags
#define VP_DISP_END_RESULT 0x01

class SectionObjCreateCallBack;
class SectionObject;

static const Class_ID linesClassID=Class_ID(0x1c6d154a, 0x59f47e60);
static const Class_ID sectionClassID=Class_ID(0x20b9236d, 0x66da18aa);

TCHAR *GetString(int id);

// types of update methods
//#define UPDATE_ALWAYS		0
#define UPDATE_SECT_MOVE	0
#define UPDATE_SECT_SELECT	1
#define UPDATE_MANUAL		2

static int updateIDs[] = 
{	
	//IDC_UPDATE_ALWAYS,
	IDC_UPDATE_SECT_MOVE,
	IDC_UPDATE_SECT_SELECT,
	IDC_UPDATE_MANUAL,
};

// types of update methods
#define SECTION_SCOPE_ALL		0
#define SECTION_SCOPE_LIMIT		1
#define SECTION_SCOPE_OFF		2

static int scopeIDs[] = 
{	
	IDC_SCOPE_ALL,
	IDC_SCOPE_LIMIT,
	IDC_SCOPE_OFF,
};

// See May 2007 notes above for info about these version variables
static const int SERIALIZATION_VERSION_0 = 0;	// For Max 1 - 9 files.
static const int SERIALIZATION_VERSION_1 = 1;	// For Max 10 files and afterwards.
static const int SERIALIZATION_CURRENT_VERSION = SERIALIZATION_VERSION_1;

static const int SHAPE_OBJ_PARAM_BLOCK_ID   = SHAPEOBJPBLOCK; // A macro!! defined in splshape.h, yet the class it defines it for is in object.h. How stupid.
static const int SECTION_OBJ_PARAM_BLOCK_ID = SHAPEOBJPBLOCK + 1;

//-------------------------------------------------------------------
//------------- LineShape Class definition ------------------------
//-------------------------------------------------------------------

class LinesShape : public LinearShape
{
public:
	Class_ID ClassID() { return linesClassID; }
	void GetClassName(TSTR& s) { s = TSTR(_T("LinesShape")); }

	RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		                        PartID& partID, RefMessage message ) 
									{ return(REF_SUCCEED); };

};


//-------------------------------------------------------------------
//------------- SectionObject Class definition ----------------------
//-------------------------------------------------------------------

class SectionObject: public LinesShape, public IParamArray 
{			   
	friend class SectionObjCreateCallBack;
	friend class SectionDlgProc;
	friend BOOL CALLBACK SectionParamDialogProc( HWND hDlg, UINT message, 
							WPARAM wParam, LPARAM lParam );
	friend BOOL CALLBACK SectionRenderParamDialogProc( HWND hDlg, UINT message, 
							WPARAM wParam, LPARAM lParam );
	
public:
	static IParamMap*	m_pmapCreate;
	static IParamMap*	m_pmapParam;
	IParamBlock*		m_pblock;
	static IObjParam	*m_ip;				// the interface pointer into Max

	// Object parameters		
	Color			m_gobj_sliceColor;	// the color of the slice
	int				m_update_method;	// when moves, when selected, when button-hit

	BOOL			m_pleaseUpdate;		// whether to update in next display
										// enabled by button hit or EndEditParams

private:
	TimeValue		m_curTime;			// the current time
	Matrix3			m_sliceTM;			// the slice's tm
	Matrix3			m_lastNodeTM;		// The nodeTM the last time display was called.
	Matrix3			m_sliceTM_inv;		// inverse of tm
	INode*			m_curNode;			// the current node being sliced
	Point3			m_tm_bbox[8];		// corners of an object's bbox, in slice space
	Mesh*			m_curMesh;			// the mesh of an object
	PolyLine		m_facelines[3];		// for computing face intersection lines
	Tab<Point3>		m_ptlist;			// for storing up the segment list points
	BOOL			m_sliceWelded;		// whether the slice has been welded
	HCURSOR			m_waitCursor;		//
	BOOL			m_justLoaded;		// whether the slice was just loaded (so don't update)
	int				m_SectionScope;		// What to slice (and what to ignore)

	float			m_lastLength;
	float			m_lastWidth;

	DWORD			m_flags;


	void	CreateSlice();
	void	TraverseWorld(INode *node, const int level, Box3& boundBox);
	void	DoMeshSlice(void);				// computes the slice thru an object's mesh
	int		CrossCheck(const Point3& pt1,	// checks that a line segment p1-p2 crosses the plane
		               const Point3& pt2,	// if so, puts cross in cross_pt	
					   Point3* cross_pt);	// returns 0 for no slice, 1 for one pt, 2 for coplanar
	void	CrossPoint(const Point3& pt1,	// same as CrossCheck, but we know the points cross the xy plane
		               const Point3& pt2, 
					   Point3* cross_pt);
	void	SliceASceneNode(void);			// compute slice thru a node
	void	SliceFace(Point3* tm_verts, const DWORD f_index); // compute a slice thru a mesh's face

	void	UpdateSlice(INode *sectionNode, TimeValue t);

	void	WeldSlice();
//------------------------------------------------------------------
	public:
		
		void InvalidateUI();
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);

		// Offset
		Matrix3	myTM;
		Interval ivalid;
		
		//  inherited virtual methods for Reference-management
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
							PartID& partID, RefMessage message );
		//void BuildMesh(TimeValue t,Mesh& amesh);
		void UpdateMesh(TimeValue t);
		//void UpdateUI(TimeValue t);
		int Select(TimeValue t, INode *inode, GraphicsWindow *gw, Material *ma, HitRegion *hr, int abortOnHit );
	
	
	public:
		SectionObject();
		SectionObject(SectionObject& so);
		~SectionObject();

		//  inherited virtual methods:

		// From ConstObject
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);

		// IParamArray get/set methods
		BOOL GetValue(int i, TimeValue t, Point3& v,	Interval &ivalid);  // for color
		BOOL SetValue(int i, TimeValue t, Point3& v);						// for color
		BOOL GetValue(int i, TimeValue t, int &v,		Interval &ivalid);
		BOOL SetValue(int i, TimeValue t, int v);
		

		// From Object
		SClass_ID SuperClassID()	{ return SHAPE_CLASS_ID; }
		ObjectState Eval(TimeValue time);
		void InitNodeName(TSTR& s) { s = GetString(IDS_DB_SECTION); }
		int UsesWireColor()			{ return 1; }
		ObjectHandle ApplyTransform(Matrix3& matrix);
		Interval ObjectValidity(TimeValue t);
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
		void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
		BOOL IsWorldSpaceObject()	{return TRUE;}

		// From ConstObject
		void GetConstructionTM( TimeValue t, INode* inode, ViewExp *vpt, Matrix3 &tm );	// Get the transform for this view
		Point3 GetSnaps( TimeValue t );

		// Animatable methods
		void DeleteThis() { delete this; }
		Class_ID ClassID() { return sectionClassID; }  
		void GetClassName(TSTR& s) { s = TSTR(GetString(IDS_DB_SECTIONER)); }
		int IsKeyable(){ return 1;}
		LRESULT CALLBACK TrackViewWinProc( HWND hwnd,  UINT message, 
	            WPARAM wParam,   LPARAM lParam ){return(0);}
		
		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		// From ref
		RefTargetHandle Clone(RemapDir& remap);
		// This class owns a Parameter Block, The next class up the class hierarchy that implements 
		// a parameter block is ShapeObject, thus increment the count by one.
		int NumRefs() {return ShapeObject::NumRefs() + 1;} 
		int RemapRefOnLoad(int iref);
		RefTargetHandle GetReference(int i);			
		void SetReference(int i, RefTargetHandle rtarg) ;		

		// Interface
		BaseInterface* GetInterface(Interface_ID id);

		TCHAR* GetObjectName()	{ return GetString(IDS_OBJECTNAME); }
		
		int NumSubs();
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int i)	{ return 1; }

		// May 2007. New member to distinguish from former versions when we are loading and 
		// setting references back (i.e. during a file load).
		static int m_version;

	private:
		void SetFlag (DWORD fl)		{ m_flags |= fl; }
		void ClearFlag (DWORD fl)	{ m_flags &= ~fl; }
		void SetFlag (DWORD fl, bool set) { if (set) SetFlag (fl); else ClearFlag (fl); }
		bool GetFlag (DWORD fl)		{ return (fl&m_flags) ? TRUE : FALSE; }

	};				

int	SectionObject::m_version = 0; // initialize static member

HINSTANCE hInstance;

//---------------------------------------------------------------------------
//-------------- Parameter Map & Block stuff  -------------------------------
//---------------------------------------------------------------------------

// Parameter block indices
#define PB_LENGTH			0
#define PB_WIDTH			1

//  animatable parameter UI description
static ParamUIDesc  descParam[] = 
{
	// length
	ParamUIDesc(PB_LENGTH, EDITTYPE_UNIVERSE, IDC_LENGTHEDIT, IDC_LENSPINNER,
				float(0), MAX_FLOAT, SPIN_AUTOSCALE),
	// width
	ParamUIDesc(PB_WIDTH, EDITTYPE_UNIVERSE, IDC_WIDTHEDIT, IDC_WIDTHSPINNER,
				float(0), MAX_FLOAT, SPIN_AUTOSCALE),
};
#define PARAMDESC_LENGTH 2

// Non-parameter block indices
#define PB_OBJSLICE_COLOR		0
#define PB_UPDATE_METHOD		1
#define PB_SECTION_SCOPE		2

static ParamUIDesc	descCreate[] =
{
	// obj slice color
	ParamUIDesc(PB_OBJSLICE_COLOR, TYPE_COLORSWATCH, IDC_OBJSLICE_COLORSWATCH),
	// obj slice display
	ParamUIDesc(PB_UPDATE_METHOD, TYPE_RADIO, updateIDs, 3),
	// Section scope
	ParamUIDesc(PB_SECTION_SCOPE, TYPE_RADIO, scopeIDs, 3),

};
#define CREATEDESC_LENGTH 3

//--------------------------------------------------------------

// animatable parameters stored in Parameter Block
static ParamBlockDescID descVer0[] = 
{
	//	param_type,		not-used,		animatable,		id
	{	TYPE_FLOAT,		NULL,			TRUE,			PB_LENGTH		},
	{	TYPE_FLOAT,		NULL,			TRUE,			PB_WIDTH		},
};
#define PBLOCK_LENGTH 2

static ParamVersionDesc *versions = NULL;
#define NUM_OLDVERSIONS	0

// Current version
#define CURRENT_VERSION	0
static ParamVersionDesc curVersion(descVer0,PBLOCK_LENGTH,CURRENT_VERSION);

//---------------------------------------------------------------------------
static int EqualMatrices(const Matrix3& m1, const Matrix3& m2) 
{
	return m1.GetRow(0)==m2.GetRow(0) && 
		   m1.GetRow(1)==m2.GetRow(1) && 
		   m1.GetRow(2)==m2.GetRow(2) && 
		   m1.GetRow(3)==m2.GetRow(3);
}



//---------------------------------------------------------------------------
//-------------- Section dialog proc - Handles the buttons ------------------
//---------------------------------------------------------------------------

class SectionDlgProc : public ParamMapUserDlgProc
{
	SectionObject *so;
public:
	SectionDlgProc( SectionObject *s ) { so = s; };
	INT_PTR DlgProc(TimeValue t, IParamMap *map, HWND hWnd, UINT msg,
					WPARAM wParam, LPARAM lParam);
	void DeleteThis() {delete this;}
};


// ------------- DoSlice DlgProc -----------------------
//
// This is the method called when the user clicks on the Do Slice button
// in the rollup.  It was registered as the dialog proc
// for this button by the SetUserDlgProc() method called from 
// BeginEditParams().
//
// also called for the weld button

INT_PTR SectionDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,
			WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_UPDATE_MANUAL: {
						HWND hBut = GetDlgItem(hWnd, IDC_SECTION_UPDATE);
						ICustButton *iBut = GetICustButton(hBut);
						iBut->Enable();
						ReleaseICustButton(iBut);
					}
					return TRUE;	
				case IDC_UPDATE_SECT_SELECT: {
						HWND hBut = GetDlgItem(hWnd, IDC_SECTION_UPDATE);
						ICustButton *iBut = GetICustButton(hBut);
						iBut->Enable();
						ReleaseICustButton(iBut);
					}
					return TRUE;;
				case IDC_UPDATE_SECT_MOVE: {
						HWND hBut = GetDlgItem(hWnd, IDC_SECTION_UPDATE);
						ICustButton *iBut = GetICustButton(hBut);
						iBut->Disable();
						ReleaseICustButton(iBut);
					}
					return TRUE;

				case IDC_SCOPE_ALL:
				case IDC_SCOPE_LIMIT:
				case IDC_SCOPE_OFF:
					so->m_pleaseUpdate = TRUE;
					so->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
					return TRUE;
					
				case IDC_SECTION_UPDATE: 
					so->m_pleaseUpdate = TRUE;
					so->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
					return TRUE;	

#ifndef CREATE_SECTION
				case IDC_SECTION_WELD: 
					so->WeldSlice();
					so->m_pleaseUpdate = TRUE;
					//GetCOREInterface()->RedrawViews(t);
					return TRUE;	

#else CREATE_SECTION
				case IDC_SECTION_CREATE: 
					so->CreateSlice();
					so->m_pleaseUpdate = TRUE;
					so->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
					return TRUE;
#endif
			}
			break;	
	}
	return FALSE;
}



//-------------------------------------------------------------------
//------------------ Section methods --------------------------------
//-------------------------------------------------------------------
//
#ifdef DEBUG_WITH_AFX
CMemoryState oldMemState, newMemState, diffMemState;
#endif


SectionObject::SectionObject() : LinesShape() 
{
	m_version = SERIALIZATION_CURRENT_VERSION;
#ifdef DEBUG_WITH_AFX
	afxMemDF = allocMemDF | checkAlwaysMemDF;

	oldMemState.Checkpoint();

	leak = new int[111];
#endif
	
	m_pblock = NULL;
	ReplaceReference(SECTION_OBJ_PARAM_BLOCK_ID, CreateParameterBlock(descVer0, PBLOCK_LENGTH, CURRENT_VERSION));

	assert(m_pblock);

	m_pblock->SetValue(PB_LENGTH, 0, float(0));
	m_pblock->SetValue(PB_WIDTH,  0, float(0));

	// initialize some other stuff
	m_gobj_sliceColor	= Color(1.0,1.0,0.0);
	m_update_method		= UPDATE_SECT_MOVE;
	m_sliceWelded		= FALSE;
	m_justLoaded		= FALSE;

	m_SectionScope		= SECTION_SCOPE_ALL;

	ivalid.SetEmpty();
	myTM.IdentityMatrix();

	m_pleaseUpdate	= FALSE;	// should we recalculate slice
	m_curTime		= -1;
	m_sliceTM.Zero();
	
	// initialize its shape spline
	shape.NewShape();

	// used for construction face slices
	m_facelines[0].SetNumPts(2,FALSE);
	m_facelines[1].SetNumPts(2,FALSE);
	m_facelines[2].SetNumPts(2,FALSE);

	m_waitCursor = LoadCursor(NULL, IDC_WAIT);
	LinesShape::LinesShape();
}

//-------------------------------------------------------------------
SectionObject::~SectionObject()
{
	// delete all refs
	RefResult res = DeleteAllRefsFromMe();
	assert(res == REF_SUCCEED);

	//LinesShape::~LinesShape();

#ifdef DEBUG_WITH_AFX
	newMemState.Checkpoint();
	if (diffMemState.Difference(oldMemState,newMemState))
	{ 
		TRACE("===Section Memory leaks:  \n");
		diffMemState.DumpStatistics();
	}
#endif

}

//-------------------------------------------------------------------

static bool oldShowEnd;

void SectionObject::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev)
{
	m_ip = ip;

	long newFlags = flags;
	newFlags |= BEGIN_EDIT_SHAPE_NO_RENDPARAM;

	// call const object parent method
	LinesShape::BeginEditParams(ip,newFlags,prev);

	if (m_pmapCreate)
	{
		// left over from last section created
		m_pmapCreate->SetParamBlock(this);
		m_pmapParam->SetParamBlock(m_pblock);

	}
	else
	{
		// gotta make new ones
		m_pmapCreate = CreateCPParamMap(
			descCreate, CREATEDESC_LENGTH, this, ip, hInstance, 
			MAKEINTRESOURCE(IDD_SECTIONCREATE), GetString(IDS_DB_CREATE_VALS), 0);

		m_pmapParam = CreateCPParamMap(
			descParam, PARAMDESC_LENGTH, m_pblock, ip, hInstance, 
			MAKEINTRESOURCE(IDD_SECTIONPARAM), GetString(IDS_DB_PARAM_VALS), 0);
	}

	if (m_pmapCreate)
	{
		m_pmapCreate->SetUserDlgProc(new SectionDlgProc(this));
	}

	if (m_update_method == UPDATE_SECT_SELECT)
	{
		m_pleaseUpdate = TRUE;
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

	// Grey out update button if update mode is not manual.
	ICustButton *iBut;
	
	iBut = GetICustButton(GetDlgItem(m_pmapCreate->GetHWnd(), IDC_SECTION_UPDATE));
	if (IsDlgButtonChecked(m_pmapCreate->GetHWnd(), IDC_UPDATE_SECT_MOVE)) {
		iBut->Disable();
	}
	else {
		iBut->Enable();
	}

	ReleaseICustButton(iBut);

#ifndef CREATE_SECTION
	assert(m_pmapCreate);
	HWND uiHwnd = m_pmapCreate->GetHWnd();
	assert(uiHwnd);
	uiHwnd = GetDlgItem(uiHwnd, IDC_SECTION_WELD);
	assert(uiHwnd);
	iBut = GetICustButton(uiHwnd);			
	if (m_sliceWelded || shape.numLines == 0)
		iBut->Disable();
	else
		iBut->Enable();

	ReleaseICustButton(iBut);

#else
	assert(m_pmapCreate);
	HWND uiHwnd = m_pmapCreate->GetHWnd();
	assert(uiHwnd);
	uiHwnd = GetDlgItem(uiHwnd, IDC_SECTION_CREATE);
	assert(uiHwnd);
	iBut = GetICustButton(uiHwnd);
	if (shape.numLines == 0)
		iBut->Disable();
	else
		iBut->Enable();

	ReleaseICustButton(iBut);
#endif

    // Set show end result.
    oldShowEnd = ip->GetShowEndResult() ? TRUE : FALSE;
    ip->SetShowEndResult (GetFlag (VP_DISP_END_RESULT));
}		
//-------------------------------------------------------------------
void SectionObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{	
	long newFlags = flags;
	newFlags |= BEGIN_EDIT_SHAPE_NO_RENDPARAM;

	LinesShape::EndEditParams(ip,newFlags,next);
	m_ip = NULL;

    // Reset show end result
    SetFlag (VP_DISP_END_RESULT, ip->GetShowEndResult() ? TRUE : FALSE);
    ip->SetShowEndResult(oldShowEnd);

	if ( flags&END_EDIT_REMOVEUI ) 
	{
		DestroyCPParamMap(m_pmapParam);
		m_pmapParam = NULL;

		DestroyCPParamMap(m_pmapCreate);
		m_pmapCreate = NULL;
	}
}


static INT_PTR CALLBACK nameDlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	TSTR *name = DLGetWindowLongPtr<TSTR*>(hWnd);
	switch (msg) {
		case WM_INITDIALOG:
			name = (TSTR*)lParam;
			DLSetWindowLongPtr(hWnd, lParam);
			CenterWindow(hWnd, GetParent(hWnd));
			SendMessage(GetDlgItem(hWnd, IDC_NAME), WM_SETTEXT, 0, (LPARAM)name->data());
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDOK: {
				int strLen = SendMessage(GetDlgItem(hWnd, IDC_NAME), WM_GETTEXTLENGTH, 0, 0);
				name->Resize(strLen+1);
				SendMessage(GetDlgItem(hWnd, IDC_NAME), WM_GETTEXT, strLen+1, (LPARAM)name->data());
				EndDialog(hWnd, 1);
				break;
			}
			case IDCANCEL:
				EndDialog(hWnd, 0);
				break;
			}
			break;
		default:
			return 0;
	}

	return 1;
}


//-------------------------------------------------------------------

void SectionObject::CreateSlice(void)
{
	TSTR shapeName;
	assert(m_ip);

	shapeName.printf(GetString(IDS_BASENAME));
	m_ip->MakeNameUnique(shapeName);

	if (!DialogBoxParam(
			hInstance,
			MAKEINTRESOURCE(IDD_NAMEPROMPT),
			m_ip->GetMAXHWnd(),
			nameDlgProc,
			(LPARAM)&shapeName)) {
		return;
	}

	WeldSlice();

	/*
	// Jon's code:
	LinesShape* instance = (LinesShape *)m_ip->CreateInstance( SHAPE_CLASS_ID, linesClassID );
	assert(instance);
	instance->shape = shape;

	INode *node = m_ip->CreateObjectNode(instance);
	node->SetName(shapeName);
	assert(node);
	node->SetNodeTM(m_ip->GetTime(), m_sliceTM);
	// End of Jon's code
	*/


	// Undo for Create Shape
	theHold.Begin();


	// Get center point of the slice
	Box3	boundingBox = shape.GetBoundingBox();
	Point3	center = boundingBox.Center();

	// Create a translation matrix for each point of the slice
	Matrix3 xlatMx = Inverse(TransMatrix(center));

	SplineShape* instance = (SplineShape *)m_ip->CreateInstance(SHAPE_CLASS_ID, splineShapeClassID/*Class_ID(0xa, 0x00)*/);

	for (int l=0; l<shape.numLines; l++) {
		Spline3D* spline = instance->shape.NewSpline();
		for (int k=0; k<shape.lines[l].numPts; k++) {
			// Transform each point with the inverse of the center
			Point3 pt = xlatMx * shape.lines[l].pts[k].p;
			spline->AddKnot(SplineKnot(KTYPE_CORNER, LTYPE_CURVE, pt, pt, pt));
		}

		if (shape.lines[l].flags & POLYLINE_CLOSED) {
			spline->SetClosed(1);
		}
		spline->ComputeBezPoints();
	}

	instance->shape.UpdateSels();
	instance->shape.InvalidateGeomCache();

	INode *node = m_ip->CreateObjectNode(instance);
	node->SetName(shapeName);

	// Set pivot point (and "restore" the translation)
	SuspendAnimate();
	AnimateOff();
	node->SetNodeTM(m_ip->GetTime(), TransMatrix(center) * m_lastNodeTM/* m_sliceTM */);
	ResumeAnimate();

	// Undo for Create Shape
	theHold.Accept(GetString(IDS_CREATE_SHAPE_UNDONAME));

}

// ------------- Convert object to TriObject -----------------------

// Return a pointer to a TriObject given an INode or return NULL
// if the node cannot be converted to a TriObject
TriObject *GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt)
{
	deleteIt = FALSE;
	Object *obj = node->EvalWorldState(t).obj;
	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) { 
		TriObject *tri = (TriObject *) obj->ConvertToType(0, 
			Class_ID(TRIOBJ_CLASS_ID, 0));
		// Note that the TriObject should only be deleted
		// if the pointer to it is not equal to the object

		// pointer that called ConvertToType()
		if (obj != tri) deleteIt = TRUE;
		return tri;
	}
	else {
		return NULL;
	}
}

#define FUZZ_FACTOR 1.0e-6f

// Not truly the correct way to compare floats of arbitrary magnitude...
BOOL EqualPoint3(Point3 p1, Point3 p2)
{
	if (fabs(p1.x - p2.x) > FUZZ_FACTOR)
		return FALSE;
	if (fabs(p1.y - p2.y) > FUZZ_FACTOR)
		return FALSE;
	if (fabs(p1.z - p2.z) > FUZZ_FACTOR)
		return FALSE;

	return TRUE;
}


	  
//-------------------------------------------------------------------
void SectionObject::UpdateSlice(INode *sectionNode, TimeValue t)
{

#ifdef DEBUG_UPDATES
	static int inhere=0;
	DebugPrint(_T("UpdateSlice #%04d - "),++inhere);
#endif
	if (!m_pleaseUpdate)
	{
		BOOL forceUpdate = FALSE;

		// Force an update of the slice if the grid size is animated
		// and the scope [extents] is limited to the grid.
		if (m_SectionScope == SECTION_SCOPE_LIMIT) {
			float length, width;
			m_pblock->GetValue(PB_LENGTH, t, length, FOREVER);
			m_pblock->GetValue(PB_WIDTH, t, width, FOREVER);

			if (length != m_lastLength || width != m_lastWidth) {
				// Parameters have changed since last update - 
				forceUpdate = TRUE;
			}
		}

		if (m_update_method == UPDATE_SECT_MOVE)
		{
			Matrix3 curTM = sectionNode->GetNodeTM(t);

			if ((m_justLoaded || EqualMatrices(curTM,m_sliceTM)) && !forceUpdate)
			{
				m_justLoaded = FALSE;
				m_sliceTM = curTM;
#ifdef DEBUG_UPDATES
				DebugPrint(_T("curTM == lastTM (or just loaded)\n"));
#endif
				return;
			}
		} 
		else
		{
#ifdef DEBUG_UPDATES
			DebugPrint(_T("no please update\n"));
#endif
			return;
		}
	}


	HCURSOR oldcur = SetCursor(m_waitCursor);

#ifdef DEBUG_UPDATES
	DebugPrint(_T("calculating slice\n"));
#endif
	
	Matrix3 sliceTM;

	assert(m_ptlist.Count()==0);

	m_sliceTM		= sectionNode->GetNodeTM(t);
	m_sliceTM_inv	= Inverse(m_sliceTM);
	m_curTime		= t;

	INode *rootNode = GetCOREInterface()->GetRootNode();
	assert(rootNode);


	////////////////////////////////////////////////////
	// Added to slice only nodes within the section lines
	//

	Box3 boundBox;
	boundBox.Init();

	Point2 vert[2];

	m_pblock->GetValue(PB_LENGTH, t, m_lastLength, FOREVER);
	m_pblock->GetValue(PB_WIDTH, t, m_lastWidth, FOREVER);

	vert[0].x = -m_lastWidth/float(2);
	vert[0].y = -m_lastLength/float(2);
	vert[1].x = m_lastWidth/float(2);
	vert[1].y = m_lastLength/float(2);

	boundBox += Point3( vert[0].x, vert[0].y, (float)0 );
	boundBox += Point3( vert[1].x, vert[0].y, (float)0 );
	boundBox += Point3( vert[0].x, vert[1].y, (float)0 );
	boundBox += Point3( vert[1].x, vert[1].y, (float)0 );

	boundBox = boundBox * m_sliceTM;

	//
	// End
	////////////////////////////////////////////////////

	if (m_SectionScope != SECTION_SCOPE_OFF) {	// Hack to turn off
		TraverseWorld(rootNode, 0, boundBox);
	}

/*
	// No detection of 0 length line segments

	shape.NewShape();
	shape.bdgBox.Init();

	shape.SetNumLines(m_ptlist.Count()/2, FALSE);

	PolyLine pl;
	pl.SetNumPts(2);

	shape.bdgBox.Init();
	for (int jj = 0; jj < m_ptlist.Count()/2; ++jj)
	{
		pl.pts[0] = m_ptlist[jj*2];
		pl.pts[1] = m_ptlist[jj*2+1];

		shape.bdgBox += pl.pts[0].p;
		shape.bdgBox += pl.pts[1].p;
		
		shape.lines[jj] = pl;
	}
*/

/*
	// There are 0 length line segements added here so we attempt a simple
	// first pass attempt to optimize the shape // CJ
	//
	// This dynamic approach turned out to be very slow. Can't use it. Bummer.

	shape.NewShape();
	shape.bdgBox.Init();
	for (int jj = 0; jj < m_ptlist.Count()/2; ++jj)
	{
		if (!EqualPoint3(m_ptlist[jj*2], m_ptlist[jj*2+1])) {
			PolyLine* pl = shape.NewLine();
			pl->Append(PolyPt(m_ptlist[jj*2]));
			pl->Append(PolyPt(m_ptlist[jj*2+1]));
		}
	}

	shape.UpdateSels();
	shape.InvalidateGeomCache(FALSE);
	shape.BuildBoundingBox();

	m_ptlist.Resize(0);  // blows away the memory associated for the point list
*/

	// Modified the original approach to discard 0 length segments
	// without dynamic allocation. Fast & Efficient. - CCJ

	shape.NewShape();
	shape.bdgBox.Init();

	shape.SetNumLines(m_ptlist.Count()/2, FALSE);

	PolyLine pl;
	pl.SetNumPts(2);
	int numLinesUsed = 0;

	shape.bdgBox.Init();
	for (int jj = 0; jj < m_ptlist.Count()/2; ++jj)
	{
		pl.pts[0] = m_ptlist[jj*2];
		pl.pts[1] = m_ptlist[jj*2+1];

		if (!EqualPoint3(pl.pts[0].p, pl.pts[1].p)) {
			shape.bdgBox += pl.pts[0].p;
			shape.bdgBox += pl.pts[1].p;
			shape.lines[numLinesUsed++] = pl;
		}
	}

	shape.SetNumLines(numLinesUsed, TRUE);
	shape.UpdateSels();
	shape.InvalidateGeomCache(FALSE);
	shape.BuildBoundingBox();

	m_ptlist.Resize(0);  // blows away the memory associated for the point list

	m_pleaseUpdate = FALSE;
	m_sliceWelded  = FALSE;

#ifndef CREATE_SECTION
	if (m_pmapCreate)
	{
		assert(m_pmapCreate);
		HWND uiHwnd = m_pmapCreate->GetHWnd();
		assert(uiHwnd);
		uiHwnd = GetDlgItem(uiHwnd, IDC_SECTION_WELD);
		assert(uiHwnd);
		ICustButton *iBut = GetICustButton(uiHwnd);			
		if (shape.numLines > 0)
			iBut->Enable();
		else
			iBut->Disable();

		ReleaseICustButton(iBut);
	}
#else
	if (m_pmapCreate)
	{
		assert(m_pmapCreate);
		HWND uiHwnd = m_pmapCreate->GetHWnd();
		assert(uiHwnd);
		uiHwnd = GetDlgItem(uiHwnd, IDC_SECTION_CREATE);
		assert(uiHwnd);
		ICustButton *iBut = GetICustButton(uiHwnd);
		if (shape.numLines > 0)
			iBut->Enable();
		else
			iBut->Disable();

		ReleaseICustButton(iBut);
	}
#endif

	SetCursor(oldcur);

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

//--------------------------------------------------
//
// slice this node and recurse thru it's children
//
void SectionObject::TraverseWorld(INode *node, const int level, Box3& boundBox)
{
	const int num = node->NumberOfChildren();

	BOOL doSlice = TRUE;

	if (m_SectionScope == SECTION_SCOPE_LIMIT) 
	{
		Object *obj = node->EvalWorldState(m_curTime).obj;
		if (obj) {
			Box3 nodeBBox;
			obj->GetDeformBBox(m_curTime, nodeBBox, &node->GetObjectTM(m_curTime));
			if (!boundBox.Intersects(nodeBBox)) {
				doSlice = FALSE;
			}
		}
	}

	if (doSlice) 
	{
		m_curNode = node;
		SliceASceneNode();
	}

	// TraverseWorld other nodes

	for (int jj = 0; jj < num; ++jj)
	{
		INode* child = node->GetChildNode(jj);
		TraverseWorld(child, level+1, boundBox);
	}
}

//--------------------------------------------------
//
// slice this node
//
void SectionObject::SliceASceneNode(void)
{
	// Get the object from the node
	ObjectState os = m_curNode->EvalWorldState(m_curTime);
	m_curNode->BeginDependencyTest();
	NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);

	if (!m_curNode->EndDependencyTest() &&
		!m_curNode->IsHidden() && 
		!m_curNode->IsFrozen() &&
		os.obj && 
		os.obj->SuperClassID() == GEOMOBJECT_CLASS_ID) 
	{
		GeomObject* geom_obj = (GeomObject*)os.obj; // the current object being sliced
		
		//SS 8/3/99 defect 200903: When a world space modifier has been applied, and the
		// node has been moved (or the pivot is not at world Z), the bbox we get back
		// is transformed---it doesn't match reality. There is a chance this is a bug
		// in the map scaler modifier. However, I am less sure of making that change.
		// I have to do some weird checking to get this to work.
		ViewExp* view = GetCOREInterface()->GetActiveViewport();
		Box3 bbox;
		if (m_curNode->GetObjectRef() != m_curNode->GetObjOrWSMRef() && os.TMIsIdentity())
		{
			geom_obj->GetLocalBoundBox( m_curTime, m_curNode, view, bbox );
		}
		else
		{
			geom_obj->GetWorldBoundBox( m_curTime, m_curNode, view, bbox );
		}
		GetCOREInterface()->ReleaseViewport(view);

		// transform the corners of the object's world bbox in slice space
		// store into m_tm_bbox and get min, max z
		float minz = MAX_FLOAT;
		float maxz = -MAX_FLOAT;
		for (int jj = 0; jj < 8; jj++)
		{
			m_tm_bbox[jj] = m_sliceTM_inv * bbox[jj];
			if (m_tm_bbox[jj].z < minz) minz = m_tm_bbox[jj].z;
			if (m_tm_bbox[jj].z > maxz) maxz = m_tm_bbox[jj].z;
		}

		// see if we touch or intersect the bounding box
		int sminz = SIGN(minz);
		int smaxz = SIGN(maxz);
		if ( sminz == 0 || smaxz == 0 || sminz != smaxz )
		{
			// m_curMesh = geom_obj->GetRenderMesh(m_curTime, m_curNode, nullView, needDelete);
			// No - this takes render geometry, not viewport geometry.
			// Fixed by CCJ. 7/15/97
			BOOL needDelete = FALSE;
			TriObject* tri = GetTriObjectFromNode(m_curNode, m_curTime, needDelete);
			if (tri) 
			{
				m_curMesh = &tri->GetMesh();
				if (m_curMesh) 
				{
					DoMeshSlice();
				}

				if (needDelete) 
				{
					delete tri;
					tri = NULL;
				}
			}
		}
	}
}

//-----------------------------------------------------------------
// checks that the two points cross the XY plane.  
// If so, computes the crossing point.
// 
// return values:
//		2  both points are on the plane (cross_pt is not changed)
//		1  cross_pt contains the one point of intersection
//		0  no points of intersection (cross_pt not changed)
//
int SectionObject::CrossCheck(const Point3& pt1, const Point3& pt2, Point3* cross_pt)
{
	const float	z1 = pt1.z;
	const float	z2 = pt2.z;
	const int	sign_z1 = SIGN(z1);
	const int	sign_z2 = SIGN(z2);
	float		dd;

	// check for corners straddling the plane
	if (sign_z1 == 0 && sign_z2 == 0)
		return 2;
	else if (sign_z1 != sign_z2 )
	{
		// straddle!  compute intersection pt
		if (cross_pt)
		{
			dd = z1 / (z1 - z2);
			cross_pt->x = pt1.x + dd * (pt2.x - pt1.x);
			cross_pt->y = pt1.y + dd * (pt2.y - pt1.y);
			cross_pt->z = float(0);
		}
		return 1;
	}

	return 0;
}

//-----------------------------------------------------------------
// called when we know  that the two points cross the XY plane.  
// computes the crossing point.
// 
void SectionObject::CrossPoint(const Point3& pt1, const Point3& pt2, Point3* cross_pt)
{
	const float	z1 = pt1.z;
	const float	z2 = pt2.z;
	const int	sign_z1 = SIGN(z1);
	const int	sign_z2 = SIGN(z2);
	float		dd;

	assert(SIGN(pt1.z) != SIGN(pt2.z));

	dd = z1 / (z1 - z2);
	cross_pt->x = pt1.x + dd * (pt2.x - pt1.x);
	cross_pt->y = pt1.y + dd * (pt2.y - pt1.y);
	cross_pt->z = float(0);
}


//-------------------------------------------------------------------
// Slice the current mesh 
//
// 1. transform the vertices of the mesh to slice space
// 2. build the adjacency edge list
// 3. create a boolean face_done[num_faces_of_mesh] array
// 4. for each edge in the mesh, 
//       if intersects slice, 
//          call SliceFace on the defined and not done faces for this edge
//
void SectionObject::DoMeshSlice(void)
{
	const Matrix3 objToWorldTM = m_curNode->GetObjTMAfterWSM(m_curTime);
	const Matrix3 objToSliceTM = objToWorldTM * m_sliceTM_inv;

	// make and fill an array of mesh points in slice coordinate system
	// also compute the sign's of all the z coords
	// TODO: move this memory allocation outside object traversal
	Point3* tm_verts  = new Point3[m_curMesh->numVerts];
	assert(tm_verts);

	for (int jj = 0; jj < m_curMesh->numVerts; jj++)
	{
		tm_verts[jj] = objToSliceTM * m_curMesh->getVert(jj);
	}

	// I profiled both of the following versions of the
	// slice algorithm.  The first is to do the expensive operation of
	// building the adjacent edge list and then traversing the
	// adjacency edge list.  The second is to slice all the faces,
	// and turned out to be faster.
	//

#ifdef ADJ_EDGES

	int			ee,num_crosses;
	DWORD		v1_index;
	DWORD		v2_index;
	char*	face_done = new char[m_curMesh->numFaces];
	assert(face_done);

	for (jj = 0; jj < m_curMesh->numFaces; jj++)
		face_done[jj] = FALSE;

	// Build the adjacency edge list
	// TODO - cache this
	// (this is expensive)
	const AdjEdgeList aelist = AdjEdgeList(*m_curMesh);

	// for each edge, see if crosses plane
	for (ee = 0; ee < aelist.edges.Count(); ee++)
	{
		v1_index	= aelist.edges[ee].v[0];
		v2_index	= aelist.edges[ee].v[1];

		assert(v1_index < DWORD(m_curMesh->numVerts));
		assert(v2_index < DWORD(m_curMesh->numVerts));

		num_crosses = CrossCheck(tm_verts[v1_index],tm_verts[v2_index],NULL);
		if (num_crosses != 0)
		{
			DWORD f1		= aelist.edges[ee].f[0];
			DWORD f2		= aelist.edges[ee].f[1];

			if (f1 != UNDEF && !face_done[f1])
			{
				SliceFace(tm_verts, f1);
				face_done[f1] = TRUE;
			}
			if (f2 != UNDEF && !face_done[f2])
			{
				SliceFace(tm_verts, f2);
				face_done[f2] = TRUE;
			}
		}
	}
	delete[] face_done;

#else ADJ_EDGES

	for (int jj = 0; jj < m_curMesh->numFaces; jj++)
	{
		if (!(m_curMesh->faces[jj].flags & FACE_HIDDEN))
		{
			SliceFace(tm_verts, jj);
		}
	}
#endif ADJ_EDGES

	delete[] tm_verts;
	tm_verts = NULL;
}

//-------------------------------------------------------------------
// slice the cutting plane thru a mesh face
//
// if the face is on the slice, add all the edges to the slice edge list
// if one edge is on the slice, add that edge
// if one vertex is on the slice, see if the opposite edge is sliced
//     if so, add that segment
// if no vertices are on the slice, find the two points of intersection and
//		add the segment between those two points
// 
// add the resulting line segment(s) to the section object's shape list 
//
void SectionObject::SliceFace(Point3* tm_verts, const DWORD f_index)
{
	Face*		face;						// the face being intersected
	Point3*		vv[3];						// pointers to vertices of each face
	int			jj, num_coplanar;
	int			cnt,check;
	int			nonz_verts[2];
	int			sign_vvz[3];
	Point3		crossPt;
#ifndef ADJ_EDGES
	int			num_neg=0, num_pos=0;
#endif
	face	= &m_curMesh->faces[f_index];

	// find number of coplanar vertices 
	// store sign of vertices z values in sign_vvz
	num_coplanar = 0;
	for (jj = 0; jj < 3; ++jj)
	{
		vv[jj] = &tm_verts[face->v[jj]];
		sign_vvz[jj] = SIGN(vv[jj]->z);
		if (sign_vvz[jj] == 0)
			num_coplanar++;
#ifndef ADJ_EDGES
		else if (sign_vvz[jj] < 0)
			num_neg++;
		else
			num_pos++;
#endif
	}

#ifndef ADJ_EDGES
	// if all on one side or other, return
	//
	// TODO - could check that crossing edges are visible
	if (num_neg==3 || num_pos==3)
		return;
#endif

	// decide which segment to add based on how many vertices are coplanar to the slice
	switch (num_coplanar)
	{
	case 3:


		// the whole face is on the slice
		// add all edges to the segment list (as long as they're visible)
		cnt = 0;
		if (face->getEdgeVis(0))
		{
			m_facelines[cnt].pts[0] = *vv[0];
			m_facelines[cnt].pts[1] = *vv[1];
			cnt++;
		}
		if (face->getEdgeVis(1))
		{
			m_facelines[cnt].pts[0] = *vv[1];
			m_facelines[cnt].pts[1] = *vv[2];
			cnt++;
		}
		if (face->getEdgeVis(2))
		{
			m_facelines[cnt].pts[0] = *vv[2];
			m_facelines[cnt].pts[1] = *vv[0];
			cnt++;
		}


			for (jj=0; jj<cnt; jj++)
			{
				m_ptlist.Append(1, &m_facelines[jj].pts[0].p, 256);
				m_ptlist.Append(1, &m_facelines[jj].pts[1].p, 256);
#ifdef DEBUG_SLICE
				DebugPrint(_T("face slice: segment #%d\t(%5.2f %5.2f %5.2f <--> %5.2f %5.2f %5.2f)\n"),
					m_ptlist.cnt/2,
					m_facelines[jj].pts[0].p.x,
					m_facelines[jj].pts[0].p.y,
					m_facelines[jj].pts[0].p.z,
					m_facelines[jj].pts[1].p.x,
					m_facelines[jj].pts[1].p.y,
					m_facelines[jj].pts[1].p.z);
#endif DEBUG_SLICE
		}
		break;

	case 2:
		// one edge is on the slice
		// add the edge who's z values are zero (as long as it's visible)
		//SS 8/5/99 defect 202020: Why should we exclude the creation of a
		// section line just because the face edge is invisible? When you 
		// have an edge that is exactly on the plane, but invisible, and
		// you exclude it, you get a discontinuous shape around the object.
		cnt = 0;
		if (sign_vvz[0]==0)
			m_facelines[0].pts[cnt++] = *vv[0];
//		else if (!face->getEdgeVis(1))
//			break;
		if (sign_vvz[1]==0)
			m_facelines[0].pts[cnt++] = *vv[1];
//		else if (!face->getEdgeVis(0))
//			break;
		if (sign_vvz[2]==0)
			m_facelines[0].pts[cnt++] = *vv[2];
//		else if (!face->getEdgeVis(2))
//			break;

		assert(cnt==2);

		m_ptlist.Append(1, &m_facelines[0].pts[0].p, 256);
		m_ptlist.Append(1, &m_facelines[0].pts[1].p, 256);

#ifdef DEBUG_SLICE
		DebugPrint(_T("edge slice: segment #%d\t(%5.2f %5.2f %5.2f <--> %5.2f %5.2f %5.2f)\n"),
				m_ptlist.cnt/2,
				m_facelines[jj].pts[0].p.x,
				m_facelines[jj].pts[0].p.y,
				m_facelines[jj].pts[0].p.z,
				m_facelines[jj].pts[1].p.x,
				m_facelines[jj].pts[1].p.y,
				m_facelines[jj].pts[1].p.z);
#endif DEBUG_SLICE

		break;

	case 1:
		// one vertex is on the slice
		// if the signs of the other vertices are not different,
		//      have a single point intersection and ignore it
		// otherwise, 
		//      compute the intersection point between the vertices
		//      add line between intersection point and point on slice
		cnt=0;
		if (sign_vvz[0]!=0) nonz_verts[cnt++] = 0;
		else m_facelines[0].pts[0] = *vv[0];
		if (sign_vvz[1]!=0) nonz_verts[cnt++] = 1;
		else m_facelines[0].pts[0] = *vv[1];
		if (sign_vvz[2]!=0) nonz_verts[cnt++] = 2;
		else m_facelines[0].pts[0] = *vv[2];

		assert(cnt==2);
		assert(sign_vvz[nonz_verts[0]]!=0 && sign_vvz[nonz_verts[1]]!=0);

		if (sign_vvz[nonz_verts[0]] == sign_vvz[nonz_verts[1]])
			break;

		CrossPoint(*vv[nonz_verts[0]], *vv[nonz_verts[1]], &crossPt);

		m_facelines[0].pts[1] = PolyPt(crossPt);

		m_ptlist.Append(1, &m_facelines[0].pts[0].p, 256);
		m_ptlist.Append(1, &m_facelines[0].pts[1].p, 256);

#ifdef DEBUG_SLICE
		DebugPrint(_T("vertex slice: segment #%d\t(%5.2f %5.2f %5.2f <--> %5.2f %5.2f %5.2f)\n"),
				m_ptlist.cnt/2,
				m_facelines[jj].pts[0].p.x,
				m_facelines[jj].pts[0].p.y,
				m_facelines[jj].pts[0].p.z,
				m_facelines[jj].pts[1].p.x,
				m_facelines[jj].pts[1].p.y,
				m_facelines[jj].pts[1].p.z);
#endif
		break;

	case 0:
		// no vertices are on the plane
		// look for zero or two intersection points
		cnt = 0;

		check = CrossCheck(*vv[0], *vv[1], &crossPt);
		assert(check != 2);
		if (check==1) m_facelines[0].pts[cnt++] = PolyPt(crossPt);

		check = CrossCheck(*vv[0], *vv[2], &crossPt);
		assert(check != 2);
		if (check==1) m_facelines[0].pts[cnt++] = PolyPt(crossPt);

		check = CrossCheck(*vv[1], *vv[2], &crossPt);
		assert(check != 2);
		if (check==1) m_facelines[0].pts[cnt++] = PolyPt(crossPt);

		assert(cnt==0 || cnt == 2);

		if (cnt==2)
		{
			m_ptlist.Append(1, &m_facelines[0].pts[0].p, 256);
			m_ptlist.Append(1, &m_facelines[0].pts[1].p, 256);

#ifdef DEBUG_SLICE 
				DebugPrint(_T("air slice: segment #%d\t(%5.2f %5.2f %5.2f <--> %5.2f %5.2f %5.2f)\n"),
					m_ptlist.cnt/2,
					m_facelines[jj].pts[0].p.x,
					m_facelines[jj].pts[0].p.y,
					m_facelines[jj].pts[0].p.z,
					m_facelines[jj].pts[1].p.x,
					m_facelines[jj].pts[1].p.y,
					m_facelines[jj].pts[1].p.z);
#endif DEBUG_SLICE
		}
		break;

	default:
		assert(FALSE);
	}
}

//-------------------------------------------------------------------
//
//  Since the slicing creates a million two point spline segments, we
//  want to clean this up
//  this weld operation is fairly stupid
//  here's what it does
//  1.  remove null segments
//  2.  remove duplicate segments
//  3.  join touching segments, using a greedy algorithm
//  4.  determine if the result is closed or open, and delete the last point for 
//      closed splines
//  5.  merge colinear segments
//
//  It doesn't take care of folds, where one segment reverses directly over another
//  And the greedy algorithm results in stupid sections for some cases, esp. patches

#define	s1p1 (shape.lines[s1].pts[0].p)
#define	s1p2 (shape.lines[s1].pts[1].p)
#define	s1plast (shape.lines[s1].pts[shape.lines[s1].numPts-1].p)
#define	s2p1 (shape.lines[s2].pts[0].p)
#define	s2p2 (shape.lines[s2].pts[1].p)


// the progress values must add up to 1000
#define PROG_NULL	 10
#define PROG_DUP	190
#define PROG_JOIN	650
#define PROG_MERGE	150

DWORD WINAPI fn(LPVOID arg) {
	return(0);
}

void SectionObject::WeldSlice()
{
	int				s1,s2,newper,oldper=-1;
	BOOL			p1p1_same, p1p2_same, p2p1_same, p2p2_same, joined_something;
	int				p1,p2,p3;
	Point3			v1,v2;
	TCHAR			buf[128];

	assert(m_ip);
	if (m_sliceWelded)
		return;

	HCURSOR oldcur = SetCursor(m_waitCursor);

	// announce to status bar
	m_ip->PushPrompt(_T(""));

	// remove null segments
	for (s1 = 0; s1 < shape.numLines; )
	{
		newper = int(s1/float(shape.numLines-1)*PROG_NULL);
		if (newper != oldper)
		{
			_stprintf(buf, GetString(IDS_WELDING_MESSAGE),double(newper)/10.0f);
			m_ip->ReplacePrompt(buf);
			oldper = newper;
		}
#ifdef DEBUG_SLICE
		DebugPrint(_T("seg #%d, s1p1.x-s1p2.x = %10.8f\n"), s1, double(s1p1.x-s1p2.x));
		DebugPrint(_T("         s1p1.y-s1p2.y = %10.8f\n"), double(s1p1.y-s1p2.y));
#endif

		if (WELD_SIGN(s1p1.x-s1p2.x)==0 && WELD_SIGN(s1p1.y-s1p2.y)==0)
		{
			shape.Delete(s1);
#ifdef DEBUG_WELD
			DebugPrint(_T("section weld: removing null segment (#%d)\n"),s1);
#endif
		}

		else
			++s1;
	}

	// remove duplicate segments
	for (s1 = 0; s1 < shape.numLines; ++s1)
	{
		newper = int(s1/float(shape.numLines-1)*PROG_DUP + PROG_NULL);
		if (newper != oldper)
		{
			_stprintf(buf, GetString(IDS_WELDING_MESSAGE),double(newper)/10.0f);
			m_ip->ReplacePrompt(buf);
			oldper = newper;
		}
		for (s2 = s1+1; s2 < shape.numLines; )
		{
			p1p1_same = WELD_SIGN(s1p1.x-s2p1.x)==0 && WELD_SIGN(s1p1.y-s2p1.y)==0;
			p1p2_same = WELD_SIGN(s1p1.x-s2p2.x)==0 && WELD_SIGN(s1p1.y-s2p2.y)==0;
			p2p1_same = WELD_SIGN(s1p2.x-s2p1.x)==0 && WELD_SIGN(s1p2.y-s2p1.y)==0;
			p2p2_same = WELD_SIGN(s1p2.x-s2p2.x)==0 && WELD_SIGN(s1p2.y-s2p2.y)==0;

			if (p1p1_same && p2p2_same || p1p2_same && p2p1_same)
			{
				shape.Delete(s2);
#ifdef DEBUG_WELD
				DebugPrint(_T("section weld: segment %d:  removing duplicate seg %d\n"),s1,s2);
#endif
			}
			else
				++s2;
		}
	}

	// join touching segments
	float orignum = float(shape.numLines);
	for (s1 = 0; s1 < shape.numLines-1; )
	{
		joined_something = FALSE;

		for (s2 = s1+1; s2 < shape.numLines; )
		{
			newper = int((orignum - shape.numLines)/(orignum-1)*PROG_JOIN + PROG_DUP + PROG_NULL);
			if (newper != oldper)
			{
				_stprintf(buf, GetString(IDS_WELDING_MESSAGE),double(newper)/10.0f);
				m_ip->ReplacePrompt(buf);
				oldper = newper;
			}

#ifdef DEBUG_WELD
			DebugPrint(_T("lines[%d]: examining lines[%d]\n"),s1,s2);
			shape.lines[s1].Dump();
#endif
			p1p1_same = WELD_SIGN(s1p1.x-s2p1.x)==0 && WELD_SIGN(s1p1.y-s2p1.y)==0;
			p1p2_same = WELD_SIGN(s1p1.x-s2p2.x)==0 && WELD_SIGN(s1p1.y-s2p2.y)==0;
			p2p1_same = WELD_SIGN(s1plast.x-s2p1.x)==0 && WELD_SIGN(s1plast.y-s2p1.y)==0;
			p2p2_same = WELD_SIGN(s1plast.x-s2p2.x)==0 && WELD_SIGN(s1plast.y-s2p2.y)==0;

			joined_something |= p1p1_same || p1p2_same || p2p1_same || p2p2_same;

			if (p1p1_same)
			{
				shape.lines[s1].Insert(0,shape.lines[s2].pts[1]);
				shape.Delete(s2);
#ifdef DEBUG_WELD
				DebugPrint(_T("section weld: to segment %d, prepended segment %d pt2\n"),s1,s2);
#endif
			}
			else if (p1p2_same)
			{
				shape.lines[s1].Insert(0,shape.lines[s2].pts[0]);
				shape.Delete(s2);
#ifdef DEBUG_WELD
				DebugPrint(_T("section weld: to segment %d, prepended segment %d pt1\n"),s1,s2);
#endif
			}
			else if (p2p1_same)
			{
				shape.lines[s1].Append(shape.lines[s2].pts[1]);
				shape.Delete(s2);
#ifdef DEBUG_WELD
				DebugPrint(_T("section weld: to segment %d, appended segment %d pt2\n"),s1,s2);
#endif
			}
			else if (p2p2_same)
			{
				shape.lines[s1].Append(shape.lines[s2].pts[0]);
				shape.Delete(s2);
#ifdef DEBUG_WELD
				DebugPrint(_T("section weld: to segment %d, appended segment %d pt1\n"),s1,s2);
#endif
			}
			else
				++s2;

		}

		if (!joined_something)
			s1++;
	}


	// set closed flag
	for (s1 = 0; s1 < shape.numLines; ++s1)
	{
#ifdef DEBUG_WELD
		DebugPrint(_T("seg #%d, s1p1.x-s1p2.x = %10.8f\n"), s1, double(s1p1.x-s1p2.x));
		DebugPrint(_T("         s1p1.y-s1p2.y = %10.8f\n"), double(s1p1.y-s1p2.y));
#endif

		if (shape.lines[s1].IsClosed())
		{
#ifdef DEBUG_WELD
			DebugPrint(_T("   already closed\n"));
#endif
			continue;
		}

		if (shape.lines[s1].numPts>2 && WELD_SIGN(s1p1.x-s1plast.x)==0 && WELD_SIGN(s1p1.y-s1plast.y)==0)
		{
#ifdef DEBUG_WELD
			DebugPrint(_T("section weld: setting spline closed(#%d)\nhere's the pts before deleting last pt:\n"),s1);
			shape.lines[s1].Dump();
#endif
			shape.lines[s1].Delete(shape.lines[s1].numPts-1);
			shape.lines[s1].Close();
		}
		else
		{
			shape.lines[s1].Open();
#ifdef DEBUG_WELD
			DebugPrint(_T("section weld: setting spline open(#%d)\n"),s1);
#endif
		}
	}

	// merge colinear segments
	// make a pass thru all the lines
	// for each line, check all the triplet points in the line
	// if the line with N points is open, go up to n-2, n-1, n
	// if the line is closed, go up to n, 0, 1
	// delete the middle point if each adjacent edge is colinear

	for (s1 = 0; s1 < shape.numLines; ++s1)
	{
		for (p1 = 0; p1 < (shape.lines[s1].IsOpen() ? shape.lines[s1].numPts - 2 : shape.lines[s1].numPts); )
		{
			newper = int( (s1 + p1/float(shape.lines[s1].numPts-1))
							/ float(shape.numLines) * PROG_MERGE 
							+ PROG_JOIN + PROG_DUP + PROG_NULL);
			if (newper != oldper)
			{
				_stprintf(buf, GetString(IDS_WELDING_MESSAGE),double(newper)/10.0f);
				m_ip->ReplacePrompt(buf);
				oldper = newper;
			}

			p2 = (p1 + 1) % shape.lines[s1].numPts;
			p3 = (p2 + 1) % shape.lines[s1].numPts;
			assert(shape.lines[s1].IsOpen() && p1 < p2 && p2 < p3 || shape.lines[s1].IsClosed());
			v1 = Normalize(shape.lines[s1].pts[p2].p - shape.lines[s1].pts[p1].p);
			v2 = Normalize(shape.lines[s1].pts[p3].p - shape.lines[s1].pts[p2].p);
			
#ifdef DEBUG_MERGE
			//DebugPrint(_T("weld colinear shape #%d, dump follows:\n"), s1);
			//shape.lines[s1].Dump();
			DebugPrint(_T("weld colinear checking shape #%d pts %d %d %d, v1.x-v2.x = %10.7f, v1.y-v2.y = %10.7f:\n"), 
				s1,p1,p2,p3,v1.x - v2.x,v1.y - v2.y);
#endif
			if (WELD_SIGN(v1.x - v2.x) == 0 && WELD_SIGN(v1.y - v2.y) == 0)
			{
#ifdef DEBUG_MERGE
				DebugPrint(_T("weld colinear shape #%d:  about to delete point %d\n"), s1,p2);
#endif
				shape.lines[s1].Delete(p2);
			}
			else
				++p1;
		}
	}

	m_sliceWelded = TRUE;

#ifndef CREATE_SECTION
	ICustButton*	iBut;
	HWND			uiHwnd;

	assert(m_pmapCreate);
	uiHwnd = m_pmapCreate->GetHWnd();
	assert(uiHwnd);
	uiHwnd = GetDlgItem(uiHwnd, IDC_SECTION_WELD);
	assert(uiHwnd);
	iBut = GetICustButton(uiHwnd);			
	iBut->Disable();
	ReleaseICustButton(iBut);
#endif

	m_ip->PopPrompt();
	m_ip->DisplayTempPrompt(GetString(IDS_WELDED_MESSAGE),3000);
	SetCursor(oldcur);
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
//-------------------------------------------------------------------

void SectionObject::UpdateMesh(TimeValue t) 
{
	float stuff;

	// Start the validity interval at forever and whittle it down.
	ivalid = FOREVER;
	m_pblock->GetValue(PB_LENGTH, t, stuff, ivalid);
	m_pblock->GetValue(PB_WIDTH,  t, stuff,  ivalid);

	// Test
	ivalid.SetInstant(t);
}

// --------  pmap stuff --------------------------------
void SectionObject::InvalidateUI()
{
	if (m_pmapParam) 
		m_pmapParam->Invalidate();
}

ParamDimension *SectionObject::GetParameterDim(int pbIndex)
{
	switch (pbIndex)
	{
	case PB_OBJSLICE_COLOR:
		return stdColorDim;
		
	case PB_UPDATE_METHOD:
		return defaultDim;

	case PB_SECTION_SCOPE:
		return defaultDim;

	default:
		return defaultDim;
	}
}

TSTR SectionObject::GetParameterName(int pbIndex)
{
	switch (pbIndex)
	{
	case PB_LENGTH:
		return TSTR(GetString( IDS_PARAM_LENGTH ));
	case PB_WIDTH:
		return TSTR(GetString( IDS_PARAM_WIDTH ));
	default:
		return TSTR(_T(""));
	}
}


//------------------------------------------------------------------------

class SectionObjCreateCallBack: public CreateMouseCallBack {
	SectionObject *ob;
	Point3 p0,p1;
	IPoint2 sp1, sp0;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(SectionObject *obj) { ob = obj; }
	};


//-------------------------------------------------------------------
int SectionObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) 
{
	Point3 d;
#ifdef _3D_CREATE
	DWORD snapdim = SNAP_IN_3D;
#else
	DWORD snapdim = SNAP_IN_PLANE;
#endif

#ifdef _OSNAP
	if (msg == MOUSE_FREEMOVE)
	{
		vpt->SnapPreview(m,m,NULL, snapdim);
	}
#endif

	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
			case 0:
				sp0 = m;
				p0 = vpt->SnapPoint(m,m,NULL,snapdim);
				mat.SetTrans(p0);

				ob->m_pblock->SetValue(PB_WIDTH, 0, float(0) );
				ob->m_pblock->SetValue(PB_LENGTH, 0, float(0) );

				//ob->m_pmapSection->Invalidate();										
				//ob->ivalid.SetEmpty();
				break;

			case 1:
				sp1 = m;
				p1 = vpt->SnapPoint(m,m,NULL,snapdim);
				d = p1-p0;

				// Creating the section corner to corner causes the
				// pivot point to be moved (centered) every mouse move.
				// Since this causes re-evaluation of the section shape
				// it's a bad idea.
#define CREATION_CENTER_OUT 1

#ifdef	CREATION_CENTER_OUT
				ob->m_pblock->SetValue(PB_WIDTH, 0, float(fabs(d.x)*2) );
				ob->m_pblock->SetValue(PB_LENGTH, 0, float(fabs(d.y)*2) );
#else	// Corner to corner
				ob->m_pblock->SetValue(PB_WIDTH, 0, float(fabs(d.x)));
				ob->m_pblock->SetValue(PB_LENGTH, 0, float(fabs(d.y)));
				mat.SetTrans((p0+p1)*0.5f);
#endif

				ob->ivalid.SetEmpty();
				ob->InvalidateUI();										

				if (msg==MOUSE_POINT) {
					if (Length(sp1-sp0) < 4) {
						return CREATE_ABORT;
					}
					else				
					{
						//ob->m_pleaseUpdate = FALSE;
						return CREATE_STOP;
					}
				}
				break;
			}
		}
	else
	if (msg == MOUSE_ABORT)
		return CREATE_ABORT;

	return TRUE;
	}

//-------------------------------------------------------------------

static SectionObjCreateCallBack sectionCreateCB;

//-------------------------------------------------------------------

CreateMouseCallBack* SectionObject::GetCreateMouseCallBack() 
{
	sectionCreateCB.SetObj(this);
	return(&sectionCreateCB);
}

	
//-------------------------------------------------------------------
void SectionObject::GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box )
{
	UpdateSlice(inode,t);

	if (shape.numLines > 0) {
		box = shape.GetBoundingBox();
		}
	else {
		box.pmin = Point3(0,0,0);
		box.pmax = Point3(0,0,0);
		}

	float length, width;
	Point2 vert[2];

	ivalid = FOREVER;
	m_pblock->GetValue(PB_LENGTH, t, length, ivalid);
	m_pblock->GetValue(PB_WIDTH, t, width, ivalid);

	vert[0].x = -width/float(2);
	vert[0].y = -length/float(2);
	vert[1].x = width/float(2);
	vert[1].y = length/float(2);

	box += Point3( vert[0].x, vert[0].y, (float)0 );
	box += Point3( vert[1].x, vert[0].y, (float)0 );
	box += Point3( vert[0].x, vert[1].y, (float)0 );
	box += Point3( vert[1].x, vert[1].y, (float)0 );
}

//-------------------------------------------------------------------
void SectionObject::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box )
{
	GetLocalBoundBox(t,inode,vpt,box);
	box = box * inode->GetObjectTM(t); // m_sliceTM;//inode->GetObjectTM(t);
}

//-------------------------------------------------------------------
// Get the transform for this view
void SectionObject::GetConstructionTM( TimeValue t, INode* inode, ViewExp *vpt, Matrix3 &tm ) {
	tm = inode->GetObjectTM(t);
	}


//-------------------------------------------------------------------
// Get snap values
Point3 SectionObject::GetSnaps( TimeValue t ) 
{
	assert(FALSE);
	float snap = 1.0f;//GetSection(t);
	return Point3(snap,snap,snap);
}


//-------------------------------------------------------------------
int SectionObject::Select(TimeValue t, INode *inode, GraphicsWindow *gw, 
						  Material *mtl, HitRegion *hr, int abortOnHit ) 
{
	DWORD	savedLimits;
	Matrix3 tm;
	float width, w2, height, h2;

	m_pblock->GetValue(PB_LENGTH, t, height, FOREVER);
	m_pblock->GetValue(PB_WIDTH, t, width, FOREVER);

	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	gw->setHitRegion(hr);
	gw->clearHitCode();
	gw->setMaterial(*mtl);

	w2 = width / (float)2;
	h2 = height / (float)2;
		
	Point3 pt[3];

	if(!inode->IsActiveGrid()) {

		pt[0] = Point3( -w2, -h2,(float)0.0);
		pt[1] = Point3( -w2, h2,(float)0.0);
		gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );

		if((hr->type != POINT_RGN) && !hr->crossing) {	
			// window select needs *every* face to be enclosed
			if(gw->checkHitCode())
				gw->clearHitCode();
			else
				return FALSE;
			}
				
		if ( abortOnHit ) {
			if(gw->checkHitCode()) {
				gw->setRndLimits(savedLimits);
				return TRUE;
				}
			}

		pt[0] = Point3( w2, -h2,(float)0.0);
		pt[1] = Point3( w2, h2,(float)0.0);
		gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );
		
		if((hr->type != POINT_RGN) && !hr->crossing) {
			if(gw->checkHitCode())
				gw->clearHitCode();
			else
				return FALSE;
			}
			
		if ( abortOnHit ) {
			if(gw->checkHitCode()) {
				gw->setRndLimits(savedLimits);
				return TRUE;
				}
			}

		pt[0] = Point3( -w2, -h2,(float)0.0);
		pt[1] = Point3( w2, -h2,(float)0.0);
		gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );
		
		if((hr->type != POINT_RGN) && !hr->crossing) {
			if(gw->checkHitCode())
				gw->clearHitCode();
			else
				return FALSE;
			}
			
		if ( abortOnHit ) {
			if(gw->checkHitCode()) {
				gw->setRndLimits(savedLimits);
				return TRUE;
				}
			}

		pt[0] = Point3( -w2, h2,(float)0.0);
		pt[1] = Point3( w2, h2,(float)0.0);
		gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );
		
		if((hr->type != POINT_RGN) && !hr->crossing) {
			if(gw->checkHitCode())
				gw->clearHitCode();
			else
				return FALSE;
			}
			
		if ( abortOnHit ) {
			if(gw->checkHitCode()) {
				gw->setRndLimits(savedLimits);
				return TRUE;
				}
			}

		pt[0] = Point3( -w2, (float)0, (float)0.0);
		pt[1] = Point3( w2, (float)0, (float)0.0);
		gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );
		
		if((hr->type != POINT_RGN) && !hr->crossing) {
			if(gw->checkHitCode())
				gw->clearHitCode();
			else
				return FALSE;
			}
			
		if ( abortOnHit ) {
			if(gw->checkHitCode()) {
				gw->setRndLimits(savedLimits);
				return TRUE;
				}
			}

		pt[0] = Point3( (float)0, -h2,(float)0.0);
		pt[1] = Point3( (float)0, h2,(float)0.0);
		gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );
		
		if((hr->type != POINT_RGN) && !hr->crossing) {
			if(gw->checkHitCode())
				gw->clearHitCode();
			else
				return FALSE;
			}
			
		if ( abortOnHit ) {
			if(gw->checkHitCode()) {
				gw->setRndLimits(savedLimits);
				return TRUE;
				}
			}
		}
	else {
		assert(FALSE);
		float section = 1.0f; //GetSection(t);
		int xSteps = (int)floor(w2 / section);
		int ySteps = (int)floor(h2 / section);
		float minX = (float)-xSteps * section;
		float maxX = (float)xSteps * section;
		float minY = (float)-ySteps * section;
		float maxY = (float)ySteps * section;
		float x,y;
		int ix;

		// Adjust steps for whole range
		xSteps *= 2;
		ySteps *= 2;

		// First, the vertical lines
		pt[0].y = minY;
		pt[0].z = (float)0;
		pt[1].y = maxY;
		pt[1].z = (float)0;

		for(ix=0,x=minX; ix<=xSteps; x+=section,++ix) {
			pt[0].x = pt[1].x = x;
			gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );
		
			if((hr->type != POINT_RGN) && !hr->crossing) {
				if(gw->checkHitCode())
					gw->clearHitCode();
				else
					return FALSE;
				}
			
			if ( abortOnHit ) {
				if(gw->checkHitCode()) {
					gw->setRndLimits(savedLimits);
					return TRUE;
					}
				}
   			}

		// Now, the horizontal lines
		pt[0].x = minX;
		pt[0].z = (float)0;
		pt[1].x = maxX;
		pt[1].z = (float)0;

		for(ix=0,y=minY; ix<=ySteps; y+=section,++ix) {
			pt[0].y = pt[1].y = y;
			gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );
		
			if((hr->type != POINT_RGN) && !hr->crossing) {
				if(gw->checkHitCode())
					gw->clearHitCode();
				else
					return FALSE;
				}
			
			if ( abortOnHit ) {
				if(gw->checkHitCode()) {
					gw->setRndLimits(savedLimits);
					return TRUE;
					}
				}
			}
		}

	if((hr->type != POINT_RGN) && !hr->crossing)
		return TRUE;
	return gw->checkHitCode();	
	}
	

//-------------------------------------------------------------------
// From BaseObject
int SectionObject::HitTest(TimeValue t, INode *inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt) {
	Matrix3 tm;	
	HitRegion hitRegion;
	GraphicsWindow *gw = vpt->getGW();	
	Material *mtl = gw->getMaterial();

   	tm = myTM * (inode->GetObjectTM(t));
	UpdateMesh(t);
	gw->setTransform(tm);

	MakeHitRegion(hitRegion, type, crossing, 4, p);

	return shape.Select(gw, mtl, &hitRegion, flags & HIT_ABORTONHIT) 
		   || Select(t, inode, gw, mtl, &hitRegion, flags & HIT_ABORTONHIT );
	}


//-------------------------------------------------------------------
static float SectionCoord(float v, float g) 
{
	float r = (float)(int((fabs(v)+0.5f*g)/g))*g;	
	return v<0.0f ? -r : r;
	}			

//-------------------------------------------------------------------
void SectionObject::Snap(TimeValue t, INode* inode, SnapInfo *info, IPoint2 *p, ViewExp *vpt)
{
	// shape snap:
	Matrix3 tm = inode->GetObjectTM(t);	
	GraphicsWindow *gw = vpt->getGW();
	gw->setTransform(tm);
	shape.Snap( gw, info, p, tm );
}

//-------------------------------------------------------------------

int SectionObject::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) 
{
	Matrix3 tm;
	float width, w2, height, h2;
	static int cnt = 0;

	//DebugPrint(_T("in Display, cnt = %d\n"),++cnt);
	
	ivalid = FOREVER;
	m_pblock->GetValue(PB_LENGTH, t, height, FOREVER);
	m_pblock->GetValue(PB_WIDTH, t, width, FOREVER);

	//DebugPrint(_T("in Display, this = 0x%x height = %f, width = %f\n"), (int)this, height, width);

	w2 = width / (float)2;
	h2 = height / (float)2;

	m_lastNodeTM = inode->GetNodeTM(t);
		
	GraphicsWindow *gw = vpt->getGW();
	Material *mtl = gw->getMaterial();

   	tm = myTM * (inode->GetObjectTM(t));
	UpdateMesh(t);		
	gw->setTransform(tm);

	if (inode->Selected()) 
		gw->setColor( LINE_COLOR, GetSelColor());
	else if(!inode->IsFrozen())
		gw->setColor( LINE_COLOR, mtl->Kd.x, mtl->Kd.y, mtl->Kd.z);

	Point3 pt[3];

	pt[0] = Point3( -w2, -h2,(float)0.0);
	pt[1] = Point3( -w2, h2,(float)0.0);
	gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );

	pt[0] = Point3( w2, -h2,(float)0.0);
	pt[1] = Point3( w2, h2,(float)0.0);
	gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );

	pt[0] = Point3( -w2, -h2,(float)0.0);
	pt[1] = Point3( w2, -h2,(float)0.0);
	gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );

	pt[0] = Point3( -w2, h2,(float)0.0);
	pt[1] = Point3( w2, h2,(float)0.0);
	gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );

	pt[0] = Point3( -w2, (float)0, (float)0.0);
	pt[1] = Point3( w2, (float)0, (float)0.0);
	gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );

	pt[0] = Point3( (float)0, -h2,(float)0.0);
	pt[1] = Point3( (float)0, h2,(float)0.0);
	gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );

	// draw arrow
	/*
	pt[0] = Point3( (float)0, (float)0, (float)0.0);
	pt[1] = Point3( (float)0, (float)0, z2);
	gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );

	pt[0] = Point3( (float)0, (float)0, z2);
	pt[1] = Point3( t2, (float)0, z2 - t2);
	gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );

	pt[0] = Point3( (float)0, (float)0, z2);
	pt[1] = Point3( -t2, (float)0, z2 - t2);
	gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );

	pt[0] = Point3( (float)0, (float)0, z2);
	pt[1] = Point3( (float)0, t2, z2 - t2);
	gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );

	pt[0] = Point3( (float)0, (float)0, z2);
	pt[1] = Point3( (float)0, -t2, z2 - t2);
	gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );
	*/

	/////////////////////////////////////////////////
	//
	//  Make sure the slice is up to date.
	//
	///////////////////////////////////////////////////

	UpdateSlice(inode,t);
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

	// draw the slice with the right color
	//
	Material *ma = gw->getMaterial();
	ma->Kd = Point3(m_gobj_sliceColor.r, m_gobj_sliceColor.g, m_gobj_sliceColor.b);

	shape.Render(gw, ma, 
		(flags&USE_DAMAGE_RECT) ? &vpt->GetDammageRect() : NULL, 
		COMP_ALL | (inode->Selected()?COMP_OBJSELECTED:0), 1 );	

#ifdef DEBUG_BBOX

	Box3 bbox;
	Point3 pts[18];
	Matrix3 mm=Matrix3(TRUE);

	gw->setTransform(mm);
	
	//GetWorldBoundBox(t, inode, vpt, bbox);
	//shape.GetWorldBoundBox(t, inode, vpt, bbox);
		//CoreExport Box3 GetBoundingBox(Matrix3 *tm=NULL); // RB: optional TM allows the box to be calculated in any space.
	/*
	bbox = shape.GetBoundingBox();//&m_sliceTM);
	pts[0] = bbox[0];
	pts[1] = bbox[1];
	pts[2] = bbox[3];
	pts[3] = bbox[2];
	pts[4] = bbox[0];
	pts[5] = bbox[4];
	pts[6] = bbox[5];
	pts[7] = bbox[7];
	pts[8] = bbox[6];
	pts[9] = bbox[2];
	pts[10]= bbox[3];
	pts[11]= bbox[7];
	pts[12]= bbox[5];
	pts[13]= bbox[1];
	pts[14]= bbox[0];
	pts[15]= bbox[4];
	pts[16]= bbox[6];

	gw->setColor( LINE_COLOR, 1.0f, .8f, .2f );	
	gw->polyline( 17, pts, NULL, NULL, FALSE, NULL );
	*/


	GetWorldBoundBox(t, inode, vpt, bbox);
	pts[0] = bbox[0];
	pts[1] = bbox[1];
	pts[2] = bbox[3];
	pts[3] = bbox[2];
	pts[4] = bbox[0];
	pts[5] = bbox[4];
	pts[6] = bbox[5];
	pts[7] = bbox[7];
	pts[8] = bbox[6];
	pts[9] = bbox[2];
	pts[10]= bbox[3];
	pts[11]= bbox[7];
	pts[12]= bbox[5];
	pts[13]= bbox[1];
	pts[14]= bbox[0];
	pts[15]= bbox[4];
	pts[16]= bbox[6];

	gw->setColor( LINE_COLOR, .8f, .8f, 1.0f );	
	gw->polyline( 17, pts, NULL, NULL, FALSE, NULL );
	
#endif DEBUG_BBOX
	return(0);
}

//-------------------------------------------------------------------
// From Object
ObjectHandle SectionObject::ApplyTransform(Matrix3& matrix)
{
	myTM = myTM * matrix;

	return(ObjectHandle(this));
}

//-------------------------------------------------------------------
int SectionObject::CanConvertToType(Class_ID obtype)
{
	return LinearShape::CanConvertToType(obtype);
}
//-------------------------------------------------------------------
Object* SectionObject::ConvertToType(TimeValue t, Class_ID obtype)
{
	Object* ob = NULL;

	if (obtype == splineShapeClassID) {
		// Get center point of the slice
		//Box3	boundingBox = shape.GetBoundingBox();
		//Point3	center = boundingBox.Center();

		// Create a translation matrix for each point of the slice
		//Matrix3 xlatMx = Inverse(TransMatrix(center));

		SplineShape* instance = (SplineShape *)GetCOREInterface()->CreateInstance(SHAPE_CLASS_ID, splineShapeClassID);

		for (int l=0; l<shape.numLines; l++) {
			Spline3D* spline = instance->shape.NewSpline();
			for (int k=0; k<shape.lines[l].numPts; k++) {
				// Transform each point with the inverse of the center
				Point3 pt = /*xlatMx * */shape.lines[l].pts[k].p;
				spline->AddKnot(SplineKnot(KTYPE_CORNER, LTYPE_CURVE, pt, pt, pt));
			}

			if (shape.lines[l].flags & POLYLINE_CLOSED) {
				spline->SetClosed(1);
			}
			spline->ComputeBezPoints();
		}

		instance->shape.UpdateSels();

		// CCJ - 5.12.99
		// If we invalidate here, the wrath of the pipeline Gods will be upon us.
		// instance->shape.InvalidateGeomCache();

		instance->SetChannelValidity(TOPO_CHAN_NUM,ConvertValidity(t));
		instance->SetChannelValidity(GEOM_CHAN_NUM,ConvertValidity(t));

		ob = instance;
	}
	else {
		ob = LinearShape::ConvertToType(t,obtype);
	}

	return ob;
}

//-------------------------------------------------------------------
//
// Reference Managment:
//

// This is only called if the object MAKES references to other things.
RefResult SectionObject::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
     PartID& partID, RefMessage message ) 
{
#ifdef DEBUG_REFS
	DebugPrint(_T("##SectObj Noti!  hTarget =0x%08x, message=0x%08x, partID=0x%08x\n"),
		(int)hTarget, (int)message, (int)partID);
#endif
	switch (message) 
	{
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			if (m_pmapCreate)
				m_pmapCreate->Invalidate();
			if (m_pmapParam)
				m_pmapParam->Invalidate();

			break;

		case REFMSG_GET_PARAM_DIM: 
		{
			GetParamDim *gpd = (GetParamDim*)partID;
			gpd->dim = GetParameterDim(gpd->index);			
			return REF_HALT; 
		}

		case REFMSG_GET_PARAM_NAME: 
		{
			GetParamName *gpn = (GetParamName*)partID;
			gpn->name = GetParameterName(gpn->index);			
			return REF_HALT; 
		}
	}
	return REF_SUCCEED;
}

//-------------------------------------------------------------------

ObjectState SectionObject::Eval(TimeValue time){
	return ObjectState(this);
	}

Interval SectionObject::ObjectValidity(TimeValue time) {
	UpdateMesh(time);
	return ivalid;	
	}

//-------------------------------------------------------------------

RefTargetHandle SectionObject::Clone(RemapDir& remap) {
	SectionObject* newob = new SectionObject();	
	assert(newob);
	newob->ReplaceReference(SECTION_OBJ_PARAM_BLOCK_ID,remap.CloneRef(m_pblock));
	newob->ivalid.SetEmpty();
	newob->myTM				= myTM;
	newob->m_sliceTM		= m_sliceTM;
	newob->m_curTime		= m_curTime;
	newob->m_pleaseUpdate	= m_pleaseUpdate;
	newob->shape			= shape;
	BaseClone(this, newob, remap);
	return(newob);
	}

// ------------------------------ Get/Set Value for ParamArray stuff
BOOL SectionObject::SetValue(int i, TimeValue t, Point3& v)
{
	switch (i)
	{
	case PB_OBJSLICE_COLOR:
		m_gobj_sliceColor = Color(v);
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		break;
	}
	return TRUE;
}

BOOL SectionObject::GetValue(int i, TimeValue t, Point3& v, Interval &ivalid)
{
	switch (i)
	{
	case PB_OBJSLICE_COLOR:
		v = Point3(m_gobj_sliceColor.r, m_gobj_sliceColor.g, m_gobj_sliceColor.b);
		break;
	}
	return TRUE;
}

BOOL SectionObject::SetValue(int i, TimeValue t, int v)
{
	switch (i)
	{
	case PB_UPDATE_METHOD:
		m_update_method = v;
		break;
	case PB_SECTION_SCOPE:
		m_SectionScope = v;
		break;
	}
	return TRUE;
}

BOOL SectionObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid)
{
	switch (i)
	{
	case PB_UPDATE_METHOD:
		v = m_update_method;
		break;
	case PB_SECTION_SCOPE:
		v = m_SectionScope;
		break;
	}
	return TRUE;
}


//-------------------------------------------------------------------
#define DATA_CHUNK	0xabcd
#define SCOPE_CHUNK	0xabce
#define VERSION_CHUNK 0xabcf



// Note: If you add chunks here -  you need to verify that the
// chunk ID's does not collide with LinShape.cpp and ShapeObj.cpp!
// This is dangerous stuff!

// IO
IOResult SectionObject::Save(ISave *isave) 
{
	ULONG nb;
	int temp;

	isave->BeginChunk(VERSION_CHUNK);
	temp = SERIALIZATION_CURRENT_VERSION;
	isave->Write(&temp, sizeof(int), &nb);
	isave->EndChunk();

	isave->BeginChunk(DATA_CHUNK);
	isave->Write(&m_gobj_sliceColor.r,	sizeof(float),		&nb);
	isave->Write(&m_gobj_sliceColor.g,	sizeof(float),		&nb);
	isave->Write(&m_gobj_sliceColor.b,	sizeof(float),		&nb);
	isave->Write(&m_update_method,		sizeof(int),		&nb);
	isave->Write(&m_curTime,			sizeof(TimeValue),	&nb);
	isave->Write(&m_sliceWelded,		sizeof(BOOL),		&nb);
	isave->EndChunk();

	isave->BeginChunk(SCOPE_CHUNK);
	isave->Write(&m_SectionScope,		sizeof(int),		&nb);
	isave->EndChunk();

	return LinesShape::Save(isave);
}


IOResult  SectionObject::Load(ILoad *iload) 
{
	ULONG nb;
	IOResult res;

	// First thing to do is to check for a version chunk.
	// This has to be the first chunk.
	m_version = SERIALIZATION_VERSION_0;
	USHORT nextChunk = iload->PeekNextChunkID();
	if (nextChunk == VERSION_CHUNK)
	{
		iload->OpenChunk();
		res = iload->Read(&m_version, sizeof(int), &nb);
		iload->CloseChunk();
	}

	while (TRUE) {
		USHORT nextChunk = iload->PeekNextChunkID();
		if ((nextChunk != DATA_CHUNK) &&
			(nextChunk != SCOPE_CHUNK))
			break;
		if (IO_OK != (res=iload->OpenChunk())) {
			iload->CloseChunk();
			return res;
		}
		switch( iload->CurChunkID() )
		{
			case DATA_CHUNK:
				res = iload->Read(&m_gobj_sliceColor.r,	sizeof(float),		&nb);
				res = iload->Read(&m_gobj_sliceColor.g,	sizeof(float),		&nb);
				res = iload->Read(&m_gobj_sliceColor.b,	sizeof(float),		&nb);
				res = iload->Read(&m_update_method,		sizeof(int),		&nb);
				res = iload->Read(&m_curTime,			sizeof(TimeValue),	&nb);
				res = iload->Read(&m_sliceWelded,		sizeof(BOOL),		&nb);
				break;
			case SCOPE_CHUNK:
				res = iload->Read(&m_SectionScope,		sizeof(int),		&nb);
				break;
		}
		iload->CloseChunk();
	}

	if (IO_OK != (res=LinesShape::Load(iload)) )
		return res;

	iload->RegisterPostLoadCallback( new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,SECTION_OBJ_PARAM_BLOCK_ID) );

	m_pleaseUpdate	= FALSE;
	m_justLoaded	= TRUE;

	return IO_OK;
}

//-------------------------------------------------------------------

BaseInterface* SectionObject::GetInterface(Interface_ID id) 
{
	return LinesShape::GetInterface(id); 
}

//-------------------------------------------------------------------

void SectionObject::SetReference(int i, RefTargetHandle rtarg) 
{
	switch(i)
	{
		case SHAPE_OBJ_PARAM_BLOCK_ID:
		{
			ShapeObject::SetReference(i, rtarg);
			break;
		}
		case SECTION_OBJ_PARAM_BLOCK_ID:
		{
			m_pblock = (IParamBlock*)rtarg;
			break;
		}
		default:
			DbgAssert(FALSE);
			break;
	}
	return;
}

RefTargetHandle SectionObject::GetReference(int i)
{
	RefTargetHandle result = NULL;

	switch(i)
	{
		case SHAPE_OBJ_PARAM_BLOCK_ID:
		{
			result = ShapeObject::GetReference(i);
			break;
		}
		case SECTION_OBJ_PARAM_BLOCK_ID:
		{
			result = (RefTargetHandle)m_pblock; 
			break;
		}
		default:
		{
			DbgAssert(FALSE);
			break;
		}
	}
	return result;
}

int SectionObject::RemapRefOnLoad(int iref)
{
	int result = 0;
	switch (m_version)
	{
	case SERIALIZATION_VERSION_0:
		result = iref + 1;
		break;
	case SERIALIZATION_VERSION_1:
		result = iref;
		break;
	default:
		DbgAssert(FALSE);
		break;
	}
	return result;
}

int SectionObject::NumSubs()
{
	return 1; 
}
Animatable* SectionObject::SubAnim(int i)
{
	if (m_pblock)
	{
		if (i < SHAPE_OBJ_NUM_SUBS)
		{
			return m_pblock;
		}
	}
	return NULL;
}
TSTR SectionObject::SubAnimName(int i)
{
	if (i < SHAPE_OBJ_NUM_SUBS)
	{
		return TSTR(GetString(IDS_DB_CREATE_VALS)); 
	}
	else
	{
		return TSTR("");
	}
}
//------------------------------------------------------
//------------------------------------------------------
//------------------------------------------------------


// class variable for section class.
IParamMap*	SectionObject::m_pmapCreate			= NULL;
IParamMap*	SectionObject::m_pmapParam			= NULL;
IObjParam*	SectionObject::m_ip					= NULL;


//------------------------------------------------------

class SectionObjClassDesc:public ClassDesc {
	public:
	int 			IsPublic()		{ return 1; }
	void *			Create(BOOL loading = FALSE) { return new SectionObject; }
	const TCHAR *	ClassName()		{ return GetString(IDS_SECTION_CLASSNAME); }
	SClass_ID		SuperClassID()	{ return SHAPE_CLASS_ID; }
	Class_ID		ClassID()		{ return sectionClassID; } 
	const TCHAR* 	Category()		{ return GetString(IDS_DB_GENERAL);  }
	};

static SectionObjClassDesc sectionObjDesc;

ClassDesc* GetSectionDesc() { return &sectionObjDesc; }

//------------------------------------------------------

class LinesClassDesc:public ClassDesc {
	public:
	int 			IsPublic()		{ return 0; }
	void *			Create(BOOL loading = FALSE) { return new LinesShape; }
	const TCHAR *	ClassName()		{ return GetString(IDS_DB_LINES); }
	SClass_ID		SuperClassID()	{ return SHAPE_CLASS_ID; }
	Class_ID		ClassID()		{ return linesClassID; };
	const TCHAR* 	Category()		{ return GetString(IDS_DB_GENERAL);  }
	};

static LinesClassDesc linesDesc;

ClassDesc* GetLinesDesc() { return &linesDesc; }

//-------------------------------------------------------------------
// get string utility

TCHAR *GetString(int id)
	{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}


// DLL Functions

//-------------------------------------------------------------------
/** public functions **/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
   if( fdwReason == DLL_PROCESS_ATTACH )
   {
      hInstance = hinstDLL;
      DisableThreadLibraryCalls(hInstance);
   }
	return(TRUE);
	}


//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_LIBDESCRIPTION); }

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses()
{
#ifdef NO_OBJECT_SHAPES_SPLINES
	return 0;
#else
	return 2;
#endif
}

__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) {
#ifdef NO_OBJECT_SHAPES_SPLINES
	return 0;
#else
	switch(i) {
		case 0:  return GetSectionDesc();
		case 1:  return GetLinesDesc();
		default: return 0;
		}
#endif
	}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}
