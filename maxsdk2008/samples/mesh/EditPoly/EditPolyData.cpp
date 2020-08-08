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

EditPolyData::EditPolyData (PolyObject *pObj, TimeValue t) : mpMesh(NULL), mpMeshOutput(NULL),
	mFlags(0x0), mpNewSelection(NULL), mpOpList(NULL), mpTemp(NULL), mpTempOutput(NULL), mpPolyOpData(NULL),
	mSlicePlaneSize(-1.0f), mpPaintMesh(NULL) {

	Initialize (pObj, t);
}

EditPolyData::EditPolyData () : mpMesh(NULL), mpMeshOutput(NULL), mFlags(0x0),
	mpNewSelection(NULL), mpOpList(NULL), mpTemp(NULL), mpTempOutput(NULL), mpPolyOpData(NULL),
	mSlicePlaneSize(-1.0f), mpPaintMesh(NULL) {
}

EditPolyData::~EditPolyData() {
	DeleteAllOperations ();
	FreeCache();
	FreeNewSelection ();
	ClearPolyOpData();
	if (mpTemp != NULL) delete mpTemp;
	if (mpTempOutput != NULL) delete mpTempOutput;
}

void EditPolyData::Initialize (PolyObject *pObj, TimeValue t) {
	MNMesh & mesh = pObj->GetMesh();
	mesh.getVertexSel (mVertSel);
	mesh.getFaceSel (mFaceSel);
	mesh.getEdgeSel (mEdgeSel);
	mesh.getVerticesByFlag (mVertHide, MN_HIDDEN);
	mesh.getFacesByFlag (mFaceHide, MN_HIDDEN);

	mesh.ClearSpecifiedNormals ();

	// Initialize the STACK_SELECT flags.
	mesh.ClearVFlags (MN_EDITPOLY_STACK_SELECT);
	mesh.ClearEFlags (MN_EDITPOLY_STACK_SELECT);
	mesh.ClearFFlags (MN_EDITPOLY_STACK_SELECT);
	mesh.PropegateComponentFlags (MNM_SL_VERTEX, MN_EDITPOLY_STACK_SELECT, MNM_SL_VERTEX, MN_SEL);
	mesh.PropegateComponentFlags (MNM_SL_EDGE, MN_EDITPOLY_STACK_SELECT, MNM_SL_EDGE, MN_SEL);
	mesh.PropegateComponentFlags (MNM_SL_FACE, MN_EDITPOLY_STACK_SELECT, MNM_SL_FACE, MN_SEL);

	SetCache (pObj, t);
}

LocalModData *EditPolyData::Clone() {
	EditPolyData *d = new EditPolyData;
	d->mVertSel = mVertSel;
	d->mFaceSel = mFaceSel;
	d->mEdgeSel = mEdgeSel;
	d->mVertHide = mVertHide;
	d->mFaceHide = mFaceHide;

	// Copy the Operation linked list:
	PolyOperationRecord *clone=NULL;
	for (PolyOperationRecord *pop = mpOpList; pop != NULL; pop = pop->Next())
	{
		if (clone == NULL) {
			clone = pop->Clone ();
			d->mpOpList = clone;
		} else {
			clone->SetNext (pop->Clone ());
			clone = clone->Next ();
		}
	}

	// Copy any current operation data:
	if (mpPolyOpData != NULL) {
		d->SetPolyOpData (mpPolyOpData->Clone());
	}

	return d;
}

void EditPolyData::SynchBitArrays() {
	if (!mpMesh) return;
	if (mVertSel.GetSize() != mpMesh->VNum()) mVertSel.SetSize(mpMesh->VNum(),TRUE);
	if (mFaceSel.GetSize() != mpMesh->FNum()) mFaceSel.SetSize(mpMesh->FNum(),TRUE);
	if (mEdgeSel.GetSize() != mpMesh->ENum()) mEdgeSel.SetSize(mpMesh->ENum(),TRUE);
	if (mVertHide.GetSize() != mpMesh->VNum()) mVertHide.SetSize (mpMesh->VNum(), true);
	if (mFaceHide.GetSize() != mpMesh->FNum()) mFaceHide.SetSize (mpMesh->FNum(), true);
	UpdatePaintObject(mpMesh);
}

void EditPolyData::SetCache(PolyObject *pObj, TimeValue t) {
	if (mpMesh) delete mpMesh;
	mpMesh = new MNMesh;
	*mpMesh = pObj->GetMesh();

#ifdef __DEBUG_PRINT_EDIT_POLY
	DebugPrint ("EditPolyData::Setting main mesh cache.\n");
#endif

	mGeomValid = pObj->ChannelValidity (t, GEOM_CHAN_NUM);
	mTopoValid = pObj->ChannelValidity (t, TOPO_CHAN_NUM);
	mMeshValid = mGeomValid & mTopoValid & pObj->ChannelValidity (t, VERT_COLOR_CHAN_NUM)
		& pObj->ChannelValidity (t, TEXMAP_CHAN_NUM) & pObj->ChannelValidity (t, SELECT_CHAN_NUM);

	SynchBitArrays ();
	InvalidateTempData (PART_ALL);
}

void EditPolyData::FreeCache() {
	if (mpMesh) delete mpMesh;
	mpMesh = NULL;
	mGeomValid.SetEmpty ();
	mTopoValid.SetEmpty ();
	mMeshValid.SetEmpty ();
	FreeOutputCache ();
	FreePaintMesh ();
	InvalidateTempData (PART_ALL);
}

void EditPolyData::SetOutputCache (MNMesh & mesh) {
	if (mpMeshOutput) delete mpMeshOutput;
	mpMeshOutput = new MNMesh();
	mpMeshOutput->CopyBasics (mesh, true);

	if (mpTempOutput) {
		delete mpTempOutput;
		mpTempOutput = NULL;
	}

#ifdef __DEBUG_PRINT_EDIT_POLY
	DebugPrint ("EditPolyData::Setting output cache.\n");
#endif
}

void EditPolyData::FreeOutputCache () {
	if (mpMeshOutput) delete mpMeshOutput;
	mpMeshOutput = NULL;
}

