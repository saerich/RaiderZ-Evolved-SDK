/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file basetypes.h 
	\brief Defines a set of basic platform-independent types used by all \SDKName components.
	\ingroup kypathdata_generic */

#ifndef KY_BASETYPES_H
#define KY_BASETYPES_H

#include <kypathdata/generic/platform.h>

/*!	Represents a null value.
	\ingroup kypathdata_generic */
#define KY_NULL 0
template <typename T>
T* KY_PTRMAXVAL() { return reinterpret_cast<T*>(-1); }


/*******************************************************************************************
      oooooo   oooooo     oooo  ooooo  ooooo      ooo          .ooo          .o
       `888.    `888.     .8'   `888'  `888b.     `8'        .88'          .d88
        `888.   .8888.   .8'     888    8 `88b.    8        d88'         .d'888
         `888  .8'`888. .8'      888    8   `88b.  8       d888P"Ybo.  .d'  888
          `888.8'  `888.8'       888    8     `88b.8       Y88[   ]88  88ooo888oo
           `888'    `888'        888    8       `888       `Y88   88P       888
            `8'      `8'        o888o  o8o        `8        `88bod8'       o888o
*******************************************************************************************/

#if defined(KY_WIN64)

#define KYLITTLEENDIAN  /* This is a little endian machine */
#define KYINT64_DEFINED /* KyInt64 is defined */

typedef long KyFixed;
typedef int  KyInt32;
typedef unsigned int KyUInt32;
typedef __int64 KyInt64;
typedef unsigned __int64 KyUInt64;
typedef short KyInt16;
typedef unsigned short KyUInt16;
typedef unsigned char KyUInt8;
typedef signed char KyInt8;
typedef float KyFloat32;
typedef double KyFloat64;
typedef unsigned char KyBool;

#endif /* defined (KY_WIN64) */

/*******************************************************************************************
	  oooooo   oooooo     oooo  ooooo  ooooo      ooo        .oooo.      .oooo.
	   `888.    `888.     .8'   `888'  `888b.     `8'      .dP""Y88b   .dP""Y88b
		`888.   .8888.   .8'     888    8 `88b.    8             ]8P'        ]8P'
		 `888  .8'`888. .8'      888    8   `88b.  8           <88b.       .d8P'
		  `888.8'  `888.8'       888    8     `88b.8            `88b.    .dP'
		   `888'    `888'        888    8       `888       o.   .88P   .oP     .o
			`8'      `8'        o888o  o8o        `8       `8bd88P'    8888888888
 *******************************************************************************************/

#if (defined (KY_WIN32) || defined (KY_XBOX)) // in both cases, next section is relevant

#define KYLITTLEENDIAN  /* This is a little endian machine */
#define KYINT64_DEFINED /* KyInt64 is defined */

typedef long KyFixed; 				/*!< Type used internally to represent a fixed integer. \ingroup kypathdata_generic */
typedef int  KyInt32; 				/*!< Type used internally to represent a 32-bit integer. \ingroup kypathdata_generic */
typedef unsigned int KyUInt32;		/*!< Type used internally to represent an unsigned 32-bit integer. \ingroup kypathdata_generic */
typedef __int64 KyInt64;			/*!< Type used internally to represent a 64-bit integer. \ingroup kypathdata_generic */
typedef unsigned __int64 KyUInt64;	/*!< Type used internally to represent an unsigned 64-bit integer. \ingroup kypathdata_generic */
typedef short KyInt16;				/*!< Type used internally to represent a 16-bit integer. \ingroup kypathdata_generic */
typedef unsigned short KyUInt16;	/*!< Type used internally to represent an unsigned 16-bit integer. \ingroup kypathdata_generic */
typedef unsigned char KyUInt8;		/*!< Type used internally to represent an unsigned 8-bit integer. \ingroup kypathdata_generic */
typedef signed char KyInt8;			/*!< Type used internally to represent an 8-bit integer. \ingroup kypathdata_generic */
typedef float KyFloat32;			/*!< Type used internally to represent a 32-bit floating-point number. \ingroup kypathdata_generic */
typedef double KyFloat64;			/*!< Type used internally to represent a 64-bit floating-point number. \ingroup kypathdata_generic */
typedef unsigned char KyBool;		/*!< Type used internally to represent a Boolean. May be either #KY_TRUE or #KY_FALSE. \ingroup kypathdata_generic */

