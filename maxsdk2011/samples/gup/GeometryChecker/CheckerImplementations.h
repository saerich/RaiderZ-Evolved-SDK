
/*==============================================================================
// Copyright (c) 1998-2008 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Geometry Checker
// AUTHOR: Michael Zyracki created 2008
//***************************************************************************/

#ifndef __CHECKER_IMPLEMENTATIONS__H__
#define __CHECKER_IMPLEMENTATIONS__H__

#include <Max.h>
#include <bmmlib.h>
#include <guplib.h>
#include <maxapi.h>
#include <iparamb2.h>
#include <notify.h>
#include <iparamm2.h>
#include <istdplug.h>
#include <maxheapdirect.h>
#include <patchobj.h>
#include <triobj.h>
#include <polyobj.h>
#include <IGeometryChecker.h>
#include <maxscrpt\MAXScrpt.h>
#include <maxscrpt\Numbers.h>
#include <maxscrpt\maxobj.h>
#include <meshadj.h>
#include <map>
#include <WindowsMessageFilter.h>
#include "resource.h"

//interfaces defines
#define FLIPPED_FACES_INTERFACE Interface_ID(0x4a815af4, 0x48c56402)
#define FLIPPED_UVW_FACES_INTERFACE Interface_ID(0x6a1e3c73, 0x45b72191)
#define ISOLATED_VERTEX_INTERFACE Interface_ID(0x4634351d, 0x11654837)
#define MISSING_UV_COORDINATES_INTERFACE Interface_ID(0x12616fdb, 0x37ef729c)
#define MULTIPLE_EDGE_INTERFACE Interface_ID(0x7a9f3524, 0x49c804b4)
#define OPEN_EDGES_INTERFACE Interface_ID(0x116d7bc3, 0x268227b5)
#define OVERLAPPED_UVW_FACES_INTERFACE Interface_ID(0x39ba6e45, 0x1fcf5023)
#define OVERLAPPING_FACES_INTERFACE Interface_ID(0x54dd1f44, 0x4a2b0b56)
#define OVERLAPPING_VERTICES_INTERFACE Interface_ID(0x43d74abe, 0x3c23abc)
#define TVERTS_INTERFACE Interface_ID(0x28770efb, 0x4b862591)

//class id defines, need them for the GUP classes.

#define FLIPPED_FACES_CLASS_ID Class_ID(0x4a815af4, 0x48c56403)
#define FLIPPED_UVW_FACES_CLASS_ID Class_ID(0x6a1e3c73, 0x45b72192)
#define ISOLATED_VERTEX_CLASS_ID Class_ID(0x4634351d, 0x11654838)
#define MISSING_UV_COORDINATES_CLASS_ID Class_ID(0x12616fdb, 0x37ef729d)
#define MULTIPLE_EDGE_CLASS_ID Class_ID(0x7a9f3524, 0x49c804b5)
#define OPEN_EDGES_CLASS_ID Class_ID(0x116d7bc3, 0x268227b6)
#define OVERLAPPED_UVW_FACES_CLASS_ID Class_ID(0x39ba6e45, 0x1fcf5024)
#define OVERLAPPING_FACES_CLASS_ID Class_ID(0x54dd1f44, 0x4a2b0b57)
#define OVERLAPPING_VERTICES_CLASS_ID Class_ID(0x1c2b3d07, 0x6fab5104)
#define TVERTS_CLASS_ID Class_ID(0x28770efb, 0x4b862592)

//define for the color of the checker results.
#define GEOMETRY_MANAGER_CLASS_ID Class_ID(0x581424c8, 0x685f0fb5)
#define GEOMETRY_DISPLAY_COLOR 0x685f0fb5
#define GEOMETRY_TEXT_COLOR 0x685f0fb6

//mxs checker interface ID
#define MXS_GEOMETRY_CHECKER_INTERFACE Interface_ID(0x75e580f, 0x44e069ef)

//define for how long before we pop up the dialog
#define POP_UP_TIME		2.0f

//define for how long we run before checking for the escape key, needed do to defect
#define ESC_TIME		1.0f

// SUPPORT CLASSES


//functions for dialogs. defined in overlappingvertices.cpp
void setupSpin(HWND hDlg, int spinID, int editID, float val);
float getSpinVal(HWND hDlg, int spinID);
void PositionWindowNearCursor(HWND hwnd);

