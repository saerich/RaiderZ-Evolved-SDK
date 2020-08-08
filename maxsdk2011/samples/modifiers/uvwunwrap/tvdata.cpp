#include "unwrap.h"

int UVW_TVFaceClass::GetGeoVertIDFromUVVertID(int uvID)
{
	for (int i = 0; i < count; i++)
	{
		int a = t[i];
		if (a == uvID)
			return v[i];
	}
	return -1;

}

void UVW_TVFaceClass::GetConnectedUVVerts(int id, int &v1, int &v2)
{
	for (int i = 0; i < count; i++)
	{
		int a = t[i];
		if (a == id)
		{
			int next = (i + 1);
			int prev = (i - 1);

			if (next >= count) next = 0;
			if (prev < 0) prev = count-1;
			v1 = t[prev];
			v2 = t[next];
			i = count;
		}
	}
}

void UVW_TVFaceClass::GetConnectedGeoVerts(int id, int &v1, int &v2)
{
	for (int i = 0; i < count; i++)
	{
		int a = v[i];
		if (a == id)
		{
			int next = (i + 1);
			int prev = (i - 1);

			if (next >= count) next = 0;
			if (prev < 0) prev = count-1;
			v1 = v[prev];
			v2 = v[next];
			i = count;
		}
	}
}


UVW_TVFaceClass* UVW_TVFaceClass::Clone()
	{
	UVW_TVFaceClass *f = new UVW_TVFaceClass;
	f->FaceIndex = FaceIndex;
	f->MatID = MatID;
	f->flags = flags;
	f->count = count;
	f->t = new int[count];
	f->v = new int[count];
	for (int i =0; i < count; i++)
		{
		f->t[i] = t[i];
		f->v[i] = v[i];
		}
	f->vecs = NULL;
	if (vecs)
		{
		UVW_TVVectorClass *vect = new UVW_TVVectorClass;
		f->vecs = vect;

		for (int i =0; i < count; i++)
			{
			vect->interiors[i] =  vecs->interiors[i];
			vect->handles[i*2] =  vecs->handles[i*2];
			vect->handles[i*2+1] =  vecs->handles[i*2+1];

			vect->vinteriors[i] =  vecs->vinteriors[i];
			vect->vhandles[i*2] =  vecs->vhandles[i*2];
			vect->vhandles[i*2+1] =  vecs->vhandles[i*2+1];
			}
		}
	return f;
	}

void UVW_TVFaceClass::DeleteVec()
	{
	if (vecs) delete vecs;
	vecs = NULL;
	}

ULONG UVW_TVFaceClass::SaveFace(ISave *isave)
	{
	ULONG nb = 0;
	isave->Write(&count, sizeof(int), &nb);
	isave->Write(t, sizeof(int)*count, &nb);
	isave->Write(&FaceIndex, sizeof(int), &nb);
	isave->Write(&MatID, sizeof(int), &nb);
	isave->Write(&flags, sizeof(int), &nb);
	isave->Write(v, sizeof(int)*count, &nb);
	if ( (vecs) && (flags & FLAG_CURVEDMAPPING))
		{
		isave->Write(vecs->handles, sizeof(int)*count*2, &nb);
		isave->Write(vecs->interiors, sizeof(int)*count, &nb);
		isave->Write(vecs->vhandles, sizeof(int)*count*2, &nb);
		isave->Write(vecs->vinteriors, sizeof(int)*count, &nb);

		}
	return nb;
	}

ULONG UVW_TVFaceClass::SaveFace(FILE *file)
	{
	ULONG nb = 1;
	fwrite(&count, sizeof(int), 1,file);
	fwrite(t, sizeof(int)*count, 1,file);
	fwrite(&FaceIndex, sizeof(int), 1,file);
	fwrite(&MatID, sizeof(int), 1,file);
	fwrite(&flags, sizeof(int), 1,file);
	fwrite(v, sizeof(int)*count, 1,file);
	if ( (vecs) && (flags & FLAG_CURVEDMAPPING))
		{
		fwrite(vecs->handles, sizeof(int)*count*2, 1,file);
		fwrite(vecs->interiors, sizeof(int)*count, 1,file);
		fwrite(vecs->vhandles, sizeof(int)*count*2, 1,file);
		fwrite(vecs->vinteriors, sizeof(int)*count, 1,file);
		}
	return nb;
	}


ULONG UVW_TVFaceClass::LoadFace(ILoad *iload)
	{
	ULONG nb = 0;
	iload->Read(&count, sizeof(int), &nb);

	if (t==NULL)
		t = new int[count];
	else
		{
		delete [] t;
		t = new int[count];
		}

	if (v==NULL)
		v = new int[count];
	else
		{
		delete [] v;
		v = new int[count];
		}


	iload->Read(t, sizeof(int)*count, &nb);
	iload->Read(&FaceIndex, sizeof(int), &nb);
	iload->Read(&MatID, sizeof(int), &nb);
	iload->Read(&flags, sizeof(int), &nb);
	iload->Read(v, sizeof(int)*count, &nb);
	vecs = NULL;

	if (flags & FLAG_CURVEDMAPPING)
		{
		vecs = new UVW_TVVectorClass;
		iload->Read(vecs->handles, sizeof(int)*count*2, &nb);
		iload->Read(vecs->interiors, sizeof(int)*count, &nb);
		iload->Read(vecs->vhandles, sizeof(int)*count*2, &nb);
		iload->Read(vecs->vinteriors, sizeof(int)*count, &nb);
		}
	return nb;
	}

ULONG UVW_TVFaceClass::LoadFace(FILE *file)
	{
	ULONG nb = 0;
	fread(&count, sizeof(int), 1, file);

	if (t==NULL)
		t = new int[count];
	else
		{
		delete [] t;
		t = new int[count];
		}

	if (v==NULL)
		v = new int[count];
	else
		{
		delete [] v;
		v = new int[count];
		}

	fread(t, sizeof(int)*count, 1, file);
	fread(&FaceIndex, sizeof(int), 1, file);
	fread(&MatID, sizeof(int), 1, file);
	fread(&flags, sizeof(int), 1, file);
	fread(v, sizeof(int)*count, 1, file);
	vecs = NULL;

	if (flags & FLAG_CURVEDMAPPING)
		{
		vecs = new UVW_TVVectorClass;
		fread(vecs->handles, sizeof(int)*count*2, 1, file);
		fread(vecs->interiors, sizeof(int)*count, 1, file);
		fread(vecs->vhandles, sizeof(int)*count*2, 1, file);
		fread(vecs->vinteriors, sizeof(int)*count, 1, file);
		}
	return nb;
	}


int UVW_TVFaceClass::FindGeomEdge(int a, int b)
{
	for (int i = 0; i < count; i++)
	{
		int fa = v[i];
		int fb = v[(i+1)%count];
		if ( (fa == a) && (fb == b) )
			return i;
		if ( (fa == b) && (fb == a) )
			return i;

	}
	return -1;
}
int UVW_TVFaceClass::FindUVEdge(int a, int b)
{
	for (int i = 0; i < count; i++)
	{
		int fa = t[i];
		int fb = t[(i+1)%count];
		if ( (fa == a) && (fb == b) )
			return i;
		if ( (fa == b) && (fb == a) )
			return i;

	}
	return -1;
}

ULONG UVW_ChannelClass::LoadFacesMax9(ILoad *iload)
	{
	ULONG nb = 0;
	for (int i=0; i < f.Count(); i++)
		{
		nb = f[i]->LoadFace(iload);
		}
	return nb;
	}


ULONG UVW_ChannelClass::LoadFaces(ILoad *iload)
	{
	ULONG nb = 0;
	int ct = 0;
	iload->Read(&ct, sizeof(ct), &nb);
	SetCountFaces(ct);

	for (int i=0; i < f.Count(); i++)
		{
		nb = f[i]->LoadFace(iload);
		}
	return nb;
	}

