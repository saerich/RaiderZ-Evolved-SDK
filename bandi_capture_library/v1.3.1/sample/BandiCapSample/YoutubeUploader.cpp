////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Youtube uploader
/// 
/// Copyright(C) 2008 BandiSoft.com All rights reserved.
///
/// Visit http://www.bandisoft.com for more information.
///
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "YoutubeUploader.h"


#define CHECK_RESULT(x) if(x == FALSE){ ASSERT(0); goto end; }

BOOL WINAPI IsWinNT()
{
  DWORD WindowsVersion = GetVersion();
  DWORD WindowsMajorVersion = (DWORD)(LOBYTE(LOWORD(WindowsVersion)));
  DWORD WindowsMinorVersion = (DWORD)(HIBYTE(LOWORD(WindowsVersion)));

  if(WindowsVersion >= 0x80000000) return FALSE;
  
  return TRUE;
}


CYoutubeUploader::CYoutubeUploader()
{
	m_hThread = NULL;

	memset(&m_info, NULL, sizeof(m_info));
	memset(&m_pi, NULL, sizeof(m_pi));

	m_sStatus[0] = '\0';

	InitializeCriticalSection(&m_CritSec);
}


CYoutubeUploader::~CYoutubeUploader()
{
	DeleteCriticalSection(&m_CritSec);
}


// 업로드 하기
BOOL CYoutubeUploader::Upload(SUploadInfo *pInfo)
{
	if(IsUploading() == TRUE)
		return FALSE;
	
	memcpy(&m_info, pInfo, sizeof(SUploadInfo));
	m_hThread = (HANDLE)_beginthread(_ThreadProc, 0, this);

	return !!m_hThread;
}


