/**********************************************************************
 *<
	FILE:			PAOperatorFunctionDesc.h

	DESCRIPTION:	SimpleSpin Operator Class Descriptor (declaration)
											 
	CREATED FOR:	Oleg Bayborodin

	HISTORY: created 05-17-03

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#ifndef  _PAOPERATORFUNCTIONDESC_H_
#define  _PAOPERATORFUNCTIONDESC_H_

#include "iparamb2.h"

namespace PartASM {

//	Descriptor declarations
class PAOperatorFunctionDesc: public ClassDesc2 {
	public:
	~PAOperatorFunctionDesc();
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

#endif // _PAOPERATORFUNCTIONDESC_H_