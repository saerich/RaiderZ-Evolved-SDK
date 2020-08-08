/**********************************************************************
 *<
	FILE: EditPoly.cpp

	DESCRIPTION: Edit Poly Modifier

	CREATED BY: Steve Anderson, based on Face Extrude modifier by Berteig, and my own Poly Select modifier.

	HISTORY: created March 2002

 *>	Copyright (c) 2002 Discreet, All Rights Reserved.
 **********************************************************************/

#include "EPoly.h"
#include "EditPoly.h"
#include "MeshDLib.h"
#include "spline3d.h"
#include "splshape.h"
#include "shape.h"
#include "MNChamferData10.h"
#include "EditPolyGrips.h"

PolyOperationRecord *PolyOperationRecord::Clone ()
{
	PolyOperationRecord *ret = new PolyOperationRecord (mpOp->Clone());
	if (mpData) ret->SetLocalData (mpData->Clone());
	return ret;
}

void PolyOperation::RecordSelection(MNMesh & mesh, EditPolyMod *pMod, EditPolyData *pData)
{
	// Each operation has a designated selection level, one of MNM_SL_VERTEX, MNM_SL_EDGE, or MNM_SL_FACE.
	// (MNM_SL_OBJECT requires no selection information.)
	if (MeshSelLevel() == MNM_SL_OBJECT) return;

	// Each operation can also indicate if it provides support for the current level, even if
	// the current level is different from its own.

	// If the current level is supported, it's translated into the correct level.
	// Otherwise, it's ignored, and the selection in the correct level is still used.

	int selLevel = pMod->GetEPolySelLevel();
	int msl = meshSelLevel[selLevel];
	bool useStackSel = pMod->getParamBlock()->GetInt (epm_stack_selection) != 0;

	if (msl == MeshSelLevel() || !SelLevelSupport(selLevel)) {
		// trivial case
		mSelection = *(pData->GetCurrentSelection (MeshSelLevel(), useStackSel));
	} else {
		// We need to translate between levels.
		int i;

		if (msl > MNM_SL_OBJECT) {
			BitArray *pSel = pData->GetCurrentSelection (msl, useStackSel);
			int limit = pSel ? pSel->GetSize() : NULL;
			switch (msl) {
				case MNM_SL_VERTEX:
					if (limit>mesh.numv) limit = mesh.numv;
					for (i=0; i<limit; i++) mesh.v[i].SetFlag (MN_USER, (*pSel)[i]!=0);
					break;
				case MNM_SL_EDGE:
					if (limit>mesh.nume) limit = mesh.nume;
					for (i=0; i<limit; i++) mesh.e[i].SetFlag (MN_USER, (*pSel)[i]!=0);
					break;
				case MNM_SL_FACE:
					if (limit>mesh.numf) limit = mesh.numf;
					for (i=0; i<limit; i++) mesh.f[i].SetFlag (MN_USER, (*pSel)[i]!=0);
					break;
			}

			switch (MeshSelLevel())
			{
			case MNM_SL_VERTEX: mesh.ClearVFlags (MN_USER); break;
			case MNM_SL_EDGE: mesh.ClearEFlags (MN_USER); break;
			case MNM_SL_FACE: mesh.ClearFFlags (MN_USER); break;
			}

			mesh.PropegateComponentFlags (MeshSelLevel(), MN_USER, msl, MN_USER);

			switch (MeshSelLevel())
			{
			case MNM_SL_VERTEX:
				mesh.getVerticesByFlag (mSelection, MN_USER);
				break;

			case MNM_SL_EDGE:
				mesh.getEdgesByFlag (mSelection, MN_USER);
				break;

			case MNM_SL_FACE:
				mesh.getFacesByFlag (mSelection, MN_USER);
				break;
			}
		}
		else
		{
			switch (MeshSelLevel())
			{
			case MNM_SL_VERTEX:
				mSelection.SetSize (mesh.numv);
				break;
			case MNM_SL_EDGE:
				mSelection.SetSize (mesh.nume);
				break;
			case MNM_SL_FACE:
				mSelection.SetSize (mesh.numf);
				break;
			}
			mSelection.SetAll ();
		}
	}

	if (UseSoftSelection())
	{
		float *softsel = mesh.getVSelectionWeights ();
		if (softsel) {
			mSoftSelection.SetCount (mesh.numv);
			memcpy (mSoftSelection.Addr(0), softsel, mesh.numv*sizeof(float));
		} else {
			mSoftSelection.SetCount (0);
		}
	}
}

void PolyOperation::SetUserFlags (MNMesh & mesh)
{
	int i, max = mSelection.GetSize ();
	if (max == 0) return;
	switch (MeshSelLevel())
	{
	case MNM_SL_VERTEX:
		if (max>mesh.numv) max = mesh.numv;
		for (i=0; i<max; i++)
			mesh.v[i].SetFlag(MN_EDITPOLY_OP_SELECT, mSelection[i]?true:false);
		for (i=max; i<mesh.numv; i++) mesh.v[i].ClearFlag (MN_EDITPOLY_OP_SELECT);
		break;

	case MNM_SL_EDGE:
		if (max>mesh.nume) max = mesh.nume;
		for (i=0; i<max; i++)
			mesh.e[i].SetFlag(MN_EDITPOLY_OP_SELECT, mSelection[i]?true:false);
		for (i=max; i<mesh.nume; i++) mesh.e[i].ClearFlag (MN_EDITPOLY_OP_SELECT);
		break;

	case MNM_SL_FACE:
		if (max>mesh.numf) max = mesh.numf;
		for (i=0; i<max; i++)
			mesh.f[i].SetFlag(MN_EDITPOLY_OP_SELECT, mSelection[i]?true:false);
		for (i=max; i<mesh.numf; i++) mesh.f[i].ClearFlag (MN_EDITPOLY_OP_SELECT);
		break;
	}
}

void PolyOperation::CopyBasics (PolyOperation *pCopyTo)
{
	pCopyTo->mSelection = mSelection;
	Tab<int> paramList;
	GetParameters (paramList);
	for (int i=0; i<paramList.Count (); i++) {
		int id = paramList[i];
		if ( Parameter(id) != NULL )
			memcpy (pCopyTo->Parameter(id), Parameter(id), ParameterSize(id));
	}
	if (Transform() != NULL)
		memcpy (pCopyTo->Transform(), Transform(), sizeof (Matrix3));
	if (ModContextTM() != NULL)
		memcpy (pCopyTo->ModContextTM(), ModContextTM(), sizeof(Matrix3));
	if (PreserveMapSettings() != NULL)
		*(pCopyTo->PreserveMapSettings()) = *(PreserveMapSettings());
}

void PolyOperation::MacroRecordBasics () {
	TSTR buf;
	switch (MeshSelLevel ()) {
		case MNM_SL_VERTEX:
			macroRecorder->Assign (_T("subobjectLevel"), mr_int, 1);
			macroRecorder->FunctionCall (_T("$.modifiers[#Edit_Poly].SetSelection"), 2, 0,
				mr_name, LookupMNMeshSelLevel(MNM_SL_VERTEX), mr_bitarray, &mSelection);
			break;
		case MNM_SL_EDGE:
			if (SelLevelSupport(EPM_SL_EDGE))
				macroRecorder->Assign (_T("subobjectLevel"), mr_int, 2);
			else
				macroRecorder->Assign (_T("subobjectLevel"), mr_int, 3);
			macroRecorder->FunctionCall (_T("$.modifiers[#Edit_Poly].SetSelection"), 2, 0,
				mr_name, LookupMNMeshSelLevel(MNM_SL_EDGE), mr_bitarray, &mSelection);
			break;
		case MNM_SL_FACE:
			if (SelLevelSupport(EPM_SL_FACE))
				macroRecorder->Assign (_T("subobjectLevel"), mr_int, 4);
			else
				macroRecorder->Assign (_T("subobjectLevel"), mr_int, 5);
			macroRecorder->FunctionCall (_T("$.modifiers[#Edit_Poly].SetSelection"), 2, 0,
				mr_name, LookupMNMeshSelLevel(MNM_SL_FACE), mr_bitarray, &mSelection);
			break;
	}

	Tab<int> paramList;
	GetParameters (paramList);
	for (int i=0; i<paramList.Count(); i++) {
		int pid = paramList[i];
		void *pVal = Parameter(pid);
		buf.printf ("$.modifiers[#Edit_Poly].%s", LookupParameterName (pid));
		switch (LookupParameterType (paramList[i])) {
			case TYPE_INT:
				macroRecorder->Assign (buf, mr_int, *((int *)pVal));
				break;
			case TYPE_FLOAT:
			case TYPE_WORLD:
				macroRecorder->Assign (buf, mr_float, *((float *)pVal));
				break;
			case TYPE_ANGLE:
				macroRecorder->Assign (buf, mr_float, (*((float *)pVal)) * 180.0f/PI);
				break;
			case TYPE_BOOL:
				macroRecorder->Assign (buf, mr_bool, *((int *)pVal));
				break;
			case TYPE_POINT3:
				macroRecorder->Assign (buf, mr_point3, ((Point3 *)pVal));
				break;
			default:
				buf.printf ("-- Unsupported parameter type: parameter %s", LookupParameterName(paramList[i]));
				macroRecorder->ScriptString (buf);
				break;
		}
	}
}

void PolyOperation::MacroRecord (LocalPolyOpData *pOpData) {
	MacroRecordBasics ();
	macroRecorder->FunctionCall (_T("$.modifiers[#Edit_Poly].ButtonOp"), 1, 0,
		mr_name, LookupOperationName (OpID()));
}

const USHORT kSelection = 0x400;
const USHORT kParameter = 0x410;
const USHORT kTransform = 0x420;
const USHORT kModContextTM = 0x430;
const USHORT kPreserveMapSettings = 0x440;

IOResult PolyOperation::SaveBasics (ISave *isave)
{
	isave->BeginChunk (kSelection);
	mSelection.Save (isave);
	isave->EndChunk ();

	ULONG nb;
	Tab<int> paramList;
	GetParameters (paramList);
	for (int i=0; i<paramList.Count (); i++)
	{
		int id = paramList[i];
		isave->BeginChunk (kParameter);
		isave->Write (&id, sizeof(int), &nb);
		isave->Write (Parameter(id), ParameterSize(id), &nb);
		isave->EndChunk ();
	}

	if (Transform() != NULL)
	{
		isave->BeginChunk (kTransform);
		Transform()->Save (isave);
		isave->EndChunk ();
	}

	if (ModContextTM() != NULL)
	{
		isave->BeginChunk (kModContextTM);
		ModContextTM()->Save (isave);
		isave->EndChunk ();
	}

	if (PreserveMapSettings() != NULL) {
		isave->BeginChunk (kPreserveMapSettings);
		PreserveMapSettings()->Save (isave);
		isave->EndChunk();
	}
	return IO_OK;
}

IOResult PolyOperation::LoadBasics (ILoad *iload) {
	IOResult res(IO_OK);
	ULONG nb(0);
	int id(0);
	void* prmtr = NULL;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID ())
		{
		case kSelection:
			res = mSelection.Load (iload);
			break;

		case kParameter:
			res = iload->Read (&id, sizeof(int), &nb);
			if (res != IO_OK) break;
			prmtr = Parameter(id);
			if (prmtr == NULL )
			{ 
				if ( id != epm_reserved1 && 
					id != epm_reserved2 && 
					id != epm_reserved3 && 
					id != epm_reserved4 &&
					id != epm_reserved5 ) 
					return IO_ERROR;
				else 
					// ignore those parameters. they are not read in : laurent R 06/05 
					// previous parameters are reset by the constructor. 
					break;
			}

			res = iload->Read (prmtr, ParameterSize(id), &nb);
			break;

		case kTransform:
			if (Transform() == NULL) return IO_ERROR;
			res = Transform()->Load (iload);
			break;

		case kModContextTM:
			if (ModContextTM() == NULL) return IO_ERROR;
			res = ModContextTM()->Load (iload);
			break;

		case kPreserveMapSettings:
			if (PreserveMapSettings() == NULL) return IO_ERROR;
			res = PreserveMapSettings()->Load (iload);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

void PolyOperation::GetValues (EPolyMod *epoly, TimeValue t, Interval & ivalid)
{
	IParamBlock2 *pBlock = epoly->getParamBlock ();
	Tab<int> paramList;
	GetParameters (paramList);
	for (int i=0; i<paramList.Count(); i++)
	{
		int id = paramList[i];
		void *prmtr = Parameter(id);
		if (prmtr == NULL) continue;
		ParamType2 type = pBlock->GetParameterType (id);

		int* iparam = NULL;
		float* fparam = NULL;
		Point3* pparam = NULL;
		Matrix3* mparam = NULL;

		switch (type)
		{
		case TYPE_INT:
		case TYPE_BOOL:
			iparam = (int *) prmtr;
			pBlock->GetValue (id, t, *iparam, ivalid);
			break;

		case TYPE_FLOAT:
		case TYPE_WORLD:
		case TYPE_ANGLE:
		case TYPE_INDEX:
			fparam = (float *)prmtr;
			pBlock->GetValue (id, t, *fparam, ivalid);
			break;

		case TYPE_POINT3:
			pparam = (Point3 *) prmtr;
			pBlock->GetValue (id, t, *pparam, ivalid);
			break;

		case TYPE_MATRIX3:
			mparam = (Matrix3 *) prmtr;
			pBlock->GetValue (id, t, *mparam, ivalid);
			break;
		}
	}

	if (Transform())
	{
		Transform()->IdentityMatrix ();
		Modifier *pMod = epoly->GetModifier();
		Control *ctrl = (Control *) pMod->GetReference (TransformReference());
		ctrl->GetValue (t, Transform(), ivalid, CTRL_RELATIVE);
	}

	if (PreserveMapSettings()) {
		*(PreserveMapSettings()) = epoly->GetPreserveMapSettings();
	}
}

const USHORT kPolyOperationBasics = 0x0600;

IOResult PolyOperation::Save (ISave *isave)
{
	isave->BeginChunk (kPolyOperationBasics);
	SaveBasics (isave);
	isave->EndChunk ();
	return IO_OK;
}

IOResult PolyOperation::Load (ILoad *iload)
{
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID ())
		{
		case kPolyOperationBasics:
			res = LoadBasics (iload);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

LocalCreateData::~LocalCreateData () {
	for (int i=0; i<mNewFaceVertex.Count(); i++)
	{
		if (mNewFaceVertex[i] == NULL) continue;
		delete [] mNewFaceVertex[i];
		mNewFaceVertex[i] = NULL;
	}
}

bool LocalCreateData::CreateFace (int *vertexArray, int degree) {
	mNewFaceDegree.Append (1, &degree);
	int *myVArray = new int[degree];
	memcpy (myVArray, vertexArray, degree*sizeof(int));
	mNewFaceVertex.Append (1, &myVArray);
	return true;
}

void LocalCreateData::RemoveLastFace ()
{
	int last = mNewFaceDegree.Count() - 1;
	mNewFaceDegree.Delete (last, 1);
	if (mNewFaceVertex[last]) delete[] mNewFaceVertex[last];
	mNewFaceVertex.Delete (last, 1);
}

LocalCreateData & LocalCreateData::operator= (const LocalCreateData & from)
{
	mNewVertex.SetCount (from.mNewVertex.Count());
	if (from.mNewVertex.Count()>0)
	{
		memcpy (mNewVertex.Addr(0), from.mNewVertex.Addr(0), from.mNewVertex.Count()*sizeof(Point3));
	}

	mNewFaceDegree.SetCount (from.mNewFaceDegree.Count());
	mNewFaceVertex.SetCount (from.mNewFaceDegree.Count());
	if (from.mNewFaceDegree.Count()>0)
	{
		memcpy (mNewFaceDegree.Addr(0), from.mNewFaceDegree.Addr(0), from.mNewFaceDegree.Count() * sizeof(int));
		for (int i=0; i<from.mNewFaceDegree.Count(); i++)
		{
			mNewFaceVertex[i] = new int[from.mNewFaceDegree[i]];
			memcpy (mNewFaceVertex[i], from.mNewFaceVertex[i], from.mNewFaceDegree[i]*sizeof(int));
		}
	}

	return (*this);
}

const USHORT kCreateVertex = 0x600;
const USHORT kCreateNumFaces = 0x608;
const USHORT kCreateFace = 0x610;

IOResult LocalCreateData::Save(ISave *isave) {
	ULONG nb;
	int num = mNewVertex.Count();
	if (num)
	{
		isave->BeginChunk (kCreateVertex);
		isave->Write (&num, sizeof(int), &nb);
		isave->Write (mNewVertex.Addr(0), num*sizeof(Point3), &nb);
		isave->EndChunk ();
	}

	num = mNewFaceDegree.Count();
	if (num)
	{
		isave->BeginChunk (kCreateNumFaces);
		isave->Write (&num, sizeof(int), &nb);
		isave->EndChunk ();

		for (int i=0; i<num; i++)
		{
			isave->BeginChunk (kCreateFace);
			isave->Write (&i, sizeof(int), &nb);
			isave->Write (mNewFaceDegree.Addr(i), sizeof(int), &nb);
			isave->Write (mNewFaceVertex[i], mNewFaceDegree[i]*sizeof(int), &nb);
			isave->EndChunk ();
		}
	}

	return IO_OK;
}

IOResult LocalCreateData::Load(ILoad *iload) {
	IOResult res;
	int num, faceID;
	ULONG nb;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID ())
		{
		case kCreateVertex:
			res = iload->Read (&num, sizeof(int), &nb);
			if (res != IO_OK) break;
			mNewVertex.SetCount (num);
			res = iload->Read (mNewVertex.Addr(0), num*sizeof(Point3), &nb);
			break;

		case kCreateNumFaces:
			res = iload->Read (&num, sizeof(int), &nb);
			if (res != IO_OK) break;
			mNewFaceDegree.SetCount (num);
			mNewFaceVertex.SetCount (num);
			break;

		case kCreateFace:
			res = iload->Read (&faceID, sizeof(int), &nb);
			if (res != IO_OK) break;
			res = iload->Read (mNewFaceDegree.Addr(faceID), sizeof(int), &nb);
			if (res != IO_OK) break;
			mNewFaceVertex[faceID] = new int[mNewFaceDegree[faceID]];
			res = iload->Read (mNewFaceVertex[faceID], mNewFaceDegree[faceID]*sizeof(int), &nb);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

class PolyOpCreate : public PolyOperation
{
public:
	PolyOpCreate () { }

	// Implement PolyOperation methods:
	int OpID () { return ep_op_create; }
	bool SelLevelSupport (int selLevel) { return (meshSelLevel[selLevel] != MNM_SL_EDGE); }
	int MeshSelLevel () { return MNM_SL_OBJECT; }

	TCHAR *Name () { return GetString (IDS_EDITPOLY_CREATE); }
	bool ChangesSelection () { return true; }

	bool CanAnimate () { return false; }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);

	PolyOperation *Clone ();
	void DeleteThis () { delete this; }
};

PolyOperation *PolyOpCreate::Clone ()
{
	PolyOpCreate *ret = new PolyOpCreate ();
	CopyBasics (ret);
	return ret;
}

bool PolyOpCreate::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	if (pOpData == NULL) return false;
	if (pOpData->OpID () != OpID()) return false;
	LocalCreateData *pCreateData = (LocalCreateData *) pOpData;

	bool ret=false;
	if (pCreateData->NumVertices()>0)
	{
		int oldNumv = mesh.numv;
		mesh.VAlloc (mesh.numv + pCreateData->NumVertices());
		for (int i=0; i<pCreateData->NumVertices(); i++) {
			int nv = mesh.NewVert (pCreateData->GetVertex (i));
			mesh.v[nv].SetFlag (MN_SEL);
		}
		if (oldNumv == 0) {
			// vedg and vfac arrays would have been null;
			// here we correct that by creating them.
			// They're trivially correct as all empty, because these vertices have no connectivity yet.
			mesh.VEdgeAlloc ();
			mesh.VFaceAlloc ();
		}
		ret = true;
	}

	if (pCreateData->NumFaces()>0)
	{
		mesh.FAlloc (mesh.numf + pCreateData->NumFaces());
		for (int i=0; i<pCreateData->NumFaces(); i++)
		{
			int deg = pCreateData->GetFaceDegree(i);
			int *vtx = pCreateData->GetFaceVertices (i);
         int j;
			for (j=0; j<deg; j++) {
				if ((vtx[j]<0) || (vtx[j]>=mesh.numv)) break;
			}
			if (j<deg) continue;
			int nf = mesh.CreateFace (deg, vtx);
			if (nf<0) {	// Try flipping the face over.
				int *w = new int[deg];
				for (int k=0; k<deg; k++) w[k] = vtx[deg-1-k];
				nf = mesh.CreateFace (deg, w);
				delete [] w;
			}
			if (nf>-1) {
				mesh.f[nf].SetFlag (MN_SEL);
				ret = true;
			}
		}
	}

	return ret;
}

class PolyOpCollapseVertices : public PolyOperation
{
public:
	int OpID () { return ep_op_collapse_vertex; }
	bool SelLevelSupport (int selLevel) { return selLevel == EPM_SL_VERTEX; }
	int MeshSelLevel () { return MNM_SL_VERTEX; }

	TCHAR *Name () { return GetString (IDS_EDITPOLY_COLLAPSE_VERTEX); }
	// No Dialog ID
	// No parameters
	bool CanDelete () { return true; }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpCollapseVertices *ret = new PolyOpCollapseVertices ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpCollapseVertices::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	mesh.ClearEFlags (MN_USER);
	// Flag all edges which have both ends flagged.
	mesh.PropegateComponentFlags (MNM_SL_EDGE, MN_USER, MNM_SL_VERTEX, MN_EDITPOLY_OP_SELECT, true);
	MNMeshUtilities mmu(&mesh);
	bool ret = mmu.CollapseEdges (MN_USER);

	// If no collapsing occurred, try a weld of selected.
	if (!ret) ret = mesh.WeldBorderVerts (999999.0f, MN_EDITPOLY_OP_SELECT);

	return ret;
}

class PolyOpCollapseEdges : public PolyOperation
{
public:
	int OpID () { return ep_op_collapse_edge; }
	bool SelLevelSupport (int selLevel) { return meshSelLevel[selLevel] == MNM_SL_EDGE; }
	int MeshSelLevel () { return MNM_SL_EDGE; }

	TCHAR *Name () { return GetString (IDS_EDITPOLY_COLLAPSE_EDGE); }
	// No Dialog ID
	// No parameters
	bool CanDelete () { return true; }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData) { MNMeshUtilities mmu(&mesh); return mmu.CollapseEdges (MN_EDITPOLY_OP_SELECT); }
	PolyOperation *Clone () {
		PolyOpCollapseEdges *ret = new PolyOpCollapseEdges ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

class PolyOpCollapseFaces : public PolyOperation
{
public:
	int OpID () { return ep_op_collapse_face; }
	bool SelLevelSupport (int selLevel) { return meshSelLevel[selLevel] == MNM_SL_FACE; }
	int MeshSelLevel () { return MNM_SL_FACE; }

	TCHAR *Name () { return GetString (IDS_EDITPOLY_COLLAPSE_FACE); }
	// No Dialog ID
	// No parameters
	bool CanDelete () { return true; }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpCollapseFaces *ret = new PolyOpCollapseFaces ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpCollapseFaces::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	mesh.ClearEFlags (MN_USER);
	mesh.PropegateComponentFlags (MNM_SL_EDGE, MN_USER, MNM_SL_FACE, MN_EDITPOLY_OP_SELECT);
	MNMeshUtilities mmu(&mesh);
	return mmu.CollapseEdges (MN_USER);
}

void LocalAttachData::AppendNode (INode *attachNode, EPolyMod *pMod, INode *myNode, TimeValue t,
								  int mapping, bool condense) {
	condense = condense && (mapping == ATTACHMAT_IDTOMAT);

	MNMesh *myMesh = NULL;	
	if (mapping == ATTACHMAT_MATTOID) {
		// Get this before we change anything...
		myMesh = pMod->EpModGetOutputMesh (myNode);
		if (myMesh == NULL) mapping = ATTACHMAT_NEITHER;	// shouldn't happen.
	}

	// Get the attach object
	bool del = false;
	PolyObject *obj = NULL;
	ObjectState os = attachNode->GetObjectRef()->Eval(t);
	if (os.obj->IsSubClassOf(polyObjectClassID)) obj = (PolyObject *) os.obj;
	else {
		if (!os.obj->CanConvertToType(polyObjectClassID)) return;
		obj = (PolyObject*)os.obj->ConvertToType(t,polyObjectClassID);
		if (obj!=os.obj) del = true;
	}
	// Get the mesh:
	MNMesh *attachMesh = new MNMesh(obj->GetMesh());
	if (del) delete obj;

	// Combine the materials of the two nodes.
	Mtl *m1 = myNode->GetMtl();
	Mtl *m2 = attachNode->GetMtl();

	int matLimit = 0;
	if (m1 && m2 && (m1 != m2)) {
		if (mapping==ATTACHMAT_IDTOMAT) {
			// We'll be changing the material IDs of each mesh.
			// This is just a matter of limiting (via the % modulo operator) the material ID range of each.
			matLimit = m1->IsMultiMtl() ? m1->NumSubMtls() : 1;
			FitPolyMeshIDsToMaterial (*attachMesh, m2);
		}

		// TODO: Consider - is this necessary?
		theHold.Suspend ();

		if (mapping==ATTACHMAT_MATTOID) {
			m1 = FitMaterialToPolyMeshIDs (*myMesh, m1);
			m2 = FitMaterialToPolyMeshIDs (*attachMesh, m2);
		}

		int matOffset;
		Mtl *multi = CombineMaterials (m1, m2, matOffset);
		if ((mapping != ATTACHMAT_NEITHER) && matOffset) {
			for (int i=0; i<attachMesh->numf; i++) attachMesh->f[i].material += matOffset;
		}

		theHold.Resume ();

		// We can't be in face subobject mode, else we screw up the materials:
		int oldSL = pMod->GetEPolySelLevel();
		if (meshSelLevel[oldSL] == MNM_SL_FACE) pMod->SetEPolySelLevel (EPM_SL_OBJECT);
		myNode->SetMtl(multi);
		if (meshSelLevel[oldSL] == MNM_SL_FACE) pMod->SetEPolySelLevel (oldSL);
		m1 = multi;
	}

	if (!m1 && m2) {
		// Attached node has material, but we don't.  Get that material for us.
		// (This material operation seems undo-safe.)

		// We can't be in face subobject mode, else we screw up the materials:
		int oldSL = pMod->GetEPolySelLevel();
		if (meshSelLevel[oldSL] == MNM_SL_FACE) pMod->SetEPolySelLevel (EPM_SL_OBJECT);
		myNode->SetMtl (m2);
		if (meshSelLevel[oldSL] == MNM_SL_FACE) pMod->SetEPolySelLevel (oldSL);
		m1 = m2;
	}

	// Construct a transformation that takes a vertex out of the space of
	// the source node and puts it into the space of the destination node.
	Matrix3 relativeTransform = attachNode->GetObjectTM(t) * Inverse(myNode->GetObjectTM(t));
	attachMesh->Transform (relativeTransform);

	mMyMatLimit.Append (1, &matLimit);
	mMesh.Append (1, &attachMesh);
}

MNMesh *LocalAttachData::PopMesh (int & matLimit) {
	int last = mMesh.Count()-1;
	MNMesh *ret = mMesh[last];
	matLimit = mMyMatLimit[last];
	mMesh[last] = NULL;
	mMesh.Delete (last, 1);
	mMyMatLimit.Delete (last, 1);
	return ret;
}

void LocalAttachData::PushMesh (MNMesh *pMesh, int matLimit) {
	mMesh.Append (1, &pMesh);
	mMyMatLimit.Append (1, &matLimit);
}

const USHORT kAttachMesh = 0x0e00;
const USHORT kAttachNum = 0x0e04;
const USHORT kAttachWhich = 0x0e08;
const USHORT kAttachMyMatLimit = 0x0e0c;

IOResult LocalAttachData::Save (ISave *isave)
{
	ULONG nb;
	int num = mMesh.Count();
	isave->BeginChunk (kAttachNum);
	isave->Write (&num, sizeof(int), &nb);
	isave->EndChunk ();

	for (int i=0; i<num; i++) {
		if (mMesh[i] == NULL) continue;

		isave->BeginChunk (kAttachWhich);
		isave->Write (&i, sizeof(int), &nb);
		isave->EndChunk ();

		isave->BeginChunk (kAttachMesh);
		mMesh[i]->Save (isave);
		isave->EndChunk ();
	}

	if (num>0) {
		isave->BeginChunk (kAttachMyMatLimit);
		isave->Write (mMyMatLimit.Addr(0), num*sizeof(int), &nb);
		isave->EndChunk();
	}

	return IO_OK;
}

IOResult LocalAttachData::Load (ILoad *iload)
{
	ULONG nb(0);
	int num = 0, which = 0, i;

	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID ())
		{
		case kAttachNum:
			res = iload->Read (&num, sizeof(int), &nb);
			if (res != IO_OK) break;
			mMesh.SetCount (num);
			for (i=0; i<num; i++) mMesh[i] = NULL;
			break;

		case kAttachWhich:
			res = iload->Read (&which, sizeof(int), &nb);
			break;

		case kAttachMesh:
			mMesh[which] = new MNMesh();
			res = mMesh[which]->Load (iload);
			break;

		case kAttachMyMatLimit:
			mMyMatLimit.SetCount(num);
			res = iload->Read (mMyMatLimit.Addr(0), num*sizeof(int), &nb);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

void LocalAttachData::RescaleWorldUnits (float f) {
	for (int i=0; i<mMesh.Count(); i++) {
		if (!mMesh[i]) continue;
		MNVert *mv = mMesh[i]->v;
		if (!mv) continue;
		int numv = mMesh[i]->numv;
		for (int j=0; j<numv; j++) mv[j].p *= f;
	}
}

// Have to figure out how to keep track of as many nodes as are required
// for either single-attach or multi-attach.
class PolyOpAttach : public PolyOperation
{
public:
	int OpID () { return ep_op_attach; }
	bool SelLevelSupport (int selLevel) { return true; }
	int MeshSelLevel () { return MNM_SL_OBJECT; }

	TCHAR *Name () { return GetString (IDS_ATTACH); }

	// No parameters - all data stored in LocalAttachData.
	bool ChangesSelection () { return true; }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpAttach *ret = new PolyOpAttach ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpAttach::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	if (!pOpData || (pOpData->OpID() != OpID())) return false;
	LocalAttachData *pAttach = (LocalAttachData *) pOpData;

	int ovnum = mesh.numv;

	for (int i=0; i<pAttach->NumMeshes(); i++) {
		int matLimit = pAttach->GetMaterialLimit(i);
		if (matLimit) {
			for (int j=0; j<mesh.numf; j++) {
				mesh.f[j].material = mesh.f[j].material%matLimit;
			}
		}
		mesh += pAttach->GetMesh(i);
	}

	if (ovnum == 0) mesh.FillInVertEdgesFaces ();

	return true;
}

class PolyOpDetachVertex : public PolyOperation
{
private:
	bool mClone;

public:
	PolyOpDetachVertex () : mClone(false) { }

	int OpID () { return ep_op_detach_vertex; }
	bool SelLevelSupport (int selLevel) { return (selLevel == EPM_SL_VERTEX); }
	int MeshSelLevel () { return MNM_SL_VERTEX; }

	TCHAR *Name () { return GetString (IDS_EDITPOLY_DETACH_VERTEX); }

	// No parameters, because "Detach as clone" is not a parameter,
	// it's a static variable of the EditPolyMod class.  So we need special
	// case code to get it in GetValues, to copy it in Clone, and to save/load it.
	void GetValues (EPolyMod *epoly, TimeValue t, Interval & ivalid) {
		PolyOperation::GetValues (epoly, t, ivalid);
		mClone = ((EditPolyMod *)epoly)->GetDetachAsClone();
	}
	bool ChangesSelection () { return mClone; }

	IOResult Save (ISave *isave);
	IOResult Load (ILoad *iload);

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpDetachVertex *ret = new PolyOpDetachVertex ();
		CopyBasics (ret);
		ret->mClone = mClone;
		return ret;
	}
	void DeleteThis () { delete this; }
};

const USHORT kDetachClone = 0x0601;

IOResult PolyOpDetachVertex::Save (ISave *isave) {
	isave->BeginChunk (kPolyOperationBasics);
	SaveBasics (isave);
	isave->EndChunk ();
	ULONG nb;
	isave->BeginChunk (kDetachClone);
	isave->Write (&mClone, sizeof(bool), &nb);
	isave->EndChunk ();
	return IO_OK;
}

IOResult PolyOpDetachVertex::Load (ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID ())
		{
		case kPolyOperationBasics:
			res = LoadBasics (iload);
			break;
		case kDetachClone:
			res = iload->Read (&mClone, sizeof(bool), &nb);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

bool PolyOpDetachVertex::Do (MNMesh & mesh, LocalPolyOpData *pOpdata) {
	// We have to do things by faces, but we have a special case for cloning isolated vertices.
	int nf = mesh.PropegateComponentFlags (MNM_SL_FACE, MN_USER, MNM_SL_VERTEX, MN_EDITPOLY_OP_SELECT);
	if (!mClone) return mesh.DetachFaces (MN_USER);

	int oldnumv = mesh.numv;
	mesh.CloneFaces (MN_USER);
	// Handle isolated vertex cloning:
	int numIsoVertsToClone=0;
   int i;
	for (i=0; i<oldnumv; i++) {
		if (!mesh.v[i].GetFlag (MN_EDITPOLY_OP_SELECT)) continue;
		mesh.v[i].ClearFlag (MN_SEL);
		if (mesh.vedg[i].Count()>0) continue;
		numIsoVertsToClone++;
	}
	for (i=oldnumv; i<mesh.numv; i++) mesh.v[i].SetFlag (MN_SEL);
	if (numIsoVertsToClone == 0) return (nf>0);

	mesh.setNumVerts (mesh.numv + numIsoVertsToClone);
	int newVert = mesh.numv;
	for (i=0; i<oldnumv; i++) {
		if (!mesh.v[i].GetFlag (MN_EDITPOLY_OP_SELECT)) continue;
		if (mesh.vedg[i].Count()>0) continue;
		mesh.v[newVert] = mesh.v[i];
		mesh.v[newVert].SetFlag (MN_SEL);
		newVert++;
	}
	return true;
}

class PolyOpDetachFace : public PolyOperation
{
private:
	bool mClone;

public:
	PolyOpDetachFace () : mClone(false) { }

	int OpID () { return ep_op_detach_face; }
	bool SelLevelSupport (int selLevel) { return selLevel>EPM_SL_VERTEX; }
	int MeshSelLevel () { return MNM_SL_FACE; }

	TCHAR *Name () { return GetString (IDS_EDITPOLY_DETACH_FACE); }

	// No parameters, because "Detach as clone" is not a parameter,
	// it's a static variable of the EditPolyMod class.  So we need special
	// case code to get it in GetValues, to copy it in Clone, and to save/load it.
	void GetValues (EPolyMod *epoly, TimeValue t, Interval & ivalid) {
		PolyOperation::GetValues (epoly, t, ivalid);
		mClone = ((EditPolyMod *)epoly)->GetDetachAsClone();
	}
	bool ChangesSelection () { return mClone; }

	IOResult Save (ISave *isave);
	IOResult Load (ILoad *iload);

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpDetachFace *ret = new PolyOpDetachFace ();
		CopyBasics (ret);
		ret->mClone = mClone;
		return ret;
	}
	void DeleteThis () { delete this; }
};

IOResult PolyOpDetachFace::Save (ISave *isave) {
	isave->BeginChunk (kPolyOperationBasics);
	SaveBasics (isave);
	isave->EndChunk ();
	ULONG nb;
	isave->BeginChunk (kDetachClone);
	isave->Write (&mClone, sizeof(bool), &nb);
	isave->EndChunk ();
	return IO_OK;
}

IOResult PolyOpDetachFace::Load (ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID ())
		{
		case kPolyOperationBasics:
			res = LoadBasics (iload);
			break;
		case kDetachClone:
			res = iload->Read (&mClone, sizeof(bool), &nb);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

bool PolyOpDetachFace::Do (MNMesh & mesh, LocalPolyOpData *pOpdata) {
	if (mClone) {
		int numf = mesh.numf;
		mesh.CloneFaces (MN_EDITPOLY_OP_SELECT);
		if (mesh.numf == numf) return false;
		for (int i=0; i<numf; i++) mesh.f[i].ClearFlag (MN_SEL);
		for (int i=numf; i<mesh.numf; i++) mesh.f[i].SetFlag (MN_SEL);
		return true;
	}
	else return mesh.DetachFaces (MN_EDITPOLY_OP_SELECT);
}

class PolyOpDeleteVertex : public PolyOperation
{
private:
	int mDeleteIsolatedVertices;
public:
	int OpID() { return ep_op_delete_vertex; }
	bool SelLevelSupport (int selLevel) { return (selLevel==EPM_SL_VERTEX); }
	int MeshSelLevel () { return MNM_SL_VERTEX; }
	TCHAR *Name () { return GetString (IDS_EDITPOLY_DELETE_VERTEX); }

	// No dialog ID.  (parameter in main UI.)
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (1);
		paramList[0] = epm_delete_isolated_verts;
	}
	void *Parameter (int paramID) {
		switch (paramID)
		{
		case epm_delete_isolated_verts: return &mDeleteIsolatedVertices;
		}
		return NULL;
	}
	bool CanDelete () { return true; }

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpDeleteVertex *ret = new PolyOpDeleteVertex ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpDeleteVertex::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	bool ret = false;
	int i;
	int numDelFaces = mesh.PropegateComponentFlags (MNM_SL_FACE, MN_DEAD, MNM_SL_VERTEX, MN_EDITPOLY_OP_SELECT);
	if (numDelFaces>0)
	{
		ret = true;
		// Delete or correct edges on deleted faces.
		for (i=0; i<mesh.nume; i++) {
			if (mesh.e[i].GetFlag (MN_DEAD)) continue;
			int f1 = mesh.e[i].f1;
			int f2 = mesh.e[i].f2;
			bool f1dead = mesh.f[f1].GetFlag (MN_DEAD);
			bool f2dead = (f2<0) || mesh.f[f2].GetFlag (MN_DEAD);
			if (!f1dead && !f2dead) continue;
			if (f1dead && f2dead) mesh.e[i].SetFlag (MN_DEAD);
			else {
				if (f1dead) mesh.e[i].Invert();
				mesh.e[i].f2 = -1;
			}
		}
	}

	// Correct the vertex records to remove dead faces and edges,
	// and also delete selected or isolated vertices if appropriate.
	for (i=0; i<mesh.numv; i++) {
		if (mesh.v[i].GetFlag (MN_DEAD)) continue;
		if (mesh.v[i].GetFlag (MN_EDITPOLY_OP_SELECT))
		{
			mesh.v[i].SetFlag (MN_DEAD);
			ret = true;
			continue;
		}

		// Otherwise, examine edges and faces that may have gone away.
		Tab<int> & vf = mesh.vfac[i];
		Tab<int> & ve = mesh.vedg[i];
		int j;
		if (vf.Count ()) {
			for (j=vf.Count()-1; j>=0; j--) {
				if (mesh.f[vf[j]].GetFlag (MN_DEAD)) vf.Delete (j,1);
			}
			if (!vf.Count() && mDeleteIsolatedVertices) {
				// Delete this newly isolated vertex.
				mesh.v[i].SetFlag (MN_DEAD);
				continue;
			}
		}
		for (j=ve.Count()-1; j>=0; j--) {
			if (mesh.e[ve[j]].GetFlag (MN_DEAD)) ve.Delete (j, 1);
		}
	}

	// This operation can create 2-boundary vertices:
	mesh.ClearFlag (MN_MESH_NO_BAD_VERTS);

	return ret;
}

class PolyOpDeleteFace : public PolyOperation
{
private:
	int mDeleteIsolatedVertices;
public:
	int OpID() { return ep_op_delete_face; }
	bool SelLevelSupport (int selLevel) {
		return (meshSelLevel[selLevel] == MNM_SL_FACE);
	}
	int MeshSelLevel () { return MNM_SL_FACE; }
	TCHAR *Name () { return GetString (IDS_EDITPOLY_DELETE_FACE); }

	//int DialogID () { return IDD_SETTINGS_DELETE; }
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (1);
		paramList[0] = epm_delete_isolated_verts;
	}
	void *Parameter (int paramID) {
		switch (paramID)
		{
		case epm_delete_isolated_verts: return &mDeleteIsolatedVertices;
		}
		return NULL;
	}
	bool CanDelete () { return true; }

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpDeleteFace *ret = new PolyOpDeleteFace ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpDeleteFace::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	int i;
	int numDelFaces = mesh.PropegateComponentFlags (MNM_SL_FACE, MN_DEAD, MNM_SL_FACE, MN_EDITPOLY_OP_SELECT);
	if (numDelFaces==0) return false;

	// Delete or correct edges on deleted faces.
	for (i=0; i<mesh.nume; i++) {
		if (mesh.e[i].GetFlag (MN_DEAD)) continue;
		int f1 = mesh.e[i].f1;
		int f2 = mesh.e[i].f2;
		bool f1dead = mesh.f[f1].GetFlag (MN_DEAD);
		bool f2dead = (f2<0) || mesh.f[f2].GetFlag (MN_DEAD);
		if (!f1dead && !f2dead) continue;
		if (f1dead && f2dead) mesh.e[i].SetFlag (MN_DEAD);
		else {
			if (f1dead)
			{
				// f1 is dead, but not f2.
				// Flip the edge.
				mesh.e[i].Invert();
			}
			mesh.e[i].f2 = -1;
		}
	}

	// Correct the vertex records to remove dead faces and edges,
	// and also delete selected or isolated vertices if appropriate.
	for (i=0; i<mesh.numv; i++) {
		if (mesh.v[i].GetFlag (MN_DEAD)) continue;

		// Otherwise, examine edges and faces that may have gone away.
		Tab<int> & vf = mesh.vfac[i];
		Tab<int> & ve = mesh.vedg[i];
		int j;
		if (vf.Count ()) {
			for (j=vf.Count()-1; j>=0; j--) {
				if (mesh.f[vf[j]].GetFlag (MN_DEAD)) vf.Delete (j,1);
			}
			if (!vf.Count() && mDeleteIsolatedVertices) {
				// Delete this newly isolated vertex.
				mesh.v[i].SetFlag (MN_DEAD);
				continue;
			}
		}
		for (j=ve.Count()-1; j>=0; j--) {
			if (mesh.e[ve[j]].GetFlag (MN_DEAD)) ve.Delete (j, 1);
		}
	}

	// This operation can create 2-boundary vertices:
	mesh.ClearFlag (MN_MESH_NO_BAD_VERTS);

	return true;
}

class PolyOpDeleteEdge : public PolyOperation
{
private:
	int mDeleteIsolatedVertices;
public:
	int OpID() { return ep_op_delete_edge; }
	bool SelLevelSupport (int selLevel) {
		return (meshSelLevel[selLevel] == MNM_SL_EDGE);
	}
	int MeshSelLevel () { return MNM_SL_EDGE; }
	TCHAR *Name () { return GetString (IDS_EDITPOLY_DELETE_EDGE); }

	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (1);
		paramList[0] = epm_delete_isolated_verts;
	}
	void *Parameter (int paramID) {
		switch (paramID)
		{
		case epm_delete_isolated_verts: return &mDeleteIsolatedVertices;
		}
		return NULL;
	}
	bool CanDelete () { return true; }

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpDeleteEdge *ret = new PolyOpDeleteEdge ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpDeleteEdge::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	int i;
	int numDelFaces = mesh.PropegateComponentFlags (MNM_SL_FACE, MN_DEAD, MNM_SL_EDGE, MN_EDITPOLY_OP_SELECT);
	if (numDelFaces==0) return false;

	// Delete or correct edges on deleted faces.
	for (i=0; i<mesh.nume; i++) {
		if (mesh.e[i].GetFlag (MN_DEAD)) continue;
		int f1 = mesh.e[i].f1;
		int f2 = mesh.e[i].f2;
		bool f1dead = mesh.f[f1].GetFlag (MN_DEAD);
		bool f2dead = (f2<0) || mesh.f[f2].GetFlag (MN_DEAD);
		if (!f1dead && !f2dead) continue;
		if (f1dead && f2dead) mesh.e[i].SetFlag (MN_DEAD);
		else {
			if (f1dead)
			{
				// f1 is dead, but not f2.
				// Flip the edge.
				mesh.e[i].Invert();
			}
			mesh.e[i].f2 = -1;
		}
	}

	// Correct the vertex records to remove dead faces and edges,
	// and also delete selected or isolated vertices if appropriate.
	for (i=0; i<mesh.numv; i++) {
		if (mesh.v[i].GetFlag (MN_DEAD)) continue;

		// Otherwise, examine edges and faces that may have gone away.
		Tab<int> & vf = mesh.vfac[i];
		Tab<int> & ve = mesh.vedg[i];
		int j;
		if (vf.Count ()) {
			for (j=vf.Count()-1; j>=0; j--) {
				if (mesh.f[vf[j]].GetFlag (MN_DEAD)) vf.Delete (j,1);
			}
			if (!vf.Count() && mDeleteIsolatedVertices) {
				// Delete this newly isolated vertex.
				mesh.v[i].SetFlag (MN_DEAD);
				continue;
			}
		}
		for (j=ve.Count()-1; j>=0; j--) {
			if (mesh.e[ve[j]].GetFlag (MN_DEAD)) ve.Delete (j, 1);
		}
	}

	// This operation can create 2-boundary vertices:
	mesh.ClearFlag (MN_MESH_NO_BAD_VERTS);

	return true;
}