// 업로드 취소
void CYoutubeUploader::Cancel()
{
	if(m_hThread)
	{
		TerminateProcess(m_pi.hProcess, 0);
		m_pi.hProcess = NULL;
		
		WaitForSingleObject(m_hThread, INFINITE);

		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	memset(&m_pi, NULL, sizeof(m_pi));
}


// 업로드 중인가?
BOOL CYoutubeUploader::IsUploading()
{
	return !!m_hThread;
}


// 업로드 상태 문자열 구하기
BOOL CYoutubeUploader::GetStatusString(LPTSTR lpStatus, int nBufferCnt)
{
	if(m_hThread == NULL)
		return FALSE;

	EnterCriticalSection(&m_CritSec);
	{
		TCHAR szTemp[STATUS_BUFFER_COUNT+1];

		USES_CONVERSION;
		_tcsncpy(szTemp, A2T(m_sStatus), STATUS_BUFFER_COUNT);
		
		int i, j;
		int len = _tcslen(szTemp);
		lpStatus[0] = '\0';

		for(i = 0, j = 0 ; i < len && j-1 < nBufferCnt ; i++, j++)
		{
			if(szTemp[i] == '\r' || szTemp[i] == '\n')
			{
				j = -1;
				continue;
			}
			
			lpStatus[j] = szTemp[i];
			lpStatus[j+1] = '\0';
		}
	}
	LeaveCriticalSection(&m_CritSec);

	return _tcslen(lpStatus);
}


// 업로드 스레드
void CYoutubeUploader::UploadProc()
{
	static TCHAR szAppFile[] = _T("uploader.dat");
	TCHAR szMappedName[MAX_PATH];
	HANDLE hMapFile = NULL;
	LPVOID pBuf = NULL;

	HANDLE hStdOut = NULL;
	HANDLE hStdOutRead = NULL;
	HANDLE hStdOutReadTmp = NULL;

	HANDLE hProcess = ::GetCurrentProcess();

	_sntprintf(szMappedName, MAX_PATH, _T("%d"), GetTickCount());
	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, 
		PAGE_READWRITE, 0, sizeof(SUploadInfo), szMappedName);
	
	CHECK_RESULT(hMapFile);

	pBuf = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SUploadInfo));           
	CHECK_RESULT(pBuf);

	memcpy(pBuf, &m_info, sizeof(SUploadInfo));

	SECURITY_ATTRIBUTES sa;
	::ZeroMemory(&sa, sizeof(SECURITY_ATTRIBUTES));
	sa.nLength= sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	CHECK_RESULT(CreatePipe(&hStdOutReadTmp, &hStdOut, &sa, 0));
	
	CHECK_RESULT(DuplicateHandle(hProcess, hStdOutReadTmp,
		hProcess, &hStdOutRead, 0, FALSE, DUPLICATE_SAME_ACCESS));

	if(hStdOutReadTmp)	CloseHandle(hStdOutReadTmp); hStdOutReadTmp = NULL;


	STARTUPINFO si;
	memset(&si, NULL, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
 	si.hStdOutput = hStdOut;

	TCHAR szPath[MAX_PATH*2];
	GetModuleFileName(NULL, szPath, MAX_PATH*2);
	*(_tcsrchr(szPath, '\\') + 1) = '\0';
	_tcscat(szPath, szAppFile);
	_tcscat(szPath, _T(" "));
	_tcscat(szPath, szMappedName);

	LPVOID lpSD = NULL;
	LPSECURITY_ATTRIBUTES lpSA = NULL;
	if(IsWinNT())
	{
		lpSD = ::GlobalAlloc(GPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
		CHECK_RESULT(::InitializeSecurityDescriptor(lpSD, SECURITY_DESCRIPTOR_REVISION));
		CHECK_RESULT(::SetSecurityDescriptorDacl(lpSD, -1, 0, 0));

		lpSA = (LPSECURITY_ATTRIBUTES)::GlobalAlloc(GPTR, sizeof(SECURITY_ATTRIBUTES));
		lpSA->nLength = sizeof(SECURITY_ATTRIBUTES);
		lpSA->lpSecurityDescriptor = lpSD;
		lpSA->bInheritHandle = TRUE;
	}

	CHECK_RESULT(CreateProcess(NULL, szPath, lpSA, NULL, TRUE, 
		CREATE_NEW_CONSOLE | NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &m_pi));

	if(lpSA != NULL) ::GlobalFree(lpSA);
	if(lpSD != NULL) ::GlobalFree(lpSD);

	::CloseHandle(m_pi.hThread);

	DWORD nBytesRead;
	CHAR  szBuffer[STATUS_BUFFER_COUNT+1];
	do
	{
		DWORD dwAvail = 0;
		if(PeekNamedPipe(hStdOutRead, NULL, 0, NULL, &dwAvail, NULL))
		{
			if(dwAvail > 0)
			{
				if(!::ReadFile(hStdOutRead, szBuffer, min(STATUS_BUFFER_COUNT, dwAvail),
					&nBytesRead, NULL) || !nBytesRead)
				{
					if(::GetLastError() == ERROR_BROKEN_PIPE)
						break;
					else
						ASSERT(FALSE);
				}
				if(nBytesRead)
				{
					szBuffer[nBytesRead] = '\0';

					EnterCriticalSection(&m_CritSec);
					{
						strcpy(m_sStatus, szBuffer);
					}
					LeaveCriticalSection(&m_CritSec);
				}
			}
		}

		if(m_pi.hProcess == NULL)
			break;

	}while(WaitForSingleObject(m_pi.hProcess, 1) == WAIT_TIMEOUT);

end:
	if(m_pi.hProcess) CloseHandle(m_pi.hProcess);
	
	if(pBuf) UnmapViewOfFile(pBuf);
	if(hMapFile) CloseHandle(hMapFile);

	memset(&m_pi, NULL, sizeof(m_pi));
	memset(&m_info, NULL, sizeof(m_info));

	if(hStdOut)			CloseHandle(hStdOut);
	if(hStdOutRead)		CloseHandle(hStdOutRead);
	if(hStdOutReadTmp)	CloseHandle(hStdOutReadTmp);

	CloseHandle(m_hThread);
	m_hThread = NULL;
	
	EnterCriticalSection(&m_CritSec);
	{
		m_sStatus[0] = '\0';
	}
	LeaveCriticalSection(&m_CritSec);

	_endthreadex(0);
}
