/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

/*!	\file
	\brief Contains macros that set up input, output and string manipulations depending on the
	target platform.
	\ingroup kypathdata_generic */

#ifndef KYCSTDIO_H
#define KYCSTDIO_H

#include <stdio.h>
#include <string.h>
#include <kypathdata/generic/basetypes.h>


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

#define KY_STRICMP _stricmp
#define KY_STRNICMP _strnicmp
#define KY_SNPRINTF _snprintf

#if defined(_MSC_VER) && _MSC_VER >= 1400
# define KY_STRCPY_S(a, s, b) strcpy_s((a), (s), (b))
# define KY_SPRINTF sprintf_s
# define KY_PRINTF printf_s
#else
# define KY_STRCPY_S(a, s, b) strcpy((a), (b))
# define KY_SPRINTF sprintf
# define KY_PRINTF printf
#endif

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

#define KY_STRICMP _stricmp
#define KY_STRNICMP _strnicmp
#define KY_SNPRINTF _snprintf

#if defined(_MSC_VER) && _MSC_VER >= 1400
# define KY_STRCPY_S(dest, destSize, src) strcpy_s((dest), (destSize), (src))
# define KY_SPRINTF sprintf_s
# define KY_PRINTF printf_s
#else
# define KY_STRCPY_S(dest, destSize, src) strcpy((dest), (src))
# define KY_SPRINTF sprintf
# define KY_PRINTF printf
#endif

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

#define KY_STRICMP stricmp
#define KY_STRNICMP _strnicmp
#define KY_SNPRINTF _snprintf
#define KY_SPRINTF sprintf
#define KY_PRINTF printf
#define KY_STRCPY_S(dest, destSize, src) strcpy((dest), (src))

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

#define KY_STRICMP strcasecmp
#define KY_STRNICMP strncasecmp
#define KY_SNPRINTF snprintf
#define KY_SPRINTF sprintf
#define KY_PRINTF printf
#define KY_STRCPY_S(dest, destSize, src) strcpy((dest), (src))

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

#define KY_STRICMP stricmp
#define KY_STRNICMP _strnicmp
#define KY_SNPRINTF 
#define KY_SPRINTF sprintf
#define KY_PRINTF printf
#define KY_STRCPY_S(dest, destSize, src) strcpy((dest), (src))

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

#define KY_STRICMP stricmp
#define KY_STRNICMP strnicmp
#define KY_SNPRINTF 
#define KY_SPRINTF sprintf
#define KY_PRINTF printf
#define KY_STRCPY_S(dest, destSize, src) strcpy((dest), (src))

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

#define KY_STRICMP strcasecmp
#define KY_STRNICMP strncasecmp
#define KY_SNPRINTF snprintf
#define KY_SPRINTF sprintf
#define KY_PRINTF printf
#define KY_STRCPY_S(dest, destSize, src) strcpy((dest), (src))

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

#define KY_STRICMP strcasecmp
#define KY_STRNICMP strncasecmp
#define KY_SNPRINTF snprintf
#define KY_SPRINTF sprintf
#define KY_PRINTF printf
#define KY_STRCPY_S(dest, destSize, src) strcpy((dest), (src))

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

#define KY_STRICMP strcasecmp
#define KY_STRNICMP strncasecmp
#define KY_SNPRINTF snprintf
#define KY_SPRINTF sprintf
#define KY_PRINTF printf
#define KY_STRCPY_S(dest, destSize, src) strcpy((dest), (src))

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

#define KY_STRICMP strcasecmp
#define KY_STRNICMP strncasecmp
#define KY_SNPRINTF snprintf
#define KY_SPRINTF sprintf
#define KY_PRINTF printf
#define KY_STRCPY_S(dest, destSize, src) strcpy((dest), (src))

#endif /* KY_PS3 */

namespace Kaim
{
	KyUInt32 KY_FOPEN(void** file, const char* filename, const char *mode);
}

#endif // KYCSTDIO_H
