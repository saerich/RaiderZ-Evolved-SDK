/********************************************************************** *<
FILE: MeshTopoData_RelaxMethods.cpp

DESCRIPTION: local mode data for the unwrap dealing with relax

HISTORY: 9/25/2006
CREATED BY: Peter Watje




*>	Copyright (c) 2006, All Rights Reserved.
**********************************************************************/

#include "unwrap.h"


//RELAX

#define EPSILON float(1e-04)

typedef Tab<DWORD> DWordTab;

class RelaxModData {
public:
   DWordTab *nbor;   // Array of neighbors for each vert.
   BitArray *vis; // visibility of edges on path to neighbors.
   int *fnum;     // Number of faces for each vert.
   BitArray sel;     // Selection information.
   int vnum;      // Size of above arrays

   RelaxModData ();
   ~RelaxModData () { Clear(); }
   void Clear();
   void SetVNum (int num);
   void MaybeAppendNeighbor(int vert, int index, int &max) {
      for (int k1=0; k1<max; k1++) {
         if (nbor[vert][k1] == index)
            return;
         }
      DWORD dwi = (DWORD)index;
      nbor[vert].Append (1, &dwi, 1);
      max++;
      }
};

RelaxModData::RelaxModData () {
   nbor = NULL;
   vis = NULL;
   fnum = NULL;
   vnum = 0;
}

void RelaxModData::Clear () {
   if (nbor) delete [] nbor;
   nbor = NULL;
   if (vis) delete [] vis;
   vis = NULL;
   if (fnum) delete [] fnum;
   fnum = NULL;
}

void RelaxModData::SetVNum (int num) {
   if (num==vnum) return;
   Clear();
   vnum = num;
   if (num<1) return;
   nbor = new DWordTab[vnum];
   vis = new BitArray[vnum];
   fnum = new int[vnum];
   sel.SetSize (vnum);
}


static void FindVertexAngles(PatchMesh &pm, float *vang) {
   for (int i=0; i<pm.numVerts + pm.numVecs; i++) vang[i] = 0.0f;
   for (int i=0; i<pm.numPatches; i++) {
      Patch &p = pm.patches[i];
      for (int j=0; j<p.type; j++) {
         Point3 d1 = pm.vecs[p.vec[j*2]].p - pm.verts[p.v[j]].p;
         Point3 d2 = pm.vecs[p.vec[((j+p.type-1)%p.type)*2+1]].p - pm.verts[p.v[j]].p;
         float len = LengthSquared(d1);
         if (len == 0) continue;
         d1 /= Sqrt(len);
         len = LengthSquared (d2);
         if (len==0) continue;
         d2 /= Sqrt(len);
         float cs = DotProd (d1, d2);
         if (cs>=1) continue; // angle of 0
         if (cs<=-1) vang[p.v[j]] += PI;
         else vang[p.v[j]] += (float) acos (cs);
      }
   }
}

static void FindVertexAngles (MNMesh &mm, float *vang) {
   for (int i=0; i<mm.numv; i++) vang[i] = 0.0f;
   for (int i=0; i<mm.numf; i++) {
      int *vv = mm.f[i].vtx;
      int deg = mm.f[i].deg;
      for (int j=0; j<deg; j++) {
         Point3 d1 = mm.v[vv[(j+1)%deg]].p - mm.v[vv[j]].p;
         Point3 d2 = mm.v[vv[(j+deg-1)%deg]].p - mm.v[vv[j]].p;
         float len = LengthSquared(d1);
         if (len == 0) continue;
         d1 /= Sqrt(len);
         len = LengthSquared (d2);
         if (len==0) continue;
         d2 /= Sqrt(len);
         float cs = DotProd (d1, d2);
         // STEVE: What about angles over PI?
         if (cs>=1) continue; // angle of 0
         if (cs<=-1) vang[vv[j]] += PI;
         else vang[vv[j]] += (float) acos (cs);
      }
   }
}


float MeshTopoData::AngleFromVectors(Point3 vec1, Point3 vec2)
{
	float iangle = 0.0f;
	float dot = DotProd(vec1,vec2);
	Point3 crossProd = CrossProd(vec2,vec1);
	if (dot >= 0.9999f)
		iangle = 0.0f;
	else if (dot == 0.0f)
		iangle = PI*0.5f;
	else if (dot <= -0.9999f)
		iangle = PI;
	else iangle = acos(dot);
	if (crossProd.z < 0.0)
		iangle *= -1.0f;
	return iangle;
}


Matrix3 MeshTopoData::GetTMFromFace(int faceIndex, BOOL useTVFace)
{
   Matrix3 tm(1);
   int edgeCount = TVMaps.f[faceIndex]->count;
   Point3 centroid = Point3(0.0f,0.0f,0.0f);
   for (int j = 0; j < edgeCount; j++)
      {        
      int index = 0;
      if (useTVFace)
         {
         index = TVMaps.f[faceIndex]->t[j];
         centroid += TVMaps.v[index].GetP();
         }
      else
         {
         index = TVMaps.f[faceIndex]->v[j];
         centroid += TVMaps.geomPoints[index];
         }

      }
   centroid = centroid/edgeCount;
   
   //get normal use it as a z axis
   Point3 vecA = Point3(0.0f,0.0f,0.0f), vecB = Point3(0.0f,0.0f,0.0f);

   int index = 0;
   Point3 basePoint = Point3(0.0f,0.0f,0.0f);

   if (useTVFace)
      {
      index = TVMaps.f[faceIndex]->t[0];
      basePoint = TVMaps.v[index].GetP();
      index = TVMaps.f[faceIndex]->t[1];
      vecA = Normalize(TVMaps.v[index].GetP() - basePoint);
      }
   else
      {
      index = TVMaps.f[faceIndex]->v[0];
      basePoint = TVMaps.geomPoints[index];
      index = TVMaps.f[faceIndex]->v[1];
      vecA = Normalize(TVMaps.geomPoints[index] - basePoint);
      }

   for (int j = 2; j < edgeCount; j++)
      {
      if (useTVFace)
         {
         index = TVMaps.f[faceIndex]->t[j];
         vecB = Normalize(TVMaps.v[index].GetP() - basePoint);
         }
      else
         {
         index = TVMaps.f[faceIndex]->v[j];
         vecB = Normalize(TVMaps.geomPoints[index] - basePoint);
         }

      if (Length(vecA-vecB) > 0.0001f)
         {
         j = edgeCount;
         }
      }
   Point3 normal = Normalize(CrossProd(vecA,vecB));

   //get an axis from centroid to first vertex use an x axis
   //get a perp and use it as a y axis
   Point3 xVec, yVec, zVec;
   zVec = normal;
   xVec = Normalize(basePoint - centroid);
   yVec = Normalize(CrossProd(zVec, xVec));
   //build matrix from it 
   tm.SetRow(0,xVec);
   tm.SetRow(1,yVec);
   tm.SetRow(2,zVec);
   tm.SetRow(3,centroid);

   return tm;

}

void  MeshTopoData::RelaxVerts2(int subobjectMode, float relax, int iter, BOOL boundary,BOOL saddle, UnwrapMod *mod, BOOL applyToAll, BOOL updateUI)

