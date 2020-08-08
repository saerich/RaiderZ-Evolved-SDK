/**********************************************************************
 *<
	FILE:			PAOperatorCondition.cpp

	DESCRIPTION:	SimpleSpin Operator implementation
					Operator to effect speed unto particles

	CREATED BY:	Oleg Bayborodin

	HISTORY: created 05-17-03

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/


#include "StdAfx.h"
#include "resource.h"

#include "PartASM_SysUtil.h"
#include "PartASM_GlobalFunctions.h"
#include "PartASM_GlobalVariables.h"

#include "PAOperatorCondition.h"

#include "PAOperatorCondition_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPViewManager.h"
#include "PFMessages.h"
#include "IParticleChannelMult.h"

namespace PartASM {

PAOperatorCondition::PAOperatorCondition()
{ 
	GetClassDesc()->MakeAutoParamBlocks(this); 
}

FPInterfaceDesc* PAOperatorCondition::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &condition_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &condition_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &condition_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PAOperatorCondition::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_CONDITION_CLASS_NAME);
}

Class_ID PAOperatorCondition::ClassID()
{
	return PAOperatorCondition_Class_ID;
}

void PAOperatorCondition::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PAOperatorCondition::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefResult PAOperatorCondition::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
														PartID& partID, RefMessage message)
{
	TimeValue curTime = 0;
	switch(message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				switch(lastUpdateID) {
				case kCondition_channelType:
				case kCondition_conditionType:
				case kCondition_useValue2:
					RefreshUI(kCondition_message_update);
				case kCondition_channelName:
				case kCondition_conditionName:
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					break;
				case kCondition_iValue1:
				case kCondition_tValue1:
				case kCondition_fValue1:
				case kCondition_pValue1:
				case kCondition_wValue1:
				case kCondition_aValue1:
				case kCondition_iValue2:
				case kCondition_tValue2:
				case kCondition_fValue2:
				case kCondition_pValue2:
				case kCondition_wValue2:
				case kCondition_aValue2:
					curTime = GetCOREInterface()->GetTime();
					switch(lastUpdateID) {
					case kCondition_iValue1:
						if (pblock()->GetInt(kCondition_iValue1,curTime) > pblock()->GetInt(kCondition_iValue2,curTime))
							pblock()->SetValue(kCondition_iValue2, curTime, pblock()->GetInt(kCondition_iValue1,curTime));
						break;
					case kCondition_tValue1:
						if (pblock()->GetTimeValue(kCondition_tValue1,curTime) > pblock()->GetTimeValue(kCondition_tValue2,curTime))
							pblock()->SetValue(kCondition_tValue2, curTime, pblock()->GetTimeValue(kCondition_tValue1,curTime));
						break;
					case kCondition_fValue1:
						if (pblock()->GetFloat(kCondition_fValue1,curTime) > pblock()->GetFloat(kCondition_fValue2,curTime))
							pblock()->SetValue(kCondition_fValue2, curTime, pblock()->GetFloat(kCondition_fValue1,curTime));
						break;
					case kCondition_pValue1:
						if (pblock()->GetFloat(kCondition_pValue1,curTime) > pblock()->GetFloat(kCondition_pValue2,curTime))
							pblock()->SetValue(kCondition_pValue2, curTime, pblock()->GetFloat(kCondition_pValue1,curTime));
						break;
					case kCondition_wValue1:
						if (pblock()->GetFloat(kCondition_wValue1,curTime) > pblock()->GetFloat(kCondition_wValue2,curTime))
							pblock()->SetValue(kCondition_wValue2, curTime, pblock()->GetFloat(kCondition_wValue1,curTime));
						break;
					case kCondition_aValue1:
						if (pblock()->GetFloat(kCondition_aValue1,curTime) > pblock()->GetFloat(kCondition_aValue2,curTime))
							pblock()->SetValue(kCondition_aValue2, curTime, pblock()->GetFloat(kCondition_aValue1,curTime));
						break;
					case kCondition_iValue2:
						if (pblock()->GetInt(kCondition_iValue1,curTime) > pblock()->GetInt(kCondition_iValue2,curTime))
							pblock()->SetValue(kCondition_iValue1, curTime, pblock()->GetInt(kCondition_iValue2,curTime));
						break;
					case kCondition_tValue2:
						if (pblock()->GetTimeValue(kCondition_tValue1,curTime) > pblock()->GetTimeValue(kCondition_tValue2,curTime))
							pblock()->SetValue(kCondition_tValue1, curTime, pblock()->GetTimeValue(kCondition_tValue2,curTime));
						break;
					case kCondition_fValue2:
						if (pblock()->GetFloat(kCondition_fValue1,curTime) > pblock()->GetFloat(kCondition_fValue2,curTime))
							pblock()->SetValue(kCondition_fValue1, curTime, pblock()->GetFloat(kCondition_fValue2,curTime));
						break;
					case kCondition_pValue2:
						if (pblock()->GetFloat(kCondition_pValue1,curTime) > pblock()->GetFloat(kCondition_pValue2,curTime))
							pblock()->SetValue(kCondition_pValue1, curTime, pblock()->GetFloat(kCondition_pValue2,curTime));
						break;
					case kCondition_wValue2:
						if (pblock()->GetFloat(kCondition_wValue1,curTime) > pblock()->GetFloat(kCondition_wValue2,curTime))
							pblock()->SetValue(kCondition_wValue1, curTime, pblock()->GetFloat(kCondition_wValue2,curTime));
						break;
					case kCondition_aValue2:
						if (pblock()->GetFloat(kCondition_aValue1,curTime) > pblock()->GetFloat(kCondition_aValue2,curTime))
							pblock()->SetValue(kCondition_aValue1, curTime, pblock()->GetFloat(kCondition_aValue2,curTime));
						break;
					}
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					break;
				}
				UpdatePViewUI(lastUpdateID);
			}
			break;
	}

	return REF_SUCCEED;
}

RefTargetHandle PAOperatorCondition::Clone(RemapDir &remap)
{
	PAOperatorCondition* newOp = new PAOperatorCondition();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PAOperatorCondition::GetObjectName()
{
	return GetString(IDS_OPERATOR_CONDITION_OBJECT_NAME);
}

const ParticleChannelMask& PAOperatorCondition::ChannelsUsed(const Interval& time) const
{
								//  read						&write channels
	static ParticleChannelMask mask(PCU_New|PCU_Time|PCU_Speed|PCU_Amount, PCG_AngAxis|PCU_Spin);
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PAOperatorCondition::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_OPERATOR_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PAOperatorCondition::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_OPERATOR_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PAOperatorCondition::HasDynamicName(TSTR& nameSuffix)
{
	nameSuffix = pblock()->GetStr(kCondition_channelName, 0);
	TSTR buf;
	int channelType = pblock()->GetInt(kCondition_channelType, 0);
	bool isLessType = (pblock()->GetInt(kCondition_conditionType, 0) == kCondition_conditionType_isLess);
	bool useSecond = (pblock()->GetInt(kCondition_useValue2, 0) != 0);
	buf = isLessType ? _T("<") : _T(">");
	nameSuffix += buf;
	Control* ctrl = NULL;
	bool isAnimated = false;
	TCHAR charBuf[64];
	int spinnerPrec = GetSpinnerPrecision();
	switch(channelType) {
	case kCondition_channelType_time:
		ctrl = pblock()->GetController(kCondition_tValue1);
		isAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		if (isAnimated) buf = _T("A");
		else TimeToString(pblock()->GetTimeValue(kCondition_tValue1,0), buf);
		break;
	case kCondition_channelType_integer:
		ctrl = pblock()->GetController(kCondition_iValue1);
		isAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		if (isAnimated) buf = _T("A");
		else {
			sprintf(charBuf,"%d",pblock()->GetInt(kCondition_iValue1,0));
			buf = charBuf;
		}
		break;
	case kCondition_channelType_float:
		ctrl = pblock()->GetController(kCondition_fValue1);
		isAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		if (isAnimated) buf = _T("A");
		else buf = FormatFloatValue(pblock()->GetFloat(kCondition_fValue1,0), spinnerPrec);
		break;
	case kCondition_channelType_percent:
		ctrl = pblock()->GetController(kCondition_pValue1);
		isAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		if (isAnimated) buf = _T("A");
		else buf = FormatFloatValue(100.0f*pblock()->GetFloat(kCondition_pValue1,0), spinnerPrec);
		buf += _T("%");
		break;
	case kCondition_channelType_worldFloat:
		ctrl = pblock()->GetController(kCondition_wValue1);
		isAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		if (isAnimated) buf = _T("A");
		else buf = FormatUniverseValue(pblock()->GetFloat(kCondition_wValue1,0));
		break;
	case kCondition_channelType_angle:
		ctrl = pblock()->GetController(kCondition_aValue1);
		isAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		if (isAnimated) buf = _T("A");
		else buf = FormatFloatValue(RAD_TO_DEG*pblock()->GetFloat(kCondition_aValue1,0), spinnerPrec);
		break;
	}
	nameSuffix += buf;

	if (useSecond) {
		buf = isLessType ? _T(" or >") : _T(" and <");
		nameSuffix += buf;
		switch(channelType) {
		case kCondition_channelType_time:
			ctrl = pblock()->GetController(kCondition_tValue2);
			isAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isAnimated) buf = _T("A");
			else TimeToString(pblock()->GetTimeValue(kCondition_tValue2,0), buf);
			break;
		case kCondition_channelType_integer:
			ctrl = pblock()->GetController(kCondition_iValue2);
			isAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isAnimated) buf = _T("A");
			else {
				sprintf(charBuf,"%d",pblock()->GetInt(kCondition_iValue2,0));
				buf = charBuf;
			}
			break;
		case kCondition_channelType_float:
			ctrl = pblock()->GetController(kCondition_fValue2);
			isAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isAnimated) buf = _T("A");
			else buf = FormatFloatValue(pblock()->GetFloat(kCondition_fValue2,0), spinnerPrec);
			break;
		case kCondition_channelType_percent:
			ctrl = pblock()->GetController(kCondition_pValue2);
			isAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isAnimated) buf = _T("A");
			else buf = FormatFloatValue(100.0f*pblock()->GetFloat(kCondition_pValue2,0), spinnerPrec);
			buf += _T("%");
			break;
		case kCondition_channelType_worldFloat:
			ctrl = pblock()->GetController(kCondition_wValue2);
			isAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isAnimated) buf = _T("A");
			else buf = FormatUniverseValue(pblock()->GetFloat(kCondition_wValue2,0));
			break;
		case kCondition_channelType_angle:
			ctrl = pblock()->GetController(kCondition_aValue2);
			isAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isAnimated) buf = _T("A");
			else buf = FormatFloatValue(RAD_TO_DEG*pblock()->GetFloat(kCondition_aValue2,0), spinnerPrec);
			break;
		}
		nameSuffix += buf;
	}

	buf = _T("->");
	nameSuffix += buf;
	buf = pblock()->GetStr(kCondition_conditionName, 0);
	nameSuffix += buf;
	return true;
}

bool PAOperatorCondition::Proceed(IObject* pCont,
									 PreciseTimeValue timeStart, 
									 PreciseTimeValue& timeEnd,
									 Object* pSystem,
									 INode* pNode,
									 INode* actionNode,
									 IPFIntegrator* integrator)
{
	// acquire all necessary channels, create additional if needed
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if(chNew == NULL) return false;
	IParticleChannelPTVR* chTime = GetParticleChannelTimeRInterface(pCont);
	if(chTime == NULL) return false;
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if(chAmount == NULL) return false;
	int i, count = chAmount->Count();
	if (count <= 0) return true;

	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	IParticleChannelMult* chMult = (IParticleChannelMult*)chCont->EnsureInterface(PARTICLECHANNELMULT_INTERFACE,
																		ParticleChannelMult_Class_ID,
																		true, PARTICLECHANNELMULT_INTERFACE,
																		PARTICLECHANNELMULT_INTERFACE, true,
																		actionNode, NULL);
	if (chMult == NULL) return false; // can't create/modify/read the main data channel

	int channelType = pblock()->GetInt(kCondition_channelType, 0);
	Class_ID channelClassID;
	ParamID getData1From, getData2From;
	switch (channelType) {
	case kCondition_channelType_time:
		channelClassID = ParticleChannelPTV_Class_ID;
		break;
	case kCondition_channelType_integer:
		channelClassID = ParticleChannelInt_Class_ID;
		break;
	case kCondition_channelType_float:
		getData1From = kCondition_fValue1;
		getData2From = kCondition_fValue2;
		channelClassID = ParticleChannelFloat_Class_ID;
		break;
	case kCondition_channelType_worldFloat:
		getData1From = kCondition_wValue1;
		getData2From = kCondition_wValue2;
		channelClassID = ParticleChannelFloat_Class_ID;
		break;
	case kCondition_channelType_percent:
		getData1From = kCondition_pValue1;
		getData2From = kCondition_pValue2;
		channelClassID = ParticleChannelFloat_Class_ID;
		break;
	case kCondition_channelType_angle:
		getData1From = kCondition_aValue1;
		getData2From = kCondition_aValue2;
		channelClassID = ParticleChannelFloat_Class_ID;
		break;
	}
	int subChannelIndex = -1;
	TCHAR* dataChannelName = pblock()->GetStr(kCondition_channelName, 0);
	IObject* dataChannel = chMult->GetSubChannel(dataChannelName, channelClassID, subChannelIndex);
	if (dataChannel == NULL) return false; // no data channel to compare with

	TCHAR* conditionChannelName = pblock()->GetStr(kCondition_conditionName, 0);
	IObject* conditionChannel = chMult->GetSubChannel(conditionChannelName, ParticleChannelBool_Class_ID, subChannelIndex);
	if (conditionChannel == NULL) { // create the channel
		subChannelIndex = chMult->NumSubChannels();
		bool added = chMult->AddSubChannel(subChannelIndex, ParticleChannelBool_Class_ID);
		if (!added) return false;
		chMult->SetName(subChannelIndex, conditionChannelName);
		conditionChannel = chMult->GetSubChannel(subChannelIndex);
		if (conditionChannel == NULL) return false; // unable to create condition filter channel
	}
	int scopeType = pblock()->GetInt(kCondition_conditionScope, 0);
	if (scopeType != chMult->GetScope(subChannelIndex))
		chMult->SetScope(subChannelIndex, scopeType);

	IParticleChannelBoolW* chFilter = (IParticleChannelBoolW*)conditionChannel->GetInterface(PARTICLECHANNELBOOLW_INTERFACE);
	if (chFilter == NULL) return false;

	bool isLessThan = (pblock()->GetInt(kCondition_conditionType, timeStart.TimeValue()) == kCondition_conditionType_isLess);
	bool useSecond = (pblock()->GetInt(kCondition_useValue2, timeStart.TimeValue()) != 0);
	bool newOnly = (pblock()->GetInt(kCondition_assignOnce, timeStart.TimeValue()) != 0);

	IParticleChannelIntR* chDataInt = NULL;
	IParticleChannelPTVR* chDataTime = NULL;
	IParticleChannelFloatR* chDataFloat = NULL;
	Control* ctrl = NULL;
	bool isValue1Animated, isValue2Animated;
	TimeValue tValue1, tValue2;
	PreciseTimeValue curTValue;
	int iValue1, iValue2, curIValue;
	float fValue1, fValue2, curFValue;
	bool resVal;
	TimeValue curLocalTime = timeStart.TimeValue();

	switch (channelType) {
	case kCondition_channelType_time:
		chDataTime = (IParticleChannelPTVR*)dataChannel->GetInterface(PARTICLECHANNELPTVR_INTERFACE);
		if (chDataTime == NULL) return false;
		ctrl = pblock()->GetController(kCondition_tValue1);
		isValue1Animated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		ctrl = pblock()->GetController(kCondition_tValue2);
		isValue2Animated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		tValue1 = pblock()->GetTimeValue(kCondition_tValue1, 0);
		tValue2 = pblock()->GetTimeValue(kCondition_tValue2, 0);
		for(i=0; i<count; i++) {
			if (newOnly)
				if (chNew->IsNew(i) == false) continue;
			curTValue = chDataTime->GetValue(i);
			if (useSecond) {
				if (isValue1Animated || isValue2Animated) curLocalTime = (chTime->GetValue(i)).TimeValue();
				if (isLessThan) {
					if (isValue1Animated) {
						if (isValue2Animated) {
							resVal = (curTValue < PreciseTimeValue(pblock()->GetTimeValue(kCondition_tValue1, curLocalTime))) ||
									(curTValue > PreciseTimeValue(pblock()->GetTimeValue(kCondition_tValue2, curLocalTime)));
						} else {
							resVal = (curTValue < PreciseTimeValue(pblock()->GetTimeValue(kCondition_tValue1, curLocalTime))) ||
									(curTValue > PreciseTimeValue(tValue2));
						}
					} else {
						if (isValue2Animated) {
							resVal = (curTValue < PreciseTimeValue(tValue1)) ||
									(curTValue > PreciseTimeValue(pblock()->GetTimeValue(kCondition_tValue2, curLocalTime)));
						} else {
							resVal = (curTValue < PreciseTimeValue(tValue1)) ||	(curTValue > PreciseTimeValue(tValue2));
						} 
					}
				} else {
					if (isValue1Animated) {
						if (isValue2Animated) {
							resVal = (curTValue > PreciseTimeValue(pblock()->GetTimeValue(kCondition_tValue1, curLocalTime))) &&
									(curTValue < PreciseTimeValue(pblock()->GetTimeValue(kCondition_tValue2, curLocalTime)));
						} else {
							resVal = (curTValue > PreciseTimeValue(pblock()->GetTimeValue(kCondition_tValue1, curLocalTime))) &&
									(curTValue < PreciseTimeValue(tValue2));
						}
					} else {
						if (isValue2Animated) {
							resVal = (curTValue > PreciseTimeValue(tValue1)) &&
									(curTValue < PreciseTimeValue(pblock()->GetTimeValue(kCondition_tValue2, curLocalTime)));
						} else {
							resVal = (curTValue > PreciseTimeValue(tValue1)) &&	(curTValue < PreciseTimeValue(tValue2));
						} 
					}
				}
			} else {
				if (isValue1Animated) curLocalTime = (chTime->GetValue(i)).TimeValue();
				if (isLessThan) {
					if (isValue1Animated) {
						resVal = ((curTValue < PreciseTimeValue(pblock()->GetTimeValue(kCondition_tValue1, curLocalTime))) != 0);
					} else {
						resVal = ((curTValue < PreciseTimeValue(tValue1)) != 0);
					}
				} else {
					if (isValue1Animated) {
						resVal = ((curTValue > PreciseTimeValue(pblock()->GetTimeValue(kCondition_tValue1, curLocalTime))) != 0);
					} else {
						resVal = ((curTValue > PreciseTimeValue(tValue1)) != 0);
					}
				}
			}
			chFilter->SetValue(i, resVal);
		}
		break;
	case kCondition_channelType_integer:
		chDataInt = (IParticleChannelIntR*)dataChannel->GetInterface(PARTICLECHANNELINTR_INTERFACE);
		if (chDataInt == NULL) return false;
		ctrl = pblock()->GetController(kCondition_iValue1);
		isValue1Animated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		ctrl = pblock()->GetController(kCondition_iValue2);
		isValue2Animated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		iValue1 = pblock()->GetInt(kCondition_iValue1, 0);
		iValue2 = pblock()->GetInt(kCondition_iValue2, 0);
		for(i=0; i<count; i++) {
			if (newOnly)
				if (chNew->IsNew(i) == false) continue;
			curIValue = chDataInt->GetValue(i);
			if (useSecond) {
				if (isValue1Animated || isValue2Animated) curLocalTime = (chTime->GetValue(i)).TimeValue();
				if (isLessThan) {
					if (isValue1Animated) {
						if (isValue2Animated) {
							resVal = (curIValue < pblock()->GetInt(kCondition_iValue1, curLocalTime)) ||
									(curIValue > pblock()->GetInt(kCondition_iValue2, curLocalTime));
						} else {
							resVal = (curIValue < pblock()->GetInt(kCondition_iValue1, curLocalTime)) ||
									(curIValue > iValue2);
						}
					} else {
						if (isValue2Animated) {
							resVal = (curIValue < iValue1) ||
									(curIValue > pblock()->GetInt(kCondition_iValue2, curLocalTime));
						} else {
							resVal = (curIValue < iValue1) || (curIValue > iValue2);
						} 
					}
				} else {
					if (isValue1Animated) {
						if (isValue2Animated) {
							resVal = (curIValue > pblock()->GetInt(kCondition_iValue1, curLocalTime)) &&
									(curIValue < pblock()->GetInt(kCondition_iValue2, curLocalTime));
						} else {
							resVal = (curIValue > pblock()->GetInt(kCondition_iValue1, curLocalTime)) &&
									(curIValue < iValue2);
						}
					} else {
						if (isValue2Animated) {
							resVal = (curIValue > iValue1) &&
									(curIValue < pblock()->GetInt(kCondition_iValue2, curLocalTime));
						} else {
							resVal = (curIValue > iValue1) &&	(curIValue < iValue2);
						} 
					}
				}
			} else {
				if (isValue1Animated) curLocalTime = (chTime->GetValue(i)).TimeValue();
				if (isLessThan) {
					if (isValue1Animated) {
						resVal = (curIValue < pblock()->GetInt(kCondition_iValue1, curLocalTime));
					} else {
						resVal = (curIValue < iValue1);
					}
				} else {
					if (isValue1Animated) {
						resVal = (curIValue > pblock()->GetInt(kCondition_iValue1, curLocalTime));
					} else {
						resVal = (curIValue > iValue1);
					}
				}
			}
			chFilter->SetValue(i, resVal);
		}
		break;
	case kCondition_channelType_float:
	case kCondition_channelType_worldFloat:
	case kCondition_channelType_percent:
	case kCondition_channelType_angle:
		chDataFloat = (IParticleChannelFloatR*)dataChannel->GetInterface(PARTICLECHANNELFLOATR_INTERFACE);
		if (chDataFloat == NULL) return false;
		ctrl = pblock()->GetController(getData1From);
		isValue1Animated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		ctrl = pblock()->GetController(getData2From);
		isValue2Animated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		fValue1 = pblock()->GetFloat(getData1From, 0);
		fValue2 = pblock()->GetFloat(getData2From, 0);
		for(i=0; i<count; i++) {
			if (newOnly)
				if (chNew->IsNew(i) == false) continue;
			curFValue = chDataFloat->GetValue(i);
			if (useSecond) {
				if (isValue1Animated || isValue2Animated) curLocalTime = (chTime->GetValue(i)).TimeValue();
				if (isLessThan) {
					if (isValue1Animated) {
						if (isValue2Animated) {
							resVal = (curFValue < pblock()->GetFloat(getData1From, curLocalTime)) ||
									(curFValue > pblock()->GetFloat(getData2From, curLocalTime));
						} else {
							resVal = (curFValue < pblock()->GetFloat(getData1From, curLocalTime)) ||
									(curFValue > fValue2);
						}
					} else {
						if (isValue2Animated) {
							resVal = (curFValue < fValue1) ||
									(curFValue > pblock()->GetFloat(getData2From, curLocalTime));
						} else {
							resVal = (curFValue < fValue1) || (curFValue > fValue2);
						} 
					}
				} else {
					if (isValue1Animated) {
						if (isValue2Animated) {
							resVal = (curFValue > pblock()->GetFloat(getData1From, curLocalTime)) &&
									(curFValue < pblock()->GetFloat(getData2From, curLocalTime));
						} else {
							resVal = (curFValue > pblock()->GetFloat(getData1From, curLocalTime)) &&
									(curFValue < fValue2);
						}
					} else {
						if (isValue2Animated) {
							resVal = (curFValue > fValue1) &&
									(curFValue < pblock()->GetFloat(getData2From, curLocalTime));
						} else {
							resVal = (curFValue > fValue1) &&	(curFValue < fValue2);
						} 
					}
				}
			} else {
				if (isValue1Animated) curLocalTime = (chTime->GetValue(i)).TimeValue();
				if (isLessThan) {
					if (isValue1Animated) {
						resVal = (curFValue < pblock()->GetFloat(getData1From, curLocalTime));
					} else {
						resVal = (curFValue < fValue1);
					}
				} else {
					if (isValue1Animated) {
						resVal = (curFValue > pblock()->GetFloat(getData1From, curLocalTime));
					} else {
						resVal = (curFValue > fValue1);
					}
				}
			}
			chFilter->SetValue(i, resVal);
		}
		break;
	}

	return true;
}

ClassDesc* PAOperatorCondition::GetClassDesc() const
{
	return GetPAOperatorConditionDesc();
}

} // end of namespace PartASM