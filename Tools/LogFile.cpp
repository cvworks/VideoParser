/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include <stdio.h> 
#include <stdarg.h>
#include "LogFile.h"

using namespace std;
using namespace vpl;

#ifdef WIN32
#define vsnprintf _vsnprintf
#endif

/*!
	@brief Prints the variable list of arguments only
	if the status of the log is ACTIVE. Otherwise, it does
	nothing.
*/
void LogFile::Print(const char* szFormat, ...)
{
	if (IsActive())
	{
		va_list args;

		va_start(args, szFormat);

		//vfprintf((FILE*)this->rdbuf()->fd(), szFormat, args);
		const int size = 512;
		char szBuff[size];

		vsnprintf(szBuff, size, szFormat, args);
	   
		va_end(args);

		if (!IsOpen())
			OpenFile();

		(*this) << szBuff;
	}
}
