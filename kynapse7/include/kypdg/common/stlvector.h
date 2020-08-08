/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/




// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_StlVector_H
#define KyPdg_StlVector_H


#include <kypathdata/generic/memory.h>
#include <kypathdata/basesystem/allocatoradapter.h>

#if defined(KY_WINDOWS_ANY) && defined(_MSC_VER)
# pragma warning (push)
# pragma warning (disable:4548) //  expression before comma has no effect; expected expression with side-effect
#include <vector>
# pragma warning (pop)
#else
#include <vector>
#endif

#define KY_STL_USED

namespace Kaim
{


template< typename T, typename A = Kaim::allocator_adapter<T> >
class StlVector : public std::vector<T, A> 
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	StlVector() : std::vector<T, A>() {}
	StlVector(size_t count) : std::vector<T, A>(count) {}
};


}


#endif