//mesh class has no explicit edge list internal, so we use these functions for id;ing edges
class CheckerMeshEdge
{
public:
	static int GetTotalNumEdges(int numTris){return numTris*3;}; //assuming each triangle is unconnected in the worse case...
	static int GetEdgeNum(int tri,int localedge) {return tri*3 +localedge;};
	static void GetTriLocalEdge(int edge, int &tri, int &localedge){ tri = edge/3;  localedge = edge - (edge/3)*3;}

};

TCHAR *GetString(int id);


//used by the Open Edges, Multiple Edge, and TVert checkers
//basically for each vertex it contains a list of edges emitting from that vertex.
//when the list is creating via the CreateEdgeFaceList function, the createCompletevariable
//determines if we create a complete set of edge lists so that an edge appears twice, for each vertex or
//if we only create the edge once, and that's then kept with the lowest vertex index.
//the open edge and mutliple edge checkers only create the edge once, since we only need to check each edge once.
class EdgeFaceList
{
public:
	EdgeFaceList();
	~EdgeFaceList();
	void DeleteData();
	void CreateEdgeFaceList(Mesh &mesh,bool createComplete =false);
	void CreateEdgeFaceList(MNMesh &mesh,bool createComplete = false);

	//between 2 or more faces..
	struct Edge
	{
		Edge():mVert1(-1),mVert2(-1){};
		~Edge(){};
		int mVert1, mVert2; 
		Tab<int> mFaces;  
		int mEdgeNum;

	};

	struct VertexEdges
	{
		~VertexEdges(){ for (int i=0;i<mEdges.Count();++i){if(mEdges[i]) delete mEdges[i];}}
		Tab<Edge*> mEdges;
	};
	int mNumVertexEdges;
	VertexEdges **mVertexEdges;

	void CreateEdges();
};


//used by the UVW checkers
class UVWChannel
{
public:
	UVWChannel(): mNumVerts(0),mVerts(NULL),mNumFaces(0),mFaces(NULL) {};
	~UVWChannel(){DeleteData();};

	void DeleteData();
	void SetWithMesh(Mesh *mesh,int channel);
	void SetWithMesh(MNMesh *mesh,int channel);
	int GetNumTVVerts();
	Point3 GetTVVert(int tvvert);
	int GetNumOfFaces();
	int GetFaceDegree(int face);
	int GetFaceTVVert(int face, int index); //return tvert index

private:
	struct Face
	{
		Face():mNumVerts(0),mTVVerts(NULL){};
		~Face(){DeleteVerts();}

		void DeleteVerts();
		void AllocateVerts(int degree);
		int mNumVerts;
		int *mTVVerts;
	};

	int mNumVerts;
	UVVert *mVerts;
	int mNumFaces;
	UVWChannel::Face *mFaces;
};

using namespace std;
//helper class to show the results of the check as a highlighted vertex, edge or face.
//used by both the geometrychecker manager system and by the flipped faces geometry checker so that it can
//draw faces with the normals flipped.
class DisplayIn3DViewport
{
public:
	struct DisplayParams
	{
		Color *mC; //override color, if NULL then no override color,
		bool mSeeThrough; //if true we basically disable the zbuffer so the result can be 'seen through'.
		bool mLighted;    //if true faces are lighted(and transparent), 
		bool mFlipFaces; //if true we flip the faces when we draw them
		bool mDrawEdges; // if true we draw the poly/tri edges when drawing in edged face mode.
		DisplayParams(): mC(NULL), mSeeThrough(false), mLighted(true), mFlipFaces(false),mDrawEdges(false){}
	};
	~DisplayIn3DViewport(){DeleteCachedResults();}
	void DisplayPolyObjectResults(DisplayParams &params, TimeValue t, INode* inode, ViewExp *vpt,PolyObject *poly,IGeometryChecker::ReturnVal type,IGeometryChecker::OutputVal &results);
	void DisplayTriObjectResults(DisplayParams &params,TimeValue t, INode* inode, ViewExp *vpt,TriObject *tri,IGeometryChecker::ReturnVal type,IGeometryChecker::OutputVal &results);

	void DisplayResults(DisplayParams &params,TimeValue t,INode *node, HWND hwnd, IGeometryChecker::ReturnVal type,Tab<int> &indices);

