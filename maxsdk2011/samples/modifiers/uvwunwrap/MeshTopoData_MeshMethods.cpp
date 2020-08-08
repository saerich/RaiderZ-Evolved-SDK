/********************************************************************** *<
FILE: MeshTopoData_MeshMethids.cpp

DESCRIPTION: local mode data for the unwrap dealing with mesh objects

HISTORY: 9/25/2006
CREATED BY: Peter Watje




*>	Copyright (c) 2006, All Rights Reserved.
**********************************************************************/

#include "unwrap.h"

void MeshTopoData::SetCache(Mesh &mesh, int mapChannel)
{
	FreeCache();
	this->mesh = new Mesh(mesh);

	mFSelPrevious.SetSize(mesh.faceSel.GetSize());
	mFSelPrevious = mesh.faceSel;
	
	//see if has mapping

	if ( (mesh.selLevel==MESH_FACE) && (mesh.faceSel.NumberSet() == 0) ) 
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

	//get channel from mesh
	TVMaps.channel = mapChannel;

	//get from mesh based on cache
	TVFace *tvFace = mesh.mapFaces(mapChannel);
	Point3 *tVerts = mesh.mapVerts(mapChannel);
	if (mesh.selLevel!=MESH_FACE) 
	{
		//copy into our structs
		TVMaps.SetCountFaces(mesh.getNumFaces());
		TVMaps.v.SetCount(mesh.getNumMapVerts (mapChannel));

		mVSel.SetSize(mesh.getNumMapVerts (mapChannel));
		mVSel.ClearAll();

		TVMaps.geomPoints.SetCount(mesh.numVerts);

		for (int j=0; j<TVMaps.f.Count(); j++) 
		{
			TVMaps.f[j]->flags = 0;
			TVMaps.f[j]->t = new int[3];
			TVMaps.f[j]->v = new int[3];
			if (tvFace == NULL)
			{

				TVMaps.f[j]->t[0] = 0;
				TVMaps.f[j]->t[1] = 0;
				TVMaps.f[j]->t[2] = 0;
				TVMaps.f[j]->FaceIndex = j;
				TVMaps.f[j]->MatID = mesh.faces[j].getMatID();
				TVMaps.f[j]->flags = 0;
				TVMaps.f[j]->count = 3;
				for (int k = 0; k < 3; k++)
				{
					int index = mesh.faces[j].v[k];
					TVMaps.f[j]->v[k] = index;
					TVMaps.geomPoints[index] = mesh.verts[index];
				}

				if (!mesh.faces[j].getEdgeVis(0))
					TVMaps.f[j]->flags |= FLAG_HIDDENEDGEA;
				if (!mesh.faces[j].getEdgeVis(1))
					TVMaps.f[j]->flags |= FLAG_HIDDENEDGEB;
				if (!mesh.faces[j].getEdgeVis(2))
					TVMaps.f[j]->flags |= FLAG_HIDDENEDGEC;

			}
			else
			{
				TVMaps.f[j]->t[0] = tvFace[j].t[0];
				TVMaps.f[j]->t[1] = tvFace[j].t[1];
				TVMaps.f[j]->t[2] = tvFace[j].t[2];
				TVMaps.f[j]->FaceIndex = j;
				TVMaps.f[j]->MatID = mesh.faces[j].getMatID();


				TVMaps.f[j]->flags = 0;
				TVMaps.f[j]->count = 3;
				for (int k = 0; k < 3; k++)
				{
					int index = mesh.faces[j].v[k];
					TVMaps.f[j]->v[k] = index;
					TVMaps.geomPoints[index] = mesh.verts[index];
				}

				if (!mesh.faces[j].getEdgeVis(0))
					TVMaps.f[j]->flags |= FLAG_HIDDENEDGEA;
				if (!mesh.faces[j].getEdgeVis(1))
					TVMaps.f[j]->flags |= FLAG_HIDDENEDGEB;
				if (!mesh.faces[j].getEdgeVis(2))
					TVMaps.f[j]->flags |= FLAG_HIDDENEDGEC;

			}
		}
		for ( int j=0; j<TVMaps.v.Count(); j++) 
		{
			TVMaps.v[j].SetFlag(0);
			if (tVerts)
				TVMaps.v[j].SetP(tVerts[j]);
			else TVMaps.v[j].SetP(Point3(.0f,0.0f,0.0f));
			TVMaps.v[j].SetInfluence( 0.0f);
			TVMaps.v[j].SetControlID(-1);
		}
		if (tvFace == NULL) 
		{
			BuildInitialMapping(&mesh);
		}
		TVMaps.mSystemLockedFlag.SetSize(TVMaps.v.Count());
		TVMaps.mSystemLockedFlag.ClearAll();

	}
	else
	{
		//copy into our structs
		TVMaps.SetCountFaces(mesh.getNumFaces());
		TVMaps.v.SetCount(mesh.getNumMapVerts (mapChannel));

		mVSel.SetSize(mesh.getNumMapVerts (mapChannel));
		mVSel.ClearAll();

		TVMaps.geomPoints.SetCount(mesh.numVerts);


		for (int j=0; j<TVMaps.f.Count(); j++) 
		{
			TVMaps.f[j]->flags = 0;
			TVMaps.f[j]->t = new int[3];
			TVMaps.f[j]->v = new int[3];

			if (tvFace == NULL)
			{
				TVMaps.f[j]->t[0] = 0;
				TVMaps.f[j]->t[1] = 0;
				TVMaps.f[j]->t[2] = 0;
				TVMaps.f[j]->FaceIndex = j;
				TVMaps.f[j]->MatID = mesh.faces[j].getMatID();
				TVMaps.f[j]->count = 3;
				if (mesh.faceSel[j])
					TVMaps.f[j]->flags = 0;
				else TVMaps.f[j]->flags = FLAG_DEAD;
				for (int k = 0; k < 3; k++)
				{
					int index = mesh.faces[j].v[k];
					TVMaps.f[j]->v[k] = index;
					TVMaps.geomPoints[index] = mesh.verts[index];
				}

				if (!mesh.faces[j].getEdgeVis(0))
					TVMaps.f[j]->flags |= FLAG_HIDDENEDGEA;
				if (!mesh.faces[j].getEdgeVis(1))
					TVMaps.f[j]->flags |= FLAG_HIDDENEDGEB;
				if (!mesh.faces[j].getEdgeVis(2))
					TVMaps.f[j]->flags |= FLAG_HIDDENEDGEC;
			}
			else
			{
				TVMaps.f[j]->t[0] = tvFace[j].t[0];
				TVMaps.f[j]->t[1] = tvFace[j].t[1];
				TVMaps.f[j]->t[2] = tvFace[j].t[2];
				TVMaps.f[j]->FaceIndex = j;
				TVMaps.f[j]->MatID = mesh.faces[j].getMatID();
				TVMaps.f[j]->count = 3;
				if (mesh.faceSel[j])
					TVMaps.f[j]->flags = 0;
				else TVMaps.f[j]->flags = FLAG_DEAD;
				for (int k = 0; k < 3; k++)
				{
					int index = mesh.faces[j].v[k];
					TVMaps.f[j]->v[k] = index;
					TVMaps.geomPoints[index] = mesh.verts[index];
				}
				if (!mesh.faces[j].getEdgeVis(0))
					TVMaps.f[j]->flags |= FLAG_HIDDENEDGEA;
				if (!mesh.faces[j].getEdgeVis(1))
					TVMaps.f[j]->flags |= FLAG_HIDDENEDGEB;
				if (!mesh.faces[j].getEdgeVis(2))
					TVMaps.f[j]->flags |= FLAG_HIDDENEDGEC;


			}
		}


		for (int j=0; j<TVMaps.v.Count(); j++) 
		{
//			TVMaps.v[j].SystemLocked(TRUE);
			if (tVerts)
				TVMaps.v[j].SetP(tVerts[j]);
			else TVMaps.v[j].SetP(Point3(0.0f,0.0f,0.0f));
			//check if vertex for this face selected
			TVMaps.v[j].SetInfluence(0.0f);
			TVMaps.v[j].SetControlID(-1);

		}
		if (tvFace == NULL) 
		{
			BuildInitialMapping(&mesh);
		}

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

			}
		}
	}	
}




