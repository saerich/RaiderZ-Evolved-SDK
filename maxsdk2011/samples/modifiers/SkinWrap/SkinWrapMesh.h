/**********************************************************************
 *<
	FILE: MeshDeformPW.h

	DESCRIPTION:	Includes for Plugins

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __MESHDEFORMPW__H
#define __MESHDEFORMPW__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "modstack.h"
#include "macrorec.h"

#include "meshadj.h"
#include "XTCObject.h"

#include "..\BonesDef\RayMeshIntersect\IRayMeshGridIntersect.h"
#include "iskin.h"
#include "iFnPub.h"
#include "ISkinWrapMesh.h"

#include "MAXScrpt\MAXScrpt.h"
#include "MAXScrpt\Listener.h"
#include "MAXScrpt\MAXObj.h"
#include "imacroscript.h"
#include "XRef\iXrefItem.h"

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

#define ScriptPrint (the_listener->edit_stream->printf)

enum {	meshdeformpw_params,		// parameter rollup
		meshdeformpw_advanceparams, // advance rollup contains mirroring and baking tools
		meshdeformpw_displayparams, // display rollup
	};


//TODO: Add enums for various parameters
enum { 
	pb_mesh,		// the node that represent the control cage
	pb_autoupdate,  // never used
	pb_threshold,	// used to as the distance limit trheshold when finding closest face
	pb_engine,		// what type of deformation engine is used 0 = face,  1 = vertex
	pb_falloff,		// this sets the falloff of a control point it is a simple pow funtion, only used with the vertex engine
	pb_distance,	// this sets the how far our a control point influence , only used with the vertex engine
	pb_facelimit,	// this is a limit to keep vertices from being influenced by seperate elements.  This restricts how far in faces a vertex can be influenced
	pb_showloops,	// displays the loops for control point selection
	pb_showaxis,	// displays the axis trip for control point selection
	pb_showfacelimit,  // this showes the potential influence area by putting red ticks onvertices that a control can influence
	pb_mirrorshowdata, // toggles on the mirror display data
	pb_mirrorplane,		//this is the mirror plane axis 0 = x, 1 = y, 2 = z
	pb_mirroroffset,	//this is the mirror offset along the mirror plane
	pb_debugpoint,		// Debug info only
	pb_meshlist,
	pb_showunassignedverts,
	pb_showverts,
	pb_weightallverts,
	pb_mirrorthreshold,
	pb_useblend,
	pb_blenddistance
};




enum {  meshdeform_getselectedvertices, meshdeform_setselectedvertices,
		meshdeform_getnumbercontrolpoints,
		meshdeform_getcontrolpointscale, meshdeform_setcontrolpointscale,
		meshdeform_getcontrolpointstr, meshdeform_setcontrolpointstr,
		meshdeform_mirrorselectedcontrolpoints,
		meshdeform_bakecontrolpoints,meshdeform_retrievecontrolpoints,
		meshdeform_resample,

		meshdeform_setlocalscale,meshdeform_setlocalstr,

		meshdeform_getcontrolpointinitialtm,
		meshdeform_getcontrolpointcurrenttm,
		meshdeform_getcontrolpointdist,
		meshdeform_getcontrolpointxvert,

		meshdeform_numbervertices,
		meshdeform_vertexnumberweights,
		meshdeform_vertexweight,
		meshdeform_vertexdistance,
		meshdeform_vertexcontrolpoint,

		meshdeform_reset,
		meshdeform_converttoskin,
		meshdeform_ouputvertexdata



};

class IMeshDeformPWModFP :   public IMeshDeformPWMod
	{
	public:

		//Function Publishing System
		//Function Map For Mixin Interface
		//*************************************************
		BEGIN_FUNCTION_MAP
			VFN_3(meshdeform_setselectedvertices, fnSetSelectVertices, TYPE_INT, TYPE_BITARRAY, TYPE_BOOL);
			FN_1(meshdeform_getselectedvertices, TYPE_BITARRAY, fnGetSelectedVertices,TYPE_INDEX);
			
			FN_1(meshdeform_getnumbercontrolpoints, TYPE_INT, fnGetNumberControlPoints,TYPE_INDEX);

			FN_2(meshdeform_getcontrolpointscale, TYPE_POINT3, fnGetPointScale, TYPE_INDEX,TYPE_INDEX);
			VFN_3(meshdeform_setcontrolpointscale, fnSetPointScale, TYPE_INDEX, TYPE_INDEX, TYPE_POINT3);

			FN_2(meshdeform_getcontrolpointstr, TYPE_FLOAT, fnGetPointStr, TYPE_INDEX,TYPE_INDEX);
			VFN_3(meshdeform_setcontrolpointstr, fnSetPointStr, TYPE_INDEX,TYPE_INDEX, TYPE_FLOAT);

			VFN_0(meshdeform_mirrorselectedcontrolpoints, fnMirrorSelectedVerts);
			VFN_0(meshdeform_bakecontrolpoints, fnBakeControlPoints);
			VFN_0(meshdeform_retrievecontrolpoints, fnRetreiveControlPoints);
			VFN_0(meshdeform_resample, fnResample);
			
			VFN_2(meshdeform_setlocalscale, fnSetLocalScale,TYPE_INDEX,TYPE_FLOAT);
			VFN_1(meshdeform_setlocalstr, fnSetLocalStr,TYPE_FLOAT);

			FN_2(meshdeform_getcontrolpointinitialtm, TYPE_MATRIX3, fnGetPointInitialTM, TYPE_INDEX,TYPE_INDEX);
			FN_2(meshdeform_getcontrolpointcurrenttm, TYPE_MATRIX3, fnGetPointCurrentTM, TYPE_INDEX,TYPE_INDEX);
			FN_2(meshdeform_getcontrolpointdist, TYPE_FLOAT, fnGetPointDist, TYPE_INDEX,TYPE_INDEX);
			FN_2(meshdeform_getcontrolpointxvert, TYPE_INT, fnGetPointXVert, TYPE_INDEX,TYPE_INDEX);

			FN_1(meshdeform_numbervertices, TYPE_INT, fnNumberOfVertices, TYPE_INODE);
			FN_2(meshdeform_vertexnumberweights, TYPE_INT, fnVertNumberWeights, TYPE_INODE, TYPE_INDEX);
			FN_3(meshdeform_vertexweight, TYPE_FLOAT, fnVertGetWeight, TYPE_INODE, TYPE_INDEX, TYPE_INDEX);
			FN_3(meshdeform_vertexdistance, TYPE_FLOAT, fnVertGetDistance, TYPE_INODE, TYPE_INDEX, TYPE_INDEX);
			FN_3(meshdeform_vertexcontrolpoint, TYPE_INT, fnVertGetControlPoint, TYPE_INODE, TYPE_INDEX, TYPE_INDEX);

			VFN_0(meshdeform_reset, fnReset);
			VFN_1(meshdeform_converttoskin, fnConvertToSkin,TYPE_BOOL);
			
			VFN_2(meshdeform_ouputvertexdata, fnOutputVertexData, TYPE_INODE,TYPE_INT);


		END_FUNCTION_MAP


		virtual void fnSetSelectVertices(int whichWrapMesh,BitArray *selList, BOOL updateViews)=0;
		virtual BitArray *fnGetSelectedVertices(int whichWrapMesh)=0;
		virtual int fnGetNumberControlPoints(int whichWrapMesh)=0;

		virtual Point3* fnGetPointScale(int whichWrapMesh,int index)=0;
		virtual void fnSetPointScale(int whichWrapMesh,int index, Point3 scale)=0;

		virtual float fnGetPointStr(int whichWrapMesh,int index)=0;
		virtual void fnSetPointStr(int whichWrapMesh,int index, float str)=0;

		virtual void fnMirrorSelectedVerts()=0;

		virtual void fnBakeControlPoints()=0;
		virtual void fnRetreiveControlPoints()=0;

		virtual void fnResample()=0;

		virtual void fnSetLocalScale(int axis,float scale)=0;
		virtual void fnSetLocalStr(float str)=0;

		virtual Matrix3* fnGetPointInitialTM(int whichWrapMesh,int index)=0;
		virtual Matrix3* fnGetPointCurrentTM(int whichWrapMesh,int index)=0;
		virtual float fnGetPointDist(int whichWrapMesh,int index) = 0;
		virtual int fnGetPointXVert(int whichWrapMesh,int index) = 0;

		virtual int fnNumberOfVertices(INode *node) = 0;
		virtual int fnVertNumberWeights(INode *node, int vindex) = 0;
		virtual float fnVertGetWeight(INode *node, int vindex, int windex) = 0;
		virtual float fnVertGetDistance(INode *node, int vindex, int windex) = 0;
		virtual int fnVertGetControlPoint(INode *node, int vindex, int windex) = 0;

		virtual int fnVertGetWrapNode(INode *node, int vindex, int windex) = 0;

		virtual void fnReset()=0;
		virtual void fnConvertToSkin(BOOL silent)=0;

		virtual void fnOutputVertexData(INode *whichNode, int whichVert)=0;

		

	};

#define PBLOCK_REF	0

//pre beta data for loading old beta files
class BoneInfoOld
{
public:
	float normalizedWeight;		/// the normalized weight
	float dist;					/// the distance from the control point
	int owningBone;				/// which control points owns this weight
};

/// This is our vertex weight info each deformed vertex has an array of these which 
/// are used to control which control points deforms the vertex
class BoneInfo
{
public:
	float normalizedWeight;		/// the normalized weight
	float dist;					/// the distance from the control point
	int owningBone;				/// which control points owns this weight
	int whichNode;				///which node owns the control point
	BoneInfo()
	{
		whichNode = -666; //test value to catch bugs for now
	}
};

//pre beta data for loading old beta files
class ParamSpaceDataOld
	{
	public:
		Point3	bary;		/// Bary coordinates of the closest point of the closest face
		float offset;		/// the offset distance from the face
		int faceIndex;		/// the closest face

		ParamSpaceDataOld()
		{
			bary = Point3 (0.0f,0.0f,0.0f);
			offset = 0.0f;
			faceIndex = 0;
		}

		~ParamSpaceDataOld()
		{
		}

	};
/// Every deform vertex also has a param space.  This ids the closest face on the control cage
/// to this vertex and the offset and bary coords of the closest point on that face
class ParamSpaceData
	{
	public:
		Point3	bary;		/// Bary coordinates of the closest point of the closest face
		float offset;		/// the offset/perpendicular distance from the face
		int faceIndex;		/// the closest face
		int whichNode;				///which node owns the control point
		float dist;			/// this is the actual distance from the face

		ParamSpaceData()
		{
			bary = Point3 (0.0f,0.0f,0.0f);
			offset = -1.0f;
			dist = -1.0f;
			faceIndex = 0;
			whichNode = -666; //test value to catch bugs
		}

		~ParamSpaceData()
		{
		}

	};

/// The weight data for each vertex that contains a list of
/// weights for this vertx
class WeightData
	{
	public:
		Tab<BoneInfo> data;   // array of our weights


		WeightData()
		{
		}

		~WeightData()
		{
		
		}

	};

/// This is an optimization class.   Basically every vertices keeps a list of potential
/// control points that it can influence to reduce looksups
class NeighboringVerts
{
	public:
		Tab<int> neighorbors;
};



#define FIELD_RESAMPLE					1
#define FIELD_REBUILDNEIGHBORS			2
#define FIELD_REBUILDSELECTEDWEIGHTS	3
#define FIELD_RESET						4

class ClosestFaces
{
public:
	Tab<int> f;
};

/// Our local data
class LocalMeshData : public LocalModData 
	{
	public:

//		Tab<NeighboringVerts*> lookupList;		/// neighbors list for selected vertices
		Tab<ParamSpaceData> paramData;			/// param data per vertex
		Tab<WeightData*> weightData;			/// weight data per vertex

		Matrix3 initialBaseTM;					/// the initial node matrix of the node that owns the modifier when the modifier is assigned

		Tab<Point3> initialPoint;				/// this is the initial point list cache when the modifier is applied
												/// this is basically the initial pose of the mesh that is stored
		int numFaces;							/// number of faces of the mesh used to check for topology changes


		Tab<Point3> localPoint;					/// list of deformed points in local space for display purpose
												/// this list goes away when the mod is not being edited
			
		INode *selfNode;						/// the inode associated with this local data instance

		BOOL resample;							/// resample and rebuild all the vertices weights
		BOOL rebuildNeighorbordata;				/// rebuild our neighbor list
		BOOL rebuildSelectedWeights;			/// rebuild only selected weights
		BOOL reset;								/// resets both param space and weights



		BitArray displayedVerts;
		BitArray potentialWeightVerts;			/// this is a list of verts that can be weighted while editing
												/// it is updated on selection of control points

		Tab<AdjEdgeList*> adjEdgeList;			/// this a list of adjacent edges one for each wrap object
												/// this list is only valid when the UI is up
												


		Matrix3 currentBaseTM;					/// the current node matrix of the node that owns the modifier when the modifier is assigned
		Matrix3 currentInvBaseTM;					/// the current node matrix of the node that owns the modifier when the modifier is assigned

		Tab<ClosestFaces*> faceIndex;		//list of closest faces for each point one per wrap object
		///Frees our weight data list
		void FreeWeightData()
		{
			for (int i = 0; i < weightData.Count(); i++)
				delete weightData[i];
		}

		void FreeAdjEdgeList()
		{
			for (int i = 0; i < adjEdgeList.Count(); i++)
			{
				if (adjEdgeList[i])
					delete adjEdgeList[i];
			}
		}

		LocalMeshData()
			{
			selfNode = NULL;
			resample = FALSE;
			rebuildNeighorbordata = FALSE;
			rebuildSelectedWeights = FALSE;
			numFaces = 0;
			reset = FALSE;
			
			}
		~LocalMeshData()
			{
			FreeWeightData();			
			FreeAdjEdgeList();
			for (int i = 0; i < faceIndex.Count(); i++)
			{
				if (faceIndex[i]) delete faceIndex[i];
			}
			}
		LocalModData*	Clone()
			{
			LocalMeshData* d = new LocalMeshData();
			d->selfNode = NULL;
			d->numFaces = numFaces;
			d->initialPoint = initialPoint;
			d->initialBaseTM = initialBaseTM;
			d->paramData = paramData;
			d->weightData.SetCount(weightData.Count());
			for (int i = 0; i < weightData.Count(); i++)
			{
				d->weightData[i] = new WeightData();
				d->weightData[i]->data = weightData[i]->data;

			}

			return d;
			}	
	};

//Legacy code it was never used in FCS but some old beta/alpha files might use it
class InitialTMDataOld
{
public:
		Matrix3 initialVertexTMs;
		int xPoints;
		float dist;
};

///This is per control point data
class InitialTMData
{
public:
		Matrix3 initialVertexTMs;		//this is the inititl TM of the control point
		Point3 localScale;				//this is the local scale of the control point
		int xPoints;					//this is the edge that defines the x axis of the TM, 
										//it is the index of the vertex that the x axis points 2
										//the z axis is the normal
		float dist;						//this is the size of the control points envelope
		float str;						//this is the strenght of this control point
};

class WrapMeshData
{
public:
		BitArray selectedVerts;


		Matrix3 initialMeshTM;
		Matrix3 currentMeshTM;
		Tab<InitialTMData> initialData;

		Tab<Matrix3> currentVertexTMs;
		Tab<Matrix3> defTMs;
		Tab<Point3> fnorms;
		Tab<Point3> vnorms;
		Tab<int> faceIndex;				//Legacy olny here to load old files
		Mesh *mesh;
		BOOL deleteMesh;
		TriObject *collapsedtobj;

		int numFaces;
		WrapMeshData()
		{
			mesh = NULL;
			deleteMesh = FALSE;
			collapsedtobj = NULL;
			numFaces = 0;
		}

};

class MeshDeformPW : public Modifier, public IMeshDeformPWModFP 
{
	public:

		static ICustButton	*iPickButton ;
		// Parameter block
		IParamBlock2	*pblock;	//ref 0

		static SelectModBoxCMode *selectMode;
		


		static IObjParam *ip;			//Access to the interface
		
		// From Animatable
		TCHAR *GetObjectName() { return GetString(IDS_MESHDEFORM); }

		//From Modifier
		ChannelMask ChannelsUsed()  { return PART_GEOM|PART_TOPO|PART_SELECT|PART_SUBSEL_TYPE; }
		//TODO: Add the channels that the modifier actually modifies
		ChannelMask ChannelsChanged() { return GEOM_CHANNEL; }
		//TODO: Return the ClassID of the object that the modifier can modify
		Class_ID InputType() {return defObjectClassID;}

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);


		Interval LocalValidity(TimeValue t);

		// From BaseObject
		//TODO: Return true if the modifier changes topology
		BOOL ChangeTopology() {return FALSE;}		
		
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}

		BOOL HasUVW();
		void SetGenUVW(BOOL sw);


		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);


		void ActivateSubobjSel(int level, XFormModes& modes);
		void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert);
		void ClearSelection(int selLevel);
		void SelectAll(int selLevel);
		void InvertSelection(int selLevel);

		Interval GetValidity(TimeValue t);

		Matrix3 CompMatrix(TimeValue t,INode *inode,ModContext *mc);
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc);
		void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);		
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);

		int NumSubObjTypes();
		ISubObjType *GetSubObjType(int i);

		// Automatic texture support
		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return MESHDEFORMPW_CLASS_ID;}		
		SClass_ID SuperClassID() { return OSM_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_MESHDEFORM);}

		RefTargetHandle Clone( RemapDir &remap );
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID,  RefMessage message);


		int NumSubs() { return 1; }
		TSTR SubAnimName(int i) { return GetString(IDS_PARAMS); }				
		Animatable* SubAnim(int i) { return pblock; }

		// TODO: Maintain the number or references here
		int NumRefs() { return 1; }
		RefTargetHandle GetReference(int i) { return pblock; }
		void SetReference(int i, RefTargetHandle rtarg) { pblock=(IParamBlock2*)rtarg; }




		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock

		void DeleteThis() { delete this; }		
		//Constructor/Destructor

		MeshDeformPW();
		~MeshDeformPW();		

		IOResult SaveLocalData(ISave *isave, LocalModData *pld);
		IOResult LoadLocalData(ILoad *iload, LocalModData **pld);

//published functions		 
		//Function Publishing method (Mixin Interface)
		//******************************
		BaseInterface* GetInterface(Interface_ID id) 
			{ 
			if (id == MESHDEFORMPW_INTERFACE) 
				return (IMeshDeformPWMod*)this; 
			else 
				return FPMixinInterface::GetInterface(id);
			} 


		INode* GetNodeFromModContext(LocalMeshData *smd);
		
		void SetResampleModContext();
		void SetRebuildNeighborDataModContext();
		void SetRebuildSelectedWeightDataModContext();

		///rebuilds our param data 
		void BuildParamData(TimeValue t,Object *obj,LocalMeshData *localData,int wrapIndex, INode *node,Matrix3 meshTM,Matrix3 baseTM, float threshold);
		void BuildParamDataForce(TimeValue t,Object *obj,LocalMeshData *localData,int wrapIndex, INode *node,Matrix3 meshTM,Matrix3 baseTM, float threshold, BitArray pointsToProcess);

		void FreeWeightData(Object *obj,LocalMeshData *localData);
		///rebuilds all the wieghts
		void BuildWeightData(Object *obj,LocalMeshData *localData,int wrapIndex, Mesh *msh,Matrix3 meshTM,Matrix3 baseTM, float falloff, float distance, int faceLimit, BitArray pointsToProcess);
		void NormalizeWeightData(LocalMeshData *localData);

		//rebuild the weights of vertices that are assigned to the selected control points
		void RebuildSelectedWeights(LocalMeshData *localData,int wrapMeshIndex, Matrix3 meshTM,Matrix3 baseTM, float falloff, float distance, int faceLimit);

		HWND hWnd;


		Tab<WrapMeshData*> wrapMeshData;


		void GetTMFromVertex(TimeValue t, int wrapIndex, Mesh *msh, Point3 *vnorms, Matrix3 meshToWorld);		
		void BuildCurrentTMFromVertex(TimeValue t,int wrapIndex, Mesh *msh, Point3 *vnorms,Matrix3 meshTM,Matrix3 baseTM, Matrix3 initialBaseTM);


		void DrawLoops(GraphicsWindow *gw, float size, Matrix3 tm);
		void DrawBox(GraphicsWindow *gw, Box3 box);

		void DisplayMirrorData(GraphicsWindow *gw, TimeValue t);


		void UpdateXYZSpinners(BOOL updateMult = TRUE);
		void UpdateScale(int axis, float s);

		void UpdateStr(float str);

		void MultScale(float scale);

		void fnSetLocalScale(int axis,float scale);
		void fnSetLocalStr(float str);

		///this updates the UI and needs to be called when the engine type or
		//subobject level changes
		void UpdateUI();

		void	SelectVertices(int whichWrapMesh,BitArray *selList, BOOL updateViews);
		BitArray *GetSelectedVertices(int whichWrapMesh);

		void fnSetSelectVertices(int whichWrapMesh,BitArray *selList, BOOL updateViews);
		BitArray *fnGetSelectedVertices(int whichWrapMesh);

		int GetNumberControlPoints(int whichWrapMesh);
		int fnGetNumberControlPoints(int whichWrapMesh);


		Point3* GetPointScale(int whichWrapMesh, int index);
		Point3* fnGetPointScale(int whichWrapMesh, int index);

		void SetPointScale(int whichWrapMesh,int index, Point3 scale);
		void fnSetPointScale(int whichWrapMesh,int index, Point3 scale);

		float GetPointStr(int whichWrapMesh,int index);
		void SetPointStr(int whichWrapMesh,int index, float str);

		float fnGetPointStr(int whichWrapMesh,int index);
		void fnSetPointStr(int whichWrapMesh,int index, float str);

		void MirrorSelectedVerts();
		void fnMirrorSelectedVerts();

		void fnBakeControlPoints();
		void fnRetreiveControlPoints();
		void BakeControlPoints();
		void RetreiveControlPoints();

		void fnResample();
		void Resample();

		Matrix3* fnGetPointInitialTM(int whichWrapMesh,int index);
		Matrix3* fnGetPointCurrentTM(int whichWrapMesh,int index);
		float fnGetPointDist(int whichWrapMesh,int index);
		int fnGetPointXVert(int whichWrapMesh,int index);
		Matrix3 GetPointInitialTM(int whichWrapMesh,int index);
		Matrix3 GetPointCurrentTM(int whichWrapMesh,int index);
		float GetPointDist(int whichWrapMesh,int index);
		int GetPointXVert(int whichWrapMesh,int index);

		int NumberOfVertices(INode *node);
		int VertNumberWeights(INode *node, int vindex);
		float VertGetWeight(INode *node, int vindex, int windex);
		float VertGetDistance(INode *node, int vindex, int windex);
		int VertGetControlPoint(INode *node, int vindex, int windex);
		int fnNumberOfVertices(INode *node);
		int fnVertNumberWeights(INode *node, int vindex);
		float fnVertGetWeight(INode *node, int vindex, int windex);
		float fnVertGetDistance(INode *node, int vindex, int windex);
		int fnVertGetControlPoint(INode *node, int vindex, int windex);

		void fnReset();
		void Reset();

		int fnVertGetWrapNode(INode *node, int vindex, int windex);
		int VertGetWrapNode(INode *node, int vindex, int windex);



		void fnOutputVertexData(INode *whichNode, int whichVert);


		//ui spinner pointer so we can quickly enable/disable them
		ISpinnerControl *xSpin,*ySpin,*zSpin;
		ISpinnerControl *strSpin;
		ISpinnerControl *multSpin;

		HWND hWndParamPanel;		///Handle to the parameters rollup
			
		BOOL cageTopoChange;		///Flag that is set when the control cage topo changes

		BOOL SkinWraptoSkin(BOOL silent);

		void ConvertToSkin(BOOL silent);
		void fnConvertToSkin(BOOL silent);

	private:
		/// this builds our neighbor data is called in the modifyobject method when
		/// the local data rebuildNeighorbordata is set
		void BuildLookUpData(LocalMeshData *ld, int wrapIndex);
		void BuildNormals(Mesh *mesh, Tab<Point3> &vnorms, Tab<Point3> &fnorms);
		LocalMeshData *GetLocalData(INode *node);
		BOOL resample;
		int debugF;
		int debugV[3];

		BOOL useOldLocalBaseTM;
		Matrix3 oldLocalBaseTM;

		void WeightAllVerts(LocalMeshData *localData);
		BOOL loadOldFaceIndices;
		
		Box3 worldBounds;
		BOOL oldFaceData;
		


};


class MeshDeformParamsMapDlgProc : public ParamMap2UserDlgProc {
	public:
		MeshDeformPW *mod;		
		MeshDeformParamsMapDlgProc(MeshDeformPW *m) {mod = m;}		
		INT_PTR DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {
				delete this;
				}
	};

class MeshDeformParamsAdvanceMapDlgProc : public ParamMap2UserDlgProc {
	public:
		MeshDeformPW *mod;		
		MeshDeformParamsAdvanceMapDlgProc(MeshDeformPW *m) {mod = m;}		
		INT_PTR DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {
				delete this;
				}
	};



class MeshDeformPWClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE);
	const TCHAR *	ClassName() { return GetString(IDS_MESHDEFORM); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return MESHDEFORMPW_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("MeshDeformPW"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};






class MeshEnumProc : public DependentEnumProc 
	{
      public :
      virtual int proc(ReferenceMaker *rmaker); 
      INodeTab Nodes;              
	};

class MeshDeformerWrapData
{
public:
		Matrix3 meshTm;
		Matrix3 fromMeshToBaseTm;
		Mesh *mesh;
		Point3* norms;
		
};

class MeshDeformer : public Deformer {
public:		
	MeshDeformPW * mod;
	LocalMeshData *localData;
	Matrix3 baseTm,fromWorldToBaseTm;
	Tab<MeshDeformerWrapData> wrapData;

	/*		Matrix3 fromMeshToBaseTm;
	Mesh *mesh;
	Point3* norms;
	*/
	int engine;
	float falloff;

	float blendDistance;
	BOOL useBlend;

	//		MeshDeformer(MeshDeformPW * mod, LocalMeshData *ld, Mesh *mesh, Matrix3 baseTm, Matrix3 meshTm, int engine)
	MeshDeformer(MeshDeformPW * mod, LocalMeshData *ld, Matrix3 baseTm,  int engine)
	{
		this->mod = mod;
		localData = ld;
		this->baseTm = baseTm;
		//			this->meshTm = meshTm;
		//			this->mesh = mesh;
		fromWorldToBaseTm = Inverse(baseTm);
		//			fromMeshToBaseTm = meshTm * Inverse(baseTm);
		this->engine = engine;
		mod->pblock->GetValue(pb_blenddistance,0,this->blendDistance ,FOREVER);
		mod->pblock->GetValue(pb_useblend,0,this->useBlend ,FOREVER);
	}

	void SetWrapDataCount(int count)
	{
		wrapData.SetCount(count);
	}
	void AddWrapData(int index, Mesh *mesh,  Matrix3 meshTm, Point3* norms)
	{
		wrapData[index].mesh = mesh;
		wrapData[index].meshTm = meshTm;
		wrapData[index].fromMeshToBaseTm = meshTm * Inverse(baseTm);
		wrapData[index].norms = norms;

	}

	Point3 Map(int i, Point3 pt)
	{
		if(!localData->paramData.Count() || !localData->weightData.Count())
			return pt;

		if (engine == 0)
		{
			

			ParamSpaceData *paramSpaceData = localData->paramData.Addr(i);
			int wrapIndex = paramSpaceData->whichNode;
			if (wrapIndex >= wrapData.Count()) return pt;
			if (wrapIndex < 0) return pt;
			if (wrapData[wrapIndex].mesh == NULL) return pt;
			Face *faces = wrapData[wrapIndex].mesh->faces;
			Point3 *verts = wrapData[wrapIndex].mesh->verts;

			float dist = paramSpaceData->offset;
			Point3 bry = paramSpaceData->bary;
			int fid = paramSpaceData->faceIndex;

			if (fid < 0) return pt;

			int a,b,c;
			a = faces[fid].v[0];
			b = faces[fid].v[1];
			c = faces[fid].v[2];
			
			Point3 pa = verts[a];
			Point3 pb = verts[b];
			Point3 pc = verts[c];

			if (mod->ip)
			{
				if (mod->wrapMeshData[wrapIndex]->selectedVerts[a] ||
					mod->wrapMeshData[wrapIndex]->selectedVerts[b] ||
					mod->wrapMeshData[wrapIndex]->selectedVerts[c])
				{
					if (i <  localData->displayedVerts.GetSize())
						localData->displayedVerts.Set(i,TRUE);	
				}
			}

			if (falloff == 0.001f)
			{


				pt.x = pa.x * bry.x + pb.x * bry.y + pc.x * bry.z; 
				pt.y = pa.y * bry.x + pb.y * bry.y + pc.y * bry.z; 
				pt.z = pa.z * bry.x + pb.z * bry.y + pc.z * bry.z; 


				pt += wrapData[wrapIndex].norms[fid] * (dist);
				pt = pt * wrapData[wrapIndex].fromMeshToBaseTm;
 
				return pt;//+vec;
			}
			else
			{

				int ct = localData->weightData[i]->data.Count();

				if (ct == 0) return pt;

				Point3 vec(0.0f,0.0f,0.0f);

				float per = 1.0f;
				if (useBlend)
				{
					if (paramSpaceData->dist > blendDistance) return pt;
					if (dist < 0.0f)
					{
						per = 1.0f-(paramSpaceData->dist/blendDistance);
						per = cos(per * PI);
						per += 1.0f;
						per *= 0.5f;
						per = 1.0f - per;;
					}
				}

				Point3 initialPT  = localData->initialPoint[i];

				for (int j = 0; j < ct; j++)
				{
					int id = localData->weightData[i]->data[j].owningBone;//v[j];
					float weight = localData->weightData[i]->data[j].normalizedWeight;//w[j];
					Matrix3 tm = mod->wrapMeshData[wrapIndex]->defTMs[id];
					vec += ((initialPT * tm) - initialPT) * weight;
				}

				initialPT += vec;
				vec = initialPT - pt;
				
				vec = vec * per;

				return (pt+vec);

			}
		}
		else
		{


			ParamSpaceData *paramSpaceData = localData->paramData.Addr(i);

			int wrapIndex = paramSpaceData->whichNode;
			if (wrapIndex >= wrapData.Count()) return pt;
			if (wrapIndex < 0) return pt;
			if (paramSpaceData->faceIndex < 0) return pt;

 			int ct = localData->weightData[i]->data.Count();

			if (ct == 0) return pt;

			Point3 vec(0.0f,0.0f,0.0f);
			
			float per = 1.0f;
 			if (useBlend)
			{
				if (paramSpaceData->dist > blendDistance) return pt;
				if (paramSpaceData->offset < 0.0f)
				{
					per = 1.0f-(paramSpaceData->dist/blendDistance);
					per = cos(per * PI);
					per += 1.0f;
					per *= 0.5f;
					per = 1.0f - per;
				}
			}

			Point3 initialPT  = localData->initialPoint[i];

			for (int j = 0; j < ct; j++)
			{
				int id = localData->weightData[i]->data[j].owningBone;
				float weight = localData->weightData[i]->data[j].normalizedWeight;
				int wrapIndex = localData->weightData[i]->data[j].whichNode;
				Matrix3 tm = mod->wrapMeshData[wrapIndex]->defTMs[id];
				vec += ((initialPT * tm) -initialPT) * weight;
			}

			initialPT += vec;
			vec = initialPT - pt;

			vec = vec * per;

			return (pt+vec);
		}
	}
};

