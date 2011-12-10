#include <io.h>

#include "basics/v_types.h"
#include "basics/auxiliaries.h"

#undef _lseeki64

ushort function_seek(FILE * fp, vint8 offset, int origin)
{

  // This version works.
  if (origin != SEEK_SET) return 0;
	int error = fsetpos(fp, &offset);
	if (error == 0) return 1;
	else return 0;

	// This implementation didn't work, fseek just doesn't work if we 
  // are too deep in the file (more than 2GB from the origin.
//  long limit = 2000000000; // 2 billion
//  vint8 bytes_left = offset;
//  if (bytes_left < 0) limit = -limit;
//  int success = fseek(fp, 0, origin);
//  if (success != 0) return 0;

//  while(vAbs(bytes_left) >= vAbs(limit))
//  {
//    success= fseek(fp, limit, SEEK_CUR);
//    if (success != 0) return 0;
//    bytes_left = bytes_left - limit;
//  }
//  success = fseek(fp, bytes_left, SEEK_CUR);
//  if (success != 0) return 0;
//  return 1;

//  This is how I implemented the function at first, but it didn't
//  work, I couldn't figure out why.

//  int handle = _fileno(fp);
//  vint8 result = _lseeki64(handle, offset, origin);
//  if (result == -1) return 0;
//  else return 1;
}


// Returns -1 in case of failure
vint8 function_tell(FILE * fp)
{
  vint8 result; 
  int error = fgetpos(fp, &result);
  if (error != 0) return -1;
  else return result;
}
