
/********************************************************************** *<
FILE: MeshTopoData.cpp

DESCRIPTION: local mode data for the unwrap dealing wiht mapping

HISTORY: 9/25/2006
CREATED BY: Peter Watje




*>	Copyright (c) 2006, All Rights Reserved.
**********************************************************************/

#include "unwrap.h"

void MeshTopoData::DetachFromGeoFaces(BitArray faceSel, BitArray &vertSel, UnwrapMod *mod)
{


	TimeValue t = GetCOREInterface()->GetTime();

	//loop through the geo vertices and create our geo list of all verts in the 
	Tab<int> geoPoints;

	BitArray usedGeoPoints;
	usedGeoPoints.SetSize(TVMaps.geomPoints.Count());
	usedGeoPoints.ClearAll();

	BitArray isolatedPoints;
	isolatedPoints.SetSize(TVMaps.v.Count());
	isolatedPoints.SetAll();

	for (int i = 0; i < TVMaps.f.Count(); i++)
	{
		if (!(TVMaps.f[i]->flags & FLAG_DEAD))
		{
			if (faceSel[i])
			{	
				int degree = TVMaps.f[i]->count;
				for (int j = 0; j < degree; j++)
				{
					int index = TVMaps.f[i]->v[j];
					if (!usedGeoPoints[index])
					{
						usedGeoPoints.Set(index);
						geoPoints.Append(1,&index,3000);
					}
					if (TVMaps.f[i]->vecs)
					{
						int index = TVMaps.f[i]->vecs->vhandles[j*2];
						if (index != -1)
						{
							if (!usedGeoPoints[index])
							{
								usedGeoPoints.Set(index);
								geoPoints.Append(1,&index,3000);
							}
						}
						index = TVMaps.f[i]->vecs->vhandles[j*2+1];
						if (index != -1)
						{
							if (!usedGeoPoints[index])
							{
								usedGeoPoints.Set(index);
								geoPoints.Append(1,&index,3000);
							}
						}
						index = TVMaps.f[i]->vecs->vinteriors[j];
						if (index != -1)
						{
							if (!usedGeoPoints[index])
							{
								usedGeoPoints.Set(index);
								geoPoints.Append(1,&index,3000);
							}
						}
					}
				}
			}
			else 
			{	
				int degree = TVMaps.f[i]->count;
				for (int j = 0; j < degree; j++)
				{
					int index = TVMaps.f[i]->t[j];
					isolatedPoints.Clear(index);
					if (TVMaps.f[i]->vecs)
					{
						int index = TVMaps.f[i]->vecs->handles[j*2];					
						if (index != -1)
						{
							isolatedPoints.Clear(index);
						}
						index = TVMaps.f[i]->vecs->handles[j*2+1];
						if (index != -1)
						{
							isolatedPoints.Clear(index);
						}
						index = TVMaps.f[i]->vecs->interiors[j];
						if (index != -1)
						{
							isolatedPoints.Clear(index);
						}
					}
				}
			}
		}

	}

	for (int i = 0; i < TVMaps.v.Count(); i++)
	{
		if (isolatedPoints[i])
			SetTVVertDead(i,TRUE);
	}
	
	//get our dead verts
	Tab<int> deadVerts;
	for (int i = 0; i < TVMaps.v.Count(); i++)
	{
		if (TVMaps.v[i].IsDead() && !TVMaps.mSystemLockedFlag[i])
		{
			deadVerts.Append(1,&i,1000);
		}
	}

	//build the look up list
	Tab<int> lookupList;
	lookupList.SetCount(TVMaps.geomPoints.Count());
	for (int i = 0; i < TVMaps.geomPoints.Count(); i++)
		lookupList[i] = -1;
	int deadIndex = 0;
	for (int i = 0; i < geoPoints.Count(); i++)
	{
		int vIndex = geoPoints[i];
		Point3 p = TVMaps.geomPoints[vIndex];
	
		if (deadIndex < deadVerts.Count())
		{
			lookupList[vIndex] = deadVerts[deadIndex];
			SetTVVertControlIndex(deadVerts[deadIndex],-1);
			SetTVVert(t,deadVerts[deadIndex],p,mod);//TVMaps.v[found].p = p;
			SetTVVertInfluence(deadVerts[deadIndex],0.0f);//TVMaps.v[found].influence = 0.0f;
			SetTVVertDead(deadVerts[deadIndex],FALSE);//TVMaps.v[found].flags -= FLAG_DEAD;
			deadIndex++;
		}
		else
		{
			lookupList[vIndex] = TVMaps.v.Count();
			UVW_TVVertClass tv;
			tv.SetP(p);
			tv.SetFlag(0);
			tv.SetInfluence(0.0f);
			tv.SetControlID(-1);
			TVMaps.v.Append(1,&tv,1);
			mVSel.SetSize(TVMaps.v.Count(), 1);
			TVMaps.mSystemLockedFlag.SetSize(TVMaps.v.Count(), 1);
		}

	}

	vertSel.SetSize(TVMaps.v.Count());
	vertSel.ClearAll();

	for (int i = 0; i < TVMaps.f.Count(); i++)
	{
		if ((faceSel[i]) && (!(TVMaps.f[i]->flags & FLAG_DEAD)))
		{
	
			int degree = TVMaps.f[i]->count;
			for (int j = 0; j < degree; j++)
			{
				int index = TVMaps.f[i]->v[j];
				TVMaps.f[i]->t[j] = lookupList[index];
				vertSel.Set(lookupList[index],TRUE);

				if (TVMaps.f[i]->vecs)
				{
					int index = TVMaps.f[i]->vecs->vhandles[j*2];
					if ((index != -1) && (lookupList[index] != -1))
					{
						TVMaps.f[i]->vecs->handles[j*2] = lookupList[index];
						vertSel.Set(lookupList[index],TRUE);
					}

					index = TVMaps.f[i]->vecs->vhandles[j*2+1];
					if ((index != -1) && (lookupList[index] != -1))
					{
						TVMaps.f[i]->vecs->handles[j*2+1] = lookupList[index];
						vertSel.Set(lookupList[index],TRUE);
					}
					index = TVMaps.f[i]->vecs->vinteriors[j];
					if ((index != -1) && (lookupList[index] != -1))
					{
						TVMaps.f[i]->vecs->interiors[j] = lookupList[index];
						vertSel.Set(lookupList[index],TRUE);
					}
				}
			}
		}
	}

}