	void DeleteCachedResults();
private:
	void DisplayResultFace (TimeValue t, GraphicsWindow *gw, MNMesh *mesh, int ff,DWORD savedLimits,bool flipFaces,bool dontdrawSelected,Point3 *colorOverride);
	void DisplayResultEdge (TimeValue t, GraphicsWindow *gw, MNMesh *mesh, int ee,bool dontdrawSelected);
	void DisplayResultVert (TimeValue t, GraphicsWindow *gw,MNMesh *mesh, int vv,bool dontdrawSelected);


	void DisplayResultFace (TimeValue t, GraphicsWindow *gw, Mesh *mesh, int ff,DWORD savedLimits,bool flipFaces,bool dontdrawSelected,Point3 *colorOverride);
	void DisplayResultEdge (TimeValue t, GraphicsWindow *gw,INode *node, Mesh *mesh, int ee,bool dontdrawSelected,bool checkVisible);
	void DisplayResultVert (TimeValue t, GraphicsWindow *gw,Mesh *mesh, int vv,bool dontdrawSelected);

	int GetNumOfEdges(int numTris) {return numTris*3;};


	//the following below is used to create an adj edge list when we display edges for a mesh. We need to that to figure out if the
	//edges on the mesh are both on 'hidden' faces in which case we don't display the edge..
	//we clear the cache whenever DeleteCachedResults is called, and not on every display call so it's kinda optimal
	typedef std::map<INode *, AdjEdgeList *> NodeAndEdges;
	NodeAndEdges mNodeAndEdges;
	AdjEdgeList * FindAdjEdgeList(INode *item,Mesh &mesh);
};


/*
•	Geometry: Flipped faces / face normals orientation 
•	Geometry: Overlapping faces (partially or completely) 
•	Geometry: Open Edges 
•	Geometry: Multiple Edge 
•	Geometry: Missing UV Coordinates --need more info
•	Geometry: Overlapping Vertices 
•	Geometry: Isolated Vertices 
•	Geometry: TVerts
•	UVW Coordinates: Flipped UVW faces  
•	UVW Coordinates: Overlapped UVW faces  
*/

//mxs checker
class MXSGeometryChecker : public IGeometryChecker
{
private:
	Value *mCheckerFunc;
	Value *mIsSupportedFunc;
	MSTR mName;
	ReturnVal mReturnVal;
	Value *mPopupFunc;  //may be NULL
	Value *mTextOverrideFunc; //may be NULL
	Value *mDisplayOverrideFunc; //may be NULL
	int mID;
public:
	//statics
	//this is where we store the funcs so we can clean them out for garbage collection
	static Array* mxsFuncs;
	
	MXSGeometryChecker(Value *checkerFunc,Value *isSupportedFunc,  IGeometryChecker::ReturnVal returnVal,MSTR &name,Value *popupFunc,
		Value *textOverrideFunc, Value *displayFunc);
	~MXSGeometryChecker(){};

	//interface
	BaseInterface* GetInterface(Interface_ID id)
	{
		if (id == MXS_GEOMETRY_CHECKER_INTERFACE) return this;
		return IGeometryChecker::GetInterface(id);
	}

	//inherited funcs
	bool IsSupported(INode*);

	ReturnVal TypeReturned();

	//! do the check
	ReturnVal GeometryCheck(TimeValue t,INode *node, OutputVal &val);

	//! the name returned
	MSTR GetName();

	//property dialog stuff.  whether or not it has a proeprty dialog, and if so, show it.
	bool HasPropertyDlg();
	void ShowPropertyDlg();

	bool HasTextOverride();
	MSTR TextOverride();

	bool HasDisplayOverride();
	void DisplayOverride(TimeValue t, INode* inode, HWND hwnd,Tab<int> &results);
	DWORD GetCheckerID(){return mID;}

	//new funcs
	void CleanOutFuncs();
};

//base class all local non-mxs checkers inherit from since we are a GUP

class GUPChecker : public GUP, public IGeometryChecker
{
public:
	GUPChecker(){};
	