MNMesh *EditPolyData::GetPaintMesh () {
	if (!mpPolyOpData || (mpPolyOpData->OpID() != ep_op_paint_deform)) return mpMesh;
	LocalPaintDeformData *pPaint = (LocalPaintDeformData *) mpPolyOpData;
	if (pPaint->NumVertices() == 0) return mpMesh;

	if (!mpMesh) return NULL;

	if (pPaint->NumVertices() != mpMesh->numv) {
		// This can happen if the topology underneath us has changed.
		// There's no risk of data loss, since the pPaint->GetMesh() is entirely derived
		// from the mpMesh and the current paint data.  So here, we just correct it, and if the
		// number of vertices goes down and then back up, we still get back our original deformations.
		int oldCount = pPaint->NumVertices(), newCount = mpMesh->numv, paintCount = GetPaintDeformCount();
		Point3* offsets = GetPaintDeformOffsets();

		pPaint->GetMesh()->setNumVerts (newCount);
		// Initialization!  Very important.
		for( int i=oldCount; i<newCount; i++ ) {
			pPaint->GetMesh()->V(i)->p = mpMesh->V(i)->p;
			if( i<paintCount ) pPaint->GetMesh()->V(i)->p += offsets[i];
		}
	}

	if (!mpPaintMesh) mpPaintMesh = new MNMesh ();

	// Refresh these every time, to be safe.  (Lightweight operation.)
	mpPaintMesh->ShallowCopy (mpMesh, PART_TOPO);
	mpPaintMesh->ShallowCopy (pPaint->GetMesh(), PART_GEOM);

	return mpPaintMesh;
}

void EditPolyData::FreePaintMesh () {
	if (!mpPaintMesh) return;
	// Just zero out all the channels...
	mpPaintMesh->FreeChannels (0, true);
	// ... and delete normally.
	delete mpPaintMesh;
	mpPaintMesh = NULL;
}

void EditPolyData::SetVertHide (BitArray &set, EditPolyMod *mod) {
	if (theHold.Holding()) theHold.Put (new EditPolyHideRestore (mod, this, false));
	mVertHide = set;
	if (mpMesh) {
		mVertHide.SetSize (mpMesh->numv);
	}

	InvalidateTempData (PART_DISPLAY);
}

void EditPolyData::SetFaceHide (BitArray &set, EditPolyMod *mod) {
	if (theHold.Holding()) theHold.Put (new EditPolyHideRestore (mod, this, true));
	mFaceHide = set;
	if (mpMesh) {
		mFaceHide.SetSize (mpMesh->numf);
	}

	InvalidateTempData (PART_DISPLAY);
}

void EditPolyData::SetVertSel(BitArray &set, IMeshSelect *imod, TimeValue t) {
	EditPolyMod *mod = (EditPolyMod *) imod;
	if (theHold.Holding()) theHold.Put (new EditPolySelectRestore (mod, this, EPM_SL_VERTEX));

	mVertSel = set;
	int max = mVertSel.GetSize() < mVertHide.GetSize() ? mVertSel.GetSize() : mVertHide.GetSize();
	for (int i=0; i<max; i++) if (mVertHide[i]) mVertSel.Clear(i);

	if (mpMesh) {
		mVertSel.SetSize (mpMesh->numv);
		mpMesh->VertexSelect (mVertSel);
	}

	InvalidateTempData (PART_SELECT);
}

void EditPolyData::SetEdgeSel(BitArray &set, IMeshSelect *imod, TimeValue t) {
	EditPolyMod *mod = (EditPolyMod *) imod;
	if (theHold.Holding()) theHold.Put (new EditPolySelectRestore (mod, this, EPM_SL_EDGE));

	mEdgeSel = set;

	if (mpMesh) {
		mEdgeSel.SetSize (mpMesh->nume);
		for (int i=0; i<mpMesh->nume; i++) {
			if (mpMesh->e[i].GetFlag (MN_DEAD)) continue;
			if (HiddenFace(mpMesh->e[i].f1) && ((mpMesh->e[i].f2<0) || HiddenFace (mpMesh->e[i].f2)))
				mEdgeSel.Clear(i);
		}

		mpMesh->EdgeSelect (mEdgeSel);
	}

	InvalidateTempData (PART_SELECT);
}

void EditPolyData::SetFaceSel(BitArray &set, IMeshSelect *imod, TimeValue t) {
	EditPolyMod *mod = (EditPolyMod *) imod;
	if (theHold.Holding()) theHold.Put (new EditPolySelectRestore (mod, this, EPM_SL_FACE));

	mFaceSel = set;
	int max = mFaceSel.GetSize() < mFaceHide.GetSize() ? mFaceSel.GetSize() : mFaceHide.GetSize();
	for (int i=0; i<max; i++) if (mFaceHide[i]) mFaceSel.Clear(i);

	if (mpMesh) {
		mFaceSel.SetSize (mpMesh->numf);
		mpMesh->FaceSelect (mFaceSel);
	}

	InvalidateTempData (PART_SELECT);
}

void EditPolyData::SetupNewSelection (int meshSelLevel) {
	SynchBitArrays ();
	if (!mpNewSelection) mpNewSelection = new BitArray;
	switch (meshSelLevel) {
	case MNM_SL_VERTEX:
		mpNewSelection->SetSize (mVertSel.GetSize());
		break;
	case MNM_SL_EDGE:
		mpNewSelection->SetSize (mEdgeSel.GetSize());
		break;
	case MNM_SL_FACE:
		mpNewSelection->SetSize (mFaceSel.GetSize());
		break;
	default:
		delete mpNewSelection;
		mpNewSelection = NULL;
	}
}

