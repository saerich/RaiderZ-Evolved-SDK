/*==============================================================================
Copyright 2009 Autodesk, Inc.  All rights reserved. 

This computer source code and related instructions and comments are the unpublished
confidential and proprietary information of Autodesk, Inc. and are protected under 
applicable copyright and trade secret law.  They may not be disclosed to, copied
or used by any third party without the prior written consent of Autodesk, Inc.

//**************************************************************************/

#include "EPoly.h"
#include "PolyEdit.h"
#include "maxscript\maxscript.h"

void EditablePolyManipulatorGrip::SetUpVisibility(BitArray &manipMask)
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
void EditablePolyManipulatorGrip::Okay(TimeValue t)
{
}
void EditablePolyManipulatorGrip::Cancel()
{
}
void EditablePolyManipulatorGrip::Apply(TimeValue t)
{
}

int EditablePolyManipulatorGrip::GetNumGripItems()
{
	return 9;
}
IBaseGrip::Type EditablePolyManipulatorGrip::GetType(int which)
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
	case eEdgeCrease:
		return IBaseGrip::eFloat;
		break;
	case eEdgeWeight:
		return IBaseGrip::eFloat;
		break;
	case eVertexWeight:
		return IBaseGrip::eFloat;
		break;
	}
	return IBaseGrip::eInt;
}

void EditablePolyManipulatorGrip::GetGripName(TSTR &string)
{
	string = TSTR(GetString(IDS_MANIPULATE));
}
bool EditablePolyManipulatorGrip::GetText(int which,TSTR &string)
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
	case eEdgeCrease:
		string = TSTR(GetString(IDS_EDGECREASE));
		break;
	case eEdgeWeight:
		string = TSTR(GetString(IDS_EDGEWEIGHT));
		break;
	case eVertexWeight:
		string = TSTR(GetString(IDS_VERTEXWEIGHT));
		break;
	default:
		return false;
	};
	return true ;
}

bool EditablePolyManipulatorGrip::GetResolvedIconName(int which,MSTR &string)
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
	case eEdgeCrease:
		string = EDGECREASE_PNG;
		break;
	case eEdgeWeight:
		string = EDGEWEIGHT_PNG;
		break;
	case eVertexWeight:
		string = VERTEXWEIGHT_PNG;
		break;
	default:
		return false;
	};
	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);

	return true;

}


DWORD EditablePolyManipulatorGrip::GetCustomization(int which)
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
	case eEdgeCrease:
		break;
	case eEdgeWeight:
		word = (int)(IBaseGrip::eSameRow);
		break;
	case eVertexWeight:
		break;
	};

	return word;
}

bool EditablePolyManipulatorGrip::GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &radioOptions)
{
	return false;
}
bool EditablePolyManipulatorGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	ParamDef theDef;
	switch(which)
	{
	case eFalloff:
		GetEditPolyObject()->getParamBlock()->GetValue (ep_ss_falloff, t, value.mFloat,FOREVER);
		break;
	case eBubble:
		GetEditPolyObject()->getParamBlock()->GetValue (ep_ss_bubble, t, value.mFloat,FOREVER);
		break;
	case ePinch:
		GetEditPolyObject()->getParamBlock()->GetValue (ep_ss_pinch, t, value.mFloat,FOREVER);
		break;
	case eSetFlow:
	case eLoopShift:
	case eRingShift:
		value.mInt = 0; //keep the value as zero for the spinner.
		break;
	case eEdgeCrease:
		{
			int num = 0;
			bool uniform(true);
			value.mFloat = GetEditPolyObject()->GetEdgeDataValue (EDATA_CREASE, &num, &uniform, MN_SEL, t);
		}
		break;
	case eEdgeWeight:
		{
			int num = 0;
			bool uniform(true);
			value.mFloat = GetEditPolyObject()->GetEdgeDataValue (EDATA_KNOT, &num, &uniform, MN_SEL, t);

		}
		break;
	case eVertexWeight:
		{
			int num = 0;
			bool uniform(true);
			value.mFloat = GetEditPolyObject()->GetVertexDataValue (VDATA_WEIGHT, &num, &uniform, MN_SEL, t);
		}
		break;
	default:
		return false;
	}
	return true;
}

bool EditablePolyManipulatorGrip::GetAutoScale(int which)
{
	switch(which)
	{
	case eFalloff:
	case eBubble:
	case ePinch:
	case eEdgeCrease:
	case eEdgeWeight:
	case eVertexWeight:
		return true;
	case eSetFlow:
	case eLoopShift:
	case eRingShift:
		return false;
	default:
		return false;
	}
	return false;
}
bool EditablePolyManipulatorGrip::GetScaleInViewSpace(int which, float &depth)
{
	if(which==eFalloff)
	{
		depth  = -200.0f;
		return true;
	}
	return false;
}

bool EditablePolyManipulatorGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
	{
	case eFalloff:
	case eBubble:
	case ePinch:
	case eEdgeCrease:
	case eEdgeWeight:
	case eVertexWeight:
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
bool EditablePolyManipulatorGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{
	  ParamBlockDesc2* pbd = GetEditPolyObject()->getParamBlock()->GetDesc();

	  switch(which)
	  {
	  case eFalloff:
		  {
			  const ParamDef& pd = pbd->paramdefs[ep_ss_falloff];
			  resetValue.mFloat = pd.def.f;
		  }
		  break;
	  case eBubble:
		  {
			  const ParamDef& pd = pbd->paramdefs[ep_ss_bubble];
			  resetValue.mFloat = pd.def.f;
		  }
		  break;
	  case ePinch:
		  {
			  const ParamDef& pd = pbd->paramdefs[ep_ss_pinch];
			  resetValue.mFloat = pd.def.f;
		  }
		  break;
	  case eEdgeCrease:
		  {
			  resetValue.mFloat = 0.0f;
		  }
		  break;
	  case eEdgeWeight:
		  {
			  resetValue.mFloat = 0.0f;
		  }
		  break;
	  case eVertexWeight:
		  {
			  resetValue.mFloat = 0.0f;
		  }
		  break;	
	  default:
		  return false;
	  }

	  return true;
}

bool EditablePolyManipulatorGrip::StartSetValue(int which,TimeValue t)
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
			theHold.Begin();  // (uses restore object in paramblock.)
			theHold.Put (new RingLoopRestore(GetEditPolyObject()));
			if ( which == eRingShift)
				theHold.Accept(GetString(IDS_SELECT_EDGE_RING));
			else 
				theHold.Accept(GetString(IDS_SELECT_EDGE_LOOP));
			
			bool alt =  (GetKeyState(VK_MENU)&0x8000) ?  true : false; 
			bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) ? true : false; 
			if(ctrl)
			{
				GetEditPolyObject()->setAltKey(false);
				GetEditPolyObject()->setCtrlKey(true);
			}
			else if(alt)
			{
				GetEditPolyObject()->setCtrlKey(false );
				GetEditPolyObject()->setAltKey(true);
			}
		}
		break;
	case eEdgeCrease:
		{
			if (!GetEditPolyObject()->GetMeshPtr()->nume) break;
			GetEditPolyObject()->BeginPerDataModify (MNM_SL_EDGE, EDATA_CREASE);
		}
		break;
	case eEdgeWeight:
		{
			if (!GetEditPolyObject()->GetMeshPtr()->nume) break;
			GetEditPolyObject()->BeginPerDataModify (MNM_SL_EDGE, EDATA_KNOT);
		}
		break;
	case eVertexWeight:
		{
			if (!GetEditPolyObject()->GetMeshPtr()->numv) break;
			GetEditPolyObject()->BeginPerDataModify (MNM_SL_VERTEX, VDATA_WEIGHT);
		}
		break;
	default:
		return false;
	}
	return true;
}

bool EditablePolyManipulatorGrip::SetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	ParamDef theDef;
	switch(which)
	{
	case eFalloff:
		GetEditPolyObject()->getParamBlock()->SetValue (ep_ss_falloff, t, value.mFloat);
		GetEditPolyObject()->getParamBlock()->GetValue (ep_ss_falloff, t, value.mFloat,FOREVER);
		break;
	case eBubble:
		GetEditPolyObject()->getParamBlock()->SetValue (ep_ss_bubble, t, value.mFloat);
		GetEditPolyObject()->getParamBlock()->GetValue (ep_ss_bubble, t, value.mFloat,FOREVER);
		break;
	case ePinch:
		GetEditPolyObject()->getParamBlock()->SetValue (ep_ss_pinch, t, value.mFloat);
		GetEditPolyObject()->getParamBlock()->GetValue (ep_ss_pinch, t, value.mFloat,FOREVER);
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
			int l_spinVal =  GetEditPolyObject()->getLoopValue();
			if(value.mInt>0)
			{
				++l_spinVal;
			}
			else
			{
				--l_spinVal;
			}
			GetEditPolyObject()->UpdateLoopEdgeSelection(l_spinVal);
			value.mInt = 0;
		}
		break;
	case eRingShift:
		{
			int l_spinVal =  GetEditPolyObject()->getRingValue();
			if(value.mInt>0)
			{
				++l_spinVal;
			}
			else
			{
				--l_spinVal;
			}
			GetEditPolyObject()->UpdateRingEdgeSelection(l_spinVal);
			value.mInt = 0;
		}
		break;
	case eEdgeCrease: 
		{
			if (!GetEditPolyObject()->GetMeshPtr()->nume) break;
			GetEditPolyObject()->SetEdgeDataValue (EDATA_CREASE, value.mFloat, MN_SEL, t);
		}
		break;
	case eEdgeWeight:
		{
			if (!GetEditPolyObject()->GetMeshPtr()->nume) break;
			GetEditPolyObject()->SetEdgeDataValue (EDATA_KNOT, value.mFloat, MN_SEL, t);
		}
		break;
	case eVertexWeight:
		{
			if (!GetEditPolyObject()->GetMeshPtr()->numv) break;
			GetEditPolyObject()->SetVertexDataValue (VDATA_WEIGHT, value.mFloat, MN_SEL, t);
		}
		break;
	default:
		return false;
	}
	return true;
}

bool EditablePolyManipulatorGrip::EndSetValue(int which,TimeValue t,bool accepted)
{
	ParamDef theDef;
	switch(which)
	{
	case eFalloff:
		if(accepted)
			theHold.Accept (TSTR(GetString(IDS_FALLOFF)));
		else
			theHold.Cancel();
		break;
	case eBubble:
		if(accepted)
			theHold.Accept (TSTR(GetString(IDS_BUBBLE)));
		else
			theHold.Cancel();
		break;
	case ePinch:
		if(accepted)
			theHold.Accept (TSTR(GetString(IDS_PINCH)));
		else 
			theHold.Cancel();
		break;
	case eSetFlow:
		ExecuteMAXScriptScript(_T("PolyBAdjustUndo PolyBoost.FlowSel"), TRUE);//this is from PolyTools.ms, SetFlowSpinnerCallback
		break;
	case eLoopShift:
	case eRingShift:	
		// reset keys
		GetEditPolyObject()->setAltKey(false);
		GetEditPolyObject()->setCtrlKey(false );
		break;
	case eEdgeCrease:
		{
			if (!GetEditPolyObject()->GetMeshPtr()->nume) break;
			if(accepted)
			{
				theHold.Begin ();
				GetEditPolyObject()->EndPerDataModify (true);
				theHold.Accept (GetString(IDS_CHANGE_CREASE_VALS));
			}
			else
				GetEditPolyObject()->EndPerDataModify (false);

		}
		break;
	case eEdgeWeight:
		{
			if (!GetEditPolyObject()->GetMeshPtr()->nume) break;
			if(accepted)
			{
				theHold.Begin ();
				GetEditPolyObject()->EndPerDataModify (true);
				theHold.Accept (GetString(IDS_CHANGEWEIGHT));	
			}
			else
				GetEditPolyObject()->EndPerDataModify (false);

		}
		break;
	case eVertexWeight:
		{
			if (!GetEditPolyObject()->GetMeshPtr()->numv) break;
			if(accepted)
			{
				theHold.Begin ();
				GetEditPolyObject()->EndPerDataModify (true);
				theHold.Accept (GetString(IDS_CHANGEWEIGHT));
			}
			else
				GetEditPolyObject()->EndPerDataModify (false);
		}
		break;
	default:
		return false;
	}
	return true;
}

//not used, 
bool EditablePolyManipulatorGrip::ResetValue(TimeValue t, int which)
{
	float fResetValue =0.0f;

	switch(which)
	{
	case eFalloff:
		theHold.Begin ();
		GetEditPolyObject()->getParamBlock()->SetValue (ep_ss_falloff, t, fResetValue);
		theHold.Accept (GetString(IDS_FALLOFF));
		break;
	case eBubble:
		theHold.Begin();
		GetEditPolyObject()->getParamBlock()->SetValue (ep_ss_bubble, t,fResetValue);
		theHold.Accept (GetString(IDS_BUBBLE));
		break;
	case ePinch:
		theHold.Begin();
		GetEditPolyObject()->getParamBlock()->SetValue (ep_ss_pinch, t, fResetValue);
		theHold.Accept (GetString(IDS_PINCH));
		break;
	case eLoopShift:
	case eRingShift:
		//do something?
		break;
	case eEdgeCrease:
		{
			if (!GetEditPolyObject()->GetMeshPtr()->nume) break;
			theHold.Begin ();
			GetEditPolyObject()->SetEdgeDataValue (EDATA_CREASE, fResetValue, MN_SEL, t);
			theHold.Accept (GetString(IDS_CHANGE_CREASE_VALS));
		}
		break;
	case eEdgeWeight:
		{
			if (!GetEditPolyObject()->GetMeshPtr()->nume) break;
			theHold.Begin ();
			GetEditPolyObject()->SetEdgeDataValue (EDATA_KNOT, fResetValue, MN_SEL, t);
			theHold.Accept (GetString(IDS_CHANGEWEIGHT));	
		}
		break;
	case eVertexWeight:
		{
			if (!GetEditPolyObject()->GetMeshPtr()->numv) break;
			theHold.Begin ();
			GetEditPolyObject()->SetVertexDataValue (VDATA_WEIGHT, fResetValue, MN_SEL, t);
			theHold.Accept (GetString(IDS_CHANGEWEIGHT));
		}
		break;	
	default:
		return false;
	}
	return true;
}

bool EditablePolyManipulatorGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{
	ParamDef theDef;
	switch(which)
	{
	case eFalloff:
		theDef = GetEditPolyObject()->getParamBlock()->GetParamDef(ep_ss_falloff);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;
	case eBubble:
		theDef = GetEditPolyObject()->getParamBlock()->GetParamDef(ep_ss_bubble);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;
	case ePinch:
		theDef = GetEditPolyObject()->getParamBlock()->GetParamDef(ep_ss_pinch);
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
	case eEdgeCrease:
		minRange.mFloat = 0.0f;
		maxRange.mFloat = 1.0f;
		break;
	case eEdgeWeight:
	case eVertexWeight:
		minRange.mFloat = 0.0f;
		maxRange.mFloat = 9999999.0f;
		break;
	default:
		return false;
	}
	return true;
}

bool EditablePolyManipulatorGrip::ShowKeyBrackets(int which,TimeValue t)
{
	return false;
}

//MZ grip prototype.. Using Bevel

BevelGrip::~BevelGrip()
{

}

int BevelGrip::GetNumGripItems()
{
	return 3; 
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
			//GetEditPolyObj()->getParamBlock()->GetValue (ep_bevel_type, 0, type,FOREVER);
			GetEditPolyObj()->getParamBlock()->GetValue (ep_bevel_type, 0, type,FOREVER);
			switch(type)
			{
				case 0:
					string =  TSTR(GetString(IDS_GROUP));
					break;
				case 1:
					string =  TSTR(GetString(IDS_LOCAL_NORMAL));
					break;
				case 2:
					string =  TSTR(GetString(IDS_BY_POLYGON));
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
	DbgAssert(which>=0&&which<=2);
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
		label->mLabel = TSTR(GetString(IDS_GROUP));
		comboOptions.Append(1,&label);
		label = new IBaseGrip::ComboLabel();
		string = BEVEL_NORMAL_PNG;
		found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
		if(found)
			label->mIcon = string;
		label->mLabel = TSTR(GetString(IDS_LOCAL_NORMAL));
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
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_bevel_height);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eBevelOutline:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_bevel_outline);
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
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bevel_type, t, value.mInt,FOREVER);
		break;
	case eBevelHeight:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bevel_height, t, value.mFloat,FOREVER);
		break;
	case eBevelOutline:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bevel_outline, t, value.mFloat,FOREVER);
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
		GetEditPolyObj()->getParamBlock()->SetValue (ep_bevel_type, t, value.mInt);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bevel_type, t, value.mInt,FOREVER);
		break;
	case eBevelHeight:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_bevel_height, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bevel_height, t, value.mFloat,FOREVER);
		break;
	case eBevelOutline:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_bevel_outline, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bevel_outline, t, value.mFloat,FOREVER);
		break;
	default:
		return false;
		
	}
	return true;
}

void BevelGrip::Apply(TimeValue t)
{
	theHold.Begin ();	// so we can parcel the operation and the selection-clearing together.
	GetEditPolyObj()->EpPreviewAccept ();
	theHold.Accept (GetString (IDS_APPLY));
	GetEditPolyObj()->EpPreviewBegin (GetPolyID());
	
}

void BevelGrip::Okay(TimeValue t)
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewAccept ();
	GetEditPolyObj()->ClearOperationSettings ();

}

void BevelGrip::Cancel()
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewCancel ();
	GetEditPolyObj()->ClearOperationSettings ();
}

bool BevelGrip::StartSetValue(int which,TimeValue t)
{
	switch(which)
	{
	case eBevelOutline:
	case eBevelHeight:

		GetEditPolyObj()->EpPreviewSetDragging (true);
		break;
	default:
		return false;
	}
	theHold.Begin();
	return true;

}
bool BevelGrip::EndSetValue(int which,TimeValue t,bool accept)
{
	switch(which)
	{
	case eBevelOutline:
	case eBevelHeight:
		GetEditPolyObj()->EpPreviewSetDragging (false);
		GetEditPolyObj()->EpPreviewInvalidate ();
		// Need to tell system something's changed:
		GetEditPolyObj()->NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
		GetEditPolyObj()->RefreshScreen ();
		break;
	default:
	   return false;
	}

	if (theHold.Holding())
	{
		if(accept)
			theHold.Accept (GetString (IDS_BEVEL));
		else
			theHold.Cancel();
	}

	return true;
}

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