	DWORD Start(){return GUPRESULT_KEEP;}
	void Stop(){};
	enum {kFailed=0, kVertices,kEdges,kFaces,kOutputEnum,kGeometryCheck,kHasPropertyDlg,kShowPropertyDlg,kLastEnum};
	virtual int GeometryCheck_FPS(TimeValue t, INode *node, Tab<int> &results);
	
	//most checkers don't have a property dialog, so do nothing here.
	bool HasPropertyDlg(){return false;}
	void ShowPropertyDlg(){};

	//most don't override, so implmenent these here.
	bool HasTextOverride(){return false;}
	MSTR TextOverride(){ return MSTR("");}

	bool HasDisplayOverride(){return false;}
	void DisplayOverride(TimeValue t, INode* inode, HWND hwnd,Tab<int> &results){};

};




class IsolatedVertexChecker : public GUPChecker,FPStaticInterface
{
private:
	// Declares the constructor of this class. It is private so a single static instance of this class
    // may be instantiated.
    DECLARE_DESCRIPTOR( IsolatedVertexChecker );

	// The single instance of this class
    static  IsolatedVertexChecker  _theInstance;
public:
	static IsolatedVertexChecker& GetInstance() { return _theInstance; }
	~IsolatedVertexChecker(){};

	bool IsSupported(INode*);
	IGeometryChecker::ReturnVal TypeReturned();
	IGeometryChecker::ReturnVal GeometryCheck(TimeValue t,INode *nodeToCheck, OutputVal &val);
	MSTR GetName();
	DWORD GetCheckerID(){return 1;}
	BEGIN_FUNCTION_MAP	
		FN_3(kGeometryCheck,TYPE_ENUM,GeometryCheck_FPS,TYPE_TIMEVALUE,TYPE_INODE,TYPE_INDEX_TAB_BR);
		FN_0(kHasPropertyDlg,TYPE_bool,HasPropertyDlg);
		VFN_0(kShowPropertyDlg,ShowPropertyDlg);
	END_FUNCTION_MAP
};

class OverlappingFacesChecker : public GUPChecker,FPStaticInterface
{
private:
	float mTolerance; //tolerance
	static const float GetDefaultTolerance(){return 0.01f;}

	// Declares the constructor of this class. It is private so a single static instance of this class
    // may be instantiated.
    DECLARE_DESCRIPTOR( OverlappingFacesChecker );

	// The single instance of this class
    static  OverlappingFacesChecker  _theInstance;

	//function for deterniming if a point is within a triangle (return true) and the distance it is away from it (float dist param)
	//pt is the point a b c are triangle point locaions, norm is the normal, positionInTriangle is location of point on triangle,
	//bary is barycentric coord, and dist is the
	//distance, which will not get modified if the point isn't within the triangle
	//distTolSquared if not NULL is used to quickly exit the function and don't do the expensive calculation of the barycentric
	//coordinate. It contains the squared distance tolerance and if the point is further away from the triangle plane than that
	//distance then we stop the calculation
	bool Dist2PtToTri(const Point3 &pt, const Point3 &a, const Point3 &b,
				   const Point3 &c, const Point3 &norm,Point3 &positionInTriangle,Point3 &bary,float &dist, float *distTolSquared);
public:
	static OverlappingFacesChecker& GetInstance() { return _theInstance; }
	~OverlappingFacesChecker(){};

	bool IsSupported(INode*);
	IGeometryChecker::ReturnVal TypeReturned();
	IGeometryChecker::ReturnVal GeometryCheck(TimeValue t,INode *nodeToCheck,OutputVal &val);
	MSTR GetName();
	bool HasPropertyDlg(){return true;}
	void ShowPropertyDlg();
	DWORD Start(){mTolerance = GetDefaultTolerance(); return GUPChecker::Start();}
	void SetTolerance(float tolerance){mTolerance = tolerance;}
	float GetTolerance(){return mTolerance;}
	enum {kGetTolerance = GUPChecker::kLastEnum+1,kSetTolerance};
	DWORD GetCheckerID(){return 2;}

	BEGIN_FUNCTION_MAP	
		FN_3(kGeometryCheck,TYPE_ENUM,GeometryCheck_FPS,TYPE_TIMEVALUE,TYPE_INODE,TYPE_INDEX_TAB_BR);
		FN_0(kHasPropertyDlg,TYPE_bool,HasPropertyDlg);
		VFN_0(kShowPropertyDlg,ShowPropertyDlg);
		PROP_FNS(kGetTolerance, GetTolerance, kSetTolerance, SetTolerance, TYPE_FLOAT);
	END_FUNCTION_MAP
};

