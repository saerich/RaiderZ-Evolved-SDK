/*==============================================================================
Copyright 2009 Autodesk, Inc.  All rights reserved. 

This computer source code and related instructions and comments are the unpublished
confidential and proprietary information of Autodesk, Inc. and are protected under 
applicable copyright and trade secret law.  They may not be disclosed to, copied
or used by any third party without the prior written consent of Autodesk, Inc.

//**************************************************************************/

#include "EPoly.h"
#include "EditPoly.h"
#include "maxscript\maxscript.h"
// InManpMode &&		BroadcastNotification(NOTIFY_MANIPULATE_MODE_OFF);



void EPManipulatorGrip::SetUpVisibility(BitArray &manipMask)
{
	if(manipMask.GetSize()== GetNumGripItems())	
	{
		for(int i=0;i< GetNumGripItems();++i)
		{
			if(manipMask[i])
				GetIGripManager()->HideGripItem(i, false);
			else
				GetIGripManager()->HideGripItem(i, true);

		}
	}
	//must reset all ui here
	GetIGripManager()->ResetAllUI();
}

//manip grip
//no okay, apply or cancel
void EPManipulatorGrip::Okay(TimeValue t)
{
}
void EPManipulatorGrip::Cancel()
{
}
void EPManipulatorGrip::Apply(TimeValue t)
{
}

int EPManipulatorGrip::GetNumGripItems()
{
	return 6;
}
IBaseGrip::Type EPManipulatorGrip::GetType(int which)
{
	switch(which)
	{
	case eFalloff:
		return IBaseGrip::eUniverse;
		break;
	case eBubble:
		return IBaseGrip::eFloat;
		break;
	case ePinch:
		return IBaseGrip::eFloat;
		break;
	case eSetFlow:
		return IBaseGrip::eInt;
		break;
	case eLoopShift:
		return  IBaseGrip::eInt;
		break;
	case eRingShift:
		return IBaseGrip::eInt;
		break;
	}
	return IBaseGrip::eInt;
}
void EPManipulatorGrip::GetGripName(TSTR &string)
{
	string = TSTR(GetString(IDS_MANIPULATE));
}
bool EPManipulatorGrip::GetText(int which,TSTR &string)
{
	switch(which)
	{
	case eFalloff:
		string = TSTR(GetString(IDS_FALLOFF));
		break;
	case eBubble:
		string = TSTR(GetString(IDS_BUBBLE));
		break;
	case ePinch:
		string = TSTR(GetString(IDS_PINCH));
		break;
	case eSetFlow:
		string = TSTR(GetString(IDS_SETFLOW));
		break;
	case eLoopShift:
		string = TSTR(GetString(IDS_LOOPSHIFT));
		break;
	case eRingShift:
		string = TSTR(GetString(IDS_RINGSHIFT));
		break;
	default:
		return false;
	};
	return true;
}
bool EPManipulatorGrip::GetResolvedIconName(int which,MSTR &string)
{
	switch(which)
	{
	case eFalloff:
		string = FALLOFF_PNG;
		break;
	case eBubble:
		string = BUBBLE_PNG;
		break;
	case ePinch:
		string = PINCH_PNG;
		break;
	case eSetFlow:
		string = SETFLOW_PNG;
		break;
	case eLoopShift:
		string = LOOPSHIFT_PNG;
		break;
	case eRingShift:
		string = RINGSHIFT_PNG;
		break;
	default:
		return false;
	};
	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

}


DWORD EPManipulatorGrip::GetCustomization(int which)
{
	DWORD word =0;
	switch(which)
	{
	case eBubble:
		word = (int)(IBaseGrip::eSameRow);
		break;
	case ePinch:
		word = (int)(IBaseGrip::eSameRow);
		break;
	case eSetFlow:
		word  = (int)(IBaseGrip::eTurnOffLabel) | (int) (IBaseGrip::eDisableAlt);
		break;
	case eLoopShift:
		word =  (int)(IBaseGrip::eTurnOffLabel) | (int) (IBaseGrip::eDisableAlt);
		break;
	case eRingShift:
		word = (int)(IBaseGrip::eTurnOffLabel) | (int) (IBaseGrip::eDisableAlt) | (int)(IBaseGrip::eSameRow);
		break;
	};

	return word;
}
bool EPManipulatorGrip::GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &radioOptions)
{
	return false;
}
bool EPManipulatorGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	ParamDef theDef;
	switch(which)
	{
	case eFalloff:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_ss_falloff, t, value.mFloat,FOREVER);
		break;
	case eBubble:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_ss_bubble, t, value.mFloat,FOREVER);
		break;
	case ePinch:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_ss_pinch, t, value.mFloat,FOREVER);
		break;
	case eSetFlow:
	case eLoopShift:
	case eRingShift:
		value.mInt = 0; //keep the value as zero for the spinner.
		break;
	default:
		return false;
	}
	return true;

}


bool EPManipulatorGrip::GetAutoScale(int which)
{
	switch(which)
	{
	case eFalloff:
	case eBubble:
	case ePinch:
		return true;
	case eSetFlow:
	case eLoopShift:
	case eRingShift:
		return false;
	}
	return true;
}
bool EPManipulatorGrip::GetScaleInViewSpace(int which, float &depth)
{
	if(which==eFalloff)
	{
		depth  = -200.0f;
		return true;
	}
	return false;
}
bool EPManipulatorGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
	{
	case eFalloff:
	case eBubble:
	case ePinch:
		scaleValue.mFloat = 1.0f; //uses autoscale
		break;
	case eSetFlow:
	case eLoopShift:
	case eRingShift:
		scaleValue.mInt = 1;
		break;
	default:
		return false;
	}
	return true;
}
//not used
bool EPManipulatorGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{
    // return false;
	ParamBlockDesc2* pbd = GetEditPolyMod()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eFalloff:
		{
			const ParamDef& pd = pbd->paramdefs[epm_ss_falloff];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eBubble:
		{
			const ParamDef& pd = pbd->paramdefs[epm_ss_bubble];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case ePinch:
		{
			const ParamDef& pd = pbd->paramdefs[epm_ss_pinch];
			resetValue.mFloat = pd.def.f;
		}
		break;
	default:
		return false;
	}
	return true;
}
bool EPManipulatorGrip::StartSetValue(int which,TimeValue t)
{
	ParamDef theDef;
	switch(which)
	{
	case eFalloff:
	case eBubble:
	case ePinch:
		theHold.Begin ();
		break;
	case eSetFlow:
		ExecuteMAXScriptScript(_T("PolyBoost.FlowSel = PolyBGetSel 2"), TRUE);//this is from PolyTools.ms, SetFlowSpinnerCallback
		ExecuteMAXScriptScript(_T("PolyBSetFlowSpinner PolyToolsUI.SetFlowAutoLoop 0 0 1"), TRUE);//this is from PolyTools.ms, SetFlowSpinnerCallback
		break;
	case eLoopShift:
	case eRingShift:
		{
			ModContextList	list;
			INodeTab		nodes;
			EditPolyData *l_polySelData = NULL ;

			mpEditPoly->ip->GetModContexts(list,nodes);

			for (int m = 0; m < list.Count(); m++) 
			{
				l_polySelData = (EditPolyData*)list[m]->localData;

				if (!l_polySelData) 
					continue;

				theHold.Begin();  // (uses restore object in paramblock.)
				theHold.Put (new SelRingLoopRestore(mpEditPoly,l_polySelData));

				if (which == eLoopShift)
					theHold.Accept(GetString(IDS_LOOP_EDGE_SEL));
				else 
					theHold.Accept(GetString(IDS_RING_EDGE_SEL));

			}		
			bool alt =  (GetKeyState(VK_MENU)&0x8000) ?  true : false; 
			bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) ? true : false; 
			if(ctrl)
			{
				GetEditPolyMod()->setAltKey(false);
				GetEditPolyMod()->setCtrlKey(true);
			}
			else if(alt)
			{
				GetEditPolyMod()->setCtrlKey(false );
				GetEditPolyMod()->setAltKey(true);
			}
		}
		break;
	default:
		return false;
	}
	return true;
}

bool EPManipulatorGrip::SetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	ParamDef theDef;
	switch(which)
	{
	case eFalloff:
		mpEditPoly->getParamBlock()->SetValue (epm_ss_falloff, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_ss_falloff, t, value.mFloat,FOREVER);
		break;
	case eBubble:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_ss_bubble, t, value.mFloat);
		break;
	case ePinch:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_ss_pinch, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_ss_pinch, t, value.mFloat,FOREVER);
		break;
	case eSetFlow:
		if(value.mInt < 0) //less than zero decrease
		{
			ExecuteMAXScriptScript(_T("PolyBSetFlowSpinner false -1 PolyToolsUI.SetFlowSpeed 0"), TRUE);//this is from PolyTools.ms, SetFlowSpinnerCallback
		}
		else
		{
			ExecuteMAXScriptScript(_T("PolyBSetFlowSpinner false 1 PolyToolsUI.SetFlowSpeed 0"), TRUE);//this is from PolyTools.ms, SetFlowSpinnerCallback
		}
		value.mInt = 0; //keep the value as zero for the spinner.
		break;
	case eLoopShift:
		{
			int l_spinVal =  GetEditPolyMod()->getLoopValue();
			if(value.mInt>0)
			{
				++l_spinVal;
			}
			else
			{
				--l_spinVal;
			}
			GetEditPolyMod()->UpdateLoopEdgeSelection(l_spinVal);
			value.mInt = 0;
		}
		break;
	case eRingShift:
		{
			int l_spinVal =  GetEditPolyMod()->getRingValue();
			if(value.mInt>0)
			{
				++l_spinVal;
			}
			else
			{
				--l_spinVal;
			}
			GetEditPolyMod()->UpdateRingEdgeSelection(l_spinVal);
			value.mInt = 0;
		}
		break;
	default:
		return false;
	}
	return true;
}


bool EPManipulatorGrip::EndSetValue(int which,TimeValue t,bool accepted)
{
	ParamDef theDef;
	switch(which)
	{
	case eFalloff:
		if(accepted)
			theHold.Accept (TSTR(GetString(IDS_FALLOFF)));
		else
			theHold.Cancel();
		GetEditPolyMod()->EpModRefreshScreen ();	
		break;
	case eBubble:
		if(accepted)
			theHold.Accept (TSTR(GetString(IDS_BUBBLE)));
		else
			theHold.Cancel();
		GetEditPolyMod()->EpModRefreshScreen ();	
		break;
	case ePinch:
		if(accepted)
			theHold.Accept (TSTR(GetString(IDS_PINCH)));
		else 
			theHold.Cancel();
		GetEditPolyMod()->EpModRefreshScreen ();	
		break;
	case eSetFlow:
		ExecuteMAXScriptScript(_T("PolyBAdjustUndo PolyBoost.FlowSel"), TRUE);//this is from PolyTools.ms, SetFlowSpinnerCallback
		break;
	case eLoopShift:
	case eRingShift:	
		// reset keys
		GetEditPolyMod()->setAltKey(false);
		GetEditPolyMod()->setCtrlKey(false );
		GetEditPolyMod()->EpModRefreshScreen ();	
		GetEditPolyMod()->EpModLocalDataChanged (PART_SELECT);
		break;
	default:
		return false;
	}
	return true;
}

//not used, 
bool EPManipulatorGrip::ResetValue(TimeValue t,int which)
{
	float fResetValue =0.0f;
	
	switch(which)
	{
	case eFalloff:
		theHold.Begin ();
		mpEditPoly->getParamBlock()->SetValue (epm_ss_falloff, t, fResetValue);
		theHold.Accept (GetString(IDS_FALLOFF));
		break;
	case eBubble:
		theHold.Begin();
		GetEditPolyMod()->getParamBlock()->SetValue (epm_ss_bubble, t,fResetValue);
		theHold.Accept (GetString(IDS_BUBBLE));
		break;
	case ePinch:
		theHold.Begin();
		GetEditPolyMod()->getParamBlock()->SetValue (epm_ss_pinch, t, fResetValue);
		theHold.Accept (GetString(IDS_PINCH));
		break;
	case eLoopShift:
	case eRingShift:
		//do something?
		break;
	default:
		return false;
	}
	return true;

}

bool EPManipulatorGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{
	ParamDef theDef;
	switch(which)
	{
	case eFalloff:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_ss_falloff);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;
	case eBubble:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_ss_bubble);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;
	case ePinch:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_ss_pinch);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;
	case eSetFlow:
		minRange.mInt = -1000000.0; //this is from ModelingRibbon.xaml, doesn't really matter
		maxRange.mInt = 1000000.0;
		break;
	case eLoopShift:
		minRange.mInt =  -9999999; 
		maxRange.mInt = 9999999;
		break;
	case eRingShift:
		minRange.mInt =  -9999999;
		maxRange.mInt = 9999999;
		break;
	default:
		return false;
	}
	return true;
}

bool EPManipulatorGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eFalloff:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_ss_falloff, t) == TRUE ? true : false;
	case eBubble:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_ss_bubble,  t) == TRUE ? true : false;
	case ePinch:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_ss_pinch,  t) == TRUE ? true : false;
	}
	return false;
}


BevelGrip::~BevelGrip()
{

}

int BevelGrip::GetNumGripItems()
{
	return 3; //would be 5 for the old one since the old inluded okay and apply (remember cancel was implicit)
}
IBaseGrip::Type BevelGrip::GetType(int which)
{
	DbgAssert(which>=0&&which<=2);
	switch(which)
	{
	case eBevelType:
		return IBaseGrip::eCombo;
	case eBevelHeight:
		return IBaseGrip::eUniverse;
	case eBevelOutline:
		return IBaseGrip::eUniverse;
	}
	return IBaseGrip::eInt;
}

void BevelGrip::GetGripName(TSTR &string)
{
	string =TSTR(GetString(IDS_BEVEL));
}

bool BevelGrip::GetText(int which,TSTR &string)
{
	switch(which)
	{
	case eBevelType:
		{
			//Group Normals, 1 for Local Normals, 2 for By Polygon 
			int type;
			GetEditPolyMod()->getParamBlock()->GetValue (epm_bevel_type, 0, type,FOREVER);
			switch(type)
			{
				case 0:
					string = /*TSTR(GetString(IDS_BEVEL_TYPE)) + TSTR(": ") +*/ TSTR(GetString(IDS_GROUP_NORMALS));
					break;
				case 1:
					string = /*TSTR(GetString(IDS_BEVEL_TYPE));// + TSTR(": ") +*/ TSTR(GetString(IDS_LOCAL_NORMALS));
					break;
				case 2:
					string = /*TSTR(GetString(IDS_BEVEL_TYPE));// + TSTR(": ") +*/ TSTR(GetString(IDS_BY_POLYGON));
					break;
	
			}
			break;
		}
	case eBevelHeight:
		{
			string =TSTR(GetString(IDS_HEIGHT));// + TSTR(": ") + TSTR(str);
		}
		break;
	case eBevelOutline:
		{
			string =TSTR(GetString(IDS_OUTLINE));//+ TSTR(": ") + TSTR(str);
		}
		break;
	default:
		return false;

	}
	return true;
}

DWORD BevelGrip::GetCustomization(int which)
{
	return 0;
}

bool BevelGrip::GetResolvedIconName(int which,MSTR &string)
{
	DbgAssert(which>=0&&which<=4);
	switch(which)
	{
	case eBevelType:
			string = BEVEL_TYPE_PNG;
			break;
	case eBevelHeight:
			string =  BEVEL_HEIGHT_PNG;
		break;
	case eBevelOutline:
			string =  BEVEL_OUTLINE_PNG;
		break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;
	
}

bool BevelGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eBevelType:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_bevel_type, t) == TRUE ? true : false;
	case eBevelHeight:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_bevel_height,  t) == TRUE ? true : false;
	case eBevelOutline:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_bevel_outline,  t) == TRUE ? true : false;
	}
	return false;
}

bool BevelGrip::GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &comboOptions)
{
	if(which== eBevelType)
	{
		BOOL found;
		IBaseGrip::ComboLabel *label = new IBaseGrip::ComboLabel();
		TSTR string = BEVEL_GROUP_PNG;
		found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
		if(found)
			label->mIcon = string;
		label->mLabel = TSTR(GetString(IDS_GROUP_NORMALS));
		comboOptions.Append(1,&label);
		label = new IBaseGrip::ComboLabel();
		string = BEVEL_NORMAL_PNG;
		found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
		if(found)
			label->mIcon = string;
		label->mLabel = TSTR(GetString(IDS_LOCAL_NORMALS));
		comboOptions.Append(1,&label);
		label = new IBaseGrip::ComboLabel();
		string = BEVEL_POLYGON_PNG;
		found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
		if(found)
			label->mIcon = string;
		label->mLabel = TSTR(GetString(IDS_BY_POLYGON));
		comboOptions.Append(1,&label);
		return true;
	}
	return false;
}

bool BevelGrip::GetCommandIcon(int which, MSTR &string)
{
	return false;
}


bool BevelGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{

	ParamDef theDef;
	switch(which)
	{
	case eBevelHeight:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_bevel_height);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eBevelOutline:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_bevel_outline);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;
	default:
		return false;

	}
	return true;
}

bool BevelGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eBevelType:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bevel_type, t, value.mInt,FOREVER);
		break;
	case eBevelHeight:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bevel_height, t, value.mFloat,FOREVER);
		break;
	case eBevelOutline:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bevel_outline, t, value.mFloat,FOREVER);
		break;
	default:
		return false;
	}
	return true;
}

bool BevelGrip::SetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eBevelType:
		if(value.mInt>2) //go back to zero
			value.mInt =0;
		if(value.mInt<0)
			value.mInt = 2;
		GetEditPolyMod()->getParamBlock()->SetValue (epm_bevel_type, t, value.mInt);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bevel_type, t, value.mInt,FOREVER);
		break;
	case eBevelHeight:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_bevel_height, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bevel_height, t, value.mFloat,FOREVER);
		break;
	case eBevelOutline:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_bevel_outline, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bevel_outline, t, value.mFloat,FOREVER);
		break;
	default:
		return false;
		
	}
	return true;
}

void BevelGrip::Apply(TimeValue t)
{
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitAndRepeat (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);
	
}

void BevelGrip::Okay(TimeValue t)
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitUnlessAnimating (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void BevelGrip::Cancel()
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,0);
	GetEditPolyMod()->EpModCancel();
	EndSetValue(-1,0,true);
	GetEditPolyMod()->EpModRefreshScreen ();
}

bool BevelGrip::StartSetValue(int ,TimeValue t)
{
	theHold.Begin ();
/*	int type = GetEditPolyMod()->GetPolyOperationID ();
	if (type != ep_op_bevel) {
		GetEditPolyMod()->EpModSetOperation (ep_op_bevel);
	} 
	*/
	return true;
}

bool BevelGrip::EndSetValue(int ,TimeValue t,bool accept)
{
	if (accept)
	{
		if(theHold.Holding())
			theHold.Accept (GetString(IDS_BEVEL));
	}
	else
	{
		if(theHold.Holding())
			theHold.Cancel();
		GetEditPolyMod()->EpModRefreshScreen ();	

	}
	return true;
}



//we don't auto scale
bool BevelGrip::GetAutoScale(int which)
{
	return true; //yes
}
//default scale
bool BevelGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
	{
	case eBevelHeight:
		scaleValue.mFloat = 1.0f;
		break;
	case eBevelOutline:
		scaleValue.mFloat = 1.0f;
		break;
	default:
		return false;
	}
	return true;
}
bool BevelGrip::GetScaleInViewSpace(int which, float &depth)
{
	depth = -200.0f; 
	switch(which)
	{
	case eBevelHeight:
	case eBevelOutline:
		return true;
	}
	
	return false;
}
bool BevelGrip::ResetValue(TimeValue t, int which)
{
	float fResetVal =0.0f;
	int  iResetVal = 0;

	switch(which)
	{
	case eBevelType:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_bevel_type, t, iResetVal);
	        GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_bevel_type,t, iResetVal, FOREVER);
			theHold.Accept (GetString(IDS_BEVEL));
		}
			break;
	case eBevelHeight:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_bevel_height, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_bevel_height,t,fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_BEVEL));
		}
		break;
	case eBevelOutline:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_bevel_outline, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_bevel_outline,t, fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_BEVEL));

		}
		break;
	default:
		return false;
	}
	return true;

}

bool BevelGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{
	ParamBlockDesc2* pbd = GetEditPolyMod()->getParamBlock()->GetDesc();
	
	switch(which)
	{
	case eBevelType:
			resetValue.mInt = 0;
			break;
	case eBevelHeight:
		{
			const ParamDef& pd = pbd->paramdefs[epm_bevel_height];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eBevelOutline:
		{
			const ParamDef& pd = pbd->paramdefs[epm_bevel_outline];
			resetValue.mFloat = pd.def.f;
		}
		break;
	default:
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Extrude Face Grips
ExtrudeFaceGrip::~ExtrudeFaceGrip()
{

}

int ExtrudeFaceGrip::GetNumGripItems()
{
	return 2; //would be 5 for the old one since the old inluded okay and apply (remember cancel was implicit)
}
IBaseGrip::Type ExtrudeFaceGrip::GetType(int which)
{
	DbgAssert(which>=0&&which<=1);
	switch(which)
	{
	case eExtrudeFaceType:
		return IBaseGrip::eCombo;
	case eExtrudeFaceHeight:
		return IBaseGrip::eUniverse;
	}
	return IBaseGrip::eInt;
}

void ExtrudeFaceGrip::GetGripName(TSTR &string)
{
	string =TSTR(GetString(IDS_FACE_EXTRUDE));
}

bool ExtrudeFaceGrip::GetText(int which,TSTR &string)
{
	switch(which)
	{
	case eExtrudeFaceType:
		{
			//Group , 1 for Local Normals, 2 for By Polygon 
			int type;
			GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_face_type, 0, type,FOREVER);
			switch(type)
			{
			case 0:
				string =TSTR(GetString(IDS_GROUP_NORMALS));
				break;
			case 1:
				string = TSTR(GetString(IDS_LOCAL_NORMALS));
				break;
			case 2:
				string = TSTR(GetString(IDS_BY_POLYGON));
				break;

			}
			break;
		}
	case eExtrudeFaceHeight:
		{
			string =TSTR(GetString(IDS_HEIGHT));
		}
		break;
	default:
		return false;

	}
	return true;
}

DWORD ExtrudeFaceGrip::GetCustomization(int which)
{
	return 0;
}

bool ExtrudeFaceGrip::GetResolvedIconName(int which,MSTR &string)
{
	switch(which)
	{
	case eExtrudeFaceType:
		string = EXTRUDE_TYPE_PNG;
		break;
	case eExtrudeFaceHeight:
		string =  EXTRUDE_HEIGHT_PNG;
		break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
    return true;
}

bool ExtrudeFaceGrip::GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &comboOptions)
{
	if(which== eExtrudeFaceType)
	{
		
		BOOL found;
		IBaseGrip::ComboLabel *label = new IBaseGrip::ComboLabel();
		TSTR string = EXTRUDE_GROUP_PNG;
		found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
		if(found)
			label->mIcon = string;
		label->mLabel = TSTR(GetString(IDS_GROUP_NORMALS));
		comboOptions.Append(1,&label);
		label = new IBaseGrip::ComboLabel();
		string = EXTRUDE_NORMAL_PNG;
		found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
		if(found)
			label->mIcon = string;
		label->mLabel = TSTR(GetString(IDS_LOCAL_NORMALS));
		comboOptions.Append(1,&label);
		label = new IBaseGrip::ComboLabel();
		string = EXTRUDE_POLYGON_PNG;
		found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
		if(found)
			label->mIcon = string;
		label->mLabel = TSTR(GetString(IDS_BY_POLYGON));
		comboOptions.Append(1,&label);
		return true;
	}
	return false;
}

bool ExtrudeFaceGrip::GetCommandIcon(int which, MSTR &string)
{
	return false;
}
bool ExtrudeFaceGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eExtrudeFaceType:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_extrude_face_type, t) == TRUE ? true : false;
	case eExtrudeFaceHeight:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_extrude_face_height,  t) == TRUE ? true : false;
	}
	return false;
}

