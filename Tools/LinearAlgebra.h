/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <vnl/vnl_math.h>
#include <vnl/algo/vnl_svd.h>
#include <vnl/algo/vnl_matrix_inverse.h>

#include "MatrixUtils.h"

/*!
	List of useful global functions that work with this matrix class:

	element_product<T>(a, b)
	vnl_matrix_inverse<T>(a);

	List of useful global class that are constructed from a matrix.

	vnl_svd<T> m(a) // computes svd of 'a' and stores the results in object 'm'
*/
template <class T> class VPLMatrix : public vnl_matrix<T>
{
public:
	typedef vnl_matrix<T> BASE;

protected:
	struct CellRef : protected std::vector<T*>
	{
		CellRef(const BASE& m)
		{
			reserve(m.size());
		}

		//! Move contructor
		CellRef(CellRef&& rhs) : std::vector<T*>(rhs)
		{
			// nothing else to do
		}

		void push_back(T* p)
		{
			push_back(p);
		}

		//! Assigns a scalar
		void operator=(const T& rhs)
		{
			forall_cells1(*this)
				**it0 = rhs;
		}

		//! Assigns the actual values referenced by the rhs CellRef
		void operator=(const CellRef& rhs)
		{
			forall_cells2(*this, rhs)
				**it0 = **it1;
		}

		//! Assigns a vector
		void operator=(const vnl_vector<T>& rhs)
		{
			forall_cells2(*this, rhs)
				**it0 = *it1;
		}

		//! Assigns a matrix
		void operator=(const vnl_matrix<T>& rhs)
		{
			forall_cells2(*this, rhs)
				**it0 = *it1;
		}
		
		/* //! Assigns any container of a convertible type
		template <typename C> void operator=(const C& rhs)
		{
			forall_cells2(*this, rhs)
				**it0 = *it1;
		}*/
	};

public:
	//! Default constructor creates an empty matrix of size 0,0.
	VPLMatrix() { }

	//! Construct a matrix of size 1 row by 1 column and sets element (0,0) to 1
	VPLMatrix(T const& v0) : BASE(1, 1, v0) { }

	//! Construct a matrix of size r rows by c columns
	VPLMatrix(unsigned r, unsigned c) : BASE(r, c) { }

	//! Construct a matrix of size r rows by c columns, and all emelemnts equal to v0
	VPLMatrix(unsigned r, unsigned c, T const& v0) : BASE(r, c, v0) { }

	//! Construct a matrix of size r rows by c columns, with a special type
	VPLMatrix(unsigned r, unsigned c, vnl_matrix_type t) : BASE(r, c, t) { }

	//! Construct a matrix of size r rows by c columns, initialised by an automatic array
	// The first n elements, are initialised row-wise, to values.
	VPLMatrix(unsigned r, unsigned c, unsigned n, T const values[]) : BASE(r, c, n, values) { }

	//! Construct a matrix of size r rows by c columns, initialised by a memory block
	// The values are initialise row wise from the data.
	VPLMatrix(T const* data_block, unsigned r, unsigned c) : BASE(data_block, r, c) { }

	//! Copy construct a matrix
	VPLMatrix(vnl_matrix<T> const& m) : BASE(m) { }

	void operator=(const vnl_matrix<T>& rhs)
	{
		BASE::operator=(rhs);
	}

	VPLMatrix& operator=(const VPLMatrix<T>& rhs)
	{
		BASE::operator=(rhs);

		return *this;
	}

	void make_scalar(const T& v0)
	{
		set_size(1, 1);
		operator()(0, 0) = v0;
	}

	//! Reference to an actual row in the matrix
	RowRef<T> row_ref(unsigned i)
	{
		return RowRef<T>(operator[](i), cols());
	}

	//! Extract the sub-matrix m(r0:rN, c0:cN)
	VPLMatrix<T> Sub(unsigned r0, unsigned rN, unsigned c0, unsigned cN) const
	{
		// Extract a sub-matrix of size r x c, starting at (top,left)
		return extract(rN - r0 + 1, cN - c0 + 1, r0, c0);
	}

	//! Extract the sub-matrix m(r0:rN, 0, 0)
	VPLMatrix<T> Sub(unsigned r0, unsigned rN) const
	{
		return Sub(r0, rN, 0, 0);
	}

	VPLMatrix<T> SubRow(unsigned i) const
	{
		return Sub(i, i, 0, cols() - 1);
	}

	VPLMatrix<T> SubCol(unsigned i) const
	{
		return Sub(0, rows() - 1, i, i);
	}

	/*! 
		Extract a sub-matrix starting at (top,left)
  
		The output is stored in a sub_matrix, and it should have the
		required size on entry.  Thus the result will contain elements
		[top,top+sub_matrix.rows()-1][left,left+sub_matrix.cols()-1]
	*/
	void Sub(VPLMatrix<T>& sub_matrix, unsigned top = 0, unsigned left = 0) const
	{
		extract(sub_matrix, top, left);
	}
	
	/*!
		Computes the min function along the row dimension. It only
		considers rows in the interval [firstRow, firstRow + numRows].

		The result is a vector of size equal to the number 
		of columns.

		Matlab function: v = min(m(firstRow:firstRow+numRows, :), [], 1)
	*/
	void RowMin(vnl_vector<T>* v, unsigned firstRow, unsigned numRows) const
	{
		unsigned r, c;

		*v = get_row(firstRow);

		for (r = firstRow + 1; r < numRows; ++r)
			for (c = 0; c < cols(); ++c)
				if (operator()(r, c) < (*v)(c))
					(*v)(c) = operator()(r, c);
	}

	/*!
		Computes the min function along the row dimension.
		The result is a vector of size equal to the number 
		of columns.

		Matlab function: v = min(m, [], 1)
	*/
	void RowMin(vnl_vector<T>* v) const
	{
		RowMin(v, 0, rows());
	}

	/*!
		Computes the min function along the column dimension. It only
		considers columns in the interval [firstCol, firstCol + numCol].

		The result is a vector of size equal to the number 
		of rows.

		Matlab function: v = min(m(:,firstCol:firstCol+numCols), [], 2)
	*/
	void ColMin(vnl_vector<T>* v, unsigned firstCol, unsigned numCols) const
	{
		unsigned r, c;

		*v = get_column(firstCol);

		for (c = firstCol + 1; c < numCols; ++c)
			for (r = 0; r < rows(); ++r)
				if (operator()(r, c) < (*v)(r))
					(*v)(r) = operator()(r, c);
	}

	/*!
		Computes the min function along the column dimension.
		The result is a vector of size equal to the number 
		of rows.

		Matlab function: v = min(m, [], 2)
	*/
	void ColMin(vnl_vector<T>* v) const
	{
		ColMin(v, 0, cols());
	}

	double MeanAlongDiagonal() const
	{
		unsigned i;
		double n;

		for (i = 0, n = 0; i < rows() && i < cols(); ++i)
			n += operator()(i, i);

		return n / i;
	}

	/*!
		Takes two matrices of vectors and calculates the
 		squared Euclidean distance between them.
		
		Both matrices must be of the same column dimension. If X has 
		M rows and N columns, and C has	L rows and N columns, then 
		the result has M rows and L columns.  The I, Jth entry is the
		squared distance from the Ith row of X to the Jth row of C.
	*/
	void AssignRowSqDistances(const vnl_matrix<T>& m1, const vnl_matrix<T>& m2)
	{
		unsigned i, j, c;
		double diff;

		CHECK_COLS(m1, m2);

		set_size(m1.rows(), m2.rows());

		for (i = 0; i < m1.rows(); ++i)
		{
			for (j = 0; j < m2.rows(); ++j)
			{
				T& val = operator()(i, j);

				for (val = 0, c = 0; c < m1.cols(); ++c)
				{
					diff = m1(i, c) - m2(j, c);
					val += diff * diff;
				}
			}
		}
	}

	//! Returns true if teh matrix contains NaNs
	bool HasNaN() const
	{
		for (unsigned i = 0; i < rows(); ++i)
			for (unsigned j = 0; j < cols(); ++j)
				if (vnl_math_isnan(operator()(i, j)))
					return true;

		return false;
	}

	//! Replace every NaN by 'val'
	void ReplaceAllNaN(const double& val)
	{
		for (unsigned i = 0; i < rows(); ++i)
			for (unsigned j = 0; j < cols(); ++j)
				if (vnl_math_isnan(operator()(i, j)))
					operator()(i, j) = val;
	}

	/*!
		Computes M = [m1, m2];
	*/
	void Set1x2(const vnl_matrix<T>& m1, const vnl_matrix<T>& m2)
	{
		CHECK_ROWS(m1, m2);

		set_size(m1.rows(), m1.cols() + m2.cols());

		update(m1, 0, 0);
		update(m2, 0, m1.cols());
	}

	/*!
		Computes M = [m1; m2];
	*/
	void Set2x1(const vnl_matrix<T>& m1, const vnl_matrix<T>& m2)
	{
		CHECK_COLS(m1, m2);

		set_size(m1.rows() + m2.rows(), m1.cols());

		update(m1, 0, 0);
		update(m2, m1.rows(), 0);
	}

	/*!
		Computes M = [m1, m2; m3, m4];
	*/
	void Set2x2(const vnl_matrix<T>& m1, const vnl_matrix<T>& m2, 
		const vnl_matrix<T>& m3, const vnl_matrix<T>& m4)
	{
		CHECK_ROWS(m1, m2);
		CHECK_ROWS(m3, m4);

		CHECK_COLS(m1, m3);
		CHECK_COLS(m2, m4);

		set_size(m1.rows() + m3.rows(), m1.cols() + m2.cols());

		update(m1, 0, 0);
		update(m2, 0, m1.cols());
		update(m3, m1.rows(), 0);
		update(m4, m1.rows(), m1.cols());
	}

	//! Returns the size of the smallest dimension, i.e., sz = min(rows,cols)
	unsigned min_size() const
	{
		return ((rows() < cols()) ? rows() : cols());
	}

	T sum() const
	{
		unsigned i, j;
		T n = 0;

		for (i = 0; i < rows(); i++)
			for (j = 0; j < cols(); j++)
				n += operator()(i, j);

		return n;
	}

	//! Binary save vnl_matrix to stream
	void Serialize(OutputStream& os) const
	{
		vsl_b_ostream bos(&os);

		vsl_b_write(bos, *this);
	}

	//! Binary load vnl_matrix from stream
	void Deserialize(InputStream& is) 
	{
		vsl_b_istream bis(&is);

		vsl_b_read(bis, *this);
	}

	// Redefine operators here

	//! Returns pointer to given row
	T* operator[](unsigned r) 
	{
		ASSERT(r < rows());

		return BASE::operator[](r); 
	}

	//! Returns pointer to given row
	T const* operator[](unsigned r) const 
	{ 
		ASSERT(r < rows());

		return BASE::operator[](r); 
	}

	//! Access an element for reading or writing
	T& operator()(unsigned r, unsigned c) 
	{
		ASSERT(r < rows() && c < cols());

		return BASE::operator()(r, c); 
	}

	//! Access an element for reading
	T const& operator()(unsigned r, unsigned c) const 
	{
		ASSERT(r < rows() && c < cols());

		return BASE::operator()(r, c); 
	}

	//! Access multiple elements for reading or writing
	CellRef operator()(const vnl_matrix<bool>& m) 
	{
		CHECK_SIZE(*this, m);

		CellRef cr(m);

		unsigned i, j;

		for (i = 0; i < rows(); i++)
			for (j = 0; j < cols(); j++)
				if (m(i, j))
					cr.push_back(&operator()(i, j));

		return cr; 
	}
};

