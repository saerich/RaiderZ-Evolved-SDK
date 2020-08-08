#include "reactionMgr.h"
#include "custcont.h"

//Pick Mode Implementations
//*******************************************************

BOOL ReactionPickMode::Filter(INode *node)
{
	return FilterAnimatable( ((Animatable*)node) );
}

BOOL ReactionPickMode::FilterAnimatable(Animatable *anim)
{
	//if ( ((ReferenceTarget*)anim)->TestForLoop(FOREVER,(ReferenceMaker *) cont)!=REF_SUCCEED ) 
	//	return FALSE;
	if (anim->SuperClassID() == BASENODE_CLASS_ID && ((INode*)anim)->IsRootNode() ) 
		return FALSE;
	
	return anim->SuperClassID() == CTRL_FLOAT_CLASS_ID ||
		anim->SuperClassID() == CTRL_POSITION_CLASS_ID ||
		anim->SuperClassID() == CTRL_POINT3_CLASS_ID ||
		anim->SuperClassID() == CTRL_SCALE_CLASS_ID ||
		anim->SuperClassID() == CTRL_ROTATION_CLASS_ID || 
		anim->SuperClassID() == BASENODE_CLASS_ID; 
}

BOOL ReactionPickMode::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
{
	INode *node = ip->PickNode(hWnd,m);	
	if (!node) return false;
	return true;
}

BOOL ReactionPickMode::Pick(IObjParam *ip,ViewExp *vpt)
{
	ReferenceTarget *node = vpt->GetClosestHit();
	nodes.Append(1, &node);
	if (!this->MultipleChoice())
		theAnimPopupMenu.DoPopupMenu(this, nodes); 
	return TRUE;
}

BOOL ReactionPickMode::PickAnimatable(Animatable* anim)
{
	if ( FilterAnimatable((Animatable*)anim) )
	{
		theHold.Begin();
		NodePicked((ReferenceTarget*)anim);
		theHold.Accept(GetName());

		GetCOREInterface()->ClearPickMode();
		return TRUE;
	}
	return FALSE;
}

void ReactionPickMode::ExitMode(IObjParam *ip)
{
	if (nodes.Count() > 0)
	{
		if (MultipleChoice())
			theAnimPopupMenu.DoPopupMenu(this, nodes); 
		nodes.SetCount(0);
	}
}

void ReactionMasterPickMode::NodePicked(ReferenceTarget* anim)
{
	Animatable* parent = NULL;
	int subAnim = 0;

	if (anim->SuperClassID()==BASENODE_CLASS_ID)
	{
		INode* nd = (INode*)anim;
		parent = nd;

		Control *c = ((INode*)nd)->GetTMController();
		if (c->ClassID() == BIPSLAVE_CONTROL_CLASS_ID || c->ClassID() == IKSLAVE_CLASSID)
		{
			subAnim = IKTRACK;
		}else {
			subAnim = WSPOSITION;
		}
	} 
	else {
		MyEnumProc dep(anim);             
		anim->DoEnumDependents(&dep);

		for(int x=0; x<dep.anims.Count(); x++)
		{
			for(int i=0; i<dep.anims[x]->NumSubs(); i++)
			{
				Animatable* n = dep.anims[x]->SubAnim(i);
				if ((Control*)n == (Control*)anim)
				{
					parent = dep.anims[x];
					subAnim = i;
				}
			}
		}
	}
	if (parent != NULL)
		GetReactionManager()->AddReaction((ReferenceTarget*)parent, subAnim);
}