class PolyOpSlice : public PolyOperation
{
	Matrix3 mPlane, mMCTM;
	int mSplit;

public:
	PolyOpSlice () : mPlane(true), mMCTM(true), mSplit(false) { }

	// Implement PolyOperation methods:
	int OpID () { return ep_op_slice; }
	bool SelLevelSupport (int selLevel) { return meshSelLevel[selLevel] != MNM_SL_FACE; }
	int MeshSelLevel () { return MNM_SL_OBJECT; }

	TCHAR *Name() { return GetString (IDS_EDITPOLY_SLICE); }
	// No dialog (split setting in is main UI)
	bool ChangesSelection()  { return true; }

	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (1);
		paramList[0] = epm_split;
	}
	void *Parameter (int paramID) {
		switch (paramID)
		{
		case epm_split: return &mSplit;
		}
		return NULL;
	}
	
	int TransformReference () { return EDIT_SLICEPLANE_REF; }
	Matrix3 *Transform () { return &mPlane; }
	Matrix3 *ModContextTM () { return &mMCTM; }
	void RescaleWorldUnits (float f) { mPlane.SetTrans (mPlane.GetTrans ()*f); mMCTM.SetTrans (mMCTM.GetTrans()*f); }

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpSlice *ret = new PolyOpSlice ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpSlice::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	Point3 normal = Normalize (mMCTM.VectorTransform (mPlane.VectorTransform (Point3(0,0,1))));
	float offset = DotProd (normal, mMCTM * mPlane.GetTrans());
	return mesh.Slice (normal, offset, MNEPS, mSplit!=0, false);
}

class PolyOpSliceFaces : public PolyOperation
{
	Matrix3 mPlane, mMCTM;
	int mSplit;

public:
	PolyOpSliceFaces () : mPlane(true), mMCTM(true), mSplit(false) { }

	// Implement PolyOperation methods:
	int OpID () { return ep_op_slice_face; }
	bool SelLevelSupport (int selLevel) { return meshSelLevel[selLevel] == MNM_SL_FACE; }
	int MeshSelLevel () { return MNM_SL_FACE; }

	TCHAR *Name() { return GetString (IDS_EDITPOLY_SLICE_FACES); }
	// No dialog (split setting in is main UI)
	bool ChangesSelection()  { return true; }

	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (1);
		paramList[0] = epm_split;
	}
	void *Parameter (int paramID) {
		switch (paramID)
		{
		case epm_split: return &mSplit;
		}
		return NULL;
	}
	
	int TransformReference () { return EDIT_SLICEPLANE_REF; }
	Matrix3 *Transform () { return &mPlane; }
	Matrix3 *ModContextTM () { return &mMCTM; }
	void RescaleWorldUnits (float f) { mPlane.SetTrans (mPlane.GetTrans ()*f); mMCTM.SetTrans (mMCTM.GetTrans()*f); }

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpSliceFaces *ret = new PolyOpSliceFaces ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpSliceFaces::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	Point3 normal = Normalize (mMCTM.VectorTransform (mPlane.VectorTransform (Point3(0,0,1))));
	float offset = DotProd (normal, mMCTM * mPlane.GetTrans());
	return mesh.Slice (normal, offset, MNEPS, mSplit!=0, false, true, MN_EDITPOLY_OP_SELECT);
}

void LocalCutData::AddCut (int startLevel, int startIndex, Point3 & start, Point3 & normal)
{
	mStartLevel.Append (1, &startLevel);
	mStartIndex.Append (1, &startIndex);
	mStart.Append (1, &start);
	mEnd.Append (1, &start);
	mNormal.Append (1, &normal);
}

void LocalCutData::RemoveLast ()
{
	int last = mStartLevel.Count()-1;
	if (last<0) return;
	mStartLevel.Delete (last, 1);
	mStartIndex.Delete (last, 1);
	mStart.Delete (last, 1);
	mEnd.Delete (last, 1);
	mNormal.Delete (last, 1);
}

void LocalCutData::RescaleWorldUnits (float f) {
	for (int i=0; i<mStart.Count(); i++)
	{
		mStart[i] *= f;
		mEnd[i] *= f;
	}
}

const USHORT kCutInfo = 0x840;

IOResult LocalCutData::Save(ISave *isave) {
	ULONG nb;
	int num = mStartLevel.Count();
	if (num)
	{
		isave->BeginChunk (kCutInfo);
		isave->Write (&num, sizeof(int), &nb);
		isave->Write (mStartLevel.Addr(0), num*sizeof(int), &nb);
		isave->Write (mStartIndex.Addr(0), num*sizeof(int), &nb);
		isave->Write (mStart.Addr(0), num*sizeof(Point3), &nb);
		isave->Write (mEnd.Addr(0), num*sizeof(Point3), &nb);
		isave->Write (mNormal.Addr(0), num*sizeof(Point3), &nb);
		isave->EndChunk ();
	}

	return IO_OK;
}

IOResult LocalCutData::Load(ILoad *iload) {
	IOResult res;
	int num;
	ULONG nb;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID ())
		{
		case kCutInfo:
			res = iload->Read (&num, sizeof(int), &nb);
			if (res != IO_OK) break;
			mStartLevel.SetCount (num);
			mStartIndex.SetCount (num);
			mStart.SetCount (num);
			mEnd.SetCount (num);
			mNormal.SetCount (num);
			if ((res = iload->Read (mStartLevel.Addr(0), num*sizeof(int), &nb)) != IO_OK) break;
			if ((res = iload->Read (mStartIndex.Addr(0), num*sizeof(int), &nb)) != IO_OK) break;
			if ((res = iload->Read (mStart.Addr(0), num*sizeof(Point3), &nb)) != IO_OK) break;
			if ((res = iload->Read (mEnd.Addr(0), num*sizeof(Point3), &nb)) != IO_OK) break;
			if ((res = iload->Read (mNormal.Addr(0), num*sizeof(Point3), &nb)) != IO_OK) break;
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

class PolyOpCut : public PolyOperation
{
	int mSplit;

public:
	PolyOpCut () : mSplit(false) { }

	// Implement PolyOperation methods:
	int OpID () { return ep_op_cut; }
	bool SelLevelSupport (int selLevel) { return true; }
	int MeshSelLevel () { return MNM_SL_OBJECT; }

	TCHAR *Name() { return GetString (IDS_CUT); }
	// No dialog (split setting in is main UI)
	bool ChangesSelection()  { return true; }

	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (1);
		paramList[0] = epm_split;
	}
	void *Parameter (int paramID) {
		switch (paramID)
		{
		case epm_split: return &mSplit;
		}
		return NULL;
	}

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpCut *ret = new PolyOpCut ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

// NOTE: copied over from MNMesh::Cut code (and from Editable Poly code).
const float kCUT_EPS = .001f;

bool PolyOpCut::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	if (!pOpData || pOpData->OpID() != OpID()) return false;

	LocalCutData *pData = (LocalCutData *)pOpData;

	MNMeshUtilities mmu(&mesh);
	mmu.CutPrepare();

	int endVertex = -1;
	for (int i=0; i<pData->NumCuts(); i++)
	{
		int startIndex = pData->StartIndex(i);
		int startLevel = pData->StartLevel(i);
		Point3 start = pData->Start(i);
		Point3 end = pData->End(i);
		Point3 normal = pData->Normal(i);

		Point3 edgeDirection;
		Point3 startDirection;

		switch (startLevel) {
		case MNM_SL_OBJECT:
			continue;

		case MNM_SL_VERTEX:
			if (startIndex>=mesh.numv) continue;
			endVertex = mesh.Cut (startIndex, end, normal, mSplit?true:false);
			break;

		case MNM_SL_EDGE:
			if (startIndex >= mesh.nume) continue;
			// Find proportion along edge for starting point:
			float prop;
			edgeDirection = mesh.v[mesh.e[startIndex].v2].p - mesh.v[mesh.e[startIndex].v1].p;
			float edgeLen2;
			edgeLen2 = LengthSquared (edgeDirection);
			startDirection = start - mesh.v[mesh.e[startIndex].v1].p;
			if (edgeLen2) prop = DotProd (startDirection, edgeDirection) / edgeLen2;
			else prop = 0.0f;
			int nv;
			if (prop < kCUT_EPS) nv = mesh.e[startIndex].v1;
			else {
				if (prop > 1-kCUT_EPS) nv = mesh.e[startIndex].v2;
				else {
					// If we need to cut the initial edge, do so before starting main cut routine:
					nv = mesh.SplitEdge (startIndex, prop);
				}
			}
			endVertex = mesh.Cut (nv, end, normal, mSplit?true:false);
			break;

		case MNM_SL_FACE:
			if (startIndex >= mesh.numf) continue;

			endVertex = mesh.CutFace (startIndex, start, end, normal, mSplit?true:false);
			break;
		}
	}

	mmu.CutCleanup ();

	pData->SetEndVertex (endVertex);

	return true;
}

class PolyOpMeshSmooth : public PolyOperation
{
private:
	float mSmoothness;
	int mSepBySmooth, mSepByMat;
	bool  mGripShown;
	MSmoothGrip  *mMSmoothGrip;

public:
	PolyOpMeshSmooth () : mSmoothness(0.0f), mSepBySmooth(0), mSepByMat(0),mGripShown(false),mMSmoothGrip(NULL) { }

	// Implement PolyOperation methods:
	int OpID () { return ep_op_meshsmooth; }
	bool SelLevelSupport (int selLevel) { return true; }
	int MeshSelLevel () { return MNM_SL_VERTEX; }

	TCHAR *Name () { return GetString (IDS_EDITPOLY_MESHSMOOTH); }
	int DialogID () { return IDD_SETTINGS_MESHSMOOTH; }
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (3);
		paramList[0] = epm_ms_smoothness;
		paramList[1] = epm_ms_sep_smooth;
		paramList[2] = epm_ms_sep_mat;
	}
	void *Parameter (int paramID) {
		switch (paramID)
		{
		case epm_ms_smoothness: return &mSmoothness;
		case epm_ms_sep_smooth: return &mSepBySmooth;
		case epm_ms_sep_mat: return &mSepByMat;
		}
		return NULL;
	}

	bool ChangesSelection () { return true; }
	void SetUserFlags (MNMesh & mesh);
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);

	PolyOperation *Clone () {
		PolyOpMeshSmooth *ret = new PolyOpMeshSmooth ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }

	bool HasGrip() { return true; }
	bool IsGripShown();
	void ShowGrip( EditPolyMod *p );
	void HideGrip();
};

bool PolyOpMeshSmooth::IsGripShown()
{
	return mGripShown;
}

void PolyOpMeshSmooth::ShowGrip(EditPolyMod *pEditPoly)
{
	if(mGripShown==false)
	{
		//always enabel and use the static one!
		mMSmoothGrip = &pEditPoly->mMSmoothGrip;
		mMSmoothGrip->Init(pEditPoly, this);
		GetIGripManager()->SetGripActive(mMSmoothGrip);
		mGripShown = true;
	}
}


void PolyOpMeshSmooth::HideGrip()
{
	if(mGripShown==true)
	{
		mGripShown = false;
		if(mMSmoothGrip)
			GetIGripManager()->SetGripsInactive();
	}
}

void PolyOpMeshSmooth::SetUserFlags (MNMesh & mesh)
{
	// The CubicNURMS algorithm uses the MN_TARG flag, not the MN_EDITPOLY_OP_SELECT flag.
	int i, max = mSelection.GetSize ();
	if (max == 0) return;
	if (max>mesh.numv) max = mesh.numv;
	for (i=0; i<max; i++)
		mesh.v[i].SetFlag(MN_TARG, mSelection[i]?true:false);
}

bool PolyOpMeshSmooth::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	if (mSmoothness < 1.0f) {
		mesh.DetargetVertsBySharpness (1.0f - mSmoothness);
	}
	mesh.ClearEFlags (MN_EDGE_NOCROSS|MN_EDGE_MAP_SEAM);
	mesh.SetMapSeamFlags ();
	mesh.FenceOneSidedEdges ();
	if (mSepByMat) mesh.FenceMaterials ();
	if (mSepBySmooth) mesh.FenceSmGroups ();
	mesh.SplitFacesUsingBothSidesOfEdge (0);
	mesh.CubicNURMS (NULL, NULL, MN_SUBDIV_NEWMAP);
	mesh.ClearEFlags (MN_EDGE_NOCROSS|MN_EDGE_MAP_SEAM);

	return true;
}

class PolyOpTessellate : public PolyOperation
{
private:
	float mTessTension;
	int mTessType;
	bool  mGripShown;
	TessellateGrip  *mTessellateGrip;

public:
	PolyOpTessellate () : mTessTension(0.0f), mTessType(0),mGripShown(false),mTessellateGrip(NULL) { }

	// Implement PolyOperation methods:
	int OpID () { return ep_op_tessellate; }
	bool SelLevelSupport (int selLevel) { return true; }
	int MeshSelLevel () { return MNM_SL_FACE; }