template <class T> class VPLVector : public vnl_vector<T>
{
public:
	typedef vnl_vector<T> BASE;

	//! Default constructor creates an empty matrix of size 0,0.
	VPLVector() { }

	//! Construct a matrix of size r rows by c columns
	VPLVector(unsigned n) : BASE(n) { }

	//! Construct a matrix of size r rows by c columns, and all emelemnts equal to v0
	VPLVector(unsigned n, T const& v0) : BASE(n, v0) { }

	//! Construct a matrix of size r rows by c columns, with a special type
	VPLVector(unsigned n, vnl_matrix_type t) : BASE(n, t) { }

	//! Construct a matrix of size r rows by c columns, initialised by an automatic array
	// The first n elements, are initialised row-wise, to values.
	VPLVector(unsigned n, unsigned nvals, T const values[]) : BASE(n, nvals, values) { }

	//! Construct a matrix of size r rows by c columns, initialised by a memory block
	// The values are initialise row wise from the data.
	VPLVector(T const* data_block, unsigned n) : BASE(data_block, n) { }

	//! Copy construct a matrix
	VPLVector(vnl_vector<T> const& v) : BASE(v) { }

	void operator=(const vnl_vector<T>& rhs)
	{
		BASE::operator=(rhs);
	}

	VPLVector& operator=(const VPLVector<T>& rhs)
	{
		BASE::operator=(rhs);

		return *this;
	}

	VPLMatrix<T> reshape(unsigned r, unsigned c)
	{
		ASSERT(size() == r * c);

		VPLMatrix<T> m(r, c);

		for (unsigned i = 0; i < size(); i++)
			m(i / c, i % c) = get(i);

		return m;
	}
};

