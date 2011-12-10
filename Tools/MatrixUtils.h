/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <vector>
#include <vnl/vnl_matrix.h>
#include <vnl/io/vnl_io_matrix.h>
#include <vnl/io/vnl_io_vector.h>
#include <vsl/vsl_binary_io.h>
#include <Tools/Serialization.h>
#include <Tools/Exceptions.h>

#define CHECK_ROWS(A, B) \
	if ((A).rows() != (B).rows()) \
		THROW_BASIC_EXCEPTION("Both matrices must be of the same row dimension")

#define CHECK_COLS(A, B) \
	if ((A).cols() != (B).cols()) \
		THROW_BASIC_EXCEPTION("Both matrices must be of the same column dimension")

#define CHECK_ROWS_AND_COLS(A, B) \
	if ((A).rows() != (B).rows() || (A).cols() != (B).cols()) \
		THROW_BASIC_EXCEPTION("Both matrices must be of the same row and column dimensions")

#define CHECK_SIZE(A, B) \
	if ((A).size() != (B).size()) \
		THROW_BASIC_EXCEPTION("Both matrices must have the same size")

#define init_cells2(m, a, b) \
	{ auto it = m.begin(); *it++ = a; *it++ = b; }

#define init_cells3(m, a, b, c) \
	{ auto it = m.begin(); *it++ = a; *it++ = b; *it++ = c; }

#define init_cells4(m, a, b, c, d) \
	{ auto it = m.begin(); *it++ = a; *it++ = b; *it++ = c; *it++ = d; }

#define init_cells5(m, a, b, c, d, e) \
	{ auto it = m.begin(); *it++ = a; *it++ = b; *it++ = c; *it++ = d; *it++ = e; }

#define init_cells6(m, a, b, c, d, e, f) \
	{ auto it = m.begin(); *it++ = a; *it++ = b; *it++ = c; *it++ = d; *it++ = e; *it++ = f; }

// Note: the end() function of matrix takes some time
#define forall_cells1(m0) \
	auto it0 = (m0).begin(); \
	const auto it0_end = (m0).end(); \
	for (; it0 != it0_end; ++it0)

// Note: the end() function of matrix takes some time
#define forall_cells2(m0, m1) \
	CHECK_SIZE(m0, m1); \
	auto it0 = (m0).begin(); \
	auto it1 = (m1).begin(); \
	const auto it0_end = (m0).end(); \
	for (; it0 != it0_end; ++it0, ++it1)

// Note: the end() function of matrix takes some time
#define forall_cells3(m0, m1, m2) \
	CHECK_SIZE(m0, m1); \
	CHECK_SIZE(m0, m2); \
	auto it0 = (m0).begin(); \
	auto it1 = (m1).begin(); \
	auto it2 = (m2).begin(); \
	const auto it0_end = (m0).end(); \
	for (; it0 != it0_end; ++it0, ++it1, ++it2)

//! Debug macro for writing a matrix as a Matlab assignment "L_N = M;" in a log file (LF)
#define DBG_LOG_MATRIX(LF, M, L, N) DBG_LOG_CALL3(PrintMatrix, LF, M, L, N)

//! Debug macro for writing a vector as a Matlab assignment "L_N = V;" in a log file (LF)
#define DBG_LOG_VECTOR(LF, V, L, N) DBG_LOG_CALL3(PrintVector, LF, V, L, N)

//! Debug macro for writing a scalar as a Matlab assignment "L_N = S;" in a log file (LF)
#define DBG_LOG_SCALAR(LF, S, L, N) DBG_LOG_CALL3(PrintScalar, LF, S, L, N)

template <class T> struct RowRef
{
	T* rowData;
	unsigned size;
	unsigned curIdx;

	RowRef(T* p, unsigned n) : rowData(p)
	{
		size = n;
		curIdx = 0;
	}

	RowRef& operator<<(const T& val)
	{
		DBG_STREAM_IF("size=" << size << " index=" << curIdx, curIdx >= size)
		ASSERT(curIdx < size);

		rowData[curIdx++] = val;

		return *this;
	}
};

