/*===========================================================================*\
 | 
 |  FILE:	MCFunc.cpp
 |			Implimentations of the moprhChannel functions of MorpherMXS
 |			3D Studio MAX R3.0
 | 
 |  AUTH:   Harry Denholm
 |			Developer Consulting Group
 |			Copyright(c) Discreet 1999
 |
 |  HIST:	Started 5-4-99
 | 
\*===========================================================================*/

#include "MorpherMXS.h"


/*===========================================================================*\
 |	This swaps the position of 2 morphs in the channel list
 |  
\*===========================================================================*/
Value*
wm3_SwapMorph_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_SwapMorph_cf, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_SwapMorph [Morpher Modifier] [Morph Index] [MorphIndex]");
	type_check(arg_list[1], Integer, "WM3_SwapMorph [Morpher Modifier] [Morph Index] [MorphIndex]");
	type_check(arg_list[2], Integer, "WM3_SwapMorph [Morpher Modifier] [Morph Index] [MorphIndex]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;


	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	int morphIndexA = arg_list[1]->to_int(); 
	morphIndexA--;

	int morphIndexB = arg_list[2]->to_int(); 
	morphIndexB--;


	
	
	if ( (morphIndexA >= 0) && (morphIndexA < 100) &&
		 (morphIndexB >= 0) && (morphIndexB < 100))
	{		

		IMorphClass imp;
		imp.SwapMorphs(mp,morphIndexB,morphIndexA,TRUE);
	}	
	return &ok;	
}

/*===========================================================================*\
 |	This swaps the position of 2 morphs in the channel list
 |  
\*===========================================================================*/
Value*
wm3_MoveMorph_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_MoveMorph_cf, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_MoveMorph [Morpher Modifier] [Morph Source Index] [Morph Destination Index]");
	type_check(arg_list[1], Integer, "WM3_MoveMorph [Morpher Modifier] [Morph Source Index] [Morph Destination Index]");
	type_check(arg_list[2], Integer, "WM3_MoveMorph [Morpher Modifier] [Morph Source Index] [Morph Destination Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;


	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	int morphIndexA = arg_list[2]->to_int(); 
	morphIndexA--;

	int morphIndexB = arg_list[1]->to_int(); 
	morphIndexB--;


	
	
	if ( (morphIndexA >= 0) && (morphIndexA < 100) &&
		 (morphIndexB >= 0) && (morphIndexB < 100))
	{		
		IMorphClass imp;
		imp.SwapMorphs(mp,morphIndexA,morphIndexB,FALSE);
		mp->Update_channelFULL();

	}	
	return &ok;	
}

/*===========================================================================*\
 |	Returns the the  progressive morph tension
 |  
\*===========================================================================*/
Value*
wm3_GetProgressiveMorphTension_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_GetProgressiveMorphTension_cf, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_GetProgressiveMorphTension [Morpher Modifier] [Morph Index]");
	type_check(arg_list[1], Integer, "WM3_GetProgressiveMorphTension [Morpher Modifier] [Morph Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;


	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	int morphIndex = arg_list[1]->to_int(); 
	morphIndex--;


	
	
	INode *node = NULL;
	float tension = 0.0f;
	if ((morphIndex >= 0) && (morphIndex < 100))
	{		
			tension = mp->chanBank[morphIndex].mCurvature;			
		
	}	
	return Float::intern(tension);	
}


/*===========================================================================*\
 |	Sets the the  progressive morph tension
 |  
\*===========================================================================*/
Value*
wm3_SetProgressiveMorphTension_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_SetProgressiveMorphTension_cf, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_SetProgressiveMorphTension [Morpher Modifier] [Morph Index] [Tension]");
	type_check(arg_list[1], Integer, "WM3_SetProgressiveMorphTension [Morpher Modifier] [Morph Index] [Tension]");
	type_check(arg_list[2], Float, "WM3_SetProgressiveMorphTension [Morpher Modifier] [Morph Index] [Tension]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;


	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	int morphIndex = arg_list[1]->to_int(); 
	morphIndex--;

	float tension = arg_list[2]->to_float(); 

	
	
	INode *node = NULL;
	
	if ((morphIndex >= 0) && (morphIndex < 100))
	{		
		IMorphClass imp;
		imp.SetTension(mp,morphIndex,tension);
//		mp->chanBank[morphIndex].mCurvature = tension;					
	}	
	mp->Update_channelParams();
	return &ok;	
}