ULONG UVW_ChannelClass::LoadFaces(FILE *file)
	{
	ULONG nb = 0;
	for (int i=0; i < f.Count(); i++)
		{
		nb = f[i]->LoadFace(file);
		}
	return nb;
	}

ULONG UVW_ChannelClass::SaveFacesMax9(ISave *isave)
	{
	ULONG nb = 0;

	for (int i=0; i < f.Count(); i++)
		{
		nb = f[i]->SaveFace(isave);
		}
	return nb;
	}

ULONG UVW_ChannelClass::SaveFaces(ISave *isave)
	{
	ULONG nb;
	int fct = f.Count();
	isave->Write(&fct, sizeof(fct), &nb);
	for (int i=0; i < f.Count(); i++)
		{
		nb = f[i]->SaveFace(isave);
		}
	return nb;
	}

ULONG UVW_ChannelClass::SaveFaces(FILE *file)
	{
	ULONG nb = 0;
	for (int i=0; i < f.Count(); i++)
		{
		nb = f[i]->SaveFace(file);
		}
	return nb;
	}


void UVW_ChannelClass::SetCountFaces(int newct)
	{
//delete existing data
	int ct = f.Count();
	for (int i =0; i < ct; i++)
		{
		if (f[i]->vecs) delete f[i]->vecs;
		if (f[i]->t) delete [] f[i]->t;
		if (f[i]->v) delete [] f[i]->v;
		f[i]->vecs = NULL;
		f[i]->t = NULL;
		f[i]->v = NULL;

		delete f[i];
		f[i] = NULL;
		}

	f.SetCount(newct);
	for (int i =0; i < newct; i++)
		{
		f[i] = new UVW_TVFaceClass;
		f[i]->vecs = NULL;
		f[i]->t = NULL;
		f[i]->v = NULL;
		}


	}

void UVW_ChannelClass::CloneFaces(Tab<UVW_TVFaceClass*> &t)
	{
	int ct = f.Count();
	t.SetCount(ct);
	for (int i =0; i < ct; i++)
		t[i] = f[i]->Clone();
	}

void UVW_ChannelClass::AssignFaces(Tab<UVW_TVFaceClass*> &t)
	{
	//nuke old data and cassign clone of new
	int ct = f.Count();
	for (int i =0; i < ct; i++)
		{
		if (f[i]->vecs) delete f[i]->vecs;
		if (f[i]->t) delete [] f[i]->t;
		if (f[i]->v) delete [] f[i]->v;
		f[i]->vecs = NULL;
		f[i]->t = NULL;
		f[i]->v = NULL;

		delete f[i];
		f[i] = NULL;
		}
	ct = t.Count();
	f.SetCount(ct);
	for (int i =0; i < ct; i++)
		f[i] = t[i]->Clone();
	}

void UVW_ChannelClass::FreeFaces()
	{
	int ct = f.Count();
	for (int i =0; i < ct; i++)
		{
		if (f[i]->vecs) delete f[i]->vecs;
		if (f[i]->t) delete [] f[i]->t;
		if (f[i]->v) delete [] f[i]->v;
		f[i]->vecs = NULL;
		f[i]->t = NULL;
		f[i]->v = NULL;

		delete f[i];
		f[i] = NULL;
		}

	}

void UVW_ChannelClass::Dump()
	{
	for (int i = 0; i < v.Count(); i++)
		{
		DebugPrint(_T("Vert %d pt %f %f %f  Dead %d\n"),i,v[i].GetP().x,v[i].GetP().y,v[i].GetP().z,v[i].IsDead());
		}
	for (int i = 0; i < f.Count(); i++)
		{
		DebugPrint(_T("face %d t %d %d %d %d\n"),i,f[i]->t[0],f[i]->t[1],f[i]->t[2],f[i]->t[3]);

		if (f[i]->vecs)
			DebugPrint(_T("     int  %d %d %d %d\n"),f[i]->vecs->interiors[0],f[i]->vecs->interiors[1],
										  f[i]->vecs->interiors[2],f[i]->vecs->interiors[3]
				   );
		
		if (f[i]->vecs)
			DebugPrint(_T("     vec  %d,%d %d,%d %d,%d %d,%d\n"),
										  f[i]->vecs->handles[0],f[i]->vecs->handles[1],
										  f[i]->vecs->handles[2],f[i]->vecs->handles[3],
										  f[i]->vecs->handles[4],f[i]->vecs->handles[5],
										  f[i]->vecs->handles[6],f[i]->vecs->handles[7]
				   );
		}
	}	

void UVW_ChannelClass::MarkDeadVertices()
	{
	BitArray usedVerts;
	usedVerts.SetSize(v.Count());
	usedVerts.ClearAll();

	for (int i =0; i < f.Count(); i++)
		{
		if (!(f[i]->flags & FLAG_DEAD))
			{
			for (int j=0; j < f[i]->count; j++)
				{
				int id = f[i]->t[j];
				if (id < usedVerts.GetSize()) usedVerts.Set(id);
				if ((f[i]->flags & FLAG_CURVEDMAPPING) && (f[i]->vecs))
					{
					id = f[i]->vecs->handles[j*2];
					if (id < usedVerts.GetSize()) usedVerts.Set(id);
					id = f[i]->vecs->handles[j*2+1];
					if (id < usedVerts.GetSize()) usedVerts.Set(id);
					if (f[i]->flags & FLAG_INTERIOR)
						{
						id = f[i]->vecs->interiors[j];
						if (id < usedVerts.GetSize()) usedVerts.Set(id);
						}
					}
	
				}
			}
		}

	for (int i =0; i < v.Count(); i++)
		{
		if (i < usedVerts.GetSize())
			{
			BOOL isRigPoint = v[i].GetFlag() & FLAG_RIGPOINT;
			if (!usedVerts[i] && (!isRigPoint))
				{
				v[i].SetDead();
				}
			}
		
		}
	

	}






void VertexLookUpListClass::addPoint(int a_index, Point3 a)
	{	
	BOOL found = FALSE;
	if (sel[a_index]) found = TRUE;
	if (!found)
		{
		VertexLookUpDataClass t;
		t.index = a_index;
		t.newindex = a_index;
		t.p = a;
		d[a_index] = t;
		sel.Set(a_index);
		}
	};


UVW_ChannelClass::UVW_ChannelClass()
	{
		edgeScale = 1.0f;
		mGeoEdgesValid = TRUE;
		edgesValid = TRUE;
	}

UVW_ChannelClass::~UVW_ChannelClass()
{
	FreeEdges();
	FreeFaces();
	FreeGeomEdges();
}

void UVW_ChannelClass::FreeEdges()
	{
	for (int i =0; i < e.Count(); i++)
	{
		for (int j =0; j < e[i]->data.Count(); j++)
			delete e[i]->data[j];
		delete e[i];
		}
	e.ZeroCount();
	ePtrList.ZeroCount();
	}