typedef VPLMatrix<bool> BoolMatrix;
typedef VPLMatrix<double> DoubleMatrix;
typedef VPLMatrix<int> IntMatrix;
typedef VPLMatrix<unsigned int> UIntMatrix;

typedef vnl_vector<bool> BoolVector;
typedef vnl_vector<double> DoubleVector;
typedef vnl_vector<int> IntVector;
typedef vnl_vector<unsigned int> UIntVector;

//! Defines the default Matrix type
typedef VPLMatrix<double> Matrix;

//! Defines the default array of matrices (note that std::vector has a move constructor)
typedef std::vector<Matrix> MatArray;

#define Zeros(m, n) Matrix(m, n, 0)
#define Ones(m, n) Matrix(m, n, 1)

//////////////////////////////////////////////////////////////////////////////
// Begin make functions
template <class T>
VPLMatrix<T> make1x1(const T& e0)
{
	return VPLMatrix<T>(1, 1, e0);
}

template <class T>
VPLMatrix<T> make1x2(const T& e0, const T& e1)
{
	VPLMatrix<T> m(1, 2);

	init_cells2(m, e0, e1);

	return m;
}

template <class T>
VPLMatrix<T> make2x1(const T& e0, const T& e1)
{
	VPLMatrix<T> m(2, 1);

	init_cells2(m, e0, e1);

	return m;
}

template <class T>
VPLMatrix<T> make1x3(const T& e0, const T& e1, const T& e2)
{
	VPLMatrix<T> m(1, 3);

	init_cells3(m, e0, e1, e2);

	return m;
}

template <class T>
VPLMatrix<T> make3x1(const T& e0, const T& e1, const T& e2)
{
	VPLMatrix<T> m(3, 1);

	init_cells3(m, e0, e1, e2);

	return m;
}
