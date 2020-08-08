/*****************************************************************************
 *<
	FILE: grid.cpp

	DESCRIPTION:  Snaps to grids

	CREATED BY: John Hutchinson		

	HISTORY: created 12/12/96

 *>	Copyright (c) 1994, All Rights Reserved.
 *****************************************************************************/
// The main MAX include file.
#include "max.h"

//master include file for osnaps
#include "osnapapi.h"

#include "data.h"
#include "resource.h"

#define SUBCOUNT 2
#define NOSUB -1
#define INT_SUB 0
#define EDGE_SUB 1
#define SMALLVALUE	(float)1.0e-10		//for use in fixing bug #162291
#define PIXEL_THRESH 2	//used for 748560. On-screen pixel distance threshold between 2 points.

// This is the DLL instance handle passed in when the plug-in is 
// loaded at startup.
HINSTANCE hInstance;

TCHAR *GetString(int id);


class gridSnap : public Osnap {

private:
	OsnapMarker markerdata[SUBCOUNT];
	TSTR name[SUBCOUNT];
	HBITMAP tools;
	HBITMAP masks;

	// Steps through the grid points located above (positive y) and below (negative y)
	// of gridHitPt and records the hits if in the snap preview radius
	void FindVerticalIntSubHit(
		const Matrix3& tmCP,				
		Point3& gridHitPt,	
		const Point2& fp,
		float gridSize);
	// Records a new snap hit with the snap manager. 
	void RecordNewIntSubHit(
		const Point3& worldHitPt, 
		const Matrix3& tmCP, 
		const Point3& gridHitPt, 
		float gridSize);

public:

	gridSnap();//constructor
	virtual ~gridSnap();

	virtual int numsubs(){return SUBCOUNT;} //the number of subsnaps this guy has
	virtual TSTR *snapname(int index); // the snap’s name to be displayed in the UI
	virtual boolean ValidInput(SClass_ID scid, Class_ID cid); 
	Class_ID ClassID() { return GRID_OSNAP_CLASS_ID; }

	virtual OsnapMarker *GetMarker(int index){return &(markerdata[index]);} 
	virtual HBITMAP getTools(){return tools;} 
	virtual HBITMAP getMasks(){return masks;} 
	virtual WORD AccelKey(int index); //virtual key codes
	virtual void Snap(Object* pobj, IPoint2 *p, TimeValue t);
	virtual WORD HiliteMode(){return HILITE_CROSSHAIR;}
};



TSTR *gridSnap::snapname(int index)
{
	return &name[index];
}


WORD gridSnap::AccelKey(int index)
{
	switch (index)
	{
	case INT_SUB:
		return 0x4F;
		break;
	case EDGE_SUB:
		return 0x42;
		break;
	default:
		return 0;
	}
}


gridSnap::gridSnap()
{
	tools = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_ICONS));
	masks = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_MASK));
	name[INT_SUB]= TSTR(GetString(IDS_GRID_INT));
	name[EDGE_SUB]= TSTR(GetString(IDS_GRID_EDGES));
#ifdef GAME_VER
	markerdata[0]=OsnapMarker(0,NULL,NULL);
	markerdata[1]=OsnapMarker(0,NULL,NULL);
#else
	markerdata[0]=OsnapMarker(5,mark0verts,mark0es);
	markerdata[1]=OsnapMarker(5,mark0verts,mark0es);
#endif //GAME_VER
}

gridSnap::~gridSnap()
{
	DeleteObject(tools);
	DeleteObject(masks);
}

boolean gridSnap::ValidInput(SClass_ID scid, Class_ID cid){
/*	boolean c_ok = FALSE, sc_ok = FALSE;
	sc_ok |= (scid == HELPER_CLASS_ID)? TRUE : FALSE;
	c_ok |= (cid == Class_ID(GRIDHELP_CLASS_ID,0))? TRUE : FALSE; 
	return sc_ok && c_ok;*/
	return theman->GetNode()->IsRootNode();
}

float GridCoord(float v, float g)
{
//	assert(g != 0.0f);

	if (g == 0.0f)
			g = SMALLVALUE;
	float vwork = (float)floor((fabs(v)+0.5f*g)/g)*g;	
	return (v < 0.0f) ? -vwork : vwork;
}			

int Screendist(IOsnapManager *theman, Point3 &pt1, Point3 &pt2)
{
	//computes the distance between two world points in screen space
	IPoint3 screen1_3, screen2_3;
	IPoint2 screen1, screen2;

	theman->wTranspoint(&pt1, &screen1_3);
	theman->wTranspoint(&pt2, &screen2_3);
	screen1.x = screen1_3.x;
	screen1.y = screen1_3.y;
	screen2.x = screen2_3.x;
	screen2.y = screen2_3.y;

	return Length(screen1-screen2);
}

