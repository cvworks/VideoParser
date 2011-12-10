/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <sstream>
#include <vector>
#include <iomanip> // for std::setw()

//! Prints the vector using the std::ostream
template<class T> void PrintSTLVector(const std::vector<T>& vec, 
	std::ostream& os, int width = 0)
{
	if (!vec.empty())
	{
		std::ostringstream oss;

		auto it = vec.begin();

		oss << *it;

		for (++it; it != vec.end(); ++it)
			oss << "," << *it;

		if (width > 0)
			os << std::setw(width);
			
		os << oss.str();
	}
	else if (width > 0)
	{
		os << std::setw(width) << " ";
	}
}
