#ifndef VASSILIS_PLATFORM_H
#define VASSILIS_PLATFORM_H

static const char * vFOPEN_READ = "rb";
static const char * Vassilis_read_file = "rb";
static const char * vFOPEN_WRITE = "wb";
static const char * Vassilis_save_file = "wb";
static char * platform_directory_slash = "\\";
static const char * MOVE_COMMAND = "move";
static const char * sequence_filter = "Video Sequences|\
*.avi;*.rle;*.ppm;*.vimseq1;*.vimseq3;*.bmp;*.ppm;*.pgm;*.aviset;*.medical;*.ima;*vid;*raw|\
AVI files|*.avi|RLE files|*.rle|vimseq files|*.vimseq1;*.vimseq3|\
BMP files|*.bmp|PPM files|*.ppm|PGM files|*.pgm|\
AVI set files|*.aviset|Medical sequences|*.medical|DICOM sequences|*.ima|VID files|*.vid|RAW files|*.raw|";
typedef __int64 vint8;

typedef char vint1;
typedef short vint2;
typedef int vint4;
typedef long long vint8;
typedef unsigned char vuint1;
typedef unsigned short vuint2;
typedef unsigned long vuint4;

// in the next line it doesn't matter if it is
// vector<long> or vector<whatever>, since the size type does not change
#include <vector>
typedef std::vector<long>::size_type vector_size;

/*
#ifdef VASSILIS_RELEASE_VERSION
//#define VASSILIS_USE_vArray
//#define VASSILIS_MEMORY_CHECK
#else
//#define VASSILIS_USE_vArray
//#define VASSILIS_MEMORY_CHECK
#endif // VASSILIS_RELEASE_VERSION
*/

#endif // VASSILIS_PLATFORM_H
