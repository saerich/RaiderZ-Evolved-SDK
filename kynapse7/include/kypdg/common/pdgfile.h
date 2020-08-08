/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/




// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_PdgFile_H
#define KyPdg_PdgFile_H


#include <kypathdata/generic/memory.h>
#include "kypathdata/containers/kystring.h"


namespace Kaim
{

class PdgSystem;


class PdgFile
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	enum MakeDir { DO_MAKE_DIR = 0, DO_NOT_MAKE_DIR = 1, MakeDir_FORCE32 = 0xFFFFFFFF };

	PdgFile();

	PdgFile(PdgSystem* sys, const char* fileName, const char* openMode, MakeDir makeDir = DO_MAKE_DIR);

	~PdgFile();

	void Init(PdgSystem* sys, const char* fileName, const char* openMode, MakeDir makeDir = DO_MAKE_DIR);

	enum DoErrorMessage { DO_ERROR_MESSAGE, DO_NOT_ERROR_MESSAGE };
	KyFilePtr GetFile(DoErrorMessage = DO_ERROR_MESSAGE);

	void Close();

private:
	PdgSystem* m_sys;
	KyString m_fileName;
	KyString m_openMode;
	MakeDir m_makeDir;
	KyFilePtr m_file;

private:
	PdgFile(const PdgFile&);
	PdgFile& operator=(const PdgFile&);
};


}


#endif
