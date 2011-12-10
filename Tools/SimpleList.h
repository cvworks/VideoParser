/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "Serialization.h"

namespace vpl {

/*!
	Very simple list

	It is initialized as:

	lastIt.ptr = new Cell(NULL, NULL);
	firstIt = lastIt;
	cellCount = 0;
*/
template <typename T> class SimpleList
{
	struct Cell
	{
		T data;
		Cell* prev;
		Cell* next;

		Cell(Cell* p, Cell* n) 
		{ 
			prev = p; 
			next = n; 
		}

		Cell(Cell* p, Cell* n, const T& d) : data(d) 
		{ 
			prev = p; 
			next = n; 
		}

		Cell (const Cell& rhs) : data(rhs.data)
		{
			prev = rhs.prev;
			next = rhs.next;
		}

		Cell& operator=(const Cell& rhs)
		{
			data = rhs.data;
			prev = rhs.prev;
			next = rhs.next;
		}
	};

public:
	class const_iterator
	{
		friend class SimpleList;

	protected:
		Cell* ptr;

	public:

		const T& operator->() const
		{
			return ptr->data;
		}

		operator const T*() const 
		{ 
			return &ptr->data; 
		}

		const_iterator& operator++() 
		{ 
			ptr = ptr->next; 
			return *this; 
		}

		const_iterator& operator--() 
		{ 
			ptr = ptr->prev; 
			return *this; 
		}
	};

	class iterator : public const_iterator
	{
	public:
		T& operator->()
		{
			return ptr->data;
		}

		const T& operator->() const
		{
			return ptr->data;
		}

		operator T*()
		{ 
			return &ptr->data; 
		}

		operator const T*() const 
		{ 
			return &ptr->data; 
		}

		iterator& operator++() 
		{ 
			ptr = ptr->next; 
			return *this; 
		}

		iterator& operator--() 
		{ 
			ptr = ptr->prev; 
			return *this; 
		}
	};

	class CellPtr
	{
		friend class SimpleList;

		Cell* ptr;

	public:
		CellPtr(Cell* p = NULL) : ptr(p) { }
		CellPtr(const CellPtr& cp) : ptr(cp.ptr) { }

		CellPtr& operator=(const CellPtr& rhs) 
		{ 
			ptr = rhs.ptr; 
			return *this;
		}

		CellPtr& operator=(Cell* p) 
		{ 
			ptr = p; 
			return *this;
		}

		bool operator==(const CellPtr& rhs) const 
		{ 
			return (ptr == rhs.ptr);
		}
	
		bool operator!=(const CellPtr& rhs) const 
		{ 
			return (ptr != rhs.ptr);
		}

		bool operator<(const CellPtr& rhs) const 
		{
			return (ptr < rhs.ptr);
		}

		DECLARE_BASIC_MEMBER_SERIALIZATION
	};

protected:
	iterator firstIt;
	iterator lastIt;
	unsigned cellCount;

protected:
	friend class graph;

	/*! 
		Returs the successor of cp, or null if the list is empty 
		or if cp is the last item.
	*/
	CellPtr Successor(CellPtr cp) const
	{
		return (cp.ptr->next == lastIt.ptr || cp.ptr == lastIt.ptr) 
			? NULL : cp.ptr->next;
	}

	/*! 
		Returs the predecessor of cp, or null if the list is empty 
		or if cp is the first item.
	*/
	CellPtr Predecessor(CellPtr cp) const
	{
		return (cp.ptr == firstIt.ptr) ? NULL : cp.ptr->prev;
	}

	CellPtr GetBeginPtr() const
	{
		return (empty()) ? NULL : firstIt.ptr;
	}

	CellPtr GetEndPtr() const
	{
		return (empty()) ? NULL : lastIt.ptr->prev;
	}

	const T& GetData(CellPtr cp) const
	{
		ASSERT(cp.ptr && cp.ptr != lastIt.ptr);

		return cp.ptr->data;
	}

	T& GetData(CellPtr cp)
	{
		ASSERT(cp.ptr && cp.ptr != lastIt.ptr);

		return cp.ptr->data;
	}

	//! Static function to get the cell data from a cell pointer
	static const T& GetRefData(CellPtr cp)
	{
		ASSERT(cp.ptr);

		return cp.ptr->data;
	}


	
	CellPtr GetCellPtr(const_iterator it) const
	{
		ASSERT(it != lastIt)

		return it.ptr;
	}

	/*const_iterator GetIter(CellPtr p) const
	{
		const_iterator it;

		it.ptr = p.ptr;

		return it;
	}

	iterator GetIter(CellPtr p)
	{
		iterator it;

		it.ptr = p.ptr;

		return it;
	}*/

public:
	SimpleList()
	{
		lastIt.ptr = new Cell(NULL, NULL);
		firstIt = lastIt;
		cellCount = 0;
	}

	SimpleList(const SimpleList& rhs)
	{
		lastIt.ptr = new Cell(NULL, NULL);
		firstIt = lastIt;
		cellCount = 0;

		operator=(rhs);
	}

	~SimpleList()
	{
		clear();

		delete lastIt.ptr;
	}

	SimpleList& operator=(const SimpleList& rhs)
	{
		clear();

		for (const_iterator it = rhs.begin(); it != rhs.end(); ++it)
		{
			push_back(*it);
		}

		return *this;
	}

	void clear()
	{
		if (!empty())
		{
			Cell* last = lastIt.ptr;

			// Delete everything but "last"
			for (Cell* p = last->prev; p; p = last->prev)
			{
				last->prev = p->prev;

				delete p;
			}

			firstIt = lastIt;
			cellCount = 0;
		}
	}

	void push_back(const T& d)
	{
		Cell* last = lastIt.ptr;

		ASSERT(last->next == NULL);

		last->prev = new Cell(last->prev, last, d);

		if (last->prev->prev)
			last->prev->prev->next = last->prev;
		else
			firstIt.ptr = last->prev;

		++cellCount;
	}

	void erase(CellPtr cp)
	{
		ASSERT(!empty());
		ASSERT(cp.ptr->next);

		cp.ptr->next->prev = cp.ptr->prev;

		if (cp.ptr->prev)
			cp.ptr->prev->next = cp.ptr->next;

		--cellCount;

		delete cp.ptr;
	}

	unsigned size() const { return cellCount; }

	bool empty() const { return cellCount == 0; }

	iterator begin() { return firstIt; }

	const_iterator begin() const { return firstIt; }

	iterator end() { return lastIt; }

	const_iterator end() const { return lastIt; }

	void Serialize(OutputStream& os) const
	{
		Serialize(os, size());

		for (const_iterator it = begin(); it != end(); ++it)
			Serialize(os, *it);
	}

	void Deserialize(InputStream& is)
	{
		unsigned sz;

		Deserialize(is, sz);

		clear();

		T x;

		for (unsigned i = 0; i < sz; ++i)
		{
			Deserialize(is, x);
			push_back(x);
		}
	}
};

} // namespace vpl