void UVW_ChannelClass::BuildEdges()
	{
	FreeEdges();
	if (v.Count() != 0)
		edgesValid = TRUE;
	e.SetCount(v.Count());
	


	for (int i = 0; i < v.Count();i++)
		{
		e[i] = new UVW_TVEdgeClass();
		}

	for (int i = 0; i < f.Count();i++)
		{
		if (!(f[i]->flags & FLAG_DEAD))
			{
			int pcount = 3;
			pcount = f[i]->count;
			for (int k = 0; k < pcount; k++)
				{
				int gv1 = f[i]->v[k];
				int gv2 = f[i]->v[(k+1)%pcount];
				int index1 = f[i]->t[k];
 
				int index2 = 0;
				if (k == (pcount-1))
					index2 = f[i]->t[0];
				else index2 = f[i]->t[k+1];
				int vec1=-1, vec2=-1;
				if ((f[i]->flags & FLAG_CURVEDMAPPING) &&(f[i]->vecs) )
					{
					vec1 = f[i]->vecs->handles[k*2];

					vec2 = f[i]->vecs->handles[k*2+1];
					}
				if (index2 < index1) 
					{
					int temp = index1;
					index1 = index2;
					index2 = temp;
					temp = vec1;
					vec1 = vec2;
					vec2 = temp;
					temp = gv1;
					gv1 = gv2;
					gv2 = temp;
					}
				BOOL hidden = FALSE;
				if (k==0)
					{
					if (f[i]->flags & FLAG_HIDDENEDGEA)
						hidden = TRUE;
					}
				else if (k==1)
					{
					if (f[i]->flags & FLAG_HIDDENEDGEB)
						hidden = TRUE;
					}
				else if (k==2)
					{
					if (f[i]->flags & FLAG_HIDDENEDGEC)
						hidden = TRUE;

					}
				AppendEdge(e,index1,vec1,index2,vec2,i,hidden,gv1,gv2);
				}	

			}
		}
	int ePtrCount = 0;
	for (int i =0; i < e.Count(); i++)
		{
		ePtrCount += e[i]->data.Count();
		}
	ePtrList.SetCount(ePtrCount);
	int ct = 0;
	for (int i =0; i < e.Count(); i++)
		{
		for (int j =0; j < e[i]->data.Count(); j++)
			ePtrList[ct++] = e[i]->data[j];
		}
//PELT
	for (int i = 0; i < ePtrList.Count(); i++)
	{
		ePtrList[i]->lookupIndex = i;
	}
	for (int i = 0; i < gePtrList.Count(); i++)
	{
		gePtrList[i]->lookupIndex = i;
	}

	MarkDeadVertices();

}

//PELT
void UVW_ChannelClass::FreeGeomEdges()
{
	for (int i =0; i < ge.Count(); i++)
	{
		for (int j =0; j < ge[i]->data.Count(); j++)
			delete ge[i]->data[j];
		delete ge[i];
	}
	ge.ZeroCount();
	gePtrList.ZeroCount();
}

void UVW_ChannelClass::BuildGeomEdges()
{
	FreeGeomEdges();
	
	mGeoEdgesValid = TRUE;
	ge.SetCount(geomPoints.Count());



	for (int i = 0; i < geomPoints.Count();i++)
	{
		ge[i] = new UVW_TVEdgeClass();
	}

	for (int i = 0; i < f.Count();i++)
	{
		if (!(f[i]->flags & FLAG_DEAD))
		{
			int pcount = 3;
			pcount = f[i]->count;
			for (int k = 0; k < pcount; k++)
			{

				int index1 = f[i]->v[k];

				int index2 = 0;
				if (k == (pcount-1))
					index2 = f[i]->v[0];
				else index2 = f[i]->v[k+1];
				int vec1=-1, vec2=-1;
				if ((f[i]->flags & FLAG_CURVEDMAPPING) &&(f[i]->vecs) )
				{
					vec1 = f[i]->vecs->vhandles[k*2];

					vec2 = f[i]->vecs->vhandles[k*2+1];
				}
				if (index2 < index1) 
				{
					int temp = index1;
					index1 = index2;
					index2 = temp;
					temp = vec1;
					vec1 = vec2;
					vec2 = temp;
				}
				BOOL hidden = FALSE;
				if (k==0)
				{
					if (f[i]->flags & FLAG_HIDDENEDGEA)
						hidden = TRUE;
				}
				else if (k==1)
				{
					if (f[i]->flags & FLAG_HIDDENEDGEB)
						hidden = TRUE;
				}
				else if (k==2)
				{
					if (f[i]->flags & FLAG_HIDDENEDGEC)
						hidden = TRUE;

	}
				AppendEdge(ge,index1,vec1,index2,vec2,i,hidden);
			}	

		}
	}
	int ePtrCount = 0;
	for (int i =0; i < ge.Count(); i++)
	{
		ePtrCount += ge[i]->data.Count();
	}
	gePtrList.SetCount(ePtrCount);
	int ct = 0;
	for (int i =0; i < ge.Count(); i++)
	{
		for (int j =0; j < ge[i]->data.Count(); j++)
			gePtrList[ct++] = ge[i]->data[j];
	}

	for (int i = 0; i < gePtrList.Count(); i++)
	{
		gePtrList[i]->lookupIndex = i;
	}

}


void UVW_ChannelClass::AppendEdge(Tab<UVW_TVEdgeClass*> &elist,int index1,int vec1, int index2,int vec2, int face, BOOL hidden, int gva, int gvb)
{
	if (elist[index1]->data.Count() == 0)
		{
		UVW_TVEdgeDataClass *temp = new UVW_TVEdgeDataClass();
		temp->a = index1;
		temp->avec = vec1;
		temp->b = index2;
		temp->bvec = vec2;
		temp->flags = 0;
		temp->ga = gva;
		temp->gb = gvb;
		if (hidden)
			temp->flags |= FLAG_HIDDENEDGEA;
		temp->faceList.Append(1,&face,4);
		elist[index1]->data.Append(1,&temp,4);
		}
	else
		{
		BOOL found = FALSE;
		for (int i =0; i < elist[index1]->data.Count(); i++)
			{
			if ( (elist[index1]->data[i]->b == index2) && (elist[index1]->data[i]->bvec == vec2))
				{
				found = TRUE;
				elist[index1]->data[i]->faceList.Append(1,&face,4);

				if ((!hidden) && (elist[index1]->data[i]->flags & FLAG_HIDDENEDGEA) )
					elist[index1]->data[i]->flags &= ~FLAG_HIDDENEDGEA;

				i = elist[index1]->data.Count();
				}
			}
		if (!found)
			{
			UVW_TVEdgeDataClass *temp = new UVW_TVEdgeDataClass();
			temp->a = index1;
			temp->avec = vec1;
			temp->b = index2;
			temp->bvec = vec2;
			temp->flags = 0;
			temp->ga = gva;
			temp->gb = gvb;
			if (hidden)
				temp->flags |= FLAG_HIDDENEDGEA;
			temp->faceList.Append(1,&face,4);
			elist[index1]->data.Append(1,&temp,4);
			}

		}

	}


float UVW_ChannelClass::LineToPoint(Point3 p1, Point3 l1, Point3 l2)
{
Point3 VectorA,VectorB,VectorC;
double Angle;
double dist = 0.0f;
VectorA = l2-l1;
VectorB = p1-l1;
float dot = DotProd(Normalize(VectorA),Normalize(VectorB));
if (dot >= 1.0f) 
	Angle = 0.0f;
else
	Angle =  acos(dot);
if (Angle > (3.14/2.0))
	{
	dist = Length(p1-l1);
	}
else
	{
	VectorA = l1-l2;
	VectorB = p1-l2;
	dot = DotProd(Normalize(VectorA),Normalize(VectorB));
	if (dot >= 1.0f) 
		Angle = 0.0f;
	else
		Angle = acos(dot);
	if (Angle > (3.14/2.0))
		{
		dist = Length(p1-l2);
		}
	else
		{
		double hyp;
		hyp = Length(VectorB);
		dist =  sin(Angle) * hyp;

		}

	}

return (float) dist;

}