void	MeshTopoData::AddVertsToCluster(int faceIndex, BitArray &processedVerts, ClusterClass *cluster)
{

	int degree = TVMaps.f[faceIndex]->count;
	for (int k = 0; k < degree; k++)
	{
		int index = TVMaps.f[faceIndex]->t[k];
		if (!processedVerts[index])
		{
			cluster->verts.Append(1,&index,100);
			processedVerts.Set(index,TRUE);
		}
		if (TVMaps.f[faceIndex]->vecs)
		{
			int index = TVMaps.f[faceIndex]->vecs->handles[k*2];
			if (!processedVerts[index])
			{
				cluster->verts.Append(1,&index,100);
				processedVerts.Set(index,TRUE);
			}
			index = TVMaps.f[faceIndex]->vecs->handles[k*2+1];
			if (!processedVerts[index])
			{
				cluster->verts.Append(1,&index,100);
				processedVerts.Set(index,TRUE);
			}
			index = TVMaps.f[faceIndex]->vecs->interiors[k];
			if (!processedVerts[index])
			{
				cluster->verts.Append(1,&index,100);
				processedVerts.Set(index,TRUE);
			}
		}
	}
}

void	MeshTopoData::UpdateClusterVertices(Tab<ClusterClass*> &clusterList)
{
	BitArray processedVerts;
	processedVerts.SetSize(TVMaps.v.Count());

	for (int i = 0; i < clusterList.Count(); i++)
	{
		
		if (clusterList[i]->ld == this)
		{
			clusterList[i]->verts.SetCount(0);
			processedVerts.ClearAll();
			for (int j = 0; j < clusterList[i]->faces.Count(); j++)
			{
				int findex = clusterList[i]->faces[j];
				AddVertsToCluster(findex, processedVerts, clusterList[i]);
			}
			for (int j = 0; j < clusterList[i]->verts.Count(); j++)
			{
				int id = clusterList[i]->verts[j];
			}
		}
	}
}

void	MeshTopoData::AlignCluster(Tab<ClusterClass*> &clusterList, int baseCluster, int moveCluster, int innerFaceIndex, int outerFaceIndex,int edgeIndex,UnwrapMod *mod)
{

	//get edges that are coincedent
	int vInner[2];
	int vOuter[2];

	int vInnerVec[2];
	int vOuterVec[2];


	int ct = 0;
	int vct = 0;
	for (int i = 0; i < TVMaps.f[innerFaceIndex]->count; i++)
	{
		int innerIndex = TVMaps.f[innerFaceIndex]->v[i];
		for (int j = 0; j < TVMaps.f[outerFaceIndex]->count; j++)
		{
			int outerIndex = TVMaps.f[outerFaceIndex]->v[j];
			if (innerIndex == outerIndex)
			{
				vInner[ct] = TVMaps.f[innerFaceIndex]->t[i];


				vOuter[ct] = TVMaps.f[outerFaceIndex]->t[j];
				ct++;
			}

		}

	}

	vInnerVec[0] = -1;
	vInnerVec[1] = -1;
	vOuterVec[0] = -1;
	vOuterVec[1] = -1;
	ct = 0;

	if ( (TVMaps.f[innerFaceIndex]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[innerFaceIndex]->vecs) &&
		(TVMaps.f[outerFaceIndex]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[outerFaceIndex]->vecs) 
		)
	{
		for (int i = 0; i < TVMaps.f[innerFaceIndex]->count*2; i++)
		{
			int innerIndex = TVMaps.f[innerFaceIndex]->vecs->vhandles[i];
			for (int j = 0; j < TVMaps.f[outerFaceIndex]->count*2; j++)
			{
				int outerIndex = TVMaps.f[outerFaceIndex]->vecs->vhandles[j];
				if (innerIndex == outerIndex)
				{
					int vec = TVMaps.f[innerFaceIndex]->vecs->handles[i];
					vInnerVec[ct] = vec;


					vec = TVMaps.f[outerFaceIndex]->vecs->handles[j];
					vOuterVec[ct] = vec;
					ct++;
				}

			}

		}
	}



	//get  align vector
	Point3 pInner[2];
	Point3 pOuter[2];

	pInner[0] = TVMaps.v[vInner[0]].GetP();
	pInner[1] = TVMaps.v[vInner[1]].GetP();

	pOuter[0] = TVMaps.v[vOuter[0]].GetP();
	pOuter[1] = TVMaps.v[vOuter[1]].GetP();

	Point3 offset = pInner[0] - pOuter[0];

	Point3 vecA, vecB;
	vecA = Normalize(pInner[1] - pInner[0]);
	vecB = Normalize(pOuter[1] - pOuter[0]);
	float dot = DotProd(vecA,vecB);

	float angle = 0.0f;
	if (dot == -1.0f)
		angle = PI;
	else if (dot >= 1.0f)
		angle = 0.f;
	else angle = acos(dot);

	if ((_isnan(angle)) || (!_finite(angle)))
		angle = 0.0f;



	Matrix3 tempMat(1);  
	tempMat.RotateZ(angle); 
	Point3 vecC = VectorTransform(tempMat,vecB);




	float negAngle = -angle;
	Matrix3 tempMat2(1); 
	tempMat2.RotateZ(negAngle); 
	Point3 vecD = VectorTransform(tempMat2,vecB);

	float la,lb;
	la = Length(vecA-vecC);
	lb = Length(vecA-vecD);
	if (la > lb)
		angle = negAngle;

	clusterList[moveCluster]->newX = offset.x;
	clusterList[moveCluster]->newY = offset.y;
	//build vert list
	//move those verts
	BitArray processVertList;
	processVertList.SetSize(TVMaps.v.Count());
	processVertList.ClearAll();
	for (int i =0; i < clusterList[moveCluster]->faces.Count(); i++)
	{
		int faceIndex = clusterList[moveCluster]->faces[i];
		for (int j =0; j < TVMaps.f[faceIndex]->count; j++)
		{
			int vertexIndex = TVMaps.f[faceIndex]->t[j];
			processVertList.Set(vertexIndex);


			if ( (patch) && (TVMaps.f[faceIndex]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[faceIndex]->vecs))
			{
				int vertIndex;

				if (TVMaps.f[faceIndex]->flags & FLAG_INTERIOR)
				{
					vertIndex = TVMaps.f[faceIndex]->vecs->interiors[j];
					if ((vertIndex >=0) && (vertIndex < processVertList.GetSize()))
						processVertList.Set(vertIndex);
				}

				vertIndex = TVMaps.f[faceIndex]->vecs->handles[j*2];
				if ((vertIndex >=0) && (vertIndex < processVertList.GetSize()))
					processVertList.Set(vertIndex);
				vertIndex = TVMaps.f[faceIndex]->vecs->handles[j*2+1];
				if ((vertIndex >=0) && (vertIndex < processVertList.GetSize()))
					processVertList.Set(vertIndex);

			}


		}
	}
	for (int i = 0; i < processVertList.GetSize(); i++)
	{
		if (processVertList[i])
		{
			Point3 p = TVMaps.v[i].GetP();
			//move to origin

			p -= pOuter[0];

			//rotate
			Matrix3 mat(1);   

			mat.RotateZ(angle); 

			p = p * mat;
			//move to anchor point        
			p += pInner[0];

			SetTVVert(0,i,p,mod);//TVMaps.v[i].p = p;
			//if (TVMaps.cont[i]) TVMaps.cont[i]->SetValue(0,&TVMaps.v[i].p);

		}
	}

	if ((vInnerVec[0] != -1) &&   (vInnerVec[1] != -1) && (vOuterVec[0] != -1) && (vOuterVec[1] != -1))
	{
		SetTVVert(0,vOuterVec[0],TVMaps.v[vInnerVec[0]].GetP(),mod);//TVMaps.v[vOuterVec[0]].p = TVMaps.v[vInnerVec[0]].p;
		//if (TVMaps.cont[vOuterVec[0]]) TVMaps.cont[vOuterVec[0]]->SetValue(0,&TVMaps.v[vInnerVec[0]].p);

		SetTVVert(0,vOuterVec[1],TVMaps.v[vInnerVec[1]].GetP(),mod);//TVMaps.v[vOuterVec[1]].p = TVMaps.v[vInnerVec[1]].p;
		//if (TVMaps.cont[vOuterVec[1]]) TVMaps.cont[vOuterVec[1]]->SetValue(0,&TVMaps.v[vInnerVec[1]].p);

	}

}


