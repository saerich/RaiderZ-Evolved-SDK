/**********************************************************************
 *<
	FILE: EditPoly.cpp

	DESCRIPTION: Edit Poly Modifier

	CREATED BY: Steve Anderson, based on Face Extrude modifier by Berteig, and my own Poly Select modifier.

	HISTORY: created March 2002

 *>	Copyright (c) 2002 Discreet, All Rights Reserved.
 **********************************************************************/

#include "epoly.h"
#include "EditPoly.h"

// EditPolySelectRestore --------------------------------------------------

EditPolySelectRestore::EditPolySelectRestore(EditPolyMod *pMod, EditPolyData *pData) : mpMod(pMod), mpData(pData) {
	mLevel = mpMod->GetEPolySelLevel();
	mpData->SetFlag (kEPDataHeld);

	switch (mLevel) {
	case EPM_SL_OBJECT: DbgAssert(0); break;
	case EPM_SL_VERTEX:
		mUndoSel = mpData->mVertSel; break;
	case EPM_SL_EDGE:
	case EPM_SL_BORDER:
		mUndoSel = mpData->mEdgeSel;
		break;
	default:
		mUndoSel = mpData->mFaceSel;
		break;
	}
}

EditPolySelectRestore::EditPolySelectRestore(EditPolyMod *pMod, EditPolyData *pData, int sLevel) : mpMod(pMod), mpData(pData), mLevel(sLevel) {
	mpData->SetFlag (kEPDataHeld);

	switch (mLevel) {
	case EPM_SL_OBJECT:
		DbgAssert(0); break;
	case EPM_SL_VERTEX:
		mUndoSel = mpData->mVertSel; break;
	case EPM_SL_EDGE:
	case EPM_SL_BORDER:
		mUndoSel = mpData->mEdgeSel; break;
	default:
		mUndoSel = mpData->mFaceSel; break;
	}
}

void EditPolySelectRestore::After () {
	switch (mLevel) {			
	case EPM_SL_VERTEX:
		mRedoSel = mpData->mVertSel; break;
	case EPM_SL_EDGE:
	case EPM_SL_BORDER:
		mRedoSel = mpData->mEdgeSel; break;
	default:
		mRedoSel = mpData->mFaceSel; break;
	}
}

void EditPolySelectRestore::Restore(int isUndo) {
	if (isUndo && mRedoSel.GetSize() == 0) After ();
	switch (mLevel) {
	case EPM_SL_VERTEX:
		mpData->mVertSel = mUndoSel; break;
	case EPM_SL_EDGE:
	case EPM_SL_BORDER:
		mpData->mEdgeSel = mUndoSel; break;
	default:
		mpData->mFaceSel = mUndoSel; break;
	}
	mpMod->EpModLocalDataChanged (PART_SELECT);
}

void EditPolySelectRestore::Redo() {
	switch (mLevel) {		
	case EPM_SL_VERTEX:
		mpData->mVertSel = mRedoSel; break;
	case EPM_SL_EDGE:
	case EPM_SL_BORDER:
		mpData->mEdgeSel = mRedoSel; break;
	default:
		mpData->mFaceSel = mRedoSel; break;
	}
	mpMod->EpModLocalDataChanged (PART_SELECT);
}

EditPolyHideRestore::EditPolyHideRestore(EditPolyMod *pMod, EditPolyData *pData) : mpMod(pMod), mpData(pData) {
	mpData->SetFlag (kEPDataHeld);
	mFaceLevel = (mpMod->GetMNSelLevel() == MNM_SL_FACE);

	if (mFaceLevel) mUndoHide = mpData->mFaceHide;
	else mUndoHide = mpData->mVertHide;
}

EditPolyHideRestore::EditPolyHideRestore(EditPolyMod *pMod, EditPolyData *pData, bool isFaceLevel)
			: mpMod(pMod), mpData(pData), mFaceLevel(isFaceLevel) {
	mpData->SetFlag (kEPDataHeld);

	if (mFaceLevel) mUndoHide = mpData->mFaceHide;
	else mUndoHide = mpData->mVertHide;
}

void EditPolyHideRestore::After () {
	if (mFaceLevel) mRedoHide = mpData->mFaceHide;
	else mRedoHide = mpData->mVertHide;
}

void EditPolyHideRestore::Restore(int isUndo) {
	if (isUndo && mRedoHide.GetSize() == 0) After ();
	if (mFaceLevel) mpData->mFaceHide = mUndoHide;
	else mpData->mVertHide = mUndoHide;
	mpMod->EpModLocalDataChanged (PART_DISPLAY);
}

