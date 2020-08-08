/**********************************************************************
 *<
	FILE: PatchDeformPW.h

	DESCRIPTION:	Includes for Plugins

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __PATCHDEFORMPW__H
#define __PATCHDEFORMPW__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "modstack.h"
#include "macrorec.h"

#include "meshadj.h"
#include "XTCObject.h"
#include "ISkinWrapPatch.h"


extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;


#define PBLOCK_REF	0

enum 
{ 
	patchdeform_resample, 
	patchdeform_getnumberofpoints, 
	patchdeform_getpointuvw,
	patchdeform_getpointlocalspace,
	patchdeform_getpointpatchspace,
	patchdeform_getpointpatchindex,

	
};

class IPatchDeformPWModFP :   public IPatchDeformPWMod
	{
	public:

		//Function Publishing System
		//Function Map For Mixin Interface
		//*************************************************
		BEGIN_FUNCTION_MAP
			VFN_0(patchdeform_resample, fnResample);

			FN_1(patchdeform_getnumberofpoints, TYPE_INT, fnGetNumberOfPoints, TYPE_INODE);
			FN_2(patchdeform_getpointuvw, TYPE_POINT3, fnGetPointUVW, TYPE_INODE, TYPE_INT);
			FN_2(patchdeform_getpointlocalspace, TYPE_POINT3, fnGetPointLocalSpace, TYPE_INODE, TYPE_INT);
			FN_2(patchdeform_getpointpatchspace, TYPE_POINT3, fnGetPointPatchSpace, TYPE_INODE, TYPE_INT);
			FN_2(patchdeform_getpointpatchindex, TYPE_INT, fnGetPointPatchIndex, TYPE_INODE, TYPE_INT);
		END_FUNCTION_MAP

		
		/// Resample()
		/// This forces the modifier to resample itself. This will force the system to resample the patch
		virtual void fnResample()=0;

		/// int GetNumberOfPoints(INode *node)
		/// This returns the number of points that are deformed
		virtual int fnGetNumberOfPoints(INode *node)=0;

		/// int Point3 GetPointUVW(INode *node, int index)
		/// This returns the closest UVW point on the patch to this point
		///		INode *node this is the node that owns the modifier so we can get the right local data
		///		int index this is the index of the point you want to lookup
		virtual Point3* fnGetPointUVW(INode *node, int index)=0;

		/// int Point3 GetPointUVW(INode *node, int index)
		/// This returns the local space point of the deforming point before deformation
		///		INode *node this is the node that owns the modifier so we can get the right local data
		///		int index this is the index of the point you want to lookup
		virtual Point3* fnGetPointLocalSpace(INode *node, int index)=0;

		/// int Point3 GetPointPatchSpace(INode *node, int index)
		/// This returns the point in the space of the patch of the deforming point before deformation
		///		INode *node this is the node that owns the modifier so we can get the right local data
		///		int index this is the index of the point you want to lookup
		virtual Point3* fnGetPointPatchSpace(INode *node, int index)=0;

		/// int int GetPointPatchIndex(INode *node, int index)
		/// This returns closest patch to this point
		///		INode *node this is the node that owns the modifier so we can get the right local data
		///		int index this is the index of the point you want to lookup
		virtual int fnGetPointPatchIndex(INode *node, int index)=0;

	};


class ParamSpaceData
	{
	public:
		Point3	uv;
		Point3	initialPoint; // in space of the uv
		Point3	initialLocalPoint; // in local space
		int patchIndex;
	};

class LocalPatchData : public LocalModData 
	{
	public:
		Tab<ParamSpaceData> paramData;
		INode *selfNode;
		int numPatches;
		float incDelta;
		BOOL resample;
		LocalPatchData()
			{
			selfNode = NULL;
			numPatches = 0;
			resample = FALSE;
			incDelta = 0.0f;
			}
		~LocalPatchData()
			{
			}	
		LocalModData*	Clone()
			{
			LocalPatchData* d = new LocalPatchData();
			return d;
			}	
	};



class PatchDeformPW : public Modifier , public IPatchDeformPWModFP {
	public:


		// Parameter block
		IParamBlock2	*pblock;	//ref 0


		static IObjParam *ip;			//Access to the interface
		
		// From Animatable
		TCHAR *GetObjectName() { return GetString(IDS_CLASS_NAME); }

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


		Interval GetValidity(TimeValue t);

		// Automatic texture support
		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return PATCHDEFORMPW_CLASS_ID;}		
		SClass_ID SuperClassID() { return OSM_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

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

		PatchDeformPW();
		~PatchDeformPW();		

		IOResult SaveLocalData(ISave *isave, LocalModData *pld);
		IOResult LoadLocalData(ILoad *iload, LocalModData **pld);

//published functions		 
		//Function Publishing method (Mixin Interface)
		//******************************
		BaseInterface* GetInterface(Interface_ID id) 
			{ 
			if (id == PATCHDEFORMPW_INTERFACE) 
				return (IPatchDeformPWMod*)this; 
			else 
				return FPMixinInterface::GetInterface(id);
			} 

		INode* GetNodeFromModContext(LocalPatchData *smd);
		LocalPatchData* GetLocalDataFromNode(INode *node);

		void SetResampleModContext(BOOL resample);

		void BuildParamData(Object *obj,LocalPatchData *localData,PatchMesh *patch,Matrix3 patchTM,Matrix3 baseTM);

		HWND hWnd;

		void fnResample();
		void Resample();

		int fnGetNumberOfPoints(INode *node);
		Point3* fnGetPointUVW(INode *node, int index);
		Point3* fnGetPointLocalSpace(INode *node, int index);
		Point3* fnGetPointPatchSpace(INode *node, int index);
		int fnGetPointPatchIndex(INode *node, int index);

		int GetNumberOfPoints(INode *node);
		Point3 GetPointUVW(INode *node, int index);
		Point3 GetPointLocalSpace(INode *node, int index);
		Point3 GetPointPatchSpace(INode *node, int index);
		int GetPointPatchIndex(INode *node, int index);

};


class PatchDeformParamsMapDlgProc : public ParamMap2UserDlgProc {
	public:
		PatchDeformPW *mod;		
		PatchDeformParamsMapDlgProc(PatchDeformPW *m) {mod = m;}		
		INT_PTR DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {
				delete this;
				}
	};


class PatchDeformPWClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE);// { return new PatchDeformPW(); }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return PATCHDEFORMPW_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("SkinWrapPatch"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};








class PatchEnumProc : public DependentEnumProc 
	{
      public :
      virtual int proc(ReferenceMaker *rmaker); 
      INodeTab Nodes;              
	};


class PatchDeformer : public Deformer {
	public:		
		LocalPatchData *localData;
		Matrix3 baseTm,patchTm;
		Matrix3 fromWorldToBaseTm;
		Matrix3 fromPatchToBaseTm;
		PatchMesh *patch;

		PatchDeformer(LocalPatchData *ld, PatchMesh *patch, Matrix3 baseTm, Matrix3 patchTm)
			{
			localData = ld;
			this->baseTm = baseTm;
			this->patchTm = patchTm;
			this->patch = patch;
			fromWorldToBaseTm = Inverse(baseTm);
			fromPatchToBaseTm = patchTm * Inverse(baseTm);
			}

		Point3 Map(int i, Point3 pt)
			{

			Point3 vec = pt - localData->paramData[i].initialLocalPoint;
			float u,v;
			u = localData->paramData[i].uv.x;
			v = localData->paramData[i].uv.y;
			int index = localData->paramData[i].patchIndex;

//get the u vec
			float du = u-localData->incDelta;
			float dv = v-localData->incDelta;

			Patch *p = &patch->patches[index];

			Point3 center = p->interp(patch, u, v);

			Point3 uVec = Normalize(p->interp(patch, du, v)-center);
//get the v vec
			Point3 vVec = Normalize(p->interp(patch, u, dv)-center);

			Point3 xAxis,yAxis,zAxis;
			xAxis = uVec;
			zAxis = CrossProd(uVec,vVec);
			yAxis = CrossProd(xAxis,zAxis);

			

			Matrix3 tm(xAxis,yAxis,zAxis,center);


			pt = localData->paramData[i].initialPoint;
			pt = pt * tm * fromPatchToBaseTm;


			return pt+vec;
			}
	};




#endif // __PATCHDEFORMPW__H
