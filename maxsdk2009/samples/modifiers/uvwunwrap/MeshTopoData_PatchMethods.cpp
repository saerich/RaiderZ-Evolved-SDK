/********************************************************************** *<
FILE: MeshTopoData_PatchMethods.cpp
Methods.cpp

DESCRIPTION: local mode data for the unwrap dealing with patch objects

HISTORY: 9/25/2006
CREATED BY: Peter Watje




*>	Copyright (c) 2006, All Rights Reserved.
**********************************************************************/

#include "unwrap.h"
void MeshTopoData::SetCache(PatchMesh &patch, int mapChannel)
{
	FreeCache();
	this->patch = new PatchMesh(patch);
	//build TVMAP and edge data

	if ( (patch.selLevel==PATCH_PATCH) && (patch.patchSel.NumberSet() == 0) ) 
	{
		TVMaps.SetCountFaces(0);
		TVMaps.v.SetCount(0);
		TVMaps.FreeEdges();
		TVMaps.FreeGeomEdges();
		mVSel.SetSize(0);
		mESel.SetSize(0);
		mFSel.SetSize(0);
		mGESel.SetSize(0);
		mGVSel.SetSize(0);
		return;
	}


	//loop through all maps
	//get channel from mesh
	TVMaps.channel = mapChannel;


	//get from mesh based on cahne
	PatchTVert *tVerts = NULL;
	TVPatch *tvFace = NULL;
	if (!patch.getMapSupport(mapChannel))
	{
		patch.setNumMaps(mapChannel+1);
	}

	tVerts = patch.tVerts[mapChannel];
	tvFace = patch.tvPatches[mapChannel];




	if (patch.selLevel!=PATCH_PATCH ) 
	{
		//copy into our structs
		TVMaps.SetCountFaces(patch.getNumPatches());
		TVMaps.v.SetCount(patch.getNumMapVerts(mapChannel));

		mVSel.SetSize(patch.getNumMapVerts (mapChannel));

		TVMaps.geomPoints.SetCount(patch.getNumVerts()+patch.getNumVecs());

		for (int j=0; j<TVMaps.f.Count(); j++) 
		{
			TVMaps.f[j]->flags = 0;


			int pcount = 3;
			if (patch.patches[j].type == PATCH_QUAD) 
			{
				pcount = 4;
			}

			TVMaps.f[j]->t = new int[pcount];
			TVMaps.f[j]->v = new int[pcount];


			if (tvFace == NULL)
			{
				TVMaps.f[j]->t[0] = 0;
				TVMaps.f[j]->t[1] = 0;
				TVMaps.f[j]->t[2] = 0;
				if (pcount ==4) TVMaps.f[j]->t[3] = 0;
				TVMaps.f[j]->FaceIndex = j;
				TVMaps.f[j]->MatID = patch.getPatchMtlIndex(j);
				TVMaps.f[j]->flags = 0;
				TVMaps.f[j]->count = pcount;

				TVMaps.f[j]->vecs = NULL;
				UVW_TVVectorClass *tempv = NULL;
				//new an instance
				if (!(patch.patches[j].flags & PATCH_AUTO))
					TVMaps.f[j]->flags |= FLAG_INTERIOR;

				if (!(patch.patches[j].flags & PATCH_LINEARMAPPING))
				{
					tempv = new UVW_TVVectorClass();
					TVMaps.f[j]->flags |= FLAG_CURVEDMAPPING;
				}
				TVMaps.f[j]->vecs = tempv;	 

				for (int k = 0; k < pcount; k++)
				{
					int index = patch.patches[j].v[k];
					TVMaps.f[j]->v[k] = index;
					TVMaps.geomPoints[index] = patch.verts[index].p;
					//do handles and interiors
					//check if linear
					if (!(patch.patches[j].flags & PATCH_LINEARMAPPING))
					{
						//do geometric points
						index = patch.patches[j].interior[k];
						TVMaps.f[j]->vecs->vinteriors[k] =patch.getNumVerts()+index;
						index = patch.patches[j].vec[k*2];
						TVMaps.f[j]->vecs->vhandles[k*2] =patch.getNumVerts()+index;
						index = patch.patches[j].vec[k*2+1];
						TVMaps.f[j]->vecs->vhandles[k*2+1] =patch.getNumVerts()+index;
						// do texture points do't need to since they don't exist in this case

						TVMaps.f[j]->vecs->interiors[k] =0;								
						TVMaps.f[j]->vecs->handles[k*2] =0;
						TVMaps.f[j]->vecs->handles[k*2+1] =0;

					}

				}

			}
			else
			{
				TVMaps.f[j]->t[0] = tvFace[j].tv[0];
				TVMaps.f[j]->t[1] = tvFace[j].tv[1];
				TVMaps.f[j]->t[2] = tvFace[j].tv[2];
				if (pcount ==4) TVMaps.f[j]->t[3] = tvFace[j].tv[3];
				TVMaps.f[j]->FaceIndex = j;
				TVMaps.f[j]->MatID = patch.getPatchMtlIndex(j);

				TVMaps.f[j]->flags = 0;
				TVMaps.f[j]->count = pcount;


				TVMaps.f[j]->vecs = NULL;
				UVW_TVVectorClass *tempv = NULL;

				if (!(patch.patches[j].flags & PATCH_AUTO))
					TVMaps.f[j]->flags |= FLAG_INTERIOR;
				//new an instance
				if (!(patch.patches[j].flags & PATCH_LINEARMAPPING))
				{
					BOOL mapLinear = FALSE;		
					for (int tvCount = 0; tvCount < patch.patches[j].type*2; tvCount++)
					{
						if (tvFace[j].handles[tvCount] < 0) mapLinear = TRUE;
					}
					if (!(patch.patches[j].flags & PATCH_AUTO))
					{
						for (int tvCount = 0; tvCount < patch.patches[j].type; tvCount++)
						{
							if (tvFace[j].interiors[tvCount] < 0) mapLinear = TRUE;
						}
					}
					if (!mapLinear)
					{
						tempv = new UVW_TVVectorClass();
						TVMaps.f[j]->flags |= FLAG_CURVEDMAPPING;
					}
				}
				TVMaps.f[j]->vecs = tempv;	 


				if ((patch.selLevel==PATCH_PATCH ) && (patch.patchSel[j] == 0))
					TVMaps.f[j]->flags |= FLAG_DEAD;

				for (int k = 0; k < pcount; k++)
				{
					int index = patch.patches[j].v[k];
					TVMaps.f[j]->v[k] = index;
					TVMaps.geomPoints[index] = patch.verts[index].p;
					//							TVMaps.f[j].pt[k] = patch.verts[index].p;
					//do handles and interiors
					//check if linear
					if (TVMaps.f[j]->flags & FLAG_CURVEDMAPPING)
					{
						//do geometric points
						index = patch.patches[j].interior[k];
						TVMaps.f[j]->vecs->vinteriors[k] =patch.getNumVerts()+index;
						index = patch.patches[j].vec[k*2];
						TVMaps.f[j]->vecs->vhandles[k*2] =patch.getNumVerts()+index;
						index = patch.patches[j].vec[k*2+1];
						TVMaps.f[j]->vecs->vhandles[k*2+1] =patch.getNumVerts()+index;
						// do texture points do't need to since they don't exist in this case
						if (TVMaps.f[j]->flags & FLAG_INTERIOR)
						{
							index = tvFace[j].interiors[k];
							TVMaps.f[j]->vecs->interiors[k] =index;
						}
						index = tvFace[j].handles[k*2];
						TVMaps.f[j]->vecs->handles[k*2] =index;
						index = tvFace[j].handles[k*2+1];
						TVMaps.f[j]->vecs->handles[k*2+1] =index;

					}

				}



			}

		}
		for (int geomvecs =0; geomvecs < patch.getNumVecs(); geomvecs++)
		{
			TVMaps.geomPoints[geomvecs+patch.getNumVerts()] = patch.vecs[geomvecs].p;
		}

		for ( int j=0; j<TVMaps.v.Count(); j++) 
		{
			TVMaps.v[j].SetFlag(0);
			if (tVerts)
				TVMaps.v[j].SetP(tVerts[j]);
			else TVMaps.v[j].SetP(Point3(0.0f,0.0f,0.0f));
			TVMaps.v[j].SetInfluence(0.0f);
			TVMaps.v[j].SetControlID(-1);
		}
		if (tvFace == NULL) BuildInitialMapping(&patch);

		TVMaps.mSystemLockedFlag.SetSize(TVMaps.v.Count());
		TVMaps.mSystemLockedFlag.ClearAll();


	}
	else
	{

		//copy into our structs
		TVMaps.SetCountFaces(patch.getNumPatches());

		TVMaps.v.SetCount(patch.getNumMapVerts (mapChannel));

		mVSel.SetSize(patch.getNumMapVerts (mapChannel));

		TVMaps.geomPoints.SetCount(patch.getNumVerts()+patch.getNumVecs());


		for (int j=0; j<TVMaps.f.Count(); j++) 
		{
			TVMaps.f[j]->flags = 0;

			int pcount = 3;

			if (patch.patches[j].type == PATCH_QUAD) 
			{
				pcount = 4;
			}

			TVMaps.f[j]->t = new int[pcount];
			TVMaps.f[j]->v = new int[pcount];

			if (tvFace == NULL)
			{
				TVMaps.f[j]->t[0] = 0;
				TVMaps.f[j]->t[1] = 0;
				TVMaps.f[j]->t[2] = 0;
				if (pcount == 4) TVMaps.f[j]->t[3] = 0;
				TVMaps.f[j]->FaceIndex = j;
				TVMaps.f[j]->MatID = patch.patches[j].getMatID();
				if (patch.patchSel[j])
					TVMaps.f[j]->flags = 0;
				else TVMaps.f[j]->flags = FLAG_DEAD;
				TVMaps.f[j]->count = pcount;

				TVMaps.f[j]->vecs = NULL;
				UVW_TVVectorClass *tempv = NULL;

				if (!(patch.patches[j].flags & PATCH_AUTO))
					TVMaps.f[j]->flags |= FLAG_INTERIOR;

				//new an instance
				if (!(patch.patches[j].flags & PATCH_LINEARMAPPING))
				{
					tempv = new UVW_TVVectorClass();
					TVMaps.f[j]->flags |= FLAG_CURVEDMAPPING;
				}
				TVMaps.f[j]->vecs = tempv;	 


				for (int k = 0; k < pcount; k++)
				{
					int index = patch.patches[j].v[k];
					TVMaps.f[j]->v[k] = index;
					TVMaps.geomPoints[index] = patch.verts[index].p;
					//							TVMaps.f[j].pt[k] = patch.verts[index].p;
					//check if linear
					if (!(patch.patches[j].flags & PATCH_LINEARMAPPING))
					{
						//do geometric points
						index = patch.patches[j].interior[k];
						TVMaps.f[j]->vecs->vinteriors[k] =patch.getNumVerts()+index;
						index = patch.patches[j].vec[k*2];
						TVMaps.f[j]->vecs->vhandles[k*2] =patch.getNumVerts()+index;
						index = patch.patches[j].vec[k*2+1];
						TVMaps.f[j]->vecs->vhandles[k*2+1] =patch.getNumVerts()+index;
						// do texture points do't need to since they don't exist in this case

						TVMaps.f[j]->vecs->interiors[k] =0;								
						TVMaps.f[j]->vecs->handles[k*2] =0;
						TVMaps.f[j]->vecs->handles[k*2+1] =0;

					}

				}

			}
			else
			{
				TVMaps.f[j]->t[0] = tvFace[j].tv[0];
				TVMaps.f[j]->t[1] = tvFace[j].tv[1];
				TVMaps.f[j]->t[2] = tvFace[j].tv[2];
				if (pcount == 4) TVMaps.f[j]->t[3] = tvFace[j].tv[3];
				TVMaps.f[j]->FaceIndex = j;
				TVMaps.f[j]->MatID = patch.patches[j].getMatID();


				if (patch.patchSel[j])
					TVMaps.f[j]->flags = 0;
				else TVMaps.f[j]->flags = FLAG_DEAD;

				int pcount = 3;
				if (patch.patches[j].type == PATCH_QUAD) 
				{
					pcount = 4;
				}
				TVMaps.f[j]->count = pcount;

				TVMaps.f[j]->vecs = NULL;
				UVW_TVVectorClass *tempv = NULL;
				if (!(patch.patches[j].flags & PATCH_AUTO))
					TVMaps.f[j]->flags |= FLAG_INTERIOR;
				//new an instance
				if (!(patch.patches[j].flags & PATCH_LINEARMAPPING))
				{
					BOOL mapLinear = FALSE;		
					for (int tvCount = 0; tvCount < patch.patches[j].type*2; tvCount++)
					{
						if (tvFace[j].handles[tvCount] < 0) mapLinear = TRUE;
					}
					if (!(patch.patches[j].flags & PATCH_AUTO))
					{
						for (int tvCount = 0; tvCount < patch.patches[j].type; tvCount++)
						{
							if (tvFace[j].interiors[tvCount] < 0) mapLinear = TRUE;
						}
					}
					if (!mapLinear)
					{
						tempv = new UVW_TVVectorClass();
						TVMaps.f[j]->flags |= FLAG_CURVEDMAPPING;
					}
				}
				TVMaps.f[j]->vecs = tempv;	 


				for (int k = 0; k < pcount; k++)
				{
					int index = patch.patches[j].v[k];
					TVMaps.f[j]->v[k] = index;
					TVMaps.geomPoints[index] = patch.verts[index].p;
					//							TVMaps.f[j].pt[k] = patch.verts[index].p;
					//do handles and interiors
					//check if linear
					if (TVMaps.f[j]->flags & FLAG_CURVEDMAPPING)
					{
						//do geometric points
						index = patch.patches[j].interior[k];
						TVMaps.f[j]->vecs->vinteriors[k] =patch.getNumVerts()+index;
						index = patch.patches[j].vec[k*2];
						TVMaps.f[j]->vecs->vhandles[k*2] =patch.getNumVerts()+index;
						index = patch.patches[j].vec[k*2+1];
						TVMaps.f[j]->vecs->vhandles[k*2+1] =patch.getNumVerts()+index;
						// do texture points do't need to since they don't exist in this case
						if (TVMaps.f[j]->flags & FLAG_INTERIOR)
						{
							index = tvFace[j].interiors[k];
							TVMaps.f[j]->vecs->interiors[k] =index;
						}
						index = tvFace[j].handles[k*2];
						TVMaps.f[j]->vecs->handles[k*2] =index;
						index = tvFace[j].handles[k*2+1];
						TVMaps.f[j]->vecs->handles[k*2+1] =index;

					}
				}

			}
		}
		for (int j =0; j < patch.getNumVecs(); j++)
		{
			TVMaps.geomPoints[j+patch.getNumVerts()] = patch.vecs[j].p;
		}



		for (int j=0; j<TVMaps.v.Count(); j++) 
		{
//			TVMaps.v[j].SystemLocked(TRUE);
			if (tVerts)
				TVMaps.v[j].SetP(tVerts[j]);
			else TVMaps.v[j].SetP(Point3(.0f,0.0f,0.0f));
			//check if vertex for this face selected
			TVMaps.v[j].SetInfluence(0.0f);
			TVMaps.v[j].SetControlID(-1);

		}


		if (tvFace == NULL) BuildInitialMapping(&patch);
		TVMaps.mSystemLockedFlag.SetSize(TVMaps.v.Count());
		TVMaps.mSystemLockedFlag.SetAll();

		for (int j=0; j<TVMaps.f.Count(); j++) 
		{
			if (!(TVMaps.f[j]->flags & FLAG_DEAD))
			{
				
				int a;
				a = TVMaps.f[j]->t[0];
				TVMaps.v[a].SetFlag(0);
				TVMaps.mSystemLockedFlag.Set(a,FALSE);
				a = TVMaps.f[j]->t[1];
				TVMaps.v[a].SetFlag(0);
				TVMaps.mSystemLockedFlag.Set(a,FALSE);
				a = TVMaps.f[j]->t[2];
				TVMaps.v[a].SetFlag(0);
				TVMaps.mSystemLockedFlag.Set(a,FALSE);

				if (TVMaps.f[j]->count > 3)
				{
					a = TVMaps.f[j]->t[3];
					TVMaps.v[a].SetFlag(0);
					TVMaps.mSystemLockedFlag.Set(a,FALSE);
				}
				if ( (TVMaps.f[j]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[j]->vecs))
				{
					for (int m =0; m < TVMaps.f[j]->count; m++) 
					{
						int hid = TVMaps.f[j]->vecs->handles[m*2];
						TVMaps.v[hid].SetFlag(0) ;
						TVMaps.mSystemLockedFlag.Set(hid,FALSE);

						hid = TVMaps.f[j]->vecs->handles[m*2+1];
						TVMaps.v[hid].SetFlag(0) ;
						TVMaps.mSystemLockedFlag.Set(hid,FALSE);
					}
					if (TVMaps.f[j]->flags & FLAG_INTERIOR) 
					{
						for (int m =0; m < TVMaps.f[j]->count; m++) 
						{
							int iid = TVMaps.f[j]->vecs->interiors[m];
							TVMaps.v[iid].SetFlag(0);
							TVMaps.mSystemLockedFlag.Set(iid,FALSE);
						}

					}


				}
			}
		}

	}

}


