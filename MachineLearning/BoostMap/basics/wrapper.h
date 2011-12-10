#ifndef VASSILIS_WRAPPER_H
#define VASSILIS_WRAPPER_H

#include "vplatform.h"

#ifdef VASSILIS_SGI_PLATFORM
#include <vector.h>
#endif // VASSILIS_SGI_PLATFORM

#include <vector>
using std::vector;

#include "auxiliaries.h"


// The functions declared here are implemented in two separate files, each called 
// wrapper.cpp, but one in the sgi_platform directory and another in the pc_platform
// directory. They are functions that are implemented in platform-specific ways. This
// file is used in both platform, and the function declarations hide all
// platform-specific details, so that programs in both platforms can call them in the
// same way.


ushort vDirectoryFiles(const char *dir, std::vector<voidp> * filenames);

// This function requires that filenames is an empty vector when
// calling this function
ushort vFilesMatching(const char * pattern, std::vector<voidp> * filenames);

// This function allows for filenames to already contain some filenames,
// and it just adds more to the end of the vector
ushort vMoreFilesMatching(const char * pattern, 
                           std::vector<voidp> * filenames);

char * vFileName(const char * filename);

char * full_pathname(const char * filename);

ushort vMatchArguments(long argc, char ** argv, long start, 
                        std::vector<voidp> * filenames);

ushort function_seek(FILE * fp, vint8 offset, int origin);

vint8 function_tell(FILE * fp);

#endif // VASSILIS_WRAPPER_H
