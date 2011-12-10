#ifndef VASSILIS_PC_AUX_H
#define VASSILIS_PC_AUX_H

#include <string.h>

void bzero(void * buffer, vint8 bytes);

#ifndef strcasecmp
#define strcasecmp _stricmp
#endif

char * vSimpleNamePC(char * filename);
void vSimpleNamePC(char * filename, char * buffer);

#endif // VASSILIS_PC_AUX_H
