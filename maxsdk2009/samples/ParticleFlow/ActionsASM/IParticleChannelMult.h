/*! \file IParticleChannelMult.h
    \brief	Channel-generic interfaces for particle channels that store multiple channels data.
*/
/**********************************************************************
 *<
	CREATED FOR:	Oleg Bayborodin

	HISTORY:		created 2003-05-17

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#ifndef _IPARTICLECHANNELMULT_H_
#define _IPARTICLECHANNELMULT_H_

//#include "max.h"

#include "PFExport.h"

// generic particle channel "multiple"
// interface ID
#define PARTICLECHANNELMULT_INTERFACE Interface_ID(0x29506700, 0x1eb34500)
#define GetParticleChannelMultInterface(obj) ((IParticleChannelMult*)obj->GetInterface(PARTICLECHANNELMULT_INTERFACE))

class IParticleChannelMult : public FPMixinInterface
{
public:

      /** @defgroup IParticleChannelMult IParticleChannelMult.h
      *  @{
      */

	/*! \fn virtual int	NumSubChannels() const = 0;
	*  \brief returns number of subchannels that maybe present. It means that there is no channels beyond NumSubChannels() but some subchannels with index less than NumSubChannels() may not be present (NULL)
	*/
	virtual int	NumSubChannels() const = 0;

	/*! \fn virtual bool	HasSubChannel(int subChannel) const = 0;
	*  \brief if subchannel is present then returns true
	*/
	virtual bool	HasSubChannel(int subChannel) const = 0;

	/*! \fn virtual IObject* GetSubChannel(int subChannel) const = 0;
	*  \brief if subchannel is present then returns the subchannel IObject pointer
	*/
	virtual IObject* GetSubChannel(int subChannel) const = 0;

	/*! \fn virtual TCHAR* GetName(int subChannel) const = 0;
	*  \brief if subchannel is present then returns the subchannel's name (NULL ended TCHAR string)
	*/
	virtual TCHAR* GetName(int subChannel) const = 0;

	/*! \fn virtual void SetName(int subChannel, TCHAR* name) = 0;
	*  \brief if subchannel is present then sets the subchannel name
	*/
	virtual void SetName(int subChannel, TCHAR* name) = 0;

	/*! \fn virtual IObject* GetSubChannel(TCHAR* name, Class_ID classID, int& subChannel) const = 0;
	*  \brief if subchannel with the name and classID is present then returns the subchannel IObject pointer and subChannel index
	*/
	virtual IObject* GetSubChannel(TCHAR* name, Class_ID classID, int& subChannel) const = 0;

	/*! \fn virtual bool	AddSubChannel(int subChannel, Class_ID classID) = 0;
	*  \brief Adds subchannel defined by the classID. Returns True is done successfully.
	*/
	virtual bool	AddSubChannel(int subChannel, Class_ID classID) = 0;

	/*! \fn virtual bool	RemoveSubChannel(int subChannel) = 0;
	*  \brief if subchannel is removed (or was empty) then returns true
	*/
	virtual bool	RemoveSubChannel(int subChannel) = 0;

	/*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELMULT_INTERFACE); }
	*  \brief 
	*/

	enum { kScope_global, kScope_local };
	/*! \fn virtual int	GetScope(int subChannel) = 0;
	*  \brief returns scope type of the subChannel. It defines if the subChannel data will be transferred to the next event
	*/
	virtual int	GetScope(int subChannel) const = 0;

	/*! \fn virtual void SetScope(int subChannel) = 0;
	*  \brief sets scope type of the subChannel. It defines if the subChannel data will be transferred to the next event
	*/
	virtual void SetScope(int subChannel, int scopeType) = 0;

	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELMULT_INTERFACE); }
};

#endif // _IPARTICLECHANNELMULT_H_
