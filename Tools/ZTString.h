/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <stdio.h>
#include <string>

class ZTString
{
	char* m_szBuffer;
	unsigned m_bufferSize;
	
public:
	ZTString()
	{
		m_szBuffer = NULL;
		m_bufferSize = 0;
	}

	/*!
		
	*/
	ZTString(const char* str)
	{
		m_bufferSize = strlen(str) + 1;
		m_szBuffer = new char[m_bufferSize];

		strcpy_s(m_szBuffer, m_bufferSize, str);
	}

	/*!
		
	*/
	ZTString(const std::string& str)
	{
		m_bufferSize = str.size() + 1;
		m_szBuffer = new char[m_bufferSize];

		strcpy_s(m_szBuffer, m_bufferSize, str.c_str());
	}

	ZTString(const ZTString& rhs)
	{
		m_bufferSize = rhs.size();
		m_szBuffer = new char[m_bufferSize];

		memcpy_s(m_szBuffer, m_bufferSize, rhs.m_szBuffer, m_bufferSize);
	}

	~ZTString()
	{
		delete[] m_szBuffer;
	}

	void operator=(const std::string& rhs)
	{
		operator=(ZTString(rhs.c_str()));
	}

	void operator=(const ZTString& rhs)
	{
		delete[] m_szBuffer;

		m_bufferSize = rhs.size();
		m_szBuffer = new char[m_bufferSize];

		memcpy_s(m_szBuffer, m_bufferSize, rhs.m_szBuffer, m_bufferSize);
	}

	//! Number of characters including the terminating null character
	unsigned size() const
	{
		return m_bufferSize;
	}

	const char* c_str() const
	{
		return m_szBuffer;
	}

	//! Replaces a character in the string (even the terminating null character)
	void replace(char oldChar, char newChar)
	{
		for (unsigned i = 0; i < m_bufferSize; ++i)
			if (m_szBuffer[i] == oldChar)
				m_szBuffer[i] = newChar;
	}
};

