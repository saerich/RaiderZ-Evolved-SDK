//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: This contains our information about pelt mapping
// AUTHOR: Peter Watje
// DATE: 2006/10/07 
//***************************************************************************/

#ifndef __PELTDATA__H
#define __PELTDATA__H

#include "TVData.h"

#define CID_PELTEDGEMODE CID_USER + 204
#define ID_TOOL_PELTSTRAIGHTEN	0x0490

class PeltEdgeMode;
class UnwrapMod;
class MeshTopoData;

class FConnnections
{
public:
	Tab<int> connectedEdge;
};

class EdgeBondage
{
public:
	EdgeBondage()
	{
//		extraStiffness = 1.0f;
		cornerIndex = -1;
	}
	float dist;
	float str;
	float distPer;
	int v1,v2;
	int vec1,vec2;
	int isEdge;
	Point3 edgePos; // if this is an edge spring this is locked point connected to v1
	Point3 originalVertPos; // this is the original pos of V1 so we can quickly recalc our distances
	int flags;
	int edgeIndex;
	int cornerIndex;
//	int distanceFromEdge;
//	float extraStiffness;
};


class SpringClass
{
public:
	Point3 vel, pos;
	Point3 tempVel[6];
	Point3 tempPos[6];
	float mass;
	float weight;
};



class Solver
{

public:
	
	BOOL Solve(int startFrame, int endFrame, int samples,
			   Tab<EdgeBondage> &springs, Tab<SpringClass> &vertexData, 
			   float stiffness, float dampening, float decay,
			   UnwrapMod *mod = NULL, MeshTopoData *ld = NULL);

private:
	float maxVelocity;
	void SolveFrame(int level, 
				   Tab<EdgeBondage> &springs, Tab<SpringClass> &vertexData,
				   float stiffness, float dampening);

	void Evaluate(Tab<EdgeBondage> &springs, Tab<SpringClass> &vertexData,
					  TimeValue i, 
					  int samples,
					  float &stiffness, float &dampening);

	Tab<int> vertsToProcess;  //just a list of indices of vertices that we need to process since we get passed all the verts to help with optimizations
	Tab<SpringClass> holdHoldVertexData;
};



//a seam edge data is basicaly an edge of a geom face

class RigPoint
{
public:
	int lookupIndex; //this is the index into  the TVMaps.v structure that holds the rig point
	int index;  //the tv point this rig point is attached to 
	int neighbor;  //the neighbor to this point
	float elen; //the len toits neighbor
	float d; //the initial distance from the rig point to the UV point	
	Point3 p; //the rig point position
	int springIndex; // the index to the spring that is attached to this rig point
	float angle;
};

class PeltData;

class PeltDialog
{
public:
	UnwrapMod *mod;
	HWND hWnd;
	PeltDialog()
	{
		hWnd = NULL;
		mod = NULL;
	}
	void SetUpDialog(HWND hWnd);
	void DestroyDialog();
	void SpinnerChange(PeltData &peltData, int controlID);
	void SetStraightenMode(BOOL on);
	BOOL GetStraightenMode();
	void UpdateSpinner(int id, int value);
	void UpdateSpinner(int id, float value);

	void SetStraightenButton(BOOL on);

private:
	

	ISpinnerControl *iSpinMirrorAngle;

	ISpinnerControl *iSpinRigStrength;
	ISpinnerControl *iSpinSamples;
	ISpinnerControl *iSpinFrames;

	ISpinnerControl *iSpinStiffness;
	ISpinnerControl *iSpinDampening;
	ISpinnerControl *iSpinDecay;
};

class PeltData
{
public:

	PeltData() 
	{
		startPoint=-1;
		rigStrength = 0.1f;
		samples = 5;
		frames = 20;

		stiffness = 0.16f;
		dampening = 0.16f;
		decay = 0.25f;
		inPeltMode = FALSE;
		inPeltPickEdgeMode = FALSE;
		inPeltPointToPointEdgeMode = FALSE;
		rigMirrorAngle = 0.0f;
		mSuspendSubObjectChange = FALSE;
		mBaseMeshTopoDataCurrent = NULL;
		mBaseMeshTopoDataPrevious = NULL;
	};
	
	
//	void PeltData(BitArray fsel, Mesh *mnMesh);
//	void PeltData(BitArray fsel, Mesh *patch);
	~PeltData();
	void Free();

	static PeltEdgeMode *peltEdgeMode;

	int HitTestPointToPointMode(UnwrapMod *mod, MeshTopoData *ld,ViewExp *vpt,GraphicsWindow *gw,IPoint2 *p,HitRegion hr,INode* inode,ModContext* mc);
	int HitTestEdgeMode(UnwrapMod *mod,MeshTopoData *ld , ViewExp *vpt,GraphicsWindow *gw,/*IPoint2 *p,*/HitRegion hr,INode* inode, ModContext *mc,int flags, int type);

	void ResolvePatchHandles(UnwrapMod *mod);
	
	

	void StartPeltDialog(UnwrapMod *mod);
	void EndPeltDialog(UnwrapMod *mod);

	void NukeRig();

	void SetupSprings(UnwrapMod *mod);

	void ValidateSeams(/*w4 UnwrapMod *mod, */ BitArray &newSeams);

	void SetRigStrength(float str);
	float GetRigStrength();

	void SetSamples(int samples);
	int GetSamples();

