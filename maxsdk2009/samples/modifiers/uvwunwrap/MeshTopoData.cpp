/********************************************************************** *<
FILE: MeshTopoData.cpp

DESCRIPTION: local mode data for the unwrap

HISTORY: 9/25/2006
CREATED BY: Peter Watje




*>	Copyright (c) 2006, All Rights Reserved.
**********************************************************************/

#include "unwrap.h"

extern void Draw3dEdge(GraphicsWindow *gw, float size, Point3 a, Point3 b, Color c);

MeshTopoData::MeshTopoData() 
{ 
	mLoaded = FALSE;
	Init();
}

MeshTopoData::MeshTopoData(ObjectState *os, int mapChannel)
{
	mLoaded = FALSE;
	Init();
	Reset(os,mapChannel);

}

void MeshTopoData::SetGeoCache(ObjectState *os)
{
#ifndef NO_PATCHES
	if (os->obj->IsSubClassOf(patchObjectClassID) && (patch == NULL))
	{
		PatchObject *pobj = (PatchObject*)os->obj;
		patch = new PatchMesh(pobj->patch);	
		if (TVMaps.f.Count() == patch->numPatches)
		{
			for (int j=0; j<TVMaps.f.Count(); j++) 
			{
				int pcount = 3;
				if (patch->patches[j].type == PATCH_QUAD) 
				{
					pcount = 4;
				}
				for (int k = 0; k < pcount; k++)
				{
					int index = patch->patches[j].v[k];
					TVMaps.f[j]->v[k] = index;
					//do handles and interiors
					//check if linear
					if (!(patch->patches[j].flags & PATCH_LINEARMAPPING))
					{
						//do geometric points
						index = patch->patches[j].interior[k];
						TVMaps.f[j]->vecs->vinteriors[k] =patch->getNumVerts()+index;
						index = patch->patches[j].vec[k*2];
						TVMaps.f[j]->vecs->vhandles[k*2] =patch->getNumVerts()+index;
						index = patch->patches[j].vec[k*2+1];
						TVMaps.f[j]->vecs->vhandles[k*2+1] =patch->getNumVerts()+index;

					}

				}
			}
			TVMaps.BuildGeomEdges();
			mGESel.SetSize(TVMaps.gePtrList.Count());
			mGESel.ClearAll();
		}
	}
	else 
#endif // NO_PATCHES
	if (os->obj->IsSubClassOf(triObjectClassID) && (mesh == NULL))
	{			
		TriObject *tobj = (TriObject*)os->obj;		
		mesh = new Mesh(tobj->mesh);

		if (TVMaps.f.Count() == mesh->numFaces)
		{
			for (int j = 0; j < mesh->numFaces; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					int index = mesh->faces[j].v[k];
					TVMaps.f[j]->v[k] = index;
				}
			}
			TVMaps.BuildGeomEdges();
			mGESel.SetSize(TVMaps.gePtrList.Count());
			mGESel.ClearAll();
		}
	}
	else if (os->obj->IsSubClassOf(polyObjectClassID)&& (mnMesh==NULL))
	{
		PolyObject *pobj = (PolyObject*)os->obj;
		mnMesh = new MNMesh(pobj->mm);

		if (TVMaps.f.Count() == mnMesh->numf)
		{

			for (int j = 0; j < mnMesh->numf; j++)
			{
				int fct = mnMesh->f[j].deg;
				for (int k = 0; k < fct; k++)
				{
					int index = mnMesh->f[j].vtx[k];
					TVMaps.f[j]->v[k] = index;
				}
			}
			TVMaps.BuildGeomEdges();
			mGESel.SetSize(TVMaps.gePtrList.Count());
			mGESel.ClearAll();
		}
	}
	if (
#ifndef NO_PATCHES // orb 02-05-03
		(!os->obj->IsSubClassOf(patchObjectClassID)) && 
#endif // NO_PATCHES		
		(!os->obj->IsSubClassOf(triObjectClassID)) &&
		(!os->obj->IsSubClassOf(polyObjectClassID)) && (mesh == NULL) )
	{
		//neither patch or poly convert to a mesh
		if (os->obj->CanConvertToType(triObjectClassID))
		{
			TimeValue t = GetCOREInterface()->GetTime();
			TriObject *tri = (TriObject *) os->obj->ConvertToType(t, Class_ID(TRIOBJ_CLASS_ID, 0));
			mesh = new Mesh(tri->GetMesh());

			if (TVMaps.f.Count() == mesh->numFaces)
			{

				for (int j = 0; j < mesh->numFaces; j++)
				{
					for (int k = 0; k < 3; k++)
					{
						int index = mesh->faces[j].v[k];
						TVMaps.f[j]->v[k] = index;
					}
				}
				TVMaps.BuildGeomEdges();
				mGESel.SetSize(TVMaps.gePtrList.Count());
				mGESel.ClearAll();
			}
			delete tri;
		}
	}

}


void MeshTopoData::Reset(ObjectState *os, int channel)
{
	FreeCache(); 
#ifndef NO_PATCHES // orb 02-05-03
	if (os->obj->IsSubClassOf(patchObjectClassID))
	{
		PatchObject *pobj = (PatchObject*)os->obj;
		SetCache(pobj->patch, channel);
	}
	else 
#endif // NO_PATCHES
	if (os->obj->IsSubClassOf(triObjectClassID))
	{			
		TriObject *tobj = (TriObject*)os->obj;
		SetCache(tobj->GetMesh(), channel);
	}
	else if (os->obj->IsSubClassOf(polyObjectClassID))
	{
		PolyObject *pobj = (PolyObject*)os->obj;
		SetCache(pobj->mm, channel);
	}
	if (
#ifndef NO_PATCHES // orb 02-05-03
		(!os->obj->IsSubClassOf(patchObjectClassID)) && 
#endif // NO_PATCHES		
		(!os->obj->IsSubClassOf(triObjectClassID)) &&
		(!os->obj->IsSubClassOf(polyObjectClassID)) )
	{
		//neither patch or poly convert to a mesh
		if (os->obj->CanConvertToType(triObjectClassID))
		{
			TimeValue t = GetCOREInterface()->GetTime();
			TriObject *tri = (TriObject *) os->obj->ConvertToType(t, Class_ID(TRIOBJ_CLASS_ID, 0));
			os->obj = tri;
			os->obj->UnlockObject();			
			SetCache(tri->GetMesh(), channel);
		}
	}

	mVSel.SetSize(TVMaps.v.Count());
	mVSel.ClearAll();

	mFSel.SetSize(TVMaps.f.Count());
	mFSel.ClearAll();

	mGVSel.SetSize(TVMaps.geomPoints.Count());
	mGVSel.ClearAll();

	TVMaps.BuildGeomEdges();

	mSeamEdges.SetSize( TVMaps.ePtrList.Count(),TRUE);

	if (mESel.GetSize() != TVMaps.ePtrList.Count())
	{
		mESel.SetSize(TVMaps.ePtrList.Count());
		mESel.ClearAll();
	}

	if (mGESel.GetSize() != TVMaps.gePtrList.Count())
	{
		mGESel.SetSize(TVMaps.gePtrList.Count());
		mGESel.ClearAll();
	}


	TVMaps.edgesValid = FALSE;

}

void MeshTopoData::Init()
{



	this->mesh = NULL;
	this->patch =  NULL;
	this->mnMesh = NULL;
	mHasIncomingSelection = FALSE;
	TVMaps.mGeoEdgesValid = FALSE;
	TVMaps.edgesValid = FALSE;
	mUserCancel = FALSE;
}

MeshTopoData::~MeshTopoData() 
{ 
	FreeCache(); 
}

LocalModData *MeshTopoData::Clone() {
	MeshTopoData *d = new MeshTopoData;
	d->mesh = NULL;
	d->mnMesh = NULL;
	d->patch = NULL;
	
	d->TVMaps    = TVMaps;
	d->TVMaps.channel = TVMaps.channel;
	d->TVMaps.v = TVMaps.v;
	d->TVMaps.geomPoints = TVMaps.geomPoints;
	d->TVMaps.CloneFaces(TVMaps.f);

	d->TVMaps.e.Resize(0); // LAM - 8/23/04 - in case mod is currently active in modify panel
	d->TVMaps.ge.Resize(0);
	d->TVMaps.ePtrList.Resize(0); // LAM - 8/23/04 - in case mod is currently active in modify panel
	d->TVMaps.gePtrList.Resize(0);

	d->mVSel  = mVSel;

	d->mESel = mESel;
	d->mFSel = mFSel;
	d->mGESel = mGESel;
	d->mGVSel = mGVSel;


	return d;
}


void MeshTopoData::FreeCache() {
	if (mesh) delete mesh;
	mesh = NULL;
	if (patch) delete patch;
	patch = NULL;
	if (mnMesh) delete mnMesh;
	mnMesh = NULL;
	
	int ct = gfaces.Count();
	for (int i =0; i < ct; i++)
	{
		if (gfaces[i]->vecs) delete gfaces[i]->vecs;
		gfaces[i]->vecs = NULL;
		delete gfaces[i];
		gfaces[i] = NULL;
	}
	TVMaps.FreeFaces();
	TVMaps.FreeEdges();
	TVMaps.f.SetCount(0);
	TVMaps.v.SetCount(0);

}

BOOL MeshTopoData::HasCacheChanged(ObjectState *os)
{
	BOOL reset = FALSE;
#ifndef NO_PATCHES // orb 02-05-03
	if (os->obj->IsSubClassOf(patchObjectClassID))
	{
		PatchObject *pobj = (PatchObject*)os->obj;
		if ((patch == NULL) || (pobj->patch.numPatches != patch->numPatches) || (TVMaps.f.Count()!=patch->numPatches))
		{
			reset = TRUE;			
		}
		//no make sure all faces have the same degree
		if (!reset && patch)
		{
			BOOL faceSubSel = FALSE;
			if (pobj->patch.selLevel == PATCH_PATCH)
				faceSubSel = TRUE;

			for (int i = 0; i < patch->numPatches; i++)
			{

				if (faceSubSel)
				{
					if (pobj->patch.patchSel[i] && GetFaceDead(i))
					{
						reset = TRUE;
					}
					else if (!pobj->patch.patchSel[i] && !GetFaceDead(i))
					{
						reset = TRUE;
					}
				}
				else
				{
					if (GetFaceDead(i))
						reset = TRUE;
				}

				if (pobj->patch.patches[i].type != patch->patches[i].type)
				{
					reset = TRUE;
					i = patch->numPatches;
				}
				else
				{
					int deg = 3;
					if (pobj->patch.patches[i].type == PATCH_QUAD)
						deg = 4;

					for (int j = 0; j < deg; j++)
					{
						int a = patch->patches[i].v[j];
						int b =  pobj->patch.patches[i].v[j];
						if (a != b)
						{
							TVMaps.mGeoEdgesValid = FALSE;
						}
					}
				}
			}
		}
	}
	else 
#endif // NO_PATCHES
	if (os->obj->IsSubClassOf(triObjectClassID))
	{			
		TriObject *tobj = (TriObject*)os->obj;
		if ((mesh == NULL) || (tobj->mesh.numFaces != mesh->numFaces) || (TVMaps.f.Count() != mesh->numFaces))
		{
			reset = TRUE;
		}
		if (!reset && mesh)
		{
			BOOL faceSubSel = FALSE;
			if (tobj->mesh.selLevel == MESH_FACE)
				faceSubSel = TRUE;
			for (int i = 0; i < mesh->numFaces; i++)
			{
				if (faceSubSel)
				{
					if (tobj->mesh.faceSel[i] && GetFaceDead(i))
					{
						reset = TRUE;
					}
					else if (!tobj->mesh.faceSel[i] && !GetFaceDead(i))
					{
						reset = TRUE;
					}
				}
				else
				{
					if (GetFaceDead(i))
						reset = TRUE;
				}
				int deg = 3;
				for (int j = 0; j < deg; j++)
				{
					int a = mesh->faces[i].v[j];
					int b = tobj->mesh.faces[i].v[j];
					if (a != b)
					{
						TVMaps.mGeoEdgesValid = FALSE;
					}
				}
			}
		}
	}
	else if (os->obj->IsSubClassOf(polyObjectClassID))
	{
		PolyObject *pobj = (PolyObject*)os->obj;
		if ((mnMesh == NULL) || (pobj->mm.numf != mnMesh->numf) || (TVMaps.f.Count() != mnMesh->numf))
		{
			reset = TRUE;		
		}
		//no make sure all faces have the same degree
		if (!reset && mnMesh)
		{
			BOOL faceSubSel = FALSE;			
			if ( pobj->mm.selLevel == MNM_SL_FACE)
				faceSubSel = TRUE;

			BitArray bs;
			pobj->mm.getFaceSel(bs);

			for (int i = 0; i < pobj->mm.numf; i++)
			{

				if (faceSubSel)
				{
					if (bs[i] && GetFaceDead(i))
					{
						reset = TRUE;
					}
					else if (!bs[i] && !GetFaceDead(i))
					{
						reset = TRUE;
					}
				}
				else
				{
					if (GetFaceDead(i))						
						reset = TRUE;
				}

				if (mnMesh->f[i].deg != pobj->mm.f[i].deg)
				{
					reset = TRUE;
					i = pobj->mm.numf;
				}
				else
				{
					int deg = pobj->mm.f[i].deg;
					for (int j = 0; j < deg; j++)
					{
						int a = mnMesh->f[i].vtx[j];
						int b =  pobj->mm.f[i].vtx[j];
						if (a != b)
						{
							TVMaps.mGeoEdgesValid = FALSE;
						}
					}
				}
			}
		}
	}
	if (
#ifndef NO_PATCHES // orb 02-05-03
		(!os->obj->IsSubClassOf(patchObjectClassID)) && 
#endif // NO_PATCHES		
		(!os->obj->IsSubClassOf(triObjectClassID)) &&
		(!os->obj->IsSubClassOf(polyObjectClassID)) )
	{
		//neither patch or poly convert to a mesh
		if (os->obj->CanConvertToType(triObjectClassID))
		{
			TimeValue t = GetCOREInterface()->GetTime();
			TriObject *tobj = (TriObject *) os->obj->ConvertToType(t, Class_ID(TRIOBJ_CLASS_ID, 0));

			if ((mesh == NULL) || (tobj->mesh.numFaces != mesh->numFaces))
			{
				reset = TRUE;
			}
			if (!reset && mesh)
			{
				BOOL faceSubSel = FALSE;
				if (tobj->mesh.selLevel == MESH_FACE)
					faceSubSel = TRUE;
				for (int i = 0; i < mesh->numFaces; i++)
				{
					if (faceSubSel)
					{
						if (tobj->mesh.faceSel[i] && GetFaceDead(i))
						{
							reset = TRUE;
						}
						else if (!tobj->mesh.faceSel[i] && !GetFaceDead(i))
						{
							reset = TRUE;
						}
					}
					else
					{
						if (GetFaceDead(i))
							reset = TRUE;
					}

					int deg = 3;
					for (int j = 0; j < deg; j++)
					{
						int a = mesh->faces[i].v[j];
						int b = tobj->mesh.faces[i].v[j];
						if (a != b)
						{
							TVMaps.mGeoEdgesValid = FALSE;
						}
					}
				}
			}
			if (tobj != os->obj)
				delete tobj;
		}
	}

	return reset;
}

void MeshTopoData::UpdateGeoData(TimeValue t, ObjectState *os)
{
	BOOL remapGeoData = FALSE;
	if ( GetNumberGeomVerts() != os->obj->NumPoints())
	{		
		SetNumberGeomVerts(os->obj->NumPoints());
		remapGeoData = TRUE;
	}
	for (int i = 0; i < GetNumberGeomVerts(); i++)
	{
		SetGeomVert(i,os->obj->GetPoint(i));
	}



	if (!TVMaps.mGeoEdgesValid)
	{
		remapGeoData = TRUE;

	}




	if (os->obj->IsSubClassOf(triObjectClassID))
	{
		TriObject *tobj = (TriObject*)os->obj;
		mHiddenPolygonsFromPipe.SetSize(tobj->GetMesh().getNumFaces());
		mHiddenPolygonsFromPipe.ClearAll();
		for (int i = 0; i < tobj->GetMesh().getNumFaces(); i++)
		{
			if (tobj->GetMesh().faces[i].Hidden())
				mHiddenPolygonsFromPipe.Set(i,TRUE);
		}
		if (tobj->GetMesh().selLevel != MESH_FACE)
			tobj->GetMesh().faceSel = mFSel;
		if (GetCOREInterface()->GetSubObjectLevel() == 3)
			tobj->GetMesh().SetDispFlag(DISP_SELFACES);
	}
	else if (os->obj->IsSubClassOf(polyObjectClassID))
	{
		PolyObject *pobj = (PolyObject*)os->obj;
		mHiddenPolygonsFromPipe.SetSize(pobj->GetMesh().numf);
		mHiddenPolygonsFromPipe.ClearAll();
		for (int i = 0; i < pobj->GetMesh().numf; i++)
		{
			if (pobj->GetMesh().f[i].GetFlag(MN_HIDDEN))
				mHiddenPolygonsFromPipe.Set(i,TRUE);
		}
		if (pobj->GetMesh().selLevel != MNM_SL_FACE)
			pobj->GetMesh().FaceSelect(mFSel);
		if (GetCOREInterface()->GetSubObjectLevel() == 3)
			pobj->GetMesh().SetDispFlag(MNDISP_SELFACES);
	}
#ifndef NO_PATCHES // orb 02-05-03
	else if (os->obj->IsSubClassOf(patchObjectClassID))
	{
		PatchObject *pobj = (PatchObject*)os->obj;
		mHiddenPolygonsFromPipe.SetSize(pobj->patch.numPatches);
		mHiddenPolygonsFromPipe.ClearAll();
		for (int i = 0; i < pobj->patch.numPatches; i++)
		{
			if (pobj->patch.patches[i].IsHidden())
				mHiddenPolygonsFromPipe.Set(i,TRUE);
		}
		if (pobj->patch.selLevel != PATCH_PATCH)
			pobj->patch.patchSel = mFSel;
		if (GetCOREInterface()->GetSubObjectLevel() == 3)
			pobj->patch.SetDispFlag(DISP_SELPATCHES|DISP_LATTICE);

	}
#endif

	if (remapGeoData)
	{
		//this sets the geo edges and cache
		if (mesh) delete mesh;
		mesh = NULL;
		if (patch) delete patch;
		patch = NULL;
		if (mnMesh) delete mnMesh;
		mnMesh = NULL;

		int ct = gfaces.Count();
		for (int i =0; i < ct; i++)
		{
			if (gfaces[i]->vecs) delete gfaces[i]->vecs;
			gfaces[i]->vecs = NULL;
			delete gfaces[i];
			gfaces[i] = NULL;
		}
		SetGeoCache(os);
	}




	if (mGVSel.GetSize() != TVMaps.geomPoints.Count())
	{	
		mGVSel.SetSize(TVMaps.geomPoints.Count());
		mGVSel.ClearAll();
	}

	

	if (!TVMaps.edgesValid)
	{
		TVMaps.BuildEdges();
		BuildVertexClusterList();
		
	}

	if (mESel.GetSize() != TVMaps.ePtrList.Count())
	{
		mESel.SetSize(TVMaps.ePtrList.Count());
		mESel.ClearAll();
	}

	TVMaps.mGeoEdgesValid = TRUE;
	TVMaps.edgesValid = TRUE;
}


void MeshTopoData::CopyFaceSelection(ObjectState *os)
{

}

void MeshTopoData::ApplyMapping(ObjectState *os, int mapChannel,TimeValue t)
{
	if (os->obj->IsSubClassOf(triObjectClassID))
	{
		TriObject *tobj = (TriObject*)os->obj;
		ApplyMapping(tobj->GetMesh(),mapChannel);
	}
	else if (os->obj->IsSubClassOf(polyObjectClassID))
	{
		PolyObject *pobj = (PolyObject*)os->obj;
		ApplyMapping(pobj->GetMesh(),mapChannel);
	}
#ifndef NO_PATCHES // orb 02-05-03
	else if (os->obj->IsSubClassOf(patchObjectClassID))
	{
		PatchObject *pobj = (PatchObject*)os->obj;
		ApplyMapping(pobj->patch,mapChannel);
	}
#endif
	else if (
#ifndef NO_PATCHES // orb 02-05-03
		(!os->obj->IsSubClassOf(patchObjectClassID)) && 
#endif // NO_PATCHES		
		(!os->obj->IsSubClassOf(triObjectClassID)) &&
		(!os->obj->IsSubClassOf(polyObjectClassID)) )
	{
		//neither patch or poly convert to a mesh
		if (os->obj->CanConvertToType(triObjectClassID))
		{
			TimeValue t = GetCOREInterface()->GetTime();
			TriObject *tri = (TriObject *) os->obj->ConvertToType(t, Class_ID(TRIOBJ_CLASS_ID, 0));
			os->obj = tri;
			os->obj->UnlockObject();			
			ApplyMapping(tri->GetMesh(), mapChannel);
		}
	}

}

void MeshTopoData::SetFaceSel(BitArray &set) 
{

	if (set.GetSize() != TVMaps.f.Count())
	{
		DbgAssert(0);
		return;
	}

	mFSel = set;
	if (mesh) mesh->faceSel = set;
	if (mnMesh) mnMesh->FaceSelect(set);
	if (patch) patch->patchSel = set;

/*	
	for (int i = 0; i < TVMaps.f.Count();i++)
	{
		if (!(TVMaps.f[i]->flags & FLAG_DEAD))
		{
			if (set[i])
			{
				TVMaps.f[i]->flags |= FLAG_SELECTED;
			}
			else TVMaps.f[i]->flags &= (~FLAG_SELECTED);
		}
	}
*/
}