#endif /* (defined (KY_WIN32) || defined (KY_XBOX)) */

/*******************************************************************************************
        ooooooo  ooooo oooooooooooo ooooo      ooo   .oooooo.   ooooo      ooo 
         `8888    d8'  `888'     `8 `888b.     `8'  d8P'  `Y8b  `888b.     `8' 
           Y888..8P     888          8 `88b.    8  888      888  8 `88b.    8  
            `8888'      888oooo8     8   `88b.  8  888      888  8   `88b.  8  
           .8PY888.     888    "     8     `88b.8  888      888  8     `88b.8  
          d8'  `888b    888       o  8       `888  `88b    d88'  8       `888  
        o888o  o88888o o888ooooood8 o8o        `8   `Y8bood8P'  o8o        `8  
 *******************************************************************************************/

#if defined(KY_XBOX360)

#define KYBIGENDIAN     /* This is a big endian machine */
#define KYINT64_DEFINED /* KyInt64 is defined */

typedef long KyFixed;
typedef int  KyInt32;
typedef unsigned int KyUInt32;
typedef __int64 KyInt64;
typedef unsigned __int64 KyUInt64;
typedef short KyInt16;
typedef unsigned short KyUInt16;
typedef unsigned char KyUInt8;
typedef signed char KyInt8;
typedef float KyFloat32;
typedef double KyFloat64;
typedef unsigned char KyBool;

#endif /* KY_XBOX360 */

/*******************************************************************************************
                     ooooooooo.     .oooooo..o    .oooo.
                     `888   `Y88.  d8P'    `Y8  .dP""Y88b
                      888   .d88'  Y88bo.             ]8P'
                      888ooo88P'    `"Y8888o.       .d8P'
                      888               `"Y88b    .dP'
                      888          oo     .d8P  .oP     .o
                     o888o         8""88888P'   8888888888
 *******************************************************************************************/
#if defined(KY_PS2)

#define KYLITTLEENDIAN     /* This is a little endian machine */

typedef int KyFixed;
typedef int KyInt32;
typedef unsigned int KyUInt32;
typedef short KyInt16;
typedef unsigned short KyUInt16;
typedef unsigned char KyUInt8;
typedef signed char KyInt8;
typedef float KyFloat32;
typedef double KyFloat64;
typedef unsigned char KyBool;

#endif /* KY_PS2 */

/*******************************************************************************************
                    .oooooo.       .oooooo.    ooooo      ooo
                   d8P'  `Y8b     d8P'  `Y8b   `888b.     `8'
                  888            888            8 `88b.    8
                  888            888            8   `88b.  8
                  888     ooooo  888            8     `88b.8
                  `88.    .88'   `88b    ooo    8       `888
                   `Y8bood8P'     `Y8bood8P'   o8o        `8
 *******************************************************************************************/
#if defined (KY_GCN)

#define KYBIGENDIAN         /* This is a big endian machine */

typedef long KyFixed;
typedef int  KyInt32;
typedef unsigned int KyUInt32;
typedef short KyInt16;
typedef unsigned short KyUInt16;
typedef unsigned char KyUInt8;
typedef signed char KyInt8;
typedef float KyFloat32;
typedef double KyFloat64;
typedef unsigned char KyBool;

#endif /* KY_GCN */

/*******************************************************************************************
						oooooo   oooooo     oooo  o8o   o8o  
						 `888.    `888.     .8'   `"'   `"'  
						  `888.   .8888.   .8'   oooo  oooo  
						   `888  .8'`888. .8'    `888  `888  
							`888.8'  `888.8'      888   888  
							 `888'    `888'       888   888  
							  `8'      `8'       o888o o888o 
*******************************************************************************************/
#if defined (KY_WII)

#define KYBIGENDIAN    /* This is a big endian machine */
#define KYINT64_DEFINED /* KyInt64 is defined */

typedef long KyFixed;
typedef int  KyInt32;
typedef unsigned int KyUInt32;
typedef long long KyInt64;
typedef unsigned long long KyUInt64;
typedef short KyInt16;
typedef unsigned short KyUInt16;
typedef unsigned char KyUInt8;
typedef signed char KyInt8;
typedef float KyFloat32;
typedef double KyFloat64;
typedef unsigned char KyBool;