void MeshTopoData::BuildInitialMapping(PatchMesh *msh)
{
	//build bounding box
	Box3 bbox;
	bbox.Init();
	//normalize the length width height
	for (int i = 0; i < TVMaps.f.Count(); i++)
	{
		int pcount = 3;
		pcount = TVMaps.f[i]->count;
		for (int j = 0; j < pcount; j++)
		{
			bbox += TVMaps.geomPoints[TVMaps.f[i]->v[j]];
			if (TVMaps.f[i]->flags & FLAG_CURVEDMAPPING)
			{
				if (TVMaps.f[i]->vecs)
				{
					bbox += TVMaps.geomPoints[TVMaps.f[i]->vecs->vhandles[j*2]];
					bbox += TVMaps.geomPoints[TVMaps.f[i]->vecs->vhandles[j*2+1]];
					if (TVMaps.f[i]->flags & FLAG_INTERIOR)
					{
						bbox += TVMaps.geomPoints[TVMaps.f[i]->vecs->vinteriors[j]];
					}
				}
			}

		}

	}
	Tab<int> indexList;

	int vct = msh->numVecs+msh->numVerts;
	indexList.SetCount(vct);
	BitArray usedIndex;
	usedIndex.SetSize(vct);
	usedIndex.ClearAll();

	for (int i = 0; i < vct; i++)
		indexList[i] = -1;

	for (int i = 0; i < TVMaps.f.Count(); i++)
	{
		if (!(TVMaps.f[i]->flags & FLAG_DEAD))
		{
			int pcount = 3;
			pcount = TVMaps.f[i]->count;

			for (int j = 0; j < pcount; j++)
			{
				//		usedIndex.Set(TVMaps.f[i].t[j]);
				usedIndex.Set(msh->patches[i].v[j]);
				if (TVMaps.f[i]->flags & FLAG_CURVEDMAPPING)
				{
					if (TVMaps.f[i]->vecs)
					{
						usedIndex.Set(msh->patches[i].vec[j*2]+msh->numVerts);
						usedIndex.Set(msh->patches[i].vec[j*2+1]+msh->numVerts);
						if (TVMaps.f[i]->flags & FLAG_INTERIOR)
						{
							usedIndex.Set(msh->patches[i].interior[j]+msh->numVerts);

						}
					}	
				}

			}
		}

	}

	int ct = 0;
	for (int i = 0; i < usedIndex.GetSize(); i++)
	{
		if (usedIndex[i])
			indexList[i] = ct++;

	}

	TVMaps.v.SetCount(usedIndex.NumberSet());
	mVSel.SetSize(usedIndex.NumberSet());

	//watje 10-19-99 bug 213437  to prevent a divide by 0 which gives you a huge u,v, or w value
	if (bbox.Width().x == 0.0f) bbox += Point3(0.5f,0.0f,0.0f);
	if (bbox.Width().y == 0.0f) bbox += Point3(0.0f,0.5f,0.0f);
	if (bbox.Width().z == 0.0f) bbox += Point3(0.0f,0.0f,0.5f);

	for (int i = 0; i < TVMaps.f.Count(); i++)
	{
		if (!(TVMaps.f[i]->flags & FLAG_DEAD))
		{
			int pcount = 3;
			pcount = TVMaps.f[i]->count;

			TVMaps.f[i]->flags &= ~FLAG_DEAD;
			for (int j = 0; j < pcount; j++)
			{
				int index;
				int a = msh->patches[i].v[j];
				index = indexList[a];
				TVMaps.f[i]->t[j] = index;
				Point3 uv(	TVMaps.geomPoints[TVMaps.f[i]->v[j]].x/bbox.Width().x + 0.5f,
							TVMaps.geomPoints[TVMaps.f[i]->v[j]].y/bbox.Width().y + 0.5f,
							TVMaps.geomPoints[TVMaps.f[i]->v[j]].z/bbox.Width().z + 0.5f);

				TVMaps.v[index].SetP(uv);
				TVMaps.v[index].SetInfluence(0.f);
				TVMaps.v[index].SetFlag(0);
				TVMaps.v[index].SetControlID(-1);
				if (TVMaps.f[i]->flags & FLAG_CURVEDMAPPING)
				{
					if (TVMaps.f[i]->vecs)
					{
						//					usedIndex.Set(msh->patches[i].vec[j*2]+msh->numVerts);
						//					usedIndex.Set(msh->patches[i].vec[j*2+1]+msh->numVerts);
						int index;
						int a = msh->patches[i].vec[j*2]+msh->numVerts;
						index = indexList[a];
						TVMaps.f[i]->vecs->handles[j*2] = index;
						Point3 uv(	TVMaps.geomPoints[TVMaps.f[i]->vecs->vhandles[j*2]].x/bbox.Width().x + 0.5f,
									TVMaps.geomPoints[TVMaps.f[i]->vecs->vhandles[j*2]].y/bbox.Width().y + 0.5f,
									TVMaps.geomPoints[TVMaps.f[i]->vecs->vhandles[j*2]].z/bbox.Width().z + 0.5f);
									
						TVMaps.v[index].SetP(uv);  
						TVMaps.v[index].SetInfluence(0.f);
						TVMaps.v[index].SetFlag(0);
						TVMaps.v[index].SetControlID(-1);

						a = msh->patches[i].vec[j*2+1]+msh->numVerts;
						index = indexList[a];
						TVMaps.f[i]->vecs->handles[j*2+1] = index;
						uv.x = TVMaps.geomPoints[TVMaps.f[i]->vecs->vhandles[j*2+1]].x/bbox.Width().x + 0.5f;
						uv.y = TVMaps.geomPoints[TVMaps.f[i]->vecs->vhandles[j*2+1]].y/bbox.Width().y + 0.5f;
						uv.z = TVMaps.geomPoints[TVMaps.f[i]->vecs->vhandles[j*2+1]].z/bbox.Width().z + 0.5f;
						TVMaps.v[index].SetP(uv);  
						TVMaps.v[index].SetInfluence(0.f);
						TVMaps.v[index].SetFlag(0);
						TVMaps.v[index].SetControlID(-1);


						if (TVMaps.f[i]->flags & FLAG_INTERIOR)
						{
							int index;
							int a = msh->patches[i].interior[j]+msh->numVerts;
							index = indexList[a];
							TVMaps.f[i]->vecs->interiors[j] = index;
							uv.x = TVMaps.geomPoints[TVMaps.f[i]->vecs->vinteriors[j]].x/bbox.Width().x + 0.5f;
							uv.y = TVMaps.geomPoints[TVMaps.f[i]->vecs->vinteriors[j]].y/bbox.Width().y + 0.5f;
							uv.z = TVMaps.geomPoints[TVMaps.f[i]->vecs->vinteriors[j]].z/bbox.Width().z + 0.5f;
							TVMaps.v[index].SetP(uv);  									
							TVMaps.v[index].SetInfluence(0.f);
							TVMaps.v[index].SetFlag(0);
							TVMaps.v[index].SetControlID(-1);

						}
					}	
				}
			}
		}

	}


}

