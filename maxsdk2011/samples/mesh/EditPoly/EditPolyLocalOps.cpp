/**********************************************************************
 *<
	FILE: EditPolyData.cpp

	DESCRIPTION: LocalModData methods for Edit Poly Modifier

	CREATED BY: Steve Anderson

	HISTORY: created March 2004

 *>	Copyright (c) 2004 Discreet, All Rights Reserved.
 **********************************************************************/

#include "epoly.h"
#include "EditPoly.h"
#include "spline3d.h"
#include "splshape.h"
#include "shape.h"

class CreateVertexRestore : public RestoreObj
{
private:
	Point3 mPoint;
	EditPolyData *mpData;
	EditPolyMod *mpMod;
public:
	CreateVertexRestore (EditPolyMod *pMod, EditPolyData *pData, Point3 point)
		: mpMod(pMod), mpData(pData), mPoint(point) { }

	void Restore (int isUndo) {
		if (!mpData->GetPolyOpData()) return;
		if (mpData->GetPolyOpData()->OpID() != ep_op_create) return;
		LocalCreateData *pCreate = (LocalCreateData *) mpData->GetPolyOpData();
		pCreate->RemoveLastVertex ();
		mpMod->EpModLocalDataChanged (PART_DISPLAY);
	}
	void Redo () {
		if (!mpData->GetPolyOpData()) return;
		if (mpData->GetPolyOpData()->OpID() != ep_op_create) return;
		LocalCreateData *pCreate = (LocalCreateData *) mpData->GetPolyOpData();
		pCreate->CreateVertex (mPoint);
		mpMod->EpModLocalDataChanged (PART_DISPLAY);
	}
	int Size () { return 20; }
	TSTR Description() { return TSTR(_T("CreateVertex")); }
};

int EditPolyMod::EpModCreateVertex (Point3 p, INode *pNode)
{
	EditPolyData *pData = GetEditPolyDataForNode (pNode);

	// Make sure we're set to the right operation:
	EpModSetOperation (ep_op_create);
	pData->SetPolyOpData (ep_op_create);

	// Create the restore object:
	if (theHold.Holding()) theHold.Put (new CreateVertexRestore(this, pData, p));

	// Create the vertex:
	LocalCreateData *pCreate = (LocalCreateData *) pData->GetPolyOpData ();
	pCreate->CreateVertex (p);

	EpModLocalDataChanged (PART_DISPLAY);

	// Macro recording:
	if (macroRecorder->Enabled())
	{
		ModContextList list;
		INodeTab nodes;
		ip->GetModContexts (list, nodes);
		if ((list.Count()>1) && (pNode == NULL))
		{
			for (int i=0; i<list.Count(); i++)
			{
				EditPolyData *pData = (EditPolyData *)list[i]->localData;
				if (pData == NULL) continue;
				if (pData->GetFlag (kEPDataPrimary))
				{
					pNode = nodes[i]->GetActualINode();
					break;
				}
			}
		}
		nodes.DisposeTemporary ();

		if (pNode != NULL)
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].CreateVertex"), 1, 1,
				mr_point3, &p,
				_T("node"), mr_reftarg, pNode);
		}
		else
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].CreateVertex"), 1, 0,
				mr_point3, &p);
		}
		macroRecorder->EmitScript();
	}

	if (!pData->GetMesh()) return pData->mVertSel.GetSize() + pCreate->NumVertices() - 1;
	return pData->GetMesh()->numv + pCreate->NumVertices() - 1;
}

class CreateFaceRestore : public RestoreObj
{
private:
	int mDegree;
	int *mVertex;
	EditPolyData *mpData;
	EditPolyMod *mpMod;
public:
	CreateFaceRestore (EditPolyMod *pMod, EditPolyData *pData, int degree, int *vertices)
		: mpMod(pMod), mpData(pData), mDegree(degree) {
		mVertex = new int[degree];
		memcpy (mVertex, vertices, degree*sizeof(int));
	}
	~CreateFaceRestore () { if (mVertex) delete [] mVertex; }

	void Restore (int isUndo) {
		if (!mpData->GetPolyOpData()) return;
		if (mpData->GetPolyOpData()->OpID() != ep_op_create) return;
		LocalCreateData *pCreate = (LocalCreateData *) mpData->GetPolyOpData();
		pCreate->RemoveLastFace ();
		mpMod->EpModLocalDataChanged (PART_DISPLAY);
	}
	void Redo () {
		if (!mpData->GetPolyOpData()) return;
		if (mpData->GetPolyOpData()->OpID() != ep_op_create) return;
		LocalCreateData *pCreate = (LocalCreateData *) mpData->GetPolyOpData();
		pCreate->CreateFace (mVertex, mDegree);
		mpMod->EpModLocalDataChanged (PART_DISPLAY);
	}
	int Size () { return 16+4*mDegree; }
	TSTR Description() { return TSTR(_T("CreateFace")); }
};

int EditPolyMod::EpModCreateFace (Tab<int> *vertex, INode *pNode)
{
	if ((vertex==NULL) || (vertex->Count()<3)) return -1;

	int deg = vertex->Count();
	int *v = vertex->Addr(0);

	EditPolyData *pData = GetEditPolyDataForNode (pNode);

	// Make sure we're set to the right operation:
	EpModSetOperation (ep_op_create);
	pData->SetPolyOpData (ep_op_create);

	// Create the face:
	LocalCreateData *pCreate = (LocalCreateData *) pData->GetPolyOpData ();
	if (!pCreate->CreateFace (v, deg)) return -1;

	// Create the restore object:
	if (theHold.Holding()) theHold.Put (new CreateFaceRestore(this, pData, deg, v));

	EpModLocalDataChanged (PART_DISPLAY);

	// Macro recording:
	if (macroRecorder->Enabled())
	{
		ModContextList list;
		INodeTab nodes;
		ip->GetModContexts (list, nodes);
		if ((list.Count()>1) && (pNode == NULL))
		{
			for (int i=0; i<list.Count(); i++)
			{
				EditPolyData *pData = (EditPolyData *)list[i]->localData;
				if (pData == NULL) continue;
				if (pData->GetFlag (kEPDataPrimary))
				{
					pNode = nodes[i]->GetActualINode();
					break;
				}
			}
		}
		nodes.DisposeTemporary ();

		// since we don't have macrorecorder support for arrays,
		// we need to throw together a string and pass it as an mr_name.
		char vertexList[500];
		char vertexID[40];
		int stringPos=0;
		vertexList[stringPos++] = '(';
		for (int i=0; i<deg; i++) {
			sprintf (vertexID, "%d", v[i]+1);
			memcpy (vertexList + stringPos, vertexID, strlen(vertexID));
			stringPos += static_cast<int>(strlen(vertexID));	// SR DCAST64: Downcast to 2G limit.
			if (i+1<deg) vertexList[stringPos++] = ',';
		}
		vertexList[stringPos++] = ')';
		vertexList[stringPos++] = '\0';

		if (pNode != NULL)
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].CreateFace"), 1, 1,
				mr_name, _T(vertexList),
				_T("node"), mr_reftarg, pNode);
		}
		else
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].CreateFace"), 1, 0,
				mr_name, _T(vertexList));
		}
		macroRecorder->EmitScript();
	}

	if (!pData->GetMesh()) return pData->mFaceSel.GetSize() + pCreate->NumFaces() - 1;
	return pData->GetMesh()->numf + pCreate->NumFaces() - 1;
}

class CreateEdgeRestore : public RestoreObj
{
private:
	int mv1, mv2;
	EditPolyData *mpData;
	EditPolyMod *mpMod;
public:
	CreateEdgeRestore (EditPolyMod *pMod, EditPolyData *pData, int v1, int v2) :
	  mpMod(pMod), mpData(pData), mv1(v1), mv2(v2) { }

	void Restore (int isUndo) {
		if (!mpData->GetPolyOpData()) return;
		if (mpData->GetPolyOpData()->OpID() != ep_op_create_edge) return;
		LocalCreateEdgeData *pCreateEdge = (LocalCreateEdgeData *) mpData->GetPolyOpData();
		pCreateEdge->RemoveLastEdge ();
		mpMod->EpModLocalDataChanged (PART_DISPLAY);
	}
	void Redo () {
		if (!mpData->GetPolyOpData()) return;
		if (mpData->GetPolyOpData()->OpID() != ep_op_create_edge) return;
		LocalCreateEdgeData *pCreateEdge = (LocalCreateEdgeData *) mpData->GetPolyOpData();
		pCreateEdge->AddEdge (mv1, mv2);
		mpMod->EpModLocalDataChanged (PART_DISPLAY);
	}
	int Size () { return 16; }
	TSTR Description() { return TSTR(_T("CreateEdge")); }
};

int EditPolyMod::EpModCreateEdge (int v1, int v2, INode *pNode)
{
	EditPolyData *pData = GetEditPolyDataForNode (pNode);

	// Make sure we're set to the right operation:
	EpModSetOperation (ep_op_create_edge);
	pData->SetPolyOpData (ep_op_create_edge);

	if (theHold.Holding()) theHold.Put (new CreateEdgeRestore (this, pData, v1, v2));

	LocalCreateEdgeData *pCreateEdge = (LocalCreateEdgeData *) pData->GetPolyOpData();
	pCreateEdge->AddEdge (v1, v2);

	EpModLocalDataChanged (PART_DISPLAY);

	// Macro recording:
	if (macroRecorder->Enabled())
	{
		ModContextList list;
		INodeTab nodes;
		ip->GetModContexts (list, nodes);
		if ((list.Count()>1) && (pNode == NULL))
		{
			for (int i=0; i<list.Count(); i++)
			{
				EditPolyData *pData = (EditPolyData *)list[i]->localData;
				if (pData == NULL) continue;
				if (pData->GetFlag (kEPDataPrimary))
				{
					pNode = nodes[i]->GetActualINode();
					break;
				}
			}
		}
		nodes.DisposeTemporary ();

		if (pNode != NULL)
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].CreateEdge"), 2, 1,
				mr_int, v1+1, mr_int, v2+1,
				_T("node"), mr_reftarg, pNode);
		}
		else
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].CreateEdge"), 2, 0,
				mr_int, v1+1, mr_int, v2+1);
		}
		macroRecorder->EmitScript();
	}

	if (!pData->GetMesh()) return pData->mEdgeSel.GetSize() + pCreateEdge->NumEdges() - 1;
	return pData->GetMesh()->nume + pCreateEdge->NumEdges()-1;
}