class MultipleEdgeChecker : public GUPChecker,FPStaticInterface
{
private:
	// Declares the constructor of this class. It is private so a single static instance of this class
    // may be instantiated.
    DECLARE_DESCRIPTOR( MultipleEdgeChecker );

	// The single instance of this class
    static  MultipleEdgeChecker  _theInstance;
public:
	static MultipleEdgeChecker& GetInstance() { return _theInstance; }
	~MultipleEdgeChecker(){};

	bool IsSupported(INode*);
	IGeometryChecker::ReturnVal TypeReturned();
	IGeometryChecker::ReturnVal GeometryCheck(TimeValue t,INode *nodeToCheck, OutputVal &val);
	MSTR GetName();
	DWORD GetCheckerID(){return 3;}

	BEGIN_FUNCTION_MAP	
		FN_3(kGeometryCheck,TYPE_ENUM,GeometryCheck_FPS,TYPE_TIMEVALUE,TYPE_INODE,TYPE_INDEX_TAB_BR);
		FN_0(kHasPropertyDlg,TYPE_bool,HasPropertyDlg);
		VFN_0(kShowPropertyDlg,ShowPropertyDlg);
	END_FUNCTION_MAP
};


class OpenEdgesChecker :public GUPChecker,FPStaticInterface
{
private:
	// Declares the constructor of this class. It is private so a single static instance of this class
    // may be instantiated.
    DECLARE_DESCRIPTOR( OpenEdgesChecker );

	// The single instance of this class
    static  OpenEdgesChecker  _theInstance;
public:
	static OpenEdgesChecker& GetInstance() { return _theInstance; }
	~OpenEdgesChecker(){};

	bool IsSupported(INode*);
	IGeometryChecker::ReturnVal TypeReturned();
	IGeometryChecker::ReturnVal GeometryCheck(TimeValue t,INode *nodeToCheck, OutputVal &val);
	MSTR GetName();
	DWORD GetCheckerID(){return 4;}

	BEGIN_FUNCTION_MAP	
		FN_3(kGeometryCheck,TYPE_ENUM,GeometryCheck_FPS,TYPE_TIMEVALUE,TYPE_INODE,TYPE_INDEX_TAB_BR);
		FN_0(kHasPropertyDlg,TYPE_bool,HasPropertyDlg);
		VFN_0(kShowPropertyDlg,ShowPropertyDlg);
	END_FUNCTION_MAP
};

class FlippedFacesChecker : public GUPChecker,FPStaticInterface
{
private:
	// Declares the constructor of this class. It is private so a single static instance of this class
    // may be instantiated.
    DECLARE_DESCRIPTOR( FlippedFacesChecker );

	// The single instance of this class
    static  FlippedFacesChecker  _theInstance;
public:
	static FlippedFacesChecker& GetInstance() { return _theInstance; }
	~FlippedFacesChecker(){};

	bool IsSupported(INode*);
	IGeometryChecker::ReturnVal TypeReturned();
	IGeometryChecker::ReturnVal GeometryCheck(TimeValue t,INode *nodeToCheck, OutputVal &val);
	bool HasDisplayOverride(){return true;}
	void DisplayOverride(TimeValue t, INode* inode, HWND hwnd,Tab<int> &results);
	bool HasTextOverride(){return true;}
	MSTR TextOverride() {return MSTR("");}
	DWORD GetCheckerID(){return 5;}


	MSTR GetName();
	BEGIN_FUNCTION_MAP	
		FN_3(kGeometryCheck,TYPE_ENUM,GeometryCheck_FPS,TYPE_TIMEVALUE,TYPE_INODE,TYPE_INDEX_TAB_BR);
		FN_0(kHasPropertyDlg,TYPE_bool,HasPropertyDlg);
		VFN_0(kShowPropertyDlg,ShowPropertyDlg);
	END_FUNCTION_MAP
};


class MissingUVCoordinatesChecker :public GUPChecker,FPStaticInterface
{
private:
	// Declares the constructor of this class. It is private so a single static instance of this class
    // may be instantiated.
    DECLARE_DESCRIPTOR( MissingUVCoordinatesChecker );