void MeshTopoData::ApplyMapping(PatchMesh &patch, int mapChannel)
{





	//get from mesh 

	if (!patch.getMapSupport(mapChannel) )
	{
		patch.setNumMaps (mapChannel+1);

	}

	TVPatch *tvFace = patch.tvPatches[mapChannel];


	int tvFaceCount =  patch.numPatches;

	if (patch.selLevel!=PATCH_PATCH) 
	{
		//copy into mesh struct
		if (!tvFace) 
		{
			// Create tvfaces and init to 0
			patch.setNumMapPatches(mapChannel,patch.getNumPatches());
			tvFace = patch.tvPatches[mapChannel];
			for (int k=0; k<patch.getNumPatches(); k++)
			{	
				for (int j=0; j<TVMaps.f[k]->count; j++) 
				{
					tvFace[k].tv[j] = 0;			
					tvFace[k].interiors[j] = 0;			
					tvFace[k].handles[j*2] = 0;			
					tvFace[k].handles[j*2+1] = 0;			
				}
			}
		}
		for (int k=0; k<tvFaceCount; k++) 
		{
			if (k < TVMaps.f.Count())
			{
				tvFace[k].tv[0] = TVMaps.f[k]->t[0];
				tvFace[k].tv[1] = TVMaps.f[k]->t[1];
				tvFace[k].tv[2] = TVMaps.f[k]->t[2];
				if (TVMaps.f[k]->count == 4) tvFace[k].tv[3] = TVMaps.f[k]->t[3];
				if (TVMaps.f[k]->flags & FLAG_CURVEDMAPPING)
				{
					patch.patches[k].flags &= ~PATCH_LINEARMAPPING;
					if (TVMaps.f[k]->vecs)
					{
						for (int m = 0;m < TVMaps.f[k]->count;m++)
						{
							if (TVMaps.f[k]->flags & FLAG_INTERIOR)
							{
								tvFace[k].interiors[m] = TVMaps.f[k]->vecs->interiors[m];			
							}

							tvFace[k].handles[m*2] = TVMaps.f[k]->vecs->handles[m*2];			
							tvFace[k].handles[m*2+1] = TVMaps.f[k]->vecs->handles[m*2+1];			
						}
					}

				}
				else patch.patches[k].flags |= PATCH_LINEARMAPPING;
			}
			else{
				tvFace[k].tv[0] = 0;
				tvFace[k].tv[1] = 0;
				tvFace[k].tv[2] = 0;
				if (TVMaps.f[k]->count == 4) tvFace[k].tv[3] = 0;
				for (int m = 0;m < TVMaps.f[k]->count;m++)
				{
					tvFace[k].interiors[m] = 0;			
					tvFace[k].handles[m*2] = 0;			
					tvFace[k].handles[m*2+1] = 0;			
				}

			}
		}
		//match verts
		patch.setNumMapVerts (mapChannel,TVMaps.v.Count());
		PatchTVert *tVerts = patch.tVerts[mapChannel];
		for (int k=0; k<TVMaps.v.Count(); k++) 
			tVerts[k].p = GetTVVert(k);
	}
	else
	{
		//copy into mesh struct
		if (!tvFace) 
		{
			// Create tvfaces and init to 0
			patch.setNumMapPatches (mapChannel,patch.getNumPatches());
			tvFace = patch.tvPatches[mapChannel];
			for (int k=0; k<patch.getNumPatches(); k++)
			{	
				for (int j=0; j<TVMaps.f[k]->count; j++) 
				{
					tvFace[k].tv[j] = 0;			
					tvFace[k].interiors[j] = 0;			
					tvFace[k].handles[j*2] = 0;			
					tvFace[k].handles[j*2+1] = 0;			

				}
			}
		}
		int offset = patch.getNumMapVerts (mapChannel);
		int current = 0;
		for (int k=0; k<tvFaceCount; k++) 
		{
			//copy if face is selected
			if (patch.patchSel[k])
			{
				tvFace[k].tv[0] = TVMaps.f[k]->t[0]+offset;
				tvFace[k].tv[1] = TVMaps.f[k]->t[1]+offset;
				tvFace[k].tv[2] = TVMaps.f[k]->t[2]+offset;
				if (TVMaps.f[k]->count == 4) tvFace[k].tv[3] = TVMaps.f[k]->t[3]+offset;
				if (TVMaps.f[k]->flags & FLAG_CURVEDMAPPING)
				{
					patch.patches[k].flags &= ~PATCH_LINEARMAPPING;
					if (TVMaps.f[k]->vecs)
					{
						for (int m = 0;m < TVMaps.f[k]->count;m++)
						{
							if (TVMaps.f[k]->flags & FLAG_INTERIOR)
							{
								tvFace[k].interiors[m] = TVMaps.f[k]->vecs->interiors[m]+offset;			
							}
							tvFace[k].handles[m*2] = TVMaps.f[k]->vecs->handles[m*2]+offset;			
							tvFace[k].handles[m*2+1] = TVMaps.f[k]->vecs->handles[m*2+1]+offset;			
						}
					}

				}
				else patch.patches[k].flags |= PATCH_LINEARMAPPING;
			}
		}
		//match verts
		patch.setNumMapVerts (mapChannel,TVMaps.v.Count()+offset,TRUE);
		PatchTVert *tVerts = patch.tVerts[mapChannel];
		for ( int k=0; k<TVMaps.v.Count(); k++) 
			tVerts[k+offset].p = GetTVVert(k);

	}
	RemoveDeadVerts(&patch,mapChannel);
}