/*===========================================================================*\
 |	Returns the the  progressive morph Node
 |  
\*===========================================================================*/
Value*
wm3_GetProgressiveMorphNode_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_GetProgressiveMorphNode_cf, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_GetProgressiveMorphNode [Morpher Modifier] [Morph Index] [Progressive Morph Index]");
	type_check(arg_list[1], Integer, "WM3_GetProgressiveMorphNode [Morpher Modifier] [Morph Index]  [Progressive Morph Index]");
	type_check(arg_list[2], Integer, "WM3_GetProgressiveMorphNode [Morpher Modifier] [Morph Index]  [Progressive Morph Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;


	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	int morphIndex = arg_list[1]->to_int(); 
	morphIndex--;

	int pMorphIndex = arg_list[2]->to_int(); 
	pMorphIndex--;

	
	
	INode *node = NULL;
	
	if ((morphIndex >= 0) && (morphIndex < 100))
	{		
		int pMorphCount =  mp->chanBank[morphIndex].mNumProgressiveTargs + 1;		
		if ((pMorphIndex >= 0) && (pMorphIndex < pMorphCount))
		{
			int refnum = 0;
			if(pMorphIndex==0) refnum = 101+morphIndex;
				else refnum = 200+pMorphIndex+morphIndex*MAX_TARGS;

			node = (INode*)mp-> GetReference(refnum);
		}
	}	
	return MAXNode::intern(node);	
}

/*===========================================================================*\
 |	Adds a porgessive Morph
 |  
\*===========================================================================*/
Value*
wm3_AddProgressiveMorphNode_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_AddProgressiveMorphNode, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_AddProgressiveMorphNode [Morpher Modifier] [Morph Index] [Node]");
	type_check(arg_list[1], Integer, "WM3_AddProgressiveMorphNode [Morpher Modifier] [Morph Index]  [Node]");	
	type_check(arg_list[2], MAXNode, "WM3_AddProgressiveMorphNode [Morpher Modifier] [Morph Index]  [Node]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;


	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	int morphIndex = arg_list[1]->to_int(); 
	morphIndex--;


	INode *node = NULL;	
	node = arg_list[2]->to_node(); 
	
	
	if (node == NULL) return &ok;
	
	if ((morphIndex >= 0) && (morphIndex < 100))
	{		
		IMorphClass imp;
		int pInitMorphCount =  mp->chanBank[morphIndex].mNumProgressiveTargs;
		imp.AddProgessiveMorph(mp,morphIndex, node);
		int pNewMorphCount =  mp->chanBank[morphIndex].mNumProgressiveTargs;

		if (pNewMorphCount == pInitMorphCount)
			return &false_value;
		else return &ok;	
	}	
	return &false_value;
	
}

/*===========================================================================*\
 |	Deletes a porgessive Morph
 |  
\*===========================================================================*/
Value*
wm3_DeleteProgressiveMorphNode_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_DeleteProgressiveMorphNode, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_DeleteProgressiveMorphNode [Morpher Modifier] [Morph Index] [Progressive Morph Index]");
	type_check(arg_list[1], Integer, "WM3_DeleteProgressiveMorphNode [Morpher Modifier] [Morph Index] [Progressive Morph Index]")	
	type_check(arg_list[2], Integer, "WM3_DeleteProgressiveMorphNode [Morpher Modifier] [Morph Index] [Progressive Morph Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;


	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	int morphIndex = arg_list[1]->to_int(); 
	morphIndex--;

	int pMorphIndex = arg_list[2]->to_int(); 
	pMorphIndex--;


	
	if ((morphIndex >= 0) && (morphIndex < 100))
	{		
		IMorphClass imp;
		int pInitMorphCount =  mp->chanBank[morphIndex].mNumProgressiveTargs;
		imp.DeleteProgessiveMorph(mp,morphIndex, pMorphIndex);
		int pNewMorphCount =  mp->chanBank[morphIndex].mNumProgressiveTargs;

		if (pNewMorphCount == pInitMorphCount)
			return &false_value;
		else return &ok;	
	}	
	return &false_value;
	
}


