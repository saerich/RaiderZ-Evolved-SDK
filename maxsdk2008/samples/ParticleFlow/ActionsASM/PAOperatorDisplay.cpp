/**********************************************************************
 *<
	FILE: PAOperatorDisplay.h

	DESCRIPTION: Viewport/Render Operator implementation
				 The Operator is used to define appearance of the particles
				 in the current Event (or globally if it's a global
				 Operator) for viewports

	CREATED FOR: Oleg Bayborodin

	HISTORY: created 05-17-03

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#include "StdAfx.h"
#include "resource.h"
#include "iparamm2.h"

#include "PartASM_SysUtil.h"
#include "PartASM_GlobalFunctions.h"
#include "PartASM_GlobalVariables.h"

#include "PAOperatorDisplay.h"

#include "PAOperatorDisplay_ParamBlock.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IParticleGroup.h"
#include "PFMessages.h"
#include "IPViewManager.h"
#include "IParticleChannelMult.h"

namespace PartASM {

// Display operator creates a particle channel to store a successive order number
// when a particle appeares in the event. The number is used to determine if
// the particle is visible given the current visibility percent
#define PARTICLECHANNELVISIBLER_INTERFACE Interface_ID(0x2de61303, 0x1eb34500) 
#define PARTICLECHANNELVISIBLEW_INTERFACE Interface_ID(0x2de61303, 0x1eb34501) 
#define GetParticleChannelVisibleRInterface(obj) ((IParticleChannelIntR*)obj->GetInterface(PARTICLECHANNELVISIBLER_INTERFACE)) 
#define GetParticleChannelVisibleWInterface(obj) ((IParticleChannelIntW*)obj->GetInterface(PARTICLECHANNELVISIBLEW_INTERFACE)) 

DWORD GetNewObjectColor();

// static members
Object*				PAOperatorDisplay::m_editOb	 = NULL;
IObjParam*			PAOperatorDisplay::m_ip      = NULL;

// constructors/destructors
PAOperatorDisplay::PAOperatorDisplay()
{ 
	RegisterParticleFlowNotification();
	_pblock() = NULL;
	GetClassDesc()->MakeAutoParamBlocks(this); 
	_activeIcon() = _inactiveIcon() = NULL;
	if (pblock() != NULL) {
		// set random initial color
		pblock()->SetValue(kDisplay_color, 0, Color(GetNewObjectColor()));
		for(int i=0; i<kNumGradientValues; i++) {
			pblock()->SetValue(kDisplay_useValue, 0, kUseGradientValue[i], i);
			pblock()->SetValue(kDisplay_gradientColor, 0, kGradientColor[i], i);
			pblock()->SetValue(kDisplay_iValue, 0, kIntegerValues[i], i);
			pblock()->SetValue(kDisplay_tValue, 0, kTimeValues[i], i);
			pblock()->SetValue(kDisplay_fValue, 0, kFloatValues[i], i);
			pblock()->SetValue(kDisplay_pValue, 0, kPercentValues[i], i);
			pblock()->SetValue(kDisplay_wValue, 0, kWorldValues[i], i);
		}
		for(int i=0; i<kNumRGBValues; i++) {
			pblock()->SetValue(kDisplay_RGBMinF, 0, kRGBFloatMinValues[i], i);
			pblock()->SetValue(kDisplay_RGBMaxF, 0, kRGBFloatMaxValues[i], i);
			pblock()->SetValue(kDisplay_RGBMinP, 0, kRGBPercentMinValues[i], i);
			pblock()->SetValue(kDisplay_RGBMaxP, 0, kRGBPercentMaxValues[i], i);
			pblock()->SetValue(kDisplay_RGBMinW, 0, kRGBWorldMinValues[i], i);
			pblock()->SetValue(kDisplay_RGBMaxW, 0, kRGBWorldMaxValues[i], i);
		}
	}
}

PAOperatorDisplay::~PAOperatorDisplay()
{
	int wasHolding = theHold.Holding();
	if (wasHolding) theHold.Suspend();
	DeleteAllRefsFromMe();
	if (wasHolding) theHold.Resume();
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From InterfaceServer									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BaseInterface* PAOperatorDisplay::GetInterface(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return (IPFAction*)this;
	if (id == PFOPERATOR_INTERFACE) return (IPFOperator*)this;
	if (id == PFVIEWPORT_INTERFACE) return (IPFViewport*)this;
	if (id == PVIEWITEM_INTERFACE) return (IPViewItem*)this;
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From FPMixinInterface							 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc* PAOperatorDisplay::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &display_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &display_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &display_PViewItem_FPInterfaceDesc;
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From Animatable									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
void PAOperatorDisplay::DeleteThis()
{
	delete this;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PAOperatorDisplay::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_DISPLAY_CLASS_NAME);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
Class_ID PAOperatorDisplay::ClassID()
{
	return PAOperatorDisplay_Class_ID;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PAOperatorDisplay::BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev)
{
	_ip() = ip; _editOb() = this;
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PAOperatorDisplay::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next)
{
	_ip() = NULL; _editOb() = NULL;
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
Animatable* PAOperatorDisplay::SubAnim(int i)
{
	switch(i) {
	case 0: return _pblock();
	}
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
TSTR PAOperatorDisplay::SubAnimName(int i)
{
	return PartASM::GetString(IDS_PARAMETERS);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
ParamDimension* PAOperatorDisplay::GetParamDimension(int i)
{
	return _pblock()->GetParamDimension(i);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
IParamBlock2* PAOperatorDisplay::GetParamBlock(int i)
{
	if (i==0) return _pblock();
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
IParamBlock2* PAOperatorDisplay::GetParamBlockByID(short id)
{
	if (id == 0) return _pblock();
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From ReferenceMaker								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
RefTargetHandle PAOperatorDisplay::GetReference(int i)
{
	if (i==0) return (RefTargetHandle)pblock();
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
void PAOperatorDisplay::SetReference(int i, RefTargetHandle rtarg)
{
	if (i==0) _pblock() = (IParamBlock2*)rtarg;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
RefResult PAOperatorDisplay::NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message)
{
	Color newColor;
	DWORD theColor;

	switch (message) {
		case REFMSG_CHANGE:
			if (pblock() == hTarget) {
				int tabIndex = -1;
				int lastUpdateID = pblock()->LastNotifyParamID(tabIndex);
				switch (lastUpdateID)
				{
				case kDisplay_showParticles:
				case kDisplay_showData:
				case kDisplay_dataChannel:
				case kDisplay_customType:
				case kDisplay_useColorCoding:
				case kDisplay_useValue:
				case kDisplay_type:
					RefreshUI(kDisplay_message_update);
					if (lastUpdateID == kDisplay_type)
						NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					// the break is omitted on purpose (bayboro 11-18-02)
				case kDisplay_visible:
					if (lastUpdateID == kDisplay_visible) recalcVisibility();
					// the break is omitted on purpose (bayboro 11-18-02)
				case kDisplay_iValue:
					if (lastUpdateID == kDisplay_iValue) {
						TimeValue time = GetCOREInterface()->GetTime();
						int curValue = pblock()->GetInt(kDisplay_iValue, time, tabIndex);
						if (tabIndex < kNumGradientValues-1)
							if (curValue > pblock()->GetInt(kDisplay_iValue, time, tabIndex+1))
								pblock()->SetValue(kDisplay_iValue, time, curValue, tabIndex+1);
						if (tabIndex > 0)
							if (curValue < pblock()->GetInt(kDisplay_iValue, time, tabIndex-1))
								pblock()->SetValue(kDisplay_iValue, time, curValue, tabIndex-1);
					}
					// the break is omitted on purpose (bayboro 11-18-02)
				case kDisplay_fValue:
				case kDisplay_pValue:
				case kDisplay_wValue:
					if ((lastUpdateID == kDisplay_fValue) || (lastUpdateID == kDisplay_pValue) || (lastUpdateID == kDisplay_wValue)) {
						TimeValue time = GetCOREInterface()->GetTime();
						float curValue = pblock()->GetFloat(lastUpdateID, time, tabIndex);
						if (tabIndex < kNumGradientValues-1)
							if (curValue > pblock()->GetFloat(lastUpdateID, time, tabIndex+1))
								pblock()->SetValue(lastUpdateID, time, curValue, tabIndex+1);
						if (tabIndex > 0)
							if (curValue < pblock()->GetFloat(lastUpdateID, time, tabIndex-1))
								pblock()->SetValue(lastUpdateID, time, curValue, tabIndex-1);
					}
					// the break is omitted on purpose (bayboro 11-18-02)
				case kDisplay_tValue:
					if (lastUpdateID == kDisplay_tValue) {
						TimeValue time = GetCOREInterface()->GetTime();
						TimeValue curValue = pblock()->GetTimeValue(kDisplay_tValue, time, tabIndex);
						if (tabIndex < kNumGradientValues-1)
							if (curValue > pblock()->GetTimeValue(kDisplay_tValue, time, tabIndex+1))
								pblock()->SetValue(kDisplay_tValue, time, curValue, tabIndex+1);
						if (tabIndex > 0)
							if (curValue < pblock()->GetTimeValue(kDisplay_tValue, time, tabIndex-1))
								pblock()->SetValue(kDisplay_tValue, time, curValue, tabIndex-1);
					}
					// the break is omitted on purpose (bayboro 11-18-02)
				case kDisplay_RGBMinF:
				case kDisplay_RGBMinP:
				case kDisplay_RGBMinW:
					if ((lastUpdateID == kDisplay_RGBMinF) || (lastUpdateID == kDisplay_RGBMinP) || (lastUpdateID == kDisplay_RGBMinW)) {
						TimeValue time = GetCOREInterface()->GetTime();
						float curValue = pblock()->GetFloat(lastUpdateID, time, tabIndex);
						if (curValue > pblock()->GetFloat(lastUpdateID+1, time, tabIndex))
							pblock()->SetValue(lastUpdateID+1, time, curValue, tabIndex);
					}
				case kDisplay_RGBMaxF:
				case kDisplay_RGBMaxP:
				case kDisplay_RGBMaxW:
					if ((lastUpdateID == kDisplay_RGBMaxF) || (lastUpdateID == kDisplay_RGBMaxP) || (lastUpdateID == kDisplay_RGBMaxW)) {
						TimeValue time = GetCOREInterface()->GetTime();
						float curValue = pblock()->GetFloat(lastUpdateID, time, tabIndex);
						if (curValue < pblock()->GetFloat(lastUpdateID-1, time, tabIndex))
							pblock()->SetValue(lastUpdateID-1, time, curValue, tabIndex);
					}
					// the break is omitted on purpose (bayboro 11-18-02)
				case kDisplay_color: 
				case kDisplay_showNumbering:
				case kDisplay_selectedOnly:
				case kDisplay_gradientColor:
				case kDisplay_gradientType:
				case kDisplay_RType:
				case kDisplay_GType:
				case kDisplay_BType:
				case kDisplay_speedFactor:
				case kDisplay_speedMode:
				case kDisplay_precision:
					newColor = pblock()->GetColor(kDisplay_color, 0);
					theColor = newColor.toRGB();
					NotifyDependents( FOREVER, (PartID)theColor, kPFMSG_UpdateWireColor );
					if (tabIndex != -1) UpdatePViewUI(lastUpdateID, tabIndex);
					else UpdatePViewUI(lastUpdateID);
					break;
				default:
					UpdatePViewUI(lastUpdateID);
				}
				return REF_STOP;
			}
			break;
		case kDisplay_RefMsg_InitDlg:
			RefreshUI(kDisplay_message_update, (IParamMap2*)partID);
			return REF_STOP;
	}	
	return REF_SUCCEED;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
RefTargetHandle PAOperatorDisplay::Clone(RemapDir &remap)
{
	PAOperatorDisplay* newOp = new PAOperatorDisplay();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
int PAOperatorDisplay::RemapRefOnLoad(int iref)
{
	return iref;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
IOResult PAOperatorDisplay::Save(ISave *isave)
{
	return IO_OK;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
IOResult PAOperatorDisplay::Load(ILoad *iload)
{
	return IO_OK;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From BaseObject									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
TCHAR* PAOperatorDisplay::GetObjectName()
{
	return GetString(IDS_OPERATOR_DISPLAY_OBJECT_NAME);
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFAction									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
bool PAOperatorDisplay::Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes)
{
	_totalParticles(pCont) = 0;
	return true;
}

bool PAOperatorDisplay::Release(IObject* pCont)
{
	return true;
}

const ParticleChannelMask& PAOperatorDisplay::ChannelsUsed(const Interval& time) const
{
								//  read								&	write channels
	static ParticleChannelMask mask(PCU_Amount|PCU_ID|PCU_Position|PCU_Speed|PCU_Shape|PCU_ShapeTexture|PCU_MtlIndex, 0);
	return mask;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFOperator								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
bool PAOperatorDisplay::Proceed(IObject* pCont, 
									 PreciseTimeValue timeStart, 
									 PreciseTimeValue& timeEnd,
									 Object* pSystem,
									 INode* pNode,
									 INode* actionNode,
									 IPFIntegrator* integrator)
{
	// acquire absolutely necessary particle channels
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	int count = chAmount->Count();
	if (count <= 0) return true; // no particles to worry about
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if (chNew == NULL) return false; // can't find newly entered particles for duration calculation
	if (chNew->IsAllOld()) return true; // everything has been initialized
	
	IChannelContainer* chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;
	// first try to get the private channel.
	// If it's not possible then create a new one
	bool initVisible = false;
	IParticleChannelIntR* chVisibleR = (IParticleChannelIntR*)chCont->EnsureInterface(PARTICLECHANNELVISIBLER_INTERFACE,
													ParticleChannelInt_Class_ID,
													true,	PARTICLECHANNELVISIBLER_INTERFACE,
															PARTICLECHANNELVISIBLEW_INTERFACE,
													true, actionNode, (Object*)this, // no transfer & private
													&initVisible);
	IParticleChannelIntW*chVisibleW = (IParticleChannelIntW*)chCont->GetPrivateInterface(PARTICLECHANNELVISIBLEW_INTERFACE, (Object*)this);
	if ((chVisibleR == NULL) || (chVisibleW == NULL)) return false; // can't find/create Visible channel

	bool initEventStart = false;
	IParticleChannelPTVR* chTime = NULL;
	IParticleChannelPTVW* chEventStart = NULL;
	int showData = pblock()->GetInt(kDisplay_showData, timeStart.TimeValue());
	if (showData != 0) {
		int dataChannel = pblock()->GetInt(kDisplay_dataChannel, timeStart.TimeValue());
		if (dataChannel == kDisplay_data_timeInEvent) {
			chTime = GetParticleChannelTimeRInterface(pCont);
			if (chTime == NULL) return false;	// can't find particle times in the container
			chEventStart = (IParticleChannelPTVW*)chCont->EnsureInterface(PARTICLECHANNELEVENTSTARTW_INTERFACE,
																ParticleChannelPTV_Class_ID,
																true, PARTICLECHANNELEVENTSTARTR_INTERFACE,
																PARTICLECHANNELEVENTSTARTW_INTERFACE, false,
																actionNode, NULL, &initEventStart);
			if (chEventStart == NULL) return false; // can't modify event start time
		}
	}

	int i;
	int total = _totalParticles(pCont);
	for(i=0; i<count; i++) {
		if (chNew->IsNew(i)) {
			int visIndex = chVisibleR->GetValue(i);
			if (visIndex == 0) {
				total += 1;
				chVisibleW->SetValue(i, total);
			}
			if (initEventStart) chEventStart->SetValue(i, chTime->GetValue(i));
		}
	}
	_totalParticles(pCont) = total;
	return true;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFViewport								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
bool AssignMeshMaterialIndices(Mesh* mesh, IParticleChannelIntR* chMtlIndex, int particleIndex)
{
	if (chMtlIndex == NULL) return false;
	int mtlIndex = chMtlIndex->GetValue(particleIndex);
	if (mtlIndex >= 0) {
		for(int i=0; i<mesh->getNumFaces(); i++)
			mesh->setFaceMtlIndex(i, mtlIndex);
		return true;
	}
	return false;
}

bool AssignMeshMapping(Mesh* mesh, IParticleChannelMeshMapR* chMapping, int particleIndex)
{
	if (chMapping == NULL) return false;
	bool hasMappingAssigned = false;
	for(int i=0; i<chMapping->GetNumMaps(); i++) {
		if (!chMapping->MapSupport(i)) continue;	
		IParticleChannelMapR* map = chMapping->GetMapReadChannel(i);
		if (map == NULL) continue;
		map->Apply(mesh, particleIndex, i);
		hasMappingAssigned = true;
	}
	return hasMappingAssigned;
}

class ColorCodingData {
public:
	void Init(IObject* pCont, IParamBlock2* pblock, TimeValue time);
	Color GetColor(int particleIndex);

	IParticleChannelPTVR* chTime;
	IParticleChannelPTVR* chBirthTime;
	IParticleChannelPTVR* chDeathTime;
	IParticleChannelPTVR* chEventStart;
	IParticleChannelPTVR* chLifespan;
	IParticleChannelPoint3R* chWorldVector;
	IParticleChannelMeshR* chMesh;
	IParticleChannelMeshMapR* chShapeTexture;
	IParticleChannelMapR* chMap;
	IParticleChannelIntR* chInteger;
	IParticleChannelQuatR* chRotation;
	IParticleChannelPoint3R* chScale;
	IParticleChannelFloatR* chFloat;
	IParticleChannelPoint3R* chVector;
	IParticleChannelBoolR* chSelection;
	IParticleChannelAngAxisR* chSpin;

	int numUseValue;
	int gradientType;
	int dataChannel;
	int vectorComponent;
	float scaleFactor;
	bool useValue[kNumGradientValues];
	Color gradientColor[kNumGradientValues];
	int iValue[kNumGradientValues];
	float fValue[kNumGradientValues];
	PreciseTimeValue tValue[kNumGradientValues];
	float pValue[kNumGradientValues];
	float wValue[kNumGradientValues];
	int RType, GType, BType;
	float fRGBMin[kNumRGBValues], fRGBMax[kNumRGBValues], pRGBMin[kNumRGBValues], pRGBMax[kNumRGBValues], wRGBMin[kNumRGBValues], wRGBMax[kNumRGBValues];
};

void ColorCodingData::Init(IObject* pCont, IParamBlock2* pblock, TimeValue time)
{
	chTime = chBirthTime = chDeathTime = chEventStart = chLifespan = NULL;
	chWorldVector = chScale = chVector = NULL;
	chMesh = NULL;
	chShapeTexture = NULL;
	chMap = NULL;
	chInteger = NULL;
	chRotation = NULL;
	chFloat = NULL;
	chSelection = NULL;
	chSpin = NULL;

	vectorComponent = 0;
	scaleFactor = 1.0f;
	dataChannel = pblock->GetInt(kDisplay_dataChannel, time);

	switch (dataChannel) {
	case kDisplay_data_custom: 
		{
			IParticleChannelMult* chMult = GetParticleChannelMultInterface(pCont);
			if (chMult != NULL) {
				TCHAR* channelName = pblock->GetStr(kDisplay_channelName, 0);
				int customType = pblock->GetInt(kDisplay_customType, 0);
				IObject* customChannel = NULL;
				int subChannelIndex;
				switch(customType) {
				case kDisplay_customType_bool:
					customChannel = chMult->GetSubChannel(channelName, ParticleChannelBool_Class_ID, subChannelIndex);
					if (customChannel != NULL) {
						chSelection = (IParticleChannelBoolR*)(customChannel->GetInterface(PARTICLECHANNELBOOLR_INTERFACE));
						dataChannel = kDisplay_data_selected;
					}
					break;
				case kDisplay_customType_time:
					customChannel = chMult->GetSubChannel(channelName, ParticleChannelPTV_Class_ID, subChannelIndex);
					if (customChannel != NULL) {
						chBirthTime = (IParticleChannelPTVR*)(customChannel->GetInterface(PARTICLECHANNELPTVR_INTERFACE));
						dataChannel = kDisplay_data_birthTime;
					}
					break;
				case kDisplay_customType_integer:
					customChannel = chMult->GetSubChannel(channelName, ParticleChannelInt_Class_ID, subChannelIndex);
					if (customChannel != NULL) {
						chInteger = (IParticleChannelIntR*)(customChannel->GetInterface(PARTICLECHANNELINTR_INTERFACE));
						dataChannel = kDisplay_data_scriptInteger;
					}
					break;
				case kDisplay_customType_float:
					customChannel = chMult->GetSubChannel(channelName, ParticleChannelFloat_Class_ID, subChannelIndex);
					if (customChannel != NULL) {
						chFloat = (IParticleChannelFloatR*)(customChannel->GetInterface(PARTICLECHANNELFLOATR_INTERFACE));
						dataChannel = kDisplay_data_scriptFloat;
					}
					break;
				case kDisplay_customType_worldFloat: // special cases for redirection
				case kDisplay_customType_percent:
				case kDisplay_customType_angle:
					customChannel = chMult->GetSubChannel(channelName, ParticleChannelFloat_Class_ID, subChannelIndex);
					if (customChannel != NULL) {
						chFloat = (IParticleChannelFloatR*)(customChannel->GetInterface(PARTICLECHANNELFLOATR_INTERFACE));
						switch(customType) {
						case kDisplay_customType_worldFloat:
							dataChannel = kDisplay_data_customWorldFloat;
							break;
						case kDisplay_customType_percent:
							dataChannel = kDisplay_data_customPercent;
							break;
						case kDisplay_customType_angle:
							dataChannel = kDisplay_data_customAngle;
							break;
						}
					}
					break;
				case kDisplay_customType_floatVector:
					customChannel = chMult->GetSubChannel(channelName, ParticleChannelPoint3_Class_ID, subChannelIndex);
					if (customChannel != NULL) {
						chVector = (IParticleChannelPoint3R*)(customChannel->GetInterface(PARTICLECHANNELPOINT3R_INTERFACE));
						dataChannel = kDisplay_data_scriptVector;
					}
					break;
				case kDisplay_customType_worldVector:
					customChannel = chMult->GetSubChannel(channelName, ParticleChannelPoint3_Class_ID, subChannelIndex);
					if (customChannel != NULL) {
						chWorldVector = (IParticleChannelPoint3R*)(customChannel->GetInterface(PARTICLECHANNELPOINT3R_INTERFACE));
						dataChannel = kDisplay_data_position;
					}
					break;
				case kDisplay_customType_percentVector:
					customChannel = chMult->GetSubChannel(channelName, ParticleChannelPoint3_Class_ID, subChannelIndex);
					if (customChannel != NULL) {
						chScale = (IParticleChannelPoint3R*)(customChannel->GetInterface(PARTICLECHANNELPOINT3R_INTERFACE));
						dataChannel = kDisplay_data_scale;
					}
					break;
				case kDisplay_customType_angleVector:  // special cases for redirection
					customChannel = chMult->GetSubChannel(channelName, ParticleChannelPoint3_Class_ID, subChannelIndex);
					if (customChannel != NULL) {
						chVector = (IParticleChannelPoint3R*)(customChannel->GetInterface(PARTICLECHANNELPOINT3R_INTERFACE));
						dataChannel = kDisplay_data_customAngleVector;
					}
					break;
				}
			}
		}
		break;
	case kDisplay_data_birthTime:
		chBirthTime = GetParticleChannelBirthTimeRInterface(pCont);
		break;
	case kDisplay_data_age:
		chTime = GetParticleChannelTimeRInterface(pCont);
		chBirthTime = GetParticleChannelBirthTimeRInterface(pCont);
		break;
	case kDisplay_data_deathTime:
	case kDisplay_data_lifespan:
		chTime = GetParticleChannelTimeRInterface(pCont);
		chDeathTime = GetParticleChannelDeathTimeRInterface(pCont);
		chLifespan = GetParticleChannelLifespanRInterface(pCont);
		break;
	case kDisplay_data_timeInEvent:
		chTime = GetParticleChannelTimeRInterface(pCont);
		chEventStart = GetParticleChannelEventStartRInterface(pCont);
		break;
	case kDisplay_data_position:
	case kDisplay_data_positionX:
	case kDisplay_data_positionY:
	case kDisplay_data_positionZ:
		chWorldVector = GetParticleChannelPositionRInterface(pCont);
		vectorComponent = dataChannel - kDisplay_data_positionX;
		break;
	case kDisplay_data_speedVector:
	case kDisplay_data_speedX:
	case kDisplay_data_speedY:
	case kDisplay_data_speedZ:
	case kDisplay_data_speedMagnitude:
		chWorldVector = GetParticleChannelSpeedRInterface(pCont);
		vectorComponent = dataChannel - kDisplay_data_speedX;
		scaleFactor = float(TIME_TICKSPERSEC);
		break;
	case kDisplay_data_mapChannel1:
	case kDisplay_data_mapChannel2:
	case kDisplay_data_mapChannel3:
	case kDisplay_data_mapChannel4:
	case kDisplay_data_mapChannel5:
		chShapeTexture = GetParticleChannelShapeTextureRInterface(pCont);
		if (chShapeTexture != NULL) {
			int mapChannel = 1 + dataChannel - kDisplay_data_mapChannel1;
			if (chShapeTexture->MapSupport(mapChannel))
				chMap = chShapeTexture->GetMapReadChannel(mapChannel);
		}
		break;
	case kDisplay_data_mtlIndex:
		chInteger = GetParticleChannelMtlIndexRInterface(pCont);
		break;
	case kDisplay_data_scriptInteger:
		chInteger = GetParticleChannelMXSIntegerRInterface(pCont);
		break;
	case kDisplay_data_rotationAxis:
	case kDisplay_data_rotationAngle:
	case kDisplay_data_euclideRotation:
		chRotation = GetParticleChannelOrientationRInterface(pCont);
		break;
	case kDisplay_data_size:
	case kDisplay_data_sizeX:
	case kDisplay_data_sizeY:
	case kDisplay_data_sizeZ:
		chMesh = GetParticleChannelShapeRInterface(pCont);
		// the break is omitted on purpose
	case kDisplay_data_scale:
	case kDisplay_data_scaleX:
	case kDisplay_data_scaleY:
	case kDisplay_data_scaleZ:
		chScale = GetParticleChannelScaleRInterface(pCont);
		break;
	case kDisplay_data_scriptFloat:
		chFloat = GetParticleChannelMXSFloatRInterface(pCont);
		break;
	case kDisplay_data_scriptVector:
		chVector = GetParticleChannelMXSVectorRInterface(pCont);
		break;
	case kDisplay_data_selected:
		chSelection = GetParticleChannelSelectionRInterface(pCont);
		break;
	case kDisplay_data_spinAxis:
	case kDisplay_data_spinRate:
		chSpin = GetParticleChannelSpinRInterface(pCont);
		break;
	}			

	gradientType = pblock->GetInt(kDisplay_gradientType, time);
	for(int i=0; i<kNumGradientValues; i++)
		useValue[i] = (pblock->GetInt(kDisplay_useValue, time, i) != 0);
	if (dataChannel == kDisplay_data_selected) useValue[0] = useValue[1] = true;

	int index = 0;
	for(int i=0; i<kNumGradientValues; i++) {
		if (!useValue[i]) continue;
		gradientColor[index] = pblock->GetColor(kDisplay_gradientColor, time, i);
		iValue[index] = pblock->GetInt(kDisplay_iValue, time, i);
		if (index) if (iValue[index] < iValue[index-1]) iValue[index] = iValue[index-1];
		fValue[index] = pblock->GetFloat(kDisplay_fValue, time, i);
		if (index) if (fValue[index] < fValue[index-1]) fValue[index] = fValue[index-1];
		tValue[index] = PreciseTimeValue(pblock->GetTimeValue(kDisplay_tValue, time, i));
		if (index) if (tValue[index] < tValue[index-1]) tValue[index] = tValue[index-1];
		pValue[index] = pblock->GetFloat(kDisplay_pValue, time, i);
		if (index) if (pValue[index] < pValue[index-1]) pValue[index] = pValue[index-1];
		wValue[index] = pblock->GetFloat(kDisplay_wValue, time, i);
		if (index) if (wValue[index] < wValue[index-1]) wValue[index] = wValue[index-1];
		index++;
	}
	numUseValue = index;

	RType = pblock->GetInt(kDisplay_RType, time);
	GType = pblock->GetInt(kDisplay_GType, time);
	BType = pblock->GetInt(kDisplay_BType, time);
	for(int i=0; i<kNumRGBValues; i++) {
		fRGBMin[i] = pblock->GetFloat(kDisplay_RGBMinF, time, i);
		fRGBMax[i] = pblock->GetFloat(kDisplay_RGBMaxF, time, i);
		if (fRGBMin[i] > fRGBMax[i]) fRGBMin[i] = fRGBMax[i];
		pRGBMin[i] = pblock->GetFloat(kDisplay_RGBMinP, time, i);
		pRGBMax[i] = pblock->GetFloat(kDisplay_RGBMaxP, time, i);
		if (pRGBMin[i] > pRGBMax[i]) pRGBMin[i] = pRGBMax[i];
		wRGBMin[i] = pblock->GetFloat(kDisplay_RGBMinW, time, i);
		wRGBMax[i] = pblock->GetFloat(kDisplay_RGBMaxW, time, i);
		if (wRGBMin[i] > wRGBMax[i]) wRGBMin[i] = wRGBMax[i];
	}
}

Color ColorCodingData::GetColor(int index)
{
	Color resCol(0.0f, 0.0f, 0.0f);

	int valueType = -1;
	bool boolValue;
	PreciseTimeValue timeValue;
	int i, intValue;
	float floatValue;
	Point3 vectorValue;

	switch( dataChannel) {
	case kDisplay_data_birthTime:
		if (chBirthTime != NULL) {
			timeValue = chBirthTime->GetValue(index);
			valueType = kDisplay_valueType_time;
		}
		break;
	case kDisplay_data_age:
		if ((chTime != NULL) && (chBirthTime != NULL)) {
			timeValue = chTime->GetValue(index) - chBirthTime->GetValue(index);
			valueType = kDisplay_valueType_time;
		}
		break;
	case kDisplay_data_deathTime:
		if ((chDeathTime != NULL) || ((chTime != NULL) && (chLifespan != NULL))) {
			if ((chTime != NULL) && (chLifespan != NULL)) {
				timeValue = chTime->GetValue(index) + chLifespan->GetValue(index);
			} else {
				timeValue = chDeathTime->GetValue(index);
			}
			valueType = kDisplay_valueType_time;
		}
		break;
	case kDisplay_data_lifespan:
		if ((chLifespan != NULL) || ((chTime != NULL) && (chDeathTime != NULL))) {
			if (chLifespan != NULL) timeValue = chLifespan->GetValue(index);
			else timeValue = chDeathTime->GetValue(index) - chTime->GetValue(index); 
			valueType = kDisplay_valueType_time;
		}
		break;
	case kDisplay_data_timeInEvent:
		if ((chTime != NULL) && (chEventStart != NULL)) {
			timeValue = chTime->GetValue(index) - chEventStart->GetValue(index); 
			valueType = kDisplay_valueType_time;
		}
		break;
	case kDisplay_data_position:
	case kDisplay_data_speedVector:
		if (chWorldVector != NULL) {
			Point3 vec = chWorldVector->GetValue(index);
			vectorValue = scaleFactor*vec;
			valueType = kDisplay_valueType_worldVector;
		}
		break;
	case kDisplay_data_positionX:
	case kDisplay_data_positionY:
	case kDisplay_data_positionZ:
	case kDisplay_data_speedX:
	case kDisplay_data_speedY:
	case kDisplay_data_speedZ:
	case kDisplay_data_speedMagnitude:
		if (chWorldVector != NULL) {
			Point3 vec = chWorldVector->GetValue(index);
			floatValue = 0.0f;
			if (dataChannel == kDisplay_data_speedMagnitude) floatValue = Length(vec);
			else floatValue = vec[vectorComponent];
			floatValue *= scaleFactor;
			valueType = kDisplay_valueType_worldFloat;
		}
		break;
	case kDisplay_data_mapChannel1:
	case kDisplay_data_mapChannel2:
	case kDisplay_data_mapChannel3:
	case kDisplay_data_mapChannel4:
	case kDisplay_data_mapChannel5:
		if (chMap != NULL) {
			if ((chMap->GetUVVert(index))->Count() == 1) {
				vectorValue = (*(chMap->GetUVVert(index)))[0];
				valueType = kDisplay_valueType_floatVector;
			}
		}
		break;
	case kDisplay_data_mtlIndex:
	case kDisplay_data_scriptInteger:
		if (chInteger != NULL) {
			intValue = chInteger->GetValue(index);
			if(dataChannel == kDisplay_data_mtlIndex) intValue++; // 1-based mtlIndices for show
			valueType = kDisplay_valueType_integer;
		}
		break;
	case kDisplay_data_rotationAxis:
	case kDisplay_data_rotationAngle:
		if (chRotation != NULL) {
			AngAxisFromQ(chRotation->GetValue(index), &floatValue, vectorValue);
			if (dataChannel == kDisplay_data_rotationAxis) {
				valueType = kDisplay_valueType_floatVector;
			} else {
				floatValue *= RAD_TO_DEG;
				valueType = kDisplay_valueType_float;
			}
		}
		break;
	case kDisplay_data_euclideRotation:
		if (chRotation != NULL) {
			Quat q = chRotation->GetValue(index);
			QuatToEuler(q, vectorValue);
			for(int k=0; k<3; k++) vectorValue[k] *= RAD_TO_DEG;
			valueType = kDisplay_valueType_floatVector;
		}
		break;
	case kDisplay_data_scale:
	case kDisplay_data_scaleX:
	case kDisplay_data_scaleY:
	case kDisplay_data_scaleZ:
		if (chScale != NULL) {
			vectorValue = chScale->GetValue(index);
			if (dataChannel != kDisplay_data_scale) {
				floatValue = vectorValue[dataChannel-kDisplay_data_scaleX];
				valueType = kDisplay_valueType_percent;
			} else {
				valueType = kDisplay_valueType_percentVector;
			}
		}
		break;
	case kDisplay_data_size:
	case kDisplay_data_sizeX:
	case kDisplay_data_sizeY:
	case kDisplay_data_sizeZ:
		if (chMesh != NULL) {
			Mesh* curShape = const_cast <Mesh*>(chMesh->GetValue(index));
			Box3 bbox;
			bbox.pmin = bbox.pmax = Point3::Origin;
			if (curShape != NULL) bbox = curShape->getBoundingBox();
			Point3 scale = Point3(1.0f, 1.0f, 1.0f);
			if (chScale) scale = chScale->GetValue(index);
			vectorValue = Point3(scale.x*(bbox.pmax.x-bbox.pmin.x), scale.y*(bbox.pmax.y-bbox.pmin.y), scale.z*(bbox.pmax.z-bbox.pmin.z));
			if (dataChannel == kDisplay_data_size) {
				valueType = kDisplay_valueType_worldVector;
			} else {
				floatValue = vectorValue[dataChannel-kDisplay_data_sizeX];
				valueType = kDisplay_valueType_worldFloat;
			}
		}
		break;
	case kDisplay_data_scriptFloat:
		if (chFloat != NULL) {
			floatValue = chFloat->GetValue(index);
			valueType = kDisplay_valueType_float;
		}
		break;
	case kDisplay_data_scriptVector:
		if (chVector != NULL) {
			vectorValue = chVector->GetValue(index);
			valueType = kDisplay_valueType_floatVector;
		}
		break;
	case kDisplay_data_selected:
		if (chSelection != NULL) {
			boolValue = chSelection->GetValue(index);
			valueType = kDisplay_valueType_bool;
		}
		break;
	case kDisplay_data_spinAxis:
	case kDisplay_data_spinRate:
		if (chSpin != NULL) {
			AngAxis aa = chSpin->GetValue(index);
			if (dataChannel == kDisplay_data_spinAxis) {
				vectorValue = aa.axis;
				valueType = kDisplay_valueType_floatVector;
			} else {
				floatValue = aa.angle*TIME_TICKSPERSEC*RAD_TO_DEG;
				valueType = kDisplay_valueType_float;
			}
		}
		break;
	case kDisplay_data_customWorldFloat:
		if (chFloat != NULL) {
			floatValue = chFloat->GetValue(index);
			valueType = kDisplay_valueType_worldFloat;
		}
		break;
	case kDisplay_data_customPercent:
		if (chFloat != NULL) {
			floatValue = chFloat->GetValue(index);
			valueType = kDisplay_valueType_percent;			
		}
		break;
	case kDisplay_data_customAngle:
		if (chFloat != NULL) {
			floatValue = chFloat->GetValue(index)*RAD_TO_DEG;
			valueType = kDisplay_valueType_float;
		}
		break;
	case kDisplay_data_customAngleVector:
		if (chVector != NULL) {
			vectorValue = chVector->GetValue(index);
			for(int k=0; k<3; k++) vectorValue[k] *= RAD_TO_DEG;
			valueType = kDisplay_valueType_floatVector;
		}
		break;
	}

	float colorFValue[kNumGradientValues];
	float rgbMin[kNumRGBValues], rgbMax[kNumRGBValues];
	switch(valueType) {
	case kDisplay_valueType_float:
		for(i=0; i<numUseValue; i++)
			colorFValue[i] = fValue[i];
		break;
	case kDisplay_valueType_worldFloat:
		for(i=0; i<numUseValue; i++)
			colorFValue[i] = wValue[i];
		break;
	case kDisplay_valueType_percent:
		for(i=0; i<numUseValue; i++)
			colorFValue[i] = pValue[i];
		break;
	case kDisplay_valueType_floatVector:
		for(i=0; i<kNumRGBValues; i++) {
			rgbMin[i] = fRGBMin[i];
			rgbMax[i] = fRGBMax[i];
		}
		break;
	case kDisplay_valueType_worldVector:
		for(i=0; i<kNumRGBValues; i++) {
			rgbMin[i] = wRGBMin[i];
			rgbMax[i] = wRGBMax[i];
		}
		break;
	case kDisplay_valueType_percentVector:
		for(i=0; i<kNumRGBValues; i++) {
			rgbMin[i] = pRGBMin[i];
			rgbMax[i] = pRGBMax[i];
		}
		break;
	}

	switch(valueType) {
	case kDisplay_valueType_bool:
		resCol = boolValue ? gradientColor[1] : gradientColor[0];
		break;
	case kDisplay_valueType_time:
		if (numUseValue == 1) resCol = gradientColor[0];
		else if (numUseValue > 1) {
			if (timeValue <= tValue[0]) resCol = gradientColor[0];
			else if (timeValue >= tValue[numUseValue-1]) resCol = gradientColor[numUseValue-1];
			else {
				for(i=1; i<numUseValue; i++) {
					if (timeValue <= tValue[i]) {
						if (gradientType == kDisplay_gradientType_linear) {
							float ratio = float(timeValue - tValue[i-1])/float(tValue[i] - tValue[i-1]);
							resCol = (1.0f-ratio)*gradientColor[i-1] + ratio*gradientColor[i];
						} else {
							resCol = gradientColor[i-1];
						}
						break;
					}
				}
			}
		}
		break;
	case kDisplay_valueType_integer:
		if (numUseValue == 1) resCol = gradientColor[0];
		else if (numUseValue > 1) {
			if (intValue <= iValue[0]) resCol = gradientColor[0];
			else if (intValue >= iValue[numUseValue-1]) resCol = gradientColor[numUseValue-1];
			else {
				for(i=1; i<numUseValue; i++) {
					if (intValue <= iValue[i]) {
						if (gradientType == kDisplay_gradientType_linear) {
							float ratio = float(intValue - iValue[i-1])/float(iValue[i] - iValue[i-1]);
							resCol = (1.0f-ratio)*gradientColor[i-1] + ratio*gradientColor[i];
						} else {
							resCol = gradientColor[i-1];
						}
						break;
					}
				}
			}
		}
		break;
	case kDisplay_valueType_float:
	case kDisplay_valueType_worldFloat:
	case kDisplay_valueType_percent:
		if (numUseValue == 1) resCol = gradientColor[0];
		else if (numUseValue > 1) {
			if (floatValue <= colorFValue[0]) resCol = gradientColor[0];
			else if (floatValue >= colorFValue[numUseValue-1]) resCol = gradientColor[numUseValue-1];
			else {
				for(i=1; i<numUseValue; i++) {
					if (floatValue <= colorFValue[i]) {
						if (gradientType == kDisplay_gradientType_linear) {
							float ratio = (floatValue - colorFValue[i-1])/(colorFValue[i] - colorFValue[i-1]);
							resCol = (1.0f-ratio)*gradientColor[i-1] + ratio*gradientColor[i];
						} else {
							resCol = gradientColor[i-1];
						}
						break;
					}
				}
			}
		}
		break;
	case kDisplay_valueType_floatVector:
	case kDisplay_valueType_worldVector:
	case kDisplay_valueType_percentVector:
		float rgbValue[3];
		rgbValue[0] = vectorValue[RType];
		rgbValue[1] = vectorValue[GType];
		rgbValue[2] = vectorValue[BType];
		for(i=0; i<3; i++) {
			if (rgbValue[i] <= rgbMin[i]) resCol[i] = 0.0f;
			else if (rgbValue[i] >= rgbMax[i]) resCol[i] = 1.0f;
			else resCol[i] = (rgbValue[i] - rgbMin[i])/(rgbMax[i] - rgbMin[i]);
		}
		break;
	}

	return resCol;
}

int PAOperatorDisplay::Display(IObject* pCont,  TimeValue time, Object* pSystem, INode* psNode, INode* pgNode, ViewExp *vpt, int flags)
{
	bool showParticles = (pblock()->GetInt(kDisplay_showParticles, time) != 0);
	int displayType = pblock()->GetInt(kDisplay_type, time);
	bool showNum = (pblock()->GetInt(kDisplay_showNumbering, time) != 0);
	bool showData = (pblock()->GetInt(kDisplay_showData, time) != 0);
	bool useColorCoding = (pblock()->GetInt(kDisplay_useColorCoding, time) != 0);
	float visPercent = GetPFFloat(pblock(), kDisplay_visible, time);
	float speedFactor = GetPFFloat(pblock(), kDisplay_speedFactor, time);
	int speedMode = pblock()->GetInt(kDisplay_speedMode, time);

	DbgAssert(pCont); DbgAssert(pSystem); DbgAssert(psNode); DbgAssert(pgNode);
	if (pCont == NULL) return 0;
	if (pSystem == NULL) return 0;
	if (psNode == NULL) return 0;
	if (pgNode == NULL) return 0;
	IPFSystem* iSystem = PFSystemInterface(pSystem);
	if (iSystem == NULL) return 0; // invalid particle system

//	if ((displayType == kDisplay_type_none) && (selectedType == kDisplay_type_none))
//		return 0;

	// acquire particle channels
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return 0; // can't find number of particles in the container
	int count = chAmount->Count();
	if (count == 0) return 0;		// there no particles to draw
	int visCount = visPercent * count;
	if (visCount <= 0) return 0;	// there no particles to draw
	IParticleChannelPoint3R* chPos = GetParticleChannelPositionRInterface(pCont);	
	if (chPos == NULL) return 0;	// can't find particle position in the container
	IParticleChannelPoint3R* chSpeed = NULL;
	if (displayType == kDisplay_type_lines)
	{
		chSpeed = GetParticleChannelSpeedRInterface(pCont);
		// if no speed channel then change drawing type to dots
		if (chSpeed == NULL) displayType = kDisplay_type_dots;
	}
	IParticleChannelIDR* chID = GetParticleChannelIDRInterface(pCont);
	// if no ID channel then it's not possible to show numbers & selections
	if (chID == NULL) showNum = 0;
	
	IParticleChannelQuatR* chOrient = NULL;
	IParticleChannelPoint3R* chScale = NULL;
	IParticleChannelMeshR* chShape = NULL;
	IParticleChannelIntR* chMtlIndex = NULL;
	IParticleChannelMeshMapR* chMapping = NULL;

	if ((displayType == kDisplay_type_boundingBoxes) ||
		(displayType == kDisplay_type_geometry))
	{
		chOrient = GetParticleChannelOrientationRInterface(pCont);
		chScale = GetParticleChannelScaleRInterface(pCont);
		chShape = GetParticleChannelShapeRInterface(pCont);
		// if no shape channel then change drawing type to X marks
		if (chShape == NULL) displayType = kDisplay_type_Xs;
		chMtlIndex = GetParticleChannelMtlIndexRInterface(pCont);
		chMapping = GetParticleChannelShapeTextureRInterface(pCont);
	}

	ColorCodingData ccd;
	ccd.Init(pCont, pblock(), time);
	// channels
	IParticleChannelPTVR* chTime = ccd.chTime;
	IParticleChannelPTVR* chBirthTime = ccd.chBirthTime;
	IParticleChannelPTVR* chDeathTime = ccd.chDeathTime;
	IParticleChannelPTVR* chEventStart = ccd.chEventStart;
	IParticleChannelPTVR* chLifespan = ccd.chLifespan;
	IParticleChannelPoint3R* chWorldVector = ccd.chWorldVector;
	IParticleChannelMeshR* chMesh = ccd.chMesh;
	int vectorComponent = ccd.vectorComponent;
	IParticleChannelMeshMapR* chShapeTexture = ccd.chShapeTexture;
	IParticleChannelMapR* chMap = ccd.chMap;
	IParticleChannelIntR* chInteger = ccd.chInteger;
	IParticleChannelQuatR* chRotation = ccd.chRotation;
	if (chScale == NULL) chScale = ccd.chScale;
	IParticleChannelFloatR* chFloat = ccd.chFloat;
	IParticleChannelPoint3R* chVector = ccd.chVector;
	IParticleChannelBoolR* chSelection = ccd.chSelection;
	IParticleChannelAngAxisR* chSpin = ccd.chSpin;

	float scaleFactor = ccd.scaleFactor;
	int dataChannel = ccd.dataChannel;
	useColorCoding = useColorCoding && showParticles && (dataChannel != kDisplay_data_verticesFaces);

	IChannelContainer* chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;
	IParticleChannelIntR* chVisibleR = (IParticleChannelIntR*)chCont->GetPrivateInterface(PARTICLECHANNELVISIBLER_INTERFACE, (Object*)this);

	int i, j, index, num, numMtls=1;
	float sizeFactor;
	Tab<int> blockNon;
	int nonNum=0;
	blockNon.SetCount(count);
	bool doVisibleFilter = ((visPercent < 1.0f) && (chVisibleR != NULL));
	for(i=0; i<count; i++) {
		if (doVisibleFilter)
			if (isInvisible(chVisibleR->GetValue(i))) continue;
		blockNon[nonNum++] = i;
	}
	if (nonNum <= 0) return 0; // no particles to show
	blockNon.SetCount(nonNum);

	GraphicsWindow* gw = vpt->getGW();
	DWORD oldRndLimits, newRndLimits;
	newRndLimits = oldRndLimits = gw->getRndLimits();

	// define color for particles
	bool doPrimColor = false;
	bool doSelColor = false;
	Color primColor, selColor;
	if (psNode->Selected()) {
		doPrimColor = doSelColor = ((oldRndLimits & GW_WIREFRAME) != 0);
		primColor = Color(GetSelColor());
		selColor = Color(GetSelColor());
		useColorCoding = useColorCoding && ((oldRndLimits & GW_WIREFRAME) == 0);
	} else if (psNode->IsFrozen()) {
		doPrimColor = doSelColor = true;
		primColor = Color(GetFreezeColor());
		selColor = Color(GetFreezeColor());
		useColorCoding = false;
	} else {
		primColor = pblock()->GetColor(kDisplay_color);
		selColor = pblock()->GetColor(kDisplay_color);
	}

	Material* nodeMtl = pgNode->Mtls();
	Matrix3 gwTM;
	gwTM.IdentityMatrix();
	gw->setTransform(gwTM);
	Material* curMtl;
	Material vpMtl;
	Point3 pos[33], speed;
	int edgeVis[33];

	enum { noneCat, markerCat, lineCat, geomCat };
	int ordCat;
	switch (displayType) {
	case kDisplay_type_boundingBoxes:
	case kDisplay_type_geometry:
		ordCat = geomCat;
		break;
	case kDisplay_type_lines:
		ordCat = lineCat;
		break;
	default:
		ordCat = markerCat;
	}
	if (showParticles == 0) ordCat = noneCat;

	if (ordCat == geomCat)
	{
		DWORD prevLimits = newRndLimits;
		DWORD boxLimits = newRndLimits;
		if (!(boxLimits&GW_BOX_MODE)) boxLimits |= GW_BOX_MODE;

		vpMtl = *(SysUtil::GetParticleMtl());
		Tab<int> *block = &blockNon;
		if (displayType == kDisplay_type_boundingBoxes)
			gw->setRndLimits(boxLimits);
		else
			gw->setRndLimits(prevLimits);
		gw->setColor(LINE_COLOR, primColor);
		if ((nodeMtl != NULL) && (!doPrimColor)) {
			curMtl = nodeMtl;
			numMtls = pgNode->NumMtls();
		} else {
			curMtl = &vpMtl;
			if (doPrimColor) curMtl->Kd = curMtl->Ks = primColor;
			else curMtl->Kd = curMtl->Ks = pblock()->GetColor(kDisplay_color);
			numMtls = 1;
		}
		if (useColorCoding) {
			curMtl = &vpMtl;
			numMtls = 1;
		}
		for(i=0; i<numMtls; i++) gw->setMaterial(curMtl[i], i);

		for(i=0; i<block->Count(); i++) {
			index = (*block)[i];
			Mesh* pMesh = NULL;
			if (chShape != NULL)
				pMesh = const_cast <Mesh*>(chShape->GetValue(index));
			if (useColorCoding) {
				Color curColor = ccd.GetColor(index);
				curMtl->Kd = curMtl->Ks = curColor;
				gw->setColor(LINE_COLOR, curColor);
			}
			Mesh curMesh;
			if (pMesh) {
				gwTM.IdentityMatrix();
				if (chOrient) gwTM.SetRotate(chOrient->GetValue(index));
				if (chScale) gwTM.PreScale(chScale->GetValue(index));
				gwTM.SetTrans(chPos->GetValue(index));
				gw->setTransform(gwTM);
				// set vertex color
				switch (psNode->GetVertexColorType()) {
				case nvct_color:
					if (pMesh->curVCChan == 0) break;
					pMesh->setVCDisplayData (0);
					break;
				case nvct_illumination:
					if (pMesh->curVCChan == MAP_SHADING) break;
					pMesh->setVCDisplayData (MAP_SHADING);
					break;
				case nvct_alpha:
					if (pMesh->curVCChan == MAP_ALPHA) break;
					pMesh->setVCDisplayData (MAP_ALPHA);
					break;
				}

				// see SDK mesh->render() method remark for necessity of this call
				if ((numMtls > 1) || useColorCoding) gw->setMaterial(curMtl[0], 0);

				if ((chMtlIndex != NULL) || (chMapping != NULL)) {
					curMesh = *pMesh;
					bool assignedMtlIndices = AssignMeshMaterialIndices(&curMesh, chMtlIndex, index);
					bool assignedMapping = AssignMeshMapping(&curMesh, chMapping, index);
					if (assignedMtlIndices || assignedMapping) 
						pMesh = &curMesh;
					if (assignedMtlIndices) {
						int mtlIndex = chMtlIndex->GetValue(index);
						if (mtlIndex < numMtls)
							gw->setMaterial(curMtl[mtlIndex], mtlIndex);
					}
					pMesh->InvalidateStrips();
				}
				pMesh->render(gw, curMtl, (flags&USE_DAMAGE_RECT) ? &vpt->GetDammageRect() : NULL, COMP_ALL, numMtls);
			} else {
				pos[0] = chPos->GetValue(index);
				gw->marker(pos, X_MRKR);
			}
		}

		gwTM.IdentityMatrix();
		gw->setTransform(gwTM);
		gw->setRndLimits(prevLimits);
	}

	if (ordCat == lineCat)
	{
		sizeFactor = GetTicksPerFrame()*speedFactor;
		for(i=0; i<32; i++)
			edgeVis[i] = (i%2) ? GW_EDGE_SKIP : GW_EDGE_VIS;
		edgeVis[32] = GW_EDGE_SKIP;
		num = blockNon.Count();

		if (useColorCoding) {
			for(index=0; index<num; index++)
			{
				i = blockNon[index];
				pos[0] = chPos->GetValue(i);
				Point3 speedVec = sizeFactor*chSpeed->GetValue(i);
				switch(speedMode) {
				case kDisplay_speedMode_leading:
					pos[1] = pos[0] + speedVec;
					break;
				case kDisplay_speedMode_centered:
					pos[1] = pos[0] + 0.5f*speedVec;
					pos[0] = pos[0] - 0.5f*speedVec;
					break;
				case kDisplay_speedMode_trailing:
					pos[1] = pos[0];
					pos[0] = pos[0] - speedVec;
					break;
				}
				gw->setColor(LINE_COLOR, ccd.GetColor(i));
				gw->polyline(2, pos, NULL, NULL, 0, edgeVis);
			}
		} else {
			gw->setColor(LINE_COLOR, primColor);
			j = 0;
			for(index=0; index<num; index++)
			{
				i = blockNon[index];
				pos[j] = chPos->GetValue(i);
				Point3 speedVec = sizeFactor*chSpeed->GetValue(i);
				switch(speedMode) {
				case kDisplay_speedMode_leading:
					pos[j+1] = pos[j] + speedVec;
					break;
				case kDisplay_speedMode_centered:
					pos[j+1] = pos[j] + 0.5f*speedVec;
					pos[j] = pos[j] - 0.5f*speedVec;
					break;
				case kDisplay_speedMode_trailing:
					pos[j+1] = pos[j];
					pos[j] = pos[j] - speedVec;
					break;
				}
				j += 2;
				if (j == 30) {
					gw->polyline(j, pos, NULL, NULL, 0, edgeVis);
					j = 0;
				}	
			}
			if (j) gw->polyline(j, pos, NULL, NULL, 0, edgeVis);
		}
	}

	if (ordCat == markerCat)
	{
		MarkerType mType;
		int num = blockNon.Count();
		switch (displayType)
		{
			case kDisplay_type_dots: mType = POINT_MRKR; break;
			case kDisplay_type_ticks: mType = PLUS_SIGN_MRKR; break;
			case kDisplay_type_circles: mType = CIRCLE_MRKR; break;
			case kDisplay_type_diamonds: mType = DIAMOND_MRKR; break;
			case kDisplay_type_boxes: mType = HOLLOW_BOX_MRKR; break;
			case kDisplay_type_asterisks: mType = ASTERISK_MRKR; break;
			case kDisplay_type_triangles: mType = TRIANGLE_MRKR; break;
			case kDisplay_type_Xs: mType = X_MRKR; break;
		}
		if (useColorCoding) {
			for(i=0; i<num; i++) {
				pos[0] = chPos->GetValue(blockNon[i]);
				gw->setColor(LINE_COLOR, ccd.GetColor(blockNon[i]));
				gw->marker(pos, mType);
			}
		} else {
			gw->setColor(LINE_COLOR, primColor);
			gw->startMarkers();
			for (i=0; i<num; i++) {
				pos[0] = chPos->GetValue(blockNon[i]);
				gw->marker(pos, mType);
			}
			gw->endMarkers();									
		}
	}

	if (showNum || showData)
	{ // show born index along with each particle
		TCHAR numText[16];
		TCHAR dataText[256];
		TCHAR completeText[272];
		num = blockNon.Count();
		if (num > 0) {
			gw->setColor(TEXT_COLOR, primColor);

			IParticleChannelBoolR* chSelected = NULL;
			bool showSelectedOnly = (pblock()->GetInt(kDisplay_selectedOnly, time) != 0);
			if (showData && showSelectedOnly)
				chSelected = GetParticleChannelSelectionRInterface(pCont);
			int spinnerPrec = GetDisplayPrecision();
			for(i=0; i<num; i++) {
				index = blockNon[i];
				if (showNum) 
					sprintf(numText, "%d", chID->GetParticleBorn(index)+1); // indices shown are 1-based
				bool showTheParticleData = showSelectedOnly ?
							((chSelected != NULL) ? (showData != 0) && chSelected->GetValue(index) : false)
							: (showData != 0);
				if (showTheParticleData) {
					TSTR dataString;
					switch( dataChannel) {
					case kDisplay_data_birthTime:
						if ((chTime == NULL) || (chBirthTime == NULL)) dataString = GetString(IDS_UNKNOWNDATA);
						else TimeToString((chBirthTime->GetValue(index)).TimeValue(), dataString);
						sprintf(dataText, "%s", dataString);
						break;
					case kDisplay_data_age:
						if ((chTime == NULL) || (chBirthTime == NULL)) dataString = GetString(IDS_UNKNOWNDATA);
						else TimeToString(TimeValue(chTime->GetValue(index) - chBirthTime->GetValue(index)), dataString);
						sprintf(dataText, "%s", dataString);
						break;
					case kDisplay_data_deathTime:
						if ((chDeathTime == NULL) && ((chTime == NULL) || (chLifespan == NULL)))
								dataString = GetString(IDS_UNKNOWNDATA);
						else {
							if ((chTime != NULL) && (chLifespan != NULL)) 
								TimeToString(TimeValue(chTime->GetValue(index) + chLifespan->GetValue(index)), dataString);
							else
								TimeToString((chDeathTime->GetValue(index)).TimeValue(), dataString);
						}
						sprintf(dataText, "%s", dataString);
						break;
					case kDisplay_data_lifespan:
						if ((chLifespan == NULL) && ((chTime == NULL) || (chDeathTime == NULL)))
							dataString = GetString(IDS_UNKNOWNDATA);
						else {
							if (chLifespan != NULL) TimeToString((chLifespan->GetValue(index)).TimeValue(), dataString);
							else TimeToString(TimeValue(chDeathTime->GetValue(index) - chTime->GetValue(index)), dataString); 
						}
						sprintf(dataText, "%s", dataString);
						break;
					case kDisplay_data_timeInEvent:
						if ((chTime == NULL) || (chEventStart == NULL)) dataString = GetString(IDS_UNKNOWNDATA);
						else TimeToString(TimeValue(chTime->GetValue(index) - chEventStart->GetValue(index)), dataString); 
						sprintf(dataText, "%s", dataString);
						break;
					case kDisplay_data_position:
					case kDisplay_data_speedVector:
						if (chWorldVector == NULL) dataString = GetString(IDS_UNKNOWNDATA);
						else {
							Point3 vec = chWorldVector->GetValue(index);
							vec = scaleFactor*vec;
							dataString = FormatUniverseVectorValue(vec);
						}
						sprintf(dataText, "%s", dataString);
						break;
					case kDisplay_data_positionX:
					case kDisplay_data_positionY:
					case kDisplay_data_positionZ:
					case kDisplay_data_speedX:
					case kDisplay_data_speedY:
					case kDisplay_data_speedZ:
					case kDisplay_data_speedMagnitude:
						if (chWorldVector == NULL) dataString = GetString(IDS_UNKNOWNDATA);
						else {
							Point3 vec = chWorldVector->GetValue(index);
							float floatValue = 0.0f;
							if (dataChannel == kDisplay_data_speedMagnitude) floatValue = Length(vec);
							else floatValue = vec[vectorComponent];
							floatValue *= scaleFactor;
							TCHAR* resVal = FormatUniverseValue(floatValue);
							dataString = (resVal[0] == '\0') ? _T("overflow") : resVal;
						}
						sprintf(dataText, "%s", dataString);
						break;
					case kDisplay_data_mapChannel1:
					case kDisplay_data_mapChannel2:
					case kDisplay_data_mapChannel3:
					case kDisplay_data_mapChannel4:
					case kDisplay_data_mapChannel5:
						if (chMap == NULL) sprintf(dataText,"%s",GetString(IDS_UNKNOWNDATA));
						else {
							switch((chMap->GetUVVert(index))->Count()) {
							case 0:
								sprintf(dataText,"%s",GetString(IDS_UNKNOWNDATA));
								break;
							case 1:															{
								UVVert tVert = (*(chMap->GetUVVert(index)))[0];
								sprintf(dataText,"%s", FormatFloatVectorValue(tVert, spinnerPrec));	}
								break;
							default:
								sprintf(dataText,"%s",GetString(IDS_MULTTEXTUREVERTICES));
								break;
							}
						}
						break;
					case kDisplay_data_mtlIndex:
					case kDisplay_data_scriptInteger:
						if (chInteger == NULL) sprintf(dataText,"%s",GetString(IDS_UNKNOWNDATA));
						else {
							int intVal = chInteger->GetValue(index);
							if(dataChannel == kDisplay_data_mtlIndex) intVal++; // 1-based mtlIndices for show
							sprintf(dataText, "%d", intVal);
						}
						break;
					case kDisplay_data_rotationAxis:
					case kDisplay_data_rotationAngle:
						if (chRotation == NULL) sprintf(dataText,"%s",GetString(IDS_UNKNOWNDATA));
						else {
							float angle;
							Point3 axis;
							AngAxisFromQ(chRotation->GetValue(index), &angle, axis);
							if (dataChannel == kDisplay_data_rotationAxis) {
								sprintf(dataText,"%s", FormatFloatVectorValue(axis, spinnerPrec));	
							} else {
								angle *= RAD_TO_DEG;
								sprintf(dataText,"%s", FormatFloatValue(angle, spinnerPrec));	
							}
						}
						break;
					case kDisplay_data_euclideRotation:
						if (chRotation == NULL) sprintf(dataText,"%s",GetString(IDS_UNKNOWNDATA));
						else {
							Point3 ang;
							Quat q = chRotation->GetValue(index);
							QuatToEuler(q, ang);
							for(int k=0; k<3; k++) ang[k] *= RAD_TO_DEG;
							sprintf(dataText,"%s", FormatFloatVectorValue(ang, spinnerPrec));	
						}
						break;
					case kDisplay_data_scale:
					case kDisplay_data_scaleX:
					case kDisplay_data_scaleY:
					case kDisplay_data_scaleZ:
						if (chScale == NULL) sprintf(dataText,"%s",GetString(IDS_UNKNOWNDATA));
						else {
							Point3 scale = chScale->GetValue(index);
							scale *= 100.0f; // shown as percentage
							if (dataChannel == kDisplay_data_scale) {
								sprintf(dataText,"%s", FormatFloatVectorValue(scale, spinnerPrec));	
							} else {
								sprintf(dataText,"%s", FormatFloatValue(scale[dataChannel-kDisplay_data_scaleX], spinnerPrec));
							}
						}
						break;
					case kDisplay_data_size:
					case kDisplay_data_sizeX:
					case kDisplay_data_sizeY:
					case kDisplay_data_sizeZ:
						if (chMesh == NULL) sprintf(dataText, "%s", GetString(IDS_UNKNOWNDATA));
						else {
							Mesh* curShape = const_cast <Mesh*>(chMesh->GetValue(index));
							Box3 bbox;
							bbox.pmin = bbox.pmax = Point3::Origin;
							if (curShape != NULL) bbox = curShape->getBoundingBox();
							Point3 scale = Point3(1.0f, 1.0f, 1.0f);
							if (chScale) scale = chScale->GetValue(index);
							Point3 size = Point3(scale.x*(bbox.pmax.x-bbox.pmin.x), scale.y*(bbox.pmax.y-bbox.pmin.y), scale.z*(bbox.pmax.z-bbox.pmin.z));
							if (dataChannel == kDisplay_data_size) {
								sprintf(dataText,"%s", FormatUniverseVectorValue(size));	
							} else {
								sprintf(dataText,"%s", FormatUniverseValue(size[dataChannel-kDisplay_data_sizeX]));
							}
						}
						break;
					case kDisplay_data_scriptFloat:
						if (chFloat == NULL) sprintf(dataText, "%s", GetString(IDS_UNKNOWNDATA));
						else sprintf(dataText,"%s", FormatFloatValue(chFloat->GetValue(index), spinnerPrec));
						break;
					case kDisplay_data_scriptVector:
						if (chVector == NULL) sprintf(dataText, "%s", GetString(IDS_UNKNOWNDATA));
						else sprintf(dataText,"%s", FormatFloatVectorValue(chVector->GetValue(index), spinnerPrec));
						break;
					case kDisplay_data_selected:
						if (chSelection == NULL) sprintf(dataText, "%s", GetString(IDS_UNKNOWNDATA));
						else sprintf(dataText, chSelection->GetValue(index) ? GetString(IDS_TRUE) : GetString(IDS_FALSE) );
						break;
					case kDisplay_data_spinAxis:
					case kDisplay_data_spinRate:
						if (chSpin == NULL) sprintf(dataText, "%s", GetString(IDS_UNKNOWNDATA));
						else {
							AngAxis aa = chSpin->GetValue(index);
							if (dataChannel == kDisplay_data_spinAxis) {
								sprintf(dataText,"%s", FormatFloatVectorValue(aa.axis, spinnerPrec));
							} else {
								float rate = aa.angle*TIME_TICKSPERSEC*RAD_TO_DEG;
								sprintf(dataText,"%s", FormatFloatValue(rate, spinnerPrec));
							}
						} 
						break;
					case kDisplay_data_verticesFaces:
						if (chMesh == NULL) sprintf(dataText, "%s", GetString(IDS_UNKNOWNDATA));
						else {
							Mesh* curShape = const_cast <Mesh*>(chMesh->GetValue(index));
							int numVerts = 0, numFaces = 0;
							if (curShape != NULL) {
								numVerts = curShape->numVerts;
								numFaces= curShape->numFaces;
							}
							sprintf(dataText, "V:%d F:%d", numVerts, numFaces);
						}
						break;
					case kDisplay_data_customWorldFloat:
						if (chFloat == NULL) dataString = GetString(IDS_UNKNOWNDATA);
						else {
							float floatValue = chFloat->GetValue(index);
							TCHAR* resVal = FormatUniverseValue(floatValue);
							dataString = (resVal[0] == '\0') ? _T("overflow") : resVal;
						}
						sprintf(dataText, "%s", dataString);
						break;
					case kDisplay_data_customPercent:
						if (chFloat == NULL) dataString = GetString(IDS_UNKNOWNDATA);
						else {
							float floatValue = 100.0f*chFloat->GetValue(index); // shown as percentage
							TCHAR* resVal = FormatFloatValue(floatValue, spinnerPrec);
							dataString = (resVal[0] == '\0') ? _T("overflow") : resVal;
						}
						sprintf(dataText, "%s", dataString);
						break;
					case kDisplay_data_customAngle:
						if (chFloat == NULL) dataString = GetString(IDS_UNKNOWNDATA);
						else {
							float floatValue = RAD_TO_DEG*chFloat->GetValue(index); // shown as degrees
							TCHAR* resVal = FormatFloatValue(floatValue, spinnerPrec);
							dataString = (resVal[0] == '\0') ? _T("overflow") : resVal;
						}
						sprintf(dataText, "%s", dataString);
						break;
					case kDisplay_data_customAngleVector:
						if (chVector == NULL) dataString = GetString(IDS_UNKNOWNDATA);
						else {
							Point3 vectorValue = RAD_TO_DEG*chVector->GetValue(index); // shown as degrees
							TCHAR* resVal = FormatFloatVectorValue(vectorValue, spinnerPrec);
						}
						sprintf(dataText, "%s", dataString);
						break;
					default:
						dataString = GetString(IDS_UNKNOWNDATA);
						sprintf(dataText, "%s", dataString);
						break;
					}
				}
				pos[0] = chPos->GetValue(index);
				if (showNum && showTheParticleData) {
					sprintf(completeText, "%s> %s", numText, dataText);
					gw->text(pos, completeText);
				} else if (showNum) gw->text(pos, numText);
				else if (showTheParticleData) gw->text(pos, dataText);
			}
		}
	}

	return 0;
}

//+--------------------------------------------------------------------------+
//|							From IPFViewport								 |
//+--------------------------------------------------------------------------+
int PAOperatorDisplay::HitTest(IObject* pCont, TimeValue time, Object* pSystem, INode* psNode, INode* pgNode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt)
{
	if (pblock()->GetInt(kDisplay_showParticles, time) == 0) return 0;
	int displayType = pblock()->GetInt(kDisplay_type, time);
	float visPercent = GetPFFloat(pblock(), kDisplay_visible, time);

	DbgAssert(pCont); DbgAssert(pSystem); DbgAssert(psNode); DbgAssert(pgNode);
	if (pCont == NULL) return 0;
	if (pSystem == NULL) return 0;
	if (psNode == NULL) return 0;
	if (pgNode == NULL) return 0;
	IPFSystem* iSystem = PFSystemInterface(pSystem);
	if (iSystem == NULL) return 0; // invalid particle system

	IParticleGroup* iPGroup = ParticleGroupInterface(pgNode);
	INode* theActionListNode = (iPGroup != NULL) ? iPGroup->GetActionList() : NULL;

	BOOL abortOnHit = flags&SUBHIT_ABORTONHIT	? TRUE : FALSE;
	
//	if ((displayType == kDisplay_type_none) && (selectedType == kDisplay_type_none))
//		return 0;

	// acquire particle channels
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return 0; // can't find number of particles in the container
	int count = chAmount->Count();
	if (count == 0) return 0;		// there no particles to hit
	int visCount = visPercent * count;
	if (visCount <= 0) return 0;	// there no particles to hit
	IParticleChannelPoint3R* chPos = GetParticleChannelPositionRInterface(pCont);	
	if (chPos == NULL) return 0;	// can't find particle position in the container
	IParticleChannelPoint3R* chSpeed = NULL;
	if (displayType == kDisplay_type_lines)
	{
		chSpeed = GetParticleChannelSpeedRInterface(pCont);
		// if no speed channel then change drawing type to dots
		if (chSpeed == NULL) displayType = kDisplay_type_dots;
	}
	IParticleChannelIDR* chID = GetParticleChannelIDRInterface(pCont);
	IParticleChannelQuatR* chOrient = NULL;
	IParticleChannelPoint3R* chScale = NULL;
	IParticleChannelMeshR* chShape = NULL;
	if ((displayType == kDisplay_type_boundingBoxes) ||
		(displayType == kDisplay_type_geometry))
	{
		chOrient = GetParticleChannelOrientationRInterface(pCont);
		chScale = GetParticleChannelScaleRInterface(pCont);
		chShape = GetParticleChannelShapeRInterface(pCont);
		// if no shape channel then change drawing type to X marks
		if (chShape == NULL) displayType = kDisplay_type_Xs;
	}

	IChannelContainer* chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;
	IParticleChannelIntR* chVisibleR = (IParticleChannelIntR*)chCont->GetPrivateInterface(PARTICLECHANNELVISIBLER_INTERFACE, (Object*)this);

	int i, index, num, numMtls;
	float sizeFactor;
	Tab<int> blockNon, blockSel;
	int nonNum=0;
	IParticleChannelBoolR* chSelect = GetParticleChannelSelectionRInterface(pCont);
	blockNon.SetCount(count);
	bool doVisibleFilter = ((visPercent < 1.0f) && (chVisibleR != NULL));
	for(i=0; i<count; i++) {
		if (doVisibleFilter)
			if (isInvisible(chVisibleR->GetValue(i))) continue;
		blockNon[nonNum++] = i;
	}
	if (nonNum <= 0) return false;
	blockNon.SetCount(nonNum);

	enum { noneCat, markerCat, lineCat, geomCat };
	int ordCat;
	switch (displayType) {
	case kDisplay_type_boundingBoxes:
	case kDisplay_type_geometry:
		ordCat = geomCat;
		break;
	case kDisplay_type_lines:
		ordCat = lineCat;
		break;
	default:
		ordCat = markerCat;
	}

	DWORD savedLimits;
	Matrix3 gwTM;
	Point3 pos[33], speed;
	int res=0;
	HitRegion hr;
	Material* curMtl;
	PFHitData* hitData = NULL;
	bool firstHit = true;
	
	GraphicsWindow* gw = vpt->getGW();
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~ GW_ILLUM);
	gwTM.IdentityMatrix();
	gw->setTransform(gwTM);
	MakeHitRegion(hr, type, crossing, 4, p);
	gw->setHitRegion(&hr);
	gw->clearHitCode();

	if (ordCat == lineCat)
	{
		sizeFactor = GetTicksPerFrame();
		num = blockNon.Count();
		for(i=0; i<num; i++)
		{
			int index = blockNon[i];
			pos[0] = chPos->GetValue(index);
			pos[1] = pos[0] + sizeFactor*chSpeed->GetValue(index);
			gw->polyline(2, pos, NULL, NULL, 1, NULL);
			if (gw->checkHitCode()) {
				res = TRUE;
					if (firstHit) hitData = new PFHitData(pgNode, theActionListNode);
				vpt->LogHit(psNode, (ModContext*)pSystem, gw->getHitDistance(), chID ? chID->GetParticleBorn(index) : 0, hitData);
				if (firstHit) {
					hitData = NULL;
					firstHit = false;
				}
				gw->clearHitCode();
				if (abortOnHit) goto hasHit;
			}
		}
	}

	if (ordCat == markerCat)
	{
		MarkerType mType;
		int num = blockNon.Count();
		switch (displayType)
		{
			case kDisplay_type_dots: mType = POINT_MRKR; break;
			case kDisplay_type_ticks: mType = PLUS_SIGN_MRKR; break;
			case kDisplay_type_circles: mType = CIRCLE_MRKR; break;
			case kDisplay_type_diamonds: mType = DIAMOND_MRKR; break;
			case kDisplay_type_boxes: mType = HOLLOW_BOX_MRKR; break;
			case kDisplay_type_asterisks: mType = ASTERISK_MRKR; break;
			case kDisplay_type_triangles: mType = TRIANGLE_MRKR; break;
			case kDisplay_type_Xs: mType = X_MRKR; break;
		}
		for(i=0; i<num; i++)
		{
			int index = blockNon[i];
			pos[0] = chPos->GetValue(index);
			gw->marker(pos, mType);
			if (gw->checkHitCode()) {
				res = TRUE;
				if (firstHit) hitData = new PFHitData(pgNode, theActionListNode);
				vpt->LogHit(psNode, (ModContext*)pSystem, gw->getHitDistance(), chID ? chID->GetParticleBorn(index) : 0, hitData);
				if (firstHit) {
					hitData = NULL;
					firstHit = false;
				}
				gw->clearHitCode();
				if (abortOnHit) goto hasHit;
			}
		}
	}

	if (ordCat == geomCat)
	{
		DWORD prevLimits = savedLimits;
		DWORD boxLimits = savedLimits;
		if (!(boxLimits&GW_BOX_MODE)) boxLimits |= GW_BOX_MODE;

		curMtl = SysUtil::GetParticleMtl();
		numMtls = 1;
		if (pgNode->Mtls() != NULL) {
			curMtl = pgNode->Mtls();
			numMtls = pgNode->NumMtls();
		}

		Tab<int> *block = &blockNon;
		if (displayType == kDisplay_type_boundingBoxes)
			gw->setRndLimits(boxLimits);
		else
			gw->setRndLimits(prevLimits);

		for(i=0; i<block->Count(); i++) {
			index = (*block)[i];
			Mesh* pMesh = NULL;
			if (chShape != NULL)
				pMesh = const_cast <Mesh*>(chShape->GetValue(index));
			if (pMesh) {
				gwTM.IdentityMatrix();
				if (chOrient) gwTM.SetRotate(chOrient->GetValue(index));
				if (chScale) gwTM.PreScale(chScale->GetValue(index));
				gwTM.SetTrans(chPos->GetValue(index));
				gw->setTransform(gwTM);
				pMesh->select(gw, curMtl, &hr, TRUE, numMtls);
			} else {
				pos[0] = chPos->GetValue(index);
				gw->marker(pos, X_MRKR);
			}

			if (gw->checkHitCode()) {
				res = TRUE;
				if (firstHit) hitData = new PFHitData(pgNode, theActionListNode); // the extended data information is attached to the hit of the first particle only to save on memory allocations
				vpt->LogHit(psNode, (ModContext*)pSystem, gw->getHitDistance(), chID ? chID->GetParticleBorn(index) : 0, hitData);
				if (firstHit) {
					hitData = NULL;
					firstHit = false;
				}
				gw->clearHitCode();
				if (abortOnHit) goto hasHit;
			}
		}
		gwTM.IdentityMatrix();
		gw->setTransform(gwTM);
		gw->setRndLimits(prevLimits);
	}

hasHit:
	gw->setRndLimits(savedLimits);
	return res;
}

//+--------------------------------------------------------------------------+
//|							From IPFViewport								 |
//+--------------------------------------------------------------------------+
void PAOperatorDisplay::GetWorldBoundBox(IObject* pCont, TimeValue time, Object* pSystem, INode* inode, ViewExp* vp, Box3& box )
{
	int showParticles = pblock()->GetInt(kDisplay_showParticles, time);
	int showNum = pblock()->GetInt(kDisplay_showNumbering, time);
	int showData = pblock()->GetInt(kDisplay_showData, time);
	if ((showParticles == 0) && (showNum == 0) && (showData == 0)) return;

	int displayType = pblock()->GetInt(kDisplay_type);
	// all particles are visible in 'select particles' mode
	if (pSystem == NULL) return;

//	if ((displayType == kDisplay_type_none) &&
//		(selectedType == kDisplay_type_none)) return; // nothing to draw

	// acquire particle channels
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return; // can't find number of particles in the container
	int count = chAmount->Count();
	if (count == 0) return; // there no particles to draw
	IParticleChannelPoint3R* chPos = GetParticleChannelPositionRInterface(pCont);	
	if (chPos == NULL) return; // can't find particle position in the container

	box += chPos->GetBoundingBox();
	
	if (displayType == kDisplay_type_lines)
	{
		IParticleChannelPoint3R* chSpeed = GetParticleChannelSpeedRInterface(pCont);
		if (chSpeed != NULL) 
			box.EnlargeBy(GetTicksPerFrame()*chSpeed->GetMaxLengthValue());
	}
	if (displayType == kDisplay_type_boundingBoxes)
	{
		IParticleChannelMeshR* chShape = GetParticleChannelShapeRInterface(pCont);
		if (chShape != NULL)	{
			Box3 meshBox = chShape->GetMaxBoundingBox();
			if (!meshBox.IsEmpty())	{
				Point3 pmin = meshBox.Min();
				Point3 pmax = meshBox.Max();
				Point3 h( max(fabs(pmin.x),fabs(pmax.x)), max(fabs(pmin.y),fabs(pmax.y)), max(fabs(pmin.z),fabs(pmax.z)) );
				float enlargeFactor = Length(h);
				IParticleChannelPoint3R* chScale = GetParticleChannelScaleRInterface(pCont);
				if (chScale)
					enlargeFactor *= chScale->GetMaxLengthValue();
				box.EnlargeBy(enlargeFactor);
			}
		}
	}
}

//+--------------------------------------------------------------------------+
//|							From IPFViewport								 |
//+--------------------------------------------------------------------------+
void PAOperatorDisplay::GetLocalBoundBox(IObject* pCont, TimeValue time, Object* pSystem, INode* inode, ViewExp* vp, Box3& box )
{
	Box3 pbox;
	GetWorldBoundBox(pCont, time, pSystem, inode, vp, pbox);
	if (!pbox.IsEmpty())
		box += pbox*Inverse(inode->GetObjTMBeforeWSM(time));
}

//+--------------------------------------------------------------------------+
//|							From IPFViewport								 |
//+--------------------------------------------------------------------------+
DWORD PAOperatorDisplay::GetWireColor() const
{
	return (pblock()->GetColor(kDisplay_color, 0)).toRGB();
}

//+--------------------------------------------------------------------------+
//|							From IPFViewport								 |
//+--------------------------------------------------------------------------+
void PAOperatorDisplay::SetWireColor(DWORD color)
{
	pblock()->SetValue(kDisplay_color, 0, Color(color));
}

//+--------------------------------------------------------------------------+
//|							From IPFViewport								 |
//+--------------------------------------------------------------------------+
void PAOperatorDisplay::MaybeEnlargeViewportRect(IObject* pCont, TimeValue time, GraphicsWindow *gw, Rect &rect)
{
	// check out if writing particles IDs
	bool showNumbers = (pblock()->GetInt(kDisplay_showNumbering, time) != 0);
	bool showData = (pblock()->GetInt(kDisplay_showData, time) != 0);
	if (!(showNumbers || showData)) return;

	// acquire particle channels
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return; // can't find number of particles in the container
	int count = chAmount->Count();
	if (count == 0) return; // there no particles to draw
	IParticleChannelIDR* chID = GetParticleChannelIDRInterface(pCont);	
	if (chID == NULL) return; // can't find particle position in the container
	int maxNumber = 0;
	for(int i=0; i<count; i++) {
		int curNum = chID->GetParticleBorn(i);
		if (curNum > maxNumber) maxNumber = curNum;
	}
	TCHAR dummy[256];
	SIZE size;
	if (showNumbers && showData)
		sprintf(dummy, "%d: [44444.444, 44444.444, 44444.444]", maxNumber);
	else if (showNumbers)
		sprintf(dummy, "%d", maxNumber);
	else sprintf(dummy, "[44444.444, 44444.444, 44444.444]");
	gw->getTextExtents(dummy, &size);
	rect.SetW(rect.w() + size.cx);
	rect.SetY(rect.y() - size.cy);
	rect.SetH(rect.h() + size.cy);
}

//+--------------------------------------------------------------------------+
//|							From PAOperatorDisplay							 |
//+--------------------------------------------------------------------------+
void PAOperatorDisplay::RefreshUI(WPARAM message, IParamMap2* map) const
{
	HWND hWnd;
	if (map != NULL) {
		hWnd = map->GetHWnd();
		if ( hWnd ) {
			SendMessage( hWnd, WM_COMMAND, message, (LPARAM)0);
		}
		return;
	}

	if ( pblock() == NULL ) return;

	map = pblock()->GetMap();
	if ( (map == NULL) && (NumPViewParamMaps() == 0) ) return;

	if (map != NULL) {
		hWnd = map->GetHWnd();
		if ( hWnd ) {
			SendMessage( hWnd, WM_COMMAND, message, (LPARAM)0);
		}
	}
	for(int i=0; i<NumPViewParamMaps(); i++) {
		hWnd = GetPViewParamMap(i)->GetHWnd();
		SendMessage( hWnd, WM_COMMAND, message, (LPARAM)0);
	}
}

//+--------------------------------------------------------------------------+
//|							From PAOperatorDisplay							 |
//+--------------------------------------------------------------------------+
void PAOperatorDisplay::UpdatePViewUI(int updateID, int tabIndex) const
{
	for(int i=0; i<NumPViewParamMaps(); i++) {
		IParamMap2* map = GetPViewParamMap(i);
		map->Invalidate(updateID, tabIndex);
	}
}

//+--------------------------------------------------------------------------+
//|							From PAOperatorDisplay							 |
//+--------------------------------------------------------------------------+
int PAOperatorDisplay::GetDisplayPrecision() const
{
	if (pblock() == NULL) return 0;
	return pblock()->GetInt(kDisplay_precision, 0);
}

//+--------------------------------------------------------------------------+
//|							From PAOperatorDisplay							 |
//+--------------------------------------------------------------------------+
bool PAOperatorDisplay::isInvisible(int index)
// index is 1-based
{
	if (index <= 0) return false;
	if (index > invisibleParticles().GetSize()) recalcVisibility(index+1023);	
	return (invisibleParticles()[index-1] != 0);
}

//+--------------------------------------------------------------------------+
//|							From PAOperatorDisplay							 |
//+--------------------------------------------------------------------------+
void PAOperatorDisplay::recalcVisibility(int amount)
{
	if (amount == 0) {
		_invisibleParticles().SetSize(0);
		return;
	}
	if (amount < invisibleParticles().GetSize()) {
		_invisibleParticles().SetSize(amount, 1);
		return;
	}
	float visPercent = GetPFFloat(pblock(), kDisplay_visible, 0);
	DbgAssert(visPercent < 1.0f);

	int oldNum = invisibleParticles().GetSize();
	int visible = oldNum - _invisibleParticles().NumberSet();
	_invisibleParticles().SetSize(amount, 1);
	if (oldNum == 0) {
		visible = 1;
		_invisibleParticles().Clear(0);
		oldNum = 1;
	}
	for(int i=oldNum; i<amount; i++) {
		if (float(visible)/i > visPercent) {
			_invisibleParticles().Set(i);
		} else {
			_invisibleParticles().Clear(i);
			visible++;
		}
	}
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PAOperatorDisplay::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_DISPLAY_ACTIVEICON),IMAGE_BITMAP,
//									kActionImageWidth, kActionImageHeight, LR_LOADTRANSPARENT|LR_SHARED);
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PAOperatorDisplay::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_DISPLAY_INACTIVEICON),IMAGE_BITMAP,
//									kActionImageWidth, kActionImageHeight, LR_LOADTRANSPARENT|LR_SHARED);
									kActionImageWidth, kActionImageHeight, LR_SHARED);

	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PAOperatorDisplay::HasDynamicName(TSTR& nameSuffix)
{
	int type	= pblock()->GetInt(kDisplay_type, 0);
	int ids;
	switch(type) {
	case kDisplay_type_dots:			ids = IDS_DISPLAYTYPE_DOTS;		break;
	case kDisplay_type_ticks:			ids = IDS_DISPLAYTYPE_TICKS;	break;
	case kDisplay_type_circles:			ids = IDS_DISPLAYTYPE_CIRCLES;	break;
	case kDisplay_type_lines:			ids = IDS_DISPLAYTYPE_LINES;	break;
	case kDisplay_type_boundingBoxes:	ids = IDS_DISPLAYTYPE_BBOXES;	break;
	case kDisplay_type_geometry:		ids = IDS_DISPLAYTYPE_GEOM;		break;
	case kDisplay_type_diamonds:		ids = IDS_DISPLAYTYPE_DIAMONDS;	break;
	case kDisplay_type_boxes:			ids = IDS_DISPLAYTYPE_BOXES;	break;
	case kDisplay_type_asterisks:		ids = IDS_DISPLAYTYPE_ASTERISKS;break;
	case kDisplay_type_triangles:		ids = IDS_DISPLAYTYPE_TRIANGLES;break;
	}
	nameSuffix = GetString(ids);
	return true;
}

ClassDesc* PAOperatorDisplay::GetClassDesc() const
{
	return GetPAOperatorDisplayDesc();
}

// here are the 256 default object colors
struct ObjectColors { BYTE	r, g, b; }; 
static ObjectColors objectColors[] = {
	// basic colors
		0xFF,0xB9,0xEF, 0xEE,0xFF,0xB9, 0xB9,0xFF,0xFF, 0xFD,0xAA,0xAA,
		0xF9,0x60,0x60, 0xC4,0x1D,0x1D, 0x96,0x07,0x07, 0xFE,0xCD,0xAB,
		0xFA,0x9F,0x61, 0xC5,0x62,0x1E, 0x96,0x42,0x09, 0xFE,0xEE,0xAB,
		0xFA,0xDD,0x61, 0xC5,0xA5,0x1E, 0x96,0x7B,0x09, 0xEE,0xFE,0xAB,
		0xDD,0xFA,0x61, 0xA5,0xC5,0x1E, 0x7E,0x96,0x07, 0xCD,0xFE,0xAB,
		0x9F,0xFA,0x61, 0x62,0xC5,0x1E, 0x44,0x96,0x07, 0xAB,0xFE,0xAB,
		0x61,0xFA,0x61, 0x1E,0xC5,0x1E, 0x07,0x96,0x07, 0xAB,0xFE,0xCD,
		0x61,0xFA,0x9F, 0x1E,0xC5,0x62, 0x07,0x96,0x41, 0xAB,0xFE,0xEE,
		0x61,0xFA,0xDD, 0x1E,0xC5,0xA5, 0x07,0x96,0x7E, 0xAC,0xEF,0xFF,
		0x62,0xDE,0xFB, 0x20,0xA6,0xC5, 0x09,0x7B,0x96, 0xAC,0xCE,0xFF,
		0x62,0xA0,0xFB, 0x20,0x63,0xC5, 0x09,0x44,0x9A, 0xAC,0xAC,0xFF,
		0x62,0x62,0xFB, 0x20,0x20,0xC5, 0x09,0x09,0x98, 0xCD,0xAD,0xFF,
		0x9C,0x62,0xFB, 0x5F,0x20,0xC5, 0x40,0x09,0x98, 0xED,0xAC,0xFF,
		0xDA,0x62,0xFB, 0xA2,0x20,0xC5, 0x79,0x09,0x98, 0xFF,0xAC,0xEF,
		0xFB,0x62,0xDE, 0xC5,0x20,0xA6, 0x9A,0x09,0x7B, 0xFE,0xAB,0xCD,
		0xFA,0x61,0x9F, 0xC5,0x1E,0x62, 0x9D,0x08,0x41, 0x60,0x60,0x60
};

DWORD GetNewObjectColor()
{
	int index = rand()%64 + 1;
	return RGB(objectColors[index].r, objectColors[index].g, objectColors[index].b);
}



} // end of namespace PartASM