/**********************************************************************
 *<
	FILE: relax.cpp

	DESCRIPTION:  Averages verts with nearby verts.

	CREATED BY: Steve Anderson

	HISTORY: created March 4, 1996

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "relaxmod.h"
#include "iparamm.h"

//--- Relax -----------------------------------------------------------

// Version info added for MAXr4, where we can now be applied to a PatchMesh and retain identity!
#define RELAXMOD_VER1 1
#define RELAXMOD_VER4 4

#define RELAXMOD_CURRENT_VERSION RELAXMOD_VER4

#define RELAX_OSM_CLASS_ID	100

#define MAX_ITER	999999999
#define MIN_ITER	0
#define MAX_RELAX	1.0f
#define MIN_RELAX	-1.0f
#define UIMIN_RELAX	0.0f

#define DEF_RELAX	0.5f
#define DEF_ITER	1
#define DEF_BOUNDARY	1
#define DEF_SADDLE 0

#define EPSILON float(1e-04)

typedef Tab<DWORD> DWordTab;

class RelaxModData : public LocalModData {
public:
	DWordTab *nbor;	// Array of neighbors for each vert.
	BitArray *vis;	// visibility of edges on path to neighbors.
	int *fnum;		// Number of faces for each vert.
	BitArray sel;		// Selection information.
	int vnum;		// Size of above arrays
	Interval ivalid;	// Validity interval of arrays.

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
	LocalModData *Clone ();
};

RelaxModData::RelaxModData () {
	nbor = NULL;
	vis = NULL;
	fnum = NULL;
	vnum = 0;
	ivalid = NEVER;
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

LocalModData *RelaxModData::Clone () {
	RelaxModData *clone;
	clone = new RelaxModData ();
	clone->SetVNum (vnum);
	for (int i=0; i<vnum; i++) {
		clone->nbor[i] = nbor[i];
		clone->vis[i].SetSize (vis[i].GetSize());
		clone->vis[i] = vis[i];
	}
	memcpy (clone->fnum, fnum, vnum*sizeof(int));
	clone->sel = sel;
	clone->ivalid = ivalid;
	return clone;
}

class RelaxMod: public Modifier {
protected:
	IParamBlock *pblock;
	int version;

	static IObjParam *ip;
	
public:
	static IParamMap *pmapParam;
	static RelaxMod *editMod;

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) { s= TSTR (GetString (IDS_RELAXMOD)); }  
	virtual Class_ID ClassID() { return Class_ID(RELAX_OSM_CLASS_ID,32670); }		
	RefTargetHandle Clone(RemapDir& remap = DefaultRemapDir());
	TCHAR *GetObjectName() { return GetString (IDS_RELAX); }
	// IO
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);

	RelaxMod();
	virtual ~RelaxMod();

	ChannelMask ChannelsUsed()  { return PART_GEOM | PART_TOPO | PART_SUBSEL_TYPE | PART_SELECT; }
	ChannelMask ChannelsChanged() { return PART_GEOM | PART_GFX_DATA; }
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);
	Class_ID InputType() { return defObjectClassID; }
	Interval LocalValidity(TimeValue t);

	// From BaseObject
	BOOL ChangeTopology() {return FALSE;}

	int NumRefs() {return 1;}
	RefTargetHandle GetReference(int i) {return pblock;}
	void SetReference(int i, RefTargetHandle rtarg) {pblock=(IParamBlock*)rtarg;}

 	int NumSubs() { return 1; }  
	Animatable* SubAnim(int i) { return pblock; }
	TSTR SubAnimName(int i) { return TSTR(GetString(IDS_PARAMETERS));}		

	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
	   PartID& partID, RefMessage message);

	CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 

	void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

	void UpdateUI(TimeValue t) {}
	Interval GetValidity(TimeValue t);
	TSTR GetParameterName(int pbIndex);
};

class RelaxClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new RelaxMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RELAX); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return  Class_ID(RELAX_OSM_CLASS_ID,32670); }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }
};

static RelaxClassDesc relaxDesc;
extern ClassDesc* GetRelaxModDesc() { return &relaxDesc; }

IObjParam*	RelaxMod::ip		= NULL;
IParamMap *	RelaxMod::pmapParam	= NULL;
RelaxMod *RelaxMod::editMod = NULL;

//--- Parameter map/block descriptors -------------------------------

#define PB_RELAX	0
#define PB_ITER		1
#define PB_BOUNDARY	2
#define PB_SADDLE 3

static ParamUIDesc descParam[] = {
	// Interp value
	ParamUIDesc (PB_RELAX, EDITTYPE_FLOAT, IDC_RELAX, IDC_RELAXSPIN,
		UIMIN_RELAX, MAX_RELAX, SPIN_AUTOSCALE),

	// Iterations
	ParamUIDesc(PB_ITER, EDITTYPE_POS_INT, IDC_ITER, IDC_ITERSPIN,
		(float) MIN_ITER, (float) MAX_ITER, 0.5f),
	
	ParamUIDesc (PB_BOUNDARY, TYPE_SINGLECHEKBOX, IDC_BOUNDARY),
	ParamUIDesc (PB_SADDLE, TYPE_SINGLECHEKBOX, IDC_SADDLE)
};

#define PARAMDESC_LENGTH 4

static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE,  PB_RELAX },
	{ TYPE_INT,   NULL, TRUE,  PB_ITER },
	{ TYPE_INT,   NULL, FALSE, PB_BOUNDARY }
};

static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE,  PB_RELAX },
	{ TYPE_INT,   NULL, TRUE,  PB_ITER },
	{ TYPE_INT,   NULL, FALSE, PB_BOUNDARY },
	{ TYPE_INT,   NULL, FALSE, PB_SADDLE }
};

#define PBLOCK_LENGTH	4

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,3,0),
};
#define NUM_OLDVERSIONS	1

// Current version
#define CURRENT_VERSION	1
static ParamVersionDesc curVersion(descVer1,PBLOCK_LENGTH,CURRENT_VERSION);

RelaxMod::RelaxMod() {
	pblock = NULL;
	ReplaceReference(0, CreateParameterBlock(descVer1, PBLOCK_LENGTH, CURRENT_VERSION));
	pblock->SetValue (PB_RELAX,		TimeValue(0), DEF_RELAX);
	pblock->SetValue (PB_ITER,		TimeValue(0), DEF_ITER);
	pblock->SetValue (PB_BOUNDARY,	TimeValue(0), DEF_BOUNDARY);
	pblock->SetValue (PB_SADDLE,	TimeValue(0), DEF_SADDLE);
	version = RELAXMOD_CURRENT_VERSION;
}

RelaxMod::~RelaxMod() { }

Interval RelaxMod::LocalValidity(TimeValue t) {
	// if being edited, return NEVER forces a cache to be built 
	// after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED)) return NEVER;  
	Interval valid = GetValidity(t);	
	return valid;
}

RefTargetHandle RelaxMod::Clone(RemapDir& remap) {
	RelaxMod* newmod = new RelaxMod();	
	newmod->ReplaceReference(0,remap.CloneRef(pblock));
	newmod->version = version;
	BaseClone(this, newmod, remap);
	return(newmod);
}

static void FindVertexAngles(PatchMesh &pm, float *vang) {
	int i;
	for (i=0; i<pm.numVerts + pm.numVecs; i++) vang[i] = 0.0f;
	for (i=0; i<pm.numPatches; i++) {
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
			if (cs>=1) continue;	// angle of 0
			if (cs<=-1) vang[p.v[j]] += PI;
			else vang[p.v[j]] += (float) acos (cs);
		}
	}
}

static void FindVertexAngles (MNMesh &mm, float *vang) {
	int i;
	for (i=0; i<mm.numv; i++) vang[i] = 0.0f;
	for (i=0; i<mm.numf; i++) {
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
			if (cs>=1) continue;	// angle of 0
			if (cs<=-1) vang[vv[j]] += PI;
			else vang[vv[j]] += (float) acos (cs);
		}
	}
}

void RelaxMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) {	
	// Get our personal validity interval...
	Interval valid = GetValidity(t);
	// and intersect it with the channels we use as input (see ChannelsUsed)
	valid &= os->obj->ChannelValidity (t, GEOM_CHAN_NUM);
	valid &= os->obj->ChannelValidity (t, TOPO_CHAN_NUM);
	valid &= os->obj->ChannelValidity (t, SUBSEL_TYPE_CHAN_NUM);
	valid &= os->obj->ChannelValidity (t, SELECT_CHAN_NUM);
	Matrix3 modmat,minv;
	
	if (mc.localData == NULL) mc.localData = new RelaxModData;
	RelaxModData *rd = (RelaxModData *) mc.localData;

	TriObject *triObj = NULL;
	PatchObject *patchObj = NULL;
	PolyObject *polyObj = NULL;
	BOOL converted = FALSE;

	// For version 4 and later, we process patch or poly meshes as they are and pass them on. 
	// Earlier versions converted to TriMeshes (done below).
	// For adding other new types of objects, add them here!
#ifndef NO_PATCHES
	if(version >= RELAXMOD_VER4 && os->obj->IsSubClassOf(patchObjectClassID)) {
		patchObj = (PatchObject *)os->obj;
		}
	else	// If it's a TriObject, process it
#endif // NO_PATCHES
	if(os->obj->IsSubClassOf(triObjectClassID)) {
		triObj = (TriObject *)os->obj;
	}
	else if (os->obj->IsSubClassOf (polyObjectClassID)) {
		polyObj = (PolyObject *) os->obj;
	}
	else	// If it can convert to a TriObject, do it
	if(os->obj->CanConvertToType(triObjectClassID)) {
		triObj = (TriObject *)os->obj->ConvertToType(t, triObjectClassID);
		converted = TRUE;
		}
	else
		return;		// We can't deal with it!

	Mesh *mesh = triObj ? &(triObj->GetMesh()) : NULL;
#ifndef NO_PATCHES
	PatchMesh &pmesh = patchObj ? patchObj->GetPatchMesh(t) : *((PatchMesh *)NULL);
#else
	PatchMesh &pmesh = *((PatchMesh *)NULL);
#endif // NO_PATCHES

	float relax, wtdRelax; // mjm - 4.8.99
	int iter;
	BOOL boundary, saddle;

	pblock->GetValue (PB_RELAX,  t, relax,	 FOREVER);
	pblock->GetValue (PB_ITER,	 t, iter,	 FOREVER);
	pblock->GetValue (PB_BOUNDARY, t, boundary, FOREVER);
	pblock->GetValue (PB_SADDLE, t, saddle, FOREVER);

	LimitValue (relax, MIN_RELAX, MAX_RELAX);
	LimitValue (iter, MIN_ITER, MAX_ITER);

	if(triObj) {
		int i, j, max;
		DWORD selLevel = mesh->selLevel;
		// mjm - 4.8.99 - add support for soft selection
		// sca - 4.29.99 - extended soft selection support to cover EDGE and FACE selection levels.
		float *vsw = (selLevel!=MESH_OBJECT) ? mesh->getVSelectionWeights() : NULL;

		if (rd->ivalid.InInterval(t) && (mesh->numVerts != rd->vnum)) {
			// Shouldn't happen, but does with Loft bug and may with other bugs.
			rd->ivalid.SetEmpty ();
		}

		if (!rd->ivalid.InInterval(t)) {
			rd->SetVNum (mesh->numVerts);
			for (i=0; i<rd->vnum; i++) {
				rd->fnum[i]=0;
				rd->nbor[i].ZeroCount();
			}
			rd->sel.ClearAll ();
			DWORD *v;
			int k1, k2, origmax;
			for (i=0; i<mesh->numFaces; i++) {
				v = mesh->faces[i].v;
				for (j=0; j<3; j++) {
					if ((selLevel==MESH_FACE) && mesh->faceSel[i]) rd->sel.Set(v[j]);
					if ((selLevel==MESH_EDGE) && mesh->edgeSel[i*3+j]) rd->sel.Set(v[j]);
					if ((selLevel==MESH_EDGE) && mesh->edgeSel[i*3+(j+2)%3]) rd->sel.Set(v[j]);
					origmax = max = rd->nbor[v[j]].Count();
					rd->fnum[v[j]]++;
					for (k1=0; k1<max; k1++) if (rd->nbor[v[j]][k1] == v[(j+1)%3]) break;
					if (k1==max) { rd->nbor[v[j]].Append (1, v+(j+1)%3, 1); max++; }
					for (k2=0; k2<max; k2++) if (rd->nbor[v[j]][k2] == v[(j+2)%3]) break;
					if (k2==max) { rd->nbor[v[j]].Append (1, v+(j+2)%3, 1); max++; }
					if (max>origmax) rd->vis[v[j]].SetSize (max, TRUE);
					if (mesh->faces[i].getEdgeVis (j)) rd->vis[v[j]].Set (k1);
					else if (k1>=origmax) rd->vis[v[j]].Clear (k1);
					if (mesh->faces[i].getEdgeVis ((j+2)%3)) rd->vis[v[j]].Set (k2);
					else if (k2>= origmax) rd->vis[v[j]].Clear (k2);
				}
			}
	// mjm - begin - 4.8.99
	//		if (selLevel==MESH_VERTEX) rd->sel = mesh->vertSel;
			if (selLevel==MESH_VERTEX)
				rd->sel = mesh->vertSel;
			else if (selLevel==MESH_OBJECT)
				rd->sel.SetAll ();
	// mjm - end
			rd->ivalid  = os->obj->ChannelValidity (t, TOPO_CHAN_NUM);
			rd->ivalid &= os->obj->ChannelValidity (t, SUBSEL_TYPE_CHAN_NUM);
			rd->ivalid &= os->obj->ChannelValidity (t, SELECT_CHAN_NUM);
		}

		Tab<float> vangles;
		if (saddle) vangles.SetCount (rd->vnum);
		Point3 *hold = new Point3[rd->vnum];
		int act;
		for (int k=0; k<iter; k++) {
			for (i=0; i<rd->vnum; i++) hold[i] = triObj->GetPoint(i);
			if (saddle) mesh->FindVertexAngles (vangles.Addr(0));
			for (i=0; i<rd->vnum; i++) {
	// mjm - begin - 4.8.99
	//			if ((selLevel!=MESH_OBJECT) && (!rd->sel[i])) continue;
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
				triObj->SetPoint (i, hold[i]*(1-wtdRelax) + avg*wtdRelax/((float)act));
	//			triObj->SetPoint (i, hold[i]*(1-relax) + avg*relax/((float)act));
	// mjm - end
			}
		}
		delete [] hold;
	}

	if (polyObj) {
		int i, j, max;
		MNMesh & mm = polyObj->mm;
		float *vsw = (mm.selLevel!=MNM_SL_OBJECT) ? mm.getVSelectionWeights() : NULL;

		if (rd->ivalid.InInterval(t) && (mm.numv != rd->vnum)) {
			// Shouldn't happen, but does with Loft bug and may with other bugs.
			rd->ivalid.SetEmpty ();
		}

		if (!rd->ivalid.InInterval(t)) {
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
			rd->ivalid = os->obj->ChannelValidity (t, TOPO_CHAN_NUM);
			rd->ivalid &= os->obj->ChannelValidity (t, SUBSEL_TYPE_CHAN_NUM);
			rd->ivalid &= os->obj->ChannelValidity (t, SELECT_CHAN_NUM);
		}

		Tab<float> vangles;
		if (saddle) vangles.SetCount (rd->vnum);
		Tab<Point3> hold;
		hold.SetCount (rd->vnum);
		int act;
		for (int k=0; k<iter; k++) {
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
				polyObj->SetPoint (i, hold[i]*(1-wtdRelax) + avg*wtdRelax/((float)act));
			}
		}
	}

#ifndef NO_PATCHES
	else
	if(patchObj) {
		int i, j, max;
		DWORD selLevel = pmesh.selLevel;
		// mjm - 4.8.99 - add support for soft selection
		// sca - 4.29.99 - extended soft selection support to cover EDGE and FACE selection levels.
		float *vsw = (selLevel!=PATCH_OBJECT) ? pmesh.GetVSelectionWeights() : NULL;

		if (rd->ivalid.InInterval(t) && (pmesh.numVerts != rd->vnum)) {
			// Shouldn't happen, but does with Loft bug and may with other bugs.
			rd->ivalid.SetEmpty ();
		}

		if (!rd->ivalid.InInterval(t)) {
			int vecBase = pmesh.numVerts;
			rd->SetVNum (pmesh.numVerts + pmesh.numVecs);
			for (i=0; i<rd->vnum; i++) {
				rd->fnum[i]=1;		// For patches, this means it's not a boundary
				rd->nbor[i].ZeroCount();
			}
			rd->sel.ClearAll ();
			for (i=0; i<pmesh.numPatches; i++) {
				Patch &p = pmesh.patches[i];
				int vecLimit = p.type * 2;
				for (j=0; j<p.type; j++) {
					PatchEdge &e = pmesh.edges[p.edge[j]];
					BOOL isBoundary = (e.patches.Count() < 2) ? TRUE : FALSE;
					int theVert = p.v[j];
					int nextVert = p.v[(j+1)%p.type];
					int nextVec = p.vec[j*2] + vecBase;
					int nextVec2 = p.vec[j*2+1] + vecBase;
					int prevEdge = (j+p.type-1)%p.type;
					int prevVec = p.vec[prevEdge*2+1] + vecBase;
					int prevVec2 = p.vec[prevEdge*2] + vecBase;
					int theInterior = p.interior[j] + vecBase;
					// Establish selection bits
					if ((selLevel==PATCH_PATCH) && pmesh.patchSel[i]) {
						rd->sel.Set(theVert);
						rd->sel.Set(nextVec);
						rd->sel.Set(prevVec);
						rd->sel.Set(theInterior);
						}
					else
					if ((selLevel==PATCH_EDGE) && pmesh.edgeSel[p.edge[j]]) {
						rd->sel.Set(e.v1);
						rd->sel.Set(e.vec12 + vecBase);
						rd->sel.Set(e.vec21 + vecBase);
						rd->sel.Set(e.v2);
						}
					else
					if ((selLevel==PATCH_VERTEX) && pmesh.vertSel[theVert]) {
						rd->sel.Set(theVert);
						rd->sel.Set(nextVec);
						rd->sel.Set(prevVec);
						rd->sel.Set(theInterior);
						}

					// Set boundary flags if necessary
					if(isBoundary) {
						rd->fnum[theVert] = 0;
						rd->fnum[nextVec] = 0;
						rd->fnum[nextVec2] = 0;
						rd->fnum[nextVert] = 0;
						}

					// First process the verts
					int work = theVert;
					max = rd->nbor[work].Count();
					// Append the neighboring vectors
					rd->MaybeAppendNeighbor(work, nextVec, max);
					rd->MaybeAppendNeighbor(work, prevVec, max);
					rd->MaybeAppendNeighbor(work, theInterior, max);

					// Now process the edge vectors
					work = nextVec;
					max = rd->nbor[work].Count();
					// Append the neighboring points
					rd->MaybeAppendNeighbor(work, theVert, max);
					rd->MaybeAppendNeighbor(work, theInterior, max);
					rd->MaybeAppendNeighbor(work, prevVec, max);
					rd->MaybeAppendNeighbor(work, nextVec2, max);
					rd->MaybeAppendNeighbor(work, p.interior[(j+1)%p.type] + vecBase, max);

					work = prevVec;
					max = rd->nbor[work].Count();
					// Append the neighboring points
					rd->MaybeAppendNeighbor(work, theVert, max);
					rd->MaybeAppendNeighbor(work, theInterior, max);
					rd->MaybeAppendNeighbor(work, nextVec, max);
					rd->MaybeAppendNeighbor(work, prevVec2, max);
					rd->MaybeAppendNeighbor(work, p.interior[(j+p.type-1)%p.type] + vecBase, max);

					// Now append the interior, if not auto
					if(!p.IsAuto()) {
						work = theInterior;
						max = rd->nbor[work].Count();
						// Append the neighboring points
						rd->MaybeAppendNeighbor(work, p.v[j], max);
						rd->MaybeAppendNeighbor(work, nextVec, max);
						rd->MaybeAppendNeighbor(work, nextVec2, max);
						rd->MaybeAppendNeighbor(work, prevVec, max);
						rd->MaybeAppendNeighbor(work, prevVec2, max);
						for(int k = 1; k < p.type; ++k)
							rd->MaybeAppendNeighbor(work, p.interior[(j+k)%p.type] + vecBase, max);
						}
					}
				}
	// mjm - begin - 4.8.99
			if (selLevel==PATCH_VERTEX) {
				for (int i=0; i<pmesh.numVerts; ++i) {
					if (pmesh.vertSel[i]) rd->sel.Set(i);
				}
			}
			else if (selLevel==PATCH_OBJECT) rd->sel.SetAll();
	// mjm - end
			rd->ivalid  = os->obj->ChannelValidity (t, TOPO_CHAN_NUM);
			rd->ivalid &= os->obj->ChannelValidity (t, SUBSEL_TYPE_CHAN_NUM);
			rd->ivalid &= os->obj->ChannelValidity (t, SELECT_CHAN_NUM);
		}

		Tab<float> vangles;
		if (saddle) vangles.SetCount (rd->vnum);
		Point3 *hold = new Point3[rd->vnum];
		int act;
		for (int k=0; k<iter; k++) {
			for (i=0; i<rd->vnum; i++) hold[i] = patchObj->GetPoint(i);
			if (saddle)
				FindVertexAngles(pmesh, vangles.Addr(0));
			for (i=0; i<rd->vnum; i++) {
	// mjm - begin - 4.8.99
	//			if ((selLevel!=MESH_OBJECT) && (!rd->sel[i])) continue;
				if ( (!rd->sel[i] ) && (!vsw || vsw[i] == 0) ) continue;
	// mjm - end
				if (saddle && (i < pmesh.numVerts) && (vangles[i] <= 2*PI*.99999f)) continue;
				max = rd->nbor[i].Count();
				if (boundary && !rd->fnum[i]) continue;
				if (max<1)
					continue;
				Point3 avg(0.0f, 0.0f, 0.0f);
				for (j=0,act=0; j<max; j++) {
					act++;
					avg += hold[rd->nbor[i][j]];
				}
				if (act<1)
					continue;
	// mjm - begin - 4.8.99
				wtdRelax = (!rd->sel[i]) ? relax * vsw[i] : relax;
				patchObj->SetPoint (i, hold[i]*(1-wtdRelax) + avg*wtdRelax/((float)act));
	//			patchObj->SetPoint (i, hold[i]*(1-relax) + avg*relax/((float)act));
	// mjm - end
			}
		}
		delete [] hold;
		patchObj->patch.computeInteriors();
		patchObj->patch.ApplyConstraints();
	}
#endif // NO_PATCHES
	
	if(!converted) {
		os->obj->SetChannelValidity(GEOM_CHAN_NUM, valid);
	} else {
		// Stuff converted object into the pipeline!
		triObj->SetChannelValidity(TOPO_CHAN_NUM, valid);
		triObj->SetChannelValidity(GEOM_CHAN_NUM, valid);
		triObj->SetChannelValidity(TEXMAP_CHAN_NUM, valid);
		triObj->SetChannelValidity(MTL_CHAN_NUM, valid);
		triObj->SetChannelValidity(SELECT_CHAN_NUM, valid);
		triObj->SetChannelValidity(SUBSEL_TYPE_CHAN_NUM, valid);
		triObj->SetChannelValidity(DISP_ATTRIB_CHAN_NUM, valid);

		os->obj = triObj;
	}
}

void RelaxMod::NotifyInputChanged (Interval changeInt, PartID part, RefMessage message, ModContext *mc) {
	if (!(part & (PART_TOPO|PART_SELECT|PART_SUBSEL_TYPE))) return;
	RelaxModData *rd = (RelaxModData *) mc->localData;
	if (rd == NULL) return;
	rd->ivalid = NEVER;
}

void RelaxMod::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev ) {
	this->ip = ip;
	editMod = this;

	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);

	pmapParam = CreateCPParamMap(
		descParam,PARAMDESC_LENGTH,
		pblock,
		ip,
		hInstance,
		MAKEINTRESOURCE(IDD_RELAX),
		GetString (IDS_PARAMETERS),
		0);
}

void RelaxMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next) {
	this->ip = NULL;
	editMod = NULL;
	
	TimeValue t = ip->GetTime();

	// aszabo|feb.05.02 This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

	DestroyCPParamMap(pmapParam);
}

Interval RelaxMod::GetValidity(TimeValue t)	{
	int i;
	float f;
	// Only have to worry about animated parameters.
	Interval valid = FOREVER;
	pblock->GetValue (PB_RELAX, t, f, valid);
	pblock->GetValue (PB_ITER, t, i, valid);
	return valid;
}

RefResult RelaxMod::NotifyRefChanged(Interval changeInt, 
		RefTargetHandle hTarget, PartID& partID, 
   		RefMessage message ) 
   	{
	GetParamDim *gpd;
	GetParamName *gpn;
	switch (message) {
		case REFMSG_CHANGE:
			if (editMod==this && pmapParam) pmapParam->Invalidate();
			break;

		case REFMSG_GET_PARAM_DIM:
			gpd = (GetParamDim*)partID;			
			gpd->dim = defaultDim;
			return REF_STOP; 

		case REFMSG_GET_PARAM_NAME:
			gpn = (GetParamName*)partID;
			gpn->name = GetParameterName(gpn->index);			
			return REF_STOP; 
	}
	return(REF_SUCCEED);
}

TSTR RelaxMod::GetParameterName(int pbIndex) {
	switch (pbIndex) {
	case PB_RELAX:	return TSTR(GetString (IDS_RVALUE));
	case PB_ITER:	return TSTR(GetString (IDS_ITERATIONS));
	case PB_BOUNDARY:	return TSTR(GetString (IDS_BOUNDARY));
	default:		return TSTR(_T(""));
	}
}

#define VERSION_CHUNK	0x1000

IOResult RelaxMod::Save(ISave *isave) {
	ULONG nb;
	Modifier::Save(isave);
	isave->BeginChunk (VERSION_CHUNK);
	isave->Write (&version, sizeof(int), &nb);
	isave->EndChunk();
	return IO_OK;
	}

IOResult RelaxMod::Load(ILoad *iload) {
	Modifier::Load(iload);
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,0));
	IOResult res;
	ULONG nb;
	version = RELAXMOD_VER1;	// Set default version to old one
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case VERSION_CHUNK:
				res = iload->Read(&version,sizeof(int),&nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
}

	