BOOL	MeshTopoData::BuildCluster( Tab<Point3> normalList, float threshold, BOOL connected, BOOL cleanUpStrayFaces, Tab<ClusterClass*> &clusterList, UnwrapMod *mod)
{
	BitArray processedFaces;

	Tab<BorderClass> clusterBorder;

	BitArray sel;

	sel.SetSize(TVMaps.f.Count());


	processedFaces.SetSize(TVMaps.f.Count());
	processedFaces.ClearAll();

	//check for type

	float radThreshold = threshold * PI/180.0f;
	TSTR statusMessage;






	Tab<Point3> objNormList;
	mod->BuildNormals(this,objNormList);


	for (int i = 0; i < mFSel.GetSize(); i++)
	{
		if (!mFSel[i])
			processedFaces.Set(i);
	}

	//build normals

	AdjEdgeList *edges = NULL;
	BOOL deleteEdges = FALSE;
	if ((mesh) && (connected))
	{
		edges = new AdjEdgeList(*mesh);
		deleteEdges = TRUE;
	}
	if (connected)
	{
		BOOL done = FALSE;
		int currentNormal = 0;

		if (normalList.Count() == 0)
			done = TRUE;

		while (!done)
		{
			sel.ClearAll();
			//find the closest normal within the threshold
			float angDist = -1.0f;
			int hitIndex = -1;
			for (int i =0; i < objNormList.Count(); i++)
			{
				if (!processedFaces[i])
				{
					float cangle = acos(DotProd(normalList[currentNormal],objNormList[i]));
					if (((cangle < angDist) || (hitIndex == -1)) && (cangle < radThreshold))
					{
						angDist = cangle;
						hitIndex = i;
					}
				}
			}
			int bail = 0;
			if ( (hitIndex != -1) )
			{
				mod->SelectFacesByGroup( this,sel,hitIndex, normalList[currentNormal], threshold, FALSE,objNormList,
					clusterBorder,
					edges);
				//add cluster
				if (sel.NumberSet() > 0)
				{
					//create a cluster and add it
					BitArray clusterVerts;
					clusterVerts.SetSize(TVMaps.v.Count());
					clusterVerts.ClearAll();

					ClusterClass *cluster = new ClusterClass();
					cluster->ld = this;
					BOOL hit = FALSE;
					cluster->normal = normalList[currentNormal];
					for (int j = 0; j < sel.GetSize(); j++)
					{
						if (sel[j] && (!processedFaces[j]))
						{
							//add to cluster
							processedFaces.Set(j,TRUE);
							cluster->faces.Append(1,&j);
							AddVertsToCluster(j,clusterVerts, cluster);
							hit = TRUE;
						}
					}

					cluster->borderData = clusterBorder;
					//add edges that were processed
					if (hit)
					{
						clusterList.Append(1,&cluster);
						bail++;
					}
					else
					{
						delete cluster;
					}

				}
			}  
			currentNormal++;
			if (currentNormal >= normalList.Count()) 
			{
				currentNormal = 0;
				if (bail == 0) done = TRUE;
			}


			int per = (processedFaces.NumberSet() * 100)/processedFaces.GetSize();
			statusMessage.printf("%s %d%%.",GetString(IDS_PW_STATUS_BUILDCLUSTER),per);
			if (mod->Bail(mod->ip,statusMessage))
			{
				if (deleteEdges) delete edges;
				return FALSE;
			}


		}
	}
	else
	{  
		for (int i =0; i < normalList.Count(); i++)
		{
			sel.ClearAll();

			//find closest face norm
			mod->SelectFacesByNormals(this,sel,normalList[i], threshold, objNormList);
			//add cluster
			int numberSet = sel.NumberSet();
			int totalNumberSet = processedFaces.NumberSet();
			if ( numberSet > 0)
			{
				//create a cluster and add it
				ClusterClass *cluster = new ClusterClass();
				cluster->ld = this;
				BOOL hit = FALSE;
				cluster->normal = normalList[i];
				BitArray clusterVerts;
				clusterVerts.SetSize(TVMaps.v.Count());
				clusterVerts.ClearAll();
				for (int j = 0; j < sel.GetSize(); j++)
				{
					if (sel[j] && (!processedFaces[j]))
					{
						//add to cluster
						processedFaces.Set(j,TRUE);
						cluster->faces.Append(1,&j);
						AddVertsToCluster(j,clusterVerts, cluster);
						hit = TRUE;
					}
				}
				if (hit)
					clusterList.Append(1,&cluster);
				else delete cluster;
			}

			int per = (i * 100)/normalList.Count();
			statusMessage.printf("%s %d%%.",GetString(IDS_PW_STATUS_BUILDCLUSTER),per);
			if (mod->Bail(mod->ip,statusMessage))
			{
				if (deleteEdges) delete edges;
				return FALSE;
			}

		}
	}
	//process the ramaining

	if (cleanUpStrayFaces)
	{
		//       Tab<int> tempSeedFaces = seedFaces;
		int ct = 0;
		if (mod->seedFaces.Count() > 0) ct = mod->seedFaces[0];
		while ( (processedFaces.NumberSet() != processedFaces.GetSize()) )
		{
			if (!processedFaces[ct])
			{
				if (connected)
				{
					mod->SelectFacesByGroup( this,sel,ct, objNormList[ct], threshold, FALSE,objNormList,
						clusterBorder,
						edges);

				}
				else mod->SelectFacesByNormals(this,sel,objNormList[ct], threshold, objNormList);
				//add cluster
				if (sel.NumberSet() > 0)
				{
					//create a cluster and add it
					ClusterClass *cluster = new ClusterClass();
					cluster->ld = this;
					cluster->normal = objNormList[ct];
					BOOL hit = FALSE;

					BitArray clusterVerts;
					clusterVerts.SetSize(TVMaps.v.Count());
					clusterVerts.ClearAll();

					for (int j = 0; j < sel.GetSize(); j++)
					{
						if (sel[j] && (!processedFaces[j]))
						{
							//add to cluster
							processedFaces.Set(j,TRUE);
							cluster->faces.Append(1,&j);
							AddVertsToCluster(j,clusterVerts, cluster);
							hit = TRUE;
						}
					}
					if (connected)
					{
						cluster->borderData = clusterBorder;
					}
					if (hit)
					{
						clusterList.Append(1,&cluster);
					}
					else
					{
						delete cluster;
					}

				}

			}
			ct++;
			if (ct >= processedFaces.GetSize())
				ct =0;

			int per = (processedFaces.NumberSet() * 100)/processedFaces.GetSize();
			statusMessage.printf("%s %d%%.",GetString(IDS_PW_STATUS_BUILDCLUSTER),per);
			if (mod->Bail(mod->ip,statusMessage))
			{
				if (deleteEdges) delete edges;
				return FALSE;
			}

		}
	}
	if (deleteEdges) delete edges;

	

	return TRUE;

}