void MeshTopoData::RemoveDeadVerts(PatchMesh *mesh, int channel)
{
	Tab<Point3> vertList;
	Tab<int> idList;
	//copy over vertlist

	int ct = mesh->getNumMapVerts(channel);
	vertList.SetCount(ct);
	PatchTVert *tVerts = mesh->mapVerts(channel);
	for (int i = 0; i < ct; i++)
		vertList[i] = tVerts[i].p;

	BitArray usedList;
	usedList.SetSize(ct);
	TVPatch *tvFace = NULL;
	if (!mesh->getMapSupport(channel))
	{
		return;
	}

	tvFace = mesh->tvPatches[channel];
	if (tvFace == NULL) return;

	for (int i =0; i < mesh->numPatches; i++)
	{
		int pcount = 3;
		if (mesh->patches[i].type == PATCH_QUAD) pcount = 4;

		for (int j = 0; j < pcount; j++)
		{
			int index = tvFace[i].tv[j];
			usedList.Set(index);
			if (!(mesh->patches[i].flags & PATCH_LINEARMAPPING))
			{
				if (!(mesh->patches[i].flags & PATCH_AUTO))
				{
					index = tvFace[i].interiors[j];
					if ((index >= 0) && (index < usedList.GetSize())) usedList.Set(index);
				}
				index = tvFace[i].handles[j*2];
				if ((index >= 0) && (index < usedList.GetSize())) usedList.Set(index);
				index = tvFace[i].handles[j*2+1];
				if ((index >= 0) && (index < usedList.GetSize())) usedList.Set(index);

			}
		}
	}
	mesh->setNumMapVerts (channel,usedList.NumberSet(),TRUE);

	int current = 0;
	tVerts = mesh->mapVerts(channel);

	for (int i = 0; i < ct; i++)
	{
		if (usedList[i])
		{
			tVerts[current].p = vertList[i];
			//now fix up faces
			for (int j = 0; j < mesh->numPatches; j++)
			{
				int pcount = 3;
				if (mesh->patches[j].type == PATCH_QUAD) pcount = 4;

				for (int k = 0; k < pcount; k++)
				{
					int index = tvFace[j].tv[k];
					if (index == i)
					{
						tvFace[j].tv[k] = current;
					}

					index = tvFace[j].interiors[k];
					if ((index >=0) && (index == i))
					{
						tvFace[j].interiors[k] = current;
					}
					index = tvFace[j].handles[k*2];
					if ((index >=0) && (index == i))
					{
						tvFace[j].handles[k*2] = current;
					}
					index = tvFace[j].handles[k*2+1];
					if ((index >=0) && (index == i))
					{
						tvFace[j].handles[k*2+1] = current;
					}


				}

			}
			current++;
		}
}

}