bool BevelGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eBevelHeight:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_bevel_height,t)==TRUE)? true : false;
	case eBevelOutline:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_bevel_outline,t)==TRUE) ? true : false;
	default:
		return false;
	}

	return false;
}


bool BevelGrip::ResetValue(TimeValue t, int which)
{
    const float resetVal = 0.0f;
	switch(which)
	{
	case eBevelType:
			GetEditPolyObj()->getParamBlock()->SetValue (ep_bevel_type, t,0);
			break;
	case eBevelHeight:
		{
			GetEditPolyObj()->getParamBlock()->SetValue (ep_bevel_height, t,resetVal);
		}
		break;
	case eBevelOutline:
		{
			GetEditPolyObj()->getParamBlock()->SetValue (ep_bevel_outline, t,resetVal);
		}
		break;
	default:
		return false;
	}
	return true;

}

bool BevelGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{

	ParamBlockDesc2* pbd = GetEditPolyObj()->getParamBlock()->GetDesc();
	
	switch(which)
	{
	case eBevelType:
			resetValue.mInt = 0;
			break;
	case eBevelHeight:
		{
			const ParamDef& pd = pbd->paramdefs[ep_bevel_height];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eBevelOutline:
		{
			const ParamDef& pd = pbd->paramdefs[ep_bevel_outline];
			resetValue.mFloat = pd.def.f;
		}
		break;
	default:
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Extrude Face Grips
ExtrudeFaceGrip::~ExtrudeFaceGrip()
{

}

int ExtrudeFaceGrip::GetNumGripItems()
{
	return 2; 
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
			//Group Normals, 1 for Local Normals, 2 for By Polygon 
			int type;
			GetEditPolyObj()->getParamBlock()->GetValue (ep_extrusion_type, 0, type,FOREVER);
			switch(type)
			{
			case 0:
				string = TSTR(GetString(IDS_GROUP));
				break;
			case 1:
				string = TSTR(GetString(IDS_LOCAL_NORMAL));
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

bool ExtrudeFaceGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{
	ParamDef theDef;
	switch(which)
	{
	case eExtrudeFaceHeight:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_face_extrude_height);
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
		label->mLabel = TSTR(GetString(IDS_GROUP));
		comboOptions.Append(1,&label);
		label = new IBaseGrip::ComboLabel();
		string = EXTRUDE_NORMAL_PNG;
		found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
		if(found)
			label->mIcon = string;
		label->mLabel = TSTR(GetString(IDS_LOCAL_NORMAL));
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
bool ExtrudeFaceGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eExtrudeFaceType:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_extrusion_type, t, value.mInt,FOREVER);
		break;
	case eExtrudeFaceHeight:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_face_extrude_height, t, value.mFloat,FOREVER);
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
		GetEditPolyObj()->getParamBlock()->SetValue (ep_extrusion_type, t, value.mInt);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_extrusion_type, t, value.mInt,FOREVER);
		break;
	case eExtrudeFaceHeight:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_face_extrude_height, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_face_extrude_height, t, value.mFloat,FOREVER);
		break;
	default:
		return false;

	}
	return true;
}

void ExtrudeFaceGrip::Apply(TimeValue t)
{
	theHold.Begin ();	// so we can parcel the operation and the selection-clearing together.
	GetEditPolyObj()->EpPreviewAccept ();
	theHold.Accept (GetString (IDS_APPLY));
	GetEditPolyObj()->EpPreviewBegin (GetPolyID());

}

void ExtrudeFaceGrip::Okay(TimeValue t)
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewAccept ();
	GetEditPolyObj()->ClearOperationSettings ();

}

void ExtrudeFaceGrip::Cancel()
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewCancel ();
	GetEditPolyObj()->ClearOperationSettings ();
}

bool ExtrudeFaceGrip::StartSetValue(int which,TimeValue t)
{
	switch(which)
	{
	case eExtrudeFaceHeight:
		GetEditPolyObj()->EpPreviewSetDragging (true);
		break;
	default:
		return false;
	}
	theHold.Begin();
	return true;
}
bool ExtrudeFaceGrip::EndSetValue(int which,TimeValue t,bool accept)
{
	switch(which)
	{
	case eExtrudeFaceHeight:
		{
			GetEditPolyObj()->EpPreviewSetDragging (false);
			GetEditPolyObj()->EpPreviewInvalidate ();
			// Need to tell system something's changed:
			GetEditPolyObj()->NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
			GetEditPolyObj()->RefreshScreen ();
		}
		break;
	default:
		return false;
	}


	if (theHold.Holding())
	{
		if(accept)
			theHold.Accept (GetString(IDS_FACE_EXTRUDE));
		else
			theHold.Cancel();
	}
	return true;
}

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

bool ExtrudeFaceGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eExtrudeFaceHeight:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_face_extrude_height,t)==TRUE)? true : false;
	default:
		return false;
	}

	return false;
}


bool ExtrudeFaceGrip::ResetValue(TimeValue t, int which)
{
	const float fResetVal = 0.0f;

	switch(which)
	{
	case eExtrudeFaceType:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_extrusion_type, t,0);
			theHold.Accept (GetString(IDS_EXTRUDE));
		}
		break;
	case eExtrudeFaceHeight:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_face_extrude_height, t,fResetVal);
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

	ParamBlockDesc2* pbd = GetEditPolyObj()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eExtrudeFaceType:
		resetValue.mInt = 0;
		break;
	case eExtrudeFaceHeight:
		{
			const ParamDef& pd = pbd->paramdefs[ep_face_extrude_height];
			resetValue.mFloat = pd.def.f;
		}
		break;
	default:
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Extrude Edge Grips
ExtrudeEdgeGrip::~ExtrudeEdgeGrip()
{

}

int ExtrudeEdgeGrip::GetNumGripItems()
{
	return 2; 
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

DWORD ExtrudeEdgeGrip::GetCustomization(int which)
{
	return 0;
}

bool ExtrudeEdgeGrip::GetResolvedIconName(int which,MSTR &string)
{
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

bool ExtrudeEdgeGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{
	ParamDef theDef;
	switch(which)
	{
	case eExtrudeEdgeHeight:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_edge_extrude_height);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eExtrudeEdgeWidth:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_edge_extrude_width);
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

bool ExtrudeEdgeGrip::GetComboOptions(int, Tab<IBaseGrip::ComboLabel*> &)
{
	//no combo buttons
	return false;
}

bool ExtrudeEdgeGrip::GetCommandIcon(int which, MSTR &string)
{
	return false;
}

bool ExtrudeEdgeGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eExtrudeEdgeHeight:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_edge_extrude_height, t, value.mFloat,FOREVER);
		break;
	case eExtrudeEdgeWidth:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_edge_extrude_width, t, value.mFloat,FOREVER);
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
		GetEditPolyObj()->getParamBlock()->SetValue (ep_edge_extrude_height, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_edge_extrude_height, t, value.mFloat,FOREVER);
		break;
	case eExtrudeEdgeWidth:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_edge_extrude_width, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_edge_extrude_width, t, value.mFloat,FOREVER);
		break;
	default:
		return false;

	}
	return true;
}

void ExtrudeEdgeGrip::Apply(TimeValue t)
{
	theHold.Begin ();	// so we can parcel the operation and the selection-clearing together.
	GetEditPolyObj()->EpPreviewAccept ();
	theHold.Accept (GetString (IDS_APPLY));
	GetEditPolyObj()->EpPreviewBegin (GetPolyID());

}

void ExtrudeEdgeGrip::Okay(TimeValue t)
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewAccept ();
	GetEditPolyObj()->ClearOperationSettings ();

}

void ExtrudeEdgeGrip::Cancel()
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewCancel ();
	GetEditPolyObj()->ClearOperationSettings ();
}

bool ExtrudeEdgeGrip::StartSetValue(int which,TimeValue t)
{
	switch(which)
	{
	case eExtrudeEdgeHeight:
	case eExtrudeEdgeWidth:
		GetEditPolyObj()->EpPreviewSetDragging (true);
		break;
	default:
		return false;
	}
	theHold.Begin();
	return true;
}
bool ExtrudeEdgeGrip::EndSetValue(int which,TimeValue t,bool accept)
{
	switch(which)
	{
	case eExtrudeEdgeHeight:
	case eExtrudeEdgeWidth:
		{
			GetEditPolyObj()->EpPreviewSetDragging (false);
			GetEditPolyObj()->EpPreviewInvalidate ();
			// Need to tell system something's changed:
			GetEditPolyObj()->NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
			GetEditPolyObj()->RefreshScreen ();
		}
		break;
	default:
		return false;
	}


	if (theHold.Holding())
	{
		if(accept)
			theHold.Accept (GetString(IDS_EDGE_EXTRUDE));
		else
			theHold.Cancel();
	}
	return true;
}

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

bool ExtrudeEdgeGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eExtrudeEdgeHeight:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_edge_extrude_height,t)==TRUE)? true : false;
	case eExtrudeEdgeWidth:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_edge_extrude_width,t)==TRUE) ? true : false;
	default:
		return false;
	}

	return false;
}


bool ExtrudeEdgeGrip::ResetValue(TimeValue t, int which)
{
	const float fResetVal = 0.0f;

	switch(which)
	{
	case eExtrudeEdgeHeight:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_edge_extrude_height, t,fResetVal);
			theHold.Accept (GetString(IDS_EXTRUDE));
		}
		break;
	case eExtrudeEdgeWidth:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_edge_extrude_width, t,fResetVal);
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
	ParamBlockDesc2* pbd = GetEditPolyObj()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eExtrudeEdgeHeight:
		{
			const ParamDef& pd = pbd->paramdefs[ep_edge_extrude_height];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eExtrudeEdgeWidth:
		{
			const ParamDef& pd = pbd->paramdefs[ep_edge_extrude_width];
			resetValue.mFloat = pd.def.f;
		}
		break;
	default:
		return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Extrude Vertex Grips
ExtrudeVertexGrip::~ExtrudeVertexGrip()
{

}

int ExtrudeVertexGrip::GetNumGripItems()
{
	return 2; 
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

bool ExtrudeVertexGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{

	ParamDef theDef;
	switch(which)
	{
	case eExtrudeVertexHeight:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_vertex_extrude_height);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eExtrudeVertexWidth:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_vertex_extrude_width);
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

bool ExtrudeVertexGrip::GetCommandIcon(int which, MSTR &string)
{
	return false;
}

bool ExtrudeVertexGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eExtrudeVertexHeight:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_vertex_extrude_height, t, value.mFloat,FOREVER);
		break;
	case eExtrudeVertexWidth:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_vertex_extrude_width, t, value.mFloat,FOREVER);
		break;
	default:
		return false;
	}
	return true;
}

bool ExtrudeVertexGrip::GetComboOptions(int, Tab<IBaseGrip::ComboLabel*> &)
{
	//no combo buttons
	return false;
}

bool ExtrudeVertexGrip::SetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eExtrudeVertexHeight:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_vertex_extrude_height, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_vertex_extrude_height, t, value.mFloat,FOREVER);
		break;
	case eExtrudeVertexWidth:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_vertex_extrude_width, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_vertex_extrude_width, t, value.mFloat,FOREVER);
		break;
	default:
		return false;

	}
	return true;
	
}

void ExtrudeVertexGrip::Apply(TimeValue t)
{
	theHold.Begin ();	// so we can parcel the operation and the selection-clearing together.
	GetEditPolyObj()->EpPreviewAccept ();
	theHold.Accept (GetString (IDS_APPLY));
	GetEditPolyObj()->EpPreviewBegin (GetPolyID());

}

void ExtrudeVertexGrip::Okay(TimeValue t)
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewAccept ();
	GetEditPolyObj()->ClearOperationSettings ();

}

void ExtrudeVertexGrip::Cancel()
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewCancel ();
	GetEditPolyObj()->ClearOperationSettings ();
}

bool ExtrudeVertexGrip::StartSetValue(int which,TimeValue t)
{
	switch(which)
	{
	case eExtrudeVertexHeight:
	case eExtrudeVertexWidth:
		GetEditPolyObj()->EpPreviewSetDragging (true);
		break;
	default:
		return false;
	}
	theHold.Begin();
	return true;
}
bool ExtrudeVertexGrip::EndSetValue(int which,TimeValue t,bool accept)
{

	switch(which)
	{
	case eExtrudeVertexHeight:
	case eExtrudeVertexWidth:
		{
			GetEditPolyObj()->EpPreviewSetDragging (false);
			GetEditPolyObj()->EpPreviewInvalidate ();
			// Need to tell system something's changed:
			GetEditPolyObj()->NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
			GetEditPolyObj()->RefreshScreen ();
		}
		break;
	default:
		return false;
	}


	if(theHold.Holding())
	{
		if(accept)
			theHold.Accept (GetString(IDS_VERTEX_EXTRUDE));
		else
			theHold.Cancel();
	}
	return true;
}

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

bool ExtrudeVertexGrip::ShowKeyBrackets(int which,TimeValue t)
{

	switch(which)
	{
	case eExtrudeVertexHeight:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_vertex_extrude_height,t)==TRUE)? true : false;
	case eExtrudeVertexWidth:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_vertex_extrude_width,t)==TRUE) ? true : false;
	default:
		return false;
	}

	return false;
}


bool ExtrudeVertexGrip::ResetValue(TimeValue t, int which)
{
	const float fResetVal = 0.0f;

	switch(which)
	{
	case eExtrudeVertexHeight:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_vertex_extrude_height, t,fResetVal);
			theHold.Accept (GetString(IDS_EXTRUDE));
		}
		break;
	case eExtrudeVertexWidth:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_vertex_extrude_width, t,fResetVal);
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

	ParamBlockDesc2* pbd = GetEditPolyObj()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eExtrudeVertexHeight:
		{
			const ParamDef& pd = pbd->paramdefs[ep_vertex_extrude_height];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eExtrudeVertexWidth:
		{
			const ParamDef& pd = pbd->paramdefs[ep_vertex_extrude_width];
			resetValue.mFloat = pd.def.f;
		}
		break;
	default:
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
	DbgAssert(which ==0 );
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

bool OutlineGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{

	ParamDef theDef;
	switch(which)
	{
	case eOutlineAmount:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_outline);
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
		GetEditPolyObj()->getParamBlock()->GetValue (ep_outline, t, value.mFloat,FOREVER);
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
		GetEditPolyObj()->getParamBlock()->SetValue (ep_outline, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_outline, t, value.mFloat,FOREVER);
		break;
	default:
		return false;
	}
	return true;
	
}

void OutlineGrip::Apply(TimeValue t)
{
	theHold.Begin ();	// so we can parcel the operation and the selection-clearing together.
	GetEditPolyObj()->EpPreviewAccept ();
	theHold.Accept (GetString (IDS_APPLY));
	GetEditPolyObj()->EpPreviewBegin (GetPolyID());

}

void OutlineGrip::Okay(TimeValue t)
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewAccept ();
	GetEditPolyObj()->ClearOperationSettings ();

}

void OutlineGrip::Cancel()
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewCancel ();
	GetEditPolyObj()->ClearOperationSettings ();
}

bool OutlineGrip::StartSetValue(int which,TimeValue t)
{
	switch(which)
	{
	case eOutlineAmount:
		GetEditPolyObj()->EpPreviewSetDragging (true);
		break;
	default:
		return false;
	}
	theHold.Begin();
	return true;
}
bool OutlineGrip::EndSetValue(int which,TimeValue t,bool accept)
{

	switch(which)
	{
	case eOutlineAmount:
		{
			GetEditPolyObj()->EpPreviewSetDragging (false);
			GetEditPolyObj()->EpPreviewInvalidate ();
			// Need to tell system something's changed:
			GetEditPolyObj()->NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
			GetEditPolyObj()->RefreshScreen ();
		}
		break;
	default:
		return false;
	}


	if(theHold.Holding())
	{
		if(accept)
			theHold.Accept (GetString(IDS_OUTLINE));
		else
			theHold.Cancel();
	}
	return true;
}

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

bool OutlineGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eOutlineAmount:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_outline,t)==TRUE) ? true : false;
	default:
		return false;
	}

	return false;
}


bool OutlineGrip::ResetValue(TimeValue t, int which)
{
	const float fResetVal =0.0f;

	switch(which)
	{
	case eOutlineAmount:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_outline, t,fResetVal);
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
	ParamBlockDesc2* pbd = GetEditPolyObj()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eOutlineAmount:
		{
			const ParamDef& pd = pbd->paramdefs[ep_outline];
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
			//Group, 1 for By Polygon 
			int type;
			//GetEditPolyObj()->getParamBlock()->GetValue (ep_Inset_type, 0, type,FOREVER);
			GetEditPolyObj()->getParamBlock()->GetValue (ep_inset_type, 0, type,FOREVER);
			switch(type)
			{
			case 0:
				string =TSTR(GetString(IDS_GROUP));
				break;
			case 1:
				string =TSTR(GetString(IDS_BY_POLYGON));
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

bool InsetGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{

	ParamDef theDef;
	switch(which)
	{
	case eInsetAmount:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_inset);
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
		GetEditPolyObj()->getParamBlock()->GetValue (ep_inset_type, t, value.mInt,FOREVER);
		break;
	case eInsetAmount:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_inset, t, value.mFloat,FOREVER);
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
		GetEditPolyObj()->getParamBlock()->SetValue (ep_inset_type, t, value.mInt);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_inset_type, t, value.mInt,FOREVER);
		break;
	case eInsetAmount:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_inset, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_inset, t, value.mFloat,FOREVER);
		break;
	default:
		return false;
	}
	return true;
	
}

void InsetGrip::Apply(TimeValue t)
{
	theHold.Begin ();	// so we can parcel the operation and the selection-clearing together.
	GetEditPolyObj()->EpPreviewAccept ();
	theHold.Accept (GetString (IDS_APPLY));
	GetEditPolyObj()->EpPreviewBegin (GetPolyID());

}