void MeshTopoData::PlanarMapNoScale(Point3 gNormal, UnwrapMod *mod )
{


	Matrix3 gtm;
	mod->UnwrapMatrixFromNormal(gNormal,gtm);

	gtm = Inverse(gtm);

	BitArray tempVSel;
	DetachFromGeoFaces(mFSel, tempVSel, mod);

	TimeValue t = GetCOREInterface()->GetTime();

	Box3 bounds;
	bounds.Init();
	for (int i = 0; i < TVMaps.v.Count(); i++)
	{
		if (tempVSel[i])
		{
			bounds += TVMaps.v[i].GetP();
		}
	}
	Point3 gCenter = bounds.Center();

	for (int i = 0; i < TVMaps.v.Count(); i++)
	{
		if (tempVSel[i])
		{
			TVMaps.v[i].SetFlag(0);
			TVMaps.v[i].SetInfluence(0.0f);

			Point3 tp = TVMaps.v[i].GetP() - gCenter;
			tp = tp * gtm;
			tp.z = 0.0f;

			SetTVVert(t,i,tp,mod);				
		}
	}
	mVSel = tempVSel;



}

BOOL MeshTopoData::NormalMap(Tab<Point3*> *normalList, Tab<ClusterClass*> &clusterList, UnwrapMod *mod )
{

	if (TVMaps.f.Count() == 0) return FALSE;


	BitArray polySel = mFSel;



	BitArray holdPolySel = mFSel;



	Point3 normal(0.0f,0.0f,1.0f);


	Tab<Point3> mapNormal;
	mapNormal.SetCount(normalList->Count());
	for (int i =0; i < mapNormal.Count(); i++)
	{
		mapNormal[i] = *(*normalList)[i];
		ClusterClass *cluster = new ClusterClass();
		cluster->normal = mapNormal[i];
		cluster->ld = this;
		clusterList.Append(1,&cluster);
	}


	//check for type

	BOOL bContinue = TRUE;
	TSTR statusMessage;


	Tab<Point3> objNormList;         
	mod->BuildNormals(this,objNormList);

	if (objNormList.Count() == 0)
	{
		return FALSE;
	}

	BitArray skipFace;
	skipFace.SetSize(mFSel.GetSize());
	skipFace.ClearAll();
	int numberSet = mFSel.NumberSet();
	for (int i = 0; i < mFSel.GetSize(); i++)
	{
		if (!mFSel[i])
			skipFace.Set(i);
	}





	for (int i =0; i < objNormList.Count(); i++)
	{
		int index = -1;
		float angle = 0.0f;
		if (skipFace[i] == FALSE)
		{
			for (int j =0; j < clusterList.Count(); j++)
			{
				if (clusterList[j]->ld == this)
				{
					Point3 debugNorm = objNormList[i];
					float dot = DotProd(debugNorm,clusterList[j]->normal);//mapNormal[j]);
					float newAngle = (acos(dot));

					if ((dot >= 1.0f) || (newAngle <= angle) || (index == -1))
					{
						index = j;
						angle = newAngle;
					}
				}
			}
			if (index != -1)
			{
				clusterList[index]->faces.Append(1,&i);
			}
		}
	}


	

	BitArray sel;
	sel.SetSize(TVMaps.f.Count());
	
	for (int i =0; i < clusterList.Count(); i++)
	{
		if (clusterList[i]->ld == this)
		{
			sel.ClearAll();
			for (int j = 0; j < clusterList[i]->faces.Count();j++)
				sel.Set(clusterList[i]->faces[j]);
			if (sel.NumberSet() > 0)
			{
	//			fnSelectPolygonsUpdate(&sel, FALSE);
				mFSel = sel;
				PlanarMapNoScale(clusterList[i]->normal,mod);

			}

			int per = (i * 100)/clusterList.Count();
			statusMessage.printf("%s %d%%.",GetString(IDS_PW_STATUS_MAPPING),per);
			if (mod->Bail(GetCOREInterface(),statusMessage))
			{
				i = clusterList.Count();
				bContinue =  FALSE;
			}
		}
	}

	


	for (int i =0; i < clusterList.Count(); i++)
	{
		if (clusterList[i]->faces.Count() == 0)
		{
			delete clusterList[i];
			clusterList.Delete(i,1);
			i--;
		}

	}

	BitArray clusterVerts;
	clusterVerts.SetSize(TVMaps.v.Count());
	
	for (int i =0; i < clusterList.Count(); i++)
	{
		
		if (clusterList[i]->ld == this)
		{
			clusterVerts.ClearAll();
			for (int j = 0; j < clusterList[i]->faces.Count(); j++)
			{
				int findex = clusterList[i]->faces[j];
				AddVertsToCluster(findex,clusterVerts, clusterList[i]);
			}
		}
	}
	mFSel = holdPolySel ;

	return bContinue;
}

