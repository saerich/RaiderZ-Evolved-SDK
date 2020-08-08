/********************************************************************** *<
FILE: MeshTopoData_MNMeshMethods.cpp

DESCRIPTION: local mode data for the unwrap dealing with mnmesh objects

HISTORY: 9/25/2006
CREATED BY: Peter Watje




*>	Copyright (c) 2006, All Rights Reserved.
**********************************************************************/

#include "unwrap.h"

void MeshTopoData::SetCache(MNMesh &mesh, int mapChannel)
{
	FreeCache();
	this->mnMesh = new MNMesh(mesh);


	TVMaps.channel = mapChannel;

	BitArray s;
	mesh.getFaceSel(s);
	mesh.getFaceSel(mFSelPrevious);

	if ( (mesh.selLevel==MNM_SL_FACE) && (s == 0) ) 
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


	//get from mesh based on cahne
	int numMaps = mesh.MNum ();
	MNMapFace *tvFace=NULL;
	Point3 *tVerts = NULL;
	if (mapChannel >= numMaps) 
	{
	}
	else
	{
		tvFace = mesh.M(mapChannel)->f;
		tVerts = mesh.M(mapChannel)->v;
	}

	if (mesh.selLevel!=MNM_SL_FACE) 
	{
		//copy into our structs
		TVMaps.SetCountFaces(mesh.FNum());
		if (tVerts)
		{
			TVMaps.v.SetCount(mesh.M(mapChannel)->VNum());
			mVSel.SetSize(mesh.M(mapChannel)->VNum());
		}
		else
		{
			TVMaps.v.SetCount(mesh.VNum());
			mVSel.SetSize(mesh.VNum());
		}


		TVMaps.geomPoints.SetCount(mesh.VNum());

		for (int j=0; j<TVMaps.f.Count(); j++) 
		{
			TVMaps.f[j]->flags = 0;
			int fct;
			fct = mesh.f[j].deg;

			if (mesh.f[j].GetFlag(MN_DEAD)) fct = 0;

			TVMaps.f[j]->t = new int[fct];
			TVMaps.f[j]->v = new int[fct];
			if (tvFace == NULL)
			{
				for (int k=0; k < fct;k++)
					TVMaps.f[j]->t[k] = 0;
				TVMaps.f[j]->FaceIndex = j;
				TVMaps.f[j]->MatID = mesh.f[j].material;
				TVMaps.f[j]->flags = 0;
				TVMaps.f[j]->count = fct;
				for (int k = 0; k < fct; k++)
				{
					int index = mesh.f[j].vtx[k];
					TVMaps.f[j]->v[k] = index;
					TVMaps.geomPoints[index] = mesh.v[index].p;
				}
			}
			else
			{
				for (int k=0; k < fct;k++)
					TVMaps.f[j]->t[k] = tvFace[j].tv[k];
				TVMaps.f[j]->FaceIndex = j;
				TVMaps.f[j]->MatID = mesh.f[j].material;


				TVMaps.f[j]->flags = 0;
				TVMaps.f[j]->count = fct;
				for (int k = 0; k < fct; k++)
				{
					int index = mesh.f[j].vtx[k];
					TVMaps.f[j]->v[k] = index;
					TVMaps.geomPoints[index] = mesh.v[index].p;
				}


			}
		}
		for ( int j=0; j<TVMaps.v.Count(); j++) 
		{
			TVMaps.v[j].SetFlag(0);
			if (tVerts)
				TVMaps.v[j].SetP(tVerts[j]);
			else TVMaps.v[j].SetP(Point3(.0f,0.0f,0.0f));
			TVMaps.v[j].SetInfluence(0.0f);
			TVMaps.v[j].SetControlID(-1);
		}
		if (tvFace == NULL) BuildInitialMapping(&mesh);

		TVMaps.mSystemLockedFlag.SetSize(TVMaps.v.Count());
		TVMaps.mSystemLockedFlag.ClearAll();

	}
	else
	{
		//copy into our structs
		TVMaps.SetCountFaces(mesh.FNum());

		int tvVertCount = 0;
		if (mapChannel < numMaps) 
		{
			tvVertCount = mesh.M(mapChannel)->VNum();
		}

		TVMaps.v.SetCount(tvVertCount);

		mVSel.SetSize(tvVertCount);

		TVMaps.geomPoints.SetCount(mesh.VNum());

		BitArray bs;
		mesh.getFaceSel(bs);

		for (int j=0; j<TVMaps.f.Count(); j++) 
		{
			TVMaps.f[j]->flags = 0;

			int fct;
			fct = mesh.f[j].deg;

			if (mesh.f[j].GetFlag(MN_DEAD)) fct = 0;


			TVMaps.f[j]->t = new int[fct];
			TVMaps.f[j]->v = new int[fct];

			if (tvFace == NULL)
			{
				for (int k=0; k < fct;k++)
					TVMaps.f[j]->t[k] = 0;
				TVMaps.f[j]->FaceIndex = j;
				TVMaps.f[j]->MatID = mesh.f[j].material;
				TVMaps.f[j]->count = fct;
				if (bs[j])
					TVMaps.f[j]->flags = 0;
				else TVMaps.f[j]->flags = FLAG_DEAD;
				for (int k = 0; k < fct; k++)
				{
					int index = mesh.f[j].vtx[k];
					TVMaps.f[j]->v[k] = index;
					TVMaps.geomPoints[index] = mesh.v[index].p;
				}
			}
			else
			{
				for (int k=0; k < fct;k++)
					TVMaps.f[j]->t[k] = tvFace[j].tv[k];
				TVMaps.f[j]->FaceIndex = j;
				TVMaps.f[j]->MatID = mesh.f[j].material;
				TVMaps.f[j]->count = fct;
				if (bs[j])
					TVMaps.f[j]->flags = 0;
				else TVMaps.f[j]->flags = FLAG_DEAD;
				for ( int k = 0; k < fct; k++)
				{
					int index = mesh.f[j].vtx[k];
					TVMaps.f[j]->v[k] = index;
					TVMaps.geomPoints[index] = mesh.v[index].p;
				}

			}
		}



		for (int j=0; j<TVMaps.v.Count(); j++) 
		{
			if (tVerts)
				TVMaps.v[j].SetP(tVerts[j]);
			else TVMaps.v[j].SetP(Point3(0.0f,0.0f,0.0f));
			//check if vertex for this face selected
			TVMaps.v[j].SetInfluence(0.0f);
			TVMaps.v[j].SetControlID(-1);

		}
		if (tvFace == NULL) BuildInitialMapping(&mesh);

		TVMaps.mSystemLockedFlag.SetSize(TVMaps.v.Count());
		TVMaps.mSystemLockedFlag.SetAll();

		for (int j=0; j<TVMaps.f.Count(); j++) 
		{
			if (!(TVMaps.f[j]->flags & FLAG_DEAD))
			{
				int a;
				for (int k =0 ; k < TVMaps.f[j]->count;k++)
				{
					a = TVMaps.f[j]->t[k];
					TVMaps.v[a].SetFlag(0);
					TVMaps.mSystemLockedFlag.Set(a,FALSE);
				}
			}
		}
	}
}