class SetDiagonalRestore : public RestoreObj
{
private:
	int mv1, mv2;
	EditPolyData *mpData;
	EditPolyMod *mpMod;
public:
	SetDiagonalRestore (EditPolyMod *pMod, EditPolyData *pData, int v1, int v2) : mpMod(pMod), mpData(pData), mv1(v1), mv2(v2) { }

	void Restore (int isUndo) {
		if (!mpData->GetPolyOpData()) return;
		if (mpData->GetPolyOpData()->OpID() != ep_op_edit_triangulation) return;
		LocalSetDiagonalData *pSetDiag = (LocalSetDiagonalData *) mpData->GetPolyOpData();
		pSetDiag->RemoveLastDiagonal ();
		mpMod->EpModLocalDataChanged (PART_DISPLAY);
	}
	void Redo () {
		if (!mpData->GetPolyOpData()) return;
		if (mpData->GetPolyOpData()->OpID() != ep_op_edit_triangulation) return;
		LocalSetDiagonalData *pSetDiag = (LocalSetDiagonalData *) mpData->GetPolyOpData();
		pSetDiag->AddDiagonal (mv1, mv2);
		mpMod->EpModLocalDataChanged (PART_DISPLAY);
	}
	int Size () { return 16; }
	TSTR Description() { return TSTR(_T("SetDiagonal")); }
};

void EditPolyMod::EpModSetDiagonal (int v1, int v2, INode *pNode)
{
	EditPolyData *pData = GetEditPolyDataForNode (pNode);

	// Make sure we're set to the right operation:
	EpModSetOperation (ep_op_edit_triangulation);
	pData->SetPolyOpData (ep_op_edit_triangulation);

	if (theHold.Holding()) theHold.Put (new SetDiagonalRestore (this, pData, v1, v2));

	LocalSetDiagonalData *pSetDiag = (LocalSetDiagonalData *) pData->GetPolyOpData();
	pSetDiag->AddDiagonal (v1, v2);

	EpModLocalDataChanged (PART_DISPLAY);

	// Macro recording:
	if (macroRecorder->Enabled())
	{
		ModContextList list;
		INodeTab nodes;
		ip->GetModContexts (list, nodes);
		if ((list.Count()>1) && (pNode == NULL))
		{
			for (int i=0; i<list.Count(); i++)
			{
				EditPolyData *pData = (EditPolyData *)list[i]->localData;
				if (pData == NULL) continue;
				if (pData->GetFlag (kEPDataPrimary))
				{
					pNode = nodes[i]->GetActualINode();
					break;
				}
			}
		}
		nodes.DisposeTemporary ();

		if (pNode != NULL)
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].SetDiagonal"), 2, 1,
				mr_int, v1+1, mr_int, v2+1,
				_T("node"), mr_reftarg, pNode);
		}
		else
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].SetDiagonal"), 2, 0,
				mr_int, v1+1, mr_int, v2+1);
		}
		macroRecorder->EmitScript();
	}
}

class CutRestore : public RestoreObj
{
private:
	int mStartLevel, mStartIndex;
	Point3 mStart, mEnd, mNormal;
	EditPolyData *mpData;
	EditPolyMod *mpMod;
public:
	CutRestore (EditPolyMod *pMod, EditPolyData *pData, int startLevel, int startIndex, Point3 startPoint, Point3 normal) :
	  mpMod(pMod), mpData(pData), mStartLevel(startLevel), mStartIndex(startIndex), mStart(startPoint), mNormal(normal) { }

	void Restore (int isUndo) {
		if (!mpData->GetPolyOpData()) return;
		if (mpData->GetPolyOpData()->OpID() != ep_op_cut) return;
		LocalCutData *pCut = (LocalCutData *) mpData->GetPolyOpData();
		if (pCut->NumCuts()==0) return;
		mEnd = pCut->End (pCut->NumCuts()-1);
		pCut->RemoveLast ();
		mpMod->EpModLocalDataChanged (PART_DISPLAY);
	}
	void Redo () {
		if (!mpData->GetPolyOpData()) return;
		if (mpData->GetPolyOpData()->OpID() != ep_op_cut) return;
		LocalCutData *pCut = (LocalCutData *) mpData->GetPolyOpData();
		pCut->AddCut (mStartLevel, mStartIndex, mStart, mNormal);
		pCut->SetEndPoint (mEnd);
		mpMod->EpModLocalDataChanged (PART_DISPLAY);
	}
	int Size () { return 52; }
	TSTR Description() { return TSTR(_T("Cut")); }
};

void EditPolyMod::EpModCut (int startLevel, int startIndex, Point3 startPoint, Point3 normal, INode *pNode)
{
	EditPolyData *pData = GetEditPolyDataForNode (pNode);

	// Make sure we're set to the right operation:
	EpModSetOperation (ep_op_cut);
	pData->SetPolyOpData (ep_op_cut);

	if (theHold.Holding()) theHold.Put
		(new CutRestore (this, pData, startLevel, startIndex, startPoint, normal));

	LocalCutData *pCut = (LocalCutData *) pData->GetPolyOpData();
	pCut->AddCut (startLevel, startIndex, startPoint, normal);

	EpModLocalDataChanged (PART_DISPLAY);

	// Macro recording:
	if (macroRecorder->Enabled())
	{
		ModContextList list;
		INodeTab nodes;
		ip->GetModContexts (list, nodes);
		if ((list.Count()>1) && (pNode == NULL))
		{
			for (int i=0; i<list.Count(); i++)
			{
				EditPolyData *pData = (EditPolyData *)list[i]->localData;
				if (pData == NULL) continue;
				if (pData->GetFlag (kEPDataPrimary))
				{
					pNode = nodes[i]->GetActualINode();
					break;
				}
			}
		}
		nodes.DisposeTemporary ();

		if (pNode != NULL)
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].StartCut"), 4, 1,
				mr_name, LookupMNMeshSelLevel(startLevel), mr_int, startIndex+1,
				mr_point3, &startPoint, mr_point3, &normal,
				_T("node"), mr_reftarg, pNode);
		}
		else
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].StartCut"), 4, 0,
				mr_name, LookupMNMeshSelLevel(startLevel), mr_int, startIndex+1,
				mr_point3, &startPoint, mr_point3, &normal);
		}
		macroRecorder->EmitScript();
	}
}

void EditPolyMod::EpModSetCutEnd (Point3 endPoint, INode *pNode)
{
	EditPolyData *pData = GetEditPolyDataForNode (pNode);

	// Make sure we're set to the right operation:
	if (GetPolyOperationID() != ep_op_cut) return;
	if (!pData->GetPolyOpData() || (pData->GetPolyOpData()->OpID() != ep_op_cut)) return;

	LocalCutData *pCut = (LocalCutData *) pData->GetPolyOpData();
	if (pCut->NumCuts()<1) return;
	pCut->SetEndPoint (endPoint);
	pData->SetFlag (kEPDataLastCut);

	EpModLocalDataChanged (PART_DISPLAY);

	// Macro recording:
	if (macroRecorder->Enabled())
	{
		ModContextList list;
		INodeTab nodes;
		ip->GetModContexts (list, nodes);
		if ((list.Count()>1) && (pNode == NULL))
		{
			for (int i=0; i<list.Count(); i++)
			{
				EditPolyData *pData = (EditPolyData *)list[i]->localData;
				if (pData == NULL) continue;
				if (pData->GetFlag (kEPDataPrimary))
				{
					pNode = nodes[i]->GetActualINode();
					break;
				}
			}
		}
		nodes.DisposeTemporary ();

		if (pNode != NULL)
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].EndCut"), 1, 1,
				mr_point3, &endPoint, _T("node"), mr_reftarg, pNode);
		}
		else
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].EndCut"), 1, 0,
				mr_point3, &endPoint);
		}
		macroRecorder->EmitScript();
	}
}

class DivideEdgeRestore : public RestoreObj
{
private:
	int mEdge;
	float mProp;
	EditPolyData *mpData;
	EditPolyMod *mpMod;
public:
	DivideEdgeRestore (EditPolyMod *pMod, EditPolyData *pData, int edge, float prop) :
	  mpMod(pMod), mpData(pData), mEdge(edge), mProp(prop) { }

	void Restore (int isUndo) {
		if (!mpData->GetPolyOpData()) return;
		if (mpData->GetPolyOpData()->OpID() != ep_op_insert_vertex_edge) return;
		LocalInsertVertexEdgeData *pInsert = (LocalInsertVertexEdgeData *) mpData->GetPolyOpData();
		pInsert->RemoveLast ();
		mpMod->EpModLocalDataChanged (PART_DISPLAY);
	}
	void Redo () {
		if (!mpData->GetPolyOpData()) return;
		if (mpData->GetPolyOpData()->OpID() != ep_op_insert_vertex_edge) return;
		LocalInsertVertexEdgeData *pInsert = (LocalInsertVertexEdgeData *) mpData->GetPolyOpData();
		pInsert->InsertVertex (mEdge, mProp);
		mpMod->EpModLocalDataChanged (PART_DISPLAY);
	}
	int Size () { return 16; }
	TSTR Description() { return TSTR(_T("DivideEdge")); }
};