	TCHAR *Name () { return GetString (IDS_EDITPOLY_TESSELLATE); }
	int DialogID () { return IDD_SETTINGS_TESSELLATE; }
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (2);
		paramList[0] = epm_tess_type;
		paramList[1] = epm_tess_tension;
	}
	void *Parameter (int paramID) {
		switch (paramID)
		{
		case epm_tess_type: return &mTessType;
		case epm_tess_tension: return &mTessTension;
		}
		return NULL;
	}

	bool ChangesSelection () { return true; }
	void SetUserFlags (MNMesh & mesh);
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);

	PolyOperation *Clone () {
		PolyOpTessellate *ret = new PolyOpTessellate ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }

	bool HasGrip() { return true; }
	bool IsGripShown();
	void ShowGrip( EditPolyMod *p );
	void HideGrip();
};

bool PolyOpTessellate::IsGripShown()
{
	return mGripShown;
}

void PolyOpTessellate::ShowGrip(EditPolyMod *pEditPoly)
{
	if(mGripShown==false)
	{
		//always enabel and use the static one!
		mTessellateGrip = &pEditPoly->mTessellateGrip;
		mTessellateGrip->Init(pEditPoly, this);
		GetIGripManager()->SetGripActive(mTessellateGrip);
		mTessellateGrip->SetUpUI();
		mGripShown = true;
	}
}


void PolyOpTessellate::HideGrip()
{
	if(mGripShown==true)
	{
		mGripShown = false;
		if(mTessellateGrip)
			GetIGripManager()->SetGripsInactive();
	}
}

void PolyOpTessellate::SetUserFlags (MNMesh & mesh)
{
	// The Tessellate algorithm uses the MN_TARG flag, not the MN_EDITPOLY_OP_SELECT flag.
	int i, max = mSelection.GetSize ();
	if (max == 0) return;
	if (max>mesh.numf) max = mesh.numf;
	for (i=0; i<max; i++)
		mesh.f[i].SetFlag (MN_TARG, mSelection[i]?true:false);
}

bool PolyOpTessellate::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	if (mTessType) mesh.TessellateByCenters ();
	else mesh.TessellateByEdges (mTessTension/400.0f);

	return true;
}

class PolyOpMakePlanar : public PolyOperation
{
public:
	// Implement PolyOperation methods:
	int OpID () { return ep_op_make_planar; }
	bool SelLevelSupport (int selLevel) { return true; }
	int MeshSelLevel () { return MNM_SL_VERTEX; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_MAKE_PLANAR); }
	// No dialog ID.
	// No parameters.
	bool UseSoftSelection () { return true; }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData) { return mesh.MakeFlaggedPlanar (MNM_SL_VERTEX, MN_EDITPOLY_OP_SELECT); }
	PolyOperation *Clone () {
		PolyOpMakePlanar *ret = new PolyOpMakePlanar();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

// Abstract class...
class PolyOpMakePlanarIn : public PolyOperation
{
public:
	// Implement PolyOperation methods:
	bool SelLevelSupport (int selLevel) { return true; }
	int MeshSelLevel () { return MNM_SL_VERTEX; }
	bool UseSoftSelection () { return true; }
	virtual int Dimension ()=0;

	// No dialog ID.
	// No parameters.
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
};

bool PolyOpMakePlanarIn::Do (MNMesh & mesh, LocalPolyOpData *pOpData) {
	int dim = Dimension();
	float heightSum = 0.0f;
	float ss, numSel = 0;
	for (int i=0; i<mesh.numv; i++) {
		if (mesh.v[i].GetFlag (MN_EDITPOLY_OP_SELECT)) {
			heightSum += mesh.v[i].p[dim];
			numSel += 1.0f;
		}
		else if (mSoftSelection.Count() && (ss=mSoftSelection[i])) {
			heightSum += mesh.v[i].p[dim] * ss;
			numSel += ss;
		}
	}
	if (numSel == 0) return false;
	float heightAvg = heightSum/numSel;

	for (int i=0; i<mesh.numv; i++) {
		if (mesh.v[i].GetFlag (MN_EDITPOLY_OP_SELECT)) {
			mesh.v[i].p[dim] = heightAvg;
		}
		else if (mSoftSelection.Count() && (ss=mSoftSelection[i])) {
			mesh.v[i].p[dim] = mesh.v[i].p[dim] * (1.0f - ss) + ss * heightAvg;
		}
	}
	return true;
}

class PolyOpMakePlanarInX : public PolyOpMakePlanarIn {
	int OpID () { return ep_op_make_planar_x; }
	int Dimension() { return 0; }
	TCHAR * Name () { return GetString (IDS_EDITPOLY_MAKE_PLANAR_IN_X); }
	PolyOperation *Clone () {
		PolyOpMakePlanarInX *ret = new PolyOpMakePlanarInX();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

class PolyOpMakePlanarInY : public PolyOpMakePlanarIn {
	int OpID () { return ep_op_make_planar_y; }
	int Dimension() { return 1; }
	TCHAR * Name () { return GetString (IDS_EDITPOLY_MAKE_PLANAR_IN_Y); }
	PolyOperation *Clone () {
		PolyOpMakePlanarInY *ret = new PolyOpMakePlanarInY();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

class PolyOpMakePlanarInZ : public PolyOpMakePlanarIn {
	int OpID () { return ep_op_make_planar_z; }
	int Dimension() { return 2; }
	TCHAR * Name () { return GetString (IDS_EDITPOLY_MAKE_PLANAR_IN_Z); }
	PolyOperation *Clone () {
		PolyOpMakePlanarInZ *ret = new PolyOpMakePlanarInZ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

class PolyOpAlign : public PolyOperation
{
private:
	Point3 mPlaneNormal;
	float mPlaneOffset;
	Matrix3 mctm;

public:
	// Implement PolyOperation methods:
	int OpID () { return ep_op_align; }
	bool SelLevelSupport (int selLevel) { return true; }
	int MeshSelLevel () { return MNM_SL_VERTEX; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_ALIGN); }
	int DialogID () { return IDD_SETTINGS_ALIGN; }
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (2);
		paramList[0] = epm_align_plane_normal;
		paramList[1] = epm_align_plane_offset;
	}
	void *Parameter (int paramID) {
		switch (paramID)
		{
		case epm_align_plane_normal: return &mPlaneNormal;
		case epm_align_plane_offset: return &mPlaneOffset;
		}
		return NULL;
	}
	int ParameterSize (int paramID) {
		if (paramID == epm_align_plane_normal) return sizeof(Point3);
		return sizeof(int);
	}
	Matrix3 *ModContextTM () { return &mctm; }

	bool UseSoftSelection () { return true; }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpAlign *ret = new PolyOpAlign();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpAlign::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	Point3 planeCenter = mPlaneNormal * mPlaneOffset;
	Point3 planeNormal = mctm.VectorTransform(mPlaneNormal);
	planeCenter = mctm * planeCenter;
	float planeOffset = DotProd (planeNormal, planeCenter);

	if (mSoftSelection.Count() == 0)
		return mesh.MoveVertsToPlane (planeNormal, planeOffset, MN_EDITPOLY_OP_SELECT);
	else
	{
		MNMeshUtilities mmu(&mesh);
		return mmu.MoveVertsToPlane (planeNormal, planeOffset, mSoftSelection.Addr(0));
	}
}

void EditPolyMod::UpdateAlignParameters (TimeValue t)
{
	// We store the normal and offset in mod-context space.
	if (ip == NULL) return;

	int alignType = mpParams->GetInt (epm_align_type, t);
	if (alignType)
	{
		// Align to construction plane.
		ViewExp *vpt = ip->GetActiveViewport();
		if (vpt == NULL) return;

		Matrix3 constTm;
		vpt->GetConstructionTM (constTm);
		ip->ReleaseViewport (vpt);

		// We'll also need our own transform:
		ModContextList mcList;
		INodeTab nodes;
		ip->GetModContexts(mcList,nodes);
		Matrix3 objTm = nodes[0]->GetObjectTM (t);
		nodes.DisposeTemporary();
		Matrix3 netTm = constTm*Inverse (objTm);	// screenspace-to-objectspace.
		Matrix3 mctm = *(mcList[0]->tm);
		netTm = netTm * mctm;

		// For znorm, we want the object-space unit vector pointing into the screen:
		Point3 znorm (0,0,-1);
		znorm = Normalize (VectorTransform (netTm, znorm));

		// Find the z-depth of the construction plane, in object space:
		float zoff = DotProd (znorm, netTm.GetTrans());

		mpParams->SetValue (epm_align_plane_normal, t, znorm);
		mpParams->SetValue (epm_align_plane_offset, t, zoff);
	}
	else
	{
		// Align to view - average depth of hard-selected subobjects.
		Matrix3 atm, otm, res;
		ViewExp *vpt = ip->GetActiveViewport();
		if (vpt == NULL) return;

		vpt->GetAffineTM(atm);
		atm = Inverse(atm);
		ip->ReleaseViewport (vpt);

		// We'll also need our own transform:
		ModContextList mcList;
		INodeTab nodes;
		ip->GetModContexts(mcList,nodes);
		otm = nodes[0]->GetObjectTM(ip->GetTime());
		nodes.DisposeTemporary();
		res = atm*Inverse (otm);	// screenspace-to-objectspace.
		Matrix3 mctm = *(mcList[0]->tm);
		res = res * mctm;	// screenspace-to-modcontext-space

		// For ZNorm, we want the object-space unit vector pointing into the screen:
		Point3 znorm (0,0,-1);
		znorm = Normalize (VectorTransform (res, znorm));

		float zoff = 0.0f;
		int ct=0;
		for (int node=0; node<mcList.Count(); node++) {
			EditPolyData *pData = (EditPolyData*)mcList[node]->localData;
			if (!pData) return;
			MNMesh *pMesh = pData->GetMesh();
			if (!pMesh) return;
			MNMesh & mm = *pMesh;
			DWORD msl = meshSelLevel[selLevel];

			Matrix3 & nodeMCtm = *(mcList[node]->tm);

			mm.ClearVFlags (MN_USER);
			if (msl == MNM_SL_OBJECT) {
				for (int i=0; i<mm.numv; i++) mm.v[i].SetFlag (MN_USER, !mm.v[i].GetFlag (MN_DEAD));
			} else mm.PropegateComponentFlags (MNM_SL_VERTEX, MN_USER, msl, MN_SEL);

			// Find the average z-depth for the current selection.
			for (int i=0; i<mm.numv; i++) {
				if (!mm.v[i].GetFlag (MN_USER)) continue;
				zoff += DotProd (znorm, nodeMCtm * mm.v[i].p);
				ct++;
			}
		}
		if (ct) zoff /= float(ct);

		mpParams->SetValue (epm_align_plane_normal, t, znorm);
		mpParams->SetValue (epm_align_plane_offset, t, zoff);
	}
}

class PolyOpRelax : public PolyOperation
{
	float mRelax;
	int mIters, mHoldBoundary, mHoldOuter;
	bool mGripShown;
	RelaxGrip  *mRelaxGrip;

public:
	PolyOpRelax():mRelax(0.0f),mIters(1),mHoldBoundary(FALSE),mHoldOuter(FALSE),mGripShown(false),mRelaxGrip(NULL){ }
	~PolyOpRelax(){ HideGrip(); }

	int OpID () { return ep_op_relax; }
	bool SelLevelSupport (int selLevel) { return true; }
	int MeshSelLevel () { return MNM_SL_VERTEX; }
	bool UseSoftSelection () { return true; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_RELAX); }
	int DialogID () { return IDD_SETTINGS_RELAX; }
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (4);
		paramList[0] = epm_relax_amount;
		paramList[1] = epm_relax_iters;
		paramList[2] = epm_relax_hold_boundary;
		paramList[3] = epm_relax_hold_outer;
	}
	void *Parameter (int paramID) {
		switch (paramID)
		{
		case epm_relax_amount: return &mRelax;
		case epm_relax_iters: return &mIters;
		case epm_relax_hold_boundary: return &mHoldBoundary;
		case epm_relax_hold_outer: return &mHoldOuter;
		}
		return NULL;
	}
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpRelax *ret = new PolyOpRelax();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }

	bool HasGrip() { return true; }
	bool IsGripShown();
	void ShowGrip( EditPolyMod *p );
	void HideGrip();
};

bool PolyOpRelax::IsGripShown()
{
	return mGripShown;
}

void PolyOpRelax::ShowGrip(EditPolyMod *pEditPoly)
{
	if(mGripShown==false)
	{
		//always enabel and use the static one!
		mRelaxGrip = &pEditPoly->mRelaxGrip;
		mRelaxGrip->Init(pEditPoly, this);
		GetIGripManager()->SetGripActive(mRelaxGrip);
		mGripShown = true;
	}
}


void PolyOpRelax::HideGrip()
{
	if(mGripShown==true)
	{
		mGripShown = false;
		if(mRelaxGrip)
			GetIGripManager()->SetGripsInactive();
	}
}


bool PolyOpRelax::Do (MNMesh & mesh, LocalPolyOpData *pOpData) {
	MNMeshUtilities mmu(&mesh);
	return mmu.Relax (MN_EDITPOLY_OP_SELECT, mSoftSelection.Count()?mSoftSelection.Addr(0):NULL,
		mRelax, mIters, mHoldBoundary != 0, mHoldOuter != 0);
}

class PolyOpHideVertex : public PolyOperation
{
public:
	int OpID() { return ep_op_hide_vertex; }
	bool SelLevelSupport (int selLevel) { return (selLevel==EPM_SL_VERTEX); }
	int MeshSelLevel () { return MNM_SL_VERTEX; }
	TCHAR *Name () { return GetString (IDS_EDITPOLY_HIDE_VERTEX); }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	//bool ChangesSelection() { return true; }
	PolyOperation *Clone () {
		PolyOpHideVertex *ret = new PolyOpHideVertex ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpHideVertex::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	return false;
	//bool ret = false;
	//for (int i=0; i<mesh.numv; i++)
	//{
	//	if (mesh.v[i].GetFlag (MN_EDITPOLY_OP_SELECT))
	//	{
	//		mesh.v[i].SetFlag (MN_HIDDEN);
	//		mesh.v[i].ClearFlag (MN_SEL|MN_EDITPOLY_STACK_SELECT);
	//		ret = true;
	//	}
	//}

	//return ret;
}

class PolyOpHideFace : public PolyOperation
{
public:
	int OpID() { return ep_op_hide_face; }
	bool SelLevelSupport (int selLevel) { return (meshSelLevel[selLevel]==MNM_SL_FACE); }
	int MeshSelLevel () { return MNM_SL_FACE; }
	TCHAR *Name () { return GetString (IDS_EDITPOLY_HIDE_FACE); }
	//bool ChangesSelection () { return true; }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpHideFace *ret = new PolyOpHideFace ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpHideFace::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	return false;
	//bool ret = false;
	//// Hide the verts on all-hidable-faces first:
	//if (mesh.PropegateComponentFlags (MNM_SL_VERTEX, MN_HIDDEN, MNM_SL_FACE, MN_EDITPOLY_OP_SELECT, true)) ret = true;
	//// (And deselect hidden vertices)
	//if (ret) mesh.PropegateComponentFlags (MNM_SL_VERTEX, MN_SEL, MNM_SL_VERTEX, MN_HIDDEN, false, false);

	//// Now hide and deselect the faces:
	//for (int i=0; i<mesh.numf; i++) {
	//	if (mesh.f[i].GetFlag (MN_DEAD|MN_HIDDEN)) continue;
	//	if (mesh.f[i].GetFlag (MN_EDITPOLY_OP_SELECT)) {
	//		mesh.f[i].SetFlag (MN_HIDDEN);
	//		mesh.f[i].ClearFlag (MN_SEL|MN_EDITPOLY_STACK_SELECT);
	//		ret = true;
	//	}
	//}

	//// Deselect edges that are only on hidden faces:
	//mesh.PropegateComponentFlags (MNM_SL_EDGE, MN_SEL|MN_EDITPOLY_STACK_SELECT,
	//	MNM_SL_FACE, MN_HIDDEN, true, false);

	//return ret;
}

class PolyOpHideUnselVertex : public PolyOperation
{
public:
	int OpID() { return ep_op_hide_unsel_vertex; }
	bool SelLevelSupport (int selLevel) { return (selLevel==EPM_SL_VERTEX); }
	int MeshSelLevel () { return MNM_SL_VERTEX; }
	TCHAR *Name () { return GetString (IDS_EDITPOLY_HIDE_UNSEL_VERTS); }
	//bool ChangesSelection() { return true; }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpHideUnselVertex *ret = new PolyOpHideUnselVertex ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpHideUnselVertex::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	return false;
	//bool ret = false;
	//DWORD ignoreFlags = MN_EDITPOLY_OP_SELECT|MN_DEAD|MN_HIDDEN;	// these are the verts we don't affect.
	//for (int i=0; i<mesh.numv; i++)
	//{
	//	if (mesh.v[i].GetFlag (ignoreFlags)) continue;
	//	mesh.v[i].SetFlag (MN_HIDDEN);
	//	mesh.v[i].ClearFlag (MN_SEL|MN_EDITPOLY_STACK_SELECT);
	//	ret = true;
	//}
	//return ret;
}

class PolyOpHideUnselFace : public PolyOperation
{
public:
	int OpID() { return ep_op_hide_unsel_face; }
	bool SelLevelSupport (int selLevel) { return (meshSelLevel[selLevel]==MNM_SL_FACE); }
	int MeshSelLevel () { return MNM_SL_FACE; }
	TCHAR *Name () { return GetString (IDS_EDITPOLY_HIDE_UNSEL_FACES); }
	//bool ChangesSelection() { return true; }	// Might affect vertex, edge selection.
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpHideUnselFace *ret = new PolyOpHideUnselFace ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpHideUnselFace::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	return false;
	//bool ret = false;
	//DWORD ignoreFlags = MN_EDITPOLY_OP_SELECT|MN_DEAD|MN_HIDDEN;	// these are the faces we don't affect.

	//mesh.ClearVFlags (MN_USER);
	//for (int i=0; i<mesh.numf; i++)
	//{
	//	if (mesh.f[i].GetFlag (ignoreFlags)) continue;
	//	mesh.f[i].SetFlag (MN_HIDDEN);
	//	mesh.f[i].ClearFlag (MN_SEL|MN_EDITPOLY_STACK_SELECT);
	//	ret = true;
	//	for (int j=0; j<mesh.f[i].deg; j++) mesh.v[mesh.f[i].vtx[j]].SetFlag (MN_USER);
	//}
	//if (!ret) return false;

	//for (i=0; i<mesh.numf; i++)
	//{
	//	if (mesh.f[i].GetFlag (MN_DEAD|MN_HIDDEN)) continue;
	//	for (int j=0; j<mesh.f[i].deg; j++) mesh.v[mesh.f[i].vtx[j]].ClearFlag (MN_USER);
	//}

	//// Hide and deselect the appropriate vertices:
	//mesh.PropegateComponentFlags (MNM_SL_VERTEX, MN_HIDDEN, MNM_SL_VERTEX, MN_USER);
	//mesh.PropegateComponentFlags (MNM_SL_VERTEX, MN_SEL|MN_EDITPOLY_STACK_SELECT,
	//	MNM_SL_VERTEX, MN_HIDDEN, false, false);

	//// Deselect edges that are only on hidden faces:
	//mesh.PropegateComponentFlags (MNM_SL_EDGE, MN_SEL|MN_EDITPOLY_STACK_SELECT,
	//	MNM_SL_FACE, MN_HIDDEN, true, false);

	//return true;
}

class PolyOpUnhideVertex : public PolyOperation
{
public:
	int OpID() { return ep_op_unhide_vertex; }
	bool SelLevelSupport (int selLevel) { return (selLevel==EPM_SL_VERTEX); }
	int MeshSelLevel () { return MNM_SL_VERTEX; }
	TCHAR *Name () { return GetString (IDS_EDITPOLY_UNHIDE_VERTICES); }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpUnhideVertex *ret = new PolyOpUnhideVertex ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpUnhideVertex::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	return false;
	//bool ret = false;
	//for (int i=0; i<mesh.numv; i++)
	//{
	//	if (mesh.v[i].GetFlag (MN_DEAD)) continue;
	//	if (!mesh.v[i].GetFlag (MN_HIDDEN)) continue;
	//	mesh.v[i].ClearFlag (MN_HIDDEN);
	//	ret = true;
	//}
	//return ret;
}

class PolyOpUnhideFace : public PolyOperation
{
public:
	int OpID() { return ep_op_unhide_face; }
	bool SelLevelSupport (int selLevel) { return (meshSelLevel[selLevel]==MNM_SL_FACE); }
	int MeshSelLevel () { return MNM_SL_FACE; }
	TCHAR *Name () { return GetString (IDS_EDITPOLY_UNHIDE_FACES); }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpUnhideFace *ret = new PolyOpUnhideFace ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpUnhideFace::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	return false;
	//bool ret = false;
	//for (int i=0; i<mesh.numf; i++) {
	//	if (mesh.f[i].GetFlag (MN_DEAD)) continue;
	//	if (!mesh.f[i].GetFlag (MN_HIDDEN)) continue;
	//	mesh.f[i].ClearFlag (MN_HIDDEN);
	//	for (int j=0; j<mesh.f[i].deg; j++) mesh.v[mesh.f[i].vtx[j]].ClearFlag (MN_HIDDEN);
	//	ret = true;
	//}
	//return ret;
}

class PolyOpRemoveVertex : public PolyOperation
{
public:
	// Implement PolyOperation methods:
	int OpID () { return ep_op_remove_vertex; }
	bool SelLevelSupport (int selLevel) { return (selLevel == EPM_SL_VERTEX); }
	int MeshSelLevel () { return MNM_SL_VERTEX; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_REMOVE_VERTEX); }
	// No dialog ID.
	// No parameters.
	bool CanDelete () { return true; }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpRemoveVertex *ret = new PolyOpRemoveVertex ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpRemoveVertex::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	bool ret = false;
	for (int i=0; i<mesh.numv; i++) {
		if (mesh.v[i].GetFlag (MN_EDITPOLY_OP_SELECT))
		{
			mesh.RemoveVertex (i);
			ret = true;
		}
	}
	return ret;
}

class PolyOpConnectVertex : public PolyOperation
{
public:
	// Implement PolyOperation methods:
	int OpID () { return ep_op_connect_vertex; }
	bool SelLevelSupport (int selLevel) { return (selLevel == EPM_SL_VERTEX); }
	int MeshSelLevel () { return MNM_SL_VERTEX; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_CONNECT_VERTEX); }
	// No dialog ID.
	// No parameters.
	bool ChangesSelection () { return true; }	// changes edge selection.

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData) { return mesh.ConnectVertices (MN_EDITPOLY_OP_SELECT); }
	PolyOperation *Clone () {
		PolyOpConnectVertex *ret = new PolyOpConnectVertex ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

class PolyOpBreakVertex : public PolyOperation
{
	float mDistance;

public:
	// Implement PolyOperation methods:
	int OpID () { return ep_op_break; }
	bool SelLevelSupport (int selLevel) { return selLevel == EPM_SL_VERTEX; }
	int MeshSelLevel () { return MNM_SL_VERTEX; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_BREAK_VERTEX); }
	// No dialog ID.
	int DialogID () { return IDD_SETTINGS_BREAK_VERTEX; }
	// No parameters.
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (1);
		paramList[0] = epm_break_vertex_distance;
	}
	void *Parameter (int paramID) {
		switch (paramID)
		{
		case epm_break_vertex_distance: return &mDistance;
		}
		return NULL;
	}	
	
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData) 
	{ 
		IMNMeshUtilities8* l_meshToBridge = static_cast<IMNMeshUtilities8*>(mesh.GetInterface( IMNMESHUTILITIES8_INTERFACE_ID ));
		bool l_ret = false; 
		
		if ( l_meshToBridge )
		{
			l_ret = mesh.SplitFlaggedVertices (MN_EDITPOLY_OP_SELECT); 

		}

		return l_ret;
	}

	PolyOperation *Clone () {
		PolyOpBreakVertex *ret = new PolyOpBreakVertex();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

class PolyOpWeldVertex : public PolyOperation
{
private:
	float mThreshold;
	bool mGripShown;
	WeldVerticesGrip *mWeldVerticesGrip;
	
public:
	PolyOpWeldVertex () : mThreshold(0.0f),mGripShown(false),mWeldVerticesGrip(NULL) { }

	// Implement PolyOperation methods:
	int OpID () { return ep_op_weld_vertex; }
	bool SelLevelSupport (int selLevel) { return selLevel == EPM_SL_VERTEX; }
	int MeshSelLevel () { return MNM_SL_VERTEX; }

	TCHAR *Name () { return GetString (IDS_EDITPOLY_WELD_VERTEX); }
	int DialogID () { return IDD_SETTINGS_WELD_VERTICES; }
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (1);
		paramList[0] = epm_weld_vertex_threshold;
	}
	void *Parameter (int paramID) {
		return (paramID == epm_weld_vertex_threshold) ? &mThreshold : NULL;
	}
	bool CanDelete () { return true; }

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpWeldVertex *ret = new PolyOpWeldVertex();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }

	// Local methods:
	bool WeldShortPolyEdges (MNMesh & mesh, DWORD vertFlag);

	bool HasGrip() { return true; }
	bool IsGripShown();
	void ShowGrip( EditPolyMod *p );
	void HideGrip();
};

bool PolyOpWeldVertex::IsGripShown()
{
	return mGripShown;
}

void PolyOpWeldVertex::ShowGrip(EditPolyMod *pEditPoly)
{
	if(mGripShown==false)
	{
		//always enabel and use the static one!
		mWeldVerticesGrip = &pEditPoly->mWeldVerticesGrip;
		mWeldVerticesGrip->Init(pEditPoly, this);
		GetIGripManager()->SetGripActive(mWeldVerticesGrip);
		mGripShown = true;
	}
}


void PolyOpWeldVertex::HideGrip()
{
	if(mGripShown==true)
	{
		mGripShown = false;
		if(mWeldVerticesGrip)
			GetIGripManager()->SetGripsInactive();
	}
}

bool PolyOpWeldVertex::Do(MNMesh & mesh, LocalPolyOpData *pOpData)
{
	// Weld the suitable border vertices:
	bool haveWelded = false;
	if (mesh.WeldBorderVerts (mThreshold, MN_EDITPOLY_OP_SELECT)) {
		haveWelded = true;
	}

	// Weld vertices that share short edges:
	if (WeldShortPolyEdges (mesh, MN_EDITPOLY_OP_SELECT)) haveWelded = true;

	if (haveWelded) {
		mesh.InvalidateTopoCache ();
		mesh.FillInMesh ();
	}

	return haveWelded;
}

bool PolyOpWeldVertex::WeldShortPolyEdges (MNMesh & mesh, DWORD vertFlag) {
	// In order to collapse vertices, we turn them into edge selections,
	// where the edges are shorter than the weld threshold.
	bool canWeld = false;
	mesh.ClearEFlags (MN_USER);
	float threshSq = mThreshold*mThreshold;
	for (int i=0; i<mesh.nume; i++) {
		if (mesh.e[i].GetFlag (MN_DEAD)) continue;
		if (!mesh.v[mesh.e[i].v1].GetFlag (vertFlag)) continue;
		if (!mesh.v[mesh.e[i].v2].GetFlag (vertFlag)) continue;
		if (LengthSquared (mesh.P(mesh.e[i].v1) - mesh.P(mesh.e[i].v2)) > threshSq) continue;
		mesh.e[i].SetFlag (MN_USER);
		canWeld = true;
	}
	if (!canWeld) return false;

	MNMeshUtilities mmu(&mesh);
	return mmu.CollapseEdges (MN_USER);
}

const USHORT kTargetWeldStart = 0x6C0;
const USHORT kTargetWeldTarget = 0x06C4;

IOResult LocalTargetWeldData::Save(ISave *isave) {
	ULONG nb;
	int num = mStart.Count();
	if (num)
	{
		isave->BeginChunk (kTargetWeldStart);
		isave->Write (&num, sizeof(int), &nb);
		isave->Write (mStart.Addr(0), num*sizeof(int), &nb);
		isave->EndChunk ();
	}

	num = mTarget.Count();
	if (num)
	{
		isave->BeginChunk (kTargetWeldTarget);
		isave->Write (&num, sizeof(int), &nb);
		isave->Write (mTarget.Addr(0), num*sizeof(int), &nb);
		isave->EndChunk ();
	}

	return IO_OK;
}

