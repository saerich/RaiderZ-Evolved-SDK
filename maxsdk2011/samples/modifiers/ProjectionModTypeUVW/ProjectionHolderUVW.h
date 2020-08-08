/**********************************************************************
 *<
	FILE:			ProjectionHolderUVW.h
	DESCRIPTION:	Holds the result of a UVW Projection Mapping operation
	CREATED BY:		Michael Russo
	HISTORY:		Created 04-02-2004

 *>	Copyright (c) 2004 Discreet, All Rights Reserved.
 **********************************************************************/

#ifndef __PROJECTIONHOLDUVW__H
#define __PROJECTIONHOLDUVW__H

#include "DllEntry.h"
#include "IProjectionMod.h"
#include "ObjectWrapper.h"

#define VERTEX_CHANNEL_NUM		-100

class ProjectionHolderUVWData {
	public:

		ProjectionHolderUVWData();
		~ProjectionHolderUVWData();

		void		Reset();
		void		Clone(ProjectionHolderUVWData *pNewData);

		enum { 
			SOURCE_TYPE_CHUNK=0x110,
			MAP_CHANNEL_CHUNK=0x120,
			P3_DATA_COUNT_CHUNK=0x130,
			P3_DATA_CHUNK=0x140,
			FACES_DATA_COUNT_CHUNK=0x150,
			FACES_DATA_CHUNK=0x160,
			MATID_DO_CHUNK=0x170,
			MATID_COUNT_CHUNK=0x180,
			MATID_CHUNK=0x190
		};

		IOResult	Load(ILoad *iload);
		IOResult	Save(ISave *isave);


		Tab<Point3> mP3Data;
		Tab<GenFace> mFaces;
		Tab<int> mMatID;
		Interval mSrcInterval;

		enum { source_uvw, source_vert };
		int			miSource;
		int			miMapChannel;
		bool		mDoMaterialIDs;
};


#define PROJECTIONHOLDERUVW_CLASS_ID	Class_ID(0x42a0115b, 0x2a504ded)
#define PBLOCK_REF 0

class ProjectionHolderUVW : public Modifier {
	public:
		//Constructor/Destructor
		ProjectionHolderUVW();
		~ProjectionHolderUVW();

		//ParamBlock params
		enum { params };
		enum { pb_projector };

		Interval GetValidity(TimeValue t);

		ProjectionHolderUVWData &GetHolderData() { return mData; };
		void SetProjectionTypeLink(ReferenceTarget *pProjector);
		ReferenceTarget *GetProjectionTypeLink();

		//-- From Animatable
		void		DeleteThis() { delete this; }
		void		GetClassName(TSTR& s) {s = GetString(IDS_PROJECTIONHOLDERUVW_CLASS_NAME);}
		Class_ID	ClassID() {return PROJECTIONHOLDERUVW_CLASS_ID;}		
		SClass_ID	SuperClassID() { return OSM_CLASS_ID; }

		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

		// Prevent us from being copied in TrackView/ModifierStack.
		BOOL		CanCopyAnim() {return TRUE;}

		int			NumSubs() { return 1; }
		Animatable*	SubAnim(int i) { return mpPBlock; }
		TSTR		SubAnimName(int i) { return GetString(IDS_PARAMS); }				

		//-- From ReferenceMaker
		IOResult	Load(ILoad *iload);
		IOResult	Save(ISave *isave);

		int			NumRefs() { return 1; }
		RefTargetHandle GetReference(int i) { return mpPBlock; }
		void		SetReference(int i, RefTargetHandle rtarg) { mpPBlock=(IParamBlock2*)rtarg; }

		int			NumParamBlocks() { return 1; }
		IParamBlock2* GetParamBlock(int i) { return mpPBlock; }
		IParamBlock2* GetParamBlockByID(BlockID id) { return (mpPBlock->ID() == id) ? mpPBlock : NULL; }

		RefResult	NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID,  RefMessage message);

		//-- From ReferenceTarget
		RefTargetHandle Clone( RemapDir &remap );

		//-- From BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}
		TCHAR*		GetObjectName() { return GetString(IDS_PROJECTIONHOLDERUVW_CLASS_NAME); }
		BOOL		ChangeTopology() {return TRUE;}

		BOOL		HasUVW();
		void		SetGenUVW(BOOL sw);

		void		NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index);
		void		NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index);

		//-- From Modifier
		Interval	LocalValidity(TimeValue t);

		ChannelMask ChannelsUsed()  { return PART_GEOM|PART_TOPO|PART_VERTCOLOR|TEXMAP_CHANNEL; }
		ChannelMask ChannelsChanged() { return PART_GEOM|PART_TOPO|TEXMAP_CHANNEL|PART_VERTCOLOR; }
		void		NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);
		void		ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);

		Class_ID	InputType() {return mapObjectClassID;}

	protected:
		static IObjParam *ip;		//Access to the interface
		IParamBlock2	*mpPBlock;	//ref 0
		ProjectionHolderUVWData mData;
};

#endif // __PROJECTIONHOLDUVW__H