BitArray *EditPolyData::GetCurrentSelection (int msl, bool useStackSelection) {
	static BitArray staticSelection;

	if (useStackSelection && mpMesh) {
		switch (msl) {
		case MNM_SL_VERTEX:
			mpMesh->getVerticesByFlag (staticSelection, MN_EDITPOLY_STACK_SELECT);
			return &staticSelection;
		case MNM_SL_EDGE:
			mpMesh->getEdgesByFlag (staticSelection, MN_EDITPOLY_STACK_SELECT);
			return &staticSelection;
		case MNM_SL_FACE:
			mpMesh->getFacesByFlag (staticSelection, MN_EDITPOLY_STACK_SELECT);
			return &staticSelection;
		}
	} else {
		switch (msl)
		{
		case MNM_SL_VERTEX:
			return &mVertSel;
		case MNM_SL_EDGE:
			return &mEdgeSel;
		case MNM_SL_FACE:
			return &mFaceSel;
		}
	}

	return NULL;
}
///may be better to change this so the level is passed in (EPM_SL_OBJECT) etc.. YES THAT WOULD HAVE BEEN BETTER!!!
bool EditPolyData::ApplyNewSelection (EditPolyMod *pMod, bool keepOld, bool invert, bool select) {
	if (!mpNewSelection) return false;
	if (!pMod) return false;
	if (pMod->GetEPolySelLevel() == EPM_SL_OBJECT) return false;

	// Do the conversion to whole-border and whole-element selections:
	//if (pMod->GetEPolySelLevel() == EPM_SL_BORDER) ConvertNewSelectionToBorder ();
	//if (pMod->GetEPolySelLevel() == EPM_SL_ELEMENT) ConvertNewSelectionToElement ();
	
	int selLevel = pMod->GetEPolySelLevel();
	if (keepOld) {
		// Find the current selection:
		BitArray *pCurrentSel = NULL;
		if(pMod->GetSelectMode()==PS_MULTI_SEL)
		{
			switch(pMod->GetCalcMeshSelLevel(selLevel))
			{
				case MNM_SL_VERTEX:
					pCurrentSel = &mVertSel;
					break;
				case MNM_SL_EDGE:
					pCurrentSel = &mEdgeSel;
					break;
				default:
					pCurrentSel = &mFaceSel;
					break;
			};

		}
		else
		{
			switch (selLevel) {
			case EPM_SL_VERTEX:
				pCurrentSel = &mVertSel;
				break;
			case EPM_SL_EDGE:
			case EPM_SL_BORDER:
				pCurrentSel = &mEdgeSel;
				break;
			case EPM_SL_FACE:
			case EPM_SL_ELEMENT:
				pCurrentSel = &mFaceSel;
				break;
			}
		}

		// Mix it properly with the new selection:
		if (invert) {
			// Bits in result should be set if set in exactly one of current, new selections
			(*mpNewSelection) ^= *pCurrentSel;
		} else {
			if (select) {
				// Result set if set in either of current, new:
				*mpNewSelection |= *pCurrentSel;
			} else {
				// Result set if in current, and _not_ in new:
				*mpNewSelection = ~(*mpNewSelection);
				*mpNewSelection &= *pCurrentSel;
			}
		}
	}

	bool ret = true;
	if(pMod->GetSelectMode()==PS_MULTI_SEL)
	{

		switch(pMod->GetCalcMeshSelLevel(selLevel))
		{
		case MNM_SL_VERTEX:
			if (*mpNewSelection == mVertSel) {
				ret = false;
				break;
			}
			SetVertSel (*mpNewSelection, pMod, TimeValue(0));
			break;
		case MNM_SL_EDGE:
			if (*mpNewSelection == mEdgeSel) {
				ret = false;
				break;
			}
			SetEdgeSel (*mpNewSelection, pMod, TimeValue(0));
			break;
		case MNM_SL_FACE:
			if (*mpNewSelection == mFaceSel) {
				ret = false;
				break;
			}
			SetFaceSel (*mpNewSelection, pMod, TimeValue(0));
			break;
		default:
			ret = false;
			break;
		}
	}
	else
	{
		switch (selLevel) {
		case EPM_SL_OBJECT:
			ret = false;
			break;
		case EPM_SL_VERTEX:
			if (*mpNewSelection == mVertSel) {
				ret = false;
				break;
			}
			SetVertSel (*mpNewSelection, pMod, TimeValue(0));
			break;
		case EPM_SL_EDGE:
		case EPM_SL_BORDER:
			if (*mpNewSelection == mEdgeSel) {
				ret = false;
				break;
			}
			SetEdgeSel (*mpNewSelection, pMod, TimeValue(0));
			break;
		case EPM_SL_FACE:
		case EPM_SL_ELEMENT:
			if (*mpNewSelection == mFaceSel) {
				ret = false;
				break;
			}
			SetFaceSel (*mpNewSelection, pMod, TimeValue(0));
			break;
		}
	}

	delete mpNewSelection;
	mpNewSelection = NULL;

	return ret;
}

void EditPolyData::TranslateNewSelection (int selLevelFrom, int selLevelTo) {
	if (!mpNewSelection) return;
	if (!mpMesh) {
		DbgAssert (0);
		return;
	}

	BitArray intermediateSelection;
	BitArray toSelection;
	switch (selLevelFrom) {
	case EPM_SL_VERTEX:
		switch (selLevelTo) {
		case EPM_SL_EDGE:
			mSelConv.VertexToEdge (*mpMesh, *mpNewSelection, toSelection);
			break;
		case EPM_SL_BORDER:
			mSelConv.VertexToEdge (*mpMesh, *mpNewSelection, intermediateSelection);
			mSelConv.EdgeToBorder (*mpMesh, intermediateSelection, toSelection);
			break;
		case EPM_SL_FACE:
			mSelConv.VertexToFace (*mpMesh, *mpNewSelection, toSelection);
			break;
		case EPM_SL_ELEMENT:
			mSelConv.VertexToFace (*mpMesh, *mpNewSelection, intermediateSelection);
			mSelConv.FaceToElement (*mpMesh, intermediateSelection, toSelection);
			break;
		}
		break;

	case EPM_SL_EDGE:
		if (selLevelTo == EPM_SL_BORDER) {
			mSelConv.EdgeToBorder (*mpMesh, *mpNewSelection, toSelection);
		}
		break;

	case EPM_SL_FACE:
		if (selLevelTo == EPM_SL_ELEMENT) {
			mSelConv.FaceToElement (*mpMesh, *mpNewSelection, toSelection);
		}
		break;
	}

	if (toSelection.GetSize() == 0) return;
	*mpNewSelection = toSelection;
}

