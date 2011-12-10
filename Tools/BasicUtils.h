/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <assert.h>
#include "Feedback.h"
#include "Timer.h"

#ifdef WIN32
#include <float.h>
#define isnan  _isnan
#define finite _finite
#define NORM2(X, Y) sqrt((X)*(X) + (Y)*(Y))

#ifndef strcasecmp
#define strcasecmp(A, B) _stricmp(A, B)
#endif

#define FILE_SEP '\\'
#else
#define NORM2(X, Y) std::sqrt((X)*(X) + (Y)*(Y))
#define FILE_SEP '/'
#endif //WIN32

#ifndef INFINITY
#define INFINITY	1000000
#endif

#ifndef MAX_PATH_SIZE
#define MAX_PATH_SIZE 260 // ie, MAX_PATH in Windows.h
#endif

#define ROUND_NUM(X) ( ((X) < 0) ? -(int)(0.5 - (X)) : (int)((X) + 0.5) )

#ifndef MIN
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b)  (((a) > (b)) ? (a) : (b))
#endif

#define SIGN(X) ( ( (X) == 0 ) ? 0 : ( ( (X) > 0 ) ? 1 : -1) )
#define AVG(X, Y) ((X + Y) / 2.0)
#define IFIN(X, Y, Z, A, B) ((X >= Y && X <= Z) ? (A) : (B))

//! Safe acos() function
#define SafeArcCosine(X) ((X > 1) ? acos(1.0) : ((X < -1) ? acos(-1.0) : acos(X)))

// Simple printing functions
#define PRINT(OS, EXP)	OS << #EXP << ": " << EXP << ", "
#define PRINTN(OS, EXP)	OS << #EXP << ": " << EXP << std::endl
#define PRINT2(OS, LBL, EXP) OS << LBL << ": " << EXP << ", "
#define PRINT_OPEN(OS, EXP)	OS << '[' << #EXP << ": " << EXP << ", "
#define PRINT_CLOSE(OS, EXP) OS << #EXP << ": " << EXP << ']' << std::endl

// XML printing functions
#define XML_PRINT(OS, LBL, EXP) OS << "<" << LBL << ">" << EXP << "</" << LBL << ">\n"
#define XML_OPEN(OS, LBL) OS << "<" << LBL << ">\n"
#define XML_CLOSE(OS, LBL) OS << "</" << LBL << ">\n"
#define XML_OPEN_ATTS(OS, LBL) OS << "<" << LBL
#define XML_PRINT_ATT(OS, A) OS << " " << #A << "=\"" << A << "\""
#define XML_PRINT_ATT2(OS, A, V) OS << " " << A << "=\"" << V << "\""
#define XML_END_ATTS(OS) OS << ">\n"
#define XML_CLOSE_ATTS(OS) OS << "/>\n"

#define PRINT_LINE  { std::cerr << std::endl << "LINE " << __LINE__ \
	<< " at " << __FILE__ << std::endl; }

// Ensure that COMPILE_DBG_STATEMENTS is defined in debug mode
#ifndef COMPILE_DBG_STATEMENTS
#ifdef _DEBUG
	#define COMPILE_DBG_STATEMENTS
#endif // _DEBUG
#endif // COMPILE_DBG_STATEMENTS

// The message of the ASSERT macro needs special care
#ifdef _DEBUG
	#define ASSERT_MSG(X) _ASSERTE(X)
#else // _DEBUG
	#define ASSERT_MSG(X) \
			std::cerr << "ERROR: Assertion failed \"" << #X \
				<< "\" in " __FILE__ << ':' << __LINE__ << std::endl
#endif

// The action of the ASSERT macro needs special care
#ifdef WIN32
	#define ASSERT_ACTION(X) throw 1
#else // WIN32
	#define ASSERT_ACTION(X) assert(false)
#endif // WIN32

// Debug Functions
#ifdef COMPILE_DBG_STATEMENTS

#define DBG_ONLY(C) C;

#define DBG_PRN_VAL(A) #A << " = " << (A) << ", "
#define DBG_PRN_LINE(A) std::cerr << A << std::endl;