void InsetGrip::Okay(TimeValue t)
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewAccept ();
	GetEditPolyObj()->ClearOperationSettings ();

}

void InsetGrip::Cancel()
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewCancel ();
	GetEditPolyObj()->ClearOperationSettings ();
}

bool InsetGrip::StartSetValue(int which,TimeValue t)
{
	switch(which)
	{
	case eInsetAmount:
		GetEditPolyObj()->EpPreviewSetDragging (true);
		break;
	default:
		return false;
	}
	theHold.Begin();
	return true;
}
bool InsetGrip::EndSetValue(int which,TimeValue t,bool accept)
{

	switch(which)
	{
	case eInsetAmount:
		{
			GetEditPolyObj()->EpPreviewSetDragging (false);
			GetEditPolyObj()->EpPreviewInvalidate ();
			// Need to tell system something's changed:
			GetEditPolyObj()->NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
			GetEditPolyObj()->RefreshScreen ();
		}
		break;
	default:
	   return false;
	}


	if(theHold.Holding())
	{
		if(accept)
			theHold.Accept (GetString(IDS_INSET));
		else
			theHold.Cancel();
	}
	return true;
}

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

bool InsetGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eInsetAmount:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_inset,t)==TRUE)? true : false;
	default:
		return false;
	}

	return false;
}


bool InsetGrip::ResetValue(TimeValue t, int which)
{
	const float fResetVal =0.0f;

	switch(which)
	{
	case eInsetType:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_inset_type, t,0);
			theHold.Accept (GetString(IDS_INSET));
		}
		break;
	case eInsetAmount:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_inset, t,fResetVal);
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

	ParamBlockDesc2* pbd = GetEditPolyObj()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eInsetType:
		resetValue.mInt = 0;
		break;
	case eInsetAmount:
		{
			const ParamDef& pd = pbd->paramdefs[ep_inset];
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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



bool ConnectEdgeGrip::GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &comboOptions)
{
	// have no radio options
	return false;
}

bool ConnectEdgeGrip::GetCommandIcon(int which, MSTR &string)
{
	return false;
}

bool ConnectEdgeGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{

	ParamDef theDef;
	switch(which)
	{
	case eConnectEdgeSegments:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_connect_edge_segments);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mInt = theDef.range_low.i;
			maxRange.mInt = theDef.range_high.i;
		}
		break;

	case eConnectEdgePinch:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_connect_edge_pinch);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mInt = theDef.range_low.i;
			maxRange.mInt = theDef.range_high.i;
		}
		break;

	case eConnectEdgeSlide:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_connect_edge_slide);
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

bool ConnectEdgeGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eConnectEdgeSegments:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_connect_edge_segments, t, value.mInt,FOREVER);
		break;
	case eConnectEdgePinch:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_connect_edge_pinch, t, value.mInt,FOREVER);
		break;
	case eConnectEdgeSlide:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_connect_edge_slide, t, value.mInt,FOREVER);
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
		GetEditPolyObj()->getParamBlock()->SetValue (ep_connect_edge_segments, t, value.mInt);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_connect_edge_segments, t, value.mInt,FOREVER);
		break;
	case eConnectEdgePinch:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_connect_edge_pinch, t, value.mInt);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_connect_edge_pinch, t, value.mInt,FOREVER);
		break;
	case eConnectEdgeSlide:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_connect_edge_slide, t, value.mInt);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_connect_edge_slide, t, value.mInt,FOREVER);
		break;
	default:
		return false;

	}
	return true;
	
}

void ConnectEdgeGrip::Apply(TimeValue t)
{
	theHold.Begin ();	// so we can parcel the operation and the selection-clearing together.
	GetEditPolyObj()->EpPreviewAccept ();
	theHold.Accept (GetString (IDS_APPLY));
	GetEditPolyObj()->EpPreviewBegin (GetPolyID());

}

void ConnectEdgeGrip::Okay(TimeValue t)
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewAccept ();
	GetEditPolyObj()->ClearOperationSettings ();

}

void ConnectEdgeGrip::Cancel()
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewCancel ();
	GetEditPolyObj()->ClearOperationSettings ();
}

bool ConnectEdgeGrip::StartSetValue(int which,TimeValue t)
{
	switch(which)
	{
	case eConnectEdgeSegments:
	case eConnectEdgePinch:
	case eConnectEdgeSlide:

		GetEditPolyObj()->EpPreviewSetDragging (true);
		break;
	default:
		return false;
	}
	theHold.Begin();
	return true;
}
bool ConnectEdgeGrip::EndSetValue(int which,TimeValue t,bool accept)
{

	switch(which)
	{
	case eConnectEdgeSegments:
	case eConnectEdgePinch:
	case eConnectEdgeSlide:
		{
			GetEditPolyObj()->EpPreviewSetDragging (false);
			GetEditPolyObj()->EpPreviewInvalidate ();
			// Need to tell system something's changed:
			GetEditPolyObj()->NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
			GetEditPolyObj()->RefreshScreen ();
		}
		break;
	default:
		return false;
	}


	if(theHold.Holding())
	{
		if(accept)
			theHold.Accept (GetString(IDS_CONNECT_EDGES));
		else
			theHold.Cancel();
	}
	return true;
}

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

bool ConnectEdgeGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eConnectEdgeSegments:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_connect_edge_segments,t)==TRUE)? true : false;
	case eConnectEdgePinch:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_connect_edge_pinch,t)==TRUE) ? true : false;
	case eConnectEdgeSlide:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_connect_edge_slide,t)==TRUE) ? true : false;
	default:
		return false;
	}

	return false;
}


bool ConnectEdgeGrip::ResetValue(TimeValue t, int which)
{
	const int iResetSegVal = 1;
	const int iResetVal = 0;

	switch(which)
	{
	case eConnectEdgeSegments:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_connect_edge_segments, t,iResetSegVal);
			theHold.Accept (GetString(IDS_CONNECT_EDGES));
		}
		break;
	case eConnectEdgePinch:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_connect_edge_pinch, t,iResetVal);
			theHold.Accept (GetString(IDS_CONNECT_EDGES));
		}
		break;
	case eConnectEdgeSlide:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_connect_edge_slide, t,iResetVal);
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

	ParamBlockDesc2* pbd = GetEditPolyObj()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eConnectEdgeSegments:
		{
			const ParamDef& pd = pbd->paramdefs[ep_connect_edge_segments];
			resetValue.mFloat = pd.def.i;
		}
		break;
	case eConnectEdgePinch:
		{
			const ParamDef& pd = pbd->paramdefs[ep_connect_edge_pinch];
			resetValue.mFloat = pd.def.i;
		}
		break;
	case eConnectEdgeSlide:
		{
			const ParamDef& pd = pbd->paramdefs[ep_connect_edge_slide];
			resetValue.mFloat = pd.def.i;
		}
		break;
	default:
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Chamfer Vertices Grip

ChamferVerticesGrip::~ChamferVerticesGrip()
{

}

int ChamferVerticesGrip::GetNumGripItems()
{
	return 2; 
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
	switch(which)
	{
	case eChamferVerticesAmount:
		string =  CHAMFER_VERTICES_AMOUNT_PNG;
		break;
	case eChamferVerticesOpen:
		string =  CHAMFER_VERTICES_OPEN_PNG;
		break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

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
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_vertex_chamfer);
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
		{
			GetEditPolyObj()->getParamBlock()->GetValue (ep_vertex_chamfer, t, value.mFloat,FOREVER);
		}
		break;
	case eChamferVerticesOpen:
		{
			int val = 0 ;
			GetEditPolyObj()->getParamBlock()->GetValue (ep_vertex_chamfer_open, t, val,FOREVER);
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
		{
			if( value.mFloat < 0.0f )
				value.mFloat = 0.0f ;
			GetEditPolyObj()->getParamBlock()->SetValue (ep_vertex_chamfer, t, value.mFloat);
		    GetEditPolyObj()->getParamBlock()->GetValue (ep_vertex_chamfer, t, value.mFloat,FOREVER);
		}
		break;
	case eChamferVerticesOpen :
		{
			int val = (int)value.mbool ;
			GetEditPolyObj()->getParamBlock()->SetValue (ep_vertex_chamfer_open, t, val);
			GetEditPolyObj()->getParamBlock()->GetValue (ep_vertex_chamfer_open, t, val,FOREVER);
		}
		break;
	default:
		return false;

	}
	return true;
}

void ChamferVerticesGrip::Apply(TimeValue t)
{
	theHold.Begin ();	// so we can parcel the operation and the selection-clearing together.
	GetEditPolyObj()->EpPreviewAccept ();
	theHold.Accept (GetString (IDS_APPLY));
	GetEditPolyObj()->EpPreviewBegin (GetPolyID());

}

void ChamferVerticesGrip::Okay(TimeValue t)
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewAccept ();
	GetEditPolyObj()->ClearOperationSettings ();

}

void ChamferVerticesGrip::Cancel()
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewCancel ();
	GetEditPolyObj()->ClearOperationSettings ();
}

bool ChamferVerticesGrip::StartSetValue(int which,TimeValue t)
{
	switch(which)
	{
	case eChamferVerticesAmount:

		GetEditPolyObj()->EpPreviewSetDragging (true);
		break;
	default:
		return false;
	}
	theHold.Begin();
	return true;
}
bool ChamferVerticesGrip::EndSetValue(int which,TimeValue t,bool accept)
{

	switch(which)
	{
	case eChamferVerticesAmount:
		{
			GetEditPolyObj()->EpPreviewSetDragging (false);
			GetEditPolyObj()->EpPreviewInvalidate ();
			// Need to tell system something's changed:
			GetEditPolyObj()->NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
			GetEditPolyObj()->RefreshScreen ();
		}
		break;
	default:
	   return false;
	}


	if(theHold.Holding())
	{
		if(accept)
			theHold.Accept (GetString(IDS_CHAMFER));
		else
			theHold.Cancel();
	}
	return true;
}

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

bool ChamferVerticesGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{

	case eChamferVerticesAmount:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_vertex_chamfer,t)==TRUE)? true : false;
	case eChamferVerticesOpen:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_vertex_chamfer_open,t)==TRUE) ? true : false;
	default:
		return false;

	}

	return false;
}


bool ChamferVerticesGrip::ResetValue(TimeValue t, int which)
{
    const float fResetVal = 0.0f;

	switch(which)
	{
	case eChamferVerticesAmount:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_vertex_chamfer, t,fResetVal);
			theHold.Accept (GetString(IDS_CHAMFER));
		}
		break;
	case eChamferVerticesOpen:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_vertex_chamfer_open, t, 0);
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

	ParamBlockDesc2* pbd = GetEditPolyObj()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eChamferVerticesAmount:
		{
			const ParamDef& pd = pbd->paramdefs[ep_vertex_chamfer];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eChamferVerticesOpen:
		{
			const ParamDef& pd = pbd->paramdefs[ep_vertex_chamfer_open];
			resetValue.mbool = (pd.def.i == 0 ? false:true);
		}
		break;
	default:
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Chamfer Edge Grip

ChamferEdgeGrip::~ChamferEdgeGrip()
{

}

int ChamferEdgeGrip::GetNumGripItems()
{
	return 3; 
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
			string =TSTR(GetString(IDS_EDGE_CHAMFER_AMOUNT));
		}
		break;
	case eChamferEdgeSegments:
		{
			string =TSTR(GetString(IDS_CONNECT_EDGE_SEGMENTS));
		}
		break;
	case eChamferEdgeOpen:
		{
			string =TSTR(GetString(IDS_OPEN_CHAMFER));
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
		string = CHAMFER_EDGE_SEGMENT_PNG;
		break;
	case eChamferEdgeOpen:
		string =  CHAMFER_EDGE_OPEN_PNG;
		break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

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
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_edge_chamfer);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = 0.0f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eChamferEdgeSegments:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_edge_chamfer_segments);
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
		GetEditPolyObj()->getParamBlock()->GetValue (ep_edge_chamfer, t, value.mFloat,FOREVER);
		break;
	case eChamferEdgeSegments:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_edge_chamfer_segments, t, value.mInt,FOREVER);
		break;
	case eChamferEdgeOpen:
		{
			int val = 0 ;
			GetEditPolyObj()->getParamBlock()->GetValue (ep_edge_chamfer_open, t, val,FOREVER);
			value.mbool = ( val == 0 ? false:true );
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
		GetEditPolyObj()->getParamBlock()->SetValue (ep_edge_chamfer, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_edge_chamfer, t, value.mFloat,FOREVER);
		break;
	case eChamferEdgeSegments:
		if(value.mInt < 1)
			value.mInt = 1;
		GetEditPolyObj()->getParamBlock()->SetValue (ep_edge_chamfer_segments, t, value.mInt);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_edge_chamfer_segments, t, value.mInt,FOREVER);
		break;
	case eChamferEdgeOpen:
		{
			int val = (int)value.mbool ;
			GetEditPolyObj()->getParamBlock()->SetValue (ep_edge_chamfer_open, t, val);
		    GetEditPolyObj()->getParamBlock()->GetValue (ep_edge_chamfer_open, t, val,FOREVER);
		}
		break;
	default:
		return false;

	}
	return true;
}

void ChamferEdgeGrip::Apply(TimeValue t)
{
	theHold.Begin ();	// so we can parcel the operation and the selection-clearing together.
	GetEditPolyObj()->EpPreviewAccept ();
	theHold.Accept (GetString (IDS_APPLY));
	GetEditPolyObj()->EpPreviewBegin (GetPolyID());

}

void ChamferEdgeGrip::Okay(TimeValue t)
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewAccept ();
	GetEditPolyObj()->ClearOperationSettings ();

}

void ChamferEdgeGrip::Cancel()
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewCancel ();
	GetEditPolyObj()->ClearOperationSettings ();
}

bool ChamferEdgeGrip::StartSetValue(int which,TimeValue t)
{
	switch(which)
	{
	case eChamferEdgeAmount:
	case eChamferEdgeSegments:

		GetEditPolyObj()->EpPreviewSetDragging (true);
		break;
	default:
		return false;
	}
	theHold.Begin();
	return true;
}
bool ChamferEdgeGrip::EndSetValue(int which,TimeValue t,bool accept)
{

	switch(which)
	{
	case eChamferEdgeAmount:
	case eChamferEdgeSegments:
		{
			GetEditPolyObj()->EpPreviewSetDragging (false);
			GetEditPolyObj()->EpPreviewInvalidate ();
			// Need to tell system something's changed:
			GetEditPolyObj()->NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
			GetEditPolyObj()->RefreshScreen ();
		}
		break;
	default:
		return false;
	}


	if(theHold.Holding())
	{
		if(accept)
			theHold.Accept (GetString(IDS_CHAMFER));
		else
			theHold.Cancel();
	}
	return true;
}

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

bool ChamferEdgeGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{

	case eChamferEdgeAmount:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_edge_chamfer,t)==TRUE)? true : false;
	case eChamferEdgeSegments:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_edge_chamfer_segments,t)==TRUE) ? true : false;
	case eChamferEdgeOpen:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_edge_chamfer_open,t)==TRUE) ? true : false;
	default:
		return false;
	}

	return false;
}


bool ChamferEdgeGrip::ResetValue(TimeValue t, int which)
{
	const int iResetSegVal = 1;
	const float fResetVal = 0.0f;

	switch(which)
	{
	case eChamferEdgeAmount:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_edge_chamfer, t,fResetVal);
			theHold.Accept (GetString(IDS_CHAMFER));
		}
		break;
	case eChamferEdgeSegments:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_edge_chamfer_segments, t,iResetSegVal);
			theHold.Accept (GetString(IDS_CHAMFER));
		}
		break;
	case eChamferEdgeOpen:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_edge_chamfer_open, t, 0);
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

	ParamBlockDesc2* pbd = GetEditPolyObj()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eChamferEdgeAmount:
		{
			const ParamDef& pd = pbd->paramdefs[ep_edge_chamfer];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eChamferEdgeSegments:
		{
			const ParamDef& pd = pbd->paramdefs[ep_edge_chamfer_segments];
			resetValue.mFloat = pd.def.i;
		}
		break;
	case eChamferEdgeOpen:
		{ 
			const ParamDef& pd = pbd->paramdefs[ep_edge_chamfer_open];
			resetValue.mbool = (pd.def.i == 0 ? false:true);

		}
		break;
	default:
		return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bridge Polygon Grip

BridgePolygonGrip::~BridgePolygonGrip()
{

}

int BridgePolygonGrip::GetNumGripItems()
{
	return 9; 
}
IBaseGrip::Type BridgePolygonGrip::GetType(int which)
{
	DbgAssert(which>=0&&which<=8);
	switch(which)
	{
	case eBridgePolygonsSegments:
		return IBaseGrip::eInt;
	case eBridgePolygonsTaper:
		return IBaseGrip::eFloat;
	case eBridgePolygonsBias:
		return IBaseGrip::eFloat;
	case eBridgePolygonsSmooth:
		return IBaseGrip::eFloat;
	case eBridgePolygonsTwist1:
		return IBaseGrip::eInt;
	case eBridgePolygonsTwist2:
		return IBaseGrip::eInt;
	case eBridgePolygonsType:
		return IBaseGrip::eCombo;
	case eBridgePolygonsPick1:
		return IBaseGrip::eCommand;
	case eBridgePolygonsPick2:
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
	GetIGripManager()->ResetGripUI((int)eBridgePolygonsPick1);
}

void BridgePolygonGrip::SetPoly2Picked(int num)
{
	TSTR buf;
	buf.printf(_T("%d"), num);
	mPoly2Picked = TSTR(GetString(IDS_FACE)) + TSTR(" ") + buf;
	mPick2Icon = PICK_FULL_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgePolygonsPick2);
}

void BridgePolygonGrip::SetPoly1PickModeStarted()
{
	mPoly1Picked =  TSTR(GetString(IDS_BRIDGE_PICKING_POLYGON_1));
	mPick1Icon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgePolygonsPick1);
}

void BridgePolygonGrip::SetPoly1PickDisabled()
{
	mPoly1Picked =  TSTR(GetString(IDS_BRIDGE_PICK_POLYGON_1));
	mPick1Icon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgePolygonsPick1);
}