void MeshTopoData::BuildInitialMapping(Mesh *msh)
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
		}

	}
	Tab<int> indexList;

	int vCT = msh->numVerts;
	indexList.SetCount(vCT);
	BitArray usedIndex;
	usedIndex.SetSize(vCT);
	usedIndex.ClearAll();

	for (int i = 0; i < vCT; i++)
		indexList[i] = -1;

	for (int i = 0; i < TVMaps.f.Count(); i++)
	{
		if (!(TVMaps.f[i]->flags & FLAG_DEAD))
		{
			int pcount = 3;
			pcount = TVMaps.f[i]->count;
			for (int j = 0; j < pcount; j++)
			{
				usedIndex.Set(msh->faces[i].v[j]);
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
	mVSel.ClearAll();

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
				int a = msh->faces[i].v[j];
				index = indexList[a];
				TVMaps.f[i]->t[j] = index;
				Point3 uv(TVMaps.geomPoints[TVMaps.f[i]->v[j]].x/bbox.Width().x + 0.5f,
					     TVMaps.geomPoints[TVMaps.f[i]->v[j]].y/bbox.Width().y + 0.5f,
						 TVMaps.geomPoints[TVMaps.f[i]->v[j]].z/bbox.Width().z + 0.5f);


				TVMaps.v[index].SetP(uv);
				TVMaps.v[index].SetInfluence(0.f);
				TVMaps.v[index].SetFlag(0.f);
				TVMaps.v[index].SetControlID(-1);

			}

		}
	}
}