bool ExtrudeFaceGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{

	ParamDef theDef;
	switch(which)
	{
	case eExtrudeFaceHeight:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_extrude_face_height);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;
	default:
		return false;

	}
	return true;
}

bool ExtrudeFaceGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eExtrudeFaceType:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_face_type, t, value.mInt,FOREVER);
		break;
	case eExtrudeFaceHeight:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_face_height, t, value.mFloat,FOREVER);
		break;
	default:
		return false;
	}
	return true;
}

bool ExtrudeFaceGrip::SetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eExtrudeFaceType:
		if(value.mInt>2) //go back to zero
			value.mInt =0;
		if(value.mInt<0)
			value.mInt = 2;
		GetEditPolyMod()->getParamBlock()->SetValue (epm_extrude_face_type, t, value.mInt);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_face_type, t, value.mInt,FOREVER);
		break;
	case eExtrudeFaceHeight:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_extrude_face_height, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_face_height, t, value.mFloat,FOREVER);
		break;
	default:
		return false;

	}
	return true;
}

void ExtrudeFaceGrip::Apply(TimeValue t)
{
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitAndRepeat (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void ExtrudeFaceGrip::Okay(TimeValue t)
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitUnlessAnimating (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void ExtrudeFaceGrip::Cancel()
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,0);
	GetEditPolyMod()->EpModCancel();
	EndSetValue(-1,0,true);
	GetEditPolyMod()->EpModRefreshScreen ();
}

bool ExtrudeFaceGrip::StartSetValue(int ,TimeValue t)
{
	theHold.Begin ();
	return true;
}

bool ExtrudeFaceGrip::EndSetValue(int ,TimeValue t,bool accept)
{
	if (accept)
	{
		if(theHold.Holding())
			theHold.Accept (GetString(IDS_EXTRUDE));

	}
	else
	{
		if(theHold.Holding())
			theHold.Cancel();
		GetEditPolyMod()->EpModRefreshScreen ();	
	}

	return true;
}

//we don't auto scale
bool ExtrudeFaceGrip::GetAutoScale(int which)
{
	return true; //yes
}
//default scale
bool ExtrudeFaceGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
	{
	case eExtrudeFaceHeight:
		scaleValue.mFloat = 1.0f;
		break;
	default:
		return false;
	}	
	return true;
}
bool ExtrudeFaceGrip::GetScaleInViewSpace(int which, float &depth)
{
	depth = -200.0f; 
	switch(which)
	{
	case eExtrudeFaceHeight:
		return true;
	}
	
	return false;
}
bool ExtrudeFaceGrip::ResetValue(TimeValue t, int which)
{
	float fResetVal = 0.0f;
	int iResetVal = 0;

	switch(which)
	{
	case eExtrudeFaceType:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_extrude_face_type, t, iResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_extrude_face_type, t , iResetVal,FOREVER);
			theHold.Accept (GetString(IDS_EXTRUDE));
		}
		break;
	case eExtrudeFaceHeight:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_extrude_face_height, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_extrude_face_height, t, fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_EXTRUDE));
		}
		break;
	default:
		return false;
	}
	return true;

}

bool ExtrudeFaceGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{
	ParamBlockDesc2* pbd = GetEditPolyMod()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eExtrudeFaceType:
		resetValue.mInt = 0;
		break;
	case eExtrudeFaceHeight:
		{
			const ParamDef& pd = pbd->paramdefs[epm_extrude_face_height];
			resetValue.mFloat = pd.def.f;
		}
		break;
	default:
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Extrude Edge Grips

ExtrudeEdgeGrip::~ExtrudeEdgeGrip()
{

}

int ExtrudeEdgeGrip::GetNumGripItems()
{
	return 2; //would be 5 for the old one since the old inluded okay and apply (remember cancel was implicit)
}
IBaseGrip::Type ExtrudeEdgeGrip::GetType(int which)
{
	DbgAssert(which>=0&&which<=1);
	switch(which)
	{
	case eExtrudeEdgeHeight:
		return IBaseGrip::eUniverse;
	case eExtrudeEdgeWidth:
		return IBaseGrip::eUniverse;
	}
	return IBaseGrip::eInt;
}

void ExtrudeEdgeGrip::GetGripName(TSTR &string)
{
	string =TSTR(GetString(IDS_EDGE_EXTRUDE));
}

bool ExtrudeEdgeGrip::GetText(int which,TSTR &string)
{
	switch(which)
	{
	case eExtrudeEdgeHeight:
		{
			string =TSTR(GetString(IDS_HEIGHT));
		}
		break;
	case eExtrudeEdgeWidth:
		{
			string =TSTR(GetString(IDS_WIDTH));
		}
		break;
	default:
		return false;

	}
	return true;
}

bool ExtrudeEdgeGrip::GetCommandIcon(int which, MSTR &string)
{
	return false;
}

DWORD ExtrudeEdgeGrip::GetCustomization(int which)
{
	return 0;
}

bool ExtrudeEdgeGrip::GetResolvedIconName(int which,MSTR &string)
{
	DbgAssert(which>=0&&which<=1);
	switch(which)
	{
	case eExtrudeEdgeHeight:
		string =  EXTRUDE_HEIGHT_PNG;
		break;
	case eExtrudeEdgeWidth:
		string =  EXTRUDE_WIDTH_PNG;
		break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

}

bool ExtrudeEdgeGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eExtrudeEdgeWidth:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_extrude_edge_width, t) == TRUE ? true : false;
	case eExtrudeEdgeHeight:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_extrude_edge_height,  t) == TRUE ? true : false;
	}
	return false;
}

bool ExtrudeEdgeGrip::GetComboOptions(int, Tab<IBaseGrip::ComboLabel*> &)
{
	//no combo buttons
	return false;
}

bool ExtrudeEdgeGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{

	ParamDef theDef;
	switch(which)
	{
	case eExtrudeEdgeHeight:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_extrude_edge_height);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eExtrudeEdgeWidth:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_extrude_edge_width);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;
	default:
		return false;

	}
	return true;
}

bool ExtrudeEdgeGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eExtrudeEdgeHeight:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_edge_height, t, value.mFloat,FOREVER);
		break;
	case eExtrudeEdgeWidth:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_edge_width, t, value.mFloat,FOREVER);
		break;
	default:
		return false;
	}
	return true;
}

bool ExtrudeEdgeGrip::SetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eExtrudeEdgeHeight:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_extrude_edge_height, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_edge_height, t, value.mFloat,FOREVER);
		break;
	case eExtrudeEdgeWidth:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_extrude_edge_width, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_edge_width, t, value.mFloat,FOREVER);
		break;
	default:
		return false;

	}
	return true;
}

void ExtrudeEdgeGrip::Apply(TimeValue t)
{
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitAndRepeat (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void ExtrudeEdgeGrip::Okay(TimeValue t)
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitUnlessAnimating (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void ExtrudeEdgeGrip::Cancel()
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,0);
	GetEditPolyMod()->EpModCancel();
	EndSetValue(-1,0,true);
	GetEditPolyMod()->EpModRefreshScreen ();
}

bool ExtrudeEdgeGrip::StartSetValue(int ,TimeValue t)
{
	theHold.Begin ();
	return true;
}

bool ExtrudeEdgeGrip::EndSetValue(int ,TimeValue t,bool accept)
{
	if (accept)
	{
		if(theHold.Holding())
			theHold.Accept (GetString(IDS_EXTRUDE));

	}
	else
	{
		if(theHold.Holding())
			theHold.Cancel();
		GetEditPolyMod()->EpModRefreshScreen ();	
	}

	return true;
}



//we don't auto scale
bool ExtrudeEdgeGrip::GetAutoScale(int which)
{
	return true; //yes
}
//default scale
bool ExtrudeEdgeGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
	{
	case eExtrudeEdgeHeight:
		scaleValue.mFloat = 1.0f;
		break;
	case eExtrudeEdgeWidth:
		scaleValue.mFloat = 1.0f;
		break;
	default:
		return false;
	}
	return true;
}
bool ExtrudeEdgeGrip::GetScaleInViewSpace(int which, float &depth)
{
	depth = -200.0f; 
	switch(which)
	{
	case eExtrudeEdgeHeight:
	case eExtrudeEdgeWidth:
		return true;
	}
	
	return false;
}
bool ExtrudeEdgeGrip::ResetValue(TimeValue t, int which)
{
	float fResetVal = 0.0f;
    
	switch(which)
	{
	case eExtrudeEdgeHeight:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_extrude_edge_height, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_extrude_edge_height, t, fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_EXTRUDE));
		}
		break;
	case eExtrudeEdgeWidth:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_extrude_edge_width, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_extrude_edge_width, t,fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_EXTRUDE));

		}
		break;
	default:
		return false;
	}
	return true;

}

bool ExtrudeEdgeGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{
	ParamBlockDesc2* pbd = GetEditPolyMod()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eExtrudeEdgeHeight:
		{
			const ParamDef& pd = pbd->paramdefs[epm_extrude_edge_height];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eExtrudeEdgeWidth:
		{
			const ParamDef& pd = pbd->paramdefs[epm_extrude_edge_width];
			resetValue.mFloat = pd.def.f;
		}
		break;
	default:
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Extrude Vertex Grips

ExtrudeVertexGrip::~ExtrudeVertexGrip()
{

}

int ExtrudeVertexGrip::GetNumGripItems()
{
	return 2; //would be 5 for the old one since the old inluded okay and apply (remember cancel was implicit)
}
IBaseGrip::Type ExtrudeVertexGrip::GetType(int which)
{
	DbgAssert(which>=0&&which<=1);
	switch(which)
	{
	case eExtrudeVertexHeight:
		return IBaseGrip::eUniverse;
	case eExtrudeVertexWidth:
		return IBaseGrip::eUniverse;
	}
	return IBaseGrip::eInt;
}

void ExtrudeVertexGrip::GetGripName(TSTR &string)
{
	string =TSTR(GetString(IDS_VERTEX_EXTRUDE));
}

bool ExtrudeVertexGrip::GetText(int which,TSTR &string)
{
	switch(which)
	{
	case eExtrudeVertexHeight:
		{
			string =TSTR(GetString(IDS_HEIGHT));
		}
		break;
	case eExtrudeVertexWidth:
		{
			string =TSTR(GetString(IDS_WIDTH));
		}
		break;
	default:
		return false;

	}
	return true;
}

DWORD ExtrudeVertexGrip::GetCustomization(int which)
{
	return 0;
}

bool ExtrudeVertexGrip::GetResolvedIconName(int which,MSTR &string)
{

	switch(which)
	{
	case eExtrudeVertexHeight:
		string =  EXTRUDE_HEIGHT_PNG;
		break;
	case eExtrudeVertexWidth:
		string =  EXTRUDE_WIDTH_PNG;
		break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

}
bool ExtrudeVertexGrip::GetComboOptions(int , Tab<IBaseGrip::ComboLabel*> &)
{
	//no combo buttons
	return false;
}


bool ExtrudeVertexGrip::GetCommandIcon(int which, MSTR &string)
{
	return false;
}


bool ExtrudeVertexGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eExtrudeVertexWidth:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_extrude_vertex_width, t) == TRUE ? true : false;
	case eExtrudeVertexHeight:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_extrude_vertex_height,  t) == TRUE ? true : false;
	}
	return false;
}

bool ExtrudeVertexGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{

	ParamDef theDef;
	switch(which)
	{
	case eExtrudeVertexHeight:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_extrude_vertex_height);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eExtrudeVertexWidth:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_extrude_vertex_width);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;
	default:
		return false;

	}
	return true;
}

bool ExtrudeVertexGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eExtrudeVertexHeight:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_vertex_height, t, value.mFloat,FOREVER);
		break;
	case eExtrudeVertexWidth:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_vertex_width, t, value.mFloat,FOREVER);
		break;
	default:
		return false;
	}
	return true;
}

bool ExtrudeVertexGrip::SetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eExtrudeVertexHeight:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_extrude_vertex_height, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_vertex_height, t, value.mFloat,FOREVER);
		break;
	case eExtrudeVertexWidth:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_extrude_vertex_width, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_vertex_width, t, value.mFloat,FOREVER);
		break;
	default:
		return false;

	}
	return true;
}

void ExtrudeVertexGrip::Apply(TimeValue t)
{
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitAndRepeat (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void ExtrudeVertexGrip::Okay(TimeValue t)
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitUnlessAnimating (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void ExtrudeVertexGrip::Cancel()
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,0);
	GetEditPolyMod()->EpModCancel();
	EndSetValue(-1,0,true);
	GetEditPolyMod()->EpModRefreshScreen ();
}

bool ExtrudeVertexGrip::StartSetValue(int ,TimeValue t)
{
	theHold.Begin ();
	return true;
}

bool ExtrudeVertexGrip::EndSetValue(int ,TimeValue t,bool accept)
{
	if (accept)
	{
		if(theHold.Holding())
			theHold.Accept (GetString(IDS_EXTRUDE));

	}
	else
	{
		if(theHold.Holding())
			theHold.Cancel();
		GetEditPolyMod()->EpModRefreshScreen ();	
	}

	return true;
}



//we don't auto scale
bool ExtrudeVertexGrip::GetAutoScale(int which)
{
	return true; //yes
}
//default scale
bool ExtrudeVertexGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
	{
	case eExtrudeVertexHeight:
		scaleValue.mFloat = 1.0f;
		break;
	case eExtrudeVertexWidth:
		scaleValue.mFloat = 1.0f;
		break;
	default:
		return false;
	}
	return true;
}
bool ExtrudeVertexGrip::GetScaleInViewSpace(int which, float &depth)
{
	depth = -200.0f; 
	switch(which)
	{
	case eExtrudeVertexHeight:
	case eExtrudeVertexWidth:
		return true;
	}
	
	return false;
}
bool ExtrudeVertexGrip::ResetValue(TimeValue t, int which)
{
	float fResetVal = 0.0f;

	switch(which)
	{
	case eExtrudeVertexHeight:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_extrude_vertex_height, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_extrude_vertex_height, t, fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_EXTRUDE));
		}
		break;
	case eExtrudeVertexWidth:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_extrude_vertex_width, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_extrude_vertex_width, t,fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_EXTRUDE));

		}
		break;
	default:
		return false;
	}
	return true;

}

bool ExtrudeVertexGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{
	ParamBlockDesc2* pbd = GetEditPolyMod()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eExtrudeVertexHeight:
		{
			const ParamDef& pd = pbd->paramdefs[epm_extrude_vertex_height];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eExtrudeVertexWidth:
		{
			const ParamDef& pd = pbd->paramdefs[epm_extrude_vertex_width];
			resetValue.mFloat = pd.def.f;
		}
		break;
	default:
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Outline Grips
OutlineGrip::~OutlineGrip()
{

}

int OutlineGrip::GetNumGripItems()
{
	return 1; 
}
IBaseGrip::Type OutlineGrip::GetType(int which)
{
	DbgAssert( which == 0 );
	switch(which)
	{
	case eOutlineAmount:
		return IBaseGrip::eUniverse;
	}
	return IBaseGrip::eInt;
}

void OutlineGrip::GetGripName(TSTR &string)
{
	string =TSTR(GetString(IDS_OUTLINE));
}

bool OutlineGrip::GetText(int which,TSTR &string)
{
	DbgAssert(which == 0);
	switch(which)
	{
	case eOutlineAmount:
		{
			string =TSTR(GetString(IDS_AMOUNT));
		}
		break;
	default:
		return false;
	}
	return true;
}

DWORD OutlineGrip::GetCustomization(int which)
{
	return 0;
}

bool OutlineGrip::GetResolvedIconName(int which,MSTR &string)
{
	switch(which)
	{
	case eOutlineAmount:
		string =  OUTLINE_AMOUNT_PNG;
		break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

}

bool OutlineGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eOutlineAmount:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_outline, t) == TRUE ? true : false;
	}
	return false;
}

bool OutlineGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{

	ParamDef theDef;
	switch(which)
	{
	case eOutlineAmount:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_outline);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;
	default:
		return false;

	}
	return true;
}

bool OutlineGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eOutlineAmount:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_outline, t, value.mFloat,FOREVER);
		break;
	default:
		return false;
	}
	return true;
}

bool OutlineGrip::SetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eOutlineAmount:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_outline, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_outline, t, value.mFloat,FOREVER);
		break;
	default:
		return false;
	}
	return true;
}

void OutlineGrip::Apply(TimeValue t)
{
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitAndRepeat (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void OutlineGrip::Okay(TimeValue t)
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitUnlessAnimating (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void OutlineGrip::Cancel()
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,0);
	GetEditPolyMod()->EpModCancel();
	EndSetValue(-1,0,true);
	GetEditPolyMod()->EpModRefreshScreen ();
}

bool OutlineGrip::StartSetValue(int ,TimeValue t)
{
	theHold.Begin ();
	/*	int type = GetEditPolyMod()->GetPolyOperationID ();
	if (type != ep_op_Outline) {
	GetEditPolyMod()->EpModSetOperation (ep_op_Outline);
	} 
	*/
	return true;
}

bool OutlineGrip::EndSetValue(int ,TimeValue t,bool accept)
{
	if (accept)
	{
		if(theHold.Holding())
			theHold.Accept (GetString(IDS_OUTLINE));

	}
	else
	{
		if(theHold.Holding())
			theHold.Cancel();
		GetEditPolyMod()->EpModRefreshScreen ();	
	}

	return true;
}



//we don't auto scale
bool OutlineGrip::GetAutoScale(int which)
{
	return true; //yes
}
//default scale
bool OutlineGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
	{
	case eOutlineAmount:
		scaleValue.mFloat = 1.0f;
		break;
	default:
		return false;
	}
	return true;
}
bool OutlineGrip::GetScaleInViewSpace(int which, float &depth)
{
	depth = -200.0f; 
	switch(which)
	{
	case eOutlineAmount:
		return true;
	}
	
	return false;
}
bool OutlineGrip::ResetValue(TimeValue t, int which)
{
	float fResetVal = 0.0f;

	switch(which)
	{
	case eOutlineAmount:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_outline, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_outline, t, fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_OUTLINE));
		}
		break;
	default:
		return false;
	}
	return true;

}

bool OutlineGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{
	ParamBlockDesc2* pbd = GetEditPolyMod()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eOutlineAmount:
		{
			const ParamDef& pd = pbd->paramdefs[epm_outline];
			resetValue.mFloat = pd.def.f;
		}
		break;
	default:
		return false;
	}
	return true;
}

bool OutlineGrip::GetComboOptions(int, Tab<IBaseGrip::ComboLabel*> &)
{
	//no combo buttons
	return false;
}


bool OutlineGrip::GetCommandIcon(int which, MSTR &string)
{
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Inset Grips
InsetGrip::~InsetGrip()
{

}

int InsetGrip::GetNumGripItems()
{
	return 2; 
}
IBaseGrip::Type InsetGrip::GetType(int which)
{
	DbgAssert(which>=0&&which<=1);
	switch(which)
	{
	case eInsetType:
		return IBaseGrip::eCombo;
	case eInsetAmount:
		return IBaseGrip::eUniverse;
	}
	return IBaseGrip::eInt;
}

void InsetGrip::GetGripName(TSTR &string)
{
	string =TSTR(GetString(IDS_INSET));
}

bool InsetGrip::GetText(int which,TSTR &string)
{
	switch(which)
	{
	case eInsetType:
		{
			//Group , 1 for By Polygon 
			int type;
			GetEditPolyMod()->getParamBlock()->GetValue (epm_inset_type, 0, type,FOREVER);
			switch(type)
			{
			case 0:
				string = TSTR(GetString(IDS_GROUP));
				break;
			case 1:
				string = TSTR(GetString(IDS_BY_POLYGON));
				break;

			}
			break;
		}
	case eInsetAmount:
		{
			string =TSTR(GetString(IDS_AMOUNT));
		}
		break;
	default:
		return false;
	}
	return true;
}

DWORD InsetGrip::GetCustomization(int which)
{
	return 0;
}

bool InsetGrip::GetResolvedIconName(int which,MSTR &string)
{
	switch(which)
	{
	case eInsetType:
		string = INSET_TYPE_PNG;
		break;
	case eInsetAmount:
		string =  INSET_AMOUNT_PNG;
		break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

}

bool InsetGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eInsetAmount:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_inset, t) == TRUE ? true : false;
	}
	return false;
}

bool InsetGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{

	ParamDef theDef;
	switch(which)
	{
	case eInsetAmount:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_inset);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;
	default:
		return false;
	}
	return true;
}

bool InsetGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eInsetType:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_inset_type, t, value.mInt,FOREVER);
		break;
	case eInsetAmount:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_inset, t, value.mFloat,FOREVER);
		break;
	default:
		return false;
	}
	return true;
}

bool InsetGrip::SetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eInsetType:
		if(value.mInt>1) //go back to zero
			value.mInt =0;
		if(value.mInt<0)
			value.mInt = 1;
		GetEditPolyMod()->getParamBlock()->SetValue (epm_inset_type, t, value.mInt);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_inset_type, t, value.mInt,FOREVER);
		break;
	case eInsetAmount:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_inset, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_inset, t, value.mFloat,FOREVER);
		break;
	default:
		return false;
	}
	return true;
}

