#include <strbasic.h>
//Some macors for facilitating use of the XML log
#define USES_LOG(a)				long logres = a; CComQIPtr<IXMLLog, &IID_IXMLLog>pLog(mpLog) ;
#define USES_LOG_SECTION(a,b)	USES_LOG(b); pLog->StartSection(a, logres);	
#define LOG_SECTION(a,b)		logres = b; pLog->StartSection(a, b);
#define DONE_LOG_SECTION		pLog->EndSection(logres);
#define LOG_ENTRY(a)			pLog->LogEvent(a, logres);
#define LOG_ENTRY_(a, b)			pLog->LogEvent(a, b);
#define LOG_ERROR(a)			LOG_ENTRY_(a,E_FAIL);
#define LOG_STATE(a, b)			(mpLog->LogEventState((a),(b),logres));
#define LOG_SECTSTATE(a, b)		(mpLog->LogSectionState((a),(b),logres));

inline void LogEntry(IXMLLog *pLog, long lvl, const mwchar_t *format, ...) {
	OLECHAR buf[512];
	va_list args;
	va_start(args,format);
	_vsnwprintf(buf,512,format,args);
	va_end(args);
	pLog->LogEvent(buf, lvl);
}

inline void MergeLog(IXMLLog *pLog)
{
	IXMLLog *pSubLog = NULL;
	IErrorInfo *pEI = NULL;
	::GetErrorInfo(0, &pEI);
	if(pEI)
	{
		HRESULT hr = pEI->QueryInterface(IID_IXMLLog, (void **) &pSubLog);
		if(hr == S_OK)
		{
			assert(pSubLog);
			pLog->MergeLog(pSubLog);
		}
	}
}


#define LOG_NONE	S_OK
#define LOG_INFO	MAKE_HRESULT(0, FACILITY_ITF, 1)
#define LOG_PROC	MAKE_HRESULT(0, FACILITY_ITF, 2)
#define LOG_FUNC	MAKE_HRESULT(0, FACILITY_ITF, 3)
#define LOG_ITER	MAKE_HRESULT(0, FACILITY_ITF, 4)