void BridgePolygonGrip::SetPoly2PickModeStarted()
{
	mPoly2Picked =  TSTR(GetString(IDS_BRIDGE_PICKING_POLYGON_2));
	mPick2Icon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgePolygonsPick2);
}

void BridgePolygonGrip::SetPoly2PickDisabled()
{
	mPoly2Picked =  TSTR(GetString(IDS_BRIDGE_PICK_POLYGON_2));
	mPick2Icon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgePolygonsPick2);
}


bool BridgePolygonGrip::GetText(int which,TSTR &string)
{
	switch(which)
	{
	case eBridgePolygonsSegments:
		{
			string =TSTR(GetString(IDS_SEGMENTS));
		}
		break;
	case eBridgePolygonsTaper:
		{
			string =TSTR(GetString(IDS_TAPER));
		}
		break;
	case eBridgePolygonsBias:
		{
			string =TSTR(GetString(IDS_BIAS));
		}
		break;
	case eBridgePolygonsSmooth:
		{
			string =TSTR(GetString(IDS_SMOOTH));
		}
		break;
	case eBridgePolygonsTwist1:
		{
			string =TSTR(GetString(IDS_TWIST1));
		}
		break;
	case eBridgePolygonsTwist2:
		{
			string =TSTR(GetString(IDS_TWIST2));
		}
		break;
	case eBridgePolygonsType:
		{
			//Group Normals, 1 for Local Normals, 2 for By Polygon 
			int type;
			//GetEditPolyObj()->getParamBlock()->GetValue (ep_BridgePolygons_type, 0, type,FOREVER);
			GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_selected, 0, type,FOREVER);
			switch(type)
			{
			case 0:
				string = TSTR(GetString(IDS_BRIDGE_SPECIFIC_POLYGONS));
				break;
			case 1:
				string = TSTR(GetString(IDS_BRIDGE_POLYGON_SELECTION));
				break;

			}
			break;
		}
	case eBridgePolygonsPick1:
		{
			string =  mPoly1Picked;
		}
		break;
	case eBridgePolygonsPick2:
		{
			string = mPoly2Picked;
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
	case eBridgePolygonsTaper:
	case eBridgePolygonsSmooth:
	case eBridgePolygonsTwist2:
	case eBridgePolygonsPick1:
	case eBridgePolygonsPick2:

		word = (int)(IBaseGrip::eSameRow);
	}
	return word;
}

bool BridgePolygonGrip::GetResolvedIconName(int which,MSTR &string)
{
	DbgAssert(which>=0&&which<=8);
	switch(which)
	{
	case eBridgePolygonsSegments:
		string =  BRIDGE_SEGMENTS_PNG;
		break;
	case eBridgePolygonsTaper:
		string = BRIDGE_TAPER_PNG;
		break;
	case eBridgePolygonsBias:
		string = BRIDGE_BIAS_PNG;
		break;
	case eBridgePolygonsSmooth:
		string = BRIDGE_SMOOTH_PNG;
		break;
	case eBridgePolygonsTwist1:
		string =  BRIDGE_TWIST1_PNG;
		break;
	case eBridgePolygonsTwist2:
		string = BRIDGE_TWIST2_PNG;
		break;
	case eBridgePolygonsType:
		string = BRIDGE_POLYGON_TYPE_PNG;
		break;
	case eBridgePolygonsPick1:
		string = BRIDGE_POLYGONS_PICK1_PNG;
		break;
	case eBridgePolygonsPick2:
		string = BRIDGE_POLYGONS_PICK2_PNG;
		break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

}



bool BridgePolygonGrip::GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &comboOptions)
{
	if(which== eBridgePolygonsType)
	{
		BOOL found;
		IBaseGrip::ComboLabel *label = new IBaseGrip::ComboLabel();
		TSTR string = BRIDGE_SELECTED_PNG;
		found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
		if(found)
			label->mIcon = string;
		label->mLabel = TSTR(GetString(IDS_BRIDGE_SPECIFIC_POLYGONS));
		comboOptions.Append(1,&label);
		label = new IBaseGrip::ComboLabel();
		string = BRIDGE_SPECIFIC_PNG;
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
	case eBridgePolygonsPick1:
		string = mPick1Icon;
		break;
	case eBridgePolygonsPick2:
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
	case eBridgePolygonsSegments:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_bridge_segments);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mInt = 1;
			maxRange.mInt = theDef.range_high.i;
		}
		break;

	case eBridgePolygonsTaper:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_bridge_taper);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eBridgePolygonsBias:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_bridge_bias);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eBridgePolygonsSmooth:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_bridge_smooth_thresh);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eBridgePolygonsTwist1:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_bridge_twist_1);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mInt = theDef.range_low.i;
			maxRange.mInt = theDef.range_high.i;
		}
		break;

	case eBridgePolygonsTwist2:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_bridge_twist_2);
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
	case eBridgePolygonsSegments:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_segments, t, value.mInt,FOREVER);
		break;
	case eBridgePolygonsTaper:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_taper, t, value.mFloat,FOREVER);
		break;
	case eBridgePolygonsBias:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_bias, t, value.mFloat,FOREVER);
		break;
	case eBridgePolygonsSmooth:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_smooth_thresh, t, value.mFloat,FOREVER);
		break;
	case eBridgePolygonsTwist1:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_twist_1, t, value.mInt,FOREVER);
		break;
	case eBridgePolygonsTwist2:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_twist_2, t, value.mInt,FOREVER);
		break;
	case eBridgePolygonsType:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_selected, t, value.mInt,FOREVER);
		break;
	

	case eBridgePolygonsPick1:
		{
			//value.mbool, 0 means not in pick mode, 1 does.
			if(GetEditPolyObj()->EpActionGetCommandMode()==epmode_pick_bridge_1) //in command mode 
				value.mbool = true;
			else
				value.mbool = false;
		}
		break;
	case eBridgePolygonsPick2:
		{
           if(GetEditPolyObj()->EpActionGetCommandMode()==epmode_pick_bridge_2) //in command mode 
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
	GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_selected, GetCOREInterface()->GetTime(), val,FOREVER);
	if( 1 == val )
	{
		GetIGripManager()->ActivateGripItem(eBridgePolygonsPick1,false);
		GetIGripManager()->ActivateGripItem(eBridgePolygonsPick2,false);
		SetPoly1PickDisabled();
		SetPoly2PickDisabled();
	}
	else
	{
		GetIGripManager()->ActivateGripItem(eBridgePolygonsPick1,true);
		GetIGripManager()->ActivateGripItem(eBridgePolygonsPick2,true);
		int targ = GetEditPolyObj()->getParamBlock ()->GetInt (ep_bridge_target_1);
		if(targ==0)
			SetPoly1PickDisabled();
		else
			SetPoly1Picked(targ-1); //need to -1 out
		targ = GetEditPolyObj()->getParamBlock()->GetInt (ep_bridge_target_2);
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
	case eBridgePolygonsSegments:
		if( value.mInt < 1)
			value.mInt = 1;
		GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_segments, t, value.mInt);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_segments, t, value.mInt,FOREVER);
		break;
	case eBridgePolygonsTaper:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_taper, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_taper, t, value.mFloat,FOREVER);
		break;
	case eBridgePolygonsBias:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_bias, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_bias, t, value.mFloat,FOREVER);
		break;
	case eBridgePolygonsSmooth:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_smooth_thresh, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_smooth_thresh, t, value.mFloat,FOREVER);
		break;
	case eBridgePolygonsType:
		if(value.mInt>1) //go back to zero
			value.mInt =0;
		if(value.mInt<0)
			value.mInt = 1;
		GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_selected, t, value.mInt);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_selected, t, value.mInt,FOREVER);
		if( 1 == value.mInt )
		{
			GetIGripManager()->ActivateGripItem(eBridgePolygonsPick1,false);
			GetIGripManager()->ActivateGripItem(eBridgePolygonsPick2,false);
		}
		else
		{
			GetIGripManager()->ActivateGripItem(eBridgePolygonsPick1,true);
			GetIGripManager()->ActivateGripItem(eBridgePolygonsPick2,true);
		}
		break;
	case eBridgePolygonsTwist1:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_twist_1, t, value.mInt);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_twist_1, t, value.mInt,FOREVER);
		break;
	case eBridgePolygonsTwist2:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_twist_2, t, value.mInt);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_twist_2, t, value.mInt,FOREVER);
		break;
	case eBridgePolygonsPick1:
		{
			//value.mbool now tells us to turn command mode, on or off
			//so we check and see if we are or aren't in the state
			if(GetEditPolyObj()->EpActionGetCommandMode()==epmode_pick_bridge_1) //in command mode 
			{
				if(!value.mbool)
				{
					GetEditPolyObj()->EpActionToggleCommandMode(epmode_pick_bridge_1);
					SetPoly1PickDisabled();
					//also set the text!
				}
			}
			else if(value.mbool) //not in that mode so toggle it on if told to
			{
				GetEditPolyObj()->EpActionToggleCommandMode(epmode_pick_bridge_1);
				SetPoly1PickModeStarted();
			}
		}
		break;
	case eBridgePolygonsPick2:
		{
            //value.mbool now tells us to turn command mode, on or off
			//so we check and see if we are or aren't in the state
			if(GetEditPolyObj()->EpActionGetCommandMode()==epmode_pick_bridge_2) //in command mode 
			{
				if(!value.mbool)
				{
					GetEditPolyObj()->EpActionToggleCommandMode(epmode_pick_bridge_2);
					SetPoly2PickDisabled();
				}
			}
			else if(value.mbool) //not in that mode so toggle it on if told to
			{
				GetEditPolyObj()->EpActionToggleCommandMode(epmode_pick_bridge_2);
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
	theHold.Begin ();	// so we can parcel the operation and the selection-clearing together.
	GetEditPolyObj()->EpPreviewAccept ();
	theHold.Accept (GetString (IDS_APPLY));
	GetEditPolyObj()->EpPreviewBegin (GetPolyID());

}

void BridgePolygonGrip::Okay(TimeValue t)
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewAccept ();
	GetEditPolyObj()->ClearOperationSettings ();

}

void BridgePolygonGrip::Cancel()
{
	//make sure we aren't in any pick modes
	GetEditPolyObj()->ExitPickBridgeModes ();
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewCancel ();
	GetEditPolyObj()->ClearOperationSettings ();
}

bool BridgePolygonGrip::StartSetValue(int which,TimeValue t)
{
	switch(which)
	{
	case eBridgePolygonsTwist1:
	case eBridgePolygonsTwist2:
	case eBridgePolygonsSegments:
	case eBridgePolygonsTaper:
	case eBridgePolygonsBias:
	case eBridgePolygonsSmooth:
		GetEditPolyObj()->EpPreviewSetDragging (true);
		break;
	default:
		return false;
	}
	theHold.Begin();
	return true;
}
bool BridgePolygonGrip::EndSetValue(int which,TimeValue t,bool accept)
{

	switch(which)
	{
	case eBridgePolygonsTwist1:
	case eBridgePolygonsTwist2:
	case eBridgePolygonsSegments:
	case eBridgePolygonsTaper:
	case eBridgePolygonsBias:
	case eBridgePolygonsSmooth:
		{
			GetEditPolyObj()->EpPreviewSetDragging (false);
			GetEditPolyObj()->EpPreviewInvalidate ();
			// Need to tell system something's changed:
			GetEditPolyObj()->NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
			GetEditPolyObj()->RefreshScreen ();
		}
		break;
	default:
		return false;
	}


	if(theHold.Holding())
	{
		if (accept)
			theHold.Accept (GetString(IDS_BRIDGE_POLYGONS));
		else
			theHold.Cancel();
	}
	return true;
}

bool BridgePolygonGrip::GetAutoScale(int which)
{
	return true; //yes
}
//default scale
bool BridgePolygonGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
	{
	case eBridgePolygonsSegments:
		scaleValue.mInt = 1;
		break;
	case eBridgePolygonsTaper:
		scaleValue.mFloat = 1.0f;
		break;
	case eBridgePolygonsBias:
		scaleValue.mFloat = 1.0f;
		break;
	case eBridgePolygonsSmooth:
		scaleValue.mFloat = 1.0f;
		break;
	case eBridgePolygonsTwist1:
		scaleValue.mInt = 1;
		break;
	case eBridgePolygonsTwist2:
		scaleValue.mInt = 1;
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

bool BridgePolygonGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eBridgePolygonsSegments:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_bridge_segments,t)==TRUE)? true : false;
	case eBridgePolygonsTaper:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_bridge_taper,t)==TRUE)? true : false;
	case eBridgePolygonsBias:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_bridge_bias,t)==TRUE)? true : false;
	case eBridgePolygonsSmooth:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_bridge_smooth_thresh,t)==TRUE)? true : false;
	case eBridgePolygonsTwist1:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_bridge_twist_1,t)==TRUE)? true : false;
	case eBridgePolygonsTwist2:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_bridge_twist_2,t)==TRUE) ? true : false;
	default:
		return false;
	
	}

	return false;
}


bool BridgePolygonGrip::ResetValue(TimeValue t, int which)
{
	const int iResetSegVal = 1;
	const int iResetTwistVal = 0;
	const float fResetVal = 0.0f;

	switch(which)
	{
	case eBridgePolygonsSegments:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_segments, t,iResetSegVal);
			theHold.Accept (GetString(IDS_BRIDGE_POLYGONS));
		}
		break;
	case eBridgePolygonsTaper:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_taper, t,fResetVal);
			theHold.Accept (GetString(IDS_BRIDGE_POLYGONS));
		}
		break;
	case eBridgePolygonsBias:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_bias, t,fResetVal);
			theHold.Accept (GetString(IDS_BRIDGE_POLYGONS));
		}
		break;
	case eBridgePolygonsSmooth:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_smooth_thresh, t,fResetVal);
			theHold.Accept (GetString(IDS_BRIDGE_POLYGONS));
		}
		break;
	case eBridgePolygonsTwist1:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_twist_1, t,iResetTwistVal);
			theHold.Accept (GetString(IDS_BRIDGE_POLYGONS));
		}
		break;
	case eBridgePolygonsTwist2:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_twist_2, t,iResetTwistVal);
			theHold.Accept (GetString(IDS_BRIDGE_POLYGONS));

		}
		break;
	case eBridgePolygonsType:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_selected, t,0);
			theHold.Accept (GetString(IDS_BRIDGE_POLYGONS));
		}
		break;
	default:
		return false;
	
	
	}
	return true;

}

bool BridgePolygonGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{

	ParamBlockDesc2* pbd = GetEditPolyObj()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eBridgePolygonsSegments:
		{
			const ParamDef& pd = pbd->paramdefs[ep_bridge_segments];
			resetValue.mInt = pd.def.i;
		}
		break;
	case eBridgePolygonsTaper:
		{
			const ParamDef& pd = pbd->paramdefs[ep_bridge_taper];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eBridgePolygonsBias:
		{
			const ParamDef& pd = pbd->paramdefs[ep_bridge_bias];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eBridgePolygonsSmooth:
		{
			const ParamDef& pd = pbd->paramdefs[ep_bridge_smooth_thresh];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eBridgePolygonsTwist1:
		{
			const ParamDef& pd = pbd->paramdefs[ep_bridge_twist_1];
			resetValue.mInt = pd.def.i;
		}
		break;
	case eBridgePolygonsTwist2:
		{
			const ParamDef& pd = pbd->paramdefs[ep_bridge_twist_2];
			resetValue.mInt = pd.def.i;
		}
		break;
	case eBridgePolygonsType:
		resetValue.mInt = 0;
		break;
	default:
		return false;
	
	
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bridge Border Grips

BridgeBorderGrip::~BridgeBorderGrip()
{

}

int BridgeBorderGrip::GetNumGripItems()
{
	return 9; 
}
IBaseGrip::Type BridgeBorderGrip::GetType(int which)
{
	DbgAssert(which>=0&&which<=8);
	switch(which)
	{
	case eBridgeBordersSegments:
		return IBaseGrip::eInt;
	case eBridgeBordersTaper:
		return IBaseGrip::eFloat;
	case eBridgeBordersBias:
		return IBaseGrip::eFloat;
	case eBridgeBordersSmooth:
		return IBaseGrip::eFloat;
	case eBridgeBordersTwist1:
		return IBaseGrip::eInt;
	case eBridgeBordersTwist2:
		return IBaseGrip::eInt;
	case eBridgeBordersType:
		return IBaseGrip::eCombo;
	case eBridgeBordersPick1:
		return IBaseGrip::eCommand;
	case eBridgeBordersPick2:
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
	GetIGripManager()->ResetGripUI((int)eBridgeBordersPick1);
}

void BridgeBorderGrip::SetEdge2Picked(int num)
{
	TSTR buf;
	buf.printf(_T("%d"), num);
	mEdge2Picked = TSTR(GetString(IDS_EDGE)) + TSTR(" ") + buf;
	mPick2Icon = PICK_FULL_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgeBordersPick2);
}

void BridgeBorderGrip::SetEdge1PickModeStarted()
{
	mEdge1Picked =  TSTR(GetString(IDS_BRIDGE_PICKING_BORDER_1));
	mPick1Icon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgeBordersPick1);
}

void BridgeBorderGrip::SetEdge1PickDisabled()
{
	mEdge1Picked =  TSTR(GetString(IDS_BRIDGE_BORDER_PICK_1));
	mPick1Icon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgeBordersPick1);
}

void BridgeBorderGrip::SetEdge2PickModeStarted()
{
	mEdge2Picked =  TSTR(GetString(IDS_BRIDGE_PICKING_BORDER_2));
	mPick2Icon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgeBordersPick2);
}

void BridgeBorderGrip::SetEdge2PickDisabled()
{
	mEdge2Picked =  TSTR(GetString(IDS_BRIDGE_BORDER_PICK_2));
	mPick2Icon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgeBordersPick2);
}