{
	TimeValue t = GetCOREInterface()->GetTime();

   BitArray mVSelTemp = mVSel;
   if (subobjectMode == 1)
      GetVertSelFromEdge(mVSelTemp);
   else if (subobjectMode == 2)
      GetVertSelFromFace(mVSelTemp);

   if (applyToAll)
   {
	   mVSelTemp.ClearAll();
	   for (int i = 0; i < GetNumberTVVerts(); i++)
	   {
		   if (IsTVVertVisible(i))
			   mVSelTemp.Set(i,TRUE);
	   }
	   
   }

   if (mVSelTemp.NumberSet() == 0) return;

   RelaxModData *rd = new RelaxModData;


   float wtdRelax = 0.0f; // mjm - 4.8.99


   Tab<float> softSel;
   softSel.SetCount(TVMaps.v.Count());



//get the local data



   Mesh *rmesh = NULL;
   PatchMesh *rpatch = NULL;
   MNMesh *rmnMesh = NULL;
   

   if (mesh)
      {
      rmesh = new Mesh();
      rmesh->setNumVerts(TVMaps.v.Count());

      rmesh->vertSel = mVSelTemp;

      rmesh->SupportVSelectionWeights();
      float *vsw = rmesh->getVSelectionWeights ();

      for (int i = 0; i<TVMaps.v.Count(); i++)
         {  
         Point3 p = TVMaps.v[i].GetP();
         p.z = 0.0f;
         rmesh->setVert(i,p);
         vsw[i] = TVMaps.v[i].GetInfluence();
         }

      rmesh->setNumFaces(TVMaps.f.Count());
      for (int i = 0; i < TVMaps.f.Count();i++)
         {
         int ct = 3;
         for (int j = 0; j < ct; j++)
            {
            rmesh->faces[i].v[j] = TVMaps.f[i]->t[j];
            }
         if (TVMaps.f[i]->flags&FLAG_HIDDENEDGEA)
            rmesh->faces[i].setEdgeVis(0, EDGE_INVIS);
         else rmesh->faces[i].setEdgeVis(0, EDGE_VIS);

         if (TVMaps.f[i]->flags&FLAG_HIDDENEDGEB)
            rmesh->faces[i].setEdgeVis(1, EDGE_INVIS);
         else rmesh->faces[i].setEdgeVis(1, EDGE_VIS);

         if (TVMaps.f[i]->flags&FLAG_HIDDENEDGEC)
            rmesh->faces[i].setEdgeVis(2, EDGE_INVIS);
         else rmesh->faces[i].setEdgeVis(2, EDGE_VIS);

         }

      if (mVSelTemp.NumberSet() == 0)
         rmesh->selLevel = MESH_OBJECT;
      else rmesh->selLevel = MESH_VERTEX;

      }
   else if (mnMesh)
      {
      rmnMesh = new MNMesh();

      rmnMesh->setNumVerts(TVMaps.v.Count());
      rmnMesh->setNumFaces(TVMaps.f.Count());


      rmnMesh->InvalidateTopoCache();
      rmnMesh->VertexSelect (mVSelTemp);

      rmnMesh->SupportVSelectionWeights();

      float *vsw = rmnMesh->getVSelectionWeights ();

      for (int i = 0; i<TVMaps.v.Count(); i++)
         {  
         Point3 p = TVMaps.v[i].GetP();
         p.z = 0.0f;
         rmnMesh->P(i) = p;
         if (mVSelTemp[i])
            rmnMesh->v[i].SetFlag (MN_SEL);
         else rmnMesh->v[i].ClearFlag (MN_SEL);

         vsw[i] = TVMaps.v[i].GetInfluence();
         }

      for (int i = 0; i < TVMaps.f.Count();i++)
         {

         int ct = TVMaps.f[i]->count;
         rmnMesh->f[i].Init();
         rmnMesh->f[i].SetDeg(ct);

         for (int j = 0; j < ct; j++)
            {
            rmnMesh->f[i].vtx[j] = TVMaps.f[i]->t[j];
            }
         }

      if (mVSelTemp.NumberSet() == 0)
         rmnMesh->selLevel = MNM_SL_OBJECT;
      else rmnMesh->selLevel = MNM_SL_VERTEX;

      // CAL-09/10/03: These don't seem to be necessary.
      // Besides, the topology & numv might be changed after FillInMesh. (Defect #520585)
      // mnMesh->InvalidateGeomCache();
      // mnMesh->FillInMesh();

      }
   else if (patch)
      {
      if (relax > .3f) relax = .3f;
	      RelaxPatch(subobjectMode,iter,relax,boundary,mod);
      return;
      
      }



   if(mesh) {
      int i, j, max;
      DWORD selLevel = rmesh->selLevel;
      // mjm - 4.8.99 - add support for soft selection
      // sca - 4.29.99 - extended soft selection support to cover EDGE and FACE selection levels.
      float *vsw = (selLevel!=MESH_OBJECT) ? rmesh->getVSelectionWeights() : NULL;


      if (1) {
         rd->SetVNum (rmesh->numVerts);
         for (i=0; i<rd->vnum; i++) {
            rd->fnum[i]=0;
            rd->nbor[i].ZeroCount();
         }
         rd->sel.ClearAll ();
         DWORD *v;
         int k1, k2, origmax;
         for (i=0; i<rmesh->numFaces; i++) {
            v = rmesh->faces[i].v;
            for (j=0; j<3; j++) {
               if ((selLevel==MESH_FACE) && rmesh->faceSel[i]) rd->sel.Set(v[j]);
               if ((selLevel==MESH_EDGE) && rmesh->edgeSel[i*3+j]) rd->sel.Set(v[j]);
               if ((selLevel==MESH_EDGE) && rmesh->edgeSel[i*3+(j+2)%3]) rd->sel.Set(v[j]);
               origmax = max = rd->nbor[v[j]].Count();
               rd->fnum[v[j]]++;
               for (k1=0; k1<max; k1++) if (rd->nbor[v[j]][k1] == v[(j+1)%3]) break;
               if (k1==max) { rd->nbor[v[j]].Append (1, v+(j+1)%3, 1); max++; }
               for (k2=0; k2<max; k2++) if (rd->nbor[v[j]][k2] == v[(j+2)%3]) break;
               if (k2==max) { rd->nbor[v[j]].Append (1, v+(j+2)%3, 1); max++; }
               if (max>origmax) rd->vis[v[j]].SetSize (max, TRUE);
               if (rmesh->faces[i].getEdgeVis (j)) rd->vis[v[j]].Set (k1);
               else if (k1>=origmax) rd->vis[v[j]].Clear (k1);
               if (rmesh->faces[i].getEdgeVis ((j+2)%3)) rd->vis[v[j]].Set (k2);
               else if (k2>= origmax) rd->vis[v[j]].Clear (k2);
            }
         }
   // mjm - begin - 4.8.99
   //    if (selLevel==MESH_VERTEX) rd->sel = mesh->vertSel;
         if (selLevel==MESH_VERTEX)
            rd->sel = rmesh->vertSel;
         else if (selLevel==MESH_OBJECT)
            rd->sel.SetAll ();
   // mjm - end
      }

      Tab<float> vangles;
      if (saddle) vangles.SetCount (rd->vnum);
      Point3 *hold = new Point3[rd->vnum];
      int act;
      for (int k=0; k<iter; k++) 
	  {
		 if (GetUserCancel())
			 k = iter;
         for (i=0; i<rd->vnum; i++) hold[i] = rmesh->verts[i];
         if (saddle) rmesh->FindVertexAngles (vangles.Addr(0));
         for (i=0; i<rd->vnum; i++) {
   // mjm - begin - 4.8.99
   //       if ((selLevel!=MESH_OBJECT) && (!rd->sel[i])) continue;
            if ( (!rd->sel[i] ) && (!vsw || vsw[i] == 0) ) continue;
   // mjm - end
            if (saddle && (vangles[i] <= 2*PI*.99999f)) continue;
            max = rd->nbor[i].Count();
            if (boundary && (rd->fnum[i] < max)) continue;
            if (max<1) continue;
            Point3 avg(0.0f, 0.0f, 0.0f);
            for (j=0,act=0; j<max; j++) {
               if (!rd->vis[i][j]) continue;
               act++;
               avg += hold[rd->nbor[i][j]];
            }
            if (act<1) continue;
   // mjm - begin - 4.8.99
            wtdRelax = (!rd->sel[i]) ? relax * vsw[i] : relax;
            rmesh->verts[i] = hold[i]*(1-wtdRelax) + avg*wtdRelax/((float)act);
   //       triObj->SetPoint (i, hold[i]*(1-relax) + avg*relax/((float)act));
   // mjm - end
         }
		 if ((k%10) == 0)
		{
			for (int i = 0; i < TVMaps.v.Count(); i++)
			{
				Point3 p = GetTVVert(i);
				p.x = rmesh->verts[i].x;
				p.y = rmesh->verts[i].y;
				SetTVVert(t,i,p,mod);

			}
			if (updateUI)
				mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
			mod->InvalidateView();
		 }

      }
      delete [] hold;
   }

   if (rmnMesh) {
      int i, j, max;
      MNMesh & mm = *rmnMesh;
      float *vsw = (mm.selLevel!=MNM_SL_OBJECT) ? mm.getVSelectionWeights() : NULL;


      if (1) {
         rd->SetVNum (mm.numv);
         for (i=0; i<rd->vnum; i++) {
            rd->fnum[i]=0;
            rd->nbor[i].ZeroCount();
         }
         rd->sel = mm.VertexTempSel ();
         int k1, k2, origmax;
         for (i=0; i<mm.numf; i++) {
            int deg = mm.f[i].deg;
            int *vtx = mm.f[i].vtx;
            for (j=0; j<deg; j++) {
               Tab<DWORD> & nbor = rd->nbor[vtx[j]];
               origmax = max = nbor.Count();
               rd->fnum[vtx[j]]++;
               DWORD va = vtx[(j+1)%deg];
               DWORD vb = vtx[(j+deg-1)%deg];
               for (k1=0; k1<max; k1++) if (nbor[k1] == va) break;
               if (k1==max) { nbor.Append (1, &va, 1); max++; }
               for (k2=0; k2<max; k2++) if (nbor[k2] == vb) break;
               if (k2==max) { nbor.Append (1, &vb, 1); max++; }
            }
         }
      }

      Tab<float> vangles;
      if (saddle) vangles.SetCount (rd->vnum);
      Tab<Point3> hold;
      hold.SetCount (rd->vnum);
      int act;
      for (int k=0; k<iter; k++) 
	  {
		 if (GetUserCancel())
			  k = iter;
         for (i=0; i<rd->vnum; i++) hold[i] = mm.P(i);
         if (saddle) FindVertexAngles (mm, vangles.Addr(0));
         for (i=0; i<rd->vnum; i++) {
            if ((!rd->sel[i]) && (!vsw || vsw[i] == 0) ) continue;
            if (saddle && (vangles[i] <= 2*PI*.99999f)) continue;
            max = rd->nbor[i].Count();
            if (boundary && (rd->fnum[i] < max)) continue;
            if (max<1) continue;
            Point3 avg(0.0f, 0.0f, 0.0f);
            for (j=0,act=0; j<max; j++) {
               act++;
               avg += hold[rd->nbor[i][j]];
            }
            if (act<1) continue;
            wtdRelax = (!rd->sel[i]) ? relax * vsw[i] : relax;
            mm.P(i) =  hold[i]*(1-wtdRelax) + avg*wtdRelax/((float)act);
         }
     
		  if ((k%10) == 0)
		  {
			  for (int i = 0; i < TVMaps.v.Count(); i++)
			  {
				  Point3 p = GetTVVert(i);
				  p.x = rmnMesh->P(i).x;
				  p.y = rmnMesh->P(i).y;
				  SetTVVert(t,i,p,mod);
			  }

			  if (updateUI)
				mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
			  mod->InvalidateView();
		  }
	  }
   }

   if (rmesh) 
      {
      // CAL-09/10/03: Use TVMaps.v.Count() (was mesh->numVerts)
      for (int i = 0; i < TVMaps.v.Count(); i++)
         {
			 Point3 p = GetTVVert(i);
			 p.x = rmesh->verts[i].x;
			 p.y = rmesh->verts[i].y;
			 SetTVVert(t,i,p,mod);

         }
      delete rmesh;
      }
   if (rmnMesh) 
      {
      // CAL-09/10/03: Use TVMaps.v.Count() (was mnMesh->numv), because numv might be changed after FillInMesh. (Defect #520585)
      for (int i = 0; i < TVMaps.v.Count(); i++)
         {
			 Point3 p = GetTVVert(i);
			 p.x = rmnMesh->P(i).x;
			 p.y = rmnMesh->P(i).y;
			 SetTVVert(t,i,p,mod);
         }

      delete rmnMesh;
      }
   if (rpatch) delete rpatch;


   if (mod->peltData.peltDialog.hWnd)
      mod->peltData.RelaxRig(iter,relax,boundary,mod);


}