int UVW_ChannelClass::EdgeIntersect(Point3 p, float threshold, int i1,int i2)
{

 static int startEdge = 0;
 if (startEdge >= ePtrList.Count()) startEdge = 0;
 if (ePtrList.Count() == 0) return -1;

 int ct = 0;
 BOOL done = FALSE;


 int hitEdge = -1;
 while (!done) 
	{
 //check bounding volumes

	Box3 bounds;
	bounds.Init();

	int index1 = ePtrList[startEdge]->a;
	int index2 = ePtrList[startEdge]->b;
	if (v[index1].IsHidden() && v[index2].IsHidden())
		{
		}
	else if (v[index1].IsFrozen() && v[index1].IsFrozen()) 
		{
		}
	else
		{
		Point3 p1(0.0f,0.0f,0.0f);
		p1[i1] = v[index1].GetP()[i1];
		p1[i2] = v[index1].GetP()[i2];
//		p1.z = 0.0f;
		bounds += p1;
		Point3 p2(0.0f,0.0f,0.0f);
		p2[i1] = v[index2].GetP()[i1];
		p2[i2] = v[index2].GetP()[i2];
//		p2.z = 0.0f;
		bounds += p2;
		bounds.EnlargeBy(threshold);
		if (bounds.Contains(p))
			{
//check edge distance
			if (LineToPoint(p, p1, p2) < threshold)
				{
				hitEdge = startEdge;
				done = TRUE;
//				LineToPoint(p, p1, p2);
				}
			}
		}
 	ct++;
	startEdge++;
	if (ct == ePtrList.Count()) done = TRUE;
	if (startEdge >= ePtrList.Count()) startEdge = 0;
	}
 
 return hitEdge;
}




void	UnwrapMod::RebuildEdges()
	{
	if (mode == ID_SKETCHMODE)
		SetMode(ID_UNWRAP_MOVE);


	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		ld->SetTVEdgeInvalid();
		ld->SetGeoEdgeInvalid();
		ld->BuildTVEdges();
		ld->BuildVertexClusterList();

	}

/*
BitArray holdVSel(vsel);


	BOOL holdSyncMode = fnGetSyncSelectionMode();
	fnSetSyncSelectionMode(FALSE);


	TVMaps.BuildEdges(  );
	//PELT
	TVMaps.BuildGeomEdges(  );

	if (esel.GetSize() != TVMaps.ePtrList.Count())
		{
		esel.SetSize(TVMaps.ePtrList.Count());
		esel.ClearAll();
		}

	if (gesel.GetSize() != TVMaps.gePtrList.Count())
		{
		gesel.SetSize(TVMaps.gePtrList.Count());
		gesel.ClearAll();
		}

	//FP 05/26/06 : PELT Mapping - CER Bucket #370291
	//When the topology of the object changes, the seams bits array need to be updated.  
	//By clearing it there, the update will be done each time the topology changes
	if (peltData.seamEdges.GetSize() != TVMaps.gePtrList.Count())
		{
		peltData.seamEdges.SetSize(TVMaps.gePtrList.Count());
		peltData.seamEdges.ClearAll();
		}




	vsel = holdVSel;

	fnSetSyncSelectionMode(holdSyncMode);

	usedVertices.SetSize(TVMaps.v.Count());
	usedVertices.ClearAll();

	for (int i = 0; i < TVMaps.f.Count(); i++)
		{
		int faceIndex = i;
		for (int k = 0; k < TVMaps.f[faceIndex]->count; k++)
			{
			if (!(TVMaps.f[faceIndex]->flags & FLAG_DEAD))
				{
				int vertIndex = TVMaps.f[faceIndex]->t[k];
				usedVertices.Set(vertIndex);
				if (objType == IS_PATCH)
					{
					if ((TVMaps.f[faceIndex]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[faceIndex]->vecs))
						{
						if (TVMaps.f[faceIndex]->flags & FLAG_INTERIOR)
							{
							vertIndex = TVMaps.f[faceIndex]->vecs->interiors[k];
							if ((vertIndex >=0) && (vertIndex < usedVertices.GetSize()))
								usedVertices.Set(vertIndex);
							}
						vertIndex = TVMaps.f[faceIndex]->vecs->handles[k*2];
						if ((vertIndex >=0) && (vertIndex < usedVertices.GetSize()))
							usedVertices.Set(vertIndex);
						vertIndex = TVMaps.f[faceIndex]->vecs->handles[k*2+1];
						if ((vertIndex >=0) && (vertIndex < usedVertices.GetSize()))
							usedVertices.Set(vertIndex);
						}
					}
				}
			}
		}

	//PELT
	for (int i = 0; i < peltData.springEdges.Count(); i++)
	{
		int vertIndex = peltData.springEdges[i].v1;
		if (vertIndex != -1)
			usedVertices.Set(vertIndex);
		vertIndex = peltData.springEdges[i].v2;
		if (vertIndex != -1)
			usedVertices.Set(vertIndex);

		vertIndex = peltData.springEdges[i].vec1;
		if ( (vertIndex != -1) && (vertIndex < usedVertices.GetSize()))
			usedVertices.Set(vertIndex);
		vertIndex = peltData.springEdges[i].vec2;
		if ( (vertIndex != -1) && (vertIndex < usedVertices.GetSize()))
			usedVertices.Set(vertIndex);

	}
	BuildEdgeDistortionData();
*/
}




