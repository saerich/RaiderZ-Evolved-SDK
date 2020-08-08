/**********************************************************************
 *<
	FILE:			PAOperatorValue.cpp

	DESCRIPTION:	SimpleSpin Operator implementation
					Operator to effect speed unto particles

	CREATED FOR:	Oleg Bayborodin

	HISTORY: created 05-17-03

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/


#include "StdAfx.h"
#include "resource.h"

#include "PartASM_SysUtil.h"
#include "PartASM_GlobalFunctions.h"
#include "PartASM_GlobalVariables.h"

#include "PAOperatorValue.h"

#include "PAOperatorValue_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPViewManager.h"
#include "PFMessages.h"
#include "IParticleChannelMult.h"

namespace PartASM {

PAOperatorValue::PAOperatorValue()
{ 
	GetClassDesc()->MakeAutoParamBlocks(this); 
	if (pblock() != NULL) {
		for(int i=0; i<kNum3DVectorValues; i++) {
			pblock()->SetValue(kValue_fVector, 0, kOperatorValueFloatValues[i], i);
			pblock()->SetValue(kValue_pVector, 0, kOperatorValuePercentValues[i], i);
			pblock()->SetValue(kValue_wVector, 0, kOperatorValueWorldValues[i], i);
			pblock()->SetValue(kValue_aVector, 0, kOperatorValueAngleValues[i], i);
		}
	}
}

FPInterfaceDesc* PAOperatorValue::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &value_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &value_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &value_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PAOperatorValue::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_VALUE_CLASS_NAME);
}

Class_ID PAOperatorValue::ClassID()
{
	return PAOperatorValue_Class_ID;
}

void PAOperatorValue::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PAOperatorValue::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefResult PAOperatorValue::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
														PartID& partID, RefMessage message)
{
	switch(message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				switch(lastUpdateID) {
				case kValue_type:
				case kValue_useCondition:
					RefreshUI(kValue_message_update);
				case kValue_fValue:
				case kValue_iValue:
				case kValue_tValue:
				case kValue_pValue:
				case kValue_wValue:
				case kValue_aValue:
				case kValue_fVector:
				case kValue_pVector:
				case kValue_wVector:
				case kValue_aVector:
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					break;
				}
				UpdatePViewUI(lastUpdateID);
			}
			break;
	}

	return REF_SUCCEED;
}

RefTargetHandle PAOperatorValue::Clone(RemapDir &remap)
{
	PAOperatorValue* newOp = new PAOperatorValue();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PAOperatorValue::GetObjectName()
{
	return GetString(IDS_OPERATOR_VALUE_OBJECT_NAME);
}

const ParticleChannelMask& PAOperatorValue::ChannelsUsed(const Interval& time) const
{
								//  read						&write channels
	static ParticleChannelMask mask(PCU_New|PCU_Time|PCU_Speed|PCU_Amount, PCG_AngAxis|PCU_Spin);
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PAOperatorValue::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_OPERATOR_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PAOperatorValue::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_OPERATOR_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PAOperatorValue::HasDynamicName(TSTR& nameSuffix)
{
	nameSuffix = pblock()->GetStr(kValue_name, 0);

/*	int type	= pblock()->GetInt(kSimpleSpin_direction, 0);
	int ids;
	switch(type) {
	case kSS_Rand_3D:			ids = IDS_RAND_3D;			break;
	case kSS_World_Space:		ids = IDS_WORLD_SPACE;		break;
	case kSS_Particle_Space:	ids = IDS_PARTICLE_SPACE;	break;
	case kSS_Speed_Space:		ids = IDS_SPEED_SPACE;		break;
	case kSS_Speed_Space_Follow:ids = IDS_SPEED_FOLLOW;		break;
	}
	nameSuffix = GetString(ids);
	Control* ctrl = pblock()->GetController(kSimpleSpin_spinrate);
	bool isRateAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
	ctrl = pblock()->GetController(kSimpleSpin_variation);
	bool isVarAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
	if ((!isRateAnimated) && (!isVarAnimated)) {
		TCHAR buf[32];
		bool addedSpace = false;
		int rate = int(pblock()->GetFloat(kSimpleSpin_spinrate, 0)*RAD_TO_DEG);
		if (rate != 0) {
			sprintf(buf," %d",rate);
			nameSuffix += buf;
			addedSpace = true;
		}
		int var = int(pblock()->GetFloat(kSimpleSpin_variation, 0)*RAD_TO_DEG);
		if (var != 0) {
			if (!addedSpace) nameSuffix += TSTR(" ");
			buf[0] = BYTE(177);
			buf[1] = 0;
			nameSuffix += buf;
			sprintf(buf,"%d",var);
			nameSuffix += buf;
		}
	} */
	return true;
}