/*===========================================================================*\
 |	Returns the the  progressive morph weight
 |  
\*===========================================================================*/
Value*
wm3_GetProgressiveMorphWeight_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_GetProgressiveMorphWeight_cf, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_GetProgressiveMorphWeight [Morpher Modifier] [Morph Index] [Progressive Node]");
	type_check(arg_list[1], Integer, "WM3_GetProgressiveMorphWeight [Morpher Modifier] [Morph Index]  [Progressive Node]");
	type_check(arg_list[2], MAXNode, "WM3_GetProgressiveMorphWeight [Morpher Modifier] [Morph Index]  [Progressive Node]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;


	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	int morphIndex = arg_list[1]->to_int(); 
	morphIndex--;

	INode *node = NULL;	
	node = arg_list[2]->to_node(); 

	float weight = 0.0f;

	if (node == NULL) return Float::intern(weight);	
	
	
	
	if ((morphIndex >= 0) && (morphIndex < 100))
	{		

		int pMorphIndex =  -1;
		int pMorphCount =  mp->chanBank[morphIndex].mNumProgressiveTargs + 1;		
		

		if (node == (INode*)mp->GetReference(101+morphIndex))
			pMorphIndex = 0;
		else
		{
			for (int i = 1; i < (pMorphCount); i++)
			{
				int refnum = 200+i+morphIndex*MAX_TARGS;
				if (node == (INode*)mp->GetReference(refnum))
					pMorphIndex = i ;
			}
		}

		if ((pMorphIndex >= 0) && (pMorphIndex < pMorphCount))
		{
			if (pMorphIndex == 0)
				weight = mp->chanBank[morphIndex].mTargetPercent;
			else weight = mp->chanBank[morphIndex].mTargetCache[pMorphIndex-1].mTargetPercent;
		}
	}	
	return Float::intern(weight);	
}


/*===========================================================================*\
 |	Returns the the  progressive morph weight
 |  
\*===========================================================================*/
Value*
wm3_SetProgressiveMorphWeight_cf(Value** arg_list, int count)
{
	check_arg_count(WM3_SetProgressiveMorphWeight_cf, 4, count);
	type_check(arg_list[0], MAXModifier, "WM3_SetProgressiveMorphWeight [Morpher Modifier] [Morph Index] [Progressive Node] [Weight]");
	type_check(arg_list[1], Integer, "WM3_SetProgressiveMorphWeight [Morpher Modifier] [Morph Index]  [Progressive Node] [Weight]");
	type_check(arg_list[2], MAXNode, "WM3_SetProgressiveMorphWeight [Morpher Modifier] [Morph Index]  [Progressive Node] [Weight]");
	type_check(arg_list[3], Float, "WM3_SetProgressiveMorphWeight [Morpher Modifier] [Morph Index]  [Progressive Node] [Weight]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;


	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	int morphIndex = arg_list[1]->to_int(); 
	morphIndex--;

	INode *node = NULL;	
	node = arg_list[2]->to_node(); 

//	int pMorphIndex = arg_list[2]->to_int(); 
//	pMorphIndex--;

	float weight = arg_list[3]->to_float(); 

	if (node == NULL) return &false_value;
	
	
	
	if ((morphIndex >= 0) && (morphIndex < 100))
	{		
		int pMorphCount =  mp->chanBank[morphIndex].mNumProgressiveTargs + 1;	

		int pMorphIndex = -1;
		if (node == (INode*)mp->GetReference(101+morphIndex))
			pMorphIndex = 0;
		else
		{
			for (int i = 1; i < (pMorphCount); i++)
			{
				int refnum = 200+i+morphIndex*MAX_TARGS;
				if (node == (INode*)mp->GetReference(refnum))
					pMorphIndex = i ;
			}
		}

		if ((pMorphIndex >= 0) && (pMorphIndex < pMorphCount))
		{
			IMorphClass imp;
			imp.HoldChannel(mp,morphIndex);
			if (pMorphIndex == 0)
				mp->chanBank[morphIndex].mTargetPercent = weight;
			else
			{
				mp->chanBank[morphIndex].mTargetCache[pMorphIndex-1].mTargetPercent = weight;
			}	
			imp.SortProgressiveTarget(mp, morphIndex, pMorphIndex);

		}
		else return &false_value;
	}	
	

	mp->Update_channelFULL();
	mp->Update_channelParams();

	return &ok;	
}


/*===========================================================================*\
 |	Returns the number of progressive morphs for a particular morph
 |  this includes the initial morph
\*===========================================================================*/
Value*
wm3_NumberOfProgressiveMorphs_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_NumberOfProgressiveMorphs_cf, 2, count);
	type_check(arg_list[0], MAXModifier, "wm3_NumberOfProgressiveMorphs [Morpher Modifier] [Morph Index]");
	type_check(arg_list[1], Integer, "wm3_NumberOfProgressiveMorphs [Morpher Modifier] [Morph Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;


	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	int morphIndex = arg_list[1]->to_int(); 
	morphIndex--;

	int ct = 0;
	if ((morphIndex >= 0) && (morphIndex < 100))
	{		
		if (mp->chanBank[morphIndex].mNumPoints == 0)
			ct = 0;
		else ct = mp->chanBank[morphIndex].mNumProgressiveTargs + 1;		
	}
	return Integer::intern(ct);
	
}

