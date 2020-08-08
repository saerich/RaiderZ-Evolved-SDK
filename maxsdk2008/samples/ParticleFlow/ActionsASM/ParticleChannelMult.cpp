/**********************************************************************
 *<
	FILE:			ParticleChannelMult.cpp

	DESCRIPTION:	ParticleChannelMult implementation
					This generic channel is used to store multiple data

	CREATED FOR:	Oleg Bayborodin

	HISTORY:		created 2003-05-18

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#include "StdAfx.h"
#include "resource.h"

#include "ParticleChannelMult.h"
#include "PFClassIDs.h"

namespace PartASM {

ParticleChannelMult::ParticleChannelMult()
{
	_count() = 0;
	_numSubChannels() = 0;
	for(int i=0; i<kMax_SubChannels; i++) {
		_dataObject(i) = NULL;
		_name(i) = NULL;
		_scope(i) = kScope_local; 
		_dataChannel(i) = NULL;
		_dataAmountR(i) = NULL;
		_dataAmountW(i) = NULL;
	}
}

ParticleChannelMult::~ParticleChannelMult()
{
	_count() = 0;
	for(int i=0; i<kMax_SubChannels; i++) {
		if (_dataObject(i)) _dataObject(i)->DeleteIObject();
		if (name(i) != NULL) delete [] _name(i);
	}
}

TCHAR* ParticleChannelMult::GetIObjectName()
{
	return _T("ParticleChannelMult");
}

BaseInterface* ParticleChannelMult::GetInterfaceAt(int index) const
{
	switch(index)
	{
	case 0: return (IParticleChannel*)this;
	case 1: return (IParticleChannelAmountR*)this;
	case 2: return (IParticleChannelAmountW*)this;
	case 3: return (IParticleChannelMult*)this;
	}
	return NULL;
}

BaseInterface* ParticleChannelMult::GetInterface(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE)
		return (IParticleChannel*)this;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) 
		return (IParticleChannelAmountR*)this;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) 
		return (IParticleChannelAmountW*)this;
	if (id == PARTICLECHANNELMULT_INTERFACE)
		return (IParticleChannelMult*)this;
	return IObject::GetInterface(id);
}

void ParticleChannelMult::AcquireIObject()
{
	_numRefs() += 1;
}

void ParticleChannelMult::ReleaseIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}

void ParticleChannelMult::DeleteIObject()
{
	_numRefs() -= 1;
	if (numRefs() <= 0) delete this;
}

ParticleChannelMultDesc TheParticleChannelMultDesc;

static FPInterfaceDesc pc_Mult_channel(
			PARTICLECHANNEL_INTERFACE, 
			_T("channel"), 0, 
			&TheParticleChannelMultDesc, FP_MIXIN, 
			
		IParticleChannel::kClone, _T("clone"), 0, TYPE_IOBJECT, 0, 0,
		IParticleChannel::kIsSimilarChannel, _T("isSimilarChannel"), 0, TYPE_bool, 0, 1,
			_T("channel"), 0, TYPE_IOBJECT,
		IParticleChannel::kIsTransferable, _T("isTransferable"), 0, TYPE_bool, 0, 0,
		IParticleChannel::kSetTransferable, _T("setTransferable"), 0, TYPE_VOID, 0, 1,
			_T("status"), 0, TYPE_bool,
		IParticleChannel::kIsPrivateChannel, _T("isPrivateChannel"), 0, TYPE_bool, 0, 0,
		IParticleChannel::kGetPrivateOwner, _T("getPrivateOwner"), 0, TYPE_OBJECT, 0, 0,
		IParticleChannel::kSetPrivateOwner, _T("setPrivateOwner"), 0, TYPE_VOID, 0, 1,
			_T("action"), 0, TYPE_OBJECT,
		IParticleChannel::kGetCreatorAction, _T("getCreatorAction"), 0, TYPE_INODE, 0, 0,
		IParticleChannel::kSetCreatorAction, _T("setCreatorAction"), 0, TYPE_VOID, 0, 1,
			_T("action"), 0, TYPE_INODE,
		IParticleChannel::kGetReadID_PartA, _T("getReadID_PartA"), 0, TYPE_DWORD, 0, 0,
		IParticleChannel::kGetReadID_PartB, _T("getReadID_PartB"), 0, TYPE_DWORD, 0, 0,
		IParticleChannel::kGetWriteID_PartA, _T("getWriteID_PartA"), 0, TYPE_DWORD, 0, 0,
		IParticleChannel::kGetWriteID_PartB, _T("getWriteID_PartB"), 0, TYPE_DWORD, 0, 0,
		IParticleChannel::kSetReadID, _T("setReadID"), 0, TYPE_VOID, 0, 2,
			_T("partA"), 0, TYPE_DWORD,
			_T("partB"), 0, TYPE_DWORD,
		IParticleChannel::kSetWriteID, _T("setWriteID"), 0, TYPE_VOID, 0, 2,
			_T("partA"), 0, TYPE_DWORD,
			_T("partB"), 0, TYPE_DWORD,

		end
); 

static FPInterfaceDesc pc_Mult_AmountR(
			PARTICLECHANNELAMOUNTR_INTERFACE,
			_T("amountRead"), 0,
			&TheParticleChannelMultDesc, FP_MIXIN,

		IParticleChannelAmountR::kCount, _T("count"), 0, TYPE_INT, 0, 0,

		end
);

static FPInterfaceDesc pc_Mult_AmountW(
			PARTICLECHANNELAMOUNTW_INTERFACE,
			_T("amountWrite"), 0,
			&TheParticleChannelMultDesc, FP_MIXIN,

		IParticleChannelAmountW::kZeroCount, _T("zeroCount"), 0, TYPE_VOID, 0, 0,
		IParticleChannelAmountW::kSetCount, _T("setCount"), 0, TYPE_bool, 0, 1,
			_T("amount"), 0, TYPE_INT,
		IParticleChannelAmountW::kDeleteByIndex, _T("delete"), 0, TYPE_INT, 0, 2,
			_T("firstIndex"), 0, TYPE_INT,
			_T("amount"), 0, TYPE_INT,
		IParticleChannelAmountW::kDeleteByArray, _T("delete"), 0, TYPE_INT, 0, 1,
			_T("toDelete"), 0, TYPE_BITARRAY_BR,
		IParticleChannelAmountW::kSplit, _T("split"), 0, TYPE_IOBJECT, 0, 1,
			_T("toSplit"), 0, TYPE_BITARRAY_BR,
		IParticleChannelAmountW::kSpawn, _T("spawn"), 0, TYPE_bool, 0, 1,
			_T("toSpawn"), 0, TYPE_INT_TAB_BR,
		IParticleChannelAmountW::kAppendNum, _T("appendNum"), 0, TYPE_bool, 0, 1,
			_T("amount"), 0, TYPE_INT,
		IParticleChannelAmountW::kAppend, _T("append"), 0, TYPE_bool, 0, 1,
			_T("channel"), 0, TYPE_IOBJECT,

		end
);

FPInterfaceDesc* ParticleChannelMult::GetDescByID(Interface_ID id)
{
	if (id == PARTICLECHANNEL_INTERFACE) return &pc_Mult_channel;
	if (id == PARTICLECHANNELAMOUNTR_INTERFACE) return &pc_Mult_AmountR;
	if (id == PARTICLECHANNELAMOUNTW_INTERFACE) return &pc_Mult_AmountW;
	return NULL;
}

Class_ID ParticleChannelMult::GetClassID() const
{
	return ParticleChannelMult_Class_ID;
}

IObject*  ParticleChannelMult::Clone() const
{
	ParticleChannelMult* newChannel = (ParticleChannelMult*)CreateInstance(REF_TARGET_CLASS_ID, GetClassID());
	newChannel->CloneChannelCore((IParticleChannel*)this);
	newChannel->_count() = count();
	newChannel->_numSubChannels() = numSubChannels();
	for(int i=0; i<numSubChannels(); i++) {
		if (dataObject(i) == NULL) continue; 
		assert(dataChannel(i));
		IObject* newChan = dataChannel(i)->Clone();
		if (newChan == NULL) continue; // not enough memory?
		newChannel->_dataObject(i) = newChan;
		newChannel->_dataChannel(i) = GetParticleChannelInterface(newChan);
		newChannel->_dataAmountR(i) = GetParticleChannelAmountRInterface(newChan);
		newChannel->_dataAmountW(i) = GetParticleChannelAmountWInterface(newChan);
		TCHAR* chanName = name(i);
		if (chanName != NULL) newChannel->SetName(i, chanName);
		newChannel->SetScope(i, GetScope(i));
	}
	return newChannel;
}

IOResult ParticleChannelMult::Save(ISave* isave) const
{
	ULONG nb;
	IOResult res;
	int num, scopeType;

	isave->BeginChunk(IParticleChannel::kChunkCount);
	num = count();
	if ((res = isave->Write(&num, sizeof(int), &nb)) != IO_OK) return res;
	isave->EndChunk();

	for(int i=0; i<numSubChannels(); i++) {
		if(dataObject(i) == NULL) continue;
		assert(dataChannel(i));
		isave->BeginChunk(IParticleChannel::kChunkValue1);
		if ((res = isave->Write(&i, sizeof(int), &nb)) != IO_OK) return res;
		isave->EndChunk();
		Class_ID channelClassID = dataChannel(i)->GetClassID();
		isave->BeginChunk(IParticleChannel::kChunkValue2);
		if ((res = isave->Write(&channelClassID, sizeof(Class_ID), &nb)) != IO_OK) return res;
		isave->EndChunk();
		isave->BeginChunk(IParticleChannel::kChunkData);
		if ((res = dataChannel(i)->Save(isave)) != IO_OK) return res;
		isave->EndChunk();
		TSTR chanName = name(i);
		int nameLen = chanName.length();
		if (nameLen > 0) {
			nameLen += 1; // the null ending
			isave->BeginChunk(IParticleChannel::kChunkValue3);
			if ((res = isave->Write(&nameLen, sizeof(int), &nb)) != IO_OK) return res;
			isave->EndChunk();
			isave->BeginChunk(IParticleChannel::kChunkValue4);
			if ((res = isave->Write(name(i), sizeof(TCHAR)*(nameLen), &nb)) != IO_OK) return res;
			isave->EndChunk();
			scopeType = scope(i);
			isave->BeginChunk(IParticleChannel::kChunkValue5);
			if ((res = isave->Write(&scopeType, sizeof(int), &nb)) != IO_OK) return res;
			isave->EndChunk();
		}
	}				

	Interface_ID id;
	isave->BeginChunk(IParticleChannel::kChunkReadID);
	id = GetReadID();
	if ((res = isave->Write(&id, sizeof(Interface_ID), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IParticleChannel::kChunkWriteID);
	id = GetWriteID();
	if ((res = isave->Write(&id, sizeof(Interface_ID), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IParticleChannel::kChunkTransferable);
	bool isTransferable = IsTransferable();
	if ((res = isave->Write(&isTransferable, sizeof(bool), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IParticleChannel::kChunkPrivate);
	bool isPrivate = IsPrivateChannel();
	if ((res = isave->Write(&isPrivate, sizeof(bool), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IParticleChannel::kChunkActionHandle);
	ULONG handle = m_creatorHandle;
	if ((res = isave->Write(&handle, sizeof(ULONG), &nb)) != IO_OK) return res;
	isave->EndChunk();

	return IO_OK;
}

IOResult ParticleChannelMult::Load(ILoad* iload)
{
	ULONG nb;
	IOResult res;
	int num;
	Interface_ID id;
	bool isg;
	int subChannelIndex, nameLen=0, scopeType;
	Class_ID channelClassID;
	IObject* newChan = NULL;

	while (IO_OK==(res=iload->OpenChunk()))
	{
		switch(iload->CurChunkID())
		{
		case IParticleChannel::kChunkCount:
			res=iload->Read(&num, sizeof(int), &nb);
			if (res == IO_OK) _count() = num;
			break;
		case IParticleChannel::kChunkValue1:
			res=iload->Read(&subChannelIndex, sizeof(int), &nb);
			break;
		case IParticleChannel::kChunkValue2:
			res = iload->Read(&channelClassID, sizeof(Class_ID), &nb);
			break;
		case IParticleChannel::kChunkData:
			newChan = (IObject*)CreateInstance(REF_TARGET_CLASS_ID, channelClassID);
			_dataObject(subChannelIndex) = newChan;
			_dataChannel(subChannelIndex) = GetParticleChannelInterface(newChan);
			_dataAmountR(subChannelIndex) = GetParticleChannelAmountRInterface(newChan);
			_dataAmountW(subChannelIndex) = GetParticleChannelAmountWInterface(newChan);
			assert(dataChannel(subChannelIndex));
			res = _dataChannel(subChannelIndex)->Load(iload);
			_numSubChannels() = subChannelIndex+1;
			break;
		case IParticleChannel::kChunkValue3:
			res = iload->Read(&nameLen, sizeof(int), &nb);
			break;
		case IParticleChannel::kChunkValue4:
			_name(subChannelIndex) = new TCHAR[nameLen];
			res = iload->Read(_name(subChannelIndex), sizeof(TCHAR)*nameLen, &nb);
			break;
		case IParticleChannel::kChunkValue5:
			res = iload->Read(&scopeType, sizeof(int), &nb);
			if (res == IO_OK) _scope(subChannelIndex) = scopeType;
			break;
		case IParticleChannel::kChunkReadID:
			res=iload->Read(&id, sizeof(Interface_ID), &nb);
			if (res == IO_OK) SetReadID(id);
			break;
		case IParticleChannel::kChunkWriteID:
			res=iload->Read(&id, sizeof(Interface_ID), &nb);
			if (res == IO_OK) SetWriteID(id);
			break;
		case IParticleChannel::kChunkTransferable:
			res=iload->Read(&isg, sizeof(bool), &nb);
			if (res == IO_OK) SetTransferable(isg);
			break;
		case IParticleChannel::kChunkPrivate:
			res=iload->Read(&m_isPrivate, sizeof(bool), &nb);
			break;
		case IParticleChannel::kChunkActionHandle:
			res=iload->Read(&m_creatorHandle, sizeof(ULONG), &nb);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}

	return IO_OK;
}

int ParticleChannelMult::MemoryUsed() const
{
	int mem = sizeof(void*)*7*kMax_SubChannels + 2*sizeof(int);
	for(int i=0; i<numSubChannels(); i++) {
		if (dataChannel(i))
			mem += dataChannel(i)->MemoryUsed();
		if (name(i)) {
			TSTR curName = name(i);
			mem += sizeof(TCHAR)*(curName.length()+1);
		}
	}
	return mem;
}

int ParticleChannelMult::Count() const
{
	return count();
}

void ParticleChannelMult::ZeroCount()
{
	_count() = 0;
	for(int i=0; i<numSubChannels(); i++)
		if (dataAmountW(i))
			_dataAmountW(i)->ZeroCount();
}

bool ParticleChannelMult::SetCount(int n)
{
	_count() = n;
	bool res = true;
	for(int i=0; i<numSubChannels(); i++)
		if (dataAmountW(i))
			res = res && _dataAmountW(i)->SetCount(n);
	return res;
}

int ParticleChannelMult::Delete(int start, int num)
{
	if (start < 0) {
		num += start;
		start = 0;
	}
	if (num <= 0)	return Count();

	if (start < Count())
	{
		if ((start+num) >= Count()) _count() = start;
			else _count() -= num;
	}
	for(int i=0; i<numSubChannels(); i++)
		if (dataAmountW(i))
			_dataAmountW(i)->Delete(start, num);
	return Count();
}

int ParticleChannelMult::Delete(BitArray& toRemove)
{	
	int checkCount = min(toRemove.GetSize(), Count());

	int numRemove = 0;
	for(int i=0; i<checkCount; i++)
		if (toRemove[i] != 0) numRemove++;
	_count() -= numRemove;

	for(int i=0; i<numSubChannels(); i++)
		if (dataAmountW(i))
			_dataAmountW(i)->Delete(toRemove);
	return Count();
}

IObject* ParticleChannelMult::Split(BitArray& toSplit)
{
	ParticleChannelMult* newChannel = (ParticleChannelMult*)Clone();
	Delete(toSplit);
	BitArray reverse = ~toSplit;
	newChannel->Delete(reverse);
	// remove all the local subChannels from the channel that is going out
	for(int i=newChannel->numSubChannels()-1; i>=0; i--) {
		if (newChannel->GetScope(i) == kScope_local)
			newChannel->RemoveSubChannel(i);
	}
	return newChannel;
}

bool ParticleChannelMult::Spawn(Tab<int>& spawnTable)
{
	int checkCount = min(spawnTable.Count(), Count());
	int newCount = 0;

	for(int i=0; i<checkCount; i++)
		if (spawnTable[i] > 0) newCount += spawnTable[i];

	_count() = newCount;
	for(int i=0; i<numSubChannels(); i++)
		if (dataAmountW(i))
			_dataAmountW(i)->Spawn(spawnTable);
	return true;
}

bool ParticleChannelMult::AppendNum(int num)
{
	if (num <= 0)	return true;

	_count() += num;
	for(int i=0; i<numSubChannels(); i++)
		if (dataAmountW(i))
			_dataAmountW(i)->AppendNum(num);
	return true;
}

bool ParticleChannelMult::Append(IObject* channel)
{
	IParticleChannel* iChannel = GetParticleChannelInterface(channel);
	if (iChannel == NULL) return false;
	if (!(GetClassID() == iChannel->GetClassID())) return false;

	IParticleChannelAmountR* iAmount = GetParticleChannelAmountRInterface(channel);
	if (iAmount == NULL) return false;
	int newNum = iAmount->Count();
	if (newNum <= 0) return true;

	IParticleChannelMult* iMult = GetParticleChannelMultInterface(channel);
	if (iMult == NULL) return false;

	bool res = true;

	int oldNum = Count();

	// appending honors naming not subChannel indices
	int oldNumSubChannels = NumSubChannels();
	BitArray needAppendNum;
	needAppendNum.SetSize(oldNumSubChannels);
	needAppendNum.SetAll();

	for(int i=0; i<iMult->NumSubChannels(); i++) {
		if (!iMult->HasSubChannel(i)) continue;
		TCHAR* subName = iMult->GetName(i);
		if (subName == NULL) continue;
		IObject* subChan = iMult->GetSubChannel(i);
		if (subChan == NULL) continue;
		IParticleChannel* iChan = GetParticleChannelInterface(subChan);
		if (iChan == NULL) continue;
		Class_ID chanClassID = iChan->GetClassID();
		int channelIndex;
		IObject* appendToChan = GetSubChannel(subName, chanClassID, channelIndex);
		if (appendToChan == NULL) {
			channelIndex = NumSubChannels();
			res = res && AddSubChannel(channelIndex, chanClassID);
			SetName(channelIndex, subName);
		} else {
			DbgAssert(channelIndex < oldNumSubChannels);
			DbgAssert(needAppendNum[channelIndex]);
			needAppendNum.Clear(channelIndex);
		}
		res = res && _dataAmountW(channelIndex)->Append(subChan);
	}

	for(int i=0; i<oldNumSubChannels; i++) {
		if (needAppendNum[i])
			if (HasSubChannel(i)) {
				DbgAssert(_dataAmountW(i));
				res = res && _dataAmountW(i)->AppendNum(newNum);
			}
	}

	_count() = count() + newNum;
	return res;
}

void ParticleChannelMult::SetName(int subChannel, TCHAR* name)
{
	TSTR curName = name;
	int nameLen = curName.length();
	if (_name(subChannel) != NULL) 
		delete [] _name(subChannel);
	_name(subChannel) = new TCHAR[nameLen+1];
	sprintf(_name(subChannel), "%s", curName);
}

void ParticleChannelMult::SetScope(int subChannel, int scopeType)
{
	_scope(subChannel) = scopeType;
}

IObject* ParticleChannelMult::GetSubChannel(TCHAR* name, Class_ID classID, int& subChannel) const 
{
	if (name == NULL) return NULL;

	for(int i=0; i<NumSubChannels(); i++) {
		if (!HasSubChannel(i)) continue;
		DbgAssert(dataChannel(i));
		if (!(dataChannel(i)->GetClassID() == classID)) continue;
		TSTR curName = GetName(i);
		TSTR getName = name;
		if (curName == getName) {
			subChannel = i;
			return dataObject(i);
		}
	}
	return NULL;
}

bool ParticleChannelMult::AddSubChannel(int subChannel, Class_ID classID)
{
	if ((subChannel < 0) || (subChannel >= kMax_SubChannels)) return false;
	if (dataChannel(subChannel) != NULL) {
		if(dataChannel(subChannel)->GetClassID() == classID) return true; // the subChannel is already present
		RemoveSubChannel(subChannel);
	}
	IObject* newChan = (IObject*)CreateInstance(REF_TARGET_CLASS_ID, classID);
	_dataObject(subChannel) = newChan;
	_dataChannel(subChannel) = GetParticleChannelInterface(newChan);
	_dataAmountR(subChannel) = GetParticleChannelAmountRInterface(newChan);
	_dataAmountW(subChannel) = GetParticleChannelAmountWInterface(newChan);
	if (subChannel >= NumSubChannels()) _numSubChannels() = subChannel+1;
	bool res = false;
	if (dataAmountW(subChannel) != NULL)
		res = _dataAmountW(subChannel)->SetCount(count());
	return res;
}

bool ParticleChannelMult::RemoveSubChannel(int subChannel)
{
	if ((subChannel < 0) || (subChannel >= kMax_SubChannels)) return false;
	if (dataObject(subChannel)) {
		_dataObject(subChannel)->DeleteIObject();
		_dataObject(subChannel) = NULL;
		_dataChannel(subChannel) = NULL;
		_dataAmountR(subChannel) = NULL;
		_dataAmountW(subChannel) = NULL;
		if (name(subChannel)) delete [] _name(subChannel);
		_name(subChannel) = NULL;
		if (numSubChannels() == subChannel+1) {
			for(int i=subChannel-1; i>=0; i--)
				if (HasSubChannel(i)) { _numSubChannels() = i+1; break; }
			if (numSubChannels() == subChannel+1) _numSubChannels() = 0;
		}
		return true;
	}
	return false;
}



} // end of namespace PartASM


