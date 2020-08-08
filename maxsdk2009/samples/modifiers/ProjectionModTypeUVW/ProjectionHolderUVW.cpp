/**********************************************************************
 *<
	FILE:			ProjectionHolderUVW.cpp
	DESCRIPTION:	Holds the result of a UVW Projection Mapping operation
	CREATED BY:		Michael Russo
	HISTORY:		Created 04-02-2004

 *>	Copyright (c) 2004 Discreet, All Rights Reserved.
 **********************************************************************/

#include "ProjectionHolderUVW.h"


//===========================================================================
//
// Class ProjectionHolderUVW
//
//===========================================================================


//***************************************************************************
// Class Descriptor
//***************************************************************************

class ProjectionHolderUVWClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() { return FALSE; }
	void *			Create(BOOL loading = FALSE) { return new ProjectionHolderUVW(); }
	const TCHAR *	ClassName() { return GetString(IDS_PROJECTIONHOLDERUVW_CLASS_NAME); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return PROJECTIONHOLDERUVW_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_PROJECTIONHOLDERUVW_CATEGORY); }

	const TCHAR*	InternalName() { return _T("ProjectionHolderUVW"); } // returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; } // returns owning module handle

};


//***************************************************************************
// ParamBlock Descriptor
//***************************************************************************

static ProjectionHolderUVWClassDesc ProjectionHolderUVWDesc;
ClassDesc2* GetProjectionHolderUVWDesc() { return &ProjectionHolderUVWDesc; }

static ParamBlockDesc2 ProjectionHolderUVW_param_blk (
	ProjectionHolderUVW::params,		_T("params"),  0, &ProjectionHolderUVWDesc, P_AUTO_CONSTRUCT , PBLOCK_REF, 
	// params
	ProjectionHolderUVW::pb_projector, _T("projector"), TYPE_REFTARG, P_SUBANIM|P_READ_ONLY, IDS_PARAM_PROJECTOR,
		end, 
	end
	);

IObjParam* ProjectionHolderUVW::ip = NULL;


//***************************************************************************
// Constructor/Destructor
//***************************************************************************

ProjectionHolderUVW::ProjectionHolderUVW()
{
	mpPBlock = NULL; 
	ProjectionHolderUVWDesc.MakeAutoParamBlocks(this);
	DbgAssert( mpPBlock!=NULL );
}

ProjectionHolderUVW::~ProjectionHolderUVW()
{
}

//***************************************************************************
// Class methods
//***************************************************************************
Interval ProjectionHolderUVW::GetValidity(TimeValue t)
{
	Interval valid = FOREVER;
	return valid;
}

void ProjectionHolderUVW::SetProjectionTypeLink(ReferenceTarget *pProjector)
{
	if( mpPBlock )
		mpPBlock->SetValue(pb_projector, 0, pProjector);
}

ReferenceTarget *ProjectionHolderUVW::GetProjectionTypeLink()
{
	ReferenceTarget *pRef = NULL;
	if( mpPBlock )
		mpPBlock->GetValue(pb_projector, 0, pRef, FOREVER);

	return pRef;
}

//***************************************************************************
// From Animatable
//***************************************************************************

void ProjectionHolderUVW::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);	

	ProjectionHolderUVWDesc.BeginEditParams(ip, this, flags, prev);
}

void ProjectionHolderUVW::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	ProjectionHolderUVWDesc.EndEditParams(ip, this, flags, next);

	TimeValue t = ip->GetTime();
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
	this->ip = NULL;
}


//***************************************************************************
// From ReferenceMaker
//***************************************************************************

IOResult ProjectionHolderUVW::Load(ILoad *iload)
{
	return mData.Load(iload);
}

IOResult ProjectionHolderUVW::Save(ISave *isave)
{
	return mData.Save(isave);
}

RefResult ProjectionHolderUVW::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	return REF_SUCCEED;
}


//***************************************************************************
// From ReferenceTarget
//***************************************************************************

