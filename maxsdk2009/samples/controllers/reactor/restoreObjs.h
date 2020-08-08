#include "reactionMgr.h"

#ifndef __REACTIONRESTORE__H
#define __REACTIONRESTORE__H


class FullRestore: public RestoreObj {
	public:		
		Reactor *sav;
		Reactor *cur;
		Reactor *redo; 
		FullRestore() { sav = cur = redo =  NULL; }
		FullRestore(Reactor *cont) {
			cur = cont;
			//theHold.Suspend();
			//sav = (Control*)cont->Clone();
			//theHold.Resume();
			sav = new Reactor();
			*sav = *cur;
			redo = NULL;
			}
		~FullRestore() {
			if (sav) delete sav;
			if (redo) delete redo;
			}		
		
		void Restore(int isUndo) {
			assert(cur); assert(sav);
			if (isUndo) {
				//theHold.Suspend();
				//redo = (Control *)cur->Clone();
				//theHold.Resume();
				redo = new Reactor();
				*redo = *cur;
				}
			//cur->Copy(sav);
			*cur = *sav;
			}
		void Redo() {
			assert(cur); 
			if (redo) 
				//cur->Copy(redo);
				*cur = *redo;
			}
		void EndHold() {}
		TSTR Description() { return TSTR(_T("FullReactorRestore")); }
	};


// A restore object to save the influence, strength, and falloff.
class SpinnerRestore : public RestoreObj {
	public:		
		Reactor *cont;
		Tab<SlaveState> ureaction, rreaction;
		int uselected, rselected;
		UINT flags;

		SpinnerRestore(Reactor *c) {
			cont=c;
			//ureaction = cont->reaction;
			ureaction.SetCount(cont->slaveStates.Count());
			for(int i = 0;i < cont->slaveStates.Count();i++)
			{
				memset(&ureaction[i], 0, sizeof(SlaveState));
				ureaction[i] = cont->slaveStates[i];
			}


			uselected = cont->selected;
			flags = cont->flags;
		}
		void Restore(int isUndo) {
			// if we're undoing, save a redo state
			if (isUndo) {
				//rreaction = cont->reaction;
				rreaction.SetCount(cont->slaveStates.Count());
				for(int i = 0;i < cont->slaveStates.Count();i++)
				{
					memset(&rreaction[i], 0, sizeof(SlaveState));
					rreaction[i] = cont->slaveStates[i];
				}

				rselected = cont->selected;
			}
			//cont->reaction = ureaction;
			cont->slaveStates.SetCount(ureaction.Count());
			for(int i = 0;i < ureaction.Count();i++)
			{
				memset(&cont->slaveStates[i], 0, sizeof(SlaveState));
				cont->slaveStates[i] = ureaction[i];
			}
			cont->selected = uselected;
			if (isUndo && !(flags&REACTOR_BLOCK_CURVE_UPDATE)) {
				cont->RebuildFloatCurves();
			}
			cont->count = cont->slaveStates.Count();
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_REACTION_COUNT_CHANGED);
		}
		void Redo() {
			//cont->reaction = rreaction;
			cont->slaveStates.SetCount(rreaction.Count());
			for(int i = 0;i < rreaction.Count();i++)
			{
				memset(&cont->slaveStates[i], 0, sizeof(SlaveState));
				cont->slaveStates[i] = rreaction[i];
			}
			cont->selected = rselected;
			cont->count = cont->slaveStates.Count();
			if (!(flags&REACTOR_BLOCK_CURVE_UPDATE))
				cont->RebuildFloatCurves();
			cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_REACTION_COUNT_CHANGED);
		}
		void EndHold()
		{
		}
		int Size()
		{
			return sizeof(cont->slaveStates) + sizeof(float);
		}

};


class StateRestore : public RestoreObj {
	public:
		Reactor *cont;
		Point3 ucurpval, rcurpval;
		float ucurfval, rcurfval;
		Quat ucurqval, rcurqval;
		Tab<SlaveState> ureaction, rreaction;

		StateRestore(Reactor *c) 
		{
			cont = c;
			ucurpval = cont->curpval;
			ucurqval = cont->curqval;
			ucurfval = cont->curfval;
			//ureaction = cont->reaction;
			ureaction.SetCount(cont->slaveStates.Count());
			for(int i = 0;i < cont->slaveStates.Count();i++)
			{
				memset(&ureaction[i], 0, sizeof(SlaveState));
				ureaction[i] = cont->slaveStates[i];
			}

		}   		
		void Restore(int isUndo) 
			{
			if (isUndo)
			{
				rcurpval = cont->curpval;
				rcurqval = cont->curqval;
				rcurfval = cont->curfval;
				//rreaction = cont->reaction;
				rreaction.SetCount(cont->slaveStates.Count());
				for(int i = 0;i < cont->slaveStates.Count();i++)
				{
					memset(&rreaction[i], 0, sizeof(SlaveState));
					rreaction[i] = cont->slaveStates[i];
				}
			}
			cont->curpval = ucurpval;
			cont->curqval = ucurqval;
			cont->curfval = ucurfval;
			//cont->reaction = ureaction;
			cont->slaveStates.SetCount(ureaction.Count());
			for(int i = 0;i < ureaction.Count();i++)
			{
				memset(&cont->slaveStates[i], 0, sizeof(SlaveState));
				cont->slaveStates[i] = ureaction[i];
			}
			if (isUndo) cont->UpdateCurves();
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		void Redo()
			{
			cont->curpval = rcurpval;
			cont->curqval = rcurqval;
			cont->curfval = rcurfval;
			//cont->reaction = rreaction;
			cont->slaveStates.SetCount(rreaction.Count());
			for(int i = 0;i < rreaction.Count();i++)
			{
				memset(&cont->slaveStates[i], 0, sizeof(SlaveState));
				cont->slaveStates[i] = rreaction[i];
			}

			cont->UpdateCurves();
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}		
		void EndHold() 
			{ 
			cont->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T("Reactor State")); }
};