IOResult LocalTargetWeldData::Load(ILoad *iload) {
	IOResult res;
	int num;
	ULONG nb;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID ())
		{
		case kTargetWeldStart:
			res = iload->Read (&num, sizeof(int), &nb);
			if (res != IO_OK) break;
			mStart.SetCount (num);
			res = iload->Read (mStart.Addr(0), num*sizeof(int), &nb);
			break;

		case kTargetWeldTarget:
			res = iload->Read (&num, sizeof(int), &nb);
			if (res != IO_OK) break;
			mTarget.SetCount (num);
			res = iload->Read (mTarget.Addr(0), num*sizeof(int), &nb);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

class PolyOpTargetWeldVertex : public PolyOperation
{
public:
	PolyOpTargetWeldVertex () { }

	// Implement PolyOperation methods:
	int OpID () { return ep_op_target_weld_vertex; }
	bool SelLevelSupport (int selLevel) { return selLevel == EPM_SL_VERTEX; }
	int MeshSelLevel () { return MNM_SL_OBJECT; }	// ignores current selection.

	TCHAR *Name () { return GetString (IDS_EDITPOLY_TARGET_WELD_VERTEX); }
	// No parameters or dialog.
	bool CanDelete () { return true; }
	bool CanAnimate() { return false; }

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpTargetWeldVertex *ret = new PolyOpTargetWeldVertex();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpTargetWeldVertex::Do (MNMesh & mesh, LocalPolyOpData *pOpData) {
	if (!pOpData || (pOpData->OpID() != OpID())) return false;

	LocalTargetWeldVertexData *pWeld = (LocalTargetWeldVertexData *)pOpData;
	if (pWeld->NumWelds()<1) return false;

	Tab<int> vertMap;
	if (pWeld->NumWelds()>1) {
		vertMap.SetCount (mesh.numv);
	}

	bool ret=false;
	for (int weld=0; weld<pWeld->NumWelds(); weld++)
	{
		int v1 = pWeld->GetStart (weld);
		int v2 = pWeld->GetTarget (weld);
		// Protect against underlying topo changes:
		if ((v1>=mesh.numv) || (v2>=mesh.numv)) continue;
		if (weld>0) {
			// Convert to input mesh indices, since we haven't collapsed the dead yet.
         int j, numLiving;
			for (j=0, numLiving=0; j<vertMap.Count(); j++) {
				if (mesh.v[j].GetFlag (MN_DEAD)) continue;
				vertMap[numLiving] = j;
				numLiving++;
			}

			// Protect against underlying topo changes:
			if ((v1>=numLiving) || (v2>=numLiving)) continue;
			v1 = vertMap[v1];
			v2 = vertMap[v2];
		}
		Point3 destination = mesh.P(v2);

		// If vertices share an edge, then take a collapse type approach;
		// Otherwise, weld them if they're suitable (open verts, etc.)
		int i;
		bool thisRet=false;
		for (i=0; i<mesh.vedg[v1].Count(); i++) {
			if (mesh.e[mesh.vedg[v1][i]].OtherVert(v1) == v2) break;
		}
		if (i<mesh.vedg[v1].Count()) {
			thisRet = mesh.WeldEdge (mesh.vedg[v1][i]);
			if (mesh.v[v1].GetFlag (MN_DEAD)) mesh.v[v2].p = destination;
			else mesh.v[v1].p = destination;
		} else {
			thisRet = mesh.WeldBorderVerts (v1, v2, &destination);
		}

		ret |= thisRet;
	}

	return ret;
}

class PolyOpExtrudeVertex : public PolyOperation
{
private:
	float mWidth, mHeight;
	ExtrudeVertexGrip  *mExtrudeVertexGrip;
	bool                    mGripShown;
public:
	PolyOpExtrudeVertex () : mWidth(0.0f), mHeight(0.0f),mGripShown(false) { }
	~PolyOpExtrudeVertex() { HideGrip(); }

	// Implement PolyOperation methods:
	int OpID () { return ep_op_extrude_vertex; }
	bool SelLevelSupport (int selLevel) { return selLevel == EPM_SL_VERTEX; }
	int MeshSelLevel () { return MNM_SL_VERTEX; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_EXTRUDE_VERTEX); }
	int DialogID () { return IDD_SETTINGS_EXTRUDE_VERTEX; }

	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (2);
		paramList[0] = epm_extrude_vertex_height;
		paramList[1] = epm_extrude_vertex_width;
	}
	void *Parameter (int paramID) {
		if (paramID == epm_extrude_vertex_height) return &mHeight;
		if (paramID == epm_extrude_vertex_width) return &mWidth;
		return NULL;
    }
    void RescaleWorldUnits (float f)
    {
        mWidth *= f;
        mHeight *= f;
    }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpExtrudeVertex *ret = new PolyOpExtrudeVertex();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }

	bool HasGrip() { return true; }
	bool IsGripShown();
	void ShowGrip( EditPolyMod *p );
	void HideGrip();
};

bool PolyOpExtrudeVertex::IsGripShown()
{
	return mGripShown;
}

void PolyOpExtrudeVertex::ShowGrip( EditPolyMod *pEditPoly )
{
	if(mGripShown==false)
	{
		//always enabel and use the static one!
		mExtrudeVertexGrip = &pEditPoly->mExtrudeVertexGrip;
		mExtrudeVertexGrip->Init( pEditPoly, this);
		GetIGripManager()->SetGripActive(mExtrudeVertexGrip);
		mGripShown = true;
	}
}

void PolyOpExtrudeVertex::HideGrip()
{
	if(mGripShown==true)
	{
		mGripShown = false;
		if(mExtrudeVertexGrip)
			GetIGripManager()->SetGripsInactive();
	}
}

bool PolyOpExtrudeVertex::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	MNChamferData chamData;
	chamData.InitToMesh(mesh);
	Tab<Point3> tUpDir;
	tUpDir.SetCount (mesh.numv);

	// Topology change:
	if (!mesh.ExtrudeVertices (MN_EDITPOLY_OP_SELECT, &chamData, tUpDir)) return false;

	// Apply map changes based on base width:
	int i;
	Tab<UVVert> tMapDelta;
	for (int mapChannel=-NUM_HIDDENMAPS; mapChannel<mesh.numm; mapChannel++) {
		if (mesh.M(mapChannel)->GetFlag (MN_DEAD)) continue;
		chamData.GetMapDelta (mesh, mapChannel, mWidth, tMapDelta);
		UVVert *pMapVerts = mesh.M(mapChannel)->v;
		if (!pMapVerts) continue;
		for (i=0; i<mesh.M(mapChannel)->numv; i++) pMapVerts[i] += tMapDelta[i];
	}

	// Apply geom changes based on base width:
	Tab<Point3> tDelta;
	chamData.GetDelta (mWidth, tDelta);
	for (i=0; i<mesh.numv; i++) mesh.v[i].p += tDelta[i];

	// Move the points up:
	for (i=0; i<tUpDir.Count(); i++) mesh.v[i].p += tUpDir[i]*mHeight;

	return true;
}

class PolyOpChamferVertex : public PolyOperation
{
private:
	float mAmount;
	BOOL  mOpen;
	bool mGripShown;
	ChamferVerticesGrip  *mChamferVerticesGrip;

public:
	PolyOpChamferVertex () : mAmount(0.0f), mOpen(FALSE),mGripShown(false),mChamferVerticesGrip(NULL) { }
	~PolyOpChamferVertex(){ HideGrip(); }

	// Implement PolyOperation methods:
	int OpID () { return ep_op_chamfer_vertex; }
	bool SelLevelSupport (int selLevel) { return selLevel == EPM_SL_VERTEX; }
	int MeshSelLevel () { return MNM_SL_VERTEX; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_CHAMFER_VERTEX); }
	int DialogID () { return IDD_SETTINGS_CHAMFER_VERTEX; }

	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (2);
		paramList[0] = epm_chamfer_vertex;
		paramList[1] = epm_chamfer_vertex_open;
	}
	void *Parameter (int paramID) {
		if (paramID == epm_chamfer_vertex) return &mAmount;
		if (paramID == epm_chamfer_vertex_open) return &mOpen;
		return NULL;
	}
    bool ChangesSelection () { return true; }
    void RescaleWorldUnits (float f)
    {
        mAmount *= f;
    }

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpChamferVertex *ret = new PolyOpChamferVertex();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }

	bool HasGrip(){ return true; }
	bool IsGripShown();
	void ShowGrip(EditPolyMod *p);
	void HideGrip();
};

bool PolyOpChamferVertex::IsGripShown()
{
	return mGripShown;
}

void PolyOpChamferVertex::ShowGrip( EditPolyMod *pEditPoly )
{
	if(mGripShown==false)
	{
		//always enabel and use the static one!
		mChamferVerticesGrip = &pEditPoly->mChamferVerticesGrip;
		mChamferVerticesGrip->Init( pEditPoly, this);
		GetIGripManager()->SetGripActive(mChamferVerticesGrip);
		mGripShown = true;
	}
}

void PolyOpChamferVertex::HideGrip()
{
	if(mGripShown==true)
	{
		mGripShown = false;
		if(mChamferVerticesGrip)
			GetIGripManager()->SetGripsInactive();
	}
}

bool PolyOpChamferVertex::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	MNChamferData chamData;

	DWORD l_flag = MN_EDITPOLY_OP_SELECT; 

	IMNMeshUtilities8* l_meshToChamfer = static_cast<IMNMeshUtilities8*>(mesh.GetInterface( IMNMESHUTILITIES8_INTERFACE_ID ));

	bool l_ret = false; 

	if (l_meshToChamfer != NULL )
	{
		bool open = mOpen ? true :false;
		l_ret = l_meshToChamfer->ChamferVertices (l_flag, &chamData,open);
	}

	if ( l_ret )
	{
		Tab<UVVert> mapDelta;
		for (int mapChannel = -NUM_HIDDENMAPS; mapChannel<mesh.numm; mapChannel++) {
			if (mesh.M(mapChannel)->GetFlag (MN_DEAD)) continue;
			chamData.GetMapDelta (mesh, mapChannel, mAmount, mapDelta);
			for (int i=0; i<mapDelta.Count(); i++) mesh.M(mapChannel)->v[i] += mapDelta[i];
		}

		Tab<Point3> vertexDelta;
		chamData.GetDelta (mAmount, vertexDelta);
		for (int i=0; i<vertexDelta.Count(); i++) mesh.P(i) += vertexDelta[i];
	}

	return l_ret;
}

class PolyOpRemoveIsolatedVertices : public PolyOperation
{
public:
	// Implement PolyOperation methods:
	int OpID () { return ep_op_remove_iso_verts; }
	bool SelLevelSupport (int selLevel) { return (selLevel==EPM_SL_VERTEX); }
	int MeshSelLevel () { return MNM_SL_OBJECT; }	// Ignores current selection.

	TCHAR * Name () { return GetString (IDS_EDITPOLY_REMOVE_ISOLATED_VERTICES); }
	// No dialog ID.
	// No parameters.
	bool CanDelete () { return true; }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpRemoveIsolatedVertices *ret = new PolyOpRemoveIsolatedVertices ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpRemoveIsolatedVertices::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	bool ret=false;
	mesh.FillInMesh ();	// just to be sure...
	for (int i=0; i<mesh.numv; i++) {
		if (mesh.v[i].GetFlag (MN_DEAD)) continue;
		if (mesh.vfac[i].Count()) continue;
		mesh.v[i].SetFlag (MN_DEAD);
		ret = true;
	}
	return ret;
}

class PolyOpRemoveIsolatedMapVertices : public PolyOperation
{
public:
	// Implement PolyOperation methods:
	int OpID () { return ep_op_remove_iso_map_verts; }
	bool SelLevelSupport (int selLevel) { return (selLevel==EPM_SL_VERTEX); }
	int MeshSelLevel () { return MNM_SL_OBJECT; }	// Ignores current selection.

	TCHAR * Name () { return GetString (IDS_EDITPOLY_REMOVE_UNUSED_MAP_VERTS); }
	// No dialog ID.
	// No parameters.
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData) { mesh.EliminateIsoMapVerts (); return true; }
	PolyOperation *Clone () {
		PolyOpRemoveIsolatedMapVertices *ret = new PolyOpRemoveIsolatedMapVertices ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

const USHORT kCreateEdge = 0x640;

IOResult LocalCreateEdgeData::Save(ISave *isave) {
	ULONG nb;
	int num = mEdgeList.Count();
	if (num)
	{
		isave->BeginChunk (kCreateEdge);
		isave->Write (&num, sizeof(int), &nb);
		isave->Write (mEdgeList.Addr(0), num*sizeof(int), &nb);
		isave->EndChunk ();
	}

	return IO_OK;
}

IOResult LocalCreateEdgeData::Load(ILoad *iload) {
	IOResult res;
	int num;
	ULONG nb;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID ())
		{
		case kCreateEdge:
			res = iload->Read (&num, sizeof(int), &nb);
			if (res != IO_OK) break;
			mEdgeList.SetCount (num);
			res = iload->Read (mEdgeList.Addr(0), num*sizeof(int), &nb);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

class PolyOpCreateEdge : public PolyOperation
{
public:
	int OpID () { return ep_op_create_edge; }
	bool SelLevelSupport (int selLevel) { return true; }
	int MeshSelLevel () { return MNM_SL_OBJECT; }

	TCHAR *Name() { return GetString (IDS_EDITPOLY_CREATE_EDGE); }
	// No dialog ID
	// No parameters
	bool CanAnimate () { return false; }
	bool ChangesSelection () { return true; }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpCreateEdge *ret = new PolyOpCreateEdge ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpCreateEdge::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	if (pOpData == NULL) return false;
	if (pOpData->OpID () != OpID()) return false;
	LocalCreateEdgeData *pCreateEdge = (LocalCreateEdgeData *) pOpData;

	bool ret=false;
	for (int edge=0; edge<pCreateEdge->NumEdges(); edge++)
	{
		int v1 = pCreateEdge->GetVertex (edge, 0);
		int v2 = pCreateEdge->GetVertex (edge, 1);
		if ((v1>=mesh.numv) || (v1<0) || (v2>=mesh.numv) || (v2<0)) continue;

		Tab<int> v1fac = mesh.vfac[v1];
		int i, j, ff = 0, v1pos = 0, v2pos=-1;
		for (i=0; i<v1fac.Count(); i++) {
			MNFace & mf = mesh.f[v1fac[i]];
			for (j=0; j<mf.deg; j++) {
				if (mf.vtx[j] == v2) v2pos = j;
				if (mf.vtx[j] == v1) v1pos = j;
			}
			if (v2pos<0) continue;
			ff = v1fac[i];
			break;
		}

		if (v2pos<0) continue;

		int nf, ne;
		mesh.SeparateFace (ff, v1pos, v2pos, nf, ne, true, true);
		ret = true;
	}

	return ret;
}

const USHORT kLocalSetDiagonals = 0x680;

IOResult LocalSetDiagonalData::Save(ISave *isave) {
	ULONG nb;
	int num = mDiagList.Count();
	if (num)
	{
		isave->BeginChunk (kLocalSetDiagonals);
		isave->Write (&num, sizeof(int), &nb);
		isave->Write (mDiagList.Addr(0), num*sizeof(int), &nb);
		isave->EndChunk ();
	}

	return IO_OK;
}

IOResult LocalSetDiagonalData::Load(ILoad *iload) {
	IOResult res;
	int num;
	ULONG nb;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID ())
		{
		case kLocalSetDiagonals:
			res = iload->Read (&num, sizeof(int), &nb);
			if (res != IO_OK) break;
			mDiagList.SetCount (num);
			res = iload->Read (mDiagList.Addr(0), num*sizeof(int), &nb);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

class PolyOpEditTriangulation : public PolyOperation
{
public:
	int OpID () { return ep_op_edit_triangulation; }
	bool SelLevelSupport (int selLevel) { return true; }
	int MeshSelLevel () { return MNM_SL_OBJECT; }

	TCHAR *Name() { return GetString (IDS_EDITPOLY_EDIT_TRIANGULATION); }
	// No dialog ID
	// No parameters
	bool CanAnimate () { return false; }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpEditTriangulation *ret = new PolyOpEditTriangulation ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpEditTriangulation::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	if (pOpData == NULL) return false;
	if (pOpData->OpID () != OpID()) return false;
	LocalSetDiagonalData *pSetDiag = (LocalSetDiagonalData *) pOpData;

	bool ret=false;
	for (int diag=0; diag<pSetDiag->NumDiagonals(); diag++)
	{
		int v1 = pSetDiag->GetVertex (diag, 0);
		int v2 = pSetDiag->GetVertex (diag, 1);
		if (v1>=mesh.numv) continue;
		if (v2>=mesh.numv) continue;
		Tab<int> v1fac = mesh.vfac[v1];
		int i, j, ff = 0, v1pos = 0, v2pos=-1;
		for (i=0; i<v1fac.Count(); i++) {
			MNFace & mf = mesh.f[v1fac[i]];
			for (j=0; j<mf.deg; j++) {
				if (mf.vtx[j] == v2) v2pos = j;
				if (mf.vtx[j] == v1) v1pos = j;
			}
			if (v2pos<0) continue;
			ff = v1fac[i];
			break;
		}

		if (v2pos<0) continue;

		mesh.SetDiagonal (ff, v1pos, v2pos);
		ret = true;
	}

	return ret;
}

const USHORT kLocalTurnEdgeEdges = 0x6F0;
const USHORT kLocalTurnEdgeFaces = 0x6F2;

IOResult LocalTurnEdgeData::Save(ISave *isave) {
	ULONG nb;
	int num = mEdgeList.Count();
	if (num) {
		isave->BeginChunk (kLocalTurnEdgeEdges);
		isave->Write (&num, sizeof(int), &nb);
		isave->Write (mEdgeList.Addr(0), num*sizeof(int), &nb);
		isave->EndChunk ();
	}

	num = mFaceList.Count();
	if (num) {
		isave->BeginChunk (kLocalTurnEdgeFaces);
		isave->Write (&num, sizeof(int), &nb);
		isave->Write (mFaceList.Addr(0), num*sizeof(int), &nb);
		isave->EndChunk ();
	}

	return IO_OK;
}

IOResult LocalTurnEdgeData::Load(ILoad *iload) {
	IOResult res;
	int num;
	ULONG nb;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID ())
		{
		case kLocalTurnEdgeEdges:
			res = iload->Read (&num, sizeof(int), &nb);
			if (res != IO_OK) break;
			mEdgeList.SetCount (num);
			res = iload->Read (mEdgeList.Addr(0), num*sizeof(int), &nb);
			break;

		case kLocalTurnEdgeFaces:
			res = iload->Read (&num, sizeof(int), &nb);
			if (res != IO_OK) break;
			mFaceList.SetCount (num);
			res = iload->Read (mFaceList.Addr(0), num*sizeof(int), &nb);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

class PolyOpTurnEdge : public PolyOperation
{
public:
	int OpID () { return ep_op_turn_edge; }
	bool SelLevelSupport (int selLevel) { return true; }
	int MeshSelLevel () { return MNM_SL_OBJECT; }

	TCHAR *Name() { return GetString (IDS_TURN_EDGE); }
	// No dialog ID
	// No parameters
	bool CanAnimate () { return false; }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpTurnEdge *ret = new PolyOpTurnEdge ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpTurnEdge::Do (MNMesh & mesh, LocalPolyOpData *pOpData) {
	if (pOpData == NULL) return false;
	if (pOpData->OpID () != OpID()) return false;
	LocalTurnEdgeData *pTurn = (LocalTurnEdgeData *) pOpData;

	bool ret=false;
	MNMeshUtilities mmu(&mesh);
	for (int turn=0; turn<pTurn->NumTurns(); turn++) {
		ret |= mmu.TurnDiagonal (pTurn->GetFace(turn), pTurn->GetDiagonal(turn));
	}

	return ret;
}

const USHORT kDivideEdge = 0x1600;

IOResult LocalInsertVertexEdgeData::Save(ISave *isave) {
	ULONG nb;
	int num = mEdge.Count();
	if (num)
	{
		isave->BeginChunk (kDivideEdge);
		isave->Write (&num, sizeof(int), &nb);
		isave->Write (mEdge.Addr(0), num*sizeof(int), &nb);
		isave->Write (mProp.Addr(0), num*sizeof(float), &nb);
		isave->EndChunk ();
	}

	return IO_OK;
}

IOResult LocalInsertVertexEdgeData::Load(ILoad *iload)
{
	IOResult res;
	int num;
	ULONG nb;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID ())
		{
		case kDivideEdge:
			if ((res = iload->Read (&num, sizeof(int), &nb)) != IO_OK) break;
			mEdge.SetCount (num);
			mProp.SetCount (num);
			if ((res = iload->Read (mEdge.Addr(0), num*sizeof(int), &nb)) != IO_OK) break;
			res = iload->Read (mProp.Addr(0), num*sizeof(float), &nb);
			break;

		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

class PolyOpInsertVertexInEdge : public PolyOperation {
public:
	// Implement PolyOperation methods:
	int OpID () { return ep_op_insert_vertex_edge; }
	bool SelLevelSupport (int selLevel) { return (selLevel == EPM_SL_EDGE); }
	int MeshSelLevel () { return MNM_SL_EDGE; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_INSERT_VERTEX_IN_EDGE); }
	// No dialog ID.
	// No parameters.
	bool ChangesSelection () { return true; }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpInsertVertexInEdge *ret = new PolyOpInsertVertexInEdge();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpInsertVertexInEdge::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	if (!pOpData || (pOpData->OpID() != OpID())) return false;

	LocalInsertVertexEdgeData *pInsert = (LocalInsertVertexEdgeData *)pOpData;
	if (pInsert->NumInserts()<1) return false;

	bool ret=false;
	for (int i=0; i<pInsert->NumInserts(); i++)
	{
		int edge = pInsert->GetEdge (i);
		if (edge>=mesh.nume) continue;

		int nv = mesh.SplitEdge (edge, pInsert->GetProp(i));
		if (nv<0) continue;
		mesh.v[nv].SetFlag (MN_SEL);
		ret = true;
	}
	return ret;
}

class PolyOpRemoveEdge : public PolyOperation
{
private:
	bool mCtrlKey;

public:

	// Implement PolyOperation methods:
	int OpID () { return ep_op_remove_edge; }
	bool SelLevelSupport (int selLevel) { return (selLevel == EPM_SL_EDGE); }
	int MeshSelLevel () { return MNM_SL_EDGE; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_REMOVE_EDGE); }

	IOResult Save (ISave *isave);
	IOResult Load (ILoad *iload);

	bool CanDelete () { return true; }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpRemoveEdge *ret = new PolyOpRemoveEdge();
		CopyBasics (ret);
		ret->mCtrlKey = mCtrlKey;
		return ret;
	}
	void DeleteThis () { delete this; }

	void SetCtrlState( bool in_ctrlKeyVal) {mCtrlKey = in_ctrlKeyVal; }
	bool GetCtrlState(){ return mCtrlKey;}
};


const USHORT kControlKey = 0x1601;

IOResult PolyOpRemoveEdge::Save (ISave *isave) {
	isave->BeginChunk (kPolyOperationBasics);
	SaveBasics (isave);
	isave->EndChunk ();
	ULONG nb;
	isave->BeginChunk (kControlKey);
	isave->Write (&mCtrlKey, sizeof(bool), &nb);
	isave->EndChunk ();
	return IO_OK;
}

IOResult PolyOpRemoveEdge::Load (ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID ())
		{
		case kPolyOperationBasics:
			res = LoadBasics (iload);
			break;
		case kControlKey:
			res = iload->Read (&mCtrlKey, sizeof(bool), &nb);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}
bool PolyOpRemoveEdge::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	bool ret = false;
	for (int i=0; i<mesh.nume; i++) {
		if (mesh.e[i].GetFlag (MN_DEAD)) continue;
		if (mesh.e[i].f2<0) continue;
		if (mesh.e[i].f1 == mesh.e[i].f2) continue;
		if (!mesh.e[i].GetFlag (MN_EDITPOLY_OP_SELECT)) continue;

		if ( mCtrlKey ) 
		{
			// set the edges's vertices as user 
			mesh.ClearVFlags (MN_USER);	
			mesh.PropegateComponentFlags (MNM_SL_VERTEX, MN_USER, MNM_SL_EDGE, MN_EDITPOLY_OP_SELECT);

		}
		mesh.RemoveEdge (i);
		if  (mCtrlKey)
		{
			// loop through all vertices 
			for ( int j = 0; j < mesh.numv; j++)
			{
				if (mesh.v[j].GetFlag (MN_DEAD)) 
					continue;
				//does this vertex have only 2 edges ? 
				if ( mesh.vedg[j].Count() != 2 )
					continue;

				if (mesh.v[j].GetFlag (MN_USER))
					ret = mesh.RemoveVertex (j);
			}
		}

		ret = true;
	}

	return ret;
}

class PolyOpSplitEdge : public PolyOperation
{
public:
	// Implement PolyOperation methods:
	int OpID () { return ep_op_split; }
	bool SelLevelSupport (int selLevel) { return (selLevel == EPM_SL_EDGE); }
	int MeshSelLevel () { return MNM_SL_EDGE; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_SPLIT_EDGE); }
	// No dialog ID.
	// No parameters.
	bool ChangesSelection () { return true; }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData) { return mesh.SplitFlaggedEdges (MN_EDITPOLY_OP_SELECT); }
	PolyOperation *Clone () {
		PolyOpSplitEdge *ret = new PolyOpSplitEdge();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

class PolyOpConnectEdge : public PolyOperation
{
private:
	int mSegments;
	int	mPinch;
	int	mSlide; 
	ConnectEdgeGrip  *mConnectEdgeGrip;
	bool                    mGripShown;

public:
	PolyOpConnectEdge()
	{
		mSegments	= 2;
		mPinch		= 0;
		mSlide		= 0;
		mGripShown = false;
		mConnectEdgeGrip = NULL;
	}

	// Implement PolyOperation methods:
	int OpID () { return ep_op_connect_edge; }
	bool SelLevelSupport (int selLevel) { return (selLevel == EPM_SL_EDGE); }
	int MeshSelLevel () { return MNM_SL_EDGE; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_CONNECT_EDGE); }
	int DialogID () { return IDD_SETTINGS_CONNECT_EDGES; }
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (3);
		paramList[0] = epm_connect_edge_segments;
		paramList[1] = epm_connect_edge_pinch;
		paramList[2] = epm_connect_edge_slide;
	}
	void *Parameter (int paramID) 
	{
		if ( paramID == epm_connect_edge_segments)
			return ( &mSegments);
		else if ( paramID == epm_connect_edge_pinch)
			return ( &mPinch);
		else if (paramID == epm_connect_edge_slide)
			return (&mSlide);
		else 
			return NULL; 
	}
	bool ChangesSelection () { return true; }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpConnectEdge *ret = new PolyOpConnectEdge ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }

	bool HasGrip() { return true; }
	bool IsGripShown();
	void ShowGrip( EditPolyMod *p );
	void HideGrip();
};

bool PolyOpConnectEdge::IsGripShown()
{
	return mGripShown;
}

void PolyOpConnectEdge::ShowGrip( EditPolyMod *pEditPoly )
{
	if(mGripShown==false)
	{
		//always enabel and use the static one!
		mConnectEdgeGrip = &pEditPoly->mConnectEdgeGrip;
		mConnectEdgeGrip->Init( pEditPoly, this);
		GetIGripManager()->SetGripActive(mConnectEdgeGrip);
		mGripShown = true;
	}
}

void PolyOpConnectEdge::HideGrip()
{
	if(mGripShown==true)
	{
		mGripShown = false;
		if(mConnectEdgeGrip)
			GetIGripManager()->SetGripsInactive();
	}
}

bool PolyOpConnectEdge::Do (MNMesh & mesh, LocalPolyOpData *pOpData) {
	// Deselect our target edges, so that only the connecting edges wind up selected
	mesh.ClearEFlags (MN_SEL);
	if ( mSegments < 1 )
		mSegments = 1;

	IMNMeshUtilities8* l_meshToBridge = static_cast<IMNMeshUtilities8*>(mesh.GetInterface( IMNMESHUTILITIES8_INTERFACE_ID ));

	bool l_ret = false; 

	if (l_meshToBridge != NULL )
	{
		l_ret = l_meshToBridge->ConnectEdges (MN_EDITPOLY_OP_SELECT, mSegments, mPinch, mSlide);
	}
	
	return l_ret;
}

class PolyOpWeldEdge : public PolyOperation
{
private:
	float mThreshold;
	bool mGripShown;
	WeldEdgesGrip  *mWeldEdgesGrip;
	
public:
	PolyOpWeldEdge () : mThreshold(0.0f),mGripShown(false),mWeldEdgesGrip(NULL) { }

	// Implement PolyOperation methods:
	int OpID () { return ep_op_weld_edge; }
	bool SelLevelSupport (int selLevel) { return meshSelLevel[selLevel] == EPM_SL_EDGE; }
	int MeshSelLevel () { return MNM_SL_EDGE; }

	TCHAR *Name () { return GetString (IDS_EDITPOLY_WELD_EDGE); }
	int DialogID () { return IDD_SETTINGS_WELD_EDGES; }
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (1);
		paramList[0] = epm_weld_edge_threshold;
	}
	void *Parameter (int paramID) {
		return (paramID == epm_weld_edge_threshold) ? &mThreshold : NULL;
	}
	bool CanDelete () { return true; }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpWeldEdge *ret = new PolyOpWeldEdge();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }

	bool HasGrip() { return true; }
	bool IsGripShown();
	void ShowGrip( EditPolyMod *p );
	void HideGrip();
};

bool PolyOpWeldEdge::IsGripShown()
{
	return mGripShown;
}

void PolyOpWeldEdge::ShowGrip(EditPolyMod *pEditPoly)
{
	if(mGripShown==false)
	{
		//always enabel and use the static one!
		mWeldEdgesGrip = &pEditPoly->mWeldEdgesGrip;
		mWeldEdgesGrip->Init(pEditPoly, this);
		GetIGripManager()->SetGripActive(mWeldEdgesGrip);
		mGripShown = true;
	}
}