void EditPolyMod::EpModDivideEdge (int edge, float prop, INode *pNode)
{
	// Don't cooperate if we're not in edge level.
	if (meshSelLevel[selLevel] != MNM_SL_EDGE) return;

	EditPolyData *pData = GetEditPolyDataForNode (pNode);

	// Make sure we're set to the right operation:
	EpModSetOperation (ep_op_insert_vertex_edge);
	pData->SetPolyOpData (ep_op_insert_vertex_edge);

	if (theHold.Holding()) theHold.Put (new DivideEdgeRestore (this, pData, edge, prop));

	LocalInsertVertexEdgeData *pInsert = (LocalInsertVertexEdgeData *) pData->GetPolyOpData();
	pInsert->InsertVertex (edge, prop);

	EpModLocalDataChanged (PART_DISPLAY);

	// Macro recording:
	if (macroRecorder->Enabled())
	{
		ModContextList list;
		INodeTab nodes;
		ip->GetModContexts (list, nodes);
		if ((list.Count()>1) && (pNode == NULL))
		{
			for (int i=0; i<list.Count(); i++)
			{
				EditPolyData *pData = (EditPolyData *)list[i]->localData;
				if (pData == NULL) continue;
				if (pData->GetFlag (kEPDataPrimary))
				{
					pNode = nodes[i]->GetActualINode();
					break;
				}
			}
		}
		nodes.DisposeTemporary ();

		if (pNode != NULL)
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].DivideEdge"), 2, 1,
				mr_int, edge+1, mr_float, prop, _T("node"), mr_reftarg, pNode);
		}
		else
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].DivideEdge"), 2, 0,
				mr_int, edge+1, mr_float, prop);
		}
		macroRecorder->EmitScript();
	}
}

class DivideFaceRestore : public RestoreObj
{
private:
	int mFace;
	Tab<float> mBary;
	EditPolyData *mpData;
	EditPolyMod *mpMod;

public:
	DivideFaceRestore (EditPolyMod *pMod, EditPolyData *pData, int face, Tab<float> *bary) :
	  mpMod(pMod), mpData(pData), mFace(face), mBary(*bary) { }

	void Restore (int isUndo) {
		if (!mpData->GetPolyOpData()) return;
		if (mpData->GetPolyOpData()->OpID() != ep_op_insert_vertex_face) return;
		LocalInsertVertexFaceData *pInsert = (LocalInsertVertexFaceData *) mpData->GetPolyOpData();
		pInsert->RemoveLast ();
		mpMod->EpModLocalDataChanged (PART_DISPLAY);
	}
	void Redo () {
		if (!mpData->GetPolyOpData()) return;
		if (mpData->GetPolyOpData()->OpID() != ep_op_insert_vertex_face) return;
		LocalInsertVertexFaceData *pInsert = (LocalInsertVertexFaceData *) mpData->GetPolyOpData();
		pInsert->InsertVertex (mFace, mBary);
		mpMod->EpModLocalDataChanged (PART_DISPLAY);
	}
	int Size () { return 24; }
	TSTR Description() { return TSTR(_T("DivideFace")); }
};

void EditPolyMod::EpModDivideFace (int face, Tab<float> *bary, INode *pNode)
{
	// Don't cooperate if we're not in face level.
	if (meshSelLevel[selLevel] != MNM_SL_FACE) return;

	EditPolyData *pData = GetEditPolyDataForNode (pNode);

	// Make sure we're set to the right operation:
	EpModSetOperation (ep_op_insert_vertex_face);
	pData->SetPolyOpData (ep_op_insert_vertex_face);

	if (theHold.Holding()) theHold.Put (new DivideFaceRestore (this, pData, face, bary));

	LocalInsertVertexFaceData *pInsert = (LocalInsertVertexFaceData *) pData->GetPolyOpData();
	pInsert->InsertVertex (face, *bary);

	EpModLocalDataChanged (PART_DISPLAY);

	// Macro recording:
	if (macroRecorder->Enabled())
	{
		ModContextList list;
		INodeTab nodes;
		ip->GetModContexts (list, nodes);
		if ((list.Count()>1) && (pNode == NULL))
		{
			for (int i=0; i<list.Count(); i++)
			{
				EditPolyData *pData = (EditPolyData *)list[i]->localData;
				if (pData == NULL) continue;
				if (pData->GetFlag (kEPDataPrimary))
				{
					pNode = nodes[i]->GetActualINode();
					break;
				}
			}
		}
		nodes.DisposeTemporary ();

		// since we don't have macrorecorder support for arrays,
		// we need to throw together a string and pass it as an mr_name.
		TSTR baryCoordString;
		TSTR baryValue;
		int stringPos=0;
		baryCoordString.Append(_T("("));
		for (int i=0; i<bary->Count(); i++) {
			baryValue.printf("%f", (*bary)[i]);
			baryCoordString.Append(baryValue);
			if (i+1<bary->Count()) baryCoordString.Append(_T(","));
		}
		baryCoordString.Append(_T(")"));

		if (pNode != NULL)
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].DivideFace"), 2, 1,
				mr_int, face+1, mr_name, _T(baryCoordString), _T("node"), mr_reftarg, pNode);
		}
		else
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].DivideFace"), 2, 0,
				mr_int, face+1, mr_name, _T(baryCoordString));
		}
		macroRecorder->EmitScript();
	}
}

class TargetWeldRestore : public RestoreObj
{
private:
	int mv1, mv2;
	EditPolyData *mpData;
	EditPolyMod *mpMod;
public:
	TargetWeldRestore (EditPolyMod *pMod, EditPolyData *pData, int v1, int v2) : mpMod(pMod), mpData(pData), mv1(v1), mv2(v2) { }

	void Restore (int isUndo) {
		if (!mpData->GetPolyOpData()) return;
		if ((mpData->GetPolyOpData()->OpID() != ep_op_target_weld_vertex) &&
			(mpData->GetPolyOpData()->OpID() != ep_op_target_weld_edge)) return;
		LocalTargetWeldData *pWeld = (LocalTargetWeldData *) mpData->GetPolyOpData();
		pWeld->RemoveWeld ();
		mpMod->EpModLocalDataChanged (PART_DISPLAY);
	}
	void Redo () {
		if (!mpData->GetPolyOpData()) return;
		if ((mpData->GetPolyOpData()->OpID() != ep_op_target_weld_vertex) &&
			(mpData->GetPolyOpData()->OpID() != ep_op_target_weld_edge)) return;
		LocalTargetWeldData *pWeld = (LocalTargetWeldData *) mpData->GetPolyOpData();
		pWeld->AddWeld (mv1, mv2);
		mpMod->EpModLocalDataChanged (PART_DISPLAY);
	}
	int Size () { return 16; }
	TSTR Description() { return TSTR(_T("TargetWeld")); }
};

void EditPolyMod::EpModWeldVerts (int v1, int v2, INode *pNode)
{
	EditPolyData *pData = GetEditPolyDataForNode (pNode);

	// Make sure we're set to the right operation:
	EpModSetOperation (ep_op_target_weld_vertex);
	pData->SetPolyOpData (ep_op_target_weld_vertex);

	if (theHold.Holding()) theHold.Put (new TargetWeldRestore (this, pData, v1, v2));

	LocalTargetWeldData *pWeld = (LocalTargetWeldData *) pData->GetPolyOpData();
	pWeld->AddWeld (v1, v2);

	EpModLocalDataChanged (PART_DISPLAY);

	// Macro recording:
	if (macroRecorder->Enabled())
	{
		ModContextList list;
		INodeTab nodes;
		ip->GetModContexts (list, nodes);
		if ((list.Count()>1) && (pNode == NULL))
		{
			for (int i=0; i<list.Count(); i++)
			{
				EditPolyData *pData = (EditPolyData *)list[i]->localData;
				if (pData == NULL) continue;
				if (pData->GetFlag (kEPDataPrimary))
				{
					pNode = nodes[i]->GetActualINode();
					break;
				}
			}
		}
		nodes.DisposeTemporary ();

		if (pNode != NULL)
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].WeldVertices"), 2, 1,
				mr_int, v1+1, mr_int, v2+1,
				_T("node"), mr_reftarg, pNode);
		}
		else
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].WeldVertices"), 2, 0,
				mr_int, v1+1, mr_int, v2+1);
		}
		macroRecorder->EmitScript();
	}
}

void EditPolyMod::EpModWeldEdges (int e1, int e2, INode *pNode)
{
	EditPolyData *pData = GetEditPolyDataForNode (pNode);

	// Make sure we're set to the right operation:
	EpModSetOperation (ep_op_target_weld_edge);
	pData->SetPolyOpData (ep_op_target_weld_edge);

	if (theHold.Holding()) theHold.Put (new TargetWeldRestore (this, pData, e1, e2));

	LocalTargetWeldData *pWeld = (LocalTargetWeldData *) pData->GetPolyOpData();
	pWeld->AddWeld (e1, e2);

	EpModLocalDataChanged (PART_DISPLAY);

	// Macro recording:
	if (macroRecorder->Enabled())
	{
		ModContextList list;
		INodeTab nodes;
		ip->GetModContexts (list, nodes);
		if ((list.Count()>1) && (pNode == NULL))
		{
			for (int i=0; i<list.Count(); i++)
			{
				EditPolyData *pData = (EditPolyData *)list[i]->localData;
				if (pData == NULL) continue;
				if (pData->GetFlag (kEPDataPrimary))
				{
					pNode = nodes[i]->GetActualINode();
					break;
				}
			}
		}
		nodes.DisposeTemporary ();

		if (pNode != NULL)
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].WeldEdges"), 2, 1,
				mr_int, e1+1, mr_int, e2+1,
				_T("node"), mr_reftarg, pNode);
		}
		else
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].WeldEdges"), 2, 0,
				mr_int, e1+1, mr_int, e2+1);
		}
		macroRecorder->EmitScript();
	}
}

