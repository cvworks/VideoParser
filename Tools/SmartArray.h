/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <memory>
#include <search.h>
#include "BasicUtils.h"

/*!
	\brief Wrapper class of an array of objects of class T. 
	It efficiently handles the copies of the array.

	This class wraps a pointer to an array of objects of class T, an keeps track
	of the number of references to it. If any non-constant function is called, 
	for an object with number of references grater than 1, 
	a copy of the array is performed 
	
	This essentially means that a copy of the array will be created only if there is
	a chance that a function is modifying the contents of a shared array. Therefore,
	whenever possible, the constant version of operator[]() must be called to avoid
	unnecessary copies of the array.
*/
template<class T> class SmartArray
{
	struct DATAREF {
		T* pData;
		int nLastItem;
		int nSize;
		int nRealSize;
		int nGrowFactor;
		int nLinks;
		
		DATAREF(int nGF)  { nLinks = 1; nGrowFactor = nGF; ASSERT(nGF > 0); }
	};

	DATAREF* ptr;

private:
	void Unlink();

protected:
	const T* GetData() const { return ptr->pData; }

public:

	SmartArray(int nSize = 0, int nGrowFactor = 1) { 
		ptr = new DATAREF(nGrowFactor); 
		ptr->pData = (nSize > 0) ? new T[nSize]:NULL;
		ptr->nLastItem = 0;
		ptr->nSize = nSize;
		ptr->nRealSize = nSize;
		ASSERT(ptr->nSize == 0 || ptr->pData != NULL);
	}

	SmartArray(const SmartArray& x)	{ 
		ptr = x.ptr;
		ptr->nLinks++;
	}

	~SmartArray() { 
		if (--ptr->nLinks == 0)
		{
			delete[] ptr->pData;
			delete ptr;
		}
	}	

	const T& operator[](int i) const { 
		ASSERT(i >= 0 && i < ptr->nSize);
		return ptr->pData[i]; 
	}

	operator const T*() const { return ptr->pData; }

	int GetSize() const  { return ptr->nSize; }
	int Size() const     { return ptr->nSize; }
	bool IsEmpty() const { return ptr->nSize == 0; }

	int GetTailIdx() const { return ptr->nLastItem - 1; }

	/*!
		Writes each element x of the array using os.write(&x, sizeof(x))
		It is faster than WriteRecursive().
	*/	
	std::ostream& Write(std::ostream& os) const
	{
		os.write((char*)&(ptr->nSize), sizeof(ptr->nSize));	

		if (ptr->nSize > 0)
			os.write((char*)(ptr->pData), sizeof(T) * ptr->nSize);

		return os;
	}

	/*!
		Calls Write(std::ostream) recursively for each element in the array.
		This function is useful for writting SmartArray< SmartArray<T> > objects.
		Note that the recursion occurs at one level only!
	*/	
	std::ostream& WriteRecursive(std::ostream& os) const
	{
		os.write((char*)&(ptr->nSize), sizeof(ptr->nSize));

		for (int i = 0; i < ptr->nSize; i++)
			operator[](i).Write(os);

		return os;
	}

	void Print(std::ostream& os = std::cout, const char* pSep = NULL) const
	{
		ASSERT(ptr->nSize >= 0);
		
		if (pSep == NULL)
			pSep = ", ";

		os << '[';
		for (int i = 0; i < ptr->nSize; i++) {
			os << operator[](i);
			if (i < ptr->nSize - 1)
				os << pSep;
		}
		os << ']';
	}

	bool operator==(const SmartArray& rhs) const
	{
		if (GetSize() != rhs.GetSize())
			return false;

		for (int i = 0; i < ptr->nSize; i++)
			if (operator[](i) != rhs.operator[](i))
				return false;
		
		return true;
	}

	bool operator!=(const SmartArray& rhs) const
	{
		return !operator==(rhs);
	}

	SmartArray Left(int nEnd) const
	{
		ASSERT(nEnd >= 0 && nEnd < GetSize());

		SmartArray l(nEnd + 1);

		for (int i = 0; i <= nEnd; i++)
			l[i] = operator[](i);
		
		return l;
	}

	SmartArray Right(int nStart) const
	{
		ASSERT(nStart >= 0 && nStart < GetSize());

		SmartArray r(GetSize() - nStart);

		for (int i = nStart; i < GetSize(); i++)
			r[i - nStart] = operator[](i);
		
		return r;
	}

	SmartArray Reverse() const
	{
		int nSize = GetSize();
		SmartArray r(nSize);

		for (int i = 0; i < nSize; i++)
			r[nSize - i - 1] = (*this)[i];

		return r;
	}
	
	T operator*(const SmartArray& rhs) const
	{
		ASSERT(GetSize() == rhs.GetSize());
		
		T n = 0;
		
		for (int i = 0; i < GetSize(); i++)
			n += operator[](i) * rhs[i];
			
		return n;
	}
	
	SmartArray operator-(const SmartArray& rhs) const
	{
		ASSERT(GetSize() == rhs.GetSize());
		SmartArray v(GetSize());
		
		for (int i = 0; i < GetSize(); i++)
			v[i] = operator[](i) - rhs[i];
			
		return v;
	}
	
	T Sum() const
	{
		T sum = 0;
		
		for (int i = 0; i < GetSize(); i++)
			sum += (*this)[i];
			
		return sum;
	}

	int GetGrowFactor() const
	{
		return ptr->nGrowFactor;
	}

// Non-const functions. All these functions must unlink the data if necessary.

	void SetGrowFactor(int nGrowFactor)
	{ 
		ASSERT(nGrowFactor > 0);

		if (ptr->nLinks > 1)
			Unlink();

		ptr->nGrowFactor = nGrowFactor;
	}

	T& operator[](int i) {
		ASSERT(i >= 0 && i < ptr->nSize);

		if (ptr->nLinks > 1)
			Unlink();

		return ptr->pData[i]; 
	}

	const T& GetTail() const { return operator[](GetSize() - 1); }
	const T& GetHead() const { return operator[](0); }

	SmartArray& operator=(const SmartArray& rhs) {

		rhs.ptr->nLinks++;	// protect against 'x = x'
		
		if (--ptr->nLinks == 0)
		{
			delete[] ptr->pData;
			delete ptr;
		}

		ptr = rhs.ptr;
		return *this;
	}

	void Sort(int (*Compare)(const void *elem1, const void *elem2 ) )
	{
		if (GetSize() > 0)
		{	  
		  if (ptr->nLinks > 1)
			Unlink();
		
		  qsort(ptr->pData, ptr->nSize, sizeof(T), Compare);
		}
	}
	
	//! Makes all elements equal to x
	void Set(const T& x)
	{
		for (int i = 0; i < GetSize(); i++)
			(*this)[i] = x;
	}

	//! Adds an element after the last element without increasing
	//! the size of the array.
	void Add(const T& x) { 
		ASSERT(ptr->nLastItem < ptr->nSize); 
		ptr->pData[ptr->nLastItem++] = x; // operator[]'ll handle the reference problem.
	}

	//! Adds an element after the last element. It increases
	//! the size of the array if necessary.
	void AddTail(const T& x) 
	{
		if (ptr->nLastItem >= ptr->nSize)
		{
			if (ptr->nLastItem < ptr->nRealSize)
			{
				if (ptr->nLinks > 1)
					Unlink();

				ptr->nSize++;
			}
			else
			{
				ASSERT(ptr->nSize == ptr->nRealSize);

				const SmartArray<T> a(*this);

				Resize(ptr->nRealSize + ptr->nGrowFactor);
				//memcpy(ptr->pData, a.GetData(), a.GetSize() * sizeof(T));

				for (int i = 0; i < a.GetSize(); i++)
					ptr->pData[i] = a.ptr->pData[i];

				ptr->nSize = a.GetSize() + 1;
				ptr->nLastItem = a.GetSize();
			}
		}

		Add(x);
	}
	
	void AddTail(const SmartArray<T>& tail)
	{
		const int nGF = GetGrowFactor();
		const int nFreeSlots = ptr->nRealSize - ptr->nSize;

		if (tail.GetSize() > nFreeSlots + nGF)
			SetGrowFactor(tail.GetSize() - nFreeSlots);

		for (int i = 0; i < tail.GetSize(); i++)
			AddTail(tail[i]);

		// Restore original growth factor
		SetGrowFactor(nGF);
	}

	void AddHead(const SmartArray<T>& head)
	{
		SmartArray<T> aux(head);

		aux.AddTail(*this);

		operator=(aux);
	}

	void Resize(int size, bool bInit = false)
	{
		ASSERT(size >= 0);

		if (size != ptr->nSize || ptr->nLinks > 1)
		{
			if (ptr->nLinks > 1) 
			{
				ptr->nLinks--;
				ptr = new DATAREF(ptr->nGrowFactor);
			}
			else if (ptr->pData) 
				delete[] ptr->pData;

			ptr->pData = (size > 0) ? new T[size]:NULL;
			ptr->nSize = size;
			ptr->nRealSize = size;
			ASSERT((size > 0 && ptr->pData != NULL) || size == 0);
		}

		ptr->nLastItem = 0;

		if (bInit && size > 0)
			memset(ptr->pData, 0, size * sizeof(T));
	}

	void Clear()
	{
		Resize(0);
	}

	/*!
		Reads each element x of the array using is.read(&x, sizeof(x))
		It is faster than ReadRecursive()
	*/
	std::istream& Read(std::istream& is)
	{
		int size = 0; // set to zero in case read fails
		is.read((char*)&size, sizeof(size));

		Resize(size);	// Resize() will handle the multiple references issue.

		if (size > 0)
		{
			is.read((char*)(ptr->pData), sizeof(T) * size);
			ptr->nLastItem = size;
		}

		return is;
	}
	
	/*!
		Calls Read(std::istream) recursively for each element of the array.
		This function is useful for reading SmartArray< SmartArray<T> > objects.
		Note that the recursion occurs at one level only!
	*/
	std::istream& ReadRecursive(std::istream& is)
	{
		int size = 0; // set to zero in case read fails
		is.read((char*)&size, sizeof(size));

		Resize(size);	// Resize() will handle the multiple references issue.

		if (size > 0)
		{
			for (int i = 0; i < size; i++)
				operator[](i).Read(is);

			ptr->nLastItem = size;
		}

		return is;
	}

	friend std::ostream& operator<<(std::ostream& os, const SmartArray<T>& x) { return x.Write(os); }
	friend std::istream& operator>>(std::istream& is, SmartArray<T>& x) { return x.Read(is); }
};

template<class T> void SmartArray<T>::Unlink()
{
	ASSERT(ptr->nLinks > 1);

	DATAREF* op = ptr;
	ptr = new DATAREF(op->nGrowFactor);

	ptr->pData = new T[op->nSize];
	ptr->nLastItem = op->nLastItem;
	ptr->nSize = op->nSize;
	ptr->nRealSize = op->nSize;

	//memcpy(ptr->pData, op->pData, op->nSize * sizeof(T));
	for (int i = 0; i < op->nSize; i++)
		ptr->pData[i] = op->pData[i];

	op->nLinks--;

	//WARNING(true, "possible not desired copy of data");
}


