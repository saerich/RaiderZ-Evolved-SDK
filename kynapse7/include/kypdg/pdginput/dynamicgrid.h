/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_DynamicGrid_H
#define KyPdg_DynamicGrid_H


#include <kypathdata/math/box2i.h>
#include <kypathdata/containers/array.h>


namespace Kaim
{
class DynamicGridColumn;

/*
cells are indexed this way
  |     |     |     |     |
--+-----+-----+-----+-----+--
  |-2, 1|-1, 1| 0, 1| 1, 1|
  |     |     |     |     |
--+-----+-----+-----+-----+--
  |-2, 0|-1, 0| 0, 0| 1, 0|
  |     |     |     |     |
--+-----+---(0,0)---+-----+--
  |-2,-1|-1,-1| 0,-1| 1,-1|
  |     |     |     |     |
--+-----+-----+-----+---- +--
  |-2,-2|-1,-2| 0,-2| 1,-2|
  |     |     |     |     |
--+-----+-----+-----+-----+--
  |     |     |     |     |
*/

class BaseElementCreator : public VirtualMemObject
{
public:
	virtual void* NewElement(KyInt32 x, KyInt32 y) = 0;
	virtual void DeleteElement(void* element) = 0;
	virtual ~BaseElementCreator() {}
};


class BaseDynamicGrid : public VirtualMemObject
{
public:
	BaseDynamicGrid(BaseElementCreator* baseElementCreator) :
		m_baseElementCreator(baseElementCreator)
	{ Clear(); }

	virtual ~BaseDynamicGrid() {}

	void Clear();

protected:
	void* GetVoidElement(KyInt32 x, KyInt32 y);
	void* GetOrCreateVoidElement(KyInt32 x, KyInt32 y);
	DynamicGridColumn* GetColumn(KyInt32 x);
	void* AddNewElement(KyInt32 x, KyInt32 y);
	void DeleteElement(void* element) { m_baseElementCreator->DeleteElement(element); }

protected:
	BaseElementCreator* m_baseElementCreator;

	Array<void*> m_elements;
	Box2i m_box;

	// columns
	KyInt32 m_root_x;
	Array<DynamicGridColumn*> m_plus_x;  //          [root_x..]
	Array<DynamicGridColumn*> m_minus_x; // ]..root_x[

	friend class DynamicGridColumn;
};


template<class T>
class DynamicGrid : public BaseDynamicGrid
{
public:
	class ElementCreator : public BaseElementCreator
	{
	public:
		virtual void* NewElement(KyInt32 x, KyInt32 y) { return KY_NEW_CLASS(T)(x, y); }
		virtual void DeleteElement(void* element) { KY_DELETE_CLASS((T*)element); }
	};

public:
	DynamicGrid() : BaseDynamicGrid(&m_elementCreator) {}
	DynamicGrid(BaseElementCreator* baseElementCreator) : BaseDynamicGrid(baseElementCreator) {}

	virtual ~DynamicGrid() { Clear(); }

	T* GetElement(KyInt32 x, KyInt32 y) { return (T*)GetVoidElement(x, y); }

	T* GetOrCreateElement(KyInt32 x, KyInt32 y) { return (T*)GetOrCreateVoidElement(x, y); }

	T** GetElements()
	{
		if (m_elements.GetCount() == 0)
			return KY_NULL;
		return (T**)&m_elements[0];
	}

	KyUInt32 GetElementsCount() const { return KyUInt32(m_elements.GetCount()); }

private:
	ElementCreator m_elementCreator;
};


}


#endif
