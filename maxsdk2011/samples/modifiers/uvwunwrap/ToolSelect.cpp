/*

Copyright [2008] Autodesk, Inc.  All rights reserved.

Use of this software is subject to the terms of the Autodesk license agreement 
provided at the time of installation or download, or which otherwise accompanies 
this software in either electronic or hard copy form.   

*/
#include "unwrap.h"


void UnwrapMod::RebuildDistCache()
{

	if (iStr==NULL) return;

	float str = iStr->GetFVal();
	falloffStr = str;
	float sstr = str*str;
	if ((str == 0.0f) || (enableSoftSelection == FALSE))
	{
		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			MeshTopoData *ld = mMeshTopoData[ldID];
			for (int i = 0; i<ld->GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++)
			{
				if (!(ld->GetTVVertWeightModified(i)))//TVMaps.v[i].flags & FLAG_WEIGHTMODIFIED))
					ld->SetTVVertInfluence(i,0.0f);//TVMaps.v[i].influence = 0.0f;
			}
		}
		
		return;
	}

	Tab<Point3> selectedVerts;
	if (falloffSpace == 0)
	{
		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			MeshTopoData *ld = mMeshTopoData[ldID];
			for (int i = 0; i<ld->GetNumberGeomVerts(); i++)
			{
				if (ld->GetGeomVertSelected(i))
				{
					Point3 p = ld->GetGeomVert(i);
					selectedVerts.Append(1,&p,10000);
				}
			}
		}
	}
	else
	{
		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			MeshTopoData *ld = mMeshTopoData[ldID];
			for (int i = 0; i<ld->GetNumberTVVerts(); i++)
			{
				if (ld->GetTVVertSelected(i))
				{
					Point3 p = ld->GetTVVert(i);
					selectedVerts.Append(1,&p,10000);
				}
			}
		}

	}

/*
	Tab<int> Selected;
	for (int i = 0; i<TVMaps.v.Count(); i++)
	{
		if (vsel[i])
			Selected.Append(1,&i,1);
	}

*/

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];	
		BitArray useableVerts;

		useableVerts.SetSize(ld->GetNumberTVVerts());//TVMaps.v.Count());

		if (limitSoftSel)
		{
			int oldMode = fnGetTVSubMode();
			fnSetTVSubMode(TVVERTMODE);
			useableVerts.ClearAll();
			BitArray holdSel;
			BitArray vsel = ld->GetTVVertSelection();
			holdSel.SetSize(vsel.GetSize());
			holdSel = vsel;
			for (int i=0; i < limitSoftSelRange; i++)
			{
				ExpandSelection(ld,0);
			}
			vsel = ld->GetTVVertSelection();
			useableVerts = vsel;
			ld->SetTVVertSelection( holdSel);
			fnSetTVSubMode(oldMode);
		}
		else useableVerts.SetAll();

		for (int i = 0; i<ld->GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++)
		{
			if (!(ld->GetTVVertWeightModified(i)))//TVMaps.v[i].flags & FLAG_WEIGHTMODIFIED))
				ld->SetTVVertInfluence(i,0.0f);//TVMaps.v[i].influence = 0.0f;
		}

		Tab<Point3> xyzSpace;
		
		if (falloffSpace == 0)
		{
			xyzSpace.SetCount(ld->GetNumberTVVerts());
			for (int i = 0; i < ld->GetNumberTVVerts(); i++)
				xyzSpace[i] = Point3(0.0f,0.0f,0.0f);
			for (int i = 0; i < ld->GetNumberFaces(); i++)//TVMaps.f.Count(); i++)
			{
				if (!ld->GetFaceDead(i))
				{
					int pcount = 3;
					pcount = ld->GetFaceDegree(i);//TVMaps.f[i]->count;
					for (int j = 0 ; j < pcount; j++)
					{
						xyzSpace[ld->GetFaceTVVert(i,j)] = ld->GetGeomVert(ld->GetFaceGeomVert(i,j));//TVMaps.f[i]->t[j]] = TVMaps.geomPoints[TVMaps.f[i]->v[j]];
						if ( ld->GetFaceHasVectors(i))//(TVMaps.f[i]->flags & FLAG_CURVEDMAPPING) &&
//							(TVMaps.f[i]->vecs)
						{
							int uvwIndex = ld->GetFaceTVHandle(i,j*2);
							int geoIndex = ld->GetFaceGeomHandle(i,j*2);
							xyzSpace[uvwIndex] = ld->GetGeomVert(geoIndex);

							uvwIndex = ld->GetFaceTVHandle(i,j*2+1);
							geoIndex = ld->GetFaceGeomHandle(i,j*2+1);
							xyzSpace[uvwIndex] = ld->GetGeomVert(geoIndex);


//							objectPointList[TVMaps.f[i]->vecs->handles[j*2]] = TVMaps.geomPoints[TVMaps.f[i]->vecs->vhandles[j*2]];
//							objectPointList[TVMaps.f[i]->vecs->handles[j*2+1]] = TVMaps.geomPoints[TVMaps.f[i]->vecs->vhandles[j*2+1]];
							if (ld->GetFaceHasInteriors(i))//TVMaps.f[i]->flags & FLAG_INTERIOR) 
							{
								uvwIndex = ld->GetFaceTVInterior(i,j);
								geoIndex = ld->GetFaceGeomInterior(i,j);
								xyzSpace[uvwIndex] = ld->GetGeomVert(geoIndex);

//								objectPointList[TVMaps.f[i]->vecs->interiors[j]] = TVMaps.geomPoints[TVMaps.f[i]->vecs->vinteriors[j]];
							}

						}	
					}
				}
			}
		}

		BitArray vsel = ld->GetTVVertSelection();
		for (int i = 0; i < ld->GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++)
		{
			if ( (vsel[i] == 0) && (useableVerts[i]) && (!(ld->GetTVVertWeightModified(i))))//TVMaps.v[i].flags & FLAG_WEIGHTMODIFIED)))
			{
				float closest_dist = BIGFLOAT;
				Point3 sp(0.0f,0.0f,0.0f);
				if (falloffSpace == 0)
				{
					sp = xyzSpace[i];
				}
				else
				{
					sp = ld->GetTVVert(i);
				}

				for (int j= 0; j < selectedVerts.Count();j++)
				{
					//use XYZ space values
					if (falloffSpace == 0)
					{
						Point3 rp = selectedVerts[j];
						float d = LengthSquared(sp-rp);
						if (d < closest_dist) closest_dist = d;

					}
					else
						//use UVW space values
					{
						Point3 rp = selectedVerts[j];
						float d = LengthSquared(sp-rp);
						if (d < closest_dist) closest_dist = d;
					}
				}
				if (closest_dist < sstr)
				{
					closest_dist = (float) sqrt(closest_dist);
					float influ = 1.0f - closest_dist/str;
					ComputeFalloff(influ,falloff);
					ld->SetTVVertInfluence(i,influ);
				}
				else 
					ld->SetTVVertInfluence(i,0.0f);//TVMaps.v[i].influence = 0.0f;
			}
		}	
	}

}

void UnwrapMod::ExpandSelection(MeshTopoData *ld, int dir)
{
	BitArray faceHasSelectedVert;

	BitArray vsel = ld->GetTVVertSelection();

	faceHasSelectedVert.SetSize(ld->GetNumberFaces());//TVMaps.f.Count());
	faceHasSelectedVert.ClearAll();
	for (int i = 0; i < ld->GetNumberFaces();i++)
	{
		if (!(ld->GetFaceDead(i)))//TVMaps.f[i]->flags & FLAG_DEAD))
		{
			int pcount = 3;
			pcount = ld->GetFaceDegree(i);//TVMaps.f[i]->count;
			int totalSelected = 0;
			for (int k = 0; k < pcount; k++)
			{
				int index = ld->GetFaceTVVert(i,k);//TVMaps.f[i]->t[k];
				if (vsel[index])
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
	for (int i = 0; i < ld->GetNumberFaces()/*TVMaps.f.Count()*/;i++)
	{
		if (faceHasSelectedVert[i])
		{
			int pcount = 3;
			pcount = ld->GetFaceDegree(i);//TVMaps.f[i]->count;
			for (int k = 0; k < pcount; k++)
			{
				int index = ld->GetFaceTVVert(i,k);//TVMaps.f[i]->t[k];
				if (dir == 0)
					vsel.Set(index,1);
				else vsel.Set(index,0);
			}

		}
	}
	ld->SetTVVertSelection(vsel);
}

void UnwrapMod::ExpandSelection(int dir, BOOL rebuildCache, BOOL hold)

{
	// LAM - 6/19/04 - defect 576948
	if (hold)
	{
		theHold.Begin();
		HoldSelection();
	}

	//convert our sub selection type to vertex selection
	TransferSelectionStart();

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];

		ExpandSelection(ld,  dir);


	}

	SelectHandles(dir);
	if (rebuildCache)
		RebuildDistCache();
	//convert our sub selection back

	TransferSelectionEnd(FALSE,TRUE);


	if (hold)
		theHold.Accept(GetString(IDS_DS_SELECT));					
}

void	UnwrapMod::fnSelectElement()
{
	theHold.Begin();
	HoldSelection();
	SelectElement();
	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();
	RebuildDistCache();
	theHold.Accept(GetString(IDS_DS_SELECT));					
}

void UnwrapMod::SelectElement(BOOL addSelection)

{


	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		ld->SelectElement(fnGetTVSubMode(),addSelection);
	}


}


void UnwrapMod::SelectHandles(int dir)

{
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		ld->SelectHandles(dir);
	}
}





void UnwrapMod::SelectFacesByNormals( MeshTopoData *md,BitArray &sel,Point3 norm, float angle, Tab<Point3> &normList)
{

	if (normList.Count() == 0)
		BuildNormals(md,normList);
	norm = Normalize(norm);
	angle = angle * PI/180.0f;
	if (md->GetMesh())
	{
		//check for contigous faces
		double newAngle;
		if (sel.GetSize() != md->GetMesh()->numFaces)
			sel.SetSize(md->GetMesh()->numFaces);
		sel.ClearAll();
		for (int i =0; i < md->GetMesh()->numFaces; i++)
		{
			Point3 debugNorm = Normalize(normList[i]);
			float dot = DotProd(debugNorm,norm);
			newAngle = (acos(dot));




			if ((dot >= 1.0f) || (newAngle <= angle))
				sel.Set(i);
			else
			{
				sel.Set(i,0);
			}
		}
	}
	else if (md->GetMNMesh())
	{
		//check for contigous faces
		double newAngle;
		if (sel.GetSize() != md->GetMNMesh()->numf)
			sel.SetSize(md->GetMNMesh()->numf);
		for (int i =0; i < md->GetMNMesh()->numf; i++)
		{
			Point3 debugNorm = normList[i];
			float dot = DotProd(normList[i],norm);
			newAngle = (acos(dot));			
			if ((dot >= 1.0f) || (newAngle <= angle))
				sel.Set(i);
			else
			{
				sel.Set(i,0);
			}
		}
	}
	else if (md->GetPatch())
	{
		//check for contigous faces
		double newAngle;
		if (sel.GetSize() != md->GetPatch()->numPatches)
			sel.SetSize(md->GetPatch()->numPatches);
		for (int i =0; i < md->GetPatch()->numPatches; i++)
		{
			Point3 debugNorm = normList[i];
			float dot = DotProd(normList[i],norm);
			newAngle = (acos(dot));
			if ((dot >= 1.0f) || (newAngle <= angle))
				sel.Set(i);
			else
			{
				sel.Set(i,0);
			}
		}
	}


}


void UnwrapMod::SelectFacesByGroup( MeshTopoData *md,BitArray &sel,int seedFaceIndex, Point3 norm, float angle, BOOL relative,Tab<Point3> &normList,
								   Tab<BorderClass> &borderData,
								   AdjEdgeList *medges)
{
	//check for type

	if (normList.Count() == 0)
		BuildNormals(md,normList);


	angle = angle * PI/180.0f;

	sel.ClearAll();



	if (md->GetMesh())
	{
		//get seed face
		if (seedFaceIndex < md->GetMesh()->numFaces)
		{

			if (sel.GetSize() != md->GetMesh()->numFaces)
				sel.SetSize(md->GetMesh()->numFaces);
			//select it
			sel.Set(seedFaceIndex);

			//build egde list of all edges that have only one edge selected
			AdjEdgeList *edges;
			BOOL deleteEdges = FALSE;
			if (medges == NULL)
			{
				edges = new AdjEdgeList(*md->GetMesh());
				deleteEdges = TRUE;
			}
			else edges = medges;

			borderData.ZeroCount();


			int numberWorkableEdges = 1;
			while (numberWorkableEdges > 0)
			{
				numberWorkableEdges = 0;
	 			for (int i = 0; i < edges->edges.Count(); i++)
				{
					//					if (!blockedEdges[i])
					{
						int a = edges->edges[i].f[0];
						int b = edges->edges[i].f[1];
						if ( (a>=0) && (b>=0) )
						{
							if (sel[a] && (!sel[b]))
							{
								float newAngle = 0.0f;
								if (!relative)
								{
									float dot = DotProd(normList[b],norm);


									if (dot <= -1.0f)
										newAngle = PI;
									else if (dot < 1.0f)
										newAngle = (acos(dot));

								}
								else
								{
									float dot = DotProd(normList[b],normList[a]);
									if (dot <= -1.0f)
										newAngle = PI;
									else if (dot < 1.0f)
										newAngle = (acos(dot));
								}
								if (newAngle <= angle)
								{
									sel.Set(b);
									numberWorkableEdges++;

								}
								else
								{
									BorderClass tempData;
									tempData.edge = i;
									tempData.innerFace = a;
									tempData.outerFace = b;
									borderData.Append(1,&tempData);
								}
							}
							else if (sel[b] && (!sel[a]))
							{
								float newAngle = 0.0f;
								if (!relative)
								{
									float dot = DotProd(normList[a],norm);
									if (dot <= -1.0f)
										newAngle = PI;
									else if (dot < 1.0f)
										newAngle = (acos(dot));
								}
								else
								{
									float dot = DotProd(normList[a],normList[b]);
									if (dot <= -1.0f)
										newAngle = PI;
									else if (dot < 1.0f)
										newAngle = (acos(dot));
								}
								if (newAngle <= angle)
								{
									sel.Set(a);
									numberWorkableEdges++;

								}
								else
								{
									BorderClass tempData;
									tempData.edge = i;
									tempData.innerFace = b;
									tempData.outerFace = a;
									borderData.Append(1,&tempData);

								}

							}
						}
					}
				}
			}
			if (deleteEdges) delete edges;
		}

	}
	else if (md->GetPatch())
	{
		if (seedFaceIndex < md->GetPatch()->numPatches)
		{
			//select it
			if (sel.GetSize() != md->GetPatch()->numPatches)
				sel.SetSize(md->GetPatch()->numPatches);

			sel.Set(seedFaceIndex);

			borderData.ZeroCount();

			//build egde list of all edges that have only one edge selected
			PatchEdge *edges = md->GetPatch()->edges;

			int numberWorkableEdges = 1;
			while (numberWorkableEdges > 0)
			{
				numberWorkableEdges = 0;
				for (int i = 0; i < md->GetPatch()->numEdges; i++)
				{
					if (edges[i].patches.Count() ==2 )
					{
						int a = edges[i].patches[0];
						int b = edges[i].patches[1];
						if ( (a>=0) && (b>=0) )
						{
							if (sel[a] && (!sel[b]))
							{
								float newAngle = 0.0f;
								float dot = 0.0f;
								if (!relative)
									dot = DotProd(normList[b],norm);
								else 
									dot = DotProd(normList[b],normList[a]);

								if (dot < 1.0f)
									newAngle = acos(dot);

								if (newAngle <= angle)
								{
									sel.Set(b);
									numberWorkableEdges++;

								}
								else
								{
									BorderClass tempData;
									tempData.edge = i;
									tempData.innerFace = a;
									tempData.outerFace = b;
									borderData.Append(1,&tempData);

								}
							}
							else if (sel[b] && (!sel[a]))
							{
								float newAngle = 0.0f;
								float dot = 0.0f;
								if (!relative)
									dot = DotProd(normList[a],norm);
								else 
									dot = DotProd(normList[a],normList[b]);

								if (dot < 1.0f)
									newAngle = acos(dot);

								
								if (newAngle <= angle)
								{
									sel.Set(a);
									numberWorkableEdges++;

								}
								else
								{
									BorderClass tempData;
									tempData.edge = i;
									tempData.innerFace = b;
									tempData.outerFace = a;
									borderData.Append(1,&tempData);

								}
							}

						}
					}
				}
			}
		}

	}
	else if (md->GetMNMesh())
	{
		//select it
		if (seedFaceIndex < md->GetMNMesh()->numf)
		{
			if (sel.GetSize() != md->GetMNMesh()->numf)
				sel.SetSize(md->GetMNMesh()->numf);
			sel.Set(seedFaceIndex);

			borderData.ZeroCount();

			//build egde list of all edges that have only one edge selected
			MNEdge *edges = md->GetMNMesh()->E(0);
			int numberWorkableEdges = 1;
			while (numberWorkableEdges > 0)
			{
				numberWorkableEdges = 0;
				for (int i = 0; i < md->GetMNMesh()->nume; i++)
				{
					int a = edges[i].f1;
					int b = edges[i].f2;

					if ( (a>=0) && (b>=0) )
					{
						if (sel[a] && (!sel[b]))
						{
							float newAngle = 0.0f;
							if (!relative)
							{
								float dot = DotProd(normList[b],norm);
								if (dot < -0.99999f)
									newAngle = PI;
								else if (dot > 0.99999f)
									newAngle = 0.0f;
								else 
									newAngle = (acos(dot));

							}
							else
							{
								float dot = DotProd(normList[b],normList[a]);
								if (dot < -0.99999f)
									newAngle = PI;
								else if (dot > 0.99999f)
									newAngle = 0.0f;
								else 
									newAngle = (acos(dot));
							}

							if (newAngle <= angle)
							{
								sel.Set(b);
								numberWorkableEdges++;

							}
							else
							{
								BorderClass tempData;
								tempData.edge = i;
								tempData.innerFace = a;
								tempData.outerFace = b;
								borderData.Append(1,&tempData);

							}
						}
						else if (sel[b] && (!sel[a]))
						{

							float newAngle = 0.0f;
							if (!relative)
							{
								float dot = DotProd(normList[a],norm);
								if (dot < -0.99999f)
									newAngle = PI;
								else if (dot > 0.99999f)
									newAngle = 0.0f;
								else 
									newAngle = (acos(dot));
							}
							else
							{
								float dot = DotProd(normList[a],normList[b]);
								if (dot < -0.99999f)
									newAngle = PI;
								else if (dot > 0.99999f)
									newAngle = 0.0f;
								else 
									newAngle = (acos(dot));
							}
							if (newAngle <= angle)
							{
								sel.Set(a);
								numberWorkableEdges++;

							}
							else
							{

								BorderClass tempData;
								tempData.edge = i;
								tempData.innerFace = b;
								tempData.outerFace = a;
								borderData.Append(1,&tempData);

							}
						}

					}
				}
			}
		}
	}




}