void EditPolyData::Hide (EditPolyMod *pMod, bool isFaceLevel, bool hideSelected)
{
	if (theHold.Holding()) {
		theHold.Put (new EditPolyHideRestore (pMod, this, false));
		if (isFaceLevel) theHold.Put (new EditPolyHideRestore (pMod, this, true));
		if (isFaceLevel || hideSelected) theHold.Put (new EditPolySelectRestore (pMod, this, EPM_SL_VERTEX));
		if (isFaceLevel && hideSelected) theHold.Put (new EditPolySelectRestore (pMod, this, EPM_SL_FACE));
	}

	SynchBitArrays ();

	bool stackSel = pMod->getParamBlock()->GetInt (epm_stack_selection) != 0;

	if (isFaceLevel) {
		BitArray & faceSel = *(GetCurrentSelection (MNM_SL_FACE, stackSel));
		int i, max = faceSel.GetSize();
		if (mFaceHide.GetSize()<max) max = mFaceHide.GetSize();
		if (hideSelected) {
			if (mpMesh) {
				// Hide vertices used by all hidden or selected faces first:
				BitArray touchedByHidden, touchedByNonHidden;
				touchedByHidden.SetSize (mpMesh->numv);
				touchedByNonHidden.SetSize (mpMesh->numv);
				int mmax = max;
				if (mmax>mpMesh->numf) mmax = mpMesh->numf;
				for (i=0; i<mmax; i++) {
					if (faceSel[i]) {
						for (int j=0; j<mpMesh->f[i].deg; j++) touchedByHidden.Set (mpMesh->f[i].vtx[j]);
					} else if (!mFaceHide[i]) {
						for (int j=0; j<mpMesh->f[i].deg; j++) touchedByNonHidden.Set (mpMesh->f[i].vtx[j]);
					}
				}
				for (i=0; i<mpMesh->numv; i++) {
					if (touchedByNonHidden[i]) touchedByHidden.Clear(i);
				}

				// Ok, now everything left in touchedByHidden should be hidden and deselected.
				mVertHide |= touchedByHidden;
				if (!stackSel) mVertSel &= ~touchedByHidden;
			}
			for (i=0; i<max; i++) {
				if (faceSel[i]) {
					if (!stackSel) faceSel.Clear(i);
					mFaceHide.Set(i,true);
				}
			}
		} else {
			if (mpMesh) {
				// Hide vertices used by all hidden or nonselected faces first:
				BitArray touchedByHidden, touchedByNonHidden;
				touchedByHidden.SetSize (mpMesh->numv);
				touchedByNonHidden.SetSize (mpMesh->numv);
				int mmax = max;
				if (mmax>mpMesh->numf) mmax = mpMesh->numf;
				for (i=0; i<mmax; i++) {
					if (!faceSel[i]) {
						for (int j=0; j<mpMesh->f[i].deg; j++) touchedByHidden.Set (mpMesh->f[i].vtx[j]);
					} else if (!mFaceHide[i]) {
						for (int j=0; j<mpMesh->f[i].deg; j++) touchedByNonHidden.Set (mpMesh->f[i].vtx[j]);
					}
				}
				for (i=0; i<mpMesh->numv; i++) {
					if (touchedByNonHidden[i]) touchedByHidden.Clear(i);
				}

				// Ok, now everything left in touchedByHidden should be hidden and deselected.
				mVertHide |= touchedByHidden;
				if (!stackSel) mVertSel &= ~touchedByHidden;
			}
			for (int i=0; i<max; i++) if (!faceSel[i]) mFaceHide.Set(i,true);
		}
	} else {
		BitArray & vertSel = *GetCurrentSelection (MNM_SL_VERTEX, pMod->getParamBlock()->GetInt(epm_stack_selection)!=0);
		int max = vertSel.GetSize();
		if (mVertHide.GetSize()<max) max = mVertHide.GetSize();
		if (hideSelected) {
			for (int i=0; i<max; i++) {
				if (vertSel[i]) {
					if (!stackSel) vertSel.Clear(i);
					mVertHide.Set(i,true);
				}
			}
		} else {
			for (int i=0; i<max; i++) if (!vertSel[i]) mVertHide.Set(i,true);
		}
	}
}

void EditPolyData::Unhide (EditPolyMod *pMod, bool isFaceLevel)
{
	SynchBitArrays ();
	if (theHold.Holding()) theHold.Put (new EditPolyHideRestore (pMod, this, isFaceLevel));
	if (isFaceLevel) mFaceHide.ClearAll ();
	else mVertHide.ClearAll();
}

void EditPolyData::PushOperation (PolyOperation *pOp)
{
	PolyOperationRecord *pRec = new PolyOperationRecord (pOp);
	if (mpPolyOpData) {
		mpPolyOpData->SetCommitted (true);
		pRec->SetLocalData (mpPolyOpData);
		mpPolyOpData = NULL;
	}
	pRec->SetNext (NULL);

	if (mpOpList == NULL)
		mpOpList = pRec;
	else
	{
		// Otherwise, find the last item in the list, and append this one to the end.
		PolyOperationRecord* pOpRec = NULL;
		for (pOpRec=mpOpList; pOpRec->Next () != NULL; pOpRec=pOpRec->Next ());
		pOpRec->SetNext (pRec);
	}

	InvalidateTempData (PART_ALL);
}

PolyOperation *EditPolyData::PopOperation ()
{
	PolyOperationRecord *ret = NULL;
	if (mpOpList == NULL) return NULL;
	if (mpOpList->Next () == NULL)
	{
		ret = mpOpList;
		mpOpList = NULL;

		mpPolyOpData = ret->LocalData();
		return ret->Operation();
	}

	// Find the second-to-last item, and set its next to null.
	// (Do not delete - this is to be used by a restore object, so we'll retain the op there.)
	PolyOperationRecord* pOpRec = NULL;
	for (pOpRec=mpOpList; pOpRec->Next()->Next() != NULL; pOpRec=pOpRec->Next ());
	ret = pOpRec->Next();
	pOpRec->SetNext (NULL);
	mpPolyOpData = ret->LocalData();
	if (mpPolyOpData) mpPolyOpData->SetCommitted (false);

	InvalidateTempData (PART_ALL);

	return ret->Operation();
}

