/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "BasicUtils.h"
#include "BasicTypes.h"

#include <fstream>

namespace vpl {

/*!
	The LogFile class specializes a fstream object for logging debug information
	It is meant to be used with the macro DBG_LOG(LogFile, a << b << c ...)

	The class also adds the Print() function with variable number of parameters.

	Note: the log file is creared automatically the first time that Print() or
	the macro DBG_LOG() are called as long as the log status variable is true.
	Do not call operator<<() before the file is created.
*/
class LogFile : public std::fstream
{
	const char* m_szFileName;
	bool* m_pLogStatus;

public:
	LogFile(bool* pLogStatus = NULL) 
	{ 
		m_pLogStatus = pLogStatus; 
		m_szFileName = NULL;
	}

	LogFile(const char* szFileName, bool* pLogStatus = NULL)
	{
		m_pLogStatus = pLogStatus;
		m_szFileName = szFileName;

		if (m_pLogStatus == NULL)
			OpenFile(); // open it right away. szFileName may be invalid later
	}

	void SetFileName(const char* szFileName)
	{
		m_szFileName = szFileName;
	}

	bool IsActive() const
	{
		return (m_pLogStatus == NULL || *m_pLogStatus);
	}

	bool IsOpen() const
	{
		return is_open();
	}

	void OpenFile()
	{
		if (!IsOpen())
			open(m_szFileName, std::ios::out | std::ios::trunc);
	}

	void Print(const char* szFormat, ...);
};

} // namespace vpl