class SetHingeEdgeRestore : public RestoreObj
{
private:
	int mEdgeBefore, mEdgeAfter;
	EditPolyData *mpData;
	EditPolyMod *mpMod;
public:
	SetHingeEdgeRestore (EditPolyMod *pMod, EditPolyData *pData, int edge) : mpMod(pMod), mpData(pData), mEdgeBefore(-1), mEdgeAfter(edge) {
		if (!mpData->GetPolyOpData()) return;
		if (mpData->GetPolyOpData()->OpID() != ep_op_hinge_from_edge) return;
		LocalHingeFromEdgeData *pHinge = (LocalHingeFromEdgeData *) mpData->GetPolyOpData();
		mEdgeBefore = pHinge->GetEdge();
	}

	void Restore (int isUndo) {
		if (!mpData->GetPolyOpData()) return;
		if (mpData->GetPolyOpData()->OpID() != ep_op_hinge_from_edge) return;
		LocalHingeFromEdgeData *pHinge = (LocalHingeFromEdgeData *) mpData->GetPolyOpData();
		pHinge->SetEdge (mEdgeBefore);
	}
	void Redo () {
		if (!mpData->GetPolyOpData()) return;
		if (mpData->GetPolyOpData()->OpID() != ep_op_hinge_from_edge) return;
		LocalHingeFromEdgeData *pHinge = (LocalHingeFromEdgeData *) mpData->GetPolyOpData();
		pHinge->SetEdge (mEdgeAfter);
	}
	int Size () { return 16; }
	TSTR Description() { return TSTR(_T("SetHingeEdge")); }
};

void EditPolyMod::EpModSetHingeEdge (int edge, Matrix3 modContextTM, INode *pNode)
{
	EditPolyData *pData = GetEditPolyDataForNode (pNode);

	// Make sure we're set to the right operation:
	EpModSetOperation (ep_op_hinge_from_edge);
	pData->SetPolyOpData (ep_op_hinge_from_edge);

	if (theHold.Holding()) theHold.Put (new SetHingeEdgeRestore (this, pData, edge));

	LocalHingeFromEdgeData *pHinge = (LocalHingeFromEdgeData *) pData->GetPolyOpData();
	pHinge->SetEdge (edge);

	// Ensure that no other nodes have hinge info here:
	ModContextList list;
	INodeTab nodes;
	ip->GetModContexts (list, nodes);
	for (int i=0; i<list.Count(); i++) {
		EditPolyData *pOtherData = (EditPolyData *) list[i]->localData;
		if (pOtherData == NULL) continue;
		if (pOtherData == pData) continue;

		// <groan> this needs to be undoable...
		if (pOtherData->GetPolyOpData()) pOtherData->RemovePolyOpData ();
	}

	// Store the hinge data in a form any node can use:
	bool mre = macroRecorder->Enabled () != 0;
	MNMesh *pMesh = pData->GetMesh();
	if (pMesh) {
		Point3 base = pMesh->P(pMesh->e[edge].v1);
		Point3 end = pMesh->P(pMesh->e[edge].v2);

		// Put these endpoints in mod-context coords:
		if (!modContextTM.IsIdentity()) {
			base = modContextTM * base;
			end = modContextTM * end;
		}
		Point3 dir = Normalize (end - base);

		// Store them as parameters without macro-recording the storage:
		if (mre) macroRecorder->Disable ();
		mpParams->SetValue (epm_hinge_base, ip->GetTime(), base);
		mpParams->SetValue (epm_hinge_dir, ip->GetTime(), dir);
		if (mre) macroRecorder->Enable ();
	}
	else DbgAssert(false);	// we should have a valid mesh pointer here!

	// Macro recording:
	if (mre)
	{
		if ((list.Count()>1) && (pNode == NULL))
		{
			for (int i=0; i<list.Count(); i++)
			{
				EditPolyData *pData = (EditPolyData *)list[i]->localData;
				if (pData == NULL) continue;
				if (pData->GetFlag (kEPDataPrimary))
				{
					pNode = nodes[i]->GetActualINode();
					break;
				}
			}
		}
		nodes.DisposeTemporary ();

		if (pNode != NULL)
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].SetHingeEdge"), 1, 1,
				mr_int, edge+1, _T("node"), mr_reftarg, pNode);
		}
		else
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].SetHingeEdge"), 1, 0,
				mr_int, edge+1);
		}
		macroRecorder->EmitScript();
	}
}

int EditPolyMod::EpModGetHingeEdge (INode *pNode)
{
	EditPolyData *pData = GetEditPolyDataForNode (pNode);
	if (!pData) return -1;
	LocalPolyOpData *pOpData = pData->GetPolyOpData();
	if (!pOpData || (pOpData->OpID() != ep_op_hinge_from_edge)) return -1;
	LocalHingeFromEdgeData *pHinge = (LocalHingeFromEdgeData *) pOpData;
	return pHinge->GetEdge();
}

void EditPolyMod::EpModSetBridgeNode (INode *pNode) {
	EditPolyData *pData = GetEditPolyDataForNode (pNode);
	if (selLevel == EPM_SL_BORDER) 
		pData->SetPolyOpData (ep_op_bridge_border);
	else if ( selLevel == EPM_SL_FACE) 
		pData->SetPolyOpData (ep_op_bridge_polygon);
	else if( selLevel == EPM_SL_EDGE)
		pData->SetPolyOpData (ep_op_bridge_edge);


	// Macro recording:
	if (macroRecorder->Enabled())
	{
		macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].SetBridgeNode"), 1, 0, mr_reftarg, pNode);
		macroRecorder->EmitScript();
	}
}

INode *EditPolyMod::EpModGetBridgeNode () {
	ModContextList list;
	INodeTab nodes;
	ip->GetModContexts (list, nodes);
	INode *ret = NULL;
	for (int i=0; i<list.Count(); i++) {
		EditPolyData *pData = (EditPolyData *) list[i]->localData;
		if (pData == NULL) 
			continue;
		if (pData->GetPolyOpData() == NULL) 
			continue;
		int opid = pData->GetPolyOpData()->OpID();

		if ((opid == ep_op_bridge_border) || (opid == ep_op_bridge_polygon) ||(opid == ep_op_bridge_edge) ) {
			ret = nodes[i]->GetActualINode();
			break;
		}
	}
	nodes.DisposeTemporary();

	return ret;
}

bool EditPolyMod::EpModReadyToBridgeSelected () 
{
	if (selLevel<EPM_SL_EDGE) return false;
	if (selLevel>EPM_SL_FACE) return false;

	bool l_border	= (selLevel == EPM_SL_BORDER);
	bool l_edge		= (selLevel == EPM_SL_EDGE);
	bool l_face		= (selLevel == EPM_SL_FACE);

	ModContextList list;
	INodeTab nodes;
	ip->GetModContexts (list, nodes);
   int i;
	for (i=0; i<list.Count(); i++) {
		EditPolyData *pData = (EditPolyData *) list[i]->localData;
		if (pData == NULL) continue;
		if (pData->GetMesh() == NULL) continue;
		MNMesh & mesh = *(pData->GetMesh());
		if ( l_border ) 
		{
			BitArray inborder;
         int j;
			for (j=0; j<mesh.nume; j++) {
				if (mesh.e[j].f2>-1) continue;
				if (mesh.e[j].GetFlag (MN_DEAD)) continue;
				if (!mesh.e[j].GetFlag (MN_SEL)) continue;
				if (inborder.GetSize()==0) {
					inborder.SetSize (mesh.nume);
					Tab<int> border;
					MNMeshUtilities mmu(&mesh);
					mmu.GetBorderFromEdge (j, border);
					for (int k=0; k<border.Count(); k++) inborder.Set(border[k]);
				} else {
					if (!inborder[j]) break;
				}
			}
			if (j<mesh.nume) break;
		} 
		else if ( l_face )
		{
			MNFaceClusters fclust (mesh, MN_SEL);
			if (fclust.count>1) break;
		}
		else
		{
			MNEdgeClusters fclust (mesh, MN_SEL);
			if (fclust.count  > 1) break;
		}
	}
	nodes.DisposeTemporary ();
	return (i<list.Count());
}

void EditPolyMod::EpModBridgeBorders (int e1, int e2, INode *pNode)
{
	mpParams->SetValue (epm_bridge_target_1, ip->GetTime(), e1+1);
	mpParams->SetValue (epm_bridge_twist_1, ip->GetTime(), 0);
	mpParams->SetValue (epm_bridge_target_2, ip->GetTime(), e2+1);
	mpParams->SetValue (epm_bridge_twist_2, ip->GetTime(), 0);
	mpParams->SetValue (epm_bridge_selected, ip->GetTime(), false);

	EpModSetOperation (ep_op_bridge_border);
	if (pNode == NULL) pNode = EpModGetPrimaryNode();
	EpModSetBridgeNode (pNode);
	EpModLocalDataChanged (PART_DISPLAY);
}