bool PAOperatorValue::Proceed(IObject* pCont,
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
	int i, j, count = chAmount->Count();
	if (count <= 0) return true;

	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	bool newOnly = (pblock()->GetInt(kValue_assignOnce, timeStart.TimeValue()) != 0);
	int syncType = pblock()->GetInt(kValue_syncType, timeStart.TimeValue());
	bool useBirthTime = (syncType == kValue_sync_age);
	bool useEventStart = (syncType == kValue_sync_event);

	IParticleChannelPTVR* chBirthTime = NULL;
	if (useBirthTime) {
		chBirthTime = GetParticleChannelBirthTimeRInterface(pCont);
		if(chBirthTime == NULL) return false;
	}

	IParticleChannelPTVR* chEventStartR = NULL;
	IParticleChannelPTVW* chEventStartW = NULL;
	bool initEventStart = false;
	if (useEventStart) {
		chEventStartR = (IParticleChannelPTVR*)chCont->EnsureInterface(PARTICLECHANNELEVENTSTARTR_INTERFACE,
																		ParticleChannelPTV_Class_ID,
																		true, PARTICLECHANNELEVENTSTARTR_INTERFACE,
																		PARTICLECHANNELEVENTSTARTW_INTERFACE, false,
																		actionNode, NULL, &initEventStart);
		if (chEventStartR == NULL) return false; // can't read event start time
		if (initEventStart) {
			chEventStartW = GetParticleChannelEventStartWInterface(pCont);
			if (chEventStartW == NULL) return false; // can't write event start time
			for(i=0; i<count; i++) {
				if (chNew->IsNew(i))
					chEventStartW->SetValue(i, chTime->GetValue(i));
			}
		}
	}
	
	IParticleChannelMult* chMult = (IParticleChannelMult*)chCont->EnsureInterface(PARTICLECHANNELMULT_INTERFACE,
																		ParticleChannelMult_Class_ID,
																		true, PARTICLECHANNELMULT_INTERFACE,
																		PARTICLECHANNELMULT_INTERFACE, true,
																		actionNode, NULL);
	if (chMult == NULL) return false; // can't create/modify/read the main data channel
	TCHAR* dataChannelName = pblock()->GetStr(kValue_name, 0);
	int channelType = pblock()->GetInt(kValue_type, 0);
	ParamID getDataFrom;
	Class_ID channelClassID;
	switch (channelType) {
	case kValue_type_time:
		channelClassID = ParticleChannelPTV_Class_ID;
		getDataFrom = kValue_tValue;
		break;
	case kValue_type_integer:
		channelClassID = ParticleChannelInt_Class_ID;
		getDataFrom = kValue_iValue;
		break;
	case kValue_type_float:
	case kValue_type_worldFloat:
	case kValue_type_percent:
	case kValue_type_angle:
		channelClassID = ParticleChannelFloat_Class_ID;
		getDataFrom = kValue_fValue + channelType - kValue_type_float;
		break;
	case kValue_type_floatVector:
	case kValue_type_worldVector:
	case kValue_type_percentVector:
	case kValue_type_angleVector:
		channelClassID = ParticleChannelPoint3_Class_ID;
		getDataFrom = kValue_fVector + channelType - kValue_type_floatVector;
		break;
	}
	int subChannelIndex = -1;
	IObject* dataChannel = chMult->GetSubChannel(dataChannelName, channelClassID, subChannelIndex);
	if (dataChannel == NULL) { // create the channel
		subChannelIndex = chMult->NumSubChannels();
		bool added = chMult->AddSubChannel(subChannelIndex, channelClassID);
		if (!added) return false;
		chMult->SetName(subChannelIndex, dataChannelName);
		dataChannel = chMult->GetSubChannel(subChannelIndex);
	}
	int scopeType = pblock()->GetInt(kValue_scope, 0);
	if (scopeType != chMult->GetScope(subChannelIndex))
		chMult->SetScope(subChannelIndex, scopeType);

	IParticleChannelBoolR* chFilter = NULL;
	bool useFilter = (pblock()->GetInt(kValue_useCondition, 0) != 0);
	if (useFilter) {
		TCHAR* filterName = pblock()->GetStr(kValue_conditionName, 0);
		int filterChannelIndex;
		IObject* filterData = chMult->GetSubChannel(filterName, ParticleChannelBool_Class_ID, filterChannelIndex);
		if (filterData != NULL)
			chFilter = (IParticleChannelBoolR*)filterData->GetInterface(PARTICLECHANNELBOOLR_INTERFACE);
		if (chFilter == NULL) useFilter = false;
	}

	BOOL isValueAnimated = FALSE;
	switch (channelType) {
	case kValue_type_time:
	case kValue_type_integer:
	case kValue_type_float:
	case kValue_type_worldFloat:
	case kValue_type_percent:
	case kValue_type_angle:
		isValueAnimated = IsControlAnimated(pblock()->GetController(getDataFrom));
		break;
	case kValue_type_floatVector:
	case kValue_type_worldVector:
	case kValue_type_percentVector:
	case kValue_type_angleVector:
		isValueAnimated = IsControlAnimated(pblock()->GetController(getDataFrom, 0)) ||
						IsControlAnimated(pblock()->GetController(getDataFrom, 1)) ||
						IsControlAnimated(pblock()->GetController(getDataFrom, 2));
		break;
	}
	if (isValueAnimated == FALSE) syncType = kValue_sync_time;

	IParticleChannelPTVW* chTimeData = NULL;
	IParticleChannelIntW* chIntData = NULL;
	IParticleChannelFloatW* chFloatData = NULL;
	IParticleChannelPoint3W* chVectorData = NULL;
	TimeValue syncTime = timeEnd.TimeValue();
	PreciseTimeValue timeToSet;
	int intToSet;
	float floatToSet;
	Point3 vectorToSet;
	switch (channelType) {
	case kValue_type_time:
		chTimeData = (IParticleChannelPTVW*)dataChannel->GetInterface(PARTICLECHANNELPTVW_INTERFACE);
		timeToSet = PreciseTimeValue(pblock()->GetTimeValue(kValue_tValue, syncTime));
		break;
	case kValue_type_integer:
		chIntData = (IParticleChannelIntW*)dataChannel->GetInterface(PARTICLECHANNELINTW_INTERFACE);
		intToSet = pblock()->GetInt(kValue_iValue, syncTime);
		break;
	case kValue_type_float:
	case kValue_type_worldFloat:
	case kValue_type_percent:
	case kValue_type_angle:
		chFloatData = (IParticleChannelFloatW*)dataChannel->GetInterface(PARTICLECHANNELFLOATW_INTERFACE);
		floatToSet = pblock()->GetFloat(getDataFrom, syncTime);
		break;
	case kValue_type_floatVector:
	case kValue_type_worldVector:
	case kValue_type_percentVector:
	case kValue_type_angleVector:
		chVectorData = (IParticleChannelPoint3W*)dataChannel->GetInterface(PARTICLECHANNELPOINT3W_INTERFACE);
		for(i=0; i<kNum3DVectorValues; i++)
			vectorToSet[i] = pblock()->GetFloat(getDataFrom, syncTime, i);
		break;
	}

	for(i=0; i<count; i++) {
		if (newOnly)
			if (chNew->IsNew(i) == false) continue;
		if (useFilter)
			if (chFilter->GetValue(i) == false) continue;
		if (syncType == kValue_sync_age)
			syncTime = (chTime->GetValue(i) - chBirthTime->GetValue(i)).TimeValue();
		else if (syncType == kValue_sync_event)
			syncTime = (chTime->GetValue(i) - chEventStartR->GetValue(i)).TimeValue();
		switch (channelType) {
		case kValue_type_time:
			if (syncType != kValue_sync_time)
				timeToSet = PreciseTimeValue(pblock()->GetTimeValue(kValue_tValue, syncTime));
			chTimeData->SetValue(i, timeToSet);
			break;
		case kValue_type_integer:
			if (syncType != kValue_sync_time)
				intToSet = pblock()->GetInt(kValue_iValue, syncTime);
			chIntData->SetValue(i, intToSet);
			break;
		case kValue_type_float:
		case kValue_type_worldFloat:
		case kValue_type_percent:
		case kValue_type_angle:
			if (syncType != kValue_sync_time)
				floatToSet = pblock()->GetFloat(getDataFrom, syncTime);
			chFloatData->SetValue(i, floatToSet);
			break;
		case kValue_type_floatVector:
		case kValue_type_worldVector:
		case kValue_type_percentVector:
		case kValue_type_angleVector:
			if (syncType != kValue_sync_time)
				for(j=0; j<kNum3DVectorValues; j++)
					vectorToSet[j] = pblock()->GetFloat(getDataFrom, syncTime, j);
			chVectorData->SetValue(i, vectorToSet);
			break;
		}
	}

	return true;
}

ClassDesc* PAOperatorValue::GetClassDesc() const
{
	return GetPAOperatorValueDesc();
}

} // end of namespace PartASM