BOOL TooDistant(IOsnapManager *theman, Point3 &pt, float gridsize)
{
	//ktong|Jan.27.2006|#748560|Limit the number of snap points to search only visibly distinct points.
	//Determines if the grid point is visibly resolvable on the screen from neighbouring points.
	//Tests along both axes.
	Point3 xadd, yadd;
	xadd = pt + Point3(gridsize, 0.0f, 0.0f);
	yadd = pt + Point3(0.0f, gridsize, 0.0f);
	int xdist, ydist;
	xdist = Screendist(theman, pt, xadd );
	ydist = Screendist(theman, pt, yadd);

	//If less than PIXEL_THRESH pixels away from its neighbouring grid points, the pt grid point is too 
	//far away to be distinguishable on the screen from its neighbours.
	return ((xdist < PIXEL_THRESH) || (ydist < PIXEL_THRESH));
}

static BOOL clipgrid(Point3 wp, ViewExp *vpt)
{
	if(!vpt->IsPerspView())
		return TRUE;
	float minx, miny, maxx, maxy;
	vpt->GetGridDims(&minx, &maxx, &miny, &maxy);
	if(wp.x > minx && wp.x < maxx && wp.y > miny && wp.y < maxy)
		return TRUE;
	else 
		return FALSE;
}



void gridSnap::Snap(Object* pobj, IPoint2 *p, TimeValue t)
{	
	ViewExp *vpt = theman->GetVpt();
	DbgAssert(vpt != NULL);

	//local copy of the cursor position
	Point2 fp = Point2((float)p->x, (float)p->y);

	BOOL got_one= FALSE;
	//Get point on the consttruction plane 
	Point3 local_cp = vpt->GetPointOnCP(*p);
	Point3 world_snapped;

	Matrix3 tmconst;
	vpt->GetConstructionTM( tmconst );

	// aszabo|nov.01.05|#596889, #613720, #703875 
	// In order to allow snapping to the grid sub-divisions computed based on viewport 
	// the zoom facto, the grid spacing is taken from the viewport.  
	// When the grid is invisible and the zoom factor changes, the viewport returns 
	// the grid spacing computed last time the grid was displayed.
	// When max starts up with hidden grids (via maxstart.max), the viewport's grid 
	// spacing will be zero and in that case we take the grid spacing value specified 
	// in the Grid setting UI.
	float gridSpacing = vpt->GetGridSize();
	if (!(gridSpacing > 0.0f || gridSpacing < 0.0f)) {
		gridSpacing = GetCOREInterface()->GetGridSpacing();
	}


	//Compute all the hit point candidates
	if(	GetActive(INT_SUB))
	{
		float gridZ = 0.f;
#ifdef GAME_VER
		float gridSpacing = GetCOREInterface()->GetGridSpacing();

		SnapInfo si;
		GetCOREInterface()->InitSnapInfo(&si);
		if(si.snapType == SNAP_25D)
			gridZ = GridCoord(local_cp.z,gridSpacing);

		Point3 intsnapped = Point3(GridCoord(local_cp.x,gridSpacing),GridCoord(local_cp.y,gridSpacing),gridZ);
		world_snapped = tmconst * intsnapped;//now in world space
		got_one = TRUE;

		hitmesh = new HitMesh(5);
		hitmesh->setVert(0,tmconst * Point3(intsnapped.x,intsnapped.y - gridSpacing,gridZ));
		hitmesh->setVert(1,tmconst * Point3(intsnapped.x,intsnapped.y + gridSpacing,gridZ));
		hitmesh->setVert(2,tmconst * Point3(intsnapped.x-gridSpacing,intsnapped.y,gridZ));
		hitmesh->setVert(3,tmconst * Point3(intsnapped.x + gridSpacing,intsnapped.y,gridZ));

		//now register a hit with the osnap manager
		//possibly screen for proximity to the foreground
		theman->RecordHit(new OsnapHit(world_snapped, this, INT_SUB, hitmesh));

#else //!GAME_VER
		// aszabo|sep.15.04|#596889|The gridSpacing spacing shouldn't be zero, but if it is, it can lead to infinite loops
		DbgAssert(gridSpacing > 0.0f || gridSpacing < 0.0f);
		if (gridSpacing > 0.0f || gridSpacing < 0.0f)
		{
			// Record all hits that fall in the snap preview area.
			// Grids are infinite, so we can't loop  through their "vertexes".
			// Instead go through only those that are potential snap points, by starting
			// at the closest grid point to the mouse and extending in all directions
			// using "grid size" steps.
			Point3 intsnapped = Point3(GridCoord(local_cp.x,gridSpacing),GridCoord(local_cp.y,gridSpacing),gridZ);
			Point3 curIntsnapped(intsnapped);
			world_snapped = tmconst * intsnapped;

			// set the window transform to identity to guarantee a world-screen transformation.
			if (theman->GetVpt() && theman->GetVpt()->getGW()) {
				theman->GetVpt()->getGW()->setTransform(Matrix3(TRUE));
			}
			// Find all snap points on the right side (positive x) of the intsnapped, including the intsnapped
			while (CheckPotentialHit(&world_snapped,0, fp)) 
			{
				got_one = TRUE;
				RecordNewIntSubHit(world_snapped, tmconst, curIntsnapped, gridSpacing);

				//ktong|Jan.27.2006|#748560|Limit the number of snap points to search only visibly distinct points.
				// If the grid point is too far away to tell it from the next gridpoint, stop searching.
				// No point in enumerating snap points a user can't visibly tell apart.
				// Check after the first snap point (the closest) is registered to at least have one.
				if (TooDistant(theman, world_snapped, gridSpacing)) {
					break;
				}

				// Go down the vertical grid line
				curIntsnapped.y -= gridSpacing;
				FindVerticalIntSubHit(tmconst, curIntsnapped, fp, (-gridSpacing));

				// Go up the vertical grid line
				curIntsnapped.y = intsnapped.y + gridSpacing;
				FindVerticalIntSubHit(tmconst, curIntsnapped, fp, gridSpacing);

				// Go to the right of the original vertival grid line
				curIntsnapped.y = intsnapped.y;
				curIntsnapped.x += gridSpacing;
				world_snapped = tmconst * curIntsnapped;
			} 
			// Find all snap points on the left side (negative x) of the intsnapped
			curIntsnapped.y = intsnapped.y;
			curIntsnapped.x = intsnapped.x - gridSpacing;
			world_snapped = tmconst * curIntsnapped;
			//ktong|Jan.27.2006|#748560|Limit the number of snap points to search only visibly distinct points.
			while (CheckPotentialHit(&world_snapped,0, fp) && !TooDistant(theman, world_snapped, gridSpacing) ) 
			{
				got_one = TRUE;
				RecordNewIntSubHit(world_snapped, tmconst, curIntsnapped, gridSpacing);

				// Go down the vertical grid line
				curIntsnapped.y -= gridSpacing;
				FindVerticalIntSubHit(tmconst, curIntsnapped, fp, (-gridSpacing));

				// Go up the vertical grid line
				curIntsnapped.y = intsnapped.y + gridSpacing;
				FindVerticalIntSubHit(tmconst, curIntsnapped, fp, gridSpacing);

				// Go to the right of the original vertival grid line
				curIntsnapped.y = intsnapped.y;
				curIntsnapped.x -= gridSpacing;
				world_snapped = tmconst * curIntsnapped;
			} 
		}
#endif //GAME_VER
	}


	if(	GetActive(EDGE_SUB) && !got_one)
	{
		BOOL snapx = FALSE;
		float xSnap = GridCoord(local_cp.x,gridSpacing);
		float ySnap = GridCoord(local_cp.y,gridSpacing);
		float xDist = (float)fabs(xSnap - local_cp.x);
		float yDist = (float)fabs(ySnap - local_cp.y);
		Point3 esnapped;

		// Which one is closer?
		if(xDist < yDist)
		{
			snapx = TRUE;
			esnapped = Point3(xSnap,local_cp.y,0.0f);
		}
		else
		{
			esnapped = Point3(local_cp.x,ySnap,0.0f);
		}
		world_snapped = tmconst * esnapped;//now in world space

		if (CheckPotentialHit(&world_snapped,0, fp))
		{
			if(clipgrid(world_snapped, vpt))
			{
				got_one = TRUE;
				HitMesh* hitmesh = new HitMesh(3);
				if(snapx)
				{
					hitmesh->setVert(0,tmconst * Point3(esnapped.x,esnapped.y - gridSpacing,0.0f));
					hitmesh->setVert(1,tmconst * Point3(esnapped.x,esnapped.y + gridSpacing,0.0f));
				}
				else
				{
					hitmesh->setVert(0,tmconst * Point3(esnapped.x - gridSpacing, esnapped.y,0.0f));
					hitmesh->setVert(1,tmconst * Point3(esnapped.x + gridSpacing, esnapped.y,0.0f));
				}
				theman->RecordHit(new OsnapHit(world_snapped, this, EDGE_SUB, hitmesh));
			}
		}
	}
}