//pelt
void UVW_ChannelClass::EdgeListFromPoints(Tab<int> &selEdges, int source, int target, Point3 vec)
{
	//make sure point a and b are on the same element if not bail
	Tab<VConnections*> ourConnects;
	BOOL del = FALSE;

	BitArray selVerts;
	selVerts.SetSize(geomPoints.Count());
	selVerts.ClearAll();

//	if (TRUE)
	
		//loop through the edges
		Tab<int> numberOfConnections;
		numberOfConnections.SetCount(geomPoints.Count());
		for (int i = 0; i < geomPoints.Count(); i++)
		{
			numberOfConnections[i] = 0;
		}
		//count the number of vertxs connected
		for (int i = 0; i < gePtrList.Count(); i++)
		{
			if (!(gePtrList[i]->flags & FLAG_HIDDENEDGEA))
			{
				int a = gePtrList[i]->a;
				numberOfConnections[a] +=1;
				int b = gePtrList[i]->b;
				numberOfConnections[b] +=1;
			}
		}
		//allocate our connections now

		ourConnects.SetCount(geomPoints.Count());
		for (int i = 0; i < geomPoints.Count(); i++)
		{
			ourConnects[i] = new VConnections();
			ourConnects[i]->closestNode = NULL;
			ourConnects[i]->linkedListChild = NULL;
			ourConnects[i]->linkedListParent = NULL;
			ourConnects[i]->accumDist = 1.0e+9f;
			ourConnects[i]->solved = FALSE;
			ourConnects[i]->vid = i;

			ourConnects[i]->connectingVerts.SetCount(numberOfConnections[i]);
		}

		for (int i = 0; i < geomPoints.Count(); i++)
		{
			numberOfConnections[i] = 0;
		}

		//build our vconnection data
		for (int i = 0; i < gePtrList.Count(); i++)
		{
			if (!(gePtrList[i]->flags & FLAG_HIDDENEDGEA))
			{
				int a = gePtrList[i]->a;
				int b = gePtrList[i]->b;

				int index = numberOfConnections[a];
				ourConnects[a]->connectingVerts[index].vertexIndex = b;
				ourConnects[a]->connectingVerts[index].edgeIndex = i;
				numberOfConnections[a] +=1;

				index = numberOfConnections[b];
				ourConnects[b]->connectingVerts[index].vertexIndex = a;
				ourConnects[b]->connectingVerts[index].edgeIndex = i;
				numberOfConnections[b] +=1;
			}
		}


		del = TRUE;
	
	//	else ourConnects = vConnects;

	//spider out till hit our target or no more left
	BOOL done = FALSE;
	BOOL hit = FALSE;
	Tab<int> vertsToDo;
	BitArray processedVerts;
	processedVerts.SetSize(ourConnects.Count());
	processedVerts.ClearAll();

	//if no more left bail
	int currentVert = source;
	while (!done)
	{
		//	int startingNumberProcessed = processedVerts.NumberSet();

		int ct = ourConnects[currentVert]->connectingVerts.Count();
		processedVerts.Set(currentVert, TRUE);
		for (int j = 0; j < ct; j++)
		{
			int index = ourConnects[currentVert]->connectingVerts[j].vertexIndex;
			if (index == target) 
			{
				done = TRUE;
				hit = TRUE;
			}
			if (!processedVerts[index])
				vertsToDo.Append(1,&index, 10000);
		}

		if (vertsToDo.Count())
		{
			currentVert = vertsToDo[vertsToDo.Count()-1];
			vertsToDo.Delete(vertsToDo.Count()-1,1);
		}

		if (vertsToDo.Count() == 0) done = TRUE;
		//		int endingNumberProcessed = processedVerts.NumberSet();

		if (currentVert == target) 
		{
			done = TRUE;
			hit = TRUE;
		}
		//		if (startingNumberProcessed == endingNumberProcessed)
		//			done = TRUE;
	}
	vertsToDo.ZeroCount();

 	if (hit)
	{
		Tab<VConnections*> solvedNodes;

		ourConnects[source]->accumDist = 0;

		VConnections* unsolvedNodeHead = ourConnects[source];
		VConnections* unsolvedNodeCurrent = unsolvedNodeHead;
		
		//put all our vertices in the unsolved list
		
		for (int i = 0; i < ourConnects.Count(); i++)
		{			
			if (ourConnects[i] != unsolvedNodeHead)
			{
				unsolvedNodeCurrent->linkedListChild = ourConnects[i];
				VConnections *parent = unsolvedNodeCurrent;
				unsolvedNodeCurrent = ourConnects[i];
				unsolvedNodeCurrent->linkedListParent = parent;
			}
		}

		//build our edge distances
		Tab<float> edgeDistances;
		edgeDistances.SetCount(gePtrList.Count());
		for (int i = 0 ; i < gePtrList.Count(); i++)
		{
			int a = gePtrList[i]->a;
			int b = gePtrList[i]->b;
			float d = Length(geomPoints[a] - geomPoints[b]);
			edgeDistances[i] = d;

		}


		BOOL done = FALSE;
		while (!done)
		{
			//pop the top unsolved
			VConnections *top = unsolvedNodeHead;


			unsolvedNodeHead = unsolvedNodeHead->linkedListChild;
			top->linkedListChild = NULL;
			top->linkedListParent = NULL;
			if (unsolvedNodeHead != NULL)
			{
				unsolvedNodeHead->linkedListParent = NULL;
				//mark it as processed
				top->solved = TRUE;
				
				//put it in our solved list
				solvedNodes.Append(1,&top,5000);

				int neighborCount = top->connectingVerts.Count();
				//loop through the neighbors
				for (int i = 0; i < neighborCount; i++)
				{
					int index  = top->connectingVerts[i].vertexIndex;
					int eindex  = top->connectingVerts[i].edgeIndex;
					VConnections *neighbor = ourConnects[index];
					//make sure it is not procssedd
					if (!neighbor->solved)
					{
						//find the distance from the top to this neighbor
						float d = neighbor->accumDist;
						float testAccumDistance = top->accumDist + edgeDistances[eindex];
						//see if it accum dist needs to be relaxed
						if (testAccumDistance<d)
						{
							float originalDist = neighbor->accumDist;
							neighbor->accumDist = testAccumDistance;
							neighbor->closestNode = top;
							//sort this node
							float dist = neighbor->accumDist;


							if (originalDist == 1.0e+9f)
							{
								//start at the top and look down
								VConnections *currentNode = unsolvedNodeHead;
								if (neighbor == currentNode)
								{
									currentNode = currentNode->linkedListChild;
									unsolvedNodeHead = currentNode;
								}
								VConnections *prevNode = NULL;
								//unhook node
								VConnections *parent = neighbor->linkedListParent;
								VConnections *child = neighbor->linkedListChild;
								if (parent)
									parent->linkedListChild = child;
								if (child)
									child->linkedListParent = parent;
									

								while ((currentNode!= NULL) && (currentNode->accumDist < dist))
								{
								
									prevNode = currentNode;
									currentNode = currentNode->linkedListChild;
									SHORT iret = GetAsyncKeyState (VK_ESCAPE);
									if (iret==-32767)
									{
										done = TRUE;
										currentNode= NULL;
									}
								}
								//empty list
								if ((prevNode==NULL) && (currentNode== NULL))
								{
									neighbor->linkedListChild = NULL;
									neighbor->linkedListParent = NULL;
									unsolvedNodeHead = neighbor;
								}
								//at top
								else if (currentNode && (prevNode == NULL))
								{
									unsolvedNodeHead->linkedListParent = neighbor;
									neighbor->linkedListParent = NULL;
									neighbor->linkedListChild =unsolvedNodeHead;
									unsolvedNodeHead = neighbor;

								}
								//at bottom
								else if (currentNode == NULL)
								{
									prevNode->linkedListChild = neighbor;
									neighbor->linkedListParent = currentNode;
									neighbor->linkedListChild = NULL;
								}

								else if (currentNode)
								{
									//insert
									VConnections *parent = currentNode->linkedListParent;
									VConnections *child = currentNode;
		
									parent->linkedListChild = neighbor;
									child->linkedListParent = neighbor;

									neighbor->linkedListParent = parent;
									neighbor->linkedListChild = child;

								}



							}
							else
							{
								//sort smallest to largest
								BOOL moveUp = FALSE;

								if (neighbor->linkedListParent && neighbor->linkedListChild)
								{
									float parentDist = neighbor->linkedListParent->accumDist;
									float childDist = neighbor->linkedListChild->accumDist;
									//walkup up or down the list till find a spot an unlink and relink
									
									if ((dist >= parentDist) && (dist <= childDist))
									{
										//done dont need to move
									}
									else if (dist < parentDist)
										moveUp = FALSE;
								}
								else if (neighbor->linkedListParent && (neighbor->linkedListChild==NULL))
								{
									moveUp = TRUE;
								}
								else if ((neighbor->linkedListParent==NULL) && (neighbor->linkedListChild))
								{
									moveUp = FALSE;
								}

								//unlink the node
								//unhook node
								VConnections *parent = neighbor->linkedListParent;
								VConnections *child = neighbor->linkedListChild;
								if (parent)
									parent->linkedListChild = child;
								else unsolvedNodeHead = child;
								if (child)
									child->linkedListParent = parent;

								VConnections *currentNode = NULL;
								if (moveUp)
									currentNode = neighbor->linkedListParent;
								else currentNode = neighbor->linkedListChild;
								VConnections *prevNode = NULL;

								while ((currentNode!= NULL) && (currentNode->accumDist < dist))
								{							
									prevNode = currentNode;
									if (moveUp)
										currentNode = currentNode->linkedListParent;
									else currentNode = currentNode->linkedListChild;

									SHORT iret = GetAsyncKeyState (VK_ESCAPE);
									if (iret==-32767)
									{
										done = TRUE;
										currentNode= NULL;
									}

								}

								//empty list
								if ((prevNode==NULL) && (currentNode== NULL))
								{
									neighbor->linkedListChild = NULL;
									neighbor->linkedListParent = NULL;
									unsolvedNodeHead = neighbor;
								}
								//at top
								else if (currentNode && (prevNode == NULL))
								{
									unsolvedNodeHead->linkedListParent = neighbor;
									neighbor->linkedListParent = NULL;
									neighbor->linkedListChild =unsolvedNodeHead;
									unsolvedNodeHead = neighbor;

								}
								//at bottom
								else if (currentNode == NULL)
								{
									prevNode->linkedListChild = neighbor;
									neighbor->linkedListParent = currentNode;
									neighbor->linkedListChild = NULL;
								}
								else if (currentNode)
								{
									//insert
									VConnections *parent = currentNode->linkedListParent;
									VConnections *child = currentNode;
		
									parent->linkedListChild = neighbor;
									child->linkedListParent = neighbor;

									neighbor->linkedListParent = parent;
									neighbor->linkedListChild = child;

								}


							}

						}					
					}
				}
			}
			

			if (unsolvedNodeHead == NULL)
				done = TRUE;
			if ((solvedNodes[solvedNodes.Count()-1]) == ourConnects[target])
				done = TRUE;
		}
		//now get our edge list
		selEdges.ZeroCount();
		VConnections *cNode = ourConnects[target];
		while ((cNode->closestNode != NULL) && (cNode != ourConnects[source]))
		{
			VConnections *pNode = cNode->closestNode;
			int ct = cNode->connectingVerts.Count();
			int edgeIndex = -1;
			for (int i = 0; i < ct; i++)
			{
				int vindex = cNode->connectingVerts[i].vertexIndex;
				int eindex = cNode->connectingVerts[i].edgeIndex;
				if (ourConnects[vindex] == pNode)
				{
					edgeIndex = eindex;
					i = ct;
				}
			}
			if (edgeIndex != -1)
				selEdges.Append(1,&edgeIndex,500);

			cNode = pNode;
		}

	}

	if (del)
	{
		for (int i = 0; i < geomPoints.Count(); i++)
		{
			delete ourConnects[i];
		}
	}
}