void EditPolyData::DeleteAllOperations()
{
	PolyOperationRecord *pOpRec, *pNext=NULL;
	for (pOpRec=mpOpList; pOpRec != NULL; pOpRec=pNext)
	{
		pNext = pOpRec->Next();
		pOpRec->DeleteThis ();
	}

	InvalidateTempData (PART_ALL);
}

void EditPolyData::ApplyAllOperations (MNMesh & mesh)
{
#ifdef __DEBUG_PRINT_EDIT_POLY
	DebugPrint ("EditPolyData::ApplyAllOperations\n");
#endif

	if (mpOpList) {
		// Preallocate if possible.  (Upon first application, this will do nothing.)
		PolyOperationRecord* pOpRec = NULL;
		int newFaces(0), newVertices(0), newEdges(0);
		Tab<int> newMapVertices;
		newMapVertices.SetCount (mesh.numm + NUM_HIDDENMAPS);
		for (int mp=-NUM_HIDDENMAPS; mp<mesh.numm; mp++) newMapVertices[mp+NUM_HIDDENMAPS] = 0;
		for (pOpRec=mpOpList; pOpRec != NULL; pOpRec=pOpRec->Next()) {
			newFaces += pOpRec->Operation()->NewFaceCount();
			newVertices += pOpRec->Operation()->NewVertexCount();
			newEdges += pOpRec->Operation()->NewEdgeCount ();

			for (int mp=-NUM_HIDDENMAPS; mp<mesh.numm; mp++) {
				if (mesh.M(mp)->GetFlag (MN_DEAD)) continue;
				newMapVertices[mp+NUM_HIDDENMAPS] += pOpRec->Operation()->NewMapVertexCount(mp);
			}
		}

		mesh.VAlloc (mesh.numv + newVertices);
		mesh.EAlloc (mesh.nume + newEdges);
		mesh.FAlloc (mesh.numf + newFaces);
		for (int mp=-NUM_HIDDENMAPS; mp<mesh.numm; mp++) {
			MNMap *map = mesh.M(mp);
			if (map->GetFlag (MN_DEAD)) continue;
			map->VAlloc (map->numv + newMapVertices[mp+NUM_HIDDENMAPS]);
			map->FAlloc (map->numf + newFaces);
		}

		for (pOpRec=mpOpList; pOpRec != NULL; pOpRec=pOpRec->Next())
		{
#ifdef __DEBUG_PRINT_EDIT_POLY
			DebugPrint ("EditPolyData::Applying %s\n", pOpRec->Operation()->Name());
#endif
			pOpRec->Operation()->SetUserFlags (mesh);
			bool ret = pOpRec->Operation()->Apply (mesh, pOpRec->LocalData());
			if (ret && pOpRec->Operation()->CanDelete()) mesh.CollapseDeadStructs ();
		}
	}
}

void EditPolyData::ApplyHide (MNMesh & mesh)
{
	int max = mVertHide.GetSize();
	if (max>mesh.numv) max = mesh.numv;
	for (int i=0; i<max; i++) mesh.v[i].SetFlag (MN_HIDDEN, mVertHide[i]!=0);
	max = mFaceHide.GetSize();
	if (max>mesh.numf) max = mesh.numf;
	for (int i=0; i<max; i++) mesh.f[i].SetFlag (MN_HIDDEN, mFaceHide[i]!=0);
}

void EditPolyData::RescaleWorldUnits (float f)
{
	PolyOperationRecord* pOpRec = NULL;
	for (pOpRec=mpOpList; pOpRec != NULL; pOpRec=pOpRec->Next())
	{
		pOpRec->Operation()->RescaleWorldUnits (f);
		if (pOpRec->LocalData()) pOpRec->LocalData()->RescaleWorldUnits(f);
	}
	InvalidateTempData (PART_ALL);
}

float *EditPolyData::GetSoftSelection (TimeValue t,
									   float falloff, float pinch, float bubble, int edist,
									   bool ignoreBackfacing, Interval & edistValid) {
	if (!GetMesh()) return NULL;
	if (GetMesh()->numv == 0) return NULL;

	if (!mpTemp) mpTemp = new MNTempData();
	mpTemp->SetMesh (GetMesh());
	int nv = GetMesh()->numv;
	mpTemp->InvalidateSoftSelection ();	// have to do, or it might remember last time's falloff, etc.

	if (!mVertexDistanceValidity.InInterval (t)) {
		mpTemp->InvalidateDistances ();
		mVertexDistanceValidity = mGeomValid;
		mVertexDistanceValidity &= mTopoValid;
		mVertexDistanceValidity &= edistValid;
	}

	// Question: Should we be using MN_SEL here, or MN_EDITPOLY_OP_SELECT based on our selection BitArrays?
	// Answer: As of this writing, this method is only called from EditPolyMod::ModifyObject,
	// where the MN_SEL flags are set to the appropriate selection, or from the Display code, which is based
	// on the cache set in ModifyObject.  So I think we're fine.
	return mpTemp->VSWeight (edist, edist, ignoreBackfacing,
		falloff, pinch, bubble, MN_SEL)->Addr(0);
}

MNTempData *EditPolyData::TempData () {
	if (!mpMesh) return NULL;
	if (!mpTemp) mpTemp = new MNTempData(mpMesh);
	return mpTemp;
}

MNTempData *EditPolyData::TempDataOutput () {
	if (!mpMeshOutput) return NULL;
	if (!mpTempOutput) mpTempOutput = new MNTempData (mpMeshOutput);
	return mpTempOutput;
}

void EditPolyData::InvalidateTempData (DWORD parts)
{
	// The paint mesh is extremely cheap to reconstruct, so we throw it away all the time:
	FreePaintMesh ();

	if (parts == PART_ALL)
	{
		if (mpTemp) {
			delete mpTemp;
			mpTemp = NULL;
		}
		if (mpTempOutput) {
			delete mpTempOutput;
			mpTempOutput = NULL;
		}
		return;
	}

	if (parts & PART_SELECT)
	{
		InvalidateSoftSelection ();
	}

	if (parts & (PART_GEOM|PART_TOPO))
	{
		InvalidateDistances ();
	}

	if (mpTemp) mpTemp->Invalidate (parts);
	if (mpTempOutput) mpTempOutput->Invalidate (parts);
}