#define DBG_PRINT1(A)             { DBG_PRN_LINE( DBG_PRN_VAL(A) ) }
#define DBG_PRINT2(A,B)           { DBG_PRN_LINE( DBG_PRN_VAL(A) << DBG_PRN_VAL(B) ) }
#define DBG_PRINT3(A,B,C)         { DBG_PRN_LINE( DBG_PRN_VAL(A) << DBG_PRN_VAL(B) << DBG_PRN_VAL(C) ) }
#define DBG_PRINT4(A,B,C,D)       { DBG_PRN_LINE( DBG_PRN_VAL(A) << DBG_PRN_VAL(B) << DBG_PRN_VAL(C) << DBG_PRN_VAL(D) ) }
#define DBG_PRINT5(A,B,C,D,E)     { DBG_PRN_LINE( DBG_PRN_VAL(A) << DBG_PRN_VAL(B) << DBG_PRN_VAL(C) << DBG_PRN_VAL(D) << DBG_PRN_VAL(E) ) }
#define DBG_PRINT6(A,B,C,D,E,F)   { DBG_PRN_LINE( DBG_PRN_VAL(A) << DBG_PRN_VAL(B) << DBG_PRN_VAL(C) << DBG_PRN_VAL(D) << DBG_PRN_VAL(E) << DBG_PRN_VAL(F) ) }
#define DBG_PRINT7(A,B,C,D,E,F,G) { DBG_PRN_LINE( DBG_PRN_VAL(A) << DBG_PRN_VAL(B) << DBG_PRN_VAL(C) << DBG_PRN_VAL(D) << DBG_PRN_VAL(E) << DBG_PRN_VAL(F) << DBG_PRN_VAL(G) ) }

#define DBG_NEWLINE { std::cerr << std::endl; }

#define DBG_PRN_LINE_INFO(A) std::cerr << "\n" << __FILE__ << "/" << __LINE__ << ": " << A << std::endl;

#define DBG_MSG1(A)              { DBG_PRN_LINE_INFO(A) }
#define DBG_MSG2(A,B)            { DBG_PRN_LINE_INFO(A << ' ' << B) }
#define DBG_MSG3(A,B,C)          { DBG_PRN_LINE_INFO(A << ' ' << B << ' ' << C) }
#define DBG_MSG4(A,B,C,D)        { DBG_PRN_LINE_INFO(A << ' ' << B << ' ' << C << ' ' << D) }
#define DBG_MSG5(A,B,C,D,E)      { DBG_PRN_LINE_INFO(A << ' ' << B << ' ' << C << ' ' << D << ' ' << E) }
#define DBG_MSG6(A,B,C,D,E,F)    { DBG_PRN_LINE_INFO(A << ' ' << B << ' ' << C << ' ' << D << ' ' << E << ' ' << F) }
#define DBG_MSG7(A,B,C,D,E,F,G)  { DBG_PRN_LINE_INFO(A << ' ' << B << ' ' << C << ' ' << D << ' ' << E << ' ' << F << ' ' << G) }

#define DBG_VAL(A) #A << " = " << (A) << ", "

#define DBG_STREAM(A) std::cerr << A << std::endl;

#define DBG_FLUSH(A) std::cerr << A << std::flush;

//! Parameters are LF: LogFile object, A: 'X' followed by any number of '<< X'. e.g., 'X << X << X'
#define DBG_LOG(LF, A) if (LF.IsActive()) { if (!LF.IsOpen()) { LF.OpenFile(); } LF << A << std::endl; }

//! Statement A is logged only if condition C is true
#define DBG_LOG_IF(LF, C, A) if (C) { DBG_LOG(LF, A) }

//! A is executed only if debug log is active
#define DBG_LOG_ONLY(LF, A) if (LF.IsActive()) \
  { if (!LF.IsOpen()) { LF.OpenFile(); } A; }

//! Calls funtion F with parameters (LF, P1)
#define DBG_LOG_CALL1(F, LF, P1) if (LF.IsActive()) \
  { if (!LF.IsOpen()) { LF.OpenFile(); } F(LF, P1); LF << std::endl; }

//! Calls funtion F with parameters (LF, P1, P2)
#define DBG_LOG_CALL2(F, LF, P1, P2) if (LF.IsActive()) \
  { if (!LF.IsOpen()) { LF.OpenFile(); } F(LF, P1, P2); LF << std::endl; }

//! Calls funtion F with parameters (LF, P1, P2, P3)
#define DBG_LOG_CALL3(F, LF, P1, P2, P3) if (LF.IsActive()) \
  { if (!LF.IsOpen()) { LF.OpenFile(); } F(LF, P1, P2, P3); LF << std::endl; }

#define DBG_PRINT_IF(A,B)  { if (B) DBG_PRN_LINE( DBG_PRN_VAL(A) ) }
#define DBG_MSG_IF(A,B)    { if (B) DBG_PRN_LINE_INFO(A) }
#define DBG_STREAM_IF(A,B) { if (B) DBG_STREAM(A) }

#define DBG_SHOW(A) { std::cerr << std::endl << __FILE__ << "/" << __LINE__ \
	<< ':' << #A << " = " << (A) << std::endl; }
	
#define DBG_LINE  { std::cerr << std::endl << "DBG_LINE " << __LINE__ \
	<< " at " << __FILE__ << std::endl; }

#define ASSERT(X) if (!(X)) {ASSERT_MSG(X); ASSERT_ACTION(X);}

