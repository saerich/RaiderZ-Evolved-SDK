/**********************************************************************
 *<
	FILE:			ParticleChannelDesc.h

	DESCRIPTION:	Class Descriptor for Particle Channels (declaration)
					Class Descriptors for specific Particle Channels (declaration)
											 
	CREATED FOR:	Oleg Bayborodin

	HISTORY:		created created 2003-05-18

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#ifndef  _PARTICLECHANNELDESC_H_
#define  _PARTICLECHANNELDESC_H_

#include "iparamb2.h"

namespace PartASM {

//	ParticleChannel-generic Descriptor declarations
class ParticleChannelDesc: public ClassDesc2 {
public:
	int 			IsPublic() { return FALSE; }
	virtual void*	Create(BOOL loading = FALSE) = 0;
	const TCHAR *	ClassName();
	SClass_ID		SuperClassID();
	Class_ID		SubClassID();
	virtual Class_ID	ClassID() = 0;
	const TCHAR* 	Category();
	virtual const TCHAR* InternalName() = 0;
};

class ParticleChannelMultDesc: public ParticleChannelDesc {
public:
	void*	Create(BOOL loading = FALSE);
	Class_ID	ClassID();
	const TCHAR* InternalName();
};



} // end of namespace PartASM

#endif // _PARTICLECHANNELDESC_H_