//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
/**********************************************************************
 	FILE: SimpleFaceDataValidator.h

	DESCRIPTION:	Header file for validator ojbects for SimpleFaceData.
					Class declarations for:
					- face index validator
					- parameter type validator
					- parameter array size validator
					- class id format validator

	AUTHOR: ktong - created 02.17.2006
***************************************************************************/

#ifndef _SIMPLEFACEDATAVALIDATOR_H_
#define _SIMPLEFACEDATAVALIDATOR_H_

#include "ISimpleFaceDataChannel.h"

// face index validator
class ParamIndexValidator : public FPValidator {
	bool Validate(FPInterface* fpi, FunctionID fid, int paramNum, FPValue& val, TSTR& msg);
};

// value parameter type validator
class ParamTypeValidator : public FPValidator {
	bool Validate(FPInterface* fpi, FunctionID fid, int paramNum, FPValue& val, TSTR& msg);
};

// value array validator - size and element type
class ParamArrayValidator : public FPValidator {
	bool Validate(FPInterface* fpi, FunctionID fid, int paramNum, FPValue& val, TSTR& msg);
};

// bitarray size validator
class ParamSelectValidator : public FPValidator {
	bool Validate(FPInterface* fpi, FunctionID fid, int paramNum, FPValue& val, TSTR& msg);
};

// class id array format validator
class ParamClassIDValidator : public FPValidator {
	bool Validate(FPInterface* fpi, FunctionID fid, int paramNum, FPValue& val, TSTR& msg);
};

// object type validator. Emesh and Epoly only.
class ParamObjectValidator : public FPValidator {
	bool Validate(FPInterface* fpi, FunctionID fid, int paramNum, FPValue& val, TSTR& msg);
};

#endif
//EOF