void ReplaceMasterPickMode::NodePicked(ReferenceTarget* anim)
{
	Animatable* parent = NULL;
	int subAnim = 0;

	if (anim->SuperClassID()==BASENODE_CLASS_ID)
	{
		INode* nd = (INode*)anim;
		parent = nd;

		Control *c = ((INode*)nd)->GetTMController();
		if (c->ClassID() == BIPSLAVE_CONTROL_CLASS_ID || c->ClassID() == IKSLAVE_CLASSID)
		{
			subAnim = IKTRACK;
		}else {
			subAnim = WSPOSITION;
		}
	} 
	else {
		MyEnumProc dep(anim);             
		anim->DoEnumDependents(&dep);

		for(int x=0; x<dep.anims.Count(); x++)
		{
			for(int i=0; i<dep.anims[x]->NumSubs(); i++)
			{
				Animatable* n = dep.anims[x]->SubAnim(i);
				if ((Control*)n == (Control*)anim)
				{
					parent = dep.anims[x];
					subAnim = i;
				}
			}
		}
	}
	if (parent != NULL)
	{
		mSet->ReplaceMaster((ReferenceTarget*)parent, subAnim);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	}
}

bool ReplaceMasterPickMode::LoopTest(ReferenceTarget* anim) 
{ 
	//for(int i = 0; i < mSet->SlaveCount(); i++)
	//{
	//	ReferenceTarget* slave = mSet->Slave(i);
	//	if (slave != NULL)
	//	{
	//		if (slave == anim)
	//			return false;

	//		if(anim->TestForLoop(FOREVER, slave )!=REF_SUCCEED)
	//			return false;
	//	}
	//}
	return true;
}

bool ReactionSlavePickMode::LoopTest(ReferenceTarget* anim) 
{ 
	// don't allow the owner or anthing below the owner.
	ReactionMaster* master = mMaster->GetReactionMaster();
	if (master != NULL)
	{
		ReferenceTarget* owner = master->Owner();
		if (owner != NULL)
			return anim != owner && owner->TestForLoop(FOREVER, anim)==REF_SUCCEED;
	}
	return true;
}

bool ReactionSlavePickMode::FilterAnimatable(Animatable *anim)
{
	if (anim) {
		Control* cont = GetControlInterface(anim);
		if (cont && (cont->NumSubs() == 0) && !cont->IsReplaceable())
			return false;
	}
	return true;
}

void ReactionSlavePickMode::NodePicked(ReferenceTarget* anim)
{
	// Last resort filtering out of irreplaceable controllers
	Control* cont = GetControlInterface(anim);
	if (cont && !cont->IsReplaceable())
		return;

	Animatable* parent = NULL;
	int subAnim = 0;

	if (anim->SuperClassID()!=BASENODE_CLASS_ID)
	{
		MyEnumProc dep(anim);             
		anim->DoEnumDependents(&dep);

		for(int x=0; x<dep.anims.Count(); x++)
		{
			for(int i=0; i<dep.anims[x]->NumSubs(); i++)
			{
				Animatable* n = dep.anims[x]->SubAnim(i);
				if ((Control*)n == (Control*)anim)
				{
					parent = dep.anims[x];
					subAnim = i;
					break;
				}
			}
		}
	}
	if (parent != NULL)
	{
		Reactor* reactor = NULL;

		switch (anim->SuperClassID())
		{
			case CTRL_POSITION_CLASS_ID:
				reactor = (Reactor*)GetPositionReactorDesc()->Create();
				break;
			case CTRL_POINT3_CLASS_ID:
				reactor = (Reactor*)GetPoint3ReactorDesc()->Create();
				break;
			case CTRL_ROTATION_CLASS_ID:
				reactor = (Reactor*)GetRotationReactorDesc()->Create();
				break;
			case CTRL_SCALE_CLASS_ID:
				reactor = (Reactor*)GetScaleReactorDesc()->Create();
				break;
			case CTRL_FLOAT_CLASS_ID:
				reactor = (Reactor*)GetFloatReactorDesc()->Create();
				break;
		}

		if (reactor != NULL)
		{
			Control* c = GetControlInterface(anim);
			if (c != NULL)
				reactor->Copy(c);
			parent->AssignController(reactor, subAnim);

			ReactionMaster* master = mMaster->GetReactionMaster();

			if (master != NULL)
			{
				if (mMasterIDs.Count() <= 0)
				{
					int defaultID = -1;
					this->mMaster->GetReactionMaster()->AddState(NULL, defaultID, true, GetCOREInterface()->GetTime());
					mMasterIDs.Append(1, &defaultID);
				}
				
				BOOL succeed = FALSE;
				if (master->Owner()->SuperClassID()==BASENODE_CLASS_ID)
					succeed = reactor->reactTo(master->Owner(), GetCOREInterface()->GetTime(), false);
				else
					succeed = reactor->reactTo((ReferenceTarget*)master->Owner()->SubAnim(master->SubIndex()), GetCOREInterface()->GetTime(), false);
				if (succeed)
				{
					for(int i = 0; i < mMasterIDs.Count(); i++)
						reactor->CreateReactionAndReturn(true, NULL, GetCOREInterface()->GetTime(), mMasterIDs[i]);
				}
			}
		}
	}
}