int MeshTopoData::GetNumberGeomVerts()
{
	return TVMaps.geomPoints.Count();
}
void MeshTopoData::SetNumberGeomVerts(int ct)
{
	TVMaps.geomPoints.SetCount(ct);
}
Point3 MeshTopoData::GetGeomVert(int index)
{
	if ((index < 0) || (index >= TVMaps.geomPoints.Count()))
	{
		DbgAssert(0);
		return Point3(0.0f,0.0f,0.0f);
	}
	else 
		return TVMaps.geomPoints[index];

	return Point3(0.0f,0.0f,0.0f);
}
void MeshTopoData::SetGeomVert(int index, Point3 p)
{
	if ((index < 0) || (index >= TVMaps.geomPoints.Count()))
	{
		DbgAssert(0);
		return;
	}
	else TVMaps.geomPoints[index] = p;
}

int MeshTopoData::GetNumberFaces()
{
	return TVMaps.f.Count();
}

int MeshTopoData::GetNumberTVVerts()
{
	return TVMaps.v.Count();
}

Point3 MeshTopoData::GetTVVert(int index)
{
	if ((index < 0) || (index >= TVMaps.v.Count()))
	{
		DbgAssert(0);
		return Point3(0.0f,0.0f,0.0f);
	}
	else return TVMaps.v[index].GetP();

	return Point3(0.0f,0.0f,0.0f);
}

void MeshTopoData::SetTVVert(TimeValue t,int index, Point3 p, UnwrapMod *mod)
{
	if ((index < 0) || (index >= TVMaps.v.Count()))
	{
		DbgAssert(0);
		return;
	}
	else
	{
		TVMaps.v[index].SetP(p);
		//now see if we need to create controls for this
		int controlIndex = TVMaps.v[index].GetControlID();

		//first check if there is a control on it and if so set the value for
		if ((controlIndex != -1) && (controlIndex < mod->ControllerCount()) && 
			 mod->Controller(controlIndex)
			)
		{
			mod->Controller(controlIndex)->SetValue(t,p);
		}
		//next check if we are animating and no control
		else if (Animating())
		{
			//create a new control
			controlIndex =  mod->ControllerAdd();
			//remember its id
			TVMaps.v[index].SetControlID(controlIndex);
			//set its value
			mod->Controller(controlIndex)->SetValue(t,p);
			mod->Controller(controlIndex)->SetValue(0,p);
		}				 		
	}
}


int MeshTopoData::GetFaceDegree(int faceIndex)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()))
	{
		DbgAssert(0);
		return 0;
	}
	else return TVMaps.f[faceIndex]->count;
	return 0;
}



BitArray MeshTopoData::GetTVVertSelection()
{
	return mVSel;
}

BitArray *MeshTopoData::GetTVVertSelectionPtr()
{
	return &mVSel;
}


void MeshTopoData::SetTVVertSelection(BitArray newSel)
{
	if (newSel.GetSize() != GetNumberTVVerts())
	{
		DbgAssert(0);
		return;
	}

	mVSel = newSel;
	for (int i = 0; i < GetNumberTVVerts(); i++)
	{
		SetTVVertSelected(i,newSel[i]);
	}

}

float MeshTopoData::GetTVVertInfluence(int index)
{
	if ((index < 0) || (index >= TVMaps.v.Count()))
	{
		DbgAssert(0);
		return 0.0f;
	}
	else return TVMaps.v[index].GetInfluence();
	return 0.0f;
}

void MeshTopoData::SetTVVertInfluence(int index, float influ)
{
	if ((index < 0) || (index >= TVMaps.v.Count()))
	{
		DbgAssert(0);
		return;
	}
	else TVMaps.v[index].SetInfluence(influ);
}

BOOL MeshTopoData::GetTVVertSelected(int index)
{
	if ((index < 0) || (index >= TVMaps.v.Count()))
	{
		DbgAssert(0);
		return FALSE;
	}
	else return mVSel[index];

	return FALSE;
}
void MeshTopoData::SetTVVertSelected(int index, BOOL sel)
{
	if ((index < 0) || (index >= TVMaps.v.Count()))
	{
		DbgAssert(0);
		return;
	}
	else
	{
		if (sel && (index < TVMaps.mSystemLockedFlag.GetSize()) && TVMaps.mSystemLockedFlag[index])
			return;

		if (mVSel.GetSize() != TVMaps.v.Count())
		{
			DbgAssert(0);
			mVSel.SetSize(TVMaps.v.Count());
			mVSel.ClearAll();
		}

		mVSel.Set(index,sel);

	}
}

BOOL MeshTopoData::GetTVVertFrozen(int index)
{
	if ((index < 0) || (index >= TVMaps.v.Count()))
	{
		DbgAssert(0);
		return FALSE;
	}
	else
		return TVMaps.v[index].IsFrozen(); 
	return FALSE;

}
void MeshTopoData::SetTVVertFrozen(int index, BOOL frozen)
{
	if ((index < 0) || (index >= TVMaps.v.Count()))
	{
		DbgAssert(0);
		return;
	}
	else
	{
		if (frozen)
			TVMaps.v[index].Freeze(); 
		else TVMaps.v[index].Unfreeze(); 
	}	
}


void MeshTopoData::SetTVVertControlIndex(int index, int id)
{
	if ((index < 0) || (index >= TVMaps.v.Count()))
	{
		DbgAssert(0);
		return;
	}
	else
	{
	
		TVMaps.v[index].SetControlID(id); 	
	}
}

BOOL MeshTopoData::GetTVVertHidden(int index)
{
	if ((index < 0) || (index >= TVMaps.v.Count()))
	{
		DbgAssert(0);
		return FALSE;
	}
	else
		return TVMaps.v[index].IsHidden(); 
	return FALSE;

}
void MeshTopoData::SetTVVertHidden(int index, BOOL hidden)
{
	if ((index < 0) || (index >= TVMaps.v.Count()))
	{
		DbgAssert(0);
		return;
	}
	else
	{
		if (hidden)
			TVMaps.v[index].Hide(); 
		else TVMaps.v[index].Show(); 
	}	
}


int MeshTopoData::GetNumberTVEdges()
{
	return TVMaps.ePtrList.Count();
}
int MeshTopoData::GetTVEdgeVert(int index, int whichEnd)
{
	if ((index < 0) || (index >= TVMaps.ePtrList.Count()))
	{
		DbgAssert(0);
		return -1;
	}
	else
	{
		if (whichEnd == 0)
			return TVMaps.ePtrList[index]->a;
		else return TVMaps.ePtrList[index]->b;
	}
	return -1;
}

int MeshTopoData::GetTVEdgeGeomVert(int index, int whichEnd)
{
	if ((index < 0) || (index >= TVMaps.ePtrList.Count()))
	{
		DbgAssert(0);
		return -1;
	}
	else
	{
		if (whichEnd == 0)
			return TVMaps.ePtrList[index]->ga;
		else return TVMaps.ePtrList[index]->gb;
	}
	return -1;
}
int MeshTopoData::GetTVEdgeVec(int edgeIndex, int whichEnd)
{
	if ((edgeIndex < 0) || (edgeIndex >= TVMaps.ePtrList.Count()))
	{
		DbgAssert(0);
		return -1;
	}
	else
	{
		if (whichEnd == 0)
			return TVMaps.ePtrList[edgeIndex]->avec;
		else return TVMaps.ePtrList[edgeIndex]->bvec;
	}
	return -1;
}

int MeshTopoData::GetTVEdgeNumberTVFaces(int index)
{
	if ((index < 0) || (index >= TVMaps.ePtrList.Count()))
	{
		DbgAssert(0);
		return 0;
	}
	else
		return TVMaps.ePtrList[index]->faceList.Count();
	return 0;
}
int MeshTopoData::GetTVEdgeConnectedTVFace(int index,int findex)
{
	if ((index < 0) || (index >= TVMaps.ePtrList.Count()))
	{
		DbgAssert(0);
		return -1;
	}
	else if ( (findex < 0) || (findex >= TVMaps.ePtrList[index]->faceList.Count()))
	{
		DbgAssert(0);
		return -1;
	}
	else
		return TVMaps.ePtrList[index]->faceList[findex];
	return -1;
}

BOOL MeshTopoData::GetTVEdgeHidden(int index)
{
	if ((index < 0) || (index >= TVMaps.ePtrList.Count()))
	{
		DbgAssert(0);
		return FALSE;
	}
	else return (TVMaps.ePtrList[index]->flags & FLAG_HIDDENEDGEA);
	return FALSE;
}


void MeshTopoData::InvertSelection(int mode)
{
	if (mode == TVVERTMODE)
	{
		mVSel = ~mVSel;
	}
	else if (mode == TVEDGEMODE)
	{
		mESel = ~mESel;
	}
	else if (mode == TVFACEMODE)
	{
		mFSel = ~mFSel;
	}
}
void MeshTopoData::AllSelection(int mode)
{
	if (mode == TVVERTMODE)
	{
		mVSel.SetAll();	
		mGVSel.SetAll();	
		for (int i=0; i<TVMaps.v.Count(); i++) 
		{
			if (!(TVMaps.v[i].GetFlag() & FLAG_WEIGHTMODIFIED))
				TVMaps.v[i].SetInfluence(0.0f);
		}

	}
	else if (mode == TVEDGEMODE)
	{
		mESel.SetAll();
		mGESel.SetAll();
	}
	else if (mode == TVFACEMODE)
	{
		mFSel.SetAll();
	}
}

void MeshTopoData::ClearSelection(int mode)
{
	if (mode == TVVERTMODE)
	{
		mVSel.ClearAll();	
		mGVSel.ClearAll();	
		for (int i=0; i<TVMaps.v.Count(); i++) 
		{
			if (!(TVMaps.v[i].GetFlag() & FLAG_WEIGHTMODIFIED))
				TVMaps.v[i].SetInfluence(0.0f);
		}

	}
	else if (mode == TVEDGEMODE)
	{
		mESel.ClearAll();
		mGESel.ClearAll();
	}
	else if (mode == TVFACEMODE)
	{
		mFSel.ClearAll();

	}
}

int MeshTopoData::TransformedPointsCount()
{
	return mTransformedPoints.Count();
}
void MeshTopoData::TransformedPointsSetCount(int ct)
{
	mTransformedPoints.SetCount(ct);
}
IPoint2 MeshTopoData::GetTransformedPoint(int i)
{
	//special case here we use -1 as a null vector so just return nothing
	if (i == -1)
		return IPoint2(0,0);

	if ((i < 0) || (i >= mTransformedPoints.Count()))
	{
		DbgAssert(0);
		return IPoint2(0,0);
	}
	return mTransformedPoints[i];
}
void MeshTopoData::SetTransformedPoint(int i, IPoint2 pt)
{
	if ((i < 0) || (i >= mTransformedPoints.Count()))
	{
		DbgAssert(0);
	}
	mTransformedPoints[i] = pt;
}


//this makes a copy of the UV channel to v
void MeshTopoData::CopyTVData(Tab<UVW_TVVertClass> &v)
{
	v = TVMaps.v;
}
void MeshTopoData::PasteTVData(Tab<UVW_TVVertClass> &v)
{
	TVMaps.v = v;
}

void MeshTopoData::UpdateControllers(TimeValue t, UnwrapMod *mod)
{
//loop through our tvs looking for ones with controlls on them
	for (int i = 0; i < TVMaps.v.Count(); i++)
	{
	//push the data off the control into our tv data
		int controllerIndex = TVMaps.v[i].GetControlID();
		Point3 p;
		if (controllerIndex != -1)
		{
			Control* cont = mod->Controller(controllerIndex);
			if (cont)
			{
				cont->GetValue(t,&p,FOREVER);
				TVMaps.v[i].SetP(p);
			}
		}
	}
}

BitArray MeshTopoData::GetTVEdgeSelection()
{
	return mESel;
}

BitArray *MeshTopoData::GetTVEdgeSelectionPtr()
{
	return &mESel;
}

void MeshTopoData::SetTVEdgeSelection(BitArray newSel)
{
	mESel = newSel;
}
BOOL MeshTopoData::GetTVEdgeSelected(int index)
{
	if ((index < 0) || (index>=mESel.GetSize()))
	{
		DbgAssert(0);
		return FALSE;
	}
	return mESel[index];
}
void MeshTopoData::SetTVEdgeSelected(int index, BOOL sel)
{
	if ((index < 0) || (index>=mESel.GetSize()))
	{
		DbgAssert(0);
		return;
	}
	mESel.Set(index,sel);
}

int MeshTopoData::EdgeIntersect(Point3 p, float threshold, int i1, int i2)
{
	return TVMaps.EdgeIntersect( p, threshold, i1, i2);
}


	//Converts the edge selection into a vertex selection set
void    MeshTopoData::GetVertSelFromEdge(BitArray &sel)
{
	sel.SetSize(TVMaps.v.Count());
	sel.ClearAll();
	for (int i = 0; i < TVMaps.ePtrList.Count(); i++)
	{
		if (mESel[i])
		{
			int index = TVMaps.ePtrList[i]->a;
			if ((index >= 0) && (index < sel.GetSize()))
				sel.Set(index);

			index = TVMaps.ePtrList[i]->b;
			if ((index >= 0) && (index < sel.GetSize()))
				sel.Set(index);

			index = TVMaps.ePtrList[i]->avec;
			if ((index >= 0) && (index < sel.GetSize()))
				sel.Set(index);

			index = TVMaps.ePtrList[i]->bvec;
			if ((index >= 0) && (index < sel.GetSize()))
				sel.Set(index);


		}

	}
}
	//Converts the vertex selection into a edge selection set
	//PartialSelect determines whether all the vertices of a edge need to be selected for that edge to be selected
void    MeshTopoData::GetEdgeSelFromVert(BitArray &sel,BOOL partialSelect)
{
	sel.SetSize(TVMaps.ePtrList.Count());
	sel.ClearAll();
	for (int i =0; i < TVMaps.ePtrList.Count(); i++)
	{
		int a,b;
		a = TVMaps.ePtrList[i]->a;
		b = TVMaps.ePtrList[i]->b;
		if (partialSelect)
		{
			if (mVSel[a] || mVSel[b])
				sel.Set(i);
		}
		else
		{
			if (mVSel[a] && mVSel[b])
				sel.Set(i);
		}
	}
}

void    MeshTopoData::GetGeomVertSelFromFace(BitArray &sel)
{	
	sel.SetSize(TVMaps.geomPoints.Count());
	sel.ClearAll();
	for (int i = 0; i < TVMaps.f.Count(); i++)
	{
		if (mFSel[i])
		{
			int pcount = 3;
			pcount = TVMaps.f[i]->count;
			for (int k = 0; k < pcount; k++)
			{
				int index = TVMaps.f[i]->v[k];
				sel.Set(index);
				if ((TVMaps.f[i]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[i]->vecs))
				{
					index = TVMaps.f[i]->vecs->vhandles[k*2];
					if ((index >= 0) && (index < sel.GetSize()))
						sel.Set(index);

					index = TVMaps.f[i]->vecs->vhandles[k*2+1];
					if ((index >= 0) && (index < sel.GetSize()))
						sel.Set(index);

					if (TVMaps.f[i]->flags & FLAG_INTERIOR) 
					{
						index = TVMaps.f[i]->vecs->vinteriors[k];
						if ((index >= 0) && (index < sel.GetSize()))
							sel.Set(index);
					}
				}
			}
		}
	}
}

//Converts the face selection into a vertex selection set
void    MeshTopoData::GetVertSelFromFace(BitArray &sel)		//Converts the vertex selection into a face selection set
{
	sel.SetSize(TVMaps.v.Count());
	sel.ClearAll();
	for (int i = 0; i < TVMaps.f.Count(); i++)
	{
		if (mFSel[i])
		{
			int pcount = 3;
			pcount = TVMaps.f[i]->count;
			for (int k = 0; k < pcount; k++)
			{
				int index = TVMaps.f[i]->t[k];
				sel.Set(index);
				if ((TVMaps.f[i]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[i]->vecs))
				{
					index = TVMaps.f[i]->vecs->handles[k*2];
					if ((index >= 0) && (index < sel.GetSize()))
						sel.Set(index);

					index = TVMaps.f[i]->vecs->handles[k*2+1];
					if ((index >= 0) && (index < sel.GetSize()))
						sel.Set(index);

					if (TVMaps.f[i]->flags & FLAG_INTERIOR) 
					{
						index = TVMaps.f[i]->vecs->interiors[k];
						if ((index >= 0) && (index < sel.GetSize()))
							sel.Set(index);
					}
				}
			}
		}
	}
}
//PartialSelect determines whether all the vertices of a face need to be selected for that face to be selected
void    MeshTopoData::GetFaceSelFromVert(BitArray &sel,BOOL partialSelect)
{

	sel.SetSize(TVMaps.f.Count());
	sel.ClearAll();	
	for (int i = 0; i < TVMaps.f.Count(); i++)
	{
		int pcount = 3;
		pcount = TVMaps.f[i]->count;
		int total = 0;
		for (int k = 0; k < pcount; k++)
		{
			int index = TVMaps.f[i]->t[k];
			if (mVSel[index]) total++;
			if ((TVMaps.f[i]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[i]->vecs))
			{
				index = TVMaps.f[i]->vecs->handles[k*2];
				if ((index >= 0) && (index < sel.GetSize()))
				{
					if (mVSel[index]) total++;
				}

				index = TVMaps.f[i]->vecs->handles[k*2+1];
				if ((index >= 0) && (index < sel.GetSize()))
				{
					if (mVSel[index]) total++;
				}

				if (TVMaps.f[i]->flags & FLAG_INTERIOR) 
				{
					index = TVMaps.f[i]->vecs->interiors[k];
					if ((index >= 0) && (index < sel.GetSize()))
					{
						if (mVSel[index]) 
							total++;
					}
				}

			}
		}
		if ((partialSelect) && (total))
			sel.Set(i);
		else if ((!partialSelect) && (total >= pcount))
			sel.Set(i);

	}
}

void MeshTopoData::TransferSelectionStart(int subMode)
{
	originalSel.SetSize(mVSel.GetSize());
	originalSel = mVSel;

	holdFSel.SetSize(mFSel.GetSize());
	holdFSel = mFSel;

	holdESel.SetSize(mESel.GetSize());
	holdESel = mESel;


	if (subMode == TVVERTMODE)
	{
	}
	else if (subMode == TVEDGEMODE)
	{
		GetVertSelFromEdge(mVSel);
	}
	else if (subMode == TVFACEMODE)
	{
		GetVertSelFromFace(mVSel);
	}
}

void MeshTopoData::TransferSelectionEnd(int subMode, BOOL partial,BOOL recomputeSelection)
{
	if (subMode == TVVERTMODE)
	{
		//is vertex selection do not need to do anything
	}
	else if (subMode == TVEDGEMODE) //face mode
	{
		//need to convert our vertex selection to face
		if ( (recomputeSelection)  || (holdESel.GetSize() != TVMaps.ePtrList.Count()))
			GetEdgeSelFromVert(mESel,partial);
		else mESel = holdESel;
		//now we need to restore the vertex selection
		if (mVSel.GetSize() == originalSel.GetSize())
			mVSel = originalSel;
	}
	else if (subMode == TVFACEMODE) //face mode
	{
		//need to convert our vertex selection to face
		if (recomputeSelection) 
			GetFaceSelFromVert(mFSel,partial);
		else mFSel = holdFSel;
		//now we need to restore the vertex selection as long as we have not changed topo
		if (mVSel.GetSize() == originalSel.GetSize())
			mVSel = originalSel;
	}
}


BOOL MeshTopoData::GetFaceSelected(int faceIndex)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()) || (faceIndex >= mFSel.GetSize()))
	{
		DbgAssert(0);
		return FALSE;
	}

	return mFSel[faceIndex];
}
void MeshTopoData::SetFaceSelected(int faceIndex, BOOL sel)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()) || (faceIndex >= mFSel.GetSize()))
	{
		DbgAssert(0);
		return;
	}

	mFSel.Set(faceIndex,sel);
}

BOOL MeshTopoData::GetFaceCurvedMaping(int faceIndex)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()))
	{
		DbgAssert(0);
		return FALSE;
	}
	if (TVMaps.f[faceIndex]->flags & FLAG_CURVEDMAPPING)
		return TRUE;
	else return FALSE;
}

BOOL MeshTopoData::GetFaceFrozen(int faceIndex)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()))
	{
		DbgAssert(0);
		return FALSE;
	}
	return TVMaps.f[faceIndex]->flags & FLAG_FROZEN;

}
void MeshTopoData::SetFaceFrozen(int faceIndex, BOOL frozen)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()))
	{
		DbgAssert(0);
		return ;
	}
	
	if (frozen)
		TVMaps.f[faceIndex]->flags |= FLAG_FROZEN;
	else TVMaps.f[faceIndex]->flags &= ~FLAG_FROZEN;

}

BOOL MeshTopoData::GetFaceHasVectors(int faceIndex)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()))
	{
		DbgAssert(0);
		return FALSE;
	}
	if (TVMaps.f[faceIndex]->vecs)
		return TRUE;
	else return FALSE;

}

BOOL MeshTopoData::GetFaceHasInteriors(int faceIndex)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()))
	{
		DbgAssert(0);
		return FALSE;
	}
	if (!TVMaps.f[faceIndex]->vecs)
		return FALSE;

	return TVMaps.f[faceIndex]->flags & FLAG_INTERIOR;

}


int MeshTopoData::GetFaceTVVert(int faceIndex, int vertexID)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()))
	{
		DbgAssert(0);
		return -1;
	}
	if ((vertexID < 0) || (vertexID >= TVMaps.f[faceIndex]->count))
	{
		DbgAssert(0);
		return -1;
	}

	return TVMaps.f[faceIndex]->t[vertexID];
}



void MeshTopoData::SetFaceTVVert(int faceIndex, int vertexID, int id)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()))
	{
		DbgAssert(0);
		return;
	}
	if ((vertexID < 0) || (vertexID >= TVMaps.f[faceIndex]->count))
	{
		DbgAssert(0);
		return;
	}

	TVMaps.f[faceIndex]->t[vertexID] = id;

}

int MeshTopoData::GetFaceTVHandle(int faceIndex, int handleID)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()))
	{
		DbgAssert(0);
		return -1;
	}
	if (TVMaps.f[faceIndex]->vecs == NULL)
	{
		DbgAssert(0);
		return -1;
	}

	return TVMaps.f[faceIndex]->vecs->handles[handleID];
}