void InsetGrip::Apply(TimeValue t)
{
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitAndRepeat (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void InsetGrip::Okay(TimeValue t)
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitUnlessAnimating (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void InsetGrip::Cancel()
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,0);
	GetEditPolyMod()->EpModCancel();
	EndSetValue(-1,0,true);
	GetEditPolyMod()->EpModRefreshScreen ();
}

bool InsetGrip::StartSetValue(int ,TimeValue t)
{
	theHold.Begin ();
	/*	int type = GetEditPolyMod()->GetPolyOperationID ();
	if (type != ep_op_Inset) {
	GetEditPolyMod()->EpModSetOperation (ep_op_Inset);
	} 
	*/
	return true;
}

bool InsetGrip::EndSetValue(int ,TimeValue t,bool accept)
{
	if (accept)
	{
		if(theHold.Holding())
			theHold.Accept (GetString(IDS_INSET));

	}
	else
	{
		if(theHold.Holding())
			theHold.Cancel();
		GetEditPolyMod()->EpModRefreshScreen ();	
	}

	return true;
}



//we don't auto scale
bool InsetGrip::GetAutoScale(int which)
{
	return true; //yes
}
//default scale
bool InsetGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
	{
	case eInsetAmount:
		scaleValue.mFloat = 1.0f;
		break;
	default:
		return false;
	}
	return true;
}
bool InsetGrip::GetScaleInViewSpace(int which, float &depth)
{
	depth = -200.0f; 
	switch(which)
	{
	case eInsetAmount:
		return true;
	}
	
	return false;
}
bool InsetGrip::ResetValue(TimeValue t, int which)
{
    float fResetVal = 0.0f;
	int iResetVal = 0;

	switch(which)
	{
	case eInsetType:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_inset_type, t, iResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_inset_type, t, iResetVal,FOREVER);
			theHold.Accept (GetString(IDS_INSET));
		}
		break;
	case eInsetAmount:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_inset, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_inset, t, fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_INSET));
		}
		break;
	default:
		return false;
	}
	return true;

}

bool InsetGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{
	ParamBlockDesc2* pbd = GetEditPolyMod()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eInsetType:
		resetValue.mInt = 0;
		break;
	case eInsetAmount:
		{
			const ParamDef& pd = pbd->paramdefs[epm_inset];
			resetValue.mFloat = pd.def.f;
		}
		break;
	default:
		return false;
	}
	return true;
}

bool InsetGrip::GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &comboOptions)
{
	if(which== eInsetType)
	{
		BOOL found;
		IBaseGrip::ComboLabel *label = new IBaseGrip::ComboLabel();
		TSTR string = BEVEL_GROUP_PNG;
		found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
		if(found)
			label->mIcon = string;
		label->mLabel = TSTR(GetString(IDS_GROUP));
		comboOptions.Append(1,&label);
		label = new IBaseGrip::ComboLabel();
		string = BEVEL_POLYGON_PNG;
		found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
		if(found)
			label->mIcon = string;
		label->mLabel = TSTR(GetString(IDS_BY_POLYGON));
		comboOptions.Append(1,&label);
		return true;
	}
	return false;
}


bool InsetGrip::GetCommandIcon(int which, MSTR &string)
{
	return false;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Connect Edge Grip

ConnectEdgeGrip::~ConnectEdgeGrip()
{

}

int ConnectEdgeGrip::GetNumGripItems()
{
	return 3; 
}
IBaseGrip::Type ConnectEdgeGrip::GetType(int which)
{
	DbgAssert(which>=0&&which<=2);
	switch(which)
	{
	case eConnectEdgeSegments:
		return IBaseGrip::eInt;
	case eConnectEdgePinch:
		return IBaseGrip::eInt;
	case eConnectEdgeSlide:
		return IBaseGrip::eInt;
	}
	return IBaseGrip::eInt;
}

void ConnectEdgeGrip::GetGripName(TSTR &string)
{
	string =TSTR(GetString(IDS_CONNECT_EDGES));
}

bool ConnectEdgeGrip::GetText(int which,TSTR &string)
{
	switch(which)
	{
	case eConnectEdgeSegments:
		{
			string =TSTR(GetString(IDS_SEGMENTS));
		}
		break;
	case eConnectEdgePinch:
		{
			string =TSTR(GetString(IDS_PINCH));
		}
		break;
	case eConnectEdgeSlide:
		{
			string =TSTR(GetString(IDS_SLIDE));
		}
		break;
	default:
		return false;

	}
	return true;
}

DWORD ConnectEdgeGrip::GetCustomization(int which)
{
	return 0;
}

bool ConnectEdgeGrip::GetResolvedIconName(int which,MSTR &string)
{
	switch(which)
	{
	case eConnectEdgeSegments:
		string = CONNECT_EDGE_SEGMENTS_PNG;
		break;
	case eConnectEdgePinch:
		string =  CONNECT_EDGE_PINCH_PNG;
		break;
	case eConnectEdgeSlide:
		string =  CONNECT_EDGE_SLIDE_PNG;
		break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

}

bool ConnectEdgeGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eConnectEdgeSegments:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_connect_edge_segments, t) == TRUE ? true : false;
	case eConnectEdgePinch:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_connect_edge_pinch, t) == TRUE ? true : false;
	case eConnectEdgeSlide:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_connect_edge_slide, t) == TRUE ? true : false;
	}
	return false;
}




bool ConnectEdgeGrip::GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &comboOptions)
{
	// have no radio options
	return false;
}

bool ConnectEdgeGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{

	ParamDef theDef;
	switch(which)
	{
	case eConnectEdgeSegments:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_connect_edge_segments);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mInt = theDef.range_low.i;
			maxRange.mInt = theDef.range_high.i;
		}
		break;

	case eConnectEdgePinch:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_connect_edge_pinch);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mInt = theDef.range_low.i;
			maxRange.mInt = theDef.range_high.i;
		}
		break;

	case eConnectEdgeSlide:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_connect_edge_slide);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mInt = theDef.range_low.i;
			maxRange.mInt = theDef.range_high.i;
		}
		break;
	default:
		return false;

	}
	return true;
}


bool ConnectEdgeGrip::GetCommandIcon(int which, MSTR &string)
{
	return false;
}


bool ConnectEdgeGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eConnectEdgeSegments:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_connect_edge_segments, t, value.mInt,FOREVER);
		break;
	case eConnectEdgePinch:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_connect_edge_pinch, t, value.mInt,FOREVER);
		break;
	case eConnectEdgeSlide:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_connect_edge_slide, t, value.mInt,FOREVER);
		break;
	default:
		return false;
	}
	return true;
}

bool ConnectEdgeGrip::SetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eConnectEdgeSegments:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_connect_edge_segments, t, value.mInt);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_connect_edge_segments, t, value.mInt,FOREVER);
		break;
	case eConnectEdgePinch:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_connect_edge_pinch, t, value.mInt);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_connect_edge_pinch, t, value.mInt,FOREVER);
		break;
	case eConnectEdgeSlide:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_connect_edge_slide, t, value.mInt);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_connect_edge_slide, t, value.mInt,FOREVER);
		break;
	default:
		return false;

	}
	return true;
}

void ConnectEdgeGrip::Apply(TimeValue t)
{
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitAndRepeat (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void ConnectEdgeGrip::Okay(TimeValue t)
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitUnlessAnimating (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void ConnectEdgeGrip::Cancel()
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,0);
	GetEditPolyMod()->EpModCancel();
	EndSetValue(-1,0,true);
	GetEditPolyMod()->EpModRefreshScreen ();
}

bool ConnectEdgeGrip::StartSetValue(int ,TimeValue t)
{
	theHold.Begin ();
	/*	int type = GetEditPolyMod()->GetPolyOperationID ();
	if (type != ep_op_ConnectEdge) {
	GetEditPolyMod()->EpModSetOperation (ep_op_ConnectEdge);
	} 
	*/
	return true;
}

bool ConnectEdgeGrip::EndSetValue(int ,TimeValue t,bool accept)
{
	if (accept)
	{
		if(theHold.Holding())
			theHold.Accept (GetString(IDS_CONNECT_EDGES));

	}
	else
	{
		if(theHold.Holding())
			theHold.Cancel();
		GetEditPolyMod()->EpModRefreshScreen ();	
	}

	return true;
}



//we don't auto scale
bool ConnectEdgeGrip::GetAutoScale(int which)
{
	return true; //yes
}
//default scale
bool ConnectEdgeGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
	{
	case eConnectEdgeSegments:
		scaleValue.mInt = 1;
		break;
	case eConnectEdgePinch:
		scaleValue.mInt = 1;
		break;
	case eConnectEdgeSlide:
		scaleValue.mInt = 1;
		break;
	default:
		return false;
	}
	return true;
}
bool ConnectEdgeGrip::GetScaleInViewSpace(int which, float &depth)
{
	depth = -200.0f; 
	return false;
}
bool ConnectEdgeGrip::ResetValue(TimeValue t, int which)
{
    int iResetSegVal = 1;
	int iResetPinchVal = 0;
	int iResetSlideVal = 0;

	switch(which)
	{
	case eConnectEdgeSegments:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_connect_edge_segments, t,iResetSegVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_connect_edge_segments, t,iResetSegVal,FOREVER);
			theHold.Accept (GetString(IDS_CONNECT_EDGES));
		}
		break;
	case eConnectEdgePinch:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_connect_edge_pinch, t,iResetPinchVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_connect_edge_pinch, t,iResetPinchVal,FOREVER);
			theHold.Accept (GetString(IDS_CONNECT_EDGES));
		}
		break;
	case eConnectEdgeSlide:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_connect_edge_slide, t,iResetSlideVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_connect_edge_slide, t,iResetSlideVal,FOREVER);
			theHold.Accept (GetString(IDS_CONNECT_EDGES));

		}
		break;
	default:
		return false;
	}
	return true;

}

bool ConnectEdgeGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{
	ParamBlockDesc2* pbd = GetEditPolyMod()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eConnectEdgeSegments:
		{
			const ParamDef& pd = pbd->paramdefs[epm_connect_edge_segments];
			resetValue.mInt = pd.def.i;
		}
		break;
	case eConnectEdgePinch:
		{
			const ParamDef& pd = pbd->paramdefs[epm_connect_edge_pinch];
			resetValue.mInt = pd.def.i;
		}
		break;
	case eConnectEdgeSlide:
		{
			const ParamDef& pd = pbd->paramdefs[epm_connect_edge_slide];
			resetValue.mInt = pd.def.i;
		}
		break;
	default:
		return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Relax Grips( polygon, vertices, edges,border all use one setting grip )

RelaxGrip::~RelaxGrip()
{

}

int RelaxGrip::GetNumGripItems()
{
	return 4; 
}
IBaseGrip::Type RelaxGrip::GetType(int which)
{
	DbgAssert(which>=0&&which<=3);
	switch(which)
	{
	case eRelaxAmount:
		return IBaseGrip::eFloat;
	case eRelaxIterations:
		return IBaseGrip::eInt;
	case eRelaxBoundaryPoints:
		return IBaseGrip::eToggle;
	case eRelaxOuterPoints:
		return IBaseGrip::eToggle;
	}
	return IBaseGrip::eInt;
}

void RelaxGrip::GetGripName(TSTR &string)
{
	string =TSTR(GetString(IDS_RELAX));
}

bool RelaxGrip::GetText(int which,TSTR &string)
{
	switch(which)
	{
	case eRelaxAmount:
		{
			string =TSTR(GetString(IDS_AMOUNT));
		}
		break;
	case eRelaxIterations:
		{
			string =TSTR(GetString(IDS_ITERATIONS));
		}
		break;
	case eRelaxBoundaryPoints:
		{
			string = TSTR(GetString(IDS_RELAX_HOLD_BOUNDARY_POINTS));
		}
		break;
	case eRelaxOuterPoints:
		{
			string = TSTR(GetString(IDS_RELAX_HOLD_OUTER_POINTS));
		}
        break;
	default:
		return false;
	}
	return true;
}

DWORD RelaxGrip::GetCustomization(int which)
{
	DWORD word =0;
	switch(which)
	{
	case eRelaxOuterPoints:
		word = (int)(IBaseGrip::eSameRow);
		break;
	}
	return word;
}

bool RelaxGrip::GetResolvedIconName(int which,MSTR &string)
{
	switch(which)
	{
	case eRelaxAmount:
		string = RELAX_AMOUNT_PNG;
		break;
	case eRelaxIterations:
		string =  RELAX_ITERATIONS_PNG;
		break;
	case eRelaxBoundaryPoints:
		{
			string = RELAX_BOUNDARY_POINTS_PNG;
		}
		break;
	case eRelaxOuterPoints:
		{
			string = RELAX_OUTER_POINTS_PNG;
		}
        break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

}

bool RelaxGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eRelaxAmount:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_relax_amount, t) == TRUE ? true : false;
	case eRelaxIterations:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_relax_iters, t) == TRUE ? true : false;
	}
	return false;
}


bool RelaxGrip::GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &comboOptions)
{
	// have no radio options
	return false;
}


bool RelaxGrip::GetCommandIcon(int which, MSTR &string)
{
	return false;
}


bool RelaxGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{

	ParamDef theDef;
	switch(which)
	{
	case eRelaxAmount:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_relax_amount);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = -1.0f;
			maxRange.mFloat = 1.0f;
		}
		break;

	case eRelaxIterations:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_relax_iters);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mInt = 1;
			maxRange.mInt = theDef.range_high.i;
		}
		break;
	default:
		return false;

	}
	return true;
}

bool RelaxGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eRelaxAmount:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_relax_amount, t, value.mFloat,FOREVER);
		break;
	case eRelaxIterations:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_relax_iters, t, value.mInt,FOREVER);
		break;
	case eRelaxBoundaryPoints:
		{
			int val = 0;
		    GetEditPolyMod()->getParamBlock()->GetValue (epm_relax_hold_boundary, t, val,FOREVER);
			value.mbool = ( val == 0 ? false:true);
		}
		break;
	case eRelaxOuterPoints:
		{
			int val = 0;
			GetEditPolyMod()->getParamBlock()->GetValue (epm_relax_hold_outer, t, val,FOREVER);
			value.mbool = ( val == 0 ? false:true);
		}
		break;
	default:
		return false;
	}
	return true;
}

bool RelaxGrip::SetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eRelaxAmount:
		if( value.mFloat > 1.0f )
			value.mFloat = 1.0f;
		if( value.mFloat < -1.0f )
			value.mFloat = -1.0f;
		GetEditPolyMod()->getParamBlock()->SetValue (epm_relax_amount, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_relax_amount, t, value.mFloat,FOREVER);
		break;
	case eRelaxIterations:
		if( value.mInt < 1)
			value.mInt = 1 ;
		GetEditPolyMod()->getParamBlock()->SetValue (epm_relax_iters, t, value.mInt);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_relax_iters, t, value.mInt,FOREVER);
		break;
	case eRelaxBoundaryPoints:
		{
			int val = (int)value.mbool ;
			GetEditPolyMod()->getParamBlock()->SetValue (epm_relax_hold_boundary, t, val);
		    GetEditPolyMod()->getParamBlock()->GetValue (epm_relax_hold_boundary, t, val,FOREVER);
		}
		break;
	case eRelaxOuterPoints:
		{
			int val = (int)value.mbool;
			GetEditPolyMod()->getParamBlock()->SetValue (epm_relax_hold_outer, t, val);
		    GetEditPolyMod()->getParamBlock()->GetValue (epm_relax_hold_outer, t, val,FOREVER);
		}
		break;
	default:
		return false;
	}
	return true;
}

void RelaxGrip::Apply(TimeValue t)
{
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitAndRepeat (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void RelaxGrip::Okay(TimeValue t)
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitUnlessAnimating (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void RelaxGrip::Cancel()
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,0);
	GetEditPolyMod()->EpModCancel();
	EndSetValue(-1,0,true);
	GetEditPolyMod()->EpModRefreshScreen ();
}

bool RelaxGrip::StartSetValue(int ,TimeValue t)
{
	theHold.Begin ();
	/*	int type = GetEditPolyMod()->GetPolyOperationID ();
	if (type != ep_op_Relax) {
	GetEditPolyMod()->EpModSetOperation (ep_op_Relax);
	} 
	*/
	return true;
}

bool RelaxGrip::EndSetValue(int ,TimeValue t,bool accept)
{
	if (accept)
	{
		if(theHold.Holding())
			theHold.Accept (GetString(IDS_RELAX));

	}
	else
	{
		if(theHold.Holding())
			theHold.Cancel();
		GetEditPolyMod()->EpModRefreshScreen ();	
	}

	return true;
}



//we don't auto scale
bool RelaxGrip::GetAutoScale(int which)
{
	return true; //yes
}
//default scale
bool RelaxGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
	{
	case eRelaxAmount:
		scaleValue.mFloat = 1.0f;
		break;
	case eRelaxIterations:
		scaleValue.mInt = 1;
		break;
	default:
		return false;
	}
	return true;
}
bool RelaxGrip::GetScaleInViewSpace(int which, float &depth)
{
	depth = -200.0f; 
	return false;
}
bool RelaxGrip::ResetValue(TimeValue t, int which)
{
	float fResetVal = 0.0f;
	int iResetIterVal = 1;
	int iResetVal = 0;

	switch(which)
	{
	case eRelaxAmount:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_relax_amount, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_relax_amount, t, fResetVal, FOREVER);
			theHold.Accept (GetString(IDS_RELAX));
		}
		break;
	case eRelaxIterations:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_relax_iters, t,iResetIterVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_relax_iters, t, iResetIterVal,FOREVER);
			theHold.Accept (GetString(IDS_RELAX));

		}
		break;
	case eRelaxBoundaryPoints:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_relax_hold_boundary, t, iResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_relax_hold_boundary, t, iResetVal,FOREVER);
			theHold.Accept (GetString(IDS_RELAX));

		}
		break;
	case eRelaxOuterPoints:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_relax_hold_outer, t, iResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_relax_hold_outer, t, iResetVal,FOREVER);
			theHold.Accept (GetString(IDS_RELAX));

		}
		break;
	default:
		return false;
	}
	return true;

}

bool RelaxGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{
	ParamBlockDesc2* pbd = GetEditPolyMod()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eRelaxAmount:
		{
			const ParamDef& pd = pbd->paramdefs[epm_relax_amount];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eRelaxIterations:
		{
			const ParamDef& pd = pbd->paramdefs[epm_relax_iters];
			resetValue.mInt = pd.def.i;
		}
		break;
	case eRelaxBoundaryPoints:
		{
			const ParamDef& pd = pbd->paramdefs[epm_relax_hold_boundary];
			resetValue.mbool = (pd.def.i == 0 ? false:true);
		}
		break;
	case eRelaxOuterPoints:
		{
			const ParamDef& pd = pbd->paramdefs[epm_relax_hold_outer];
			resetValue.mbool = (pd.def.i == 0 ? false:true);
		}
		break;
	default:
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Chamfer Vertices Grip

ChamferVerticesGrip::~ChamferVerticesGrip()
{

}

int ChamferVerticesGrip::GetNumGripItems()
{
	return 2; //would be 5 for the old one since the old inluded okay and apply (remember cancel was implicit)
}
IBaseGrip::Type ChamferVerticesGrip::GetType(int which)
{
	DbgAssert(which>=0&&which<=1);
	switch(which)
	{
	case eChamferVerticesAmount:
		return IBaseGrip::eUniverse;
	case eChamferVerticesOpen:
		return IBaseGrip::eToggle;
	}
	return IBaseGrip::eInt;
}

void ChamferVerticesGrip::GetGripName(TSTR &string)
{
	string =TSTR(GetString(IDS_CHAMFER));
}

bool ChamferVerticesGrip::GetText(int which,TSTR &string)
{
	DbgAssert(which>=0&&which<=1);
	switch(which)
	{
	case eChamferVerticesAmount:
		{
			string =TSTR(GetString(IDS_VERTEX_CHAMFER_AMOUNT));
		}
		break;
	case eChamferVerticesOpen:
		{
			string =TSTR(GetString(IDS_OPEN_CHAMFER));
		}
		break;
	default:
		return false;

	}
	return true;
}

DWORD ChamferVerticesGrip::GetCustomization(int which)
{
	return 0;
}

bool ChamferVerticesGrip::GetResolvedIconName(int which,MSTR &string)
{
	DbgAssert(which>=0&&which<=1);
	switch(which)
	{
	case eChamferVerticesAmount:
		string =  CHAMFER_VERTICES_AMOUNT_PNG;
		break;
	case eChamferVerticesOpen:
		{
			string =  CHAMFER_VERTICES_OPEN_PNG;
		}
		break;
	default:
		return false;

	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

}

bool ChamferVerticesGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eChamferVerticesAmount:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_chamfer_vertex, t) == TRUE ? true : false;
	}
	return false;
}


bool ChamferVerticesGrip::GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &comboOptions)
{
	// have no radio options
	return false;
}


bool ChamferVerticesGrip::GetCommandIcon(int which, MSTR &string)
{
	return false;
}

bool ChamferVerticesGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{

	ParamDef theDef;
	switch(which)
	{
	case eChamferVerticesAmount:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_chamfer_vertex);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = 0.0f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;
	default:
		return false;

	}
	return true;
}

bool ChamferVerticesGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eChamferVerticesAmount:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_chamfer_vertex, t, value.mFloat,FOREVER);
		break;
	case eChamferVerticesOpen:
		{
			//need to go BOOL (int) to bool
			int val =0;
		    GetEditPolyMod()->getParamBlock()->GetValue (epm_chamfer_vertex_open, t, val ,FOREVER);
			value.mbool = ( val == 0 ? false:true );
		}
		break;
	default:
		return false;
	}
	return true;
}

bool ChamferVerticesGrip::SetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eChamferVerticesAmount:
		if( value.mFloat < 0.0f )
			value.mFloat = 0.0f ;
		GetEditPolyMod()->getParamBlock()->SetValue (epm_chamfer_vertex, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_chamfer_vertex, t, value.mFloat,FOREVER);
		break;
	case eChamferVerticesOpen:
		{
            int val = (int)value.mbool ;
		    GetEditPolyMod()->getParamBlock()->SetValue (epm_chamfer_vertex_open, t, val );
		    GetEditPolyMod()->getParamBlock()->GetValue (epm_chamfer_vertex_open, t, val ,FOREVER);
		}
		break;
	default:
		return false;

	}
	return true;
}

void ChamferVerticesGrip::Apply(TimeValue t)
{
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitAndRepeat (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void ChamferVerticesGrip::Okay(TimeValue t)
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitUnlessAnimating (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void ChamferVerticesGrip::Cancel()
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,0);
	GetEditPolyMod()->EpModCancel();
	EndSetValue(-1,0,true);
	GetEditPolyMod()->EpModRefreshScreen ();
}

bool ChamferVerticesGrip::StartSetValue(int ,TimeValue t)
{
	theHold.Begin ();
	/*	int type = GetEditPolyMod()->GetPolyOperationID ();
	if (type != ep_op_ChamferVertices) {
	GetEditPolyMod()->EpModSetOperation (ep_op_ChamferVertices);
	} 
	*/
	return true;
}

bool ChamferVerticesGrip::EndSetValue(int ,TimeValue t,bool accept)
{
	if (accept)
	{
		if(theHold.Holding())
			theHold.Accept (GetString(IDS_CHAMFER));
	}
	else
	{
		if(theHold.Holding())
			theHold.Cancel();
		GetEditPolyMod()->EpModRefreshScreen ();	
	}

	return true;
}



//we don't auto scale
bool ChamferVerticesGrip::GetAutoScale(int which)
{
	return true; //yes
}
//default scale
bool ChamferVerticesGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
	{
	case eChamferVerticesAmount:
		scaleValue.mFloat = 1.0f;
		break;
	default:
		return false;
	}
	return true;
}
bool ChamferVerticesGrip::GetScaleInViewSpace(int which, float &depth)
{
	depth = -200.0f; 
	switch(which)
	{
	case eChamferVerticesAmount:
		return true;
	}
	
	return false;
}
bool ChamferVerticesGrip::ResetValue(TimeValue t, int which)
{
    float fResetVal = 0.0f;
	int iResetVal = 0;

	switch(which)
	{
	case eChamferVerticesAmount:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_chamfer_vertex, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_chamfer_vertex, t, fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_CHAMFER));
		}
		break;
	case eChamferVerticesOpen:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_chamfer_vertex_open, t, iResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_chamfer_vertex_open, t , iResetVal,FOREVER);
			theHold.Accept (GetString(IDS_CHAMFER));

		}
		break;
	default:
		return false;
	}
	return true;

}