void MeshTopoData::RelaxPatch(int subobjectMode, int iteration, float str, BOOL lockEdges,UnwrapMod *mod)
{
	TimeValue t = GetCOREInterface()->GetTime();
	TransferSelectionStart(subobjectMode); 

	BOOL clearSel = FALSE;
	if (mVSel.NumberSet() == 0) 
	{
		clearSel = TRUE;
		mVSel.SetAll();
	}

	// get the number of polyons
	int faceCount = TVMaps.f.Count();
	if (faceCount == 0) return;

	Tab<Point3> centerList;
	Tab<int> centerCount;
	centerList.SetCount(faceCount);
	centerCount.SetCount(faceCount);

	//loop through faces
	Point3 p = Point3(0.0f, 0.0f, 0.0f);
	for (int faceIndex = 0; faceIndex < faceCount; faceIndex++)
	{
		//compute center points
		centerList[faceIndex] = p;
		centerCount[faceIndex] = 0;
	}

	int vertexCount = TVMaps.v.Count();
	if (vertexCount == 0) return;


	for (int faceIndex = 0; faceIndex < faceCount; faceIndex++)
	{

		int pointCount = TVMaps.f[faceIndex]->count;

		for (int pointIndex = 0; pointIndex < pointCount; pointIndex++)
		{
			int index =  TVMaps.f[faceIndex]->t[pointIndex];
			p = TVMaps.v[index].GetP();
			centerList[faceIndex] = centerList[faceIndex] + p;
			centerCount[faceIndex] = centerCount[faceIndex] + 1;
		}
	}

	for (int faceIndex = 0; faceIndex < faceCount; faceIndex++)
	{
		centerList[faceIndex] = centerList[faceIndex]/centerCount[faceIndex];
	}

	//store off orignal points
	Tab<Point3> originalPoints;
	originalPoints.SetCount(vertexCount);

	for (int j = 0; j < vertexCount; j++)
	{
		originalPoints[j] = TVMaps.v[j].GetP();
	}


	//set iteration level
	Tab<Point3> deltaPoints;
	deltaPoints.SetCount(vertexCount);

	for (int i = 0; i < iteration; i++)
	{

		if (GetUserCancel())
			i = iteration;
		for (int j = 0; j < vertexCount; j++)
		{
			deltaPoints[j] = originalPoints[j];
		}

		for (int faceIndex = 0; faceIndex < faceCount; faceIndex++)
		{
			int edgeCount = TVMaps.f[faceIndex]->count;

			for (int pointIndex = 0; pointIndex < edgeCount; pointIndex++)
			{
				int index = TVMaps.f[faceIndex]->t[pointIndex];
				Point3 p = Point3(0.0f, 0.0f, 0.0f);
				p = (centerList[faceIndex] - originalPoints[index]) * str;
				deltaPoints[index] = deltaPoints[index] + p;
				if ( (!(TVMaps.f[faceIndex]->flags & FLAG_DEAD)) &&
					(TVMaps.f[faceIndex]->flags & FLAG_CURVEDMAPPING) &&
					(TVMaps.f[faceIndex]->vecs) )
				{
					index = TVMaps.f[faceIndex]->vecs->handles[pointIndex*2];
					p = (centerList[faceIndex] - originalPoints[index]) * str;
					deltaPoints[index] = deltaPoints[index] + p;

					index = TVMaps.f[faceIndex]->vecs->handles[pointIndex*2+1];
					p = (centerList[faceIndex] - originalPoints[index]) * str;
					deltaPoints[index] = deltaPoints[index] + p;

					if (TVMaps.f[faceIndex]->flags & FLAG_INTERIOR)
					{
						index  = TVMaps.f[faceIndex]->vecs->interiors[pointIndex];
						p = (centerList[faceIndex] - originalPoints[index]) * str;
						deltaPoints[index] = deltaPoints[index] + p;
					}

				}


			}
		}     

		for (int j = 0; j < vertexCount; j++)
		{
			originalPoints[j] = deltaPoints[j];
		}
	}

	BitArray skipVerts;
	skipVerts.SetSize(vertexCount);
	skipVerts.ClearAll();


	if (lockEdges)
	{

		BitArray tempFSel;
		tempFSel.SetSize(TVMaps.f.Count());
		tempFSel.ClearAll();

		for (int j = 0; j < TVMaps.f.Count(); j++)
		{
			int count = TVMaps.f[j]->count;
			int selCount = 0;
			for (int k = 0; k < count; k++)
			{
				int index = TVMaps.f[j]->t[k];
				if (mVSel[index] || TVMaps.v[index].GetInfluence() > 0.0f)
					selCount++;
			}
			if (selCount == count)
				tempFSel.Set(j);
		}
		for (int j = 0; j < TVMaps.ePtrList.Count(); j++)
		{
			int a,b, avec, bvec;
			int fcount;
			fcount = TVMaps.ePtrList[j]->faceList.Count();
			a = TVMaps.ePtrList[j]->a;
			b = TVMaps.ePtrList[j]->b;
			avec = TVMaps.ePtrList[j]->avec;
			bvec = TVMaps.ePtrList[j]->bvec;

			int selCount = 0;
			for (int k =0; k < fcount; k++)
			{
				if (tempFSel[TVMaps.ePtrList[j]->faceList[k]])
					selCount++;
			}
			if ((fcount == 1) || (selCount == 1))
			{
				skipVerts.Set(a);
				skipVerts.Set(b);
				if ((avec >= 0) && (avec < TVMaps.v.Count())) skipVerts.Set(avec);
				if ((bvec >= 0) && (bvec < TVMaps.v.Count())) skipVerts.Set(bvec);
			}
		}
	}

	for (int j = 0; j < vertexCount; j++)
	{
		float weight = 0.0f;
		if (mVSel[j])
			weight = 1.0f;
		else  
			weight = TVMaps.v[j].GetInfluence();

		if (skipVerts[j]) weight = 0.0f;

		if (weight > 0.0f) 
		{
			Point3 originalPos = Point3(0.0f, 0.0f, 0.0f);
			originalPos = TVMaps.v[j].GetP();

			Point3 p = (originalPoints[j] - originalPos) * weight;

			originalPos = originalPos + p;

			SetTVVert(t,j,originalPos,mod);

		}
	}

	if (clearSel)
		mVSel.ClearAll();

	TransferSelectionEnd(subobjectMode,FALSE,FALSE);

}



void MeshTopoData::Relax(int subobjectMode, int iteration, float str, BOOL lockEdges, BOOL matchArea,UnwrapMod *mod)
{
// get the number of polyons
   int faceCount = TVMaps.f.Count();
   if (faceCount == 0) return;

   Tab<Point3> centerList;
   Tab<int> centerCount;
   centerList.SetCount(faceCount);
   centerCount.SetCount(faceCount);

//loop through faces
   Point3 p = Point3(0.0f, 0.0f, 0.0f);
   for (int faceIndex = 0; faceIndex < faceCount; faceIndex++)
      {
//compute center points
      centerList[faceIndex] = p;
      centerCount[faceIndex] = 0;
      }
   
   int vertexCount = TVMaps.v.Count();
   if (vertexCount == 0) return;


   for (int faceIndex = 0; faceIndex < faceCount; faceIndex++)
      {

      int pointCount = TVMaps.f[faceIndex]->count;
   
      for (int pointIndex = 0; pointIndex < pointCount; pointIndex++)
         {
         int index =  TVMaps.f[faceIndex]->t[pointIndex];
         p = TVMaps.v[index].GetP();
         centerList[faceIndex] = centerList[faceIndex] + p;
         centerCount[faceIndex] = centerCount[faceIndex] + 1;
         }
      }

   for (int faceIndex = 0; faceIndex < faceCount; faceIndex++)
      {
      centerList[faceIndex] = centerList[faceIndex]/centerCount[faceIndex];
      }
   
//store off orignal points
   Tab<Point3> originalPoints;
   originalPoints.SetCount(vertexCount);
   
   for (int j = 0; j < vertexCount; j++)
      {
      originalPoints[j] = TVMaps.v[j].GetP();
      }


//set iteration level
   Tab<Point3> deltaPoints;
   deltaPoints.SetCount(vertexCount);

   for (int i = 0; i < iteration; i++)
      {

      for (int j = 0; j < vertexCount; j++)
         {
         deltaPoints[j] = originalPoints[j];
         }
      
      for (int faceIndex = 0; faceIndex < faceCount; faceIndex++)
         {
         int edgeCount = TVMaps.f[faceIndex]->count;
   
         for (int pointIndex = 0; pointIndex < edgeCount; pointIndex++)
            {
            int index = TVMaps.f[faceIndex]->t[pointIndex];
            Point3 p = Point3(0.0f, 0.0f, 0.0f);
            p = (centerList[faceIndex] - originalPoints[index]) * str;
            deltaPoints[index] = deltaPoints[index] + p;
            if ( (!(TVMaps.f[faceIndex]->flags & FLAG_DEAD)) &&
                  (TVMaps.f[faceIndex]->flags & FLAG_CURVEDMAPPING) &&
                  (TVMaps.f[faceIndex]->vecs) )
               {
               index = TVMaps.f[faceIndex]->vecs->handles[pointIndex*2];
               p = (centerList[faceIndex] - originalPoints[index]) * str;
               deltaPoints[index] = deltaPoints[index] + p;

               index = TVMaps.f[faceIndex]->vecs->handles[pointIndex*2+1];
               p = (centerList[faceIndex] - originalPoints[index]) * str;
               deltaPoints[index] = deltaPoints[index] + p;
               
               if (TVMaps.f[faceIndex]->flags & FLAG_INTERIOR)
                  {
                  index  = TVMaps.f[faceIndex]->vecs->interiors[pointIndex];
                  p = (centerList[faceIndex] - originalPoints[index]) * str;
                  deltaPoints[index] = deltaPoints[index] + p;
                  }

               }
      

            }
         }     
      
      for (int j = 0; j < vertexCount; j++)
         {
         originalPoints[j] = deltaPoints[j];
         }
      }

   BitArray skipVerts;
   skipVerts.SetSize(vertexCount);
   skipVerts.ClearAll();

   TransferSelectionStart(subobjectMode); 

   if (lockEdges)
      {

      BitArray tempFSel;
      tempFSel.SetSize(TVMaps.f.Count());
      tempFSel.ClearAll();

      for (int j = 0; j < TVMaps.f.Count(); j++)
         {
         int count = TVMaps.f[j]->count;
         int selCount = 0;
         for (int k = 0; k < count; k++)
            {
            int index = TVMaps.f[j]->t[k];
            if (mVSel[index] || TVMaps.v[index].GetInfluence() > 0.0f)
               selCount++;
            }
         if (selCount == count)
            tempFSel.Set(j);
         }
      for (int j = 0; j < TVMaps.ePtrList.Count(); j++)
         {
         int a,b, avec, bvec;
         int fcount;
         fcount = TVMaps.ePtrList[j]->faceList.Count();
         a = TVMaps.ePtrList[j]->a;
         b = TVMaps.ePtrList[j]->b;
         avec = TVMaps.ePtrList[j]->avec;
         bvec = TVMaps.ePtrList[j]->bvec;

         int selCount = 0;
         for (int k =0; k < fcount; k++)
            {
            if (tempFSel[TVMaps.ePtrList[j]->faceList[k]])
               selCount++;
            }
         if ((fcount == 1) || (selCount == 1))
            {
            skipVerts.Set(a);
            skipVerts.Set(b);
            if ((avec >= 0) && (avec < TVMaps.v.Count())) skipVerts.Set(avec);
            if ((bvec >= 0) && (bvec < TVMaps.v.Count())) skipVerts.Set(bvec);
            }

         }
      }
   

   
   TimeValue t = GetCOREInterface()->GetTime();
   
   for (int j = 0; j < vertexCount; j++)
      {
      float weight = 0.0f;
      if (mVSel[j])
         weight = 1.0f;
      else  
         weight = TVMaps.v[j].GetInfluence();
   
      if (skipVerts[j]) weight = 0.0f;
   
      if (weight > 0.0f) 
         {
         Point3 originalPos = Point3(0.0f, 0.0f, 0.0f);
         originalPos = TVMaps.v[j].GetP();

         Point3 p = (originalPoints[j] - originalPos) * weight;
      
         originalPos = originalPos + p;
		 SetTVVert(t,j,originalPos,mod);
//         TVMaps.v[j].p = originalPos;
  //       if (TVMaps.cont[j]) TVMaps.cont[j]->SetValue(t,&TVMaps.v[j].p);
         }
      }

   TransferSelectionEnd(subobjectMode,FALSE,FALSE); 

}