void UnwrapMod::fnSelectPolygonsUpdate(BitArray *sel, BOOL update, INode *node)
{
		MeshTopoData *ld = GetMeshTopoData(node);
		if (ld)
		{
			ld->SetFaceSelection(*sel);
		}
		ComputeSelectedFaceData();
}

void UnwrapMod::fnSelectPolygonsUpdate(BitArray *sel, BOOL update)
{
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			ld->SetFaceSelection(*sel);
		}
		ComputeSelectedFaceData();
	}
}

void	UnwrapMod::fnSelectFacesByNormal(Point3 normal, float angleThreshold, BOOL update)
{
	//check for type

	if (!ip) return;

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *md = mMeshTopoData[ldID];//(MeshTopoData*)mcList[0]->localData;

		if (md) 
		{
			BitArray sel;
			Tab<Point3> normList;
			normList.ZeroCount();
			SelectFacesByNormals( md,sel, normal, angleThreshold,normList);
		}
	}
	ComputeSelectedFaceData();
}

void	UnwrapMod::fnSelectClusterByNormal( float angleThreshold, int seedIndex, BOOL relative, BOOL update)
{
	//check for type
	if (!ip) return;
	seedIndex--;


	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *md = mMeshTopoData[ldID];//(MeshTopoData*)mcList[0]->localData;

		if (md) 
		{
			BitArray sel;

			Tab<BorderClass> dummy;
			Tab<Point3> normList;
			normList.ZeroCount();
			BuildNormals(md, normList);
			if ((seedIndex >= 0) && (seedIndex <normList.Count()))
			{
				Point3 normal = normList[seedIndex];
				SelectFacesByGroup( md,sel,seedIndex, normal, angleThreshold, relative,normList,dummy);
			}
		}
	}
	ComputeSelectedFaceData();
}



BOOL	UnwrapMod::fnGetLimitSoftSel()
{
	return limitSoftSel;
}

void	UnwrapMod::fnSetLimitSoftSel(BOOL limit)
{
	limitSoftSel = limit;
	RebuildDistCache();
	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	InvalidateView();

}

int		UnwrapMod::fnGetLimitSoftSelRange()
{
	return limitSoftSelRange;
}

void	UnwrapMod::fnSetLimitSoftSelRange(int range)
{
	limitSoftSelRange = range;
	RebuildDistCache();
	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	InvalidateView();
}


float	UnwrapMod::fnGetVertexWeight(int index, INode *node)
{
	float v = 0.0f;
	index--;
	MeshTopoData *ld = GetMeshTopoData(node);		
	if (ld)
	{
		v = ld->GetTVVertInfluence(index);
	}

	return v;
}

float	UnwrapMod::fnGetVertexWeight(int index)
{
	float v = 0.0f;
	index--;
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];		
		if (ld)
		{
			v = ld->GetTVVertInfluence(index);
		}
	}
	return v;
}

void	UnwrapMod::fnSetVertexWeight(int index,float weight, INode *node)
{
	index--;
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = GetMeshTopoData(node);		
		if (ld)
		{
			ld->SetTVVertInfluence(index, weight);//	TVMaps.v[index].influence = weight;
			int flag = ld->GetTVVertFlag(index);
			ld->SetTVVertFlag(index,flag|FLAG_WEIGHTMODIFIED);//TVMaps.v[index].flags  |= FLAG_WEIGHTMODIFIED;
			NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
			InvalidateView();
		}
	}
}


void	UnwrapMod::fnSetVertexWeight(int index,float weight)
{
	index--;
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];		
		if (ld)
		{
			ld->SetTVVertInfluence(index, weight);//	TVMaps.v[index].influence = weight;
			int flag = ld->GetTVVertFlag(index);
			ld->SetTVVertFlag(index,flag|FLAG_WEIGHTMODIFIED);//TVMaps.v[index].flags  |= FLAG_WEIGHTMODIFIED;
			NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
			InvalidateView();
		}
	}
}


BOOL	UnwrapMod::fnIsWeightModified(int index, INode *node)
{
	index--;
	BOOL mod = FALSE;
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = GetMeshTopoData(node);		
		if (ld)
		{
			int flag = ld->GetTVVertFlag(index);
			mod = (flag & FLAG_WEIGHTMODIFIED);
		}
	}
	return mod;

}

BOOL	UnwrapMod::fnIsWeightModified(int index)
{
	index--;
	BOOL mod = FALSE;
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];		
		if (ld)
		{
			int flag = ld->GetTVVertFlag(index);
			mod = (flag & FLAG_WEIGHTMODIFIED);
		}
	}
	return mod;

}

void	UnwrapMod::fnModifyWeight(int index, BOOL modified, INode *node)
{
	index--;

	MeshTopoData *ld = GetMeshTopoData(node);		
	if (ld)
	{
		int flag = ld->GetTVVertFlag(index);
		if (modified)
			 ld->SetTVVertFlag(index,flag | FLAG_WEIGHTMODIFIED);//TVMaps.v[index].flags |= FLAG_WEIGHTMODIFIED;
		else
		{
			ld->SetTVVertFlag(index,flag & ~FLAG_WEIGHTMODIFIED);//TVMaps.v[index].flags &= ~FLAG_WEIGHTMODIFIED;
		}
		NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
		InvalidateView();
	}
}

void	UnwrapMod::fnModifyWeight(int index, BOOL modified)
{
	index--;

	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];		
		if (ld)
		{
			int flag = ld->GetTVVertFlag(index);
			if (modified)
				 ld->SetTVVertFlag(index,flag | FLAG_WEIGHTMODIFIED);//TVMaps.v[index].flags |= FLAG_WEIGHTMODIFIED;
			else
			{
				ld->SetTVVertFlag(index,flag & ~FLAG_WEIGHTMODIFIED);//TVMaps.v[index].flags &= ~FLAG_WEIGHTMODIFIED;
			}
			NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
			InvalidateView();
		}
	}
}

BOOL	UnwrapMod::fnGetGeomElemMode()
{
	return geomElemMode;
}

void	UnwrapMod::fnSetGeomElemMode(BOOL elem)
{
	geomElemMode = elem;
	CheckDlgButton(hSelRollup,IDC_SELECTELEMENT_CHECK,geomElemMode);
}
/*
void	UnwrapMod::SelectGeomElement(MeshTopoData* tmd, BOOL addSel, BitArray *originalSel)
{

	BitArray holdSel = tmd->faceSel;

	if (!addSel && (originalSel != NULL))
	{
		BitArray oSel = *originalSel;
		tmd->faceSel = (~tmd->faceSel) & oSel;
	}
	Tab<BorderClass> dummy;
	Tab<Point3> normList;
	normList.ZeroCount();
	BuildNormals(tmd, normList);
	BitArray tempSel;
	tempSel.SetSize(tmd->faceSel.GetSize());

	tempSel = tmd->faceSel;

	for (int i =0; i < tmd->faceSel.GetSize(); i++)
	{
		if ((tempSel[i]) && (i >= 0) && (i <normList.Count()))
		{
			BitArray sel;
			Point3 normal = normList[i];
			SelectFacesByGroup( tmd,sel,i, normal, 180.0f, FALSE,normList,dummy);
			tmd->faceSel |= sel;
			for (int j = 0; j < tempSel.GetSize(); j++)
			{
				if (sel[j]) tempSel.Set(j,FALSE);
			}
		}
	}

	if (!addSel && (originalSel != NULL))
	{
		BitArray oSel = *originalSel;
		tmd->faceSel =  oSel & (~tmd->faceSel);
	}
}
*/



void	UnwrapMod::SelectGeomFacesByAngle(MeshTopoData* tmd)
{


	Tab<BorderClass> dummy;
	Tab<Point3> normList;
	normList.ZeroCount();
	BuildNormals(tmd, normList);
	BitArray tempSel;
	tempSel.SetSize(tmd->GetNumberFaces());//faceSel.GetSize());

	tempSel = tmd->GetFaceSelection();

	for (int i =0; i < tmd->GetNumberFaces(); i++)
	{
		if ((tempSel[i]) && (i >= 0) && (i <normList.Count()))
		{
			BitArray sel;
			Point3 normal = normList[i];
			SelectFacesByGroup( tmd,sel,i, normal, planarThreshold, FALSE,normList,dummy);
			BitArray faceSel = tmd->GetFaceSelection();
			faceSel |= sel;
			tmd->SetFaceSelection(faceSel);
			for (int j = 0; j < tempSel.GetSize(); j++)
			{
				if (sel[j]) tempSel.Set(j,FALSE);
			}
		}
	}




}

BOOL	UnwrapMod::fnGetGeomPlanarMode()
{
	return planarMode ;
}

void	UnwrapMod::fnSetGeomPlanarMode(BOOL planar)
{
	planarMode = planar;
	//update UI
	CheckDlgButton(hSelRollup,IDC_PLANARANGLE_CHECK,planarMode);
}

float	UnwrapMod::fnGetGeomPlanarModeThreshold()
{
	return planarThreshold;
}

void	UnwrapMod::fnSetGeomPlanarModeThreshold(float threshold)
{
	planarThreshold = threshold;
	//update UI
	if (iPlanarThreshold)
		iPlanarThreshold->SetValue(fnGetGeomPlanarModeThreshold(),TRUE);
}


void	UnwrapMod::fnSelectByMatID(int matID)
{
	//check for type
//	ModContextList mcList;		
//	INodeTab nodes;

	if (!ip) return;
//	ip->GetModContexts(mcList,nodes);

//	int objects = mcList.Count();

	matID--;

	theHold.Begin();
	HoldSelection();
	theHold.Accept (GetString (IDS_DS_SELECT));

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{

		mMeshTopoData[ldID]->SelectByMatID(matID);
	}

	theHold.Suspend();
	if (fnGetSyncSelectionMode()) fnSyncTVSelection();
	theHold.Resume();


	ComputeSelectedFaceData();
	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	InvalidateView();
	ip->RedrawViews(ip->GetTime());

	
}

void	UnwrapMod::fnSelectBySG(int sg)
{

	//check for type
//	ModContextList mcList;		
//	INodeTab nodes;

	if (!ip) return;
//	ip->GetModContexts(mcList,nodes);

//	int objects = mcList.Count();

	sg--;

	theHold.Begin();
	HoldSelection();
	theHold.Accept (GetString (IDS_DS_SELECT));

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *tmd = mMeshTopoData[ldID];


//		MeshTopoData *tmd = (MeshTopoData*)mcList[0]->localData;

		if (tmd == NULL) 
		{
			continue;
		}


		BitArray faceSel = tmd->GetFaceSelection();
		faceSel.ClearAll();

		sg = 1 << sg;
		if(tmd->GetMesh())
		{
			for (int i =0; i < tmd->GetMesh()->numFaces; i++)
			{
				if (tmd->GetMesh()->faces[i].getSmGroup() & sg) 
					faceSel.Set(i);
			}
		}
		else if (tmd->GetMNMesh())
		{
			for (int i =0; i < tmd->GetMNMesh()->numf; i++)
			{
				if (tmd->GetMNMesh()->f[i].smGroup & sg) 
					faceSel.Set(i);
			}
		}
		else if (tmd->GetPatch())
		{
			for (int i =0; i < tmd->GetPatch()->numPatches; i++)
			{
				if (tmd->GetPatch()->patches[i].smGroup & sg)
					faceSel.Set(i);
			}
		}
		tmd->SetFaceSelection(faceSel);
	}

	if (fnGetSyncSelectionMode()) 
		fnSyncTVSelection();

	ComputeSelectedFaceData();
	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	InvalidateView();
	ip->RedrawViews(ip->GetTime());


}