bool ChamferVerticesGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{
	ParamBlockDesc2* pbd = GetEditPolyMod()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eChamferVerticesAmount:
		{
			const ParamDef& pd = pbd->paramdefs[epm_chamfer_vertex];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eChamferVerticesOpen:
		{
			const ParamDef& pd = pbd->paramdefs[epm_chamfer_vertex_open];
			resetValue.mbool = (pd.def.i ==0 ? false : true);
		}
		break;
	default:
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Chamfer Edge Grip

ChamferEdgeGrip::~ChamferEdgeGrip()
{

}

int ChamferEdgeGrip::GetNumGripItems()
{
	return 3; //would be 5 for the old one since the old inluded okay and apply (remember cancel was implicit)
}
IBaseGrip::Type ChamferEdgeGrip::GetType(int which)
{
	DbgAssert(which>=0&&which<=2);
	switch(which)
	{
	case eChamferEdgeAmount:
		return IBaseGrip::eUniverse;
	case eChamferEdgeSegments:
		return IBaseGrip::eInt;
	case eChamferEdgeOpen:
		return IBaseGrip::eToggle;
	}
	return IBaseGrip::eInt;
}

void ChamferEdgeGrip::GetGripName(TSTR &string)
{
	string =TSTR(GetString(IDS_CHAMFER));
}

bool ChamferEdgeGrip::GetText(int which,TSTR &string)
{
	switch(which)
	{
	case eChamferEdgeAmount:
		{
			string =TSTR(GetString(IDS_AMOUNT));// + TSTR(": ") + TSTR(str);
		}
		break;
	case eChamferEdgeSegments:
		{
			string =TSTR(GetString(IDS_SEGMENTS));//+ TSTR(": ") + TSTR(str);
		}
		break;
	case eChamferEdgeOpen:
		{
			string = TSTR(GetString(IDS_OPEN));
		}
		break;
	default:
		return false;

	}
	return true;
}

DWORD ChamferEdgeGrip::GetCustomization(int which)
{
	return 0;
}

bool ChamferEdgeGrip::GetResolvedIconName(int which,MSTR &string)
{
	switch(which)
	{
	case eChamferEdgeAmount:
		string = CHAMFER_EDGE_AMOUNT_PNG;
		break;
	case eChamferEdgeSegments:
		string =  CHAMFER_EDGE_SEGMENT_PNG;
		break;
	case eChamferEdgeOpen:
		string = CHAMFER_EDGE_OPEN_PNG;
		break;
	default:
		return false;
	}
	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

}

bool ChamferEdgeGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eChamferEdgeAmount:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_chamfer_edge, t) == TRUE ? true : false;
	case eChamferEdgeSegments:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_edge_chamfer_segments, t) == TRUE ? true : false;
	}
	return false;
}


bool ChamferEdgeGrip::GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &comboOptions)
{
	// have no radio options
	return false;
}


bool ChamferEdgeGrip::GetCommandIcon(int which, MSTR &string)
{
	return false;
}

bool ChamferEdgeGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{

	ParamDef theDef;
	switch(which)
	{
	case eChamferEdgeAmount:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_chamfer_edge);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = 0.0f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eChamferEdgeSegments:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_edge_chamfer_segments);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mInt = 1;
			maxRange.mInt = theDef.range_high.i;
		}
		break;
	default:
		return false;

	}
	return true;
}

bool ChamferEdgeGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eChamferEdgeAmount:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_chamfer_edge, t, value.mFloat,FOREVER);
		break;
	case eChamferEdgeSegments:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_edge_chamfer_segments, t, value.mInt,FOREVER);
		break;
	case eChamferEdgeOpen:
		{
			// need to go BOOL(int) to bool
            int val = 0;
		    GetEditPolyMod()->getParamBlock()->GetValue (epm_chamfer_edge_open, t, val,FOREVER);
			value.mbool = (val == 0 ? false: true);
		}
		break;
	default:
		return false;
	}
	return true;
}

bool ChamferEdgeGrip::SetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eChamferEdgeAmount:
		if( value.mFloat < 0.0f )
			value.mFloat = 0.0f;
		GetEditPolyMod()->getParamBlock()->SetValue (epm_chamfer_edge, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_chamfer_edge, t, value.mFloat,FOREVER);
		break;
	case eChamferEdgeSegments:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_edge_chamfer_segments, t, value.mInt);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_edge_chamfer_segments, t, value.mInt,FOREVER);
		break;
	case eChamferEdgeOpen:
		{
			int val =(int)value.mbool ;
			GetEditPolyMod()->getParamBlock()->SetValue (epm_chamfer_edge_open, t, val);
		    GetEditPolyMod()->getParamBlock()->GetValue (epm_chamfer_edge_open, t, val,FOREVER);
		}
		break;
	default:
		return false;

	}
	return true;
}

void ChamferEdgeGrip::Apply(TimeValue t)
{
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitAndRepeat (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void ChamferEdgeGrip::Okay(TimeValue t)
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitUnlessAnimating (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void ChamferEdgeGrip::Cancel()
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,0);
	GetEditPolyMod()->EpModCancel();
	EndSetValue(-1,0,true);
	GetEditPolyMod()->EpModRefreshScreen ();
}

bool ChamferEdgeGrip::StartSetValue(int ,TimeValue t)
{
	theHold.Begin ();
	/*	int type = GetEditPolyMod()->GetPolyOperationID ();
	if (type != ep_op_ChamferEdge) {
	GetEditPolyMod()->EpModSetOperation (ep_op_ChamferEdge);
	} 
	*/
	return true;
}

bool ChamferEdgeGrip::EndSetValue(int ,TimeValue t,bool accept)
{
	if (accept)
	{
		if(theHold.Holding())
			theHold.Accept (GetString(IDS_CHAMFER));

	}
	else
	{
		if(theHold.Holding())
			theHold.Cancel();
		GetEditPolyMod()->EpModRefreshScreen ();	
	}

	return true;
}



//we don't auto scale
bool ChamferEdgeGrip::GetAutoScale(int which)
{
	return true; //yes
}
//default scale
bool ChamferEdgeGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
	{
	case eChamferEdgeAmount:
		scaleValue.mFloat = 1.0f;
		break;
	case eChamferEdgeSegments:
		scaleValue.mInt = 1;
		break;
	default:
		return false;
	}
	return true;
}
bool ChamferEdgeGrip::GetScaleInViewSpace(int which, float &depth)
{
	depth = -200.0f; 
	switch(which)
	{
	case eChamferEdgeAmount:
		return true;
	}
	
	return false;
}
bool ChamferEdgeGrip::ResetValue(TimeValue t, int which)
{
    float fResetVal = 0.0f;
    int iResetSegVal = 1;
	int iResetVal = 0;

	switch(which)
	{
	case eChamferEdgeAmount:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_chamfer_edge, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_chamfer_edge, t, fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_CHAMFER));
		}
		break;
	case eChamferEdgeSegments:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_edge_chamfer_segments, t,iResetSegVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_edge_chamfer_segments, t,iResetSegVal,FOREVER);
			theHold.Accept (GetString(IDS_CHAMFER));

		}
		break;
	case eChamferEdgeOpen:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_chamfer_edge_open, t, iResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_chamfer_edge_open,t,iResetVal,FOREVER);
			theHold.Accept (GetString(IDS_CHAMFER));

		}
		break;
	default:
		return false;
	}
	return true;

}

bool ChamferEdgeGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{
	ParamBlockDesc2* pbd = GetEditPolyMod()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eChamferEdgeAmount:
		{
			const ParamDef& pd = pbd->paramdefs[epm_chamfer_edge];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eChamferEdgeSegments:
		{
			const ParamDef& pd = pbd->paramdefs[epm_edge_chamfer_segments];
			resetValue.mInt = pd.def.i;
		}
		break;
	case eChamferEdgeOpen:
		{
			const ParamDef& pd = pbd->paramdefs[epm_chamfer_edge_open];
			resetValue.mbool = (pd.def.i == 0 ? false:true) ;
		}
		break;
	default:
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bridge Edge Grips

BridgeEdgeGrip::~BridgeEdgeGrip()
{

}

int BridgeEdgeGrip::GetNumGripItems()
{
	return 7; //would be 5 for the old one since the old inluded okay and apply (remember cancel was implicit)
}
IBaseGrip::Type BridgeEdgeGrip::GetType(int which)
{
	DbgAssert(which>=0&&which<=6);
	switch(which)
	{
	case eBridgeEdgeSegments:
		return IBaseGrip::eInt;
	case eBridgeEdgeSmooth:
		return IBaseGrip::eFloat;
	case eBridgeEdgeAdjacent:
		return IBaseGrip::eFloat;
	case eBridgeEdgeReverseTri:
		return IBaseGrip::eToggle;
	case eBridgeEdgeType:
		return IBaseGrip::eCombo;
	case eBridgeEdgePick1:
		return IBaseGrip::eCommand;
	case eBridgeEdgePick2:
		return IBaseGrip::eCommand;
	}
	return IBaseGrip::eInt;
}

void BridgeEdgeGrip::GetGripName(TSTR &string)
{
	if(mEdge1Picked == TSTR(GetString(IDS_BRIDGE_PICKING_EDGE_1)))
		string =  TSTR(GetString(IDS_BRIDGE_PICKING_EDGE_1));
	else if (mEdge2Picked ==  TSTR(GetString(IDS_BRIDGE_PICKING_EDGE_2)))
		string = TSTR(GetString(IDS_BRIDGE_PICKING_EDGE_2));
	else
		string =TSTR(GetString(IDS_BRIDGE_EDGES));
}

void BridgeEdgeGrip::SetEdge1Picked(int num)
{
	TSTR buf;
	buf.printf(_T("%d"), num);
	mEdge1Picked = TSTR(GetString(IDS_EDGE)) + TSTR(" ") + buf;
	mPick1Icon = PICK_FULL_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgeEdgePick1);
}

void BridgeEdgeGrip::SetEdge2Picked(int num)
{
	TSTR buf;
	buf.printf(_T("%d"), num);
	mEdge2Picked = TSTR(GetString(IDS_EDGE)) + TSTR(" ") + buf;
	mPick2Icon = PICK_FULL_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgeEdgePick2);
}

void BridgeEdgeGrip::SetEdge1PickModeStarted()
{
	mEdge1Picked =  TSTR(GetString(IDS_BRIDGE_PICKING_EDGE_1));
	mPick1Icon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgeEdgePick1);
}

void BridgeEdgeGrip::SetEdge1PickDisabled()
{
	mEdge1Picked =  TSTR(GetString(IDS_BRIDGE_PICK_EDGE_1));
	mPick1Icon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgeEdgePick1);
}

void BridgeEdgeGrip::SetEdge2PickModeStarted()
{
	mEdge2Picked =  TSTR(GetString(IDS_BRIDGE_PICKING_EDGE_2));
	mPick2Icon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgeEdgePick2);
}

void BridgeEdgeGrip::SetEdge2PickDisabled()
{
	mEdge2Picked =  TSTR(GetString(IDS_BRIDGE_PICK_EDGE_2));
	mPick2Icon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgeEdgePick2);
}

bool BridgeEdgeGrip::GetText(int which,TSTR &string)
{
	switch(which)
	{
	
	case eBridgeEdgeSegments:
		{
			string =TSTR(GetString(IDS_SEGMENTS));// + TSTR(": ") + TSTR(str);
		}
		break;
	case eBridgeEdgeSmooth:
		{
			string =TSTR(GetString(IDS_SMOOTH));//+ TSTR(": ") + TSTR(str);
		}
		break;
	case eBridgeEdgeAdjacent:
		{
			string =TSTR(GetString(IDS_BRIDGE_ADJACENT));//+ TSTR(": ") + TSTR(str);
		}
		break;
	case eBridgeEdgeReverseTri:
		{
			string =TSTR(GetString(IDS_BRIDGE_REVERSE_TRI));//+ TSTR(": ") + TSTR(str);
		}
		break;
	case eBridgeEdgeType:
		{
			//Group Normals, 1 for Local Normals, 2 for By Polygon 
			int type;
			GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_selected, 0, type,FOREVER);
			switch(type)
			{
			case 0:
				string =  TSTR(GetString(IDS_BRIDGE_SPECIFIC_EDGES));
				break;
			case 1:
				string =  TSTR(GetString(IDS_BRIDGE_EDGE_SELECTION));
				break;

			}
			break;
		}
	case eBridgeEdgePick1:
		{
			string = mEdge1Picked;
		}
		break;
	case eBridgeEdgePick2:
		{
			string = mEdge2Picked;
		}
		break;
	default:
		return false;
	}
	return true;
}

DWORD BridgeEdgeGrip::GetCustomization(int which)
{
	DWORD word =0;
	switch(which)
	{

	case eBridgeEdgeSmooth:
	case eBridgeEdgeReverseTri:
	case eBridgeEdgePick1:
	case eBridgeEdgePick2:
		word = (int)(IBaseGrip::eSameRow);
		break;
	}
	return word;
}

bool BridgeEdgeGrip::GetResolvedIconName(int which,MSTR &string)
{
	switch(which)
	{
	case eBridgeEdgeType:
		string = BRIDGE_EDGE_TYPE_PNG;
		break;
	case eBridgeEdgeSegments:
		string =  BRIDGE_SEGMENTS_PNG;
		break;
	case eBridgeEdgeSmooth:
		string =  BRIDGE_SMOOTH_PNG;
		break;
	case eBridgeEdgeAdjacent:
		string = BRIDGE_ADJACENT_PNG;
        break;
	case eBridgeEdgePick1:
		string = BRIDGE_EDGE_PICK1_PNG;
		break;
	case eBridgeEdgePick2:
		string = BRIDGE_EDGE_PICK2_PNG;
		break;
	case eBridgeEdgeReverseTri:
			string =BRIDGE_REVERSE_TRI_PNG;
		break;
	default:
		return false;

	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

}

bool BridgeEdgeGrip::ShowKeyBrackets(int which,TimeValue t)
{

	switch(which)
	{
	case eBridgeEdgeSegments:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_bridge_segments, t) == TRUE ? true : false;
	case eBridgeEdgeSmooth:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_bridge_smooth_thresh, t) == TRUE ? true : false;
	case eBridgeEdgeAdjacent:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_bridge_adjacent, t) == TRUE ? true : false;
	}
	return false;

}


bool BridgeEdgeGrip::GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &comboOptions)
{
	if(which== eBridgeEdgeType)
	{
		BOOL found;
		IBaseGrip::ComboLabel *label = new IBaseGrip::ComboLabel();
		TSTR string = BRIDGE_SPECIFIC_PNG;
		found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
		if(found)
			label->mIcon = string;
		label->mLabel = TSTR(GetString(IDS_BRIDGE_SPECIFIC_EDGES));
		comboOptions.Append(1,&label);
		label = new IBaseGrip::ComboLabel();
		string = BRIDGE_SELECTED_PNG;
		found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
		if(found)
			label->mIcon = string;
		label->mLabel = TSTR(GetString(IDS_BRIDGE_EDGE_SELECTION));
		comboOptions.Append(1,&label);
		return true;
	}
	return false;
}

bool BridgeEdgeGrip::GetCommandIcon(int which, MSTR &string)
{
	switch(which)
	{
	case eBridgeEdgePick1:
		string = mPick1Icon;
		break;
	case eBridgeEdgePick2:
		string = mPick2Icon;
		break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return found ? true : false;
}

bool BridgeEdgeGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{

	ParamDef theDef;
	switch(which)
	{
	case eBridgeEdgeSegments:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_bridge_segments);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mInt = 1;
			maxRange.mInt = theDef.range_high.i;
		}
		break;

	case eBridgeEdgeSmooth:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_bridge_smooth_thresh);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eBridgeEdgeAdjacent:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_bridge_adjacent);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;
	default:
		return false;
	}
	return true;
}

bool BridgeEdgeGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{

	switch(which)
	{
	case eBridgeEdgeType:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_selected, t, value.mInt,FOREVER);
		break;
	case eBridgeEdgeSegments:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_segments, t, value.mInt,FOREVER);
		break;
	case eBridgeEdgeSmooth:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_smooth_thresh, t, value.mFloat,FOREVER);
		break;
	case eBridgeEdgeAdjacent:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_adjacent, t, value.mFloat,FOREVER);
		break;
	case eBridgeEdgeReverseTri:
		{
			int val =0;
			GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_reverse_triangle, t, val,FOREVER);
			value.mbool = (val == 0 ? false:true);
		}
		break;
	case eBridgeEdgePick1:
		{
			//value.mbool, 0 means not in pick mode, 1 does.
			if(GetEditPolyMod()->EpModGetCommandMode() == ep_mode_pick_bridge_1) // in command mode
				value.mbool = true;
			else
				value.mbool = false;
		}
		break;
	case eBridgeEdgePick2:
		{
			if(GetEditPolyMod()->EpModGetCommandMode()==ep_mode_pick_bridge_2)  // in command mode
				value.mbool = true;
			else
				value.mbool = false;
		}
		break;
	default:
		return false;
	}
	return true;
}

//need to activate grips
void BridgeEdgeGrip::SetUpUI()
{
	int val;
	GetEditPolyMod()->getParamBlock()->GetValue(epm_bridge_selected, GetCOREInterface()->GetTime(),val,FOREVER);
	if( 1 == val )
	{
		GetIGripManager()->ActivateGripItem(eBridgeEdgePick1,false);
		GetIGripManager()->ActivateGripItem(eBridgeEdgePick2,false);
		SetEdge1PickDisabled();
		SetEdge2PickDisabled();
	}
	else
	{
		GetIGripManager()->ActivateGripItem(eBridgeEdgePick1,true);
		GetIGripManager()->ActivateGripItem(eBridgeEdgePick2,true);
		int targ = GetEditPolyMod()->getParamBlock ()->GetInt (epm_bridge_target_1);
		if(targ==0)
			SetEdge1PickDisabled();
		else
			SetEdge1Picked(targ-1); //need to -1 out
		targ = GetEditPolyMod()->getParamBlock()->GetInt (epm_bridge_target_2);
		if(targ==0)
			SetEdge2PickDisabled();
		else
			SetEdge2Picked(targ-1); //need to -1 out
	}
	GetIGripManager()->ResetAllUI();
}

bool BridgeEdgeGrip::SetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eBridgeEdgeType:
		if(value.mInt>1) //go back to zero
			value.mInt =0;
		if(value.mInt<0)
			value.mInt = 1;
		if( 1 == value.mInt )
		{
			GetIGripManager()->ActivateGripItem(eBridgeEdgePick1, false);
			GetIGripManager()->ActivateGripItem(eBridgeEdgePick2, false);
		}
		else
		{
			GetIGripManager()->ActivateGripItem(eBridgeEdgePick1, true);
			GetIGripManager()->ActivateGripItem(eBridgeEdgePick2, true);
		}
		GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_selected, t, value.mInt);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_selected, t, value.mInt,FOREVER);
		break;
	case eBridgeEdgeSegments:
		if( value.mInt < 1)
			value.mInt = 1;
		GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_segments, t, value.mInt);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_segments, t, value.mInt,FOREVER);
		break;
	case eBridgeEdgeSmooth:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_smooth_thresh, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_smooth_thresh, t, value.mFloat,FOREVER);
		break;
	case eBridgeEdgeAdjacent:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_adjacent, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_adjacent, t, value.mFloat,FOREVER);
		break;
	case eBridgeEdgeReverseTri:
		{
		    int val = (int)value.mbool ;
			GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_reverse_triangle, t, val);
		    GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_reverse_triangle, t, val,FOREVER);
		}
		break;
	case eBridgeEdgePick1:
		{
			//value.mbool now tells us to turn command mode, on or off
			//so we check and see if we are or aren't in the state
			if(GetEditPolyMod()->EpModGetCommandMode()==ep_mode_pick_bridge_1) //in command mode 
			{
				if(!value.mbool)
				{
					GetEditPolyMod()->EpModToggleCommandMode(ep_mode_pick_bridge_1);
					SetEdge1PickDisabled();
					//also set the text!
				}
			}
			else if(value.mbool) //not in that mode so toggle it on if told to
			{
				GetEditPolyMod()->EpModToggleCommandMode(ep_mode_pick_bridge_1);
				SetEdge1PickModeStarted();
			}
		}
		break;
	case eBridgeEdgePick2:
		{
			//value.mbool now tells us to turn command mode, on or off
			//so we check and see if we are or aren't in the state
			if(GetEditPolyMod()->EpModGetCommandMode()==ep_mode_pick_bridge_2) //in command mode 
			{
				if(!value.mbool)
				{
					GetEditPolyMod()->EpModToggleCommandMode(ep_mode_pick_bridge_2);
					SetEdge2PickDisabled();
					//also set the text!
				}
			}
			else if(value.mbool) //not in that mode so toggle it on if told to
			{
				GetEditPolyMod()->EpModToggleCommandMode(ep_mode_pick_bridge_2);
				SetEdge2PickModeStarted();
			}
		}
		break;
	default:
		return false;

	}
	return true;
}

