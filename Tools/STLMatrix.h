/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <vector>

//! Simple matrix created using STL vectors
template <typename T> class STLMatrix : private std::vector<std::vector<T>>
{
	typedef std::vector<std::vector<T>> ParentClass;

public:
	STLMatrix()
	{
	}

	STLMatrix(unsigned ni, unsigned nj)
		: ParentClass(ni)
	{
		for (auto it = ParentClass::begin(); it != ParentClass::end(); ++it)
			it->resize(nj);
	}

	STLMatrix(unsigned ni, unsigned nj, T val)
		: std::vector<std::vector<T>>(ni)
	{
		for (auto it = ParentClass::begin(); it != ParentClass::end(); ++it)
			it->resize(nj, val);
	}

	void resize(unsigned ni, unsigned nj)
	{
		ParentClass::resize(ni);

		for (auto it = ParentClass::begin(); it != ParentClass::end(); ++it)
			it->resize(nj);
	}

	void resize(unsigned ni, unsigned nj, T val)
	{
		ParentClass::resize(ni);

		for (auto it = ParentClass::begin(); it != ParentClass::end(); ++it)
			it->resize(nj, val);
	}

	T& operator()(unsigned i, unsigned j)
	{
		return (*this)[i][j];
	}

	const T& operator()(unsigned i, unsigned j) const
	{
		return (*this)[i][j];
	}

	T& get(unsigned i, unsigned j)
	{
		return operator()(i, j);
	}

	const T& get(unsigned i, unsigned j) const
	{
		return operator()(i, j);
	}

	void operator=(const STLMatrix& rhs)
	{
		ParentClass::resize(rhs.ni());

		for (unsigned i = 0; i < size(); ++i)
			(*this)[i] = rhs[i];
	}

	bool empty() const  { return ParentClass::empty(); }

	unsigned ni() const { return size(); }
	unsigned nj() const { return (empty()) ? 0 : front().size(); }

	struct iterator
	{
		unsigned i, j, ni;
	
		iterator(unsigned col, unsigned row, unsigned cols)
			: i(col), j(row), ni(cols) { }

		iterator& operator++() 
		{ 
			if (++i >= ni)
			{
				i = 0;
				j++;
			}

			return *this; 
		}

		bool operator==(const iterator& rhs) const
		{
			return (i == rhs.i && j == rhs.j);
		}

		bool operator!=(const iterator& rhs) const
		{
			return !operator==(rhs);
		}
	};

	iterator begin() const
	{
		return iterator(0, 0, ni());
	}

	iterator end() const
	{
		return iterator(ni(), nj(), ni());
	}
};