RefTargetHandle ProjectionHolderUVW::Clone(RemapDir& remap)
{
	ProjectionHolderUVW* newmod = new ProjectionHolderUVW();	
	newmod->ReplaceReference(PBLOCK_REF,remap.CloneRef(mpPBlock));

	mData.Clone(&newmod->mData);

	BaseClone(this, newmod, remap);
	return(newmod);
}

//***************************************************************************
// From BaseObject
//***************************************************************************

BOOL ProjectionHolderUVW::HasUVW() 
{ 
	return TRUE; 
}

void ProjectionHolderUVW::SetGenUVW(BOOL sw) 
{  
	if (sw==HasUVW()) return;
}

//	Between NotifyPreCollapse and NotifyPostCollapse, Modify is called by
//  the system.  NotifyPreCollapse can be used to save any plugin dependant data e.g. LocalModData
void ProjectionHolderUVW::NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index)
{
}

void ProjectionHolderUVW::NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index)
{
}


//***************************************************************************
// From Modifier
//***************************************************************************

//	The validity of the parameters.  First a test for editing is performed
//  then Start at FOREVER, and intersect with the validity of each item
Interval ProjectionHolderUVW::LocalValidity(TimeValue t)
{
	// if being edited, return NEVER forces a cache to be built 
	// after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;  
	return NEVER;
}

//NotifyInputChanged is called each time the input object is changed in some way
//We can find out how it was changed by checking partID and message
void ProjectionHolderUVW::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc)
{
}

//ModifyObject will do all the work in a full modifier
//This includes casting objects to their correct form, doing modifications
//changing their parameters, etc
void ProjectionHolderUVW::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
{
	ObjectWrapper object;
	object.Init(t,*os,false, ObjectWrapper::allEnable, ObjectWrapper::triObject);

	if( object.IsEmpty() )
		return;

	// Check topology
	if( object.NumFaces() != mData.mFaces.Count() )
		return;
	
	for( int i=0; i<object.NumFaces(); i++ ) {
		GenFace face = object.GetFace(i);
		if( face.numVerts != mData.mFaces[i].numVerts )
			return;
	}

	if( mData.miMapChannel != VERTEX_CHANNEL_NUM ) {
		// Check Channel Support
		if( !object.GetChannelSupport(mData.miMapChannel) ) 
			object.SetChannelSupport( mData.miMapChannel, true );

		object.SetNumMapVerts( mData.miMapChannel, mData.mP3Data.Count() );

		for( int i=0; i<mData.mP3Data.Count(); i++ )
			object.SetMapVert( mData.miMapChannel, i, mData.mP3Data[i] );

		for( int i=0; i<mData.mFaces.Count(); i++ ) {
			object.SetMapFace( mData.miMapChannel, i, mData.mFaces[i] );
			// Material ID Support
			if( mData.mDoMaterialIDs )
				object.SetMtlID(i, mData.mMatID[i] );
		}

		os->obj->PointsWereChanged();
	}
	else {
		object.SetNumVerts( mData.mP3Data.Count() );

		for( int i=0; i<mData.mP3Data.Count(); i++ )
			object.SetVert( i, mData.mP3Data[i] );

		for( int i=0; i<mData.mFaces.Count(); i++ ) {
			object.SetFace( i, mData.mFaces[i] );
			// Material ID Support
			if( mData.mDoMaterialIDs )
				object.SetMtlID(i, mData.mMatID[i] );
		}

		os->obj->PointsWereChanged();
		if( object.Type() == ObjectWrapper::polyObject ) {
			MNMesh *poly = object.GetPolyMesh();
			if( poly )
				poly->FillInMesh();
		}
		else if( object.Type() == ObjectWrapper::triObject ) {
			Mesh *mesh = object.GetTriMesh();
			if( mesh )
				mesh->InvalidateTopologyCache();
		}
	}

	Interval iv;
	iv = FOREVER;
	iv &= mData.mSrcInterval;

	if( mData.miMapChannel > 0 ) 
		iv &= os->obj->ChannelValidity (t, TEXMAP_CHAN_NUM);
	else if( mData.miMapChannel == VERTEX_CHANNEL_NUM ) {
		iv &= os->obj->ChannelValidity(t,GEOM_CHAN_NUM);
		iv &= os->obj->ChannelValidity(t,TOPO_CHAN_NUM);
	}
	else
		iv &= os->obj->ChannelValidity(t,VERT_COLOR_CHAN_NUM);

	if( mData.miMapChannel > 0 ) 
		os->obj->UpdateValidity(TEXMAP_CHAN_NUM,iv);
	else if( mData.miMapChannel == VERTEX_CHANNEL_NUM ) {
		os->obj->UpdateValidity(GEOM_CHAN_NUM,iv);	
		os->obj->UpdateValidity(TOPO_CHAN_NUM,iv);	
	}
	else
		os->obj->UpdateValidity(VERT_COLOR_CHAN_NUM,iv);
}