	void SetFrames(int frames);
	int GetFrames();

	void SetStiffness(float stiff);
	float GetStiffness();

	void SetDampening(float damp);
	float GetDampening();

	void SetDecay(float decay);
	float GetDecay();

	void SetMirrorAngle(float angle);
	float GetMirrorAngle();

	Point3 GetMirrorCenter();

	void SnapRig(UnwrapMod *mod);
	void Run(UnwrapMod *mod);
	void RunRelax(UnwrapMod *mod, int relaxLevel);

	void SelectRig(UnwrapMod *mod, BOOL replace = FALSE);
	void SelectPelt(UnwrapMod *mod, BOOL replace = FALSE);

	void ResetRig(UnwrapMod *mod);

	void MirrorRig(UnwrapMod *mod);
	void StraightenSeam(UnwrapMod *mod, int a, int b);

	void ExpandSelectionToSeams(UnwrapMod *mod);

	BOOL GetPeltMapMode();
	void SetPeltMapMode(UnwrapMod *mod,BOOL mode);

	BOOL GetEditSeamsMode();
	void SetEditSeamsMode(UnwrapMod *mod,BOOL mode);

	BOOL GetPointToPointSeamsMode();
	void SetPointToPointSeamsMode(UnwrapMod *mod,BOOL mode);

	void RelaxRig(int iterations, float str, BOOL lockBoundaries,UnwrapMod *mod);

	//our spring data
	Tab<EdgeBondage> springEdges;
	//our rig data
	Tab<RigPoint> rigPoints;
	int startPoint;
	Tab<SpringClass> verts;  //this is a list off all our verts, there positions and velocities

	
	PeltDialog peltDialog;
	
	
//	BitArray seamEdges;
	BitArray peltFaces;
//	Tab<GeomEdge> seamGeoEdges;
	BitArray tempPoints;

	
	int currentPointHit;
	int previousPointHit;
	Point3 viewZVec;

	void ShowUI(HWND hWnd, BOOL show);
	void EnablePeltButtons(HWND hWnd, BOOL enable);

	void SetParamRollupHandle(HWND hWnd) { hParams = hWnd;};
	void SetRollupHandle(HWND hWnd) { hMapParams = hWnd;};
	void SetSelRollupHandle(HWND hWnd) { hSelParams = hWnd;};
	void SetPeltDialogHandle(HWND hWnd) { peltDialoghWnd = hWnd;};


	Point3 pointToPointAnchorPoint;
	MeshTopoData *mBaseMeshTopoDataCurrent;
	MeshTopoData *mBaseMeshTopoDataPrevious;
	IPoint2 basep;
	IPoint2 cp,pp;

	IPoint2 lastMouseClick;
	IPoint2 currentMouseClick;

	BOOL IsSubObjectUpdateSuspended() { return mSuspendSubObjectChange;}
	void SubObjectUpdateSuspend() { mSuspendSubObjectChange = TRUE;};
	void SubObjectUpdateResume() { mSuspendSubObjectChange = FALSE;};

	BitArray mIsSpring;
	Tab<float> mPeltSpringLength;

private:

	void PeltMode(UnwrapMod *mod,BOOL start);

	BOOL inPeltMode;
	BOOL inPeltPickEdgeMode;
	BOOL inPeltPointToPointEdgeMode;

	HWND hMapParams;
	HWND hSelParams;
	HWND hParams;
	HWND peltDialoghWnd;

	float rigStrength;
	int samples;
	int frames;
	float stiffness;
	float dampening;
	float decay;
	Tab<Point3> initialPointData;

	void CutSeams(UnwrapMod *mod, BitArray seams);
	BitArray hiddenVerts;
	Point3 rigCenter;
	float rigMirrorAngle;
	BOOL mSuspendSubObjectChange;

//	Tab<FaceData*> faces;	//this is a list of faces for this pelt
//	Tab<SeamEdgeData> seams; //this is a list of seams
	


	

	//our velocity data
};



class UnwrapPeltSeamRestore : public RestoreObj {
public:
	BitArray useams, rseams;
	UnwrapMod *mod;
	MeshTopoData *ld;
	int up1,up2;
	int rp1,rp2;
	IPoint2 ulastP, ucurrentP;
	IPoint2 rlastP, rcurrentP;

	//watje 685880 might need to restore the anchor point
	BOOL mRestoreAnchorPoint;
	Point3 mUAnchorPoint;
	Point3 mRAnchorPoint;

	UnwrapPeltSeamRestore(UnwrapMod *m, MeshTopoData *d, Point3 *anchorPoint=NULL);
	
	void Restore(int isUndo);
	void Redo();
	int Size() { return 1; }
	void EndHold() {}
	TSTR Description() { return TSTR(_T(GetString(IDS_PW_PIVOTRESTORE))); 
	}
};


class UnwrapPeltVertVelocityRestore : public RestoreObj {
public:
	UnwrapMod *mod;
	Tab<SpringClass> uverts;
	Tab<SpringClass> rverts;

	UnwrapPeltVertVelocityRestore(UnwrapMod *m);
	
	void Restore(int isUndo);
	void Redo();
	int Size() { return 1; }
	void EndHold() {}
	TSTR Description() { return TSTR(_T(GetString(IDS_PW_PIVOTRESTORE))); 
	}
};



#endif // __PELTDATA__H
