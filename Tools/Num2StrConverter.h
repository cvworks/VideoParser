/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <stdio.h>
#include <string>

class Num2StrConverter
{
	char* m_szBuffer;
	unsigned m_bufferSize;
	bool m_deleteBuffer;
	
public:
	/*!
		Constructs a converter with a buffer is size 'bufferSize'.
	*/
	Num2StrConverter(unsigned bufferSize)
	{
		m_szBuffer = new char[bufferSize];
		m_bufferSize = bufferSize;
		m_deleteBuffer = true;
	}

	/*!
		Constructs a converter using a specified buffer. If 'deleteBuffer' is
		true, "delete[] buffer" is called at destruction time. Otherwise,
		the memory is NOT deallocated (the default behaviour).
	*/
	Num2StrConverter(char* buffer, unsigned bufferSize, bool deleteBuffer = false)
	{
		m_szBuffer = buffer;
		m_bufferSize = bufferSize;
		m_deleteBuffer = deleteBuffer;
	}

	~Num2StrConverter()
	{
		if (m_deleteBuffer)
			delete[] m_szBuffer;
	}

	/*!
		Converts the number 'n' to a zero-terminated string representation and 
		stores it in the buffer. 

		@return a pointer to the buffer
	*/
	const char* toCharPtr(int n)
	{
		return toCharPtr("", n);
	}

	/*!
		Converts the number 'n' to a zero-terminated string representation and 
		stores it in the buffer. The prefix 'szPrefix' is copied before the
		number.

		@return a pointer to the buffer
	*/
	const char* toCharPtr(const char* szPrefix, int n)
	{
		unsigned int i, d, nn;
		
		for (i = 0; szPrefix[i] != '\0' && i < m_bufferSize; i++)
			m_szBuffer[i] = szPrefix[i];

		if (n >= 0)
		{
			d = 1;
			nn = n;
		}
		else
		{
			d = 2;
			nn = -n;
		}
			
		for (; nn > 9; d++)
			nn /= 10;

		if (i + d < m_bufferSize)
			sprintf(m_szBuffer + i, "%d", n);

		return m_szBuffer;
	}

	std::string toString(int n)
	{
		return toCharPtr(n);
	}

	std::string toString(const char* szPrefix, int n)
	{
		return toCharPtr(szPrefix, n);
	}
};