void EditPolyMod::EpModBridgePolygons (int f1, int f2, INode *pNode)
{
	int twist2(0);

	EditPolyData *pData = GetEditPolyDataForNode (pNode);
	if (pData && pData->GetMesh() && (f1>=0) && (f2>=0)) {
		// This is a good place to automatically figure out twist values.
		MNMeshUtilities mmu (pData->GetMesh());
		twist2 = mmu.FindDefaultBridgeTwist (f1, f2);
	}

	// None of these parameters can be animated, so we don't have to worry about accidentally setting keys:
	mpParams->SetValue (epm_bridge_target_1, ip->GetTime(), f1+1);
	mpParams->SetValue (epm_bridge_twist_1, ip->GetTime(), 0);
	mpParams->SetValue (epm_bridge_target_2, ip->GetTime(), f2+1);
	mpParams->SetValue (epm_bridge_twist_2, ip->GetTime(), twist2);
	mpParams->SetValue (epm_bridge_selected, ip->GetTime(), false);

	EpModSetOperation (ep_op_bridge_polygon);
	if (pNode == NULL) pNode = EpModGetPrimaryNode();
	EpModSetBridgeNode (pNode);
	EpModLocalDataChanged (PART_DISPLAY);
}

void EditPolyMod::EpModBridgeEdges (const int e1, const int e2, INode *pNode)
{
	mpParams->SetValue (epm_bridge_target_1, ip->GetTime(), e1+1);
	mpParams->SetValue (epm_bridge_twist_1, ip->GetTime(), 0);
	mpParams->SetValue (epm_bridge_target_2, ip->GetTime(), e2+1);
	mpParams->SetValue (epm_bridge_twist_2, ip->GetTime(), 0);
	mpParams->SetValue (epm_bridge_selected, ip->GetTime(), false);

	EpModSetOperation (ep_op_bridge_edge);
	if (pNode == NULL) pNode = EpModGetPrimaryNode();
	EpModSetBridgeNode (pNode);
	EpModLocalDataChanged (PART_DISPLAY);

}

void EditPolyMod::EpModUpdateRingEdgeSelection ( int in_val, INode *in_pNode)
{
	int l_spinVal = 0; 

	bool multipleNodes = false;

	if ( in_pNode == NULL ) 
	{
		ModContextList	list;
		INodeTab		nodes;

		ip->GetModContexts (list, nodes);

		multipleNodes = (list.Count()>1);

		for (int i = 0; i < list.Count(); i++ )
		{
			EditPolyData *pData = (EditPolyData *)list[i]->localData;
			if (pData == NULL) 
				continue;

			if (pData->GetFlag (kEPDataPrimary))
			{
				in_pNode = nodes[i]->GetActualINode();
				break;
			}
		}

		if (in_pNode == NULL) in_pNode = nodes[0]->GetActualINode();

		nodes.DisposeTemporary ();
	}


	EditPolyData *l_polySelData = GetEditPolyDataForNode (in_pNode);

	if ( l_polySelData )
	{
		theHold.Begin ();
		theHold.Put (new SelRingLoopRestore(this,l_polySelData));
		theHold.Accept(GetString(IDS_SELECT_EDGE_RING));
	}


	if (in_val == RING_UP || in_val == RING_UP_ADD || in_val == RING_UP_SUBTRACT )
		l_spinVal = mRingSpinnerValue + 1 ;
	else if ( in_val == RING_DOWN  || in_val == RING_DOWN_ADD || in_val == RING_DOWN_SUBTRACT )
		l_spinVal = mRingSpinnerValue - 1 ;

	// update the ctrl-add settings 
	if ( in_val == RING_UP_ADD  || in_val == RING_DOWN_ADD)
	{
		setAltKey(false);
		setCtrlKey(true);
	}
	else if ( in_val == RING_UP_SUBTRACT || in_val == RING_DOWN_SUBTRACT )
	{
		setCtrlKey(false);
		setAltKey(true);
	}

	// call the selection ring method
	UpdateRingEdgeSelection(l_spinVal);

	// reset all keys 
	setCtrlKey(false);
	setAltKey(false);

	EpModLocalDataChanged (PART_SELECT);
	EpModRefreshScreen ();


}

void EditPolyMod::EpModUpdateLoopEdgeSelection (  int in_val, INode *in_pNode)
{
	int l_spinVal = 0; 

	bool multipleNodes = false;

	if ( in_pNode == NULL ) 
	{
		ModContextList	list;
		INodeTab		nodes;

		ip->GetModContexts (list, nodes);

		multipleNodes = (list.Count()>1);

		for (int i = 0; i < list.Count(); i++ )
		{
			EditPolyData *pData = (EditPolyData *)list[i]->localData;
			if (pData == NULL) 
				continue;

			if (pData->GetFlag (kEPDataPrimary))
			{
				in_pNode = nodes[i]->GetActualINode();
				break;
			}
		}

		if (in_pNode == NULL) in_pNode = nodes[0]->GetActualINode();

		nodes.DisposeTemporary ();
	}

	EditPolyData *l_polySelData = GetEditPolyDataForNode (in_pNode);

	if ( l_polySelData )
	{
		theHold.Begin ();
		theHold.Put (new SelRingLoopRestore(this,l_polySelData));
		theHold.Accept(GetString(IDS_SELECT_EDGE_LOOP));
	}

	if (in_val == LOOP_UP || in_val == LOOP_UP_ADD || in_val == LOOP_UP_SUBTRACT )
		l_spinVal = mLoopSpinnerValue + 1 ;
	else if ( in_val == LOOP_DOWN  || in_val == LOOP_DOWN_ADD || in_val == LOOP_DOWN_SUBTRACT )
		l_spinVal = mLoopSpinnerValue - 1;


	// update the ctrl-add settings 
	if ( in_val == LOOP_UP_ADD || in_val ==LOOP_DOWN_ADD )
	{
		setAltKey(false);
		setCtrlKey(true);
	}
	else if ( in_val == LOOP_UP_SUBTRACT || in_val == LOOP_DOWN_SUBTRACT )
	{
		setCtrlKey(false );
		setAltKey(true);
	}

	// call the selection ring method
	UpdateLoopEdgeSelection(l_spinVal);

	// reset keys
	setAltKey(false);
	setCtrlKey(false );

	EpModLocalDataChanged (PART_SELECT);
	EpModRefreshScreen ();
}




void EditPolyMod::EpModSetLoopShift(  int in_val, bool in_MoveOnly, bool in_Add )
{

	// update the ctrl-add settings 
	if ( in_MoveOnly )
	{
		setCtrlKey(false);
		setAltKey(false);
	}
	else 
	{
		if ( in_Add )
		{
			setAltKey(false);
			setCtrlKey(true);
		}
		else 
		{
			setCtrlKey(false);
			setAltKey(true);
		}
	}

	for ( int i = 0; i < abs ( in_val); i++)
	{
		if ( in_val >=0 )
			this->UpdateLoopEdgeSelection(i+1);
		else 
			this->UpdateLoopEdgeSelection(-i-1);
	
	}

	EpModLocalDataChanged (PART_SELECT);
	EpModRefreshScreen ();
}

void EditPolyMod::EpModSetRingShift(  int in_val, bool in_MoveOnly, bool in_Add )
{

	// update the ctrl-add settings 
	if ( in_MoveOnly )
	{
		setCtrlKey(false);
		setAltKey(false);
	}
	else 
	{
		if ( in_Add )
		{
			setAltKey(false);
			setCtrlKey(true);
		}
		else 
		{
			setCtrlKey(false);
			setAltKey(true);
		}
	}

	for ( int i = 0; i < abs (in_val); i++)
	{
		if ( in_val >=0 )
			this->UpdateRingEdgeSelection(i+1);
		else 
			this->UpdateRingEdgeSelection(-i-1);

	}

	EpModLocalDataChanged (PART_SELECT);
	EpModRefreshScreen ();
}

class TurnEdgeRestore : public RestoreObj
{
private:
	int mEdge, mFace;
	EditPolyData *mpData;
	EditPolyMod *mpMod;
public:
	TurnEdgeRestore (EditPolyMod *pMod, EditPolyData *pData, int edge, int face) :
	  mpMod(pMod), mpData(pData), mEdge(edge), mFace(face) { }

	void Restore (int isUndo) {
		if (!mpData->GetPolyOpData()) return;
		if (mpData->GetPolyOpData()->OpID() != ep_op_turn_edge) return;
		LocalTurnEdgeData *pTurn = (LocalTurnEdgeData *) mpData->GetPolyOpData();
		pTurn->RemoveLast ();
		mpMod->EpModLocalDataChanged (PART_DISPLAY);
	}
	void Redo () {
		if (!mpData->GetPolyOpData()) return;
		if (mpData->GetPolyOpData()->OpID() != ep_op_turn_edge) return;
		LocalTurnEdgeData *pTurn = (LocalTurnEdgeData *) mpData->GetPolyOpData();
		pTurn->TurnDiagonal (mEdge, mFace);
		mpMod->EpModLocalDataChanged (PART_DISPLAY);
	}
	int Size () { return 16; }
	TSTR Description() { return TSTR(_T("TurnEdge")); }
};

