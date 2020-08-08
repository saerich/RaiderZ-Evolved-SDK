/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_StlMap_H
#define KyPdg_StlMap_H


#include <kypathdata/generic/memory.h>
#include <kypathdata/basesystem/allocatoradapter.h>
#include <map>

#define KY_STL_USED

namespace Kaim
{


template< typename Key, typename Type, typename Traits = std::less<Key>, typename A = Kaim::allocator_adapter< std::pair <const Key, Type> > >
class StlMap : public std::map<Key, Type, Traits, A> 
{
	KY_DEFINE_NEW_DELETE_OPERATORS
};


template< typename Key, typename Type, typename Traits = std::less<Key>, typename A = Kaim::allocator_adapter< std::pair <const Key, Type> > >
class StlMultiMap : public std::multimap<Key, Type, Traits, A> 
{
	KY_DEFINE_NEW_DELETE_OPERATORS
};


}


#endif