bool BridgeBorderGrip::GetText(int which,TSTR &string)
{
	switch(which)
	{
	case eBridgeBordersSegments:
		{
			string =TSTR(GetString(IDS_SEGMENTS));
		}
		break;
	case eBridgeBordersTaper:
		{
			string =TSTR(GetString(IDS_TAPER));
		}
		break;
	case eBridgeBordersBias:
		{
			string =TSTR(GetString(IDS_BIAS));
		}
		break;
	case eBridgeBordersSmooth:
		{
			string =TSTR(GetString(IDS_SMOOTH));
		}
		break;
	case eBridgeBordersTwist1:
		{
			string =TSTR(GetString(IDS_TWIST1));
		}
		break;
	case eBridgeBordersTwist2:
		{
			string =TSTR(GetString(IDS_TWIST2));
		}
		break;
	case eBridgeBordersType:
		{
			int type;
			GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_selected, 0, type,FOREVER);
			switch(type)
			{
			case 0:
				string = TSTR(GetString(IDS_BRIDGE_SPECIFIC_BORDERS));
				break;
			case 1:
				string = TSTR(GetString(IDS_BRIDGE_BORDER_SELECTION));
				break;

			}
			break;
		}
	case eBridgeBordersPick1:
		{
			string = mEdge1Picked;
		}
		break;
	case eBridgeBordersPick2:
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
	case eBridgeBordersTaper:
	case eBridgeBordersSmooth:
	case eBridgeBordersTwist2:
	case eBridgeBordersPick1:
	case eBridgeBordersPick2:
		word = (int)(IBaseGrip::eSameRow);
	}
	return word;
}

bool BridgeBorderGrip::GetResolvedIconName(int which,MSTR &string)
{
	switch(which)
	{
	case eBridgeBordersSegments:
		string =  BRIDGE_SEGMENTS_PNG;
		break;
	case eBridgeBordersTaper:
		string = BRIDGE_TAPER_PNG;
		break;
	case eBridgeBordersBias:
		string = BRIDGE_BIAS_PNG;
		break;
	case eBridgeBordersSmooth:
		string = BRIDGE_SMOOTH_PNG;
		break;
	case eBridgeBordersTwist1:
		string =  BRIDGE_TWIST1_PNG;
		break;
	case eBridgeBordersTwist2:
		string = BRIDGE_TWIST2_PNG;
		break;
	case eBridgeBordersType:
		string = BRIDGE_BORDER_TYPE_PNG;
		break;
	case eBridgeBordersPick1:
		string = BRIDGE_BORDERS_PICK1_PNG;
		break;
	case eBridgeBordersPick2:
		string = BRIDGE_BORDERS_PICK2_PNG;
		break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

}



bool BridgeBorderGrip::GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &comboOptions)
{
	if(which== eBridgeBordersType)
	{
		BOOL found;
		IBaseGrip::ComboLabel *label = new IBaseGrip::ComboLabel();
		TSTR string = BRIDGE_SELECTED_PNG;
		found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
		if(found)
			label->mIcon = string;
		label->mLabel = TSTR(GetString(IDS_BRIDGE_SPECIFIC_BORDERS));
		comboOptions.Append(1,&label);
		label = new IBaseGrip::ComboLabel();
		string = BRIDGE_SPECIFIC_PNG;
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
	case eBridgeBordersPick1:
		string = mPick1Icon;
		break;
	case eBridgeBordersPick2:
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
	case eBridgeBordersSegments:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_bridge_segments);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mInt = 1;
			maxRange.mInt = theDef.range_high.i;
		}
		break;

	case eBridgeBordersTaper:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_bridge_taper);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eBridgeBordersBias:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_bridge_bias);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eBridgeBordersSmooth:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_bridge_smooth_thresh);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eBridgeBordersTwist1:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_bridge_twist_1);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mInt = theDef.range_low.i;
			maxRange.mInt = theDef.range_high.i;
		}
		break;

	case eBridgeBordersTwist2:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_bridge_twist_2);
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
	case eBridgeBordersSegments:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_segments, t, value.mInt,FOREVER);
		break;
	case eBridgeBordersTaper:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_taper, t, value.mFloat,FOREVER);
		break;
	case eBridgeBordersBias:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_bias, t, value.mFloat,FOREVER);
		break;
	case eBridgeBordersSmooth:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_smooth_thresh, t, value.mFloat,FOREVER);
		break;
	case eBridgeBordersTwist1:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_twist_1, t, value.mInt,FOREVER);
		break;
	case eBridgeBordersTwist2:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_twist_2, t, value.mInt,FOREVER);
		break;
	case eBridgeBordersType:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_selected, t, value.mInt,FOREVER);
		break;
	case eBridgeBordersPick1:
		{
			//value.mbool, 0 means not in pick mode, 1 does.
			if(GetEditPolyObj()->EpActionGetCommandMode()==epmode_pick_bridge_1) //in command mode 
				value.mbool = true;
			else
				value.mbool = false;
		}
		break;
	case eBridgeBordersPick2:
		{
           if(GetEditPolyObj()->EpActionGetCommandMode()==epmode_pick_bridge_2) //in command mode 
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
	GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_selected,  GetCOREInterface()->GetTime(), val,FOREVER);
	if( 1 == val )
	{
		GetIGripManager()->ActivateGripItem(eBridgeBordersPick1,false);
		GetIGripManager()->ActivateGripItem(eBridgeBordersPick2,false);
		SetEdge1PickDisabled();
		SetEdge2PickDisabled();
	}
	else
	{
		GetIGripManager()->ActivateGripItem(eBridgeBordersPick1,true);
		GetIGripManager()->ActivateGripItem(eBridgeBordersPick2,true);
		int targ = GetEditPolyObj()->getParamBlock ()->GetInt (ep_bridge_target_1);
		if(targ==0)
			SetEdge1PickDisabled();
		else
			SetEdge1Picked(targ-1); //need to -1 out
		targ = GetEditPolyObj()->getParamBlock()->GetInt (ep_bridge_target_2);
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
	case eBridgeBordersSegments:
		if( value.mInt < 1)
			value.mInt = 1;
		GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_segments, t, value.mInt);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_segments, t, value.mInt,FOREVER);
		break;
	case eBridgeBordersTaper:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_taper, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_taper, t, value.mFloat,FOREVER);
		break;
	case eBridgeBordersBias:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_bias, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_bias, t, value.mFloat,FOREVER);
		break;
	case eBridgeBordersSmooth:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_smooth_thresh, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_smooth_thresh, t, value.mFloat,FOREVER);
		break;
	case eBridgeBordersType:
		if(value.mInt>1) //go back to zero
			value.mInt =0;
		if(value.mInt<0)
			value.mInt = 1;
		GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_selected, t, value.mInt);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_selected, t, value.mInt,FOREVER);
		if( 1 == value.mInt )
		{
			GetIGripManager()->ActivateGripItem(eBridgeBordersPick1,false);
			GetIGripManager()->ActivateGripItem(eBridgeBordersPick2,false);
		}
		else
		{
			GetIGripManager()->ActivateGripItem(eBridgeBordersPick1,true);
			GetIGripManager()->ActivateGripItem(eBridgeBordersPick2,true);
		}
		break;
	case eBridgeBordersTwist1:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_twist_1, t, value.mInt);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_twist_1, t, value.mInt,FOREVER);
		break;
	case eBridgeBordersTwist2:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_twist_2, t, value.mInt); 
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_twist_2, t, value.mInt,FOREVER);
		break;
	case eBridgeBordersPick1:
		{
			//value.mbool now tells us to turn command mode, on or off
			//so we check and see if we are or aren't in the state
			if(GetEditPolyObj()->EpActionGetCommandMode()==epmode_pick_bridge_1) //in command mode 
			{
				if(!value.mbool)
				{
					GetEditPolyObj()->EpActionToggleCommandMode(epmode_pick_bridge_1);
					SetEdge1PickDisabled();
					//also set the text!
				}
			}
			else if(value.mbool) //not in that mode so toggle it on if told to
			{
				GetEditPolyObj()->EpActionToggleCommandMode(epmode_pick_bridge_1);
				SetEdge1PickModeStarted();
			}
		}
		break;
	case eBridgeBordersPick2:
		{
			//value.mbool now tells us to turn command mode, on or off
			//so we check and see if we are or aren't in the state
			if(GetEditPolyObj()->EpActionGetCommandMode()==epmode_pick_bridge_2) //in command mode 
			{
				if(!value.mbool)
				{
					GetEditPolyObj()->EpActionToggleCommandMode(epmode_pick_bridge_2);
					SetEdge2PickDisabled();
				}
			}
			else if(value.mbool) //not in that mode so toggle it on if told to
			{
				GetEditPolyObj()->EpActionToggleCommandMode(epmode_pick_bridge_2);
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
	theHold.Begin ();	// so we can parcel the operation and the selection-clearing together.
	GetEditPolyObj()->EpPreviewAccept ();
	theHold.Accept (GetString (IDS_APPLY));
	GetEditPolyObj()->EpPreviewBegin (GetPolyID());

}

void BridgeBorderGrip::Okay(TimeValue t)
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewAccept ();
	GetEditPolyObj()->ClearOperationSettings ();

}

void BridgeBorderGrip::Cancel()
{
	//make sure we aren't in any pick modes
	GetEditPolyObj()->ExitPickBridgeModes ();
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewCancel ();
	GetEditPolyObj()->ClearOperationSettings ();
}

bool BridgeBorderGrip::StartSetValue(int which,TimeValue t)
{
	switch(which)
	{
	case eBridgeBordersTwist1:
	case eBridgeBordersTwist2:
	case eBridgeBordersSegments:
	case eBridgeBordersTaper:
	case eBridgeBordersBias:
	case eBridgeBordersSmooth:
		GetEditPolyObj()->EpPreviewSetDragging (true);
		break;
	default:
		return false;
	}
	theHold.Begin();
	return true;
}
bool BridgeBorderGrip::EndSetValue(int which,TimeValue t,bool accept)
{

	switch(which)
	{
	case eBridgeBordersTwist1:
	case eBridgeBordersTwist2:
	case eBridgeBordersSegments:
	case eBridgeBordersTaper:
	case eBridgeBordersBias:
	case eBridgeBordersSmooth:
		{
			GetEditPolyObj()->EpPreviewSetDragging (false);
			GetEditPolyObj()->EpPreviewInvalidate ();
			// Need to tell system something's changed:
			GetEditPolyObj()->NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
			GetEditPolyObj()->RefreshScreen ();
		}
		break;
	default:
		return false;
	}


	if(theHold.Holding())
	{
		if (accept)
			theHold.Accept (GetString(IDS_BRIDGE));
		else 
			theHold.Cancel();
	}
	return true;
}

bool BridgeBorderGrip::GetAutoScale(int which)
{
	return true; //yes
}
//default scale
bool BridgeBorderGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
	{
	case eBridgeBordersSegments:
		scaleValue.mInt = 1;
		break;
	case eBridgeBordersTaper:
		scaleValue.mFloat = 1.0f;
		break;
	case eBridgeBordersBias:
		scaleValue.mFloat = 1.0f;
		break;
	case eBridgeBordersSmooth:
		scaleValue.mFloat = 1.0f;
		break;
	case eBridgeBordersTwist1:
		scaleValue.mInt = 1;
		break;
	case eBridgeBordersTwist2:
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

bool BridgeBorderGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eBridgeBordersSegments:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_bridge_segments,t)==TRUE)? true : false;
	case eBridgeBordersTaper:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_bridge_taper,t)==TRUE)? true : false;
	case eBridgeBordersBias:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_bridge_bias,t)==TRUE)? true : false;
	case eBridgeBordersSmooth:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_bridge_smooth_thresh,t)==TRUE)? true : false;
	case eBridgeBordersTwist1:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_bridge_twist_1,t)==TRUE)? true : false;
	case eBridgeBordersTwist2:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_bridge_twist_2,t)==TRUE) ? true : false;
	default:
		return false;

	}

	return false;
}


bool BridgeBorderGrip::ResetValue(TimeValue t, int which)
{
	const float fResetVal = 0.0f;
	const int  iResetSegVal = 1;
	const int  iResetTwistVal = 0;

	switch(which)
	{
	case eBridgeBordersSegments:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_segments, t,iResetSegVal);
			theHold.Accept (GetString(IDS_BRIDGE_BORDERS));
		}
		break;
	case eBridgeBordersTaper:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_taper, t,fResetVal);
			theHold.Accept (GetString(IDS_BRIDGE_BORDERS));
		}
		break;
	case eBridgeBordersBias:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_bias, t,fResetVal);
			theHold.Accept (GetString(IDS_BRIDGE_BORDERS));
		}
		break;
	case eBridgeBordersSmooth:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_smooth_thresh, t,fResetVal);
			theHold.Accept (GetString(IDS_BRIDGE_BORDERS));
		}
		break;
	case eBridgeBordersTwist1:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_twist_1, t,iResetTwistVal);
			theHold.Accept (GetString(IDS_BRIDGE_BORDERS));
		}
		break;
	case eBridgeBordersTwist2:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_twist_2, t,iResetTwistVal);
			theHold.Accept (GetString(IDS_BRIDGE_BORDERS));

		}
		break;
	case eBridgeBordersType:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_selected, t,0);
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
	ParamBlockDesc2* pbd = GetEditPolyObj()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eBridgeBordersSegments:
		{
			const ParamDef& pd = pbd->paramdefs[ep_bridge_segments];
			resetValue.mInt = pd.def.i;
		}
		break;
	case eBridgeBordersTaper:
		{
			const ParamDef& pd = pbd->paramdefs[ep_bridge_taper];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eBridgeBordersBias:
		{
			const ParamDef& pd = pbd->paramdefs[ep_bridge_bias];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eBridgeBordersSmooth:
		{
			const ParamDef& pd = pbd->paramdefs[ep_bridge_smooth_thresh];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eBridgeBordersTwist1:
		{
			const ParamDef& pd = pbd->paramdefs[ep_bridge_twist_1];
			resetValue.mInt = pd.def.i;
		}
		break;
	case eBridgeBordersTwist2:
		{
			const ParamDef& pd = pbd->paramdefs[ep_bridge_twist_2];
			resetValue.mInt = pd.def.i;
		}
		break;
	case eBridgeBordersType:
		resetValue.mInt = 0;
		break;
	default:
		return false;


	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bridge Edges Grips

BridgeEdgeGrip::~BridgeEdgeGrip()
{

}

int BridgeEdgeGrip::GetNumGripItems()
{
	return 7; 
}
IBaseGrip::Type BridgeEdgeGrip::GetType(int which)
{
	DbgAssert(which>=0&&which<=6);
	switch(which)
	{
	case eBridgeEdgesSegments:
		return IBaseGrip::eInt;
	case eBridgeEdgesSmooth:
		return IBaseGrip::eFloat;
	case eBridgeEdgesAdjacent:
		return IBaseGrip::eFloat;
	case eBridgeEdgesReverseTriangulation:
		return IBaseGrip::eToggle;
	case eBridgeEdgesType:
		return IBaseGrip::eCombo;
	case eBridgeEdgesPick1:
		return IBaseGrip::eCommand;
	case eBridgeEdgesPick2:
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
	GetIGripManager()->ResetGripUI((int)eBridgeEdgesPick1);
}

void BridgeEdgeGrip::SetEdge2Picked(int num)
{
	TSTR buf;
	buf.printf(_T("%d"), num);
	mEdge2Picked = TSTR(GetString(IDS_EDGE)) + TSTR(" ") + buf;
	mPick2Icon = PICK_FULL_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgeEdgesPick2);
}

void BridgeEdgeGrip::SetEdge1PickModeStarted()
{
	mEdge1Picked =  TSTR(GetString(IDS_BRIDGE_PICKING_EDGE_1));
	mPick1Icon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgeEdgesPick1);
}

void BridgeEdgeGrip::SetEdge1PickDisabled()
{
	mEdge1Picked =  TSTR(GetString(IDS_BRIDGE_PICK_EDGE_1));
	mPick1Icon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgeEdgesPick1);
}

void BridgeEdgeGrip::SetEdge2PickModeStarted()
{
	mEdge2Picked =  TSTR(GetString(IDS_BRIDGE_PICKING_EDGE_2));
	mPick2Icon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgeEdgesPick2);
}

void BridgeEdgeGrip::SetEdge2PickDisabled()
{
	mEdge2Picked =  TSTR(GetString(IDS_BRIDGE_PICK_EDGE_2));
	mPick2Icon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eBridgeEdgesPick2);
}

bool BridgeEdgeGrip::GetText(int which,TSTR &string)
{
	switch(which)
	{
	case eBridgeEdgesSegments:
		{
			string =TSTR(GetString(IDS_SEGMENTS));
		}
		break;
	case eBridgeEdgesSmooth:
		{
			string =TSTR(GetString(IDS_SMOOTH));
		}
		break;
	case eBridgeEdgesAdjacent:
		{
			string =TSTR(GetString(IDS_BRIDGE_ADJACENT));
		}
		break;
	case eBridgeEdgesReverseTriangulation:
		{
			string =TSTR(GetString(IDS_BRIDGE_REVERSE_TRI));
		}
		break;
	case eBridgeEdgesType:
		{
			int type;
			GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_selected, 0, type,FOREVER);
			switch(type)
			{
			case 0:
				string = TSTR(GetString(IDS_BRIDGE_SPECIFIC_EDGES));
				break;
			case 1:
				string = TSTR(GetString(IDS_BRIDGE_EDGE_SELECTION));
				break;

			}
			break;
		}
	case eBridgeEdgesPick1:
		{
			string = mEdge1Picked;
		}
		break;
	case eBridgeEdgesPick2:
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

	case eBridgeEdgesSmooth:
	case eBridgeEdgesReverseTriangulation:
	case eBridgeEdgesPick1:
	case eBridgeEdgesPick2:
		word = (int)(IBaseGrip::eSameRow);
		break;
	}
	return word;
}

