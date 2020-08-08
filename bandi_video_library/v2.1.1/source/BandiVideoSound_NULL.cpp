////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// BandiVideoLibrary 2.0
/// 
/// Copyright(C) 2008-2009 BandiSoft.com All rights reserved.
///
/// Visit http://www.bandisoft.com for more information.
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <process.h>
#include "BandiVideoSound_NULL.h"

CBandiVideoSound_NULL::CBandiVideoSound_NULL() : m_ref_count(0)
{
	m_init_count = 0;
	m_start_time = 0;

	memset(&m_crit_sec, NULL, sizeof(m_crit_sec));
	m_crit_sec_inited = FALSE;	

	m_playing = FALSE;

	m_hevent = NULL;
	m_hthread = NULL;
	m_thread_id = 0;

	m_callback = NULL;

	// Performance Frequency ±¸ÇÏ±â
	LARGE_INTEGER freq = {0, 0};
	QueryPerformanceFrequency(&freq);
	m_freq_units = freq.QuadPart;
}


CBandiVideoSound_NULL::~CBandiVideoSound_NULL()
{
	Close();
}


STDMETHODIMP_(ULONG) CBandiVideoSound_NULL::AddRef()
{                             
    LONG lRef = InterlockedIncrement( &m_ref_count );
	ASSERT(lRef >= 0);

    return m_ref_count;
}


STDMETHODIMP_(ULONG) CBandiVideoSound_NULL::Release()
{
    LONG lRef = InterlockedDecrement( &m_ref_count );
    ASSERT(lRef >= 0);
   
	if(lRef == 0) 
	{
        delete this;
        return ULONG(0);
    }

	return m_ref_count;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Open sound device.
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT		CBandiVideoSound_NULL::Open(INT channels, INT samplerate, IBandiVideoSoundCB *callback)
{
	InitializeCriticalSection(&m_crit_sec);
	m_crit_sec_inited = TRUE;

	// Create an event object for periodical processing
	m_hevent = CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset event
	if(m_hevent == NULL)
		goto error;

	// Create thread for playback
	m_hthread = (HANDLE)_beginthreadex(NULL, 0, InitialThreadProc, this, CREATE_SUSPENDED, &m_thread_id);
	if(m_hthread == NULL)
		goto error;

	m_callback = callback;

	SetThreadPriority(m_hthread, THREAD_PRIORITY_TIME_CRITICAL);
	ResumeThread(m_hthread);

	return S_OK;

error:
	Close();

	return E_FAIL;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Close sound device.
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
void		CBandiVideoSound_NULL::Close()
{
    HANDLE hThread = (HANDLE)InterlockedExchangePointer(&m_hthread, 0);
    if(hThread) 
	{
		if(m_hevent) SetEvent(m_hevent);

		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
	}
	
	if(m_hevent)
	{
		CloseHandle(m_hevent);
		m_hevent = NULL;
	}

	if(m_crit_sec_inited)
	{
		DeleteCriticalSection(&m_crit_sec);
		m_crit_sec_inited = FALSE;
	}

	m_init_count = 0;
	m_start_time = 0;
}


static LONGLONG GetPerformanceCounter()
{
	LARGE_INTEGER count;
	::QueryPerformanceCounter(&count);
	return count.QuadPart;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Start 
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT		CBandiVideoSound_NULL::Start()
{
	EnterCriticalSection(&m_crit_sec);
	{
		m_init_count = GetPerformanceCounter();

		m_start_time = 0;
		m_playing = TRUE;
	}
	LeaveCriticalSection(&m_crit_sec);

	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Stop
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT		CBandiVideoSound_NULL::Stop()
{
	EnterCriticalSection(&m_crit_sec);
	{
		m_start_time = 0;
		m_playing = FALSE;
	}
	LeaveCriticalSection(&m_crit_sec);

	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Stop
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT		CBandiVideoSound_NULL::Pause(BOOL pause)
{
	EnterCriticalSection(&m_crit_sec);
	{
		if(pause)
		{
			m_start_time = CalcTime();
			m_playing = FALSE;
		}
		else
		{
			m_init_count = GetPerformanceCounter();
			m_playing = TRUE;
		}
	}
	LeaveCriticalSection(&m_crit_sec);

	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Get play time (100nsec units)
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT		CBandiVideoSound_NULL::GetTime(INT64 &time)
{
	EnterCriticalSection(&m_crit_sec);
	{
		time = CalcTime();
	}
	LeaveCriticalSection(&m_crit_sec);

	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Set sound volume
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT		CBandiVideoSound_NULL::SetVolume(INT volume)
{
	return E_NOTIMPL;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// ThreadProc
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
UINT	CBandiVideoSound_NULL::ThreadProc()
{
	BOOL bWorking = FALSE;
	while(m_hthread)
	{
		EnterCriticalSection(&m_crit_sec);
		{
			if(m_playing)
				PlayNULL();
		}
		LeaveCriticalSection(&m_crit_sec);
		
		WaitForSingleObject(m_hevent, 10);
	}
	
	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Play NullSound
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT		CBandiVideoSound_NULL::PlayNULL()
{
	if(m_callback)
	{
		INT16 buf[1024*2];
		m_callback->GetPcmData(buf, 1024);
	}

	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Calculate current time
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
LONGLONG	CBandiVideoSound_NULL::CalcTime()
{
	LONGLONG ret;
	LONGLONG delta_time = 0;

	EnterCriticalSection(&m_crit_sec);
	{
		if(m_playing && m_freq_units)
			delta_time = LONGLONG(double(GetPerformanceCounter() - m_init_count)/(double)m_freq_units*10000000.0);

		ret = m_start_time + delta_time;
	}
	LeaveCriticalSection(&m_crit_sec);

	return ret;
}