class SelectRestore : public RestoreObj 
{
	public:
		MeshDeformPW *mod;
		int wrapIndex;
		BitArray uSel,rSel;
		
		SelectRestore(MeshDeformPW *m, int wrapIndex) 
			{
			mod = m;				
			this->wrapIndex = wrapIndex;
			uSel = mod->wrapMeshData[wrapIndex]->selectedVerts;
			
			}   
		void Restore(int isUndo) 
			{

			if (isUndo)
				rSel = mod->wrapMeshData[wrapIndex]->selectedVerts;

			mod->wrapMeshData[wrapIndex]->selectedVerts = uSel;

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

			}
		void Redo()
			{

			mod->wrapMeshData[wrapIndex]->selectedVerts = rSel;

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

			}		
		void EndHold() 
			{ 
			mod->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_SELECT))); }


};

class LocalScaleRestore : public RestoreObj 
{
	public:
		MeshDeformPW *mod;
		int wrapIndex;
		Tab<Point3> uScale,rScale;
		
		LocalScaleRestore(MeshDeformPW *m, int wrapIndex) 
			{
			mod = m;			
			this->wrapIndex = wrapIndex;
			uScale.SetCount(mod->wrapMeshData[wrapIndex]->initialData.Count());
			for (int i = 0; i < uScale.Count(); i++)
			{
				uScale[i] = mod->wrapMeshData[wrapIndex]->initialData[i].localScale;
			}
			
			}   
		void Restore(int isUndo) 
			{

			if (isUndo)
			{
				rScale.SetCount(mod->wrapMeshData[wrapIndex]->initialData.Count());
				for (int i = 0; i < uScale.Count(); i++)
				{
					rScale[i] = mod->wrapMeshData[wrapIndex]->initialData[i].localScale;
				}
			}

			for (int i = 0; i < uScale.Count(); i++)
			{
				mod->wrapMeshData[wrapIndex]->initialData[i].localScale = uScale[i];
			}

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

			}
		void Redo()
			{

			for (int i = 0; i < rScale.Count(); i++)
			{
				mod->wrapMeshData[wrapIndex]->initialData[i].localScale = rScale[i];
			}

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

			}		
		void EndHold() 
			{ 
			mod->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_SCALE))); }


};