void MeshTopoData::RelaxFit(int subobjectMode,int iteration, float str, UnwrapMod *mod)
{

//get scale

   
   float scale = 1.0f;
   float tvLen = 0.0f, gLen = 0.0f;
//get the edge length in tv space
//get the edge length in obj space
   Matrix3 toWorld(1);




   TransferSelectionStart(subobjectMode); 

   
   for (int i = 0; i < TVMaps.f.Count(); i++)
      {
      int edgeCount = TVMaps.f[i]->count;
//get geom points transfer in face space
      for (int j = 0; j < edgeCount; j++)
         {
         int a,b;
         a = TVMaps.f[i]->t[j];

         if ((j+1) == edgeCount)
            b = TVMaps.f[i]->t[0];
         else b = TVMaps.f[i]->t[j+1];

         if (mVSel[a] && mVSel[b])
            {
            tvLen += Length(TVMaps.v[a].GetP()-TVMaps.v[b].GetP());
   
            a = TVMaps.f[i]->v[j];
            if ((j+1) == edgeCount)
               b = TVMaps.f[i]->v[0];
            else b = TVMaps.f[i]->v[j+1];
            gLen += Length(TVMaps.geomPoints[a]-TVMaps.geomPoints[b]);
            }
         }
      }

//get the scale from this
   scale = gLen/tvLen;

//build center list
//build matrix for each geom face
   Tab<Matrix3> geomFaceTM;
   geomFaceTM.SetCount(TVMaps.f.Count());
   for (int i = 0; i < TVMaps.f.Count(); i++)
      {
      geomFaceTM[i] = GetTMFromFace(i, FALSE);
      geomFaceTM[i].Scale(Point3(scale,scale,scale));
      geomFaceTM[i] = Inverse(geomFaceTM[i]);

      }


//loop through faces
//tv
   TimeValue t = GetCOREInterface()->GetTime();
   for (int k = 0; k < iteration;k++)
      {
      Tab<Point3> deltaList;
      deltaList.SetCount(TVMaps.v.Count());
      for (int i = 0; i < TVMaps.v.Count(); i++)
         deltaList[i] = Point3(0.0f,0.0f,0.0f);

      for (int i = 0; i < TVMaps.f.Count(); i++)
         {
   //get uv matrix
         Matrix3 uvFaceTM = GetTMFromFace(i, TRUE);
         Matrix3 gmFaceTM = geomFaceTM[i];

         int edgeCount = TVMaps.f[i]->count;

         Tab<Point3> geomList;
         Tab<Point3> uvList;
         geomList.SetCount(edgeCount);
         uvList.SetCount(edgeCount);
   //get geom points transfer in face space
         for (int j = 0; j < edgeCount; j++)
            {
            int index = TVMaps.f[i]->v[j];
            geomList[j] = TVMaps.geomPoints[index];
            geomList[j] = geomList[j] * geomFaceTM[i]; //optimize to collapse matrix


            index = TVMaps.f[i]->t[j];
            uvList[j] = TVMaps.v[index].GetP();
            uvList[j] = uvList[j] * Inverse(uvFaceTM); //optimize to collapse matrix
            Point3 vec;
            vec = (geomList[j] - uvList[j])*str;
            vec = VectorTransform(uvFaceTM,vec);
            deltaList[index] += vec;
            }
         }
      for (int i = 0; i < TVMaps.v.Count(); i++)
         {
         float weight = 1.0f;
         if (mVSel[i])
            weight = 1.0f;
         else  weight = TVMaps.v[i].GetInfluence();
		 Point3 p = TVMaps.v[i].GetP() +  (deltaList[i]*weight);
		 SetTVVert(t,i,p,mod);
//         TVMaps.v[i].p += deltaList[i]*weight;
  //       if (TVMaps.cont[i]) TVMaps.cont[i]->SetValue(t,&TVMaps.v[i].p);
         }
      }


   TransferSelectionEnd(subobjectMode,FALSE,FALSE); 
}

void MeshTopoData::RelaxByFaceAngle(int subobjectMode, int frames, float stretch, float str, BOOL lockEdges,HWND statusHWND, UnwrapMod *mod, BOOL applyToAll)

{

	stretch = (1.0f-(stretch));
	TimeValue t = GetCOREInterface()->GetTime();

	float reducer = 1.0f;


	TransferSelectionStart(subobjectMode); 

	BitArray mVSelTemp = mVSel;
	if (applyToAll)
	{
		mVSelTemp.ClearAll();
		for (int i = 0; i < GetNumberTVVerts(); i++)
		{
			if (IsTVVertVisible(i))
				mVSelTemp.Set(i,TRUE);
		}
	}


	BitArray rigPoint;
	MeshTopoData *peltLD = mod->peltData.mBaseMeshTopoDataCurrent;
	if (mod->peltData.peltDialog.hWnd)
	{		
		if (peltLD == this)
		{
			rigPoint.SetSize(peltLD->GetNumberTVVerts());//TVMaps.v.Count());
			rigPoint.ClearAll();
			for (int i = 0; i < mod->peltData.rigPoints.Count(); i++)
			{
				rigPoint.Set(mod->peltData.rigPoints[i].lookupIndex);
			}
		}
	}

	BitArray edgeVerts;
	edgeVerts.SetSize(TVMaps.v.Count());
	edgeVerts.ClearAll();

	float uvEdgeDist = 0.0f;
	float geomEdgeDist = 0.0f;

	Tab<float> originalWs;
	originalWs.SetCount( TVMaps.v.Count());
	for (int i = 0; i < originalWs.Count(); i++)
	{
		Point3 p =  TVMaps.v[i].GetP();
		originalWs[i] = p.z;
		p.z = 0.0f;
		TVMaps.v[i].SetP(p);//p.z = 0.0f;
	}




	for (int i = 0; i < TVMaps.ePtrList.Count(); i++)
	{
		int a = TVMaps.ePtrList[i]->a;
		int b = TVMaps.ePtrList[i]->b;

		if (TVMaps.ePtrList[i]->faceList.Count() == 1)
		{
			edgeVerts.Set(a,TRUE);
			edgeVerts.Set(b,TRUE);
		}
	}

	int ct = 0;
	for (int i = 0; i < TVMaps.ePtrList.Count(); i++)
	{
		int a = TVMaps.ePtrList[i]->a;
		int b = TVMaps.ePtrList[i]->b;
		int ga = TVMaps.ePtrList[i]->ga;
		int gb = TVMaps.ePtrList[i]->gb;

		float wa = 0.0f;
		float wb = 0.0f;

		if (mVSelTemp[a]) 
			wa = 1.0f;
		else wa = TVMaps.v[a].GetInfluence();
		if (mVSelTemp[b] ) 
			wb = 1.0f;
		else wb = TVMaps.v[b].GetInfluence();
		if ((wa > 0.0f) && (wb == 0.0f))
			edgeVerts.Set(a,TRUE);
		if ((wb > 0.0f) && (wa == 0.0f))
			edgeVerts.Set(b,TRUE);

		if ((wa > 0.0f) && (wb > 0.0f))
		{

			uvEdgeDist += Length(TVMaps.v[a].GetP() - TVMaps.v[b].GetP() );
			geomEdgeDist += Length(TVMaps.geomPoints[ga]-TVMaps.geomPoints[gb]);
			ct++;
		}

	}

	if (ct)
	{

		uvEdgeDist = uvEdgeDist/(ct);
		geomEdgeDist = geomEdgeDist/(ct);
		float edgeScale = (uvEdgeDist/geomEdgeDist) *reducer;


		//get the normals at each vertex

		Tab<Point3> geomFaceNormals;
		geomFaceNormals.SetCount(TVMaps.f.Count());
		for (int i =0; i < TVMaps.f.Count(); i++)
		{
			geomFaceNormals[i] = TVMaps.GeomFaceNormal(i);
		}




		Tab<Matrix3> tmList;
		tmList.SetCount(TVMaps.f.Count());
		for (int i = 0; i < tmList.Count(); i++)
		{
			MatrixFromNormal(geomFaceNormals[i],tmList[i]);
			tmList[i] = Inverse(tmList[i]);
		}

		Tab<MyPoint3Tab*> gFacePoints;


		gFacePoints.SetCount(TVMaps.f.Count());

		for (int i = 0; i < TVMaps.f.Count(); i++)
		{
			gFacePoints[i] = NULL;
			if (!GetFaceDead(i))
			{			
				gFacePoints[i] = new MyPoint3Tab();
				int deg = TVMaps.f[i]->count;
				gFacePoints[i]->p.SetCount(deg);

				Point3 p(0.0f,0.0f,0.0f);
				Point3 up(0.0f,0.0f,0.0f);
				for (int j = 0; j < deg; j++)
				{
					int index = TVMaps.f[i]->v[j];
					int uindex = TVMaps.f[i]->t[j];



					gFacePoints[i]->p[j] = TVMaps.geomPoints[index] * tmList[i];
					gFacePoints[i]->p[j].z = 0.0f;
					p += gFacePoints[i]->p[j];
				}
				Point3 gCenter = p/deg;


				for (int j = 0; j < deg; j++)
				{
					gFacePoints[i]->p[j] -= gCenter;
					gFacePoints[i]->p[j] *= edgeScale;
					Point3 p = gFacePoints[i]->p[j];
				}
			}
		}

		Tab<Point3> deltaList;
		Tab<int> deltaCount;

		deltaCount.SetCount(TVMaps.v.Count());
		deltaList.SetCount(TVMaps.v.Count());

		BitArray useFace;
		useFace.SetSize(TVMaps.f.Count());
		for (int i = 0; i < TVMaps.f.Count(); i++)
		{
			if (!GetFaceDead(i))
			{			
				int deg = TVMaps.f[i]->count;

				BOOL sel = FALSE;
				for (int j = 0; j < deg; j++)
				{
					int index = TVMaps.f[i]->t[j];
					float w = 0.0f;
					if (mVSelTemp[index]) 
						w = 1.0f;
					else w = TVMaps.v[index].GetInfluence();
					if (w > 0.f) sel = TRUE;

				}
				if (sel) useFace.Set(i,TRUE);
			}
		}

		for (int i = 0; i < TVMaps.f.Count(); i++)
		{
			if (!GetFaceDead(i))
			{	
				int deg = TVMaps.f[i]->count;

				for (int j = 0; j < deg; j++)
				{
					int index = TVMaps.f[i]->t[j];
					if (!edgeVerts[index])
						gFacePoints[i]->p[j] *= stretch;				
				}
			}
		}


		for (int cframe = 0; cframe < frames; cframe++)
		{

			if (GetUserCancel())
			{
				cframe = frames;
			}

			if (statusHWND != NULL)
			{
				TSTR progress;
				progress.printf("%s %d(%d)",GetString(IDS_PROCESS),cframe,frames);
				SetWindowText(statusHWND,progress);
			}

			for (int i = 0; i < TVMaps.v.Count(); i++)
			{
				deltaList[i] = Point3(0.0f,0.0f,0.0f);
				deltaCount[i] = 0;
			}

			for (int i = 0; i < TVMaps.f.Count(); i++)
			{
				//line up the face
				if (useFace[i])
				{
					int deg =  TVMaps.f[i]->count;
					Tab<Point3> uvPoints;
					uvPoints.SetCount(deg);
					Point3 center(0.0f,0.0f,0.0f);
					for (int j = 0; j < deg; j++)
					{
						int uindex = TVMaps.f[i]->t[j];
						uvPoints[j] = TVMaps.v[uindex].GetP();
						center += uvPoints[j];
					}
					center = center/(float)deg;
					for (int j = 0; j < deg; j++)
					{
						uvPoints[j] -= center;
					}


					//add the deltas


					float fangle = 0.0f;
					for (int j = 0; j < deg; j++)
					{
						Point3 gvec = gFacePoints[i]->p[j];
						Point3 uvec = uvPoints[j];
						uvec = Normalize(uvec);
						gvec = Normalize(gvec);
						float ang = AngleFromVectors(gvec,uvec);
						fangle += ang;
					}
					float fangle2 = -fangle/(deg);

					Matrix3 tm(1);


					Point3 rvec(0.0f,0.0f,0.0f);
					Point3 ovec = Normalize(gFacePoints[i]->p[0]);
					for (int j = 0; j < deg; j++)
					{
						Point3 gvec = gFacePoints[i]->p[j];
						Point3 uvec = uvPoints[j];
						uvec = Normalize(uvec);
						gvec = Normalize(gvec);
						float ang = AngleFromVectors(gvec,uvec);
						Matrix3 rtm(1);
						rtm.RotateZ(ang);
						rvec += ovec * rtm;					 
					}
					rvec = Normalize(rvec);
					fangle = AngleFromVectors(ovec,rvec);

					tm.RotateZ(fangle);


					for (int j = 0; j < deg; j++)
					{
						Point3 gvec = gFacePoints[i]->p[j] * tm;
						Point3 delta = gvec - uvPoints[j];

						int uindex = TVMaps.f[i]->t[j];
						deltaList[uindex] += delta;
						deltaCount[uindex] += 1;				
					}
				}
			}
			for (int i = 0; i < TVMaps.v.Count(); i++)
			{
				float w = 0.0f;
				if (mVSelTemp[i]) 
					w = 1.0f;
				else w = TVMaps.v[i].GetInfluence();
				BOOL isRigPoint = FALSE;
				if (this==peltLD)
				{
					if (i < rigPoint.GetSize())
						isRigPoint =  rigPoint[i];
				}
				if ((w > 0.0f) && (!(TVMaps.v[i].IsFrozen())) && !isRigPoint && deltaCount[i])//	(!rigPoint[i])))
				{
					if (lockEdges)
					{

						if (!edgeVerts[i])
							TVMaps.v[i].SetP(TVMaps.v[i].GetP() + ((deltaList[i]/(float) deltaCount[i]) * w * str));			 
					}
					else TVMaps.v[i].SetP(TVMaps.v[i].GetP() + ( (deltaList[i]/(float) deltaCount[i]) * w * str));	
				}
			}

			if ((cframe%10) == 0)
			{
				mod->peltData.ResolvePatchHandles(mod);				
				mod->InvalidateView();
				if (statusHWND)
				{
					mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
					UpdateWindow(mod->hWnd);
					if (mod->ip) mod->ip->RedrawViews(t);	
				}
			}
		}

		for (int i = 0; i < TVMaps.f.Count(); i++)
		{
			delete gFacePoints[i];
		}


		TSTR progress;

		if (statusHWND)
		{
			progress.printf("    ");
			SetWindowText(statusHWND,progress);
		}

		for (int i = 0; i < originalWs.Count(); i++)
		{
			Point3 p = TVMaps.v[i].GetP();
			p.z = originalWs[i];
			TVMaps.v[i].SetP(p);//TVMaps.v[i].p.z = originalWs[i];

		}

		if (mod->peltData.peltDialog.hWnd && !applyToAll)
			mod->peltData.RelaxRig(frames,str,lockEdges,mod);

		//	TimeValue t = GetCOREInterface()->GetTime();
		for (int i = 0; i < TVMaps.v.Count(); i++)
		{
			float w = 0.0f;
			if (mVSelTemp[i] ) 
				w = 1.0f;
			else w = TVMaps.v[i].GetInfluence();
			if ( (w > 0.0f) && (!(TVMaps.v[i].IsFrozen())))
			{	
				Point3 p = TVMaps.v[i].GetP();
				SetTVVert(t,i,p,mod);//if (TVMaps.cont[i]) TVMaps.cont[i]->SetValue(t,&TVMaps.v[i].p);

			}
		}

		mod->peltData.ResolvePatchHandles(mod);

	}

	if (mod->peltData.peltDialog.hWnd)
		mod->peltData.RelaxRig(frames,str,lockEdges,mod);

	TransferSelectionEnd(subobjectMode,FALSE,FALSE); 

}