void MeshTopoData::SetFaceTVHandle(int faceIndex, int handleID, int id)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()))
	{
		DbgAssert(0);
		return;
	}
	if (TVMaps.f[faceIndex]->vecs == NULL)
	{
		DbgAssert(0);
		return;
	}

	TVMaps.f[faceIndex]->vecs->handles[handleID] = id;
}

int MeshTopoData::GetFaceTVInterior(int faceIndex, int handleID)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()))
	{
		DbgAssert(0);
		return -1;
	}
	if (TVMaps.f[faceIndex]->vecs == NULL)
	{
		DbgAssert(0);
		return -1;
	}

	return TVMaps.f[faceIndex]->vecs->interiors[handleID];
}

void MeshTopoData::SetFaceTVInterior(int faceIndex, int handleID, int id)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()))
	{
		DbgAssert(0);
		return;
	}
	if (TVMaps.f[faceIndex]->vecs == NULL)
	{
		DbgAssert(0);
		return;
	}

	TVMaps.f[faceIndex]->vecs->interiors[handleID] = id;
}

BitArray MeshTopoData::GetFaceSelection()
{
	return mFSel;
}

BitArray *MeshTopoData::GetFaceSelectionPtr()
{
	return &mFSel;
}


void MeshTopoData::SetFaceSelection(BitArray sel)
{
	mFSel = sel;
}


int MeshTopoData::PolyIntersect(Point3 p1, int i1, int i2, BitArray *ignoredFaces )
{

	static int startFace = 0;
	int ct = 0;
	Point3 p2 = p1;
	float x = FLT_MIN;
	for (int i =0; i < TVMaps.v.Count(); i++)
	{
		if (!(TVMaps.v[i].IsDead()) && !TVMaps.mSystemLockedFlag[i])
		{
			float tx = TVMaps.v[i].GetP()[i1];
			if (tx > x) x = tx;
		}
	}
	p2.x = x+10.0f;

	if (startFace >= TVMaps.f.Count()) startFace = 0;


	while (ct != TVMaps.f.Count())
	{
		int pcount = TVMaps.f[startFace]->count;
		int hit = 0;
		BOOL bail = FALSE;
		if (ignoredFaces)
		{
			if ((*ignoredFaces)[startFace])
				bail = TRUE;
		}
		if ( IsFaceVisible(startFace) && (!bail))
		{
			int frozen = 0, hidden = 0;
			for (int j=0; j<pcount; j++) 
			{
				int index = TVMaps.f[startFace]->t[j];
				if (TVMaps.v[index].IsHidden()) hidden++;
				if (TVMaps.v[index].IsFrozen()) frozen++;
			}
			if ((frozen == pcount) || (hidden == pcount))
			{
			}	
			else if ( (patch != NULL) && (!(TVMaps.f[startFace]->flags & FLAG_CURVEDMAPPING)) && (TVMaps.f[startFace]->vecs))
			{
				Spline3D spl;
				spl.NewSpline();
				int i = startFace;
				for (int j=0; j<pcount; j++) 
				{
					Point3 in, p, out;
					int index = TVMaps.f[i]->t[j];
					p = GetTVVert(index);

					index = TVMaps.f[i]->vecs->handles[j*2];
					out = GetTVVert(index);
					if (j==0)
						index = TVMaps.f[i]->vecs->handles[pcount*2-1];
					else index = TVMaps.f[i]->vecs->handles[j*2-1];

					in = GetTVVert(index);

					SplineKnot kn(KTYPE_BEZIER_CORNER, LTYPE_CURVE, p, in, out);
					spl.AddKnot(kn);

					spl.SetClosed();
					spl.ComputeBezPoints();
				}
				//draw curves
				Point3 ptList[7*4];
				int ct = 0;
				for (int j=0; j<pcount; j++) 
				{
					int jNext = j+1;
					if (jNext >= pcount) jNext = 0;
					Point3 p;
					int index = TVMaps.f[i]->t[j];
					if (j==0)
						ptList[ct++] = GetTVVert(index);

					for (int iu = 1; iu <= 5; iu++)
					{
						float u = (float) iu/5.f;
						ptList[ct++] = spl.InterpBezier3D(j, u);

					}



				}
				for (int j=0; j < ct; j++) 
				{
					int index;
					if (j == (ct-1))
						index = 0;
					else index = j+1;
					Point3 a(0.0f,0.0f,0.0f),b(0.0f,0.0f,0.0f);
					a.x = ptList[j][i1];
					a.y = ptList[j][i2];

					b.x = ptList[index][i1];
					b.y = ptList[index][i2];

					if (LineIntersect(p1, p2, a,b)) hit++;
					//					if (LineIntersect(p1, p2, ptList[j],ptList[index])) hit++;
				}


			}
			else
			{
				for (int i = 0; i < pcount; i++)
				{
					int faceIndexA;  
					faceIndexA = TVMaps.f[startFace]->t[i];
					int faceIndexB;
					if (i == (pcount-1))
						faceIndexB  = TVMaps.f[startFace]->t[0];
					else faceIndexB  = TVMaps.f[startFace]->t[i+1];
					Point3 a(0.0f,0.0f,0.0f),b(0.0f,0.0f,0.0f);
					a.x = TVMaps.v[faceIndexA].GetP()[i1];
					a.y = TVMaps.v[faceIndexA].GetP()[i2];
					b.x = TVMaps.v[faceIndexB].GetP()[i1];
					b.y = TVMaps.v[faceIndexB].GetP()[i2];
					if (LineIntersect(p1, p2, a,b)) 
						hit++;
				}
			}


			if ((hit%2) == 1) 
				return startFace;
		}
		startFace++;
		if (startFace >= TVMaps.f.Count()) startFace = 0;
		ct++;
	}
	return -1;
	
}

BOOL MeshTopoData::LineIntersect(Point3 p1, Point3 p2, Point3 q1, Point3 q2)
{


	float a, b, c, d, det;  /* parameter calculation variables */
	float max1, max2, min1, min2; /* bounding box check variables */

	/*  First make the bounding box test. */
	max1 = maxmin(p1.x, p2.x, min1);
	max2 = maxmin(q1.x, q2.x, min2);
	if((max1 < min2) || (min1 > max2)) return(FALSE); /* no intersection */
	max1 = maxmin(p1.y, p2.y, min1);
	max2 = maxmin(q1.y, q2.y, min2);
	if((max1 < min2) || (min1 > max2)) return(FALSE); /* no intersection */

	/* See if the endpoints of the second segment lie on the opposite
	sides of the first.  If not, return 0. */
	a = (q1.x - p1.x) * (p2.y - p1.y) -
		(q1.y - p1.y) * (p2.x - p1.x);
	b = (q2.x - p1.x) * (p2.y - p1.y) -
		(q2.y - p1.y) * (p2.x - p1.x);
	if(a!=0.0f && b!=0.0f && SAME_SIGNS(a, b)) return(FALSE);

	/* See if the endpoints of the first segment lie on the opposite
	sides of the second.  If not, return 0.  */
	c = (p1.x - q1.x) * (q2.y - q1.y) -
		(p1.y - q1.y) * (q2.x - q1.x);
	d = (p2.x - q1.x) * (q2.y - q1.y) -
		(p2.y - q1.y) * (q2.x - q1.x);
	if(c!=0.0f && d!=0.0f && SAME_SIGNS(c, d) ) return(FALSE);

	/* At this point each segment meets the line of the other. */
	det = a - b;
	if(det == 0.0f) return(FALSE); /* The segments are colinear.  Determining */
	return(TRUE);
}


BitArray MeshTopoData::GetGeomVertSelection()
{
	return mGVSel;
}

BitArray *MeshTopoData::GetGeomVertSelectionPtr()
{
	return &mGVSel;
}


void MeshTopoData::SetGeomVertSelection(BitArray newSel)
{
	mGVSel = newSel;
}

BOOL MeshTopoData::GetGeomVertSelected(int index)
{
	if ((index < 0) || (index >= TVMaps.geomPoints.Count()))
	{
		DbgAssert(0);
		return FALSE;
	}
	if (mGVSel.GetSize() != TVMaps.geomPoints.Count())
	{
		DbgAssert(0);
		mGVSel.SetSize(TVMaps.geomPoints.Count());
		mGVSel.ClearAll();
		return FALSE;
	}

	return mGVSel[index];
	
}
void MeshTopoData::SetGeomVertSelected(int index, BOOL sel)
{
	if ((index < 0) || (index >= TVMaps.geomPoints.Count()))
	{
		DbgAssert(0);
		return ;
	}
	if (mGVSel.GetSize() != TVMaps.geomPoints.Count())
	{
		DbgAssert(0);
		mGVSel.SetSize(TVMaps.geomPoints.Count());
		mGVSel.ClearAll();
		return ;
	}

	mGVSel.Set(index,sel);
}


int MeshTopoData::GetNumberGeomEdges()
{
	return TVMaps.gePtrList.Count();
}
BitArray *MeshTopoData::GetGeomEdgeSelectionPtr()
{
	return &mGESel;
}
BitArray MeshTopoData::GetGeomEdgeSelection()
{
	return mGESel;
}
void MeshTopoData::SetGeomEdgeSelection(BitArray newSel)
{
	if (newSel.GetSize() != TVMaps.gePtrList.Count())
	{
		DbgAssert(0);
		mGESel = newSel;
		return;
	}
	mGESel = newSel;
}
BOOL MeshTopoData::GetGeomEdgeSelected(int index)
{
	if ((index < 0) || (index >= mGESel.GetSize()))
	{
		DbgAssert(0);
		return FALSE;
	}
	return mGESel[index];
}
void MeshTopoData::SetGeomEdgeSelected(int index, BOOL sel)
{
	if ((index < 0) || (index >= mGESel.GetSize()))
	{
		DbgAssert(0);
		return;
	}
	mGESel.Set(index,sel);
}


int MeshTopoData::GetGeomEdgeVert(int edgeIndex, int whichEnd)
{
	if ((edgeIndex < 0) || (edgeIndex >= TVMaps.gePtrList.Count()))
	{
		DbgAssert(0);
		return -1;
	}
	if (whichEnd == 0)
		return TVMaps.gePtrList[edgeIndex]->a;
	else 
		return TVMaps.gePtrList[edgeIndex]->b;
}

int MeshTopoData::GetGeomEdgeNumberOfConnectedFaces(int index)
{
	if ((index < 0) || (index >= TVMaps.gePtrList.Count()))
	{
		DbgAssert(0);
		return 0;
	}
	else
		return TVMaps.gePtrList[index]->faceList.Count();
	return 0;
}
int MeshTopoData::GetGeomEdgeConnectedFace(int index,int findex)
{
	if ((index < 0) || (index >= TVMaps.gePtrList.Count()))
	{
		DbgAssert(0);
		return -1;
	}
	else if ( (findex < 0) || (findex >= TVMaps.gePtrList[index]->faceList.Count()))
	{
		DbgAssert(0);
		return -1;
	}
	else
		return TVMaps.gePtrList[index]->faceList[findex];
	return -1;
}


BOOL MeshTopoData::GetGeomEdgeHidden(int index)
{
	if ((index < 0) || (index >= TVMaps.gePtrList.Count()))
	{
		DbgAssert(0);
		return FALSE;
	}
	return TVMaps.gePtrList[index]->flags & FLAG_HIDDENEDGEA;
}

int MeshTopoData::GetGeomEdgeVec(int edgeIndex, int whichEnd)
{
	if ((edgeIndex < 0) || (edgeIndex >= TVMaps.gePtrList.Count()) )
	{
		DbgAssert(0);
		return -1;
	}
	if (whichEnd == 0)
		return TVMaps.gePtrList[edgeIndex]->avec;
	else 
		return TVMaps.gePtrList[edgeIndex]->bvec;
}/*
int MeshTopoData::GetFaceTVVertIndex(int faceIndex, int ithVert)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()))
	{
		DbgAssert(0);
		return -1;
	}
	if ((ithVert < 0) || (ithVert >= TVMaps.f[faceIndex]->count))
	{
		DbgAssert(0);
		return -1;
	}

	return  TVMaps.f[faceIndex]->t[ithVert];
}
*/
int MeshTopoData::GetFaceGeomVert(int faceIndex, int ithVert)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()))
	{
		DbgAssert(0);
		return -1;
	}
	if ((ithVert < 0) || (ithVert >= TVMaps.f[faceIndex]->count))
	{
		DbgAssert(0);
		return -1;
	}

	return  TVMaps.f[faceIndex]->v[ithVert];

}

void MeshTopoData::ClearTVVertSelection()
{
	mVSel.ClearAll();
}

void MeshTopoData::ClearGeomVertSelection()
{
	mGVSel.ClearAll();
}

void MeshTopoData::ClearTVEdgeSelection()
{
	mESel.ClearAll();
}

void MeshTopoData::ClearGeomEdgeSelection()
{
	mGESel.ClearAll();
}


void MeshTopoData::ClearFaceSelection()
{
	mFSel.ClearAll();
}

int MeshTopoData::FindGeomEdge(int faceIndex,int a, int b)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()))
	{
		DbgAssert(0);
		return -1;
	}
	return TVMaps.f[faceIndex]->FindGeomEdge(a, b);
}

int MeshTopoData::FindUVEdge(int faceIndex,int a, int b)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()))
	{
		DbgAssert(0);
		return -1;
	}
	return TVMaps.f[faceIndex]->FindUVEdge(a, b);
}


ULONG MeshTopoData::SaveFaces(ISave *isave)
{
	return TVMaps.SaveFaces(isave);
}

ULONG MeshTopoData::SaveTVVerts(ISave *isave)
{
	int vct = TVMaps.v.Count();
	ULONG nb = 0;
	isave->Write(&vct, sizeof(vct), &nb);
	return isave->Write(TVMaps.v.Addr(0), sizeof(UVW_TVVertClass)*vct, &nb);
}

ULONG MeshTopoData::SaveGeoVerts(ISave *isave)
{
	int vct = TVMaps.geomPoints.Count();
	ULONG nb = 0;
	isave->Write(&vct, sizeof(vct), &nb);
	return isave->Write(TVMaps.geomPoints.Addr(0), sizeof(Point3)*vct, &nb);
}


ULONG MeshTopoData::LoadFaces(ILoad *iload)
{
	ULONG iret = TVMaps.LoadFaces(iload);
	mFSel.SetSize(TVMaps.f.Count());
	mFSel.ClearAll();
	return iret;
}
ULONG MeshTopoData::LoadTVVerts(ILoad *iload)
{
	ULONG nb = 0;
	int ct = 0;
	iload->Read(&ct, sizeof(ct), &nb);
	TVMaps.v.SetCount(ct);

	mVSel.SetSize(TVMaps.v.Count());
	mVSel.ClearAll();

	return iload->Read(TVMaps.v.Addr(0), sizeof(UVW_TVVertClass)*TVMaps.v.Count(), &nb);
}
ULONG MeshTopoData::LoadGeoVerts(ILoad *iload)
{
	ULONG nb = 0;
	int ct = 0;
	iload->Read(&ct, sizeof(ct), &nb);
	TVMaps.geomPoints.SetCount(ct);
	return iload->Read(TVMaps.geomPoints.Addr(0), sizeof(Point3)*TVMaps.geomPoints.Count(), &nb);
}

void MeshTopoData::SetTVEdgeInvalid()
{
	TVMaps.edgesValid = FALSE;	
}

void MeshTopoData::SetGeoEdgeInvalid()
{
	TVMaps.mGeoEdgesValid = FALSE;
}



ULONG MeshTopoData::LoadTVVertSelection(ILoad *iload)
{
	return mVSel.Load(iload);
}
ULONG MeshTopoData::LoadGeoVertSelection(ILoad *iload)
{
	return mGVSel.Load(iload);
}
ULONG MeshTopoData::LoadTVEdgeSelection(ILoad *iload)
{
	return mESel.Load(iload);
}
ULONG MeshTopoData::LoadGeoEdgeSelection(ILoad *iload)
{
	return mGESel.Load(iload);
}
ULONG MeshTopoData::LoadFaceSelection(ILoad *iload)
{
	return mFSel.Load(iload);
}

ULONG MeshTopoData::SaveTVVertSelection(ISave *isave)
{
	return mVSel.Save(isave);
}
ULONG MeshTopoData::SaveGeoVertSelection(ISave *isave)
{
	return mGVSel.Save(isave);
}
ULONG MeshTopoData::SaveTVEdgeSelection(ISave *isave)
{
	return mESel.Save(isave);
}
ULONG MeshTopoData::SaveGeoEdgeSelection(ISave *isave)
{
	return mGESel.Save(isave);
}
ULONG MeshTopoData::SaveFaceSelection(ISave *isave)
{
	return mFSel.Save(isave);
}

void MeshTopoData::ResolveOldData(UVW_ChannelClass &oldData)
{
//see if we have matching face data if not bail
	TVMaps.ResolveOldData(oldData);
}

void MeshTopoData::SaveOldData(ISave *isave)
{
	ULONG nb;

	int vct = GetNumberTVVerts();//TVMaps.v.Count(), 
	int fct = GetNumberFaces();//TVMaps.f.Count();

	isave->BeginChunk(VERTCOUNT_CHUNK);
	isave->Write(&vct, sizeof(vct), &nb);
	isave->EndChunk();

	if (vct) {
		isave->BeginChunk(VERTS2_CHUNK);
		//FIX
		Tab<UVW_TVVertClass_Max9> oldTVData;
		oldTVData.SetCount(vct);
		for (int i = 0; i < vct; i++)
		{
			oldTVData[i].p = TVMaps.v[i].GetP();
			oldTVData[i].flags = TVMaps.v[i].GetFlag();
			oldTVData[i].influence = TVMaps.v[i].GetInfluence();
		}

		isave->Write(oldTVData.Addr(0), sizeof(UVW_TVVertClass_Max9)*vct, &nb);
		isave->EndChunk();
	}

	isave->BeginChunk(FACECOUNT_CHUNK);
	isave->Write(&fct, sizeof(fct), &nb);
	isave->EndChunk();

	if (fct) {
		isave->BeginChunk(FACE4_CHUNK);
		TVMaps.SaveFacesMax9(isave);
		isave->EndChunk();
	}

	isave->BeginChunk(VERTSEL_CHUNK);
	mVSel.Save(isave);
	isave->EndChunk();

	fct = TVMaps.geomPoints.Count();
	isave->BeginChunk(GEOMPOINTSCOUNT_CHUNK);
	isave->Write(&fct, sizeof(fct), &nb);
	isave->EndChunk();

	if (fct) {
		isave->BeginChunk(GEOMPOINTS_CHUNK);
		isave->Write(TVMaps.geomPoints.Addr(0), sizeof(Point3)*fct, &nb);
		isave->EndChunk();
	}


	isave->BeginChunk(FACESELECTION_CHUNK);
	mFSel.Save(isave);
	isave->EndChunk();


	isave->BeginChunk(UEDGESELECTION_CHUNK);
	mESel.Save(isave);
	isave->EndChunk();

	isave->BeginChunk(GEDGESELECTION_CHUNK);
	mGESel.Save(isave);
	isave->EndChunk();
}


int MeshTopoData::GetFaceGeomHandle(int faceIndex, int ithVert)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()) )
	{
		DbgAssert(0);
		return -1;
	}
	return TVMaps.f[faceIndex]->vecs->vhandles[ithVert];
}

int MeshTopoData::GetFaceGeomInterior(int faceIndex, int ithVert)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()) )
	{
		DbgAssert(0);
		return -1;
	}
	return TVMaps.f[faceIndex]->vecs->vinteriors[ithVert];
}

int MeshTopoData::FindGeoEdge(int a, int b)
{
	return TVMaps.FindGeoEdge(a,b);
}

int  MeshTopoData::GetFaceMatID(int faceIndex)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()) )
	{
		DbgAssert(0);
		return -1;
	}
	return TVMaps.f[faceIndex]->MatID;
}



BOOL MeshTopoData::GetFaceDead(int faceIndex)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()) )
	{
		DbgAssert(0);
		return -1;
	}
	return TVMaps.f[faceIndex]->flags & FLAG_DEAD;
}

void MeshTopoData::SetFaceDead(int faceIndex, BOOL dead)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()) )
	{
		DbgAssert(0);
		return;
	}
	if (dead)
		TVMaps.f[faceIndex]->flags |= FLAG_DEAD;
	else TVMaps.f[faceIndex]->flags &= ~FLAG_DEAD;
}

BOOL MeshTopoData::GetFaceHidden(int faceIndex)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()))
	{
		DbgAssert(0);
		return FALSE;
	}
	int deg = TVMaps.f[faceIndex]->count;
	for (int i = 0; i < deg; i++)
	{
		int id = TVMaps.f[faceIndex]->t[i];
		
		if (!IsTVVertVisible(id))
			return TRUE;
	}
	return FALSE;
}


Point3 MeshTopoData::GetGeomFaceNormal(int faceIndex)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()) )
	{
		DbgAssert(0);
		return Point3(1.0f,0.0f,0.0f);
	}
	return TVMaps.GeomFaceNormal(faceIndex);
}
Point3 MeshTopoData::GetUVFaceNormal(int faceIndex)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()) )
	{
		DbgAssert(0);
		return Point3(1.0f,0.0f,0.0f);
	}
	
	return TVMaps.UVFaceNormal(faceIndex);
}

void	MeshTopoData::ConvertFaceToEdgeSel()
{

	mESel.ClearAll();
	for (int i = 0; i < TVMaps.ePtrList.Count();i++)
	{
		for (int j = 0; j < TVMaps.ePtrList[i]->faceList.Count();j++)
		{
			int index = TVMaps.ePtrList[i]->faceList[j];
			if (mFSel[index])
			{
				mESel.Set(i);
				j = TVMaps.ePtrList[i]->faceList.Count();
			}
		}

	}

}


int MeshTopoDataContainer::Count()
{
	return mMeshTopoDataNodeInfo.Count();
}
void MeshTopoDataContainer::SetCount(int ct)
{
//	mMeshTopoData.SetCount(ct);
	mMeshTopoDataNodeInfo.SetCount(ct);
}