void BridgeEdgeGrip::Apply(TimeValue t)
{
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitAndRepeat (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void BridgeEdgeGrip::Okay(TimeValue t)
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitUnlessAnimating (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void BridgeEdgeGrip::Cancel()
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,0);
	GetEditPolyMod()->EpModCancel();
	EndSetValue(-1,0,true);
	GetEditPolyMod()->EpModRefreshScreen ();
}

bool BridgeEdgeGrip::StartSetValue(int ,TimeValue t)
{
	theHold.Begin ();
	/*	int type = GetEditPolyMod()->GetPolyOperationID ();
	if (type != ep_op_BridgeEdge) {
	GetEditPolyMod()->EpModSetOperation (ep_op_BridgeEdge);
	} 
	*/
	return true;
}

bool BridgeEdgeGrip::EndSetValue(int ,TimeValue t,bool accept)
{
	if (accept)
	{
		if(theHold.Holding())
			theHold.Accept (GetString(IDS_BRIDGE_EDGES));

	}
	else
	{
		if(theHold.Holding())
			theHold.Cancel();
		GetEditPolyMod()->EpModRefreshScreen ();	
	}

	return true;
}



//we don't auto scale
bool BridgeEdgeGrip::GetAutoScale(int which)
{
	return true; //yes
}
//default scale
bool BridgeEdgeGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
	{
	case eBridgeEdgeSegments:
		scaleValue.mInt = 1;
		break;
	case eBridgeEdgeSmooth:
		scaleValue.mFloat = 1.0f;
		break;
	case eBridgeEdgeAdjacent:
		scaleValue.mFloat = 1.0f;
		break;
	default:
		return false;
	}
	return true;
}
bool BridgeEdgeGrip::GetScaleInViewSpace(int which, float &depth)
{
	depth = -200.0f; 
	return false;
}
bool BridgeEdgeGrip::ResetValue(TimeValue t, int which)
{
	float fResetVal = 0.0f;
	int iResetSegVal =1;
	int iResetVal = 0;

	switch(which)
	{
	case eBridgeEdgeType:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_selected, t, iResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_bridge_selected, t, iResetVal,FOREVER);
			theHold.Accept (GetString(IDS_BRIDGE_EDGES));
		}
		break;
	case eBridgeEdgeSegments:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_segments, t,iResetSegVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_bridge_segments, t, iResetSegVal,FOREVER);
			theHold.Accept (GetString(IDS_BRIDGE_EDGES));
		}
		break;
	case eBridgeEdgeSmooth:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_smooth_thresh, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_bridge_smooth_thresh, t, fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_BRIDGE_EDGES));

		}
		break;

	case eBridgeEdgeAdjacent:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_adjacent, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_bridge_adjacent, t, fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_BRIDGE_EDGES));

		}
		break;

	case eBridgeEdgeReverseTri:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_reverse_triangle, t, iResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_bridge_reverse_triangle, t, iResetVal,FOREVER);
			theHold.Accept (GetString(IDS_BRIDGE_EDGES));

		}
		break;
	default:
		return false;

	}
	return true;

}

bool BridgeEdgeGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{
	ParamBlockDesc2* pbd = GetEditPolyMod()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eBridgeEdgeType:
		resetValue.mInt = 0;
		break;
	case eBridgeEdgeSegments:
		{
			const ParamDef& pd = pbd->paramdefs[epm_bridge_segments];
			resetValue.mInt = pd.def.i;
		}
		break;
	case eBridgeEdgeSmooth:
		{
			const ParamDef& pd = pbd->paramdefs[epm_bridge_smooth_thresh];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eBridgeEdgeAdjacent:
		{
			const ParamDef& pd = pbd->paramdefs[epm_bridge_adjacent];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eBridgeEdgeReverseTri:
		{
			const ParamDef& pd = pbd->paramdefs[epm_bridge_reverse_triangle];
			resetValue.mbool = (pd.def.i == 0 ? false:true);
		}
		break;
	default:
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bridge Border Grips

BridgeBorderGrip::~BridgeBorderGrip()
{

}

int BridgeBorderGrip::GetNumGripItems()
{
	return 9; //would be 5 for the old one since the old inluded okay and apply (remember cancel was implicit)
}
IBaseGrip::Type BridgeBorderGrip::GetType(int which)
{
	DbgAssert(which>=0&&which<=8);
	switch(which)
	{
	case eBridgeBorderType:
		return IBaseGrip::eCombo;
	case eBridgeBorderSegments:
		return IBaseGrip::eInt;
	case eBridgeBorderTaper:
		return IBaseGrip::eFloat;
	case eBridgeBorderBias:
		return IBaseGrip::eFloat;
	case eBridgeBorderSmooth:
		return IBaseGrip::eFloat;
	case eBridgeBorderTwist1:
		return IBaseGrip::eInt;
	case eBridgeBorderTwist2:
		return IBaseGrip::eInt;
	case eBridgeBorderPick1:
		return IBaseGrip::eCommand;
	case eBridgeBorderPick2:
		return IBaseGrip::eCommand;
	}
	return IBaseGrip::eInt;
}

void BridgeBorderGrip::GetGripName(TSTR &string)
{
	if(mEdge1Picked == TSTR(GetString(IDS_BRIDGE_PICKING_BORDER_1)))
		string = TSTR(GetString(IDS_BRIDGE_PICKING_BORDER_1));
	else if(mEdge2Picked ==  TSTR(GetString(IDS_BRIDGE_PICKING_BORDER_2)))
		string =  TSTR(GetString(IDS_BRIDGE_PICKING_BORDER_2));
	else
		string =TSTR(GetString(IDS_BRIDGE_BORDERS));
}

void BridgeBorderGrip::SetEdge1Picked(int num)
{
	TSTR buf;
	buf.printf(_T("%d"), num);
	mEdge1Picked = TSTR(GetString(IDS_EDGE)) + TSTR(" ") + buf;
	mPick1Icon = PICK_FULL_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgeBorderPick1);
}

void BridgeBorderGrip::SetEdge2Picked(int num)
{
	TSTR buf;
	buf.printf(_T("%d"), num);
	mEdge2Picked = TSTR(GetString(IDS_EDGE)) + TSTR(" ") + buf;
	mPick2Icon = PICK_FULL_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgeBorderPick2);
}

void BridgeBorderGrip::SetEdge1PickModeStarted()
{
	mEdge1Picked =  TSTR(GetString(IDS_BRIDGE_PICKING_BORDER_1));
	mPick1Icon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgeBorderPick1);
}

void BridgeBorderGrip::SetEdge1PickDisabled()
{
	mEdge1Picked =  TSTR(GetString(IDS_BRIDGE_BORDER_PICK_1));
	mPick1Icon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgeBorderPick1);
}

void BridgeBorderGrip::SetEdge2PickModeStarted()
{
	mEdge2Picked =  TSTR(GetString(IDS_BRIDGE_PICKING_BORDER_2));
	mPick2Icon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgeBorderPick2);
}

void BridgeBorderGrip::SetEdge2PickDisabled()
{
	mEdge2Picked =  TSTR(GetString(IDS_BRIDGE_BORDER_PICK_2));
	mPick2Icon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgeBorderPick2);
}

bool BridgeBorderGrip::GetText(int which,TSTR &string)
{
	switch(which)
	{
	case eBridgeBorderType:
		{
			//Group Normals, 1 for Local Normals, 2 for By Polygon 
			int type;
			GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_selected, 0, type,FOREVER);
			switch(type)
			{
			case 0:
				string =  TSTR(GetString(IDS_BRIDGE_SPECIFIC_BORDERS));
				break;
			case 1:
				string =  TSTR(GetString(IDS_BRIDGE_BORDER_SELECTION));
				break;

			}
			break;
		}
	case eBridgeBorderSegments:
		{
			string =TSTR(GetString(IDS_SEGMENTS));// + TSTR(": ") + TSTR(str);
		}
		break;
	case eBridgeBorderTaper:
		{
			string =TSTR(GetString(IDS_TAPER));//+ TSTR(": ") + TSTR(str);
		}
		break;
	case eBridgeBorderBias:
		{
			string =TSTR(GetString(IDS_BIAS));//+ TSTR(": ") + TSTR(str);
		}
		break;
	case eBridgeBorderSmooth:
		{
			string =TSTR(GetString(IDS_SMOOTH));//+ TSTR(": ") + TSTR(str);
		}
		break;
	case eBridgeBorderTwist1:
		{
			string =TSTR(GetString(IDS_TWIST1));//+ TSTR(": ") + TSTR(str);
		}
		break;
	case eBridgeBorderTwist2:
		{
			string =TSTR(GetString(IDS_TWIST2));//+ TSTR(": ") + TSTR(str);
		}
		break;
	case eBridgeBorderPick1:
		{
			string = mEdge1Picked;
		}
		break;
	case eBridgeBorderPick2:
		{
			string = mEdge2Picked;
		}
		break;
	default:
		return false;

	}
	return true;
}

DWORD BridgeBorderGrip::GetCustomization(int which)
{
	DWORD word=0;
	switch(which)
	{
	case eBridgeBorderTaper:
	case eBridgeBorderSmooth:
	case eBridgeBorderTwist2:
	case eBridgeBorderPick1:
	case eBridgeBorderPick2:
		word = (int)(IBaseGrip::eSameRow);
	}
	return word;
}

bool BridgeBorderGrip::GetResolvedIconName(int which,MSTR &string)
{
	switch(which)
	{
	case eBridgeBorderType:
		string = BRIDGE_BORDER_TYPE_PNG;
		break;
	case eBridgeBorderSegments:
		string =  BRIDGE_SEGMENTS_PNG;
		break;
	case eBridgeBorderTaper:
		string =  BRIDGE_TAPER_PNG;
		break;
	case eBridgeBorderBias:
		string =  BRIDGE_BIAS_PNG;
		break;
	case eBridgeBorderSmooth:
		string =  BRIDGE_SMOOTH_PNG;
		break;
	case eBridgeBorderTwist1:
		string =  BRIDGE_TWIST1_PNG;
		break;
	case eBridgeBorderTwist2:
		string =  BRIDGE_TWIST2_PNG;
		break;
	case eBridgeBorderPick1:
		string = BRIDGE_BORDER_PICK1_PNG;
		break;
	case eBridgeBorderPick2:
		string = BRIDGE_BORDER_PICK2_PNG;
		break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

}

bool BridgeBorderGrip::ShowKeyBrackets(int which,TimeValue t)
{

	switch(which)
	{
	case eBridgeBorderSegments:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_bridge_segments, t) == TRUE ? true : false;
	case eBridgeBorderTaper:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_bridge_taper, t) == TRUE ? true : false;
	case eBridgeBorderBias:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_bridge_bias, t) == TRUE ? true : false;
	case eBridgeBorderSmooth:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_bridge_smooth_thresh, t) == TRUE ? true : false;
	case eBridgeBorderTwist1:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_bridge_twist_1, t) == TRUE ? true : false;
	case eBridgeBorderTwist2:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_bridge_twist_2, t) == TRUE ? true : false;

	}
	return false;

}


bool BridgeBorderGrip::GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &comboOptions)
{
	if(which== eBridgeBorderType)
	{
		BOOL found;
		IBaseGrip::ComboLabel *label = new IBaseGrip::ComboLabel();
		TSTR string = BRIDGE_SPECIFIC_PNG;
		found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
		if(found)
			label->mIcon = string;
		label->mLabel = TSTR(GetString(IDS_BRIDGE_SPECIFIC_BORDERS));
		comboOptions.Append(1,&label);
		label = new IBaseGrip::ComboLabel();
		string = BRIDGE_SELECTED_PNG;
		found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
		if(found)
			label->mIcon = string;
		label->mLabel = TSTR(GetString(IDS_BRIDGE_BORDER_SELECTION));
		comboOptions.Append(1,&label);
		return true;
	}
	return false;
}

bool  BridgeBorderGrip::GetCommandIcon(int which, MSTR &string)
{
	switch(which)
	{
	case eBridgeBorderPick1:
		string = mPick1Icon;
		break;
	case eBridgeBorderPick2:
		string = mPick2Icon;
		break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return found ? true : false;
}

bool BridgeBorderGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{

	ParamDef theDef;
	switch(which)
	{
	case eBridgeBorderSegments:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_bridge_segments);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mInt = 1;
			maxRange.mInt = theDef.range_high.i;
		}
		break;

	case eBridgeBorderTaper:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_bridge_taper);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eBridgeBorderBias:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_bridge_bias);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eBridgeBorderSmooth:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_bridge_smooth_thresh);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eBridgeBorderTwist1:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_bridge_twist_1);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mInt = theDef.range_low.i;
			maxRange.mInt = theDef.range_high.i;
		}
		break;

	case eBridgeBorderTwist2:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_bridge_twist_2);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mInt = theDef.range_low.i;
			maxRange.mInt = theDef.range_high.i;
		}
		break;
	default:
		return false;

	}
	return true;
}

bool BridgeBorderGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eBridgeBorderType:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_selected, t, value.mInt,FOREVER);
		break;
	case eBridgeBorderSegments:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_segments, t, value.mInt,FOREVER);
		break;
	case eBridgeBorderTaper:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_taper, t, value.mFloat,FOREVER);
		break;
	case eBridgeBorderBias:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_bias, t, value.mFloat,FOREVER);
		break;
	case eBridgeBorderSmooth:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_smooth_thresh, t, value.mFloat,FOREVER);
		break;
	case eBridgeBorderTwist1:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_twist_1, t, value.mInt,FOREVER);
		break;
	case eBridgeBorderTwist2:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_twist_2, t, value.mInt,FOREVER);
		break;
	case eBridgeBorderPick1:
		{
			//value.mbool, 0 means not in pick mode, 1 does.
			if(GetEditPolyMod()->EpModGetCommandMode()==ep_mode_pick_bridge_1) // in command mode
				value.mbool = true;
			else
				value.mbool = false;
		}
		break;
	case eBridgeBorderPick2:
		{
			if(GetEditPolyMod()->EpModGetCommandMode()==ep_mode_pick_bridge_2) // in command mode
				value.mbool = true;
			else
				value.mbool = false;
		}
		break;
	default:
		return false;
	}
	return true;
}

//need to activate grips
void BridgeBorderGrip::SetUpUI()
{
	int val;
	GetEditPolyMod()->getParamBlock()->GetValue(epm_bridge_selected,GetCOREInterface()->GetTime(),val,FOREVER);
	if( 1 == val )
	{
		GetIGripManager()->ActivateGripItem(eBridgeBorderPick1,false);
		GetIGripManager()->ActivateGripItem(eBridgeBorderPick2,false);
		SetEdge1PickDisabled();
		SetEdge2PickDisabled();
	}
	else
	{
		GetIGripManager()->ActivateGripItem(eBridgeBorderPick1,true);
		GetIGripManager()->ActivateGripItem(eBridgeBorderPick2,true);
		int targ = GetEditPolyMod()->getParamBlock ()->GetInt (epm_bridge_target_1);
		if(targ==0)
			SetEdge1PickDisabled();
		else
			SetEdge1Picked(targ-1); //need to -1 out
		targ = GetEditPolyMod()->getParamBlock()->GetInt (epm_bridge_target_2);
		if(targ==0)
			SetEdge2PickDisabled();
		else
			SetEdge2Picked(targ-1); //need to -1 out
	}
	GetIGripManager()->ResetAllUI();
}


bool BridgeBorderGrip::SetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eBridgeBorderType:
		if(value.mInt>1) //go back to zero
			value.mInt =0;
		if(value.mInt<0)
			value.mInt = 1;
		if( 1 == value.mInt )
		{
			GetIGripManager()->ActivateGripItem(eBridgeBorderPick1, false);
			GetIGripManager()->ActivateGripItem(eBridgeBorderPick2, false);
		}
		else
		{
			GetIGripManager()->ActivateGripItem(eBridgeBorderPick1, true);
			GetIGripManager()->ActivateGripItem(eBridgeBorderPick2, true);
		}
		GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_selected, t, value.mInt);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_selected, t, value.mInt,FOREVER);
		break;
	case eBridgeBorderSegments:
		if( value.mInt < 1)
			value.mInt = 1;
		GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_segments, t, value.mInt);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_segments, t, value.mInt,FOREVER);
		break;
	case eBridgeBorderTaper:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_taper, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_taper, t, value.mFloat,FOREVER);
		break;
	case eBridgeBorderBias:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_bias, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_bias, t, value.mFloat,FOREVER);
		break;
	case eBridgeBorderSmooth:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_smooth_thresh, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_smooth_thresh, t, value.mFloat,FOREVER);
		break;
	case eBridgeBorderTwist1:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_twist_1, t, value.mInt);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_twist_1, t, value.mInt,FOREVER);
		break;
	case eBridgeBorderTwist2:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_twist_2, t, value.mInt);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_twist_2, t, value.mInt,FOREVER);
		break;
	case eBridgeBorderPick1:
		{
			//value.mbool now tells us to turn command mode, on or off
			//so we check and see if we are or aren't in the state
			if(GetEditPolyMod()->EpModGetCommandMode()==ep_mode_pick_bridge_1) // in command mode
			{
				if(!value.mbool)
				{
					GetEditPolyMod()->EpModToggleCommandMode(ep_mode_pick_bridge_1);
					SetEdge1PickDisabled();
					//also set the text!
				}
			}
			else if(value.mbool) //not in that mode so toggle it on if told to
			{
				GetEditPolyMod()->EpModToggleCommandMode(ep_mode_pick_bridge_1);
				SetEdge1PickModeStarted();
			}
		}
		break;
	case eBridgeBorderPick2:
		{
			//value.mbool now tells us to turn command mode, on or off
			//so we check and see if we are or aren't in the state
			if(GetEditPolyMod()->EpModGetCommandMode()==ep_mode_pick_bridge_2) // in command mode
			{
				if(!value.mbool)
				{
					GetEditPolyMod()->EpModToggleCommandMode(ep_mode_pick_bridge_2);
					SetEdge2PickDisabled();
				}
			}
			else if(value.mbool) //not in that mode so toggle it on if told to
			{
				GetEditPolyMod()->EpModToggleCommandMode(ep_mode_pick_bridge_2);
				SetEdge2PickModeStarted();
			}
		}
		break;
	default:
		return false;
	}
	return true;
}

void BridgeBorderGrip::Apply(TimeValue t)
{
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitAndRepeat (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void BridgeBorderGrip::Okay(TimeValue t)
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitUnlessAnimating (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void BridgeBorderGrip::Cancel()
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,0);
	GetEditPolyMod()->EpModCancel();
	EndSetValue(-1,0,true);
	GetEditPolyMod()->EpModRefreshScreen ();
}

bool BridgeBorderGrip::StartSetValue(int ,TimeValue t)
{
	theHold.Begin ();
	/*	int type = GetEditPolyMod()->GetPolyOperationID ();
	if (type != ep_op_BridgeBorder) {
	GetEditPolyMod()->EpModSetOperation (ep_op_BridgeBorder);
	} 
	*/
	return true;
}

bool BridgeBorderGrip::EndSetValue(int ,TimeValue t,bool accept)
{
	if (accept)
	{
		if(theHold.Holding())
			theHold.Accept (GetString(IDS_BRIDGE_BORDERS));

	}
	else
	{
		if(theHold.Holding())
			theHold.Cancel();
		GetEditPolyMod()->EpModRefreshScreen ();	
	}

	return true;
}



//we don't auto scale
bool BridgeBorderGrip::GetAutoScale(int which)
{
	return true; //yes
}
//default scale
bool BridgeBorderGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
	{
	case eBridgeBorderSegments:
		scaleValue.mInt = 1;
		break;
	case eBridgeBorderTaper:
		scaleValue.mFloat = 1.0f;
		break;
	case eBridgeBorderBias:
		scaleValue.mFloat = 1.0f;
		break;
	case eBridgeBorderSmooth:
		scaleValue.mFloat = 1.0f;
		break;
	case eBridgeBorderTwist1:
		scaleValue.mInt = 1;
		break;
	case eBridgeBorderTwist2:
		scaleValue.mInt = 1;
		break;
	default:
		return false;
	}
	return true;
}
bool BridgeBorderGrip::GetScaleInViewSpace(int which, float &depth)
{
	depth = -200.0f; 
	return false;
}
bool BridgeBorderGrip::ResetValue(TimeValue t, int which)
{
	float fResetVal = 0.0f;
     int iResetSegVal = 1;
	 int iResetTwistVal = 0;
	 int iResetVal = 0;

	switch(which)
	{
	case eBridgeBorderType:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_selected, t, iResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_bridge_selected, t, iResetVal,FOREVER);
			theHold.Accept (GetString(IDS_BRIDGE_BORDERS));
		}
		break;
	case eBridgeBorderSegments:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_segments, t,iResetSegVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_bridge_segments, t, iResetSegVal,FOREVER);
			theHold.Accept (GetString(IDS_BRIDGE_BORDERS));
		}
		break;
	case eBridgeBorderTaper:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_taper, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_bridge_taper, t, fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_BRIDGE_BORDERS));
		}
		break;
	case eBridgeBorderBias:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_bias, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_bridge_bias, t, fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_BRIDGE_BORDERS));
		}
		break;
	case eBridgeBorderSmooth:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_smooth_thresh, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_bridge_smooth_thresh, t, fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_BRIDGE_BORDERS));
		}
		break;
	case eBridgeBorderTwist1:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_twist_1, t,iResetTwistVal);
		    GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_bridge_twist_1, t, iResetTwistVal,FOREVER);
			theHold.Accept (GetString(IDS_BRIDGE_BORDERS));
		}
		break;
	case eBridgeBorderTwist2:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_twist_2, t,iResetTwistVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_bridge_twist_2, t, iResetTwistVal,FOREVER);
			theHold.Accept (GetString(IDS_BRIDGE_BORDERS));
		}
		break;
	default:
		return false;
	}
	return true;

}

bool BridgeBorderGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{

	ParamBlockDesc2* pbd = GetEditPolyMod()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eBridgeBorderType:
		resetValue.mInt = 0;
		break;
	case eBridgeBorderSegments:
		{
			const ParamDef& pd = pbd->paramdefs[epm_bridge_segments];
			resetValue.mInt = pd.def.i;
		}
		break;
	case eBridgeBorderTaper:
		{
			const ParamDef& pd = pbd->paramdefs[epm_bridge_taper];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eBridgeBorderBias:
		{
			const ParamDef& pd = pbd->paramdefs[epm_bridge_bias];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eBridgeBorderSmooth:
		{
			const ParamDef& pd = pbd->paramdefs[epm_bridge_smooth_thresh];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eBridgeBorderTwist1:
		{
			const ParamDef& pd = pbd->paramdefs[epm_bridge_twist_1];
			resetValue.mInt = pd.def.i;
		}
		break;
	case eBridgeBorderTwist2:
		{
			const ParamDef& pd = pbd->paramdefs[epm_bridge_twist_2];
			resetValue.mInt = pd.def.i;
		}
		break;
	default:
		return false;

	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bridge Polygon Grips

BridgePolygonGrip::~BridgePolygonGrip()
{

}

int BridgePolygonGrip::GetNumGripItems()
{
	return 9; //would be 5 for the old one since the old inluded okay and apply (remember cancel was implicit)
}
IBaseGrip::Type BridgePolygonGrip::GetType(int which)
{
	DbgAssert(which>=0&&which<=8);
	switch(which)
	{
	case eBridgePolygonType:
		return IBaseGrip::eCombo;
	case eBridgePolygonSegments:
		return IBaseGrip::eInt;
	case eBridgePolygonTaper:
		return IBaseGrip::eFloat;
	case eBridgePolygonBias:
		return IBaseGrip::eFloat;
	case eBridgePolygonSmooth:
		return IBaseGrip::eFloat;
	case eBridgePolygonTwist1:
		return IBaseGrip::eInt;
	case eBridgePolygonTwist2:
		return IBaseGrip::eInt;
	case eBridgePolygonPick1:
		return IBaseGrip::eCommand;
	case eBridgePolygonPick2:
		return IBaseGrip::eCommand;
	}
	return IBaseGrip::eInt;
}

void BridgePolygonGrip::GetGripName(TSTR &string)
{
	
	if(mPoly1Picked == TSTR(GetString(IDS_BRIDGE_PICKING_POLYGON_1)))
		string = TSTR(GetString(IDS_BRIDGE_PICKING_POLYGON_1));
	else if(mPoly2Picked == TSTR(GetString(IDS_BRIDGE_PICKING_POLYGON_2)))
		string = TSTR(GetString(IDS_BRIDGE_PICKING_POLYGON_2));
	else
		string =TSTR(GetString(IDS_BRIDGE_POLYGONS));
}

void BridgePolygonGrip::SetPoly1Picked(int num)
{
	TSTR buf;
	buf.printf(_T("%d"), num);
	mPoly1Picked = TSTR(GetString(IDS_FACE)) + TSTR(" ") + buf;
	mPick1Icon = PICK_FULL_PNG;
 	GetIGripManager()->ResetGripUI((int)eBridgePolygonPick1);
}

void BridgePolygonGrip::SetPoly2Picked(int num)
{
	TSTR buf;
	buf.printf(_T("%d"), num);
	mPoly2Picked = TSTR(GetString(IDS_FACE)) + TSTR(" ") + buf;
	mPick2Icon = PICK_FULL_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgePolygonPick2);
}

void BridgePolygonGrip::SetPoly1PickModeStarted()
{
	mPoly1Picked =  TSTR(GetString(IDS_BRIDGE_PICKING_POLYGON_1));
	mPick1Icon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgePolygonPick1);
}

void BridgePolygonGrip::SetPoly1PickDisabled()
{
	mPoly1Picked =  TSTR(GetString(IDS_BRIDGE_PICK_POLYGON_1));
	mPick1Icon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgePolygonPick1);
}

void BridgePolygonGrip::SetPoly2PickModeStarted()
{
	mPoly2Picked =  TSTR(GetString(IDS_BRIDGE_PICKING_POLYGON_2));
	mPick2Icon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgePolygonPick2);
}