void MeshTopoData::RelaxByEdgeAngle(int subobjectMode, int frames, float stretch,float str, BOOL lockEdges,HWND statusHWND, UnwrapMod *mod, BOOL applyToAll)

{
 	stretch = (1.0f-(stretch));
 	float reducer = 1.0f;

	TransferSelectionStart(subobjectMode); 

	BitArray mVSelTemp = mVSel;
	if (applyToAll)
	{
		mVSelTemp.ClearAll();
		for (int i = 0; i < GetNumberTVVerts(); i++)
		{
			if (IsTVVertVisible(i))
				mVSelTemp.Set(i,TRUE);
		}

	}


	BitArray rigPoint;
	MeshTopoData *peltLD = mod->peltData.mBaseMeshTopoDataCurrent;
	if (mod->peltData.peltDialog.hWnd)
	{		
		if (peltLD == this)
		{
			rigPoint.SetSize(peltLD->GetNumberTVVerts());//TVMaps.v.Count());
			rigPoint.ClearAll();
			for (int i = 0; i < mod->peltData.rigPoints.Count(); i++)
			{
				rigPoint.Set(mod->peltData.rigPoints[i].lookupIndex);
			}
		}
	}

	BitArray edgeVerts;
	edgeVerts.SetSize(TVMaps.v.Count());
	edgeVerts.ClearAll();

	BitArray useVerts;
	useVerts.SetSize(TVMaps.v.Count());
	useVerts.ClearAll();



	float uvEdgeDist = 0.0f;
	float geomEdgeDist = 0.0f;

	TimeValue t = GetCOREInterface()->GetTime();

	Tab<float> originalWs;
	originalWs.SetCount( TVMaps.v.Count());
	for (int i = 0; i < originalWs.Count(); i++)
	{
		Point3 p = TVMaps.v[i].GetP();
		originalWs[i] = p.z;
		p.z = 0.0f;
		TVMaps.v[i].SetP(p);
	}


	for (int i = 0; i < TVMaps.v.Count(); i++)
	{
		int a = i;
		float wa = 0.0f;
		if (mVSelTemp[a]) 
			wa = 1.0f;
		else wa = TVMaps.v[a].GetInfluence();
		if (wa > 0.0f) useVerts.Set(a,TRUE);
	}

	for (int i = 0; i < TVMaps.ePtrList.Count(); i++)
	{
		int a = TVMaps.ePtrList[i]->a;
		int b = TVMaps.ePtrList[i]->b;

		if (TVMaps.ePtrList[i]->faceList.Count() == 1)
		{
			edgeVerts.Set(a,TRUE);
			edgeVerts.Set(b,TRUE);
		}

		float wa = 0.0f;
		if (mVSelTemp[a]) 
			wa = 1.0f;
		else wa = TVMaps.v[a].GetInfluence();

		float wb = 0.0f;
		if (mVSelTemp[b]) 
			wb = 1.0f;
		else wb = TVMaps.v[b].GetInfluence();

		if (useVerts[a] && (!useVerts[b]))
			edgeVerts.Set(a,TRUE);
		if (useVerts[b] && (!useVerts[a]))
			edgeVerts.Set(b,TRUE);
	}

	int ct = 0;
	for (int i = 0; i < TVMaps.ePtrList.Count(); i++)
	{
		int a = TVMaps.ePtrList[i]->a;
		int b = TVMaps.ePtrList[i]->b;
		int ga = TVMaps.ePtrList[i]->ga;
		int gb = TVMaps.ePtrList[i]->gb;

		if (useVerts[a] || useVerts[b])
		{
			uvEdgeDist += Length(TVMaps.v[a].GetP() - TVMaps.v[b].GetP() );
			geomEdgeDist += Length(TVMaps.geomPoints[ga]-TVMaps.geomPoints[gb]);
			ct++;
		}

	}

   	uvEdgeDist = uvEdgeDist/(ct);
	geomEdgeDist = geomEdgeDist/(ct);
	float edgeScale = (uvEdgeDist/geomEdgeDist) *reducer;



	//get the normals at each vertex
	
	Tab<Point3> geomFaceNormals;
	geomFaceNormals.SetCount(TVMaps.f.Count());
	for (int i =0; i < TVMaps.f.Count(); i++)
	{
		geomFaceNormals[i] = TVMaps.GeomFaceNormal(i);
	}

	Tab<AdjacentItem*> facesAtVert;
 	TVMaps.BuildAdjacentUVFacesToVerts(facesAtVert);

	Tab<ClusterInfo*> clusterInfo;


	clusterInfo.SetCount(TVMaps.v.Count());

	//loop through our verts
  	for (int i = 0; i < TVMaps.v.Count(); i++)
	{
		clusterInfo[i] = NULL;
	//get the faces attached to this vert
		if (useVerts[i] && facesAtVert[i]->index.Count() )
		{						
			int ct =facesAtVert[i]->index.Count();
			clusterInfo[i] = new ClusterInfo();

			Tab<EdgeVertInfo> edgeVerts;
			edgeVerts.SetCount(ct);
			BitArray usedEdge;
			usedEdge.SetSize(ct);
			usedEdge.ClearAll();

			
 			for (int j = 0; j < ct; j++)
			{
				edgeVerts[j].faceIndex = facesAtVert[i]->index[j];
				int faceIndex = edgeVerts[j].faceIndex;
				int uvVertID = i;
				int geoVertID = TVMaps.f[faceIndex]->GetGeoVertIDFromUVVertID(uvVertID);

				int geoP,geoN;
				int uvN,uvP;
				TVMaps.f[faceIndex]->GetConnectedGeoVerts(geoVertID,geoP,geoN);
				edgeVerts[j].geoIndexA = geoVertID;				
				edgeVerts[j].geoIndexP = geoP;
				edgeVerts[j].geoIndexN = geoN;

				TVMaps.f[faceIndex]->GetConnectedUVVerts(uvVertID,uvP,uvN);
				edgeVerts[j].uvIndexA = uvVertID;
				edgeVerts[j].uvIndexP = uvP;
				edgeVerts[j].uvIndexN = uvN;				
			}
			//now sort them
			usedEdge.Set(0);

			clusterInfo[i]->edgeVerts.Append(1,&edgeVerts[0]);
			int n = edgeVerts[0].uvIndexN;
			int p = edgeVerts[0].uvIndexP;
			
			BOOL done = FALSE;
			while (!done)
			{
				BOOL hit = FALSE;
				for (int j = 0; j < ct; j++)
				{
					if (!usedEdge[j])
					{
						int testN,testP;
						testN = edgeVerts[j].uvIndexN;
						testP = edgeVerts[j].uvIndexP;
						if (testN == p)
						{
							usedEdge.Set(j,TRUE);
							clusterInfo[i]->edgeVerts.Append(1,&edgeVerts[j]);
							p = testP;
							hit = TRUE;
						}
						else if (testP == n)
						{
							usedEdge.Set(j,TRUE);
							clusterInfo[i]->edgeVerts.Insert(0,1,&edgeVerts[j]);
							n = testN;
							hit = TRUE;
						}
					}
				}
				if (!hit) done = TRUE;
			}			
		}
	}

	//now create our ideal cluster info
  	for (int i = 0; i < TVMaps.v.Count(); i++)
	{	
		if (clusterInfo[i])
		{
			BOOL open = TRUE;
			int startID, endID;
			int ct = clusterInfo[i]->edgeVerts.Count();
			startID = clusterInfo[i]->edgeVerts[0].uvIndexN;
			endID = clusterInfo[i]->edgeVerts[ct-1].uvIndexP;
			if (startID == endID)
				open = FALSE;
			float totalAngle = 0.0f;
			
			for (int j = 0; j < ct; j++)
			{
				int nID = clusterInfo[i]->edgeVerts[j].geoIndexN;
				int pID = clusterInfo[i]->edgeVerts[j].geoIndexP;
				
				int centerID = clusterInfo[i]->edgeVerts[j].geoIndexA;
				Point3 center = TVMaps.geomPoints[centerID];

				Point3 vec1 = TVMaps.geomPoints[nID] - center;
				Point3 vec2 = TVMaps.geomPoints[pID] - center;
				vec1 = Normalize(vec1);
				vec2 = Normalize(vec2);
				float angle = fabs(AngleFromVectors(vec2,vec1));
				clusterInfo[i]->edgeVerts[j].angle = angle;
				totalAngle += angle;
			}
			for (int j = 0; j < ct; j++)
			{
				if (open && (totalAngle < (PI*2.0f)))
				{
				}
				else
				{
					float angle = clusterInfo[i]->edgeVerts[j].angle;
					angle = (angle/totalAngle)*(PI*2.0f);
					clusterInfo[i]->edgeVerts[j].angle = angle;
				}				
			}
			float currentAngle = 0.0f;
			for (int j = 0; j < ct; j++)
			{
				int centerID = clusterInfo[i]->edgeVerts[j].geoIndexA;
				Point3 center = TVMaps.geomPoints[centerID];
				Matrix3 tm(1);
				tm.RotateZ(currentAngle);
				int nID = clusterInfo[i]->edgeVerts[j].geoIndexN;
				Point3 vec1 = TVMaps.geomPoints[nID] - center;
				float l = Length(vec1)*edgeScale;
				Point3 yaxis(0.0f,l,0.0f);
				Point3 v = yaxis * tm;
				clusterInfo[i]->edgeVerts[j].idealPos = v;
				clusterInfo[i]->edgeVerts[j].idealPosNormalized = Normalize(v);
				currentAngle += clusterInfo[i]->edgeVerts[j].angle;
			}
			if (open)
			{
				EdgeVertInfo eInfo;
				int centerID = clusterInfo[i]->edgeVerts[ct-1].geoIndexA;
				Point3 center = TVMaps.geomPoints[centerID];

				Matrix3 tm(1);
				tm.RotateZ(currentAngle);
				int nID = clusterInfo[i]->edgeVerts[ct-1].geoIndexP;
				Point3 vec1 = TVMaps.geomPoints[nID] - center;
				float l = Length(vec1)*edgeScale;
				Point3 yaxis(0.0f,l,0.0f);
				Point3 v = yaxis * tm;
				eInfo.idealPos = v;
				eInfo.idealPosNormalized = Normalize(v);
				eInfo.geoIndexN = nID;
				eInfo.uvIndexN = clusterInfo[i]->edgeVerts[ct-1].uvIndexP;
				clusterInfo[i]->edgeVerts.Append(1,&eInfo);
			}
		}
	}

 	for (int i = 0; i < facesAtVert.Count(); i++)
	{
		delete facesAtVert[i];
	}

	Tab<Point3> deltaList;
	Tab<int> deltaCount;

	deltaCount.SetCount(TVMaps.v.Count());
	deltaList.SetCount(TVMaps.v.Count());

	for (int i = 0; i < clusterInfo.Count(); i++)
	{

		if (clusterInfo[i])
		{
			int deg = clusterInfo[i]->edgeVerts.Count();

			for (int j = 0; j < deg; j++)
			{
				int index = clusterInfo[i]->edgeVerts[j].uvIndexN;
				if (!edgeVerts[index])
					clusterInfo[i]->edgeVerts[j].idealPos *= stretch;				
			}
		}
	}

 	for (int cframe = 0; cframe < frames; cframe++)
	{

		if (statusHWND != NULL)
		{
			TSTR progress;
			progress.printf("%s %d(%d)",GetString(IDS_PROCESS),cframe,frames);
			SetWindowText(statusHWND,progress);
		}

		if (GetUserCancel())
		{
			cframe = frames;
		}

		for (int i = 0; i < TVMaps.v.Count(); i++)
		{
			deltaList[i] = Point3(0.0f,0.0f,0.0f);
			deltaCount[i] = 0;
		}

		for (int i = 0; i < clusterInfo.Count(); i++)
		{
			//line up the face
			if (clusterInfo[i])
			{
				int deg = clusterInfo[i]->edgeVerts.Count();
				Tab<Point3> uvPoints;
				uvPoints.SetCount(deg);
				Point3 center(0.0f,0.0f,0.0f);
				for (int j = 0; j < deg; j++)
				{
					int uindex = clusterInfo[i]->edgeVerts[j].uvIndexN;
					uvPoints[j] = TVMaps.v[uindex].GetP();
					center += uvPoints[j];
				}
//				center = center/(float)deg;
				center = TVMaps.v[i].GetP();
				for (int j = 0; j < deg; j++)
				{
					uvPoints[j] -= center;
				}


				//add the deltas

				
  				float fangle = 0.0f;

				float angle = 0.0f;
				float iangle = 0.0f;
				int ct = clusterInfo[i]->edgeVerts.Count();
				
				for (int j = 0; j < ct; j++)
				{
					if (j == 0)
					{
						
						Point3 geomVec1 = clusterInfo[i]->edgeVerts[j].idealPosNormalized;
						geomVec1 = Normalize(geomVec1);

						int uvIndex = clusterInfo[i]->edgeVerts[j].uvIndexN;
						Point3 uvVec1 = TVMaps.v[uvIndex].GetP() - center;

						uvVec1 = Normalize(uvVec1);

						float dot = DotProd(uvVec1,geomVec1);
						Point3 crossProd = CrossProd(uvVec1,geomVec1);
						if (dot >= 0.9999f)
							iangle = 0.0f;
						else if (dot == 0.0f)
							iangle = PI*0.5f;
						else if (dot <= -0.9999f)
							iangle = PI;
						else iangle = acos(dot);
						clusterInfo[i]->angle = iangle;
						if (crossProd.z > 0.0)
							iangle *= -1.0f;
						
					}
					else
					{
						Point3 geomVec1 = clusterInfo[i]->edgeVerts[j].idealPosNormalized;
						geomVec1 = Normalize(geomVec1);
						Matrix3 tm(1);
						tm.RotateZ(iangle);
						geomVec1 = geomVec1 * tm;

						int uvIndex = clusterInfo[i]->edgeVerts[j].uvIndexN;
						Point3 uvVec1 = TVMaps.v[uvIndex].GetP() - center;

						uvVec1 = Normalize(uvVec1);

						float uangle = 0.0f;
						float dot = DotProd(uvVec1,geomVec1);
						if (dot >= 0.9999f)
							uangle += 0.0f;
						else if (dot == 0.0f)
							uangle += PI*0.5f;
						else if (dot <= -0.999f)
							uangle += PI;
						else uangle += acos(dot);

						Point3 crossProd = CrossProd(uvVec1,geomVec1);
						if (crossProd.z > 0.0)
							uangle *= -1.0f;
						angle += uangle;

					}


				}
    				angle = (angle)/(float)(ct-1);
				clusterInfo[i]->counterAngle = angle;
  				fangle = angle;


 				Point3 rvec(0.0f,0.0f,0.0f);
 				Point3 ovec = Normalize(clusterInfo[i]->edgeVerts[0].idealPos);
				BOOL centerIsEdge = FALSE;
				if (edgeVerts[i]) centerIsEdge = TRUE;
				for (int j = 1; j < deg; j++)
				{
					Point3 gvec = clusterInfo[i]->edgeVerts[j].idealPos;
					Point3 uvec = uvPoints[j];
					uvec = Normalize(uvec);
					gvec = Normalize(gvec);
					float ang = AngleFromVectors(gvec,uvec);
					Matrix3 rtm(1);
					rtm.RotateZ(ang);
					if (centerIsEdge)
					{
						int uindex = clusterInfo[i]->edgeVerts[j].uvIndexN;
						if (edgeVerts[uindex])
							rvec += ovec * rtm;					 
					}
					else rvec += ovec * rtm;					 
				}
 				rvec = Normalize(rvec);
				fangle = AngleFromVectors(ovec,rvec);

				Matrix3 tm(1);
				tm.RotateZ(fangle);


				for (int j = 0; j < deg; j++)
				{
					Point3 gvec =  clusterInfo[i]->edgeVerts[j].idealPos * tm;
					clusterInfo[i]->edgeVerts[j].tempPos = gvec;
					Point3 delta = gvec - uvPoints[j];
					delta *= 0.5f;

					int uindex = clusterInfo[i]->edgeVerts[j].uvIndexN;
					deltaList[uindex] += delta;
					deltaCount[uindex] += 1;				

					deltaList[i] -= delta;
					deltaCount[i] += 1;	
				}
			}
		}
 		for (int i = 0; i < TVMaps.v.Count(); i++)
		{
			float w = 0.0f;
			if (mVSelTemp[i] ) 
				w = 1.0f;
			else w = TVMaps.v[i].GetInfluence();
			BOOL isRigPoint = FALSE;
			if (this == peltLD)
			{
				if (i < rigPoint.GetSize())
					isRigPoint = rigPoint[i];
			}

			if ((w > 0.0f) && (!(TVMaps.v[i].IsFrozen()) && (!isRigPoint))&& deltaCount[i]) //&& (!rigPoint[i]))
			{

				if (lockEdges)
				{
					if (!edgeVerts[i])
						TVMaps.v[i].SetP(TVMaps.v[i].GetP() + ((deltaList[i]/(float) deltaCount[i]) * w * str));			 
				}
				else TVMaps.v[i].SetP(TVMaps.v[i].GetP() + ((deltaList[i]/(float) deltaCount[i]) * w * str)); 
			}
		}

		if ((cframe%10) == 0)
		{
			mod->peltData.ResolvePatchHandles(mod);			
			mod->InvalidateView();
			if (statusHWND)
			{
				mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);		
				UpdateWindow(mod->hWnd);
				if (mod->ip) mod->ip->RedrawViews(t);
			}
		}

	}


	mod->peltData.ResolvePatchHandles(mod);

	for (int i = 0; i < originalWs.Count(); i++)
	{
		Point3 p = TVMaps.v[i].GetP();
		p.z = originalWs[i];
		TVMaps.v[i].SetP(p);		
	}

	if (mod->peltData.peltDialog.hWnd && !applyToAll)
		mod->peltData.RelaxRig(frames,str,lockEdges,mod);


	for (int i = 0; i < TVMaps.v.Count(); i++)
	{
		float w = 0.0f;
		if (mVSelTemp[i] ) 
			w = 1.0f;
		else w = TVMaps.v[i].GetInfluence();
		if ((w > 0.0f) && (!(TVMaps.v[i].IsFrozen())))
		{	
			Point3 p = TVMaps.v[i].GetP();
			SetTVVert(t,i,p,mod);//if (TVMaps.cont[i]) TVMaps.cont[i]->SetValue(t,&TVMaps.v[i].p);
				
		}
	}


	//delete the clusters
	for (int i = 0; i < clusterInfo.Count(); i++)
	{
//if cluster
		if (clusterInfo[i])
			delete clusterInfo[i];
	}


	TransferSelectionEnd(subobjectMode,FALSE,FALSE); 

}

