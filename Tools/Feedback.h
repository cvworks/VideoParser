/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <iostream>
#include <string>

enum VERBOSE_LEVEL {VERBOSE_LEVEL_SILENT, VERBOSE_LEVEL_MINIMAL, 
	VERBOSE_LEVEL_MEDIUM, VERBOSE_LEVEL_ALL};

extern int g_verboseMode;

inline VERBOSE_LEVEL VerboseLevel()
{
	return (VERBOSE_LEVEL) g_verboseMode;
}

#define ShowStatus(M) if (g_verboseMode >= VERBOSE_LEVEL_MEDIUM) std::cout << std::string(M) << std::endl

#define ShowStatus1(M,A) if (g_verboseMode >= VERBOSE_LEVEL_MEDIUM) std::cout << std::string(M) << \
	" " << std::string(A) << std::endl

#define ShowStatus2(M,N,S) if (g_verboseMode >= VERBOSE_LEVEL_MEDIUM) std::cout << std::string(M) << " " \
	<< N << " " << std::string(S) << std::endl

#define StreamStatus(CMD) if (g_verboseMode >= VERBOSE_LEVEL_MEDIUM) std::cout << CMD << std::endl

#define ShowMsg(M) if (g_verboseMode >= VERBOSE_LEVEL_MINIMAL) std::cout << "\n" << std::string(M) << std::endl
#define StreamMsg(CMD) if (g_verboseMode >= VERBOSE_LEVEL_MINIMAL) std::cout << "\n" << CMD << std::endl

#define ShowUsage(M) std::cerr << "Usage: " << std::string(M) << std::endl

#define ShowError(M) std::cerr << "Error: " << std::string(M) << std::endl
#define ShowError1(M,A) std::cerr << "Error: " << std::string(M) \
	<< " " << std::string(A) << std::endl

#define StreamError(CMD) std::cerr << "Error: " << CMD << std::endl

#define ShowErrorAndNumber(M,N) std::cerr << "Error: " << std::string(M) \
	<< " " << N << std::endl

#define ShowOpenFileError(F) std::cerr << "Error: Cannot open file '" \
	<< std::string(F) << "'" << std::endl

#define ShowOpenFileError2(F, REASON) std::cerr << "Error: Cannot open file '" \
	<< std::string(F) << "' because " << REASON << std::endl

#define ShowCreateFileError(F) std::cerr << "Error: Cannot create file '" \
	<< std::string(F) << "'" << std::endl