void MeshTopoData::ApplyMapping(Mesh &mesh, int mapChannel)
{


	if (!mesh.mapSupport(mapChannel)) 
	{
		// allocate texture verts. Setup tv faces into a parallel
		// topology as the regular faces
		if (mapChannel >= mesh.getNumMaps ()) 
			mesh.setNumMaps (mapChannel+1, TRUE);
		mesh.setMapSupport (mapChannel, TRUE);

		TVFace *tvFace = mesh.mapFaces(mapChannel);
		for (int k =0; k < mesh.numFaces;k++)
		{
			tvFace[k].t[0] = 0;
			tvFace[k].t[1] = 0;
			tvFace[k].t[2] = 0;
		}
	} 



	TVFace *tvFace = mesh.mapFaces(mapChannel);

	int tvFaceCount =  mesh.numFaces;

	if (mesh.selLevel!=MESH_FACE) 
	{
		//copy into mesh struct

		for (int k=0; k<tvFaceCount; k++) 
		{
			if (k < TVMaps.f.Count())
			{
				tvFace[k].t[0] = TVMaps.f[k]->t[0];
				tvFace[k].t[1] = TVMaps.f[k]->t[1];
				tvFace[k].t[2] = TVMaps.f[k]->t[2];
			}
			else 
			{
				tvFace[k].t[0] = 0;
				tvFace[k].t[1] = 0;
				tvFace[k].t[2] = 0;
			}
		}
		//match verts
		mesh.setNumMapVerts (mapChannel, TVMaps.v.Count());
		Point3 *tVerts = mesh.mapVerts(mapChannel);
		for ( int k=0; k<TVMaps.v.Count(); k++) 
		{
			tVerts[k] = GetTVVert(k);
		}


	}
	else
	{
		//copy into mesh struct
		//check if mesh has existing tv faces
		int offset = mesh.getNumMapVerts (mapChannel);
		int current = 0;
		for (int k=0; k<tvFaceCount; k++) 
		{
			//copy if face is selected
			if (mesh.faceSel[k]==1)
			{
				tvFace[k].t[0] = TVMaps.f[k]->t[0] + offset;
				tvFace[k].t[1] = TVMaps.f[k]->t[1] + offset;
				tvFace[k].t[2] = TVMaps.f[k]->t[2] + offset;
			}
		}
		//add our verts
		mesh.setNumMapVerts (mapChannel,TVMaps.v.Count()+offset,TRUE);
		Point3 *tVerts = mesh.mapVerts(mapChannel);
		for ( int k=0; k<TVMaps.v.Count(); k++) 
			tVerts[k+offset] = GetTVVert(k);



	}
	mesh.DeleteIsoMapVerts(mapChannel);

}