void PolyOpWeldEdge::HideGrip()
{
	if(mGripShown==true)
	{
		mGripShown = false;
		if(mWeldEdgesGrip)
			GetIGripManager()->SetGripsInactive();
	}
}

bool PolyOpWeldEdge::Do(MNMesh & mesh, LocalPolyOpData *pOpData)
{
	// Mark the corresponding vertices:
	mesh.ClearVFlags (MN_USER);
	mesh.PropegateComponentFlags (MNM_SL_VERTEX, MN_USER, MNM_SL_EDGE, MN_EDITPOLY_OP_SELECT);

	// Weld border edges together via border vertices:
	mesh.ClearVFlags (MN_VERT_WELDED);
	bool ret = mesh.WeldBorderVerts (mThreshold, MN_USER);
	ret |= mesh.WeldOpposingEdges (MN_EDITPOLY_OP_SELECT);
	if (!ret) return false;

	// Check for edges for which one end has been welded but not the other.
	// This is made possible by the MN_VERT_WELDED flag which is set in WeldBorderVerts.
	for (int i=0; i<mesh.nume; i++) {
		if (mesh.e[i].GetFlag (MN_DEAD)) continue;
		if (!mesh.e[i].GetFlag (MN_EDITPOLY_OP_SELECT)) continue;
		int v1 = mesh.e[i].v1;
		int v2 = mesh.e[i].v2;

		// If both were welded, or both were not welded, continue.
		if (mesh.v[v1].GetFlag (MN_VERT_WELDED) == mesh.v[v2].GetFlag (MN_VERT_WELDED)) continue;

		// Ok, one's been welded but the other hasn't.  See if there's another weld we can do to seal it up.
		int weldEnd = mesh.v[v1].GetFlag (MN_VERT_WELDED) ? 0 : 1;
		int unweldEnd = 1-weldEnd;
		int va = mesh.e[i][weldEnd];
		int vb = mesh.e[i][unweldEnd];

		// First of all, if the welded vert only has 2 flagged, open edges, it's clear they should be welded together.
		// Use this routine to find all flagged, open edges touching va that *aren't* i:
		Tab<int> elist;
		for (int j=0; j<mesh.vedg[va].Count(); j++) {
			int eid = mesh.vedg[va][j];
			if (eid == i) continue;
			if (mesh.e[eid].f2 > -1) continue;
			if (!mesh.e[eid].GetFlag (MN_EDITPOLY_OP_SELECT)) continue;
			if (mesh.e[eid][unweldEnd] != va) continue;	// should have va at the opposite end from edge i.
			elist.Append (1, &eid);
		}
		if (elist.Count() != 1) {
			// Give up for now.  (Perhaps make better solution later.)
			continue;
		}

		// Otherwise, there's exactly one, and we'll want to weld it to edge i:
		int vc = mesh.e[elist[0]].OtherVert (va);
		if (mesh.v[vc].GetFlag (MN_VERT_WELDED)) continue;	// might happen if vc already welded.

		// Ok, now we know which vertices to weld.
		Point3 dest = (mesh.v[vb].p + mesh.v[vc].p)*.5f;
		if (mesh.WeldBorderVerts (vb, vc, &dest)) mesh.v[vb].SetFlag (MN_VERT_WELDED);
	}

	return true;
}

class PolyOpTargetWeldEdge : public PolyOperation
{
public:
	PolyOpTargetWeldEdge () { }

	// Implement PolyOperation methods:
	int OpID () { return ep_op_target_weld_edge; }
	bool SelLevelSupport (int selLevel) { return true; }
	int MeshSelLevel () { return MNM_SL_OBJECT; }	// ignores current selection.

	TCHAR *Name () { return GetString (IDS_EDITPOLY_TARGET_WELD_EDGE); }
	// No parameters or dialog.
	bool CanDelete () { return true; }
	bool CanAnimate() { return false; }

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpTargetWeldEdge *ret = new PolyOpTargetWeldEdge();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpTargetWeldEdge::Do (MNMesh & mesh, LocalPolyOpData *pOpData) {
	if (!pOpData || (pOpData->OpID() != OpID())) return false;

	LocalTargetWeldVertexData *pWeld = (LocalTargetWeldVertexData *)pOpData;
	if (pWeld->NumWelds()<1) return false;

	IntTab edgeMap;
	if (pWeld->NumWelds()>1) edgeMap.SetCount (mesh.nume);

	bool ret=false;
	for (int weld=0; weld<pWeld->NumWelds(); weld++)
	{
		int e1 = pWeld->GetStart(weld);
		int e2 = pWeld->GetTarget(weld);

		// Protect against changes in underlying topology.
		if ((e1>=mesh.nume) || (e2>=mesh.nume)) continue;

		if (weld>0) {
			// Convert to input mesh indices, since we haven't collapsed the dead yet.
         int j, numLiving;
			for (j=0, numLiving=0; j<edgeMap.Count(); j++) {
				if (mesh.e[j].GetFlag (MN_DEAD)) continue;
				edgeMap[numLiving] = j;
				numLiving++;
			}

			// Protect against changes in underlying topology.
			if ((e1>=numLiving) || (e2>=numLiving)) continue;

			e1 = edgeMap[e1];
			e2 = edgeMap[e2];
		}

			ret |= mesh.WeldBorderEdges (e1, e2);
	}

	return ret;
}

class PolyOpExtrudeEdge : public PolyOperation
{
private:
	float mWidth, mHeight;
    ExtrudeEdgeGrip  *mExtrudeEdgeGrip;
	bool                    mGripShown;
public:
	PolyOpExtrudeEdge () : mWidth(0.0f), mHeight(0.0f),mGripShown(false) { }
	~PolyOpExtrudeEdge() { HideGrip(); }

	// Implement PolyOperation methods:
	int OpID () { return ep_op_extrude_edge; }
	bool SelLevelSupport (int selLevel) { return meshSelLevel[selLevel] == MNM_SL_EDGE; }
	int MeshSelLevel () { return MNM_SL_EDGE; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_EXTRUDE_EDGE); }
	int DialogID () { return IDD_SETTINGS_EXTRUDE_EDGE; }

	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (2);
		paramList[0] = epm_extrude_edge_height;
		paramList[1] = epm_extrude_edge_width;
	}
	void *Parameter (int paramID) {
		if (paramID == epm_extrude_edge_height) return &mHeight;
		if (paramID == epm_extrude_edge_width) return &mWidth;
		return NULL;
    }
    void RescaleWorldUnits (float f)
    {
        mWidth *= f;
        mHeight *= f;
    }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpExtrudeEdge *ret = new PolyOpExtrudeEdge();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }

	bool HasGrip() { return true; }
	bool IsGripShown();
	void ShowGrip( EditPolyMod *p );
	void HideGrip();
};

bool PolyOpExtrudeEdge::IsGripShown()
{
	return mGripShown;
}

void PolyOpExtrudeEdge::ShowGrip( EditPolyMod *pEditPoly )
{
	if(mGripShown==false)
	{
		//always enable and use the static one!
		mExtrudeEdgeGrip = &pEditPoly->mExtrudeEdgeGrip;
		mExtrudeEdgeGrip->Init( pEditPoly, this);
		GetIGripManager()->SetGripActive(mExtrudeEdgeGrip);
		mGripShown = true;
	}
}

void PolyOpExtrudeEdge::HideGrip()
{
	if(mGripShown==true)
	{
		mGripShown = false;
		if(mExtrudeEdgeGrip)
			GetIGripManager()->SetGripsInactive();
	}
}

bool PolyOpExtrudeEdge::Do (MNMesh & mesh, LocalPolyOpData *pOpData) {
	MNChamferData chamData;
	chamData.InitToMesh(mesh);
	Tab<Point3> tUpDir;
	tUpDir.SetCount (mesh.numv);

	// Topology change:
	if (!mesh.ExtrudeEdges (MN_EDITPOLY_OP_SELECT, &chamData, tUpDir)) return false;

	// Apply map changes based on base width:
	int i;
	Tab<UVVert> tMapDelta;
	for (int mapChannel=-NUM_HIDDENMAPS; mapChannel<mesh.numm; mapChannel++) {
		if (mesh.M(mapChannel)->GetFlag (MN_DEAD)) continue;
		chamData.GetMapDelta (mesh, mapChannel, mWidth, tMapDelta);
		UVVert *pMapVerts = mesh.M(mapChannel)->v;
		if (!pMapVerts) continue;
		for (i=0; i<mesh.M(mapChannel)->numv; i++) pMapVerts[i] += tMapDelta[i];
	}

	// Apply geom changes based on base width:
	Tab<Point3> tDelta;
	chamData.GetDelta (mWidth, tDelta);
	for (i=0; i<mesh.numv; i++) mesh.v[i].p += tDelta[i];

	// Move the points up:
	for (i=0; i<tUpDir.Count(); i++) mesh.v[i].p += tUpDir[i]*mHeight;
	return true;
}

class PolyOpChamferEdge : public PolyOperation
{
private:
	float mAmount;
	bool  mOpen; 
	int mSegments;
	bool mGripShown;
	ChamferEdgeGrip  *mChamferEdgeGrip;

public:
	PolyOpChamferEdge () : mAmount(0.0f), mOpen(false), mSegments(1),mGripShown(false),mChamferEdgeGrip(NULL) { }
	~PolyOpChamferEdge() { HideGrip(); }
	// Implement PolyOperation methods:
	int OpID () { return ep_op_chamfer_edge; }
	bool SelLevelSupport (int selLevel) { return meshSelLevel[selLevel] == MNM_SL_EDGE; }
	int MeshSelLevel () { return MNM_SL_EDGE; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_CHAMFER_EDGE); }
	int DialogID () { return IDD_SETTINGS_CHAMFER_EDGE; }

	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (3);
		paramList[0] = epm_chamfer_edge;
		paramList[1] = epm_chamfer_edge_open;
		paramList[2] = epm_edge_chamfer_segments;
	}
	void *Parameter (int paramID) {
		if (paramID == epm_chamfer_edge) return &mAmount;
		if (paramID == epm_chamfer_edge_open) return &mOpen;
		if (paramID == epm_edge_chamfer_segments) return &mSegments;
		return NULL;
	}
	bool ChangesSelection () { return true; }
    void RescaleWorldUnits (float f)
    {
        mAmount *= f;
    }

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpChamferEdge *ret = new PolyOpChamferEdge();
		CopyBasics (ret);
		ret->mAmount = mAmount;
		ret->mOpen = mOpen;
		ret->mSegments = mSegments;
		return ret;
	}
	void DeleteThis () { delete this; }

	bool HasGrip() { return true; }
	bool IsGripShown();
	void ShowGrip( EditPolyMod *p );
	void HideGrip();
};

bool PolyOpChamferEdge::IsGripShown()
{
	return mGripShown;
}

void PolyOpChamferEdge::ShowGrip(EditPolyMod *pEditPoly)
{
	if(mGripShown==false)
	{
		//always enabel and use the static one!
		mChamferEdgeGrip = &pEditPoly->mChamferEdgeGrip;
		mChamferEdgeGrip->Init(pEditPoly, this);
		GetIGripManager()->SetGripActive(mChamferEdgeGrip);
		mGripShown = true;
	}
}


void PolyOpChamferEdge::HideGrip()
{
	if(mGripShown==true)
	{
		mGripShown = false;
		if(mChamferEdgeGrip)
			GetIGripManager()->SetGripsInactive();
	}
}

bool PolyOpChamferEdge::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	MNChamferData pMeshChamData;
	MNChamferData10 pMeshChamData10(pMeshChamData);

	DWORD l_flag = MN_EDITPOLY_OP_SELECT; 

	IMNMeshUtilities10* l_meshToChamfer = static_cast<IMNMeshUtilities10*>(mesh.GetInterface( IMNMESHUTILITIES10_INTERFACE_ID ));

	bool l_ret = false; 

	if (l_meshToChamfer != NULL )
	{
		l_ret = l_meshToChamfer->ChamferEdges (l_flag, pMeshChamData10,mOpen, mSegments);
	}

	if ( l_ret )
	{

		Tab<UVVert> mapDelta;
		for (int mapChannel = -NUM_HIDDENMAPS; mapChannel<mesh.numm; mapChannel++) {
			if (mesh.M(mapChannel)->GetFlag (MN_DEAD)) continue;
			pMeshChamData10.GetMapDelta (mesh, mapChannel, mAmount, mapDelta);
			for (int i=0; i<mapDelta.Count(); i++) mesh.M(mapChannel)->v[i] += mapDelta[i];
		}

		Tab<Point3> vertexDelta;
		pMeshChamData10.GetDelta (mAmount, vertexDelta);
		for (int i=0; i<vertexDelta.Count(); i++) mesh.P(i) += vertexDelta[i];
	}

	return l_ret;
}

class PolyOpCapHoles : public PolyOperation
{
public:
	// Implement PolyOperation methods:
	int OpID () { return ep_op_cap; }
	bool SelLevelSupport (int selLevel) { return (selLevel == EPM_SL_BORDER); }
	int MeshSelLevel () { return MNM_SL_EDGE; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_CAP_HOLES); }
	// No dialog ID.
	// No parameters.
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpCapHoles *ret = new PolyOpCapHoles();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpCapHoles::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	MNMeshBorder brdr;
	mesh.GetBorder (brdr, MNM_SL_EDGE, MN_EDITPOLY_OP_SELECT);
   int i;
	for (i=0; i<brdr.Num(); i++) if (brdr.LoopTarg (i)) break;
	if (i>= brdr.Num()) return false;	// Nothing to do.

	mesh.FillInBorders (&brdr);

	return true;
}

LocalInsertVertexFaceData::LocalInsertVertexFaceData (LocalInsertVertexFaceData *pFrom) : 
	mFace(pFrom->mFace), mNumBary(pFrom->mNumBary)
{
	if (pFrom->mBary.Count() == 0) return;

	mBary.SetCount (pFrom->mBary.Count());
	for (int i=0; i<mBary.Count(); i++)
	{
		mBary[i] = new float[mNumBary[i]];
		memcpy (mBary[i], pFrom->mBary[i], mNumBary[i]*sizeof(float));
	}
}

LocalInsertVertexFaceData::~LocalInsertVertexFaceData ()
{
	for (int i=0; i<mBary.Count(); i++) delete [] mBary[i];
}

void LocalInsertVertexFaceData::InsertVertex (int face, Tab<float> & bary)
{
	mFace.Append (1, &face);
	int num = bary.Count();
	mNumBary.Append (1, &num);
	float *bcopy = new float[num];
	memcpy (bcopy, bary.Addr(0), num*sizeof(float));
	mBary.Append (1, &bcopy);
}

void LocalInsertVertexFaceData::RemoveLast ()
{
	int last = mFace.Count()-1;
	if (last<0) return;
	mFace.Delete (last,1);
	mNumBary.Delete (last,1);
	delete [] mBary[last];
	mBary.Delete (last,1);
}

const USHORT kNumDivideFaces = 0x1608;
const USHORT kDivideFace = 0x1610;

IOResult LocalInsertVertexFaceData::Save(ISave *isave) {
	ULONG nb;
	int num = mFace.Count();
	if (num)
	{
		isave->BeginChunk (kNumDivideFaces);
		isave->Write (&num, sizeof(int), &nb);
		isave->EndChunk ();

		for (int i=0; i<num; i++)
		{
			isave->BeginChunk (kDivideFace);
			isave->Write (&i, sizeof(int), &nb);
			isave->Write (mFace.Addr(i), sizeof(int), &nb);
			isave->Write (mNumBary.Addr(i), sizeof(int), &nb);
			isave->Write (mBary[i], mNumBary[i]*sizeof(float), &nb);
			isave->EndChunk ();
		}
	}

	return IO_OK;
}

IOResult LocalInsertVertexFaceData::Load(ILoad *iload)
{
	IOResult res;
	int num, which;
	ULONG nb;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID ())
		{
		case kNumDivideFaces:
			if ((res = iload->Read (&num, sizeof(int), &nb)) != IO_OK) break;
			mFace.SetCount (num);
			mNumBary.SetCount (num);
			mBary.SetCount (num);
			break;

		case kDivideFace:
			if ((res = iload->Read (&which, sizeof(int), &nb)) != IO_OK) break;
			if ((res = iload->Read (mFace.Addr(which), sizeof(int), &nb)) != IO_OK) break;
			if ((res = iload->Read (mNumBary.Addr(which), sizeof(int), &nb)) != IO_OK) break;
			mBary[which] = new float[mNumBary[which]];
			res = iload->Read (mBary[which], mNumBary[which]*sizeof(float), &nb);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

class PolyOpInsertVertexInFace : public PolyOperation {
public:
	// Implement PolyOperation methods:
	int OpID () { return ep_op_insert_vertex_face; }
	bool SelLevelSupport (int selLevel) { return (selLevel == EPM_SL_FACE); }
	int MeshSelLevel () { return MNM_SL_FACE; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_INSERT_VERTEX_IN_FACE); }
	// No dialog ID.
	// No parameters.
	bool ChangesSelection () { return true; }
	bool CanDelete() { return true; }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpInsertVertexInFace *ret = new PolyOpInsertVertexInFace();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpInsertVertexInFace::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	if (!pOpData || (pOpData->OpID() != OpID())) return false;

	LocalInsertVertexFaceData *pInsert = (LocalInsertVertexFaceData *)pOpData;
	if (pInsert->NumInserts()<1) return false;

	Tab<float> bary;
	for (int i=0; i<pInsert->NumInserts(); i++)
	{
		int face = pInsert->GetFace(i);
		if (face>=mesh.numf) continue;	// face out of range.
		if (i>0) {
			// Convert to input mesh indices, since we haven't collapsed the dead yet.
         int j, numLiving;
			for (j=0, numLiving=0; j<mesh.numf; j++) {
				if (mesh.f[j].GetFlag (MN_DEAD)) continue;
				if (numLiving == face)
				{
					face = j;
					break;
				}
				numLiving++;
			}
			if (j==mesh.numf) continue;	// face out of range.
		}
		bary.SetCount (pInsert->GetNumBary(i), false);
		memcpy (bary.Addr(0), pInsert->GetBary(i), pInsert->GetNumBary(i)*sizeof(float));
		int nv = mesh.DivideFace (face, bary);
		mesh.v[nv].SetFlag (MN_SEL);
	}
	return true;
}

class PolyOpExtrudeFace : public PolyOperation
{
private:
	float mHeight;
	int mType;
	ExtrudeFaceGrip   *mExtrudeFaceGrip;
	bool                     mGripShown;
public:
	PolyOpExtrudeFace () : mHeight(0.0f), mType(0),mGripShown(false) { }
	~PolyOpExtrudeFace(){ HideGrip(); }

	// Implement PolyOperation methods:
	int OpID () { return ep_op_extrude_face; }
	bool SelLevelSupport (int selLevel) { return meshSelLevel[selLevel] == MNM_SL_FACE; }
	int MeshSelLevel () { return MNM_SL_FACE; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_EXTRUDE_FACE); }
	int DialogID () { return IDD_SETTINGS_EXTRUDE_FACE; }

	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (2);
		paramList[0] = epm_extrude_face_height;
		paramList[1] = epm_extrude_face_type;
	}
	void *Parameter (int paramID) {
		if (paramID == epm_extrude_face_height) return &mHeight;
		if (paramID == epm_extrude_face_type) return &mType;
		return NULL;
    }
    void RescaleWorldUnits (float f)
    {
        mHeight *= f;
    }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpExtrudeFace *ret = new PolyOpExtrudeFace();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }

	bool HasGrip()  { return true; }
	bool IsGripShown();
	void ShowGrip( EditPolyMod *p );
	void HideGrip();
};

bool PolyOpExtrudeFace::IsGripShown()
{
	return mGripShown;
}

void PolyOpExtrudeFace::ShowGrip( EditPolyMod *pEditPoly )
{
	if(mGripShown==false)
	{
		//always enabel and use the static one!
		mExtrudeFaceGrip = &pEditPoly->mExtrudeFaceGrip;
		mExtrudeFaceGrip->Init( pEditPoly, this);
		GetIGripManager()->SetGripActive(mExtrudeFaceGrip);
		mGripShown = true;
	}
}

void PolyOpExtrudeFace::HideGrip()
{
	if(mGripShown==true)
	{
		mGripShown = false;
		if(mExtrudeFaceGrip)
			GetIGripManager()->SetGripsInactive();
	}
}

bool PolyOpExtrudeFace::Do(MNMesh & mesh, LocalPolyOpData *pOpData)
{
	MNChamferData chamData;
	if (mType<2) {
		MNFaceClusters fclust(mesh, MN_EDITPOLY_OP_SELECT);
		if (!mesh.ExtrudeFaceClusters (fclust)) return false;
		if (mType == 0) {
			// Get fresh face clusters:
			MNFaceClusters fclustAfter(mesh, MN_EDITPOLY_OP_SELECT);
			Tab<Point3> clusterNormals, clusterCenters;
			fclustAfter.GetNormalsCenters (mesh, clusterNormals, clusterCenters);
			mesh.GetExtrudeDirection (&chamData, &fclustAfter, clusterNormals.Addr(0));
		} else {
			mesh.GetExtrudeDirection (&chamData, MN_EDITPOLY_OP_SELECT);
		}
	} else {
		// Polygon-by-polygon extrusion.
		if (!mesh.ExtrudeFaces (MN_EDITPOLY_OP_SELECT)) return false;
		MNFaceClusters fclustAfter(mesh, MN_EDITPOLY_OP_SELECT);
		Tab<Point3> clusterNormals, clusterCenters;
		fclustAfter.GetNormalsCenters (mesh, clusterNormals, clusterCenters);
		mesh.GetExtrudeDirection (&chamData, &fclustAfter, clusterNormals.Addr(0));
	}

	// Move vertices
	for (int i=0; i<mesh.numv; i++) {
		if (mesh.v[i].GetFlag (MN_DEAD)) continue;
		mesh.v[i].p += chamData.vdir[i]*mHeight;
	}
	return true;
}

class PolyOpBevelFace : public PolyOperation
{
private:
	float mHeight, mOutline;
	int mType;

	BevelGrip	*mBevelGrip; //set from EditPolyMod::mBevelGrip
	bool mGripShown;//TODO maybe use IGripManager->GetActiveGrip()==&mBevelGrip????

public:
	PolyOpBevelFace () : mHeight(0.0f), mOutline(0.0f), mType(0),mGripShown(false) { }
	~PolyOpBevelFace (){HideGrip();}
	// Implement PolyOperation methods:
	int OpID () { return ep_op_bevel; }
	bool SelLevelSupport (int selLevel) { return meshSelLevel[selLevel] == MNM_SL_FACE; }
	int MeshSelLevel () { return MNM_SL_FACE; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_BEVEL_FACE); }
	int DialogID () { return IDD_SETTINGS_BEVEL; }

	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (3);
		paramList[0] = epm_bevel_height;
		paramList[1] = epm_bevel_type;
		paramList[2] = epm_bevel_outline;
	}
	void *Parameter (int paramID) {
		if (paramID == epm_bevel_height) return &mHeight;
		if (paramID == epm_bevel_type) return &mType;
		if (paramID == epm_bevel_outline) return &mOutline;
		return NULL;
	}
	void RescaleWorldUnits (float f) { mHeight *= f; mOutline *= f; }

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpBevelFace *ret = new PolyOpBevelFace();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }

	bool HasGrip() {return true;}
	bool IsGripShown();
	void ShowGrip(EditPolyMod *p);
	void HideGrip();

};

bool PolyOpBevelFace::IsGripShown()
{
	return mGripShown;
}

void PolyOpBevelFace::ShowGrip(EditPolyMod *pEditPoly)
{
	if(mGripShown==false)
	{
		//always enabel and use the static one!
		mBevelGrip = &pEditPoly->mBevelGrip;
		mBevelGrip->Init(pEditPoly, this);
		GetIGripManager()->SetGripActive(mBevelGrip);
		mGripShown = true;
	}
}


void PolyOpBevelFace::HideGrip()
{
	if(mGripShown==true)
	{
		mGripShown = false;
		if(mBevelGrip)
			GetIGripManager()->SetGripsInactive();
	}
}

bool PolyOpBevelFace::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	// Topological extrusion first:
	MNChamferData chamData;
	if (mType<2) {
		MNFaceClusters fclust(mesh, MN_EDITPOLY_OP_SELECT);
		if (!mesh.ExtrudeFaceClusters (fclust)) return false;
		if (mType == 0) {
			// Get fresh face clusters:
			MNFaceClusters fclustAfter(mesh, MN_EDITPOLY_OP_SELECT);
			Tab<Point3> clusterNormals, clusterCenters;
			fclustAfter.GetNormalsCenters (mesh, clusterNormals, clusterCenters);
			mesh.GetExtrudeDirection (&chamData, &fclustAfter, clusterNormals.Addr(0));
		} else {
			mesh.GetExtrudeDirection (&chamData, MN_EDITPOLY_OP_SELECT);
		}
	} else {
		// Polygon-by-polygon extrusion.
		if (!mesh.ExtrudeFaces (MN_EDITPOLY_OP_SELECT)) return false;
		MNFaceClusters fclustAfter(mesh, MN_EDITPOLY_OP_SELECT);
		Tab<Point3> clusterNormals, clusterCenters;
		fclustAfter.GetNormalsCenters (mesh, clusterNormals, clusterCenters);
		mesh.GetExtrudeDirection (&chamData, &fclustAfter, clusterNormals.Addr(0));
	}

	int i;
	if (mHeight) {
		for (i=0; i<mesh.numv; i++) mesh.v[i].p += chamData.vdir[i]*mHeight;
	}

	if (mOutline) {
		MNTempData temp(&mesh);
		Tab<Point3> *outDir;
		if (mType == 0) outDir = temp.OutlineDir (MESH_EXTRUDE_CLUSTER, MN_EDITPOLY_OP_SELECT);
		else outDir = temp.OutlineDir (MESH_EXTRUDE_LOCAL, MN_EDITPOLY_OP_SELECT);
		if (outDir && outDir->Count()) {
			Point3 *od = outDir->Addr(0);
			for (i=0; i<mesh.numv; i++) mesh.v[i].p += od[i]*mOutline;
		}
	}

	return true;
}

class PolyOpInsetFace : public PolyOperation
{
private:
	float mAmount;
	int mType;
	InsetGrip  *mInsetGrip;
	bool  mGripShown;

public:
	PolyOpInsetFace () : mAmount(0.0f), mType(0),mGripShown(false) { }
	~PolyOpInsetFace(){ HideGrip(); }

	// Implement PolyOperation methods:
	int OpID () { return ep_op_inset; }
	bool SelLevelSupport (int selLevel) { return meshSelLevel[selLevel] == MNM_SL_FACE; }
	int MeshSelLevel () { return MNM_SL_FACE; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_INSET_FACE); }
	int DialogID () { return IDD_SETTINGS_INSET; }

	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (2);
		paramList[0] = epm_inset_type;
		paramList[1] = epm_inset;
	}
	void *Parameter (int paramID) {
		if (paramID == epm_inset_type) return &mType;
		if (paramID == epm_inset) return &mAmount;
		return NULL;
	}
	void RescaleWorldUnits (float f) { mAmount *= f; }

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpInsetFace *ret = new PolyOpInsetFace();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }

	bool HasGrip() { return true; }
	bool IsGripShown();
	void ShowGrip(EditPolyMod *p);
	void HideGrip();
};

bool PolyOpInsetFace::IsGripShown()
{
	return mGripShown;
}

void PolyOpInsetFace::ShowGrip( EditPolyMod *pEditPoly)
{
	if(mGripShown==false)
	{
		//always enabel and use the static one!
		mInsetGrip = &pEditPoly->mInsetGrip;
		mInsetGrip->Init(pEditPoly, this);
		GetIGripManager()->SetGripActive(mInsetGrip);
		mGripShown = true;
	}
}


void PolyOpInsetFace::HideGrip()
{
	if(mGripShown==true)
	{
		mGripShown = false;
		if(mInsetGrip)
			GetIGripManager()->SetGripsInactive();
	}
}

bool PolyOpInsetFace::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	// Topological extrusion first:
	MNChamferData chamData;
	if (mType==0) {
		// Group inset.
		MNFaceClusters fclust(mesh, MN_EDITPOLY_OP_SELECT);
		if (!mesh.ExtrudeFaceClusters (fclust)) return false;
		if (mType == 0) {
			// Get fresh face clusters:
			MNFaceClusters fclustAfter(mesh, MN_EDITPOLY_OP_SELECT);
			Tab<Point3> clusterNormals, clusterCenters;
			fclustAfter.GetNormalsCenters (mesh, clusterNormals, clusterCenters);
			mesh.GetExtrudeDirection (&chamData, &fclustAfter, clusterNormals.Addr(0));
		}
	} else {
		// Polygon-by-polygon inset.
		if (!mesh.ExtrudeFaces (MN_EDITPOLY_OP_SELECT)) return false;
		MNFaceClusters fclustAfter(mesh, MN_EDITPOLY_OP_SELECT);
		Tab<Point3> clusterNormals, clusterCenters;
		fclustAfter.GetNormalsCenters (mesh, clusterNormals, clusterCenters);
		mesh.GetExtrudeDirection (&chamData, &fclustAfter, clusterNormals.Addr(0));
	}

	if (mAmount) {
		MNTempData temp(&mesh);
		Tab<Point3> *outDir;
		if (mType == 0) outDir = temp.OutlineDir (MESH_EXTRUDE_CLUSTER, MN_EDITPOLY_OP_SELECT);
		else outDir = temp.OutlineDir (MESH_EXTRUDE_LOCAL, MN_EDITPOLY_OP_SELECT);
		if (outDir && outDir->Count()) {
			Point3 *od = outDir->Addr(0);
			for (int i=0; i<mesh.numv; i++) mesh.v[i].p -= od[i]*mAmount;
		}
	}

	return true;
}