void EditPolyHideRestore::Redo() {
	if (mFaceLevel) mpData->mFaceHide = mRedoHide;
	else mpData->mVertHide = mRedoHide;
	mpMod->EpModLocalDataChanged (PART_DISPLAY);
}

EditPolyFlagRestore::EditPolyFlagRestore( EditPolyMod *epMod, EditPolyData *epData, int selLevel )
{
	mpEPMod = epMod;
	mpEPData = epData;
	mSelectionLevel = selLevel;
	int i;

	MNMesh * mesh = mpEPData->GetMesh();
	if ( mesh == NULL )
	{
		return;
	}

	switch (mSelectionLevel)
	{
		case MNM_SL_OBJECT:
		{
			return;
		}
		case MNM_SL_VERTEX:
		{		
			undo.SetCount( mesh->numv );
			for ( i = 0; i < undo.Count(); i++ )
			{
				undo[i] = mesh->v[i].ExportFlags ();
			}
			break;
		}
		case MNM_SL_EDGE:
		{
			undo.SetCount(mesh->nume);
			for ( i = 0; i < undo.Count(); i++ )
			{
				undo[i] = mesh->e[i].ExportFlags ();
			}
			break;
		}
		case MNM_SL_FACE:
		{
			undo.SetCount(mesh->numf);
			for ( i = 0; i < undo.Count(); i++ )
			{
				undo[i] = mesh->f[i].ExportFlags ();
			}
			break;
		}
	}
}

void EditPolyFlagRestore::Restore (int isUndo)
{
	int i, max = undo.Count ();

	MNMesh * mesh = mpEPData->GetMesh();
	if ( mesh == NULL )
	{
		return;
	}
	switch (mSelectionLevel)
	{
		case MNM_SL_OBJECT:
		{
			return;
		}
		case MNM_SL_VERTEX:
		{
			redo.SetCount (mesh->numv);
			for ( i = 0; i < redo.Count(); i++ )
			{
				redo[i] = mesh->v[i].ExportFlags();
				if ( i < max )
				{
					mesh->v[i].ImportFlags( undo[i] );
				}
			}
			break;
		}
		case MNM_SL_EDGE:
		{
			redo.SetCount (mesh->nume);
			for ( i = 0; i < redo.Count(); i++ )
			{
				redo[i] = mesh->e[i].ExportFlags ();
				if ( i < max )
				{
					mesh->e[i].ImportFlags( undo[i] );
				}
			}
			break;
		}
		case MNM_SL_FACE:
		{
			redo.SetCount(mesh->numf);
			for ( i = 0; i < redo.Count(); i++ )
			{
				redo[i] = mesh->f[i].ExportFlags ();
				if ( i < max )
				{
					mesh->f[i].ImportFlags( undo[i] );
				}
			}
			break;
		}
	}

	// We may have changed topology or selection ...
	mpEPMod->EpModLocalDataChanged( PART_TOPO|PART_SELECT|PART_GEOM );
	// sca - or geometry - such as bounding boxes.
}

void EditPolyFlagRestore::Redo ()
{
	int i, max = undo.Count ();

	MNMesh * mesh = mpEPData->GetMesh();
	if ( mesh == NULL )
	{
		return;
	}
	switch (mSelectionLevel)
	{
		case MNM_SL_OBJECT:
		{
			return;
		}
		case MNM_SL_VERTEX:
		{
			if ( max > mesh->numv)
			{
				max = mesh->numv;
			}
			for ( i = 0; i < max; i++ )
			{
				mesh->v[i].ImportFlags(redo[i]);
			}
			break;
		}
		case MNM_SL_EDGE:
		{
			if ( max > mesh->nume)
			{
				max = mesh->nume;
			}
			for ( i = 0; i < max; i++ )
			{
				mesh->e[i].ImportFlags(redo[i]);
			}
			break;
		}
		case MNM_SL_FACE:
		{
			if ( max > mesh->numf )
			{
				max = mesh->numf;
			}
			for ( i = 0; i < max; i++ )
			{
				mesh->f[i].ImportFlags(redo[i]);
			}
			break;
		}
	}

	// We may have changed topology or selection ...
	mpEPMod->EpModLocalDataChanged( PART_TOPO|PART_SELECT|PART_GEOM );
}