/*
void EditPolyMod::EpModTurnEdge (int edge, INode *pNode)
{
	// Don't cooperate if we're not in edge level.
	if (meshSelLevel[selLevel] != MNM_SL_EDGE) return;

	EditPolyData *pData = GetEditPolyDataForNode (pNode);

	// Make sure we're set to the right operation:
	EpModSetOperation (ep_op_turn_edge);
	pData->SetPolyOpData (ep_op_turn_edge);

	if (theHold.Holding()) theHold.Put (new TurnEdgeRestore (this, pData, edge, -1));

	LocalTurnEdgeData *pTurn = (LocalTurnEdgeData *) pData->GetPolyOpData();
	pTurn->TurnEdge (edge);

	EpModLocalDataChanged (PART_DISPLAY);

	// Macro recording:
	if (macroRecorder->Enabled())
	{
		ModContextList list;
		INodeTab nodes;
		ip->GetModContexts (list, nodes);
		if ((list.Count()>1) && (pNode == NULL))
		{
			for (int i=0; i<list.Count(); i++)
			{
				EditPolyData *pData = (EditPolyData *)list[i]->localData;
				if (pData == NULL) continue;
				if (pData->GetFlag (kEPDataPrimary))
				{
					pNode = nodes[i]->GetActualINode();
					break;
				}
			}
		}
		nodes.DisposeTemporary ();

		if (pNode != NULL)
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].TurnEdge"), 1, 1,
				mr_int, edge+1, _T("node"), mr_reftarg, pNode);
		}
		else
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].TurnEdge"), 1, 0,
				mr_int, edge+1);
		}
		macroRecorder->EmitScript();
	}
}
*/

void EditPolyMod::EpModTurnDiagonal (int face, int diag, INode *pNode)
{
	EditPolyData *pData = GetEditPolyDataForNode (pNode);

	// Make sure we're set to the right operation:
	EpModSetOperation (ep_op_turn_edge);
	pData->SetPolyOpData (ep_op_turn_edge);

	if (theHold.Holding()) theHold.Put (new TurnEdgeRestore (this, pData, diag, face));

	LocalTurnEdgeData *pTurn = (LocalTurnEdgeData *) pData->GetPolyOpData();
	pTurn->TurnDiagonal (face, diag);

	EpModLocalDataChanged (PART_DISPLAY);

	// Macro recording:
	if (macroRecorder->Enabled())
	{
		ModContextList list;
		INodeTab nodes;
		ip->GetModContexts (list, nodes);
		if ((list.Count()>1) && (pNode == NULL))
		{
			for (int i=0; i<list.Count(); i++)
			{
				EditPolyData *pData = (EditPolyData *)list[i]->localData;
				if (pData == NULL) continue;
				if (pData->GetFlag (kEPDataPrimary))
				{
					pNode = nodes[i]->GetActualINode();
					break;
				}
			}
		}
		nodes.DisposeTemporary ();

		if (pNode != NULL)
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].TurnDiagonal"), 2, 1,
				mr_int, face+1, mr_int, diag+1, _T("node"), mr_reftarg, pNode);
		}
		else
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].TurnDiagonal"), 2, 0,
				mr_int, face+1, mr_int, diag+1);
		}
		macroRecorder->EmitScript();
	}
}

class AttachRestore : public RestoreObj
{
private:
	EditPolyData *mpData;
	EditPolyMod *mpMod;
	MNMesh *mpMesh;
	int mNumMats;
public:
	AttachRestore (EditPolyMod *pMod, EditPolyData *pData, INode *node) : mpMod(pMod), mpData(pData) { }

	void Restore (int isUndo) {
		if (!mpData->GetPolyOpData()) return;
		if (mpData->GetPolyOpData()->OpID() != ep_op_attach) return;
		LocalAttachData *pAttach = (LocalAttachData *) mpData->GetPolyOpData();
		mpMesh = pAttach->PopMesh (mNumMats);
		mpMod->EpModLocalDataChanged (PART_DISPLAY);
	}
	void Redo () {
		if (!mpData->GetPolyOpData()) return;
		if (mpData->GetPolyOpData()->OpID() != ep_op_attach) return;
		LocalAttachData *pAttach = (LocalAttachData *) mpData->GetPolyOpData();
		pAttach->PushMesh (mpMesh, mNumMats);
		mpMod->EpModLocalDataChanged (PART_DISPLAY);
	}
	int Size () { return 12; }
	TSTR Description() { return TSTR(_T("Attach")); }
};

void EditPolyMod::EpModAttach (INode *node, INode *pNodeArg, TimeValue t) {
	INode *pNode = pNodeArg;
	bool multipleNodes = false;
	if (pNode == NULL) {
		ModContextList list;
		INodeTab nodes;
		ip->GetModContexts (list, nodes);
		multipleNodes = (list.Count()>1);
		for (int i=0; i<list.Count(); i++)
		{
			EditPolyData *pData = (EditPolyData *)list[i]->localData;
			if (pData == NULL) continue;
			if (pData->GetFlag (kEPDataPrimary))
			{
				pNode = nodes[i]->GetActualINode();
				break;
			}
		}
		if (pNode == NULL) pNode = nodes[0]->GetActualINode();
		nodes.DisposeTemporary ();
	}
	EditPolyData *pData = GetEditPolyDataForNode (pNode);

	// Make sure we're set to the right operation:
	EpModSetOperation (ep_op_attach);
	pData->SetPolyOpData (ep_op_attach);

	// Add the given node to the attach list:
	//mpParams->Append (epm_attach_nodes, 1, &node);

	LocalAttachData *pAttach = (LocalAttachData *) pData->GetPolyOpData();
	pAttach->AppendNode (node, this, pNode, t, mAttachType, mAttachCondense);
	ip->DeleteNode(node);

	EpModLocalDataChanged (PART_ALL - PART_SUBSEL_TYPE);
	EpModCommit (t, false, true);

	if (multipleNodes || pNodeArg != NULL) {
		macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].Attach"), 1, 1,
			mr_reftarg, node, _T("editPolyNode"), mr_reftarg, pNode);
	}
	else
	{
		macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].Attach"), 1, 0, mr_reftarg, node);
	}
	macroRecorder->EmitScript();
}

void EditPolyMod::EpModMultiAttach (Tab<INode *> & nodeTab, INode *pNodeArg, TimeValue t) {
	INode *pNode = pNodeArg;
	bool multipleNodes = false;
	if (pNode == NULL) {
		ModContextList list;
		INodeTab nodes;
		ip->GetModContexts (list, nodes);
		multipleNodes = (list.Count()>1);
		for (int i=0; i<list.Count(); i++)
		{
			EditPolyData *pData = (EditPolyData *)list[i]->localData;
			if (pData == NULL) continue;
			if (pData->GetFlag (kEPDataPrimary))
			{
				pNode = nodes[i]->GetActualINode();
				break;
			}
		}
		if (pNode == NULL) pNode = nodes[0]->GetActualINode();
		nodes.DisposeTemporary ();
	}
	EditPolyData *pData = GetEditPolyDataForNode (pNode);

	// Make sure we're set to the right operation:
	SetOperation (ep_op_attach, false, false);
	EpModSetOperation (ep_op_attach);
	pData->SetPolyOpData (ep_op_attach);

	LocalAttachData *pAttach = (LocalAttachData *) pData->GetPolyOpData();
	for (int i=0; i<nodeTab.Count(); i++) {
		pAttach->AppendNode (nodeTab[i], this, pNode, t, mAttachType, mAttachCondense);
		ip->DeleteNode (nodeTab[i]);
	}

	EpModLocalDataChanged (PART_ALL - PART_SUBSEL_TYPE);
	EpModCommit (t, false, true);

	for (int i=0; i<nodeTab.Count(); i++) {
		if (multipleNodes || pNodeArg != NULL) {
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].Attach"), 1, 1,
				mr_reftarg, nodeTab[i], _T("editPolyNode"), mr_reftarg, pNode);
		}
		else
		{
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].Attach"), 1, 0,
				mr_reftarg, nodeTab[i]);
		}
	}
	macroRecorder->EmitScript();
}

