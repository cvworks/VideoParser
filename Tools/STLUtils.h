/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "BasicTypes.h"
#include <list>
#include <vector>
#include <map>
#include <string>

typedef std::pair<std::string, int> StrIntPair;
typedef std::pair<int, double> IntDoublePair;

typedef std::list<int>   IntList;
typedef std::list<unsigned int> UIntList;
typedef std::list<bool>  BoolList;
typedef std::list<float> FloatList;
typedef std::list<double> DoubleList;
typedef std::list<vpl::Point> PointList;
typedef std::list<std::string> StrList;
typedef std::list<const char*> CSzList;

typedef std::list<StrIntPair> StrIntList;
typedef std::list<IntDoublePair> IntDoubleList;

typedef std::vector<int>   IntArray;
typedef std::vector<unsigned int> UIntArray;
typedef std::vector<bool>  BoolArray;
typedef std::vector<float> FloatArray;
typedef std::vector<double> DoubleArray;
typedef std::vector<vpl::Point> PointArray;
typedef std::vector<std::string> StrArray;
typedef std::vector<const char*> CSzArray;

#define std_forall(I, C) for (auto I = (C).begin(); I != (C).end(); ++I)

void append_num(std::string& str, int n);
void append_num(std::string& str, unsigned n);

template <typename T>
inline void copy_back(const std::vector<T>& src, std::vector<T>& dst)
{
	dst.reserve(dst.size() + src.size());

	dst.insert(dst.end(), src.begin(), src.end());
}

template <typename T>
inline void copy_back(const std::list<T>& src, std::list<T>& dst)
{
	dst.insert(dst.end(), src.begin(), src.end());
}

template <typename T>
inline void move_back(std::list<T>& src, std::list<T>& dst)
{
	dst.splice(dst.end(), src);
}

inline std::string replace_char(const std::string& strIn, 
	char oldChar, char newChar)
{
	std::string strOut(strIn);

	for (auto it = strOut.begin(); it != strOut.end(); ++it)
		if (*it == oldChar)
			*it = newChar;

	return strOut;
}

inline std::string replace_str(const std::string& strIn, 
	const char* oldSubstr, const char* newSubstr)
{
	std::string strOut(strIn);

	size_t i = 0;
	size_t n = strlen(oldSubstr);

	while ((i = strOut.find(oldSubstr, i)) != std::string::npos)
	{
		strOut.replace(i, n, newSubstr);
		i += n;
	}

	return strOut;
}

/*!
	Splits an input string into a list of tokens stored as a StrList.
	The input list is NOT cleared before adding the tokens.
*/
inline void Tokenize(const std::string& text, const char* separators,
					 StrList& words)
{
	int n = text.length();
	int start, stop;
	
	start = text.find_first_not_of(separators);
	
	while (start >= 0 && start < n)
	{
		stop = text.find_first_of(separators, start);

		if (stop < 0 || stop > n) 
			stop = n;

		words.push_back(text.substr(start, stop - start));

		start = text.find_first_not_of(separators, stop + 1);
	}
}

/*!
	Splits an input string into a list of tokens stored as a StrList.
	The default separator are a space and a comma.
*/
/*inline StrList Tokenize(const std::string& text, 
						const char* separators = " ,")
{
	StrList words;

	Tokenize(text, separators, words);

	return words;
}*/

/*!
	Splits an input string into an array of tokens stored as a StrArray.
	The default separator are a space and a comma.
*/
inline StrArray Tokenize(const std::string& text, 
						 const char* separators = " ,")
{
	StrList words;

	Tokenize(text, separators, words);

	return StrArray(words.begin(), words.end());
}

/*!
	Copies at most 'count' characters from 'str'. That is, if str
	has less than 'count' characters, then the number of characters 
	copied is less than 'count'.

	Note: count must be smaller than dest_size, so that there is 
	room for th enull character. Use count = 0 to set
	count to dest_size - 1.
*/
inline void string_copy(const std::string& str, char* sz, 
	unsigned dest_size, unsigned count = 0)
{
	ASSERT(dest_size > 0);

	dest_size--; // for the null character

	if (count == 0)
		count = dest_size;

	ASSERT(dest_size >= count);

	WARNING(str.size() > dest_size, "String is too long");

#ifdef WIN32
		unsigned n = str._Copy_s(sz, dest_size, count);
#else
		unsigned n = str.copy(name, count);
#endif

	sz[n] = '\0';
}

/*! 
	Returns a zero-terminate copy of the string, which must be 
	deleted by the caller of the function;
*/
inline char* string_copy(const std::string& str)
{
	unsigned dest_size = str.size() + 1;

	char* sz = new char[dest_size];

	string_copy(str, sz, dest_size);

	return sz;
}

//! Appends tail to dest
template <typename T> void append(std::list<T>& dest, const std::list<T>& tail)
{
	dest.insert(dest.end(), tail.begin(), tail.end());
}