bool BridgeEdgeGrip::GetResolvedIconName(int which,MSTR &string)
{
	switch(which)
	{
	case eBridgeEdgesSegments:
		string =  BRIDGE_SEGMENTS_PNG;
		break;
	case eBridgeEdgesSmooth:
		string = BRIDGE_SMOOTH_PNG;
		break;
	case eBridgeEdgesAdjacent:
		string = BRIDGE_ADJACENT_PNG;
		break;
	case eBridgeEdgesReverseTriangulation:
		string =  BRIDGE_REVERSE_TRI_PNG;
		break;
	case eBridgeEdgesType:
		string = BRIDGE_EDGE_TYPE_PNG;
		break;
	case eBridgeEdgesPick1:
		string = BRIDGE_EDGES_PICK1_PNG;
		break;
	case eBridgeEdgesPick2:
		string = BRIDGE_EDGES_PICK2_PNG;
		break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

}



bool BridgeEdgeGrip::GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &comboOptions)
{
	if(which== eBridgeEdgesType)
	{
		BOOL found;
		IBaseGrip::ComboLabel *label = new IBaseGrip::ComboLabel();
		TSTR string = BRIDGE_SELECTED_PNG;
		found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
		if(found)
			label->mIcon = string;
		label->mLabel = TSTR(GetString(IDS_BRIDGE_SPECIFIC_EDGES));
		comboOptions.Append(1,&label);
		label = new IBaseGrip::ComboLabel();
		string = BRIDGE_SPECIFIC_PNG;
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
	case eBridgeEdgesPick1:
		string = mPick1Icon;
		break;
	case eBridgeEdgesPick2:
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
	case eBridgeEdgesSegments:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_bridge_segments);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mInt = 1;
			maxRange.mInt = theDef.range_high.i;
		}
		break;

	case eBridgeEdgesSmooth:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_bridge_smooth_thresh);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eBridgeEdgesAdjacent:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_bridge_adjacent);
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
	case eBridgeEdgesSegments:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_segments, t, value.mInt,FOREVER);
		break;
	case eBridgeEdgesSmooth:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_smooth_thresh, t, value.mFloat,FOREVER);
		break;
	case eBridgeEdgesAdjacent:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_adjacent, t, value.mFloat,FOREVER);
		break;
	case eBridgeEdgesReverseTriangulation:
		{
			int val = 0;
			GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_reverse_triangle, t, val,FOREVER);
			value.mbool = (val == 0 ? false:true) ;
		}
		break;
	case eBridgeEdgesType:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_selected, t, value.mInt,FOREVER);
		break;
	case eBridgeEdgesPick1:
		{
			//value.mbool, 0 means not in pick mode, 1 does.
			if(GetEditPolyObj()->EpActionGetCommandMode()==epmode_pick_bridge_1) //in command mode 
				value.mbool = true;
			else
				value.mbool = false;
		}
		break;
	case eBridgeEdgesPick2:
		{
           if(GetEditPolyObj()->EpActionGetCommandMode()==epmode_pick_bridge_2) //in command mode 
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
	GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_selected,  GetCOREInterface()->GetTime(), val,FOREVER);
	if( 1 == val )
	{
		GetIGripManager()->ActivateGripItem(eBridgeEdgesPick1,false);
		GetIGripManager()->ActivateGripItem(eBridgeEdgesPick2,false);
		SetEdge1PickDisabled();
		SetEdge2PickDisabled();
	}
	else
	{
		GetIGripManager()->ActivateGripItem(eBridgeEdgesPick1,true);
		GetIGripManager()->ActivateGripItem(eBridgeEdgesPick2,true);
		int targ = GetEditPolyObj()->getParamBlock ()->GetInt (ep_bridge_target_1);
		if(targ==0)
			SetEdge1PickDisabled();
		else
			SetEdge1Picked(targ-1); //need to -1 out
		targ = GetEditPolyObj()->getParamBlock()->GetInt (ep_bridge_target_2);
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
	case eBridgeEdgesSegments:
		if( value.mInt < 1)
			value.mInt = 1;
		GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_segments, t, value.mInt);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_segments, t, value.mInt,FOREVER);
		break;
	case eBridgeEdgesSmooth:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_smooth_thresh, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_smooth_thresh, t, value.mFloat,FOREVER);
		break;
	case eBridgeEdgesAdjacent:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_adjacent, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_adjacent, t, value.mFloat,FOREVER);
		break;
	case eBridgeEdgesReverseTriangulation:
		{
			int val =(int)value.mbool ;
			GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_reverse_triangle, t, val);
		    GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_reverse_triangle, t, val,FOREVER);
		}
		break;
	
	case eBridgeEdgesType:
		if(value.mInt>1) //go back to zero
			value.mInt =0;
		if(value.mInt<0)
			value.mInt = 1;
		GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_selected, t, value.mInt);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_bridge_selected, t, value.mInt,FOREVER);
		if( 1 == value.mInt )
		{
			GetIGripManager()->ActivateGripItem(eBridgeEdgesPick1,false);
			GetIGripManager()->ActivateGripItem(eBridgeEdgesPick2,false);
		}
		else
		{
			GetIGripManager()->ActivateGripItem(eBridgeEdgesPick1,true);
			GetIGripManager()->ActivateGripItem(eBridgeEdgesPick2,true);
		}
		break;

	case eBridgeEdgesPick1:
		{
			//value.mbool now tells us to turn command mode, on or off
			//so we check and see if we are or aren't in the state
			if(GetEditPolyObj()->EpActionGetCommandMode()==epmode_pick_bridge_1) //in command mode 
			{
				if(!value.mbool)
				{
					GetEditPolyObj()->EpActionToggleCommandMode(epmode_pick_bridge_1);
					SetEdge1PickDisabled();
					//also set the text!
				}
			}
			else if(value.mbool) //not in that mode so toggle it on if told to
			{
				GetEditPolyObj()->EpActionToggleCommandMode(epmode_pick_bridge_1);
				SetEdge1PickModeStarted();
			}
		}
		break;
	case eBridgeEdgesPick2:
		{
			if(GetEditPolyObj()->EpActionGetCommandMode()==epmode_pick_bridge_2) //in command mode 
			{
				if(!value.mbool)
				{
					GetEditPolyObj()->EpActionToggleCommandMode(epmode_pick_bridge_2);
					SetEdge2PickDisabled();
				}
			}
			else if(value.mbool) //not in that mode so toggle it on if told to
			{
				GetEditPolyObj()->EpActionToggleCommandMode(epmode_pick_bridge_2);
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
	theHold.Begin ();	// so we can parcel the operation and the selection-clearing together.
	GetEditPolyObj()->EpPreviewAccept ();
	theHold.Accept (GetString (IDS_APPLY));
	GetEditPolyObj()->EpPreviewBegin (GetPolyID());

}

void BridgeEdgeGrip::Okay(TimeValue t)
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewAccept ();
	GetEditPolyObj()->ClearOperationSettings ();

}

void BridgeEdgeGrip::Cancel()
{
	//make sure we aren't in any pick modes
	GetEditPolyObj()->ExitPickBridgeModes ();
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewCancel ();
	GetEditPolyObj()->ClearOperationSettings ();
}

bool BridgeEdgeGrip::StartSetValue(int which,TimeValue t)
{
	switch(which)
	{
	case eBridgeEdgesSegments:
	case eBridgeEdgesSmooth:
	case eBridgeEdgesAdjacent:
		GetEditPolyObj()->EpPreviewSetDragging (true);
		break;
	default:
		return false;
	}
	theHold.Begin();
	return true;
}
bool BridgeEdgeGrip::EndSetValue(int which,TimeValue t,bool accept)
{

	switch(which)
	{
	case eBridgeEdgesSegments:
	case eBridgeEdgesSmooth:
	case eBridgeEdgesAdjacent:
		{
			GetEditPolyObj()->EpPreviewSetDragging (false);
			GetEditPolyObj()->EpPreviewInvalidate ();
			// Need to tell system something's changed:
			GetEditPolyObj()->NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
			GetEditPolyObj()->RefreshScreen ();
		}
        break;
	default:
		return false;
	}


	if(theHold.Holding())
	{
		if (accept)
			theHold.Accept (GetString(IDS_BRIDGE_EDGES));
		else 
			theHold.Cancel();
	}
	return true;
}

bool BridgeEdgeGrip::GetAutoScale(int which)
{
	return true; //yes
}
//default scale
bool BridgeEdgeGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
	{
	case eBridgeEdgesSegments:
		scaleValue.mInt = 1;
		break;
	case eBridgeEdgesSmooth:
		scaleValue.mFloat = 1.0f;
		break;
	case eBridgeEdgesAdjacent:
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

bool BridgeEdgeGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eBridgeEdgesSegments:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_bridge_segments,t)==TRUE)? true : false;
	case eBridgeEdgesSmooth:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_bridge_smooth_thresh,t)==TRUE)? true : false;
	case eBridgeEdgesAdjacent:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_bridge_adjacent,t)==TRUE)? true : false;
	case eBridgeEdgesReverseTriangulation:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_bridge_reverse_triangle,t)==TRUE)? true : false;
	default:
		return false;

	}

	return false;
}


bool BridgeEdgeGrip::ResetValue(TimeValue t, int which)
{
	const int iResetSegVal = 1;
	const float fResetVal = 0.0f;

	switch(which)
	{
	case eBridgeEdgesSegments:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_segments, t,iResetSegVal);
			theHold.Accept (GetString(IDS_BRIDGE_EDGES));
		}
		break;

	case eBridgeEdgesSmooth:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_smooth_thresh, t,fResetVal);
			theHold.Accept (GetString(IDS_BRIDGE_EDGES));
		}
		break;

	case eBridgeEdgesAdjacent:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_adjacent, t,fResetVal);
			theHold.Accept (GetString(IDS_BRIDGE_EDGES));
		}
		break;

	case eBridgeEdgesReverseTriangulation:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_reverse_triangle, t, 0);
			theHold.Accept (GetString(IDS_BRIDGE_EDGES));
		}
		break;
	
	case eBridgeEdgesType:
		theHold.Begin ();
		GetEditPolyObj()->getParamBlock()->SetValue (ep_bridge_selected, t,0);
		theHold.Accept (GetString(IDS_BRIDGE_EDGES));
		break;
	default:
		return false;

	}
	return true;

}

bool BridgeEdgeGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{

	ParamBlockDesc2* pbd = GetEditPolyObj()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eBridgeEdgesSegments:
		{
			const ParamDef& pd = pbd->paramdefs[ep_bridge_segments];
			resetValue.mInt = pd.def.i;
		}
		break;
	case eBridgeEdgesSmooth:
		{
			const ParamDef& pd = pbd->paramdefs[ep_bridge_smooth_thresh];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eBridgeEdgesAdjacent:
		{
			const ParamDef& pd = pbd->paramdefs[ep_bridge_adjacent];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eBridgeEdgesReverseTriangulation:
		{
			const ParamDef& pd = pbd->paramdefs[ep_bridge_reverse_triangle];
			resetValue.mbool = (pd.def.i == 0 ? false:true);
		}
		break;
	case eBridgeEdgesType:
		resetValue.mInt = 0;
		break;
	default:
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Relax Grips( Polygons ,Border , Edge, Vertices all have one setting grip )

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
			string =TSTR(GetString(IDS_RELAX_HOLD_BOUNDARY_POINTS));
		}
		break;
	case eRelaxOuterPoints:
		{
			string =TSTR(GetString(IDS_RELAX_HOLD_OUTER_POINTS));
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
		string =  RELAX_BOUNDARY_PNG;
		break;
	case eRelaxOuterPoints:
		string =  RELAX_OUTER_PNG;
		break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

}



bool RelaxGrip::GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &comboOptions)
{
	// has no radio options
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
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_relax_amount);
		if(theDef.flags&P_HAS_RANGE)
		{
			//minRange.mFloat = theDef.range_low.f;
			//maxRange.mFloat = theDef.range_high.f;
			minRange.mFloat = -1.0f;
			maxRange.mFloat = 1.0f;
		}
		break;

	case eRelaxIterations:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_relax_iters);
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
		GetEditPolyObj()->getParamBlock()->GetValue (ep_relax_amount, t, value.mFloat,FOREVER);
		break;
	case eRelaxIterations:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_relax_iters, t, value.mInt,FOREVER);
		break;
	case eRelaxBoundaryPoints:
		{
			int val = 0;
			GetEditPolyObj()->getParamBlock()->GetValue (ep_relax_hold_boundary, t, val ,FOREVER);
			value.mbool = ( val == 0 ? false:true);
		}
		break;
	case eRelaxOuterPoints:
		{
			int val = 0;
			GetEditPolyObj()->getParamBlock()->GetValue (ep_relax_hold_outer, t, val ,FOREVER);
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
			value.mFloat = 1.0f ;
		else if( value.mFloat < -1.0f )
			value.mFloat = -1.0f ;
		GetEditPolyObj()->getParamBlock()->SetValue (ep_relax_amount, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_relax_amount, t, value.mFloat,FOREVER);
		break;
	case eRelaxIterations:
		if( value.mInt < 1)
			value.mInt = 1;
		GetEditPolyObj()->getParamBlock()->SetValue (ep_relax_iters, t, value.mInt);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_relax_iters, t, value.mInt,FOREVER);
		break;
	case eRelaxBoundaryPoints:
		{
			int val = (int)value.mbool ;
			GetEditPolyObj()->getParamBlock()->SetValue (ep_relax_hold_boundary, t, val);
		    GetEditPolyObj()->getParamBlock()->GetValue (ep_relax_hold_boundary, t, val,FOREVER);
		}
		break;
	case eRelaxOuterPoints:
		{
			int val =(int)value.mbool ;
			GetEditPolyObj()->getParamBlock()->SetValue (ep_relax_hold_outer, t, val);
		    GetEditPolyObj()->getParamBlock()->GetValue (ep_relax_hold_outer, t, val,FOREVER);
		}
		break;
	default:
		return false;

	}
	return true;
}

void RelaxGrip::Apply(TimeValue t)
{
	theHold.Begin ();	// so we can parcel the operation and the selection-clearing together.
	GetEditPolyObj()->EpPreviewAccept ();
	theHold.Accept (GetString (IDS_APPLY));
	GetEditPolyObj()->EpPreviewBegin (GetPolyID());

}

void RelaxGrip::Okay(TimeValue t)
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewAccept ();
	GetEditPolyObj()->ClearOperationSettings ();

}

void RelaxGrip::Cancel()
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewCancel ();
	GetEditPolyObj()->ClearOperationSettings ();
}

bool RelaxGrip::StartSetValue(int which,TimeValue t)
{
	switch(which)
	{
	case eRelaxAmount:
	case eRelaxIterations:

		GetEditPolyObj()->EpPreviewSetDragging (true);
		break;
	default:
		return false;
	}
	theHold.Begin();
	return true;
}
bool RelaxGrip::EndSetValue(int which,TimeValue t,bool accept)
{

	switch(which)
	{
	case eRelaxAmount:
	case eRelaxIterations:
		{
			GetEditPolyObj()->EpPreviewSetDragging (false);
			GetEditPolyObj()->EpPreviewInvalidate ();
			// Need to tell system something's changed:
			GetEditPolyObj()->NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
			GetEditPolyObj()->RefreshScreen ();
		}
		break;
	default:
		return false;
	}


	if(theHold.Holding())
	{
		if(accept)
			theHold.Accept (GetString(IDS_RELAX));
		else
			theHold.Cancel();
	}
	return true;
}

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

bool RelaxGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{

	case eRelaxAmount:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_relax_amount,t)==TRUE)? true : false;
	case eRelaxIterations:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_relax_iters,t)==TRUE) ? true : false;
	case eRelaxBoundaryPoints:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_relax_hold_boundary,t)==TRUE) ? true : false;
	case eRelaxOuterPoints:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_relax_hold_outer,t)==TRUE) ? true : false;
	default:
		return false;
	}

	return false;
}


bool RelaxGrip::ResetValue(TimeValue t, int which)
{
	const int iResetIterVal = 1;
	const float fResetVal = 0.0f;

	switch(which)
	{
	case eRelaxAmount:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_relax_amount, t,fResetVal);
			theHold.Accept (GetString(IDS_RELAX_AMOUNT));
		}
		break;
	case eRelaxIterations:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_relax_iters, t,iResetIterVal);
			theHold.Accept (GetString(IDS_RELAX_ITERATIONS));
		}
		break;
	case eRelaxBoundaryPoints:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_relax_hold_boundary, t, 0);
			theHold.Accept (GetString(IDS_RELAX_HOLD_BOUNDARY_POINTS));
		}
		break;
	case eRelaxOuterPoints:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_relax_hold_outer, t, 0);
			theHold.Accept (GetString(IDS_RELAX_HOLD_OUTER_POINTS));
		}
		break;
	default:
		return false;
	}
	return true;

}

bool RelaxGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{
	ParamBlockDesc2* pbd = GetEditPolyObj()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eRelaxAmount:
		{
			const ParamDef& pd = pbd->paramdefs[ep_relax_amount];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eRelaxIterations:
		{
			const ParamDef& pd = pbd->paramdefs[ep_relax_iters];
			resetValue.mInt = pd.def.i;
		}
		break;
	case eRelaxBoundaryPoints:
		{
			const ParamDef& pd = pbd->paramdefs[ep_relax_hold_boundary];
			resetValue.mbool = (pd.def.i == 0 ? false:true);
		}
		break;
	case eRelaxOuterPoints:
		{
			const ParamDef& pd = pbd->paramdefs[ep_relax_hold_outer];
			resetValue.mbool = (pd.def.i == 0 ? false:true);
		}
		break;
	default:
		return false;
	}
	return true;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Hinge Grips

HingeGrip::~HingeGrip()
{

}

int HingeGrip::GetNumGripItems()
{
	return 3; 
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
	if(mEdgePicked == TSTR(GetString(IDS_PICKING_LIFT_EDGE)))
		string = mEdgePicked;
	else
		string =TSTR(GetString(IDS_LIFT_FROM_EDGE));
}

void HingeGrip::SetUpUI()
{

	int targ = GetEditPolyObj()->getParamBlock()->GetInt (ep_lift_edge);
	if((targ>=0)&&(mbEdgePicked))
		//SetEdgePickDisabled();
	    SetEdgePicked(targ);
	else
	{
		//SetEdgePicked(targ); //need to -1 out
	    SetEdgePickDisabled();
	}
	GetIGripManager()->ResetAllUI();

}

void HingeGrip::SetEdgePickModeStarted()
{
	mEdgePicked =  TSTR(GetString(IDS_PICKING_LIFT_EDGE));
	mPickIcon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eHingePick);
}

void HingeGrip::SetEdgePicked(int num)
{
	TSTR buf;
	buf.printf(_T("%d"), num+1);
	mEdgePicked = TSTR(GetString(IDS_EDGE)) + TSTR(" ") + buf;
	mPickIcon = PICK_FULL_PNG;
	GetIGripManager()->ResetGripUI((int)eHingePick);
}

