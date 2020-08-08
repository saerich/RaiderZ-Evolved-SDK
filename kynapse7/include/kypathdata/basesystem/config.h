/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*! \file
	\ingroup kypathdata_basesystem */



#ifndef KYNO_CONFIG_H
#define KYNO_CONFIG_H

#include <kypathdata/generic/memory.h>
#include <kypathdata/basesystem/io.h>
#include <kypathdata/generic/cstdio.h>

namespace Kaim
{

/*! Sets a KyFloat32 variable to a specified value, if the value is greater than or equal to 0. If the value is less than 0,
	an error message is logged.
	\param varname			A variable or configuration parameter name to be printed in error messages.
	\param var				The variable whose value is to be set. 
	\param value			The value to set for \c var. */
#define	KY_AFFECT_POSITIVE_FLOAT(varname, var, value) \
	do { \
	if (0 <= (value)) { \
	var = value; \
	} else { \
	KY_MESSAGE(KY_LOG_WARNING, ("Invalid value (%.2f) for '%s' parameter, we keep value (%.2f). Should be positive.", (value), (varname), (var))); \
	} \
	} while (Kaim::IsFalse(true))

/*! Sets a KyFloat32 variable to a specified value, if the value is greater than 0. If the value is equal to or less than 0,
	an error message is logged.
	\param varname			A variable or configuration parameter name to be printed in error messages.
	\param var				The variable whose value is to be set. 
	\param value			The value to set for \c var. */
#define	KY_AFFECT_STRICT_POSITIVE_FLOAT(varname, var, value) \
	do { \
	if (0 < (value)) { \
	var = value; \
	} else { \
	KY_MESSAGE(KY_LOG_WARNING, ("Invalid value (%.2f) for '%s' parameter, we keep value (%.2f). Should be strictly positive.", (value), (varname), (var))); \
	} \
	} while (Kaim::IsFalse(true))

/*! Sets a KyFloat32 variable to a specified value, if the value is within a specified range (inclusive of the start and end of the range). 
	If the value is outside of the range, an error message is logged.
	\param varname			A variable or configuration parameter name to be printed in error messages.
	\param minRange			The lowest acceptable value for the variable.
	\param maxRange			The largest acceptable value for the variable.
	\param var				The variable whose value is to be set. 
	\param value			The value to set for \c var. */
#define	KY_AFFECT_FLOAT_IN_RANGE(varname, minRange, maxRange, var, value) \
	do { \
	if (minRange <= (value) && (value) <= maxRange) { \
	var = value; \
	} else { \
	KY_MESSAGE(KY_LOG_WARNING, ("Invalid value (%.2f) for '%s' parameter, we keep value (%.2f). Should be in range[%.2f; %.2f].", (value), (varname), (var), (minRange), (maxRange))); \
	} \
	} while (Kaim::IsFalse(true))

/*! Sets a KyInt32 variable to a specified value, if the value is greater than or equal to 0. If the value is less than 0,
	an error message is logged.
	\param varname			A variable or configuration parameter name to be printed in error messages.
	\param var				The variable whose value is to be set. 
	\param value			The value to set for \c var. */
#define	KY_AFFECT_POSITIVE_INT(varname, var, value) \
	do { \
	if (0 <= (value)) { \
	var = value; \
	} else { \
	KY_MESSAGE(KY_LOG_WARNING, ("Invalid value (%d) for '%s' parameter, we keep value (%d). Should be positive.", (value), (varname), var)); \
	} \
	} while (Kaim::IsFalse(true))

/*! Sets a KyInt32 variable to a specified value, if the value is greater than 0. If the value is equal to or less than 0,
	an error message is logged.
	\param varname			A variable or configuration parameter name to be printed in error messages.
	\param var				The variable whose value is to be set. 
	\param value			The value to set for \c var. */
#define	KY_AFFECT_STRICT_POSITIVE_INT(varname, var, value) \
	do { \
	if (0 < (value)) { \
	var = value; \
	} else { \
	KY_MESSAGE(Kaim::KY_LOG_WARNING, ("Invalid value (%d) for '%s' parameter, we keep value (%d). Should be strictly positive.", (value), (varname), var)); \
	} \
	} while (Kaim::IsFalse(true))


/*!	Returns a copy of the specified string.
	\ingroup kypathdata_basesystem */
inline char* Strdup(const char*s)
{
	if (s == KY_NULL)
		return KY_NULL;

	KyUInt32 size = static_cast<KyUInt32>(strlen(s) + 1);
	char* res = KY_MALLOC(char, size);

	KY_STRCPY_S(res, size, s);
	return res;
}

} // namespace Kaim

#endif //KYNO_CONFIG_H