void BridgePolygonGrip::SetPoly2PickDisabled()
{
	mPoly2Picked =  TSTR(GetString(IDS_BRIDGE_PICK_POLYGON_2));
	mPick2Icon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgePolygonPick2);
}

bool BridgePolygonGrip::GetText(int which,TSTR &string)
{
	switch(which)
	{
	case eBridgePolygonType:
		{
			//Group Normals, 1 for Local Normals, 2 for By Polygon 
			int type;
			GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_selected, 0, type,FOREVER);
			switch(type)
			{
			case 0:
				string =  TSTR(GetString(IDS_BRIDGE_SPECIFIC_POLYGONS));
				break;
			case 1:
				string =  TSTR(GetString(IDS_BRIDGE_POLYGON_SELECTION));
				break;

			}
			break;
		}
	case eBridgePolygonSegments:
		{
			string =TSTR(GetString(IDS_SEGMENTS));// + TSTR(": ") + TSTR(str);
		}
		break;
	case eBridgePolygonTaper:
		{
			string =TSTR(GetString(IDS_TAPER));//+ TSTR(": ") + TSTR(str);
		}
		break;
	case eBridgePolygonBias:
		{
			string =TSTR(GetString(IDS_BIAS));//+ TSTR(": ") + TSTR(str);
		}
		break;
	case eBridgePolygonSmooth:
		{
			string =TSTR(GetString(IDS_SMOOTH));//+ TSTR(": ") + TSTR(str);
		}
		break;
	case eBridgePolygonTwist1:
		{
			string =TSTR(GetString(IDS_TWIST1));//+ TSTR(": ") + TSTR(str);
		}
		break;
	case eBridgePolygonTwist2:
		{
			string =TSTR(GetString(IDS_TWIST2));//+ TSTR(": ") + TSTR(str);
		}
		break;
	case eBridgePolygonPick1:
		{
			string =  mPoly1Picked;
		}
		break;
	case eBridgePolygonPick2:
		{
			string =  mPoly2Picked;
		}
		break;
	default:
		return false;
	}
	return true;
}

DWORD BridgePolygonGrip::GetCustomization(int which)
{
	DWORD word=0;
	switch(which)
	{
	case eBridgePolygonTaper:
	case eBridgePolygonSmooth:
	case eBridgePolygonTwist2:
	case eBridgePolygonPick1:
	case eBridgePolygonPick2:
		word = (int)(IBaseGrip::eSameRow);
	}
	return word;
}

bool BridgePolygonGrip::GetResolvedIconName(int which,MSTR &string)
{
	switch(which)
	{
	case eBridgePolygonType:
		string = BRIDGE_POLYGON_TYPE_PNG;
		break;
	case eBridgePolygonSegments:
		string =  BRIDGE_SEGMENTS_PNG;
		break;
	case eBridgePolygonTaper:
		string =  BRIDGE_TAPER_PNG;
		break;
	case eBridgePolygonBias:
		string =  BRIDGE_BIAS_PNG;
		break;
	case eBridgePolygonSmooth:
		string =  BRIDGE_SMOOTH_PNG;
		break;
	case eBridgePolygonTwist1:
		string =  BRIDGE_TWIST1_PNG;
		break;
	case eBridgePolygonTwist2:
		string =  BRIDGE_TWIST2_PNG;
		break;
	case eBridgePolygonPick1:
		string = BRIDGE_POLYGON_PICK1_PNG;
		break;
	case eBridgePolygonPick2:
		string = BRIDGE_POLYGON_PICK2_PNG;
		break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

}

bool BridgePolygonGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eBridgePolygonSegments:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_bridge_segments, t) == TRUE ? true : false;
	case eBridgePolygonTaper:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_bridge_taper, t) == TRUE ? true : false;
	case eBridgePolygonBias:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_bridge_bias, t) == TRUE ? true : false;
	case eBridgePolygonSmooth:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_bridge_smooth_thresh, t) == TRUE ? true : false;
	case eBridgePolygonTwist1:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_bridge_twist_1, t) == TRUE ? true : false;
	case eBridgePolygonTwist2:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_bridge_twist_2, t) == TRUE ? true : false;

	}
	return false;
}


bool BridgePolygonGrip::GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &comboOptions)
{
	if(which== eBridgePolygonType)
	{
		BOOL found;
		IBaseGrip::ComboLabel *label = new IBaseGrip::ComboLabel();
		TSTR string = BRIDGE_SPECIFIC_PNG;
		found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
		if(found)
			label->mIcon = string;
		label->mLabel = TSTR(GetString(IDS_BRIDGE_SPECIFIC_POLYGONS));
		comboOptions.Append(1,&label);
		label = new IBaseGrip::ComboLabel();
		string = BRIDGE_SELECTED_PNG;
		found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
		if(found)
			label->mIcon = string;
		label->mLabel = TSTR(GetString(IDS_BRIDGE_POLYGON_SELECTION));
		comboOptions.Append(1,&label);
		return true;
	}
	return false;
}

bool  BridgePolygonGrip::GetCommandIcon(int which, MSTR &string)
{
	switch(which)
	{
	case eBridgePolygonPick1:
		string = mPick1Icon;
		break;
	case eBridgePolygonPick2:
		string = mPick2Icon;
		break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return found ? true : false;
}

bool BridgePolygonGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{

	ParamDef theDef;
	switch(which)
	{
	case eBridgePolygonSegments:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_bridge_segments);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mInt = 1;
			maxRange.mInt = theDef.range_high.i;
		}
		break;

	case eBridgePolygonTaper:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_bridge_taper);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eBridgePolygonBias:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_bridge_bias);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eBridgePolygonSmooth:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_bridge_smooth_thresh);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eBridgePolygonTwist1:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_bridge_twist_1);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mInt = theDef.range_low.i;
			maxRange.mInt = theDef.range_high.i;
		}
		break;

	case eBridgePolygonTwist2:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_bridge_twist_2);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mInt = theDef.range_low.i;
			maxRange.mInt = theDef.range_high.i;
		}
		break;
	default:
		return false;

	}
	return true;
}

bool BridgePolygonGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eBridgePolygonType:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_selected, t, value.mInt,FOREVER);
		break;
	case eBridgePolygonSegments:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_segments, t, value.mInt,FOREVER);
		break;
	case eBridgePolygonTaper:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_taper, t, value.mFloat,FOREVER);
		break;
	case eBridgePolygonBias:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_bias, t, value.mFloat,FOREVER);
		break;
	case eBridgePolygonSmooth:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_smooth_thresh, t, value.mFloat,FOREVER);
		break;
	case eBridgePolygonTwist1:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_twist_1, t, value.mInt,FOREVER);
		break;
	case eBridgePolygonTwist2:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_twist_2, t, value.mInt,FOREVER);
		break;
	case eBridgePolygonPick1:
		{
			//value.mbool, 0 means not in pick mode, 1 does.
			if(GetEditPolyMod()->EpModGetCommandMode() == ep_mode_pick_bridge_1 ) // in command mode
				value.mbool = true;
			else
				value.mbool = false;
		}
		break;
	case eBridgePolygonPick2:
		{
			//value.mbool, 0 means not in pick mode, 1 does.
			if(GetEditPolyMod()->EpModGetCommandMode() == ep_mode_pick_bridge_2 ) // in command mode
				value.mbool = true;
			else
				value.mbool = false;
		}
		break;
	default:
		return false;
	}
	return true;
}

//need to activate grips
void BridgePolygonGrip::SetUpUI()
{
	int val;
	GetEditPolyMod()->getParamBlock()->GetValue(epm_bridge_selected,GetCOREInterface()->GetTime(),val,FOREVER);
	if( 1 == val )
	{
		GetIGripManager()->ActivateGripItem(eBridgePolygonPick1,false);
		GetIGripManager()->ActivateGripItem(eBridgePolygonPick2,false);
		SetPoly1PickDisabled();
		SetPoly2PickDisabled();
	}
	else
	{
		GetIGripManager()->ActivateGripItem(eBridgePolygonPick1,true);
		GetIGripManager()->ActivateGripItem(eBridgePolygonPick2,true);
		int targ = GetEditPolyMod()->getParamBlock()->GetInt(epm_bridge_target_1);
		if(targ==0)
			SetPoly1PickDisabled();
		else
			SetPoly1Picked(targ-1); //need to -1 out
		targ = GetEditPolyMod()->getParamBlock()->GetInt(epm_bridge_target_2);
		if(targ==0)
			SetPoly2PickDisabled();
		else
			SetPoly2Picked(targ-1); //need to -1 out

	}
	GetIGripManager()->ResetAllUI();
}


bool BridgePolygonGrip::SetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eBridgePolygonType:
		if(value.mInt>1) //go back to zero
			value.mInt =0;
		if(value.mInt<0)
			value.mInt = 1;
		if( 1 == value.mInt )
		{
			GetIGripManager()->ActivateGripItem(eBridgePolygonPick1, false);
			GetIGripManager()->ActivateGripItem(eBridgePolygonPick2, false);
		}
		else
		{
			GetIGripManager()->ActivateGripItem(eBridgePolygonPick1, true);
			GetIGripManager()->ActivateGripItem(eBridgePolygonPick2, true);
		}
		GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_selected, t, value.mInt);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_selected, t, value.mInt,FOREVER);
		break;
	case eBridgePolygonSegments:
		if( value.mInt < 1)
			value.mInt = 1;
		GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_segments, t, value.mInt);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_segments, t, value.mInt,FOREVER);
		break;
	case eBridgePolygonTaper:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_taper, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_taper, t, value.mFloat,FOREVER);
		break;
	case eBridgePolygonBias:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_bias, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_bias, t, value.mFloat,FOREVER);
		break;
	case eBridgePolygonSmooth:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_smooth_thresh, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_smooth_thresh, t, value.mFloat,FOREVER);
		break;
	case eBridgePolygonTwist1:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_twist_1, t, value.mInt);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_twist_1, t, value.mInt,FOREVER);
		break;
	case eBridgePolygonTwist2:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_twist_2, t, value.mInt);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_bridge_twist_2, t, value.mInt,FOREVER);
		break;
	case eBridgePolygonPick1:
		{
			//value.mbool now tells us to turn command mode, on or off
			//so we check and see if we are or aren't in the state
			if(GetEditPolyMod()->EpModGetCommandMode()==ep_mode_pick_bridge_1)
			{
				if(!value.mbool)
				{
					GetEditPolyMod()->EpModToggleCommandMode(ep_mode_pick_bridge_1);
					SetPoly1PickDisabled();
					//also set the text!
				}
			}
			else if(value.mbool) //not in that mode so toggle it on if told to
			{
				GetEditPolyMod()->EpModToggleCommandMode(ep_mode_pick_bridge_1);
				SetPoly1PickModeStarted();
			}
		}
		break;
	case eBridgePolygonPick2:
		{
			//value.mbool now tells us to turn command mode, on or off
			//so we check and see if we are or aren't in the state
			if(GetEditPolyMod()->EpModGetCommandMode()==ep_mode_pick_bridge_2)
			{
				if(!value.mbool)
				{
					GetEditPolyMod()->EpModToggleCommandMode(ep_mode_pick_bridge_2);
					SetPoly2PickDisabled();
					//also set the text!
				}
			}
			else if(value.mbool) //not in that mode so toggle it on if told to
			{
				GetEditPolyMod()->EpModToggleCommandMode(ep_mode_pick_bridge_2);
				SetPoly2PickModeStarted();
			}

		}
		break;
	default:
		return false;

	}
	return true;
}

void BridgePolygonGrip::Apply(TimeValue t)
{
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitAndRepeat (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void BridgePolygonGrip::Okay(TimeValue t)
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitUnlessAnimating (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void BridgePolygonGrip::Cancel()
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,0);
	GetEditPolyMod()->EpModCancel();
	EndSetValue(-1,0,true);
	GetEditPolyMod()->EpModRefreshScreen ();
}

bool BridgePolygonGrip::StartSetValue(int ,TimeValue t)
{
	theHold.Begin ();
	/*	int type = GetEditPolyMod()->GetPolyOperationID ();
	if (type != ep_op_BridgePolygon) {
	GetEditPolyMod()->EpModSetOperation (ep_op_BridgePolygon);
	} 
	*/
	return true;
}

bool BridgePolygonGrip::EndSetValue(int ,TimeValue t,bool accept)
{
	if (accept)
	{
		if(theHold.Holding())
			theHold.Accept (GetString(IDS_BRIDGE_BORDERS));

	}
	else
	{
		if(theHold.Holding())
			theHold.Cancel();
		GetEditPolyMod()->EpModRefreshScreen ();	
	}

	return true;
}



//we don't auto scale
bool BridgePolygonGrip::GetAutoScale(int which)
{
	return true; //yes
}
//default scale
bool BridgePolygonGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
	{
	case eBridgePolygonSegments:
	case eBridgePolygonTwist1:
	case eBridgePolygonTwist2:
		scaleValue.mInt = 1;
		break;
	case eBridgePolygonTaper:
	case eBridgePolygonBias:
	case eBridgePolygonSmooth:
		scaleValue.mFloat = 1.0f;
		break;
	default:
		return false;
	}
	return true;
}
bool BridgePolygonGrip::GetScaleInViewSpace(int which, float &depth)
{
	depth = -200.0f; 
	return false;
}
bool BridgePolygonGrip::ResetValue(TimeValue t, int which)
{
	float fResetVal = 0.0f;
     int iResetSegVal = 1;
	 int iResetTwistVal = 0;
	 int iResetVal = 0;

	switch(which)
	{
	case eBridgePolygonType:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_selected, t, iResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_bridge_selected, t, iResetVal,FOREVER);
			theHold.Accept (GetString(IDS_BRIDGE_BORDERS));
		}
		break;
	case eBridgePolygonSegments:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_segments, t,iResetSegVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_bridge_segments, t, iResetSegVal,FOREVER);
			theHold.Accept (GetString(IDS_BRIDGE_BORDERS));
		}
		break;
	case eBridgePolygonTaper:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_taper, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
		    GetEditPolyMod()->getParamBlock()->GetValue(epm_bridge_taper, t, fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_BRIDGE_BORDERS));
		}
		break;
	case eBridgePolygonBias:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_bias, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_bridge_bias, t, fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_BRIDGE_BORDERS));
		}
		break;
	case eBridgePolygonSmooth:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_smooth_thresh, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_bridge_smooth_thresh, t, fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_BRIDGE_BORDERS));
		}
		break;
	case eBridgePolygonTwist1:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_twist_1, t,iResetTwistVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_bridge_twist_1, t, iResetTwistVal,FOREVER);
			theHold.Accept (GetString(IDS_BRIDGE_BORDERS));
		}
		break;
	case eBridgePolygonTwist2:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_bridge_twist_2, t,iResetTwistVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_bridge_twist_2, t, iResetTwistVal,FOREVER);
			theHold.Accept (GetString(IDS_BRIDGE_BORDERS));
		}
		break;
	default:
		return false;
	}
	return true;

}

bool BridgePolygonGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{

	ParamBlockDesc2* pbd = GetEditPolyMod()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eBridgePolygonType:
		resetValue.mInt = 0;
		break;
	case eBridgePolygonSegments:
		{
			const ParamDef& pd = pbd->paramdefs[epm_bridge_segments];
			resetValue.mInt = pd.def.i;
		}
		break;
	case eBridgePolygonTaper:
		{
			const ParamDef& pd = pbd->paramdefs[epm_bridge_taper];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eBridgePolygonBias:
		{
			const ParamDef& pd = pbd->paramdefs[epm_bridge_bias];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eBridgePolygonSmooth:
		{
			const ParamDef& pd = pbd->paramdefs[epm_bridge_smooth_thresh];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eBridgePolygonTwist1:
		{
			const ParamDef& pd = pbd->paramdefs[epm_bridge_twist_1];
			resetValue.mInt = pd.def.i;
		}
		break;
	case eBridgePolygonTwist2:
		{
			const ParamDef& pd = pbd->paramdefs[epm_bridge_twist_2];
			resetValue.mInt = pd.def.i;
		}
		break;
	default:
		return false;

	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Hinge Grips

HingeGrip::~HingeGrip()
{

}

int HingeGrip::GetNumGripItems()
{
	return 3; //would be 5 for the old one since the old inluded okay and apply (remember cancel was implicit)
}
IBaseGrip::Type HingeGrip::GetType(int which)
{
	DbgAssert(which>=0&&which<=2);
	switch(which)
	{
	case eHingeAngle:
		return IBaseGrip::eFloat;
	case eHingeSegments:
		return IBaseGrip::eInt;
	case eHingePick:
		return IBaseGrip::eCommand;
	}
	return IBaseGrip::eInt;
}

void HingeGrip::GetGripName(TSTR &string)
{
	//if picking then return that as the name,
	if(mEdgePicked == TSTR(GetString(IDS_PICKING_HINGE_EDGE)))
		string = mEdgePicked;
	else
		string =TSTR(GetString(IDS_HINGE));
}

void HingeGrip::SetUpUI()
{
	GetCOREInterface()->GetModContexts (mList, mNodes);
	int hinge = -1;
	for (i=0; i<mList.Count(); i++) {
		hinge = GetEditPolyMod()->EpModGetHingeEdge (mNodes[i]);
		if (hinge>-1) break;
	}
	if (hinge<0)  
		SetEdgePickDisabled();
	else 
	   SetEdgePicked(hinge+1);

	GetIGripManager()->ResetAllUI();
}

void HingeGrip::SetEdgePickModeStarted()
{
	mEdgePicked =  TSTR(GetString(IDS_PICKING_HINGE_EDGE));
	mPickIcon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eHingePick);
}

void HingeGrip::SetEdgePicked(int num)
{
	MSTR buf;
	if (mList.Count() == 1) 
		buf.printf ("%s %d", GetString (IDS_EDGE), num);
	else 
		buf.printf ("%s - %s %d\n", mNodes[i]->GetName(), GetString (IDS_EDGE), num);

	mEdgePicked = buf;
	mPickIcon = PICK_FULL_PNG;
	GetIGripManager()->ResetGripUI((int)eHingePick);
}

void HingeGrip::SetEdgePickDisabled()
{
	mEdgePicked =  TSTR(GetString(IDS_PICK_HINGE_EDGE));
	mPickIcon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eHingePick);
}

bool HingeGrip::GetText(int which,TSTR &string)
{
	switch(which)
	{
	case eHingeAngle:
		{
			string =TSTR(GetString(IDS_ANGLE));
		}
		break;
	case eHingeSegments:
		{
			string =TSTR(GetString(IDS_SEGMENTS));
		}
		break;
	case eHingePick:
		{
			string = mEdgePicked;
		}
		break;
	default:
		return false;

	}
	return true;
}

DWORD HingeGrip::GetCustomization(int which)
{
	return 0;
}

bool HingeGrip::GetResolvedIconName(int which,MSTR &string)
{
	switch(which)
	{
	case eHingeAngle:
		string = HINGE_FROM_EDGE_ANGLE_PNG;
		break;
	case eHingeSegments:
		string =  HINGE_FROM_EDGE_SEGMENTS_PNG;
		break;
	case eHingePick:
		string =  HINGE_PICK_PNG;
		break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

}

bool HingeGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eHingeAngle:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_hinge_angle, t) == TRUE ? true : false;
	case eHingeSegments:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_hinge_segments, t) == TRUE ? true : false;
	}
	return false;
}


bool HingeGrip::GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &comboOptions)
{
	// have no radio options
	return false;
}

bool HingeGrip::GetCommandIcon(int which, MSTR &string)
{
	if(which == eHingePick)
	{
		string = mPickIcon;
		BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
		return found ? true : false;
	}
	return false;
}
bool HingeGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{

	ParamDef theDef;
	switch(which)
	{
	case eHingeAngle:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_hinge_angle);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eHingeSegments:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_hinge_segments);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mInt = 1;
			maxRange.mInt = theDef.range_high.i;
		}
		break;
	default:
		return false;

	}
	return true;
}

bool HingeGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eHingeAngle:
		{
			float valueRad = 0.0f;
			GetEditPolyMod()->getParamBlock()->GetValue (epm_hinge_angle, t, valueRad,FOREVER);
			value.mFloat = RadToDeg(valueRad);
		}
		break;
	case eHingeSegments:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_hinge_segments, t, value.mInt,FOREVER);
		break;
	case eHingePick:
		{
			//value.mbool, 0 means not in pick mode, 1 does.
			if(GetEditPolyMod()->EpModGetCommandMode()==ep_mode_pick_hinge) // in command mode
				value.mbool = true;
			else
				value.mbool = false;
		}
		break;
	default:
		return false;
	}
	return true;
}

bool HingeGrip::SetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eHingeAngle:
		{
			float valueRad = DegToRad(value.mFloat) ;
			GetEditPolyMod()->getParamBlock()->SetValue (epm_hinge_angle, t, valueRad);
			GetEditPolyMod()->getParamBlock()->GetValue (epm_hinge_angle, t, valueRad,FOREVER);
			value.mFloat = RadToDeg(valueRad);
		}
		break;
	case eHingeSegments:
		if( value.mInt < 1)
			value.mInt = 1;
		GetEditPolyMod()->getParamBlock()->SetValue (epm_hinge_segments, t, value.mInt);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_hinge_segments, t, value.mInt,FOREVER);
		break;
	case eHingePick:
		//value.mbool now tells us to turn command mode, on or off
		//so we check and see if we are or aren't in the state
		if(GetEditPolyMod()->EpModGetCommandMode()==ep_mode_pick_hinge) // in command mode
		{
			if(!value.mbool)
			{
				GetEditPolyMod()->EpModToggleCommandMode(ep_mode_pick_hinge);
				SetEdgePickDisabled();
			}
		}
		else if(value.mbool) //not in that mode so toggle it on if told to
		{
			GetEditPolyMod()->EpModToggleCommandMode(ep_mode_pick_hinge);
			SetEdgePickModeStarted();
		}
		break;
	default:
		return false;

	}
	return true;
}