void MeshTopoData::ApplyMap(int mapType,  BOOL normalizeMap, Matrix3 gizmoTM, UnwrapMod *mod)
{
//this just catches some strane cases where the z axis is 0 which causes some bad numbers
/*
if (Length(gizmoTM.GetRow(2)) == 0.0f)
	{
		gizmoTM.SetRow(2,Point3(0.0f,0.0f,1.0f));
	}
*/

	float circ = 1.0f;
/*
 	if (!normalizeMap)
	{
		for (int i = 0; i < 3; i++)
		{
			Point3 vec = gizmoTM.GetRow(i);
			vec = Normalize(vec);
			gizmoTM.SetRow(i,vec);
		}
	}
*/
	Matrix3 toGizmoSpace = gizmoTM;

	BitArray tempVSel;
	DetachFromGeoFaces(mFSel, tempVSel, mod);
	TimeValue t = GetCOREInterface()->GetTime();

	//build available list

	if (mapType == SPHERICALMAP)
	{
		Tab<int> quads;
		quads.SetCount(TVMaps.v.Count());

		float longestR = 0.0f;
		for (int i = 0; i < TVMaps.v.Count(); i++)
		{
			BOOL found = FALSE;
			quads[i] = -1;
			if (tempVSel[i])
			{

				Point3 tp = TVMaps.v[i].GetP() * gizmoTM;//gverts.d[i].p * gtm;
					//z our y

				int quad = 0;
				if ((tp.x >= 0) && (tp.y >= 0))
					quad = 0;
				else if ((tp.x < 0) && (tp.y >= 0))
					quad = 1;
				else if ((tp.x < 0) && (tp.y < 0))
					quad = 2;
				else if ((tp.x >= 0) && (tp.y < 0))
					quad = 3;

				quads[i] = quad;

				//find the quad the point is in
			 	Point3 xvec(1.0f,0.0f,0.0f);
				Point3 zvec(0.0f,0.0f,-1.0f);

				float x= 0.0f;
				Point3 zp = tp;
				zp.z = 0.0f;
				
				float dot = DotProd(xvec,Normalize(zp));
				float angle = 0.0f;
				if (dot >= 1.0f)
					angle = 0.0f;
				else angle = acos(dot);

				x = angle/(PI*2.0f);
				if (quad > 1)
					x = (0.5f - x) + 0.5f;

				dot = DotProd(zvec,Normalize(tp));
				float yangle = 0.0f;
				if (dot >= 1.0f)
					yangle = 0.0f;
				else yangle = acos(dot);

				float y = yangle/(PI);
								
				tp.x = x;	
				tp.y = y;//tp.z;	
				float d = Length(tp);	
				tp.z = d;
				if (d > longestR)
					longestR = d;	

				TVMaps.v[i].SetFlag( 0 );
				TVMaps.v[i].SetInfluence( 0.0f );					
				TVMaps.v[i].SetP(tp);
				TVMaps.v[i].SetControlID(-1);
				mVSel.Set(i);
/*
				if (ct < alist.Count() )
				{
					int j = alist[ct];
					TVMaps.v[j].flags = 0;
					TVMaps.v[j].influence = 0.0f;					


					TVMaps.v[j].p = tp;
					vsel.Set(j);

					if (TVMaps.cont[j]) TVMaps.cont[j]->SetValue(0,&tp,CTRL_ABSOLUTE);
					gverts.d[i].newindex = j;
					ct++;

				}
				else
				{
					UVW_TVVertClass tempv;

					tempv.p = tp;

					tempv.flags = 0;
					tempv.influence = 0.0f;
					gverts.d[i].newindex = TVMaps.v.Count();
					TVMaps.v.Append(1,&tempv,5000);

					int vct = TVMaps.v.Count()-1;
					extraSel.Append(1,&vct,5000);


					Control* c;
					c = NULL;
					TVMaps.cont.Append(1,&c,5000);
					if (TVMaps.cont[TVMaps.v.Count()-1]) 
						TVMaps.cont[TVMaps.v.Count()-1]->SetValue(0,&TVMaps.v[TVMaps.v.Count()-1].p,CTRL_ABSOLUTE);
				}
*/
			}
		}


		//now copy our face data over
		Tab<int> faceAcrossSeam;
		for (int i = 0; i < TVMaps.f.Count(); i++)
		{			
			if (mFSel[i])
			{
				//		ActiveAddFaces.Append(1,&i,1);
				int ct = i;//gfaces[i]->FaceIndex;
//				TVMaps.f[ct]->flags = gfaces[i]->flags;
//				TVMaps.f[ct]->flags |= FLAG_SELECTED;
				int pcount = 3;
				//		if (gfaces[i].flags & FLAG_QUAD) pcount = 4;
				pcount = TVMaps.f[i]->count;//gfaces[i]->count;
				int quad0 = 0;
				int quad3 = 0;
				for (int j = 0; j < pcount; j++)
				{
					int index = TVMaps.f[i]->t[j];
					if (quads[index] == 0) quad0++;
					if (quads[index] == 3) quad3++;
					//find spot in our list
//					TVMaps.f[ct]->t[j] = gverts.d[index].newindex;
					if ((TVMaps.f[ct]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[ct]->vecs))
					{
						index = TVMaps.f[i]->vecs->handles[j*2];

						if (quads[index] == 0) quad0++;
						if (quads[index] == 3) quad3++;
						//find spot in our list
//						TVMaps.f[ct]->vecs->handles[j*2] = gverts.d[index].newindex;

						index = TVMaps.f[i]->vecs->handles[j*2+1];
						if (quads[index] == 0) quad0++;
						if (quads[index] == 3) quad3++;
						//find spot in our list
//						TVMaps.f[ct]->vecs->handles[j*2+1] = gverts.d[index].newindex;

						if (TVMaps.f[ct]->flags & FLAG_INTERIOR)
						{
							index = TVMaps.f[i]->vecs->interiors[j];
							if (quads[index] == 0) quad0++;
							if (quads[index] == 3) quad3++;
							//find spot in our list
//							TVMaps.f[ct]->vecs->interiors[j] = gverts.d[index].newindex;
						}

					}
				}

				if ((quad3 > 0) && (quad0 > 0))
				{
 					for (int j = 0; j < pcount; j++)
					{
						//find spot in our list
						int index = TVMaps.f[ct]->t[j];
						if (TVMaps.v[index].GetP().x <= 0.25f)
						{
							UVW_TVVertClass tempv = TVMaps.v[index];
							Point3 fp = tempv.GetP();
							fp.x += 1.0f;
							tempv.SetP(fp);
							tempv.SetFlag(0);
							tempv.SetInfluence( 0.0f);
							TVMaps.v.Append(1,&tempv,5000);
							int vct = TVMaps.v.Count()-1;
							TVMaps.f[ct]->t[j] = vct;
						}

						if ((TVMaps.f[ct]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[ct]->vecs))
						{
							

							//find spot in our list
							int index = TVMaps.f[ct]->vecs->handles[j*2];

							if (TVMaps.v[index].GetP().x <= 0.25f)
							{
								UVW_TVVertClass tempv = TVMaps.v[index];
								Point3 fp = tempv.GetP();
								fp.x += 1.0f;
								tempv.SetP(fp);
								tempv.SetFlag(0);
								tempv.SetInfluence( 0.0f);
								TVMaps.v.Append(1,&tempv,5000);
								int vct = TVMaps.v.Count()-1;
								TVMaps.f[ct]->vecs->handles[j*2] = vct;
							}
							//find spot in our list
							index = TVMaps.f[ct]->vecs->handles[j*2+1];
							if (TVMaps.v[index].GetP().x <= 0.25f)
							{
								UVW_TVVertClass tempv = TVMaps.v[index];
								Point3 fp = tempv.GetP();
								fp.x += 1.0f;
								tempv.SetP(fp);
								tempv.SetFlag(0);
								tempv.SetInfluence( 0.0f);
								TVMaps.v.Append(1,&tempv,5000);
								int vct = TVMaps.v.Count()-1;
								TVMaps.f[ct]->vecs->handles[j*2+1] = vct;
							}
							if (TVMaps.f[ct]->flags & FLAG_INTERIOR)
							{
								index = TVMaps.f[ct]->vecs->interiors[j];
								if (TVMaps.v[index].GetP().x <= 0.25f)
								{
									UVW_TVVertClass tempv = TVMaps.v[index];
									Point3 fp = tempv.GetP();
									fp.x += 1.0f;
									tempv.SetP(fp);
									tempv.SetFlag(0);
									tempv.SetInfluence( 0.0f);
									TVMaps.v.Append(1,&tempv,5000);
									int vct = TVMaps.v.Count()-1;
									TVMaps.f[ct]->vecs->interiors[j] = vct;
								}
							}

						}
					}
				}

			}
		}

		if (!normalizeMap)
		{
			BitArray processedVerts;
			processedVerts.SetSize(TVMaps.v.Count());
			processedVerts.ClearAll();
			circ = circ * PI * longestR * 2.0f;
			for (int i = 0; i < TVMaps.f.Count(); i++)
			{	
				if (mFSel[i])
				{
					int pcount = 3;
					//		if (gfaces[i].flags & FLAG_QUAD) pcount = 4;
					pcount = TVMaps.f[i]->count;
 					int ct = i;
						
						for (int j = 0; j < pcount; j++)
						{
							//find spot in our list
							int index = TVMaps.f[ct]->t[j];
							if (!processedVerts[index])
							{
								Point3 p = TVMaps.v[index].GetP();
								p.x *= circ;
								p.y *= circ;
								TVMaps.v[index].SetP(p);
								processedVerts.Set(index,TRUE);
							}

							if ((TVMaps.f[ct]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[ct]->vecs))
							{
								

								//find spot in our list
								int index = TVMaps.f[ct]->vecs->handles[j*2];
								if (!processedVerts[index])
								{
									Point3 p = TVMaps.v[index].GetP();
									p.x *= circ;
									p.y *= circ;
									TVMaps.v[index].SetP(p);
									processedVerts.Set(index,TRUE);
								}
								//find spot in our list
								index = TVMaps.f[ct]->vecs->handles[j*2+1];
								if (!processedVerts[index])
								{
									Point3 p = TVMaps.v[index].GetP();
									p.x *= circ;
									p.y *= circ;
									TVMaps.v[index].SetP(p);
									processedVerts.Set(index,TRUE);
								}
								if (TVMaps.f[ct]->flags & FLAG_INTERIOR)
								{
									index = TVMaps.f[ct]->vecs->interiors[j];
									if (!processedVerts[index])
									{
										Point3 p = TVMaps.v[index].GetP();
										p.x *= circ;
										p.y *= circ;
										TVMaps.v[index].SetP(p);
										processedVerts.Set(index,TRUE);
									}
								}
							}
						}						
					}
			}
		}

		
		//now find the seam
		mVSel.SetSize(TVMaps.v.Count(), 1);
		mVSel.Set(TVMaps.v.Count()-1);
		TVMaps.mSystemLockedFlag.SetSize(TVMaps.v.Count(), 1);
/*		for (int i = 0; i < extraSel.Count(); i++)
			vsel.Set(extraSel[i],TRUE);
*/
	}
	else if (mapType == CYLINDRICALMAP)
	{
		Tab<int> quads;
		quads.SetCount(TVMaps.v.Count());

		float longestR = 0.0f;

		for (int i = 0; i < TVMaps.v.Count(); i++)
		{

			BOOL found = FALSE;
			quads[i] = -1;
			if (tempVSel[i])
//			if (gverts.sel[i])
			{

				Point3 gp = TVMaps.v[i].GetP();//gverts.d[i].p;
				Point3 tp = gp * toGizmoSpace;
					//z our y

				int quad = 0;
				if ((tp.x >= 0) && (tp.y >= 0))
					quad = 0;
				else if ((tp.x < 0) && (tp.y >= 0))
					quad = 1;
				else if ((tp.x < 0) && (tp.y < 0))
					quad = 2;
				else if ((tp.x >= 0) && (tp.y < 0))
					quad = 3;

				quads[i] = quad;

				//find the quad the point is in
			 	Point3 xvec(1.0f,0.0f,0.0f);
				
				float x= 0.0f;
				Point3 zp = tp;
				zp.z = 0.0f;
				
				float dot = DotProd(xvec,Normalize(zp));
				float angle = 0.0f;
				if (dot >= 1.0f)
					angle = 0.0f;
				else angle = acos(dot);

				x = angle/(PI*2.0f);
				if (quad > 1)
					x = (0.5f - x) + 0.5f;

				TVMaps.v[i].SetFlag(0);
				TVMaps.v[i].SetInfluence(0.0f);

				Point3 fp;
				fp.x = x;	
				fp.y = tp.z;	
				float d = Length(zp);	
				fp.z = d;

				SetTVVert(t,i,fp,mod);				


				if (d > longestR)
					longestR = d;
								
			}
		}


		//now copy our face data over
		Tab<int> faceAcrossSeam;
		for (int i = 0; i < TVMaps.f.Count(); i++)
		{			
			if (mFSel[i])
			{
				int ct = i;
				int pcount = 3;
				//		if (gfaces[i].flags & FLAG_QUAD) pcount = 4;
				pcount = TVMaps.f[i]->count;
				int quad0 = 0;
				int quad3 = 0;
				for (int j = 0; j < pcount; j++)
				{
					int index = TVMaps.f[i]->t[j];
					if (quads[index] == 0) quad0++;
					if (quads[index] == 3) quad3++;
					if ((TVMaps.f[ct]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[ct]->vecs))
					{
						index = TVMaps.f[i]->vecs->handles[j*2];

						if (quads[index] == 0) quad0++;
						if (quads[index] == 3) quad3++;

						index = TVMaps.f[i]->vecs->handles[j*2+1];
						if (quads[index] == 0) quad0++;
						if (quads[index] == 3) quad3++;
						//find spot in our list

						if (TVMaps.f[ct]->flags & FLAG_INTERIOR)
						{
							index = TVMaps.f[i]->vecs->interiors[j];
							if (quads[index] == 0) quad0++;
							if (quads[index] == 3) quad3++;
						}

					}
				}

				if ((quad3 > 0) && (quad0 > 0))
				{
 					for (int j = 0; j < pcount; j++)
					{
						//find spot in our list
						int index = TVMaps.f[ct]->t[j];
						if (TVMaps.v[index].GetP().x <= 0.25f)
						{
							UVW_TVVertClass tempv = TVMaps.v[index];
							Point3 tpv = tempv.GetP();
							tpv.x += 1.0f;
							tempv.SetP(tpv);							
							tempv.SetFlag(0);
							tempv.SetInfluence(0.0f);
							tempv.SetControlID(-1);
							TVMaps.v.Append(1,&tempv,5000);
							int vct = TVMaps.v.Count()-1;
							TVMaps.f[ct]->t[j] = vct;
						}

						if ((TVMaps.f[ct]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[ct]->vecs))
						{
							

							//find spot in our list
							int index = TVMaps.f[ct]->vecs->handles[j*2];

							if (TVMaps.v[index].GetP().x <= 0.25f)
							{
								UVW_TVVertClass tempv = TVMaps.v[index];
								Point3 tpv = tempv.GetP();
								tpv.x += 1.0f;
								tempv.SetP(tpv);							
								tempv.SetFlag(0);
								tempv.SetInfluence(0.0f);
								tempv.SetControlID(-1);
								TVMaps.v.Append(1,&tempv,5000);
								int vct = TVMaps.v.Count()-1;
								TVMaps.f[ct]->vecs->handles[j*2] = vct;
							}
							//find spot in our list
							index = TVMaps.f[ct]->vecs->handles[j*2+1];
							if (TVMaps.v[index].GetP().x <= 0.25f)
							{
								UVW_TVVertClass tempv = TVMaps.v[index];
								Point3 tpv = tempv.GetP();
								tpv.x += 1.0f;
								tempv.SetP(tpv);							
								tempv.SetFlag(0);
								tempv.SetInfluence(0.0f);
								tempv.SetControlID(-1);
								TVMaps.v.Append(1,&tempv,5000);
								int vct = TVMaps.v.Count()-1;
								TVMaps.f[ct]->vecs->handles[j*2+1] = vct;
								
							}
							if (TVMaps.f[ct]->flags & FLAG_INTERIOR)
							{
								index = TVMaps.f[ct]->vecs->interiors[j];
								if (TVMaps.v[index].GetP().x <= 0.25f)
								{
									UVW_TVVertClass tempv = TVMaps.v[index];
									Point3 tpv = tempv.GetP();
									tpv.x += 1.0f;
									tempv.SetP(tpv);							
									tempv.SetFlag(0);
									tempv.SetInfluence(0.0f);
									tempv.SetControlID(-1);
									TVMaps.v.Append(1,&tempv,5000);
									int vct = TVMaps.v.Count()-1;
									TVMaps.f[ct]->vecs->interiors[j] = vct;
								}
							}

						}
					}
				}

			}
		}

		if (!normalizeMap)
		{
			BitArray processedVerts;
			processedVerts.SetSize(GetNumberTVVerts());//TVMaps.v.Count());
			processedVerts.ClearAll();
			circ = circ * PI * longestR * 2.0f;
			for (int i = 0; i < GetNumberFaces(); i++)//gfaces.Count(); i++)
			{	
				if (mFSel[i])
				{
					int pcount = 3;
					//		if (gfaces[i].flags & FLAG_QUAD) pcount = 4;
					pcount = GetFaceDegree(i);//gfaces[i]->count;
 					int ct = i;//gfaces[i]->FaceIndex;
						
						for (int j = 0; j < pcount; j++)
						{
							//find spot in our list
							int index = TVMaps.f[ct]->t[j];
							if (!processedVerts[index])
							{
								Point3 p = TVMaps.v[index].GetP();
								p.x *= circ;
								TVMaps.v[index].SetP(p);
								processedVerts.Set(index,TRUE);
							}

							if ((TVMaps.f[ct]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[ct]->vecs))
							{
								

								//find spot in our list
								int index = TVMaps.f[ct]->vecs->handles[j*2];
								if (!processedVerts[index])
								{
									Point3 p = TVMaps.v[index].GetP();
									p.x *= circ;
									TVMaps.v[index].SetP(p);
									processedVerts.Set(index,TRUE);
								}
								//find spot in our list
								index = TVMaps.f[ct]->vecs->handles[j*2+1];
								if (!processedVerts[index])
								{
									Point3 p = TVMaps.v[index].GetP();
									p.x *= circ;
									TVMaps.v[index].SetP(p);
									processedVerts.Set(index,TRUE);
								}
								if (TVMaps.f[ct]->flags & FLAG_INTERIOR)
								{
									index = TVMaps.f[ct]->vecs->interiors[j];
									if (!processedVerts[index])
									{
										Point3 p = TVMaps.v[index].GetP();
										p.x *= circ;
										TVMaps.v[index].SetP(p);
										processedVerts.Set(index,TRUE);
									}
								}

							}
						
					}
				}
			}
		}

		
		//now find the seam
		mVSel.SetSize(TVMaps.v.Count(), 1);
		mVSel.Set(TVMaps.v.Count()-1);
		TVMaps.mSystemLockedFlag.SetSize(TVMaps.v.Count(), 1);
//		for (int i = 0; i < extraSel.Count(); i++)
//			vsel.Set(extraSel[i],TRUE);

	}
	else if ((mapType == PLANARMAP) || (mapType == PELTMAP)|| (mapType == BOXMAP))
	{
		for (int i = 0; i < TVMaps.v.Count(); i++)
		{
			if (tempVSel[i])
			{
				TVMaps.v[i].SetFlag(0);
				TVMaps.v[i].SetInfluence(0.0f);

				Point3 tp = TVMaps.v[i].GetP();
				tp = tp * toGizmoSpace;
				tp.x += 0.5f;
				tp.y += 0.5f;
				tp.z = 0.0f;
				SetTVVert(t,i,tp,mod);				
			}
		}
	}
	
	BuildTVEdges();	
	BuildVertexClusterList();
}