void MeshTopoDataContainer::Append(int ct, MeshTopoData* ld, INode *node, int extraCT)
{
//	mMeshTopoData.Append(ct,ld,extraCT);
	MeshTopoDataNodeInfo t;
	t.mNode = node;
	t.mMeshTopoData = ld;
	mMeshTopoDataNodeInfo.Append(1,&t,10);

	
}

Point3 MeshTopoDataContainer::GetNodeColor(int index)
{
	if ((index < 0) || (index >= mMeshTopoDataNodeInfo.Count()))
	{
		DbgAssert(0);
		return Point3(0.5f,0.5f,0.5f);
	}
	DWORD clr = mMeshTopoDataNodeInfo[index].mNode->GetWireColor();
	return Point3((float)GetRValue(clr)/255.0f,(float)GetGValue(clr)/255.0f,(float)GetBValue(clr)/255.0f);

}

INode* MeshTopoDataContainer::GetNode(int index)
{
	if ((index < 0) || (index >= mMeshTopoDataNodeInfo.Count()))
	{
		DbgAssert(0);
		return NULL;
	}
	return mMeshTopoDataNodeInfo[index].mNode;
}

Matrix3 MeshTopoDataContainer::GetNodeTM(TimeValue t, int index)
{
	Matrix3 tm(1);
	if ((index < 0) || (index >= mMeshTopoDataNodeInfo.Count()) || (mMeshTopoDataNodeInfo[index].mNode == NULL))
	{
		DbgAssert(0);
		return tm;
	}

	tm = mMeshTopoDataNodeInfo[index].mNode->GetObjectTM(t);
	return tm;

}

int MeshTopoData::GetTVVertFlag(int index)
{
	if ((index < 0) || (index >= TVMaps.v.Count()))
	{
		DbgAssert(0);
		return 0;
	}
	return TVMaps.v[index].GetFlag();
}

void MeshTopoData::SetTVVertFlag(int index, int flag)
{
	if ((index < 0) || (index >= TVMaps.v.Count()))
	{
		DbgAssert(0);
		return;
	}
	return TVMaps.v[index].SetFlag(flag);
}


void MeshTopoData::SetTVVertDead(int index, BOOL dead)
{
	if ((index < 0) || (index >= TVMaps.v.Count()))
	{
		DbgAssert(0);
		return;
	}
	else if (dead)
		TVMaps.v[index].SetDead(); 
	else 
		TVMaps.v[index].SetAlive(); 

}
BOOL MeshTopoData::GetTVVertDead(int index)
{
	if ((index < 0) || (index >= TVMaps.v.Count()))
	{
		DbgAssert(0);
		return FALSE;
	}
	else
		return TVMaps.v[index].IsDead(); 
	return FALSE;
}

BOOL MeshTopoData::GetTVSystemLock(int index)
{
	if ((index < 0) || (index >= TVMaps.mSystemLockedFlag.GetSize()))
	{
		DbgAssert(0);
		return FALSE;
	}
	else
		return TVMaps.mSystemLockedFlag[index]; 
	return FALSE;
}

BOOL MeshTopoData::GetTVVertWeightModified(int index)
{
	if ((index < 0) || (index >= TVMaps.v.Count()))
	{
		DbgAssert(0);
		return FALSE;
	}
	else
		return TVMaps.v[index].IsWeightModified(); 
	return FALSE;

}
void MeshTopoData::SetTVVertWeightModified(int index, BOOL modified)
{
	if ((index < 0) || (index >= TVMaps.v.Count()))
	{
		DbgAssert(0);
		return;
	}
	else if (modified)
		TVMaps.v[index].SetWeightModified(); 
	else TVMaps.v[index].SetWeightUnModified(); 

}


int MeshTopoData::AddTVVert(TimeValue t,Point3 p, UnwrapMod *mod, Tab<int> *deadVertList)
{
	//loop through vertex list looking for dead ones else attache to end
	int found = -1;
	if (deadVertList)
	{
		if (deadVertList->Count())
		{
			int index =  (*deadVertList)[deadVertList->Count()-1];

			BOOL isSystemLockVert = FALSE;
			if (index < TVMaps.mSystemLockedFlag.GetSize())
				isSystemLockVert = TVMaps.mSystemLockedFlag[index];
			if (!isSystemLockVert)
			{			
				found = index;
			}
			deadVertList->Delete(deadVertList->Count()-1,1);
		}
	}
	else
	{
		for (int m = 0; m <GetNumberTVVerts();m++)//TVMaps.v.Count();m++)
		{
			BOOL isSystemLockVert = FALSE;
			if (m < TVMaps.mSystemLockedFlag.GetSize())
				isSystemLockVert = TVMaps.mSystemLockedFlag[m];
			if (GetTVVertDead(m) && !isSystemLockVert)//TVMaps.v[m].flags & FLAG_DEAD)
			{
				found =m;
				m = TVMaps.v.Count();
			}
		}
	}
	//found dead spot add to it
	if (found != -1)
	{
		SetTVVertControlIndex(found,-1);
		SetTVVert(t,found,p,mod);//TVMaps.v[found].p = p;
		SetTVVertInfluence(found,0.0f);//TVMaps.v[found].influence = 0.0f;
		SetTVVertDead(found,FALSE);//TVMaps.v[found].flags -= FLAG_DEAD;
		return found;
	}
	//create a new vert
	else
	{
		UVW_TVVertClass tv;
		tv.SetP(p);
		tv.SetFlag(0);
		tv.SetInfluence(0.0f);
		tv.SetControlID(-1);
		TVMaps.v.Append(1,&tv,1000);

		mVSel.SetSize(TVMaps.v.Count(), 1);
		TVMaps.mSystemLockedFlag.SetSize(TVMaps.v.Count(), 1);
		return TVMaps.v.Count()-1;
	}
	return 0;

}

int MeshTopoData::AddTVVert(TimeValue t, Point3 p, int faceIndex, int ithVert, UnwrapMod *mod,  BOOL sel, Tab<int> *deadVertList)
{
	if ((faceIndex < 0) || (faceIndex >= GetNumberFaces()))
	{
		DbgAssert(0);
		return -1;
	}

	int id = AddTVVert(t,p, mod,deadVertList);
	if (sel)
		mVSel.Set(id,TRUE);
	TVMaps.f[faceIndex]->t[ithVert] = id;
	return id;

}


int MeshTopoData::AddTVHandle(TimeValue t, Point3 p, int faceIndex, int ithVert, UnwrapMod *mod,  BOOL sel, Tab<int> *deadVertList)
{
	if ((faceIndex < 0) || (faceIndex >= GetNumberFaces()))
	{
		DbgAssert(0);
		return -1;
	}
	if (TVMaps.f[faceIndex]->vecs == NULL)
	{
		DbgAssert(0);
		return -1;
	}

	int id = AddTVVert(t,p, mod,deadVertList);
	if (sel)
		mVSel.Set(id,TRUE);
	TVMaps.f[faceIndex]->vecs->handles[ithVert] = id;
	return id;
}


int MeshTopoData::AddTVInterior(TimeValue t, Point3 p, int faceIndex, int ithVert, UnwrapMod *mod,  BOOL sel, Tab<int> *deadVertList)
{
	if ((faceIndex < 0) || (faceIndex >= GetNumberFaces()))
	{
		DbgAssert(0);
		return -1;
	}
	if (TVMaps.f[faceIndex]->vecs == NULL)
	{
		DbgAssert(0);
		return -1;
	}

	int id = AddTVVert(t,p, mod,deadVertList);
	if (sel)
		mVSel.Set(id,TRUE);
	TVMaps.f[faceIndex]->vecs->interiors[ithVert] = id;
	return id;
}

void MeshTopoData::DeleteTVVert(int index, UnwrapMod *mod)
{
	if ((index < 0) || (index >= GetNumberTVVerts()))
	{
		DbgAssert(0);
		return;			
	}
	TVMaps.v[index].SetDead();//flags |= FLAG_DEAD;
	int controlID = TVMaps.v[index].GetControlID();
	if (controlID >= 0)
		mod->DeleteReference(controlID);
}

BOOL MeshTopoData::IsTVVertVisible(int index)
{
	if ((index < 0) || (index >= GetNumberTVVerts()))
	{
		DbgAssert(0);
		return FALSE;			
	}


	if ((index < TVMaps.mSystemLockedFlag.GetSize()) && TVMaps.mSystemLockedFlag[index])
		return FALSE;

	if ( TVMaps.v[index].IsDead())
		return FALSE;

	if ( TVMaps.v[index].IsHidden())
		return FALSE;

//also need to check matid
	if (mVertMatIDFilterList.GetSize() != GetNumberTVVerts())
	{
		mVertMatIDFilterList.SetSize(GetNumberTVVerts());
		mVertMatIDFilterList.SetAll();
	}
	BOOL iret = FALSE;
	if (mVertMatIDFilterList[index])
		iret = TRUE;

//and then selected face
	if (mFilterFacesVerts.GetSize() != GetNumberTVVerts())
	{
		mFilterFacesVerts.SetSize(GetNumberTVVerts());
		mFilterFacesVerts.SetAll();
	}
	if (!mFilterFacesVerts[index])
		iret = FALSE;

	return iret;
/*
	if ( (!(TVMaps.v[i].IsDead())) && (!(TVMaps.v[i].flags & FLAG_HIDDEN)) )
	{
		if ((filterSelectedFaces==0) || (IsSelected(i) == 1))
		{
			if ((matid == -1)|| (i >= vertMatIDList.GetSize()))
				return 1;
			else 
			{
				if (vertMatIDList[i]) return 1;

			}
		}
	}
*/
}


BOOL MeshTopoData::IsFaceVisible(int index)
{
	if ((index < 0) || (index >= GetNumberFaces()))
	{
		DbgAssert(0);
		return FALSE;			
	}

	if (TVMaps.f[index]->flags & FLAG_DEAD) 
		return FALSE;

	return TRUE;
}

void MeshTopoData::BuilMatIDFilter(int matid)
{
	if (mVertMatIDFilterList.GetSize() != TVMaps.v.Count())
	{
		mVertMatIDFilterList.SetSize(TVMaps.v.Count());
		mVertMatIDFilterList.SetAll();
	}
	if (matid == -1)
		mVertMatIDFilterList.SetAll();
	else 
	{
		mVertMatIDFilterList.ClearAll();
		
		for (int j = 0; j < TVMaps.f.Count(); j++)
		{
			int faceMatID = TVMaps.f[j]->MatID;

			if (faceMatID == matid)
			{
				int pcount = 3;
				pcount = TVMaps.f[j]->count;
				for (int k = 0; k < pcount; k++)
				{
					int index = TVMaps.f[j]->t[k];
					//6-29-99 watje
					mVertMatIDFilterList.Set(index,TRUE);
					


					if ( (TVMaps.f[j]->flags & FLAG_CURVEDMAPPING) &&
						(TVMaps.f[j]->vecs) 
						)
					{
						index = TVMaps.f[j]->vecs->handles[k*2];
						mVertMatIDFilterList.Set(index,TRUE);

						index = TVMaps.f[j]->vecs->handles[k*2+1];
						mVertMatIDFilterList.Set(index,TRUE);

						index = TVMaps.f[j]->vecs->interiors[k];
						mVertMatIDFilterList.Set(index,TRUE);
					}
				}
			}
		}
		
	}
}


void MeshTopoData::BuilFilterSelectedFaces(BOOL filterSelFaces)
{
	mFilterFacesVerts.SetSize(TVMaps.v.Count());

	if (!filterSelFaces)
		mFilterFacesVerts.SetAll();
	else
	{
		mFilterFacesVerts.ClearAll();
		int sel = 0;
		

		for (int i = 0; i < TVMaps.f.Count();i++)
		{
			int pcount = 3;
			//	if (TVMaps.f[i].flags & FLAG_QUAD) pcount = 4;
			pcount = TVMaps.f[i]->count;
			for (int j = 0;  j < pcount;j++)
			{
				if (mFSel[i])
				{
					mFilterFacesVerts.Set(TVMaps.f[i]->t[j]);
					if ((TVMaps.f[i]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[i]->vecs))
					{
						mFilterFacesVerts.Set(TVMaps.f[i]->vecs->handles[j*2]);
						mFilterFacesVerts.Set(TVMaps.f[i]->vecs->handles[j*2+1]);
						if (TVMaps.f[i]->flags & FLAG_INTERIOR)
							mFilterFacesVerts.Set(TVMaps.f[i]->vecs->interiors[j]);
					}
				}

			}
		}
	
	}
}

int MeshTopoData::GetTVVertGeoIndex(int index)
{
	if (mVertexClusterList.Count() != TVMaps.v.Count())
		BuildVertexClusterList();
	if ((index < 0) || (index >= mVertexClusterList.Count() ))
	{
		DbgAssert(0);
		return -1;
	}
	return mVertexClusterList[index];
}

void MeshTopoData::BuildVertexClusterList()
{

	int vCount = 0;

	if (mVertexClusterList.Count() != TVMaps.v.Count())
		mVertexClusterList.SetCount(TVMaps.v.Count());

	for (int i = 0; i < TVMaps.v.Count(); i++)
		mVertexClusterList[i] = -1;

	if (TVMaps.v.Count()==0) return;


	for (int i =0; i < TVMaps.f.Count(); i++)
	{
		int ct = TVMaps.f[i]->count;
		for (int j =0; j < ct; j++)
		{
			int vIndex = TVMaps.f[i]->v[j];
			int tIndex = TVMaps.f[i]->t[j];
			mVertexClusterList[tIndex] = vIndex;
			if (vIndex > vCount) vCount = vIndex;
			//do patch handles also
			if ( (TVMaps.f[i]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[i]->vecs))
			{
				int tvertIndex = TVMaps.f[i]->vecs->interiors[j];
				int vvertIndex = TVMaps.f[i]->vecs->vinteriors[j];
				if (tvertIndex >=0) 
				{
					mVertexClusterList[tvertIndex] = vvertIndex;
					if (vvertIndex > vCount) vCount = vvertIndex;

				}

				tvertIndex = TVMaps.f[i]->vecs->handles[j*2];
				vvertIndex = TVMaps.f[i]->vecs->vhandles[j*2];
				if (tvertIndex >=0)
				{
					mVertexClusterList[tvertIndex] = vvertIndex;
					if (vvertIndex > vCount) vCount = vvertIndex;

				}

				tvertIndex = TVMaps.f[i]->vecs->handles[j*2+1];
				vvertIndex = TVMaps.f[i]->vecs->vhandles[j*2+1];

				if (tvertIndex >= 0)
				{
					mVertexClusterList[tvertIndex] = vvertIndex;
					if (vvertIndex > vCount) vCount = vvertIndex;
				}

			}        
		}
	}
	vCount++;
	Tab<int> tVertexClusterListCounts;
	if (tVertexClusterListCounts.Count() != vCount)
		tVertexClusterListCounts.SetCount(vCount);
	for (int i = 0; i < vCount; i++)
		tVertexClusterListCounts[i] = 0;

	for (int i = 0; i < mVertexClusterList.Count(); i++)
	{
		int vIndex = mVertexClusterList[i];
		if ((vIndex < 0) || (vIndex >= tVertexClusterListCounts.Count()))
		{
		}
		else tVertexClusterListCounts[vIndex] += 1;
	}

	if (mVertexClusterListCounts.Count() != TVMaps.v.Count())
		mVertexClusterListCounts.SetCount(TVMaps.v.Count());

	for (int i = 0; i < TVMaps.v.Count(); i++)
	{
		int vIndex  = mVertexClusterList[i];
		int ct = tVertexClusterListCounts[vIndex];
		mVertexClusterListCounts[i] = ct;
	}

}

void MeshTopoData::BuildSnapBuffer(int width, int height)
{
		//clear out the buffers
	int s = width*height;
	if (mVertexSnapBuffer.Count() != s)
		mVertexSnapBuffer.SetCount(s);
	if (mEdgeSnapBuffer.Count() != s)
		mEdgeSnapBuffer.SetCount(s);

	for (int i = 0; i < s; i++)
	{
		mVertexSnapBuffer[i] = -1;
		mEdgeSnapBuffer[i] = -2;
	}
	
	mEdgesConnectedToSnapvert.SetSize(GetNumberTVEdges());//TVMaps.ePtrList.Count());
	mEdgesConnectedToSnapvert.ClearAll();
}

void MeshTopoData::SetVertexSnapBuffer(int index, int value)
{
	if ((index < 0) || (index >= mVertexSnapBuffer.Count()))
	{
		DbgAssert(0);
		return;
	}
	mVertexSnapBuffer[index] = value;
}
int MeshTopoData::GetVertexSnapBuffer(int index)
{
	if ((index < 0) || (index >= mVertexSnapBuffer.Count()))
	{
		DbgAssert(0);
		return -1;
	}
	return mVertexSnapBuffer[index];
}

void MeshTopoData::SetEdgeSnapBuffer(int index, int value)
{
	if ((index < 0) || (index >= mEdgeSnapBuffer.Count()))
	{
		DbgAssert(0);
		return;
	}
	mEdgeSnapBuffer[index] = value;
}
int MeshTopoData::GetEdgeSnapBuffer(int index)
{
	if ((index < 0) || (index >= mEdgeSnapBuffer.Count()))
	{
		DbgAssert(0);
		return -1;
	}
	return mEdgeSnapBuffer[index];
}

Tab<int> &MeshTopoData::GetEdgeSnapBuffer()
{
	return mEdgeSnapBuffer;
}

void MeshTopoData::SetEdgesConnectedToSnapvert(int index, BOOL value)
{
	if ((index < 0) || (index >= mEdgesConnectedToSnapvert.GetSize()))
	{
		DbgAssert(0);
		return;
	}
	mEdgesConnectedToSnapvert.Set(index,value);
}
BOOL MeshTopoData::GetEdgesConnectedToSnapvert(int index)
{
	if ((index < 0) || (index >= mEdgesConnectedToSnapvert.GetSize()))
	{
		DbgAssert(0);
		return FALSE;
	}
	return mEdgesConnectedToSnapvert[index];
}


void MeshTopoData::SaveFaces(FILE *file)
{
	TVMaps.SaveFaces(file);
}

void MeshTopoData::LoadFaces(FILE *file, int ver)
{
	int fct;
	fread(&fct, sizeof(fct), 1,file);
	TVMaps.SetCountFaces(fct);
	if ((fct) && (ver < 4)) {
		//fix me old data
		Tab<UVW_TVFaceOldClass> oldData;
		oldData.SetCount(fct);
		fread(oldData.Addr(0), sizeof(UVW_TVFaceOldClass)*fct, 1,file);
		for (int i = 0; i < fct; i++)
		{
			TVMaps.f[i]->t[0] = oldData[i].t[0];
			TVMaps.f[i]->t[1] = oldData[i].t[1];
			TVMaps.f[i]->t[2] = oldData[i].t[2];
			TVMaps.f[i]->t[3] = oldData[i].t[3];
			TVMaps.f[i]->FaceIndex = oldData[i].FaceIndex;
			TVMaps.f[i]->MatID = oldData[i].MatID;
			TVMaps.f[i]->flags = oldData[i].flags;
			TVMaps.f[i]->vecs = NULL;
			if (TVMaps.f[i]->flags & 8)  // not this was FLAG_QUAD but this define got removed
				TVMaps.f[i]->count=4;
			else TVMaps.f[i]->count=3;

		}
		//now compute the geom points
		for (int i = 0; i < TVMaps.f.Count(); i++)
		{
			int pcount = 3;
			pcount = TVMaps.f[i]->count;

			for (int j =0; j < pcount; j++)
			{
				BOOL found = FALSE;
				int index = 0;
				for (int k =0; k < TVMaps.geomPoints.Count();k++)
				{
					if (oldData[i].pt[j] == TVMaps.geomPoints[k])
					{
						found = TRUE;
						index = k;
						k = TVMaps.geomPoints.Count();
					}
				}
				if (found)
				{
					TVMaps.f[i]->v[j] = index;
				}
				else
				{
					TVMaps.f[i]->v[j] = TVMaps.geomPoints.Count();
					TVMaps.geomPoints.Append(1,&oldData[i].pt[j],1);
				}
			}

		}

	}
	else
	{
		TVMaps.LoadFaces(file);
	}

	TVMaps.edgesValid = FALSE;

}

int MeshTopoData::LoadTVVerts(FILE *file)
{
	//check if oldversion
	int ver = 3;
	int vct = GetNumberTVVerts();//TVMaps.v.Count(); 
	int fct = GetNumberFaces();//TVMaps.f.Count();

	fread(&ver, sizeof(ver), 1,file);
	if (ver == -1)
	{
		fread(&ver, sizeof(ver), 1,file);
		fread(&vct, sizeof(vct), 1,file);
	}
	else
	{
		vct = ver;
		ver = 3;
	}




	if (vct) 
	{
		Tab<UVW_TVVertClass_Max9> oldData;
		oldData.SetCount(vct);
		fread(oldData.Addr(0), sizeof(UVW_TVVertClass_Max9)*vct, 1,file);

		TVMaps.v.SetCount(vct);
		for (int i = 0; i < vct; i++)
		{
			TVMaps.v[i].SetP(oldData[i].p);
			TVMaps.v[i].SetInfluence(0.0f);
			TVMaps.v[i].SetFlag(oldData[i].flags);
			TVMaps.v[i].SetControlID(-1);
		}
		mVSel.SetSize(vct);
	}





	return ver;
}
void MeshTopoData::SaveTVVerts(FILE *file)
{
	int vct = TVMaps.v.Count();
	Tab<UVW_TVVertClass_Max9> oldData;
	oldData.SetCount(vct);
	for (int i = 0; i < vct; i++)
	{
		oldData[i].p = GetTVVert(i);
		oldData[i].flags = GetTVVertFlag(i);
		oldData[i].influence = 0.0f;
	}

	fwrite(oldData.Addr(0), sizeof(UVW_TVVertClass_Max9)*vct, 1,file);//				fwrite(TVMaps.v.Addr(0), sizeof(UVW_TVVertClass)*vct, 1,file);

}



UVW_TVFaceClass *MeshTopoData::CloneFace(int faceIndex)
{
	if ((faceIndex < 0) || (faceIndex >= TVMaps.f.Count()))
	{
		DbgAssert(0);
		return NULL;
	}

	return TVMaps.f[faceIndex]->Clone();
}


