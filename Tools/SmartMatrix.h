/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "SmartArray.h"

/*!
	\brief Wrapper class of an 2-D array of objects of class T. 
	It efficiently handles the copies of the array.

	This class wraps pointers to arrays of arrays of objects of class T,
	an keeps track of the number of references to them. If any non-constant 
	function is called, for an object with number of references grater than 1, 
	a copy of the array is performed 
	
	This essentially means that a copy of the array will be created only if there is
	a chance that a function is modifying the contents of a shared array. Therefore,
	whenever possible, the constant version of operator[]() must be called to avoid
	unnecessary copies of the array.
*/
template <class T> class SmartMatrix : public SmartArray< SmartArray<T> >
{
	typedef SmartArray< SmartArray<T> > base_class;

	/*! 
		@brief Privated function that should not be called.

		This function must never be called and so it is made private.
	*/
	void Resize(int size, bool bInit = false) 
	{ 
		SmartArray< SmartArray<T> >::Resize(size, bInit);
	}

public:
	void Resize(int rows, int cols, bool bInit = false)
	{
		Resize(rows, false); // never init this array!!!

		for (int i = 0; i < NRows(); i++)
			this->operator[](i).Resize(cols, bInit);
	}
	
	void Init(int rows, int cols, T* rhs)
	{
		Resize(rows, cols, false);
		SmartMatrix<T>& lhs = *this;
		int r, c;

		for (r = 0; r < NRows(); r++)
			for (c = 0; c < NCols(); c++)
				lhs[r][c] = rhs + (NCols() * c + r);
	}

	SmartMatrix<T>()
	{
	}

	SmartMatrix<T>(int rows, int cols, bool bInit = false)
	{
		Resize(rows, cols, bInit);
	}

	std::ostream& Write(std::ostream& os) const
	{
		int nSize = NRows();

		os.write((char*) &nSize, sizeof(nSize));	

		if (nSize > 0)
		  for (int i = 0; i < nSize; i++)
			this->operator[](i).Write(os);
			
		return os;
	}

	std::istream& Read(std::istream& is)
	{
		int size = 0; // set to zero in case read fails
		is.read((char*)&size, sizeof(size));

		Resize(size);

		if (size > 0)
		  for (int i = 0; i < NRows(); i++)
			this->operator[](i).Read(is);

		return is;
	}
	
	int NRows() const { return base_class::Size(); }
	int NCols() const { return (NRows() > 0) ? (*this)[0].Size():0; }

	double Size() const { return NRows() * NCols(); }

	double GetSize() const { return Size(); }

	//! Sums the value of all elements in row nRow
	T RowSum(int nRow) const
	{
		return this->operator[](nRow).Sum();
	}

	//! Sums the value of all elements in column nCol
	T ColSum(int nCol) const
	{
		T sum = 0;

		for (int i = 0; i < NRows(); i++)
			sum += this->operator[](i)[nCol];

		return sum;
	}
	
	const SmartMatrix& Transpose() const
	{
		SmartMatrix t;
		int i, j;
		
		t.Resize(NCols(), NRows(), false);
		
		for (i = 0; i < NRows(); i++)
			for (j = 0; j < NCols(); j++)
				t[j][i] = (*this)[i][j];
		
		return t;
	}
	
	SmartArray<T> operator*(const SmartArray<T>& v) const
	{
		ASSERT(NCols() == v.Size());
		
		SmartArray<T> r(v.Size());
		
		for (int i = 0; i < NRows(); i++)
			r[i] = (*this)[i] * v;
			
		return r;
	}
	
	/*SmartMatrix operator*(const SmartMatrix& rhs) const
	{
		ASSERT(NCols() == rhs.NRows());
		
		SmartMatrix t;
		SmartMatrix m;
		int i, j;
		
		m.Resize(NRows(), rhs.NCols(), false);
		t = rhs.Transpose();
		
		for (i = 0; i < NRows(); i++)
			for (j = 0; j < rhs.NCols(); j++)
				m[i][j] = (*this)[i] * t[j];
			
		return m;
	}*/
	
	SmartMatrix operator-(const SmartMatrix& rhs) const
	{
		ASSERT(NRows() == rhs.NRows() && NCols() == rhs.NCols());
		SmartMatrix m;
		
		m.Resize(NRows(), false);
		
		for (int i = 0; i < NRows(); i++)
			m[i] = (*this)[i] - rhs[i];
			
		return m;
	}
	
	void Print(std::ostream& os = std::cout, int width = 3, int precision = 6, bool bShowLabels = true) const
	{
		int i, j;
		
		int nOldPres = os.precision(precision); // useful when T is double
		
		os << "[\n";
		
		if (bShowLabels)
		{
			os.width(width);
			os << '#';
			
			for (int j = 0; j < NCols(); j++)
				os << j;
				
			os << "\n";
		}
		
		for (i = 0; i < NRows(); i++)
		{
			if (bShowLabels)
				os << i;
				
			for (j = 0; j < NCols(); j++)
				os << (*this)[i][j];
				
			os << "\n";
		}
			
		os << ']';
		
		os.precision(nOldPres); // back to default value
	}
};