//////////////////////////////////////////////////////////////////////////////
// Begin global arithmetic operators
/*template <class T>
vnl_matrix<T> operator+(const T& x, const vnl_matrix<T>& m)
{
	return m.operator+(x);
}

template <class T>
vnl_matrix<T> operator-(const T& x, const vnl_matrix<T>& m)
{
	vnl_matrix<T> r(m.rows(), m.cols());

	forall_cells2(r, m)
		*it0 = x - *it1;

	return r;
}

template <class T>
vnl_matrix<T> operator*(const T& x, const vnl_matrix<T>& m)
{
	return m.operator*(x);
}

template <class T>
vnl_matrix<T> operator/(const T& x, const vnl_matrix<T>& m)
{
	vnl_matrix<T> r(m.rows(), m.cols());

	forall_cells2(r, m)
		*it0 = x / *it1;

	return r;
}*/

//////////////////////////////////////////////////////////////////////////////
// Begin general functions [unary]

//! Element-wise square root
template <class T>
vnl_matrix<T> sqrt(const vnl_matrix<T>& m)
{
	vnl_matrix<T> r(m.rows(), m.cols());

	forall_cells2(r, m)
		*it0 = sqrt(*it1);

	return r;
}

//! Element-wise square power
template <class T>
vnl_matrix<T> epow2(const vnl_matrix<T>& m)
{
	vnl_matrix<T> r(m.rows(), m.cols());

	forall_cells2(r, m)
		*it0 = (*it1) * (*it1);

	return r;
}

template <class T>
T epow2(const T& v)
{
	return v * v;
}

//! Element-wise power
template <class T>
vnl_matrix<T> epow(const vnl_matrix<T>& m, double e)
{
	vnl_matrix<T> r(m.rows(), m.cols());

	forall_cells2(r, m)
		*it0 = pow(*it1, e);

	return r;
}

template <class T>
T epow(const T& v, double e)
{
	return pow(v, e);
}

//! Element-wise sign function
template<class T> 
vnl_matrix<T> sign(const vnl_matrix<T>& m)
{
	vnl_matrix<T> r(m.rows(), m.cols());

	forall_cells2(r, m)
		*it0 = (*it1 > 0) ? 1 : ((*it1 < 0) ? -1 : 0);

	return r;
}

//! Element-wise absolute value
template<class T> 
vnl_matrix<T> absval(const vnl_matrix<T>& m)
{
	vnl_matrix<T> r(m.rows(), m.cols());

	forall_cells2(r, m)
		*it0 = fabs(*it1);

	return r;
}

//! Element-wise sine
template<class T> 
vnl_matrix<T> sin(const vnl_matrix<T>& m)
{
	vnl_matrix<T> r(m.rows(), m.cols());

	forall_cells2(r, m)
		*it0 = std::sin(*it1);

	return r;
}

//! Element-wise cosine
template<class T> 
vnl_matrix<T> cos(const vnl_matrix<T>& m)
{
	vnl_matrix<T> r(m.rows(), m.cols());

	forall_cells2(r, m)
		*it0 = std::cos(*it1);

	return r;
}

//! Element-wise atan2
template<class T> 
vnl_matrix<T> atan2(const vnl_matrix<T>& x, const vnl_matrix<T>& y)
{
	CHECK_SIZE(x, y);

	vnl_matrix<T> r(x.rows(), x.cols());

	forall_cells3(r, x, y)
		*it0 = std::atan2(*it1, *it2);

	return r;
}

//! Returns true  if S is a 1 x 1 matrix
//! and false  otherwise.
template <class T>
inline bool isscalar(const vnl_matrix<T>& m)
{
	return m.size() == 1;
}

//! Get the size as a [rows(), cols()] vector of type 'double'
template <class T>
inline vnl_vector<double> size(const vnl_matrix<T>& m)
{
	vnl_vector<double> sz(2);

	sz(0) = m.rows();
	sz(1) = m.cols();

	return sz;
}

//! Get the size as a 'double' type
template <class T>
inline double size_d(const vnl_matrix<T>& m)
{
	return m.size();
}

//! Count all the true elements of the bool matrix
template <class T>
inline unsigned count(const vnl_matrix<bool>& m)
{
	unsigned n = 0;

	forall_cells1(m)
		if (*it0)
			n++;

	return n;
}

//////////////////////////////////////////////////////////////////////////////
// Begin general functions [binary]

//! Element-wise multiplication
template<class T> 
vnl_matrix<T> emul(const vnl_matrix<T>& m0, const vnl_matrix<T>& m1)
{
	CHECK_SIZE(m0, m1);

	vnl_matrix<T> r(m0.rows(), m0.cols());

	forall_cells3(r, m0, m1)
		*it0 = (*it1) * (*it2);

	return r;
}