class PolyOpOutlineFace : public PolyOperation
{
private:
	float mAmount;
	OutlineGrip *mOutlineGrip;
	bool  mGripShown;

public:
	PolyOpOutlineFace () : mAmount(0.0f),mGripShown(false) { }
	~PolyOpOutlineFace(){HideGrip();}

	// Implement PolyOperation methods:
	int OpID () { return ep_op_outline; }
	bool SelLevelSupport (int selLevel) { return meshSelLevel[selLevel] == MNM_SL_FACE; }
	int MeshSelLevel () { return MNM_SL_FACE; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_OUTLINE_FACE); }
	int DialogID () { return IDD_SETTINGS_OUTLINE; }

	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (1);
		paramList[0] = epm_outline;
	}
	void *Parameter (int paramID) {
		if (paramID == epm_outline) return &mAmount;
		return NULL;
	}
	void RescaleWorldUnits (float f) { mAmount *= f; }

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpOutlineFace *ret = new PolyOpOutlineFace();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }

	bool HasGrip(){ return true; }
	bool IsGripShown();
	void ShowGrip(EditPolyMod *p );
	void HideGrip();
};

bool PolyOpOutlineFace::IsGripShown()
{
	return mGripShown;
}

void PolyOpOutlineFace::ShowGrip(EditPolyMod *pEditPoly)
{
	if(mGripShown==false)
	{
		//always enabel and use the static one!
		mOutlineGrip = &pEditPoly->mOutlineGrip;
		mOutlineGrip->Init(pEditPoly, this);
		GetIGripManager()->SetGripActive(mOutlineGrip);
		mGripShown = true;
	}
}


void PolyOpOutlineFace::HideGrip()
{
	if(mGripShown==true)
	{
		mGripShown = false;
		if(mOutlineGrip)
			GetIGripManager()->SetGripsInactive();
	}
}

bool PolyOpOutlineFace::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	if (mAmount == 0) return false;
	if (mesh.numv == 0) return false;

	int i;
	Tab<Point3> tDelta;
	tDelta.SetCount (mesh.numv);
	MNTempData temp;
	temp.SetMesh (&mesh);

	Tab<Point3> *outDir = temp.OutlineDir (MESH_EXTRUDE_CLUSTER, MN_EDITPOLY_OP_SELECT);
	Point3 *od = outDir->Addr(0);
	for (i=0; i<mesh.numv; i++) mesh.v[i].p += od[i]*mAmount;

	return true;
}

const USHORT kHingeEdge = 0x32f0;

IOResult LocalHingeFromEdgeData::Save (ISave *isave) {
	ULONG nb;
	isave->BeginChunk (kHingeEdge);
	isave->Write (&mEdge, sizeof(int), &nb);
	isave->EndChunk ();
	return IO_OK;
}

IOResult LocalHingeFromEdgeData::Load (ILoad *iload) {
	IOResult res;
	ULONG nb;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID ())
		{
		case kHingeEdge:
			res = iload->Read (&mEdge, sizeof(int), &nb);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

class PolyOpHingeFromEdge : public PolyOperation
{
private:
	int mSegments;
	float mAngle;
	Point3 mBase, mDir;
	Matrix3 mMCTM;
	bool mGripShown;
	HingeGrip  *mHingeGrip;

public:
	PolyOpHingeFromEdge () : mSegments(1), mAngle(0.0f), mBase(0,0,0), mDir(0,0,0),mGripShown(false),mHingeGrip(NULL) { }

	// Implement PolyOperation methods:
	int OpID () { return ep_op_hinge_from_edge; }
	bool SelLevelSupport (int selLevel) { return meshSelLevel[selLevel] == MNM_SL_FACE; }
	int MeshSelLevel () { return MNM_SL_FACE; }

	TCHAR * Name () { return GetString (IDS_HINGE_FROM_EDGE); }
	int DialogID () { return IDD_SETTINGS_HINGE_FROM_EDGE; }
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (4);
		paramList[0] = epm_hinge_angle;
		paramList[1] = epm_hinge_base;
		paramList[2] = epm_hinge_dir;
		paramList[3] = epm_hinge_segments;
	}
	void *Parameter (int paramID) {
		if (paramID == epm_hinge_angle) return &mAngle;
		if (paramID == epm_hinge_base) return &mBase;
		if (paramID == epm_hinge_dir) return &mDir;
		if (paramID == epm_hinge_segments) return &mSegments;
		return NULL;
	}
	int ParameterSize (int paramID)
	{
		switch (paramID) {
		case epm_hinge_angle: return sizeof(float);
		case epm_hinge_base: return sizeof(Point3);
		case epm_hinge_dir: return sizeof(Point3);
		default: return sizeof(int);
		}
	}
	Matrix3 *ModContextTM () { return &mMCTM; }

	void RescaleWorldUnits (float f) { mBase *= f; mMCTM.SetTrans(mMCTM.GetTrans()*f); }

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpHingeFromEdge *ret = new PolyOpHingeFromEdge();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }

	bool HasGrip() { return true; }
	bool IsGripShown();
	void ShowGrip( EditPolyMod *p );
	void HideGrip();
};


bool PolyOpHingeFromEdge::IsGripShown()
{
	return mGripShown;
}

void PolyOpHingeFromEdge::ShowGrip(EditPolyMod *pEditPoly)
{
	if(mGripShown==false)
	{
		//always enabel and use the static one!
		mHingeGrip = &pEditPoly->mHingeGrip;
		mHingeGrip->Init(pEditPoly, this);
		GetIGripManager()->SetGripActive(mHingeGrip);
		mHingeGrip->SetUpUI();
		mGripShown = true;
	}
}


void PolyOpHingeFromEdge::HideGrip()
{
	if(mGripShown==true)
	{
		mGripShown = false;
		if(mHingeGrip)
			GetIGripManager()->SetGripsInactive();
	}
}

bool PolyOpHingeFromEdge::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	if (mAngle == 0.0f) return false;

	int hingeEdge(-1);
	if (pOpData) {
		LocalHingeFromEdgeData *pHinge = (LocalHingeFromEdgeData *) pOpData;
		hingeEdge = pHinge->GetEdge();
		if (hingeEdge>=mesh.nume) hingeEdge = -1;
	}

	bool ret = false;
	MNFaceClusters fClust (mesh, MN_EDITPOLY_OP_SELECT);
	if ( mSegments < 1 )
		mSegments = 1;

	if (hingeEdge>-1)
	{
		for (int i=0; i<fClust.count; i++) {
			mesh.LiftFaceClusterFromEdge (hingeEdge, mAngle, mSegments, fClust, i);
		}
	}
	else
	{
		Point3 base = mMCTM * mBase;
		Point3 dir = mMCTM.VectorTransform (mDir);

		MNMeshUtilities mmu(&mesh);
		for (int i=0; i<fClust.count; i++) {
			ret |= mmu.HingeFromEdge (base, dir, mAngle, mSegments, fClust, i);
		}
	}
	return ret;
}

class PolyOpExtrudeAlongSpline : public PolyOperation
{
private:
	Spline3D* mpSpline;
	Matrix3 mSplineXfm;
	Interval mSplineValidity;
	int mSegments, mAlign;
	float mTaper, mTaperCurve, mTwist, mRotation;
	Matrix3 mWorld2Obj, mModContextTM;
	bool  mGripShown;
	ExtrudeAlongSplineGrip  *mExtrudeAlongSplineGrip;

public:
	PolyOpExtrudeAlongSpline () : mpSpline(NULL), mSplineXfm(true), mSegments(10),
		mAlign(true), mTaper(0.0f), mTaperCurve(0.0f), mTwist(0.0f), mRotation(0.0f),mGripShown(false),mExtrudeAlongSplineGrip(NULL) { }
	~PolyOpExtrudeAlongSpline() 
	{
		HideGrip();
		ClearSpline();
	}
	void ClearSpline ();

	// Implement PolyOperation methods:
	int OpID () { return ep_op_extrude_along_spline; }
	bool SelLevelSupport (int selLevel) { return meshSelLevel[selLevel] == MNM_SL_FACE; }
	int MeshSelLevel () { return MNM_SL_FACE; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_EXTRUDE_ALONG_SPLINE); }
	int DialogID () { return IDD_SETTINGS_EXTRUDE_ALONG_SPLINE; }
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (7);
		paramList[0] = epm_extrude_spline_segments;
		paramList[1] = epm_extrude_spline_taper;
		paramList[2] = epm_extrude_spline_taper_curve;
		paramList[3] = epm_extrude_spline_twist;
		paramList[4] = epm_extrude_spline_rotation;
		paramList[5] = epm_extrude_spline_align;
		paramList[6] = epm_world_to_object_transform;
	}
	void *Parameter (int paramID) {
		if (paramID == epm_extrude_spline_segments) return &mSegments;
		if (paramID == epm_extrude_spline_taper) return &mTaper;
		if (paramID == epm_extrude_spline_taper_curve) return &mTaperCurve;
		if (paramID == epm_extrude_spline_twist) return &mTwist;
		if (paramID == epm_extrude_spline_rotation) return &mRotation;
		if (paramID == epm_extrude_spline_align) return &mAlign;
		if (paramID == epm_world_to_object_transform) return &mWorld2Obj;
		return NULL;
	}
	int ParameterSize (int paramID) {
		if (paramID == epm_world_to_object_transform) return sizeof(Matrix3);
		return PolyOperation::ParameterSize (paramID);
	}
	Matrix3 *ModContextTM () { return &mModContextTM; }

	void GetNode (EPolyMod *epoly, TimeValue t, Interval & ivalid);
	void ClearNode (int paramID) {
		if ((paramID<0) || (paramID == epm_extrude_spline_node)) ClearSpline();
	}

	void RescaleWorldUnits (float f);

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpExtrudeAlongSpline *ret = new PolyOpExtrudeAlongSpline();
		CopyBasics (ret);
		if (mpSpline != NULL) ret->mpSpline = new Spline3D(*mpSpline);
		ret->mSplineValidity = FOREVER;
		ret->mSplineXfm = mSplineXfm;
		return ret;
	}
	IOResult Save (ISave *isave);
	IOResult Load (ILoad *iload);
	void DeleteThis () { delete this; }

	bool HasGrip() { return true; }
	bool IsGripShown();
	void ShowGrip( EditPolyMod *p );
	void HideGrip();
};

bool PolyOpExtrudeAlongSpline::IsGripShown()
{
	return mGripShown;
}

void PolyOpExtrudeAlongSpline::ShowGrip(EditPolyMod *pEditPoly)
{
	if(mGripShown==false)
	{
		//always enabel and use the static one!
		mExtrudeAlongSplineGrip = &pEditPoly->mExtrudeAlongSplineGrip;
		mExtrudeAlongSplineGrip->Init(pEditPoly, this);
		GetIGripManager()->SetGripActive(mExtrudeAlongSplineGrip);
		mExtrudeAlongSplineGrip->SetUpUI();
		mGripShown = true;
	}
}


void PolyOpExtrudeAlongSpline::HideGrip()
{
	if(mGripShown==true)
	{
		mGripShown = false;
		if(mExtrudeAlongSplineGrip)
			GetIGripManager()->SetGripsInactive();
	}
}

void PolyOpExtrudeAlongSpline::RescaleWorldUnits (float f) {
	PolyOperation::RescaleWorldUnits (f);	// handles all the regular parameters.

	// Now we need to rescale the spline.
	if (mpSpline) {
		Matrix3 scaleTM(true);
		scaleTM.SetScale (Point3(f,f,f));
		mpSpline->Transform (&scaleTM);
	}
}

void PolyOpExtrudeAlongSpline::GetNode (EPolyMod *epoly, TimeValue t, Interval &ivalid)
{
	IParamBlock2 *pblock = epoly->getParamBlock();

	// Special case for the spline & transform.
	if (!mSplineValidity.InInterval (t) || !mpSpline)
	{
		mSplineValidity.SetInfinite ();
		INode* pSplineNode = NULL;
		pblock->GetValue (epm_extrude_spline_node, t, pSplineNode, mSplineValidity);
		if (pSplineNode == NULL) mpSpline = NULL;
		else
		{
			bool del = FALSE;
			SplineShape *pSplineShape = NULL;
			ObjectState os = pSplineNode->GetObjectRef()->Eval(t);
			if (os.obj->IsSubClassOf(splineShapeClassID)) pSplineShape = (SplineShape *) os.obj;
			else {
				if (!os.obj->CanConvertToType(splineShapeClassID)) return;
				pSplineShape = (SplineShape*)os.obj->ConvertToType (t, splineShapeClassID);
				if (pSplineShape!=os.obj) del = TRUE;
			}
			mSplineValidity &= os.obj->ObjectValidity (t);
			BezierShape & bezShape = pSplineShape->GetShape();

			mSplineXfm = pSplineNode->GetObjectTM (t, &mSplineValidity);
			if (mpSpline != NULL) delete mpSpline;
			mpSpline = new Spline3D(*bezShape.GetSpline(0));

			if (del) delete pSplineShape;
		}
	}

	ivalid &= mSplineValidity;
}

void PolyOpExtrudeAlongSpline::ClearSpline ()
{
	if (mpSpline != NULL)
	{
		delete mpSpline;
		mpSpline = NULL;
	}
	mSplineValidity.SetEmpty ();
}

class FrenetFinder {
	BasisFinder mBasisFinder;
public:
	void ConvertPathToFrenets (Spline3D *pSpline, Matrix3 relativeTransform, Tab<Matrix3> & tFrenets,
						   int numSegs, bool align, float rotateAroundZ);
};

static FrenetFinder theFrenetFinder;

void FrenetFinder::ConvertPathToFrenets (Spline3D *pSpline, Matrix3 relativeTransform, Tab<Matrix3> & tFrenets,
						   int numSegs, bool align, float rotateAroundZ) {
	// Given a path, a sequence of points in 3-space, create transforms
	// for putting a cross-section around each of those points, loft-style.

	// bezShape is provided by user, tFrenets contains output, numSegs is one less than the number of transforms requested.

	// Strategy: The Z-axis is mapped along the path, and the X and Y axes 
	// are chosen in a well-defined manner to get an orthonormal basis.

	int i;

	if (numSegs < 1) return;
	tFrenets.SetCount (numSegs+1);

	int numIntervals = pSpline->Closed() ? numSegs+1 : numSegs;
	float denominator = float(numIntervals);
	Point3 xDir, yDir, zDir, location, tangent, axis;
	float position, sine, cosine, theta;
	Matrix3 rotation;

	// Find initial x,y directions:
	location = relativeTransform * pSpline->InterpCurve3D (0.0f);
	tangent = relativeTransform.VectorTransform (pSpline->TangentCurve3D (0.0f));
	zDir = tangent;

	Matrix3 inverseBasisOfSpline(1);
	if (align) {
		xDir = Point3(0.0f, 0.0f, 0.0f);
		yDir = xDir;
		mBasisFinder.BasisFromZDir (zDir, xDir, yDir);
		if (rotateAroundZ) {
			Matrix3 rotator(1);
			rotator.SetRotate (AngAxis (zDir, rotateAroundZ));
			xDir = xDir * rotator;
			yDir = yDir * rotator;
		}
		Matrix3 basisOfSpline(1);
		basisOfSpline.SetRow (0, xDir);
		basisOfSpline.SetRow (1, yDir);
		basisOfSpline.SetRow (2, tangent);
		basisOfSpline.SetTrans (location);
		inverseBasisOfSpline = Inverse (basisOfSpline);
	} else {
		inverseBasisOfSpline.SetRow (3, -location);
	}

	// Make relative transform take the spline from its own object space to our object space,
	// and from there into the space defined by its origin and initial direction:
	relativeTransform = relativeTransform * inverseBasisOfSpline;
	// (Note left-to-right evaluation order: Given matrices A,B, point x, x(AB) = (xA)B

	// The first transform is necessarily the identity:
	tFrenets[0].IdentityMatrix ();

	// Set up xDir, yDir, zDir to match our first-point basis:
	xDir = Point3 (1,0,0);
	yDir = Point3 (0,1,0);
	zDir = Point3 (0,0,1);

	for (i=1; i<=numIntervals; i++) {
		position = float(i) / denominator;
		location = relativeTransform * pSpline->InterpCurve3D (position);
		tangent = relativeTransform.VectorTransform (pSpline->TangentCurve3D (position));

		// This is the procedure we follow at each step in the path: find the
		// orthonormal basis with the right orientation, then compose with
		// the translation putting the origin at the path-point.

		// As we proceed along the path, we apply minimal rotations to
		// our original basis to keep the Z-axis tangent to the curve.
		// The X and Y axes follow in a natural manner.

		// xDir, yDir, zDir still have their values from last time...
		// Create a rotation matrix which maps the last zDir onto the current tangent:
		axis = zDir ^ tangent;	// gives axis, scaled by sine of angle.
		sine = FLength(axis);	// positive - keeps angle value in (0,PI) range.
		cosine = DotProd (zDir, tangent);	// Gives cosine of angle.
		theta = atan2f (sine, cosine);
		rotation.SetRotate (AngAxis (Normalize(axis), theta));
		Point3 testVector = rotation * zDir;
		xDir = Normalize (rotation * xDir);
		yDir = Normalize (rotation * yDir);
		zDir = tangent;

		if (i<=numSegs) {
			tFrenets[i].IdentityMatrix ();
			tFrenets[i].SetRow (0, xDir);
			tFrenets[i].SetRow (1, yDir);
			tFrenets[i].SetRow (2, tangent);
			tFrenets[i].SetTrans (location);
		}
	}
}

bool PolyOpExtrudeAlongSpline::Do (MNMesh & mesh, LocalPolyOpData *pOpData) {
	if (mpSpline == NULL) return false;

	Matrix3 relativeTM = mSplineXfm * (mWorld2Obj * mModContextTM);
	Tab<Matrix3> tTransforms;
	if ( mSegments < 1 )
		mSegments = 1;

	theFrenetFinder.ConvertPathToFrenets (mpSpline, relativeTM, tTransforms,
		mSegments, mAlign?true:false, mRotation);

	// Apply taper and twist to transforms:
	float denom = float(tTransforms.Count()-1);
	for (int i=1; i<tTransforms.Count(); i++) {
		float amount = float(i)/denom;
		// This equation taken from Taper modifier:
		float taperAmount =	1.0f + amount*mTaper + mTaperCurve*amount*(1.0f-amount);
		if (taperAmount != 1.0f) {
			// Pre-scale matrix by taperAmount.
			tTransforms[i].PreScale (Point3(taperAmount, taperAmount, taperAmount));
		}
		if (mTwist != 0.0f) {
			float twistAmount = mTwist * amount;
			tTransforms[i].PreRotateZ (twistAmount);
		}
	}

	// Note:
	// If there are multiple face clusters, the first call to ExtrudeFaceClusterAlongPath
	// will bring mesh.numf and fClust.clust.Count() out of synch - fClust isn't updated.
	// So we fix that here.
	bool ret = false;
	MNFaceClusters fClust (mesh, MN_EDITPOLY_OP_SELECT);
	for (int i=0; i<fClust.count; i++) {
		if (mesh.ExtrudeFaceClusterAlongPath (tTransforms, fClust, i, mAlign?true:false)) {
			if (i+1<fClust.count) {
				// New faces not in any cluster.
				int oldnumf = fClust.clust.Count();
				fClust.clust.SetCount (mesh.numf);
				for (int j=oldnumf; j<mesh.numf; j++) fClust.clust[j] = -1;
			}
			ret = true;
		}
	}

	return ret;
}

//const USHORT kPolyOperationBasics = 0x0600;
const USHORT kPolyOpExtrudeSpline = 0x0640;
const USHORT kPolyOpExtrudeSplineXfm = 0x0644;

IOResult PolyOpExtrudeAlongSpline::Save (ISave *isave)
{
	isave->BeginChunk (kPolyOperationBasics);
	SaveBasics (isave);
	isave->EndChunk ();

	if (mpSpline != NULL)
	{
		isave->BeginChunk (kPolyOpExtrudeSplineXfm);
		mSplineXfm.Save (isave);
		isave->EndChunk ();

		isave->BeginChunk (kPolyOpExtrudeSpline);
		mpSpline->Save (isave);
		isave->EndChunk ();
	}
	return IO_OK;
}

IOResult PolyOpExtrudeAlongSpline::Load (ILoad *iload)
{
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID ())
		{
		case kPolyOperationBasics:
			res = LoadBasics (iload);
			break;
		case kPolyOpExtrudeSplineXfm:
			res = mSplineXfm.Load (iload);
			break;
		case kPolyOpExtrudeSpline:
			mpSpline = new Spline3D();
			res = mpSpline->Load (iload);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

class PolyOpBridgeBorder : public PolyOperation
{
	float mTaper, mBias, mThresh;
	int mSegments, mTarget1, mTarget2, mTwist1, mTwist2, mUseSelected;
	bool mGripShown;
	BridgeBorderGrip *mBridgeBorderGrip;

public:
	PolyOpBridgeBorder():mGripShown(false){}
	~PolyOpBridgeBorder(){ HideGrip(); }
	// Implement PolyOperation methods:
	int OpID () { return ep_op_bridge_border; }
	bool SelLevelSupport (int selLevel) { return (selLevel == EPM_SL_BORDER); }
	int MeshSelLevel () { return MNM_SL_EDGE; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_BRIDGE_BORDER); }

	int DialogID () { return IDD_SETTINGS_BRIDGE_BORDER; }
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (9);
		paramList[0] = epm_bridge_taper;
		paramList[1] = epm_bridge_bias;
		paramList[2] = epm_bridge_segments;
		paramList[3] = epm_bridge_smooth_thresh;
		paramList[4] = epm_bridge_target_1;
		paramList[5] = epm_bridge_target_2;
		paramList[6] = epm_bridge_twist_1;
		paramList[7] = epm_bridge_twist_2;
		paramList[8] = epm_bridge_selected;
	}
	void *Parameter (int paramID) {
		if (paramID == epm_bridge_taper) return &mTaper;
		if (paramID == epm_bridge_bias) return &mBias;
		if (paramID == epm_bridge_segments) return &mSegments;
		if (paramID == epm_bridge_smooth_thresh) return &mThresh;
		if (paramID == epm_bridge_target_1) return &mTarget1;
		if (paramID == epm_bridge_target_2) return &mTarget2;
		if (paramID == epm_bridge_twist_1) return &mTwist1;
		if (paramID == epm_bridge_twist_2) return &mTwist2;
		if (paramID == epm_bridge_selected) return &mUseSelected;
		return NULL;
	}

	bool ChangesSelection () { return true; }
	bool CanDelete() { return true; }

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpBridgeBorder *ret = new PolyOpBridgeBorder();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }

	bool HasGrip() { return true; }
	bool IsGripShown();
	void ShowGrip( EditPolyMod *p );
	void HideGrip();
};


bool PolyOpBridgeBorder::IsGripShown()
{
	return mGripShown;
}

void PolyOpBridgeBorder::ShowGrip(EditPolyMod *pEditPoly)
{
	if(mGripShown==false)
	{
		//always enabel and use the static one!
		mBridgeBorderGrip = &pEditPoly->mBridgeBorderGrip;
		mBridgeBorderGrip->Init(pEditPoly, this);
		GetIGripManager()->SetGripActive(mBridgeBorderGrip);
		mBridgeBorderGrip->SetUpUI();
		mGripShown = true;
	}
}


void PolyOpBridgeBorder::HideGrip()
{
	if(mGripShown==true)
	{
		mGripShown = false;
		if(mBridgeBorderGrip)
			GetIGripManager()->SetGripsInactive();
	}
}


bool PolyOpBridgeBorder::Do (MNMesh & mesh, LocalPolyOpData *pOpdata) {
	int oldnumf = mesh.numf;
	bool ret = false;
	MNMeshUtilities mmu(&mesh);
	if ( mSegments < 1 )
		mSegments = 1;

	if (mUseSelected) {
		ret = mmu.BridgeSelectedBorders (MN_EDITPOLY_OP_SELECT, mThresh, mSegments, mTaper, mBias, mTwist1, -mTwist2);
	} else {
		if (!pOpdata || (pOpdata->OpID() != ep_op_bridge_border)) return false;

		if (mTarget1==0) return false;
		if (mTarget2==0) return false;
		ret = mmu.BridgeBorders (mTarget1-1, mTwist1, mTarget2-1, -mTwist2,
			mThresh, mSegments, mTaper, mBias);
	}

	if (ret) {
		for (int i=0; i<oldnumf; i++) mesh.f[i].ClearFlag (MN_SEL);
		for (int i=oldnumf; i<mesh.numf; i++) mesh.f[i].SetFlag (MN_SEL);
	}

	return ret;
}

class PolyOpBridgePolygon : public PolyOperation
{
	float mTaper, mBias, mThresh;
	int mSegments, mTarget1, mTarget2, mTwist1, mTwist2, mUseSelected;
	bool mGripShown;
	BridgePolygonGrip *mBridgePolygonGrip;

public:
	PolyOpBridgePolygon():mGripShown(false){ }
	~PolyOpBridgePolygon(){ HideGrip(); }
	// Implement PolyOperation methods:
	int OpID () { return ep_op_bridge_polygon; }
	bool SelLevelSupport (int selLevel) { return (selLevel == EPM_SL_FACE); }
	int MeshSelLevel () { return MNM_SL_FACE; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_BRIDGE_POLYGON); }

	int DialogID () { return IDD_SETTINGS_BRIDGE_POLYGONS; }
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (9);
		paramList[0] = epm_bridge_taper;
		paramList[1] = epm_bridge_bias;
		paramList[2] = epm_bridge_segments;
		paramList[3] = epm_bridge_smooth_thresh;
		paramList[4] = epm_bridge_target_1;
		paramList[5] = epm_bridge_target_2;
		paramList[6] = epm_bridge_twist_1;
		paramList[7] = epm_bridge_twist_2;
		paramList[8] = epm_bridge_selected;
	}
	void *Parameter (int paramID) {
		if (paramID == epm_bridge_taper) return &mTaper;
		if (paramID == epm_bridge_bias) return &mBias;
		if (paramID == epm_bridge_segments) return &mSegments;
		if (paramID == epm_bridge_smooth_thresh) return &mThresh;
		if (paramID == epm_bridge_target_1) return &mTarget1;
		if (paramID == epm_bridge_target_2) return &mTarget2;
		if (paramID == epm_bridge_twist_1) return &mTwist1;
		if (paramID == epm_bridge_twist_2) return &mTwist2;
		if (paramID == epm_bridge_selected) return &mUseSelected;
		return NULL;
	}
	bool ChangesSelection () { return true; }
	bool CanDelete() { return true; }

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpBridgePolygon *ret = new PolyOpBridgePolygon();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }

	bool HasGrip() { return true; }
	bool IsGripShown();
	void ShowGrip( EditPolyMod *p );
	void HideGrip();
};

bool PolyOpBridgePolygon::IsGripShown()
{
	return mGripShown;
}

void PolyOpBridgePolygon::ShowGrip(EditPolyMod *pEditPoly)
{
	if(mGripShown==false)
	{
		//always enabel and use the static one!
		mBridgePolygonGrip = &pEditPoly->mBridgePolygonGrip;
		mBridgePolygonGrip->Init(pEditPoly, this);
		GetIGripManager()->SetGripActive(mBridgePolygonGrip);
		mBridgePolygonGrip->SetUpUI();
		mGripShown = true;
	}
}


void PolyOpBridgePolygon::HideGrip()
{
	if(mGripShown==true)
	{
		mGripShown = false;
		if(mBridgePolygonGrip)
			GetIGripManager()->SetGripsInactive();
	}
}



bool PolyOpBridgePolygon::Do (MNMesh & mesh, LocalPolyOpData *pOpdata) {
	int oldnumf = mesh.numf;
	bool ret = false;
	MNMeshUtilities mmu(&mesh);
	
	if ( mSegments < 1 )
		mSegments = 1;

	if (mUseSelected) {
		ret = mmu.BridgePolygonClusters (MN_EDITPOLY_OP_SELECT, mThresh, mSegments, mTaper, mBias, mTwist1, -mTwist2);
	} else {
		if (!pOpdata || (pOpdata->OpID() != ep_op_bridge_polygon)) return false;

		if (mTarget1==0) return false;
		if (mTarget2==0) return false;
		ret = mmu.BridgePolygons (mTarget1-1, mTwist1, mTarget2-1, -mTwist2,
			mThresh, mSegments, mTaper, mBias);
	}

	if (ret) {
		for (int i=0; i<mesh.numf; i++) mesh.f[i].ClearFlag (MN_SEL);
		for (int i=oldnumf; i<mesh.numf; i++) mesh.f[i].SetFlag (MN_SEL);
	}

	return ret;
}

//---------------------------------------------------------------------------------