class RangeRestore : public RestoreObj {
	public:
		Reactor *cont;
		Interval ur, rr;
		RangeRestore(Reactor *c) 
			{
			cont = c;
			ur   = cont->range;
			}   		
		void Restore(int isUndo) 
			{
			rr = cont->range;
			cont->range = ur;
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		void Redo()
			{
			cont->range = rr;
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}		
		void EndHold() 
			{ 
			cont->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T("Reactor control range")); }
	};

class MasterStateRestore : public RestoreObj
{
	public:
		ReactionMaster *master;
		float fVal;
		Quat qVal;
		Point3 pVal;
		int index;

		MasterStateRestore(ReactionMaster *m, int i) 
		{
			master = m;
			index = i;
			fVal = m->GetState(i)->fvalue;
			pVal = m->GetState(i)->pvalue;
			qVal = m->GetState(i)->qvalue;
		}   		
		void Restore(int isUndo) 
			{
			float tempFVal = 0;
			Point3 tempPVal;
			Quat tempQVal;
			MasterState *state = master->GetState(index);
			if (isUndo)
			{
				tempFVal = state->fvalue;
				tempPVal = state->pvalue;
				tempQVal = state->qvalue;
			}

			state->fvalue = fVal;
			state->pvalue = pVal;
			state->qvalue = qVal;
			if (isUndo) 
			{
				fVal = tempFVal;
				pVal = tempPVal;
				qVal = tempQVal;
			}
			master->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		void Redo()
			{
				Restore(TRUE);
			}		
		TSTR Description() { return TSTR(_T("Master State")); }
};

class MasterStateListRestore : public RestoreObj
{
	public:
		ReactionMaster *master;
		Tab<MasterState*> states;

		MasterStateListRestore(ReactionMaster *m) 
		{
			master = m;
			for(int i = 0; i < master->states.Count(); i++)
			{
				MasterState* newState = master->states[i]->Clone();
				states.Append(1, &newState);
			}
		}   	

		~MasterStateListRestore() 
		{
			for(int i = 0; i < states.Count(); i++)
			{
				delete states[i];
			}
		}

		void Restore(int isUndo) 
		{
			Tab<MasterState*> tempStates;
			if (isUndo)
			{
				for(int i = 0; i < master->states.Count(); i++)
				{
					MasterState* newState = master->states[i]->Clone();
					tempStates.Append(1, &newState);
					delete master->states[i];
				}
			}

			master->states = states;

			if (isUndo) 
				states = tempStates;

			master->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			master->NotifyDependents(FOREVER,PART_ALL,REFMSG_REACTION_COUNT_CHANGED);
		}
		void Redo()
		{
			Restore(TRUE);
		}		
		TSTR Description() { return TSTR(_T("Master State List")); }
};


class MasterIDRestore : public RestoreObj
{
	public:
		Reactor* mReactor;
		int mID;

		MasterIDRestore(Reactor *r, int id) 
		{
			mReactor = r;
			mID = id;
		}   	

		~MasterIDRestore() 
		{
		}

		void Restore(int isUndo) 
		{
			for(int x = mReactor->slaveStates.Count()-1; x >= 0; x--)
			{
				if (mReactor->slaveStates[x].masterID >= mID)
					mReactor->slaveStates[x].masterID++;
			}
		}
		void Redo()
		{
			for(int x = mReactor->slaveStates.Count()-1; x >= 0; x--)
			{
				if (mReactor->slaveStates[x].masterID > mID)
					mReactor->slaveStates[x].masterID--;
			}
		}
		TSTR Description() { return TSTR(_T("Master IDs")); }
};

class DlgUpdateRestore : public RestoreObj
{
	public:

		DlgUpdateRestore() 
		{
		}   	

		~DlgUpdateRestore() 
		{
		}

		void Restore(int isUndo) 
		{
			if (GetReactionDlg())
				GetReactionDlg()->InvalidateReactionList();
		}
		void Redo()
		{
			Restore(FALSE);
		}
		TSTR Description() { return TSTR(_T("Update Reaction Dialog")); }
};

#endif // __REACTIONRESTORE__H
