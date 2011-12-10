
#include <assert.h>
#include <string.h>

#include "basics/auxiliaries.h"
#include "pc_aux.h"


using std::vector;


void bzero(void * buffer, vint8 bytes)
{
	vint8 vint8_bytes = bytes / sizeof(vint8);
	vint8 left_overs = bytes % sizeof(vint8);
	vint8 * vint8_buffer = (vint8 *) buffer;
	vint8 i;
	for (i = 0; i < vint8_bytes; i++)
		vint8_buffer[i] = 0;
	char * char_buffer = (char *) &(vint8_buffer[vint8_bytes]);
	for (i = 0; i < left_overs; i++)
		char_buffer[i] = 0;
}


//int strcasecmp(const char * string1, const char * string2)
//{
//	return _stricmp(string1, string2);
//}


char * vSimpleNamePC(char * filename)
{
	return vSimpleName(filename, "\\");
}


void vSimpleNamePC(char * filename, char * simple_name)
{
	char * temp_name = vSimpleNamePC(filename);
	strcpy(simple_name, temp_name);
	vdelete2(temp_name);
}