	// The single instance of this class
    static  MissingUVCoordinatesChecker  _theInstance;
public:
	static MissingUVCoordinatesChecker& GetInstance() { return _theInstance; }
	~MissingUVCoordinatesChecker(){};

	bool IsSupported(INode*);
	IGeometryChecker::ReturnVal TypeReturned();
	IGeometryChecker::ReturnVal GeometryCheck(TimeValue t,INode *nodeToCheck, OutputVal &val);
	MSTR GetName();
	DWORD GetCheckerID(){return 6;}

	BEGIN_FUNCTION_MAP	
		FN_3(kGeometryCheck,TYPE_ENUM,GeometryCheck_FPS,TYPE_TIMEVALUE,TYPE_INODE,TYPE_INDEX_TAB_BR);
		FN_0(kHasPropertyDlg,TYPE_bool,HasPropertyDlg);
		VFN_0(kShowPropertyDlg,ShowPropertyDlg);
	END_FUNCTION_MAP
};

class OverlappingVerticesChecker :public GUPChecker,FPStaticInterface
{
private:
	float mTolerance; //tolerance
	static const float GetDefaultTolerance(){return 0.01f;}

	// Declares the constructor of this class. It is private so a single static instance of this class
    // may be instantiated.
    DECLARE_DESCRIPTOR( OverlappingVerticesChecker );

	// The single instance of this class
    static  OverlappingVerticesChecker  _theInstance;

public:

    static  OverlappingVerticesChecker& GetInstance() { return _theInstance; }
	~OverlappingVerticesChecker(){};

	bool IsSupported(INode*);
	IGeometryChecker::ReturnVal TypeReturned();
	IGeometryChecker::ReturnVal GeometryCheck(TimeValue t,INode *nodeToCheck, OutputVal &val);
	MSTR GetName();
	bool HasPropertyDlg(){return true;}
	void ShowPropertyDlg();
	DWORD Start(){mTolerance = GetDefaultTolerance(); return GUPChecker::Start();}


	void SetTolerance(float tolerance){mTolerance = tolerance;}
	float GetTolerance(){return mTolerance;}
	DWORD GetCheckerID(){return 7;}


	enum {kGetTolerance = GUPChecker::kLastEnum+1,kSetTolerance};
	BEGIN_FUNCTION_MAP	
		FN_3(kGeometryCheck,TYPE_ENUM,GeometryCheck_FPS,TYPE_TIMEVALUE,TYPE_INODE,TYPE_INDEX_TAB_BR);
		FN_0(kHasPropertyDlg,TYPE_bool,HasPropertyDlg);
		VFN_0(kShowPropertyDlg,ShowPropertyDlg);
		PROP_FNS(kGetTolerance, GetTolerance, kSetTolerance, SetTolerance, TYPE_FLOAT);
	END_FUNCTION_MAP

};





class TVertsChecker :public GUPChecker,FPStaticInterface
{
private:
	// Declares the constructor of this class. It is private so a single static instance of this class
    // may be instantiated.
    DECLARE_DESCRIPTOR( TVertsChecker);

	// The single instance of this class
    static  TVertsChecker  _theInstance;
public:
	static TVertsChecker& GetInstance() { return _theInstance; }
	~TVertsChecker(){};

	bool IsSupported(INode*);
	IGeometryChecker::ReturnVal TypeReturned();
	IGeometryChecker::ReturnVal GeometryCheck(TimeValue t,INode *nodeToCheck, OutputVal &val);
	MSTR GetName();
	DWORD GetCheckerID(){return 8;}

	BEGIN_FUNCTION_MAP	
		FN_3(kGeometryCheck,TYPE_ENUM,GeometryCheck_FPS,TYPE_TIMEVALUE,TYPE_INODE,TYPE_INDEX_TAB_BR);
		FN_0(kHasPropertyDlg,TYPE_bool,HasPropertyDlg);
		VFN_0(kShowPropertyDlg,ShowPropertyDlg);
	END_FUNCTION_MAP
};


class FlippedUVWFacesChecker :public GUPChecker,FPStaticInterface
{
private:
	// Declares the constructor of this class. It is private so a single static instance of this class
    // may be instantiated.
    DECLARE_DESCRIPTOR( FlippedUVWFacesChecker );