int UVW_ChannelClass::GetNextEdge(int currentEdgeIndex, int cornerVert, int currentFace)
{
	//get the a b points
	int a = ePtrList[currentEdgeIndex]->a;
	int b = ePtrList[currentEdgeIndex]->b;

	//loop through the face looking for the corner
	int deg = f[currentFace]->count;
	int opposingVert = -1;
	for (int j = 0; j < deg; j++)
	{
		int fa = f[currentFace]->t[j];
		int fb = f[currentFace]->t[(j+1)%deg];

		if (fa == cornerVert)
		{
			//make sure it is not our initial edge
			if ( ((fa!= a) && (fb != b)) || ((fa!= b) && (fb != a)) )
				opposingVert = fb;
		}
		if (fb == cornerVert)
		{
			//make sure it is not our initial edge
			if ( ((fa!= a) && (fb != b)) || ((fa!= b) && (fb != a)) )
				opposingVert = fa;
		}
	}

//find out the edge
	int nextEdge = -1;
	if (opposingVert != -1)
	{
		int ct = e[cornerVert]->data.Count();
		
		for (int i = 0; i < ct; i++)
		{
			int ea = e[cornerVert]->data[i]->a;
			int eb = e[cornerVert]->data[i]->b;
			if ( ( (ea == cornerVert) && (eb == opposingVert) ) ||
				 ( (eb == cornerVert) && (ea == opposingVert) ) )
			{
				nextEdge =  e[cornerVert]->data[i]->lookupIndex;
				i = ct; 
			}
		}
		if (nextEdge != -1)
		{
			ct = e[opposingVert]->data.Count();
					
			for (int i = 0; i < ct; i++)
			{
				int ea = e[opposingVert]->data[i]->a;
				int eb = e[opposingVert]->data[i]->b;
				if ( ( (ea == cornerVert) && (eb == opposingVert) ) ||
					( (eb == cornerVert) && (ea == opposingVert) ) )
				{
					nextEdge =  e[opposingVert]->data[i]->lookupIndex;
					i = ct; 
				}
			}
		}
	}
	return nextEdge;
}

//PELT

void UVW_ChannelClass::SplitUVEdges(BitArray esel)
{
	
	//get our point selection from the edges
	BitArray pointSel;
	pointSel.SetSize(v.Count());
	pointSel.ClearAll();
	for (int i = 0; i < ePtrList.Count(); i++)
	{
		if (esel[i])
		{
			int a = ePtrList[i]->a;
			int b = ePtrList[i]->b;
			pointSel.Set(a,TRUE);
			pointSel.Set(b,TRUE);
		}
	}

//build a lis of egdes for each vert
	Tab<VEdges*> edgesAtVertex;
	edgesAtVertex.SetCount(v.Count());
	for (int i = 0; i < v.Count(); i++)
		edgesAtVertex[i] = NULL;

	for (int i = 0; i < ePtrList.Count(); i++)
	{
		int a = ePtrList[i]->a;
		if (pointSel[a])
		{
			if (edgesAtVertex[a] == NULL)
				edgesAtVertex[a] = new VEdges();

			edgesAtVertex[a]->edgeIndex.Append(1,&i,5);
		}

		a = ePtrList[i]->b;
		if (pointSel[a])
		{
			if (edgesAtVertex[a] == NULL)
				edgesAtVertex[a] = new VEdges();

			edgesAtVertex[a]->edgeIndex.Append(1,&i,5);
		}

	}

	BitArray processedFaces;
	processedFaces.SetSize(f.Count());

	BitArray processedEdges;
	processedEdges.SetSize(ePtrList.Count());

	int originalCount = v.Count();
	for (int i = 0; i < originalCount; i++)
	{
		if (pointSel[i])
		{
			processedFaces.ClearAll();
			processedEdges.ClearAll();

			int ct = edgesAtVertex[i]->edgeIndex.Count();
			BOOL first = TRUE;


			for (int j = 0; j < ct; j++)
			{
				int currrentEdgeIndex =  edgesAtVertex[i]->edgeIndex[j];
				//find a selected edge
				if (esel[currrentEdgeIndex])
				{

					int numFaces = ePtrList[currrentEdgeIndex]->faceList.Count();

					for (int m = 0; m < numFaces; m++)
					{
						int newVertIndex = -1;
						if (first)
						{
							//just set the index
							newVertIndex = i;
							first = FALSE;
						}
						else
						{
							//create a new vertex 
							UVW_TVVertClass newVert;
							newVert.SetP(v[i].GetP());
							newVert.SetInfluence(0.0f);
							newVert.SetFlag(0);
							//create a new handle if we need one
							v.Append(1,&newVert);

							newVertIndex = v.Count()-1;
						}
						int currentFaceIndex = ePtrList[currrentEdgeIndex]->faceList[m];
						if (!processedFaces[currentFaceIndex])
						{
							BOOL done = FALSE;
							int lastEdge = j;
							while (!done)
							{
								//loop through this face looking for matching vert
								int deg = f[currentFaceIndex]->count;
								for (int n = 0; n < deg; n++)
								{
									//replace it
									if (f[currentFaceIndex]->t[n] == i)
									{
										f[currentFaceIndex]->t[n] = newVertIndex;
									}
								}
								processedFaces.Set(currentFaceIndex,TRUE);

								//loop til we find another selected face or an opent edge
								int nextEdge = -1;								
								for (int n = 0; n < ct; n++)
								{
									//mkae sure we are not looking at the current edge
									int potentialEdge = edgesAtVertex[i]->edgeIndex[n];
									if (n != lastEdge)
									{										
										int nfaces = ePtrList[potentialEdge]->faceList.Count();
										if (nfaces != -1)
										{
											for (int p = 0; p < nfaces; p++)
											{
												if (ePtrList[potentialEdge]->faceList[p] == currentFaceIndex)
												{
													nextEdge = potentialEdge;
													lastEdge = n;
													p = nfaces;
													n = ct; 
												}
											}
										}
									}
								}
								//if we hit an edge we are done with this cluster
								if (nextEdge==-1)
									done = TRUE;
								else if (ePtrList[nextEdge]->a == ePtrList[nextEdge]->b)
									done = TRUE;
								else if (esel[nextEdge])
									done = TRUE;
								else if (nextEdge != -1)
								{
									//get the next face
									int nfaces = ePtrList[nextEdge]->faceList.Count();
									BOOL hit = FALSE;
									for (int p = 0; p < nfaces; p++)
									{
										if (ePtrList[nextEdge]->faceList[p]!= currentFaceIndex)
										{
											currentFaceIndex = ePtrList[nextEdge]->faceList[p];
											hit = TRUE;
											p = nfaces;
										}
									}
									if (!hit) done = TRUE;

								}



							}
						}
					}
				}
			}
		}
	}

	for (int i = 0; i < originalCount; i++)
	{
		if (edgesAtVertex[i])
			delete edgesAtVertex[i];
		
	}


	

	BuildEdges();
	BuildGeomEdges();

	Tab<int> numberConnectedEdges;
	numberConnectedEdges.SetCount(v.Count());
	for (int i = 0; i < v.Count(); i++)
	{
		numberConnectedEdges[i] = 0;
	}
//loop through our egdes
	for (int i = 0; i < ePtrList.Count(); i++)
	{
		int veca = ePtrList[i]->avec;
		int vecb = ePtrList[i]->bvec;
		if (veca != -1)
			numberConnectedEdges[veca] += 1;
		if (vecb != -1)
			numberConnectedEdges[vecb] += 1;
	}

	for (int i = 0; i < f.Count(); i++)
	{
		if (f[i]->vecs)
		{
			int deg = f[i]->count;
			for (int j = 0; j < deg*2; j++)
			{
				int va;
				va = f[i]->vecs->handles[j];
				if (numberConnectedEdges[va] > 1)
				{
					//clone the vert
					UVW_TVVertClass newVert;
					newVert.SetP(v[va].GetP());
					newVert.SetInfluence(0.0f);
					newVert.SetFlag(0);
					//create a new handle if we need one
					v.Append(1,&newVert);

					int newVertIndex = v.Count()-1;
					//assign it
					f[i]->vecs->handles[j] = newVertIndex;
					//dec our counrt
					numberConnectedEdges[va] -= 1;
				}
			}
		}
	}

	mSystemLockedFlag.SetSize(v.Count(), 1);

	BuildEdges();
	BuildGeomEdges();

	

}


