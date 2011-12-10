#include "vplatform.h"

//#include <winsock.h>
#include "pc_aux.h"

#include <assert.h>
#include <string.h>

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

#include "auxiliaries.h"
#include "general_templates.h"
#include "wrapper.h"
#include "basics/definitions.h"

using namespace std;

unsigned short UniqueDebug = 0;

class_unique::class_unique()
{
	count = new long;
	*count = 1;
	if (UniqueDebug != 0)
		printf("New class_unique, count = %li, %lx\n", *count, (long) count);
}


class_unique::class_unique(class_unique const & unique)
{
	count = unique.count;
	(*count)++;
	if (UniqueDebug != 0)
		printf("Copy class_unique, count = %li, %lx\n", *count, (long) count);
}


class_unique::~class_unique()
{
	if (UniqueDebug != 0)
		printf("class_unique destructor, count = %lx\n", (long) count);
}


void class_unique::operator = (class_unique const & unique)
{
	remove_reference();
	count = unique.count;
	(*count)++;
	if (UniqueDebug != 0)
		printf("Assigning unique, count = %li, %lx\n", *count, (long) count);
}


void class_unique::remove_reference()
{
	if (count == 0) return;
	(*count)--;
	if (UniqueDebug != 0)
		printf("Checking unique's references count, count = %li, %lx\n", 
		*count, (long) count);
	if (*count == 0)
	{
		vdelete(count);
		if (UniqueDebug != 0)
			printf("Ready to call delete_unique\n");
		delete_unique();
	}

	count = 0;
	// The preceding line solves the following problem: Suppose a class
	// A inherits from class_unique and B inherits from A. Suppose we
	// want to delete B. Then, we have to call DecrementReference from
	// the destructor, so that the virtual DeleteReference will be
	// called while the object is still valid. However, we also have to
	// call remove_reference from the destructor of A, for objects
	// that are just A and not B. Here, the first call to
	// remove_reference sets count = 0, so that the next times we just return.
}


Label::Label()
{
	id = 0;
	count = 0;
	left = 999999999;
	right = 0;
	top = 999999999;
	bottom = 0;
}


ushort LabelMore::operator() (const Label & label1, const Label & label2)
{
	return (label1.count > label2.count);
}


ushort LabelBoxMore::operator() (const Label & label1, const Label & label2)
{
	vint8 height1 = label1.bottom - label1.top + 1;
	vint8 width1 = label1.right - label1.left + 1;
	vint8 size1 = height1 * width1;

	vint8 height2 = label2.bottom - label2.top + 1;
	vint8 width2 = label2.right - label2.left + 1;
	vint8 size2 = height2 * width2;

	return (size1 > size2);
}


ushort S_BoxesOverlapAux(vBoundingBox & b1, vBoundingBox & b2)
{
	ushort result = 0;
	if ((b1.top >= b2.top) && (b1.top <= b2.bottom) &&
		(b1.left >= b2.left) && (b1.left <= b2.right))
	{
		result = 1;
	}
	else if ((b1.top >= b2.top) && (b1.top <= b2.bottom) &&
		(b1.right >= b2.left) && (b1.right <= b2.right))
	{
		result = 1;
	}
	else if ((b1.bottom >= b2.top) && (b1.bottom <= b2.bottom) &&
		(b1.left >= b2.left) && (b1.left <= b2.right))
	{
		result = 1;
	}
	else if ((b1.bottom >= b2.top) && (b1.bottom <= b2.bottom) &&
		(b1.right >= b2.left) && (b1.right <= b2.right))
	{
		result = 1;
	}
	return result;
}

ushort vBoxesOverlap(vBoundingBox & b1, vBoundingBox & b2)
{
	if (S_BoxesOverlapAux(b1, b2)) return 1;
	if (S_BoxesOverlapAux(b2, b1)) return 1;
	return 0;
}


void vCheckArgs(int arg, int low, int high, const char * error)
{
	if (arg < low || arg > high)
	{
		printf("%s\n", error);
		exit(1);
	}
}


short vIsGIF(const char *filename)
{
	char image_signature[4];
	char * gif_signature = "GIF8";
	FILE* fp = fopen(filename, vFOPEN_READ);
	if (fp == 0) return 0;
	fread(image_signature, sizeof(char), 4, fp);
	fclose(fp);
	if (memcmp(image_signature, gif_signature, 4) == 0)
		return 1;
	else return 0;
}


short vIsJPEG(const char * filename)
{
	char image_signature[4];
	char * jpg_signature = "ÿØÿà";

	FILE* fp = fopen(filename, vFOPEN_READ);
	if (fp == 0) return 0;
	fread(image_signature, sizeof(char), 4, fp);
	fclose(fp);
	if (memcmp(image_signature, jpg_signature, 4) == 0)
		return 1;
	else return 0;
}


short vIsPPM(const char * filename)
{
	char image_signature[3];
	char * ppm_signature = "P6\n";

	FILE* fp = fopen(filename, vFOPEN_READ);
	if (fp == 0) return 0;
	fread(image_signature, sizeof(char), 3, fp);
	fclose(fp);
	if (memcmp(image_signature, ppm_signature, 3) == 0)
		return 1;
	else return 0;
}


short vIsPGM(const char * filename)
{
	char image_signature[3];
	char * png_signature = "P5\n";

	FILE* fp = fopen(filename, vFOPEN_READ);
	if (fp == 0) return 0;
	fread(image_signature, sizeof(char), 3, fp);
	fclose(fp);
	if (memcmp(image_signature, png_signature, 3) == 0)
		return 1;
	else return 0;
}


