/**********************************************************************
 *<
	FILE:			PAOperatorRandom.cpp

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

#include "PAOperatorRandom.h"

#include "PAOperatorRandom_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPViewManager.h"
#include "PFMessages.h"
#include "IParticleChannelMult.h"

namespace PartASM {

PAOperatorRandom::PAOperatorRandom()
{ 
	GetClassDesc()->MakeAutoParamBlocks(this); 
}

FPInterfaceDesc* PAOperatorRandom::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &random_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &random_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &random_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PAOperatorRandom::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_RANDOM_CLASS_NAME);
}

Class_ID PAOperatorRandom::ClassID()
{
	return PAOperatorRandom_Class_ID;
}

void PAOperatorRandom::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PAOperatorRandom::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefResult PAOperatorRandom::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
														PartID& partID, RefMessage message)
{
	bool check12=false, check13=false, check31=false, check32=false;

	switch(message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				switch(lastUpdateID) {
				case kRandom_type:
				case kRandom_floatType:
					if (lastUpdateID == kRandom_floatType) {
						if (pblock()->GetInt(kRandom_type,0) == kRandom_type_float) {
							if (pblock()->GetInt(kRandom_floatType,0) == kRandom_floatType_triangular) {
								TimeValue t = GetCOREInterface()->GetTime();
								float p1 = pblock()->GetFloat(kRandom_param1, t);
								float p2 = pblock()->GetFloat(kRandom_param2, t);
								float p3 = pblock()->GetFloat(kRandom_param3, t);
								if (p1 > p2) pblock()->SetValue(kRandom_param2, t, p1);
								if (p2 > p3) pblock()->SetValue(kRandom_param2, t, p3);
							}
						}
					}
				case kRandom_vectorType:
				case kRandom_useCondition:
					RefreshUI(kRandom_message_update);
				case kRandom_name:
				case kRandom_param1:
					if (lastUpdateID == kRandom_param1) {
						if (pblock()->GetInt(kRandom_type,0) == kRandom_type_float) {
							int fT = pblock()->GetInt(kRandom_floatType,0);
							check12 = (fT == kRandom_floatType_triangular);
							check13 = (fT == kRandom_floatType_uniform);
						} else if (pblock()->GetInt(kRandom_type,0) == kRandom_type_vector) {
							check13 = (pblock()->GetInt(kRandom_vectorType,0) == kRandom_vectorType_uniform);
						}
						if (check12 || check13) {
							TimeValue t = GetCOREInterface()->GetTime();
							float p1 = pblock()->GetFloat(kRandom_param1, t);
							if (check13) {
								float p3 = pblock()->GetFloat(kRandom_param3, t);
								if (p1 > p3) pblock()->SetValue(kRandom_param3, t, p1);
							}
							if (check12) {
								float p2 = pblock()->GetFloat(kRandom_param2, t);
								if (p1 > p2) pblock()->SetValue(kRandom_param2, t, p1);
							}
						}
					}
				case kRandom_param2:
					if (lastUpdateID == kRandom_param2) {
						if (pblock()->GetInt(kRandom_type,0) == kRandom_type_float) {
							if (pblock()->GetInt(kRandom_floatType,0) == kRandom_floatType_triangular) {
								TimeValue t = GetCOREInterface()->GetTime();
								float p1 = pblock()->GetFloat(kRandom_param1, t);
								float p2 = pblock()->GetFloat(kRandom_param2, t);
								float p3 = pblock()->GetFloat(kRandom_param3, t);
								if (p1 > p2) pblock()->SetValue(kRandom_param1, t, p2);
								if (p2 > p3) pblock()->SetValue(kRandom_param3, t, p2);
							}
						}
					}
				case kRandom_param3:
					if (lastUpdateID == kRandom_param3) {
						if (pblock()->GetInt(kRandom_type,0) == kRandom_type_float) {
							int fT = pblock()->GetInt(kRandom_floatType,0);
							check32 = (fT == kRandom_floatType_triangular);
							check31 = (fT == kRandom_floatType_uniform);
						} else if (pblock()->GetInt(kRandom_type,0) == kRandom_type_vector) {
							check31 = (pblock()->GetInt(kRandom_vectorType,0) == kRandom_vectorType_uniform);
						}
						if (check32 || check31) {
							TimeValue t = GetCOREInterface()->GetTime();
							float p3 = pblock()->GetFloat(kRandom_param3, t);
							if (check31) {
								float p1 = pblock()->GetFloat(kRandom_param1, t);
								if (p1 > p3) pblock()->SetValue(kRandom_param1, t, p3);
							}
							if (check32) {
								float p2 = pblock()->GetFloat(kRandom_param2, t);
								if (p2 > p3) pblock()->SetValue(kRandom_param2, t, p3);
							}
						}
					}
				case kRandom_posParam1:
				case kRandom_posParam2:
				case kRandom_posParam3:
				case kRandom_intParam1:
					if (lastUpdateID == kRandom_intParam1) {
						TimeValue t = GetCOREInterface()->GetTime();
						int p1 = pblock()->GetInt(kRandom_intParam1, t);
						int p2 = pblock()->GetInt(kRandom_intParam3, t);
						if (p1 > p2)
							pblock()->SetValue(kRandom_intParam3, t, p1);
					}
				case kRandom_intParam2:
				case kRandom_intParam3:
					if (lastUpdateID == kRandom_intParam3) {
						TimeValue t = GetCOREInterface()->GetTime();
						int p1 = pblock()->GetInt(kRandom_intParam1, t);
						int p2 = pblock()->GetInt(kRandom_intParam3, t);
						if (p1 > p2)
							pblock()->SetValue(kRandom_intParam1, t, p2);
					}
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					break;
				}
				UpdatePViewUI(lastUpdateID);
			}
			break;

		case kRandom_RefMsg_NewRand:
			theHold.Begin();
			NewRand();
			theHold.Accept(GetString(IDS_NEWRANDOMSEED));
			return REF_STOP;
	}

	return REF_SUCCEED;
}

RefTargetHandle PAOperatorRandom::Clone(RemapDir &remap)
{
	PAOperatorRandom* newOp = new PAOperatorRandom();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PAOperatorRandom::GetObjectName()
{
	return GetString(IDS_OPERATOR_RANDOM_OBJECT_NAME);
}

const ParticleChannelMask& PAOperatorRandom::ChannelsUsed(const Interval& time) const
{
								//  read						&write channels
	static ParticleChannelMask mask(PCU_New|PCU_Time|PCU_Speed|PCU_Amount, PCG_AngAxis|PCU_Spin);
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PAOperatorRandom::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_OPERATOR_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PAOperatorRandom::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_OPERATOR_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PAOperatorRandom::HasDynamicName(TSTR& nameSuffix)
{
	TSTR randomName, functionName;
	TSTR num1, num2, num3;
	TCHAR chNum1[32], chNum2[32], chNum3[32];
	bool isNumAnimated;
	Control* ctrl;
	int spinnerPrec;
	int type = pblock()->GetInt(kRandom_type, 0);
	int floatType = pblock()->GetInt(kRandom_floatType, 0);
	int vectorType = pblock()->GetInt(kRandom_vectorType, 0);
	switch(type) {
	case kRandom_type_integer:
		randomName = GetString(IDS_DYNNAME_UNIFORM);
		ctrl = pblock()->GetController(kRandom_intParam1);
		isNumAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		if (isNumAnimated) num1 = TSTR("A");
		else {
			sprintf(chNum1, "%d", pblock()->GetInt(kRandom_intParam1, 0));
			num1 = chNum1;
		}
		ctrl = pblock()->GetController(kRandom_intParam3);
		isNumAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		if (isNumAnimated) num2 = TSTR("A");
		else {
			sprintf(chNum2, "%d", pblock()->GetInt(kRandom_intParam3, 0));
			num2 = chNum2;
		}
		functionName = randomName + TSTR("[") + num1 + TSTR(" ,") + num2 + TSTR("]");
		break;

	case kRandom_type_float:
		spinnerPrec = GetSpinnerPrecision();
		switch(floatType) {
		case kRandom_floatType_uniform:
			randomName = GetString(IDS_DYNNAME_UNIFORM);
			ctrl = pblock()->GetController(kRandom_param1);
			isNumAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isNumAnimated) num1 = TSTR("A");
			else {
				sprintf(chNum1, "%s", FormatFloatValue(pblock()->GetFloat(kRandom_param1, 0), spinnerPrec));
				num1 = chNum1;
			}
			ctrl = pblock()->GetController(kRandom_param3);
			isNumAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isNumAnimated) num2 = TSTR("A");
			else {
				sprintf(chNum2, "%s", FormatFloatValue(pblock()->GetFloat(kRandom_param3, 0), spinnerPrec));
				num2 = chNum2;
			}
			functionName = randomName + TSTR("[") + num1 + TSTR(" ,") + num2 + TSTR("]");
			break;

		case kRandom_floatType_triangular:
			randomName = GetString(IDS_DYNNAME_TRIANGULAR);
			ctrl = pblock()->GetController(kRandom_param1);
			isNumAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isNumAnimated) num1 = TSTR("A");
			else {
				sprintf(chNum1, "%s", FormatFloatValue(pblock()->GetFloat(kRandom_param1, 0), spinnerPrec));
				num1 = chNum1;
			}
			ctrl = pblock()->GetController(kRandom_param2);
			isNumAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isNumAnimated) num2 = TSTR("A");
			else {
				sprintf(chNum2, "%s", FormatFloatValue(pblock()->GetFloat(kRandom_param2, 0), spinnerPrec));
				num2 = chNum2;
			}
			ctrl = pblock()->GetController(kRandom_param3);
			isNumAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isNumAnimated) num3 = TSTR("A");
			else {
				sprintf(chNum3, "%s", FormatFloatValue(pblock()->GetFloat(kRandom_param3, 0), spinnerPrec));
				num3 = chNum3;
			}
			functionName = randomName + TSTR("[") + num1 + TSTR(" ,") + num2 + TSTR(" ,") + num3 + TSTR("]");
			break;

		case kRandom_floatType_normal:
			randomName = GetString(IDS_DYNNAME_NORMAL);
			ctrl = pblock()->GetController(kRandom_param1);
			isNumAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isNumAnimated) num1 = TSTR("A");
			else {
				sprintf(chNum1, "%s", FormatFloatValue(pblock()->GetFloat(kRandom_param1, 0), spinnerPrec));
				num1 = chNum1;
			}
			ctrl = pblock()->GetController(kRandom_posParam2);
			isNumAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isNumAnimated) num2 = TSTR("A");
			else {
				sprintf(chNum2, "%s", FormatFloatValue(pblock()->GetFloat(kRandom_posParam2, 0), spinnerPrec));
				num2 = chNum2;
			}
			functionName = randomName + TSTR("[") + num1 + TSTR(" ,") + num2 + TSTR("]");
			break;

		case kRandom_floatType_exponential:
			randomName = GetString(IDS_DYNNAME_EXPONENTIAL);
			ctrl = pblock()->GetController(kRandom_posParam1);
			isNumAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isNumAnimated) num1 = TSTR("A");
			else {
				sprintf(chNum1, "%s", FormatFloatValue(pblock()->GetFloat(kRandom_posParam1, 0), spinnerPrec));
				num1 = chNum1;
			}
			functionName = randomName + TSTR("[") + num1 + TSTR("]");
			break;

		case kRandom_floatType_weibull:
			randomName = GetString(IDS_DYNNAME_WEIBULL);
			ctrl = pblock()->GetController(kRandom_posParam1);
			isNumAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isNumAnimated) num1 = TSTR("A");
			else {
				sprintf(chNum1, "%s", FormatFloatValue(pblock()->GetFloat(kRandom_posParam1, 0), spinnerPrec));
				num1 = chNum1;
			}
			ctrl = pblock()->GetController(kRandom_posParam2);
			isNumAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isNumAnimated) num2 = TSTR("A");
			else {
				sprintf(chNum2, "%s", FormatFloatValue(pblock()->GetFloat(kRandom_posParam2, 0), spinnerPrec));
				num2 = chNum2;
			}
			ctrl = pblock()->GetController(kRandom_param3);
			isNumAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isNumAnimated) num3 = TSTR("A");
			else {
				sprintf(chNum3, "%s", FormatFloatValue(pblock()->GetFloat(kRandom_param3, 0), spinnerPrec));
				num3 = chNum3;
			}
			functionName = randomName + TSTR("[") + num1 + TSTR(" ,") + num2 + TSTR(" ,") + num3 + TSTR("]");
			break;
		}
		break;
	case kRandom_type_vector:
		spinnerPrec = GetSpinnerPrecision();
		switch(vectorType) {
		case kRandom_vectorType_uniform:
			randomName = GetString(IDS_DYNNAME_UNIFORM3D);
			ctrl = pblock()->GetController(kRandom_param1);
			isNumAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isNumAnimated) num1 = TSTR("A");
			else {
				sprintf(chNum1, "%s", FormatFloatValue(pblock()->GetFloat(kRandom_param1, 0), spinnerPrec));
				num1 = chNum1;
			}
			ctrl = pblock()->GetController(kRandom_param3);
			isNumAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isNumAnimated) num2 = TSTR("A");
			else {
				sprintf(chNum2, "%s", FormatFloatValue(pblock()->GetFloat(kRandom_param3, 0), spinnerPrec));
				num2 = chNum2;
			}
			functionName = randomName + TSTR("[") + num1 + TSTR(" ,") + num2 + TSTR("]");
			break;

		case kRandom_vectorType_sphereSurface:
		case kRandom_vectorType_sphereVolume:
			if (vectorType == kRandom_vectorType_sphereSurface)
				randomName = GetString(IDS_DYNNAME_SPHERESURFACE);
			else
				randomName = GetString(IDS_DYNNAME_SPHEREVOLUME);
			ctrl = pblock()->GetController(kRandom_posParam2);
			isNumAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isNumAnimated) num2 = TSTR("A");
			else {
				sprintf(chNum2, "%s", FormatFloatValue(pblock()->GetFloat(kRandom_posParam2, 0), spinnerPrec));
				num2 = chNum2;
			}
			functionName = randomName + TSTR("[") + num2 + TSTR("]");
			break;

		case kRandom_vectorType_normal:
			randomName = GetString(IDS_DYNNAME_NORMAL3D);
			ctrl = pblock()->GetController(kRandom_posParam2);
			isNumAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isNumAnimated) num2 = TSTR("A");
			else {
				sprintf(chNum2, "%s", FormatFloatValue(pblock()->GetFloat(kRandom_posParam2, 0), spinnerPrec));
				num2 = chNum2;
			}
			functionName = randomName + TSTR("[0 ,") + num2 + TSTR("]");
			break;
		}
		break;
	}

	TSTR name = pblock()->GetStr(kRandom_name, 0);
	nameSuffix = functionName + TSTR("->") + name;
	return true;
}

bool PAOperatorRandom::Proceed(IObject* pCont,
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

	IParticleChannelBoolR* chFilter = NULL;
	bool useCondition = (pblock()->GetInt(kRandom_useCondition, 0) != 0);
	if (useCondition) {
		TCHAR* filterName = pblock()->GetStr(kRandom_conditionName, 0);
		int filterChannelIndex;
		IObject* filterData = chMult->GetSubChannel(filterName, ParticleChannelBool_Class_ID, filterChannelIndex);
		if (filterData != NULL)
			chFilter = (IParticleChannelBoolR*)filterData->GetInterface(PARTICLECHANNELBOOLR_INTERFACE);
		if (chFilter == NULL) useCondition = false;
	}

	bool newOnly = (pblock()->GetInt(kRandom_assignOnce, 0) != 0);
	Tab<bool> processValue;
	processValue.SetCount(count);
	if (newOnly) {
		if (useCondition) {
			for(i=0; i<count; i++)
				processValue[i] = (chNew->IsNew(i) && chFilter->GetValue(i)) ? true : false;
		} else {
			for(i=0; i<count; i++)
				processValue[i] = chNew->IsNew(i) ? true : false;
		}
	} else {
		if (useCondition) {
			for(i=0; i<count; i++)
				processValue[i] = chFilter->GetValue(i) ? true : false;
		} else {
			for(i=0; i<count; i++)
				processValue[i] = true;
		}
	}

	bool hasValuesToProcess = false;
	for(i=0; i<count; i++)
		if (processValue[i]) {
			hasValuesToProcess = true;
			break;
		}
	if (!hasValuesToProcess) return true; // no particles for processing

	TCHAR* channelName = pblock()->GetStr(kRandom_name, 0);
	int channelType = pblock()->GetInt(kRandom_type, 0);
	Class_ID channelClassID;
	switch (channelType) {
	case kRandom_type_integer:
		channelClassID = ParticleChannelInt_Class_ID;
		break;
	case kRandom_type_float:
		channelClassID = ParticleChannelFloat_Class_ID;
		break;
	case kRandom_type_vector:
		channelClassID = ParticleChannelPoint3_Class_ID;
		break;
	}
	int subChannelIndex = -1;
	IObject* dataChannel = chMult->GetSubChannel(channelName, channelClassID, subChannelIndex);
	if (dataChannel == NULL) { // create the channel
		subChannelIndex = chMult->NumSubChannels();
		bool added = chMult->AddSubChannel(subChannelIndex, channelClassID);
		if (!added) return false;
		chMult->SetName(subChannelIndex, channelName);
		dataChannel = chMult->GetSubChannel(subChannelIndex);
	}
	int scopeType = pblock()->GetInt(kRandom_scope, 0);
	if (scopeType != chMult->GetScope(subChannelIndex))
		chMult->SetScope(subChannelIndex, scopeType);

	IParticleChannelIntW* chInt = NULL;
	IParticleChannelFloatW* chFloat = NULL;
	IParticleChannelPoint3W* chVector = NULL;

	switch (channelType) {
	case kRandom_type_integer:
		chInt = (IParticleChannelIntW*)dataChannel->GetInterface(PARTICLECHANNELINTW_INTERFACE);
		if (chInt == NULL) return false;
		break;
	case kRandom_type_float:
		chFloat = (IParticleChannelFloatW*)dataChannel->GetInterface(PARTICLECHANNELFLOATW_INTERFACE);
		if (chFloat == NULL) return false;
		break;
	case kRandom_type_vector:
		chVector = (IParticleChannelPoint3W*)dataChannel->GetInterface(PARTICLECHANNELPOINT3W_INTERFACE);
		if (chVector == NULL) return false;
		break;
	}
	
	float fp1,fp2,fp3,pp1,pp2,pp3;
	int ip1,ip2,ip3;
	bool fp1Anim=false,fp2Anim=false,fp3Anim=false;
	bool pp1Anim=false,pp2Anim=false,pp3Anim=false;
	bool ip1Anim=false,ip2Anim=false,ip3Anim=false;
	Control* ctrl;
	TimeValue time = timeEnd.TimeValue();
	fp1 = pblock()->GetFloat(kRandom_param1, time);
	ctrl = pblock()->GetController(kRandom_param1);
	fp1Anim = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
	fp2 = pblock()->GetFloat(kRandom_param2, time);
	ctrl = pblock()->GetController(kRandom_param2);
	fp2Anim = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
	fp3 = pblock()->GetFloat(kRandom_param3, time);
	ctrl = pblock()->GetController(kRandom_param3);
	fp3Anim = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		
	pp1 = GetPFFloat(pblock(), kRandom_posParam1, time);
	ctrl = pblock()->GetController(kRandom_posParam1);
	pp1Anim = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
	pp2 = GetPFFloat(pblock(), kRandom_posParam2, time);
	ctrl = pblock()->GetController(kRandom_posParam2);
	pp2Anim = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
	pp3 = GetPFFloat(pblock(), kRandom_posParam3, time);
	ctrl = pblock()->GetController(kRandom_posParam3);
	pp3Anim = (ctrl) ? (ctrl->IsAnimated()!=0) : false;

	ip1 = pblock()->GetInt(kRandom_intParam1, time);
	ctrl = pblock()->GetController(kRandom_intParam1);
	ip1Anim = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
	ip2 = pblock()->GetInt(kRandom_intParam2, time);
	ctrl = pblock()->GetController(kRandom_intParam2);
	ip2Anim = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
	ip3 = pblock()->GetInt(kRandom_intParam3, time);
	ctrl = pblock()->GetController(kRandom_intParam3);
	ip3Anim = (ctrl) ? (ctrl->IsAnimated()!=0) : false;

	RandGenerator* rand = randLinker().GetRandGenerator(pCont);
	if (rand == NULL) return false;

	int type = pblock()->GetInt(kRandom_type, time);
	int floatType = pblock()->GetInt(kRandom_floatType, time);
	int vectorType = pblock()->GetInt(kRandom_vectorType, time);
	switch(type) {
	case kRandom_type_integer:
		for(i=0; i<count; i++)
			if (processValue[i]) {
				if (ip1Anim) ip1 = pblock()->GetInt(kRandom_intParam1, (chTime->GetValue(i)).TimeValue());
				if (ip3Anim) ip3 = pblock()->GetInt(kRandom_intParam3, (chTime->GetValue(i)).TimeValue());
				if (ip3 < ip1) ip3 = ip1;
				if (ip1 == ip3) chInt->SetValue(i, ip1);
				else chInt->SetValue(i, rand->Rand0X(ip3-ip1) + ip1);
			}
		break;

	case kRandom_type_float:
		switch(floatType) {
		case kRandom_floatType_uniform:
			for(i=0; i<count; i++)
				if (processValue[i]) {
					if (fp1Anim) fp1 = pblock()->GetFloat(kRandom_param1, (chTime->GetValue(i)).TimeValue());
					if (fp3Anim) fp3 = pblock()->GetFloat(kRandom_param3, (chTime->GetValue(i)).TimeValue());
					if (fp3 < fp1) fp3 = fp1;
					if (fp1 == fp3) chFloat->SetValue(i, fp1);
					else chFloat->SetValue(i, rand->Rand01()*(fp3-fp1) + fp1);
				}
			break; 

		case kRandom_floatType_triangular:
			for(i=0; i<count; i++)
				if (processValue[i]) {
					if (fp1Anim) fp1 = pblock()->GetFloat(kRandom_param1, (chTime->GetValue(i)).TimeValue());
					if (fp2Anim) fp2 = pblock()->GetFloat(kRandom_param2, (chTime->GetValue(i)).TimeValue());
					if (fp3Anim) fp3 = pblock()->GetFloat(kRandom_param3, (chTime->GetValue(i)).TimeValue());
					if (fp3 < fp1) fp3 = fp1;
					if (fp2 < fp1) fp2 = fp1;
					if (fp2 > fp3) fp2 = fp3;
					if (fp1 == fp3) chFloat->SetValue(i, fp1);
					else {
						float r1 = rand->Rand01();
						float r2 = r1*(fp3-fp1) + fp1;
						float r3 = rand->Rand01();
						float r4 = rand->Rand01();
						float r5 = max(r3, r4);
						if (r2 < fp2) {
							float r5 = max(r3, r4);
							r2 = fp1 + r5*(fp2-fp1);							
						} else if (r2 > fp2) {
							float r5 = 1.0f - max(r3, r4);
							r2 = fp2 + r5*(fp3-fp2);
						}
						chFloat->SetValue(i, r2);
					}
				}
			break; 

		case kRandom_floatType_normal:
			for(i=0; i<count; i++)
				if (processValue[i]) {
					if (fp1Anim) fp1 = pblock()->GetFloat(kRandom_param1, (chTime->GetValue(i)).TimeValue());
					if (pp2Anim) pp2 = pblock()->GetFloat(kRandom_posParam2, (chTime->GetValue(i)).TimeValue());
					if (pp2 <= 0.0f) chFloat->SetValue(i, fp1);
					else {
						float x1, x2, w, r1;
						do {
							x1 = rand->Rand11();
							x2 = rand->Rand11();
							w = x1*x1 + x2*x2;
						} while ( w >= 1.0f);
						if (w != 0.0f) w = sqrt((-2.0f*log(w))/w);
						r1 = x1*w*sqrt(pp2) + fp1;
						chFloat->SetValue(i, r1);
					}
				}						
			break;

		case kRandom_floatType_exponential:
			for(i=0; i<count; i++)
				if (processValue[i]) {
					if (pp1Anim) pp1 = pblock()->GetFloat(kRandom_posParam1, (chTime->GetValue(i)).TimeValue());
					if (pp1 <= 0.0f) chFloat->SetValue(i, 0.0f);
					else {
						float y = 1.0f - rand->Rand01();
						if (y <= 0.0f) chFloat->SetValue(i, 999999999.0f);
						else chFloat->SetValue(i, -pp1*log(y) );
					}
				}
			break;

		case kRandom_floatType_weibull:
			for(i=0; i<count; i++)
				if (processValue[i]) {
					if (pp1Anim) pp1 = pblock()->GetFloat(kRandom_posParam1, (chTime->GetValue(i)).TimeValue());
					if (pp2Anim) pp2 = pblock()->GetFloat(kRandom_posParam2, (chTime->GetValue(i)).TimeValue());
					if (fp3Anim) fp3 = pblock()->GetFloat(kRandom_param3, (chTime->GetValue(i)).TimeValue());
					if ((pp1 <= 0.0f) || (pp2 <= 0.0f)) chFloat->SetValue(i, 0.0f);
					else {
						float y = 1.0f - rand->Rand01();
						if (y <= 0.0f) chFloat->SetValue(i, 999999999.0f);
						else chFloat->SetValue(i, fp3*pow(-log(y)/pp1, 1.0f/pp2));
					}
				}
			break; 
		}
		break;

	case kRandom_type_vector:
		switch(vectorType) {
		case kRandom_vectorType_uniform:
			for(i=0; i<count; i++)
				if (processValue[i]) {
					if (fp1Anim) fp1 = pblock()->GetFloat(kRandom_param1, (chTime->GetValue(i)).TimeValue());
					if (fp3Anim) fp3 = pblock()->GetFloat(kRandom_param3, (chTime->GetValue(i)).TimeValue());
					if (fp3 < fp1) fp3 = fp1;
					if (fp1 == fp3) chVector->SetValue(i, Point3(fp1,fp1,fp1));
					else {
						Point3 r;
						r.x = rand->Rand01()*(fp3-fp1) + fp1;
						r.y = rand->Rand01()*(fp3-fp1) + fp1;
						r.z = rand->Rand01()*(fp3-fp1) + fp1;
						chVector->SetValue(i, r);
					}
				}
			break;

		case kRandom_vectorType_sphereSurface:
		case kRandom_vectorType_sphereVolume:
			for(i=0; i<count; i++)
				if (processValue[i]) {
					if (pp2Anim) pp2 = pblock()->GetFloat(kRandom_posParam2, (chTime->GetValue(i)).TimeValue());
					if (pp2 <= 0.0f) chVector->SetValue(i, Point3::Origin);
					else {
						if (vectorType == kRandom_vectorType_sphereSurface)
							chVector->SetValue(i, pp2*RandSphereSurface(rand));
						else
							chVector->SetValue(i, pp2*RandSphereVolume(rand));
					}
				}
			break;

		case kRandom_vectorType_normal:
			for(i=0; i<count; i++)
				if (processValue[i]) {
					if (pp2Anim) pp2 = pblock()->GetFloat(kRandom_posParam2, (chTime->GetValue(i)).TimeValue());
					if (pp2 <= 0.0f) chVector->SetValue(i, Point3::Origin);
					else {
						float x1, x2, w;
						float coef = sqrt(pp2);
						do {
							x1 = rand->Rand11();
							x2 = rand->Rand11();
							w = x1*x1 + x2*x2;
						} while ( w >= 1.0f);
						if (w != 0.0f) w = sqrt((-2.0f*log(w))/w);
						Point3 r;
						r.x = x1*w*coef;
						r.y = x2*w*coef;
						do {
							x1 = rand->Rand11();
							x2 = rand->Rand11();
							w = x1*x1 + x2*x2;
						} while ( w >= 1.0f);
						if (w != 0.0f) w = sqrt((-2.0f*log(w))/w);
						r.z = x1*w*coef;
						chVector->SetValue(i, r);
					}
				}						

			break;
		}
		break;
	}

	return true;
}

ClassDesc* PAOperatorRandom::GetClassDesc() const
{
	return GetPAOperatorRandomDesc();
}

int PAOperatorRandom::GetRand()
{
	return pblock()->GetInt(kRandom_seed);
}

void PAOperatorRandom::SetRand(int seed)
{
	_pblock()->SetValue(kRandom_seed, 0, seed);
}


} // end of namespace PartASM