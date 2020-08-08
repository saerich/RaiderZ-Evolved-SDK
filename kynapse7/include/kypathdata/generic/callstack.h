/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KYCALLSTACK_H
#define KYCALLSTACK_H

#include "kypathdata/containers/staticstack.h"

#include <new>
#include <stddef.h>

namespace Kaim
{
	/* Stores information about a function call or an instruction.
		This class us used by memory allocators. Do not allocate with new.
	*/
	struct Call
	{
		const char* name;
		const char* file;
		KyUInt32 line;

		/*! Constructor.
		*/
		Call(const char* name_, const char* file_, KyUInt32 line_) 
			: name(name_), file(file_), line(line_)
		{
		}

		/*! Default constructor.
		*/
		Call() 
			: name(0), file(0), line(0)
		{
		}

		/*! Test whether two calls are equal. 
		Two calls are considered equal if they have the same name, on the same line in the same file.
		Hence, multiple calls with the same name on the same line can lead to unexpected results. 	
		*/
		bool operator==(const Call& other)
		{
			return (name == other.name && file == other.file && line == other.line);
		}

		void* operator new(size_t , void* placement_new) 
		{ 
			return placement_new; 
		}

		void* operator new[](size_t , void* placement_new) 
		{  
			return placement_new; 
		}
	};	

	/* Stack of function calls.
	*/
	struct CallStack : StaticStack<Call, 32>
	{
		typedef StaticStack<Call, 32> base_type;
	};

	/* Automatic stacked call.
	Pushes a call on the stack, then automatically pops the call when getting out of scope.
	*/
	struct ScopedCall
	{
		typedef CallStack callstack_type;
		typedef callstack_type::ValueType call_type;

		ScopedCall(callstack_type* callStack_, call_type call_)
		{
			if (callStack_ && !(callStack_->IsFull()))
			{
				callStack = callStack_;
				callStack->PushBack(call_);
			}
			else
			{
				callStack = 0;
			}
		}

		~ScopedCall()
		{
			if (callStack)
			{
				callStack->PopBack();
			}
		}
	
	protected:
		callstack_type* callStack;
	};
}

#endif // KYCALLSTACK_H