class PolyOpBridgeEdge : public PolyOperation
{
	float mTaper, mBias, mThresh, mAdjacent;
	int mSegments, mTarget1, mTarget2, mTwist1, mTwist2, mUseSelected;
	BOOL mReverseTriangles;
	bool mGripShown;
	BridgeEdgeGrip *mBridgeEdgeGrip;

public:
	PolyOpBridgeEdge () 
	{
		mTaper				= 0.0; 
		mBias				= 0.0;
		mThresh				= PI/4.0f;
		mAdjacent			= PI/4.0f;
		mSegments			= 1;
		mTarget1			= 0;
		mTarget2			= 0; 
		mTwist1				= 0;
		mTwist2				= 0;
		mUseSelected		= FALSE ;
		mReverseTriangles	= FALSE; 
		mGripShown  = false;
		mBridgeEdgeGrip = NULL;
	}
	~PolyOpBridgeEdge() { HideGrip();}
	// Implement PolyOperation methods:
	int OpID () { return ep_op_bridge_edge; }
	bool SelLevelSupport (int selLevel) { return (selLevel == EPM_SL_EDGE); }
	int MeshSelLevel () { return MNM_SL_EDGE; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_BRIDGE_EDGE); }

	int DialogID () { return IDD_SETTINGS_BRIDGE_EDGE; }
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (11);
		paramList[0] = epm_bridge_taper;
		paramList[1] = epm_bridge_bias;
		paramList[2] = epm_bridge_segments;
		paramList[3] = epm_bridge_smooth_thresh;
		paramList[4] = epm_bridge_target_1;
		paramList[5] = epm_bridge_target_2;
		paramList[6] = epm_bridge_twist_1;
		paramList[7] = epm_bridge_twist_2;
		paramList[8] = epm_bridge_selected;
		paramList[9] = epm_bridge_adjacent;
		paramList[10] = epm_bridge_reverse_triangle;
		
	}
	void *Parameter (int paramID) {
		if (paramID == epm_bridge_taper) return &mTaper;
		if (paramID == epm_bridge_bias) return &mBias;
		if (paramID == epm_bridge_segments) return &mSegments;
		if (paramID == epm_bridge_smooth_thresh) return &mThresh;
		if (paramID == epm_bridge_target_1) return &mTarget1;
		if (paramID == epm_bridge_target_2) return &mTarget2;
		if (paramID == epm_bridge_twist_1) return &mTwist1;
		if (paramID == epm_bridge_twist_2) return &mTwist2;
		if (paramID == epm_bridge_selected) return &mUseSelected;
		if (paramID == epm_bridge_adjacent) return &mAdjacent;
		if (paramID == epm_bridge_reverse_triangle) return &mReverseTriangles;
		
		return NULL;
	}
	bool ChangesSelection () { return true; }
	bool CanDelete() { return true; }				

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpBridgeEdge *ret = new PolyOpBridgeEdge();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }

	bool HasGrip() { return true; }
	bool IsGripShown();
	void ShowGrip( EditPolyMod *p );
	void HideGrip();
};

bool PolyOpBridgeEdge::IsGripShown()
{
	return mGripShown;
}

void PolyOpBridgeEdge::ShowGrip(EditPolyMod *pEditPoly)
{
	if(mGripShown==false)
	{
		//always enabel and use the static one!
		mBridgeEdgeGrip = &pEditPoly->mBridgeEdgeGrip;
		mBridgeEdgeGrip->Init(pEditPoly, this);
		GetIGripManager()->SetGripActive(mBridgeEdgeGrip);
		mBridgeEdgeGrip->SetUpUI();
		mGripShown = true;
	}
}


void PolyOpBridgeEdge::HideGrip()
{
 	if(mGripShown==true)
	{
		mGripShown = false;
		if(mBridgeEdgeGrip)
			GetIGripManager()->SetGripsInactive();
	}
}

bool PolyOpBridgeEdge::Do (MNMesh & mesh, LocalPolyOpData *pOpdata) {
	int oldnumf = mesh.numf;
	bool ret = false;
	IMNMeshUtilities8* l_meshToBridge = static_cast<IMNMeshUtilities8*>(mesh.GetInterface( IMNMESHUTILITIES8_INTERFACE_ID ));

	if ( l_meshToBridge != NULL )
	{
		if ( mSegments < 1 )
			mSegments = 1;

		bool reverseTriangles = (mReverseTriangles) ? true :false;
		if (mUseSelected) 
			ret = l_meshToBridge->BridgeSelectedEdges ( MN_EDITPOLY_OP_SELECT,mThresh, mSegments,mAdjacent, reverseTriangles);
		else 
		{
			if (!pOpdata || (pOpdata->OpID() != ep_op_bridge_edge)) return false;
			
			if (mTarget1==0) return false;
			if (mTarget2==0) return false;

			ret = l_meshToBridge->BridgeTwoEdges (mTarget1-1, mTarget2-1, mSegments);
		}

		if (ret) {
			for (int i=0; i<mesh.numf; i++) mesh.f[i].ClearFlag (MN_SEL);
			for (int i=oldnumf; i<mesh.numf; i++) mesh.f[i].SetFlag (MN_SEL);
		}
	}

	return ret;
}
//----------------------------------------------------------------------------------------------

class PolyOpRetriangulate : public PolyOperation
{
public:
	// Implement PolyOperation methods:
	int OpID () { return ep_op_retriangulate; }
	bool SelLevelSupport (int selLevel) { return (meshSelLevel[selLevel]==MNM_SL_FACE); }
	int MeshSelLevel () { return MNM_SL_FACE; }

	TCHAR * Name () { return GetString (IDS_EDITPOLY_RETRIANGULATE); }
	// No dialog ID.
	// No parameters.
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpRetriangulate *ret = new PolyOpRetriangulate();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpRetriangulate::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	int i;
	bool ret = false;
	for (i=0; i<mesh.numf; i++) {
		if (mesh.f[i].GetFlag (MN_DEAD)) continue;
		if (!mesh.f[i].GetFlag (MN_EDITPOLY_OP_SELECT)) continue;
		mesh.RetriangulateFace (i);
		ret = true;
	}

	return ret;
}

class PolyOpFlipFaceNormals : public PolyOperation
{
public:
	// Implement PolyOperation methods:
	int OpID () { return ep_op_flip_face; }
	bool SelLevelSupport (int selLevel) { return (selLevel==EPM_SL_FACE); }
	int MeshSelLevel () { return MNM_SL_FACE; }

	TCHAR * Name () { return GetString (IDS_FLIP_NORMALS); }
	// No dialog ID.
	// No parameters.
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData) { return mesh.FlipFaceNormals (MN_EDITPOLY_OP_SELECT); }
	PolyOperation *Clone () {
		PolyOpFlipFaceNormals *ret = new PolyOpFlipFaceNormals();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

class PolyOpFlipElementNormals : public PolyOperation
{
public:
	// Implement PolyOperation methods:
	int OpID () { return ep_op_flip_element; }
	bool SelLevelSupport (int selLevel) { return (meshSelLevel[selLevel]==MNM_SL_FACE); }
	int MeshSelLevel () { return MNM_SL_FACE; }

	TCHAR * Name () { return GetString (IDS_FLIP_NORMALS); }
	// No dialog ID.
	// No parameters.
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData) { return mesh.FlipElementNormals (MN_EDITPOLY_OP_SELECT); }
	PolyOperation *Clone () {
		PolyOpFlipElementNormals *ret = new PolyOpFlipElementNormals();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

class PolyOpAutoSmooth : public PolyOperation
{
private:
	float mThreshold;
public:
	PolyOpAutoSmooth () : mThreshold(0.0f) { }
	// Implement PolyOperation methods:
	int OpID () { return ep_op_autosmooth; }
	bool SelLevelSupport (int selLevel) { return (meshSelLevel[selLevel]==MNM_SL_FACE); }
	int MeshSelLevel () { return MNM_SL_FACE; }

	TCHAR * Name () { return GetString (IDS_AUTOSMOOTH); }
	// No dialog ID.
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (1);
		paramList[0] = epm_autosmooth_threshold;
	}
	void *Parameter (int paramID) {
		switch (paramID)
		{
		case epm_autosmooth_threshold: return &mThreshold;
		}
		return NULL;
	}
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpAutoSmooth *ret = new PolyOpAutoSmooth();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpAutoSmooth::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	MNMeshUtilities mmu(&mesh);
	return mmu.AutoSmooth (mThreshold, MN_EDITPOLY_OP_SELECT);
}

class PolyOpSmooth : public PolyOperation
{
private:
	int mSetBits, mClearBits;
public:
	PolyOpSmooth () : mSetBits(0), mClearBits(0) { }
	// Implement PolyOperation methods:
	int OpID () { return ep_op_smooth; }
	bool SelLevelSupport (int selLevel) { return (meshSelLevel[selLevel]==MNM_SL_FACE); }
	int MeshSelLevel () { return MNM_SL_FACE; }

	TCHAR * Name () { return GetString (IDS_SET_SMOOTHING); }
	
	// No dialog ID - parameters handled in main rollouts.
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (2);
		paramList[0] = epm_smooth_group_set;
		paramList[1] = epm_smooth_group_clear;
	}
	void *Parameter (int paramID) {
		switch (paramID)
		{
		case epm_smooth_group_set: return &mSetBits;
		case epm_smooth_group_clear: return &mClearBits;
		}
		return NULL;
	}
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpSmooth *ret = new PolyOpSmooth();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpSmooth::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	DWORD *set = (DWORD *) ((void *) &mSetBits);
	DWORD *clear = (DWORD *) ((void *) &mClearBits);
	if ((*clear == 0) && (*set == 0)) return false;

	DWORD notClear = ~(*clear);
	bool ret = false;
	for (int i=0; i<mesh.numf; i++)
	{
		if (!mesh.f[i].FlagMatch (MN_EDITPOLY_OP_SELECT|MN_DEAD, MN_EDITPOLY_OP_SELECT)) continue;
		mesh.f[i].smGroup &= notClear;
		mesh.f[i].smGroup |= (*set);
		ret = true;
	}
	return ret;
}

class PolyOpSetMaterial : public PolyOperation
{
private:
	int mMaterialID;
public:
	PolyOpSetMaterial () : mMaterialID(0) { }
	// Implement PolyOperation methods:
	int OpID () { return ep_op_set_material; }
	bool SelLevelSupport (int selLevel) { return (meshSelLevel[selLevel]==MNM_SL_FACE); }
	int MeshSelLevel () { return MNM_SL_FACE; }

	TCHAR * Name () { return GetString (IDS_SET_MATERIAL_ID); }
	// No dialog ID - parameters handled in main rollouts.
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (1);
		paramList[0] = epm_material;
	}
	void *Parameter (int paramID) {
		switch (paramID)
		{
		case epm_material: return &mMaterialID;
		}
		return NULL;
	}

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpSetMaterial *ret = new PolyOpSetMaterial();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpSetMaterial::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	MtlID matID = (MtlID) mMaterialID;

	bool ret=false;
	for (int i=0; i<mesh.numf; i++)
	{
		if (!mesh.f[i].FlagMatch (MN_EDITPOLY_OP_SELECT|MN_DEAD, MN_EDITPOLY_OP_SELECT)) continue;
		if (mesh.f[i].material == matID) continue;
		mesh.f[i].material = matID;
		ret = true;
	}
	return ret;
}

class PolyOpPaintDeform : public PolyOperation
{
public:
	PolyOpPaintDeform () { }
	int OpID() { return ep_op_paint_deform; }
	bool SelLevelSupport (int selLevel) { return true; }
	int MeshSelLevel () { return MNM_SL_OBJECT; }