void HingeGrip::SetEdgePickDisabled()
{
	mEdgePicked =  TSTR(GetString(IDS_PICK_LIFT_EDGE));
	mPickIcon = PICK_EMPTY_PNG;
	GetIGripManager()->ResetGripUI((int)eHingePick);
}

bool HingeGrip::GetText(int which,TSTR &string)
{
	switch(which)
	{
	case eHingeAngle:
		{
			string = TSTR(GetString(IDS_ANGLE));
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
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_lift_angle);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eHingeSegments:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_lift_segments);
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
			GetEditPolyObj()->getParamBlock()->GetValue (ep_lift_angle, t, valueRad,FOREVER);
			value.mFloat = RadToDeg(valueRad);
		}
		break;
	case eHingeSegments:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_lift_segments, t, value.mInt,FOREVER);
		break;
	case eHingePick:
		{
			//value.mbool, 0 means not in pick mode, 1 does.
			if(GetEditPolyObj()->EpActionGetCommandMode()==epmode_pick_lift_edge) //in command mode 
			{
				value.mbool = true;
				mbEdgePicked = true;
			}
			else
			{
				value.mbool = false;
				mbEdgePicked = false;
			}
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
			float valueRad = DegToRad(value.mFloat);
			GetEditPolyObj()->getParamBlock()->SetValue (ep_lift_angle, t, valueRad);
			GetEditPolyObj()->getParamBlock()->GetValue (ep_lift_angle, t, valueRad,FOREVER);
			value.mFloat = RadToDeg(valueRad);
		}
		break;
	case eHingeSegments:
		if( value.mInt < 1)
			value.mInt = 1 ;
		GetEditPolyObj()->getParamBlock()->SetValue (ep_lift_segments, t, value.mInt);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_lift_segments, t, value.mInt,FOREVER);
		break;
	case eHingePick:
		//value.mbool now tells us to turn command mode, on or off
		//so we check and see if we are or aren't in the state
		if(GetEditPolyObj()->EpActionGetCommandMode()==epmode_pick_lift_edge) //in command mode 
		{
			if(!value.mbool)
			{
				GetEditPolyObj()->EpActionToggleCommandMode(epmode_pick_lift_edge);
				SetEdgePickDisabled();
			}
		}
		else if(value.mbool) //not in that mode so toggle it on if told to
		{
			GetEditPolyObj()->EpActionToggleCommandMode(epmode_pick_lift_edge);
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
	theHold.Begin ();	// so we can parcel the operation and the selection-clearing together.
	GetEditPolyObj()->EpPreviewAccept ();
	theHold.Accept (GetString (IDS_APPLY));
	GetEditPolyObj()->EpPreviewBegin (GetPolyID());

}

void HingeGrip::Okay(TimeValue t)
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewAccept ();
	GetEditPolyObj()->ClearOperationSettings ();

}

void HingeGrip::Cancel()
{
	//make sure we aren't in any pick modes
	GetEditPolyObj()->ExitPickLiftEdgeMode (); 
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewCancel ();
	GetEditPolyObj()->ClearOperationSettings ();
}

bool HingeGrip::StartSetValue(int which,TimeValue t)
{
	switch(which)
	{
	case eHingeAngle:
	case eHingeSegments:

		GetEditPolyObj()->EpPreviewSetDragging (true);
		break;
	default:
		return false;
	}
	theHold.Begin();
	return true;
}
bool HingeGrip::EndSetValue(int which,TimeValue t,bool accept)
{

	switch(which)
	{
	case eHingeAngle:
	case eHingeSegments:
		{
			GetEditPolyObj()->EpPreviewSetDragging (false);
			GetEditPolyObj()->EpPreviewInvalidate ();
			// Need to tell system something's changed:
			GetEditPolyObj()->NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
			GetEditPolyObj()->RefreshScreen ();
		}
		break;
	default:
		return false;
	}


	if(theHold.Holding())
	{
		if(accept)
			theHold.Accept (GetString(IDS_LIFT_FROM_EDGE));
		else
			theHold.Cancel();
	}
	return true;
}

bool HingeGrip::GetAutoScale(int which)
{
	return true; //yes
}
//default scale
bool HingeGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
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

bool HingeGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{

	case eHingeAngle:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_lift_angle,t)==TRUE)? true : false;
	case eHingeSegments:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_lift_segments,t)==TRUE) ? true : false;
	default:
		return false;
	}

	return false;
}


bool HingeGrip::ResetValue(TimeValue t, int which)
{
	const int iResetSegVal = 1;
	const float fResetVal = 0.0f;
	const int iResetPickVal = -1;

	switch(which)
	{
	case eHingeAngle:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_lift_angle, t,fResetVal);
			theHold.Accept (GetString(IDS_LIFT_EDGE));
		}
		break;
	case eHingeSegments:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_lift_segments, t,iResetSegVal);
			theHold.Accept (GetString(IDS_LIFT_EDGE));

		}
		break;
	case eHingePick:
		{
			theHold.Begin();
			GetEditPolyObj()->getParamBlock()->SetValue(ep_lift_edge,t,iResetPickVal);
			theHold.Accept(GetString(IDS_LIFT_EDGE));
		}
		break;
	default:
		return false;
	}
	return true;

}

bool HingeGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{

	ParamBlockDesc2* pbd = GetEditPolyObj()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eHingeAngle:
		{
			float valueRad = 0.0f;
			const ParamDef& pd = pbd->paramdefs[ep_lift_angle];
			valueRad = pd.def.f;
			resetValue.mFloat = RadToDeg(valueRad);
		}
		break;
	case eHingeSegments:
		{
			const ParamDef& pd = pbd->paramdefs[ep_lift_segments];
			resetValue.mInt = pd.def.i;
		}
		break;
	case eHingePick:
		{
			const ParamDef& pd = pbd->paramdefs[ep_lift_edge];
			resetValue.mInt = pd.def.i;
		}
		break;
	default:
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
			string =TSTR(GetString(IDS_TAPER_AMOUNTS));// + TSTR(": ") + TSTR(str);
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
			string =TSTR(GetString(IDS_EXTRUDE_ALONG_SPLINE_ALIGN));//+ TSTR(": ") + TSTR(str);
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
		string = EXTRUDE_ALONG_SPLINE_SEGMENTS;
		break;
	case eExtrudeAlongSplineAlignToFaceNormal:
		string = EXTRUDE_ALONG_SPLINE_TO_FACE_NORMAL_PNG;
		break;
	case eExtrudeAlongSplineTaperAmount:
		string =  EXTRUDE_ALONG_SPLINE_TAPER_AMOUNT;
		break;
	case eExtrudeAlongSplineTaperCurve:
		string =  EXTRUDE_ALONG_SPLINE_TAPER_CURVE;
		break;
	case eExtrudeAlongSplineTwist:
		string = EXTRUDE_ALONG_SPLINE_TWIST;
		break;
	case eExtrudeAlongSplineRotation:
		string = EXTRUDE_ALONG_SPLINE_ROTATION;
		break;
	case eExtrudeAlongSplinePickSpline:
		string = EXTRUDE_ALONG_SPLINE_PICK;
		break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

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
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_extrude_spline_segments);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mInt = 1;
			maxRange.mInt = theDef.range_high.i;
		}
		break;

	case eExtrudeAlongSplineTaperAmount:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_extrude_spline_taper);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eExtrudeAlongSplineTaperCurve:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_extrude_spline_taper_curve);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eExtrudeAlongSplineTwist:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_extrude_spline_twist);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = theDef.range_low.f;
			maxRange.mFloat = theDef.range_high.f;
		}
		break;

	case eExtrudeAlongSplineRotation:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_extrude_spline_rotation);
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
		GetEditPolyObj()->getParamBlock()->GetValue (ep_extrude_spline_segments, t, value.mInt,FOREVER);
		break;
	case eExtrudeAlongSplineTaperAmount:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_extrude_spline_taper, t, value.mFloat,FOREVER);
		break;
	case eExtrudeAlongSplineTaperCurve:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_extrude_spline_taper_curve, t, value.mFloat,FOREVER);
		break;
	case eExtrudeAlongSplineTwist:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_extrude_spline_twist, t, value.mFloat,FOREVER);
		break;
	case eExtrudeAlongSplineRotation:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_extrude_spline_rotation, t, value.mFloat,FOREVER);
		break;
	case eExtrudeAlongSplineAlignToFaceNormal:
		{
			//need to go BOOL (int) to bool
			int val;
			GetEditPolyObj()->getParamBlock()->GetValue (ep_extrude_spline_align, t, val,FOREVER);
			value.mbool =  ( val==0 ? false :true);
		}
		break;
	case eExtrudeAlongSplinePickSpline:
		{
			if(GetEditPolyObj()->EpActionGetPickMode()==epmode_pick_shape) //in command mode 
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
	GetEditPolyObj()->getParamBlock()->GetValue (ep_extrude_spline_align,  GetCOREInterface()->GetTime(), val,FOREVER);
	if(!val)
	{
		GetIGripManager()->ActivateGripItem(eExtrudeAlongSplineRotation,false);
		SetSplinePickDisabled();
	}
	else
	{
		GetIGripManager()->ActivateGripItem(eExtrudeAlongSplineRotation,true);
		
		INode* splineNode = NULL;
		GetEditPolyObj()->getParamBlock()->GetValue (ep_extrude_spline_node, TimeValue(0), splineNode, FOREVER);
		if (splineNode) 
			SetSplinePicked(splineNode);
		else
		{
			if(GetEditPolyObj()->EpActionGetPickMode()==epmode_pick_shape)  //in command mode so still picking
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
		GetEditPolyObj()->getParamBlock()->SetValue (ep_extrude_spline_segments, t, value.mInt);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_extrude_spline_segments, t, value.mInt,FOREVER);
		break;
	case eExtrudeAlongSplineTaperAmount:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_extrude_spline_taper, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_extrude_spline_taper, t, value.mFloat,FOREVER);
		break;
	case eExtrudeAlongSplineTaperCurve:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_extrude_spline_taper_curve, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_extrude_spline_taper_curve, t, value.mFloat,FOREVER);
		break;
	case eExtrudeAlongSplineTwist:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_extrude_spline_twist, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_extrude_spline_twist, t, value.mFloat,FOREVER);
		break;
	case eExtrudeAlongSplineAlignToFaceNormal:
		{
			int val  = (int) value.mbool;
			GetEditPolyObj()->getParamBlock()->SetValue (ep_extrude_spline_align, t, val);
			GetEditPolyObj()->getParamBlock()->GetValue (ep_extrude_spline_align, t, val,FOREVER);
			if(!val)
				GetIGripManager()->ActivateGripItem(eExtrudeAlongSplineRotation,false);
			else
				GetIGripManager()->ActivateGripItem(eExtrudeAlongSplineRotation,true);
			break;
		}
	case eExtrudeAlongSplineRotation:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_extrude_spline_rotation, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_extrude_spline_rotation, t, value.mFloat,FOREVER);
		break;
	case eExtrudeAlongSplinePickSpline:
		{
			if(GetEditPolyObj()->EpActionGetPickMode()==epmode_pick_shape)  //in command mode so still picking
			{
				if(!value.mbool)
				{
					GetEditPolyObj()->EpActionEnterPickMode(epmode_pick_shape);
					SetSplinePickDisabled();
				}
			}
			else if(value.mbool)
			{
				GetEditPolyObj()->EpActionEnterPickMode(epmode_pick_shape);
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
	theHold.Begin ();	// so we can parcel the operation and the selection-clearing together.
	GetEditPolyObj()->EpPreviewAccept ();
	theHold.Accept (GetString (IDS_APPLY));
	GetEditPolyObj()->EpPreviewBegin (GetPolyID());

}

void ExtrudeAlongSplineGrip::Okay(TimeValue t)
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewAccept ();
	GetEditPolyObj()->ClearOperationSettings ();

}

void ExtrudeAlongSplineGrip::Cancel()
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewCancel ();
	GetEditPolyObj()->ClearOperationSettings ();
}

bool ExtrudeAlongSplineGrip::StartSetValue(int which,TimeValue t)
{
	switch(which)
	{
	case eExtrudeAlongSplineSegments:
	case eExtrudeAlongSplineTaperAmount:
	case eExtrudeAlongSplineTaperCurve:
	case eExtrudeAlongSplineTwist:
	case eExtrudeAlongSplineRotation:
		GetEditPolyObj()->EpPreviewSetDragging (true);
		break;
	default:
		return false;
	}
	theHold.Begin();
	return true;
}
bool ExtrudeAlongSplineGrip::EndSetValue(int which,TimeValue t,bool accept)
{

	switch(which)
	{
	case eExtrudeAlongSplineSegments:
	case eExtrudeAlongSplineTaperAmount:
	case eExtrudeAlongSplineTaperCurve:
	case eExtrudeAlongSplineTwist:
	case eExtrudeAlongSplineRotation:
		{
			GetEditPolyObj()->EpPreviewSetDragging (false);
			GetEditPolyObj()->EpPreviewInvalidate ();
			// Need to tell system something's changed:
			GetEditPolyObj()->NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
			GetEditPolyObj()->RefreshScreen ();
		}
		break;
	default:
		return false;
	}


	if(theHold.Holding())
	{
		if (accept)
			theHold.Accept (GetString(IDS_EXTRUDE));
		else
			theHold.Cancel();
	}
	return true;
}

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

bool ExtrudeAlongSplineGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{

	case eExtrudeAlongSplineSegments:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_extrude_spline_segments,t)==TRUE)? true : false;
	case eExtrudeAlongSplineTaperAmount:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_extrude_spline_taper,t)==TRUE) ? true : false;
	case eExtrudeAlongSplineTaperCurve:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_extrude_spline_taper_curve,t)==TRUE) ? true : false;
	case eExtrudeAlongSplineTwist:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_extrude_spline_twist,t)==TRUE) ? true : false;
	case eExtrudeAlongSplineAlignToFaceNormal:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_extrude_spline_align,t)==TRUE) ? true : false;
	case eExtrudeAlongSplineRotation:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_extrude_spline_rotation,t)==TRUE) ? true : false;
	default:
		return false;
	}

	return false;
}


bool ExtrudeAlongSplineGrip::ResetValue(TimeValue t, int which)
{
	const int iResetSegVal = 1;
	const float fResetVal = 0.0f;

	switch(which)
	{
	case eExtrudeAlongSplineSegments:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_extrude_spline_segments, t,iResetSegVal);
			theHold.Accept (GetString(IDS_EXTRUDE_ALONG_SPLINE));
		}
		break;
	case eExtrudeAlongSplineTaperAmount:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_extrude_spline_taper, t,fResetVal);
			theHold.Accept (GetString(IDS_EXTRUDE_ALONG_SPLINE));
		}
		break;
	case eExtrudeAlongSplineTaperCurve:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_extrude_spline_taper_curve, t,fResetVal);
			theHold.Accept (GetString(IDS_EXTRUDE_ALONG_SPLINE));
		}
		break;
	case eExtrudeAlongSplineTwist:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_extrude_spline_twist, t,fResetVal);
			theHold.Accept (GetString(IDS_EXTRUDE_ALONG_SPLINE));
		}
		break;
	case eExtrudeAlongSplineRotation:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_extrude_spline_rotation, t,fResetVal);
			theHold.Accept (GetString(IDS_EXTRUDE_ALONG_SPLINE));
		}
		break;
	case eExtrudeAlongSplineAlignToFaceNormal:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_extrude_spline_align, t, 0);
			theHold.Accept (GetString(IDS_EXTRUDE_ALONG_SPLINE));
		}
		break;
	default:
		return false;
	}
	return true;

}

bool ExtrudeAlongSplineGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{

	ParamBlockDesc2* pbd = GetEditPolyObj()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eExtrudeAlongSplineSegments:
		{
			const ParamDef& pd = pbd->paramdefs[ep_extrude_spline_segments];
			resetValue.mInt = pd.def.i;
		}
		break;
	case eExtrudeAlongSplineTaperAmount:
		{
			const ParamDef& pd = pbd->paramdefs[ep_extrude_spline_taper];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eExtrudeAlongSplineTaperCurve:
		{
			const ParamDef& pd = pbd->paramdefs[ep_extrude_spline_taper_curve];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eExtrudeAlongSplineTwist:
		{
			const ParamDef& pd = pbd->paramdefs[ep_extrude_spline_twist];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eExtrudeAlongSplineRotation:
		{
			const ParamDef& pd = pbd->paramdefs[ep_extrude_spline_rotation];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eExtrudeAlongSplineAlignToFaceNormal:
		{
			const ParamDef& pd = pbd->paramdefs[ep_extrude_spline_align];
			resetValue.mbool = pd.def.i ==0 ? false : true;
		}
		break;
	default:
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Weld Vertices Grip

WeldVerticesGrip::~WeldVerticesGrip()
{

}

int WeldVerticesGrip::GetNumGripItems()
{
	return 2; 
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
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_weld_threshold);
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
		GetEditPolyObj()->getParamBlock()->GetValue (ep_weld_threshold, t, value.mFloat,FOREVER);
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
		GetEditPolyObj()->getParamBlock()->SetValue (ep_weld_threshold, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_weld_threshold, t, value.mFloat,FOREVER);
		break;
	default:
		return false;

	}
	return true;
}

void WeldVerticesGrip::Apply(TimeValue t)
{
	theHold.Begin ();	// so we can parcel the operation and the selection-clearing together.
	GetEditPolyObj()->EpPreviewAccept ();
	theHold.Accept (GetString (IDS_APPLY));
	GetEditPolyObj()->EpPreviewBegin (GetPolyID());

}

void WeldVerticesGrip::Okay(TimeValue t)
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewAccept ();
	GetEditPolyObj()->ClearOperationSettings ();

}

void WeldVerticesGrip::Cancel()
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewCancel ();
	GetEditPolyObj()->ClearOperationSettings ();
}

bool WeldVerticesGrip::StartSetValue(int which,TimeValue t)
{
	switch(which)
	{
	case eWeldVerticesWeldThreshold:

		GetEditPolyObj()->EpPreviewSetDragging (true);
		break;
	default:
		return false;
	}
	theHold.Begin();
	return true;
}
bool WeldVerticesGrip::EndSetValue(int which,TimeValue t,bool accept)
{

	switch(which)
	{
	case eWeldVerticesWeldThreshold:
		{
			GetEditPolyObj()->EpPreviewSetDragging (false);
			GetEditPolyObj()->EpPreviewInvalidate ();
			// Need to tell system something's changed:
			GetEditPolyObj()->NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
			GetEditPolyObj()->RefreshScreen ();
		}
		break;
	default:
		return false;
	}


	if(theHold.Holding())
	{
		if(accept)
			theHold.Accept (GetString(IDS_WELD_VERTS));
		else
			theHold.Cancel();
	}
	return true;
}

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

bool WeldVerticesGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eWeldVerticesWeldThreshold:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_weld_threshold,t)==TRUE)? true : false;
	default:
		return false;

	}

	return false;
}