void MeshTopoData::CopyFaceData(Tab<UVW_TVFaceClass*> &f)
{
	TVMaps.CloneFaces(f);
}
	//this paste the tv face data back from f
void MeshTopoData::PasteFaceData(Tab<UVW_TVFaceClass*> &f)
{
	TVMaps.AssignFaces(f);	
	SetTVEdgeInvalid();

}

void MeshTopoData::BreakEdges(BitArray uvEdges)
{
	TVMaps.SplitUVEdges(uvEdges);
}

void MeshTopoData::BreakEdges(UnwrapMod *mod)
{
	TimeValue t = GetCOREInterface()->GetTime();
	BitArray weldEdgeList;
	weldEdgeList.SetSize(TVMaps.ePtrList.Count()); 
	weldEdgeList.ClearAll();


	BitArray vertsOnselectedEdge;
	vertsOnselectedEdge.SetSize(TVMaps.v.Count());
	vertsOnselectedEdge.ClearAll();

	for (int i = 0; i < TVMaps.ePtrList.Count(); i++)
	{
		if (mESel[i])
		{
			int a = TVMaps.ePtrList[i]->a;
			vertsOnselectedEdge.Set(a,TRUE);
			a = TVMaps.ePtrList[i]->b;
			vertsOnselectedEdge.Set(a,TRUE);
		}
	}
	for (int i = 0; i < TVMaps.ePtrList.Count(); i++)
	{
		if (!mESel[i])
		{
			int a = TVMaps.ePtrList[i]->a;
			int b = TVMaps.ePtrList[i]->b;
			if (vertsOnselectedEdge[a] || vertsOnselectedEdge[b])
				weldEdgeList.Set(i,TRUE);

		}
	}

	BitArray processedVerts;
	processedVerts.SetSize(TVMaps.v.Count());
	processedVerts.ClearAll();

	BitArray processedFace;
	processedFace.SetSize(TVMaps.f.Count());
	processedFace.ClearAll();


	Tab<int> groupsAtThisVert;
	groupsAtThisVert.SetCount(TVMaps.v.Count());

	for (int i = 0; i < TVMaps.v.Count(); i++)
	{
		groupsAtThisVert[i] = 0;
	}

	Tab<AdjacentItem*> edgesAtVert;
	TVMaps.BuildAdjacentUVEdgesToVerts( edgesAtVert);

	Tab<AdjacentItem*> facesAtVert;
	TVMaps.BuildAdjacentUVFacesToVerts( facesAtVert);

	BitArray boundaryVert;
	boundaryVert.SetSize(TVMaps.v.Count());

	Tab<UVW_TVFaceClass*> tempF;
	tempF.SetCount(TVMaps.f.Count());
	for (int i = 0; i < TVMaps.f.Count(); i++)
	{
		tempF[i] = TVMaps.f[i]->Clone();
	}

	Tab<UVW_TVVertClass> tempV;
	tempV =  TVMaps.v;

	for (int i = 0; i < tempV.Count(); i++)
	{
		if (vertsOnselectedEdge[i])
		{
			//get a selected edge
			int numberOfEdges = edgesAtVert[i]->index.Count();
			boundaryVert.ClearAll();
			for (int j = 0; j < numberOfEdges; j++)
			{
				int edgeIndex = edgesAtVert[i]->index[j];
				if (mESel[edgeIndex] || TVMaps.ePtrList[edgeIndex]->faceList.Count()==1)
				{

					int a = TVMaps.ePtrList[edgeIndex]->a;
					int b = TVMaps.ePtrList[edgeIndex]->b;
					if (a == i)
					{
						boundaryVert.Set(b,TRUE);
					}
					else
					{
						boundaryVert.Set(a,TRUE);
					}
				}
			}
			//get a seed face
			int currentGroup = 0;

			while (facesAtVert[i]->index.Count() > 0)
			{
				Tab<int> groupFaces;
				int faceIndex = facesAtVert[i]->index[0];
				groupFaces.Append(1,&faceIndex,20);
				facesAtVert[i]->index.Delete(0,1);
				int a = i;
				int nextVert = -1;
				int previousVert = -1;
				tempF[faceIndex]->GetConnectedUVVerts(a, nextVert, previousVert);

				BOOL done = FALSE;
				while (!done)
				{
					//add face

					for (int k =0; k < facesAtVert[i]->index.Count(); k++)
					{
						int testFaceIndex = facesAtVert[i]->index[k];
						int n,p;

						tempF[testFaceIndex]->GetConnectedUVVerts(a, n, p);

						if ((n == previousVert) && (!boundaryVert[previousVert]))
						{
							previousVert = p;
							groupFaces.Append(1,&testFaceIndex,20);
							facesAtVert[i]->index.Delete(k,1);
							k--;
						}
						else if ((p == nextVert) && (!boundaryVert[nextVert]))
						{
							nextVert = n;
							groupFaces.Append(1,&testFaceIndex,20);
							facesAtVert[i]->index.Delete(k,1);
							k--;
						}
					}

					if (boundaryVert[nextVert] && boundaryVert[previousVert])
						done = TRUE;
				}

				if (currentGroup > 0)
				{
					int newIndex = -1;
					for (int k = 0; k < groupFaces.Count(); k++)
					{
						//find all the verts == i 
						int faceIndex = groupFaces[k];
						int deg = TVMaps.f[faceIndex]->count;
						//make a new vert

						for (int m = 0; m < deg; m++)
						{
							int tindex = TVMaps.f[faceIndex]->t[m];
							if (tindex == a)
							{
								if (newIndex == -1)
								{
									Point3 p = TVMaps.v[tindex].GetP();
									AddTVVert(t,p, faceIndex,m,mod, FALSE);
									newIndex  = TVMaps.f[faceIndex]->t[m];
								}
								else 
								{
									TVMaps.f[faceIndex]->t[m] = newIndex;
								}
							}
						}

					}
				}

				currentGroup++;


			}


		}
	}

	for (int i = 0; i < TVMaps.f.Count(); i++)
	{
		if (tempF[i])
			delete tempF[i];
	}

	for (int i = 0; i < facesAtVert.Count(); i++)
	{
		if (facesAtVert[i])
			delete facesAtVert[i];
	}
	for (int i = 0; i < edgesAtVert.Count(); i++)
	{
		if (edgesAtVert[i])
			delete edgesAtVert[i];
	}
	//now split patch handles if any
	for (int i = 0; i < TVMaps.ePtrList.Count(); i++)
	{
		if (mESel[i])
		{
			int veca = TVMaps.ePtrList[i]->avec;
			if (veca != -1)
			{
				int faceIndex = TVMaps.ePtrList[i]->faceList[0];
				int deg = TVMaps.f[faceIndex]->count;
				int j = faceIndex;
				for (int k =0; k < deg*2; k++)
				{

					if ( (TVMaps.f[j]->flags & FLAG_CURVEDMAPPING) &&
						(TVMaps.f[j]->vecs) &&
						(TVMaps.f[j]->vecs->handles[k] == veca) && 
						(!(TVMaps.f[j]->flags & FLAG_DEAD))
						)						
					{
						Point3 p = TVMaps.v[veca].GetP();
						AddTVHandle(t,p, j, k,mod,TRUE);
					}

				}
			}
			veca = TVMaps.ePtrList[i]->bvec;
			if (veca != -1)
			{
				int faceIndex = TVMaps.ePtrList[i]->faceList[0];
				int deg = TVMaps.f[faceIndex]->count;
				int j = faceIndex;
				for (int k =0; k < deg*2; k++)
				{

					if ( (TVMaps.f[j]->flags & FLAG_CURVEDMAPPING) &&
						(TVMaps.f[j]->vecs) &&
						(TVMaps.f[j]->vecs->handles[k] == veca) && 
						(!(TVMaps.f[j]->flags & FLAG_DEAD))
						)						
					{
						Point3 p = TVMaps.v[veca].GetP();
						AddTVHandle(t,p, j, k,mod,TRUE);
					}

				}
			}
		}
	}
	SetTVEdgeInvalid();
	TVMaps.BuildEdges();
	BuildVertexClusterList();
	if (mESel.GetSize() != TVMaps.ePtrList.Count())
	{
		mESel.SetSize(TVMaps.ePtrList.Count());
		mESel.ClearAll();
	}

}
void MeshTopoData::BreakVerts(UnwrapMod *mod)
{
	TimeValue t = GetCOREInterface()->GetTime();

	BitArray weldEdgeList;
	weldEdgeList.SetSize(TVMaps.ePtrList.Count()); 
	weldEdgeList.ClearAll();

	for (int i=0; i<TVMaps.v.Count(); i++) 
	{
		if ( (mVSel[i]) && (!(TVMaps.v[i].IsDead())) && !TVMaps.mSystemLockedFlag[i] )
		{
			//find all faces attached to this vertex
			Point3 p = TVMaps.v[i].GetP();//GetPoint(ip->GetTime(),i);
			BOOL first = TRUE;
			for (int j=0; j<TVMaps.f.Count(); j++) 
			{
				int pcount = 3;
				pcount = TVMaps.f[j]->count ;
				for (int k = 0; k < pcount;k++)
				{
					if ((TVMaps.f[j]->t[k] == i) && (!(TVMaps.f[j]->flags & FLAG_DEAD)))
					{
						if (first)
						{
							first = FALSE;
						}
						else
						{
							AddTVVert(t,p, j, k,mod,TRUE);
						}
					}
					if ( (TVMaps.f[j]->flags & FLAG_CURVEDMAPPING) &&
						(TVMaps.f[j]->vecs) &&
						(TVMaps.f[j]->vecs->handles[k*2] == i) && 
						(!(TVMaps.f[j]->flags & FLAG_DEAD))
						)
					{
						if (first)
						{
							first = FALSE;
						}
						else
						{
							AddTVHandle(t,p, j, k*2,mod,TRUE);
						}

					}
					if ( (TVMaps.f[j]->flags & FLAG_CURVEDMAPPING) &&
						(TVMaps.f[j]->vecs) &&
						(TVMaps.f[j]->vecs->handles[k*2+1] == i) && 
						(!(TVMaps.f[j]->flags & FLAG_DEAD))
						)
					{
						if (first)
						{
							first = FALSE;
						}
						else
						{
							AddTVHandle(t,p, j, k*2+1,mod,TRUE);
						}

					}
					if ( (TVMaps.f[j]->flags & FLAG_CURVEDMAPPING) &&
						(TVMaps.f[j]->flags & FLAG_INTERIOR) &&
						(TVMaps.f[j]->vecs) &&
						(TVMaps.f[j]->vecs->interiors[k] == i) && 
						(!(TVMaps.f[j]->flags & FLAG_DEAD))
						)
					{
						if (first)
						{
							first = FALSE;
						}
						else
						{
							AddTVInterior(t,p, j, k,mod,TRUE);
						}
					}
				}
			}

		}
	}

	SetTVEdgeInvalid();	
	TVMaps.BuildEdges();
	BuildVertexClusterList();
	if (mESel.GetSize() != TVMaps.ePtrList.Count())
	{
		mESel.SetSize(TVMaps.ePtrList.Count());
		mESel.ClearAll();
	}
}


void MeshTopoData::SelectHandles(int dir)

{
	//if face is selected select me
	for (int j = 0; j < TVMaps.f.Count();j++)
	{
		if ( (!(TVMaps.f[j]->flags & FLAG_DEAD)) &&
			(TVMaps.f[j]->flags & FLAG_CURVEDMAPPING) &&
			(TVMaps.f[j]->vecs) 
			)

		{
			int pcount = 3;
			pcount = TVMaps.f[j]->count;
			for (int k = 0; k < pcount; k++)
			{
				int id = TVMaps.f[j]->t[k];
				if (dir ==0)
				{
					if ((mVSel[id])/* && (!wasSelected[id])*/)
					{
						int vid1 = TVMaps.f[j]->vecs->handles[k*2];
						int vid2 = 0;
						if (k ==0)
							vid2 = TVMaps.f[j]->vecs->handles[pcount*2-1];
						else vid2 = TVMaps.f[j]->vecs->handles[k*2-1];

						if ( (IsTVVertVisible(vid1)) &&  (!(TVMaps.v[vid1].IsFrozen()) ))
							mVSel.Set(vid1,1);
						if ( (IsTVVertVisible(vid2)) &&  (!(TVMaps.v[vid2].IsFrozen()) ))
							mVSel.Set(vid2,1);

						if (TVMaps.f[j]->flags & FLAG_INTERIOR)
						{
							int ivid1 = TVMaps.f[j]->vecs->interiors[k];
							if ( (ivid1 >= 0) && (IsTVVertVisible(ivid1)) &&  (!(TVMaps.v[ivid1].IsFrozen())) )
								mVSel.Set(ivid1,1);
						}
					}
				}
				else
				{
					if (!mVSel[id])
					{
						int vid1 = TVMaps.f[j]->vecs->handles[k*2];
						int vid2 = 0;
						if (k ==0)
							vid2 = TVMaps.f[j]->vecs->handles[pcount*2-1];
						else vid2 = TVMaps.f[j]->vecs->handles[k*2-1];

						if ( (IsTVVertVisible(vid1)) &&  (!(TVMaps.v[vid1].IsFrozen())) )
							mVSel.Set(vid1,0);
						if ( (IsTVVertVisible(vid2)) &&  (!(TVMaps.v[vid2].IsFrozen())) )
							mVSel.Set(vid2,0);

						if (TVMaps.f[j]->flags & FLAG_INTERIOR)
						{
							int ivid1 = TVMaps.f[j]->vecs->interiors[k];
							if ( (ivid1 >= 0) && (IsTVVertVisible(ivid1)) &&  (!(TVMaps.v[ivid1].IsFrozen())) )
								mVSel.Set(ivid1,0);
						}

					}

				}

			}
		}
	}

}

void MeshTopoData::SelectVertexElement(BOOL addSelection)
{





	BitArray faceHasSelectedVert;

	faceHasSelectedVert.SetSize(GetNumberFaces());//TVMaps.f.Count());
	faceHasSelectedVert.ClearAll();

	int count = -1;

	while (count != mVSel.NumberSet())
	{
		count = mVSel.NumberSet();
		for (int i = 0; i < GetNumberFaces()/*TVMaps.f.Count()*/;i++)
		{
			if (!(GetFaceDead(i)))//TVMaps.f[i]->flags & FLAG_DEAD))
			{
				int pcount = 3;
				pcount = GetFaceDegree(i);//TVMaps.f[i]->count;
				int totalSelected = 0;
				for (int k = 0; k < pcount; k++)
				{
					int index = GetFaceTVVert(i,k);//TVMaps.f[i]->t[k];
					if (mVSel[index])
					{
						totalSelected++;
					}
				}

				if ( (totalSelected != pcount) && (totalSelected!= 0))
				{
					faceHasSelectedVert.Set(i);
				}
			}
		}
		for (int i = 0; i < GetNumberFaces()/*TVMaps.f.Count()*/;i++)
		{
			if (faceHasSelectedVert[i])
			{
				int pcount = 3;
				pcount = GetFaceDegree(i);//TVMaps.f[i]->count;
				int totalSelected = 0;
				for (int k = 0; k < pcount; k++)
				{
					int index =  GetFaceTVVert(i,k);//TVMaps.f[i]->t[k];
					if (addSelection)
						mVSel.Set(index,1);
					else mVSel.Set(index,0);
				}
			}

		}
	}
	SelectHandles(0);

	
}

void MeshTopoData::SelectElement(int mode,BOOL addSelection)
{
	bool faceMode = false;
	BitArray tempFSel;

	TransferSelectionStart(mode);



	//		BitArray fsel = GetFaceSelection();
	//		BitArray vsel = GetTVVertSelection();

	if (mode == TVFACEMODE)
	{
		faceMode = true;
		tempFSel = mFSel;
	}



	if (faceMode && (!addSelection))
	{
		for (int i = 0; i < GetNumberFaces()/*TVMaps.f.Count()*/;i++)
		{
			if (!(GetFaceDead(i)) && (!tempFSel[i]))
			{
				int pcount = 3;
				pcount = GetFaceDegree(i);//TVMaps.f[i]->count;
				int totalSelected = 0;
				for (int k = 0; k < pcount; k++)
				{
					int index = GetFaceTVVert(i,k);//TVMaps.f[i]->t[k];
					mVSel.Set(index,false);
				}
			}
		}
	}


	BitArray faceHasSelectedVert;

	faceHasSelectedVert.SetSize(GetNumberFaces());//TVMaps.f.Count());
	faceHasSelectedVert.ClearAll();

	int count = -1;

	while (count != mVSel.NumberSet())
	{
		count = mVSel.NumberSet();
		for (int i = 0; i < GetNumberFaces()/*TVMaps.f.Count()*/;i++)
		{
			if (!(GetFaceDead(i)))//TVMaps.f[i]->flags & FLAG_DEAD))
			{
				int pcount = 3;
				pcount = GetFaceDegree(i);//TVMaps.f[i]->count;
				int totalSelected = 0;
				for (int k = 0; k < pcount; k++)
				{
					int index = GetFaceTVVert(i,k);//TVMaps.f[i]->t[k];
					if (mVSel[index])
					{
						totalSelected++;
					}
				}

				if ( (totalSelected != pcount) && (totalSelected!= 0))
				{
					faceHasSelectedVert.Set(i);
				}
			}
		}
		for (int i = 0; i < GetNumberFaces()/*TVMaps.f.Count()*/;i++)
		{
			if (faceHasSelectedVert[i])
			{
				int pcount = 3;
				pcount = GetFaceDegree(i);//TVMaps.f[i]->count;
				int totalSelected = 0;
				for (int k = 0; k < pcount; k++)
				{
					int index =  GetFaceTVVert(i,k);//TVMaps.f[i]->t[k];
					if (addSelection)
						mVSel.Set(index,1);
					else mVSel.Set(index,0);
				}
			}

		}
	}
	SelectHandles(0);

	TransferSelectionEnd(mode,FALSE,TRUE);
}

void  MeshTopoData::GetEdgeCluster(BitArray &cluster)
{

	BitArray tempArray;
	//next check to make sure we only have one cluster and if not element the smaller ones

	tempArray.SetSize(TVMaps.v.Count());
	tempArray.ClearAll();
	tempArray = mVSel;


	int seedVert=-1;
	int seedSize = -1;
	BitArray oppoProcessedElement;
	oppoProcessedElement.SetSize(TVMaps.v.Count());
	oppoProcessedElement.ClearAll();


	//5.1.04  remove any verts that share common geo verts
	Tab<int> geoIDs;
	geoIDs.SetCount(TVMaps.v.Count());
	for (int i = 0; i < TVMaps.f.Count(); i++)
	{
		UVW_TVFaceClass *face = TVMaps.f[i];
		for (int j = 0; j < face->count; j++)
		{
			int gid = face->v[j];
			int vid = face->t[j];
			if ((gid >= 0) && (gid < geoIDs.Count()))
				geoIDs[vid] = gid;
		}
	}
	for (int i = 0; i < TVMaps.v.Count(); i++)
	{
		if (cluster[i])
		{
			//tag all shared geo verts
			int gid = geoIDs[i];
			BitArray sharedVerts;
			sharedVerts.SetSize(TVMaps.v.Count());
			sharedVerts.ClearAll();
			for (int j = 0; j < TVMaps.v.Count(); j++)
			{
				if ((geoIDs[j] == gid) && cluster[j])
					sharedVerts.Set(j);
			}
			//now look to see any of these
			if (sharedVerts.NumberSet() > 1)
			{
				int sharedEdge = -1;
				for (int j = 0; j < TVMaps.v.Count(); j++)
				{
					if (sharedVerts[j])
					{
						//loop through our edges and see if this one touches any of our vertices
						for (int k = 0; k < TVMaps.ePtrList.Count(); k++)
						{
							int a = TVMaps.ePtrList[k]->a;
							int b = TVMaps.ePtrList[k]->b;
							if (a == j)
							{
								if (cluster[b]) sharedEdge = j;
							}
							else if (b == j)
							{
								if (cluster[a]) sharedEdge = j;
							}

						}
					}
				}
				if (sharedEdge == -1)
				{
					BOOL first = TRUE;
					for (int j = 0; j < TVMaps.v.Count(); j++)
					{
						if (sharedVerts[j])
						{
							if (!first)
								cluster.Set(j,FALSE);
							first = FALSE;
						}
					}
				}
				else
				{
					for (int j = 0; j < TVMaps.v.Count(); j++)
					{
						if (sharedVerts[j])
						{
							cluster.Set(j,FALSE);
						}
					}
					cluster.Set(sharedEdge,TRUE);

				}
			}

		}
	}


	for (int i = 0; i < TVMaps.v.Count(); i++)
	{
		if ((!oppoProcessedElement[i])  && (cluster[i]))
		{
			mVSel.ClearAll();
			mVSel.Set(i);
			SelectVertexElement(TRUE);

			oppoProcessedElement |= mVSel;

			mVSel = mVSel & cluster;
			int ct = mVSel.NumberSet();
			if (ct > seedSize)
			{
				seedSize = ct;
				seedVert = i;
			}

		}
	}
	if (seedVert != -1)
	{
		mVSel.ClearAll();
		mVSel.Set(seedVert);
		SelectVertexElement(TRUE);
		cluster = mVSel & cluster;
	}

	mVSel = tempArray;

}


