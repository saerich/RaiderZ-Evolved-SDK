/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPathData_Filehandler_H
#define KyPathData_Filehandler_H


#include <kypathdata/generic/types.h>
#include <kypathdata/generic/memory.h>


namespace Kaim
{


class FileHandler
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	FileHandler() : m_file(KY_NULL) {}

	KyFilePtr Open(const char* fileName, const char* openMode);

	~FileHandler() { Close(); }

	void Close();

private:
	KyFilePtr m_file;

private:
	FileHandler(const FileHandler&);
	FileHandler& operator=(const FileHandler&);
};


}


#endif