void  MeshTopoData::RelaxBySprings(int subobjectMode,int frames, float stretch, BOOL lockEdges, UnwrapMod *mod)
{
	float reducer = 1.0f;

	TransferSelectionStart(subobjectMode); 

	//find our boundary edges
	BitArray edgeSprings;
	BitArray springs;
	springs.SetSize(TVMaps.ePtrList.Count());
	springs.ClearAll();
	edgeSprings.SetSize(TVMaps.ePtrList.Count());
	edgeSprings.ClearAll();

	BitArray edgeVerts;
	edgeVerts.SetSize(TVMaps.v.Count());
	edgeVerts.ClearAll();



	BitArray springVerts;
	springVerts.SetSize(TVMaps.v.Count());
	springVerts.ClearAll();

	float uvEdgeDist = 0.0f;
	float geomEdgeDist = 0.0f;

	TimeValue t = GetCOREInterface()->GetTime();

	for (int i = 0; i < TVMaps.v.Count(); i++)
	{

		float wa = 0.0f;
		if (mVSel[i]) wa = 1.0f;
		else wa = TVMaps.v[i].GetInfluence();
		if (wa > 0.0f)
		{
			Point3 p = TVMaps.v[i].GetP();		
			p.z = 0.0f;
			SetTVVert(t,i,p,mod);//TVMaps.v[i].SetP(p);
			//if (TVMaps.cont[i]) TVMaps.cont[i]->SetValue(t,&TVMaps.v[i].p);
			springVerts.Set(i,TRUE);
		}
		
	}
	

	for (int i = 0; i < TVMaps.f.Count(); i++)
	{
		int deg = TVMaps.f[i]->count;
		int ct = 0;
		for (int j = 0; j < deg; j++)
		{
			int a = TVMaps.f[i]->t[j];
			float wa = 0.0f;
			if (mVSel[a]) wa = 1.0f;
			else wa = TVMaps.v[a].GetInfluence();
			if (wa > 0.0f) ct++;
		}
		if (ct != deg)
		{
			for (int j = 0; j < deg; j++)
			{
				int a = TVMaps.f[i]->t[j];

				float wa = 0.0f;
				if (mVSel[a]) wa = 1.0f;
				else wa = TVMaps.v[a].GetInfluence();
				if (wa > 0.0f)
					edgeVerts.Set(a,TRUE);
			}

		}
	}

	for (int i = 0; i < TVMaps.ePtrList.Count(); i++)
	{
		int a = TVMaps.ePtrList[i]->a;
		int b = TVMaps.ePtrList[i]->b;

		if (TVMaps.ePtrList[i]->faceList.Count() == 1)
		{
			edgeVerts.Set(a,TRUE);
			edgeVerts.Set(b,TRUE);
		}
	}

	int ct = 0;
	for (int i = 0; i < TVMaps.ePtrList.Count(); i++)
	{
		int a = TVMaps.ePtrList[i]->a;
		int b = TVMaps.ePtrList[i]->b;
		int ga = TVMaps.ePtrList[i]->ga;
		int gb = TVMaps.ePtrList[i]->gb;

		float wa = 0.0f;
		float wb = 0.0f;

		if (mVSel[a]) wa = 1.0f;
		else wa = TVMaps.v[a].GetInfluence();
		if (mVSel[b]) wb = 1.0f;
		else wb = TVMaps.v[b].GetInfluence();
		if ((wa > 0.0f) && (wb > 0.0f))
		{
			if (lockEdges)
			{
				if (edgeVerts[a] && edgeVerts[b])
				{

				}
				else if ((!edgeVerts[a]) && (!edgeVerts[b]))
				{
					springs.Set(i,TRUE);
					uvEdgeDist += Length(TVMaps.v[a].GetP() - TVMaps.v[b].GetP() );
					geomEdgeDist += Length(TVMaps.geomPoints[ga]-TVMaps.geomPoints[gb]);
					ct++;
				}
				else if ( ((edgeVerts[a]) && (!edgeVerts[b])) ||
					      ((edgeVerts[b]) && (!edgeVerts[a])) )
				{
					springs.Set(i,TRUE);
					edgeSprings.Set(i,TRUE);
					uvEdgeDist += Length(TVMaps.v[a].GetP() - TVMaps.v[b].GetP() );
					geomEdgeDist += Length(TVMaps.geomPoints[ga]-TVMaps.geomPoints[gb]);
					ct++;
				}

			}
			else
			{
  				if (edgeVerts[a] && edgeVerts[b])
				{
					springs.Set(i,TRUE);
					uvEdgeDist += Length(TVMaps.v[a].GetP() - TVMaps.v[b].GetP() );
					geomEdgeDist += Length(TVMaps.geomPoints[ga]-TVMaps.geomPoints[gb]);
					ct++;
				}
				else if ((!edgeVerts[a]) && (!edgeVerts[b]))
				{
					springs.Set(i,TRUE);
					uvEdgeDist += Length(TVMaps.v[a].GetP() - TVMaps.v[b].GetP() );
					geomEdgeDist += Length(TVMaps.geomPoints[ga]-TVMaps.geomPoints[gb]);
					ct++;
				}
				else if ( ((edgeVerts[a]) && (!edgeVerts[b])) ||
					      ((edgeVerts[b]) && (!edgeVerts[a])) )
				{
					springs.Set(i,TRUE);
					uvEdgeDist += Length(TVMaps.v[a].GetP() - TVMaps.v[b].GetP() );
					geomEdgeDist += Length(TVMaps.geomPoints[ga]-TVMaps.geomPoints[gb]);
					ct++;
				}
			}
		}

	}

   	uvEdgeDist = uvEdgeDist/(ct);
	geomEdgeDist = geomEdgeDist/(ct);
	float edgeScale = (uvEdgeDist/geomEdgeDist) *reducer;






	//get the normals at each vertex
	
	Tab<Point3> geomFaceNormals;
	geomFaceNormals.SetCount(TVMaps.f.Count());
	for (int i =0; i < TVMaps.f.Count(); i++)
	{
		geomFaceNormals[i] = TVMaps.GeomFaceNormal(i);
	}

	Tab<Point3> geomVertexNormals;
	Tab<int> geomVertexNormalsCT;
	geomVertexNormals.SetCount(TVMaps.geomPoints.Count());
	geomVertexNormalsCT.SetCount(TVMaps.geomPoints.Count());

	for (int i =0; i < TVMaps.geomPoints.Count(); i++)
	{
		geomVertexNormals[i] = Point3(0.0f,0.0f,0.0f);
		geomVertexNormalsCT[i] = 0;
	}
	for (int i =0; i < TVMaps.f.Count(); i++)
	{
		int deg = TVMaps.f[i]->count;
		for (int j = 0; j < deg; j++)
		{
			int index = TVMaps.f[i]->v[j];
			geomVertexNormalsCT[index] = geomVertexNormalsCT[index] + 1;
			geomVertexNormals[index] += geomFaceNormals[i];
		}
	}

	for (int i =0; i < TVMaps.geomPoints.Count(); i++)
	{
		if (geomVertexNormalsCT[i] != 0)
			geomVertexNormals[i] = geomVertexNormals[i]/(float)geomVertexNormalsCT[i];
		geomVertexNormals[i] = Normalize(geomVertexNormals[i]);

	}

	Tab<EdgeBondage> springEdges;
	Tab<AdjacentItem*> facesAtVert;
 	TVMaps.BuildAdjacentUVFacesToVerts(facesAtVert);

	//loop through our verts
	for (int i = 0; i < TVMaps.v.Count(); i++)
	{
	//get the faces attached to this vert
		if (springVerts[i])
		{
			Tab<int> connectedFaces;
			connectedFaces.SetCount(facesAtVert[i]->index.Count());
			for (int j = 0; j < facesAtVert[i]->index.Count(); j++)
			{
				connectedFaces[j] = facesAtVert[i]->index[j];
			}
		//get the uv/geom verts attached to this vert
			Tab<int> connectedUVVerts;
			Tab<int> connectedGeomVerts;
			int centerGIndex = -1;
			for (int j = 0; j < connectedFaces.Count(); j++)
			{
				
				int faceIndex = connectedFaces[j];
				int deg = TVMaps.f[faceIndex]->count;
				
				for (int k = 0; k < deg; k++)
				{
					BOOL visibleEdge = TRUE;
					if (deg == 3)
					{
						if (k == 0)
							visibleEdge = (!(TVMaps.f[faceIndex]->flags & FLAG_HIDDENEDGEA));
						if (k == 1)
							visibleEdge = (!(TVMaps.f[faceIndex]->flags & FLAG_HIDDENEDGEB));
						if (k == 2)
							visibleEdge = (!(TVMaps.f[faceIndex]->flags & FLAG_HIDDENEDGEC));
					}
					if (visibleEdge)
					{
						int a = TVMaps.f[faceIndex]->t[k];
						int b = TVMaps.f[faceIndex]->t[(k+1)%deg];

						int ga = TVMaps.f[faceIndex]->v[k];
						int gb = TVMaps.f[faceIndex]->v[(k+1)%deg];

						if (a == i)
						{
							connectedUVVerts.Append(1,&b,5);
							connectedGeomVerts.Append(1,&gb,5);
							centerGIndex = ga;
						}
						if (b == i)
						{
							connectedUVVerts.Append(1,&a,5);
							connectedGeomVerts.Append(1,&ga,5);
							centerGIndex = gb;
						}
					}

				}
			}

		//remove the duplicates
	 		BitArray processedVerts;
			processedVerts.SetSize(TVMaps.v.Count());
			processedVerts.ClearAll();

			Tab<int> cUVVerts;
			Tab<int> cGeomVerts;
			for (int j = 0; j < connectedUVVerts.Count(); j++)
			{
				int index = connectedUVVerts[j];
				int gindex = connectedGeomVerts[j];
				if (!processedVerts[index])
				{
					cUVVerts.Append(1,&index,5);
					cGeomVerts.Append(1,&gindex,5);
					processedVerts.Set(index,TRUE);
				}
			}

			Point3 center = TVMaps.geomPoints[centerGIndex];
			Tab<Point3> edgePoints;
			edgePoints.SetCount(cGeomVerts.Count());

			for (int j = 0; j < cGeomVerts.Count(); j++)
			{
				int uindex = cUVVerts[j];

				int index = cGeomVerts[j];
				edgePoints[j] = TVMaps.geomPoints[index];
			}
		//flatten the verts 
			Point3 norm = geomVertexNormals[centerGIndex];

			for (int j = 0; j < cGeomVerts.Count(); j++)
			{
				Point3 vec = edgePoints[j] - center;
				float l = Length(vec);
				vec = Normalize(vec);
				Point3 axis = Normalize(CrossProd(norm,vec));


				Quat q = QFromAngAxis(PI*0.5f,axis);
				Matrix3 tm(1);
				
				q.MakeMatrix(tm);
				Point3 ep = norm * tm;

				edgePoints[j] = vec * l;
			}

		//build the springs


		//do the edge to edge spings
			for (int j = 0; j < cGeomVerts.Count(); j++)
			{
				int a = cUVVerts[j];
				int crossIndex = (j+1)%cGeomVerts.Count();
				int b = cUVVerts[crossIndex];
				int ga = cGeomVerts[j];
				int gb = cGeomVerts[crossIndex];
				float d = Length(edgePoints[j] - edgePoints[crossIndex]);
				d = d * edgeScale;

 				EdgeBondage sp;
				sp.v1 = a;
				sp.v2 = b;
				sp.vec1 = -1;
				sp.vec2 = -1;
				sp.dist = d;
				sp.str = 0.9f;
				sp.distPer = 1.0f;
				sp.isEdge = FALSE;
				sp.edgeIndex = -1;
				springEdges.Append(1,&sp,5000);
			}
		//do the cross springs
 			if (cGeomVerts.Count()> 3)
			{
				for (int j = 0; j < cGeomVerts.Count(); j++)
				{
					int a = cUVVerts[j];
					int crossIndex = (j+(cGeomVerts.Count()/2))%cGeomVerts.Count();
					int b = cUVVerts[crossIndex];
					int ga = cGeomVerts[j];
					int gb = cGeomVerts[crossIndex];
					float d = Length(edgePoints[j] - edgePoints[crossIndex]);
					d = d * edgeScale;


 					EdgeBondage sp;
					sp.v1 = a;
					sp.v2 = b;
					sp.vec1 = -1;
					sp.vec2 = -1;
					sp.dist = d;
					sp.str =0.90f;

					sp.distPer = 1.0f;
					sp.isEdge = FALSE;
					sp.edgeIndex = -1;
					springEdges.Append(1,&sp,5000);
				}
			}

		}

	}
  	for (int i = 0; i < TVMaps.ePtrList.Count(); i++)
	{
			//loop the through the edges
		int a = TVMaps.ePtrList[i]->a;
		int b = TVMaps.ePtrList[i]->b;
		if (springVerts[a]&&springVerts[b])
		{
			int ga = TVMaps.ePtrList[i]->ga;
			int gb = TVMaps.ePtrList[i]->gb;

			int veca = TVMaps.ePtrList[i]->avec;
			int vecb = TVMaps.ePtrList[i]->bvec;
			BOOL isHidden = TVMaps.ePtrList[i]->flags & FLAG_HIDDENEDGEA;




			EdgeBondage sp;
			sp.v1 = a;
			sp.v2 = b;
			sp.vec1 = veca;
			sp.vec2 = vecb;
			float dist = Length(TVMaps.geomPoints[ga]-TVMaps.geomPoints[gb]);
			dist = dist * edgeScale;
			sp.dist = dist;
			sp.str = 1.0f;

			sp.distPer = 1.0f;
			sp.isEdge = FALSE;
			sp.edgeIndex = i;
			springEdges.Append(1,&sp,5000);

			if ((isHidden) && (TVMaps.ePtrList[i]->faceList.Count() > 1))
			{
				//get face 1
				int a1,b1;
				a1 = -1;
				b1 = -1;
				int ga1 = 0, gb1 = 0;

				int faceIndex = TVMaps.ePtrList[i]->faceList[0];

				int deg = TVMaps.f[faceIndex]->count;
				for (int j = 0; j < deg; j++)
				{
					int tvA = TVMaps.f[faceIndex]->t[j];
					if ((tvA != a) && (tvA != b))
					{
						a1 = tvA;
						ga1 = TVMaps.f[faceIndex]->v[j];
					}

				}

				//get face 2
				faceIndex = TVMaps.ePtrList[i]->faceList[1];
				deg = TVMaps.f[faceIndex]->count;
				for (int j = 0; j < deg; j++)
				{
					int tvA = TVMaps.f[faceIndex]->t[j];
					if ((tvA != a) && (tvA != b))
					{
						b1 = tvA;
						gb1 = TVMaps.f[faceIndex]->v[j];
					}
				}

				if ((a1 != -1) && (b1 != -1))
				{

					if (edgeVerts[a1])
					{
						int ta = a1;
						a1 = b1;
						b1 = ta;

						ta = ga1;
						ga1 = gb1;
						gb1 = ta;


					}


 					EdgeBondage sp;
					sp.v1 = a1;
					sp.v2 = b1;
					sp.vec1 = -1;
					sp.vec2 = -1;
					float dist = Length(TVMaps.geomPoints[ga1]-TVMaps.geomPoints[gb1]);
					sp.dist = dist * edgeScale;
					sp.str = 1.0f;
//					if (lockEdges)
//						sp.str = 0.0f;

					sp.distPer = 1.0f;

					float wa = 0.0f;
					float wb = 0.0f;

					if (mVSel[a]) wa = 1.0f;
					else wa = TVMaps.v[a].GetInfluence();
					if (mVSel[b]) wb = 1.0f;
					else wb = TVMaps.v[b].GetInfluence();

					sp.isEdge = FALSE;


					sp.edgeIndex = -1;
					springEdges.Append(1,&sp,5000);
				}
			}
			
		}
	}



	for (int i = 0; i < facesAtVert.Count(); i++)
	{
		delete facesAtVert[i];
	}


	Solver solver;

	
	Tab<SpringClass> verts;  //this is a list off all our verts, there positions and velocities

	verts.SetCount(TVMaps.v.Count());
	for (int i = 0; i < TVMaps.v.Count(); i++)
	{
		verts[i].pos = TVMaps.v[i].GetP();
		verts[i].vel = Point3(0.0f,0.0f,0.0f);

		float wa = 0.0f;
		if (mVSel[i]) wa = 1.0f;
		else wa = TVMaps.v[i].GetInfluence();
		verts[i].weight = wa;
	}

	//build our distance from edge

	BitArray processedVerts;
	processedVerts = edgeVerts;
	int distLevel = 0;

/*
	Tab<float> edgeWeight;
	edgeWeight.SetCount(TVMaps.ePtrList.Count());
  	for (int i = 0; i < edgeWeight.Count(); i++)
		edgeWeight[i] = -1.0f;
*/
//	dverts = mVSel;
//	dverts.ClearAll();

    for (int i = 0; i < springEdges.Count(); i++)
	{
		if (!lockEdges)
			springEdges[i].isEdge = FALSE;
		else
		{
			int a = springEdges[i].v1;
			int b = springEdges[i].v2;
			//look for an egde with 2 edge verts delete it
			if (edgeVerts[a] && edgeVerts[b])
			{
				springEdges.Delete(i,1);
				i--;
			}
			else if ((!edgeVerts[a]) && edgeVerts[b])
			{
				springEdges[i].isEdge = TRUE;
//				dverts.Set(a);
			}
			else if ((!edgeVerts[b]) && edgeVerts[a])
			{
//				dverts.Set(b);
				springEdges[i].isEdge = TRUE;
				int temp;
				
				temp = springEdges[i].v1;
				springEdges[i].v1 = springEdges[i].v2;
				springEdges[i].v2 = temp;
			//look for one
			//swap a and b if need be
			}
		}
	}



//	solver.Solve(0, frames, 5,
//		springEdges, verts,
//		0.16f,0.16f,0.25f,this);

// 	dspringEdges = springEdges;

   	for (int k = 0; k < frames; k++)
	{
		BOOL iret = solver.Solve(0, frames, 2,
			springEdges, verts,
 			0.001f,0.001f,0.25f,mod);

		if (iret == FALSE)
			k = frames;


		for (int j = 0; j < springEdges.Count(); j++)
		{
			float d = springEdges[j].dist;
			int a = springEdges[j].v1;
			int b = springEdges[j].v2;
			if (edgeVerts[a] && edgeVerts[b])
			{
			}
			else
			{
				
				springEdges[j].dist *= stretch; 
			}
		}
	}


	//reduce our spring lengths and repeat

	//put the data back

	UVW_ChannelClass *tvData = &TVMaps;
	for (int j = 0; j < verts.Count(); j++)
	{
		Point3 p = verts[j].pos	;		
		SetTVVert(t,j,p,mod);
//		tvData->v[j].p = p;
//		if (tvData->cont[j]) tvData->cont[j]->SetValue(t,&tvData->v[j].p);

	}
	
	mod->peltData.ResolvePatchHandles(mod);

	TransferSelectionEnd(subobjectMode,FALSE,FALSE); 


}