vint8 vFilesize(const char * filename)
{
	FILE* fp = fopen(filename, vFOPEN_READ);
	if (fp == 0) 
	{
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	int result = ftell(fp);
	fclose(fp);
	return result;
}


unsigned short vCopyFile(const char * source, const char * target)
{
	vint8 size = vFilesize(source);

	FILE * in_fp = fopen(source, vFOPEN_READ);
	if (in_fp == 0) return 0;
	FILE * out_fp = fopen(target, vFOPEN_WRITE);
	if (out_fp == 0) 
	{
		fclose(in_fp);
		return 0;
	}

	long buffer_size = 1000000;
	char * buffer = new char[buffer_size];
	long bytes_in, bytes_out;
	long bytes_copied = 0;

	while(bytes_copied <size)
	{
		bytes_in = fread(buffer, sizeof(char), buffer_size, in_fp);
		bytes_out = fwrite(buffer, sizeof(char), bytes_in, out_fp);
		if (bytes_in != bytes_out) break;
		bytes_copied += bytes_in;
		if ((bytes_copied < size) && (bytes_in < buffer_size)) break;
	}


	fclose(in_fp);
	fclose(out_fp);
	delete [] buffer;
	if (bytes_copied == size) return 1;
	else return 0;
}


short vStringEndsIn(const char * string, const char * ending)
{
	long string_length = strlen(string);
	long ending_length = strlen(ending);
	if (ending_length > string_length) return 0;

	long result = strcmp(&(string[string_length - ending_length]), ending);
	return (result == 0);
}


short vStringCaseEndsIn(const char * string, const char * ending)
{
	if (ending == 0) return 1;
	if (string == 0) return 0;
	long string_length = strlen(string);
	long ending_length = strlen(ending);
	if (ending_length > string_length) return 0;

	long result = strcasecmp(&(string[string_length - ending_length]), ending);
	return (result == 0);
}


void vAdjustDirectoryName(char * directory_name)
{
	long len = strlen(directory_name);
	if (directory_name[len - 1] == '/') directory_name[len - 1] = 0;
	else if (directory_name[len - 1] == '\\') directory_name[len - 1] = 0;
}



vint8 vGetNumber(char * directory)
{
	char * filename = vJoinPaths(directory, ".number");
	FILE * fp = fopen(filename, vFOPEN_READ);
	vdelete2(filename);
	if (fp == 0) return 0;

	long number;
	fscanf(fp, "%li", &number);
	fclose(fp);
	return number;
}

void vIncreaseNumber(char * directory)
{
	char * filename = vJoinPaths(directory, ".number");
	long number;

	FILE * fp = fopen(filename, vFOPEN_READ);
	if (fp == 0) 
	{
		number = 0;
	}
	else
	{
		fscanf(fp, "%li", &number);
		fclose(fp);
	}

	fp = fopen(filename, vFOPEN_WRITE);
	number++;
	vdelete2(filename);
	assert(fp != 0);

	fprintf(fp, "%li\n", number);
	fclose(fp);
}




//////////////////////////////////////////////////////////////////////
// Implementation of the class Bitmap
//////////////////////////////////////////////////////////////////////

Bitmap::Bitmap(vint8 the_size)
{
	size = the_size;
	byte_size = size / 8;
	if (size % 8 != 0) ++byte_size;
	vint8 buffer_size = BufferSize();

	buffer = new uchar[(vector_size) buffer_size];
	memcpy(buffer, &size, sizeof(vint8));
	data = buffer + (buffer_size - byte_size);
	bzero(data, byte_size);
	delete_buffer = 1;
}

Bitmap::Bitmap(vint8 the_size, uchar * the_buffer)
{
	size = the_size;
	byte_size = size / 8;
	vint8 buffer_size = BufferSize();
	if (size % 8 != 0) ++byte_size;
	buffer = the_buffer;
	memcpy(buffer, &size, sizeof(vint8));
	data = buffer + (buffer_size - byte_size);
	bzero(data, byte_size);
	delete_buffer = 0;
}


Bitmap::~Bitmap()
{
	if (delete_buffer != 0)
		vdelete2(buffer);
}

void Bitmap::Save(const char * filename)
{
	FILE * fp = fopen(filename, vFOPEN_WRITE);
	assert(fp != 0);
	Save(fp);
	fclose(fp);
}

void Bitmap::Save(FILE * fp)
{
	fwrite(&size, sizeof(long), 1, fp);
	fwrite(data, sizeof(uchar), (long) byte_size, fp);
}

Bitmap * Bitmap::Load(const char * filename)
{
	FILE * fp = fopen(filename, vFOPEN_READ);
	assert(fp != 0);
	Bitmap * result = Load(fp);
	fclose(fp);
	return result;
}

Bitmap * Bitmap::Load(FILE * fp)
{
	long size = 0;
	fread(&size, sizeof(long), 1, fp);
	Bitmap * result = new Bitmap(size);
	result->LoadData(fp);
	return result;
}

vint8 Bitmap::LoadData(FILE * fp)
{
	vint8 byte_size = size / 8;
	if (size % 8 != 0) byte_size++;
	vint8 items = fread(data, sizeof(uchar), (long) byte_size, fp);
	if (items != byte_size)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

void Bitmap::Write(vint8 index, uchar value)
{
	vint8 byte_index = index / 8;
	vint8 byte_offset = index % 8;
	vint8 byte_mask = 1 << byte_offset;
	if (value == 0)
		data[byte_index] = data[byte_index] & (uchar) ~byte_mask;
	else data[byte_index] = data[byte_index] | (uchar) byte_mask;
}

uchar Bitmap::Read(vint8 index)
{
	vint8 byte_index = index / 8;
	vint8 byte_offset = index % 8;
	vint8 byte_mask = 1 << byte_offset;
	uchar value = data[byte_index] & (uchar) byte_mask;
	uchar result = (value != 0);
	return result;
}


void Bitmap::Print()
{
	long i;
	for (i = 0; i < size; i++)
		printf("%li", Read(i));
	printf("\n");
}


vint8 Bitmap::ByteSize()
{
	return byte_size;
}


vint8 Bitmap::Size()
{
	return size;
}


vint8 Bitmap::BufferSize()
{
	return byte_size + sizeof(vint8);
}


const uchar * Bitmap::GetData()
{
	return data;
}


uchar * Bitmap::WritableData()
{
	return data;
}


void vAddArrays(long * array1, long * array2, long size)
{
	long i;
	for (i = 0; i < size; i++)
		array1[i] = array1[i] + array2[i];
}

void vSubtractArrays(long * array1, long * array2, long size)
{
	long i;
	for (i = 0; i < size; i++)
		array1[i] = array1[i] - array2[i];
}


char * vAdjustEnding2(const char * source, const char * ending)
{
	if (vStringCaseEndsIn(source, ending))
	{
		return vCopyString(source);
	}
	else
	{
		long length1 = strlen(source);
		long length2 = strlen(ending);
		char * result = new char[length1 + length2 + 1];
		sprintf(result, "%s%s", source, ending);
		return result;
	}
}


void vAdjustEnding3(const char * source, char * target, const char * ending)
{
	if (vStringEndsIn(source, ending) == 0)
		sprintf(target, "%s%s", source, ending);
}


char * vCopyString(const char * source)
{
	if (source == 0) return 0;
	long length = strlen(source);
	char * result = new char[length + 1];
	strcpy(result, source);
	return result;
}


char * vCopyString(const char * source, vint8 start, vint8 end)
{
	long length = strlen(source);
	if ((start < 0) || (end < start) || (end >= length)) return 0;

	vint8 new_length = end - start + 1;
	char * result = new char[(vector_size) new_length + 1];
	memcpy(result, &(source[start]), (vector_size) new_length);
	result[new_length] = 0;
	return result;
}


// Replaces every occurrence of old_char in str with new_char.
vint8 vReplaceCharacter(char * str, char old_char, char new_char)
{
	char * pointer = str;
	while(*pointer != 0)
	{
		if (*pointer == old_char)
		{
			*pointer = new_char;
		}
		pointer++;
	}
	return 1;
}


char * string_from_number(vint8 number)
{
	vint8 length = function_decimal_length(number);
	char * result = new char[(vector_size) length+1];
	sprintf(result, "%li", (long)number);
	return result;
}


// this function adds zeros to the beginning of the string, so that
// the length of the result matches the argument "length".
char *  string_from_number(vint8 number, vint8 length)
{
	vint8 number_length = function_decimal_length(number);
	if ((number_length > length) || (number < 0))
	{
		exit_error ("error in  string_from_number(%li, %li)\n", number, length);
	}
	char * result = new char[(vector_size) length+1];
	vint8 index;
	vint8 difference = length - number_length;
	char * current = result;

	for (index = 0; index <difference; index ++)
	{
		current[0] = '0';
		current ++;
	}
	sprintf(current, "%li", (long)number);
	return result;
}


char * vMergeStrings2(const char * first, const char * second)
{
	vint8 length1 = strlen(first);
	vint8 length2 = strlen(second);
	vint8 length = length1 + length2 + 1;
	char * result = new char[(vector_size) length];
	sprintf(result, "%s%s", first, second);
	return result;
}


char * vMergeStrings3(const char * first, const char * second,
	const char * third)
{
	char * temp = vMergeStrings2(first, second);
	char * result = vMergeStrings2(temp, third);
	vdelete2(temp);
	return result;
}


char * vMergeStrings4(const char * first, const char * second,
	const char * third, const char * fourth)
{
	char * temp = vMergeStrings3(first, second, third);
	char * result = vMergeStrings2(temp, fourth);
	vdelete2(temp);
	return result;
}


char * vMergeStrings5(const char * first, const char * second,
	const char * third, const char * fourth,
	const char * fifth)
{
	char * temp = vMergeStrings4(first, second, third, fourth);
	char * result = vMergeStrings2(temp, fifth);
	vdelete2(temp);
	return result;
}


// It returns the shortest ending substring of filename that
// starts with the '.' character. Otherwise it returns the
// empty string "".
char * vGetEnding(const char * filename)
{
	const char * ending_pointer = filename;
	const char * last_dot = 0;
	// Move ending_pointer either to the end of the string
	// or to first dot.
	while(1)
	{
		if (*ending_pointer == 0) break;
		if (*ending_pointer == '.')
		{
			last_dot = ending_pointer;
		}
		ending_pointer++;
	}
	if (last_dot == 0) return 0;

	ending_pointer = last_dot;
	char * result = vCopyString(ending_pointer);
	return result;
}


// It returns result of removing the filename's ending
// from the filename (see comments before vGetEnding on
// how the "ending" of a file is defined). The difference
// between this function and vFileTitle is that vFileTitle
// discards the path part of the filename.
char * vRemoveEnding(const char * filename)
{
	char * ending = vGetEnding(filename);
	vint8 filename_length = strlen(filename);
	vint8 ending_length = strlen(ending);
	vint8 result_length = filename_length - ending_length + 1;
	char * result = new char[(vector_size) result_length];
	memcpy(result, filename, (vector_size) result_length - 1);
	result[result_length - 1] = 0;
	vdelete2(ending);
	return result;
}


char * vFileTitle(const char * filename)
{
	char * simple = vSimpleName(filename, platform_directory_slash);
	char * result = vRemoveEnding(simple);
	vdelete2(simple);
	return result;
}


// If name ends in old_ending, we return a copy of the name, but
// adjusted so that the old ending is replaced by the new
// ending. If name doesn't end in old_ending, then we just append
// the new ending to it.
char * vReplaceEnding3(const char * name, const char * old_ending,
	const char * new_ending)
{
	if (vStringCaseEndsIn(name, old_ending))
	{
		vint8 name_length = strlen(name);
		vint8 old_length;
		if (old_ending == 0) old_length = 0;
		else old_length = strlen(old_ending);
		vint8 new_length = strlen(new_ending);
		char * result = new char[(vector_size) (name_length - old_length + new_length + 1)];
		memcpy(result, name, (vector_size) (name_length - old_length));
		sprintf(&(result[name_length - old_length]), "%s", new_ending);
		return result;
	}
	else
	{
		return vMergeStrings2(name, new_ending);
	}
}


// vReplaceEnding2 finds the ending of the name(defined as the
// smallest ending substring of name that starts with '.') and
// replaces it with new_ending  (which should start with the
// character '.' if we want to have a dot before the ending in
// the result).
char * vReplaceEnding2(const char * name, const char * new_ending)
{
	char * ending = vGetEnding(name);
	char * result = vReplaceEnding3(name, ending, new_ending);
	vdelete2(ending);
	return result;
}


ushort vDate::Read(FILE * fp)
{
	vint8 items_matched = fscanf(fp, "%li/%li/%li", &month, &day, &year);
	if (items_matched != 3) return 0;
	items_matched = fscanf(fp, "%li:%li:%li,", &hour, &minute, &second);
	if (items_matched != 3) return 0;
	items_matched = fscanf(fp, "%li msec", &millisecond);
	if (items_matched != 1) return 0;
	else return 1;
	//9/26/2000 18:10:46, 984 msec
}


void vDate::Print(FILE * fp)
{
	fprintf(fp, "%li/%li/%li %li:%li:%li, %li msec",
		month, day, year, hour, minute, second, millisecond);
}






char * vParentDir(const char * filename, const char * dir_separator)
{
	assert(strlen(dir_separator) == 1);
	int length = strlen(filename);
	char *name2 = new char[length + 1];
	strcpy(name2, filename);
	if (name2[length - 1] == '/') name2[length - 1] = 0;

	const char *last = strrchr(name2, dir_separator[0]);
	char * result;

	if (last == 0)
	{
		result = vCopyString(".");
	}
	else
	{
		int dir_length = strlen(name2) - strlen(last);
		result = new char[dir_length + 1];
		strncpy(result, name2, dir_length);
		result[dir_length] = 0;
	}

	vdelete2(name2);
	return result;
}


char * vSimpleName(const char * filename, const char * separator)
{
	if (filename == 0) 
	{
		return 0;
	}

	assert(strlen(separator) == 1);
	const char *last = strrchr(filename, separator[0]);
	if (last == 0) last = filename;
	else ++last;

	int len = strlen(last);
	char *result = new char[len + 1];
	strcpy(result, last);
	result[len] = 0;
	return result;
}


ushort vPathComponents(const char * original_filename, vector<voidp> * components)
{
	// This makes sure that components is really empty at the start of this function.
	// This guarantees that at the end it will only have the actual components of the 
	// filename.
	if (components->size() != 0) return 0;

	char * filename = vCopyString(original_filename);
	vAdjustDirectoryName(filename);
	vint8 length = strlen(filename);
	vint8 first_i = 0;

	// For the PC case, if the first part of the filename is something like
	// x:, we don't treat that like a component, but instead we merge it with the 
	// next component.
#ifdef WIN32
	if ((length > 3) && (filename[1] == ':') && 
		((filename[2] == '/') || (filename[2] == '\\')) )
	{
		first_i = 3;
	}
#endif

	vint8 start = 0;
	vint8 i;
	for (i = first_i; i < length;)
	{
		char current;
		do
		{
			current = filename[i++];
		}
		while((current != '/') && (current != '\\') && (i < length));

		char * component = vCopyString(filename, start, i - 1);
		vAdjustDirectoryName(component);
		components->push_back(component);
		start = i;
	}

	vdelete2(filename);
	return 1;
}


unsigned short vFileExists(const char *filename)
{
	unsigned short result;
	if (filename == 0) return 0;
	if (filename[0] == 0) return 0;
	FILE * fp = fopen(filename, vFOPEN_READ);
	if (fp == 0) result = 0;
	else 
	{
		result = 1;
		fclose(fp);
	}
	return result;
}


// This function checks if a directory exists by trying to create
// a file called directory/trash and checking if it succeeds.
// If it doesn't succeed, then we assume that the directory doesn't
// exist. Otherwise we know that it exists.
//
// Things can go wrong if the directory exists but we failed to 
// create a file. Here are two examples when that may happen: If
// directory/trash already exists and we don't have write permission to
// it (or to the whole directory). Also, if the disk is full and we 
// fail to create the file because of that. I
// can't think of any other cases
ushort vDirectoryExists(const char * filename)
{
	char * test_name = vJoinPaths(filename, "trash");
	ushort result = vWritableFileExists(test_name);
	vdelete2(test_name);
	return result;
}


ulong vMaskedValue(ulong number, ulong mask)
{
	uchar shift = vMaskShift(mask);
	ulong result = number & mask;
	result = result >> shift;
	return result;
}    


uchar vMaskShift(ulong mask)
{
	uchar result = 0;
	uchar remainder;
	remainder = (uchar) (mask % 2);
	mask = mask / 2;
	while(remainder == 0)
	{
		++result;
		remainder = (uchar) (mask % 2);
		mask = mask / 2;
	}

	return result;
}


void stdarg_test1(int number_of_args, ...)
{
	va_list argument_list;
	char * array[10];
	vint8 counter = 0;
	va_start(argument_list, number_of_args);

	while (counter < number_of_args)
	{
		array[counter] = va_arg(argument_list, char *);
		printf("%s\n", array[counter]);
		counter++;
	}

	va_end(argument_list);
}


void vInsertCharacters(char * source, char * target, char * characters,
	vint8 start, ushort start_from_end)
{
	vint8 source_length = strlen(source);
	vint8 characters_length = strlen(characters);
	if (start >= source_length) return;
	if (start_from_end)
	{
		start = source_length - start - 1;
	}
	memcpy(target, source, (vector_size) start);
	memcpy(&target[start], characters, (vector_size) characters_length);
	strcpy(&(target[start+characters_length]), &(source[start]));
}


char * vInsertCharacters(char * source, char * characters,
	vint8 start, ushort start_from_end)
{
	vint8 source_length = strlen(source);
	vint8 characters_length = strlen(characters);
	vint8 new_length = source_length + characters_length;
	char * result = new char[(vector_size) new_length + 1]; // +1 for the NULL character at the end.
	vInsertCharacters(source, result, characters, start, start_from_end);
	return result;
}


// Removes #number characters from the indicated start position.

void vRemoveCharacters(char * source, char * target,
	vint8 start, vint8 number, unsigned short start_from_end)
{
	vint8 source_length = strlen(source);
	if (start >= source_length) return;
	if (number >= source_length - start) return;
	if (number < 0) return;

	if (start_from_end)
	{
		start = source_length - start - number;
	}
	memcpy(target, source, (vector_size) start);
	strcpy(&(target[start]), &(source[start+number]));
}


char * vRemoveCharacters(char * source, vint8 start,
	vint8 number, unsigned short start_from_end)
{
	if (number < 0) return 0;
	vint8 source_length = strlen(source);
	vint8 new_length = source_length - number;

	char * result = new char[(vector_size) new_length + 1]; 
	// +1 for the NULL character at the end.

	vRemoveCharacters(source, result, start, number, start_from_end);
	return result;
}



void vEraseCharpVector(vector<voidp> * strings)
{
	if (strings == 0) return;
	vint8 number = strings->size();
	vint8 i;
	for (i = 0; i < number; i++)
	{
		vdelete2((char *&) (*strings)[(vector_size) i]);
	}
	strings->erase(strings->begin(), strings->end());
}


void vEraseCharpVector(vector<char *> * strings)
{
	if (strings == 0) return;
	vint8 number = strings->size();
	vint8 i;
	for (i = 0; i < number; i++)
	{
		vdelete2((*strings)[(vector_size) i]);
	}
	strings->erase(strings->begin(), strings->end());
}


ushort vHostToPCUshort(ushort number)
{
#ifdef WIN32
	return number;
#else
	ushort result;
	uchar * buffer = (uchar *) (&result);

	vint8 i;
	ushort temp = number;
	for (i = 0; i < sizeof(ushort); i++)
	{
		buffer[i] = temp % 256;
		temp = temp / 256;
	}
	return result;
#endif
}


vint4 vHostToPCLong(long number)
{
#ifdef WIN32
	return number;
#else
	if (number < 0)
	{
		exit_error("\nerror: vHostToPCLong implementation cannot handle negative numbers.\n");
	}

	vint4 result;
	uchar * buffer = (uchar *) (&result);

	long i;
	long temp = number;
	for (i = 0; i < sizeof(vint4); i++)
	{
		buffer[i] = (uchar) (temp % 256);
		temp = temp / 256;
	}
	return result;
#endif
}


ushort vPCToHostUshort(ushort number)
{
	ushort result;
	uchar * buffer = (uchar *) (&number);

	result = buffer[0] + buffer[1] * 256;
	return result;
}


vint4 vPCToHostLong(vint4 number)
{
	long result;
	uchar * buffer = (uchar *) (&number);

	result = buffer[0] + buffer[1] * 256;
	result = result + buffer[2] * 65536 + buffer[3] * 65536 * 256;

	return result;
}


long vUnixToHostLong(long number)
{
	long result;
	uchar * buffer = (uchar *) (&number);

	result = buffer[3] + buffer[2] * 256;
	result = result + buffer[1] * 65536 + buffer[0] * 65536 * 256;
	return result;
}


vint8 vReadPCUshorts(FILE * fp, ushort * numbers, vint8 length)
{
	ushort * pc_numbers = new ushort[(vector_size) length];
	vint8 items = fread(pc_numbers, sizeof(ushort), (long) length, fp);
	vint8 i;
	for (i = 0; i < items; i++)
	{
		numbers[i] = vPCToHostUshort(pc_numbers[i]);
	}
	delete [] pc_numbers;
	return items;
}


long vReadUnixUshorts(FILE * fp, ushort * numbers, long length)
{
	ushort * pc_numbers = new ushort[length];
	uchar * buffer = (uchar*) pc_numbers;
	long items = fread(pc_numbers, sizeof(ushort), length, fp);
	long i;
	for (i = 0; i < items; i++)
	{
		numbers[i] = buffer[1] + buffer[0] * 256;
		buffer = buffer + 2;
	}
	delete [] pc_numbers;
	return items;
}


vint8 read_integer(FILE * fp, integer * number)
{
	vint4 pc_number;
	vint8 items_read = fread(&pc_number, sizeof(vint4), 1, fp);
	*number = vPCToHostLong(pc_number);
	return items_read;
}


vint8 read_integer(class_file * fp, integer * number)
{
	vint4 pc_number;
	vint8 items_read = fread(&pc_number, sizeof(vint4), 1, fp);
	*number = vPCToHostLong(pc_number);
	return items_read;
}


vint8 read_integers(FILE * fp, integer * numbers, vint8 items)
{
	vint4 * pc_longs = new vint4[(vector_size) items];
	vint8 items_read = fread(pc_longs, sizeof(vint4), (long)items, fp);
	vint8 i;
	for (i = 0; i < items_read; i++)
	{
		numbers[i] = vPCToHostLong(pc_longs[i]);
	}
	delete [] pc_longs;
	return items_read;
}


vint8 read_integers(class_file * fp, integer * numbers, vint8 items)
{
	vint4 * pc_longs = new vint4[(vector_size) items];
	vint8 items_read = fread(pc_longs, sizeof(vint4), (long) items, fp);
	vint8 i;
	for (i = 0; i < items_read; i++)
	{
		numbers[i] = vPCToHostLong(pc_longs[i]);
	}
	delete [] pc_longs;
	return items_read;
}


vint8 vReadUnixLong(FILE * fp, long * number)
{
	long pc_number;
	long items_read = fread(&pc_number, sizeof(long), 1, fp);
	*number = vUnixToHostLong(pc_number);
	return items_read;
}


vint8 vReadUnixLongs(FILE * fp, long * numbers, long items)
{
	long * pc_longs = new long[items];
	long items_read = fread(pc_longs, sizeof(long), items, fp);
	long i;
	for (i = 0; i < items_read; i++)
	{
		numbers[i] = vUnixToHostLong(pc_longs[i]);
	}
	delete [] pc_longs;
	return items_read;
}


vint8 vWritePCUshorts(FILE * fp, ushort * numbers, vint8 length)
{
	ushort * pc_numbers = new ushort[(vector_size) length];
	vint8 i;
	for (i = 0; i < length; i++)
	{
		pc_numbers[i] = vHostToPCUshort(numbers[i]);
	}
	vint8 items = fwrite(pc_numbers, sizeof(ushort), (long) length, fp);
	delete [] pc_numbers;
	return items;
}


vint8 store_integer(FILE * fp, integer number)
{
	vint4 pc_number = vHostToPCLong(number);
	long items_written = fwrite(&pc_number, sizeof(vint4), 1, fp);
	return items_written;
}


vint8 store_integer(class_file * fp, integer number)
{
	vint4 pc_number = vHostToPCLong(number);
	vint8 items_written = fwrite(&pc_number, sizeof(vint4), 1, fp);
	return items_written;
}


vint8 store_integers(FILE * fp, integer * numbers, vint8 items)
{
	vint4 * pc_longs = new vint4[(vector_size) items];
	vint8 i;
	for (i = 0; i < items; i++)
	{
		pc_longs[i] = vHostToPCLong(numbers[i]);
	}
	long items_written = fwrite(pc_longs, sizeof(vint4), (long) items, fp);
	delete [] pc_longs;
	return items_written;
}


vint8 store_integers(class_file * fp, integer * numbers, vint8 items)
{
	vint4 * pc_longs = new vint4[(vector_size) items];
	long i;
	for (i = 0; i < items; i++)
	{
		pc_longs[i] = vHostToPCLong(numbers[i]);
	}
	vint8 items_written = fwrite(pc_longs, sizeof(vint4), (long) items, fp);
	delete [] pc_longs;
	return items_written;
}


// Reading and writing doubles in the PC byte order is not yet
// implemented in a platform-independent way, so far it only
// works correctly on PC platforms where we can use fwrite to
// to it.
vint8 vReadPCDoubles(FILE * fp, double * numbers, long items)
{
#ifdef WIN32
	return fread(numbers, sizeof(double), items, fp);
#else
	assert(0);
	return -1;
#endif
}

vint8 vWritePCDoubles(FILE * fp, double * numbers, long items)
{
#ifdef WIN32
	return fwrite(numbers, sizeof(double), items, fp);
#else
	assert(0);
	return -1;
#endif
}


vint8 read_vint8(FILE * fp, vint8 * a)
{
	vint8 c;
	uchar * buffer = (uchar *) &c;
	long items= fread(buffer, 8, 1, fp);

	long i;
	for (i = 0; i < 8; i++)
	{
		*a = *a << 8;
		*a = *a + buffer[i];
	}
	return items;
}


vint8 read_vint8s(FILE * fp, vint8 * numbers, vint8 items)
{
	vint8 * new_numbers = new vint8[(vector_size) items];
	long items_read = fread(new_numbers, 8, (long) items, fp);
	long i;
	for (i = 0; i < items_read; i++)
	{
		uchar * buffer = (uchar *) &new_numbers[i];
		vint8 & temp = numbers[i];
		for (long j = 0; j < 8; j++)
		{
			temp = temp >> 8;
			temp = temp + buffer[i];
		}
	}
	vdelete2(new_numbers);
	return items_read;
}


vint8 read_vint8s(class_file * fp, vint8 * numbers, vint8 items)
{
	vint8 * new_numbers = new vint8[(vector_size) items];
	vint8 items_read = fread(new_numbers, 8, (long) items, fp);
	vint8 i;
	for (i = 0; i < items_read; i++)
	{
		uchar * buffer = (uchar *) &new_numbers[i];
		vint8 & temp = numbers[i];
		for (vint8 j = 0; j < 8; j++)
		{
			temp = temp >> 8;
			temp = temp + buffer[i];
		}
	}
	vdelete2(new_numbers);
	return items_read;
}


// read_old_longs is useful for reading 32-bit integers into vint8 arrays
// the name comes from the fact that vint8 is a replacement for
// the "long" type, which in old (32-bit) platforms was only a 32-bit integer.
vint8 read_old_longs(FILE * fp, vint8 * numbers, vint8 items)
{
	vint4 * temporary = new vint4[(vector_size) items];
	vint8 items_read = read_integers(fp, temporary, items);
	vint8 i;
	for (i = 0; i < items_read; i++)
	{
		numbers[i] = temporary[i];
	}
	delete_pointer(temporary);
	return items_read;
}


vint8 read_old_longs(class_file * fp, vint8 * numbers, vint8 items)
{
	vint4 * temporary = new vint4[(vector_size) items];
	vint8 items_read = read_integers(fp, temporary, items);
	vint8 i;
	for (i = 0; i < items_read; i++)
	{
		numbers[i] = temporary[i];
	}
	return items_read;
}



vint8 store_vint8(file_handle * fp, vint8 a)
{
	vint8 c;
	uchar * buffer = (uchar *) &c;
	vint8 b = a;
	long i;
	for (i = 0; i < 8; i++)
	{
		buffer[i] = (uchar) (b & 255);
		b = b >> 8;
	}
	vint8 items_written = fwrite(buffer, 8, 1, fp);
	return items_written;
}


vint8 store_vint8(class_file * fp, vint8 a)
{
	vint8 c;
	uchar * buffer = (uchar *) &c;
	vint8 b = a;
	long i;
	for (i = 0; i < 8; i++)
	{
		buffer[i] = (uchar) (b & 255);
		b = b >> 8;
	}
	vint8 items_written = fwrite(buffer, 8, 1, fp);
	return items_written;
}


vint8 store_vint8s(file_handle * fp, const vint8 * numbers, vint8 items)
{
	vint8 * new_numbers = new vint8[(vector_size) items];
	vint8 i;
	for (i = 0; i < items; i++)
	{
		uchar * buffer = (uchar *) &new_numbers[i];
		vint8 temp = numbers[i];
		for (long j = 0; j < 8; j++)
		{
			buffer[j] = (uchar) (temp & 255);
			temp = temp >> 8;
		}
	}
	vint8 items_written = fwrite(new_numbers, 8, 1, fp);
	return items_written;
}


vint8 store_vint8s(class_file * fp, const vint8 * numbers, vint8 items)
{
	vint8 * new_numbers = new vint8[(vector_size) items];
	vint8 i;
	for (i = 0; i < items; i++)
	{
		uchar * buffer = (uchar *) &new_numbers[i];
		vint8 temp = numbers[i];
		for (vector_size j = 0; j < 8; j++)
		{
			buffer[j] = (uchar) (temp & 255);
			temp = temp >> 8;
		}
	}
	vint8 items_written = fwrite(new_numbers, 8, 1, fp);
	return items_written;
}


// store_old_longs is useful for reading 32-bit integers into vint8 arrays
// the name comes from the fact that vint8 is a replacement for
// the "long" type, which in old (32-bit) platforms was only a 32-bit integer.
vint8 store_old_longs(file_handle * fp, const vint8 * numbers, vint8 items)
{
	integer * temporary = new integer[(vector_size) items];
	vint8 i;
	for (i = 0; i < items; i++)
	{
		if (function_absolute(numbers[i]) >= 2147483648) // 2^31
		{
			exit_error("error: vint8 too big to convert to integer");
		}
		temporary[i] = (integer) (numbers[i]);
	}
	vint8 items_written = store_integers(fp, temporary, items);
	delete_pointer(temporary);
	return items_written;
}


vint8 store_old_longs(class_file * fp, const vint8 * numbers, vint8 items)
{
	integer * temporary = new integer[(vector_size) items];
	vint8 i;
	for (i = 0; i < items; i++)
	{
		if (function_absolute(numbers[i]) >= 2147483641) // 2^31
		{
			exit_error("error: vint8 too big to convert to integer");
		}
		temporary[i] = (integer) (numbers[i]);
	}
	vint8 items_written = store_integers(fp, temporary, items);
	delete_pointer(temporary);
	return items_written;
}


void vGetVint8Bytes(vint8 a, uchar * buffer)
{
	vint8 b = a;
	long i;
	for (i = 0; i < 8; i++)
	{
		buffer[i] = (uchar) (b & 255);
		b = b >> 8;
	}
}



ushort S_QuickMatchTest(const char * pattern, const char * filename)
{
	if (pattern == 0) return 0;
	if (filename == 0) return 0;
	long pattern_length = strlen(pattern);
	long name_length = strlen(filename);
	long pattern_index = 0;
	long name_index = 0;

	while(1)
	{
		if ((pattern_index == pattern_length) &&
			(name_index == name_length)) return 1;
		if (pattern_index == pattern_length) return 1;
		if (name_index == name_length) return 0;
		char pattern_char = pattern[pattern_index++];
		if (pattern_char == '*') continue;
		if (pattern_char == '?') continue;
		char name_char = filename[name_index++];
		while((name_index < name_length) && (name_char != pattern_char))
		{
			name_char = filename[name_index++];
		}
		if (name_char != pattern_char) return 0;
	}
}


// This function assumes that neither the pattern nor the filename 
// end in / or \. The only wildcards supported are * and ?.
ushort vFilePatternsMatch(const char * original_pattern, 
	const char * filename)
{
	if (original_pattern == 0) return 0;
	if (filename == 0) return 0;

	char * pattern = vCopyString(original_pattern);
	long pattern_length = strlen(pattern);
	// Because a pattern can be like "D:/Vassilis", i.e. the first component
	// ("D:") is not treated like a component, we have to detect that case here.
	if (pattern_length > 3)
	{
		char * possible_drive = vCopyString(pattern, 0, 3);
		if ((possible_drive[1] == ':') &&
			((possible_drive[2] == '/') || (possible_drive[2] == '\\')))
		{
			vdelete2(pattern);
			pattern = vCopyString(original_pattern, 3, pattern_length - 1);
			pattern_length = strlen(pattern);
		}
		vdelete2(possible_drive);
	}

	//  ushort quick = S_QuickMatchTest(pattern, filename);
	//  if (quick == 0) return 0;

	// First take care of pathological cases (empty pattern and/or empty name).
	long name_length = strlen(filename);
	if ((pattern_length == 0) && (name_length == 0)) return 1;
	if (pattern_length == 0) return 0;
	long i;
	if (name_length == 0) 
	{
		for (i = 0; i < pattern_length; i++)
		{
			if (pattern[i] != '*') return 0;
		}
		return 1;
	}

	// We will use dynamic programming: matrix[i][j] is 0 iff 
	// pattern up to and including i doesn't match filename up
	// to and including j.
	uchar ** matrix = new uchar*[pattern_length];
	for (i = 0; i < pattern_length; i++)
	{
		matrix[i] = new uchar[name_length];
	}

	uchar star_flag = 1; // True until we find the first non-star.

	// Fill up the first row of the matrix. 
	char pattern_char = pattern[0];
	char file_char = filename[0];
	// Make it case-insensitive
	if ((pattern_char >= 97) && (pattern_char <= 122))
	{
		pattern_char = pattern_char - 32;
	}
	if ((file_char >= 97) && (file_char <= 122))
	{
		file_char = file_char - 32;
	}
	if (pattern_char == '*')
	{
		for (i = 1; i < name_length; i++)
		{
			matrix[0][i] = 1;
		}
	}
	else if ((pattern_char == '?') || (pattern_char == file_char))
	{
		matrix[0][0] = 1;
		for (i = 1; i < name_length; i++)
		{
			matrix[0][i] = 0;
		}
		star_flag = 0;
	}

	// Fill up the first column of the matrix.
	for (i = 1; i < pattern_length; i++)
	{
		pattern_char = pattern[i];
		// Make it case-insensitive
		if ((pattern_char >= 97) && (pattern_char <= 122))
		{
			pattern_char = pattern_char - 32;
		}

		uchar previous = matrix[i-1][0];
		if (pattern_char == '*') matrix[i][0] = previous;
		else 
		{
			if (pattern_char == '?') matrix[i][0] = star_flag;
			else if (pattern_char == file_char) matrix[i][0] = star_flag;
			else matrix[i][0] = 0;
			star_flag = 0;
		}
	}

	// Fill out the rest of the matrix.
	long j;
	for (i = 1; i < pattern_length; i++)
	{
		pattern_char = pattern[i];
		if ((pattern_char >= 97) && (pattern_char <= 122))
		{
			pattern_char = pattern_char - 32;
		}
		for (j = 1; j < name_length; j++)
		{
			file_char = filename[j];
			if (pattern_char == '*')
			{
				if ((matrix[i-1][j-1] == 1) || (matrix[i][j-1] == 1) ||
					(matrix[i-1][j] == 1)) 
				{
					matrix[i][j] = 1;
				}
				else matrix[i][j] = 0;
			}
			else if (pattern_char == '?')
			{
				matrix[i][j] = matrix[i-1][j-1];
			}
			else 
			{
				// Make it case-insensitive
				if ((file_char >= 97) && (file_char <= 122))
				{
					file_char = file_char - 32;
				}
				if (pattern_char != file_char) 
				{
					matrix[i][j] = 0;
				}
				else matrix[i][j] = matrix[i-1][j-1];
			}
		}
	}

	unsigned short result = matrix[pattern_length-1][name_length-1];

	for (i = 0; i < pattern_length; i++)
	{
		vdelete2(matrix[i]);
	}
	vdelete2(matrix);

	vdelete2(pattern);
	return result;
}


void vWaitForEnter()
{
	cout << "Press <ENTER> to continue: ";
#ifdef VASSILIS_WINDOWS_MFC
	cin.flush();
	char buffer[20];
	cin.getline(buffer, 10);
#endif
	getchar();
	getchar();
}


char * vJoinPaths(const char * original_path1, const char * path2)
{
	// Added by Diego. It is handy to allow either param to be
	// a null pointer.
	if (original_path1 == NULL && path2 == NULL)
		return NULL;
	else if (original_path1 == NULL)
		return vCopyString(path2);
	else if (path2 == NULL)
		return vCopyString(original_path1);

	// Otherwise, concatenate strings
	char * path1 = vCopyString(original_path1);
	long length1 = strlen(path1);

	// If the last char is already a path slash, then remove it
	// so that it can later be reinserted
	if (path1[length1 - 1] == platform_directory_slash[0])
		path1[length1 - 1] = 0;

	long length2 = strlen(path2);

	char * result = new char[length1 + length2 + 10];

	sprintf(result, "%s%s%s", path1, platform_directory_slash, path2);

	delete [] path1;

	return result;
}


char * vJoinPaths3(const char * path1, const char * path2, 
	const char * path3)
{
	char * temp = vJoinPaths(path1, path2);
	char * result = vJoinPaths(temp, path3);
	vdelete2(temp);
	return result;
}


char * vJoinPaths4(const char * path1, const char * path2, 
	const char * path3, const char * path4)
{
	char * temp = vJoinPaths3(path1, path2, path3);
	char * result = vJoinPaths(temp, path4);
	vdelete2(temp);
	return result;
}


char * vJoinPaths5(const char * path1, const char * path2, 
	const char * path3, const char * path4, const char * path5)
{
	char * temp = vJoinPaths4(path1, path2, path3, path4);
	char * result = vJoinPaths(temp, path5);
	vdelete2(temp);
	return result;
}


int vPrintAndFlush(const char * format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	int result = vprintf(format, arguments);
	va_end(arguments);
	fflush(stdout);

	return result;
}


// This version is fine for standard consoles (UNIX and DOS applications)

int vScanAndVerify(const char * format, ...)
{
	va_list arguments;
	va_start(arguments, format);

	long result = vVScanAndVerify(format, arguments);
	va_end(arguments);
	return result;
}


// like vscanf, but it doesn't give up until all fields have been 
// successfully matched.
int vVScanAndVerify(const char * format, va_list arguments)
{
	long length = strlen(format);
	long fields = 0;
	long i;
	vector<long> field_indices;

	// Count how many arguments we expect, and where each of them is.
	for (i = 0; i < length; i++)
	{
		if (format[i] == '%') 
		{
			fields++;
			field_indices.push_back(i);
		}
	}
	field_indices.push_back(length);

	char * temp_format = new char[length+1];
	i = 0;

	while (i < fields)
	{
		int items_matched;
		long start = field_indices[i];
		long end = field_indices[i+1];
		long temp_length = end - start;
		memcpy(temp_format, &(format[start]), temp_length);
		temp_format[temp_length] = 0;
		void * argument = va_arg(arguments, void*);
		do
		{
			items_matched = scanf(temp_format, argument);
			if (items_matched != 1) 
			{
				vPrint("%s: ", &format[start]);;
				fflush(stdin);
			}
		}
		while(items_matched != 1);

		// Commented out this while loop on 6/20/2001. I don't see why it is needed.
		//    do
		//    {
		//**  items_matched = console.TryToReadItem(temp_format, argument);
		//      if (items_matched != 1) 
		//      {
		//        vPrint("%s: ", &format[start]);
		//    console.GetLine();
		//      }
		//    }
		//    while(items_matched != 1);

		i++;
	}

	delete [] temp_format;
	return fields;
}


// Like fscanf, but it takes a va_list as an argument (this comes with C++
// in some platforms, called vfscanf)
int vFscan(FILE * fp, const char * format, va_list arguments)
{
	long length = strlen(format);
	long fields = 0;
	long i;
	vector<long> field_indices;

	// Count how many arguments we expect, and where each of them is.
	// Note that a format string containing "%%" wouldn't work here.
	for (i = 0; i < length; i++)
	{
		if (format[i] == '%') 
		{
			fields++;
			field_indices.push_back(i);
		}
	}
	field_indices.push_back(length);

	char * temp_format = new char[length+1];
	long fields_read = 0;

	while (fields_read < fields)
	{
		int items_matched;
		long start = field_indices[fields_read];
		long end = field_indices[fields_read+1];
		long temp_length = end - start;
		memcpy(temp_format, &(format[start]), temp_length);
		temp_format[temp_length] = 0;
		void * argument = va_arg(arguments, void*);

		items_matched = fscanf(fp, temp_format, argument);
		if (items_matched != 1) 
		{
			vPrint("%s: ", &format[start]);;
			fflush(stdin);
			break;
		}

		fields_read++;
	}

	delete [] temp_format;
	return fields_read;
}


// Get a non-empty line from a file. If the first non-empty line
// found exceeds max_length, only the first max_length characters 
// are returned. It returns the length of the string (with the
// newline character removed).
long vFileGetNonEmptyLine(FILE * fp, char * buffer, long max_length)
{
	char character = '\n';

	// First, go through empty lines.
	while (character == '\n')
	{
		long result = fscanf(fp, "%c", &character);
		// if we failed to read the next character, we assume
		// that we got to the end of file. That assumption may be wrong
		// in case of hardware or network failure.
		if (result != 1) 
		{
			buffer[0] = 0;
			return 0;
		}
	}

	// Now read the whole line.
	long index = 0;
	while(character != '\n')
	{
		buffer[index++] = character;
		if (index == max_length) break;
		long result = fscanf(fp, "%c", &character);
		// if we got to the end of file or failed for whatever reason,
		// we return a success value, since we managed to read a
		// non-empty line. In principle, maybe we didn't get to the 
		// end of file but the disk broke down or something, but I
		// not handling that here.
		if (result != 1) break;
	}

	buffer[index] = 0;
	return index;
}


// returns the length of the line that was read
vint8 vShellGetLine(char * buffer, vint8 max_length)
{
	vint8 i;
	for (i = 0; i < max_length - 1; i++)
	{
		char c = getchar();
		if (c == '\n')
		{
			break;
		}
		else
		{
			buffer[i] = c;
		}
	}
	buffer[i] = 0;
	return i;
}


vint8 vGetNonEmptyLine(char * buffer, vint8 max_length)
{
	vint8 length;
	do
	{
		length = vGetLine(buffer, max_length);
	}
	while (length == 0);
	return 1;
}


vint8 vTypeToVint8(Type type)
{
	switch(type)
	{
	case ScharType:
		return 0;

	case ShortType:
		return 1;

	case IntType:
		return 2;

	case Vint8Type:
		return 3;

	case FloatType:
		return 4;

	case DoubleType:
		return 5;

	case UcharType:
		return 6;

	case UshortType:
		return 7;

	case UintType:
		return 8;

	case UlongType:
		return 9;

	case OtherType:
		return 10;

	default:
		assert(0);
		exit_error("Bad type passed to vTypeToVint8\n");
		return 0;
	}
	return 0;
}


Type vVint8ToType(vint8 number)
{
	switch(number)
	{
	case 0:
		return ScharType;

	case 1:
		return ShortType;

	case 2:
		return IntType;

	case 3:
		return Vint8Type;

	case 4:
		return FloatType;

	case 5:
		return DoubleType;

	case 6:
		return UcharType;

	case 7:
		return UshortType;

	case 8:
		return UintType;

	case 9:
		return UlongType;

	case 10:
		return OtherType;

	default:
		assert(0);
		exit_error("Bad type passed to vVint8Type\n");
		return OtherType;
	}
	return OtherType;
}


vint8 vSizeOfType(Type type)
{
	switch(type)
	{
	case ScharType:
		return sizeof(signed char);

	case ShortType:
		return sizeof(short);

	case IntType:
		return sizeof(int);

	case Vint8Type:
		return sizeof(long);

	case FloatType:
		return sizeof(float);

	case DoubleType:
		return sizeof(double);

	case UcharType:
		return sizeof(uchar);

	case UshortType:
		return sizeof(ushort);

	case UintType:
		return sizeof(uint);

	case UlongType:
		return sizeof(ulong);

	case OtherType:
		assert(0);
		return -1;

	default:
		assert(0);
		exit_error("Bad type passed to vSizeOfType\n");
		return 0;
	}
	return 0;
}


unsigned short vWritableFileExists(char * filename)
{
	FILE * fp = fopen(filename, vFOPEN_WRITE);
	if (fp == 0) return 0;
	char buffer[2];
	buffer[0] = 0;
	buffer[1] = 0;
	int items_written = fwrite(buffer, sizeof(char), 2, fp);
	fclose(fp);
	//  unlink(filename);
	remove(filename);
	if (items_written == 2) return 1;
	else return 0;
}


// note: vReadStringText and vWriteStringText are problematic,
// because they cannot handle cases where the string contains
// spaces or other special characters. Such strings can be saved,
// but they cannot be read.
char * vReadStringText(FILE * fp)
{
	long length;
	long scan_result;

	scan_result = fscanf(fp, "%li", &length);
	if (scan_result != 1) return 0;
	char * result = new char[length+1];
	scan_result = fscanf(fp, "%s\n", result);
	if (scan_result != 1)
	{
		vdelete2(result);
		return 0;
	}
	result[length] = 0;
	return result;
}

vint8 vWriteStringText(FILE * fp, const char * string)
{
	vint8 result = fprintf(fp, "%i %s\n", strlen(string), string);
	return result;
}


vint8 vWritePCString(FILE * fp, char * string)
{
	vint8 success = 1;
	vint8 length = strlen(string);
	vint8 temp_success = store_vint8(fp, length);
	if (temp_success != 1) 
	{
		success = 0;
	}
	temp_success = fwrite(string, sizeof(char), (long) length, fp);
	if (temp_success != length) 
	{
		success = 0;
	}
	return success;
}


char * vReadPCString(FILE * fp)
{
	vint8 length = 0;
	vint8 temp_success = read_vint8s(fp, &length, 1);
	if (temp_success != 1) 
	{
		return 0;
	}
	char * result = new char[(vector_size) length+1];
	temp_success = fread(result, sizeof(char), (long) length, fp);
	if (temp_success != length) 
	{
		delete [] result;
		return 0;
	}
	result[length] = 0;
	return result;
}


// Assuming that all the data in the is numbers, the numbers
// are read and stored in the vector "numbers".
vint8 vReadDoubleVectorB(const char * filename, 
	std::vector<double> * numbers)
{
	FILE * fp = fopen(filename, vFOPEN_READ);
	if (fp == 0) 
	{
		vPrint("\nFailed to open file %s from vReadDoubleVector\n", filename);
		return 0;
	}
	vint8 success = vReadDoubleVectorA(fp, numbers);
	fclose(fp);
	return success;
}


vint8 vReadDoubleVectorA(FILE * fp, std::vector<double> * numbers)
{
	double number = 0;
	long success = 1;
	while(1)
	{
		long items_matched = fscanf(fp, "%lf", &number);
		// Finding the end of file is expected to happen at some point,
		// as long as no problems have occurred.
		if (items_matched == EOF) break;
		// Any other negative result indicates a problem.
		if (items_matched != 1)
		{
			vPrint("\nproblems in vReadDoubleVector\n");
			success = 0;
			break;
		}
		numbers->push_back(number);
	}

	return success;
}



// Length required to write a decimal number with characters. We include
// a character for the minus sign, if the number is negative.
vint8 function_decimal_length(vint8 number)
{
	if (number < 0) return function_decimal_length(-number) + 1;

	vint8 result = 1;
	vint8 temp = number;
	while(temp >= 10)
	{
		temp = temp / 10;
		result++;
	}
	return result;
}


// Returns a random 32-bit integer using rand (which returns a 15-bit 
// random integer).
vint8 function_random_vint8()
{
	ulong result;
	//  srand(time(0));
	ushort * ushorts = (ushort *) (&result);
	ushorts[0] = rand();
	ushorts[1] = rand();
	int random3 = rand();
	// Use random 3 to get two more random bits (basically, extend
	// ushorts[0] and [1] from 15 bits to 16 bits.
	int bit1 = random3 % 2;
	int bit2 = (random3 / 2) % 2;
	ushorts[0] = ushorts[0] * 2 + bit1;
	ushorts[1] = ushorts[1] * 2 + bit2;
	return result;
}


// Returns a random number, uniformly distributed between low and high
// (including low and high).
vint8 function_random_vint8(vint8 low, vint8 high)
{
	vint8 random_number = function_random_vint8();
	vint8 range = high - low + 1;
	vint8 temp = random_number % range;
	vint8 result = temp + low;
	return result;
}


// Returns a random number, uniformly distributed between low and high
double vRandomDouble(double low, double high)
{
	const long precision = 2000000000; // 2 billion 
	double random_number = (double) function_random_vint8(0, precision);
	double range = high - low;
	double temp = random_number * (range / (double) precision);
	double result = temp + low;
	return result;
}


// exit_error is a handy way to exit when we encounter a condition
// that the code cannot handle. Sometimes that condition is caused by
// bugs, sometimes it is simply stuff I chose not to address to save
// time. Calling this function we get a helpful message. Another very
// useful thing is that, in debug mode, if we set a breakpoint on
// this function, we can easily get to the lines of code that demonstrate
// the problem.
vint8 exit_error(char * format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	vPrint("\n");
	int result = vVprint(format, arguments);
	va_end(arguments);

	// Before we exit, wait for the user to type in something.
	int junk = 0;
	vScan("%i", &junk);
	exit(0);
	return 1;
}


// function_warning is an alternative to calling exit_error, when we
// encounter a condition that we can handle, but which is strong 
// evidence that something is wrong. Again, by putting a breakpoint 
// on this function, we can easily spot the cases where that happens.
vint8 function_warning(char * format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	vPrint("\n");
	int result = vVprint(format, arguments);
	va_end(arguments);

	// Before we let the program continue, wait for the user to type 
	// in something.
	vPrint("\nEnter a number to continue:\n");
	int junk = 0;
	vScan("%i", &junk);
	return junk;
	// return 1;
}


// Choose "number" numbers between low and high, and store them
// in result. No repetitions are allowed.
// Current implementation is very inefficient.
vint8 vPickWithoutRepetitions2(long low, long high, long number, 
	vector<vint8> * result)
{
	long range = high - low + 1;
	if (number > range) return 0;

	long i;
	for (i = 0; i < number; i++)
	{
		long found = 0;
		vint8 pick = 0;
		do
		{
			pick = function_random_vint8(low, high);
			found = 0;
			long j;
			for (j = 0; j < i; j++)
			{
				if (pick == (*result)[j])
				{ 
					found = 1;
					break;
				}
			}
		}
		while(found == 1);
		result->push_back(pick);
	}
	return 1;
}


/////////////////////////////////////////////////////////////////////
//                                                                 //
//          Beginning of implementation of vStringTable           //
//                                                                 //
/////////////////////////////////////////////////////////////////////


vStringTable::vStringTable()
{
}


vStringTable::~vStringTable()
{
	long size = keys.size();
	if (size > 0)
	{
		vPrint("\nWarning: string table deleted with %li undeleted objects.\n",
			size);
	}
	long i;
	for (i = 0; i < size; i++)
	{
		vPrint("undeleted item %li stored under key %s\n", i, keys[i]);
		vdelete2(keys[i]);
	}
}


vint8 vStringTable::Clear()
{
	long size = keys.size();
	long i;
	for (i = 0; i < size; i++)
	{
		vdelete2(keys[i]);
	}
	keys.erase(keys.begin(), keys.end());
	objects.erase(objects.begin(), objects.end());

	return 1;
}



// Store object under key. Fail if key is already used.
vint8 vStringTable::Put(const char * key, void * object)
{
	// If the key is already used, fail.
	void * object2 = Get(key);
	if (object2 != 0) return 0;

	keys.push_back(vCopyString(key));
	objects.push_back(object);
	return 1;
}


// Return object stored under given key
void * vStringTable::Get(const char * key)
{
	void * result = 0;
	long size = keys.size();

	long i;
	for (i = 0; i < size; i++)
	{
		if (strcmp(key, keys[i]) == 0)
		{
			result = objects[i];
			break;
		}
	}

	return result;
}

// Remove object stored under given key, and return it.
void * vStringTable::Remove(const char * key)
{
	void * result = 0;
	long size = keys.size();

	long i;
	for (i = 0; i < size; i++)
	{
		if (strcmp(key, keys[i]) == 0)
		{
			return RemoveIndex(i);
		}
	}

	return result;
}


// Return the number of objects stored.
vint8 vStringTable::Size()
{
	return keys.size();
}


// Return the i-th object stored. 
void * vStringTable::GetIndex(vint8 index)
{
	if ((index < 0) || (index >= (long) objects.size())) return 0;
	return objects[(vector_size) index];
}

// Remove the i-th object, and return it.
void * vStringTable::RemoveIndex(vint8 index)
{
	vint8 size = Size();
	void * result = objects[(vector_size) index];
	vdelete2(keys[(vector_size) index]);
	vint8 i;
	for (i = index; i < size-1; i++)
	{
		keys[(vector_size) i] = keys[(vector_size) i+1];
		objects[(vector_size) i] = objects[(vector_size) i+1];
	}
	keys.pop_back();
	objects.pop_back();
	return result;
}


// Returns true if the key is being used already, false
// otherwise.
vint8 vStringTable::KeyExists(const char * key)
{
	long result = 0;
	long size = keys.size();

	long i;
	for (i = 0; i < size; i++)
	{
		if (strcmp(key, keys[i]) == 0)
		{
			result = 1;
			break;
		}
	}

	return result;
}


// Returns the number of keys in the table that are mapped to
// the given object.
vint8 vStringTable::CountKeys(void * object)
{
	long result = 0;
	long size = keys.size();

	long i;
	for (i = 0; i < size; i++)
	{
		if (objects[i] == object)
		{
			result = result + 1;
		}
	}

	return result;
}


// It returns some key that is mapped to the given object. If no
// such key exists, then it returns 0.
const char * vStringTable::GetKey(void * object)
{
	const char * result = 0;
	long size = keys.size();

	long i;
	for (i = 0; i < size; i++)
	{
		if (objects[i] == object)
		{
			result = keys[i];
			break;
		}
	}

	return result;
}


/////////////////////////////////////////////////////////////////////
//                                                                 //
//             End of implementation of vStringTable              //
//                                                                 //
/////////////////////////////////////////////////////////////////////


vint8 function_make_directory(const char * directory)
{
	if (vDirectoryExists(directory) > 0)
	{
		return 2;
	}

	char * buffer = new char[strlen(directory) + 20];
	sprintf(buffer, "mkdir \"%s\"", directory);
	system(buffer);
	vdelete2(buffer);

	if (vDirectoryExists(directory) <= 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}


class_file::class_file (const char* filename, const char* in_specifications)
{
	specifications = function_copy_string (in_specifications);
	split = 0;
	file_size = 0;
	pathname = 0;
	file_pointer = fopen (filename, specifications);
	if (file_pointer != 0)
	{
		return;
	}

	split = 1;
	char* parent = function_parent_directory (filename, platform_directory_slash);
	char* first_simple = function_simple_name (filename, platform_directory_slash);
	char* second_simple = function_replace_ending_three (first_simple, "_distances.bin", "");

	char* simple = function_merge_strings ("split_", second_simple);

	pathname = function_join_paths(parent, simple);
	delete_pointer (parent);
	delete_pointer (first_simple);
	delete_pointer (second_simple);
	delete_pointer (simple);
	char* temporary_filename = pathname_from_index (0);
	file_pointer = fopen (temporary_filename, specifications);
	if (file_pointer != 0)
	{
		current_index = 0;
		file_size = function_file_size(temporary_filename);
		if (file_size < 0)
		{
			exit_error ("error in class_file, file_size = %li\n", file_size);
		}
	}
	else
	{
		delete_pointer (pathname);
		split = 0;
		pathname = 0;
	}

	delete_pointer (temporary_filename);
}


class_file::~class_file ()
{
	delete_pointer (specifications);
	delete_pointer (pathname);
	if (file_pointer != 0)
	{
		fclose (file_pointer);
	}
}

// these functions add the right suffix to the pathname,
// for split files.  They should probably be private
// or protected.
char* class_file::pathname_from_position(vint8 position)
{
	vint8 index = index_from_position(position);
	return pathname_from_index(index);
}


char* class_file::pathname_from_index(vint8 index)
{
	if (split == 0)
	{
		exit_error ("error: pathname_from_index called for non-split file\n");
	}

	const long maximum = 26*26 -1;
	if ((index < 0) || (index > maximum))
	{
		exit_error ("error in pathname_from_index: index = %li\n", index);
	}

	vint8 base = (vint8) 'a';
	vint8 first_number = index/26;
	vint8 second_number = index % 26;
	char first_character = (char) (base + first_number);
	char second_character = (char) (base + second_number);

	vint8 length = strlen(pathname) + 3;
	char* result = new char[(vector_size) length];
	sprintf(result, "%s%c%c", pathname, first_character, second_character);
	return result;
}


vint8 class_file::index_from_position(vint8 position)
{
	if (split == 0)
	{
		exit_error ("error: pathname_from_index called for non-split file\n");
	}

	vint8 result = position/file_size;
	return result;
}


vint8 class_file::open_index(vint8 index)
{
	if (split == 0)
	{
		exit_error ("error: pathname_from_index called for non-split file\n");
	}

	fclose(file_pointer);
	char* temporary_filename = pathname_from_index (index);
	file_pointer = fopen (temporary_filename, specifications);
	delete_pointer (temporary_filename);
	if (file_pointer != 0)
	{
		current_index = index;
		return 1;
	}
	else
	{
		return 0;
	}
}

// this should be the substitute for fread.
vint8 class_file::read (void* buffer, size_t type_size, size_t items)
{
	if (file_pointer == 0)
	{
		return 0;
	}

	if (split == 0)
	{
		return fread(buffer, type_size, items, file_pointer);
	}

	long number = fread(buffer, type_size, items, file_pointer);
	if (number == items)
	{
		return number;
	}

	vint8 success = open_index (current_index +1);
	if (success <= 0)
	{
		return number;
	}

	char* char_buffer = (char*) buffer;
	char_buffer = char_buffer + number*type_size;
	number = number + fread(char_buffer, type_size, items - number, file_pointer);
	return number;
}

// this should be the substitute for fwrite.  For the time
// being, this will only work for non-split files.
vint8 class_file::write(void* buffer, size_t type_size, size_t items)
{
	if (split != 0)
	{
		exit_error ("error: class_file:: write called with non-split file\n");
	}
	return fwrite (buffer, type_size, items, file_pointer);
}

// this should be the substitute for fseek and my 64-bit
// equivalent.
vint8 class_file::seek (vint8 position, int origin)
{
	if (split == 0)
	{
		return function_seek(file_pointer, position, origin);
	}

	if (origin != SEEK_SET)
	{
		exit_error ("error: class_file:: seek can only handle SEEK_SET\n");
	}

	long index = (long) (position/file_size);
	vint8 displacement = position% file_size;
	vint8 success = open_index(index);
	if (success <= 0)
	{
		return 1;
	}

	long result = function_seek(file_pointer, displacement,SEEK_SET);
	return result;
}

// they should be the substitute for ftell
vint8 class_file::tell ()
{
	if (split == 0)
	{
		return function_tell (file_pointer);
	}

	vint8 displacement = function_tell (file_pointer);
	vint8 additional = file_size*current_index;
	vint8 result = displacement + additional;
	return result;
}


vint8 fread(void* buffer, size_t type_size, size_t items, 
	class_file* file_pointer)
{
	return file_pointer->read (buffer, type_size, items);
}


vint8 fwrite (void* buffer, size_t type_size, size_t items, 
	class_file* file_pointer)
{
	return file_pointer->write(buffer, type_size, items);
}


vint8 fseek (class_file* file_pointer, long position, int origin)
{
	return file_pointer->seek (position, origin);
}


vint8 function_seek(class_file* file_pointer, vint8 position, int origin)
{
	return file_pointer->seek (position, origin);
}


long ftell(class_file* file_pointer)
{
	return (long) file_pointer->tell ();
}


vint8 function_tell (class_file* file_pointer)
{
	return file_pointer->tell ();
}


vint8 fclose (class_file* file_pointer)
{
	function_delete (file_pointer);
	return 1;
}


vint8 string_length (const char* argument)
{
	return strlen (argument);
}



file_handle * open_read_file(const char * filename)
{
	file_handle * result = fopen(filename, Vassilis_read_file);
	return result;
}


file_handle * open_save_file(const char * filename)
{
	file_handle * result = fopen(filename, Vassilis_save_file);
	return result;
}


vint8 close_file(file_handle * file_pointer)
{
	long result = fclose(file_pointer);
	return result;
}


vint8 function_read_floats(file_handle * file_pointer, float * numbers, const vint8 size)
{
	long items_read = fread(&numbers, sizeof(float), 1, file_pointer);
	return items_read;
}


vint8 function_save_floats(file_handle * file_pointer, const float * numbers, const vint8 size)
{
	vint8 items = fwrite(&numbers, sizeof(float), 1, file_pointer);
	return items;
}


long function_read_doubles(file_handle * file_pointer, double * numbers, long size)
{
	long items_read = fread(&numbers, sizeof(double), 1, file_pointer);
	return items_read;
}


long function_save_doubles(file_handle * file_pointer, double * numbers, long size)
{
	long items = fwrite(&numbers, sizeof(double), 1, file_pointer);
	return items;
}