#define WARNING(X, M) \
	if(X) { \
		std::cerr << "WARNING! " << M << ": \"" << #X << "\" in " __FILE__ \
			<< ':' << __LINE__ << std::endl; \
	}

#define WARNING1(X, M, A) \
	if(X) { \
		std::cerr << "WARNING! " << M << ": \"" << #X \
			<< " with " << #A << " = " << A \
			<< "\" in " __FILE__ << ':' << __LINE__ << std::endl; \
	}

#define WARNING2(X, M, A, B) \
	if(X) { \
		std::cerr << "WARNING! " << M << ": \"" << #X \
			<< " with " << #A << " = " << A << " and " << #B << " = " << B \
			<< "\" in " __FILE__ << ':' << __LINE__ << std::endl; \
	}
	
#define ASSERT_VALID_NUM(A) ASSERT(!isnan(A) && finite(A))
#define ASSERT_VALID_POINT(P) ASSERT(!isnan(P.x) && finite(P.x) && !isnan(P.y) && finite(P.y))
#define ASSERT_UNIT_INTERVAL(A) ASSERT(A >= 0 && A <= 1)
#define ASSERT_VALID_DENOMINATOR(A) ASSERT(A != 0 && !isnan(A) && finite(A))

#ifdef WIN32
	#ifdef _CRTDBG_MAP_ALLOC
			#define MYDEBUG_NEW   new(_NORMAL_BLOCK, __FILE__, __LINE__)
			#define new MYDEBUG_NEW
	#endif
#endif

// Timing functions
#define DBG_DECLARE_TIMER(name) Timer name;

#define DBG_RESET_TIMER(name) if (vpl::Timer::s_enableTimers) name.Reset();

//#define DBG_ELAPSED_TIME(name) name.ElapsedTime();

#define DBG_PRINT_ELAPSED_TIME(name, msg) if (vpl::Timer::s_enableTimers) \
{ \
	double t = name.ElapsedTime(); \
	StreamStatus("\n" << msg << ": elapsed time " << t << " milliseconds"); \
}

#define DBG_PRINT_ELAPSED_TIME_IF_GREATER_THAN(name, T, msg) if (vpl::Timer::s_enableTimers) \
{ \
	double t = name.ElapsedTime(); \
	if (t > T) \
		StreamStatus("\n" << msg << ": elapsed time " << t << " milliseconds"); \
}


#else //COMPILE_DBG_STATEMENTS
	#define DBG_PRN_VAL(A)
	#define DBG_PRN_LINE(A)

	#define DBG_PRINT1(A)
	#define DBG_PRINT2(A,B)
	#define DBG_PRINT3(A,B,C)
	#define DBG_PRINT4(A,B,C,D)
	#define DBG_PRINT5(A,B,C,D,E)
	#define DBG_PRINT6(A,B,C,D,E,F)
	#define DBG_PRINT7(A,B,C,D,E,F,G)
	
	#define DBG_PRINT_NEWLINE

	#define DBG_MSG1(A)
	#define DBG_MSG2(A,B)
	#define DBG_MSG3(A,B,C)
	#define DBG_MSG4(A,B,C,D)
	#define DBG_MSG5(A,B,C,D,E)
	#define DBG_MSG6(A,B,C,D,E,F)
	#define DBG_MSG7(A,B,C,D,E,F,G)

	#define DBG_VAL(A)
	#define DBG_STREAM(A)
	#define DBG_FLUSH(A)

	#define DBG_LOG(LF, A)
	#define DBG_LOG_IF(LF, C, A)
	#define DBG_LOG_ONLY(LF, A)
	#define DBG_LOG_CALL1(F, LF, P1)
	#define DBG_LOG_CALL2(F, LF, P1, P2)
	#define DBG_LOG_CALL3(F, LF, P1, P2, P3)

	#define DBG_PRINT_IF(A,B)
	#define DBG_MSG_IF(A,B)
	#define DBG_STREAM_IF(A,B)
	#define DBG_SHOW(A)
	#define DBG_LINE 
	#define DBG_ONLY(C)
	#define ASSERT(X)
	#define WARNING(X, M)
	#define WARNING1(X, M, A)
	#define WARNING2(X, M, A, B)
	#define ASSERT_VALID_NUM(A)
	#define ASSERT_VALID_POINT(P)
	#define ASSERT_UNIT_INTERVAL(A)

	#define DBG_DECLARE_TIMER(name)
	#define DBG_RESET_TIMER(name)
	//#define DBG_ELAPSED_TIME(name)
	#define DBG_PRINT_ELAPSED_TIME(name, msg)
	#define DBG_PRINT_ELAPSED_TIME_IF_GREATER_THAN(name, T, msg)
#endif //_DEBUG