void	UnwrapMod::GeomExpandFaceSel(MeshTopoData* tmd)
{
	BitArray selectedVerts;

	if (tmd->GetMesh())
	{
		Mesh *mesh = tmd->GetMesh();
		BitArray faceSel = tmd->GetFaceSelection();

		selectedVerts.SetSize(mesh->getNumVerts());
		selectedVerts.ClearAll();
		for (int i =0; i < mesh->getNumFaces(); i++)
		{
			if (faceSel[i])
			{
				for (int j = 0;j < 3; j++)
				{
					int index = mesh->faces[i].v[j];
					selectedVerts.Set(index);
				}
			}
		}
		for (int i =0; i < mesh->getNumFaces(); i++)
		{
			if (!faceSel[i])
			{
				for (int j = 0;j < 3; j++)
				{
					int index = mesh->faces[i].v[j];
					if (selectedVerts[index])
						faceSel.Set(i);

				}
			}
		}
		tmd->SetFaceSelection(faceSel);

	}
	else if (tmd->GetPatch())
	{
		PatchMesh *patch = tmd->GetPatch();
		BitArray faceSel = tmd->GetFaceSelection();

		selectedVerts.SetSize(patch->getNumVerts());
		selectedVerts.ClearAll();
		for (int i =0; i < patch->getNumPatches(); i++)
		{
			if (faceSel[i])
			{
				int ct = 4;
				if (patch->patches[i].type == PATCH_TRI) ct = 3;
				for (int j = 0;j < ct; j++)
				{
					int index = patch->patches[i].v[j];
					selectedVerts.Set(index);
				}
			}
		}
		for (int i =0; i < patch->getNumPatches(); i++)
		{
			if (!faceSel[i])
			{
				int ct = 4;
				if (patch->patches[i].type == PATCH_TRI) ct = 3;
				for (int j = 0;j < ct; j++)
				{
					int index = patch->patches[i].v[j];
					if (selectedVerts[index])
						faceSel.Set(i);

				}
			}
		}
		tmd->SetFaceSelection(faceSel);
	}
	else if (tmd->GetMNMesh())
	{
		MNMesh *mnMesh = tmd->GetMNMesh();
		BitArray faceSel = tmd->GetFaceSelection();

		selectedVerts.SetSize(mnMesh->numv);
		selectedVerts.ClearAll();
		for (int i =0; i < mnMesh->numf; i++)
		{
			if (faceSel[i])
			{
				int ct = mnMesh->f[i].deg;
				for (int j = 0;j < ct; j++)
				{
					int index = mnMesh->f[i].vtx[j];
					selectedVerts.Set(index);
				}
			}
		}
		for (int i =0; i < mnMesh->numf; i++)
		{
			if (!faceSel[i])
			{
				int ct = mnMesh->f[i].deg;
				for (int j = 0;j < ct; j++)
				{
					int index = mnMesh->f[i].vtx[j];
					if (selectedVerts[index])
						faceSel.Set(i);

				}
			}
		}
		tmd->SetFaceSelection(faceSel);
	}

}
void	UnwrapMod::GeomContractFaceSel(MeshTopoData* tmd)
{

	BitArray unselectedVerts;

	if (tmd->GetMesh())
	{
		Mesh *mesh = tmd->GetMesh();
		BitArray faceSel = tmd->GetFaceSelection();
		unselectedVerts.SetSize(mesh->getNumVerts());
		unselectedVerts.ClearAll();

		for (int i =0; i < mesh->getNumFaces(); i++)
		{
			if (!faceSel[i])
			{
				for (int j = 0;j < 3; j++)
				{
					int index = mesh->faces[i].v[j];
					unselectedVerts.Set(index);
				}
			}
		}
		for (int i =0; i < mesh->getNumFaces(); i++)
		{
			if (faceSel[i])
			{
				for (int j = 0;j < 3; j++)
				{
					int index = mesh->faces[i].v[j];
					if (unselectedVerts[index])
					faceSel.Set(i,FALSE);

				}
			}
		}
		tmd->SetFaceSelection(faceSel);

	}
	else if (tmd->GetPatch())
	{
		PatchMesh *patch = tmd->GetPatch();
		BitArray faceSel = tmd->GetFaceSelection();

		unselectedVerts.SetSize(patch->getNumVerts());
		unselectedVerts.ClearAll();
		for (int i =0; i < patch->getNumPatches(); i++)
		{
			if (!faceSel[i])
			{
				int ct = 4;
				if (patch->patches[i].type == PATCH_TRI) ct = 3;
				for (int j = 0;j < ct; j++)
				{
					int index = patch->patches[i].v[j];
					unselectedVerts.Set(index);
				}
			}
		}
		for (int i =0; i < patch->getNumPatches(); i++)
		{
			if (faceSel[i])
			{
				int ct = 4;
				if (patch->patches[i].type == PATCH_TRI) ct = 3;
				for (int j = 0;j < ct; j++)
				{
					int index = patch->patches[i].v[j];
					if (unselectedVerts[index])
						faceSel.Set(i,FALSE);

				}
			}
		}
		tmd->SetFaceSelection(faceSel);
	}
	else if (tmd->GetMNMesh())
	{
		MNMesh *mnMesh = tmd->GetMNMesh();
		BitArray faceSel = tmd->GetFaceSelection();

		unselectedVerts.SetSize(mnMesh->numv);
		unselectedVerts.ClearAll();
		for (int i =0; i < mnMesh->numf; i++)
		{
			if (!faceSel[i])
			{
				int ct = mnMesh->f[i].deg;
				for (int j = 0;j < ct; j++)
				{
					int index = mnMesh->f[i].vtx[j];
					unselectedVerts.Set(index);
				}
			}
		}
		for (int i =0; i < mnMesh->numf; i++)
		{
			if (faceSel[i])
			{
				int ct = mnMesh->f[i].deg;
				for (int j = 0;j < ct; j++)
				{
					int index = mnMesh->f[i].vtx[j];
					if (unselectedVerts[index])
						faceSel.Set(i,FALSE);

				}
			}
		}
		tmd->SetFaceSelection(faceSel);
	}
}
void	UnwrapMod::fnGeomExpandFaceSel()
{
	//check for type
	ModContextList mcList;		
	INodeTab nodes;

	if (!ip) return;
/*	ip->GetModContexts(mcList,nodes);

	int objects = mcList.Count();

	if (objects != 0)
	{
*/
	theHold.Begin();
	HoldSelection();
	theHold.Accept(_T(GetString(IDS_DS_SELECT)));

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{


		MeshTopoData *tmd = mMeshTopoData[ldID];//(MeshTopoData*)mcList[0]->localData;

		if (tmd == NULL) 
		{
			continue;
		}
		GeomExpandFaceSel(tmd);

	}
	if (fnGetSyncSelectionMode()) 
		fnSyncTVSelection();

	ComputeSelectedFaceData();
	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	InvalidateView();
	ip->RedrawViews(ip->GetTime());

}
void	UnwrapMod::fnGeomContractFaceSel()
{
	if (!ip) return;
	theHold.Begin();
	HoldSelection();
	theHold.Accept(_T(GetString(IDS_DS_SELECT)));

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{


		MeshTopoData *tmd = mMeshTopoData[ldID];//(MeshTopoData*)mcList[0]->localData;

		if (tmd == NULL) 
		{
			continue;
		}
		GeomContractFaceSel(tmd);

	}
	if (fnGetSyncSelectionMode()) 
		fnSyncTVSelection();

	ComputeSelectedFaceData();
	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	InvalidateView();
	ip->RedrawViews(ip->GetTime());
}



BitArray* UnwrapMod::fnGetSelectedVerts(INode *node)
{
	MeshTopoData *ld = GetMeshTopoData(node);
	if (ld)
	{
		return ld->GetTVVertSelectionPtr();
	}
	return NULL;

}

BitArray* UnwrapMod::fnGetSelectedVerts()
{
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			return ld->GetTVVertSelectionPtr();
		}
	}
	return NULL;

}


void UnwrapMod::fnSelectVerts(BitArray *sel, INode *node)
{

		MeshTopoData *ld = GetMeshTopoData(node);
		if (ld)
		{
			theHold.Begin();
			theHold.Put(new TSelRestore(this,ld));
			theHold.Accept(_T(GetString(IDS_PW_SELECT_UVW)));

			BitArray vsel = ld->GetTVVertSelection();
			vsel.ClearAll();
			for (int i =0; i < vsel.GetSize(); i++)
			{
				if (i < sel->GetSize())
				{
					if ((*sel)[i]) 
						vsel.Set(i);
				}
			}
			ld->SetTVVertSelection(vsel);

			if (fnGetSyncSelectionMode()) fnSyncGeomSelection();

			InvalidateView();
		}
	
}

void UnwrapMod::fnSelectVerts(BitArray *sel)
{
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			theHold.Begin();
			theHold.Put(new TSelRestore(this,ld));
			theHold.Accept(_T(GetString(IDS_PW_SELECT_UVW)));

			BitArray vsel = ld->GetTVVertSelection();
			vsel.ClearAll();
			for (int i =0; i < vsel.GetSize(); i++)
			{
				if (i < sel->GetSize())
				{
					if ((*sel)[i]) 
						vsel.Set(i);
				}
			}
			ld->SetTVVertSelection(vsel);

			if (fnGetSyncSelectionMode()) fnSyncGeomSelection();

			InvalidateView();
		}
	}
}

BOOL UnwrapMod::fnIsVertexSelected(int index, INode *node)
{

	index--;

	MeshTopoData *ld = GetMeshTopoData(node);
	if (ld)
	{			
		return ld->GetTVVertSelected(index);
	}
	return FALSE;
}

BOOL UnwrapMod::fnIsVertexSelected(int index)
{

	index--;
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{			
			return ld->GetTVVertSelected(index);
		}
	}
	return FALSE;
}

BitArray* UnwrapMod::fnGetSelectedFaces(INode *node)
{
	MeshTopoData *ld = GetMeshTopoData(node);
	if (ld)
	{
		return ld->GetFaceSelectionPtr();
	}
	return NULL;
}

BitArray* UnwrapMod::fnGetSelectedFaces()
{
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			return ld->GetFaceSelectionPtr();
		}
	}
	return NULL;
}

void	UnwrapMod::fnSelectFaces(BitArray *sel,INode *node)
{

		MeshTopoData *ld = GetMeshTopoData(node);
		if (theHold.Holding())
			HoldSelection();

		if (ld)
		{
			BitArray fsel = ld->GetFaceSelection();
			fsel.ClearAll();
			for (int i =0; i < fsel.GetSize(); i++)
			{
				if (i < sel->GetSize())
				{
					if ((*sel)[i]) fsel.Set(i);
				}
			}
			ld->SetFaceSelection(fsel);

			if (fnGetSyncSelectionMode()) fnSyncGeomSelection();
			InvalidateView();
		}
		ComputeSelectedFaceData();
}

void	UnwrapMod::fnSelectFaces(BitArray *sel)
{

	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			if (theHold.Holding())
				HoldSelection();
			BitArray fsel = ld->GetFaceSelection();
			fsel.ClearAll();
			for (int i =0; i < fsel.GetSize(); i++)
			{
				if (i < sel->GetSize())
				{
					if ((*sel)[i]) fsel.Set(i);
				}
			}
			ld->SetFaceSelection(fsel);

			if (fnGetSyncSelectionMode()) fnSyncGeomSelection();
			InvalidateView();
		}
		ComputeSelectedFaceData();
	}
}

BOOL	UnwrapMod::fnIsFaceSelected(int index, INode *node)
{

	index--;

	MeshTopoData *ld = GetMeshTopoData(node);
	if (ld)
	{
		return ld->GetFaceSelected(index);
	}

	return FALSE;
}

BOOL	UnwrapMod::fnIsFaceSelected(int index)
{

	index--;

	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			return ld->GetFaceSelected(index);
		}
	}


	return FALSE;
}




void	UnwrapMod::TransferSelectionStart()
{
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		if (ld)
		{
			ld->TransferSelectionStart(fnGetTVSubMode());
		}
	}


}	

//this transfer our vertex selection into our curren selection
void	UnwrapMod::TransferSelectionEnd(BOOL partial,BOOL recomputeSelection)
{

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		if (ld)
		{
			ld->TransferSelectionEnd(fnGetTVSubMode(),partial,recomputeSelection);
		}
	}

}

BitArray* UnwrapMod::fnGetSelectedEdges(INode *node)
{
		MeshTopoData *ld = GetMeshTopoData(node);
		if (ld)
		{
			return ld->GetTVEdgeSelectionPtr();
		}
	
	return NULL;
}

BitArray* UnwrapMod::fnGetSelectedEdges()
{
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			return ld->GetTVEdgeSelectionPtr();
		}
	}
	
	return NULL;
}

void	UnwrapMod::fnSelectEdges(BitArray *sel, INode *node)
{
		MeshTopoData *ld = GetMeshTopoData(node);
		if (ld)
		{
			theHold.Begin();
			theHold.Put(new TSelRestore(this,ld));
			theHold.Accept(_T(GetString(IDS_PW_SELECT_UVW)));

			BitArray esel = ld->GetTVEdgeSelection();
			esel.ClearAll();
			for (int i =0; i < esel.GetSize(); i++)
			{
				if (i < sel->GetSize())
				{
					if ((*sel)[i]) esel.Set(i);
				}
			}
			ld->SetTVEdgeSelection(esel);
			if (fnGetSyncSelectionMode()) fnSyncGeomSelection();
			InvalidateView();

		}

}

void	UnwrapMod::fnSelectEdges(BitArray *sel)
{
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			theHold.Begin();
			theHold.Put(new TSelRestore(this,ld));
			theHold.Accept(_T(GetString(IDS_PW_SELECT_UVW)));

			BitArray esel = ld->GetTVEdgeSelection();
			esel.ClearAll();
			for (int i =0; i < esel.GetSize(); i++)
			{
				if (i < sel->GetSize())
				{
					if ((*sel)[i]) esel.Set(i);
				}
			}
			ld->SetTVEdgeSelection(esel);
			if (fnGetSyncSelectionMode()) fnSyncGeomSelection();
			InvalidateView();

		}
	}

}

BOOL	UnwrapMod::fnIsEdgeSelected(int index, INode *node)
{

	index--;
		MeshTopoData *ld = GetMeshTopoData(node);
		if (ld)
		{
			return ld->GetTVEdgeSelected(index);
		}
	return FALSE;
}

BOOL	UnwrapMod::fnIsEdgeSelected(int index)
{

	index--;
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			return ld->GetTVEdgeSelected(index);
		}
	}
	return FALSE;
}

BOOL	UnwrapMod::fnGetUVEdgeMode()
{
	return uvEdgeMode;
}
void	UnwrapMod::fnSetUVEdgeMode(BOOL uvmode)	
{
	if (uvmode)
	{
		tvElementMode = FALSE;
		openEdgeMode = FALSE;
	}
	uvEdgeMode = uvmode;
}

BOOL	UnwrapMod::fnGetTVElementMode()
{
	return tvElementMode;
}
void	UnwrapMod::fnSetTVElementMode(BOOL mode)
{
	if (mode)
	{
		uvEdgeMode = FALSE;
		openEdgeMode = FALSE;
	}
	tvElementMode =mode;
	MoveScriptUI();
	if ( (ip) &&(hWnd) )
	{	
		IMenuBarContext* pContext = (IMenuBarContext*) GetCOREInterface()->GetMenuManager()->GetContext(kUnwrapMenuBar);
		if (pContext)
			pContext->UpdateWindowsMenu();
	}
}

void	UnwrapMod::SelectUVEdge(BOOL selectOpenEdges)
{		
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];

		BitArray holdSelection = ld->GetTVEdgeSelection();
		BitArray finalSelection = holdSelection;
		finalSelection.ClearAll();
		
		Tab<int> selEdges;
		for (int i = 0; i < holdSelection.GetSize(); i++)
		{
			if (holdSelection[i])
				selEdges.Append(1,&i,1000);
		}

		for (int i = 0; i < selEdges.Count(); i++)
		{
			int edgeIndex = selEdges[i];
			int eselSet = 0;
			ld->ClearTVEdgeSelection();
			ld->SetTVEdgeSelected(edgeIndex,TRUE);

			while (eselSet!= ld->GetTVEdgeSelection().NumberSet())
			{
				eselSet =  ld->GetTVEdgeSelection().NumberSet();
				GrowUVLoop(selectOpenEdges);
				//get connecting a edge
			}
			finalSelection |= ld->GetTVEdgeSelection();
		}
		ld->SetTVEdgeSelection(finalSelection);
	}
}


void	UnwrapMod::SelectOpenEdge()
{	
	
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		int eselSet = 0;

		while (eselSet != ld->GetTVEdgeSelection().NumberSet())
		{
			eselSet =  ld->GetTVEdgeSelection().NumberSet();//esel.NumberSet();
			GrowSelectOpenEdge();
			//get connecting a edge
		}
	}
}


void	UnwrapMod::GrowSelectOpenEdge()
{
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];

		BitArray esel = ld->GetTVEdgeSelection();
		BitArray vsel = ld->GetTVVertSelection();

		int edgeCount = esel.GetSize();

		Tab<int> edgeCounts;
		edgeCounts.SetCount(ld->GetNumberTVVerts());//TVMaps.v.Count());
		for (int i = 0; i < ld->GetNumberTVVerts()/*TVMaps.v.Count()*/; i++)
			edgeCounts[i] = 0;
		for (int i = 0; i < edgeCount; i++)
		{
			int a = ld->GetTVEdgeVert(i,0);//TVMaps.ePtrList[i]->a;
			int b = ld->GetTVEdgeVert(i,1);//TVMaps.ePtrList[i]->b;
			if (esel[i])
			{
				edgeCounts[a]++;
				edgeCounts[b]++;
			}
		}
		for (int i = 0; i < edgeCount; i++)
		{
			if ( (!esel[i]) && (ld->GetTVEdgeNumberTVFaces(i)/*TVMaps.ePtrList[i]->faceList.Count()*/ == 1) )
			{
				int a = ld->GetTVEdgeVert(i,0);//TVMaps.ePtrList[i]->a;
				int b = ld->GetTVEdgeVert(i,1);//TVMaps.ePtrList[i]->b;
				int aCount = edgeCounts[a];
				int bCount = edgeCounts[b];
				if ( ( ( aCount == 0) && (bCount >= 1) ) ||
					( (bCount == 0) && (aCount >= 1) ) ||
					( (bCount >= 1) && (aCount >= 1) ) )
				{					
					esel.Set(i,TRUE);
				}
			}
		}
		ld->SetTVEdgeSelection(esel);
	}

}

void	UnwrapMod::ShrinkSelectOpenEdge()
{


	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];

		BitArray esel = ld->GetTVEdgeSelection();
		BitArray vsel = ld->GetTVVertSelection();
		int edgeCount = esel.GetSize();

		Tab<int> edgeCounts;
		edgeCounts.SetCount(ld->GetNumberTVVerts());//TVMaps.v.Count());
		for (int i = 0; i < ld->GetNumberTVVerts()/*TVMaps.v.Count()*/; i++)
			edgeCounts[i] = 0;
		for (int i = 0; i < edgeCount; i++)
		{
			int a = ld->GetTVEdgeVert(i,0);//TVMaps.ePtrList[i]->a;
			int b = ld->GetTVEdgeVert(i,1);//TVMaps.ePtrList[i]->b;
			if (esel[i])
			{
				edgeCounts[a]++;
				edgeCounts[b]++;
			}
		}
		for (int i = 0; i < edgeCount; i++)
		{
			if ( (esel[i])  && (ld->GetTVEdgeNumberTVFaces(i)/*TVMaps.ePtrList[i]->faceList.Count()*/ == 1) )
			{
				int a = ld->GetTVEdgeVert(i,0);//TVMaps.ePtrList[i]->a;
				int b = ld->GetTVEdgeVert(i,1);//TVMaps.ePtrList[i]->b;
				if ( (edgeCounts[a] == 1) || (edgeCounts[b] == 1) )
					esel.Set(i,FALSE);
			}
		}
		ld->SetTVEdgeSelection(esel);
	}

}