/*===========================================================================*\
 |	Returns the number of markers for the modifier
\*===========================================================================*/
Value*
wm3_NumberOfMarkers_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_NumberOfMarkers_cf, 1, count);
	type_check(arg_list[0], MAXModifier, "WM3_NumberOfMarkers [Morpher Modifier]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;


	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	int ct = mp->markerName.Count();

	return Integer::intern(ct);
}


/*===========================================================================*\
 |	Returns the current active marker.  This returns -1 if none are active
\*===========================================================================*/
Value*
wm3_GetCurrentMarker_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_NumberOfMarkers_cf, 1, count);
	type_check(arg_list[0], MAXModifier, "wm3_GetCurrentMarker [Morpher Modifier]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;


	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	int index = mp->markerSel;

	if (index >= 0) index++;

	return Integer::intern(index);
}



/*===========================================================================*\
 |	Lets you set the active marker
\*===========================================================================*/

Value*
wm3_SetCurrentMarker_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_NumberOfMarkers_cf, 2, count);
	type_check(arg_list[0], MAXModifier, "wm3_SetCurrentMarker [Morpher Modifier] [Marker Index]");
	type_check(arg_list[1], Integer, "wm3_SetCurrentMarker [Morpher Modifier] [Marker Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;


	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	int sel = arg_list[1]->to_int(); 
	sel -= 1;

	if ((sel >= 0) && (sel < mp->markerName.Count()))
	{
		IMorphClass imp;
		imp.HoldMarkers(mp);

		mp->chanSel = 0;
		mp->chanNum = (mp->markerIndex[sel]);
		mp->markerSel = sel;
		mp->Update_channelFULL();
		mp->Update_channelParams();
		return &true_value;
	}

	return &false_value;
}

/*===========================================================================*\
 |	Lets you create a marker marker
\*===========================================================================*/

Value*
wm3_CreateMarker_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_CreateMarker_cf, 3, count);
	type_check(arg_list[0], MAXModifier, "wm3_CreateMarker [Morpher Modifier] [Morph Index] [Marker Name]");
	type_check(arg_list[1], Integer, "wm3_CreateMarker [Morpher Modifier] [Morph Index] [Marker Name]");
	type_check(arg_list[2], String, "wm3_CreateMarker [Morpher Modifier] [Morph Index] [Marker Name]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;


	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	int sel = arg_list[1]->to_int(); 
	sel -= 1;

	TCHAR *name = arg_list[2]->to_string();

	if (sel >= 0) 
	{
		IMorphClass imp;
		imp.HoldMarkers(mp);

		mp->markerName.AddName(name);
		mp->markerIndex.Append(1,&sel,0);
		mp->Update_channelMarkers();
		return &true_value;
	}

	return &false_value;
}


/*===========================================================================*\
 |	Lets you delete a marker
\*===========================================================================*/