/// *************************************************************
/// Anim popup menu implementation


void AnimPopupMenu::add_hmenu_items(Tab<AnimEntry*>& wparams, HMENU menu, int& i)
{
	assert(menu != NULL);

	menus.Append(1, &menu);
	// build the menu
	while (i < wparams.Count())
	{
		AnimEntry *wp = wparams[i];
		i += 1;
		if (!wp->pair.Count()) continue;
		if (wp->level > 0)
		{
			// new submenu
			HMENU submenu = CreatePopupMenu();
			AppendMenu(menu, MF_POPUP, (UINT_PTR)submenu, wp->pname);
			add_hmenu_items(wparams, submenu, i);
		}
		else if (wp->level < 0)
			// end submenu
			return;
		else
			// add menu item
			AppendMenu(menu, MF_STRING, i, wp->pname);
	}
}

bool AnimPopupMenu::wireable(Animatable* parent, int subnum)
{
	Control* c;
	Animatable* anim = parent->SubAnim(subnum);
	if (anim != NULL && (c = GetControlInterface(anim)) != NULL )
		return true;
	else if (parent->SuperClassID() == PARAMETER_BLOCK_CLASS_ID)
	{
		IParamBlock* pb = (IParamBlock*)parent;
		if (pb->GetAnimParamControlType(subnum) != 0)
			return true;
	}
	else if (parent->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID)
	{
		IParamBlock2* pb = (IParamBlock2*)parent;
		if (pb->GetAnimParamControlType(subnum) != 0)
			return true;
	}
	return false;
}

bool AnimPopupMenu::Filter(
	PickObjectCallback* cb, 
	Animatable* anim, 
	Animatable* child, 
	bool includeChildren	// = false
	)
{
	bool valid = true;
	//ReferenceTarget* rt = (ReferenceTarget*)anim->GetInterface(REFERENCE_TARGET_INTERFACE); // not reliable
	ReferenceTarget* rt = (ReferenceTarget*)anim;
	if (rt == NULL)
		valid = false;
	else 
		valid = cb->LoopTest(rt);

	if (anim->SuperClassID() == BASENODE_CLASS_ID && ((INode*)anim)->IsRootNode() ) 
		valid = false;

	// In the case of slave picking, filter out invalid children like irreplaceable controllers 
	ReactionSlavePickMode* pickMode = dynamic_cast<ReactionSlavePickMode*>(cb);
	if (valid && pickMode && child) {
		if (!pickMode->FilterAnimatable(child))
			valid = false;
	}

	if (!includeChildren) 
		return valid;

	for (int i = 0; i< anim->NumSubs(); i++)
	{
		if (anim->SubAnim(i))
		{
			valid = Filter(cb, anim->SubAnim(i), NULL);
			if (valid) 
				return valid;
		}
	}
	return valid;
}