	TCHAR *Name() { return GetString (IDS_PAINTDEFORM); }
	// No dialog (except its usual one), no parameters.
	bool CanAnimate () { return false; }
	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpPaintDeform *ret = new PolyOpPaintDeform ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpPaintDeform::Do (MNMesh & mesh, LocalPolyOpData *pOpData) {
	if (!pOpData) return false;
	if (pOpData->OpID() != OpID()) return false;
	LocalPaintDeformData *pDeform = (LocalPaintDeformData *)pOpData;

	int max = mesh.numv;
	if (pDeform->NumVertices()<max) max = pDeform->NumVertices();
	if (max==0) return false;

	for (int i=0; i<max; i++) mesh.v[i].p = pDeform->Vertices()[i].p;
	return true;
}

const USHORT kChunkLocalTransformOffsetCount = 0x12c3;
const USHORT kChunkLocalTransformOffsets = 0x12c4;
const USHORT kChunkLocalTransformNodeName = 0x12c5;

IOResult LocalTransformData::Save (ISave *isave) {
	ULONG nb;

	int num = mOffset.Count();
	if (num>0) {
		isave->BeginChunk (kChunkLocalTransformOffsetCount);
		isave->Write (&num, sizeof(int), &nb);
		isave->EndChunk ();

		isave->BeginChunk (kChunkLocalTransformOffsets);
		isave->Write (mOffset.Addr(0), num*sizeof(Point3), &nb);
		isave->EndChunk();

		isave->BeginChunk (kChunkLocalTransformNodeName);
		isave->WriteWString (mNodeName);
		isave->EndChunk ();
	}

	return IO_OK;
}

IOResult LocalTransformData::Load (ILoad *iload) {
	IOResult res(IO_OK);
	int num(0);
	ULONG nb(0);
	TCHAR* readString = NULL;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID ())
		{
		case kChunkLocalTransformOffsetCount:
			res = iload->Read (&num, sizeof(int), &nb);
			mOffset.SetCount (num);
			break;

		case kChunkLocalTransformOffsets:
			if (mOffset.Count() == 0) break;
			res = iload->Read (mOffset.Addr(0), mOffset.Count()*sizeof(Point3), &nb);
			break;

		case kChunkLocalTransformNodeName:
			res = iload->ReadWStringChunk (&readString);
			if (res == IO_OK) mNodeName = TSTR(readString);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

void LocalTransformData::RescaleWorldUnits (float f) {
	for (int i=0; i<mOffset.Count(); i++) mOffset[i] *= f;
}

class PolyOpTransform : public PolyOperation {
	int mConstrainType, mPreserveMaps;
	MapBitArray mPreserveMapSettings;

public:
	PolyOpTransform () : mConstrainType(0), mPreserveMaps(false), mPreserveMapSettings(true,false) {}

	int OpID () { return ep_op_transform; }
	bool SelLevelSupport (int selLevel) { return true; }
	int MeshSelLevel () { return MNM_SL_OBJECT; }

	TCHAR *Name () { return GetString (IDS_EDITPOLY_TRANSFORM); }
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (2);
		paramList[0] = epm_constrain_type;
		paramList[1] = epm_preserve_maps;
	}
	void *Parameter (int paramID) {
		switch (paramID) {
			case epm_constrain_type: return &mConstrainType;
			case epm_preserve_maps: return &mPreserveMaps;
		}
		return NULL;
	}
	MapBitArray *PreserveMapSettings () { return &mPreserveMapSettings; }

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpTransform *ret = new PolyOpTransform ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpTransform::Do (MNMesh & mesh, LocalPolyOpData *pOpData) {
	if (pOpData == NULL) return false;
	if (pOpData->OpID() != OpID()) return false;
	LocalTransformData *data = (LocalTransformData *)pOpData;
	if (data->NumVertices() == 0) return false;

	IMNMeshUtilities10* mnu10 = static_cast<IMNMeshUtilities10*>(mesh.GetInterface( IMNMESHUTILITIES10_INTERFACE_ID ));

	int i, max = mesh.numv;
	if (max>data->NumVertices()) max = data->NumVertices();
	if (max<1) return false;

	Tab<Point3> offset;
	offset.SetCount (max);
	for (i=0; i<max; i++) offset[i] = data->CurrentOffset(i);

	if (mConstrainType) {
		MNMeshUtilities mmu(&mesh);
		switch (mConstrainType) {
		case 1:
			mmu.ConstrainDeltaToEdges (&offset, &offset);
			break;
		case 2:
			mmu.ConstrainDeltaToFaces (&offset, &offset);
			break;
		case 3:
			mnu10->ConstrainDeltaToNormals (&offset, &offset);
			break;
		}
	}

	if (mPreserveMaps) {
		MNMapPreserveData imdd;
		imdd.Initialize (mesh, mPreserveMapSettings);
		imdd.Apply (mesh, offset.Addr(0), max);
	}

	for (int i=0; i<max; i++) mesh.v[i].p += offset[i];

	return true;
}

class PolyOpCloneVertex : public PolyOperation
{
public:
	PolyOpCloneVertex () { }

	// Implement PolyOperation methods:
	int OpID () { return ep_op_clone_vertex; }
	bool SelLevelSupport (int selLevel) { return selLevel == EPM_SL_VERTEX; }
	int MeshSelLevel () { return MNM_SL_VERTEX; }

	TCHAR *Name() { return GetString (IDS_EDITPOLY_CLONE_VERTEX); }
	bool ChangesSelection()  { return true; }
	// No dialog or parameters.

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpCloneVertex *ret = new PolyOpCloneVertex ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpCloneVertex::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	int numClones = 0;
	for (int i=0; i<mesh.numv; i++) {
		if (!mesh.v[i].FlagMatch (MN_EDITPOLY_OP_SELECT|MN_DEAD, MN_EDITPOLY_OP_SELECT)) continue;
		numClones++;
	}
	if (!numClones) return false;

	int oldnumv = mesh.numv;
	mesh.setNumVerts (oldnumv+numClones);
	numClones = 0;
	for (int i=0; i<oldnumv; i++) {
		if (!mesh.v[i].FlagMatch (MN_EDITPOLY_OP_SELECT|MN_DEAD, MN_EDITPOLY_OP_SELECT)) continue;
		mesh.v[numClones+oldnumv] = mesh.v[i];
		mesh.v[i].ClearFlag (MN_SEL);
		mesh.v[numClones+oldnumv].SetFlag (MN_SEL);
		numClones++;
	}

	return true;
}

class PolyOpCloneEdge : public PolyOperation
{
public:
	PolyOpCloneEdge () { }

	// Implement PolyOperation methods:
	int OpID () { return ep_op_clone_edge; }
	bool SelLevelSupport (int selLevel) { return (meshSelLevel[selLevel]==MNM_SL_EDGE); }
	int MeshSelLevel () { return MNM_SL_EDGE; }

	TCHAR *Name() { return GetString (IDS_EDITPOLY_CLONE_EDGE); }
	bool ChangesSelection()  { return true; }

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpCloneEdge *ret = new PolyOpCloneEdge ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpCloneEdge::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	MNMeshUtilities mmu(&mesh);
	int oldnume = mesh.nume;
	bool ret = mmu.ExtrudeOpenEdges (MN_EDITPOLY_OP_SELECT, false, false);
	if (ret == false) return false;
	for (int i=0; i<oldnume; i++) mesh.e[i].ClearFlag (MN_SEL);
	return true;
}

class PolyOpCloneFace : public PolyOperation
{
public:
	PolyOpCloneFace () { }

	// Implement PolyOperation methods:
	int OpID () { return ep_op_clone_face; }
	bool SelLevelSupport (int selLevel) { return (selLevel>EPM_SL_VERTEX); }
	int MeshSelLevel () { return MNM_SL_FACE; }

	TCHAR *Name() { return GetString (IDS_EDITPOLY_CLONE_FACE); }
	bool ChangesSelection()  { return true; }

	bool Do (MNMesh & mesh, LocalPolyOpData *pOpData);
	PolyOperation *Clone () {
		PolyOpCloneFace *ret = new PolyOpCloneFace ();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

bool PolyOpCloneFace::Do (MNMesh & mesh, LocalPolyOpData *pOpData)
{
	int numf = mesh.numf;
	mesh.CloneFaces (MN_EDITPOLY_OP_SELECT, true);
	if (numf == mesh.numf) return false;

	for (int i=0; i<numf; i++) mesh.f[i].ClearFlag (MN_SEL);
	for (int i=numf; i<mesh.numf; i++) mesh.f[i].SetFlag (MN_SEL);
	return true;
}

void EditPolyMod::InitializeOperationList ()
{
	if (mOpList.Count() > 0) return;	// already initialized.
	mOpList.SetCount (100);	// overallocate, so we don't have to count carefully.
	int numOps=0;
	mOpList[numOps++] = new PolyOperation ();
	mOpList[numOps++] = new PolyOpCreate ();
	mOpList[numOps++] = new PolyOpCreateEdge ();
	mOpList[numOps++] = new PolyOpDeleteVertex ();
	mOpList[numOps++] = new PolyOpDeleteFace ();
	mOpList[numOps++] = new PolyOpDeleteEdge ();
	mOpList[numOps++] = new PolyOpCollapseVertices ();
	mOpList[numOps++] = new PolyOpCollapseEdges ();
	mOpList[numOps++] = new PolyOpCollapseFaces ();
	mOpList[numOps++] = new PolyOpAttach ();
	mOpList[numOps++] = new PolyOpDetachVertex ();
	mOpList[numOps++] = new PolyOpDetachFace ();
	mOpList[numOps++] = new PolyOpSlice ();
	mOpList[numOps++] = new PolyOpSliceFaces ();
	mOpList[numOps++] = new PolyOpCut ();
	mOpList[numOps++] = new PolyOpMeshSmooth ();
	mOpList[numOps++] = new PolyOpTessellate ();
	mOpList[numOps++] = new PolyOpMakePlanar ();
	mOpList[numOps++] = new PolyOpMakePlanarInX();
	mOpList[numOps++] = new PolyOpMakePlanarInY();
	mOpList[numOps++] = new PolyOpMakePlanarInZ();
	mOpList[numOps++] = new PolyOpAlign ();
	mOpList[numOps++] = new PolyOpRelax ();
	mOpList[numOps++] = new PolyOpHideVertex ();
	mOpList[numOps++] = new PolyOpHideFace ();
	mOpList[numOps++] = new PolyOpHideUnselVertex ();
	mOpList[numOps++] = new PolyOpHideUnselFace ();
	mOpList[numOps++] = new PolyOpUnhideVertex ();
	mOpList[numOps++] = new PolyOpUnhideFace ();

	mOpList[numOps++] = new PolyOpWeldVertex ();
	mOpList[numOps++] = new PolyOpChamferVertex ();
	mOpList[numOps++] = new PolyOpExtrudeVertex ();
	mOpList[numOps++] = new PolyOpBreakVertex ();
	mOpList[numOps++] = new PolyOpRemoveVertex ();
	mOpList[numOps++] = new PolyOpConnectVertex ();
	mOpList[numOps++] = new PolyOpTargetWeldVertex();
	mOpList[numOps++] = new PolyOpRemoveIsolatedVertices ();
	mOpList[numOps++] = new PolyOpRemoveIsolatedMapVertices ();

	mOpList[numOps++] = new PolyOpInsertVertexInEdge ();
	mOpList[numOps++] = new PolyOpChamferEdge ();
	mOpList[numOps++] = new PolyOpExtrudeEdge ();
	mOpList[numOps++] = new PolyOpRemoveEdge ();
	mOpList[numOps++] = new PolyOpSplitEdge ();
	mOpList[numOps++] = new PolyOpWeldEdge ();
	mOpList[numOps++] = new PolyOpTargetWeldEdge();
	mOpList[numOps++] = new PolyOpConnectEdge ();
	mOpList[numOps++] = new PolyOpCapHoles ();
	mOpList[numOps++] = new PolyOpBridgeBorder ();

	mOpList[numOps++] = new PolyOpInsertVertexInFace ();
	mOpList[numOps++] = new PolyOpExtrudeFace ();
	mOpList[numOps++] = new PolyOpBevelFace ();
	mOpList[numOps++] = new PolyOpInsetFace ();
	mOpList[numOps++] = new PolyOpOutlineFace ();
	mOpList[numOps++] = new PolyOpExtrudeAlongSpline ();
	mOpList[numOps++] = new PolyOpHingeFromEdge ();
	mOpList[numOps++] = new PolyOpRetriangulate ();
	mOpList[numOps++] = new PolyOpEditTriangulation ();
	mOpList[numOps++] = new PolyOpTurnEdge ();
	mOpList[numOps++] = new PolyOpFlipFaceNormals ();
	mOpList[numOps++] = new PolyOpFlipElementNormals ();
	mOpList[numOps++] = new PolyOpBridgePolygon ();
	mOpList[numOps++] = new PolyOpBridgeEdge ();

	mOpList[numOps++] = new PolyOpAutoSmooth ();
	mOpList[numOps++] = new PolyOpSmooth ();
	mOpList[numOps++] = new PolyOpSetMaterial ();

	mOpList[numOps++] = new PolyOpPaintDeform ();

	mOpList[numOps++] = new PolyOpTransform ();
	mOpList[numOps++] = new PolyOpCloneVertex ();
	mOpList[numOps++] = new PolyOpCloneEdge ();
	mOpList[numOps++] = new PolyOpCloneFace ();

	// correct the count:
	mOpList.SetCount (numOps);
}

class AlterSelectionEnumProc : public ModContextEnumProc {
private:
	EditPolyMod* mpMod;
	int mSelLevel;
	int mOperation;
public:
	AlterSelectionEnumProc (EditPolyMod *mod, int selLev, int operation)
		: mpMod(mod), mSelLevel(selLev) { mOperation = operation; }
	BOOL proc (ModContext *mc);
};

BOOL AlterSelectionEnumProc::proc (ModContext *mc) {
	if (!mc->localData) return true;
	EditPolyData *epd = (EditPolyData *) mc->localData;

	switch (mOperation)
	{
	case ep_op_sel_grow:
		epd->GrowSelection (mpMod, mSelLevel);
		break;
	case ep_op_sel_shrink:
		epd->ShrinkSelection (mpMod, mSelLevel);
		break;
	case ep_op_hide_vertex:
		epd->Hide (mpMod, false, true);
		break;
	case ep_op_hide_face:
		epd->Hide (mpMod, true, true);
		break;
	case ep_op_hide_unsel_vertex:
		epd->Hide (mpMod, false, false);
		break;
	case ep_op_hide_unsel_face:
		epd->Hide (mpMod, true, false);
		break;

	case ep_op_unhide_vertex:
		epd->Unhide (mpMod, false);
		break;
	case ep_op_unhide_face:
		epd->Unhide (mpMod, true);
		break;

	case ep_op_get_stack_selection:
		epd->AcquireStackSelection (mpMod, mSelLevel);
		break;
	case ep_op_select_ring:
		epd->SelectEdgeRing (mpMod);
		break;
	case ep_op_select_loop:
		epd->SelectEdgeLoop (mpMod);
		break;
	case ep_op_select_by_material:
		epd->SelectByMaterial (mpMod);
		break;
	case ep_op_select_by_smooth:
		epd->SelectBySmoothingGroup (mpMod);
		break;
	}

	return true;
}

void EditPolyMod::EpModPopupDialog (int opcode) {
	// Special cases for detach dialog, create Shape dialog.
	switch (opcode) {
	case ep_op_detach_vertex:
	case ep_op_detach_face:
		if (!GetDetachObject()) return;
		EpModButtonOp (opcode);
		break;
	case ep_op_create_shape:
		if (!GetCreateShape()) return;
		EpModButtonOp (opcode);
		break;
	case ep_op_attach:
		if (ip) {
			EditPolyAttachHitByName proc(this, ip);
			ip->DoHitByNameDialog(&proc);
		}
		break;
	default:
		EpModSetOperation (opcode);
		EpModShowOperationDialog();
		break;
	}
}

void EditPolyMod::SetOperation (int opcode, bool autoCommit, bool macroRecord) {
	int currentOp = GetPolyOperationID();

	// prevent from doing anything else when in a move op lrr 03/05
	if ( m_Transforming && opcode != ep_op_transform )
	{
		return ;
	}

	if (opcode == currentOp)
	{
		if (!autoCommit) return;
		int animateMode = mpParams->GetInt (epm_animation_mode, TimeValue(0));
		if (animateMode) return;

		// Commit and macro-record.
		EpModCommit (ip->GetTime(), true);
		return;
	}

	if (macroRecord && macroRecorder->Enabled())
	{
		TCHAR *opName = LookupOperationName (opcode);
		TCHAR *functionName = autoCommit ? _T("$.modifiers[#Edit_Poly].ButtonOp")
			: _T("$.modifiers[#Edit_Poly].SetOperation");
		if (opName) macroRecorder->FunctionCall (functionName, 1, 0, mr_name, opName);
		else macroRecorder->FunctionCall (functionName, 1, 0, mr_int, opcode);
		macroRecorder->EmitScript();
	}

	// Special cases - these ops are done instantly, not handled as PolyOps.
	bool usePolyOp = false;
	switch (opcode)
	{
	case ep_op_select_ring:
	case ep_op_select_loop:
		if (meshSelLevel[selLevel] != MNM_SL_EDGE) break;
		// Otherwise, fall through...

	case ep_op_sel_grow:
	case ep_op_sel_shrink:
	case ep_op_select_by_smooth:
	case ep_op_select_by_material:
	case ep_op_get_stack_selection:
		if (mpParams->GetInt (epm_stack_selection)) break;	// nothing to do.
		{
		theHold.Begin ();
		EpModAboutToChangeSelection ();
		AlterSelectionEnumProc alter(this, selLevel, opcode);
		EnumModContexts (&alter);
		theHold.Accept (GetString (IDS_SELECT));
		EpModLocalDataChanged (PART_SELECT);
		EpModRefreshScreen ();
		}
		EpSetLastOperation (opcode);
		break;

	case ep_op_hide_vertex:
	case ep_op_hide_face:
	case ep_op_hide_unsel_vertex:
	case ep_op_hide_unsel_face:
	case ep_op_unhide_vertex:
	case ep_op_unhide_face:
		{
		theHold.Begin ();
		EpModAboutToChangeSelection ();
		AlterSelectionEnumProc alter(this, selLevel, opcode);
		EnumModContexts (&alter);
		theHold.Accept (GetString (IDS_HIDE));
		EpModLocalDataChanged (PART_DISPLAY|PART_SELECT);
		EpModRefreshScreen ();
		}
		EpSetLastOperation (opcode);
		break;

	case ep_op_ns_copy:
		NSCopy ();
		break;

	case ep_op_ns_paste:
		if (mpParams->GetInt (epm_stack_selection)) break;	// nothing to do.
		theHold.Begin ();
		NSPaste ();
		theHold.Accept (GetString (IDS_PASTE_NS));
		EpModLocalDataChanged (PART_SELECT);
		EpModRefreshScreen ();
		break;

	case ep_op_toggle_shaded_faces:
		theHold.Begin ();
		ToggleShadedFaces ();
		theHold.Accept (GetString (IDS_SHADED_FACE_TOGGLE));
		EpModRefreshScreen ();
		break;

	case ep_op_reset_plane:
		theHold.Begin ();
		EpResetSlicePlane ();
		theHold.Accept (GetString (IDS_RESET_SLICE_PLANE));
		break;

	case ep_op_create_shape:
		EpModCreateShape (mCreateShapeName, ip->GetTime());
		break;

	case ep_op_preserve_uv_settings:
		PreserveMapsUIHandler::GetInstance()->SetEPoly (this);
		PreserveMapsUIHandler::GetInstance()->DoDialog ();
		break;

	default:
		usePolyOp = true;
		break;
	}

	if (!usePolyOp)
	{
		EpSetLastOperation (opcode);
		return;
	}

	// Otherwise, we're setting to a particular PolyOperation.

	// First, check that the operation is available:
	PolyOperation *pOp = GetPolyOperationByID (opcode);
	if (pOp == NULL) return;
	TSTR name = pOp->Name();

	TimeValue t = ip ? ip->GetTime() : TimeValue (0);

	// Otherwise, we're currently set to some other operation, or null.

	bool localHold = false;
	if (!theHold.Holding())
	{
		localHold = true;
		theHold.Begin ();
	}

	if (InSlicePreview()) ExitSliceMode ();
	else {
		// If we already have a "current" operation, we need to bake it in,
		// then apply the new operation.
		EpModCommit (t, false);	// does nothing if current op is already "null".
	}

	bool mre = macroRecorder->Enabled()!=0;
	if (mre) macroRecorder->Disable();
	mpParams->SetValue (epm_current_operation, t, opcode);
	if (mre) macroRecorder->Enable ();
	EpSetLastOperation (opcode);

	if ( opcode == ep_op_remove_edge ) 
	{
		bool l_removeVertex = GetKeyState(VK_CONTROL)<0;
		pOp = GetPolyOperation();
		PolyOpRemoveEdge *pOpRemove = (PolyOpRemoveEdge *) pOp;
		pOpRemove->SetCtrlState(l_removeVertex);
	}

	if (autoCommit)
	{
		int animationMode = mpParams->GetInt (epm_animation_mode, t);
		pOp = GetPolyOperation ();
		if (pOp && !pOp->CanAnimate()) animationMode = false;
		if (!animationMode) EpModCommit (t, false);
	}

	if ((opcode == ep_op_detach_vertex) || (opcode == ep_op_detach_face)) {
		if (mDetachToObject)
			EpModDetachToObject (mDetachName, t);
	}

	if (localHold) theHold.Accept (name);

	EpModLocalDataChanged (PART_TOPO|PART_GEOM|PART_SELECT|PART_TEXMAP|PART_VERTCOLOR);
	EpModRefreshScreen ();
}

void EditPolyMod::EpModHighlightChanged()
{
	if (ip) //again sanity check.. only happens when modifier panel is up and we are active..
	{
		InvalidateNumberHighlighted ();
	}
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
}

void EditPolyMod::EpModLocalDataChanged (DWORD parts) {
	if (parts & (PART_SELECT|PART_GEOM|PART_TOPO)) InvalidateDistanceCache ();
	if (parts & PART_SELECT)
	{
		if (ip) {
			InvalidateNumberSelected ();
			UpdateNamedSelDropDown ();
		}
	}
	if (parts & (PART_SELECT|PART_TOPO)) {InvalidateSurfaceUI ();InvalidateFaceSmoothUI();}

	if ((parts & PART_SUBSEL_TYPE) && ip) {
		// Notify the LocalModData about this chage.
		ModContextList mcList;
		INodeTab nodes;
		ip->GetModContexts(mcList,nodes);
		for ( int i = 0; i < mcList.Count(); i++ ) {
			EditPolyData *polyData = (EditPolyData*)mcList[i]->localData;
			if ( !polyData ) continue;
			polyData->InvalidateTempData (PART_SUBSEL_TYPE);
		}
	}

	NotifyDependents(FOREVER, parts, REFMSG_CHANGE);
}

void EditPolyMod::EpModRefreshScreen () {
	if (ip) ip->RedrawViews (ip->GetTime ());
}

void EditPolyMod::CloneSelSubComponents(TimeValue t) {
	if (!ip) return;
	switch (meshSelLevel[selLevel])
	{
	case MNM_SL_VERTEX:
		EpModButtonOp (ep_op_clone_vertex);
		break;
	case MNM_SL_EDGE:
		if (!mSuspendCloneEdges)
			EpModButtonOp (ep_op_clone_edge);
		mSuspendCloneEdges = false;
		break;
	case MNM_SL_FACE:
		EpModButtonOp (ep_op_clone_face);
		break;
	}
}

void EditPolyMod::AcceptCloneSelSubComponents(TimeValue t) {
	// If we're in animating mode, don't do anything;
	if (mpParams->GetInt (epm_animation_mode, t)>0) return;

	// Otherwise, we're in modeling mode, so provide detach option.
	switch (selLevel) {
	case EPM_SL_OBJECT:
	case EPM_SL_EDGE:
	case EPM_SL_BORDER:
		return;
	}

	if (!ip) return;

	if (GetDetachClonedObject ()) EpModDetachToObject (mDetachName, t);

	EpModRefreshScreen ();
}

void EditPolyMod::EpModRepeatLast () {
	theHold.Begin ();
	if (GetPolyOperationID () != ep_op_null)
	{
		TimeValue t = ip ? ip->GetTime() : GetCOREInterface()->GetTime();
		EpModCommitAndRepeat (t);
	}
	else
	{
		if (mLastOperation == ep_op_null) {
			theHold.Cancel ();
			return;
		}
		EpModButtonOp (mLastOperation);
	}
	theHold.Accept (GetString (IDS_REPEAT_LAST_OPERATION));
	EpModRefreshScreen ();
}

void EditPolyMod::ToggleShadedFaces () {
	if (!ip) return;

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	for (int i=0; i<nodes.Count(); i++)
	{
		int oldShow, oldShaded, oldType;
		oldShow = nodes[i]->GetCVertMode ();
		oldShaded = nodes[i]->GetShadeCVerts ();
		oldType = nodes[i]->GetVertexColorType ();

		// If we have anything other than our perfect (true, true, nvct_soft_select) combo, set to that combo.
		// Otherwise, turn off shading.
		if (oldShow && oldShaded && (oldType == nvct_soft_select)) {
			if (theHold.Holding ()) theHold.Put (new ToggleShadedRestore(nodes[i], false));
			nodes[i]->SetCVertMode (false);
		} else {
			if (theHold.Holding ()) theHold.Put (new ToggleShadedRestore(nodes[i], true));
			nodes[i]->SetCVertMode (true);
			nodes[i]->SetShadeCVerts (true);
			nodes[i]->SetVertexColorType (nvct_soft_select);
		}
		nodes[i]->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	}

	//macroRecorder->FunctionCall(_T("$.EditablePoly.ToggleShadedFaces"), 0, 0);
	//macroRecorder->EmitScript ();

	// KLUGE: above notify dependents call only seems to set a limited refresh region.  Result looks bad.
	// So we do this too:
	EpModLocalDataChanged (PART_DISPLAY);
}

// We want the optimal macroRecorder output here.  All arguments but "val" are optional.
// 3 separate decisions: non-default time, non-default parent/axis matrices, non-default "local".
void TransformMacroRecord (TCHAR *fname, int valType, void *val, Matrix3 *partm=NULL,
						   Matrix3 *tmAxis=NULL, BOOL localOrigin=false) {

	TCHAR *optionNames[4];
	int optionTypes[4];
	INT_PTR optionValues[4];
	int numOptions = 0;
	TCHAR buf1[200], buf2[200];

	if (partm) {
		partm->ValidateFlags();
		if (!partm->IsIdentity()) {
			optionNames[numOptions] = _T("parent");
			if ((partm->GetIdentFlags() & (ROT_IDENT|SCL_IDENT)) == (ROT_IDENT|SCL_IDENT)) {
				Point3 trans = partm->GetTrans();
				_stprintf (buf1, _T("(transMatrix [%5.4f,%5.4f,%5.4f])"), trans.x, trans.y, trans.z);
				optionTypes[numOptions] = mr_varname;
				optionValues[numOptions] = reinterpret_cast<INT_PTR>(buf1);
			} else {
				optionTypes[numOptions] = mr_matrix3;
				optionValues[numOptions] = reinterpret_cast<INT_PTR>(partm);
			}
			numOptions++;
		}
	}

	if (tmAxis) {
		tmAxis->ValidateFlags();
		if (!tmAxis->IsIdentity()) {
			optionNames[numOptions] = _T("axis");
			if ((tmAxis->GetIdentFlags() & (ROT_IDENT|SCL_IDENT)) == (ROT_IDENT|SCL_IDENT)) {
				Point3 trans = tmAxis->GetTrans();
				_stprintf (buf2, _T("(transMatrix [%5.4f,%5.4f,%5.4f])"), trans.x, trans.y, trans.z);
				optionTypes[numOptions] = mr_varname;
				optionValues[numOptions] = reinterpret_cast<INT_PTR>(buf2);
			} else {
				optionTypes[numOptions] = mr_matrix3;
				optionValues[numOptions] = reinterpret_cast<INT_PTR>(tmAxis);
			}
			numOptions++;
		}
	}

	if (localOrigin) {
		optionNames[numOptions] = _T("localOrigin");
		optionTypes[numOptions] = mr_bool;
      // SR NOTE64: We can put this argument into the INT_PTR array and still read
      // it properly in the function call because arguments pushed/popped are
      // automatically promoted to intptr-sized chunks (for instance, on AMD64,
      // you can't push anything smaller than 8 bytes on the stack).
		optionValues[numOptions] = true;
		numOptions++;
	}

	switch (numOptions) {
		case 0:
			macroRecorder->FunctionCall (fname, 2, 0, mr_varname, _T(""), valType, val);
			break;

		case 1:
			macroRecorder->FunctionCall (fname, 2, 1, mr_varname, _T(""), valType, val,
				optionNames[0], optionTypes[0], optionValues[0]);
			break;

		case 2:
			macroRecorder->FunctionCall (fname, 2, 2, mr_varname, _T(""), valType, val,
				optionNames[0], optionTypes[0], optionValues[0],
				optionNames[1], optionTypes[1], optionValues[1]);
			break;

		case 3:
			macroRecorder->FunctionCall (fname, 2, 3, mr_varname, _T(""), valType, val,
				optionNames[0], optionTypes[0], optionValues[0],
				optionNames[1], optionTypes[1], optionValues[1],
				optionNames[2], optionTypes[2], optionValues[2]);
			break;

		case 4:
			macroRecorder->FunctionCall (fname, 2, 4, mr_varname, _T(""), valType, val,
				optionNames[0], optionTypes[0], optionValues[0],
				optionNames[1], optionTypes[1], optionValues[1],
				optionNames[2], optionTypes[2], optionValues[2],
				optionNames[3], optionTypes[3], optionValues[3]);
			break;
	}
}

void EditPolyMod::Transform (TimeValue t, Matrix3& partm, Matrix3 tmAxis, BOOL localOrigin, Matrix3 xfrm, int type) {
	if (!ip) return;

	// Definitely transforming subobject geometry:
	//DragMoveRestore ();

	// Get modcontexts
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	// Get axis type:
	int numAxis = ip->GetNumAxis();

	// Special case for vertices: Only individual axis when moving in local space
	if ((selLevel==EPM_SL_VERTEX) && (numAxis==NUMAXIS_INDIVIDUAL)) {
		if (ip->GetRefCoordSys()!=COORDS_LOCAL || 
			ip->GetCommandMode()->ID()!=CID_SUBOBJMOVE) {
			numAxis = NUMAXIS_ALL;
		}
	}

	int msl = meshSelLevel[selLevel];

	// Get soft selection parameters:
	int softSel = 0, edgeIts=1, useEdgeDist=FALSE;
	float falloff = 0.0f, pinch = 0.0f, bubble = 0.0f;
	Interval frvr = FOREVER;
	mpParams->GetValue (epm_ss_use, t, softSel, frvr);
	if (softSel) {
		// These are only used if lock is turned off, or if the locked soft selections are missing.
		mpParams->GetValue (epm_ss_edist_use, t, useEdgeDist, frvr);
		if (useEdgeDist) mpParams->GetValue (epm_ss_edist, t, edgeIts, frvr);
		falloff = mpParams->GetFloat (epm_ss_falloff, t);
		pinch = mpParams->GetFloat (epm_ss_pinch, t);
		bubble = mpParams->GetFloat (epm_ss_bubble, t);
	}
	BOOL lockSoftSel = mpParams->GetInt( epm_ss_lock );

	// test first modcontext local data to see if in transform mode. If not, assuming calling from maxscript
	// and EpModDragInit has not been called yet. Call it on all nodes, do the op, and then call EpModDragAccept
	// on all nodes.
	bool doDragAccept = false;
	if (mcList.Count() != 0 && mcList[0]->localData)
	{
		EditPolyData *pData = (EditPolyData*)mcList[0]->localData;
		if (pData && (!pData->GetPolyOpData() || (pData->GetPolyOpData()->OpID() != ep_op_transform)))
		{
			//Maxscript calling transformation - ensure that the transform operation is set so that the points
			//will be updated later when ModifyObject is called (otherwise it won't know what operation to perform)
			EpModSetOperation (ep_op_transform);
			doDragAccept = true;
			for (int nd=0; nd<mcList.Count(); nd++)
				EpModDragInit (nodes[nd]->GetActualINode(), t);
		}
	}

	for (int nd=0; nd<mcList.Count(); nd++) {
		EditPolyData *pData = (EditPolyData*)mcList[nd]->localData;
		if (!pData) continue;
		if (!pData->GetPolyOpData()) continue;
		if (pData->GetPolyOpData()->OpID() != ep_op_transform) continue;
		LocalTransformData *pTransformData = (LocalTransformData *) pData->GetPolyOpData();
		Point3 *pCurrentOffsets = pTransformData->OffsetPointer(0);

		// We can't use the output mesh here, or we're totally unstable.
		// But we can't use the input mesh, either.
		// What we need is the mesh after previous transforms, but before the current one.
		MNMesh *mesh = pData->GetMesh ();
		MNTempData *td = pData->TempData ();
		float *vsw = SoftSelection (t, pData, FOREVER);
		if (!mesh || !td) continue;

		// partm is useless -- need this matrix which includes object-offset:
		Matrix3 objTM = nodes[nd]->GetObjectTM(t);

		// Selected vertices - either directly or indirectly through selected faces or edges.
		BitArray sel = mesh->VertexTempSel();
		if (!sel.NumberSet() && !vsw) continue;

		// Setup some of the affect region stuff
		int i, nv=mesh->numv;

		Tab<Point3> offset;
		offset.SetCount (nv);
		for (i=0; i<nv; i++) offset[i] = Point3(0,0,0);

		BOOL useClusterTransform = FALSE;  // we only need to do the cluster transform for scale and rotation and if there are more than 1 cluster
		CommandMode *commandMode = ip->GetCommandMode();
		if ( (commandMode->ID() == CID_SUBOBJMOVE) ||
			 (commandMode->ID() == CID_SUBOBJROTATE) ||
			 (commandMode->ID() == CID_SUBOBJSCALE) ||
			 (commandMode->ID() == CID_SUBOBJUSCALE) ||
			 (commandMode->ID() == CID_SUBOBJSQUASH) )

		{
			if (selLevel != EPM_SL_VERTEX)
			{
				DWORD count = (msl == MNM_SL_EDGE) ? td->EdgeClusters()->count : td->FaceClusters()->count;
				if (count > 1)
					useClusterTransform = TRUE;
			}
		}
		
		if (!useClusterTransform)	//make sure that if we skip the cluster transform we set the axis to 
									//NUMAXIS_ALL so we dont do local transform which for some reason steve did not want to do for edge and face 
		{
			if (selLevel != EPM_SL_VERTEX) 
				numAxis = NUMAXIS_ALL;
		}


		// Compute the transforms
		if ((numAxis==NUMAXIS_INDIVIDUAL) && (selLevel != EPM_SL_VERTEX) && useClusterTransform) {
			// Do each cluster one at a time

			// If we have soft selections from multiple clusters,
			// we need to add up the vectors and divide by the total soft selection,
			// to get the right direction for movement,
			// but we also need to add up the squares of the soft selections and divide by the total soft selection,
			// essentially getting a weighted sum of the selection weights themselves,
			// to get the right scale of movement.

			// (Note that this works out to ordinary soft selections in the case of a single cluster.)

			DWORD count = (msl == MNM_SL_EDGE) ? td->EdgeClusters()->count : td->FaceClusters()->count;
			Tab<int> *vclust = td->VertexClusters(msl);
			float *clustDist=NULL, *sss=NULL, *ssss=NULL;
			Tab<float> softSelSum, softSelSquareSum;
			Matrix3 tm, itm;
			if (softSel) {
				softSelSum.SetCount(nv);
				sss = softSelSum.Addr(0);
				softSelSquareSum.SetCount (nv);
				ssss = softSelSquareSum.Addr(0);
				for (i=0; i<nv; i++) {
					sss[i] = 0.0f;
					ssss[i] = 0.0f;
				}
			}
			for (DWORD j=0; j<count; j++) {
				tmAxis = ip->GetTransformAxis (nodes[nd], j);
				tm  = objTM * Inverse(tmAxis);
				itm = Inverse(tm);
				tm *= xfrm;
				if (softSel) clustDist = td->ClusterDist (msl, MN_SEL, j, useEdgeDist, edgeIts)->Addr(0);
				for (i=0; i<nv; i++) {
					if (sel[i]) {
						if ((*vclust)[i]!=j) continue;
						Point3 old = mesh->v[i].p;
						if (pCurrentOffsets) old += pCurrentOffsets[i];
						offset[i] = (tm*old)*itm - old;
					} else {
						if (!softSel) continue;
						if (clustDist[i] < 0) continue;
						if (clustDist[i] > falloff) continue;
						float af;
						if (lockSoftSel && vsw) af = vsw[i];
						else af = AffectRegionFunction (clustDist[i], falloff, pinch, bubble);
						sss[i] += fabsf(af);
						ssss[i] += af*af;
						Point3 old = mesh->v[i].p;
						if (pCurrentOffsets) old += pCurrentOffsets[i];
						offset[i] += ((tm*old)*itm - old)*af;
					}
				}
			}
			if (softSel && (count>1)) {
				for (i=0; i<nv; i++) {
					if (sel[i]) continue;
					if (sss[i] == 0) continue;
					offset[i] *= (ssss[i] / (sss[i]*sss[i]));
				}
			}
		} else {
			Matrix3 tm  = objTM * Inverse(tmAxis);
			Matrix3 itm = Inverse(tm);
			tm *= xfrm;
			Matrix3 ntm;
			for (i=0; i<nv; i++) {
				if (!sel[i] && (!vsw || !vsw[i])) continue;
				Point3 old = mesh->v[i].p;
				if (pCurrentOffsets) old += pCurrentOffsets[i];
				Point3 delta;
				if (numAxis == NUMAXIS_INDIVIDUAL) {
					tmAxis = ip->GetTransformAxis (nodes[nd], i);
					tm  = objTM * Inverse(tmAxis);
					itm = Inverse(tm);
					delta = itm*(xfrm*(tm*old)) - old;
				} else {
					delta = itm*(tm*old)-old;
				}
				if (sel[i]) offset[i] = delta;
				else offset[i] = delta * vsw[i];
			}
		}

		EpModDragMove (offset, nodes[nd]->GetActualINode(), t);
	}

	if (doDragAccept)
	{
		for (int nd=0; nd<mcList.Count(); nd++)
			EpModDragAccept (nodes[nd]->GetActualINode(), t);
	}
	nodes.DisposeTemporary();
}

void EditPolyMod::EpModMoveSelection (Point3 &val, Matrix3 &partm, Matrix3 &tmAxis, BOOL localOrigin, TimeValue t) {
	Transform (t, partm, tmAxis, false, TransMatrix(val), 0);	
	TransformMacroRecord (_T("$.modifiers[#Edit_Poly].MoveSelection"), mr_point3, &val,
		&partm, &tmAxis, localOrigin);
}

void EditPolyMod::EpModMoveSlicePlane (Point3 &val, Matrix3 &partm, Matrix3 &tmAxis, TimeValue t) {
	SetXFormPacket pckt (val, partm, tmAxis);
	mpSliceControl->SetValue (t, &pckt, TRUE, CTRL_RELATIVE);

	TransformMacroRecord (_T("$.modifiers[#Edit_Poly].MoveSlicePlane"), mr_point3, &val,
		&partm, &tmAxis, false);
}

void EditPolyMod::Move (TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin) {
	if (mSliceMode)
		EpModMoveSlicePlane (val, partm, tmAxis, t);
	else if (GetPolyOperation() && (GetPolyOperation()->OpID() == ep_op_transform))
		EpModMoveSelection (val, partm, tmAxis, localOrigin, t);
}

void EditPolyMod::EpModRotateSelection (Quat &val, Matrix3 &partm, Matrix3 &tmAxis, BOOL localOrigin, TimeValue t) {
	Matrix3 mat;
	val.MakeMatrix(mat);
	Transform(t, partm, tmAxis, localOrigin, mat, 1);
	TransformMacroRecord (_T("$.modifiers[#Edit_Poly].RotateSelection"), mr_quat, &val,
		&partm, &tmAxis, localOrigin);
}

void EditPolyMod::EpModRotateSlicePlane (Quat &val, Matrix3 &partm, Matrix3 &tmAxis, BOOL localOrigin, TimeValue t) {
	SetXFormPacket pckt (val, localOrigin, partm, tmAxis);
	mpSliceControl->SetValue (t, &pckt, TRUE, CTRL_RELATIVE);

	TransformMacroRecord (_T("$.modifiers[#Edit_Poly].RotateSlicePlane"), mr_quat, &val,
		&partm, &tmAxis, localOrigin);
}

void EditPolyMod::Rotate (TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin) {
	if (mSliceMode)
		EpModRotateSlicePlane (val, partm, tmAxis, localOrigin, t);
	else if (GetPolyOperation() && (GetPolyOperation()->OpID() == ep_op_transform))
		EpModRotateSelection (val, partm, tmAxis, localOrigin, t);
}

void EditPolyMod::EpModScaleSelection (Point3 &val, Matrix3 &partm, Matrix3 &tmAxis, BOOL localOrigin, TimeValue t) {
	Transform (t, partm, tmAxis, localOrigin, ScaleMatrix(val), 2);	
	TransformMacroRecord (_T("$.modifiers[#Edit_Poly].ScaleSelection"), mr_point3, &val,
		&partm, &tmAxis, localOrigin);
}

void EditPolyMod::EpModScaleSlicePlane (Point3 &val, Matrix3 &partm, Matrix3 &tmAxis, BOOL localOrigin, TimeValue t) {
	SetXFormPacket pckt (val, localOrigin, partm, tmAxis);
	mpSliceControl->SetValue (t, &pckt, TRUE, CTRL_RELATIVE);

	TransformMacroRecord (_T("$.modifiers[#Edit_Poly].ScaleSlicePlane"), mr_point3, &val,
		&partm, &tmAxis, localOrigin);
}

void EditPolyMod::Scale (TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Point3& val, BOOL localOrigin) {
	if (mSliceMode) EpModScaleSlicePlane (val, partm, tmAxis, localOrigin, t);
	else if (GetPolyOperation() && (GetPolyOperation()->OpID() == ep_op_transform))
		EpModScaleSelection (val, partm, tmAxis, localOrigin, t);
}

void EditPolyMod::TransformStart (TimeValue t)
{
	if (!ip) return;

	ip->LockAxisTripods(TRUE);

	if (mSliceMode) return;

	theHold.Begin ();

	EpModSetOperation (ep_op_transform);

	ModContextList list;
	INodeTab nodes;
	ip->GetModContexts (list, nodes);

	for (int i=0; i<list.Count(); i++) {
		EpModDragInit (nodes[i]->GetActualINode(), t);
	}
	nodes.DisposeTemporary();
	m_Transforming = true;			

	theHold.Accept (GetString (IDS_START_TRANSFORM));
}

void EditPolyMod::TransformHoldingFinish (TimeValue t)
{
	if (mSliceMode) return;

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	for (int i=0; i<mcList.Count(); i++) {
		EpModDragAccept (nodes[i]->GetActualINode(), t);
	}
	nodes.DisposeTemporary ();

	EpModCommitUnlessAnimating (t);
}

void EditPolyMod::TransformFinish(TimeValue t) {
	if (!ip) return;
	ip->LockAxisTripods(FALSE);

	m_Transforming = false;			

}

void EditPolyMod::TransformCancel (TimeValue t) {
	if (!ip) return;
	ip->LockAxisTripods(FALSE);

	if (mSliceMode) return;
	ClearDisplayResult ();

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	for (int i=0; i<mcList.Count(); i++) {
		EpModDragCancel (nodes[i]->GetActualINode());
	}
	nodes.DisposeTemporary ();

	m_Transforming = false;			

}

void EditPolyMod::EpResetSlicePlane () {
	ReplaceReference (EDIT_SLICEPLANE_REF, NewDefaultMatrix3Controller ());
	Matrix3 tm(1);
	mpSliceControl->GetValue (0, &tm, FOREVER, CTRL_RELATIVE);

	// Indicate to the local data that the slice plane size needs to be reset.
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts (mcList, nodes);
	for (int i=0; i<mcList.Count(); i++) {
		EditPolyData *pData = (EditPolyData *) mcList[i]->localData;
		if (pData == NULL) continue;
		pData->SetSlicePlaneSize (-1.0f);
	}
	nodes.DisposeTemporary();

	// No macroRecording, because this is typically recorded as "ButtonOp #ResetSlicePlane".
	//macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].ResetSlicePlane"), 0, 0);
	//macroRecorder->EmitScript ();

	EpModLocalDataChanged (PART_DISPLAY);
	EpModRefreshScreen ();
}

Matrix3 EditPolyMod::EpGetSlicePlaneTM (TimeValue t) {
	Matrix3 tm(1);
	mpSliceControl->GetValue (t, &tm, FOREVER, CTRL_RELATIVE);
	return tm;
}

void EditPolyMod::EpGetSlicePlane (Point3 & planeNormal, Point3 & planeCenter, TimeValue t) {
	Matrix3 tm = EpGetSlicePlaneTM (t);
	planeNormal = tm.GetRow(2);
	planeCenter = tm.GetTrans();
}

void EditPolyMod::EpSetSlicePlane (Point3 & planeNormal, Point3 & planeCenter, TimeValue t) {
	Matrix3 currentTM = EpGetSlicePlaneTM (t);

	// Get the rotation which brings the plane normal to where we need it:
	planeNormal.FNormalize();
	Point3 currentNormal = currentTM.GetRow(2);
	currentNormal.FNormalize ();
	Point3 axis = planeNormal ^ currentNormal;
	float axLen = axis.FLength();
	float dp = DotProd (currentNormal, planeNormal);
	float angle = atan2f (axLen, dp);

	if (axLen==0)
	{
		// Current and desired normal point in the same (or opposite) direction.
		// Choose a better axis - anything will do, so long as it's orthogonal to these guys.
		axis = Point3(1,0,0);
		axis -= DotProd(axis, currentNormal) * currentNormal;
		axLen = axis.FLength();
		if (axLen<1.0e-04f) {
			// we can do better than that.
			axis = Point3(0,1,0);
			axis -= DotProd(axis, currentNormal) * currentNormal;
			axLen = axis.FLength();
		}
	}

	axis /= axLen;
	
	AngAxis aa(axis, angle);
	Quat q;
	q.Set (aa);

	SetXFormPacket rotatePacket (q, false);
	mpSliceControl->SetValue (t, &rotatePacket, TRUE, CTRL_RELATIVE);

	Matrix3 currentTM2 = EpGetSlicePlaneTM (t);
	Point3 translation = planeCenter - currentTM2.GetTrans();
	SetXFormPacket movePacket (translation);
	mpSliceControl->SetValue (t, &movePacket, TRUE, CTRL_RELATIVE);

	macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].SetSlicePlane"), 3, 0,
		mr_varname, _T(""), // Dummy first argument to prevent iterated output.
		mr_point3, &planeNormal, mr_point3, &planeCenter);
}


