/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <vector>
#include "BasicUtils.h"

namespace vpl {

//! Simple matrix created using an *private* STL vector ancestor
template <typename T> class SimpleMatrix : private std::vector<T>
{
	typedef std::vector<T> ParentClass;

	size_t m_rows;
	size_t m_cols;

public:
	typedef ParentClass::iterator iterator;
	typedef ParentClass::const_iterator const_iterator;

public:
	SimpleMatrix()
	{
		m_rows = 0;
		m_cols = 0;
	}

	SimpleMatrix(const SimpleMatrix& rhs) : ParentClass(rhs)
	{
		m_rows = rhs.m_rows;
		m_cols = rhs.m_cols;
	}

	SimpleMatrix(SimpleMatrix&& rhs) : ParentClass(rhs)
	{
		m_rows = rhs.m_rows;
		m_cols = rhs.m_cols;
	}

	SimpleMatrix(unsigned ni, unsigned nj)
		: ParentClass(ni * nj)
	{
		m_rows = ni;
		m_cols = nj;
	}

	SimpleMatrix(unsigned ni, unsigned nj, T val)
		: ParentClass(ni * nj, val)
	{
		m_rows = ni;
		m_cols = nj;
	}

	iterator begin()             { return ParentClass::begin(); }
	const_iterator begin() const { return ParentClass::begin(); }

	iterator end()             { return ParentClass::end(); }
	const_iterator end() const { return ParentClass::end(); }

	void resize(unsigned ni, unsigned nj)
	{
		ParentClass::resize(ni * nj);

		m_rows = ni;
		m_cols = nj;
	}

	void resize(unsigned ni, unsigned nj, T val)
	{
		ParentClass::resize(ni * nj, val);

		m_rows = ni;
		m_cols = nj;
	}

	T& operator()(unsigned i, unsigned j)
	{
		if (i >= m_rows || j >= m_cols)
		{
			DBG_PRINT4(i, m_rows, j, m_cols)
		}

		ASSERT(i < m_rows && j < m_cols);

		return (*this)[i * m_cols + j];
	}

	const T& operator()(unsigned i, unsigned j) const
	{
		if (i >= m_rows || j >= m_cols)
		{
			DBG_PRINT4(i, m_rows, j, m_cols)
		}

		ASSERT(i < m_rows && j < m_cols);

		return (*this)[i * m_cols + j];
	}

	T& get(unsigned i, unsigned j)
	{
		return operator()(i, j);
	}

	const T& get(unsigned i, unsigned j) const
	{
		return operator()(i, j);
	}

	void operator=(const SimpleMatrix& rhs)
	{
		ParentClass::operator=(rhs);

		m_rows = rhs.m_rows;
		m_cols = rhs.m_cols;
	}

	void clear()
	{
		ParentClass::clear();

		m_rows = 0;
		m_cols = 0;
	}

	bool empty() const  { return ParentClass::empty(); }

	unsigned ni() const { return m_rows; }
	unsigned nj() const { return m_cols; }

	T maxj(unsigned j) const
	{
		T maxVal = get(0, j);

		for (unsigned i = 1; i < ni(); i++)
		{
			const T& val = get(i, j);

			if (val > maxVal)
				maxVal = val;
		}

		return maxVal;
	}
};

} // namespace vpl

