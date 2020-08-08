/**********************************************************************
 *<
	FILE:			PAOperatorFunction.cpp

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

#include "PAOperatorFunction.h"

#include "PAOperatorFunction_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPViewManager.h"
#include "PFMessages.h"
#include "IParticleChannelMult.h"

namespace PartASM {

PAOperatorFunction::PAOperatorFunction()
{ 
	GetClassDesc()->MakeAutoParamBlocks(this); 
}

FPInterfaceDesc* PAOperatorFunction::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &function_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &function_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &function_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PAOperatorFunction::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_FUNCTION_CLASS_NAME);
}

Class_ID PAOperatorFunction::ClassID()
{
	return PAOperatorFunction_Class_ID;
}

void PAOperatorFunction::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PAOperatorFunction::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefResult PAOperatorFunction::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
														PartID& partID, RefMessage message)
{
	switch(message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				switch(lastUpdateID) {
				case kFunction_opAName:
				case kFunction_opAType:
				case kFunction_opAFactor:
				case kFunction_useOpB:
				case kFunction_opBName:
				case kFunction_opBType:
				case kFunction_opBFactor:
				case kFunction_bbType:
				case kFunction_ttType:
				case kFunction_fType:
				case kFunction_ffType:
				case kFunction_vType:
				case kFunction_vfType:
				case kFunction_vvType:
				case kFunction_resName:
				case kFunction_resFactor:
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
				case kFunction_useCondition:
					RefreshUI(kFunction_message_update);
					break;
				}
				UpdatePViewUI(lastUpdateID);
			}
			break;
	}

	return REF_SUCCEED;
}

RefTargetHandle PAOperatorFunction::Clone(RemapDir &remap)
{
	PAOperatorFunction* newOp = new PAOperatorFunction();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PAOperatorFunction::GetObjectName()
{
	return GetString(IDS_OPERATOR_FUNCTION_OBJECT_NAME);
}

const ParticleChannelMask& PAOperatorFunction::ChannelsUsed(const Interval& time) const
{
								//  read						&write channels
	static ParticleChannelMask mask(PCU_New|PCU_Time|PCU_Speed|PCU_Amount, PCG_AngAxis|PCU_Spin);
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PAOperatorFunction::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_OPERATOR_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PAOperatorFunction::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_OPERATOR_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PAOperatorFunction::HasDynamicName(TSTR& nameSuffix)
{
	TSTR aName = pblock()->GetStr(kFunction_opAName, 0);
	TSTR bName = pblock()->GetStr(kFunction_opBName, 0);
	TSTR rName = pblock()->GetStr(kFunction_resName, 0);
	int  aType = pblock()->GetInt(kFunction_opAType, 0);
	int  bType = pblock()->GetInt(kFunction_opBType, 0);
	bool useSecOp = (pblock()->GetInt(kFunction_useOpB, 0) != 0);
	int spinnerPrec = GetSpinnerPrecision();
	bool useAFactor=true, useBFactor=true, useRFactor=true;
	TSTR aFactor, bFactor, rFactor;
	if (IsControlAnimated(pblock()->GetController(kFunction_opAFactor))) {
		aFactor = _T("A");
	} else {
		float f = pblock()->GetFloat(kFunction_opAFactor, 0);
		if (f == 1.0f) useAFactor = false;
		else aFactor = FormatFloatValue(f, spinnerPrec);
	}
	if (IsControlAnimated(pblock()->GetController(kFunction_opBFactor))) {
		bFactor = _T("A");
	} else {
		float f = pblock()->GetFloat(kFunction_opBFactor, 0);
		if (f == 1.0f) useBFactor = false;
		else bFactor = FormatFloatValue(f, spinnerPrec);
	}
	if (IsControlAnimated(pblock()->GetController(kFunction_resFactor))) {
		rFactor = _T("A");
	} else {
		float f = pblock()->GetFloat(kFunction_resFactor, 0);
		if (f == 1.0f) useRFactor = false;
		else rFactor = FormatFloatValue(f, spinnerPrec);
	}

	if (aType != kFunction_opAType_boolean) {
		if (useAFactor) aName = aFactor + TSTR("*(") + aName + TSTR(")");
		if (useSecOp && useBFactor) bName = bFactor + TSTR("*(") + bName + TSTR(")");
	}

	switch(aType) {
	case kFunction_opAType_boolean:
		useRFactor = false;
		if (useSecOp) {
			TSTR op;
			switch(pblock()->GetInt(kFunction_bbType,0)) {
			case kFunction_bbType_and: op = _T(")AND("); break;
			case kFunction_bbType_or: op = _T(")OR("); break;
			case kFunction_bbType_xor: op = _T(")XOR("); break; 
			}
			nameSuffix = TSTR("(")+aName+op+bName+TSTR(")");
		} else {
			nameSuffix = TSTR("NOT(")+aName+TSTR(")");
		}
		break;

	case kFunction_opAType_integer:
	case kFunction_opAType_float:
		if (useSecOp) {
			if (bType == kFunction_opBType_vector) {
				nameSuffix = TSTR("(")+aName+TSTR(")*(")+bName+TSTR(")");
			} else {
				switch(pblock()->GetInt(kFunction_ffType,0)) {
				case kFunction_ffType_add:
					nameSuffix = TSTR("(")+aName+TSTR(")+(")+bName+TSTR(")"); break;
				case kFunction_ffType_subtract:
					nameSuffix = TSTR("(")+aName+TSTR(")-(")+bName+TSTR(")"); break;
				case kFunction_ffType_multiply:
					nameSuffix = TSTR("(")+aName+TSTR(")*(")+bName+TSTR(")"); break;
				case kFunction_ffType_divide:
					nameSuffix = TSTR("(")+aName+TSTR(")/(")+bName+TSTR(")"); break;
				case kFunction_ffType_mod:
					nameSuffix = TSTR("(")+aName+TSTR(")mod(")+bName+TSTR(")"); break;
				case kFunction_ffType_power:
					nameSuffix = TSTR("(")+aName+TSTR(")**(")+bName+TSTR(")"); break;
				case kFunction_ffType_log:
					nameSuffix = TSTR("log(")+aName+TSTR(")base(")+bName+TSTR(")"); break;
				case kFunction_ffType_min:
					nameSuffix = TSTR("min(")+aName+TSTR(", ")+bName+TSTR(")"); break;
				case kFunction_ffType_max:
					nameSuffix = TSTR("max(")+aName+TSTR(", ")+bName+TSTR(")"); break;
				}
			}
		} else {
			switch(pblock()->GetInt(kFunction_fType,0)) {
			case kFunction_fType_abs:
				nameSuffix = TSTR("abs(")+aName+TSTR(")"); break;
			case kFunction_fType_inverse:
				nameSuffix = TSTR("1/(")+aName+TSTR(")"); break;
			case kFunction_fType_identity:
				nameSuffix = TSTR(aName); break;
			case kFunction_fType_square:
				nameSuffix = TSTR("(")+aName+TSTR(")**2"); break;
			case kFunction_fType_squareRoot:
				nameSuffix = TSTR("sqrt(")+aName+TSTR(")"); break;
			case kFunction_fType_cube:
				nameSuffix = TSTR("(")+aName+TSTR(")**3"); break;
			case kFunction_fType_cubicRoot:
				nameSuffix = TSTR("(")+aName+TSTR(")**(1/3)"); break;
			case kFunction_fType_nExp:
				nameSuffix = TSTR("e**(")+aName+TSTR(")"); break;
			case kFunction_fType_dExp:
				nameSuffix = TSTR("10**(")+aName+TSTR(")"); break;
			case kFunction_fType_bExp:
				nameSuffix = TSTR("2**(")+aName+TSTR(")"); break;
			case kFunction_fType_nLog:
				nameSuffix = TSTR("ln(")+aName+TSTR(")"); break;
			case kFunction_fType_dLog:
				nameSuffix = TSTR("log10(")+aName+TSTR(")"); break;
			case kFunction_fType_bLog:
				nameSuffix = TSTR("log2(")+aName+TSTR(")"); break;
			case kFunction_fType_sin:
				nameSuffix = TSTR("sin(")+aName+TSTR(")"); break;
			case kFunction_fType_cos:
				nameSuffix = TSTR("cos(")+aName+TSTR(")"); break;
			case kFunction_fType_tg:
				nameSuffix = TSTR("tg(")+aName+TSTR(")"); break;
			case kFunction_fType_ctg:
				nameSuffix = TSTR("ctg(")+aName+TSTR(")"); break;
			}
		}
		break;

	case kFunction_opAType_time:
		switch(pblock()->GetInt(kFunction_ttType,0)) {
		case kFunction_ttType_add:
			nameSuffix = TSTR("(")+aName+TSTR(")+(")+bName+TSTR(")"); break;
		case kFunction_ttType_subtract:
			nameSuffix = TSTR("(")+aName+TSTR(")-(")+bName+TSTR(")"); break;
		}
		break;

	case kFunction_opAType_vector:
		if (useSecOp) {
			if (bType == kFunction_opBType_vector) {
				switch(pblock()->GetInt(kFunction_vvType,0)) {
				case kFunction_vvType_add:
					nameSuffix = TSTR("(")+aName+TSTR(")+(")+bName+TSTR(")"); break;
				case kFunction_vvType_subtract:
					nameSuffix = TSTR("(")+aName+TSTR(")-(")+bName+TSTR(")"); break;
				case kFunction_vvType_distance:
					nameSuffix = TSTR("length(")+aName+TSTR(")-(")+bName+TSTR(")"); break;
				case kFunction_vvType_dot:
					nameSuffix = TSTR("DotProd(")+aName+TSTR(", ")+bName+TSTR(")"); break;
				case kFunction_vvType_cross:
					nameSuffix = TSTR("CrossProd(")+aName+TSTR(", ")+bName+TSTR(")"); break;
				case kFunction_vvType_addRotation:
					nameSuffix = TSTR("AddRotations(")+aName+TSTR(", ")+bName+TSTR(")"); break;
				case kFunction_vvType_scaleProd:
					nameSuffix = TSTR("ScaleProd(")+aName+TSTR(", ")+bName+TSTR(")"); break;
				case kFunction_vvType_conform:
					nameSuffix = TSTR("(")+aName+TSTR(")normalTo(")+bName+TSTR(")"); break;
				case kFunction_vvType_rotateBy:
					nameSuffix = TSTR("(")+aName+TSTR(")rotateBy(")+bName+TSTR(")"); break;
				}
			} else {
				switch(pblock()->GetInt(kFunction_vfType,0)) {
				case kFunction_vfType_multiply:
					nameSuffix = TSTR("(")+aName+TSTR(")*(")+bName+TSTR(")"); break;
				case kFunction_vfType_divide:
					nameSuffix = TSTR("(")+aName+TSTR(")/(")+bName+TSTR(")"); break;
				case kFunction_vfType_deviate:
					nameSuffix = TSTR("(")+aName+TSTR(")deviateBy(")+bName+TSTR(")"); break;
				}
			}
		} else {
			switch(pblock()->GetInt(kFunction_vType,0)) {
			case kFunction_vType_norm:
				nameSuffix = TSTR("normalize(")+aName+TSTR(")"); break;
			case kFunction_vType_length:
				nameSuffix = TSTR("length(")+aName+TSTR(")"); break;
			case kFunction_vType_identity:
				nameSuffix = TSTR(aName); break;
			}
		}
		break;
	}

	if (useRFactor) nameSuffix = rFactor + TSTR("*(") + nameSuffix + TSTR(")");
	nameSuffix = nameSuffix + TSTR("->") + rName;
	return true;
}

float DivRem(float a, float b)
{
	if (b == 0.0f) return 0.0f;
	if (b < 0.0f) b = -b;
	bool wasNeg = (a < 0.0f);
	if (wasNeg) a = -a;
	int div = int(floor(a/b));
	float rem = a - div*b;
	if (wasNeg) rem = -rem;
	return rem;
}

bool PAOperatorFunction::Proceed(IObject* pCont,
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

	TimeValue time = timeEnd.TimeValue();
	TCHAR* aName = pblock()->GetStr(kFunction_opAName, 0);
	TCHAR* bName = pblock()->GetStr(kFunction_opBName, 0);
	int  aType = pblock()->GetInt(kFunction_opAType, 0);
	int  bType = pblock()->GetInt(kFunction_opBType, 0);
	bool useSecOp = (pblock()->GetInt(kFunction_useOpB, 0) != 0);
	if (aType == kFunction_opAType_time) useSecOp = true;

	IParticleChannelBoolR* chFilter = NULL;
	bool useCondition = (pblock()->GetInt(kFunction_useCondition, 0) != 0);
	if (useCondition) {
		TCHAR* filterName = pblock()->GetStr(kFunction_conditionName, 0);
		int filterChannelIndex;
		IObject* filterData = chMult->GetSubChannel(filterName, ParticleChannelBool_Class_ID, filterChannelIndex);
		if (filterData != NULL)
			chFilter = (IParticleChannelBoolR*)filterData->GetInterface(PARTICLECHANNELBOOLR_INTERFACE);
		if (chFilter == NULL) useCondition = false;
	}

	bool newOnly = (pblock()->GetInt(kFunction_assignOnce, 0) != 0);
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

	bool useAFactor=true, useBFactor=true, useRFactor=true;
	bool isAAnim=false, isBAnim=false, isRAnim=false;
	float aFactor=0.0f, bFactor=0.0f, rFactor=0.0f;
	if (IsControlAnimated(pblock()->GetController(kFunction_opAFactor))) {
		isAAnim = true;
	} else {
		aFactor = pblock()->GetFloat(kFunction_opAFactor, 0);
		useAFactor = (aFactor != 1.0f);
	}
	if (IsControlAnimated(pblock()->GetController(kFunction_opBFactor))) {
		isBAnim = true;
	} else {
		bFactor = pblock()->GetFloat(kFunction_opBFactor, 0);
		useBFactor = (bFactor != 1.0f);
	}
	if (IsControlAnimated(pblock()->GetController(kFunction_resFactor))) {
		isRAnim = true;
	} else {
		rFactor = pblock()->GetFloat(kFunction_resFactor, 0);
		useRFactor = (rFactor != 1.0f);
	}

	IParticleChannelBoolR *chBoolA=NULL, *chBoolB=NULL;
	IParticleChannelPTVR *chTimeA=NULL, *chTimeB=NULL;
	IParticleChannelIntR *chIntA=NULL, *chIntB=NULL;
	IParticleChannelFloatR *chFloatA=NULL, *chFloatB=NULL;
	IParticleChannelPoint3R *chVectorA=NULL, *chVectorB=NULL;

	Tab<bool> boolDataA, boolDataB;
	Tab<int> intDataA, intDataB;
	Tab<float> floatDataA, floatDataB;
	Tab<PreciseTimeValue> timeDataA, timeDataB;
	Tab<Point3> vectorDataA, vectorDataB;

	switch(bType) {
	case kFunction_opBType_integer: bType = kFunction_opType_integer; break;
	case kFunction_opBType_float: bType = kFunction_opType_float; break;
	case kFunction_opBType_vector: bType = kFunction_opType_vector; break;
	}

	IObject* dataChannel = NULL;
	int subChannelIndex = -1;
	switch(aType) {
	case kFunction_opType_boolean:
		dataChannel = chMult->GetSubChannel(aName, ParticleChannelBool_Class_ID, subChannelIndex);
		if (dataChannel != NULL)
			chBoolA = (IParticleChannelBoolR*)dataChannel->GetInterface(PARTICLECHANNELBOOLR_INTERFACE);
		if (chBoolA == NULL) return false;
		bType = kFunction_opType_boolean;
		boolDataA.SetCount(count);
		for(i=0; i<count; i++)
			if (processValue[i])
				boolDataA[i] = chBoolA->GetValue(i);
		break;
	case kFunction_opType_time:
		dataChannel = chMult->GetSubChannel(aName, ParticleChannelPTV_Class_ID, subChannelIndex);
		if (dataChannel != NULL)
			chTimeA = (IParticleChannelPTVR*)dataChannel->GetInterface(PARTICLECHANNELPTVR_INTERFACE);
		if (chTimeA == NULL) return false;
		bType = kFunction_opType_time;
		timeDataA.SetCount(count);
		for(i=0; i<count; i++)
			if (processValue[i]) {
				timeDataA[i] = chTimeA->GetValue(i);
				if (useAFactor)
					timeDataA[i] *= (isAAnim ? pblock()->GetFloat(kFunction_opAFactor, (chTime->GetValue(i)).TimeValue()) : aFactor);
			}
		break;
	case kFunction_opType_integer:
		dataChannel = chMult->GetSubChannel(aName, ParticleChannelInt_Class_ID, subChannelIndex);
		if (dataChannel != NULL)
			chIntA = (IParticleChannelIntR*)dataChannel->GetInterface(PARTICLECHANNELINTR_INTERFACE);
		if (chIntA == NULL) return false;
		if (useAFactor) {
			aType = kFunction_opType_float;
			floatDataA.SetCount(count);
			for(i=0; i<count; i++)
				if (processValue[i])
					floatDataA[i] = (isAAnim ? pblock()->GetFloat(kFunction_opAFactor, (chTime->GetValue(i)).TimeValue()) : aFactor)*chIntA->GetValue(i);
		} else {
			intDataA.SetCount(count);
			for(i=0; i<count; i++)
				if (processValue[i])
					intDataA[i] = chIntA->GetValue(i);
		}
		break;
	case kFunction_opType_float:
		dataChannel = chMult->GetSubChannel(aName, ParticleChannelFloat_Class_ID, subChannelIndex);
		if (dataChannel != NULL)
			chFloatA = (IParticleChannelFloatR*)dataChannel->GetInterface(PARTICLECHANNELFLOATR_INTERFACE);
		if (chFloatA == NULL) return false;
		floatDataA.SetCount(count);
		for(i=0; i<count; i++) 
			if (processValue[i]) {
				floatDataA[i] = chFloatA->GetValue(i);
				if (useAFactor)
					floatDataA[i] = (isAAnim ? pblock()->GetFloat(kFunction_opAFactor, (chTime->GetValue(i)).TimeValue()) : aFactor)*floatDataA[i];
			}
		break;
	case kFunction_opType_vector:
		dataChannel = chMult->GetSubChannel(aName, ParticleChannelPoint3_Class_ID, subChannelIndex);
		if (dataChannel != NULL)
			chVectorA = (IParticleChannelPoint3R*)dataChannel->GetInterface(PARTICLECHANNELPOINT3R_INTERFACE);
		if (chVectorA == NULL) return false;
		vectorDataA.SetCount(count);
		for(i=0; i<count; i++)
			if (processValue[i]) {
				vectorDataA[i] = chVectorA->GetValue(i);
				if (useAFactor)
					vectorDataA[i] = (isAAnim ? pblock()->GetFloat(kFunction_opAFactor, (chTime->GetValue(i)).TimeValue()) : aFactor)*vectorDataA[i];
			}
		break;
	}

	if (useSecOp) {
		switch(bType) {
		case kFunction_opType_boolean:
			dataChannel = chMult->GetSubChannel(bName, ParticleChannelBool_Class_ID, subChannelIndex);
			if (dataChannel != NULL)
				chBoolB = (IParticleChannelBoolR*)dataChannel->GetInterface(PARTICLECHANNELBOOLR_INTERFACE);
			if (chBoolB == NULL) return false;
			boolDataB.SetCount(count);
			for(i=0; i<count; i++)
				if (processValue[i])
					boolDataB[i] = chBoolB->GetValue(i);
			break;
		case kFunction_opType_time:
			dataChannel = chMult->GetSubChannel(bName, ParticleChannelPTV_Class_ID, subChannelIndex);
			if (dataChannel != NULL)
				chTimeB = (IParticleChannelPTVR*)dataChannel->GetInterface(PARTICLECHANNELPTVR_INTERFACE);
			if (chTimeB == NULL) return false;
			timeDataB.SetCount(count);
			for(i=0; i<count; i++)
				if (processValue[i]) {
					timeDataB[i] = chTimeB->GetValue(i);
					if (useBFactor)
						timeDataB[i] *= (isBAnim ? pblock()->GetFloat(kFunction_opBFactor, (chTime->GetValue(i)).TimeValue()) : bFactor);
				}
			break;
		case kFunction_opType_integer:
			dataChannel = chMult->GetSubChannel(bName, ParticleChannelInt_Class_ID, subChannelIndex);
			if (dataChannel != NULL)
				chIntB = (IParticleChannelIntR*)dataChannel->GetInterface(PARTICLECHANNELINTR_INTERFACE);
			if (chIntB == NULL) return false;
			if (useBFactor) {
				bType = kFunction_opType_float;
				floatDataB.SetCount(count);
				for(i=0; i<count; i++)
					if (processValue[i])
						floatDataB[i] = (isBAnim ? pblock()->GetFloat(kFunction_opBFactor, (chTime->GetValue(i)).TimeValue()) : bFactor)*chIntB->GetValue(i);
			} else {
				intDataB.SetCount(count);
				for(i=0; i<count; i++)
					if (processValue[i])
						intDataB[i] = chIntB->GetValue(i);
			}
			break;
		case kFunction_opType_float:
			dataChannel = chMult->GetSubChannel(bName, ParticleChannelFloat_Class_ID, subChannelIndex);
			if (dataChannel != NULL)
				chFloatB = (IParticleChannelFloatR*)dataChannel->GetInterface(PARTICLECHANNELFLOATR_INTERFACE);
			if (chFloatB == NULL) return false;
			floatDataB.SetCount(count);
			for(i=0; i<count; i++) 
				if (processValue[i]) {
					floatDataB[i] = chFloatB->GetValue(i);
					if (useBFactor)
						floatDataB[i] = (isBAnim ? pblock()->GetFloat(kFunction_opBFactor, (chTime->GetValue(i)).TimeValue()) : bFactor)*floatDataB[i];
				}
			break;
		case kFunction_opType_vector:
			dataChannel = chMult->GetSubChannel(bName, ParticleChannelPoint3_Class_ID, subChannelIndex);
			if (dataChannel != NULL)
				chVectorB = (IParticleChannelPoint3R*)dataChannel->GetInterface(PARTICLECHANNELPOINT3R_INTERFACE);
			if (chVectorB == NULL) return false;
			vectorDataB.SetCount(count);
			for(i=0; i<count; i++)
				if (processValue[i]) {
					vectorDataB[i] = chVectorB->GetValue(i);
					if (useBFactor)
						vectorDataB[i] = (isBAnim ? pblock()->GetFloat(kFunction_opBFactor, (chTime->GetValue(i)).TimeValue()) : bFactor)*vectorDataB[i];
				}
			break;
		}
	}

	Tab<bool> boolData;
	Tab<int> intData;
	Tab<float> floatData;
	Tab<PreciseTimeValue> timeData;
	Tab<Point3> vectorData;
	int intValue;
	float floatValue;
	Point3 vectorValue, vectorValue1, vectorValue2;
	int rType = -1;

	switch(aType) {
	case kFunction_opType_boolean:
		rType = kFunction_opType_boolean;
		boolData.SetCount(count);
		if (useSecOp) {
			switch(pblock()->GetInt(kFunction_bbType,0)) {
			case kFunction_bbType_and:
				for(i=0; i<count; i++)
					if (processValue[i])
						boolData[i] = boolDataA[i] && boolDataB[i];
				break;
			case kFunction_bbType_or:
				for(i=0; i<count; i++)
					if (processValue[i])
						boolData[i] = boolDataA[i] || boolDataB[i];
				break;
			case kFunction_bbType_xor:
				for(i=0; i<count; i++)
					if (processValue[i])
						boolData[i] = (boolDataA[i]) ? !boolDataB[i] : boolDataB[i];
				break;
			}
		} else {
			for(i=0; i<count; i++)
				if (processValue[i])
					boolData[i] = !boolDataA[i];
		}
		break;

	case kFunction_opType_integer:
	case kFunction_opType_float:
		if (useSecOp) {
			if (bType == kFunction_opType_vector) {
				rType = kFunction_opType_vector;
				vectorData.SetCount(count);
				if (aType == kFunction_opType_integer) {
					for(i=0; i<count; i++)
						if (processValue[i])
							vectorData[i] = float(intDataA[i])*vectorDataB[i];
				} else {
					for(i=0; i<count; i++)
						if (processValue[i])
							vectorData[i] = floatDataA[i]*vectorDataB[i];
				}
			} else {
				rType = kFunction_opType_float;
				floatData.SetCount(count);
				switch(pblock()->GetInt(kFunction_ffType,0)) {
				case kFunction_ffType_add:
					if (aType == kFunction_opType_integer) {
						if (bType == kFunction_opType_integer) {
							for(i=0; i<count; i++)
								if (processValue[i])
									floatData[i] = float(intDataA[i]+intDataB[i]);
						} else {
							for(i=0; i<count; i++)
								if (processValue[i])
									floatData[i] = float(intDataA[i])+floatDataB[i];
						}
					} else {
						if (bType == kFunction_opType_integer) {
							for(i=0; i<count; i++)
								if (processValue[i])
									floatData[i] = floatDataA[i]+float(intDataB[i]);
						} else {
							for(i=0; i<count; i++)
								if (processValue[i])
									floatData[i] = floatDataA[i]+floatDataB[i];
						}
					}
					break;

				case kFunction_ffType_subtract:
					if (aType == kFunction_opType_integer) {
						if (bType == kFunction_opType_integer) {
							for(i=0; i<count; i++)
								if (processValue[i])
									floatData[i] = float(intDataA[i]-intDataB[i]);
						} else {
							for(i=0; i<count; i++)
								if (processValue[i])
									floatData[i] = float(intDataA[i])-floatDataB[i];
						}
					} else {
						if (bType == kFunction_opType_integer) {
							for(i=0; i<count; i++)
								if (processValue[i])
									floatData[i] = floatDataA[i]-float(intDataB[i]);
						} else {
							for(i=0; i<count; i++)
								if (processValue[i])
									floatData[i] = floatDataA[i]-floatDataB[i];
						}
					}
					break;

				case kFunction_ffType_multiply:
					if (aType == kFunction_opType_integer) {
						if (bType == kFunction_opType_integer) {
							for(i=0; i<count; i++)
								if (processValue[i])
									floatData[i] = float(intDataA[i]*intDataB[i]);
						} else {
							for(i=0; i<count; i++)
								if (processValue[i])
									floatData[i] = float(intDataA[i])*floatDataB[i];
						}
					} else {
						if (bType == kFunction_opType_integer) {
							for(i=0; i<count; i++)
								if (processValue[i])
									floatData[i] = floatDataA[i]*float(intDataB[i]);
						} else {
							for(i=0; i<count; i++)
								if (processValue[i]) {
									float ff1 = floatDataA[i];
									float ff2 = floatDataB[i];
									floatData[i] = floatDataA[i]*floatDataB[i];
									float ff3 = floatData[i];
								}
						}
					}
					break;

				case kFunction_ffType_divide:
					if (aType == kFunction_opType_integer) {
						if (bType == kFunction_opType_integer) {
							for(i=0; i<count; i++)
								if (processValue[i]) {
									intValue = intDataB[i];
									floatData[i] = (intValue == 0) ? 0.0f : float(intDataA[i])/float(intValue);
								}
						} else {
							for(i=0; i<count; i++)
								if (processValue[i]) {
									floatValue = floatDataB[i];
									floatData[i] = (floatValue == 0.0f) ? 0.0f : float(intDataA[i])/floatValue;
								}
						}
					} else {
						if (bType == kFunction_opType_integer) {
							for(i=0; i<count; i++)
								if (processValue[i]) {
									intValue = intDataB[i];
									floatData[i] = (intValue == 0) ? 0.0f : floatDataA[i]/float(intValue);
								}
						} else {
							for(i=0; i<count; i++)
								if (processValue[i]) {
									floatValue = floatDataB[i];
									floatData[i] = (floatValue == 0.0f) ? 0.0f : floatDataA[i]/floatValue;
								}
						}
					}
					break;

				case kFunction_ffType_mod:
					if (aType == kFunction_opType_integer) {
						if (bType == kFunction_opType_integer) {
							for(i=0; i<count; i++)
								if (processValue[i]) {
									intValue = intDataB[i];
									floatData[i] = (intValue == 0) ? 0.0f : float(intDataA[i]%intValue);
								}
						} else {
							for(i=0; i<count; i++)
								if (processValue[i])
									floatData[i] = DivRem(float(intDataA[i]), floatDataB[i]);
						}
					} else {
						if (bType == kFunction_opType_integer) {
							for(i=0; i<count; i++)
								if (processValue[i])
									floatData[i] = DivRem(floatDataA[i], float(intDataB[i]));
						} else {
							for(i=0; i<count; i++)
								if (processValue[i])
									floatData[i] = DivRem(floatDataA[i], floatDataB[i]);
						}
					}
					break;

				case kFunction_ffType_power:
					if (aType == kFunction_opType_integer) {
						if (bType == kFunction_opType_integer) {
							for(i=0; i<count; i++)
								if (processValue[i]) {
									if ((intDataA[i] == 0) && (intDataB[i] < 0)) floatData[i] = 0.0f;
									else floatData[i] = pow(float(intDataA[i]), float(intDataB[i]));
								}
						} else {
							for(i=0; i<count; i++)
								if (processValue[i]) {
									if ((intDataA[i] == 0) && (floatDataB[i] < 0.0f)) floatData[i] = 0.0f;
									else floatData[i] = pow(float(intDataA[i]), floatDataB[i]);
								}
						}
					} else {
						if (bType == kFunction_opType_integer) {
							for(i=0; i<count; i++)
								if (processValue[i]) {
									if ((floatDataA[i] == 0.0f) && (intDataB[i] < 0)) floatData[i] = 0.0f;
									else floatData[i] = pow(floatDataA[i], float(intDataB[i]));
								}
						} else {
							for(i=0; i<count; i++)
								if (processValue[i]) {
									if ((floatDataA[i] == 0.0f) && (floatDataB[i] < 0.0f)) floatData[i] = 0.0f;
									else floatData[i] = pow(floatDataA[i], floatDataB[i]);
								}
						}
					}
					break;

				case kFunction_ffType_log:
					if (aType == kFunction_opType_integer) {
						if (bType == kFunction_opType_integer) {
							for(i=0; i<count; i++)
								if (processValue[i]) {
									if ((intDataA[i] <= 0) && (intDataB[i] <= 0)) floatData[i] = 0.0f;
									else floatData[i] = log(float(intDataA[i]))/log(float(intDataB[i]));
								}
						} else {
							for(i=0; i<count; i++)
								if (processValue[i]) {
									if ((intDataA[i] <= 0) && (floatDataB[i] <= 0.0f)) floatData[i] = 0.0f;
									else floatData[i] = log(float(intDataA[i]))/log(floatDataB[i]);
								}
						}
					} else {
						if (bType == kFunction_opType_integer) {
							for(i=0; i<count; i++)
								if (processValue[i]) {
									if ((floatDataA[i] <= 0.0f) && (intDataB[i] <= 0)) floatData[i] = 0.0f;
									else floatData[i] = log(floatDataA[i])/log(float(intDataB[i]));
								}
						} else {
							for(i=0; i<count; i++)
								if (processValue[i]) {
									if ((floatDataA[i] <= 0.0f) && (floatDataB[i] <= 0.0f)) floatData[i] = 0.0f;
									else floatData[i] = log(floatDataA[i])/log(floatDataB[i]);
								}
						}
					}
					break;

				case kFunction_ffType_min:
					if (aType == kFunction_opType_integer) {
						if (bType == kFunction_opType_integer) {
							for(i=0; i<count; i++)
								if (processValue[i])
									floatData[i] = float(__min(intDataA[i],intDataB[i]));
						} else {
							for(i=0; i<count; i++)
								if (processValue[i]) {
									floatValue = intDataA[i];
									floatData[i] = __min(floatValue,floatDataB[i]);
								}
						}
					} else {
						if (bType == kFunction_opType_integer) {
							for(i=0; i<count; i++)
								if (processValue[i]) {
									floatValue = intDataB[i];
									floatData[i] = __min(floatDataA[i],floatValue);
								}
						} else {
							for(i=0; i<count; i++)
								if (processValue[i])
									floatData[i] = __min(floatDataA[i],floatDataB[i]);
						}
					}
					break;

				case kFunction_ffType_max:
					if (aType == kFunction_opType_integer) {
						if (bType == kFunction_opType_integer) {
							for(i=0; i<count; i++)
								if (processValue[i])
									floatData[i] = float(__max(intDataA[i],intDataB[i]));
						} else {
							for(i=0; i<count; i++)
								if (processValue[i]) {
									floatValue = intDataA[i];
									floatData[i] = __max(floatValue,floatDataB[i]);
								}
						}
					} else {
						if (bType == kFunction_opType_integer) {
							for(i=0; i<count; i++)
								if (processValue[i]) {
									floatValue = intDataB[i];
									floatData[i] = __max(floatDataA[i],floatValue);
								}
						} else {
							for(i=0; i<count; i++)
								if (processValue[i])
									floatData[i] = __max(floatDataA[i],floatDataB[i]);
						}
					}
					break;
				}
			}
		} else {
			rType = kFunction_opType_float;
			floatData.SetCount(count);
			switch(pblock()->GetInt(kFunction_fType,0)) {
			case kFunction_fType_abs:
				if (bType == kFunction_opType_integer) {
					for(i=0; i<count; i++)
						if (processValue[i])
							floatData[i] = fabs(float(intDataA[i]));
				} else {
					for(i=0; i<count; i++)
						if (processValue[i])
							floatData[i] = fabs(floatDataA[i]);
				}
				break;

			case kFunction_fType_inverse:
				if (bType == kFunction_opType_integer) {
					for(i=0; i<count; i++)
						if (processValue[i]) {
							floatValue = float(intDataA[i]);
							floatData[i] = (intDataA[i] != 0) ? 1.0f/floatValue : 0.0f;
						}
				} else {
					for(i=0; i<count; i++)
						if (processValue[i])
							floatData[i] = (floatDataA[i] != 0.0f) ? 1.0f/(floatDataA[i]) : 0.0f;
				}
				break;

			case kFunction_fType_identity:
				if (bType == kFunction_opType_integer) {
					for(i=0; i<count; i++)
						if (processValue[i]) floatData[i] = float(intDataA[i]);
				} else {
					for(i=0; i<count; i++)
						if (processValue[i])
							floatData[i] = floatDataA[i];
				}
				break;

			case kFunction_fType_square:
				if (bType == kFunction_opType_integer) {
					for(i=0; i<count; i++)
						if (processValue[i]) {
							intValue = intDataA[i];
							floatData[i] = float(intValue*intValue);
						}
				} else {
					for(i=0; i<count; i++)
						if (processValue[i]) {
							floatValue = floatDataA[i];
							floatData[i] = floatValue*floatValue;
						}
				}
				break;

			case kFunction_fType_squareRoot:
				if (bType == kFunction_opType_integer) {
					for(i=0; i<count; i++)
						if (processValue[i]) {
							intValue = intDataA[i];
							if (intValue <= 0) floatData[i] = 0.0f;
							else floatData[i] = sqrt(float(intValue));
						}
				} else {
					for(i=0; i<count; i++)
						if (processValue[i]) {
							floatValue = floatDataA[i];
							if (floatValue <= 0.0f) floatData[i] = 0.0f;
							else floatData[i] = sqrt(floatValue);
						}
				}
				break;

			case kFunction_fType_cube:
				if (bType == kFunction_opType_integer) {
					for(i=0; i<count; i++)
						if (processValue[i]) {
							intValue = intDataA[i];
							floatData[i] = float(intValue*intValue*intValue);
						}
				} else {
					for(i=0; i<count; i++)
						if (processValue[i]) {
							floatValue = floatDataA[i];
							floatData[i] = floatValue*floatValue*floatValue;
						}
				}
				break;

			case kFunction_fType_cubicRoot:
				if (bType == kFunction_opType_integer) {
					for(i=0; i<count; i++)
						if (processValue[i]) 
							floatData[i] = pow(float(intDataA[i]),1.0f/3.0f);
				} else {
					for(i=0; i<count; i++)
						if (processValue[i])
							floatData[i] = pow(floatDataA[i],1.0f/3.0f);
				}
				break;

			case kFunction_fType_nExp:
				if (bType == kFunction_opType_integer) {
					for(i=0; i<count; i++)
						if (processValue[i]) 
							floatData[i] = exp(float(intDataA[i]));
				} else {
					for(i=0; i<count; i++)
						if (processValue[i])
							floatData[i] = exp(floatDataA[i]);
				}
				break;

			case kFunction_fType_dExp:
				if (bType == kFunction_opType_integer) {
					for(i=0; i<count; i++)
						if (processValue[i]) 
							floatData[i] = pow(10.0f, float(intDataA[i]));
				} else {
					for(i=0; i<count; i++)
						if (processValue[i])
							floatData[i] = pow(10.0f, floatDataA[i]);
				}
				break;

			case kFunction_fType_bExp:
				if (bType == kFunction_opType_integer) {
					for(i=0; i<count; i++)
						if (processValue[i]) 
							floatData[i] = pow(2.0f, float(intDataA[i]));
				} else {
					for(i=0; i<count; i++)
						if (processValue[i])
							floatData[i] = pow(2.0f, floatDataA[i]);
				}
				break;

			case kFunction_fType_nLog:
				if (bType == kFunction_opType_integer) {
					for(i=0; i<count; i++)
						if (processValue[i]) {
							intValue = intDataA[i];
							floatData[i] = (intValue > 0) ? log(float(intValue)) : 0.0f;
						}
				} else {
					for(i=0; i<count; i++)
						if (processValue[i]) {
							floatValue = floatDataA[i];
							floatData[i] = (floatValue > 0.0f) ? log(floatValue) : 0.0f;
						}
				}
				break;

			case kFunction_fType_dLog:
				if (bType == kFunction_opType_integer) {
					for(i=0; i<count; i++)
						if (processValue[i]) {
							intValue = intDataA[i];
							floatData[i] = (intValue > 0) ? log10(float(intValue)) : 0.0f;
						}
				} else {
					for(i=0; i<count; i++)
						if (processValue[i]) {
							floatValue = floatDataA[i];
							floatData[i] = (floatValue > 0.0f) ? log10(floatValue) : 0.0f;
						}
				}
				break;

			case kFunction_fType_bLog:
				if (bType == kFunction_opType_integer) {
					for(i=0; i<count; i++)
						if (processValue[i]) {
							intValue = intDataA[i];
							floatData[i] = (intValue > 0) ? log(2.0f)/log(float(intValue)) : 0.0f;
						}
				} else {
					for(i=0; i<count; i++)
						if (processValue[i]) {
							floatValue = floatDataA[i];
							floatData[i] = (floatValue > 0.0f) ? log(2.0f)/log(floatValue) : 0.0f;
						}
				}
				break;

			case kFunction_fType_sin:
				if (bType == kFunction_opType_integer) {
					for(i=0; i<count; i++)
						if (processValue[i]) 
							floatData[i] = sin(float(intDataA[i]));
				} else {
					for(i=0; i<count; i++)
						if (processValue[i])
							floatData[i] = sin(floatDataA[i]);
				}
				break;

			case kFunction_fType_cos:
				if (bType == kFunction_opType_integer) {
					for(i=0; i<count; i++)
						if (processValue[i]) 
							floatData[i] = cos(float(intDataA[i]));
				} else {
					for(i=0; i<count; i++)
						if (processValue[i])
							floatData[i] = cos(floatDataA[i]);
				}
				break;

			case kFunction_fType_tg:
				if (bType == kFunction_opType_integer) {
					for(i=0; i<count; i++)
						if (processValue[i]) 
							floatData[i] = tan(float(intDataA[i]));
				} else {
					for(i=0; i<count; i++)
						if (processValue[i])
							floatData[i] = tan(floatDataA[i]);
				}
				break;

			case kFunction_fType_ctg:
				if (bType == kFunction_opType_integer) {
					for(i=0; i<count; i++)
						if (processValue[i]) {
							intValue = intDataA[i];
							floatData[i] = (intValue == 0) ? 0.0f : 1.0f/tan(float(intValue));
						}
				} else {
					for(i=0; i<count; i++)
						if (processValue[i]) {
							floatValue = floatDataA[i];
							floatData[i] = (floatValue == 0.0f) ? 0.0f : 1.0f/tan(floatValue);
						}
				}
				break;
			}
		}
		break;

	case kFunction_opType_time:
		rType = kFunction_opType_time;
		timeData.SetCount(count);
		switch(pblock()->GetInt(kFunction_ttType,0)) {
		case kFunction_ttType_add:
			for(i=0; i<count; i++)
				if (processValue[i])
					timeData[i] = timeDataA[i] + timeDataB[i];
			break;

		case kFunction_ttType_subtract:
			for(i=0; i<count; i++)
				if (processValue[i])
					timeData[i] = timeDataA[i] - timeDataB[i];
			break;
		}
		break;

	case kFunction_opType_vector:
		if (useSecOp) {
			if (bType == kFunction_opType_vector) {
				switch(pblock()->GetInt(kFunction_vvType,0)) {
				case kFunction_vvType_add:
					rType = kFunction_opType_vector;
					vectorData.SetCount(count);
					for(i=0; i<count; i++)
						if (processValue[i])
							vectorData[i] = vectorDataA[i] + vectorDataB[i];
					break;

				case kFunction_vvType_subtract:
					rType = kFunction_opType_vector;
					vectorData.SetCount(count);
					for(i=0; i<count; i++)
						if (processValue[i])
							vectorData[i] = vectorDataA[i] - vectorDataB[i];
					break;

				case kFunction_vvType_distance:
					rType = kFunction_opType_float;
					floatData.SetCount(count);
					for(i=0; i<count; i++)
						if (processValue[i])
							floatData[i] = Length(vectorDataA[i] - vectorDataB[i]);
					break;

				case kFunction_vvType_dot:
					rType = kFunction_opType_float;
					floatData.SetCount(count);
					for(i=0; i<count; i++)
						if (processValue[i])
							floatData[i] = DotProd(vectorDataA[i], vectorDataB[i]);
					break;

				case kFunction_vvType_cross:
					rType = kFunction_opType_vector;
					vectorData.SetCount(count);
					for(i=0; i<count; i++)
						if (processValue[i])
							vectorData[i] = CrossProd(vectorDataA[i], vectorDataB[i]);
					break;

				case kFunction_vvType_addRotation:
					rType = kFunction_opType_vector;
					vectorData.SetCount(count);
					for(i=0; i<count; i++)
						if (processValue[i]) {
							Quat q1,q2;
							vectorValue1 = vectorDataA[i];
							vectorValue2 = vectorDataB[i];
							EulerToQuat(&(vectorValue1.x), q1, EULERTYPE_XYZ);
							EulerToQuat(&(vectorValue2.x), q2, EULERTYPE_XYZ);
							QuatToEuler(q1+q2, &(vectorValue.x)); 
							vectorData[i] = vectorValue;
						}
					break;

				case kFunction_vvType_scaleProd:
					rType = kFunction_opType_vector;
					vectorData.SetCount(count);
					for(i=0; i<count; i++)
						if (processValue[i]) {
							vectorValue1 = vectorDataA[i];
							vectorValue2 = vectorDataB[i];
							vectorData[i] = Point3(vectorValue1.x*vectorValue2.x, vectorValue1.y*vectorValue2.y, vectorValue1.z*vectorValue2.z);
						}
					break;

				case kFunction_vvType_conform:
					rType = kFunction_opType_vector;
					vectorData.SetCount(count);
					for(i=0; i<count; i++)
						if (processValue[i]) {
							vectorValue1 = vectorDataA[i];
							vectorValue2 = Normalize(vectorDataB[i]);
							vectorData[i] = vectorValue1 - vectorValue2*DotProd(vectorValue1,vectorValue2);
						}
					break;

				case kFunction_vvType_rotateBy:
					rType = kFunction_opType_vector;
					vectorData.SetCount(count);
					for(i=0; i<count; i++)
						if (processValue[i]) {
							vectorValue1 = vectorDataA[i];
							vectorValue2 = vectorDataB[i];
							Quat q;
							EulerToQuat(&(vectorValue2.x), q, EULERTYPE_XYZ);
							Matrix3 m;
							q.MakeMatrix(m);
							vectorData[i] = vectorValue1*m;
						}
					break;
				}
			} else {
				rType = kFunction_opType_vector;
				vectorData.SetCount(count);
				switch(pblock()->GetInt(kFunction_vfType,0)) {
				case kFunction_vfType_multiply:
					if (bType == kFunction_opType_integer) {					
						for(i=0; i<count; i++)
							if (processValue[i]) 
								vectorData[i] = float(intDataB[i])*vectorDataA[i];
					} else {
						for(i=0; i<count; i++)
							if (processValue[i])
								vectorData[i] = floatDataB[i]*vectorDataA[i];
					}
					break;

				case kFunction_vfType_divide:
					if (bType == kFunction_opType_integer) {					
						for(i=0; i<count; i++)
							if (processValue[i]) {
								intValue = intDataB[i];
								vectorData[i] = (intValue != 0) ? vectorDataA[i]/float(intDataB[i]) : Point3::Origin;
							}
					} else {
						for(i=0; i<count; i++)
							if (processValue[i]) {
								floatValue = floatDataB[i];
								vectorData[i] = (floatValue != 0.0f) ? vectorDataA[i]/floatDataB[i] : Point3::Origin;
							}
					}
					break;

				case kFunction_vfType_deviate: {
					TCHAR* devAxisName = GetString(IDS_DEVIATIONAXIS);
					dataChannel = chMult->GetSubChannel(devAxisName, ParticleChannelPoint3_Class_ID, subChannelIndex);
					IParticleChannelPoint3R* chDevAxis = NULL;
					if (dataChannel != NULL)
						chDevAxis = (IParticleChannelPoint3R*)dataChannel->GetInterface(PARTICLECHANNELPOINT3R_INTERFACE);
					for(i=0; i<count; i++)
						if (processValue[i]) {
							Point3 axis = (chDevAxis == NULL) ? Point3::ZAxis : Normalize(chDevAxis->GetValue(i));
							float angle = (bType == kFunction_opType_integer) ? float(intDataB[i]) : floatDataB[i];
							Quat q = QFromAngAxis(angle, axis);
							Matrix3 m;
							q.MakeMatrix(m);
							vectorData[i] = vectorDataA[i]*m;
						}
											   }
					break;
				}
			}
		} else {
			switch(pblock()->GetInt(kFunction_vType,0)) {
			case kFunction_vType_norm:
				rType = kFunction_opType_vector;
				vectorData.SetCount(count);
				for(i=0; i<count; i++)
					if (processValue[i])
						vectorData[i] = Normalize(vectorDataA[i]);
				break;

			case kFunction_vType_length:
				rType = kFunction_opType_float;
				floatData.SetCount(count);
				for(i=0; i<count; i++)
					if (processValue[i])
						floatData[i] = Length(vectorDataA[i]);
				break;

			case kFunction_vType_identity:
				rType = kFunction_opType_vector;
				vectorData.SetCount(count);
				for(i=0; i<count; i++)
					if (processValue[i])
						vectorData[i] = vectorDataA[i];
				break;
			}
		}
		break;
	}

	Class_ID channelClassID;
	switch (rType) {
	case kFunction_opType_boolean:
		channelClassID = ParticleChannelBool_Class_ID;
		break;
	case kFunction_opType_time:
		channelClassID = ParticleChannelPTV_Class_ID;
		break;
	case kFunction_opType_integer:
		channelClassID = ParticleChannelInt_Class_ID;
		break;
	case kFunction_opType_float:
		channelClassID = ParticleChannelFloat_Class_ID;
		break;
	case kFunction_opType_vector:
		channelClassID = ParticleChannelPoint3_Class_ID;
		break;
	}
	TCHAR* rName = pblock()->GetStr(kFunction_resName, 0);
	dataChannel = chMult->GetSubChannel(rName, channelClassID, subChannelIndex);
	if (dataChannel == NULL) { // create the channel
		subChannelIndex = chMult->NumSubChannels();
		bool added = chMult->AddSubChannel(subChannelIndex, channelClassID);
		if (!added) return false;
		chMult->SetName(subChannelIndex, rName);
		dataChannel = chMult->GetSubChannel(subChannelIndex);
	}
	if (dataChannel == NULL) return false;
	int scopeType = pblock()->GetInt(kFunction_scope, 0);
	if (scopeType != chMult->GetScope(subChannelIndex))
		chMult->SetScope(subChannelIndex, scopeType);
	
	IParticleChannelBoolW* chBoolR=NULL;
	IParticleChannelPTVW* chTimeR=NULL;
	IParticleChannelFloatW* chFloatR=NULL;
	IParticleChannelPoint3W* chVectorR=NULL;

	switch (rType) {
	case kFunction_opType_boolean:
		chBoolR = (IParticleChannelBoolW*)dataChannel->GetInterface(PARTICLECHANNELBOOLW_INTERFACE);
		if (chBoolR == NULL) return false;
		for(i=0; i<count; i++)
			if (processValue[i])
				chBoolR->SetValue(i, boolData[i]);
		break;
	case kFunction_opType_time:
		chTimeR = (IParticleChannelPTVW*)dataChannel->GetInterface(PARTICLECHANNELPTVW_INTERFACE);
		if (chTimeR == NULL) return false;
		for(i=0; i<count; i++)
			if (processValue[i]) {
				if (useRFactor)
					timeData[i] *= (isRAnim ? pblock()->GetFloat(kFunction_resFactor, (chTime->GetValue(i)).TimeValue()) : rFactor);
				chTimeR->SetValue(i, timeData[i]);
			}
		break;
	case kFunction_opType_integer:
	case kFunction_opType_float:
		chFloatR = (IParticleChannelFloatW*)dataChannel->GetInterface(PARTICLECHANNELFLOATW_INTERFACE);
		if (chFloatR == NULL) return false;
		for(i=0; i<count; i++)
			if (processValue[i]) {
				if (useRFactor)
					floatData[i] = (isRAnim ? pblock()->GetFloat(kFunction_resFactor, (chTime->GetValue(i)).TimeValue()) : rFactor)*floatData[i];
				chFloatR->SetValue(i, floatData[i]);
			}
		break;
	case kFunction_opType_vector:
		chVectorR = (IParticleChannelPoint3W*)dataChannel->GetInterface(PARTICLECHANNELPOINT3W_INTERFACE);
		if (chVectorR == NULL) return false;
		for(i=0; i<count; i++)
			if (processValue[i]) {
				if (useRFactor)
					vectorData[i] = (isRAnim ? pblock()->GetFloat(kFunction_resFactor, (chTime->GetValue(i)).TimeValue()) : rFactor)*vectorData[i];
				chVectorR->SetValue(i, vectorData[i]);
			}
		break;
	}

	return true;
}

ClassDesc* PAOperatorFunction::GetClassDesc() const
{
	return GetPAOperatorFunctionDesc();
}

} // end of namespace PartASM