void EditPolyData::InvalidateDistances ()
{
	mVertexDistanceValidity.SetEmpty ();
	if (mpTemp) mpTemp->InvalidateDistances ();
	if (mpTempOutput) mpTempOutput->InvalidateDistances ();
}

void EditPolyData::InvalidateSoftSelection ()
{
	if (mpTemp) mpTemp->InvalidateSoftSelection ();
	if (mpTempOutput) mpTempOutput->InvalidateSoftSelection ();
}

void EditPolyData::CollapseDeadSelections (EditPolyMod *pMod, MNMesh & mesh)
{
	BitArray delSet;

	// Fix the vertex selection and hide arrays:
	int max = mesh.numv;
	if (max>mVertSel.GetSize()) max = mVertSel.GetSize ();
	delSet.SetSize (max);
	for (int i=0; i<max; i++) delSet.Set (i, mesh.v[i].GetFlag (MN_DEAD));
	if (delSet.NumberSet()>0)
	{
		if (theHold.Holding()) theHold.Put (new EditPolySelectRestore (pMod, this, EPM_SL_VERTEX));
		mVertSel.DeleteSet (delSet);
		if (theHold.Holding()) theHold.Put (new EditPolyHideRestore (pMod, this, false));
		mVertHide.DeleteSet (delSet);
	}

	// Fix the edge selection array:
	max = mesh.nume;
	if (max>mEdgeSel.GetSize()) max = mEdgeSel.GetSize ();
	delSet.SetSize (max);
	for (int i=0; i<max; i++) delSet.Set (i, mesh.e[i].GetFlag (MN_DEAD));
	if (delSet.NumberSet()>0)
	{
		if (theHold.Holding()) theHold.Put (new EditPolySelectRestore (pMod, this, EPM_SL_EDGE));
		mEdgeSel.DeleteSet (delSet);
	}

	// Fix the face selection and hide arrays:
	max = mesh.numf;
	if (max>mFaceSel.GetSize()) max = mFaceSel.GetSize ();
	delSet.SetSize (max);
	for (int i=0; i<max; i++) delSet.Set (i, mesh.f[i].GetFlag (MN_DEAD));
	if (delSet.NumberSet()>0)
	{
		if (theHold.Holding()) theHold.Put (new EditPolySelectRestore (pMod, this, EPM_SL_FACE));
		mFaceSel.DeleteSet (delSet);
		if (theHold.Holding()) theHold.Put (new EditPolyHideRestore (pMod, this, true));
		mFaceHide.DeleteSet (delSet);
	}
}

void EditPolyData::GrowSelection (IMeshSelect *imod, int level) {
	DbgAssert (mpMesh);
	if( !mpMesh ) return;

	BitArray newSel;
	int mnSelLevel = meshSelLevel[level];
	DbgAssert (mpMesh->GetFlag (MN_MESH_FILLED_IN));
	if (!mpMesh->GetFlag (MN_MESH_FILLED_IN)) return;

	SynchBitArrays();

	int i;
	switch (mnSelLevel) {
	case MNM_SL_VERTEX:
		for (i=0; i<mpMesh->numv; i++) mpMesh->v[i].SetFlag (MN_USER, mVertSel[i]!=0);
		mpMesh->ClearEFlags (MN_USER);
		mpMesh->PropegateComponentFlags (MNM_SL_EDGE, MN_USER, MNM_SL_VERTEX, MN_USER);
		newSel.SetSize (mpMesh->numv);
		for (i=0; i<mpMesh->nume; i++) {
			if (mpMesh->e[i].GetFlag (MN_USER)) {
				newSel.Set (mpMesh->e[i].v1);
				newSel.Set (mpMesh->e[i].v2);
			}
		}
		SetVertSel (newSel, imod, TimeValue(0));
		break;
	case MNM_SL_EDGE:
		for (i=0; i<mpMesh->nume; i++) mpMesh->e[i].SetFlag (MN_USER, mEdgeSel[i]!=0);
		mpMesh->ClearVFlags (MN_USER);
		mpMesh->PropegateComponentFlags (MNM_SL_VERTEX, MN_USER, MNM_SL_EDGE, MN_USER);
		newSel.SetSize (mpMesh->nume);
		for (i=0; i<mpMesh->nume; i++) {
			if (mpMesh->v[mpMesh->e[i].v1].GetFlag (MN_USER)
				|| mpMesh->v[mpMesh->e[i].v2].GetFlag (MN_USER))
				newSel.Set (i);
		}
		SetEdgeSel (newSel, imod, TimeValue(0));
		break;
	case MNM_SL_FACE:
		for (i=0; i<mpMesh->numf; i++) mpMesh->f[i].SetFlag (MN_USER, mFaceSel[i]!=0);
		mpMesh->ClearVFlags (MN_USER);
		mpMesh->PropegateComponentFlags (MNM_SL_VERTEX, MN_USER, MNM_SL_FACE, MN_USER);
		newSel.SetSize (mpMesh->numf);
		for (i=0; i<mpMesh->numf; i++) {
         int j;
			for (j=0; j<mpMesh->f[i].deg; j++) {
				if (mpMesh->v[mpMesh->f[i].vtx[j]].GetFlag (MN_USER)) break;
			}
			if (j<mpMesh->f[i].deg) newSel.Set (i);
		}
		SetFaceSel (newSel, imod, TimeValue(0));
		break;
	}
}