	// The single instance of this class
    static  FlippedUVWFacesChecker  _theInstance;
public:
	static FlippedUVWFacesChecker& GetInstance() { return _theInstance; }


	~FlippedUVWFacesChecker(){};

	bool IsSupported(INode*);
	IGeometryChecker::ReturnVal TypeReturned();
	IGeometryChecker::ReturnVal GeometryCheck(TimeValue t,INode *nodeToCheck, OutputVal &val);
	MSTR GetName();
	DWORD GetCheckerID(){return 9;}

	BEGIN_FUNCTION_MAP	
		FN_3(kGeometryCheck,TYPE_ENUM,GeometryCheck_FPS,TYPE_TIMEVALUE,TYPE_INODE,TYPE_INDEX_TAB_BR);
		FN_0(kHasPropertyDlg,TYPE_bool,HasPropertyDlg);
		VFN_0(kShowPropertyDlg,ShowPropertyDlg);
	END_FUNCTION_MAP
};

class OverlappedUVWFacesChecker : public GUPChecker,FPStaticInterface
{
private:
	// Declares the constructor of this class. It is private so a single static instance of this class
    // may be instantiated.
    DECLARE_DESCRIPTOR( OverlappedUVWFacesChecker );

	// The single instance of this class
    static  OverlappedUVWFacesChecker  _theInstance;
	IGeometryChecker::ReturnVal GeometryCheckWithUVMesh(UVWChannel &uvmesh,TimeValue t,INode *nodeToCheck, BitArray &arrayOfFaces);
public:
	static OverlappedUVWFacesChecker& GetInstance() { return _theInstance; }
	~OverlappedUVWFacesChecker(){};

	bool IsSupported(INode*);
	IGeometryChecker::ReturnVal TypeReturned();
	IGeometryChecker::ReturnVal GeometryCheck(TimeValue t,INode *nodeToCheck, OutputVal &val);
	MSTR GetName();
	DWORD GetCheckerID(){return 10;}

	BEGIN_FUNCTION_MAP	
		FN_3(kGeometryCheck,TYPE_ENUM,GeometryCheck_FPS,TYPE_TIMEVALUE,TYPE_INODE,TYPE_INDEX_TAB_BR);
		FN_0(kHasPropertyDlg,TYPE_bool,HasPropertyDlg);
		VFN_0(kShowPropertyDlg,ShowPropertyDlg);
	END_FUNCTION_MAP

};

//function to handle the escape button when escaping out of a checker. It's checks the processID
//to make sure it's the max process ID.
class EscapeChecker {
public:
	EscapeChecker (){};
	void Setup ();
	bool Check ();

	static EscapeChecker theEscapeChecker;
private:
	MaxSDK::WindowsMessageFilter mMessageFilter;
};

//global 

//What this function does is based upon the start time and ups value (calculate via QueryPerformance counter) 
//it determines if the checker should show that dialog.  Once the dialog is shown it sets the checkTime flag to false so this function no longer needs to get called so the dialog doesn’t keep getting poped open.
//It also will set the compute flag to false if the checker should stop computing.  
class DialogChecker
{
public:

	inline static void Check(bool &checkTime, bool &compute, int numFaces, LARGE_INTEGER &startTime,LARGE_INTEGER &ups)
	{

		LARGE_INTEGER endTime;
		QueryPerformanceCounter(&endTime);
		float sec = (float)((double)(endTime.QuadPart-startTime.QuadPart)/(double)ups.QuadPart);
		if(sec>POP_UP_TIME)
		{
			checkTime = false;
			if(ShowWarningDialogThenStop(numFaces))
				compute = false;	
		}
	}
private:
	//function used by the checkers to see if they need to through up dialog to check
	//if they should keep running.
	//Will return true if we should stop the check.
	static bool ShowWarningDialogThenStop(int numfaces);
};

//needs to be a #define unfortunately for omp atomic to work, no function call, even inlined is allowed
#define SetBitItem(bitArray, loc) 	bitArray|= (1<<loc)

static inline void ClearBitItem(DWORD bitArray, DWORD loc) {
	bitArray &= (~(1<<loc));
}
static inline DWORD GetBitItem(DWORD bitArray, DWORD loc) {
	return (bitArray&(1<<loc));
}




#endif