void	UnwrapMod::GrowUVLoop(BOOL selectOpenEdges)
{


	// get current edgesel
	Tab<int> edgeConnectedCount;
	Tab<int> numberEdgesAtVert;

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];

		BitArray esel = ld->GetTVEdgeSelection();
		BitArray vsel = ld->GetTVVertSelection();

		int edgeCount = esel.GetSize();
		int vertCount = vsel.GetSize();  

		BitArray openEdgeSels = esel;
		openEdgeSels.ClearAll();

		if (!selectOpenEdges)
		{
			for (int i = 0; i < edgeCount; i++)
			{
				if (esel[i])
				{
					if (ld->GetTVEdgeNumberTVFaces(i) <= 1)
					{
						openEdgeSels.Set(i,TRUE);
						esel.Set(i,FALSE);
					}
				}
			}
		}




		edgeConnectedCount.SetCount(vertCount);
		numberEdgesAtVert.SetCount(vertCount);

		for (int i = 0; i < vertCount; i++)
		{
			edgeConnectedCount[i] = 0;
			numberEdgesAtVert[i] = 0;
		}

		//find all the vertices that touch a selected edge
		// and keep a count of the number of selected edges that touch that //vertex  
		for (int i = 0; i < edgeCount; i++)
		{
			int a = ld->GetTVEdgeVert(i,0);//TVMaps.ePtrList[i]->a;
			int b = ld->GetTVEdgeVert(i,1);//TVMaps.ePtrList[i]->b;
			if (a!=b)
			{

				if (!(ld->GetTVEdgeHidden(i)))//TVMaps.ePtrList[i]->flags & FLAG_HIDDENEDGEA))
				{
					numberEdgesAtVert[a]++;
					numberEdgesAtVert[b]++;
				}
				if (esel[i])
				{
					edgeConnectedCount[a]++;
					edgeConnectedCount[b]++;
				}
			}
		}


		BitArray edgesToExpand;
		edgesToExpand.SetSize(edgeCount);
		edgesToExpand.ClearAll();

		//tag any edge that has only one vertex count since that will be an end edge  
		for (int i = 0; i < edgeCount; i++)
		{
			int a = ld->GetTVEdgeVert(i,0);//TVMaps.ePtrList[i]->a;
			int b = ld->GetTVEdgeVert(i,1);//TVMaps.ePtrList[i]->b;
			if (a!=b)
			{
				if (esel[i])
				{
					if ((edgeConnectedCount[a] == 1) || (edgeConnectedCount[b] == 1))
						edgesToExpand.Set(i,TRUE);
				}
			}
		}

		for (int i = 0; i < edgeCount; i++)
		{
			if (edgesToExpand[i])
			{
				//make sure we have an even number of edges at this vert
				//if odd then we can not go further
				//   if ((numberEdgesAtVert[i] % 2) == 0)
				//now need to find our most opposing edge
				//find all edges connected to the vertex

				for (int k = 0; k < 2; k++)
				{
					int a = 0;
					if (k==0) a = ld->GetTVEdgeVert(i,0);//TVMaps.ePtrList[i]->a;
					else a = ld->GetTVEdgeVert(i,1);//TVMaps.ePtrList[i]->b;

					int oddCount = (numberEdgesAtVert[a] % 2);

					
					if ((ld->GetTVEdgeNumberTVFaces(i) <= 1) && selectOpenEdges)
					{
						if (numberEdgesAtVert[a]==3)
							oddCount = 0;
					}

					if ( (edgeConnectedCount[a] == 1) && (oddCount == 0))
					{
						int centerVert = a;
						Tab<int> edgesConnectedToVert;
						for (int j = 0; j < edgeCount; j++)
						{
							if (j!=i)  //make sure we dont add our selected vertex
							{
								int a = ld->GetTVEdgeVert(j,0);//TVMaps.ePtrList[j]->a;
								int b = ld->GetTVEdgeVert(j,1);//TVMaps.ePtrList[j]->b;

								if (a!=b)
								{
									if ((a == centerVert) || (b==centerVert))
									{
										edgesConnectedToVert.Append(1,&j);
									}
								}
							}

						}
						//get a face connected to our oririnal egd
						int faceIndex = ld->GetTVEdgeConnectedTVFace(i,0);//TVMaps.ePtrList[i]->faceList[0];
						int count = numberEdgesAtVert[centerVert]/2;
						if ((ld->GetTVEdgeNumberTVFaces(i) <= 1) && selectOpenEdges)
						{
							count = 2;
						}
						int tally = 0;
						BOOL done = FALSE;
						while (!done)
						{
							int lastEdge = -1;


							for (int m = 0; m < edgesConnectedToVert.Count(); m++)
							{
								int edgeIndex = edgesConnectedToVert[m];
								for (int n = 0; n < ld->GetTVEdgeNumberTVFaces(edgeIndex)/*TVMaps.ePtrList[edgeIndex]->faceList.Count()*/; n++)
								{
									if (faceIndex == ld->GetTVEdgeConnectedTVFace(edgeIndex,n))//TVMaps.ePtrList[edgeIndex]->faceList[n])
									{
										for (int p = 0; p < ld->GetTVEdgeNumberTVFaces(edgeIndex)/*TVMaps.ePtrList[edgeIndex]->faceList.Count()*/; p++)
										{
											if (faceIndex != ld->GetTVEdgeConnectedTVFace(edgeIndex,p))//TVMaps.ePtrList[edgeIndex]->faceList[p])
											{
												faceIndex = ld->GetTVEdgeConnectedTVFace(edgeIndex,p);//TVMaps.ePtrList[edgeIndex]->faceList[p];
												p = ld->GetTVEdgeNumberTVFaces(edgeIndex);//TVMaps.ePtrList[edgeIndex]->faceList.Count();
											}
										}
										if (!(ld->GetTVEdgeHidden(edgeIndex)))//TVMaps.ePtrList[edgeIndex]->flags& FLAG_HIDDENEDGEA))
											tally++;
										edgesConnectedToVert.Delete(m,1);
										m = edgesConnectedToVert.Count();
										n = ld->GetTVEdgeNumberTVFaces(edgeIndex);//TVMaps.ePtrList[edgeIndex]->faceList.Count();

										lastEdge = edgeIndex;
									}
								}
							}
							if (lastEdge == -1)
							{
								//					        assert(0);
								done = TRUE;
							}
							if (tally >= count)
							{
								done = TRUE;
								if (lastEdge != -1)
									esel.Set(lastEdge,TRUE);
							}

						}
					}
				}
			}
		}	
		esel |= openEdgeSels;
		ld->SetTVEdgeSelection(esel);
	}

}



void	UnwrapMod::ShrinkUVLoop()
{

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];

		BitArray esel = ld->GetTVEdgeSelection();
		BitArray vsel = ld->GetTVVertSelection();



		int edgeCount = esel.GetSize();

		Tab<int> edgeCounts;
		edgeCounts.SetCount(ld->GetNumberTVVerts());
		for (int i = 0; i < ld->GetNumberTVVerts(); i++)
			edgeCounts[i] = 0;
		for (int i = 0; i < edgeCount; i++)
		{
			int a = ld->GetTVEdgeVert(i,0);
			int b = ld->GetTVEdgeVert(i,1);
			if (esel[i])
			{
				edgeCounts[a]++;
				edgeCounts[b]++;
			}
		}
		for (int i = 0; i < edgeCount; i++)
		{
			if ( esel[i] )
			{
				int a = ld->GetTVEdgeVert(i,0);
				int b = ld->GetTVEdgeVert(i,1);
				if ( (edgeCounts[a] == 1) || (edgeCounts[b] == 1) )
					esel.Set(i,FALSE);
			}
		}
		ld->SetTVEdgeSelection(esel);
	}
}

void	UnwrapMod::GrowUVRing(bool doall)
{
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		BitArray tempGeSel;
		MeshTopoData *ld = mMeshTopoData[ldID];
		BitArray gesel = ld->GetTVEdgeSelection();
		tempGeSel = gesel;
		tempGeSel.ClearAll();


		//get the selected edge
		for (int i = 0; i < ld->GetNumberTVEdges(); i++)
		{
			if (gesel[i])
			{
				//get the face attached to that edge
				for (int j = 0; j < ld->GetTVEdgeNumberTVFaces(i); j++)
				{
					//get all the visible edges attached to that face
					int currentFace = ld->GetTVEdgeConnectedTVFace(i,j);

					int currentEdge = i;

					BOOL done = FALSE;
					int startEdge = currentEdge;
					BitArray edgesForThisFace;
					edgesForThisFace.SetSize(ld->GetNumberTVEdges());


					Tab<int> facesToProcess;
					BitArray processedFaces;
					processedFaces.SetSize(ld->GetNumberFaces());
					processedFaces.ClearAll();


					while (!done)
					{
						edgesForThisFace.ClearAll();
						facesToProcess.Append(1,&currentFace,100);
						while (facesToProcess.Count() > 0)
						{
							//pop the stack
							currentFace = facesToProcess[0];
							facesToProcess.Delete(0,1);

							processedFaces.Set(currentFace,TRUE);
							int numberOfEdges = ld->GetFaceDegree(currentFace);
							//loop through the edges
							for (int k = 0; k < numberOfEdges; k++)
							{
								//if edge is invisisble add the edges of the cross face
								int a = ld->GetFaceTVVert(currentFace,k);
								int b = ld->GetFaceTVVert(currentFace,(k+1)%numberOfEdges);
								if (a!=b)
								{
									int eindex = ld->FindUVEdge(a,b);
									if (!( ld->GetTVEdgeHidden(eindex)))
									{
										edgesForThisFace.Set(eindex,TRUE);
									}
									else
									{
										for (int m = 0; m < ld->GetTVEdgeNumberTVFaces(eindex); m++)
										{
											int faceIndex = ld->GetTVEdgeConnectedTVFace(eindex,m);
											if (!processedFaces[faceIndex])
											{
												facesToProcess.Append(1,&faceIndex,100);
											}
										}
									}
								}

							}
						}
						//if odd edge count we are done
						if ( ((edgesForThisFace.NumberSet()%2) == 1) || (edgesForThisFace.NumberSet() <= 2))
							done = TRUE;
						else
						{
							//get the mid edge 
							//start at the seed
							int goal = edgesForThisFace.NumberSet()/2;
							int currentGoal = 0;
							int vertIndex = ld->GetTVEdgeVert(currentEdge,0);
							Tab<int> edgeList;
							for (int m = 0; m < edgesForThisFace.GetSize(); m++)
							{
								if (edgesForThisFace[m])
									edgeList.Append(1,&m,100);

							}


							int holdCurrentEdge = currentEdge;

							//find next edge
							while (currentGoal != goal)
							{

								BOOL noHit = TRUE;
								for (int i = 0; i < edgeList.Count(); i++)
								{

									int potentialEdge = edgeList[i];
									if (potentialEdge != currentEdge)
									{
										int a = ld->GetTVEdgeVert(potentialEdge,0);
										int b = ld->GetTVEdgeVert(potentialEdge,1);
										if (a == vertIndex)
										{
											vertIndex = b;
											currentEdge = potentialEdge;
											i = edgeList.Count();

											//increment current
											currentGoal++;
											noHit = FALSE;
										}
										else if (b == vertIndex)
										{
											vertIndex = a;
											currentEdge = potentialEdge;
											i = edgeList.Count();

											//increment current
											currentGoal++;
											noHit = FALSE;
										}

									}
								}
								//this is a case where we have a hidden edge on the border which breaks the border loop
								//in this case we can just bail since the loop is incomplete and we are done
								if (noHit)
								{
									currentGoal = goal;
									done = TRUE;
									currentEdge = holdCurrentEdge;
								}
							}

						}

						if (tempGeSel[currentEdge])
							done = TRUE;
						tempGeSel.Set(currentEdge,TRUE);

						for (int m = 0; m < ld->GetTVEdgeNumberTVFaces(currentEdge); m++)
						{
							int faceIndex = ld->GetTVEdgeConnectedTVFace(currentEdge,m);
							if ((faceIndex != currentFace) && (!processedFaces[faceIndex]))
							{
								currentFace = faceIndex;
								m = ld->GetTVEdgeNumberTVFaces(currentEdge);
							}
						}
						if (ld->GetTVEdgeNumberTVFaces(currentEdge) == 1)
							done = TRUE;
						//if we hit the start egde we are done
						if (currentEdge == startEdge) 
							done = TRUE;

						if ( !doall)
							done = TRUE;
					}


				}
			}
		}

		gesel = gesel | tempGeSel;
		ld->SetTVEdgeSelection(gesel);
	}


}
void	UnwrapMod::ShrinkUVRing()
{
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];


		BitArray esel = ld->GetTVEdgeSelection();
		BitArray vsel;
		vsel.SetSize(ld->GetNumberTVVerts());
		vsel.ClearAll();

		Tab<int> connectionCount;
		connectionCount.SetCount(ld->GetNumberTVVerts());
		for (int i = 0; i < ld->GetNumberTVVerts(); i++)
			connectionCount[i] = 0;


		for (int i = 0; i < ld->GetNumberTVEdges(); i++)
		{
			if (!ld->GetTVEdgeHidden(i) && esel[i])
			{
				for (int j = 0; j < 2; j++)
				{
					int tid = ld->GetTVEdgeVert(i,j);
					vsel.Set(tid,TRUE);
				}
			}
		}

		for (int i = 0; i < ld->GetNumberTVEdges(); i++)
		{
			if (!ld->GetTVEdgeHidden(i) && !esel[i])
			{
				int tid0 = ld->GetTVEdgeVert(i,0);
				int tid1 = ld->GetTVEdgeVert(i,1);
				if (vsel[tid0] && vsel[tid1])
				{
					connectionCount[tid0] += 1;
					connectionCount[tid1] += 1;
				}
			}
		}

		for (int i = 0; i < ld->GetNumberTVEdges(); i++)
		{
			if (!ld->GetTVEdgeHidden(i) && esel[i])
			{
				int tid0 = ld->GetTVEdgeVert(i,0);
				int tid1 = ld->GetTVEdgeVert(i,1);
				if (connectionCount[tid0] < 2 || 
					connectionCount[tid1] < 2 )
				{
					esel.Set(i,FALSE);
				}
			}
		}

		ld->SetTVEdgeSelection(esel);
	}
}


BOOL	UnwrapMod::fnGetOpenEdgeMode()
{
	return openEdgeMode;
}

void	UnwrapMod::fnSetOpenEdgeMode(BOOL mode)
{
	if (mode)
	{
		uvEdgeMode = FALSE;
		tvElementMode =FALSE;
	}

	openEdgeMode = mode;
}

void	UnwrapMod::fnUVEdgeSelect()
{
	theHold.Begin();
	HoldSelection();

	SelectUVEdge(FALSE);
	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();
	theHold.Accept(GetString(IDS_DS_SELECT));	
	InvalidateView();			
}

void	UnwrapMod::fnOpenEdgeSelect()
{
	theHold.Begin();
	HoldSelection();

	SelectOpenEdge();
	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();
	theHold.Accept(GetString(IDS_DS_SELECT));					
	InvalidateView();
}


void	UnwrapMod::fnVertToEdgeSelect()
{
	theHold.Begin();
	HoldSelection();

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		BitArray esel = ld->GetTVEdgeSelection();
		ld->GetEdgeSelFromVert(esel,FALSE);
		ld->SetTVEdgeSelection(esel);
	}
	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();
	theHold.Accept(GetString(IDS_DS_SELECT));	
	InvalidateView();
}
void	UnwrapMod::fnVertToFaceSelect()
{
	theHold.Begin();
	HoldSelection();

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		BitArray fsel = ld->GetFaceSelection();
		ld->GetFaceSelFromVert(fsel,FALSE);
		ld->SetFaceSelection(fsel);
	}

	if (fnGetSyncSelectionMode()) 
		fnSyncGeomSelection();

	theHold.Accept(GetString(IDS_DS_SELECT));	
	InvalidateView();
}