void EditPolyData::ShrinkSelection (IMeshSelect *imod, int level) {
	DbgAssert (mpMesh);
	if( !mpMesh ) return;

	BitArray newSel;
	int mnSelLevel = meshSelLevel[level];
	DbgAssert (mpMesh->GetFlag (MN_MESH_FILLED_IN));
	if (!mpMesh->GetFlag (MN_MESH_FILLED_IN)) return;

	SynchBitArrays();

	switch (mnSelLevel) {
	case MNM_SL_VERTEX: 
		// Find the edges between two selected vertices.
		mpMesh->ClearEFlags (MN_USER);
		mpMesh->PropegateComponentFlags (MNM_SL_EDGE, MN_USER, mnSelLevel, MN_SEL, true);
		newSel = mVertSel;
		// De-select all the vertices touching edges to unselected vertices:
		for (int i=0; i<mpMesh->nume; i++) {
			if (!mpMesh->e[i].GetFlag (MN_USER)) {
				newSel.Clear (mpMesh->e[i].v1);
				newSel.Clear (mpMesh->e[i].v2);
			}
		}
		SetVertSel (newSel, imod, TimeValue(0));
		break;
	case MNM_SL_EDGE:
		// Find all vertices used by only selected edges:
		mpMesh->ClearVFlags (MN_USER);
		mpMesh->PropegateComponentFlags (MNM_SL_VERTEX, MN_USER, mnSelLevel, MN_SEL, true);
		newSel = mEdgeSel;
		for (int i=0; i<mpMesh->nume; i++) {
			// Deselect edges with at least one vertex touching a nonselected edge:
			if (!mpMesh->v[mpMesh->e[i].v1].GetFlag (MN_USER) || !mpMesh->v[mpMesh->e[i].v2].GetFlag (MN_USER))
				newSel.Clear (i);
		}
		SetEdgeSel (newSel, imod, TimeValue(0));
		break;
	case MNM_SL_FACE:
		// Find all vertices used by only selected faces:
		mpMesh->ClearVFlags (MN_USER);
		mpMesh->PropegateComponentFlags (MNM_SL_VERTEX, MN_USER, mnSelLevel, MN_SEL, true);
		newSel = mFaceSel;
		for (int i=0; i<mpMesh->numf; i++) {
			for (int j=0; j<mpMesh->f[i].deg; j++) {
				if (!mpMesh->v[mpMesh->f[i].vtx[j]].GetFlag (MN_USER)) 
            {
               // Deselect faces with at least one vertex touching a nonselected face:
               newSel.Clear (i);
               break;
            }
			}
		}
		SetFaceSel (newSel, imod, TimeValue(0));
		break;
	}
}

void EditPolyData::AcquireStackSelection (EditPolyMod *pMod, int selLevel) {
	if (selLevel == EPM_SL_OBJECT) return;	// shouldn't happen
	if (!GetMesh()) return;	// also shouldn't happen.

	int msl = meshSelLevel[selLevel];
	SetupNewSelection (msl);
	switch (msl) {
	case MNM_SL_VERTEX:
		GetMesh()->getVerticesByFlag (*mpNewSelection, MN_EDITPOLY_STACK_SELECT);
		break;
	case MNM_SL_EDGE:
		GetMesh()->getEdgesByFlag (*mpNewSelection, MN_EDITPOLY_STACK_SELECT);
		break;
	case MNM_SL_FACE:
		GetMesh()->getFacesByFlag (*mpNewSelection, MN_EDITPOLY_STACK_SELECT);
		break;
	}
	ApplyNewSelection (pMod);
}

void EditPolyData::SelectEdgeRing(IMeshSelect *imod) {
	DbgAssert (mpMesh);
	if( !mpMesh ) return;
	BitArray nsel = mEdgeSel;
	mpMesh->SelectEdgeRing (nsel);
	SetEdgeSel (nsel, imod, TimeValue(0));
}

void EditPolyData::SelectEdgeLoop(IMeshSelect *imod) {
	DbgAssert (mpMesh);
	if( !mpMesh ) return;
	BitArray nsel = mEdgeSel;
	mpMesh->SelectEdgeLoop (nsel);
	SetEdgeSel (nsel, imod, TimeValue(0));
}

void EditPolyData::SelectByMaterial (EditPolyMod *pMod) {
	if (pMod->GetMNSelLevel() != MNM_SL_FACE) return;
	if (!mpMesh) return;

	int clear = pMod->getParamBlock()->GetInt (epm_material_selby_clear);
	int matInt = pMod->getParamBlock()->GetInt (epm_material_selby);
	MtlID matID = (MtlID) matInt;

	SetupNewSelection (MNM_SL_FACE);
	BitArray *pSel = GetNewSelection ();
	for (int i=0; i<pSel->GetSize(); i++)
	{
		if (mpMesh->f[i].material == matID) pSel->Set(i);
	}

	ApplyNewSelection (pMod, !clear);
}

void EditPolyData::SelectBySmoothingGroup (EditPolyMod *pMod) {
	if (pMod->GetMNSelLevel() != MNM_SL_FACE) return;
	if (!mpMesh) return;

	int clear = pMod->getParamBlock()->GetInt (epm_smoother_selby_clear);
	int smgInt = pMod->getParamBlock()->GetInt (epm_smoother_selby);
	DWORD *smg = (DWORD *) ((void *)&smgInt);

	SetupNewSelection (MNM_SL_FACE);
	BitArray *pSel = GetNewSelection ();
	for (int i=0; i<pSel->GetSize(); i++)
	{
		if (mpMesh->f[i].smGroup & (*smg)) pSel->Set(i);
	}

	ApplyNewSelection (pMod, !clear);
}

class PolyOpDataRestore : public RestoreObj
{
private:
	int mOpID;
	EditPolyData *mpData;

public:
	PolyOpDataRestore (EditPolyData *pData, int opID)
		: mpData(pData), mOpID(opID) { }

	void Restore (int isUndo) { mpData->ClearPolyOpData (); }
	void Redo () { mpData->SetPolyOpData (mOpID); }
	int Size () { return 8; }
	TSTR Description() { return TSTR(_T("SetPolyOpData")); }
};

void EditPolyData::SetPolyOpData (int opID)
{
	if (mpPolyOpData && (mpPolyOpData->OpID() == opID)) return;

	if (mpPolyOpData) ClearPolyOpData ();

	mpPolyOpData = CreateLocalPolyOpData (opID);
	if (mpPolyOpData) {
		if (theHold.Holding()) theHold.Put (new PolyOpDataRestore (this, opID));
	}
}

