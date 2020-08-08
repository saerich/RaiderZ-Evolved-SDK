/**********************************************************************
 *<
	FILE:			PartAsm_GlobalVariables.h

	DESCRIPTION:	Collection of global variables and constants (declaration).
											 
	CREATED FOR:	Oleg Bayborodin

	HISTORY:		2003-05-17

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#ifndef _PARTASM_GLOBALVARIABLES_H_
#define _PARTASM_GLOBALVARIABLES_H_

//#include "PFOperatorDisplayDesc.h"
//#include "PFOperatorRenderDesc.h"
//#include "PFOperatorSimpleBirthDesc.h"
#include "ParticleChannelDesc.h"
#include "PAOperatorDisplayDesc.h"
#include "PAOperatorValueDesc.h"
#include "PAOperatorCopyDesc.h"
#include "PAOperatorFunctionDesc.h"
#include "PAOperatorRandomDesc.h"
#include "PAOperatorConditionDesc.h"
#include "PATestDesc.h"
#include "PAOperatorObjectDesc.h"

namespace PartASM {

extern HINSTANCE hInstance;

enum { kNum3DVectorValues = 3 };


//extern PFOperatorRenderDesc					ThePFOperatorRenderDesc;
//extern PFOperatorSimpleBirthDesc			ThePFOperatorSimpleBirthDesc;

//extern PFTestDurationDesc				ThePFTestDurationDesc;
//extern PFTestSpawnDesc					ThePFTestSpawnDesc;
//extern PFTestSpawnOnCollisionDesc		ThePFTestSpawnOnCollisionDesc;

//extern PFOperatorSimpleBirthStateDesc		ThePFOperatorSimpleBirthStateDesc;
//extern PFOperatorSimplePositionStateDesc	ThePFOperatorSimplePositionStateDesc;
//extern PFOperatorInstanceShapeStateDesc		ThePFOperatorInstanceShapeStateDesc;

//==================================================================================//
//						Particle Channels											//
//==================================================================================//
#define ParticleChannelMult_Class_ID		Class_ID(0x26c97b01, ParticleChannelClassID_PartB)

extern ParticleChannelMultDesc		TheParticleChannelMultDesc;


//==================================================================================//
//						Operators													//
//==================================================================================//
#define PAOperatorDisplay_Class_ID			Class_ID(0x26c97c01, PFActionClassID_PartB)
#define PAOperatorValue_Class_ID			Class_ID(0x26c97c02, PFActionClassID_PartB)
#define PAOperatorCopy_Class_ID				Class_ID(0x26c97c03, PFActionClassID_PartB)
#define PAOperatorFunction_Class_ID			Class_ID(0x26c97c04, PFActionClassID_PartB)
#define PAOperatorRandom_Class_ID			Class_ID(0x26c97c05, PFActionClassID_PartB)
#define PAOperatorCondition_Class_ID		Class_ID(0x26c97c06, PFActionClassID_PartB)
#define PATest_Class_ID						Class_ID(0x26c97c07, PFActionClassID_PartB)
#define PAOperatorObject_Class_ID			Class_ID(0x26c97c08, PFActionClassID_PartB)

extern PAOperatorDisplayDesc				ThePAOperatorDisplayDesc;
extern PAOperatorValueDesc					ThePAOperatorValueDesc;
extern PAOperatorCopyDesc					ThePAOperatorCopyDesc;
extern PAOperatorFunctionDesc				ThePAOperatorFunctionDesc;
extern PAOperatorRandomDesc					ThePAOperatorRandomDesc;
extern PAOperatorConditionDesc				ThePAOperatorConditionDesc;
extern PATestDesc							ThePATestDesc;
extern PAOperatorObjectDesc					ThePAOperatorObjectDesc;


//==================================================================================//
//						Tests														//
//==================================================================================//




} // end of namespace PartASM

#endif // _PARTASM_GLOBALVARIABLES_H_