void MeshTopoData::Stitch(BOOL bAlign, float fBias, BOOL bScale, UnwrapMod *mod)
{

	BOOL firstVert = mVSel[0];

	TimeValue t = GetCOREInterface()->GetTime();
	BitArray initialVertexSelection = mVSel;

	BuildVertexClusterList();

	//clean up selection
	//remove any verts not on edge
	for (int i =0; i < TVMaps.v.Count(); i++)
	{
		//only process selected vertices that have more than one connected verts
		//and that have not been already processed (tprocessedVerts)
		if ((mVertexClusterListCounts[i] <= 1) && (mVSel[i]))
		{
			mVSel.Set(i,FALSE);
		}
	}
	//if multiple edges selected set it to the largest
	GetEdgeCluster(mVSel);

	firstVert = mVSel[0];

	//this builds a list of verts connected to the selection
	BitArray oppositeVerts;
	oppositeVerts.SetSize(TVMaps.v.Count());
	oppositeVerts.ClearAll();

	if (mVSel.NumberSet() == 1)
	{
		//get our one vertex
		int vIndex = 0;
		for (int i = 0; i < mVSel.GetSize(); i++)
		{
			if (mVSel[i]) {
				vIndex = i;
				break;
			}
		}
		//loop through our edges
		BitArray connectedVerts;
		connectedVerts.SetSize(mVSel.GetSize());
		connectedVerts.ClearAll();


		for (int i = 0; i < TVMaps.ePtrList.Count(); i++)
		{
			//see if one edge vert equals out
			int a,b;
			a = TVMaps.ePtrList[i]->a;
			b = TVMaps.ePtrList[i]->b;

			if (a == vIndex) 
				connectedVerts.Set(b);
			if (b == vIndex) 
				connectedVerts.Set(a);
		}

		int clusterId = mVertexClusterList[vIndex];
		for (int i = 0; i < TVMaps.ePtrList.Count(); i++)
		{        
			int a,b;
			a = TVMaps.ePtrList[i]->a;
			b = TVMaps.ePtrList[i]->b;
			int potentialVertex = -1;
			int potentialVertexOp = -1;
			if ( mVertexClusterList[a] == clusterId)
			{
				potentialVertex = a;
				potentialVertexOp = b;
			}
			if ( mVertexClusterList[b] == clusterId)
			{
				potentialVertex = b;
				potentialVertexOp = a;
			}
			if ((potentialVertex != -1) && (potentialVertex != vIndex))
			{
				if (connectedVerts[potentialVertexOp])
				{
					oppositeVerts.Set(potentialVertex);
					i = TVMaps.ePtrList.Count();
				}
			}

		}
	}
	else
	{
		for (int i =0; i < TVMaps.v.Count(); i++)
		{
			//only process selected vertices that have more than one connected verts
			//and that have not been already processed (tprocessedVerts)
			if ((mVertexClusterListCounts[i] > 1) && (mVSel[i]))
			{
				int clusterId = mVertexClusterList[i];
				//now loop through all the
				for (int j =0; j < TVMaps.v.Count(); j++)
				{
					if ((clusterId == mVertexClusterList[j]) && (j!=i))
					{
						if (!mVSel[j])
							oppositeVerts.Set(j);
					}
				}

			}
		}
	}

	firstVert = mVSel[0];

	GetEdgeCluster(oppositeVerts);

	BitArray elem,tempArray;
	tempArray.SetSize(TVMaps.v.Count());
	tempArray.ClearAll();

	//clean up mVSel and rmeove any that are not part of the opposite set
	for (int i =0; i < TVMaps.v.Count(); i++)
	{
		if (oppositeVerts[i])
		{
			int clusterId = mVertexClusterList[i];
			for (int j =0; j < TVMaps.v.Count(); j++)
			{
				if ((clusterId == mVertexClusterList[j]) && (j!=i) && (mVSel[j]))
					tempArray.Set(j);
			}

		}
	}
	mVSel = mVSel & tempArray;


	tempArray.SetSize(TVMaps.v.Count());
	tempArray.ClearAll();
	tempArray = mVSel;

	//this builds a list of the two elements clusters
	//basically all the verts that are connected to the selection and the opposing verts
	int seedA=-1, seedB=-1;
	for (int i =0; i < TVMaps.v.Count(); i++)
	{
		if ((mVSel[i]) && (seedA == -1))
			seedA = i;
		if ((oppositeVerts[i]) && (seedB == -1))
			seedB = i;
	}

	if (seedA == -1) return;
	if (seedB == -1) return;



	elem.SetSize(TVMaps.v.Count());
	elem.SetSize(TVMaps.v.Count());

	tempArray.SetSize(TVMaps.v.Count());
	tempArray.ClearAll();
	tempArray = mVSel;

	mVSel.ClearAll();
	mVSel.Set(seedA);
	SelectVertexElement(TRUE);
	elem = mVSel;

	firstVert = mVSel[0];

	mVSel.ClearAll();
	mVSel.Set(seedB);
	SelectVertexElement(TRUE);
	elem |= mVSel;

	firstVert = mVSel[0];

	mVSel = tempArray;

	int sourcePoints[2];
	int targetPoints[2];



	//align the clusters if they are seperate 
	if (bAlign)
	{
		//check to make sure there are only 2 elements
		int numberOfElements = 2;

		BitArray elem1,elem2, holdArray;

		//build element 1
		elem1.SetSize(TVMaps.v.Count());
		elem2.SetSize(TVMaps.v.Count());
		elem1.ClearAll();
		elem2.ClearAll();


		holdArray.SetSize(mVSel.GetSize());
		holdArray = mVSel;

		SelectVertexElement(TRUE);
		firstVert = mVSel[0];

		elem1 = mVSel;

		mVSel = oppositeVerts;
		elem2 = oppositeVerts;
		SelectVertexElement(TRUE);

		firstVert = mVSel[0];


		for (int i = 0; i < TVMaps.v.Count(); i++)
		{
			if (mVSel[i] && elem1[i])
			{
				numberOfElements = 0;
				break;
			}
		}



		if (numberOfElements == 2)
		{
			//build a line from selection of element 1
			elem1 = holdArray;

			sourcePoints[0] = -1;
			sourcePoints[1] = -1;
			targetPoints[0] = -1;
			targetPoints[1] = -1;

			int ct = 0;
			Tab<int> connectedCount;
			connectedCount.SetCount(TVMaps.v.Count());

			for (int i = 0; i < TVMaps.v.Count(); i++)
				connectedCount[i] = 0;

			//find a center point and then the farthest point from it
			Point3 centerP(0.0f,0.0f,0.0f);
			Box3 bounds;
			bounds.Init();
			for (int i = 0; i < TVMaps.v.Count(); i++)
			{
				if ((holdArray[i])  && (mVertexClusterListCounts[i] > 1))
				{
					bounds += TVMaps.v[i].GetP();
				}
			}
			centerP = bounds.Center();
			int closestIndex = -1;
			float closestDist = 0.0f;
			centerP.z = 0.0f;

			for (int i = 0; i < TVMaps.v.Count(); i++)
			{
				if ((holdArray[i])  && (mVertexClusterListCounts[i] ==2 ) )//watjeFIX
				{
					float dist;
					Point3 p = TVMaps.v[i].GetP();
					p.z = 0.0f;
					dist = LengthSquared(p-centerP);
					if ((dist > closestDist) || (closestIndex == -1))
					{
						closestDist = dist;
						closestIndex = i;
					}
				}
			}
			sourcePoints[0] = closestIndex;

			closestIndex = -1;
			closestDist = 0.0f;
			for (int i = 0; i < TVMaps.v.Count(); i++)
			{
				if ((holdArray[i])  && (mVertexClusterListCounts[i] == 2))//watjeFIX
				{
					float dist;
					Point3 p = TVMaps.v[i].GetP();
					p.z = 0.0f;
					dist = LengthSquared(p-TVMaps.v[sourcePoints[0]].GetP());
					if (((dist > closestDist) || (closestIndex == -1)) && (i!=sourcePoints[0])) //fix 5.1.04
					{
						closestDist = dist;
						closestIndex = i;
					}
				}
			}
			sourcePoints[1] = closestIndex;

			//fix 5.1.04 if our guess fails  pick 2 at random
			if ( (sourcePoints[0] == -1) || (sourcePoints[1] == -1) )
			{
				int ct = 0;
				for (int i = 0; i < TVMaps.v.Count(); i++)
				{
					if ((holdArray[i])  && (mVertexClusterListCounts[i] >= 2 ) )
					{
						sourcePoints[ct] = i;
						ct++;
						if (ct == 2) break;

					}
				}

			}


			// find the matching corresponding points on the opposite edge
			//by matching the 
			ct = 0;
			int clusterID = mVertexClusterList[sourcePoints[0]];
			for (int i = 0; i < TVMaps.v.Count(); i++)
			{
				if (elem2[i])
				{
					if ( (sourcePoints[ct] != i) && (clusterID == mVertexClusterList[i]))
					{
						targetPoints[ct] = i;
						ct++;
						break;
					}
				}
			}

			clusterID = mVertexClusterList[sourcePoints[1]];
			for (int i = 0; i < TVMaps.v.Count(); i++)
			{
				if (elem2[i])
				{
					if ( (sourcePoints[ct] != i) && (clusterID == mVertexClusterList[i]) && (targetPoints[0]!=i) ) //watjeFIX
					{
						targetPoints[ct] = i;
						ct++;
						break;
					}
				}
			}

			if ( (targetPoints[0] != -1) && (targetPoints[1] != -1) &&
				(sourcePoints[0] != -1) && (sourcePoints[1] != -1) )
			{
				//build a line from selection of element 1
				Point3 centerTarget,centerSource,vecTarget,vecSource;

				vecSource = Normalize(TVMaps.v[sourcePoints[1]].GetP()-TVMaps.v[sourcePoints[0]].GetP());
				vecSource.z = 0.0f;
				vecSource = Normalize(vecSource);

				//build a line from selection of element 2
				vecTarget = Normalize(TVMaps.v[targetPoints[1]].GetP()-TVMaps.v[targetPoints[0]].GetP());
				vecTarget.z = 0.0f;
				vecTarget = Normalize(vecTarget);

				Point3 targetStart = TVMaps.v[targetPoints[0]].GetP();
				Point3 targetEnd = TVMaps.v[targetPoints[0]].GetP();
				Point3 sourceStart = TVMaps.v[sourcePoints[0]].GetP();
				Point3 sourceEnd = TVMaps.v[sourcePoints[0]].GetP();

				centerSource = (TVMaps.v[sourcePoints[0]].GetP() + TVMaps.v[sourcePoints[1]].GetP()) / 2.0f; 
				centerTarget = (TVMaps.v[targetPoints[0]].GetP() + TVMaps.v[targetPoints[1]].GetP()) / 2.0f; 

				float angle = 0.0f;
				float dist = Length(vecSource-vecTarget*-1.0f);
				if ( dist < 0.001f) angle = PI;
				else if (Length(vecSource-vecTarget) < 0.001f)  //fixes 5.1.01 fixes a stitch error
					angle = 0.0f;
				else angle = acos(DotProd( vecSource, vecTarget));

				float x,y,x1,y1;
				float tx = vecTarget.x;
				float ty = vecTarget.y;

				float sx = vecSource.x;
				float sy = vecSource.y;

				Point3 sp,spn;
				sp = vecTarget;
				spn = vecTarget;
				Matrix3 tm(1);
				tm.SetRotateZ(angle);
				sp = sp * tm;

				tm.IdentityMatrix();
				tm.SetRotateZ(-angle);
				spn = spn * tm;

				x = sp.x;
				y = sp.y;

				x1 = spn.x;
				y1 = spn.y;

				if ( (fabs(x-vecSource.x) > 0.001f) || (fabs(y-vecSource.y) > 0.001f))
				{
					angle = -angle;
				}
				//now align the elem2
				Point3 delta = centerSource - centerTarget;

				for (int i = 0; i < TVMaps.v.Count(); i++)
				{
					if (mVSel[i])
					{
						Point3 p = TVMaps.v[i].GetP();
						p = p - centerTarget;
						float tx = p.x;
						float ty = p.y;
						p.x = (tx * cos(angle)) - (ty * sin(angle));
						p.y = (tx * sin(angle)) + (ty * cos(angle));
						p += centerSource;

						SetTVVert(t,i,p,mod);
//						if (TVMaps.cont[i]) 
//							TVMaps.cont[i]->SetValue(0,&TVMaps.v[i].GetP());

					}
				}
				//find an offset so the elements do not intersect
				//find an offset so the elements do not intersect
				//new scale
				if (bScale)
				{
					//get the center to start point vector
					//scale all the points
					float tLen = Length(centerTarget-targetEnd);
					float sLen = Length(centerSource-sourceEnd);
					float scale = (sLen/tLen) ;
					Point3 scaleVec = Normalize(targetEnd-centerTarget);

					for (int i = 0; i < TVMaps.v.Count(); i++)
					{
						if (mVSel[i])
						{
							Point3 vec = (TVMaps.v[i].GetP() - centerSource) * scale;
							vec = centerSource + vec;
							SetTVVert(t,i,vec + ((TVMaps.v[i].GetP()-vec) * fBias),mod);
//							if (TVMaps.cont[i]) TVMaps.cont[i]->SetValue(0,&TVMaps.v[i].GetP());

						}
					}
				}
			}

			//align element 2 to element 1

		}
		mVSel  = holdArray;

	}
	//now loop through the vertex list and process only those that are selected.
	BitArray processedVerts;
	processedVerts.SetSize(TVMaps.v.Count());
	processedVerts.ClearAll();



	Tab<Point3> movePoints;
	movePoints.SetCount(TVMaps.v.Count());
	for ( int i =0; i < TVMaps.v.Count(); i++)
		movePoints[i]= Point3(0.0f,0.0f,0.0f);




	for (int i =0; i < TVMaps.v.Count(); i++)
	{
		//only process selected vertices that have more than one connected verts
		//and that have not been already processed (tprocessedVerts)

		if ((mVertexClusterListCounts[i] > 1) && (!oppositeVerts[i]) && (mVSel[i]) && (!processedVerts[i]))
		{
			int clusterId = mVertexClusterList[i];
			//now loop through all the
			Tab<int> slaveVerts;
			processedVerts.Set(i);
			for (int j =0; j < TVMaps.v.Count(); j++)
			{

				if ((clusterId == mVertexClusterList[j]) && (j!=i) && (oppositeVerts[j]))
				{
					slaveVerts.Append(1,&j);

					//             processedVerts.Set(j);
				}
			}
			Point3 center;
			center = TVMaps.v[i].GetP();


			if (slaveVerts.Count()>=2) //watjeFIX
			{
				// find the selected then find the closest
				int selected = 0;
				//          for (int j = 0; j < slaveVerts.Count(); j++)
				//             {
				//             processedVerts.Set(slaveVerts[j],FALSE);
				//             }
				float dist = -1.0f;
				int closest = 0;
				for (int j = 0; j < slaveVerts.Count(); j++)
				{
					float ldist = Length(TVMaps.v[slaveVerts[j]].GetP()-TVMaps.v[i].GetP());
					if ((dist < 0.0f) || (ldist<dist))
					{
						dist = ldist;
						closest = slaveVerts[j];
					}
				}

				processedVerts.Set(closest);
				slaveVerts.SetCount(1);
				slaveVerts[0] = closest;


			}
			else if (slaveVerts.Count()==1) processedVerts.Set(slaveVerts[0]);


			Tab<Point3> averages;
			averages.SetCount(slaveVerts.Count());

			for (int j = 0; j < slaveVerts.Count(); j++)
			{
				int slaveIndex = slaveVerts[j];
				averages[j] = (TVMaps.v[slaveIndex].GetP() - center) * fBias;
			}

			Point3 mid(0.0f,0.0f,0.0f);
			for (int j = 0; j < slaveVerts.Count(); j++)
			{
				mid += averages[j];
			}
			mid = mid/(float)slaveVerts.Count();

			center += mid;

			for (int j = 0; j < slaveVerts.Count(); j++)
			{
				int slaveIndex = slaveVerts[j];
				movePoints[slaveIndex]= center;
			}


			movePoints[i]= center;


		}
	}


	BitArray oldSel;
	oldSel.SetSize(mVSel.GetSize());
	oldSel = mVSel;

	mVSel = processedVerts;
	mod->RebuildDistCache();

	for (int i =0; i < TVMaps.v.Count(); i++)
	{
		float influ = TVMaps.v[i].GetInfluence();
		if ((influ != 0.0f) && (!processedVerts[i]) && (!(TVMaps.v[i].IsDead())) && (elem[i]) && !TVMaps.mSystemLockedFlag[i] )
		{
			//find closest processed vert and use that delta
			int closestIndex = -1;
			float closestDist = 0.0f;
			for (int j =0; j < TVMaps.v.Count(); j++)
			{
				if (processedVerts[j])
				{
					int vertexCT = mVertexClusterListCounts[j];
					if (( vertexCT > 1) && (!(TVMaps.v[j].IsDead())) && !TVMaps.mSystemLockedFlag[j])
					{
						float tdist = LengthSquared(TVMaps.v[i].GetP() - TVMaps.v[j].GetP());
						if ((closestIndex == -1) || (tdist < closestDist))
						{
							closestDist = tdist;
							closestIndex = j;
						}
					}
				}
			}
			if (closestIndex == -1)
			{
/*
				if (gDebugLevel >= 1)
				{
					ScriptPrint("Error stitch soft sel vert %d\n",i); 
				}
*/
			}
			else
			{     
/*
				if (gDebugLevel >= 3)
				{
					ScriptPrint("Moving stitch soft sel vert %d closest edge %d dist %f\n",i,closestIndex, closestDist); 
				}           
*/
				Point3 delta = (movePoints[closestIndex] - TVMaps.v[closestIndex].GetP()) * influ;
				SetTVVert(t,i,TVMaps.v[i].GetP() + delta,mod);
//				if (TVMaps.cont[i]) TVMaps.cont[i]->SetValue(0,&TVMaps.v[i].GetP());
			}
		}

	}

	for (int i =0; i < TVMaps.v.Count(); i++)
	{
		if ( processedVerts[i])
		{

			SetTVVert(t,i,movePoints[i],mod);
		}
	}

}

void MeshTopoData::BuildTVEdges()
{
	TVMaps.BuildEdges();
	if (mESel.GetSize() != TVMaps.ePtrList.Count())
		mESel.SetSize(TVMaps.ePtrList.Count());

}


void MeshTopoData::SelectByMatID(int matID)
{
	mFSel.ClearAll();
	for (int i = 0; i < GetNumberFaces()/*TVMaps.f.Count()*/; i++)
	{
		if (GetFaceMatID(i) == matID) 
			mFSel.Set(i);
	}
}

void  MeshTopoData::HoldFaceSel()
{
	holdFSel = mFSel;
}
void  MeshTopoData::RestoreFaceSel()
{
	mFSel = holdFSel;
}


void MeshTopoData::EdgeListFromPoints(Tab<int> &selEdges, int a, int b, Point3 vec)
{
	TVMaps.EdgeListFromPoints(selEdges, a,b,vec);
}

void MeshTopoData::ExpandSelectionToSeams()
{
	for (int i = 0; i < GetNumberTVEdges(); i++)//tvData->ePtrList.Count(); i++)
	{
		TVMaps.ePtrList[i]->lookupIndex = i;
	}
	for (int i = 0; i < TVMaps.gePtrList.Count(); i++)
	{
		TVMaps.gePtrList[i]->lookupIndex = i;
	}


	int seedFace = -1;

	for (int i = 0; i < mFSel.GetSize(); i++)
	{
		if (mFSel[i])
			seedFace = i;
	}
	if (seedFace == -1) return;



	Tab<FConnnections*> face;
	face.SetCount(TVMaps.f.Count());
	for (int i = 0; i < TVMaps.f.Count(); i++)
		face[i] = new FConnnections();

	for (int i = 0; i < TVMaps.f.Count(); i++)
	{
		int deg = TVMaps.f[i]->count;
		for (int j = 0; j < deg; j++)
		{
			int a = TVMaps.f[i]->v[j];
			int b = TVMaps.f[i]->v[(j+1)%deg];
			int edge = TVMaps.FindGeoEdge(a,b);

			face[i]->connectedEdge.Append(1,&edge,4);
		}
	}


	// start adding edges
	BOOL done = FALSE;
	int currentFace = seedFace;

	BitArray selEdges;
	selEdges.SetSize(TVMaps.gePtrList.Count());
	selEdges.ClearAll();

	BitArray processedFaces;
	processedFaces.SetSize(TVMaps.f.Count());
	processedFaces.ClearAll();
	Tab<int> faceStack;

	if (mSeamEdges.GetSize() != TVMaps.gePtrList.Count())
	{
		mSeamEdges.SetSize(TVMaps.gePtrList.Count());
		mSeamEdges.ClearAll();
	}

	while (!done)
	{
		//loop through the current face edges
		int deg = TVMaps.f[currentFace]->count;
		//mark this face as processed
		processedFaces.Set(currentFace,TRUE);
		for (int j = 0; j < deg; j++)
		{
			int a = TVMaps.f[currentFace]->v[j];
			int b = TVMaps.f[currentFace]->v[(j+1)%deg];
			int edge = TVMaps.FindGeoEdge(a,b);
			//add the edges if they are not a seam
			if (!mSeamEdges[edge])
			{
				selEdges.Set(edge,TRUE);				

				//find the connected faces to this edge
				int numberConnectedFaces = TVMaps.gePtrList[edge]->faceList.Count();
				for (int k = 0; k < numberConnectedFaces; k++)
				{
					//add them to the stack if they have not been processed
					int potentialFaceIndex = TVMaps.gePtrList[edge]->faceList[k];

					if (!processedFaces[potentialFaceIndex])
						faceStack.Append(1,&potentialFaceIndex,100);
				}

			}
		}

		//if stack == empty
		if (faceStack.Count() == 0)
			done = TRUE;
		else
		{
			//else current face = pop top of the stack		
			currentFace = faceStack[faceStack.Count()-1];
			faceStack.Delete(faceStack.Count()-1,1);
		}

	}

	processedFaces.ClearAll();
	for (int i = 0; i < TVMaps.gePtrList.Count(); i++)
	{
		if (selEdges[i] )
		{
			int numberConnectedFaces = TVMaps.gePtrList[i]->faceList.Count();
			for (int k = 0; k < numberConnectedFaces; k++)
			{
				//add them to the stack if they have not been processed
				int potentialFaceIndex = TVMaps.gePtrList[i]->faceList[k];

				processedFaces.Set(potentialFaceIndex,TRUE);						
			}
		}
	}

	mFSel = processedFaces;
	for (int i = 0; i < TVMaps.f.Count(); i++)
		delete face[i];

}


