/**********************************************************************
 *<
	FILE:			   composite.h
	DESCRIPTION:	Composite Class Descriptor (declaration)
	CREATED BY:		Hans Larsen
	HISTORY:		   Created 27.sep.07
 *>	Copyright (c) 2007, All Rights Reserved.
 **********************************************************************/
#ifndef __COMPOSITE_H__
#define __COMPOSITE_H__

#include "iparamm2.h"
#include "iFnPub.h"

////////////////////////////////////////////////////////////////////////////////
// ClassDesc
namespace Composite {
	// Even though the instance has a common name, it is situated in an anonymous
	// namespace so it's only available inside this file.
	struct Desc : public ClassDesc2 {
		int         IsPublic();
		void      * Create(BOOL);
		SClass_ID   SuperClassID();
		Class_ID    ClassID();
		HINSTANCE   HInstance();

		const TCHAR     * ClassName();
		const TCHAR     * Category();
		const TCHAR     * InternalName();
	};

}

ClassDesc2* GetCompositeDesc();

////////////////////////////////////////////////////////////////////////////////
// Function Publishing Interface
namespace Composite {
	struct Interface;
}

#define COMPOSITE_INTERFACE Interface_ID(0x5f666b75, 0x5f4367db)
#define GetCompositeInterface( obj ) \
	((Composite::Interface*)obj->GetInterface(COMPOSITE_INTERFACE))

// Definition of the function IDs
namespace Composite {
	namespace Function {
		enum {
			CountLayers,
			AddLayer,
			DeleteLayer,
			DuplicateLayer,
			MoveLayer,
		};
	}
}

// Interface for function publishing
namespace Composite {
	struct Interface : public FPMixinInterface {
		// Prefixing with MXS_ to prevent name clash
		BEGIN_FUNCTION_MAP
			FN_0 ( Function::CountLayers,		TYPE_INT,	MXS_Count		);

			VFN_0( Function::AddLayer,			MXS_AddLayer		);
			VFN_1( Function::DeleteLayer,		MXS_DeleteLayer,	TYPE_INDEX );
			VFN_1( Function::DuplicateLayer,	MXS_DuplicateLayer,	TYPE_INDEX );
			VFN_3( Function::MoveLayer,			MXS_MoveLayer,		TYPE_INDEX, TYPE_INDEX, TYPE_bool );
		END_FUNCTION_MAP

			FPInterfaceDesc* GetDesc();
		virtual int  MXS_Count			(     ) = 0;
		virtual void MXS_AddLayer		(     ) = 0;
		virtual void MXS_DeleteLayer	( int ) = 0;
		virtual void MXS_DuplicateLayer	( int ) = 0;
		virtual void MXS_MoveLayer		( int, int, bool ) = 0;
	};
}

////////////////////////////////////////////////////////////////////////////////
#endif // __COMPOSITE_H__
