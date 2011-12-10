/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <sstream>
#include <array>
#include <string>
#include "Exceptions.h"

namespace vpl {

/*!
	NOTE: Only a few types T have an operator>> define below...
*/
template <typename T, unsigned DIM, char BEGIN_BRACKET = '{', char END_BRACKET = '}'> 
struct Tuple : public std::array<T, DIM>
{
	typedef std::array<T, DIM> BASE_CLASS;

	/*Tuple(std::initializer_list<T> list) : BASE_CLASS(list)
	{ 
		ASSERT(list.size() == size());
	}*/

	/*void set(std::initializer_list<T> list)
	{
		ASSERT(list.size() == size());

		assignt(list.begin(), list.end());
	}*/

	unsigned Dim() const
	{
		return DIM;
	}

	Tuple()
	{
		// Nothing else to do 
	}

	Tuple(T v0)
	{
		ASSERT(DIM == 1);
		at(0) = v0;
	}

	Tuple(T v0, T v1)
	{
		ASSERT(DIM == 2);
		at(0) = v0;
		at(1) = v1;
	}

	Tuple(T v0, T v1, T v2)
	{
		ASSERT(DIM == 3);
		at(0) = v0;
		at(1) = v1;
		at(2) = v2;
	}

	Tuple(T v0, T v1, T v2, T v3)
	{
		ASSERT(DIM == 4);
		at(0) = v0;
		at(1) = v1;
		at(2) = v2;
		at(3) = v3;
	}

	void clear()
	{
		for (auto it = begin(); it != end(); ++it)
			*it = T();
	}

	void operator=(const Tuple<T, DIM, BEGIN_BRACKET, END_BRACKET>& rhs)
	{
		ASSERT(size() == rhs.size());

		BASE_CLASS::operator=(rhs);
	}

	/*const T& operator[](unsigned i) const
	{
		return BASE_CLASS::operator[](i);
	}

	T& operator[](unsigned i)
	{
		return BASE_CLASS::operator[](i);
	}*/

	void Print(std::ostream& os = std::cout) const
	{
		std::ostringstream oss;

		oss << *this;

		os << oss.str();
	}

	/*!
		Writes a text representation of a Param object using the format '{sig,k,sz}'. 
	*/
	template <typename T, unsigned DIM, char BEGIN_BRACKET, char END_BRACKET>
	friend std::ostringstream& operator<<(std::ostringstream& os, 
		const Tuple<T, DIM, BEGIN_BRACKET, END_BRACKET>& st)
	{ 
		os << BEGIN_BRACKET;
		
		for (unsigned i = 1; i < st.size(); ++i)
			os << st[i - 1] << ",";
		
		os << st.back() << END_BRACKET; 

		return os;
	}
		
	// This not longer used. See below...
	/*template <typename T, unsigned DIM>
	friend std::istringstream& operator>>(std::istringstream& is, 
		const Tuple<T, DIM>& st) ;*/
};

// Specializations of the reading functions for tuples...

/*!
	Reads a tuple of type string
*/
template <unsigned DIM, char BEGIN_BRACKET, char END_BRACKET>
std::istringstream& operator>>(std::istringstream& is, 
	Tuple<std::string, DIM, BEGIN_BRACKET, END_BRACKET>& st) 
{ 
	// Ensure that there is no data in the tuple
	st.clear();

	char ch;

	// Read the 'begin bracked'
	is >> ch;

	if (ch != BEGIN_BRACKET)
		THROW_BASIC_EXCEPTION("Invalid string Tuple 'begin bracket'");

	// Read all fields and their separators
	for (unsigned i = 0; i < st.size(); ++i)
	{
		while (!is.eof()) 
		{
			is >> ch;

			if (ch != ',' && ch != END_BRACKET)
				st[i].push_back(ch);
			else
				break;
		}

		if (ch == END_BRACKET && i + 1 < st.size())
			THROW_BASIC_EXCEPTION("String Tuple has missing field(s)");
	}

	// The last character read must be the 'end bracket'
	if (ch != END_BRACKET)
		THROW_BASIC_EXCEPTION("Invalid string Tuple 'end bracket'");

	return is;
}

/*!
	Reads a tuple of type double
*/
template <unsigned DIM, char BEGIN_BRACKET, char END_BRACKET>
std::istringstream& operator>>(std::istringstream& is, 
	Tuple<double, DIM, BEGIN_BRACKET, END_BRACKET>& st) 
{ 
	char ch;
	std::string buffer;

	// Read the 'begin bracked'
	is >> ch;

	if (ch != BEGIN_BRACKET)
		THROW_BASIC_EXCEPTION("Invalid numeric Tuple 'begin bracket'");

	// Read all fields and their separators
	for (unsigned i = 0; i < st.size(); ++i)
	{
		buffer.clear();

		while (!is.eof()) 
		{
			is >> ch;
			
			if (ch != ',' && ch != END_BRACKET)
				buffer.push_back(ch);
			else
				break;
		}

		st[i] = atof(buffer.c_str());

		if (ch == END_BRACKET && i + 1 < st.size())
			THROW_BASIC_EXCEPTION("Numeric Tuple has missing field(s)");
	}
	
	// The last character read must be the 'end bracket'
	if (ch != END_BRACKET)
		THROW_BASIC_EXCEPTION("Invalid numeric Tuple 'end bracket'");

	return is;
}

/*!
	Reads a tuple of type unsigned
*/
template <unsigned DIM, char BEGIN_BRACKET, char END_BRACKET>
std::istringstream& operator>>(std::istringstream& is, 
	Tuple<unsigned, DIM, BEGIN_BRACKET, END_BRACKET>& st) 
{ 
	char ch;
	std::string buffer;

	// Read the 'begin bracked'
	is >> ch;

	if (ch != BEGIN_BRACKET)
		THROW_BASIC_EXCEPTION("Invalid numeric Tuple 'begin bracket'");

	// Read all fields and their separators
	for (unsigned i = 0; i < st.size(); ++i)
	{
		buffer.clear();

		while (!is.eof()) 
		{
			is >> ch;
			
			if (ch != ',' && ch != END_BRACKET)
				buffer.push_back(ch);
			else
				break;
		}

		st[i] = atoi(buffer.c_str());

		if (ch == END_BRACKET && i + 1 < st.size())
			THROW_BASIC_EXCEPTION("Unsigned integer Tuple has missing field(s)");
	}
	
	// The last character read must be the 'end bracket'
	if (ch != END_BRACKET)
		THROW_BASIC_EXCEPTION("Invalid unsigned integer Tuple 'end bracket'");

	return is;
}

/*!
		
*/
/*template <typename T, unsigned DIM>
std::istringstream& operator>>(std::istringstream& is, Tuple<T, DIM>& st) 
{ 
	// Handle the special 0-DIM tuple
	if (st.empty())
		return is;

	char sep;

	// Read the 'begin bracked'
	is >> sep;

	if (sep != '{')
		THROW_BASIC_EXCEPTION("Invalid Tuple 'begin bracket'");

	// Rea all but the last fields and their separators
	for (unsigned i = 1; i < st.size(); ++i)
	{
		is >> st[i - 1] >> sep;

		if (sep != ',')
			THROW_BASIC_EXCEPTION("Invalid Tuple separator");
	}

	// Read the last field and the 'end bracked'
	is >> st.back() >> sep;

	if (sep != '}')
		THROW_BASIC_EXCEPTION("Invalid Tuple 'end bracket'");

	return is;
}*/

} // namespace vpl

