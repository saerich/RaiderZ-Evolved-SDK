/**********************************************************************
 *<
	FILE:			PAOperatorObjectDesc.h

	DESCRIPTION:	SimpleSpin Operator Class Descriptor (declaration)
											 
	CREATED BY:	Oleg Bayborodin

	HISTORY: created 05-17-03

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#ifndef  _PAOPERATOROBJECTDESC_H_
#define  _PAOPERATOROBJECTDESC_H_

#include "iparamb2.h"

namespace PartASM {

//	Descriptor declarations
class PAOperatorObjectDesc: public ClassDesc2 {
	public:
	~PAOperatorObjectDesc();
	int 			IsPublic();
	void *			Create(BOOL loading = FALSE);
	const TCHAR *	ClassName();
	SClass_ID		SuperClassID();
	Class_ID		ClassID();
	Class_ID		SubClassID();
	const TCHAR* 	Category();

	const TCHAR*	InternalName();
	HINSTANCE		HInstance();

	INT_PTR Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3);

	static HBITMAP m_depotIcon;
	static HBITMAP m_depotMask;
};

} // end of namespace PartASM

#endif // _PAOPERATOROBJECTDESC_H_