//-----------------------------------------------------------------------------
void gridSnap::FindVerticalIntSubHit(
	const Matrix3& tmCP, 
	Point3& gridHitPt, 
	const Point2& fp,
	float gridSize)
{
	Point3 world_snapped = tmCP * gridHitPt;
	//ktong|Jan.25.2006|#748560|Limit the number of snap points to search only visibly distinct points.
	while (CheckPotentialHit(&world_snapped,0, fp) && !TooDistant(theman, world_snapped, gridSize)) 
	{
		RecordNewIntSubHit(world_snapped, tmCP, gridHitPt, fabsf(gridSize));
		gridHitPt.y += gridSize;
		world_snapped = tmCP * gridHitPt;
	}
}

//-----------------------------------------------------------------------------
void gridSnap::RecordNewIntSubHit(
	const Point3& worldHitPt, 
	const Matrix3& tmCP, 
	const Point3& gridHitPt, 
	float gridSize)
{
	HitMesh* hitmesh = new HitMesh(5);

	hitmesh->setVert(0,tmCP * Point3(gridHitPt.x, gridHitPt.y - gridSize, gridHitPt.z));
	hitmesh->setVert(1,tmCP * Point3(gridHitPt.x, gridHitPt.y + gridSize, gridHitPt.z));
	hitmesh->setVert(2,tmCP * Point3(gridHitPt.x - gridSize, gridHitPt.y, gridHitPt.z));
	hitmesh->setVert(3,tmCP * Point3(gridHitPt.x + gridSize, gridHitPt.y, gridHitPt.z));
	theman->RecordHit(new OsnapHit(worldHitPt, this, INT_SUB, hitmesh));
}

