////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Youtube uploader
/// 
/// Copyright(C) 2008 BandiSoft.com All rights reserved.
///
/// Visit http://www.bandisoft.com for more information.
///
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#define UP_MAX_ID		128
#define UP_MAX_PWD		128
#define UP_MAX_TITLE	256
#define UP_MAX_DESC		512
#define UP_MAX_KEYWORD	128

// 업로드 정보 구조체
struct SUploadInfo
{
	CHAR szID[UP_MAX_ID];				// Youtube Username
	CHAR szPWD[UP_MAX_PWD];				// Youtube Password

	WCHAR szFilePath[MAX_PATH];			// 업로드 파일 경로
	WCHAR szTitle[UP_MAX_TITLE];		// 제목
	WCHAR szDesc[UP_MAX_DESC];			// 설명
	WCHAR szKeyword[UP_MAX_KEYWORD];	// 키워드
};

#define STATUS_BUFFER_COUNT 2048

class CYoutubeUploader
{
public:
	CYoutubeUploader();
	~CYoutubeUploader();

	BOOL Upload(SUploadInfo *pInfo);						// 업로드
	void Cancel();											// 업로드 취소

	BOOL IsUploading();										// 업로드중인가?
	BOOL GetStatusString(LPTSTR lpStatus, int nBufferCnt);	// 업로드 상태 문자열 구하기

protected:
	void UploadProc();
	static void _ThreadProc(void *param)
	{
		((CYoutubeUploader*)param)->UploadProc();
	};

	HANDLE m_hThread;

	SUploadInfo			m_info;		// 업로드 정보
	PROCESS_INFORMATION m_pi;		// 업로드 프로세스 정보

	CHAR				m_sStatus[STATUS_BUFFER_COUNT+1];	// 업로드 상태 정보 문자열
	CRITICAL_SECTION	m_CritSec;
};