//! Element-wise division
template <class T>
vnl_matrix<T> ediv(const vnl_matrix<T>& m0, const vnl_matrix<T>& m1)
{
	CHECK_SIZE(m0, m1);

	vnl_matrix<T> r(m0.rows(), m0.cols());

	forall_cells3(r, m0, m1)
		*it0 = (*it1) / (*it2);

	return r;
}

//////////////////////////////////////////////////////////////////////////////
// Begin casting functions

template <class T, class U>
vnl_matrix<T> cast(const vnl_matrix<U>& m)
{
	vnl_matrix<T> r(m.rows(), m.cols());

	forall_cells2(r, m)
		*it0 = T(*it1);

	return r;
}

//////////////////////////////////////////////////////////////////////////////
// Begin comparison functions

template <class T>
vnl_matrix<bool> operator>(const vnl_matrix<T>& lhs, const vnl_matrix<T>& rhs)
{
	vnl_matrix<bool> r(lhs.rows(), lhs.cols());
		
	forall_cells3(r, lhs, rhs)
		*it0 = *it1 > *it2;

	return r;
}

template <class T>
vnl_matrix<bool> operator>(const vnl_matrix<T>& lhs, const T& rhs)
{
	vnl_matrix<bool> r(lhs.rows(), lhs.cols());
		
	forall_cells2(r, lhs)
		*it0 = *it1 > rhs;

	return r;
}
 
template <class T>
vnl_matrix<bool> operator<(const vnl_matrix<T>& lhs, const vnl_matrix<T>& rhs)
{
	vnl_matrix<bool> r(lhs.rows(), lhs.cols());
		
	forall_cells3(r, lhs, rhs)
		*it0 = *it1 < *it2;

	return r;
}

template <class T>
vnl_matrix<bool> operator<(const vnl_matrix<T>& lhs, const T& rhs)
{
	vnl_matrix<bool> r(lhs.rows(), lhs.cols());
		
	forall_cells2(r, lhs)
		*it0 = *it1 < rhs;

	return r;
}
	
template <class T>
vnl_matrix<bool> operator==(const vnl_matrix<T>& lhs, const vnl_matrix<T>& rhs)
{
	vnl_matrix<bool> r(lhs.rows(), lhs.cols());
		
	forall_cells3(r, lhs, rhs)
		*it0 = *it1 == *it2;

	return r;
}

template <class T>
vnl_matrix<bool> operator==(const vnl_matrix<T>& lhs, const T& rhs)
{
	vnl_matrix<bool> r(lhs.rows(), lhs.cols());
		
	forall_cells2(r, lhs)
		*it0 = *it1 == rhs;

	return r;
}

template <class T>
vnl_matrix<bool> operator!=(const vnl_matrix<T>& lhs, const vnl_matrix<T>& rhs)
{
	vnl_matrix<bool> r(lhs.rows(), lhs.cols());
		
	forall_cells3(r, lhs, rhs)
		*it0 = *it1 != *it2;

	return r;
}

template <class T>
vnl_matrix<bool> operator!=(const vnl_matrix<T>& lhs, const T& rhs)
{
	vnl_matrix<bool> r(lhs.rows(), lhs.cols());
		
	forall_cells2(r, lhs)
		*it0 = *it1 != rhs;

	return r;
}

template <class T>
vnl_matrix<bool> operator>=(const vnl_matrix<T>& lhs, const vnl_matrix<T>& rhs)
{
	vnl_matrix<bool> r(lhs.rows(), lhs.cols());
		
	forall_cells3(r, lhs, rhs)
		*it0 = *it1 >= *it2;

	return r;
}

template <class T>
vnl_matrix<bool> operator>=(const vnl_matrix<T>& lhs, const T& rhs)
{
	vnl_matrix<bool> r(lhs.rows(), lhs.cols());
		
	forall_cells2(r, lhs)
		*it0 = *it1 >= rhs;

	return r;
}

template <class T>
vnl_matrix<bool> operator<=(const vnl_matrix<T>& lhs, const vnl_matrix<T>& rhs)
{
	vnl_matrix<bool> r(lhs.rows(), lhs.cols());
		
	forall_cells3(r, lhs, rhs)
		*it0 = *it1 <= *it2;

	return r;
}

