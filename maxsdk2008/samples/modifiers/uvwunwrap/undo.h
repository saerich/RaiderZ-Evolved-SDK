//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: These are our undo classes
// AUTHOR: Peter Watje
// DATE: 2006/10/07 
//***************************************************************************/

#ifndef __UNDO__H
#define __UNDO__H

class UnwrapMod;
class MeshTopoData;

//*********************************************************
// Undo record for TV posiitons only
//*********************************************************

class TVertRestore : public RestoreObj {
	public:
		UnwrapMod *mod;
		MeshTopoData *ld;
		Tab<UVW_TVVertClass> undo, redo;
		BitArray uvsel, rvsel;
		BitArray uesel, resel;
		BitArray ufsel, rfsel;

		BOOL updateView;

		TVertRestore(UnwrapMod *m, MeshTopoData *ld, BOOL update = TRUE); 
		void Restore(int isUndo);
		void Redo();
		void EndHold();
		TSTR Description();
		
		
	};

//*********************************************************
// Undo record for TV posiitons and face topology
//*********************************************************


class TVertAndTFaceRestore : public RestoreObj {
	public:
		UnwrapMod *mod;

		MeshTopoData *ld;

		Tab<UVW_TVVertClass> undo, redo;
		Tab<UVW_TVFaceClass*> fundo, fredo;
		BitArray uvsel, rvsel;
		BitArray ufsel, rfsel;
		BitArray uesel, resel;

		BOOL update;
		bool mHidePeltDialog;

		TVertAndTFaceRestore(UnwrapMod *m, MeshTopoData *ld, bool hidePeltDialog = false);
		~TVertAndTFaceRestore() ;
		void Restore(int isUndo) ;
		void Redo();
		void EndHold();
		TSTR Description();
	};


class TSelRestore : public RestoreObj {
	public:
		UnwrapMod *mod;
		MeshTopoData *ld;
		BitArray undo, redo;
		BitArray fundo, fredo;
		BitArray eundo, eredo;

		BitArray geundo, geredo;
		BitArray gvundo, gvredo;

		TSelRestore(UnwrapMod *m,MeshTopoData *ld);
		void Restore(int isUndo);
		void Redo();
		void EndHold();
		TSTR Description();
	};	

class ResetRestore : public RestoreObj {
	public:
		UnwrapMod *mod;
		int uchan, rchan;


		ResetRestore(UnwrapMod *m);
		~ResetRestore();
		void Restore(int isUndo);
		void Redo();
		void EndHold();

		
		TSTR Description();
	};




class UnwrapPivotRestore : public RestoreObj {
public:
	Point3 upivot, rpivot;
	UnwrapMod *mod;

	UnwrapPivotRestore(UnwrapMod *m);
	
	void Restore(int isUndo);
	void Redo();
	int Size() { return 1; }
	void EndHold() {}
	TSTR Description() { return TSTR(_T(GetString(IDS_PW_PIVOTRESTORE))); 
	}
};

class UnwrapSeamAttributesRestore : public RestoreObj {
public:
	
	UnwrapMod *mod;
	BOOL uThick, uShowMapSeams, uShowPeltSeams, uReflatten;
	BOOL rThick, rShowMapSeams, rShowPeltSeams, rReflatten;

	UnwrapSeamAttributesRestore(UnwrapMod *m);
	
	void Restore(int isUndo);
	void Redo();
	int Size() { return 1; }
	void EndHold() {}
	TSTR Description() { return TSTR(_T(GetString(IDS_PW_PIVOTRESTORE))); 
	}
};


class UnwrapMapAttributesRestore : public RestoreObj {
public:
	
	UnwrapMod *mod;
	BOOL uPreview, uNormalize;
	int uAlign;
	
	BOOL rPreview, rNormalize;
	int rAlign;
	

	UnwrapMapAttributesRestore(UnwrapMod *m);
	
	void Restore(int isUndo);
	void Redo();
	int Size() { return 1; }
	void EndHold() {}
	TSTR Description() { return TSTR(_T(GetString(IDS_PW_PIVOTRESTORE))); 
	}
};



#endif // __UWNRAP__H