void HingeGrip::Apply(TimeValue t)
{
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitAndRepeat (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void HingeGrip::Okay(TimeValue t)
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitUnlessAnimating (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void HingeGrip::Cancel()
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,0);
	GetEditPolyMod()->EpModCancel();
	EndSetValue(-1,0,true);
	GetEditPolyMod()->EpModRefreshScreen ();
}

bool HingeGrip::StartSetValue(int ,TimeValue t)
{
	theHold.Begin ();
	/*	int type = GetEditPolyMod()->GetPolyOperationID ();
	if (type != ep_op_Hinge) {
	GetEditPolyMod()->EpModSetOperation (ep_op_Hinge);
	} 
	*/
	return true;
}

bool HingeGrip::EndSetValue(int ,TimeValue t,bool accept)
{
	if (accept)
	{
		if(theHold.Holding())
			theHold.Accept (GetString(IDS_HINGE));

	}
	else
	{
		if(theHold.Holding())
			theHold.Cancel();
		GetEditPolyMod()->EpModRefreshScreen ();	
	}

	return true;
}



//we don't auto scale
bool HingeGrip::GetAutoScale(int which)
{
	return true; //yes
}
//default scale
bool HingeGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch( which)
	{
	case eHingeAngle:
		scaleValue.mFloat = 1.0f;
		break;
	case eHingeSegments:
		scaleValue.mInt = 1;
		break;
	default:
		return false;
	}
	return true;
}
bool HingeGrip::GetScaleInViewSpace(int which, float &depth)
{
	depth = -200.0f; 
	return false;
}
bool HingeGrip::ResetValue(TimeValue t, int which)
{
	float fResetVal = 0.0f;
	int iResetSegVal = 1;

	switch(which)
	{
	case eHingeAngle:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_hinge_angle, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_hinge_angle, t, fResetVal, FOREVER);
			theHold.Accept (GetString(IDS_HINGE));
		}
		break;
	case eHingeSegments:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_hinge_segments, t,iResetSegVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_hinge_segments, t, iResetSegVal,FOREVER);
			theHold.Accept (GetString(IDS_HINGE));

		}
		break;
	default:
		return false;
	}
	return true;

}

bool HingeGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{

	ParamBlockDesc2* pbd = GetEditPolyMod()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eHingeAngle:
		{
			float valueRad = 0.0f;
			const ParamDef& pd = pbd->paramdefs[epm_hinge_angle];
			valueRad = pd.def.f;
			resetValue.mFloat = RadToDeg(valueRad);
		}
		break;
	case eHingeSegments:
		{
			const ParamDef& pd = pbd->paramdefs[epm_hinge_segments];
			resetValue.mInt = pd.def.i;
		}
		break;
	default:
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Extrude Along Spline Grip

ExtrudeAlongSplineGrip::~ExtrudeAlongSplineGrip()
{

}

int ExtrudeAlongSplineGrip::GetNumGripItems()
{
	return 7; 
}
IBaseGrip::Type ExtrudeAlongSplineGrip::GetType(int which)
{
	DbgAssert(which>=0&&which<=6);
	switch(which)
	{
	case eExtrudeAlongSplineSegments:
		return IBaseGrip::eInt;
	case eExtrudeAlongSplineTaperAmount:
		return IBaseGrip::eFloat;
	case eExtrudeAlongSplineTaperCurve:
		return IBaseGrip::eFloat;
	case eExtrudeAlongSplineTwist:
		return IBaseGrip::eFloat;
	case eExtrudeAlongSplineAlignToFaceNormal:
		return IBaseGrip::eToggle;
	case eExtrudeAlongSplineRotation:
		return IBaseGrip::eFloat;
	case eExtrudeAlongSplinePickSpline:
		return IBaseGrip::eCommand;
	}
	return IBaseGrip::eInt;
}

void ExtrudeAlongSplineGrip::GetGripName(TSTR &string)
{
	if(mSplinePicked == TSTR(GetString(IDS_PICKING_SPLINE)))
		string = TSTR(GetString(IDS_PICKING_SPLINE));
	else
		string =TSTR(GetString(IDS_EXTRUDE_ALONG_SPLINE));
}

void ExtrudeAlongSplineGrip::SetSplinePickModeStarted()
{
	mSplinePicked = TSTR(GetString(IDS_PICKING_SPLINE));
	mPickIcon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eExtrudeAlongSplinePickSpline);
}

void ExtrudeAlongSplineGrip::SetSplinePicked(INode *node)
{
	mSplinePicked = TSTR(node->GetName());
	mPickIcon = PICK_FULL_PNG;
	GetIGripManager()->ResetGripUI((int)eExtrudeAlongSplinePickSpline);
}

void ExtrudeAlongSplineGrip::SetSplinePickDisabled()
{
	mSplinePicked = TSTR(GetString(IDS_PICK_SPLINE));
	mPickIcon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eExtrudeAlongSplinePickSpline);
}

bool ExtrudeAlongSplineGrip::GetText(int which,TSTR &string)
{
	switch(which)
	{
	case eExtrudeAlongSplineSegments:
		{
			string =TSTR(GetString(IDS_SEGMENTS));// + TSTR(": ") + TSTR(str);
		}
		break;
	case eExtrudeAlongSplineTaperAmount:
		{
			string =TSTR(GetString(IDS_TAPER_AMOUNTS));//+ TSTR(": ") + TSTR(str);
		}
		break;
	case eExtrudeAlongSplineTaperCurve:
		{
			string =TSTR(GetString(IDS_TAPER_CURVE));//+ TSTR(": ") + TSTR(str);
		}
		break;
	case eExtrudeAlongSplineTwist:
		{
			string =TSTR(GetString(IDS_TWIST));//+ TSTR(": ") + TSTR(str);
		}
		break;
	case eExtrudeAlongSplineAlignToFaceNormal:
		{
			string =TSTR(GetString(IDS_ALIGN_TO_FACE_NORMAL));//+ TSTR(": ") + TSTR(str);
		}
		break;
	case eExtrudeAlongSplineRotation:
		{
			string =TSTR(GetString(IDS_ROTATION));//+ TSTR(": ") + TSTR(str);
		}
		break;
	case eExtrudeAlongSplinePickSpline:
		{
			string  = mSplinePicked;
		}
		break;
	default:
		return false;

	}
	return true;
}


DWORD ExtrudeAlongSplineGrip::GetCustomization(int which)
{
	DWORD word =0;
	switch(which)
	{

	case eExtrudeAlongSplineTaperAmount:
	case eExtrudeAlongSplineTwist:
	case eExtrudeAlongSplineRotation:
		word = (int)(IBaseGrip::eSameRow);
		word = (int)(IBaseGrip::eSameRow);
		break;
	}
	return word;
}

bool ExtrudeAlongSplineGrip::GetResolvedIconName(int which,MSTR &string)
{
	switch(which)
	{
	case eExtrudeAlongSplineSegments:
		string = EXTRUDE_ALONG_SPLINE_SEGMENTS_PNG;
		break;
	case eExtrudeAlongSplineTaperAmount:
		string =  EXTRUDE_ALONG_SPLINE_TAPER_AMOUNT_PNG;
		break;
	case eExtrudeAlongSplineTaperCurve:
		string =  EXTRUDE_ALONG_SPLINE_TAPER_CURVE_PNG;
		break;
	case eExtrudeAlongSplineTwist:
		string = EXTRUDE_ALONG_SPLINE_TWIST_PNG;
		break;
	case eExtrudeAlongSplineRotation:
		string = EXTRUDE_ALONG_SPLINE_ROTATION_PNG;
		break;
	case eExtrudeAlongSplinePickSpline:
		string = EXTRUDE_ALONG_SPLINE_PICK_PNG;
		break;
	case eExtrudeAlongSplineAlignToFaceNormal:
		string =EXTRUDE_ALONG_SPLINE_TO_FACE_NORMAL_PNG;
		break;
	default:
		return false;

	}
	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;
}

bool ExtrudeAlongSplineGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eExtrudeAlongSplineSegments:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_extrude_spline_segments, t) == TRUE ? true : false;
	case eExtrudeAlongSplineTaperAmount:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_extrude_spline_taper, t) == TRUE ? true : false;
	case eExtrudeAlongSplineTaperCurve:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_extrude_spline_taper_curve, t) == TRUE ? true : false;
	case eExtrudeAlongSplineTwist:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_extrude_spline_twist, t) == TRUE ? true : false;
	case eExtrudeAlongSplineRotation:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_extrude_spline_rotation, t) == TRUE ? true : false;
	}
	return false;
}


bool ExtrudeAlongSplineGrip::GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &comboOptions)
{
	// have no radio options
	return false;
}


bool ExtrudeAlongSplineGrip::GetCommandIcon(int which, MSTR &string)
{
	if(which == eExtrudeAlongSplinePickSpline)
	{
		string = mPickIcon;
		BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
		return found ? true : false;
	}
	return false;
}

bool ExtrudeAlongSplineGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{

	ParamDef theDef;
	switch(which)
	{
	case eExtrudeAlongSplineSegments:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_extrude_spline_segments);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mInt = 1;
			maxRange.mInt = theDef.range_high.i;
		}
		break;

	case eExtrudeAlongSplineTaperAmount:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_extrude_spline_taper);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eExtrudeAlongSplineTaperCurve:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_extrude_spline_taper_curve);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eExtrudeAlongSplineTwist:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_extrude_spline_twist);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;
	case eExtrudeAlongSplineRotation:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_extrude_spline_rotation);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;
	default:
		return false;

	}
	return true;
}

bool ExtrudeAlongSplineGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eExtrudeAlongSplineSegments:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_spline_segments, t, value.mInt,FOREVER);
		break;
	case eExtrudeAlongSplineTaperAmount:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_spline_taper, t, value.mFloat,FOREVER);
		break;
	case eExtrudeAlongSplineTaperCurve:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_spline_taper_curve, t, value.mFloat,FOREVER);
		break;
	case eExtrudeAlongSplineTwist:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_spline_twist, t, value.mFloat,FOREVER);
		break;
	case eExtrudeAlongSplineAlignToFaceNormal:
		{
			//need to go BOOL (int) to bool
			int val =0;
			GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_spline_align, t, val,FOREVER);
			value.mbool =  ( val==0 ? false :true);
		}
		break;
	case eExtrudeAlongSplineRotation:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_spline_rotation, t, value.mFloat,FOREVER);
		break;
	case eExtrudeAlongSplinePickSpline:
		{
			if(GetEditPolyMod()->EpModGetPickMode()==ep_mode_pick_shape) //in command mode 
				value.mbool = true;
			else
				value.mbool = false;
		}
		break;
	default:
		return false;
	}
	return true;
}

//need to activate grips
void ExtrudeAlongSplineGrip::SetUpUI()
{
	int val;
	GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_spline_align,  GetCOREInterface()->GetTime(), val,FOREVER);
	if(!val)
	{
		GetIGripManager()->ActivateGripItem(eExtrudeAlongSplineRotation,false);
		SetSplinePickDisabled();
	}
	else
	{
		GetIGripManager()->ActivateGripItem(eExtrudeAlongSplineRotation,true);

		INode* splineNode = NULL;
		GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_spline_node, TimeValue(0), splineNode, FOREVER);
		if (splineNode) 
			SetSplinePicked(splineNode);
		else
		{
			if(GetEditPolyMod()->EpModGetPickMode()==ep_mode_pick_shape)  //in command mode so still picking
				SetSplinePickModeStarted();
			else
				SetSplinePickDisabled();
		}
	}
	GetIGripManager()->ResetAllUI();
}

bool ExtrudeAlongSplineGrip::SetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eExtrudeAlongSplineSegments:
		if( value.mInt < 1)
			value.mInt = 1;
		GetEditPolyMod()->getParamBlock()->SetValue (epm_extrude_spline_segments, t, value.mInt);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_spline_segments, t, value.mInt,FOREVER);
		break;
	case eExtrudeAlongSplineTaperAmount:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_extrude_spline_taper, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_spline_taper, t, value.mFloat,FOREVER);
		break;
	case eExtrudeAlongSplineTaperCurve:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_extrude_spline_taper_curve, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_spline_taper_curve, t, value.mFloat,FOREVER);
		break;
	case eExtrudeAlongSplineTwist:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_extrude_spline_twist, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_spline_twist, t, value.mFloat,FOREVER);
		break;
	case eExtrudeAlongSplineAlignToFaceNormal:
		{
			int val  = (int) value.mbool;
		   GetEditPolyMod()->getParamBlock()->SetValue (epm_extrude_spline_align, t, val);
		   GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_spline_align, t, val,FOREVER);
		   if(!val)
			   GetIGripManager()->ActivateGripItem(eExtrudeAlongSplineRotation,false);
		   else
			   GetIGripManager()->ActivateGripItem(eExtrudeAlongSplineRotation,true);
		}
		break;
	case eExtrudeAlongSplineRotation:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_extrude_spline_rotation, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_extrude_spline_rotation, t, value.mFloat,FOREVER);
		break;
	case eExtrudeAlongSplinePickSpline:
		{
			if(GetEditPolyMod()->EpModGetPickMode()==ep_mode_pick_shape)  //in command mode so still picking
			{
				if(!value.mbool)
				{
					GetEditPolyMod()->EpModEnterPickMode(ep_mode_pick_shape);
					SetSplinePickDisabled();
				}
			}
			else if(value.mbool)
			{
				GetEditPolyMod()->EpModEnterPickMode(ep_mode_pick_shape);
				SetSplinePickModeStarted();
			}
		}
		break;
	default:
		return false;

	}
	return true;
}

void ExtrudeAlongSplineGrip::Apply(TimeValue t)
{
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitAndRepeat (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void ExtrudeAlongSplineGrip::Okay(TimeValue t)
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitUnlessAnimating (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void ExtrudeAlongSplineGrip::Cancel()
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,0);
	GetEditPolyMod()->EpModCancel();
	EndSetValue(-1,0,true);
	GetEditPolyMod()->EpModRefreshScreen ();
}

bool ExtrudeAlongSplineGrip::StartSetValue(int ,TimeValue t)
{
	theHold.Begin ();
	/*	int type = GetEditPolyMod()->GetPolyOperationID ();
	if (type != ep_op_ExtrudeAlongSpline) {
	GetEditPolyMod()->EpModSetOperation (ep_op_ExtrudeAlongSpline);
	} 
	*/
	return true;
}

bool ExtrudeAlongSplineGrip::EndSetValue(int ,TimeValue t,bool accept)
{
	if (accept)
	{
		if(theHold.Holding())
			theHold.Accept (GetString(IDS_EXTRUDE_ALONG_SPLINE));

	}
	else
	{
		if(theHold.Holding())
			theHold.Cancel();
		GetEditPolyMod()->EpModRefreshScreen ();	
	}

	return true;
}



//we don't auto scale
bool ExtrudeAlongSplineGrip::GetAutoScale(int which)
{
	return true; //yes
}
//default scale
bool ExtrudeAlongSplineGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
	{
	case eExtrudeAlongSplineSegments:
		scaleValue.mInt = 1;
		break;
	case eExtrudeAlongSplineTaperAmount:
		scaleValue.mFloat = 1.0f;
		break;
	case eExtrudeAlongSplineTaperCurve:
		scaleValue.mFloat = 1.0f;
		break;
	case eExtrudeAlongSplineTwist:
		scaleValue.mFloat = 1.0f;
		break;
	case eExtrudeAlongSplineRotation:
		scaleValue.mFloat = 1.0f;
		break;
	default:
		return false;
	}
	return true;
}
bool ExtrudeAlongSplineGrip::GetScaleInViewSpace(int which, float &depth)
{
	depth = -200.0f; 
	return false;
}
bool ExtrudeAlongSplineGrip::ResetValue(TimeValue t, int which)
{
	float fResetVal = 0.0f;
	int iResetSegVal = 1;
	int iResetVal = 0;

	switch(which)
	{
	case eExtrudeAlongSplineSegments:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_extrude_spline_segments, t,iResetSegVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_extrude_spline_segments, t, iResetSegVal,FOREVER);
			theHold.Accept (GetString(IDS_EXTRUDE_ALONG_SPLINE));
		}
		break;
	case eExtrudeAlongSplineTaperAmount:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_extrude_spline_taper, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_extrude_spline_taper, t, fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_EXTRUDE_ALONG_SPLINE));

		}
		break;
	case eExtrudeAlongSplineTaperCurve:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_extrude_spline_taper_curve, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_extrude_spline_taper_curve, t, fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_EXTRUDE_ALONG_SPLINE));

		}
		break;
	case eExtrudeAlongSplineTwist:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_extrude_spline_twist, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_extrude_spline_twist, t, fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_EXTRUDE_ALONG_SPLINE));

		}
		break;
	case eExtrudeAlongSplineAlignToFaceNormal:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_extrude_spline_align, t, iResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_extrude_spline_align, t, iResetVal,FOREVER);
			theHold.Accept (GetString(IDS_EXTRUDE_ALONG_SPLINE));

		}
		break;
	case eExtrudeAlongSplineRotation:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_extrude_spline_rotation, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_extrude_spline_rotation, t,fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_EXTRUDE_ALONG_SPLINE));

		}
		break;
	case eExtrudeAlongSplinePickSpline:
		break;
	default:
		return false;
	}
	return true;

}

bool ExtrudeAlongSplineGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{

	ParamBlockDesc2* pbd = GetEditPolyMod()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eExtrudeAlongSplineSegments:
		{
			const ParamDef& pd = pbd->paramdefs[epm_extrude_spline_segments];
			resetValue.mInt = pd.def.i;
		}
		break;
	case eExtrudeAlongSplineTaperAmount:
		{
			const ParamDef& pd = pbd->paramdefs[epm_extrude_spline_taper];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eExtrudeAlongSplineTaperCurve:
		{
			const ParamDef& pd = pbd->paramdefs[epm_extrude_spline_taper_curve];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eExtrudeAlongSplineTwist:
		{
			const ParamDef& pd = pbd->paramdefs[epm_extrude_spline_twist];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eExtrudeAlongSplineAlignToFaceNormal:
		{
			const ParamDef& pd = pbd->paramdefs[epm_extrude_spline_align];
			resetValue.mbool = (pd.def.i ==0 ? false : true);
		}
		break;
	case eExtrudeAlongSplineRotation:
		{
			const ParamDef& pd = pbd->paramdefs[epm_extrude_spline_rotation];
			resetValue.mFloat = pd.def.f;
		}
		break;
	default:
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Weld Vertices Grip

WeldVerticesGrip::~WeldVerticesGrip()
{

}

int WeldVerticesGrip::GetNumGripItems()
{
	return 2; //would be 5 for the old one since the old inluded okay and apply (remember cancel was implicit)
}
IBaseGrip::Type WeldVerticesGrip::GetType(int which)
{
	DbgAssert(which>=0&&which<=1);
	switch(which)
	{
	case eWeldVerticesWeldThreshold:
		return IBaseGrip::eUniverse;
	case eWeldVerticesNum:
		return IBaseGrip::eStatus;
	}
	return IBaseGrip::eInt;
}

void WeldVerticesGrip::GetGripName(TSTR &string)
{
	string =TSTR(GetString(IDS_WELD_VERTS));
}

bool WeldVerticesGrip::GetText(int which,TSTR &string)
{
	switch(which)
	{
	case eWeldVerticesWeldThreshold:
		{
			string =TSTR(GetString(IDS_WELDTHRESHOLD));// + TSTR(": ") + TSTR(str);
		}
		break;
	case eWeldVerticesNum:
		{
			string = TSTR(GetString(IDS_WELD_BEFORE)) + TSTR(": ") + 
				mStrBefore + TSTR(", ") + TSTR(GetString(IDS_WELD_AFTER)) +  TSTR(": ") + 
				mStrAfter;
		}
		break;
	default:
		return false;

	}
	return true;
}

DWORD WeldVerticesGrip::GetCustomization(int which)
{
	return 0;
}

bool WeldVerticesGrip::GetResolvedIconName(int which,MSTR &string)
{
	switch(which)
	{
	case eWeldVerticesWeldThreshold:
		string = WELD_THRESHOLD_PNG;
		break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

}

bool WeldVerticesGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eWeldVerticesWeldThreshold:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_weld_vertex_threshold, t) == TRUE ? true : false;
	}
	return false;
}


bool WeldVerticesGrip::GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &comboOptions)
{
	// have no radio options
	return false;
}

bool WeldVerticesGrip::GetCommandIcon(int which, MSTR &string)
{
	return false;
}


bool WeldVerticesGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{

	ParamDef theDef;
	switch(which)
	{
	case eWeldVerticesWeldThreshold:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_weld_vertex_threshold);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = 0.0f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;
	default:
		return false;

	}
	return true;
}

bool WeldVerticesGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eWeldVerticesWeldThreshold:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_weld_vertex_threshold, t, value.mFloat,FOREVER);
		break;
	default:
		return false;
	}
	return true;
}

bool WeldVerticesGrip::SetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eWeldVerticesWeldThreshold:
		if( value.mFloat < 0.0f )
			value.mFloat = 0.0f;
		GetEditPolyMod()->getParamBlock()->SetValue (epm_weld_vertex_threshold, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_weld_vertex_threshold, t, value.mFloat,FOREVER);
		break;
	default:
		return false;
	}
	return true;
}

void WeldVerticesGrip::Apply(TimeValue t)
{
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitAndRepeat (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void WeldVerticesGrip::Okay(TimeValue t)
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitUnlessAnimating (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void WeldVerticesGrip::Cancel()
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,0);
	GetEditPolyMod()->EpModCancel();
	EndSetValue(-1,0,true);
	GetEditPolyMod()->EpModRefreshScreen ();
}

bool WeldVerticesGrip::StartSetValue(int ,TimeValue t)
{
	theHold.Begin ();
	/*	int type = GetEditPolyMod()->GetPolyOperationID ();
	if (type != ep_op_WeldVertices) {
	GetEditPolyMod()->EpModSetOperation (ep_op_WeldVertices);
	} 
	*/
	return true;
}

bool WeldVerticesGrip::EndSetValue(int ,TimeValue t,bool accept)
{
	if (accept)
	{
		if(theHold.Holding())
			theHold.Accept (GetString(IDS_WELD_VERTS));

	}
	else
	{
		if(theHold.Holding())
			theHold.Cancel();
		GetEditPolyMod()->EpModRefreshScreen ();	
	}

	return true;

}



//we don't auto scale
bool WeldVerticesGrip::GetAutoScale(int which)
{
	return true; //yes
}
//default scale
bool WeldVerticesGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
	{
	case eWeldVerticesWeldThreshold:
		scaleValue.mFloat = 1.0f;
		break;
	default:
		return false;
	}
	return true;
}
bool WeldVerticesGrip::GetScaleInViewSpace(int which, float &depth)
{
	depth = -200.0f; 
	switch(which)
	{
	case eWeldVerticesWeldThreshold:
		return true;
	}
	
	return false;
}
bool WeldVerticesGrip::ResetValue(TimeValue t, int which)
{
	float fResetVal = 0.0f;

	switch(which)
	{
	case eWeldVerticesWeldThreshold:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_weld_vertex_threshold, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_weld_vertex_threshold, t, fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_WELD_VERTS));
		}
		break;
	default:
		return false;
	}
	return true;

}

