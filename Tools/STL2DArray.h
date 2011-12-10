/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "STLUtils.h"

template <typename T> class STL2DArray 
	: public std::vector<std::vector<unsigned>>
{
	typedef std::vector<std::vector<unsigned>> ParentClass;
public:
	typedef ParentClass::iterator iterator;
	typedef ParentClass::const_iterator const_iterator;

	const T& get(unsigned i, unsigned j) const
	{
		ASSERT(i < size() && j < operator[](j));

		return operator[](i)[j];
	}

	T& get(unsigned i, unsigned j)
	{
		ASSERT(i < size() && j < operator[](j));

		return operator[](i)[j];
	}

	T maxValue() const
	{
		ASSERT(!empty());

		T maxVal = front().front();

		for (auto it0 = begin(); it0 != end(); ++it0)
			for (auto it1 = it0->begin(); it1 != it0->end(); ++it1)
				if (*it1 > maxVal)
					maxVal = *it1;

		return maxVal;
	}
};