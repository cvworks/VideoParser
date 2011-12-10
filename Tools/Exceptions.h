/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <iostream>
#include <fstream>
#include <string>

class BasicException
{
public:
	std::string m_strText;
	const char* m_szFileName;
	int m_nLine;

	BasicException()
	{
		m_strText = "no detailed message given";
		m_nLine = 0;
		m_szFileName = "";
	}

	BasicException(std::string strText, const char* szFileName, int nLine)
	{
		m_strText = strText;
		m_szFileName = szFileName;
		m_nLine = nLine;
	}

	BasicException(const BasicException& e)
	{
		*this = e;
	}

	BasicException& operator=(const BasicException& e)
	{
		m_strText = e.m_strText;
		m_szFileName = e.m_szFileName;
		m_nLine = e.m_nLine;

		return *this;
	}

	void Print(std::ostream& os = std::cerr)
	{
#ifndef _DEBUG
		PrintSimple(os);
#else
		PrintFull(os);
#endif
	}

	void PrintSimple(std::ostream& os = std::cerr)
	{
		os << "\nError: " << m_strText << std::endl;
	}

	void PrintFull(std::ostream& os = std::cerr)
	{
		os << "\n\nException occurred at line " << m_nLine << " in " << m_szFileName 
			<< ": " << m_strText << std::endl << std::endl;
	}
};

#define THROW_BASIC_EXCEPTION(A) throw BasicException(A, __FILE__, __LINE__)