void MeshTopoData::CutSeams(UnwrapMod *mod, BitArray seams)
{
 	UVW_ChannelClass *tvData = &TVMaps;

	//loop through our seams and remove any that are open edges

	for (int i = 0; i < seams.GetSize(); i++)
	{
		if (seams[i])
		{
			if (tvData->gePtrList[i]->faceList.Count() == 1)
			{
				seams.Set(i,FALSE);
			}
		}
	}
	

	
	Tab<int> geomToUVVerts;
	geomToUVVerts.SetCount(tvData->geomPoints.Count());
	for (int i = 0; i < tvData->geomPoints.Count(); i++)
		geomToUVVerts[i] = -1;


	for (int i = 0; i < tvData->f.Count(); i++)
	{
		if (mFSel[i])//tvData->f[i]->flags & FLAG_SELECTED)
		{
			int deg = tvData->f[i]->count;
			for (int j = 0; j < deg; j++)
			{
				int tvA = tvData->f[i]->t[j];
				int geoA = tvData->f[i]->v[j];
				geomToUVVerts[geoA] = tvA;
			}
		}
	}
	for (int i = 0; i < tvData->ePtrList.Count(); i++)
	{
		tvData->ePtrList[i]->lookupIndex = i;
	}
	//convert our geom edges to uv edges
	BitArray newUVEdgeSel;
	newUVEdgeSel.SetSize(tvData->ePtrList.Count());
	newUVEdgeSel.ClearAll();

	int numberSeams = seams.NumberSet();

	for (int i = 0; i < seams.GetSize(); i++)
	{
		if (seams[i])
		{
			int ga = tvData->gePtrList[i]->a;
			int gb = tvData->gePtrList[i]->b;
			//convert to uv indices
			int uvA = geomToUVVerts[ga];
			int uvB = geomToUVVerts[gb];
			//find matching UV edge 
			int uvEdge = -1;
			int ct = tvData->e[uvA]->data.Count();
			for (int j = 0; j < ct; j++)
			{
				int matchA = tvData->e[uvA]->data[j]->a;
				int matchB = tvData->e[uvA]->data[j]->b;
				if ( ((matchA == uvA) && (matchB == uvB)) ||
					 ((matchA == uvB) && (matchB == uvA)) )
				{
					BOOL selected = FALSE;

					uvEdge = tvData->e[uvA]->data[j]->lookupIndex;
				}
			}
			if (uvEdge == -1)
			{
				ct = tvData->e[uvB]->data.Count();
				for (int j = 0; j < ct; j++)
				{
					int matchA = tvData->e[uvB]->data[j]->a;
					int matchB = tvData->e[uvB]->data[j]->b;
					if ( ((matchA == uvA) && (matchB == uvB)) ||
						((matchA == uvB) && (matchB == uvA)) )
					{
						uvEdge = tvData->e[uvB]->data[j]->lookupIndex;
					}
				}
			}
			if (uvEdge != -1) 
			{
				newUVEdgeSel.Set(uvEdge,TRUE);
			}
			else
			{
			}
		}
	}

	//do a  break now
	TVMaps.SplitUVEdges(newUVEdgeSel);
	

	mESel.SetSize(TVMaps.ePtrList.Count());
	mESel.ClearAll();

}

void MeshTopoData::BuildSpringData(Tab<EdgeBondage> &springEdges, Tab<SpringClass> &verts, Point3 &rigCenter,Tab<RigPoint> &rigPoints, Tab<Point3> &initialPointData, float rigStrength)
{

	UVW_ChannelClass *tvData = &TVMaps;
	//get our face selection
	//for now we are building all of them
	//copy our geom data to the uvw data
	Tab<int> uvToGeom;
	uvToGeom.SetCount(tvData->v.Count());
	Box3 bounds;
	bounds.Init();
	springEdges.ZeroCount();


	verts.SetCount(tvData->v.Count());
	for (int i = 0; i < tvData->v.Count(); i++)
	{
		verts[i].pos = tvData->v[i].GetP();
		verts[i].vel = Point3(0.0f,0.0f,0.0f);
	}

	bounds.Init();
	for (int i = 0; i < tvData->f.Count(); i++)
	{
		if ((!(tvData->f[i]->flags & FLAG_DEAD)) && mFSel[i])//(tvData->f[i]->flags & FLAG_SELECTED))
		{
			int deg = tvData->f[i]->count;
			for (int j = 0; j < deg; j++)
			{
				int tvA = tvData->f[i]->t[j];
				bounds += tvData->v[tvA].GetP();
			}
		}
	}

	Point3 center = bounds.Center();
	//build our spring list
	for (int i = 0; i < tvData->ePtrList.Count(); i++)
	{
		//loop the through the edges
		int a = tvData->ePtrList[i]->a;
		int b = tvData->ePtrList[i]->b;
		int veca = tvData->ePtrList[i]->avec;
		int vecb = tvData->ePtrList[i]->bvec;
		BOOL isHidden = tvData->ePtrList[i]->flags & FLAG_HIDDENEDGEA;

		BOOL faceSelected = FALSE;
		for (int j = 0; j < tvData->ePtrList[i]->faceList.Count(); j++)
		{
			int faceIndex = tvData->ePtrList[i]->faceList[j];
			if ((!(tvData->f[faceIndex]->flags & FLAG_DEAD)) && mFSel[faceIndex])//(tvData->f[faceIndex]->flags & FLAG_SELECTED))
				faceSelected = TRUE;
		}

		if (faceSelected)
		{
			EdgeBondage sp;
			sp.v1 = a;
			sp.v2 = b;
			sp.vec1 = veca;
			sp.vec2 = vecb;
			float dist = Length(tvData->v[a].GetP()-tvData->v[b].GetP());
			sp.dist = dist;
			sp.str = 1.0f;
			sp.distPer = 1.0f;
			sp.isEdge = FALSE;
			sp.edgeIndex = i;
			springEdges.Append(1,&sp,5000);

			//add a spring for each edge
			//if edge is not visible find cross edge
			if ((isHidden) && (tvData->ePtrList[i]->faceList.Count() > 1))
			{
				//get face 1
				int a1,b1;
				a1 = -1;
				b1 = -1;

				int faceIndex = tvData->ePtrList[i]->faceList[0];

				if ((!(tvData->f[faceIndex]->flags & FLAG_DEAD)) && mFSel[faceIndex])//(tvData->f[faceIndex]->flags & FLAG_SELECTED))
				{
					int deg = tvData->f[faceIndex]->count;
					for (int j = 0; j < deg; j++)
					{
						int tvA = tvData->f[faceIndex]->t[j];
						if ((tvA != a) && (tvA != b))
							a1 = tvA;

					}

					//get face 2
					faceIndex = tvData->ePtrList[i]->faceList[1];
					deg = tvData->f[faceIndex]->count;
					for (int j = 0; j < deg; j++)
					{
						int tvA = tvData->f[faceIndex]->t[j];
						if ((tvA != a) && (tvA != b))
							b1 = tvA;
					}

					if ((a1 != -1) && (b1 != -1))
					{
						EdgeBondage sp;
						sp.v1 = a1;
						sp.v2 = b1;
						sp.vec1 = -1;
						sp.vec2 = -1;
						float dist = Length(tvData->v[a1].GetP()-tvData->v[b1].GetP());
						sp.dist = dist;
						sp.str = 1.0f;
						sp.distPer = 1.0f;
						sp.isEdge = FALSE;
						sp.edgeIndex = -1;
						springEdges.Append(1,&sp,5000);
					}
				}
			}
		}
	}

	//build our initial rig

	//find our edge verts
	//loop through our seams
	//build our outeredge list
	//check for multiple holes
	BitArray masterSeamList;
	masterSeamList.SetSize(tvData->ePtrList.Count());
	masterSeamList.ClearAll();



	Tab<int> tempBorderVerts;
	GetBorders(tempBorderVerts);
	Tab<int> borderVerts;
	for (int j = 0; j < tempBorderVerts.Count(); j++)
	{
		if (tempBorderVerts[j] == -1)
			j = tempBorderVerts.Count();
		else
		{
			borderVerts.Append(1,&tempBorderVerts[j],1000);
			DebugPrint("Border vert %d\n",tempBorderVerts[j]);
		}
	}

	int startVert = -1;
	Point3 initialVec = Point3(0.0f,0.0f,0.0f);
	rigCenter = center;
	float d = 0.0f;
	float rigSize = 0.0f;
	for (int i = 0; i < borderVerts.Count(); i++)
	{

		int currentVert = borderVerts[i];
		int nextVert = borderVerts[(i+1)%borderVerts.Count()];
		Point3 a = tvData->v[currentVert].GetP();
		Point3 b = tvData->v[nextVert].GetP();
		d += Length(a-b);
		if (Length(a-rigCenter) > rigSize)
			rigSize = Length(a-rigCenter);
		if (Length(b-rigCenter) > rigSize)
			rigSize = Length(b-rigCenter);
	}
	rigSize *= 2.0f;

	BOOL first = TRUE;
	float currentAngle = 0.0f;

	for (int i = 0; i < borderVerts.Count(); i++)
	{

		int currentVert = borderVerts[i];

		int nextVert = borderVerts[(i+1)%borderVerts.Count()];
		int prevVert = 0;
		if (i == 0)
			prevVert = borderVerts[borderVerts.Count()-1];
		else
			prevVert = borderVerts[i-1];



		//need to make sure we are going the right direction

		Point3 a = tvData->v[currentVert].GetP();
		Point3 b = tvData->v[nextVert].GetP();

		float l = Length(a-b);
		float per = l/d;
		float angle = PI * 2.0f * per;

		//rotate our current
		Matrix3 rtm(1);
		rtm.RotateZ(currentAngle);
		if (first)
		{
			first = FALSE;
			Point3 ta = a;
			a.z = 0.0f;
			Point3 tb = center;
			b.z = 0.0f;

			initialVec = Normalize(ta-tb);
		}
		Point3 v = initialVec;
		v = v * rtm;

		RigPoint rp;

		rp.p = v * rigSize+center;//Point3(0.5f,0.5f,0.0f);
		rp.d = Length(rp.p - tvData->v[currentVert].GetP());
		rp.index = currentVert;
		rp.neighbor = nextVert;
		rp.elen = l;
		rp.springIndex = springEdges.Count();
		rp.angle = currentAngle;

		EdgeBondage sp;
		sp.v1 = currentVert;

		int found = -1;
		BOOL append = TRUE;
		for (int m= 0; m <tvData->v.Count();m++)
		{
			if (tvData->v[m].IsDead() && !TVMaps.mSystemLockedFlag[m])//flags & FLAG_DEAD)
			{
				found =m;
				append = FALSE;
				m = tvData->v.Count();
			}
		}
		int newVertIndex = found;
		if (found == -1)
			newVertIndex = tvData->v.Count();


		UVW_TVVertClass newVert;
		newVert.SetP(rp.p);
		newVert.SetInfluence(0.0f);
		newVert.SetFlag(0);
		newVert.SetControlID(-1);
		rp.lookupIndex = newVertIndex;
		if (append)
		{
			tvData->v.Append(1,&newVert);
		}
		else
		{
			tvData->v[newVertIndex].SetP(newVert.GetP());
			tvData->v[newVertIndex].SetInfluence(newVert.GetInfluence());
			tvData->v[newVertIndex].SetFlag(newVert.GetFlag());
		}

		tvData->v[newVertIndex].SetRigPoint(TRUE);			

		rigPoints.Append(1,&rp,5000);

		SpringClass vd;
		vd.pos = tvData->v[newVertIndex].GetP();
		vd.vel = Point3(0.0f,0.0f,0.0f);
		vd.weight = 1.0f;

		if (append)
			verts.Append(1,&vd,5000);
		else
		{
			verts[newVertIndex] = vd;
		}


		sp.v2 = newVertIndex;
		sp.vec1 = -1;
		sp.vec2 = -1;
		//build our rig springs			
		sp.dist = rp.d * 0.1f;
		//				sp.dist = rigSize*0.1f;
		sp.str = rigStrength;
		sp.edgePos = rp.p;
		sp.edgeIndex = -1;
		sp.originalVertPos = tvData->v[currentVert].GetP();

		sp.distPer = 1.0f;
		sp.isEdge = TRUE;
		springEdges.Append(1,&sp,5000);
		currentAngle += angle;
	}


	initialPointData.SetCount(verts.Count());
	for (int i=0;i<verts.Count();i++)
	{
		verts[i].vel = Point3(0.0f,0.0f,0.0f);
		for (int j = 0; j < 6; j++)
		{
			verts[i].tempVel[j] = Point3(0.0f,0.0f,0.0f);
			verts[i].tempPos[j] = Point3(0.0f,0.0f,0.0f);
		}
		initialPointData[i] = tvData->v[i].GetP();
	}

	TVMaps.mSystemLockedFlag.SetSize(tvData->v.Count(),1);
}



void MeshTopoData::InitReverseSoftData()
{


	BitArray originalVSel(mVSel);

	mSketchBelongsToList.SetCount(TVMaps.v.Count());
	mOriginalPos.SetCount(TVMaps.v.Count());

	for (int i = 0; i < TVMaps.v.Count(); i++)
	{
		mSketchBelongsToList[i] = -1;
		mOriginalPos[i] = TVMaps.v[i].GetP();
	}

	for (int i = 0; i < TVMaps.v.Count(); i++)
	{
		if ((!originalVSel[i]) && (TVMaps.v[i].GetInfluence() > 0.0f))
		{
			int closest = -1;
			float closestDist = 0.0f;
			Point3 a = TVMaps.v[i].GetP();
			for (int j = 0; j < TVMaps.v.Count(); j++)
			{
				if (mVSel[j])
				{
					Point3 b = TVMaps.v[j].GetP();
					float dist = Length(a-b);
					if ((dist < closestDist) || (closest == -1))
					{
						closest = j;
						closestDist = dist;
					}
				}
			}
			if (closest != -1)
			{
				mSketchBelongsToList[i] = closest;
			}
		}
	}
	mVSel = originalVSel;



}
void MeshTopoData::ApplyReverseSoftData(UnwrapMod *mod)
{

	TimeValue t = GetCOREInterface()->GetTime();

	for (int i = 0; i < TVMaps.v.Count(); i++)
	{
		if (mSketchBelongsToList[i] >= 0)
		{
			Point3 accumVec(0.0f,0.0f,0.0f);
			int index = mSketchBelongsToList[i];
			Point3 vec = TVMaps.v[index].GetP() - mOriginalPos[index];
			accumVec += vec * TVMaps.v[i].GetInfluence();

			Point3 p = TVMaps.v[i].GetP() + accumVec;
			SetTVVert(t,i,p,mod);
		}
	}

}

BOOL MeshTopoData::HasIncomingFaceSelection()
{
	if ( mesh && (mesh->selLevel==MESH_FACE))
		return TRUE;
	else if ( mnMesh && (mnMesh->selLevel==MNM_SL_FACE))
		return TRUE;
	else if ( patch && (patch->selLevel==PATCH_PATCH))
		return TRUE;

	return FALSE;
}

void MeshTopoData::DisplayGeomEdge(GraphicsWindow *gw, int eindex, float size,BOOL thickOpenEdges, Color c)
{
	Point3 plist[3];
	int ga = GetGeomEdgeVert(eindex,0);//TVMaps.gePtrList[i]->a;
	int gb = GetGeomEdgeVert(eindex,1);//TVMaps.gePtrList[i]->b;
	plist[0] = GetGeomVert(gb);//TVMaps.geomPoints[gb];
	plist[1] = GetGeomVert(ga);//TVMaps.geomPoints[ga];

	int faceIndex = GetGeomEdgeConnectedFace(eindex,0);//TVMaps.gePtrList[i]->faceList[0]


	if (!patch)
	{
		if (thickOpenEdges)
			Draw3dEdge(gw,size, plist[0], plist[1], c);
		else gw->segment(plist,1);
	}
	else
	{

		
		Point3 avec,bvec;

		int faceIndex = this->GetGeomEdgeConnectedFace(eindex,0);
		int deg = GetFaceDegree(faceIndex);
		for (int i = 0; i < deg; i++)
		{
			int a = i;
			int b = (i+1)%deg;
			int pa = patch->patches[faceIndex].v[a];
			int pb = patch->patches[faceIndex].v[b];
			if ((pa == ga) && (pb == gb))
			{
				plist[0] = patch->verts[pa].p;
				plist[1] = patch->verts[pb].p;
				avec = patch->vecs[patch->patches[faceIndex].vec[a*2]].p;
				bvec = patch->vecs[patch->patches[faceIndex].vec[a*2+1]].p;
			}
			else if ((pb == ga) && (pa == gb))
			{
				plist[0] = patch->verts[pa].p;
				plist[1] = patch->verts[pb].p;

				avec = patch->vecs[patch->patches[faceIndex].vec[a*2]].p;
				bvec = patch->vecs[patch->patches[faceIndex].vec[a*2+1]].p;
			}
		}


		Spline3D sp;
		SplineKnot ka(KTYPE_BEZIER_CORNER,LTYPE_CURVE,plist[0],avec,avec);
		SplineKnot kb(KTYPE_BEZIER_CORNER,LTYPE_CURVE,plist[1],bvec,bvec);
		sp.NewSpline();
		sp.AddKnot(ka);
		sp.AddKnot(kb);
		sp.SetClosed(0);
		sp.InvalidateGeomCache();
		Point3 ip1,ip2;

		//										Draw3dEdge(gw,size, plist[0], plist[1], c);
		for (int k = 0; k < 8; k++)
		{
			float per = k/7.0f;
			ip1 = sp.InterpBezier3D(0, per);
			if ( k > 0)
			{
				if (thickOpenEdges)
					Draw3dEdge(gw,size, ip1, ip2, c);
				else
				{
					plist[0] = ip1;
					plist[1] = ip2;
					gw->segment(plist,1);
				}
			}
			ip2 = ip1;

		}
	}
}

void MeshTopoData::DisplayGeomEdge(GraphicsWindow *gw, int vertexA,int vecA,int vertexB,int vecB, float size, BOOL thick, Color c)
{
	Point3 plist[3];
	int ga = vertexA;
	int gb = vertexB;
	BOOL hasHandles = FALSE;
	if ((vecA != -1) && (vecB != -1))
		hasHandles = TRUE;

	plist[0] = GetGeomVert(gb);//TVMaps.geomPoints[gb];
	plist[1] = GetGeomVert(ga);//TVMaps.geomPoints[ga];


	if (!hasHandles)
	{
		if (thick)
			Draw3dEdge(gw,size, plist[0], plist[1], c);
		else gw->segment(plist,1);
	}
	else
	{
		int va = vecA;
		int vb = vecB;
		Point3 avec,bvec;
		avec = GetGeomVert(vb);//TVMaps.geomPoints[vb];
		bvec = GetGeomVert(va);//TVMaps.geomPoints[va];
		Spline3D sp;
		SplineKnot ka(KTYPE_BEZIER_CORNER,LTYPE_CURVE,plist[0],avec,avec);
		SplineKnot kb(KTYPE_BEZIER_CORNER,LTYPE_CURVE,plist[1],bvec,bvec);
		sp.NewSpline();
		sp.AddKnot(ka);
		sp.AddKnot(kb);
		sp.SetClosed(0);
		sp.InvalidateGeomCache();
		Point3 ip1,ip2;

		//										Draw3dEdge(gw,size, plist[0], plist[1], c);
		for (int k = 0; k < 8; k++)
		{
			float per = k/7.0f;
			ip1 = sp.InterpBezier3D(0, per);
			if ( k > 0)
			{
				if (thick)
					Draw3dEdge(gw,size, ip1, ip2, c);
				else
				{
					plist[0] = ip1;
					plist[1] = ip2;
					gw->segment(plist,1);
				}
			}
			ip2 = ip1;

		}
	}
}


BitArray MeshTopoData::GetIncomingFaceSelection()
{
	if ( mesh && (mesh->selLevel==MESH_FACE))
	{
		return mesh->faceSel;
	}
	else if ( mnMesh && (mnMesh->selLevel==MNM_SL_FACE))
	{
		BitArray s;
		mnMesh->getFaceSel(s);
		return s;
	}
	else if ( patch && (patch->selLevel==PATCH_PATCH))
	{
		return patch->patchSel;		
	}

	BitArray error;
	return error;
}