void MeshTopoData::BuildInitialMapping(MNMesh *msh)
{
	//build bounding box
	Box3 bbox;
	bbox.Init();

	int vertCount = 0;
	//normalize the length width height
	for (int i = 0; i < TVMaps.f.Count(); i++)
	{
		int pcount = 3;
		pcount = TVMaps.f[i]->count;
		vertCount += pcount;
		for (int j = 0; j < pcount; j++)
		{
			bbox += TVMaps.geomPoints[TVMaps.f[i]->v[j]];
		}

	}

	vertCount = msh->numv;
	Tab<int> indexList;

	indexList.SetCount(vertCount);
	BitArray usedIndex;
	usedIndex.SetSize(vertCount);
	usedIndex.ClearAll();

	for (int i = 0; i < vertCount; i++)
		indexList[i] = -1;

	for (int i = 0; i < TVMaps.f.Count(); i++)
	{
		if (!(TVMaps.f[i]->flags & FLAG_DEAD))
		{
			int pcount = 3;
			pcount = TVMaps.f[i]->count;
			for (int j = 0; j < pcount; j++)
			{
				usedIndex.Set(msh->f[i].vtx[j]);
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
				int a = msh->f[i].vtx[j];
				index = indexList[a];
				TVMaps.f[i]->t[j] = index;
				Point3 uv(	TVMaps.geomPoints[TVMaps.f[i]->v[j]].x/bbox.Width().x + 0.5f,
					TVMaps.geomPoints[TVMaps.f[i]->v[j]].y/bbox.Width().y + 0.5f,
					TVMaps.geomPoints[TVMaps.f[i]->v[j]].z/bbox.Width().z + 0.5f );
				TVMaps.v[index].SetP(uv);
				TVMaps.v[index].SetInfluence(0.f);
				TVMaps.v[index].SetFlag(0);
				TVMaps.v[index].SetControlID(-1);

			}

		}
	}

}

void MeshTopoData::ApplyMapping(MNMesh &mesh, int mapChannel)
{
	// allocate texture verts. Setup tv faces into a parallel
	// topology as the regular faces
	int numMaps = mesh.MNum ();
	if (mapChannel >= numMaps) 
	{
		mesh.SetMapNum(mapChannel+1);
		mesh.InitMap(mapChannel);
	}

	MNMapFace *tvFace = mesh.M(mapChannel)->f;

	if (!tvFace)
	{
		mesh.InitMap(mapChannel);
		tvFace = mesh.M(mapChannel)->f;
	}

	int tvFaceCount =  mesh.FNum();

	if (mesh.selLevel!=MNM_SL_FACE) 
	{
		//copy into mesh struct

		for (int k=0; k<tvFaceCount; k++) 
		{
			if (k < TVMaps.f.Count())
			{
				for (int m = 0; m< TVMaps.f[k]->count; m++) 
					tvFace[k].tv[m] = TVMaps.f[k]->t[m];
			}
			else 
			{
				for (int m = 0; m< TVMaps.f[k]->count; m++) 
					tvFace[k].tv[m] = 0;
			}
		}
		//match verts
		mesh.M(mapChannel)->setNumVerts( TVMaps.v.Count());
		Point3 *tVerts = mesh.M(mapChannel)->v;
		for ( int k=0; k<TVMaps.v.Count(); k++) 
		{
			tVerts[k] = GetTVVert(k);
		}


	}
	else
	{
		//copy into mesh struct
		//check if mesh has existing tv faces
		int offset = mesh.M(mapChannel)->VNum();
		int current = 0;
		BitArray s;
		mesh.getFaceSel(s);
		for (int k=0; k<tvFaceCount; k++) 
		{
			//copy if face is selected
			if (s[k]==1)
			{
				for (int m = 0; m< TVMaps.f[k]->count; m++) 
					tvFace[k].tv[m] = TVMaps.f[k]->t[m] + offset;
			}
		}
		//add our verts
		for ( int k=0; k<TVMaps.v.Count(); k++) 
			mesh.M(mapChannel)->NewVert( GetTVVert(k));
	}
	mesh.M(mapChannel)->CollapseDeadVerts(mesh.f);
}