bool WeldVerticesGrip::ResetValue(TimeValue t, int which)
{
	const float fResetVal =0.0f;

	switch(which)
	{
	case eWeldVerticesWeldThreshold:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_weld_threshold, t,fResetVal);
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
	ParamBlockDesc2* pbd = GetEditPolyObj()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eWeldVerticesWeldThreshold:
		{
			const ParamDef& pd = pbd->paramdefs[ep_weld_threshold];
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Weld Edge Grips

WeldEdgeGrip::~WeldEdgeGrip()
{

}

int WeldEdgeGrip::GetNumGripItems()
{
	return 2; 
}
IBaseGrip::Type WeldEdgeGrip::GetType(int which)
{
	DbgAssert(which>=0&&which<=1);
	switch(which)
	{
	case eWeldEdgeWeldThreshold:
		return IBaseGrip::eUniverse;
	case eWeldEdgeNum:
		return IBaseGrip::eStatus;
	}
	return IBaseGrip::eInt;
}

void WeldEdgeGrip::GetGripName(TSTR &string)
{
	string =TSTR(GetString(IDS_WELD_EDGES));
}

bool WeldEdgeGrip::GetText(int which,TSTR &string)
{
	switch(which)
	{
	case eWeldEdgeWeldThreshold:
		{
			string =TSTR(GetString(IDS_WELDTHRESHOLD));// + TSTR(": ") + TSTR(str);
		}
		break;
	case eWeldEdgeNum:
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

DWORD WeldEdgeGrip::GetCustomization(int which)
{
	return 0;
}

bool WeldEdgeGrip::GetResolvedIconName(int which,MSTR &string)
{
	switch(which)
	{
	case eWeldEdgeWeldThreshold:
		string = WELD_THRESHOLD_PNG;
		break;
	default:
		return false;
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

}



bool WeldEdgeGrip::GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &comboOptions)
{
	// have no radio options
	return false;
}

bool WeldEdgeGrip::GetCommandIcon(int which, MSTR &string)
{
	return false;
}

bool WeldEdgeGrip::GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)
{

	ParamDef theDef;
	switch(which)
	{
	case eWeldEdgeWeldThreshold:
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_edge_weld_threshold);
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

bool WeldEdgeGrip::GetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eWeldEdgeWeldThreshold:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_edge_weld_threshold, t, value.mFloat,FOREVER);
		break;
	default:
		return false;
	}
	return true;
}

bool WeldEdgeGrip::SetValue(int which,TimeValue t,IBaseGrip::GripValue &value)
{
	switch(which)
	{
	case eWeldEdgeWeldThreshold:
		if( value.mFloat < 0.0f )
			value.mFloat = 0.0f;
		GetEditPolyObj()->getParamBlock()->SetValue (ep_edge_weld_threshold, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_edge_weld_threshold, t, value.mFloat,FOREVER);
		break;
	default:
		return false;

	}
	return true;
}

void WeldEdgeGrip::Apply(TimeValue t)
{
	theHold.Begin ();	// so we can parcel the operation and the selection-clearing together.
	GetEditPolyObj()->EpPreviewAccept ();
	theHold.Accept (GetString (IDS_APPLY));
	GetEditPolyObj()->EpPreviewBegin (GetPolyID());

}

void WeldEdgeGrip::Okay(TimeValue t)
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewAccept ();
	GetEditPolyObj()->ClearOperationSettings ();

}

void WeldEdgeGrip::Cancel()
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewCancel ();
	GetEditPolyObj()->ClearOperationSettings ();
}

bool WeldEdgeGrip::StartSetValue(int which,TimeValue t)
{
	switch(which)
	{
	case eWeldEdgeWeldThreshold:

		GetEditPolyObj()->EpPreviewSetDragging (true);
		break;
	default:
		return false;
	}
	theHold.Begin();
	return true;
}
bool WeldEdgeGrip::EndSetValue(int which,TimeValue t,bool accept)
{

	switch(which)
	{
    case eWeldEdgeWeldThreshold:
		{
			GetEditPolyObj()->EpPreviewSetDragging (false);
			GetEditPolyObj()->EpPreviewInvalidate ();
			// Need to tell system something's changed:
			GetEditPolyObj()->NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
			GetEditPolyObj()->RefreshScreen ();
		}
        break;
	default:
		return false;
	}


	if(theHold.Holding())
	{
		if(accept)
			theHold.Accept (GetString(IDS_WELD_EDGES));
		else 
			theHold.Cancel();
	}
	return true;
}

bool WeldEdgeGrip::GetAutoScale(int which)
{
	return true; //yes
}
//default scale
bool WeldEdgeGrip::GetScale(int which, IBaseGrip::GripValue &scaleValue)
{
	switch(which)
	{
	case eWeldEdgeWeldThreshold:
		scaleValue.mFloat = 1.0f;
		break;
	default:
		return false;
	}
	return true;
}
bool WeldEdgeGrip::GetScaleInViewSpace(int which, float &depth)
{
	depth = -200.0f; 
	switch(which)
	{
	case eWeldEdgeWeldThreshold:
		return true;
	}
	return false;
}

bool WeldEdgeGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eWeldEdgeWeldThreshold:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_edge_weld_threshold,t)==TRUE)? true : false;
	default:
		return false;
	}

	return false;
}


bool WeldEdgeGrip::ResetValue(TimeValue t, int which)
{
	const float fResetVal =0.0f;

	switch(which)
	{
	case eWeldEdgeWeldThreshold:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_edge_weld_threshold, t,fResetVal);
			theHold.Accept (GetString(IDS_WELD_EDGES));
		}
		break;
	default:
		return false;
	}
	return true;

}

bool WeldEdgeGrip::GetResetValue(int which,IBaseGrip::GripValue &resetValue)
{
	ParamBlockDesc2* pbd = GetEditPolyObj()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eWeldEdgeWeldThreshold:
		{
			const ParamDef& pd = pbd->paramdefs[ep_edge_weld_threshold];
			resetValue.mFloat = pd.def.f;
		}
		break;
	default:
		return false;
	}
	return true;
}

void WeldEdgeGrip::SetNumVerts(int before,int after)
{
	mStrBefore.printf(_T("%d"),before);
	mStrAfter.printf(_T("%d"),after);
	GetIGripManager()->ResetGripUI((int)eWeldEdgeNum);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MSmooth Grips

MSmoothGrip::~MSmoothGrip()
{

}

int MSmoothGrip::GetNumGripItems()
{
	return 3; 
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
			string =TSTR(GetString(IDS_MS_SMOOTHNESS));// + TSTR(": ") + TSTR(str);
		}
		break;
	case eMSmoothSmoothingGroups:
		{
			string =TSTR(GetString(IDS_MS_SEP_BY_SMOOTH));//+ TSTR(": ") + TSTR(str);
		}
		break;
	case eMSmoothMaterials:
		{
			string =TSTR(GetString(IDS_MS_SEP_BY_MATID));//+ TSTR(": ") + TSTR(str);
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
	DbgAssert(which>=0&&which<=2);
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
	}

	BOOL found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
	return true;

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
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_ms_smoothness);
		if(theDef.flags&P_HAS_RANGE)
		{
			minRange.mFloat = 0.0f;
			maxRange.mFloat = 1.0f;
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
		GetEditPolyObj()->getParamBlock()->GetValue (ep_ms_smoothness, t, value.mFloat,FOREVER);
		break;
	case eMSmoothSmoothingGroups:
		{
			int val = 0;
			GetEditPolyObj()->getParamBlock()->GetValue (ep_ms_sep_smooth, t, val,FOREVER);
			value.mbool = (val == 0 ? false:true);
		}
		break;
	case eMSmoothMaterials:
		{
			int val = 0;
			GetEditPolyObj()->getParamBlock()->GetValue (ep_ms_sep_mat, t, val,FOREVER);
			value.mbool = ( val == 0 ? false:true);
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
		if( value.mFloat > 1.0f )
			value.mFloat = 1.0f;
		if( value.mFloat < 0.0f )
			value.mFloat = 0.0f;
		GetEditPolyObj()->getParamBlock()->SetValue (ep_ms_smoothness, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_ms_smoothness, t, value.mFloat,FOREVER);
		break;
	case eMSmoothSmoothingGroups:
		{
			int val = (int)value.mbool ;
			GetEditPolyObj()->getParamBlock()->SetValue (ep_ms_sep_smooth, t, val);
		    GetEditPolyObj()->getParamBlock()->GetValue (ep_ms_sep_smooth, t, val,FOREVER);
		}
		break;
	case eMSmoothMaterials:
		{
			int val = (int)value.mbool ;
			GetEditPolyObj()->getParamBlock()->SetValue (ep_ms_sep_mat, t, val);
		    GetEditPolyObj()->getParamBlock()->GetValue (ep_ms_sep_mat, t, val,FOREVER);
		}
		break;
	default:
		return false;

	}
	return true;
}

void MSmoothGrip::Apply(TimeValue t)
{
	theHold.Begin ();	// so we can parcel the operation and the selection-clearing together.
	GetEditPolyObj()->EpPreviewAccept ();
	theHold.Accept (GetString (IDS_APPLY));
	GetEditPolyObj()->EpPreviewBegin (GetPolyID());

}

void MSmoothGrip::Okay(TimeValue t)
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewAccept ();
	GetEditPolyObj()->ClearOperationSettings ();

}

void MSmoothGrip::Cancel()
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewCancel ();
	GetEditPolyObj()->ClearOperationSettings ();
}

bool MSmoothGrip::StartSetValue(int which,TimeValue t)
{
	switch(which)
	{
	case eMSmoothSmoothness:

		GetEditPolyObj()->EpPreviewSetDragging (true);
		break;
	default:
		return false;
	}
	theHold.Begin();
	return true;
}
bool MSmoothGrip::EndSetValue(int which,TimeValue t,bool accept)
{

	switch(which)
	{
	case eMSmoothSmoothness:
		{
			GetEditPolyObj()->EpPreviewSetDragging (false);
			GetEditPolyObj()->EpPreviewInvalidate ();
			// Need to tell system something's changed:
			GetEditPolyObj()->NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
			GetEditPolyObj()->RefreshScreen ();
		}
		break;
	default:
		return false;
	}

	if(theHold.Holding())
	{
		if(accept)
			theHold.Accept (GetString(IDS_MESHSMOOTH));
		else
			theHold.Cancel();
	}
	return true;
}

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

bool MSmoothGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{
	case eMSmoothSmoothness:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_ms_smoothness,t)==TRUE)? true : false;
	case eMSmoothSmoothingGroups:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_ms_sep_smooth,t)==TRUE) ? true : false;
	case eMSmoothMaterials:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_ms_sep_mat,t)==TRUE) ? true : false;
	default:
		return false;
	}

	return false;
}


bool MSmoothGrip::ResetValue(TimeValue t, int which)
{
	const float fResetVal = 0.0f;

	switch(which)
	{
	case eMSmoothSmoothness:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_ms_smoothness, t,fResetVal);
			theHold.Accept (GetString(IDS_MESHSMOOTH));
		}
		break;
	case eMSmoothSmoothingGroups:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_ms_sep_smooth, t, 0);
			theHold.Accept (GetString(IDS_MESHSMOOTH));

		}
		break;
	case eMSmoothMaterials:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_ms_sep_mat, t, 0);
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
	ParamBlockDesc2* pbd = GetEditPolyObj()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eMSmoothSmoothness:
		{
			const ParamDef& pd = pbd->paramdefs[ep_ms_smoothness];
			resetValue.mFloat = pd.def.f;
		}
		break;
	case eMSmoothSmoothingGroups:
		{
			const ParamDef& pd = pbd->paramdefs[ep_ms_sep_smooth];
			resetValue.mbool = (pd.def.i == 0 ? false:true);
		}
		break;
	case eMSmoothMaterials:
		{
			const ParamDef& pd = pbd->paramdefs[ep_ms_sep_mat];
			resetValue.mbool = (pd.def.i == 0 ? false:true);
		}
		break;
	default:
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tessellate Grips

TessellateGrip::~TessellateGrip()
{

}

int TessellateGrip::GetNumGripItems()
{
	return 2; 
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
			//Edge,Face
			int type;
			//GetEditPolyObj()->getParamBlock()->GetValue (ep_Tessellate_type, 0, type,FOREVER);
			GetEditPolyObj()->getParamBlock()->GetValue (ep_tess_type, 0, type,FOREVER);
			switch(type)
			{
			case 0:
				string =  TSTR(GetString(IDS_TESS_EDGE));
				break;
			case 1:
				string =  TSTR(GetString(IDS_TESS_FACE));
				break;
			
			}
			break;
		}
	case eTessellateTension:
		{
			string =TSTR(GetString(IDS_TENSION));// + TSTR(": ") + TSTR(str);
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
	DbgAssert(which>=0&&which<=1);
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
		label->mLabel = TSTR(GetString(IDS_TESS_EDGE));
		comboOptions.Append(1,&label);
		label = new IBaseGrip::ComboLabel();
		string = TESSELLATE_FACE_PNG;
		found = GetColorManager()->ResolveIconFolder(TSTR(STR_ICON_DIR) + string, string);
		if(found)
			label->mIcon = string;
		label->mLabel = TSTR(GetString(IDS_TESS_FACE));
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
		theDef = GetEditPolyObj()->getParamBlock()->GetParamDef(ep_tess_tension);
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
		GetEditPolyObj()->getParamBlock()->GetValue (ep_tess_type, t, value.mInt,FOREVER);
		break;
	case eTessellateTension:
		GetEditPolyObj()->getParamBlock()->GetValue (ep_tess_tension, t, value.mFloat,FOREVER);
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
	GetEditPolyObj()->getParamBlock()->GetValue (ep_tess_type,  GetCOREInterface()->GetTime(), val,FOREVER);
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
		GetEditPolyObj()->getParamBlock()->SetValue (ep_tess_type, t, value.mInt);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_tess_type, t, value.mInt,FOREVER);
		if( 1 == value.mInt )
			GetIGripManager()->ActivateGripItem(eTessellateTension, false);
		else
			GetIGripManager()->ActivateGripItem(eTessellateTension, true);
		break;
	case eTessellateTension:
		GetEditPolyObj()->getParamBlock()->SetValue (ep_tess_tension, t, value.mFloat);
		GetEditPolyObj()->getParamBlock()->GetValue (ep_tess_tension, t, value.mFloat,FOREVER);
		break;
	default:
		return false;

	}
	return true;
}

void TessellateGrip::Apply(TimeValue t)
{
	theHold.Begin ();	// so we can parcel the operation and the selection-clearing together.
	GetEditPolyObj()->EpPreviewAccept ();
	theHold.Accept (GetString (IDS_APPLY));
	GetEditPolyObj()->EpPreviewBegin (GetPolyID());

}

void TessellateGrip::Okay(TimeValue t)
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewAccept ();
	GetEditPolyObj()->ClearOperationSettings ();

}

void TessellateGrip::Cancel()
{
	GetEditPolyObj()->HideGrip(false);
	GetEditPolyObj()->EpPreviewCancel ();
	GetEditPolyObj()->ClearOperationSettings ();
}

bool TessellateGrip::StartSetValue(int which,TimeValue t)
{
	switch(which)
	{
	case eTessellateTension:

		GetEditPolyObj()->EpPreviewSetDragging (true);
		break;
	default:
		return false;
	}
	theHold.Begin();
	return true;
}
bool TessellateGrip::EndSetValue(int which,TimeValue t,bool accept)
{

	switch(which)
	{
	case eTessellateTension:
		{
			GetEditPolyObj()->EpPreviewSetDragging (false);
			GetEditPolyObj()->EpPreviewInvalidate ();
			// Need to tell system something's changed:
			GetEditPolyObj()->NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
			GetEditPolyObj()->RefreshScreen ();
		}
        break;
	default:
		return false;
	}


	if(theHold.Holding())
	{
		if(accept)
			theHold.Accept (GetString(IDS_TESSELLATE));
		else
			theHold.Cancel();
	}
	return true;
}

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

bool TessellateGrip::ShowKeyBrackets(int which,TimeValue t)
{
	switch(which)
	{

	case eTessellateType:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_tess_type,t)==TRUE)? true : false;
	case eTessellateTension:
		return (GetEditPolyObj()->getParamBlock()->KeyFrameAtTime((ParamID)ep_tess_tension,t)==TRUE) ? true : false;
	default:
		return false;
	}

	return false;
}


bool TessellateGrip::ResetValue(TimeValue t, int which)
{
	const float fResetVal = 0.0f;

	switch(which)
	{
	case eTessellateType:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_tess_type, t,0);
			theHold.Accept (GetString(IDS_TESSELLATE));
		}
		break;
	case eTessellateTension:
		{
			theHold.Begin ();
			GetEditPolyObj()->getParamBlock()->SetValue (ep_tess_tension, t,fResetVal);
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
	ParamBlockDesc2* pbd = GetEditPolyObj()->getParamBlock()->GetDesc();

	switch(which)
	{
	case eTessellateType:
		resetValue.mInt = 0;
		break;
	case eTessellateTension:
		{
			const ParamDef& pd = pbd->paramdefs[ep_tess_tension];
			resetValue.mFloat = pd.def.f;
		}
		break;
	default:
		return false;
	}
	return true;
}