void MeshTopoData::WeldSelectedVerts( float sweldThreshold, UnwrapMod *mod)
{

	sweldThreshold = sweldThreshold * sweldThreshold;
	TimeValue t = GetCOREInterface()->GetTime();
	for (int m = 0; m < GetNumberTVVerts(); m++)//TVMaps.v.Count(); m++) 
	{
		if (GetTVVertSelected(m)&& (!GetTVVertDead(m)))//vsel[m])
		{
			Point3 p(0.0f,0.0f,0.0f);
			Point3 op(0.0f,0.0f,0.0f);
			p = GetTVVert(m);//GetPoint(ip->GetTime(),m);
			op = p;
			int ct = 0;
			int index = -1;
			for (int i=m+1; i< GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++) 
			{
				if ( GetTVVertSelected(i) && (!GetTVVertDead(i)))//(vsel[i]) && (!(TVMaps.v[i].flags & FLAG_DEAD)) )
					//	if (vsel[i]) 
				{
					Point3 np;

					np= GetTVVert(i);//GetPoint(ip->GetTime(),i);
					if (LengthSquared(np-op) < sweldThreshold)
					{
						p+= np;
						ct++;
						if (index == -1)
							index = m;
						DeleteTVVert(i,mod);//TVMaps.v[i].flags |= FLAG_DEAD;
					}
				}
			}

			if ((index == -1) || (ct == 0))
			{
				//			theHold.SuperCancel();
				//			theHold.Cancel();
				//			return;
			}
			else
			{
				ct++;
				p = p /(float)ct;

				SetTVVert(t,index,p,mod);
				for (int i = 0; i < GetNumberFaces(); i++)//TVMaps.f.Count(); i++) 
				{
					int pcount = 3;
					pcount = GetFaceDegree(i);//TVMaps.f[i]->count;
					for (int j=0; j<pcount; j++) 
					{
						int tvfIndex = GetFaceTVVert(i,j);//TVMaps.f[i]->t[j];
						Point3 np = GetTVVert(tvfIndex);//TVMaps.v[tvfIndex].p;
						if ((GetTVVertSelected(tvfIndex)/*vsel[tvfIndex]*/) && (LengthSquared(np-op) < sweldThreshold)) 
						{
							if (tvfIndex != index)
							{
								SetFaceTVVert(i,j,index);
								//									TVMaps.f[i]->t[j] = index;
							}

						}

						if ((GetFaceHasVectors(i)))// (TVMaps.f[i]->flags & FLAG_CURVEDMAPPING) &&	(TVMaps.f[i]->vecs) 
							//								)
						{
							tvfIndex = GetFaceTVHandle(i,j*2);//TVMaps.f[i]->vecs->handles[j*2];
							Point3 np = GetTVVert(tvfIndex);//TVMaps.v[tvfIndex].p;
							if ((GetTVVertSelected(tvfIndex)/*vsel[tvfIndex]*/) && (LengthSquared(np-op) < sweldThreshold)) 
							{
								if (tvfIndex != index)
								{
									SetFaceTVHandle(i,j*2,index);//	TVMaps.f[i]->vecs->handles[j*2] = index;
								}

							}

							tvfIndex =  GetFaceTVHandle(i,j*2+1);//TVMaps.f[i]->vecs->handles[j*2+1];
							np = GetTVVert(tvfIndex);//TVMaps.v[tvfIndex].p;
							if ((GetTVVertSelected(tvfIndex)/*vsel[tvfIndex]*/) && (LengthSquared(np-op) < sweldThreshold)) 
							{
								if (tvfIndex != index)
								{
									SetFaceTVHandle(i,j*2+1,index);//TVMaps.f[i]->vecs->handles[j*2+1] = index;
								}

							}
							if (GetFaceHasInteriors(i))//TVMaps.f[i]->flags & FLAG_INTERIOR)
							{
								tvfIndex = GetFaceTVInterior(i,j);//TVMaps.f[i]->vecs->interiors[j];
								np = GetTVVert(tvfIndex);//TVMaps.v[tvfIndex].p;
								if ((GetTVVertSelected(tvfIndex)/*vsel[tvfIndex]*/) && (LengthSquared(np-op) < sweldThreshold)) 
								{
									if (tvfIndex != index)
									{
										SetFaceTVInterior(i,j,index);//TVMaps.f[i]->vecs->interiors[j] = index;
									}

								}

							}


						}
					}
				}
			}
		}
	}
	SetTVEdgeInvalid();//TVMaps.edgesValid= FALSE;
}

void MeshTopoData::FixUpLockedFlags()
{
	TVMaps.mSystemLockedFlag.SetSize(GetNumberTVVerts());
	TVMaps.mSystemLockedFlag.SetAll();
	for (int i=0; i < GetNumberFaces(); i++) 
	{
		if (!GetFaceDead(i))
		{
			int pcount = 3;
			pcount = GetFaceDegree(i);
			//if it is patch with curve mapping
			for (int j=0; j<pcount; j++) 
			{
				int index = TVMaps.f[i]->t[j];
				TVMaps.mSystemLockedFlag.Clear(index);
				if ( GetFaceCurvedMaping(i) &&
					GetFaceHasVectors(i) )
				{
					index = GetFaceTVHandle(i,j*2);
					TVMaps.mSystemLockedFlag.Clear(index);
					index = GetFaceTVHandle(i,j*2+1);
					TVMaps.mSystemLockedFlag.Clear(index);
					if (GetFaceHasInteriors(i))
					{
						index = GetFaceTVInterior(i,j);
						TVMaps.mSystemLockedFlag.Clear(index);
					}
				}				
			}
		}
	}
}

void MeshTopoData::ConvertGeomEdgeSelectionToTV(BitArray geoSel, BitArray &uvwSel)
{

		Tab<GeomToTVEdges*> edgeInfo;
		edgeInfo.SetCount(GetNumberFaces()/*TVMaps.f.Count()*/);
		for (int i = 0; i < GetNumberFaces()/*TVMaps.f.Count()*/; i++)
		{
			edgeInfo[i] = new GeomToTVEdges();
			int deg = GetFaceDegree(i);//TVMaps.f[i]->count;
			edgeInfo[i]->edgeInfo.SetCount(deg);
			for (int j = 0; j < deg; j++)
			{
				edgeInfo[i]->edgeInfo[j].gIndex = -1;
				edgeInfo[i]->edgeInfo[j].tIndex = -1;
			}
		}
		//loop through the geo edges
		for (int i = 0; i < GetNumberGeomEdges();/*TVMaps.gePtrList.Count();*/ i++)
		{
			int numberOfFaces =  GetGeomEdgeNumberOfConnectedFaces(i);//TVMaps.gePtrList[i]->faceList.Count();
			for (int j = 0; j < numberOfFaces; j++)
			{
				int faceIndex = GetGeomEdgeConnectedFace(i,j);//TVMaps.gePtrList[i]->faceList[j];
				int a = GetGeomEdgeVert(i,0);//TVMaps.gePtrList[i]->a;
				int b = GetGeomEdgeVert(i,1);//TVMaps.gePtrList[i]->b;
				int ithEdge = FindGeomEdge(faceIndex,a,b);//TVMaps.f[faceIndex]->FindGeomEdge(a, b);
				edgeInfo[faceIndex]->edgeInfo[ithEdge].gIndex = i;

			}
		}

		//loop through the uv edges
		for (int i = 0; i < GetNumberTVEdges();/*TVMaps.ePtrList.Count();*/ i++)
		{
			int numberOfFaces =  GetTVEdgeNumberTVFaces(i);//TVMaps.ePtrList[i]->faceList.Count();
			for (int j = 0; j < numberOfFaces; j++)
			{
				int faceIndex = GetTVEdgeConnectedTVFace(i,j);//TVMaps.ePtrList[i]->faceList[j];
				int a = GetTVEdgeVert(i,0);//TVMaps.ePtrList[i]->a;
				int b = GetTVEdgeVert(i,1);//TVMaps.ePtrList[i]->b;
				int ithEdge = FindUVEdge(faceIndex, a, b);//TVMaps.f[faceIndex]->FindUVEdge(a, b);
				edgeInfo[faceIndex]->edgeInfo[ithEdge].tIndex = i;

			}
		}

		uvwSel = GetTVEdgeSelection();
		uvwSel.ClearAll();


		for (int i = 0; i < GetNumberFaces()/*TVMaps.f.Count()*/; i++)
		{

			int deg =  GetFaceDegree(i);//TVMaps.f[i]->count;
			for (int j = 0; j < deg; j++)
			{
				int gIndex = edgeInfo[i]->edgeInfo[j].gIndex;
				int tIndex = edgeInfo[i]->edgeInfo[j].tIndex;
				if ((gIndex != -1) &&(tIndex != -1))
				{
					if (geoSel[gIndex])//GetGeomEdgeSelected(gIndex)/*gesel[gIndex]*/)
					{
						int tva,tvb;
						tva = GetTVEdgeVert(tIndex,0);//TVMaps.ePtrList[tIndex]->a;
						tvb = GetTVEdgeVert(tIndex,1);//TVMaps.ePtrList[tIndex]->b;

						if (GetTVVertFrozen(tva) || GetTVVertFrozen(tvb))
//							(TVMaps.v[tva].flags & FLAG_FROZEN) ||
//							(TVMaps.v[tvb].flags & FLAG_FROZEN) )
						{
							uvwSel.Set(tIndex,FALSE);
							geoSel.Set(gIndex,FALSE);
//							SetGeomEdgeSelected(gIndex,FALSE);
//							esel.Set(tIndex,FALSE);
//							gesel.Set(gIndex,FALSE);
						}
						else 
							uvwSel.Set(tIndex,TRUE);
//							SetTVEdgeSelected(tIndex,TRUE);
//							esel.Set(tIndex,TRUE);
					}
				}

			}

		}

		for (int i = 0; i < GetNumberFaces()/*TVMaps.f.Count()*/; i++)
		{
			delete edgeInfo[i];
		}
}



void MeshTopoData::GetFaceTVPoints(int faceIndex, Tab<int> &vIndices)
{
	vIndices.SetCount(0);
	if (GetFaceDead(faceIndex))
		return;

	int degree = GetFaceDegree(faceIndex);
	for (int j = 0 ; j < degree; j++)
	{
		int tvIndex = GetFaceTVVert(faceIndex,j);

		vIndices.Append(1,&tvIndex,10000);
		if (GetFaceCurvedMaping(faceIndex))
		{
			if (GetFaceHasVectors(faceIndex))
			{

				int tvIndex = GetFaceTVHandle(faceIndex,j*2);
				vIndices.Append(1,&tvIndex,10000);
				tvIndex = GetFaceTVHandle(faceIndex,j*2+1);
				vIndices.Append(1,&tvIndex,10000);

				if (GetFaceHasInteriors(faceIndex))
				{
					int tvIndex = GetFaceTVInterior(faceIndex,j);
					vIndices.Append(1,&tvIndex,10000);
				}
			}
		}
	}
}
void MeshTopoData::GetFaceGeomPoints(int faceIndex, Tab<int> &vIndices)
{
	vIndices.SetCount(0);
	if (GetFaceDead(faceIndex))
		return;

	int degree = GetFaceDegree(faceIndex);
	for (int j = 0 ; j < degree; j++)
	{
		int geomIndex = GetFaceGeomVert(faceIndex,j);

		vIndices.Append(1,&geomIndex,10000);
		if (GetFaceCurvedMaping(faceIndex))
		{
			if (GetFaceHasVectors(faceIndex))
			{

				int geomIndex = GetFaceGeomHandle(faceIndex,j*2);
				vIndices.Append(1,&geomIndex,10000);
				geomIndex = GetFaceGeomHandle(faceIndex,j*2+1);
				vIndices.Append(1,&geomIndex,10000);

				if (GetFaceHasInteriors(faceIndex))
				{
					int geomIndex = GetFaceGeomInterior(faceIndex,j);
					vIndices.Append(1,&geomIndex,10000);
				}
			}
		}
	}
}


void MeshTopoData::GetFaceTVPoints(int faceIndex, BitArray &vIndices)
{	
	if (GetFaceDead(faceIndex))
		return;

	int degree = GetFaceDegree(faceIndex);
	for (int j = 0 ; j < degree; j++)
	{
		int tvIndex = GetFaceTVVert(faceIndex,j);

		vIndices.Set(tvIndex,TRUE);
		if (GetFaceCurvedMaping(faceIndex))
		{
			if (GetFaceHasVectors(faceIndex))
			{

				int tvIndex = GetFaceTVHandle(faceIndex,j*2);
				vIndices.Set(tvIndex,TRUE);
				tvIndex = GetFaceTVHandle(faceIndex,j*2+1);
				vIndices.Set(tvIndex,TRUE);

				if (GetFaceHasInteriors(faceIndex))
				{
					int tvIndex = GetFaceTVInterior(faceIndex,j);
					vIndices.Set(tvIndex,TRUE);
				}
			}
		}
	}
}
void MeshTopoData::GetFaceGeomPoints(int faceIndex, BitArray &vIndices)
{	
	if (GetFaceDead(faceIndex))
		return;

	int degree = GetFaceDegree(faceIndex);
	for (int j = 0 ; j < degree; j++)
	{
		int geomIndex = GetFaceGeomVert(faceIndex,j);

		vIndices.Set(geomIndex,TRUE);
		if (GetFaceCurvedMaping(faceIndex))
		{
			if (GetFaceHasVectors(faceIndex))
			{

				int geomIndex = GetFaceGeomHandle(faceIndex,j*2);
				vIndices.Set(geomIndex,TRUE);
				geomIndex = GetFaceGeomHandle(faceIndex,j*2+1);
				vIndices.Set(geomIndex,TRUE);

				if (GetFaceHasInteriors(faceIndex))
				{
					int geomIndex = GetFaceGeomInterior(faceIndex,j);
					vIndices.Set(geomIndex,TRUE);
				}
			}
		}
	}
}


void MeshTopoData::PreIntersect()
{

	mIntersectNorms.SetCount(GetNumberFaces());
	for (int i = 0; i < GetNumberFaces(); i++)
	{
		mIntersectNorms[i] = GetGeomFaceNormal(i);
	}

}

#define EPSILON   0.0001f

bool MeshTopoData::CheckFace(Ray ray, Point3 p1,Point3 p2,Point3 p3, Point3 n, float &at)
{
///	Point3 v0, v2;
	Point3  p, bry;
	float d, rn, a; 

	// See if the ray intersects the plane (backfaced)
	rn = DotProd(ray.dir,n);
	if (rn > -EPSILON) return false;

	// Use a point on the plane to find d
	Point3 v1 = p1;
	d = DotProd(v1,n);

	// Find the point on the ray that intersects the plane
	a = (d - DotProd(ray.p,n)) / rn;

	// Must be positive...
	if (a < 0.0f) return false;

	// Must be closer than the closest at so far
	if (at != -1.0f)
	{
		if (a > at) return false;
	}
	

	// The point on the ray and in the plane.
	p = ray.p + a*ray.dir;

	// Compute barycentric coords.
	bry = BaryCoords(p1,p2,p3,p);  // MaxAssert(bry.x + bry.y+ bry.z = 1.0) 

	// barycentric coordinates must sum to 1 and each component must
	// be in the range 0-1
	if (bry.x<0.0f || bry.y<0.0f || bry.z<0.0f ) return false;   // DS 3/8/97 this test is sufficient

	at    = a;
	return true;
}
bool  MeshTopoData::Intersect(Ray ray,bool checkFrontFaces,bool checkBackFaces, float &at)
{


	BOOL first = FALSE;
	at = -1.0f;
	int hitFace = -1;

	for (int i = 0; i < GetNumberFaces(); i++)
	{
		int deg = GetFaceDegree(i);
		Point3 n = mIntersectNorms[i];
		for (int j = 0; j < deg-2; j++)
		{
			
			Point3 p1, p2, p3;

	
			p1 = GetGeomVert(GetFaceGeomVert(i,0));
			p2 = GetGeomVert(GetFaceGeomVert(i,j+1));
			p3 = GetGeomVert(GetFaceGeomVert(i,j+2));

			if (checkFrontFaces && CheckFace(ray, p1,p2,p3,  n, at))
				hitFace = i;

			n = n*-1.0f;
			if (checkBackFaces && CheckFace(ray, p1,p2,p3,  n, at))
				hitFace = i;
		}
	}

	if (hitFace == -1)
		return false;
	else return true;
}
void MeshTopoData::PostIntersect()
{
	mIntersectNorms.SetCount(0);
}


void MeshTopoData::HitTestFaces(GraphicsWindow *gw, INode *node, HitRegion hr, Tab<int> &hitFaces)
{

	hitFaces.SetCount(0);


	TimeValue t = GetCOREInterface()->GetTime();
	


	DWORD limits = gw->getRndLimits();
	gw->setRndLimits((limits | GW_PICK) & ~GW_ILLUM | GW_BACKCULL);
	
	gw->clearHitCode();

	gw->setHitRegion(&hr);
	Matrix3 mat = node->GetObjectTM(t);
	gw->setTransform(mat);

	if (GetMesh())
	{
		


		SubObjHitList hitList;
		MeshSubHitRec *rec;	


		Mesh &mesh = *GetMesh();
		//				mesh.faceSel = ((MeshTopoData*)mc->localData)->faceSel;
		mesh.SubObjectHitTest(gw, gw->getMaterial(), &hr,
			SUBHIT_FACES|SUBHIT_SELSOLID, hitList);

		rec = hitList.First();
		while (rec) {
			int hitface = rec->index;
			hitFaces.Append(1,&hitface);

			rec = rec->Next();
		}
	}
	else if (GetPatch())
	{
		SubPatchHitList hitList;

		PatchMesh &patch = *GetPatch();
		patch.SubObjectHitTest(gw, gw->getMaterial(), &hr,
			SUBHIT_PATCH_PATCHES|SUBHIT_SELSOLID|SUBHIT_PATCH_IGNORE_BACKFACING, hitList);

		PatchSubHitRec *rec = hitList.First();
		while (rec) {
			int hitface = rec->index;
			hitFaces.Append(1,&hitface);
			rec = rec->Next();
		}

	}
	else if (GetMNMesh())
	{
		SubObjHitList hitList;
		MeshSubHitRec *rec;	


		MNMesh &mesh = *GetMNMesh();
		mesh.SubObjectHitTest(gw, gw->getMaterial(), &hr,
			SUBHIT_MNFACES|SUBHIT_SELSOLID, hitList);

		rec = hitList.First();

		while (rec) {
			int hitface = rec->index;
			hitFaces.Append(1,&hitface);
			rec = rec->Next();
		}
		

	}
	gw->setRndLimits(limits);

}

Box3 MeshTopoData::GetSelFaceBoundingBox()
{
	Box3 bounds;
	bounds.Init();
	for (int i = 0; i < GetNumberFaces(); i++) 
	{
		if (GetFaceSelected(i))
		{
			int degree = GetFaceDegree(i);
			for (int j = 0; j < degree; j++) 
			{
				int id = GetFaceGeomVert(i,j);//TVMaps.f[i]->v[j];
				bounds += GetGeomVert(id);
			}
		}
	}
	return bounds;
}

Box3 MeshTopoData::GetBoundingBox()
{
	Box3 bounds;
	bounds.Init();
	for (int i = 0; i < GetNumberFaces(); i++) 
	{
		int degree = GetFaceDegree(i);
		for (int j = 0; j < degree; j++) 
		{
			int id = GetFaceGeomVert(i,j);//TVMaps.f[i]->v[j];
			bounds += GetGeomVert(id);
		}
	}
	return bounds;
}

BOOL MeshTopoData::GetUserCancel()
{
//check to see if the user pressed esc or the system wants to end the thread
	SHORT iret = GetAsyncKeyState (VK_ESCAPE);

	if (iret==-32767) 
		mUserCancel = TRUE;

	return mUserCancel;
}
void MeshTopoData::SetUserCancel(BOOL cancel)
{
	mUserCancel = cancel;
}


class VertData
{
public:
	int mNextVertID;
	int mPrevVertID;	
	BOOL mIsBorderEdge;
};

class CornerData
{
public:
	Tab<VertData> mData;
};


void MeshTopoData::GetBorders(Tab<int> &borderVerts)
{
	borderVerts.SetCount(0);
	Tab<CornerData*> corner;
	int numVerts = GetNumberTVVerts();
	corner.SetCount(numVerts);
	for (int i = 0; i < numVerts; i++)
	{
		corner[i] = new CornerData();
	}


	int numFaces = GetNumberFaces();
	BitArray degenerateVert;
	degenerateVert.SetSize(GetNumberTVVerts());
//build a list of verts connected toverts in the face direction
	for (int i = 0; i < numFaces; i++)
	{
		int deg = GetFaceDegree(i);

		//make sure that the face is not degenerate
		BOOL degenerateFace = FALSE;
		degenerateVert.ClearAll();
		for (int j = 0; j < deg; j++)
		{
			int index = GetFaceTVVert(i,j); 
			if (degenerateVert[index])
				degenerateFace = TRUE;				
			degenerateVert.Set(index,TRUE);
		}

		if (!degenerateFace)
		{
			for (int j = 0; j < deg; j++)
			{
	//get all our openedges
				int prevID = (j-1);
				if (prevID < 0) prevID = deg-1;
				int nextID = (j+1);
				if (nextID >= deg) nextID = 0;
	//get the order verts connecting to verts
				VertData d;
				int id = GetFaceTVVert(i,j);
				nextID = GetFaceTVVert(i,nextID);
				prevID = GetFaceTVVert(i,prevID);

				d.mNextVertID = nextID;
				d.mPrevVertID = prevID;
				d.mIsBorderEdge = TRUE;
				corner[id]->mData.Append(1,&d,4);
			}
		}
	}

	//now compute our borders
	for (int i = 0; i < numVerts; i++)
	{
		int deg = corner[i]->mData.Count();
		for (int j = 0; j < deg; j++)
		{
			int nextVert = corner[i]->mData[j].mNextVertID;
			int nextVertDeg = corner[nextVert]->mData.Count();
			for (int k = 0; k < nextVertDeg; k++)
			{
				int checkVert = corner[nextVert]->mData[k].mNextVertID;
				//if we can reach back to the vert from another vert it is not
				//a border edge
				if (checkVert == i)
				{
					corner[nextVert]->mData[k].mIsBorderEdge = FALSE;
					corner[i]->mData[j].mIsBorderEdge = FALSE;
					k = nextVertDeg;
				}
			}
		}
	}

	BitArray processedVert;
	processedVert.SetSize(numVerts);
	processedVert.ClearAll();

	BitArray border;
	border.SetSize(numVerts);
	border.ClearAll();
//now build our borders
	for (int i = 0; i < numVerts; i++)
	{
		if (!processedVert[i])
		{
			if (IsTVVertVisible(i))
			{
				int currentVert = i;

				BOOL done = FALSE;
				int lastVert = -1;
				border.ClearAll();
				int startVert = i;
				while (!done)
				{
					int deg = corner[currentVert]->mData.Count();
					BOOL found = FALSE;
					int numberOfConnectingEdges = 0;
					int connectingEdgeID = -1;
					for (int j = 0; j < deg; j++)
					{
						if (corner[currentVert]->mData[j].mIsBorderEdge)
						{

							if (!processedVert[currentVert])
							{
								numberOfConnectingEdges++;
								connectingEdgeID = j;
								found = TRUE;							
							}
						}
					}

					if (found)
					{
						borderVerts.Append(1,&currentVert,1000);		
						if (numberOfConnectingEdges == 1)
							processedVert.Set(currentVert,TRUE);
						lastVert = currentVert;
						currentVert = corner[currentVert]->mData[connectingEdgeID].mNextVertID;
					}
					
					if (!found) 
						done = TRUE;
					if (startVert == currentVert)
						done = TRUE;
				}				
				int endID = -1;
				if (borderVerts.Count())
					borderVerts.Append(1,&endID,1000);
			}			
		}
	}

	for (int i = 0; i < numVerts; i++)
	{
		delete corner[i];
	}

}
