#ifndef VASSILIS_DEFINITIONS_H
#define VASSILIS_DEFINITIONS_H

#include "vplatform.h"
#include "memory.h"

#ifdef VASSILIS_WINDOWS_MFC
#include "io_aux.h"


extern vConsoleStream * pc_console_stream;

#ifndef printf
#define printf vprint
#endif

#define scanf vscan
#define cin vInitializeConsole(); (*pc_console_stream)
#define cout vInitializeConsole(); (*pc_console_stream) 
#define cerr vInitializeConsole(); (*pc_console_stream) 
#define endl "\n"
#define vPrint vprint
#define vVprint vvprint
#define vScan vscan
#define vVScan vvscan
#define vGetLine vMFCGetLine
#define function_print vprint
#define function_question vscan

#else 
#define vPrint vPrintAndFlush
#define vVprint vprintf
#define vScan vScanAndVerify
#define vVScan vVScanAndVerify
#define vGetLine vShellGetLine
#define function_print vPrintAndFlush
#define function_question vScanAndVerify
#endif // VASSILIS_WINDOWS_MFC

// The following definitions apply in the case where we want to 
// check for correct memory usage.

#ifdef VASSILIS_MEMORY_CHECK
#define new vNewController() << ::new
#define vdelete(pointer) vDeleteAndSetToZero(pointer)
#define function_delete(pointer) vDeleteAndSetToZero(pointer)



// The following definitions apply in the case where we don't
// want to check for correct memory usage.
#else
#define vdelete(pointer) delete pointer
#define function_delete(pointer) delete pointer
#endif



#endif // VASSILIS_DEFINITIONS_H