//static gridSnap thegridSnap;

TCHAR *GetString(int id)
{
	static TCHAR buf[256];
	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}


/*===========================================================================*\
 | The Class Descriptor
\*===========================================================================*/

class OsnapClassDesc:public ClassDesc {
	public:
	// The IsPublic() method should return TRUE if the plug-in can be picked
	// and assigned by the user. Some plug-ins may be used privately by other
	// plug-ins implemented in the same DLL and should not appear in lists for
	// user to choose from, so these plug-ins would return FALSE.
	int 			IsPublic() { return 0; }
	// This is the method that actually creates a new instance of
	// a plug-in class.  By having the system call this method,
	// the plug-in may use any memory manager it wishes to 
	// allocate its objects.  The system calls the correspoding 
	// DeleteThis() method of the plug-in to free the memory.  Our 
	// implementations use 'new' and 'delete'.
	void *			Create(BOOL loading = FALSE) {return new gridSnap();}
//	void *			Create(OsnapManager *pman) { return new gridSnap(pman); }
	// This is used for debugging purposes to give the class a 
	// displayable name.  It is also the name that appears on the button
	// in the MAX user interface.
	const TCHAR *	ClassName() { return _T("gridSnap"); }
	// The system calls this method at startup to determine the type of object
	// this is.  In our case, we're a geometric object so we return 
	// GEOMOBJECT_CLASS_ID.  The possible options are defined in PLUGAPI.H
	SClass_ID		SuperClassID() { return OSNAP_CLASS_ID; }
	// The system calls this method to retrieve the unique
	// class id for this object.
	Class_ID		ClassID() { 
		return GRID_OSNAP_CLASS_ID; }
	// The category is selected
	// in the bottom most drop down list in the create branch.
	// If this is set to be an exiting category (i.e. "Primatives", ...) then
	// the plug-in will appear in that category. If the category doesn't
	// yet exists then it is created.  We use the new How To category for
	// all the example plug-ins in the How To sections.
	const TCHAR* 	Category() { return _T(""); }
	};

// Declare a static instance of the class descriptor.
static OsnapClassDesc sampDesc;
// This function returns the address of the descriptor.  We call it from 
// the LibClassDesc() function, which is called by the system when loading
// the DLLs at startup.
ClassDesc* GetSampDesc() { return &sampDesc; }

/*===========================================================================*\
 | The DLL Functions
\*===========================================================================*/
// This function is called by Windows when the DLL is loaded.  This 
// function may also be called many times during time critical operations
// like rendering.  Therefore developers need to be careful what they
// do inside this function.  In the code below, note how after the DLL is
// loaded the first time only a few statements are executed.
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
	{	
      if( fdwReason == DLL_PROCESS_ATTACH )
      {
         hInstance = hinstDLL;				// Hang on to this DLL's instance handle.
         DisableThreadLibraryCalls(hInstance);
      }

	return(TRUE);
	}

// This function returns the number of plug-in classes this DLL implements
__declspec( dllexport ) int LibNumberClasses() {return 1;}

// This function return the ith class descriptor
__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) {
	switch(i) {
		case 0: return GetSampDesc();		
		default: return 0;
		}
	}

// This function returns a string that describes the DLL and where the user
// could purchase the DLL if they don't have it.
__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_LIB_DESCRIPTION); }

// This function returns a pre-defined constant indicating the version of 
// the system under which it was compiled.  It is used to allow the system
// to catch obsolete DLLs.
__declspec( dllexport ) ULONG
LibVersion() {  return VERSION_3DSMAX; }