#endif /* KY_WII */

/*******************************************************************************************
  ooooo         ooooo  ooooo      ooo  ooooo     ooo  ooooooo  ooooo       .ooo         .o
  `888'         `888'  `888b.     `8'  `888'     `8'   `8888    d8'      .88'         .d88
   888           888    8 `88b.    8    888       8      Y888..8P       d88'        .d'888
   888           888    8   `88b.  8    888       8       `8888'       d888P"Ybo. .d'  888
   888           888    8     `88b.8    888       8      .8PY888.      Y88[   ]88 88ooo888oo
   888       o   888    8       `888    `88.    .8'     d8'  `888b     `Y88   88P      888
  o888ooooood8  o888o  o8o        `8      `YbodP'     o888o  o88888o    `88bod8'      o888o
 *******************************************************************************************/

#if defined(KY_LINUX64)

#define KYLITTLEENDIAN  /* This is a little endian machine */
#define KYINT64_DEFINED /* KyInt64 is defined */

typedef long KyFixed;
typedef int  KyInt32;
typedef unsigned int KyUInt32;
typedef long KyInt64;
typedef unsigned long KyUInt64;
typedef short KyInt16;
typedef unsigned short KyUInt16;
typedef unsigned char KyUInt8;
typedef signed char KyInt8;
typedef float KyFloat32;
typedef double KyFloat64;
typedef unsigned char KyBool;

#endif // defined(KY_LINUX64)

/*******************************************************************************************
      ooooo         ooooo  ooooo      ooo  ooooo     ooo  ooooooo  ooooo     .oooo.      .oooo.
      `888'         `888'  `888b.     `8'  `888'     `8'   `8888    d8'    .dP""Y88b   .dP""Y88b
       888           888    8 `88b.    8    888       8      Y888..8P            ]8P'        ]8P'
       888           888    8   `88b.  8    888       8       `8888'           <88b.       .d8P'
       888           888    8     `88b.8    888       8      .8PY888.           `88b.    .dP'
       888       o   888    8       `888    `88.    .8'     d8'  `888b     o.   .88P   .oP     .o
      o888ooooood8  o888o  o8o        `8      `YbodP'     o888o  o88888o   `8bd88P'    8888888888
 *******************************************************************************************/

#if defined (KY_LINUX32)

#define KYLITTLEENDIAN  /* This is a little endian machine */
#define KYINT64_DEFINED /* KyInt64 is defined */

; /* syntax analyser bug work around (on genfullxml) */
typedef long KyFixed;
typedef int  KyInt32;
typedef unsigned int KyUInt32;
typedef long long KyInt64;
typedef unsigned long long KyUInt64;
typedef short KyInt16;
typedef unsigned short KyUInt16;
typedef unsigned char KyUInt8;
typedef signed char KyInt8;
typedef float KyFloat32;
typedef double KyFloat64;
typedef unsigned char KyBool;

#endif /* KY_LINUX32 */

/*******************************************************************************************
                     ooooooooo.     .oooooo..o  ooooooooo.
                     `888   `Y88.  d8P'    `Y8  `888   `Y88.
                      888   .d88'  Y88bo.        888   .d88'
                      888ooo88P'    `"Y8888o.    888ooo88P'
                      888               `"Y88b   888
                      888          oo     .d8P   888
                     o888o         8""88888P'   o888o
 *******************************************************************************************/

#if defined (KY_PSP)

#define KYLITTLEENDIAN         /* This is a little endian machine */

typedef int KyFixed;
typedef int KyInt32;
typedef unsigned int KyUInt32;
typedef short KyInt16;
typedef unsigned short KyUInt16;
typedef unsigned char KyUInt8;
typedef signed char KyInt8;
typedef float KyFloat32;
typedef double KyFloat64;
typedef unsigned char KyBool;

#endif /* KY_PSP */