LocalPolyOpData *EditPolyData::CreateLocalPolyOpData (int opID) {
	switch (opID) {
		case ep_op_create: return new LocalCreateData ();
		case ep_op_target_weld_vertex: return new LocalTargetWeldVertexData();
		case ep_op_target_weld_edge: return new LocalTargetWeldEdgeData();
		case ep_op_create_edge: return new LocalCreateEdgeData();
		case ep_op_edit_triangulation: return new LocalSetDiagonalData();
		case ep_op_cut: return new LocalCutData();
		case ep_op_insert_vertex_edge: return new LocalInsertVertexEdgeData();
		case ep_op_insert_vertex_face: return new LocalInsertVertexFaceData();
		case ep_op_attach: return new LocalAttachData();
		case ep_op_hinge_from_edge: return new LocalHingeFromEdgeData ();
		case ep_op_bridge_border: return new LocalBridgeBorderData();
		case ep_op_bridge_polygon: return new LocalBridgePolygonData ();
		case ep_op_turn_edge: return new LocalTurnEdgeData ();
		case ep_op_paint_deform: return new LocalPaintDeformData ();
		case ep_op_transform: return new LocalTransformData ();
		case ep_op_bridge_edge: return new LocalBridgeEdgeData ();
	}
	return NULL;
}

INode *EditPolyMod::EpModGetPrimaryNode ()
{
	if (!ip) return NULL;

	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);

	INode *ret = NULL;
	for (int i=0; i<list.Count(); i++)
	{
		if (list[i]->localData == NULL) continue;
		EditPolyData *pData = (EditPolyData *) list[i]->localData;
		if (pData->GetFlag (kEPDataPrimary))
		{
			ret = nodes[i];
			break;
		}
	}

	if (!ret) {
		// No primary? Ok, return first one we get to (and make it primary).
		for (int i=0; i<list.Count(); i++)
		{
			if (list[i]->localData == NULL) continue;
			EditPolyData *pData = (EditPolyData *) list[i]->localData;
			pData->SetFlag (kEPDataPrimary);
			ret = nodes[i];
			break;
		}
	}

	if (ret) ret = ret->GetActualINode ();
	nodes.DisposeTemporary();

	return ret;
}

EditPolyData *EditPolyMod::GetEditPolyDataForNode (INode *node) {
	if (node == NULL) return (EditPolyData *) GetPrimaryLocalModData();

	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);
	for (int i=0; i<list.Count(); i++)
	{
		if (nodes[i]->GetActualINode() != node) continue;
		EditPolyData *pData = (EditPolyData *) list[i]->localData;
		return pData;
	}

	return NULL;
}

LocalModData *EditPolyMod::GetPrimaryLocalModData ()
{
	if (!ip) return NULL;

	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);

	for (int i=0; i<list.Count(); i++)
	{
		if (list[i]->localData == NULL) continue;
		EditPolyData *pData = (EditPolyData *) list[i]->localData;
		if (pData->GetFlag (kEPDataPrimary)) return pData;
	}
	// No primary? Ok, return first mesh we get to (and make it primary).
	for (int i=0; i<list.Count(); i++)
	{
		if (list[i]->localData == NULL) continue;
		EditPolyData *pData = (EditPolyData *) list[i]->localData;
		pData->SetFlag (kEPDataPrimary);
		return pData;
	}

	return NULL;
}

void EditPolyMod::EpModSetPrimaryNode (INode *inode)
{
	if (!ip) return;

	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);

	for (int i=0; i<list.Count(); i++)
	{
		if (list[i]->localData == NULL) continue;
		EditPolyData *pdat = (EditPolyData *) list[i]->localData;
		if (!inode) inode = nodes[i]->GetActualINode();	// Pick the first one, if we're called with NULL.
		if (nodes[i]->GetActualINode() == inode) pdat->SetFlag (kEPDataPrimary);
		else pdat->ClearFlag (kEPDataPrimary);
	}
}

Matrix3 EditPolyMod::EpModGetNodeTM (TimeValue t, INode *node)
{
	if (node) return node->GetObjectTM (t);

	if (!ip) return Matrix3(true);

	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);

	for (int i=0; i<list.Count(); i++)
	{
		if (list[i]->localData == NULL) continue;
		EditPolyData *pData = (EditPolyData *) list[i]->localData;
		if (pData->GetFlag (kEPDataPrimary)) return nodes[i]->GetObjectTM (t);
	}

	// No primary? Ok, return first node we get to (and set it to primary):
	for (int i=0; i<list.Count(); i++)
	{
		if (list[i]->localData == NULL) continue;
		EditPolyData *pData = (EditPolyData *) list[i]->localData;
		pData->SetFlag (kEPDataPrimary);
		return nodes[i]->GetObjectTM (t);
	}

	return Matrix3(true);
}

void EditPolyMod::EpModListOperations (INode *pNode) {
	EditPolyData *pData = GetEditPolyDataForNode (pNode);
	if (!pData) return;
	bool macroRecEnabled1 = macroRecorder->Enabled()!=0;
	if (!macroRecEnabled1) macroRecorder->Enable();
	bool macroRecEnabled2 = macroRecorder->Enabled()!=0;
	if (!macroRecEnabled2) macroRecorder->Enable();

	macroRecorder->ScriptString (_T(""));
	macroRecorder->ScriptString (_T("-- Describing all Edit Poly Operations:"));
	for (PolyOperationRecord *pRec = pData->mpOpList; pRec != NULL; pRec = pRec->Next()) {
		pRec->Operation()->MacroRecord(pRec->LocalData());
	}
	macroRecorder->EmitScript ();
	if (!macroRecEnabled2) macroRecorder->Disable();
	if (!macroRecEnabled1) macroRecorder->Disable();
}

MNMesh *EditPolyMod::EpModGetMesh (INode *pNode) {
	EditPolyData *pData = GetEditPolyDataForNode (pNode);
	if (!pData) return NULL;
	return pData->GetMesh();
}

MNMesh *EditPolyMod::EpModGetOutputMesh (INode *pNode) {
	EditPolyData *pData = GetEditPolyDataForNode (pNode);
	if (!pData) return NULL;
	return pData->GetMeshOutput();
}