template <class T>
vnl_matrix<bool> operator<=(const vnl_matrix<T>& lhs, const T& rhs)
{
	vnl_matrix<bool> r(lhs.rows(), lhs.cols());
		
	forall_cells2(r, lhs)
		*it0 = *it1 <= rhs;

	return r;
}

//////////////////////////////////////////////////////////////////////////////////
// Begin logic operators

//! Element-wise AND function
inline
vnl_matrix<bool> operator&(const vnl_matrix<bool>& lhs,
	const vnl_matrix<bool>& rhs)
{
	vnl_matrix<bool> r(lhs.rows(), lhs.cols());
		
	forall_cells3(r, lhs, rhs)
		*it0 = *it1 && *it2;

	return r;
}

//! Element-wise OR function
inline
vnl_matrix<bool> operator|(const vnl_matrix<bool>& lhs,
	const vnl_matrix<bool>& rhs)
{
	vnl_matrix<bool> r(lhs.rows(), lhs.cols());
		
	forall_cells3(r, lhs, rhs)
		*it0 = *it1 || *it2;

	return r;
}

//! Element-wise NOT function
inline 
vnl_matrix<bool> operator~(const vnl_matrix<bool>& rhs)
{
	vnl_matrix<bool> r(rhs.rows(), rhs.cols());
		
	forall_cells2(r, rhs)
		*it0 = !(*it1);

	return r;
}

//////////////////////////////////////////////////////////////////////////////////
// Define the (de)serialization of vpl_vector
template<typename T> 
inline void Serialize(OutputStream& os, const vnl_vector<T>& vec)
{
	vsl_b_ostream bos(&os);

	vsl_b_write(bos, vec);
}

template<typename T>
inline void Deserialize(InputStream& is, vnl_vector<T>& vec) 
{
	vsl_b_istream bis(&is);

	vsl_b_read(bis, vec);
}

//////////////////////////////////////////////////////////////////////////////////
/*!
	Prints the matrix in ASCII. If a label is given, the matrix is printed
	as a variable assignment that can be read by Malab.

	The optional numLabel creates a label of the form 'label_num', so that
	the assignemt becomes: 
	
    label_num = [...];
*/
template<class T> void PrintMatrix(vcl_ostream& os, const vnl_matrix<T>& m, 
								   const char* label = NULL, int numLabel = -1)
{
	if (label)
		os << label;

	if (numLabel >= 0)
		os << "_" << numLabel;
		
	os << " = [\n";

	for (unsigned i = 0; i < m.rows(); ++i)
	{
		for (unsigned j = 0; j < m.cols(); ++j)
			os << m(i, j) << ' ';

		os << "\n";
	}

	os << "];\n";
}

/*!
	Prints the vector in ASCII. If a label is given, the matrix is printed
	as a variable assignment that can be read by Malab.

	The optional numLabel creates a label of the form 'label_num', so that
	the assignemt becomes: 
	
    label_num = [...];
*/
template<class T> 
void PrintVector(vcl_ostream& os, const vnl_vector<T>& v, 
				 const char* label = NULL, int numLabel = -1)
{
	if (label)
		os << label;

	if (numLabel >= 0)
		os << "_" << numLabel;
		
	os << " = [ ";

	for (unsigned i = 0; i < v.size(); ++i)
		os << v(i) << ' ';

	os << "];\n";
}

/*!
	Prints the vector in ASCII. If a label is given, the vector is printed
	as a variable assignment that can be read by Malab.

	The optional numLabel creates a label of the form 'label_num', so that
	the assignemt becomes: 
	
    label_num = [...];
*/
template<class T> 
void PrintVector(vcl_ostream& os, const std::vector<T>& v, 
				 const char* label = NULL, int numLabel = -1)
{
	if (label)
		os << label;
		
	if (numLabel >= 0)
		os << "_" << numLabel;

	os << " = [ ";

	for (unsigned i = 0; i < v.size(); ++i)
		os << v[i] << ' ';

	os << "];\n";
}

/*!
	Prints the scalar in ASCII. If a label is given, the scalar is printed
	as a variable assignment that can be read by Malab.

	The optional numLabel creates a label of the form 'label_num', so that
	the assignemt becomes: 
	
    label_num = x;
*/
template<class T> 
void PrintScalar(vcl_ostream& os, const T& x, const char* label = NULL, 
				 int numLabel = -1)
{
	if (label)
		os << label;
		
	if (numLabel >= 0)
		os << "_" << numLabel;

	os << " = " << x << ";\n";
}