/*******************************************************************************************
                     ooooooooo.     .oooooo..o      .oooo.
                     `888   `Y88.  d8P'    `Y8	  .dP""Y88b
                      888   .d88'  Y88bo.		       ]8P'
                      888ooo88P'    `"Y8888o.        <88b.
                      888               `"Y88b	      `88b.
                      888          oo     .d8P	  o.   .88P
                     o888o         8""88888P'	   `8bd88P'
 *******************************************************************************************/
#if defined (KY_PS3)

#define KYBIGENDIAN     /* This is a big endian machine */
#define KYINT64_DEFINED /* KyInt64 is defined */

typedef long KyFixed;
typedef int  KyInt32;
typedef unsigned int KyUInt32;
typedef long long KyInt64;
typedef unsigned long long KyUInt64;
typedef short KyInt16;
typedef unsigned short KyUInt16;
typedef unsigned char KyUInt8;
typedef signed char KyInt8;
typedef float KyFloat32;
typedef double KyFloat64;
typedef unsigned char KyBool;

#endif /* KY_PS3 */

#define KY_TRUE KyBool(1)			/*!< One of the possible values of the #KyBool type. Evaluates to \c true. \ingroup kypathdata_generic */
#define KY_FALSE KyBool(0)			/*!< One of the possible values of the #KyBool type. Evaluates to \c false. \ingroup kypathdata_generic */

/* Limits of types */

/*! The maximum value that can be stored in a variable of the #KyInt32 type. \ingroup kypathdata_generic */
#define KyInt32MAXVAL       0x7FFFFFFF

/*! The minimum value that can be stored in a variable of the #KyInt32 type. \ingroup kypathdata_generic */
#define KyInt32MINVAL       (KyInt32(0x80000000))

/*! The maximum value that can be stored in a variable of the #KyUInt32 type. \ingroup kypathdata_generic */
#define KyUInt32MAXVAL      (KyUInt32)(0xFFFFFFFF)

/*! The minimum value that can be stored in a variable of the #KyUInt32 type. \ingroup kypathdata_generic */
#define KyUInt32MINVAL      0x00000000

/*! The maximum value that can be stored in a variable of the #KyFloat32 type. \ingroup kypathdata_generic */
#define KyFloat32MAXVAL     (KyFloat32)(3.402823466e+38F)

/*! The maximum value that can be stored in a variable of the #KyInt16 type. \ingroup kypathdata_generic */
#define KyInt16MAXVAL       0x7FFF

/*! The minimum value that can be stored in a variable of the #KyInt16 type. \ingroup kypathdata_generic */
#define KyInt16MINVAL       (KyInt16(0x8000))

/*! The maximum value that can be stored in a variable of the #KyUInt16 type. \ingroup kypathdata_generic */
#define KyUInt16MAXVAL      0xFFFF

/*! The minimum value that can be stored in a variable of the #KyUInt16 type. \ingroup kypathdata_generic */
#define KyUInt16MINVAL      0x0000

#ifdef KYINT64_DEFINED
/*! The maximum value that can be stored in a variable of the #KyInt64 type. \ingroup kypathdata_generic */
# define KyInt64MAXVAL       0x7FFFFFFFFFFFFFFF
/*! The minimum value that can be stored in a variable of the #KyInt64 type. \ingroup kypathdata_generic */
# define KyInt64MINVAL       0x8000000000000000
/*! The maximum value that can be stored in a variable of the #KyUInt64 type. \ingroup kypathdata_generic */
# define KyUInt64MAXVAL      (KyUInt64)(0xFFFFFFFFFFFFFFFF)
/*! The minimum value that can be stored in a variable of the #KyUInt64 type. \ingroup kypathdata_generic */
# define KyUInt64MINVAL      0x0000000000000000
#endif // KYINT64_DEFINED

typedef void* KyFilePtr;

#ifndef KYFORCEENUMSIZEINT
#define KYFORCEENUMSIZEINT ((KyInt32)((~((KyUInt32)0))>>1))
#endif /* (!defined(KYFORCEENUMSIZEINT)) */



/*!	Defines a type that can be returned by methods or functions in the \SDKName SDK to
	indicate the results of the requested computations. For a list of possible values, see the Kaim::Result
	namespace.
	\ingroup kypathdata_generic */
typedef KyInt32 KyResult;