Value*
wm3_DeleteMarker_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_DeleteMarker_cf, 2, count);
	type_check(arg_list[0], MAXModifier, "wm3_DeletMarker [Morpher Modifier] [Morph Index]");
	type_check(arg_list[1], Integer, "wm3_DeletMarker [Morpher Modifier] [Morph Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;


	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	int sel = arg_list[1]->to_int(); 
	sel -= 1;



	if ( (sel >= 0) && (sel < mp->markerName.Count()))
	{
		IMorphClass imp;
		imp.HoldMarkers(mp);

		mp->markerName.Delete(sel,1);
		mp->markerIndex.Delete(sel,1);
		mp->Update_channelMarkers();
		return &true_value;
	}

	return &false_value;
}

/*===========================================================================*\
 |	Returns the morph index of a marker
\*===========================================================================*/
Value*
wm3_GetMarkerIndex_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_GetMarkerIndex_cf, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_GetMarkerIndex [Morpher Modifier] [Marker Index]");
	type_check(arg_list[1], Integer, "WM3_GetMarkerIndex [Morpher Modifier] [Marker Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;


	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	int index = arg_list[1]->to_int(); 
	index--;

	if ((index < 0) || (index >= mp->markerName.Count()))
		return &false_value;

	int id = mp->markerIndex[index];
	id++;

	return Integer::intern(id);
}


/*===========================================================================*\
 |	Returns the markers name
\*===========================================================================*/

Value*
wm3_GetMarkerName_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_GetMarkerName_cf, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_GetMarkerName [Morpher Modifier] [Marker Index]");
	type_check(arg_list[1], Integer, "WM3_GetMarkerName [Morpher Modifier] [Marker Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;


	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	int index = arg_list[1]->to_int(); 
	index--;

	if ((index < 0) || (index >= mp->markerName.Count()))
		return &false_value;


	return new String(mp->markerName[index]);
}

/*===========================================================================*\
 |	Given the <target object> from the scene, this function initializes the 
 |  channel with all necessary data
\*===========================================================================*/

Value*
wm3_mc_bfn_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_bfn, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_BuildFromNode [Morpher Modifier] [Channel Index] [Target]");
	type_check(arg_list[1], Integer, "WM3_MC_BuildFromNode [Morpher Modifier] [Channel Index] [Target]");
	type_check(arg_list[2], MAXNode, "WM3_MC_BuildFromNode [Morpher Modifier] [Channel Index] [Target]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	INode *node = arg_list[2]->to_node();
	mp->ReplaceReference(101+sel,node);
	mp->chanBank[sel].buildFromNode( node );

	return &true_value;
}

/*===========================================================================*\
 |	Lets you edit a marker 
\*===========================================================================*/

Value*
wm3_SetMarkerData_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_CreateMarker_cf, 4, count);
	type_check(arg_list[0], MAXModifier, "wm3_SetMarkerData [Morpher Modifier] [Marker Index] [Morph Index] [Marker Name]");
	type_check(arg_list[1], Integer, "wm3_SetMarkerData [Morpher Modifier] [Marker Index]  [Morph Index] [Marker Name]");
	type_check(arg_list[2], Integer, "wm3_SetMarkerData [Morpher Modifier] [Marker Index]  [Morph Index] [Marker Name]");
	type_check(arg_list[3], String, "wm3_SetMarkerData [Morpher Modifier] [Marker Index]  [Morph Index] [Marker Name]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;


	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	int markerIndex = arg_list[1]->to_int(); 
	markerIndex -= 1;

	int morphIndex = arg_list[2]->to_int(); 
	morphIndex -= 1;

	TCHAR *name = arg_list[3]->to_string();

	if ((markerIndex >= 0) && (markerIndex < mp->markerIndex.Count()))
	{
		mp->markerName.SetName(markerIndex,name);
		mp->markerIndex[markerIndex] = morphIndex;
		
		mp->Update_channelMarkers();
		mp->Update_channelFULL();
		mp->Update_channelParams();
		return &true_value;
	}

	return &false_value;
}


/*===========================================================================*\
 |	Rebuilds optimization and morph data in this channel
 |  Use this after changing the channel's target
\*===========================================================================*/