//! Finds element in list
template<class T> typename std::list<T>::const_iterator
	find(const std::list<T>& l, const T& val)
{
	return std::find(l.begin(), l.end(), val);
}

//! Checks if the element is in the list
template<class T> typename bool contains(const std::list<T>& l, const T& val)
{
	return std::find(l.begin(), l.end(), val) != l.end();
}

//! Checks if the element is in the list
template<class T> typename bool contains(const std::vector<T>& l, const T& val)
{
	return std::find(l.begin(), l.end(), val) != l.end();
}

/*!
	Returns the iterator to the element in the list 'l' at position 'at'.

	@toso Use std::advance instead.

	ie, 

	auto it = l.begin();
	std::advance(it, i);
	return it;
*/
template<class T> typename std::list<T>::iterator 
	iterator_at(std::list<T>& l, unsigned int i)
{
	for (std::list<T>::iterator it = l.begin(); it != l.end(); 
		++it, --i)
	{
		if (i == 0)
			return it;
	}

	return l.end();
}

/*!
	Returns the iterator to the element in the list 'l' at position 'at'.
*/
template<class T> typename std::list<T>::const_iterator
	iterator_at(const std::list<T>& l, unsigned int i)
{
	for (std::list<T>::const_iterator it = l.begin(); it != l.end(); 
		++it, --i)
	{
		if (i == 0)
			return it;
	}

	return l.end();
}

/*!
	Returns the element in the list 'l' at position 'at'.
*/
template<class T> T& element_at(std::list<T>& l, unsigned int i)
{
	for (std::list<T>::iterator it = l.begin(); it != l.end(); 
		++it, --i)
	{
		if (i == 0)
			return *it;
	}

	return l.front();
}

/*!
	Returns the element in the list 'l' at position 'at'.
*/
template<class T> const T& element_at(const std::list<T>& l, unsigned int i)
{
	for (std::list<T>::const_iterator it = l.begin(); it != l.end(); 
		++it, --i)
	{
		if (i == 0)
			return *it;
	}

	return l.front();
}

//! Returns the last character in a string
inline char last_char(const std::string& str)
{
	return str[str.length() - 1];
}

//! Converts the given containter to a vector of ints
template<class T> std::vector<int> ToIntVector(const T& cont)
{
	std::vector<int> nums(cont.size());

	unsigned i = 0;

	for (auto it = cont.begin(); it != cont.end(); ++it, ++i)
		nums[i] = int(*it);

	return nums;
}

/*!
	STL vector that can be initialized as

	STLInitVector<T> v(a, b, c,...); //-> v[0]= a, ..., v[2]= c
*/
template <typename T> struct STLInitVector : public std::vector<T>
{
	STLInitVector() { }

	STLInitVector(T e0) : std::vector<T>(1, e0) { }
	
	STLInitVector(T e0, T e1) 
		: std::vector<T>(2) 
	{ 
		operator[](0) = e0;
		operator[](1) = e1;
	}

	STLInitVector(T e0, T e1, T e2) 
		: std::vector<T>(3) 
	{ 
		operator[](0) = e0;
		operator[](1) = e1;
		operator[](2) = e2;
	}

	STLInitVector(T e0, T e1, T e2, T e3) 
		: std::vector<T>(4) 
	{ 
		operator[](0) = e0;
		operator[](1) = e1;
		operator[](2) = e2;
		operator[](3) = e3;
	}

	STLInitVector(T e0, T e1, T e2, T e3, T e4) 
		: std::vector<T>(5) 
	{ 
		operator[](0) = e0;
		operator[](1) = e1;
		operator[](2) = e2;
		operator[](3) = e3;
		operator[](4) = e4;
	}

	STLInitVector(T e0, T e1, T e2, T e3, T e4,
		T e5) : std::vector<T>(6) 
	{ 
		operator[](0) = e0;
		operator[](1) = e1;
		operator[](2) = e2;
		operator[](3) = e3;
		operator[](4) = e4;
		operator[](5) = e5;
	}

	STLInitVector(T e0, T e1, T e2, T e3, T e4,
		T e5, T e6) : std::vector<T>(7) 
	{ 
		operator[](0) = e0;
		operator[](1) = e1;
		operator[](2) = e2;
		operator[](3) = e3;
		operator[](4) = e4;
		operator[](5) = e5;
		operator[](6) = e6;
	}

	STLInitVector(T e0, T e1, T e2, T e3, T e4,
		T e5, T e6, T e7) : std::vector<T>(8) 
	{ 
		operator[](0) = e0;
		operator[](1) = e1;
		operator[](2) = e2;
		operator[](3) = e3;
		operator[](4) = e4;
		operator[](5) = e5;
		operator[](6) = e6;
		operator[](7) = e7;
	}
};

