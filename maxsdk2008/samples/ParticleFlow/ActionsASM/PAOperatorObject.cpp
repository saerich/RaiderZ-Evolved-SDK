/**********************************************************************
 *<
	FILE:			PAOperatorObject.cpp

	DESCRIPTION:	SimpleSpin Operator implementation
					Operator to effect speed unto particles

	CREATED BY:	Oleg Bayborodin

	HISTORY: created 05-17-03

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/


#include "StdAfx.h"
#include "resource.h"

#include "macrorec.h"

#include "PartASM_SysUtil.h"
#include "PartASM_GlobalFunctions.h"
#include "PartASM_GlobalVariables.h"

#include "PAOperatorObject.h"

#include "PAOperatorObject_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPViewManager.h"
#include "PFMessages.h"
#include "IParticleChannelMult.h"

namespace PartASM {

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|						GeomObjectValidatorClass							 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BOOL IsPAObject(Object *obj)
{ if (obj!=NULL) 
  { if (GetPFObject(obj)->IsParticleSystem()) return FALSE;
	if (obj->SuperClassID()==HELPER_CLASS_ID) return TRUE;
    if (obj->SuperClassID()==GEOMOBJECT_CLASS_ID)
    { if (obj->IsSubClassOf(triObjectClassID)) 
        return TRUE;
      else 
	  { if (obj->CanConvertToType(triObjectClassID)) 
	  	return TRUE;			
	  }
	}
  }
  return FALSE;
}

INode* GetHighestClosedGroupNode(INode *iNode)
{
	if (iNode == NULL) return NULL;

	INode *node = iNode;
	while (node->IsGroupMember()) {
		node = node->GetParentNode();
		while (!(node->IsGroupHead())) node = node->GetParentNode();
		if (!(node->IsOpenGroupHead())) iNode = node;
	}
	return iNode;
}

BOOL PAObjectValidatorClass::Validate(PB2Value &v) 
{
	if (action == NULL) return FALSE;
	INode *node = (INode*) v.r;
	if (node == NULL) return FALSE;
	if (node->TestForLoop(FOREVER,action)!=REF_SUCCEED) return FALSE;

	// check if the item is already included
	if (paramID >= 0) {
		IParamBlock2* pblock = action->GetParamBlock(0);
		if (pblock != NULL) {
			int numItems = pblock->Count(paramID);
			for(int i=0; i<numItems; i++)
				if (pblock->GetINode(paramID, 0, i) == node)
					return FALSE;
		}
	}

	if (node->IsGroupMember())		// get highest closed group node
		node = GetHighestClosedGroupNode(node);
	v.r = node;

	Tab<INode*> stack;
	stack.Append(1, &node, 10);
	TimeValue time = GetCOREInterface()->GetTime();
	while (stack.Count())
	{
		INode* iNode = stack[stack.Count()-1];
		stack.Delete(stack.Count()-1, 1);

		Object* obj = iNode->EvalWorldState(time).obj;
//		if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) // <-- picks up splines which is not good
		if (IsPAObject(obj))
			return TRUE;

		// add children to the stack
		for (int i=0; i<iNode->NumberOfChildren(); i++) {
			INode *childNode = iNode->GetChildNode(i);
			if (childNode) stack.Append(1, &childNode, 10);
		}
	}

	return FALSE;
};

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							PAOperatorObjectLocalData						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
void PAOperatorObjectLocalData::FreeAll()
{
	for(int i=0; i<geomData().Count(); i++)
		if (geomData(i) != NULL) delete _geomData(i);
	_geomData().ZeroCount();
}

void PAOperatorObjectLocalData::InitNodeGeometry(int num)
{
	_geomData().SetCount(num);
	for(int i=0; i<num; i++) _geomData(i) = NULL;
}

int PAOperatorObjectLocalData::NodeGeometryCount() const
{
	return geomData().Count();
}

PFNodeGeometry* PAOperatorObjectLocalData::GetNodeGeometry(int i)
{
	if ((i < 0) || (i >= geomData().Count())) return NULL;
	return geomData(i);
}

void PAOperatorObjectLocalData::SetNodeGeometry(int i, PFNodeGeometry* nodeG)
{
	if ((i>=0) && (i<geomData().Count())) _geomData(i) = nodeG;
}

PFNodeGeometry* PAOperatorObjectLocalData::GetNodeGeometry(TimeValue time, float randValue)
{
	double totalShare = 0.0;
	int i;
	for(i=0; i<geomData().Count(); i++)
		if (geomData(i) != NULL)
			totalShare += _geomData(i)->GetProbabilityShare(time);
	if (totalShare == 0.0f) return NULL;
	double curShare = 0.0;
	for(i=0; i<geomData().Count(); i++)
		if (geomData(i) != NULL) {
			curShare += _geomData(i)->GetProbabilityShare(time)/totalShare;
			if (curShare >= randValue) return geomData(i);
		}
	return NULL;
}

PFNodeGeometry* PAOperatorObjectLocalData::GetNodeGeometry(INode* node)
{
	for(int i=0; i<geomData().Count(); i++)
		if (geomData(i) != NULL)
			if (geomData(i)->GetNode() == node)
				return geomData(i);
	return NULL;
}

PFNodeGeometry* PAOperatorObjectLocalData::GetClosestNodeSurface(
									TimeValue time, const Point3& toPoint,
									Point3& worldLocation, Point3& localLocation, int& faceIndex, int& nodeIndex)
{
	Point3 curWorldPoint, curLocalPoint;
	int curFaceIndex;
	float minDistance2 = 1.0f;
	bool hasMin = false;
	for(int j=0; j<geomData().Count(); j++) {
		if (!geomData(j)->GetClosestPoint(time, toPoint, curWorldPoint, curLocalPoint, curFaceIndex))
				continue;
		Point3 difPoint = toPoint - curWorldPoint;
		float curDistance2 = DotProd(difPoint, difPoint);
		if ((curDistance2 < minDistance2) || !hasMin) {
			hasMin = true;
			minDistance2 = curDistance2;
			nodeIndex = j;
			worldLocation = curWorldPoint;
			localLocation = curLocalPoint;
			faceIndex = curFaceIndex;
		}
	}
	if (hasMin) return geomData(nodeIndex);
	return NULL;
}

PFNodeGeometry* PAOperatorObjectLocalData::GetClosestNodeVertex(TimeValue time, const Point3& toPoint,
										Point3& worldLocation, int& vertexIndex, int& nodeIndex)
{
	Point3 curWorldPoint;
	int curVertexIndex;
	float minDistance2 = 1.0f;
	bool hasMin = false;
	for(int j=0; j<geomData().Count(); j++) {
		if (!geomData(j)->GetClosestVertex(time, toPoint, curWorldPoint, curVertexIndex))
				continue;
		Point3 difPoint = toPoint - curWorldPoint;
		float curDistance2 = DotProd(difPoint, difPoint);
		if ((curDistance2 < minDistance2) || !hasMin) {
			hasMin = true;
			minDistance2 = curDistance2;
			nodeIndex = j;
			worldLocation = curWorldPoint;
			vertexIndex = curVertexIndex;
		}
	}
	if (hasMin) return geomData(nodeIndex);
	return NULL;
}

PFNodeGeometry* PAOperatorObjectLocalData::GetClosestNodePivot(
									TimeValue time, const Point3& toPoint,
									int& nodeIndex)
{
	float minDistance2 = 1.0f;
	bool hasMin = false;
	if (geomData().Count() == 1) {
		nodeIndex = 0;
		hasMin = true;
	} else {
		for(int j=0; j<geomData().Count(); j++) {
			Matrix3* tm = geomData(j)->GetTM(time);
			if (tm == NULL) continue;
			Point3 difPoint = toPoint - tm->GetTrans();
			float curDistance2 = DotProd(difPoint, difPoint);
			if ((curDistance2 < minDistance2) || !hasMin) {
				hasMin = true;
				nodeIndex = j;
				minDistance2 = curDistance2;
			}
		}
	}
	if (hasMin) return geomData(nodeIndex);
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							PAOperatorObject								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
PAObjectValidatorClass PAOperatorObject::validator;
bool PAOperatorObject::validatorInitiated = false;

PAOperatorObject::PAOperatorObject()
{ 
	_numObjects() = 0;
	if (!validatorInitiated) {
		validator.action = NULL;
		validator.paramID = kObject_objects;
		object_paramBlockDesc.ParamOption(kObject_objects,p_validator,&validator);
		validatorInitiated = true;
	}
	GetClassDesc()->MakeAutoParamBlocks(this); 
}

FPInterfaceDesc* PAOperatorObject::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &object_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &object_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &object_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PAOperatorObject::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_OBJECT_CLASS_NAME);
}

Class_ID PAOperatorObject::ClassID()
{
	return PAOperatorObject_Class_ID;
}

void PAOperatorObject::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PAOperatorObject::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefResult PAOperatorObject::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
														PartID& partID, RefMessage message)
{
	switch(message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) { 
				int lastUpdateID = pblock()->LastNotifyParamID(); 
				if (lastUpdateID == kObject_objectsMaxscript) {
					if (IsIgnoringRefNodeChange()) return REF_STOP;
					updateFromMXSObjects();
					return REF_STOP;
				} 
				if (!(theHold.Holding() || theHold.RestoreOrRedoing())) return REF_STOP;
				switch(lastUpdateID) {
				case kObject_channelType:
				case kObject_propertyInteger:
				case kObject_propertyFloat:
				case kObject_propertyVector:
					RefreshUI(kObject_message_update);
					// the break is omitted on purpose (bayboro)
				case kObject_channelName:
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					break;
				case kObject_useSubMtl:
				case kObject_useCondition:
				case kObject_useDesignate:
				case kObject_sync:
				case kObject_animated:
				case kObject_assignOnce:
					RefreshUI(kObject_message_update);
					break;
				case kObject_objects:
					if (IsIgnoringRefNodeChange()) return REF_STOP;
					updateFromRealObjects();
					if (updateObjectNames(kObject_objects)) {
						RefreshUI(kObject_message_update);
						NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
						UpdatePViewUI(lastUpdateID);
					}
					return REF_SUCCEED; // to avoid unnecessary UI update
				}
				UpdatePViewUI(lastUpdateID);
			}
			break;
		case REFMSG_NODE_NAMECHANGE:
			if ( hTarget == pblock() ) { 
				RefreshUI(kObject_message_update);
				NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
				UpdatePViewUI(kObject_objects);
			}
			return REF_SUCCEED; // to avoid unnecessary UI update
		case kObject_RefMsg_ListSelect:
			validator.action = this;
			GetCOREInterface()->DoHitByNameDialog(this);
			return REF_STOP;
		case kObject_RefMsg_ResetValidatorAction:
			validator.action = this;
			return REF_STOP;
	}

	return REF_SUCCEED;
}

RefTargetHandle PAOperatorObject::Clone(RemapDir &remap)
{
	PAOperatorObject* newOp = new PAOperatorObject();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PAOperatorObject::GetObjectName()
{
	return GetString(IDS_OPERATOR_OBJECT_OBJECT_NAME);
}


//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void CollectPANodeObjects(INode* inode, Tab<INode*>& collectedNodes)
{
	if (inode == NULL) return;
	// check out if the node is already collected
	bool alreadyCollected = false;
	int i;
	for(i=0; i<collectedNodes.Count(); i++) {
		if (inode == collectedNodes[i]) {
			alreadyCollected = true;
			break;
		}
	}

	if (!alreadyCollected) collectedNodes.Append(1, &inode);
	for(i=0; i<inode->NumberOfChildren(); i++)
		CollectPANodeObjects(inode->GetChildNode(i), collectedNodes);
}

bool PAOperatorObject::Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes)
{
	bool initRes = PFSimpleAction::Init(pCont, pSystem, node, actions, actionNodes);
	// collect all nodes with surface geometry
	Tab<INode*> nodeList;
	int i;
	RandGenerator* randGen = randLinker().GetRandGenerator(pCont);
	for(i=0; i<pblock()->Count(kObject_objects); i++)
		CollectPANodeObjects(pblock()->GetINode(kObject_objects, 0, i), nodeList);
	if (nodeList.Count() > 0) {
		_localData(pCont).InitNodeGeometry(nodeList.Count());
		for(i=0; i<nodeList.Count(); i++) {
			PFNodeGeometry* nodeG = new PFNodeGeometry();
			nodeG->Init(nodeList[i], pblock()->GetInt(kObject_animated, 0),
									false,
									kLocationType_surface, randGen);
			_localData(pCont).SetNodeGeometry(i, nodeG);
		}
	}
	return initRes;
}

bool PAOperatorObject::Release(IObject* pCont)
{
	PFSimpleAction::Release(pCont);
	_localData(pCont).FreeAll();
	_localData().erase( pCont );
	return true;
}


const ParticleChannelMask& PAOperatorObject::ChannelsUsed(const Interval& time) const
{
								//  read						&write channels
	static ParticleChannelMask mask(PCU_New|PCU_Time|PCU_Speed|PCU_Amount, PCG_AngAxis|PCU_Spin);
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PAOperatorObject::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_OPERATOR_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PAOperatorObject::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_OPERATOR_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PAOperatorObject::HasDynamicName(TSTR& nameSuffix)
{
	int num = pblock()->Count(kObject_objects);
	int count=0, firstIndex=-1;
	for(int i=0; i<num; i++) {
		if (pblock()->GetINode(kObject_objects, 0, i) != NULL) {
			count++;
			if (firstIndex < 0) firstIndex = i;
		}
	}
	TSTR objectName = GetString(IDS_NONE);
	if (count > 0) {
		INode* obj = pblock()->GetINode(kObject_objects, 0, firstIndex);
		objectName = obj->GetName();
		if (count > 1) {
			TCHAR buf[32];
			sprintf(buf, " +%d", count-1);
			objectName += buf;
		}
	}
	
	switch(pblock()->GetInt(kObject_channelType, 0)) {
		case kObject_channelType_integer:
			switch(pblock()->GetInt(kObject_propertyInteger)) {
				case kObject_propertyInteger_objectIndexByPivot:	nameSuffix = GetString(IDS_PROPERTYINTEGER_OBJECTBYPIVOT); break;
				case kObject_propertyInteger_objectIndexByVertex:	nameSuffix = GetString(IDS_PROPERTYINTEGER_OBJECTBYVERTEX); break;
				case kObject_propertyInteger_objectIndexBySurface:	nameSuffix = GetString(IDS_PROPERTYINTEGER_OBJECTBYSURFACE); break;
				case kObject_propertyInteger_vertexIndex:			nameSuffix = GetString(IDS_PROPERTYINTEGER_VERTEXINDEX); break;
				case kObject_propertyInteger_faceIndex:				nameSuffix = GetString(IDS_PROPERTYINTEGER_FACEINDEX); break;
				case kObject_propertyInteger_materialIndex:			nameSuffix = GetString(IDS_PROPERTYINTEGER_MATERIALINDEX); break;
			}
			break;
		case kObject_channelType_float:
			switch(pblock()->GetInt(kObject_propertyFloat)) {
				case kObject_propertyFloat_distanceToPivot:		nameSuffix = GetString(IDS_PROPERTYFLOAT_DISTANCETOPIVOT); break;	
				case kObject_propertyFloat_distanceToVertex:	nameSuffix = GetString(IDS_PROPERTYFLOAT_DISTANCETOVERTEX); break;
				case kObject_propertyFloat_distanceToSurface:	nameSuffix = GetString(IDS_PROPERTYFLOAT_DISTANCETOSURFACE); break;
				case kObject_propertyFloat_objectSpinRate:		nameSuffix = GetString(IDS_PROPERTYFLOAT_OBJECTSPINRATE); break;
				case kObject_propertyFloat_pointLuminance:		nameSuffix = GetString(IDS_PROPERTYFLOAT_POINTLUMINANCE); break;
				case kObject_propertyFloat_pointOpacity:		nameSuffix = GetString(IDS_PROPERTYFLOAT_POINTOPACITY); break;
				case kObject_propertyFloat_faceSquare:			nameSuffix = GetString(IDS_PROPERTYFLOAT_FACESQUARE); break;
				case kObject_propertyFloat_softSelection:		nameSuffix = GetString(IDS_PROPERTYFLOAT_SOFTSELECTION); break;
			}
			break;
		case kObject_channelType_vector:
			switch(pblock()->GetInt(kObject_propertyVector)) {
				case kObject_propertyVector_nearestSurfacePoint:	nameSuffix = GetString(IDS_PROPERTYVECTOR_NEARESTSURFACEPOINT); break;	
				case kObject_propertyVector_nearestVertex:			nameSuffix = GetString(IDS_PROPERTYVECTOR_NEARESTVERTEX); break;
				case kObject_propertyVector_nearestNormal:			nameSuffix = GetString(IDS_PROPERTYVECTOR_NEARESTNORMAL); break;
				case kObject_propertyVector_objectPosition:			nameSuffix = GetString(IDS_PROPERTYVECTOR_OBJECTPOSITION); break;
				case kObject_propertyVector_objectSpeed:			nameSuffix = GetString(IDS_PROPERTYVECTOR_OBJECTSPEED); break;
				case kObject_propertyVector_localSpeed:				nameSuffix = GetString(IDS_PROPERTYVECTOR_LOCALSPEED); break;
				case kObject_propertyVector_objectScale:			nameSuffix = GetString(IDS_PROPERTYVECTOR_OBJECTSCALE); break;
				case kObject_propertyVector_objectEuclideRotation:	nameSuffix = GetString(IDS_PROPERTYVECTOR_OBJECTEUCLIDEROTATION); break;
				case kObject_propertyVector_nearestMapping:			nameSuffix = GetString(IDS_PROPERTYVECTOR_NEARESTMAPPING); break;
				case kObject_propertyVector_mappingUVector:			nameSuffix = GetString(IDS_PROPERTYVECTOR_MAPPINGUVECTOR); break;
				case kObject_propertyVector_mappingVVector:			nameSuffix = GetString(IDS_PROPERTYVECTOR_MAPPINGVVECTOR); break;
				case kObject_propertyVector_mappingWVector:			nameSuffix = GetString(IDS_PROPERTYVECTOR_MAPPINGWVECTOR); break;
				case kObject_propertyVector_nearestColor:			nameSuffix = GetString(IDS_PROPERTYVECTOR_NEARESTCOLOR); break;
				case kObject_propertyVector_objectSpinAxis:			nameSuffix = GetString(IDS_PROPERTYVECTOR_OBJECTSPINAXIS); break;
			}
			break;
	}

	TSTR channelName = pblock()->GetStr(kObject_channelName, 0);
	nameSuffix = nameSuffix + TSTR("(") + objectName +TSTR(")") + TSTR("->") + channelName;
	return true;
}

Point3 GradientVector(const Point3& pos0, const Point3& pos1, const Point3& pos2,
					  const float val0, const float val1, const float val2)
{
	if ((val0 == val1) && (val0 == val2)) return Point3::Origin;
	float v[3];
	Point3 p[3];
	v[0] = val0; v[1] = val1; v[2] = val2;
	p[0] = pos0; p[1] = pos1; p[2] = pos2;
	if (val0 == val2) { // switch 1st and 2nd points
		v[1] = val2; p[1] = pos2;
		v[2] = val1; p[2] = pos1;
	}
	if (val1 == val2) { // switch 0st and 2nd points
		v[0] = val2; p[0] = pos2;
		v[2] = val0; p[2] = pos0;
	}
	if (v[0] == v[1]) { 
		// create normal vector from (pos0-pos1) towards pos2
		Point3 v0 = p[1] - p[0];
		float d0 = LengthSquared(v0);
		Point3 v1 = p[2] - p[0];
		if (d0 <= 0.0f) return (v1/(v[2] - v[0]));
		Point3 gradV = v1 - (DotProd(v1,v0)/d0)*v0;
		return (gradV/(v[2] - v[0]));
	}
	// do the sorting to make v[0] the smallest value and v[2] the greatest value
	for(int i=0; i<3; i++) {
		int j=0, k=1;
		if (i==1) { j=1; k=2; }
		if (v[j] > v[k]) {
			float vv = v[k]; v[k] = v[j]; v[j] = vv;
			Point3 pp = p[k]; p[k] = p[j]; p[j] = pp;
		}
	}
	float r = (v[1] - v[0])/(v[2] - v[0]);
	Point3 ip = p[0] + r*(p[2] - p[0]);
	Point3 v0 = p[1] - ip;
	float d0 = LengthSquared(v0);
	Point3 v1 = p[2] - ip;
	if (d0 <= 0.0f) return (v1/(v[2] - v[1]));
	Point3 gradV = v1 - (DotProd(v1,v0)/d0)*v0;
	return (gradV/(v[2] - v[1]));
}

bool PAOperatorObject::Proceed(IObject* pCont,
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

	IParticleChannelPoint3R* chPos = GetParticleChannelPositionRInterface(pCont);
	if (chPos == NULL) return false;

	IParticleChannelPoint3R* chSpeed = GetParticleChannelSpeedRInterface(pCont);

	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	bool newOnly = (pblock()->GetInt(kObject_assignOnce, timeStart.TimeValue()) != 0);
	int syncType = pblock()->GetInt(kObject_sync, timeStart.TimeValue());
	bool useBirthTime = (syncType == kObject_sync_age);
	bool useEventStart = (syncType == kObject_sync_event);
	bool useSyncChannel = (syncType == kObject_sync_channel);
	bool compensateLag = (pblock()->GetInt(kObject_compensateIntegration, timeStart.TimeValue()) != 0);
	PreciseTimeValue integrationLag;
	bool useSubMtl = (pblock()->GetInt(kObject_useSubMtl, 0) != 0);
	int mtlID = pblock()->GetInt(kObject_subMtl, 0);

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
	TCHAR* dataChannelName = pblock()->GetStr(kObject_channelName, 0);
	int channelType = pblock()->GetInt(kObject_channelType, 0);
	Class_ID channelClassID;
	switch (channelType) {
	case kObject_channelType_integer:
		channelClassID = ParticleChannelInt_Class_ID;
		break;
	case kObject_channelType_float:
		channelClassID = ParticleChannelFloat_Class_ID;
		break;
	case kObject_channelType_vector:
		channelClassID = ParticleChannelPoint3_Class_ID;
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
	int scopeType = pblock()->GetInt(kObject_channelScope, 0);
	if (scopeType != chMult->GetScope(subChannelIndex))
		chMult->SetScope(subChannelIndex, scopeType);

	IParticleChannelBoolR* chFilter = NULL;
	bool useFilter = (pblock()->GetInt(kObject_useCondition, 0) != 0);
	if (useFilter) {
		TCHAR* filterName = pblock()->GetStr(kObject_conditionName, 0);
		int filterChannelIndex;
		IObject* filterData = chMult->GetSubChannel(filterName, ParticleChannelBool_Class_ID, filterChannelIndex);
		if (filterData != NULL)
			chFilter = (IParticleChannelBoolR*)filterData->GetInterface(PARTICLECHANNELBOOLR_INTERFACE);
		if (chFilter == NULL) useFilter = false;
	}

	IParticleChannelPTVR* chSyncTime = NULL;
	if (useSyncChannel) {
		TCHAR* syncName = pblock()->GetStr(kObject_syncName, 0);
		int syncChannelIndex;
		IObject* syncData = chMult->GetSubChannel(syncName, ParticleChannelPTV_Class_ID, syncChannelIndex);
		if (syncData != NULL)
			chSyncTime = (IParticleChannelPTVR*)syncData->GetInterface(PARTICLECHANNELPTVR_INTERFACE);
		if (chSyncTime == NULL) {
			useSyncChannel = false;
			syncType = kObject_sync_time;
		}
	}

	IParticleChannelIntR* chObjectIndex = NULL;
	bool useObjectIndex = (pblock()->GetInt(kObject_useDesignate, 0) != 0);
	if (useObjectIndex) {
		TCHAR* designateName = pblock()->GetStr(kObject_designateName, 0);
		int designateIndex;
		IObject* designateData = chMult->GetSubChannel(designateName, ParticleChannelInt_Class_ID, designateIndex);
		if (designateData != NULL)
			chObjectIndex = (IParticleChannelIntR*)designateData->GetInterface(PARTICLECHANNELINTR_INTERFACE);
		if (chObjectIndex == NULL) useObjectIndex = false;
	}

	IParticleChannelIntW* chIntData = NULL;
	IParticleChannelFloatW* chFloatData = NULL;
	IParticleChannelPoint3W* chVectorData = NULL;
	int propertyType = 0;
	switch (channelType) {
	case kObject_channelType_integer:
		chIntData = (IParticleChannelIntW*)dataChannel->GetInterface(PARTICLECHANNELINTW_INTERFACE);
		propertyType = pblock()->GetInt(kObject_propertyInteger, 0);
		break;
	case kObject_channelType_float:
		chFloatData = (IParticleChannelFloatW*)dataChannel->GetInterface(PARTICLECHANNELFLOATW_INTERFACE);
		propertyType = pblock()->GetInt(kObject_propertyFloat, 0);
		break;
	case kObject_channelType_vector:
		chVectorData = (IParticleChannelPoint3W*)dataChannel->GetInterface(PARTICLECHANNELPOINT3W_INTERFACE);
		propertyType = pblock()->GetInt(kObject_propertyVector, 0);
		break;
	}

	// some calls for a reference node TM may initiate REFMSG_CHANGE notification
	// we have to ignore that while processing the particles
	bool wasIgnoring = IsIgnoringRefNodeChange();
	if (!wasIgnoring) SetIgnoreRefNodeChange();

	PAOperatorObjectLocalData* localData = &(_localData(pCont));
	int objectNum = localData->NodeGeometryCount();
	BitArray loadedMapFiles;
	loadedMapFiles.SetSize(objectNum);
	loadedMapFiles.ClearAll();

	PreciseTimeValue time, syncTime, targetTime;

	for(i=0; i<count; i++) {
		if (newOnly)
			if (chNew->IsNew(i) == false) continue;
		if (useFilter)
			if (chFilter->GetValue(i) == false) continue;

		time = chTime->GetValue(i);
		switch(syncType) {
		case kObject_sync_time:
			syncTime = time;
			break;
		case kObject_sync_age:
			syncTime = time - chBirthTime->GetValue(i);
			break;
		case kObject_sync_event:
			syncTime = time - chEventStartR->GetValue(i);
			break;
		case kObject_sync_channel:
			syncTime = chSyncTime->GetValue(i);
			break;
		}
		if (compensateLag) {
			integrationLag = timeEnd -time;
			syncTime = syncTime + integrationLag;
		}

		Point3 particlePosition = chPos->GetValue(i);
		if (compensateLag && (chSpeed != NULL))
			particlePosition += chSpeed->GetValue(i)*float(integrationLag);

		switch (channelType) {
		case kObject_channelType_integer:
			switch (propertyType) {
			case kObject_propertyInteger_objectIndexByPivot: {
				// Index of the object closest to a particle by the particle / object's-pivot distance.
				int nodeIndex = -1;
				PFNodeGeometry* closestNode = localData->GetClosestNodePivot(syncTime, particlePosition, nodeIndex);
				if (closestNode != NULL) nodeIndex += 1;
				else nodeIndex = 0;
				chIntData->SetValue(i, nodeIndex);
															 }
				break;
			case kObject_propertyInteger_objectIndexByVertex: {
				// Index of the object closest to a particle by the particle / object's-closest-vertex distance.
				int nodeIndex = -1;
				int vertexIndex = 0;
				Point3 worldLoc;
				PFNodeGeometry* closestNode = localData->GetClosestNodeVertex(syncTime, particlePosition, worldLoc, vertexIndex, nodeIndex);
				if (closestNode != NULL) nodeIndex += 1;
				else nodeIndex = 0;
				chIntData->SetValue(i, nodeIndex);
															  }
				break;
			case kObject_propertyInteger_objectIndexBySurface: {
				// Index of the object closest to a particle by the particle / object's-closest-point distance.
				int nodeIndex = -1;
				int faceIndex = 0;
				Point3 worldLoc, localLoc;
				PFNodeGeometry* closestNode = localData->GetClosestNodeSurface(syncTime, particlePosition, worldLoc, localLoc, faceIndex, nodeIndex);
				if (closestNode != NULL) nodeIndex += 1;
				else nodeIndex = 0;
				chIntData->SetValue(i, nodeIndex);
															  }
				break;
			case kObject_propertyInteger_vertexIndex: {
				// Index of the vertex closest to a particle.
				int vertexIndex = 0;
				Point3 worldLoc;
				if (useObjectIndex) {
					PFNodeGeometry* objectNode = NULL;
					int nodeIndex = chObjectIndex->GetValue(i);
					if ((nodeIndex <= 0) || (nodeIndex > localData->NodeGeometryCount())) {
						// the node index is invalid; the closest node is considered
						objectNode = localData->GetClosestNodeVertex(syncTime, particlePosition, worldLoc, vertexIndex, nodeIndex);
						if (objectNode != NULL) vertexIndex += 1;
					} else {
						objectNode = localData->GetNodeGeometry(nodeIndex-1);
						if (objectNode != NULL) {
							if (objectNode->GetClosestVertex(syncTime, particlePosition, worldLoc, vertexIndex))
								vertexIndex += 1;
						}
					}
				} else {
					int nodeIndex = -1;
					PFNodeGeometry* closestNode = localData->GetClosestNodeVertex(syncTime, particlePosition, worldLoc, vertexIndex, nodeIndex);
					if (closestNode != NULL) vertexIndex += 1;
				}
				chIntData->SetValue(i, vertexIndex);
													  }
				break;
			case kObject_propertyInteger_faceIndex:
			case kObject_propertyInteger_materialIndex:	{
				// Index of the face closest to a particle or
				// sub-material index of the face closest to a particle
				int faceIndex = 0;
				Point3 worldLoc, localLoc;
				PFNodeGeometry* objectNode = NULL;
				if (useObjectIndex) {
					int nodeIndex = chObjectIndex->GetValue(i);
					if ((nodeIndex <= 0) || (nodeIndex > localData->NodeGeometryCount())) {
						// the node index is invalid; the closest node is considered
						objectNode = localData->GetClosestNodeSurface(syncTime, particlePosition, worldLoc, localLoc, faceIndex, nodeIndex);
						if (objectNode != NULL) faceIndex += 1;
					} else {
						objectNode = localData->GetNodeGeometry(nodeIndex-1);
						if (objectNode != NULL) {
							if (objectNode->GetClosestPoint(syncTime, particlePosition, worldLoc, localLoc, faceIndex))
								faceIndex += 1;
						}
					}
				} else {
					int nodeIndex = -1;
					objectNode = localData->GetClosestNodeSurface(syncTime, particlePosition, worldLoc, localLoc, faceIndex, nodeIndex);
					if (objectNode != NULL) faceIndex += 1;
				}
				if (propertyType == kObject_propertyInteger_faceIndex) {
					chIntData->SetValue(i, faceIndex);
				} else {
					if ((faceIndex > 0) && (objectNode != NULL)) {
						Mesh* mesh = objectNode->GetMesh(syncTime);
						Face f = mesh->faces[faceIndex-1];
						int mtlIndex = f.getMatID();
						chIntData->SetValue(i, mtlIndex);
					} else {
						chIntData->SetValue(i, 0);
					}
				}
												   }													
				break;
			}
			break;

		case kObject_channelType_float:
			switch(propertyType) {
			case kObject_propertyFloat_distanceToPivot:
			case kObject_propertyFloat_objectSpinRate: {
				// Find the nearest object first (by distance to the object's pivot)
				int nodeIndex = 0;
				PFNodeGeometry* objectNode = NULL;
				if (useObjectIndex) {
					nodeIndex = chObjectIndex->GetValue(i);
					if ((nodeIndex <= 0) || (nodeIndex > localData->NodeGeometryCount())) {
						// the node index is invalid; the closest node is considered
						objectNode = localData->GetClosestNodePivot(syncTime, particlePosition, nodeIndex);
					} else {
						objectNode = localData->GetNodeGeometry(nodeIndex-1);
					}
				} else {
					objectNode = localData->GetClosestNodePivot(syncTime, particlePosition, nodeIndex);
				}
				if (objectNode != NULL) {
					Matrix3* tm = objectNode->GetTM(syncTime);
					if (tm != NULL) {
						switch(propertyType) {
						case kObject_propertyFloat_distanceToPivot: {
							// Distance from a particle to the object's pivot point.
							Point3 dif = particlePosition - tm->GetTrans();
							chFloatData->SetValue(i, dif.Length());
																	}
							break;
						case kObject_propertyFloat_objectSpinRate: {
							// Object's spin rate in degrees per tick.
							Matrix3 *tm2 = objectNode->GetTM(syncTime - PreciseTimeValue(1));
							if (tm2 != NULL) {
								Quat q = Quat(*tm);
								Quat q2 = Quat(*tm2);
								q2.MakeClosest(q);
								Quat r = q/q2;
								float angle;
								Point3 axis;
								AngAxisFromQ(r, &angle, axis);
								chFloatData->SetValue(i, angle);
							}
							break;
																   }
						}
					}
				}
														}
				break;
			case kObject_propertyFloat_distanceToVertex: {
				// Distance from a particle to the closest vertex on the object's body.
				int nodeIndex = -1;
				int vertexIndex = 0;
				Point3 worldLoc;
				PFNodeGeometry* objectNode = NULL;
				if (useObjectIndex) {
					nodeIndex = chObjectIndex->GetValue(i);
					if ((nodeIndex <= 0) || (nodeIndex > localData->NodeGeometryCount())) {
						// the node index is invalid; the closest node is considered
						objectNode = localData->GetClosestNodeVertex(syncTime, particlePosition, worldLoc, vertexIndex, nodeIndex);
					} else {
						objectNode = localData->GetNodeGeometry(nodeIndex-1);
						if (objectNode != NULL)
							objectNode->GetClosestVertex(syncTime, particlePosition, worldLoc, vertexIndex);
					}
				} else {
					objectNode = localData->GetClosestNodeVertex(syncTime, particlePosition, worldLoc, vertexIndex, nodeIndex);
				}
				if (objectNode != NULL) {
					Point3 dif = particlePosition - worldLoc;
					chFloatData->SetValue(i, dif.Length());
				}
														  }
				break;
			case kObject_propertyFloat_distanceToSurface:
			case kObject_propertyFloat_pointLuminance:
			case kObject_propertyFloat_pointOpacity:
			case kObject_propertyFloat_faceSquare:
			case kObject_propertyFloat_softSelection: {
				// Find the nearest point first
				int nodeIndex = -1;
				int faceIndex = 0;
				Point3 worldLoc, localLoc;
				PFNodeGeometry* objectNode = NULL;
				if (useObjectIndex) {
					nodeIndex = chObjectIndex->GetValue(i);
					if ((nodeIndex <= 0) || (nodeIndex > localData->NodeGeometryCount())) {
						// the node index is invalid; the closest node is considered
						objectNode = localData->GetClosestNodeSurface(syncTime, particlePosition, worldLoc, localLoc, faceIndex, nodeIndex);
					} else {
						objectNode = localData->GetNodeGeometry(nodeIndex-1);
						if (objectNode != NULL)
							objectNode->GetClosestPoint(syncTime, particlePosition, worldLoc, localLoc, faceIndex);
					}
				} else {
					objectNode = localData->GetClosestNodeSurface(syncTime, particlePosition, worldLoc, localLoc, faceIndex, nodeIndex);
				}
				if (objectNode != NULL) {
					switch (propertyType) {
					case kObject_propertyFloat_distanceToSurface: {
						// Distance from a particle to the closest point of the object's surface.
						Point3 dif = particlePosition - worldLoc;
						chFloatData->SetValue(i, dif.Length());
																  }
						break;
					case kObject_propertyFloat_pointLuminance:
						// Luminance factor at the closest to a particle point on the object's surface.
					case kObject_propertyFloat_pointOpacity: {
						// Opacity factor at the closest to a particle point on the object's surface. 
						INode* inode = objectNode->GetNode(); DbgAssert(inode);
						Mtl* mtl = inode->GetMtl();
						if ((mtl != NULL) && useSubMtl)
							if (mtlID < mtl->NumSubMtls()) mtl = mtl->GetSubMtl(mtlID);
						if (mtl != NULL) {
							if (!loadedMapFiles[nodeIndex]) {
								int numSubs = mtl->NumSubTexmaps();
								for(int subIndex = 0; subIndex<numSubs; subIndex++) {
									Texmap* subTexmap = mtl->GetSubTexmap(subIndex);
									if (subTexmap != NULL)
										subTexmap->LoadMapFiles(timeEnd);
								}
								loadedMapFiles.Set(nodeIndex);
							}
							PFObjectMaterialShadeContext sc;
							LocationPoint lp;
							lp.init = true;
							lp.location = localLoc;
							lp.node = inode;
							lp.pointIndex = faceIndex;
							sc.Init(objectNode, lp, worldLoc, TimeValue(syncTime));
							// pre-init colors to avoid under-initialized colors
							sc.out.c = Point3(1.0f, 1.0f, 1.0f);
							mtl->Shade(sc);
							if (propertyType == kObject_propertyFloat_pointLuminance)
								chFloatData->SetValue(i, (sc.out.c.r+sc.out.c.g+sc.out.c.b)/3.0f);
							else
								chFloatData->SetValue(i, 1.0f - (sc.out.t.r+sc.out.t.g+sc.out.t.b)/3.0f);
						}
															 }
						break;
					case kObject_propertyFloat_faceSquare: {
						// Surface area of the object's face closest to a particle.
						Mesh* curMesh = objectNode->GetMesh(syncTime);
						if (curMesh != NULL)
							chFloatData->SetValue(i, FaceArea(curMesh, faceIndex));
														   }
						break;
					case kObject_propertyFloat_softSelection: {
						Mesh* curMesh = objectNode->GetMesh(syncTime);
						float selData = 0.0f;
						if (curMesh != NULL) {
							float* softSel = curMesh->getVSelectionWeights();
							if (softSel != NULL) {
								Face f = curMesh->faces[faceIndex];
								selData = (1.0f-localLoc.x-localLoc.y)*softSel[f.v[0]] +
												localLoc.x*softSel[f.v[1]] + localLoc.y*softSel[f.v[2]];
							}
						}
						chFloatData->SetValue(i, selData);
															  }
						break;
					}
				}
														  }
				break;
			}
			break;
		
		case kObject_channelType_vector:
			switch (propertyType) {
			case kObject_propertyVector_nearestVertex: {
				// Position of the object's vertex closest to a particle.
				int nodeIndex = -1;
				int vertexIndex = 0;
				Point3 worldLoc;
				PFNodeGeometry* objectNode = NULL;
				if (useObjectIndex) {
					nodeIndex = chObjectIndex->GetValue(i);
					if ((nodeIndex <= 0) || (nodeIndex > localData->NodeGeometryCount())) {
						// the node index is invalid; the closest node is considered
						objectNode = localData->GetClosestNodeVertex(syncTime, particlePosition, worldLoc, vertexIndex, nodeIndex);
					} else {
						objectNode = localData->GetNodeGeometry(nodeIndex-1);
						if (objectNode != NULL)
							objectNode->GetClosestVertex(syncTime, particlePosition, worldLoc, vertexIndex);
					}
				} else {
					objectNode = localData->GetClosestNodeVertex(syncTime, particlePosition, worldLoc, vertexIndex, nodeIndex);
				}
				if (objectNode != NULL) chVectorData->SetValue(i, worldLoc);
														  }
				break;
			case kObject_propertyVector_objectPosition:
			case kObject_propertyVector_objectSpeed:
			case kObject_propertyVector_objectScale:
			case kObject_propertyVector_objectEuclideRotation:
			case kObject_propertyVector_objectSpinAxis: {
				// Find the nearest object first (by distance to the object's pivot)
				float distance = 0.0f;
				int nodeIndex = 0;
				PFNodeGeometry* objectNode = NULL;
				if (useObjectIndex) {
					nodeIndex = chObjectIndex->GetValue(i);
					if ((nodeIndex <= 0) || (nodeIndex > localData->NodeGeometryCount())) {
						// the node index is invalid; the closest node is considered
						objectNode = localData->GetClosestNodePivot(syncTime, particlePosition, nodeIndex);
					} else {
						objectNode = localData->GetNodeGeometry(nodeIndex-1);
					}
				} else {
					objectNode = localData->GetClosestNodePivot(syncTime, particlePosition, nodeIndex);
				}
				if (objectNode != NULL) {
					Matrix3 *tm = NULL, *tm2 = NULL;
					switch (propertyType) {
					case kObject_propertyVector_objectPosition:
						// Object's pivot position in world space.
						tm = objectNode->GetTM(syncTime);
						if (tm != NULL) chVectorData->SetValue(i, tm->GetTrans());
						break;
					case kObject_propertyVector_objectSpeed:
						// Object's pivot speed in units per tick.
						tm = objectNode->GetTM(syncTime);
						tm2 = objectNode->GetTM(syncTime - PreciseTimeValue(1));
						if ((tm != NULL) && (tm2 != NULL)) 
							chVectorData->SetValue(i, tm->GetTrans() - tm2->GetTrans());
						break; 
					case kObject_propertyVector_objectScale:
						// Object's scale factor as a vector for X, Y and Z scales.
						tm = objectNode->GetTM(syncTime);
						if (tm != NULL) {
							Point3 axisX = tm->GetRow(0);
							Point3 axisY = tm->GetRow(1);
							Point3 axisZ = tm->GetRow(2);
							Point3 scaleV = Point3(axisX.Length(), axisY.Length(), axisZ.Length());
							chVectorData->SetValue(i, scaleV);
						}
						break; 
					case kObject_propertyVector_objectEuclideRotation:
						// Object orientation as a series of X, Y, Z Euclide rotations.
						tm = objectNode->GetTM(syncTime);
						if (tm != NULL) {
							Quat q = Quat(*tm);
							Point3 v;
							QuatToEuler(q, v);
							chVectorData->SetValue(i, v);
						}
						break;
					case kObject_propertyVector_objectSpinAxis:
						// Axis of the object's spinning.
						tm = objectNode->GetTM(syncTime);
						tm2 = objectNode->GetTM(syncTime - PreciseTimeValue(1));
						if ((tm != NULL) && (tm2 != NULL)) {
							Quat q = Quat(*tm);
							Quat q2 = Quat(*tm2);
							q2.MakeClosest(q);
							Quat r = q/q2;
							float angle;
							Point3 axis;
							AngAxisFromQ(r, &angle, axis);
							chVectorData->SetValue(i, axis);
						}
						break; 
					}
				}
														}
				break;
			case kObject_propertyVector_nearestSurfacePoint:
				// Position of the object's surface point closest to a particle.
			case kObject_propertyVector_nearestColor: {
				// Color at the closest to a particle point on the object's surface.
			case kObject_propertyVector_nearestNormal:
				// Normal vector at the object's surface point closest to a particle.
			case kObject_propertyVector_localSpeed:
				// local speed on the surface of the reference object
			case kObject_propertyVector_nearestMapping:
				// mapping value at the nearest surface point
			case kObject_propertyVector_mappingUVector:
			case kObject_propertyVector_mappingVVector:
			case kObject_propertyVector_mappingWVector:
				// tangent gradient vector for mapping (U, V, or W value gradient)
				int nodeIndex = -1;
				int faceIndex = 0;
				Point3 worldLoc, localLoc;
				PFNodeGeometry* objectNode = NULL;
				if (useObjectIndex) {
					nodeIndex = chObjectIndex->GetValue(i);
					if ((nodeIndex <= 0) || (nodeIndex > localData->NodeGeometryCount())) {
						// the node index is invalid; the closest node is considered
						objectNode = localData->GetClosestNodeSurface(syncTime, particlePosition, worldLoc, localLoc, faceIndex, nodeIndex);
					} else {
						objectNode = localData->GetNodeGeometry(nodeIndex-1);
						if (objectNode != NULL)
							objectNode->GetClosestPoint(syncTime, particlePosition, worldLoc, localLoc, faceIndex);
					}
				} else {
					objectNode = localData->GetClosestNodeSurface(syncTime, particlePosition, worldLoc, localLoc, faceIndex, nodeIndex);
				}
				if (objectNode != NULL) {
					switch(propertyType) {
					case kObject_propertyVector_nearestSurfacePoint:
						// Position of the object's surface point closest to a particle.
						chVectorData->SetValue(i, worldLoc);
						break;
					case kObject_propertyVector_nearestColor: {
						// Color at the closest to a particle point on the object's surface.
						INode* inode = objectNode->GetNode(); DbgAssert(inode);
						Mtl* mtl = inode->GetMtl();
						if ((mtl != NULL) && useSubMtl)
							if (mtlID < mtl->NumSubMtls()) mtl = mtl->GetSubMtl(mtlID);
						if (mtl != NULL) {
							if (!loadedMapFiles[nodeIndex]) {
								int numSubs = mtl->NumSubTexmaps();
								for(int subIndex = 0; subIndex<numSubs; subIndex++) {
									Texmap* subTexmap = mtl->GetSubTexmap(subIndex);
									if (subTexmap != NULL)
										subTexmap->LoadMapFiles(timeEnd);
								}
								loadedMapFiles.Set(nodeIndex);
							}
							PFObjectMaterialShadeContext sc;
							LocationPoint lp;
							lp.init = true;
							lp.location = localLoc;
							lp.node = inode;
							lp.pointIndex = faceIndex;
							sc.Init(objectNode, lp, worldLoc, TimeValue(syncTime));
							// pre-init colors to avoid under-initialized colors
							sc.out.c = Point3(1.0f, 1.0f, 1.0f);
							mtl->Shade(sc);
							chVectorData->SetValue(i, Point3(sc.out.c.r, sc.out.c.g, sc.out.c.b));
						}
															  }
						break;
					case kObject_propertyVector_nearestNormal: {
						// Normal vector at the object's surface point closest to a particle.
						//INode* inode = objectNode->GetNode(); DbgAssert(inode);
						Mesh* curMesh = objectNode->GetMesh(syncTime);
						//objectNode = localData->GetClosestNodeSurface(syncTime, particlePosition, worldLoc, localLoc, faceIndex, nodeIndex);
						Point3* normals = objectNode->GetNormals(syncTime);
						Point3 nearNormal = Point3::XAxis;
						if (normals != NULL) {
							nearNormal = (1.0f-localLoc.x-localLoc.y)*normals[3*faceIndex] + 
										localLoc.x*normals[3*faceIndex+1] + localLoc.y*normals[3*faceIndex+2];
							Matrix3* tm = objectNode->GetTM(syncTime);
							if (tm != NULL) nearNormal = tm->VectorTransform(nearNormal);
						}
						chVectorData->SetValue(i, nearNormal);
															   }
						break;
					case kObject_propertyVector_localSpeed: {
						Mesh* lastMesh = objectNode->GetMesh(syncTime-1);
						Matrix3* tm = objectNode->GetTM(syncTime-1);
						if ((lastMesh != NULL) && (tm != NULL)) {
							if (faceIndex < lastMesh->numFaces) {
								Face f = lastMesh->faces[faceIndex];
								Point3 lastLoc = (1.0f-localLoc.x-localLoc.y)*lastMesh->verts[f.v[0]] + 
										   localLoc.x*lastMesh->verts[f.v[1]] + localLoc.y*lastMesh->verts[f.v[2]];
								lastLoc = tm->VectorTransform(lastLoc);
								Point3 speed = worldLoc - lastLoc;
								chVectorData->SetValue(i, speed);
							}
						}
															}
						break;
					case kObject_propertyVector_nearestMapping: {
						Mesh* curMesh = objectNode->GetMesh(syncTime);
						int mapChannel = pblock()->GetInt(kObject_mappingChannel, 0);
						if (curMesh == NULL) break;
						if (!curMesh->mapSupport(mapChannel)) break;
						TVFace* mapFaces = curMesh->mapFaces(mapChannel);
						UVVert* mapVerts = curMesh->mapVerts(mapChannel);
						if ((mapFaces == NULL) || (mapVerts == NULL)) break;
						TVFace tvf = mapFaces[faceIndex];
						UVVert locMap = (1.0f-localLoc.x-localLoc.y)*mapVerts[tvf.t[0]] + 
										localLoc.x*mapVerts[tvf.t[1]] + localLoc.y*mapVerts[tvf.t[2]];
						chVectorData->SetValue(i, locMap);
																}
						break;
					case kObject_propertyVector_mappingUVector:
					case kObject_propertyVector_mappingVVector:
					case kObject_propertyVector_mappingWVector: {
						Mesh* curMesh = objectNode->GetMesh(syncTime);
						int mapChannel = pblock()->GetInt(kObject_mappingChannel, 0);
						if (curMesh == NULL) break;
						if (!curMesh->mapSupport(mapChannel)) break;
						TVFace* mapFaces = curMesh->mapFaces(mapChannel);
						UVVert* mapVerts = curMesh->mapVerts(mapChannel);
						if ((mapFaces == NULL) || (mapVerts == NULL)) break;
						TVFace tvf = mapFaces[faceIndex];
						Face f = curMesh->faces[faceIndex];
						float v[3];
						Point3 p[3];						
						for(int j=0; j<3; j++) {
							v[j] = (mapVerts[tvf.t[j]])[propertyType - kObject_propertyVector_mappingUVector];
							p[j] = curMesh->verts[f.v[j]];
						}
						Point3 mapGradient = GradientVector(p[0], p[1], p[2], v[0], v[1], v[2]);
						Matrix3* tm = objectNode->GetTM(syncTime);
						if (tm != NULL) mapGradient = tm->VectorTransform(mapGradient);
						chVectorData->SetValue(i, mapGradient);
																}
						break;
					}
				}
													  }				
				break;
			}
			break;
		}
	}

	if (!wasIgnoring) ClearIgnoreRefNodeChange();

	return true;
}

ClassDesc* PAOperatorObject::GetClassDesc() const
{
	return GetPAOperatorObjectDesc();
}

//+--------------------------------------------------------------------------+
//|							From PFTestSpeedGoToTarget						 |
//+--------------------------------------------------------------------------+
int PAOperatorObject::getNumObjects()
{
	int num = pblock()->Count(kObject_objects);
	int count=0;
	for(int i=0; i<num; i++)
		if (pblock()->GetINode(kObject_objects, 0, i) != NULL)
			count++;
	return count;	
}

bool VerifyObjectsMXSSync(IParamBlock2* pblock, int kRealRefObjectsID, int kMXSRefObjectsID)
{
	if (pblock == NULL) return true;
	int count = pblock->Count(kRealRefObjectsID);
	if (count != pblock->Count(kMXSRefObjectsID)) return false;
	for(int i=0; i<count; i++) {
		if (pblock->GetINode(kRealRefObjectsID, 0, i) != 
			pblock->GetINode(kMXSRefObjectsID, 0, i)) 
			return false;
	}
	return true;
}

bool UpdateFromRealRefObjects(IParamBlock2* pblock, int kRealRefObjectsID, int kMXSRefObjectsID, 
							  bool& gUpdateInProgress)
{
	if (pblock == NULL) return false;
	if (gUpdateInProgress) return false;
	if (VerifyObjectsMXSSync(pblock, kRealRefObjectsID, kMXSRefObjectsID)) return false;
	gUpdateInProgress = true;
	pblock->ZeroCount(kMXSRefObjectsID);
	for(int i=0; i<pblock->Count(kRealRefObjectsID); i++) {
		INode* refNode = pblock->GetINode(kRealRefObjectsID, 0, i);
		pblock->Append(kMXSRefObjectsID, 1, &refNode);
	}
	gUpdateInProgress = false;
	return true;
}

bool UpdateFromMXSRefObjects(IParamBlock2* pblock, int kRealRefObjectsID, int kMXSRefObjectsID, 
							 bool& gUpdateInProgress, HitByNameDlgCallback* callback)
{
	if (pblock == NULL) return false;
	if (gUpdateInProgress) return false;
	if (VerifyObjectsMXSSync(pblock, kRealRefObjectsID, kMXSRefObjectsID)) return false;
	gUpdateInProgress = true;
	pblock->ZeroCount(kRealRefObjectsID);
	for(int i=0; i<pblock->Count(kMXSRefObjectsID); i++) {
		INode* node = pblock->GetINode(kMXSRefObjectsID, 0, i);
		if (node == NULL) {
			pblock->Append(kRealRefObjectsID, 1, &node);
		} else {
			if (callback->filter(node) == TRUE) {
				pblock->Append(kRealRefObjectsID, 1, &node);
			} else {
				node = NULL;
				pblock->Append(kRealRefObjectsID, 1, &node);
				pblock->SetValue(kMXSRefObjectsID, 0, node, i);
			}
		}
	}
	gUpdateInProgress = false;
	return true;
}

void MacrorecObjects(ReferenceTarget* rtarg, IParamBlock2* pblock, int paramID, TCHAR* paramName)
{
	if ((pblock == NULL) || (rtarg == NULL)) return;
	TSTR selItemNames = _T("(");
	int numItems = pblock->Count(paramID);
	bool nonFirst = false;
	for(int i=0; i<numItems; i++) {
		INode* selNode = pblock->GetINode(paramID, 0, i);
		if (selNode == NULL) continue;
		if (nonFirst) selItemNames = selItemNames + _T(", ");
		selItemNames = selItemNames + _T("$'");
		selItemNames = selItemNames + selNode->GetName();
		selItemNames = selItemNames + _T("'");
		nonFirst = true;
	}
	selItemNames = selItemNames + _T(")");
	TCHAR selNames[8192];
	sprintf(selNames,"%s", selItemNames);
	macroRec->EmitScript();
	macroRec->SetProperty(rtarg, paramName, mr_name, selNames);
}

bool gUpdateObjectsInProgress = false;

bool PAOperatorObject::updateFromRealObjects()
{
	if (theHold.RestoreOrRedoing()) return false;
	bool res = UpdateFromRealRefObjects(pblock(), kObject_objects, 
				kObject_objectsMaxscript, gUpdateObjectsInProgress);
	if (res && theHold.Holding())
		MacrorecObjects(this, pblock(), kObject_objects, _T("Objects"));
	return res;
}

bool PAOperatorObject::updateFromMXSObjects()
{
	if (theHold.RestoreOrRedoing()) return false;
	return UpdateFromMXSRefObjects(pblock(), kObject_objects, 
				kObject_objectsMaxscript, gUpdateObjectsInProgress, this);
}

bool PAOperatorObject::updateObjectNames(int pblockID)
{
	if (pblock() == NULL) return false;
	bool needUpdate = false;
	int count = pblock()->Count(pblockID);
	if (count != objectsToShow().Count()) needUpdate = true;
	if (!needUpdate) {
		for(int i=0; i<count; i++)
			if (pblock()->GetINode(pblockID, 0, i) != objectToShow(i))
			{	needUpdate = true; 	break;	}
	}
	if (!needUpdate) return false;
	_objectsToShow().SetCount(count);
	for(int i=0; i<count; i++)
		_objectToShow(i) = pblock()->GetINode(pblockID, 0, i);
	return true;
}

//+--------------------------------------------------------------------------+
//|							From HitByNameDlgCallback						 |
//+--------------------------------------------------------------------------+
int PAOperatorObject::filter(INode *node)
{
	PB2Value v;
	v.r = node;
	validator.action = this;
	return validator.Validate(v);
}

void PAOperatorObject::proc(INodeTab &nodeTab)
{
	if (nodeTab.Count() == 0) return;
	theHold.Begin();
	pblock()->Append(kObject_objects, nodeTab.Count(), nodeTab.Addr(0));
	theHold.Accept(GetString(IDS_PARAMETERCHANGE));
}



} // end of namespace PartASM