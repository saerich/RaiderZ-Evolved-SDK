/**********************************************************************
 *<
	FILE:			PAOperatorCopy.cpp

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

#include "PAOperatorCopy.h"

#include "PAOperatorCopy_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPViewManager.h"
#include "PFMessages.h"
#include "IParticleChannelMult.h"

namespace PartASM {

PAOperatorCopy::PAOperatorCopy()
{ 
	GetClassDesc()->MakeAutoParamBlocks(this); 
}

FPInterfaceDesc* PAOperatorCopy::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &value_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &value_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &value_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PAOperatorCopy::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_VALUE_CLASS_NAME);
}

Class_ID PAOperatorCopy::ClassID()
{
	return PAOperatorCopy_Class_ID;
}

void PAOperatorCopy::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PAOperatorCopy::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefResult PAOperatorCopy::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
														PartID& partID, RefMessage message)
{
	switch(message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				switch(lastUpdateID) {
				case kCopy_srcChannel:
				case kCopy_srcName:
				case kCopy_dstChannel:
				case kCopy_dstName:
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
				case kCopy_srcType:
				case kCopy_dstType:
				case kCopy_useCondition:
					RefreshUI(kCopy_message_update);
					break;
				}
				UpdatePViewUI(lastUpdateID);
			}
			break;
	}

	return REF_SUCCEED;
}

RefTargetHandle PAOperatorCopy::Clone(RemapDir &remap)
{
	PAOperatorCopy* newOp = new PAOperatorCopy();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PAOperatorCopy::GetObjectName()
{
	return GetString(IDS_OPERATOR_COPY_OBJECT_NAME);
}

const ParticleChannelMask& PAOperatorCopy::ChannelsUsed(const Interval& time) const
{
								//  read						&write channels
	static ParticleChannelMask mask(PCU_New|PCU_Time|PCU_Speed|PCU_Amount, PCG_AngAxis|PCU_Spin);
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PAOperatorCopy::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_OPERATOR_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PAOperatorCopy::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_OPERATOR_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PAOperatorCopy::HasDynamicName(TSTR& nameSuffix)
{
	int srcChannel = pblock()->GetInt(kCopy_srcChannel, 0);
	TCHAR* srcName = pblock()->GetStr(kCopy_srcName, 0);
	int dstChannel = pblock()->GetInt(kCopy_dstChannel, 0);
	TCHAR* dstName = pblock()->GetStr(kCopy_dstName, 0);
	TSTR fromName, toName;
	switch (srcChannel) {
	case kCopy_data_age:			fromName = GetString(IDS_DATATYPE_AGE); break;
	case kCopy_data_localTime:		fromName = GetString(IDS_DATATYPE_TIME); break;
	case kCopy_data_birthTime:		fromName = GetString(IDS_DATATYPE_BIRTHTIME); break;
	case kCopy_data_custom:			fromName = srcName; break;
	case kCopy_data_deathTime:		fromName = GetString(IDS_DATATYPE_DEATHTIME); break;
	case kCopy_data_lifespan:		fromName = GetString(IDS_DATATYPE_LIFESPAN); break;
	case kCopy_data_mapChannel1:	fromName = GetString(IDS_DATATYPE_MAPCHANNEL1); break;
	case kCopy_data_mapChannel2:	fromName = GetString(IDS_DATATYPE_MAPCHANNEL2); break;
	case kCopy_data_mapChannel3:	fromName = GetString(IDS_DATATYPE_MAPCHANNEL3); break;
	case kCopy_data_mapChannel4:	fromName = GetString(IDS_DATATYPE_MAPCHANNEL4); break;
	case kCopy_data_mapChannel5:	fromName = GetString(IDS_DATATYPE_MAPCHANNEL5); break;
	case kCopy_data_mtlIndex:		fromName = GetString(IDS_DATATYPE_MTLINDEX); break;
	case kCopy_data_particleID:		fromName = GetString(IDS_DATATYPE_PARTICLEID); break;
	case kCopy_data_position:		fromName = GetString(IDS_DATATYPE_POSITION); break;
	case kCopy_data_rotationAngle:	fromName = GetString(IDS_DATATYPE_ROTATIONANGLE); break;
	case kCopy_data_rotationAxis:	fromName = GetString(IDS_DATATYPE_ROTATIONAXIS); break;
	case kCopy_data_euclideRotation:fromName = GetString(IDS_DATATYPE_EUCLIDEROTATION); break;
	case kCopy_data_scale:			fromName = GetString(IDS_DATATYPE_SCALE); break;
	case kCopy_data_scriptFloat:	fromName = GetString(IDS_DATATYPE_SCRIPTFLOAT); break;
	case kCopy_data_scriptInteger:	fromName = GetString(IDS_DATATYPE_SCRIPTINTEGER); break;
	case kCopy_data_scriptMatrixX:	fromName = GetString(IDS_DATATYPE_SCRIPTMATRIXX); break;
	case kCopy_data_scriptMatrixY:	fromName = GetString(IDS_DATATYPE_SCRIPTMATRIXY); break;
	case kCopy_data_scriptMatrixZ:	fromName = GetString(IDS_DATATYPE_SCRIPTMATRIXZ); break;
	case kCopy_data_scriptMatrixT:	fromName = GetString(IDS_DATATYPE_SCRIPTMATRIXT); break;
	case kCopy_data_scriptVector:	fromName = GetString(IDS_DATATYPE_SCRIPTVECTOR); break;
	case kCopy_data_selected:		fromName = GetString(IDS_DATATYPE_SELECTED); break;
	case kCopy_data_size:			fromName = GetString(IDS_DATATYPE_SIZE); break;
	case kCopy_data_shapeFaces:		fromName = GetString(IDS_DATATYPE_SHAPEFACES); break;
	case kCopy_data_shapeSurface:	fromName = GetString(IDS_DATATYPE_SHAPESURFACE); break;
	case kCopy_data_shapeVertices:	fromName = GetString(IDS_DATATYPE_SHAPEVERTICES); break;
	case kCopy_data_shapeVolume:	fromName = GetString(IDS_DATATYPE_SHAPEVOLUME); break;
	case kCopy_data_speed:			fromName = GetString(IDS_DATATYPE_SPEED); break;
	case kCopy_data_spinAxis:		fromName = GetString(IDS_DATATYPE_SPINAXIS); break;
	case kCopy_data_spinRate:		fromName = GetString(IDS_DATATYPE_SPINRATE); break;
	case kCopy_data_timeInEvent:	fromName = GetString(IDS_DATATYPE_TIMEINEVENT); break;
	}

	switch (dstChannel) {
	case kCopy_data_age:			toName = GetString(IDS_DATATYPE_AGE); break;
	case kCopy_data_birthTime:		toName = GetString(IDS_DATATYPE_BIRTHTIME); break;
	case kCopy_data_custom:			toName = dstName; break;
	case kCopy_data_deathTime:		toName = GetString(IDS_DATATYPE_DEATHTIME); break;
	case kCopy_data_lifespan:		toName = GetString(IDS_DATATYPE_LIFESPAN); break;
	case kCopy_data_mapChannel1:	toName = GetString(IDS_DATATYPE_MAPCHANNEL1); break;
	case kCopy_data_mapChannel2:	toName = GetString(IDS_DATATYPE_MAPCHANNEL2); break;
	case kCopy_data_mapChannel3:	toName = GetString(IDS_DATATYPE_MAPCHANNEL3); break;
	case kCopy_data_mapChannel4:	toName = GetString(IDS_DATATYPE_MAPCHANNEL4); break;
	case kCopy_data_mapChannel5:	toName = GetString(IDS_DATATYPE_MAPCHANNEL5); break;
	case kCopy_data_mtlIndex:		toName = GetString(IDS_DATATYPE_MTLINDEX); break;
	case kCopy_data_position:		toName = GetString(IDS_DATATYPE_POSITION); break;
	case kCopy_data_rotationAngle:	toName = GetString(IDS_DATATYPE_ROTATIONANGLE); break;
	case kCopy_data_rotationAxis:	toName = GetString(IDS_DATATYPE_ROTATIONAXIS); break;
	case kCopy_data_euclideRotation:toName = GetString(IDS_DATATYPE_EUCLIDEROTATION); break;
	case kCopy_data_scale:			toName = GetString(IDS_DATATYPE_SCALE); break;
	case kCopy_data_scriptFloat:	toName = GetString(IDS_DATATYPE_SCRIPTFLOAT); break;
	case kCopy_data_scriptInteger:	toName = GetString(IDS_DATATYPE_SCRIPTINTEGER); break;
	case kCopy_data_scriptMatrixX:	toName = GetString(IDS_DATATYPE_SCRIPTMATRIXX); break;
	case kCopy_data_scriptMatrixY:	toName = GetString(IDS_DATATYPE_SCRIPTMATRIXY); break;
	case kCopy_data_scriptMatrixZ:	toName = GetString(IDS_DATATYPE_SCRIPTMATRIXZ); break;
	case kCopy_data_scriptMatrixT:	toName = GetString(IDS_DATATYPE_SCRIPTMATRIXT); break;
	case kCopy_data_scriptVector:	toName = GetString(IDS_DATATYPE_SCRIPTVECTOR); break;
	case kCopy_data_selected:		toName = GetString(IDS_DATATYPE_SELECTED); break;
	case kCopy_data_size:			toName = GetString(IDS_DATATYPE_SIZE); break;
	case kCopy_data_shapeFaces:		toName = GetString(IDS_DATATYPE_SHAPEFACES); break;
	case kCopy_data_shapeSurface:	toName = GetString(IDS_DATATYPE_SHAPESURFACE); break;
	case kCopy_data_shapeVertices:	toName = GetString(IDS_DATATYPE_SHAPEVERTICES); break;
	case kCopy_data_shapeVolume:	toName = GetString(IDS_DATATYPE_SHAPEVOLUME); break;
	case kCopy_data_speed:			toName = GetString(IDS_DATATYPE_SPEED); break;
	case kCopy_data_spinAxis:		toName = GetString(IDS_DATATYPE_SPINAXIS); break;
	case kCopy_data_spinRate:		toName = GetString(IDS_DATATYPE_SPINRATE); break;
	case kCopy_data_timeInEvent:	toName = GetString(IDS_DATATYPE_TIMEINEVENT); break;
	}

	nameSuffix = fromName + TSTR("->") + toName;
	return true;
}

float FaceArea(Mesh* mesh, int faceIndex, Point3 scale)
{
	DbgAssert(mesh);
	DbgAssert(faceIndex >= 0);
	DbgAssert(faceIndex < mesh->getNumFaces());
	if (IsBadFace(mesh, faceIndex)) return 0.0f;

	float resValue = 0.0f;
	Point3 vertex[3];
	for(int i=0; i<3; i++)
		vertex[i] = mesh->verts[mesh->faces[faceIndex].v[i]];
	Point3 a = vertex[1] - vertex[0];
	a = Point3(a.x*scale.x, a.y*scale.y, a.z*scale.z);
	Point3 b = vertex[2] - vertex[0];
	b = Point3(b.x*scale.x, b.y*scale.y, b.z*scale.z);
	resValue = 0.5f*Length(CrossProd(a, b));
	return resValue;
}

float GetMeshSurface(Mesh* mesh, Point3 scale)
{
	double sum = 0;
	for(int i=0; i<mesh->numFaces; i++)
		sum += FaceArea(mesh, i, scale);
	return float(sum);
}

float GetMeshVolume(Mesh* mesh, Point3 scale)
{
	float v = MeshVolume(mesh);
	v *= scale.x*scale.y*scale.z;
	return v;
}

bool PAOperatorCopy::Proceed(IObject* pCont,
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

	TimeValue curT = timeStart.TimeValue();
	int srcData = pblock()->GetInt(kCopy_srcChannel, curT);
	TCHAR* srcName = pblock()->GetStr(kCopy_srcName, curT);
	int srcType = pblock()->GetInt(kCopy_srcType, curT);
	int dstData = pblock()->GetInt(kCopy_dstChannel, curT);
	TCHAR* dstName = pblock()->GetStr(kCopy_dstName, curT);
	int dstType = pblock()->GetInt(kCopy_dstType, curT);
	int scopeType = pblock()->GetInt(kCopy_dstScope, curT);
	bool newOnly = (pblock()->GetInt(kCopy_assignOnce, curT) != 0);
	bool useCondition = (pblock()->GetInt(kCopy_useCondition, curT) != 0);

	IParticleChannelMult* chMult = NULL;
	if (useCondition || (srcData == kCopy_data_custom) || (dstData == kCopy_data_custom)) {
		chMult = (IParticleChannelMult*)chCont->EnsureInterface(PARTICLECHANNELMULT_INTERFACE,
																		ParticleChannelMult_Class_ID,
																		true, PARTICLECHANNELMULT_INTERFACE,
																		PARTICLECHANNELMULT_INTERFACE, true,
																		actionNode, NULL);
		if (chMult == NULL) return false; // can't create/modify/read the main data channel
	}

	IParticleChannelBoolR* chFilter = NULL;
	if (useCondition) {
		TCHAR* filterName = pblock()->GetStr(kCopy_conditionName, 0);
		int filterChannelIndex;
		IObject* filterData = chMult->GetSubChannel(filterName, ParticleChannelBool_Class_ID, filterChannelIndex);
		if (filterData != NULL)
			chFilter = (IParticleChannelBoolR*)filterData->GetInterface(PARTICLECHANNELBOOLR_INTERFACE);
		if (chFilter == NULL) useCondition = false;
	}

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

	Tab<bool> boolData;
	Tab<int> intData;
	Tab<float> floatData;
	Tab<PreciseTimeValue> timeData;
	Tab<Point3> vectorData;

	float floatValue, scaleFactor;
	Point3 vectorValue;

	int multChannelIndex;
	IObject* channelObj = NULL;
	IParticleChannelBoolR* chBoolR = NULL;
	IParticleChannelBoolW* chBoolW = NULL;
	IParticleChannelPTVR* chPTVR = NULL;
	IParticleChannelPTVW* chPTVW = NULL;
	IParticleChannelIntR* chIntR = NULL;
	IParticleChannelIntW* chIntW = NULL;
	IParticleChannelFloatR* chFloatR = NULL;
	IParticleChannelFloatW* chFloatW = NULL;
	IParticleChannelPoint3R* chPoint3R = NULL;
	IParticleChannelPoint3W* chPoint3W = NULL;
	bool initEventStart = false;
	IParticleChannelPTVR* chEventStartR = NULL;
	IParticleChannelPTVW* chEventStartW = NULL;
	IParticleChannelPTVR* chDeathTimeR = NULL;
	IParticleChannelPTVW* chDeathTimeW = NULL;
	IParticleChannelPTVR* chLifespanR = NULL;
	IParticleChannelPTVW* chLifespanW = NULL;
	IParticleChannelMeshMapR* chShapeTextureR = NULL;
	IParticleChannelMeshMapW* chShapeTextureW = NULL;
	IParticleChannelMapR* chMapR = NULL;
	IParticleChannelMapW* chMapW = NULL;
	IParticleChannelQuatR* chQuatR = NULL;
	IParticleChannelQuatW* chQuatW = NULL;
	IParticleChannelMatrix3R* chMatrixR = NULL;
	IParticleChannelMatrix3W* chMatrixW = NULL;
	IParticleChannelMeshR* chMesh = NULL;
	IParticleChannelAngAxisR* chAngAxisR = NULL;
	IParticleChannelAngAxisW* chAngAxisW = NULL;

	switch(srcData) {
	case kCopy_data_custom:
		switch(srcType) {
		case kCopy_type_boolean:
			channelObj = chMult->GetSubChannel(srcName, ParticleChannelBool_Class_ID, multChannelIndex);
			if (channelObj != NULL)
				chBoolR = (IParticleChannelBoolR*)channelObj->GetInterface(PARTICLECHANNELBOOLR_INTERFACE);
			if (chBoolR == NULL) return false;
			boolData.SetCount(count);
			for(i=0; i<count; i++)
				if (processValue[i])
					boolData[i] = chBoolR->GetValue(i);
			break;
		case kCopy_type_time:
			channelObj = chMult->GetSubChannel(srcName, ParticleChannelPTV_Class_ID, multChannelIndex);
			if (channelObj != NULL)
				chPTVR = (IParticleChannelPTVR*)channelObj->GetInterface(PARTICLECHANNELPTVR_INTERFACE);
			if (chPTVR == NULL) return false;
			timeData.SetCount(count);
			for(i=0; i<count; i++)
				if (processValue[i])
					timeData[i] = chPTVR->GetValue(i);
			break;
		case kCopy_type_integer:
			channelObj = chMult->GetSubChannel(srcName, ParticleChannelInt_Class_ID, multChannelIndex);
			if (channelObj != NULL)
				chIntR = (IParticleChannelIntR*)channelObj->GetInterface(PARTICLECHANNELINTR_INTERFACE);
			if (chIntR == NULL) return false;
			intData.SetCount(count);
			for(i=0; i<count; i++)
				if (processValue[i])
					intData[i] = chIntR->GetValue(i);
			break;
		case kCopy_type_float:
		case kCopy_type_percent:
		case kCopy_type_worldFloat:
		case kCopy_type_angle:
			channelObj = chMult->GetSubChannel(srcName, ParticleChannelFloat_Class_ID, multChannelIndex);
			if (channelObj != NULL)
				chFloatR = (IParticleChannelFloatR*)channelObj->GetInterface(PARTICLECHANNELFLOATR_INTERFACE);
			if (chFloatR == NULL) return false;
			floatData.SetCount(count);
			for(i=0; i<count; i++)
				if (processValue[i])
					floatData[i] = chFloatR->GetValue(i);
			break;
		case kCopy_type_floatVector:
		case kCopy_type_percentVector:
		case kCopy_type_worldVector:
		case kCopy_type_angleVector:
			channelObj = chMult->GetSubChannel(srcName, ParticleChannelPoint3_Class_ID, multChannelIndex);
			if (channelObj != NULL)
				chPoint3R = (IParticleChannelPoint3R*)channelObj->GetInterface(PARTICLECHANNELPOINT3R_INTERFACE);
			if (chPoint3R == NULL) return false;
			vectorData.SetCount(count);
			for(i=0; i<count; i++)
				if (processValue[i])
					vectorData[i] = chPoint3R->GetValue(i);
			break;
		}
		break;

	case kCopy_data_localTime:
		timeData.SetCount(count);
		for(i=0; i<count; i++)
			if (processValue[i])
				timeData[i] = chTime->GetValue(i);
		srcType = kCopy_type_time;
		break;

	case kCopy_data_age:
	case kCopy_data_birthTime:
		chPTVR = GetParticleChannelBirthTimeRInterface(pCont);
		if (chPTVR == NULL) return false;
		timeData.SetCount(count);
		if (srcData == kCopy_data_age) {
			for(i=0; i<count; i++)
				if (processValue[i])
					timeData[i] = chTime->GetValue(i) - chPTVR->GetValue(i);
		} else {
			for(i=0; i<count; i++)
				if (processValue[i])
					timeData[i] = chPTVR->GetValue(i);
		}
		srcType = kCopy_type_time;
		break;

	case kCopy_data_timeInEvent:
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
		timeData.SetCount(count);
		for(i=0; i<count; i++)
			if (processValue[i])
				timeData[i] = chTime->GetValue(i) - chEventStartR->GetValue(i);
		srcType = kCopy_type_time;
		break;

	case kCopy_data_deathTime:
	case kCopy_data_lifespan:
		chPTVR = GetParticleChannelBirthTimeRInterface(pCont);
		chDeathTimeR = GetParticleChannelDeathTimeRInterface(pCont);
		chLifespanR = GetParticleChannelLifespanRInterface(pCont);
		if (chPTVR == NULL) return false;
		if ((chDeathTimeR == NULL) && (chLifespanR == NULL)) return false;
		timeData.SetCount(count);
		if (srcData == kCopy_data_deathTime) {
			if (chLifespanR != NULL) {
				for(i=0; i<count; i++)
					if (processValue[i])
						timeData[i] = chPTVR->GetValue(i) + chLifespanR->GetValue(i);
			} else {
				for(i=0; i<count; i++)
					if (processValue[i])
						timeData[i] = chDeathTimeR->GetValue(i);
			}
		} else {
			if (chLifespanR != NULL) {
				for(i=0; i<count; i++)
					if (processValue[i])
						timeData[i] = chLifespanR->GetValue(i);
			} else {
				for(i=0; i<count; i++)
					if (processValue[i])
						timeData[i] = chDeathTimeR->GetValue(i) - chPTVR->GetValue(i);
			}
		}
		srcType = kCopy_type_time;
		break;

	case kCopy_data_mapChannel1:
	case kCopy_data_mapChannel2:
	case kCopy_data_mapChannel3:
	case kCopy_data_mapChannel4:
	case kCopy_data_mapChannel5:
		chShapeTextureR = GetParticleChannelShapeTextureRInterface(pCont);
		if (chShapeTextureR != NULL) {
			int mapChannel = 1 + srcData - kCopy_data_mapChannel1;
			if (chShapeTextureR->MapSupport(mapChannel))
				chMapR = chShapeTextureR->GetMapReadChannel(mapChannel);
		}
		if (chMapR == NULL) return false;
		vectorData.SetCount(count);
		for(i=0; i<count; i++) {
			if (processValue[i]) {
				if ((chMapR->GetUVVert(i))->Count() == 1)
					vectorData[i] = (*(chMapR->GetUVVert(i)))[0];
				else
					vectorData[i] = Point3::Origin;
			}
		}
		srcType = kCopy_type_floatVector;				
		break;

	case kCopy_data_mtlIndex:
		chIntR = GetParticleChannelMtlIndexRInterface(pCont);
		if (chIntR == NULL) return false;
		intData.SetCount(count);
		for(i=0; i<count; i++)
			if (processValue[i])
				intData[i] = chIntR->GetValue(i);
		srcType = kCopy_type_integer;
		break;

	case kCopy_data_position:
	case kCopy_data_speed:
		if (srcData == kCopy_data_position)
			chPoint3R = GetParticleChannelPositionRInterface(pCont);
		else
			chPoint3R = GetParticleChannelSpeedRInterface(pCont);
		if (chPoint3R == NULL) return false;
		vectorData.SetCount(count);
		for(i=0; i<count; i++) 
			if(processValue[i])
				vectorData[i] = chPoint3R->GetValue(i);
		srcType = kCopy_type_worldVector;
		break;

	case kCopy_data_rotationAngle:
		chQuatR = GetParticleChannelOrientationRInterface(pCont);
		if (chQuatR == NULL) return false;
		floatData.SetCount(count);
		for(i=0; i<count; i++) {
			if (processValue[i]) {
				AngAxisFromQ(chQuatR->GetValue(i), &floatValue, vectorValue);
				floatData[i] = floatValue;
			}
		}
		srcType = kCopy_type_angle;
		break;

	case kCopy_data_rotationAxis:
		chQuatR = GetParticleChannelOrientationRInterface(pCont);
		if (chQuatR == NULL) return false;
		vectorData.SetCount(count);
		for(i=0; i<count; i++) {
			if (processValue[i]) {
				AngAxisFromQ(chQuatR->GetValue(i), &floatValue, vectorValue);
				vectorData[i] = vectorValue;
			}
		}
		srcType = kCopy_type_floatVector;
		break;

	case kCopy_data_euclideRotation:
		chQuatR = GetParticleChannelOrientationRInterface(pCont);
		if (chQuatR == NULL) return false;
		vectorData.SetCount(count);
		for(i=0; i<count; i++) {
			if (processValue[i]) {
				Quat curQ = chQuatR->GetValue(i);
				QuatToEuler(curQ, vectorValue);
				vectorData[i] = vectorValue;
			}
		}
		srcType = kCopy_type_angleVector;
		break;

	case kCopy_data_scale:
		chPoint3R = GetParticleChannelScaleRInterface(pCont);
		if (chPoint3R == NULL) return false;
		vectorData.SetCount(count);
		for(i=0; i<count; i++)
			if (processValue[i])
				vectorData[i] = chPoint3R->GetValue(i);
		srcType = kCopy_type_percentVector;
		break;

	case kCopy_data_scriptFloat:
		chFloatR = GetParticleChannelMXSFloatRInterface(pCont);
		if (chFloatR == NULL) return false;
		floatData.SetCount(count);
		for(i=0; i<count; i++)
			if (processValue[i])
				floatData[i] = chFloatR->GetValue(i);
		srcType =kCopy_type_float;
		break;

	case kCopy_data_scriptInteger:
		chIntR = GetParticleChannelMXSIntegerRInterface(pCont);
		if (chIntR == NULL) return false;
		intData.SetCount(count);
		for(i=0; i<count; i++)
			if (processValue[i])
				intData[i] = chIntR->GetValue(i);
		srcType =kCopy_type_integer;
		break;

	case kCopy_data_scriptMatrixX:
	case kCopy_data_scriptMatrixY:
	case kCopy_data_scriptMatrixZ:
	case kCopy_data_scriptMatrixT: {
		chMatrixR = GetParticleChannelMXSMatrixRInterface(pCont);
		if (chMatrixR == NULL) return false;
		int rowIndex = srcData - kCopy_data_scriptMatrixX;
		vectorData.SetCount(count);
		for(i=0; i<count; i++)
			if (processValue[i])
				vectorData[i] = (chMatrixR->GetValue(i)).GetRow(rowIndex);
		srcType = kCopy_type_floatVector;
								   }
		break;

	case kCopy_data_scriptVector:
		chPoint3R = GetParticleChannelMXSVectorRInterface(pCont);
		if (chPoint3R == NULL) return false;
		vectorData.SetCount(count);
		for(i=0; i<count; i++)
			if (processValue[i])
				vectorData[i] = chPoint3R->GetValue(i);
		srcType = kCopy_type_floatVector;
		break;

	case kCopy_data_selected:
		chBoolR = GetParticleChannelSelectionRInterface(pCont);
		if (chBoolR == NULL) return false;
		boolData.SetCount(count);
		for(i=0; i<count; i++)
			if (processValue[i])
				boolData[i] = chBoolR->GetValue(i);
		srcType = kCopy_type_boolean;
		break;

	case kCopy_data_size:
		chMesh = GetParticleChannelShapeRInterface(pCont);
		chPoint3R = GetParticleChannelScaleRInterface(pCont);
		if (chMesh == NULL) return false;
		vectorData.SetCount(count);
		for(i=0; i<count; i++) {
			if (processValue[i]) {
				Mesh* curShape = const_cast <Mesh*>(chMesh->GetValue(i));
				Box3 bbox;
				bbox.pmin = bbox.pmax = Point3::Origin;
				if (curShape != NULL) bbox = curShape->getBoundingBox();
				Point3 scale = Point3(1.0f, 1.0f, 1.0f);
				if (chPoint3R) scale = chPoint3R->GetValue(i);
				vectorData[i] = Point3(scale.x*(bbox.pmax.x-bbox.pmin.x), scale.y*(bbox.pmax.y-bbox.pmin.y), scale.z*(bbox.pmax.z-bbox.pmin.z));
			}
		}
		srcType = kCopy_type_worldVector;
		break;

	case kCopy_data_shapeFaces:
	case kCopy_data_shapeVertices:
		chMesh = GetParticleChannelShapeRInterface(pCont);
		if (chMesh == NULL) return false;
		intData.SetCount(count);
		for(i=0; i<count; i++) {
			if (processValue[i]) {
				Mesh* curShape = const_cast <Mesh*>(chMesh->GetValue(i));
				if (srcData == kCopy_data_shapeFaces)
					intData[i] = (curShape != NULL) ? curShape->numFaces : 0;
				else
					intData[i] = (curShape != NULL) ? curShape->numVerts : 0;
			}
		}
		srcType = kCopy_type_integer;
		break;

	case kCopy_data_particleID: {
		IParticleChannelIDR* chID = GetParticleChannelIDRInterface(pCont);
		if (chID == NULL) return false;
		intData.SetCount(count);
		for(i=0; i<count; i++) {
			if (processValue[i]) intData[i] = chID->GetParticleBorn(i) + 1;
		}
		srcType = kCopy_type_integer;
								}
		break;

	case kCopy_data_shapeSurface:
	case kCopy_data_shapeVolume:
		chMesh = GetParticleChannelShapeRInterface(pCont);
		chPoint3R = GetParticleChannelScaleRInterface(pCont);
		if (chMesh == NULL) return false;
		floatData.SetCount(count);
		for(i=0; i<count; i++) {
			if (processValue[i]) {
				Mesh* curShape = const_cast <Mesh*>(chMesh->GetValue(i));
				floatValue = 0.0f;
				if (curShape != NULL) {
					Point3 scale = Point3(1.0f, 1.0f, 1.0f);
					if (chPoint3R) scale = chPoint3R->GetValue(i);
					if (srcData == kCopy_data_shapeSurface)
						floatValue = GetMeshSurface(curShape, scale);
					else
						floatValue = GetMeshVolume(curShape, scale);
				}
				floatData[i] = floatValue;
			}
		}
		srcType = kCopy_type_float;
		break;

	case kCopy_data_spinAxis:
	case kCopy_data_spinRate:
		chAngAxisR = GetParticleChannelSpinRInterface(pCont);
		if (chAngAxisR == NULL) return false;
		else {
			AngAxis aa;
			if (srcData == kCopy_data_spinAxis) {
				vectorData.SetCount(count);
				for(i=0; i<count; i++) {
					if (processValue[i]) {
						aa = chAngAxisR->GetValue(i);
						vectorData[i] = aa.axis;
					}
				}
				srcType = kCopy_type_floatVector;
			} else {
				floatData.SetCount(count);
				for(i=0; i<count; i++) {
					if (processValue[i]) {
						aa = chAngAxisR->GetValue(i);
						floatData[i] = aa.angle;
					}
				}
				srcType = kCopy_type_angle;
			}
		}
		break;
	}

	// clarify type of the destination channel
	switch(dstData) {
	case kCopy_data_age: case kCopy_data_deathTime:
	case kCopy_data_lifespan: case kCopy_data_timeInEvent:
		dstType = kCopy_type_time;
		break;
	case kCopy_data_mapChannel1: case kCopy_data_mapChannel2:
	case kCopy_data_mapChannel3: case kCopy_data_mapChannel4:
	case kCopy_data_mapChannel5: case kCopy_data_scriptVector:
	case kCopy_data_scriptMatrixX: case kCopy_data_scriptMatrixY:
	case kCopy_data_scriptMatrixZ: case kCopy_data_scriptMatrixT:
	case kCopy_data_rotationAxis: case kCopy_data_spinAxis:
		dstType = kCopy_type_floatVector;
		break;
	case kCopy_data_mtlIndex: case kCopy_data_scriptInteger:
		dstType = kCopy_type_integer;
		break;
	case kCopy_data_position: case kCopy_data_speed:
		dstType = kCopy_type_worldVector;
		break;
	case kCopy_data_rotationAngle: case kCopy_data_spinRate:
		dstType = kCopy_type_angle;
		break;
	case kCopy_data_euclideRotation:
		dstType = kCopy_type_angleVector;
		break;
	case kCopy_data_scale:
		dstType = kCopy_type_percentVector;
		break;
	case kCopy_data_scriptFloat:
		dstType = kCopy_type_float;
		break;
	case kCopy_data_selected:
		dstType = kCopy_type_boolean;
		break;
	}

	// rid off worldUnit type; it's equivalent to a simple float type for out purposes
	if (srcType == kCopy_type_worldFloat) srcType = kCopy_type_float;
	if (srcType == kCopy_type_worldVector) srcType = kCopy_type_floatVector;
	if (dstType == kCopy_type_worldFloat) dstType = kCopy_type_float;
	if (dstType == kCopy_type_worldVector) dstType = kCopy_type_floatVector;

	// rid off src boolean type if we can; upscale it to integer type
	if ((srcType == kCopy_type_boolean) && (dstType != kCopy_type_boolean)) {
		intData.SetCount(count);
		for(i=0; i<count; i++)
			if (processValue[i])
				intData[i] = boolData[i] ? 1 : 0;
		srcType = kCopy_type_integer;
	}

	// rid off src integer type if we can; upscale it to float type
	if ((srcType == kCopy_type_integer) && (dstType != kCopy_type_integer) && (dstType != kCopy_type_boolean)) {
		floatData.SetCount(count);
		for(i=0; i<count; i++)
			if (processValue[i])
				floatData[i] = float(intData[i]);
		srcType = kCopy_type_float;
	}

	int scalarToVectorType = pblock()->GetInt(kCopy_scalarToVectorType, curT);
	int floatToIntegerType = pblock()->GetInt(kCopy_floatToIntegerType, curT);
	int floatToTimeType = pblock()->GetInt(kCopy_floatToTimeType, curT);
	int floatToPercentType = pblock()->GetInt(kCopy_floatToPercentType, curT);
	int floatToAngleType = pblock()->GetInt(kCopy_floatToAngleType, curT);

	// rid off src time type if we can; upscale it to float type
	if ((srcType == kCopy_type_time) && (dstType != kCopy_type_time)) {
		floatData.SetCount(count);
		float scaleFactor = 1.0f;
		if (floatToTimeType == kCopy_ftType_sec) scaleFactor = 1.0f/TIME_TICKSPERSEC;
		if (floatToTimeType == kCopy_ftType_frame) scaleFactor = 1.0f/GetTicksPerFrame();
		for(i=0; i<count; i++)
			if (processValue[i])
				floatData[i] = scaleFactor*(float(timeData[i].tick) + timeData[i].fraction);
		srcType = kCopy_type_float;
	}

	// rid off src percent type if we can; convert it to float
	if ((srcType == kCopy_type_percent) && (dstType != kCopy_type_percent) && (dstType != kCopy_type_percentVector)) {
		if (floatToPercentType == kCopy_fpType_factor1) {
			for(i=0; i<count; i++)
				if (processValue[i])
					floatData[i] *= 100.0f;
		}
		srcType = kCopy_type_float;
	}

	// rid off src angle type if we can; convert it to float
	if ((srcType == kCopy_type_angle) && (dstType != kCopy_type_angle) && (dstType != kCopy_type_angleVector)) {
		if (floatToAngleType == kCopy_faType_degrees) {
			for(i=0; i<count; i++)
				if (processValue[i])
					floatData[i] *= RAD_TO_DEG;
		}
		srcType = kCopy_type_float;
	}

	// rid off src percentVector type if we can; convert it to floatVector
	if ((srcType == kCopy_type_percentVector) && (dstType != kCopy_type_percent) && (dstType != kCopy_type_percentVector)) {
		if (floatToPercentType == kCopy_fpType_factor1) {
			for(i=0; i<count; i++)
				if (processValue[i])
					vectorData[i] *= 100.0f;
		}
		srcType = kCopy_type_floatVector;
	}

	// rid off src angleVector type if we can; convert it to floatVector
	if ((srcType == kCopy_type_angleVector) && (dstType != kCopy_type_angle) && (dstType != kCopy_type_angleVector)) {
		if (floatToAngleType == kCopy_faType_degrees) {
			for(i=0; i<count; i++)
				if (processValue[i])
					vectorData[i] *= RAD_TO_DEG;
		}
		srcType = kCopy_type_floatVector;
	}

	// rid off floatVector type if we can; convert it float
	// or convert percentVector to percent or angleVector to angle
	if (((srcType == kCopy_type_floatVector) && (dstType < kCopy_type_floatVector)) ||
		((srcType == kCopy_type_percentVector) && (dstType == kCopy_type_percent)) ||
		((srcType == kCopy_type_angleVector) && (dstType == kCopy_type_angle))) {
		floatData.SetCount(count);
		switch (scalarToVectorType) {
		case kCopy_svType_x: case kCopy_svType_xyz:
			for(i=0; i<count; i++)
				if (processValue[i])
					floatData[i] = vectorData[i].x;
			break;
		case kCopy_svType_y:
			for(i=0; i<count; i++)
				if (processValue[i])
					floatData[i] = vectorData[i].y;
			break;
		case kCopy_svType_z:
			for(i=0; i<count; i++)
				if (processValue[i])
					floatData[i] = vectorData[i].z;
			break;
		case kCopy_svType_length:
			for(i=0; i<count; i++)
				if (processValue[i])
					floatData[i] = Length(vectorData[i]);
			break;
		case kCopy_svType_average:
			for(i=0; i<count; i++)
				if (processValue[i])
					floatData[i] = (vectorData[i].x+vectorData[i].y+vectorData[i].z)/3.0f;
			break;
		case kCopy_svType_minimum:
			for(i=0; i<count; i++)
				if (processValue[i])
					floatData[i] = min(vectorData[i].x, min(vectorData[i].y, vectorData[i].z));
			break;
		case kCopy_svType_maximum:
			for(i=0; i<count; i++)
				if (processValue[i])
					floatData[i] = max(vectorData[i].x, max(vectorData[i].y, vectorData[i].z));
			break;
		}
		srcType = kCopy_type_float;
	}

	// convert data
	switch(dstType) {
	case kCopy_type_boolean:
		switch(srcType) {
		case kCopy_type_integer:
			boolData.SetCount(count);
			for(i=0; i<count; i++) 
				if (processValue[i])
					boolData[i] = (intData[i] != 0) ? true : false;
			break;
		case kCopy_type_float:
			boolData.SetCount(count);
			switch(floatToIntegerType) {
			case kCopy_fiType_up:
				for(i=0; i<count; i++)
					if (processValue[i])
						boolData[i] = (floatData[i] == 0.0f) ? false : true;
				break;
			case kCopy_fiType_down:
				for(i=0; i<count; i++)
					if (processValue[i])
						boolData[i] = (fabs(floatData[i]) < 1.0f) ? false : true;
				break;
			case kCopy_fiType_round:
				for(i=0; i<count; i++)
					if (processValue[i])
						boolData[i] = (fabs(floatData[i]) < 0.5f) ? false : true;
				break;
			}
			break;
		}
		break;

	case kCopy_type_time:
		switch(srcType) {
		case kCopy_type_float:
			timeData.SetCount(count);
			float scaleFactor = 1.0f;
			switch(floatToTimeType) {
			case kCopy_ftType_sec:
				scaleFactor = TIME_TICKSPERSEC;
				break;
			case kCopy_ftType_frame:
				scaleFactor = GetTicksPerFrame();
				break;
			}
			for(i=0; i<count; i++)
				if (processValue[i])
					timeData[i] = PreciseTimeValue(floatData[i]*scaleFactor);
			break;
		}
		break;

	case kCopy_type_integer:
		switch(srcType) {
		case kCopy_type_float:
			intData.SetCount(count);
			switch(floatToIntegerType) {
			case kCopy_fiType_up:
				for(i=0; i<count; i++)
					if (processValue[i])
						if (floatData[i] >= 0.0f)
							intData[i] = int(ceil(floatData[i]));
				break;
			case kCopy_fiType_down:
				for(i=0; i<count; i++)
					if (processValue[i])
						intData[i] = int(floor(floatData[i]));
				break;
			case kCopy_fiType_round:
				for(i=0; i<count; i++)
					if (processValue[i])
						intData[i] = int(floor(floatData[i]+0.5f));
				break;
			}
			break;
		}
		break;

	case kCopy_type_percent:
		switch(srcType) {
		case kCopy_type_float:
			if (floatToPercentType == kCopy_fpType_factor1)
				for(i=0; i<count; i++)
					if (processValue[i])
						floatData[i] *= 0.01f;
			break;
		}
		break;

	case kCopy_type_angle:
		switch(srcType) {
		case kCopy_type_float:
			if (floatToAngleType == kCopy_faType_degrees)
				for(i=0; i<count; i++)
					if (processValue[i])
						floatData[i] *= RAD_TO_DEG;
			break;
		}
		break;

	case kCopy_type_floatVector:
		switch(srcType) {
		case kCopy_type_float:
			vectorData.SetCount(count);
			for(i=0; i<count; i++)
				if (processValue[i])
					vectorData[i] = Point3(floatData[i], floatData[i], floatData[i]);
			break;
		}
		break;

	case kCopy_type_percentVector:
		switch(srcType) {
		case kCopy_type_float:
			vectorData.SetCount(count);
			scaleFactor = (floatToPercentType == kCopy_fpType_factor1) ? 0.01f : 1.0f;
			for(i=0; i<count; i++)
				if (processValue[i])
					vectorData[i] = scaleFactor*Point3(floatData[i], floatData[i], floatData[i]);
			break;
		case kCopy_type_percent:
			vectorData.SetCount(count);
			for(i=0; i<count; i++)
				if (processValue[i])
					vectorData[i] = Point3(floatData[i], floatData[i], floatData[i]);
			break;
		case kCopy_type_floatVector:
			if (floatToPercentType == kCopy_fpType_factor1) {
				for(i=0; i<count; i++)
					if (processValue[i])
						vectorData[i] *= 0.01f;
			}
			break;
		}
		break;

	case kCopy_type_angleVector:
		switch(srcType) {
		case kCopy_type_float:
			vectorData.SetCount(count);
			scaleFactor = (floatToAngleType == kCopy_faType_degrees) ? RAD_TO_DEG : 1.0f;
			for(i=0; i<count; i++)
				if (processValue[i])
					vectorData[i] = scaleFactor*Point3(floatData[i], floatData[i], floatData[i]);
			break;
		case kCopy_type_angle:
			vectorData.SetCount(count);
			for(i=0; i<count; i++)
				if (processValue[i])
					vectorData[i] = Point3(floatData[i], floatData[i], floatData[i]);
			break;
		case kCopy_type_floatVector:
			if (floatToAngleType == kCopy_faType_degrees) {
				for(i=0; i<count; i++)
					if (processValue[i])
						vectorData[i] *= RAD_TO_DEG;
			}
			break;
		}
		break;
	}

	bool initChannel = false;
	bool fromScalarToVector = (srcType < kCopy_type_floatVector) && (dstType > kCopy_type_angle);
	
	switch(dstData) {
	case kCopy_data_custom: {
		Class_ID channelClassID;
		switch (dstType) {
		case kCopy_type_boolean:
			channelClassID = ParticleChannelBool_Class_ID;
			break;
		case kCopy_type_time:
			channelClassID = ParticleChannelPTV_Class_ID;
			break;
		case kCopy_type_integer:
			channelClassID = ParticleChannelInt_Class_ID;
			break;
		case kCopy_type_float:
		case kCopy_type_worldFloat:
		case kCopy_type_percent:
		case kCopy_type_angle:
			channelClassID = ParticleChannelFloat_Class_ID;
			break;
		case kCopy_type_floatVector:
		case kCopy_type_worldVector:
		case kCopy_type_percentVector:
		case kCopy_type_angleVector:
			channelClassID = ParticleChannelPoint3_Class_ID;
			break;
		}
		int subChannelIndex = -1;
		IObject* dataChannel = chMult->GetSubChannel(dstName, channelClassID, subChannelIndex);
		if (dataChannel == NULL) { // create the channel
			subChannelIndex = chMult->NumSubChannels();
			bool added = chMult->AddSubChannel(subChannelIndex, channelClassID);
			if (!added) return false;
			chMult->SetName(subChannelIndex, dstName);
			dataChannel = chMult->GetSubChannel(subChannelIndex);
		}
		if (scopeType != chMult->GetScope(subChannelIndex))
			chMult->SetScope(subChannelIndex, scopeType);
		switch (dstType) {
		case kCopy_type_boolean:
			chBoolW = (IParticleChannelBoolW*)dataChannel->GetInterface(PARTICLECHANNELBOOLW_INTERFACE);
			if (chBoolW == NULL) return false;
			for(i=0; i<count; i++)
				if (processValue[i])
					chBoolW->SetValue(i, boolData[i]);
			break;
		case kCopy_type_time:
			chPTVW = (IParticleChannelPTVW*)dataChannel->GetInterface(PARTICLECHANNELPTVW_INTERFACE);
			if (chPTVW == NULL) return false;
			for(i=0; i<count; i++)
				if (processValue[i])
					chPTVW->SetValue(i, timeData[i]);
			break;
		case kCopy_type_integer:
			chIntW = (IParticleChannelIntW*)dataChannel->GetInterface(PARTICLECHANNELINTW_INTERFACE);
			if (chIntW == NULL) return false;
			for(i=0; i<count; i++)
				if (processValue[i])
					chIntW->SetValue(i, intData[i]);
			break;
		case kCopy_type_float:
		case kCopy_type_worldFloat:
		case kCopy_type_percent:
		case kCopy_type_angle:
			chFloatW = (IParticleChannelFloatW*)dataChannel->GetInterface(PARTICLECHANNELFLOATW_INTERFACE);
			if (chFloatW == NULL) return false;
			for(i=0; i<count; i++)
				if (processValue[i])
					chFloatW->SetValue(i, floatData[i]);
			break;
		case kCopy_type_floatVector:
		case kCopy_type_worldVector:
		case kCopy_type_percentVector:
		case kCopy_type_angleVector:
			chPoint3W = (IParticleChannelPoint3W*)dataChannel->GetInterface(PARTICLECHANNELPOINT3W_INTERFACE);
			if (chPoint3W == NULL) return false;
			if (fromScalarToVector) {
				chPoint3R = (IParticleChannelPoint3R*)dataChannel->GetInterface(PARTICLECHANNELPOINT3R_INTERFACE);
				for(i=0; i<count; i++)
					if (processValue[i]) {
						vectorValue = chPoint3R->GetValue(i);
						switch (scalarToVectorType) {
						case kCopy_svType_x:
							vectorValue.x = vectorData[i].x;
							break;
						case kCopy_svType_y:
							vectorValue.y = vectorData[i].y;
							break;
						case kCopy_svType_z:
							vectorValue.z = vectorData[i].z;
							break;
						default:
							vectorValue = vectorData[i];
						}
						chPoint3W->SetValue(i, vectorValue);
					}
			} else {
				for(i=0; i<count; i++)
					if (processValue[i])
						chPoint3W->SetValue(i, vectorData[i]);
			}
			break;
		}
					}
		break;

	case kCopy_data_age: 
		chPTVW = GetParticleChannelBirthTimeWInterface(pCont);
		if (chPTVW == NULL) return false;
		for(i=0; i<count; i++)
			if (processValue[i]) {
				PreciseTimeValue curT = (float(timeData[i]) >= 0.0f) ? timeData[i] : PreciseTimeValue(0);
				chPTVW->SetValue(i, chTime->GetValue(i) - curT);
			}
		break;
	
	case kCopy_data_deathTime: {
	case kCopy_data_lifespan: 
		chPTVR = GetParticleChannelBirthTimeRInterface(pCont);
		if (chPTVR == NULL) return false;
		bool initLifespan = false;
		chLifespanW = (IParticleChannelPTVW*)chCont->EnsureInterface(PARTICLECHANNELLIFESPANW_INTERFACE,
																	ParticleChannelPTV_Class_ID,
																	true, PARTICLECHANNELLIFESPANR_INTERFACE,
																		PARTICLECHANNELLIFESPANW_INTERFACE, true,
																		actionNode, NULL, &initLifespan);
		if (chLifespanW == NULL) return false; 
		for(i=0; i<count; i++) {
			if (processValue[i]) {
				if (dstData == kCopy_data_deathTime)
					chLifespanW->SetValue(i, timeData[i] - chPTVR->GetValue(i));
				else
					chLifespanW->SetValue(i, timeData[i]);
				continue;
			}
			if (initLifespan)
				if (chNew->IsNew(i))
					chLifespanW->SetValue(i, PreciseTimeValue(TIME_PosInfinity));
		}
							   }
		break;
	
	case kCopy_data_timeInEvent:
		chEventStartW = (IParticleChannelPTVW*)chCont->EnsureInterface(PARTICLECHANNELEVENTSTARTW_INTERFACE,
																		ParticleChannelPTV_Class_ID,
																		true, PARTICLECHANNELEVENTSTARTR_INTERFACE,
																		PARTICLECHANNELEVENTSTARTW_INTERFACE, false,
																		actionNode, NULL, &initEventStart);
		if (chEventStartW == NULL) return false; // can't read event start time
		for(i=0; i<count; i++) {
			if (processValue[i]) {
				PreciseTimeValue curT = (float(timeData[i]) >= 0.0f) ? timeData[i] : PreciseTimeValue(0);
				chEventStartW->SetValue(i, chTime->GetValue(i) - curT);
				continue;
			}
			if (initEventStart)
				if (chNew->IsNew(i))
					chEventStartW->SetValue(i, chTime->GetValue(i));
		}
		break;

	case kCopy_data_mapChannel1: case kCopy_data_mapChannel2:
	case kCopy_data_mapChannel3: case kCopy_data_mapChannel4:
	case kCopy_data_mapChannel5: {
		chShapeTextureW = (IParticleChannelMeshMapW*)chCont->EnsureInterface(PARTICLECHANNELSHAPETEXTUREW_INTERFACE,
																	ParticleChannelMeshMap_Class_ID,
																	true, PARTICLECHANNELSHAPETEXTURER_INTERFACE,
																	PARTICLECHANNELSHAPETEXTUREW_INTERFACE, true,
																	actionNode, NULL, &initChannel);
		if (chShapeTextureW == NULL) return false;
		chShapeTextureR = GetParticleChannelShapeTextureRInterface(pCont);
		if (chShapeTextureR == NULL) return false;
		int channelNum = 1 + dstData - kCopy_data_mapChannel1;
		if (!chShapeTextureR->MapSupport(channelNum))
			chShapeTextureW->SetMapSupport(channelNum, true);
		chMapW = chShapeTextureW->GetMapChannel(channelNum);
		if (chMapW == NULL) return false;
		chMapW->SetTVFace(NULL); // no tvFace data for all particles
		if (fromScalarToVector) {
			chMapR = chShapeTextureR->GetMapReadChannel(channelNum);
			for(i=0; i<count; i++) {
				if (processValue[i]) {
					if ((chMapR->GetUVVert(i))->Count() == 1)
						vectorValue = (*(chMapR->GetUVVert(i)))[0];
					else
						vectorValue = Point3::Origin;
					switch (scalarToVectorType) {
					case kCopy_svType_x:
					case kCopy_svType_y:
					case kCopy_svType_z:
						vectorValue[scalarToVectorType - kCopy_svType_x] = (vectorData[i])[scalarToVectorType - kCopy_svType_x];
						break;
					default:
						vectorValue = vectorData[i];
					}
					chMapW->SetUVVert(i, vectorValue);
					continue;
				}
				if (initChannel)
					if (chNew->IsNew(i))
						chMapW->SetUVVert(i, Point3::Origin);

			}
		} else {
			for(i=0; i<count; i++) {
				if (processValue[i]) {
					chMapW->SetUVVert(i, vectorData[i]);
					continue;
				}
				if (initChannel)
					if (chNew->IsNew(i))
						chMapW->SetUVVert(i, Point3::Origin);
			}
		}
								 }
		break;
	
	case kCopy_data_scriptVector:
		chPoint3W = (IParticleChannelPoint3W*)chCont->EnsureInterface(PARTICLECHANNELMXSVECTORW_INTERFACE,
																	ParticleChannelPoint3_Class_ID,
																	true, PARTICLECHANNELMXSVECTORR_INTERFACE,
																	PARTICLECHANNELMXSVECTORW_INTERFACE, true,
																	actionNode, NULL, &initChannel);
		if (chPoint3W == NULL) return false;
		if (fromScalarToVector) {
			chPoint3R = GetParticleChannelMXSVectorRInterface(pCont);
			if (chPoint3R == NULL) return false;
			for(i=0; i<count; i++) {
				if (processValue[i]) {
					vectorValue = chPoint3R->GetValue(i);
					switch(scalarToVectorType) {
					case kCopy_svType_x:
					case kCopy_svType_y:
					case kCopy_svType_z:
						vectorValue[scalarToVectorType - kCopy_svType_x] = (vectorData[i])[scalarToVectorType - kCopy_svType_x];
						break;
					default:
						vectorValue = vectorData[i];
					}
					chPoint3W->SetValue(i, vectorValue);
					continue;
				}
				if (initChannel)
					if (chNew->IsNew(i))
						chPoint3W->SetValue(i, Point3::Origin);
			}
		} else {
			for(i=0; i<count; i++) {
				if (processValue[i]) {
					chPoint3W->SetValue(i, vectorData[i]);
					continue;
				}
				if (initChannel)
					if (chNew->IsNew(i))
						chPoint3W->SetValue(i, Point3::Origin);
			}
		}
		break;

	case kCopy_data_scriptMatrixX: case kCopy_data_scriptMatrixY:
	case kCopy_data_scriptMatrixZ: case kCopy_data_scriptMatrixT: {
		chMatrixW = (IParticleChannelMatrix3W*)chCont->EnsureInterface(PARTICLECHANNELMXSMATRIXW_INTERFACE,
																	ParticleChannelMatrix3_Class_ID,
																	true, PARTICLECHANNELMXSMATRIXR_INTERFACE,
																	PARTICLECHANNELMXSMATRIXW_INTERFACE, true,
																	actionNode, NULL, &initChannel);
		if (chMatrixW == NULL) return false;
		chMatrixR = GetParticleChannelMXSMatrixRInterface(pCont);
		if (chMatrixR == NULL) return false;
		int rowNum = dstData - kCopy_data_scriptMatrixX;
		if (initChannel) {
			Matrix3 initM;
			initM.IdentityMatrix();
			for(i=0; i<count; i++)
				if (chNew->IsNew(i))
					chMatrixW->SetValue(i, initM);
		}
		for(i=0; i<count; i++) {
			if (processValue[i]) {
				Matrix3 curM = chMatrixR->GetValue(i);
				vectorValue = curM.GetRow(rowNum);
				if (fromScalarToVector) {
					switch(scalarToVectorType) {
					case kCopy_svType_x:
					case kCopy_svType_y:
					case kCopy_svType_z:
						vectorValue[scalarToVectorType - kCopy_svType_x] = (vectorData[i])[scalarToVectorType - kCopy_svType_x];
						break;
					default:
						vectorValue = vectorData[i];
					}
				} else {
					vectorValue = vectorData[i];
				}
				curM.SetRow(rowNum, vectorValue);
				chMatrixW->SetValue(i, curM);
			}
		}
								   }
		break;

	case kCopy_data_rotationAxis: 
	case kCopy_data_rotationAngle:
	case kCopy_data_euclideRotation: {
		chQuatW = (IParticleChannelQuatW*)chCont->EnsureInterface(PARTICLECHANNELORIENTATIONW_INTERFACE,
																	ParticleChannelQuat_Class_ID,
																	true, PARTICLECHANNELORIENTATIONR_INTERFACE,
																	PARTICLECHANNELORIENTATIONW_INTERFACE, true,
																	actionNode, NULL, &initChannel);
		if (chQuatW == NULL) return false;
		chQuatR = GetParticleChannelOrientationRInterface(pCont);
		if (chQuatR == NULL) return false;
		if (initChannel) {
			AngAxis aa;
			aa.axis = Point3(1.0f, 0.0f, 0.0f);
			aa.angle = 0.0f;
			Quat initQ(aa);
			for(i=0; i<count; i++)
				if (chNew->IsNew(i))
					chQuatW->SetValue(initQ);
		}
		Quat curQ;
		float curAngle;
		switch (dstData) {
		case kCopy_data_rotationAxis: 
			for(i=0; i<count; i++) {
				if (processValue[i]) {
					curQ = chQuatR->GetValue(i);
					AngAxisFromQ(curQ, &curAngle, vectorValue);
					if (fromScalarToVector) {
						switch(scalarToVectorType) {
						case kCopy_svType_x:
						case kCopy_svType_y:
						case kCopy_svType_z:
							vectorValue[scalarToVectorType - kCopy_svType_x] = (vectorData[i])[scalarToVectorType - kCopy_svType_x];
							break;
						default:
							vectorValue = vectorData[i];
						}
					} else {
						vectorValue = vectorData[i];
					}
					curQ = QFromAngAxis(curAngle, vectorValue);
					chQuatW->SetValue(i, curQ);
				}
			}
			break;
		case kCopy_data_rotationAngle:
			for(i=0; i<count; i++) {
				if (processValue[i]) {
					curQ = chQuatR->GetValue(i);
					AngAxisFromQ(curQ, &curAngle, vectorValue);
					curAngle = floatData[i];
					curQ = QFromAngAxis(curAngle, vectorValue);
					chQuatW->SetValue(i, curQ);
				}
			}
			break;
		case kCopy_data_euclideRotation:
			for(i=0; i<count; i++) {
				if (processValue[i]) {
					Quat curQ = chQuatR->GetValue(i);
					QuatToEuler(curQ, &(vectorValue.x));
					if (fromScalarToVector) {
						switch(scalarToVectorType) {
						case kCopy_svType_x:
						case kCopy_svType_y:
						case kCopy_svType_z:
							vectorValue[scalarToVectorType - kCopy_svType_x] = (vectorData[i])[scalarToVectorType - kCopy_svType_x];
							break;
						default:
							vectorValue = vectorData[i];
						}
					} else {
						vectorValue = vectorData[i];
					}
					EulerToQuat(&(vectorValue.x), curQ, EULERTYPE_XYZ);
					chQuatW->SetValue(i, curQ);
				}
			}
			break;
		}
									 }
		break;

	case kCopy_data_spinAxis:
	case kCopy_data_spinRate:
		chAngAxisW = (IParticleChannelAngAxisW*)chCont->EnsureInterface(PARTICLECHANNELSPINW_INTERFACE,
																	ParticleChannelAngAxis_Class_ID,
																	true, PARTICLECHANNELSPINR_INTERFACE,
																	PARTICLECHANNELSPINW_INTERFACE, true,
																	actionNode, NULL, &initChannel);
		if (chAngAxisW == NULL) return false;
		chAngAxisR = GetParticleChannelSpinRInterface(pCont);
		if (chAngAxisR == NULL) return false;
		if (initChannel) {
			AngAxis aa;
			aa.axis = Point3(1.0f, 0.0f, 0.0f);
			aa.angle = 0.0f;
			for(i=0; i<count; i++)
				if (chNew->IsNew(i))
					chAngAxisW->SetValue(aa);
		}
		if (dstData == kCopy_data_spinAxis) {
			AngAxis curAA;
			for(i=0; i<count; i++) {
				if (processValue[i]) {
					curAA = chAngAxisR->GetValue(i);
					vectorValue = curAA.axis;
					if (fromScalarToVector) {
						switch(scalarToVectorType) {
						case kCopy_svType_x:
						case kCopy_svType_y:
						case kCopy_svType_z:
							vectorValue[scalarToVectorType - kCopy_svType_x] = (vectorData[i])[scalarToVectorType - kCopy_svType_x];
							break;
						default:
							vectorValue = vectorData[i];
						}
					} else {
						vectorValue = vectorData[i];
					}
					curAA.axis = vectorValue;
					chAngAxisW->SetValue(i, curAA);
				}
			}
		} else {
			AngAxis curAA;
			for(i=0; i<count; i++) {
				if (processValue[i]) {
					curAA = chAngAxisR->GetValue(i);
					curAA.angle = floatData[i];
					chAngAxisW->SetValue(i, curAA);
				}
			}
		}
		break;

	case kCopy_data_mtlIndex: 
		chIntW = (IParticleChannelIntW*)chCont->EnsureInterface(PARTICLECHANNELMTLINDEXW_INTERFACE,
																ParticleChannelInt_Class_ID,
																true, PARTICLECHANNELMTLINDEXR_INTERFACE,
																PARTICLECHANNELMTLINDEXW_INTERFACE, true,
																actionNode, NULL, &initChannel);
		if (chIntW == NULL) return false; // can't modify MaterialStatic Index channel in the container
		for(i=0; i<count; i++) {
			if (processValue[i]) {
				chIntW->SetValue(i, intData[i]);
				continue;
			}
			if (initChannel)
				if (chNew->IsNew(i))
					chIntW->SetValue(i, 1);
		}
		break;

	case kCopy_data_scriptInteger:
		chIntW = (IParticleChannelIntW*)chCont->EnsureInterface(PARTICLECHANNELMXSINTEGERW_INTERFACE,
																ParticleChannelInt_Class_ID,
																true, PARTICLECHANNELMXSINTEGERR_INTERFACE,
																PARTICLECHANNELMXSINTEGERW_INTERFACE, true,
																actionNode, NULL, &initChannel);
		if (chIntW == NULL) return false; // can't modify MaterialStatic Index channel in the container
		for(i=0; i<count; i++) {
			if (processValue[i]) {
				chIntW->SetValue(i, intData[i]);
				continue;
			}
			if (initChannel)
				if (chNew->IsNew(i))
					chIntW->SetValue(i, 0);
		}
		break;

	case kCopy_data_position: 
	case kCopy_data_speed:
		if (dstData == kCopy_data_position) 
			chPoint3W = (IParticleChannelPoint3W*)chCont->EnsureInterface(PARTICLECHANNELPOSITIONW_INTERFACE,
																	ParticleChannelPoint3_Class_ID,
																	true, PARTICLECHANNELPOSITIONR_INTERFACE,
																	PARTICLECHANNELPOSITIONW_INTERFACE, true,
																	actionNode, NULL, &initChannel);
		else
			chPoint3W = (IParticleChannelPoint3W*)chCont->EnsureInterface(PARTICLECHANNELSPEEDW_INTERFACE,
																	ParticleChannelPoint3_Class_ID,
																	true, PARTICLECHANNELSPEEDR_INTERFACE,
																	PARTICLECHANNELSPEEDW_INTERFACE, true,
																	actionNode, NULL, &initChannel);

		if (chPoint3W == NULL) return false;
		if (fromScalarToVector) {
			if(dstData == kCopy_data_position)
				chPoint3R = GetParticleChannelPositionRInterface(pCont);
			else
				chPoint3R = GetParticleChannelSpeedRInterface(pCont);
			if (chPoint3R == NULL) return false;
			for(i=0; i<count; i++) {
				if (processValue[i]) {
					vectorValue = chPoint3R->GetValue(i);
					switch(scalarToVectorType) {
					case kCopy_svType_x:
					case kCopy_svType_y:
					case kCopy_svType_z:
						vectorValue[scalarToVectorType - kCopy_svType_x] = (vectorData[i])[scalarToVectorType - kCopy_svType_x];
						break;
					default:
						vectorValue = vectorData[i];
					}
					chPoint3W->SetValue(i, vectorValue);
					continue;
				}
				if (initChannel)
					if (chNew->IsNew(i))
						chPoint3W->SetValue(i, Point3::Origin);
			}
		} else {
			for(i=0; i<count; i++) {
				if (processValue[i]) {
					chPoint3W->SetValue(i, vectorData[i]);
					continue;
				}
				if (initChannel)
					if (chNew->IsNew(i))
						chPoint3W->SetValue(i, Point3::Origin);
			}
		}
		break;

	case kCopy_data_scale:
		chPoint3W = (IParticleChannelPoint3W*)chCont->EnsureInterface(PARTICLECHANNELSCALEW_INTERFACE,
																	ParticleChannelPoint3_Class_ID,
																	true, PARTICLECHANNELSCALER_INTERFACE,
																	PARTICLECHANNELSCALEW_INTERFACE, true,
																	actionNode, NULL, &initChannel);
		if (chPoint3W == NULL) return false;
		if (fromScalarToVector) {
			chPoint3R = GetParticleChannelScaleRInterface(pCont);
			if (chPoint3R == NULL) return false;
			for(i=0; i<count; i++) {
				if (processValue[i]) {
					vectorValue = chPoint3R->GetValue(i);
					switch(scalarToVectorType) {
					case kCopy_svType_x:
					case kCopy_svType_y:
					case kCopy_svType_z:
						vectorValue[scalarToVectorType - kCopy_svType_x] = (vectorData[i])[scalarToVectorType - kCopy_svType_x];
						break;
					default:
						vectorValue = vectorData[i];
					}
					chPoint3W->SetValue(i, vectorValue);
					continue;
				}
				if (initChannel)
					if (chNew->IsNew(i))
						chPoint3W->SetValue(i, Point3(1.0f, 1.0f, 1.0f));
			}
		} else {
			for(i=0; i<count; i++) {
				if (processValue[i]) {
					chPoint3W->SetValue(i, vectorData[i]);
					continue;
				}
				if (initChannel)
					if (chNew->IsNew(i))
						chPoint3W->SetValue(i, Point3(1.0f, 1.0f, 1.0f));
			}
		}
		break;

	case kCopy_data_scriptFloat:
		chFloatW = (IParticleChannelFloatW*)chCont->EnsureInterface(PARTICLECHANNELMXSFLOATW_INTERFACE,
																ParticleChannelFloat_Class_ID,
																true, PARTICLECHANNELMXSFLOATR_INTERFACE,
																PARTICLECHANNELMXSFLOATW_INTERFACE, true,
																actionNode, NULL, &initChannel);
		if (chFloatW == NULL) return false; 
		for(i=0; i<count; i++) {
			if (processValue[i]) {
				chFloatW->SetValue(i, floatData[i]);
				continue;
			}
			if (initChannel)
				if (chNew->IsNew(i))
					chFloatW->SetValue(i, 0.0f);
		}
		break;
	case kCopy_data_selected:
		chBoolW = (IParticleChannelBoolW*)chCont->EnsureInterface(PARTICLECHANNELSELECTIONW_INTERFACE,
																ParticleChannelBool_Class_ID,
																true, PARTICLECHANNELSELECTIONR_INTERFACE,
																PARTICLECHANNELSELECTIONW_INTERFACE, true,
																actionNode, NULL, &initChannel);
		if (chBoolW == NULL) return false; 
		for(i=0; i<count; i++) {
			if (processValue[i]) {
				chBoolW->SetValue(i, boolData[i]);
				continue;
			}
			if (initChannel)
				if (chNew->IsNew(i))
					chBoolW->SetValue(i, false);
		}
		break;
	}

	return true;
}

ClassDesc* PAOperatorCopy::GetClassDesc() const
{
	return GetPAOperatorCopyDesc();
}

} // end of namespace PartASM