bool AnimPopupMenu::add_subanim_params(PickObjectCallback* cb, Tab<AnimEntry*>& wparams, TCHAR* name, Tab<ReferenceTarget*>& parents, int level, Tab<ReferenceTarget*>& roots)
{
	bool subAdded = false;
	Tab<TSTR> subAnimNames;
	subAnimNames.SetCount(0);

	// get the first parent
	if (parents.Count() > 0)
	{
		ReferenceTarget* parent = parents[0]; 
		// loop through its sub Anims
		for (int i = 0; i < parent->NumSubs(); i++)
		{
			//get the name and then check to make sure all other parents have the same subAnims.
			//order shouldn't be important
			TSTR subAnimName = parent->SubAnimName(i);
			Tab<SubAnimPair*> subAnimPairs;
			Tab<ReferenceTarget*> subs;
			subs.SetCount(0);
			subAnimPairs.SetCount(0);
			bool match = true;

			for (int p2 = 0; p2 < parents.Count(); p2++)
			{
				if (p2 != 0)
				{
					bool match2 = false;
					for(int i2 = 0; i2 < parents[p2]->NumSubs(); i2++)
					{
						if (!_tcscmp(subAnimName, parents[p2]->SubAnimName(i2)))// if there are no differences in the names
						{
							ReferenceTarget* s = (ReferenceTarget*)parents[p2]->SubAnim(i2);
							if (s != NULL)
							{
								if (Filter(cb, parents[p2], s))
								{
									match2 = true;
									subs.Append(1, &s);
									SubAnimPair* pair = new SubAnimPair();
									pair->parent = parents[p2];
									pair->id = i2;
									subAnimPairs.Append(1, &pair);
								}
							}
							break;
						}
					}
					if (!match2)
					{
						match = false;
						break;
					}
				}
				else
				{
					ReferenceTarget* s = (ReferenceTarget*)parent->SubAnim(i);
					if (s != NULL)
					{
						if (Filter(cb, parent, s))
						{
							subs.Append(1, &s);
							SubAnimPair* pair = new SubAnimPair();
							pair->parent = parent;
							pair->id = i;
							subAnimPairs.Append(1, &pair);
						}
					}
				}
			}
			if (match && subs.Count())
			{
				if (subs[0]->BypassTreeView() || subs[0]->BypassPropertyLevel())
				{	
					if (add_subanim_params(cb, wparams, NULL, subs, 0, roots))
						subAdded = true;
					for(int z = 0; z < subAnimPairs.Count(); z++)
						delete subAnimPairs[z];
				}
				else if (subs[0]->NumSubs() > 0 || subs[0]->GetCustAttribContainer() != NULL)
				{
					AnimEntry *wp = new AnimEntry(subAnimName, 1, subAnimPairs);
					wparams.Append(1, &wp);

					//add_subanim_params(cb, wparams, subAnimName, subs, 1, roots);
					if(!add_subanim_params(cb, wparams, subAnimName, subs, 1, roots))
						wparams.Delete(wparams.Count()-1, 1);
					else 
						subAdded = true;
				}
				else if (wireable(parent, i)) 
				{
					AnimEntry *wp = new AnimEntry(subAnimName, 0, subAnimPairs);
					wparams.Append(1, &wp);
					subAdded = true;
				}
			}
		}
	}
	//if(!cb->MultipleChoice())
	{
		// check for custom attributes
		ReferenceTarget* cc = (ReferenceTarget*)parents[0]->GetCustAttribContainer();
		if (cc != NULL)
		{
			Tab<ReferenceTarget*> ccs;

			ccs.Append(1, &cc);
			add_subanim_params(cb, wparams, NULL, ccs, 0, roots);
		}

		if (level == 1)
		{
			if (subAdded)
			{
				SubAnimPair* pair = new SubAnimPair();
				pair->parent = (ReferenceTarget*)parents[0];
				pair->id = 0;
				Tab<SubAnimPair*> subAnimPairs;
				subAnimPairs.Append(1, &pair);

				AnimEntry *wp = new AnimEntry(name, -1, subAnimPairs);
					wparams.Append(1, &wp);
			}
		}
	}

	return subAdded;
}

