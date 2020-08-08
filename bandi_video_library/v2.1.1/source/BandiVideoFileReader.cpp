////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// BandiVideoLibrary 2.0
/// 
/// Copyright(C) 2008-2009 BandiSoft.com All rights reserved.
///
/// Visit http://www.bandisoft.com for more information.
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "BandiVideoFileReader.h"


CBandiVideoFileReader::CBandiVideoFileReader() : m_ref_count(0), m_file_handle(NULL)
{
	
}


CBandiVideoFileReader::~CBandiVideoFileReader()
{
	Close();
}


STDMETHODIMP_(ULONG) CBandiVideoFileReader::AddRef()
{                             
    LONG lRef = InterlockedIncrement(&m_ref_count);
	ASSERT(lRef >= 0);

    return m_ref_count;
}


STDMETHODIMP_(ULONG) CBandiVideoFileReader::Release()
{
    LONG lRef = InterlockedDecrement(&m_ref_count);
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
/// Open a bandivideo file.
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CBandiVideoFileReader::Open(LPCSTR path_name)
{
	Close();

	HANDLE hFile = CreateFileA(path_name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if(hFile == INVALID_HANDLE_VALUE) 
		return E_FAIL;

	m_file_handle = hFile;

	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Close a bandivideo file.
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
void	CBandiVideoFileReader::Close()
{
	if(m_file_handle) 
	{
		::CloseHandle(m_file_handle);
		m_file_handle = NULL;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Read data from a file.
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
INT		CBandiVideoFileReader::Read(BYTE* buf, INT bytes_to_read)
{
	DWORD dwBytesRead = 0;
	if (!::ReadFile(m_file_handle, buf, bytes_to_read, &dwBytesRead, NULL))
        return 0;

	return dwBytesRead;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Move the file pointer of the specified file.
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
INT64	CBandiVideoFileReader::SetPosition(INT64 pos)
{
	LARGE_INTEGER li;
	li.QuadPart = pos;
	
	li.LowPart = ::SetFilePointer(m_file_handle , li.LowPart , &li.HighPart , FILE_BEGIN);
	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
	{
		li.QuadPart = -1;
	}

	return li.QuadPart;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Return the file pointer of the specified file.
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
INT64	CBandiVideoFileReader::GetPosition()
{
	LARGE_INTEGER li;
	li.QuadPart = 0;
	
	li.LowPart = ::SetFilePointer(m_file_handle , li.LowPart , &li.HighPart , FILE_CURRENT);

	return li.QuadPart;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Return the size of the specified file. 
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
INT64	CBandiVideoFileReader::GetFileSize()
{
	LARGE_INTEGER size = {0, 0};
	size.LowPart = ::GetFileSize(m_file_handle , (DWORD*)&size.HighPart);

	return size.QuadPart;
}

