// TestMarshalSpeed.h : Declaration of the CTestMarshalSpeed

#ifndef __TESTMARSHALSPEED_H_
#define __TESTMARSHALSPEED_H_

#include "resource.h"       // main symbols
#define BIG_HUNK 524288

/////////////////////////////////////////////////////////////////////////////
// CTestMarshalSpeed
class ATL_NO_VTABLE CTestMarshalSpeed : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CTestMarshalSpeed, &CLSID_TestMarshalSpeed>,
	public ITestMarshalSpeed
{
public:
	CTestMarshalSpeed()
	{
		mpData = new unsigned char[BIG_HUNK];
	}
	virtual ~CTestMarshalSpeed() {;}

DECLARE_REGISTRY_RESOURCEID(IDR_TESTMARSHALSPEED)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CTestMarshalSpeed)
	COM_INTERFACE_ENTRY(ITestMarshalSpeed)
END_COM_MAP()

// ITestMarshalSpeed
public:
	STDMETHOD(getData)(/*[out]*/ long *pcMax, /*[out, size_is(,*pcMax)]*/ unsigned char **pprgs);
private:
	unsigned char * mpData;
};

#endif //__TESTMARSHALSPEED_H_
