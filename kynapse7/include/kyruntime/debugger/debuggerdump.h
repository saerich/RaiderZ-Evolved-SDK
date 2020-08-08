/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: ? - secondary contact: NOBODY
#ifndef KYDEBUGGERDUMP_H
#define KYDEBUGGERDUMP_H


#include <kyruntime/debugger/debuggertypes.h>
#include <kypathdata/containers/kystring.h>
#include <kyruntime/debugger/debuggernetarray.h>
			
namespace Kaim
{
	namespace Debugger
	{
		struct Dump
		{
			/*
			*	\post IsOpen() == false
			*/
			Dump();

			/*
			*	\post IsOpen() == false
			*/
			~Dump();

			/*
			*	\post IsOpen() == true
			*	\return Result::Unknown if can not open the file
			*/

			KyResult Open(const KyString& dumpFileName);

			/*
			*	\post IsOpen() == false
			*/
			KyResult Close();

			bool IsOpen() const
			{
				return m_dumpFile != 0;
			}

			/*
			*	\pre IsOpen() == true
			*/
			KyResult operator<<(const Net::NetArray& buffer); 

			KyResult Loop();

		protected:
			/*
			*	\pre IsOpen() == true
			*/
			KyResult SeekAtHeader();

			/*
			*	\pre IsOpen() == true
			*/
			KyResult SeekAfterHeader();

			/*
			*	\pre IsOpen() == true
			*/
			KyResult WriteHeader();


		protected:
			static const char* file_id;
			static const KyFixed version = 1;
			static const KyInt32 header_size = 4*sizeof(KyFixed) + 25 /*KynapseRemoteDebuggerDump*/;

		protected:
			KyFilePtr m_dumpFile;
			KyFixed m_dumpLength;
		};
	}
}	

#endif // KYDEBUGGERDUMP_H