class LocalStrRestore : public RestoreObj 
{
	public:
		MeshDeformPW *mod;
		int wrapIndex;
		Tab<float> uStr,rStr;
		
		LocalStrRestore(MeshDeformPW *m, int wrapIndex) 
			{
			mod = m;			
			this->wrapIndex = wrapIndex;
			uStr.SetCount(mod->wrapMeshData[wrapIndex]->initialData.Count());
			for (int i = 0; i < uStr.Count(); i++)
			{
				uStr[i] = mod->wrapMeshData[wrapIndex]->initialData[i].str;
			}
			
			}   
		void Restore(int isUndo) 
			{

			if (isUndo)
			{
				rStr.SetCount(mod->wrapMeshData[wrapIndex]->initialData.Count());
				for (int i = 0; i < uStr.Count(); i++)
				{
					rStr[i] = mod->wrapMeshData[wrapIndex]->initialData[i].str;
				}
			}

			for (int i = 0; i < uStr.Count(); i++)
			{
				mod->wrapMeshData[wrapIndex]->initialData[i].str = uStr[i];
			}

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

			}
		void Redo()
			{

			for (int i = 0; i < rStr.Count(); i++)
			{
				mod->wrapMeshData[wrapIndex]->initialData[i].str = rStr[i];
			}

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

			}		
		void EndHold() 
			{ 
			mod->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_STR))); }


};


class EnableRestore : public RestoreObj 
{
	public:
		MeshDeformPW *mod;
		
		EnableRestore(MeshDeformPW *m) 
			{
				mod = m;						
			}   
		void Restore(int isUndo) 
			{
				mod->EnableMod();

			}
		void Redo()
			{

				mod->DisableMod();

			}		
		void EndHold() 
			{ 
			mod->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_STR))); }


};


class EnvHitData : public HitData
{
public:
	EnvHitData(int v,int n)
	{
		whichVertex = v;
		whichNode = n;
	}
	int whichVertex;
	int whichNode;
};


class PickControlNode : 
		public PickModeCallback,
		public PickNodeCallback {
	public:				
		MeshDeformPW *mod;
		PickControlNode() {
					mod=NULL;
					};
		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);		
		BOOL Pick(IObjParam *ip,ViewExp *vpt);		
		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);		
		BOOL Filter(INode *node);
		BOOL AllowMultiSelect() {return TRUE;}
		PickNodeCallback *GetFilter() {return this;}
		BOOL RightClick(IObjParam *ip,ViewExp *vpt) {return TRUE;}
	};


#endif // __MESHDEFORMPW__H