void AnimPopupMenu::build_params(PickObjectCallback* cb, Tab<AnimEntry*>& wparams, Tab<ReferenceTarget*> roots)
{
	// build the param list for given node
	for (int j = 0; j < wparams.Count(); j++)
		if (wparams[j]->pname != NULL)
			free(wparams[j]->pname);
	wparams.ZeroCount();

	//// params from node subanim tree
	if (cb->IncludeRoots())
	{
		for(int i = 0; i < roots.Count(); i++)
		{
			if (Filter(cb, roots[i], NULL))
			{
				SubAnimPair* p = new SubAnimPair();
				p->parent = roots[i];
				p->id = 0;
				Tab<SubAnimPair*> pairs;
				pairs.Append(1, &p);
				AnimEntry *wp = new AnimEntry(((INode*)roots[i])->GetName(), 0, pairs);
				wparams.Append(1, &wp);
			}
		}
	}
	add_subanim_params(cb, wparams, NULL, roots, 1, roots);
}

void AnimPopupMenu::DoPopupMenu(PickObjectCallback* cb, Tab<ReferenceTarget*> nodes)
{
	Interface* ip = GetCOREInterface();
	POINT mp;

	IPoint3 np;
	Point3 zero(0.0,0.0,0.0);

	GetCursorPos(&mp);
	ViewExp* view = ip->GetActiveViewport();
	HWND hwnd = view->GetHWnd();

	HMENU menu;

	// create menu
	if ((menu = CreatePopupMenu()) != NULL)
	{
		Tab<AnimEntry*> AnimEntries;		// currently-showing menu data

		// load source nodes param tree into wparams
		build_params(cb, AnimEntries, nodes);

		// build the popup menu
		int i = 0;
		add_hmenu_items(AnimEntries, menu, i);

		// pop the menu
		int id = TrackPopupMenu(menu, TPM_CENTERALIGN + TPM_NONOTIFY + TPM_RETURNCMD, mp.x, mp.y, 0, hwnd, NULL);
		// destroy the menus
		for (i = 0; i < menus.Count(); i++)
			DestroyMenu(menus[i]);
		menus.ZeroCount();
		if (id > 0)
		{
			theHold.Begin();
			for(i = 0; i < AnimEntries[id-1]->pair.Count(); i++)
			{
				SubAnimPair *pair = AnimEntries[id-1]->pair[i];
				if (!cb->IncludeRoots() || id > 1)
				{
					if (pair->parent->SubAnim(pair->id)->SuperClassID() == CTRL_USERTYPE_CLASS_ID )
					{
						if (!AssignDefaultController(pair->parent, pair->id))
							continue;
					}
					
					cb->NodePicked((ReferenceTarget*)pair->parent->SubAnim(pair->id));
				}
				else 
					if (id == 1) cb->NodePicked(pair->parent);
			}
			theHold.Accept(cb->GetName());
			cb->PostPick();
		}
		for(int z = 0; z < AnimEntries.Count(); z++)
		{
			delete AnimEntries[z];
		}
	}
}

Control* AnimPopupMenu::AssignDefaultController(Animatable* parent, int subnum)
{
	Control* ctrl = NULL;
	if (parent->SuperClassID() == PARAMETER_BLOCK_CLASS_ID)
	{
		IParamBlock* pb = (IParamBlock*)parent;
		if (pb->GetAnimParamControlType(subnum) == CTRL_FLOAT_CLASS_ID )
			ctrl = NewDefaultFloatController();
		else if (pb->GetAnimParamControlType(subnum) == CTRL_POINT3_CLASS_ID )
			ctrl = NewDefaultPoint3Controller();
	}
	else if (parent->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID)
	{
		IParamBlock2* pb = (IParamBlock2*)parent;
		if (pb->GetAnimParamControlType(subnum) == CTRL_FLOAT_CLASS_ID )
			ctrl = NewDefaultFloatController();
		else if (pb->GetAnimParamControlType(subnum) == CTRL_POINT3_CLASS_ID )
			ctrl = NewDefaultPoint3Controller();
	}
	
	if (ctrl != NULL) 
	{
		macroRecorder->Disable();
		parent->AssignController(ctrl, subnum);
		macroRecorder->Enable();
	}
	return ctrl;
}