//===========================================================================
//
// Class ProjectionHolderUVWData
//
//===========================================================================

ProjectionHolderUVWData::ProjectionHolderUVWData()
{
	Reset();
}

ProjectionHolderUVWData::~ProjectionHolderUVWData()
{
	Reset();
}

void ProjectionHolderUVWData::Reset()
{
	miSource = source_uvw;
	miMapChannel = 1;

	mP3Data.ZeroCount();

	for( int i=0; i<mFaces.Count(); i++ ) {
		if( mFaces[i].verts )
			delete [] mFaces[i].verts;
	}

	mFaces.ZeroCount();

	mMatID.ZeroCount();

	mSrcInterval = FOREVER;

	mDoMaterialIDs = false;
}

void ProjectionHolderUVWData::Clone(ProjectionHolderUVWData *pNewData)
{
	if( !pNewData )
		return;

	pNewData->miSource = this->miSource;
	pNewData->miMapChannel = this->miMapChannel;
	pNewData->mDoMaterialIDs = this->mDoMaterialIDs;
	pNewData->mSrcInterval = this->mSrcInterval;

	pNewData->mMatID.SetCount(this->mMatID.Count());
	for( int i=0; i<this->mMatID.Count(); i++ )
		pNewData->mMatID[i] = this->mMatID[i];

	pNewData->mP3Data.SetCount(this->mP3Data.Count());
	for( int i=0; i<this->mP3Data.Count(); i++ )
		pNewData->mP3Data[i] = this->mP3Data[i];

	pNewData->mFaces.SetCount(this->mFaces.Count());
	for( int i=0; i<this->mFaces.Count(); i++ ) {
		pNewData->mFaces[i].numVerts = this->mFaces[i].numVerts;
		pNewData->mFaces[i].verts = new DWORD[pNewData->mFaces[i].numVerts];
		for( int j=0; j<pNewData->mFaces[i].numVerts; j++ )
			pNewData->mFaces[i].verts[j] = this->mFaces[i].verts[j];
	}
}