int UVW_ChannelClass::FindGeoEdge(int a, int b)
{
	int edgeIndex = -1;
	int ct = ge[a]->data.Count();
	for (int i = 0; i < ct; i++)
	{
		int ea = ge[a]->data[i]->a;
		int eb = ge[a]->data[i]->b;

		if ( ((ea == a) && (eb == b)) ||
			 ((ea == b) && (eb == a)) )
			 return ge[a]->data[i]->lookupIndex;
	}

	ct = ge[b]->data.Count();
	for (int i = 0; i < ct; i++)
	{
		int ea = ge[b]->data[i]->a;
		int eb = ge[b]->data[i]->b;

		if ( ((ea == a) && (eb == b)) ||
			 ((ea == b) && (eb == a)) )
			 return ge[b]->data[i]->lookupIndex;
	}

	return edgeIndex;
}


int UVW_ChannelClass::FindUVEdge(int a, int b)
{
	int edgeIndex = -1;
	int ct = e[a]->data.Count();
	for (int i = 0; i < ct; i++)
	{
		int ea = e[a]->data[i]->a;
		int eb = e[a]->data[i]->b;

		if ( ((ea == a) && (eb == b)) ||
			((ea == b) && (eb == a)) )
			return e[a]->data[i]->lookupIndex;
	}

	ct = e[b]->data.Count();
	for (int i = 0; i < ct; i++)
	{
		int ea = e[b]->data[i]->a;
		int eb = e[b]->data[i]->b;

		if ( ((ea == a) && (eb == b)) ||
			((ea == b) && (eb == a)) )
			return e[b]->data[i]->lookupIndex;
	}

	return edgeIndex;
}



void UVW_ChannelClass::BuildAdjacentUVEdgesToVerts(Tab<AdjacentItem*> &verts)
{
	verts.SetCount(v.Count());
	for (int i = 0; i < v.Count(); i++)
	{
		verts[i] = new AdjacentItem();
	}

	for (int i = 0; i < ePtrList.Count(); i++)
	{
		int a = ePtrList[i]->a;
		verts[a]->index.Append(1,&i,5);
		a = ePtrList[i]->b;
		verts[a]->index.Append(1,&i,5);
	}

}


void UVW_ChannelClass::BuildAdjacentGeomEdgesToVerts(Tab<AdjacentItem*> &verts)
{
	verts.SetCount(geomPoints.Count());
	for (int i = 0; i < geomPoints.Count(); i++)
	{
		verts[i] = new AdjacentItem();
	}

	for (int i = 0; i < gePtrList.Count(); i++)
	{
		int a = gePtrList[i]->a;
		verts[a]->index.Append(1,&i,5);
		a = gePtrList[i]->b;
		verts[a]->index.Append(1,&i,5);
	}
}

void UVW_ChannelClass::BuildAdjacentUVFacesToVerts(Tab<AdjacentItem*> &verts)
{
	verts.SetCount(v.Count());
	for (int i = 0; i < v.Count(); i++)
	{
		verts[i] = new AdjacentItem();
	}

	for (int i = 0; i < f.Count(); i++)
	{
		if (!(f[i]->flags & FLAG_DEAD ))
		{
			int deg = f[i]->count;
			for (int j = 0; j < deg; j++)
			{
				int a = f[i]->t[j];

				verts[a]->index.Append(1,&i,5);
			}
		}
	}
}


Point3 UVW_ChannelClass::UVFaceNormal(int index)
{
	if (index < 0) return Point3(0.0f,0.0f,1.0f);
	if (index >= f.Count()) return Point3(0.0f,0.0f,1.0f);

	if (f[index]->count < 3) 
		return Point3(0.0f,0.0f,1.0f);



	Point3 vec1,vec2;
	if (f[index]->count == 3)
	{
		int a = f[index]->t[0];
		int b = f[index]->t[1];
		int c = f[index]->t[2];
		vec1 = Normalize(v[b].GetP()-v[a].GetP());
		vec2 = Normalize(v[c].GetP()-v[a].GetP());

	}
	else
	{
		int a = f[index]->t[0];
		int b = f[index]->t[1];
		
		vec1 = Normalize(v[b].GetP()-v[a].GetP());
		for (int i = 2; i < f[index]->count; i++)
		{
			b = f[index]->t[i];
			vec2 = Normalize(v[b].GetP()-v[a].GetP());
			float dot = DotProd(vec1,vec2);
			if (fabs(dot) != 1.0f) 
				i = f[index]->count;
		}

	}

	Point3 norm = CrossProd(vec1,vec2);
	return Normalize(norm);

}

Point3 UVW_ChannelClass::GeomFaceNormal(int index)
{
	if (index < 0) return Point3(0.0f,0.0f,1.0f);
	if (index >= f.Count()) return Point3(0.0f,0.0f,1.0f);

	if (f[index]->count < 3) 
		return Point3(0.0f,0.0f,1.0f);

	Point3 vec1,vec2;
	if (f[index]->count == 3)
	{
		int a = f[index]->v[0];
		int b = f[index]->v[1];
		int c = f[index]->v[2];
		vec1 = Normalize(geomPoints[b]-geomPoints[a]);
		vec2 = Normalize(geomPoints[c]-geomPoints[a]);
		Point3 norm = CrossProd(vec1,vec2);
		return Normalize(norm);
	}
	else
	{

		int i;
		Point3 tempC(0.0f,0.0f,0.0f); // Don't mess with f[fc].c.
		int deg = f[index]->count;
		for (i = 0; i < deg; i++) 
		{
			int a = f[index]->v[i];
			Point3 p = geomPoints[a];
			tempC += p;
		}

		tempC = tempC/((float)deg);
		Point3 norm = Point3(0.0f,0.0f,0.0f);
		for (i=0; i< deg; i++) 
		{
			int a =  f[index]->v[i];
			int b =  f[index]->v[(i+1)%deg];
			norm += (geomPoints[a] - tempC) ^ (geomPoints[b] - tempC);
		}
		norm = Normalize(norm);
		return norm;
	}



}