// We assume here that the relevant geometry has already been cloned or detached from the main mesh.
void EditPolyMod::EpModDetachToObject (TSTR & name, TimeValue t) {
	// Animation confuses things.
	SuspendAnimate();
	AnimateOff();

	int msl = meshSelLevel[selLevel];
	bool stackSel = mpParams->GetInt(epm_stack_selection) != 0;

	ModContextList list;
	INodeTab nodes;
	ip->GetModContexts (list, nodes);

	int i;
	bool somethingDetached(false);
	for (int nd=0; nd<list.Count(); nd++) {
		EditPolyData *pData = (EditPolyData *) list[nd]->localData;
		if (pData == NULL) continue;

		MNMesh *pMesh = pData->GetMesh();
		if (pMesh == NULL) continue;

		// Set the MN_USER flag on the faces we're planning to detach.
		pMesh->ClearFFlags (MN_USER);
		int numSel = 0, isoVertCount=0;
		if (stackSel) {
			numSel = pMesh->PropegateComponentFlags (MNM_SL_FACE, MN_USER, msl, MN_EDITPOLY_STACK_SELECT);
			if (msl == MNM_SL_VERTEX) {
				// Check for isolated vertices:
				for (i=0; i<pMesh->numv; i++) {
					if (pMesh->v[i].GetFlag (MN_EDITPOLY_STACK_SELECT) && (pMesh->vedg[i].Count() == 0))
						isoVertCount++;
				}
			}
		} else {
			switch (msl) {
				case MNM_SL_VERTEX:
					pMesh->ClearVFlags (MN_USER);
					for (i=0; i<pMesh->numv; i++) {
						if (!pData->mVertSel[i]) continue;
						pMesh->v[i].SetFlag (MN_USER);
						if (pMesh->vedg[i].Count() == 0) isoVertCount++;
					}
					numSel = pMesh->PropegateComponentFlags (MNM_SL_FACE, MN_USER, msl, MN_USER);
					break;

				case MNM_SL_EDGE:
					pMesh->ClearEFlags (MN_USER);
					for (i=0; i<pMesh->nume; i++) {
						if (!pData->mEdgeSel[i]) continue;
						pMesh->e[i].SetFlag (MN_USER);
					}
					numSel = pMesh->PropegateComponentFlags (MNM_SL_FACE, MN_USER, msl, MN_USER);
					break;

				case MNM_SL_FACE:
					for (i=0; i<pMesh->numf; i++) {
						if (!pData->mFaceSel[i]) continue;
						pMesh->f[i].SetFlag (MN_USER);
						numSel++;
					}
					break;
			}
		}

		if (!numSel && !isoVertCount) continue;

		somethingDetached = true;

		// Create the new node, and move our geometry over to it.
		PolyObject *newObj = CreateEditablePolyObject();
		pMesh->DetachElementToObject (newObj->mm, MN_USER, false);	// no point in deleting from temp cache.

		// Add in the isolated vertices, if any:
		if (isoVertCount) {
			int oldNumV = newObj->mm.numv;
			newObj->mm.setNumVerts (oldNumV + isoVertCount);
			isoVertCount = oldNumV;
			DWORD flag = stackSel ? MN_EDITPOLY_STACK_SELECT : MN_USER;
			for (i=0; i<pMesh->numv; i++) {
				if (!pMesh->v[i].GetFlag (flag)) continue;
				if (pMesh->vedg[i].Count()) continue;
				newObj->mm.v[isoVertCount++] = pMesh->v[i];
			}
		}

		// Clear our Edit Poly flags, so they don't clutter up the new object.
		newObj->mm.ClearVFlags (MN_EDITPOLY_STACK_SELECT|MN_USER|MN_EDITPOLY_OP_SELECT);
		newObj->mm.ClearEFlags (MN_EDITPOLY_STACK_SELECT|MN_USER|MN_EDITPOLY_OP_SELECT);
		newObj->mm.ClearFFlags (MN_EDITPOLY_STACK_SELECT|MN_USER|MN_EDITPOLY_OP_SELECT);

		// SCA Fix 1/29/01: the newObj mesh was always being created with the NONTRI flag empty,
		// leading to problems in rendering and a couple other areas.  Instead we copy the
		// NONTRI flag from our current mesh.  Note that this may result in the NONTRI flag
		// being set on a fully triangulated detached region, but that's acceptable; leaving out
		// the NONTRI flag when it is non-triangulated is much more serious.
		newObj->mm.CopyFlags (pMesh, MN_MESH_NONTRI);

		// Add the object to the scene. Give it the given name
		// and set its transform to ours.
		// Also, give it our material.
		INode *node = ip->CreateObjectNode (newObj);
		Matrix3 ntm = nodes[nd]->GetNodeTM(t);
		if (GetCOREInterface()->GetINodeByName(name) != node) {	// Just in case name = "Object01" or some such default.
			TSTR uname = name;
			if (GetCOREInterface()->GetINodeByName (uname)) GetCOREInterface()->MakeNameUnique(uname);
			node->SetName(uname);
		}
		node->SetNodeTM(t,ntm);
		node->CopyProperties (nodes[nd]);
		node->FlagForeground(t,FALSE);
		node->SetMtl(nodes[nd]->GetMtl());
		node->SetObjOffsetPos (nodes[nd]->GetObjOffsetPos());
		node->SetObjOffsetRot (nodes[nd]->GetObjOffsetRot());
		node->SetObjOffsetScale (nodes[nd]->GetObjOffsetScale());
	}

	// Now we need to delete what we just detached.
	if (somethingDetached) {
		bool mre = macroRecorder->Enabled() != 0;
		if (mre) macroRecorder->Disable();
		if (msl == MNM_SL_VERTEX) EpModButtonOp (ep_op_delete_vertex);
		else {
			int delIso = mpParams->GetInt (epm_delete_isolated_verts, t);
			if (!delIso) {
				// "very quietly" turn on Delete Isolated Vertices for a moment.
				mpParams->SetValue (epm_delete_isolated_verts, t, true);
			}
			if (msl == MNM_SL_FACE) EpModButtonOp (ep_op_delete_face);
			else EpModButtonOp (ep_op_delete_edge);
			if (!delIso) {
				mpParams->SetValue (epm_delete_isolated_verts, t, false);
			}
		}
		if (mre) macroRecorder->Enable ();
		EpModLocalDataChanged (PART_ALL - PART_SUBSEL_TYPE);

		if (mre) {
			macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].DetachToObject"), 1, 0, mr_string, name);
			macroRecorder->EmitScript();
		}
	}

	ResumeAnimate();
}

void EditPolyMod::EpModCreateShape (TSTR & shapeObjectName, TimeValue t) {
	// Animation confuses things.
	SuspendAnimate();
	AnimateOff();

	theHold.Begin ();

	int msl = meshSelLevel[selLevel];
	bool stackSel = mpParams->GetInt(epm_stack_selection) != 0;
	bool curved = (mCreateShapeType == IDC_SHAPE_SMOOTH);

	ModContextList list;
	INodeTab nodes;
	ip->GetModContexts (list, nodes);

	int i;
	for (int nd=0; nd<list.Count(); nd++) {
		EditPolyData *pData = (EditPolyData *) list[nd]->localData;
		if (pData == NULL) continue;

		MNMesh *pMesh = pData->GetMesh();
		if (pMesh == NULL) continue;

		HoldSuspend hs; // LAM - 6/12/04 - defect 571821
		int numSel = 0;
		if (stackSel) {
			numSel = pMesh->PropegateComponentFlags (MNM_SL_EDGE, MN_USER, MNM_SL_EDGE, MN_EDITPOLY_STACK_SELECT);
		} else {
			for (i=0; i<pMesh->nume; i++) {
				if (!pData->mEdgeSel[i]) pMesh->e[i].ClearFlag (MN_USER);
				else {
					pMesh->e[i].SetFlag (MN_USER);
					numSel++;
				}
			}
		}
		if (!numSel) continue;

		SplineShape *shape = (SplineShape*)GetSplineShapeDescriptor()->Create(0);	
		BitArray done;
		done.SetSize (pMesh->nume);

		for (i=0; i<pMesh->nume; i++) {
			if (done[i]) continue;
			if (pMesh->e[i].GetFlag (MN_DEAD)) continue;
			if (!pMesh->e[i].GetFlag(MN_USER)) continue;
			if (stackSel) {
				if (!pMesh->e[i].GetFlag (MN_EDITPOLY_STACK_SELECT)) continue;
			} else {
				if (!pData->mEdgeSel[i]) continue;
			}

			// The array of points for the spline
			Tab<Point3> pts;

			// Add the first two points.
			pts.Append(1,&pMesh->v[pMesh->e[i].v1].p,10);
			pts.Append(1,&pMesh->v[pMesh->e[i].v2].p,10);
			int nextv = pMesh->e[i].v2, start = pMesh->e[i].v1;

			// Mark this edge as done
			done.Set(i);

			// Trace along selected edges
			// Use loopcount/maxLoop just to avoid a while(1) loop.
			int loopCount, maxLoop=pMesh->nume;
			for (loopCount=0; loopCount<maxLoop; loopCount++) {
				Tab<int> & ve = pMesh->vedg[nextv];
            int j;
				for (j=0; j<ve.Count(); j++) {
					if (done[ve[j]]) continue;
					if (pMesh->e[ve[j]].GetFlag (MN_USER)) break;
				}
				if (j==ve.Count()) break;
				if (pMesh->e[ve[j]].v1 == nextv) nextv = pMesh->e[ve[j]].v2;
				else nextv = pMesh->e[ve[j]].v1;

				// Mark this edge as done
				done.Set(ve[j]);

				// Add this vertex to the list
				pts.Append(1,&pMesh->v[nextv].p,10);
			}
			int lastV = nextv;

			// Now trace backwards
			nextv = start;
			for (loopCount=0; loopCount<maxLoop; loopCount++) {
				Tab<int> & ve = pMesh->vedg[nextv];
            int j;
				for (j=0; j<ve.Count(); j++) {
					if (done[ve[j]]) continue;
					if (pMesh->e[ve[j]].GetFlag (MN_USER)) break;
				}
				if (j==ve.Count()) break;
				if (pMesh->e[ve[j]].v1 == nextv) nextv = pMesh->e[ve[j]].v2;
				else nextv = pMesh->e[ve[j]].v1;

				// Mark this edge as done
				done.Set(ve[j]);

				// Add this vertex to the list
				pts.Insert(0,1,&pMesh->v[nextv].p);
			}
			int firstV = nextv;

			// Now we've got all the points. Create the spline and add points
			Spline3D *spline = new Spline3D(KTYPE_AUTO,KTYPE_BEZIER);					
			int max = pts.Count();
			if (firstV == lastV) {
				max--;
				spline->SetClosed ();
			}
			if (curved) {
				for (int j=0; j<max; j++) {
					int prvv = j ? j-1 : ((firstV==lastV) ? max-1 : 0);
					int nxtv = (max-1-j) ? j+1 : ((firstV==lastV) ? 0 : max-1);
					float prev_length = Length(pts[j] - pts[prvv])/3.0f;
					float next_length = Length(pts[j] - pts[nxtv])/3.0f;
					Point3 tangent = Normalize (pts[nxtv] - pts[prvv]);
					SplineKnot sn (KTYPE_BEZIER, LTYPE_CURVE, pts[j],
							pts[j] - prev_length*tangent, pts[j] + next_length*tangent);
					spline->AddKnot(sn);
				}
			} else {
				for (int j=0; j<max; j++) {
					SplineKnot sn(KTYPE_CORNER, LTYPE_LINE, pts[j],pts[j],pts[j]);
					spline->AddKnot(sn);
				}
				spline->ComputeBezPoints();
			}
			shape->shape.AddSpline(spline);
		}

		shape->shape.InvalidateGeomCache();
		shape->shape.UpdateSels();

		hs.Resume();
		INode *node = ip->CreateObjectNode (shape);
		hs.Suspend();

		INode *nodeByName = ip->GetINodeByName (shapeObjectName);
		if (nodeByName != node) {
			TSTR localName(shapeObjectName);
			if (nodeByName) ip->MakeNameUnique(localName);
			node->SetName (localName);
		}
		Matrix3 ntm = nodes[nd]->GetNodeTM(ip->GetTime());
		node->SetNodeTM (ip->GetTime(),ntm);
		node->FlagForeground (ip->GetTime(),FALSE);
		node->SetMtl (nodes[nd]->GetMtl());
		node->SetObjOffsetPos (nodes[nd]->GetObjOffsetPos());
		node->SetObjOffsetRot (nodes[nd]->GetObjOffsetRot());
		node->SetObjOffsetScale (nodes[nd]->GetObjOffsetScale());
	}
	ResumeAnimate();

	theHold.Accept (GetString (IDS_CREATE_SHAPE_FROM_EDGES));

	macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].CreateShape"), 1, 0, mr_string, shapeObjectName);
	macroRecorder->EmitScript();
}