void	UnwrapMod::fnEdgeToVertSelect()
{
	theHold.Begin();
	HoldSelection();
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		BitArray vsel = ld->GetTVVertSelection();
		ld->GetVertSelFromEdge(vsel);
		ld->SetTVVertSelection(vsel);
	}

	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();

	theHold.Accept(GetString(IDS_DS_SELECT));	
	InvalidateView();	
}
void	UnwrapMod::fnEdgeToFaceSelect()
{
	theHold.Begin();
	HoldSelection();

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		BitArray vsel = ld->GetTVVertSelection();
		BitArray fsel = ld->GetFaceSelection();
		BitArray tempSel;
		tempSel.SetSize(vsel.GetSize());
		tempSel = vsel;
		ld->GetVertSelFromEdge(vsel);
		ld->SetTVVertSelection(vsel);
		ld->GetFaceSelFromVert(fsel,FALSE);

//		vsel = tempSel;
		ld->SetTVVertSelection(tempSel);
		ld->SetFaceSelection(fsel);
	}

	if (fnGetSyncSelectionMode()) 
		fnSyncGeomSelection();

	theHold.Accept(GetString(IDS_DS_SELECT));	
	InvalidateView();
}

void	UnwrapMod::fnFaceToVertSelect()
{
	theHold.Begin();
	HoldSelection();

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		BitArray vsel = ld->GetTVVertSelection();
		ld->GetVertSelFromFace(vsel);
		ld->SetTVVertSelection(vsel);
	}

	if (fnGetSyncSelectionMode()) 
		fnSyncGeomSelection();

	theHold.Accept(GetString(IDS_DS_SELECT));	
	InvalidateView();
}

void	UnwrapMod::fnFaceToEdgeSelect()
{
	theHold.Begin();
	HoldSelection();

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		ld->ConvertFaceToEdgeSel();
	}

	if (fnGetSyncSelectionMode()) 
		fnSyncGeomSelection();

	theHold.Accept(GetString(IDS_DS_SELECT));	
	InvalidateView();
}


void UnwrapMod::InitReverseSoftData()
{


	RebuildDistCache();

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		mMeshTopoData[ldID]->InitReverseSoftData();
	}

}
void UnwrapMod::ApplyReverseSoftData()
{

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		mMeshTopoData[ldID]->ApplyReverseSoftData(this);
	}
}


int		UnwrapMod::fnGetHitSize()
{
	return hitSize;
}
void	UnwrapMod::fnSetHitSize(int size)
{
	hitSize = size;
}


BOOL	UnwrapMod::fnGetPolyMode()
{
	return polyMode;
}
void	UnwrapMod::fnSetPolyMode(BOOL pmode)
{
	polyMode = pmode;
}


void	UnwrapMod::fnPolySelect()
{
	fnPolySelect2(TRUE);
}

void	UnwrapMod::fnPolySelect2(BOOL add)
{

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		
		BitArray esel = ld->GetTVEdgeSelection();
		BitArray vsel = ld->GetTVVertSelection();
		BitArray fsel = ld->GetFaceSelection();

		BitArray originalESel(esel);
		BitArray originalVSel(vsel);

		BitArray tempSel;
		//convert to edge sel
		ld->ConvertFaceToEdgeSel();
		esel = ld->GetTVEdgeSelection();

		//repeat until selection  not done
		int selCount = fsel.NumberSet();
		BOOL done= FALSE;
		int eSelCount = esel.GetSize();
		while (!done)
		{
			for (int i =0; i < eSelCount; i++)
			{
				if ( (esel[i])  && (ld->GetTVEdgeHidden(i)))//TVMaps.ePtrList[i]->flags&FLAG_HIDDENEDGEA))
				{
					int ct = ld->GetTVEdgeNumberTVFaces(i);//TVMaps.ePtrList[i]->faceList.Count();
					BOOL unselFace = FALSE;
					for (int j = 0; j < ct; j++)
					{
						int index = ld->GetTVEdgeConnectedTVFace(i,j);//TVMaps.ePtrList[i]->faceList[j];
						if (add)
							fsel.Set(index);				
						else						
						{
							if (!fsel[index])
								unselFace = TRUE;

						}
					}
					if (unselFace && (!add))
					{
						for (int j = 0; j < ct; j++)
						{
							int index = ld->GetTVEdgeConnectedTVFace(i,j);//TVMaps.ePtrList[i]->faceList[j];

							fsel.Set(index,FALSE);				
						}
					}
				}
			}

			if (selCount == fsel.NumberSet()) 
				done = TRUE;
			else
			{
				selCount = fsel.NumberSet();
				ld->SetFaceSelection(fsel);
				ld->ConvertFaceToEdgeSel();
				esel = ld->GetTVEdgeSelection();

			}
		}

		esel = originalESel;
		vsel = originalVSel;

		ld->SetTVEdgeSelection(esel);
		ld->SetTVVertSelection(vsel);
		ld->SetFaceSelection(fsel);
	}

	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();

	InvalidateView();

}


BOOL	UnwrapMod::fnGetSyncSelectionMode()
{
	return TRUE;
	//	return syncSelection;
}

void	UnwrapMod::fnSetSyncSelectionMode(BOOL sync)
{
	syncSelection = sync;
}


void	UnwrapMod::SyncGeomToTVSelection(MeshTopoData *ld)

{

	//get our geom face sel
	if (fnGetTVSubMode() == TVVERTMODE)
	{
		ld->ClearGeomVertSelection();//gvsel.ClearAll();
		//loop through our faces
		for (int i = 0; i < ld->GetNumberFaces()/*TVMaps.f.Count()*/; i++)
		{
			if (!ld->GetFaceDead(i))
			{
				//iterate through the faces
				int deg = ld->GetFaceDegree(i);//TVMaps.f[i]->count;
				for (int j  = 0; j < deg; j++)
				{
					//get our geom index
					int geomIndex = ld->GetFaceGeomVert(i,j);//TVMaps.f[i]->v[j];
					//get our tv index
					int tvIndex = ld->GetFaceTVVert(i,j);//TVMaps.f[i]->t[j];
					//if geom index is selected select the tv index
					if (ld->GetTVVertSelected(tvIndex))//vsel[tvIndex])
						ld->SetGeomVertSelected(geomIndex,TRUE);
	//					gvsel.Set(geomIndex,TRUE);
				}
			}
		}

	}

	else if (fnGetTVSubMode() == TVEDGEMODE)
	{
		Tab<GeomToTVEdges*> edgeInfo;
		edgeInfo.SetCount(ld->GetNumberFaces());//TVMaps.f.Count());
		for (int i = 0; i < ld->GetNumberFaces()/*TVMaps.f.Count()*/; i++)
		{
			edgeInfo[i] = new GeomToTVEdges();
			int deg = ld->GetFaceDegree(i);//TVMaps.f[i]->count;
			edgeInfo[i]->edgeInfo.SetCount(deg);
			for (int j = 0; j < deg; j++)
			{
				edgeInfo[i]->edgeInfo[j].gIndex = -1;
				edgeInfo[i]->edgeInfo[j].tIndex = -1;
			}
		}
		//loop through the geo edges
		for (int i = 0; i < ld->GetNumberGeomEdges()/*TVMaps.gePtrList.Count()*/; i++)
		{
			int numberOfFaces =  ld->GetGeomEdgeNumberOfConnectedFaces(i);///TVMaps.gePtrList[i]->faceList.Count();
			for (int j = 0; j < numberOfFaces; j++)
			{
				int faceIndex = ld->GetGeomEdgeConnectedFace(i,j);//TVMaps.gePtrList[i]->faceList[j];
				int a = ld->GetGeomEdgeVert(i,0);//TVMaps.gePtrList[i]->a;
				int b = ld->GetGeomEdgeVert(i,1);//TVMaps.gePtrList[i]->b;
				int ithEdge = ld->FindGeomEdge(faceIndex,a,b);//TVMaps.f[faceIndex]->FindGeomEdge(a, b);
				edgeInfo[faceIndex]->edgeInfo[ithEdge].gIndex = i;

			}
		}

		//loop through the uv edges
		for (int i = 0; i < ld->GetNumberTVEdges()/*TVMaps.ePtrList.Count()*/; i++)
		{
			int numberOfFaces =  ld->GetTVEdgeNumberTVFaces(i);//TVMaps.ePtrList[i]->faceList.Count();
			for (int j = 0; j < numberOfFaces; j++)
			{
				int faceIndex = ld->GetTVEdgeConnectedTVFace(i,j);//TVMaps.ePtrList[i]->faceList[j];
				int a = ld->GetTVEdgeVert(i,0);//TVMaps.ePtrList[i]->a;
				int b = ld->GetTVEdgeVert(i,1);//TVMaps.ePtrList[i]->b;
				int ithEdge = ld->FindUVEdge(faceIndex,a,b);//TVMaps.f[faceIndex]->FindUVEdge(a, b);
				edgeInfo[faceIndex]->edgeInfo[ithEdge].tIndex = i;

			}
		}

		ld->ClearGeomEdgeSelection();
//		gesel.ClearAll();
		for (int i = 0; i < ld->GetNumberFaces()/*TVMaps.f.Count()*/; i++)
		{

			int deg = ld->GetFaceDegree(i);//TVMaps.f[i]->count;
			for (int j = 0; j < deg; j++)
			{
				int gIndex = edgeInfo[i]->edgeInfo[j].gIndex;
				int tIndex = edgeInfo[i]->edgeInfo[j].tIndex;
				if ( (gIndex>=0) && (gIndex < ld->GetNumberGeomEdges()/*gesel.GetSize()*/) && 
					(tIndex>=0) && (tIndex < ld->GetNumberTVEdges()/*esel.GetSize()*/) && 
					ld->GetTVEdgeSelected(tIndex))
//					(esel[tIndex]) )
					ld->SetGeomEdgeSelected(gIndex,TRUE);
//					gesel.Set(gIndex,TRUE);

			}

		}

		for (int i = 0; i < ld->GetNumberFaces()/*TVMaps.f.Count()*/; i++)
		{
			delete edgeInfo[i];
		}

	}
	else if (fnGetTVSubMode() == TVFACEMODE)
	{
//		md->faceSel = fsel;
	}

}

void	UnwrapMod::SyncTVToGeomSelection(MeshTopoData *ld)
{


	if (ip == NULL) return;
	//get our geom face sel
	if (fnGetTVSubMode() == TVVERTMODE)
	{
		ld->ClearTVVertSelection();//vsel.ClearAll();
		//loop through our faces
		for (int i = 0; i < ld->GetNumberFaces(); i++)
		{
			//iterate through the faces
			int deg = ld->GetFaceDegree(i);//TVMaps.f[i]->count;
			for (int j  = 0; j < deg; j++)
			{
				//get our geom index
				int geomIndex = ld->GetFaceGeomVert(i,j);//TVMaps.f[i]->v[j];
				//get our tv index
				int tvIndex = ld->GetFaceTVVert(i,j);//TVMaps.f[i]->t[j];
				//if geom index is selected select the tv index
				if (ld->GetGeomVertSelected(geomIndex)/*gvsel[geomIndex]*/)
				{
					ld->SetTVVertSelected(tvIndex,TRUE);//vsel.Set(tvIndex,TRUE);
					if (ld->GetTVVertFrozen(tvIndex)/*TVMaps.v[tvIndex].flags & FLAG_FROZEN*/)
					{
						ld->SetTVVertSelected(tvIndex,FALSE);
						ld->SetGeomVertSelected(geomIndex,FALSE);
/*
						vsel.Set(tvIndex,FALSE);
						gvsel.Set(geomIndex,FALSE);
*/
					}
				}
			}
		}


	}
	else if (fnGetTVSubMode() == TVEDGEMODE)
	{
		BitArray uvEdgeSel = ld->GetTVEdgeSelection();
		ld->ConvertGeomEdgeSelectionToTV(ld->GetGeomEdgeSelection(), uvEdgeSel );
		ld->SetTVEdgeSelection(uvEdgeSel);


	}
	else if (fnGetTVSubMode() == TVFACEMODE)
	{
		BitArray fsel = ld->GetFaceSelection();//md->faceSel;
		for (int i = 0; i < ld->GetNumberFaces()/*TVMaps.f.Count()*/; i++)
		{
			if (!ld->GetFaceDead(i))
			{
				int deg = ld->GetFaceDegree(i);//TVMaps.f[i]->count;
				BOOL frozen = FALSE;
				for (int j = 0; j < deg; j++)
				{
					int index = ld->GetFaceTVVert(i,j);//TVMaps.f[i]->t[j];
					if (ld->GetTVVertFrozen(index))//TVMaps.v[index].flags & FLAG_FROZEN)
						frozen = TRUE;
				}
				if (frozen)
				{
					fsel.Set(i,FALSE);
	//				md->faceSel.Set(i,FALSE);
				}
			}
		}
		ld->SetFaceSelection(fsel);
	}

}


void	UnwrapMod::fnSyncTVSelection()
{
	if (!ip) return;

	// LAM - 6/19/04 - defect 576948 
	BOOL wasHolding = theHold.Holding();
	if (!wasHolding )
		theHold.Begin();

	HoldSelection();


/*
	//check for type
	ModContextList mcList;		
	INodeTab nodes;

	ip->GetModContexts(mcList,nodes);

	int objects = mcList.Count();
*/

//	if (objects != 0)
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{

		MeshTopoData *md = mMeshTopoData[ldID];//(MeshTopoData*)mcList[0]->localData;
		SyncTVToGeomSelection(md);
	}
	InvalidateView();

	if (!wasHolding )
		theHold.Accept(GetString(IDS_DS_SELECT));					


}

void	UnwrapMod::fnSyncGeomSelection()
{

	if (!ip) return;

	// LAM - 6/19/04 - defect 576948 
	BOOL wasHolding = theHold.Holding();
	if (!wasHolding )
		theHold.Begin();

	HoldSelection();
	//check for type
/*
	ModContextList mcList;		
	INodeTab nodes;

	ip->GetModContexts(mcList,nodes);

	int objects = mcList.Count();


	if (objects != 0)
*/	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *md = mMeshTopoData[ldID];//MeshTopoData *md = (MeshTopoData*)mcList[0]->localData;

		if (md)
		{
			SyncGeomToTVSelection(md);
		}
	}


	if (!wasHolding )
		theHold.Accept(GetString(IDS_DS_SELECT));					
	InvalidateView();
	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	if (ip)	ip->RedrawViews(ip->GetTime());

}


BOOL	UnwrapMod::fnGetPaintMode()
{
	if (mode==ID_PAINTSELECTMODE)
		return TRUE;
	else return FALSE;
}
void	UnwrapMod::fnSetPaintMode(BOOL paint)
{
	if (paint)
	{
		if (ip) ip->ReplacePrompt( GetString(IDS_PW_PAINTSELECTPROMPT));
		SetMode(ID_PAINTSELECTMODE);
		paintSelectMode->first = TRUE;
	}
	else SetMode(oldMode);

}


int		UnwrapMod::fnGetPaintSize()
{
	return paintSize ;
}

void	UnwrapMod::fnSetPaintSize(int size)
{

	paintSize = size;

	if (paintSize < 1) paintSize = 1;


}

void	UnwrapMod::fnIncPaintSize()
{
	paintSize++;
	if (paintSize < 1) paintSize = 1;

}
void	UnwrapMod::fnDecPaintSize()
{
	paintSize--;
	if (paintSize < 1) paintSize = 1;

}


void	UnwrapMod::fnSelectInvertedFaces()
{

	//see if face mode if not bail
	if (fnGetTVSubMode() != TVFACEMODE) 
		return;

	theHold.Begin();
	HoldSelection();
	theHold.Accept(_T(GetString(IDS_DS_SELECT)));

	for (int ldID= 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		BitArray fsel = ld->GetFaceSelection();
		//clear our selection
		fsel.ClearAll();
		//loop through our faces
		for (int i = 0; i < ld->GetNumberFaces(); i++)//TVMaps.f.Count(); i++)
		{
			//get the normal of that face
			int deg = ld->GetFaceDegree(i);//TVMaps.f[i]->count;

			BOOL hidden = FALSE;
			int hiddenCT = 0;
			for (int j = 0; j < deg; j++)
			{
				int index = ld->GetFaceTVVert(i,j);//TVMaps.f[i]->t[j];
				if ((ld->GetTVVertFrozen(index)) || (!ld->IsTVVertVisible(index)) )
//				if ((TVMaps.v[index].flags & FLAG_FROZEN) || (!IsVertVisible(index)) )
					hiddenCT++;
			}

			if (hiddenCT == deg) 
				hidden = TRUE;

			if (ld->IsFaceVisible(i) && (!hidden))
			{

				Point3 vecA,vecB;
				int a,b;

				BOOL validFace = FALSE;
				for (int j = 0; j < deg; j++)
				{
					int a1,a2,a3;
					a1 = j - 1;
					a2 = j;
					a3 = j + 1;

					if (j == 0)
						a1 = deg-1;
					if (j == (deg-1))
						a3 = 0;

					a = ld->GetFaceTVVert(i,a2);//TVMaps.f[i]->t[a2];
					b = ld->GetFaceTVVert(i,a1);//TVMaps.f[i]->t[a1];
					vecA = Normalize(ld->GetTVVert(b) - ld->GetTVVert(a));
//					vecA = Normalize(TVMaps.v[b].p - TVMaps.v[a].p);

					a = ld->GetFaceTVVert(i,a2);//TVMaps.f[i]->t[a2];
					b = ld->GetFaceTVVert(i,a3);//TVMaps.f[i]->t[a3];
					vecB = Normalize(ld->GetTVVert(b) - ld->GetTVVert(a));
					float dot = DotProd(vecA,vecB);
					if (dot < 1.0f)
					{
						j = deg;				
						validFace = TRUE;
					}
					else
					{
					}
				}
				if (validFace)
				{
					//if it is negative flip it
					Point3 vec = CrossProd(vecA,vecB);
					if (vec.z >= 0.0f)
						fsel.Set(i,TRUE);
				}
			}
		}
		ld->SetFaceSelection(fsel);
	}

	//put back the selection
	if (fnGetSyncSelectionMode()) 
		fnSyncGeomSelection();

	InvalidateView();

}