IOResult ProjectionHolderUVWData::Load(ILoad *pILoad)
{
	IOResult res;
	int iP3Count = 0;
	int iGenFaceCount = 0;
	int iMatIDCount = 0;

	while (IO_OK==(res=pILoad->OpenChunk())) {
		ULONG uRead;
		switch(pILoad->CurChunkID())  {
		case SOURCE_TYPE_CHUNK:
			pILoad->Read( (void*)&miSource, sizeof(int), &uRead );
			break;
		case MAP_CHANNEL_CHUNK:
			pILoad->Read( (void*)&miMapChannel, sizeof(int), &uRead );
			break;
		case P3_DATA_COUNT_CHUNK:
			pILoad->Read( (void*)&iP3Count, sizeof(int), &uRead );
			break;
		case P3_DATA_CHUNK:
			if( iP3Count > 0 ) {
				mP3Data.SetCount(iP3Count);
				pILoad->Read(mP3Data.Addr(0), iP3Count*sizeof(Point3), &uRead);
			}
			break;
		case MATID_DO_CHUNK:
			pILoad->Read( (void*)&mDoMaterialIDs, sizeof(bool), &uRead );
			break;
		case MATID_COUNT_CHUNK:
			pILoad->Read( (void*)&iMatIDCount, sizeof(int), &uRead );
			break;
		case MATID_CHUNK:
			if( iMatIDCount > 0 ) {
				mMatID.SetCount(iMatIDCount);
				pILoad->Read(mMatID.Addr(0), iMatIDCount*sizeof(int), &uRead);
			}
			break;
		case FACES_DATA_COUNT_CHUNK:
			pILoad->Read( (void*)&iGenFaceCount, sizeof(int), &uRead );
			break;
		case FACES_DATA_CHUNK:
			if( iGenFaceCount > 0 ) {
				mFaces.SetCount(iGenFaceCount);
				for( int i=0; i<iGenFaceCount; i++ ) {
					unsigned char iNumVerts;
					pILoad->Read( (void*)&iNumVerts, sizeof(unsigned char), &uRead );
					mFaces[i].numVerts = iNumVerts;
					if( iNumVerts > 0 ) {
						mFaces[i].verts = new DWORD[iNumVerts];
						pILoad->Read(mFaces[i].verts, iNumVerts*sizeof(DWORD), &uRead);
					}
				}
			}
			break;
		}
		pILoad->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}

	return IO_OK;

}

IOResult ProjectionHolderUVWData::Save(ISave *pISave)
{
	ULONG nb;

	pISave->BeginChunk(SOURCE_TYPE_CHUNK); 
	pISave->Write( (void*)&miSource, sizeof(int), &nb );
	pISave->EndChunk();

	pISave->BeginChunk(MAP_CHANNEL_CHUNK); 
	pISave->Write( (void*)&miMapChannel, sizeof(int), &nb );
	pISave->EndChunk();

	int iCount = mP3Data.Count();
	pISave->BeginChunk(P3_DATA_COUNT_CHUNK);
	pISave->Write( (void*)&iCount, sizeof(int), &nb );
	pISave->EndChunk();

	if( mP3Data.Count() > 0 ) {
		pISave->BeginChunk(P3_DATA_CHUNK); 
		pISave->Write( mP3Data.Addr(0), iCount*sizeof(Point3), &nb );
		pISave->EndChunk();
	}

	// FIXME
	iCount = mFaces.Count();
	pISave->BeginChunk(FACES_DATA_COUNT_CHUNK);
	pISave->Write( (void*)&iCount, sizeof(int), &nb );
	pISave->EndChunk();

	if( mFaces.Count() > 0 ) {
		pISave->BeginChunk(FACES_DATA_CHUNK); 
		for(int i=0; i<iCount; i++ ) {
			pISave->Write( &(mFaces[i].numVerts), sizeof(unsigned char), &nb);
			if( mFaces[i].numVerts > 0 )
				pISave->Write( mFaces[i].verts, mFaces[i].numVerts*sizeof(DWORD), &nb );
		}
		pISave->EndChunk();
	}

	pISave->BeginChunk(MATID_DO_CHUNK); 
	pISave->Write( (void*)&mDoMaterialIDs, sizeof(bool), &nb );
	pISave->EndChunk();

	iCount = mMatID.Count();
	pISave->BeginChunk(MATID_COUNT_CHUNK);
	pISave->Write( (void*)&iCount, sizeof(int), &nb );
	pISave->EndChunk();

	if( mMatID.Count() > 0 ) {
		pISave->BeginChunk(MATID_CHUNK); 
		pISave->Write( mMatID.Addr(0), iCount*sizeof(int), &nb );
		pISave->EndChunk();
	}

	return IO_OK;
}

