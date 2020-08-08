#ifndef __VARUTILS__H
#define __VARUTILS__H

#include "utillib.h"
#include "OLE_auto.h"

void	VTToMxsString( VARTYPE vt, TSTR& str);
HRESULT value_from_variant(VARIANTARG* var, Value** pval, ITypeInfo* pTypeInfo = NULL, Value* progID = NULL);
HRESULT variant_from_value(Value* val, VARIANTARG* var, ITypeInfo* pTypeInfo = NULL);

#endif // __VARUTILS__H