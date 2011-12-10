/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "BasicUtils.h"
#include "STLUtils.h"

/*! 
	Returns an STL string with the name of the type T (which can 
	be a polymorphic type).

	The typeid operator allows the type of an object to be determined at run time.
	The typeid operator does a run-time check when applied to an l-value of a 
	polymorphic class type.

	The type_info::raw_name member function returns a const char* to a null-terminated 
	string representing the decorated name of the object type. This is faster than
	calling type_info::name, because the name is stored in is decorated form.
*/
#define vpl_TypeName(T) std::string(typeid(T).raw_name())

typedef std::istream InputStream;
typedef std::ostream OutputStream;

typedef void* VOID_PTR;

/////////////////////////////////////////////////////////////////////////////
// Macro to define member Serialize / Deserialize of basic types 

/*! 
	Macro to declare the Deserialization and Serialization member functions 
	of basic types. 
	
	Note that this macro does NOT declare virtual serialization functions.
*/
#define DECLARE_BASIC_MEMBER_SERIALIZATION \
	void Serialize(OutputStream& os) const \
	{ \
		os.write((char*) this, sizeof(*this)); \
	} \
	void Deserialize(InputStream& is) \
	{ \
		is.read((char*) this, sizeof(*this)); \
	}

/////////////////////////////////////////////////////////////////////////////
// Macro to call the Serialize / Deserialize non-member functions of a base class

/*! 
	Macro to call the non-member serialize function of a base class.
*/
#define CALL_BASE_CLASS_GLOBAL_SERIALIZE(OS, T) ::Serialize(OS, (*(T*)this))

/*! 
	Macro to call the non-member deserialize function of a base class.
*/
#define CALL_BASE_CLASS_GLOBAL_DESERIALIZE(IS, T) ::Deserialize(IS, (*(T*)this))

/////////////////////////////////////////////////////////////////////////////
// Serialize / Deserialize basic types 

/*! 
	Macro to declare the Deserialization and Serialization functions of
	basic types.

	This Macro can be used to declare such functions for additional types 
	not included in this header file.  When doing this, it is necessary
	that the macro is inclused within the appropriate namespace. e.g.,
	namespace DECLARE_BASIC_SERIALIZATION(my_simple_type)
*/
#define DECLARE_BASIC_SERIALIZATION(T) \
	inline void Serialize(OutputStream& os, const T& x) \
	{ \
		os.write((char*) &x, sizeof(x)); \
	} \
	inline void Deserialize(InputStream& is, T& x) \
	{ \
		is.read((char*) &x, sizeof(x)); \
	} \

DECLARE_BASIC_SERIALIZATION(bool)
DECLARE_BASIC_SERIALIZATION(char)
DECLARE_BASIC_SERIALIZATION(unsigned char)
DECLARE_BASIC_SERIALIZATION(int)
DECLARE_BASIC_SERIALIZATION(unsigned int)
DECLARE_BASIC_SERIALIZATION(long)
DECLARE_BASIC_SERIALIZATION(unsigned long)
DECLARE_BASIC_SERIALIZATION(float)
DECLARE_BASIC_SERIALIZATION(double)
DECLARE_BASIC_SERIALIZATION(std::streampos)
DECLARE_BASIC_SERIALIZATION(std::streamoff)
DECLARE_BASIC_SERIALIZATION(vpl::Point)

DECLARE_BASIC_SERIALIZATION(VOID_PTR)

/////////////////////////////////////////////////////////////////////////////
// Serialize / Deserialize STL string

inline void Serialize(OutputStream& os, const std::string& x)
{
	unsigned sz = x.size();

	Serialize(os, sz);

	if (sz > 0)
		os.write((char*) x.c_str(), sz);
}

inline void Deserialize(InputStream& is, std::string& x)
{
	unsigned sz;

	Deserialize(is, sz);

	if (sz > 0)
	{
		char* charArray = new char[sz];

		is.read(charArray, sz);

		x.assign(charArray, sz);

		delete[] charArray;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Serialize / Deserialize STL pair

template<typename T, typename U>
inline void Serialize(OutputStream& os, const std::pair<T, U>& P) 
{
	Serialize(os, P.first);
	Serialize(os, P.second);
}

template<typename T, typename U>
inline void Deserialize(InputStream& is, std::pair<T, U>& P) 
{
	Deserialize(is, P.first);
	Deserialize(is, P.second);
}

/////////////////////////////////////////////////////////////////////////////
// Serialize STL containers 
template<typename T> 
inline void Serialize(OutputStream& os, const std::vector<T>& c)
{
	Serialize(os, c.size());

	for (std::vector<T>::const_iterator I = c.begin(); I != c.end(); ++I)
		Serialize(os, *I);
}

template<typename T> 
inline void Serialize(OutputStream& os, const std::list<T>& c)
{
	Serialize(os, c.size());

	for (std::list<T>::const_iterator I = c.begin(); I != c.end(); ++I)
		Serialize(os, *I);
}

template<typename T, typename U> 
inline void Serialize(OutputStream& os, const std::map<T,U>& c)
{
	Serialize(os, c.size());

	for (std::map<T,U>::const_iterator I = c.begin(); I != c.end(); ++I)
		Serialize(os, *I);
}

/////////////////////////////////////////////////////////////////////////////
// Deserialize STL containers
template<typename T>
inline void Deserialize(InputStream& is, std::vector<T>& C) 
{
	unsigned sz;

	Deserialize(is, sz);

	C.clear();
	C.resize(sz);

	for (unsigned i = 0; i < sz; ++i)
		Deserialize(is, C[i]);
}

template<typename T>
inline void Deserialize(InputStream& is, std::list<T>& C) 
{
	unsigned sz;

	Deserialize(is, sz);

	C.clear();
	T x;

	for (unsigned i = 0; i < sz; ++i)
	{
		Deserialize(is, x);
		C.push_back(x);
	}
}

template<typename T, typename U>
inline void Deserialize(InputStream& is, std::map<T, U>& C) 
{
	unsigned sz;

	Deserialize(is, sz);

	C.clear();

	T x;
	U y;

	for (unsigned i = 0; i < sz; ++i)
	{
		Deserialize(is, x);
		Deserialize(is, y);
		C[x] = y;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Serialize and Deserialize class objects 

template <typename T>
inline void Serialize(OutputStream& os, const T& x)
{
	x.Serialize(os);
}

template <typename T>
inline void Deserialize(InputStream& is, T& x)
{
	x.Deserialize(is);
}