Value*
wm3_mc_rebuild_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_rebuild, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_Rebuild [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_Rebuild [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();
	mp->chanBank[sel].rebuildChannel();

	return &true_value;
}


/*===========================================================================*\
 |	Deletes the channel
\*===========================================================================*/

Value*
wm3_mc_delete_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_delete, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_Delete [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_Delete [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	if(mp->chanBank[sel].mConnection) 
		mp->DeleteReference(101+sel);

	// Ask channel to reset itself
	mp->chanBank[sel].ResetMe();

	// Reassign paramblock info
	ParamBlockDescID *channelParams = new ParamBlockDescID[1];

	ParamBlockDescID add;
	add.type=TYPE_FLOAT;
	add.user=NULL;
	add.animatable=TRUE;
	add.id=1;
	channelParams[0] = add;

	mp->ReplaceReference(1+sel, CreateParameterBlock(channelParams,1,1));	
	assert(mp->chanBank[sel].cblock);

	Control *c = (Control*)CreateInstance(CTRL_FLOAT_CLASS_ID,GetDefaultController(CTRL_FLOAT_CLASS_ID)->ClassID());

	mp->chanBank[sel].cblock->SetValue(0,0,0.0f);
	mp->chanBank[sel].cblock->SetController(0,c);

	delete channelParams;

	// Refresh system
	mp->Update_channelFULL();
	mp->Update_channelParams();	
	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_SUBANIM_STRUCTURE_CHANGED);

	needs_redraw_set();

	return &true_value;
}


/*===========================================================================*\
 |	Retrieves the name of the morpher channel
\*===========================================================================*/

Value*
wm3_mc_getname_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_getname, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_GetName [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_GetName [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	return new String(mp->chanBank[sel].mName);
}


/*===========================================================================*\
 |	Sets the name of the channel to be <string name>
\*===========================================================================*/

Value*
wm3_mc_setname_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_setname, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_SetName [Morpher Modifier] [Channel Index] [Name String]");
	type_check(arg_list[1], Integer, "WM3_MC_SetName [Morpher Modifier] [Channel Index] [Name String]");
	type_check(arg_list[2], String, "WM3_MC_SetName [Morpher Modifier] [Channel Index] [Name String]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();
	mp->chanBank[sel].mName = arg_list[2]->to_string();
	return &true_value;
}


/*===========================================================================*\
 |	Returns the weighting value of the channel
\*===========================================================================*/

Value*
wm3_mc_getamount_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_getamount, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_GetValue [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_GetValue [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	// The value of the channel - ie, its weighted percentage
	float tmpVal;
	mp->chanBank[sel].cblock->GetValue(0, MAXScript_time(), tmpVal, FOREVER);

	return Float::intern(tmpVal);
}


/*===========================================================================*\
 |	Sets the weighted value of the channel
\*===========================================================================*/

Value*
wm3_mc_setamount_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_setamount, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_SetValue [Morpher Modifier] [Channel Index] [Value]");
	type_check(arg_list[1], Integer, "WM3_MC_SetValue [Morpher Modifier] [Channel Index] [Value]");
	type_check(arg_list[2], Float, "WM3_MC_SetValue [Morpher Modifier] [Channel Index] [Value]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();
	mp->chanBank[sel].cblock->SetValue(0, MAXScript_time(), arg_list[2]->to_float());
	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

	needs_redraw_set();

	return &true_value;
}


/*===========================================================================*\
 |	Returns TRUE if the channel has an active connection to a scene object
\*===========================================================================*/

Value*
wm3_mc_hastarget_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_hastarget, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_HasTarget [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_HasTarget [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	if(mp->chanBank[sel].mConnection!=NULL) return &true_value;
	else return &false_value;
}


/*===========================================================================*\
 |	Returns a pointer to the object in the scene the channel is connected to
\*===========================================================================*/

Value*
wm3_mc_gettarget_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_gettarget, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_GetTarget [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_GetTarget [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	if(mp->chanBank[sel].mConnection!=NULL) return new MAXNode(mp->chanBank[sel].mConnection);
	else return &false_value;
}


/*===========================================================================*\
 |	Returns TRUE if the channel has not been marked as an invalid channel
\*===========================================================================*/

Value*
wm3_mc_isvalid_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_isvalid, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_IsValid [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_IsValid [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	if(!mp->chanBank[sel].mInvalid) return &true_value;
	else return &false_value;
}


/*===========================================================================*\
 |	Returns TRUE if the channel has some morpher data in it (Indicator: BLUE)
\*===========================================================================*/

Value*
wm3_mc_hasdata_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_hasdata, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_HasData [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_HasData [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	if(mp->chanBank[sel].mActive) return &true_value;
	else return &false_value;
}


/*===========================================================================*\
 |	Returns TRUE if the channel is turned on and used in the Morph
\*===========================================================================*/

Value*
wm3_mc_isactive_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_isactive, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_IsActive [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_IsActive [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	if(mp->chanBank[sel].mActiveOverride) return &true_value;
	return &false_value;
}



/*===========================================================================*\
 |	Sets wether or not the channel is used in the morph results or not
\*===========================================================================*/

Value*
wm3_mc_setactive_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_setactive, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_SetActive [Morpher Modifier] [Channel Index] [true/false]");
	type_check(arg_list[1], Integer, "WM3_MC_SetActive [Morpher Modifier] [Channel Index] [true/false]");
	type_check(arg_list[2], Boolean, "WM3_MC_SetActive [Morpher Modifier] [Channel Index] [true/false]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	mp->chanBank[sel].mActiveOverride = arg_list[2]->to_bool();
	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

	needs_redraw_set();

	return &true_value;
}



/*===========================================================================*\
 |	Returns TRUE if the 'Use Limits' checkbox is on
\*===========================================================================*/

Value*
wm3_mc_getuselimits_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_getuselimits, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_GetUseLimits [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_GetUseLimits [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	if(mp->chanBank[sel].mUseLimit) return &true_value;
	return &false_value;
}


/*===========================================================================*\
 |	Turns on and off the 'Use Limits' checkbox
\*===========================================================================*/

Value*
wm3_mc_setuselimits_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_setuselimits, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_SetUseLimits [Morpher Modifier] [Channel Index] [true/false]");
	type_check(arg_list[1], Integer, "WM3_MC_SetUseLimits [Morpher Modifier] [Channel Index] [true/false]");
	type_check(arg_list[2], Boolean, "WM3_MC_SetUseLimits [Morpher Modifier] [Channel Index] [true/false]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	mp->chanBank[sel].mUseLimit = arg_list[2]->to_bool();
	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

	needs_redraw_set();

	return &true_value;
}


/*===========================================================================*\
 |	Returns the upper limit for the channel values (only used if 'Use Limits' is on)
\*===========================================================================*/

Value*
wm3_mc_getlimitmax_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_getlimitmax, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_GetLimitMAX [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_GetLimitMAX [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	return Float::intern(mp->chanBank[sel].mSpinmax);
}


/*===========================================================================*\
 |	Returns the lower limit for the channel values (only used if 'Use Limits' is on)
\*===========================================================================*/

Value*
wm3_mc_getlimitmin_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_getlimitmin, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_GetLimitMIN [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_GetLimitMIN [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	return Float::intern(mp->chanBank[sel].mSpinmin);
}


/*===========================================================================*\
 |	Sets the high limit for the channel's value
\*===========================================================================*/

Value*
wm3_mc_setlimitmax_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_setlimitmax, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_SetLimitMAX [Morpher Modifier] [Channel Index] [Float Value]");
	type_check(arg_list[1], Integer, "WM3_MC_SetLimitMAX [Morpher Modifier] [Channel Index] [Float Value]");
	type_check(arg_list[2], Float, "WM3_MC_SetLimitMAX [Morpher Modifier] [Channel Index] [Float Value]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	mp->chanBank[sel].mSpinmax = arg_list[2]->to_float();
	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

	needs_redraw_set();

	return &true_value;
}


/*===========================================================================*\
 |	Sets the lower limit for the channel's value
\*===========================================================================*/

Value*
wm3_mc_setlimitmin_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_setlimitmin, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_SetLimitMIN [Morpher Modifier] [Channel Index] [Float Value]");
	type_check(arg_list[1], Integer, "WM3_MC_SetLimitMIN [Morpher Modifier] [Channel Index] [Float Value]");
	type_check(arg_list[2], Float, "WM3_MC_SetLimitMIN [Morpher Modifier] [Channel Index] [Float Value]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	mp->chanBank[sel].mSpinmin = arg_list[2]->to_float();
	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

	needs_redraw_set();

	return &true_value;
}


/*===========================================================================*\
 |	Returns TRUE if the 'Use Vertex Selection' button is on
\*===========================================================================*/

Value*
wm3_mc_getvertsel_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_getvertsel, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_GetUseVertexSel [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_GetUseVertexSel [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	if(mp->chanBank[sel].mUseSel) return &true_value;
	return &false_value;
}


/*===========================================================================*\
 |	Sets whether or not to use vertex selection in this channel
\*===========================================================================*/

Value*
wm3_mc_setvertsel_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_setvertsel, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_SetUseVertexSel [Morpher Modifier] [Channel Index] [true/false]");
	type_check(arg_list[1], Integer, "WM3_MC_SetUseVertexSel [Morpher Modifier] [Channel Index] [true/false]");
	type_check(arg_list[2], Boolean, "WM3_MC_SetUseVertexSel [Morpher Modifier] [Channel Index] [true/false]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	mp->chanBank[sel].mUseSel = arg_list[2]->to_bool();
	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

	needs_redraw_set();

	return &true_value;
}


/*===========================================================================*\
 |	Returns an estimation of how many bytes this channel takes up in memory
\*===========================================================================*/

Value*
wm3_mc_getmemuse_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_getmemuse, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_GetMemUse [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_GetMemUse [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	float tmSize = 0.0f;
	tmSize += mp->chanBank[sel].getMemSize();

	return Float::intern(tmSize);
}


/*===========================================================================*\
 |	The actual number of points in this channel
\*===========================================================================*/

Value*
wm3_mc_getnumpts_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_getnumpts, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_NumPts [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_NumPts [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	return Integer::intern(mp->chanBank[sel].mNumPoints);
}


/*===========================================================================*\
 |  The number of 'morphable points' in this channel
 |  'morphable points' are those that are different to the original object
\*===========================================================================*/

Value*
wm3_mc_getnummpts_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_getnummpts, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_NumMPts [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_NumMPts [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	return Integer::intern(static_cast<int>(mp->chanBank[sel].mPoints.size()));	// SR DCAST64: Downcast to 2G limit.
}


/*===========================================================================*\
 |	Gets a Point3 value of the <index> point in the channel
\*===========================================================================*/

Value*
wm3_mc_getmorphpoint_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_getmorphpoint, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_GetMorphPoint [Morpher Modifier] [Channel Index] [Index]");
	type_check(arg_list[1], Integer, "WM3_MC_GetMorphPoint [Morpher Modifier] [Channel Index] [Index]");
	type_check(arg_list[2], Integer, "WM3_MC_GetMorphPoint [Morpher Modifier] [Channel Index] [Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	return new Point3Value(mp->chanBank[sel].mPoints[arg_list[2]->to_int()]);
}


/*===========================================================================*\
 |	Gets a floating point value of the <index> weight in the channel
\*===========================================================================*/

Value*
wm3_mc_getmorphweight_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_getmorphweight, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_GetMorphWeight [Morpher Modifier] [Channel Index] [Index]");
	type_check(arg_list[1], Integer, "WM3_MC_GetMorphWeight [Morpher Modifier] [Channel Index] [Index]");
	type_check(arg_list[2], Integer, "WM3_MC_GetMorphWeight [Morpher Modifier] [Channel Index] [Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	return Float::intern((float)mp->chanBank[sel].mWeights[arg_list[2]->to_int()]);
}


/*===========================================================================*\
 |	Resets the internal object cache, forcing a complete rebuild.
\*===========================================================================*/

Value*
wm3_rebuildIMC_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_rebuildIMC, 1, count);
	type_check(arg_list[0], MAXModifier, "WM3_RebuildInternalCache [Morpher Modifier]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();
	mp->MC_Local.NukeCache();
	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

	needs_redraw_set();

	return &true_value;
}