void	UnwrapMod::fnGeomExpandEdgeSel()
{


	theHold.Begin();
	HoldSelection();
	theHold.Accept(_T(GetString(IDS_DS_SELECT)));

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		BitArray gesel = ld->GetGeomEdgeSelection();
		//get the an empty vertex selection
		BitArray tempVSel;
		tempVSel.SetSize(ld->GetNumberGeomVerts());//TVMaps.v.Count());
		tempVSel.ClearAll();
		for (int i = 0; i < ld->GetNumberGeomEdges()/*TVMaps.gePtrList.Count()*/; i++)
		{
			if (gesel[i] && (!(ld->GetGeomEdgeHidden(i))))//TVMaps.gePtrList[i]->flags & FLAG_HIDDENEDGEA)))
			{
				int a = ld->GetGeomEdgeVert(i,0);//TVMaps.gePtrList[i]->a;
				tempVSel.Set(a,TRUE);
				a = ld->GetGeomEdgeVert(i,1);//TVMaps.gePtrList[i]->b;
				tempVSel.Set(a,TRUE);
			}
		}

		BitArray tempESel;
		tempESel.SetSize(ld->GetNumberGeomEdges());
		tempESel.ClearAll();
		for (int i = 0; i < ld->GetNumberGeomEdges(); i++)
		{
			if (!(ld->GetGeomEdgeHidden(i)))//TVMaps.gePtrList[i]->flags & FLAG_HIDDENEDGEA))
			{
				int a = ld->GetGeomEdgeVert(i,0);//TVMaps.gePtrList[i]->a;
				int b = ld->GetGeomEdgeVert(i,1);//TVMaps.gePtrList[i]->b;
				if (tempVSel[a] || tempVSel[b])
					tempESel.Set(i);
			}
		}
		gesel = tempESel;
		ld->SetGeomEdgeSelection(gesel);
	}

	fnSyncTVSelection();

	InvalidateView();
	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());

}
void	UnwrapMod::fnGeomContractEdgeSel()
{


	theHold.Begin();
	HoldSelection();
	theHold.Accept(_T(GetString(IDS_DS_SELECT)));
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		BitArray gesel = ld->GetGeomEdgeSelection();
		BitArray tempVSel;
		tempVSel.SetSize(ld->GetNumberGeomVerts());//TVMaps.v.Count());
		tempVSel.ClearAll();
		for (int i = 0; i < ld->GetNumberGeomEdges()/*TVMaps.gePtrList.Count()*/; i++)
		{
			if ((gesel[i]) && (!(ld->GetGeomEdgeHidden(i))))
			{
				int a = ld->GetGeomEdgeVert(i,0);//TVMaps.gePtrList[i]->a;
				tempVSel.Set(a,TRUE);
				a = ld->GetGeomEdgeVert(i,1);//TVMaps.gePtrList[i]->b;
				tempVSel.Set(a,TRUE);
			}
		}

		BitArray tempVBorderSel;
		tempVBorderSel.SetSize(ld->GetNumberGeomVerts());//TVMaps.v.Count());
		tempVBorderSel.ClearAll();
		for (int i = 0; i < ld->GetNumberGeomEdges(); i++)
		{
			if (!(ld->GetGeomEdgeHidden(i)))
			{
				int a = ld->GetGeomEdgeVert(i,0);//TVMaps.gePtrList[i]->a;
				int b = ld->GetGeomEdgeVert(i,1);//TVMaps.gePtrList[i]->b;
				if (tempVSel[a] && !tempVSel[b])
					tempVBorderSel.Set(a,TRUE);
				if (tempVSel[b] && !tempVSel[a])
					tempVBorderSel.Set(b,TRUE);			
			}
		}

		BitArray tempESel;
		tempESel.SetSize(ld->GetNumberGeomEdges());
		tempESel= gesel;
		for (int i = 0; i < ld->GetNumberGeomEdges(); i++)
		{
			int a = ld->GetGeomEdgeVert(i,0);//TVMaps.gePtrList[i]->a;
			int b = ld->GetGeomEdgeVert(i,1);//TVMaps.gePtrList[i]->b;
			if (tempVBorderSel[a] || tempVBorderSel[b])
				tempESel.Set(i,FALSE);
		}

		gesel = tempESel;
		ld->SetGeomEdgeSelection(gesel);
	}

	fnSyncTVSelection();

	InvalidateView();
	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());

}

void	UnwrapMod::fnGeomExpandVertexSel()
{


	theHold.Begin();
	HoldSelection();
	theHold.Accept(_T(GetString(IDS_DS_SELECT)));

	//get the an empty vertex selection
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		BitArray gvsel = ld->GetGeomVertSelection();
		BitArray tempVSel;
		tempVSel.SetSize(ld->GetNumberGeomVerts());//TVMaps.geomPoints.Count());
		tempVSel.ClearAll();
		for (int i = 0; i < ld->GetNumberGeomEdges(); i++)//TVMaps.gePtrList.Count(); i++)
		{

			int a = ld->GetGeomEdgeVert(i,0);//TVMaps.gePtrList[i]->a;
			int b = ld->GetGeomEdgeVert(i,1);//TVMaps.gePtrList[i]->b;

			if (gvsel[a] || gvsel[b])
			{
				tempVSel.Set(a,TRUE);
				tempVSel.Set(b,TRUE);
			}
		}
		gvsel = gvsel | tempVSel;
		ld->SetGeomVertSelection(gvsel);
	}

	fnSyncTVSelection();

	InvalidateView();
	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());


}
void	UnwrapMod::fnGeomContractVertexSel()
{

	theHold.Begin();
	HoldSelection();
	theHold.Accept(_T(GetString(IDS_DS_SELECT)));

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		BitArray gvsel = ld->GetGeomVertSelection();
		//get the an empty vertex selection
		BitArray tempVSel = gvsel;
		for (int i = 0; i < ld->GetNumberGeomEdges(); i++)//TVMaps.gePtrList.Count(); i++)
		{

			int a = ld->GetGeomEdgeVert(i,0);//TVMaps.gePtrList[i]->a;
			int b = ld->GetGeomEdgeVert(i,1);//TVMaps.gePtrList[i]->b;

			if (gvsel[a] && (!gvsel[b]))
			{
				tempVSel.Set(a,FALSE);				
			}
			else if (gvsel[b] && (!gvsel[a]))
			{
				tempVSel.Set(b,FALSE);				
			}
		}


		gvsel =  tempVSel;
		ld->SetGeomVertSelection(gvsel);
	}

	fnSyncTVSelection();

	InvalidateView();
	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());

}


void UnwrapMod::fnGeomLoopSelect()
{

	theHold.Begin();
	HoldSelection();
	theHold.Accept(_T(GetString(IDS_EDGELOOPSELECTION)));


	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		
		MeshTopoData *ld = mMeshTopoData[ldID];
		BitArray gesel = ld->GetGeomEdgeSelection();
		//get our selection
		//build a vertex connection list
		Tab<VEdges*> edgesAtVertex;
		edgesAtVertex.SetCount(ld->GetNumberGeomVerts());//TVMaps.geomPoints.Count());
		for (int i = 0; i < ld->GetNumberGeomVerts()/*TVMaps.geomPoints.Count()*/; i++)
			edgesAtVertex[i] = NULL;

		for (int i = 0; i < ld->GetNumberGeomEdges()/*TVMaps.gePtrList.Count()*/; i++)
		{

			{
				int a = ld->GetGeomEdgeVert(i,0);//TVMaps.gePtrList[i]->a;
				//			if (pointSel[a])
				{
					if (edgesAtVertex[a] == NULL)
						edgesAtVertex[a] = new VEdges();


					edgesAtVertex[a]->edgeIndex.Append(1,&i,5);
				}

				a = ld->GetGeomEdgeVert(i,1);//TVMaps.gePtrList[i]->b;
				//			if (pointSel[a])
				{
					if (edgesAtVertex[a] == NULL)
						edgesAtVertex[a] = new VEdges();

					edgesAtVertex[a]->edgeIndex.Append(1,&i,5);
				}

			}

		}



		//loop through our selection 
		BitArray tempGeSel;
		tempGeSel = gesel;
		tempGeSel.ClearAll();
		//get our start and end point
		for (int i = 0; i < ld->GetNumberGeomEdges()/*TVMaps.gePtrList.Count()*/; i++)
		{
			//find the mid edge repeat until hit self or no mid edge
			if (gesel[i])
			{
				for (int j = 0; j < 2; j++)
				{
					int starVert = ld->GetGeomEdgeVert(i,0);//TVMaps.gePtrList[i]->a;
					if (j==1) starVert = ld->GetGeomEdgeVert(i,1);//TVMaps.gePtrList[i]->b;
					int startEdge = i;
					int currentEdge = i;
					BOOL done = FALSE;
					while (!done)
					{
						//get the number of visible edges at this vert
						int ct = 0;
						BOOL openEdge = FALSE;
						BOOL degenFound = FALSE;
						for (int k = 0; k < edgesAtVertex[starVert]->edgeIndex.Count(); k++)
						{
							int eindex = edgesAtVertex[starVert]->edgeIndex[k];
							if (ld->GetGeomEdgeVert(eindex,0)/*TVMaps.gePtrList[eindex]->a*/ == ld->GetGeomEdgeVert(eindex,1)/*TVMaps.gePtrList[eindex]->b*/)
								degenFound = TRUE;
							if (ld->GetGeomEdgeNumberOfConnectedFaces(eindex)/*TVMaps.gePtrList[eindex]->faceList.Count()*/ == 1)
								openEdge = TRUE;
							if (!( ld->GetGeomEdgeHidden(eindex)))//TVMaps.gePtrList[eindex]->flags & FLAG_HIDDENEDGEA))
								ct++;
						}

						//if odd bail
						//if there is an open edge bail
						if (((ct%2) == 1) || openEdge || degenFound)
						{
							done = TRUE;
						}					
						else
						{
							int goalEdge = ct/2;
							int goalCount = 0;
							//now find our opposite edge 

							int currentFace = ld->GetGeomEdgeConnectedFace(currentEdge,0);//TVMaps.gePtrList[currentEdge]->faceList[0];
							while (goalCount != goalEdge)
							{
								//loop through our edges find the one that is connected to this face

								for (int k = 0; k < edgesAtVertex[starVert]->edgeIndex.Count(); k++)
								{
									int eindex = edgesAtVertex[starVert]->edgeIndex[k];
									if (eindex != currentEdge)
									{
										for (int m = 0; m < ld->GetGeomEdgeNumberOfConnectedFaces(eindex)/*TVMaps.gePtrList[eindex]->faceList.Count()*/; m++)
										{
											if (ld->GetGeomEdgeConnectedFace(eindex,m)/*TVMaps.gePtrList[eindex]->faceList[m]*/ == currentFace)
											{
												currentEdge = eindex;
												if (!( ld->GetGeomEdgeHidden(eindex)))///TVMaps.gePtrList[eindex]->flags & FLAG_HIDDENEDGEA))
												{
													goalCount++;
												}
												m = ld->GetGeomEdgeNumberOfConnectedFaces(eindex);//TVMaps.gePtrList[eindex]->faceList.Count();
												k = edgesAtVertex[starVert]->edgeIndex.Count();
											}
										}
									}
								}
								//find our next face
								for (int m = 0; m < ld->GetGeomEdgeNumberOfConnectedFaces(currentEdge)/*TVMaps.gePtrList[currentEdge]->faceList.Count()*/; m++)
								{
									if (ld->GetGeomEdgeConnectedFace(currentEdge,m)/*TVMaps.gePtrList[currentEdge]->faceList[m]*/!=currentFace)
									{
										currentFace = ld->GetGeomEdgeConnectedFace(currentEdge,m);//TVMaps.gePtrList[currentEdge]->faceList[m];
										m = ld->GetGeomEdgeNumberOfConnectedFaces(currentEdge);//TVMaps.gePtrList[currentEdge]->faceList.Count();
									}
								}
							}
							//set new edge
							//set the new vert
						}
						int a = ld->GetGeomEdgeVert(currentEdge,0);//TVMaps.gePtrList[currentEdge]->a;
						if (a == starVert)
							a = ld->GetGeomEdgeVert(currentEdge,1);//TVMaps.gePtrList[currentEdge]->b;
						starVert = a;

						tempGeSel.Set(currentEdge,TRUE);
						if (currentEdge == startEdge)
							done = TRUE;

					}
				}
			}
		}

		gesel = gesel | tempGeSel;

		for (int i = 0; i < edgesAtVertex.Count(); i++)
		{
			if (edgesAtVertex[i])
				delete edgesAtVertex[i];

		}
		ld->SetGeomEdgeSelection(gesel);
	}

	theHold.Suspend();
	fnSyncTVSelection();
	theHold.Resume();

	InvalidateView();
	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());

}


void UnwrapMod::fnGeomRingSelect()
{


	theHold.Begin();
	HoldSelection();
	theHold.Accept(_T(GetString(IDS_EDGERINGSELECTION)));

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		BitArray tempGeSel;
		MeshTopoData *ld = mMeshTopoData[ldID];
		BitArray gesel = ld->GetGeomEdgeSelection();
		tempGeSel = gesel;
		tempGeSel.ClearAll();


		//get the selected edge
		for (int i = 0; i < ld->GetNumberGeomEdges()/*TVMaps.gePtrList.Count()*/; i++)
		{
			if (gesel[i])
			{
				//get the face atatched to that edge
				for (int j = 0; j < ld->GetGeomEdgeNumberOfConnectedFaces(i)/*TVMaps.gePtrList[i]->faceList.Count()*/; j++)
				{
					//get all the visible edges attached to that face
					int currentFace = ld->GetGeomEdgeConnectedFace(i,j);//TVMaps.gePtrList[i]->faceList[j];

					int currentEdge = i;

					BOOL done = FALSE;
					int startEdge = currentEdge;
					BitArray edgesForThisFace;
					edgesForThisFace.SetSize(ld->GetNumberGeomEdges());//TVMaps.gePtrList.Count());


					Tab<int> facesToProcess;
					BitArray processedFaces;
					processedFaces.SetSize(ld->GetNumberFaces());//TVMaps.f.Count());
					processedFaces.ClearAll();


					while (!done)
					{


						edgesForThisFace.ClearAll();
						facesToProcess.Append(1,&currentFace,100);
						while (facesToProcess.Count() > 0)
						{
							//pop the stack
							currentFace = facesToProcess[0];
							facesToProcess.Delete(0,1);

							processedFaces.Set(currentFace,TRUE);
							int numberOfEdges = ld->GetFaceDegree(currentFace);//TVMaps.f[currentFace]->count;
							//loop through the edges
							for (int k = 0; k < numberOfEdges; k++)
							{
								//if edge is invisisble add the edges of the cross face
								int a = ld->GetFaceGeomVert(currentFace,k);//TVMaps.f[currentFace]->v[k];
								int b = ld->GetFaceGeomVert(currentFace,(k+1)%numberOfEdges);//TVMaps.f[currentFace]->v[(k+1)%numberOfEdges];
								if (a!=b)
								{
									int eindex = ld->FindGeoEdge(a,b);
									if (!( ld->GetGeomEdgeHidden(eindex)))//TVMaps.gePtrList[eindex]->flags & FLAG_HIDDENEDGEA))
									{
										edgesForThisFace.Set(eindex,TRUE);
									}
									else
									{
										for (int m = 0; m < ld->GetGeomEdgeNumberOfConnectedFaces(eindex);/*TVMaps.gePtrList[eindex]->faceList.Count();*/ m++)
										{
											int faceIndex = ld->GetGeomEdgeConnectedFace(eindex,m);//TVMaps.gePtrList[eindex]->faceList[m];
											if (!processedFaces[faceIndex])
											{
												facesToProcess.Append(1,&faceIndex,100);
											}
										}
									}
								}

							}
						}
						//if odd edge count we are done
						if ( ((edgesForThisFace.NumberSet()%2) == 1) || (edgesForThisFace.NumberSet() <= 2))
							done = TRUE;
						else
						{
							//get the mid edge 
							//start at the seed
							int goal = edgesForThisFace.NumberSet()/2;
							int currentGoal = 0;
							int vertIndex = ld->GetGeomEdgeVert(currentEdge,0);//TVMaps.gePtrList[currentEdge]->a;
							Tab<int> edgeList;
							for (int m = 0; m < edgesForThisFace.GetSize(); m++)
							{
								if (edgesForThisFace[m])
									edgeList.Append(1,&m,100);

							}
							while (currentGoal != goal)
							{
								//find next edge
								for (int i = 0; i < edgeList.Count(); i++)
								{
									int potentialEdge = edgeList[i];
									if (potentialEdge != currentEdge)
									{
										int a = ld->GetGeomEdgeVert(potentialEdge,0);//TVMaps.gePtrList[potentialEdge]->a;
										int b = ld->GetGeomEdgeVert(potentialEdge,1);//TVMaps.gePtrList[potentialEdge]->b;
										if (a == vertIndex)
										{
											vertIndex = b;
											currentEdge = potentialEdge;
											i = edgeList.Count();

											//increment current
											currentGoal++;
										}
										else if (b == vertIndex)
										{
											vertIndex = a;
											currentEdge = potentialEdge;
											i = edgeList.Count();

											//increment current
											currentGoal++;
										}
									}
								}


							}

						}

						if (tempGeSel[currentEdge])
							done = TRUE;
						tempGeSel.Set(currentEdge,TRUE);

						for (int m = 0; m < ld->GetGeomEdgeNumberOfConnectedFaces(currentEdge)/*TVMaps.gePtrList[currentEdge]->faceList.Count()*/; m++)
						{
							int faceIndex = ld->GetGeomEdgeConnectedFace(currentEdge,m);//TVMaps.gePtrList[currentEdge]->faceList[m];
							if ((faceIndex != currentFace) && (!processedFaces[faceIndex]))
							{
								currentFace = faceIndex;
								m = ld->GetGeomEdgeNumberOfConnectedFaces(currentEdge);//TVMaps.gePtrList[currentEdge]->faceList.Count();
							}
						}
						if (ld->GetGeomEdgeNumberOfConnectedFaces(currentEdge)/*TVMaps.gePtrList[currentEdge]->faceList.Count()*/ == 1)
							done = TRUE;
						//if we hit the start egde we are done
						if (currentEdge == startEdge) 
							done = TRUE;
					}


				}
			}
		}

		gesel = gesel | tempGeSel;
		ld->SetGeomEdgeSelection(gesel);
	}

	theHold.Suspend();
	fnSyncTVSelection();
	theHold.Resume();

	InvalidateView();
	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	if (ip) 
		ip->RedrawViews(ip->GetTime());

}