void EditPolyMod::SetPreserveMapSettings (const MapBitArray & mapSettings) {
	// Use sequence of calls to EpModSetPreserveMap to get proper macro-recording.
	int i, min, max;
	min = mapSettings.Smallest();
	if (mPreserveMapSettings.Smallest() < min) min = mPreserveMapSettings.Smallest();
	max = mapSettings.Largest();
	if (mPreserveMapSettings.Largest() > max) max = mPreserveMapSettings.Largest();
	for (i=min; i<=max; i++) {
		if (mapSettings[i] != mPreserveMapSettings[i])
			EpModSetPreserveMap (i, mapSettings[i]);
	}
}

class TogglePreserveMapRestore : public RestoreObj {
private:
	EditPolyMod *mpMod;
	int mChannel;
	bool mValue;
public:
	TogglePreserveMapRestore (EditPolyMod *pMod, int channel, bool origVal) : mpMod(pMod), mChannel(channel), mValue(origVal) { }

	void Restore (int isUndo) { mpMod->EpModSetPreserveMap (mChannel, mValue); }
	void Redo () { mpMod->EpModSetPreserveMap (mChannel, !mValue); }
	int Size () { return 9; }
	TSTR Description() { return TSTR(_T("TogglePreserveMap")); }
};

void EditPolyMod::EpModSetPreserveMap (int mapChannel, bool preserve) {
	if (mPreserveMapSettings[mapChannel] == preserve) return;

	if (theHold.Holding()) {
		theHold.Put (new TogglePreserveMapRestore (this, mapChannel, !preserve));
	}

	mPreserveMapSettings.Set (mapChannel, preserve);

	if (mpCurrentOperation && mpCurrentOperation->PreserveMapSettings()) {
		EpModLocalDataChanged ((mapChannel<1) ? PART_VERTCOLOR : PART_TEXMAP);
	}

	macroRecorder->FunctionCall (_T("$.modifiers[#Edit_Poly].SetPreserveMap"), 2, 0,
		mr_int, mapChannel, mr_varname, preserve ? _T("true") : _T("false"));
	macroRecorder->EmitScript();
}

class GeomDeltaRestore : public RestoreObj
{
private:
	Tab<Point3> mBefore, mAfter;
	EditPolyData *mpData;
	EditPolyMod *mpMod;
public:
	GeomDeltaRestore (EditPolyMod *pMod, EditPolyData *pData) : mpMod(pMod), mpData(pData) {
		if (!pData->GetPolyOpData() || (pData->GetPolyOpData()->OpID() != ep_op_transform)) return;
		LocalTransformData *pTransform = (LocalTransformData *) pData->GetPolyOpData();
		mBefore.SetCount (pTransform->NumOffsets());
		if (mBefore.Count()) memcpy (mBefore.Addr(0), pTransform->OffsetPointer(0), mBefore.Count()*sizeof(Point3));
	}

	void After () {
		if (!mpData->GetPolyOpData() || (mpData->GetPolyOpData()->OpID() != ep_op_transform)) return;
		LocalTransformData *pTransform = (LocalTransformData *) mpData->GetPolyOpData();
		mAfter.SetCount (pTransform->NumOffsets());
		if (mAfter.Count()) memcpy (mAfter.Addr(0), pTransform->OffsetPointer(0), mAfter.Count()*sizeof(Point3));
	}

	void Restore (int isUndo) {
		if (!mpData->GetPolyOpData() || (mpData->GetPolyOpData()->OpID() != ep_op_transform)) return;
		LocalTransformData *pTransform = (LocalTransformData *) mpData->GetPolyOpData();
		pTransform->SetNumOffsets (mBefore.Count());
		if (mBefore.Count()) memcpy (pTransform->OffsetPointer(0), mBefore.Addr(0), mBefore.Count()*sizeof(Point3));
		mpMod->EpModLocalDataChanged (PART_DISPLAY|PART_GEOM);
	}
	void Redo () {
		if (!mpData->GetPolyOpData() || (mpData->GetPolyOpData()->OpID() != ep_op_transform)) return;
		LocalTransformData *pTransform = (LocalTransformData *) mpData->GetPolyOpData();
		pTransform->SetNumOffsets (mAfter.Count());
		if (mAfter.Count()) memcpy (pTransform->OffsetPointer(0), mAfter.Addr(0), mAfter.Count()*sizeof(Point3));
		mpMod->EpModLocalDataChanged (PART_DISPLAY|PART_GEOM);
	}
	int Size () { return 16 + (mBefore.Count() + mAfter.Count()) * sizeof(Point3); }
	TSTR Description() { return TSTR(_T("GeomDelta")); }
};

void EditPolyMod::EpModDragInit (INode *pNode, TimeValue t) {
	EditPolyData *pData = GetEditPolyDataForNode (pNode);

	// Make sure we're set to the right operation:
	pData->SetPolyOpData (ep_op_transform);

	LocalTransformData *pTransform = (LocalTransformData *) pData->GetPolyOpData();
	pTransform->ClearTempOffset ();
	pTransform->SetNodeName (pNode->NodeName());
}

void EditPolyMod::EpModDragMove (Tab<Point3> & delta, INode *pNode, TimeValue t) {
	if (delta.Count() == 0) return;
	EditPolyData *pData = GetEditPolyDataForNode (pNode);
	if (!pData->GetPolyOpData() || (pData->GetPolyOpData()->OpID() != ep_op_transform)) return;
	LocalTransformData *pTransform = (LocalTransformData *) pData->GetPolyOpData();
	pTransform->SetTempOffset (delta);
	EpModLocalDataChanged (PART_GEOM|PART_DISPLAY);
}

void EditPolyMod::EpModDragCancel (INode *pNode) {
	EditPolyData *pData = GetEditPolyDataForNode (pNode);
	if (!pData->GetPolyOpData() || (pData->GetPolyOpData()->OpID() != ep_op_transform)) return;
	LocalTransformData *pTransform = (LocalTransformData *) pData->GetPolyOpData();
	pTransform->ClearTempOffset ();
	EpModLocalDataChanged (PART_GEOM|PART_DISPLAY);
}

void EditPolyMod::EpModDragAccept (INode *pNode, TimeValue t) {
	EditPolyData *pData = GetEditPolyDataForNode (pNode);

	// Make sure we're set to the right operation:
	if (!pData->GetPolyOpData() || (pData->GetPolyOpData()->OpID() != ep_op_transform)) return;

	if (AreWeKeying(t) && (mPointControl.Count() == 0)) CreatePointControllers ();

	GeomDeltaRestore *gdrest = NULL;
	if (theHold.Holding()) gdrest = new GeomDeltaRestore (this, pData);

	LocalTransformData *pTransform = (LocalTransformData *) pData->GetPolyOpData();

	TSTR nodeName = pNode->GetName();
   int j;
	for (j=0; j<mPointNode.length(); j++) {
		if (nodeName == mPointNode[j]) break;
	}
	int nodeStart = (j<mPointNode.length()) ? mPointRange[j] : 0;

//#if defined(NEW_SOA)
//	GetPolyObjDescriptor()->Execute(I_EXEC_EVAL_SOA_TIME, reinterpret_cast<ULONG_PTR>(&t));
//#endif

	int numVerts = pData->GetMesh() ? pData->GetMesh()->numv : pData->mVertSel.NumberSet();

	if (AreWeKeying(t) && (j<mPointNode.length())) {
		bool addedCont = false;
		for (int i=0; i<numVerts; i++) {
			if (!pTransform->HasTempOffset(i)) continue;
			if (PlugController (t, i, nodeStart, pNode)) addedCont = true;
		}
		if (addedCont) NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	}

	int oldCt = pTransform->NumOffsets();
	pTransform->SetNumOffsets (numVerts);
	// Don't forget initialization:
	for (int i=oldCt; i<numVerts; i++) pTransform->Offset(i) = Point3(0,0,0);

	int limit = numVerts;
	if (limit>pTransform->NumTempOffsets()) limit = pTransform->NumTempOffsets();
	for (int i=0; i<limit; i++) {
		if (!pTransform->HasTempOffset(i)) continue;
		pTransform->Offset(i) += pTransform->TempOffset(i);
		if (mPointControl.Count() && (j<mPointNode.length())) {
			int k = i+nodeStart;
			if ((k<mPointRange[j+1]) && mPointControl[k])
				mPointControl[k]->SetValue (t, pTransform->OffsetPointer(i));
		}
	}
	pTransform->ClearTempOffset();

	if (gdrest) {
		gdrest->After ();
		theHold.Put (gdrest);
	}

	EpModLocalDataChanged (PART_GEOM|PART_DISPLAY);

	// No macro recording for this method.  (Macro recording handled at Move/Rotate/Scale level.)
}