bool WeldVerticesGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{

	ParamBlockDesc2* pbd = GetEditPolyMod()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eWeldVerticesWeldThreshold:
		{
			const ParamDef& pd = pbd->paramdefs[epm_weld_vertex_threshold];
			resetValue.mFloat = pd.def.f;
		}
		break;
	default:
		return false;
	}
	return true;
}

void WeldVerticesGrip::SetNumVerts(int before,int after)
{
	mStrBefore.printf(_T("%d"),before);
	mStrAfter.printf(_T("%d"),after);
	GetIGripManager()->ResetGripUI((int)eWeldVerticesNum);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Weld Edge Grip

WeldEdgesGrip::~WeldEdgesGrip()
{

}

int WeldEdgesGrip::GetNumGripItems()
{
	return 2; 
}

IBaseGrip::Type WeldEdgesGrip::GetType(int which)
{
	DbgAssert(which>=0&&which<=1);
	switch(which)
	{
	case eWeldEdgesWeldThreshold:
		return IBaseGrip::eUniverse;
	case eWeldEdgesNum:
		return IBaseGrip::eStatus;
	}
	return IBaseGrip::eInt;
}

void WeldEdgesGrip::GetGripName(TSTR &string)
{
	string =TSTR(GetString(IDS_WELD_EDGES));
}

bool WeldEdgesGrip::GetText(int which,TSTR &string)
{
	switch(which)
	{
	case eWeldEdgesWeldThreshold:
		{
			string =TSTR(GetString(IDS_WELDTHRESHOLD));// + TSTR(": ") + TSTR(str);
		}
		break;
	case eWeldEdgesNum:
		{
			string = TSTR(GetString(IDS_WELD_BEFORE)) + TSTR(": ") + 
				mStrBefore + TSTR(", ") + TSTR(GetString(IDS_WELD_AFTER)) +  TSTR(": ") + 
				mStrAfter;
		}
		break;
	default:
		return false;

	}
	return true;
}

DWORD WeldEdgesGrip::GetCustomization(int which)
{
	return 0;
}

bool WeldEdgesGrip::GetResolvedIconName(int which,MSTR &string)
{
	DbgAssert(which>=0&&which<=1);
	switch(which)
	{
	case eWeldEdgesWeldThreshold:
		string = WELD_THRESHOLD_PNG;
		break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

}

bool WeldEdgesGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eWeldEdgesWeldThreshold:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_weld_edge_threshold, t) == TRUE ? true : false;
	}
	return false;
}


bool WeldEdgesGrip::GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &comboOptions)
{
	// have no radio options
	return false;
}

bool WeldEdgesGrip::GetCommandIcon(int which, MSTR &string)
{
	return false;
}
bool WeldEdgesGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{

	ParamDef theDef;
	switch(which)
	{
	case eWeldEdgesWeldThreshold:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_weld_edge_threshold);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = 0.0f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;
	default:
		return false;

	}
	return true;
}

bool WeldEdgesGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eWeldEdgesWeldThreshold:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_weld_edge_threshold, t, value.mFloat,FOREVER);
		break;
	default:
		return false;
	}
	return true;
}

bool WeldEdgesGrip::SetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eWeldEdgesWeldThreshold:
		if( value.mFloat < 0.0f )
			value.mFloat = 0.0f;
		GetEditPolyMod()->getParamBlock()->SetValue (epm_weld_edge_threshold, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_weld_edge_threshold, t, value.mFloat,FOREVER);
		break;
	default:
		return false;
	}
	return true;
}

void WeldEdgesGrip::Apply(TimeValue t)
{
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitAndRepeat (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void WeldEdgesGrip::Okay(TimeValue t)
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitUnlessAnimating (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void WeldEdgesGrip::Cancel()
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,0);
	GetEditPolyMod()->EpModCancel();
	EndSetValue(-1,0,true);
	GetEditPolyMod()->EpModRefreshScreen ();
}

bool WeldEdgesGrip::StartSetValue(int ,TimeValue t)
{
	theHold.Begin ();
	/*	int type = GetEditPolyMod()->GetPolyOperationID ();
	if (type != ep_op_WeldEdges) {
	GetEditPolyMod()->EpModSetOperation (ep_op_WeldEdges);
	} 
	*/
	return true;
}

bool WeldEdgesGrip::EndSetValue(int ,TimeValue t,bool accept)
{
	if (accept)
	{
		if(theHold.Holding())
			theHold.Accept (GetString(IDS_WELD_EDGES));

	}
	else
	{
		if(theHold.Holding())
			theHold.Cancel();
		GetEditPolyMod()->EpModRefreshScreen ();	
	}

	return true;
}



//we don't auto scale
bool WeldEdgesGrip::GetAutoScale(int which)
{
	return true; //yes
}
//default scale
bool WeldEdgesGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
	{
	case eWeldEdgesWeldThreshold:
		scaleValue.mFloat = 1.0f;
		break;
	default:
		return false;
	}	
	return true;
}
bool WeldEdgesGrip::GetScaleInViewSpace(int which, float &depth)
{
	depth = -200.0f; 
	switch(which)
	{
	case eWeldEdgesWeldThreshold:
		return true;
	}
	
	return false;
}
bool WeldEdgesGrip::ResetValue(TimeValue t, int which)
{
    float fResetVal = 0.0f;

	switch(which)
	{
	case eWeldEdgesWeldThreshold:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_weld_edge_threshold, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_weld_edge_threshold, t, fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_WELD_VERTS));
		}
		break;
	default:
		return false;
	}
	return true;

}

bool WeldEdgesGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{

	ParamBlockDesc2* pbd = GetEditPolyMod()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eWeldEdgesWeldThreshold:
		{
			const ParamDef& pd = pbd->paramdefs[epm_weld_edge_threshold];
			resetValue.mFloat = pd.def.f;
		}
		break;
	default:
		return false;
	}
	return true;
}

void WeldEdgesGrip::SetNumVerts(int before,int after)
{
	mStrBefore.printf(_T("%d"),before);
	mStrAfter.printf(_T("%d"),after);
	GetIGripManager()->ResetGripUI((int)eWeldEdgesNum);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MSmooth Grip

MSmoothGrip::~MSmoothGrip()
{

}

int MSmoothGrip::GetNumGripItems()
{
	return 3; //would be 5 for the old one since the old inluded okay and apply (remember cancel was implicit)
}
IBaseGrip::Type MSmoothGrip::GetType(int which)
{
	DbgAssert(which>=0&&which<=2);
	switch(which)
	{
	case eMSmoothSmoothness:
		return IBaseGrip::eFloat;
	case eMSmoothSmoothingGroups:
		return IBaseGrip::eToggle;
	case eMSmoothMaterials:
		return IBaseGrip::eToggle;
	}
	return IBaseGrip::eInt;
}

void MSmoothGrip::GetGripName(TSTR &string)
{
	string =TSTR(GetString(IDS_MESHSMOOTH));
}

bool MSmoothGrip::GetText(int which,TSTR &string)
{
	switch(which)
	{
	case eMSmoothSmoothness:
		{
			string =TSTR(GetString(IDS_SMOOTHNESS));// + TSTR(": ") + TSTR(str);
		}
		break;
	case eMSmoothSmoothingGroups:
		{
			string =TSTR(GetString(IDS_SMOOTHING_GROUPS));//+ TSTR(": ") + TSTR(str);
		}
		break;
	case eMSmoothMaterials:
		{
			string =TSTR(GetString(IDS_MATERIALS));//+ TSTR(": ") + TSTR(str);
		}
		break;
	default:
		return false;

	}
	return true;
}

DWORD MSmoothGrip::GetCustomization(int which)
{
	DWORD word =0;
	switch(which)
	{
	case eMSmoothMaterials:
		word = (int)(IBaseGrip::eSameRow);
		break;
	}
	return word;
}

bool MSmoothGrip::GetResolvedIconName(int which,MSTR &string)
{
	switch(which)
	{
	case eMSmoothSmoothness:
		string = MSMOOTH_SMOOTHNESS_PNG;
		break;
	case eMSmoothSmoothingGroups:
		string =MSMOOTH_SMOOTHING_GROUPS_PNG;
		break;
	case eMSmoothMaterials:
		string =MSMOOTH_SMOOTH_MATERIALS_PNG;
		break;
	default:
		return false;
	}
	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;
}

bool MSmoothGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eMSmoothSmoothness:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_ms_smoothness, t) == TRUE ? true : false;
	}
	return false;
}


bool MSmoothGrip::GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &comboOptions)
{
	// have no radio options
	return false;
}

bool MSmoothGrip::GetCommandIcon(int which, MSTR &string)
{
	return false;
}

bool MSmoothGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{

	ParamDef theDef;
	switch(which)
	{
	case eMSmoothSmoothness:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_ms_smoothness);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;
	default:
		return false;

	}
	return true;
}

bool MSmoothGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eMSmoothSmoothness:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_ms_smoothness, t, value.mFloat,FOREVER);
		break;
	case eMSmoothSmoothingGroups:
		{
			int val = 0;
			GetEditPolyMod()->getParamBlock()->GetValue (epm_ms_sep_smooth, t, val,FOREVER);
			value.mbool = (val == 0 ? false:true );
		}
		break;
	case eMSmoothMaterials:
		{
			int val = 0;
			GetEditPolyMod()->getParamBlock()->GetValue (epm_ms_sep_mat, t, val,FOREVER);
			value.mbool = (val == 0 ? false:true );
		}
		break;
	default:
		return false;
	}
	return true;
}

bool MSmoothGrip::SetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eMSmoothSmoothness:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_ms_smoothness, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_ms_smoothness, t, value.mFloat,FOREVER);
		break;
	case eMSmoothSmoothingGroups:
		{
			int val = (int)value.mbool ;
			GetEditPolyMod()->getParamBlock()->SetValue (epm_ms_sep_smooth, t, val);
		    GetEditPolyMod()->getParamBlock()->GetValue (epm_ms_sep_smooth, t, val,FOREVER);
		}
		break;
	case eMSmoothMaterials:
		{
			int val =(int)value.mbool ;
			GetEditPolyMod()->getParamBlock()->SetValue (epm_ms_sep_mat, t, val);
		    GetEditPolyMod()->getParamBlock()->GetValue (epm_ms_sep_mat, t, val,FOREVER);
		}
		break;
	default:
		return false;

	}
	return true;
}

void MSmoothGrip::Apply(TimeValue t)
{
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitAndRepeat (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void MSmoothGrip::Okay(TimeValue t)
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitUnlessAnimating (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void MSmoothGrip::Cancel()
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,0);
	GetEditPolyMod()->EpModCancel();
	EndSetValue(-1,0,true);
	GetEditPolyMod()->EpModRefreshScreen ();
}

bool MSmoothGrip::StartSetValue(int ,TimeValue t)
{
	theHold.Begin ();
	/*	int type = GetEditPolyMod()->GetPolyOperationID ();
	if (type != ep_op_MSmooth) {
	GetEditPolyMod()->EpModSetOperation (ep_op_MSmooth);
	} 
	*/
	return true;
}

bool MSmoothGrip::EndSetValue(int ,TimeValue t,bool accept)
{
	if (accept)
	{
		if(theHold.Holding())
			theHold.Accept (GetString(IDS_MESHSMOOTH));

	}
	else
	{
		if(theHold.Holding())
			theHold.Cancel();
		GetEditPolyMod()->EpModRefreshScreen ();	
	}

	return true;
}



//we don't auto scale
bool MSmoothGrip::GetAutoScale(int which)
{
	return true; //yes
}
//default scale
bool MSmoothGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
	{
	case eMSmoothSmoothness:
		scaleValue.mFloat = 1.0f;
		break;
	default:
		return false;
	}
	return true;
}
bool MSmoothGrip::GetScaleInViewSpace(int which, float &depth)
{
	depth = -200.0f; 
	return false;
}
bool MSmoothGrip::ResetValue(TimeValue t, int which)
{
	float fResetVal = 0.0f;
	int iResetVal = 0;

	switch(which)
	{
	case eMSmoothSmoothness:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_ms_smoothness, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_ms_smoothness, t, fResetVal,FOREVER);
			theHold.Accept (GetString(IDS_MESHSMOOTH));
		}
		break;
	case eMSmoothSmoothingGroups:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_ms_sep_smooth, t, iResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_ms_sep_smooth,t, iResetVal,FOREVER);
			theHold.Accept (GetString(IDS_MESHSMOOTH));

		}
		break;
	case eMSmoothMaterials:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_ms_sep_mat, t, iResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_ms_sep_mat, t, iResetVal,FOREVER);
			theHold.Accept (GetString(IDS_MESHSMOOTH));

		}
		break;
	default:
		return false;
	}
	return true;

}

bool MSmoothGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{

	ParamBlockDesc2* pbd = GetEditPolyMod()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eMSmoothSmoothness:
		{
			const ParamDef& pd = pbd->paramdefs[epm_ms_smoothness];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eMSmoothSmoothingGroups:
		{
			const ParamDef& pd = pbd->paramdefs[epm_ms_sep_smooth];
			resetValue.mbool = (pd.def.i == 0 ? false:true);
		}
		break;
	case eMSmoothMaterials:
		{
			const ParamDef& pd = pbd->paramdefs[epm_ms_sep_mat];
			resetValue.mbool = (pd.def.i == 0 ? false:true);
		}
		break;
	default:
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tessellate Grip

TessellateGrip::~TessellateGrip()
{

}

int TessellateGrip::GetNumGripItems()
{
	return 2; //would be 5 for the old one since the old inluded okay and apply (remember cancel was implicit)
}
IBaseGrip::Type TessellateGrip::GetType(int which)
{
	DbgAssert(which>=0&&which<=1);
	switch(which)
	{
	case eTessellateType:
		return IBaseGrip::eCombo;
	case eTessellateTension:
		return IBaseGrip::eFloat;
	}
	return IBaseGrip::eInt;
}

void TessellateGrip::GetGripName(TSTR &string)
{
	string =TSTR(GetString(IDS_TESSELLATE));
}

bool TessellateGrip::GetText(int which,TSTR &string)
{
	switch(which)
	{
	case eTessellateType:
		{
			int type;
			GetEditPolyMod()->getParamBlock()->GetValue (epm_tess_type, 0, type,FOREVER);
			switch(type)
			{
			case 0:
				string =  TSTR(GetString(IDS_EDGE));
				break;
			case 1:
				string =  TSTR(GetString(IDS_FACE));
				break;

			}
			break;
		}
	case eTessellateTension:
		{
			string =TSTR(GetString(IDS_TESS_TENSION));
		}
		break;
	default:
		return false;

	}
	return true;
}

DWORD TessellateGrip::GetCustomization(int which)
{
	return 0;
}

bool TessellateGrip::GetResolvedIconName(int which,MSTR &string)
{
	switch(which)
	{
	case eTessellateType:
		string = TESSELLATE_TYPE_PNG;
		break;
	case eTessellateTension:
		string =  TESSELLATE_TENSION_PNG;
		break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

}

bool TessellateGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eTessellateTension:
		return GetEditPolyMod()->getParamBlock()->KeyFrameAtTime((ParamID)epm_tess_tension, t) == TRUE ? true : false;
	}
	return false;
}

bool TessellateGrip::GetCommandIcon(int which, MSTR &string)
{
	return false;
}


bool TessellateGrip::GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &comboOptions)
{
	if(which== eTessellateType)
	{
		BOOL found;
		IBaseGrip::ComboLabel *label = new IBaseGrip::ComboLabel();
		TSTR string = TESSELLATE_EDGE_PNG;
		found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
		if(found)
			label->mIcon = string;
		label->mLabel = TSTR(GetString(IDS_EDGE));
		comboOptions.Append(1,&label);
		label = new IBaseGrip::ComboLabel();
		string = TESSELLATE_FACE_PNG;
		found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
		if(found)
			label->mIcon = string;
		label->mLabel = TSTR(GetString(IDS_FACE));
		comboOptions.Append(1,&label);
		return true;
	}
	return false;
}

bool TessellateGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{

	ParamDef theDef;
	switch(which)
	{
	case eTessellateTension:
		theDef = GetEditPolyMod()->getParamBlock()->GetParamDef(epm_tess_tension);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;
	default:
		return false;

	}
	return true;
}

bool TessellateGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eTessellateType:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_tess_type, t, value.mInt,FOREVER);
		break;
	case eTessellateTension:
		GetEditPolyMod()->getParamBlock()->GetValue (epm_tess_tension, t, value.mFloat,FOREVER);
		break;
	default:
		return false;
	}
	return true;
}

//need to activate grips
void TessellateGrip::SetUpUI()
{
	int val;
	GetEditPolyMod()->getParamBlock()->GetValue (epm_tess_type,  GetCOREInterface()->GetTime(), val,FOREVER);
	if( 1 == val )
		GetIGripManager()->ActivateGripItem(eTessellateTension, false);
	else
		GetIGripManager()->ActivateGripItem(eTessellateTension, true);

	GetIGripManager()->ResetAllUI();
}

bool TessellateGrip::SetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eTessellateType:
		if(value.mInt>1) //go back to zero
			value.mInt =0;
		if(value.mInt<0)
			value.mInt = 1;
		if( 1 == value.mInt )
			GetIGripManager()->ActivateGripItem(eTessellateTension, false);
		else
			GetIGripManager()->ActivateGripItem(eTessellateTension,true);
		GetEditPolyMod()->getParamBlock()->SetValue (epm_tess_type, t, value.mInt);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_tess_type, t, value.mInt,FOREVER);
		break;
	case eTessellateTension:
		GetEditPolyMod()->getParamBlock()->SetValue (epm_tess_tension, t, value.mFloat);
		GetEditPolyMod()->getParamBlock()->GetValue (epm_tess_tension, t, value.mFloat,FOREVER);
		break;
	default:
		return false;

	}
	return true;
}

void TessellateGrip::Apply(TimeValue t)
{
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitAndRepeat (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void TessellateGrip::Okay(TimeValue t)
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,t);
	GetEditPolyMod()->EpModCommitUnlessAnimating (t);
	GetEditPolyMod()->EpModRefreshScreen ();
	EndSetValue(-1,t,true);

}

void TessellateGrip::Cancel()
{
	GetPolyOperation()->HideGrip();
	StartSetValue(-1,0);
	GetEditPolyMod()->EpModCancel();
	EndSetValue(-1,0,true);
	GetEditPolyMod()->EpModRefreshScreen ();
}

bool TessellateGrip::StartSetValue(int ,TimeValue t)
{
	theHold.Begin ();
	/*	int type = GetEditPolyMod()->GetPolyOperationID ();
	if (type != ep_op_Tessellate) {
	GetEditPolyMod()->EpModSetOperation (ep_op_Tessellate);
	} 
	*/
	return true;
}

bool TessellateGrip::EndSetValue(int ,TimeValue t,bool accept)
{
	if (accept)
	{
		if(theHold.Holding())
			theHold.Accept (GetString(IDS_TESSELLATE));

	}
	else
	{
		if(theHold.Holding())
			theHold.Cancel();
		GetEditPolyMod()->EpModRefreshScreen ();	
	}

	return true;
}



//we don't auto scale
bool TessellateGrip::GetAutoScale(int which)
{
	return true; //yes
}
//default scale
bool TessellateGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
	{
	case eTessellateTension:
		scaleValue.mFloat = 1.0f;
		break;
	default:
		return false;
	}
	return true;
}
bool TessellateGrip::GetScaleInViewSpace(int which, float &depth)
{
	depth = -200.0f; 
	return false;
}
bool TessellateGrip::ResetValue(TimeValue t, int which)
{
	float fResetVal =0.0f;
	int  iResetVal = 0;

	switch(which)
	{
	case eTessellateType:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_tess_type, t, iResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_tess_type, t, iResetVal,FOREVER);
			theHold.Accept (GetString(IDS_TESSELLATE));
		}
		break;
	case eTessellateTension:
		{
			theHold.Begin ();
			GetEditPolyMod()->getParamBlock()->SetValue (epm_tess_tension, t,fResetVal);
			GetCOREInterface()->RedrawViews (GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE);
			GetEditPolyMod()->getParamBlock()->GetValue(epm_tess_tension, t, fResetVal, FOREVER);
			theHold.Accept (GetString(IDS_TESSELLATE));
		}
		break;
	default:
		return false;
	}
	return true;

}

bool TessellateGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{
	ParamBlockDesc2* pbd = GetEditPolyMod()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eTessellateType:
		resetValue.mInt = 0;
		break;
	case eTessellateTension:
		{
			const ParamDef& pd = pbd->paramdefs[epm_tess_tension];
			resetValue.mFloat = pd.def.f;
		}
		break;
	default:
		return false;
	}
	return true;
}




/* work on later. for first prototype simply do normal one for bevel
//todo move to the right spot
//Generic poly operation grip. For a current poly operation this is the default grip for it.
class PolyOperationGrip :public IBaseGrip
{

public:


	PolyOperationGrip(EditPolyMod *m, PolyOperations *o);
	~PolyOperationGrip(){};
	int GetNumGrips();
	IBaseGrip::Type GetType(int which);
	void GetText(int which,TSTR &string);
	void GetValue(int which,IBaseGrip::GripValue &value);
	void SetValue(int which,IBaseGrip::GripValue &value); 
	bool GetAutoScale(int which);
	void GetScale(int which, IBaseGrip::GripValue &scaleValue);
	void GetResetValue(int which,IBaseGrip::GripValue &resetValue);
private:

	EditPolyMod						*mpEditPolyMod;
	PolyOperation *GetPolyOperation();
	int mNumOperations; //will mParamList and probably +1 for the action!
	Tab<int> mParamList; //list of params, just ints and floats...
};

PolyOperationGrip::PolyOperationGrip(EditPolyMod *m, PolyOperations *o)
{
	DbgAssert(m&&o);
	GetEditPolyMod() = m;
	GetPolyOperation()(o);
	//now we go through and find out how many we have .. this mirrors polyop to a big but for now 
	//we don't support point3 and matrix and so it may need to change
	
	mNumOperations = 0;
	mParamList.ZeroCount();
	IParamBlock2 *pBlock = GetEditPolyMod()->getParamBlock ();
	Tab<int> paramList;
	GetPolyOperation()->GetParameters (paramList);
	for (int i=0; i<paramList.Count(); i++)
	{
		int id = paramList[i];
		ParamType2 type = pBlock->GetParameterType (id);

		switch (type)
		{
		case TYPE_INT:
		case TYPE_BOOL:
			++mOperations;
			mParamList.Append(1,&id);

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


	}
}

*/