void	UnwrapMod::SelectGeomElement(MeshTopoData *ld, BOOL addSel, BitArray *originalSel)
{

	if (ip)
	{
		
		if (ip->GetSubObjectLevel() == 1)
		{
			BitArray gvsel = ld->GetGeomVertSelection();
			if (!addSel && (originalSel != NULL))
			{
				BitArray oSel = *originalSel;
				gvsel = (~gvsel) & oSel;
			}
			//loop through our edges finding ones with selected vertices
			//get the an empty vertex selection
			BitArray tempVSel;
			tempVSel.SetSize(ld->GetNumberGeomVerts());//TVMaps.geomPoints.Count());
			tempVSel.ClearAll();
			int selCount = -1;
			while (selCount != tempVSel.NumberSet())
			{
				selCount = tempVSel.NumberSet();
				for (int i = 0; i < ld->GetNumberGeomEdges();/*TVMaps.gePtrList.Count();*/ i++)
				{

					int a = ld->GetGeomEdgeVert(i,0);//TVMaps.gePtrList[i]->a;
					int b = ld->GetGeomEdgeVert(i,1);//TVMaps.gePtrList[i]->b;

					if (gvsel[a] || gvsel[b])
					{
						tempVSel.Set(a,TRUE);
						tempVSel.Set(b,TRUE);
					}
				}
				gvsel = gvsel | tempVSel;
			}

			if (!addSel && (originalSel != NULL))
			{
				BitArray oSel = *originalSel;
				gvsel =  oSel & (~gvsel);
			}

			ld->SetGeomVertSelection(gvsel);
		}

		else if (ip->GetSubObjectLevel() == 2)
		{
			BitArray gesel = ld->GetGeomEdgeSelection();
			if (!addSel && (originalSel != NULL))
			{
				BitArray oSel = *originalSel;
				gesel = (~gesel) & oSel;
			}

			//get the an empty vertex selection
			int selCount = -1;
			while (selCount != gesel.NumberSet())
			{
				selCount = gesel.NumberSet();
				BitArray tempVSel;
				tempVSel.SetSize(ld->GetNumberGeomVerts());//TVMaps.geomPoints.Count());
				tempVSel.ClearAll();
				for (int i = 0; i < ld->GetNumberGeomEdges();/*TVMaps.gePtrList.Count();*/ i++)
				{
					if (gesel[i])
					{
						int a = ld->GetGeomEdgeVert(i,0);//TVMaps.gePtrList[i]->a;
						tempVSel.Set(a,TRUE);
						a = ld->GetGeomEdgeVert(i,1);//TVMaps.gePtrList[i]->b;
						tempVSel.Set(a,TRUE);
					}
				}

				BitArray tempESel;
				tempESel.SetSize(ld->GetNumberGeomEdges());//TVMaps.gePtrList.Count());
				tempESel.ClearAll();
				for (int i = 0; i < ld->GetNumberGeomEdges();/*TVMaps.gePtrList.Count();*/ i++)
				{
					int a = ld->GetGeomEdgeVert(i,0);//TVMaps.gePtrList[i]->a;
					int b = ld->GetGeomEdgeVert(i,1);//TVMaps.gePtrList[i]->b;
					if (tempVSel[a] || tempVSel[b])
						tempESel.Set(i);
				}
				gesel = tempESel;
			}

			if (!addSel && (originalSel != NULL))
			{
				BitArray oSel = *originalSel;
				gesel =  oSel & (~gesel);
			}
			ld->SetGeomEdgeSelection(gesel);
		}
		else if (ip->GetSubObjectLevel() == 3)
		{
			BitArray holdSel = ld->GetFaceSelection();
			BitArray faceSel = ld->GetFaceSelection();
			if (!addSel && (originalSel != NULL))
			{
				BitArray oSel = *originalSel;
				faceSel = (~faceSel) & oSel;
			}

			Tab<BorderClass> dummy;
			Tab<Point3> normList;
			normList.ZeroCount();
			BuildNormals(ld, normList);
			BitArray tempSel;
			tempSel.SetSize(faceSel.GetSize());

			tempSel = faceSel;

			for (int i =0; i < faceSel.GetSize(); i++)
			{
				if ((tempSel[i]) && (i >= 0) && (i <normList.Count()))
				{
					BitArray sel;
					Point3 normal = normList[i];
					SelectFacesByGroup( ld,sel,i, normal, 180.0f, FALSE,normList,dummy);
					faceSel |= sel;
					for (int j = 0; j < tempSel.GetSize(); j++)
					{
						if (sel[j]) 
							tempSel.Set(j,FALSE);
					}
				}
			}

			if (!addSel && (originalSel != NULL))
			{
				BitArray oSel = *originalSel;
				faceSel =  oSel & (~faceSel);
			}
			ld->SetFaceSelection(faceSel);
		}
	}
}


BitArray* UnwrapMod::fnGetGeomVertexSelection(INode *node)
{
		MeshTopoData *ld = GetMeshTopoData(node);
		if (ld)
		{
			return ld->GetGeomVertSelectionPtr();
		}
	return NULL;
}

BitArray* UnwrapMod::fnGetGeomVertexSelection()
{
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			return ld->GetGeomVertSelectionPtr();
		}
	}
	return NULL;
}

void UnwrapMod::fnSetGeomVertexSelection(BitArray *sel, INode *node)
{

		MeshTopoData *ld = GetMeshTopoData(node);
		if (ld)
		{
			BitArray gvsel = ld->GetGeomVertSelection();
			gvsel.ClearAll();
			for (int i = 0 ; i < (*sel).GetSize(); i++)
			{
				if ((i < gvsel.GetSize()) && ((*sel)[i]))
					gvsel.Set(i,TRUE);
			}

			ld->SetGeomVertSelection(gvsel);

			if (fnGetSyncSelectionMode()) 
				fnSyncTVSelection();
			RebuildDistCache();

			NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);

			if (ip) ip->RedrawViews(ip->GetTime());
		}
}

void UnwrapMod::fnSetGeomVertexSelection(BitArray *sel)
{
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			BitArray gvsel = ld->GetGeomVertSelection();
			gvsel.ClearAll();
			for (int i = 0 ; i < (*sel).GetSize(); i++)
			{
				if ((i < gvsel.GetSize()) && ((*sel)[i]))
					gvsel.Set(i,TRUE);
			}

			ld->SetGeomVertSelection(gvsel);
			NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);

			if (ip) ip->RedrawViews(ip->GetTime());
		}
	}
}

BitArray* UnwrapMod::fnGetGeomEdgeSelection(INode *node)
{
	{
		MeshTopoData *ld = GetMeshTopoData(node);
		if (ld)
		{
			ld->GetGeomEdgeSelectionPtr();
		}
	}
	return NULL;
}


BitArray* UnwrapMod::fnGetGeomEdgeSelection()
{
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			ld->GetGeomEdgeSelectionPtr();
		}
	}
	return NULL;
}

void UnwrapMod::fnSetGeomEdgeSelection(BitArray *sel, INode *node)
{
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = GetMeshTopoData(node);
		if (ld)
		{
			BitArray gesel = ld->GetGeomEdgeSelection();
			gesel.ClearAll();
			for (int i = 0 ; i < (*sel).GetSize(); i++)
			{
				if ((i < gesel.GetSize()) && ((*sel)[i]))
					gesel.Set(i,TRUE);
			}		
			ld->SetGeomEdgeSelection(gesel);

			if (fnGetSyncSelectionMode()) 
				fnSyncTVSelection();
			RebuildDistCache();

			NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
			if (ip) ip->RedrawViews(ip->GetTime());
		}
	}
}

void UnwrapMod::fnSetGeomEdgeSelection(BitArray *sel)
{
	if (mMeshTopoData.Count())
	{
		MeshTopoData *ld = mMeshTopoData[0];
		if (ld)
		{
			BitArray gesel = ld->GetGeomEdgeSelection();
			gesel.ClearAll();
			for (int i = 0 ; i < (*sel).GetSize(); i++)
			{
				if ((i < gesel.GetSize()) && ((*sel)[i]))
					gesel.Set(i,TRUE);
			}
		
			ld->SetGeomEdgeSelection(gesel);
			NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
			if (ip) ip->RedrawViews(ip->GetTime());
		}
	}
}


void UnwrapMod::fnPeltSeamsToSel(BOOL replace)
{

	theHold.Begin();
	HoldSelection();
	if (replace)
		theHold.Accept(GetString(IDS_PW_SEAMTOSEL));	
	else theHold.Accept(GetString(IDS_PW_SEAMTOSEL2));	


	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		if (replace)
			ld->SetGeomEdgeSelection(ld->mSeamEdges);
		else
		{
			for (int i = 0; i < ld->mSeamEdges.GetSize(); i++)
			{
				if (ld->mSeamEdges[i])
					ld->SetGeomEdgeSelected(i,TRUE);//	gesel.Set(i,TRUE);


			}
		}
	}


	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());


}
void UnwrapMod::fnPeltSelToSeams(BOOL replace)
{

	theHold.Begin();

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		theHold.Put (new UnwrapPeltSeamRestore (this,ld));
	}

	if (replace)
		theHold.Accept(GetString(IDS_PW_SELTOSEAM));
	else theHold.Accept(GetString(IDS_PW_SELTOSEAM2));

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];

		BitArray gesel = ld->GetGeomEdgeSelection();
		if (replace)
		{


			ld->mSeamEdges = gesel;
		}
		else
		{
			if (ld->mSeamEdges.GetSize() != gesel.GetSize())
			{
				ld->mSeamEdges.SetSize(gesel.GetSize());
				ld->mSeamEdges.ClearAll();
			}

			for (int i = 0; i < gesel.GetSize(); i++)
			{
				if (gesel[i])
					ld->mSeamEdges.Set(i,TRUE);

			}
		}
	}

	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());

}

class CellInfo
{
public:
	Tab<int> facesInThisCell;
};

class Line
{
	Tab<CellInfo*> cells;
public:
	int gridSize;
	float min, max;
	float len;

	void InitializeGrid(int cellSize, float min, float max);
	void AddLine(int index, float a, float b);
	void HitLine(float a, float b);
	void FreeLine();
	int ct;
	BitArray bHitFaces;
	Tab<int> hitFaces;

};

void Line::InitializeGrid(int size, float min, float max)
{
	this->min = min;
	this->max = max;
	gridSize = size;
	len = max - min;
	cells.SetCount(size);
	for (int i = 0; i < size; i++)
		cells[i] = NULL;
	ct = 0;

}
void Line::AddLine(int index, float a, float b)
{
	a -= min;
	b -= min;
	if (a > b)
	{
		float temp = a;
		a = b;
		b = temp;
	}
	//get the start cell
	int startIndex = (a/len) * gridSize;
	//get the end cell
	int endIndex = (b/len) * gridSize;

	if (startIndex >= gridSize) startIndex = gridSize-1;
	if (endIndex >= gridSize) endIndex = gridSize-1;

	//if null create it
	for (int i = startIndex; i <= endIndex; i++)
	{
		if (cells[i] == NULL)
			cells[i] = new CellInfo();
		//add the primitive
		cells[i]->facesInThisCell.Append(1,&index,10);	
	}
	if (index >= ct)
		ct = index+1;
}
void Line::HitLine(float a, float b)
{
	a -= min;
	b -= min;

	if (bHitFaces.GetSize() != ct)
		bHitFaces.SetSize(ct);
	bHitFaces.ClearAll();
	hitFaces.SetCount(0);

	if (a > b)
	{
		float temp = a;
		a = b;
		b = temp;
	}
	//get the start cell
	int startIndex = (a/len) * gridSize;
	//get the end cell
	int endIndex = (b/len) * gridSize;

	if (startIndex >= gridSize) startIndex = gridSize-1;
	if (endIndex >= gridSize) endIndex = gridSize-1;
	for (int i = startIndex; i <= endIndex; i++)
	{
		if (cells[i] != NULL)
		{
			int numberOfFaces = cells[i]->facesInThisCell.Count();
			for (int j = 0; j < numberOfFaces; j++)
			{
				int index = cells[i]->facesInThisCell[j];
				if (!bHitFaces[index])
				{
					hitFaces.Append(1,&index, 500);
					bHitFaces.Set(index,TRUE);
				}
			}
		}
	}

}
void Line::FreeLine()
{

	for (int i = 0; i < cells.Count(); i++)
	{
		if (cells[i])
			delete cells[i];
		cells[i] = NULL;
	}
}

void UnwrapMod::fnSelectOverlap()
{
	theHold.Begin();
	HoldSelection();
	theHold.Accept(_T(GetString(IDS_SELECT_OVERLAP)));

	if (fnGetSyncSelectionMode()) 
		fnSyncGeomSelection();

	SelectOverlap();

	InvalidateView();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());

}



void OverlapMap::Hit(int mapIndex, MeshTopoData *ld, int faceIndex)
{
	BOOL hit = TRUE;

	if (mBuffer[mapIndex].mLD == NULL)
		hit = FALSE;

	//if nothing in this cell just add it
	if (!hit)
	{
		mBuffer[mapIndex].mLD = ld;
		mBuffer[mapIndex].mFaceID = faceIndex;
	}
	else
	{
		//have somethign in the cell need to check
		//get the ld and face id in the cell
		MeshTopoData *baseLD = mBuffer[mapIndex].mLD;
		int baseFaceIndex = mBuffer[mapIndex].mFaceID;
		//hit on the same mesh id cannot be the same
		if ( (baseLD == ld) &&
			(baseFaceIndex != faceIndex))
		{
			ld->GetFaceSelectionPtr()->Set(faceIndex);
			baseLD->GetFaceSelectionPtr()->Set(baseFaceIndex);
		}
		//hit on different mesh dont care about the face ids
		else if ( (baseLD != ld) )
		{
			ld->GetFaceSelectionPtr()->Set(faceIndex);
			baseLD->GetFaceSelectionPtr()->Set(baseFaceIndex);
		}

	}
}