//
// Error checking
//
namespace Kaim
{
	namespace Result
	{
		// check error code validity
		/*!	Returns true if the specified result code indicates that the requested operation
			completed successfully.
			\ingroup kypathdata_generic */
		inline bool Check(KyResult result) { return result >= 0; }

		/*!	Returns true if the specified result code indicates that the requested operation
			failed.
			\ingroup kypathdata_generic */
		inline bool Fail(KyResult result) { return result < 0; }

		// default error codes
		static const KyResult Success					=  0;	/*!< Indicates that the requested operation was completed successfully. \ingroup kypathdata_generic */
		static const KyResult Failure					= -1;	/*!< Indicates that the requested operation could not be completed successfully. This return indicates the failure of a "Try" request. \ingroup kypathdata_generic */
		static const KyResult NotImplemented			= -2;	/*!< Indicates that the requested operation is not implemented in the current version of the SDK. \ingroup kypathdata_generic */
		static const KyResult NotSupported				= -3;	/*!< Indicates that the requested operation cannot be completed given the current state of the objects or data involved. \ingroup kypathdata_generic */
		static const KyResult BadParameter				= -4;	/*!< Indicates that at least one parameter provided for the requested operation was invalid. \ingroup kypathdata_generic */
		static const KyResult PreconditionFailure		= -5;	/*!< Indicates that at least one pre-requisite for the requested operation has not been fulfilled. \ingroup kypathdata_generic */
		static const KyResult Duplicate					= -6;	/*!< Indicates that you have attempted to add a duplicate item to a container that can only hold unique elements. \ingroup kypathdata_generic */
		static const KyResult NotFound					= -7;	/*!< Indicates that a requested object or data does not exist. \ingroup kypathdata_generic */
		static const KyResult OutOfMemory				= -8;	/*!< Indicates that memory allocation failed during the requested operation. \ingroup kypathdata_generic */
		static const KyResult Unknown					= -9;	/*!< Indicates that an unknown error occurred. \ingroup kypathdata_generic */
		static const KyResult Uninitialized				= -10;	/*!< Indicates that a requested component was not initialized. \ingroup kypathdata_generic */
		static const KyResult AlreadyInitialized		= -11;	/*!< Indicates that a requested component was already initialized. \ingroup kypathdata_generic */
		static const KyResult BadVersion                = -12;  /*!< Indicates that an incompatibility has occurred between different versions of the SDK. \ingroup kypathdata_generic */
		static const KyResult Base_FirstFreeErrorCode	= -13;  // first available code
	}
}

/*! Shorthand for Result::Failure. \ingroup kypathdata_generic */
#define KY_ERROR Kaim::Result::Failure

/*! Shorthand for Result::Success. \ingroup kypathdata_generic */
#define KY_SUCCESS Kaim::Result::Success

#define KY_FORWARD_ERROR_NO_LOG(expression) { KyResult error = (expression); if (Kaim::Result::Fail(error)) { return error; }}

/*! Shorthand for Result::Fail. \ingroup kypathdata_generic */
#define KY_FAILED(expression) (Kaim::Result::Fail(expression))

/*! Shorthand for Result::Check. \ingroup kypathdata_generic */
#define KY_SUCCEEDED(expression) (Kaim::Result::Check(expression))


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Support for 32/64 bits pointer arithmetic
///////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Kaim
{
	namespace details
	{
		template <int size>//
		struct ptr_difference;

		template <int size>
		struct ptr_size;

		template <>
		struct ptr_difference<4>
		{
			typedef KyInt32 type;
		};

		template <>
		struct ptr_size<4>
		{
			typedef KyUInt32 type;
		};

#ifdef KYINT64_DEFINED
		template <>
		struct ptr_difference<8>
		{
			typedef KyInt64 type;
		};

		template <>
		struct ptr_size<8>
		{
			typedef KyUInt64 type;
		};
#endif // KYINT64_DEFINED
	} // details

	template <typename T>
	struct ptr_difference : details::ptr_difference<sizeof(T)>
	{
	};

	template <typename T>
	struct ptr_size : details::ptr_size<sizeof(T)>
	{
	};

	typedef void* ptr_type;
	typedef ptr_difference<ptr_type>::type ptr_diff_type;
	typedef ptr_size<ptr_type>::type ptr_size_type;
} // Kaim



#endif /* KY_BASETYPES_H */