Matrix3 UVW_ChannelClass::MatrixFromGeoFace(int index)
{

	if (f[index]->count < 3) 
		return Matrix3(1);

	Matrix3 tm(1);
	Point3 xvec,yvec,zvec;
	zvec = GeomFaceNormal(index);
	int a,b;
	a = f[index]->v[0];
	b = f[index]->v[1];
	xvec = Normalize(geomPoints[b]-geomPoints[a]);
	yvec = Normalize(CrossProd(xvec,zvec));

	tm.SetRow(0,xvec);
	tm.SetRow(1,yvec);
	tm.SetRow(2,zvec);
	tm.SetRow(3,geomPoints[a]);

	return tm;
}
Matrix3 UVW_ChannelClass::MatrixFromUVFace(int index)
{
	Matrix3 tm(1);

	if (f[index]->count < 3) 
		return Matrix3(1);

	Point3 xvec,yvec,zvec;
	zvec = UVFaceNormal(index);
	int a,b;
	a = f[index]->t[0];
	b = f[index]->t[1];
	xvec = Normalize(v[b].GetP()-v[a].GetP());
	yvec = Normalize(CrossProd(xvec,zvec));

	tm.SetRow(0,xvec);
	tm.SetRow(1,yvec);
	tm.SetRow(2,zvec);
	tm.SetRow(3,v[a].GetP());

	return tm;
}


void UVW_ChannelClass::LoadOlderVersions(ILoad *iload)
{
	int ct = 0;
	ULONG nb = 0;
	
	switch(iload->CurChunkID())  
	{
		case VERTCOUNT_CHUNK:
			iload->Read(&ct, sizeof(ct), &nb);
			v.SetCount(ct);
			break;
		case FACECOUNT_CHUNK:
			iload->Read(&ct, sizeof(ct), &nb);
			SetCountFaces(ct);
			break;
/*
		case GEOMPOINTSCOUNT_CHUNK:
			iload->Read(&ct, sizeof(ct), &nb);
			geomPoints.SetCount(ct);
			break;
*/
			//old way here for legacy reason only
		case FACE_CHUNK:
			{
//				version = 1;
//				oldDataPresent = TRUE;
				Tab<TVFace> tf;
				tf.SetCount(f.Count());
				iload->Read(tf.Addr(0), sizeof(TVFace)*f.Count(), &nb);
				for (int i=0;i<f.Count();i++)
				{

					f[i]->count  = 3;
					f[i]->t = new int[f[i]->count];
					f[i]->v = new int[f[i]->count];

					f[i]->t[0] = tf[i].t[0];
					f[i]->t[1] = tf[i].t[1];
					f[i]->t[2] = tf[i].t[2];
					f[i]->flags = 0;
					f[i]->vecs = NULL;
				}
				break;
			}
			//old way here for legacy reason only
		case VERTS_CHUNK:
			{
				Tab<Point3> p;
				p.SetCount(v.Count());
//				oldDataPresent = TRUE;

				iload->Read(p.Addr(0), sizeof(Point3)*v.Count(), &nb);

				for (int i=0;i<v.Count();i++)
				{
					v[i].SetP(p[i]);
					v[i].SetFlag(0);
					v[i].SetInfluence(0.0f);
					v[i].SetControlID(-1);
				}
				break;
			}
		case FACE2_CHUNK:
			{
				Tab<UVW_TVFaceOldClass> oldData;
				oldData.SetCount(f.Count());
				iload->Read(oldData.Addr(0), sizeof(UVW_TVFaceOldClass)*oldData.Count(), &nb);
				for (int i = 0; i < f.Count(); i++)
				{
					//fix for bug 281118 it was checking an uniitiliazed flag
					if (oldData[i].flags & 8)  // not this was FLAG_QUAD but this define got removed
						f[i]->count=4;
					else f[i]->count=3;

					f[i]->t = new int[f[i]->count];
					f[i]->v = new int[f[i]->count];

					for (int j = 0; j < f[i]->count; j++)
						f[i]->t[j] = oldData[i].t[j];
					f[i]->FaceIndex = oldData[i].FaceIndex;
					f[i]->MatID = oldData[i].MatID;
					f[i]->flags = oldData[i].flags;
					f[i]->vecs = NULL;
				}
				/*
				//now compute the geom points
				for (int i = 0; i < f.Count(); i++)
				{
					int pcount = 3;
					pcount = f[i]->count;

					for (int j =0; j < pcount; j++)
					{
						BOOL found = FALSE;
						int index;
						for (int k =0; k < geomPoints.Count();k++)
						{
							if (oldData[i].pt[j] == geomPoints[k])
							{
								found = TRUE;
								index = k;
								k = geomPoints.Count();
							}
						}
						if (found)
						{
							f[i]->v[j] = index;
						}
						else
						{
							f[i]->v[j] = geomPoints.Count();
							geomPoints.Append(1,&oldData[i].pt[j],1);
						}
					}

				}
	*/
				break;
			}

		case FACE4_CHUNK:
			LoadFacesMax9(iload);	

			break;

		case VERTS2_CHUNK:
//fix
			{
				Tab<UVW_TVVertClass_Max9> tempV;
				tempV.SetCount(v.Count());
				iload->Read(tempV.Addr(0), sizeof(UVW_TVVertClass_Max9)*v.Count(), &nb);
				for (int i=0;i<v.Count();i++)
				{
					v[i].SetP(tempV[i].p);
					v[i].SetFlag(tempV[i].flags);
					v[i].SetInfluence(0.0f);
					v[i].SetControlID(-1);
				}
				break;
			}
/*
		case GEOMPOINTS_CHUNK:
			iload->Read(geomPoints.Addr(0), sizeof(Point3)*geomPoints.Count(), &nb);
			break;
*/
	}

}

void UVW_ChannelClass::ResolveOldData(UVW_ChannelClass &oldData)
{
	if (oldData.f.Count() == f.Count())
	{
		
		BOOL bail = FALSE;
		//make sure similiar topology
		for (int i = 0; i < f.Count();i++)
		{
			int degree = f[i]->count;
			int oldDegree = oldData.f[i]->count;
			if (oldDegree != degree)
				bail = TRUE;
		}
		if (!bail)
		{
			//copy the face data
			for (int i = 0; i < f.Count();i++)
			{
				int degree = f[i]->count;
				UVW_TVFaceClass *holdFace = f[i];
				f[i] = oldData.f[i]->Clone();
				for (int j = 0; j < degree; j++)
					f[i]->v[j] = holdFace->v[j];
				if (f[i]->vecs)
				{
					for (int j = 0; j < 8; j++)
						f[i]->vecs->vhandles[j] = holdFace->vecs->vhandles[j];
					for (int j = 0; j < 4; j++)
						f[i]->vecs->vinteriors[j] = holdFace->vecs->vinteriors[j];

				}
				delete holdFace;
			}
		//copy the vertex data
			v.SetCount(oldData.v.Count());
			for (int i = 0; i < oldData.v.Count();i++)
			{
				v[i] = oldData.v[i];
			}
			BuildEdges();
		}
	}
}