void OverlapMap::Map(MeshTopoData *ld, int faceIndex, Point3 pa, Point3 pb, Point3 pc)
{
	pa *= mBufferSize;
	pb *= mBufferSize;
	pc *= mBufferSize;

	long x1 = (long) pa.x;
	long y1 = (long) pa.y;
	long x2 = (long) pb.x;
	long y2 = (long) pb.y;
	long x3 = (long) pc.x;
	long y3 = (long) pc.y;

	//sort top to bottom
	int sx[3], sy[3];
	sx[0] = x1;
	sy[0] = y1;

	if (y2 < sy[0])
	{
		sx[0] = x2;
		sy[0] = y2;

		sx[1] = x1;
		sy[1] = y1;
	}
	else 
	{
		sx[1] = x2;
		sy[1] = y2;
	}

	if (y3 < sy[0])
	{
		sx[2] = sx[1];
		sy[2] = sy[1];

		sx[1] = sx[0];
		sy[1] = sy[0];


		sx[0] = x3;
		sy[0] = y3;
	}
	else if (y3 < sy[1])
	{
		sx[2] = sx[1];
		sy[2] = sy[1];

		sx[1] = x3;
		sy[1] = y3;
	}
	else
	{
		sx[2] = x3;
		sy[2] = y3;
	}

	int hit = 0;

	int width = mBufferSize;
	int height = mBufferSize;
	//if flat top
	if (sy[0] == sy[1])
	{

		float xInc0 = 0.0f;
		float xInc1 = 0.0f;
		float yDist = DL_abs(sy[0] - sy[2]);
		float xDist0 = sx[2] - sx[0];
		float xDist1 = sx[2] - sx[1];
		xInc0 = xDist0/yDist;		
		xInc1 = xDist1/yDist;

		float cx0 = sx[0];
		float cx1 = sx[1];
		for (int i = sy[0]; i < sy[2]; i++)
		{

			int ix0 = cx0;
			int ix1 = cx1;

			if (cx0 - floor(cx0) >= 0.5f) ix0 += 1;
			if (cx1 - floor(cx1) >= 0.5f) ix1 += 1;


			//line 
			if (ix0 > ix1)
			{
				int temp = ix0;
				ix0 = ix1;
				ix1 = temp;
			}
			for (int j = ix0; j <= ix1; j++)
			{
				int index = j + i * width;
				if ((j >= 0) && (j <width) && (i >= 0) && (i < height))
				{

					if ((j > (ix0+1)) && (j < (ix1-1)))
					{
						Hit(index, ld, faceIndex)	;					
					}	
					else
					{
						mBuffer[index].mLD = ld;
						mBuffer[index].mFaceID = faceIndex;
					}
				}

			}
			cx0 += xInc0;
			cx1 += xInc1;
		}

	}
	//it flat bottom
	else if (sy[1] == sy[2])
	{

		float xInc0 = 0.0f;
		float xInc1 = 0.0f;
		float yDist = DL_abs(sy[0] - sy[2]);
		float xDist0 = sx[1]- sx[0];
		float xDist1 = sx[2] - sx[0];
		xInc0 = xDist0/yDist;		
		xInc1 = xDist1/yDist;

		float cx0 = sx[0];
		float cx1 = sx[0];
		for (int i = sy[0]; i < sy[2]; i++)
		{

			int ix0 = cx0;
			int ix1 = cx1;

			if (cx0 - floor(cx0) >= 0.5f) ix0 += 1;
			if (cx1 - floor(cx1) >= 0.5f) ix1 += 1;

			//line 
			if (ix0 > ix1)
			{
				int temp = ix0;
				ix0 = ix1;
				ix1 = temp;
			}
			for (int j = ix0; j <= ix1; j++)
			{

				int index = j + i * width;
				if ((j >= 0) && (j <width) && (i >= 0) && (i < height))
				{
					if ((j > (ix0+1)) && (j < (ix1-1)))
					{
						Hit(index, ld, faceIndex);						
					}
					else
					{
						mBuffer[index].mLD = ld;
						mBuffer[index].mFaceID = faceIndex;
					}
					//						processedPixels.Set(index,TRUE);
				}


			}
			cx0 += xInc0;
			cx1 += xInc1;
		}		

	}
	else
	{


		float xInc1 = 0.0f;
		float xInc2 = 0.0f;
		float xInc3 = 0.0f;
		float yDist0to1 = DL_abs(sy[1] - sy[0]);
		float yDist0to2 = DL_abs(sy[2] - sy[0]);
		float yDist1to2 = DL_abs(sy[2] - sy[1]);

		float xDist1 = sx[1]- sx[0];
		float xDist2 = sx[2] - sx[0];
		float xDist3 = sx[2] - sx[1];
		xInc1 = xDist1/yDist0to1;		
		xInc2 = xDist2/yDist0to2;
		xInc3 = xDist3/yDist1to2;

		float cx0 = sx[0];
		float cx1 = sx[0];
		//go from s[0] to s[1]
		for (int i = sy[0]; i < sy[1]; i++)
		{

			int ix0 = cx0;
			int ix1 = cx1;

			if (cx0 - floor(cx0) >= 0.5f) ix0 += 1;
			if (cx1 - floor(cx1) >= 0.5f) ix1 += 1;

			//line 
			if (ix0 > ix1)
			{
				int temp = ix0;
				ix0 = ix1;
				ix1 = temp;
			}
			for (int j = ix0; j <= ix1; j++)
			{

				int index = j + i * width;
				if ((j >= 0) && (j <width) && (i >= 0) && (i < height))
				{
					if ((j > (ix0+1)) && (j < (ix1-1)))
					{
						Hit(index, ld, faceIndex);						
					}
					else
					{
						mBuffer[index].mLD = ld;
						mBuffer[index].mFaceID = faceIndex;
					}
				}

			}
			cx0 += xInc1;
			cx1 += xInc2;
		}	
		//go from s[1] to s[2]
		for (int i = sy[1]; i < sy[2]; i++)
		{

			int ix0 = cx0;
			int ix1 = cx1;

			if (cx0 - floor(cx0) >= 0.5f) ix0 += 1;
			if (cx1 - floor(cx1) >= 0.5f) ix1 += 1;

			//line 
			if (ix0 > ix1)
			{
				int temp = ix0;
				ix0 = ix1;
				ix1 = temp;
			}
			for (int j = ix0; j <= ix1; j++)
			{

				int index = j + i * width;
				if ((j >= 0) && (j <width) && (i >= 0) && (i < height))
				{
					if ( (j > (ix0+1)) && (j < (ix1-1)))
					{
						Hit(index, ld, faceIndex);						
					}
					else
					{
						mBuffer[index].mLD = ld;
						mBuffer[index].mFaceID = faceIndex;
					}
				}

			}
			cx0 += xInc3;
			cx1 += xInc2;
		}	

	}
}

void UnwrapMod::SelectOverlap()
{
	//see if we have a selection
	BOOL hasFaceSelection = FALSE;
	//get our bounding box
	Box3 bounds;
	bounds.Init();
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		int numVerts = ld->GetNumberTVVerts();
		for (int i = 0; i < numVerts; i++)
		{
			bounds += ld->GetTVVert(i);
		}
	}	
	//put a small fudge to keep faces off the edge
	bounds.EnlargeBy(0.05f);
	//build our transform

	float xScale = bounds.pmax.x - bounds.pmin.x;
	float yScale = bounds.pmax.y - bounds.pmin.y;
	Point3 offset = bounds.pmin;

	//create our buffer
	OverlapMap overlapMap;
	overlapMap.Init();


	//see if we have any existing selections
	BOOL hasSelection = FALSE;
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		if (ld->GetFaceSelectionPtr()->NumberSet())
		{
			hasSelection = TRUE;
			ldID = mMeshTopoData.Count();
		}
	}

	//add all the faces or just selected faces if there are any
	Tab<OverlapCell> facesToCheck;
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		int numFaces = ld->GetNumberFaces();
		for (int i = 0; i < numFaces; i++)
		{
			if (hasSelection)
			{
				if ((*ld->GetFaceSelectionPtr())[i])
				{
					OverlapCell t;
					t.mFaceID = i;
					t.mLD = ld;
					facesToCheck.Append(1,&t,10000);
				}
			}
			else
			{
				OverlapCell t;
				t.mFaceID = i;
				t.mLD = ld;
				facesToCheck.Append(1,&t,10000);
			}
		}
	}

	//clear all the selections
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		ld->GetFaceSelectionPtr()->ClearAll();
	}

	//loop through the faces to check
	for (int ldID = 0; ldID < facesToCheck.Count(); ldID++)
	{
		MeshTopoData *ld = facesToCheck[ldID].mLD;
		//loop through the faces
		int i = facesToCheck[ldID].mFaceID;

		int deg = ld->GetFaceDegree(i);
		int index[4];
		index[0] = ld->GetFaceTVVert(i,0);
		Point3 pa = ld->GetTVVert(index[0]);
		pa.x -= offset.x;
		pa.x /= xScale;

		pa.y -= offset.y;
		pa.y /= yScale;

		for (int j = 0; j < deg -2; j++)
		{
			index[1] = ld->GetFaceTVVert(i,j+1);
			index[2] = ld->GetFaceTVVert(i,j+2);
			Point3 pb = ld->GetTVVert(index[1]);
			Point3 pc = ld->GetTVVert(index[2]);

			pb.x -= offset.x;
			pb.x /= xScale;
			pb.y -= offset.y;
			pb.y /= yScale;

			pc.x -= offset.x;
			pc.x /= xScale;
			pc.y -= offset.y;
			pc.y /= yScale;

			//add face to buffer
			//select anything that overlaps
			overlapMap.Map(ld,i,pa,pb,pc);
		}
	}
}

BOOL UnwrapMod::BXPLine(long x1,long y1,long x2,long y2,
						int width, int height,	int id,				   
						Tab<int> &map, BOOL clearEnds)



{
	long i,px,py,x,y;
	long dx,dy,dxabs,dyabs,sdx,sdy;
	long count;


	if (clearEnds)
	{
		map[x1+y1*(width)] = -1;
		map[x2+y2*(width)] = -1;
	}


	dx = x2-x1;
	dy = y2-y1;

	if (dx >0)
		sdx = 1;
	else
		sdx = -1;

	if (dy >0)
		sdy = 1;
	else
		sdy = -1;


	dxabs = abs(dx);
	dyabs = abs(dy);

	x=0;
	y=0;
	px=x1;
	py=y1;

	count = 0;
	BOOL iret = FALSE;


	if (dxabs >= dyabs)
		for (i=0; i<=dxabs; i++)
		{
			y +=dyabs;
			if (y>=dxabs)
			{
				y-=dxabs;
				py+=sdy;
			}


			if ( (px>=0) && (px<width) &&
				(py>=0) && (py<height)    )
			{
				int tid = map[px+py*width];
				if (tid != -1)
				{
					if ((tid != 0) && (tid != id))
						iret = TRUE;
					map[px+py*width] = id;
				}	
			}


			px+=sdx;
		}


	else

		for (i=0; i<=dyabs; i++)
		{
			x+=dxabs;
			if (x>=dyabs)
			{
				x-=dyabs;
				px+=sdx;
			}


			if ( (px>=0) && (px<width) &&
				(py>=0) && (py<height)    )
			{
				int tid = map[px+py*width];
				if (tid != -1)
				{

					if ((tid != 0) &&(tid != id))
						iret = TRUE;
					map[px+py*width] = id;
				}
			}



			py+=sdy;
		}
		(count)--;
		return iret;
}

//VSNAP
void UnwrapMod::BuildSnapBuffer()
{
	TransferSelectionStart();
	BOOL vSnap,eSnap;
	pblock->GetValue(unwrap_vertexsnap,0,vSnap,FOREVER);
	pblock->GetValue(unwrap_edgesnap,0,eSnap,FOREVER);

	if ( !vSnap && !eSnap )
	{
		FreeSnapBuffer();
	}
	else
	{
		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			MeshTopoData *ld = mMeshTopoData[ldID];
			//get our window width height
			float xzoom, yzoom;
			int width,height;
			ComputeZooms(hView,xzoom,yzoom,width,height);

			try
			{
				ld->BuildSnapBuffer(width,height);
			}
			catch ( std::bad_alloc& )
			{
				MessageBox( NULL, "Out of memory - vertex and edge snaps have been disabled.\nTry using a smaller Edit UVWs window.", "Out of memory", MB_OK );
				FreeSnapBuffer();
				pblock->SetValue(unwrap_vertexsnap,0,FALSE);
				pblock->SetValue(unwrap_edgesnap,0,FALSE);
				break;
			}

			Tab<IPoint2> transformedPoints;
			transformedPoints.SetCount(ld->GetNumberTVVerts());//TVMaps.v.Count());

			//build the vertex bufffer list
			for (int i=0; i < ld->GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++) 
			{
				Point3 tvPoint = UVWToScreen(ld->GetTVVert(i),xzoom,yzoom,width,height);
				IPoint2 tvPoint2;
				tvPoint2.x  = (int) tvPoint.x;
				tvPoint2.y  = (int) tvPoint.y;
				transformedPoints[i] = tvPoint2;
			}

			//loop through our verts 
			if (vSnap)
			{
				for (int i = 0; i < ld->GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++) 
				{
					//if in window add it
					if ( (transformedPoints[i].x >= 0) && (transformedPoints[i].x < width) &&
						(transformedPoints[i].y >= 0) && (transformedPoints[i].y < height) )
					{
						int x = transformedPoints[i].x;
						int y = transformedPoints[i].y;

						int index = y * width + x;
						if ( (!ld->GetTVVertSelected(i)) && ld->IsTVVertVisible(i))
							ld->SetVertexSnapBuffer(index,i);
	//						vertexSnapBuffer[index] = i;
					}
				}
			}



			//loop through the edges


			if (eSnap)
			{
				for (int i =0; i < ld->GetNumberTVEdges(); i++)//TVMaps.ePtrList.Count(); i++)
				{
					//add them to the edge buffer
					if (!(ld->GetTVEdgeHidden(i)))//TVMaps.ePtrList[i]->flags & FLAG_HIDDENEDGEA))
					{
						int a = ld->GetTVEdgeVert(i,0);//TVMaps.ePtrList[i]->a;
						int b = ld->GetTVEdgeVert(i,1);//TVMaps.ePtrList[i]->b;
						BOOL aHidden = (!ld->IsTVVertVisible(a));
						BOOL bHidden = (!ld->IsTVVertVisible(b));
						if ((a == mMouseHitVert) || (b == mMouseHitVert) || ld->GetTVVertSelected(a) || ld->GetTVVertSelected(b) || aHidden || bHidden)
						{
							ld->SetEdgesConnectedToSnapvert(i,TRUE);
						}
						IPoint2 pa,pb;
						pa = transformedPoints[a];
						pb = transformedPoints[b];

						long x1,y1,x2,y2;

						x1 = (long)pa.x;
						y1 = (long)pa.y;

						x2 = (long)pb.x;
						y2 = (long)pb.y;

						if (!ld->GetEdgesConnectedToSnapvert(i))
							BXPLine(x1,y1,x2,y2,
							width, height,i,				   
							ld->GetEdgeSnapBuffer(), FALSE);
					}

				}
			}

		}
	}

	TransferSelectionEnd(FALSE,FALSE);

}

void UnwrapMod::FreeSnapBuffer()
{
	for ( int ldID = 0; ldID < mMeshTopoData.Count(); ++ldID )
	{
		MeshTopoData* ld = mMeshTopoData[ldID];
		ld->FreeSnapBuffer();
	}
}

BOOL UnwrapMod::GetGridSnap()
{
	BOOL snap; 
	pblock->GetValue(unwrap_gridsnap,0,snap,FOREVER);
	return snap;
}

void UnwrapMod::SetGridSnap(BOOL snap)
{
	pblock->SetValue(unwrap_gridsnap,0,snap);
}

BOOL UnwrapMod::GetVertexSnap()
{
	BOOL snap; 
	pblock->GetValue(unwrap_vertexsnap,0,snap,FOREVER);
	return snap;

}
void UnwrapMod::SetVertexSnap(BOOL snap)
{
	pblock->SetValue(unwrap_vertexsnap,0,snap);
}
BOOL UnwrapMod::GetEdgeSnap()
{
	BOOL snap; 
	pblock->GetValue(unwrap_edgesnap,0,snap,FOREVER);
	return snap;

}
void UnwrapMod::SetEdgeSnap(BOOL snap)
{
	pblock->SetValue(unwrap_edgesnap,0,snap);
}


bool UnwrapMod::AnyThingSelected()
{

	int selCount = 0;
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		if (fnGetTVSubMode() == TVVERTMODE)
		{
			selCount += ld->GetTVVertSelection().NumberSet();
		}
		else if (fnGetTVSubMode() == TVEDGEMODE)
		{
			selCount += ld->GetTVEdgeSelection().NumberSet();
		}
		else if (fnGetTVSubMode() == TVFACEMODE)
		{
			selCount += ld->GetFaceSelection().NumberSet();
		}
		
	}

	if (selCount != 0)
	{
		return true;
	}
	return false;
}