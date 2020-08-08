/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

#ifndef KyPdg_PriorityQueue_H
#define KyPdg_PriorityQueue_H

#include <kypdg/common/stlvector.h>
#include <kypathdata/generic/algorithm.h>
#include <kypathdata/generic/basetypes.h>

namespace Kaim
{

template <typename T, typename CMP = DefaultLess<T, T> >
class PriorityQueue
{
public:
	typedef typename StlVector<T>::const_iterator ConstIterator;
	typedef typename StlVector<T>::iterator Iterator;
public:

	PriorityQueue() {}
	explicit PriorityQueue(const CMP& cmp) : m_cmp(cmp) {}

	void Add(const T& value)
	{
		m_queue.push_back(value);
	}
	void Pop()
	{
		Iterator last = m_queue.end(); --last;
		Iterator best = GetBest();

		*best = *last;
		m_queue.erase(last);
	}

	const T& GetFirst() const { return *GetBest(); }

	bool IsEmpty() const { return m_queue.empty(); }
	KyUInt32 GetCount() const { return KyUInt32(m_queue.size()); }

	ConstIterator Begin() const { return m_queue.begin(); }
	Iterator Begin() { return m_queue.begin(); }
	ConstIterator End() const { return m_queue.end(); }
	Iterator End() { return m_queue.end(); }

	Iterator Remove(Iterator it) { return m_queue.erase(it); }

private:
	Iterator GetBest()
	{
		Iterator it = m_queue.begin();
		Iterator best = it;
		CMP& cmp = m_cmp;

		for (++it; it != m_queue.end(); ++it)
		{
			if (cmp(*it, *best))
				best = it;
		}
		return best;
	}
	ConstIterator GetBest() const { return const_cast<PriorityQueue&>(*this).GetBest(); }
private:
	StlVector<T> m_queue;
	CMP m_cmp;
};

} // namespace Kaim

#endif // KyPdg_PriorityQueue_H
