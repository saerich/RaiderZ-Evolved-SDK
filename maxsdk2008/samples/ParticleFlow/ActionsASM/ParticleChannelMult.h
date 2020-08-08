/**********************************************************************
 *<
	FILE:			ParticleChannelMult.h

	DESCRIPTION:	ParticleChannelMult channel interface
					This generic channel is used to store multiple data

	CREATED FOR:	Oleg Bayborodin

	HISTORY:		created 2003-05-18

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#ifndef _PARTICLECHANNELMULT_H_
#define _PARTICLECHANNELMULT_H_

#include "PartAsm_GlobalVariables.h"

#include "IParticleChannel.h"
#include "IParticleChannelMult.h"
#include "IParticleChannelAmount.h"

namespace PartASM {

class ParticleChannelMult:	public IObject, 
							public IParticleChannel,
							public IParticleChannelAmountR,
							public IParticleChannelAmountW,
							public IParticleChannelMult
{
public:

	ParticleChannelMult();
	virtual ~ParticleChannelMult();

	// from IObject interface
	TCHAR* GetIObjectName();
	int NumInterfaces() const { return 4; }
	BaseInterface* GetInterfaceAt(int index) const;
	BaseInterface* GetInterface(Interface_ID id);
	void AcquireIObject();
	void ReleaseIObject();
	void DeleteIObject();

	// from FPMixinInterface
	FPInterfaceDesc* GetDescByID(Interface_ID id);

	// from IParticleChannel
	Class_ID GetClassID() const;
	IObject*  Clone() const;
	IOResult Save(ISave* isave) const;
	IOResult Load(ILoad* iload);
	int MemoryUsed() const;

	// from IParticleChannelAmountR
	int Count() const;

	// from IParticleChannelAmountW
	void	ZeroCount();
	bool	SetCount(int n);
	int	Delete(int start,int num);
	int	Delete(BitArray& toRemove);
	IObject*	Split(BitArray& toSplit);
	bool	Spawn( Tab<int>& spawnTable );
	bool	AppendNum(int num);
	bool	Append(IObject* channel);

	// from IParticleChannelMult
enum { kMax_SubChannels = 100 };
	int			NumSubChannels()				const { return m_numSubChannels; }
	bool		HasSubChannel(int subChannel)	const { return (dataObject(subChannel) != NULL); }
	IObject*	GetSubChannel(int subChannel)	const { return dataObject(subChannel); }
	TCHAR*		GetName(int subChannel)			const { return name(subChannel); }
	void		SetName(int subChannel, TCHAR* name);
	IObject*	GetSubChannel(TCHAR* name, Class_ID classID, int& subChannel) const;
	bool		AddSubChannel(int subChannel, Class_ID classID);
	bool		RemoveSubChannel(int subChannel);
	int			GetScope(int subChannel)		const { return scope(subChannel); }
	void		SetScope(int subChannel, int scopeType);

protected:
	// const access to class members
	IObject*					dataObject(int subChannel)	const	{ return m_dataObject[subChannel]; }
	TCHAR*						name(int subChannel)		const	{ return m_name[subChannel]; }
	int							scope(int subChannel)		const	{ return m_scope[subChannel]; }
	IParticleChannel*			dataChannel(int subChannel)	const	{ return m_dataChannel[subChannel]; }
	IParticleChannelAmountR*	dataAmountR(int subChannel)	const	{ return m_dataAmountR[subChannel]; }
	IParticleChannelAmountW*	dataAmountW(int subChannel)	const	{ return m_dataAmountW[subChannel]; }
	int							count()						const	{ return m_count; }
	int							numSubChannels()			const	{ return m_numSubChannels; }

	// access to class members
	IObject*&					_dataObject(int subChannel)		{ return m_dataObject[subChannel]; }
	TCHAR*&						_name(int subChannel)			{ return m_name[subChannel]; }
	int&						_scope(int subChannel)			{ return m_scope[subChannel]; }
	IParticleChannel*&			_dataChannel(int subChannel)	{ return m_dataChannel[subChannel]; }
	IParticleChannelAmountR*&	_dataAmountR(int subChannel)	{ return m_dataAmountR[subChannel]; }
	IParticleChannelAmountW*&	_dataAmountW(int subChannel)	{ return m_dataAmountW[subChannel]; }
	int&						_count()						{ return m_count; }
	int&						_numSubChannels()				{ return m_numSubChannels; }

private:
	// data
	IObject*					m_dataObject[kMax_SubChannels];
	// channel names
	TCHAR*						m_name[kMax_SubChannels];
	// scope type
	int							m_scope[kMax_SubChannels];

	// to speed up data retrieval
	IParticleChannel*			m_dataChannel[kMax_SubChannels];
	IParticleChannelAmountR*	m_dataAmountR[kMax_SubChannels];
	IParticleChannelAmountW*	m_dataAmountW[kMax_SubChannels];

	// for count synchronization
	int	m_count; 
	// for fast channel looping
	int m_numSubChannels;
};

} // end of namespace PartASM

#endif // _PARTICLECHANNELMULT_H_
