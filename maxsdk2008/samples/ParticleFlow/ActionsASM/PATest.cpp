/**********************************************************************
 *<
	FILE: PATest.h

	DESCRIPTION: Duration Test implementation
				 The Test checks particles for either absolute time
				 or particle age or time period particle is present
				 in the current Event

	CREATED BY: Oleg Bayborodin

	HISTORY: created 12-04-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "StdAfx.h"
#include "resource.h"

#include "PartASM_SysUtil.h"
#include "PartASM_GlobalFunctions.h"
#include "PartASM_GlobalVariables.h"

#include "PATest.h"

#include "PATest_ParamBlock.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPFArrow.h"
#include "PFMessages.h"
#include "IPViewManager.h"
#include "IParticleChannelMult.h"

namespace PartASM {

// static members
Object*		PATest::m_editOb				= NULL;
IObjParam*	PATest::m_ip					= NULL;

// constructors/destructors
PATest::PATest()
{ 
	GetClassDesc()->MakeAutoParamBlocks(this); 
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From FPMixinInterface							 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc* PATest::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &test_action_FPInterfaceDesc;
	if (id == PFTEST_INTERFACE) return &test_test_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &test_PViewItem_FPInterfaceDesc;
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From Animatable									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
void PATest::GetClassName(TSTR& s)
{
	s = GetString(IDS_TEST_CLASS_NAME);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
Class_ID PATest::ClassID()
{
	return PATest_Class_ID;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PATest::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	_ip() = ip; _editOb() = this;
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PATest::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	_ip() = NULL; _editOb() = NULL;
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From ReferenceMaker								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
RefResult PATest::NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message)
{
	TimeValue curTime = 0;
	switch(message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				switch(lastUpdateID) {
				case kTest_channelType:
				case kTest_conditionType:
				case kTest_useValue2:
					RefreshUI(kTest_message_update);
				case kTest_channelName:
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					break;
				case kTest_iValue1:
				case kTest_tValue1:
				case kTest_fValue1:
				case kTest_pValue1:
				case kTest_wValue1:
				case kTest_aValue1:
				case kTest_iValue2:
				case kTest_tValue2:
				case kTest_fValue2:
				case kTest_pValue2:
				case kTest_wValue2:
				case kTest_aValue2:
					curTime = GetCOREInterface()->GetTime();
					switch(lastUpdateID) {
					case kTest_iValue1:
						if (pblock()->GetInt(kTest_iValue1,curTime) > pblock()->GetInt(kTest_iValue2,curTime))
							pblock()->SetValue(kTest_iValue2, curTime, pblock()->GetInt(kTest_iValue1,curTime));
						break;
					case kTest_tValue1:
						if (pblock()->GetTimeValue(kTest_tValue1,curTime) > pblock()->GetTimeValue(kTest_tValue2,curTime))
							pblock()->SetValue(kTest_tValue2, curTime, pblock()->GetTimeValue(kTest_tValue1,curTime));
						break;
					case kTest_fValue1:
						if (pblock()->GetFloat(kTest_fValue1,curTime) > pblock()->GetFloat(kTest_fValue2,curTime))
							pblock()->SetValue(kTest_fValue2, curTime, pblock()->GetFloat(kTest_fValue1,curTime));
						break;
					case kTest_pValue1:
						if (pblock()->GetFloat(kTest_pValue1,curTime) > pblock()->GetFloat(kTest_pValue2,curTime))
							pblock()->SetValue(kTest_pValue2, curTime, pblock()->GetFloat(kTest_pValue1,curTime));
						break;
					case kTest_wValue1:
						if (pblock()->GetFloat(kTest_wValue1,curTime) > pblock()->GetFloat(kTest_wValue2,curTime))
							pblock()->SetValue(kTest_wValue2, curTime, pblock()->GetFloat(kTest_wValue1,curTime));
						break;
					case kTest_aValue1:
						if (pblock()->GetFloat(kTest_aValue1,curTime) > pblock()->GetFloat(kTest_aValue2,curTime))
							pblock()->SetValue(kTest_aValue2, curTime, pblock()->GetFloat(kTest_aValue1,curTime));
						break;
					case kTest_iValue2:
						if (pblock()->GetInt(kTest_iValue1,curTime) > pblock()->GetInt(kTest_iValue2,curTime))
							pblock()->SetValue(kTest_iValue1, curTime, pblock()->GetInt(kTest_iValue2,curTime));
						break;
					case kTest_tValue2:
						if (pblock()->GetTimeValue(kTest_tValue1,curTime) > pblock()->GetTimeValue(kTest_tValue2,curTime))
							pblock()->SetValue(kTest_tValue1, curTime, pblock()->GetTimeValue(kTest_tValue2,curTime));
						break;
					case kTest_fValue2:
						if (pblock()->GetFloat(kTest_fValue1,curTime) > pblock()->GetFloat(kTest_fValue2,curTime))
							pblock()->SetValue(kTest_fValue1, curTime, pblock()->GetFloat(kTest_fValue2,curTime));
						break;
					case kTest_pValue2:
						if (pblock()->GetFloat(kTest_pValue1,curTime) > pblock()->GetFloat(kTest_pValue2,curTime))
							pblock()->SetValue(kTest_pValue1, curTime, pblock()->GetFloat(kTest_pValue2,curTime));
						break;
					case kTest_wValue2:
						if (pblock()->GetFloat(kTest_wValue1,curTime) > pblock()->GetFloat(kTest_wValue2,curTime))
							pblock()->SetValue(kTest_wValue1, curTime, pblock()->GetFloat(kTest_wValue2,curTime));
						break;
					case kTest_aValue2:
						if (pblock()->GetFloat(kTest_aValue1,curTime) > pblock()->GetFloat(kTest_aValue2,curTime))
							pblock()->SetValue(kTest_aValue1, curTime, pblock()->GetFloat(kTest_aValue2,curTime));
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

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
RefTargetHandle PATest::Clone(RemapDir &remap)
{
	PATest* newTest = new PATest();
	newTest->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newTest, remap);
	return newTest;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From BaseObject									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
TCHAR* PATest::GetObjectName()
{
	return GetString(IDS_TEST_OBJECT_NAME);
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFAction									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
const ParticleChannelMask& PATest::ChannelsUsed(const Interval& time) const
{
								//  read									  &	write channels
	static ParticleChannelMask mask(PCU_New|PCU_ID|PCU_Time|PCU_BirthTime, 0);
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PATest::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_TEST_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PATest::GetTruePViewIcon()
{
	if (trueIcon() == NULL)
		_trueIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_TEST_TRUEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return trueIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PATest::GetFalsePViewIcon()
{
	if (falseIcon() == NULL)
		_falseIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_TEST_FALSEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return falseIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PATest::HasDynamicName(TSTR& nameSuffix)
{
	nameSuffix = pblock()->GetStr(kTest_channelName, 0);
	TSTR buf;
	int channelType = pblock()->GetInt(kTest_channelType, 0);
	bool isLessType = (pblock()->GetInt(kTest_conditionType, 0) == kTest_conditionType_isLess);
	if (channelType == kTest_channelType_boolean) {
		buf = isLessType ? _T("==false") : _T("==true");
		nameSuffix += buf;
		return true;
	}
	bool useSecond = (pblock()->GetInt(kTest_useValue2, 0) != 0);
	buf = isLessType ? _T("<") : _T(">");
	nameSuffix += buf;
	Control* ctrl = NULL;
	bool isAnimated = false;
	TCHAR charBuf[64];
	int spinnerPrec = GetSpinnerPrecision();
	switch(channelType) {
	case kTest_channelType_time:
		ctrl = pblock()->GetController(kTest_tValue1);
		isAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		if (isAnimated) buf = _T("A");
		else TimeToString(pblock()->GetTimeValue(kTest_tValue1,0), buf);
		break;
	case kTest_channelType_integer:
		ctrl = pblock()->GetController(kTest_iValue1);
		isAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		if (isAnimated) buf = _T("A");
		else {
			sprintf(charBuf,"%d",pblock()->GetInt(kTest_iValue1,0));
			buf = charBuf;
		}
		break;
	case kTest_channelType_float:
		ctrl = pblock()->GetController(kTest_fValue1);
		isAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		if (isAnimated) buf = _T("A");
		else buf = FormatFloatValue(pblock()->GetFloat(kTest_fValue1,0), spinnerPrec);
		break;
	case kTest_channelType_percent:
		ctrl = pblock()->GetController(kTest_pValue1);
		isAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		if (isAnimated) buf = _T("A");
		else buf = FormatFloatValue(100.0f*pblock()->GetFloat(kTest_pValue1,0), spinnerPrec);
		buf += _T("%");
		break;
	case kTest_channelType_worldFloat:
		ctrl = pblock()->GetController(kTest_wValue1);
		isAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		if (isAnimated) buf = _T("A");
		else buf = FormatUniverseValue(pblock()->GetFloat(kTest_wValue1,0));
		break;
	case kTest_channelType_angle:
		ctrl = pblock()->GetController(kTest_aValue1);
		isAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		if (isAnimated) buf = _T("A");
		else buf = FormatFloatValue(RAD_TO_DEG*pblock()->GetFloat(kTest_aValue1,0), spinnerPrec);
		break;
	}
	nameSuffix += buf;

	if (useSecond) {
		buf = isLessType ? _T(" or >") : _T(" and <");
		nameSuffix += buf;
		switch(channelType) {
		case kTest_channelType_time:
			ctrl = pblock()->GetController(kTest_tValue2);
			isAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isAnimated) buf = _T("A");
			else TimeToString(pblock()->GetTimeValue(kTest_tValue2,0), buf);
			break;
		case kTest_channelType_integer:
			ctrl = pblock()->GetController(kTest_iValue2);
			isAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isAnimated) buf = _T("A");
			else {
				sprintf(charBuf,"%d",pblock()->GetInt(kTest_iValue2,0));
				buf = charBuf;
			}
			break;
		case kTest_channelType_float:
			ctrl = pblock()->GetController(kTest_fValue2);
			isAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isAnimated) buf = _T("A");
			else buf = FormatFloatValue(pblock()->GetFloat(kTest_fValue2,0), spinnerPrec);
			break;
		case kTest_channelType_percent:
			ctrl = pblock()->GetController(kTest_pValue2);
			isAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isAnimated) buf = _T("A");
			else buf = FormatFloatValue(100.0f*pblock()->GetFloat(kTest_pValue2,0), spinnerPrec);
			buf += _T("%");
			break;
		case kTest_channelType_worldFloat:
			ctrl = pblock()->GetController(kTest_wValue2);
			isAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isAnimated) buf = _T("A");
			else buf = FormatUniverseValue(pblock()->GetFloat(kTest_wValue2,0));
			break;
		case kTest_channelType_angle:
			ctrl = pblock()->GetController(kTest_aValue2);
			isAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
			if (isAnimated) buf = _T("A");
			else buf = FormatFloatValue(RAD_TO_DEG*pblock()->GetFloat(kTest_aValue2,0), spinnerPrec);
			break;
		}
		nameSuffix += buf;
	}

	return true;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFTest									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
bool PATest::Proceed(IObject* pCont, 
							PreciseTimeValue timeStart, 
							PreciseTimeValue& timeEnd, 
							Object* pSystem, 
							INode* pNode, 
							INode* actionNode, 
							IPFIntegrator* integrator, 
							BitArray& testResult, 
							Tab<float>& testTime)
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

	int channelType = pblock()->GetInt(kTest_channelType, 0);
	Class_ID channelClassID;
	ParamID getData1From, getData2From;
	switch (channelType) {
	case kTest_channelType_boolean:
		channelClassID = ParticleChannelBool_Class_ID;
		break;
	case kTest_channelType_time:
		channelClassID = ParticleChannelPTV_Class_ID;
		break;
	case kTest_channelType_integer:
		channelClassID = ParticleChannelInt_Class_ID;
		break;
	case kTest_channelType_float:
		getData1From = kTest_fValue1;
		getData2From = kTest_fValue2;
		channelClassID = ParticleChannelFloat_Class_ID;
		break;
	case kTest_channelType_worldFloat:
		getData1From = kTest_wValue1;
		getData2From = kTest_wValue2;
		channelClassID = ParticleChannelFloat_Class_ID;
		break;
	case kTest_channelType_percent:
		getData1From = kTest_pValue1;
		getData2From = kTest_pValue2;
		channelClassID = ParticleChannelFloat_Class_ID;
		break;
	case kTest_channelType_angle:
		getData1From = kTest_aValue1;
		getData2From = kTest_aValue2;
		channelClassID = ParticleChannelFloat_Class_ID;
		break;
	}
	int subChannelIndex = -1;
	TCHAR* dataChannelName = pblock()->GetStr(kTest_channelName, 0);
	IObject* dataChannel = chMult->GetSubChannel(dataChannelName, channelClassID, subChannelIndex);
	if (dataChannel == NULL) return false; // no data channel to compare with

	bool isLessThan = (pblock()->GetInt(kTest_conditionType, timeStart.TimeValue()) == kTest_conditionType_isLess);
	bool useSecond = (pblock()->GetInt(kTest_useValue2, timeStart.TimeValue()) != 0);

	IParticleChannelBoolR* chDataBool = NULL;
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

	// test all particles
	PreciseTimeValue curParticleValue;
	testResult.SetSize(count);
	testResult.ClearAll();
	testTime.SetCount(count);
	for(i=0; i<count; i++) testTime[i] = 0.0f;

	switch (channelType) {
	case kTest_channelType_boolean:
		chDataBool = (IParticleChannelBoolR*)dataChannel->GetInterface(PARTICLECHANNELBOOLR_INTERFACE);
		if (chDataBool == NULL) return false;
		for(i=0; i<count; i++) {
			if (isLessThan) {
				if (chDataBool->GetValue(i) == false) testResult.Set(i);
			} else {
				if (chDataBool->GetValue(i) == true) testResult.Set(i);
			}
		}
		break;
	case kTest_channelType_time:
		chDataTime = (IParticleChannelPTVR*)dataChannel->GetInterface(PARTICLECHANNELPTVR_INTERFACE);
		if (chDataTime == NULL) return false;
		ctrl = pblock()->GetController(kTest_tValue1);
		isValue1Animated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		ctrl = pblock()->GetController(kTest_tValue2);
		isValue2Animated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		tValue1 = pblock()->GetTimeValue(kTest_tValue1, 0);
		tValue2 = pblock()->GetTimeValue(kTest_tValue2, 0);
		for(i=0; i<count; i++) {
			curTValue = chDataTime->GetValue(i);
			if (useSecond) {
				if (isValue1Animated || isValue2Animated) curLocalTime = (chTime->GetValue(i)).TimeValue();
				if (isLessThan) {
					if (isValue1Animated) {
						if (isValue2Animated) {
							resVal = (curTValue < PreciseTimeValue(pblock()->GetTimeValue(kTest_tValue1, curLocalTime))) ||
									(curTValue > PreciseTimeValue(pblock()->GetTimeValue(kTest_tValue2, curLocalTime)));
						} else {
							resVal = (curTValue < PreciseTimeValue(pblock()->GetTimeValue(kTest_tValue1, curLocalTime))) ||
									(curTValue > PreciseTimeValue(tValue2));
						}
					} else {
						if (isValue2Animated) {
							resVal = (curTValue < PreciseTimeValue(tValue1)) ||
									(curTValue > PreciseTimeValue(pblock()->GetTimeValue(kTest_tValue2, curLocalTime)));
						} else {
							resVal = (curTValue < PreciseTimeValue(tValue1)) ||	(curTValue > PreciseTimeValue(tValue2));
						} 
					}
				} else {
					if (isValue1Animated) {
						if (isValue2Animated) {
							resVal = (curTValue > PreciseTimeValue(pblock()->GetTimeValue(kTest_tValue1, curLocalTime))) &&
									(curTValue < PreciseTimeValue(pblock()->GetTimeValue(kTest_tValue2, curLocalTime)));
						} else {
							resVal = (curTValue > PreciseTimeValue(pblock()->GetTimeValue(kTest_tValue1, curLocalTime))) &&
									(curTValue < PreciseTimeValue(tValue2));
						}
					} else {
						if (isValue2Animated) {
							resVal = (curTValue > PreciseTimeValue(tValue1)) &&
									(curTValue < PreciseTimeValue(pblock()->GetTimeValue(kTest_tValue2, curLocalTime)));
						} else {
							resVal = (curTValue > PreciseTimeValue(tValue1)) &&	(curTValue < PreciseTimeValue(tValue2));
						} 
					}
				}
			} else {
				if (isValue1Animated) curLocalTime = (chTime->GetValue(i)).TimeValue();
				if (isLessThan) {
					if (isValue1Animated) {
						resVal = ((curTValue < PreciseTimeValue(pblock()->GetTimeValue(kTest_tValue1, curLocalTime))) != 0);
					} else {
						resVal = ((curTValue < PreciseTimeValue(tValue1)) != 0);
					}
				} else {
					if (isValue1Animated) {
						resVal = ((curTValue > PreciseTimeValue(pblock()->GetTimeValue(kTest_tValue1, curLocalTime))) != 0);
					} else {
						resVal = ((curTValue > PreciseTimeValue(tValue1)) != 0);
					}
				}
			}
			if (resVal) testResult.Set(i);
		}
		break;
	case kTest_channelType_integer:
		chDataInt = (IParticleChannelIntR*)dataChannel->GetInterface(PARTICLECHANNELINTR_INTERFACE);
		if (chDataInt == NULL) return false;
		ctrl = pblock()->GetController(kTest_iValue1);
		isValue1Animated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		ctrl = pblock()->GetController(kTest_iValue2);
		isValue2Animated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		iValue1 = pblock()->GetInt(kTest_iValue1, 0);
		iValue2 = pblock()->GetInt(kTest_iValue2, 0);
		for(i=0; i<count; i++) {
			curIValue = chDataInt->GetValue(i);
			if (useSecond) {
				if (isValue1Animated || isValue2Animated) curLocalTime = (chTime->GetValue(i)).TimeValue();
				if (isLessThan) {
					if (isValue1Animated) {
						if (isValue2Animated) {
							resVal = (curIValue < pblock()->GetInt(kTest_iValue1, curLocalTime)) ||
									(curIValue > pblock()->GetInt(kTest_iValue2, curLocalTime));
						} else {
							resVal = (curIValue < pblock()->GetInt(kTest_iValue1, curLocalTime)) ||
									(curIValue > iValue2);
						}
					} else {
						if (isValue2Animated) {
							resVal = (curIValue < iValue1) ||
									(curIValue > pblock()->GetInt(kTest_iValue2, curLocalTime));
						} else {
							resVal = (curIValue < iValue1) || (curIValue > iValue2);
						} 
					}
				} else {
					if (isValue1Animated) {
						if (isValue2Animated) {
							resVal = (curIValue > pblock()->GetInt(kTest_iValue1, curLocalTime)) &&
									(curIValue < pblock()->GetInt(kTest_iValue2, curLocalTime));
						} else {
							resVal = (curIValue > pblock()->GetInt(kTest_iValue1, curLocalTime)) &&
									(curIValue < iValue2);
						}
					} else {
						if (isValue2Animated) {
							resVal = (curIValue > iValue1) &&
									(curIValue < pblock()->GetInt(kTest_iValue2, curLocalTime));
						} else {
							resVal = (curIValue > iValue1) &&	(curIValue < iValue2);
						} 
					}
				}
			} else {
				if (isValue1Animated) curLocalTime = (chTime->GetValue(i)).TimeValue();
				if (isLessThan) {
					if (isValue1Animated) {
						resVal = (curIValue < pblock()->GetInt(kTest_iValue1, curLocalTime));
					} else {
						resVal = (curIValue < iValue1);
					}
				} else {
					if (isValue1Animated) {
						resVal = (curIValue > pblock()->GetInt(kTest_iValue1, curLocalTime));
					} else {
						resVal = (curIValue > iValue1);
					}
				}
			}
			if (resVal) testResult.Set(i);
		}
		break;
	case kTest_channelType_float:
	case kTest_channelType_worldFloat:
	case kTest_channelType_percent:
	case kTest_channelType_angle:
		chDataFloat = (IParticleChannelFloatR*)dataChannel->GetInterface(PARTICLECHANNELFLOATR_INTERFACE);
		if (chDataFloat == NULL) return false;
		ctrl = pblock()->GetController(getData1From);
		isValue1Animated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		ctrl = pblock()->GetController(getData2From);
		isValue2Animated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
		fValue1 = pblock()->GetFloat(getData1From, 0);
		fValue2 = pblock()->GetFloat(getData2From, 0);
		for(i=0; i<count; i++) {
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
			if (resVal) testResult.Set(i);
		}
		break;
	}

	return true;
}

ClassDesc* PATest::GetClassDesc() const
{
	return GetPATestDesc();
}



} // end of namespace PartASM