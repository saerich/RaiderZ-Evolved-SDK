//**********************************************************************
//
// Copyright (c) 2002
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#include <vector>

// disable warning 'symbol truncated in debugging information'
#pragma warning( disable : 4786 )

// disable warning 'return type for 'nList_Internal::const_iterator<class cShape *>::operator ->' is 'class cShape **const  ' (ie; not a UDT or reference to a UDT.  Will produce errors if applied using infix notation)'
#pragma warning( disable : 4284 )

typedef long tSigned32;
typedef unsigned long tUnsigned32;
typedef __int64 tSigned64;
typedef unsigned __int64 tUnsigned64;

template <class T> tSigned32
SizeL(const T& container)
{
    return static_cast<tSigned32>(container.size());
}

template <class T> T*
VectorBuffer(std::vector<T>& v)
{
    if(v.empty())
    {
        return 0;
    }
    return &v.front();
}
template <class T> const T*
VectorBuffer(const std::vector<T>& v)
{
    if(v.empty())
    {
        return 0;
    }
    return &v.front();
}
