// TestMarshalSpeed.cpp : Implementation of CTestMarshalSpeed
#include "stdafx.h"
#include "Comsrv.h"
#include "TestMarshalSpeed.h"

/////////////////////////////////////////////////////////////////////////////
// CTestMarshalSpeed


STDMETHODIMP CTestMarshalSpeed::getData(long *pcMax, unsigned char **pprgs)
{
	*pcMax = 524288;

	*pprgs = (unsigned char *) CoTaskMemAlloc(*pcMax);
	_ASSERTE(*pprgs);

	memcpy(*pprgs, mpData, *pcMax);

	return S_OK;
}
