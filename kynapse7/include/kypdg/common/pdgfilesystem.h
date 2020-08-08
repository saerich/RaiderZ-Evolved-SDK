/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: MAMU
#ifndef KyPathData_PdgFileSystem_H
#define KyPathData_PdgFileSystem_H


#include <kypathdata/generic/memory.h>
#include "kypathdata/containers/kystring.h"


namespace Kaim
{


/*
---------- Vocabulary (Absolute) ----------
FileName:        C:/MyDir1/MyDir2/MyFile.txt
FileDirectory:   C:/MyDir1/MyDir2
FileSimpleName:                   MyFile.txt

---------- Vocabulary (Relative) ----------
FileName:        MyDir1/MyDir2/MyFile.txt
FileDirectory:   MyDir1/MyDir2
FileSimpleName:                MyFile.txt
*/
class PdgFileSystem
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	enum FileOrDirType { NOT_FOUND, TYPE_FILE, TYPE_DIRECTORY };

public:
	void Setup(const char* absoluteRootDirectory)
	{
		m_absoluteRootDirectory = absoluteRootDirectory;
	}

	KyString GetAbsoluteFileName(const char* relativeFileName) const
	{
		if (IsAbsolute(relativeFileName))
			return relativeFileName;

		if (m_absoluteRootDirectory == "")
			return relativeFileName;
		if (    (m_absoluteRootDirectory[m_absoluteRootDirectory.Size()-1] == '/')
			 || (m_absoluteRootDirectory[m_absoluteRootDirectory.Size()-1] == '\\'))
			return m_absoluteRootDirectory + relativeFileName;
		else
			return m_absoluteRootDirectory + "/" + relativeFileName;
	}

	KyResult MakeDirectory(const char* relativeFileName) const
	{
		return MakeDirectory_Absolute(GetAbsoluteFileName(relativeFileName).CStr());
	}

	KyResult MakeDirectoryForFile(const char* relativeFileName) const
	{
		return MakeDirectoryForFile_Absolute(GetAbsoluteFileName(relativeFileName).CStr());
	}

	bool DoesFileOrDirExist(const char* relativeFileName) const
	{
		return DoesFileOrDirExist_Absolute(GetAbsoluteFileName(relativeFileName).CStr());
	}

	FileOrDirType GetFileOrDirType(const char* relativeFileOrDirName) const
	{
		return GetFileOrDirType_Absolute(GetAbsoluteFileName(relativeFileOrDirName).CStr());
	}


public: // works with relativeFileNames and absoluteFleNames
	static KyInt32 GetLastSeparatorIdx(const char* fileOrDirName);
	static KyInt32 GetLastSeparatorIdx(const KyString& fileOrDirName);
	static KyString GetContainingDirectory(const char* fileOrDirName);
	static KyString GetContainingDirectory(const KyString& fileOrDirName);
	static KyString GetFileSimpleName(const char* fileName);
	static KyString GetFileNameExtension(const char* fileName);
	static KyString GetFileNameWithoutExtension(const char* fileName);
	static KyString GetFileNameWithoutExtension(const KyString& fileName);
	static void RemoveTrailingSlashs(KyString& fileName);
	static KyString RemoveTrailingSlashs(const char* fileName);
	static bool IsWinDrive(const char* dirName);
	static bool IsAbsolute(const char* fileOrDirName);

public:
	static KyResult MakeDirectory_Absolute(const char* absoluteDirName);
	static KyResult MakeDirectory_Absolute(const KyString& absoluteDirName);
	static KyResult MakeDirectoryForFile_Absolute(const char* absoluteFileName);
	static bool DoesFileOrDirExist_Absolute(const char* absoluteFileName);
	static FileOrDirType GetFileOrDirType_Absolute(const char* absoluteFileName);
	static FileOrDirType GetFileOrDirType_Absolute(const KyString& absoluteFileName);

public:
	KyString m_absoluteRootDirectory;
};


}


#endif

